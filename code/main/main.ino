#include <SD.h>
/* Documentation: https://github.com/TMRh20/TMRpcm/wiki */
#include <TMRpcm.h>
#include <SPI.h>
/* 'CS' on the reader board, any digital pin on the Arduino */
#define SD_ChipSelectPin 10
#define button_A 4
#define button_B 5
#define button_C 6
#define button_D 7
#define button_E 8
#define speaker 9
#define mode 2
//#define MODE_YELLOW
#define MODE_RED
//#define MODE_GREEN

/**
 * Global variables
 */
int i;
unsigned long curtime;
TMRpcm tmrpcm;
File myFile;
/*  The SD library uses the old 8.3 filename convention,
 *  so filenames can only be 8 characters long top, with
 *  3 characters for the extension (dot excluded).
 */
const char* exp_ID_filename = "exp_ID.txt";
const char* exp_filename_base = "exp_";
const char* exp_filename_extension = ".csv";
String exp_filename;
char buf[4];
int exp_ID = 0;
const char* measures_header = "observation, face, pression_start, pression_stop, sound";

//bool triggered = false;
bool triggered[5] = {false, false, false, false, false};
bool rising_edge[5] = {false, false, false, false, false};
bool falling_edge[5] = {false, false, false, false, false};
unsigned long rising_time[5] = {0, 0, 0, 0, 0};

unsigned int cur_obs = 1;

void setup() {
  Serial.begin(9600);

  /* Don't know why the delay works, but it avoids to run the setup twice, creating empty experiments. */
  delay(1000);

  // If analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  // https://reference.arduino.cc/reference/en/language/functions/random-numbers/random/
  randomSeed(analogRead(0));
  
  tmrpcm.speakerPin = 9;
  /* 0 to 7 */
  tmrpcm.setVolume(5);

  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD fail");  
    return;
  }

  /* Determine the experiment ID */
  if(!SD.exists(exp_ID_filename)) {
    //Serial.println("The file does not exist yet");
    myFile = SD.open(exp_ID_filename, FILE_WRITE);
    exp_ID = 1;
    myFile.println("1");
    myFile.close();
  }
  else {
    //Serial.println("ID file exists");
    myFile = SD.open(exp_ID_filename, FILE_READ);
    while(myFile.available()) {
      int max_len = myFile.available();
      int read_len = myFile.readBytesUntil('\n', buf, max_len);
      exp_ID = atoi(buf);
    }
    myFile.close();
    //Serial.println("ID read:");
    //Serial.println(String(exp_ID));
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
  myFile.flush();
  
  
  pinMode(button_A, INPUT);
  pinMode(button_B, INPUT);
  pinMode(button_C, INPUT);
  pinMode(button_D, INPUT);
  pinMode(button_E, INPUT);
  pinMode(mode, INPUT);

  delay(1000);

  
  //tmrpcm.play((char*)"coucou.wav");
}

void loop() {
  if(!digitalRead(mode)) {
    demoRoutine();
  }
  else {
    experimentRoutine();
  }
}

void playSound() {
    tmrpcm.play((char*)"coucou.wav");
    delay(100);
}

void demoRoutine()
{
  #ifdef MODE_YELLOW
  /* A sound is played on every button press. */
  if(digitalRead(button_A) || digitalRead(button_B) || digitalRead(button_C) || digitalRead(button_D) || digitalRead(button_E)) {
    playSound();
  }
  #endif

  #ifdef  MODE_RED
  /* A sound is played with a 50% chance on a button press. */
  if(digitalRead(button_A) || digitalRead(button_B) || digitalRead(button_C) || digitalRead(button_D) || digitalRead(button_E)) {
    // Delay a bit for the button to settle, otherwise we draw several random values at once.
    delay(500);
    if(random(100) > 50) {
      playSound();
    }
  }
  #endif

  #ifdef  MODE_GREEN
  /* Nothing happens in this mode. */
  #endif
}

void experimentRoutine()
{
  tmrpcm.disable();
  curtime = millis();
  // TODO FACTORISE THIS!
  if(digitalRead(button_A)){
    //Serial.print("A");
    rising_edge[0] = true;
  }
  else if (!digitalRead(button_A) && rising_edge[0]) {
    rising_edge[0] = false;
    falling_edge[0] = true;
  }
  if(digitalRead(button_B)){
    //Serial.print("B");
    rising_edge[1] = true;
  }
  else if(!digitalRead(button_B) && rising_edge[1]){
    rising_edge[1] = false;
    falling_edge[1] = true;
  }
  if(digitalRead(button_C)){
    //Serial.print("C");
    rising_edge[2] = true;
  }
  else if(!digitalRead(button_C) && rising_edge[2]){
    rising_edge[2] = false;
    falling_edge[2] = true;
  }
  if(digitalRead(button_D)){
    //Serial.print("D");
    rising_edge[3] = true;
  }
  else if(!digitalRead(button_D) && rising_edge[3]){
    rising_edge[3] = false;
    falling_edge[3] = true;
  }
  if(digitalRead(button_E)){
    //Serial.print("E");
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
      String measure_str = String(cur_obs) + "," + String(i+1) + "," + String(rising_time[i]) + "," + String(curtime) + "," + 0;
      //Serial.println("Writing this line to the SD card:");
      //Serial.println(measure_str);
      myFile.println(measure_str);
      myFile.flush();
      cur_obs++;
      triggered[i] = false;
      falling_edge[i] = false;
    }
  }

  delay(5);


}
