#ifndef SQUID_OS_SOLARIS_H
#define SQUID_OS_SOLARIS_H

#if _SQUID_SOLARIS_

/*
 * ugly hack. System headers require wcsstr, but don't define it.
 */
#include <wchar.h>
#ifdef wcsstr
#undef wcsstr
#endif /* wcsstr */
#define wcsstr wcswcs

/*
 * On Solaris 9 x86, gcc may includes a "fixed" set of old system
 * include files that is incompatible with the updated Solaris
 * header files.
 */
#if defined(i386) || defined(__i386)
#if !HAVE_PAD128_T
typedef union {
    long double	_q;
    int32_t		_l[4];
} pad128_t;
#endif
#if !HAVE_UPAD128_T
typedef union {
    long double	_q;
    uint32_t	_l[4];
} upad128_t;
#endif
#endif

/**
 * prototypes for system function missing from system includes
 * NP: sys/resource.h and sys/time.h are apparently order-dependant.
 */
#include <sys/time.h>
#include <sys/resource.h>
SQUIDCEXTERN int getrusage(int, struct rusage *);

/**
 * prototypes for system function missing from system includes
 * on some Solaris systems.
 */
SQUIDCEXTERN int getpagesize(void);
#if !defined(_XPG4_2) && !(defined(__EXTENSIONS__) || \
(!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)))
SQUIDCEXTERN int gethostname(char *, int);
#endif

/*
 * SunPro CC handles extern inline as inline, PLUS extern symbols.
 */
#if !defined(_SQUID_EXTERNNEW_) && defined(__SUNPRO_CC)
#define _SQUID_EXTERNNEW_ extern
#endif

/*
 * SunStudio CC does not define C++ portability API __FUNCTION__
 */
#if defined(__SUNPRO_CC) && !defined(__FUNCTION__)
#define __FUNCTION__ ""
#endif

/* Exclude CPPUnit tests from the allocator restrictions. */
/* BSD implementation uses these still */
#if defined(SQUID_UNIT_TEST)
#define SQUID_NO_STRING_BUFFER_PROTECT 1
#endif

/* Bug 2500: Solaris 10/11 require s6_addr* defines. */
//#define s6_addr8   _S6_un._S6_u8
//#define s6_addr16  _S6_un._S6_u16
#define s6_addr32  _S6_un._S6_u32

/* Bug 3057: Solaris 9 defines struct addrinfo with size_t instead of socklen_t
 *           this causes binary incompatibility on 64-bit systems.
 *  Fix this by bundling a copy of the OpenSolaris 10 netdb.h to use instead.
 */
#if defined(__sparcv9)
#include "compat/os/opensolaris_10_netdb.h"
#endif

/* Solaris 10 lacks SUN_LEN */
#if !defined(SUN_LEN)
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

/* Soaris 10 does not define POSIX AF_LOCAL, but does define the Unix name */
#if !defined(AF_LOCAL)
#define AF_LOCAL AF_UNIX
#endif

/* Solaris lacks paths.h by default */
#if HAVE_PATHS_H
#include <paths.h>
#endif
#if !defined(_PATH_DEVNULL)
#define _PATH_DEVNULL "/dev/null"
#endif

/* Solaris 10 does not define strsep() */
#include "compat/strsep.h"

#endif /* _SQUID_SOLARIS_ */
#endif /* SQUID_OS_SOALRIS_H */
