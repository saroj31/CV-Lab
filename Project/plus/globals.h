
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;
int		ShowBigDots;
int   draw = 0, drag = 0;
int   playmode = 0, stepmode = 0, done = 0;
int   x, y;
int   value, lock = 0;


		// Image data
unsigned char	*OriginalImage,*data,*smooth;
int				ROWS,COLS,total_count;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning;
int     xclick, yclick, xlock, ylock, lockindex;
int     i = 0, j = 0, p = 0, q = 0;
int     play = 0, step = 0;
float   abs_dif = 10, cent_dis = 10;
int     line[1000][2] = { 0 }, sline[500][2] = { 0 };
int     contnum = 0, thread = 0;
int     wind = 5, winball = 2;
int     decisionOver2;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage();
void AnimationThread(void *);		/* passes address of window */