#ifndef SQUID_OS_HPUX_H
#define SQUID_OS_PHUX_H

#if _SQUID_HPUX_

/****************************************************************************
 *--------------------------------------------------------------------------*
 * DO *NOT* MAKE ANY CHANGES below here unless you know what you're doing...*
 *--------------------------------------------------------------------------*
 ****************************************************************************/

#if !defined(HAVE_GETPAGESIZE)
#define HAVE_GETPAGESIZE
#define getpagesize( )   sysconf(_SC_PAGE_SIZE)
#endif

/*
 * getrusage(...) not available on some HPUX
 */
#if !HAVE_GETRUSAGE
#define HAVE_GETRUSAGE 1
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif

/*
 * getpagesize(...) not available on HPUX
 */
#if !defined(HAVE_GETPAGESIZE)
#define HAVE_GETPAGESIZE
#define getpagesize( )   sysconf(_SC_PAGE_SIZE)
#endif

#endif /* _SQUID_HPUX_ */
#endif /* SQUID_OS_HPUX_H */
