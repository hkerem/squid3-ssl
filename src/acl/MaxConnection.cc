/*
 * DEBUG: section 28    Access Control
 * AUTHOR: Duane Wessels
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 *
 * Copyright (c) 2003, Robert Collins <robertc@squid-cache.org>
 */

#include "squid.h"
#include "acl/FilledChecklist.h"
#include "acl/MaxConnection.h"
#include "cache_cf.h"
#include "client_db.h"
#include "Debug.h"
#include "SquidConfig.h"
#include "wordlist.h"

ACL *
ACLMaxConnection::clone() const
{
    return new ACLMaxConnection(*this);
}

ACLMaxConnection::ACLMaxConnection (char const *theClass) : class_ (theClass), limit(-1)
{}

ACLMaxConnection::ACLMaxConnection (ACLMaxConnection const & old) :class_ (old.class_), limit (old.limit)
{}

ACLMaxConnection::~ACLMaxConnection()
{}

char const *
ACLMaxConnection::typeString() const
{
    return class_;
}

bool
ACLMaxConnection::empty () const
{
    return false;
}

bool
ACLMaxConnection::valid () const
{
    return limit > 0;
}

void
ACLMaxConnection::parse()
{
    char *t = strtokFile();

    if (!t)
        return;

    limit = (atoi (t));

    /* suck out file contents */
    // ignore comments
    bool ignore = false;
    while ((t = strtokFile())) {
        ignore |= (*t != '#');

        if (ignore)
            continue;

        debugs(89, DBG_CRITICAL, "WARNING: max_conn only accepts a single limit value.");
        limit = 0;
    }
}

int
ACLMaxConnection::match(ACLChecklist *checklist)
{
    return clientdbEstablished(Filled(checklist)->src_addr, 0) > limit ? 1 : 0;
}

wordlist *
ACLMaxConnection::dump() const
{
    if (!limit)
        return NULL;

    wordlist *W = NULL;

    char buf[32];

    snprintf(buf, sizeof(buf), "%d", limit);

    wordlistAdd(&W, buf);

    return W;
}

void
ACLMaxConnection::prepareForUse()
{
    if (0 != Config.onoff.client_db)
        return;

    debugs(22, DBG_CRITICAL, "WARNING: 'maxconn' ACL (" << name << ") won't work with client_db disabled");
}
