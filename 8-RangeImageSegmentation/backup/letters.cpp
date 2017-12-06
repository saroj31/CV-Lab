/*
*/

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include <cmath>
#include "image.h"
#include "kernel.h"

const int MAX_QUEUE 10000;	/* max perimeter size (pixels) of border wavefront */
const double PI = 3.14159265;

struct point
{
	int r; //y in cartesian
	int c; //x in cartesian
};

struct point3d
{
	int c; //x in cartesian
	int r; //y in cartesian
	int d; //z in cartesian, it means depth
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


void thresholdImage(image &inImg,int iTmin=50,int iTmax=150){

	//unsigned char *pOutCropPixels = new unsigned char[9*15];
	int ix0,ixn,iy0,iyn;
	unsigned char **ppImgPix = inImg.getppPixels();
	int imgH = inImg.getRows();
	int imgW = inImg.getCols();

	int cnt = 0;
	for(int r=0;r<imgH;r++){
		for(int c=0;c<=imgW;c++){

			if((ppImgPix[r][c] < iTmin) || (ppImgPix[r][c] > iTmax))
				ppImgPix[r][c] = 0;	
		}
	}


}

struct point3d get3dCoords(image &inImg_t,struct point ipt){

	int COLS = inImg_t.getCols();
	int ROWS = inImg_t.getRows();
	int r,c;
	double	cp[7];
	unsigned char **RangeImage=inImg_t.getppPixels();
	double	ScanDirectionFlag=1,SlantCorrection=0; /* Odetics image -- scan direction downward */
	double	xangle,yangle,dist;
	struct point3d oPt3d = {0,0,0};

	cp[0]=1220.7;		/* horizontal mirror angular velocity in rpm */
	cp[1]=32.0;		/* scan time per single pixel in microseconds */
	cp[2]=(COLS/2)-0.5;		/* middle value of columns */
	cp[3]=1220.7/192.0;	/* vertical mirror angular velocity in rpm */
	cp[4]=6.14;		/* scan time (with retrace) per line in milliseconds */
	cp[5]=(ROWS/2)-0.5;		/* middle value of rows */
	cp[6]=10.0;		/* standoff distance in range units (3.66cm per r.u.) */

	cp[0]=cp[0]*3.1415927/30.0;	/* convert rpm to rad/sec */
	cp[3]=cp[3]*3.1415927/30.0;	/* convert rpm to rad/sec */
	cp[0]=2.0*cp[0];		/* beam ang. vel. is twice mirror ang. vel. */
	cp[3]=2.0*cp[3];		/* beam ang. vel. is twice mirror ang. vel. */
	cp[1]/=1000000.0;		/* units are microseconds : 10^-6 */
	cp[4]/=1000.0;			/* units are milliseconds : 10^-3 */

	c = ipt.c;
	r = ipt.r;
	//RangeImage = inImg_t.getPixels();


	SlantCorrection=cp[3]*cp[1]*((double)c-cp[2]);
    xangle=cp[0]*cp[1]*((double)c-cp[2]);
    yangle=(cp[3]*cp[4]*(cp[5]-(double)r))+	/* Standard Transform Part */
	SlantCorrection*ScanDirectionFlag;	/*  + slant correction */
    dist=(double)RangeImage[r][c]+cp[6];
    /*P[2][r*COLS+c]*/oPt3d.d =sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle))+(tan(yangle)*tan(yangle))));
    /*P[0][r*COLS+c]*/oPt3d.c =tan(xangle)*oPt3d.d;
    /*P[1][r*COLS+c]*/oPt3d.r =tan(yangle)*oPt3d.d;

	return oPt3d;
}

double dotProduct(struct point3d iVecA,struct point3d iVecB){

	int a1,a2,a3=0;
	int b1,b2,b3=0;
	double oDotVal = 0.0;
	

	a1 = iVecA.c;
	a2 = iVecA.r;
	a3 = iVecA.d;

	b1 = iVecB.c;
	b2 = iVecB.r;
	b3 = iVecB.d;

	dotVal = a1*b1 + a2*b2 + a3*b3;

	return oDotVal;


}


struct point3d crossProduct(struct point3d iVecA,struct point3d iVecB){

	int a1,a2,a3=0;
	int b1,b2,b3=0;

	struct point3d oVecC = {0,0,0};

	a1 = iVecA.c;
	a2 = iVecA.r;
	a3 = iVecA.d;

	b1 = iVecB.c;
	b2 = iVecB.r;
	b3 = iVecB.d;

	oVecC.c = a2*b3 - a3*b2;
	oVecC.r = a3*b1 - a1*b3;
	oVecC.d = a1*b2 - a2*b1;

	return oVecC;


}

double calcMagnitude(struct point3d iVec){

	double magNorm,tot=0;
	tot = (iVec.r*iVec.r)+(iVec.c*iVec.c)+(iVec.d*iVec.d);
	magNorm = sqrt(tot);

	return magNorm;
}

double calcTheta(struct point3d iVec){

	double magn,oTheta=0;

	magn = calcMagnitude(iVec);
	if(magn!=0)
	  	oTheta = acos((double)iVec.d/magn);
	else
	  	oTheta = 0;

	oTheta = oTheta* 180.0 / PI;

	return oTheta;
}


// void RegionGrow(unsigned char *image,	/* image data */
// 		unsigned char *labels,	/* segmentation labels */
// 		int ROWS,int COLS,	/* size of image */
// 		int r,int c,		/* pixel to paint from */
// 		int paint_over_label,	/* image label to paint over */
// 		int new_label,		/* image label for painting */
// 		int *indices,		/* output:  indices of pixels painted */
// 		int *count)		/* output:  count of pixels painted */
// {
// int	r2,c2;
// int	queue[MAX_QUEUE],qh,qt;
// int	average,total;	/* average and total intensity in growing region */

// *count=0;
// if (labels[r*COLS+c] != paint_over_label)
//   return;
// labels[r*COLS+c]=new_label;
// average=total=(int)image[r*COLS+c];
// if (indices != NULL)
//   indices[0]=r*COLS+c;
// queue[0]=r*COLS+c;
// qh=1;	/* queue head */
// qt=0;	/* queue tail */
// (*count)=1;
// while (qt != qh)
//   {
//   if ((*count)%50 == 0)	/* recalculate average after each 50 pixels join */
//     {
//     average=total/(*count);
//     // printf("new avg=%d\n",average);
//     }
//   for (r2=-1; r2<=1; r2++)
//     for (c2=-1; c2<=1; c2++)
//       {
//       if (r2 == 0  &&  c2 == 0)
//         continue;
//       if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
// 	  (queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
//         continue;
//       if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
//         continue;
// 		/* test criteria to join region */
//       if (abs((int)(image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2])
// 		-average) > 10)
//         continue;
//       labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;
//       if (indices != NULL)
//         indices[*count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
//       total+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
//       (*count)++;
//       queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
//       qh=(qh+1)%MAX_QUEUE;
//       if (qh == qt)
//         {
//         printf("Max queue size exceeded\n");
//         exit(0);
//         }
//       }
//   qt=(qt+1)%MAX_QUEUE;
//   }
// }

int main(int argc, char *argv[]){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  struct point3d img3dPts[128*128] = {0}; //Caution: hard coding here not Good
  char fileName[30];
  int thres;
  int imgCSize,imgRSize;

  //check the command line argument for the file name
  strcpy(fileName,"img/chair-range.ppm");
  image origImg(fileName);

  //make a copy and work on it
  strcpy(fileName,"img/chair-range.ppm");
  image tmpImg(fileName);

  //make a copy and work on it
  strcpy(fileName,"img/chair-range.ppm");
  struct point3d *pImgNormals = new struct point3d[tmpImg.getRows() * tmpImg.getCols()]

  imgCSize = tmpImg.getCols();
  imgRSize = tmpImg.getRows();
  thresholdImage(tmpImg,59,140);


  for(int j = 0;j<tmpImg.getCols()-2;j++){
  	for(int i = 0;i<tmpImg.getRows()-2;i++){

  	  struct point ptX,ptA,ptB = {0,0};
	  struct point3d pt3dX,pt3dA,pt3dB = {0,0,0};
	  struct point3d vecAX,vecBX,vecNorm={0,0,0};
	  double magNorm,thetaNorm,tot=0;	//theta is wrt to yz plane
	  
	  ptX.r = j;
	  ptX.c = i;

	  //accroding to lectture notes naming the points
	  ptB.r = j+2;
	  ptB.c = i;
	  ptB.r = j;
	  ptB.c = i+2;

	  //need to do only for the thresholded pixels
	  if(tmpImg.getppPixels()[j][i] != 0){


	  	pt3dX = get3dCoords(tmpImg,ptX);	
	  	pt3dA = get3dCoords(tmpImg,ptA);
	  	pt3dB = get3dCoords(tmpImg,ptB);

	  	vecAX.r = pt3dA.r - pt3dX.r;
	  	vecAX.c = pt3dA.c - pt3dX.c;
	  	vecAX.d = pt3dA.d - pt3dX.d;

	  	vecBX.r = pt3dB.r - pt3dX.r;
	  	vecBX.c = pt3dB.c - pt3dX.c;
	  	vecBX.d = pt3dB.d - pt3dX.d;

	  	vecNorm = crossProduct(vecBX,vecAX);


	  }

	  // tot = (vecNorm.r*vecNorm.r)+(vecNorm.c*vecNorm.c)+(vecNorm.d*vecNorm.d);
	  // magNorm = sqrt(tot);
	  magNorm = calcMagnitude(vecNorm);
	  thetaNorm = calcTheta(vecNorm);
  
	  //printf("%4d",(int)thetaNorm);
	  //degImg.getppPixels()[j][i] = thetaNorm;

	  struct point3d this3dPt = pImgNormals[i*tmpImg.getCols()+j];

	  this3dPt.r = vecNorm.r;
	  this3dPt.c = vecNorm.c;
	  this3dPt.d = vecNorm.d;
	  
	  
  	}
  	//std::cout<<std::endl;
  }
  	  
  //we will have a degree image hereso need to do regeion grow on that
  strcpy(fileName,"img/output.ppm");
  saveImage(tmpImg.getPixels(),tmpImg.getCols(),tmpImg.getRows(),fileName);

  strcpy(fileName,"img/outdeg.ppm");
  saveImage(degImg.getPixels(),degImg.getCols(),degImg.getRows(),fileName);

  delete pImgNormals;
  return 0;

}
