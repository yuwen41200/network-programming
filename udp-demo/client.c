#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
	if (argc != 2)
		fprintf(stderr, "Usage: %s [Server IP]\n", argv[0]);

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(57345);
	if (inet_pton(AF_INET, argv[1], &srvAddr.sin_addr) <= 0)
		perror("inet_pton() error");

	char buf[4096];
	while (fgets(buf, 4096, stdin) != NULL) {
		if (!strcmp(buf, "\n"))
			break;

		if (sendto(sockFd, buf, strlen(buf) + 1, 0, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0)
			perror("sendto() error");

		if (recvfrom(sockFd, buf, 4096, 0, NULL, NULL) < 0)
			perror("recvfrom() error");

		fputs(buf, stdout);
	}

	return 0;
}
