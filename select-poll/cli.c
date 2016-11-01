#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include "../include/netutils.h"

int main(int argc, char **argv) {
	if (argc != 2)
		fprintf(stderr, "Usage: %s [Server IP]\n", argv[0]);

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0)
		perror("inet_pton() error");

	if (connect(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		perror("connect() error");

	char buf[2048];
	ssize_t len;
	if ((len = forceread(sockFd, buf, 2048)) < 0)
		perror("read() error");
	buf[len] = 0;
	fputs(buf, stdout);

	while (1) {
		fgets(buf, 2048, stdin);
		if (forcewrite(sockFd, buf, strlen(buf)) < 0)
			perror("write() error");

		if ((len = forceread(sockFd, buf, 2048)) < 0)
			perror("read() error");
		buf[len] = 0;
		fputs(buf, stdout);

		if (strcmp(buf, "**bye**"))
			break;
	}

	if (close(sockFd) < 0)
		perror("close() error");

	return 0;
}
