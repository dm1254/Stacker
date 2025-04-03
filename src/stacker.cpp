#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>


U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
const int Button = 12;
int block_Position_x[2] = {60,55};
int block_Position_y = 124;
int direction = 1;
volatile bool buttonPressed = false;
static unsigned long lastDebounceTime = 0;
static unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
const long interval = 200;
unsigned long lastButtonCheck = 0;
const long buttonCheckInterval = 20;
unsigned long lastPressTime = 0;



void setup() {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(Button,INPUT);

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
    
    u8g2.clearBuffer(); 
    for(int i=0; i<2; i++){
      u8g2.drawBox(block_Position_y,block_Position_x[i],4,4);
    }
  } while ( u8g2.nextPage() );

  if (buttonPressed){
    buttonPressed = false;
    delay(1000);
    block_Position_y -=4;
    block_Position_x[0] = 60;
    block_Position_x[1] = 55;
   
  }
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    if (block_Position_x[0] >= 60) {
      direction = -1;
    } else if (block_Position_x[0] <= 9) {
      direction = 1;
    }
    for (int i = 0; i < 2; i++) {
      block_Position_x[i] += 4 * direction;
    }

  }
}
