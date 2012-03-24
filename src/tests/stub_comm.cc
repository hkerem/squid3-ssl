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
#include "comm.h"
#include "comm/Connection.h"
#include "comm/Loops.h"
#include "CommRead.h"
#include "fde.h"

DeferredReadManager::~DeferredReadManager()
{
    /* no networked tests yet */
}

DeferredRead::DeferredRead (DeferrableRead *, void *, CommRead const &)
{
    fatal ("Not implemented");
}

void
DeferredReadManager::delayRead(DeferredRead const &aRead)
{
    fatal ("Not implemented");
}

void
DeferredReadManager::kickReads(int const count)
{
    fatal ("Not implemented");
}

void
comm_read(const Comm::ConnectionPointer &conn, char *buf, int size, IOCB *handler, void *handler_data)
{
    fatal ("Not implemented");
}

void
comm_read(const Comm::ConnectionPointer &conn, char*, int, AsyncCall::Pointer &callback)
{
    fatal ("Not implemented");
}

/* should be in stub_CommRead */
#include "CommRead.h"
CommRead::CommRead(const Comm::ConnectionPointer &, char *buf, int len, AsyncCall::Pointer &callback)
{
    fatal ("Not implemented");
}

CommRead::CommRead ()
{
    fatal ("Not implemented");
}

void
commSetCloseOnExec(int fd)
{
    /* for tests... ignore */
}

#if 0
void
Comm::SetSelect(int fd, unsigned int type, PF * handler, void *client_data, time_t timeout)
{
    /* all test code runs synchronously at the moment */
}

void
Comm::QuickPollRequired()
{
    /* for tests ... ignore */
}
#endif

int
ignoreErrno(int ierrno)
{
    fatal ("Not implemented");
    return -1;
}

void
commUnsetFdTimeout(int fd)
{
    fatal ("Not implemented");
}

int
commSetNonBlocking(int fd)
{
    fatal ("Not implemented");
    return COMM_ERROR;
}

int
commUnsetNonBlocking(int fd)
{
    fatal ("Not implemented");
    return -1;
}

void
comm_init(void)
{
    fd_table =(fde *) xcalloc(Squid_MaxFD, sizeof(fde));

    /* Keep a few file descriptors free so that we don't run out of FD's
     * after accepting a client but before it opens a socket or a file.
     * Since Squid_MaxFD can be as high as several thousand, don't waste them */
    RESERVED_FD = min(100, Squid_MaxFD / 4);
}

/* MinGW needs also a stub of _comm_close() */
void
_comm_close(int fd, char const *file, int line)
{
    fatal ("Not implemented");
}

int
commSetTimeout(int fd, int timeout, AsyncCall::Pointer& callback)
{
    fatal ("Not implemented");
    return -1;
}

int
comm_open_uds(int sock_type, int proto, struct sockaddr_un* addr, int flags)
{
    fatal ("Not implemented");
    return -1;
}

void
comm_write(int fd, const char *buf, int size, AsyncCall::Pointer &callback, FREE * free_func)
{
    fatal ("Not implemented");
}
