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
** without Debug: >>: g++ -lrt 2DConv_v1.cpp image.cpp -o v1_conv 
** with debug:>>: g++ -g -lrt 2DConv_v1.cpp image.cpp -o v1_conv
** Command to run the output exe:>>: ./v1_conv
** Output: tells the time taken by the convolve operation in microsecs
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "image.h"
#include "kernel.h"

//Utlity function: to save the image with name out.ppm in ppm format
void saveImage(unsigned char *pOutImg, int iC, int iR){

	FILE *fp;
	fp=fopen("img/out.ppm","w");

	fprintf(fp,"P5 %d %d 255\n",iC,iR);
	fwrite(pOutImg,iR*iC,1,fp);
	fclose(fp);

}

void cleanPixels(unsigned int **ppIn,int iC,int iR){

	for(int r=0;r<iR;r++){
		delete ppIn[r];
	}
	delete ppIn;
}

unsigned char *normalizePixels(int *iOldPix,int inC,int inR){

	//find oldmin an old max
	int oldMin = iOldPix[0];
	int oldMax = iOldPix[0];

	for(int i = 0;i<(inC*inR);i++){
		if(iOldPix[i] > oldMax)
			oldMax = iOldPix[i];
		else if(iOldPix[i] < oldMin)
			oldMin = iOldPix[i];
	}

	//use the oldMin and oldMax to get the normalized value
	unsigned char *opPixels = new unsigned char[inR*inC];
	for(int r=0;r<inR;r++){
		for(int c=0;c<inC;c++){
			int ithPix = r*inC+c;
			float normFactor = (float)(iOldPix[ithPix]-oldMin)/(float)(oldMax-oldMin);
			opPixels[ithPix] = (int)((float)normFactor*255);
		}
	}

	return opPixels;
}


//Separated kernel multiplication operation
int kernel2dConv(image &iImg,int iCol,int iRow,int **ker,kernel &k){

	int r,c,sum=0;
	int outPixel = 0;
	int kr = k.getRows(),kc=k.getCols();
	unsigned char **ppInPixels = iImg.getppPixels();

	for( r=-(kr/2); r<=(kr/2); r++)	
		for(c=-(kc/2); c<=(kc/2); c++)
			outPixel += ppInPixels[iRow+r][iCol+c]*ker[r+(kr/2)][c+(kc/2)];

	//for debug : uncomment below
	//std::cout<<outPixel<<"   "<<iCol<<"  "<<iRow<<std::endl;
	return outPixel;
}


//do the main convolution operation here
//whoever uses this method should delete the returned pointer to free the memory
unsigned char *convolveSimple(image &iImg,kernel &k,int **kMSF_t){
   unsigned char *opPixels = NULL;
   int *tmpPixels = NULL;
   int r,c;
   int sum;
   int ir=iImg.getRows(),ic= iImg.getCols(),kr = k.getRows(),kc = k.getCols();

   tmpPixels = new int[iImg.getRows() * iImg.getCols()];


   //Do convolve operation here
  for(r=(kr/2); r< (ir-(kr/2)); r++){
		for(c=(kc/2) ; c<(ic-(kc/2)); c++){
			tmpPixels[r*ic+c] = kernel2dConv(iImg,c,r,kMSF_t,k);
		}
  }

	opPixels = normalizePixels(tmpPixels,ic,ir);

	//delete tmpPixels;
  return opPixels;

}



int **computeMSFMeanKernel(kernel &ker){
	
	unsigned int kerMean = 0;
	unsigned char **ppkPixels = ker.getPixels2dArray();
	int **ppOut = new int*[ker.getRows()];
	unsigned int sum = 0;

	//calculate the kernel mean
	for(int r=0; r<ker.getRows();r++){
		for(int c=0; c<ker.getCols();c++){
			sum += ppkPixels[r][c];
		}
	}

	kerMean = sum/(ker.getRows()*ker.getCols());
	//std::cout<<"Kernel Sum = "<<sum<<std::endl;

	//subtract each cell of the kernel with the mean
	for(int r=0; r<ker.getRows();r++){
		ppOut[r] = new int[ker.getCols()];
		for(int c=0; c<ker.getCols();c++){
			ppOut[r][c] = ppkPixels[r][c]-kerMean;
			//std::cout<<ppOut[r][c]<<"   ";
		}
		//std::cout<<std::endl;
	}

	return ppOut;
}

unsigned char *getMSFImage(kernel &ker,image &iImg,int iT){

	//get the MSFtemplate mean kernel matrix
	int **ppMeanKernel = computeMSFMeanKernel(ker);

	//convolve the MSFMeankernel with the image
	unsigned char *pMSFImage = convolveSimple(iImg,ker,ppMeanKernel);

	//Debug: Without threshold MSFImage
	saveImage(pMSFImage,iImg.getCols(),iImg.getRows());
		
	//loop through the image and make it binary
	for(int i=0;i<(iImg.getCols()*iImg.getRows());i++){

		//if the returned pixel is greater than the threshold
		if(pMSFImage[i]>=iT)
			pMSFImage[i]=255; //then set 255 on output image
		else
			pMSFImage[i]=0; //else set to 0
	}

	return pMSFImage;
}

void TestMSFImage(unsigned char *pMSFImage,char infileName[30],image &img,kernel &ker){

	int imgR=img.getRows(),imgC=img.getCols();
	int nbTP=0,nbFP=0;
	int nbTN=0,nbFN=0;
	char letter;
	int gtR,gtC;

	//read GroudTruth file
	FILE *fpt = fopen(infileName,"rb");

	while(1){
		//Get GroudTruth value from file line by line
		int rc = fscanf(fpt,"%c %d %d\n",&letter,&gtC,&gtR);
		if(EOF == rc) break;

		bool isDetected = false;
		for(int r=gtR-ker.getRows();r<gtR+ker.getRows();r++){
			for(int c=gtC-ker.getCols();c<gtC+ker.getCols();c++){

				if(pMSFImage[(r*imgC)+c] == 255){
					isDetected=true;
					r=gtR+ker.getRows(); //this will stop the loop
					c=gtC+ker.getCols();
				}
			}
		}

		if(isDetected && ('e' == letter))	nbTP+=1;   //True Positive
		if(isDetected && ('e' != letter))		nbFP+=1; //Falses Positive
		if(!isDetected && ('e' != letter))  nbTN+=1;	//True Negative
		if(!isDetected && ('e' == letter))  nbFN+=1;  //False Negative

	}

	fclose(fpt);
	//Make observation of TP and FP
	float TPR,FPR;
	TPR = (float)nbTP/float(nbTP+nbFN);
	FPR = (float)nbFP/float(nbFP+nbTN);
	std::cout<<nbTP<<","<<nbFP<<","<<nbTN<<","<<nbFN<<","<<TPR<<","<<FPR<<std::endl;
	return;
}

int main(int argc, char *argv[]){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  char fileName[30];
  struct timespec tp1,tp2;
  int thres;

  //check the command line argument for the file name
  strcpy(fileName,"img/parenthood_e_template.ppm");
  kernel ker(fileName);

  strcpy(fileName,"img/parenthood.ppm");
  image img(fileName);

  thres = 210; //default value can be set to anything
  if(argc==2){ //is command line argument given
  	thres = atoi(argv[1]);
  }
  unsigned char *pMSFImage = NULL;
  for(thres = 0;thres<=256;thres++){
   	std::cout<<thres<<",";
   	pMSFImage=getMSFImage(ker,img,thres);

  	strcpy(fileName,"parenthood_gt.txt");
  	TestMSFImage(pMSFImage,fileName,img,ker);
	}
  saveImage(pMSFImage,img.getCols(),img.getRows());

  return 0;

}
