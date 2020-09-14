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

#define READ true     // for the setAddress procedure
#define WRITE false   // for the setAddress procedure

#define CHIP_ENABLE true
#define CHIP_DISABLE false

#define ROM_NR 2

#define LAST_MC_ROM 3
#define FIRST_LABEL_ROM (LAST_MC_ROM + 1)
#define IS_LABEL_ROM (ROM_NR >= FIRST_LABEL_ROM)
#define IS_MC_ROM !IS_LABEL_ROM

#define _HC 1
#define _IC ((uint32_t)1 << 1)
#define _IL ((uint32_t)1 << 2)
#define _PC ((uint32_t)1 << 3)
#define _SC ((uint32_t)1 << 4)
#define _SD ((uint32_t)1 << 5)
#define _PS ((uint32_t)1 << 6)
#define _TI ((uint32_t)1 << 7)

#define _PO ((uint32_t)1 << 8)
#define _SO ((uint32_t)1 << 9)
#define _NO ((uint32_t)1 << 10)
#define _PI ((uint32_t)1 << 11)
#define _SI ((uint32_t)1 << 12)
#define _NL ((uint32_t)1 << 13)
#define _NH ((uint32_t)1 << 14)
#define _PW ((uint32_t)1 << 15)

#define _MW ((uint32_t)1 << 16)
#define _AW ((uint32_t)1 << 17)
#define _BW ((uint32_t)1 << 18)
#define _CW ((uint32_t)1 << 19)
#define _DW ((uint32_t)1 << 20)
#define _OW ((uint32_t)1 << 21)
#define _ZW ((uint32_t)1 << 22)
#define _FW ((uint32_t)1 << 23)

#define __E0 ((uint32_t)1 << 24)
#define __E1 ((uint32_t)1 << 25)
#define __E2 ((uint32_t)1 << 26)
#define __E3 ((uint32_t)1 << 27)
#define _ZS ((uint32_t)1 << 28)
#define _Z0 ((uint32_t)1 << 29)
#define _Z1 ((uint32_t)1 << 30)
#define _Z2 ((uint32_t)1 << 31)

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

#define _ALU_0   ((uint32_t)0    )
#define _ALU_SUB (_Z0            )
#define _ALU_ADD (_Z0 | _Z1      )
#define _ALU_XOR (            _Z2)
#define _ALU_OR  (_Z0 |       _Z2)
#define _ALU_AND (      _Z1 | _Z2)
#define _ALU_255 (_Z0 | _Z1 | _Z2)
#define _ALU_BUS (_Z0             | _ZS)
#define _ALU_LSL (      _Z1       | _ZS)
#define _ALU_LSR (_Z0 | _Z1       | _ZS)

#define _FLG_BUS (_Z0             | _ZS)
#define _FLG_CLC (            _Z2 | _ZS)
#define _FLG_STC (_Z0       | _Z2 | _ZS)

#define _F_C ((int)1)
#define _F_Z ((int)2)
#define _F_N ((int)4)
#define _F_V ((int)8)
#define _F_II ((int)16) /* 1 = interrupt inhibited */
#define _F_IR ((int)32) /* 1 = interrupt requested */
#define _F_IF ((int)64) /* 1 = instruction fetch */

#define _negativ (_IC | _IL | _SC | _PS | _PO | _SO | _NO | _PI | _NL | _NH | _PW | _MW | _AW | _BW | _CW | _DW | _OW | _ZW | _FW)

int fetchAddr = 0;
int interruptAddr = 0;

const char     regName[] = {'A', 'B', 'C', 'D'};
const uint32_t allRegW[] = {_AW, _BW, _CW, _DW};
const uint32_t allRegE[] = {_AE, _BE, _CE, _DE};

int writeErrors = 0;

void setAddress4M(unsigned long address, boolean outputEnable, boolean chipEnable) {
  digitalWrite(SHIFT_LATCH, LOW);
  
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 16) | (outputEnable ? 0 : 0x80) | (chipEnable ? 0 : 0x40));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  
  digitalWrite(SHIFT_LATCH, HIGH);
}

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

  if (cnt == 0){
    char buf[60];
    sprintf(buf, "\r\nWRITE ERROR on %06x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
    writeErrors++;
  }
}

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

void setAddress(int address, boolean outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);  
}

void writeEEPROM(int address, byte odata) {
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

  delay(15);
  byte l = readEEPROM(address);
/*  int cnt = 1000;
  while ((cnt > 0) && (l != odata)) {
    cnt--;
    if ((address & 0xff) == 0)
      l = readEEPROM(address + 1);
    else 
      l = readEEPROM(address - 1);
    l = readEEPROM(address);
  }
  if (cnt == 0){
*/
  if (l != odata){
    char buf[60];
    sprintf(buf, "\r\nWRITE ERROR on %04x! (%02x %02x)\r\n", address, odata, l);
    Serial.println(buf);
    writeErrors++;
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

void writeLabelUncond(boolean command, int labelNumber, int destAddress) {
  writeLabelForNotFlags(0, command, labelNumber, destAddress);
}

void writeLabelForFlags(int flags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
  for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V  | _F_II); f++) {
    if ((f & flags) != 0) {
      writeLabelByte(f, command, labelNumber, destAddress);
      writeLabelByte(f | _F_IR, command, labelNumber, destAddress);
      if (command) {
        writeLabelByte(f | _F_IF, command, labelNumber, fetchAddr);
        if ((f & _F_II) == 0) {
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

void writeLabelForNotFlags(int flags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
  for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V | _F_II); f++) {
    if ((f & flags) == 0) {
      writeLabelByte(f, command, labelNumber, destAddress);
      writeLabelByte(f | _F_IR, command, labelNumber, destAddress);
      if (command) {
        writeLabelByte(f | _F_IF, command, labelNumber, fetchAddr);
        if ((f & _F_II) == 0) {
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
void writeLabelByte(int labelFlags, boolean command, int labelNumber, int destAddress) {
  if (!IS_LABEL_ROM)
    return;
    
  unsigned long labelAddress = labelNumber + (command ? 0 : 256) + ((unsigned long) labelFlags) * 512;
  byte data = (destAddress >> ((ROM_NR - FIRST_LABEL_ROM) * 8)) & 0xff;

  /*
  char buf[80];
  sprintf(buf, "label %04lx: (F%01x C%d N%02x A%04x) %02x", labelAddress, labelFlags, command, labelNumber, destAddress, data);
  Serial.println(buf);
  */

  writeEEPROM4MData(labelAddress, data);
}

uint32_t _goto(int labelNumber) {
  return _IL | (((uint32_t)labelNumber) << 24);
}

void showCommand(char* cmdName, int cmdCode) {
  char buf[60];
  if (strstr(cmdName, ",#") != NULL) {
    sprintf(buf, " %s{v}\t-> 0x%02x @ v[7:0]", cmdName, cmdCode);
  }  
  else if (strstr(cmdName, "addr") != NULL) {
    sprintf(buf, " %s\t-> 0x%02x @ ad[7:0] @ ad[15:8]", cmdName, cmdCode);
    char* ap = strstr(buf, "addr");
    ap[0] = '{';
    ap[1] = 'a';
    ap[2] = 'd';
    ap[3] = '}';
  }
  else if (strstr(cmdName, "val") != NULL) {
    sprintf(buf, " %s\t-> 0x%02x @ v[7:0]", cmdName, cmdCode);
    char* ap = strstr(buf, "val");
    ap[0] = '{';
    ap[1] = 'v';
    ap[2] = '}';
  }
  else {
    sprintf(buf, " %s\t-> 0x%02x", cmdName, cmdCode);
  }
  Serial.println(buf);
}

void setup() {
  // put your setup code code here, to run once:
  Serial.begin(57600);
  char buf[60];
  sprintf(buf, "start ROM #%d", ROM_NR);
  Serial.println(buf);  
  
  unsigned long startMillis = millis();
  
  initPorts();  

  if (IS_LABEL_ROM) {
    Serial.println(F("Erasing Chip..."));
    eraseChip4M();
  }

  int addr = 0;
  int cmd = 0;

  /* RESET */
  writeMicroCodeByte(addr++, 0); /* NOOP */

             /* Ensure Interrupt inhibited */
  writeMicroCodeByte(addr++, _goto(0));   
  writeLabelForNotFlags(_F_II, false, 0, addr);  
  writeMicroCodeByte(addr++, _TI);   
  writeLabelForFlags(_F_II, false, 0, addr);  

           /* read program start from 0xfffc */
  writeMicroCodeByte(addr++, _ALU_255 | _ZW); /* Z-Register = 0xff */
  writeMicroCodeByte(addr++, _FLG_CLC | _FW); /* Clear Carry for Shift */
  writeMicroCodeByte(addr++, _ZE | _NH | _ALU_LSL | _ZW); /* pick NH=0xff, ZW=0xfe */
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW);  /* ZW=0xfc */
  writeMicroCodeByte(addr++, _ZE | _NL); /* pick NL=0xfc */
  writeMicroCodeByte(addr++, _NO | _ME | _CW); /* read low byte to C-Register */

  writeMicroCodeByte(addr++, _ALU_255 | _ZW); /* Z-Register = 0xff */
  writeMicroCodeByte(addr++, _FLG_CLC | _FW); /* Clear Carry for Shift */
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); /* Z-Register = 0xfe */
  writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW); /* Z-Register = 0xfd (C-Flag!) */
  writeMicroCodeByte(addr++, _ZE | _NL); /* pick NL=0xfd */
  writeMicroCodeByte(addr++, _NO | _ME | _DW); /* read high byte to D-Register */
  
  writeMicroCodeByte(addr++, _DE | _NH); /* get high byte from D-Register */
  writeMicroCodeByte(addr++, _CE | _NL); /* get low byte from C-Register */
  writeMicroCodeByte(addr++, _NO | _PI); /* set Progam counter */

            /* Initialize registers to zero */
  writeMicroCodeByte(addr++, _ALU_0 | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH | _NL | _AW | _BW | _CW | _DW | _OW | _FLG_BUS | _FW);  
  writeMicroCodeByte(addr++, _NO | _SI | _SD);    
  writeMicroCodeByte(addr++, _SC | _SD ); /* Stack-Pointer to 0xffff */
           /* fall through to fetch */
           
  /* Fetch */
  fetchAddr = addr;
  writeMicroCodeByte(addr++, _PO | _PC | _ME | _IC | _IL);

  /* Interrupt */
  interruptAddr = addr;
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD | _TI);
  writeMicroCodeByte(addr++, _SC | _SD             | _ALU_255 | _ZW);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD | _FLG_CLC | _FW);
  writeMicroCodeByte(addr++, _SC | _SD             | _ZE | _NH);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD             | _ZE | _ALU_LSL | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _SO | _ZE | _MW);
  writeMicroCodeByte(addr++, _ALU_255 | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NL);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
  writeMicroCodeByte(addr++, _ZE | _NH);
  writeMicroCodeByte(addr++, _SO | _ME | _NL);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  /* NOP */
  int nopAddr = addr;
  showCommand("NOP", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _IC);

  /* HLT */
  showCommand("HLT", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _HC | _IC);

  /* CLC */
  showCommand("CLC", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_CLC | _FW | _IC);

  /* STC */
  showCommand("STC", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FLG_STC | _FW | _IC);

  int iiToggleAddr = addr;
  writeMicroCodeByte(addr++, _TI | _IC);

  /* SII */
  showCommand("SII", cmd);
  writeLabelForFlags(_F_II, true, cmd, nopAddr);
  writeLabelForNotFlags(_F_II, true, cmd++, iiToggleAddr);

  /* CII */
  showCommand("CII", cmd);
  writeLabelForFlags(_F_II, true, cmd, iiToggleAddr);
  writeLabelForNotFlags(_F_II, true, cmd++, nopAddr);

  /* RTS */
  showCommand("RTS", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NH | _SO | _ME);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NL | _SO | _ME);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);

  /* RTI */
  showCommand("RTI", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NH | _SO | _ME);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _NL | _SO | _ME);
  writeMicroCodeByte(addr++, _NO | _PI | _SC);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _TI | _IC);
  
  /* Register */
  int dest = 0;
  int src = 0;
  while (dest < 4) {
    src = 0;
    while (src < 4) {
      if (src != dest) {
        sprintf(buf, "MOV R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[src] | allRegW[dest] | _IC);
        
        sprintf(buf, "ADD R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_ADD | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
        sprintf(buf, "SUB R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_SUB | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
        sprintf(buf, "AND R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_OR | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
        sprintf(buf, "OR  R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_OR | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
        sprintf(buf, "XOR R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
        writeLabelUncond(true, cmd++, addr);
        writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
        writeMicroCodeByte(addr++, allRegE[src]  | _ALU_XOR | _ZW | _FW);
        writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
        sprintf(buf, "CMP R%c,R%c", regName[dest], regName[src]);
        showCommand(buf, cmd);
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
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | allRegW[dest] | _PC | _IC);
        
    sprintf(buf, "ADD R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);
        
    sprintf(buf, "SUB R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);
        
    sprintf(buf, "AND R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);
        
    sprintf(buf, "OR  R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);
        
    sprintf(buf, "XOR R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);
        
    sprintf(buf, "CMP R%c,#", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _PO | _ME | _PC | _ALU_SUB | _FW | _IC);

    dest++; 
  }

  /* absolut */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "MOV R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _PC | _IC);
    
    sprintf(buf, "MOV addr,R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _PC | _IC);

    sprintf(buf, "ADD R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "SUB R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "AND R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "OR  R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "XOR R%c,addr", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
    writeMicroCodeByte(addr++, _PO | _ME | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "CMP R%c,addr", regName[dest]);
    showCommand(buf, cmd);
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
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _IC);
    
    sprintf(buf, "MOV [RCD],R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _IC);

    sprintf(buf, "ADD R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "SUB R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "AND R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "OR  R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "XOR R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _CE | _NL);
    writeMicroCodeByte(addr++, _DE | _NH);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
    writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
        
    sprintf(buf, "CMP R%c,[RCD]", regName[dest]);
    showCommand(buf, cmd);
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
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _OW | _IC);
    dest++; 
  }
  
  showCommand("OUT addr", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _OW | _PC | _IC);
  
  showCommand("OUT [RCD]", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _OW | _IC);

  /* LSL */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "LSL R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_LSL | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
    dest++; 
  }
  
  showCommand("LSL addr", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSL | _ZW | _FW | _PC);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);
  
  showCommand("LSL [RCD]", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSL | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  /* LSR */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "LSR R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _ALU_LSR | _ZW | _FW);
    writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);
    dest++; 
  }
  
  showCommand("LSR addr", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSR | _ZW | _FW | _PC);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);
  
  showCommand("LSR [RCD]", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _ME | _ALU_LSR | _ZW | _FW);
  writeMicroCodeByte(addr++, _NO | _ZE | _MW | _IC);

  /* PSH/PUL */
  dest = 0;
  while (dest < 4) {
    sprintf(buf, "PSH R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, allRegE[dest] | _SO | _MW | _SD);
    writeMicroCodeByte(addr++, _SC | _SD | _IC);
    
    sprintf(buf, "PUL R%c", regName[dest]);
    showCommand(buf, cmd);
    writeLabelUncond(true, cmd++, addr);
    writeMicroCodeByte(addr++, _SC);
    writeMicroCodeByte(addr++, allRegW[dest] | _SO | _ME | _IC);
    
    dest++; 
  }
  
  showCommand("PSF", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD | _IC);
    
  showCommand("PLF", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _SC);
  writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC);

  /* Stackpointer */
  showCommand("MOV SP,RCD", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _SI | _IC);
  
  showCommand("MOV RCD,SP", cmd);
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
  
  showCommand("JMP addr", cmd);
  writeLabelUncond(true, cmd++, jmpAddr);
  
  showCommand("JSR addr", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
  writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _NO | _PI | _SC | _SD | _IC);
  
  showCommand("JCS addr", cmd);
  writeLabelForFlags(_F_C, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, noJmpAddr);

  showCommand("JZS addr", cmd);
  writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, noJmpAddr);

  showCommand("JNS addr", cmd);
  writeLabelForFlags(_F_N, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, noJmpAddr);

  showCommand("JVS addr", cmd);
  writeLabelForFlags(_F_V, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, noJmpAddr);
  
  showCommand("JNC addr", cmd);
  writeLabelForFlags(_F_C, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);
  
  showCommand("JNZ addr", cmd);
  writeLabelForFlags(_F_Z, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);
  
  showCommand("JNN addr", cmd);
  writeLabelForFlags(_F_N, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);
  
  showCommand("JNV addr", cmd);
  writeLabelForFlags(_F_V, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);
  
  /* JMP [RCD] */
  jmpAddr = addr;  
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _NO | _PI | _IC);
  
  noJmpAddr = nopAddr;
  
  showCommand("JMP [RCD]", cmd);
  writeLabelUncond(true, cmd++, jmpAddr);
  
  showCommand("JSR [RCD]", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _CE | _NL);
  writeMicroCodeByte(addr++, _DE | _NH);
  writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _SC | _SD);
  writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
  writeMicroCodeByte(addr++, _NO | _PI | _SC | _SD | _IC);
  
  showCommand("JCS [RCD]", cmd);
  writeLabelForFlags(_F_C, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, noJmpAddr);

  showCommand("JZS [RCD]", cmd);
  writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, noJmpAddr);

  showCommand("JNS [RCD]", cmd);
  writeLabelForFlags(_F_N, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, noJmpAddr);

  showCommand("JVS [RCD]", cmd);
  writeLabelForFlags(_F_V, true, cmd, jmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, noJmpAddr);
  
  showCommand("JNC [RCD]", cmd);
  writeLabelForFlags(_F_C, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);
  
  showCommand("JNZ [RCD]", cmd);
  writeLabelForFlags(_F_Z, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);
  
  showCommand("JNN [RCD]", cmd);
  writeLabelForFlags(_F_N, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);
  
  showCommand("JNV [RCD]", cmd);
  writeLabelForFlags(_F_V, true, cmd, noJmpAddr);
  writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);

  showCommand("OUT val,RA", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
  writeMicroCodeByte(addr++, _AE | _PW | _IC);

  showCommand("INP RA,val", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
  writeMicroCodeByte(addr++, _AW | _PE | _IC);

  showCommand("OUT RB,RA", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _BE | _PS);
  writeMicroCodeByte(addr++, _AE | _PW | _IC);

  showCommand("INP RA,RB", cmd);
  writeLabelUncond(true, cmd++, addr);
  writeMicroCodeByte(addr++, _BE | _PS);
  writeMicroCodeByte(addr++, _AW | _PE | _IC);

  showCommand("MOV RA,addr,RB", cmd);
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

  showCommand("MOV RA,[addr],RB", cmd);
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
  writeMicroCodeByte(addr++, _NO | _ME | _AW);
  writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

  showCommand("MOV addr,RB,RA", cmd);
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

  showCommand("MOV [addr],RB,RA", cmd);
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

  /* set unused op codes to HLT */
  writeMicroCodeByte(addr, _HC);
  while (cmd < 256) {
    writeLabelUncond(true, cmd++, addr);
  }
  addr++;

  if (IS_MC_ROM)
    printContents(0);
  else 
    printContents4M(0);      

  int spentSeconds = (millis()-startMillis) / 1000;
  sprintf(buf, "took %ds ROM#%d", spentSeconds, ROM_NR);
  Serial.println(buf);
  if (writeErrors > 0) {
    sprintf(buf, "\r\nERR: %d\r\n", writeErrors);
    Serial.println(buf);    
  }
  Serial.println(F("done."));
}

void loop() {
  // put your main code here, to run repeatedly:

}
