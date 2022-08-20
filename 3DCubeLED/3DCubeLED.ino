int latchPin = 5;  // Latch pin of 74HC595 is connected to Digital pin 5
int clockPin = 6; // Clock pin of 74HC595 is connected to Digital pin 6
int dataPin = 4;  // Data pin of 74HC595 is connected to Digital pin 4

byte level[4];

unsigned int red[4];
unsigned int blue[4]; // Variable to hold the pattern of which LEDs are currently turned on or off

/*
 * setup() - this function runs once when you turn your Arduino on
 * We initialize the serial connection with the computer
 */
void setup() 
{
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
    red[i] = 0x0000;
    blue[i] = 0x0000;
  }

  red[0] = 0xFF00;
  blue[0] = 0x00FF;

  red[1] = 0x0F0F;
  blue[1] = 0xF0F0;

  red[2] = 0xF0F0;
  blue[2] = 0x0F0F;
}

/*
 * loop() - this function runs over and over again
 */
void loop() 
{
  // Initially turns all the LEDs off, by giving the variable 'leds' the value 0
  updateShiftRegister();
  
  
}

/*
 * updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.
 */
void updateShiftRegister()
{   for(int i = 0; i < 4; i++){
       digitalWrite(level[i], HIGH);
  
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, blue[i] >> 8);
       shiftOut(dataPin, clockPin, LSBFIRST, blue[i]);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
        
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, red[i] >> 8);
       shiftOut(dataPin, clockPin, LSBFIRST, red[i]);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
       digitalWrite(level[i], LOW);
     }
}
