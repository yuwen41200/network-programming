#include <sys/socket.h> /* socket(), bind(), listen(), accept(), AF_INET, SOCK_STREAM */
#include <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), INADDR_ANY */
#include <stdio.h> /* fprintf(), snprintf(), stderr, NULL */
#include <stdlib.h> /* exit() */
#include <string.h> /* bzero(), strlen() */
#include <unistd.h> /* write(), close() */
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
		if ((connFD = accept(sockFD, (struct sockaddr *) NULL, NULL)) < 0) {
			fprintf(stderr, "accept() error\n");
			exit(1);
		}

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

	}
	#pragma clang diagnostic pop

}
