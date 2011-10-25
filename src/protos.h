/*
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
#ifndef SQUID_PROTOS_H
#define SQUID_PROTOS_H

/* included for routines that have not moved out to their proper homes
 * yet.
 */
#include "Packer.h"
/* for routines still in this file that take CacheManager parameters */
#include "ip/Address.h"
/* for parameters that still need these */
#include "enums.h"
/* some parameters stil need this */
#include "wordlist.h"

/* for parameters that still need these */
#include "lookup_t.h"


class HttpRequestMethod;
#if USE_DELAY_POOLS
class ClientInfo;
#endif


#if USE_FORW_VIA_DB
SQUIDCEXTERN void fvdbCountVia(const char *key);
SQUIDCEXTERN void fvdbCountForw(const char *key);
#endif
#if HEADERS_LOG
SQUIDCEXTERN void headersLog(int cs, int pq, const HttpRequestMethod& m, void *data);
#endif
SQUIDCEXTERN int logTypeIsATcpHit(log_type);

/*
 * cache_cf.c
 */
SQUIDCEXTERN void configFreeMemory(void);
class MemBuf;
SQUIDCEXTERN void wordlistCat(const wordlist *, MemBuf * mb);
SQUIDCEXTERN void self_destruct(void);
SQUIDCEXTERN void add_http_port(char *portspec);
extern int xatoi(const char *token);
extern long xatol(const char *token);


/* extra functions from cache_cf.c useful for lib modules */
SQUIDCEXTERN void parse_int(int *var);
SQUIDCEXTERN void parse_onoff(int *var);
SQUIDCEXTERN void parse_eol(char *volatile *var);
SQUIDCEXTERN void parse_wordlist(wordlist ** list);
SQUIDCEXTERN void requirePathnameExists(const char *name, const char *path);
SQUIDCEXTERN void parse_time_t(time_t * var);


/* client_side.c - FD related client side routines */

SQUIDCEXTERN void clientdbInit(void);

#include "anyp/ProtocolType.h"
SQUIDCEXTERN void clientdbUpdate(const Ip::Address &, log_type, AnyP::ProtocolType, size_t);

SQUIDCEXTERN int clientdbCutoffDenied(const Ip::Address &);
void clientdbDump(StoreEntry *);
SQUIDCEXTERN void clientdbFreeMemory(void);

SQUIDCEXTERN int clientdbEstablished(const Ip::Address &, int);
#if USE_DELAY_POOLS
SQUIDCEXTERN void clientdbSetWriteLimiter(ClientInfo * info, const int writeSpeedLimit,const double initialBurst,const double highWatermark);
SQUIDCEXTERN ClientInfo * clientdbGetInfo(const Ip::Address &addr);
#endif
SQUIDCEXTERN void clientOpenListenSockets(void);
SQUIDCEXTERN void clientHttpConnectionsClose(void);
SQUIDCEXTERN void httpRequestFree(void *);

extern void clientAccessCheck(void *);

#include "Debug.h"

/* packs, then prints an object using debugs() */
SQUIDCEXTERN void debugObj(int section, int level, const char *label, void *obj, ObjPackMethod pm);

/* disk.c */
SQUIDCEXTERN int file_open(const char *path, int mode);
SQUIDCEXTERN void file_close(int fd);
/* Adapter file_write for object callbacks */

template <class O>
void
FreeObject(void *address)
{
    O *anObject = static_cast <O *>(address);
    delete anObject;
}

SQUIDCEXTERN void file_write(int, off_t, void const *, int len, DWCB *, void *, FREE *);
SQUIDCEXTERN void file_write_mbuf(int fd, off_t, MemBuf mb, DWCB * handler, void *handler_data);
SQUIDCEXTERN void file_read(int, char *, int, off_t, DRCB *, void *);
SQUIDCEXTERN void disk_init(void);

SQUIDCEXTERN void dnsShutdown(void);
SQUIDCEXTERN void dnsInit(void);
SQUIDCEXTERN void dnsSubmit(const char *lookup, HLPCB * callback, void *data);

/* dns_internal.c */
SQUIDCEXTERN void idnsInit(void);
SQUIDCEXTERN void idnsShutdown(void);
SQUIDCEXTERN void idnsALookup(const char *, IDNSCB *, void *);

SQUIDCEXTERN void idnsPTRLookup(const Ip::Address &, IDNSCB *, void *);

SQUIDCEXTERN void fd_close(int fd);
SQUIDCEXTERN void fd_open(int fd, unsigned int type, const char *);
SQUIDCEXTERN void fd_note(int fd, const char *);
SQUIDCEXTERN void fd_bytes(int fd, int len, unsigned int type);
SQUIDCEXTERN void fdDumpOpen(void);
SQUIDCEXTERN int fdUsageHigh(void);
SQUIDCEXTERN void fdAdjustReserved(void);

SQUIDCEXTERN fileMap *file_map_create(void);
SQUIDCEXTERN int file_map_allocate(fileMap *, int);
SQUIDCEXTERN int file_map_bit_set(fileMap *, int);
SQUIDCEXTERN int file_map_bit_test(fileMap *, int);
SQUIDCEXTERN void file_map_bit_reset(fileMap *, int);
SQUIDCEXTERN void filemapFreeMemory(fileMap *);


SQUIDCEXTERN void fqdncache_nbgethostbyaddr(const Ip::Address &, FQDNH *, void *);

SQUIDCEXTERN const char *fqdncache_gethostbyaddr(const Ip::Address &, int flags);
SQUIDCEXTERN void fqdncache_init(void);
void fqdnStats(StoreEntry *);
SQUIDCEXTERN void fqdncacheReleaseInvalid(const char *);

SQUIDCEXTERN const char *fqdnFromAddr(const Ip::Address &);
SQUIDCEXTERN int fqdncacheQueueDrain(void);
SQUIDCEXTERN void fqdncacheFreeMemory(void);
SQUIDCEXTERN void fqdncache_restart(void);
void fqdncache_purgelru(void *);
SQUIDCEXTERN void fqdncacheAddEntryFromHosts(char *addr, wordlist * hostnames);

class FwdState;

/**
 \defgroup ServerProtocolFTPAPI Server-Side FTP API
 \ingroup ServerProtocol
 */

/// \ingroup ServerProtocolFTPAPI
SQUIDCEXTERN void ftpStart(FwdState *);

class HttpRequest;
class HttpReply;

/// \ingroup ServerProtocolFTPAPI
SQUIDCEXTERN const char *ftpUrlWith2f(HttpRequest *);


/**
 \defgroup ServerProtocolGopherAPI Server-Side Gopher API
 \ingroup ServerProtocol
 */

/// \ingroup ServerProtocolGopherAPI
SQUIDCEXTERN void gopherStart(FwdState *);

/// \ingroup ServerProtocolGopherAPI
SQUIDCEXTERN int gopherCachable(const HttpRequest *);


/**
 \defgroup ServerProtocolWhoisAPI Server-Side WHOIS API
 \ingroup ServerProtocol
 */

/// \ingroup ServerProtocolWhoisAPI
SQUIDCEXTERN void whoisStart(FwdState *);


/* http.c */
/* for http_hdr_type field */
#include "HttpHeader.h"
SQUIDCEXTERN int httpCachable(const HttpRequestMethod&);
SQUIDCEXTERN void httpStart(FwdState *);
SQUIDCEXTERN mb_size_t httpBuildRequestPrefix(HttpRequest * request,
        HttpRequest * orig_request,
        StoreEntry * entry,
        MemBuf * mb,
        http_state_flags);
SQUIDCEXTERN void httpAnonInitModule(void);
SQUIDCEXTERN int httpAnonHdrAllowed(http_hdr_type hdr_id);
SQUIDCEXTERN int httpAnonHdrDenied(http_hdr_type hdr_id);
SQUIDCEXTERN const char *httpMakeVaryMark(HttpRequest * request, HttpReply const * reply);

#include "HttpStatusCode.h"
SQUIDCEXTERN const char *httpStatusString(http_status status);

/* Http Body */
/* init/clean */
SQUIDCEXTERN void httpBodyInit(HttpBody * body);
SQUIDCEXTERN void httpBodyClean(HttpBody * body);
/* get body ptr (always use this) */
SQUIDCEXTERN const char *httpBodyPtr(const HttpBody * body);
/* set body, does not clone mb so you should not reuse it */
SQUIDCEXTERN void httpBodySet(HttpBody * body, MemBuf * mb);

/* pack */
SQUIDCEXTERN void httpBodyPackInto(const HttpBody * body, Packer * p);

/* Http Cache Control Header Field */
SQUIDCEXTERN void httpHdrCcInitModule(void);
SQUIDCEXTERN void httpHdrCcCleanModule(void);
SQUIDCEXTERN void httpHdrCcUpdateStats(const HttpHdrCc * cc, StatHist * hist);
void httpHdrCcStatDumper(StoreEntry * sentry, int idx, double val, double size, int count);

/* Http Header Tools */
class HttpHeaderFieldInfo;
SQUIDCEXTERN HttpHeaderFieldInfo *httpHeaderBuildFieldsInfo(const HttpHeaderFieldAttrs * attrs, int count);
SQUIDCEXTERN void httpHeaderDestroyFieldsInfo(HttpHeaderFieldInfo * info, int count);
SQUIDCEXTERN http_hdr_type httpHeaderIdByName(const char *name, size_t name_len, const HttpHeaderFieldInfo * attrs, int end);
SQUIDCEXTERN http_hdr_type httpHeaderIdByNameDef(const char *name, int name_len);
SQUIDCEXTERN const char *httpHeaderNameById(int id);
SQUIDCEXTERN int httpHeaderHasConnDir(const HttpHeader * hdr, const char *directive);
SQUIDCEXTERN void strListAdd(String * str, const char *item, char del);
SQUIDCEXTERN int strListIsMember(const String * str, const char *item, char del);
SQUIDCEXTERN int strListIsSubstr(const String * list, const char *s, char del);
SQUIDCEXTERN int strListGetItem(const String * str, char del, const char **item, int *ilen, const char **pos);
SQUIDCEXTERN const char *getStringPrefix(const char *str, const char *end);
SQUIDCEXTERN int httpHeaderParseInt(const char *start, int *val);
SQUIDCEXTERN int httpHeaderParseOffset(const char *start, int64_t * off);
SQUIDCEXTERN void
httpHeaderPutStrf(HttpHeader * hdr, http_hdr_type id, const char *fmt,...) PRINTF_FORMAT_ARG3;


/* Http Header */
SQUIDCEXTERN void httpHeaderInitModule(void);
SQUIDCEXTERN void httpHeaderCleanModule(void);

/* store report about current header usage and other stats */
void httpHeaderStoreReport(StoreEntry * e);
SQUIDCEXTERN void httpHdrMangleList(HttpHeader *, HttpRequest *, int req_or_rep);
SQUIDCEXTERN int httpReqHdrManglersConfigured();

#if SQUID_SNMP
SQUIDCEXTERN PF snmpHandleUdp;
SQUIDCEXTERN void snmpInit(void);
SQUIDCEXTERN void snmpConnectionOpen(void);
SQUIDCEXTERN void snmpConnectionShutdown(void);
SQUIDCEXTERN void snmpConnectionClose(void);
SQUIDCEXTERN const char * snmpDebugOid(oid * Name, snint Len, MemBuf &outbuf);

SQUIDCEXTERN void addr2oid(Ip::Address &addr, oid *Dest);
SQUIDCEXTERN void oid2addr(oid *Dest, Ip::Address &addr, u_int code);

SQUIDCEXTERN Ip::Address *client_entry(Ip::Address *current);
extern variable_list *snmp_basicFn(variable_list *, snint *);
extern variable_list *snmp_confFn(variable_list *, snint *);
extern variable_list *snmp_sysFn(variable_list *, snint *);
extern variable_list *snmp_prfSysFn(variable_list *, snint *);
extern variable_list *snmp_prfProtoFn(variable_list *, snint *);
extern variable_list *snmp_prfPeerFn(variable_list *, snint *);
extern variable_list *snmp_netIpFn(variable_list *, snint *);
extern variable_list *snmp_netFqdnFn(variable_list *, snint *);
#if USE_DNSSERVERS
extern variable_list *snmp_netDnsFn(variable_list *, snint *);
#else
extern variable_list *snmp_netIdnsFn(variable_list *, snint *);
#endif /* USE_DNSSERVERS */
extern variable_list *snmp_meshPtblFn(variable_list *, snint *);
extern variable_list *snmp_meshCtblFn(variable_list *, snint *);
#endif /* SQUID_SNMP */

#if USE_WCCP
SQUIDCEXTERN void wccpInit(void);
SQUIDCEXTERN void wccpConnectionOpen(void);
SQUIDCEXTERN void wccpConnectionClose(void);
#endif /* USE_WCCP */

#if USE_WCCPv2
extern void wccp2Init(void);
extern void wccp2ConnectionOpen(void);
extern void wccp2ConnectionClose(void);
#endif /* USE_WCCPv2 */

SQUIDCEXTERN char *mime_get_header(const char *mime, const char *header);
SQUIDCEXTERN char *mime_get_header_field(const char *mime, const char *name, const char *prefix);
SQUIDCEXTERN size_t headersEnd(const char *, size_t);

SQUIDCEXTERN void mimeInit(char *filename);
SQUIDCEXTERN void mimeFreeMemory(void);
SQUIDCEXTERN char *mimeGetContentEncoding(const char *fn);
SQUIDCEXTERN char *mimeGetContentType(const char *fn);
SQUIDCEXTERN char const *mimeGetIcon(const char *fn);
SQUIDCEXTERN const char *mimeGetIconURL(const char *fn);
SQUIDCEXTERN char mimeGetTransferMode(const char *fn);
SQUIDCEXTERN int mimeGetDownloadOption(const char *fn);
SQUIDCEXTERN int mimeGetViewOption(const char *fn);

#include "ipcache.h"
SQUIDCEXTERN int mcastSetTtl(int, int);
SQUIDCEXTERN IPH mcastJoinGroups;

SQUIDCEXTERN peer *getFirstPeer(void);
SQUIDCEXTERN peer *getFirstUpParent(HttpRequest *);
SQUIDCEXTERN peer *getNextPeer(peer *);
SQUIDCEXTERN peer *getSingleParent(HttpRequest *);
SQUIDCEXTERN int neighborsCount(HttpRequest *);
SQUIDCEXTERN int neighborsUdpPing(HttpRequest *,
                                  StoreEntry *,
                                  IRCB * callback,
                                  void *data,
                                  int *exprep,
                                  int *timeout);
SQUIDCEXTERN void neighborAddAcl(const char *, const char *);

SQUIDCEXTERN void neighborsUdpAck(const cache_key *, icp_common_t *, const Ip::Address &);
SQUIDCEXTERN void neighborAdd(const char *, const char *, int, int, int, int, int);
SQUIDCEXTERN void neighbors_init(void);
#if USE_HTCP
SQUIDCEXTERN void neighborsHtcpClear(StoreEntry *, const char *, HttpRequest *, const HttpRequestMethod &, htcp_clr_reason);
#endif
SQUIDCEXTERN peer *peerFindByName(const char *);
SQUIDCEXTERN peer *peerFindByNameAndPort(const char *, unsigned short);
SQUIDCEXTERN peer *getDefaultParent(HttpRequest * request);
SQUIDCEXTERN peer *getRoundRobinParent(HttpRequest * request);
SQUIDCEXTERN peer *getWeightedRoundRobinParent(HttpRequest * request);
SQUIDCEXTERN void peerClearRRStart(void);
SQUIDCEXTERN void peerClearRR(void);
SQUIDCEXTERN lookup_t peerDigestLookup(peer * p, HttpRequest * request);
SQUIDCEXTERN peer *neighborsDigestSelect(HttpRequest * request);
SQUIDCEXTERN void peerNoteDigestLookup(HttpRequest * request, peer * p, lookup_t lookup);
SQUIDCEXTERN void peerNoteDigestGone(peer * p);
SQUIDCEXTERN int neighborUp(const peer * e);
SQUIDCEXTERN CBDUNL peerDestroy;
SQUIDCEXTERN const char *neighborTypeStr(const peer * e);
SQUIDCEXTERN peer_t neighborType(const peer *, const HttpRequest *);
SQUIDCEXTERN void peerConnectFailed(peer *);
SQUIDCEXTERN void peerConnectSucceded(peer *);
SQUIDCEXTERN void dump_peer_options(StoreEntry *, peer *);
SQUIDCEXTERN int peerHTTPOkay(const peer *, HttpRequest *);

SQUIDCEXTERN peer *whichPeer(const Ip::Address &from);

/* peer_digest.c */
class PeerDigest;
SQUIDCEXTERN PeerDigest *peerDigestCreate(peer * p);
SQUIDCEXTERN void peerDigestNeeded(PeerDigest * pd);
SQUIDCEXTERN void peerDigestNotePeerGone(PeerDigest * pd);
SQUIDCEXTERN void peerDigestStatsReport(const PeerDigest * pd, StoreEntry * e);

#include "comm/forward.h"
extern void getOutgoingAddress(HttpRequest * request, Comm::ConnectionPointer conn);
extern Ip::Address getOutgoingAddr(HttpRequest * request, struct peer *dst_peer);

SQUIDCEXTERN void urnStart(HttpRequest *, StoreEntry *);

SQUIDCEXTERN void redirectInit(void);
SQUIDCEXTERN void redirectShutdown(void);

extern void refreshAddToList(const char *, int, time_t, int, time_t);
extern int refreshIsCachable(const StoreEntry *);
extern int refreshCheckHTTP(const StoreEntry *, HttpRequest *);
extern int refreshCheckICP(const StoreEntry *, HttpRequest *);
extern int refreshCheckHTCP(const StoreEntry *, HttpRequest *);
extern int refreshCheckDigest(const StoreEntry *, time_t delta);
extern time_t getMaxAge(const char *url);
extern void refreshInit(void);
extern const refresh_t *refreshLimits(const char *url);

extern void shut_down(int);
extern void rotate_logs(int);
extern void reconfigure(int);


extern void start_announce(void *unused);
extern void waisStart(FwdState *);

SQUIDCEXTERN void statInit(void);
SQUIDCEXTERN void statFreeMemory(void);
SQUIDCEXTERN double median_svc_get(int, int);
SQUIDCEXTERN void pconnHistCount(int, int);
SQUIDCEXTERN int stat5minClientRequests(void);
SQUIDCEXTERN double stat5minCPUUsage(void);
SQUIDCEXTERN double statRequestHitRatio(int minutes);
SQUIDCEXTERN double statRequestHitMemoryRatio(int minutes);
SQUIDCEXTERN double statRequestHitDiskRatio(int minutes);
SQUIDCEXTERN double statByteHitRatio(int minutes);

/* StatHist */
SQUIDCEXTERN void statHistClean(StatHist * H);
SQUIDCEXTERN void statHistCount(StatHist * H, double val);
SQUIDCEXTERN void statHistCopy(StatHist * Dest, const StatHist * Orig);
SQUIDCEXTERN void statHistSafeCopy(StatHist * Dest, const StatHist * Orig);
SQUIDCEXTERN double statHistDeltaMedian(const StatHist * A, const StatHist * B);
SQUIDCEXTERN double statHistDeltaPctile(const StatHist * A, const StatHist * B, double pctile);
SQUIDCEXTERN void statHistDump(const StatHist * H, StoreEntry * sentry, StatHistBinDumper * bd);
SQUIDCEXTERN void statHistLogInit(StatHist * H, int capacity, double min, double max);
SQUIDCEXTERN void statHistEnumInit(StatHist * H, int last_enum);
SQUIDCEXTERN void statHistIntInit(StatHist * H, int n);
SQUIDCEXTERN StatHistBinDumper statHistEnumDumper;
SQUIDCEXTERN StatHistBinDumper statHistIntDumper;


/* mem */
SQUIDCEXTERN void memClean(void);
SQUIDCEXTERN void memInitModule(void);
SQUIDCEXTERN void memCleanModule(void);
SQUIDCEXTERN void memConfigure(void);
SQUIDCEXTERN void *memAllocate(mem_type);
SQUIDCEXTERN void *memAllocString(size_t net_size, size_t * gross_size);
SQUIDCEXTERN void *memAllocBuf(size_t net_size, size_t * gross_size);
SQUIDCEXTERN void *memReallocBuf(void *buf, size_t net_size, size_t * gross_size);
SQUIDCEXTERN void memFree(void *, int type);
void memFree2K(void *);
void memFree4K(void *);
void memFree8K(void *);
void memFree16K(void *);
void memFree32K(void *);
void memFree64K(void *);
SQUIDCEXTERN void memFreeString(size_t size, void *);
SQUIDCEXTERN void memFreeBuf(size_t size, void *);
SQUIDCEXTERN FREE *memFreeBufFunc(size_t size);
SQUIDCEXTERN int memInUse(mem_type);
SQUIDCEXTERN void memDataInit(mem_type, const char *, size_t, int, bool zeroOnPush = true);
SQUIDCEXTERN void memCheckInit(void);


/* Mem */
SQUIDCEXTERN void memConfigure(void);

/* ----------------------------------------------------------------- */

/* repl_modules.c */
SQUIDCEXTERN void storeReplSetup(void);

/*
 * store_log.c
 */
SQUIDCEXTERN void storeLog(int tag, const StoreEntry * e);
SQUIDCEXTERN void storeLogRotate(void);
SQUIDCEXTERN void storeLogClose(void);
SQUIDCEXTERN void storeLogOpen(void);


/*
 * store_key_*.c
 */
SQUIDCEXTERN cache_key *storeKeyDup(const cache_key *);
SQUIDCEXTERN cache_key *storeKeyCopy(cache_key *, const cache_key *);
SQUIDCEXTERN void storeKeyFree(const cache_key *);
SQUIDCEXTERN const cache_key *storeKeyScan(const char *);
SQUIDCEXTERN const char *storeKeyText(const cache_key *);
SQUIDCEXTERN const cache_key *storeKeyPublic(const char *, const HttpRequestMethod&);
SQUIDCEXTERN const cache_key *storeKeyPublicByRequest(HttpRequest *);
SQUIDCEXTERN const cache_key *storeKeyPublicByRequestMethod(HttpRequest *, const HttpRequestMethod&);
SQUIDCEXTERN const cache_key *storeKeyPrivate(const char *, const HttpRequestMethod&, int);
SQUIDCEXTERN int storeKeyHashBuckets(int);
SQUIDCEXTERN int storeKeyNull(const cache_key *);
SQUIDCEXTERN void storeKeyInit(void);
SQUIDCEXTERN HASHHASH storeKeyHashHash;
SQUIDCEXTERN HASHCMP storeKeyHashCmp;

/*
 * store_digest.c
 */
SQUIDCEXTERN void storeDigestInit(void);
SQUIDCEXTERN void storeDigestNoteStoreReady(void);
SQUIDCEXTERN void storeDigestScheduleRebuild(void);
SQUIDCEXTERN void storeDigestDel(const StoreEntry * entry);
extern void storeDigestReport(StoreEntry *);

/*
 * store_rebuild.c
 */
SQUIDCEXTERN void storeRebuildStart(void);

SQUIDCEXTERN void storeRebuildComplete(struct _store_rebuild_data *);
SQUIDCEXTERN void storeRebuildProgress(int sd_index, int total, int sofar);

/// loads entry from disk; fills supplied memory buffer on success
bool storeRebuildLoadEntry(int fd, int diskIndex, MemBuf &buf, struct _store_rebuild_data &counts);
/// parses entry buffer and validates entry metadata; fills e on success
bool storeRebuildParseEntry(MemBuf &buf, StoreEntry &e, cache_key *key, struct _store_rebuild_data &counts, uint64_t expectedSize);
/// checks whether the loaded entry should be kept; updates counters
bool storeRebuildKeepEntry(const StoreEntry &e, const cache_key *key, struct _store_rebuild_data &counts);


/*
 * store_swapin.c
 */
class store_client;
SQUIDCEXTERN void storeSwapInStart(store_client *);

/*
 * store_client.c
 */
SQUIDCEXTERN store_client *storeClientListAdd(StoreEntry * e, void *data);
SQUIDCEXTERN int storeClientCopyPending(store_client *, StoreEntry * e, void *data);
SQUIDCEXTERN int storeUnregister(store_client * sc, StoreEntry * e, void *data)
;
SQUIDCEXTERN int storePendingNClients(const StoreEntry * e);
SQUIDCEXTERN int storeClientIsThisAClient(store_client * sc, void *someClient);


SQUIDCEXTERN const char *getMyHostname(void);
SQUIDCEXTERN const char *uniqueHostname(void);
SQUIDCEXTERN void safeunlink(const char *path, int quiet);

#include "fatal.h"
void death(int sig);
void sigusr2_handle(int sig);
void sig_child(int sig);
void sig_shutdown(int sig); ///< handles shutdown notifications from kids
SQUIDCEXTERN void leave_suid(void);
SQUIDCEXTERN void enter_suid(void);
SQUIDCEXTERN void no_suid(void);
SQUIDCEXTERN void writePidFile(void);
SQUIDCEXTERN void setSocketShutdownLifetimes(int);
SQUIDCEXTERN void setMaxFD(void);
SQUIDCEXTERN void setSystemLimits(void);
extern void squid_signal(int sig, SIGHDLR *, int flags);
SQUIDCEXTERN pid_t readPidFile(void);
SQUIDCEXTERN void keepCapabilities(void);
SQUIDCEXTERN void BroadcastSignalIfAny(int& sig);
/// whether the current process is the parent of all other Squid processes
SQUIDCEXTERN bool IamMasterProcess();
/**
    whether the current process is dedicated to doing things that only
    a single process should do, such as PID file maintenance and WCCP
*/
SQUIDCEXTERN bool IamPrimaryProcess();
/// whether the current process coordinates worker processes
SQUIDCEXTERN bool IamCoordinatorProcess();
/// whether the current process handles HTTP transactions and such
SQUIDCEXTERN bool IamWorkerProcess();
/// whether the current process is dedicated to managing a cache_dir
bool IamDiskProcess();
/// Whether we are running in daemon mode
SQUIDCEXTERN bool InDaemonMode(); // try using specific Iam*() checks above first
/// Whether there should be more than one worker process running
SQUIDCEXTERN bool UsingSmp(); // try using specific Iam*() checks above first
/// number of Kid processes as defined in src/ipc/Kid.h
SQUIDCEXTERN int NumberOfKids();
/// a string describing this process roles such as worker or coordinator
String ProcessRoles();
SQUIDCEXTERN int DebugSignal;

/* AYJ debugs function to show locations being reset with memset() */
SQUIDCEXTERN void *xmemset(void *dst, int, size_t);

SQUIDCEXTERN void debug_trap(const char *);
SQUIDCEXTERN void logsFlush(void);
SQUIDCEXTERN const char *checkNullString(const char *p);

SQUIDCEXTERN void squid_getrusage(struct rusage *r);

SQUIDCEXTERN double rusage_cputime(struct rusage *r);

SQUIDCEXTERN int rusage_maxrss(struct rusage *r);

SQUIDCEXTERN int rusage_pagefaults(struct rusage *r);
SQUIDCEXTERN void releaseServerSockets(void);
SQUIDCEXTERN void PrintRusage(void);
SQUIDCEXTERN void dumpMallocStats(void);

#if USE_UNLINKD
SQUIDCEXTERN void unlinkdInit(void);
SQUIDCEXTERN void unlinkdClose(void);
SQUIDCEXTERN void unlinkdUnlink(const char *);
#endif

SQUIDCEXTERN AnyP::ProtocolType urlParseProtocol(const char *, const char *e = NULL);
SQUIDCEXTERN void urlInitialize(void);
SQUIDCEXTERN HttpRequest *urlParse(const HttpRequestMethod&, char *, HttpRequest *request = NULL);
SQUIDCEXTERN const char *urlCanonical(HttpRequest *);
SQUIDCEXTERN char *urlCanonicalClean(const HttpRequest *);
SQUIDCEXTERN const char *urlCanonicalFakeHttps(const HttpRequest * request);
SQUIDCEXTERN bool urlIsRelative(const char *);
SQUIDCEXTERN char *urlMakeAbsolute(const HttpRequest *, const char *);
SQUIDCEXTERN char *urlRInternal(const char *host, unsigned short port, const char *dir, const char *name);
SQUIDCEXTERN char *urlInternal(const char *dir, const char *name);
SQUIDCEXTERN int matchDomainName(const char *host, const char *domain);
SQUIDCEXTERN int urlCheckRequest(const HttpRequest *);
SQUIDCEXTERN int urlDefaultPort(AnyP::ProtocolType p);
SQUIDCEXTERN char *urlHostname(const char *url);
SQUIDCEXTERN void urlExtMethodConfigure(void);

SQUIDCEXTERN peer_t parseNeighborType(const char *s);

/* tools.c */
//UNUSED	#include "dlink.h"
//UNUSED	SQUIDCEXTERN void dlinkAdd(void *data, dlink_node *, dlink_list *);
//UNUSED	SQUIDCEXTERN void dlinkAddAfter(void *, dlink_node *, dlink_node *, dlink_list *);
//UNUSED	SQUIDCEXTERN void dlinkAddTail(void *data, dlink_node *, dlink_list *);
//UNUSED	SQUIDCEXTERN void dlinkDelete(dlink_node * m, dlink_list * list);
//UNUSED	SQUIDCEXTERN void dlinkNodeDelete(dlink_node * m);
//UNUSED	SQUIDCEXTERN dlink_node *dlinkNodeNew(void);

SQUIDCEXTERN void kb_incr(kb_t *, size_t);
SQUIDCEXTERN int stringHasWhitespace(const char *);
SQUIDCEXTERN int stringHasCntl(const char *);
SQUIDCEXTERN void linklistPush(link_list **, void *);
SQUIDCEXTERN void *linklistShift(link_list **);
SQUIDCEXTERN int xrename(const char *from, const char *to);
SQUIDCEXTERN int isPowTen(int);
SQUIDCEXTERN void parseEtcHosts(void);
SQUIDCEXTERN int getMyPort(void);
SQUIDCEXTERN void setUmask(mode_t mask);

SQUIDCEXTERN char *strwordtok(char *buf, char **t);
SQUIDCEXTERN void strwordquote(MemBuf * mb, const char *str);


/*
 * ipc.c
 */
SQUIDCEXTERN pid_t ipcCreate(int type,
                             const char *prog,
                             const char *const args[],
                             const char *name,
                             Ip::Address &local_addr,
                             int *rfd,
                             int *wfd,
                             void **hIpc);


/* CacheDigest */
SQUIDCEXTERN CacheDigest *cacheDigestCreate(int capacity, int bpe);
SQUIDCEXTERN void cacheDigestDestroy(CacheDigest * cd);
SQUIDCEXTERN CacheDigest *cacheDigestClone(const CacheDigest * cd);
SQUIDCEXTERN void cacheDigestClear(CacheDigest * cd);
SQUIDCEXTERN void cacheDigestChangeCap(CacheDigest * cd, int new_cap);
SQUIDCEXTERN int cacheDigestTest(const CacheDigest * cd, const cache_key * key);
SQUIDCEXTERN void cacheDigestAdd(CacheDigest * cd, const cache_key * key);
SQUIDCEXTERN void cacheDigestDel(CacheDigest * cd, const cache_key * key);
SQUIDCEXTERN size_t cacheDigestCalcMaskSize(int cap, int bpe);
SQUIDCEXTERN int cacheDigestBitUtil(const CacheDigest * cd);
SQUIDCEXTERN void cacheDigestGuessStatsUpdate(cd_guess_stats * stats, int real_hit, int guess_hit);
SQUIDCEXTERN void cacheDigestGuessStatsReport(const cd_guess_stats * stats, StoreEntry * sentry, const char *label);
SQUIDCEXTERN void cacheDigestReport(CacheDigest * cd, const char *label, StoreEntry * e);

SQUIDCEXTERN void internalStart(const Comm::ConnectionPointer &clientConn, HttpRequest *, StoreEntry *);
SQUIDCEXTERN int internalCheck(const char *urlpath);
SQUIDCEXTERN int internalStaticCheck(const char *urlpath);
SQUIDCEXTERN char *internalLocalUri(const char *dir, const char *name);
SQUIDCEXTERN char *internalRemoteUri(const char *, unsigned short, const char *, const char *);
SQUIDCEXTERN const char *internalHostname(void);
SQUIDCEXTERN int internalHostnameIs(const char *);

SQUIDCEXTERN void carpInit(void);
SQUIDCEXTERN peer *carpSelectParent(HttpRequest *);

SQUIDCEXTERN void peerUserHashInit(void);
SQUIDCEXTERN peer * peerUserHashSelectParent(HttpRequest * request);

SQUIDCEXTERN void peerSourceHashInit(void);
SQUIDCEXTERN peer * peerSourceHashSelectParent(HttpRequest * request);

#if USE_LEAKFINDER
SQUIDCEXTERN void leakInit(void);
SQUIDCEXTERN void *leakAddFL(void *, const char *, int);
SQUIDCEXTERN void *leakTouchFL(void *, const char *, int);
SQUIDCEXTERN void *leakFreeFL(void *, const char *, int);
#endif

/*
 * prototypes for system functions missing from system includes
 */

#if _SQUID_SOLARIS_

SQUIDCEXTERN int getrusage(int, struct rusage *);
SQUIDCEXTERN int getpagesize(void);
#if !defined(_XPG4_2) && !(defined(__EXTENSIONS__) || \
(!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)))
SQUIDCEXTERN int gethostname(char *, int);
#endif
#endif

/*
 * hack to allow snmp access to the statistics counters
 */
SQUIDCEXTERN StatCounters *snmpStatGet(int);

/* Vary support functions */
SQUIDCEXTERN int varyEvaluateMatch(StoreEntry * entry, HttpRequest * req);

/* CygWin & Windows NT Port */
/* win32.c */
#if _SQUID_WINDOWS_
SQUIDCEXTERN int WIN32_Subsystem_Init(int *, char ***);
SQUIDCEXTERN void WIN32_sendSignal(int);
SQUIDCEXTERN void WIN32_Abort(int);
SQUIDCEXTERN void WIN32_Exit(void);
SQUIDCEXTERN void WIN32_SetServiceCommandLine(void);
SQUIDCEXTERN void WIN32_InstallService(void);
SQUIDCEXTERN void WIN32_RemoveService(void);
SQUIDCEXTERN int SquidMain(int, char **);
#endif /* _SQUID_WINDOWS_ */
#if _SQUID_MSWIN_

SQUIDCEXTERN int WIN32_pipe(int[2]);

SQUIDCEXTERN int WIN32_getrusage(int, struct rusage *);
SQUIDCEXTERN void WIN32_ExceptionHandlerInit(void);

SQUIDCEXTERN int Win32__WSAFDIsSet(int fd, fd_set* set);
SQUIDCEXTERN DWORD WIN32_IpAddrChangeMonitorInit();

#endif

/* external_acl.c */
class external_acl;
        SQUIDCEXTERN void parse_externalAclHelper(external_acl **);

        SQUIDCEXTERN void dump_externalAclHelper(StoreEntry * sentry, const char *name, const external_acl *);

        SQUIDCEXTERN void free_externalAclHelper(external_acl **);

        typedef void EAH(void *data, void *result);
        class ACLChecklist;
            SQUIDCEXTERN void externalAclLookup(ACLChecklist * ch, void *acl_data, EAH * handler, void *data);

            SQUIDCEXTERN void externalAclInit(void);

            SQUIDCEXTERN void externalAclShutdown(void);

            SQUIDCEXTERN char *strtokFile(void);

#if USE_WCCPv2

            SQUIDCEXTERN void parse_wccp2_method(int *v);
            SQUIDCEXTERN void free_wccp2_method(int *v);
            SQUIDCEXTERN void dump_wccp2_method(StoreEntry * e, const char *label, int v);
            SQUIDCEXTERN void parse_wccp2_amethod(int *v);
            SQUIDCEXTERN void free_wccp2_amethod(int *v);
            SQUIDCEXTERN void dump_wccp2_amethod(StoreEntry * e, const char *label, int v);

            SQUIDCEXTERN void parse_wccp2_service(void *v);
            SQUIDCEXTERN void free_wccp2_service(void *v);
            SQUIDCEXTERN void dump_wccp2_service(StoreEntry * e, const char *label, void *v);

            SQUIDCEXTERN int check_null_wccp2_service(void *v);

            SQUIDCEXTERN void parse_wccp2_service_info(void *v);

            SQUIDCEXTERN void free_wccp2_service_info(void *v);

            SQUIDCEXTERN void dump_wccp2_service_info(StoreEntry * e, const char *label, void *v);

#endif

#if USE_AUTH

#if HAVE_AUTH_MODULE_NEGOTIATE && HAVE_KRB5 && HAVE_GSSAPI
            /* upstream proxy authentication */
            SQUIDCEXTERN char *peer_proxy_negotiate_auth(char *principal_name, char *proxy);
#endif

                namespace Auth {
        /* call to ensure the auth component schemes exist. */
        extern void Init(void);
        } // namespace Auth

#endif /* USE_AUTH */

#endif /* SQUID_PROTOS_H */
