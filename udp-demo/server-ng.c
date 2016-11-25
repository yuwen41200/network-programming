#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

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
		// TODO
	}
}
