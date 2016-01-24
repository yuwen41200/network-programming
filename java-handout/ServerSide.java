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
	private boolean listening;
	protected DateFormat dateFormat = DateFormat.getDateTimeInstance();
	protected JTextArea textArea;
	protected ServerSocket serverSocket;
	public static final int PORT = 5600;

	public ServerSide() {
		Font font = new Font(Font.MONOSPACED, Font.PLAIN, 16);
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setFont(font);
		JScrollPane scrollPane = new JScrollPane(textArea);
		scrollPane.setPreferredSize(new Dimension(720, 450));
		JButton startButton = new JButton("Start");
		startButton.addActionListener(e -> {
			try { start(); }
			catch (IOException ex) { ExceptionUtils.showStackTrace(ex); }
		});
		JButton stopButton = new JButton("Stop");
		stopButton.addActionListener(e -> {
			try { stop(); }
			catch (IOException ex) { ExceptionUtils.showStackTrace(ex); }
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
		serverSocket = new ServerSocket(PORT);
		listening = true;
		ListenThread listenThread = new ListenThread(serverSocket);
		listenThread.start();
		textArea.append("The server is started at " + dateFormat.format(new Date()) + ".\n");
	}

	public void stop() throws IOException {
		if (serverSocket != null) {
			listening = false;
			serverSocket.close();
			serverSocket = null;
			textArea.append("The server is stopped at " + dateFormat.format(new Date()) + ".\n");
		}
	}

	public static void main(String args[]) {
		new ServerSide();
	}

	class ListenThread extends Thread {
		private ServerSocket serverSocket;

		public ListenThread(ServerSocket serverSocket) {
			this.serverSocket = serverSocket;
		}

		@Override
		public void run() {
			while (listening) {
				try {
					Socket socket = serverSocket.accept();
					RunThread runThread = new RunThread(socket);
					runThread.start();
				}
				catch (IOException ex) {
					ExceptionUtils.showStackTrace(ex);
				}
			}
		}
	}

	class RunThread extends Thread {
		private Socket socket;
		private boolean running;

		public RunThread(Socket socket) {
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
					textArea.append(dateFormat.format(new Date()) + " Input Message: " +
							inputMessage + "\n");
					String outputMessage = inputMessage.toUpperCase();
					dataOutputStream.writeUTF(outputMessage);
					textArea.append(dateFormat.format(new Date()) + " Output Message: " +
							outputMessage + "\n");
				}
				timer.cancel();
				socket.close();
				textArea.append(dateFormat.format(new Date()) + " Close the connection from " +
						inetAddress.getHostName() + " (" + inetAddress.getHostAddress() + ").\n");
			}
			catch (IOException ex) {
				ExceptionUtils.showStackTrace(ex);
			}
		}
	}
}
