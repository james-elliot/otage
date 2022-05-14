#include "types.h"
#include "osc.h"
#include "alpha.h"
#include "eval2.h"
#include "table.h"

#define MAX(a,b) (a>b? a:b)
#define MIN(a,b) (a<b? a:b)

static int leaf,nodes,collisions,hits;
static int display,kind;
static unsigned int hashval,hashsig;
static int movnumbase;

#ifdef DEBUG
static int list_moves[100];
#endif
static char * GlobalFlipStack[2048];
static char **FlipStack = &(GlobalFlipStack[0]);
static char dirinc[] = {1, -1,  9, -9, 10, -10, 11, -11, 0};
/* The bit mask for direction i is 1<<i */

/* Bit masks for the directions squares can flip in,
* for example dirmask[10]=81=64+16+1=(1<<6)+(1<<4)+(1<<0)
* hence square 10 (A1) can flip in directions dirinc[0]=1,
* dirinc[4]=9, and dirinc[6]=10: */
static char dirmask[100] = {
0,0,0,0,0,0,0,0,0,0,
0,81,81,87,87,87,87,22,22,0,
0,81,81,87,87,87,87,22,22,0,
0,121,121,255,255,255,255,182,182,0,
0,121,121,255,255,255,255,182,182,0,
0,121,121,255,255,255,255,182,182,0,
0,121,121,255,255,255,255,182,182,0,
0,41,41,171,171,171,171,162,162,0,
0,41,41,171,171,171,171,162,162,0,
0,0,0,0,0,0,0,0,0,0
};


int DrctnlFlips(char *sq, int inc, char color, char oppcol, int *liber)
{
  int lib=0;
   char *pt = sq + inc;
   if(*pt == oppcol){
      int count = 1;
      pt += inc;
      if(*pt == oppcol){
          count++;                /* 2 */
          pt += inc;
          if(*pt == oppcol){
              count++;                /* 3 */
              pt += inc;
              if(*pt == oppcol){
                  count++;        /* 4 */
                  pt += inc;
                  if(*pt == oppcol){
                      count++;        /* 5 */
                      pt += inc;
                      if(*pt == oppcol){
                          count++;        /* 6 */
                          pt += inc;
                      }
                  }
              }
          }
      }
      if(*pt == color){
          int i,g = count;
          do{
             pt -= inc;
             *pt = color;
	     /* Modif */
	     for (i=0;i<8;i++)
	       {
		 if (*(pt+dirinc[i])==0) lib++;
	       }
	     /* Fin modif */
             *(FlipStack++) = pt;
          }while(--g);
	  *liber=lib;
          return count;
      }
   }
  *liber=0;
   return 0;
}


int DoFlips( char *board, int sqnum,
         char color, char oppcol, int *libt )
{
   int i;
   int ct=0;
   int j=dirmask[sqnum];
   char *sq;
   int lib=0;
   *libt=0;
   sq = sqnum + board;
   for(i=7; i>=0; i--){
      if( j&(1<<i) )
	{
	  ct += DrctnlFlips( sq, dirinc[i], color, oppcol ,&lib);
	  (*libt)+=lib;
	}
   }
   return ct;
}

void UndoFlips( int FlipCount, char oppcol )
{/************************************************************************
** This is functionally equivalent to the simpler but slower code line:  *
**   while(FlipCount){ FlipCount--;  *(*(--FlipStack)) = oppcol; }       *
*************************************************************************/
   if(FlipCount&1){
      FlipCount--;
      * (*(--FlipStack)) = oppcol;
   }
   while(FlipCount){
      FlipCount -= 2;
      * (*(--FlipStack)) = oppcol;
      * (*(--FlipStack)) = oppcol;
   }
}

static int pos[65];
static char tab[100];
static char couleur;
static int libw,libb,nbb,nbw;
static int overall_best;
static int profmax;

#define MAXNODE 1
#define MINNODE -1


int fait(char color,int prof,int passe,
	 int alpha,int beta,int type_node)
{
  int i,j;
  int c,nbflips,canplay,ret,num,val,g;
  int libt,oldlibb,oldlibw,oldnbb,oldnbw;  
  int bestmove,done=0,do_save;

  if (prof==0)
    {
      ret=eval2(tab,libb,libw,nbb,nbw,-color);
      leaf++;
      if (couleur==1) 
	return ret;
      else 
	return -ret;
    }

  if (type_node==MAXNODE) g= -32767;
  else g=32767;
  nodes++;

  if (table_hash[hashval].sig==hashsig)
    {
      hits++;
      bestmove=table_hash[hashval].best_move;

#ifdef DEBUG
      printf("Hit : %d\n",bestmove);
#endif
      do_save=TRUE;
      table_hash[hashval].depth=prof;
    }
  else
    {
      if (table_hash[hashval].depth<=prof)
	{
	  do_save=TRUE;
	  table_hash[hashval].sig=hashsig;
	  table_hash[hashval].best_move=0;
	  table_hash[hashval].depth=prof;
	  bestmove=0;
	}
      else
	{
	  bestmove=0;
	  collisions++;
	  do_save=FALSE;
	}
    }

  canplay=FALSE;

  num=0;
  while(pos[num]!=-1 )
    {
      if (bestmove!=0)
	{
	  c=bestmove;done=bestmove;bestmove=0;num=-1;
	}
      else
	{
	  c=pos[num];
	  if (c==done)
	    goto suite;
	}
	 
      if (tab[c]==0)
	{
	  nbflips= DoFlips( tab, c, color, -color,&libt );
	  if (nbflips>0)
	    {

#ifdef DEBUG
	      list_moves[prof]=c;
	      printf("debut eval move=%d prof=%d\n",c,prof);
#endif
#undef DEBUG
	      oldlibb=libb;oldlibw=libw;
	      oldnbb=nbb;oldnbw=nbw;
	      hashval^=vect_pos[c+((movnumbase-prof)<<7)];
	      hashsig^=vect_sig[c+((movnumbase-prof)<<7)];


	      if (color==1) 
		{libb+=libt;libw-=libt;nbb+=(nbflips+1);nbw-=nbflips;}
	      else
		{libw+=libt;libb-=libt;nbw+=(nbflips+1);nbb-=nbflips;}
	      libt=0;
	      for (i=0;i<8;i++)
		{
		  switch (tab[c+depla[i]])
		    {
		    case 0:
		      libt++;
		      break;
		    case 1:
		      libb--;
		      break;
		    case -1:
		      libw--;
		    }
		}
	      if (color==1) 
		libb+=libt;
	      else
		libw+=libt;
		  
	      tab[c]=color;
		  
		  /*	      CountDisc(tab,nbb,nbw,oldnbb,oldnbw,nbflips);*/
	      canplay=TRUE;

	      val= fait(-color,prof-1,0,alpha,beta,-type_node);

#ifdef DEBUG
	      printf("fin eval move=%d prof=%d val=%d g=%d beta=%d alpha=%d nodes=%d leaf=%d\n",
		     c,prof,val,g,beta,alpha,nodes,leaf);
	      for (i=profmax;i>=prof;i--)
		printf("%d ",list_moves[i]);
	      printf("\n");
#endif
#undef DEBUG
	      
	      UndoFlips(nbflips,-color);
	      tab[c]=0;
	      libb=oldlibb;libw=oldlibw;
	      nbb=oldnbb;nbw=oldnbw;
	      hashval^=vect_pos[c+((movnumbase-prof)<<7)];
	      hashsig^=vect_sig[c+((movnumbase-prof)<<7)];

	      if (type_node==MAXNODE)
		{
		  g=MAX(g,val);
		  if (g>alpha)
		    {
		      alpha=g;
		      if (do_save)
			{
#ifdef DEBUG
			  printf("Avant: bestmove=%d\n",
				 table_hash[hashval].best_move);
#endif
			  table_hash[hashval].best_move=c;
#ifdef DEBUG
			  printf("Apres: bestmove=%d\n",
				 table_hash[hashval].best_move);
#endif

			}
		      if (prof==profmax)
			{
			  overall_best=c;
			  if (kind==TCLTK)
			    {
			      printf("CoupEv_%d_%d_%d\n",c,val,leaf);
			      fflush(stdout);
			    }
			  else if (display==TRUE)
			    printf ("mcoup=%d valeur=%d leaf=%d nodes=%d hits=%d collisions=%d\n",
				    c,val,leaf,nodes,hits,collisions);
			}
		      if (g>=beta) 
			return g;
		    }
		}
	      else /* MINNODE */
		{
		  g=MIN(g,val);
		  if (g<beta)
		    {
		      beta=g;
		      if (do_save)
#ifdef DEBUG
			  printf("Avant: bestmove=%d\n",
				 table_hash[hashval].best_move);
#endif
			  table_hash[hashval].best_move=c;
#ifdef DEBUG
			  printf("Apres: bestmove=%d\n",
				 table_hash[hashval].best_move);
#endif
		      if (g<=alpha) 
			return g;
		    }
		}
	    }
	}
    suite:
      num++;
    }

  if (canplay) 
    {
      return g;
    }
  else
    {
      if (passe==1)
	{
	  ret=0;
	  for (i=10;i<90;i+=10)
	    for (j=i+1;j<i+9;j++)
	      {
		ret+=tab[j];
	      }
	  if (ret>0) ret+=32700; else if (ret<0) ret-=32700;
	  leaf++;
	  if (couleur==1)
	    return ret;
	  else 
	    return -ret;
	}
      else
	{/*WARNING*/
	  hashval^=vect_pos[0+((movnumbase-prof)<<7)];
	  hashsig^=vect_sig[0+((movnumbase-prof)<<7)];
	  ret= fait(-color,prof-1,1,alpha,beta,-type_node);
	  hashval^=vect_pos[0+((movnumbase-prof)<<7)];
	  hashsig^=vect_sig[0+((movnumbase-prof)<<7)];
	  return(ret);
	}
    }
}

int alpha(char *tableau,int maxi,int col,int *position,int *valeur,
	  int *nb_leaf,int *nb_nodes,int *nb_hits,int *nb_colls,
	  int inf,int sup,int ki,int disp,unsigned int hash,
	  unsigned int sig,int movnum)
{
  int i,j,k;
  display=disp;
  kind=ki;
  leaf=0;
  nodes=0;hits=0;collisions=0;
  couleur=col;
  for (i=0;i<65;i++) 
    pos[i]=position[i];

  for (i=0;i<100;i++) tab[i]=tableau[i];

  libb=0;libw=0;nbb=0;nbw=0;
  for (i=10;i<90;i+=10)
    for (j=i+1;j<i+9;j++)
      {
	if (tab[j]==1) nbb++;
	if (tab[j]==-1) nbw++;
	if (tab[j]==0)
	  {
	    for (k=0;k<8;k++)
	      {
		switch (tab[j+depla[k]])
		  {
		  case 1:
		    libb++;
		    break;
		  case -1:
		    libw++;
		  }
	      }
	  }
      }
  profmax=maxi;
  hashval=hash;
  hashsig=sig;

  movnumbase=movnum+profmax;
  *valeur=fait(col,maxi,0,inf,sup,MAXNODE);
  *nb_leaf=leaf;
  *nb_nodes=nodes;*nb_hits=hits;*nb_colls=collisions;
  return(overall_best);
}

void CountDisc(char *tab,int nbb,int nbw,int oldnbb,int oldnbw,int nbflips)
{
  int i,j;
  int tnbb=0,tnbw=0;
  for (i=10;i<90;i+=10)
    for (j=1;j<9;j++)
      switch(tab[i+j])
	{
	case 1:
	  tnbb++;
	  break;
	case -1:
	  tnbw++;
	  break;
	}
  if ((nbb!=tnbb)||(nbw!=tnbw))
    {
      printf("nbb=%d tnbb=%d oldnbb=%d nbw=%d tnbw=%d oldnbw=%d nbflips=%d\n",
	     nbb,tnbb,oldnbb,nbw,tnbw,oldnbw,nbflips);
      exit(0);
    } 
}


