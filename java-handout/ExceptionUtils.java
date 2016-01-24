/**
 * Utilities for Throwable Objects
 */

import javax.swing.JOptionPane;
import java.io.StringWriter;
import java.io.PrintWriter;

public class ExceptionUtils {
	public static String getStackTrace(Throwable throwable) {
		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		throwable.printStackTrace(printWriter);
		return stringWriter.toString();
	}

	public static void showStackTrace(Throwable throwable) {
		String message = ExceptionUtils.getStackTrace(throwable);
		JOptionPane.showMessageDialog(null,
				message,
				"Exception Thrown: " + throwable.getClass().getName(),
				JOptionPane.ERROR_MESSAGE);
	}
}
