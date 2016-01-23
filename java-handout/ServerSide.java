/**
 * A Simple Networking Application - Server Side
 */

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.DateFormat;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.WindowConstants;

public class ServerSide {
	private boolean running = false;
	protected DateFormat dateFormat;
	protected JTextArea textArea;
	protected ServerSocket serverSocket;

	public ServerSide() {
		Font font = new Font(Font.MONOSPACED, Font.PLAIN, 16);
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setFont(font);
		textArea.setPreferredSize(new Dimension(600, 450));
		JScrollPane scrollPane = new JScrollPane(textArea);
		JButton startButton = new JButton("Start");
		startButton.addActionListener(e -> {
			try { start(); }
			catch (IOException ex) { ex.printStackTrace(); }
		});
		JButton stopButton = new JButton("Stop");
		stopButton.addActionListener(e -> {
			try { stop(); }
			catch (IOException ex) { ex.printStackTrace(); }
		});
		JPanel buttonPanel = new JPanel();
		buttonPanel.add(startButton, BorderLayout.WEST);
		buttonPanel.add(stopButton, BorderLayout.EAST);
		JFrame topFrame = new JFrame();
		topFrame.add(scrollPane, BorderLayout.CENTER);
		topFrame.add(buttonPanel, BorderLayout.SOUTH);
		topFrame.setTitle("Server Application");
		topFrame.pack();
		topFrame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		topFrame.setVisible(true);
	}

	public void start() throws IOException {
		dateFormat = DateFormat.getDateTimeInstance();
		serverSocket = new ServerSocket(5600);
		textArea.append("The server is started at " + dateFormat.format(new Date()) + ".\n");
		Socket socket = serverSocket.accept();
		RunServer runServer = new RunServer(socket);
		Thread thread = new Thread(runServer);
		thread.start();
	}

	public void stop() throws IOException {
		if (serverSocket != null)
			serverSocket.close();
		if (dateFormat != null)
			textArea.append("The server is stopped at " + dateFormat.format(new Date()) + ".\n");
	}

	public static void main(String args[]) {
		new ServerSide();
	}

	class RunServer implements Runnable {
		private Socket socket;

		public RunServer(Socket socket) {
			this.socket = socket;
		}

		@Override
		public void run() {
			try {
				DataInputStream dataInputStream = new DataInputStream(socket.getInputStream());
				DataOutputStream dataOutputStream = new DataOutputStream(socket.getOutputStream());
				InetAddress inetAddress = socket.getInetAddress();
				textArea.append(dateFormat.format(new Date()) + " Begin a connection from " +
						inetAddress.getHostName() + " (" + inetAddress.getHostAddress() + ").\n");
				Timer timer = new Timer();
				timer.schedule(new TimerTask() {
					@Override
					public void run() {
						running = false;
					}
				}, 60000);
				running = true;
				while (running) {
					String inputMessage = dataInputStream.readUTF();
					textArea.append(dateFormat.format(new Date()) + " Input Message: " + inputMessage + "\n");
					String outputMessage = inputMessage.toUpperCase();
					dataOutputStream.writeUTF(outputMessage);
					textArea.append(dateFormat.format(new Date()) + " Output Message: " + outputMessage + "\n");
				}
				timer.cancel();
				socket.close();
				textArea.append(dateFormat.format(new Date()) + " Close the connection from " +
						inetAddress.getHostName() + " (" + inetAddress.getHostAddress() + ").\n");
			}
			catch (IOException ex) {
				ex.printStackTrace();
			}
		}
	}
}
