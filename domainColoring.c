#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <complex.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

// Iterations to go through
#define START_VALUE  0
#define END_VALUE 50

// Screen Resolution
#define X_RESN 800
#define Y_RESN 600

// Ranges
#define REAL_MIN -4
#define REAL_MAX 4
#define IMAG_MIN -4
#define IMAG_MAX 4

// Defined constants
const double PI = 3.1415926535897932384626433832795;
const double E  = 2.7182818284590452353602874713527;
 
// Function to evaluate, change this to get a new graph
// c: Complex input
// x: horizontal coordinate of current pixel
// y: vertical coordinate of current pixel
// n: current iteration(time) 
// Returns a complex variable
complex fun( complex c, int x, int y, int n ){
  complex ret = 0;
  complex i = 1 + n * .05;
 
  // Tetration 
  int k = 0;
  ret = c;

  while ( k <= n*2 + 1)
  {
     ret = ctan(ctan(cpow(ret,ret)));
     k ++;
  }

  return ret;
}
 
int main()
{
   // X11 variables setup
   Window win;    
   unsigned int width, height, x, y, border_width, display_width, display_height,screen; 
   char *window_name = "Domain coloring", *display_name = NULL;
   GC gc;
   unsigned long valuemask = 0;
   XGCValues values;
   Display *display;
   XSizeHints size_hints;
   Pixmap bitmap;
   XPoint points[1000];
   char str[100];

   // Connect to X11 server
   XSetWindowAttributes attr[1];
   if (  (display = XOpenDisplay (display_name)) == NULL ) {
      fprintf (stderr, "drawon: cannot connect to X server %s\n",
      XDisplayName (display_name) );
   }

  // Setup the screen
  screen = DefaultScreen (display);
  display_width = DisplayWidth (display, screen);
  display_height = DisplayHeight (display, screen);
  width = X_RESN;
  height = Y_RESN;
  x = 0;
  y = 0;
  border_width = 4;
  win = XCreateSimpleWindow (display, RootWindow (display, screen),
     x, y, width, height, border_width, 
     BlackPixel (display, screen), WhitePixel (display, screen));

  size_hints.flags = USPosition|USSize;
  size_hints.x = x;
  size_hints.y = y;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.min_width = 400;
  size_hints.min_height = 400;
  XSetNormalHints (display, win, &size_hints);
  XStoreName(display, win, window_name);
  gc = XCreateGC (display, win, valuemask, &values);
  XSetBackground (display, gc, WhitePixel (display, screen));
  XSetForeground (display, gc, BlackPixel (display, screen));
  XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

  attr[0].backing_store = Always;
  attr[0].backing_planes = 1;
  attr[0].backing_pixel = BlackPixel(display, screen);

  XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

  XMapWindow (display, win);
  XFlush(display);

  // Domain coloring variable setup
  const int dimx = X_RESN; 
  const int dimy = Y_RESN;
  const double real_min = REAL_MIN; 
  const double real_max = REAL_MAX;
  const double imag_min = IMAG_MIN; 
  const double imag_max = IMAG_MAX;
  int i,j,count;
  char fileString[25];

  // Setup for a primitive status indicator
  printf("00%% ");
  fflush(stdout);

  // Perform the actual domain coloring
  for(count = START_VALUE; count < END_VALUE; count++)
  {
     sprintf(fileString, "data/complex%03d.ppm", count - START_VALUE);
     printf("\b\b\b\b%02d%% ", 100*count/END_VALUE);
     fflush(stdout);
     FILE * fp = fopen(fileString,"wb");
     fprintf(fp,"P6\n%d %d\n255\n",dimx,dimy);
     for(j=0;j<dimy;++j)
     {
       double imag = imag_max - (imag_max-imag_min)*j/(dimy-1);
       for(i=0;i<dimx;++i)
       {
         double real = real_max - (real_max - real_min) * i/(dimx-1);
         complex c = real + imag * I;
         complex v = fun(c, i,j, count);
         double a = carg(v);
         while(a<0) a += 2*PI; a /= 2*PI;
         double m = cabs(v);
         double range_start = 0;
         double range_end = 1;
         while(m>range_end)
         {
            range_start = range_end;
            range_end *= E;
         }
         double k = (m-range_start)/(range_end-range_start);
         double sat = 0.0;
         if ( k < .5 ) sat = k*2;
         else sat = 1 - ( k - .5) * 2;
         double val = sat; 
         sat = 1 - pow( (1-sat), 3); 
         sat = 0.4 + sat*0.6;
         val = 1 - val;
         val = 1 - pow( (1-val), 3); 
         val = 0.6 + val*0.4;

         // Calculate the RGB values
         static unsigned char color[3];
         double r,g,b;
         if(sat==0)
         {
            r = g = b = val;
         } else {
            if(a==1) a = 0;
            double z = floor(a*6); 
            int w = z;
            double f = a*6 - z;
            double p = val*(1-sat);
            double q = val*(1-sat*f);
            double t = val*(1-sat*(1-f));
            switch(w)
            {
               case 0: 
                  r=val; 
                  g=t; 
                  b=p; 
                  break;
               case 1:
                  r=q; 
                  g=val; 
                  b=p; 
                  break;
               case 2: 
                  r=p; 
                  g=val; 
                  b=t; 
                  break;
               case 3: 
                  r=p; 
                  g=q; 
                  b=val; 
                  break;
               case 4:
                  r=t; 
                  g=p; 
                  b=val; 
                  break;
               case 5: 
                  r=val; 
                  g=p; 
                  b=q;
                  break;
             }
         }   

         // Fix colors and output them to both the screen and file
         color[0] = (r >= 1) ? 255 : r*256;
         color[1] = (g >= 1) ? 255 : g*256;
         color[2] = (b >= 1) ? 255 : b*256;
         fwrite(color,1,3,fp);
         XSetForeground(display, gc, (long)(color[0] + (color[1] << 8) + (color[2] << 16)));
         XDrawPoint (display, win, gc, i, j);
      }
     }
     fclose(fp);
  }
  return 0;
}
