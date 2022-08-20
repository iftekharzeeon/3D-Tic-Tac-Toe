#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

byte level[4];

unsigned int red[4];
unsigned int blue[4];
unsigned int renderRed[4];
unsigned int renderBlue[4];

/* use this bools to inform motion */
bool moveX = false;
bool moveY = false;
bool moveZ = false;

/* game logic vars */
bool gameOver;
bool playerSelecting;
bool redTurn;

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
  level[0] = 13;
  level[1] = 12;
  level[2] = 11;
  level[3] = 10;
  for(int i = 0; i < 4; i++){
    pinMode(level[i], OUTPUT);
    digitalWrite(level[i],LOW);
  }
  for(int i = 0; i < 4; i++){
    red[i] = renderRed[i] = 0x6FF6;
    blue[i] = renderBlue[i] = 0x9009;
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
    mpu.update();
  /* 
  if(!gameOver){
    sensorChecking();
    movement();
    cursorRender();
    gameLogic();
  }else{
    winningRender();
  }
  */
//  renderCube();
    sensorChecking();
    delay(1000);
}

void sensorChecking(){
  

  /*Get the values*/
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
  
}

void gameLogic(){
  
}

void cursorRender(){
  static int blink = 0;

  for(int i = 0; i < 4; i++){
    renderRed[i] = red[i];
    renderBlue[i] = blue[i] ;
  }
  
  if(playerSelecting && blink > 100 && blink < 200){
    renderRed[cursorLevel] = red[cursorLevel] && cursorLocation;
    renderBlue[cursorLevel] = blue[cursorLevel] && cursorLocation;
  }else{
    renderRed[cursorLevel] = red[cursorLevel];
    renderBlue[cursorLevel] = blue[cursorLevel];
  }

  blink++;
  if(blink > 200){
    blink = 0;
  }
}

void movement(){
  if(!playerSelecting){
    //spawn cursor     0123012301230123
    cursorLocation = 0b1000000000000000;
    cursorLevel = 0;

    //check conflict
    while( (red[cursorLevel] && cursorLocation ) || (blue[cursorLevel] && cursorLocation) ){
      cursorLocation = cursorLocation >> 1;
      if(cursorLocation == 0){
        cursorLocation = 0b1000000000000000;
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
}

void renderCube()
{   
  for(int i = 0; i < 4; i++){
       digitalWrite(level[i], HIGH);

       unsigned int dummyBlue;
       unsigned int dummyRed;
       dummyBlue = ~renderBlue[i];
       dummyRed = ~renderRed[i];
       
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyBlue);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyBlue >> 8);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
        
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyRed);
       shiftOut(dataPin, clockPin, LSBFIRST, dummyRed >> 8);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
       digitalWrite(level[i], LOW);
     }
}
