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

	int extStat;
	int intStat;

public:
	void init(int, std::string);
	bool isAlive();
	std::string getFile();
	bool putFile(std::string);
	std::string getName();
};

#endif
