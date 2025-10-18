package net.derulf.eightbit;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Micro Code Generator for my 8-Bit Computer.
 * 
 * Generates the byte data for all six Micro Code ROMs. Also puts out all
 * #cpudef statements for
 * <a href="https://github.com/hlorenzi/customasm/">customasm</a> onto the
 * console.
 * 
 * @author Ulf Caspers
 *
 */
public class MicroCode {

	private int ROM_NR; // current ROM number being generated
	private boolean IS_LABEL_ROM; // is it a ROM with jump labels?
	private boolean IS_MC_ROM; // is it a ROM with Micro Code

	private byte[] MC_ROM = new byte[4 * 1024 * 1024];
	private boolean[] MC_ROM_W = new boolean[4 * 1024 * 1024];
	private int maxbyte = 0; // last written byte position in the ROM
	private int cntbyte = 0; // current byte write position

	private int LAST_MC_ROM = 3;
	private int FIRST_LABEL_ROM = (LAST_MC_ROM + 1);

	private static long _HC = (long) 1; // Halt Clock
	private static long _IC = ((long) 1 << 1); // Instruction Command
	private static long _IL = ((long) 1 << 2); // Instruction Load
	private static long _PC = ((long) 1 << 3); // ProgramCounter++
	private static long _SC = ((long) 1 << 4); // Stack pointer Change
	private static long _SD = ((long) 1 << 5); // Stackpointer down
	private static long _PS = ((long) 1 << 6); // Port Set number
	private static long _TI = ((long) 1 << 7); // Toggle Interrupt inhibit

	private static long _PO = ((long) 1 << 8); // Program counter On address bus
	private static long _SO = ((long) 1 << 9); // Stack pointer On address bus
	private static long _NO = ((long) 1 << 10); // New address On address bus
	private static long _PI = ((long) 1 << 11); // Program counter In from address bus
	private static long _SI = ((long) 1 << 12); // Stack pointer In from address bus
	private static long _NL = ((long) 1 << 13); // New address Low byte from D-Bus
	private static long _NH = ((long) 1 << 14); // New address High byte from D-Bus
	private static long _PW = ((long) 1 << 15); // Port Write byte from D-Bus

	private static long _NC = (_NL | _NH); // Count NewAddress

	private static long _MW = ((long) 1 << 16); // Memory Write from D-Bus
	private static long _AW = ((long) 1 << 17); // register A Write from D-Bus
	private static long _BW = ((long) 1 << 18); // register B Write from D-Bus
	private static long _CW = ((long) 1 << 19); // register C Write from D-Bus
	private static long _DW = ((long) 1 << 20); // register D Write from D-Bus
	private static long _OW = ((long) 1 << 21); // Out register Write from D-Bus
	private static long _ZW = ((long) 1 << 22); // ALU register Write
	private static long _FW = ((long) 1 << 23); // ALU Flag register Write

	// D-Bus enable bits
	private static long __E0 = ((long) 1 << 24);
	private static long __E1 = ((long) 1 << 25);
	private static long __E2 = ((long) 1 << 26);
	private static long __E3 = ((long) 1 << 27);

	// D-Bus enable control statements
	private static long _P1 = ((long) 1 << 24);
	private static long _P2 = ((long) 2 << 24);
	private static long _S1 = ((long) 3 << 24);
	private static long _S2 = ((long) 4 << 24);
	private static long _N1 = ((long) 5 << 24);
	private static long _N2 = ((long) 6 << 24);
	private static long _ME = ((long) 7 << 24);
	private static long _AE = ((long) 8 << 24);
	private static long _BE = ((long) 9 << 24);
	private static long _CE = ((long) 10 << 24);
	private static long _DE = ((long) 11 << 24);
	private static long _ZE = ((long) 12 << 24);
	private static long _FE = ((long) 13 << 24);
	private static long _PE = ((long) 14 << 24);
	// private static long =((long)15 << 24);

	// ALU function bits
	private static long _ZS = ((long) 1 << 28);
	private static long _Z0 = ((long) 1 << 29);
	private static long _Z1 = ((long) 1 << 30);
	private static long _Z2 = ((long) 1 << 31);

	// ALU functions
	private static long _ALU_0 = ((long) 0);
	private static long _ALU_SUB = (_Z0);
	private static long _ALU_SBI = (_Z1);
	private static long _ALU_ADD = (_Z0 | _Z1);
	private static long _ALU_XOR = (_Z2);
	private static long _ALU_OR = (_Z0 | _Z2);
	private static long _ALU_AND = (_Z1 | _Z2);
	private static long _ALU_255 = (_Z0 | _Z1 | _Z2);
	private static long _ALU_BUS = (_Z0 | _ZS);
	private static long _ALU_LSL = (_Z1 | _ZS);
	private static long _ALU_LSR = (_Z0 | _Z1 | _ZS);

	// ALU Flag functions
	private static long _FLG_BUS = (_Z0 | _ZS);
	private static long _FLG_CLC = (_Z2 | _ZS);
	private static long _FLG_STC = (_Z0 | _Z2 | _ZS);

	// CPU State Flags
	private static int _F_C = ((int) 1);
	private static int _F_Z = ((int) 2);
	private static int _F_N = ((int) 4);
	private static int _F_V = ((int) 8);
	private static int _F_II = ((int) 16); /* 1 = interrupt inhibited */
	private static int _F_IR = ((int) 32); /* 1 = interrupt requested */
	private static int _F_IF = ((int) 64); /* 1 = instruction fetch */
	private static int _F_EC = ((int) 128); /* 1 = extended command */

	// all active low control lines
	private static long _negativ = (_IC | _IL | _SC | _SD | _PS | _PO | _SO | _NO | _PI | _PW | _MW | _AW | _BW | _CW
			| _DW | _OW | _ZW | _FW);

	private int fetchAddr = 0; // Micro Code address of the fetch code
	private int interruptAddr = 0; // Micro Code address of the interrupt code
	private int nopAddr = 0; // Micro Code address of the NOP code

	// all register names
	private char regName[] = { 'A', 'B', 'C', 'D' };
	// all register write control lines
	private long allRegW[] = { _AW, _BW, _CW, _DW };
	// all register enable control lines
	private long allRegE[] = { _AE, _BE, _CE, _DE };

	/**
	 * Sets the number of the ROM to generate.
	 * 
	 * @param pRomNr ROM number (0 to 5)
	 */
	public void setRomNr(int pRomNr) {
		ROM_NR = pRomNr;
		IS_LABEL_ROM = (ROM_NR >= FIRST_LABEL_ROM);
		IS_MC_ROM = !IS_LABEL_ROM;
	}

	/**
	 * Write a byte of a micro code step.
	 *
	 * @param address address in EEPROM
	 * @param code    the full micro code step
	 */
	private void writeMicroCodeByte(int address, long code) {
		if (!IS_MC_ROM)
			return;

		long inv = code ^ _negativ;
		byte data = (byte) ((inv >> (ROM_NR * 8)) & 0xff);

		/*
		 * char buf[80]; sprintf(buf, "Out %04x: (%08lx, %08lx) %02x", address, code,
		 * inv, data); Serial.println(buf);
		 */

		writeEEPROM(address, data);
	}

	/**
	 * Write all bytes for an unconditional label into the label EEPROM.
	 * 
	 * @param command     <code>true</code>/<code>false</code> whether or not this
	 *                    label is for an instruction
	 * @param labelNumber 0-255 number of the label
	 * @param destAddress micro code address this label points to
	 */
	private void writeLabelUncond(boolean command, int labelNumber, int destAddress) {
		if (!IS_LABEL_ROM)
			return;
		writeLabelForNotFlags(0, command, labelNumber, destAddress);
	}

	/**
	 * Write all bytes for an conditional label into the label EEPROM.
	 * 
	 * @param flags       ORed list of flags that MUST be set for this label to be
	 *                    active
	 * @param command     <code>true</code>/<code>false</code> whether or not this
	 *                    label is for an instruction
	 * @param labelNumber 0-255 number of the label
	 * @param destAddress micro code address this label points to
	 */
	private void writeLabelForFlags(int flags, boolean command, int labelNumber, int destAddress) {
		if (!IS_LABEL_ROM)
			return;
		for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V | _F_II); f++) {
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

	/**
	 * Write all bytes for an conditional label into the label EEPROM.
	 * 
	 * @param flags       ORed list of flags that MUST NOT be set for this label to
	 *                    be active
	 * @param command     <code>true</code>/<code>false</code> whether or not this
	 *                    label is for an instruction
	 * @param labelNumber 0-255 number of the label, 0-511 number of the command
	 * @param destAddress micro code address this label points to
	 */
	private void writeLabelForNotFlags(int flags, boolean command, int labelNumber, int destAddress) {
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

	/**
	 * writes a byte into the label EEPROM.
	 * 
	 * @param labelFlags  exact combination of active flags for which this label is
	 *                    valid
	 * @param command     <code>true</code>/<code>false</code> whether or not this
	 *                    label is for an instruction
	 * @param labelNumber 0-255 number of the label, 0-511 number of the command
	 * @param destAddress micro code address this label points to
	 */
	private void writeLabelByte(int labelFlags, boolean command, int labelNumber, int destAddress) {
		if (!IS_LABEL_ROM)
			return;

		int labelAddress = (labelNumber & 0xff) + (command ? 0 : 256) + (labelFlags) * 512;
		// extended command
		if (labelNumber > 255) {
			labelAddress += (_F_EC) * 512;
		}
		byte data = (byte) ((destAddress >> ((ROM_NR - FIRST_LABEL_ROM) * 8)) & 0xff);

		/*
		 * char buf[80]; sprintf(buf, "label %04lx: (F%01x C%d N%02x A%04x) %02x",
		 * labelAddress, labelFlags, command, labelNumber, destAddress, data);
		 * Serial.println(buf);
		 */

		writeEEPROM(labelAddress, data);
	}

	/**
	 * Write a byte into a Micro Code EEPROM.
	 * 
	 * @param address address in EEPROM
	 * @param data    data byte
	 */
	private void writeEEPROM(int address, byte data) {
		MC_ROM[address] = data;
		if (address > maxbyte)
			maxbyte = address;
		cntbyte++;
		if (MC_ROM_W[address])
			System.err.println("Doppelt: " + address);
		else
			MC_ROM_W[address] = true;
	};

	/**
	 * Calculate the micro code for a micro code go to label.
	 * 
	 * @param labelNumber label number
	 * @return micro code
	 */
	private long _goto(int labelNumber) {
		if (labelNumber % 16 == _PE) {
			System.err.println("PortEnable darf nicht Zieldresse sein!");
			System.exit(1);
		}
		return _IL | (((long) labelNumber) << 24);
	}

	/**
	 * Output a command definition.
	 * 
	 * @param cmdName     assembler mnemonic for this command
	 * @param pcmdCode    instruction code byte for this command
	 * @param destAddress address in Micro Code EEPROM
	 */
	private void showCommand(String cmdName, int pcmdCode, int destAddress) {
		int cmdCode = pcmdCode & 0xff;
		String buf;

		String extCmd;
		if (pcmdCode > 255) {
			extCmd = " 0xff @";
		} else {
			extCmd = "";
		}
		/*
		 * if (cmdName.contains(", #")) { buf =
		 * String.format(" %s{v}\t=>%s 0x%02x @ v[7:0] ; 0x%03x", cmdName, extCmd,
		 * cmdCode, destAddress); } else
		 */ if (cmdName.contains("addr")) {
			buf = String.format(" %s\t=>%s 0x%02x @ ad[7:0] @ ad[15:8] ; 0x%03x", cmdName, extCmd, cmdCode,
					destAddress);
			buf = buf.replace("addr", "{ad}");
		} else if (cmdName.contains("val")) {
			buf = String.format(" %s\t=>%s 0x%02x @ v[7:0] ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
			buf = buf.replace("val", "{v}");
		} else {
			buf = String.format(" %s\t=>%s 0x%02x ; 0x%03x", cmdName, extCmd, cmdCode, destAddress);
		}
		System.out.println(buf);
	}

	/**
	 * Generates the data for a single Micro Code ROM.
	 * 
	 * @param pRomNr   specific ROM number (0 to 5)
	 * @param pOutFile Output file
	 * @throws IOException Execption while writing the output file
	 */
	private void generate(int pRomNr, File pOutFile) throws IOException {
		setRomNr(pRomNr);

		int addr = 0;
		int cmd = 0;

		/* mark all bytes as "unwitten" */
		for (int i = 0; i < MC_ROM_W.length; i++) {
			MC_ROM_W[i] = false;
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
		final int LAB_RESET_II = 0;
		writeMicroCodeByte(addr++, _goto(LAB_RESET_II));
		writeLabelForNotFlags(_F_II, false, LAB_RESET_II, addr);
		writeMicroCodeByte(addr++, _TI);
		writeLabelForFlags(_F_II, false, LAB_RESET_II, addr);

		writeMicroCodeByte(addr++, _ZE | _AW | _BW | _CW | _DW | _IC);
		/* fall through to fetch */

		/* Fetch */
		fetchAddr = addr;
		writeMicroCodeByte(addr++, _PO | _PC | _ME | _IC | _IL);

		/* Interrupt */
		interruptAddr = addr;
		writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _SC | _SD | _ZE | _NH);
		writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
		writeMicroCodeByte(addr++, _SC | _SD | _ZE | _NL);
		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
		writeMicroCodeByte(addr++, _SC | _SD | _NO | _ME | _ALU_BUS | _ZW | _SD | _NC);
		writeMicroCodeByte(addr++, _NO | _ME | _NL);
		writeMicroCodeByte(addr++, _ZE | _NH | _TI);
		writeMicroCodeByte(addr++, _NO | _PI | _IC);

//		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _SC | _SD | _ALU_255 | _ZW);
//		writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD | _FLG_CLC | _FW);
//		writeMicroCodeByte(addr++, _SC | _SD | _ZE | _NH);
//		writeMicroCodeByte(addr++, _P2 | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _SC | _SD | _ZE | _ALU_LSL | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NL);
//		writeMicroCodeByte(addr++, _NO | _PI);
//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _TI);
//		writeMicroCodeByte(addr++, _NO | _PI | _IC);

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
		writeMicroCodeByte(addr++, _NH | _SO | _ME | _SC);
		writeMicroCodeByte(addr++, _NL | _SO | _ME);
		writeMicroCodeByte(addr++, _NO | _PI | _IC);

		/* RTI */
		showCommand("RTI", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SC);
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _SC);
		writeMicroCodeByte(addr++, _NH | _SO | _ME | _SC);
		writeMicroCodeByte(addr++, _NL | _SO | _ME | _TI);
		writeMicroCodeByte(addr++, _NO | _PI | _IC);

		/* Register */
		int dest = 0;
		int src = 0;
		while (dest < 4) {
			src = 0;
			while (src < 4) {
				if (src != dest) {
					showCommand(String.format("MOV R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[src] | allRegW[dest] | _IC);

					showCommand(String.format("ADD R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_ADD | _ZW | _FW);
					writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

					showCommand(String.format("SUB R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_SUB | _ZW | _FW);
					writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

					showCommand(String.format("AND R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_AND | _ZW | _FW);
					writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

					showCommand(String.format("OR  R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_OR | _ZW | _FW);
					writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

					showCommand(String.format("XOR R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_XOR | _ZW | _FW);
					writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

					showCommand(String.format("CMP R%c, R%c", regName[dest], regName[src]), cmd, addr);
					writeLabelUncond(true, cmd++, addr);
					writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
					writeMicroCodeByte(addr++, allRegE[src] | _ALU_SUB | _FW | _IC);
				}
				src++;
			}
			dest++;
		}

		/* immediate */
		dest = 0;
		while (dest < 4) {
			showCommand(String.format("MOV R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | allRegW[dest] | _PC | _IC);

			showCommand(String.format("ADD R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_ADD | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

			showCommand(String.format("SUB R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_SUB | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

			showCommand(String.format("AND R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_AND | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

			showCommand(String.format("OR  R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_OR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

			showCommand(String.format("XOR R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_XOR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _PC | _IC);

			showCommand(String.format("CMP R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _PO | _ME | _PC | _ALU_SUB | _FW | _IC);

			dest++;
		}

		/* absolut */
		dest = 0;
		while (dest < 4) {
			showCommand(String.format("MOV R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _PC | _IC);

			showCommand(String.format("MOV addr, R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _PC | _IC);

			showCommand(String.format("ADD R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("SUB R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("AND R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("OR  R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("XOR R%c, addr", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
			writeMicroCodeByte(addr++, _PO | _ME | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW | _PC);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("CMP R%c, addr", regName[dest]), cmd, addr);
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
			showCommand(String.format("MOV R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, _NO | _ME | allRegW[dest] | _IC);

			showCommand(String.format("MOV [RCD], R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, _NO | _MW | allRegE[dest] | _IC);

			showCommand(String.format("ADD R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("SUB R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_SUB | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("AND R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_AND | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("OR  R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_OR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("XOR R%c, [RCD]", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _CE | _NL);
			writeMicroCodeByte(addr++, _DE | _NH);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_BUS | _ZW);
			writeMicroCodeByte(addr++, _NO | _ME | _ALU_XOR | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("CMP R%c, [RCD]", regName[dest]), cmd, addr);
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
			showCommand(String.format("OUT R%c", regName[dest]), cmd, addr);
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
			showCommand(String.format("LSL R%c", regName[dest]), cmd, addr);
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
			showCommand(String.format("LSR R%c", regName[dest]), cmd, addr);
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
			showCommand(String.format("PSH R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, allRegE[dest] | _SO | _MW | _SD);
			writeMicroCodeByte(addr++, _SC | _SD | _IC);

			showCommand(String.format("PUL R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _SC);
			writeMicroCodeByte(addr++, allRegW[dest] | _SO | _ME | _IC);

			dest++;
		}

		/* Push Flags */
		showCommand("PSF", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
		writeMicroCodeByte(addr++, _SC | _SD | _IC);

		/* Pull Flags */
		final int LAB_PLF_II = 1;
		showCommand("PLF", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SC | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _SO | _ME | _ALU_OR | _FW);
		writeMicroCodeByte(addr++, _goto(LAB_PLF_II));
		int toggleAddr = addr;
		writeMicroCodeByte(addr++, _TI);
		int noToggleAddr = addr;
		if (IS_LABEL_ROM) {
			for (int f = 0; f <= (_F_C | _F_Z | _F_N | _F_V | _F_II | _F_IR | _F_IF); f++) {
				if ((f & (_F_N | _F_II)) == 0) {
					writeLabelByte(f, false, LAB_PLF_II, noToggleAddr);
					writeLabelByte(f | _F_N, false, LAB_PLF_II, toggleAddr);
					writeLabelByte(f | _F_II, false, LAB_PLF_II, toggleAddr);
					writeLabelByte(f | _F_N | _F_II, false, LAB_PLF_II, noToggleAddr);
				}
			}
		}
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC);

		/* Stackpointer */
		showCommand("MOV SP, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL);
		writeMicroCodeByte(addr++, _DE | _NH);
		writeMicroCodeByte(addr++, _NO | _SI | _IC);

		showCommand("MOV RCD, SP", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _S1 | _CW);
		writeMicroCodeByte(addr++, _S2 | _DW | _IC);

		/* JMP addr */
		int jmpAddr = addr;
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH);
		writeMicroCodeByte(addr++, _NO | _PI | _IC);

		int skipTwoBytesAddr = addr;
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
		writeLabelForNotFlags(_F_C, true, cmd++, skipTwoBytesAddr);

		showCommand("JZS addr", cmd, jmpAddr);
		writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_Z, true, cmd++, skipTwoBytesAddr);

		showCommand("JNS addr", cmd, jmpAddr);
		writeLabelForFlags(_F_N, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_N, true, cmd++, skipTwoBytesAddr);

		showCommand("JVS addr", cmd, jmpAddr);
		writeLabelForFlags(_F_V, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_V, true, cmd++, skipTwoBytesAddr);

		showCommand("JNC addr", cmd, jmpAddr);
		writeLabelForFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);

		showCommand("JNZ addr", cmd, jmpAddr);
		writeLabelForFlags(_F_Z, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);

		showCommand("JNN addr", cmd, jmpAddr);
		writeLabelForFlags(_F_N, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);

		showCommand("JNV addr", cmd, jmpAddr);
		writeLabelForFlags(_F_V, true, cmd, skipTwoBytesAddr);
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

		/* Port Input/Output */
		showCommand("OUT #val, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
		writeMicroCodeByte(addr++, _AE | _PW | _IC);

		showCommand("INP RA, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC);
		writeMicroCodeByte(addr++, _AW | _PE | _IC);

		showCommand("OUT RB, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _BE | _PS);
		writeMicroCodeByte(addr++, _AE | _PW | _IC);

		showCommand("INP RA, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _BE | _PS);
		writeMicroCodeByte(addr++, _AW | _PE | _IC);

		/* Indexed addressing */
		showCommand("MOV RA, addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
		writeMicroCodeByte(addr++, _NH | _ZE | _PC);
		writeMicroCodeByte(addr++, _NO | _ME | _AW);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV RA, [addr], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);

		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH);
		writeMicroCodeByte(addr++, _NO | _ME | _AW);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

//		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL | _SC);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH);
//		writeMicroCodeByte(addr++, _NO | _ME | _AW);
//		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV addr, RB, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
		writeMicroCodeByte(addr++, _NH | _ZE | _PC);
		writeMicroCodeByte(addr++, _NO | _MW | _AE);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV [addr], RB, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH);
		writeMicroCodeByte(addr++, _NO | _MW | _AE);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

//		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL | _SC);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH);
//		writeMicroCodeByte(addr++, _NO | _MW | _AE);
//		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		/* set unused op codes to HLT */
		System.out.println("; Writing unsused codes");

		writeMicroCodeByte(addr, _HC);
		while (cmd < 0xFF) {
			writeLabelUncond(true, cmd++, addr);
		}
		addr++;

		System.out.println("; Writing extended commands");
		/* extended Command always at command 0xFF */
		writeLabelUncond(true, cmd++, fetchAddr);

		/* SPI Bus read Byte */
		final int LAB_INB_LOOP = 2;

		showCommand("INB RA, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _FLG_STC | _FW);
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
		writeMicroCodeByte(addr++, _ZE | _AW | _ALU_LSL | _ZW); // A=1; Z=2
		writeMicroCodeByte(addr++, _ZE | _NL); // NL=2
		writeMicroCodeByte(addr++, _PE | _ALU_OR | _ZW); // Z=SPI, CLK=1
		writeMicroCodeByte(addr++, _AE | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI=1
		writeMicroCodeByte(addr++, _ZE | _NH); // NH=SPI, CLK=1, MOSI=1
		writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=1
		writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=1

		writeLabelForNotFlags(_F_C, false, LAB_INB_LOOP, addr);
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c_flg=MISO
		writeMicroCodeByte(addr++, _AE | _ALU_LSL | _ZW | _FW); // Z=A<-MISO; C-Flg=<-A
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _AW); // A=Z
		writeMicroCodeByte(addr++, _goto(LAB_INB_LOOP));

		writeLabelForFlags(_F_C, false, LAB_INB_LOOP, addr);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW | _IC);

		/* SPI Bus read Buffer */
		final int LAB_INB_OLOOP = 3;
		final int LAB_INB_ILOOP = 4; /* no longer needed due to loop unrolling */
		showCommand("INB [RCD], RB, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC); // Port selector
		writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD); // save PC to stack
		writeMicroCodeByte(addr++, _SC | _SD); // save PC to stack
		writeMicroCodeByte(addr++, _P2 | _SO | _MW); // save PC to stack

		writeMicroCodeByte(addr++, _CE | _NL); // dest buffer low
		writeMicroCodeByte(addr++, _DE | _NH | _ALU_0 | _ZW); // dest buffer high
		writeMicroCodeByte(addr++, _NO | _PI | _FLG_STC | _FW); // dest buffer -> PC

		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
		writeMicroCodeByte(addr++, _ZE | _DW | _ALU_LSL | _ZW); // D=1; Z=2
		writeMicroCodeByte(addr++, _ZE | _NL); // NL=2
		writeMicroCodeByte(addr++, _PE | _ALU_OR | _ZW); // Z=SPI, CLK=1
		writeMicroCodeByte(addr++, _DE | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI=1
		writeMicroCodeByte(addr++, _ZE | _NH); // NH=SPI, CLK=1, MOSI=1
		writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=1
		writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=1

		writeLabelForNotFlags(_F_Z, false, LAB_INB_OLOOP, addr);

		// Bit 7
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 6
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 5
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 4
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 3
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 2
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 1
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z
		// Bit 0
		writeMicroCodeByte(addr++, _N2 | _PW); // SPI CLK=1
		writeMicroCodeByte(addr++, _PE | _ALU_LSL | _FW); // c-Flg=MISO
		writeMicroCodeByte(addr++, _N1 | _PW); // SPI CLK=0
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=Z<-MISO; C-Flg=<-Z

		writeMicroCodeByte(addr++, _ZE | _PO | _MW | _FLG_STC | _FW); // save received byte
		writeMicroCodeByte(addr++, _PC | _BE | _ALU_BUS | _ZW); // Z=B
		writeMicroCodeByte(addr++, _DE | _ALU_SUB | _ZW | _FW); // Z=B-1, Z-Flg=(B==0)
		writeMicroCodeByte(addr++, _ZE | _BW); // B=B-1
		writeMicroCodeByte(addr++, _goto(LAB_INB_OLOOP));

		writeLabelForFlags(_F_Z, false, LAB_INB_OLOOP, addr);
		writeMicroCodeByte(addr++, _P1 | _CW);
		writeMicroCodeByte(addr++, _P2 | _DW);
		writeMicroCodeByte(addr++, _SO | _ME | _NH);
		writeMicroCodeByte(addr++, _SC);
		writeMicroCodeByte(addr++, _SO | _ME | _NL);
		writeMicroCodeByte(addr++, _NO | _PI | _FLG_CLC | _FW | _IC);

		/* SPI Bus write Byte */
		showCommand("OUB #val, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC | _ALU_0 | _ZW); // port selection
		writeMicroCodeByte(addr++, _CE | _SO | _MW | _FLG_STC | _FW); // c register on stack
		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
		writeMicroCodeByte(addr++, _ZE | _NL | _CW | _ALU_LSL | _ZW); // NL=1;C=1; Z=2
		writeMicroCodeByte(addr++, _ZE | _NH); // NH=2 (=>CLK=1)
		writeMicroCodeByte(addr++, _PE | _ALU_OR | _ZW); // Z=SPI, CLK=1
		writeMicroCodeByte(addr++, _N1 | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI=1
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
		final int LAB_OUB_OLOOP = 6;
		final int LAB_OUB_ILOOP = 7;
		showCommand("OUB #val, [RCD], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _PS | _PC); // Port selector
		writeMicroCodeByte(addr++, _P1 | _SO | _MW | _SD); // save PC to stack
		writeMicroCodeByte(addr++, _SC | _SD); // save PC to stack
		writeMicroCodeByte(addr++, _P2 | _SO | _MW); // save PC to stack

		writeMicroCodeByte(addr++, _CE | _NL); // src buffer low
		writeMicroCodeByte(addr++, _DE | _NH | _ALU_0 | _ZW); // src buffer high
		writeMicroCodeByte(addr++, _NO | _PI | _FLG_STC | _FW); // src buffer -> PC

		writeMicroCodeByte(addr++, _ZE | _ALU_LSL | _ZW | _FW); // Z=1; C-Flg=0
		writeMicroCodeByte(addr++, _ZE | _NL | _DW | _ALU_LSL | _ZW); // NL=1;D=1; Z=2
		writeMicroCodeByte(addr++, _ZE | _NH); // NH=2 (=>CLK=1)
		writeMicroCodeByte(addr++, _PE | _ALU_OR | _ZW); // Z=SPI, CLK=1
		writeMicroCodeByte(addr++, _N1 | _ALU_OR | _ZW); // Z=SPI, CLK=1, MOSI=1
		writeMicroCodeByte(addr++, _N1 | _ALU_XOR | _ZW); // Z=SPI, CLK=1, MOSI=0
		writeMicroCodeByte(addr++, _N2 | _ALU_XOR | _ZW); // Z=SPI, CLK=0, MOSI=0
		writeMicroCodeByte(addr++, _ZE | _NL); // NL=SPI, CLK=0, MOSI=0

		writeLabelForNotFlags(_F_Z, false, LAB_OUB_OLOOP, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _AW); // next byte to send
		writeMicroCodeByte(addr++, _DE | _CW | _PC); // c=1

		writeLabelForNotFlags(_F_C, false, LAB_OUB_ILOOP, addr);
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
		writeMicroCodeByte(addr++, _goto(LAB_OUB_ILOOP)); // c>255 (c-flag set?)

		writeLabelForFlags(_F_C, false, LAB_OUB_ILOOP, addr);
		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // Z=B
		writeMicroCodeByte(addr++, _DE | _ALU_SUB | _ZW | _FW); // Z=B-1, Z-Flg=(B==0)
		writeMicroCodeByte(addr++, _ZE | _BW); // B=B-1
		writeMicroCodeByte(addr++, _goto(LAB_OUB_OLOOP));

		writeLabelForFlags(_F_Z, false, LAB_OUB_OLOOP, addr);
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
		writeLabelForNotFlags(_F_C, true, cmd++, skipTwoBytesAddr);

		showCommand("JZS [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_Z, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_Z, true, cmd++, skipTwoBytesAddr);

		showCommand("JNS [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_N, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_N, true, cmd++, skipTwoBytesAddr);

		showCommand("JVS [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_V, true, cmd, jmpAddr);
		writeLabelForNotFlags(_F_V, true, cmd++, skipTwoBytesAddr);

		showCommand("JNC [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, jmpAddr);

		showCommand("JNZ [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_Z, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_Z, true, cmd++, jmpAddr);

		showCommand("JNN [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_N, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_N, true, cmd++, jmpAddr);

		showCommand("JNV [addr]", cmd, jmpAddr);
		writeLabelForFlags(_F_V, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_V, true, cmd++, jmpAddr);

		/* INC/DEC */
		int incAddr;
		int decAddr;
		dest = 0;
		while (dest < 4) {
			showCommand(String.format("INC R%c", regName[dest]), cmd, addr);
			writeLabelForNotFlags(_F_C, true, cmd, addr);
			writeMicroCodeByte(addr++, _FLG_STC | _FW);
			incAddr = addr;
			writeLabelForFlags(_F_C, true, cmd++, addr);
			writeMicroCodeByte(addr++, _ALU_0 | _ZW);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_ADD | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("ICC R%c", regName[dest]), cmd, incAddr);
			writeLabelForNotFlags(_F_C, true, cmd, nopAddr);
			writeLabelForFlags(_F_C, true, cmd++, incAddr);

			showCommand(String.format("DEC R%c", regName[dest]), cmd, addr);
			writeLabelForFlags(_F_C, true, cmd, addr);
			writeMicroCodeByte(addr++, _FLG_CLC | _FW);
			decAddr = addr;
			writeLabelForNotFlags(_F_C, true, cmd++, addr);
			writeMicroCodeByte(addr++, _ALU_0 | _ZW);
			writeMicroCodeByte(addr++, allRegE[dest] | _ALU_SBI | _ZW | _FW);
			writeMicroCodeByte(addr++, allRegW[dest] | _ZE | _IC);

			showCommand(String.format("DCC R%c", regName[dest]), cmd, decAddr);
			writeLabelForFlags(_F_C, true, cmd, nopAddr);
			writeLabelForNotFlags(_F_C, true, cmd++, decAddr);

			dest++;
		}

		showCommand("INC RCD", cmd, addr);
		writeLabelForNotFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_STC | _FW);
		writeLabelForFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _CE | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _CW | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _DE | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _DW | _ZE | _IC);

		showCommand("DEC RCD", cmd, addr);
		writeLabelForFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeLabelForNotFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _CE | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _CW | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _DE | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _DW | _ZE | _IC);

		showCommand("INC addr", cmd, addr);
		writeLabelForNotFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_STC | _FW);
		incAddr = addr;
		writeLabelForFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("ICC addr", cmd, incAddr);
		writeLabelForNotFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForFlags(_F_C, true, cmd++, incAddr);

		showCommand("DEC addr", cmd, addr);
		writeLabelForFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		decAddr = addr;
		writeLabelForNotFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("DCC addr", cmd, decAddr);
		writeLabelForFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, decAddr);

		showCommand("INC addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		incAddr = addr;
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
		writeMicroCodeByte(addr++, _NH | _ZE | _PC | _FLG_STC | _FW);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("ICC addr, RB", cmd, addr);
		writeLabelForNotFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForFlags(_F_C, true, cmd++, incAddr);

		showCommand("DEC addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		decAddr = addr;
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW | _PC);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW);
		writeMicroCodeByte(addr++, _NH | _ZE | _PC | _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("DCC addr, RB", cmd, addr);
		writeLabelForFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, decAddr);

		showCommand("INC [addr], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		incAddr = addr;
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC);
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _NH);
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ZW | _ALU_ADD);
		writeMicroCodeByte(addr++, _ZE | _NH | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _FLG_STC | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("ICC [addr], RB", cmd, addr);
		writeLabelForNotFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForFlags(_F_C, true, cmd++, incAddr);

//		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL | _SC);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH);
//		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("DEC [addr], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		decAddr = addr;
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC);
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _NH);
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ZW | _ALU_ADD);
		writeMicroCodeByte(addr++, _ZE | _NH | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH | _FLG_CLC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("DCC [addr], RB", cmd, addr);
		writeLabelForFlags(_F_C, true, cmd, skipTwoBytesAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, decAddr);

//		writeMicroCodeByte(addr++, _FE | _SO | _MW | _SD);
//		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC | _SD | _SC);
//		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _SO | _MW | _FLG_STC | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N1 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NL | _ZE);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _NH | _ZE);
//		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW);
//		writeMicroCodeByte(addr++, _SO | _ME | _ALU_ADD | _ZW | _FW);
//		writeMicroCodeByte(addr++, _ZE | _NL | _SC);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW);
//		writeMicroCodeByte(addr++, _ZE | _NH);
//		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW);
//		writeMicroCodeByte(addr++, _ALU_0 | _ZW);
//		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
//		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		/* move 16-bit address into RC/RD register pair */
		showCommand("MVA RCD, addr", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _CW | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _DW | _PC | _IC);

		/* INC/DEC indirect */
		showCommand("INC [RCD]", cmd, addr);
		writeLabelForNotFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_STC | _FW);
		incAddr = addr;
		writeLabelForFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _DE | _NH);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_ADD | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("ICC [RCD]", cmd, incAddr);
		writeLabelForNotFlags(_F_C, true, cmd, nopAddr);
		writeLabelForFlags(_F_C, true, cmd++, incAddr);

		showCommand("DEC [RCD]", cmd, addr);
		writeLabelForFlags(_F_C, true, cmd, addr);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		decAddr = addr;
		writeLabelForNotFlags(_F_C, true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _DE | _NH);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _IC);

		showCommand("DCC [RCD]", cmd, decAddr);
		writeLabelForFlags(_F_C, true, cmd, nopAddr);
		writeLabelForNotFlags(_F_C, true, cmd++, decAddr);

		/* Push/Pull memory block onto/from Stack */
		final int LAB_PSB_LOOP = 8;
		showCommand("PSB addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC);

		writeLabelForNotFlags(_F_Z, false, LAB_PSB_LOOP, addr);
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _SD);
		writeMicroCodeByte(addr++, _SO | _MW | _ZE | _SD | _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW | _NC | _SC | _SD);
		writeMicroCodeByte(addr++, _BE | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _ZE | _BW);
		writeMicroCodeByte(addr++, _goto(LAB_PSB_LOOP));

		writeLabelForFlags(_F_Z, false, LAB_PSB_LOOP, addr);
		writeMicroCodeByte(addr++, _IC);

		final int LAB_PLB_LOOP = 9;
		showCommand("PLB addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _SC);

		writeLabelForNotFlags(_F_Z, false, LAB_PLB_LOOP, addr);
		writeMicroCodeByte(addr++, _SO | _ME | _ALU_BUS | _ZW);
		writeMicroCodeByte(addr++, _NO | _MW | _ZE | _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _ALU_0 | _ZW | _NC | _SC);
		writeMicroCodeByte(addr++, _BE | _ALU_SBI | _ZW | _FW);
		writeMicroCodeByte(addr++, _ZE | _BW);
		writeMicroCodeByte(addr++, _goto(LAB_PLB_LOOP) | _SD);

		writeLabelForFlags(_F_Z, false, LAB_PLB_LOOP, addr);
		writeMicroCodeByte(addr++, _SD | _SC | _IC);

		/* Stack content manipulation commands */
		dest = 0;
		while (dest < 4) {

			/* Peek Stack with fixed offset */
			showCommand(String.format("STR R%c, #val", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
			writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
			writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
			writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NH); // store high address
			writeMicroCodeByte(addr++, _NO | _SO | _ME | allRegW[dest]); // load value
			writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

			/* Peek Stack with RB offset */
			showCommand(String.format("STR R%c, RB", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
			writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
			writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
			writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
			writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NH); // store high address
			writeMicroCodeByte(addr++, _NO | _SO | _ME | allRegW[dest]); // load value
			writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

			/* Poke Stack with fixed offset */
			showCommand(String.format("STW #val, R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
			writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
			writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
			writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
			writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NH); // store high address
			writeMicroCodeByte(addr++, _NO | _SO | _MW | allRegE[dest]); // save value
			writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

			/* Poke Stack with RB offset */
			showCommand(String.format("STW RB, R%c", regName[dest]), cmd, addr);
			writeLabelUncond(true, cmd++, addr);
			writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
			writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
			writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
			writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
			writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
			writeMicroCodeByte(addr++, _ZE | _NH); // store high address
			writeMicroCodeByte(addr++, _NO | _SO | _MW | allRegE[dest]); // save value
			writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state
			dest++;
		}

		/* 16 Bit Peek Stack with fixed offset */
		showCommand("STR RCD, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SO | _ME | _CW | _NC); // load low value
		writeMicroCodeByte(addr++, _NO | _SO | _ME | _DW); // load high value
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

		/* 16 Bit Peek Stack with RB offset */
		showCommand("STR RCD, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SO | _ME | _CW | _NC); // load low value
		writeMicroCodeByte(addr++, _NO | _SO | _ME | _DW); // load high value
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

		/* 16 Bit Poke Stack with fixed offset */
		showCommand("STW #val, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SO | _MW | _CE | _NC); // save low value
		writeMicroCodeByte(addr++, _NO | _SO | _MW | _DE); // save high value
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

		/* 16 Bit Poke Stack with RB offset */
		showCommand("STW RB, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _SO | _MW | _FE); // save current flag state
		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // always at least one byte forward
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SO | _MW | _CE | _NC); // save low value
		writeMicroCodeByte(addr++, _NO | _SO | _MW | _DE); // save high value
		writeMicroCodeByte(addr++, _SO | _ME | _FLG_BUS | _FW | _IC); // restore flag state

		/* deallocate a fixed number of bytes on the stack */
		showCommand("ADD SP, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
		writeMicroCodeByte(addr++, _FLG_CLC | _FW); // no carry
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SI | _IC); // set new stack pointer

		/* deallocate a variable number of bytes on the stack */
		showCommand("ADD SP, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
		writeMicroCodeByte(addr++, _FLG_CLC | _FW); // no carry
		writeMicroCodeByte(addr++, _S1 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_ADD | _ZW | _FW); // add stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SI | _IC); // set new stack pointer

		/* allocate a fixed number of bytes on the stack */
		showCommand("SUB SP, #val", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // no carry
		writeMicroCodeByte(addr++, _S1 | _ALU_SBI | _ZW | _FW); // subtract from stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_SBI | _ZW | _FW); // subtract from pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SI | _IC); // set new stack pointer

		/* allocate a fixed number of bytes on the stack */
		showCommand("SUB SP, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _BE | _ALU_BUS | _ZW); // get offset value
		writeMicroCodeByte(addr++, _FLG_STC | _FW); // no carry
		writeMicroCodeByte(addr++, _S1 | _ALU_SBI | _ZW | _FW); // subtract from stack pointer value
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW); // store low address
		writeMicroCodeByte(addr++, _S2 | _ALU_SBI | _ZW | _FW); // subtract from pointer value
		writeMicroCodeByte(addr++, _ZE | _NH); // store high address
		writeMicroCodeByte(addr++, _NO | _SI | _IC); // set new stack pointer

		/* 24 bit output */
		showCommand("OUW addr, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PW | _AE | _NO | _IC);

		showCommand("OUW [RCD], RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL );
		writeMicroCodeByte(addr++, _DE | _NH | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PW | _AE | _NO | _IC);

		showCommand("OUW [addr], RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_255 | _ZW); // base address low byte
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PW | _AE | _NO | _IC);

		showCommand("OUW [addr], RB, RA", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE);
		writeMicroCodeByte(addr++, _PW | _AE | _NO);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		/* 24 bit input */
		showCommand("INW RA, addr", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC);
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PE | _NO); // advertise address lines
		writeMicroCodeByte(addr++, 0); // NOOP, let the value get read
		writeMicroCodeByte(addr++, _PE | _AW | _NO | _IC);

		showCommand("INW RA, [RCD]", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL);
		writeMicroCodeByte(addr++, _DE | _NH | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PE | _NO); // advertise address lines
		writeMicroCodeByte(addr++, 0); // NOOP, let the value get read
		writeMicroCodeByte(addr++, _PE | _AW | _NO | _IC);

		showCommand("INW RA, [addr]", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_255 | _ZW); // base address low byte
		writeMicroCodeByte(addr++, _PS | _ZE); // highest port number
		writeMicroCodeByte(addr++, _PE | _NO); // advertise address lines
		writeMicroCodeByte(addr++, 0); // NOOP, let the value get read
		writeMicroCodeByte(addr++, _PE | _AW | _NO | _IC);

		showCommand("INW RA, [addr], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH | _ALU_255 | _ZW);
		writeMicroCodeByte(addr++, _PS | _ZE);
		writeMicroCodeByte(addr++, _PE | _NO); // advertise address lines
		writeMicroCodeByte(addr++, 0); // NOOP, let the value get read
		writeMicroCodeByte(addr++, _PE | _AW | _NO); // read value
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		// double register moves
		showCommand("MOV addr, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch address low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC); // fetch address high byte
		writeMicroCodeByte(addr++, _NO | _MW | _CE | _NC); // store data low byte
		writeMicroCodeByte(addr++, _NO | _MW | _DE | _IC); // store data high byte

		showCommand("MOV RCD, addr", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch address low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC); // fetch address high byte
		writeMicroCodeByte(addr++, _NO | _ME | _CW | _NC); // fetch data low byte
		writeMicroCodeByte(addr++, _NO | _ME | _DW | _IC); // fetch data high byte

		showCommand("MOV addr, RB, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW | _PC);
		writeMicroCodeByte(addr++, _NH | _ZE);
		writeMicroCodeByte(addr++, _NO | _MW | _CE | _NC);
		writeMicroCodeByte(addr++, _NO | _MW | _DE);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV RCD, addr, RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW);
		writeMicroCodeByte(addr++, _FLG_CLC | _FW);
		writeMicroCodeByte(addr++, _PO | _ME | _ALU_BUS | _ZW | _PC);
		writeMicroCodeByte(addr++, _ALU_ADD | _ZW | _BE | _FW);
		writeMicroCodeByte(addr++, _NL | _ZE | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _ALU_ADD | _PO | _ME | _ZW | _PC);
		writeMicroCodeByte(addr++, _NH | _ZE);
		writeMicroCodeByte(addr++, _NO | _ME | _CW | _NC);
		writeMicroCodeByte(addr++, _NO | _ME | _DW);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV [addr], RB, RCD", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH);
		writeMicroCodeByte(addr++, _NO | _MW | _CE | _NC);
		writeMicroCodeByte(addr++, _NO | _MW | _DE);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV RCD, [addr], RB", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _FE | _SO | _MW); // save flags
		writeMicroCodeByte(addr++, _PO | _ME | _NL | _PC); // fetch pointer low byte
		writeMicroCodeByte(addr++, _PO | _ME | _NH | _PC | _FLG_CLC | _FW); // fetch pointer high byte
		writeMicroCodeByte(addr++, _NO | _ME | _ALU_BUS | _ZW | _NC); // base address low byte
		writeMicroCodeByte(addr++, _NO | _ME | _NH); // base address high byte
		writeMicroCodeByte(addr++, _BE | _ALU_ADD | _ZW | _FW); // add RB to low byte
		writeMicroCodeByte(addr++, _ZE | _NL | _ALU_0 | _ZW);
		writeMicroCodeByte(addr++, _N2 | _ALU_ADD | _ZW); // carry to base address high byte
		writeMicroCodeByte(addr++, _ZE | _NH);
		writeMicroCodeByte(addr++, _NO | _ME | _CW | _NC);
		writeMicroCodeByte(addr++, _NO | _ME | _DW);
		writeMicroCodeByte(addr++, _FLG_BUS | _FW | _SO | _ME | _IC);

		showCommand("MOV RCD, [RCD]", cmd, addr);
		writeLabelUncond(true, cmd++, addr);
		writeMicroCodeByte(addr++, _CE | _NL); // fetch address low byte
		writeMicroCodeByte(addr++, _DE | _NH); // fetch address high byte
		writeMicroCodeByte(addr++, _NO | _ME | _CW | _NC); // fetch data low byte
		writeMicroCodeByte(addr++, _NO | _ME | _DW | _IC); // fetch data high byte

		/* set unused extended op codes to HLT */
		System.out.println("; Writing unsused codes");

		writeMicroCodeByte(addr, _HC);
		while (cmd < 0x200) {
			writeLabelUncond(true, cmd++, addr);
		}

		System.out.println("; bytes: " + cntbyte);
		System.out.println("; size: " + maxbyte);

		FileOutputStream fos = new FileOutputStream(pOutFile);
		fos.write(MC_ROM, 0, maxbyte + 1);
		fos.close();
	}

	/**
	 * Main program entry.
	 * 
	 * @param args destination path for MicroCode files
	 * @throws IOException error accessing the file system
	 */
	public static void main(String[] args) throws IOException {
		String baseDir = "I:\\gitrepos\\8bit-computer\\MicroCode";
		if (args.length > 0)
			baseDir = args[0];

		MicroCode mc = new MicroCode();
		/* generate all ROMs */
		for (int i = 0; i < 6; i++) {
			File romFile = new File(baseDir, "ROM#" + i);
			mc.generate(i, romFile);
		}

		for (int i = 0; i < 6; i++) {
			File romFile = new File(baseDir, "ROM#" + i);
			File oromFile = new File(baseDir, "ROM#" + i + ".last");
			if (!oromFile.exists()) {
				System.out.println("ROM#" + i + " is new");
			} else if (isDifferent(romFile, oromFile)) {
				System.out.println("ROM#" + i + " has changed");
			}
		}
	}

	/**
	 * compares the content of two files
	 * 
	 * @param romFile  new file
	 * @param oromFile old file
	 * @return <code>true</code> if there are changed bytes in the new file
	 * @throws IOException error accessing the file system
	 */
	private static boolean isDifferent(File romFile, File oromFile) throws IOException {
		int len = (int) romFile.length();
		int olen = (int) oromFile.length();
		if (len > olen)
			return true;

		byte[] data = new byte[len];
		byte[] odata = new byte[olen];
		FileInputStream fis = new FileInputStream(romFile);
		fis.read(data);
		fis.close();

		FileInputStream ofis = new FileInputStream(oromFile);
		ofis.read(odata);
		ofis.close();

		for (int i = 0; i < len; i++) {
			if (data[i] != odata[i])
				return true;
		}

		return false;
	}
}
