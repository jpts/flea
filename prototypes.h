#ifdef __STDC__
/* main.c */
void DoCommand(char *) ;
int  Echo(char *) ;
int  Fleaexit() ;
int  ReadCommandFile(char *) ;
int  SaveLine(char *) ;
int  SaveText(char *) ;
int  SetDevice(char *) ;
int  Write(char *) ;
void byebye() ;
void cls() ;
int  doopts(int,char **,int) ;
void error(char *) ;
int  findbool(char *) ;
int  findvar(char *) ;
void getbuf(char *) ;
struct Variable *getval(char *) ;
int  interact() ;
int  set(char *) ;
int  source(char *) ;
void stuff() ;
int  test(char *) ;
void usage() ;

/*buildbox.c*/
struct box *BuildElement(int,int,int,int) ;
void CalculateCellBounds(struct CellList *) ;
void ClearCellLink() ;
int  CompareArea(struct box *,struct box *) ;
int  CompareXOrder(struct box *,struct box *) ;
int  CompareYOrder(struct box *,struct box *) ;
void CreateCell(char *) ;
void CreateOtherTrees(struct box *) ;
void DescendTree(struct box *,int,void (*)(struct box *)) ;
int  FindCell(char *) ;
void FreeTree(struct box *) ;
void FreeUp() ;
void InsertBoxIntoCell(int,int,int,int) ;
void InsertTreeElement(struct box *,struct box **,int,
                       int (*)(struct box *, struct box *)) ;
void LinkBoxSides(struct box *) ;
struct BoxList *MakeBoxListElement(struct box *) ;
void ProcessCellLayers() ;
void PromptCellModes(struct CellList *,char *) ;
void SelectLayer(char *) ;
struct CellLayers *SelectLayerInCurrentCell(char *) ;
void SetCellModes(struct CellList *,int,int,int) ;
void XAddToBoxList(struct box *) ;
void YAddToBoxList(struct box *) ;
int  askmode(char *,int,int) ;
/* output.c */
void DrawBox(struct box *) ;
void DrawCell(struct CellList *,struct Transform *,struct LayerList *,int,
              struct Cells *) ;
void DrawLayout() ;
void DrawLine(int,int,int,int) ;
void FillBox(struct box *) ;
void FindScale(struct Transform *) ;
int  TransformX(struct Transform *,int,int) ;
int  TransformY(struct Transform *,int,int) ;
void UpdateTransform(struct Transform *,struct Transform *) ;

/*readtech.c*/
int   ReadTech(char *) ;
char *killsp(char *) ;
char *wordget(char *) ;
int  wordlength(char *) ;

/*readmagic.c*/
int  ReadMagicFile(char *,int) ;
void clear(void *,int) ;
char *decomment(char *,int,FILE *) ;

/*readcif.c*/
void CifError(char *) ;
int  CifGetBuf(FILE *,int,char *) ;
void ReadCifFile(char *) ;
int  blank(int) ;
struct Bounds * connectcells(struct CellList *) ;
char *skipint(char *) ;
char *skipsep(char *) ;
int  upperChar(int) ;

/*cifobjs.c*/
struct cifobjs *CreateCifElement() ;
struct points *CreatePoint(int,int) ;
void InsertCifElementIntoCell(struct cifobjs *) ;
void InsertPoint(struct cifobjs *,int,int) ;
void UpdateBounds(int,int) ;

/*tilde.c*/
char *tilde(char *) ;
char *EndInSlash(char *) ;

/*devices.c*/
struct Device *BuildDevice() ;
void MakeDevices() ;
void  NullFunction() ;
void SelectDevice(char *) ;
int cisncmp(char *,char *,int) ;


void    PLInit() ;
void    PLBInit() ;
void    PLCInit() ;
void    PLDInit() ;
void    HPGLInit() ;
void    HPGLBInit() ;
void    HPGLCInit() ;
void    HPGLDInit() ;
void    PSInit() ;
void    LWInit() ;
#ifdef HP7580
void    HPInit() ;
#endif
#ifdef VERSATEC
void    VPInit() ;
#endif

#else
/* main.c */
void DoCommand() ;
int  Echo() ;
int  Fleaexit() ;
int  ReadCommandFile() ;
int  SaveLine() ;
int  SaveText() ;
int  SetDevice() ;
int  Write() ;
void byebye() ;
void cls() ;
int  doopts() ;
void error() ;
int  findbool() ;
int  findvar() ;
void getbuf() ;
struct Variable *getval() ;
int  interact() ;
int  set() ;
int  source() ;
void stuff() ;
int  test() ;
void usage() ;

/*buildbox.c*/
struct box *BuildElement() ;
void CalculateCellBounds() ;
void ClearCellLink() ;
int  CompareArea() ;
int  CompareXorder() ;
int  CompareYorder() ;
void CreateCell() ;
void CreateOtherTrees() ;
void DescendTree() ;
int  FindCell() ;
void FreeTree() ;
void FreeUp() ;
void InsertBoxIntoCell() ;
void InsertTreeElement() ;
void LinkBoxSides() ;
struct BoxList *MakeBoxListElement() ;
void ProcessCellLayers() ;
void PromptCellModes() ;
void SelectLayer() ;
struct CellLayers *SelectLayerInCurrentCell() ;
void SetCellModes() ;
void XAddToBoxList() ;
void YAddToBoxList() ;
int  askmode() ;
/* output.c */
void DrawBox() ;
void DrawCell() ;
void DrawLayout() ;
void DrawLine() ;
void FillBox() ;
void FindScale() ;
int  TransformX() ;
int  TransformY() ;
void UpdateTransform() ;

/*readtech.c*/
int   ReadTech() ;
char *killsp() ;
char *wordget() ;
int  wordlength() ;

/*readmagic.c*/
int  ReadMagicFile() ;
void clear() ;
char *decomment() ;

/*readcif.c*/
void CifError() ;
int  CifGetBuf() ;
void ReadCifFile() ;
int  blank() ;
struct Bounds * connectcells() ;
char *skipint() ;
char *skipsep() ;
int  upperChar() ;

/*cifobjs.c*/
struct cifobjs *CreateCifElement() ;
struct points *CreatePoint() ;
void InsertCifElementIntoCell() ;
void InsertPoint() ;
void UpdateBounds() ;

/*tilde.c*/
char *tilde() ;
char *EndInSlash() ;

/*devices.c*/
struct Device *BuildDevice() ;
void MakeDevices() ;
int  NullFunction() ;
void SelectDevice() ;
int cisncmp() ;

void    PLInit() ;
void    PLBInit() ;
void    PLCInit() ;
void    PLDInit() ;
void    HPGLInit() ;
void    HPGLBInit() ;
void    HPGLCInit() ;
void    HPGLDInit() ;
void    PSInit() ;
void    LWInit() ;
#ifdef HP7580
void    HPInit() ;
#endif
#ifdef VERSATEC
void    VPInit() ;
#endif

#endif
