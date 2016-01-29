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
#ifndef _MSC_VER
#include "hphp/runtime/ext/std/ext_std_network.h"
#include "hphp/runtime/ext/std/ext_std_network-internal.h"

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


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

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
  int ntype;
  if (!validate_dns_arguments(host, type, ntype)) {
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
  Array ret = Array::Create();
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

  authnsRef.assignIfRef(authns);
  addtlRef.assignIfRef(addtl);
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
    mxhostsRef.assignIfRef(mxhosts);
    weightsRef.assignIfRef(weights);
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
}
#endif
