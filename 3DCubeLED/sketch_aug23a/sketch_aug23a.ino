#define serData A0
#define outEn A1
#define latch A2
#define serClk A3

int R1[3] = {13, 12, 11};
int R2[3] = {10, 9, 8};
int writeReg[3] = {7, 6, 5};

#define regWrite 4
#define clk A5

int writeData[4] = {3, 2, 1, 0};

byte Data[8];
bool latchOutput = false;
byte reg;
bool writeFlag = false;

void setup() {
  pinMode(serData, OUTPUT);
  pinMode(outEn, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(serClk, OUTPUT);

  for(int i = 0; i < 3; i++){
    pinMode(R1[i], INPUT);
    pinMode(R2[i], INPUT);
    pinMode(writeReg[i], INPUT);
  }

  for(int i = 0; i < 4; i++){
    pinMode(writeData[i], INPUT);
  }

  pinMode(clk, INPUT);
  pinMode(regWrite, INPUT);

  for(int i = 0; i < 8; i++){
    Data[i] = 0;
  }

   latchOutput = false;

   writeFlag = false;
}


byte a, b, c, sendData;

void loop() {
  
  if(digitalRead(clk)){
    delay(50);

    a = 0; //select R1
    b = 0; //select R2
    c = 0; //select writeReg
    
    for(int i = 0; i < 3; i++){
      a = a | (digitalRead(R1[i]) << i);
    }
    for(int i = 0; i < 3; i++){
      b = b | (digitalRead(R2[i]) << i);
    }
    for(int i = 0; i < 3; i++){
      c = c | (digitalRead(writeReg[i]) << i);
    }

  }
  
  Data[0] = 0; // 0 reg to alway make it zero

  //read functionality
  if(digitalRead(clk) && (!latchOutput)){
    sendData = 0;
    
    Data[a] = 0x0F & Data[a];
    Data[b] = 0x0F & Data[b];
    sendData = sendData | Data[a] | (Data[b] << 4); // make 8 bit data and send it
    reg = Data[c];

    digitalWrite(outEn, HIGH);
    digitalWrite(latch, LOW);
    shiftOut(serData, serClk, LSBFIRST, sendData);
    digitalWrite(latch, HIGH);
    digitalWrite(outEn, LOW);
    latchOutput = true;
  }
  
  if(!digitalRead(clk)){
    latchOutput = false;
    digitalWrite(outEn, HIGH);
  }


  //write functionality
  if(digitalRead(clk) && digitalRead(regWrite) && (!(writeFlag))){
    writeFlag = true;
    
    reg = 0;

    for(int i = 0; i < 4; i++){
      reg = reg | (digitalRead(writeData[i]) << i);
    }
  }

  if((!digitalRead(clk)) && writeFlag){
    writeFlag = false;
    Data[c] = reg;
  }
  
}
