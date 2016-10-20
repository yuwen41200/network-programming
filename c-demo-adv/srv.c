#include <sys/socket.h> /* socklen_t, socket(), bind(), listen(), accept(), AF_INET, SOCK_STREAM */
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), ntohs(), INADDR_ANY */
#include <arpa/inet.h> /* inet_ntop() */
#include <stdio.h> /* fprintf(), snprintf(), stderr, stdout, NULL */
#include <stdlib.h> /* exit() */
#include <string.h> /* bzero(), strlen() */
#include <unistd.h> /* pid_t, fork(), write(), close() */
#include <time.h> /* time_t, time(), ctime() */

int main(int argc, char **argv) {

	int sockFD;
	if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() error\n");
		exit(1);
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFD, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		fprintf(stderr, "bind() error\n");
		exit(1);
	}

	if (listen(sockFD, 16) < 0) {
		fprintf(stderr, "listen() error\n");
		exit(1);
	}

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {

		int connFD;
		struct sockaddr_in clieAddr;
		socklen_t len = sizeof(clieAddr);
		if ((connFD = accept(sockFD, (struct sockaddr *) &clieAddr, &len)) < 0) {
			fprintf(stderr, "accept() error\n");
			exit(1);
		}

		pid_t pid;
		if ((pid = fork()) < 0) {
			fprintf(stderr, "fork() error\n");
			exit(1);
		}

		if (pid == 0) {
			if (close(sockFD) < 0) {
				fprintf(stderr, "close() error\n");
				exit(1);
			}

			char strIP[128];
			inet_ntop(AF_INET, &clieAddr.sin_addr, strIP, sizeof(strIP));
			fprintf(stdout, "Client IP: %s (port %d)\n", strIP, ntohs(clieAddr.sin_port));

			time_t timer;
			char sendLine[256];
			time(&timer);
			snprintf(sendLine, sizeof(sendLine), "%s\n", ctime(&timer));

			if (write(connFD, sendLine, strlen(sendLine)) < 0) {
				fprintf(stderr, "write() error\n");
				exit(1);
			}

			if (close(connFD) < 0) {
				fprintf(stderr, "close() error\n");
				exit(1);
			}

			exit(0);
		}

		if (close(connFD) < 0) {
			fprintf(stderr, "close() error\n");
			exit(1);
		}

	}
	#pragma clang diagnostic pop

}
