//
// Test for EEPROM-Programmer 4MBit
//


#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

#define READ true     // for the setAddress4M procedure
#define WRITE false   // for the setAddress4M procedure

#define CHIP_ENABLE true
#define CHIP_DISABLE false


void setAddress4M(long address, boolean outputEnable, boolean chipEnable) {
  digitalWrite(SHIFT_LATCH, LOW);
  
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 16) | (outputEnable ? 0 : 0x80) | (chipEnable ? 0 : 0x40));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  
  digitalWrite(SHIFT_LATCH, HIGH);
}

byte writeEEPROM4MByte(long address, byte odata) {
  
  setDataPinMode(OUTPUT);
    
  setAddress4M(address, WRITE, CHIP_ENABLE);

  digitalWrite(WRITE_ENABLE, LOW);  
  byte data = odata;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    digitalWrite(pin, data & 0x80);
    data = data << 1;
  }
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE, HIGH);
}

byte writeEEPROM4MData(long address, byte odata) {

  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(0x5555, 0xA0);
  writeEEPROM4MByte(address, odata);
  
  byte l = readEEPROM4M(address);
  int cnt = 1000;
  while ((cnt > 0) && (l != odata)) {
    cnt--;
    setAddress4M(address, WRITE, CHIP_DISABLE);
    l = readEEPROM4M(address);
  }

  if (cnt == 0){
    char buf[80];
    sprintf(buf, "\r\nWRITE ERROR on %06x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
  }
}

byte readEEPROM4M(long address) {
  setDataPinMode(INPUT);  
  
  setAddress4M(address, READ, CHIP_ENABLE);

  delayMicroseconds(1);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = data * 2 + digitalRead(pin);
  }
  return data;
}

void eraseSector4M(long baseAddress) {

  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(0x5555, 0x80);
  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(baseAddress, 0x30);
  
  byte l = readEEPROM4M(baseAddress);
  unsigned long expectedEnd = millis() + 100;
  while ((l != 0xff) && millis() < expectedEnd) {
    setAddress4M(baseAddress, WRITE, CHIP_DISABLE);
    l = readEEPROM4M(baseAddress);
  }

  if (l != 0xff) {
    char buf[80];
    sprintf(buf, "\r\nSector Erase ERROR on %06x!", baseAddress);
    Serial.println(buf);
  }  
}

void eraseChip4M() {

  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(0x5555, 0x80);
  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(0x5555, 0x10);
  
  byte l = readEEPROM4M(0);
  unsigned long expectedEnd = millis() + 250;
  while ((l != 0xff) && millis() < expectedEnd) {
    setAddress4M(0, WRITE, CHIP_DISABLE);
    l = readEEPROM4M(0);
  }

  if (l != 0xff) {
    Serial.println("\r\nChip Erase ERROR!\r\n");
  }  
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

void printContents4M(long baseAddress) {
  for (long base = 0; base <= 255; base += 16) {
    byte data[16];
    for (long offset = 0; offset <= 15; offset++) {
      data[offset] = readEEPROM4M(baseAddress + base + offset);
    }

    char buf[80];
    sprintf(buf, "%06x: %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
      baseAddress + base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
      data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

int getChipID() {
  writeEEPROM4MByte(0x5555, 0xAA);
  writeEEPROM4MByte(0x2AAA, 0x55);
  writeEEPROM4MByte(0x5555, 0x90);
  setAddress4M(0x5555L, WRITE, CHIP_DISABLE);
  delayMicroseconds(20);
  byte b1 = readEEPROM4M(0);
  byte b2 = readEEPROM4M(1);
  writeEEPROM4MByte(0x5555, 0xF0);
  setAddress4M(0x5555L, WRITE, CHIP_DISABLE);
  delayMicroseconds(1);
  setAddress4M(0x5555L, READ, CHIP_ENABLE);
  return b1 + 256 * b2;  
}

void setup() {
  // put your setup code code here, to run once:
  Serial.begin(57600);
  Serial.println("start");
  unsigned long startMillis = millis();
  
  initPorts();

  int chipID = getChipID();
  
  char buf[80];
  sprintf(buf, "ID: %04x", chipID);
  Serial.println(buf);
  
  //writeEEPROMData(3, 0xCF);

  printContents4M(0);
  eraseSector4M(0);
  printContents4M(0);

  int spendSeconds = (millis()-startMillis) / 1000;
  sprintf(buf, "took %d seconds", spendSeconds);
  Serial.println(buf);
  Serial.println("done.");
}

void loop() {
  // put your main code here, to run repeatedly:
}
