OBJS = table.o alpha.o book.o endgame.o eval2.o search.o osc.o valid.o client.o main.o
CFLAGS =  -O2  -W -Wall -fomit-frame-pointer
CC = gcc
#CFLAGS =  -g  -W -Wall
LIBS = -lm

all : a.out valeursb valeursw

valeursb :
	gzip -d -c valeursb.gz > valeursb

valeursw :
	gzip -d -c valeursw.gz > valeursw

a.out : $(OBJS)
	$(CC) $(OBJS) $(LIBS)

clean : 
	\rm -f *.o a.out *~ core valeursw valeursb

depend:
	gcc -MM *.c > .depend

include .depend
