/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"

#define HIGH(x,y) ((x>y)?x:y)
#define LOW(x,y)  ((x>y)?y:x)

extern struct LayerList  *Layers ;
extern struct LayerList  *CurrentLayer ;
extern struct CellList   *Cells ;
extern struct CellList   *CurrentCell ;
extern struct CellLayers *CurrentCellLayer ;
extern struct Device     *CurrentDevice ;
extern int labelrecurse ;
extern int cellrecurse ;
extern int fastmode ;
extern int cellpromptmode ;

void InsertTreeElement(Element,TreeRoot,TreeNumber,Compare)
struct box *Element,**TreeRoot ;
int TreeNumber ;
#ifdef __STDC__
int (*Compare) (struct box *,struct box *) ;
#else
int (*Compare) () ;
#endif
{
    while(*TreeRoot != NULL) {
        TreeRoot = (*Compare)(*TreeRoot,Element)?
          &((*TreeRoot)->Trees[TreeNumber].right):
            &((*TreeRoot)->Trees[TreeNumber].left) ;
    }

    *TreeRoot = Element ;
}

struct box *BuildElement(x1,y1,x2,y2)
int x1,y1,x2,y2 ;
{
    int x ;
    struct box *Element ;
    
    if((Element = (struct box *) malloc(sizeof(struct box))) == NULL)
      error("Out of memory!\n") ;
    for(x=0;x<3;x++) {
        Element->Trees[x].left = NULL ;
        Element->Trees[x].right = NULL ;
    }
    Element->xLow = (x1<x2)?x1:x2 ;
    Element->xHigh = (x1<x2)?x2:x1 ;
    Element->yLow = (y1<y2)?y1:y2 ;
    Element->yHigh = (y1<y2)?y2:y1 ;
    Element->LeftSide = NULL ;
    Element->RightSide = NULL ;
    Element->Top = NULL ;
    Element->Bottom = NULL ;
/*    Element->Area = (Element->xHigh - Element->xLow) *
      (Element->yHigh - Element->yLow) ;*/
    Element->Area = rand() ;
    if(CurrentCell->Bounds == NULL) {
        CurrentCell->Bounds = (struct Bounds *) malloc(sizeof(struct Bounds)) ;
        CurrentCell->Bounds->x1 = Element->xLow ;
        CurrentCell->Bounds->y1 = Element->yLow ;
        CurrentCell->Bounds->x2 = Element->xHigh ;
        CurrentCell->Bounds->y2 = Element->yHigh ;
    } else {
        if(CurrentCell->Bounds->x1 > Element->xLow)
          CurrentCell->Bounds->x1 = Element->xLow ;
        if(CurrentCell->Bounds->y1 > Element->yLow)
          CurrentCell->Bounds->y1 = Element->yLow ;
        if(CurrentCell->Bounds->x2 < Element->xHigh)
          CurrentCell->Bounds->x2 = Element->xHigh ;
        if(CurrentCell->Bounds->y2 < Element->yHigh)
          CurrentCell->Bounds->y2 = Element->yHigh ;
    }
    return(Element) ;
}

int CompareArea(box1,box2)
struct box *box1,*box2 ;
{
    return(box1->Area > box2->Area) ;
/*    double drand48() ;
    return drand48() > 0.5 ;*/
}

int CompareXOrder(box1,box2)
struct box *box1,*box2 ;
{
    return(box1->xLow > box2->xLow) ;
}

int CompareYOrder(box1,box2)
struct box *box1,*box2 ;
{
    return(box1->yLow > box2->yLow) ;
}

void InsertBoxIntoCell(x1,y1,x2,y2)
int x1,x2,y1,y2 ;
{
    struct box *Element ;
    struct OtherLayersList *p ;
    struct CellLayers *v ;
    
    if(CurrentCellLayer == NULL)
      error("No layer selected for box!\n") ;
    if(!CurrentCellLayer->LayerInfo->Turd) {
        Element = BuildElement(x1,y1,x2,y2) ;
        InsertTreeElement(Element,&(CurrentCellLayer->AreaRoot)
                          ,AreaTree,CompareArea) ;
    }
    for(p=CurrentCellLayer->LayerInfo->OtherLayers;p!=NULL;p=p->next) {
        for(v=CurrentCell->Contents;v!=NULL;v=v->next)
          if(v->LayerInfo == p->Layer && !v->LayerInfo->Turd) {
              Element = BuildElement(x1,y1,x2,y2) ;
              InsertTreeElement(Element,&(v->AreaRoot)
                                ,AreaTree,CompareArea) ;
              break ;
          }
        if(v == NULL && !p->Layer->Turd) {
            v = (struct CellLayers *) malloc(sizeof(struct CellLayers)) ;
            if(v==NULL)
              error("Out of Memory!\n") ;
            v->AreaRoot = BuildElement(x1,y1,x2,y2) ;
            v->XOrderedRoot = NULL ;
            v->YOrderedRoot = NULL ;
            v->cifstuff = NULL ;
            v->LayerInfo = p->Layer ;
            v->next = CurrentCell->Contents ;
            CurrentCell->Contents = v ;
        }
    }
}

struct boxstack {
    struct box *elem ;
    struct boxstack *next ;
} ;

static struct boxstack *stack = NULL ;

static struct boxstack *freelist = NULL ;

static void push_box(struct box *elem)
{
    struct boxstack *entry ;
    if(freelist == NULL) {
        int i ;
        freelist = (struct boxstack *)malloc(sizeof(struct boxstack)*1024) ;
        for(i=0;i<1023;i++)
          freelist[i].next = &(freelist[i+1]) ;
        freelist[1023].next = NULL ;
    }

    
    entry = freelist ;
    freelist = freelist->next ;

    entry->elem = elem ;

    entry->next = stack ;
    stack = entry ;
}

static struct box *pop_box()
{
    struct box *bp ;
    struct boxstack *fp ;
    if(stack == NULL)
      return NULL ;
    fp = stack ;
    bp = stack->elem ;
    stack = stack->next ;
    fp->next = freelist ;
    freelist = fp ;
    return bp ;
}

void DescendTree(TreeRoot,TreeNumber,DoThis)
struct box *TreeRoot ;
int TreeNumber ;
#ifdef __STDC__
void (*DoThis)(struct box *) ;
#else
void (*DoThis)() ;
#endif
{
    
#ifdef OLDWAY
    if(TreeRoot == NULL)
      return;
    DescendTree(TreeRoot->Trees[TreeNumber].left,TreeNumber,DoThis) ;
    (*DoThis)(TreeRoot) ;
    DescendTree(TreeRoot->Trees[TreeNumber].right,TreeNumber,DoThis) ;
#else
    static struct box sentinel ;
    push_box(&sentinel) ;

    while(TreeRoot) {
        push_box(TreeRoot) ;
        TreeRoot = TreeRoot->Trees[TreeNumber].left ;
    }

    while((TreeRoot = pop_box()) != &sentinel) {
        (*DoThis)(TreeRoot) ;
        TreeRoot = TreeRoot->Trees[TreeNumber].right ;
        while(TreeRoot) {
            push_box(TreeRoot) ;
            TreeRoot = TreeRoot->Trees[TreeNumber].left ;
        } 
    }

#endif
}

void CreateOtherTrees(TreeRoot)
struct box *TreeRoot ;
{
    InsertTreeElement(TreeRoot,&(CurrentCellLayer->XOrderedRoot),XOrderTree,CompareXOrder) ;
    InsertTreeElement(TreeRoot,&(CurrentCellLayer->YOrderedRoot),YOrderTree,CompareYOrder) ;
}

static struct box *CurrentBox ;

struct BoxList *MakeBoxListElement(box)
struct box *box ;
{
    struct BoxList *Element ;
    
    if((Element = (struct BoxList *)malloc(sizeof(struct BoxList))) == NULL)
      error("Out of memory!\n") ;
    Element->box = box ;
    return(Element) ;
}

void YAddToBoxList(Box)
struct box *Box ;
{
    struct BoxList *Element ;
    
    if(CurrentBox == Box)
      return ;
    if(CurrentBox->xLow == Box->xHigh)
      if((Box->yHigh>CurrentBox->yLow && CurrentBox->yHigh>Box->yLow)) {
          Element = MakeBoxListElement(Box) ;
          Element->next = CurrentBox->LeftSide ;
          CurrentBox->LeftSide = Element ;
      }
    if(CurrentBox->xHigh == Box->xLow)
      if((Box->yHigh>CurrentBox->yLow && CurrentBox->yHigh>Box->yLow)) {
          Element = MakeBoxListElement(Box) ;
          Element->next = CurrentBox->RightSide ;
          CurrentBox->RightSide = Element ;
      }
}

void XAddToBoxList(Box)
struct box *Box ;
{
    struct BoxList *Element ;
    
    if(CurrentBox == Box)
      return ;
    if(CurrentBox->yLow == Box->yHigh)
      if((Box->xHigh>CurrentBox->xLow && CurrentBox->xHigh>Box->xLow)) {
          Element = MakeBoxListElement(Box) ;
          Element->next = CurrentBox->Bottom ;
          CurrentBox->Bottom = Element ;
      }
    if(CurrentBox->yHigh == Box->yLow)
      if((Box->xHigh>CurrentBox->xLow && CurrentBox->xHigh>Box->xLow)) {
          Element = MakeBoxListElement(Box) ;
          Element->next = CurrentBox->Top ;
          CurrentBox->Top = Element ;
      }
}

void LinkBoxSides(Box)
struct box *Box ;
{
    CurrentBox = Box ;
    DescendTree(CurrentCellLayer->XOrderedRoot,XOrderTree,XAddToBoxList) ;
    DescendTree(CurrentCellLayer->YOrderedRoot,YOrderTree,YAddToBoxList) ;
}

void ProcessCellLayers()
{
    SetCellModes(CurrentCell,labelrecurse,cellrecurse,0) ;
    if(cellpromptmode)
      PromptCellModes(CurrentCell,CurrentCell->filename) ;
}

void CreateCell(CellName)
char *CellName ;
{
    char *p ;
    struct CellList *c ;
    
    if((p = (char *) malloc(strlen(CellName)+1)) == NULL)
      error("Out of memory!\n") ;
    strcpy(p,CellName) ;
    if((c = (struct CellList *)malloc(sizeof(struct CellList))) == NULL)
      error("Out of memory!\n") ;
    c->filename = p ;
    c->Contents = NULL ;
    c->CellCalls = NULL ;
    c->Labels = NULL ;
    c->Bounds = NULL ;
    c->next = Cells ;
    Cells = c ;
}

int FindCell(CellName)
char *CellName ;
{
    struct CellList *p ;
    
    if (Cells == NULL)
      return(NA) ;
    for(p=Cells;p!=NULL;p=p->next)
      if(!strcmp(p->filename,CellName)) {
          CurrentCell = p ;
          return(YEA) ;
      }
    return(NA) ;
}

void SelectLayer(LayerName)
char *LayerName ;
{
    struct LayerList *p ;
    
    for(p=Layers;p!=NULL;p=p->next)
      if(!strcmp(p->LayerName,LayerName))
        break ;
    if(p == NULL)
      if(CurrentLayer == NULL) {
          fprintf(stderr,"SelectLayer: Layer '%s' undefined!\n",LayerName) ;
          fprintf(stderr,"             No previous layer to default to!\n") ;
          exit(-1) ;
      } else {
          fprintf(stderr,"SelectLayer: Layer '%s' undefined!\n",LayerName) ;
          fprintf(stderr,"             Defaulting to '%s'\n",CurrentLayer->LayerName) ;
          return ;
      }
    CurrentLayer = p ;
}

struct CellLayers *SelectLayerInCurrentCell(LayerName)
char *LayerName ;
{
    struct CellLayers *p ;
    
    if(CurrentCell == NULL)
      error("Cell not selected!\n") ;
    SelectLayer(LayerName) ;
    for(p=CurrentCell->Contents;p!=NULL;p=p->next)
      if(p->LayerInfo == CurrentLayer)
        break ;
    if(p == NULL) {
        if((p = (struct CellLayers *) malloc(sizeof(struct CellLayers))) == NULL)
          error("Out of memory!\n") ;
        p->LayerInfo = CurrentLayer ;
        p->AreaRoot = NULL ;
        p->XOrderedRoot = NULL ;
        p->YOrderedRoot = NULL ;
        p->cifstuff = NULL ;
        p->next = CurrentCell->Contents ;
        CurrentCell->Contents = p ;
    }
    CurrentCellLayer = p ;
    return(p) ;
}


void CalculateCellBounds(Cell)
struct CellList *Cell ;
{
    struct Cells *p ;
    int dx,dy,x1,y1,x2,y2,xb1,yb1,xb2,yb2 ;
    
    if(Cell == NULL)
      return ;
    for(p=Cell->CellCalls;p!=NULL;p=p->next)
      CalculateCellBounds(p->CellData) ;
    for(p=Cell->CellCalls;p!=NULL;p=p->next) {
        if(p->CellData == NULL)
          continue ;
        if(p->CellData->Bounds == NULL)
          continue ;
        xb1 = p->CellData->Bounds->x1 ;
        yb1 = p->CellData->Bounds->y1 ;
        xb2 = p->CellData->Bounds->x2 ;
        yb2 = p->CellData->Bounds->y2 ;
        x1 = TransformX(&p->Transform,xb1,yb1) ;
        y1 = TransformY(&p->Transform,xb1,yb1) ;
        x2 = TransformX(&p->Transform,xb2,yb2) ;
        y2 = TransformY(&p->Transform,xb2,yb2) ;
        xb1 = LOW(x1,x2) ;
        xb2 = HIGH(x1,x2) ;
        yb1 = LOW(y1,y2) ;
        yb2 = HIGH(y1,y2) ;
        x1 = xb1 ;
        x2 = xb2 ;
        y1 = yb1 ;
        y2 = yb2 ;
        dx = abs(p->Array.xlo-p->Array.xhi) ;
        if(dx != 0)
          if(0 < p->Array.xsep)
            x2 += dx * p->Array.xsep ;
          else
            x1 += dx * p->Array.xsep ;
        dy = abs(p->Array.ylo-p->Array.yhi) ;
        if(dy != 0)
          if(0 < p->Array.ysep)
            y2 += dy * p->Array.ysep ;
          else
            y1 += dy * p->Array.ysep ;
        if(Cell->Bounds == NULL) {
            Cell->Bounds = (struct Bounds *) malloc(sizeof(struct Bounds)) ;
            Cell->Bounds->x1 = x1 ;
            Cell->Bounds->y1 = y1 ;
            Cell->Bounds->x2 = x2 ;
            Cell->Bounds->y2 = y2 ;
        } else {
            if(Cell->Bounds->x1 > x1)
              Cell->Bounds->x1 = x1 ;
            if(Cell->Bounds->y1 > y1)
              Cell->Bounds->y1 = y1 ;
            if(Cell->Bounds->x2 < x2)
              Cell->Bounds->x2 = x2 ;
            if(Cell->Bounds->y2 < y2)
              Cell->Bounds->y2 = y2 ;
        }
    }
}

void FreeBox(box)
struct box *box ;
{
    while(box->Bottom != NULL) {
        struct BoxList *t ;
          
        t = box->Bottom ;
        box->Bottom = t->next ;
        free(t) ;
    }
    while(box->Top != NULL) {
        struct BoxList *t ;
          
        t = box->Top ;
        box->Top = t->next ;
        free(t) ;
    }
    while(box->LeftSide != NULL) {
        struct BoxList *t ;
          
        t = box->LeftSide ;
        box->LeftSide = t->next ;
        free(t) ;
    }
    while(box->RightSide != NULL) {
        struct BoxList *t ;
          
        t = box->RightSide ;
        box->RightSide = t->next ;
        free(t) ;
    }
    free(box) ;
}

void FreeTree(box)
struct box *box ;
{
    static struct box sentinel ;
    push_box(&sentinel) ;
    while(box) {
        push_box(box) ;
        box = box->Trees[0].left ;
    }

    while((box = pop_box()) != &sentinel) {
        struct box *boxr = box->Trees[0].right ;
        FreeBox(box) ;
        box = boxr ;
        while(box) {
            push_box(box) ;
            box = box->Trees[0].left ;
        } 
    }
}

void FreeUp()
{
    while(Cells != NULL) {
        struct CellList *t ;
          
        if(Cells->Bounds != NULL)
          free(Cells->Bounds) ;
        if(Cells->filename != NULL)
          free(Cells->filename) ;
        while(Cells->Contents != NULL) {
            struct CellLayers *t ;
            
            FreeTree(Cells->Contents->AreaRoot) ;
            t = Cells->Contents ;
            Cells->Contents = t->next ;
            free(t) ;
        }
        while(Cells->CellCalls != NULL) {
            struct Cells *t ;
            
            t = Cells->CellCalls ;
            Cells->CellCalls = t->next ;
            free(t) ;
        }
        while(Cells->Labels) {
            struct Label *t ;
            
            t = Cells->Labels ;
            Cells->Labels = t->next ;
            free(t->Layer) ;
            free(t->Text) ;
            free(t) ;
        }
        t = Cells ;
        Cells = t->next ;
        free(t) ;
    }
    CurrentCell = NULL ;
    CurrentCellLayer = NULL ;
}

void SetCellModes(cell,labrec,cellrec,reclev)
struct CellList *cell ;
int labrec,cellrec,reclev ;
{
    struct Cells *p ;
    
    if(cell == NULL)
      return ;
    if(reclev > MAXCELLRECURSE) {
        fprintf(stderr,"Cell recurse level too deep!\nCheck input file for recursive cell call\n") ;
        fprintf(stderr,"Offending Cell name is '%s'.\n",cell->filename) ;
        exit(1) ;
    }
    labrec-- ;
    cellrec-- ;
    for(p=cell->CellCalls;p!=NULL;p=p->next) {
        if(p->CellData == NULL)
          p->Mode = 0 ;
        else if(cellrec == 0)
          p->Mode = BOUNDBOX | CELLNAME ;
        else if(cellrec > 0)
          p->Mode = RECURSE | DRAW ;
        else
          p->Mode = 0 ;
        if(labrec>0)
          p->Mode |= LABELS ;
        SetCellModes(p->CellData,labrec,cellrec,(reclev+1)) ;
    }
}

int askmode(s,mode,modebit)
char *s ;
int mode,modebit ;
{
    char buf[512],*killsp() ;
    
    printf("%s (Default = %s): ",s,(mode&modebit)?"Yes":"No") ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*killsp(buf) == '\0')
      return(mode) ;
    return((mode&~modebit)|(findbool(killsp(buf))?modebit:0)) ;
}

void PromptCellModes(cell,pastnames)
struct CellList *cell ;
char *pastnames ;
{
    struct Cells *p ;
    
    for(p=cell->CellCalls;p!=NULL;p=p->next) {
        char *namelist ;
          
        if(p->CellData == NULL)
          continue ;
        namelist = (char *) malloc(strlen(pastnames)+strlen(p->Use_ID)+3) ;
        strcpy(namelist,pastnames) ;
        if(*pastnames != '\0')
          strcat(namelist,"->") ;
        strcat(namelist,p->Use_ID) ;
        printf("CELL: %s\n",namelist) ;
        p->Mode = askmode("Draw Labels?",p->Mode,LABELS) ;
        p->Mode = askmode("Draw Bounding Box?",p->Mode,BOUNDBOX) ;
        if(p->Mode&BOUNDBOX) {
            p->Mode = askmode("Draw Cellname?",p->Mode,CELLNAME) ;
            p->Mode &= ~DRAW ;
        } else {
            p->Mode = askmode("Draw Cell Contents?",p->Mode,DRAW) ;
            p->Mode &= ~CELLNAME ;
        }
        p->Mode = askmode("Recurse to next level?",p->Mode,RECURSE) ;
        if(p->Mode&RECURSE)
          PromptCellModes(p->CellData,namelist) ;
        else
          SetCellModes(p->CellData,-1,-1,0) ;
        free(namelist) ;
    }
}

void ClearCellLink()
{
    struct CellList *p ;
    
    for(p=Cells;p!=NULL;p=p->next)
      p->Linked = NA ;
}


