#include "kernel.h"
#include <iostream>
#include <cstdlib>
#include <string.h>

unsigned char *kernel::readKernelImage(FILE *fpt){

		char ws;
		unsigned char *pKPix = new unsigned char[_rows*_cols];
		unsigned char **ppKPixels = new unsigned char*[_rows];

		//ws = fgetc(fpt);  //kernel images doesn't have the whitepace
		fread(pKPix,1,_rows*_cols,fpt);

		//Having a double array version for better operations
		for(int i=0;i<_rows;i++){
			ppKPixels[i]=pKPix+(i*_cols);
		}

		_ppPixels = ppKPixels;

		return pKPix;
}

kernel::kernel(FILE *fpt):_rows(0),_cols(0){

	fscanf(fpt,"%s %d %d %d\n",_header,&_cols,&_rows,&_bytes);

	_pPixels = readKernelImage(fpt);
}

kernel::kernel(char ifileName[30]):_rows(0),_cols(0){

	FILE *fpt=NULL;
	if ((fpt=fopen(ifileName,"rb")) == NULL)
  {
    std::cout<<"Unable to open <kernelFileName>.ppm for reading\n";
    exit(0);
  }


	fscanf(fpt,"%s %d %d %d\n",_header,&_cols,&_rows,&_bytes);

	_pPixels = readKernelImage(fpt);
	strcpy(_fileName,ifileName);

	fclose(fpt);
}

kernel::~kernel(){
	
	delete _ppPixels;
	delete _pPixels;
}


int kernel::getRows(){
	return _rows;
}

int kernel::getCols(){
	return _cols;
}

int kernel::getBytes(){
	return _bytes;
}

unsigned char *kernel::getPixels1dArray(){
	return _pPixels;
}

unsigned char **kernel::getPixels2dArray(){
	return _ppPixels;
}

