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
#include <unistd.h>
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
	if (forceread(sockFd, buf, 2048, false) <= 0 || memcmp(buf, "IG", 2)) { // CLIENT ID GET operation
		fprintf(stderr, "cannot initialize connection\n");
		return 1;
	}
	memcpy(clientId, buf + 2, 5); // client id, 5 bytes
	clientId[5] = 0;

	bool stdinEof = false;
	char tempBuf[128];
	std::ofstream *pFile = NULL;
	long decoded;
	std::string tempName;

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
				std::ifstream file(tokens[1], std::ios::in | std::ios::binary);
				struct stat statBuf;
				if (!file.is_open())
					fprintf(stderr, "cannot read %s\n", tokens[1].c_str());
				if (stat(tokens[1].c_str(), &statBuf) < 0)
					perror("stat() error");

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
				sprintf(tempBuf, "PUT %s %s succeeded\n", tokens[1].c_str(), tokens[2].c_str());
				strcat(buf, tempBuf); // success message

				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");

				file.close();
			}

			else if (tokens.front() == "GET" && tokens.size() == 3) {
				tempName = tokens[2];
				pFile = new std::ofstream(tokens[2], std::ios::out | std::ios::trunc | std::ios::binary);
				if (!pFile->is_open())
					fprintf(stderr, "cannot write %s\n", tokens[2].c_str());

				strcpy(buf, "GI"); // GET FILE INITIALIZE operation
				sprintf(tempBuf, "%05d", (int) tokens[1].size() + 5);
				strcat(buf, tempBuf); // length of file name, 5 bytes
				strcat(buf, clientId); // client id as prefix of file name
				strcat(buf, tokens[1].c_str()); // original file name
				sprintf(tempBuf, "GET %s %s succeeded\n", tokens[1].c_str(), tokens[2].c_str());
				strcat(buf, tempBuf); // success message

				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");
			}

			else if (tokens.front() == "LIST" && tokens.size() == 1) {
				strcpy(buf, "LR"); // LIST FILES REQUEST operation
				strcat(buf, clientId); // client id as prefix of file name

				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");
			}

			else if (tokens.front() == "CLIENTID:" && tokens.size() == 2 && tokens[1].size() == 5) {
				strcpy(clientId, tokens[1].c_str());

				strcpy(buf, "IS"); // CLIENT ID SET operation
				strcat(buf, clientId); // client id, 5 bytes

				if (forcewrite(sockFd, buf, 2048) < 0)
					perror("write() error");
			}

			else
				fprintf(stderr, "invalid command\n");
		}

		if (FD_ISSET(sockFd, &readFds)) {
			ssize_t len;
			if ((len = forceread(sockFd, buf, 2048, false)) < 0)
				perror("read() error");

			else if (len == 0) {
				if (!stdinEof)
					fprintf(stderr, "server disconnected\n");
				break;
			}

			else if (!memcmp(buf, "PE", 2)) // PUT FILE ERROR operation
				fprintf(stderr, "failed to send the designated file\n");

			else if (!memcmp(buf, "GE", 2)) { // GET FILE ERROR operation
				fprintf(stderr, "failed to receive the designated file\n");
				pFile->close();
				unlink(tempName.c_str());
			}

			else if (!memcmp(buf, "GD", 2)) { // GET FILE DATA operation
				memcpy(tempBuf, buf + 2, 5); // length of data, 5 bytes
				tempBuf[5] = 0;
				decoded = strtol(tempBuf, NULL, 10);
				pFile->write(buf + 7, decoded); // data to receive
			}

			else if (!memcmp(buf, "GF", 2)) { // GET FILE FINALIZE operation
				memcpy(tempBuf, buf + 2, 10); // length of file, 10 bytes
				tempBuf[10] = 0;
				decoded = strtol(tempBuf, NULL, 10);
				if (pFile->tellp() == decoded)
					fputs(buf + 12, stdout); // success message
				else
					fprintf(stderr, "server's file and client's file are different\n");
				pFile->close();
			}

			else if (!memcmp(buf, "MP", 2)) // MESSAGE PRINT STDOUT operation
				fputs(buf + 2, stdout); // message to print

			else if (!memcmp(buf, "ME", 2)) // MESSAGE PRINT STDERR operation
				fputs(buf + 2, stderr); // message to print

			else
				fprintf(stderr, "invalid response\n");
		}
	}

	if (close(sockFd) < 0)
		perror("close() error");

	return 0;
}
