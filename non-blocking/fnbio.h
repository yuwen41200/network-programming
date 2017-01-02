/**
 * FILE TRANSFER USING NON-BLOCKING INPUT/OUTPUT
 */

#ifndef _FNBIO_H
#define _FNBIO_H

#include <string>
#include <fstream>

class Connection {
private:
	int cliFd;
	std::string cliName;
	bool alive;

	std::ifstream ifs;
	std::ofstream ofs;
	char buf[2048];
	char tempBuf[128];

	struct stat statBuf;
	long fileSize;
	std::string cliFilename;
	ssize_t len;
	long decoded;

	void *ptr;
	size_t size_left;
	ssize_t	size_read;
	ssize_t size_written;

	int extStat;
	int intStat;

	ssize_t	forceread(int, void *, size_t);
	ssize_t	forcewrite(int, void *, size_t);

public:
	void init(int, std::string);
	bool isAlive();
	std::string getFile();
	bool putFile(std::string);
	std::string getName();
};

#endif
