/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <sys/utsname.h>

#include "folly/String.h"

#include "hphp/util/lock.h"
#include "hphp/util/process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// without calling res_init(), any call to getaddrinfo() may leak memory:
//  http://sources.redhat.com/ml/libc-hacker/2004-02/msg00049.html

class ResolverLibInitializer {
public:
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

std::string Util::safe_inet_ntoa(struct in_addr &in) {
  char buf[256];
  memset(buf, 0, sizeof(buf));
  inet_ntop(AF_INET, &in, buf, sizeof(buf)-1);
  return buf;
}

bool Util::safe_gethostbyname(const char *address, HostEnt &result) {
#if defined(__APPLE__)
  struct hostent *hp = gethostbyname(address);

  if (!hp) {
    return false;
  }

  result.hostbuf = *hp;
  freehostent(hp);
  return true;
#else
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
#endif
}

///////////////////////////////////////////////////////////////////////////////

std::string Util::GetPrimaryIP() {
  struct utsname buf;
  uname((struct utsname *)&buf);

  HostEnt result;
  if (!safe_gethostbyname(buf.nodename, result)) {
    return buf.nodename;
  }

  struct in_addr in;
  memcpy(&in.s_addr, *(result.hostbuf.h_addr_list), sizeof(in.s_addr));
  return safe_inet_ntoa(in);
}

static std::string normalizeIPv6Address(const std::string& address) {
  struct in6_addr addr;
  if (inet_pton(AF_INET6, address.c_str(), &addr) <= 0) {
    return std::string();
  }

  char ipPresentation[INET6_ADDRSTRLEN];
  if (inet_ntop(AF_INET6, &addr, ipPresentation, INET6_ADDRSTRLEN) == nullptr) {
    return std::string();
  }

  return ipPresentation;
}

Util::HostURL::HostURL(const std::string &hosturl, int port) :
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

  // IPv6 address?
  auto bpos = hosturl.find('[');
  if (bpos != std::string::npos) {
    // Extract out the IPAddress from [..]
    // Look for the ending position of ']'
    auto epos = hosturl.rfind(']');
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
        m_port = folly::to<uint16_t>(hosturl.substr(cpos + 1));
        m_hosturl += hosturl.substr(cpos);
      } catch (...) {
        m_port = 0;
      }
    }
    m_ipv6 = true;
  } else {
    // IPv4 or hostname
    auto cpos = hosturl.find(':', spos);
    if (cpos != std::string::npos) {
      m_host = hosturl.substr(spos, cpos - spos);
      m_hosturl += m_host;
      try {
        m_port = folly::to<uint16_t>(hosturl.substr(cpos + 1));
        m_hosturl += hosturl.substr(cpos);
      } catch (...) {
        m_port = 0;
      }
    } else {
      m_host = hosturl.substr(spos);
      m_hosturl += m_host;
    }
    m_ipv6 = false;
  }
  m_valid = true;
}

///////////////////////////////////////////////////////////////////////////////
}
