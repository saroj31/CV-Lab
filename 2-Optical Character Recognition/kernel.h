#include <stdio.h>
#include <iostream>

class kernel{

private:
	char _fileName[30],_header[30];
	int _rows,_cols,_bytes;
	unsigned char **_ppPixels;
	unsigned char *_pPixels;

	unsigned char *readKernelImage(FILE *fpt);

public:

	kernel(FILE *fpt);
	kernel(char ifileName[30]);
	~kernel();
	int getRows();
	int getCols();
	int getBytes();
	unsigned char **getPixels2dArray();
	unsigned char *getPixels1dArray();
};