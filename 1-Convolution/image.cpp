#include "image.h"

unsigned char *image::readImage(FILE *fpt){

		char ws;
		unsigned char *pImgPixels = new unsigned char[_rows*_cols];
		ws = fgetc(fpt);
		fread(pImgPixels,1,_rows*_cols,fpt);

		return pImgPixels;
}

image::image(FILE *fpt):_rows(0),_cols(0){

	fscanf(fpt,"%s %d %d %d\n",_header,&_cols,&_rows,&_bytes);

	_pPixels = readImage(fpt);
}

image::~image(){
	delete _pPixels;
}


int image::getRows(){
	return _rows;
}

int image::getCols(){
	return _cols;
}

int image::getBytes(){
	return _bytes;
}

unsigned char *image::getPixels(){
	return _pPixels;
}

