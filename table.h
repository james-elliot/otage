#ifndef _TABLE_H
#define _TABLE_H
#ifdef _TABLE_C
#define GLOBAL
#else
#define GLOBAL extern 
#endif


GLOBAL unsigned int *vect_pos;
GLOBAL unsigned int *vect_sig;

typedef struct _TableElem
{
  unsigned int sig;
  signed char depth;
  unsigned char best_move;
}
TableElem;

GLOBAL TableElem *table_hash;
void InitVects();
void ClearTable();

#endif

