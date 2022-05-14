#include "valid.h"
#include "types.h"
#include "osc.h"

void supprime(int coup,int *pos)
{
  int i,j;
  if (pos==NULL) return;
  /*  for (i=0;i<64;i++)
    printf("pos[%d]=%d ",i,pos[i]);
  printf("\n");*/
  for (i=0;i<64;i++)
    if (pos[i]==coup) break;
  for (j=i;j<=63;j++) pos[j]=pos[j+1];
}

int valid(char *tab,int coup,int t,int oui,int *pos)
{
  int flag=0;
  int i,buf,buf2;

  if (tab[coup]!=0) return(0);
  for (i=0;i<8;i++)
    {
      buf=coup;
      while (tab[buf+=depla[i]]==t);
      if ((buf!=coup+depla[i])&&(tab[buf]==-t))
	{
	  flag=1;
	  for (buf2=coup;buf2!=buf;buf2+=depla[i])
	    {
	      if (oui)
		{
		  tab[buf2]= -t;
		}
	    }
	}
    }
  if ((flag)&&(oui))
    {
      supprime(coup,pos);
    }
  return(flag);
}

