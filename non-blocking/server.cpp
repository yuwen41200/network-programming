#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <random>
#include <string>
#include <deque>
#include <map>
#include "fnbio.h"

class Client {
public:
	std::string name;
	std::deque<std::string> jobQueue;
	Connection conn;
};

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [SERVER PORT]\n", argv[0]);
		return 1;
	}

	int sockFd;
	if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error");
		return 1;
	}

	int flags = fcntl(sockFd, F_GETFL, 0);
	if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl() error");
		return 1;
	};

	struct sockaddr_in srvAddr;
	bzero(&srvAddr, sizeof(srvAddr));
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons((uint16_t) std::stoul(argv[1]));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockFd, (const struct sockaddr *) &srvAddr, sizeof(srvAddr)) < 0) {
		perror("bind() error");
		return 1;
	}

	if (listen(sockFd, 16) < 0) {
		perror("listen() error");
		return 1;
	}

	std::map<int, Client> clients;

	while (1) {
		int connFd = accept(sockFd, (struct sockaddr *) NULL, NULL);

		if (connFd >= 0) {
			flags = fcntl(connFd, F_GETFL, 0);
			if (fcntl(connFd, F_SETFL, flags | O_NONBLOCK) < 0)
				perror("fcntl() error");

			clients[connFd].name = "magic_string_pending";
			clients[connFd].jobQueue.push_back("magic_string_pending");
			clients[connFd].conn.init(connFd, "magic_string_pending");
		}

		else if (errno != EWOULDBLOCK && errno != EAGAIN)
			perror("accept() error");

		for (auto client: clients) {
			DIR *dp;
			struct dirent *de;
			std::string str;

			if (!client.second.conn.isAlive()) {
				dp = opendir(".");
				str = client.second.name;
				while ((de = readdir(dp)))
					if (!strncmp(de->d_name, str.c_str(), str.size()))
						unlink(de->d_name);
				closedir(dp);

				if (close(client.first) < 0)
					perror("close() error");

				clients.erase(client.first);
				break;
			}

			else if (client.second.jobQueue.empty())
				if ((str = client.second.conn.getFile()) != "")
					for (auto i: clients)
						if (i.second.name == client.second.name && i.first != client.first)
							i.second.jobQueue.push_back(str);

			else if (client.second.jobQueue.front() == "magic_string_pending") {
				if ((str = client.second.conn.getName()) != "") {
					client.second.name = str;
					client.second.jobQueue.pop_front();
					client.second.conn.init(client.first, str);

					dp = opendir(".");
					while ((de = readdir(dp)))
						if (!strncmp(de->d_name, str.c_str(), str.size()))
							client.second.jobQueue.push_back(std::string(de->d_name));
					closedir(dp);
				}
			}

			else if ((str = client.second.jobQueue.front()) != "")
				if (client.second.conn.putFile(str))
					client.second.jobQueue.pop_front();
		}
	}
}
