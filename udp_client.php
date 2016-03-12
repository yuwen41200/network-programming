<?php

define('SERVER_ADDR', '127.0.0.1');
define('SERVER_PORT', 12000);

$clientSocket = socket_create(AF_INET, SOCK_DGRAM, getprotobyname('udp'));

echo 'Sending Message: ';
$sendMsg = trim(fgets(STDIN, 2048));
socket_sendto($clientSocket, $sendMsg, 2048, MSG_EOR, SERVER_ADDR, SERVER_PORT);

socket_recvfrom($clientSocket, $recvMsg, 2048, MSG_PEEK, $serverAddr, $serverPort);
echo 'Received Message: '. $recvMsg . "\n";

socket_close($clientSocket);

?>
