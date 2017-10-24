#include "image.h"
#include <cstring>
#include <cstdlib>

unsigned char *image::readImage(FILE *fpt){

		char ws;
		unsigned char *pImgPixels = new unsigned char[_rows*_cols];
		unsigned char **ppImgPixels = new unsigned char*[_rows];
		fread(pImgPixels,1,_rows*_cols,fpt);

		//Having a double array version for better operations
		for(int i=0;i<_rows;i++){
			ppImgPixels[i]=pImgPixels+(i*_cols);
		}

		_ppPixels = ppImgPixels;

		return pImgPixels;
}

image::image(){
	_ppPixels = NULL;
	_pPixels = NULL;
}

image::image(FILE *fpt):_rows(0),_cols(0){

	fscanf(fpt,"%s %d %d %d\n",_header,&_cols,&_rows,&_bytes);

	_pPixels = readImage(fpt);
}

image::image(char iFileName[30]):_rows(0),_cols(0){

	FILE *fpt=NULL;
	
	if ((fpt=fopen(iFileName,"rb")) == NULL)
  {
    std::cout<<"Unable to open <imageFileName>.ppm for reading\n";
    exit(0);
  }

  fscanf(fpt,"%s %d %d %d\n",_header,&_cols,&_rows,&_bytes);
	_pPixels = readImage(fpt);
	strcpy(_fileName,iFileName);
	fclose(fpt);

}

image::~image(){
	delete _ppPixels;
	delete _pPixels;
}


void image::operator=(image &inImg){

	
	_rows = inImg.getRows();
	_cols = inImg.getCols();
	_bytes = inImg.getBytes();
	_pPixels = new unsigned char[_rows*_cols];
	_ppPixels = new unsigned char*[_rows];

	unsigned char *inPixels = inImg.getPixels();
	unsigned char **inPPixels = inImg.getppPixels();

	for(int r=0;r<_rows;r++){

		_ppPixels[r] = _pPixels+(r*_cols);
		for(int c = 0;c<_cols;c++){
			_pPixels[r*_cols + c] = inPixels[r*_cols + c];
		}
	}


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

unsigned char **image::getppPixels(){
	return _ppPixels;
}

