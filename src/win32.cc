/*
 * Windows support
 * AUTHOR: Guido Serassio <serassio@squid-cache.org>
 * inspired by previous work by Romeo Anghelache & Eric Stern.
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
#include "squid_windows.h"
#include "win32.h"

#if _SQUID_MSWIN_
#if HAVE_WIN32_PSAPI
#include <psapi.h>
#endif
#ifndef _MSWSOCK_
#include <mswsock.h>
#endif
#include <fde.h>

SQUIDCEXTERN LPCRITICAL_SECTION dbg_mutex;
void WIN32_ExceptionHandlerCleanup(void);
static LPTOP_LEVEL_EXCEPTION_FILTER Win32_Old_ExceptionHandler = NULL;

int WIN32_pipe(int handles[2])
{
    int new_socket;
    fde *F = NULL;

    Ip::Address localhost;
    Ip::Address handle0;
    Ip::Address handle1;
    struct addrinfo *AI = NULL;

    localhost.SetLocalhost();

    /* INET6: back-compatible: localhost pipes default to IPv4 unless set otherwise.
     *        it is blocked by untested helpers on many admins configs
     *        if this proves to be wrong it can die easily.
     */
    localhost.SetIPv4();

    handles[0] = handles[1] = -1;

    ++statCounter.syscalls.sock.sockets;

    handle0 = localhost;
    handle0.SetPort(0);
    handle0.GetAddrInfo(AI);

    if ((new_socket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) < 0)
        return -1;

    if (bind(new_socket, AI->ai_addr, AI->ai_addrlen) < 0 ||
            listen(new_socket, 1) < 0 || getsockname(new_socket, AI->ai_addr, &(AI->ai_addrlen) ) < 0 ||
            (handles[1] = socket(AI->ai_family, AI->ai_socktype, 0)) < 0) {
        closesocket(new_socket);
        return -1;
    }

    handle0 = *AI; // retrieve the new details returned by connect()

    handle1.SetPort(handle1.GetPort());
    handle1.GetAddrInfo(AI);

    if (connect(handles[1], AI->ai_addr, AI->ai_addrlen) < 0 ||
            (handles[0] = accept(new_socket, AI->ai_addr, &(AI->ai_addrlen)) ) < 0) {
        closesocket(handles[1]);
        handles[1] = -1;
        closesocket(new_socket);
        return -1;
    }

    closesocket(new_socket);

    F = &fd_table[handles[0]];
    F->local_addr = handle0;

    F = &fd_table[handles[1]];
    F->local_addr = localhost;
    handle1.NtoA(F->ipaddr, MAX_IPSTRLEN);
    F->remote_port = handle1.GetPort();

    return 0;
}

int WIN32_getrusage(int who, struct rusage *usage)
{
#if HAVE_WIN32_PSAPI

    if (WIN32_OS_version >= _WIN_OS_WINNT) {
        /* On Windows NT and later call PSAPI.DLL for process Memory */
        /* informations -- Guido Serassio                       */
        HANDLE hProcess;
        PROCESS_MEMORY_COUNTERS pmc;
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                               PROCESS_VM_READ,
                               FALSE, GetCurrentProcessId());
        {
            /* Microsoft CRT doesn't have getrusage function,  */
            /* so we get process CPU time information from PSAPI.DLL. */
            FILETIME ftCreate, ftExit, ftKernel, ftUser;

            if (GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser)) {
                int64_t *ptUser = (int64_t *)&ftUser;
                int64_t tUser64 = *ptUser / 10;
                int64_t *ptKernel = (int64_t *)&ftKernel;
                int64_t tKernel64 = *ptKernel / 10;
                usage->ru_utime.tv_sec =(long)(tUser64 / 1000000);
                usage->ru_stime.tv_sec =(long)(tKernel64 / 1000000);
                usage->ru_utime.tv_usec =(long)(tUser64 % 1000000);
                usage->ru_stime.tv_usec =(long)(tKernel64 % 1000000);
            } else {
                CloseHandle( hProcess );
                return -1;
            }
        }

        if (GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc))) {
            usage->ru_maxrss=(DWORD)(pmc.WorkingSetSize / getpagesize());
            usage->ru_majflt=pmc.PageFaultCount;
        } else {
            CloseHandle( hProcess );
            return -1;
        }

        CloseHandle( hProcess );
    }

#endif
    return 0;
}

int
Win32__WSAFDIsSet(int fd, fd_set FAR * set)
{
    fde *F = &fd_table[fd];
    SOCKET s = F->win32.handle;

    return __WSAFDIsSet(s, set);
}

LONG CALLBACK WIN32_ExceptionHandler(EXCEPTION_POINTERS* ep)
{
    EXCEPTION_RECORD* er;

    er = ep->ExceptionRecord;

    switch (er->ExceptionCode) {

    case EXCEPTION_ACCESS_VIOLATION:
        raise(SIGSEGV);
        break;

    case EXCEPTION_DATATYPE_MISALIGNMENT:

    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:

    case EXCEPTION_IN_PAGE_ERROR:
        death(SIGBUS);
        break;

    default:
        break;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

void WIN32_ExceptionHandlerInit()
{
#if !defined(_DEBUG)

    if (Win32_Old_ExceptionHandler == NULL)
        Win32_Old_ExceptionHandler = SetUnhandledExceptionFilter(WIN32_ExceptionHandler);

#endif
}

void WIN32_ExceptionHandlerCleanup()
{
    if (Win32_Old_ExceptionHandler != NULL)
        SetUnhandledExceptionFilter(Win32_Old_ExceptionHandler);
}

#endif /* SQUID_MSWIN_ */
