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
	protected DateFormat dateFormat = DateFormat.getDateTimeInstance();
	protected JTextArea textArea;
	protected Socket socket;

	public ClientSide() {
		Font font = new Font(Font.MONOSPACED, Font.PLAIN, 16);
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setFont(font);
		textArea.setPreferredSize(new Dimension(600, 450));
		JScrollPane scrollPane = new JScrollPane(textArea);
		JLabel label = new JLabel("Enter Message to Send: ");
		JTextField textField = new JTextField();
		textField.setHorizontalAlignment(JTextField.RIGHT);
		textField.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent e) {
				if (e.getKeyCode() == KeyEvent.VK_ENTER) {
					try { send(); }
					catch (IOException ex) { ex.printStackTrace(); }
				}
			}
		});
		JButton button = new JButton("Send");
		button.addActionListener(e -> {
			try { send(); }
			catch (IOException ex) { ex.printStackTrace(); }
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
	}

	public void init() throws IOException {
		socket = new Socket("localhost", 5600);
		DataInputStream dataInputStream = new DataInputStream(socket.getInputStream());
		DataOutputStream dataOutputStream = new DataOutputStream(socket.getOutputStream());
		textArea.append("The server is started at " + dateFormat.format(new Date()) + ".\n");
	}

	public void send() throws IOException {
		if (serverSocket != null) {
			listening = false;
			serverSocket.close();
			serverSocket = null;
			textArea.append("The server is stopped at " + dateFormat.format(new Date()) + ".\n");
		}
	}

	public static void main(String args[]) {
		new ClientSide();
	}
}
