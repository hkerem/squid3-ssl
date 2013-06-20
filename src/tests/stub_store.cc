#include "squid.h"
#include "RequestFlags.h"

#define STUB_API "store.cc"
#include "tests/STUB.h"

/* and code defined in the wrong .cc file */
#include "SwapDir.h"
void StoreController::maintain() STUB
#include "RemovalPolicy.h"
RemovalPolicy * createRemovalPolicy(RemovalPolicySettings * settings) STUB_RETVAL(NULL)

#include "Store.h"
StorePointer Store::CurrentRoot = NULL;
StoreIoStats store_io_stats;
bool StoreEntry::checkDeferRead(int fd) const STUB_RETVAL(false)
const char *StoreEntry::getMD5Text() const STUB_RETVAL(NULL)
StoreEntry::StoreEntry() STUB
StoreEntry::StoreEntry(const char *url, const char *log_url) STUB
StoreEntry::~StoreEntry() STUB
HttpReply const *StoreEntry::getReply() const STUB_RETVAL(NULL)
void StoreEntry::write(StoreIOBuffer) STUB
bool StoreEntry::isAccepting() const STUB_RETVAL(false)
size_t StoreEntry::bytesWanted(Range<size_t> const, bool) const STUB_RETVAL(0)
void StoreEntry::complete() STUB
store_client_t StoreEntry::storeClientType() const STUB_RETVAL(STORE_NON_CLIENT)
char const *StoreEntry::getSerialisedMetaData() STUB_RETVAL(NULL)
void StoreEntry::replaceHttpReply(HttpReply *, bool andStartWriting) STUB
bool StoreEntry::mayStartSwapOut() STUB_RETVAL(false)
void StoreEntry::trimMemory(const bool preserveSwappable) STUB
void StoreEntry::abort() STUB
void StoreEntry::unlink() STUB
void StoreEntry::makePublic() STUB
void StoreEntry::makePrivate() STUB
void StoreEntry::setPublicKey() STUB
void StoreEntry::setPrivateKey() STUB
void StoreEntry::expireNow() STUB
void StoreEntry::releaseRequest() STUB
void StoreEntry::negativeCache() STUB
void StoreEntry::cacheNegatively() STUB
void StoreEntry::invokeHandlers() STUB
void StoreEntry::purgeMem() STUB
void StoreEntry::swapOut() STUB
void StoreEntry::swapOutFileClose(int how) STUB
const char *StoreEntry::url() const STUB_RETVAL(NULL)
int StoreEntry::checkCachable() STUB_RETVAL(0)
int StoreEntry::checkNegativeHit() const STUB_RETVAL(0)
int StoreEntry::locked() const STUB_RETVAL(0)
int StoreEntry::validToSend() const STUB_RETVAL(0)
bool StoreEntry::memoryCachable() const STUB_RETVAL(false)
void StoreEntry::createMemObject(const char *, const char *) STUB
void StoreEntry::hideMemObject() STUB
void StoreEntry::dump(int debug_lvl) const STUB
void StoreEntry::hashDelete() STUB
void StoreEntry::hashInsert(const cache_key *) STUB
void StoreEntry::registerAbort(STABH * cb, void *) STUB
void StoreEntry::reset() STUB
void StoreEntry::setMemStatus(mem_status_t) STUB
void StoreEntry::timestampsSet() STUB
void StoreEntry::unregisterAbort() STUB
void StoreEntry::destroyMemObject() STUB
int StoreEntry::checkTooSmall() STUB_RETVAL(0)
void StoreEntry::delayAwareRead(const Comm::ConnectionPointer&, char *buf, int len, AsyncCall::Pointer callback) STUB
void StoreEntry::setNoDelay (bool const) STUB
bool StoreEntry::modifiedSince(HttpRequest * request) const STUB_RETVAL(false)
bool StoreEntry::hasIfMatchEtag(const HttpRequest &request) const STUB_RETVAL(false)
bool StoreEntry::hasIfNoneMatchEtag(const HttpRequest &request) const STUB_RETVAL(false)
RefCount<SwapDir> StoreEntry::store() const STUB_RETVAL(NULL)
size_t StoreEntry::inUseCount() STUB_RETVAL(0)
void StoreEntry::getPublicByRequestMethod(StoreClient * aClient, HttpRequest * request, const HttpRequestMethod& method) STUB
void StoreEntry::getPublicByRequest(StoreClient * aClient, HttpRequest * request) STUB
void StoreEntry::getPublic(StoreClient * aClient, const char *uri, const HttpRequestMethod& method) STUB
void *StoreEntry::operator new(size_t byteCount)
{
    STUB
    return new StoreEntry();
}
void StoreEntry::operator delete(void *address) STUB
void StoreEntry::setReleaseFlag() STUB
//#if USE_SQUID_ESI
//ESIElement::Pointer StoreEntry::cachedESITree STUB_RETVAL(NULL)
//#endif
void StoreEntry::append(char const *, int len) STUB
void StoreEntry::buffer() STUB
void StoreEntry::flush() STUB
int StoreEntry::unlock() STUB_RETVAL(0)
int64_t StoreEntry::objectLen() const STUB_RETVAL(0)
int64_t StoreEntry::contentLen() const STUB_RETVAL(0)
void StoreEntry::lock() STUB
void StoreEntry::release() STUB

NullStoreEntry *NullStoreEntry::getInstance() STUB_RETVAL(NULL)
const char *NullStoreEntry::getMD5Text() const STUB_RETVAL(NULL)
void NullStoreEntry::operator delete(void *address) STUB
// private virtual. Why is this linked from outside?
const char *NullStoreEntry::getSerialisedMetaData() STUB_RETVAL(NULL)

void Store::Root(Store *) STUB
void Store::Root(RefCount<Store>) STUB
void Store::Stats(StoreEntry * output) STUB
void Store::Maintain(void *unused) STUB
void Store::create() STUB
void Store::diskFull() STUB
void Store::sync() STUB
void Store::unlink(StoreEntry &) STUB

std::ostream &operator <<(std::ostream &os, const StoreEntry &)
{
    STUB
    return os;
}

size_t storeEntryInUse() STUB_RETVAL(0)
const char *storeEntryFlags(const StoreEntry *) STUB_RETVAL(NULL)
void storeEntryReplaceObject(StoreEntry *, HttpReply *) STUB
StoreEntry *storeGetPublic(const char *uri, const HttpRequestMethod& method) STUB_RETVAL(NULL)
StoreEntry *storeGetPublicByRequest(HttpRequest * request) STUB_RETVAL(NULL)
StoreEntry *storeGetPublicByRequestMethod(HttpRequest * request, const HttpRequestMethod& method) STUB_RETVAL(NULL)
StoreEntry *storeCreateEntry(const char *, const char *, const RequestFlags &, const HttpRequestMethod&) STUB_RETVAL(NULL)
void storeInit(void) STUB
void storeConfigure(void) STUB
void storeFreeMemory(void) STUB
int expiresMoreThan(time_t, time_t) STUB_RETVAL(0)
void storeAppendPrintf(StoreEntry *, const char *,...) STUB
void storeAppendVPrintf(StoreEntry *, const char *, va_list ap) STUB
int storeTooManyDiskFilesOpen(void) STUB_RETVAL(0)
void storeHeapPositionUpdate(StoreEntry *, SwapDir *) STUB
void storeSwapFileNumberSet(StoreEntry * e, sfileno filn) STUB
void storeFsInit(void) STUB
void storeFsDone(void) STUB
void storeReplAdd(const char *, REMOVALPOLICYCREATE *) STUB
void destroyStoreEntry(void *) STUB
// in Packer.cc !? void packerToStoreInit(Packer * p, StoreEntry * e) STUB
void storeGetMemSpace(int size) STUB

#if !_USE_INLINE_
#include "Store.cci"
#endif
