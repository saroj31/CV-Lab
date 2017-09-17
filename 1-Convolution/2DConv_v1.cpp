/*
** This program reads bridge.ppm, a 512 x 512 PPM image.
** It smooths it using a standard 7x7 mean filter.
** The program also times the piece of code.
** Algorithm complexity: O(r*c*kr*kc)
** where: 
** 		r = image height
** 		c = image width
** 		kr = kernel width
** 		kc = kernel height
**
** Command to Compile:
** with Debug: >>: g++ -lrt 2DConv_v1.cpp image.cpp -o v1_conv 
** without debug:>>: g++ -g -lrt 2DConv_v1.cpp image.cpp -o v1_conv
** Command to run the output exe:>>: ./v1_conv
** Output: tells the time taken by the convolve operation in microsecs
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "image.h"

//Utlity function: to save the image with name out.ppm in ppm format
void saveImage(unsigned char *pOutImg, int iC, int iR){

	FILE *fp;
	fp=fopen("img/out.ppm","w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iC*iR,1,fp);
	fclose(fp);

}

//Utlity function: to save the image with name out.ppm in ppm format
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

//Separated kernel multiplication operation
int kernel2dConv(image &iImg,int iCol,int iRow){

	int r,c,sum=0;
	int outPixel = 0;
	int ker[7][7] = {    //Now using a mean filter but later we can use any filter here
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1},
						{1,1,1,1,1,1,1}
					};
	int kSize = 7;
	unsigned char *pInPixels = iImg.getPixels();

	for( r=-(kSize/2); r<=(kSize/2); r++)	
		for(c=-(kSize/2); c<=(kSize/2); c++)
			sum += (pInPixels[(iRow+r)*iImg.getCols() + (iCol+c)]*ker[c+(kSize/2)][r+(kSize/2)]);

	outPixel = sum/(kSize*kSize);

	//for debug : uncomment below
	//std::cout<<outPixel<<"   "<<iCol<<"  "<<iRow<<std::endl;
	return outPixel;
}


//do the main convolution operation here
//whoever uses this method should delete the returned pointer to free the memory
unsigned char *convolveSimple(image &iImg){
   unsigned char *opPixels = NULL;
   int r,c;
   int sum;

   //allocate memory for the out Pixel pointer
   opPixels = new unsigned char[iImg.getRows() * iImg.getCols()];

   //Do convolve operation here
  for(r=3; r< (iImg.getRows()-3); r++)
		for(c=3 ; c<(iImg.getCols()-3); c++)
			opPixels[r*iImg.getCols()+c] = (unsigned char)kernel2dConv(iImg,c,r);


   return opPixels;

}



int main(){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  char fileName[30];
  struct timespec tp1,tp2;

  //check the command line argument for the file name
  strcpy(fileName,"img/bridge.ppm");
  //strcpy(fileName,"mat.txt");

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
  pOutImg = convolveSimple(img);

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
