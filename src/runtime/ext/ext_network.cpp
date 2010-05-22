/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_network.h>
#include <runtime/ext/ext_apc.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_stats.h>
#include <util/lock.h>
#include <runtime/base/file/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <util/network.h>

#define MAXPACKET  8192 /* max packet size used internally by BIND */
#define DNS_T_A		1
#define DNS_T_NS	2
#define DNS_T_CNAME	5
#define DNS_T_SOA	6
#define DNS_T_PTR	12
#define DNS_T_HINFO	13
#define DNS_T_MINFO	14
#define DNS_T_MX	15
#define DNS_T_TXT	16
#define DNS_T_AAAA	28
#define DNS_T_SRV	33
#define DNS_T_NAPTR	35
#define DNS_T_A6	38
#define DNS_T_ANY	255

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

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

static Mutex NetworkMutex;

Variant f_gethostbyaddr(CStrRef ip_address) {
  IOStatusHelper io("gethostbyaddr", ip_address.data());
  Lock lock(NetworkMutex);

  struct in6_addr addr6;
  struct in_addr addr;
  struct hostent *hp;
  if (inet_pton(AF_INET6, ip_address.data(), &addr6)) {
    hp = gethostbyaddr((char *) &addr6, sizeof(addr6), AF_INET6);
  } else if (inet_pton(AF_INET, ip_address.data(), &addr)) {
    hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
  } else {
    return false;
  }

  if (!hp || hp->h_name == NULL || hp->h_name[0] == '\0') {
    return ip_address;
  }

  return String(hp->h_name, CopyString);
}

String f_gethostbyname(CStrRef hostname) {
  IOStatusHelper io("gethostbyname", hostname.data());
  if (RuntimeOption::EnableDnsCache) {
    Variant success;
    Variant resolved = f_apc_fetch(hostname, ref(success),
                                   SHARED_STORE_DNS_CACHE);
    if (same(success, true)) {
      if (same(resolved, false)) {
        return hostname;
      }
      return resolved.toString();
    }
  }

  Util::HostEnt result;
  if (!Util::safe_gethostbyname(hostname.data(), result)) {
    if (RuntimeOption::EnableDnsCache) {
      f_apc_store(hostname, false, RuntimeOption::DnsCacheTTL,
                  SHARED_STORE_DNS_CACHE);
    }
    return hostname;
  }

  struct in_addr in;
  memcpy(&in.s_addr, *(result.hostbuf.h_addr_list), sizeof(in.s_addr));
  String ret(Util::safe_inet_ntoa(in));
  if (RuntimeOption::EnableDnsCache) {
    f_apc_store(hostname, ret, RuntimeOption::DnsCacheTTL,
                SHARED_STORE_DNS_CACHE);
  }
  return ret;
}

Variant f_gethostbynamel(CStrRef hostname) {
  IOStatusHelper io("gethostbynamel", hostname.data());
  Util::HostEnt result;
  if (!Util::safe_gethostbyname(hostname.data(), result)) {
    return false;
  }

  Array ret;
  for (int i = 0 ; result.hostbuf.h_addr_list[i] != 0 ; i++) {
    struct in_addr in = *(struct in_addr *)result.hostbuf.h_addr_list[i];
    ret.append(String(Util::safe_inet_ntoa(in)));
  }
  return ret;
}

Variant f_getprotobyname(CStrRef name) {
  Lock lock(NetworkMutex);

  struct protoent *ent = getprotobyname(name.data());
  if (ent == NULL) {
    return false;
  }
  return ent->p_proto;
}

Variant f_getprotobynumber(int number) {
  Lock lock(NetworkMutex);

  struct protoent *ent = getprotobynumber(number);
  if (ent == NULL) {
    return false;
  }
  return String(ent->p_name, CopyString);
}

Variant f_getservbyname(CStrRef service, CStrRef protocol) {
  Lock lock(NetworkMutex);

  struct servent *serv = getservbyname(service.data(), protocol.data());
  if (serv == NULL) {
    return false;
  }
  return ntohs(serv->s_port);
}

Variant f_getservbyport(int port, CStrRef protocol) {
  Lock lock(NetworkMutex);

  struct servent *serv = getservbyport(htons(port), protocol.data());
  if (serv == NULL) {
    return false;
  }
  return String(serv->s_name, CopyString);
}

Variant f_inet_ntop(CStrRef in_addr) {
  int af = AF_INET;
  if (in_addr.size() == 16) {
    af = AF_INET6;
  } else if (in_addr.size() != 4) {
    raise_warning("Invalid in_addr value");
    return false;
  }

  char buffer[40];
  if (!inet_ntop(af, in_addr.data(), buffer, sizeof(buffer))) {
    raise_warning("An unknown error occured");
    return false;
  }
  return String(buffer, CopyString);
}

Variant f_inet_pton(CStrRef address) {
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

Variant f_ip2long(CStrRef ip_address) {
  unsigned long int ip;
  if (ip_address.empty() ||
      (ip = inet_addr(ip_address.data())) == INADDR_NONE) {
    /* the only special case when we should return -1 ourselves,
     * because inet_addr() considers it wrong. We return 0xFFFFFFFF and
     * not -1 or ~0 because of 32/64bit issues.
     */
    if (ip_address == "255.255.255.255") {
      return (int64)0xFFFFFFFF;
    }
    return false;
  }
  return (int64)ntohl(ip);
}

String f_long2ip(int proper_address) {
  struct in_addr myaddr;
  myaddr.s_addr = htonl(proper_address);
  return Util::safe_inet_ntoa(myaddr);
}

bool f_dns_check_record(CStrRef host, CStrRef type /* = null_string */) {
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
  return res_search(host.data(), C_IN, ntype, ans, sizeof(ans)) >= 0;
}

typedef union {
  HEADER qb1;
  u_char qb2[65536];
} querybuf;

static void php_dns_free_res(struct __res_state res) {
  int ns;
  for (ns = 0; ns < MAXNS; ns++) {
    if (res._u._ext.nsaddrs[ns] != NULL) {
      free(res._u._ext.nsaddrs[ns]);
      res._u._ext.nsaddrs[ns] = NULL;
    }
  }
}

static unsigned char *php_parserr(unsigned char *cp, querybuf *answer,
                                  int type_to_fetch, bool store,
                                  Array &subarray) {
  unsigned short type, cls, dlen;
  unsigned long ttl;
  int64 n, i;
  unsigned short s;
  unsigned char *tp, *p;
  char name[MAXHOSTNAMELEN];
  int have_v6_break = 0, in_v6_break = 0;

  n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, sizeof(name) - 2);
  if (n < 0) {
    return NULL;
  }
  cp += n;

  GETSHORT(type, cp);
  GETSHORT(cls, cp);
  GETLONG(ttl, cp);
  GETSHORT(dlen, cp);
  if (type_to_fetch != T_ANY && type != type_to_fetch) {
    cp += dlen;
    return cp;
  }

  if (!store) {
    cp += dlen;
    return cp;
  }

  subarray.set("host", String(name, CopyString));
  switch (type) {
  case DNS_T_A:
    subarray.set("type", "A");
    snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
    subarray.set("ip", String(name, CopyString));
    cp += dlen;
    break;
  case DNS_T_MX:
    subarray.set("type", "MX");
    GETSHORT(n, cp);
    subarray.set("pri", n);
    /* no break; */
  case DNS_T_CNAME:
    if (type == DNS_T_CNAME) {
      subarray.set("type", "CNAME");
    }
    /* no break; */
  case DNS_T_NS:
    if (type == DNS_T_NS) {
      subarray.set("type", "NS");
    }
    /* no break; */
  case DNS_T_PTR:
    if (type == DNS_T_PTR) {
      subarray.set("type", "PTR");
    }
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set("target", String(name, CopyString));
    break;
  case DNS_T_HINFO:
    /* See RFC 1010 for values */
    subarray.set("type", "HINFO");
    n = *cp & 0xFF;
    cp++;
    subarray.set("cpu", String((const char *)cp, n, CopyString));
    cp += n;
    n = *cp & 0xFF;
    cp++;
    subarray.set("os", String((const char *)cp, n, CopyString));
    cp += n;
    break;
  case DNS_T_TXT:
    subarray.set("type", "TXT");
    n = cp[0];
    tp = (unsigned char *)malloc(n + 1);
    memcpy(tp, cp + 1, n);
    tp[n] = '\0';
    cp += dlen;
    subarray.set("txt", String((const char *)tp, n, AttachString));
    break;
  case DNS_T_SOA:
    subarray.set("type", "SOA");
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) -2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set("mname", String(name, CopyString));
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) -2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set("rname", String(name, CopyString));
    GETLONG(n, cp);
    subarray.set("serial", n);
    GETLONG(n, cp);
    subarray.set("refresh", n);
    GETLONG(n, cp);
    subarray.set("retry", n);
    GETLONG(n, cp);
    subarray.set("expire", n);
    GETLONG(n, cp);
    subarray.set("minimum-ttl", n);
    break;
  case DNS_T_AAAA:
    tp = (unsigned char *)name;
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
    subarray.set("type", "AAAA");
    subarray.set("ipv6", String(name, CopyString));
    break;
  case DNS_T_A6:
    p = cp;
    subarray.set("type", "A6");
    n = ((int)cp[0]) & 0xFF;
    cp++;
    subarray.set("masklen", n);
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
    subarray.set("ipv6", String(name, CopyString));
    if (cp < p + dlen) {
      n = dn_expand(answer->qb2, answer->qb2+65536, cp, name,
                    (sizeof name) - 2);
      if (n < 0) {
        return NULL;
      }
      cp += n;
      subarray.set("chain", String(name, CopyString));
    }
    break;
  case DNS_T_SRV:
    subarray.set("type", "SRV");
    GETSHORT(n, cp);
    subarray.set("pri", n);
    GETSHORT(n, cp);
    subarray.set("weight", n);
    GETSHORT(n, cp);
    subarray.set("port", n);
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set("target", String(name, CopyString));
    break;
  case DNS_T_NAPTR:
    subarray.set("type", "NAPTR");
    GETSHORT(n, cp);
    subarray.set("order", n);
    GETSHORT(n, cp);
    subarray.set("pref", n);
    n = (cp[0] & 0xFF);
    subarray.set("flags", String((const char *)(++cp), n, CopyString));
    cp += n;
    n = (cp[0] & 0xFF);
    subarray.set("services", String((const char *)(++cp), n, CopyString));
    cp += n;
    n = (cp[0] & 0xFF);
    subarray.set("regex", String((const char *)(++cp), n, CopyString));
    cp += n;
    n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
    if (n < 0) {
      return NULL;
    }
    cp += n;
    subarray.set("replacement", String(name, CopyString));
    break;
  default:
    cp += dlen;
  }

  subarray.set("class", "IN");
  subarray.set("ttl", (int)ttl);
  return cp;
}

Variant f_dns_get_record(CStrRef hostname, int type /* = -1 */,
                         Variant authns /* = null */,
                         Variant addtl /* = null */) {
  IOStatusHelper io("dns_get_record", hostname.data(), type);
  if (type < 0) type = PHP_DNS_ALL;
  if (type & ~PHP_DNS_ALL && type != PHP_DNS_ANY) {
    raise_warning("Type '%d' not supported", type);
    return false;
  }

  /* Initialize the return array */
  Array ret;
  authns = Array::Create();
  addtl = Array::Create();

  unsigned char *cp = NULL, *end = NULL;
  int qd, an, ns = 0, ar = 0;
  querybuf buf, answer;

  /* - We emulate an or'ed type mask by querying type by type.
   *   (Steps 0 - NUMTYPES-1 )
   *   If additional info is wanted we check again with DNS_T_ANY
   *   (step NUMTYPES / NUMTYPES+1 )
   *   store_results is used to skip storing the results retrieved in step
   *   NUMTYPES+1 when results were already fetched.
   * - In case of PHP_DNS_ANY we use the directly fetch DNS_T_ANY.
   *   (step NUMTYPES+1 )
   */
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

    struct __res_state res;
    memset(&res, 0, sizeof(res));
    res_ninit(&res);
    res.retrans = 5;
    res.options &= ~RES_DEFNAMES;

    int n = res_nmkquery(&res, QUERY, hostname.data(), C_IN, type_to_fetch,
                         NULL, 0, NULL, buf.qb2, sizeof buf);
    if (n < 0) {
      raise_warning("res_nmkquery() failed");
      res_nclose(&res);
      php_dns_free_res(res);
      return false;
    }

    n = res_nsend(&res, buf.qb2, n, answer.qb2, sizeof answer);
    if (n < 0) {
      raise_warning("res_nsend() failed");
      res_nclose(&res);
      php_dns_free_res(res);
      return false;
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
        res_nclose(&res);
        php_dns_free_res(res);
        return false;
      }
      cp += n + QFIXEDSZ;
    }

    /* YAY! Our real answers! */
    while (an-- && cp && cp < end) {
      Array retval;
      cp = php_parserr(cp, &answer, type_to_fetch, store_results, retval);
      if (!retval.empty() && store_results) {
        ret.append(retval);
      }
    }
    res_nclose(&res);
    php_dns_free_res(res);
  }

  /* List of Authoritative Name Servers */
  while (ns-- > 0 && cp && cp < end) {
    Array retval;
    cp = php_parserr(cp, &answer, DNS_T_ANY, true, retval);
    if (!retval.empty()) {
      authns.append(retval);
    }
  }

  /* Additional records associated with authoritative name servers */
  while (ar-- > 0 && cp && cp < end) {
    Array retval;
    cp = php_parserr(cp, &answer, DNS_T_ANY, true, retval);
    if (!retval.empty()) {
      addtl.append(retval);
    }
  }

  return ret;
}

bool f_dns_get_mx(CStrRef hostname, Variant mxhosts,
                  Variant weights /* = null */) {
  IOStatusHelper io("dns_get_mx", hostname.data());
  int count, qdc;
  unsigned short type, weight;
  unsigned char ans[MAXPACKET];
  char buf[MAXHOSTNAMELEN];
  unsigned char *cp, *end;

  mxhosts = Array::Create();
  weights = Array::Create();

  /* Go! */
  int i = res_search(hostname.data(), C_IN, DNS_T_MX, (unsigned char *)&ans,
                     sizeof(ans));
  if (i < 0) {
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
      return false;
    }
  }
  count = ntohs((unsigned short)hp->ancount);
  while (--count >= 0 && cp < end) {
    if ((i = dn_skipname(cp, end)) < 0 ) {
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
      return false;
    }
    cp += i;
    mxhosts.append(String(buf, CopyString));
    weights.append(weight);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// socket

/**
 * f_fsockopen() and f_pfsockopen() are implemented in ext_socket.cpp.
 */

///////////////////////////////////////////////////////////////////////////////
// http

void f_header(CStrRef str, bool replace /* = true */,
              int http_response_code /* = 0 */) {
  if (f_headers_sent()) {
    raise_warning("Cannot modify header information - headers already sent");
  }

  // new line safety check
  // NOTE: PHP actually allows "\n " and "\n\t" to fall through. Is that bad
  // for security?
  if (str.find('\n') >= 0) {
    raise_warning("Header may not contain more than a single header, "
                  "new line detected");
    return;
  }

  Transport *transport = g_context->getTransport();
  if (transport) {
    const char *header_line = str->data();

    // handle single line of status code
    if (str->size() >= 5 && strncasecmp(header_line, "HTTP/", 5) == 0) {
      int code = 200;
      for (const char *ptr = header_line; *ptr; ptr++) {
        if (*ptr == ' ' && *(ptr + 1) != ' ') {
          code = atoi(ptr + 1);
          break;
        }
      }
      if (code) {
        transport->setResponse(code);
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
          newHeader = str + ";charset=utf-8";
        }
      }
    }
    if (replace) {
      transport->replaceHeader(newHeader.empty() ? str : newHeader);
    } else {
      transport->addHeader(newHeader.empty() ? str : newHeader);
    }
    if (http_response_code) {
      transport->setResponse(http_response_code);
    }
  }
}

Array f_headers_list() {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    Array ret;
    for (HeaderMap::const_iterator iter = headers.begin();
         iter != headers.end(); ++iter) {
      const vector<string> &values = iter->second;
      for (unsigned int i = 0; i < values.size(); i++) {
        ret.append(String(iter->first + ": " + values[i]));
      }
    }
    return ret;
  }
  return Array();
}

bool f_headers_sent(Variant file /* = null */, Variant line /* = null */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->headersSent();
  }
  return false;
}

void f_header_remove(CStrRef name /* = null_string */) {
  if (f_headers_sent()) {
    raise_warning("Cannot modify header information - headers already sent");
  }
  Transport *transport = g_context->getTransport();
  if (transport) {
    if (name.isNull()) {
      transport->removeAllHeaders();
    } else {
      transport->removeHeader(name.data());
    }
  }
}

bool f_setcookie(CStrRef name, CStrRef value /* = null_string */,
                 int expire /* = 0 */, CStrRef path /* = null_string */,
                 CStrRef domain /* = null_string */, bool secure /* = false */,
                 bool httponly /* = false */) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->setCookie(name, value, expire, path, domain, secure,
                                httponly, true);
  }
  return false;
}

bool f_setrawcookie(CStrRef name, CStrRef value /* = null_string */,
                    int expire /* = 0 */, CStrRef path /* = null_string */,
                    CStrRef domain /* = null_string */,
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
}
