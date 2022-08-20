#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;
int outputEn = 3;

int selectPin = 8;

byte level[4];

unsigned int red[4];
unsigned int blue[4];
unsigned int renderRed[4];
unsigned int renderBlue[4];
unsigned int winningRed[4];
unsigned int winningBlue[4];

/* use this bools to inform motion */
bool moveX = false;
bool moveY = false;
bool moveZ = false;

/* game logic vars */
bool gameOver;
bool playerSelecting;
bool redTurn;
bool redWon;
bool blueWon;

/* movement vars */
unsigned int cursorLocation;
int cursorLevel;

/* Gyro Variables */
float xAxisAngle, yAxisAngle, zAxisAngle;
float prevX = 0;
float prevY = 0;
float prevZ = 0;


void setup() 
{
  Serial.begin(9600);
  
  // Set all the pins of 74HC595 as OUTPUT
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(outputEn, OUTPUT);
  pinMode(selectPin, INPUT);
  level[0] = 13;
  level[1] = 12;
  level[2] = 11;
  level[3] = 10;
  for(int i = 0; i < 4; i++){
    pinMode(level[i], OUTPUT);
    digitalWrite(level[i],LOW);
  }
  for(int i = 0; i < 4; i++){
    red[i] = renderRed[i] = winningRed[i] = 0x0000;
    blue[i] = renderBlue[i] = winningBlue[i] = 0x0000;
  }

  red[0] = 0xF00F;
  red[1] = 0x0FF0;
  red[2] = 0x0FF0;
  red[3] = 0xF00F;

  blue[0] = 0xF00F;
  blue[1] = 0x0FF0;
  blue[2] = 0x0FF0;
  blue[3] = 0xF00F;
  
  
  gameOver = false;
  playerSelecting = false;
  redTurn = true;

  //Calibrate the Gyroscope
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){
    Serial.println("Gyro could not be connected!\n");  
  } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");
}

void loop() 
{
  if(!gameOver){
    mpu.update();
    sensorChecking();
    movement();
    cursorRender();
    //gameLogic();
  }else{
    winningRender();
  }
  
  //rowByrow();
  renderCube();
}

void sensorChecking(){

  xAxisAngle = mpu.getAccAngleX();
//  delay(10);  
  yAxisAngle = mpu.getAccAngleY();
//  delay(10);
  zAxisAngle = mpu.getAngleZ();

  Serial.print(prevX);Serial.print("\t");Serial.println(mpu.getAccAngleX());
  Serial.print(prevY);Serial.print("\t");Serial.println(mpu.getAccAngleY());
  Serial.print(prevZ);Serial.print("\t");Serial.println(mpu.getAngleZ());
  Serial.println(F("=====================================================\n"));

  if (xAxisAngle <= -18) {
    moveX = true;
    Serial.println("Going right"); 
  } else if (yAxisAngle >= 18) {
    moveY = true;
    Serial.println("Going Up"); 
  } else if (zAxisAngle >= 18) {
    moveZ = true;
    Serial.println("Going forward"); 
  }

  /*Store the previous values*/
  prevX = xAxisAngle;
  prevY = yAxisAngle;
  prevZ = zAxisAngle;  
  
}

void winningRender(){
  if(redWon){
    for(int i = 0; i < 4; i++){
      renderRed[i] = 0xFFFF;
      renderBlue[i] = 0x0000;
    }
  }if(blueWon){
    for(int i = 0; i < 4; i++){
      renderBlue[i] = 0xFFFF;
      renderRed[i] = 0x0000;
    }
  }

  static int winBlink;
  if(winBlink > 100 && winBlink < 200){
    for(int i = 0; i< 4; i++){
      renderRed[i] = renderRed[i] | winningBlue[i];
      renderBlue[i] = renderBlue[i] | winningRed[i];
    }
  }

  winBlink += 4;
  if(winBlink > 200){
    winBlink = 0;
  }
  
}

void gameLogic(){
  for(int i = 0; i < 4; i++){
      winningRed[i] = 0x0000;
      winningBlue[i] = 0x0000;
  }

  bool match;
  bool redMatched = false;
  bool blueMatched = false;
  unsigned int checker;
  unsigned int checkers[4];

  
  //red
  do{
    //level match
    for(int i = 0; i < 4; i++){
      
      //col match
      checker = 0x8888;
      for(int j = 0; j < 4; j++){ 
        if(checker >> j == red[i] & checker >> j){
          redMatched = true;
          winningRed[i] = checker >> j;
          break;
        }
      }

      //row match
      checker = 0xF000;
      for(int j = 0; j < 4; j++){ 
        if(checker >> j*4 == red[i] & checker >> j*4){
          redMatched = true;
          winningRed[i] = checker >> j*4;
          break;
        }
      }

      //cross match
      checker = 0x8421;
      if( checker == checker & red[i] ){
          redMatched = true;
          winningRed[i] = checker;
          break;
      }

      checker = 0x1248;
      if( checker == checker & red[i] ){
          redMatched = true;
          winningRed[i] = checker;
          break;
      }
    }
    
    //vertical match
    unsigned int lev = red[0] & red[1] & red[2] & red[3];
    if(lev){
      for(int i = 0; i++; i < 16){
        if(lev & 1 << i){
          redMatched = true;
          winningRed[0] = 1 << i;
          winningRed[1] = 1 << i;
          winningRed[2] = 1 << i;
          winningRed[3] = 1 << i;
          break;
        }
      }
    }
    
    //crosses in rows
    checkers[0] = 0x8000;
    checkers[1] = 0x0800;
    checkers[2] = 0x0080;
    checkers[3] = 0x0008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        redMatched = true;
        winningRed[0] = checkers[0];
        winningRed[1] = checkers[1];
        winningRed[2] = checkers[2];
        winningRed[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 1;
      }
    }

    checkers[0] = 0x0008;
    checkers[1] = 0x0080;
    checkers[2] = 0x0800;
    checkers[3] = 0x8000;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        redMatched = true;
        winningRed[0] = checkers[0];
        winningRed[1] = checkers[1];
        winningRed[2] = checkers[2];
        winningRed[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 1;
      }
    }

    //crosses in cols
    checkers[0] = 0x8000;
    checkers[1] = 0x4000;
    checkers[2] = 0x2000;
    checkers[3] = 0x1008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        redMatched = true;
        winningRed[0] = checkers[0];
        winningRed[1] = checkers[1];
        winningRed[2] = checkers[2];
        winningRed[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 4;
      }
    }

    checkers[0] = 0x1000;
    checkers[1] = 0x2000;
    checkers[2] = 0x4000;
    checkers[3] = 0x8008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        redMatched = true;
        winningRed[0] = checkers[0];
        winningRed[1] = checkers[1];
        winningRed[2] = checkers[2];
        winningRed[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 4;
      }
    }

    //special cross
    checkers[0] = 0x8000;
    checkers[1] = 0x0400;
    checkers[2] = 0x0020;
    checkers[3] = 0x0001;

    match = true;
    for(int j = 0; j < 4; j++){
      if(!(red[j] & checkers[j])) match = false;
    }
    if(match){
      redMatched = true;
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      break;
    }

    //special cross
    checkers[0] = 0x0001;
    checkers[1] = 0x0020;
    checkers[2] = 0x0400;
    checkers[3] = 0x8000;

    match = true;
    for(int j = 0; j < 4; j++){
      if(!(red[j] & checkers[j])) match = false;
    }
    if(match){
      redMatched = true;
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      break;
    }
  }while(false);




  // blue
  do{
    //level match
    for(int i = 0; i < 4; i++){
      
      //col match
      checker = 0x8888;
      for(int j = 0; j < 4; j++){ 
        if(checker >> j == red[i] & checker >> j){
          blueMatched = true;
          winningBlue[i] = checker >> j;
          break;
        }
      }

      //row match
      checker = 0xF000;
      for(int j = 0; j < 4; j++){ 
        if(checker >> j*4 == red[i] & checker >> j*4){
          blueMatched = true;
          winningBlue[i] = checker >> j*4;
          break;
        }
      }

      //cross match
      checker = 0x8421;
      if( checker == checker & red[i] ){
          blueMatched = true;
          winningBlue[i] = checker;
          break;
      }

      checker = 0x1248;
      if( checker == checker & red[i] ){
          blueMatched = true;
          winningBlue[i] = checker;
          break;
      }
    }
    
    //vertical match
    unsigned int lev = red[0] & red[1] & red[2] & red[3];
    if(lev){
      for(int i = 0; i++; i < 16){
        if(lev & 1 << i){
          blueMatched = true;
          winningBlue[0] = 1 << i;
          winningBlue[1] = 1 << i;
          winningBlue[2] = 1 << i;
          winningBlue[3] = 1 << i;
          break;
        }
      }
    }
    
    //crosses in rows
    checkers[0] = 0x8000;
    checkers[1] = 0x0800;
    checkers[2] = 0x0080;
    checkers[3] = 0x0008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        blueMatched = true;
        winningBlue[0] = checkers[0];
        winningBlue[1] = checkers[1];
        winningBlue[2] = checkers[2];
        winningBlue[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 1;
      }
    }

    checkers[0] = 0x0008;
    checkers[1] = 0x0080;
    checkers[2] = 0x0800;
    checkers[3] = 0x8000;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        blueMatched = true;
        winningBlue[0] = checkers[0];
        winningBlue[1] = checkers[1];
        winningBlue[2] = checkers[2];
        winningBlue[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 1;
      }
    }

    //crosses in cols
    checkers[0] = 0x8000;
    checkers[1] = 0x4000;
    checkers[2] = 0x2000;
    checkers[3] = 0x1008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        blueMatched = true;
        winningBlue[0] = checkers[0];
        winningBlue[1] = checkers[1];
        winningBlue[2] = checkers[2];
        winningBlue[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 4;
      }
    }

    checkers[0] = 0x1000;
    checkers[1] = 0x2000;
    checkers[2] = 0x4000;
    checkers[3] = 0x8008;

    for(int i = 0; i < 4; i++){
      match = true;
      for(int j = 0; j < 4; j++){
        if(!(red[j] & checkers[j])) match = false;
      }
      if(match){
        blueMatched = true;
        winningBlue[0] = checkers[0];
        winningBlue[1] = checkers[1];
        winningBlue[2] = checkers[2];
        winningBlue[3] = checkers[3];
        break;
      }
      for(int j = 0; j < 4; j++){
        checkers[j] = checkers[j] >> 4;
      }
    }

    //special cross
    checkers[0] = 0x8000;
    checkers[1] = 0x0400;
    checkers[2] = 0x0020;
    checkers[3] = 0x0001;

    match = true;
    for(int j = 0; j < 4; j++){
      if(!(red[j] & checkers[j])) match = false;
    }
    if(match){
      blueMatched = true;
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      break;
    }

    //special cross
    checkers[0] = 0x0001;
    checkers[1] = 0x0020;
    checkers[2] = 0x0400;
    checkers[3] = 0x8000;

    match = true;
    for(int j = 0; j < 4; j++){
      if(!(red[j] & checkers[j])) match = false;
    }
    if(match){
      blueMatched = true;
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      break;
    }
  }while(false);

  if(redMatched){
    gameOver = true;
    redWon = true;
    blueWon = false;
  }else if(blueMatched){
    gameOver = true;
    blueWon = true;
    redWon = false;
  }
  
}

void cursorRender(){
  static int blink = 0;

  for(int i = 0; i < 4; i++){
    renderRed[i] = red[i];
    renderBlue[i] = blue[i] ;
  }
  
  if(playerSelecting && blink > 100 && blink < 200){
    renderRed[cursorLevel] = red[cursorLevel] | cursorLocation;
    renderBlue[cursorLevel] = blue[cursorLevel] | cursorLocation;
  }

  blink += 4;
  if(blink > 200){
    blink = 0;
  }
}

void movement(){
  if(!playerSelecting){
    cursorLocation = 0x8000;
    cursorLevel = 0;

    //check conflict
    while( (red[cursorLevel] & cursorLocation ) || (blue[cursorLevel] & cursorLocation) ){
      cursorLocation = cursorLocation >> 1;
      if(cursorLocation == 0){
        cursorLocation = 0x8000;
        cursorLevel++;
      }
    }

    playerSelecting = true;
  }

  if(moveX){
    moveX = false;
    cursorLocation = cursorLocation >> 1;
    
    if(cursorLocation == 0x0800){
      cursorLocation = 0x8000;
    }else if(cursorLocation == 0x0080){
      cursorLocation = 0x0800;
    }else if(cursorLocation == 0x0008){
      cursorLocation = 0x0080;
    }else if(cursorLocation == 0x0000){
      cursorLocation = 0x0008;
    }
  }else if(moveY){
    moveY = false;
    
    if(cursorLocation == 0x0008)
      cursorLocation = 0x8000;
    else if(cursorLocation == 0x0004)
      cursorLocation = 0x4000;
    else if(cursorLocation == 0x0002)
      cursorLocation = 0x2000;
    else if(cursorLocation == 0x0001)
      cursorLocation = 0x1000;
    else
      cursorLocation = cursorLocation >> 4;
  }else if(moveZ){
    moveZ = false;
    
    cursorLevel = (cursorLevel + 1) % 4;
  }

  static int debounceDelay = 0;
  if(digitalRead(selectbit) && debounceDelay > 50){
    debounceDelay = 0;
    if((red[cursorLevel] & cursorLocation ) || (blue[cursorLevel] & cursorLocation)){
      //conflict
    }else{
      if(redTurn){
        red[cursorLevel] = red[cursorLevel] | cursorLocation;
        redTurn = !redTurn;
        playerSelecting = false;
      }else{
        blue[cursorLevel] = blue[cursorLevel] | cursorLocation;
        redTurn = !redTurn;
        playerSelecting = false;
      }
    }
  }
  debounceDelay++;
}

void renderCube()
{   
  for(int i = 0; i < 4; i++){

       unsigned int dummyBlue;
       unsigned int dummyRed;
       dummyBlue = ~renderBlue[i];
       dummyRed = ~renderRed[i];

       digitalWrite(outputEn, HIGH);
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyBlue);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyBlue >> 8);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       digitalWrite(latchPin, HIGH);
       digitalWrite(outputEn, LOW);

       digitalWrite(level[i], HIGH);
       delay(1);
       digitalWrite(level[i], LOW);
       
       digitalWrite(outputEn, HIGH);
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyRed);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyRed >> 8);
       digitalWrite(latchPin, HIGH);
       digitalWrite(outputEn, LOW);

       digitalWrite(level[i], HIGH);
       delay(1);
       digitalWrite(level[i], LOW);
     }
}

void rowByrow(){
  static bool redGlow = true;
  for(int i = 0; i < 4; i++){
    red[i] = renderRed[i] = winningRed[i] = 0x0000;
    blue[i] = renderBlue[i] = winningBlue[i] = 0x0000;
  }
  static unsigned int j = 0xF;
  static unsigned int l = 0;

  if(redGlow){
    renderRed[l] = j;
    renderBlue[l] = ~j;
  }
  else{
    renderBlue[l] = j;
    renderRed[l] = ~j;
  }

  if(j == 0xF000) {
    j = 0xF;
    l++;
    
    if(l >= 4){
      l = 0;
      redGlow = !redGlow;
      
    }
  }
  else j = j << 4;
}
