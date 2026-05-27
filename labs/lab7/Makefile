CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu99

all: server client

serwer: server.c
	$(CC) $(CFLAGS) -o server server.c

klient: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client

.PHONY: all clean
