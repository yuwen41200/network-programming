#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/netutils.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <SERVER IP> <SERVER PORT>\n", argv[0]);
		return 1;
	}

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons((uint16_t) strtoul(argv[2], NULL, 0));
	if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0)
		perror("inet_pton() error");

	if (connect(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		perror("connect() error");

	char buf[2048];
	ssize_t len;
	if ((len = forceread(sockFd, buf, 2048)) < 0)
		perror("read() error");
	else if (len > 0)
		fputs(buf, stdout);

	bool stdinEof = false;
	while (1) {
		fd_set readFds;
		FD_ZERO(&readFds);
		if (!stdinEof)
			FD_SET(STDIN_FILENO, &readFds);
		FD_SET(sockFd, &readFds);

		if (select(sockFd + 1, &readFds, NULL, NULL, NULL) < 0)
			perror("select() error");

		if (FD_ISSET(STDIN_FILENO, &readFds)) {
			fgets(buf, 2048, stdin);
			if (!strcmp(buf, "exit\n")) {
				if (shutdown(sockFd, SHUT_WR) < 0)
					perror("shutdown() error");
				stdinEof = true;
			}
			else if (forcewrite(sockFd, buf, strlen(buf) + 1) < 0)
				perror("write() error");
		}

		if (FD_ISSET(sockFd, &readFds)) {
			if ((len = forceread(sockFd, buf, 2048)) < 0)
				perror("read() error");
			else if (len == 0 && stdinEof)
				break;
			else if (len > 0)
				fputs(buf, stdout);
		}
	}

	if (close(sockFd) < 0)
		perror("close() error");

	return 0;
}
