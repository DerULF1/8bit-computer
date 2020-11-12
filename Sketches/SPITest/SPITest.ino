#define SPI_CS 10
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_CLK 13

#define CLK_DELAY 50
#define BYTE_DELAY 300

#define BLKSIZE 512

void SPI_init() {
  digitalWrite(SPI_CLK, LOW);
  digitalWrite(SPI_CS, HIGH);
  digitalWrite(SPI_MOSI, HIGH);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_CLK, OUTPUT);
  
  for (int i = 0; i < 20; i++) {
    SPI_send_byte(0xff);
  }
  
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x40);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x95);  
  
  byte response = SPI_receive_response_byte();

  char buf[80];
  sprintf(buf, "Response auf CMD0: %d", response);
  Serial.println(buf);
  //SPI_send_byte(0xff);

  SPI_send_byte(0x48);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x01);
  SPI_send_byte(0xaa);
  SPI_send_byte(0x87);  
  
  response = SPI_receive_response_byte();
  sprintf(buf, "Response auf CMD8: %d", response);
  Serial.println(buf);
  if (response == 1) {
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD8: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD8: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD8: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD8: %02x", response);
    Serial.println(buf);
  }

  int cnt = 10;
  do {
  //SPI_send_byte(0xff);

  SPI_send_byte(55+64);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0xff);  
  
  response = SPI_receive_response_byte();
  sprintf(buf, "Response auf CMD55: %d", response);
  Serial.println(buf);
  
  //SPI_send_byte(0xff);

  SPI_send_byte(41+64);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0xff);  
  
  response = SPI_receive_response_byte();
  sprintf(buf, "Response auf CMD41: %d", response);
  Serial.println(buf);
  } while (cnt-- > 0 && response == 1);
  
  //SPI_send_byte(0xff);

  SPI_send_byte(58+64);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(0xff);  
  
  response = SPI_receive_response_byte();
  sprintf(buf, "Response auf CMD58: %d", response);
  Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD58: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD58: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD58: %02x", response);
    Serial.println(buf);
    response = SPI_receive_byte();
    sprintf(buf, "Response auf CMD58: %02x", response);
    Serial.println(buf);
  
  //SPI_send_byte(0xff);

  SPI_send_byte(16+64);
  SPI_send_byte(0x00);
  SPI_send_byte(0x00);
  SPI_send_byte(BLKSIZE >> 8);
  SPI_send_byte(BLKSIZE & 0xff);
  SPI_send_byte(0xff);  
  
  response = SPI_receive_response_byte();
  sprintf(buf, "Response auf CMD16: %d", response);
  Serial.println(buf);
  
  digitalWrite(SPI_CS, HIGH);
  SPI_send_byte(0xff);

}

byte read_block(uint32_t offset, byte sect[]) {  
  //SPI_send_byte(0xff);

  byte addr0 = offset & 0xff;
  byte addr1 = (offset >> 8)  & 0xff;
  byte addr2 = (offset >> 16)  & 0xff;
  byte addr3 = (offset >> 24)  & 0xff;
  SPI_send_byte(17+64);
  SPI_send_byte(addr3);
  SPI_send_byte(addr2);
  SPI_send_byte(addr1);
  SPI_send_byte(addr0);
  SPI_send_byte(0xff);  
  
  byte response = SPI_receive_response_byte();
  char buf[40];
  sprintf(buf, "Response auf CMD17 ByteOffset %d: %d", offset, response);
  Serial.println(buf);
  if (response == 0) {
    byte dataResp = 0;
    while (dataResp != 0xfe) {
      dataResp = SPI_receive_byte();
    }

    for (int i = 0 ; i < (BLKSIZE / 16); i++) {
      for (int j = 0 ; j < 16; j++) {
        dataResp = SPI_receive_byte();
        sect[i*16+j] = dataResp;
      }
    }
  
    // CRC
    dataResp = SPI_receive_byte();
    dataResp = SPI_receive_byte();
  }

  return response;
}

byte SPI_receive_response_byte() {
  delayMicroseconds(BYTE_DELAY);
  digitalWrite(SPI_MOSI, HIGH);
  byte data = 0xff;
  int cnt = 64;
  while (cnt-- > 0 && (data & 0x80) != 0) {
    data = (data << 1);
    digitalWrite(SPI_CLK, HIGH);
    delayMicroseconds(CLK_DELAY);
    byte responseBit = digitalRead(SPI_MISO);
    data = data | (responseBit == 0 ? 0 : 1);
    digitalWrite(SPI_CLK, LOW);
    delayMicroseconds(CLK_DELAY);
  }
  return data;
}

byte SPI_receive_byte() {
  delayMicroseconds(BYTE_DELAY);
  digitalWrite(SPI_MOSI, HIGH);
  byte data = 0xff;
  for (int i = 0; i < 8; i++) {
    data = (data << 1);
    digitalWrite(SPI_CLK, HIGH);
    delayMicroseconds(CLK_DELAY);
    byte responseBit = digitalRead(SPI_MISO);
    data = data | (responseBit == 0 ? 0 : 1);
    digitalWrite(SPI_CLK, LOW);
    delayMicroseconds(CLK_DELAY);
  }
  return data;
}

void SPI_send_byte(byte data) {
  delayMicroseconds(BYTE_DELAY);
  for (int i = 0; i < 8; i++) {
    digitalWrite(SPI_MOSI, (data&0x80) ? HIGH : LOW);
    digitalWrite(SPI_CLK, HIGH);
    delayMicroseconds(CLK_DELAY);
    digitalWrite(SPI_CLK, LOW);
    delayMicroseconds(CLK_DELAY);
    data = (data << 1);
  }
}

void show_sector(byte sect[]) {
  char buf[80];
  
  for (int i = 0 ; i < (BLKSIZE / 16); i++) {
    sprintf(buf, "%04x: ", i*16);
    Serial.print(buf);
    for (int j = 0 ; j < 16; j++) {
      sprintf(buf, "%02x ", sect[i*16+j]);
      Serial.print(buf);
    }
    Serial.println();
  }
  
}

void setup() {
  Serial.begin(57600);
  SPI_init();
  
  digitalWrite(SPI_CS, LOW);

  byte sect[BLKSIZE];        
  byte response = read_block((uint32_t) 0, sect);

  char buf[80];
  sprintf(buf, "Response auf ReadSector: %d", response);
  Serial.println(buf);
  show_sector(sect);
    
  if (sect[0x01c2] != 0) {
    uint32_t sectnum = sect[0x1c6] + 256 * sect[0x1c7] + 65536 * sect[0x1c8] + 65536 * 256 * sect[0x1c9];
    uint32_t num = sectnum * 512;
    response = read_block(num, sect);

    sprintf(buf, "Response auf ReadSector: %d", response);
    Serial.println(buf);
    show_sector(sect);
  }

  digitalWrite(SPI_CS, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

}
