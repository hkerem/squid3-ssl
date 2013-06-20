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

#ifndef SQUID_MEMOBJECT_H
#define SQUID_MEMOBJECT_H

#include "CommRead.h"
#include "dlink.h"
#include "HttpRequestMethod.h"
#include "RemovalPolicy.h"
#include "stmem.h"
#include "StoreIOBuffer.h"
#include "StoreIOState.h"

#if USE_DELAY_POOLS
#include "DelayId.h"
#endif

typedef void STMCB (void *data, StoreIOBuffer wroteBuffer);

class store_client;
class HttpRequest;
class HttpReply;

class MemObject
{

public:
    static size_t inUseCount();
    MEMPROXY_CLASS(MemObject);

    void dump() const;
    MemObject(char const *, char const *);
    ~MemObject();

    /// replaces construction-time URLs with correct ones; see hidden_mem_obj
    void resetUrls(char const *aUrl, char const *aLog_url);

    void write(StoreIOBuffer, STMCB *, void *);
    void unlinkRequest();
    HttpReply const *getReply() const;
    void replaceHttpReply(HttpReply *newrep);
    void stat (MemBuf * mb) const;
    int64_t endOffset () const;
    void markEndOfReplyHeaders(); ///< sets _reply->hdr_sz to endOffset()
    /// negative if unknown; otherwise, expected object_sz, expected endOffset
    /// maximum, and stored reply headers+body size (all three are the same)
    int64_t expectedReplySize() const;
    int64_t size() const;
    void reset();
    int64_t lowestMemReaderOffset() const;
    bool readAheadPolicyCanRead() const;
    void addClient(store_client *);
    /* XXX belongs in MemObject::swapout, once swaphdrsz is managed
     * better
     */
    int64_t objectBytesOnDisk() const;
    int64_t policyLowestOffsetToKeep(bool swap) const;
    int64_t availableForSwapOut() const; ///< buffered bytes we have not swapped out yet
    void trimSwappable();
    void trimUnSwappable();
    bool isContiguous() const;
    int mostBytesWanted(int max, bool ignoreDelayPools) const;
    void setNoDelay(bool const newValue);
#if USE_DELAY_POOLS
    DelayId mostBytesAllowed() const;
#endif

#if URL_CHECKSUM_DEBUG

    void checkUrlChecksum() const;
#endif

    HttpRequestMethod method;
    char *url;
    mem_hdr data_hdr;
    int64_t inmem_lo;
    dlink_list clients;

    /** \todo move into .cc or .cci */
    size_t clientCount() const {return nclients;}

    bool clientIsFirst(void *sc) const {return (clients.head && sc == clients.head->data);}

    int nclients;

    class SwapOut
    {

    public:
        int64_t queue_offset; ///< number of bytes sent to SwapDir for writing
        StoreIOState::Pointer sio;

        /// Decision states for StoreEntry::swapoutPossible() and related code.
        typedef enum { swNeedsCheck = 0, swImpossible = -1, swPossible = +1 } Decision;
        Decision decision; ///< current decision state
    };

    SwapOut swapout;

    /* Read only - this reply must be preserved by store clients */
    /* The original reply. possibly with updated metadata. */
    HttpRequest *request;

    struct timeval start_ping;
    IRCB *ping_reply_callback;
    void *ircb_data;

    struct {
        STABH *callback;
        void *data;
    } abort;
    char *log_url;
    RemovalPolicyNode repl;
    int id;
    int64_t object_sz;
    size_t swap_hdr_sz;
#if URL_CHECKSUM_DEBUG

    unsigned int chksum;
#endif

    const char *vary_headers;

    void delayRead(DeferredRead const &);
    void kickReads();

private:
    HttpReply *_reply;

    DeferredReadManager deferredReads;
};

MEMPROXY_CLASS_INLINE(MemObject);

/** global current memory removal policy */
extern RemovalPolicy *mem_policy;

#endif /* SQUID_MEMOBJECT_H */
