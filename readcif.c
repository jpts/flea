 /*
  *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
  *   Written By  Edward A Luke for
  *   Mississippi State University
  *   Last Modified  Oct 1989
  *
  *   Modified Dec 5, 1989         Edward A. Luke, Mississippi State University
  *   Cifreader choked on a / seperator in the DS command.  I did a kludge fix
  *   for this one case, but a more complete reader interface is needed, one
  *   that treats the seperators, and blanks properly, this version of the
  *   reader explicitly expects seperators to be blanks, and sometimes ','
  *   characters.  This needs to be corrected, But hey, it works for most
  *   cases, and I don't have time right now to fix it right.
  *
  *   Modified Dec 8, 1989         Edward A. Luke, Mississippi State University
  *   Fixed cifreader to work as defined in  *Introductions to VLSI Systems* by
  *   Mead & Conway.  Kludge of Dec 5, eliminated.
  *
  *   Modified April 4, 1991       Edward A. Luke, Mississippi State University
  *   Fixed a bug that occured in defining bounding boxes for cif cells that
  *   only contained cell calls.
  */

#include "flea.h"
#include <ctype.h>
/*
 * Non-manhattan cif reader.  Tries to convert all manhattan declarations
 * into boxes which flea uses better,  any non-manhattan construct is
 * read into seperate cif data structures
 */

extern struct LayerList  *Layers ;
extern struct LayerList  *CurrentLayer ;
extern struct CellList   *Cells ;
extern struct CellList   *CurrentCell ;
extern struct CellLayers *CurrentCellLayer ;
extern struct Device     *CurrentDevice ;
extern char              *currentfilename ;


/*
 * Cif reader modes.
 */

#define LOOKCELL 0
#define DEFNCELL 1
#define LOOKNAME 2

static int linenumber ;

void CifError(text)
char *text ;
{
    fprintf(stderr,"Cif Error in line %d: %s\n",linenumber,text) ;
}

int upperChar(ch)
int ch ;
{
    if(ch >= 'A' && ch <='Z')
      return 1 ;
    return 0 ;
}

int digit(ch)
int ch ;
{
    if(ch >= '0' && ch <= '9')
      return 1 ;
    return 0 ;
}

int blank(ch)
int ch ;
{
    if(ch == '\0')
      return 0 ;
    if(!digit(ch) && !upperChar(ch) && ch != '-' && ch != '(' && ch != ')' && ch !=';')
      return 1 ;
    return 0 ;
}

char *
  skipblank(s)
char *s ;
{
    while(*s != '\0') {
        if(!blank(*s))
          return s ;
        s++ ;
    }
    return s ;
}

char *skipint(s)
char *s ;
{
    while(*s == '-') s++ ;
    while(*s != '\0') {
        if(!digit(*s))
          return s ;
        s++ ;
    }
    return s ;
}

char *skipsep(s)
char *s ;
{
    while(*s != '\0') {
        if(!upperChar(*s) && !blank(*s))
          return s ;
        s++ ;
    }
    return s ;
}

#define NORMAL      0
#define KILLSP      1 
#define RETURN      2
#define KILLBLANK   3
#define KILLCOMMENT 4

/*
 * This routine Fills a buffer with one cif command, kills blanks,
 * and converts all whitespaces to ' '.
 */

int CifGetBuf(fp,size,s)
FILE *fp ;
int size ;
char *s ;
{
    int cgbm = KILLBLANK;
    int bufcnt = 1 ;
    int c ;
    
    while((c = getc(fp)) != EOF) {
        if(c == '\n')
          linenumber++ ;
        switch(cgbm) {
            case RETURN:
              return NA ;
            case KILLCOMMENT:
              if(c != ')')
                break ;
              cgbm = NORMAL ;
            case KILLBLANK:
              if(blank(c))
                break ;
              cgbm = NORMAL ;
            case KILLSP:
              if((c == ' ') || (c == '\t') || (c == '\n'))
                break ;
              cgbm = NORMAL ;
            case NORMAL:
              if((c == ' ') || (c == '\t') || (c == '\n')) {
                  cgbm = KILLSP ;
                  c = ' ' ;
              }
              if(c == ';') {
                  cgbm = RETURN ;
                  c = '\0' ;
              }
              bufcnt++ ;
              if(bufcnt == size) {
                  CifError("Buffer overflow.") ;
                  c = '\0' ;
                  cgbm = RETURN ;
              }
              *s++ = c ;
              if(bufcnt == 2 && c == '(') {
                  cgbm = KILLCOMMENT ;
                  break ;
              }
              if(bufcnt == 2 && c == 'E') {
                  *s = '\0' ;
                  return YEA ;
              }
              if(cgbm == RETURN)
                return YEA ;
              break ;
          }
    }
    return NA ;
}

void ReadCifFile(filename)
char *filename ;
{
#define MAX_LINE_SIZE (65535)
    char *realfilename ;
    char *killsp() ;
    char *wordget() ;
    FILE *fp ;
    int mode ;
    char buf[MAX_LINE_SIZE] ;
    int flag ;
    
    linenumber = 1 ;
    if(Layers == NULL) {
        char v[32] ;
        
        strcpy(v,"cif_") ;
        strcat(v,CurrentDevice->DeviceName) ;
        printf("Reading techfile '%s'\n",v) ;
        ReadTech(v) ;
    }
    filename = tilde(filename) ;
    realfilename = (char *) malloc(strlen(filename)+5) ;
    strcpy(realfilename,filename) ;
    free(filename) ;
    if((fp = fopen(realfilename,"r")) == NULL) {	
        strcat(realfilename,".cif") ;
        if((fp = fopen(realfilename,"r")) == NULL) {
            fprintf(stderr,"Can't open '%s'.",realfilename) ;
            error("\n") ;
        }
    }
    currentfilename = realfilename ;
    printf("\n\nCIF READER v1.2\n") ;
    printf("READING CIF FILE '%s'\n",realfilename) ;
    mode = LOOKCELL ;
    flag = NA ;
    while(flag || CifGetBuf(fp,sizeof(buf),buf)) {
        char *t ;
        int l,w,cx,cy,x1,y1,x2,y2,symbolnum,a,b ;
        
        if(buf[0] == '(')
          continue ;
        if(buf[0] == 'E')
          break ;
        switch(mode) {
            case LOOKCELL:
              if(!((buf[0]=='D')&&(*skipblank(buf+1)=='S'))) {
                  flag = YEA ;
                  mode = DEFNCELL ;
                  CreateCell("!design") ;
                  a = 1 ;
                  b = 1 ;
                  Cells->SymbolNumber = -1 ;
                  CurrentCell = Cells ;
                  break ;					
              }
              t = skipsep(skipblank(buf+1)+1) ;
              sscanf(t,"%d",&symbolnum) ;
              t = skipsep(skipint(t)) ;
              mode = LOOKNAME ;
              if( *t == '\0') {
                  a = 1 ;
                  b = 1 ;
                  break ;
              }
              sscanf(t,"%d",&a) ;
              t = skipsep(skipint(t)) ;
              sscanf(t,"%d",&b) ;
              break ;
              
            case LOOKNAME :
              if(buf[0] == '9') {
                  t = killsp(buf+1) ;
                  t = wordget(t) ;
                  printf("Defining '%s'\n",t) ;
              } else {
                  t = (char *) malloc(32) ;
                  sprintf(t,"%d",symbolnum) ;
              }
              CreateCell(t) ;
              free(t) ;
              Cells->SymbolNumber = symbolnum ;
              CurrentCell = Cells ;
              mode = DEFNCELL ;
              if(buf[0] == '9')
                break ;
              
            case DEFNCELL :
              flag = NA ;
              if((buf[0] == 'D') && (*skipblank(buf+1) == 'F')) {
                  mode = LOOKCELL ;
                  break ;
              }
              if(buf[0] == 'L') {
                  t = skipblank(buf+1) ;
                  SelectLayerInCurrentCell(t) ;
                  break ;
              }
              if(buf[0] == 'R') {
                  int diam,cx,cy ;
                  int numints ;
                  struct cifobjs *cp, *CreateCifElement() ;
                  
                  t = skipsep(buf+1) ;
                  numints = 0 ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&diam) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&cx) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&cy) ;
                  if(numints != 3) {
                      CifError("Invalid arguments to cif command\n") ;
                      break ;
                  }
                  cp = CreateCifElement() ;
                  cp->type = 'R' ;
                  cp->width = diam*a/b ;
                  cp->cx = cx*a/b ;
                  cp->cy = cy*a/b ;
                  diam = cp->width/2 ;
                  UpdateBounds(cp->cx+diam,cp->cy+diam) ;
                  UpdateBounds(cp->cx-diam,cp->cy+diam) ;
                  UpdateBounds(cp->cx-diam,cp->cy-diam) ;
                  UpdateBounds(cp->cx+diam,cp->cy-diam) ;
                  InsertCifElementIntoCell(cp) ;
                  break ;
              }
              
              
              if(buf[0] == 'B')	{
                  int rot1,rot2,numints ;
                  
                  t = skipsep(buf+1) ;
                  numints = 0 ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&l) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&w) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&cx) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&cy) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&rot1) ;
                  t = skipsep(skipint(t)) ;
                  if(*t != '\0')
                    numints++ ;
                  sscanf(t,"%d",&rot2) ;
                  if(numints < 4 || numints == 5) {
                      CifError("Invalid arguments to box command") ;
                      break ;
                  }
                  if(numints == 6) {
                      if(rot1 == 0 && rot2 == 0)
                        CifError("Warning: zero direction vector on box\n") ;
                      if(rot1 == 0)
                        rot2 = 1 ;
                      if(rot2 == 0)
                        rot1 = 1 ;
                      if(rot1+rot2 != 1) {
                          char *strtok() ;
                          struct cifobjs *cp, *CreateCifElement() ;
                          int max ;
                          
                          cp = CreateCifElement() ;
                          cp->type = 'B' ;
                          cp->width = w*a/b ;
                          cp->length = l*a/b ;
                          cp->cx = cx*a/b ;
                          cp->cy = cy*a/b ;
                          max = (cp->width > cp->length)?cp->width:cp->length ;
                          max = max/2 ;
                          UpdateBounds(cp->cx+max,cp->cy+max) ;
                          UpdateBounds(cp->cx-max,cp->cy+max) ;
                          UpdateBounds(cp->cx-max,cp->cy-max) ;
                          UpdateBounds(cp->cx+max,cp->cy-max) ;
                          cp->vx = rot1 ;
                          cp->vy = rot2 ;
                          InsertCifElementIntoCell(cp) ;
                          break ;
                      }
                      if(rot2 == 1) {
                          int temp ;
                          
                          temp = w ;
                          w = l ;
                          l = temp ;
                          
                      }
                  }
                  x1 = (cx - l/2)*a/b ;
                  x2 = (cx + l/2)*a/b ;
                  y1 = (cy - w/2)*a/b ;
                  y2 = (cy + w/2)*a/b ;
                  InsertBoxIntoCell(x1,y1,x2,y2) ;
                  break ;
              }
              if(buf[0] == 'P') {
                  int pox,poy,px,py ;
                  char *strtok() ;
                  struct cifobjs *cp, *CreateCifElement() ;
                  
                  t = skipsep(buf+1) ;
                  if(*t == '\0') {
                      CifError("Bad Arguments on Polygon Command.") ;
                      break ;
                  }
                  pox = atoi(t) ;
                  t = skipsep(skipint(t)) ;
                  if(*t == '\0') {
                      CifError("Bad Arguments on Polygon Command.") ;
                      break ;
                  }
                  poy = atoi(t) ;
                  px = pox ;
                  py = poy ;
                  cp = CreateCifElement() ;
                  cp->type = 'P' ;
                  cp->width = 0 ;
                  InsertPoint(cp,px*a/b,py*a/b) ;
                  while(*(t = skipsep(skipint(t))) != '\0') {
                      px = atoi(t) ;
                      if(*(t = skipsep(skipint(t))) == '\0') {
                          CifError("Incomplete point in Polygon Command.") ;
                          break ;
                      }
                      py = atoi(t) ;
                      InsertPoint(cp,px*a/b,py*a/b) ;
                  }
                  if((pox != px) || (poy != py))
                    InsertPoint(cp,pox*a/b,poy*a/b) ;
                  InsertCifElementIntoCell(cp) ;
                  break ;
              }
              if(buf[0] == 'W') {
                  int pox,poy,px,py,width ;
                  char *strtok() ;
                  struct cifobjs *cp, *CreateCifElement() ;
                  
                  t = skipsep(buf+1) ;
                  if(*t == '\0') {
                      CifError("Bad Arguments on Wire Command.") ;
                      break ;
                  }
                  width = atoi(t) ;
                  t = skipsep(skipint(t)) ;
                  if(*t == '\0') {
                      CifError("Bad Arguments on Wire Command.") ;
                      break ;
                  }
                  pox = atoi(t) ;
                  t = skipsep(skipint(t)) ;
                  if(*t == '\0') {
                      CifError("Bad Arguments on Wire Command.") ;
                      break ;
                  }
                  poy = atoi(t) ;
                  px = pox ;
                  py = poy ;
                  cp = CreateCifElement() ;
                  cp->type = 'W' ;
                  cp->width = width*a/b ;
                  InsertPoint(cp,px*a/b,py*a/b) ;
                  while(*(t = skipsep(skipint(t))) != '\0') {
                      px = atoi(t) ;
                      if((t = skipsep(skipint(t))) == '\0') {
                          CifError("Incomplete point in Wire Command.") ;
                          break ;
                      }
                      py = atoi(t) ;
                      InsertPoint(cp,px*a/b,py*a/b) ;
                      UpdateBounds(px*a/b+cp->width,py*a/b+cp->width) ;
                      UpdateBounds(px*a/b-cp->width,py*a/b+cp->width) ;
                      UpdateBounds(px*a/b-cp->width,py*a/b-cp->width) ;
                      UpdateBounds(px*a/b+cp->width,py*a/b-cp->width) ;
                  }
                  InsertCifElementIntoCell(cp) ;
                  break ;
              }
              if(buf[0] == 'C') {
                  struct Transform ident ;
                  struct Transform tell ;
                  struct Transform make ;
                  char *t ;
                  int cellnum ;
                  struct Cells *cellcall ;
                  
                  ident.a = 1 ;
                  ident.b = 0 ;
                  ident.c = 0 ;
                  ident.d = 0 ;
                  ident.e = 1 ;
                  ident.f = 0 ;
                  tell = ident ;
                  make = ident ;
                  t = skipsep(buf+1) ;
                  sscanf(t,"%d",&cellnum) ;
                  t = skipblank(skipint(t)) ;
                  while(*t != '\0') {
                      int x,y ;
                      double c,sqrt(),floor() ;
                      
                      switch(*t) {
                          case 'T' :
                            t = skipsep(t+1) ;
                            if(sscanf(t,"%d",&x) == 0)
                              CifError("Incomplete point on Transform") ;
                            t = skipsep(skipint(t)) ;
                            if(sscanf(t,"%d",&y) == 0) 
                              CifError("Incomplete point on transform") ;
                            t = skipblank(skipint(t)) ;
                            make = ident ;
                            make.c = x*a/b ;
                            make.f = y*a/b ;
                            UpdateTransform(&tell,&make) ;
                            break ;
                            
                          case 'M' :
                            t++ ;
                            t =skipblank(t) ;
                            make = ident ;
                            if(*t++ == 'X')
                              make.a = -1 ;
                            else
                              make.e = -1 ;
                            UpdateTransform(&tell,&make) ;
                            t = skipblank(t) ;
                            break ;
                            
                          case 'R' :
                            t = skipsep(t+1) ;
                            if(sscanf(t,"%d",&x) == 0)
                              CifError("Incomplete Rotation") ;
                            t = skipsep(skipint(t)) ;
                            if(sscanf(t,"%d",&y) == 0)
                              CifError("Incomplete Rotation") ;
                            t = skipblank(skipint(t)) ;
                            make = ident ;
                            c = sqrt((double)(x*x + y*y)) ;
                            make.a = (int)floor((double)x/c + 0.5) ;
                            make.b = -(int)floor((double)y/c + 0.5) ;
                            make.d = (int)floor((double)y/c + 0.5) ;
                            make.e = (int)floor((double)x/c + 0.5) ;
                            UpdateTransform(&tell,&make) ;
                            break ;
                            
                            default :
                              t++ ;
                            t = skipblank(t) ;
                            CifError("Unknown Command on Cell call\n") ;
                            break ;
                        }
                  }
                  cellcall = (struct Cells *)malloc(sizeof(struct Cells)) ;
                  cellcall->filename = "CIFCELL" ;
                  cellcall->Use_ID = "CIF USE_ID" ;
                  clear(&cellcall->Array,sizeof(struct Array)) ;
                  clear(&cellcall->Bounds,sizeof(struct Bounds)) ;
                  cellcall->Transform = tell ;
                  cellcall->CellData = NULL ;
                  cellcall->CallSymbol = cellnum ;
                  cellcall->next = CurrentCell->CellCalls ;
                  CurrentCell->CellCalls = cellcall ;
                  break ;
              }
              if(buf[0] == '9' && buf[1] == '4') {
                  struct Label *label ;
                  
                  t = killsp(buf+2) ;
                  label = (struct Label *)malloc(sizeof(struct Label)) ;
                  label->Text = wordget(t) ;
                  t = killsp(t+wordlength(t)) ;
                  sscanf(t,"%d",&label->xbot) ;
                  t = skipsep(skipint(t)) ;
                  sscanf(t,"%d",&label->ybot) ;
                  label->xtop = label->xbot*a/b ;
                  label->ytop = label->ybot*a/b ;
                  label->xbot = label->xtop ;
                  label->ybot = label->ytop ;
                  label->position = 1 ;
                  t = killsp(t+wordlength(t)) ;
                  label->Layer = wordget(t) ;
                  if(!strcmp(label->Layer,"")) {
                      char *s = (char *) malloc(10) ;
                      free(label->Layer) ;
                      strcpy(s,"labels") ;
                      label->Layer = s ;
                  }
                  label->next = CurrentCell->Labels ;
                  CurrentCell->Labels = label ;
                  break ;
              }
              CifError("Unknown CIF Command.") ;
              
              default :
                break ;
          }
    }
    FindCell("!design") ;
    connectcells(CurrentCell) ;
}


struct Bounds *
  connectcells(Cell)
struct CellList *Cell ;
{
    struct Cells *p ;
    struct CellList *v ;
    struct Label *l ;
    
    if(Cell == NULL)
      return  NULL ;
    if(Cell->Bounds != NULL) {
        int yb ;
        
        yb = (Cell->Bounds->y1 + Cell->Bounds->y2)/2 ;
        for(l=Cell->Labels;l!=NULL;l=l->next)
          if(l->ybot < yb)
            l->position = 5 ;
    }
    for(p=Cell->CellCalls;p!=NULL;p=p->next) {
        for(v=Cells;v!=NULL;v=v->next)
          if(v->SymbolNumber == p->CallSymbol)
            break ;
        if(v == NULL) {
            fprintf(stderr,"v is NULL\n") ;
            continue ;
        }
        p->CellData = v ;
        p->Use_ID = v->filename ;
        p->filename = v->filename ;
    }
#define MIN(a,b)  ((a<b)?a:b)
#define MAX(a,b)  ((a>b)?a:b)
    for(p=Cell->CellCalls;p!=NULL;p=p->next) {
        struct Bounds *bp ;
        int x1,y1,x2,y2 ;
        bp = connectcells(p->CellData) ;
        if(bp==NULL)
          continue ;
        x1 = TransformX(&p->Transform,bp->x1,bp->y1) ;
        y1 = TransformY(&p->Transform,bp->x1,bp->y1) ;
        x2 = TransformX(&p->Transform,bp->x2,bp->y2) ;
        y2 = TransformY(&p->Transform,bp->x2,bp->y2) ;
        if(Cell->Bounds == NULL) {
            Cell->Bounds = (struct Bounds *) malloc(sizeof(struct Bounds)) ;
            Cell->Bounds->x1 = x1 ;
            Cell->Bounds->y1 = y1 ;
            Cell->Bounds->x2 = x2 ;
            Cell->Bounds->y2 = y2 ;
        }
        Cell->Bounds->x1 = MIN(x1,Cell->Bounds->x1) ;
        Cell->Bounds->y1 = MIN(y1,Cell->Bounds->y1) ;
        Cell->Bounds->x2 = MAX(x2,Cell->Bounds->x2) ;
        Cell->Bounds->y2 = MAX(y2,Cell->Bounds->y2) ;
    }
    for(p=Cell->CellCalls;p!=NULL;p=p->next) {
        v = p->CellData ;
        if(v->Bounds) {
            p->Bounds.x1 = v->Bounds->x1 ;
            p->Bounds.y1 = v->Bounds->y1 ;
            p->Bounds.x2 = v->Bounds->x2 ;
            p->Bounds.y2 = v->Bounds->y2 ;
        } else
          fprintf(stderr,"Problem in cell '%s'\n",p->filename) ;
    }
    return Cell->Bounds ;
}

