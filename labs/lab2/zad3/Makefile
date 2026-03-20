CC = gcc
CFLAGS = -Wall -O3

AR = ar
ARFLAGS = rcs

LIB_SOURCES = $(wildcard sig_*.c)
STATIC_OBJECTS = $(LIB_SOURCES:.c=.o)
SHARED_OBJECTS = $(LIB_SOURCES:.c=.pic.o)

STATIC_LIBSIG = libsig.a

SHARED_LIBSIG_FULL_VERSION = libsig.so.1.0.0
empty :=
space := $(empty) $(empty)
TEMP_LIST = $(subst .,$(space),$(SHARED_LIBSIG_FULL_VERSION))
SHARED_LIBSIG_MAIN_VERSION = $(subst $(space),.,$(wordlist 1,3,$(TEMP_LIST)))
SHARED_LIBSIG = $(subst $(space),.,$(wordlist 1,2,$(TEMP_LIST)))

SOURCES = main.c child.c
OBJECTS = $(SOURCES:%.c=%.o)
EXECS = $(SOURCES:%.c=%)

.PHONY: all clean static shared build_static build_shared build_dynamic_load


all:
	@echo "Usage: make clean|static|shared|build_static|build_shared"


static: $(STATIC_LIBSIG) 

$(STATIC_LIBSIG): $(STATIC_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@


shared: $(SHARED_LIBSIG)

$(SHARED_LIBSIG): $(SHARED_OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHARED_LIBSIG_MAIN_VERSION) $^ -o $(SHARED_LIBSIG_FULL_VERSION) -lc
	ln -s $(SHARED_LIBSIG_FULL_VERSION) $(SHARED_LIBSIG_MAIN_VERSION)
	ln -s $(SHARED_LIBSIG_MAIN_VERSION) $@

%.pic.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@


build_static: clean static
	$(foreach exec,$(EXECS),\
		$(CC) $(CFLAGS) $(exec).c -L./ -lsig -o $(exec);)


build_shared: clean shared
	$(foreach exec,$(EXECS),\
		$(CC) $(CFLAGS) $(exec).c -L./ -lsig -Wl,-rpath,. -o $(exec);)


build_dynamic_load: clean shared
	$(foreach exec,$(EXECS),\
		$(CC) $(CFLAGS) -DDYNAMIC_LOAD $(exec).c -L./ -Wl,-rpath,. -o $(exec);)


clean:
	-@ rm $(STATIC_OBJECTS)
	-@ rm $(STATIC_LIBSIG)
	-@ rm $(SHARED_OBJECTS)
	-@ rm $(SHARED_LIBSIG_FULL_VERSION) $(SHARED_LIBSIG_MAIN_VERSION) $(SHARED_LIBSIG)
	-@ rm $(EXECS)
