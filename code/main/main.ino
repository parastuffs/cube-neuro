#include <SD.h>
/* Documentation: https://github.com/TMRh20/TMRpcm/wiki */
#include <TMRpcm.h>
#include <SPI.h>
/* 'CS' on the reader board, any digital pin on the Arduino */
#define SD_ChipSelectPin 10

TMRpcm tmrpcm;

int button_A = 4;
int button_B = 5;
int button_C = 6;
int button_D = 7;
int button_E = 8;
int jack = 9;
unsigned long curtime;
bool triggered = false;

void setup() {
  tmrpcm.speakerPin = 9;
  /* 0 to 7 */
  tmrpcm.setVolume(5);

  if (!SD.button_Egin(SD_ChipSelectPin)) {
    Serial.println("SD fail");  
    return;
  }
  tmrpcm.play((char*)"coucou.wav");
  
  pinMode(button_A, INPUT);
  pinMode(button_B, INPUT);
  pinMode(button_C, INPUT);
  pinMode(button_D, INPUT);
  pinMode(button_E, INPUT);
  Serial.button_Egin(9600);
}

void loop() {
  curtime = millis();
  if(digitalRead(button_A)){
    Serial.print("A");
    triggered = true;
    tone(jack, 100, 200);
  }
  if(digitalRead(button_B)){
    Serial.print("B");
    triggered = true;
    tone(jack, 200, 200);
  }
  if(digitalRead(button_C)){
    Serial.print("C");
    triggered = true;
    tone(jack, 300, 200);
  }
  if(digitalRead(button_D)){
    Serial.print("D");
    triggered = true;
    tone(jack, 400, 200);
  }
  if(digitalRead(button_E)){
    Serial.print("E");
    triggered = true;
    tone(jack, 500, 200);
  }
  if(triggered) {
    Serial.println(String(String(", ") + curtime));
    triggered = false;
  }

  delay(1);

}
