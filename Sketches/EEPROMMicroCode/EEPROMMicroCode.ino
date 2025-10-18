//
// Creates the EEPROMs for the MicroCode.
//
// Written 01.06.2020 by Ulf Caspers
//

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE 13

#define DISPLAY_BYTES true

#define READ true     // for the setAddress procedure
#define WRITE false   // for the setAddress procedure

#define CHIP_ENABLE true
#define CHIP_DISABLE false

#define ROM_NR 5
#define LAST_MC_ROM 3
#define FIRST_LABEL_ROM (LAST_MC_ROM + 1)
#define IS_LABEL_ROM (ROM_NR >= FIRST_LABEL_ROM)
#define IS_MC_ROM !IS_LABEL_ROM

#define _HC (uint32_t)1 // Halt Clock
#define _IC ((uint32_t)1 << 1) // Instruction Command
#define _IL ((uint32_t)1 << 2) // Instruction Load
#define _PC ((uint32_t)1 << 3) // ProgramCounter++
#define _SC ((uint32_t)1 << 4) // Stack pointer Change
#define _SD ((uint32_t)1 << 5) // Stackpointer down
#define _PS ((uint32_t)1 << 6) // Port Set number
#define _TI ((uint32_t)1 << 7) // Toggle Interrupt inhibit

#define _PO ((uint32_t)1 << 8) // Program counter On address bus
#define _SO ((uint32_t)1 << 9) // Stack pointer On address bus
#define _NO ((uint32_t)1 << 10) // New address On address bus
#define _PI ((uint32_t)1 << 11) // Program counter In from address bus
#define _SI ((uint32_t)1 << 12) // Stack pointer In from address bus
#define _NL ((uint32_t)1 << 13) // New address Low byte from D-Bus
#define _NH ((uint32_t)1 << 14) // New address High byte from D-Bus
#define _PW ((uint32_t)1 << 15) // Port Write byte from D-Bus

#define _NC (_NL | _NH) // Count NewAddress

#define _MW ((uint32_t)1 << 16) // Memory Write from D-Bus
#define _AW ((uint32_t)1 << 17) // register A Write from D-Bus
#define _BW ((uint32_t)1 << 18) // register B Write from D-Bus
#define _CW ((uint32_t)1 << 19) // register C Write from D-Bus
#define _DW ((uint32_t)1 << 20) // register D Write from D-Bus
#define _OW ((uint32_t)1 << 21) // Out register Write from D-Bus
#define _ZW ((uint32_t)1 << 22) // ALU register Write
#define _FW ((uint32_t)1 << 23) // ALU Flag register Write

// D-Bus enable bits
#define __E0 ((uint32_t)1 << 24)
#define __E1 ((uint32_t)1 << 25)
#define __E2 ((uint32_t)1 << 26)
#define __E3 ((uint32_t)1 << 27)

// D-Bus enable control statements
#define _P1 ((uint32_t)1 << 24)
#define _P2 ((uint32_t)2 << 24)
#define _S1 ((uint32_t)3 << 24)
#define _S2 ((uint32_t)4 << 24)
#define _N1 ((uint32_t)5 << 24)
#define _N2 ((uint32_t)6 << 24)
#define _ME ((uint32_t)7 << 24)
#define _AE ((uint32_t)8 << 24)
#define _BE ((uint32_t)9 << 24)
#define _CE ((uint32_t)10 << 24)
#define _DE ((uint32_t)11 << 24)
#define _ZE ((uint32_t)12 << 24)
#define _FE ((uint32_t)13 << 24)
#define _PE ((uint32_t)14 << 24)
//#define  ((uint32_t)15 << 24)

// ALU function bits
#define _ZS ((uint32_t)1 << 28)
#define _Z0 ((uint32_t)1 << 29)
#define _Z1 ((uint32_t)1 << 30)
#define _Z2 ((uint32_t)1 << 31)

// ALU functions
#define _ALU_0   ((uint32_t)0    )
#define _ALU_SUB (_Z0            )
#define _ALU_SBI (      _Z1      )
#define _ALU_ADD (_Z0 | _Z1      )
#define _ALU_XOR (            _Z2)
#define _ALU_OR  (_Z0 |       _Z2)
#define _ALU_AND (      _Z1 | _Z2)
#define _ALU_255 (_Z0 | _Z1 | _Z2)
#define _ALU_BUS (_Z0             | _ZS)
#define _ALU_LSL (      _Z1       | _ZS)
#define _ALU_LSR (_Z0 | _Z1       | _ZS)

// ALU Flag functions
#define _FLG_BUS (_Z0             | _ZS)
#define _FLG_CLC (            _Z2 | _ZS)
#define _FLG_STC (_Z0       | _Z2 | _ZS)

// CPU State Flags
#define _F_C ((int)1)
#define _F_Z ((int)2)
#define _F_N ((int)4)
#define _F_V ((int)8)
#define _F_II ((int)16) /* 1 = interrupt inhibited */
#define _F_IR ((int)32) /* 1 = interrupt requested */
#define _F_IF ((int)64) /* 1 = instruction fetch */
#define _F_EC ((int)128) /* 1 = extended command */

#define _negativ (_IC | _IL | _SC | _SD | _PS | _PO | _SO | _NO | _PI | _PW | _MW | _AW | _BW | _CW | _DW | _OW | _ZW | _FW)

int fetchAddr = 0;
int interruptAddr = 0;
int nopAddr = 0;

int addr = 0;
int cmd = 0;

const char     regName[] = {'A', 'B', 'C', 'D'};
const uint32_t allRegW[] = {_AW, _BW, _CW, _DW};
const uint32_t allRegE[] = {_AE, _BE, _CE, _DE};

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

  if (DISPLAY_BYTES) {
    char buf[60];
    sprintf(buf, "@%02x", address/65536);
    Serial.print(buf);    
    sprintf(buf, "%04x=%02x", address%65536, odata);
    Serial.println(buf);    
    return;
  } 

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
// Erase a sector on the 4M bit chip.
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
// Print the content of a 256 byte area on the 4M bit chip.
//
// baseAddress - start address of the 256 bytes
//
void printContents4M(unsigned long baseAddress) {
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

  if (DISPLAY_BYTES) {
    char buf[60];
    sprintf(buf, "@%06x=%02x", address, odata);
    Serial.println(buf);    
    return;
  } 
  
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

//
// Print the content of a 256 byte area on the 8k byte chip.
//
// baseAddress - start address of the 256 bytes
//
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

//
// Write a byte of a micro code step.
// 
// code - the full micro code step
//
void writeMicroCodeByte(int address, uint32_t code) {
  if (!IS_MC_ROM)
    return;

  uint32_t inv = code ^ _negativ;
  byte data = (inv >> (ROM_NR * 8)) & 0xff;

  /*
    char buf[80];
    sprintf(buf, "Out %04x: (%08lx, %08lx) %02x", address, code, inv, data);
    Serial.println(buf);
  */

  writeEEPROM(address, data);
}

//
// Write all bytes for an unconditional label into the label EEPROM.
//
// command     - true/false whether or not this label is for an instruction
// labelNumber - 0-255 number of the label
// destAddress - micro code address this label points to
//
void writeLabelUncond(boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
  writeLabelForNotFlags(0, command, labelNumber, destAddress);
}

//
// Write all bytes for an conditional label into the label EEPROM.
//
// flags       - ORed list of flags that MUST NOT be set for this label to be active
// command     - true/false whether or not this label is for an instruction
// labelNumber - 0-255 number of the label
// destAddress - micro code address this label points to
//
void writeLabelForFlags(int flags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
  for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V  | _F_II); f++) {
    if ((f & flags) != 0) {
      writeLabelByte(f, command, labelNumber, destAddress);
      writeLabelByte(f | _F_IR, command, labelNumber, destAddress);
      if (command) {
        writeLabelByte(f | _F_IF, command, labelNumber, fetchAddr);
        // no interrupt if inhibited or between the two extended command bytes
        if (((f & _F_II) == 0) && (labelNumber < 256)) {
          writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, interruptAddr);
        } else {
          writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, fetchAddr);
        }
      } else {
        writeLabelByte(f | _F_IF, command, labelNumber, destAddress);
        writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, destAddress);
      }
    }
  }
}


//
// Write all bytes for an conditional label into the label EEPROM.
//
// flags       - ORed list of flags that MUST be set for this label to be active
// command     - true/false whether or not this label is for an instruction
// labelNumber - 0-255 number of the label, 0-511 number of the command
// destAddress - micro code address this label points to
//
void writeLabelForNotFlags(int flags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
  for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V | _F_II); f++) {
    if ((f & flags) == 0) {
      writeLabelByte(f, command, labelNumber, destAddress);
      writeLabelByte(f | _F_IR, command, labelNumber, destAddress);
      if (command) {
        writeLabelByte(f | _F_IF, command, labelNumber, fetchAddr);
        // no interrupt if inhibited or between the two extended command bytes
        if (((f & _F_II) == 0) && (labelNumber < 256)) { 
          writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, interruptAddr);
        } else {
          writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, fetchAddr);
        }
      } else {
        writeLabelByte(f | _F_IF, command, labelNumber, destAddress);
        writeLabelByte(f | _F_IR | _F_IF, command, labelNumber, destAddress);
      }
    }
  }
}

//
// writes a byte into the label EEPROM
//
// labelFlags  - exact combination of active flags for which this label is valid
// command     - true/false whether or not this label is for an instruction
// lableNumber - 0-255 number of the label, 0-511 number of the command
// destAddress - micro code address this label points to
//
void writeLabelByte(int labelFlags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;

  unsigned long labelAddress = (labelNumber & 0xff) + (command ? 0 : 256) + ((unsigned long) labelFlags) * 512;
  // extended command
  if (labelNumber > 255) {
    labelAddress  += ((unsigned long) _F_EC) * 512;
  }
  byte data = (destAddress >> ((ROM_NR - FIRST_LABEL_ROM) * 8)) & 0xff;

  /*
    char buf[80];
    sprintf(buf, "label %04lx: (F%01x C%d N%02x A%04x) %02x", labelAddress, labelFlags, command, labelNumber, destAddress, data);
    Serial.println(buf);
  */

  writeEEPROM4MData(labelAddress, data);
}

//
// Calculate the micro code for a micro code go to label.
//
uint32_t _goto(int labelNumber) {
  return _IL | (((uint32_t)labelNumber) << 24);
}

//
// Output a command definition on the serial bus.
//
// cmdName  - assembler mnemonic for this command
// pcmdCode - instruction code byte for this command
//
void showCommand(char* cmdName, int pcmdCode, int destAddress) {
  int cmdCode = pcmdCode & 0xff;
  char buf[60];
  
  char extCmd[8];
  if (pcmdCode > 255) { 
    extCmd[0] = ' ';
    extCmd[1] = '0';
    extCmd[2] = 'x';
    extCmd[3] = 'f';
    extCmd[4] = 'f';
    extCmd[5] = ' ';
    extCmd[6] = '@';
    extCmd[7] = '\0';
  }
  else {  
    extCmd[0] = '\0';
  }
  
  if (strstr(cmdName, ",#") != NULL) {
    sprintf(buf, " %s{v}\t=>%s 0x%02x @ v[7:0] ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
  }
  else if (strstr(cmdName, "addr") != NULL) {
    sprintf(buf, " %s\t=>%s 0x%02x @ ad[7:0] @ ad[15:8] ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
    char* ap = strstr(buf, "addr");
    ap[0] = '{';
    ap[1] = 'a';
    ap[2] = 'd';
    ap[3] = '}';
  }
  else if (strstr(cmdName, "val") != NULL) {
    sprintf(buf, " %s\t=>%s 0x%02x @ v[7:0] ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
    char* ap = strstr(buf, "val");
    ap[0] = '{';
    ap[1] = 'v';
    ap[2] = '}';
  }
  else {
    sprintf(buf, " %s\t=>%s 0x%02x ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
  }
  Serial.println(buf);
}

void setup() {
  // put your setup code code here, to run once:
  Serial.begin(57600);
  char buf[60];
  sprintf_P(buf, PSTR("start ROM #%d"), ROM_NR);
  Serial.println(buf);

  while (Serial.available() == 0)
  {
  }

  unsigned long startMillis = millis();

  initPorts();

  if (IS_LABEL_ROM) {
    Serial.println(F("Erasing..."));
    eraseChip4M();
  }

  /* RESET */
  writeMicroCodeByte(addr++, 0); /* NOOP */

  /* read program start from 0xfffc */
  writeMicroCodeByte(addr++, _ALU_255 | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _NO | _SI); /* Stack-Pointer to 0xffff */
  writeMicroCodeByte(addr++, _ALU_0 | _ZW | _SD); /* Stack-Pointer count down */
  writeMicroCodeByte(addr++, _SC | _SD); /* Stack-Pointer to 0xfffe */
  writeMicroCodeByte(addr++, _SC | _SD); /* Stack-Pointer to 0xfffd */
  writeMicroCodeByte(addr++, _SC | _SD); /* Stack-Pointer to 0xfffc */
  writeMicroCodeByte(addr++, _SO | _PI); /* Program counter to 0xfffc */
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SC); /* read low byte to new address */
                                             /* Program counter to 0xfffd */
                                             /* Stack-Pointer to 0xfffd */
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _SC); /* read high byte to new address */
                                            /* Stack-Pointer to 0xfffe */
  writeMicroCodeByte(addr++, _NO | _PI | _SC); /* set Progam counter */
                                         /* Stack-Pointer to 0xffff */
  
  /* Initialize registers to zero */
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _ZE | _NH | _FLG_BUS | _FW | _OW);

  /* Ensure Interrupt inhibited */
  writeMicroCodeByte(addr++, _goto(0));
  writeLabelForNotFlags(_F_II, false, 0, addr);
  writeMicroCodeByte(addr++, _TI);
  writeLabelForFlags(_F_II, false, 0, addr);
  
  writeMicroCodeByte(addr++, _ZE | _AW | _BW | _CW | _DW | _IC);
  /* fall through to fetch */

  /* Fetch */
  fetchAddr = addr;
  writeMicroCodeByte(addr++, _PO | _PC | _ME | _IC | _IL);

  /* Interrupt */
  interruptAddr = addr;
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD             | _ALU_255 | _ZW);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _SC | _SD             | _ZE | _NH);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD             | _ZE | _ALU_LSL | _ZW);  
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _NO | _PI);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _TI);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);  

  /* NOP */
  nopAddr = addr;
  showCommand("NOP", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _IC);

  /* HLT */
  showCommand("HLT", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _HC | _IC);

  /* CLC */
  showCommand("CLC", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW | _IC);

  /* STC */
  showCommand("STC", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_STC | _FW | _IC);

  int iiToggleAddr = addr;
  writeMicroCodeByte(addr++, _TI | _IC);

  /* SII */
  showCommand("SII", cmd, iiToggleAddr);
  writeLabelForFlags(_F_II, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_II, true, cmd++, iiToggleAddr);

  /* CII */
  showCommand("CII", cmd, iiToggleAddr);
  writeLabelForFlags(_F_II, true, cmd, iiToggleAddr);
  writeLabelForNotFlags(_F_II, true, cmd++, nopAddr);

  /* RTS */
  showCommand("RTS", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NH | _SO | _ME);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NL | _SO | _ME);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  /* RTI */
  showCommand("RTI", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NH | _SO | _ME);
  writeMicroCodeByte(addr++, _SC | _TI);
  writeMicroCodeByte(addr++, _NL | _SO | _ME);
  writeMicroCodeByte(addr++, _NO | _PI | _SC);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC);  
  
  /* Register */
  int dest = 0;
  int src = 0;
  while (dest < 4) {
    src = 0;
    while (src < 4) {
      if (src != dest) {
        sprintf_P(buf, PSTR("MOV R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[src] | allRegW[dest] | _IC);

        sprintf_P(buf, PSTR("ADD R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_ADD | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

        sprintf_P(buf, PSTR("SUB R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_SUB | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

        sprintf_P(buf, PSTR("AND R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_AND | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

        sprintf_P(buf, PSTR("OR  R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_OR | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

        sprintf_P(buf, PSTR("XOR R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_XOR | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

        sprintf_P(buf, PSTR("CMP R%c,R%c"), regName[dest], regName[src]);
        showCommand(buf, cmd, addr);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_SUB | _FW | _IC);
      }
      src++;
    }
    dest++;
  }

  /* immediate */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "MOV R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | allRegW[dest] | _PC | _IC);

    sprintf(buf, "ADD R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

    sprintf(buf, "SUB R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

    sprintf(buf, "AND R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

    sprintf(buf, "OR  R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

    sprintf(buf, "XOR R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

    sprintf(buf, "CMP R%c,#", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _PC | _ALU_SUB | _FW | _IC);

    dest++;
  }

  /* absolut */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "MOV R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _PC | _IC);

    sprintf(buf, "MOV addr,R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _PC | _IC);

    sprintf(buf, "ADD R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "SUB R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "AND R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "OR  R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "XOR R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "CMP R%c,addr", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _FW | _IC);

    dest++;
  }

  /* indirect */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "MOV R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _IC);

    sprintf(buf, "MOV [RCD],R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _IC);

    sprintf(buf, "ADD R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "SUB R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "AND R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "OR  R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "XOR R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

    sprintf(buf, "CMP R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _FW | _IC);

    dest++;
  }

  /* OUT */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "OUT R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _OW | _IC);
    dest++;
  }

  showCommand("OUT addr", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _OW | _PC | _IC);

  showCommand("OUT [RCD]", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _OW | _IC);

  /* LSL */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "LSL R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_LSL | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
    dest++;
  }

  showCommand("LSL addr", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSL | _ZW | _FW | _PC);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  showCommand("LSL [RCD]", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSL | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  /* LSR */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "LSR R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_LSR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
    dest++;
  }

  showCommand("LSR addr", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSR | _ZW | _FW | _PC);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  showCommand("LSR [RCD]", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSR | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  /* PSH/PUL */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "PSH R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _SO | _MW | _SD);
    writeMicroCodeByte(addr++, _SC | _SD | _IC);

    sprintf(buf, "PUL R%c", regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _SC);
    writeMicroCodeByte(addr++, allRegW[dest] | _SO | _ME | _IC);

    dest++;
  }

  showCommand("PSF", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD | _IC);

  showCommand("PLF", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_OR | _FW);
  writeMicroCodeByte(addr++, _goto(1));
  int toggleAddr = addr;
  writeMicroCodeByte(addr++, _TI);
  int noToggleAddr = addr;
  if (IS_LABEL_ROM) {
    for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V | _F_II | _F_IR | _F_IF); f++) {
      if ((f & (_F_N | _F_II)) == 0) {
        writeLabelByte(f, false, 1, noToggleAddr);
        writeLabelByte(f | _F_N, false, 1, toggleAddr);
        writeLabelByte(f | _F_II, false, 1, toggleAddr);
        writeLabelByte(f | _F_N | _F_II, false, 1, noToggleAddr);
      }
    }
  }
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC);

  /* Stackpointer */
  showCommand("MOV SP,RCD", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _SI | _IC);

  showCommand("MOV RCD,SP", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _S1 | _CW);
  writeMicroCodeByte(addr++, _S2 | _DW | _IC);

  /* JMP addr */
  int jmpAddr = addr;
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  int noJmpAddr = addr;
  writeMicroCodeByte(addr++, _PC);
  writeMicroCodeByte(addr++, _PC | _IC);

  showCommand("JMP addr", cmd, jmpAddr);
  writeLabelUncond(true, cmd++, jmpAddr);

  showCommand("JSR addr", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _NO | _PI | _SC | _SD | _IC);

  showCommand("JCS addr", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, noJmpAddr);

  showCommand("JZS addr", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, noJmpAddr);

  showCommand("JNS addr", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, noJmpAddr);

  showCommand("JVS addr", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, noJmpAddr);

  showCommand("JNC addr", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);

  showCommand("JNZ addr", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);

  showCommand("JNN addr", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);

  showCommand("JNV addr", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);

  /* JMP [RCD] */
  jmpAddr = addr;
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  showCommand("JMP [RCD]", cmd, jmpAddr);
  writeLabelUncond(true, cmd++, jmpAddr);

  showCommand("JSR [RCD]", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _NO | _PI | _SC | _SD | _IC);

  showCommand("JCS [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, nopAddr);

  showCommand("JZS [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, nopAddr);

  showCommand("JNS [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, nopAddr);

  showCommand("JVS [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, nopAddr);

  showCommand("JNC [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);

  showCommand("JNZ [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);

  showCommand("JNN [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);

  showCommand("JNV [RCD]", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);

  showCommand("OUT val,RA", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
  writeMicroCodeByte(addr++, _AE | _PW | _IC);

  showCommand("INP RA,val", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
  writeMicroCodeByte(addr++, _AW | _PE | _IC);

  showCommand("OUT RB,RA", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _BE | _PS);
  writeMicroCodeByte(addr++, _AE | _PW | _IC);

  showCommand("INP RA,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _BE | _PS);
  writeMicroCodeByte(addr++, _AW | _PE | _IC);

  showCommand("MOV RA,addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _AW);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

  showCommand("MOV RA,[addr],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  /*
  writeMicroCodeByte(addr++, _FE | _SO | _MW) // save flags
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
  writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
  writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
  writeMicroCodeByte(addr++, _ZE | _NL );
  writeMicroCodeByte(addr++, _ALU_0 | _ZW ); // carry to base address high byte
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW ); 
  writeMicroCodeByte(addr++, _ZE | _NH );
  writeMicroCodeByte(addr++, _NO | _ME | _AW);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);
  */
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL | _SC);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _AW);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

  showCommand("MOV addr,RB,RA", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC);
  writeMicroCodeByte(addr++, _NO | _MW | _AE);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

  showCommand("MOV [addr],RB,RA", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL | _SC);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _NO | _MW | _AE);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

  /* set unused op codes to HLT */
  Serial.println(F("; Writing unsused codes"));

  writeMicroCodeByte(addr, _HC);
  while (cmd < 0xFF) {
    writeLabelUncond(true, cmd++, addr);
  }
  addr++;

  Serial.println(F("; Writing extended commands"));
  /* extended Command always at command 0xFF */
  writeLabelUncond(true, cmd++, fetchAddr);

  /* SPI Bus read Byte */
  showCommand("INB RA,val", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
  writeMicroCodeByte(addr++, _ZE | _AW | _ALU_LSL | _ZW); // A=1; Z=2
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=2
  writeMicroCodeByte(addr++, _PE | _ALU_OR  | _ZW); // Z=SPI, CLK=1
  writeMicroCodeByte(addr++, _AE | _ALU_OR  | _ZW); // Z=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _ZE | _NH); // NH=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=1
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=1
  
  writeLabelForNotFlags(_F_C, false, 2, addr);
  writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
  writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c_flg=MISO
  writeMicroCodeByte(addr++, _AE | _ALU_LSL | _ZW | _FW); // Z=A<-MISO; C-Flg=<-A
  writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
  writeMicroCodeByte(addr++, _ZE | _AW); // A=Z
  writeMicroCodeByte(addr++, _goto(2));
  
  writeLabelForFlags(_F_C, false, 2, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW | _IC);

  /* SPI Bus read Buffer */
  showCommand("INB [RCD],RB,val", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC); // Port selector
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD); // save PC to stack
  writeMicroCodeByte(addr++, _SC | _SD ); // save PC to stack
  writeMicroCodeByte(addr++, _P2 | _SO | _MW); // save PC to stack
  
  writeMicroCodeByte(addr++, _CE | _NL); // dest buffer low
  writeMicroCodeByte(addr++, _DE | _NH | _ALU_0 | _ZW); // dest buffer high
  writeMicroCodeByte(addr++, _NO | _PI | _FLG_STC | _FW); // dest buffer -> PC
  
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
  writeMicroCodeByte(addr++, _ZE | _DW | _ALU_LSL | _ZW); // D=1; Z=2
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=2
  writeMicroCodeByte(addr++, _PE | _ALU_OR  | _ZW); // Z=SPI, CLK=1
  writeMicroCodeByte(addr++, _DE | _ALU_OR  | _ZW); // Z=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _ZE | _NH); // NH=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=1
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=1
  
  writeLabelForNotFlags(_F_Z, false, 3, addr);
  writeMicroCodeByte(addr++, _DE | _ALU_BUS | _ZW); // Z=1
  
  writeLabelForNotFlags(_F_C, false, 4, addr);
  writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
  writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
  writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
  writeMicroCodeByte(addr++, _goto(4));
  
  writeLabelForFlags(_F_C, false, 4, addr);
  writeMicroCodeByte(addr++, _ZE | _PO | _MW); // save received byte
  writeMicroCodeByte(addr++, _PC | _BE | _ALU_BUS | _ZW); // Z=B
  writeMicroCodeByte(addr++, _DE | _ALU_SUB | _ZW | _FW); // Z=B-1, Z-Flg=(B==0)
  writeMicroCodeByte(addr++, _ZE | _BW); // B=B-1
  writeMicroCodeByte(addr++, _goto(3));
  
  writeLabelForFlags(_F_Z, false, 3, addr);
  writeMicroCodeByte(addr++, _P1 | _CW);
  writeMicroCodeByte(addr++, _P2 | _DW);
  writeMicroCodeByte(addr++, _SO | _ME | _NH);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _SO | _ME | _NL);  
  writeMicroCodeByte(addr++, _NO | _PI | _FLG_CLC | _FW | _IC);

  /* SPI Bus write Byte */
  showCommand("OUB val,RA", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC | _ALU_0 | _ZW); // port selection
  writeMicroCodeByte(addr++, _CE | _SO | _MW | _FLG_STC | _FW); //c register on stack
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
  writeMicroCodeByte(addr++, _ZE | _NL | _CW | _ALU_LSL | _ZW); // NL=1;C=1; Z=2
  writeMicroCodeByte(addr++, _ZE | _NH); // NH=2 (=>CLK=1)
  writeMicroCodeByte(addr++, _PE | _ALU_OR  | _ZW); // Z=SPI, CLK=1
  writeMicroCodeByte(addr++, _N1 | _ALU_OR  | _ZW); // Z=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=1, MOSI=0
  writeMicroCodeByte(addr++, _N2 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=0
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=0
  
  writeLabelForNotFlags(_F_C, false, 5, addr);
  writeMicroCodeByte(addr++, _AE | _ALU_LSL | _ZW | _FW); // c_flg=MOSI
  writeMicroCodeByte(addr++, _ZE | _AW | _ALU_0 | _ZW); // store byte
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW); // Z=SPI, MOSI; c-flg=0
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=0, MOSI
  writeMicroCodeByte(addr++, _N2 | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=1
  writeMicroCodeByte(addr++, _N2 | _ALU_XOR | _ZW); // Z=SPI, CLK=0
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=0
  writeMicroCodeByte(addr++, _CE | _ALU_LSL | _ZW | _FW); // Z=c*2
  writeMicroCodeByte(addr++, _ZE | _CW); // c=c*2
  writeMicroCodeByte(addr++, _goto(5)); // c>255 (c-flag set?)
  
  writeLabelForFlags(_F_C, false, 5, addr);
  writeMicroCodeByte(addr++, _SO | _ME | _CW | _FLG_CLC | _FW | _IC);

  /* SPI Bus write Buffer */
  showCommand("OUB val,[RCD],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC); // Port selector
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD); // save PC to stack
  writeMicroCodeByte(addr++, _SC | _SD ); // save PC to stack
  writeMicroCodeByte(addr++, _P2 | _SO | _MW); // save PC to stack
  
  writeMicroCodeByte(addr++, _CE | _NL); // src buffer low
  writeMicroCodeByte(addr++, _DE | _NH | _ALU_0 | _ZW); // src buffer high
  writeMicroCodeByte(addr++, _NO | _PI | _FLG_STC | _FW); // src buffer -> PC

  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
  writeMicroCodeByte(addr++, _ZE | _NL | _DW | _ALU_LSL | _ZW); // NL=1;D=1; Z=2
  writeMicroCodeByte(addr++, _ZE | _NH); // NH=2 (=>CLK=1)
  writeMicroCodeByte(addr++, _PE | _ALU_OR  | _ZW); // Z=SPI, CLK=1
  writeMicroCodeByte(addr++, _N1 | _ALU_OR  | _ZW); // Z=SPI, CLK=1, MOSI=1
  writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=1, MOSI=0
  writeMicroCodeByte(addr++, _N2 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=0
  writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=0

  writeLabelForNotFlags(_F_Z, false, 6, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _AW); // next byte to send
  writeMicroCodeByte(addr++, _DE | _CW | _PC); // c=1
  
  writeLabelForNotFlags(_F_C, false, 7, addr);
  writeMicroCodeByte(addr++, _AE | _ALU_LSL | _ZW | _FW); // c_flg=MOSI
  writeMicroCodeByte(addr++, _ZE | _AW | _ALU_0 | _ZW); // store byte
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW); // Z=SPI, MOSI; c-flg=0
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=0, MOSI
  writeMicroCodeByte(addr++, _N2 | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=1
  writeMicroCodeByte(addr++, _N2 | _ALU_XOR | _ZW); // Z=SPI, CLK=0
  writeMicroCodeByte(addr++, _ZE | _PW); // SPI CLK=0
  writeMicroCodeByte(addr++, _CE | _ALU_LSL | _ZW | _FW); // Z=c*2
  writeMicroCodeByte(addr++, _ZE | _CW); // c=c*2
  writeMicroCodeByte(addr++, _goto(7)); // c>255 (c-flag set?)
  
  writeLabelForFlags(_F_C, false, 7, addr);  
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // Z=B
  writeMicroCodeByte(addr++, _DE | _ALU_SUB | _ZW | _FW); // Z=B-1, Z-Flg=(B==0)
  writeMicroCodeByte(addr++, _ZE | _BW); // B=B-1
  writeMicroCodeByte(addr++, _goto(6));

  writeLabelForFlags(_F_Z, false, 6, addr);
  writeMicroCodeByte(addr++, _P1 | _CW);
  writeMicroCodeByte(addr++, _P2 | _DW);
  writeMicroCodeByte(addr++, _SO | _ME | _NH);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _SO | _ME | _NL);  
  writeMicroCodeByte(addr++, _NO | _PI | _FLG_CLC | _FW | _IC);

  /* JMP [addr] */
  jmpAddr = addr;
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _PI);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  showCommand("JMP [addr]", cmd, jmpAddr);
  writeLabelUncond(true, cmd++, jmpAddr);

  showCommand("JSR [addr]", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _NO | _PI | _SC | _SD);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  showCommand("JCS [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, noJmpAddr);

  showCommand("JZS [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, noJmpAddr);

  showCommand("JNS [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, noJmpAddr);

  showCommand("JVS [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, noJmpAddr);

  showCommand("JNC [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_C, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);

  showCommand("JNZ [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_Z, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);

  showCommand("JNN [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_N, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);

  showCommand("JNV [addr]", cmd, jmpAddr);
  writeLabelForFlags(_F_V, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);

  /* INC/DEC */
  int incAddr;
  int decAddr;
  dest = 0;
  while (dest < 4) {
    sprintf_P(buf, PSTR("INC R%c"), regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelForNotFlags(_F_C, true, cmd, addr);
    writeMicroCodeByte(addr++, _FLG_STC | _FW);
    incAddr = addr;
    writeLabelForFlags(_F_C, true, cmd++, addr);
    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
  
    sprintf_P(buf, PSTR("ICC R%c"), regName[dest]);
    showCommand(buf, cmd, incAddr);
    writeLabelUncond(true, cmd++, incAddr);
    
    sprintf_P(buf, PSTR("DEC R%c"), regName[dest]);
    showCommand(buf, cmd, addr);
    writeLabelForFlags(_F_C, true, cmd, addr);
    writeMicroCodeByte(addr++, _FLG_CLC | _FW);
    decAddr = addr;
    writeLabelForNotFlags(_F_C, true, cmd++, addr);
    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_SBI | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
  
    sprintf_P(buf, PSTR("DCC R%c"), regName[dest]);
    showCommand(buf, cmd, decAddr);
    writeLabelUncond(true, cmd++, decAddr);

    dest++;
  }
  
  sprintf_P(buf, PSTR("INC RCD"));
  showCommand(buf, cmd, addr);
  writeLabelForNotFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_STC | _FW);
  writeLabelForFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _CE | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _CW | _ZE | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _DE | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _DW | _ZE | _IC);
  
  sprintf_P(buf, PSTR("DEC RCD"));
  showCommand(buf, cmd, addr);
  writeLabelForFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeLabelForNotFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _CE | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _CW | _ZE | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _DE | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _DW | _ZE | _IC);
  
  sprintf_P(buf, PSTR("INC addr"));
  showCommand(buf, cmd, addr);
  writeLabelForNotFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_STC | _FW);
  incAddr = addr;
  writeLabelForFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);
  
  sprintf_P(buf, PSTR("ICC addr"));
  showCommand(buf, cmd, incAddr);
  writeLabelUncond(true, cmd++, incAddr);
    
  sprintf_P(buf, PSTR("DEC addr"));
  showCommand(buf, cmd, addr);
  writeLabelForFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  decAddr = addr;
  writeLabelForNotFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);
  
  sprintf_P(buf, PSTR("DCC addr"));
  showCommand(buf, cmd, decAddr);
  writeLabelUncond(true, cmd++, decAddr);

  showCommand("INC addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("ICC addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("DEC addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("DCC addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE | _PC);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("INC [addr],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("ICC [addr],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL | _SC);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("DEC [addr],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  showCommand("DCC [addr],RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NL | _ZE);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NH | _ZE);
  writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _NL | _SC);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

  /* move 16-bit address into RC/RD register pair */
  showCommand("MVA RCD,addr", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _CW | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _DW | _PC | _IC);
  
  showCommand("INC [RCD]", cmd, addr);
  writeLabelForNotFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_STC | _FW);
  incAddr = addr;
  writeLabelForFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _DE | _NH );
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);
  
  showCommand("ICC [RCD]", cmd, incAddr);
  writeLabelUncond(true, cmd++, incAddr);
    
  showCommand("DEC [RCD]", cmd, addr);
  writeLabelForFlags(_F_C, true, cmd, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW);
  decAddr = addr;
  writeLabelForNotFlags(_F_C, true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL | _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _DE | _NH );
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);
  
  showCommand("DCC [RCD]", cmd, decAddr);
  writeLabelUncond(true, cmd++, decAddr);

  /* Push/Pull memory block onto/from Stack */
  showCommand("PSB addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  
  writeLabelForNotFlags(_F_Z, false, 8, addr);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _SD);
  writeMicroCodeByte(addr++, _SO | _MW | _ZE | _NC | _SD | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW | _SC | _SD);
  writeMicroCodeByte(addr++, _BE | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _BW);
  writeMicroCodeByte(addr++, _goto(8));
  
  writeLabelForFlags(_F_Z, false, 8, addr);
  writeMicroCodeByte(addr++, _IC);
  
  showCommand("PLB addr,RB", cmd, addr);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _SC);
  
  writeLabelForNotFlags(_F_Z, false, 9, addr);
  writeMicroCodeByte(addr++, _SO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _NO | _MW | _ZE | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _ALU_0 | _ZW | _NC | _SC);
  writeMicroCodeByte(addr++, _BE | _ALU_SBI | _ZW | _FW);
  writeMicroCodeByte(addr++, _ZE | _BW | _SD);
  writeMicroCodeByte(addr++, _goto(9));
  
  writeLabelForFlags(_F_Z, false, 9, addr);
  writeMicroCodeByte(addr++, _SD | _SC | _IC);

  /* set unused extended op codes to HLT */
  Serial.println(F("; Writing unsused codes"));

  writeMicroCodeByte(addr, _HC);
  while (cmd < 0x200) {
    writeLabelUncond(true, cmd++, addr);
  }
/*
  if (IS_MC_ROM)
    printContents(0);
  else
    printContents4M(0);
*/
/*  int spentSeconds = (millis() - startMillis) / 1000;

  sprintf_P(buf, PSTR("last addr: 0x%03x\r\n\r\ntook %ds ROM#%d"), addr, spentSeconds, ROM_NR);
  Serial.println(buf);
  */
  if (writeErrors > 0) {
    sprintf_P(buf, PSTR("\r\nERRORs: %d\r\n"), writeErrors);
    Serial.println(buf);
  }
 
 Serial.println(F("done."));
}


  /*
    // Test routines
    sprintf(buf, "Cmd MALU: %02x %02x", addr, cmd);
    writeLabelUncond(true, cmd++, addr);
    Serial.println(buf);

    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, _ZE | _NH | _NL | _AW | _BW | _CW | _DW);
    writeMicroCodeByte(addr++, _NO | _SI | _PI);

    writeLabelUncond(false, 8, addr);

    writeMicroCodeByte(addr++, _P1 | _BW);
    writeMicroCodeByte(addr++, _P1 | _CW);
    writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW );
    writeMicroCodeByte(addr++, _FLG_STC | _FW);
    writeMicroCodeByte(addr++, _CE | _ALU_SUB | _ZW);
    writeMicroCodeByte(addr++, _FLG_CLC | _FW);
    writeMicroCodeByte(addr++, _AE | _ALU_ADD | _ZW);
    writeMicroCodeByte(addr++, _ZE | _AW );
    writeMicroCodeByte(addr++, _PC);
    writeMicroCodeByte(addr++, _goto(8));
    writeMicroCodeByte(addr++, _HC);

    sprintf(buf, "Cmd MALUD: %02x %02x", addr, cmd);
    writeLabelUncond(true, cmd++, addr);
    Serial.println(buf);

    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, _ZE | _NH | _NL | _AW | _BW | _CW | _DW);
    writeMicroCodeByte(addr++, _NO | _SI | _PI);

    writeLabelUncond(false, 9, addr);

    writeMicroCodeByte(addr++, _DE | _BW);
    writeMicroCodeByte(addr++, _DE | _CW);
    writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW );
    writeMicroCodeByte(addr++, _FLG_STC | _FW);
    writeMicroCodeByte(addr++, _CE | _ALU_SUB | _ZW);
    writeMicroCodeByte(addr++, _FLG_CLC | _FW);
    writeMicroCodeByte(addr++, _AE | _ALU_ADD | _ZW);
    writeMicroCodeByte(addr++, _ZE | _AW );
    writeMicroCodeByte(addr++, _FLG_STC | _FW);
    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, _DE | _ALU_ADD | _ZW);
    writeMicroCodeByte(addr++, _ZE | _DW );
    writeMicroCodeByte(addr++, _goto(9));
    writeMicroCodeByte(addr++, _HC);

    sprintf(buf, "Cmd MMem: %02x %02x", addr, cmd);
    writeLabelUncond(true, cmd++, addr);
    Serial.println(buf);

    writeMicroCodeByte(addr++, _ALU_0 | _ZW);
    writeMicroCodeByte(addr++, _ZE | _NH | _NL | _AW | _BW | _CW | _DW);
    writeMicroCodeByte(addr++, _NO | _SI | _PI);

    writeLabelUncond(false, 10, addr);

    writeMicroCodeByte(addr++, _P1 | _OW | _CW);
    writeMicroCodeByte(addr++, _P2 | _DW);
    writeMicroCodeByte(addr++, _PO | _MW | _CE);

    writeMicroCodeByte(addr++, _CE | _NL | _PC);
    writeMicroCodeByte(addr++, _DE | _NH | _FLG_STC | _FW);
    writeMicroCodeByte(addr++, _NO | _ME | _ZW | _ALU_BUS);
    writeMicroCodeByte(addr++, _CE | _ALU_SUB | _ZW);
    writeMicroCodeByte(addr++, _FLG_CLC | _FW);
    writeMicroCodeByte(addr++, _AE | _ALU_ADD | _ZW);
    writeMicroCodeByte(addr++, _ZE | _AW | _NO | _MW);

    writeMicroCodeByte(addr++, _goto(10));
    writeMicroCodeByte(addr++, _HC);
  */

void loop() {
  // put your main code here, to run repeatedly:

}
