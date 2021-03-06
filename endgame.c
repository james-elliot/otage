#include "types.h"
#include "endgame.h"
#include <signal.h>
static jmp_buf my_env;
static int leaf=0,node=0;
static int empty=0;

/** Fast endgame solver code, written by
       Warren D Smith = wds@research.nec.nj.com
based on code supplied by Jean-Christophe Weill = jcw@seh.etca.fr.
2 Bugs now fixed, thanks to Charles Garrett and Mark Brockington.
[The bugs is from wds not from jcw's code]
Date: Wed Nov 16 07:11:14 EST 1994

See the testdriver routine for how to call it. PrepareToSolve() and
EndSolve() are the routines to export. Everything else is local.
Compile with gcc -O2 thisfile.c and run.

I have translated Jcw's code into English and made a few algorithm mods:
   The most important enhancements are the WINNER_GETS_EMPTIES scoring
mode (which will slow you down by a factor of 1.05), and the use of
parity to help do move ordering, which results in a speedup factor (on
the 112-position test set) of 1.30 with USE_PARITY=2. The flip
stack is simplified.

This code is designed to solve positions with <=10 empties (although
in principle it will work for any number of empties up to the number
of bits in a uint (32 for me). **/
#define MAXEMPTIES 32
/** It is plain alphabeta, no transposition table.  It can be used for
WLD solve by setting alpha=-1 and beta=1 (in which case
WINNER_GETS_EMPTIES could be turned off to get a tiny speedup...) or
for full solve with alpha=-64, beta=64.  It uses a fixed preference
ordering of squares. This is not such a bad thing to do when <=10
empties, where the demand for speed is paramount. When USE_PARITY=X is
turned on (X>0), it also uses parity to help with move ordering,
specifically it will consider all moves into odd regions before
considering any moves into even regions, in situations with more than
X empties.  **/
#define USE_PARITY 4
/** In situations with <=X empties, parity will not be used. I also
put in response-based killers, which yield a speedup of 1.01. **/
#define KILLERHEUR 1
/** Putting killers, or even just killer data-gathering, in the
deeper (parity using) part of the search is experimentally not worth it. **/
#define KILLERHEUR2 0
/** If WINNER_GETS_EMPTIES is turned on it will do scoring using the
winner-gets-empties convention - which never changes who won, but can
change the final score. **/
#define WINNER_GETS_EMPTIES 1
/** Appears to work... at least, it produces the correct game values
(-12,-6,28,-8) for the first 4 test positions and also
for the last 12 (trivial and artificial) test positions. Has now been
more extensively tested.

Unix time results: 33 MHZ IP12 Processor, MIPS R2000A/R3000, gcc -g -O2
  (with WINNER_GETS_EMPTIES=0, FULLSOLVE=2,
   and the 112 test positions in the driver,
   only 100 of which take time [each of the 100 is 12 empties])
USE_PARITY=0: 85.6u 0.4s 1:28 97%
USE_PARITY=1: 69.0u 0.4s 1:13 95%
USE_PARITY=2: 65.8u 0.3s 1:08 97%
USE_PARITY=3: 66.8u 0.3s 1:09 97%
USE_PARITY=4: 67.8u 0.3s 1:11 95% *
USE_PARITY=5: 70.9u 0.5s 1:14 95%
USE_PARITY=6: 72.3u 0.4s 1:16 95% 
  with WINNER_GETS_EMPTIES=1, USE_PARITY=2, FULLSOLVE=2: 69.2u 0.5s 1:13 95%
  with WINNER_GETS_EMPTIES=0, USE_PARITY=2, FULLSOLVE=1: 15.3u 0.1s 0:16 91%
  with WINNER_GETS_EMPTIES=1, USE_PARITY=2, FULLSOLVE=0: 11.8u 0.1s 0:12 92%
  with WINNER_GETS_EMPTIES=0, USE_PARITY=2, FULLSOLVE=0: 11.8u 0.1s 0:12 92%
Node rate: (where each invocation of EndSolve is a "node") 37000-39000/sec.

(On morgoth, 150 MHZ IP19 MIPS R4400, get 147000-149000/sec. 3.8 times faster.)

Later note: some new code mods have now speeded it up about 1.21,
further, beyond the experiments above, and the best value of USE_PARITY
is now 4. Thus the *'d line above now is
    53.0u 0.1s 0:52 100%.
Using TRIES=2 proves that an additional 1.09 speedup could be obtained
by initializing the killer tables to something sensible before searching.

So 175000/sec on morgoth now. Possibly changing uchars
to uints or vice versa in certain places will lead to even more
speedups, depending on the machine. I believe the current char/int combination
is, however, optimal on *my* machine.  In place of the fixed order
best2worst[] you could use some order which (based on past experience
searching positions closely related to this one, e.g. Schaeffer's history
heuristic) you think will be good for this particular position. 
In place of the killertables being initially set to all 0's, you
could initially set them to be responses you think will be good in this
particular position. All this could lead to speedups of an
additional 1.5 or so.
********************/

/* Inside this fast endgame solver, the board is represented by
* a 1D array of 91 uchars board[0..90]:
   ddddddddd
   dxxxxxxxx
   dxxxxxxxx
   dxxxxxxxx
   dxxxxxxxx
   dxxxxxxxx
   dxxxxxxxx
   dxxxxxxxx       where A1 is board[10], H8 is board[80].
   dxxxxxxxx       square(a,b) = board[10+a+b*9] for 0<= a,b <=7.
   dddddddddd   
where d (dummy) squares contain DUMMY, x are EMPTY, BLACK, or WHITE: */
#define WHITE 0
#define EMPTY 1
#define BLACK 2
#define DUMMY 3
#define uchar unsigned char
#define schar signed char
#define uint unsigned int
uchar board[91];
/* Also there is a doubly linked list of the empty squares.
EmHead points to the first empty square in the list (or NULL if none).
The list in maintained in a fixed best-to-worst order. */
struct EmList
{
    int square;
    struct EmList *pred;
    struct EmList *succ;
} EmHead, Ems[64];
#ifndef NULL
#define NULL 0
#endif
/* Also, and finally, each empty square knows the region it is in
and knows the directions you can flip in via some bit masks.
There are up to 32 regions. The parities of the regions are in
the RegionParity bit vector: */
#if USE_PARITY
uint HoleId[91];
uint RegionParity;
#endif

/* The 8 legal directions: */
schar dirinc[] = {1, -1, 8, -8, 9, -9, 10, -10, 0};
/* The bit mask for direction i is 1<<i */

/* Bit masks for the directions squares can flip in,
* for example dirmask[10]=81=64+16+1=(1<<6)+(1<<4)+(1<<0)
* hence square 10 (A1) can flip in directions dirinc[0]=1,
* dirinc[4]=9, and dirinc[6]=10: */
uchar dirmask[91] = {
0,0,0,0,0,0,0,0,0,
0,81,81,87,87,87,87,22,22,
0,81,81,87,87,87,87,22,22,
0,121,121,255,255,255,255,182,182,
0,121,121,255,255,255,255,182,182,
0,121,121,255,255,255,255,182,182,
0,121,121,255,255,255,255,182,182,
0,41,41,171,171,171,171,162,162,
0,41,41,171,171,171,171,162,162,
0,0,0,0,0,0,0,0,0,0
};

/* fixed square ordering: */
/* jcw's order, which is the best of 4 tried: */
int worst2best[64] =
{
/*B2*/      20 , 25 , 65 , 70 ,
/*B1*/      11 , 16 , 19 , 26 , 64 , 71 , 74 , 79 ,
/*C2*/      21 , 24 , 29 , 34 , 56 , 61 , 66 , 69 ,
/*D2*/      22 , 23 , 38 , 43 , 47 , 52 , 67 , 68 ,
/*D3*/      31 , 32 , 39 , 42 , 48 , 51 , 58 , 59 ,
/*D1*/      13 , 14 , 37 , 44 , 46 , 53 , 76 , 77 ,
/*C3*/      30 , 33 , 57 , 60 ,
/*C1*/      12 , 15 , 28 , 35 , 55 , 62 , 75 , 78 ,
/*A1*/      10 , 17 , 73 , 80 , 
/*D4*/      40 , 41 , 49 , 50
};

uchar * GlobalFlipStack[2048];
uchar **FlipStack = &(GlobalFlipStack[0]);

/* sq is a pointer to the square the move is to. */
/* inc is the increment to go in some direction. */
/* color is the color of the mover */
/* oppcol = 2-color is the opposite color. */
/* FlipStack records locations of flipped men so can unflip later. */
/* This routine flips in direction inc and returns count of flips it made: */

int
EndDrctnlFlips( uchar *sq, int inc, int color, int oppcol )
{
   uchar *pt = sq + inc;
   node++;
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
          int g = count;
          do{
             pt -= inc;
             *pt = color;
             *(FlipStack++) = pt;
          }while(--g);
          return count;
      }
   }
   return 0;
}

/* Do all flips involved in making a move to square sqnum of board,
 * and return their count. */
int EndDoFlips( uchar *board, int sqnum,
         int color, int oppcol )
{
   int i;
   int ct=0;
   int j=dirmask[sqnum];
   uchar *sq;
   sq = sqnum + board;
   for(i=7; i>=0; i--){
      if( j&(1<<i) )
         ct += EndDrctnlFlips( sq, dirinc[i], color, oppcol );
   }
   return ct;
}

/* For the last move, we compute the score without updating the board: */

int
CtEndDrctnlFlips( uchar *sq, int inc, int color, int oppcol )
{
   uchar *pt = sq + inc;
   leaf++;
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
      if(*pt == color) return count;
   }

   return 0;
}

int
CountFlips( uchar *board, int sqnum,
         int color, int oppcol )
{
   int i;
   int ct=0;
   int j=dirmask[sqnum];
   uchar *sq;
   sq = sqnum + board;
   for(i=7; i>=0; i--){
      if( j&(1<<i) )
         ct += CtEndDrctnlFlips( sq, dirinc[i], color, oppcol );
   }
   return(ct);
}

/* Call this right after FlipCount=EndDoFlips() to Undo those flips! */

void
EndUndoFlips( int FlipCount, int oppcol )
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

#if KILLERHEUR
struct EmList *killertab[81*2];
#endif


uint minu(uint a, uint b)
{
   if(a<b) return a;
   return b;
}

/* Set up the data structures, other than board array,
 * which will be used by solver. Since this routine consumes
 * about 0.0002 of the time needed for a 12-empty solve,
 * I haven't worried about speeding it up. */
void
PrepareToSolve( uchar *board )
{
   int i,sqnum;
   uint k=0;
   struct EmList *pt;
#if USE_PARITY
   int z;
#endif
   /* create list of empty squares: */
   pt = &EmHead;
   for(i=60-1; i>=0; i--){
      sqnum = worst2best[i];
      if( board[sqnum]==EMPTY ){
         pt->succ = &(Ems[k]);
         Ems[k].pred = pt;
         k++;
         pt = pt->succ;
         pt->square = sqnum;
      }
   }
   pt->succ = NULL;
   if(k>MAXEMPTIES) abort(); /* better not have too many empties... */
#if USE_PARITY
   /* find hole IDs: */
   k = 1;
   for(i=10; i<=80; i++){
      if(board[i]==EMPTY){
         if( board[i-10]==EMPTY ) HoleId[i] = HoleId[i-10];
         else if( board[i-9]==EMPTY ) HoleId[i] = HoleId[i-9];
         else if( board[i-8]==EMPTY ) HoleId[i] = HoleId[i-8];
         else if( board[i-1]==EMPTY ) HoleId[i] = HoleId[i-1];
         else{ HoleId[i] = k; k<<=1; }
      }
      else HoleId[i] = 0;
   }
#define MAXITERS 1
   /* In some sense this is wrong, since you
    * ought to keep doing iters until reach fixed point, but in most
    * othello positions with few empties this ought to work, and besides,
    * this is justifiable since the definition of "hole" in othello
    * is somewhat arbitrary anyway. */
   for(z=MAXITERS; z>0; z--){
      for(i=80; i>=10; i--){
         if(board[i]==EMPTY){
            k = HoleId[i];
            if( board[i+10]==EMPTY ) HoleId[i] = minu(k,HoleId[i+10]);
            if( board[i+9]==EMPTY ) HoleId[i] = minu(k,HoleId[i+9]);
            if( board[i+8]==EMPTY ) HoleId[i] = minu(k,HoleId[i+8]);
            if( board[i+1]==EMPTY ) HoleId[i] = minu(k,HoleId[i+1]);
         }
      }
      for(i=10; i<=80; i++){
         if(board[i]==EMPTY){
            k = HoleId[i];
            if( board[i-10]==EMPTY ) HoleId[i] = minu(k,HoleId[i-10]);
            if( board[i-9]==EMPTY ) HoleId[i] = minu(k,HoleId[i-9]);
            if( board[i-8]==EMPTY ) HoleId[i] = minu(k,HoleId[i-8]);
            if( board[i-1]==EMPTY ) HoleId[i] = minu(k,HoleId[i-1]);
         }
      }
   }
   /* find parity of holes: */
   RegionParity = 0;
   for(i=10; i<=80; i++){
      RegionParity ^= HoleId[i];
   }
#endif
#if KILLERHEUR
   /* Outside routines will set the killer table */
   for(i=81*2-1; i>=0; i--) killertab[i]=0;
   /* some other initial setting may be a better choice in
    * any particular instance */
#endif
}

int
NoParEndSolve (uchar *board, int alpha, int beta, 
   int color, int empties, int discdiff, int prevmove )
{
   int MustPass=1;
   int score = -30000;
   int oppcol = 2-color;
   int sqnum,j,ev;
   struct EmList *em;
#if KILLERHEUR2
   struct EmList *bestmv;
#endif
   for(em=EmHead.succ; em!=NULL; em = em->succ){
      /* go thru list of possible move-squares */
      sqnum = em->square;
      j = EndDoFlips( board, sqnum, color, oppcol );
      if(j){ /* legal move */
          /* place your disc: */
          *(board+sqnum) = color;
          /* delete square from empties list: */
          em->pred->succ = em->succ;
          if(em->succ != NULL)
             em->succ->pred = em->pred;
          /* since legal move exists, MustPass is false: */
          MustPass = 0;
          if(EmHead.succ == NULL){/* After my move, board filled, game over: */
             ev = discdiff + 2*j + 1;
          }
          else if(empties==2){ /* So, now filled but for 1 empty: */
             int j1;
             j1 = CountFlips( board, EmHead.succ->square, oppcol, color);
             if(j1){ /* I move then he moves */
                ev = discdiff + 2*(j-j1);
             }
             else{ /* he will have to pass */
                j1 = CountFlips(board, EmHead.succ->square, color, oppcol);
                if(j1){ /* I pass then he passes then I move */
                   ev = discdiff + 2*(j+j1) + 2;
                }
                else{ /* I move then both must pass, so game over */
                   ev = discdiff + 2*j + 1;
#if WINNER_GETS_EMPTIES
                   if(ev>0) ev++; else if(ev<0) ev--;
#endif
                }
             }
          }
          else{
             ev = -NoParEndSolve(board, -beta, -alpha, 
                      oppcol, empties-1, -discdiff-2*j-1, sqnum);
          }
          EndUndoFlips( j, oppcol );
          /* un-place your disc: */
          *(board+sqnum) = EMPTY;
          /* restore deleted empty square: */
          em->pred->succ = em;
          if(em->succ != NULL)
             em->succ->pred = em;

          if(ev > score){ /* better move: */
             score = ev;
#if KILLERHEUR2
             bestmv = em;
#endif
             if(ev > alpha){
                alpha = ev;
                if(ev >= beta){ /* cutoff */
#if KILLERHEUR2
                   killertab[(prevmove<<1) | (color==WHITE)] = bestmv;
#endif    
                   return score;
                }
             }
          }
       }
    }
    if(MustPass){
       if(prevmove == 0){ /* game over: */
#if WINNER_GETS_EMPTIES
          if(discdiff>0) return discdiff+empties;
          if(discdiff<0) return discdiff-empties;
          return 0;
#else
          return discdiff;
#endif
       }
       else /* I pass: */ return
          -NoParEndSolve( board, -beta, -alpha, oppcol, empties, -discdiff, 0);
    }
#if KILLERHEUR2
    killertab[(prevmove<<1) | (color==WHITE)] = bestmv;
#endif    
    return score;
}

int
ParEndSolve (uchar *board, int alpha, int beta, 
   int color, int empties, int discdiff, int prevmove )
{
   int MustPass=1;
   int score = -30000;
   int oppcol = 2-color;
   int sqnum,j,ev;
   struct EmList *em;
#if KILLERHEUR
   struct EmList *em2, *bestmv, *killer;
   int killaddr;
#endif
#if USE_PARITY
   int par, holepar;
   for( par=(empties>USE_PARITY? 1:0); par>=0; par-- ){
#endif

#if KILLERHEUR
   killaddr = (prevmove<<1) | (color==WHITE);
   killer = killertab[killaddr];
   if( killer && board[killer->square]==EMPTY ) em2 = &EmHead;
   else em2 = EmHead.succ;
   for( ; em2!=NULL; em2 = em2->succ) if(em2!=killer){
      em = (em2==&EmHead)? killer : em2;
#else
   for(em=EmHead.succ; em!=NULL; em = em->succ){
#endif
      /* go thru list of possible move-squares */
      sqnum = em->square;
#if USE_PARITY
      holepar=HoleId[sqnum];
      if( par? (holepar&RegionParity) : !(holepar&RegionParity) ){
#endif
      j = EndDoFlips( board, sqnum, color, oppcol );
      if(j){ /* legal move */
          /* place your disc: */
          *(board+sqnum) = color;
#if USE_PARITY
          /* update parity: */
          RegionParity ^= holepar;
#endif
          /* delete square from empties list: */
          em->pred->succ = em->succ;
          if(em->succ != NULL)
             em->succ->pred = em->pred;
          /* since legal move exists, MustPass is false: */
          MustPass = 0;
#if USE_PARITY
          if(empties<=1+USE_PARITY)
             ev = -NoParEndSolve(board, -beta, -alpha, 
                   oppcol, empties-1, -discdiff-2*j-1, sqnum);
          else
#endif
             ev = -ParEndSolve(board, -beta, -alpha, 
                   oppcol, empties-1, -discdiff-2*j-1, sqnum);
          EndUndoFlips( j, oppcol );
#if USE_PARITY
          /* restore parity of hole */
          RegionParity ^= holepar;
#endif
          /* un-place your disc: */
          *(board+sqnum) = EMPTY;
          /* restore deleted empty square: */
          em->pred->succ = em;
          if(em->succ != NULL)
             em->succ->pred = em;

          if(ev > score){ /* better move: */
             score = ev;
#if KILLERHEUR

             bestmv = em;
#endif
             if(ev > alpha){
                alpha = ev;
                if(ev >= beta){ 
#if KILLERHEUR
                   killertab[killaddr] = bestmv;
#endif    
                   return score;
	       }
	    }
	 }
       }
#if USE_PARITY
    }}
#endif
    }

    if(MustPass){
       if(prevmove == 0){ /* game over: */
#if WINNER_GETS_EMPTIES
          if(discdiff>0) return discdiff+empties;
          if(discdiff<0) return discdiff-empties;
          return 0;
#else
          return discdiff;
#endif
       }
       else /* I pass: */ return
          -ParEndSolve( board, -beta, -alpha, oppcol, empties, -discdiff, 0);
    }
#if KILLERHEUR
    killertab[killaddr] = bestmv;
#endif    
    return score;
}

/* The search itself. Assumes relevant data structures have been set up with
 * PrepareToSolve.
 * color is the color on move. Discdiff is color disc count - opposite
 * color disc count. The value of this at the end of the game is returned.
 * prevmove==0 if previous move was a pass, otherwise non0.
 * empties>0 is number of empty squares.
*******************/

int
EndSolve (uchar *board, int alpha, int beta, 
   int color, int empties, int discdiff, int prevmove )
{
#if USE_PARITY
   if(empties <= (2>USE_PARITY ? 2 : USE_PARITY) )
      return NoParEndSolve(board,alpha,beta,color,empties,discdiff,prevmove);
   else 
#endif
   return ParEndSolve(board,alpha,beta,color,empties,discdiff,prevmove);
}

static void sighandler()
{
  longjmp(my_env,-1);
}

int endgame(char *tab,int *pos,int color,int *valeur,int alpha,int beta,
	    double time_level,int time,int npos,int kind)
{
  int num,i,j,c,d,dep,flag,toto;
  int wc,bc,emp,val;
  int i1,k,disc,max,coup=0,nbdone=0;
  char tab2[100];
  double remtime;
  int authtime;
  struct itimerval timer;

/*  signal(SIGALRM,sighandler);*/
  signal(SIGVTALRM,sighandler);
  remtime=time_level-(clock()-time)/CLOCKS_PER_SEC;
  if (remtime<=0) remtime=1;

  *valeur=-65;
  max=alpha;
  num=0;
  while (pos[num]!=-1)
    {
      c=pos[num];
      flag=0;
      for (i=0;i<8;i++)
	{
	  dep = depla[i];
	  d=c+dep;
	  if (tab[d]== -color)
	    {
	      do
		{
		  d+=dep;
		}
	      while(tab[d]== -color);
	      if (tab[d]==color)
		{
		  
		  if (flag==0)
		    bcopy(tab,tab2,sizeof(char)*100);
		  flag=1;
		  for (j=c;j!=d;j+=dep) tab2[j]=color;
		}
	    }
	}
      if (flag==1) 
	{
	  authtime=(int)(remtime/(npos-nbdone)+0.5);
	  if (kind!=JAVA) printf("coup=%d time=%d\n",pos[num],authtime);

	  for (j=0;j<=90;j++) board[j]=DUMMY;
	  wc=bc=emp=0;
	  for (i=1;i<9;i++)
	    for (j=1;j<9;j++)
	      {
		i1=10*i+j;k=10+9*(i-1)+(j-1);
		disc=tab2[i1];
		if (disc==color) {board[k]=WHITE;wc++;}
		if (disc==-color) {board[k]=BLACK;bc++;}
		if (disc==0) {board[k]=EMPTY;emp++;}
	      }
	  PrepareToSolve(board);
	  toto=clock();
	  if (setjmp(my_env))
	    {
	      if (kind!=JAVA) printf("out of here\n");
	      *valeur= *valeur-1000;
	      return coup;
	    }

/*	  alarm(authtime);*/
	  timer.it_interval.tv_sec=0;
	  timer.it_interval.tv_usec=0;
	  timer.it_value.tv_sec=authtime;
	  timer.it_value.tv_usec=0;
	  setitimer(ITIMER_VIRTUAL,&timer,NULL);
	  leaf=0;node=0;
	  val=-EndSolve(board,-beta,-max,BLACK,emp,bc-wc,1);
/*	  alarm(0);*/
	  timer.it_interval.tv_sec=0;
	  timer.it_interval.tv_usec=0;
	  timer.it_value.tv_sec=0;
	  timer.it_value.tv_usec=0;
	  setitimer(ITIMER_VIRTUAL,&timer,NULL);

	  nbdone++;
	  toto=(clock()-toto);
	  if (kind!=JAVA) printf("node=%d leaf=%d time=%f\n",node,leaf,
		 ((float)toto)/CLOCKS_PER_SEC) ;

	  if (val>max)
	    {
	      max=val;
	      *valeur=max;
	      coup=pos[num];
	      if (kind!=JAVA) printf("coup=%d max=%d beta=%d\n",coup,max,beta);
	      if (max>=beta) return coup;
	    }
	  toto=toto/CLOCKS_PER_SEC;
	  remtime-=toto;
	  if (remtime<=0) 
	    {
	      *valeur= *valeur-1000;
	      return coup;
	    }
	}
      num++;
    }
  return coup;
}


void testend()
{   
  int val,emp,wc,bc,j,k,x,y;
  char bds[65] =
    "..wwwww.b.wwbw..bbwbbwwwbwbbb.wwbbwbbwbwbbbwbwww..bbwbw....bbbbw";
  int col=WHITE;

      for(j=0; j<=90; j++) board[j] = DUMMY;
      wc=bc=emp=0;
      for(j=0; j<64; j++)
	{
	  x = j&7; y = (j>>3)&7; k = x+10+9*y;
	  if(bds[j]=='w'){ board[k] = WHITE; wc++; }
	  else if(bds[j]=='b'){ board[k] = BLACK; bc++; }
	  else if(bds[j]=='.'){ board[k] = EMPTY; emp++; }
	}
  empty=emp;
  PrepareToSolve( board );
  val = EndSolve( board, -64, 64, col, emp, wc-bc, 1 );
  /*  printf("val=%d node=%d leaf=%d\n",val,node,leaf); */
}
