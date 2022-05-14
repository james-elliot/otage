#include <stdio.h>

int symetrie(int n, int ouv)
{
  int i,j;
  
  i=n/10;
  j=n%10;
  switch(ouv)
    {
    case 34:
      return n;
      break;
    case 43:
      return i+j*10;
      break;
    case 65:
      return (9-i)*10+9-j;
      break;
    case 56:
      return (9-j)*10+9-i;
      break;
    default:
      printf("blegr,zejlfghejk\n");
      exit(-1);
    }
}


main()
{
  FILE * rptr;
  FILE * wptr;
  int n;
  int nb;
  int i;
  int ouv;
  int a;

  rptr=fopen("parties","r");
  wptr=fopen("parties_sym","w");
  
  /* re-ecriture nombre de parties */
  fscanf(rptr,"%d",&nb);
  fprintf(wptr,"%d\n",nb);
  
  for(i=0;i<nb;i++)
    {
      /* re-ecriture couleur otage */
      fscanf(rptr,"%d",&n);
      fprintf(wptr,"%d ",n);
      /* re-ecriture victoire-defaite */
      fscanf(rptr,"%d",&n);
      fprintf(wptr,"%d ",n);
      /* re-ecriture difference pions */
      fscanf(rptr,"%d",&n);
      fprintf(wptr,"%d ",n);
      
      fscanf(rptr,"%d",&n);
      ouv=n;
      while(n!=-1)
	{
	  a=symetrie(n,ouv);
	  fprintf(wptr,"%d ",(a%10)*10+a/10);
	  fscanf(rptr,"%d",&n);
	}
      fprintf(wptr,"-1\n");
    }
  fclose(rptr);
  fclose(wptr);
}      
