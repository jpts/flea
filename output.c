/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"
#include <sys/types.h>
#include <pwd.h>
#include <time.h>

#define SCALEX(x1,y1) ((int) floor((double)TransformX(CurrentTransform,(x1),(y1)) * scalex + .5 + offsetx))
#define	SCALEY(x1,y1) ((int) floor((double)TransformY(CurrentTransform,(x1),(y1)) * scaley + .5 + offsety))
#define SCALES(c)     ((int) floor(((double)c) * scalex +.5 ))
#define SGN(value) ((value>0)?1:-1)

extern struct LayerList  *Layers ;
extern struct LayerList  *CurrentLayer ;
extern struct CellList   *Cells ;
extern struct CellList   *CurrentCell ;
extern struct CellLayers *CurrentCellLayer ;
extern struct Device     *CurrentDevice ;
extern struct line       *ListOfLines ;
extern struct text       *ListOfText ;

extern float topmargin ;
extern float bottommargin ;
extern float leftmargin ;
extern float rightmargin ;
extern int labelrot ;
extern int cellrecurse ;
extern int labelrecurse ;
extern int fullpage ;
extern int fastmode ;
extern int cif ;
extern int iscale ;
extern char *currentfilename ;
extern char *option ;
extern char *options[] ;

static struct Transform *CurrentTransform ;
static double scalex ;
static double scaley ;
static double offsetx ;
static double offsety ;

struct pts list_o_pts[MAXPOINTS] ;

void UpdateTransform(u,t)
struct Transform *u,*t ;
{
    struct Transform vt ;
    
    vt.a = u->a*t->a + u->d*t->b ;
    vt.b = u->b*t->a + u->e*t->b ;
    vt.c = u->c*t->a + u->f*t->b + t->c ;
    vt.d = u->a*t->d + u->d*t->e ;
    vt.e = u->b*t->d + u->e*t->e ;
    vt.f = u->c*t->d + u->f*t->e + t->f ;
    memcpy((char *)u,(char *)&vt,sizeof(struct Transform)) ;
}

int TransformX(u,x,y)
struct Transform *u ;
int x,y ;
{
    return(x*u->a+y*u->b+u->c) ;
}

int TransformY(u,x,y)
struct Transform *u ;
int x,y ;
{
    return(x*u->d+y*u->e+u->f) ;
}

void FindScale(u)
struct Transform *u ;
{
    int   top,
    bottom,
    left,
    right,
    sgnx,sgny,
    cx1,cy1,cx2,cy2,
    cdx,cdy,pdx,pdy ;
    float scale,
    aspect,
    calc_units ;
    
    if(CurrentCell == NULL) {
        fprintf(stderr,"No cell to plot! (bye)\n") ;
        (*CurrentDevice->HardCloseDevice)() ;
        exit(0) ;
    }
    
    if(CurrentCell->Bounds == NULL) {
        fprintf(stderr,"What?  This cell has no bounds!\n") ;
        fprintf(stderr,"I can't handle this, I quit.\n\n") ;
        (*CurrentDevice->HardCloseDevice)() ;
        exit(0) ;
    }
    sgnx = SGN(CurrentDevice->xLeft-CurrentDevice->xRight) ;
    sgny = SGN(CurrentDevice->yTop-CurrentDevice->yBottom) ;
    top = CurrentDevice->yTop-
      sgny*((CurrentDevice->plotter)?
            (int)floor(topmargin*CurrentDevice->yAspect+.5):0) ;
    bottom = CurrentDevice->yBottom+
      sgny*((CurrentDevice->plotter)?
            (int)floor(bottommargin*CurrentDevice->yAspect+.5):0) ;
    left = CurrentDevice->xLeft-
      sgnx*((CurrentDevice->plotter)?
            (int)floor(leftmargin*CurrentDevice->xAspect+.5):0) ;
    right = CurrentDevice->xRight+
      sgnx*((CurrentDevice->plotter)?
            (int)floor(rightmargin*CurrentDevice->xAspect+.5):0) ;
    cx1 = TransformX(u,CurrentCell->Bounds->x1,CurrentCell->Bounds->y1) ;
    cy1 = TransformY(u,CurrentCell->Bounds->x1,CurrentCell->Bounds->y1) ;
    cx2 = TransformX(u,CurrentCell->Bounds->x2,CurrentCell->Bounds->y2) ;
    cy2 = TransformY(u,CurrentCell->Bounds->x2,CurrentCell->Bounds->y2) ;
    cdx = cx2 - cx1 ;
    cdy = cy2 - cy1 ;
    pdx = left - right ;
    pdy = top-bottom ;
    if(SGN(abs(cdx)-abs(cdy))+SGN(abs(pdx)-abs(pdy)) == 0) {
        struct Transform rot ;
        
        rot.a = 0 ;
        rot.b = -1 ;
        rot.c = 0 ;
        rot.d = -1 ;
        rot.e = 0 ;
        rot.f = 0 ;
        UpdateTransform(u,&rot) ;
        cx1 = TransformX(u,CurrentCell->Bounds->x1,CurrentCell->Bounds->y1) ;
        cy1 = TransformY(u,CurrentCell->Bounds->x1,CurrentCell->Bounds->y1) ;
        cx2 = TransformX(u,CurrentCell->Bounds->x2,CurrentCell->Bounds->y2) ;
        cy2 = TransformY(u,CurrentCell->Bounds->x2,CurrentCell->Bounds->y2) ;
        cdx = cx2 - cx1 ;
        cdy = cy2 - cy1 ;
        labelrot = YEA ;
    }
    scale = ((double)pdx)/((double)cdx) ;
    aspect = ((double)CurrentDevice->xAspect)/((double)CurrentDevice->yAspect) ;
    if(fabs(scale*(double)cdy/aspect)>=fabs((double)pdy)) {
        scaley = ((double)pdy)/((double)cdy) ;
        scalex = fabs(scaley*aspect) * ((pdx>0)?1.0:-1.0) ;
    } else {
        scalex = scale ;
        scaley = fabs(scale/aspect) * ((pdy>0)?1.0:-1.0) ;
    }
    calc_units = fabs((double)CurrentDevice->xAspect/scalex) ;
    if(iscale && CurrentDevice->plotter) {
        printf("scale is %d.\n",iscale) ;
        calc_units = (double)iscale ;
        scalex = (double)CurrentDevice->xAspect / calc_units *SGN(scalex);
        scaley = (double)CurrentDevice->yAspect / calc_units *SGN(scaley);
    }
    else if(!fullpage && CurrentDevice->plotter) {	
        int base ;
        
        base = 10 ;
        if(cif)
          base = 1000 ;
        calc_units = ceil(calc_units/base) * base ;
        scalex = (double)CurrentDevice->xAspect / calc_units *SGN(scalex);
        scaley = (double)CurrentDevice->yAspect / calc_units *SGN(scaley);
    } else {
        scalex = (double)CurrentDevice->xAspect / calc_units *SGN(scalex);
        scaley = (double)CurrentDevice->yAspect / calc_units *SGN(scaley);
    }
    
    if(CurrentDevice->plotter) {
        int Left,Right,Top,Bottom ;
        
        Left = CurrentDevice->xLeft ;
        Right = CurrentDevice->xRight ;
        Top = CurrentDevice->yTop ;
        Bottom = CurrentDevice->yBottom ;
          
        SelectLayer("labels") ;
        if(CurrentLayer->PenNumber != -1) {
            struct line *l ;
            struct text *t ;
            
            (*CurrentDevice->SelectPen)(CurrentLayer->PenNumber) ;
            (*CurrentDevice->SelectLineStyle)(-1) ;
            for(l=ListOfLines;l!=NULL;l=l->next) {
                int x1,y1,x2,y2 ;
                
                switch(l->x1ref) {
                  case '<':
                    x1=Left-sgnx*(int)floor(l->x1*CurrentDevice->xAspect+.5) ;
                    break ;
                  case '>':
                    x1=Right+sgnx*(int)floor(l->x1*CurrentDevice->xAspect+.5) ;
                    break ;
                  default:
                    x1=(Left+Right)/2
                      +sgnx*(int)floor(l->x1*CurrentDevice->xAspect+.5) ;
                }
                                                      
                switch(l->y1ref) {
                    
                  case '\\':
                    y1=Top-sgny*(int)floor(l->y1*CurrentDevice->yAspect+.5) ;
                    break ;
                  case '/':
                    y1=Bottom+sgny*(int)floor(l->y1*CurrentDevice->xAspect+.5);
                    break ;
                  default:
                    y1=(Top+Bottom)/2
                      +sgny*(int)floor(l->y1*CurrentDevice->yAspect+.5) ;
                }
                
                switch(l->x2ref) {
                  case '<':
                    x2=Left-sgnx*(int)floor(l->x2*CurrentDevice->xAspect+.5) ;
                    break ;
                  case '>':
                    x2=Right+sgnx*(int)floor(l->x2*CurrentDevice->xAspect+.5) ;
                    break ;
                  default:
                    x2=(Left+Right)/2
                      +sgnx*(int)floor(l->x2*CurrentDevice->xAspect+.5) ;
                }
                              
                switch(l->y2ref) {
                  case '\\':
                    y2=Top-sgny*(int)floor(l->y2*CurrentDevice->yAspect+.5) ;
                    break ;
                  case '/':
                    y2=Bottom+sgny*(int)floor(l->y2*CurrentDevice->xAspect+.5);
                    break ;
                  default:
                    y2=(Top+Bottom)/2
                      +sgny*(int)floor(l->y2*CurrentDevice->yAspect+.5) ;
                }
                (*CurrentDevice->DrawLine)(x2,y2,x1,y1) ;
            }
            for(t=ListOfText;t!=NULL;t=t->next) {
                int x,y ;
                char buf[512] ;
                char *p,*v ;
                
                switch(t->xref) {
                  case '<':
                  case 'L':
                    x=Left-sgnx*(int)floor(t->x*CurrentDevice->xAspect+.5) ;
                    break ;
                  case '>':
                  case 'R':
                    x=Right+sgnx*(int)floor(t->x*CurrentDevice->xAspect+.5) ;
                    break ;
                  default:
                    x=(Left+Right)/2
                      +sgnx*(int)floor(t->x*CurrentDevice->xAspect+.5) ;
                }
                switch(t->yref) {
                  case '\\':
                  case 'T':
                    y=Top-sgny*(int)floor(t->y*CurrentDevice->yAspect+.5) ;
                    break ;
                  case '/':
                  case 'B':
                    y=Bottom+sgny*(int)floor(t->y*CurrentDevice->xAspect+.5) ;
                    break ;
                  default:
                    y=(Top+Bottom)/2
                      +sgny*(int)floor(t->y*CurrentDevice->yAspect+.5) ;
                }
                for(p=t->text,v=buf;p!=NULL && *p!='\0';p++)
                  if(*p == '%') {
                      if(*(p+1) == '\0')
                        continue ;
                      p++ ;
                      switch(*p) {
                          char *c,*z,number[24] ;
                          long ti ;
                          struct passwd *pass ;
                          
                        case 'l':
                          for(c=currentfilename;*c!='\0';*v++=(*c++)) ;
                          break ;
                        case 'f':
                          z = strrchr(currentfilename,'/') ;
                          if(z==NULL)
                            z=currentfilename ;
                          else
                            z=z+1 ;
                          for(c=z;*c!='\0';*v++=(*c++)) ;
                          break ;
                        case 'b':
                          z = strrchr(currentfilename,'/') ;
                          if(z==NULL)
                            z=currentfilename ;
                          else
                            z=z+1 ;
                          for(c=z;(*c!='\0')&&(*c!='.');*v++=(*c++)) ;
                          break ;
                        case 'o':
                          for(c=option;*c!='\0';*v++=(*c++)) ;
                          break ;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                          for(c=options[*p-'0'];*c!='\0';*v++=(*c++)) ;
                          break ;
                        case 't':
                          ti = time(0) ;
                          z = ctime(&ti) ;
                          for(c=z;*c!='\n';*v++=(*c++)) ;
                          break ;
                        case 'd':
                          ti = time(0) ;
                          z = ctime(&ti) ;
                          z = z+4;
                          strcpy(z+7,z+15) ;
                          *(z+6)=',' ;
                          for(c=z;*c!='\n';*v++=(*c++)) ;
                          break ;
                        case 'u':
                          pass = getpwuid(getuid()) ;
                          if(pass == NULL)
                            break ;
                          for(c=pass->pw_name;*c!='\0';*v++=(*c++)) ;
                          break ;
                        case 's':
                          sprintf(number,"%d\n",
                                  (int)floor(cif?((calc_units/100)+.5):(calc_units+.5))) ;
                          for(c=number;*c!='\n';*v++=(*c++)) ;
                          break ;
                        case '@':
                        {
                            char *myfile,*killsp() ;
                            char buf[512] ;
                            FILE *magfile ;
                            
                            myfile = (char *) malloc(strlen(currentfilename)+5) ;
                            strcpy(myfile,currentfilename) ;
                            z = strrchr(myfile,'.') ;
                            if(z == NULL)
                              z = myfile+strlen(myfile) ;
                            strcpy(z,".mag") ;
                            if((magfile = fopen(myfile,"r")) == NULL){
                                fprintf(stderr,"Can't get timestamp.\n") ;
                                break ;
                            }
                            while(fgets(buf,512,magfile) != NULL) {
                                if(!(strncmp(killsp(buf),"timestamp",9))) {
                                    z = killsp(killsp(buf)+9) ;
                                    while((*z>='0')&&(*z<='9'))
                                      *v++ = (*z++) ;
                                    break ;
                                }
                            }
                            fclose(magfile) ;
                            free(myfile) ;
                        }
                          break ;
                        default:
                          *v++ = *p ;
                          break ;
                      }
                  } else
                    *v++ = *p ;
                *v = '\0' ;
                (*CurrentDevice->DrawLabel)(buf,x,y,t->pos,t->size,t->rotate) ;
            }
        }
    }
    offsetx = ((double)(left+right))/2.0
      - ((double)(cx1+cx2))/2.0*scalex ;
    offsety = ((double)(top+bottom))/2.0
      - ((double)(cy1+cy2))/2.0*scaley ;
}

void DrawLine(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    int px1,py1,px2,py2 ;
    
    px1 = SCALEX(x1,y1) ;
    py1 = SCALEY(x1,y1) ;
    px2 = SCALEX(x2,y2) ;
    py2 = SCALEY(x2,y2) ;
    (*CurrentDevice->DrawLine)(px1,py1,px2,py2) ;
}

void DrawBox(box)
struct box *box ;
{
    struct BoxList *BoxOnSide ;
    int state, point ;
    
    if(box->LeftSide == NULL)
      DrawLine(box->xLow,box->yLow,box->xLow,box->yHigh) ;
    else {
        BoxOnSide = box->LeftSide ;
        point = box->yLow ;
        state = box->yLow < BoxOnSide->box->yLow ;
        while(YEA) {
            if(state)
              DrawLine(box->xLow,point,box->xLow,BoxOnSide->box->yLow) ;
            if(BoxOnSide->next == NULL) {
                if(BoxOnSide->box->yHigh < box->yHigh)
                  DrawLine(box->xLow,BoxOnSide->box->yHigh,
                           box->xLow,box->yHigh) ;
                break ;
            }
            point = BoxOnSide->box->yHigh ;
            BoxOnSide = BoxOnSide->next ;
            state = (point != BoxOnSide->box->yLow) ;
        }
    }
    if(box->Top == NULL)
      DrawLine(box->xLow,box->yHigh,box->xHigh,box->yHigh) ;
    else {
        BoxOnSide = box->Top ;
        point = box->xLow ;
        state = box->xLow < BoxOnSide->box->xLow ;
        while(YEA) {
            if(state)
              DrawLine(point,box->yHigh,BoxOnSide->box->xLow,box->yHigh) ;
            if(BoxOnSide->next == NULL) {
                if(BoxOnSide->box->xHigh < box->xHigh)
                  DrawLine(BoxOnSide->box->xHigh,box->yHigh,
                           box->xHigh,box->yHigh) ;
                break ;
            }
            point = BoxOnSide->box->xHigh ;
            BoxOnSide = BoxOnSide->next ;
            state = (point != BoxOnSide->box->xLow) ;
        }
    }
    if(box->Bottom == NULL)
      DrawLine(box->xLow,box->yLow,box->xHigh,box->yLow) ;
    else {
        BoxOnSide = box->Bottom ;
        point = box->xLow ;
        state = box->xLow < BoxOnSide->box->xLow ;
        while(YEA) {
            if(state)
              DrawLine(point,box->yLow,BoxOnSide->box->xLow,box->yLow) ;
            if(BoxOnSide->next == NULL) {
                if(BoxOnSide->box->xHigh < box->xHigh)
                  DrawLine(BoxOnSide->box->xHigh,box->yLow,
                           box->xHigh,box->yLow) ;
                break ;
            }
            point = BoxOnSide->box->xHigh ;
            BoxOnSide = BoxOnSide->next ;
            state = (point != BoxOnSide->box->xLow) ;
        }
    }
    if(box->RightSide == NULL)
      DrawLine(box->xHigh,box->yLow,box->xHigh,box->yHigh) ;
    else {
        BoxOnSide = box->RightSide ;
        point = box->yLow ;
        state = box->yLow < BoxOnSide->box->yLow ;
        while(YEA) {
            if(state)
              DrawLine(box->xHigh,point,box->xHigh,BoxOnSide->box->yLow) ;
            if(BoxOnSide->next == NULL) {
                if(BoxOnSide->box->yHigh < box->yHigh)
                  DrawLine(box->xHigh,BoxOnSide->box->yHigh,
                           box->xHigh,box->yHigh) ;
                break ;
            }
            point = BoxOnSide->box->yHigh ;
            BoxOnSide = BoxOnSide->next ;
            state = (point != BoxOnSide->box->yLow) ;
        }
    }
}

void FillBox(box)
struct box *box ;
{
    int px1,py1,px2,py2 ;
    
    px1 = SCALEX(box->xLow,box->yLow) ;
    py1 = SCALEY(box->xLow,box->yLow) ;
    px2 = SCALEX(box->xHigh,box->yHigh) ;
    py2 = SCALEY(box->xHigh,box->yHigh) ;
    if(CurrentCellLayer->LayerInfo->FillStyle == -2) {
        (*CurrentDevice->DrawLine)(px1,py1,px2,py2) ;
        (*CurrentDevice->DrawLine)(px1,py2,px2,py1) ;
        (*CurrentDevice->DrawLine)(px1,py1,px1,py2) ;
        (*CurrentDevice->DrawLine)(px1,py2,px2,py2) ;
        (*CurrentDevice->DrawLine)(px2,py2,px2,py1) ;
        (*CurrentDevice->DrawLine)(px2,py1,px1,py1) ;
        return ;
    }
    if(CurrentCellLayer->LayerInfo->FillStyle == -3) {
        (*CurrentDevice->DrawLine)(px1,py1,px1,py2) ;
        (*CurrentDevice->DrawLine)(px1,py2,px2,py2) ;
        (*CurrentDevice->DrawLine)(px2,py2,px2,py1) ;
        (*CurrentDevice->DrawLine)(px2,py1,px1,py1) ;
        (*CurrentDevice->DrawLine)((px1+px2)/2,py1,px1,(py1+py2)/2) ;
        (*CurrentDevice->DrawLine)((px1+px2)/2,py2,px1,(py1+py2)/2) ;
        (*CurrentDevice->DrawLine)((px1+px2)/2,py1,px2,(py1+py2)/2) ;
        (*CurrentDevice->DrawLine)((px1+px2)/2,py2,px2,(py1+py2)/2) ;
        return ;
    }
    
    (*CurrentDevice->SelectFillStyle)(CurrentCellLayer->LayerInfo->FillStyle) ;
    (*CurrentDevice->FillBox)(px1,py1,px2,py2) ;
}

void DrawLayout()
{
    struct Transform Begin ;
    struct LayerList *p ;
    Begin.a = 1 ;
    Begin.b = 0 ;
    Begin.c = 0 ;
    Begin.d = 0 ;
    Begin.e = 1 ;
    Begin.f = 0 ;
    CurrentTransform = &Begin ;
    if(Cells == NULL)
      return ;
    (*CurrentDevice->OpenDevice)() ;
    FindScale(&Begin) ;
    for(p=Layers;p!=NULL;p=p->next) {
        ClearCellLink() ;
        DrawCell(CurrentCell,&Begin,p,
                 DRAW|RECURSE|((labelrecurse>0)?LABELS:0),NULL) ;
    }
    (*CurrentDevice->CloseDevice)() ;
}

void DrawCell(cell,Transform,layer,Mode,caller)
struct CellList  *cell ;
struct Transform *Transform ;
struct LayerList *layer ;
int               Mode ;
struct Cells     *caller ;
{
    struct Transform tempt ;
    struct Transform mv,ident,temp ;
    struct Label *p ;
    struct Cells *c ;
    
    ident.a = 1 ;
    ident.b = 0 ;
    ident.c = 0 ;
    ident.d = 0 ;
    ident.e = 1 ;
    ident.f = 0 ;
    CurrentTransform = Transform ;
    if(cell == NULL) {
        fprintf(stderr,"DrawCell recurse error: Eeeeeeeeeekkk!!!\n") ;
        return ;
    }
    if(!strcmp(layer->LayerName,"labels") && (Mode&LABELS)) {
        for(p = cell->Labels;p!=NULL;p=p->next) {
            int xt,yt,xb,yb ;
            int x1,y1,x2,y2 ;
                
            xt = p->xtop ;
            yt = p->ytop ;
            xb = p->xbot ;
            yb = p->ybot ;
            x1 = SCALEX(xt,yt) ;
            y1 = SCALEY(xt,yt) ;
            x2 = SCALEX(xb,yb) ;
            y2 = SCALEY(xb,yb) ;
            SelectLayer(p->Layer) ;
            if(CurrentLayer->LabelPen == -1)
              continue ;
            (*CurrentDevice->SelectPen)(CurrentLayer->LabelPen) ;
            if(x1 == x2) {
                if(y1 == y2)
                  (*CurrentDevice->DrawCross)(x1,y1) ;
                else
                  (*CurrentDevice->DrawLine)(x1,y1,x2,y2) ;
            } else {
                if(y1 == y2)
                  (*CurrentDevice->DrawLine)(x1,y1,x2,y2) ;
                else {
                    (*CurrentDevice->DrawLine)(x1,y1,x1,y2) ;
                    (*CurrentDevice->DrawLine)(x1,y2,x2,y2) ;
                    (*CurrentDevice->DrawLine)(x2,y2,x2,y1) ;
                    (*CurrentDevice->DrawLine)(x2,y1,x1,y1) ;
                }
            }
        }
        SelectLayer("labels") ;
        (*CurrentDevice->SelectPen)(CurrentLayer->LabelPen) ;
        (*CurrentDevice->SelectLineStyle)(-1) ;
        for(p = cell->Labels;p!=NULL;p=p->next) {
            int x1,y1,x2,y2 ;
                
            SelectLayer(p->Layer) ;
            x1 = (p->xtop+p->xbot)/2 ;
            y1 = (p->ytop+p->ybot)/2 ;
            x2 = SCALEX(x1,y1) ;
            y2 = SCALEY(x1,y1) ;
            if(CurrentLayer->LabelSize == 0)
              CurrentLayer->LabelSize = 8 ;
            (*CurrentDevice->DrawLabel)(p->Text,x2,y2,p->position,
                                        CurrentLayer->LabelSize,labelrot) ;
        }
    }
    if(!strcmp(layer->LayerName,"labels") && (Mode&BOUNDBOX) && caller) {
        int bx1,bx2,by1,by2 ;
          
        SelectLayer("labels") ;
        bx1=SCALEX(caller->Bounds.x1,caller->Bounds.y1) ;
        by1=SCALEY(caller->Bounds.x1,caller->Bounds.y1) ;
        bx2=SCALEX(caller->Bounds.x2,caller->Bounds.y2) ;
        by2=SCALEY(caller->Bounds.x2,caller->Bounds.y2) ;
        (*CurrentDevice->SelectPen)(CurrentLayer->PenNumber) ;
        (*CurrentDevice->SelectLineStyle)(CurrentLayer->LineStyle) ;
        (*CurrentDevice->DrawLine)(bx1,by1,bx1,by2) ;
        (*CurrentDevice->DrawLine)(bx1,by2,bx2,by2) ;
        (*CurrentDevice->DrawLine)(bx2,by2,bx2,by1) ;
        (*CurrentDevice->DrawLine)(bx2,by1,bx1,by1) ;
    }
    if(!strcmp(layer->LayerName,"labels") && (Mode&CELLNAME) && caller) {
        int bx1,bx2,by1,by2 ;
        int cx,cy,sdx,sdy ;
        int len,aspect,size ;
          
        SelectLayer("labels") ;
        bx1=SCALEX(caller->Bounds.x1,caller->Bounds.y1) ;
        by1=SCALEY(caller->Bounds.x1,caller->Bounds.y1) ;
        bx2=SCALEX(caller->Bounds.x2,caller->Bounds.y2) ;
        by2=SCALEY(caller->Bounds.x2,caller->Bounds.y2) ;
        cx = (bx1+bx2)/2 ;
        cy = (by1+by2)/2 ;
        (*CurrentDevice->SelectPen)(CurrentLayer->LabelPen) ;
        if(CurrentDevice->plotter) {
            len = strlen(caller->Use_ID) ;
            sdx = abs(bx1 - bx2) ;
            sdy = abs(by1 - by2) ;
            if(labelrot)
              sdx = sdy ;
            aspect = (labelrot)?CurrentDevice->yAspect:CurrentDevice->xAspect ;
            size = (int)floor((float)sdx/(float)aspect/(float)len*100) ;
            if(size <=1)
              size = -1 ;
            if(size > 25)
              size = 25 ;
        }
        else
          size=CurrentLayer->LabelSize ;
        if(size > 0)
          (*CurrentDevice->DrawLabel)(caller->Use_ID,cx,cy,0,size,labelrot) ;
    }
    if(Mode&DRAW) {
        for(CurrentCellLayer = cell->Contents;
            (CurrentCellLayer) && (CurrentCellLayer->LayerInfo != layer);
            CurrentCellLayer = CurrentCellLayer->next)
          /* NULL STATEMENT */ ;

        if(CurrentCellLayer) {
            if(layer->FillPen != -1) {
                if(!cell->Linked) {
                    DescendTree(CurrentCellLayer->AreaRoot,AreaTree,
                                CreateOtherTrees) ;
                    cell->Linked = 2 ;
                }
                (*CurrentDevice->SelectFillStyle)(layer->FillStyle) ;
                (*CurrentDevice->SelectPen)(layer->FillPen) ;
                (*CurrentDevice->SelectLineStyle)(-1) ;
                DescendTree(CurrentCellLayer->XOrderedRoot,XOrderTree,
                            FillBox) ;
            }
            if(layer->PenNumber != -1) {
                if(!cell->Linked) {
                    DescendTree(CurrentCellLayer->AreaRoot,AreaTree,
                                CreateOtherTrees) ;
                    cell->Linked = 2 ;
                }
                if(cell->Linked == 2 && !fastmode) {
                    DescendTree(CurrentCellLayer->AreaRoot,AreaTree,
                                LinkBoxSides) ;
                    cell->Linked = -1 ;
                }
                (*CurrentDevice->SelectLineStyle)(layer->LineStyle) ;
                (*CurrentDevice->SelectPen)(layer->PenNumber) ;
                DescendTree(CurrentCellLayer->XOrderedRoot,XOrderTree,
                            DrawBox) ;
            }
            if(CurrentCellLayer->cifstuff &&
               (layer->PenNumber != -1 || layer->FillPen != -1)) {
                int pen = layer->PenNumber ;
                struct cifobjs *cop ;
                      
                if(pen == -1)
                  pen = layer->FillPen ;
                (*CurrentDevice->SelectPen)(pen) ;
                for(cop = CurrentCellLayer->cifstuff; cop != NULL;
                    cop = cop->next) {
                    switch(cop->type) {
                      case 'P':
                      {   int numpts = 0 ;
                          struct points *pp ; 
                                
                          for(pp=cop->Points;pp!=NULL;pp=pp->next) {
                              list_o_pts[numpts].x=SCALEX(pp->x,pp->y) ;
                              list_o_pts[numpts].y=SCALEY(pp->x,pp->y) ;
                              numpts++ ;
                          }
                          (*CurrentDevice->Polygon)(numpts) ;
                      }
                        break ;
                      case 'W':
                      {   int numpts = 0 ;
                          struct points *pp ; 
                                
                          for(pp=cop->Points;pp!=NULL;pp=pp->next) {
                              list_o_pts[numpts].x=SCALEX(pp->x,pp->y) ;
                              list_o_pts[numpts].y=SCALEY(pp->x,pp->y) ;
                              numpts++ ;
                          }
                          (*CurrentDevice->Wire)(numpts,SCALES(cop->width)) ;
                      }
                        break ;
                      case 'B':
                      {   int length,width,cx,cy,vx,vy ;
                          struct Transform *u = CurrentTransform ;
                                
                          length = SCALES(cop->length) ;
                          width  = SCALES(cop->width) ;
                          cx = SCALEX(cop->cx,cop->cy) ;
                          cy = SCALEY(cop->cx,cop->cy) ;
                          vx = cop->vx*u->a + cop->vy*u->b - u->c ;
                          vy = cop->vx*u->d + cop->vy*u->e - u->f ;
                          (*CurrentDevice->Box)(length,width,cx,cy,vx,vy) ;
                      }
                        break ;
                      case 'R':
                      {   int diam,cx,cy ;
                                
                          diam = SCALES(cop->width) ;
                          cx = SCALEX(cop->cx,cop->cy) ;
                          cy = SCALEY(cop->cx,cop->cy) ;
                          (*CurrentDevice->RoundFlash)(diam,cx,cy) ;
                      }
                        break ;
                      default:
                        printf("bad cif object\n") ;
                        break ;
                    }
                }
            }
        }
    }
    if(!(Mode&RECURSE))
      return ;
    for(c=cell->CellCalls;c!=NULL;c=c->next) {
        int xr,yr ;
          
        memcpy((char *)&tempt,(char *)Transform,sizeof(struct Transform)) ;
        mv = c->Transform ;
        UpdateTransform(&mv,&tempt) ;
        tempt = mv ;
        for(xr=c->Array.xlo;xr<=c->Array.xhi;xr++) {
            for(yr=c->Array.ylo,temp = tempt;yr<=c->Array.yhi;yr++) {
                DrawCell(c->CellData,&tempt,layer,c->Mode,c) ;
                mv = ident ;
                mv.f += c->Array.ysep ;
                UpdateTransform(&mv,&tempt) ;
                tempt = mv ;
            }
            mv = ident ;
            tempt = temp ;
            mv.c += c->Array.xsep ;
            UpdateTransform(&mv,&tempt) ;
            tempt = mv ;
        }
    }
}
