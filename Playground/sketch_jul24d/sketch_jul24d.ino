// C++ code
//
#define LEDDATA 13
#define LEDEN 12
#define LEDCLK 11

unsigned int arrayRed[4] = {0x00F0, 0x0F00, 0xF000, 0x000F};
unsigned int level[4] = {0xFFFF, 0x0000, 0xFFFF, 0x0000};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  
}

void loop()
{
  digitalWrite(12, LOW);
  delay(10);
  for(int i = 0; i < 4; i++){
    unsigned value = level[i];
    Serial.print(value);
    for(int j = 0; j < 8; j++){
        digitalWrite(13, !((value >> j) & 0x01));
        
        delay(100);
        digitalWrite(11, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(11, LOW);
        digitalWrite(LED_BUILTIN, LOW);
    }
    value = arrayRed[i];
    for(int j = 0; j < 16; j++){
        digitalWrite(13, !((value >> j) & 0x01));
        delay(100);
        digitalWrite(11, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(11, LOW);
        digitalWrite(LED_BUILTIN, LOW);
    }
    delay(100);
//        digitalWrite(LEDCLK, HIGH);
//        delay(5);
//        digitalWrite(LEDCLK, LOW);
  }
   // Delay a little bit to improve simulation performance
}
