#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fstream>
#include "fnbio.h"

void Connection::init(int cliFd, std::string cliName) {
	this->cliFd = cliFd;
	this->cliName = cliName;
	this->alive = true;
	this->extStat = 0;
	this->intStat = 0;
}

bool Connection::isAlive() {
	return alive;
}

std::string Connection::getFile() {
	if ((len = forceread(cliFd, buf, 2048)) <= 0) {
		if (len == 0)
			alive = false;
	}

	else if (!memcmp(buf, "PI", 2)) { // PUT FILE INITIALIZE operation
		memcpy(tempBuf, buf + 2, 5); // length of file name, 5 bytes
		tempBuf[5] = 0;
		decoded = strtol(tempBuf, NULL, 10);
		memcpy(tempBuf, buf + 7, (size_t) decoded); // original file name
		tempBuf[decoded] = 0;
		fileSize = strtol(buf + 7 + decoded, NULL, 10); // length of file, 10 bytes
		cliFilename = cliName + tempBuf;
		if (ofs.is_open())
			ofs.close();
		ofs.open(cliFilename, std::ios::out | std::ios::trunc | std::ios::binary);
	}

	else if (!memcmp(buf, "PD", 2) && ofs.is_open()) { // PUT FILE DATA operation
		memcpy(tempBuf, buf + 2, 5); // length of data, 5 bytes
		tempBuf[5] = 0;
		decoded = strtol(tempBuf, NULL, 10);
		ofs.write(buf + 7, decoded); // data to send
	}

	else if (!memcmp(buf, "PF", 2) && ofs.is_open()) { // PUT FILE FINALIZE operation
		if (ofs.tellp() != fileSize)
			fprintf(stderr, "server's file and client's file are different\n");
		ofs.close();
		return cliFilename;
	}

	return "";
}

bool Connection::putFile(std::string filename) {
	switch (extStat) {
		case 0:
			goto EXT_STAT_0;
		case 1:
			goto EXT_STAT_1;
		case 2:
			goto EXT_STAT_2;
		case 3:
			goto EXT_STAT_3;
		case 4:
			goto EXT_STAT_4;
		case 5:
			goto EXT_STAT_5;
		default:
			extStat = 0;
			goto EXT_STAT_0;
	}

	EXT_STAT_0:
	ifs.open(filename, std::ios::in | std::ios::binary);
	stat(filename.c_str(), &statBuf);
	fileSize = statBuf.st_size;
	cliFilename = filename.substr(cliName.size());

	strcpy(buf, "GI"); // GET FILE INITIALIZE operation
	sprintf(tempBuf, "%05d", (int) cliFilename.size());
	strcat(buf, tempBuf); // length of file name, 5 bytes
	strcat(buf, cliFilename.c_str()); // original file name
	sprintf(tempBuf, "%010d", (int) fileSize);
	strcat(buf, tempBuf); // length of file, 10 bytes

	EXT_STAT_1:
	extStat = forcewrite(cliFd, buf, 2048) < 0 ? 1 : 2;
	return false;
	EXT_STAT_2:

	while (ifs.good()) {
		ifs.read(buf + 7, 1920); // data to receive
		strcpy(buf, "GD"); // GET FILE DATA operation
		sprintf(tempBuf, "%05d", (int) ifs.gcount());
		memcpy(buf + 2, tempBuf, 5); // length of data, 5 bytes

		EXT_STAT_3:
		extStat = forcewrite(cliFd, buf, 2048) < 0 ? 3 : 4;
		return false;
		EXT_STAT_4:;
	}

	strcpy(buf, "GF"); // GET FILE FINALIZE operation
	ifs.close();

	EXT_STAT_5:
	if (forcewrite(cliFd, buf, 2048) < 0) {
		extStat = 5;
		return false;
	}
	else {
		extStat = 0;
		return true;
	}
}

std::string Connection::getName() {
	if ((len = forceread(cliFd, buf, 2048)) <= 0) {
		if (len == 0)
			alive = false;
	}

	else if (!memcmp(buf, "IS", 2)) { // CLIENT ID SET operation
		memcpy(tempBuf, buf + 2, 5); // length of client id, 5 bytes
		tempBuf[5] = 0;
		decoded = strtol(tempBuf, NULL, 10);
		memcpy(tempBuf, buf + 7, (size_t) decoded); // client id
		tempBuf[decoded] = 0;
		return tempBuf;
	}

	return "";
}

ssize_t Connection::forceread(int fd, void *new_ptr, size_t size) {
	switch (intStat) {
		case 0:
			goto INT_STAT_0;
		case 1:
			goto INT_STAT_1;
		default:
			intStat = 0;
			goto INT_STAT_0;
	}

	INT_STAT_0:
	ptr = new_ptr;
	size_left = size;

	INT_STAT_1:
	if (size_left > 0) {
		size_read = read(fd, ptr, size_left);
		if (size_read < 0) {
			if (errno == EINTR)
				size_read = 0;
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
				return -1;
			else {
				perror("read() error");
				return -1;
			}
		}
		else if (size_read == 0)
			return 0;
		size_left -= size_read;
		ptr = (char *) ptr + size_read;
	}

	if (size_left > 0) {
		intStat = 1;
		return -1;
	}
	else {
		intStat = 0;
		return 1;
	}
}

ssize_t Connection::forcewrite(int fd, void *new_ptr, size_t size) {
	switch (intStat) {
		case 0:
			goto INT_STAT_0;
		case 1:
			goto INT_STAT_1;
		default:
			intStat = 0;
			goto INT_STAT_0;
	}

	INT_STAT_0:
	ptr = new_ptr;
	size_left = size;

	INT_STAT_1:
	if (size_left > 0) {
		size_written = write(fd, ptr, size_left);
		if (size_written < 0) {
			if (errno == EINTR)
				size_written = 0;
			else if (errno == EWOULDBLOCK || errno == EAGAIN)
				return -1;
			else {
				perror("write() error");
				return -1;
			}
		}
		else if (size_written == 0)
			return -1;
		size_left -= size_written;
		ptr = (char *) ptr + size_written;
	}

	if (size_left > 0) {
		intStat = 1;
		return -1;
	}
	else {
		intStat = 0;
		return 1;
	}
}
