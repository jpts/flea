/****************************************************************************
*****************************************************************************
**                                                                         **
** This is a package of generalized subroutines.  These routines           **
** implement the generalized plotter locking standard as implemented here  **
** at Mississippi State University.                                        **
**                                                                         **
**									   **
** Revisions:								   **
**    Modified to use the flock facility instead of lock files in a        **
**    specific directory.  This was needed because if the process was      **
**    killed the lock was not killed as well with the lockfiles.           **
**                   8/10/86   JWG                                         **
**   									   **
*****************************************************************************
****************************************************************************/


#include <stdio.h>
#include "/msu/include/plotters.h"
#include <sys/file.h>
#define PLOTDATABASE "/etc/plotcap"		/* Place where plotter live */

extern char *malloc() ;

static struct plotter *temp ;		/* Temporary plotter pointer */
static int pl_open_flag = 0 ;		/* Is plotcap open ??? */
static FILE *pl_in_file ;		/* Input from plotfile */
static struct plotter *ptr ;		/* Prt to locked plotter */
static FILE *fileptr ;		/* File pointer to open device */




/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : makeplotter                                                 **
** Purpose   : Dynamically create a plotter entry                          **
** Input     : pname - plotter name                                        **
** Output    : dname - device name (/dev/tty22)                            **
** Results   : lname - lock file name                                      **
** Effects   : none                                                        **
**                                                                         **
*****************************************************************************
****************************************************************************/

makeplotter(pname,dname)
char *pname ;		/* Plotter name */
char *dname ;		/* Device name */
{
	if (temp)
	{
		if (temp->pl_name)
			free(temp->pl_name) ;
		if (temp->pl_dev)
			free(temp->pl_dev) ;
		free(temp) ;
	}
	temp = (struct plotter *)malloc(sizeof(struct plotter)) ;
	temp->pl_name = malloc((strlen(pname) + 1) * sizeof(char)) ;
	temp->pl_dev = malloc((strlen(dname) + 1) * sizeof(char)) ;

	strcpy(temp->pl_name,pname) ;
	strcpy(temp->pl_dev,dname) ;
	return ;
}

/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : getplent                                                    **
** Purpose   : gets the next plotter entry from the plotcap file opening   **
**             the file if necessary                                       **
** Input     : none                                                        **
** Returns   : a pointer to a plotter structure                            **
** Results   : none                                                        **
** Effects   : none                                                        **
**                                                                         **
*****************************************************************************
****************************************************************************/

/* The plotter description file entry is a single line for each plotter.
   This lines consists of two strings seperated by a colon.  The first string
   is the common name for the plotter.  The second string is the device name
   for the plotter.
   EXAMPLE:
	   bighp:/dev/ttyA4
	   smallhp:/dev/tty2
*/

struct plotter *getplent()
{
	char buf[512] , *s , *t ;
	char pname[64] , dname[64] , lname[256] ;

/* Open the file if it is not already open */
	if (!pl_open_flag)
	{
		if ((pl_in_file = fopen(PLOTDATABASE,"r")) == NULL)
			return(NULL) ;
		pl_open_flag = 1 ;
	}
again:
	if (!fgets(buf,512,pl_in_file))
		return(NULL) ;
	if (buf[0] == '#')
		goto again ;
	linefix(buf) ;			/* Delete the carriage return */
	s = buf ;
	t = pname ;			/* Get the common name */
	while (*s != ':' && *s != '\0')
		*t++ = *s++  ;
	*t = '\0' ;
	if (*s == '\0')
		goto again ;		/* Malformed name - get another */
	s++ ;
	t = dname ;			/* Get the device name */
	while (*s != '\0' && *s != '#')
		*t++ = *s++ ;
	*t = '\0' ;
	makeplotter(pname,dname) ;
	return(temp) ;
}

/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : setplent                                                    **
** Purpose   : rewind the plotter file                                     **
** Input     : none                                                        **
** Output    : none                                                        **
** Results   : none                                                        **
** Effects   : none                                                        **
**                                                                         **
*****************************************************************************
****************************************************************************/

setplent()
{
	if (pl_open_flag)
		rewind(pl_in_file) ;
}



/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : endplent                                                    **
** Purpose   : Close the plotcap if it is open                             **
** Input     : none                                                        **
** Returns   : none                                                        **
** Results   : none                                                        **
** Effects   : Any structure returned from previous calls to getplent or   **
**             getplnam are invalid because the structure is dellocated    **
**                                                                         **
*****************************************************************************
****************************************************************************/

endplent()
{
	free(temp->pl_name) ;
	free(temp->pl_dev) ;
	free(temp) ;
	fclose(pl_in_file) ;
}


/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : getplnam                                                    **
** Purpose   : get the named plotter from the plotcap file                 **
** Input     : s - the name of the plotter                                 **
** Returns   : pointer to the plotter structure (NULL if it does not       **
**             exists)                                                     **
** Results   : none                                                        **
** Effects   : none                                                        **
**                                                                         **
*****************************************************************************
****************************************************************************/

struct plotter *getplnam(s)
char *s ;
{
	struct plotter *ptr ;
	
	setplent() ;
	do
	{
		ptr = getplent() ;
		if (!ptr)
		{
			endplent() ;
			return(NULL) ;
		}
		if (!strcmp(ptr->pl_name,s))
		{
			endplent() ;
			return(ptr) ;
		}
	} while (1) ;
}

/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : linefix                                                     **
** Purpose   : Delete the carriage return at the end of the line           **
** Input     : s - the string                                              **
** Output    : none                                                        **
** Results   : string is modified                                          **
** Effects   : none                                                        **
**                                                                         **
*****************************************************************************
****************************************************************************/

linefix(s)
char *s ;
{
	while (*s != '\0' && *s != '\n') 
		s++ ;
	*s = '\0' ;
}



/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : checklock                                                   **
** Purpose   : Lock the named plotter or return an error status     	   **
** Input     : s - the plotter name                                        **
** Output    : A file pointer to use to talk to the plotter		   **
** Results   : string is modified                                          **
** Effects   : The integer pointed at by status is modified		   **
**                                                                         **
*****************************************************************************
****************************************************************************/

FILE *checklock(s,status)
char *s ;
int *status ;
{
	ptr = getplnam(s) ;
	if (ptr == NULL)
	{
		*status = NOPLOTTER ;
		fprintf(stderr,"Plotter named '%s' not in /etc/plotcap.\n",s) ;
		return NULL ;
	}
	if ((fileptr = fopen(ptr->pl_dev,"r+")) == NULL)
	{
		*status = OPENFAILED ;
		fprintf(stderr,"Can't open '%s'\n",ptr->pl_dev) ;
		return NULL ;
	}
	if (flock(fileno(fileptr) , LOCK_EX | LOCK_NB))
	{
		*status = LOCKFAILED ;
		fclose(fileptr) ;
		return NULL  ;
	}
	*status = SUCESSFUL ;
	return(fileptr) ;
}



/****************************************************************************
*****************************************************************************
**                                                                         **
** Name      : endlock                                                     **
** Purpose   : Release the lock on the current plotter			   **
** Input     : None							   **
** Output    : 0 Sucessful   -1 Failed					   **
** Results   : flock is removed					 	   **
** Effects   : None 							   **
**                                                                         **
*****************************************************************************
****************************************************************************/

endlock()
{
	if (flock(fileno(fileptr),LOCK_UN))
		return(-1) ;
	if (fclose(fileptr) == EOF)
		return(-1) ;
	return(0) ;
}
