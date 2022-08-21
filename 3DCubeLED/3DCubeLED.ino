#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

int redPin = 7;
int bluePin = 2;

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;
int outputEn = 3;

int selectPin = 8;

byte level[4];

int loopCount = 0;

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
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
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
  loopCount++;
  static int movementBlink = 0;
  mpu.update();
  if(!gameOver){
    sensorChecking();
    if(movementBlink > 10){
      movement();
      movementBlink = 0;

      /*
       * serial check
       */
       Serial.println("red");
       for(int i = 0; i < 4; i++){
        Serial.println(red[i],HEX);
       }
       Serial.println("blue");
       for(int i = 0; i < 4; i++){
        Serial.println(blue[i],HEX);
       }
       Serial.println(F("=====================================================\n"));
       
    }
    cursorRender();
    gameLogic();
  }else{
    winningRender();
  }
  
//  rowByrow();
  renderCube();
  movementBlink++;
  moveX = false;
  moveY = false;
  moveZ = false;
}

void sensorChecking(){

  if (loopCount % 10 == 0) {
        xAxisAngle = mpu.getAccAngleX();

  if (xAxisAngle <= -18) {
    moveX = true;
    Serial.println("Going right"); 
    return;
  }
  yAxisAngle = mpu.getAccAngleY();
  if (yAxisAngle >= 18) {
    moveY = true;
    Serial.println("Going Up"); 
    return;
  }
  zAxisAngle = mpu.getAngleZ();
  if (zAxisAngle >= 18) {
    moveZ = true;
    Serial.println("Going forward"); 
    return;
  }
  
////  delay(10);  
//  yAxisAngle = mpu.getAccAngleY();
////  delay(10);
//  zAxisAngle = mpu.getAngleZ();

//  Serial.print(prevX);Serial.print("\t");Serial.println(xAxisAngle);
//  Serial.print(prevY);Serial.print("\t");Serial.println(yAxisAngle);
//  Serial.print(prevZ);Serial.print("\t");Serial.println(zAxisAngle);
//  Serial.println(F("=====================================================\n"));

//  if (xAxisAngle <= -18) {
//    moveX = true;
//    Serial.println("Going right"); 
//  } else if (yAxisAngle >= 18) {
//    moveY = true;
//    Serial.println("Going Up"); 
//  } else if (zAxisAngle >= 18) {
//    moveZ = true;
//    Serial.println("Going forward"); 
//  }

  /*Store the previous values*/
  prevX = xAxisAngle;
  prevY = yAxisAngle;
  prevZ = zAxisAngle;     
  }
 
  
}

void winningRender(){
  bool match;
  
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


bool checkRed(){
  unsigned int checker;
  unsigned int checkers[4];
  bool match;
  
  for(int i = 0; i < 4; i++){
    
    //col match
    checker = 0x8888;
    for(int j = 0; j < 4; j++){ 
      if(checker == (red[i] & checker)){
        winningRed[i] = checker;
        return true;
      }
      checker = checker >> 1;
    }

    //row match
    checker = 0xF000;
    for(int j = 0; j < 4; j++){ 
      if(checker == (red[i] & checker)){
        winningRed[i] = checker ;
        return true;
      }
      checker = checker >> 4;
    }

    //cross match
    checker = 0x8421;
    if( checker == (checker & red[i]) ){
        winningRed[i] = checker;
        return true;
    }

    checker = 0x1248;
    if( checker == (checker & red[i]) ){
        winningRed[i] = checker;
        return true;
    }
  }
  
  //vertical match
  unsigned int lev = red[0] & red[1] & red[2] & red[3];
  if(lev != 0x0000){
    Serial.println(lev);
    winningRed[0] = lev;
    winningRed[1] = lev;
    winningRed[2] = lev;
    winningRed[3] = lev;
    return true;
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
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      return true;
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
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      return true;
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
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      return true;
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
      winningRed[0] = checkers[0];
      winningRed[1] = checkers[1];
      winningRed[2] = checkers[2];
      winningRed[3] = checkers[3];
      return true;
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
    winningRed[0] = checkers[0];
    winningRed[1] = checkers[1];
    winningRed[2] = checkers[2];
    winningRed[3] = checkers[3];
    return true;
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
    winningRed[0] = checkers[0];
    winningRed[1] = checkers[1];
    winningRed[2] = checkers[2];
    winningRed[3] = checkers[3];
    return true;
  }

  return false;
}

bool checkBlue(){
  unsigned int checker;
  unsigned int checkers[4];
  bool match;

  for(int i = 0; i < 4; i++){
    
    //col match
    checker = 0x8888;
    for(int j = 0; j < 4; j++){ 
      if(checker == (blue[i] & checker)){
        
        winningBlue[i] = checker;
        return true;
      }
      checker = checker >> 1;
    }

    //row match
    checker = 0xF000;
    for(int j = 0; j < 4; j++){ 
      if(checker == (blue[i] & checker)){
        
        winningBlue[i] = checker;
        return true;
      }
      checker >> 4;
    }

    //cross match
    checker = 0x8421;
    if( checker == (checker & blue[i]) ){
        
        winningBlue[i] = checker;
        return true;
    }

    checker = 0x1248;
    if( checker == (checker & blue[i]) ){
        
        winningBlue[i] = checker;
        return true;
    }
  }
  
  //vertical match
  unsigned int lev = blue[0] & blue[1] & blue[2] & blue[3];
  if(lev != 0x0000){
    Serial.println(lev);
    winningBlue[0] = lev;
    winningBlue[1] = lev;
    winningBlue[2] = lev;
    winningBlue[3] = lev;
    return true;
  }
  
  //crosses in rows
  checkers[0] = 0x8000;
  checkers[1] = 0x0800;
  checkers[2] = 0x0080;
  checkers[3] = 0x0008;

  for(int i = 0; i < 4; i++){
    match = true;
    for(int j = 0; j < 4; j++){
      if(!(blue[j] & checkers[j])) match = false;
    }
    if(match){
      
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      return true;
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
      if(!(blue[j] & checkers[j])) match = false;
    }
    if(match){
      
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      return true;
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
      if(!(blue[j] & checkers[j])) match = false;
    }
    if(match){
      
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      return true;
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
      if(!(blue[j] & checkers[j])) match = false;
    }
    if(match){
      
      winningBlue[0] = checkers[0];
      winningBlue[1] = checkers[1];
      winningBlue[2] = checkers[2];
      winningBlue[3] = checkers[3];
      return true;
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
    if(!(blue[j] & checkers[j])) match = false;
  }
  if(match){
    
    winningBlue[0] = checkers[0];
    winningBlue[1] = checkers[1];
    winningBlue[2] = checkers[2];
    winningBlue[3] = checkers[3];
    return true;
  }

  //special cross
  checkers[0] = 0x0001;
  checkers[1] = 0x0020;
  checkers[2] = 0x0400;
  checkers[3] = 0x8000;

  match = true;
  for(int j = 0; j < 4; j++){
    if(!(blue[j] & checkers[j])) match = false;
  }
  if(match){
    
    winningBlue[0] = checkers[0];
    winningBlue[1] = checkers[1];
    winningBlue[2] = checkers[2];
    winningBlue[3] = checkers[3];
    return true;
  }

  return false;
}



void gameLogic(){
  for(int i = 0; i < 4; i++){
      winningRed[i] = 0x0000;
      winningBlue[i] = 0x0000;
  }

  bool redMatched = false;
  bool blueMatched = false;

  redMatched = checkRed();
  blueMatched = checkBlue();

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
  
  if(playerSelecting && blink > 100 && blink < 300){
    renderRed[cursorLevel] = red[cursorLevel] | cursorLocation;
    renderBlue[cursorLevel] = blue[cursorLevel] | cursorLocation;
  }

  blink += 50;
  if(blink > 300){
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
  }else if(moveZ){
    moveZ = false;
    
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
  }else if(moveY){
    moveY = false;
    
    cursorLevel = (cursorLevel + 1) % 4;
  }

  static bool firstClick = true;
  static unsigned int debounceDelay = 0;

  if(firstClick){
    if(digitalRead(selectPin)){
      if((red[cursorLevel] & cursorLocation ) || (blue[cursorLevel] & cursorLocation)){
        //conflict
      }else{
        if(redTurn){
          red[cursorLevel] = red[cursorLevel] | cursorLocation;
          redTurn = false;
          playerSelecting = false;
        }else{
          blue[cursorLevel] = blue[cursorLevel] | cursorLocation;
          redTurn = true;
          playerSelecting = false;
        }
      }
    }
    firstClick = false;
  }else{
    if(digitalRead(selectPin) && debounceDelay > 300){
      if((red[cursorLevel] & cursorLocation ) || (blue[cursorLevel] & cursorLocation)){
        //conflict
      }else{
        if(redTurn){
          red[cursorLevel] = red[cursorLevel] | cursorLocation;
          redTurn = false;
          playerSelecting = false;
        }else{
          blue[cursorLevel] = blue[cursorLevel] | cursorLocation;
          redTurn = true;
          playerSelecting = false;
        }
      }
    }
    debounceDelay++;
  }
}

void renderCube()
{  
  if(!gameOver){
    if(redTurn){
      digitalWrite(redPin, HIGH);
      digitalWrite(bluePin, LOW);
    }else{
      digitalWrite(bluePin, HIGH);
      digitalWrite(redPin, LOW);
    }
  }
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
  static unsigned int j = 0x1;
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
    j = 0x1;
    l++;
    
    if(l >= 4){
      l = 0;
      redGlow = !redGlow;
      
    }
  }
  else j = j << 1;
}
