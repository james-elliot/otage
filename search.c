#include "types.h"
#include "alpha.h"
#include "search.h"
#include "endgame.h"
#include "table.h"


static double tabext[25];
static double tabtest[25];
static int minext,mintest;
void initsearch(int kind)
{
  double time;
  double meanext13;
  int i;

  time=clock();
  testend();
  time=clock()-time;
  if (kind!=JAVA) printf("time=%f\n",time/CLOCKS_PER_SEC);
  meanext13 = 2.0*time/CLOCKS_PER_SEC; /*extensive=13*/
  tabext[13]=2*meanext13;
  for (i=14;i<25;i++)
    tabext[i]=4*tabext[i-1];
  for (i=12;i>=0;i--)
    tabext[i]=tabext[i+1]/4;
  for (i=24;i>=0;i--)
    if (tabext[i]<1.0) 
      {
	minext=i;
	break;
      }


/*  tabtest[16]=33*tabext[13];*/
  tabtest[16]=16*tabext[13];
  for (i=17;i<25;i++)
    tabtest[i]=4*tabtest[i-1];
  for (i=15;i>=0;i--)
    tabtest[i]=tabtest[i+1]/4;
  for (i=24;i>=0;i--)
    if (tabtest[i]<1.0) 
      {
	mintest=i;
	break;
      }
  if (kind!=JAVA)
    {
      printf("mintest:%d minext:%d\n",mintest,minext);
      for (i=0;i<25;i++) printf("%d : %f %f\n",i,tabtest[i],tabext[i]);
    }
}

extern int victory;

int search(char *tab,int np,int co,int *pos,double *time_level,double *val,
	   int kind,int display,int npos,unsigned int hash,unsigned int sig,
	   int movnum)
{
  int pos2[65];
  int extensive=0,maxdepth=2,testable=0,fastout;
  double time, time_used=0;
  int i,coup,tcoup,max,valeur,leaf,nodes,hits,collisions;
  int inf,sup,oldval;

  if (kind==JAVA) display=FALSE;

  for (i=24;i>=0;i--)
    if (tabtest[i]< *time_level)
      {
	testable=i;
	break;
      }
  for (i=24;i>=0;i--)
    if (tabext[i]< *time_level)
      {
	extensive=i;
	break;
      }
  if (testable<mintest) testable=mintest;
  if (extensive<minext) extensive=minext;

  if (display==TRUE)
    printf("test=%d ext=%d np=%d\n",testable,extensive,np);

  time=clock();
  if ((np<minext)||(np<extensive-1)) 
    {
      inf=-64;sup=64;valeur=0;
      if (kind==TCLTK)
	{
	  printf("CoupEv_%d_%d_%d\n",0,0,0);
	  fflush(stdout);
	}

      leaf=0;nodes=0;hits=0;collisions=0;
      /*      coup=endgame(tab,pos,co,&valeur,inf,sup,*time_level,time,npos,kind);*/
      coup=endgame(tab,pos,co,&valeur,inf,sup,100000,time,npos,kind);
      *val=valeur;
      if (valeur<0) victory=FALSE;
      if (kind==TCLTK)
	{
	  printf("Eval_%d_%d_%d_%d_%d_%d\n",np+1,coup,valeur,-64,64,0);
	  fflush(stdout);
	}
      else if (display==TRUE)
	printf("max=%d coup=%d valeur=%d\n",np+1,coup,valeur);
      goto fin;
    }

  max=maxdepth;
  for (i=0;i<65;i++) pos2[i]=pos[i];
  sup=32767;inf=-32767;
  do
    {
    back:
      valeur=32767;
      if (kind==TCLTK)
	{
	  printf("CoupEv_%d_%d_%d\n",0,0,0);
	  fflush(stdout);
	}
      coup=alpha(tab,max,co,pos2,&valeur,&leaf,&nodes,&hits,&collisions,
		 inf,sup,kind,display,hash,sig,movnum);

      *val=valeur/1000.0;
      if (kind!=TCLTK)
	{
	  if (display==TRUE)
	    printf("max=%d inf=%d sup=%d coup=%d valeur=%d leaf=%d nodes=%d hits=%d collisions=%d\n",
		   max,inf,sup,coup,valeur,leaf,nodes,hits,collisions);
	}
      else
	{
	  printf("Eval_%d_%d_%d_%d_%d_%d\n",
		 max,coup,valeur,inf,sup,leaf);
	  fflush(stdout);
	}
      
      if (valeur<=inf) 
	{ 
	  inf=-32767;
	  sup=valeur+1;goto back; 
	}
      else if (valeur>=sup)
	{ 
	  inf=valeur-1;
	  sup=32767;goto back;
	}
      else
	{
	  max++;
	  if (max%2==0) {sup=valeur+100;inf=valeur;}
	  else {sup=valeur;inf=valeur-100;}
	}
      time_used=(clock()-time)/CLOCKS_PER_SEC;
      if (display==TRUE) {
	printf("time_used=%f\n",time_used);
	fflush(stdout);}
    }
  while (((kind!=JAVA) && (((*time_level*2)/np)>3*time_used)&&(max<=np)) ||
	 ((kind==JAVA) && ((*time_level*2)>3*time_used)&&(max<=np)));
  /*  while(max<=11);*/

  if (np<testable+2)
    {
      fastout=FALSE;
      inf=-1;sup=1;
      oldval=valeur;
      tcoup=endgame(tab,pos2,co,&valeur,inf,sup,*time_level,time,npos,kind);
      if (valeur<-100)
	{
	  fastout=TRUE;valeur+=1000;
	  if (valeur<0)
	    {
	      valeur=oldval;
	      goto fin;
	    }
	}
      if (valeur>=0) 
	{
	  coup=tcoup;
	  if (valeur>0) valeur=65;
	}
      else
	{
	  valeur=-65;
	  victory=FALSE;
	}
      if (kind==TCLTK){
	printf("Eval_%d_%d_%d_%d_%d_%d\n",
	       np+1,coup,valeur,0,0,0);
	fflush(stdout);}
      else if (display==TRUE)
	printf("max=%d coup=%d valeur=%d\n",
	       np+1,coup,valeur);
      *val=valeur;
      if (valeur==0) goto fin;
      if (fastout) goto fin;
    }

  if (np<extensive+2) 
    {
      if (valeur>0)
	{inf=0;sup=64;}
      else
	{inf=-64;sup=0;}
      if (kind==TCLTK)
	{
	  printf("CoupEv_%d_%d_%d\n",0,0,0);
	  fflush(stdout);
	}

      tcoup=endgame(tab,pos2,co,&valeur,inf,sup,*time_level,time,npos,kind);
      if (tcoup==0) goto fin;
      coup=tcoup;
      if (valeur<-100) valeur+=1000;
      *val=valeur;
      if (valeur<0) victory=FALSE;
      if (kind==TCLTK)
	{
	  printf("Eval_%d_%d_%d_%d_%d_%d\n",np+1,coup,valeur,-64,64,0);
	  fflush(stdout);
	}
      else if (display==TRUE)
	printf("max=%d coup=%d valeur=%d\n",np+1,coup,valeur);
    }
  
 fin:
  *time_level-=(clock()-time)/CLOCKS_PER_SEC;
  ClearTable();
  return coup;
}
