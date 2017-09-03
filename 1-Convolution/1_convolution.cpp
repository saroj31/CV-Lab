/*
** This program reads bridge.ppm, a 512 x 512 PPM image.
** It smooths it using a standard 7x7 mean filter.
** The program also times the piece of code.
**
** To compile, must link using -lrt  (man clock_gettime() function).
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "image.h"

void saveImage(unsigned char *pOutImg, int iC, int iR){

	FILE *fp;
	fp=fopen("img/out.ppm","w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iC*iR,1,fp);
	fclose(fp);

}

//Lets separate the kernel operation
int kernel2dConv(image &iImg,int iCol,int iRow,bool ibSum = false){

	int r,c,sum=0;
	int outPixel = 0;
	int ker[7][7] = {
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1}
					};
	int kSize = 7;
	int kSum = kSize*kSize;
	unsigned char *pInPixels = iImg.getPixels();

	for( r=-(kSize/2); r<=(kSize/2); r++)
	{
		for(c=-(kSize/2); c<=(kSize/2); c++){

			sum += (pInPixels[(iRow+r)*iImg.getCols() + (iCol+c)]*ker[c+(kSize/2)][r+(kSize/2)]);
		}
	}

	if(!ibSum)
		outPixel = sum/kSum;
	else	
		outPixel = sum;
	
	return outPixel;
}

//kernel multiplication with separable filters
int kernelSep(image &iImg,int iCol,int iRow){

	int r,c,sum=0;
	int outPix = 0,temp =0;
	int outPix1[7];
	int kerR[7] = {1,1,1,1,1,1,1};
	int kSize = 7;
	unsigned char *pInPixels = iImg.getPixels();

	// column convolution
	for( c= 0; c<kSize; c++){	
		sum = 0;
		for( r=-(kSize/2); r<=(kSize/2); r++)
			sum += pInPixels[(iRow+r)*iImg.getCols()+(iCol+c)]*kerR[r+(kSize/2)];

		temp = sum/kSize;
		outPix1[c] = temp;
	}

	//horizontal convolution
	sum = 0;
	for( r=0; r<=(kSize); r++)
		sum += outPix1[r]*kerR[r];

	outPix = sum/kSize;

	return outPix;
}


//Kernel sliding window algorithm
int kernelSlideWin(image &iImg,int iCol,int iRow){

	int outPix=0;
	static int prev1stColSum = 0,prevSum = 0;
	int sum = 0;
	unsigned char *pInPixels = iImg.getPixels();

	//kernel details
	int ker[7][7] = {
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1},
					  {1,1,1,1,1,1,1}
					};
	int kSum = 49;
	int kSize = 7;

	//first pixel of the row, iCol = 3 
	if( 3 == iCol){
		//Compute the full kernel and save the prev1stColSum and the prevSum
		for( int c=-(kSize/2); c<=(kSize/2); c++)
		{
			if( (c+(kSize/2)) == 1 )//save the prev1stColSum
				prev1stColSum = sum;

			for(int r=-(kSize/2); r<=(kSize/2); r++){

				sum += (pInPixels[(iRow+r)*iImg.getCols() + (iCol+c)]*ker[r+(kSize/2)][c+(kSize/2)]);
			}
		}
		//save the prevSum
		prevSum = sum;

	}//not a first pixel of the row, iCol!=3
	else{
		int lastColSum=0,nb1stColSum=0;
		//Compute the last col sum, lastColSum
		for(int i=-(kSize/2); i<=(kSize/2); i++){

				lastColSum  += (pInPixels[(iRow+i)*iImg.getCols() + (iCol+(kSize/2))]*ker[i+(kSize/2)][kSize-1]);
				nb1stColSum += (pInPixels[(iRow+i)*iImg.getCols() + (iCol-(kSize/2))]*ker[i+(kSize/2)][0]);
			}

		//compute sum, sum = prevSum - prev1stColSum + lastColSum
		sum = prevSum - prev1stColSum + lastColSum;
		prevSum = sum;
		prev1stColSum = nb1stColSum;
	}

	outPix = sum/kSum;

	return outPix;

}


//do the main convolutioon operation here
//whoever uses this method should release the returned pointer
unsigned char *convolveSimple(image &iImg){
   unsigned char *opPixels = NULL;
   int r,c;
   int sum;

   //allocate memory for the out Pixel pointer
   opPixels = new unsigned char[iImg.getRows() * iImg.getCols()];

   //Do convolve operation here
   for(r=3; r< (iImg.getRows()-3); r++)
	for(c=3 ; c<(iImg.getCols()-3); c++)
		//opPixels[r*iImg.getCols()+c] = (unsigned char)kernel2dConv(iImg,c,r);
		//opPixels[r*iImg.getCols()+c] = (unsigned char)kernelSep(iImg,c,r);
		opPixels[r*iImg.getCols()+c] = (unsigned char)kernelSlideWin(iImg,c,r);

   return opPixels;

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

  //Operation on the pixel matrix
  pOutImg = convolveSimple(img);

  
  //Save the new image as out.ppm image
  saveImage(pOutImg,img.getCols(),img.getRows());


  

  //TestCase
  strcpy(fileName,"img/out.ppm");
  if ((fpt=fopen(fileName,"rb")) == NULL)
  {
    std::cout<<"Unable to open out.ppm for reading\n";
    exit(0);
  }

  //load all image data to image object
  image imgOut(fpt);
  fclose(fpt);

  strcpy(fileName,"img/outsep.ppm");
  if ((fpt=fopen(fileName,"rb")) == NULL)
  {
    std::cout<<"Unable to open outsep.ppm for reading\n";
    exit(0);
  }

  //load all image data to image object
  image imgOutSep(fpt);
  fclose(fpt);


  diffImage(imgOutSep.getPixels(),imgOut.getPixels(),img.getCols(),img.getRows());


  delete pOutImg;

  return 0;

}
