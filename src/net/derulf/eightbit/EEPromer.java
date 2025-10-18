package net.derulf.eightbit;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.List;

import dk.thibaut.serial.SerialConfig;
import dk.thibaut.serial.SerialPort;
import dk.thibaut.serial.enums.BaudRate;

/**
 * Basic Communication Protocol for Programming an EEPROM via an Arduino.
 * <p>
 * This code requires the JSerial libraries for the communication with a serial
 * port. An Arduino must be connected through the USB interface and the Arduino
 * specific port drivers must be installed. On the Arduino a matching sketch
 * must be executed.
 * </p>
 * 
 * @author Ulf Caspers
 *
 */
public class EEPromer {

	private String comPortName = "COM3";
	private SerialPort port;
	private OutputStream os;
	private InputStream is;
	private BufferedReader br;
	private boolean _report = false;

	private IEEPromerLogger _logger;

	// write block size of the 4M chip
	private int WRITE_BLOCK_SIZE = 4096;

	/**
	 * Standard constructor with a default COM port.
	 */
	public EEPromer() {
	}

	/**
	 * Standard constructor with a default COM port.
	 * 
	 * @param pComPortName COM port name for the connection to the Arduino
	 */
	public EEPromer(String pComPortName) {
		comPortName = pComPortName;
	}

	/**
	 * Returns a list of all available serial ports.
	 * 
	 * @return names of all available serial ports
	 */
	public static List<String> getPortList() {
		return SerialPort.getAvailablePortsNames();
	}

	public IEEPromerLogger getLogger() {
		return _logger;
	}

	public void setLogger(IEEPromerLogger pLogger) {
		_logger = pLogger;
	}

	public void logln(String pText) {
		if (_logger == null)
			System.out.println(pText);
		else
			_logger.logln(pText);
	}

	public void log(String pText) {
		if (_logger == null)
			System.out.print(pText);
		else
			_logger.log(pText);
	}

	/**
	 * Establishes the connection to the Arduino.
	 * 
	 * @throws IOException Error opening byte streams
	 */
	public void connect() throws IOException {
		port = SerialPort.open(comPortName);
		SerialConfig cfg = port.getConfig();
		cfg.BaudRate = BaudRate.B57600;
		port.setConfig(cfg);
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			// ignore
		}
		os = port.getOutputStream();
		is = port.getInputStream();
		InputStreamReader bir = new InputStreamReader(is);
		br = new BufferedReader(bir);
	}

	/**
	 * Terminates the connection.
	 * 
	 * @throws IOException error accessing the connection
	 */
	public void disconnect() throws IOException {
		port.close();
	}

	/**
	 * Redas a single line from the Arduino.
	 * 
	 * @return next line sent from the Arduino
	 * @throws IOException error accessing the connection
	 */
	private String readLine() throws IOException {
		return br.readLine();
	}

	/**
	 * Writes a single line to the Arduino.
	 * 
	 * @param pLine line to be written to the Arduino
	 * @throws IOException error accessing the connection
	 */
	private void writeLine(String pLine) throws IOException {
		os.write(pLine.getBytes("UTF-8"));
		os.write('\n');
		os.flush();
	}

	/**
	 * Main Menu selection.
	 * 
	 * @param pSelection (1/2/3) menu selection
	 * @throws IOException error accessing the connection
	 */
	private void selectMainMenu(String pSelection) throws IOException {
		String line;
		while (!">".equals(line = readLine())) {
			logln(line);
		}
		writeLine(pSelection);
	}

	/**
	 * Menu selection for the chip type.
	 * 
	 * @param pChipType (1/2) chip type
	 * @throws IOException error accessing the connection
	 */
	public void setChipType(int pChipType) throws IOException {
		selectMainMenu("1");
		String line;
		while (!">".equals(line = readLine())) {
			logln(line);
		}
		writeLine(Integer.toString(pChipType));
	}

	/**
	 * Read bytes from an EEPROM chip into a local byte buffer.
	 * 
	 * @param pBuf    byte buffer
	 * @param pStart  chip read start position
	 * @param pLength number of bytes to read
	 * @return number of bytes read
	 * @throws IOException error accessing the connection
	 */
	private int readChip(byte[] pBuf, int pStart, int pLength) throws IOException {
		selectMainMenu("2");
		String line;
		while (!">".equals(line = readLine())) {
			logln(line);
		}
		writeLine(pStart + " " + pLength + " b");

		int cnt = 0;
		int c;
		while ((c = is.read()) >= 0 && cnt < pLength) {
			pBuf[cnt++] = (byte) c;
			if (cnt % 256 == 0)
				log(".");
		}
		logln("");
		return cnt;
	}

	/**
	 * Copy a specified number of bytes from an EEPROM chip into a local file.
	 * 
	 * @param pName   destination file name
	 * @param pLength number of bytes to read
	 * @throws IOException error accessing the connection
	 */
	public void readChip(String pName, int pLength) throws IOException {
		byte[] data = new byte[pLength];
		for (int i = 0; i < data.length; i++)
			data[i] = 0;

		int bytesRead = readChip(data, 0, pLength);

		FileOutputStream fos = new FileOutputStream(pName);
		fos.write(data, 0, bytesRead);
		fos.close();
		logln((bytesRead) + " Bytes nach " + pName);
	}

	/**
	 * Write data from a local file to the EEPROM chip.
	 * 
	 * @param pName local file name
	 * @throws IOException error accessing the connection or reading the file
	 */
	public void writeChip(String pName) throws IOException {
		long startTime = System.currentTimeMillis();
		File f = new File(pName);
		int len = (int) f.length();
		byte[] data = new byte[len];

		FileInputStream fis = new FileInputStream(pName);
		fis.read(data);
		fis.close();

		String oldName = pName + ".last";
		File of = new File(oldName);
		byte[] odata;
		if (of.exists()) {
			int olen = (int) of.length();
			odata = new byte[olen];
			FileInputStream ofis = new FileInputStream(oldName);
			ofis.read(odata);
			ofis.close();
		} else {
			odata = new byte[0];
		}

		int errors = 0;
		int adr = 0;
		while (adr < len) {
			if (adr >= odata.length || data[adr] != odata[adr]) {
				int blockstart = adr & (0xfffffff - (WRITE_BLOCK_SIZE - 1));
				logln("Write Block " + blockstart / WRITE_BLOCK_SIZE);
				if (!_report) {
					int writeLen = Math.min(WRITE_BLOCK_SIZE, len - blockstart);
					errors += writeChip(data, blockstart, writeLen);
				}
				adr = blockstart + WRITE_BLOCK_SIZE;
			} else {
				adr++;
			}
		}

		long endTime = System.currentTimeMillis();
		logln((endTime - startTime) / 1000 + " Sekunden");
		logln((errors) + " Fehler");

		if (errors == 0 && !_report) {
			FileOutputStream fos = new FileOutputStream(oldName);
			fos.write(data, 0, len);
			fos.close();
		}
	}

	/**
	 * Write data from a local byte buffer onto the EEPROM chip.
	 * 
	 * @param pBuf    byte buffer
	 * @param pStart  start position on the chip
	 * @param pLength number of bytes to write
	 * @return number of bytes written
	 * @throws IOException error accessing the connection
	 */
	private int writeChip(byte[] pBuf, int pStart, int pLength) throws IOException {
		int errors = 0;
		selectMainMenu("3");
		String line;
		while (!">".equals(line = readLine())) {
			logln(line);
		}
		writeLine(pStart + " " + pLength + " b");
		String l = br.readLine();
		logln(l);

		int cnt = 0;
		while (cnt < pLength) {
			byte c = pBuf[pStart + cnt++];
			os.write(c);
			os.flush();
			if (cnt % 64 == 0) {
				l = br.readLine();
				try {
					if (Integer.parseInt(l) > errors) {
						logln(l);
						errors = Integer.parseInt(l);
					}
				} catch (Exception exc) {
					logln(l);
					errors = 1;
				}
			}
			if (cnt % 256 == 0)
				log(".");
		}
		logln("");

		while (!(line = br.readLine()).startsWith("ERRORS:")) {
			logln(line);
		}

		String[] parts = line.split("\\s+");
		if (parts.length > 1)
			errors = Integer.parseInt(parts[1]);

		return errors;
	}

	/**
	 * Test code.
	 * 
	 * @param args (ignored)
	 */
	public static void main(String[] args) {
		// lists the names of all available COM ports
		List<String> ports = SerialPort.getAvailablePortsNames();
		for (String string : ports) {
			System.out.println(string);
		}
		
		System.exit(0);

		// Test connection
		EEPromer ep = new EEPromer();
		try {
			ep.connect();
			ep.setChipType(1);

			ep.writeChip("ROMTest");

			ep.disconnect();

			System.out.println("Ende");
		} catch (Exception exc) {
			// TODO Auto-generated catch block
			exc.printStackTrace();
		}

	}

}
