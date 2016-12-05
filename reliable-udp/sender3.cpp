#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

char buf[1024];
char tmp[256];

void sendWindow(int fd, unsigned baseSeqNo, const char *data, unsigned dataLen);
void finalize(int fd);

int main(int argc, char **argv) {
	if (argc != 4)
		fprintf(stderr, "usage: %s [receiver address] [receiver port] [input file]\n", argv[0]);

	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in sa;
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((uint16_t) strtoul(argv[2], NULL, 0));
	if (inet_pton(AF_INET, argv[1], &sa.sin_addr) <= 0)
		perror("inet_pton() error");

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt() error");

	if (connect(fd, (const struct sockaddr *) &sa, sizeof(sa)) < 0)
		perror("connect() error");

	std::ifstream fs(argv[3], std::ios::in | std::ios::binary);
	if (!fs.is_open())
		fprintf(stderr, "cannot read %s\n", argv[3]);

	unsigned no = 0;
	char data[129024];
	while (fs.good()) {
		fs.read(data, 129024);
		sendWindow(fd, no, data, (unsigned) fs.gcount());
		no += 128;
	}

	fs.close();
	finalize(fd);

	return 0;
}

void sendWindow(int fd, unsigned baseSeqNo, const char *data, unsigned dataLen) {
	std::vector<unsigned> remainingPackets(128);
	for (unsigned i = 0; i < 128; ++i)
		remainingPackets[i] = i;

	while (true) {
		for (auto i = remainingPackets.begin(); i != remainingPackets.end(); ++i) {
			// SEQ_NO
			//   sequence number, in text, 8 bytes
			sprintf(buf, "%08d", baseSeqNo + *i);

			// LEN
			//   length of data, in text, 8 bytes
			int slen = dataLen - *i * 1008;
			slen = slen < 0 ? 0 : slen > 1008 ? 1008 : slen;
			unsigned len = (unsigned) slen;
			sprintf(tmp, "%08d", len);
			strcat(buf, tmp);

			// DATA
			//   data, in binary, LEN bytes
			memcpy(buf + 16, data + *i * 1008, len);

			// send the packet, maximum 1024 bytes
			if (send(fd, buf, len + 16, 0) < 0)
				perror("send() error");
		}

		bool done = false;
		while (true) {
			if (remainingPackets.empty()) {
				done = true;
				break;
			}

			// receive the packet, always 8 bytes
			if (recv(fd, buf, 8, 0) < 0) {
				if (errno == EWOULDBLOCK)
					break;
				else
					perror("recv() error");
			}

			// ACK_NO
			//   acknowledgement number, in text, 8 bytes
			buf[8] = 0;
			unsigned no = (unsigned) strtoul(buf, NULL, 0);

			if (no < baseSeqNo)
				continue;
			auto it = std::find(remainingPackets.begin(), remainingPackets.end(), no - baseSeqNo);
			if (it != remainingPackets.end())
				remainingPackets.erase(it);
		}

		if (done)
			break;
	}
}

void finalize(int fd) {
	int count = 0;
	while (count < 64) {
		count++;

		// FIN
		//   tell receiver to close, in text, 8 bytes
		strcpy(buf, "FFFFFFFF");
		if (send(fd, buf, 8, 0) < 0)
			perror("send() error");

		if (recv(fd, buf, 8, 0) < 0) {
			if (errno != EWOULDBLOCK)
				perror("recv() error");
			continue;
		}

		// FIN_ACK
		//   receiver got it, in text, 8 bytes
		strcpy(tmp, "EEEEEEEE");
		if (!memcmp(buf, tmp, 8))
			break;
	}
}
