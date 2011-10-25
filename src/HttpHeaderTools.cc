/*
 * $Id$
 *
 * DEBUG: section 66    HTTP Header Tools
 * AUTHOR: Alex Rousskov
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
#include "acl/FilledChecklist.h"
#include "compat/strtoll.h"
#include "HttpHdrContRange.h"
#include "HttpHeader.h"
#include "MemBuf.h"

static void httpHeaderPutStrvf(HttpHeader * hdr, http_hdr_type id, const char *fmt, va_list vargs);


HttpHeaderFieldInfo *
httpHeaderBuildFieldsInfo(const HttpHeaderFieldAttrs * attrs, int count)
{
    int i;
    HttpHeaderFieldInfo *table = NULL;
    assert(attrs && count);

    /* allocate space */
    table = new HttpHeaderFieldInfo[count];

    for (i = 0; i < count; ++i) {
        const http_hdr_type id = attrs[i].id;
        HttpHeaderFieldInfo *info = table + id;
        /* sanity checks */
        assert(id >= 0 && id < count);
        assert(attrs[i].name);
        assert(info->id == HDR_ACCEPT && info->type == ftInvalid);	/* was not set before */
        /* copy and init fields */
        info->id = id;
        info->type = attrs[i].type;
        info->name = attrs[i].name;
        assert(info->name.size());
    }

    return table;
}

void
httpHeaderDestroyFieldsInfo(HttpHeaderFieldInfo * table, int count)
{
    int i;

    for (i = 0; i < count; ++i)
        table[i].name.clean();

    delete [] table;
}

void
httpHeaderMaskInit(HttpHeaderMask * mask, int value)
{
    memset(mask, value, sizeof(*mask));
}

/** calculates a bit mask of a given array; does not reset mask! */
void
httpHeaderCalcMask(HttpHeaderMask * mask, http_hdr_type http_hdr_type_enums[], size_t count)
{
    size_t i;
    const int * enums = (const int *) http_hdr_type_enums;
    assert(mask && enums);
    assert(count < sizeof(*mask) * 8);	/* check for overflow */

    for (i = 0; i < count; ++i) {
        assert(!CBIT_TEST(*mask, enums[i]));	/* check for duplicates */
        CBIT_SET(*mask, enums[i]);
    }
}

/* same as httpHeaderPutStr, but formats the string using snprintf first */
void
httpHeaderPutStrf(HttpHeader * hdr, http_hdr_type id, const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);

    httpHeaderPutStrvf(hdr, id, fmt, args);
    va_end(args);
}

/* used by httpHeaderPutStrf */
static void
httpHeaderPutStrvf(HttpHeader * hdr, http_hdr_type id, const char *fmt, va_list vargs)
{
    MemBuf mb;
    mb.init();
    mb.vPrintf(fmt, vargs);
    hdr->putStr(id, mb.buf);
    mb.clean();
}


/** wrapper arrounf PutContRange */
void
httpHeaderAddContRange(HttpHeader * hdr, HttpHdrRangeSpec spec, int64_t ent_len)
{
    HttpHdrContRange *cr = httpHdrContRangeCreate();
    assert(hdr && ent_len >= 0);
    httpHdrContRangeSet(cr, spec, ent_len);
    hdr->putContRange(cr);
    httpHdrContRangeDestroy(cr);
}


/**
 * return true if a given directive is found in at least one of
 * the "connection" header-fields note: if HDR_PROXY_CONNECTION is
 * present we ignore HDR_CONNECTION.
 */
int
httpHeaderHasConnDir(const HttpHeader * hdr, const char *directive)
{
    String list;
    int res;
    /* what type of header do we have? */

#if USE_HTTP_VIOLATIONS
    if (hdr->has(HDR_PROXY_CONNECTION))
        list = hdr->getList(HDR_PROXY_CONNECTION);
    else
#endif
        if (hdr->has(HDR_CONNECTION))
            list = hdr->getList(HDR_CONNECTION);
        else
            return 0;

    res = strListIsMember(&list, directive, ',');

    list.clean();

    return res;
}

/** returns true iff "m" is a member of the list */
int
strListIsMember(const String * list, const char *m, char del)
{
    const char *pos = NULL;
    const char *item;
    int ilen = 0;
    int mlen;
    assert(list && m);
    mlen = strlen(m);

    while (strListGetItem(list, del, &item, &ilen, &pos)) {
        if (mlen == ilen && !strncasecmp(item, m, ilen))
            return 1;
    }

    return 0;
}

/** returns true iff "s" is a substring of a member of the list */
int
strListIsSubstr(const String * list, const char *s, char del)
{
    assert(list && del);
    return (list->find(s) != String::npos);

    /** \note
     * Note: the original code with a loop is broken because it uses strstr()
     * instead of strnstr(). If 's' contains a 'del', strListIsSubstr() may
     * return true when it should not. If 's' does not contain a 'del', the
     * implementaion is equavalent to strstr()! Thus, we replace the loop with
     * strstr() above until strnstr() is available.
     */
}

/** appends an item to the list */
void
strListAdd(String * str, const char *item, char del)
{
    assert(str && item);

    if (str->size()) {
        char buf[3];
        buf[0] = del;
        buf[1] = ' ';
        buf[2] = '\0';
        str->append(buf, 2);
    }

    str->append(item, strlen(item));
}

/**
 * iterates through a 0-terminated string of items separated by 'del's.
 * white space around 'del' is considered to be a part of 'del'
 * like strtok, but preserves the source, and can iterate several strings at once
 *
 * returns true if next item is found.
 * init pos with NULL to start iteration.
 */
int
strListGetItem(const String * str, char del, const char **item, int *ilen, const char **pos)
{
    size_t len;
    /* ',' is always enabled as field delimiter as this is required for
     * processing merged header values properly, even if Cookie normally
     * uses ';' as delimiter.
     */
    static char delim[3][8] = {
        "\"?,",
        "\"\\",
        " ?,\t\r\n"
    };
    int quoted = 0;
    assert(str && item && pos);

    delim[0][1] = del;
    delim[2][1] = del;

    if (!*pos) {
        *pos = str->termedBuf();

        if (!*pos)
            return 0;
    }

    /* skip leading whitespace and delimiters */
    *pos += strspn(*pos, delim[2]);

    *item = *pos;		/* remember item's start */

    /* find next delimiter */
    do {
        *pos += strcspn(*pos, delim[quoted]);
        if (**pos == '"') {
            quoted = !quoted;
            *pos += 1;
        } else if (quoted && **pos == '\\') {
            *pos += 1;
            if (**pos)
                *pos += 1;
        } else {
            break;		/* Delimiter found, marking the end of this value */
        }
    } while (**pos);

    len = *pos - *item;		/* *pos points to del or '\0' */

    /* rtrim */
    while (len > 0 && xisspace((*item)[len - 1]))
        len--;

    if (ilen)
        *ilen = len;

    return len > 0;
}

/** handy to printf prefixes of potentially very long buffers */
const char *
getStringPrefix(const char *str, const char *end)
{
#define SHORT_PREFIX_SIZE 512
    LOCAL_ARRAY(char, buf, SHORT_PREFIX_SIZE);
    const int sz = 1 + (end ? end - str : strlen(str));
    xstrncpy(buf, str, (sz > SHORT_PREFIX_SIZE) ? SHORT_PREFIX_SIZE : sz);
    return buf;
}

/**
 * parses an int field, complains if soemthing went wrong, returns true on
 * success
 */
int
httpHeaderParseInt(const char *start, int *value)
{
    assert(value);
    *value = atoi(start);

    if (!*value && !xisdigit(*start)) {
        debugs(66, 2, "failed to parse an int header field near '" << start << "'");
        return 0;
    }

    return 1;
}

int
httpHeaderParseOffset(const char *start, int64_t * value)
{
    errno = 0;
    int64_t res = strtoll(start, NULL, 10);
    if (!res && EINVAL == errno)	/* maybe not portable? */
        return 0;
    *value = res;
    return 1;
}


/**
 * Parses a quoted-string field (RFC 2616 section 2.2), complains if
 * something went wrong, returns non-zero on success.
 * Un-escapes quoted-pair characters found within the string.
 * start should point at the first double-quote.
 */
int
httpHeaderParseQuotedString(const char *start, const int len, String *val)
{
    const char *end, *pos;
    val->clean();
    if (*start != '"') {
        debugs(66, 2, HERE << "failed to parse a quoted-string header field near '" << start << "'");
        return 0;
    }
    pos = start + 1;

    while (*pos != '"' && len > (pos-start)) {

        if (*pos =='\r') {
            pos++;
            if ((pos-start) > len || *pos != '\n') {
                debugs(66, 2, HERE << "failed to parse a quoted-string header field with '\\r' octet " << (start-pos)
                       << " bytes into '" << start << "'");
                val->clean();
                return 0;
            }
        }

        if (*pos == '\n') {
            pos++;
            if ( (pos-start) > len || (*pos != ' ' && *pos != '\t')) {
                debugs(66, 2, HERE << "failed to parse multiline quoted-string header field '" << start << "'");
                val->clean();
                return 0;
            }
            // TODO: replace the entire LWS with a space
            val->append(" ");
            pos++;
            debugs(66, 2, HERE << "len < pos-start => " << len << " < " << (pos-start));
            continue;
        }

        bool quoted = (*pos == '\\');
        if (quoted) {
            pos++;
            if (!*pos || (pos-start) > len) {
                debugs(66, 2, HERE << "failed to parse a quoted-string header field near '" << start << "'");
                val->clean();
                return 0;
            }
        }
        end = pos;
        while (end < (start+len) && *end != '\\' && *end != '\"' && (unsigned char)*end > 0x1F && *end != 0x7F)
            end++;
        if (((unsigned char)*end <= 0x1F && *end != '\r' && *end != '\n') || *end == 0x7F) {
            debugs(66, 2, HERE << "failed to parse a quoted-string header field with CTL octet " << (start-pos)
                   << " bytes into '" << start << "'");
            val->clean();
            return 0;
        }
        val->append(pos, end-pos);
        pos = end;
    }

    if (*pos != '\"') {
        debugs(66, 2, HERE << "failed to parse a quoted-string header field which did not end with \" ");
        val->clean();
        return 0;
    }
    /* Make sure it's defined even if empty "" */
    if (!val->defined())
        val->limitInit("", 0);
    return 1;
}

/**
 * Checks the anonymizer (header_access) configuration.
 *
 * \retval 0    Header is explicitly blocked for removal
 * \retval 1    Header is explicitly allowed
 * \retval 1    Header has been replaced, the current version can be used.
 * \retval 1    Header has no access controls to test
 */
static int
httpHdrMangle(HttpHeaderEntry * e, HttpRequest * request, int req_or_rep)
{
    int retval;

    /* check with anonymizer tables */
    header_mangler *hm;
    assert(e);

    if (ROR_REQUEST == req_or_rep) {
        hm = &Config.request_header_access[e->id];
    } else if (ROR_REPLY == req_or_rep) {
        hm = &Config.reply_header_access[e->id];
    } else {
        /* error. But let's call it "request". */
        hm = &Config.request_header_access[e->id];
    }

    /* mangler or checklist went away. default allow */
    if (!hm || !hm->access_list) {
        return 1;
    }

    ACLFilledChecklist checklist(hm->access_list, request, NULL);

    if (checklist.fastCheck() == ACCESS_ALLOWED) {
        /* aclCheckFast returns true for allow. */
        retval = 1;
    } else if (NULL == hm->replacement) {
        /* It was denied, and we don't have any replacement */
        retval = 0;
    } else {
        /* It was denied, but we have a replacement. Replace the
         * header on the fly, and return that the new header
         * is allowed.
         */
        e->value = hm->replacement;
        retval = 1;
    }

    return retval;
}

/** Mangles headers for a list of headers. */
void
httpHdrMangleList(HttpHeader * l, HttpRequest * request, int req_or_rep)
{
    HttpHeaderEntry *e;
    HttpHeaderPos p = HttpHeaderInitPos;

    int headers_deleted = 0;
    while ((e = l->getEntry(&p)))
        if (0 == httpHdrMangle(e, request, req_or_rep))
            l->delAt(p, headers_deleted);

    if (headers_deleted)
        l->refreshMask();
}

/**
 * return 1 if manglers are configured.  Used to set a flag
 * for optimization during request forwarding.
 */
int
httpReqHdrManglersConfigured()
{
    for (int i = 0; i < HDR_ENUM_END; i++) {
        if (NULL != Config.request_header_access[i].access_list)
            return 1;
    }

    return 0;
}
