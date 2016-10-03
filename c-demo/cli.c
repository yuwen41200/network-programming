#include <sys/socket.h> /* socket(), connect(), AF_INET, SOCK_STREAM */
#include <netinet/in.h> /* struct sockaddr_in, htons() */
#include <arpa/inet.h> /* inet_pton() */
#include <stdio.h> /* fprintf(), stderr, stdout */
#include <stdlib.h> /* exit() */
#include <string.h> /* bzero() */
#include <unistd.h> /* read() */

int main(int argc, char **argv) {

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [Server IP]\n", argv[0]);
		exit(1);
	}

	int sockFD;
	if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() error\n");
		exit(1);
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
		fprintf(stderr, "inet_pton() error for %s\n", argv[1]);
		exit(1);
	}

	if (connect(sockFD, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		fprintf(stderr, "connect() error\n");
		exit(1);
	}

	int len;
	char recvLine[256];
	while ((len = (int) read(sockFD, recvLine, 255)) > 0) {
		recvLine[len] = 0;
		fprintf(stdout, "%s\n", recvLine);
	}

	if (len < 0) {
		fprintf(stderr, "read() error\n");
		exit(1);
	}

	return 0;

}
