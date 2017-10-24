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
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include "image.h"
#include "kernel.h"

struct  point
{
	int r;
	int c;
};

struct  gTruth
{
	unsigned char letter;
	struct point pt;
};


//Utlity function: to save the image with name out.ppm in ppm format
void saveImage(unsigned char *pOutImg, int iC, int iR,char fileName[30]){

	FILE *fp;

	fp=fopen(fileName,"w");

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


std::vector<struct gTruth> getGroundTruth(FILE *fp_gt){

	std::vector<struct gTruth> opGT;
	struct gTruth tmpGT;

	while(1){
			int rc = fscanf(fp_gt,"%c %d %d\n",&tmpGT.letter,&tmpGT.pt.c,&tmpGT.pt.r);
			if(EOF == rc) break;

			opGT.push_back(tmpGT);

		}

		fclose(fp_gt);

	return opGT;
}

//delete the returned pixel
//ix is column
//iy is row
void cropAndThreshold_9x15(image &inImg,int ix,int iy){

	//unsigned char *pOutCropPixels = new unsigned char[9*15];
	int ix0,ixn,iy0,iyn;
	ix0 = ix-(9/2)-1;
	ixn = ix+(9/2);
	iy0 = iy-(15/2)+1;
	iyn = iy+(15/2)+1;
	int T = 128;
	unsigned char **ppImgPix = inImg.getppPixels();

	int cnt = 0;
	for(int r=iy0;r<=iyn;r++){
		for(int c=ix0;c<=ixn;c++){

			//edgecases
			if((c==ix0) || (c==ixn) || (r==iy0) || (r==iyn))
				ppImgPix[r][c] = 255;

			if(ppImgPix[r][c] > T)
				ppImgPix[r][c] = 255;
			else
				ppImgPix[r][c] = 0;
		}
	}


}


//delete nbr after being used in the client
unsigned char *getNeighBors(image &img,int c,int r){

	unsigned char *nbr = new unsigned char[9];
	int cnt = 0;
	int i =0;
	unsigned char **ppPixels = img.getppPixels();

	for(i = c-1;i<=(c+1);i++)
		nbr[cnt++] = ppPixels[r-1][i];

	nbr[cnt++] = ppPixels[r][c+1];

	for(i = c+1;i>=(c-1);i--)
		nbr[cnt++] = ppPixels[r+1][i];

	nbr[cnt++] = ppPixels[r][c-1];
	nbr[cnt++] = ppPixels[r-1][c-1];

	return nbr;


}

void thinning(image &img,int gtc,int gtr){

	//thinning has to be done for single pixel wide components

	int ic0,icn,ir0,irn;
	ic0 = gtc-(9/2);
	icn = gtc+(9/2);
	ir0 = gtr-(15/2);
	irn = gtr+(15/2);
	unsigned char **ppImgPix = img.getppPixels();
	std::vector<struct point> vErasePoints;
	int c =0;


	//loop through the thresholded image
	do{

		//erase the marked points
		for(int ithP=0;ithP < vErasePoints.size(); ithP++){
			//erase here by making them 255
			ppImgPix[vErasePoints[ithP].r][vErasePoints[ithP].c] = 255;
			//std::cout<<"pixels erased: "<<c<<" at r= "<<vErasePoints[ithP].r<<" ,c= "<<vErasePoints[ithP].c<<std::endl;
		}
		if(vErasePoints.size()) vErasePoints.clear();

		for(int r=ir0;r<=irn;r++){
			for(int c=ic0;c<=icn;c++){
				//Pass through the image looking at each pixel with value 0 i.e. edge pixels
				if( 0 == ppImgPix[r][c]){
					//check for erasure
					unsigned char *nbr = getNeighBors(img,c,r);
					int nbE2NE = 0;
					int N=0,E=0,W=0,S=0;
					int nbE = 0;

					//check for all edge to non-edge transition
					for(int i =0;i<8;i++){
						//printf("%d ",nbr[i]);

						if( (nbr[i]==0) && (nbr[i+1]==255))
							nbE2NE+=1;

						//get neighbors
						if( (i == 1) && (nbr[i]==255)) N=1;
						if( (i == 3) && (nbr[i]==255)) E=1;
						if( (i == 5) && (nbr[i]==255)) S=1;
						if( (i == 7) && (nbr[i]==255)) W=1;

						//get number of edge neighbors
						if( 0 == nbr[i])
							nbE+=1;

					}

					//the thinning condition to mark the pixel location
					if( (1 == nbE2NE) && ((3<=nbE)&&(7>=nbE)) && ( N | E | (W&S) )) {
							struct point p;
							p.c = c;
							p.r = r;
							vErasePoints.push_back(p);
					}

					delete nbr;
				}

			}
		}
	}while(vErasePoints.size());

	return;
}


bool is_e_detected(image &img,int gtC,int gtR){

	bool isE = false;
	int nb_E2NE=0;
	int nb_ep=0,nb_bp=0;
	int ic0,icn,ir0,irn;
	ic0 = gtC-(9/2);
	icn = gtC+(9/2);
	ir0 = gtR-(15/2);
	irn = gtR+(15/2);
	unsigned char *nbr = NULL;


	for(int r=ir0;r<=irn;r++){
		for(int c=ic0;c<=icn;c++){

			//printf("%4d ",(img.getppPixels())[r][c]);
			if( 255 == (img.getppPixels())[r][c])	{
				printf("%4d ",(img.getppPixels())[r][c]);
				continue;
			}

			nbr = getNeighBors(img,c,r);
			for(int i=0;i<8;i++){
				//count edge to non-edge
				if( (nbr[i]==0) && (nbr[i+1]==255))
					nb_E2NE+=1;
			}

			if( 1 == nb_E2NE){
				nb_ep+=1;
				//(img.getppPixels())[r][c] = 1;
				printf("   1 ");
				nb_E2NE = 0;
				continue;
			}

			if( 2 < nb_E2NE){
				nb_bp+=1;
				int y = 2;
				printf("   2 ");	
				nb_E2NE = 0;
				continue;
			}

			nb_E2NE=0;
			printf("%4d ",(img.getppPixels())[r][c]);

			delete nbr;
		}
		printf("nb_ep=%d nb_bp=%d",nb_ep,nb_bp);
		printf(" gtC = %d gtR = %d ",gtC,gtR );
		printf("\n");
	}
	printf("\n");

	//printf("nb_ep=%d nb_bp=%d gtC=%d gtR=%d ",nb_ep,nb_bp,gtC,gtR);
	if((nb_bp ==1) && (nb_ep==1))
		isE = true;

	/*if(isE)
		printf(" Detected \n");
	else
		printf(" \n");
*/
	return isE;
}

int main(int argc, char *argv[]){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  char fileName[30];
  struct timespec tp1,tp2;
  int thres;

  //check the command line argument for the file name
  strcpy(fileName,"img/parenthood_e_template.ppm");
  image ker(fileName);

  strcpy(fileName,"img/parenthood.ppm");
  image origImg(fileName);

  strcpy(fileName,"parenthood_gt.txt");
  FILE *fp_gt = fopen(fileName,"rb");
  std::vector<struct gTruth> vGT = getGroundTruth(fp_gt);

  strcpy(fileName,"img/msf_e.ppm");
  image msfImg(fileName);
  unsigned char **ppMsfImg = msfImg.getppPixels();

  //Loop through the following steps for a range of T: so Started from 100 to 255
  int TP=0,FP=0,TN=0,FN=0;
  int TPMSF=0;
  unsigned char letter='e';
  int gtC=347,gtR=41;
  int iT = 250;

  image img;
  img = origImg;
  printf(" T  TP  FP \n");
  for(; iT<251; iT++){

  	//Loop through the ground truth letter locations
  	for(int ithGT=0;ithGT<vGT.size();ithGT++){


  		//strcpy(fileName,"img/parenthood.ppm");
  		//image img(fileName);

			letter = vGT[ithGT].letter;
			gtC = vGT[ithGT].pt.c;
			gtR = vGT[ithGT].pt.r;

			/*Check a 9 x 15 pixel area centered at the ground truth location.  If
			any pixel in the msf image is greater than the threshold, consider
			the letter “detected”.  If none of the pixels in the 9 x 15 area are
			greater than the threshold, consider the letter “not detected”.*/

			bool isDetected = false;
			for(int r=gtR-(ker.getRows()/2);r<gtR+(ker.getRows()/2);r++){
				for(int c=gtC-(ker.getCols()/2);c<gtC+(ker.getCols()/2);c++){

					if(ppMsfImg[r][c] > iT){
						isDetected=true;
						r=gtR+ker.getRows(); //this will stop the loop
						c=gtC+ker.getCols();

					}
				}
			}


			//If the letter is “not detected” continue to the next letter.
			if( false == isDetected)		continue;
			TPMSF+=1;

			//at this point letter is detected
			//Create a 9 x 15 pixel image that is a copy of the area centered at the ground truth location (center of letter) from the original image
			cropAndThreshold_9x15(img,gtC,gtR);
			strcpy(fileName,"img/crop.ppm");
			saveImage(img.getPixels(),img.getCols(),img.getRows(),fileName);

			//thinning on the ground truth coordinates
			thinning(img,gtC,gtR);
			strcpy(fileName,"img/thin.ppm");
			saveImage(img.getPixels(),img.getCols(),img.getRows(),fileName);


			//test thiinned image for edge points and branch points
			bool isE = is_e_detected(img,gtC,gtR);
			if((true == isE)  && ('e'==letter))
				TP+=1;

			if((true == isE)  && ('e'!=letter)){
				//printf(" FP at gtc = %d gtR = %d \n",gtC,gtR);
				FP+=1;
			}

			if((false == isE)  && ('e'==letter)){
				//printf(" FN at gtc = %d gtR = %d \n",gtC,gtR);
				//strcpy(fileName,"img/thin1.ppm");
				//saveImage(img.getPixels(),img.getCols(),img.getRows(),fileName);
				//getchar();
				FN+=1;
			}


		}//while loop ends here
		//printf("%d %d %d FN=%d totP=%d TPMSF=%d\n",iT,TP,FP,FN,TP+FP,TPMSF);
		printf("%2d %3d %3d\n",iT,TP,FP);
		TP = 0;
		FP = 0;
		TPMSF = 0;
		FN = 0;
	}//the Threshold Loop

	//strcpy(fileName,"img/thin.ppm");
	//saveImage(img.getPixels(),img.getCols(),img.getRows(),fileName);

  return 0;

}
