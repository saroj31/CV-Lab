/*
  This version of 2d Convolution implements a 2d convolution in an image using separable kernel technique.

  We are separating a 7x7 kernel to a 7x1 and 1x7 kernel for our operation.

  It is a C++ code so use the below cocde to compile it:-

  g++ -lrt 2DConvSeparable_v2.cpp image.cpp
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "image.h"

void saveImage(unsigned char *pOutImg, int iC, int iR){

	FILE *fp;
	fp=fopen("img/outv2.ppm","w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iC*iR,1,fp);
	fclose(fp);

}

void diffImage(unsigned char *ip1,unsigned char *ip2,int iC, int iR){

	int diff = -1;
	for(int i=0; i<iR;i++){
		for(int j=0; j<iC; j++){
			diff = ip1[i*iC + j] - ip2[i*iC + j];
			printf("%d    ",diff);
		}
		printf("\n");
	}

}


//Convolves a 1x7 kernel horizontally
//@return: 
//				outPixSum : sum of kernel operation for the mean filter
int kernelSepHor(image &iImg,int iCol,int iRow){
 
	int r,c,kSize = 7;
	int outPixSum = 0;
	int kerR[7] = {1,1,1,1,1,1,1};  
	int sum =0,temp =0;
	unsigned char *pInPixels = iImg.getPixels();

	// horizontal convolution
	for( c=-(kSize/2); c<=(kSize/2); c++)
			sum += pInPixels[(iRow)*iImg.getCols()+(iCol+c)]*kerR[c+(kSize/2)];

	outPixSum = sum;
	//std::cout<<outPixSum<<"   "<<iCol<<"  "<<iRow<<std::endl;

	return outPixSum;
}


//Convolves a 7x1 kernel vertically
//@return: 
//				outPixSum : sum of kernel operation for the mean filter
int kernelSepVer(unsigned int *&pInPixels,int iCol,int iRow,int iW,int iH){

	int r,c,kSize = 7;
	int outPixSum = 0;
	int kerR[7] = {1,1,1,1,1,1,1};
	int sum =0,temp =0;
	
	// vertical convolution
	for( r=-(kSize/2); r<=(kSize/2); r++)
			sum += pInPixels[((iRow+r)*iW)+(iCol)]*kerR[r+(kSize/2)];

	outPixSum = sum;
	//std::cout<<outPix<<"   "<<iCol<<"  "<<iRow<<std::endl;

	return outPixSum;
}


//do the main convolutioon operation here
//whoever uses this method should release the returned pointer
unsigned char *convolveImage(image &iImg){
   unsigned char *opPixels = NULL;
   unsigned int *tmpPixels = NULL;
   int r,c,kSize = 7;
   int sum;

   //allocate memory for the out Pixel pointer
   opPixels = new unsigned char[iImg.getRows() * iImg.getCols()];
   tmpPixels = new unsigned int[iImg.getRows() * iImg.getCols()];

   //Do horizontal convolution
   for(r=0; r<iImg.getRows(); r++)
		for(c=3 ; c<(iImg.getCols()-3); c++)
			tmpPixels[r*iImg.getCols()+c] = kernelSepHor(iImg,c,r);

		//Do vertical convolution
   for(c=3; c<(iImg.getRows()-3); c++)
		for(r=3 ; r<(iImg.getCols()-3); r++)
			opPixels[r*iImg.getCols()+c] = (unsigned char)(kernelSepVer(tmpPixels,c,r,iImg.getCols(),iImg.getRows())/(kSize*kSize));

	 delete(tmpPixels);

   return opPixels;

}


int main(){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  char fileName[30];
  struct timespec tp1,tp2;

  //check the command line argument for the file name
  strcpy(fileName,"img/bridge.ppm");

  // get the file pointer and validate it
  if ((fpt=fopen(fileName,"rb")) == NULL)
  {
    std::cout<<"Unable to open bridge.ppm for reading\n";
    exit(0);
  }

  //load all image data to image object
  image img(fpt);
  fclose(fpt);

  //query timer
  clock_gettime(CLOCK_REALTIME,&tp1);
  //printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

  //Operation on the pixel matrix
  pOutImg = convolveImage(img);

  	/* query timer */
  clock_gettime(CLOCK_REALTIME,&tp2);
  //printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
  
  	/* report how long it took to smooth */
	printf("%ld microsecs\n",(tp2.tv_nsec-tp1.tv_nsec)/1000);  
  
  //Save the new image as out.ppm image
  saveImage(pOutImg,img.getCols(),img.getRows());

  delete pOutImg;

  return 0;

}