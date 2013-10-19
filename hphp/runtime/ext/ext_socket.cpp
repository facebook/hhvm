/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_socket.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/ssl-socket.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/logger.h"
#include "folly/String.h"

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
#include "hphp/util/network.h"
#include <poll.h>

#define PHP_NORMAL_READ 0x0001
#define PHP_BINARY_READ 0x0002

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(sockets);
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

static bool get_sockaddr(sockaddr *sa, socklen_t salen,
                         Variant &address, Variant &port) {
  switch (sa->sa_family) {
  case AF_INET6:
    {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
      char addr6[INET6_ADDRSTRLEN+1];
      inet_ntop(AF_INET6, &sin6->sin6_addr, addr6, INET6_ADDRSTRLEN);
      address = String(addr6, CopyString);
      port = htons(sin6->sin6_port);
    }
    return true;
  case AF_INET:
    {
      struct sockaddr_in *sin = (struct sockaddr_in *)sa;
      address = String(Util::safe_inet_ntoa(sin->sin_addr));
      port = htons(sin->sin_port);
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

static bool php_set_inet6_addr(struct sockaddr_in6 *sin6, const char *address,
                               Socket *sock) {
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

static bool php_set_inet_addr(struct sockaddr_in *sin, const char *address,
                              Socket *sock) {
  struct in_addr tmp;

  if (inet_aton(address, &tmp)) {
    sin->sin_addr.s_addr = tmp.s_addr;
  } else {
    Util::HostEnt result;
    if (!Util::safe_gethostbyname(address, result)) {
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

static bool set_sockaddr(sockaddr_storage &sa_storage, Socket *sock,
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

static void sock_array_to_fd_set(CArrRef sockets, pollfd *fds, int &nfds,
                                 short flag) {
  assert(fds);
  for (ArrayIter iter(sockets); iter; ++iter) {
    File *sock = iter.second().toResource().getTyped<File>();
    pollfd &fd = fds[nfds++];
    fd.fd = sock->fd();
    fd.events = flag;
    fd.revents = 0;
  }
}

static void sock_array_from_fd_set(Variant &sockets, pollfd *fds, int &nfds,
                                   int &count, short flag) {
  assert(sockets.is(KindOfArray));
  Array sock_array = sockets.toArray();
  Array ret;
  for (ArrayIter iter(sock_array); iter; ++iter) {
    pollfd &fd = fds[nfds++];
    assert(fd.fd == iter.second().toResource().getTyped<File>()->fd());
    if (fd.revents & flag) {
      ret.append(iter.second());
      count++;
    }
  }
  sockets = ret;
}

static int php_read(Socket *sock, void *buf, int maxlen, int flags) {
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

static bool create_new_socket(const Util::HostURL &hosturl,
                              Variant &errnum, Variant &errstr, Resource &ret,
                              Socket *&sock, double timeout) {
  int domain = hosturl.isIPv6() ? AF_INET6 : AF_INET;
  int type = SOCK_STREAM;
  const std::string scheme = hosturl.getScheme();

  if (scheme == "udp" || scheme == "udg") {
    type = SOCK_DGRAM;
  } else if (scheme == "unix") {
    domain = AF_UNIX;
  }

  sock = new Socket(socket(domain, type, 0), domain,
                    hosturl.getHost().c_str(), hosturl.getPort(), timeout);
  ret = Resource(sock);
  if (!sock->valid()) {
    SOCKET_ERROR(sock, "unable to create socket", errno);
    errnum = sock->getError();
    errstr = String(folly::errnoStr(sock->getError()).toStdString());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_socket_create(int domain, int type, int protocol) {
  check_socket_parameters(domain, type);
  int socketId = socket(domain, type, protocol);
  if (socketId == -1) {
    Socket dummySock; // for setting last socket error
    SOCKET_ERROR((&dummySock), "Unable to create socket", errno);
    return false;
  }
  Socket *sock = new Socket(socketId, domain);
  Resource ret(sock);
  return ret;
}

Variant f_socket_create_listen(int port, int backlog /* = 128 */) {
  Util::HostEnt result;
  if (!Util::safe_gethostbyname("0.0.0.0", result)) {
    return false;
  }

  struct sockaddr_in la;
  memcpy((char *) &la.sin_addr, result.hostbuf.h_addr,
         result.hostbuf.h_length);
  la.sin_family = result.hostbuf.h_addrtype;
  la.sin_port = htons((unsigned short)port);

  Socket *sock = new Socket(socket(PF_INET, SOCK_STREAM, 0), PF_INET,
                            "0.0.0.0", port);
  Resource ret(sock);
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

  return ret;
}

bool f_socket_create_pair(int domain, int type, int protocol, VRefParam fd) {
  check_socket_parameters(domain, type);

  int fds_array[2];
  if (socketpair(domain, type, protocol, fds_array) != 0) {
    Socket dummySock; // for setting last socket error
    SOCKET_ERROR((&dummySock), "unable to create socket pair", errno);
    return false;
  }

  Array ret;
  ret.set(0, Resource(new Socket(fds_array[0], domain)));
  ret.set(1, Resource(new Socket(fds_array[1], domain)));
  fd = ret;
  return true;
}

const StaticString
  s_l_onoff("l_onoff"),
  s_l_linger("l_linger"),
  s_sec("sec"),
  s_usec("usec");

Variant f_socket_get_option(CResRef socket, int level, int optname) {
  Socket *sock = socket.getTyped<Socket>();
  Array ret;
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

      ret.set(s_l_onoff, linger_val.l_onoff);
      ret.set(s_l_linger, linger_val.l_linger);
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
      ret.set(s_sec,  (int)tv.tv_sec);
      ret.set(s_usec, (int)tv.tv_usec);
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
  return ret;
}

bool f_socket_getpeername(CResRef socket, VRefParam address,
                          VRefParam port /* = null */) {
  Socket *sock = socket.getTyped<Socket>();

  sockaddr_storage sa_storage;
  socklen_t salen = sizeof(sockaddr_storage);
  struct sockaddr *sa = (struct sockaddr *)&sa_storage;
  if (getpeername(sock->fd(), sa, &salen) < 0) {
    SOCKET_ERROR(sock, "unable to retrieve peer name", errno);
    return false;
  }
  return get_sockaddr(sa, salen, address, port);
}

bool f_socket_getsockname(CResRef socket, VRefParam address,
                          VRefParam port /* = null */) {
  Socket *sock = socket.getTyped<Socket>();

  sockaddr_storage sa_storage;
  socklen_t salen = sizeof(sockaddr_storage);
  struct sockaddr *sa = (struct sockaddr *)&sa_storage;
  if (getsockname(sock->fd(), sa, &salen) < 0) {
    SOCKET_ERROR(sock, "unable to retrieve peer name", errno);
    return false;
  }
  return get_sockaddr(sa, salen, address, port);
}

bool f_socket_set_block(CResRef socket) {
  Socket *sock = socket.getTyped<Socket>();
  return sock->setBlocking(true);
}

bool f_socket_set_nonblock(CResRef socket) {
  Socket *sock = socket.getTyped<Socket>();
  return sock->setBlocking(false);
}

bool f_socket_set_option(CResRef socket, int level, int optname,
                         CVarRef optval) {
  Socket *sock = socket.getTyped<Socket>();

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

bool f_socket_connect(CResRef socket, const String& address, int port /* = 0 */) {
  Socket *sock = socket.getTyped<Socket>();

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
    msg += boost::lexical_cast<std::string>(port);
    SOCKET_ERROR(sock, msg.c_str(), errno);
    return false;
  }

  return true;
}

bool f_socket_bind(CResRef socket, const String& address, int port /* = 0 */) {
  Socket *sock = socket.getTyped<Socket>();

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
    msg += boost::lexical_cast<std::string>(port);
    SOCKET_ERROR(sock, msg.c_str(), errno);
    return false;
  }

  return true;
}

bool f_socket_listen(CResRef socket, int backlog /* = 0 */) {
  Socket *sock = socket.getTyped<Socket>();
  if (listen(sock->fd(), backlog) != 0) {
    SOCKET_ERROR(sock, "unable to listen on socket", errno);
    return false;
  }
  return true;
}

Variant f_socket_select(VRefParam read, VRefParam write, VRefParam except,
                        CVarRef vtv_sec, int tv_usec /* = 0 */) {
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
      File *file = iter.second().toResource().getTyped<File>();
      if (file->bufferedLen() > 0) {
        hasData.append(iter.second());
      }
    }
    if (hasData.size() > 0) {
      if (!write.isNull()) {
        write = Array::Create();
      }
      if (!except.isNull()) {
        except = Array::Create();
      }
      read = hasData;
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

Variant f_socket_server(const String& hostname, int port /* = -1 */,
                        VRefParam errnum /* = null */,
                        VRefParam errstr /* = null */) {
  Util::HostURL hosturl(static_cast<const std::string>(hostname), port);
  return socket_server_impl(hosturl,
                            k_STREAM_SERVER_BIND|k_STREAM_SERVER_LISTEN,
                            errnum, errstr);
}

Variant socket_server_impl(const Util::HostURL &hosturl,
                           int flags, /* = STREAM_SERVER_BIND|STREAM_SERVER_LISTEN */
                           VRefParam errnum /* = null */,
                           VRefParam errstr /* = null */) {
  Resource ret;
  Socket *sock = NULL;
  if (!create_new_socket(hosturl, errnum, errstr, ret, sock, 0.0)) {
    return false;
  }
  assert(ret.get() && sock);

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

  return ret;
}

Variant f_socket_accept(CResRef socket) {
  Socket *sock = socket.getTyped<Socket>();
  struct sockaddr sa;
  socklen_t salen = sizeof(sa);
  Socket *new_sock = new Socket(accept(sock->fd(), &sa, &salen),
                                sock->getType());
  if (!new_sock->valid()) {
    SOCKET_ERROR(new_sock, "unable to accept incoming connection", errno);
    delete new_sock;
    return false;
  }
  return Resource(new_sock);
}

Variant f_socket_read(CResRef socket, int length, int type /* = 0 */) {
  if (length <= 0) {
    return false;
  }
  Socket *sock = socket.getTyped<Socket>();

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

Variant f_socket_write(CResRef socket, const String& buffer, int length /* = 0 */) {
  Socket *sock = socket.getTyped<Socket>();
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

Variant f_socket_send(CResRef socket, const String& buf, int len, int flags) {
  Socket *sock = socket.getTyped<Socket>();
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

Variant f_socket_sendto(CResRef socket, const String& buf, int len, int flags,
                        const String& addr, int port /* = -1 */) {
  Socket *sock = socket.getTyped<Socket>();
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

Variant f_socket_recv(CResRef socket, VRefParam buf, int len, int flags) {
  if (len <= 0) {
    return false;
  }
  Socket *sock = socket.getTyped<Socket>();

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

const StaticString
  s_2colons("::"),
  s_0_0_0_0("0.0.0.0");

Variant f_socket_recvfrom(CResRef socket, VRefParam buf, int len, int flags,
                      VRefParam name, VRefParam port /* = -1*/) {
  if (len <= 0) {
    return false;
  }

  Socket *sock = socket.getTyped<Socket>();

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

      retval = recvfrom(sock->fd(), recv_buf, len, flags,
                        (struct sockaddr *)&sin, (socklen_t *)&slen);
      if (retval < 0) {
        free(recv_buf);
        SOCKET_ERROR(sock, "unable to recvfrom", errno);
        return false;
      }
      recv_buf[retval] = 0;
      buf = String(recv_buf, retval, AttachString);

      name = String(Util::safe_inet_ntoa(sin.sin_addr));
      if (name.toString().empty()) {
        name = s_0_0_0_0;
      }
      port = ntohs(sin.sin_port);
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

      retval = recvfrom(sock->fd(), recv_buf, len, flags,
                        (struct sockaddr *)&sin6, (socklen_t *)&slen);
      if (retval < 0) {
        free(recv_buf);
        SOCKET_ERROR(sock, "unable to recvfrom", errno);
        return false;
      }

      char addr6[INET6_ADDRSTRLEN];
      const char* succ =
        inet_ntop(AF_INET6, &sin6.sin6_addr, addr6, INET6_ADDRSTRLEN);

      recv_buf[retval] = 0;
      buf = String(recv_buf, retval, AttachString);
      if (succ) {
        name = String(addr6, CopyString);
      } else {
        name = s_2colons;
      }
      port = ntohs(sin6.sin6_port);
    }
    break;
  default:
    raise_warning("Unsupported socket type %d", sock->getType());
    return false;
  }

  return retval;
}

bool f_socket_shutdown(CResRef socket, int how /* = 0 */) {
  Socket *sock = socket.getTyped<Socket>();
  if (shutdown(sock->fd(), how) != 0) {
    SOCKET_ERROR(sock, "unable to shutdown socket", errno);
    return false;
  }
  return true;
}

void f_socket_close(CResRef socket) {
  Socket *sock = socket.getTyped<Socket>();
  sock->close();
}

String f_socket_strerror(int errnum) {
  return String(folly::errnoStr(errnum).toStdString());
}

int64_t f_socket_last_error(CResRef socket /* = null_object */) {
  if (!socket.isNull()) {
    Socket *sock = socket.getTyped<Socket>();
    return sock->getError();
  }
  return Socket::getLastError();
}

void f_socket_clear_error(CResRef socket /* = null_object */) {
  if (!socket.isNull()) {
    Socket *sock = socket.getTyped<Socket>();
    sock->setError(0);
  }
}
///////////////////////////////////////////////////////////////////////////////
// fsock: treating sockets as "file"

Variant sockopen_impl(const Util::HostURL &hosturl, VRefParam errnum,
                      VRefParam errstr, double timeout, bool persistent) {

  string key;
  if (persistent) {
    key = hosturl.getHostURL();
    Socket *sock =
      dynamic_cast<Socket*>(g_persistentObjects->get("socket", key.c_str()));
    if (sock) {
      if (sock->getError() == 0 && sock->checkLiveness()) {
        return Resource(sock);
      }

      // socket had an error earlier, we need to remove it from persistent
      // storage, and create a new one
      g_persistentObjects->remove("socket", key.c_str());
    }
  }

  Resource ret;
  Socket *sock = NULL;

  if (timeout < 0) timeout = RuntimeOption::SocketDefaultTimeout;
  // test if protocol is SSL
  SSLSocket *sslsock = SSLSocket::Create(hosturl, timeout);
  if (sslsock) {
    sock = sslsock;
    ret = sock;
  } else if (!create_new_socket(hosturl, errnum, errstr, ret, sock, timeout)) {
    return false;
  }
  assert(ret.get() && sock);

  sockaddr_storage sa_storage;
  struct sockaddr *sa_ptr;
  size_t sa_size;
  if (!set_sockaddr(sa_storage, sock, hosturl.getHost().c_str(),
                    hosturl.getPort(), sa_ptr, sa_size)) {
    return false;
  }

  int retval;
  int fd = sock->fd();
  IOStatusHelper io("socket::connect",
                    hosturl.getHostURL().c_str(), hosturl.getPort());
  if (timeout <= 0) {
    retval = connect(fd, sa_ptr, sa_size);
  } else {
    // set non-blocking so we can do timeouts
    long arg = fcntl(fd, F_GETFL, NULL);
    fcntl(fd, F_SETFL, arg | O_NONBLOCK);

    retval = connect(fd, sa_ptr, sa_size);
    if (retval < 0) {
      if (errno == EINPROGRESS) {
        struct pollfd fds[1];
        fds[0].fd = fd;
        fds[0].events = POLLOUT;
        if (poll(fds, 1, (int)(timeout * 1000))) {
          socklen_t lon = sizeof(int);
          int valopt;
          getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
          if (valopt) {
            std::string msg = "failed to connect to " + hosturl.getHostURL();
            SOCKET_ERROR(sock, msg.c_str(), valopt);
            errnum = sock->getError();
            errstr = String(folly::errnoStr(sock->getError()).toStdString());
            return false;
          } else {
            retval = 0; // success
          }
        } else {
          std::string msg = "timed out after ";
          msg += boost::lexical_cast<std::string>(timeout);
          msg += " seconds when connecting to " + hosturl.getHostURL();
          SOCKET_ERROR(sock, msg.c_str(), ETIMEDOUT);
          errnum = sock->getError();
          errstr = String(folly::errnoStr(sock->getError()).toStdString());
          return false;
        }
      }
    }

    // set to blocking mode
    arg = fcntl(fd, F_GETFL, NULL);
    fcntl(fd, F_SETFL, arg & ~O_NONBLOCK);
  }

  if (retval != 0) {
    errnum = sock->getError();
    errstr = String(folly::errnoStr(sock->getError()).toStdString());
    return false;
  }

  if (sslsock && !sslsock->onConnect()) {
    raise_warning("Failed to enable crypto");
    return false;
  }

  if (persistent) {
    assert(!key.empty());
    g_persistentObjects->set("socket", key.c_str(), sock);
  }

  return ret;
}

Variant f_fsockopen(const String& hostname, int port /* = -1 */,
                    VRefParam errnum /* = null */,
                    VRefParam errstr /* = null */,
                    double timeout /* = -1.0 */) {
  Util::HostURL hosturl(static_cast<const std::string>(hostname), port);
  return sockopen_impl(hosturl, errnum, errstr, timeout, false);
}

Variant f_pfsockopen(const String& hostname, int port /* = -1 */,
                     VRefParam errnum /* = null */,
                     VRefParam errstr /* = null */,
                     double timeout /* = -1.0 */) {
  // TODO: persistent socket handling
  Util::HostURL hosturl(static_cast<const std::string>(hostname), port);
  return sockopen_impl(hosturl, errnum, errstr, timeout, true);
}

String ipaddr_convert(struct sockaddr *addr, int addrlen) {
  char buffer[NI_MAXHOST];
  int error = getnameinfo(addr, addrlen, buffer, sizeof(buffer), NULL, 0, NI_NUMERICHOST);

  if (error) {
    raise_warning("%s", gai_strerror(error));
    return "";
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

Variant f_getaddrinfo(const String& host, const String& port, int family /* = 0 */,
                      int socktype /* = 0 */, int protocol /* = 0 */,
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
    Array data = Array::Create();
    Array sockinfo = Array::Create();

    data.set(s_family, res->ai_family);
    data.set(s_socktype, res->ai_socktype);
    data.set(s_protocol, res->ai_protocol);

    switch (res->ai_addr->sa_family) {
      case AF_INET:
      {
        struct sockaddr_in *a;
        String buffer = ipaddr_convert(res->ai_addr, sizeof(*a));
        if (!buffer.empty()) {
          a = (struct sockaddr_in *)res->ai_addr;
          sockinfo.set(s_address, buffer);
          sockinfo.set(s_port, ntohs(a->sin_port));
        }
        break;
      }
      case AF_INET6:
      {
        struct sockaddr_in6 *a;
        String buffer = ipaddr_convert(res->ai_addr, sizeof(*a));
        if (!buffer.empty()) {
          a = (struct sockaddr_in6 *)res->ai_addr;
          sockinfo.set(s_address, buffer);
          sockinfo.set(s_port, ntohs(a->sin6_port));
          sockinfo.set(s_flow_info, (int32_t)a->sin6_flowinfo);
          sockinfo.set(s_scope_id, (int32_t)a->sin6_scope_id);
        }
        break;
      }
    }

    data.set(s_sockaddr, (sockinfo.empty() ? nullptr : sockinfo));

    ret.append(data);
  }

  if (res0) {
    freeaddrinfo(res0);
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
