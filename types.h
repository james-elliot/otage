#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <setjmp.h>
#include <sys/time.h>

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
extern int depla[];
#define NORMAL 0
#define TCLTK 1
#define AUTOPLAY 2
#define IOS 3
#define REPLAY 4
#define JAVA 5

#define PATH "./"
/*#define PATH "/home2/jma/alliot/OTAGE/Work/"*/
/*#define PATH "/devt/cgi-bin/otage/"*/

