/**
 * WARNING Stay on esp32 library version <3.0.0. Version 2.0.17 is working fine.
 * With later version, they complied with ESP32 IDF v5, which changes lots of
 * definitions and breaks the compatibility of the audio library using the
 * internal DAC.
 */

#include "Arduino.h"
#include "Audio.h"
#include "FS.h"
#include <driver/i2s.h>

#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define button_A       4 // Top face
#define button_B      32 // Board side
#define button_C      33 // Speaker side
#define button_D      15 // Left side
#define button_E      13 // Right side

#define PAM8304_SD    27 // /SD input of the audio amplifier

#define led_pin 22

/* Alpha: Always play sound */
//#define MODE_ALPHA
/* Beta: No sound */
//#define MODE_BETA
/* Gamma: Sound / No sound */
//#define MODE_GAMMA
/* Delta (YesNoYes): No sound*7 / Sound */
#define MODE_DELTA

/**
 * Global variables
 */
int i;
unsigned long curtime;
File myFile;
const char* exp_ID_filename = "/exp_ID.txt"; // For some reason, the leading '/' is now required.
const char* exp_filename_base = "/exp_";
const char* exp_filename_extension = ".csv";
String exp_filename;
char buf[4];
int exp_ID = 0;

int modeSwitchTime = 2000; // 2 seconds
  
#ifdef  MODE_ALPHA
const char* measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment, alpha";
#endif
#ifdef  MODE_BETA
const char* measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment, beta";
#endif
#ifdef  MODE_GAMMA
const char* measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment, gamma";
#endif
#ifdef  MODE_DELTA
const char* measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment, delta";
#endif

bool triggered[5] = {false, false, false, false, false};
bool rising_edge[5] = {false, false, false, false, false};
bool falling_edge[5] = {false, false, false, false, false};
unsigned long rising_time[5] = {0, 0, 0, 0, 0};
unsigned int cur_obs = 1;
unsigned int demo_press_count = 0;

bool isStandby = false; // True, don't do anything in the main loop.
bool isExperiment = false; // False is demo, true is experiment
bool experimentEntry = false;

Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);

bool eof = false;

void(* resetFunc) (void) = 0;

void audio_eof_mp3(const char *info){  //end of file
    eof = true;
}

void play_audio() {
  while(not eof) {
    audio.loop();
  }
  eof = false;
  //audio.connecttoFS(SD, "/bird.mp3");
  audio.connecttoFS(SD, "/sound.mp3");
}


/** **********************
 *  
 *  SETUP
 * 
 * ***********************
 */
void setup() {
  delay(100);
  //while(!Serial);
  Serial.begin(115200);
  /* Don't know why the delay works, but it avoids to run the setup twice, creating empty experiments. */
  delay(1000);

  Serial.println("Starting setup");
  
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  
  if(!SD.begin(SD_CS)){
    Serial.println("Card Mount Failed, reset.");
    resetFunc();
    return;
  }

  pinMode(button_A, INPUT);
  pinMode(button_B, INPUT);
  pinMode(button_C, INPUT);
  pinMode(button_D, INPUT);
  pinMode(button_E, INPUT);

  pinMode(PAM8304_SD, OUTPUT);
  digitalWrite(PAM8304_SD, HIGH);

  newExperimentFile();

//  listDir(SD, "/", 0);
  
  audio.setVolume(21); // 0...21

  audio.connecttoFS(SD, "/sound.mp3");

  //play_audio();

  
  pinMode(led_pin, OUTPUT); // Specify that LED pin is output
  digitalWrite(led_pin, HIGH);

  //disableWiFi();
  //disableBluetooth();

}


/** **********************
 *  
 *  MAIN LOOP
 * 
 * ***********************
 */
void loop()
{
  if(!isExperiment && !isStandby) {
    demoRoutine();
  }
  else if(isExperiment && !isStandby) {
    experimentRoutine();
  }
  delay(20);
}

void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}

/** **********************
 *  
 *  DEMONSTRATION ROUTINE
 * 
 * ***********************
 */
void demoRoutine()
{
  
  scanButtons(isExperiment);
  
  #ifdef MODE_ALPHA
  if(digitalRead(button_A) || digitalRead(button_B) || digitalRead(button_C) || digitalRead(button_D) || digitalRead(button_E)) {
    play_audio();
    /* Delay for a time, limiting multiple concurrent presses and overlapping sounds. */
    delay(300);
  }
  #endif

  #ifdef  MODE_GAMMA
  if(digitalRead(button_A) || digitalRead(button_B) || digitalRead(button_C) || digitalRead(button_D) || digitalRead(button_E)) {
    demo_press_count ++;
    if(demo_press_count == 2) {
      play_audio();
      demo_press_count = 0;
    }
    /* Delay for a time, limiting multiple concurrent presses and overlapping sounds. */
    delay(300);
  }
  #endif

  #ifdef  MODE_DELTA
  if(digitalRead(button_A) || digitalRead(button_B) || digitalRead(button_C) || digitalRead(button_D) || digitalRead(button_E)) {
    demo_press_count ++;
    if(demo_press_count == 8) {
      play_audio();
      demo_press_count = 0;
    }
    /* Delay for a time, limiting multiple concurrent presses and overlapping sounds. */
    delay(300);
  }
  #endif

  #ifdef  MODE_BETA
  /* Nothing happens in this mode. */
  #endif
}


/** **********************
 *  
 *   EXPERIMENT ROUTINE
 * 
 * ***********************
 */
void experimentRoutine()
{
  
  // If we are switching to experiment, flash the light twice.
  if(experimentEntry) {
    experimentEntry = false;
    digitalWrite(led_pin, LOW);
    delay(200);
    digitalWrite(led_pin, HIGH);
    delay(200);
    digitalWrite(led_pin, LOW);
    delay(200);
    digitalWrite(led_pin, HIGH);

    // Disable the audio amplifier to save battery
    digitalWrite(PAM8304_SD, LOW);
  }
  scanButtons(isExperiment);
  delay(5);


}

void scanButtons(bool isXP){
  curtime = millis();
  // TODO FACTORISE THIS!
  if(digitalRead(button_A)){
    Serial.print("A");
    rising_edge[0] = true;
  }
  else if (!digitalRead(button_A) && rising_edge[0]) {
    rising_edge[0] = false;
    falling_edge[0] = true;
  }
  if(digitalRead(button_B)){
    Serial.print("B");
    rising_edge[1] = true;
  }
  else if(!digitalRead(button_B) && rising_edge[1]){
    rising_edge[1] = false;
    falling_edge[1] = true;
  }
  if(digitalRead(button_C)){
    Serial.print("C");
    rising_edge[2] = true;
  }
  else if(!digitalRead(button_C) && rising_edge[2]){
    rising_edge[2] = false;
    falling_edge[2] = true;
  }
  if(digitalRead(button_D)){
    Serial.print("D");
    rising_edge[3] = true;
  }
  else if(!digitalRead(button_D) && rising_edge[3]){
    rising_edge[3] = false;
    falling_edge[3] = true;
  }
  if(digitalRead(button_E)){
    Serial.print("E");
    rising_edge[4] = true;
  }
  else if(!digitalRead(button_E) && rising_edge[4]){
    rising_edge[4] = false;
    falling_edge[4] = true;
  }
  
  for(i=0;i<5;i++) {
    if(rising_edge[i] && !triggered[i]) {
      rising_time[i] = curtime;
      triggered[i] = true;
    }
  }

  for(i=0;i<5;i++) {
    if(falling_edge[i]) {
      //measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment";
      String measure_str = String(cur_obs) + "," + String(i+1) + "," + String(rising_time[i]) + "," + String(curtime) + "," + 0 + "," + isXP;
      unsigned long duration = curtime - rising_time[i];
      if (duration >= modeSwitchTime && !isXP){
        isExperiment = true;
        experimentEntry = true;
        Serial.println("Go to experiment!!!");
        //break;
      }
      Serial.println("Writing this line to the SD card:");
      Serial.println(measure_str);
      /* Write to SD card, then immediatly flush the data to make sure it's actually written.
       *  This removes the need to close the file handler. We can shut down the system without
       *  corrupting the SD card data.
       */
      myFile = SD.open(exp_filename, FILE_APPEND);
      myFile.println(measure_str);
      myFile.close();
      cur_obs++;
      triggered[i] = false;
      falling_edge[i] = false;
    }
  }

//  //Serial.println("-----------");
//  for(i=1;i<5;i++) {
////    Serial.print("Button ");
////    Serial.print(i);
////    Serial.print("(");
////    Serial.print(falling_edge[i]);
////    Serial.print(")");
////    Serial.print(" pressed, rise time: ");
////    Serial.print(rising_time[i]);
////    Serial.print(", curtime: ");
////    Serial.print(curtime);
////    Serial.print("\n");
//    if(falling_edge[i]) {
//      unsigned long duration = curtime - rising_time[i];
//      if (duration >= modeSwitchTime){
//        isExperiment = true;
//        Serial.println("Go to experiment!!!");
//        break;
//      }
//    }
//  }
}

void newExperimentFile(){
  Serial.println("Creating a new experiment file.");
  /* Determine the experiment ID */
  if(!SD.exists(exp_ID_filename)) {
    Serial.println("The experiment list file does not exist yet");
    exp_ID = 1;
    writeFile(SD, exp_ID_filename, "1");
  }
  else {
    Serial.println("ID file exists");
    myFile = SD.open(exp_ID_filename, FILE_READ);
    while(myFile.available()) {
      int max_len = myFile.available();
      int read_len = myFile.readBytesUntil('\n', buf, max_len);
      exp_ID = atoi(buf);
    }
    myFile.close();
    Serial.println("ID read:");
    Serial.println(String(exp_ID));
    exp_ID += 1;
    Serial.print("Experiment ID: ");
    Serial.println(String(exp_ID));
    myFile = SD.open(exp_ID_filename, FILE_WRITE);
    myFile.println(String(exp_ID));
    myFile.close();
  }

  /* Craft a new experiment file */
  exp_filename = String(exp_filename_base) + exp_ID + exp_filename_extension;
  Serial.print("New experiment filename: ");
  Serial.println(exp_filename);
  myFile = SD.open(exp_filename, FILE_WRITE);
  myFile.println(measures_header);
  /* Do not close the file at this step, so that we can keep using it in the loop(). */
//  myFile.flush();
  myFile.close(); // If I want Bluetooth to work, I actually need to close the file. I can't keep it open.
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void disableWiFi(){
    //adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void disableBluetooth(){
    btStop();
}
