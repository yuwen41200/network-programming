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
bool done = false;

void recvWindow(int fd, unsigned baseSeqNo, char *data, unsigned *dataLen);
void finalize(int fd);

int main(int argc, char **argv) {
	if (argc != 3)
		fprintf(stderr, "usage: %s [receiver port] [output file]\n", argv[0]);

	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("socket() error");

	struct sockaddr_in sa;
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((uint16_t) strtoul(argv[1], NULL, 10));
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (const struct sockaddr *) &sa, sizeof(sa)) < 0)
		perror("bind() error");

	std::ofstream fs(argv[2], std::ios::out | std::ios::trunc | std::ios::binary);
	if (!fs.is_open())
		fprintf(stderr, "cannot write %s\n", argv[2]);

	unsigned len, no = 0;
	char data[129024];
	while (!done) {
		recvWindow(fd, no, data, &len);
		fs.write(data, len);
		no += 128;
	}

	fs.close();
	finalize(fd);

	return 0;
}

void recvWindow(int fd, unsigned baseSeqNo, char *data, unsigned *dataLen) {
	std::vector<unsigned> remainingPackets(128);
	for (unsigned i = 0; i < 128; ++i)
		remainingPackets[i] = i;
	*dataLen = 0;

	while (!remainingPackets.empty()) {
		// receive the packet, maximum 1024 bytes
		struct sockaddr_in ca;
		socklen_t caLen = sizeof(ca);
		if (recvfrom(fd, buf, 8, MSG_PEEK, (struct sockaddr *) &ca, &caLen) < 0)
			perror("recvfrom() error");

		// FIN
		//   tell receiver to close, in text, 8 bytes
		strcpy(tmp, "FFFFFFFF");
		if (!memcmp(buf, tmp, 8)) {
			done = true;
			break;
		}

		// SEQ_NO
		//   sequence number, in text, 8 bytes
		buf[8] = 0;
		unsigned no = (unsigned) strtoul(buf, NULL, 10);

		caLen = sizeof(ca);
		if (recvfrom(fd, buf, 16, MSG_PEEK, (struct sockaddr *) &ca, &caLen) < 0)
			perror("recvfrom() error");

		// LEN
		//   length of data, in text, 8 bytes
		buf[16] = 0;
		unsigned len = (unsigned) strtoul(buf + 8, NULL, 10);

		caLen = sizeof(ca);
		if (recvfrom(fd, buf, len + 16, 0, (struct sockaddr *) &ca, &caLen) < 0)
			perror("recvfrom() error");

		if (no < baseSeqNo) {
			// ACK_NO
			//   acknowledgement number, in text, 8 bytes
			// send the packet, always 8 bytes
			if (sendto(fd, buf, 8, 0, (const struct sockaddr *) &ca, caLen) < 0)
				perror("sendto() error");
			continue;
		}

		if (no >= baseSeqNo + 128)
			continue;

		auto it = std::find(remainingPackets.begin(), remainingPackets.end(), no - baseSeqNo);
		if (it == remainingPackets.end()) {
			// ACK_NO
			//   acknowledgement number, in text, 8 bytes
			// send the packet, always 8 bytes
			if (sendto(fd, buf, 8, 0, (const struct sockaddr *) &ca, caLen) < 0)
				perror("sendto() error");
			continue;
		}

		// DATA
		//   data, in binary, LEN bytes
		memcpy(data + *it * 1008, buf + 16, len);
		remainingPackets.erase(it);
		*dataLen += len;

		// ACK_NO
		//   acknowledgement number, in text, 8 bytes
		// send the packet, always 8 bytes
		if (sendto(fd, buf, 8, 0, (const struct sockaddr *) &ca, caLen) < 0)
			perror("sendto() error");
	}
}

void finalize(int fd) {
	struct sockaddr_in ca;
	socklen_t caLen = sizeof(ca);
	if (recvfrom(fd, buf, 8, 0, (struct sockaddr *) &ca, &caLen) < 0)
		perror("recvfrom() error");

	int count = 0;
	while (count < 32) {
		count++;

		// FIN_ACK
		//   receiver got it, in text, 8 bytes
		strcpy(buf, "EEEEEEEE");
		if (sendto(fd, buf, 8, 0, (const struct sockaddr *) &ca, caLen) < 0)
			perror("sendto() error");
	}
}
