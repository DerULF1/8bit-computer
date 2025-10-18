package net.derulf.eightbit;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.util.prefs.Preferences;

import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

/**
 * @author Ulf Caspers
 *
 */
public class EEPromerUI {

	private static final String CMD_CHOOSE = "choose";

	private static final String PREF_PORT_NAME = "PORT_NAME";

	private static JFrame mainFrame;
	private static File burnFile = null;
	private static JLabel fileLabel;
	private static JLabel portLabel;
	private static JComboBox<String> portCombo;
	private static JRadioButton chip8k;
	private static JRadioButton chip32k;
	private static JRadioButton chip4M;

	private static EEPromer _eepromer;
	private static JButton startButton;
	private static JButton readButton;
	private static JTextArea _logOut;

	private static class MyActionListener implements ActionListener {

		@Override
		public void actionPerformed(ActionEvent pEvent) {
			String cmd = pEvent.getActionCommand();
			if (CMD_CHOOSE.equals(cmd)) {
				JFileChooser jc = new JFileChooser(".");
				int res = jc.showSaveDialog(mainFrame);
				if (res == JFileChooser.APPROVE_OPTION) {
					burnFile = jc.getSelectedFile();
					fileLabel.setText(burnFile.getName());
					readButton.setEnabled(true);
					startButton.setEnabled(burnFile.exists());
					if (burnFile.exists()) {
						if (burnFile.length() > 8192) {
							chip4M.setSelected(true);
							chip8k.setSelected(false);
						} else {
							chip4M.setSelected(false);
							chip8k.setSelected(true);
						}
					}
					mainFrame.pack();
				}
			} else if ("quit".equals(cmd)) {
				Preferences prefs = Preferences.userNodeForPackage(EEPromerUI.class);
				prefs.put(PREF_PORT_NAME, (String) portCombo.getSelectedItem());
				disconnect();
				mainFrame.dispose();
			} else if ("read".equals(cmd)) {
				Thread t = new Thread(new Runnable() {

					@Override
					public void run() {
						try {
							_logOut.setText("");
							connect();
							_eepromer.setChipType(chip4M.isSelected() ? 2 : 1);
							int length;
							if (chip8k.isSelected())
								length = 8192;
							else if (chip32k.isSelected())
								length = 32768;
							else
								length = 524288;
							_eepromer.readChip(burnFile.getAbsolutePath(), length);
						} catch (IOException e) {
							e.printStackTrace();
							disconnect();
						}
					}
				});
				t.start();
			} else if ("burn".equals(cmd)) {
				Thread t = new Thread(new Runnable() {

					@Override
					public void run() {
						try {
							connect();
							_eepromer.setChipType(chip8k.isSelected() ? 1 : 2);
							_eepromer.writeChip(burnFile.getAbsolutePath());
						} catch (IOException e) {
							e.printStackTrace();
							disconnect();
						}
					}
				});
				t.start();
			}
		}

		/**
		 * Connect to Arduino.
		 * 
		 * @throws IOException error connecting
		 */
		private void connect() throws IOException {
			if (_eepromer == null) {
				String portName = (String) portCombo.getSelectedItem();
				_eepromer = new EEPromer(portName);
				_eepromer.setLogger(new EEPromerLogger());
				_eepromer.connect();

			}
		}

		/**
		 * Disconnect from Arduino.
		 */
		private void disconnect() {
			if (_eepromer != null) {
				try {
					_eepromer.disconnect();
				} catch (Exception exc) {
					exc.printStackTrace();
				}
				_eepromer = null;
			}
		}
	}

	private static class EEPromerLogger implements IEEPromerLogger {

		@Override
		public void log(String pText) {
			_logOut.append(pText);
			_logOut.setCaretPosition(_logOut.getText().length());
		}

		@Override
		public void logln(String pText) {
			_logOut.append(pText);
			_logOut.append("\r\n");
			_logOut.setCaretPosition(_logOut.getText().length());
		}

	}

	/**
	 * Create the GUI and show it. For thread safety, this method should be invoked
	 * from the event-dispatching thread.
	 */
	private static void createAndShowGUI() {
		// Create and set up the window.
		mainFrame = new JFrame("EEPROM Burner");
		mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		Container pane = mainFrame.getContentPane();
		pane.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();

		JLabel label = new JLabel("EEPROM Burner");
		c.gridx = 0;
		c.gridy = 0;
		c.gridwidth = 2;
		pane.add(label, c);
		c.gridwidth = 1;

		portLabel = new JLabel("Arduino Port:");
		c.gridx = 0;
		c.gridy = 1;
		pane.add(portLabel, c);

		Preferences prefs = Preferences.userNodeForPackage(EEPromerUI.class);
		String lastPort = prefs.get(PREF_PORT_NAME, "COM1");
		portCombo = new JComboBox<>();
		c.gridx = 1;
		c.gridy = 1;
		pane.add(portCombo, c);
		portCombo.setEditable(false);
		for (String name : EEPromer.getPortList()) {
			portCombo.addItem(name);
		}
		portCombo.setSelectedItem(lastPort);

		fileLabel = new JLabel("");
		c.gridx = 0;
		c.gridy = 2;
		pane.add(fileLabel, c);

		MyActionListener mal = new MyActionListener();
		JButton choose = new JButton("Choose File");
		choose.setActionCommand(CMD_CHOOSE);
		choose.addActionListener(mal);
		c.gridx = 1;
		c.gridy = 2;
		pane.add(choose, c);

		ButtonGroup chipGroup = new ButtonGroup();

		chip8k = new JRadioButton("8k");
		chip8k.setActionCommand("8k");
		chip8k.addActionListener(mal);
		c.gridx = 0;
		c.gridy = 3;
		pane.add(chip8k, c);
		chipGroup.add(chip8k);

		chip32k = new JRadioButton("32k");
		chip32k.setActionCommand("32k");
		chip32k.addActionListener(mal);
		c.gridx = 1;
		c.gridy = 3;
		pane.add(chip32k, c);
		chipGroup.add(chip32k);

		chip4M = new JRadioButton("4M");
		chip4M.setActionCommand("4M");
		chip4M.addActionListener(mal);
		c.gridx = 2;
		c.gridy = 3;
		pane.add(chip4M, c);
		chipGroup.add(chip4M);

		startButton = new JButton("Burn");
		startButton.setEnabled(burnFile != null);
		startButton.setActionCommand("burn");
		startButton.addActionListener(mal);
		c.gridx = 0;
		c.gridy = 4;
		pane.add(startButton, c);

		readButton = new JButton("Read");
		readButton.setEnabled(burnFile != null);
		readButton.setActionCommand("read");
		readButton.addActionListener(mal);
		c.gridx = 1;
		c.gridy = 4;
		pane.add(readButton, c);

		JButton endButton = new JButton("Quit");
		endButton.setActionCommand("quit");
		endButton.addActionListener(mal);
		c.gridx = 0;
		c.gridy = 5;
		c.gridwidth = 2;
		pane.add(endButton, c);
		c.gridwidth = 1;

		_logOut = new JTextArea("", 17, 22);
		c.gridx = 0;
		c.gridy = 6;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.BOTH;
		_logOut.setMinimumSize(new Dimension(200, 300));
		pane.add(new JScrollPane(_logOut), c);
		c.gridwidth = 1;

		// Display the window.
		mainFrame.pack();
		mainFrame.setVisible(true);
	}

	public static void main(String[] args) {
		// Schedule a job for the event-dispatching thread:
		// creating and showing this application's GUI.
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				createAndShowGUI();
			}
		});
	}
}
