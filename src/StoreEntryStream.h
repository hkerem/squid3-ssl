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

#ifndef SQUID_STORE_ENTRY_STREAM_H
#define SQUID_STORE_ENTRY_STREAM_H

#include "Store.h"

#if HAVE_OSTREAM
#include <ostream>
#endif

/*
 * This class provides a streambuf interface for writing
 * to StoreEntries. Typical use is via a StoreEntryStream
 * rather than direct manipulation
 */

class StoreEntryStreamBuf : public std::streambuf
{

public:
    StoreEntryStreamBuf(StoreEntry *anEntry) : theEntry(anEntry) {

        theEntry->lock();
        theEntry->buffer();
    }

    ~StoreEntryStreamBuf() {
        theEntry->unlock();
    }

protected:
    /* flush the current buffer and the character that is overflowing
     * to the store entry.
     */
    virtual int_type overflow(int_type aChar = traits_type::eof()) {
        std::streamsize pending(pptr() - pbase());

        if (pending && sync ())
            return traits_type::eof();

        if (aChar != traits_type::eof()) {
            // NP: cast because GCC promotes int_type to 32-bit type
            //     std::basic_streambuf<char>::int_type {aka int}
            //     despite the definition with 8-bit type value.
            char chars[1] = {char(aChar)};

            if (aChar != traits_type::eof())
                theEntry->append(chars, 1);
        }

        pbump (-pending);  // Reset pptr().
        return aChar;
    }

    /* push the buffer to the store */
    virtual int sync() {
        std::streamsize pending(pptr() - pbase());

        if (pending)
            theEntry->append(pbase(), pending);

        theEntry->flush();

        return 0;
    }

    /* write multiple characters to the store entry
     * - this is an optimisation method.
     */
    virtual std::streamsize xsputn(const char * chars, std::streamsize number) {
        if (number)
            theEntry->append(chars, number);

        return number;
    }

private:
    StoreEntry *theEntry;

};

class StoreEntryStream : public std::ostream
{

public:
    /* create a stream for writing text etc into theEntry */
    // See http://www.codecomments.com/archive292-2005-2-396222.html
    StoreEntryStream(StoreEntry *entry): std::ostream(0), theBuffer(entry) {
        rdbuf(&theBuffer); // set the buffer to now-initialized theBuffer
        clear(); //clear badbit set by calling init(0)
    }

private:
    StoreEntryStreamBuf theBuffer;
};

#endif /* SQUID_STORE_ENTRY_STREAM_H */
