<?php

define('SERVER_ADDR', '127.0.0.1');
define('SERVER_PORT', 12000);

$clientSocket = socket_create(AF_INET, SOCK_STREAM, getprotobyname('tcp'));
socket_connect($clientSocket, SERVER_ADDR, SERVER_PORT);

echo 'Sending Message: ';
$sendMsg = trim(fgets(STDIN, 2048));
socket_send($clientSocket, $sendMsg, 2048, MSG_EOR);

socket_recv($clientSocket, $recvMsg, 2048, MSG_PEEK);
echo 'Received Message: '. $recvMsg . "\n";

socket_close($clientSocket);

?>
