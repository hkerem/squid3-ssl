/*
 * DEBUG: section 29    Authenticator
 * AUTHOR: Robert Collins
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
 * Copyright (c) 2004, Robert Collins <robertc@squid-cache.org>
 */

#include "squid.h"
#include "auth/Scheme.h"
#include "auth/Gadgets.h"
#include "auth/Config.h"
#include "globals.h"

Vector<Auth::Scheme::Pointer> *Auth::Scheme::_Schemes = NULL;

void
Auth::Scheme::AddScheme(Auth::Scheme::Pointer instance)
{
    iterator i = GetSchemes().begin();

    while (i != GetSchemes().end()) {
        assert(strcmp((*i)->type(), instance->type()) != 0);
        ++i;
    }

    GetSchemes().push_back(instance);
}

Auth::Scheme::Pointer
Auth::Scheme::Find(const char *typestr)
{
    for (iterator i = GetSchemes().begin(); i != GetSchemes().end(); ++i) {
        if (strcmp((*i)->type(), typestr) == 0)
            return *i;
    }

    return Auth::Scheme::Pointer(NULL);
}

Vector<Auth::Scheme::Pointer> &
Auth::Scheme::GetSchemes()
{
    if (!_Schemes)
        _Schemes = new Vector<Auth::Scheme::Pointer>;

    return *_Schemes;
}

/**
 * Called when a graceful shutdown is to occur of each scheme module.
 * On completion the auth components are to be considered deleted.
 * None will be available globally. Some may remain around for their
 * currently active connections to close, but only those active
 * connections will retain pointers to them.
 */
void
Auth::Scheme::FreeAll()
{
    assert(shutting_down);

    while (GetSchemes().size()) {
        Auth::Scheme::Pointer scheme = GetSchemes().back();
        GetSchemes().pop_back();
        scheme->shutdownCleanup();
    }
}
