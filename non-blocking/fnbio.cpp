#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include "../include/netutils.h"
#include "../include/strutils.h"
#include "fnbio.h"

void Connection::init(int cliFd) {
	this->cliFd = cliFd;
}

bool Connection::isAlive() {
	return alive;
}

std::string Connection::getFile() {
	if (!memcmp(buf, "PI", 2)) { // PUT FILE INITIALIZE operation
		memcpy(tempBuf, buf + 2, 5); // length of file name, 5 bytes
		tempBuf[5] = 0;
		decoded = strtol(tempBuf, NULL, 10);
		memcpy(tempBuf, buf + 7, (size_t) decoded); // file name
		tempBuf[decoded] = 0;
		fileSize = strtol(buf + 7 + decoded, NULL, 10); // length of file, 10 bytes

		pFile = new std::ofstream(tempBuf, std::ios::out | std::ios::trunc | std::ios::binary);
		strcpy(buf, "PE"); // PUT FILE ERROR operation
		if (!pFile->is_open())
			if (forcewrite(cliFd, buf, 2048) < 0)
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
			if (forcewrite(cliFd, buf, 2048) < 0)
				perror("write() error");
		}
		else {
			strcpy(buf, "ME"); // MESSAGE PRINT STDERR operation
			strcat(buf, "server's file and client's file are different\n");
			// message to print
			if (forcewrite(cliFd, buf, 2048) < 0)
				perror("write() error");
		}
		pFile->close();
	}

	return "filename";
}

bool Connection::putFile(std::string filename) {
	if (!memcmp(buf, "GI", 2)) { // GET FILE INITIALIZE operation
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
			if (forcewrite(cliFd, buf, 2048) < 0)
				perror("write() error");

		while (file.good()) {
			file.read(buf + 7, 1920); // data to receive
			strcpy(buf, "GD"); // GET FILE DATA operation
			sprintf(tempBuf, "%05d", (int) file.gcount());
			memcpy(buf + 2, tempBuf, 5); // length of data, 5 bytes

			if (forcewrite(cliFd, buf, 2048) < 0)
				perror("write() error");
		}

		if (memcmp(buf, "GE", 2)) {
			strcpy(buf, "GF"); // GET FILE FINALIZE operation
			sprintf(tempBuf, "%010d", (int) statBuf.st_size);
			strcat(buf, tempBuf); // length of file, 10 bytes
			strcat(buf, success.c_str()); // success message

			if (forcewrite(cliFd, buf, 2048) < 0)
				perror("write() error");

			file.close();
		}
	}

	return true;
}

std::string Connection::getName() {
	if (!memcmp(buf, "IS", 2)) { // CLIENT ID SET operation
		memcpy(tempBuf, buf + 2, 5); // client id, 5 bytes
		tempBuf[5] = 0;
	}

	return "name";
}
