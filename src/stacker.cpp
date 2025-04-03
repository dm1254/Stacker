#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
const int Button = 12;
int block_Position_x[2] = {60,55};
int block_Position_y = 124;
int direction = 1;

bool Debounce(int Button){
  
  static int buttonState = LOW;
  static int lastButtonState = LOW; 
  static unsigned long lastDebounceTime = 0;
  static unsigned long debounceTime = 100;

  int reading = digitalRead(Button);
  if(reading != lastButtonState){
    lastDebounceTime = millis();
  }
  if((millis() - lastDebounceTime)> debounceTime){
    if(reading != buttonState){
      buttonState = reading;
      return true;
    }
  }
  return false;
}


void setup(void) {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(Button,INPUT);
}

void loop(void) {
  bool buttonPressed = Debounce(Button);
  u8g2.firstPage();
  do {
    
    u8g2.clearBuffer(); 
    for(int i=0; i<2; i++){
      u8g2.drawBox(block_Position_y,block_Position_x[i],4,4);
    }
  } while ( u8g2.nextPage() );
  

  if(buttonPressed){
    block_Position_y -= 4;
    u8g2.setCursor(50,30);
    u8g2.print("pressed");
  }else{
    for(int i=0;i<2;i++){
      block_Position_x[i] += 4 * direction;
    }

    if(block_Position_x[0] >= 60){
      direction = -1;
    }else if(block_Position_x[0] <= 8){
      direction = 1;
    }
  }
  delay(500);
}


