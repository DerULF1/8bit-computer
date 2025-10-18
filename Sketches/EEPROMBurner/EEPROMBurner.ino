//
// Writes EEPROMs operated from PC.
//
// Written 18.046.2021 by Ulf Caspers
//

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

#define READ true     // for the setAddress procedure
#define WRITE false   // for the setAddress procedure

#define CHIP_ENABLE true
#define CHIP_DISABLE false

int CHIP_TYPE = 1;
int writeErrors = 0;

//
// Sets the 19 bit value of the address lines on the chip.
//
// address      - value of the address lines on the chip
// outputEnable - READ/WRITE value of the OE line on the chip
// chipEnable   - CHIP_ENABLE/CHIP_DISABLE value of the CE line on the chip
//
void setAddress4M(unsigned long address, boolean outputEnable, boolean chipEnable) {
  digitalWrite(SHIFT_LATCH, LOW);

  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 16) | (outputEnable ? 0 : 0x80) | (chipEnable ? 0 : 0x40));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, HIGH);
}

//
// Send a byte to the 4M bit chip.
//
// address - value of the address lines to send the byte to
// odata   - value of the byte to send
//
void writeEEPROM4MByte(unsigned long address, byte odata) {

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

//
// Store a byte onto the 4M bit chip.
//
// address - address of the byte to store
// odata   - value of the byte to store
//
void writeEEPROM4MData(unsigned long address, byte odata) {

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

  if (cnt == 0) {
    char buf[60];
    sprintf(buf, "\r\nWRITE ERROR on %06x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
    writeErrors++;
  }
}

//
// Read a byte from the 4M bit chip.
//
// address - address of the byte to read
//
// returns: byte from the specified address
//
byte readEEPROM4M(unsigned long address) {
  setDataPinMode(INPUT);

  setAddress4M(address, READ, CHIP_ENABLE);

  delayMicroseconds(1);
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = data * 2 + digitalRead(pin);
  }
  return data;
}

//
// Erase a 4k sector on the 4M bit chip.
//
// baseAddress - number of the sector to erase
//
void eraseSector4M(unsigned long baseAddress) {

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
    char buf[40];
    sprintf(buf, "\r\nSector Erase ERROR on %06x!", baseAddress);
    Serial.println(buf);
  }
}

//
// Erase all data on the 4M bit chip.
//
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
    Serial.println(F("\r\nChip Erase ERROR!\r\n"));
  }
}

//
// Read the ID of the 4M bit chip.
//
// returns: 16 bit chip ID
//
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

//
// Sets the 14 bit value of the address lines on the chip.
//
// address      - value of the address lines on the chip
// outputEnable - READ/WRITE value of the OE line on the chip
// chipEnable   - CHIP_ENABLE/CHIP_DISABLE value of the CE line on the chip
//
void setAddress(int address, boolean outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

//
// Store a byte onto the 8k byte chip.
//
// address - address of the byte to store
// odata   - value of the byte to store
//
void writeEEPROM(int address, byte odata) {
  
  byte old = readEEPROM(address);
  if (old != odata) {
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

    /*  delay(15);*/
    byte l = readEEPROM(address);
    int cnt = 1000;
    while ((cnt > 0) && (l != odata)) {
      cnt--;
      if ((address & 0xff) == 0)
        l = readEEPROM(address + 1);
      else
        l = readEEPROM(address - 1);
      l = readEEPROM(address);
    }
    if (l != odata) {
      char buf[60];
      sprintf(buf, "\r\nWRITE ERROR on %04x! (%02x %02x)\r\n", address, odata, l);
      Serial.println(buf);
      writeErrors++;
    }
  }
}

//
// Read a byte from 8k byte chip.
//
// address - address of the byte to read
//
// returns: byte from the specified address
//
byte readEEPROM(int address) {
  setDataPinMode(INPUT);

  setAddress(address, READ);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = data * 2 + digitalRead(pin);
  }
  return data;
}

//
// Configure the data pins on the Arduino.
//
// mode - READ/WRITE
//
void setDataPinMode(int mode) {
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    pinMode(pin, mode);
  }
}

//
// Configure the communication pins of the arduino.
//
void initPorts() {
  digitalWrite(WRITE_ENABLE, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(WRITE_ENABLE, OUTPUT);
}

void setup() {
  // put your setup code code here, to run once:
  Serial.begin(57600);
  Serial.setTimeout(10*1000);
  
  initPorts();
  
  char buf[60];

  unsigned long startMillis = millis();

  
  if (writeErrors > 0) {
    sprintf(buf, "\r\nERRORs: %d\r\n", writeErrors);
    Serial.println(buf);
  }
 
 Serial.println(F("Init done."));
}

char getSelectionCharacter() {
  while (Serial.available() == 0)
  {
  }
  int c = Serial.read();
  int b = 0;
  while (b != 10)
  {
    b = Serial.read();
  }
  return (char) c;
}

String getLine() {
  while (Serial.available() == 0)
  {
  }
  String s = Serial.readStringUntil((char) 10);
  return s;
}

void selectChipType() {
  Serial.println(F("Select Chip type"));
  Serial.println(CHIP_TYPE);
  Serial.println(F("1) AT28Cxxx    - 28 PDIP"));
  Serial.println(F("2) SST39SFXXXA - 32 PDIP"));
  Serial.println(">");
    
  char b = getSelectionCharacter();
    
  if (b=='1') {
    CHIP_TYPE = 1;
  }
  if (b=='2') {
    CHIP_TYPE = 2;
  }
}

void readChip() {
  Serial.println(F("Read Chip"));
  Serial.println(CHIP_TYPE);
  Serial.println(F("startAddress byteCount"));
  Serial.println(">");
  String s = getLine();
  if (s.length() > 0) {
    int p = s.indexOf(' ');
    if (p>0) {
      boolean binMode = false;
      String startstr = s.substring(0, p);
      String rest = s.substring(p+1);
      int p = rest.indexOf(' ');
      String bytestr;
      if (p>0) {
        bytestr = rest.substring(0, p);
        rest = rest.substring(p+1);
        binMode = (rest.charAt(0) == 'b');        
      } else {
        bytestr =  rest;
      }
      long adr = startstr.toInt();
      long byteCnt = bytestr.toInt();
      while (byteCnt-- > 0) {
        byte b;
        if (CHIP_TYPE == 1)
          b = readEEPROM(adr++);
        else
          b = readEEPROM4M(adr++);
          
        if (binMode)
          Serial.write(b);
        else {
          String str = String(b, HEX);
          if (str.length() < 2)
            str = "0"+str;
          Serial.print(str);
        }
      }
    }
  }
}

void writeChip() {
  Serial.println(F("Write Chip"));
  Serial.println(CHIP_TYPE);
  Serial.println(F("startAddress byteCount"));
  Serial.println(">");
  String s = getLine();
  if (s.length() > 0) {
    int p = s.indexOf(' ');
    if (p>0) {
      boolean binMode = false;
      String startstr = s.substring(0, p);
      String rest = s.substring(p+1);
      int p = rest.indexOf(' ');
      String bytestr;
      if (p>0) {
        bytestr = rest.substring(0, p);
        rest = rest.substring(p+1);
        binMode = (rest.charAt(0) == 'b');        
      } else {
        bytestr =  rest;
      }
      long adr = startstr.toInt();
      long byteCnt = bytestr.toInt();
      Serial.print(F("Write "));
      Serial.print(byteCnt);
      Serial.print(F(" Bytes from "));
      Serial.println(adr);
      writeErrors = 0;
      char buf[] = {' ', ' ', '\0'};
      int cnt = 0;
      while (cnt++ < byteCnt) {
        int b;
          
        if (binMode)
          do {
             b = Serial.read();
          } while (b < 0);
        else {
          Serial.readBytes(buf, 2);
          Serial.println(buf);
          b = (int) strtol(buf, 0, 16);
        }

        if (CHIP_TYPE == 1)
          writeEEPROM(adr++, b);
        else {
          if (adr%4096 == 0) {
            eraseSector4M(adr);
          }
          writeEEPROM4MData(adr++, b);
        }

        if (cnt % 64 == 0) {
          Serial.println(writeErrors);
        }
      }
      Serial.print(F("ERRORS: "));
      Serial.println(writeErrors);
    }
  }
}

void loop() {
  Serial.println(F("EEPROM Burner ready"));
  Serial.println(F("1) Set Chip type"));
  Serial.println(F("2) Read Chip"));
  Serial.println(F("3) Write Chip"));
  if (CHIP_TYPE==2)
    Serial.println(F("4) Erase Chip"));

  Serial.println(">");
  char b = getSelectionCharacter();
  
  if (b=='1') {
    selectChipType();
  }
  
  if (b=='2') {
    readChip();
  }
  
  if (b=='3') {
    writeChip();
  }
  
  if (b=='4' && CHIP_TYPE==2) {
    eraseChip4M();
  }
}
