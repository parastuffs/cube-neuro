/**
 * WARNING Stay on esp32 library version <3.0.0. Version 2.0.17 is working fine.
 * With later version, they complied with ESP32 IDF v5, which changes lots of
 * definitions and breaks the compatibility of the audio library using the
 * internal DAC.
 * ESP 32 is a serie of microcontroler. 
 */

 /** All this code was in C ++
 This type of code is based on the following nomenclature
 
 Type of function, function name (paremeter1, parameter2, ...,) { Bloc of several instructions}
 Four type of principal functions
 Void function () {} // void mute function
 int function () {} // integer function, the expected output of this function is specificaly an integer.
 bool function () {} // boolean function, the expected output of this function is specificaly a bolean TRUE vs FALSE. 
 float function () {} // float function, the expectd output of this function is specificaly a float. 
*/

#include "Arduino.h" // the function "#include" is to load the library, is the equivalent of import in python and library in R. 
#include "Audio.h"
#include "FS.h"
#include <driver/i2s.h>

#define SD_CS          5 // the number five indicate that it is the fith pin. Thus here the SD card is connected to the fifth part.
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

// In this project, we designed 5 cubes with different configurations based on the following modes:

/*
 MODE_ALPHA: Always plays sound. This mode corresponds to the code section where #define MODE_ALPHA is used.
 */

/*
 * MODE_BETA: No sound. This mode corresponds to the code section where #define MODE_BETA is used.
 */

/*
 * MODE_GAMMA: Alternates between playing sound and no sound. This mode corresponds to the code section where #define MODE_GAMMA is used.
 */

/*
 * MODE_DELTA (YesNoYes): No sound for the first 7 interactions, then plays sound on the 8th interaction. This mode corresponds to the code section where #define MODE_DELTA is used.
 */

#define MODE_ALPHA


/**
 * Global variables
 */
int i; // Declares an integer variable named "i". This can be used for counting or indexing. In this case the purpose it to count the number of index for itterations 
unsigned long curtime;
// Declares a variable "curtime" to store the current time in milliseconds. This code is done to timing the operation, in this case, it permit to calculate the time duration of each button pression.
// This is useful for measuring how long something takes or timing events.

File myFile;
// Declares a variable "myFile" to work with files on an SD card.
// This allows the program to read from or write to files.

// The filename for experiment files is built in three parts:
const char* exp_ID_filename = "/exp_ID.txt";
// The base part of the filename, common to all experiment files. For some reason, the leading '/' is now required. In this case the line said to some extend "store the experiment ID counter"
const char* exp_filename_base = "/exp_";
// A file that stores the unique ID for each experiment, ensuring each filename is different. It is to some extend the "prefix" for each experiment file
const char* exp_filename_extension = ".csv";
// The file extension, indicating the file format (CSV in this case).

String exp_filename;
// A variable to hold the complete filename for the current experiment file.
// This is built using the base name, experiment ID, and file extension.

char buf[4];
// A small storage space (array) to temporarily hold data read from files, like the experiment ID.

int exp_ID = 0;
// A variable to store the current experiment ID, read from a file and incremented for each new experiment.

int modeSwitchTime = 4000;
// A variable to store a time duration in milliseconds (e.g., 4000 milliseconds = 4 seconds).
// The following sections define headers for logging data in different modes:


#ifdef  MODE_ALPHA 
// if the code detect that we use the cube alpha thus, it consider the sequence as follow.
const char* measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment, alpha";
#endif
// Defines the columns for logging data in MODE_ALPHA.

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
// An array to track whether each of the 5 buttons has been pressed.
// Initialized to "false" to indicate no buttons have been pressed at the start.

bool rising_edge[5] = {false, false, false, false, false};
bool rising_edge_detected[5] = {false, false, false, false, false};
// Arrays to track the start (rising edge) and end (falling edge) of button presses.

bool falling_edge[5] = {false, false, false, false, false};
unsigned long rising_time[5] = {0, 0, 0, 0, 0};
// An array to store the time when each button was first pressed.
// Initialized to 0, indicating no buttons have been pressed yet.

unsigned int cur_obs = 1;
// A variable to track the current observation number, starting at 1.

unsigned int demo_press_count = 0;
// A variable to count the number of demo button presses, starting at 0.

bool isStandby = false; // True, do not do anything in the main loop.
bool isExperiment = false; // False is demo, true is experiment
bool experimentEntry = false; // A variable to track entry into the experiment mode.

Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN); // Initializes an audio object for playing sound using both audio channels.

bool eof = false; // A variable to track if the end of the audio file has been reached.

void(* resetFunc) (void) = 0;
// Declares a function pointer that can be used to reset the system.
// Initialized to 0, meaning it doesn't point to any function initially.

void audio_eof_mp3(const char *info){  //end of file
    eof = true;
}
// A function called when the end of the audio file is reached, setting "eof" to true.
//eof means "end of file", thus the idea here is to say, when the audio last part of the sound is finished flag it as finished

/*
 * Function: play_audio
 * -------------------
 * Plays the current audio file to completion and prepares the next one.
 * 
 * The function works by:
 * 1. Running audio.loop() continuously until the end of file is reached
 * 2. Resetting the EOF flag after playback completes
 * 3. Loading the next audio file (sound.mp3) from SD card
 * 
 * Note: This function only loads the next file but does not start playing it.
 * Another call to play_audio() or other playback trigger would be needed to
 * play the newly loaded file.
 */


void play_audio() {
  // Continue processing audio data until end of file is reached
  while(not eof) {
    audio.loop();
  }
  // Reset EOF flag after playback completes
  eof = false;
  //audio.connecttoFS(SD, "/bird.mp3");  // Previously used audio file (disabled)
  audio.connecttoFS(SD, "/sound.mp3"); // Load next audio file from SD card
}


/** **********************
 *  
 *  SETUP
 * 
 * ***********************
 */
void setup() {
  // The setup function runs once when Arduino starts up
  // It initializes all necessary components and settings before the main program loop begins
  
  delay(100);
  // Short delay to ensure hardware is stable after power-up
  // Gives components time to reach proper operating state
  
  // while(!Serial);  
  // Commented out code - would wait until Serial connection is established
  // Useful for debugging when you need to see all messages from the beginning

  Serial.begin(115200);
  // Starts serial communication between Arduino and computer at 115200 bits per second
  // IMPORTANT: Set your Serial Monitor to the same baud rate (115200) or you will not see output

  /* Do not know why the delay works, but it avoids running the setup twice, creating empty experiments. */
  delay(1000);
  // One second delay prevents an issue where setup might run twice
  // This helps ensure reliable experiment initiation and prevents empty data records

  Serial.println("Starting setup");
  // Sends a message to Serial Monitor indicating setup has begun
  // Useful for confirming the program is running and tracking execution flow

 
  // === SD CARD INITIALIZATION ===
  
  pinMode(SD_CS, OUTPUT);
  // Configures the SD card's Chip Select pin as an OUTPUT
  // SD_CS is a pin number defined in line 25 in the code and correspond to the pin 5. 
  // This pin controls when the Arduino communicates with the SD card

  digitalWrite(SD_CS, HIGH);
  // Sets the SD card's Chip Select pin to HIGH (inactive state)
  // This temporarily disables communication with the SD card
  // When HIGH, the SD card ignores commands on the SPI bus

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  // Initializes the SPI communication protocol with specified pins:
  // - SCK: Clock signal that synchronizes data transmission
  // - MISO: Master In Slave Out - data from SD card to Arduino
  // - MOSI: Master Out Slave In - data from Arduino to SD card
  // SPI is the protocol used to communicate with SD cards and many other devices

  SPI.setFrequency(1000000);
  // Sets SPI communication speed to 1 MHz (1,000,000 Hz)
  // Lower frequencies provide more reliable communication with SD cards
  // Higher frequencies may cause errors with some SD cards or longer wires
  
  // IMPORTANT: This 1MHz frequency only sets the communication speed between the Arduino and SD card hardware.
  // It does NOT determine the sampling rate or how many data points per second will be saved in your CSV file.
  // Your actual data collection frequency is determined by your main program's timing code.
  
  if(!SD.begin(SD_CS)){
  // Attempts to initialize the SD card using the Chip Select pin
  // If initialization fails (returns false), the code inside these brackets runs
    
    Serial.println("Card Mount Failed, reset.");
    // Sends error message to Serial Monitor
    // "Card Mount Failed" means the Arduino couldn't establish communication with the SD card
    
    resetFunc();
    // Calls a function (defined elsewhere) that restarts the Arduino
    // This is a recovery mechanism - if SD card fails, start over
    
    return;
    // Exits the setup function early
    // Program won't continue beyond this point if SD card initialization failed
  }



// === BUTTON SETUP ===

  pinMode(button_A, INPUT);
  pinMode(button_B, INPUT);
  pinMode(button_C, INPUT);
  pinMode(button_D, INPUT);
  pinMode(button_E, INPUT);
  
  // Configures pins for buttons A through E as INPUTS
  // This tells the Arduino to read voltage levels on these pins
  // When buttons are pressed, the Arduino will detect the voltage change
  // Note: You might need pull-up/pull-down resistors depending on your button circuit


// === AUDIO AMPLIFIER SETUP ===

  pinMode(PAM8304_SD, OUTPUT);
  // Sets the PAM8304_SD pin as an OUTPUT
  // This pin controls the PAM8304 audio amplifier's shutdown mode.
  
  
  digitalWrite(PAM8304_SD, HIGH);
  // Sets the PAM8304_SD pin HIGH which activates the amplifier
  // LOW would put the amplifier in shutdown/sleep mode to save power 


// === EXPERIMENT FILE SETUP ===

  newExperimentFile();
  // Creates a new file on the SD card to store experiment data
  // This custom function likely generates a unique filename and opens it for writing
  // The function is defined elsewhere in your code
  
  audio.setVolume(21); 
  // Sets audio volume to maximum (21 is the highest value)
  // The audio library accepts volume values from 0 (mute) to 21 (loudest)

  audio.connecttoFS(SD, "/sound.mp3");
  // Connects the audio player to a specific MP3 file on the SD card
  // The file must be in the root directory of the SD card
  // Audio won't play until you call a play function


  //=== LED SETUP ===

  pinMode(led_pin, OUTPUT);
  // Configures the LED pin as an OUTPUT
  // This allows the Arduino to control the LED by setting the pin HIGH or LOW
  digitalWrite(led_pin, HIGH);
  // Turns ON the LED by setting its pin HIGH
  // The LED will remain on until the pin is set LOW

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
  // The loop function runs repeatedly after setup completes
  // This is where the main program logic executes continuously
  
  if(!isExperiment && !isStandby) {
    // Checks if we are NOT in experiment mode AND NOT in standby mode
    // isExperiment and isStandby are boolean variables defined elsewhere
    // Both conditions must be false to enter this code block
    
    demoRoutine();
    // Calls the demoRoutine function when in demo mode
    // This likely runs a demonstration sequence or test pattern
    // The function is defined elsewhere in the code
  }
  else if(isExperiment && !isStandby) {
    // Checks if we ARE in experiment mode AND NOT in standby mode
    // This condition runs when the device is actively collecting experimental data

    experimentRoutine();
    // Calls the experimentRoutine function for active experiment
    // This is where the actual data collection and experiment logic happens
    // The function is defined elsewhere in the code
  }
  delay(20);
  // Pauses program execution for 20 milliseconds
  // This creates a short delay between loop iterations (50 loops per second)
  // Helps prevent the processor from running too fast and provides timing control
}

void audio_info(const char *info) {
  // Callback function used by the audio library to report status information
  // This function is called automatically by the audio library when status changes
  
  Serial.print("info        "); Serial.println(info);
  // Prints the status information string and adds a line break
  // The "info" parameter contains status details from the audio library
}

/** **********************
 *  
 *  DEMONSTRATION ROUTINE
 * 
 * ***********************
 */
void demoRoutine()
{
// This function handles the demo mode behavior
// It runs continuously when the device is in demo mode (not in experiment or standby mode)  

  scanButtons(isExperiment);
  // Reads the current state of all buttons
  // The isExperiment parameter likely changes how button presses are processed or recorded
  // This function updates button state variables used below

  #ifdef MODE_ALPHA
  // Conditional compilation - this code only compiles if MODE_ALPHA is defined
  // Different modes allow different behaviors without changing the core code

  if(rising_edge_detected[0] || rising_edge_detected[1] || rising_edge_detected[2] || rising_edge_detected[3] || rising_edge_detected[4]) {
    // Checks if ANY of the five buttons has just been pressed (rising edge)
    // rising_edge_detected is an array of boolean values that tracks button press events
    
    rising_edge_detected[0] = false;
    rising_edge_detected[1] = false;
    rising_edge_detected[2] = false;
    rising_edge_detected[3] = false;
    rising_edge_detected[4] = false;
    // Resets all button press flags to prevent multiple triggers from a single press

    play_audio();
    // Plays the audio file specified earlier with audio.connecttoFS()
    // In MODE_ALPHA, any button press immediately triggers sound playback

    delay(300);
    // Waits 300 milliseconds (0.3 seconds) before checking buttons again
    // This prevents multiple rapid triggers from button bounce or quick presses
  }
  #endif

  #ifdef  MODE_GAMMA
  // Conditional compilation - this code only compiles if MODE_GAMMA is defined

  if(rising_edge_detected[0] || rising_edge_detected[1] || rising_edge_detected[2] || rising_edge_detected[3] || rising_edge_detected[4]) {
    // Checks if ANY of the five buttons has just been pressed

    rising_edge_detected[0] = false;
    rising_edge_detected[1] = false;
    rising_edge_detected[2] = false;
    rising_edge_detected[3] = false;
    rising_edge_detected[4] = false;
    // Resets all button press flags

    demo_press_count ++;
    // Increments a counter tracking how many times buttons have been pressed

    if(demo_press_count == 2) { 
      // When the counter reaches exactly 2 button presses

      play_audio();
      // Plays the sound file
      // In MODE_GAMMA, sound only plays after every 2nd button press

      demo_press_count = 0;
      // Resets the counter to start counting again

    }
    /* Delay for a time, limiting multiple concurrent presses and overlapping sounds. */
    delay(300);
    // Adds a short delay to prevent multiple rapid triggers
  }
  #endif

  #ifdef  MODE_DELTA
  // Conditional compilation - this code only compiles if MODE_DELTA is defined

  if(rising_edge_detected[0] || rising_edge_detected[1] || rising_edge_detected[2] || rising_edge_detected[3] || rising_edge_detected[4]) {
    // Checks if ANY of the five buttons has just been pressed

    rising_edge_detected[0] = false;
    rising_edge_detected[1] = false;
    rising_edge_detected[2] = false;
    rising_edge_detected[3] = false;
    rising_edge_detected[4] = false;
    // Resets all button press flags

    demo_press_count ++;
    // Increments the button press counter

    if(demo_press_count == 8) {
      // When the counter reaches exactly 8 button presses

      play_audio();
      // Plays the sound file
      // In MODE_DELTA, sound only plays after every 8th button press
      
      demo_press_count = 0;
      // Resets the counter to start counting again
    }
    /* Delay for a time, limiting multiple concurrent presses and overlapping sounds. */
    delay(300);
    // Adds a short delay to prevent multiple rapid triggers
  }
  #endif

  #ifdef  MODE_BETA
  /* Nothing happens in this mode. */

  // Conditional compilation - this section compiles if MODE_BETA is defined
  // In MODE_BETA, button presses do not trigger any actions
  // This comment documents that the empty section is intentional
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
  // This function handles the device behavior when in experiment mode
  // It is called continuously from the main loop when isExperiment is true
  
  // If we are switching to experiment, flash the light twice.
  if(experimentEntry) {
    // experimentEntry is a flag that is true only when first entering experiment mode
    // This ensures the initialization sequence below runs exactly once
    experimentEntry = false;
    // Clear the flag immediately to prevent this sequence from running again  

    digitalWrite(led_pin, LOW);
    delay(200);
    digitalWrite(led_pin, HIGH);
    delay(200);
    digitalWrite(led_pin, LOW);
    delay(200);
    digitalWrite(led_pin, HIGH);
    // Flashes the LED twice (OFF-ON-OFF-ON) with 200ms intervals
    // This provides visual feedback that experiment mode has been activated
    
    // Disable the audio amplifier... since in our protocol we wanted that the cube become silent during this phase. 
    digitalWrite(PAM8304_SD, LOW);
    // Sets the audio amplifier's shutdown pin to LOW
    // This puts the amplifier into sleep mode to conserve power during experiments
  }
  scanButtons(isExperiment);
  // Reads all button states and processes any presses
  // Passes isExperiment=true to indicate we are in experiment mode
  // This affects how button data is recorded (see scanButtons function)

  delay(5);
  // Small delay to control the sampling rate of button checking
  // 5ms delay means buttons are checked approximately 200 times per second

}

void scanButtons(bool isXP){
  // This function handles button detection and data recording
  // It tracks both press and release events for all five buttons
  // isXP parameter indicates whether we are in experiment mode

  curtime = millis();
  // Gets the current time in milliseconds since program start
  // Used for timestamping button events and measuring press duration

  if(digitalRead(button_A) && !rising_edge[0]){
    // Checks if button A is pressed (HIGH) AND was not pressed before
    // This detects only the moment when button state changes from unpressed to pressed

    Serial.print("A");
    // Sends the letter "A" to Serial Monitor (for debugging)
    rising_edge[0] = true;
    // Sets a flag indicating button A is currently pressed
    // This prevents detecting multiple press events while button remains held

    rising_edge_detected[0] = true;
    // Sets a flag that can be checked by other functions to know button A was just pressed
    // This flag is used by demoRoutine() to trigger actions
  }
  else if (!digitalRead(button_A) && rising_edge[0]) {
    // Checks if button A is NOT pressed (LOW) AND was previously pressed
    // This detects when button is released after being pressed

    rising_edge[0] = false;
    // Clears the pressed flag since button is no longer pressed

    falling_edge[0] = true;
    // Sets a flag indicating button A was just released
    // This will trigger data recording later in the function
  }

  // Similar blocks repeated for buttons B, C, D, and E
  if(digitalRead(button_B) && !rising_edge[1]){
    Serial.print("B");
    rising_edge[1] = true;
    rising_edge_detected[1] = true;
  }
  else if(!digitalRead(button_B) && rising_edge[1]){
    rising_edge[1] = false;
    falling_edge[1] = true;
  }
  if(digitalRead(button_C) && !rising_edge[2]){
    Serial.print("C");
    rising_edge[2] = true;
    rising_edge_detected[2] = true;
  }
  else if(!digitalRead(button_C) && rising_edge[2]){
    rising_edge[2] = false;
    falling_edge[2] = true;
  }
  if(digitalRead(button_D) && !rising_edge[3]){
    Serial.print("D");
    rising_edge[3] = true;
    rising_edge_detected[3] = true;
  }
  else if(!digitalRead(button_D) && rising_edge[3]){
    rising_edge[3] = false;
    falling_edge[3] = true;
  }
  if(digitalRead(button_E) && !rising_edge[4]){
    Serial.print("E");
    rising_edge[4] = true;
    rising_edge_detected[4] = true;
  }
  else if(!digitalRead(button_E) && rising_edge[4]){
    rising_edge[4] = false;
    falling_edge[4] = true;
  }
  
  for(i=0;i<5;i++) {
    // Loop through all 5 buttons to record press start times
    if(rising_edge[i] && !triggered[i]) {
    // If button is pressed AND hasn't been marked as triggered yet
      rising_time[i] = curtime;
      // Record the timestamp when the button was pressed
      triggered[i] = true;
      // Mark this button as triggered to prevent recording multiple start times
    }
  }

  for(i=0;i<5;i++) {
    // Loop through all 5 buttons to handle button releases
    if(falling_edge[i]) {
      // If this button was just released (falling edge detected)

      //measures_header = "observation, face, pression_start, pression_stop, sound, isExperiment";
      // Comment showing the CSV format used for data recording

      String measure_str = String(cur_obs) + "," + String(i+1) + "," + String(rising_time[i]) + "," + String(curtime) + "," + 0 + "," + isXP;
      // Creates a comma-separated data string with:
      // - Observation number (counter)
      // - Button number (i+1, so buttons are numbered 1-5 instead of 0-4)
      // - Timestamp when button was pressed
      // - Timestamp when button was released
      // - Sound flag (0 = no sound played)
      // - Experiment mode flag (true/false)

      unsigned long duration = curtime - rising_time[i];
      // Calculates how long the button was held down in milliseconds

      if (duration >= modeSwitchTime && !isXP){
        // If button was held for longer than modeSwitchTime AND not already in experiment mode
        
        isExperiment = true;
         // Switch to experiment mode
        experimentEntry = true;
        // Set flag so experimentRoutine() will run its initialization sequence
        Serial.println("Go to experiment!!!");
        // Print message to Serial Monitor indicating mode change
        
        //break;
        // (Commented out) Would exit the loop early
      }
      
      Serial.println("Writing this line to the SD card:");
      Serial.println(measure_str);
      
      // Debug output showing what data is being saved
      
      /* Write to SD card, then immediatly flush the data to make sure it is actually written.
       *  This removes the need to close the file handler. We can shut down the system without
       *  corrupting the SD card data.
       */

      myFile = SD.open(exp_filename, FILE_APPEND);
      // Opens the experiment file for appending (adding data to end)
      // exp_filename was set earlier in the program

      myFile.println(measure_str);
      // Writes the data string to the file

      myFile.close();
      // Closes the file to ensure data is saved
      // This is critical to prevent data loss if power is interrupted

      cur_obs++;
      // Increment the observation counter for the next button press

      triggered[i] = false;
      falling_edge[i] = false;
      // Reset flags for this button so new presses can be detected
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
  // This function creates a new experiment file on the SD card
  // It handles experiment numbering, file creation, and header writing

  Serial.println("Creating a new experiment file.");
  // Debug message indicating a new experiment file is being created
  
  /* Determine the experiment ID */
  if(!SD.exists(exp_ID_filename)) {
    // Checks if the experiment ID tracking file exists on the SD card
    // This file stores the last used experiment number

    Serial.println("The experiment list file does not exist yet");
    exp_ID = 1;
    // Sets the experiment ID to 1 since this is the first experiment

    writeFile(SD, exp_ID_filename, "1");
    // Creates the ID tracking file and writes "1" to it
    // This establishes the experiment numbering system
  
  }


  else {
    // If the experiment ID file already exists on the SD card

    Serial.println("ID file exists");
    // Debug message confirming the ID file was found

    myFile = SD.open(exp_ID_filename, FILE_READ);
    // Opens the ID file for reading

    while(myFile.available()) {
    // Continues as long as there's data to read
    // Typically just one line containing the latest experiment ID number

      int max_len = myFile.available();
    // Gets the number of bytes available to read
    
      int read_len = myFile.readBytesUntil('\n', buf, max_len);
      // Reads data from the file until a newline character or max_len bytes
      // Stores the data in the 'buf' array (defined elsewhere in the code)

      exp_ID = atoi(buf);
      // Converts the text from the file to an integer using atoi()
      // This gives us the last used experiment ID

    }
    myFile.close();
    // Closes the file after reading

    Serial.println("ID read:");
    Serial.println(String(exp_ID));
    // Displays the ID that was read from the file

    exp_ID += 1;
    // Increments the experiment ID for the new experiment

    Serial.print("Experiment ID: ");
    Serial.println(String(exp_ID));
    // Displays the new experiment ID

    myFile = SD.open(exp_ID_filename, FILE_WRITE);
    // Opens the ID file for writing (this will overwrite the existing content)

    myFile.println(String(exp_ID));
    // Writes the new experiment ID to the file

    myFile.close();
    // Closes the file to save the changes
  }

  /* Craft a new experiment file */
  exp_filename = String(exp_filename_base) + exp_ID + exp_filename_extension;
  // Creates the experiment filename by combining:
  // - A base prefix (defined elsewhere)
  // - The experiment ID number
  // - A file extension (like ".csv")
  // Example: "experiment_42.csv"


  Serial.print("New experiment filename: ");
  Serial.println(exp_filename);
  // Displays the complete filename for debugging

  myFile = SD.open(exp_filename, FILE_WRITE);
  // Creates the new experiment file (or overwrites if it somehow already exists)

  myFile.println(measures_header);
  // Writes the header row to the CSV file
  // measures_header is defined elsewhere and contains column names
  // Example: "observation,face,pression_start,pression_stop,sound,isExperiment"
  
  /* Do not close the file at this step, so that we can keep using it in the loop(). */
  //  myFile.flush();
  // Commented out code that would write data to SD card without closing the file


  myFile.close();
  // Closes the file to ensure the header is written
  // Comment explains this is necessary for Bluetooth compatibility
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  // This function lists all files in a directory on the SD card
  // Parameters:
  // - fs: The filesystem to use (SD in this case)
  // - dirname: The directory path to list
  // - levels: How many levels of subdirectories to include (recursive depth)

  Serial.printf("Listing directory: %s\n", dirname);
  // Displays which directory is being listed


  File root = fs.open(dirname);
  // Opens the specified directory

  if(!root){
    Serial.println("Failed to open directory");
    // Error message if the path isn't actually a directory
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  // Gets the first file/subdirectory in this directory

  while(file){
    // Continues until there are no more files/subdirectories
    if(file.isDirectory()){
      // If this item is a subdirectory
      Serial.print("  DIR : ");
      Serial.println(file.name());
      // Displays the subdirectory name

      if(levels){
        // If we haven't reached the maximum recursion depth

        listDir(fs, file.name(), levels -1);
        // Recursively list contents of this subdirectory with reduced depth count
      }
    } else {
      // If this item is a file

      Serial.print("  FILE: ");
      Serial.print(file.name());
      // Displays the filename

      Serial.print("  SIZE: ");
      Serial.println(file.size());
       // Displays the file size in bytes
    }
    file = root.openNextFile();
    // Gets the next file/subdirectory
  }
}

void readFile(fs::FS &fs, const char * path){
  // This function reads and displays the contents of a file
  // Parameters:
  // - fs: The filesystem to use (SD in this case)
  // - path: The file path to read

  Serial.printf("Reading file: %s\n", path);
  // Displays which file is being read

  File file = fs.open(path);
  // Opens the specified file

  if(!file){
    Serial.println("Failed to open file for reading");
    // Error message if file could not be opened
    return;
  }

  Serial.print("Read from file: ");
  // Label for the file contents

  while(file.available()){
  // Continues until there's no more data to read
  
    Serial.write(file.read());
  // Reads one byte at a time and sends it to Serial Monitor
  }
  file.close();
  // Closes the file when done
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  // This function creates a new file with the specified content
  // Parameters:
  // - fs: The filesystem to use (SD in this case)
  // - path: The file path to write
  // - message: The content to write to the file

  Serial.printf("Writing file: %s\n", path);
  // Displays which file is being written

  File file = fs.open(path, FILE_WRITE);
  // Opens the file for writing (creates new or overwrites existing)

  if(!file){
    Serial.println("Failed to open file for writing");
    // Error message if file could not be opened
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
    // Success message if write operation worked
  } else {
    Serial.println("Write failed");
    // Error message if write operation failed
  }
  file.close();
   // Closes the file to save changes
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  // This function adds content to the end of an existing file
  // Parameters:
  // - fs: The filesystem to use (SD in this case)
  // - path: The file path to append to
  // - message: The content to add to the file

  Serial.printf("Appending to file: %s\n", path);
  // Displays which file is being appended to

  File file = fs.open(path, FILE_APPEND);
  // Opens the file for appending (adding to end)

  if(!file){
    Serial.println("Failed to open file for appending");
    // Error message if file could not be opened
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
    // Success message if append operation worked
  } else {
    Serial.println("Append failed");
    // Error message if append operation failed
  }
  file.close();
  // Closes the file to save changes
}

void disableWiFi(){
  // This function turns off WiFi to save power

  //adc_power_off();
  // Commented out code that would turn off the analog-to-digital converter
  
    WiFi.disconnect(true);
    // Disconnects from any connected WiFi network
    // The 'true' parameter erases stored network settings

    WiFi.mode(WIFI_OFF);
    // Completely turns off the WiFi radio
}

void disableBluetooth(){
  // This function turns off Bluetooth to save power
    btStop();
  // Stops the Bluetooth controller
  // This saves significant power on ESP32-based Arduinos
}
