/*
 * $Id$
 *
 * DEBUG: section 84    Helper process maintenance
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
 */

#include "squid.h"
#include "comm/Connection.h"
#include "MemObject.h"
#include "HttpReply.h"
#if USE_DELAY_POOLS
#include "DelayPools.h"
#endif

RemovalPolicy * mem_policy = NULL;

int64_t
MemObject::endOffset() const
{
    return data_hdr.endOffset();
}

void
MemObject::trimSwappable()
{
    fatal ("Not implemented");
}

void
MemObject::trimUnSwappable()
{
    fatal ("Not implemented");
}

int64_t
MemObject::policyLowestOffsetToKeep(bool swap) const
{
    fatal ("Not implemented");
    return -1;
}

MemObject::MemObject (char const *, char const *)
{}

HttpReply const *
MemObject::getReply() const
{
    return NULL;
}

void
MemObject::reset()
{
    fatal ("Not implemented");
}

void
MemObject::delayRead(DeferredRead const &aRead)
{
    fatal ("Not implemented");
}

bool
MemObject::readAheadPolicyCanRead() const
{
    fatal ("Not implemented");
    return false;
}

void
MemObject::setNoDelay(bool const newValue)
{
    fatal ("Not implemented");
}

MemObject::~MemObject()
{
    fatal ("Not implemented");
}

int
MemObject::mostBytesWanted(int max, bool ignoreDelayPools) const
{
    fatal ("Not implemented");
    return -1;
}

#if USE_DELAY_POOLS
DelayId
MemObject::mostBytesAllowed() const
{
    DelayId result;
    fatal ("Not implemented");
    return result;
}
#endif

void
MemObject::unlinkRequest()
{
    fatal ("Not implemented");
}

void
MemObject::write(StoreIOBuffer writeBuffer, STMCB *callback, void *callbackData)
{
    PROF_start(MemObject_write);
    debugs(19, 6, "memWrite: offset " << writeBuffer.offset << " len " << writeBuffer.length);

    /* the offset is into the content, not the headers */
    writeBuffer.offset += (_reply ? _reply->hdr_sz : 0);

    /* We don't separate out mime headers yet, so ensure that the first
     * write is at offset 0 - where they start
     */
    assert (data_hdr.endOffset() || writeBuffer.offset == 0);

    assert (data_hdr.write (writeBuffer));
    callback (callbackData, writeBuffer);
    PROF_stop(MemObject_write);
}

void
MemObject::replaceHttpReply(HttpReply *newrep)
{
    fatal ("Not implemented");
}

int64_t
MemObject::lowestMemReaderOffset() const
{
    fatal ("Not implemented");
    return 0;
}

void
MemObject::kickReads()
{
    fatal ("Not implemented");
}

int64_t
MemObject::objectBytesOnDisk() const
{
    fatal ("MemObject.cc required.");
    return 0;
}

bool
MemObject::isContiguous() const
{
    fatal ("MemObject.cc required.");
    return false;
}

int64_t
MemObject::expectedReplySize() const
{
    fatal ("MemObject.cc required.");
    return 0;
}

void
MemObject::resetUrls(char const*, char const*)
{
    fatal ("MemObject.cc required.");
}

void
MemObject::markEndOfReplyHeaders()
{
    fatal ("MemObject.cc required.");
}

size_t
MemObject::inUseCount()
{
    fatal ("MemObject.cc required.");
    return 0;
}

int64_t
MemObject::availableForSwapOut() const
{
    fatal ("MemObject.cc required.");
    return 0;
}
