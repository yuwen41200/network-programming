<?php

define('SERVER_ADDR', '127.0.0.1');
define('SERVER_PORT', 12000);

$serverSocket = socket_create(AF_INET, SOCK_STREAM, getprotobyname('tcp'));
socket_bind($serverSocket, SERVER_ADDR, SERVER_PORT);
socket_listen($serverSocket, 1);
echo "Server Ready\n";

while (true) {
	$connSocket = socket_accept($serverSocket);
	socket_recv($connSocket, $recvMsg, 2048, MSG_PEEK);
	$sendMsg = strtoupper($recvMsg);
	socket_send($connSocket, $sendMsg, 2048, MSG_EOR);
	socket_close($connSocket);
}

?>
