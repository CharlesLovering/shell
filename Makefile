OBJS = shell.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

shell : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o shell

shell.o : shell.h shell.cpp 
	$(CC) $(CFLAGS) shell.cpp

clean:
	\rm *.o *~ shell

tar:
	tar cfv shell.cpp shell.h README.txt Makefile