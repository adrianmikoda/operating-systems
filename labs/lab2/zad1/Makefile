CC = gcc
CFLAGS = -Wall -O3
LDFLAGS = -lm
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(SOURCES:%.c=%)

.PHONY: all clean

all: $(OBJECTS) $(TARGETS)

%: %.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-@ rm *.o 
	-@ rm $(TARGETS)
