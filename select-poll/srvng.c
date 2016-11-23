#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
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

	unsigned maxClient = 0;
	struct pollfd clients[FOPEN_MAX];
	clients[0].fd = sockFd;
	clients[0].events = POLLRDNORM;
	for (int i = 1; i < FOPEN_MAX; ++i)
		clients[i].fd = -1;

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {
		int readyNum;
		if ((readyNum = poll(clients, maxClient + 1, -1)) < 0)
			perror("poll() error");

		if (clients[0].revents & POLLRDNORM) {
			int connFd;
			struct sockaddr_in clieAddr;
			socklen_t clieLen = sizeof(clieAddr);
			if ((connFd = accept(sockFd, (struct sockaddr *) &clieAddr, &clieLen)) < 0)
				perror("accept() error");

			unsigned i;
			for (i = 1; i < FOPEN_MAX; ++i) {
				if (clients[i].fd < 0) {
					clients[i].fd = connFd;
					break;
				}
			}

			if (i == FOPEN_MAX)
				fprintf(stderr, "maximum number of clients reached\n");
			if (i > maxClient)
				maxClient = i;

			clients[i].events = POLLRDNORM;

			if (forcewrite(connFd, "**hello**\n", 11) < 0)
				perror("write() error");

			if (--readyNum <= 0)
				continue;
		}

		for (int i = 1; i <= maxClient; ++i) {
			int clieFd = clients[i].fd;
			if (clieFd < 0)
				continue;

			if (clients[i].revents & (POLLRDNORM | POLLERR)) {
				char buf[2048];
				ssize_t len;
				if ((len = forceread(clieFd, buf, 2048)) < 0) {
					if (errno == ECONNRESET) {
						if (close(clieFd) < 0)
							perror("close() error");
						clients[i].fd = -1;
					}
					else
						perror("read() error");
				}

				else if (len == 0) {
					if (close(clieFd) < 0)
						perror("close() error");
					clients[i].fd = -1;
				}

				else if (!strcmp(buf, "\n")) {
					if (forcewrite(clieFd, "**bye**\n", 9) < 0)
						perror("write() error");
					if (close(clieFd) < 0)
						perror("close() error");
					clients[i].fd = -1;
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
