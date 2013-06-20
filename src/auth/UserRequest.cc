/*
 * DEBUG: section 29    Authenticator
 * AUTHOR:  Robert Collins
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

/* The functions in this file handle authentication.
 * They DO NOT perform access control or auditing.
 * See acl.c for access control and client_side.c for auditing */

#include "squid.h"
#include "auth/Config.h"
#include "auth/Scheme.h"
#include "auth/UserRequest.h"
#include "auth/User.h"
#include "client_side.h"
#include "comm/Connection.h"
#include "HttpReply.h"
#include "HttpRequest.h"

/* Generic Functions */

char const *
Auth::UserRequest::username() const
{
    if (user() != NULL)
        return user()->username();
    else
        return NULL;
}

/**** PUBLIC FUNCTIONS (ALL GENERIC!)  ****/

/* send the initial data to an authenticator module */
void
Auth::UserRequest::start(AUTHCB * handler, void *data)
{
    assert(handler);
    assert(data);
    debugs(29, 9, HERE << "auth_user_request '" << this << "'");
    module_start(handler, data);
}

bool
Auth::UserRequest::valid() const
{
    debugs(29, 9, HERE << "Validating Auth::UserRequest '" << this << "'.");

    if (user() == NULL) {
        debugs(29, 4, HERE << "No associated Auth::User data");
        return false;
    }

    if (user()->auth_type == Auth::AUTH_UNKNOWN) {
        debugs(29, 4, HERE << "Auth::User '" << user() << "' uses unknown scheme.");
        return false;
    }

    if (user()->auth_type == Auth::AUTH_BROKEN) {
        debugs(29, 4, HERE << "Auth::User '" << user() << "' is broken for it's scheme.");
        return false;
    }

    /* any other sanity checks that we need in the future */

    /* finally return ok */
    debugs(29, 5, HERE << "Validated. Auth::UserRequest '" << this << "'.");
    return true;
}

void *
Auth::UserRequest::operator new (size_t byteCount)
{
    fatal("Auth::UserRequest not directly allocatable\n");
    return (void *)1;
}

void
Auth::UserRequest::operator delete (void *address)
{
    fatal("Auth::UserRequest child failed to override operator delete\n");
}

Auth::UserRequest::UserRequest():
        _auth_user(NULL),
        message(NULL),
        lastReply(AUTH_ACL_CANNOT_AUTHENTICATE)
{
    debugs(29, 5, HERE << "initialised request " << this);
}

Auth::UserRequest::~UserRequest()
{
    assert(RefCountCount()==0);
    debugs(29, 5, HERE << "freeing request " << this);

    if (user() != NULL) {
        /* release our references to the user credentials */
        user(NULL);
    }

    safe_free(message);
}

void
Auth::UserRequest::setDenyMessage(char const *aString)
{
    safe_free(message);
    message = xstrdup(aString);
}

char const *
Auth::UserRequest::getDenyMessage()
{
    return message;
}

char const *
Auth::UserRequest::denyMessage(char const * const default_message)
{
    if (this == NULL || getDenyMessage() == NULL) {
        return default_message;
    }

    return getDenyMessage();
}

static void
authenticateAuthUserRequestSetIp(Auth::UserRequest::Pointer auth_user_request, Ip::Address &ipaddr)
{
    Auth::User::Pointer auth_user = auth_user_request->user();

    if (!auth_user)
        return;

    auth_user->addIp(ipaddr);
}

void
authenticateAuthUserRequestRemoveIp(Auth::UserRequest::Pointer auth_user_request, Ip::Address const &ipaddr)
{
    Auth::User::Pointer auth_user = auth_user_request->user();

    if (!auth_user)
        return;

    auth_user->removeIp(ipaddr);
}

void
authenticateAuthUserRequestClearIp(Auth::UserRequest::Pointer auth_user_request)
{
    if (auth_user_request != NULL)
        auth_user_request->user()->clearIp();
}

int
authenticateAuthUserRequestIPCount(Auth::UserRequest::Pointer auth_user_request)
{
    assert(auth_user_request != NULL);
    assert(auth_user_request->user() != NULL);
    return auth_user_request->user()->ipcount;
}

/*
 * authenticateUserAuthenticated: is this auth_user structure logged in ?
 */
int
authenticateUserAuthenticated(Auth::UserRequest::Pointer auth_user_request)
{
    if (auth_user_request == NULL || !auth_user_request->valid())
        return 0;

    return auth_user_request->authenticated();
}

Auth::Direction
Auth::UserRequest::direction()
{
    if (user() == NULL)
        return Auth::CRED_ERROR; // No credentials. Should this be a CHALLENGE instead?

    if (authenticateUserAuthenticated(this))
        return Auth::CRED_VALID;

    return module_direction();
}

void
Auth::UserRequest::addAuthenticationInfoHeader(HttpReply * rep, int accelerated)
{}

void
Auth::UserRequest::addAuthenticationInfoTrailer(HttpReply * rep, int accelerated)
{}

void
Auth::UserRequest::onConnectionClose(ConnStateData *)
{}

const char *
Auth::UserRequest::connLastHeader()
{
    fatal("Auth::UserRequest::connLastHeader should always be overridden by conn based auth schemes");
    return NULL;
}

/*
 * authenticateAuthenticateUser: call the module specific code to
 * log this user request in.
 * Cache hits may change the auth_user pointer in the structure if needed.
 * This is basically a handle approach.
 */
static void
authenticateAuthenticateUser(Auth::UserRequest::Pointer auth_user_request, HttpRequest * request, ConnStateData * conn, http_hdr_type type)
{
    assert(auth_user_request.getRaw() != NULL);

    auth_user_request->authenticate(request, conn, type);
}

static Auth::UserRequest::Pointer
authTryGetUser(Auth::UserRequest::Pointer auth_user_request, ConnStateData * conn, HttpRequest * request)
{
    if (auth_user_request != NULL)
        return auth_user_request;
    else if (request != NULL && request->auth_user_request != NULL)
        return request->auth_user_request;
    else if (conn != NULL)
        return conn->auth_user_request;
    else
        return NULL;
}

/* returns one of
 * AUTH_ACL_CHALLENGE,
 * AUTH_ACL_HELPER,
 * AUTH_ACL_CANNOT_AUTHENTICATE,
 * AUTH_AUTHENTICATED
 *
 * How to use: In your proxy-auth dependent acl code, use the following
 * construct:
 * int rv;
 * if ((rv = AuthenticateAuthenticate()) != AUTH_AUTHENTICATED)
 *   return rv;
 *
 * when this code is reached, the request/connection is authenticated.
 *
 * if you have non-acl code, but want to force authentication, you need a
 * callback mechanism like the acl testing routines that will send a 40[1|7] to
 * the client when rv==AUTH_ACL_CHALLENGE, and will communicate with
 * the authenticateStart routine for rv==AUTH_ACL_HELPER
 *
 * Caller is responsible for locking and unlocking their *auth_user_request!
 */
AuthAclState
Auth::UserRequest::authenticate(Auth::UserRequest::Pointer * auth_user_request, http_hdr_type headertype, HttpRequest * request, ConnStateData * conn, Ip::Address &src_addr)
{
    const char *proxy_auth;
    assert(headertype != 0);

    proxy_auth = request->header.getStr(headertype);

    /*
     * a note on proxy_auth logix here:
     * proxy_auth==NULL -> unauthenticated request || already
     * authenticated connection so we test for an authenticated
     * connection when we recieve no authentication header.
     */

    /* a) can we find other credentials to use? and b) are they logged in already? */
    if (proxy_auth == NULL && !authenticateUserAuthenticated(authTryGetUser(*auth_user_request,conn,request))) {
        /* no header or authentication failed/got corrupted - restart */
        debugs(29, 4, HERE << "No Proxy-Auth header and no working alternative. Requesting auth header.");

        /* something wrong with the AUTH credentials. Force a new attempt */

        /* connection auth we must reset on auth errors */
        if (conn != NULL) {
            conn->auth_user_request = NULL;
        }

        *auth_user_request = NULL;
        return AUTH_ACL_CHALLENGE;
    }

    /*
     * Is this an already authenticated connection with a new auth header?
     * No check for function required in the if: its compulsory for conn based
     * auth modules
     */
    if (proxy_auth && conn != NULL && conn->auth_user_request != NULL &&
            authenticateUserAuthenticated(conn->auth_user_request) &&
            conn->auth_user_request->connLastHeader() != NULL &&
            strcmp(proxy_auth, conn->auth_user_request->connLastHeader())) {
        debugs(29, 2, "WARNING: DUPLICATE AUTH - authentication header on already authenticated connection!. AU " <<
               conn->auth_user_request << ", Current user '" <<
               conn->auth_user_request->username() << "' proxy_auth " <<
               proxy_auth);

        /* remove this request struct - the link is already authed and it can't be to reauth. */

        /* This should _only_ ever occur on the first pass through
         * authenticateAuthenticate
         */
        assert(*auth_user_request == NULL);
        conn->auth_user_request = NULL;
    }

    /* we have a proxy auth header and as far as we know this connection has
     * not had bungled connection oriented authentication happen on it. */
    debugs(29, 9, HERE << "header " << (proxy_auth ? proxy_auth : "-") << ".");

    if (*auth_user_request == NULL) {
        if (conn != NULL) {
            debugs(29, 9, HERE << "This is a new checklist test on:" << conn->clientConnection);
        }

        if (proxy_auth && request->auth_user_request == NULL && conn != NULL && conn->auth_user_request != NULL) {
            Auth::Config * scheme = Auth::Config::Find(proxy_auth);

            if (conn->auth_user_request->user() == NULL || conn->auth_user_request->user()->config != scheme) {
                debugs(29, DBG_IMPORTANT, "WARNING: Unexpected change of authentication scheme from '" <<
                       conn->auth_user_request->user()->config->type() <<
                       "' to '" << proxy_auth << "' (client " <<
                       src_addr << ")");

                conn->auth_user_request = NULL;
            }
        }

        if (request->auth_user_request == NULL && (conn == NULL || conn->auth_user_request == NULL)) {
            /* beginning of a new request check */
            debugs(29, 4, HERE << "No connection authentication type");

            *auth_user_request = Auth::Config::CreateAuthUser(proxy_auth);
            if (*auth_user_request == NULL)
                return AUTH_ACL_CHALLENGE;
            else if (!(*auth_user_request)->valid()) {
                /* the decode might have left a username for logging, or a message to
                 * the user */

                if ((*auth_user_request)->username()) {
                    request->auth_user_request = *auth_user_request;
                }

                *auth_user_request = NULL;
                return AUTH_ACL_CHALLENGE;
            }

        } else if (request->auth_user_request != NULL) {
            *auth_user_request = request->auth_user_request;
        } else {
            assert (conn != NULL);
            if (conn->auth_user_request != NULL) {
                *auth_user_request = conn->auth_user_request;
            } else {
                /* failed connection based authentication */
                debugs(29, 4, HERE << "Auth user request " <<
                       *auth_user_request << " conn-auth user request " <<
                       conn->auth_user_request << " conn type " <<
                       conn->auth_user_request->user()->auth_type << " authentication failed.");

                *auth_user_request = NULL;
                return AUTH_ACL_CHALLENGE;
            }
        }
    }

    if (!authenticateUserAuthenticated(*auth_user_request)) {
        /* User not logged in. Try to log them in */
        authenticateAuthenticateUser(*auth_user_request, request, conn, headertype);

        switch ((*auth_user_request)->direction()) {

        case Auth::CRED_CHALLENGE:

            if (request->auth_user_request == NULL) {
                request->auth_user_request = *auth_user_request;
            }

            /* fallthrough to ERROR case and do the challenge */

        case Auth::CRED_ERROR:
            /* this ACL check is finished. */
            *auth_user_request = NULL;
            return AUTH_ACL_CHALLENGE;

        case Auth::CRED_LOOKUP:
            /* we are partway through authentication within squid,
             * the *auth_user_request variables stores the auth_user_request
             * for the callback to here - Do not Unlock */
            return AUTH_ACL_HELPER;

        case Auth::CRED_VALID:
            /* authentication is finished */
            /* See if user authentication failed for some reason */
            if (!authenticateUserAuthenticated(*auth_user_request)) {
                if ((*auth_user_request)->username()) {
                    if (!request->auth_user_request) {
                        request->auth_user_request = *auth_user_request;
                    }
                }

                *auth_user_request = NULL;
                return AUTH_ACL_CHALLENGE;
            }
            // otherwise fallthrough to acceptance.
        }
    }

    /* copy username to request for logging on client-side */
    /* the credentials are correct at this point */
    if (request->auth_user_request == NULL) {
        request->auth_user_request = *auth_user_request;
        authenticateAuthUserRequestSetIp(*auth_user_request, src_addr);
    }

    return AUTH_AUTHENTICATED;
}

AuthAclState
Auth::UserRequest::tryToAuthenticateAndSetAuthUser(Auth::UserRequest::Pointer * aUR, http_hdr_type headertype, HttpRequest * request, ConnStateData * conn, Ip::Address &src_addr)
{
    // If we have already been called, return the cached value
    Auth::UserRequest::Pointer t = authTryGetUser(*aUR, conn, request);

    if (t != NULL && t->lastReply != AUTH_ACL_CANNOT_AUTHENTICATE && t->lastReply != AUTH_ACL_HELPER) {
        if (*aUR == NULL)
            *aUR = t;

        if (request->auth_user_request == NULL && t->lastReply == AUTH_AUTHENTICATED) {
            request->auth_user_request = t;
        }
        return t->lastReply;
    }

    // ok, call the actual authenticator routine.
    AuthAclState result = authenticate(aUR, headertype, request, conn, src_addr);

    // auth process may have changed the UserRequest we are dealing with
    t = authTryGetUser(*aUR, conn, request);

    if (t != NULL && result != AUTH_ACL_CANNOT_AUTHENTICATE && result != AUTH_ACL_HELPER)
        t->lastReply = result;

    return result;
}

void
Auth::UserRequest::addReplyAuthHeader(HttpReply * rep, Auth::UserRequest::Pointer auth_user_request, HttpRequest * request, int accelerated, int internal)
/* send the auth types we are configured to support (and have compiled in!) */
{
    http_hdr_type type;

    switch (rep->sline.status) {

    case HTTP_PROXY_AUTHENTICATION_REQUIRED:
        /* Proxy authorisation needed */
        type = HDR_PROXY_AUTHENTICATE;
        break;

    case HTTP_UNAUTHORIZED:
        /* WWW Authorisation needed */
        type = HDR_WWW_AUTHENTICATE;
        break;

    default:
        /* Keep GCC happy */
        /* some other HTTP status */
        type = HDR_ENUM_END;
        break;
    }

    debugs(29, 9, HERE << "headertype:" << type << " authuser:" << auth_user_request);

    if (((rep->sline.status == HTTP_PROXY_AUTHENTICATION_REQUIRED)
            || (rep->sline.status == HTTP_UNAUTHORIZED)) && internal)
        /* this is a authenticate-needed response */
    {

        if (auth_user_request != NULL && auth_user_request->direction() == Auth::CRED_CHALLENGE)
            /* add the scheme specific challenge header to the response */
            auth_user_request->user()->config->fixHeader(auth_user_request, rep, type, request);
        else {
            /* call each configured & running authscheme */

            for (Auth::ConfigVector::iterator  i = Auth::TheConfig.begin(); i != Auth::TheConfig.end(); ++i) {
                Auth::Config *scheme = *i;

                if (scheme->active())
                    scheme->fixHeader(NULL, rep, type, request);
                else
                    debugs(29, 4, HERE << "Configured scheme " << scheme->type() << " not Active");
            }
        }

    }

    /*
     * allow protocol specific headers to be _added_ to the existing
     * response - currently Digest or Negotiate auth
     */
    if (auth_user_request != NULL) {
        auth_user_request->addAuthenticationInfoHeader(rep, accelerated);
        if (auth_user_request->lastReply != AUTH_AUTHENTICATED)
            auth_user_request->lastReply = AUTH_ACL_CANNOT_AUTHENTICATE;
    }
}

// TODO remove wrapper.
void
authenticateFixHeader(HttpReply * rep, Auth::UserRequest::Pointer auth_user_request, HttpRequest * request, int accelerated, int internal)
{
    Auth::UserRequest::addReplyAuthHeader(rep, auth_user_request, request, accelerated, internal);
}

/* call the active auth module and allow it to add a trailer to the request */
// TODO remove wrapper
void
authenticateAddTrailer(HttpReply * rep, Auth::UserRequest::Pointer auth_user_request, HttpRequest * request, int accelerated)
{
    if (auth_user_request != NULL)
        auth_user_request->addAuthenticationInfoTrailer(rep, accelerated);
}

Auth::Scheme::Pointer
Auth::UserRequest::scheme() const
{
    return Auth::Scheme::Find(user()->config->type());
}
