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
#include "hphp/runtime/ext/sockets/ext_sockets.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/uio.h>
#include <poll.h>

#include <folly/String.h>
#include <folly/SocketAddress.h>

#include "hphp/util/network.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/ssl-socket.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/zend-php-config.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/util/logger.h"

#define PHP_NORMAL_READ 0x0001
#define PHP_BINARY_READ 0x0002

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

static void check_socket_parameters(int &domain, int &type) {
  if (domain != AF_UNIX && domain != AF_INET6 && domain != AF_INET) {
    raise_warning("invalid socket domain [%d] specified for argument 1, "
                    "assuming AF_INET", domain);
    domain = AF_INET;
  }

  if (type > 10) {
    raise_warning("invalid socket type [%d] specified for argument 2, "
                    "assuming SOCK_STREAM", type);
    type = SOCK_STREAM;
  }
}

const StaticString
  s_2colons("::"),
  s_0_0_0_0("0.0.0.0");

static bool get_sockaddr(sockaddr *sa, socklen_t salen,
                         Variant &address, Variant &port) {
  switch (sa->sa_family) {
  case AF_INET6:
    {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
      try {
        folly::SocketAddress addr;
        addr.setFromSockaddr(sin6);

        address = String(addr.getAddressStr(), CopyString);
        port = addr.getPort();
      } catch (...) {
        address = s_2colons;
        port = 0;
      }
    }
    return true;
  case AF_INET:
    {
      struct sockaddr_in *sin = (struct sockaddr_in *)sa;
      try {
        folly::SocketAddress addr;
        addr.setFromSockaddr(sin);

        address = String(addr.getAddressStr(), CopyString);
        port = addr.getPort();
      } catch (...) {
        address = s_0_0_0_0;
        port = 0;
      }
    }
    return true;
  case AF_UNIX:
    {
      // NB: an unnamed socket has no path, and sun_path should not be
      // inspected. In that case the length is just the size of the
      // struct without sun_path.
      struct sockaddr_un *s_un = (struct sockaddr_un *)sa;
      if (salen > offsetof(sockaddr_un, sun_path)) {
        address = String(s_un->sun_path, CopyString);
      }
    }
    return true;

  default:
    break;
  }

  raise_warning("Unsupported address family %d", sa->sa_family);
  return false;
}

static bool php_set_inet6_addr(struct sockaddr_in6 *sin6,
                               const char *address,
                               req::ptr<Socket> sock) {
  struct in6_addr tmp;
  struct addrinfo hints;
  struct addrinfo *addrinfo = NULL;

  if (inet_pton(AF_INET6, address, &tmp)) {
    memcpy(&(sin6->sin6_addr.s6_addr), &(tmp.s6_addr),
           sizeof(struct in6_addr));
  } else {
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET6;
    getaddrinfo(address, NULL, &hints, &addrinfo);
    if (!addrinfo) {
      // 10000 is a magic value to indicate a host error.
      SOCKET_ERROR(sock, "Host lookup failed", (-10000 - h_errno));
      return false;
    }
    if (addrinfo->ai_family != PF_INET6 ||
        addrinfo->ai_addrlen != sizeof(struct sockaddr_in6)) {
      raise_warning("Host lookup failed: Non AF_INET6 domain "
                      "returned on AF_INET6 socket");
      freeaddrinfo(addrinfo);
      return false;
    }

    memcpy(&(sin6->sin6_addr.s6_addr),
           ((struct sockaddr_in6*)(addrinfo->ai_addr))->sin6_addr.s6_addr,
           sizeof(struct in6_addr));
    freeaddrinfo(addrinfo);
  }

  return true;
}

static bool php_set_inet_addr(struct sockaddr_in *sin,
                              const char *address,
                              req::ptr<Socket> sock) {
  struct in_addr tmp;

  if (inet_aton(address, &tmp)) {
    sin->sin_addr.s_addr = tmp.s_addr;
  } else {
    HostEnt result;
    if (!safe_gethostbyname(address, result)) {
      /* Note: < -10000 indicates a host lookup error */
      SOCKET_ERROR(sock, "Host lookup failed", (-10000 - result.herr));
      return false;
    }
    if (result.hostbuf.h_addrtype != AF_INET) {
      raise_warning("Host lookup failed: Non AF_INET domain "
                      "returned on AF_INET socket");
      return false;
    }
    memcpy(&(sin->sin_addr.s_addr), result.hostbuf.h_addr_list[0],
           result.hostbuf.h_length);
  }

  return true;
}

static bool set_sockaddr(sockaddr_storage &sa_storage, req::ptr<Socket> sock,
                         const char *addr, int port,
                         struct sockaddr *&sa_ptr, size_t &sa_size) {
  struct sockaddr *sock_type = (struct sockaddr*) &sa_storage;
  switch (sock->getType()) {
  case AF_UNIX:
    {
      struct sockaddr_un *sa = (struct sockaddr_un *)sock_type;
      memset(sa, 0, sizeof(sa_storage));
      sa->sun_family = AF_UNIX;
      snprintf(sa->sun_path, 108, "%s", addr);
      sa_ptr = (struct sockaddr *)sa;
      sa_size = SUN_LEN(sa);
    }
    break;
  case AF_INET:
    {
      struct sockaddr_in *sa = (struct sockaddr_in *)sock_type;
      memset(sa, 0, sizeof(sa_storage)); /* Apparently, Mac OSX needs this */
      sa->sin_family = AF_INET;
      sa->sin_port = htons((unsigned short) port);
      if (!php_set_inet_addr(sa, addr, sock)) {
        return false;
      }
      sa_ptr = (struct sockaddr *)sa;
      sa_size = sizeof(struct sockaddr_in);
    }
    break;
  case AF_INET6:
    {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)sock_type;
      memset(sa, 0, sizeof(sa_storage)); /* Apparently, Mac OSX needs this */
      sa->sin6_family = AF_INET6;
      sa->sin6_port = htons((unsigned short) port);
      if (!php_set_inet6_addr(sa, addr, sock)) {
        return false;
      }
      sa_ptr = (struct sockaddr *)sa;
      sa_size = sizeof(struct sockaddr_in6);
    }
    break;
  default:
    raise_warning("unsupported socket type '%d', must be "
                    "AF_UNIX, AF_INET, or AF_INET6", sock->getType());
    return false;
  }
  return true;
}

static void sock_array_to_fd_set(const Array& sockets, pollfd *fds, int &nfds,
                                 short flag) {
  assert(fds);
  for (ArrayIter iter(sockets); iter; ++iter) {
    auto sock = cast<File>(iter.second());
    int intfd = sock->fd();
    if (intfd < 0) {
      raise_warning(
        "cannot represent a stream of type user-space as a file descriptor"
      );
      continue;
    }
    pollfd &fd = fds[nfds++];
    fd.fd = intfd;
    fd.events = flag;
    fd.revents = 0;
  }
}

static void sock_array_from_fd_set(Variant &sockets, pollfd *fds, int &nfds,
                                   int &count, short flag) {
  assert(sockets.is(KindOfArray));
  Array sock_array = sockets.toArray();
  Array ret = Array::Create();
  for (ArrayIter iter(sock_array); iter; ++iter) {
    const pollfd &fd = fds[nfds++];
    assert(fd.fd == cast<File>(iter.second())->fd());
    if (fd.revents & flag) {
      ret.set(iter.first(), iter.second());
      count++;
    }
  }
  sockets = ret;
}

static int php_read(req::ptr<Socket> sock, void *buf, int maxlen, int flags) {
  int m = fcntl(sock->fd(), F_GETFL);
  if (m < 0) {
    return m;
  }
  int nonblock = (m & O_NONBLOCK);
  m = 0;

  char *t = (char *)buf;
  *t = '\0';
  int n = 0;
  int no_read = 0;
  while (*t != '\n' && *t != '\r' && n < maxlen) {
    if (m > 0) {
      t++;
      n++;
    } else if (m == 0) {
      no_read++;
      if (nonblock && no_read >= 2) {
        return n;
        /* The first pass, m always is 0, so no_read becomes 1
         * in the first pass. no_read becomes 2 in the second pass,
         * and if this is nonblocking, we should return.. */
      }

      if (no_read > 200) {
        errno = ECONNRESET;
        return -1;
      }
    }

    if (n < maxlen) {
      m = recv(sock->fd(), (void *)t, 1, flags);
    }

    if (errno != 0 && errno != ESPIPE && errno != EAGAIN) {
      return -1;
    }
    errno = 0;
  }

  if (n < maxlen) {
    n++;
    /* The only reasons it makes it to here is
     * if '\n' or '\r' are encountered. So, increase
     * the return by 1 to make up for the lack of the
     * '\n' or '\r' in the count (since read() takes
     * place at the end of the loop..) */
  }

  return n;
}

static req::ptr<Socket> create_new_socket(
  const HostURL &hosturl,
  Variant &errnum,
  Variant &errstr
) {
  int domain = hosturl.isIPv6() ? AF_INET6 : AF_INET;
  int type = SOCK_STREAM;
  const std::string scheme = hosturl.getScheme();

  if (scheme == "udp" || scheme == "udg") {
    type = SOCK_DGRAM;
  } else if (scheme == "unix") {
    domain = AF_UNIX;
  }

  auto sock = req::make<Socket>(
    socket(domain, type, 0),
    domain,
    hosturl.getHost().c_str(),
    hosturl.getPort()
  );

  if (!sock->valid()) {
    SOCKET_ERROR(sock, "unable to create socket", errno);
    errnum = sock->getError();
    errstr = HHVM_FN(socket_strerror)(sock->getError());
    sock.reset();
  }
  return sock;
}

static int connect_with_timeout(int fd, struct sockaddr *sa_ptr,
                                size_t sa_size, double timeout,
                                const HostURL &hosturl,
                                std::string &errstr, int &errnum) {
  if (timeout <= 0) {
    int retval = connect(fd, sa_ptr, sa_size);
    if (retval < 0) {
      errstr = "unable to connect to " + hosturl.getHostURL();
      errnum = errno;
    }
    return retval;
  }

  // set non-blocking so we can do timeouts
  long arg = fcntl(fd, F_GETFL, nullptr);
  fcntl(fd, F_SETFL, arg | O_NONBLOCK);

  int retval = connect(fd, sa_ptr, sa_size);
  if (retval < 0) {
    if (errno == EINPROGRESS) {
      struct pollfd fds[1];
      fds[0].fd = fd;
      fds[0].events = POLLOUT;
      if (poll(fds, 1, (int)(timeout * 1000))) {
        int valopt;
        socklen_t lon = sizeof(int);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
        if (valopt) {
          errstr = "failed to connect to " + hosturl.getHostURL();
          errnum = valopt;
        }
        retval = valopt ? -1 : 0;
      } else {
        errstr = "timed out after ";
        errstr += folly::to<std::string>(timeout);
        errstr += " seconds when connecting to " + hosturl.getHostURL();
        errnum = ETIMEDOUT;
        retval = -1;
      }
    } else {
      errstr = "unable to connect to " + hosturl.getHostURL();
      errnum = errno;
    }
  }

  // set to blocking mode
  arg = fcntl(fd, F_GETFL, nullptr);
  fcntl(fd, F_SETFL, arg & ~O_NONBLOCK);

  return retval;
}

static Variant new_socket_connect(const HostURL &hosturl, double timeout,
                                  const req::ptr<StreamContext>& streamctx,
                                  Variant &errnum, Variant &errstr) {
  int domain = AF_UNSPEC;
  int type = SOCK_STREAM;
  auto const& scheme = hosturl.getScheme();
  req::ptr<Socket> sock;
  req::ptr<SSLSocket> sslsock;
  std::string sockerr;
  int error;

  if (scheme == "udp" || scheme == "udg") {
    type = SOCK_DGRAM;
  } else if (scheme == "unix") {
    domain = AF_UNIX;
  }

  int fd = -1;
  if (domain == AF_UNIX) {
    sockaddr_storage sa_storage;
    struct sockaddr *sa_ptr;
    size_t sa_size;

    fd = socket(domain, type, 0);
    sock = req::make<Socket>(
      fd, domain, hosturl.getHost().c_str(), hosturl.getPort());

    if (!set_sockaddr(sa_storage, sock, hosturl.getHost().c_str(),
                      hosturl.getPort(), sa_ptr, sa_size)) {
      // set_sockaddr raises its own warning on failure
      return false;
    }
    if (connect_with_timeout(fd, sa_ptr, sa_size, timeout,
                             hosturl, sockerr, error) != 0) {
      SOCKET_ERROR(sock, sockerr.c_str(), error);
      errnum = sock->getLastError();
      errstr = HHVM_FN(socket_strerror)(sock->getLastError());
      return false;
    }
  } else {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = domain;
    hints.ai_socktype = type;

    auto port = folly::to<std::string>(hosturl.getPort());
    auto host = hosturl.getHost();

    struct addrinfo *aiHead;
    int errcode = getaddrinfo(host.c_str(), port.c_str(), &hints, &aiHead);
    if (errcode != 0) {
      errstr = String(gai_strerror(errcode), CopyString);
      return false;
    }
    SCOPE_EXIT { freeaddrinfo(aiHead); };

    for (struct addrinfo *ai = aiHead; ai != nullptr; ai = ai->ai_next) {
      domain = ai->ai_family;
      fd = socket(domain, ai->ai_socktype, ai->ai_protocol);
      if (fd == -1) {
        continue;
      }

      if (connect_with_timeout(fd, ai->ai_addr, ai->ai_addrlen, timeout,
                               hosturl, sockerr, error) == 0) {
        break;
      }
      close(fd);
      fd = -1;
    }

    sslsock = SSLSocket::Create(fd, domain, hosturl, timeout, streamctx);
    if (sslsock) {
      sock = sslsock;
    } else {
      sock = req::make<Socket>(fd,
                                  domain,
                                  hosturl.getHost().c_str(),
                                  hosturl.getPort());
    }
  }

  if (!sock->valid()) {
    SOCKET_ERROR(sock,
        sockerr.empty() ? "unable to create socket" : sockerr.c_str(), error);
    errnum = sock->getLastError();
    errstr = HHVM_FN(socket_strerror)(sock->getLastError());
    return false;
  }

  if (sslsock && !sslsock->onConnect()) {
    raise_warning("Failed to enable crypto");
    return false;
  }

  return Variant(std::move(sock));
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(socket_create,
                      int domain,
                      int type,
                      int protocol) {
  check_socket_parameters(domain, type);
  int socketId = socket(domain, type, protocol);
  if (socketId == -1) {
    SOCKET_ERROR(req::make<Socket>(),
                 "Unable to create socket",
                 errno);
    return false;
  }
  return Variant(req::make<Socket>(socketId, domain));
}

Variant HHVM_FUNCTION(socket_create_listen,
                      int port,
                      int backlog /* = 128 */) {
  HostEnt result;
  if (!safe_gethostbyname("0.0.0.0", result)) {
    return false;
  }

  struct sockaddr_in la;
  memcpy((char *) &la.sin_addr, result.hostbuf.h_addr,
         result.hostbuf.h_length);
  la.sin_family = result.hostbuf.h_addrtype;
  la.sin_port = htons((unsigned short)port);

  auto sock = req::make<Socket>(
    socket(PF_INET, SOCK_STREAM, 0), PF_INET, "0.0.0.0", port);

  if (!sock->valid()) {
    SOCKET_ERROR(sock, "unable to create listening socket", errno);
    return false;
  }

  if (::bind(sock->fd(), (struct sockaddr *)&la, sizeof(la)) < 0) {
    SOCKET_ERROR(sock, "unable to bind to given address", errno);
    return false;
  }

  if (listen(sock->fd(), backlog) < 0) {
    SOCKET_ERROR(sock, "unable to listen on socket", errno);
    return false;
  }

  return Variant(std::move(sock));
}

const StaticString
  s_socktype_generic("generic_socket");

bool HHVM_FUNCTION(socket_create_pair,
                   int domain,
                   int type,
                   int protocol,
                   VRefParam fd) {
  check_socket_parameters(domain, type);

  int fds_array[2];
  if (socketpair(domain, type, protocol, fds_array) != 0) {
    SOCKET_ERROR(req::make<Socket>(),
                 "unable to create socket pair",
                 errno);
    return false;
  }

  fd = make_packed_array(
    Variant(req::make<Socket>(fds_array[0], domain, nullptr, 0, 0.0,
                                 s_socktype_generic)),
    Variant(req::make<Socket>(fds_array[1], domain, nullptr, 0, 0.0,
                                 s_socktype_generic))
  );
  return true;
}

const StaticString
  s_l_onoff("l_onoff"),
  s_l_linger("l_linger"),
  s_sec("sec"),
  s_usec("usec");

Variant HHVM_FUNCTION(socket_get_option,
                      const Resource& socket,
                      int level,
                      int optname) {
  auto sock = cast<Socket>(socket);
  socklen_t optlen;

  switch (optname) {
  case SO_LINGER:
    {
      struct linger linger_val;
      optlen = sizeof(linger_val);
      if (getsockopt(sock->fd(), level, optname, (char*)&linger_val,
                     &optlen) != 0) {
        SOCKET_ERROR(sock, "unable to retrieve socket option", errno);
        return false;
      }

      return make_map_array(
        s_l_onoff, linger_val.l_onoff,
        s_l_linger, linger_val.l_linger
      );
    }
    break;

  case SO_RCVTIMEO:
  case SO_SNDTIMEO:
    {
      struct timeval tv;
      optlen = sizeof(tv);
      if (getsockopt(sock->fd(), level, optname, (char*)&tv, &optlen) != 0) {
        SOCKET_ERROR(sock, "unable to retrieve socket option", errno);
        return false;
      }
      return make_map_array(
        s_sec,  (int)tv.tv_sec,
        s_usec, (int)tv.tv_usec
      );
    }
    break;

  default:
    {
      int other_val;
      optlen = sizeof(other_val);
      if (getsockopt(sock->fd(), level, optname, (char*)&other_val, &optlen)) {
        SOCKET_ERROR(sock, "unable to retrieve socket option", errno);
        return false;
      }
      return other_val;
    }
  }
  not_reached();
}

bool HHVM_FUNCTION(socket_getpeername,
                   const Resource& socket,
                   VRefParam address,
                   VRefParam port /* = null */) {
  auto sock = cast<Socket>(socket);

  sockaddr_storage sa_storage;
  socklen_t salen = sizeof(sockaddr_storage);
  struct sockaddr *sa = (struct sockaddr *)&sa_storage;
  if (getpeername(sock->fd(), sa, &salen) < 0) {
    SOCKET_ERROR(sock, "unable to retrieve peer name", errno);
    return false;
  }
  return get_sockaddr(sa, salen, address, port);
}

bool HHVM_FUNCTION(socket_getsockname,
                   const Resource& socket,
                   VRefParam address,
                   VRefParam port /* = null */) {
  auto sock = cast<Socket>(socket);

  sockaddr_storage sa_storage;
  socklen_t salen = sizeof(sockaddr_storage);
  struct sockaddr *sa = (struct sockaddr *)&sa_storage;
  if (getsockname(sock->fd(), sa, &salen) < 0) {
    SOCKET_ERROR(sock, "unable to retrieve peer name", errno);
    return false;
  }
  return get_sockaddr(sa, salen, address, port);
}

bool HHVM_FUNCTION(socket_set_block,
                   const Resource& socket) {
  return cast<Socket>(socket)->setBlocking(true);
}

bool HHVM_FUNCTION(socket_set_nonblock,
                   const Resource& socket) {
  return cast<Socket>(socket)->setBlocking(false);
}

bool HHVM_FUNCTION(socket_set_option,
                   const Resource& socket,
                   int level,
                   int optname,
                   const Variant& optval) {
  auto sock = cast<Socket>(socket);

  struct linger lv;
  struct timeval tv;
  int ov;
  int optlen;
  void *opt_ptr;

  switch (optname) {
  case SO_LINGER:
    {
      Array value = optval.toArray();
      if (!value.exists(s_l_onoff)) {
        raise_warning("no key \"l_onoff\" passed in optval");
        return false;
      }
      if (!value.exists(s_l_linger)) {
        raise_warning("no key \"l_linger\" passed in optval");
        return false;
      }

      lv.l_onoff = (unsigned short)value[s_l_onoff].toInt32();
      lv.l_linger = (unsigned short)value[s_l_linger].toInt32();
      optlen = sizeof(lv);
      opt_ptr = &lv;
    }
    break;

  case SO_RCVTIMEO:
  case SO_SNDTIMEO:
    {
      Array value = optval.toArray();
      if (!value.exists(s_sec)) {
        raise_warning("no key \"sec\" passed in optval");
        return false;
      }
      if (!value.exists(s_usec)) {
        raise_warning("no key \"usec\" passed in optval");
        return false;
      }

      tv.tv_sec = value[s_sec].toInt32();
      tv.tv_usec = value[s_usec].toInt32();
      if (tv.tv_usec >= 1000000) {
        tv.tv_sec += tv.tv_usec / 1000000;
        tv.tv_usec %= 1000000;
      }
      if (tv.tv_sec < 0) {
        tv.tv_sec = ThreadInfo::s_threadInfo.getNoCheck()->
        m_reqInjectionData.getSocketDefaultTimeout();
      }
      optlen = sizeof(tv);
      opt_ptr = &tv;
      sock->setTimeout(tv);
    }
    break;

  default:
    ov = optval.toInt32();
    optlen = sizeof(ov);
    opt_ptr = &ov;
    break;
  }

  if (setsockopt(sock->fd(), level, optname, opt_ptr, optlen) != 0) {
    SOCKET_ERROR(sock, "unable to set socket option", errno);
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(socket_connect,
                   const Resource& socket,
                   const String& address,
                   int port /* = 0 */) {
  auto sock = cast<Socket>(socket);

  switch (sock->getType()) {
  case AF_INET6:
  case AF_INET:
    if (port == 0) {
      raise_warning("Socket of type AF_INET/6 requires 3 arguments");
      return false;
    }
    break;
  default:
    break;
  }

  const char *addr = address.data();
  sockaddr_storage sa_storage;
  struct sockaddr *sa_ptr;
  size_t sa_size;
  if (!set_sockaddr(sa_storage, sock, addr, port, sa_ptr, sa_size)) {
    return false;
  }

  IOStatusHelper io("socket::connect", address.data(), port);
  int retval = connect(sock->fd(), sa_ptr, sa_size);
  if (retval != 0) {
    std::string msg = "unable to connect to ";
    msg += addr;
    msg += ":";
    msg += folly::to<std::string>(port);
    SOCKET_ERROR(sock, msg.c_str(), errno);
    return false;
  }

  return true;
}

bool HHVM_FUNCTION(socket_bind,
                   const Resource& socket,
                   const String& address,
                   int port /* = 0 */) {
  auto sock = cast<Socket>(socket);

  const char *addr = address.data();
  sockaddr_storage sa_storage;
  struct sockaddr *sa_ptr;
  size_t sa_size;
  if (!set_sockaddr(sa_storage, sock, addr, port, sa_ptr, sa_size)) {
    return false;
  }

  long retval = ::bind(sock->fd(), sa_ptr, sa_size);
  if (retval != 0) {
    std::string msg = "unable to bind address";
    msg += addr;
    msg += ":";
    msg += folly::to<std::string>(port);
    SOCKET_ERROR(sock, msg.c_str(), errno);
    return false;
  }

  return true;
}

bool HHVM_FUNCTION(socket_listen,
                   const Resource& socket,
                   int backlog /* = 0 */) {
  auto sock = cast<Socket>(socket);
  if (listen(sock->fd(), backlog) != 0) {
    SOCKET_ERROR(sock, "unable to listen on socket", errno);
    return false;
  }
  return true;
}

Variant HHVM_FUNCTION(socket_select,
                      VRefParam read,
                      VRefParam write,
                      VRefParam except,
                      const Variant& vtv_sec,
                      int tv_usec /* = 0 */) {
  int count = 0;
  if (!read.isNull()) {
    count += read.toArray().size();
  }
  if (!write.isNull()) {
    count += write.toArray().size();
  }
  if (!except.isNull()) {
    count += except.toArray().size();
  }
  if (!count) {
    return false;
  }

  struct pollfd *fds = (struct pollfd *)calloc(count, sizeof(struct pollfd));
  count = 0;
  if (!read.isNull()) {
    sock_array_to_fd_set(read.toArray(), fds, count, POLLIN);
  }
  if (!write.isNull()) {
    sock_array_to_fd_set(write.toArray(), fds, count, POLLOUT);
  }
  if (!except.isNull()) {
    sock_array_to_fd_set(except.toArray(), fds, count, POLLPRI);
  }
  if (!count) {
    raise_warning("no resource arrays were passed to select");
    free(fds);
    return false;
  }

  IOStatusHelper io("socket_select");
  int timeout_ms = -1;
  if (!vtv_sec.isNull()) {
    timeout_ms = vtv_sec.toInt32() * 1000 + tv_usec / 1000;
  }

  /* slight hack to support buffered data; if there is data sitting in the
   * read buffer of any of the streams in the read array, let's pretend
   * that we selected, but return only the readable sockets */
  if (!read.isNull()) {
    auto hasData = Array::Create();
    for (ArrayIter iter(read.toArray()); iter; ++iter) {
      auto file = cast<File>(iter.second());
      if (file->bufferedLen() > 0) {
        hasData.append(iter.second());
      }
    }
    if (hasData.size() > 0) {
      if (!write.isNull()) {
        write = empty_array();
      }
      if (!except.isNull()) {
        except = empty_array();
      }
      read = hasData;
      free(fds);
      return hasData.size();
    }
  }

  int retval = poll(fds, count, timeout_ms);
  if (retval == -1) {
    raise_warning("unable to select [%d]: %s", errno,
                  folly::errnoStr(errno).c_str());
    free(fds);
    return false;
  }

  count = 0;
  int nfds = 0;
  if (!read.isNull()) {
    sock_array_from_fd_set(read, fds, nfds, count, POLLIN|POLLERR|POLLHUP);
  }
  if (!write.isNull()) {
    sock_array_from_fd_set(write, fds, nfds, count, POLLOUT|POLLERR);
  }
  if (!except.isNull()) {
    sock_array_from_fd_set(except, fds, nfds, count, POLLPRI|POLLERR);
  }

  free(fds);
  return count;
}

Variant HHVM_FUNCTION(socket_server,
                      const String& hostname,
                      int port /* = -1 */,
                      VRefParam errnum /* = null */,
                      VRefParam errstr /* = null */) {
  HostURL hosturl(static_cast<const std::string>(hostname), port);
  return socket_server_impl(hosturl,
                            k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                            errnum, errstr);
}

Variant socket_server_impl(
  const HostURL &hosturl,
  int flags, /* = STREAM_SERVER_BIND|STREAM_SERVER_LISTEN */
  VRefParam errnum /* = null */,
  VRefParam errstr /* = null */
) {
  errnum = 0;
  errstr = empty_string();
  auto sock = create_new_socket(hosturl, errnum, errstr);
  if (!sock) {
    return false;
  }

  sockaddr_storage sa_storage;
  struct sockaddr *sa_ptr;
  size_t sa_size;
  if (!set_sockaddr(sa_storage, sock, hosturl.getHost().c_str(),
                    hosturl.getPort(), sa_ptr, sa_size)) {
    return false;
  }
  int yes = 1;
  setsockopt(sock->fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if ((flags & k_STREAM_SERVER_BIND) != 0 &&
      ::bind(sock->fd(), sa_ptr, sa_size) < 0) {
    SOCKET_ERROR(sock, "unable to bind to given address", errno);
    return false;
  }
  if ((flags & k_STREAM_SERVER_LISTEN) != 0 && listen(sock->fd(), 128) < 0) {
    SOCKET_ERROR(sock, "unable to listen on socket", errno);
    return false;
  }

  return Variant(std::move(sock));
}

Variant HHVM_FUNCTION(socket_accept,
                      const Resource& socket) {
  auto sock = cast<Socket>(socket);
  struct sockaddr sa;
  socklen_t salen = sizeof(sa);
  auto new_sock = req::make<Socket>(
    accept(sock->fd(), &sa, &salen), sock->getType());
  if (!new_sock->valid()) {
    SOCKET_ERROR(new_sock, "unable to accept incoming connection", errno);
    return false;
  }
  return Variant(std::move(new_sock));
}

Variant HHVM_FUNCTION(socket_read,
                      const Resource& socket,
                      int length,
                      int type /* = 0 */) {
  if (length <= 0) {
    return false;
  }
  auto sock = cast<Socket>(socket);

  char *tmpbuf = (char *)malloc(length + 1);
  int retval;
  if (type == PHP_NORMAL_READ) {
    retval = php_read(sock, tmpbuf, length, 0);
  } else {
    retval = recv(sock->fd(), tmpbuf, length, 0);
  }

  if (retval == -1) {
    /* if the socket is in non-blocking mode and there's no data to read,
    don't output any error, as this is a normal situation, and not an error */
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      sock->setError(errno);
    } else {
      SOCKET_ERROR(sock, "unable to read from socket", errno);
    }

    free(tmpbuf);
    return false;
  }

  tmpbuf[retval] = '\0' ;
  return String(tmpbuf, retval, AttachString);
}

Variant HHVM_FUNCTION(socket_write,
                      const Resource& socket,
                      const String& buffer,
                      int length /* = 0 */) {
  auto sock = cast<Socket>(socket);
  if (length == 0 || length > buffer.size()) {
    length = buffer.size();
  }
  int retval = write(sock->fd(), buffer.data(), length);
  if (retval < 0) {
    SOCKET_ERROR(sock, "unable to write to socket", errno);
    return false;
  }
  return retval;
}

Variant HHVM_FUNCTION(socket_send,
                      const Resource& socket,
                      const String& buf,
                      int len,
                      int flags) {
  auto sock = cast<Socket>(socket);
  if (len > buf.size()) {
    len = buf.size();
  }
  int retval = send(sock->fd(), buf.data(), len, flags);
  if (retval == -1) {
    SOCKET_ERROR(sock, "unable to write to socket", errno);
    return false;
  }
  return retval;
}

Variant HHVM_FUNCTION(socket_sendto,
                      const Resource& socket,
                      const String& buf,
                      int len,
                      int flags,
                      const String& addr,
                      int port /* = -1 */) {
  auto sock = cast<Socket>(socket);
  if (len > buf.size()) {
    len = buf.size();
  }
  int retval;
  switch (sock->getType()) {
  case AF_UNIX:
    {
      struct sockaddr_un  s_un;
      memset(&s_un, 0, sizeof(s_un));
      s_un.sun_family = AF_UNIX;
      snprintf(s_un.sun_path, 108, "%s", addr.data());

      retval = sendto(sock->fd(), buf.data(), len, flags,
                      (struct sockaddr *)&s_un, SUN_LEN(&s_un));
    }
    break;
  case AF_INET:
    {
      if (port == -1) {
        throw_missing_arguments_nr("socket_sendto", 6, 5);
        return false;
      }

      struct sockaddr_in  sin;
      memset(&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_port = htons((unsigned short) port);
      if (!php_set_inet_addr(&sin, addr.c_str(), sock)) {
        return false;
      }

      retval = sendto(sock->fd(), buf.data(), len, flags,
                      (struct sockaddr *)&sin, sizeof(sin));
    }
    break;
  case AF_INET6:
    {
      if (port == -1) {
        throw_missing_arguments_nr("socket_sendto", 6, 5);
        return false;
      }

      struct sockaddr_in6  sin6;
      memset(&sin6, 0, sizeof(sin6));
      sin6.sin6_family = AF_INET6;
      sin6.sin6_port = htons((unsigned short) port);

      if (!php_set_inet6_addr(&sin6, addr.c_str(), sock)) {
        return false;
      }

      retval = sendto(sock->fd(), buf.data(), len, flags,
                      (struct sockaddr *)&sin6, sizeof(sin6));
    }
    break;
  default:
    raise_warning("Unsupported socket type %d", sock->getType());
    return false;
  }

  if (retval == -1) {
    SOCKET_ERROR(sock, "unable to write to socket", errno);
    return false;
  }

  return retval;
}

Variant HHVM_FUNCTION(socket_recv,
                      const Resource& socket,
                      VRefParam buf,
                      int len,
                      int flags) {
  if (len <= 0) {
    return false;
  }
  auto sock = cast<Socket>(socket);

  char *recv_buf = (char *)malloc(len + 1);
  int retval;
  if ((retval = recv(sock->fd(), recv_buf, len, flags)) < 1) {
    free(recv_buf);
    buf = uninit_null();
  } else {
    recv_buf[retval] = '\0';
    buf = String(recv_buf, retval, AttachString);
  }

  if (retval == -1) {
    SOCKET_ERROR(sock, "unable to read from socket", errno);
    return false;
  }
  return retval;
}

Variant HHVM_FUNCTION(socket_recvfrom,
                      const Resource& socket,
                      VRefParam buf,
                      int len,
                      int flags,
                      VRefParam name,
                      VRefParam port /* = -1*/) {
  if (len <= 0) {
    return false;
  }

  auto sock = cast<Socket>(socket);

  char *recv_buf = (char *)malloc(len + 2);
  socklen_t slen;
  int retval;

  switch (sock->getType()) {
  case AF_UNIX:
    {
      struct sockaddr_un s_un;
      slen = sizeof(s_un);
      memset(&s_un, 0, slen);
      s_un.sun_family = AF_UNIX;
      retval = recvfrom(sock->fd(), recv_buf, len, flags,
                        (struct sockaddr *)&s_un, (socklen_t *)&slen);
      if (retval < 0) {
        free(recv_buf);
        SOCKET_ERROR(sock, "unable to recvfrom", errno);
        return false;
      }

      recv_buf[retval] = 0;
      buf = String(recv_buf, retval, AttachString);
      name = String(s_un.sun_path, CopyString);
    }
    break;
  case AF_INET:
    {
      if (int(port) == -1) {
        throw_missing_arguments_nr("socket_recvfrom", 5, 4);
        return false;
      }

      struct sockaddr_in sin;
      slen = sizeof(sin);
      memset(&sin, 0, slen);
      sin.sin_family = AF_INET; // recvfrom doesn't fill this in

      retval = recvfrom(sock->fd(), recv_buf, len, flags,
                        (struct sockaddr *)&sin, (socklen_t *)&slen);
      if (retval < 0) {
        free(recv_buf);
        SOCKET_ERROR(sock, "unable to recvfrom", errno);
        return false;
      }
      recv_buf[retval] = 0;
      buf = String(recv_buf, retval, AttachString);

      try {
        folly::SocketAddress addr;
        addr.setFromSockaddr(&sin);

        name = String(addr.getAddressStr(), CopyString);
        port = addr.getPort();
      } catch (...) {
        name = s_0_0_0_0;
        port = 0;
      }
    }
    break;
  case AF_INET6:
    {
      if (int(port) == -1) {
        throw_missing_arguments_nr("socket_recvfrom", 5, 4);
        return false;
      }

      struct sockaddr_in6 sin6;
      slen = sizeof(sin6);
      memset(&sin6, 0, slen);
      sin6.sin6_family = AF_INET6; // recvfrom doesn't fill this in

      retval = recvfrom(sock->fd(), recv_buf, len, flags,
                        (struct sockaddr *)&sin6, (socklen_t *)&slen);
      if (retval < 0) {
        free(recv_buf);
        SOCKET_ERROR(sock, "unable to recvfrom", errno);
        return false;
      }

      recv_buf[retval] = 0;
      buf = String(recv_buf, retval, AttachString);

      try {
        folly::SocketAddress addr;
        addr.setFromSockaddr(&sin6);

        name = String(addr.getAddressStr(), CopyString);
        port = addr.getPort();
      } catch (...) {
        name = s_2colons;
        port = 0;
      }
    }
    break;
  default:
    raise_warning("Unsupported socket type %d", sock->getType());
    return false;
  }

  return retval;
}

bool HHVM_FUNCTION(socket_shutdown,
                   const Resource& socket,
                   int how /* = 0 */) {
  /* For some operations that are conceptually a socket operation
   * (eg fopen('http://...)) we actually complete it and store the result in
   * a memfile. As the fact that it's not really a socket is an implementation
   * detail, user code needs to be able to call shutdown on it.
   */
  if (socket.is<MemFile>()) {
    return true;
  }
  auto sock = cast<Socket>(socket);
  if (shutdown(sock->fd(), how) != 0) {
    SOCKET_ERROR(sock, "unable to shutdown socket", errno);
    return false;
  }
  return true;
}

void HHVM_FUNCTION(socket_close,
                   const Resource& socket) {
  cast<Socket>(socket)->close();
}

String HHVM_FUNCTION(socket_strerror,
                     int errnum) {
  /*
   * PHP5 encodes both the h_errno and errno values into a single int:
   * < -10000: transformed h_errno value
   * >= -10000: errno value
   */
  if (errnum < -10000) {
    errnum = (-errnum) - 10000;
#ifdef HAVE_HSTRERROR
    return String(hstrerror(errnum), CopyString);
#endif
    return folly::format("Host lookup error {}", errnum).str();
  }

  return String(folly::errnoStr(errnum).toStdString());
}

int64_t HHVM_FUNCTION(socket_last_error,
                      const Variant& socket /* = null */) {
  if (!socket.isNull()) {
    return cast<Socket>(socket)->getError();
  }
  return Socket::getLastError();
}

void HHVM_FUNCTION(socket_clear_error,
                   const Variant& socket /* = null */) {
  if (!socket.isNull()) {
    cast<Socket>(socket)->setError(0);
  }
}
///////////////////////////////////////////////////////////////////////////////
// fsock: treating sockets as "file"

namespace {

thread_local std::unordered_map<
  std::string,
  std::shared_ptr<SocketData>
> s_sockets;

}

Variant sockopen_impl(const HostURL &hosturl, VRefParam errnum,
                      VRefParam errstr, double timeout, bool persistent,
                      const Variant& context) {
  errnum = 0;
  errstr = empty_string();
  std::string key;
  if (persistent) {
    key = hosturl.getHostURL() + ":" +
          folly::to<std::string>(hosturl.getPort());
    auto sockItr = s_sockets.find(key);
    if (sockItr != s_sockets.end()) {
      auto sock = req::make<Socket>(sockItr->second);

      if (sock->getError() == 0 && sock->checkLiveness()) {
        return Variant(sock);
      }

      // socket had an error earlier, we need to close it, remove it from
      // persistent storage, and create a new one (in that order)
      sock->close();
      s_sockets.erase(sockItr);
    }
  }

  if (timeout < 0) {
    timeout = ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.getSocketDefaultTimeout();
  }


  req::ptr<StreamContext> streamctx;
  if (context.isResource()) {
    streamctx = cast<StreamContext>(context.toResource());
  }
  auto socket = new_socket_connect(hosturl, timeout, streamctx, errnum, errstr);
  if (!socket.isResource()) {
    return false;
  }

  if (persistent) {
    assert(!key.empty());
    s_sockets[key] = cast<Socket>(socket)->getData();
    assert(s_sockets[key]);
  }

  return socket;
}

Variant HHVM_FUNCTION(fsockopen,
                      const String& hostname,
                      int port /* = -1 */,
                      VRefParam errnum /* = null */,
                      VRefParam errstr /* = null */,
                      double timeout /* = -1.0 */) {
  HostURL hosturl(static_cast<const std::string>(hostname), port);
  return sockopen_impl(hosturl, errnum, errstr, timeout, false, null_variant);
}

Variant HHVM_FUNCTION(pfsockopen,
                      const String& hostname,
                      int port /* = -1 */,
                      VRefParam errnum /* = null */,
                      VRefParam errstr /* = null */,
                      double timeout /* = -1.0 */) {
  HostURL hosturl(static_cast<const std::string>(hostname), port);
  return sockopen_impl(hosturl, errnum, errstr, timeout, true, null_variant);
}

String ipaddr_convert(struct sockaddr *addr, int addrlen) {
  char buffer[NI_MAXHOST];
  int error = getnameinfo(addr, addrlen, buffer, sizeof(buffer), NULL, 0, NI_NUMERICHOST);

  if (error) {
    raise_warning("%s", gai_strerror(error));
    return empty_string();
  }
  return String(buffer, CopyString);
}

const StaticString
  s_family("family"),
  s_socktype("socktype"),
  s_protocol("protocol"),
  s_address("address"),
  s_port("port"),
  s_flow_info("flow_info"),
  s_scope_id("scope_id"),
  s_sockaddr("sockaddr");

Variant HHVM_FUNCTION(getaddrinfo,
                      const String& host,
                      const String& port,
                      int family /* = 0 */,
                      int socktype /* = 0 */,
                      int protocol /* = 0 */,
                      int flags /* = 0 */) {
  const char *hptr = NULL, *pptr = NULL;
  if (!host.empty()) {
    hptr = host.c_str();
  }
  if (!port.empty()) {
    pptr = port.c_str();
  }

  struct addrinfo hints, *res;
  struct addrinfo *res0 = NULL;
  int error;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_protocol = protocol;
  hints.ai_flags = flags;
  error = getaddrinfo(hptr, pptr, &hints, &res0);

  if (error) {
    raise_warning("%s", gai_strerror(error));

    if (res0) {
      freeaddrinfo(res0);
    }
    return false;
  }

  Array ret = Array::Create();

  for (res = res0; res; res = res->ai_next) {
    Array data = make_map_array(
      s_family, res->ai_family,
      s_socktype, res->ai_socktype,
      s_protocol, res->ai_protocol
    );

    switch (res->ai_addr->sa_family) {
      case AF_INET:
      {
        struct sockaddr_in *a;
        String buffer = ipaddr_convert(res->ai_addr, sizeof(*a));
        if (!buffer.empty()) {
          a = (struct sockaddr_in *)res->ai_addr;
          data.set(
            s_sockaddr,
            make_map_array(
              s_address, buffer,
              s_port, ntohs(a->sin_port)
            )
          );
        }
        break;
      }
      case AF_INET6:
      {
        struct sockaddr_in6 *a;
        String buffer = ipaddr_convert(res->ai_addr, sizeof(*a));
        if (!buffer.empty()) {
          a = (struct sockaddr_in6 *)res->ai_addr;
          data.set(
            s_sockaddr,
            make_map_array(
              s_address, buffer,
              s_port, ntohs(a->sin6_port),
              s_flow_info, (int32_t)a->sin6_flowinfo,
              s_scope_id, (int32_t)a->sin6_scope_id
            )
          );
        }
        break;
      }
      default:
        data.set(s_sockaddr, empty_array());
        break;
    }

    ret.append(data);
  }

  if (res0) {
    freeaddrinfo(res0);
  }

  return ret;
}

#define SOCK_CONST(v) Native::registerConstant<KindOfInt64>( \
                              makeStaticString(#v), v);

const StaticString
  s_SOL_TCP("SOL_TCP"),
  s_SOL_UDP("SOL_UDP");

class SocketsExtension final : public Extension {
 public:
  SocketsExtension() : Extension("sockets", NO_EXTENSION_VERSION_YET) {}

  void moduleInit() override {
    SOCK_CONST(AF_UNIX);
    SOCK_CONST(AF_INET);
    SOCK_CONST(AF_INET6);
    SOCK_CONST(SOCK_STREAM);
    SOCK_CONST(SOCK_DGRAM);
    SOCK_CONST(SOCK_RAW);
    SOCK_CONST(SOCK_SEQPACKET);
    SOCK_CONST(SOCK_RDM);

    SOCK_CONST(MSG_OOB);
    SOCK_CONST(MSG_WAITALL);
    SOCK_CONST(MSG_CTRUNC);
    SOCK_CONST(MSG_TRUNC);
    SOCK_CONST(MSG_PEEK);
    SOCK_CONST(MSG_DONTROUTE);
#ifdef MSG_EOR
    SOCK_CONST(MSG_EOR);
#endif
#ifdef MSG_EOF
    SOCK_CONST(MSG_EOF);
#endif
#ifdef MSG_CONFIRM
    SOCK_CONST(MSG_CONFIRM);
#endif
#ifdef MSG_ERRQUEUE
    SOCK_CONST(MSG_ERRQUEUE);
#endif
#ifdef MSG_NOSIGNAL
    SOCK_CONST(MSG_NOSIGNAL);
#endif
#ifdef MSG_DONTWAIT
    SOCK_CONST(MSG_DONTWAIT);
#endif
#ifdef MSG_MORE
    SOCK_CONST(MSG_MORE);
#endif
#ifdef MSG_WAITFORONE
    SOCK_CONST(MSG_WAITFORONE);
#endif
#ifdef MSG_CMSG_CLOEXEC
    SOCK_CONST(MSG_CMSG_CLOEXEC);
#endif

    SOCK_CONST(SO_DEBUG);
    SOCK_CONST(SO_REUSEADDR);
#ifdef SO_REUSEPORT
    SOCK_CONST(SO_REUSEPORT);
#endif
    SOCK_CONST(SO_KEEPALIVE);
    SOCK_CONST(SO_DONTROUTE);
    SOCK_CONST(SO_LINGER);
    SOCK_CONST(SO_BROADCAST);
    SOCK_CONST(SO_OOBINLINE);
    SOCK_CONST(SO_SNDBUF);
    SOCK_CONST(SO_RCVBUF);
    SOCK_CONST(SO_SNDLOWAT);
    SOCK_CONST(SO_RCVLOWAT);
    SOCK_CONST(SO_SNDTIMEO);
    SOCK_CONST(SO_RCVTIMEO);
    SOCK_CONST(SO_TYPE);
#ifdef SO_FAMILY
    SOCK_CONST(SO_FAMILY);
#endif
    SOCK_CONST(SO_ERROR);
#ifdef SO_BINDTODEVICE
    SOCK_CONST(SO_BINDTODEVICE);
#endif
    SOCK_CONST(SOL_SOCKET);
    SOCK_CONST(SOMAXCONN);
#ifdef TCP_NODELAY
    SOCK_CONST(TCP_NODELAY);
#endif
    SOCK_CONST(PHP_NORMAL_READ);
    SOCK_CONST(PHP_BINARY_READ);

    /* TODO: MCAST_* constants and logic to handle them */

    SOCK_CONST(IP_MULTICAST_IF);
    SOCK_CONST(IP_MULTICAST_TTL);
    SOCK_CONST(IP_MULTICAST_LOOP);
    SOCK_CONST(IPV6_MULTICAST_IF);
    SOCK_CONST(IPV6_MULTICAST_HOPS);
    SOCK_CONST(IPV6_MULTICAST_LOOP);
    SOCK_CONST(IPPROTO_IP);
    SOCK_CONST(IPPROTO_IPV6);

    Native::registerConstant<KindOfInt64>(s_SOL_TCP.get(), IPPROTO_TCP);
    Native::registerConstant<KindOfInt64>(s_SOL_UDP.get(), IPPROTO_UDP);

    SOCK_CONST(IPV6_UNICAST_HOPS);

#define REGISTER_LONG_CONSTANT(name, val, flags) \
  Native::registerConstant<KindOfInt64>(makeStaticString(name), val)
#include "hphp/runtime/ext/sockets/unix_socket_constants.h"
#undef REGISTER_LONG_CONSTANT

    HHVM_FE(socket_create);
    HHVM_FE(socket_create_listen);
    HHVM_FE(socket_create_pair);
    HHVM_FE(socket_get_option);
    HHVM_FE(socket_getpeername);
    HHVM_FE(socket_getsockname);
    HHVM_FE(socket_set_block);
    HHVM_FE(socket_set_nonblock);
    HHVM_FE(socket_set_option);
    HHVM_FE(socket_connect);
    HHVM_FE(socket_bind);
    HHVM_FE(socket_listen);
    HHVM_FE(socket_select);
    HHVM_FE(socket_server);
    HHVM_FE(socket_accept);
    HHVM_FE(socket_read);
    HHVM_FE(socket_write);
    HHVM_FE(socket_send);
    HHVM_FE(socket_sendto);
    HHVM_FE(socket_recv);
    HHVM_FE(socket_recvfrom);
    HHVM_FE(socket_shutdown);
    HHVM_FE(socket_close);
    HHVM_FE(socket_strerror);
    HHVM_FE(socket_last_error);
    HHVM_FE(socket_clear_error);
    HHVM_FE(getaddrinfo);

    loadSystemlib();
  }
} s_sockets_extension;

///////////////////////////////////////////////////////////////////////////////
}
