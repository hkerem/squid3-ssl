/*
 * -----------------------------------------------------------------------------
 *
 * Author: Markus Moeller (markus_moeller at compuserve.com)
 *
 * Copyright (C) 2007 Markus Moeller. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 *   As a special exemption, M Moeller gives permission to link this program
 *   with MIT, Heimdal or other GSS/Kerberos libraries, and distribute
 *   the resulting executable, without including the source code for
 *   the Libraries in the source distribution.
 *
 * -----------------------------------------------------------------------------
 */
/*
 * Hosted at http://sourceforge.net/projects/squidkerbauth
 */
#include "squid.h"
#include "rfc1738.h"
#include "compat/getaddrinfo.h"
#include "compat/getnameinfo.h"

#if HAVE_GSSAPI

#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDOI_H
#include <stdio.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_TIME_H
#include <time.h>
#endif

#include "util.h"
#include "base64.h"

#if HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#elif HAVE_GSSAPI_H
#include <gssapi.h>
#endif

#if !HAVE_HEIMDAL_KERBEROS
#if HAVE_GSSAPI_GSSAPI_KRB5_H
#include <gssapi/gssapi_krb5.h>
#endif
#if HAVE_GSSAPI_GSSAPI_GENERIC_H
#include <gssapi/gssapi_generic.h>
#endif
#if HAVE_GSSAPI_GSSAPI_EXT_H
#include <gssapi/gssapi_ext.h>
#endif
#endif

#ifndef gss_nt_service_name
#define gss_nt_service_name GSS_C_NT_HOSTBASED_SERVICE
#endif

#define PROGRAM "negotiate_kerberos_auth"

#ifndef MAX_AUTHTOKEN_LEN
#define MAX_AUTHTOKEN_LEN   65535
#endif
#ifndef SQUID_KERB_AUTH_VERSION
#define SQUID_KERB_AUTH_VERSION "3.0.4sq"
#endif

int check_gss_err(OM_uint32 major_status, OM_uint32 minor_status,
                  const char *function, int log);
char *gethost_name(void);
static const char *LogTime(void);

static const unsigned char ntlmProtocol[] = {'N', 'T', 'L', 'M', 'S', 'S', 'P', 0};

static const char *
LogTime()
{
    struct tm *tm;
    struct timeval now;
    static time_t last_t = 0;
    static char buf[128];

    gettimeofday(&now, NULL);
    if (now.tv_sec != last_t) {
        tm = localtime((time_t *) & now.tv_sec);
        strftime(buf, 127, "%Y/%m/%d %H:%M:%S", tm);
        last_t = now.tv_sec;
    }
    return buf;
}

char *
gethost_name(void)
{
    /*
     * char hostname[sysconf(_SC_HOST_NAME_MAX)];
     */
    char hostname[1024];
    struct addrinfo *hres = NULL, *hres_list;
    int rc, count;

    rc = gethostname(hostname, sizeof(hostname)-1);
    if (rc) {
        fprintf(stderr, "%s| %s: ERROR: resolving hostname '%s' failed\n",
                LogTime(), PROGRAM, hostname);
        return NULL;
    }
    rc = getaddrinfo(hostname, NULL, NULL, &hres);
    if (rc != 0) {
        fprintf(stderr,
                "%s| %s: ERROR: resolving hostname with getaddrinfo: %s failed\n",
                LogTime(), PROGRAM, gai_strerror(rc));
        return NULL;
    }
    hres_list = hres;
    count = 0;
    while (hres_list) {
        ++count;
        hres_list = hres_list->ai_next;
    }
    rc = getnameinfo(hres->ai_addr, hres->ai_addrlen, hostname,
                     sizeof(hostname), NULL, 0, 0);
    if (rc != 0) {
        fprintf(stderr,
                "%s| %s: ERROR: resolving ip address with getnameinfo: %s failed\n",
                LogTime(), PROGRAM, gai_strerror(rc));
        freeaddrinfo(hres);
        return NULL;
    }
    freeaddrinfo(hres);
    hostname[sizeof(hostname)-1] = '\0';
    return (xstrdup(hostname));
}

int
check_gss_err(OM_uint32 major_status, OM_uint32 minor_status,
              const char *function, int log)
{
    if (GSS_ERROR(major_status)) {
        OM_uint32 maj_stat, min_stat;
        OM_uint32 msg_ctx = 0;
        gss_buffer_desc status_string;
        char buf[1024];
        size_t len;

        len = 0;
        msg_ctx = 0;
        do {
            /* convert major status code (GSS-API error) to text */
            maj_stat = gss_display_status(&min_stat, major_status,
                                          GSS_C_GSS_CODE, GSS_C_NULL_OID, &msg_ctx, &status_string);
            if (maj_stat == GSS_S_COMPLETE && status_string.length > 0) {
                if (sizeof(buf) > len + status_string.length + 1) {
                    snprintf(buf + len, (sizeof(buf) - len), "%s", (char *) status_string.value);
                    len += status_string.length;
                }
            } else
                msg_ctx = 0;
            gss_release_buffer(&min_stat, &status_string);
        } while (msg_ctx);
        if (sizeof(buf) > len + 2) {
            snprintf(buf + len, (sizeof(buf) - len), "%s", ". ");
            len += 2;
        }
        msg_ctx = 0;
        do {
            /* convert minor status code (underlying routine error) to text */
            maj_stat = gss_display_status(&min_stat, minor_status,
                                          GSS_C_MECH_CODE, GSS_C_NULL_OID, &msg_ctx, &status_string);
            if (maj_stat == GSS_S_COMPLETE && status_string.length > 0) {
                if (sizeof(buf) > len + status_string.length) {
                    snprintf(buf + len, (sizeof(buf) - len), "%s", (char *) status_string.value);
                    len += status_string.length;
                }
            } else
                msg_ctx = 0;
            gss_release_buffer(&min_stat, &status_string);
        } while (msg_ctx);
        debug((char *) "%s| %s: ERROR: %s failed: %s\n", LogTime(), PROGRAM, function, buf);
        fprintf(stdout, "BH %s failed: %s\n", function, buf);
        if (log)
            fprintf(stderr, "%s| %s: INFO: User not authenticated\n", LogTime(),
                    PROGRAM);
        return (1);
    }
    return (0);
}

int
main(int argc, char *const argv[])
{
    char buf[MAX_AUTHTOKEN_LEN];
    char *c, *p;
    char *user = NULL;
    int length = 0;
    static int err = 0;
    int opt, log = 0, norealm = 0;
    OM_uint32 ret_flags = 0, spnego_flag = 0;
    char *service_name = (char *) "HTTP", *host_name = NULL;
    char *token = NULL;
    char *service_principal = NULL;
    OM_uint32 major_status, minor_status;
    gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
    gss_name_t client_name = GSS_C_NO_NAME;
    gss_name_t server_name = GSS_C_NO_NAME;
    gss_cred_id_t server_creds = GSS_C_NO_CREDENTIAL;
    gss_buffer_desc service = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    const unsigned char *kerberosToken = NULL;
    const unsigned char *spnegoToken = NULL;
    size_t spnegoTokenLength = 0;

    setbuf(stdout, NULL);
    setbuf(stdin, NULL);

    while (-1 != (opt = getopt(argc, argv, "dirs:h"))) {
        switch (opt) {
        case 'd':
            debug_enabled = 1;
            break;
        case 'i':
            log = 1;
            break;
        case 'r':
            norealm = 1;
            break;
        case 's':
            service_principal = xstrdup(optarg);
            break;
        case 'h':
            fprintf(stderr, "Usage: \n");
            fprintf(stderr, "squid_kerb_auth [-d] [-i] [-s SPN] [-h]\n");
            fprintf(stderr, "-d full debug\n");
            fprintf(stderr, "-i informational messages\n");
            fprintf(stderr, "-r remove realm from username\n");
            fprintf(stderr, "-s service principal name\n");
            fprintf(stderr, "-h help\n");
            fprintf(stderr,
                    "The SPN can be set to GSS_C_NO_NAME to allow any entry from keytab\n");
            fprintf(stderr, "default SPN is HTTP/fqdn@DEFAULT_REALM\n");
            exit(0);
        default:
            fprintf(stderr, "%s| %s: WARNING: unknown option: -%c.\n", LogTime(),
                    PROGRAM, opt);
        }
    }

    debug((char *) "%s| %s: INFO: Starting version %s\n", LogTime(), PROGRAM, SQUID_KERB_AUTH_VERSION);
    if (service_principal && strcasecmp(service_principal, "GSS_C_NO_NAME")) {
        service.value = service_principal;
        service.length = strlen((char *) service.value);
    } else {
        host_name = gethost_name();
        if (!host_name) {
            fprintf(stderr,
                    "%s| %s: FATAL: Local hostname could not be determined. Please specify the service principal\n",
                    LogTime(), PROGRAM);
            fprintf(stdout, "BH hostname error\n");
            exit(-1);
        }
        service.value = xmalloc(strlen(service_name) + strlen(host_name) + 2);
        snprintf((char *) service.value, strlen(service_name) + strlen(host_name) + 2,
                 "%s@%s", service_name, host_name);
        service.length = strlen((char *) service.value);
    }

    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            if (ferror(stdin)) {
                debug((char *) "%s| %s: FATAL: fgets() failed! dying..... errno=%d (%s)\n",
                      LogTime(), PROGRAM, ferror(stdin),
                      strerror(ferror(stdin)));

                fprintf(stdout, "BH input error\n");
                exit(1);	/* BIIG buffer */
            }
            fprintf(stdout, "BH input error\n");
            exit(0);
        }
        c = (char *) memchr(buf, '\n', sizeof(buf) - 1);
        if (c) {
            *c = '\0';
            length = c - buf;
        } else {
            err = 1;
        }
        if (err) {
            debug((char *) "%s| %s: ERROR: Oversized message\n", LogTime(), PROGRAM);
            fprintf(stdout, "BH Oversized message\n");
            err = 0;
            continue;
        }
        debug((char *) "%s| %s: DEBUG: Got '%s' from squid (length: %d).\n", LogTime(), PROGRAM, buf, length);

        if (buf[0] == '\0') {
            debug((char *) "%s| %s: ERROR: Invalid request\n", LogTime(), PROGRAM);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (strlen(buf) < 2) {
            debug((char *) "%s| %s: ERROR: Invalid request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (!strncmp(buf, "QQ", 2)) {
            gss_release_buffer(&minor_status, &input_token);
            gss_release_buffer(&minor_status, &output_token);
            gss_release_buffer(&minor_status, &service);
            gss_release_cred(&minor_status, &server_creds);
            if (server_name)
                gss_release_name(&minor_status, &server_name);
            if (client_name)
                gss_release_name(&minor_status, &client_name);
            if (gss_context != GSS_C_NO_CONTEXT)
                gss_delete_sec_context(&minor_status, &gss_context, NULL);
            if (kerberosToken) {
                /* Allocated by parseNegTokenInit, but no matching free function exists.. */
                if (!spnego_flag)
                    xfree((char *) kerberosToken);
                kerberosToken = NULL;
            }
            if (spnego_flag) {
                /* Allocated by makeNegTokenTarg, but no matching free function exists.. */
                if (spnegoToken)
                    xfree((char *) spnegoToken);
                spnegoToken = NULL;
            }
            if (token) {
                xfree(token);
                token = NULL;
            }
            if (host_name) {
                xfree(host_name);
                host_name = NULL;
            }
            fprintf(stdout, "BH quit command\n");
            exit(0);
        }
        if (strncmp(buf, "YR", 2) && strncmp(buf, "KK", 2)) {
            debug((char *) "%s| %s: ERROR: Invalid request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid request\n");
            continue;
        }
        if (!strncmp(buf, "YR", 2)) {
            if (gss_context != GSS_C_NO_CONTEXT)
                gss_delete_sec_context(&minor_status, &gss_context, NULL);
            gss_context = GSS_C_NO_CONTEXT;
        }
        if (strlen(buf) <= 3) {
            debug((char *) "%s| %s: ERROR: Invalid negotiate request [%s]\n", LogTime(), PROGRAM, buf);
            fprintf(stdout, "BH Invalid negotiate request\n");
            continue;
        }
        input_token.length = base64_decode_len(buf+3);
        debug((char *) "%s| %s: DEBUG: Decode '%s' (decoded length: %d).\n",
              LogTime(), PROGRAM, buf + 3, (int) input_token.length);
        input_token.value = xmalloc(input_token.length);

        input_token.length = base64_decode((char *) input_token.value, input_token.length, buf+3);

        if ((input_token.length >= sizeof ntlmProtocol + 1) &&
                (!memcmp(input_token.value, ntlmProtocol, sizeof ntlmProtocol))) {
            debug((char *) "%s| %s: WARNING: received type %d NTLM token\n",
                  LogTime(), PROGRAM,
                  (int) *((unsigned char *) input_token.value +
                          sizeof ntlmProtocol));
            fprintf(stdout, "BH received type %d NTLM token\n",
                    (int) *((unsigned char *) input_token.value +
                            sizeof ntlmProtocol));
            goto cleanup;
        }
        if (service_principal) {
            if (strcasecmp(service_principal, "GSS_C_NO_NAME")) {
                major_status = gss_import_name(&minor_status, &service,
                                               (gss_OID) GSS_C_NULL_OID, &server_name);

            } else {
                server_name = GSS_C_NO_NAME;
                major_status = GSS_S_COMPLETE;
            }
        } else {
            major_status = gss_import_name(&minor_status, &service,
                                           gss_nt_service_name, &server_name);
        }

        if (check_gss_err(major_status, minor_status, "gss_import_name()", log))
            goto cleanup;

        major_status =
            gss_acquire_cred(&minor_status, server_name, GSS_C_INDEFINITE,
                             GSS_C_NO_OID_SET, GSS_C_ACCEPT, &server_creds, NULL, NULL);
        if (check_gss_err(major_status, minor_status, "gss_acquire_cred()", log))
            goto cleanup;

        major_status = gss_accept_sec_context(&minor_status,
                                              &gss_context,
                                              server_creds,
                                              &input_token,
                                              GSS_C_NO_CHANNEL_BINDINGS,
                                              &client_name, NULL, &output_token, &ret_flags, NULL, NULL);

        if (output_token.length) {
            spnegoToken = (const unsigned char *) output_token.value;
            spnegoTokenLength = output_token.length;
            token = (char *) xmalloc(base64_encode_len(spnegoTokenLength));
            if (token == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            base64_encode_str(token, base64_encode_len(spnegoTokenLength),
                              (const char *) spnegoToken, spnegoTokenLength);

            if (check_gss_err(major_status, minor_status, "gss_accept_sec_context()", log))
                goto cleanup;
            if (major_status & GSS_S_CONTINUE_NEEDED) {
                debug((char *) "%s| %s: INFO: continuation needed\n", LogTime(), PROGRAM);
                fprintf(stdout, "TT %s\n", token);
                goto cleanup;
            }
            gss_release_buffer(&minor_status, &output_token);
            major_status =
                gss_display_name(&minor_status, client_name, &output_token,
                                 NULL);

            if (check_gss_err(major_status, minor_status, "gss_display_name()", log))
                goto cleanup;
            user = (char *) xmalloc(output_token.length + 1);
            if (user == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            memcpy(user, output_token.value, output_token.length);
            user[output_token.length] = '\0';
            if (norealm && (p = strchr(user, '@')) != NULL) {
                *p = '\0';
            }
            fprintf(stdout, "AF %s %s\n", token, user);
            debug((char *) "%s| %s: DEBUG: AF %s %s\n", LogTime(), PROGRAM, token, rfc1738_escape(user));
            if (log)
                fprintf(stderr, "%s| %s: INFO: User %s authenticated\n", LogTime(),
                        PROGRAM, rfc1738_escape(user));
            goto cleanup;
        } else {
            if (check_gss_err(major_status, minor_status, "gss_accept_sec_context()", log))
                goto cleanup;
            if (major_status & GSS_S_CONTINUE_NEEDED) {
                debug((char *) "%s| %s: INFO: continuation needed\n", LogTime(), PROGRAM);
                fprintf(stdout, "NA %s\n", token);
                goto cleanup;
            }
            gss_release_buffer(&minor_status, &output_token);
            major_status =
                gss_display_name(&minor_status, client_name, &output_token,
                                 NULL);

            if (check_gss_err(major_status, minor_status, "gss_display_name()", log))
                goto cleanup;
            /*
             *  Return dummy token AA. May need an extra return tag then AF
             */
            user = (char *) xmalloc(output_token.length + 1);
            if (user == NULL) {
                debug((char *) "%s| %s: ERROR: Not enough memory\n", LogTime(), PROGRAM);
                fprintf(stdout, "BH Not enough memory\n");
                goto cleanup;
            }
            memcpy(user, output_token.value, output_token.length);
            user[output_token.length] = '\0';
            if (norealm && (p = strchr(user, '@')) != NULL) {
                *p = '\0';
            }
            fprintf(stdout, "AF %s %s\n", "AA==", user);
            debug((char *) "%s| %s: DEBUG: AF %s %s\n", LogTime(), PROGRAM, "AA==", rfc1738_escape(user));
            if (log)
                fprintf(stderr, "%s| %s: INFO: User %s authenticated\n", LogTime(),
                        PROGRAM, rfc1738_escape(user));

        }
cleanup:
        gss_release_buffer(&minor_status, &input_token);
        gss_release_buffer(&minor_status, &output_token);
        gss_release_cred(&minor_status, &server_creds);
        if (server_name)
            gss_release_name(&minor_status, &server_name);
        if (client_name)
            gss_release_name(&minor_status, &client_name);
        if (kerberosToken) {
            /* Allocated by parseNegTokenInit, but no matching free function exists.. */
            if (!spnego_flag)
                xfree((char *) kerberosToken);
            kerberosToken = NULL;
        }
        if (spnego_flag) {
            /* Allocated by makeNegTokenTarg, but no matching free function exists.. */
            if (spnegoToken)
                xfree((char *) spnegoToken);
            spnegoToken = NULL;
        }
        if (token) {
            xfree(token);
            token = NULL;
        }
        if (user) {
            xfree(user);
            user = NULL;
        }
        continue;
    }
}
#else
#include <stdio.h>
#include <stdlib.h>
#ifndef MAX_AUTHTOKEN_LEN
#define MAX_AUTHTOKEN_LEN   65535
#endif
int
main(int argc, char *const argv[])
{
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    char buf[MAX_AUTHTOKEN_LEN];
    while (1) {
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            fprintf(stdout, "BH input error\n");
            exit(0);
        }
        fprintf(stdout, "BH Kerberos authentication not supported\n");
    }
}
#endif /* HAVE_GSSAPI */
