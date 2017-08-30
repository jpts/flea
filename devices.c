/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"
#include <ctype.h>

#define TL(CHAR) ((isupper(CHAR))?(tolower(CHAR)):(CHAR))

extern struct Device *DeviceList ;
extern struct Device *CurrentDevice ;
extern char *CurrentDeviceName ;

int cisncmp(s1,s2,n)
char *s1,*s2 ;
int n ;
{
    while(n>0)
      {
          if(*s1=='\0')
            return((*s2=='\0')?NA:YEA) ;
          if(TL(*s1)!=TL(*s2))
            return(YEA) ;
          s1++ ;
          s2++ ;
          n-- ;
      }
    return(NA) ;
}

void NullFunction()
{}

void Unsupported()
{
    fprintf(stderr,"The current device driver can not handle all cif constructs\n") ;
    fprintf(stderr,"Currently the hpgl devices are the only devices handling\n") ;
    fprintf(stderr,"wires,polygons,roundflashes, and non-manhattan boxes\n") ;
}

struct Device *BuildDevice()
{
    struct Device *p ;
    
    if((p=(struct Device *)malloc(sizeof(struct Device))) == NULL)
      error("Out of memory!\n") ;
    p->next = DeviceList ;
    DeviceList = p ;
    CurrentDevice = p ;
    p->DeviceName[0] = '\0' ;
    p->yTop = 0 ;
    p->yBottom = 0 ;
    p->xLeft = 0 ;
    p->xRight = 0 ;
    p->OpenDevice = NullFunction ;
    p->SelectPen = Unsupported ;
    p->SelectFillStyle = Unsupported ;
    p->SelectLineStyle = Unsupported ;
    p->DrawLine = Unsupported ;
    p->DrawLabel = Unsupported ;
    p->DrawCross = Unsupported ;
    p->FillBox = Unsupported ;
    p->CloseDevice = NullFunction ;
    p->HardCloseDevice = NullFunction ;
    p->Polygon = Unsupported ;
    p->Wire = Unsupported ;
    p->RoundFlash = Unsupported ;
    p->Box = Unsupported ;
    p->plotter = NA ;
    return(p) ;
}

void SelectDevice(DeviceName)
char *DeviceName ;
{
    struct Device *p ;
    
    if(*DeviceName == '\0')
      return ;
    if(DeviceList == NULL)
      error("No Devices exist!\n") ;
    for(p=DeviceList;p!=NULL;p=p->next)
      if(!cisncmp(DeviceName,p->DeviceName,STRINGSIZE))
        {
            CurrentDevice = p ;
            CurrentDeviceName = p->DeviceName ;
            return ;
        }
    error("Bad device name!\n") ;
}

void MakeDevices()
{
    PLInit() ;
    PLBInit() ;
    PLCInit() ;
    PLDInit() ;
    HPGLInit() ;
    HPGLBInit() ;
    HPGLCInit() ;
    HPGLDInit() ;
    PSInit() ;
    LWInit() ;
#ifdef HP7580
    HPInit() ;
#endif
#ifdef VERSATEC
    VPInit() ;
#endif
}
