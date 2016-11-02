#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
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

	char buf[2048];
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

			char strIp[128];
			inet_ntop(AF_INET, &clieAddr.sin_addr, strIp, sizeof(strIp));
			sprintf(buf, "[Server] Hello, anonymous! From: %s/%d\n", strIp, ntohs(clieAddr.sin_port));

			if (forcewrite(connFd, buf, strlen(buf) + 1) < 0)
				perror("write() error");

			if (--readyNum <= 0)
				continue;
		}

		for (int i = 0; i <= maxClient; i++) {
			int clieFd = clients[i];
			if (clieFd < 0)
				continue;

			if (FD_ISSET(clieFd, &readFds)) {
				ssize_t len;
				if ((len = forceread(clieFd, buf, 2048)) < 0)
					perror("read() error");

				else if (len == 0) {
					if (close(clieFd) < 0)
						perror("close() error");
					FD_CLR(clieFd, &allFds);
					clients[i] = -1;
				}

				else if (!strcmp(buf, "\n")) {
					if (forcewrite(clieFd, "**bye**\n", 9) < 0)
						perror("write() error");
					if (close(clieFd) < 0)
						perror("close() error");
					FD_CLR(clieFd, &allFds);
					clients[i] = -1;
				}

				else {
					for (int j = 0; j < len; ++j)
						if ('a' <= buf[j] && buf[j] <= 'z')
							buf[j] -= 0x20;
					if (forcewrite(clieFd, buf, (size_t) len) < 0)
						perror("write() error");
				}

				if (--readyNum <= 0)
					break;
			}
		}
	}
	#pragma clang diagnostic pop
}
