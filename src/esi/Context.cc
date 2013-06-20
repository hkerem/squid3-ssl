
/*
 * DEBUG: section 86    ESI processing
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
 ;  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"

/* MS Visual Studio Projects are monolithic, so we need the following
 * #if to exclude the ESI code from compile process when not needed.
 */
#if (USE_SQUID_ESI == 1)

#include "esi/Context.h"
#include "Store.h"
#include "client_side_request.h"

void
ESIContext::updateCachedAST()
{
    assert (http);
    assert (http->storeEntry());

    if (hasCachedAST()) {
        debugs(86, 5, "ESIContext::updateCachedAST: not updating AST cache for entry " <<
               http->storeEntry() << " from ESI Context " << this <<
               " as there is already a cached AST.");

        return;
    }

    ESIElement::Pointer treeToCache = tree->makeCacheable();
    debugs(86, 5, "ESIContext::updateCachedAST: Updating AST cache for entry " <<
           http->storeEntry() << " with current value " <<
           http->storeEntry()->cachedESITree.getRaw() << " to new value " <<
           treeToCache.getRaw());

    if (http->storeEntry()->cachedESITree.getRaw())
        http->storeEntry()->cachedESITree->finish();

    http->storeEntry()->cachedESITree = treeToCache;

    treeToCache = NULL;
}

bool
ESIContext::hasCachedAST() const
{
    assert (http);
    assert (http->storeEntry());

    if (http->storeEntry()->cachedESITree.getRaw()) {
        debugs(86, 5, "ESIContext::hasCachedAST: " << this <<
               " - Cached AST present in store entry " << http->storeEntry() << ".");
        return true;
    } else {
        debugs(86, 5, "ESIContext::hasCachedAST: " << this <<
               " - Cached AST not present in store entry " << http->storeEntry() << ".");
        return false;
    }
}

void
ESIContext::getCachedAST()
{
    if (cachedASTInUse)
        return;

    assert (hasCachedAST());

    assert (varState);

    parserState.popAll();

    tree = http->storeEntry()->cachedESITree->makeUsable (this, *varState);

    cachedASTInUse = true;
}

void
ESIContext::setErrorMessage(char const *anError)
{
    if (!errormessage)
        errormessage = xstrdup (anError);
}

#endif /* USE_SQUID_ESI == 1 */
