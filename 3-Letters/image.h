#include <stdio.h>
#include <iostream>

class image{

private:
	char _fileName[30],_header[30];
	int _rows,_cols,_bytes;
	unsigned char *_pPixels;
	unsigned char **_ppPixels;

	unsigned char *readImage(FILE *fpt);

public:

	image(FILE *fpt);
	image(char iFileName[30]);
	~image();
	int getRows();
	int getCols();
	int getBytes();
	unsigned char *getPixels();
	unsigned char **getppPixels();
};