/*
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
 */

#ifndef SQUID_AUTH_SCHEME_H
#define SQUID_AUTH_SCHEME_H

#if USE_AUTH

#include "Array.h"
#include "RefCount.h"

/**
 \defgroup AuthSchemeAPI	Authentication Scheme API
 \ingroup AuthAPI
 */

namespace Auth
{

class Config;

/**
 * \ingroup AuthAPI
 * \ingroup AuthSchemeAPI
 * \par
 * I represent an authentication scheme. For now my children
 * store the scheme metadata.
 * \par
 * Should we need multiple configs of a single scheme,
 * a new class should be made, and the config specific calls on Auth::Scheme moved to it.
 */
class Scheme : public RefCountable
{
public:
    typedef RefCount<Scheme> Pointer;
    typedef Vector<Scheme::Pointer>::iterator iterator;
    typedef Vector<Scheme::Pointer>::const_iterator const_iterator;

public:
    Scheme() : initialised (false) {};
    virtual ~Scheme() {};

    static void AddScheme(Scheme::Pointer);

    /**
     * Final termination of all authentication components.
     * To be used only on shutdown. All global pointers are released.
     * After this all schemes will appear completely unsupported
     * until a call to InitAuthModules().
     * Release the Auth::TheConfig handles instead to disable authentication
     * without terminiating all support.
     */
    static void FreeAll();

    /**
     * Locate an authentication scheme component by Name.
     */
    static Scheme::Pointer Find(const char *);

    /* per scheme methods */
    virtual char const *type() const = 0;
    virtual void shutdownCleanup() = 0;
    virtual Auth::Config *createConfig() = 0;

    // Not implemented
    Scheme(Scheme const &);
    Scheme &operator=(Scheme const&);

    static Vector<Scheme::Pointer> &GetSchemes();

protected:
    bool initialised;

private:
    static Vector<Scheme::Pointer> *_Schemes;
};

} // namespace Auth

#endif /* USE_AUTH */
#endif /* SQUID_AUTH_SCHEME_H */
