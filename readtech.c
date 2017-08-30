/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"

extern struct LayerList  *Layers ;
extern struct LayerList  *CurrentLayer ;
extern char *PlotLayers ;
extern char *TechLib ;

char *killsp(s)
char *s ;
{
    while(*s == ' ' || *s == '\t')
      s++ ;
    return(s) ;
}

int wordlength(s)
char *s ;
{
    int i = 0 ;
    
    while(*s != ' ' && *s != '\t' && *s != '\0' && *s != '\n')
      s++,i++ ;
    return(i) ;
}

char *wordget(s)
char *s ;
{
    int i ;
    char *t ;
    
    i = wordlength(s) ;
    t = (char *) malloc(i+1) ;
    strncpy(t,s,i) ;
    *(t+i) = '\0' ;
    return(t) ;
}

int ReadTech(techname)
char *techname ;
{
    FILE *fp ;
    char buf[512], *s, *z, *realfilename ;
    char tech_name[512] ;
    int localflag = YEA, fl2 = YEA, i ;
    struct LayerList *p ;
    
    
    if(Layers != NULL)
      return 0;
    
    realfilename = NULL ;
    for(i=0;techname[i]!='\0'&&techname[i]!='-';i++)
      tech_name[i] = techname[i] ;
    tech_name[i] = '\0' ;
    techname = tilde(tech_name) ;
    realfilename = techname ;
    if((fp = fopen(techname,"r")) == NULL) {
        while(localflag) {
            if(TechLib != NULL) {
                char *pa,*v,*new,*strtok(),*EndInSlash() ;
                
                pa = (char *) malloc(strlen(TechLib)+1) ;
                strcpy(pa,TechLib) ;
                v = strtok(pa,":") ;
                while(v != NULL) {
                    char *s ;
                    v = tilde(v) ;
                    s = EndInSlash(v) ;
                    free(v) ;
                    v = s ;
                    new = (char *)malloc(strlen(v)+strlen(realfilename)+1) ;
                    strcpy(new,v) ;
                    strcat(new,realfilename) ;
                    free(v) ;
                    if((fp = fopen(new,"r")) != NULL) {
                        if(!fl2)
                      fprintf(stderr,"ReadTech: Found default techfile.\n") ;
                        free(realfilename) ;
                        realfilename = new ;
                        goto readfile ;
                    }
                    free(new) ;
                    v = strtok(NULL,":") ;
                }
            }
            if(fp == NULL) {
                if(fl2 == YEA) {
                    char *s = (char *) malloc(10) ;
                    fl2 = NA ;
    fprintf(stderr,"ReadTech: Can't find '%s' techfile.\n",techname) ;
    fprintf(stderr,"ReadTech: Trying to find a default techfile.\n") ;
                    strcpy(s,"default") ;
                    realfilename = s ;
                } else {
                    localflag = NA ;
    fprintf(stderr,"ReadTech: Couldn't find a default techfile either.\n") ;
                    return(NA) ;
                }
            }
        }
    }
  readfile:
    free(realfilename) ;
    while(fgets(buf,512,fp)) {
        struct LayerList *p ;
        struct OtherLayersList *o ;
        int length ;
        char *t ;
        
        for(t=buf;*t!='\n';t++) 
          /*NULL STATEMENT*/;
        *t = '\0' ;
        switch(*(s = killsp(buf))) {
          case '#': break ;
          case '>':
            s = killsp(s+1) ;
            length = 0 ;
            for(t=s;*t!=' '&&*t!='\t'&&*t!='\0'&&*t!='\n';length++,t++) 
              /*NULL STATEMENT*/;
            if(!length){
                fprintf(stderr,"ReadTech: No name for layer definition!\n") ;
                break ;
            }
            t = (char *) malloc(length+1) ;
            strncpy(t,s,length) ;
            t[length] = '\0' ;
            for(p=Layers;p!=NULL;p=p->next)
              if(!strcmp(p->LayerName,t))
                break ;
            if(p != NULL) {
                fprintf(stderr,"ReadTech: Can't redefine layers! (layer = %s)\n",t) ;
                free(t) ;
                break ;
            }
            CurrentLayer = (struct LayerList *) malloc(sizeof(struct LayerList)) ;
            if(!CurrentLayer)
              error("Out of memory!\n") ;
            CurrentLayer->LayerName = t ;
            CurrentLayer->PenNumber = -1 ;
            CurrentLayer->LineStyle = -1 ;
            CurrentLayer->FillPen = -1 ;
            CurrentLayer->FillStyle = -1 ;
            CurrentLayer->Turd = NA ;
            CurrentLayer->OtherLayers = NULL ;
            CurrentLayer->next = Layers ;
            Layers = CurrentLayer ;
            break ;
          case '.':
            if(CurrentLayer == NULL) {
                fprintf(stderr,"ReadTech: No Layer name defined.\n") ;
                break ;
            }
            s = killsp(s+1) ;
            if(!strncmp(s,"PenNumber",9)) {
                sscanf(s+9,"%d",&CurrentLayer->PenNumber) ;
                break ;
            }
            if(!strncmp(s,"LineStyle",9)) {
                sscanf(s+9,"%d",&CurrentLayer->LineStyle) ;
                break ;
            }
            if(!strncmp(s,"FillPen",7)) {
                sscanf(s+7,"%d",&CurrentLayer->FillPen) ;
                break ;
            }
            if(!strncmp(s,"FillStyle",9)) {
                sscanf(s+9,"%d",&CurrentLayer->FillStyle) ;
                break ;
            }
            if(!strncmp(s,"LabelSize",9)) {
                sscanf(s+9,"%d",&CurrentLayer->LabelSize) ;
                break ;
            }
            if(!strncmp(s,"LabelPen",8)) {
                sscanf(s+8,"%d",&CurrentLayer->LabelPen) ;
                break ;
            }
            fprintf(stderr,"ReadTech: Did not act on '%s'\n",s) ;
            break ;
          case '&':
            s = killsp(s+1) ;
            length = 0 ;
            for(t=s;*t!=' '&&*t!='\t'&&*t!='\0'&&*t!='\n';length++,t++) 
              /*NULL STATEMENT*/;
            s[length] = '\0' ;
            for(p=Layers;p!=NULL;p=p->next)
              if(!strcmp(s,p->LayerName))
                break ;
            if(p==NULL || p==CurrentLayer) {
                fprintf(stderr,"ReadTech: Bad LayerName on & command.\n") ;
                fprintf(stderr,"ReadTech: %s\n",buf) ;
                break ;
            }
            o = (struct OtherLayersList *) malloc(sizeof(struct OtherLayersList)) ;
            if(!o)
              error("Out of memory!\n") ;
            o->next = CurrentLayer->OtherLayers ;
            CurrentLayer->OtherLayers = o ;
            o->Layer = p ;
            break ;
          default:
            if(!*killsp(s))
              break ;
            fprintf(stderr,"ReadTech: What?\n") ;
            fprintf(stderr,"ReadTech: %s\n",buf) ;
            break ;
        }
    }
    fclose(fp) ;
    if(PlotLayers == NULL)
      return  0 ;
    for(p=Layers;p!=NULL;p=p->next)
      p->Turd = YEA ;
    z = strtok(PlotLayers,", ") ;
    while(z!=NULL) {
        SelectLayer(z) ;
        CurrentLayer->Turd = NA ;
        z = strtok(NULL,",") ;
    }
    return 1 ;
}
