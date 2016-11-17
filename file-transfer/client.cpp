#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sstream>
#include <sys/stat.h>
#include <fstream>
#include "../include/netutils.h"
#include "../include/strutils.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [SERVER ADDRESS] [SERVER PORT]\n", argv[0]);
		return 1;
	}

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return 1;
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons((uint16_t) strtoul(argv[2], NULL, 0));
	if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
		struct hostent *host;
		if ((host = gethostbyname(argv[1])) != NULL) {
			struct in_addr **addrs = (struct in_addr **) host->h_addr_list;
			if (addrs[0] != NULL)
				servAddr.sin_addr = *addrs[0];
		}
	}

	if (connect(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("connect() error");
		return 1;
	}

	char buf[2048], clientId[6];
	if (forceread(sockFd, buf, 2048) <= 0 || memcmp(buf, "IG", 2)) { // CLIENT ID GET operation
		fprintf(stderr, "cannot initialize connection\n");
		return 1;
	}
	memcpy(clientId, buf + 2, 5); // client id, 5 bytes
	clientId[5] = 0;

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

			std::istringstream tokenizer(buf);
			std::vector<std::string> tokens;
			std::string token;
			while (tokenizer >> token)
				tokens.push_back(token);

			if (tokens.empty());

			else if (tokens.front() == "EXIT" && tokens.size() == 1) {
				if (shutdown(sockFd, SHUT_WR) < 0)
					perror("shutdown() error");
				stdinEof = true;
			}

			else if (tokens.front() == "PUT" && tokens.size() == 3) {
				std::ifstream file(tokens[1], std::ios::in);
				struct stat statBuf;
				if (!file.is_open())
					fprintf(stderr, "cannot read %s\n", tokens[1].c_str());
				if (stat(tokens[1].c_str(), &statBuf) < 0)
					perror("stat() error");

				char tempBuf[128];
				strcpy(buf, "PI"); // PUT FILE INITIALIZE operation
				sprintf(tempBuf, "%05d", (int) tokens[2].size() + 5);
				strcat(buf, tempBuf); // length of file name, 5 bytes
				strcat(buf, clientId); // client id as prefix of file name
				strcat(buf, tokens[2].c_str()); // original file name
				sprintf(tempBuf, "%010d", (int) statBuf.st_size);
				strcat(buf, tempBuf); // length of file, 10 bytes

				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");

				while (file.good()) {
					file.read(buf + 7, 1920); // data to send
					strcpy(buf, "PD"); // PUT FILE DATA operation
					sprintf(tempBuf, "%05d", (int) file.gcount());
					memcpy(buf + 2, tempBuf, 5); // length of data, 5 bytes

					if (forcewrite(sockFd, buf, 2048) < 0)
						perror("write() error");
				}

				strcpy(buf, "PF"); // PUT FILE FINALIZE operation
				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");

				file.close();
			}

			else
				fprintf(stderr, "invalid command\n");
		}

		if (FD_ISSET(sockFd, &readFds)) {
			ssize_t len;
			if ((len = forceread(sockFd, buf, 2048)) < 0)
				perror("read() error");
			else if (len == 0) {
				if (!stdinEof)
					fprintf(stderr, "server disconnected\n");
				break;
			}
			else
				fputs(buf, stdout);
		}
	}

	if (close(sockFd) < 0)
		perror("close() error");

	return 0;
}
