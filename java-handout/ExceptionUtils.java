/**
 * Utilities for Throwable Objects
 */

import java.awt.Dimension;
import java.io.StringWriter;
import java.io.PrintWriter;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

public class ExceptionUtils {
	public static String getStackTrace(Throwable throwable) {
		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		throwable.printStackTrace(printWriter);
		return stringWriter.toString();
	}

	public static void showStackTrace(Throwable throwable) {
		String message = ExceptionUtils.getStackTrace(throwable);
		JTextArea textArea = new JTextArea(message);
		JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setPreferredSize(new Dimension(720, 450));
		JOptionPane.showMessageDialog(null,
				scrollPane,
				"Exception Thrown: " + throwable.getClass().getName(),
				JOptionPane.ERROR_MESSAGE);
	}
}
