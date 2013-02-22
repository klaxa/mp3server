AUTOMAKE_OPTIONS = \
	foreign \
	1.11 \
        dist-bzip2 \
        no-dist-gzip

ACLOCAL_AMFLAGS = -I m4

SET_FEATURES = -D_POSIX_C_SOURCE=200112L $(shell getconf LFS_CFLAGS)

AM_CFLAGS = $(SET_FEATURES)

bin_PROGRAMS = mp3server

mp3server_SOURCES = mp3split.c \
                    server.c \
                    mp3split.h \
                    mp3stream.h

