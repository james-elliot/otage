#include "types.h"
#include "osc.h"
#include "book.h"
#define CLIENT
#include "client.h"
#include "table.h"
#include "search.h"

char *pname="otage";
char *passwd="alliot62";

int main(int argc,char **argv)
{
  int opening=FALSE,kind;
  int fonctab[]={554,
		   -111,-302,
		   111,10,3,
		   9,0,-4,-1,
		   20,123};
  int fonctab2[]={      500,
                       -240, -130,
                        85,    49,  1,
                        69,    23,  9, 32,
                        180,312};
  int time_moves=0;

  if (argc==1) 
    {
      kind=JAVA;
    }
  else
    {
      if (strcmp(argv[1],"-tcltk")==0)
	{
	  kind=TCLTK;
	}
      else if (strcmp(argv[1],"-autoplay")==0)
	{
	  kind=AUTOPLAY;
	  if (argc>2) 
	    {
	      time_moves=atoi(argv[2]);
	      if ((time_moves>30)||(time_moves<0))
		exit(0);
	      time_moves*=60;
	    }
	}
      else if (strcmp(argv[1],"-ios")==0)
	kind=IOS;
      else if (strcmp(argv[1],"-norm")==0)
	kind=NORMAL;
      else
	exit(0);
    }

  initsearch(kind);
  InitVects();
  opening=initbook();
  
  if (kind==IOS)
    {
      ios_connect("external.nj.nec.com",5000);
      ios_logon(pname,passwd,1,1);
    }
  else if (kind==JAVA) 
    {
      printf("content-type: text/plain\n\n");
      fflush(stdout);
    }

  /*  othello(fonctab,fonctab2,opening,kind,TRUE,0,time_moves,NULL,NULL,NULL);*/
  othello(fonctab,fonctab2,opening,kind,TRUE,0,time_moves,NULL,NULL,NULL);
  exit(0);
}

