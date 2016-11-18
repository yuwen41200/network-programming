#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <random>
#include <sstream>
#include <set>
#include <fstream>
#include <sys/stat.h>
#include "../include/netutils.h"
#include "../include/strutils.h"

int main() {
	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return 1;
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind() error");
		return 1;
	}

	if (listen(sockFd, 16) < 0) {
		perror("listen() error");
		return 1;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 99999);
	std::map<std::string, int> clients;
	std::set<std::string> permanentIds;

	fd_set allFds;
	FD_ZERO(&allFds);
	FD_SET(sockFd, &allFds);
	int maxFd = sockFd;

	char buf[2048], tempBuf[128];
	std::ofstream *pFile = NULL;
	long decoded, fileSize = -1;

	while (1) {
		int readyNum;
		fd_set readFds = allFds;
		if ((readyNum = select(maxFd + 1, &readFds, NULL, NULL, NULL)) < 0)
			perror("select() error");

		if (FD_ISSET(sockFd, &readFds)) {
			int connFd;
			if ((connFd = accept(sockFd, (struct sockaddr *) NULL, NULL)) < 0)
				perror("accept() error");

			if (clients.size() == FD_SETSIZE) {
				fprintf(stderr, "maximum number of clients reached\n");
				continue;
			}

			char key[6];
			while (1) {
				int rdNum = dis(gen);
				sprintf(key, "%05d", rdNum);
				if (clients.find(key) == clients.end()) {
					clients[key] = connFd;
					break;
				}
			}

			FD_SET(connFd, &allFds);
			if (connFd > maxFd)
				maxFd = connFd;

			strcpy(buf, "IG"); // CLIENT ID GET operation
			strcat(buf, key); // client id, 5 bytes
			if (forcewrite(connFd, buf, 2048) < 0)
				perror("write() error");

			if (--readyNum <= 0)
				continue;
		}

		for (auto it = clients.begin(); it != clients.end(); ) {
			std::string clientId = it->first;
			int clieFd = it->second;
			ssize_t len = -1;

			if (FD_ISSET(clieFd, &readFds)) {
				if ((len = forceread(clieFd, buf, 2048, false)) < 0)
					perror("read() error");

				else if (len == 0) {
					fputs("client has closed the connection\n", stdout);

					if (permanentIds.find(clientId) == permanentIds.end()) {
						// TODO: delete all files belongs to that client id
					}

					if (close(clieFd) < 0)
						perror("close() error");
					FD_CLR(clieFd, &allFds);
					it = clients.erase(it);
				}

				else if (!memcmp(buf, "GI", 2)) { // GET FILE INITIALIZE operation
					memcpy(tempBuf, buf + 2, 5); // length of file name, 5 bytes
					tempBuf[5] = 0;
					decoded = strtol(tempBuf, NULL, 10);
					memcpy(tempBuf, buf + 7, (size_t) decoded); // file name
					tempBuf[decoded] = 0;
					std::string success(buf + 7 + decoded); // success message

					std::ifstream file(tempBuf, std::ios::in | std::ios::binary);
					struct stat statBuf;
					strcpy(buf, "GE"); // GET FILE ERROR operation
					if (!file.is_open() || stat(tempBuf, &statBuf) < 0)
						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");

					while (file.good()) {
						file.read(buf + 7, 1920); // data to receive
						strcpy(buf, "GD"); // GET FILE DATA operation
						sprintf(tempBuf, "%05d", (int) file.gcount());
						memcpy(buf + 2, tempBuf, 5); // length of data, 5 bytes

						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");
					}

					if (memcmp(buf, "GE", 2)) {
						strcpy(buf, "GF"); // GET FILE FINALIZE operation
						sprintf(tempBuf, "%010d", (int) statBuf.st_size);
						strcat(buf, tempBuf); // length of file, 10 bytes
						strcat(buf, success.c_str()); // success message

						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");

						file.close();
					}
				}

				else if (!memcmp(buf, "PI", 2)) { // PUT FILE INITIALIZE operation
					memcpy(tempBuf, buf + 2, 5); // length of file name, 5 bytes
					tempBuf[5] = 0;
					decoded = strtol(tempBuf, NULL, 10);
					memcpy(tempBuf, buf + 7, (size_t) decoded); // file name
					tempBuf[decoded] = 0;
					fileSize = strtol(buf + 7 + decoded, NULL, 10); // length of file, 10 bytes

					pFile = new std::ofstream(tempBuf, std::ios::out | std::ios::trunc | std::ios::binary);
					strcpy(buf, "PE"); // PUT FILE ERROR operation
					if (!pFile->is_open())
						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");
				}

				else if (!memcmp(buf, "PD", 2) && pFile->is_open()) { // PUT FILE DATA operation
					memcpy(tempBuf, buf + 2, 5); // length of data, 5 bytes
					tempBuf[5] = 0;
					decoded = strtol(tempBuf, NULL, 10);
					pFile->write(buf + 7, decoded); // data to send
				}

				else if (!memcmp(buf, "PF", 2) && pFile->is_open()) { // PUT FILE FINALIZE operation
					if (pFile->tellp() == fileSize) {
						memcpy(buf, "MP", 2); // MESSAGE PRINT STDOUT operation
						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");
					}
					else {
						strcpy(buf, "ME"); // MESSAGE PRINT STDERR operation
						strcat(buf, "server's file and client's file are different\n"); // message to print
						if (forcewrite(clieFd, buf, 2048) < 0)
							perror("write() error");
					}
					pFile->close();
				}

				else if (!memcmp(buf, "LR", 2)) { // LIST FILES REQUEST operation
					strcpy(buf, "MP"); // MESSAGE PRINT STDOUT operation
					// TODO: list all files belongs to that client id
					if (forcewrite(clieFd, buf, 2048) < 0)
						perror("write() error");
				}

				else if (!memcmp(buf, "IS", 2)) { // CLIENT ID SET operation
					memcpy(tempBuf, buf + 2, 5); // client id, 5 bytes
					tempBuf[5] = 0;

					if (permanentIds.find(clientId) == permanentIds.end()) {
						// TODO: delete all files belongs to that client id
					}
					permanentIds.insert(tempBuf);

					len = 0;
					it = clients.erase(it);
					clients[tempBuf] = clieFd;
				}

				else {
					strcpy(buf, "ME"); // MESSAGE PRINT STDERR operation
					strcat(buf, "invalid request\n"); // message to print
					if (forcewrite(clieFd, buf, 2048) < 0)
						perror("write() error");
				}

				if (--readyNum <= 0)
					break;
			}

			if (len)
				++it;
		}
	}
}
