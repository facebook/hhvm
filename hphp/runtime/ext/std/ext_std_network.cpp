/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/runtime/ext/std/ext_std_network.h"

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>

#include <folly/IPAddress.h>
#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/sockets/ext_sockets.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/lock.h"
#include "hphp/util/network.h"

#if defined(__APPLE__)
# include <arpa/nameser_compat.h>
#include <vector>
#endif

// HOST_NAME_MAX is recommended by POSIX, but not required.
// FreeBSD and OSX (as of 10.9) are known to not define it.
// 255 is generally the safe value to assume and upstream
// PHP does this as well.
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

#define MAXPACKET  8192 /* max packet size used internally by BIND */
#define DNS_T_A 1
#define DNS_T_NS 2
#define DNS_T_CNAME 5
#define DNS_T_SOA 6
#define DNS_T_PTR 12
#define DNS_T_HINFO 13
#define DNS_T_MINFO 14
#define DNS_T_MX 15
#define DNS_T_TXT 16
#define DNS_T_AAAA 28
#define DNS_T_SRV 33
#define DNS_T_NAPTR 35
#define DNS_T_A6 38
#define DNS_T_ANY 255

#define PHP_DNS_NUM_TYPES   12  // Number of DNS Types Supported by PHP
#define PHP_DNS_A      0x00000001
#define PHP_DNS_NS     0x00000002
#define PHP_DNS_CNAME  0x00000010
#define PHP_DNS_SOA    0x00000020
#define PHP_DNS_PTR    0x00000800
#define PHP_DNS_HINFO  0x00001000
#define PHP_DNS_MX     0x00004000
#define PHP_DNS_TXT    0x00008000
#define PHP_DNS_A6     0x01000000
#define PHP_DNS_SRV    0x02000000
#define PHP_DNS_NAPTR  0x04000000
#define PHP_DNS_AAAA   0x08000000
#define PHP_DNS_ANY    0x10000000
#define PHP_DNS_ALL    (PHP_DNS_A|PHP_DNS_NS|PHP_DNS_CNAME|PHP_DNS_SOA| \
                        PHP_DNS_PTR|PHP_DNS_HINFO|PHP_DNS_MX|PHP_DNS_TXT| \
                        PHP_DNS_A6|PHP_DNS_SRV|PHP_DNS_NAPTR|PHP_DNS_AAAA)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

static Mutex NetworkMutex;

class ResolverInit {
public:
  ResolverInit() : m_res(nullptr) {
    m_res = (struct __res_state *)calloc(1, sizeof(*m_res));
    initRes();
  }
  ~ResolverInit() {
    if (m_res)
      free(m_res);
    m_res = nullptr;
  }

  struct __res_state *getResolver(void) {
    initRes();
    return m_res;
  }

  static DECLARE_THREAD_LOCAL(ResolverInit, s_res);
private:
  void initRes(void) {
    if (m_res) {
      memset(m_res, 0, sizeof(*m_res));
      if (res_ninit(m_res)) {
        free(m_res);
        m_res = nullptr;
      }
    }
  }

  struct __res_state *m_res;
};
IMPLEMENT_THREAD_LOCAL(ResolverInit, ResolverInit::s_res);

Variant HHVM_FUNCTION(gethostname) {
  char h_name[HOST_NAME_MAX];

  if (gethostname(h_name, sizeof(h_name)) != 0) {
    raise_warning(
        "gethostname() failed with errorno=%d: %s", errno, strerror(errno));
    return false;
  }
  // gethostname may not null-terminate
  h_name[sizeof(h_name) - 1] = '\0';

  return String(h_name, CopyString);
}

Variant HHVM_FUNCTION(gethostbyaddr, const String& ip_address) {
  IOStatusHelper io("gethostbyaddr", ip_address.data());
  struct addrinfo hints, *res, *res0;
  char h_name[NI_MAXHOST];
  int error;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  error = getaddrinfo(ip_address.data(), NULL, &hints, &res0);
  if (error)  {
    return false;
  }

  for (res = res0; res; res = res->ai_next) {
    if (getnameinfo(res->ai_addr, res->ai_addrlen, h_name, NI_MAXHOST,
          NULL, 0, 0) < 0) {
      continue;
    }
    freeaddrinfo(res0);
    return String(h_name, CopyString);
  }
  freeaddrinfo(res0);
  return ip_address;
}

const StaticString s_empty("");

String HHVM_FUNCTION(gethostbyname, const String& hostname) {
  IOStatusHelper io("gethostbyname", hostname.data());

  HostEnt result;
  if (!safe_gethostbyname(hostname.data(), result)) {
    return hostname;
  }

  struct in_addr in;
  memcpy(&in.s_addr, *(result.hostbuf.h_addr_list), sizeof(in.s_addr));
  try {
    return String(folly::IPAddressV4(in).str());
  } catch (folly::IPAddressFormatException &e) {
    return hostname;
  }
}

Variant HHVM_FUNCTION(gethostbynamel, const String& hostname) {
  IOStatusHelper io("gethostbynamel", hostname.data());
  HostEnt result;
  if (!safe_gethostbyname(hostname.data(), result)) {
    return false;
  }

  Array ret;
  for (int i = 0 ; result.hostbuf.h_addr_list[i] != 0 ; i++) {
    struct in_addr in = *(struct in_addr *)result.hostbuf.h_addr_list[i];
    try {
      ret.append(String(folly::IPAddressV4(in).str()));
    } catch (folly::IPAddressFormatException &e) {
        // ok to skip
    }
  }
  return ret;
}

Variant HHVM_FUNCTION(getprotobyname, const String& name) {
  Lock lock(NetworkMutex);

  struct protoent *ent = getprotobyname(name.data());
  if (ent == NULL) {
    return false;
  }
  return ent->p_proto;
}

Variant HHVM_FUNCTION(getprotobynumber, int64_t number) {
  Lock lock(NetworkMutex);

  struct protoent *ent = getprotobynumber(number);
  if (ent == NULL) {
    return false;
  }
  return String(ent->p_name, CopyString);
}

Variant HHVM_FUNCTION(getservbyname, const String& service,
                                     const String& protocol) {
  Lock lock(NetworkMutex);

  struct servent *serv = getservbyname(service.data(), protocol.data());
  if (serv == NULL) {
    return false;
  }
  return ntohs(serv->s_port);
}

Variant HHVM_FUNCTION(getservbyport, int64_t port, const String& protocol) {
  Lock lock(NetworkMutex);

  struct servent *serv = getservbyport(htons(port), protocol.data());
  if (serv == NULL) {
    return false;
  }
  return String(serv->s_name, CopyString);
}

Variant HHVM_FUNCTION(inet_ntop, const String& in_addr) {
  int af = AF_INET;
  if (in_addr.size() == 16) {
    af = AF_INET6;
  } else if (in_addr.size() != 4) {
    raise_warning("Invalid in_addr value");
    return false;
  }

  char buffer[40];
  if (!inet_ntop(af, in_addr.data(), buffer, sizeof(buffer))) {
    raise_warning("An unknown error occurred");
    return false;
  }
  return String(buffer, CopyString);
}

Variant HHVM_FUNCTION(inet_pton, const String& address) {
  int af = AF_INET;
  const char *saddress = address.data();
  if (strchr(saddress, ':')) {
    af = AF_INET6;
  } else if (!strchr(saddress, '.')) {
    raise_warning("Unrecognized address %s", saddress);
    return false;
  }

  char buffer[17];
  memset(buffer, 0, sizeof(buffer));
  int ret = inet_pton(af, saddress, buffer);
  if (ret <= 0) {
    raise_warning("Unrecognized address %s", saddress);
    return false;
  }

  return String(buffer, af == AF_INET ? 4 : 16, CopyString);
}

Variant HHVM_FUNCTION(ip2long, const String& ip_address) {
  struct in_addr ip;
  if (ip_address.empty() ||
      inet_pton(AF_INET, ip_address.data(), &ip) != 1) {
    return false;
  }

  return (int64_t)ntohl(ip.s_addr);
}

String HHVM_FUNCTION(long2ip, const String& proper_address) {
  unsigned long ul = strtoul(proper_address.c_str(), nullptr, 0);
  try {
    return folly::IPAddress::fromLongHBO(ul).str();
  } catch (folly::IPAddressFormatException &e) {
    return s_empty;
  }
}

/* just a hack to free resources allocated by glibc in __res_nsend()
 * See also:
 *   res_thread_freeres() in glibc/resolv/res_init.c
 *   __libc_res_nsend()   in resolv/res_send.c
 * */

static void php_dns_free_res(struct __res_state *res) {
#if defined(__GLIBC__)
  int ns;
  for (ns = 0; ns < MAXNS; ns++) {
    if (res->_u._ext.nsaddrs[ns] != NULL) {
      free(res->_u._ext.nsaddrs[ns]);
      res->_u._ext.nsaddrs[ns] = NULL;
    }
  }
#endif
}

bool HHVM_FUNCTION(checkdnsrr, const String& host,
                               const String& type /* = null_string */) {
  IOStatusHelper io("dns_check_record", host.data());
  const char *stype;
  if (type.empty()) {
    stype = "MX";
  } else {
    stype = type.data();
  }
  if (host.empty()) {
    throw_invalid_argument("host: [empty]");
  }

  int ntype;
  if (!strcasecmp("A", stype)) ntype = DNS_T_A;
  else if (!strcasecmp("NS",    stype)) ntype = DNS_T_NS;
  else if (!strcasecmp("MX",    stype)) ntype = DNS_T_MX;
  else if (!strcasecmp("PTR",   stype)) ntype = DNS_T_PTR;
  else if (!strcasecmp("ANY",   stype)) ntype = DNS_T_ANY;
  else if (!strcasecmp("SOA",   stype)) ntype = DNS_T_SOA;
  else if (!strcasecmp("TXT",   stype)) ntype = DNS_T_TXT;
  else if (!strcasecmp("CNAME", stype)) ntype = DNS_T_CNAME;
  else if (!strcasecmp("AAAA",  stype)) ntype = DNS_T_AAAA;
  else if (!strcasecmp("SRV",   stype)) ntype = DNS_T_SRV;
  else if (!strcasecmp("NAPTR", stype)) ntype = DNS_T_NAPTR;
  else if (!strcasecmp("A6",    stype)) ntype = DNS_T_A6;
  else {
    throw_invalid_argument("type: %s", stype);
    return false;
  }

  unsigned char ans[MAXPACKET];
  struct __res_state *res;
  res = ResolverInit::s_res.get()->getResolver();
  if (res == NULL) {
    return false;
  }
  int i = res_nsearch(res, host.data(), C_IN, ntype, ans, sizeof(ans));
  res_nclose(res);
  php_dns_free_res(res);
  return (i >= 0);
}

typedef union {
  HEADER qb1;
  u_char qb2[65536];
} querybuf;

const StaticString
  s_host("host"),
  s_type("type"),
  s_ip("ip"),
  s_pri("pri"),
  s_weight("weight"),
  s_port("port"),
  s_order("order"),
  s_pref("pref"),
  s_target("target"),
  s_cpu("cpu"),
  s_os("os"),
  s_txt("txt"),
  s_mname("mname"),
  s_rname("rname"),
  s_serial("serial"),
  s_refresh("refresh"),
  s_retry("retry"),
  s_expire("expire"),
  s_minimum_ttl("minimum-ttl"),
  s_ipv6("ipv6"),
  s_masklen("masklen"),
  s_chain("chain"),
  s_flags("flags"),
  s_services("services"),
  s_regex("regex"),
  s_replacement("replacement"),
  s_class("class"),
  s_ttl("ttl"),
  s_A("A"),
  s_MX("MX"),
  s_CNAME("CNAME"),
  s_NS("NS"),
  s_PTR("PTR"),
  s_HINFO("HINFO"),
  s_TXT("TXT"),
  s_SOA("SOA"),
  s_AAAA("AAAA"),
  s_A6("A6"),
  s_SRV("SRV"),
  s_NAPTR("NAPTR"),
  s_IN("IN");

#define CHECKCP(n) do { \
  if ((cp + (n)) > end) { \
    return nullptr; \
  } \
} while (0)

static unsigned char *php_parserr(unsigned char *cp, unsigned char* end,
                                  querybuf *answer,
                                  int type_to_fetch, bool store,
                                  Array &subarray) {
  unsigned short type, cls ATTRIBUTE_UNUSED, dlen;
  unsigned long ttl;
  int64_t n, i;
  unsigned short s;
  unsigned char *tp, *p;
  char name[255 + 2];  // IETF STD 13 section 3.1; 255 bytes
  int have_v6_break = 0, in_v6_break = 0;

  n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, sizeof(name) - 2);
  if (n < 0) {
    return NULL;
  }
  cp += n;

  CHECKCP(10);
  GETSHORT(type, cp);
  GETSHORT(cls, cp);
  GETLONG(ttl, cp);
  GETSHORT(dlen, cp);
  CHECKCP(dlen);
  if (type_to_fetch != T_ANY && type != type_to_fetch) {
    cp += dlen;
    return cp;
  }

  if (!store) {
    cp += dlen;
    return cp;
  }

  subarray.set(s_host, String(name, CopyString));
  switch (type) {
  case DNS_T_A:
    CHECKCP(4);
    subarray.set(s_type, s_A);
    snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
    subarray.set(s_ip, String(name, CopyString));
    cp += dlen;
    break;
  case DNS_T_MX:
    CHECKCP(2);
    subarray.set(s_type, s_MX);
    GETSHORT(n, cp);
    subarray.set(s_pri, n);
    /* no break; */
  case DNS_T_CNAME:
    if (type == DNS_T_CNAME) {
      subarray.set(s_type, s_CNAME);
    }
    /* no break; */
  case DNS_T_NS:
    if (type == DNS_T_NS) {
      subarray.set(s_type, s_NS);
    }
    /* no break; */
  case DNS_T_PTR:
    if (type == DNS_T_PTR) {
      subarray.set(s_type, s_PTR);
    }
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set(s_target, String(name, CopyString));
    break;
  case DNS_T_HINFO:
    /* See RFC 1010 for values */
    subarray.set(s_type, s_HINFO);
    CHECKCP(1);
    n = *cp & 0xFF;
    cp++;
    CHECKCP(n);
    subarray.set(s_cpu, String((const char *)cp, n, CopyString));
    cp += n;
    CHECKCP(1);
    n = *cp & 0xFF;
    cp++;
    CHECKCP(n);
    subarray.set(s_os, String((const char *)cp, n, CopyString));
    cp += n;
    break;
  case DNS_T_TXT: {
    int l1 = 0, l2 = 0;

    String s = String(dlen, ReserveString);
    tp = (unsigned char *)s.mutableData();

    while (l1 < dlen) {
      n = cp[l1];
      if ((n + l1) > dlen) {
        // bad record, don't set anything
        break;
      }
      memcpy(tp + l1 , cp + l1 + 1, n);
      l1 = l1 + n + 1;
      l2 = l2 + n;
    }
    s.setSize(l2);
    cp += dlen;

    subarray.set(s_type, s_TXT);
    subarray.set(s_txt, s);
    break;
  }
  case DNS_T_SOA:
    subarray.set(s_type, s_SOA);
    n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set(s_mname, String(name, CopyString));
    n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set(s_rname, String(name, CopyString));
    CHECKCP(5*4);
    GETLONG(n, cp);
    subarray.set(s_serial, n);
    GETLONG(n, cp);
    subarray.set(s_refresh, n);
    GETLONG(n, cp);
    subarray.set(s_retry, n);
    GETLONG(n, cp);
    subarray.set(s_expire, n);
    GETLONG(n, cp);
    subarray.set(s_minimum_ttl, n);
    break;
  case DNS_T_AAAA:
    tp = (unsigned char *)name;
    CHECKCP(8*2);
    for (i = 0; i < 8; i++) {
      GETSHORT(s, cp);
      if (s != 0) {
        if (tp > (u_char *)name) {
          in_v6_break = 0;
          tp[0] = ':';
          tp++;
        }
        tp += sprintf((char *)tp, "%x", s);
      } else {
        if (!have_v6_break) {
          have_v6_break = 1;
          in_v6_break = 1;
          tp[0] = ':';
          tp++;
        } else if (!in_v6_break) {
          tp[0] = ':';
          tp++;
          tp[0] = '0';
          tp++;
        }
      }
    }
    if (have_v6_break && in_v6_break) {
      tp[0] = ':';
      tp++;
    }
    tp[0] = '\0';
    subarray.set(s_type, s_AAAA);
    subarray.set(s_ipv6, String(name, CopyString));
    break;
  case DNS_T_A6:
    p = cp;
    subarray.set(s_type, s_A6);
    CHECKCP(1);
    n = ((int)cp[0]) & 0xFF;
    cp++;
    subarray.set(s_masklen, n);
    tp = (unsigned char *)name;
    if (n > 15) {
      have_v6_break = 1;
      in_v6_break = 1;
      tp[0] = ':';
      tp++;
    }
    if (n % 16 > 8) {
      /* Partial short */
      if (cp[0] != 0) {
        if (tp > (u_char *)name) {
          in_v6_break = 0;
          tp[0] = ':';
          tp++;
        }
        sprintf((char *)tp, "%x", cp[0] & 0xFF);
      } else {
        if (!have_v6_break) {
          have_v6_break = 1;
          in_v6_break = 1;
          tp[0] = ':';
          tp++;
        } else if (!in_v6_break) {
          tp[0] = ':';
          tp++;
          tp[0] = '0';
          tp++;
        }
      }
      cp++;
    }
    for (i = (n + 8)/16; i < 8; i++) {
      CHECKCP(2);
      GETSHORT(s, cp);
      if (s != 0) {
        if (tp > (u_char *)name) {
          in_v6_break = 0;
          tp[0] = ':';
          tp++;
        }
        tp += sprintf((char*)tp,"%x",s);
      } else {
        if (!have_v6_break) {
          have_v6_break = 1;
          in_v6_break = 1;
          tp[0] = ':';
          tp++;
        } else if (!in_v6_break) {
          tp[0] = ':';
          tp++;
          tp[0] = '0';
          tp++;
        }
      }
    }
    if (have_v6_break && in_v6_break) {
      tp[0] = ':';
      tp++;
    }
    tp[0] = '\0';
    subarray.set(s_ipv6, String(name, CopyString));
    if (cp < p + dlen) {
      n = dn_expand(answer->qb2, end, cp, name,
                    (sizeof name) - 2);
      if (n < 0) {
        return NULL;
      }
      cp += n;
      subarray.set(s_chain, String(name, CopyString));
    }
    break;
  case DNS_T_SRV:
    CHECKCP(3*2);
    subarray.set(s_type, s_SRV);
    GETSHORT(n, cp);
    subarray.set(s_pri, n);
    GETSHORT(n, cp);
    subarray.set(s_weight, n);
    GETSHORT(n, cp);
    subarray.set(s_port, n);
    n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set(s_target, String(name, CopyString));
    break;
  case DNS_T_NAPTR:
    CHECKCP(2*2);
    subarray.set(s_type, s_NAPTR);
    GETSHORT(n, cp);
    subarray.set(s_order, n);
    GETSHORT(n, cp);
    subarray.set(s_pref, n);

    CHECKCP(1);
    n = (cp[0] & 0xFF);
    ++cp;
    CHECKCP(n);
    subarray.set(s_flags, String((const char *)cp, n, CopyString));
    cp += n;

    CHECKCP(1);
    n = (cp[0] & 0xFF);
    ++cp;
    CHECKCP(n);
    subarray.set(s_services, String((const char *)cp, n, CopyString));
    cp += n;

    CHECKCP(1);
    n = (cp[0] & 0xFF);
    ++cp;
    CHECKCP(n);
    subarray.set(s_regex, String((const char *)cp, n, CopyString));
    cp += n;

    n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set(s_replacement, String(name, CopyString));
    break;
  default:
    cp += dlen;
  }

  subarray.set(s_class, s_IN);
  subarray.set(s_ttl, (int)ttl);
  return cp;
}

Variant HHVM_FUNCTION(dns_get_record, const String& hostname, int type /*= -1*/,
                      VRefParam authnsRef /* = null */,
                      VRefParam addtlRef /* = null */) {
  IOStatusHelper io("dns_get_record", hostname.data(), type);
  if (type < 0) type = PHP_DNS_ALL;
  if (type & ~PHP_DNS_ALL && type != PHP_DNS_ANY) {
    raise_warning("Type '%d' not supported", type);
    return false;
  }

  unsigned char *cp = NULL, *end = NULL;
  int qd, an, ns = 0, ar = 0;
  querybuf answer;

  /* - We emulate an or'ed type mask by querying type by type.
   *   (Steps 0 - NUMTYPES-1 )
   *   If additional info is wanted we check again with DNS_T_ANY
   *   (step NUMTYPES / NUMTYPES+1 )
   *   store_results is used to skip storing the results retrieved in step
   *   NUMTYPES+1 when results were already fetched.
   * - In case of PHP_DNS_ANY we use the directly fetch DNS_T_ANY.
   *   (step NUMTYPES+1 )
   */
  Array ret;
  bool first_query = true;
  bool store_results = true;
  for (int t = (type == PHP_DNS_ANY ? (PHP_DNS_NUM_TYPES + 1) : 0);
       t < PHP_DNS_NUM_TYPES + 2 || first_query; t++) {
    first_query = false;
    int type_to_fetch;
    switch (t) {
    case 0:  type_to_fetch = type & PHP_DNS_A     ? DNS_T_A     : 0; break;
    case 1:  type_to_fetch = type & PHP_DNS_NS    ? DNS_T_NS    : 0; break;
    case 2:  type_to_fetch = type & PHP_DNS_CNAME ? DNS_T_CNAME : 0; break;
    case 3:  type_to_fetch = type & PHP_DNS_SOA   ? DNS_T_SOA   : 0; break;
    case 4:  type_to_fetch = type & PHP_DNS_PTR   ? DNS_T_PTR   : 0; break;
    case 5:  type_to_fetch = type & PHP_DNS_HINFO ? DNS_T_HINFO : 0; break;
    case 6:  type_to_fetch = type & PHP_DNS_MX    ? DNS_T_MX    : 0; break;
    case 7:  type_to_fetch = type & PHP_DNS_TXT   ? DNS_T_TXT   : 0; break;
    case 8:  type_to_fetch = type & PHP_DNS_AAAA  ? DNS_T_AAAA  : 0; break;
    case 9:  type_to_fetch = type & PHP_DNS_SRV   ? DNS_T_SRV   : 0; break;
    case 10: type_to_fetch = type & PHP_DNS_NAPTR ? DNS_T_NAPTR : 0; break;
    case 11: type_to_fetch = type & PHP_DNS_A6    ? DNS_T_A6    : 0; break;
    case PHP_DNS_NUM_TYPES:
      store_results = false;
      continue;
    default:
    case (PHP_DNS_NUM_TYPES + 1):
      type_to_fetch = DNS_T_ANY;
      break;
    }
    if (!type_to_fetch) continue;

    struct __res_state *res;
    res = ResolverInit::s_res.get()->getResolver();
    if (res == NULL) {
      return false;
    }

    int n = res_nsearch(res, hostname.data(), C_IN, type_to_fetch,
                        answer.qb2, sizeof answer);
    if (n < 0) {
      res_nclose(res);
      php_dns_free_res(res);
      continue;
    }

    HEADER *hp;
    cp = answer.qb2 + HFIXEDSZ;
    end = answer.qb2 + n;
    hp = (HEADER *)&answer;
    qd = ntohs(hp->qdcount);
    an = ntohs(hp->ancount);
    ns = ntohs(hp->nscount);
    ar = ntohs(hp->arcount);

    /* Skip QD entries, they're only used by dn_expand later on */
    while (qd-- > 0) {
      n = dn_skipname(cp, end);
      if (n < 0) {
        raise_warning("Unable to parse DNS data received");
        res_nclose(res);
        php_dns_free_res(res);
        return false;
      }
      cp += n + QFIXEDSZ;
    }

    /* YAY! Our real answers! */
    while (an-- && cp && cp < end) {
      Array retval;
      cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, retval);
      if (!retval.empty() && store_results) {
        ret.append(retval);
      }
    }
    res_nclose(res);
    php_dns_free_res(res);
  }

  Array authns;
  Array addtl;

  /* List of Authoritative Name Servers */
  while (ns-- > 0 && cp && cp < end) {
    Array retval;
    cp = php_parserr(cp, end, &answer, DNS_T_ANY, true, retval);
    if (!retval.empty()) {
      authns.append(retval);
    }
  }

  /* Additional records associated with authoritative name servers */
  while (ar-- > 0 && cp && cp < end) {
    Array retval;
    cp = php_parserr(cp, end, &answer, DNS_T_ANY, true, retval);
    if (!retval.empty()) {
      addtl.append(retval);
    }
  }

  authnsRef = authns;
  addtlRef = addtl;
  return ret;
}

bool HHVM_FUNCTION(getmxrr, const String& hostname,
                            VRefParam mxhostsRef,
                            VRefParam weightsRef /* = null */) {
  IOStatusHelper io("dns_get_mx", hostname.data());
  int count, qdc;
  unsigned short type, weight;
  unsigned char ans[MAXPACKET];
  char buf[255 + 1];  // IETF STD 13 section 3.1; 255 bytes
  unsigned char *cp, *end;

  Array mxhosts;
  Array weights;
  SCOPE_EXIT {
    mxhostsRef = mxhosts;
    weightsRef = weights;
  };

  /* Go! */
  struct __res_state *res;
  res = ResolverInit::s_res.get()->getResolver();
  if (res == NULL) {
    return false;
  }

  int i = res_nsearch(res, hostname.data(), C_IN, DNS_T_MX,
                      (unsigned char*)&ans, sizeof(ans));
  if (i < 0) {
    res_nclose(res);
    php_dns_free_res(res);
    return false;
  }
  if (i > (int)sizeof(ans)) {
    i = sizeof(ans);
  }
  HEADER *hp = (HEADER *)&ans;
  cp = (unsigned char *)&ans + HFIXEDSZ;
  end = (unsigned char *)&ans +i;
  for (qdc = ntohs((unsigned short)hp->qdcount); qdc--; cp += i + QFIXEDSZ) {
    if ((i = dn_skipname(cp, end)) < 0 ) {
      res_nclose(res);
      php_dns_free_res(res);
      return false;
    }
  }
  count = ntohs((unsigned short)hp->ancount);
  while (--count >= 0 && cp < end) {
    if ((i = dn_skipname(cp, end)) < 0 ) {
      res_nclose(res);
      php_dns_free_res(res);
      return false;
    }
    cp += i;
    GETSHORT(type, cp);
    cp += INT16SZ + INT32SZ;
    GETSHORT(i, cp);
    if (type != DNS_T_MX) {
      cp += i;
      continue;
    }
    GETSHORT(weight, cp);
    if ((i = dn_expand(ans, end, cp, buf, sizeof(buf)-1)) < 0) {
      res_nclose(res);
      php_dns_free_res(res);
      return false;
    }
    cp += i;
    mxhosts.append(String(buf, CopyString));
    weights.append(weight);
  }
  res_nclose(res);
  php_dns_free_res(res);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// http

void HHVM_FUNCTION(header, const String& str, bool replace /* = true */,
                   int http_response_code /* = 0 */) {
  if (HHVM_FN(headers_sent)()) {
    raise_warning("Cannot modify header information - headers already sent");
  }

  String header = HHVM_FN(rtrim)(str);

  // new line safety check
  if (header.find('\n') >= 0 || header.find('\r') >= 0) {
    raise_error("Header may not contain more than a single header, "
                "new line detected");
    return;
  }

  Transport *transport = g_context->getTransport();
  if (transport && header.size()) {
    const char *header_line = header.data();

    // handle single line of status code
    if ((header.size() >= 5 && strncasecmp(header_line, "HTTP/", 5) == 0) ||
        (header.size() >= 7 && strncasecmp(header_line, "Status:", 7) == 0)) {
      int code = 200;
      const char *reason = nullptr;
      for (const char *ptr = header_line + 5; *ptr; ptr++) {
        if (*ptr == ' ' && *(ptr + 1) != ' ') {
          code = atoi(ptr + 1);
          for (ptr++; *ptr; ptr++) {
            if (*ptr == ' ' && *(ptr + 1) != ' ') {
              reason = ptr + 1;
              break;
            }
          }
          break;
        }
      }
      if (code) {
        transport->setResponse(code, reason);
      }
      return;
    }

    const char *colon_offset = strchr(header_line, ':');
    String newHeader;
    if (colon_offset) {
      if (!strncasecmp(header_line, "Content-Type",
                       colon_offset - header_line)) {
        const char *ptr = colon_offset+1, *mimetype = NULL;
        while (*ptr == ' ') ptr++;
        mimetype = ptr;
        if (strncmp(mimetype, "text/", 5) == 0 &&
            strstr(mimetype, "charset=") == NULL) {
          newHeader = header + ";charset=utf-8";
        }
      }
    }
    if (replace) {
      transport->replaceHeader(newHeader.empty() ? header : newHeader);
    } else {
      transport->addHeader(newHeader.empty() ? header : newHeader);
    }
    if (http_response_code) {
      transport->setResponse(http_response_code);
    }
  }
}

static IMPLEMENT_THREAD_LOCAL(int, s_response_code);

Variant HHVM_FUNCTION(http_response_code, int response_code /* = 0 */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    *s_response_code = transport->getResponseCode();
    if (response_code) {
      transport->setResponse(response_code);
    }
  }

  int old_code = *s_response_code;
  if (response_code) {
    *s_response_code = response_code;
  }

  if (old_code) {
    return old_code;
  }

  return response_code ? true : false;
}

Array HHVM_FUNCTION(headers_list) {
  Transport *transport = g_context->getTransport();
  Array ret = Array::Create();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      const std::vector<std::string> &values = iter->second;
      for (unsigned int i = 0; i < values.size(); i++) {
        ret.append(String(iter->first + ": " + values[i]));
      }
    }
  }
  return ret;
}

bool HHVM_FUNCTION(headers_sent, VRefParam file /* = null */,
                                 VRefParam line /* = null */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    file = String(transport->getFirstHeaderFile());
    line = transport->getFirstHeaderLine();
    return transport->headersSent();
  } else {
    return g_context->getStdoutBytesWritten() > 0;
  }
  return false;
}

Variant HHVM_FUNCTION(header_register_callback, const Variant& callback) {
  Transport *transport = g_context->getTransport();

  if (!is_callable(callback)) {
    raise_warning("First argument is expected to be a valid callback");
    return init_null();
  }

  if (!transport) {
    // fail if there is no transport
    return false;
  }
  if (transport->headersSent()) {
    // fail if headers have already been sent
    return false;
  }
  return transport->setHeaderCallback(callback);
}

void HHVM_FUNCTION(header_remove, const Variant& name /* = null_string */) {
  if (HHVM_FN(headers_sent)()) {
    raise_warning("Cannot modify header information - headers already sent");
  }
  Transport *transport = g_context->getTransport();
  if (transport) {
    if (name.isNull()) {
      transport->removeAllHeaders();
    } else {
      transport->removeHeader(name.toString().data());
    }
  }
}

int64_t HHVM_FUNCTION(get_http_request_size) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->getRequestSize();
  } else {
    return 0;
  }
}

bool HHVM_FUNCTION(setcookie, const String& name,
                              const String& value /* = null_string */,
                              int64_t expire /* = 0 */,
                              const String& path /* = null_string */,
                              const String& domain /* = null_string */,
                              bool secure /* = false */,
                              bool httponly /* = false */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->setCookie(name, value, expire, path, domain, secure,
                                httponly, true);
  }
  return false;
}

bool HHVM_FUNCTION(setrawcookie, const String& name,
                                 const String& value /* = null_string */,
                                 int64_t expire /* = 0 */,
                                 const String& path /* = null_string */,
                                 const String& domain /* = null_string */,
                                 bool secure /* = false */,
                                 bool httponly /* = false */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->setCookie(name, value, expire, path, domain, secure,
                                httponly, false);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(openlog, const String& ident, int option, int facility) {
  openlog(ident.data(), option, facility);
  return true;
}

bool HHVM_FUNCTION(closelog) {
  closelog();
  return true;
}

bool HHVM_FUNCTION(syslog, int priority, const String& message) {
  syslog(priority, "%s", message.data());
  return true;
}

void StandardExtension::initNetwork() {
  HHVM_FE(gethostname);
  HHVM_FE(gethostbyaddr);
  HHVM_FE(gethostbyname);
  HHVM_FE(gethostbynamel);
  HHVM_FE(getprotobyname);
  HHVM_FE(getprotobynumber);
  HHVM_FE(getservbyname);
  HHVM_FE(getservbyport);
  HHVM_FE(inet_ntop);
  HHVM_FE(inet_pton);
  HHVM_FE(ip2long);
  HHVM_FE(long2ip);
  HHVM_FE(checkdnsrr);
  HHVM_FE(dns_get_record);
  HHVM_FE(getmxrr);
  HHVM_FE(header);
  HHVM_FE(http_response_code);
  HHVM_FE(headers_list);
  HHVM_FE(headers_sent);
  HHVM_FE(header_register_callback);
  HHVM_FE(header_remove);
  HHVM_FE(get_http_request_size);
  HHVM_FE(setcookie);
  HHVM_FE(setrawcookie);
  HHVM_FE(openlog);
  HHVM_FE(closelog);
  HHVM_FE(syslog);

  // These are defined in ext_socket, but Zend has them in network
  HHVM_FE(fsockopen);
  HHVM_FE(pfsockopen);

  loadSystemlib("std_network");
}

///////////////////////////////////////////////////////////////////////////////
}
