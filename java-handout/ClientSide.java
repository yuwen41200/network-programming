/**
 * A Simple Networking Application - Client Side
 */

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.net.Socket;
import java.text.DateFormat;
import java.util.Date;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.WindowConstants;

public class ClientSide {
	protected DataInputStream dataInputStream;
	protected DataOutputStream dataOutputStream;
	protected DateFormat dateFormat = DateFormat.getDateTimeInstance();
	protected JTextArea textArea;

	public ClientSide() {
		Font font = new Font(Font.MONOSPACED, Font.PLAIN, 16);
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setFont(font);
		JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setPreferredSize(new Dimension(720, 450));
		JLabel label = new JLabel("Enter Message to Send: ");
		final JTextField textField = new JTextField();
		textField.setHorizontalAlignment(JTextField.RIGHT);
		textField.setPreferredSize(new Dimension(450, 20));
		textField.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e) {
				if (e.getKeyCode() == KeyEvent.VK_ENTER)
					send(textField.getText().trim());
			}
		});
		JButton button = new JButton("Send");
		button.addActionListener(e -> send(textField.getText().trim()));
		JPanel sendPanel = new JPanel();
		sendPanel.add(label, BorderLayout.WEST);
		sendPanel.add(textField, BorderLayout.CENTER);
		sendPanel.add(button, BorderLayout.EAST);
		JFrame topFrame = new JFrame();
		topFrame.add(scrollPane, BorderLayout.CENTER);
		topFrame.add(sendPanel, BorderLayout.SOUTH);
		topFrame.setTitle("Client Application");
		topFrame.pack();
		topFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		topFrame.setVisible(true);
		connect();
	}

	public void connect() {
		short retryCount = 0;
		do {
			try {
				Socket socket = new Socket("localhost", ServerSide.PORT);
				dataInputStream = new DataInputStream(socket.getInputStream());
				dataOutputStream = new DataOutputStream(socket.getOutputStream());
				textArea.append("Connected to server at " + dateFormat.format(new Date()) + ".\n");
			}
			catch (IOException ex) {
				ExceptionUtils.showStackTrace(ex);
				if (retryCount < 3) {
					textArea.append("Failed to start a connection at " + dateFormat.format(new Date()) +
							", retry after 5 seconds...\n");
					retryCount++;
					try {
						Thread.sleep(5000);
					}
					catch (InterruptedException exc) {
						ExceptionUtils.showStackTrace(exc);
					}
				}
				else {
					textArea.append("Failed to start a connection at " + dateFormat.format(new Date()) +
							", already retried 3 times.\n");
					break;
				}
			}
		} while (dataInputStream == null || dataOutputStream == null);
	}

	public void send(String message) {
		if (dataInputStream != null && dataOutputStream != null) {
			try {
				dataOutputStream.writeUTF(message);
				dataOutputStream.flush();
				textArea.append(dateFormat.format(new Date()) + " Result: " +
						dataInputStream.readUTF() + "\n");
			}
			catch (IOException ex) {
				ExceptionUtils.showStackTrace(ex);
				dataInputStream = null;
				dataOutputStream = null;
				textArea.append("Connection lost at " + dateFormat.format(new Date()) +
						", reconnecting...\n");
				connect();
			}
		}
	}

	public static void main(String args[]) {
		new ClientSide();
	}
}
