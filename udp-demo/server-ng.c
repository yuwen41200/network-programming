#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../include/netutils.h"

void handler(int signum) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
	return;
}

int main() {
	int tcpFd;
	if ((tcpFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(57345);
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	const int on = 1;
	if (setsockopt(tcpFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		perror("setsockopt() error");

	if (bind(tcpFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
		perror("bind() error");

	if (listen(tcpFd, 16) < 0)
		perror("listen() error");

	int udpFd;
	if ((udpFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	if (bind(udpFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
		perror("bind() error");

	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &act, NULL) < 0)
		perror("sigaction() error");

	fd_set fds;
	FD_ZERO(&fds);
	int maxFd = tcpFd > udpFd ? tcpFd : udpFd;

	while (1) {
		ssize_t len;
		char buf[4096];
		FD_SET(tcpFd, &fds);
		FD_SET(udpFd, &fds);

		if (select(maxFd + 1, &fds, NULL, NULL, NULL) < 0) {
			if (errno == EINTR)
				continue;
			else
				perror("select() error");
		}

		if (FD_ISSET(tcpFd, &fds)) {
			int connFd;
			if ((connFd = accept(tcpFd, (struct sockaddr *) NULL, NULL)) < 0)
				perror("accept() error");

			pid_t pid;
			if ((pid = fork()) < 0)
				perror("fork() error");

			if (pid == 0) {
				if (close(tcpFd) < 0)
					perror("close() error");

				while (1) {
					if ((len = forceread(connFd, buf, 4096)) < 0)
						perror("read() error");

					if (len == 0)
						break;

					if (forcewrite(connFd, buf, (size_t) len) < 0)
						perror("write() error");
				}

				if (close(connFd) < 0)
					perror("close() error");

				_exit(0);
			}

			if (close(connFd) < 0)
				perror("close() error");
		}

		if (FD_ISSET(udpFd, &fds)) {
			struct sockaddr_in cliAddr;
			socklen_t addrLen = sizeof(cliAddr);

			if ((len = recvfrom(udpFd, buf, 4096, 0, (struct sockaddr *) &cliAddr, &addrLen)) < 0)
				perror("recvfrom() error");

			if (sendto(udpFd, buf, (size_t) len, 0, (const struct sockaddr *) &cliAddr, addrLen) < 0)
				perror("sendto() error");
		}
	}
}
