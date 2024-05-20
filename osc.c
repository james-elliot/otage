#include "types.h"
#include "osc.h"
#include "valid.h"
#include "search.h"
#include "eval2.h"
#include "book.h"
#include "endgame.h"
#include "table.h"

#include "client.h"
#include <stdio.h>

char username[256];
void save_score (int res1,int res2)
{
}

void init_score()
{
  char *s;
  s=cuserid(username);
  printf("%s\n",username);
}

void clean_up(int i)
{
  exit (0);
}



int depla[]={-11,-10,-9,-1,1,9,10,11};
void printpos(char *tab,int kind);
int victory;
extern char *pname,*passwd;

int numgames=0;
static int nbreplay=0;
int othello(int *fonctab,int *fonctab2,int opening,int kind,int display,
	    int couleur,int time_moves,int *moves,int *moves2,char *paratab)
{
  int boolier,oldtime,numero=0,numero2=0;
  int res1,res2,do_it_again=FALSE;
  int passe,i,j,k,np,myturn,wincol;
  int pos2[70]={
    88,81,11,18, /* COINS */
    31,61,13,16,83,86,38,68, /* A */
    33,63,36,66, /* D */
    14,15,41,51,84,85,48,58,  /* B */
    34,35,43,53,46,56,64,65, /* E */
    24,25,42,52,74,75,47,57, /* G */
    32,23,62,73,67,76,37,26, /* F */
    71,21,17,28,12,82,78,87, /* C */
    22,27,72,77,-1,-1,-1,-1 /* X */
  };
  int pos[65];
  char tab[100],s[1024],tmp[100],svalid[256],oldtab[100],*ptr;
  int co,ret,continue_loop;
  double time_level,time_move;
  int coup,nloops,ecart;
  int tabmove[128];
  double val;
  int movnum=0;
  unsigned int hash=0,sig=0;

/*  toto=time(NULL);
  srand48(toto);
*/

  if (kind==TCLTK) init_score();
  if (kind==REPLAY)
    {
      
      printf("couleur=%d\n",couleur);
      for (i=0;i<33;i++)
	printf("%d ",moves[i]);
      printf("\n");
      for (i=0;i<33;i++)
	printf("%d ",moves2[i]);
      printf("\n");
      if (nbreplay++==10) return 0;
    }

 again:
  if (kind!=REPLAY) nbreplay=0;
  movnum=0;
  val=0;
  for (i=0;i<64;i++)
    pos[i]=pos2[i];

  pos[64]=-1;

  initvals2(fonctab);

  
  

  for (i=0;i<100;i++) tab[i]=0;
  tab[44]= MY_WHITE; 
  tab[45]= MY_BLACK; 
  tab[54]= MY_BLACK; 
  tab[55]= MY_WHITE;

  hash=0;sig=0;

  if (kind==AUTOPLAY)
    {
      for (k=0;k<10;k++)
	{
	  i=(lrand48()%4+3)*10+(lrand48()%4+3);
	  tab[i]=(lrand48()%2)*2-1;
	}
/*    for (k=11;k<19;k++) tab[k]=1;*/

    }

  

  if (kind==REPLAY)
    for (i=0;i<100;i++) tab[i]=paratab[i];
  do_it_again=FALSE;
  for (k=0;k<100;k++)
    oldtab[k]=tab[k];

  np=63;
  if ((kind!=IOS)&&(kind!=JAVA))
    {
      for (i=0;i<100;i++) 
	if (tab[i]!=0)
	  {
	    supprime(i,pos);
	    np--;
	  }
    }
  

  passe=0;
  co=0;

  if ((kind==NORMAL)||(kind==TCLTK))
    {
      do
	{
	  if (kind!=TCLTK) 
	      printf("Qui commence:");
	  ret=scanf("%s",s);
	  if (ret==EOF) clean_up(0);
	  if (s[0]=='O') co=MY_BLACK;
	  if (s[0]=='H') co=MY_WHITE;
	}
      while ((co!=1)&&(co!=-1));
      if (co==-1)
	{
	  myturn=FALSE;
	}
      else
	{
	  myturn=TRUE;
	}
      fflush(stdout);
      
      do
	{
	  if (kind!=TCLTK) printf("Temps de jeu par partie (en minutes) : ");
	  ret=scanf("%d",&i);
	  if (ret==EOF) clean_up(0);
	}
      while ((i<0)||(i>60));
      time_level=i*60;
    }

  if (kind==AUTOPLAY)
    {
      co= 1;
      myturn=TRUE;
      time_level=time_moves;
    }

  if (kind==REPLAY)
    {
      co=couleur;
      time_level=time_moves;
      if (co==1) myturn=TRUE; else myturn=FALSE;
    }

  continue_loop=TRUE;
  nloops=0;
  victory=TRUE;

  if (kind==JAVA)
    {
      ret=scanf("%s",s);
      ptr=s;
      ptr=strtok(ptr,"_");
      /*      printf("couleur:%s\n",ptr);*/
      if (strcmp(ptr,"B")==0) co=MY_BLACK;
      else if (strcmp(ptr,"W")==0) co=MY_WHITE;
      else {
	printf("erreur\n");
	exit(0);
      }

      ptr=strtok(NULL,"_");
      /*      printf("time:%s\n",ptr);*/
      time_level=5*atoi(ptr);
      
      ptr=strtok(NULL,"_");
      /*      printf("passe:%s\n",ptr);*/
      passe=atoi(ptr); /* 1 si passe, 0 sinon */
      
      for (i=10;i<90;i+=10)
	for (j=1;j<9;j++)
	  {
	    ptr=strtok(NULL,"_");
	    if (strcmp(ptr,".")!=0)
	      {
		np--;
		if (strcmp(ptr,"W")==0) tab[i+j]=MY_WHITE;
		else if (strcmp(ptr,"B")==0) tab[i+j]=MY_BLACK;
		else {
		  printf("erreur\n");
		  exit(0);
		}
		supprime(i+j,pos);
	      }
	  }
      myturn=TRUE;
    }

  if (kind==IOS)
    {
      continue_loop=TRUE;
      while (continue_loop)
	{
	  if (nloops++==3000)
	    {
	      printf("Looking for partners...\n");
	      fflush(stdout);
	      v_ios.type=IOS_WHO;
	      ios_put();
	      nloops=0;
	    }
	  
	  v_ios.type=IOS_NONE;
	  ios_get();
	  usleep(200000);
	  switch(v_ios.type)
	    {
	    case IOS_WHO:
	      {
		printf("Retour de IOS_WHO\n");
		fflush(stdout);
		ios_challenge(30,0,120,0,TRUE,1800,2900,TRUE,FALSE);
		break;
	      }
	    case IOS_DECLINE:
	      {
		printf("%s\n",v_ios.u.DECLINE.reason);
		fflush(stdout);
		break;
	      }
	    case IOS_MATCH :
	      {
		printf("MATCH\n");
		printf("color=%d opp=%s\n",
		       v_ios.u.MATCH.my_color,v_ios.u.MATCH.op_name);
		if ( 
		    (v_ios.u.MATCH.my_inc!=0) ||
		    (v_ios.u.MATCH.op_inc!=0) ||
		    (v_ios.u.MATCH.my_time!=v_ios.u.MATCH.op_time))
		  {
		    v_ios.type=IOS_DECLINE;
		    strcpy(v_ios.u.DECLINE.name,v_ios.u.MATCH.op_name);
		    strcpy(v_ios.u.DECLINE.reason,
   "My maker does not allow me to play games with clock increment or games with different times on clock");
		    ios_put();
		  }
		else
		  {
		    v_ios.type=IOS_ACCEPT;
		    strcpy(v_ios.u.ACCEPT.name,v_ios.u.MATCH.op_name);
		    ios_put();
		  }
		break;
	      }
	    case IOS_GAME_CREATE :
	      {
		printf("CREATE\n");
		printf("black=%s white=%s\n",
		       v_ios.u.CREATE.black,v_ios.u.CREATE.white);
		break;
	      }
	    case IOS_BOARD:
	      {
		printf("turn_color:%d turn_my:%d last_move=%d\n",
		       v_ios.u.BOARD.turn_color,
		       v_ios.u.BOARD.turn_my,
		       v_ios.u.BOARD.last_move);
		if (v_ios.u.BOARD.turn_my==TRUE)
		  {
		    co=v_ios.u.BOARD.turn_color;
		    if (co==MY_BLACK)
		      time_level=(double)v_ios.u.BOARD.time_b;
		    if (co==MY_WHITE)
		      time_level=(double)v_ios.u.BOARD.time_w;
		    continue_loop=FALSE;
		    for (i=0;i<100;i++)
		      {
			if (v_ios.u.BOARD.board[i]!=0)
			  {
			    np--;
			    tab[i]=v_ios.u.BOARD.board[i];
			    supprime(i,pos);
			  }
		      }
		    if (v_ios.u.BOARD.last_move==0) 
		      passe=1;
		    else 
		      {
			passe=0;
			coup=v_ios.u.BOARD.last_move;
			hash^=vect_pos[coup+(movnum<<7)];
			sig^=vect_sig[coup+(movnum<<7)];
			tabmove[movnum++]=coup;
		      }
		  }
		myturn=TRUE;
		break;
	      }
	    default:
	      break;
	      fflush(stdout);
	    }
	}
    }
      
  oldtime=time_level;
  for (i=0;i<10;i++) tab[i]=2;
  for (i=90;i<100;i++) tab[i]=2;
  for (i=0;i<100;i+=10) tab[i]=2;
  for (i=9;i<100;i+=10) tab[i]=2;
  coup=0;
  while ((passe<2)&&(np>=0))
    {
      if (myturn)
	{
	  if (kind==TCLTK)
	    {
	      printf("Time_%d\n",(int)time_level);
	      fflush(stdout);
	    }
	  boolier=0;
	  coup=0;
	  if ((kind!=TCLTK)&&(kind!=JAVA)) 
	    if (display==TRUE) printf("coups jouables : ");
	  for (i=0;pos[i]!=-1;i++)
	    if (valid(tab,pos[i],-co,0,pos)!=0)
	      {
		boolier++;
		coup=pos[i];
		s[1]=coup/10+'1'-1;
		s[0]=coup%10+'A'-1;
		s[2]=0;
		if ((kind!=TCLTK)&&(kind!=JAVA)) 
		  if (display==TRUE) printf("%s ",s);
	      }
	  if ((kind!=TCLTK)&&(kind!=JAVA)) 
	    if (display==TRUE)
	      printf("\n");
	  fflush(stdout);
	  /* np est le nbre de cases */
	  time_move=clock();
	  if (boolier>1)
	    {
	      if ((opening)&&(np>30))
		{
		  coup=book(tab,co);
		  if (coup==0)
		    {
		      coup=search(tab,np,co,pos,&time_level,
				  &val,kind,display,boolier,hash,sig,movnum);
		    }
		}
	      else
		coup=search(tab,np,co,pos,&time_level,&val,kind,
			    display,boolier,hash,sig,movnum);
	    }
	  if (kind==JAVA) {
	    printf("%d\n",coup);
	    fflush(stdout);
	    exit(0);}
	  time_move=(clock()-time_move)/CLOCKS_PER_SEC;
	  if ((kind==REPLAY) && (coup!=moves2[numero2++])) 
	    {
	      printf("coup=%d move=%d\n",coup,moves2[numero2-1]);
	      return 0;
	    }
	  if (coup!=0)
	    {
	      np--;
	      i=valid(tab,coup,-co,1,pos);
	      if (i==0) 
		{
		  printf("bug...\n");
		  clean_up(0);
		}
	      if (kind!=TCLTK)
		if (display==TRUE)
		  printf("time_remaining=%5.2f\n",time_level);
		else 
		  {
		  }
	      else
		{
		  printf("Time_%d\n",(int)time_level);
		  fflush(stdout);
		}
	      s[1]=coup/10+'1'-1;
	      s[0]=coup%10+'A'-1;
	      s[2]=0;
	      if (kind==IOS)
		{
		  printf("je joue:%d time_move=%f\n",coup,time_move);
		  fflush(stdout);
		  v_ios.type=IOS_MOVE;
		  v_ios.u.MOVE.yx=coup;
		  v_ios.u.MOVE.time=(float)(time_move);
		  v_ios.u.MOVE.value=(float)val;
		  ios_put();
		}
	      else if (kind==TCLTK)
		{
		  printf("Coup_%s\n",s);
		  printpos(tab,kind);
		  fflush(stdout);
		}
	      else 
		if (display==TRUE)
		  {
		    printf("je joue:%s time_move=%f\n",s,time_move);
		    printpos(tab,kind);
		  }
	      passe=0;
	    }
	  else
	    {
	      passe++;
	      if (kind==IOS)
		{
		  printf("je joue:%d time_move=%f\n",coup,time_move);
		  fflush(stdout);
		  v_ios.type=IOS_MOVE;
		  v_ios.u.MOVE.yx=coup;
		  v_ios.u.MOVE.time=(float)(time_move);
		  v_ios.u.MOVE.value=(float)0.0;
		  ios_put();
		}
	      if (display==TRUE)
		printf("je joue:PA\n");
	    }
	  hash^=vect_pos[coup+(movnum<<7)];
	  sig^=vect_sig[coup+(movnum<<7)];
	  tabmove[movnum++]=coup;
	  printf("coup=%d\n",coup);
	}

      myturn=TRUE;
      boolier=0;
      coup=0;
      for (i=0;pos[i]!=-1;i++)
	if (valid(tab,pos[i],co,0,pos)!=0)
	  {
	    if (boolier==0)
	      if (kind!=TCLTK) 
		if (display==TRUE) printf("coups jouables : ");
		else
		  {}
	      else
		sprintf(svalid,"valid");
	    boolier++;
	    coup=pos[i];
	    if (kind!=TCLTK)
	      {
		s[1]=coup/10+'1'-1;
		s[0]=coup%10+'A'-1;
		s[2]=0;
		if (display==TRUE)
		  printf("%s ",s);
	      }
	    else 
	      {
		s[1]=coup/10+'0'-1;
		s[0]=coup%10+'0'-1;
		s[2]=0;
		sprintf(tmp,"_%s",s);
		strcat(svalid,tmp);
	      }
	  }
      if ((display==TRUE)||(kind==TCLTK))
	printf("\n");

      /* np est le nbre de cases */
      switch(kind)
	{
	case AUTOPLAY:
	  {
	    /* Warning avec l'Autoplay et le hash */
	    if (boolier>1)
	      coup=search(tab,np,-co,pos,&time_level,&val,kind,
			  display,boolier,hash,sig,movnum);
	    if (coup!=0)
	      {
		np--;
		i=valid(tab,coup,co,1,pos);
		if (i==FALSE) 
		  {
		    printf("bug...\n");
		    clean_up(0);
		  }
		passe=0;
		if (display==TRUE)
		  printpos(tab,kind);
	      }
	    else 
	      {
		coup=0;
		passe++;
	      }
	    break;
	  }
	case IOS:
	  {
	    continue_loop=TRUE;
	    while(continue_loop)
	      {
		v_ios.type=IOS_NONE;
		ios_get();
		usleep(200000);
		switch(v_ios.type)
		  {
		  case IOS_BOARD:
		    {
		      if (v_ios.u.BOARD.turn_my==FALSE) break;
		      coup=v_ios.u.BOARD.last_move;
		      printf("coup recu=%d\n",coup);
		      if (coup!=0)
			{
			  i=valid(tab,coup,co,1,pos);
			  if (i==FALSE) 
			    printf("bug\n");
			  np--;
			  passe=0;
			}
		      else
			{
			  coup=0;
			  passe++;
			}
		      fflush(stdout);
		      continue_loop=FALSE;
		      break;
		    }
		  case IOS_GAME_END:
		    {
		      passe=2;continue_loop=FALSE;coup=0;
		      break;
		    }
		  default:
		    break;
		  }
	      }
	    break;
	  }
	case REPLAY:
	  coup=moves[numero++];
	  if (display) printf("je joue %d\n",coup);
	  if (coup!=0)
	    {
	      i=valid(tab,coup,co,1,pos);
	      if (i==0)
		{
		  printf("bug\n");
		  clean_up(0);
		}
	      np--;
	    }
	  else
	    passe++;
	  break;
	case TCLTK:
	case NORMAL:
	  {
	    if (boolier!=0)
	      {
		do
		  {
		    if (kind!=TCLTK) 
		      if (display==TRUE) printf("coup:");
		      else
			{}
		    else
		      {
			printf("%s\n",svalid);
			fflush(stdout);
		      }
		    ret=scanf("%s",s);
		    if (ret==EOF) clean_up(0);
		    if (strcmp(s,"cancel")==0) 
		      goto again;
		    if (kind!=TCLTK) 
		      {
			coup=(s[0]-'A'+1)+(s[1]-'1'+1)*10;
		      }
		    else
		      {
			if (strcmp(s,"fin")==0) 
			  {
			    clean_up(0);
			  }
			if (strlen(s)==1) coup=10*(s[0]-'0'+1)+1; 
			else coup=(s[0]-'0'+1)+(s[1]-'0'+1)*10;
		      }

		    i=valid(tab,coup,co,1,pos);
		    
		  } while (i==0);
		passe=0;
		if ((display==TRUE)||(kind==TCLTK))
		  printpos(tab,kind);
		np--;
	      }
	    else 
	      {
		coup=0;
		passe++;
	      }
	  }
	}
      printf("coup=%d\n",coup);
      hash^=vect_pos[coup+(movnum<<7)];
      sig^=vect_sig[coup+(movnum<<7)];
      tabmove[movnum++]=coup;
    }
      
  res1=0;res2=0;
  for (i=0;i<100;i++)
    {
      if (tab[i]==co) res1++;
      if (tab[i]==-co) res2++;
    }

  if (kind==TCLTK)
    save_score(res1,res2);

    if (res1>res2)
      {
	if (display==TRUE)
	  printf("L'ordinateur gagne: %d a %d\n",res1,res2);
	if (kind==REPLAY) return 0;
      }
    else if (res2>res1)
      {
	victory=FALSE;
	if (display==TRUE)
	  printf("Vous gagnez: %d a %d\n",res2,res1);
      }
    else
      {
	if (display==TRUE)
	  printf("match nul\n");
      }

  if ((victory==FALSE)/*&&(kind!=AUTOPLAY)*/)
    {
      if (kind==IOS)
	ios_disconnect();
      ecart=abs(res2-res1)/10+1;
      if (res2>res1) wincol=-co;else wincol=co; /* Cas de l'autoplay */
      replay(fonctab,fonctab2,opening,oldtime,oldtab,tabmove,movnum,
	     wincol,ecart);
      if (kind==REPLAY) return 0;
/*      savevals();*/

      if (kind==IOS)
	{
	  ios_connect("faust.uni-paderborn.de",5000);
	  ios_logon(pname,passwd,1,0);
	}
    }

  if (kind==TCLTK)
    {
      printf("fin\n");
      fflush(stdout);
      do
	{
	  ret=scanf("%s",s);
	  if (ret==EOF) clean_up(0);
	}
      while (strcmp(s,"fin")!=0);
    }
  numgames++;

  if ((kind!=TCLTK)&&(kind!=AUTOPLAY))
    {
      goto again;
    }
  return res1-res2;
}

void printpos(char *tab,int kind)
{
  int i,j;
  int res1,res2;
  if (kind!=TCLTK) 
    {
      printf("   A B C D E F G H\n");
      for (i=1;i<9;i++)
	{
	  printf("%2d ",i);
	  for(j=1;j<9;j++)
	    switch (tab[10*i+j])
	      {
	      case 0:
		printf(". ");
		break;
	      case 1:
		printf("X ");
		break;
	      case -1:
		printf("O ");
	      }
	  printf("\n");
	}
      fflush(stdout);
    }
  else
    {
      printf("pos");
      for (i=1;i<9;i++)
	{
	  for(j=1;j<9;j++)
	    switch (tab[10*i+j])
	      {
	      case 0:
		printf("_.");
		break;
	      case 1:
		printf("_X");
		break;
	      case -1:
		printf("_O");
	      }
	}
      printf("\n");
      fflush(stdout);
      res1=0;res2=0;
      for (i=0;i<100;i++)
	{
	  if (tab[i]==1) res1++;
	  if (tab[i]==-1) res2++;
	}
      printf("score_%d_%d\n",res1,res2);
      fflush(stdout);
    }
}

  



