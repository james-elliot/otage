#include <stdio.h>
#include <time.h>

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
      if (courant->fils==NULL) exit(0)
      nouveau_fils=courant->fils;
    }
  else
    {
      for (dernier_fils=courant->fils;dernier_fils->frere!=NULL;dernier_fils=dernier_fils->frere);
      dernier_fils->frere=(Coup *)malloc(sizeof(Coup));
      if (dernier_fils->frere==NULL) exit(0)
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

/******************************************/
/**** cette fonction genere la base de donnees ****/
/******************************************/
remplir (Coup * arbre)
{
  FILE * fptr; /* fichier des parties */
  int nb_parties,no_partie;
  int nb; /* nombre lu */
  int resultat; /* victoire ou defaite */
  Coup *le_fils;
  Coup *courant;
  int pions;
  
  if ((fptr=fopen("parties_sym","r"))==NULL)
    {
      printf("Erreur d'ouverture du fichier 'parties'\n");
      exit(-1);
    }
  
  /***********/
  /* traitement */
  /***********/
  fscanf(fptr,"%d",&nb_parties);  
  /* on boucle sur les differentes parties de la base de donnees */
  for (no_partie=0;no_partie<nb_parties;no_partie++)
    {
      courant=arbre;
      fscanf(fptr,"%d",&resultat); /* lecture victoire ou defaite */
      fscanf(fptr,"%d",&pions); /* lecture de la difference des pions */
      fscanf(fptr,"%d",&nb);
      /* on boucle sur les coups de la partie */
      while (nb!=-1)
	{
	  le_fils=trouver_fils(courant,nb);
	  if (le_fils==NULL) /* sequence inconnue */
	    le_fils=creer_fils(nb,courant);
	  /* modification des donnees dans l'arbre*/
	  le_fils->nb_parties_jouees++;
	  if (resultat==1)
	    le_fils->nb_parties_gagnees++;
	  else
	    if (resultat==-1)
	      le_fils->nb_parties_perdues++;
	  le_fils->pions+=pions;
	  /* coup suivant */
	  courant=le_fils;
	  fscanf(fptr,"%d",&nb);
	}
    }
}

/********************************************/
/**** recherche d'une sequence dans l'arbre ****/
/**** renvoie le Coup correspondant ou NULL     ****/
/*******************************************/
Coup * rechercher(Coup * arbre,int * tab,int l)
{
  int i;
  Coup * fils;

  for (i=0;i<l;i++)
    {
      fils=trouver_fils(arbre,tab[i]);
      if (fils==NULL)
	return NULL;
      arbre=fils;
    }
  return fils;
}

/**************************************/
/**** interrogation de la base de donnees ****/
/**************************************/
interroger(Coup * arbre)
{
  int l,i;
  int * tab;
  Coup * resultat;
  double t1;

  printf("longueur de la sequence ?\n");
  scanf("%d",&l);
  tab=(int *)malloc(l*sizeof(int));
  if (tab==NULL) exit(0);
  for (i=0;i<l;i++)
    {
      printf("coup No%D : ",i+1);
      scanf("%d",&tab[i]);
      printf("\n");
    }
  printf("recherche......\n");
  t1=clock();
  resultat=rechercher(arbre,tab,l);
  t1=clock()-t1;
  printf("temps de recherche : %f\n",t1/CLOCKS_PER_SEC);
  if (resultat==NULL)
    printf("pas de donnees pour cette sequence\n");
  else
    {
      printf("nombre de parties jouees : %d\n",resultat->nb_parties_jouees);
      printf("nombre de parties gagnees : %d\n",resultat->nb_parties_gagnees);
      printf("nombre de parties perdues : %d\n",resultat->nb_parties_perdues);
      printf("pions : %d\n",resultat->pions);
    }
}

afficher(Coup * freres)
{
  Coup * courant;
  char eric[100];

  for(courant=freres;courant!=NULL;courant=courant->frere)
    {
      printf("identite : %d\n",courant->id);
      printf("nb parties : %d\n",courant->nb_parties_jouees);
      printf("nb gagnees : %d\n",courant->nb_parties_gagnees);
      printf("nb perdues : %d\n",courant->nb_parties_perdues);
      printf("pions : %d\n",courant->pions);
      if (courant->fils!=NULL)
	afficher(courant->fils);
    }
}

main()
{
  Coup arbre;

  arbre.fils=NULL;
  arbre.frere=NULL;

  remplir(&arbre);
  if (arbre.fils==NULL)
    printf("merde !!\n");
/*  afficher(arbre.fils);*/
  while (1)
    interroger(&arbre);  
}
