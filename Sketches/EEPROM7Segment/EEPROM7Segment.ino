//
// Creates the EEPROM for a 4-digt-7-segment-LED-display.
//
// A number is represented by its four digits, where leading
// zeroes are left out in decimal numbers.
//
// The EEPROM can hold more than one representation of a number.
// This code creates a representation for unsigned integer, signed
// integer (with leading sign) and hexadecimal display.
// Every representation occupies a total of 1k bytes in the
// EEPROM. The digits are stored from right to left, so the
// least significant digit comes first.
//
// Written 05.05.2020 by Ulf Caspers 
//

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

#define READ true     // for the setAddress procedure
#define WRITE false   // for the setAddress procedure

#define SIGN_DIGIT 16 // special "digit" for the sign
#define HEX_DIGIT 17  // special "digit" for the hex marker

//
// Segment numbers of the 7 segment display 
//
//     111
//    6   2
//    6   2
//     777
//    5   3
//    5   3
//     444 8
//
int segments[22][8] = {
  {1,2,3,4,5,6,0},   // 0
  {2,3,0},           // 1
  {1,2,4,5,7,0},     // 2
  {1,2,3,4,7,0},     // 3
  {2,3,6,7,0},       // 4
  {1,3,4,6,7,0},     // 5
  {1,3,4,5,6,7,0},   // 6
  {1,2,3,0},         // 7
  {1,2,3,4,5,6,7,0}, // 8
  {1,2,3,4,6,7,0},   // 9
  {1,2,3,5,6,7,0},   // A
  {3,4,5,6,7,0},     // b
  {1,4,5,6,0},       // C
  {2,3,4,5,7,0},     // d
  {1,4,5,6,7,0},     // E
  {1,5,6,7,0},       // F
  {7,0},             // -
  {3,5,6,7,0},       // h
  {3,5},             // 00b
  {2,3,5},           // 01b
  {3,5,6},           // 10b
  {2,3,5,6}          // 11b
};

#define BIN_OFFSET 18

byte digit[22];

void calculateDigitBytes() {
  char buf[20];
  for (int d = 0; d <= 21; d++) {
    int p = 0;
    int val = 0;
    while ( p <= 8 && segments[d][p] > 0 ) {
      int shift = segments[d][p] - 1;
      val |= ( 1 << shift);
      p++;
    }
    digit[d] = val;
    sprintf(buf, "Digit %d = %x", d, val);
    Serial.println(buf);
  }
}

void setAddress(int address, boolean outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);  
}

byte writeEEPROM(int address, byte odata) {
  setDataPinMode(OUTPUT);
  
  setAddress(address, WRITE);

  byte data = odata;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    digitalWrite(pin, data & 0x80);
    data = data << 1;
  }
  digitalWrite(WRITE_ENABLE, LOW);  
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE, HIGH);
  
  byte l = readEEPROM(address);
  int cnt = 1000;
  while ((cnt > 0) && (l != odata)) {
    cnt--;
    if (address == 0)
      l = readEEPROM(256);
    else 
      l = readEEPROM(0);
    l = readEEPROM(address);
  }

  if (cnt == 0){
    char buf[80];
    sprintf(buf, "\r\nWRITE ERROR on %04x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
  }
}

byte readLastByte() {
  setDataPinMode(INPUT);  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = data * 2 + digitalRead(pin);
  }
  return data;
}

byte readEEPROM(int address) {
  setDataPinMode(INPUT);  
  
  setAddress(address, READ);
  
  return readLastByte();
}

void setDataPinMode(int mode) {
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    pinMode(pin, mode);
  }  
}

void initPorts() {
  digitalWrite(WRITE_ENABLE, HIGH);  
  digitalWrite(SHIFT_LATCH, LOW);
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(WRITE_ENABLE, OUTPUT);
}

void printContents(int baseAddress) {
  for (int base = 0; base <= 255; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset++) {
      data[offset] = readEEPROM(baseAddress + base + offset);
    }

    char buf[80];
    sprintf(buf, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
      baseAddress + base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
      data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void setup() {
  // put your setup code code here, to run once:
  Serial.begin(57600);
  Serial.println("start generating 7 segment LED EEPROM");
  unsigned long startMillis = millis();
  
  initPorts();
  
  calculateDigitBytes();

  Serial.println("writing decimal unsigned 1st digit ");
  int baseAddress = 0;
  for (int i = 0; i <= 255; i++) {
    int d = i % 10;
    writeEEPROM(baseAddress + i, digit[d]);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal unsigned 2nd digit ");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit;
    if ((i/10) == 0)
      outDigit = 0;
    else  
      outDigit = digit[(i/10) % 10];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal unsigned 3rd digit ");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit;
    if ((i/100) == 0)
      outDigit = 0;
    else  
      outDigit = digit[(i/100) % 10];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal unsigned 4th digit (no digit)");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    writeEEPROM(baseAddress + i, 0);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal signed 1st digit ");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int z;
    if (i > 127)
      z = i-256;
    else
      z = i;
    int d = z % 10;
    if (d < 0)
      d *= -1;  
    int outDigit = digit[d];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal signed 2nd digit ");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int z;
    if (i > 127)
      z = i-256;
    else
      z = i;
    int outDigit;
    if (z < 0 && z > -10)
      outDigit = digit[SIGN_DIGIT];
    else if (z >= 0 && z < 10)
      outDigit = 0;
    else {
      int d = (z/10) % 10;  
      if (d < 0)
        d *= -1;
      outDigit = digit[d];
    }
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal signed 3rd digit ");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int z;
    if (i > 127)
      z = i-256;
    else
      z = i;
    int outDigit;
    if (z < -9 && z > -100)
      outDigit = digit[SIGN_DIGIT];
    else if (z >= -9 && z < 100)
      outDigit = 0;
    else {
      int d = (z/100) % 10;  
      if (d < 0)
        d *= -1;
      outDigit = digit[d];
    }
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing decimal signed 4th digit (maybe a sign)");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int z;
    if (i > 127)
      z = i-256;
    else
      z = i;
    int outDigit;
    if (z < -99 )
      outDigit = digit[SIGN_DIGIT];
    else 
      outDigit = 0;
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing hexadecimal 1st digit (hex marker)");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[HEX_DIGIT];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing hexadecimal 2nd digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[i % 16];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing hexadecimal 3rd digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[i / 16];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing hexadecimal 4th digit (no digit)");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = 0;
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing binary 1st digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[(i & 3) + BIN_OFFSET];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing binary 2nd digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[((i >> 2) & 3) + BIN_OFFSET];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing binary 3rd digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[((i >> 4) & 3) + BIN_OFFSET];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);
  
  Serial.println("writing binary 4th digit");
  baseAddress += 256;
  for (int i = 0; i <= 255; i++) {
    int outDigit = digit[((i >> 6) & 3) + BIN_OFFSET];
    writeEEPROM(baseAddress + i, outDigit);
  }
  printContents(baseAddress);

  int spendSeconds = (millis()-startMillis) / 1000;
  char buf[80];
  sprintf(buf, "took %d seconds", spendSeconds);
  Serial.println(buf);
  Serial.println("done.");
}

void loop() {
  // put your main code here, to run repeatedly:

}
