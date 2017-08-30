/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"

extern struct CellList   *CurrentCell ;
extern struct CellLayers *CurrentCellLayer ;

struct cifobjs *CurrCifElement  ;

struct cifobjs *
  CreateCifElement()
{
    CurrCifElement = (struct cifobjs *) malloc(sizeof(*CurrCifElement)) ;
    CurrCifElement->next = NULL ;
    CurrCifElement->Points = NULL ;
    return CurrCifElement ;
}

struct points *
  CreatePoint(x,y)
int x,y ;
{
    struct points *p ;
    
    p = (struct points *) malloc(sizeof(*p)) ;
    p->next = NULL ;
    p->x = x ;
    p->y = y ;
    return p ;
}

void InsertPoint(cifobj,x,y)
struct cifobjs *cifobj ;
int x,y ;
{
    struct points *p = CreatePoint(x,y) ;
    
    p->next = cifobj->Points ;
    cifobj->Points = p ;
    UpdateBounds(x,y) ;
}

void UpdateBounds(x,y)
int x,y ;
{
    if(CurrentCell->Bounds == NULL)
      {
          CurrentCell->Bounds = (struct Bounds *) malloc(sizeof(struct Bounds)) ;
          CurrentCell->Bounds->x1 = x ;
          CurrentCell->Bounds->y1 = y ;
          CurrentCell->Bounds->x2 = x ;
          CurrentCell->Bounds->y2 = y ;
      }
    else
      {
          if(CurrentCell->Bounds->x1 > x)
            CurrentCell->Bounds->x1 = x ;
          if(CurrentCell->Bounds->y1 > y)
            CurrentCell->Bounds->y1 = y ;
          if(CurrentCell->Bounds->x2 < x)
            CurrentCell->Bounds->x2 = x ;
          if(CurrentCell->Bounds->y2 < y)
            CurrentCell->Bounds->y2 = y ;
      }
}

void InsertCifElementIntoCell(cifobj)
struct cifobjs *cifobj ;
{
    struct OtherLayersList *p ;
    struct CellLayers      *v ;
    
    if(CurrentCellLayer == NULL)
      error("No layer selected for cif element!\n") ;
    if(!CurrentCellLayer->LayerInfo->Turd)
      {
          cifobj->next = CurrentCellLayer->cifstuff ;
          CurrentCellLayer->cifstuff = cifobj ;
      }
    for(p=CurrentCellLayer->LayerInfo->OtherLayers;p!=NULL;p=p->next)
      {
          for(v=CurrentCell->Contents;v!=NULL;v=v->next)
            if(v->LayerInfo == p->Layer && !v->LayerInfo->Turd)
              {
                  cifobj->next = CurrentCellLayer->cifstuff ;
                  CurrentCellLayer->cifstuff = cifobj ;
                  break ;
              }
          if(v == NULL && !p->Layer->Turd)
            {
                v = (struct CellLayers *) malloc(sizeof(struct CellLayers)) ;
                if(v==NULL)
                  error("Out of Memory!\n") ;
                v->cifstuff = cifobj ;
                v->AreaRoot = NULL ;
                v->XOrderedRoot = NULL ;
                v->YOrderedRoot = NULL ;
                v->LayerInfo = p->Layer ;
                v->next = CurrentCell->Contents ;
                CurrentCell->Contents = v ;
            }
      }
}
