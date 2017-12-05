/*
  Version 3: A 2d Convolution implements a 2d convolution in an image using separable and sliding Windowo technique.

  We are using a separable and sliding window algorithm here

  It is a C++ code so use the below code to compile it:-

Command for compiling with debug:
  g++ -g -lrt 2DConSepSlideWin.cpp image.cpp -o v3_conv

Command for compiling without debug:
  g++ -lrt 2DConSepSlideWin.cpp image.cpp -o v3_conv

Command to run output: 
	./v3_conv

Output: 
	Time taken to convolve the image in microsecs
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "image.h"

//utility Function: to save the image if the pixels value and size ofo row and column is given
void saveImage(unsigned char *pOutImg, int iC, int iR,bool isTemp = false){

	FILE *fp;
	char fileName[30];
	
	if(isTemp)
		strcpy(fileName,"img/temp.ppm");
	else
		strcpy(fileName,"img/outv3.ppm");

	fp=fopen(fileName,"w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iC*iR,1,fp);
	fclose(fp);

}

//utility Function: to find difference in two images
bool diffImage(unsigned char *ip1,unsigned char *ip2,int iC, int iR){

	int diff = -1;
	bool isOK = true;
	for(int i=0; i<iR;i++){
		for(int j=0; j<iC; j++){
			diff = ip1[i*iC + j] - ip2[i*iC + j];
			if(diff)	{
				printf("%d   %d %d\n",diff,j,i);
				isOK = false;
			}
		}
		
	}

	return isOK;
}

//Kernel sliding window algorithm
//a 1x7 filter is slided horizontally in teh image
int kernelSepSlideWinhor(image &iImg,int iCol,int iRow){

	int outPixSum=0;
	static int prev1stColSum = 0,prevSum = 0;
	int sum = 0;
	unsigned char *pInPixels = iImg.getPixels();

	//kernel separable details
	int ker[7] = {1,1,1,1,1,1,1};
	int kSize = 7;

	//first pixel of the row, iCol = 3 
	if( (kSize/2) == iCol){
		//Compute the full kernel and save the prev1stColSum and the prevSum
		for( int c=-(kSize/2); c<=(kSize/2); c++)
		{
			sum += (pInPixels[(iRow)*iImg.getCols() + (iCol+c)]*ker[c+(kSize/2)]);  //we increment horizontally

			if((c+(kSize/2) == 0))
				prev1stColSum = sum; //which is the first convolved value
		}
		//save the prevSum
		prevSum = sum;

	}//not a first pixel of the row, iCol!=3
	else{
		int lastColSum=0,nb1stColSum=0;
		//Compute the last col sum, lastColSum
		lastColSum = pInPixels[iRow*(iImg.getCols()) + (iCol+(kSize/2))];
		nb1stColSum = pInPixels[iRow*(iImg.getCols()) + (iCol-(kSize/2))];

		//compute sum, sum = prevSum - prev1stColSum + lastColSum & save values for next iteration
		sum = prevSum - prev1stColSum + lastColSum;
		prevSum = sum;
		prev1stColSum = nb1stColSum;
	}

	outPixSum = sum;

	return outPixSum;

}

//kernel separable filter applied vertically
//a 7x1 filter is slided vertically in the image
int kernelSepSlideWinVer(unsigned int *&pInPixels,int iCol,int iRow,int imgWidth,int imgHeight){
	int outPix=0;
	static int prev1stRowSum,prevSum;
	int sum = 0;
	//unsigned char *pInPixels = iImg.getPixels();

	//kernel separable details
	int ker[7] = {1,1,1,1,1,1,1};
	int kSize = 7;

	//first pixel of the row, iCol = 3 
	if( (kSize/2) == iRow){
		//Compute the full kernel and save the prev1stRowSum and the prevSum
		for( int r=-(kSize/2); r<=(kSize/2); r++)
		{
			sum += (pInPixels[(iRow+r)*imgWidth + (iCol)]*ker[r+(kSize/2)]); 

			if((r+(kSize/2)) == 0)
				prev1stRowSum = sum; //which is the first convolved value
		}
		//save the prevSum
		prevSum = sum;

	}//not a first pixel of the row, iCol!=3
	else{
		int lastRowSum=0,nb1stRowSum=0;
		//Compute the last col sum, lastRowSum
		lastRowSum = pInPixels[((iRow+(kSize/2))*(imgWidth)) + (iCol)];
		nb1stRowSum = pInPixels[((iRow-(kSize/2))*(imgWidth)) + (iCol)];

		//compute sum, sum = prevSum - prev1stRowSum + lastRowSum
		sum = prevSum - prev1stRowSum + lastRowSum;
		prevSum = sum;
		prev1stRowSum = nb1stRowSum;
	}

	outPix = sum;

	//debug
	//std::cout<<outPix<<"  "<<iCol<<" "<<iRow<<std::endl;

	return outPix;

}

//do the main convolution operation here we will clock this function
//whoever uses this method should release the returned pointer
unsigned char *convolveImage(image &iImg){
  
  unsigned char *opPixels = NULL;
  unsigned int *tmpPixels = NULL;
  int kSize = 7;
  int r,c;
  int sum;

  //allocate memory for the out Pixel pointer
  opPixels = new unsigned char[iImg.getRows() * iImg.getCols()];
  tmpPixels = new unsigned int[iImg.getRows() * iImg.getCols()];


  //Do convolve operation horizontally
  for(r=0; r<(iImg.getRows()); r++)  //rows will be the same here
		for(c=3 ; c<(iImg.getCols()-3); c++)
			tmpPixels[r*iImg.getCols()+c] = kernelSepSlideWinhor(iImg,c,r);

  //Do convolve operation vertically
  for(c=3; c<(iImg.getRows()-3); c++)  //rows will be the same here
   for(r=3 ; r<(iImg.getCols()-3); r++)
		opPixels[r*iImg.getCols()+c] = (unsigned char)(kernelSepSlideWinVer(tmpPixels,c,r,iImg.getCols(),iImg.getRows())/(kSize*kSize));


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