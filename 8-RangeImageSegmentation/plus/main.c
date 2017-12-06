
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"


int * image_copy;
int r, c;
unsigned char paint_over_label;
char new_label;
//int new_color;
DWORD new_color;
int *position;
int total_count;
int distance=500, intensity=5;
int playMode, jmpMode = 0;
int bwait;
HANDLE jmpEvt;
int image_reloaded = 0;

void RegionGrow(HWND AnimationWindowHandle);
void color_image(int *local, int count);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,400,400,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;
ShowBigDots = 0;

strcpy(filename,"");
OriginalImage=NULL;
ROWS=COLS=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}

BOOL CALLBACK getPredicate(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int dist, inten;
	BOOL ferror1, ferror2;
	switch (Message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			dist = GetDlgItemInt(hWnd, IDC_EDIT1, &ferror1, FALSE);
			inten = GetDlgItemInt(hWnd, IDC_EDIT2, &ferror2, FALSE);
			if (!ferror1)
			{
				if (!ferror2)
				{
					MessageBox(hWnd, "No values entered for Distance or Intensity. Default values selected", "Error", MB_OK | MB_ICONWARNING);
				}
				else
				{
					if (inten < 0 || inten > 255)
						MessageBox(hWnd, "Please enter correct values for Intensity gradient", "Error", MB_OK | MB_ICONWARNING);
					else
						intensity = inten;
				}
			}
			else
			{
				if (dist< 0)
					MessageBox(hWnd, "Please enter correct values for Distance to choose predicate ", "Error", MB_OK | MB_ICONWARNING);
				else
					distance = dist;
				if (ferror2)
					if (inten < 0 || inten > 255)
						MessageBox(hWnd, "Please enter correct values for Intensity gradient", "Error", MB_OK | MB_ICONWARNING);
					else
						intensity = inten;
			}
			EndDialog(hWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			DestroyWindow(hWnd);
			return TRUE;
			break;

		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)

{
HMENU				hMenu;
OPENFILENAME		ofn;
FILE				*fpt;
HDC					hDC;
char				header[320],text[320];
int					BYTES,xPos,yPos;

CHOOSECOLOR color;
static COLORREF acrCustClr[16];
static DWORD rgb_curr;
int ret;

switch (uMsg)
  {
  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
	  case ID_SHOWPIXELCOORDS:
		ShowPixelCoords=(ShowPixelCoords+1)%2;
		PaintImage();
		break;
  
	  case ID_FILE_LOAD:
		if (OriginalImage != NULL)
		  {
      for (int i = 0; i < ROWS*COLS; i++)
        OriginalImage[i]=255;
      PaintImage();
      image_reloaded = 1;
		  free(OriginalImage);
		  OriginalImage=NULL;
      
		  }
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=filename;
		filename[0]=0;
		ofn.nMaxFile=MAX_FILENAME_CHARS;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrFilter = "PPM files\0*.ppm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
		if (strcmp(header,"P5") != 0  ||  BYTES != 255)
		  {
		  MessageBox(NULL,"Not a PPM (P5 greyscale) image",filename,MB_OK | MB_APPLMODAL);
		  fclose(fpt);
		  break;
		  }
		OriginalImage=(unsigned char *)calloc(ROWS*COLS,1);
		image_copy = (int *)calloc(ROWS*COLS, sizeof(int));
		position = (int *)calloc(ROWS*COLS, sizeof(int));
		header[0]=fgetc(fpt);	/* whitespace character after header */
		fread(OriginalImage,1,ROWS*COLS,fpt);
		fclose(fpt);
		for (int i = 0; i < ROWS*COLS; i++)
			image_copy[i] = OriginalImage[i];
		SetWindowText(hWnd,filename);
		PaintImage();
		break;

      case ID_FILE_QUIT:
        DestroyWindow(hWnd);
        break;
	
	  case ID_REGIONGROWINGSETTINGS_COLOR:
		  ZeroMemory(&color, sizeof(color));	//macro to clear up the memory or set it black
		  color.lStructSize = sizeof(color);

		  color.hwndOwner = hWnd;
		  color.lpCustColors = (LPDWORD)acrCustClr;
		  color.rgbResult = rgb_curr;
		  color.Flags = CC_FULLOPEN | CC_RGBINIT;
		  if (ChooseColor(&color) == TRUE)
		  	  rgb_curr = color.rgbResult;
		  break;
	  
	  case ID_SET_PREDICATE:
		  ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, getPredicate);
		  PaintImage();
		  break;
	  case ID_REGIONGROWINGSETTINGS_STEPMODE:
		  playMode = 0;
		  jmpEvt = CreateEvent(
			  NULL,					
			  FALSE,				
			  FALSE,				
			  TEXT("J-Event")		
		  );
		  jmpMode = (jmpMode + 1) % 2;
		  image_reloaded = 0;
		  break;
	  case ID_REGIONGROWINGSETTINGS_PLAYMODE:
      jmpMode = 0;
		  playMode = (playMode + 1) % 2;
		  bwait = 0;
		  image_reloaded = 0;
      break;

    case ID_DISPLAY_RESTOREIMAGE:
      KillTimer(MainWnd, TIMER_SECOND);
      ThreadRunning = 0;
      for (int i = 0; i < ROWS*COLS; i++)
        image_copy[i] = OriginalImage[i];
      PaintImage();
      total_count = 0;
      playMode = 0;
      jmpMode = 0;
      image_reloaded = 1;
     
      break;
      }
    break;
  case WM_SIZE:		  /* could be used to detect when window size changes */
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_PAINT:
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:
	  c = LOWORD(lParam);
	  r = HIWORD(lParam);
	  if (c >= 0 && c < COLS  &&  r >= 0 && r < ROWS)
	  {
		  hDC = GetDC(MainWnd);
		  paint_over_label = GetRValue(GetPixel(hDC, c, r));  //gets the last value of the rGB value returned.since gray scale
      //paint_over_label = GetGValue(GetPixel(hDC, c, r));
      ////paint_over_label = GetBValue(GetPixel(hDC, c, r));
      
		  new_color = rgb_curr;
		  new_label = -1;
		  ReleaseDC(MainWnd, hDC);
		  _beginthread(RegionGrow, 0, MainWnd);
	  }
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;
  case WM_MOUSEMOVE:
	if (ShowPixelCoords == 1)
	  {
	  xPos=LOWORD(lParam);
	  yPos=HIWORD(lParam);
	  if (OriginalImage && xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
		{
		sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		hDC=GetDC(MainWnd);
		TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		//SetPixel(hDC,xPos, yPos, rgb_curr);	/* color the cursor position red */
		ReleaseDC(MainWnd,hDC);
		}
	  }
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_KEYDOWN:
	if (wParam == 's'  ||  wParam == 'S')
	  PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
	if ((TCHAR)wParam == '1')
	  {
	  TimerRow=TimerCol=0;
	  SetTimer(MainWnd,TIMER_SECOND,10,NULL);	/* start up 10 ms timer */
	  }
	if ((TCHAR)wParam == '2')
	  {
	  KillTimer(MainWnd,TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
	  PaintImage();								/* redraw original image, erasing animation */
	  }
	if ((TCHAR)wParam == '3')
	  {
	  ThreadRunning=1;
	  _beginthread(AnimationThread,0,MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
	  }
 	if ((TCHAR)wParam == '4')
	  {
	  ThreadRunning=0;							/* this is used to stop the child thread (see its code below) */
	  }
	if ((TCHAR)wParam == 'J' || (TCHAR)wParam == 'j' && playMode == 0)
	{
		jmpMode = 1;
    playMode = 0;
    
	}
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
	hDC=GetDC(MainWnd);
	SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
	ReleaseDC(MainWnd,hDC);
	TimerRow++;
	TimerCol+=2;
	break;
  case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage();
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
    break;
  }


hMenu=GetMenu(MainWnd);
if (ShowPixelCoords == 1)
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);

//if (ShowBigDots == 1)
//CheckMenuItem(hMenu, ID_DISPLAY_BIGDOTS, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
//else
//CheckMenuItem(hMenu, ID_DISPLAY_BIGDOTS, MF_UNCHECKED);

if (playMode == 1)
CheckMenuItem(hMenu, ID_REGIONGROWINGSETTINGS_PLAYMODE, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
CheckMenuItem(hMenu, ID_REGIONGROWINGSETTINGS_PLAYMODE, MF_UNCHECKED);

if (jmpMode == 1)
CheckMenuItem(hMenu, ID_REGIONGROWINGSETTINGS_STEPMODE, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
CheckMenuItem(hMenu, ID_REGIONGROWINGSETTINGS_STEPMODE, MF_UNCHECKED);

DrawMenuBar(hWnd);

return(0L);
}




void PaintImage()
{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
unsigned char		*DisplayImage;

if (OriginalImage == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
  for (c=0; c<COLS; c++)
	DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8; 
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);

free(DisplayImage);
free(bm_info);
}




void AnimationThread(HWND AnimationWindowHandle)

{
HDC		hDC;
char	text[300];

ThreadRow=ThreadCol=0;
while (ThreadRunning == 1)
  {
  hDC=GetDC(MainWnd);
  SetPixel(hDC,ThreadCol,ThreadRow,RGB(0,255,0));	/* color the animation pixel green */
  sprintf(text,"%d,%d     ",ThreadRow,ThreadCol);
  TextOut(hDC,300,0,text,strlen(text));		/* draw text on the window */
  ReleaseDC(MainWnd,hDC);
  ThreadRow+=3;
  ThreadCol++;
  Sleep(100);		/* pause 100 ms */
  }
}


#define MAX_QUEUE 100000	/* max perimeter size (pixels) of border wavefront */

void RegionGrow(HWND AnimationWindowHandle)
{
	int	r2, c2;
	int	queue[MAX_QUEUE], qh, qt;
	int	average, total;	/* average and total intensity in growing region */
	int *indices = (int *)calloc(ROWS*COLS, sizeof(int));
	int count = 0;
	HDC	hdc;

	int xnbr = 0;
	int ynbr = 0;
	int cum_x = 0;
	int cum_y = 0;
	int abs_dist = 0;
	int diff = 0;
	int avg = 0, cum_inten = 0;

	if (image_copy[r*COLS + c] != paint_over_label)
		return;
	indices[0] = r*COLS + c;
	image_copy[r*COLS + c] = new_label;
	avg = cum_inten = OriginalImage[r*COLS + c];
	hdc = GetDC(MainWnd);
  SetPixel(hdc, indices[0] % COLS, indices[0] / COLS, RGB(GetRValue(new_color), GetGValue(new_color), GetBValue(new_color)));
	position[total_count++] = indices[0];
	ReleaseDC(MainWnd, hdc);
	queue[0] = r*COLS + c;
	qh = 1;
	qt = 0;
	count = 1;
	color_image(position, total_count);
	do
	{
		for (r2 = -1; r2 <= 1; r2++)
		{
			for (c2 = -1; c2 <= 1; c2++)
			{
				if (r2 == 0 && c2 == 0)
					continue;
				if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS ||
					(queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS)
					continue;
				//the intensity rule of predicate
				diff = abs((int)(image_copy[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2]) - avg);
				if ((diff) > intensity)
					continue;
				cum_x += queue[qt] / COLS + r2;
				cum_y += queue[qt] % COLS + c2;
				xnbr = cum_x / count;   //nbr means neighbors
				ynbr = cum_y / count;
				//euclidean distance
				abs_dist = (int)abs(sqrt(SQR(xnbr - queue[qt] / COLS + r2) + SQR(ynbr - queue[qt] % COLS + c2))); 

				if (distance != 0)
				//the distance rule of the predicate
					if (abs_dist > distance)
					{
						cum_x -= queue[qt] / COLS + r2;
						cum_y -= queue[qt] % COLS + c2;
						continue;
					}
				image_copy[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2] = new_label;
				indices[count] = (queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2;
				cum_inten += OriginalImage[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2];
				hdc = GetDC(MainWnd);
        SetPixel(hdc, indices[count] % COLS, indices[count] / COLS, RGB(GetRValue(new_color), GetGValue(new_color), GetBValue(new_color)));

        position[total_count++] = indices[count];
				ReleaseDC(MainWnd, hdc);
				count++;
				avg = cum_inten / count;
				queue[qh] = (queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2;
				qh = (qh + 1) % MAX_QUEUE;
				if (qh == qt) 	printf("Max queue size exceeded\n");
				
			}
		}
		qt = (qt + 1) % MAX_QUEUE;
		if (playMode)			Sleep(1);
		
    if (jmpMode) 	bwait = 1;

		if (bwait)
		{
			jmpMode = 0;
			DWORD dwwait_step;
			while (!playMode && !jmpMode)
			{
				dwwait_step = WaitForInputIdle(jmpEvt, 100);
				switch (dwwait_step)
				{
				case 0:
					jmpMode = 1;
					break;
				}
				if (image_reloaded == 1)
				{
					break;
				}
			}
		}
	} while (qt != qh && (playMode || jmpMode));
}


void color_image(int *local, int count)
{
	int x = 0, y = 0;
	int i = 0;
	HDC hdc = GetDC(MainWnd);
	if (count == 0)
	{
		ReleaseDC(MainWnd, hdc);
		return;
	}
	for (i = 0; i < count; i++)
	{
		x = local[i] / COLS;
		y = local[i] % COLS;
		SetPixel(hdc, y, x, RGB(GetRValue(new_color), GetGValue(new_color), GetBValue(new_color)));
	}
	ReleaseDC(MainWnd, hdc);
}

