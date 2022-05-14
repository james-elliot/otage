#include <stdio.h>

main()
{
  FILE * rptr;
  FILE * wptr;
  char * noir;
  char * poub;
  char * score;
  char * coup;
  int sn,sb;
  int n;
  int diff;
  int i;
  int j;

  noir=(char *)malloc(1000*sizeof(char));
  poub=(char *)malloc(1000*sizeof(char));
  score=(char *)malloc(1000*sizeof(char));
  coup=(char *)malloc(1000*sizeof(char));

  rptr=fopen("zobi","r");
  wptr=fopen("parties","w");
  fprintf(wptr,"210\n");
  
  for (i=0;i<210;i++)
    {
      if (i==0)
	fscanf(rptr,"%s",noir);
      else
	{
	  strcpy(noir,coup);
	}
      fscanf(rptr,"%s",poub);
      fscanf(rptr,"%s",poub);
      fscanf(rptr,"%s",score);
      printf("%d> chaine lue : <%s>\n",i+1,score);
      sn=atoi(strtok(score,":"));
      sb=atoi(strtok(NULL,":"));
/*      strcpy(poub,score);
	j=0;
	while(poub[j]!=':')
	j++;
	poub[j]='\0';
	sn=atoi(poub);
      strcpy(poub,score);
       poub=poub+j+1;
       sb=atoi(poub);*/
      if (strcmp(noir,"otage")==0)
	{
	  fprintf(wptr,"1 ");
	  n=1;
	}
      else
	{
	  fprintf(wptr,"0 ");
	  n=0;
	}
      if (sn==sb)
	fprintf(wptr,"0");
      else
	{
	  if (sn>sb)
	    {
	      if (n)
		fprintf(wptr,"1");
	      else
		fprintf(wptr,"-1");
	    }
	  else
	    {
	      if (n)
		fprintf(wptr,"-1");
	      else
		fprintf(wptr,"1");
	    }
	}
      diff=sn-sb;
/*      if (!n)
	diff=-diff;*/
      fprintf(wptr," %d",diff);
/*      if (!n)
	fprintf(wptr," 0");  */
      fscanf(rptr,"%s",coup);
      while(strlen(coup)==2)
	{
	  if (strcmp(coup,"pa")==0)
	    fprintf(wptr," 0");
	  else
	    fprintf(wptr," %d%d",coup[0]-96,coup[1]-48);
	  fscanf(rptr,"%s",coup);
	}
      fprintf(wptr," -1\n");
    }
  fclose(rptr);
  fclose(wptr);
}
