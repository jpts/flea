#define SUCESSFUL	0
#define NOPLOTTER 	1
#define OPENFAILED	2
#define LOCKFAILED	3

struct plotter
{
	char *pl_name ;			/* Plotter name */
	char *pl_dev ;			/* Device name */
} ;

extern FILE *checklock() ;
struct plotter *getplent(),*getplnam() ;
