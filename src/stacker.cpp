/*
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "music.h"

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
const int resetButton = 3;
const int Button = 12;
int block_Position_x[3] = {58,52,46};
int block_Position_y = 122;
int static_block_x[16];
int static_block_y[16];
int direction = 1;
volatile bool buttonPressed = false;
static unsigned long lastDebounceTime = 0;
static unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
const long interval = 200;
unsigned long lastButtonCheck = 0;
const long buttonCheckInterval = 20;
unsigned long lastPressTime = 0;
int level = 0;
int numberofBlock = 3;
int set_x[3];
bool onStack[3] = {true,true,true};
int dropBlock = false;
bool firstRun = true; 
int new_position_x[3]; 
void (* resetFunc) (void) = 0;

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(Button,INPUT);
  pinMode(resetButton,INPUT);
  Serial.println(onStack[0]);
  Serial.println(onStack[1]);
  Serial.println(onStack[2]);
}

void checkButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonCheck >= buttonCheckInterval) {
    int buttonState = digitalRead(Button);
    if (buttonState == HIGH && currentTime - lastPressTime > debounceDelay) {
      buttonPressed = true;
      lastPressTime = currentTime;
    }
    lastButtonCheck = currentTime;
  }
}



void loop() {




  unsigned long currentMillis = millis();
  checkButton();
  u8g2.firstPage();
  do {
    if(dropBlock){
      for(int i=0;i<3 ;i++){

        if(onStack[i]){
          u8g2.drawBox(block_Position_y,block_Position_x[i],6,6);
        }
      }
    }else{
      for(int i=0; i<numberofBlock; i++){
        u8g2.drawBox(block_Position_y,block_Position_x[i],6,6);
      }
    }
    for(int i =0; i<level; i++){
      if(static_block_x[i] != 0){
        u8g2.drawBox(static_block_y[i],static_block_x[i],6,6);
      }
    }
  } while ( u8g2.nextPage() );
  dropBlock = false;
  if(digitalRead(resetButton) == HIGH){
      resetFunc();
  }

  if (buttonPressed){
    Serial.println("Button pressed");

    buttonPressed = false;
    delay(1000);

    Serial.println(block_Position_x[0]);
    Serial.println(set_x[0]);
    if(firstRun){
      for(int i =0;i<3;i++){
        set_x[i] = block_Position_x[i];
      }
      firstRun = false;
    }else{
          
      for(int i = 0; i<3;i++){
        if(block_Position_x[i] != set_x[i]){
          onStack[i] = false;
          --numberofBlock;
          dropBlock = true;
        }
      }
          if(numberofBlock == 0){
            resetFunc();
        }
      }
    }
    for(int i = 0;i< 3;i++){
      static_block_x[level] = block_Position_x[i];
      static_block_y[level] = block_Position_y;
      level++;
    }

    block_Position_y -=6;
    block_Position_x[0] = 58;
    block_Position_x[1] = 52;
    block_Position_x[2] = 46;
    delay(10);

    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      if (block_Position_x[0] >= 58) {
        direction = -1;
      } else if (block_Position_x[0] <= 18) {
        direction = 1;
      }
      for (int i = 0; i < 3; i++) {
        block_Position_x[i] += 6 * direction;
      }

    }

}
*/
