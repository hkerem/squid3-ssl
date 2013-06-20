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

#ifndef SQUID_ACLHTTPSTATUS_H
#define SQUID_ACLHTTPSTATUS_H

#include "acl/Acl.h"
#include "acl/Checklist.h"
#include "splay.h"

/// \ingroup ACLAPI
struct acl_httpstatus_data {
    int status1, status2;
    acl_httpstatus_data(int);
    acl_httpstatus_data(int, int);
    void toStr(char* buf, int len) const;

    static int compare(acl_httpstatus_data* const& a, acl_httpstatus_data* const& b);
};

/// \ingroup ACLAPI
class ACLHTTPStatus : public ACL
{

public:
    MEMPROXY_CLASS(ACLHTTPStatus);

    ACLHTTPStatus(char const *);
    ACLHTTPStatus(ACLHTTPStatus const &);
    ~ACLHTTPStatus();
    ACLHTTPStatus&operator=(ACLHTTPStatus const &);

    virtual ACL *clone()const;
    virtual char const *typeString() const;
    virtual void parse();
    virtual int match(ACLChecklist *checklist);
    virtual wordlist *dump() const;
    virtual bool empty () const;
    virtual bool requiresReply() const { return true; }

protected:
    static Prototype RegistryProtoype;
    static ACLHTTPStatus RegistryEntry_;
    SplayNode<acl_httpstatus_data*> *data;
    char const *class_;
};

MEMPROXY_CLASS_INLINE(ACLHTTPStatus);

#endif /* SQUID_ACLHTTPSTATUS_H */
