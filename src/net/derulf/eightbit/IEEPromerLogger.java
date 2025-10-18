package net.derulf.eightbit;

/**
 * @author Ulf Caspers
 *
 */
public interface IEEPromerLogger {

	/**
	 * Write a text to the log output.
	 * 
	 * @param pText text to write
	 */
	public void log(String pText);

	/**
	 * Write a text to the log output and appends a line break.
	 * 
	 * @param pText text to write
	 */
	public void logln(String pText);
}
