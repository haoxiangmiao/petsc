#ifdef PETSC_RCS_HEADER
"$Id: petscconf.h,v 1.5 2001/06/20 21:12:57 buschelm Exp $"
"Defines the configuration for this machine"
#endif

#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define PARCH_win32 
#define PETSC_ARCH_NAME "win32"
#define PETSC_HAVE_WIN32
#define PETSC_HAVE_ICL
#define PETSC_HAVE_LIMITS_H
#define PETSC_HAVE_STDLIB_H 
#define PETSC_HAVE_STRING_H 
#define PETSC_HAVE_SEARCH_H
#define PETSC_HAVE_IO_H
#define PETSC_HAVE_SYS_STAT_H

#define PETSC_HAVE_STD_COMPLEX
#define PETSC_MISSING_LAPACK_GESVD
#define PETSC_MISSING_LAPACK_GEEV

#define PETSC_HAVE_FORTRAN_CAPS 

#define PETSC_HAVE_READLINK
#define PETSC_HAVE_MEMMOVE

#define PETSC_HAVE_RAND
#define PETSC_CANNOT_START_DEBUGGER
#define PETSC_HAVE_CLOCK

#define PETSC_HAVE_GET_USER_NAME
#define PETSC_SIZEOF_VOID_P 4
#define PETSC_SIZEOF_INT 4
#define PETSC_SIZEOF_DOUBLE 8
#define PETSC_BITS_PER_BYTE 8
#define PETSC_SIZEOF_FLOAT 4
#define PETSC_SIZEOF_LONG 4
#define PETSC_SIZEOF_LONG_LONG 8

#define PETSC_USE_NT_TIME
#define PETSC_HAVE_NO_GETRUSAGE

#define PETSC_MISSING_SIGBUS
#define PETSC_MISSING_SIGQUIT
#define PETSC_MISSING_SIGSYS

#define PETSC_HAVE__ACCESS
#define PETSC_HAVE__GETCWD
#define PETSC_HAVE__SLEEP
#define PETSC_HAVE__SLEEP_MILISEC
#define PETSC_HAVE_IARG_COUNT_PROGNAME

#ifdef PETSC_USE_MAT_SINGLE
#  define PETSC_MEMALIGN 16
#  define PETSC_HAVE_SSE "iclsse.h"
#endif
#define PETSC_HAVE_CXX_NAMESPACE

#endif
