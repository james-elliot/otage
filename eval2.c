#include "types.h"
#include "eval2.h"
#include "valid.h"
#include "osc.h"


static int valedge=2;
static int valcorner=1;
static int valib=7;
/*static int valedge=2;
static int valcorner=2;
static int valib=4;*/


static short int edges[2][65536];
static short int edgestotal[2][65536];
static short int probedge[2][65536];
static short int corns[2][262144];
static short int cornstotal[2][262144];
static short int probcorn[2][262144];
#define MAXNUMBER 500

/**********/
/* scanned */
/**********/
/* regenere le bord a partir de l'indice */
void compute_edge_back(int num,char *bord)
{
  int mask=3,i;

  for (i=0;i<8;i++)
    {
      bord[i]=((num&mask)>>(2*i))-1;
      mask=mask*4;
    }
}

/**********/
/* scanned */
/**********/
/* renvoie l'indice du bord */
int compute_edge(char *bord,int *outv)
{
  int i;
  int out;
  int num,prod;

  /* elimination des bords symetriques au niveau des couleurs*/
  for (i=0;i<8;i++)
    if (bord[i]!= -bord[7-i]) break;
  if (i==8) return -1; /* Position symetrique donc de valeur nulle! */


  out=1;
  for (i=0;i<8;i++)
    if (bord[i]>bord[7-i]) 
      break;
    else 
      if (bord[i]<bord[7-i]) 
	{
	  out=-1;
	  break;
	}
  
  if (out==1)
    {
      out=1;
      for (i=0;i<8;i++)
	if (bord[i]>-bord[7-i]) 
	  break;
	else 
	  if (bord[i]<-bord[7-i]) 
	    {
	      out=-1;
	      break;
	    }
    }
  else
    {
      out=2;
      for (i=0;i<8;i++)
	if (bord[7-i]>-bord[i]) 
	  break;
	else 
	  if (bord[7-i]<-bord[i]) 
	    {
	      out=-2;
	      break;
	    }
    }
  num=0;
  prod=0;

  switch(out)
    {
    case 1:
      {
	for (i=0;i<8;i++)
	  {
	    num|=(bord[i]+1)<<prod;
	    prod+=2;
	  }
	*outv=1;
	break;
      }
    case -1:
      {
	for (i=0;i<8;i++)
	  {
	    num|=(-bord[7-i]+1)<<prod;
	    prod+=2;
	  }
	*outv=-1;
	break;
      }
    case 2:
      {
	for (i=0;i<8;i++)
	  {
	    num|=(bord[7-i]+1)<<prod;
	    prod+=2;
	  }
	*outv=1;
	break;
      }
    case -2:
      {
	for (i=0;i<8;i++)
	  {
	    num|=(-bord[i]+1)<<prod;
	    prod+=2;
	  }
	*outv=-1;
	break;
      }
    }  
  return num;
}

/**********/
/* scanned */
/**********/
void compute_corner_back(int num,char *coin)
{
  int mask=3,i;
  
  for (i=0;i<9;i++)
    {
      coin[i]=((num&mask)>>(2*i))-1;
      mask=mask*4;
    }
}

/**********/
/* scanned */
/**********/
int compute_corner(char *corn,int *outv)
{
  int i,j,num,prod,out,tmp;
  int indice2[9]={1,3,2,6,5,7,0,4,8}; /* Ordre des cases pour la comparaison*/
  /*
     0,1,2
     3,4,5
     6,7,8
*/
  if ((corn[1]== -corn[3])&&(corn[2]==-corn[6])&&(corn[5]==-corn[7])
      &&(corn[0]==0)&&(corn[4]==0)&&(corn[8]==0))
    return -1; /* Position symetrique */

  *outv=1;
  out=1;
  for (i=0;i<9;i++)
    if (corn[indice2[i]]!=0)
      {
	if (corn[indice2[i]]==-1)
	  {
	    *outv=-1;
	    for (j=0;j<9;j++)
	      corn[j]= -corn[j];
	  }
	if ((indice2[i]==3)||
	    (indice2[i]==6)||
	    (indice2[i]==7))
	  out=-1;
	break;
      }

  if (out==-1)
    {
      tmp=corn[3];
      corn[1]=tmp;
      corn[3]=corn[1];
      tmp=corn[6];
      corn[2]=tmp;
      corn[6]=corn[2];
      tmp=corn[7];
      corn[5]=tmp;
      corn[7]=corn[5];
    }
      
  num=0;
  prod=0;
  for (i=0;i<9;i++)
    {
      num|=(corn[i]+1)<<prod;
      prod+=2;
    }
  return num;
}


static int corners_done[256];
static int nb_corners;

/**********/
/* scanned */
/**********/
int is_new_corner(int num)
{
  int i;
  for (i=0;i<nb_corners;i++)
    if (num==corners_done[i]) break;
  if (i<nb_corners) return FALSE;
  corners_done[nb_corners++]=num;
  return TRUE;
}

static int edges_done[256];
static int nb_edges;

/**********/
/* scanned */
/**********/
int is_new_edge(int num)
{
  int i;

  for (i=0;i<nb_edges;i++)
    if (num==edges_done[i]) 
      break;
  if (i<nb_edges) return FALSE;
  edges_done[nb_edges++]=num;
  return TRUE;
}

/**********/
/* scanned */
/**********/
void enforce_edge(char *bord,int edge,int val,int color)
{
  int num,compnum,out;

  /* indice du bord */
  num=compute_edge(bord,&out); /* out nous dit si ya une symetrie en couleur */
  if (num==-1) return; /* position symetrique */
  compnum=((num<<2)+edge)<<1+color;
  if (is_new_edge(compnum)==FALSE) return;

  if (out==1) 
    edges[color][num]+=val;
  else
    edges[color][num]-=val;
  edgestotal[color][num]+=abs(val);
}

/**********/
/* scanned */
/**********/
void enforce_corner(char *corn,int c,int val,int color)
{
  int num,out,compnum;
  
  num=compute_corner(corn,&out);
  if (num==-1) return ; /* Position symetrique */
  compnum=((num<<2)+c)<<1+color;
  if (is_new_corner(compnum)==FALSE) return;


  if (out==1) 
    corns[color][num]+=val;
  else
    corns[color][num]-=val;
  cornstotal[color][num]+=abs(val);
}

/**********/
/* scanned */
/**********/
void recompense(char *tab,int val,int color)
{
  int i,j,k;
  char bord[8],corn[9];
  
  /* pour les 4 bords */
  j=0;
  for (i=11; i<=18; i++)
    bord[j++]=tab[i];
  enforce_edge(bord,0,val,color);

  j=0;
  for (i=11;i<=81;i+=10)
    bord[j++]=tab[i];
  enforce_edge(bord,1,val,color);
  
  j=0;
  for (i=18;i<=88;i+=10)
    bord[j++]=tab[i];
  enforce_edge(bord,2,val,color);
  
  j=0;
  for (i=81;i<=88;i++)
    bord[j++]=tab[i];
  enforce_edge(bord,3,val,color);

  /* pour les 4 corners */
  k=0;
  for (i=10;i<=30;i+=10)
    for (j=1;j<=3;j++)
      corn[k++]=tab[i+j];
  enforce_corner(corn,0,val,color);
  
  k=0;
  for (i=80;i>=60;i-=10)
    for (j=1;j<=3;j++)
      corn[k++]=tab[i+j];
  enforce_corner(corn,1,val,color);
  
  k=0;
  for (i=10;i<=30;i+=10)
    for (j=8;j>=6;j--)
      corn[k++]=tab[i+j];
  enforce_corner(corn,2,val,color);
  
  k=0;
  for (i=80;i>=60;i-=10)
    for (j=8;j>=6;j--)
      corn[k++]=tab[i+j];
  enforce_corner(corn,3,val,color);
}

/**********/
/* scanned */
/**********/
void recompute_prob()
{
  int i,j,num,prod;
  char test[9];
  int color;

  for (color=0;color<2;color++)
    {
      for (i=0;i<65536;i++)
	if (edges[color][i]!=0)
	  {
	    if (edgestotal[color][i]>MAXNUMBER) 
	      {
		edgestotal[color][i]/=2;
		edges[color][i]/=2;
	      }
	    probedge[color][i]=(1000*edges[color][i])/edgestotal[color][i];
	    
	    compute_edge_back(i,test);
	    
	    num=0;prod=0;
	    for (j=7;j>=0;j--)  /* Symetrie laterale */
	      {
		num|=(test[j]+1)<<prod;
		prod+=2;
	      }
	    probedge[color][num]=probedge[color][i];
	    
	    num=0;prod=0;
	    for (j=0;j<8;j++)  /* Symetrie par couleur */
	      {
		num|=(-test[j]+1)<<prod;
		prod+=2;
	      }
	    probedge[color][num]= -probedge[color][i];
	    
	    num=0;prod=0;
	    for (j=7;j>=0;j--)  /* Symetrie laterale et en couleur*/
	      {
		num|=(-test[j]+1)<<prod;
		prod+=2;
	      }
	    probedge[color][num]= -probedge[color][i];
	    
	  }
      
      for (i=0;i<65536*4;i++)
	if (corns[color][i]!=0)
	  {
	    if (cornstotal[color][i]>MAXNUMBER) 
	      {
		cornstotal[color][i]/=2;
		corns[color][i]/=2;
	      }
	    probcorn[color][i]=(1000*corns[color][i])/cornstotal[color][i];
	    
	    compute_corner_back(i,test);
	    
	    num=0;prod=0;
	    for (j=0;j<9;j++)  /* Symetrie laterale */
	      {
		switch(j)
		  {
		  case 1:
		    num|=(test[3]+1)<<prod;
		    break;
		  case 3:
		    num|=(test[1]+1)<<prod;
		    break;
		  case 2:
		    num|=(test[6]+1)<<prod;
		    break;
		  case 6:
		    num|=(test[2]+1)<<prod;
		    break;
		  case 5:
		    num|=(test[7]+1)<<prod;
		    break;
		  case 7:
		    num|=(test[5]+1)<<prod;
		    break;
		  default:
		    num|=(test[j]+1)<<prod;
		    break;
		  }
		prod+=2;
	      }
	    probcorn[color][num]=probcorn[color][i];
	    
	    num=0;prod=0;
	    for (j=0;j<9;j++)  /* Symetrie laterale et en couleurs*/
	      {
		switch(j)
		  {
		  case 1:
		    num|=(-test[3]+1)<<prod;
		    break;
		  case 3:
		    num|=(-test[1]+1)<<prod;
		    break;
		  case 2:
		    num|=(-test[6]+1)<<prod;
		    break;
		  case 6:
		    num|=(-test[2]+1)<<prod;
		    break;
		  case 5:
		    num|=(-test[7]+1)<<prod;
		    break;
		  case 7:
		    num|=(-test[5]+1)<<prod;
		    break;
		  default:
		    num|=(-test[j]+1)<<prod;
		    break;
		  }
		prod+=2;
	      }
	    probcorn[color][num]= -probcorn[color][i];
	    
	    num=prod=0;
	    for (j=0;j<9;j++)
	      {
		num|= (-test[j]+1)<<prod;
		prod+=2;
	      }
	    probcorn[color][num]= -probcorn[color][i];
	  }
    }      
}

/**********/
/* scanned */
/**********/
void savevals()
{
  FILE *fp;
  int i,j,k;
  char test[9];
  char fichier[2][3][30];
  int color;

  strcpy(fichier[0][0],"valeursw");
  strcpy(fichier[1][0],"valeursb");
  strcpy(fichier[0][1],"edgesw");
  strcpy(fichier[1][1],"edgesb");
  strcpy(fichier[0][2],"cornersw");
  strcpy(fichier[1][2],"cornersb");
  for (color=0;color<2;color++)
    {
      rename(fichier[color][0],strcat(fichier[color][0],".bak"));
      if ((fp=fopen(fichier[color][0],"w"))==NULL)
	{
	  printf("erreur ouverture fichier %s\n",fichier[color][0]);
	  exit(-1);
	}
      fwrite(edges[color],sizeof(short int),65536,fp);
      fwrite(edgestotal[color],sizeof(short int),65536,fp);
      fwrite(corns[color],sizeof(short int),262144,fp);
      fwrite(cornstotal[color],sizeof(short int),262144,fp);
      fclose(fp);
      
      if ((fp=fopen(fichier[color][1],"w"))==NULL)
	{
	  printf("Erreur ouverture fichier <%s>\n",fichier[color][1]);
	  exit(-1);
	}
      for (i=0;i<65536;i++)
	if (edgestotal[color][i]!=0)
	  {
	    compute_edge_back(i,test);
	    for (j=0;j<8;j++)
	      fprintf(fp,"%3d",test[j]);
	    fprintf(fp," =%7d %7d %8.6f\n",
		    edges[color][i],edgestotal[color][i],
		    edges[color][i]/(double)edgestotal[color][i]);
	  }
      fflush(fp);
      fclose(fp);
      
      if ((fp=fopen(fichier[color][2],"w"))==NULL)
	{
	  printf("Erreur ouverture fichier <%s>\n",fichier[color][2]);
	  exit(-1);
	}
      for (i=0;i<65536*4;i++)
	if (cornstotal[color][i]!=0)
	  {
	    compute_corner_back(i,test);
	    for (j=0;j<9;j+=3)
	      {
		for (k=0;k<3;k++)
		  fprintf(fp,"%3d",test[j+k]);
		fprintf(fp,"\n");
	      }
	    fprintf(fp," =%7d %7d %8.6f\n",
		    corns[color][i],cornstotal[color][i],
		    corns[color][i]/(double)cornstotal[color][i]);
	  }
      fflush(fp);
      fclose(fp);
    }
}

/**********/
/* scanned */
/**********/
void replay(int *fonctab,int *fonctab2,int opening,int old_time,char *oldtab,int *tabmove,int movnum,int val,int ecart)
{
  int i,coup;
  int np=63,co=1;
  char tab[100];
  int moves[100],moves2[100];
  int numero=0,numero2=0;

  
  /* affichage */
  printf("val=%d ecart=%d\n",val,ecart);
  for (i=0;i<movnum;i++)
    printf("%d ",tabmove[i]);
  printf("\n");

  /* cense ne pas arriver */
  if (tabmove[0]==0)
    {
      printf("coucou\n");
      for (i=0;i<movnum-1;i++)
	tabmove[i]=tabmove[i+1];
      movnum--;
      for (i=0;i<movnum;i++)
	printf("%d ",tabmove[i]);
      printf("\n");
      printf("coucou\n");
    }

  /* on recopie la position de depart dans tab */
  for (i=0;i<100;i++) tab[i]=oldtab[i];

  nb_corners=nb_edges=0;

  for (i=0;i<100;i++)
    if (tab[i]!=0) np--;
  /* donc np=nombre de cases vides (-1) */
  /* on boucle sur les coups de la partie */
  for (i=0;i<movnum;i++)
    {
      /* val represente la couleur qui a gagne (1 pour les noirs, et */
      /* -1 pour les blancs). */

      /* on recompense si pas fin de partie */
      if (np>13) 
	{
	  if (co==val)
	    recompense(tab,co*ecart,(co+1)/2);
	  else
	    recompense(tab,-co*ecart,(co+1)/2);
	}
      /*******************/

      coup=tabmove[i];
      if (co==val) 
	moves[numero++]=coup; /* moves = tableau des coups du vainqueur */
      else
	moves2[numero2++]=coup; /* moves2 = tableau des coups du perdant */

      if (coup!=0) /* si ca a pas passe */
	{
	  valid(tab,coup,-co,1,NULL);
	  np--;
	}
      co=-co;
    }
  recompute_prob();
  savevals();
  othello(fonctab,fonctab2,opening,REPLAY,TRUE,-val,old_time,moves,moves2,
	  oldtab);
}

/**********/
/* scanned */
/**********/
void initvals2(int *tab)
{
  int i;
  static int initdone=FALSE;
  FILE *fp;
  char fichier[2][512];
  int color;
  
  /*
  char *path;
  path=getenv("OTAGEP");
      if (path==NULL) exit(0);*/
  char path[256];
  strcpy(path,PATH);
  
  strcpy(fichier[0],path);
  strcpy(fichier[1],path);
  if (initdone) return;
  initdone=TRUE;
  strcat(fichier[0],"valeursw");
  strcat(fichier[1],"valeursb");
  for(color=0;color<2;color++)
    {
      if((fp=fopen(fichier[color],"r"))==NULL)
	{
	  printf("Erreur ouverture fichier <%s>\n",fichier[color]);
	  exit(-1);
	}
      fread(edges[color],sizeof(short int),65536,fp);
      fread(edgestotal[color],sizeof(short int),65536,fp);
      fread(corns[color],sizeof(short int),262144,fp);
      fread(cornstotal[color],sizeof(short int),262144,fp);
      fclose(fp);
    }
  for (i=0;i<65536;i++)
    {
      probedge[0][i]=0;
      probedge[1][i]=0;
    }
  for (i=0;i<65536*4;i++)
    {
      probcorn[0][i]=0;
      probcorn[1][i]=0;
    }
  recompute_prob();
}

/**********/
/* scanned */
/**********/
int eval2(char *tab,int libb,int libw,int nbb,int nbw,int whowillplay)
{
    int nb_edge,nb_corner,lib;

    int i,j;
    int val,num,prod;

    whowillplay=(whowillplay+1)>>1;

    if (nbb==0) return -32700-nbw;
    if (nbw==0) return 32700+nbb;
    lib=(1000*(libw-libb))/(libb+libw);


    nb_edge=0;
    nb_corner=0;

    num=0;prod=0;
    for (i=11; i<=18; i++)
      {num|=(tab[i]+1)<<prod;prod+=2;}
    nb_edge+=probedge[whowillplay][num];

    num=0;prod=0;
    for (i=11; i<=81; i+=10)
      {num|=(tab[i]+1)<<prod;prod+=2;}
    nb_edge+=probedge[whowillplay][num];

    num=0;prod=0;
    for (i=18; i<=88; i+=10)
      {num|=(tab[i]+1)<<prod;prod+=2;}
    nb_edge+=probedge[whowillplay][num];

    num=0;prod=0;
    for (i=81; i<=88; i++)
      {num|=(tab[i]+1)<<prod;prod+=2;}
    nb_edge+=probedge[whowillplay][num];

    num=prod=0;
    for (i=10;i<=30;i+=10)
      for (j=i+1;j<=i+3;j++)
	{num|=(tab[j]+1)<<prod;prod+=2;}
    nb_corner+=probcorn[whowillplay][num];

    num=prod=0;
    for (i=80;i>=60;i-=10)
      for (j=i+1;j<=i+3;j++)
	{num|=(tab[j]+1)<<prod;prod+=2;}
    nb_corner+=probcorn[whowillplay][num];
    
    num=prod=0;
    for (i=10;i<=30;i+=10)
      for (j=i+8;j>=i+6;j--)
	{num|=(tab[j]+1)<<prod;prod+=2;}
    nb_corner+=probcorn[whowillplay][num];
    
    num=prod=0;
    for (i=80;i>=60;i-=10)
      for (j=i+8;j>=i+6;j--)
	{num|=(tab[j]+1)<<prod;prod+=2;}
    nb_corner+=probcorn[whowillplay][num];

    
    val=(nb_edge*valedge+nb_corner*valcorner+lib*valib)
      /(valedge+valcorner+valib);
    return(val);
}



