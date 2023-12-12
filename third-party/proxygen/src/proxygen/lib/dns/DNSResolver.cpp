/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/DNSResolver.h"

#include <folly/Conv.h>
#include <glog/logging.h>

using folly::SocketAddress;
using proxygen::DNSResolver;

namespace proxygen {

const std::chrono::seconds DNSResolver::kInvalidDnsTtl =
    std::chrono::seconds(0);

const std::chrono::milliseconds DNSResolver::kMaxTimeout =
    std::chrono::milliseconds(60 * 1000);

const std::chrono::seconds DNSResolver::kLiteralTTL = std::chrono::seconds(60);

const int DNSResolver::kMaxCnameResolutions = 9;

DNSResolver::ResolutionCallback::~ResolutionCallback() {
  this->cancelResolution();
}

void DNSResolver::ResolutionCallback::cancelResolution() {
  std::set<QueryBase*> queries;
  {
    std::lock_guard<std::mutex> g(mutex_);
    queries = std::move(queries_);
    // the clear is not really needed but I'll keep it here in case
    // we decide to use a copy assignment later
    queries_.clear();
  }
  for (auto q : queries) {
    q->cancelResolutionImpl();
  }
}

std::string DNSResolver::getPtrName(const SocketAddress& address) {
  static const char kHexVals[] = {'0',
                                  '1',
                                  '2',
                                  '3',
                                  '4',
                                  '5',
                                  '6',
                                  '7',
                                  '8',
                                  '9',
                                  'a',
                                  'b',
                                  'c',
                                  'd',
                                  'e',
                                  'f'};
  static const char kIPv4Domain[] = "in-addr.arpa.";
  static const char kIPv6Domain[] = "ip6.arpa.";
  char buf[128];

  switch (address.getFamily()) {
    case AF_INET: {

      const in_addr ia = address.getIPAddress().asV4().toAddr();
      const in_addr* iap = &ia;

#ifdef _WIN32
#define PRIsockaddr "%lu"
#else
#define PRIsockaddr "%d"
#endif
      snprintf(buf,
               sizeof(buf),
               PRIsockaddr "." PRIsockaddr "." PRIsockaddr "." PRIsockaddr
                           ".%s",
               (iap->s_addr >> 24) & 0xff,
               (iap->s_addr >> 16) & 0xff,
               (iap->s_addr >> 8) & 0xff,
               iap->s_addr & 0xff,
               kIPv4Domain);

      break;
    }

    case AF_INET6: {
      const in6_addr i6a = address.getIPAddress().asV6().toAddr();
      const in6_addr* i6ap = &i6a;

      char* bufp = buf;
      for (int i = sizeof(i6ap->s6_addr) - 1; i >= 0; --i) {
        *(bufp++) = kHexVals[(i6ap->s6_addr[i]) & 0xf];
        *(bufp++) = '.';
        *(bufp++) = kHexVals[(i6ap->s6_addr[i] >> 4) & 0xf];
        *(bufp++) = '.';
      }
      *bufp = 0;
      memcpy(bufp, kIPv6Domain, sizeof(kIPv6Domain));

      break;
    }

    default:
      LOG(FATAL) << "Unsupported address family " << address.getFamily()
                 << " could not be turned into a PTR name";
  }

  return std::string(buf);
}

void DNSResolver::resolveMailExchange(ResolutionCallback* /*cb*/,
                                      const std::string& /*domain*/,
                                      std::chrono::milliseconds /*timeout*/) {
}

std::string describe(const DNSResolver::ResolutionStatus status, bool details) {
#define DNSRESOLVER_RESOLUTION_STATUS_STR(sym, descr) #sym,
  static const char* errorTable[] = {
      DNSRESOLVER_RESOLUTION_STATUS_GEN(DNSRESOLVER_RESOLUTION_STATUS_STR)};
#undef DNSRESOLVER_RESOLUTION_STATUS_STR

#define DNSRESOLVER_RESOLUTION_STATUS_DETAILS(sym, descr) descr,
  static const char* detailsTable[] = {
      DNSRESOLVER_RESOLUTION_STATUS_GEN(DNSRESOLVER_RESOLUTION_STATUS_DETAILS)};
#undef DNSRESOLVER_RESOLUTION_STATUS_DETAILS

  size_t idx = static_cast<size_t>(status);

  if (!details) {
    return errorTable[idx];
  } else {
    return folly::to<std::string>(
        errorTable[idx], " (", detailsTable[idx], ")");
  }
}

std::ostream& operator<<(std::ostream& os,
                         const DNSResolver::ResolutionStatus status) {
  os << describe(status, true);
  return os;
}

folly::StringPiece familyToString(sa_family_t family) {
  switch (family) {
    case AF_INET:
      return "AF_INET";
    case AF_INET6:
      return "AF_INET6";
    case AF_UNSPEC:
      return "AF_UNSPEC";
    default:
      return "";
  }
}

} // namespace proxygen
