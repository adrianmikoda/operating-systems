CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -pthread

all: program1 program2 program3

program1: common.o program1.o
	$(CC) -o program1 common.o program1.o $(LDFLAGS)

program2: common_l2.o program2.o
	$(CC) -o program2 common_l2.o program2.o $(LDFLAGS)

program3: common_l3.o program3.o
	$(CC) -o program3 common_l3.o program3.o $(LDFLAGS)

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

common_l2.o: common.c common.h
	$(CC) $(CFLAGS) -DZAD2 -c common.c -o common_l2.o

common_l3.o: common.c common.h
	$(CC) $(CFLAGS) -DZAD3 -c common.c -o common_l3.o

program1.o: program1.c common.h
	$(CC) $(CFLAGS) -DZAD1 -c program1.c

program2.o: program2.c common.h
	$(CC) $(CFLAGS) -DZAD2 -c program2.c

program3.o: program3.c common.h
	$(CC) $(CFLAGS) -DZAD3 -c program3.c

clean:
	-@ rm -f *.o program1 program2 program3
