int latchPin = 5;  // Latch pin of 74HC595 is connected to Digital pin 5
int clockPin = 6; // Clock pin of 74HC595 is connected to Digital pin 6
int dataPin = 4;  // Data pin of 74HC595 is connected to Digital pin 4

byte level[4];

byte row12 = 0;
byte row34 = 0; // Variable to hold the pattern of which LEDs are currently turned on or off

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
}

/*
 * loop() - this function runs over and over again
 */
void loop() 
{
  row12 = 0b00000000;
  row34 = 0b00000000;// Initially turns all the LEDs off, by giving the variable 'leds' the value 0
  updateShiftRegister();
  
  
}

/*
 * updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.
 */
void updateShiftRegister()
{   for(int i = 0; i < 4; i++){
       digitalWrite(level[i], HIGH);
  
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, row34);
       shiftOut(dataPin, clockPin, LSBFIRST, row12);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
        
       digitalWrite(latchPin, LOW);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, 0xFF);
       shiftOut(dataPin, clockPin, LSBFIRST, row34);
       shiftOut(dataPin, clockPin, LSBFIRST, row12);
       digitalWrite(latchPin, HIGH);
    
       delay(1);
       digitalWrite(level[i], LOW);
     }
}