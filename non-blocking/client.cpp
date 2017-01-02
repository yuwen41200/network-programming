#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "../include/netutils.h"
#include "../include/strutils.h"

int main(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s [SERVER ADDRESS] [SERVER PORT] [USERNAME]\n", argv[0]);
		return 1;
	}

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return 1;
	}

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons((uint16_t) std::stoul(argv[2]));
	if (inet_pton(AF_INET, argv[1], &srvAddr.sin_addr) <= 0) {
		struct hostent *host;
		if ((host = gethostbyname(argv[1])) != NULL) {
			struct in_addr **addrs = (struct in_addr **) host->h_addr_list;
			if (addrs[0] != NULL)
				srvAddr.sin_addr = *addrs[0];
		}
	}

	if (connect(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0) {
		perror("connect() error");
		return 1;
	}

	char buf[2048];
	char tempBuf[128];
	std::ofstream ofs;
	bool stdinEof = false;
	long decoded, fileSize = -1;

	strcpy(buf, "IS"); // CLIENT ID SET operation
	sprintf(tempBuf, "%05d", (int) strlen(argv[3]));
	strcat(buf, tempBuf); // length of client id, 5 bytes
	strcat(buf, argv[3]); // client id

	if (forcewrite(sockFd, buf, 2048) < 0)
		perror("write() error");

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

			else if (tokens.front() == "/exit" && tokens.size() == 1) {
				if (shutdown(sockFd, SHUT_WR) < 0)
					perror("shutdown() error");
				stdinEof = true;
			}

			else if (tokens.front() == "/put" && tokens.size() == 2) {
				struct stat statBuf;
				std::ifstream ifs(tokens[1], std::ios::in | std::ios::binary);

				if (!ifs.is_open())
					fprintf(stderr, "cannot read %s\n", tokens[1].c_str());

				else if (stat(tokens[1].c_str(), &statBuf) < 0)
					perror("stat() error");

				else {
					strcpy(buf, "PI"); // PUT FILE INITIALIZE operation
					sprintf(tempBuf, "%05d", (int) tokens[1].size());
					strcat(buf, tempBuf); // length of file name, 5 bytes
					strcat(buf, tokens[1].c_str()); // original file name
					sprintf(tempBuf, "%010d", (int) statBuf.st_size);
					strcat(buf, tempBuf); // length of file, 10 bytes

					if (forcewrite(sockFd, buf, 2048) < 0)
						perror("write() error");

					while (ifs.good()) {
						ifs.read(buf + 7, 1920); // data to send
						strcpy(buf, "PD"); // PUT FILE DATA operation
						sprintf(tempBuf, "%05d", (int) ifs.gcount());
						memcpy(buf + 2, tempBuf, 5); // length of data, 5 bytes

						if (forcewrite(sockFd, buf, 2048) < 0)
							perror("write() error");
					}

					strcpy(buf, "PF"); // PUT FILE FINALIZE operation
					ifs.close();

					if (forcewrite(sockFd, buf, 2048) < 0)
						perror("write() error");
				}
			}

			else if (tokens.front() == "/sleep" && tokens.size() == 2) {
				decoded = strtol(tokens[1], NULL, 10);
				sleep((unsigned) decoded);
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

			if (!memcmp(buf, "GI", 2)) { // GET FILE INITIALIZE operation
				memcpy(tempBuf, buf + 2, 5); // length of file name, 5 bytes
				tempBuf[5] = 0;
				decoded = strtol(tempBuf, NULL, 10);
				memcpy(tempBuf, buf + 7, (size_t) decoded); // original file name
				tempBuf[decoded] = 0;
				fileSize = strtol(buf + 7 + decoded, NULL, 10); // length of file, 10 bytes
				if (ofs.is_open())
					ofs.close();
				ofs.open(tempBuf, std::ios::out | std::ios::trunc | std::ios::binary);
			}

			else if (!memcmp(buf, "GD", 2) && ofs.is_open()) { // GET FILE DATA operation
				memcpy(tempBuf, buf + 2, 5); // length of data, 5 bytes
				tempBuf[5] = 0;
				decoded = strtol(tempBuf, NULL, 10);
				ofs.write(buf + 7, decoded); // data to receive
			}

			else if (!memcmp(buf, "GF", 2) && ofs.is_open()) { // GET FILE FINALIZE operation
				if (ofs.tellp() != fileSize)
					fprintf(stderr, "server's file and client's file are different\n");
				ofs.close();
			}

			else
				fprintf(stderr, "invalid response\n");
		}
	}

	if (close(sockFd) < 0)
		perror("close() error");

	return 0;
}
