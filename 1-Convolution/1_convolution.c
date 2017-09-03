/*
** This program reads bridge.ppm, a 512 x 512 PPM image.
** It smooths it using a standard 7x7 mean filter.
** The program also times the piece of code.
**
** To compile, must link using -lrt  (man clock_gettime() function).
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


//Just reads the image and returns the pixels of the image
unsigned char *readImage(FILE *ifpt,int iC,int iR){

	unsigned char *pImgPixels = NULL;
	char ws;        

 	pImgPixels=(unsigned char *)calloc(iR*iC,sizeof(unsigned char));
	ws = fgetc(ifpt);
	fread(pImgPixels,1,iC*iR,ifpt);

	return pImgPixels;

}

void saveImage(unsigned char *pOutImg, int iC, int iR){

	FILE *fp;

	fp=fopen("out.ppm","w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iC*iR,1,fp);
	fclose(fp);

}

//do the main convolutioon operation here
unsigned char *convolveOperation(unsigned char *ipPixels){

   unsigned char *opPixels = NULL;
   
   //allocate memory for the out Pixel pointer

   //Do convolve operation here
   opPixels = ipPixels;

   return opPixels;

}

int main(){

  FILE    *fpt;
  unsigned char *pInputImg = NULL;
  unsigned char *pOutImg = NULL;
  char fileName[30],header[320];
  int iR,iC,kR,kC,sum,BYTES;
  struct timespec tp1,tp2;

  //check the command line argument for the file name
  strcpy(fileName,"bridge.ppm");

  // get the file pointer and validate it
  if ((fpt=fopen(fileName,"rb")) == NULL)
  {
    printf("Unable to open bridge.ppm for reading\n");
    exit(0);
  }
  //read the specifications of the image
  fscanf(fpt,"%s %d %d %d\n",header,&iC,&iR,&BYTES);

  //call readImage on the file pointer
  pInputImg = readImage(fpt,iC,iR);
  printf("%ld\n",sizeof(pInputImg));
  fclose(fpt);

  //Operation on the pixel matrix
  pOutImg = convolveOperation(pInputImg);

  //Save the new image as out.ppm image
  saveImage(pOutImg,iC,iR);


  return 0;

}
