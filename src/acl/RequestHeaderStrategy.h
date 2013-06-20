
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
 *
 * Copyright (c) 2003, Robert Collins <robertc@squid-cache.org>
 */

#ifndef SQUID_ACLREQUESTHEADERSTRATEGY_H
#define SQUID_ACLREQUESTHEADERSTRATEGY_H
#include "acl/Acl.h"
#include "acl/Data.h"
#include "acl/Strategy.h"
#include "HttpRequest.h"
#include "acl/FilledChecklist.h"

template <http_hdr_type header>

class ACLRequestHeaderStrategy : public ACLStrategy<char const *>
{

public:
    virtual int match (ACLData<char const *> * &, ACLFilledChecklist *);
    virtual bool requiresRequest() const {return true;}

    static ACLRequestHeaderStrategy *Instance();
    /* Not implemented to prevent copies of the instance. */
    /* Not private to prevent brain dead g+++ warnings about
     * private constructors with no friends */
    ACLRequestHeaderStrategy(ACLRequestHeaderStrategy const &);

private:
    static ACLRequestHeaderStrategy *Instance_;
    ACLRequestHeaderStrategy() {}

    ACLRequestHeaderStrategy&operator=(ACLRequestHeaderStrategy const &);
};

template <http_hdr_type header>
int
ACLRequestHeaderStrategy<header>::match (ACLData<char const *> * &data, ACLFilledChecklist *checklist)
{
    char const *theHeader = checklist->request->header.getStr(header);

    if (NULL == theHeader)
        return 0;

    return data->match(theHeader);
}

template <http_hdr_type header>
ACLRequestHeaderStrategy<header> *
ACLRequestHeaderStrategy<header>::Instance()
{
    if (!Instance_)
        Instance_ = new ACLRequestHeaderStrategy<header>;

    return Instance_;
}

template <http_hdr_type header>
ACLRequestHeaderStrategy<header> * ACLRequestHeaderStrategy<header>::Instance_ = NULL;

#endif /* SQUID_REQUESTHEADERSTRATEGY_H */
