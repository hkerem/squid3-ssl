#include "squid.h"
#include "acl/FilledChecklist.h"
#include "client_side.h"
#include "comm/Connection.h"
#include "comm/forward.h"
#include "HttpReply.h"
#include "HttpRequest.h"
#include "SquidConfig.h"
#if USE_AUTH
#include "auth/UserRequest.h"
#include "auth/AclProxyAuth.h"
#endif

CBDATA_CLASS_INIT(ACLFilledChecklist);

void *
ACLFilledChecklist::operator new (size_t size)
{
    assert (size == sizeof(ACLFilledChecklist));
    CBDATA_INIT_TYPE(ACLFilledChecklist);
    ACLFilledChecklist *result = cbdataAlloc(ACLFilledChecklist);
    return result;
}

void
ACLFilledChecklist::operator delete (void *address)
{
    ACLFilledChecklist *t = static_cast<ACLFilledChecklist *>(address);
    cbdataFree(t);
}

ACLFilledChecklist::ACLFilledChecklist() :
        dst_peer(NULL),
        dst_rdns(NULL),
        request (NULL),
        reply (NULL),
#if USE_AUTH
        auth_user_request (NULL),
#endif
#if SQUID_SNMP
        snmp_community(NULL),
#endif
#if USE_SSL
        sslErrors(NULL),
#endif
        extacl_entry (NULL),
        conn_(NULL),
        fd_(-1),
        destinationDomainChecked_(false),
        sourceDomainChecked_(false)
{
    my_addr.SetEmpty();
    src_addr.SetEmpty();
    dst_addr.SetEmpty();
    rfc931[0] = '\0';
}

ACLFilledChecklist::~ACLFilledChecklist()
{
    assert (!asyncInProgress());

    safe_free(dst_rdns); // created by xstrdup().

    if (extacl_entry)
        cbdataReferenceDone(extacl_entry);

    HTTPMSGUNLOCK(request);

    HTTPMSGUNLOCK(reply);

    cbdataReferenceDone(conn_);

#if USE_SSL
    cbdataReferenceDone(sslErrors);
#endif

    debugs(28, 4, HERE << "ACLFilledChecklist destroyed " << this);
}

ConnStateData *
ACLFilledChecklist::conn() const
{
    return  conn_;
}

void
ACLFilledChecklist::conn(ConnStateData *aConn)
{
    if (conn() == aConn)
        return;
    assert (conn() == NULL);
    conn_ = cbdataReference(aConn);
}

int
ACLFilledChecklist::fd() const
{
    return (conn_ != NULL && conn_->clientConnection != NULL) ? conn_->clientConnection->fd : fd_;
}

void
ACLFilledChecklist::fd(int aDescriptor)
{
    assert(!conn() || conn()->clientConnection == NULL || conn()->clientConnection->fd == aDescriptor);
    fd_ = aDescriptor;
}

bool
ACLFilledChecklist::destinationDomainChecked() const
{
    return destinationDomainChecked_;
}

void
ACLFilledChecklist::markDestinationDomainChecked()
{
    assert (!finished() && !destinationDomainChecked());
    destinationDomainChecked_ = true;
}

bool
ACLFilledChecklist::sourceDomainChecked() const
{
    return sourceDomainChecked_;
}

void
ACLFilledChecklist::markSourceDomainChecked()
{
    assert (!finished() && !sourceDomainChecked());
    sourceDomainChecked_ = true;
}

/*
 * There are two common ACLFilledChecklist lifecycles paths:
 *
 * A) Using aclCheckFast(): The caller creates an ACLFilledChecklist object
 *    on stack and calls aclCheckFast().
 *
 * B) Using aclNBCheck() and callbacks: The caller allocates an
 *    ACLFilledChecklist object (via operator new) and passes it to
 *    aclNBCheck(). Control eventually passes to ACLChecklist::checkCallback(),
 *    which will invoke the callback function as requested by the
 *    original caller of aclNBCheck().  This callback function must
 *    *not* delete the list.  After the callback function returns,
 *    checkCallback() will delete the list (i.e., self).
 */
ACLFilledChecklist::ACLFilledChecklist(const acl_access *A, HttpRequest *http_request, const char *ident):
        dst_peer(NULL),
        dst_rdns(NULL),
        request(NULL),
        reply(NULL),
#if USE_AUTh
        auth_user_request(NULL),
#endif
#if SQUID_SNMP
        snmp_community(NULL),
#endif
#if USE_SSL
        sslErrors(NULL),
#endif
        extacl_entry (NULL),
        conn_(NULL),
        fd_(-1),
        destinationDomainChecked_(false),
        sourceDomainChecked_(false)
{
    my_addr.SetEmpty();
    src_addr.SetEmpty();
    dst_addr.SetEmpty();
    rfc931[0] = '\0';

    // cbdataReferenceDone() is in either fastCheck() or the destructor
    if (A)
        accessList = cbdataReference(A);

    if (http_request != NULL) {
        request = HTTPMSGLOCK(http_request);
#if FOLLOW_X_FORWARDED_FOR
        if (Config.onoff.acl_uses_indirect_client)
            src_addr = request->indirect_client_addr;
        else
#endif /* FOLLOW_X_FORWARDED_FOR */
            src_addr = request->client_addr;
        my_addr = request->my_addr;

        if (request->clientConnectionManager.valid())
            conn(request->clientConnectionManager.get());
    }

#if USE_IDENT
    if (ident)
        xstrncpy(rfc931, ident, USER_IDENT_SZ);
#endif
}

