#ifndef SQUID_OS_MACOSX_H
#define SQUID_OS_MACOSX_H

#if _SQUID_APPLE_

/****************************************************************************
 *--------------------------------------------------------------------------*
 * DO *NOT* MAKE ANY CHANGES below here unless you know what you're doing...*
 *--------------------------------------------------------------------------*
 ****************************************************************************/

/*
 *   This OS has at least one version that defines these as private
 *   kernel macros commented as being 'non-standard'.
 *   We need to use them, much nicer than the OS-provided __u*_*[]
 */
//#define s6_addr8  __u6_addr.__u6_addr8
//#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32

#include "compat/cmsg.h"

// MacOS GCC 4.0.1 and 4.2.1 supply __GNUC_GNU_INLINE__ but do not actually define  __attribute__((gnu_inline))
#if defined(__cplusplus) && !defined(_SQUID_EXTERNNEW_)
#define _SQUID_EXTERNNEW_ extern inline
#endif

#endif /* _SQUID_APPLE_ */
#endif /* SQUID_OS_MACOSX_H */
