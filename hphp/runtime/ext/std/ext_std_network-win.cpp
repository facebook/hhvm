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
#ifdef _MSC_VER

#include "hphp/runtime/ext/std/ext_std_network.h"
#include "hphp/runtime/ext/std/ext_std_network-internal.h"

#include <Windns.h>

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


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

bool HHVM_FUNCTION(checkdnsrr, const String& host,
                               const String& type /* = null_string */) {
  int ntype;
  if (!validate_dns_arguments(host, type, ntype)) {
    return false;
  }
  DNS_STATUS      status;           /* Return value of DnsQuery_A() function */
  PDNS_RECORD     pResult;          /* Pointer to DNS_RECORD structure */
  status = DnsQuery_A(host.c_str(), ntype, DNS_QUERY_STANDARD, nullptr,
                      &pResult, nullptr);

  if (status) {
    return false;
  }

  return true;
}

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

static void php_parserr(PDNS_RECORD pRec, int type_to_fetch, int store,
                        Array* subarray)
{
  int type;
  u_long ttl;

  type = pRec->wType;
  ttl = pRec->dwTtl;

  if (type_to_fetch != DNS_TYPE_ANY && type != type_to_fetch) {
    return;
  }

  if (!store) {
    return;
  }

  subarray->set(s_host, pRec->pName);
  subarray->set(s_class, s_IN);
  subarray->set(s_ttl, (long)ttl);

  switch (type) {
  case DNS_TYPE_A: {
    IN_ADDR ipaddr;
    ipaddr.S_un.S_addr = (pRec->Data.A.IpAddress);
    subarray->set(s_type, s_A);
    subarray->set(s_ip, inet_ntoa(ipaddr));
    break;
  }

  case DNS_TYPE_MX:
    subarray->set(s_type, s_MX);
    subarray->set(s_pri, pRec->Data.Srv.wPriority);
    /* no break; */

  case DNS_TYPE_CNAME:
    if (type == DNS_TYPE_CNAME) {
      subarray->set(s_type, s_CNAME);
    }
    /* no break; */

  case DNS_TYPE_NS:
    if (type == DNS_TYPE_NS) {
      subarray->set(s_type, s_NS);
    }
    /* no break; */

  case DNS_TYPE_PTR:
    if (type == DNS_TYPE_PTR) {
      subarray->set(s_type, s_PTR);
    }
    subarray->set(s_target, pRec->Data.MX.pNameExchange);
    break;

    /* Not available on windows, the query is possible but
     * there is no DNS_HINFO_DATA structure */
  case DNS_TYPE_HINFO:
  case DNS_TYPE_TEXT:
  {
    DWORD i = 0;
    DNS_TXT_DATA *data_txt = &pRec->Data.TXT;
    DWORD count = data_txt->dwStringCount;
    String txt;

    subarray->set(s_type, s_TXT);
    for (i = 0; i < count; i++) {
      txt += String(data_txt->pStringArray[i], CopyString);
    }
    subarray->set(s_txt, txt);
  }
  break;

  case DNS_TYPE_SOA:
  {
    DNS_SOA_DATA *data_soa = &pRec->Data.Soa;

    subarray->set(s_type, s_SOA);

    subarray->set(s_mname, data_soa->pNamePrimaryServer);
    subarray->set(s_rname, data_soa->pNameAdministrator);
    subarray->set(s_serial, (int)data_soa->dwSerialNo);
    subarray->set(s_refresh, (int)data_soa->dwRefresh);
    subarray->set(s_retry, (int)data_soa->dwRetry);
    subarray->set(s_expire, (int)data_soa->dwExpire);
    subarray->set(s_minimum_ttl, (int)data_soa->dwDefaultTtl);
  }
  break;

  case DNS_TYPE_AAAA:
  {
    DNS_AAAA_DATA *data_aaaa = &pRec->Data.AAAA;
    char buf[sizeof("AAAA:AAAA:AAAA:AAAA:AAAA:AAAA:AAAA:AAAA")];
    char *tp = buf;
    int i;
    unsigned short out[8];
    int have_v6_break = 0, in_v6_break = 0;

    for (i = 0; i < 4; ++i) {
      DWORD chunk = data_aaaa->Ip6Address.IP6Dword[i];
      out[i * 2] = htons(LOWORD(chunk));
      out[i * 2 + 1] = htons(HIWORD(chunk));
    }

    for (i = 0; i < 8; i++) {
      if (out[i] != 0) {
        if (tp > buf) {
          in_v6_break = 0;
          tp[0] = ':';
          tp++;
        }
        tp += sprintf((char*)tp, "%x", out[i]);
      }
      else {
        if (!have_v6_break) {
          have_v6_break = 1;
          in_v6_break = 1;
          tp[0] = ':';
          tp++;
        }
        else if (!in_v6_break) {
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

    subarray->set(s_type, s_AAAA);
    subarray->set(s_ipv6, buf);
  }
  break;

  case DNS_TYPE_SRV:
  {
    DNS_SRV_DATA *data_srv = &pRec->Data.Srv;

    subarray->set(s_type, s_SRV);
    subarray->set(s_pri, data_srv->wPriority);
    subarray->set(s_weight, data_srv->wWeight);
    subarray->set(s_port, data_srv->wPort);
    subarray->set(s_target, data_srv->pNameTarget);
  }
  break;

  case DNS_TYPE_NAPTR:
  {
    DNS_NAPTR_DATA * data_naptr = &pRec->Data.Naptr;

    subarray->set(s_type, s_NAPTR);
    subarray->set(s_order, data_naptr->wOrder);
    subarray->set(s_pref, data_naptr->wPreference);
    subarray->set(s_flags, data_naptr->pFlags);
    subarray->set(s_services, data_naptr->pService);
    subarray->set(s_regex, data_naptr->pRegularExpression);
    subarray->set(s_replacement, data_naptr->pReplacement);
  }
  break;

  default:
    /* unknown type */
    subarray->clear();
    return;
  }

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

  Array ret = Array::Create();
  int type2;
  Array authns;
  Array addtl;
  if (type == PHP_DNS_ANY) {
    type2 = PHP_DNS_NUM_TYPES + 1;
  }
  else {
    type2 = 0;
  }

  bool first_query = true;
  bool store_results = true;
  for (;
  type2 < (!addtlRef.isNull()
        ? (PHP_DNS_NUM_TYPES + 2)
        : PHP_DNS_NUM_TYPES) || first_query;
    type2++
    ) {
    first_query = false;
    int type_to_fetch;
    DNS_STATUS      status; /* Return value of DnsQuery_A() function */
    PDNS_RECORD     pResult, pRec; /* Pointer to DNS_RECORD structure */

    switch (type2) {
    case -1: /* raw */
      type_to_fetch = type;
      /* skip over the rest and go directly to additional records */
      type = PHP_DNS_NUM_TYPES - 1;
      break;
    case 0:
      type_to_fetch = type&PHP_DNS_A ? DNS_TYPE_A : 0;
      break;
    case 1:
      type_to_fetch = type&PHP_DNS_NS ? DNS_TYPE_NS : 0;
      break;
    case 2:
      type_to_fetch = type&PHP_DNS_CNAME ? DNS_TYPE_CNAME : 0;
      break;
    case 3:
      type_to_fetch = type&PHP_DNS_SOA ? DNS_TYPE_SOA : 0;
      break;
    case 4:
      type_to_fetch = type&PHP_DNS_PTR ? DNS_TYPE_PTR : 0;
      break;
    case 5:
      type_to_fetch = type&PHP_DNS_HINFO ? DNS_TYPE_HINFO : 0;
      break;
    case 6:
      type_to_fetch = type&PHP_DNS_MX ? DNS_TYPE_MX : 0;
      break;
    case 7:
      type_to_fetch = type&PHP_DNS_TXT ? DNS_TYPE_TEXT : 0;
      break;
    case 8:
      type_to_fetch = type&PHP_DNS_AAAA ? DNS_TYPE_AAAA : 0;
      break;
    case 9:
      type_to_fetch = type&PHP_DNS_SRV ? DNS_TYPE_SRV : 0;
      break;
    case 10:
      type_to_fetch = type&PHP_DNS_NAPTR ? DNS_TYPE_NAPTR : 0;
      break;
    case 11:
      type_to_fetch = type&PHP_DNS_A6 ? DNS_TYPE_A6 : 0;
      break;
    case PHP_DNS_NUM_TYPES:
      store_results = false;
      continue;
    default:
    case (PHP_DNS_NUM_TYPES + 1) :
      type_to_fetch = DNS_TYPE_ANY;
      break;
    }

    if (type_to_fetch) {
      status = DnsQuery_A(hostname.c_str(), type_to_fetch,
                          DNS_QUERY_STANDARD, nullptr, &pResult, nullptr);

      if (status) {
        if (status == DNS_INFO_NO_RECORDS ||
            status == DNS_ERROR_RCODE_NAME_ERROR) {
          continue;
        }
        else {
          raise_warning("DNS Query failed");
          return false;
        }
      }

      for (pRec = pResult; pRec; pRec = pRec->pNext) {
        Array retval;

        if (pRec->Flags.S.Section == DnsSectionAnswer) {
          php_parserr(pRec, type_to_fetch, store_results, &retval);
          if (!retval.empty() && store_results) {
            ret.append(retval);
          }
        }

        if (!authns.isNull() && pRec->Flags.S.Section == DnsSectionAuthority) {

          php_parserr(pRec, type_to_fetch, 1, &retval);
          if (!retval.empty()) {
            authns.append(retval);
          }
        }

        /* Stupid typo in PSDK 6.1, WinDNS.h(1258)... */
#ifndef DnsSectionAdditional
# ifdef DnsSectionAddtional
#  define DnsSectionAdditional DnsSectionAddtional
# else
# define DnsSectionAdditional 3
# endif
#endif
        if (!addtlRef.isNull() &&
            pRec->Flags.S.Section == DnsSectionAdditional) {
          php_parserr(pRec, type_to_fetch, 1, &retval);
          if (!retval.empty()) {
            addtl.append(retval);
          }
        }
      }
      /* Free memory allocated for DNS records. */
      DnsRecordListFree(pResult, DnsFreeRecordListDeep);
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
  Array mxhosts;
  Array weights;
  SCOPE_EXIT {
    mxhostsRef.assignIfRef(mxhosts);
    weightsRef.assignIfRef(weights);
  };

  DNS_STATUS      status;         /* Return value of DnsQuery_A() function */
  PDNS_RECORD     pResult, pRec;  /* Pointer to DNS_RECORD structure */

  status = DnsQuery_A(hostname.c_str(), DNS_TYPE_MX, DNS_QUERY_STANDARD,
                      nullptr, &pResult, nullptr);

  if (status) {
    return false;
  }

  for (pRec = pResult; pRec; pRec = pRec->pNext) {
    DNS_SRV_DATA *srv = &pRec->Data.Srv;

    if (pRec->wType != DNS_TYPE_MX) {
      continue;
    }

    mxhosts.append(String(pRec->Data.MX.pNameExchange, CopyString));
    weights.append(srv->wPriority);
  }

  /* Free memory allocated for DNS records. */
  DnsRecordListFree(pResult, DnsFreeRecordListDeep);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
#endif
