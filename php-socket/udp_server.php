<?php

define('SERVER_ADDR', '127.0.0.1');
define('SERVER_PORT', 12000);

$serverSocket = socket_create(AF_INET, SOCK_DGRAM, getprotobyname('udp'));
socket_bind($serverSocket, SERVER_ADDR, SERVER_PORT);
echo "Server Ready\n";

while (true) {
	socket_recvfrom($serverSocket, $recvMsg, 2048, 0, $clientAddr, $clientPort);
	$sendMsg = strtoupper($recvMsg);
	socket_sendto($serverSocket, $sendMsg, 2048, MSG_EOR, $clientAddr, $clientPort);
}

?>
