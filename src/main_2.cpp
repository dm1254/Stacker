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

//======= I/O devices =======
const int RLED = 9;
const int GLED = 6;
const int BLED = 5;
const int SPEAKER = 3;
const int Button = 2;
const int resetButton = 4;
//======= Debounce/Millis variables =======
// Button
static unsigned long lastDebounceTime = 0;
static unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
const long interval = 200;
unsigned long lastButtonCheck = 0;
const long buttonCheckInterval = 20;
unsigned long lastPressTime = 0;
//Music
int currentNoteMain = 0;
int currentNoteWon = 0;
int currentNoteLose = 0;
bool notePlaying = false;
const int noteInterval = 400;
unsigned long lastNoteTime;
//LED
unsigned long previousLEDTime;
bool ledState = LOW;
const int blinkPeriod = 400;
//======= Game variables =======
bool gameOver = false; 
bool gameWon = false;
bool buttonPressed = false;
bool movingRight = true;  // Direction of block movement
int blockWidth = 3;       // Width of the moving block (max 8 columns)
int blockPos = 0;         //Initial block Position
int currentLevel = 15;    // Current row (15 = bottom, 0 = top)
int stack[16][8] = {0};   // Stack array (16 rows, 8 columns; 1 = LED on, 0 = off)
int direction = 1;        // Direction in which stack is moving
long moveInterval = 115;

//======= Music =======
//Notes Won Array
int notesWon[] = {
  NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_C5, NOTE_E5, NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4 };
//Size of notesWOn array
int noteCountWon = sizeof(notesWon)/sizeof(notesWon[0]);
//Notes Lost array
int notesLose[] = {
  NOTE_B4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B3 };
//Size of notesLose array
int noteCountLose = sizeof(notesLose)/sizeof(notesLose[0]);
// Main stage notes
int notesMain[] = {
      NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
      NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
      NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4,
      NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4,
      NOTE_C4, NOTE_E4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_G4,
      NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4,
      NOTE_F4, NOTE_F4, NOTE_E4, NOTE_D4 };
//Size of main notes array
int noteCountMain = sizeof(notesMain)/ sizeof(notesMain[0]); 


//======= Setup =======
void setup(){
  mx.begin();
  mx.clear();
  pinMode(RLED,OUTPUT);
  pinMode(GLED,OUTPUT);
  pinMode(BLED,OUTPUT);
  pinMode(SPEAKER,OUTPUT);
  pinMode(resetButton,INPUT);
  pinMode(Button,INPUT);
}


//======= Functions =======
void (* resetFunc) (void) = 0;

//David Moreno: Function to invert the Rows and Columns to desired output. Utlilizes the setPoint() function
//from the MD_MAX72XX library to change state
void setPoint(int row, int col, bool state) {
  // Native: 8 rows, 16 cols (0-7 first/bottom module, 8-15 second/top module)
  // Desired: 16 rows (0 top of second module, 15 bottom of first module), 8 cols (0 left, 7 right)
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

//Mohammad Salamah LED functionality
void blinkRed(){
  unsigned long currentTime = millis();
  if(currentTime - previousLEDTime >= blinkPeriod){
    previousLEDTime = currentTime;
    ledState = !ledState;

    digitalWrite(RLED, ledState ? HIGH:LOW); // Check to see if ledState is HIGH or LOW
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
    digitalWrite(BLED,ledState ? HIGH:LOW); //// Check to see if ledState is HIGH or LOW
  }
}

// David Moreno
void gameStateMusicAndLED(){
  // Current time in milliseconds 
  unsigned long currentTime = millis();
  // Game won condition
  if(gameWon == true){
    blinkBlue(); // Use blinkBlue function to flash blue light if won
    // Play winning notes if there are still notes remaining in array
    if(currentNoteWon < noteCountWon){
      //Start playing note if no notes are playing
      if(!notePlaying){
        tone(3,notesWon[currentNoteWon]); //Use tone to play note
        notePlaying = true; // Set notePlaying flag to true
        lastNoteTime = millis(); // Set lastNoteTime to millis to record when note started
        }
      // Check to see if note is finished playing
      if(currentTime - lastNoteTime >= noteInterval){
        noTone(SPEAKER); // Stop the current tone
        currentNoteWon++; // Increment to the next note in the array
        notePlaying = false; // Set notePlaying to false
      }
    }
  // Same polling function as the gameWon state
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
  // Default playing state
  }else{
    // Display a constatn red illumination while playing
    digitalWrite(RLED,HIGH);
    digitalWrite(GLED,LOW);
    digitalWrite(BLED,LOW);
    // Check to see if neither game over nor game won and play the music
    if(!gameOver || !gameWon){
      // Implement same polling function as game won and game over
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

// Victor Avendano 
void checkButton() {
  unsigned long currentTime = millis();
  // See if enough time has passes since last button check
  if (currentTime - lastButtonCheck >= buttonCheckInterval) {
    // Read current state of button 
    int buttonState = digitalRead(Button);
    // Check to see if button has been presses and its past debounce time
    if (buttonState == HIGH && currentTime - lastPressTime > debounceDelay) {
      buttonPressed = true; // Set buttonPressed state to true
      lastPressTime = currentTime; // Reset the the time of the last true button press
    }
    lastButtonCheck = currentTime; // Update time of the last button check
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

// ======= Main loop =======
void loop(){
  checkresetButton();
  gameStateMusicAndLED();
  // Keeps looping if game is not over
  if(!gameOver){
    unsigned long currentMillis = millis();
    // Sets the current block Position plus the width of the block(amount of block in stack) to off
    if(currentMillis - previousMillis >= moveInterval){
      for(int i= blockPos; i < blockPos + blockWidth; i++){
        setPoint(currentLevel,i,false);
      }
      blockPos += direction; // Increments the block position by direction(1 or -1)
      //Check bounds of the block position
      if(blockPos >= 8-blockWidth){
        blockPos = 8-blockWidth; //If the position of the block is greater than the amount of columns - the amount of block on stack 
        direction = -1; // Change direction and go left
      
      // If the positon is <= to 0 or the left most column 
      }else if(blockPos <= 0){
        blockPos = 0; // Set the block position to 0
        direction = 1; //Set direction to 1 to go right
      }
      // Loop overr the block position + the width of the stack and set the Leds on
      for(int i = blockPos; i<blockPos+blockWidth;i++){
        setPoint(currentLevel,i,true);
      }
      // A 2D array that represents the rows and columns in the matrix that are declared as 1 or true 
      for(int r=0;r<16;r++){
        for(int c=0;c<8;c++){
          if(stack[r][c]){ 
            setPoint(r,c,true); // Set all the rows and columns in the array if they are 1 to true or turn on for that positions LED
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
      delay(200); // Pause to show stacks position
      
      // Turn off the current block
      for (int i = blockPos; i < blockPos + blockWidth; i++)
        setPoint(currentLevel, i, false);

      // Check alignment with stack below
      if (currentLevel < 15) {
        // Find overlap with stack below
        int newBlockPos = blockPos;
        int newBlockWidth = 0;
        for (int i = blockPos; i < blockPos + blockWidth; i++) {
          // Checking the previous level and if each position in the row array is set to 1  
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
          moveInterval -= 5; // Decrease the interval speed by 5 each level 
          for (int c = blockPos; c < blockPos + blockWidth; c++)
            setPoint(currentLevel, c, true);
        }
      }
    }
    }else{
      gameOver = true;
  }
}

  
  
  
  



