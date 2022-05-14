#include "types.h"
#include "book.h"
#include "valid.h"

typedef struct coup 
{
  int id; /* numero du coup */
  struct coup * fils;  
  struct coup * frere;
  int nb_parties_gagnees;
  int nb_parties_perdues;
  int nb_parties_jouees;
  int pions;
} Coup;
static Coup arbre;
static char fourpos[4][100];
static int discs;
static Coup * ouvertures[50];
static int symetries[50];
static int nb_ouv;


static int comparepos(char *tab1,char *tab2)
{
  int i;
  for (i=0;i<100;i++)
    if (tab1[i]!=tab2[i]) return FALSE;
  return TRUE;
}

/********************/
/**** rajoute un fils ****/
/********************/
Coup * creer_fils(int nb, Coup * courant)
{  
  Coup * nouveau_fils;
  Coup * dernier_fils;
 
  /* recherche de l'emplacement du nouveau fils */
  if (courant->fils==NULL)
    {
      courant->fils=(Coup *)malloc(sizeof(Coup));
      if (courant->fils==NULL) exit(0);

      nouveau_fils=courant->fils;
    }
  else
    {
      for (dernier_fils=courant->fils;dernier_fils->frere!=NULL;dernier_fils=dernier_fils->frere);
      dernier_fils->frere=(Coup *)malloc(sizeof(Coup));
      if (dernier_fils->frere==NULL) exit(0);
      nouveau_fils=dernier_fils->frere;
    }
  /* remplissage du fils */
  nouveau_fils->id=nb;
  nouveau_fils->fils=NULL;
  nouveau_fils->frere=NULL;
  nouveau_fils->nb_parties_jouees=0;
  nouveau_fils->nb_parties_gagnees=0;
  nouveau_fils->nb_parties_perdues=0;
  nouveau_fils->pions=0;
  /* retour */
  return nouveau_fils;
}

/****************************************/
/**** renvoie le fils de courant de numero id ****/
/****                 NULL si pas trouve               ****/
/****************************************/
Coup * trouver_fils(Coup * courant, int id)
{
  Coup * fils;

  if (courant->fils==NULL)
    return NULL;
  for (fils=courant->fils;fils!=NULL;fils=fils->frere)
    if (fils->id==id)
      return fils;
  return NULL;
}

/******************************************/
/**** cette fonction genere la base de donnees ****/
/******************************************/
void remplir (Coup * arbre)
{
  FILE * fptr; /* fichier des parties */
  int nb_parties,no_partie;
  int nb; /* nombre lu */
  int resultat; /* victoire ou defaite */
  int noir;
  Coup *le_fils;
  Coup *courant;
  int pions;
  char tmp[256];
  int res;
  /*
  char *path;
  path=getenv("OTAGEP");
  if (path==NULL) 
    {
      printf("setenv OTAGEP first!\n");
      exit(0);
    }
  strcpy(tmp,path);
  */

  strcpy(tmp,PATH);
  strcat(tmp,"parties_sym");
  if ((fptr=fopen(tmp,"r"))==NULL)
    {
      printf("Erreur d'ouverture du fichier 'parties'\n");
      exit(-1);
    }
  
  /***********/
  /* traitement */
  /***********/
  res=fscanf(fptr,"%d",&nb_parties);
  if (res==EOF) {perror("fscanf error");exit(-1);}
  /* on boucle sur les differentes parties de la base de donnees */
  for (no_partie=0;no_partie<nb_parties;no_partie++)
    {
      courant=arbre;
      res=fscanf(fptr,"%d",&noir); /* noir ? */
      if (res==EOF) {perror("fscanf error");exit(-1);}
      res=fscanf(fptr,"%d",&resultat); /* lecture victoire ou defaite */
      if (res==EOF) {perror("fscanf error");exit(-1);}
      res=fscanf(fptr,"%d",&pions); /* lecture de la difference des pions pour noir */
      if (res==EOF) {perror("fscanf error");exit(-1);}
      res=fscanf(fptr,"%d",&nb);
      if (res==EOF) {perror("fscanf error");exit(-1);}
      /* on boucle sur les coups de la partie */
      while (nb!=-1)
	{
	  le_fils=trouver_fils(courant,nb);
	  if (le_fils==NULL) /* sequence inconnue */
	    le_fils=creer_fils(nb,courant);
	  /* modification des donnees dans l'arbre*/
	  le_fils->nb_parties_jouees++;
	  if ((noir && resultat==1)||(!noir && resultat==-1))
	    le_fils->nb_parties_gagnees++;
	  else
	    if (resultat!=0)
	      le_fils->nb_parties_perdues++;
	  le_fils->pions+=pions;
	  /* coup suivant */
	  courant=le_fils;
	  res=fscanf(fptr,"%d",&nb);
	  if (res==EOF) {perror("fscanf error");exit(-1);}
	}
    }
}

/*************/
/*************/
/*************/
int initbook()
{
  static int done=FALSE,opening=FALSE;

  if (done) return opening;
  arbre.fils=NULL;
  arbre.frere=NULL;
  remplir(&arbre);
  opening=TRUE;
  done=TRUE;
  return TRUE;
}

void rechercher(int p, Coup * pere_courant, char * tableau)
{
  char t[100];
  Coup * fils;
  int i,continuer,color,c;

  continuer=1;
  if (p==discs-4)
    {
      for (i=0;i<4 && continuer;i++)
	if (comparepos(tableau,fourpos[i])==TRUE)
	  {
	    ouvertures[nb_ouv]=pere_courant;
	    symetries[nb_ouv]=i;
	    nb_ouv++;
	    continuer=0;
	  }
    }
  else
    {
      /* copie du tableau */
      for (i=0;i<100;i++)
	t[i]=tableau[i];
      /* jouer le coup du pere */
      c=pere_courant->id;
      if (p%2)
	color=-1;
      else
	color=1;
      if (valid(t,c,-color,1,NULL)==0)
	printf("couac\n");
      /* on boucle sur les fils
	 rechercher(nb-1,fils) */
      for(fils=pere_courant->fils;fils!=NULL;fils=fils->frere)
	rechercher(p+1,fils,t);
    }
}

int book(char *tab,int co)
{
  char inittab[100];
  Coup * temp;
  float best_pc;
  int best_sym,i,j,i1,i2,best_coup,won,lost;
  
  /* gdb */
  /*  printf("premier coup : %d\n",arbre.id); */
  /**/
  discs=0;
  /* initialisation fourpos */
  for (i=0;i<4;i++)
    for (j=0;j<100;j++) fourpos[i][j]=0;
  
  /* recopie symetrisee de tab dans fourpos */
  for (i=1;i<9;i++)
    for (j=1;j<9;j++)
      {
	fourpos[0][10*i+j]=tab[10*i+j];
	fourpos[1][10*j+i]=tab[10*i+j];
	fourpos[2][10*(9-i)+(9-j)]=tab[10*i+j];
	fourpos[3][10*(9-j)+(9-i)]=tab[10*i+j];
	if (tab[10*i+j]!=0) discs++;
      }
  
  /* recherche dans la base */
  for (i=0;i<100;i++)
    inittab[i]=0;
  inittab[44]=-1; 
  inittab[45]= 1; 
  inittab[54]= 1; 
  inittab[55]=-1; 
  nb_ouv=0;
  rechercher(0,arbre.fils,inittab);
  
  if (nb_ouv==0)
    return 0;
  else
    {      
      best_coup=-1;
      best_pc=0.0;
      /*recuperer le meilleur fils */
      for (i=0;i<nb_ouv;i++)
	for (temp=ouvertures[i];temp!=NULL;temp=temp->frere)
	  {
	    switch(co) {
	    case 1:
	      won=temp->nb_parties_gagnees;
	      lost=temp->nb_parties_perdues;
	      break;
	    case -1:
	      lost=temp->nb_parties_gagnees;
	      won=temp->nb_parties_perdues;
	      break;
	    default:
	      fprintf(stderr,"co must be 1 or -1");
	      exit(-1);
	    }
	    if ((won > lost) && 
		(((float)won / (float)temp->nb_parties_jouees) > best_pc))
	      {
		best_coup=temp->id;
		best_pc=((float)won / (float)temp->nb_parties_jouees);
		best_sym=symetries[i];
		printf("%f ",best_pc);
	      }
	  }
      printf("\n");
      /* ya pas --> return 0 */
      if (best_coup==-1)
	return 0;
      /* ya return meilleur fils */
      /* il faut de-symetriser */
      i1=best_coup%10;
      i2=best_coup/10;
      switch (best_sym)
	{
	case 0:
	  return 10*i2+i1;
	case 1:
	  return 10*i1+i2;
	case 2:
	  return 10*(9-i2)+(9-i1);
	case 3:
	  return 10*(9-i1)+(9-i2);
	default:
	  fprintf(stderr,"best_sym must be between 0 and 3");
	  exit(-1);
	}
    }
}


