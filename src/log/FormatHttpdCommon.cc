/*
 * $Id$
 *
 * DEBUG: section 46    Access Log - Apache common format
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
 */

#include "config.h"
#include "AccessLogEntry.h"
#include "format/Quoting.h"
#include "format/Tokens.h"
#include "log/File.h"
#include "log/Formats.h"
#include "SquidTime.h"

void
Log::Format::HttpdCommon(AccessLogEntry * al, Logfile * logfile)
{
    char clientip[MAX_IPSTRLEN];
    const char *user_auth = ::Format::QuoteUrlEncodeUsername(al->cache.authuser);
    const char *user_ident = ::Format::QuoteUrlEncodeUsername(al->cache.rfc931);

    logfilePrintf(logfile, "%s %s %s [%s] \"%s %s %s/%d.%d\" %d %"PRId64" %s%s:%s%s",
                  al->cache.caddr.NtoA(clientip,MAX_IPSTRLEN),
                  user_ident ? user_ident : dash_str,
                  user_auth ? user_auth : dash_str,
                  Time::FormatHttpd(squid_curtime),
                  al->_private.method_str,
                  al->url,
                  AnyP::ProtocolType_str[al->http.version.protocol],
                  al->http.version.major, al->http.version.minor,
                  al->http.code,
                  al->cache.replySize,
                  ::Format::log_tags[al->cache.code],
                  al->http.statusSfx(),
                  hier_code_str[al->hier.code],
                  (Config.onoff.log_mime_hdrs?"":"\n"));

    safe_free(user_auth);
    safe_free(user_ident);

    if (Config.onoff.log_mime_hdrs) {
        char *ereq = ::Format::QuoteMimeBlob(al->headers.request);
        char *erep = ::Format::QuoteMimeBlob(al->headers.reply);
        logfilePrintf(logfile, " [%s] [%s]\n", ereq, erep);
        safe_free(ereq);
        safe_free(erep);
    }
}
