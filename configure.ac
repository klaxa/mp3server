#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([mp3server],[0.2])
AC_CONFIG_SRCDIR([mp3split.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign 1.11 dist-bzip2 tar-ustar])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AX_CFLAGS_SUN_OPTION([-xc99])

dnl check if the compiler supports '-c' and '-o' options
AM_PROG_CC_C_O

AC_CANONICAL_HOST

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h getopt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Obtain size of a 'unsigned long' and define as SIZEOF_UNSIGNED_LONG
AC_CHECK_SIZEOF(unsigned long)

# Obtain size of size_t and define as SIZEOF_SIZE_T
AC_CHECK_SIZEOF(size_t)

# Obtain size of an unsigned int pointer and define as SIZEOF_UNSIGNED_INTP
AC_CHECK_SIZEOF(unsigned int*)

# Add appropriate values-xpg6.o on Solaris when building with GCC
case "$host_os" in
    solaris*)
        if test x$GCC = xyes
        then
          AC_RUN_IFELSE(
            [AC_LANG_PROGRAM([], [
            #ifndef _LP64
              return -1;
            #endif
            ])],
            [solaris_gcc_xpg_abi=abi64],
            [solaris_gcc_xpg_abi=abi32]
          )

          if test x$solaris_gcc_xpg_abi = xabi64
          then
            LIBS="$LIBS /usr/lib/64/values-xpg6.o"
          else
            LIBS="$LIBS /usr/lib/values-xpg6.o"
          fi
        fi
        ;;
esac

if test x$GCC = xyes
then
        AX_APPEND_COMPILE_FLAGS([-Wall])
        AX_APPEND_COMPILE_FLAGS([-Wextra])
fi

# Checks for library functions.
AC_CHECK_FUNCS([accept fcntl getaddrinfo memset select setsockopt socket strerror getopt_long])
AC_SEARCH_LIBS([accept], [socket])
AC_SEARCH_LIBS([getaddrinfo], [socket])
AC_SEARCH_LIBS([socket], [socket])
AC_SEARCH_LIBS([setsockopt], [socket])

# required system services
AC_SYS_LARGEFILE

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
