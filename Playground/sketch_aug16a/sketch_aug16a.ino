#define led_data 13
#define led_clk 12
#define led_en 11
#define led_select 10

unsigned int arrayRed[4] = {0xFF, 0xFF, 0xFF, 0xFF};
unsigned int arrayGreen[4] = {0xFFFF, 0x00, 0x00, 0x00};
unsigned int arrayBlue[4] = {0xFFFF, 0x00, 0x00, 0x00};
unsigned int arrayLevel[4] = {0xFFFF, 0x002F, 0x004F, 0x008F};

unsigned inverseData(unsigned data){
  return 65536 - data;
}

void renderCube(){
  unsigned value;
  for(int level = 0; level < 4; level++){
    digitalWrite(led_en, LOW);
    value = arrayLevel[level];
    shiftOut(led_data, led_clk, LSBFIRST, value);
    value = arrayRed[level];
    shiftOut(led_data, led_clk, LSBFIRST, value);
    shiftOut(led_data, led_clk, LSBFIRST, value >> 8);
    value = arrayGreen[level];
    shiftOut(led_data, led_clk, LSBFIRST, value);
    shiftOut(led_data, led_clk, LSBFIRST, value >> 8);
    value = arrayBlue[level];
    shiftOut(led_data, led_clk, LSBFIRST, value);
    shiftOut(led_data, led_clk, LSBFIRST, value >> 8);
    digitalWrite(led_en, HIGH);
    delay(1);
  }
}

//debug
void debugReg(){
  digitalWrite(led_select, HIGH);
  digitalWrite(led_en, HIGH);
  for(unsigned i = 0; i < 256; i++){
    digitalWrite(led_en, HIGH);
    unsigned value = inverseData(i);
    shiftOut(led_data, led_clk, LSBFIRST, value);
    digitalWrite(led_en, LOW);
    delay(1000);
  }
  digitalWrite(led_select,LOW);
}


void setup()
{
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
}

void loop()
{
  renderCube();
}
