SHELL = /bin/sh
CC = /usr/bin/gcc
CCLD = $(CC)
POSIX_VER = 200112L
DEFS = -D_POSIX_C_SOURCE=$(POSIX_VER)

LFS_CFLAGS = $(shell getconf LFS_CFLAGS)
C99_STD_CFLAGS = -std=c99
ABI_CFLAGS = -m64
CFLAGS += $(ABI_CFLAGS) $(C99_STD_CFLAGS) $(LFS_CFLAGS)

# Solaris needs libsocket for bind()/connect() etc.
#LIBS_ADD = -lsocket
LIBS_ADD =
LDFLAGS =

SOURCES = server.c mp3split.c
OBJECTS = server.o mp3split.o
PROGS = mp3server

all: mp3server

.SUFFIXES:
.SUFFIXES: .c .o

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -o $@ -c $<

mp3server: $(OBJECTS)
	$(CCLD) $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS_ADD) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(PROGS)

distclean: clean

.PHONY: clean distclean
