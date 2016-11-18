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

	char buf[2048];
	fd_set allFds;
	FD_ZERO(&allFds);
	FD_SET(sockFd, &allFds);
	int maxFd = sockFd;

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
					fputs("Client has closed the connection\n", stdout);

					if (permanentIds.find(clientId) == permanentIds.end()) {
						// TODO: delete all files belongs to that client id
					}

					if (close(clieFd) < 0)
						perror("close() error");
					FD_CLR(clieFd, &allFds);
					it = clients.erase(it);
				}

				else if (!memcmp(buf, "GI", 2)) { // GET FILE INITIALIZE operation
					for (auto i = clients.begin(); i != clients.end(); ++i) {
						struct sockaddr_in clieAddr;
						socklen_t clieLen = sizeof(clieAddr);
						if (getpeername(i->second, (struct sockaddr *) &clieAddr, &clieLen) < 0)
							perror("getpeername() error");

						const char *usr;
						if (!i->first.compare(0, 11, "_anonymous_"))
							usr = "anonymous";
						else
							usr = i->first.c_str();

						char ip[128];
						inet_ntop(AF_INET, &clieAddr.sin_addr, ip, sizeof(ip));
						int port = ntohs(clieAddr.sin_port);
						if (i->first == it->first)
							sprintf(buf, "[Server] %s %s/%d ->me\n", usr, ip, port);
						else
							sprintf(buf, "[Server] %s %s/%d\n", usr, ip, port);

						if (forcewrite(clieFd, buf, strlen(buf)) < 0)
							perror("write() error");
					}

					if (forcewrite(clieFd, "", 1) < 0)
						perror("write() error");
				}

				else if (!memcmp(buf, "TD", 2)) { // TODO...
					const char *usr;
					if (!it->first.compare(0, 11, "_anonymous_"))
						usr = "anonymous";
					else
						usr = it->first.c_str();

					std::string token;
					sprintf(buf, "[Server] %s yell %s\n", usr, token.c_str());
					for (auto i = clients.begin(); i != clients.end(); ++i)
						if (forcewrite(i->second, buf, strlen(buf) + 1) < 0)
							perror("write() error");
				}

				else
					fprintf(stderr, "invalid request\n");

				if (--readyNum <= 0)
					break;
			}

			if (len)
				++it;
		}
	}
}
