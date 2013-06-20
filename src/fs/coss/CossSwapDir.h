#ifndef __COSSSWAPDIR_H__
#define __COSSSWAPDIR_H__

class StoreEntry;
class CossSwapDir;
class CossMemBuf;
class DiskIOStrategy;
class DiskIOModule;
class ConfigOptionVector;
class DiskFile;

#include "SwapDir.h"
#include "DiskIO/IORequestor.h"

#ifndef COSS_MEMBUF_SZ
#define	COSS_MEMBUF_SZ	1048576
#endif

/* Note that swap_filen in sio/e are actually disk offsets too! */

/* What we're doing in storeCossAllocate() */
#define COSS_ALLOC_ALLOCATE		1
#define COSS_ALLOC_REALLOC		2

/// \ingroup COSS
class CossSwapDir : public SwapDir, public IORequestor
{

public:
    CossSwapDir();
    virtual void init();
    virtual void create();
    virtual void dump(StoreEntry &)const;
    ~CossSwapDir();
    virtual StoreSearch *search(String const url, HttpRequest *);
    virtual bool unlinkdUseful() const;
    virtual void unlink (StoreEntry &);
    virtual void statfs (StoreEntry &)const;
    virtual bool canStore(const StoreEntry &e, int64_t diskSpaceNeeded, int &load) const;
    virtual int callback();
    virtual void sync();
    virtual StoreIOState::Pointer createStoreIO(StoreEntry &, StoreIOState::STFNCB *, StoreIOState::STIOCB *, void *);
    virtual StoreIOState::Pointer openStoreIO(StoreEntry &, StoreIOState::STFNCB *, StoreIOState::STIOCB *, void *);
    virtual void openLog();
    virtual void closeLog();
    virtual int writeCleanStart();
    virtual void writeCleanDone();
    virtual void logEntry(const StoreEntry & e, int op) const;
    virtual void parse (int index, char *path);
    virtual void reconfigure();
    virtual void swappedOut(const StoreEntry &e);
    virtual uint64_t currentSize() const { return cur_size; }
    virtual uint64_t currentCount() const { return n_disk_objects; }
    /* internals */
    virtual off_t storeCossFilenoToDiskOffset(sfileno);
    virtual sfileno storeCossDiskOffsetToFileno(off_t);
    virtual CossMemBuf *storeCossFilenoToMembuf(sfileno f);
    /* IORequestor routines */
    virtual void ioCompletedNotification();
    virtual void closeCompleted();
    virtual void readCompleted(const char *buf, int len, int errflag, RefCount<ReadRequest>);
    virtual void writeCompleted(int errflag, size_t len, RefCount<WriteRequest>);
    //private:
    int swaplog_fd;
    int count;
    dlink_list membufs;

    CossMemBuf *current_membuf;
    off_t current_offset;	/* in Blocks */
    int numcollisions;
    dlink_list cossindex;
    unsigned int blksz_bits;
    unsigned int blksz_mask;  /* just 1<<blksz_bits - 1*/
    DiskIOStrategy *io;
    RefCount<DiskFile> theFile;
    char *storeCossMemPointerFromDiskOffset(off_t offset, CossMemBuf ** mb);
    void storeCossMemBufUnlock(StoreIOState::Pointer);
    CossMemBuf *createMemBuf(off_t start, sfileno curfn, int *collision);
    sfileno allocate(const StoreEntry * e, int which);
    void startMembuf();
    StoreEntry *addDiskRestore(const cache_key *const key,
                               int file_number,
                               uint64_t swap_file_sz,
                               time_t expires,
                               time_t timestamp,
                               time_t lastref,
                               time_t lastmod,
                               uint32_t refcount,
                               uint16_t flags,
                               int clean);

private:
    void changeIO(DiskIOModule *module);
    bool optionIOParse(char const *option, const char *value, int reconfiguring);
    void optionIODump(StoreEntry * e) const;
    void optionBlockSizeDump(StoreEntry *) const;
    bool optionBlockSizeParse(const char *, const char *, int);
    char const *stripePath() const;
    ConfigOption * getOptionTree() const;
    const char *ioModule;
    mutable ConfigOptionVector *currentIOOptions;
    const char *stripe_path;
    uint64_t cur_size; ///< currently used space in the storage area
    uint64_t n_disk_objects; ///< total number of objects stored
};

/// \ingroup COSS
void storeCossAdd(CossSwapDir *, StoreEntry *);
/// \ingroup COSS
void storeCossRemove(CossSwapDir *, StoreEntry *);
/// \ingroup COSS
void storeCossStartMembuf(CossSwapDir * SD);

#include "StoreSearch.h"

/// \ingroup COSS
class StoreSearchCoss : public StoreSearch
{

public:
    StoreSearchCoss(RefCount<CossSwapDir> sd);
    StoreSearchCoss(StoreSearchCoss const &);
    ~StoreSearchCoss();
    /* Iterator API - garh, wrong place */
    /* callback the client when a new StoreEntry is available
     * or an error occurs
     */
    virtual void next(void (callback)(void *cbdata), void *cbdata);
    /* return true if a new StoreEntry is immediately available */
    virtual bool next();
    virtual bool error() const;
    virtual bool isDone() const;
    virtual StoreEntry *currentItem();

private:
    RefCount<CossSwapDir> sd;
    void (*callback)(void *cbdata);
    void *cbdata;
    bool _done;
    dlink_node * current;
    dlink_node * next_;

    CBDATA_CLASS2(StoreSearchCoss);
};

#endif
