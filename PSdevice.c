/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 *
 *	11/25/89 - Fixed PSFillBox, PSDoHorizontal, and PSDoVertical
 *		so that cross hatching works.  D. R. Miller - MITRE Corp.
 */
#include <stdio.h>
#include <sys/file.h>
#include "device.h"
#include <string.h>

#define TYPEMASK 0x07
#define SPACEMASK 0xf8
#define ANGLEMASK 0x300



extern char *currentfilename ;

static FILE *fp = NULL ;

void PSOpenDevice()
{
    char *outfile ;
    char *z ;
	
    outfile = (char *) malloc(strlen(currentfilename)+6) ;
    strcpy(outfile,currentfilename) ;
    z = strrchr(outfile,'.') ;
    if(z == NULL)
      z = outfile+strlen(outfile) ;
    strcpy(z,".ps") ;
    if ((fp = fopen(outfile,"w")) == NULL) {
        perror("Flea!:") ;
        exit(1) ;
    }
    free(outfile) ;
    fprintf(fp,"%%!PS-Flea output file\n") ;
    fprintf(fp,"initgraphics grestoreall 72.0 300.0 div 72.0 300.0 div scale\n") ;
    fprintf(fp,"/line { newpath moveto lineto stroke } def\n");
    fprintf(fp,"\n");
    fprintf(fp,"/text\n");
    fprintf(fp,"{\t/Courier-Bold findfont exch scalefont setfont gsave\n");
    fprintf(fp,"\ttranslate rotate translate moveto true charpath fill stroke grestore\n");
    fprintf(fp,"} def\n");
    fprintf(fp,"\n");
}

void LWOpenDevice()
{
    char *pipedev, *getenv() ;
    
    if((pipedev = getenv("LWOUTPUT")) == NULL)
      pipedev = "lpr -Pps" ;
    if ((fp = popen(pipedev,"w")) == NULL) {
        fprintf(stderr,"popen(\"%s\") failed\n",pipedev) ;
        perror("Flea!:") ;
        exit(1) ;
    }
    fprintf(fp,"%%!PS-Flea output file\n") ;
    fprintf(fp,"initgraphics grestoreall 72.0 300.0 div 72.0 300.0 div scale\n") ;
    fprintf(fp,"/line { newpath moveto lineto stroke } def\n");
    fprintf(fp,"\n");
    fprintf(fp,"/text\n");
    fprintf(fp,"{\t/Courier-Bold findfont exch scalefont setfont gsave\n");
    fprintf(fp,"\ttranslate rotate translate moveto true charpath fill stroke grestore\n");
    fprintf(fp,"} def\n");
    fprintf(fp,"\n");
}

void PSCloseDevice()
{
    if(fp == NULL)
      return ;
    
    fprintf(fp,"showpage\n") ;
    fclose(fp) ;
    fp = NULL ;
}

void PSHardCloseDevice()
{
    if(fp == NULL)
      return ;
    
    fprintf(fp,"%% Improperly terminated\n") ;
    fclose(fp) ;
    fp = NULL ;
}

void LWCloseDevice()
{
    if(fp == NULL)
      return ;
    fprintf(fp,"showpage\n") ;
    pclose(fp) ;
    fp = NULL ;
}

void LWHardCloseDevice()
{
    if(fp == NULL)
      return ;
    fprintf(fp,"%% Improperly terminated\n") ;
    pclose(fp) ;
    fp = NULL ;
}

void PSSelectLineStyle(style)
int style ;
{
    static int laststyle = -1;
    
    if(laststyle == style)
      return ;
    laststyle = style ;
    if(laststyle == -1)
      fprintf(fp,"[] 0 setdash\n");
    else {
        style %= 10;            /* styles 0 thru 9 */
        switch( style ) {
          case 0:
            fprintf(fp,"[] 0 setdash\n");
            break;
          case 1:
            fprintf(fp,"[20 20] 0 setdash\n");
            break;
          case 2:
            fprintf(fp,"[48 20] 0 setdash\n");
            break;
          case 3:
            fprintf(fp,"[48 12 4 12] 0 setdash\n");
            break;
          case 4:
            fprintf(fp,"[40 20 20 20] 0 setdash\n");
            break;
          case 5:
            fprintf(fp,"[4 12 8 12] 0 setdash\n");
            break;
          case 6:
            fprintf(fp,"[48 12 20 12 4 12] 0 setdash\n");
            break;
          case 7:
            fprintf(fp,"[60 20 20 20 20 20] 0 setdash\n");
            break;
          case 8:
            fprintf(fp,"[20 20 40 20 4 20 40 20] 0 setdash\n");
            break;
          case 9:
          default:
            fprintf(fp,"[4 12 4 12] 0 setdash\n");
            break;
        }
    }
}

void PSDrawLine(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    fprintf(fp,"%d %d %d %d line\n",y1,x1,y2,x2) ;
}


extern struct pts list_o_pts[] ;

void PSPolygon(numpts)
int numpts ;
{
    int i ;
	
    for(i = 0; (i+1) < numpts ; i++)
      PSDrawLine(list_o_pts[i].x,list_o_pts[i].y,list_o_pts[i+1].x,list_o_pts[i+1].y) ;
}

#define ROTVEC {double t ; t = -nx; nx = ny; ny = t; }
#define NROTVEC {double t ; t = -ny; ny = nx ; nx = t; }
#define MAX(a,b)  ((a>b)?a:b)
#define MIN(a,b)  ((a<b)?a:b)

#define SGN(a)  (a>=0?1:-1)

void PSBox(length,width,cx,cy,vx,vy)
int length,width,cx,cy,vx,vy ;
{
    double nx,ny ;              /* normalized direction vector */
    double nc ;
    double px,py, floor(), sqrt() ;
    int x1,x2,y1,y2 ;
	
    nc = sqrt((double)(vx*vx + vy*vy)) ;
    nx = vx/nc ;
    ny = vy/nc ;
    px = nx*((double)length)/2.0 + cx ;
    py = ny*((double)length)/2.0 + cy ;
    ROTVEC ;
    px = nx*((double)width)/2.0 + px ;
    py = ny*((double)width)/2.0 + py ;
    x1 = (int) floor(px + .5) ;
    y1 = (int) floor(py + .5) ;
    ROTVEC ;
    px = nx*length + px ;
    py = ny*length + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PSDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*width + px ;
    py = ny*width + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PSDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*length + px ;
    py = ny*length + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PSDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*width + px ;
    py = ny*width + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PSDrawLine(x1,y1,x2,y2) ;
}

static int fillstyle ;

void PSSelectFillStyle(style)
int style ;
{
    if(fillstyle == style)
      return ;
    fillstyle = style ;
}
int ps_domod(x,y)
int x,y ;
{
    if (x % y >= 0)
      return(x % y) ;
    else
      return(y + x % y) ;
}


void PSDoHorizontal(x1,y1,x2,y2,sp)
int x1 , y1 , x2 , y2 , sp ;
{
    int curx ;

    if (ps_domod(curx,sp))
      curx = (x1 + sp) / sp * sp ;
    else
      curx = x1 + sp ;
    while (curx <= x2) {
        PSDrawLine(curx,y1,curx,y2) ;
        curx += sp ;
    }
}

void PSDoVertical(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int cury ;

    if (ps_domod(cury,sp))
      cury = (y1 + sp) / sp * sp ;
    else
      cury = y1 + sp ;
    while (cury <= y2) {
        PSDrawLine(x1,cury,x2,cury) ;
        cury += sp ;
    }
}

void PSDoLeftSlant(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int curx , cury ;
    int xp , yp  ;

    curx = x1 + ps_domod(y1-x1,sp) ;
    while (curx <= x2) {
        xp = y2 - y1 + curx ;
        yp = y2 ;
        if (xp > x2) {
            xp = x2 ;
            yp = x2 - curx + y1 ;
        }
        PSDrawLine(curx,y1,xp,yp) ;
        curx += sp ;
    }
    cury = y1 + ps_domod(x1-y1,sp) ;
    while (cury <= y2) {
        xp = y2 - cury + x1 ;
        yp = y2 ;
        if (xp > x2) {
            xp = x2 ;
            yp = x2 + cury - x1 ;
        }
        PSDrawLine(x1,cury,xp,yp) ;
        cury += sp ;
    }
}

void PSDoRightSlant(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int curx , cury ;
    int xp , yp  ;
	
    curx = x1 +  ps_domod(-y1-x1,sp) ;
    while (curx <= x2) {
        xp = curx + y1 - y2 ;
        yp = y2 ;
        if (xp < x1) {
            xp = x1 ;
            yp = curx-x1+y1 ;
        }
        PSDrawLine(curx,y1,xp,yp) ;
        curx += sp ;
    }
    cury = y1 + ps_domod(-x2-y1,sp) ;
    while (cury <= y2) {
        xp = cury - y2 + x2 ;
        yp = y2 ;
        if (xp < x1) {
            xp = x1 ;
            yp = cury - x1 + x2 ;
        }
        PSDrawLine(x2,cury,xp,yp) ;
        cury += sp ;
    }
}

void PSFillBox(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    int type ;
    int spacing ;
    int angle ;
    int temp ;
    int oldfillstyle ;

    type = fillstyle & 7 ;
    spacing = ((fillstyle >> 3) & 31) * 10;
    angle = (((fillstyle >> 8) & 7) * 45) % 180 ;
    if (x1 > x2) {
        temp = x1 ;
        x1 = x2 ;
        x2 = temp ;
    }
    if (y1 > y2) {
        temp = y1 ;
        y1 = y2 ;
        y2 = temp ;
    }
    if (type == 1 || type == 2)
      return ;
    if (type == 4) {
        oldfillstyle = fillstyle ;
        fillstyle &= ~TYPEMASK ;
        fillstyle |= 3 ;
        PSFillBox(x1,y1,x2,y2) ;
        temp = fillstyle & ANGLEMASK;
        temp += 0x200;
        fillstyle &= ~ANGLEMASK ;
        fillstyle |= temp;
        PSFillBox(x1,y1,x2,y2) ;
        fillstyle = oldfillstyle ;
        return ;
    }
    switch(angle) {
      case 0:
        PSDoHorizontal(x1,y1,x2,y2,spacing) ;
        break ;
      case 45:
        PSDoLeftSlant(x1,y1,x2,y2,spacing) ;
        break ;
      case 90:
        PSDoVertical(x1,y1,x2,y2,spacing) ;
        break ;
      case 135:
        PSDoRightSlant(x1,y1,x2,y2,spacing) ;
        break ;
    }
}



void PSSelectPen(PenNumber)
int PenNumber ;
{
    fprintf(fp,"%d setlinewidth %f setgray %% PenNumber %d\n",3-(PenNumber&0x3),(double)(PenNumber>>2)/4,PenNumber) ;
}

void PSDrawLabel(text, y, x, pos, size, rot)
char *text ;
int x, y, pos, size;
int rot ;
{   
    extern double sin(), cos(), atan2(), sqrt();
    extern char *strchr();
    int len, n;
    double x0, y0, x1, y1, xs, ys, xt, yt, degrees;
    double sw, sh, theta;

    if(rot)
      degrees = 0.0 ;
    else
      degrees = 90.0 ;
    if( strchr(text,'\n') != NULL)
      *(strchr(text,'\n')) = '\0';
    len = strlen(text);
    if (len == 0)
      return;
    sh = ( size / 100.0 ) * 400.0; /* convert from hundredths of an inch to point units */
    sw = .7 * sh;

    switch( pos ) {
      case 0:                   /*  center at cursor */
        x0 = x - sw * ( len / 2 );
        x1 = x + sw * ( len / 2 + 1 );
        y0 = y;
        y1 = y;
        break ;
      case 1:                   /*  center above cursor */
        x0 = x - sw * len / 2;
        x1 = x + sw * ( len / 2 + 1 );
        y0 = y + sh;
        y1 = y + sh;
        break ;
      case 2:                   /*  left justify above cursor */
        x0 = x;
        x1 = x + sw * ( len + 1 );
        y0 = y + sh;
        y1 = y + sh;
        break ;
      case 3:                   /*  left justify to the right of cursor */
        x0 = x + sw;
        x1 = x + sw * ( len + 1 );
        y0 = y;
        y1 = y;
        break ;
      case 4:                   /*  left justify below cursor */
        x0 = x + sw;
        x1 = x + sw * ( len + 1 );
        y0 = y - sh;
        y1 = y - sh;
        break ;
      case 5:                   /*  center below cursor */
        x0 = x - sw * len / 2;
        x1 = x + sw * ( len / 2 + 1 );
        y0 = y - sh;
        y1 = y - sh;
        break;
      case 6:                   /*  right justify below cursor */
        x0 = x - sw * len;
        x1 = x + sw;
        y0 = y - sh;
        y1 = y - sh;
        break;
      case 7:                   /*  right justify to the left of cursor */
        x0 = x - sw * len;
        x1 = x + sw;
        y0 = y;
        y1 = y;
        break ;
      case 8:                   /*  right justify above cursor */
        x0 = x - sw * len;
        x1 = x + sw;
        y0 = y + sh;
        y1 = y + sh;
        break ;
      case 9:                   /*  center two lines below cursor */
        x0 = x - sw * len;
        x1 = x + sw * ( len + 1 );
        y0 = y - 2 * sh;
        y1 = y - 2 * sh;
        break;
      case 10:                  /*  center two lines above cursor */
        x0 = x - sw * len;
        x1 = x + sw * ( len + 1 );
        y0 = y + 2 * sh;
        y1 = y + 2 * sh;
        break;
      default :
        break ;
    }
    /***************************************
     * now rotate (x0,y0) and (x1,y1)       *
     * about (x,y) to produce new (x0,y0)   *
     * and (x1,y1).                         *
     ***************************************/
    theta = -1 * degrees * 3.14159 / 180;

    x0 -= x;
    y0 -= y;
    x1 -= x;
    y1 -= y;
    
    xt = x0;
    yt = y0;

    x0 = xt * cos(theta) + yt * sin( theta);
    y0 = yt * cos(theta) - xt * sin( theta);
        
    xt = x1;
    yt = y1;
    x1 = xt * cos(theta) + yt * sin( theta);
    y1 = yt * cos(theta) - xt * sin( theta);
    
    x0 += x;
    y0 += y;
    x1 += x;
    y1 += y; 
    for(n = 0; n < len; n++) {
    	int x;
    	int y;
    	
        xs = x0 + ( ( x1 - x0 ) / len ) * n;
        ys = y0 + ( ( y1 - y0 ) / len ) * n;
        x = (int)xs;
        y = (int)ys;
        fprintf(fp,"(%c) %d %d %d %d %g %d %d %d text\n",
                text[n],x,y,-1*x,-1*y,degrees,x,y,(int)sh); 
    }
}



void PSDrawCross(x,y) 
int x,y ;
{
    PSDrawLine(x-15,y,x+15,y) ;
    PSDrawLine(x,y-15,x,y+15) ;
}

void PSInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"PS",STRINGSIZE) ;
    p->OpenDevice = PSOpenDevice ;
    p->CloseDevice = PSCloseDevice ;
    p->HardCloseDevice = PSHardCloseDevice ;
    p->DrawLine = PSDrawLine ;
    p->DrawCross = PSDrawCross ;
    p->SelectPen = PSSelectPen ;
    p->SelectFillStyle = PSSelectFillStyle ;
    p->SelectLineStyle = PSSelectLineStyle ;
    p->FillBox = PSFillBox ;
    p->DrawLabel = PSDrawLabel ;	
    p->Polygon = PSPolygon ;
    p->Box = PSBox ;
    p->plotter = YEA ;
    p->xAspect = 300 ;
    p->yAspect = 300 ;
    p->xLeft = 3170;
    p->xRight = 175 ;
    p->yTop = 200 ;
    p->yBottom = 2375 ;
}

void LWInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"LW",STRINGSIZE) ;
    p->OpenDevice = LWOpenDevice ;
    p->CloseDevice = LWCloseDevice ;
    p->HardCloseDevice = LWHardCloseDevice ;
    p->DrawLine = PSDrawLine ;
    p->DrawCross = PSDrawCross ;
    p->SelectPen = PSSelectPen ;
    p->SelectFillStyle = PSSelectFillStyle ;
    p->SelectLineStyle = PSSelectLineStyle ;
    p->FillBox = PSFillBox ;
    p->DrawLabel = PSDrawLabel ;
    p->Polygon = PSPolygon ;
    p->Box = PSBox ;
    p->plotter = YEA ;
    p->xAspect = 300 ;
    p->yAspect = 300 ;
    p->xLeft =   3170 ;
    p->xRight =  175 ;
    p->yTop =    200 ;
    p->yBottom = 2375 ;
}



