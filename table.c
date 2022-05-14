#include <stdlib.h>
#include<stdio.h>
#define _TABLE_C
#include "table.h"

#define HASH_BITS 20

void ClearTable()
{
  int i;
  for (i=0;i<(1<<HASH_BITS);i++)
    table_hash[i].depth=-1;
}

void InitVects()
{
  int i;
  int size=64*128;

  vect_pos=(unsigned int *)malloc(size*sizeof(unsigned int));
  if (vect_pos==NULL) 
    {
      perror("erreur dans malloc");
      exit(0);
    }

  vect_sig=(unsigned int *)malloc(size*sizeof(unsigned int));
  if (vect_sig==NULL) 
    {
      perror("erreur dans malloc");
      exit(0);
    }


  for (i=0;i<size;i++)
    {
      vect_sig[i]=(unsigned int)mrand48();
      vect_pos[i]=lrand48()%(1<<HASH_BITS);
    }

  table_hash=(TableElem *)malloc( (1<<HASH_BITS)*sizeof(TableElem));
  if (table_hash==NULL) 
    {
      perror("erreur dans malloc");
      exit(0);
    }
  ClearTable();
}


