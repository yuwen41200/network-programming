/**
 * FILE TRANSFER USING NON-BLOCKING INPUT/OUTPUT
 */

#ifndef _FNBIO_H
#define _FNBIO_H

#include <string>

class Connection {
private:
	bool alive = true;
	int cliFd;
	char buf[2048];
	char tempBuf[128];
	long decoded;
	long fileSize = -1;
	std::ofstream *pFile = NULL;

public:
	void init(int);
	bool isAlive();
	std::string getFile();
	bool putFile(std::string);
	std::string getName();
};

#endif
