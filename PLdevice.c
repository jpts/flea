/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include "device.h"
#include <string.h>

#ifdef HP7580
#include <sgtty.h>
#include "/msu/include/plotters.h"
#endif HP7580

#define DEV "hp"
#define ESC 0x1b
#define ETX 03
#define TYPEMASK 0x07
#define SPACEMASK 0xf8
#define ANGLEMASK 0x300

static FILE *fp = NULL ;
static int fillstyle = -255 ;
#ifdef __STDC__
void PLDrawLine(int,int,int,int), PLSelectPen(int) ;
void PLSelectLineStyle(int), PLDrawLabel(char *,int, int, int, int, int) ;
void PLDrawCross(int,int), PLBox(int,int,int,int,int,int) ;
void PLPolygon(int), PLWire(int,int), PLRoundFlash(int,int,int) ;
void HPFillBox(int,int,int,int), HPSelectFillStyle(int) ;
#else
void PLDrawLine(), PLSelectPen(int), PLSelectLineStyle(), PLDrawLabel() ;
void PLDrawCross(), PLBox(), PLPolygon(), PLWire(), PLRoundFlash() ;
void HPFillBox(), HPSelectFillStyle() ;
#endif
#ifdef HP7580
HPOpenDevice()
{
    int x1,y1,x2,y2,stat ;
    struct sgttyb mode ;
    char ib[80];
    int ibs,i ;
	
    while((fp = checklock(DEV,&stat))==NULL){
        if(stat == NOPLOTTER) {
            exit(-1) ;
        }
        if(stat == OPENFAILED) {
            exit(-1) ;
        }
        if(stat == LOCKFAILED) {
            fprintf(stderr,"HP: Someone else is using the plotter.\n") ;
            fprintf(stderr,"HP: Waiting for plotter freedom.\n") ;
        }
        sleep(60) ;
    }
    if (gtty(fileno(fp),&mode) == -1) {
        fprintf(stderr,"Output device is not a character device!\n") ;
        fprintf(stderr,"Check '%s' entry in /etc/plotcap.\n",DEV) ;
        endlock(DEV) ;
        exit(-1) ;
    }
    mode.sg_flags  |= (TANDEM + CBREAK + CRMOD ) ;
    mode.sg_flags  &= ~(RAW + ECHO + LCASE) ;
    mode.sg_ispeed = B4800 ;
    mode.sg_ospeed = B4800 ;
    if (stty(fileno(fp),&mode) == -1) {
        fprintf(stderr,"Output device is not a character device!\n") ;
        fprintf(stderr,"Check '%s' entry in /etc/plotcap.\n",DEV) ;
        endlock(DEV) ;
        exit(-1) ;
    }
    fprintf(fp,"%c.(%c.I124;;17:%c.N;19:%c.@1024;2:\n",
            ESC,ESC,ESC,ESC) ;
    fprintf(fp,"IN;\n") ;
    fprintf(fp,"OP;") ;
    fflush(fp) ;
    getnums(fileno(fp),&x1,&y1,&x2,&y2) ;
    CurrentDevice->xLeft = x2 ;
    CurrentDevice->xRight = x1 ;
    CurrentDevice->yTop = y1 ;
    CurrentDevice->yBottom = y2 ;
    fprintf(fp,"PA;PU;LT;DT%c;\n",ETX) ;
}

readint(fd)
{
    char buf[80] ;
    int i ;
	
    for(i = 0 ; i < 80 ; i++) {
        if(read(fd,&(buf[i]),1)<=0) {
            perror("readint:") ;
            exit(-1) ;
        }
        if((buf[i] >= '0' && buf[i] <= '9') || buf[i] == '-')
          continue ;
        return(atoi(buf)) ;
    }
}


getnums(fd,x1,y1,x2,y2)
int fd, *x1, *y1, *x2, *y2 ;
{
    *x1 = readint(fd) ;
    *y1 = readint(fd) ;
    *x2 = readint(fd) ;
    *y2 = readint(fd) ;
}




HPCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;NR;\n") ;
    endlock(DEV) ;
    fclose(fp) ;
    fp = NULL ;
}

HPHardCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;NR;\n") ;
    endlock(DEV) ;
    fclose(fp) ;
    fp = NULL ;
}


HPInit()
{
    struct Device *p ;
    p = BuildDevice() ;
    strncpy(p->DeviceName,"HP",STRINGSIZE) ;
    p->OpenDevice = HPOpenDevice ;
    p->CloseDevice = HPCloseDevice ;
    p->HardCloseDevice = HPHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = HPSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = HPFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->DrawCross = PLDrawCross ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft =   250 ;
    p->xRight =  10000 ;
    p->yTop =    596 ;
    p->yBottom = 7796 ;
}

#endif
int domod(x,y)
int x,y ;
{
    if (x % y >= 0)
      return(x % y) ;
    else
      return(y + x % y) ;
}


void HPDoHorizontal(x1,y1,x2,y2,sp)
int x1 , y1 , x2 , y2 , sp ;
{
    int curx ;

    if (domod(curx,sp))
      curx = (x1 + sp) / sp * sp ;
    else
      curx = sp ;
    while (curx <= x2) {
        PLDrawLine(curx,y1,curx,y2) ;
        curx += sp ;
    }
}

void HPDoVertical(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int cury ;

    if (domod(cury,sp))
      cury = (y1 + sp) / sp * sp ;
    else
      cury = sp ;
    while (cury <= y2) {
        PLDrawLine(x1,cury,x2,cury) ;
        cury += sp ;
    }
}

void HPDoLeftSlant(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int curx , cury ;
    int xp , yp  ;

    curx = x1 + domod(y1-x1,sp) ;
    while (curx <= x2) {
        xp = y2 - y1 + curx ;
        yp = y2 ;
        if (xp > x2) {
            xp = x2 ;
            yp = x2 - curx + y1 ;
        }
        PLDrawLine(curx,y1,xp,yp) ;
        curx += sp ;
    }
    cury = y1 + domod(x1-y1,sp) ;
    while (cury <= y2) {
        xp = y2 - cury + x1 ;
        yp = y2 ;
        if (xp > x2) {
            xp = x2 ;
            yp = x2 + cury - x1 ;
        }
        PLDrawLine(x1,cury,xp,yp) ;
        cury += sp ;
    }
}

void HPDoRightSlant(x1,y1,x2,y2,sp)
int x1,y1,x2,y2,sp ;
{
    int curx , cury ;
    int xp , yp  ;
	
    curx = x1 +  domod(-y1-x1,sp) ;
    while (curx <= x2) {
        xp = curx + y1 - y2 ;
        yp = y2 ;
        if (xp < x1) {
            xp = x1 ;
            yp = curx-x1+y1 ;
        }
        PLDrawLine(curx,y1,xp,yp) ;
        curx += sp ;
    }
    cury = y1 + domod(-x2-y1,sp) ;
    while (cury <= y2) {
        xp = cury - y2 + x2 ;
        yp = y2 ;
        if (xp < x1) {
            xp = x1 ;
            yp = cury - x1 + x2 ;
        }
        PLDrawLine(x2,cury,xp,yp) ;
        cury += sp ;
    }
}


void HPSelectFillStyle(style)
int style ;
{
    if(fillstyle == style)
      return ;
    fillstyle = style ;
}

void HPFillBox(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    int type ;
    int spacing ;
    int angle ;
    int temp ;
    int oldfillstyle ;

    type = fillstyle & 7 ;
    spacing = ((fillstyle >> 3) & 31) * 20;
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
        HPFillBox(x1,y1,x2,y2) ;
        fillstyle &= ~ANGLEMASK ;
        fillstyle |= (angle+90) % 180 ;
        HPFillBox(x1,y2,x2,y2) ;
        fillstyle = oldfillstyle ;
        return ;
    }
    switch(angle) {
      case 0:
        HPDoHorizontal(x1,y1,x2,y2,spacing) ;
        break ;
      case 45:
        HPDoLeftSlant(x1,y1,x2,y2,spacing) ;
        break ;
      case 90:
        HPDoVertical(x1,y1,x2,y2,spacing) ;
        break ;
      case 135:
        HPDoRightSlant(x1,y1,x2,y2,spacing) ;
        break ;
    }
}


void PLOpenDevice()
{
    char *pipedev, *getenv() ;
	
    if((pipedev = getenv("PLOUTPUT")) == NULL)
      pipedev = "lpr -Ppl" ;
    if ((fp = popen(pipedev,"w")) == NULL) {
        perror(pipedev) ;
        exit(1) ;
    }
    fprintf(fp,"IN;PU;LT;DT%c;\n",ETX) ;
}

void PLCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;PG;\n") ;
    fclose(fp) ;
    fp = NULL ;
}

void PLHardCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;PG;\n") ;
    fclose(fp) ;
    fp = NULL ;
}

void PLDrawLine(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    fprintf(fp,"PU%d,%d;PD%d,%d;\n",x1,y1,x2,y2) ;
}

extern struct pts list_o_pts[] ;

void PLPolygon(numpts)
int numpts ;
{
    int i ;
	
    for(i = 0; (i+1) < numpts ; i++)
      PLDrawLine(list_o_pts[i].x,list_o_pts[i].y,list_o_pts[i+1].x,list_o_pts[i+1].y) ;
}

#define ROTVEC {double t ; t = -nx; nx = ny; ny = t; }
#define NROTVEC {double t ; t = -ny; ny = nx ; nx = t; }
#define MAX(a,b)  ((a>b)?a:b)
#define MIN(a,b)  ((a<b)?a:b)

#define SGN(a)  (a>=0?1:-1)

void PLWire(numpts,width)
int numpts,width ;
{
    int i ;
    double tx,ty,nx,ny,nc, x,y, a1,b1,c1, a2,b2,c2 ,xnxt,ynxt ;
    double floor(), sqrt() ;
    double hw = (((double)width)/2.0) ;

    if(numpts < 2)
      return ;

    tx = list_o_pts[1].x - list_o_pts[0].x ;
    ty = list_o_pts[1].y - list_o_pts[0].y ;
    nc = sqrt(tx*tx + ty*ty) ;
    nx = tx/nc ;
    ny = ty/nc ;
    NROTVEC ;
    x = nx*hw + list_o_pts[0].x ;
    y = ny*hw + list_o_pts[0].y ;
    ROTVEC ;
    a1 = ny ;
    b1 = nx ;
    c1 = y*nx - x*ny ;
    fprintf(fp,"PU%d,%d;",(int)floor(x + .5), (int)floor(y+.5)) ;
    NROTVEC ;
    tx = nx ;
    ty = ny ;
    xnxt = tx*hw + list_o_pts[1].x ;
    ynxt = ty*hw + list_o_pts[1].y ;
	
    for(i = 1; (i+1) < numpts ; i++) {
        double det_a,xn2,yn2 ;
	
        tx = list_o_pts[i+1].x - list_o_pts[i].x ;
        ty = list_o_pts[i+1].y - list_o_pts[i].y ;
        nc = sqrt(tx*tx + ty*ty) ;
        nx = tx/nc ;
        ny = ty/nc ;
        NROTVEC ;
        x = nx*hw + list_o_pts[i].x ;
        y = ny*hw + list_o_pts[i].y ;
        ROTVEC ;
        a2 = ny ;
        b2 = nx ;
        c2 = y*nx - x*ny ;
        NROTVEC ;
        tx = nx ;
        ty = ny ;
        xn2 = tx*hw + list_o_pts[i+1].x ;
        yn2 = ty*hw + list_o_pts[i+1].y ;
        det_a = a1*b2 - a2*b1 ;
        if(det_a != 0.0) {
            double mx,mn ;
		
            tx = (c2*b1 - c1*b2)/det_a ;
            ty = (-a2*c1 + a1*c2)/det_a ;
            mx = MAX(x,xn2) ;
            mn = MIN(x,xn2) ;
            if(tx > mx || tx < mn)
              det_a = 0.0 ;
            mx = MAX(y,yn2) ;
            mn = MIN(y,yn2) ;
            if(ty > mx || ty < mn)
              det_a = 0.0 ;
        }
        if(det_a == 0.0) {
            double ux,uy,vx,vy,udotv,nuv,an, acos() ;
			
            ux = list_o_pts[i].x - xnxt ;
            uy = list_o_pts[i].y - ynxt ;
            vx = list_o_pts[i].x - x ;
            vy = list_o_pts[i].y - y ;
            udotv = ux*vx + uy*vy ;
            nuv = sqrt(ux*ux + uy*uy)*sqrt(vx*vx+vy*vy) ;
            an = acos(udotv/nuv) ;
            an = an * 180.0 / 3.1415927 ;
            /*			if(an < 0)
				an = -an ;
                                an *= SGN(x-xnxt)*SGN(y-ynxt) ;*/
            fprintf(fp,"PD%d,%d;",(int)floor(xnxt + .5),(int)floor(ynxt + .5)) ;
            fprintf(fp,"AA%d,%d,%d;",list_o_pts[i].x,list_o_pts[i].y,(int)floor(an+.5)) ;
        } else
          fprintf(fp,"PD%d,%d;",(int)floor(tx + .5),(int)floor(ty + .5)) ;
        xnxt = xn2 ;
        ynxt = yn2 ;
        a1 = a2 ;
        b1 = b2 ;
        c1 = c2 ;
    }
    fprintf(fp,"PD%d,%d;",(int)floor(xnxt + .5),(int)floor(ynxt + .5)) ;
    fprintf(fp,"AA%d,%d,%d;\n",list_o_pts[numpts-1].x,list_o_pts[numpts-1].y,
            180*SGN(xnxt - list_o_pts[numpts-1].x)*SGN(ynxt - list_o_pts[numpts-1].y)) ;
	
    tx = list_o_pts[numpts-2].x - list_o_pts[numpts-1].x ;
    ty = list_o_pts[numpts-2].y - list_o_pts[numpts-1].y ;
    nc = sqrt(tx*tx + ty*ty) ;
    nx = tx/nc ;
    ny = ty/nc ;
    NROTVEC ;
    x = nx*hw + list_o_pts[numpts-1].x ;
    y = ny*hw + list_o_pts[numpts-1].y ;
    ROTVEC ;
    a1 = ny ;
    b1 = nx ;
    c1 = y*nx - x*ny ;
    NROTVEC ;
    tx = nx ;
    ty = ny ;
    xnxt = tx*hw + list_o_pts[numpts-2].x ;
    ynxt = ty*hw + list_o_pts[numpts-2].y ;
	
    for(i = numpts-2; i>0 ; i--) {
        double det_a,xn2,yn2 ;
	
        tx = list_o_pts[i-1].x - list_o_pts[i].x ;
        ty = list_o_pts[i-1].y - list_o_pts[i].y ;
        nc = sqrt(tx*tx + ty*ty) ;
        nx = tx/nc ;
        ny = ty/nc ;
        NROTVEC ;
        x = nx*hw + list_o_pts[i].x ;
        y = ny*hw + list_o_pts[i].y ;
        ROTVEC ;
        a2 = ny ;
        b2 = nx ;
        c2 = y*nx - x*ny ;
        NROTVEC ;
        tx = nx ;
        ty = ny ;
        xn2 = tx*hw + list_o_pts[i-1].x ;
        yn2 = ty*hw + list_o_pts[i-1].y ;
        det_a = a1*b2 - a2*b1 ;
        if(det_a != 0.0) {
            double mx,mn ;
		
            tx = (c2*b1 - c1*b2)/det_a ;
            ty = (-a2*c1 + a1*c2)/det_a ;
            mx = MAX(x,xn2) ;
            mn = MIN(x,xn2) ;
            if(tx > mx || tx < mn)
              det_a = 0.0 ;
            mx = MAX(y,yn2) ;
            mn = MIN(y,yn2) ;
            if(ty > mx || ty < mn)
              det_a = 0.0 ;
        }
        if(det_a == 0.0) {
            double ux,uy,vx,vy,udotv,nuv,an, acos() ;
			
            ux = list_o_pts[i].x - xnxt ;
            uy = list_o_pts[i].y - ynxt ;
            vx = list_o_pts[i].x - x ;
            vy = list_o_pts[i].y - y ;
            udotv = ux*vx + uy*vy ;
            nuv = sqrt(ux*ux + uy*uy)*sqrt(vx*vx+vy*vy) ;
            an = acos(udotv/nuv) ;
            an = an * 180.0 / 3.1415927 ;
            /*			if(an < 0)
				an = -an ;
                                an *= SGN(x-xnxt)*SGN(y-ynxt) ;*/
            fprintf(fp,"PD%d,%d;",(int)floor(xnxt + .5),(int)floor(ynxt + .5)) ;
            fprintf(fp,"AA%d,%d,-%d;",list_o_pts[i].x,list_o_pts[i].y,(int)floor(an+.5)) ;
        } else
          fprintf(fp,"PD%d,%d;",(int)floor(tx + .5),(int)floor(ty + .5)) ;
        xnxt = xn2 ;
        ynxt = yn2 ;
        a1 = a2 ;
        b1 = b2 ;
        c1 = c2 ;
    }
    fprintf(fp,"PD%d,%d;",(int)floor(xnxt + .5),(int)floor(ynxt + .5)) ;
    fprintf(fp,"AA%d,%d,%d;\n",list_o_pts[0].x,list_o_pts[0].y,
            180*SGN(xnxt - list_o_pts[0].x)*SGN(ynxt - list_o_pts[0].y)) ;
}


void PLBox(length,width,cx,cy,vx,vy)
int length, width, cx,cy,vx,vy ;
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
    PLDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*width + px ;
    py = ny*width + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PLDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*length + px ;
    py = ny*length + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PLDrawLine(x1,y1,x2,y2) ;
    x1 = x2;
    y1 = y2;
    ROTVEC ;
    px = nx*width + px ;
    py = ny*width + py ;
    x2 = (int) floor(px + .5) ;
    y2 = (int) floor(py + .5) ;
    PLDrawLine(x1,y1,x2,y2) ;
}

void PLRoundFlash(diam,cx,cy)
int diam,cx,cy ;
{
    fprintf(fp,"PU%d,%d;",cx,cy) ;
    fprintf(fp,"CI%d;",diam/2) ;
}

void PLSelectFillStyle(style)
int style ;
{
    static int laststyle = -255 ;
    if(laststyle == style)
      return ;
    laststyle = style ;
    fprintf(fp,"FT %d,%d,%d;\n",laststyle&7,((laststyle/8)&31)*20,((laststyle/256)&7)*45) ;
}

void PLSelectLineStyle(style)
int style ;
{
    static int laststyle = -255 ;
    if(laststyle == style)
      return ;
    laststyle = style ;
    if(laststyle == -1)
      fprintf(fp,"LT;") ;
    else if(!(laststyle&0xf8))
      fprintf(fp,"LT%d,4;",laststyle&7) ;
    else
      fprintf(fp,"LT%d,%d;",laststyle&7,(laststyle>3)) ;
}

void PLFillBox(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    fprintf(fp,"PU%d,%d;RA%d,%d;\n",x1,y1,x2,y2) ;
}

void PLSelectPen(PenNumber)
int PenNumber ;
{
    static int lastpen = -255 ;
	
    if(lastpen == PenNumber)
      return ;
    lastpen = PenNumber ;
    if(PenNumber >= 0)
      fprintf(fp,"SP%d;\n",PenNumber) ;
}

void PLDrawLabel(text,x,y,pos,size,rot)
char *text ;
int x,y,pos,size,rot ;
{
    double sw,sh ;
	
    if(rot)
      fprintf(fp,"DR0,-1;") ;
    else
      fprintf(fp,"DR;") ;
    sh = .0243 * size ;
    sw = .7 * sh ;
    fprintf(fp,"SI%f,%f;",sw,sh) ;
    fprintf(fp,"PU%d,%d;",x,y) ;
    switch(pos) {
      case 0:
        fprintf(fp,"CP%d,0;",-(strlen(text)/2)) ;
        break ;
      case 1:
        fprintf(fp,"CP%d,1;",-(strlen(text)/2)) ;
        break ;
      case 2:
        fprintf(fp,"CP1,1;") ;
        break ;
      case 3:
        fprintf(fp,"CP1,0;") ;
        break ;
      case 4:
        fprintf(fp,"CP1,-1;") ;
        break ;
      case 5:
        fprintf(fp,"CP%d,-1;",-(strlen(text)/2)) ;
        break ;
      case 6:
        fprintf(fp,"CP%d,-1;",-(strlen(text)+1)) ;
        break ;
      case 7:
        fprintf(fp,"CP%d,0;",-(strlen(text)+1)) ;
        break ;
      case 8:
        fprintf(fp,"CP%d,1;",-(strlen(text)+1)) ;
        break ;
      default :
        break ;
    }
    fprintf(fp,"LB%s%c",text,ETX) ;
}

void PLDrawCross(x,y) 
int x,y ;
{
    fprintf(fp,"PU%d,%d;PD%d,%d;",x+40,y,x-40,y) ;
    fprintf(fp,"PU%d,%d;PD%d,%d;PU;",x,y+40,x,y-40) ;
}

void PLInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"PL",STRINGSIZE) ;
    p->OpenDevice = PLOpenDevice ;
    p->CloseDevice = PLCloseDevice ;
    p->HardCloseDevice = PLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = PLSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = PLFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 10000 ;
    p->xRight  = 250 ;
    p->yTop    = 7796 ;
    p->yBottom = 596 ;
}

void PLBInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"PL-B",STRINGSIZE) ;
    p->OpenDevice = PLOpenDevice ;
    p->CloseDevice = PLCloseDevice ;
    p->HardCloseDevice = PLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = PLSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = PLFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 10668 ;
    p->xRight  = 508 ;
    p->yTop    = 16764 ;
    p->yBottom = 508 ;

}

void PLCInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"PL-C",STRINGSIZE) ;
    p->OpenDevice = PLOpenDevice ;
    p->CloseDevice = PLCloseDevice ;
    p->HardCloseDevice = PLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = PLSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = PLFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 16764 ;
    p->xRight  = 508 ;
    p->yTop    = 21844 ;
    p->yBottom = 508 ;
}

void PLDInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"PL-D",STRINGSIZE) ;
    p->OpenDevice = PLOpenDevice ;
    p->CloseDevice = PLCloseDevice ;
    p->HardCloseDevice = PLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = PLSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = PLFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 21844 ;
    p->xRight  = 508 ;
    p->yTop    = 34036 ;
    p->yBottom = 508 ;
}

void HPGLOpenDevice()
{
    char *outfile ;
    char *z ;
    extern char *currentfilename ;
	
    outfile = (char *) malloc(strlen(currentfilename)+6) ;
    strcpy(outfile,currentfilename) ;
    z = strrchr(outfile,'.') ;
    if(z == NULL)
      z = outfile+strlen(outfile) ;
    strcpy(z,".hpgl") ;
    if ((fp = fopen(outfile,"w")) == NULL) {
        perror("Flea!:") ;
        exit(1) ;
    }
    free(outfile) ;
    fprintf(fp,"IN;PU;LT;DT%c;\n",ETX) ;
}

void HPGLCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;PG;\n") ;
    fclose(fp) ;
    fp = NULL ;
}

void HPGLHardCloseDevice()
{
    if(fp == NULL)
      return ;
    PLSelectPen(-255) ;
    fprintf(fp,"PU;SP;PG;\n") ;
    fclose(fp) ;
    fp = NULL ;
}



void HPGLInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"HPGL",STRINGSIZE) ;
    p->OpenDevice = HPGLOpenDevice ;
    p->CloseDevice = HPGLCloseDevice ;
    p->HardCloseDevice = HPGLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = HPSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = HPFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 10000 ;
    p->xRight  = 250 ;
    p->yTop    = 7796 ;
    p->yBottom = 596 ;
}

void HPGLBInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"HPGL-B",STRINGSIZE) ;
    p->OpenDevice = HPGLOpenDevice ;
    p->CloseDevice = HPGLCloseDevice ;
    p->HardCloseDevice = HPGLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = HPSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = HPFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 10668 ;
    p->xRight  = 508 ;
    p->yTop    = 16764 ;
    p->yBottom = 508 ;
}


void HPGLCInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"HPGL-C",STRINGSIZE) ;
    p->OpenDevice = HPGLOpenDevice ;
    p->CloseDevice = HPGLCloseDevice ;
    p->HardCloseDevice = HPGLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = HPSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = HPFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 16764 ;
    p->xRight  = 508 ;
    p->yTop    = 21844 ;
    p->yBottom = 508 ;
}

void HPGLDInit()
{
    struct Device *p ;
	
    p = BuildDevice() ;
    strncpy(p->DeviceName,"HPGL-D",STRINGSIZE) ;
    p->OpenDevice = HPGLOpenDevice ;
    p->CloseDevice = HPGLCloseDevice ;
    p->HardCloseDevice = HPGLHardCloseDevice ;
    p->DrawLine = PLDrawLine ;
    p->DrawCross = PLDrawCross ;
    p->SelectPen = PLSelectPen ;
    p->SelectFillStyle = HPSelectFillStyle ;
    p->SelectLineStyle = PLSelectLineStyle ;
    p->FillBox = HPFillBox ;
    p->DrawLabel = PLDrawLabel ;
    p->Polygon = PLPolygon ;
    p->Box     = PLBox ;
    p->Wire    = PLWire ;
    p->RoundFlash = PLRoundFlash ;
    p->plotter = YEA ;
    p->xAspect = 1016 ;
    p->yAspect = 1016 ;
    p->xLeft   = 21844 ;
    p->xRight  = 508 ;
    p->yTop    = 34036 ;
    p->yBottom = 508 ;
}

