#include "squid.h"
#include "AccessLogEntry.h"
#include "HttpRequest.h"

#define STUB_API "HttpRequest.cc"
#include "tests/STUB.h"

HttpRequest::HttpRequest() : HttpMsg(hoRequest) STUB
        HttpRequest::HttpRequest(const HttpRequestMethod& method, AnyP::ProtocolType protocol, const char *aUrlpath) : HttpMsg(hoRequest) STUB
        HttpRequest::~HttpRequest() STUB
        void HttpRequest::packFirstLineInto(Packer * p, bool full_uri) const STUB
        bool HttpRequest::sanityCheckStartLine(MemBuf *buf, const size_t hdr_len, http_status *error) STUB_RETVAL(false)
        void HttpRequest::hdrCacheInit() STUB
        void HttpRequest::reset() STUB
        bool HttpRequest::expectingBody(const HttpRequestMethod& unused, int64_t&) const STUB_RETVAL(false)
        void HttpRequest::initHTTP(const HttpRequestMethod& aMethod, AnyP::ProtocolType aProtocol, const char *aUrlpath) STUB
        bool HttpRequest::parseFirstLine(const char *start, const char *end) STUB_RETVAL(false)
        HttpRequest * HttpRequest::clone() const STUB_RETVAL(NULL)
        bool HttpRequest::inheritProperties(const HttpMsg *aMsg) STUB_RETVAL(false)
        int64_t HttpRequest::getRangeOffsetLimit() STUB_RETVAL(0)
