
/*
 * -----------------------------------------------------------------------------
 *
 * Author: Markus Moeller (markus_moeller at compuserve.com)
 *
 * Copyright (C) 2007 Markus Moeller. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * -----------------------------------------------------------------------------
 */

#include "config.h"
#include "util.h"

#ifdef HAVE_LDAP

#include "support.h"

struct gdstruct *init_gd(void);

struct gdstruct *
init_gd(void) {
    struct gdstruct *gdsp;
    gdsp = (struct gdstruct *) xmalloc(sizeof(struct gdstruct));
    gdsp->group = NULL;
    gdsp->domain = NULL;
    gdsp->next = NULL;
    return gdsp;
}

char *utf8dup(struct main_args *margs);

char *
utf8dup(struct main_args *margs)
{
    int c = 0, s;
    size_t n;
    char *src;
    unsigned char *p, *dupp;

    src = margs->glist;
    if (!src)
        return NULL;
    for (n = 0; n < strlen(src); n++)
        if ((unsigned char) src[n] > 127)
            c++;
    if (c != 0) {
        p = (unsigned char *) xmalloc(strlen(src) + c);
        dupp = p;
        for (n = 0; n < strlen(src); n++) {
            s = (unsigned char) src[n];
            if (s > 127 && s < 192) {
                *p = 194;
                p++;
                *p = s;
            } else if (s > 191 && s < 256) {
                *p = 195;
                p++;
                *p = s - 64;
            } else
                *p = s;
            p++;
        }
        *p = '\0';
        debug((char *) "%s| %s: INFO: Group %s as UTF-8: %s\n", LogTime(), PROGRAM, src, dupp);
        return (char *) dupp;
    } else
        return xstrdup(src);
}

char *hex_utf_char(struct main_args *margs, int flag);
/*
 * UTF8    = UTF1 / UTFMB
 * UTFMB   = UTF2 / UTF3 / UTF4
 *
 * UTF0    = %x80-BF
 * UTF1    = %x00-7F
 * UTF2    = %xC2-DF UTF0
 * UTF3    = %xE0 %xA0-BF UTF0 / %xE1-EC 2(UTF0) /
 * %xED %x80-9F UTF0 / %xEE-EF 2(UTF0)
 * UTF4    = %xF0 %x90-BF 2(UTF0) / %xF1-F3 3(UTF0) /
 * %xF4 %x80-8F 2(UTF0)
 *
 * http://www.utf8-chartable.de/unicode-utf8-table.pl
 */

char *
hex_utf_char(struct main_args *margs, int flag)
{
    char *up;
    char *upd;
    char *ul;
    int a, n, nl, ival, ichar;
    int iUTF2, iUTF3, iUTF4;

    if (flag) {
        up = margs->ulist;
    } else {
        up = margs->tlist;
    }

    if (!up)
        return NULL;

    upd = strrchr(up, '@');
    if (upd)
        a = upd - up;
    else
        a = strlen(up);

    ul = (char *) xmalloc(strlen(up));
    n = 0;
    nl = 0;
    iUTF2 = 0;
    iUTF3 = 0;
    iUTF4 = 0;

    while (n < (int) strlen(up)) {
        if (flag && n == a)
            break;
        if (up[n] == '@') {
            ul[nl] = '@';
            nl++;
            n++;
            continue;
        }
        ival = up[n];
        if (ival > 64 && ival < 71)
            ichar = (ival - 55) * 16;
        else if (ival > 96 && ival < 103)
            ichar = (ival - 87) * 16;
        else if (ival > 47 && ival < 58)
            ichar = (ival - 48) * 16;
        else {
            debug((char *) "%s| %s: WARNING: Invalid Hex value %c\n", LogTime(), PROGRAM, ival);
            if (ul)
                xfree(ul);
            return NULL;
        }


        if (n == a - 1) {
            debug((char *) "%s| %s: WARNING: Invalid Hex UTF-8 string %s\n", LogTime(), PROGRAM, up);
            if (ul)
                xfree(ul);
            return NULL;
        }
        n++;
        ival = up[n];
        if (ival > 64 && ival < 71)
            ichar = ichar + ival - 55;
        else if (ival > 96 && ival < 103)
            ichar = ichar + ival - 87;
        else if (ival > 47 && ival < 58)
            ichar = ichar + ival - 48;
        else {
            debug((char *) "%s| %s: WARNING: Invalid Hex value %c\n", LogTime(), PROGRAM, ival);
            if (ul)
                xfree(ul);
            return NULL;
        }

        if (iUTF2) {
            if (iUTF2 == 0xC2 && ichar > 0x7F && ichar < 0xC0) {
                iUTF2 = 0;
                ul[nl - 1] = ichar;
            } else if (iUTF2 == 0xC3 && ichar > 0x7F && ichar < 0xC0) {
                iUTF2 = 0;
                ul[nl - 1] = ichar + 64;
            } else if (iUTF2 > 0xC3 && iUTF2 < 0xE0 && ichar > 0x7F && ichar < 0xC0) {
                iUTF2 = 0;
                ul[nl] = ichar;
                nl++;
            } else {
                iUTF2 = 0;
                ul[nl] = ichar;
                ul[nl + 1] = '\0';
                debug((char *) "%s| %s: WARNING: Invalid UTF-8 sequence for Unicode %s\n", LogTime(), PROGRAM, ul);
                if (ul)
                    xfree(ul);
                return NULL;
            }
        } else if (iUTF3) {
            if (iUTF3 == 0xE0 && ichar > 0x9F && ichar < 0xC0) {
                iUTF3 = 1;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF3 > 0xE0 && iUTF3 < 0xED && ichar > 0x7F && ichar < 0xC0) {
                iUTF3 = 2;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF3 == 0xED && ichar > 0x7F && ichar < 0xA0) {
                iUTF3 = 3;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF3 > 0xED && iUTF3 < 0xF0 && ichar > 0x7F && ichar < 0xC0) {
                iUTF3 = 4;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF3 > 0 && iUTF3 < 5 && ichar > 0x7F && ichar < 0xC0) {
                iUTF3 = 0;
                ul[nl] = ichar;
                nl++;
            } else {
                iUTF3 = 0;
                ul[nl] = ichar;
                ul[nl + 1] = '\0';
                debug((char *) "%s| %s: WARNING: Invalid UTF-8 sequence for Unicode %s\n", LogTime(), PROGRAM, ul);
                if (ul)
                    xfree(ul);
                return NULL;
            }
        } else if (iUTF4) {
            if (iUTF4 == 0xF0 && ichar > 0x8F && ichar < 0xC0) {
                iUTF4 = 1;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF4 > 0xF0 && iUTF3 < 0xF4 && ichar > 0x7F && ichar < 0xC0) {
                iUTF4 = 2;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF4 == 0xF4 && ichar > 0x7F && ichar < 0x90) {
                iUTF4 = 3;
                ul[nl] = ichar;
                nl++;
            } else if (iUTF4 > 0 && iUTF4 < 5 && ichar > 0x7F && ichar < 0xC0) {
                if (iUTF4 == 4)
                    iUTF4 = 0;
                else
                    iUTF4 = 4;
                ul[nl] = ichar;
                nl++;
            } else {
                iUTF4 = 0;
                ul[nl] = ichar;
                ul[nl + 1] = '\0';
                debug((char *) "%s| %s: WARNING: Invalid UTF-8 sequence for Unicode %s\n", LogTime(), PROGRAM, ul);
                if (ul)
                    xfree(ul);
                return NULL;
            }
        } else if (ichar < 0x80) {
            /* UTF1 */
            ul[nl] = ichar;
            nl++;
        } else if (ichar > 0xC1 && ichar < 0xE0) {
            /* UTF2 (Latin) */
            iUTF2 = ichar;
            ul[nl] = ichar;
            nl++;
        } else if (ichar > 0xDF && ichar < 0xF0) {
            /* UTF3 */
            iUTF3 = ichar;
            ul[nl] = ichar;
            nl++;
        } else if (ichar > 0xEF && ichar < 0xF5) {
            /* UTF4 */
            iUTF4 = ichar;
            ul[nl] = ichar;
            nl++;
        } else {
            ul[nl] = ichar;
            ul[nl + 1] = '\0';
            debug((char *) "%s| %s: WARNING: Invalid UTF-8 sequence for Unicode %s\n", LogTime(), PROGRAM, ul);
            if (ul)
                xfree(ul);
            return NULL;
        }
        n++;
    }

    ul[nl] = '\0';
    if (iUTF2 || iUTF3 || iUTF4) {
        debug((char *) "%s| %s: INFO: iUTF2: %d iUTF3: %d iUTF4: %d\n", LogTime(), PROGRAM, iUTF2, iUTF3, iUTF4);
        debug((char *) "%s| %s: WARNING: Invalid UTF-8 sequence for Unicode %s\n", LogTime(), PROGRAM, ul);
        if (ul)
            xfree(ul);
        return NULL;
    }
    if (flag && upd)
        ul = strcat(ul, upd);
    return ul;
}


int
create_gd(struct main_args *margs)
{
    char *gp, *dp;
    char *hp1, *hp2, *up;
    char *p;
    struct gdstruct *gdsp = NULL, *gdspn = NULL;
    /*
     *  Group list format:
     *
     *     glist=Pattern1[:Pattern2]
     *
     *     Pattern=Group           Group for all domains(including non Kerberos domains using ldap url options) if no
     *                             other group definition for domain exists or users without
     *                             domain information.
     *                             gdstruct.domain=NULL, gdstruct.group=Group
     *
     *  or Pattern=Group@          Group for all Kerberos domains if no other group definition
     *                             exists
     *                             gdstruct.domain="", gdstruct.group=Group
     *
     *  or Pattern=Group@Domain    Group for a specific Kerberos domain
     *                             gdstruct.domain=Domain, gdstruct.group=Group
     *
     *
     */
    hp1 = hex_utf_char(margs, 0);
    hp2 = hex_utf_char(margs, 1);
    up = utf8dup(margs);
    p = up;
    if (hp1) {
        if (hp2) {
            if (up) {
                p = (char *) xmalloc(strlen(up) + strlen(hp1) + strlen(hp2) + 2);
                strcpy(p, up);
                strcat(p, ":");
                strcat(p, hp1);
                strcat(p, ":");
                strcat(p, hp2);
            } else {
                p = (char *) xmalloc(strlen(hp1) + strlen(hp2) + 1);
                strcpy(p, hp1);
                strcat(p, ":");
                strcat(p, hp2);
            }
        } else {
            if (up) {
                p = (char *) xmalloc(strlen(up) + strlen(hp1) + 1);
                strcpy(p, up);
                strcat(p, ":");
                strcat(p, hp1);
            } else
                p = hp1;
        }
    } else {
        if (hp2) {
            if (up) {
                p = (char *) xmalloc(strlen(up) + strlen(hp2) + 1);
                strcpy(p, up);
                strcat(p, ":");
                strcat(p, hp2);
            } else
                p = hp2;
        } else
            p = up;
    }
    gp = p;
    debug((char *) "%s| %s: INFO: Group list %s\n", LogTime(), PROGRAM, p ? p : "NULL");
    dp = NULL;

    if (!p) {
        debug((char *) "%s| %s: ERROR: No groups defined.\n", LogTime(), PROGRAM);
        return (1);
    }
    while (*p) {		/* loop over group list */
        if (*p == '\n' || *p == '\r') {		/* Ignore CR and LF if exist */
            p++;
            continue;
        }
        if (*p == '@') {	/* end of group name - start of domain name */
            if (p == gp) {	/* empty group name not allowed */
                debug((char *) "%s| %s: ERROR: No group defined for domain %s\n", LogTime(), PROGRAM, p);
                return (1);
            }
            *p = '\0';
            p++;
            gdsp = init_gd();
            gdsp->group = gp;
            if (gdspn)		/* Have already an existing structure */
                gdsp->next = gdspn;
            dp = p;		/* after @ starts new domain name */
        } else if (*p == ':') {	/* end of group name or end of domain name */
            if (p == gp) {	/* empty group name not allowed */
                debug((char *) "%s| %s: ERROR: No group defined for domain %s\n", LogTime(), PROGRAM, p);
                return (1);
            }
            *p = '\0';
            p++;
            if (dp) {		/* end of domain name */
                gdsp->domain = xstrdup(dp);
                dp = NULL;
            } else {		/* end of group name and no domain name */
                gdsp = init_gd();
                gdsp->group = gp;
                if (gdspn)	/* Have already an existing structure */
                    gdsp->next = gdspn;
            }
            gdspn = gdsp;
            gp = p;		/* after : starts new group name */
            debug((char *) "%s| %s: INFO: Group %s  Domain %s\n", LogTime(), PROGRAM, gdsp->group, gdsp->domain ? gdsp->domain : "NULL");
        } else
            p++;
    }
    if (p == gp) {		/* empty group name not allowed */
        debug((char *) "%s| %s: ERROR: No group defined for domain %s\n", LogTime(), PROGRAM, p);
        return (1);
    }
    if (dp) {			/* end of domain name */
        gdsp->domain = xstrdup(dp);
    } else {			/* end of group name and no domain name */
        gdsp = init_gd();
        gdsp->group = gp;
        if (gdspn)		/* Have already an existing structure */
            gdsp->next = gdspn;
    }
    debug((char *) "%s| %s: INFO: Group %s  Domain %s\n", LogTime(), PROGRAM, gdsp->group, gdsp->domain ? gdsp->domain : "NULL");

    margs->groups = gdsp;
    return (0);
}
#endif
