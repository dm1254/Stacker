#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>


U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
const int resetButton = 3;
const int Button = 12;
int block_Position_x[2] = {60,55};
int block_Position_y = 124;
int static_block_x[32];
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
int set_x[2];
bool firstRun = true; 
 
void (* resetFunc) (void) = 0;

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(Button,INPUT);
  pinMode(resetButton,INPUT);

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

    for(int i=0;i<sizeof(static_block_y)/sizeof(static_block_y[0]);i++){
      for(int j = 0; j<sizeof(static_block_x)/sizeof(static_block_x[0]); j++){
        u8g2.drawBox(static_block_y[i],static_block_x[j],4,4);
      }
    }
  } while ( u8g2.nextPage() );
  
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
      for(int i =0;i<2;i++){
        set_x[i] = block_Position_x[i];
      }
      firstRun = false;
    }else{
      if(block_Position_x[0] != set_x[0]){
          resetFunc();
      }
    }
    for(int i = 0;i< 2;i++){
      static_block_x[level+1] = block_Position_x[i];
      static_block_y[level+1] = block_Position_y;

      level++;
    }

    block_Position_y -=4;
    block_Position_x[0] = 60;
    block_Position_x[1] = 55;


  }
    delay(10);


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
