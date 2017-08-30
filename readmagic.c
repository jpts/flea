/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 *
 *	11/22/89 - Added "decomment" routine to handle comment lines
 *		in magic files.  D. R. Miller - MITRE Corp.
 */
#include "flea.h"

extern struct LayerList  *Layers ;
extern struct LayerList  *CurrentLayer ;
extern struct CellList   *Cells ;
extern struct CellList   *CurrentCell ;
extern struct CellLayers *CurrentCellLayer ;
extern struct Device     *CurrentDevice ;
extern char              *Paths ;
extern char              *currentfilename ;
extern int               cellpromptmode ;

void clear(start,length)
void *start ;
int length ;
{
    memset(start,0,length) ;
}

char *decomment(s,n,stream)
char *s ;
int n ;
FILE *stream;
{
    char *c ;
    do {
        c = fgets(s,n,stream);
    } while ((c!=NULL) && ( *killsp(s) == '#' )) ;

    return(c);
}

int ReadMagicFile(filename,recurse)
char *filename ;
int recurse ;
{
    char *realfilename ;
    char *killsp() ;
    char *wordget() ;
    char *filename2 ;
    int  reread = NA ;
    FILE *fp ;
    char buf[512] ;
    struct Cells *cellcall = NULL ;
    struct Cells *l ;
    
    if(cellpromptmode)
      recurse  = 1000;
    if(FindCell(filename))
      if(recurse != 0 && CurrentCell->SymbolNumber == 0)
        reread = YEA ;
      else
        return(YEA) ;
    filename2 = tilde(filename) ;
    realfilename = (char *) malloc(strlen(filename2)+5) ;
    strcpy(realfilename,filename2) ;
    strcat(realfilename,".mag") ;
    if((fp = fopen(filename2,"r")) != NULL) {
        free(realfilename) ;
        realfilename = filename2 ;
    }  else {
        free(filename2) ;
        if((fp = fopen(realfilename,"r")) == NULL) {
            if(Paths != NULL) {
                char *pa,*v,*new,*strtok() ;
            
                pa = (char *) malloc(strlen(Paths)+1) ;
                strcpy(pa,Paths) ;
                v = strtok(pa,":") ;
                while(v != NULL) {
                    v = tilde(v) ;
                    v = EndInSlash(v) ;
                    new = (char *)malloc(strlen(v)+strlen(realfilename)+1) ;
                    strcpy(new,v) ;
                    strcat(new,realfilename) ;
                    if((fp = fopen(new,"r")) != NULL) {
                        realfilename = new ;
                        break ;
                    }
                    free(new) ;
                    v = strtok(NULL,":") ;
                }
            }
        }
        if(fp == NULL) {
            fprintf(stderr,"Can't open '%s'\n",realfilename) ;
            return(NA) ;
        }
    }
    if(currentfilename == NULL)
      currentfilename = realfilename ;
    if(reread)
      printf("Re-Reading '%s'.\n",realfilename) ;
    else
      printf("Reading '%s'.\n",realfilename) ;
    if(!reread) {
        CreateCell(filename) ;
        CurrentCell = Cells ;
    }
    CurrentCell->SymbolNumber = recurse ;
    decomment(buf,512,fp) ;
    if(strncmp(buf,"magic",5)) {
        fprintf(stderr,"'%s' is not a magic file!\n",realfilename) ;
        fclose(fp) ;
        return(NA) ;
    }
    while(decomment(buf,512,fp) != NULL) {
        char *t,*v ;
        int x1,y1,x2,y2 ;
        
        if(!strncmp(killsp(buf),"timestamp",9))
          continue ;
        if(!strncmp(killsp(buf),"tech",4)) {
            if(Layers == NULL) {
                char *t,*v ;
                char *wordget() ;

                printf("READING TECHFILE FOR %s",killsp(buf)+4) ;
                t = wordget(killsp(killsp(buf)+4)) ;
                v = (char *)malloc(strlen(t)+18) ;
                strcpy(v,t) ;
                strcat(v,"_") ;
                strcat(v,CurrentDevice->DeviceName) ;
                ReadTech(v) ;
                free(v) ;
                free(t) ;
            }
            continue ;
        }
        if(!strncmp(killsp(buf),"<<",2)) {
            t = killsp(buf)+3 ;
            for(v=t;*v!='\n';v++)
              if(*v == ' ')
                *v = '\0' ;
            if(!strcmp(t,"end"))
              break ;
            SelectLayerInCurrentCell(t) ;
            continue ;
        }
        if(!strncmp(killsp(buf),"rect",4)) {
            t = killsp(buf) + 4 ;
            sscanf(t,"%d %d %d %d\n",&x1,&y1,&x2,&y2) ;
            if(recurse != 0)
              InsertBoxIntoCell(x1,y1,x2,y2) ;
            else
              if(CurrentCell->Bounds == NULL) {
                  CurrentCell->Bounds = (struct Bounds *) malloc(sizeof(struct Bounds)) ;
                  CurrentCell->Bounds->x1 = x1 ;
                  CurrentCell->Bounds->y1 = y1 ;
                  CurrentCell->Bounds->x2 = x2 ;
                  CurrentCell->Bounds->y2 = y2 ;
              } else {
                  if(CurrentCell->Bounds->x1 > x1)
                    CurrentCell->Bounds->x1 = x1 ;
                  if(CurrentCell->Bounds->y1 > y1)
                    CurrentCell->Bounds->y1 = y1 ;
                  if(CurrentCell->Bounds->x2 < x2)
                    CurrentCell->Bounds->x2 = x2 ;
                  if(CurrentCell->Bounds->y2 < y2)
                    CurrentCell->Bounds->y2 = y2 ;
              }
            continue ;
        }
        if(!strncmp(killsp(buf),"use",3)) {
            cellcall = (struct Cells *)malloc(sizeof(struct Cells)) ;
            t = killsp(killsp(buf) + 3) ;
            cellcall->filename = wordget(t) ;
            t = killsp(t + wordlength(t)) ;
            cellcall->Use_ID = wordget(t) ;
            clear(&cellcall->Array,sizeof(struct Array)) ;
            clear(&cellcall->Transform,sizeof(struct Transform)) ;
            clear(&cellcall->Bounds,sizeof(struct Bounds)) ;
            cellcall->CellData = NULL ;
            cellcall->next = CurrentCell->CellCalls ;
            CurrentCell->CellCalls = cellcall ;
            continue ;
        }
        if(cellcall && !strncmp(killsp(buf),"array",5)) {
            t = killsp(buf)+5 ;
            sscanf(t,"%d %d %d %d %d %d",&cellcall->Array.xlo
                   ,&cellcall->Array.xhi,&cellcall->Array.xsep,&cellcall->Array.ylo
                   ,&cellcall->Array.yhi,&cellcall->Array.ysep) ;
            continue ;
        }
        if(cellcall && !strncmp(killsp(buf),"transform",9)) {
            t = killsp(buf)+9 ;
            sscanf(t,"%d %d %d %d %d %d",&cellcall->Transform.a
                   ,&cellcall->Transform.b,&cellcall->Transform.c,&cellcall->Transform.d
                   ,&cellcall->Transform.e,&cellcall->Transform.f) ;
            continue ;
        }
        if(cellcall && !strncmp(killsp(buf),"box",3)) {
            t = killsp(buf)+3 ;
            sscanf(t,"%d %d %d %d",&cellcall->Bounds.x1,&cellcall->Bounds.y1
                   ,&cellcall->Bounds.x2,&cellcall->Bounds.y2) ;
            continue ;
        }
        if(!strncmp(killsp(buf),"rlabel",6)) {
            struct Label *label ;
            int i ;
            
            t = killsp(killsp(buf)+6) ;
            label = (struct Label *)malloc(sizeof(struct Label)) ;
            label->Layer = wordget(t) ;
            t = killsp(t+wordlength(t)) ;
            sscanf(t,"%d %d %d %d %d",&label->xbot,&label->ybot,
                   &label->xtop,&label->ytop,&label->position) ;
            for(i=0;i<5;i++) 
              t = killsp(t+wordlength(t)) ;
            label->Text = wordget(t) ;
            label->next = CurrentCell->Labels ;
            CurrentCell->Labels = label ;
            continue ;
        }
        printf("What?>> %s",buf) ;
    }
    fclose(fp) ;
    if(recurse != 0)
      recurse-- ;
    for(l=CurrentCell->CellCalls;l!=NULL;l=l->next) {
        ReadMagicFile(l->filename,recurse) ;
        if(!FindCell(l->filename))
          continue ;
        l->CellData = CurrentCell ;
    }
    return 0 ;
}
