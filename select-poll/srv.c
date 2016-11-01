#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/netutils.h"

int main() {
	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		perror("bind() error");

	if (listen(sockFd, 16) < 0)
		perror("listen() error");

	int maxFd = sockFd, maxClient = -1;
	int clients[FD_SETSIZE];
	for (int i = 0; i < FD_SETSIZE; ++i)
		clients[i] = -1;

	fd_set allFds;
	FD_ZERO(&allFds);
	FD_SET(sockFd, &allFds);

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {
		int readyNum;
		fd_set readFds = allFds;
		if ((readyNum = select(maxFd + 1, &readFds, NULL, NULL, NULL)) < 0)
			perror("select() error");

		if (FD_ISSET(sockFd, &readFds)) {
			int connFd;
			struct sockaddr_in clieAddr;
			socklen_t clieLen = sizeof(clieAddr);
			if ((connFd = accept(sockFd, (struct sockaddr *) &clieAddr, &clieLen)) < 0)
				perror("accept() error");

			int i;
			for (i = 0; i < FD_SETSIZE; ++i) {
				if (clients[i] < 0) {
					clients[i] = connFd;
					break;
				}
			}

			if (i == FD_SETSIZE)
				fprintf(stderr, "maximum number of clients reached\n");
			if (i > maxClient)
				maxClient = i;

			FD_SET(connFd, &allFds);
			if (connFd > maxFd)
				maxFd = connFd;

			if (forcewrite(connFd, "**hello**", 9) < 0)
				perror("write() error");

			if (--readyNum <= 0)
				continue;
		}

		for (int i = 0; i <= maxClient; i++) {
			int clieFd = clients[i];
			if (clieFd < 0)
				continue;

			if (FD_ISSET(clieFd, &readFds)) {
				char buf[2048];
				ssize_t len = forceread(clieFd, buf, 2048);

				switch (len) {
					case -1:
						perror("read() error");
						break;

					case 0:
						if (forcewrite(clieFd, "**bye**", 7) < 0)
							perror("write() error");
						if (close(clieFd) < 0)
							perror("close() error");
						FD_CLR(clieFd, &allFds);
						clients[i] = -1;
						break;

					default:
						for (int j = 0; j < len; ++j)
							if ('a' <= buf[j] && buf[j] <= 'z')
								buf[j] -= 0x20;
						if (forcewrite(clieFd, buf, (size_t) len) < 0)
							perror("write() error");
						break;
				}

				if (--readyNum <= 0)
					break;
			}
		}
	}
	#pragma clang diagnostic pop
}
