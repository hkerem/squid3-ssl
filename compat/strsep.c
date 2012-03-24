/* Copyright (C) 2004 Free Software Foundation, Inc.
 * Written by Yoann Vandoorselaere <yoann@prelude-ids.org>
 *
 * The file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include "squid.h"
#include "compat/strsep.h"

#include <string.h>

char *
strsep(char **stringp, const char *delim)
{
    char *start = *stringp;
    char *ptr;

    if (!start)
        return NULL;

    if (!*delim)
        ptr = start + strlen (start);
    else {
        ptr = strpbrk (start, delim);
        if (!ptr) {
            *stringp = NULL;
            return start;
        }
    }

    *ptr = '\0';
    *stringp = ptr + 1;

    return start;
}
