/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/util/network.h"

#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/utsname.h>

#include <folly/IPAddress.h>
#include <folly/String.h>
#include <folly/portability/Sockets.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// without calling res_init(), any call to getaddrinfo() may leak memory:
//  http://sources.redhat.com/ml/libc-hacker/2004-02/msg00049.html

struct ResolverLibInitializer {
  ResolverLibInitializer() {
    res_init();
    // We call sethostent with stayopen = 1 to keep /etc/hosts open across calls
    // to prevent mmap contention inside the kernel.  Two calls are necessary to
    // properly initialize the stayopen flag in glibc.
    sethostent(1);
    sethostent(1);
  }
};
static ResolverLibInitializer _resolver_lib_initializer;

///////////////////////////////////////////////////////////////////////////////
// thread-safe network functions

bool safe_gethostbyname(const char *address, HostEnt &result) {
  struct hostent *hp;
  int res;

  size_t hstbuflen = 1024;
  result.tmphstbuf = (char*)malloc(hstbuflen);
  while ((res = gethostbyname_r(address, &result.hostbuf, result.tmphstbuf,
                                hstbuflen, &hp, &result.herr)) == ERANGE) {
    hstbuflen *= 2;
    result.tmphstbuf = (char*)realloc(result.tmphstbuf, hstbuflen);
  }
  return !res && hp;
}

///////////////////////////////////////////////////////////////////////////////
std::string GetPrimaryIPImpl(int af) {
  const static std::string s_empty;
  struct addrinfo hints;
  struct addrinfo *res = nullptr;
  int error;

  SCOPE_EXIT {
    if (res) {
      freeaddrinfo(res);
    }
  };

  struct utsname buf;
  uname((struct utsname *)&buf);
  const char* nodename = buf.nodename;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = af;

  error = getaddrinfo(nodename, nullptr, &hints, &res);
  if (error) {
    return s_empty;
  }

  try {
    return folly::IPAddress(res->ai_addr).toFullyQualified();
  } catch (folly::IPAddressFormatException& ) {
    return s_empty;
  }
}

std::string GetPrimaryIPv4() {
  return GetPrimaryIPImpl(AF_INET);
}

std::string GetPrimaryIPv6() {
  return GetPrimaryIPImpl(AF_INET6);
}

std::string GetPrimaryIP() {
  auto ipaddress = GetPrimaryIPv4();
  if (ipaddress.empty()) {
    ipaddress = GetPrimaryIPv6();
  }
  return ipaddress;
}

static std::string normalizeIPv6Address(std::string address) {
  // strip the scopeId
  std::string scopeId;
  auto it = address.find('%');
  if (it != std::string::npos) {
    scopeId = address.substr(it + 1);
    address = address.substr(0, it);
  }

  struct in6_addr addr;
  if (inet_pton(AF_INET6, address.c_str(), &addr) <= 0) {
    return std::string();
  }

  char ipPresentation[INET6_ADDRSTRLEN];
  if (inet_ntop(AF_INET6, &addr, ipPresentation, INET6_ADDRSTRLEN) == nullptr) {
    return std::string();
  }

  // restore the scopeId if it was originally present
  std::string res(ipPresentation);
  if (!scopeId.empty()) {
    res += "%" + scopeId;
  }

  return res;
}

HostURL::HostURL(const std::string &hosturl, int port) :
  m_ipv6(false), m_port(port) {

  // Find the scheme
  auto spos = hosturl.find("://");
  if (spos != std::string::npos) {
    m_hosturl = m_scheme = hosturl.substr(0, spos);
    m_hosturl += "://";
    spos += 3;
  } else {
    spos = 0;
  }

  // strip off /EXTRA from prot://addr:port/EXTRA
  auto extraPos = hosturl.find('/', spos);
  auto validLen = extraPos != std::string::npos ? extraPos : hosturl.size();

  // IPv6 address?
  auto bpos = hosturl.find('[');
  if (bpos != std::string::npos) {
    // Extract out the IPAddress from [..]
    // Look for the ending position of ']'
    auto epos = hosturl.rfind(']', validLen - 1);
    if (epos == std::string::npos) {
      // This isn't a valid IPv6 address, so bail.
      m_valid = false;
      m_hosturl = hosturl;
      return;
    }

    // IPv6 address between '[' and ']'
    auto v6h = normalizeIPv6Address(hosturl.substr(bpos + 1, epos - bpos - 1));
    if (v6h.empty()) {
      m_valid = false;
      m_hosturl = hosturl;
      return;
    }
    m_host = v6h;
    m_hosturl += '[';
    m_hosturl += v6h;
    m_hosturl += ']';

    // Colon for port.  Start after ']';
    auto cpos = hosturl.find(':', epos);
    if (cpos != std::string::npos) {
      try {
        auto portLen = validLen - cpos - 1;
        m_port = folly::to<uint16_t>(hosturl.substr(cpos + 1, portLen));
        m_hosturl += hosturl.substr(cpos);
      } catch (...) {
        m_port = 0;
      }
    } else if (extraPos != std::string::npos) {
      m_hosturl += hosturl.substr(extraPos);
    }
    m_ipv6 = true;
  } else if (m_scheme == "unix" || m_scheme == "udg") {
    // unix socket
    m_host = hosturl.substr(spos);
    m_hosturl += m_host;
  } else {
    // IPv4 or hostname
    auto cpos = hosturl.find(':', spos);
    if (cpos != std::string::npos) {
      m_host = hosturl.substr(spos, cpos - spos);
      m_hosturl += m_host;
      try {
        auto portLen = validLen - cpos - 1;
        m_port = folly::to<uint16_t>(hosturl.substr(cpos + 1, portLen));
        m_hosturl += hosturl.substr(cpos);
      } catch (...) {
        m_port = 0;
      }
    } else {
      m_host = hosturl.substr(spos, validLen - spos);
      m_hosturl += hosturl.substr(spos);
    }
    m_ipv6 = false;
  }
  m_valid = true;
}

///////////////////////////////////////////////////////////////////////////////
}
