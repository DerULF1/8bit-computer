#define SPI_CS 10
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_CLK 13

#define CLK_DELAY 1
#define BYTE_DELAY 1


void SPI_init() {
  digitalWrite(SPI_CLK, LOW);
  digitalWrite(SPI_CS, HIGH);
  digitalWrite(SPI_MOSI, HIGH);
  pinMode(SPI_CS, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_CLK, OUTPUT);
  for (int i = 0; i<20; i++) {
    SPI_send_byte(0xff);
  }
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

void setup() {
  Serial.begin(57600);
  SPI_init();
  
  digitalWrite(SPI_CS, LOW);

  SPI_send_byte(0x00);
  byte b = SPI_receive_byte();
/*  digitalWrite(SPI_CS, HIGH);
  
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x01);*/
  byte b2 = SPI_receive_byte();
  byte b3 = SPI_receive_byte();
  byte b4 = SPI_receive_byte();
  byte b5 = SPI_receive_byte();
  byte b6 = SPI_receive_byte();
  byte b7 = SPI_receive_byte();

  char buf[80];
  sprintf(buf, "RTC-Byte: %02x %02x %02x %02x %02x %02x %02x", b, b2, b3, b4, b5, b6, b7);
  Serial.println(buf);
  
  digitalWrite(SPI_CS, HIGH);
  /*
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x80);
  SPI_send_byte(0x45);
  SPI_send_byte(0x55);
  SPI_send_byte(0x18);
  SPI_send_byte(0x05);
  SPI_send_byte(0x21);
  SPI_send_byte(0x08);
  SPI_send_byte(0x20);
  digitalWrite(SPI_CS, HIGH);
  */
  digitalWrite(SPI_CS, LOW);

  SPI_send_byte(0x00);
  b = SPI_receive_byte();
/*  digitalWrite(SPI_CS, HIGH);
  
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x01);*/
  b2 = SPI_receive_byte();
  b3 = SPI_receive_byte();

  sprintf(buf, "RTC-Byte: %02x %02x %02x", b, b2, b3);
  Serial.println(buf);
  
  digitalWrite(SPI_CS, HIGH);
  
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x0e);
  b = SPI_receive_byte();
  sprintf(buf, "RTC-0eByte: %02x", b);
  Serial.println(buf);
  digitalWrite(SPI_CS, HIGH);
  
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x0f);
  b = SPI_receive_byte();
  sprintf(buf, "RTC-Byte: %02x", b);
  Serial.println(buf);
  digitalWrite(SPI_CS, HIGH);
  /*
  digitalWrite(SPI_CS, LOW);
  SPI_send_byte(0x8f);
  SPI_send_byte(0x48);
  digitalWrite(SPI_CS, HIGH);
  */
}

void loop() {
  // put your main code here, to run repeatedly:

}
