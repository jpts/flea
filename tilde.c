/*
 *   Flea, (F)un (L)oveable (E)ngineering (A)rtist
 *   Written By  Edward A Luke for
 *   Mississippi State University
 *   Last Modified  Oct 1989
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <pwd.h>

char *tilde(file)
char *file ;
{
    struct passwd *pass ;
    char buf[512] ;
    char *newfile ;
    
    if(file[0] != '~') {
        char *s = (char *)malloc(strlen(file)+1) ;
        strcpy(s,file) ;
        return s ;
    } 
    if(file[1] == '/') {
        pass = getpwuid(getuid()) ;
    } else {
        char *t ;
        char *s ;
        
        s = file+1 ;
        t = buf ;
        while(*s != '/' && *s != '\0')
          *t++ = *s++ ;
        *t = '\0' ;
        pass = getpwnam(buf) ;
    }
    if(pass == NULL) {
        char *s = (char *)malloc(strlen(file)+1) ;
        strcpy(s,file) ;
        return s ;
    }
    newfile = (char *)malloc(strlen(file)+strlen(pass->pw_dir)+1) ;
    strcpy(newfile,pass->pw_dir) ;
    strcat(newfile,file) ;
    return(newfile) ;
}

char *EndInSlash(file)
char *file ;
{
    char *t ;
    
    if(file == NULL) {
        t = (char *) malloc(1) ;
        *t = '\0' ;
        return t ;
    }
    t = file + strlen(file)-1 ;
    if(*t != '/') {
        t = (char *) malloc(strlen(file)+2) ;
        strcpy(t,file) ;
        strcat(t,"/") ;
        return(t) ;
    }
    t = (char *)malloc(strlen(file)+1) ;
    strcpy(t,file) ;
    return t ;
}
