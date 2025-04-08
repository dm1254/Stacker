#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "pitches.h"
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2
#define CS_PIN 10
#define CLK_PIN 13
#define DATA_PIN 11

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE,DATA_PIN,CLK_PIN,CS_PIN,MAX_DEVICES);

// I/O devices
const int RLED = 9;
const int GLED = 6;
const int BLED = 5;
const int SPEAKER = 3;
const int Button = 2;
const int resetButton = 4;
// Debounce variables
static unsigned long lastDebounceTime = 0;
static unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
const long interval = 200;
unsigned long lastButtonCheck = 0;
const long buttonCheckInterval = 20;
unsigned long lastPressTime = 0;
//Game variables
bool buttonPressed = false;
int blockWidth = 3; // Width of the moving block (max 8 columns)
int blockPos = 0;
int currentLevel = 15;         // Current row (15 = bottom, 0 = top)
bool movingRight = true;     // Direction of block movement
int stack[16][8] = {0};      // Stack array (16 rows, 8 columns; 1 = LED on, 0 = off)
bool gameOver = false; 
bool gameWon = false;
unsigned long lastMove = 0;  // Last block move time
long moveInterval = 115;
int direction =1;
//Music
//Note Array
int notesWon[] = {
  NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_C5, NOTE_E5, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4 };
//The Duration of each note (in ms)
int noteCountWon = sizeof(notesWon)/sizeof(notesWon[0]); 
int notesLose[] = {
  NOTE_B4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B3 };
//The Duration of each note (in ms)
int noteCountLose = sizeof(notesLose)/sizeof(notesLose[0]);

int notesMain[] = {
      NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
      NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
      NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4,
      NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4,
      NOTE_C4, NOTE_E4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_G4,
      NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4,
      NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4 };
//The Duration of each note (in ms)
int noteCountMain = sizeof(notesMain)/ sizeof(notesMain[0]); 
unsigned long previousLEDTime;
unsigned long lastNoteTime;
bool ledState = LOW;
const int noteInterval = 400;
const int blinkPeriod = 400;
int currentNoteMain = 0;
int currentNoteWon = 0;
int currentNoteLose = 0;
bool notePlaying = false;
//Setup
void setup(){
  Serial.begin(9600);
  mx.begin();
  mx.clear();
  pinMode(RLED,OUTPUT);
  pinMode(GLED,OUTPUT);
  pinMode(BLED,OUTPUT);
  pinMode(SPEAKER,OUTPUT);
  pinMode(resetButton,INPUT);
  pinMode(Button,INPUT);
}


// Functions
void (* resetFunc) (void) = 0;

void setPoint(int row, int col, bool state) {
  // Native: 8 rows, 16 cols (0-7 first/bottom module, 8-15 second/top module)
  // Desired: 16 rows (0 top of second module, 15 bottom of first module), 8 cols (0 left, 7 right)
  // Invert row mapping: row 15 → orig col 0 (first module), row 0 → orig col 15 (second module)
  int origRow, origCol;
  int invertedRow = 15 - row;  // Invert row: 15→0, 0→15
  if (invertedRow < 8) {  // First module (rows 15-8, orig cols 0-7)
    origRow = 7 - col;    // Cols invert to match left-right
    origCol = invertedRow;  // Map to cols 0-7
  } else {  // Second module (rows 7-0, orig cols 8-15)
    origRow = 7 - col;
    origCol = invertedRow - 8 + 8;  // Offset to cols 8-15
  }
  mx.setPoint(origRow, origCol, state);
}

void blinkRed(){
  unsigned long currentTime = millis();
  if(currentTime - previousLEDTime >= blinkPeriod){
    previousLEDTime = currentTime;
    ledState = !ledState;

    digitalWrite(RLED, ledState ? HIGH:LOW);
    digitalWrite(GLED,LOW);
    digitalWrite(BLED,LOW);
  }
}


void blinkBlue(){
  unsigned long currentTime = millis();
  if(currentTime - previousLEDTime >= blinkPeriod){
    previousLEDTime = currentTime;
    ledState = !ledState;

    digitalWrite(RLED,LOW);
    digitalWrite(GLED,LOW);
    digitalWrite(BLED,ledState ? HIGH:LOW);
  }
}


void gameStateMusicAndLED(){
  unsigned long currentTime = millis();
  int noteInterval = 400;

  if(gameWon == true){
    blinkBlue();
    if(currentNoteWon < noteCountWon){
      if(!notePlaying){
        tone(3,notesWon[currentNoteWon]);
        notePlaying = true;
        lastNoteTime = millis();
        }
    
      if(currentTime - lastNoteTime >= noteInterval){
        noTone(SPEAKER);
        currentNoteWon++;
        notePlaying = false;
      }
    }
    
  }else if(gameOver == true){
    blinkRed();

    if(currentNoteLose < noteCountLose){
      if(!notePlaying){
        tone(3,notesLose[currentNoteLose]);
        notePlaying = true;
        lastNoteTime = millis();
        }
    
      if(currentTime - lastNoteTime >= noteInterval){
        noTone(SPEAKER);
        currentNoteLose++;
        notePlaying = false;
      }
    }
  }else{
    digitalWrite(RLED,HIGH);
    digitalWrite(GLED,LOW);
    digitalWrite(BLED,LOW);
    if(!gameOver || !gameWon){
      if(!notePlaying){
        tone(3,notesMain[currentNoteMain]);
        notePlaying = true;
        lastNoteTime = millis();
        }
    
      if(currentTime - lastNoteTime >= noteInterval){
        noTone(SPEAKER);
        currentNoteMain++;
        notePlaying = false;
        if(currentNoteMain >= noteCountMain){
          currentNoteMain = 0;
        }
      }

    }
  }
}
void checkresetButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonCheck >= buttonCheckInterval) {
    int buttonState = digitalRead(resetButton);
    if (buttonState == HIGH && currentTime - lastPressTime > debounceDelay) {
      resetFunc();
      lastPressTime = currentTime;
    }
    lastButtonCheck = currentTime;
  }
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


void loop(){
  checkresetButton();
  gameStateMusicAndLED();
  if(!gameOver){
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= moveInterval){
      for(int i= blockPos; i < blockPos + blockWidth; i++){
        setPoint(currentLevel,i,false);
      }
      blockPos += direction;
      if(blockPos >= 8-blockWidth){
        blockPos = 8-blockWidth;
        direction = -1;
      }else if(blockPos <= 0){
        blockPos =0;
        direction =1;
      }
      for(int i = blockPos; i<blockPos+blockWidth;i++){
        setPoint(currentLevel,i,true);
      }
      for(int r=0;r<16;r++){
        for(int c=0;c<8;c++){
          if(stack[r][c]){ 
            setPoint(r,c,true);
          }
        }
      }
      previousMillis = currentMillis;
    }

    //Button Press function
    checkButton();

    //Check for button press
    if(buttonPressed){
      buttonPressed = false;  // Reset button state
      delay(200); // Pause to show block position
      
      // Turn off the current block
      for (int i = blockPos; i < blockPos + blockWidth; i++)
        setPoint(currentLevel, i, false);

      // Check alignment with stack below
      if (currentLevel < 15) {
        // Find overlap with stack below
        int newBlockPos = blockPos;
        int newBlockWidth = 0;
        for (int i = blockPos; i < blockPos + blockWidth; i++) {
          
          if (stack[currentLevel + 1][i] == 1) {
            newBlockWidth++;  // Count supported LEDs
          } else if (newBlockWidth == 0) {
            newBlockPos++;  // Shift right until first overlap
          }
        }

        // Update block properties
        if (newBlockWidth <= 0) {
          gameOver = true;  // No overlap, game over
        } else {
          blockPos = newBlockPos;  // Adjust position to overlap
          blockWidth = newBlockWidth;  // Shrink to supported width
        }
      }

      // Place block in stack (only where supported)
      if (!gameOver) {
        for (int i = blockPos; i < blockPos + blockWidth; i++) {
          if (currentLevel == 15 || stack[currentLevel + 1][i] == 1) {
            stack[currentLevel][i] = 1;  // Set stack only if supported
          }
        }

        // Move to next row up
        currentLevel--;
        if (currentLevel < 0) {
          gameWon = true;  // Game ends if top reached
        } else {
          // Adjust blockPos if it exceeds new blockWidth
          blockPos = 0;// Initialize new block
          direction = 1;  // Start moving right
          moveInterval -= 5;
          Serial.println(moveInterval);
          for (int c = blockPos; c < blockPos + blockWidth; c++)
            setPoint(currentLevel, c, true);
        }
      }


    }
    }else{
      gameOver = true;


  }
}

  
  

