/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "device.h"

#define TYPEMASK 0x07
#define SPACEMASK 0xf8
#define ANGLEMASK 0x300

#define UP   &up
#define DOWN &down

int up = 3 ;
int down = 2 ;

static int fillstyle = -255 ;

int  iarg[2] ;
float rarg[4] ;

#define MIN(a,b) ((a<b)?a:b)


VPOpenDevice()
{
	int ierr,icode,izero ;
	float vmin,rzero,r1,r2,r3,r4 ;
	
	izero = 0 ;
	rzero = 0.0 ;
	iarg[0] = 0 ;
	rarg[0] = 0.0 ;
	icode = 101 ;
	vpopt_(&icode,iarg,rarg,&ierr) ;
	plots_(&izero,&izero,&izero) ;
	vpopt_(&icode,iarg,rarg,&ierr) ;
	icode = -1 ;
	tonflg_(&icode) ;
	r1 = 0.0 ; r2 = 8.5 ; r3 = 11.0 ;
	icode = 1 ;
	newpen_(&icode) ;	
}

VPCloseDevice()
{
	float rzero ;
	int ival ;
	
	rzero = 0.0 ;
	ival = 999 ;
	plot_(&rzero,&rzero,&ival) ;
}

VPHardCloseDevice()
{
	VPCloseDevice() ;
}

VPDrawLabel(text,x,y,pos,size,rot)
char *text ;
int x,y,pos,size,rot ;
{
	float sw,sh ;
	float rx,ry ;
	float angle ;
	int nc ;
	
	rx = ((float)x/10000.0) ;
	ry = ((float)y/10000.0) ;
	
	sh = .01 * size ;
	switch(pos)
	{
		case 0:
			if(!rot)
				rx -= (strlen(text)/2)*sh ;
			else
				ry -= (strlen(text)/2)*sh ;
			break ;
		case 1:
			if(!rot)
			{
				rx -= (strlen(text)/2)*sh ;
				ry += sh*2 ;
			} else {
				ry -= (strlen(text)/2)*sh ;
				rx += sh*2 ;
			}
			break ;
		case 2:
			if(!rot)
			{
				rx += sh*2 ;
				ry += sh*2 ;
			} else {
				ry += sh*2 ;
				rx += sh*2 ;
			}
			break ;
		case 3:
			if(!rot)
				rx += sh*2 ;
			else
				ry += sh*2 ;
			break ;
		case 4:
			if(!rot)
			{
				rx += sh*2 ;
				ry -= sh*2 ;
			} else {
				ry += sh*2 ;
				rx -= sh*2 ;
			}
			break ;
		case 5:
			if(!rot)
			{
				rx -= (strlen(text)/2)*sh ;
				ry -= sh*2 ;
			} else {
				ry -= (strlen(text)/2)*sh ;
				rx -= sh*2 ;
			}
			break ;
		case 6:
			if(!rot)
			{
				rx -= (strlen(text)+2)*sh ;
				ry -= sh*2 ;
			} else {
				ry -= (strlen(text)+2)*sh ;
				rx -= sh*2 ;
			}
			break ;
		case 7:
			if(!rot)
				rx -= (strlen(text)+2)*sh ;
			else
				ry -= (strlen(text)+2)*sh ;
			break ;
		case 8:
			if(!rot)
			{
				rx -= (strlen(text)+2)*sh ;
				ry += sh*2 ;
			} else {
				ry -= (strlen(text)+2)*sh ;
				rx += sh*2 ;
			}
			break ;
		default :
			break ;
	}
	if(rot)
		angle = 90.0 ;
	else
		angle = 0.0 ;
	nc = strlen(text) ;
	{char *s ;
		for (s = text;*s != '\0';s++)
		{
			if(*s >= 'a' && *s <= 'z')
				*s &= ~0x20 ;
		}
	}
	symbol_(&rx,&ry,&sh,text,&angle,&nc,nc) ;
}

VPDrawCross(x,y) 
int x,y ;
{
	float rx,ry ;
	float ru,rd,rl,rr ;
	
	rx = ((float)x/10000.0) ;
	ry = ((float)y/10000.0) ;
	ru = ry + 0.1 ;
	rd = ry - 0.1 ;
	rl = rx + 0.1 ;
	rr = rx - 0.1 ;
	plot_(&rx,&ru,UP) ;
	plot_(&rx,&rd,DOWN) ;
	plot_(&rl,&ry,UP) ;
	plot_(&rr,&ry,DOWN) ;
}


VPDrawLine(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
	float rx1,rx2,ry1,ry2 ;
	
	rx1 = ((float)x1/10000.0) ;
	rx2 = ((float)x2/10000.0) ;
	ry1 = ((float)y1/10000.0) ;
	ry2 = ((float)y2/10000.0) ;
	plot_(&rx1,&ry1,UP) ;
	plot_(&rx2,&ry2,DOWN) ;
}

VPSelectFillStyle(style)
int style ;
{
	if(fillstyle == style)
		return ;
	fillstyle = style ;
	if(style > 0)
		tonclr_(&style) ;
}

VPSelectLineStyle(style)
int style ;
{
	static int laststyle = -255 ;
	if(laststyle == style)
		return ;
	laststyle = style ;
}

VPFillBox(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
	float rx1,rx2,ry1,ry2 ;
	int iflg ;
	
	rx1 = ((float)x1/10000.0) ;
	rx2 = ((float)x2/10000.0) ;
	ry1 = ((float)y1/10000.0) ;
	ry2 = ((float)y2/10000.0) ;
	iflg = 0 ;
	rect_(&rx1,&rx2,&ry1,&ry2,&iflg) ;
}

VPSelectPen(PenNumber)
int PenNumber ;
{
	static int lastpen = -255 ;
	int pen ;
	
	if(lastpen == PenNumber)
		return ;
	lastpen = PenNumber ;
	pen = 1 ;
	penclr_(&pen,&lastpen) ;
}

VPInit()
{
	struct Device *p ;
	
	p = BuildDevice() ;
	strncpy(p->DeviceName,"VP",STRINGSIZE) ;
	p->OpenDevice = VPOpenDevice ;
	p->CloseDevice = VPCloseDevice ;
	p->HardCloseDevice = VPHardCloseDevice ;
	p->DrawLine = VPDrawLine ;
	p->SelectPen = VPSelectPen ;
	p->SelectFillStyle = VPSelectFillStyle ;
	p->SelectLineStyle = VPSelectLineStyle ;
	p->FillBox = VPFillBox ;
	p->DrawLabel = VPDrawLabel ;
	p->DrawCross = VPDrawCross ;
	p->plotter = YEA ;
	p->xAspect = 10000 ;
	p->yAspect = 10000 ;
	p->xLeft =  420000 ;
	p->xRight = 0 ;
	p->yTop =   420000 ;
	p->yBottom = 0 ;
}

