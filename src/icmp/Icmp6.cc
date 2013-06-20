/*
 * DEBUG: section 42    ICMP Pinger program
 * AUTHOR: Duane Wessels, Amos Jeffries
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
//#define SQUID_HELPER 1

#include "squid.h"

#if USE_ICMP

#include "leakcheck.h"
#include "SquidTime.h"
#include "Debug.h"
#include "Icmp6.h"
#include "IcmpPinger.h"

// Some system headers are only neeed internally here.
// They should not be included via the header.

#if HAVE_NETINET_IP6_H
#include <netinet/ip6.h>
#endif

// Icmp6 OP-Codes
// see http://www.iana.org/assignments/icmpv6-parameters
// NP: LowPktStr is for codes 0-127
static const char *icmp6LowPktStr[] = {
    "ICMP 0",			// 0
    "Destination Unreachable",	// 1 - RFC2463
    "Packet Too Big", 		// 2 - RFC2463
    "Time Exceeded",		// 3 - RFC2463
    "Parameter Problem",		// 4 - RFC2463
    "ICMP 5",			// 5
    "ICMP 6",			// 6
    "ICMP 7",			// 7
    "ICMP 8",			// 8
    "ICMP 9",			// 9
    "ICMP 10"			// 10
};

// NP: HighPktStr is for codes 128-255
static const char *icmp6HighPktStr[] = {
    "Echo Request",					// 128 - RFC2463
    "Echo Reply",					// 129 - RFC2463
    "Multicast Listener Query",			// 130 - RFC2710
    "Multicast Listener Report",			// 131 - RFC2710
    "Multicast Listener Done",			// 132 - RFC2710
    "Router Solicitation",				// 133 - RFC4861
    "Router Advertisement",				// 134 - RFC4861
    "Neighbor Solicitation",			// 135 - RFC4861
    "Neighbor Advertisement",			// 136 - RFC4861
    "Redirect Message",				// 137 - RFC4861
    "Router Renumbering",				// 138 - Crawford
    "ICMP Node Information Query",			// 139 - RFC4620
    "ICMP Node Information Response",		// 140 - RFC4620
    "Inverse Neighbor Discovery Solicitation",	// 141 - RFC3122
    "Inverse Neighbor Discovery Advertisement",	// 142 - RFC3122
    "Version 2 Multicast Listener Report",		// 143 - RFC3810
    "Home Agent Address Discovery Request",		// 144 - RFC3775
    "Home Agent Address Discovery Reply",		// 145 - RFC3775
    "Mobile Prefix Solicitation",			// 146 - RFC3775
    "Mobile Prefix Advertisement",			// 147 - RFC3775
    "Certification Path Solicitation",		// 148 - RFC3971
    "Certification Path Advertisement",		// 149 - RFC3971
    "ICMP Experimental (150)",			// 150 - RFC4065
    "Multicast Router Advertisement",		// 151 - RFC4286
    "Multicast Router Solicitation",		// 152 - RFC4286
    "Multicast Router Termination",			// 153 - [RFC4286]
    "ICMP 154",
    "ICMP 155",
    "ICMP 156",
    "ICMP 157",
    "ICMP 158",
    "ICMP 159",
    "ICMP 160"
};

Icmp6::Icmp6() : Icmp()
{
    ; // nothing new.
}

Icmp6::~Icmp6()
{
    Close();
}

int
Icmp6::Open(void)
{
    icmp_sock = socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    if (icmp_sock < 0) {
        debugs(50, DBG_CRITICAL, HERE << " icmp_sock: " << xstrerror());
        return -1;
    }

    icmp_ident = getpid() & 0xffff;
    debugs(42, DBG_IMPORTANT, "pinger: ICMPv6 socket opened");

    return icmp_sock;
}

/**
 * Generates an RFC 4443 Icmp6 ECHO Packet and sends into the network.
 */
void
Icmp6::SendEcho(Ip::Address &to, int opcode, const char *payload, int len)
{
    int x;
    LOCAL_ARRAY(char, pkt, MAX_PKT6_SZ);
    struct icmp6_hdr *icmp = NULL;
    icmpEchoData *echo = NULL;
    struct addrinfo *S = NULL;
    size_t icmp6_pktsize = 0;

    memset(pkt, '\0', MAX_PKT6_SZ);
    icmp = (struct icmp6_hdr *)pkt;

    /*
     * cevans - beware signed/unsigned issues in untrusted data from
     * the network!!
     */
    if (len < 0) {
        len = 0;
    }

    // Construct Icmp6 ECHO header
    icmp->icmp6_type = ICMP6_ECHO_REQUEST;
    icmp->icmp6_code = 0;
    icmp->icmp6_cksum = 0;
    icmp->icmp6_id = icmp_ident;
    icmp->icmp6_seq = (unsigned short) icmp_pkts_sent;
    ++icmp_pkts_sent;

    icmp6_pktsize = sizeof(struct icmp6_hdr);

    // Fill Icmp6 ECHO data content
    echo = (icmpEchoData *) (pkt + sizeof(icmp6_hdr));
    echo->opcode = (unsigned char) opcode;
    memcpy(&echo->tv, &current_time, sizeof(struct timeval));

    icmp6_pktsize += sizeof(struct timeval) + sizeof(char);

    if (payload) {
        if (len > MAX_PAYLOAD)
            len = MAX_PAYLOAD;

        memcpy(echo->payload, payload, len);

        icmp6_pktsize += len;
    }

    icmp->icmp6_cksum = CheckSum((unsigned short *) icmp, icmp6_pktsize);

    to.GetAddrInfo(S);
    ((sockaddr_in6*)S->ai_addr)->sin6_port = 0;

    assert(icmp6_pktsize <= MAX_PKT6_SZ);

    debugs(42, 5, HERE << "Send Icmp6 packet to " << to << ".");

    x = sendto(icmp_sock,
               (const void *) pkt,
               icmp6_pktsize,
               0,
               S->ai_addr,
               S->ai_addrlen);

    if (x < 0) {
        debugs(42, DBG_IMPORTANT, HERE << "Error sending to ICMPv6 packet to " << to << ". ERR: " << xstrerror());
    }
    debugs(42,9, HERE << "x=" << x);

    Log(to, 0, NULL, 0, 0);
    to.FreeAddrInfo(S);
}

/**
 * Reads an RFC 4443 Icmp6 ECHO-REPLY Packet from the network.
 */
void
Icmp6::Recv(void)
{
    int n;
    struct addrinfo *from = NULL;
//    struct ip6_hdr *ip = NULL;
    static char *pkt = NULL;
    struct icmp6_hdr *icmp6header = NULL;
    icmpEchoData *echo = NULL;
    struct timeval now;
    static pingerReplyData preply;

    if (icmp_sock < 0) {
        debugs(42, DBG_CRITICAL, HERE << "dropping ICMPv6 read. No socket!?");
        return;
    }

    if (pkt == NULL) {
        pkt = (char *)xmalloc(MAX_PKT6_SZ);
    }

    preply.from.InitAddrInfo(from);

    n = recvfrom(icmp_sock,
                 (void *)pkt,
                 MAX_PKT6_SZ,
                 0,
                 from->ai_addr,
                 &from->ai_addrlen);

    preply.from = *from;

#if GETTIMEOFDAY_NO_TZP

    gettimeofday(&now);

#else

    gettimeofday(&now, NULL);

#endif

    debugs(42, 8, HERE << n << " bytes from " << preply.from);

// FIXME INET6 : The IPv6 Header (ip6_hdr) is not availble directly >:-(
//
// TTL still has to come from the IP header somewhere.
//	still need to strip and process it properly.
//	probably have to rely on RTT as given by timestamp in data sent and current.
    /* IPv6 Header Structures (linux)
    struct ip6_hdr

    // fields (via simple define)
    #define ip6_vfc		// N.A
    #define ip6_flow	// N/A
    #define ip6_plen	// payload length.
    #define ip6_nxt		// expect to be type 0x3a - ICMPv6
    #define ip6_hlim	// MAX hops  (always 64, but no guarantee)
    #define ip6_hops	// HOPS!!!  (can it be true??)

        ip = (struct ip6_hdr *) pkt;
        pkt += sizeof(ip6_hdr);

    debugs(42, DBG_CRITICAL, HERE << "ip6_nxt=" << ip->ip6_nxt <<
    		", ip6_plen=" << ip->ip6_plen <<
    		", ip6_hlim=" << ip->ip6_hlim <<
    		", ip6_hops=" << ip->ip6_hops	<<
    		" ::: 40 == sizef(ip6_hdr) == " << sizeof(ip6_hdr)
    );
    */

    icmp6header = (struct icmp6_hdr *) pkt;
    pkt += sizeof(icmp6_hdr);

    if (icmp6header->icmp6_type != ICMP6_ECHO_REPLY) {

        switch (icmp6header->icmp6_type) {
        case 134:
        case 135:
        case 136:
            /* ignore Router/Neighbour Advertisements */
            break;

        default:
            debugs(42, 8, HERE << preply.from << " said: " << icmp6header->icmp6_type << "/" << (int)icmp6header->icmp6_code << " " <<
                   ( icmp6header->icmp6_type&0x80 ? icmp6HighPktStr[(int)(icmp6header->icmp6_type&0x7f)] : icmp6LowPktStr[(int)(icmp6header->icmp6_type&0x7f)] )
                  );
        }
        preply.from.FreeAddrInfo(from);
        return;
    }

    if (icmp6header->icmp6_id != icmp_ident) {
        debugs(42, 8, HERE << "dropping Icmp6 read. IDENT check failed. ident=='" << icmp_ident << "'=='" << icmp6header->icmp6_id << "'");
        preply.from.FreeAddrInfo(from);
        return;
    }

    echo = (icmpEchoData *) pkt;

    preply.opcode = echo->opcode;

    struct timeval tv;
    memcpy(&tv, &echo->tv, sizeof(struct timeval));
    preply.rtt = tvSubMsec(tv, now);

    /*
     * FIXME INET6: Without access to the IPv6-Hops header we must rely on the total RTT
     *      and could caculate the hops from that, but it produces some weird value mappings using ipHops
     *	for now everything is 1 v6 hop away with variant RTT
     * WANT:    preply.hops = ip->ip6_hops; // ipHops(ip->ip_hops);
     */
    preply.hops = 1;

    preply.psize = n - /* sizeof(ip6_hdr) - */ sizeof(icmp6_hdr) - (sizeof(icmpEchoData) - MAX_PKT6_SZ);

    /* Ensure the response packet has safe payload size */
    if ( preply.psize > (unsigned short) MAX_PKT6_SZ) {
        preply.psize = MAX_PKT6_SZ;
    } else if ( preply.psize < (unsigned short)0) {
        preply.psize = 0;
    }

    Log(preply.from,
        icmp6header->icmp6_type,
        ( icmp6header->icmp6_type&0x80 ? icmp6HighPktStr[(int)(icmp6header->icmp6_type&0x7f)] : icmp6LowPktStr[(int)(icmp6header->icmp6_type&0x7f)] ),
        preply.rtt,
        preply.hops);

    /* send results of the lookup back to squid.*/
    control.SendResult(preply, (sizeof(pingerReplyData) - PINGER_PAYLOAD_SZ + preply.psize) );
    preply.from.FreeAddrInfo(from);
}

#endif /* USE_ICMP */
