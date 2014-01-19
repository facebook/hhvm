/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
// #define __STDC_FORMAT_MACROS

#include "thrift/lib/cpp/transport/TSocketAddress.h"

#include "thrift/lib/cpp/transport/TTransportException.h"

#include <boost/functional/hash.hpp>
#include <boost/static_assert.hpp>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <string>

namespace {

/**
 * A structure to free a struct addrinfo when it goes out of scope.
 */
struct ScopedAddrInfo {
  explicit ScopedAddrInfo(struct addrinfo* info) : info(info) {}
  ~ScopedAddrInfo() {
    freeaddrinfo(info);
  }

  struct addrinfo* info;
};

/**
 * A simple data structure for parsing a host-and-port string.
 *
 * Accepts a string of the form "<host>:<port>" or just "<port>",
 * and contains two string pointers to the host and the port portion of the
 * string.
 *
 * The HostAndPort may contain pointers into the original string.  It is
 * responsible for the user to ensure that the input string is valid for the
 * lifetime of the HostAndPort structure.
 */
struct HostAndPort {
  HostAndPort(const char* str, bool hostRequired)
    : host(nullptr),
      port(nullptr),
      allocated(nullptr) {
    using apache::thrift::transport::TTransportException;

    // Look for the last colon
    const char* colon = strrchr(str, ':');
    if (colon == nullptr) {
      // No colon, just a port number.
      if (hostRequired) {
        throw TTransportException(TTransportException::INTERNAL_ERROR,
                                  "expected a host and port string of the "
                                  "form \"<host>:<port>\"");
      }
      port = str;
      return;
    }

    // We have to make a copy of the string so we can modify it
    // and change the colon to a NUL terminator.
    allocated = strdup(str);
    if (!allocated) {
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "out of memory: strdup() failed parsing host "
                                "and port string");
    }

    char *allocatedColon = allocated + (colon - str);
    *allocatedColon = '\0';
    host = allocated;
    port = allocatedColon + 1;
    // bracketed IPv6 address, remove the brackets
    // allocatedColon[-1] is fine, as allocatedColon >= host and
    // *allocatedColon != *host therefore allocatedColon > host
    if (*host == '[' && allocatedColon[-1] == ']') {
      allocatedColon[-1] = '\0';
      ++host;
    }
  }

  ~HostAndPort() {
    free(allocated);
  }

  const char* host;
  const char* port;
  char* allocated;
};

bool isPrivateIPv4Address(uint32_t addr) {
  // IPv4 private network prefix.
  const static uint32_t prefixes[] = {0x0A000000,  // 10/8 prefix
                                      0x7F000000,  // 127/8 prefix
                                      0xAC100000,  // 172.16/12 prefix
                                      0xC0A80000}; // 192.168/16 prefix
  const static uint32_t masks[] =    {0xFF000000,  // 10/8 mask
                                      0xFF000000,  // 127/8 mask
                                      0xFFF00000,  // 172.16/12 mask
                                      0xFFFF0000}; // 192.168/16 mask

  uint32_t hostOrder = ntohl(addr);
  for (int i = 0; i < sizeof masks / sizeof masks[0]; i++) {
    if ((hostOrder & masks[i]) == prefixes[i])
      return true;
  }

  return false;
}

inline bool isLoopbackIPv4Address(uint32_t addr) {
  return (ntohl(addr) & 0xff000000) == 0x7f000000;
}

inline uint32_t getIPv4FromMapped(const sockaddr_in6* addr6) {
  assert(IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr));
  // With _GNU_SOURCE we could use sin6_addr.s6_addr32, but perform the
  // cast ourselves so we don't have to rely on glibc-specific code.
  const uint32_t* addr_u32 =
    reinterpret_cast<const uint32_t*>(addr6->sin6_addr.s6_addr);
  return addr_u32[3];
}

} // unnamed namespace

namespace apache { namespace thrift { namespace transport {

bool TSocketAddress::isPrivateAddress() const {
  if (storage_.addr.sa_family == AF_INET) {
    return isPrivateIPv4Address(storage_.ipv4.sin_addr.s_addr);
  } else if (storage_.addr.sa_family == AF_INET6) {
    // There is an IN6_IS_ADDR_SITELOCAL() macro for the deprecated
    // "site local" prefix, but unfortunately no macro for unique local
    // addresses.
    if ((storage_.ipv6.sin6_addr.s6_addr[0] & 0xfe) == 0xfc) {
      // fc00::/7 addresses are IPv6 Unique Local Addresses (RFC4193)
      return true;
    }
    if (IN6_IS_ADDR_LINKLOCAL(&storage_.ipv6.sin6_addr)) {
      // fe80::/10 addresses are link-local
      return true;
    }
    // Check for IPv4-mapped addresses
    if (IN6_IS_ADDR_V4MAPPED(&storage_.ipv6.sin6_addr)) {
      return isPrivateIPv4Address(getIPv4FromMapped(&storage_.ipv6));
    }
  } else if (storage_.addr.sa_family == AF_UNIX) {
    // Unix addresses are always local to a host.  Return true,
    // since this conforms to the semantics of returning true for IP loopback
    // addresses.
    return true;
  }
  return false;
}

bool TSocketAddress::isLoopbackAddress() const {
  if (storage_.addr.sa_family == AF_INET) {
    return isLoopbackIPv4Address(storage_.ipv4.sin_addr.s_addr);
  } else if (storage_.addr.sa_family == AF_INET6) {
    if (IN6_IS_ADDR_LOOPBACK(&storage_.ipv6.sin6_addr)) {
      return true;
    }
    // Check for IPv4-mapped addresses
    if (IN6_IS_ADDR_V4MAPPED(&storage_.ipv6.sin6_addr)) {
      return isLoopbackIPv4Address(getIPv4FromMapped(&storage_.ipv6));
    }
  } else if (storage_.addr.sa_family == AF_UNIX) {
    // Return true for UNIX addresses, since they are always local to a host.
    return true;
  }
  return false;
}

void TSocketAddress::setFromHostPort(const char* host, uint16_t port) {
  ScopedAddrInfo results(getAddrInfo(host, port, 0));
  setFromAddrInfo(results.info);
}

void TSocketAddress::setFromIpPort(const char* ip, uint16_t port) {
  ScopedAddrInfo results(getAddrInfo(ip, port, AI_NUMERICHOST));
  setFromAddrInfo(results.info);
}

void TSocketAddress::setFromLocalPort(uint16_t port) {
  ScopedAddrInfo results(getAddrInfo(nullptr, port, AI_ADDRCONFIG));
  setFromLocalAddr(results.info);
}

void TSocketAddress::setFromLocalPort(const char* port) {
  ScopedAddrInfo results(getAddrInfo(nullptr, port, AI_ADDRCONFIG));
  setFromLocalAddr(results.info);
}

void TSocketAddress::setFromLocalIpPort(const char* addressAndPort) {
  HostAndPort hp(addressAndPort, false);
  ScopedAddrInfo results(getAddrInfo(hp.host, hp.port,
                                     AI_NUMERICHOST | AI_ADDRCONFIG));
  setFromLocalAddr(results.info);
}

void TSocketAddress::setFromIpPort(const char* addressAndPort) {
  HostAndPort hp(addressAndPort, true);
  ScopedAddrInfo results(getAddrInfo(hp.host, hp.port, AI_NUMERICHOST));
  setFromAddrInfo(results.info);
}

void TSocketAddress::setFromHostPort(const char* hostAndPort) {
  HostAndPort hp(hostAndPort, true);
  ScopedAddrInfo results(getAddrInfo(hp.host, hp.port, 0));
  setFromAddrInfo(results.info);
}

void TSocketAddress::setFromPath(const char* path, size_t len) {
  if (storage_.addr.sa_family != AF_UNIX) {
    storage_.un.init();
  }

  storage_.un.len = offsetof(struct sockaddr_un, sun_path) + len;
  if (len > sizeof(storage_.un.addr->sun_path)) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "socket path too large to fit into sockaddr_un");
  } else if (len == sizeof(storage_.un.addr->sun_path)) {
    // Note that there will be no terminating NUL in this case.
    // We allow this since getsockname() and getpeername() may return
    // Unix socket addresses with paths that fit exactly in sun_path with no
    // terminating NUL.
    memcpy(storage_.un.addr->sun_path, path, len);
  } else {
    memcpy(storage_.un.addr->sun_path, path, len + 1);
  }
}

void TSocketAddress::setFromPeerAddress(int socket) {
  int errnum = setFromSocket(socket, getpeername);
  if (errnum != 0) {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "getpeername() failed", errnum);
  }
}

void TSocketAddress::setFromLocalAddress(int socket) {
  int errnum = setFromSocket(socket, getsockname);
  if (errnum != 0) {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "getsockname() failed", errnum);
  }
}

void TSocketAddress::setFromSockaddr(const struct sockaddr* address) {
  if (address->sa_family == AF_INET) {
    setFromSockaddr(reinterpret_cast<const struct sockaddr_in*>(address));
  } else if (address->sa_family == AF_INET6) {
    setFromSockaddr(reinterpret_cast<const struct sockaddr_in6*>(address));
  } else if (address->sa_family == AF_UNIX) {
    // We need an explicitly specified length for AF_UNIX addresses,
    // to be able to distinguish anonymous addresses from addresses
    // in Linux's abstract namespace.
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "TSocketAddress::setFromSockaddr(): the address "
                              "length must be explicitly specified when "
                              "setting AF_UNIX addresses");
  } else {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "TSocketAddress::setFromSockaddr() called "
                              "with unsupported address type");
  }
}

void TSocketAddress::setFromSockaddr(const struct sockaddr* address,
                                     socklen_t addrlen) {
  // Check the length to make sure we can access address->sa_family
  if (addrlen < (offsetof(struct sockaddr, sa_family) +
                 sizeof(address->sa_family))) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "TSocketAddress::setFromSockaddr() called "
                              "with length too short for a sockaddr");
  }

  if (address->sa_family == AF_INET) {
    if (addrlen < sizeof(struct sockaddr_in)) {
      throw TTransportException(TTransportException::BAD_ARGS,
                                "TSocketAddress::setFromSockaddr() called "
                                "with length too short for a sockaddr_in");
    }
    setFromSockaddr(reinterpret_cast<const struct sockaddr_in*>(address));
  } else if (address->sa_family == AF_INET6) {
    if (addrlen < sizeof(struct sockaddr_in6)) {
      throw TTransportException(TTransportException::BAD_ARGS,
                                "TSocketAddress::setFromSockaddr() called "
                                "with length too short for a sockaddr_in6");
    }
    setFromSockaddr(reinterpret_cast<const struct sockaddr_in6*>(address));
  } else if (address->sa_family == AF_UNIX) {
    setFromSockaddr(reinterpret_cast<const struct sockaddr_un*>(address),
                    addrlen);
  } else {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "TSocketAddress::setFromSockaddr() called "
                              "with unsupported address type");
  }
}

void TSocketAddress::setFromSockaddr(const struct sockaddr_in* address) {
  assert(address->sin_family == AF_INET);
  prepFamilyChange(AF_INET);
  storage_.ipv4 = *address;
}

void TSocketAddress::setFromSockaddr(const struct sockaddr_in6* address) {
  assert(address->sin6_family == AF_INET6);
  prepFamilyChange(AF_INET6);
  storage_.ipv6 = *address;
}

void TSocketAddress::setFromSockaddr(const struct sockaddr_un* address,
                                     socklen_t addrlen) {
  assert(address->sun_family == AF_UNIX);
  if (addrlen > sizeof(struct sockaddr_un)) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "TSocketAddress::setFromSockaddr() called "
                              "with length too long for a sockaddr_un");
  }

  prepFamilyChange(AF_UNIX);
  memcpy(storage_.un.addr, address, addrlen);
  updateUnixAddressLength(addrlen);

  // Fill the rest with 0s, just for safety
  if (addrlen < sizeof(struct sockaddr_un)) {
    char *p = reinterpret_cast<char*>(storage_.un.addr);
    memset(p + addrlen, 0, sizeof(struct sockaddr_un) - addrlen);
  }
}

struct sockaddr*
TSocketAddress::getMutableAddress(sa_family_t family,
                                  socklen_t *sizeReturn) {
  if (family != AF_UNIX) {
    if (getFamily() == AF_UNIX) {
      storage_.un.free();
    }
    // Set sa_family to the expected new family already.
    // This way if the caller calls addressUpdated() without modifying the
    // address it won't fail due to an unexpected family.
    // This can happen for example if the caller calls accept() and accept
    // fails.  accept() won't modify the address in this case.
    storage_.addr.sa_family = family;

    *sizeReturn = sizeof(storage_);
    return &storage_.addr;
  } else {
    if (getFamily() != AF_UNIX) {
      storage_.un.init();
    }
    *sizeReturn = sizeof(*storage_.un.addr);
    return reinterpret_cast<struct sockaddr*>(storage_.un.addr);
  }
}

socklen_t TSocketAddress::getActualSize() const {
  switch (storage_.addr.sa_family) {
    case AF_UNSPEC:
      // We return sizeof struct sockaddr here, though only sa_family
      // is guaranteed to be initialized.
      return sizeof(storage_.addr);
    case AF_INET:
      return sizeof(storage_.ipv4);
    case AF_INET6:
      return sizeof(storage_.ipv6);
    case AF_UNIX:
      return storage_.un.len;
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress::getActualSize() called "
                                "with unrecognized address family");
  }
}

std::string TSocketAddress::getAddressStr() const {
  char buf[INET6_ADDRSTRLEN];
  getAddressStr(buf, sizeof(buf));
  return buf;
}

void TSocketAddress::getAddressStr(char* buf, size_t buflen) const {
  if (storage_.addr.sa_family == AF_INET) {
    // this is a hot path, so we use a hand-rolled conversion function
    getAddressStrIPv4Fast(buf, buflen);
  } else {
    // IPv6 is a much more complicated case and also not very common yet
    // so we just call the library function
    getIpString(buf, buflen, NI_NUMERICHOST);
  }
}

void TSocketAddress::getAddressStrIPv4Fast(char* buf, size_t buflen) const {
  assert(buflen >= sizeof("255.255.255.255"));
  const uint8_t* ip =
      reinterpret_cast<const uint8_t*>(&storage_.ipv4.sin_addr);

  int pos = 0;
  for (int k = 0; k < 4; ++k) {
    uint8_t num = ip[k];

    if (num >= 200) {
      buf[pos++] = '2';
      num -= 200;
    } else if (num >= 100) {
      buf[pos++] = '1';
      num -= 100;
    }

    // num < 100
    if (ip[k] >= 10) {
      buf[pos++] = '0' + num / 10;
      buf[pos++] = '0' + num % 10;
    } else {
      buf[pos++] = '0' + num;
    }

    buf[pos++] = '.';
  }
  buf[pos-1] = '\0';
}

uint16_t TSocketAddress::getPort() const {
  switch (storage_.addr.sa_family) {
    case AF_INET:
      return ntohs(storage_.ipv4.sin_port);
    case AF_INET6:
      return ntohs(storage_.ipv6.sin6_port);
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress::getPort() called on non-IP "
                                "address");
  }
}

void TSocketAddress::setPort(uint16_t port) {
  switch (storage_.addr.sa_family) {
    case AF_INET:
      storage_.ipv4.sin_port = htons(port);
      return;
    case AF_INET6:
      storage_.ipv6.sin6_port = htons(port);
      return;
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress::setPort() called on non-IP "
                                "address");
  }
}

void TSocketAddress::convertToIPv4() {
  if (!tryConvertToIPv4()) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "convertToIPv4() called on an addresse that is "
                              "not an IPv4-mapped address");
  }
}

bool TSocketAddress::tryConvertToIPv4() {
  if (!isIPv4Mapped()) {
    return false;
  }

  uint16_t port = storage_.ipv6.sin6_port;
  uint32_t addr = getIPv4FromMapped(&storage_.ipv6);
  storage_.addr.sa_family = AF_INET;
  storage_.ipv4.sin_port = port;
  storage_.ipv4.sin_addr.s_addr = addr;
  return true;
}

std::string TSocketAddress::getHostStr() const {
  return getIpString(0);
}

std::string TSocketAddress::getPath() const {
  if (getFamily() != AF_UNIX) {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "TSocketAddress: attempting to get path "
                              "for a non-Unix address");
  }

  if (storage_.un.pathLength() == 0) {
    // anonymous address
    return std::string();
  }
  if (storage_.un.addr->sun_path[0] == '\0') {
    // abstract namespace
    return std::string(storage_.un.addr->sun_path, storage_.un.pathLength());
  }

  return std::string(storage_.un.addr->sun_path,
                     strnlen(storage_.un.addr->sun_path,
                             storage_.un.pathLength()));
}

std::string TSocketAddress::describe() const {
  switch (storage_.addr.sa_family) {
    case AF_UNSPEC:
      return "<uninitialized address>";
    case AF_INET:
    {
      char buf[NI_MAXHOST + 16];
      getAddressStr(buf, sizeof(buf));
      size_t iplen = strlen(buf);
      snprintf(buf + iplen, sizeof(buf) - iplen, ":%hu", getPort());
      return buf;
    }
    case AF_INET6:
    {
      char buf[NI_MAXHOST + 18];
      buf[0] = '[';
      getAddressStr(buf + 1, sizeof(buf) - 1);
      size_t iplen = strlen(buf);
      snprintf(buf + iplen, sizeof(buf) - iplen, "]:%hu", getPort());
      return buf;
    }
    case AF_UNIX:
    {
      if (storage_.un.pathLength() == 0) {
        return "<anonymous unix address>";
      }

      if (storage_.un.addr->sun_path[0] == '\0') {
        // Linux supports an abstract namespace for unix socket addresses
        return "<abstract unix address>";
      }

      return std::string(storage_.un.addr->sun_path,
                         strnlen(storage_.un.addr->sun_path,
                                 storage_.un.pathLength()));
    }
    default:
    {
      char buf[64];
      snprintf(buf, sizeof(buf), "<unknown address family %d>",
               storage_.addr.sa_family);
      return buf;
    }
  }
}

bool TSocketAddress::operator==(const TSocketAddress& other) const {
  if (other.storage_.addr.sa_family != storage_.addr.sa_family) {
    return false;
  }

  switch (storage_.addr.sa_family) {
    case AF_INET:
      return ((other.storage_.ipv4.sin_addr.s_addr ==
               storage_.ipv4.sin_addr.s_addr) &&
              (other.storage_.ipv4.sin_port == storage_.ipv4.sin_port));
    case AF_INET6:
      // We don't check sin6_flowinfo
      if (other.storage_.ipv6.sin6_port != storage_.ipv6.sin6_port) {
        return false;
      }
      if (other.storage_.ipv6.sin6_scope_id != storage_.ipv6.sin6_scope_id) {
        return false;
      }
      return memcmp(other.storage_.ipv6.sin6_addr.s6_addr,
                    storage_.ipv6.sin6_addr.s6_addr,
                    sizeof(storage_.ipv6.sin6_addr.s6_addr)) == 0;
    case AF_UNIX:
    {
      // anonymous addresses are never equal to any other addresses
      if (storage_.un.pathLength() == 0 ||
          other.storage_.un.pathLength() == 0) {
        return false;
      }

      if (storage_.un.len != other.storage_.un.len) {
        return false;
      }
      int cmp = memcmp(storage_.un.addr->sun_path,
                       other.storage_.un.addr->sun_path,
                       storage_.un.pathLength());
      return cmp == 0;
    }
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress: unsupported address family "
                                "for comparison");
  }
}

bool TSocketAddress::prefixMatch(const TSocketAddress& other,
    unsigned prefixLength) const {
  if (other.storage_.addr.sa_family != storage_.addr.sa_family) {
    return false;
  }
  switch (storage_.addr.sa_family) {
    case AF_INET:
    {
      uint64_t mask = ~(uint64_t(0)) << (32 - prefixLength);
      uint32_t addr1 = storage_.ipv4.sin_addr.s_addr;
      uint32_t addr2 = other.storage_.ipv4.sin_addr.s_addr;
      return (ntohl(addr1 ^ addr2) & mask) == 0;
    }
    case AF_INET6:
    {
      const uint32_t* addr1 = storage_.ipv6.sin6_addr.s6_addr32;
      const uint32_t* addr2 = other.storage_.ipv6.sin6_addr.s6_addr32;
      for (unsigned i = 0; i < 4; i++, addr1++, addr2++) {
        if (prefixLength >= 32) {
          if (*addr1 != *addr2) {
            return false;
          }
        } else if (prefixLength == 0) {
          return true;
        } else {
          uint64_t mask = ~(uint64_t(0)) << (32 - prefixLength);
          return (ntohl(*addr1 ^ *addr2) & mask) == 0;
        }
        prefixLength -= 32;
      }
      return true;
    }
    default:
      return false;
  }
}

size_t TSocketAddress::hash() const {
  size_t seed = storage_.addr.sa_family;

  switch (storage_.addr.sa_family) {
    case AF_INET:
      boost::hash_combine(seed, storage_.ipv4.sin_port);
      boost::hash_combine(seed, storage_.ipv4.sin_addr.s_addr);
      break;
    case AF_INET6: {
      boost::hash_combine(seed, storage_.ipv6.sin6_port);
      // The IPv6 address is 16 bytes long.
      // Combine it in blocks of sizeof(size_t) bytes each.
      BOOST_STATIC_ASSERT(sizeof(struct in6_addr) % sizeof(size_t) == 0);
      const size_t* p =
        reinterpret_cast<const size_t*>(storage_.ipv6.sin6_addr.s6_addr);
      for (int amtHashed = 0;
           amtHashed < sizeof(struct in6_addr);
           amtHashed += sizeof(*p), ++p) {
        boost::hash_combine(seed, *p);
      }
      boost::hash_combine(seed, storage_.ipv6.sin6_scope_id);
      break;
    }
    case AF_UNIX:
    {
      enum { kUnixPathMax = sizeof(storage_.un.addr->sun_path) };
      const char *path = storage_.un.addr->sun_path;
      size_t pathLength = storage_.un.pathLength();
      // TODO: this probably could be made more efficient
      for (unsigned int n = 0; n < pathLength; ++n) {
        boost::hash_combine(seed, path[n]);
      }
      break;
    }
    case AF_UNSPEC:
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress: unsupported address family "
                                "for hashing");
  }

  return seed;
}

struct addrinfo* TSocketAddress::getAddrInfo(const char* host,
                                             uint16_t port,
                                             int flags) {
  // getaddrinfo() requires the port number as a string
  char portString[sizeof("65535")];
  snprintf(portString, sizeof(portString), "%hu", port);

  return getAddrInfo(host, portString, flags);
}

struct addrinfo* TSocketAddress::getAddrInfo(const char* host,
                                             const char* port,
                                             int flags) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | flags;

  struct addrinfo *results;
  int error = getaddrinfo(host, port, &hints, &results);
  if (error != 0) {
    std::ostringstream os;
    os << "Failed to resolve address for \"" << host << "\": " <<
      gai_strerror(error) << " (error=" << error << ")";
    throw TTransportException(TTransportException::INTERNAL_ERROR, os.str());
  }

  return results;
}

void TSocketAddress::setFromAddrInfo(const struct addrinfo* info) {
  setFromSockaddr(info->ai_addr, info->ai_addrlen);
}

void TSocketAddress::setFromLocalAddr(const struct addrinfo* info) {
  // If an IPv6 address is present, prefer to use it, since IPv4 addresses
  // can be mapped into IPv6 space.
  for (const struct addrinfo* ai = info; ai != nullptr; ai = ai->ai_next) {
    if (ai->ai_family == AF_INET6) {
      setFromSockaddr(ai->ai_addr, ai->ai_addrlen);
      return;
    }
  }

  // Otherwise, just use the first address in the list.
  setFromSockaddr(info->ai_addr, info->ai_addrlen);
}

int TSocketAddress::setFromSocket(int socket,
                                  int (*fn)(int, sockaddr*, socklen_t*)) {
  // If this was previously an AF_UNIX socket, free the external buffer.
  // TODO: It would be smarter to just remember the external buffer, and then
  // re-use it or free it depending on if the new address is also a unix
  // socket.
  if (getFamily() == AF_UNIX) {
    storage_.un.free();
  }

  // Optimistically try to put the address into the local storage buffer.
  socklen_t addrLen = sizeof(storage_);
  if (fn(socket, &storage_.addr, &addrLen) != 0) {
    return errno;
  }

  // If this was a unix domain socket, we need to store the address in an
  // external buffer.  The first call to fn() used a buffer that was too small,
  // so the path name returned may have been truncated.
  if (storage_.addr.sa_family == AF_UNIX) {
    storage_.un.init();
    addrLen = sizeof(*storage_.un.addr);
    if (fn(socket, reinterpret_cast<struct sockaddr*>(storage_.un.addr),
           &addrLen) != 0) {
      storage_.un.free();
      return errno;
    }
    updateUnixAddressLength(addrLen);
    assert(storage_.addr.sa_family == AF_UNIX);
  }

  return 0;
}

std::string TSocketAddress::getIpString(int flags) const {
  char addrString[NI_MAXHOST];
  getIpString(addrString, sizeof(addrString), flags);
  return std::string(addrString);
}

void TSocketAddress::getIpString(char *buf, size_t buflen, int flags) const {
  if (storage_.addr.sa_family != AF_INET &&
      storage_.addr.sa_family != AF_INET6) {
    throw TTransportException(TTransportException::INTERNAL_ERROR,
                              "TSocketAddress: attempting to get IP address "
                              "for a non-IP address");
  }

  int rc = getnameinfo(&storage_.addr, sizeof(storage_),
                       buf, buflen, nullptr, 0, flags);
  if (rc != 0) {
    std::ostringstream os;
    os << "getnameinfo() failed in getIpString() error = " <<
      gai_strerror(rc);
    throw TTransportException(TTransportException::INTERNAL_ERROR, os.str());
  }
}

void TSocketAddress::addressUpdateFailure(sa_family_t expectedFamily) {
  if (expectedFamily == AF_UNIX) {
    // Free the external address space allocated for this address
    storage_.un.free();
  }
  if (storage_.addr.sa_family == AF_UNIX) {
    // Set the family to AF_UNSPEC, since there is no external storage
    // allocated for this address
    storage_.addr.sa_family = AF_UNSPEC;
  }

  throw TTransportException(TTransportException::INVALID_STATE,
                            "TSocketAddress update illegally changed "
                            "address families");
}

void TSocketAddress::updateUnixAddressLength(socklen_t addrlen) {
  if (addrlen < offsetof(struct sockaddr_un, sun_path)) {
    throw TTransportException(TTransportException::BAD_ARGS,
                              "TSocketAddress: attempted to set a Unix socket "
                              "with a length too short for a sockaddr_un");
  }

  storage_.un.len = addrlen;
  if (storage_.un.pathLength() == 0) {
    // anonymous address
    return;
  }

  if (storage_.un.addr->sun_path[0] == '\0') {
    // abstract namespace.  honor the specified length
  } else {
    // Call strnlen(), just in case the length was overspecified.
    socklen_t maxLength = addrlen - offsetof(struct sockaddr_un, sun_path);
    size_t pathLength = strnlen(storage_.un.addr->sun_path, maxLength);
    storage_.un.len = offsetof(struct sockaddr_un, sun_path) + pathLength;
  }
}

bool TSocketAddress::operator<(const TSocketAddress& other) const {
  if (storage_.addr.sa_family != other.storage_.addr.sa_family) {
    return storage_.addr.sa_family < other.storage_.addr.sa_family;
  }

  switch (storage_.addr.sa_family) {
    case AF_INET:
      if (storage_.ipv4.sin_port != other.storage_.ipv4.sin_port) {
        return storage_.ipv4.sin_port < other.storage_.ipv4.sin_port;
      }

      return
        storage_.ipv4.sin_addr.s_addr < other.storage_.ipv4.sin_addr.s_addr;
    case AF_INET6: {
      if (storage_.ipv6.sin6_port != other.storage_.ipv6.sin6_port) {
        return storage_.ipv6.sin6_port < other.storage_.ipv6.sin6_port;
      }

      const void *p1 = reinterpret_cast<const void*>(
        storage_.ipv6.sin6_addr.s6_addr);
      const void *p2 = reinterpret_cast<const void*>(
        other.storage_.ipv6.sin6_addr.s6_addr);

      return memcmp(p1, p2, sizeof(struct in6_addr)) < 0;
    }
    case AF_UNIX: {
      // Anonymous addresses can't be compared to anything else.
      // Return that they are never less than anything.
      //
      // Note that this still meets the requirements for a strict weak
      // ordering, so we can use this operator<() with standard C++ containers.
      size_t thisPathLength = storage_.un.pathLength();
      if (thisPathLength == 0) {
        return false;
      }
      size_t otherPathLength = other.storage_.un.pathLength();
      if (otherPathLength == 0) {
        return true;
      }

      // Compare based on path length first, for efficiency
      if (thisPathLength != otherPathLength) {
        return thisPathLength < otherPathLength;
      }
      int cmp = memcmp(storage_.un.addr->sun_path,
                       other.storage_.un.addr->sun_path,
                       thisPathLength);
      return cmp < 0;
    }
    case AF_UNSPEC:
    default:
      throw TTransportException(TTransportException::INTERNAL_ERROR,
                                "TSocketAddress: unsupported address family "
                                "for comparing");
  }
}

size_t hash_value(const TSocketAddress& address) {
  return address.hash();
}

std::ostream& operator<<(std::ostream& os, const TSocketAddress& addr) {
  os << addr.describe();
  return os;
}

}}} // apache::thrift::transport
