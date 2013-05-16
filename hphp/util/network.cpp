/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/util/lock.h"
#include "hphp/util/process.h"
#include "util.h"

#include "netinet/in.h"
#include "arpa/nameser.h"
#include <resolv.h>

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

///////////////////////////////////////////////////////////////////////////////
}
