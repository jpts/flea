/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include "flea.h"
#include <sys/file.h>



struct LayerList  *Layers           = NULL ;
struct CellList   *Cells            = NULL ;
struct CellList   *CurrentCell      = NULL ;
struct CellLayers *CurrentCellLayer = NULL ;
struct LayerList  *CurrentLayer     = NULL ;
struct Device     *DeviceList       = NULL ;
struct Device     *CurrentDevice    = NULL ;
struct line       *ListOfLines      = NULL ;
struct text       *ListOfText       = NULL ;

int labelrot = NA;
int cif = NA ;
int fullpage = NA ;
int cellrecurse = 1 ;
int labelrecurse = 2 ;
int interactive = NA ;
int numfiles = 0 ;
int iscale = 0 ;
int fastmode = NA ;
int cellpromptmode = NA ;

float topmargin = 0 ;
float bottommargin = 0 ;
float leftmargin = 0 ;
float rightmargin = 0 ;

char *PlotLayers = NULL ;
char *Paths = NULL ;
char *TechLib = TECHFILES ;
char *usefile = NULL ;
char *CurrentDeviceName = "NULL" ;
char *currentfilename = NULL ;
char *option = "" ;
char *options[10] = {"","","","","","","","","",""} ;

FILE *commfile ;

struct Command commlist[] = 
{
  {"exit",     Fleaexit},
  {"set",      set},
  {"device",   SetDevice},
  {"source",   source},
  {"techfile", ReadTech},
  {"test",     test},
  {"echo",     Echo},
  {"write",    Write},
  {"interact", interact},
  {"line",     SaveLine},
  {"text",     SaveText},
  {NULL,       Fleaexit}
} ;

struct Variable varlist[] =
{
  {"cif",              BOOLEAN,    {&cif},              NA},
  {"fullpage",         BOOLEAN,    {&fullpage},         NA},
  {"fastmode",         BOOLEAN,    {&fastmode},         NA},
  {"promptcells",      BOOLEAN,    {&cellpromptmode},   NA}, 
  {"scale",            INTEGER,    {&iscale},           NA},
  {"cellrecurse",      INTEGER,    {&cellrecurse},      NA},
  {"labelrecurse",     INTEGER,    {&labelrecurse},     NA},
  {"interactive",      BOOLEAN,    {&interactive},      NA},
  {"plotlayers",       STRING,     {&PlotLayers},       NA},
  {"path",             STRING,     {&Paths},            NA},
  {"techlib",          STRING,     {&TechLib},          NA},
  {"option",           STRING,     {&option},           NA},
  {"0",                STRING,     {&options[0]},       NA},
  {"1",                STRING,     {&options[1]},       NA},
  {"2",                STRING,     {&options[2]},       NA},
  {"3",                STRING,     {&options[3]},       NA},
  {"4",                STRING,     {&options[4]},       NA},
  {"5",                STRING,     {&options[5]},       NA},
  {"6",                STRING,     {&options[6]},       NA},
  {"7",                STRING,     {&options[7]},       NA},
  {"8",                STRING,     {&options[8]},       NA},
  {"9",                STRING,     {&options[9]},       NA},
  {"device",           STRING,     {&CurrentDeviceName},NA},
  {"right-margin",     FLOAT,      {&rightmargin},      NA},
  {"left-margin",      FLOAT,      {&leftmargin},       NA},
  {"top-margin",       FLOAT,      {&topmargin},        NA},
  {"bottom-margin",    FLOAT,      {&bottommargin},     NA},
  {(char *)NULL, (int) NULL,           {NULL},          NA}
} ;

struct Boolean bools[] =
{
  {"y",      YEA},
  {"yes",    YEA},
  {"yea",    YEA},
  {"ya",     YEA},
  {"sure",   YEA},
  {"n",      NA},
  {"no",     NA},
  {"nope",   NA},
  {"never",  NA},
  {"always", YEA},
  {"na",     NA},
  {"true",   YEA},
  {"false",  NA},
  {"t",      YEA},
  {"f",      NA},
  {"ok",     YEA},
  {"null",   NA},
  {"on",     YEA},
  {"off",    NA},
  {"yep",    YEA},
  {"0",      NA},
  {"1",      YEA},
  {NULL,     NA}
} ;

int SaveLine(text)
char *text ;
{
    struct line *t ;
    char *parsed ;
    
    t = (struct line *) malloc(sizeof(struct line)) ;
    text = killsp(text) ;
    switch(*text) {
      case '>':
      case '<':
      case '.':
        t->x1ref = *text++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0;
      default:
        t->x1ref = '.' ;
        break ;
    }
    text = killsp(text) ;
    parsed = strtok(text,",") ;
    sscanf(parsed,"%f",&t->x1) ;
    parsed = strtok(NULL,",") ;
    switch(*parsed) {
      case '\\':
      case '/':
      case '.':
        t->y1ref = *parsed++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0;
      default:
        t->y1ref = '.' ;
        break ;
    }
    parsed = killsp(parsed) ;
    sscanf(parsed,"%f",&t->y1) ;
    parsed = strtok(NULL,",") ;
    parsed = killsp(parsed) ;
    switch(*parsed) {
      case '<':
      case '>':
      case '.':
        t->x2ref = *parsed++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0;
      default:
        t->x2ref = '.' ;
        break ;
    }
    parsed = killsp(parsed) ;
    sscanf(parsed,"%f",&t->x2) ;
    parsed = strtok(NULL,"") ;
    parsed = killsp(parsed) ;
    switch(*parsed) {
      case '\\':
      case '/':
      case '.':
        t->y2ref = *parsed++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0 ;
      default:
        t->y2ref = '.' ;
        break ;
    }
    parsed = killsp(parsed) ;
    sscanf(parsed,"%f",&t->y2) ;
    t->next = ListOfLines ;
    ListOfLines = t ;
    return 0 ;
}

int Fleaexit()
{
    while(getc(commfile)!=EOF)
      /* NULL STATEMENT */;
    return 0 ;
}

int SaveText(text)
char *text ;
{
    int flag ;
    struct text *t ;
    char *parsed ;
    
    t = (struct text *) malloc(sizeof(struct text)) ;
    text = killsp(text) ;
    t->rotate = NA ;
    if(!strncmp(text,"rotate",6)) {
        t->rotate = YEA ;
        text += 6 ;
        text = killsp(text) ;
        if(*text == ',')
          text++ ;
        text = killsp(text) ;
    }
    parsed = strtok(text," ,\t") ;
    flag = NA ;
    if(!strncmp(parsed,"center",6))
      t->pos = 0 ;
    else if(!strncmp(parsed,"right",5))
      t->pos = 7 ;
    else if(!strncmp(parsed,"left",4))
      t->pos = 3 ;
    else
      flag = YEA ;
    if(!flag)
      text = strtok(NULL,"") ;
    parsed = strtok(text,", \t") ;
    t->size = atoi(parsed) ;
    text = strtok(NULL,"") ;
    switch(*text) {
      case '>':
      case '<':
      case '.':
        t->xref = *text++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0;
      default:
        t->xref = '.' ;
        break ;
    }
    text = killsp(text) ;
    parsed = strtok(text,",") ;
    sscanf(parsed,"%f",&t->x) ;
    parsed = strtok(NULL,",") ;
    switch(*parsed) {
      case '\\':
      case '/':
      case '.':
        t->yref = *parsed++ ;
        break ;
      case '\0':
        printf("Flearc: bad line statement.\n") ;
        return 0;
      default:
        t->yref = '.' ;
        break ;
    }
    parsed = killsp(parsed) ;
    sscanf(parsed,"%f",&t->y) ;
    parsed = strtok(NULL,"") ;
    t->text = (char *) malloc(strlen(parsed)+1) ;
    strcpy(t->text,parsed) ;
    t->next = ListOfText ;
    ListOfText = t ;
    return 0 ;
}


int findbool(boolname)
char *boolname ;
{
    int i ;
    char *s ;
    
    for(s=boolname;*s != '\0';s++)
      *s |= 0x60 ;
    for(i=0;bools[i].BoolName != NULL;i++)
      if(!strcmp(bools[i].BoolName,boolname))
        return(bools[i].BoolValue) ;
    fprintf(stderr,"FindBool: bad boolean -> %s.\n",boolname) ;
    return(NA) ;
}

void DoCommand(buf)
char *buf ;
{
    char *parsed ;
    int i ;
    
    buf = killsp(buf) ;
    if(*buf == '#' || *buf == '\0' || *buf == '\n')
      return ;
    parsed = strtok(buf," \n\t") ;
    for(i=0;commlist[i].CommandName != NULL;i++) {
        if(!strcmp(parsed,commlist[i].CommandName)) {
            (*commlist[i].CommandFunction)(strtok(NULL,"\n")) ;
            break ;
        }
    }
    if(commlist[i].CommandName == NULL)
      fprintf(stderr,"Unknown command '%s'\n",parsed) ;
}

int source(filename)
char *filename ;
{
    if(!ReadCommandFile(filename))
      fprintf(stderr,"Can't open '%s'\n",filename) ;
    return 0 ;
}

int ReadCommandFile(filename)
char *filename ;
{
    FILE *savefile ;
    char buf[4096] ;
    
    savefile = commfile ;
    filename = tilde(filename) ;
    if((commfile = fopen(filename,"r")) == NULL) {
        free(filename) ;
        commfile = savefile ;
        return(NA) ;
    }
    free(filename) ;
    while(fgets(buf,4096,commfile) != NULL)
      DoCommand(buf) ;
    fclose(commfile) ;
    commfile = savefile ;
    return(YEA) ;
}

int findvar(varname)
char *varname ;
{
    int i ;
    
    for(i = 0;varlist[i].VarName != NULL;i++)
      if(!strcmp(varname,varlist[i].VarName))
        return(i) ;
    return(-1) ;
}

struct Variable *getval(valname)
char *valname ;
{
    int i ;
    
    valname = killsp(valname) ;
    if(*valname == '$') 
      if((i = findvar(valname+1)) == -1) {
          printf("flearc: Unknown variable it test statement.\n") ;
          return(NULL) ;
      }
      else
        return(&varlist[i]) ;
    if(*valname == '"') {
        char buf[512],*t,**v ;
        struct Variable *res ;
          
        for(valname++,t=buf;*valname!='"'&&*valname!='\0';valname++)
          *t++ = *valname ;
        *t++ = '\0' ;
        res = (struct Variable *) malloc(sizeof(struct Variable)) ;
        res->Allocated = YEA ;
        t = (char *) malloc(strlen(buf)+1) ;
        strcpy(t,buf) ;
        v = (char **) malloc(sizeof(char *)) ;
        *v = t ;
        if(res != NULL) {
            res->DataPtr.STRINGPtr = v ;
            res->DataType = STRING ;
        }
        return(res) ;
    }
    if(*valname >= '0' && *valname <= '9') {
        int *v ;
        struct Variable *res ;
          
        res = (struct Variable *) malloc(sizeof(struct Variable)) ;
        res->Allocated = YEA ;
        v = (int *) malloc(sizeof(int)) ;
        *v = atoi(valname) ;
        if(res != NULL) {
            res->DataPtr.INTEGERPtr = v ;
            res->DataType = INTEGER ;
        }
        return(res) ;
    } else {
        struct Variable *res = (struct Variable *) malloc(sizeof(struct Variable)) ;
        int *i;
          
        res->Allocated = YEA ;
          
        i = (int *) malloc(sizeof(int)) ;
        *i = findbool(valname) ;
        if(res != NULL) {
            res->DataPtr.BOOLEANPtr = i ;
            res->DataType = BOOLEAN ;
        }
        return(res) ;
    }
}

int test(data)
char *data ;
{
    char buf[512],*t ;
    struct Variable *v1,*v2 ;
    int equel, result ;
    
    result = NA ;
    if(!strcmp(data,"ignore"))
      goto ignore ;
    data = killsp(data) ;
    for(t=buf;*data!=' '&&*data!='\t'&&*data!='<'&&*data!='=';data++)
      if((*t++ = *data) == '\0') {
          printf("flearc: Bad test statement.\n") ;
          return 0;
      }
    *t = '\0' ;
    if((v1 = getval(buf)) == NULL) {
        printf("flearc: Bad variable in test statement.\n") ;
        return 0;
    }
    data = killsp(data) ;
    while(*data=='<'||*data=='>'||*data=='=')
      data++;
    if(*(data-1) == '>')
      equel = NA ;
    else if(*(data-1) == '=')
      equel = YEA ;
    else {
        printf("flearc: Bad comparison operator in test statement.\n") ;
        return 0;
    }
    data = killsp(data) ;
    for(t=buf;*data!=' '&&*data!='\t'&&*data!='<'&&*data!='=';data++)
      if((*t++ = *data) == '\0')
        break ;
    *t = '\0' ;
    if((v2 = getval(buf)) == NULL) {
        printf("flearc: Bad variable in test statement.\n") ;
        return 0;
    }
    switch(v1->DataType) {
      case STRING:
        if(v2->DataType != STRING) {
            printf("Incompatable types in test.\n") ;
            return 0;
        }
        result = !strcmp(*(v1->DataPtr.STRINGPtr),*(v2->DataPtr.STRINGPtr)) ;
        break ;
      case BOOLEAN:
        if(v2->DataType != BOOLEAN) {
            printf("Incompatable types in test.\n") ;
            return 0;
        }
        result = (*(v1->DataPtr.BOOLEANPtr) == *(v2->DataPtr.BOOLEANPtr)) ;
        break ;
      case INTEGER:
        if(v2->DataType != INTEGER) {
            printf("Incompatable types in test.\n") ;
            return 0;
        }
        result = (*(v1->DataPtr.INTEGERPtr) == *(v2->DataPtr.INTEGERPtr)) ;
        break ;
      case FLOAT:
        if(v2->DataType != FLOAT) {
            printf("Incompatable types in test.\n") ;
            return 0;
        }
        result = (*(v1->DataPtr.FLOATPtr) == *(v2->DataPtr.FLOATPtr)) ;
        break ;
    }
    if(v2->Allocated) {
        if(v2->DataType==STRING) {
            char **v = v2->DataPtr.STRINGPtr ;
            char *s = *v ;
            free(s) ;
        }
        free(v2->DataPtr.VOIDPtr) ;
        free(v2) ;
    }
    if(v1->Allocated) {
        if(v1->DataType==STRING) {
            char **v = v1->DataPtr.STRINGPtr ;
            char *s = *v ;
            free(s) ;
        }
        free(v1->DataPtr.VOIDPtr) ;
        free(v1) ;
    }
    if(!equel)
      result = !result ;
  ignore:
    while(fgets(buf,512,commfile) != NULL)
      if(!strncmp(killsp(buf),"test",4) && !result)
        test("ignore") ;
      else if(!strncmp(killsp(buf),"endtest",7))
        return 0;
      else if(result)
        DoCommand(buf) ;
    return 0 ;
}

int set(varname)
char *varname ;
{
    int i ;
    char *parsed ;
    
    parsed = strtok(varname," \t") ;
    for(i=0;varlist[i].VarName != NULL;i++) {
        if(!strcmp(parsed,varlist[i].VarName)) {
            char buf[512] ;
            parsed = strtok(NULL,"") ;
            if(*parsed == '?') {
                gets(buf) ;
                if(*killsp(buf) == '\0')
                  return 0;
                parsed = killsp(buf) ;
            }
            switch(varlist[i].DataType) {
                double dv ;
                char *t ;
                int *ip ;
                      
              case INTEGER:
                ip = varlist[i].DataPtr.INTEGERPtr ;
                sscanf(parsed,"%d",ip) ;
                break ;
                      
              case STRING:
                t = (char *)malloc(strlen(parsed)+1) ;
                strcpy(t,parsed) ;
                *(varlist[i].DataPtr.STRINGPtr) = t ;
                break ;
                      
              case BOOLEAN:
                if(!strncmp(parsed,"toggle",6))
                  *(varlist[i].DataPtr.BOOLEANPtr) = 
                    !*(varlist[i].DataPtr.BOOLEANPtr) ;
                else
                  *(varlist[i].DataPtr.BOOLEANPtr) = findbool(parsed) ;
                break ;
                      
              case FLOAT:
                sscanf(parsed,"%lf",&dv) ;
                *(varlist[i].DataPtr.FLOATPtr) = dv ;
                break ;
            }
            return 0;
        }
    }
    fprintf(stderr,"set variable '%s' not defined.\n",parsed) ;
    return 0 ;
}

int Echo(data)
char *data ;
{
    printf("%s\n",data) ;
    return 0 ;
}

int Write(data)
char *data ;
{
    printf("%s",data) ;
    fflush(stdout) ;
    return 0 ;
}

int SetDevice(devname)
char *devname ;
{
    char buf[512],*parsed ;
    
    parsed = strtok(devname,"\t\0 ") ;
    if(*killsp(parsed) == '?') {
        gets(buf)  ;
        parsed = killsp(buf) ;
    }
    SelectDevice(parsed) ;
    return 0 ;
}

void error(s)
char *s ;
{
    if(CurrentDevice != NULL)
      (*CurrentDevice->HardCloseDevice)() ;
    fprintf(stderr,s) ;
    exit(-1) ;
}

void usage()
{
    fprintf(stderr,"Usage: flea [ -dicfrlLtp ] [ options arguments ] filename.\n") ;
    exit(-1) ;
}

void stuff()
{
    char buf[512] ;
    
    sprintf(buf,"more %s/howto",TECHFILES) ;
    system(buf) ;
    exit (-1) ;
}

void getbuf(s)
char *s ;
{
    if(gets(s) == NULL)
      *s = '\0' ;
}

int interact()
{
    char buf[512], *killsp() ;
    struct Device *d ;
    
    printf("\n\nInteractive mode. Press return to use default values.\n\n") ;
    if(numfiles < 1)
      do {
          fflush(stdin) ;
          printf("Enter filename: ") ;
          fflush(stdout) ;
          fflush(stdin) ;
          getbuf(buf) ;
          if(*buf == '\0')
            printf("No default filename!\n") ;
          else {
              usefile = (char *)malloc(strlen(buf)+1) ;
              strcpy(usefile,buf) ;
          }
      } while(usefile == NULL) ;
    do {
        fflush(stdin) ;
        printf("Device type (Default = %s): ",CurrentDevice->DeviceName) ;
        fflush(stdout) ;
        getbuf(buf) ;
        if(*buf != '\0') {
            for(d=DeviceList;d!=NULL;d=d->next)
              if(!cisncmp(buf,d->DeviceName,STRINGSIZE))
                break ;
            if(d == NULL)
              printf("Bad device name!\n") ;
        } else
          d = CurrentDevice ;
    } while(d == NULL) ;
    SelectDevice(buf) ;
    printf("Use fast mode? (Default = %s): ",fastmode?"Yes":"No") ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*killsp(buf) != '\0')
      fastmode = findbool(killsp(buf)) ;
    printf("Read in cif file? (Default = %s): ",cif?"Yes":"No") ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*killsp(buf) != '\0')
      cif = findbool(killsp(buf)) ;
    if(CurrentDevice->plotter) {
        sprintf(buf,"%d",iscale) ;
        printf("Enter scale in units per inch (Default = %s): ",iscale==0?"none":buf) ;
        fflush(stdout) ;
        fflush(stdin) ;
        getbuf(buf) ;
        if(*killsp(buf) != '\0')
          sscanf(killsp(buf),"%d",&iscale) ;
        if(iscale == 0) {
            printf("Scale to full page? (Default = %s): ",fullpage?"Yes":"No") ;
            fflush(stdout) ;
            fflush(stdin) ;
            getbuf(buf) ;
            if(*killsp(buf) != '\0')
              fullpage = findbool(killsp(buf)) ;
        }
    }
    printf("Enter cell call recurse level (Default = %d): ",cellrecurse) ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*buf != '\0')
      sscanf(buf,"%d",&cellrecurse) ;
    printf("Enter label recurse level (Default = %d): ",labelrecurse) ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*buf != '\0')
      sscanf(buf,"%d",&labelrecurse) ;
    printf("Prompt for cell plotting info? (Default = %s): ",cellpromptmode?"Yes":"No") ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*killsp(buf) != '\0')
      cellpromptmode = findbool(killsp(buf)) ;
    if(!cif) {
        if(Paths != NULL)
          printf("\nPaths = '%s'\n",Paths) ;
        printf("Enter search paths %s: ",Paths==NULL?"(<CR> to ignore)":"(<CR> to default)") ;
        fflush(stdout) ;
        fflush(stdin) ;
        getbuf(buf) ;
        if(*killsp(buf) != '\0') {
            Paths = (char *)malloc(strlen(buf)+1) ;
            strcpy(Paths,buf) ;
        }
    }
    if(PlotLayers != NULL)
      printf("\nPlotLayers = '%s'\n",PlotLayers) ;
    printf("Enter layers to plot seperated by commas %s: ",PlotLayers==NULL?"(<CR> to plot all layers)":"(<CR> to default)") ;
    fflush(stdout) ;
    fflush(stdin) ;
    getbuf(buf) ;
    if(*killsp(buf) != '\0') {
        PlotLayers = (char *)malloc(strlen(buf)+1) ;
        strcpy(PlotLayers,buf) ;
    }
    if(Layers == NULL) {
        printf("Techfile to use (<CR> to ignore): ") ;
        fflush(stdout) ;
        fflush(stdin) ;
        gets(buf) ;
        if(*killsp(buf) != '\0')
          ReadTech(killsp(buf)) ;
    }
    fflush(stdin) ;
    interactive = NA ;
    return 0 ;
}

int doopts(ac,av,firstpass)
int ac ;
char **av ;
int firstpass ;
{
    int object = 0 ;
    
    if(ac != 0 && **av == '-') {
        int x ;
        char c ;
          
        object = 1 ;
        ac-- ;
        for(x=1;(*av)[x] != '\0';x++)
          switch((c = (*av)[x])) {
            case 'F':
              if(firstpass)
                fastmode = !fastmode ;
              break ;
            case 'c':
              if(firstpass)
                cif = !cif ;
              break ;
            case 'd':
              if(ac-- < 1)
                stuff() ;
              SelectDevice(av[object++]) ;
              break ;
            case 's':
              if(ac-- < 1)
                stuff() ;
              sscanf(av[object++],"%d",&iscale) ;
              break ;
            case 'r':
              if(ac-- < 1)
                stuff() ;
              sscanf(av[object++],"%d",&cellrecurse) ;
              break ;
            case 'l':
              if(ac-- < 1)
                stuff() ;
              sscanf(av[object++],"%d",&labelrecurse) ;
              break ;
            case 'f':
              if(firstpass)
                fullpage = !fullpage ;
              break ;
            case 't':
              if(ac-- < 1)
                stuff() ;
              if(firstpass)
                ReadTech(av[object++]) ;
              else
                object++ ;
              break ;
            case 'o':
              if(ac-- < 1)
                stuff() ;
              option = av[object++] ;
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
              if(ac-- < 1)
                stuff() ;
              options[c-'0'] = av[object++] ;
              break ;
            case 'L':
              if(ac-- < 1)
                stuff() ;
              PlotLayers = av[object++] ;
              break ;
            case 'p':
              if(ac-- < 1)
                stuff() ;
              Paths = av[object++] ;
              break ;
            case 'i':
              if(firstpass)
                interactive = !interactive ;
              break ;
            default:
              stuff() ;
          }
        return(object) ;
    }
    return(0) ;
}

void main(ac,av)
int ac ;
char **av ;
{
    int object ;

    MakeDevices() ;
    SelectDevice(DEFAULTDEV) ;
    ac--;
    av++;
    object = doopts(ac,av,YEA) ;
    numfiles = ac - object ;
    if(!ReadCommandFile(".flearc")) {
        char *t,*home ;
          
        if((home = (char *) getenv("HOME")) != NULL) {
            t = (char *)malloc(strlen(home)+10) ;
            strcpy(t,home) ;
            strcat(t,"/.flearc") ;
            if(!ReadCommandFile(t)) {
                char buf[512] ;
                sprintf(buf,"%s/flearc",TECHFILES) ;
                ReadCommandFile(buf) ;
            }
            free (t) ;
        }
    }
    object = doopts(ac,av,NA) ;
    ac -= object ;
    av += object ;
    if(interactive)
      interact() ;
    if((ac < 1) && (usefile == NULL))
      usage() ;
    if(usefile != NULL) {


        if(!cif) 
          ReadMagicFile(usefile,
                        (cellrecurse>labelrecurse)?cellrecurse:labelrecurse) ;
        else
          ReadCifFile(usefile) ;
        
        if(!cif)
          FindCell(usefile) ;
        else {
            FindCell("!design") ;
            CalculateCellBounds(CurrentCell) ;
            CurrentCell = CurrentCell->CellCalls->CellData ;
        }
        ProcessCellLayers() ;
        CalculateCellBounds(CurrentCell) ;
        DrawLayout() ;
    } else
      while(ac > 0) {
          if(!cif)
            ReadMagicFile(*av,(cellrecurse>labelrecurse)?cellrecurse:labelrecurse) ;
          else
            ReadCifFile(*av) ;
          if(!cif)
            FindCell(*av) ;
          else {
              FindCell("!design") ;
              if(CurrentCell->CellCalls != NULL)
                CurrentCell = CurrentCell->CellCalls->CellData ;
          }
          ProcessCellLayers() ;
          CalculateCellBounds(CurrentCell) ;
          DrawLayout() ;
          FreeUp() ;
          if(currentfilename)
            free(currentfilename) ;
          currentfilename = NULL ;
          ac-- ;
          av++ ;
      }
    exit(0) ;
}

