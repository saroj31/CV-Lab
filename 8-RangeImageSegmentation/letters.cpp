/*
*/

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "image.h"
#include "kernel.h"

#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */
#define PI  3.14159265

int thres =45;
int inSize =3; 

struct point
{
	double r; //y in cartesian
	double c; //x in cartesian
};

struct point3d
{
	double r; //y in cartesian
	double c; //x in cartesian
	double d; //z in cartesian, it means depth
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
	unsigned char *pImgPix = inImg.getPixels();
	int imgH = inImg.getRows();
	int imgW = inImg.getCols();

	int cnt = 0;
	for(int r=0;r<imgH;r++){
		for(int c=0;c<=imgW;c++){

			if((pImgPix[r*imgW + c] < iTmin) || (pImgPix[r*imgW + c] > iTmax))
				pImgPix[r*imgW + c] = 0;	
		}
	}


}

struct point3d get3dCoords(image &inImg_t,struct point ipt){

	int COLS = inImg_t.getCols();
	int ROWS = inImg_t.getRows();
	int r,c;
	double	cp[7];
	unsigned char *RangeImage=inImg_t.getPixels();
	double	ScanDirectionFlag=1,SlantCorrection; /* Odetics image -- scan direction downward */
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
    dist=(double)RangeImage[r*COLS+c]+cp[6];
    oPt3d.d =sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle))+(tan(yangle)*tan(yangle))));
    oPt3d.c =tan(xangle)*oPt3d.d;
    oPt3d.r =tan(yangle)*oPt3d.d;

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

	oDotVal = a1*b1 + a2*b2 + a3*b3;

	return oDotVal;


}


struct point3d crossProduct(struct point3d iVecA,struct point3d iVecB){

	double a1,a2,a3=0;
	double b1,b2,b3=0;
	struct point3d oVecC = {0,0,0};

	a1 = iVecA.c;
	a2 = iVecA.r;
	a3 = iVecA.d;

	b1 = iVecB.c;
	b2 = iVecB.r;
	b3 = iVecB.d;

	oVecC.c = (a2*b3) - (a3*b2);
	oVecC.r = (a3*b1) - (a1*b3);
	oVecC.d = (a1*b2) - (a2*b1);

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

double calcAngleBetween(struct point3d iVecA,struct point3d iVecB){

	double oArc = 0.0;
	double oTot = calcMagnitude(iVecA) * calcMagnitude(iVecB);

	oArc = (dotProduct(iVecA,iVecB))/oTot;
	oArc = acos(oArc);
	oArc = oArc*180 / PI;

	return oArc;

}

void RegionGrow(struct point3d *&image,	/* image data */
		unsigned char *&labels,	/* segmentation labels */
		int ROWS,int COLS,	/* size of image */
		int r,int c,		/* pixel to paint from */
		int paint_over_label,	/* image label to paint over */
		int new_label,		/* image label for painting */
		int *&indices,		/* output:  indices of pixels painted */
		int &count)		/* output:  count of pixels painted */
{
int	r2,c2;
int	queue[MAX_QUEUE],qh,qt;
struct point3d	average,total;	/* average and total intensity in growing region */
double pred = 0.0;

count=0;
if (labels[r*COLS+c] != paint_over_label)
  return;
labels[r*COLS+c]=new_label;
average=total=image[r*COLS+c];
if (indices != NULL)
  indices[0]=r*COLS+c;
queue[0]=r*COLS+c;
qh=1;	/* queue head */
qt=0;	/* queue tail */
(count)=1;
while (qt != qh)
  {
  if ((count)%25 == 0)	/* recalculate average after each 50 pixels join */
    {
    average.r=total.r/(count);
    average.c=total.c/(count);
    average.d=total.d/(count);
    // printf("new avg=%d\n",average);
    }
  for (r2=-1; r2<=1; r2++)
    for (c2=-1; c2<=1; c2++)
      {
      if (r2 == 0  &&  c2 == 0)
        continue;
      if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
	  (queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
        continue;
      if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
        continue;
		/* test criteria to join region */

      pred = calcAngleBetween(image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2],average);
      if ( pred > thres)
        continue;
      labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;
      if (indices != NULL)
        indices[count]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
      total.r+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2].r;
      total.c+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2].c;
      total.d+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2].d;
      (count)++;
      queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
      qh=(qh+1)%MAX_QUEUE;
      if (qh == qt)
        {
        printf("Max queue size exceeded\n");
        exit(0);
        }
      }
  qt=(qt+1)%MAX_QUEUE;
  }
}

int main(int argc, char *argv[]){

  FILE    *fpt;
  unsigned char *pOutImg = NULL;
  struct point3d img3dPts[128*128] = {0}; //Caution: hard coding here not Good
  char fileName[30];
  //int thres;
  int imgCSize,imgRSize;

  if( argc == 3){
  	thres = atoi(argv[1]);
  	inSize = atoi(argv[2]);

  }

  //check the command line argument for the file name
  strcpy(fileName,"img/chair-range.ppm");
  image origImg(fileName);
  const int ROWS = origImg.getRows();
  const int COLS = origImg.getCols();

  //make a copy and work on it
  strcpy(fileName,"img/chair-range.ppm");
  image tmpImg(fileName);

  //make a copy and work on it
  strcpy(fileName,"img/chair-range.ppm");
  image degImg(fileName);
  struct point3d *pImgNormals = new struct point3d[ROWS * COLS];

  imgCSize = tmpImg.getCols();
  imgRSize = tmpImg.getRows();
  thresholdImage(tmpImg,59,140);
  unsigned char *imgPixels = tmpImg.getPixels();
  int r,c;

  for(c = 0;c<(COLS-inSize);c++){
  	for(r = 0;r<(ROWS-inSize);r++){

  	  struct point ptX,ptA,ptB = {0,0};
	  struct point3d pt3dX,pt3dA,pt3dB = {0,0,0};
	  struct point3d vecAX,vecBX,vecNorm={0,0,0};
	  double magNorm,thetaNorm,tot=0;	//theta is wrt to yz plane
	  
	  ptX.r = r;
	  ptX.c = c;

	  //accroding to lectture notes naming the points
	  ptB.r = r+inSize;
	  ptB.c = c;
	  ptA.r = r;
	  ptA.c = c+inSize;

	  //need to do only for the thresholded pixels
	  if(imgPixels[r*COLS+c] != 0){


	  	pt3dX = get3dCoords(origImg,ptX);	
	  	pt3dA = get3dCoords(origImg,ptA);
	  	pt3dB = get3dCoords(origImg,ptB);

//  		printf("%f,%f,%f  ",pt3dX.c,pt3dX.r,pt3dX.d);

	  	vecAX.r = pt3dA.r - pt3dX.r;
	  	vecAX.c = pt3dA.c - pt3dX.c;
	  	vecAX.d = pt3dA.d - pt3dX.d;

	  	vecBX.r = pt3dB.r - pt3dX.r;
	  	vecBX.c = pt3dB.c - pt3dX.c;
	  	vecBX.d = pt3dB.d - pt3dX.d;

	  	vecNorm = crossProduct(vecAX,vecBX);

	  }

	  // tot = (vecNorm.r*vecNorm.r)+(vecNorm.c*vecNorm.c)+(vecNorm.d*vecNorm.d);
	  // magNorm = sqrt(tot);
	  //magNorm = calcMagnitude(vecNorm);
	  //thetaNorm = calcTheta(vecNorm);
  
	  //printf("%4d",(int)thetaNorm);
	  //degImg.getppPixels()[r][c] = thetaNorm;

	  pImgNormals[r*COLS+c].r = vecNorm.r;
	  pImgNormals[r*COLS+c].c = vecNorm.c;
	  pImgNormals[r*COLS+c].d = vecNorm.d;

	  //if(r == 108 && c == 87)
	  	//printf("%d,%d,%d  ",pImgNormals[i*tmpImg.getCols()+j].r,pImgNormals[i*tmpImg.getCols()+j].c,pImgNormals[i*tmpImg.getCols()+j].d);
	  //printf("%f,%f,%f \n",vecNorm.c,vecNorm.r,vecNorm.d);

	  
  	}
  	//std::cout<<std::endl;
  }


  int TotalRegions=0;
  struct point3d avg,var;
  int RegionSize=0;
  unsigned char *labels=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	/* used to quickly erase small grown regions */
  int *indices=(int *)calloc(ROWS*COLS,sizeof(int));;

  int region6cnt = 0;
  for(int c = 0;c<COLS;c++){
  	for(int r = 0;r<ROWS;r++){

  		if(imgPixels[r*COLS+c] == 0){
  			labels[r*COLS+c] = 255;
  			region6cnt+=1;
  		}

  	}
  }


	for (r=2; r<ROWS-2; r++)
	  {
	  for (c=2; c<COLS-2; c++)
	    {
	    if (labels[r*COLS+c] != 0)
	      continue;

	  	bool seedIt = true;
	    for (int r2=-2; r2<=2; r2++){
	      for (int c2=-2; c2<=2; c2++){
	         
	         if(labels[(r+r2)*COLS+(c+c2)] != 0){
	         	seedIt = false;
	         	break;
	         }
	         	
	      }
	      
	  	}
	    
	    
	    if (seedIt == true)	/* condition for seeding a new region is low var */
	      {
	      	// printf("%d,%d avg=%lf var=%lf\n",r,c,avg,var);
	      TotalRegions++;
	      if (TotalRegions == 255)
	        {
	        printf("Segmentation incomplete.  Ran out of labels.\n");
	        break;
	        }
	      RegionGrow(pImgNormals,labels,ROWS,COLS,r,c,0,TotalRegions,indices,RegionSize);
	      if (RegionSize < 100)
	        {	/* erase region (relabel pixels back to 0) */
	        for (int i=0; i<RegionSize; i++)
	          labels[indices[i]]=0;
	          TotalRegions--;
	        }
	      else{
	        printf("Region labeled %d is %d in size\n",TotalRegions,RegionSize);

	        struct point3d avgNorm = {0,0,0};
	        long double a1=0;
	        long double a2=0;
	        long double a3=0;
	        for(int x=0;x<RegionSize;x++){

	        	//printf("%d\n",x );
	        	a1 += pImgNormals[indices[x]].c;
	        	a2 += pImgNormals[indices[x]].r;
	        	a3 += pImgNormals[indices[x]].d;

	        	struct point rc;
	        	rc.r = (double)indices[x]/ROWS;
	        	rc.c = (indices[x])%COLS;
	        	struct point3d rca = get3dCoords(tmpImg,rc);

	        	// if(labels[r*COLS+c] == 1){
	        	// 	printf("%d (%f,%f)  (%f,%f,%f)\n",labels[r*COLS+c],rc.c,rc.r,pImgNormals[indices[x]].c,pImgNormals[indices[x]].r,pImgNormals[indices[x]].d);
	        	// }

	        }

	        avgNorm.c = a1/RegionSize;
	        avgNorm.r = a2/RegionSize;
	        avgNorm.d = a3/RegionSize;
	        printf("%d %f,%f,%f\n",labels[r*COLS+c],avgNorm.c,avgNorm.r,avgNorm.d);
	    	}
	      }

	    }
	  	if (c < COLS-3)
	    break;	/* ran out of labels -- break both loops */
	  }
	printf("%d total regions were found\n",TotalRegions);
	printf("Region labeled 6 is %d in size\n",region6cnt);
  
  unsigned char **outPix = degImg.getppPixels();
  double val = 40;
  for(r = 0; r<ROWS; r++){
  	for(c = 0; c<COLS; c++){

  		outPix[r][c] = (unsigned char)val*labels[r*COLS+c];
  		//printf("%d %f,%f,%f\n",labels[r*COLS+c],pImgNormals[r*COLS+c].c,pImgNormals[r*COLS+c].r,pImgNormals[r*COLS+c].d);

  	}

  }


  //we will have a degree image hereso need to do regeion grow on that
  //strcpy(fileName,"img/output.ppm");
  //saveImage(tmpImg.getPixels(),tmpImg.getCols(),tmpImg.getRows(),fileName);

  strcpy(fileName,"img/outdeg.ppm");
  saveImage(degImg.getPixels(),degImg.getCols(),degImg.getRows(),fileName);

  return 0;

}
