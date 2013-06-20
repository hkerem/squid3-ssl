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
#ifndef SQUID_HTTPHDRCONTRANGE_H
#define SQUID_HTTPHDRCONTRANGE_H

class HttpHeader;

/* for HttpHdrRangeSpec */
#include "HttpHeaderRange.h"

/** HTTP Content-Range: header field */
class HttpHdrContRange
{

public:
    HttpHdrRangeSpec spec;
    int64_t elength;		/**< entity length, not content length */
};

/** \todo CLEANUP: Move httpHdrContRange* functions into the class methods */

HttpHdrContRange *httpHdrContRangeCreate(void);
HttpHdrContRange *httpHdrContRangeParseCreate(const char *crange_spec);
/** returns true if range is valid; inits HttpHdrContRange */
int httpHdrContRangeParseInit(HttpHdrContRange * crange, const char *crange_spec);
void httpHdrContRangeDestroy(HttpHdrContRange * crange);
HttpHdrContRange *httpHdrContRangeDup(const HttpHdrContRange * crange);
void httpHdrContRangePackInto(const HttpHdrContRange * crange, Packer * p);
/** inits with given spec */
void httpHdrContRangeSet(HttpHdrContRange *, HttpHdrRangeSpec, int64_t);
void httpHeaderAddContRange(HttpHeader *, HttpHdrRangeSpec, int64_t);

#endif /* SQUID_HTTPHDRCONTRANGE_H */
