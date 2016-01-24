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
		textArea.setPreferredSize(new Dimension(600, 450));
		JScrollPane scrollPane = new JScrollPane(textArea);
		JLabel label = new JLabel("Enter Message to Send: ");
		final JTextField textField = new JTextField();
		textField.setHorizontalAlignment(JTextField.RIGHT);
		textField.setPreferredSize(new Dimension(480, 20));
		textField.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e) {
				if (e.getKeyCode() == KeyEvent.VK_ENTER) {
					try { send(textField.getText().trim()); }
					catch (IOException ex) { ExceptionUtils.showStackTrace(ex); }
				}
			}
		});
		JButton button = new JButton("Send");
		button.addActionListener(e -> {
			try { send(textField.getText().trim()); }
			catch (IOException ex) { ExceptionUtils.showStackTrace(ex); }
		});
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
		do {
			try {
				init();
			}
			catch (IOException ex) {
				ExceptionUtils.showStackTrace(ex);
			}
			try {
				Thread.sleep(5000);
			}
			catch (InterruptedException ex) {
				ExceptionUtils.showStackTrace(ex);
			}
		} while (dataInputStream == null || dataOutputStream == null);
	}

	public void init() throws IOException {
		Socket socket = new Socket("localhost", 5600);
		dataInputStream = new DataInputStream(socket.getInputStream());
		dataOutputStream = new DataOutputStream(socket.getOutputStream());
		textArea.append("Connected to server at " + dateFormat.format(new Date()) + ".\n");
	}

	public void send(String message) throws IOException {
		if (dataInputStream != null && dataOutputStream != null) {
			dataOutputStream.writeUTF(message);
			dataOutputStream.flush();
			textArea.append(dateFormat.format(new Date()) + " Result: " +
					dataInputStream.readUTF() + "\n");
		}
	}

	public static void main(String args[]) {
		new ClientSide();
	}
}
