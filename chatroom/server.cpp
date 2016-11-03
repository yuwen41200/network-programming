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
#include "../include/strutils.h"

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

				else {
					std::istringstream tokenizer(buf);
					std::vector<std::string> tokens;
					std::string token;
					while (tokenizer >> token)
						tokens.push_back(token);
					if (tokens.empty())
						tokens.push_back("null");

					if (tokens.front() == "who" && tokens.size() == 1) {
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

					else if (tokens.front() == "name" && tokens.size() == 2) {
						if (tokens[1] == "anonymous") {
							strcpy(buf, "[Server] ERROR: Username cannot be anonymous.\n");
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}

						else if (clients.find(tokens[1]) != clients.end()) {
							sprintf(buf, "[Server] ERROR: %s has been used by others.\n", tokens[1].c_str());
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}

						else if (2 <= tokens[1].size() && tokens[1].size() <= 12) {
							bool alpha = true;
							for (size_t i = 0; i < tokens[1].size(); ++i)
								if (!isalpha(tokens[1][i]))
									alpha = false;

							if (alpha) {
								const char *usr;
								if (!it->first.compare(0, 11, "_anonymous_"))
									usr = "anonymous";
								else
									usr = it->first.c_str();

								for (auto i = clients.begin(); i != clients.end(); ++i) {
									if (i->second == clieFd)
										sprintf(buf, "[Server] You're now known as %s.\n", tokens[1].c_str());
									else
										sprintf(buf, "[Server] %s is now known as %s.\n", usr, tokens[1].c_str());

									if (forcewrite(i->second, buf, strlen(buf) + 1) < 0)
										perror("write() error");
								}

								len = 0;
								it = clients.erase(it);
								clients[tokens[1]] = clieFd;
							}

							else {
								strcpy(buf, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
								if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
									perror("write() error");
							}
						}

						else {
							strcpy(buf, "[Server] ERROR: Username can only consists of 2~12 English letters.\n");
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}
					}

					else if (tokens.front() == "tell" && tokens.size() >= 3) {
						if (!it->first.compare(0, 11, "_anonymous_")) {
							strcpy(buf, "[Server] ERROR: You are anonymous.\n");
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}

						else if (tokens[1] == "anonymous") {
							strcpy(buf, "[Server] ERROR: The client to which you sent is anonymous.\n");
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}

						else if (clients.find(tokens[1]) == clients.end()) {
							strcpy(buf, "[Server] ERROR: The receiver doesn't exist.\n");
							if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
								perror("write() error");
						}

						else {
							tokenizer.str(buf);
							tokenizer.clear();
							tokenizer >> token;
							tokenizer >> token;
							std::getline(tokenizer, token);
							trim(token);

							sprintf(buf, "[Server] %s tell you %s\n", it->first.c_str(), token.c_str());
							if (forcewrite(clients[tokens[1]], buf, strlen(buf) + 1) < 0)
								perror("write() error");
							else {
								strcpy(buf, "[Server] SUCCESS: Your message has been sent.\n");
								if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
									perror("write() error");
							}
						}
					}

					else {
						strcpy(buf, "[Server] ERROR: Error command.\n");
						if (forcewrite(clieFd, buf, strlen(buf) + 1) < 0)
							perror("write() error");
					}
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
