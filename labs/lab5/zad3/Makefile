CC = gcc
CFLAGS = -Wall -O3

SHARED_OBJECTS = helpers.o
TARGETS = producer consumer setup cleanup main manager

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): %: %.o $(SHARED_OBJECTS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-@ ./cleanup
	-@ rm *.o 
	-@ rm $(TARGETS)
