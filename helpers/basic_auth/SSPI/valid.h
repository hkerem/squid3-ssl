/*
  NT_auth -  Version 2.0

  Modified to act as a Squid authenticator module.
  Returns OK for a successful authentication, or ERR upon error.

  Guido Serassio, Torino - Italy

  Uses code from -
    Antonino Iannella 2000
    Andrew Tridgell 1997
    Richard Sharpe 1996
    Bill Welliver 1999

 * Distributed freely under the terms of the GNU General Public License,
 * version 2. See the file COPYING for licensing details
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/

#ifndef _VALID_H_
#define _VALID_H_

#if _SQUID_CYGWIN_
#include <windows.h>
#endif
#include <lm.h>
#include "sspwin32.h"
#undef debug

/************* CONFIGURATION ***************/
/*
 * define this if you want debugging
 */
#ifndef DEBUG
#define DEBUG
#endif

#define safe_free(x)	if (x) { free(x); x = NULL; }

/* SMB User verification function */

#define NTV_NO_ERROR 0
#define NTV_SERVER_ERROR 1
#define NTV_GROUP_ERROR 2
#define NTV_LOGON_ERROR 3

#ifndef LOGON32_LOGON_NETWORK
#define LOGON32_LOGON_NETWORK       3
#endif

#define NTV_DEFAULT_DOMAIN "."

extern char * NTAllowedGroup;
extern char * NTDisAllowedGroup;
extern int UseDisallowedGroup;
extern int UseAllowedGroup;
extern int debug_enabled;
extern char Default_NTDomain[DNLEN+1];
extern const char * errormsg;

#include <sys/types.h>

/* Debugging stuff */

#ifdef __GNUC__			/* this is really a gcc-ism */
#ifdef DEBUG
#include <stdio.h>
#include <unistd.h>
static char *__foo;
#define debug(X...) if (debug_enabled) { \
                    fprintf(stderr,"nt_auth[%d](%s:%d): ", getpid(), \
                    ((__foo=strrchr(__FILE__,'/'))==NULL?__FILE__:__foo+1),\
                    __LINE__);\
                    fprintf(stderr,X); }
#else /* DEBUG */
#define debug(X...)		/* */
#endif /* DEBUG */
#else /* __GNUC__ */
static void
debug(char *format,...)
{
#ifdef DEBUG
#if _SQUID_MSWIN_
    if (debug_enabled) {
        va_list args;

        va_start(args,format);
        fprintf(stderr, "nt_auth[%d]: ",getpid());
        vfprintf(stderr, format, args);
        va_end(args);
    }
#endif /* _SQUID_MSWIN_ */
#endif /* DEBUG */
}
#endif /* __GNUC__ */

int Valid_User(char *,char *, char *);

#endif
