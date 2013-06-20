
/*
 * DEBUG: section 39    Peer user hash based selection
 * AUTHOR: Henrik Nordstrom
 * BASED ON: carp.cc
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

#if USE_AUTH

#include "auth/UserRequest.h"
#include "CachePeer.h"
#include "globals.h"
#include "HttpRequest.h"
#include "mgr/Registration.h"
#include "neighbors.h"
#include "SquidConfig.h"
#include "Store.h"

#if HAVE_MATH_H
#include <math.h>
#endif

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

static int n_userhash_peers = 0;
static CachePeer **userhash_peers = NULL;
static OBJH peerUserHashCachemgr;
static void peerUserHashRegisterWithCacheManager(void);

static int
peerSortWeight(const void *a, const void *b)
{
    const CachePeer *const *p1 = (const CachePeer *const *)a;
    const CachePeer *const *p2 = (const CachePeer *const *)b;
    return (*p1)->weight - (*p2)->weight;
}

void
peerUserHashInit(void)
{
    int W = 0;
    int K;
    int k;
    double P_last, X_last, Xn;
    CachePeer *p;
    CachePeer **P;
    char *t;
    /* Clean up */

    for (k = 0; k < n_userhash_peers; ++k) {
        cbdataReferenceDone(userhash_peers[k]);
    }

    safe_free(userhash_peers);
    n_userhash_peers = 0;
    /* find out which peers we have */

    peerUserHashRegisterWithCacheManager();

    for (p = Config.peers; p; p = p->next) {
        if (!p->options.userhash)
            continue;

        assert(p->type == PEER_PARENT);

        if (p->weight == 0)
            continue;

        ++n_userhash_peers;

        W += p->weight;
    }

    if (n_userhash_peers == 0)
        return;

    userhash_peers = (CachePeer **)xcalloc(n_userhash_peers, sizeof(*userhash_peers));

    /* Build a list of the found peers and calculate hashes and load factors */
    for (P = userhash_peers, p = Config.peers; p; p = p->next) {
        if (!p->options.userhash)
            continue;

        if (p->weight == 0)
            continue;

        /* calculate this peers hash */
        p->userhash.hash = 0;

        for (t = p->name; *t != 0; ++t)
            p->userhash.hash += ROTATE_LEFT(p->userhash.hash, 19) + (unsigned int) *t;

        p->userhash.hash += p->userhash.hash * 0x62531965;

        p->userhash.hash = ROTATE_LEFT(p->userhash.hash, 21);

        /* and load factor */
        p->userhash.load_factor = ((double) p->weight) / (double) W;

        if (floor(p->userhash.load_factor * 1000.0) == 0.0)
            p->userhash.load_factor = 0.0;

        /* add it to our list of peers */
        *P++ = cbdataReference(p);
    }

    /* Sort our list on weight */
    qsort(userhash_peers, n_userhash_peers, sizeof(*userhash_peers), peerSortWeight);

    /* Calculate the load factor multipliers X_k
     *
     * X_1 = pow ((K*p_1), (1/K))
     * X_k = ([K-k+1] * [P_k - P_{k-1}])/(X_1 * X_2 * ... * X_{k-1})
     * X_k += pow ((X_{k-1}, {K-k+1})
     * X_k = pow (X_k, {1/(K-k+1)})
     * simplified to have X_1 part of the loop
     */
    K = n_userhash_peers;

    P_last = 0.0;		/* Empty P_0 */

    Xn = 1.0;			/* Empty starting point of X_1 * X_2 * ... * X_{x-1} */

    X_last = 0.0;		/* Empty X_0, nullifies the first pow statement */

    for (k = 1; k <= K; ++k) {
        double Kk1 = (double) (K - k + 1);
        p = userhash_peers[k - 1];
        p->userhash.load_multiplier = (Kk1 * (p->userhash.load_factor - P_last)) / Xn;
        p->userhash.load_multiplier += pow(X_last, Kk1);
        p->userhash.load_multiplier = pow(p->userhash.load_multiplier, 1.0 / Kk1);
        Xn *= p->userhash.load_multiplier;
        X_last = p->userhash.load_multiplier;
        P_last = p->userhash.load_factor;
    }
}

static void
peerUserHashRegisterWithCacheManager(void)
{
    Mgr::RegisterAction("userhash", "peer userhash information", peerUserHashCachemgr,
                        0, 1);
}

CachePeer *
peerUserHashSelectParent(HttpRequest * request)
{
    int k;
    const char *c;
    CachePeer *p = NULL;
    CachePeer *tp;
    unsigned int user_hash = 0;
    unsigned int combined_hash;
    double score;
    double high_score = 0;
    const char *key = NULL;

    if (n_userhash_peers == 0)
        return NULL;

    if (request->auth_user_request != NULL)
        key = request->auth_user_request->username();

    if (!key)
        return NULL;

    /* calculate hash key */
    debugs(39, 2, "peerUserHashSelectParent: Calculating hash for " << key);

    for (c = key; *c != 0; ++c)
        user_hash += ROTATE_LEFT(user_hash, 19) + *c;

    /* select CachePeer */
    for (k = 0; k < n_userhash_peers; ++k) {
        tp = userhash_peers[k];
        combined_hash = (user_hash ^ tp->userhash.hash);
        combined_hash += combined_hash * 0x62531965;
        combined_hash = ROTATE_LEFT(combined_hash, 21);
        score = combined_hash * tp->userhash.load_multiplier;
        debugs(39, 3, "peerUserHashSelectParent: " << tp->name << " combined_hash " << combined_hash  <<
               " score " << std::setprecision(0) << score);

        if ((score > high_score) && peerHTTPOkay(tp, request)) {
            p = tp;
            high_score = score;
        }
    }

    if (p)
        debugs(39, 2, "peerUserHashSelectParent: selected " << p->name);

    return p;
}

static void
peerUserHashCachemgr(StoreEntry * sentry)
{
    CachePeer *p;
    int sumfetches = 0;
    storeAppendPrintf(sentry, "%24s %10s %10s %10s %10s\n",
                      "Hostname",
                      "Hash",
                      "Multiplier",
                      "Factor",
                      "Actual");

    for (p = Config.peers; p; p = p->next)
        sumfetches += p->stats.fetches;

    for (p = Config.peers; p; p = p->next) {
        storeAppendPrintf(sentry, "%24s %10x %10f %10f %10f\n",
                          p->name, p->userhash.hash,
                          p->userhash.load_multiplier,
                          p->userhash.load_factor,
                          sumfetches ? (double) p->stats.fetches / sumfetches : -1.0);
    }
}

#endif /* USE_AUTH */
