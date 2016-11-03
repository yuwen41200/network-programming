#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <random>
#include <sstream>
#include "../include/netutils.h"

int main() {
	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(57345);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFd, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind() error");
		return 1;
	}

	if (listen(sockFd, 16) < 0)
		perror("listen() error");

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, std::numeric_limits<int>::max());
	std::map<std::string, int> clients;

	char buf[2048];
	fd_set allFds;
	FD_ZERO(&allFds);
	FD_SET(sockFd, &allFds);
	int maxFd = sockFd;

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {
		int readyNum;
		fd_set readFds = allFds;
		if ((readyNum = select(maxFd + 1, &readFds, NULL, NULL, NULL)) < 0)
			perror("select() error");

		if (FD_ISSET(sockFd, &readFds)) {
			int connFd;
			struct sockaddr_in clieAddr;
			socklen_t clieLen = sizeof(clieAddr);
			if ((connFd = accept(sockFd, (struct sockaddr *) &clieAddr, &clieLen)) < 0)
				perror("accept() error");

			if (clients.size() == FD_SETSIZE) {
				fprintf(stderr, "maximum number of clients reached\n");
				continue;
			}

			while (1) {
				int rdNum = dis(gen);
				std::string key = "_anonymous_" + std::to_string(rdNum);
				auto search = clients.find(key);
				if (search == clients.end()) {
					clients[key] = connFd;
					break;
				}
			}

			FD_SET(connFd, &allFds);
			if (connFd > maxFd)
				maxFd = connFd;

			char ip[128];
			inet_ntop(AF_INET, &clieAddr.sin_addr, ip, sizeof(ip));
			int port = ntohs(clieAddr.sin_port);
			sprintf(buf, "[Server] Hello, anonymous! From: %s/%d\n", ip, port);
			if (forcewrite(connFd, buf, strlen(buf) + 1) < 0)
				perror("write() error");

			strcpy(buf, "[Server] Someone is coming!\n");
			for (auto it = clients.begin(); it != clients.end(); ++it)
				if (it->second != connFd)
					if (forcewrite(it->second, buf, strlen(buf) + 1) < 0)
						perror("write() error");

			if (--readyNum <= 0)
				continue;
		}

		for (auto it = clients.begin(); it != clients.end(); ) {
			int clieFd = it->second;
			ssize_t len = -1;

			if (FD_ISSET(clieFd, &readFds)) {
				if ((len = forceread(clieFd, buf, 2048)) < 0)
					perror("read() error");

				else if (len == 0) {
					const char *usr;
					if (!it->first.compare(0, 11, "_anonymous_"))
						usr = "anonymous";
					else
						usr = it->first.c_str();

					sprintf(buf, "[Server] %s is offline.\n", usr);
					for (auto i = clients.begin(); i != clients.end(); ++i)
						if (i->second != clieFd)
							if (forcewrite(i->second, buf, strlen(buf) + 1) < 0)
								perror("write() error");

					if (close(clieFd) < 0)
						perror("close() error");
					FD_CLR(clieFd, &allFds);
					it = clients.erase(it);
				}

					else if (!strcmp(buf, "who\n")) {
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

					else {
						strcpy(buf, "[Server] ERROR: Error command.\n");
						if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
							perror("write() error");
					}

				if (--readyNum <= 0)
					break;
			}

			if (len != 0)
				++it;
		}
	}
	#pragma clang diagnostic pop
}
