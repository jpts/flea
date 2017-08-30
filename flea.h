/***************************************************************************
 **                                                                       **
 ** File: flea.h                                                          **
 ** Written by: Ed Luke                                                   **
 ** Contains: Customizing definitions, Structure Declarations, and        **
 **           Constants.                                                  **
 **                                                                       **
 ***************************************************************************/


#include <stdio.h>                     /* All flea files use stdio defns.  */
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <string.h>

/* This is the most important adjustable feature.  This define must
   contain the complete path to the installed lib directory.  This directory
   must include at a minimum what the lib directory in this installation
   includes.
   */
#define TECHFILES  "/home3/lush/src/flea.1.4.5/lib/" 

#ifdef INCLUDE_VERSATEC
#define VERSATEC                      /* Include versatec driver */
#endif

#ifdef INCLUDE_HP7580
#define HP7580                  /* Include HP7580 device driver */
#define HP7580LOCK              /* Include Plotget Plotter lock library*/
#define HP7580PORT "/dev/tty22" /* tty plotter is on if LOCK is not included */
#endif

#define DEFAULTDEV "PS"            /* Default output device          */

#define STRINGSIZE 16    /* Size of static strings such as device names    */
#define MAXPOINTS  8192  /* Max polygon and wire sizes */
#define MAXCELLRECURSE 200
#define AreaTree   0    /* Definitions for an array of structures which    */
#define XOrderTree 1    /* defines three different binary trees. The array */
#define YOrderTree 2    /* index is passed to generic binary tree routines */

#define INTEGER 1       /* These Constants defines variable types that are */
#define STRING  2       /* set by the set statement in the .flearc files.  */
#define BOOLEAN 3
#define FLOAT   4

#define BOUNDBOX 1      /* These Constants are used to describe cell info  */
#define CELLNAME 2      /* used for plotting cells.                        */
#define RECURSE  4
#define DRAW     8
#define LABELS   16

#define YEA 1           /* Boolean Constants */
#define NA  0

struct Device  {    /* This structure defines a device in the device list  */
                    /* See devices.c for Device handling routines          */
    char DeviceName[STRINGSIZE] ; /* Storage space for name of device  */
    int yTop,                     /* Plotter coordinates for top,      */
    yBottom,                      /*         bottom,                   */
    xLeft,                        /*         left, and                 */
    xRight,                       /*         right of page.            */
    xAspect,                      /* Number of units/inch x-direction  */
    yAspect,                      /* Number of units/inch y-direction  */
    plotter ;                     /* If YEA use units/inch given above */
    void (*OpenDevice)() ;        /* Initializes device,get paper size */
    void (*SelectPen)() ;         /* Select drawing pen (Color)        */
    void (*SelectFillStyle)() ;   /* Select fill pattern               */
    void (*SelectLineStyle)() ;   /* Select line patterb (Dashed)      */
    void (*DrawLine)() ;          /* Draw a line between two points    */
    void (*DrawLabel)() ;         /* Draw a text relative to a point   */
    void (*DrawCross)() ;         /* Draw a cross (used for labeling)  */
    void (*FillBox)() ;           /* Fill a box using selected pattern */
    void (*CloseDevice)() ;       /* End of drawing                    */
    void (*HardCloseDevice)() ;   /* Error exit                        */
    void (*Polygon)() ;           /* Cif output interface */
    void  (*Wire)() ;
    void (*RoundFlash)() ;
    void (*Box)() ;
    struct Device *next ;         /* Pointer to next device in list    */
} ;

struct Command { /* This structure is used to make a command list for the  */
                /* flearc interpreter found in main.c where an array of   */
                 /* these structures are kept.                             */
    char *CommandName ;        /* Name of the command                  */
    int (*CommandFunction)() ; /* Pointer to the function which        */
} ;                            /*  performs the command task           */

struct Variable {/* This structure is used to make a variable list for     */
                 /* the set command found in main.c                        */
    char *VarName ;  /* The name of the variable                       */
    int  DataType ;  /* Variable type (STRING,INTEGER,BOOLEAN,etc.     */
    union {
        void *VOIDPtr ;
        float *FLOATPtr ;
        char **STRINGPtr ;
        int *INTEGERPtr ;
        int *BOOLEANPtr ;
    } DataPtr ;
    int  Allocated ; /* True for Allocated Variables that must be freed*/
} ;

struct Boolean { /* This structure is used to define boolean inputs for    */
                 /* FindBool() found in main.c                             */
    char *BoolName ;  /* Name of Boolean  */
    int BoolValue ;   /* Value of Boolean */
} ;

struct line {     /* This structure is used to build a stack saving data    */
                  /* about plotting lines given by commands in .flearc      */
    float x1,y1,x2,y2 ; /* Coordinates given for plotting line in inches    */
    char  x1ref ;       /* Relative position value of points */
    char  y1ref ;       /* < = left; > = right; . = center   */
    char  x2ref ;       /* / = bottom; \ = top               */
    char  y2ref ;
              
    struct line *next ; /* pointer to next element on stack            */
} ;

struct text {    /* This structure is used to build a stack saving text     */
                 /* to be plotted given by the text command in .flearc      */
    float x,y ;  /* Coordinates of text placement                           */
    char  xref,yref ; /* Relative position value of points(Same as above)   */
    int   pos,        /* Justification (left,right,or center)               */
          rotate,     /*rotate text 90 degrees if true                      */
          size ;      /* size of text in 100ths of inches                   */
    char *text ;      /* text to be plotted                                 */
        
    struct text *next ; /* pointer to next element on stack                 */
} ;

struct node {    /* This structure used to hold the pointers associated     */
                 /* a binary tree node.                                     */
    struct box *right ;
    struct box *left ;
} ;

struct BoxList { /* This structure holds a pointer to a box description.    */
                 /* This is used for linear lists of boxes of co-existence  */
    struct box *box ;
    struct BoxList *next ;
} ;

struct box {     /* This structure is a box element to be plotted.     */
    struct node Trees[3] ;      /* Three binary trees are used         */
    struct BoxList *LeftSide ;  /* List of boxes concurrent with sides */
    struct BoxList *RightSide ;
    struct BoxList *Top ;
    struct BoxList *Bottom ;
    int xLow,yLow,xHigh,yHigh ; /* Box bounds description              */
    int Area ;                  /* Box area:(hashing and smarts)       */
} ;

struct points {    /* This structure holds points for wires, polygons... */
    int x,y ;      /* Line Coordinates (cell cordnts untrnsfmd           */
    struct points *next ; 
} ;

struct pts {
    int x,y ;
} ;

/*
 * The boxes are attached to a layer in a cell. Each layer contains 
 * parameters associated with informing flea how to plot that layer. 
 * Each layer also contains a list of layers in which boxes must also 
 * be inserted on. Example: A transistor may include the transistor, 
 * active and poly layers.
 */

struct OtherLayersList {/* This structure is used for a linear list of ptrs */
                        /* to other layers to also insert boxes on          */
    struct LayerList *Layer ;      /* Layer pointer */
    struct OtherLayersList *next ; /* Next in list  */
} ;

struct LayerList  {    /* This structure holds the information about how   */
                       /* to plot each layer.                              */
    char *LayerName ;  /* Text name of layer - used to find a layer        */
    int PenNumber ;    /* Pen Color to plot layer in                       */
    int LineStyle ;    /* Line Style (dashed dotted and so on)             */
    int FillPen ;      /* Pen Color to fill box with                       */
    int FillStyle ;    /* Fill Pattern                                     */
    int LabelSize ;    /* Size of text attached to this layer              */
    int LabelPen ;     /* Color of text attached to this layer             */
    int Turd ;         /* if turd exists do not insert boxes on this layer */
    struct OtherLayersList *OtherLayers ; /* Layers to also insert on      */
    struct LayerList *next ; /* Next layer in list of layers (linear)      */
} ;

struct cifobjs {
    int type ;
    int width ;
    int length ;
    int cx,cy ;
    int vx,vy ;
    struct points *Points ;
    struct cifobjs *next ;
} ;

struct CellLayers {                   /* for layer lists for each cell     */
    struct LayerList *LayerInfo ;     /* Plotting information about layer  */
    struct box *AreaRoot ;            /* 3 pointers for storing box info   */
    struct box *XOrderedRoot ;
    struct box *YOrderedRoot ;
    struct cifobjs *cifstuff ;
    struct CellLayers *next ;         /* Linear linked list of cell layers */
} ;

struct Label {           /* holds info about text in cell                  */
    char *Layer ;                          /* Layer name label attached    */
    char *Text ;                           /* Text of label                */
    int xbot,ybot,xtop,ytop,position ;     /* posistion of label           */
    struct Label *next ;                   /* labels held in a linear list */
} ;

struct Array {      /* Structure for holding cell array information         */
    int xlo,xhi,xsep,ylo,yhi,ysep ; 
} ;

struct Transform { /* Structure for holding cell call transform info       */
    int a,b,c,d,e,f ;
} ;

struct Bounds  {   /* Structure for holding cells bounding box             */
    int x1,y1,x2,y2 ;
} ;

struct Cells {/* This structure is for holding the definition of a cell call*/
    char *filename ;             /* Name of magic file                 */
    char *Use_ID ;               /* Name used for plotting cell name   */
    struct Array Array ;         /* Cell Call array value              */
    struct Transform Transform ; /* Cell call transform (rot and so on)*/
    struct Bounds Bounds ;       /* Cells bounding box                 */
    int Mode ; /* Cells Current Read status: Is call actually read in? */
    int CallSymbol ;             /* Used for the cif reader            */
    struct CellList *CellData ;  /* Pointer to cell called             */
    struct Cells *next ;         /* Cell calls held in linear lists    */
} ;

struct CellList { /* This structure used to hold cell contents         */
    char *filename ;              /* name of cell                      */
    int SymbolNumber ;            /* Used for the cif reader           */
    int Linked ;                  /* Tells if Contents were processed  */
    struct Bounds *Bounds ;       /* Tell size of contents of cell     */
    struct CellLayers *Contents ; /* storage of boxes                  */
    struct Cells *CellCalls ;     /* List of cell calls in cell        */
    struct Label *Labels ;        /* List of text in cell              */
    struct CellList *next ;       /* Cells are held in a linear list   */
} ;

#include "prototypes.h"


