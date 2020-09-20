//
// Test for EEPROM-Programmer
//


#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

#define READ true     // for the setAddress procedure
#define WRITE false   // for the setAddress procedure


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
    l = readEEPROM(0);
    l = readEEPROM(address);
  }

  if (cnt == 0){
    char buf[80];
    sprintf(buf, "\r\nWRITE ERROR on %04x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
  }
}

byte readEEPROM(int address) {
  setDataPinMode(INPUT);  
  
  setAddress(address, READ);
  
  setDataPinMode(INPUT);  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--) {
    data = data * 2 + digitalRead(pin);
  }
  return data;
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
  Serial.println("start reading EEPROM");
  unsigned long startMillis = millis();
  
  initPorts();
  
  for (int i = 0; i < 1; i++) {
    printContents(i*256);
  }

  int spendSeconds = (millis()-startMillis) / 1000;
  char buf[80];
  sprintf(buf, "took %d seconds", spendSeconds);
  Serial.println(buf);
  Serial.println("done.");
}

void loop() {
  // put your main code here, to run repeatedly:

}
