/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/std/ext_std_network-internal.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

static Mutex NetworkMutex;

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
    file.assignIfRef(String(transport->getFirstHeaderFile()));
    line.assignIfRef(transport->getFirstHeaderLine());
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

bool validate_dns_arguments(const String& host, const String& type,
                            int& ntype) {
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

#define PHP_DNS_RC_INT(cns) Native::registerConstant<KindOfInt64> \
  (makeStaticString("DNS_" #cns), PHP_DNS_ ## cns);
  PHP_DNS_RC_INT(A);
  PHP_DNS_RC_INT(A6);
  PHP_DNS_RC_INT(AAAA);
  PHP_DNS_RC_INT(ALL);
  PHP_DNS_RC_INT(ANY);
  PHP_DNS_RC_INT(CNAME);
  PHP_DNS_RC_INT(HINFO);
  PHP_DNS_RC_INT(MX);
  PHP_DNS_RC_INT(NAPTR);
  PHP_DNS_RC_INT(NS);
  PHP_DNS_RC_INT(PTR);
  PHP_DNS_RC_INT(SOA);
  PHP_DNS_RC_INT(SRV);
  PHP_DNS_RC_INT(TXT);
#undef PHP_DNS_RC_INT

  HHVM_RC_INT_SAME(LOG_EMERG);
  HHVM_RC_INT_SAME(LOG_ALERT);
  HHVM_RC_INT_SAME(LOG_CRIT);
  HHVM_RC_INT_SAME(LOG_ERR);
  HHVM_RC_INT_SAME(LOG_WARNING);
  HHVM_RC_INT_SAME(LOG_NOTICE);
  HHVM_RC_INT_SAME(LOG_INFO);
  HHVM_RC_INT_SAME(LOG_DEBUG);

#ifdef LOG_KERN
   HHVM_RC_INT_SAME(LOG_KERN);
#endif
#ifdef LOG_USER
   HHVM_RC_INT_SAME(LOG_USER);
#endif
#ifdef LOG_MAIL
   HHVM_RC_INT_SAME(LOG_MAIL);
#endif
#ifdef LOG_DAEMON
   HHVM_RC_INT_SAME(LOG_DAEMON);
#endif
#ifdef LOG_AUTH
   HHVM_RC_INT_SAME(LOG_AUTH);
#endif
#ifdef LOG_SYSLOG
   HHVM_RC_INT_SAME(LOG_SYSLOG);
#endif
#ifdef LOG_LPR
   HHVM_RC_INT_SAME(LOG_LPR);
#endif
#ifdef LOG_PID
   HHVM_RC_INT_SAME(LOG_PID);
#endif
#ifdef LOG_CONS
   HHVM_RC_INT_SAME(LOG_CONS);
#endif
#ifdef LOG_ODELAY
   HHVM_RC_INT_SAME(LOG_ODELAY);
#endif
#ifdef LOG_NDELAY
   HHVM_RC_INT_SAME(LOG_NDELAY);
#endif
#ifdef LOG_NEWS
  HHVM_RC_INT_SAME(LOG_NEWS);
#endif
#ifdef LOG_UUCP
  HHVM_RC_INT_SAME(LOG_UUCP);
#endif
#ifdef LOG_CRON
  HHVM_RC_INT_SAME(LOG_CRON);
#endif
#ifdef LOG_AUTHPRIV
  HHVM_RC_INT_SAME(LOG_AUTHPRIV);
#endif
#ifdef LOG_NOWAIT
  HHVM_RC_INT_SAME(LOG_NOWAIT);
#endif
#ifdef LOG_PERROR
  HHVM_RC_INT_SAME(LOG_PERROR);
#endif

#ifndef _WIN32
  HHVM_RC_INT_SAME(LOG_LOCAL0);
  HHVM_RC_INT_SAME(LOG_LOCAL1);
  HHVM_RC_INT_SAME(LOG_LOCAL2);
  HHVM_RC_INT_SAME(LOG_LOCAL3);
  HHVM_RC_INT_SAME(LOG_LOCAL4);
  HHVM_RC_INT_SAME(LOG_LOCAL5);
  HHVM_RC_INT_SAME(LOG_LOCAL6);
  HHVM_RC_INT_SAME(LOG_LOCAL7);
#endif

  loadSystemlib("std_network");
}

///////////////////////////////////////////////////////////////////////////////
}
