/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp/transport/TSocket.h>

#include <errno.h>
#include <sys/types.h>

#include <cstring>
#include <sstream>
#include <thread>

#include <folly/portability/Fcntl.h>
#include <folly/portability/Sockets.h>
#include <folly/portability/Unistd.h>

#include <thrift/lib/cpp/thrift_config.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp/util/PausableTimer.h>

namespace apache {
namespace thrift {
namespace transport {

using namespace std;

namespace fsp = folly::portability::sockets;

// Global var to track total socket sys calls
uint32_t g_socket_syscalls = 0;

// Global helper functions

static int msTimeFromTimeval(struct timeval s) {
  return folly::to<int>(s.tv_sec * 1000 + s.tv_usec / 1000);
}

ostream& operator<<(ostream& os, const TSocket::Options& o) {
  os << "SOCKET OPTIONS"
     << "\n";
  os << "connTimeout = " << o.connTimeout << "\n";
  os << "sendTimeout = " << o.sendTimeout << "\n";
  os << "recvTimeout = " << o.recvTimeout << "\n";
  os << "sendBufSize = " << o.sendBufSize << "\n";
  os << "recvBufSize = " << o.recvBufSize << "\n";
  os << "lingerOn    = " << o.lingerOn << "\n";
  os << "lingerVal   = " << o.lingerVal << "\n";
  os << "noDelay     = " << o.noDelay << "\n";
  os << "reuseAddr   = " << o.reuseAddr << "\n";
  return os;
}

/**
 * TSocket implementation.
 *
 */

TSocket::TSocket(string host, int port)
    : host_(host), port_(port), socket_(-1), maxRecvRetries_(5) {}

TSocket::TSocket(const folly::SocketAddress* address)
    : host_(address->isFamilyInet() ? address->getAddressStr() : ""),
      port_(address->isFamilyInet() ? address->getPort() : 0),
      path_(address->isFamilyInet() ? "" : address->getPath()),
      socket_(-1),
      maxRecvRetries_(5) {
  // For convenience with the existing code that resolves hostnames,
  // we just store the IP address as a string in host_, and convert it back to
  // an address in connect()
}

TSocket::TSocket(const folly::SocketAddress& address)
    : host_(address.isFamilyInet() ? address.getAddressStr() : ""),
      port_(address.isFamilyInet() ? address.getPort() : 0),
      path_(address.isFamilyInet() ? "" : address.getPath()),
      socket_(-1),
      maxRecvRetries_(5) {
  // For convenience with the existing code that resolves hostnames,
  // we just store the IP address as a string in host_, and convert it back to
  // an address in connect()
}

TSocket::TSocket(string path)
    : host_(""), port_(0), path_(path), socket_(-1), maxRecvRetries_(5) {}

TSocket::TSocket() : host_(""), port_(0), socket_(-1), maxRecvRetries_(5) {}

TSocket::TSocket(int socket)
    : host_(""), port_(0), socket_(socket), maxRecvRetries_(5) {}

TSocket::~TSocket() {
  close();
}

bool TSocket::isOpen() {
  return (socket_ >= 0);
}

bool TSocket::peek() {
  if (!isOpen()) {
    return false;
  }
  uint8_t buf;
  ssize_t r = recv(socket_, &buf, 1, MSG_PEEK);
  if (r == -1) {
    int errno_copy = errno;
#if defined __FreeBSD__ || defined __MACH__
    /* shigin:
     * freebsd returns -1 and ECONNRESET if socket was closed by
     * the other side
     */
    if (errno_copy == ECONNRESET) {
      close();
      return false;
    }
#endif
    GlobalOutput.perror(
        "TSocket::peek() recv() " + getSocketInfo(), errno_copy);
    throw TTransportException(
        TTransportException::UNKNOWN, "recv()", errno_copy);
  }
  return (r > 0);
}

void TSocket::openConnection(struct addrinfo* res) {
  apache::thrift::util::PausableTimer pausableTimer(options_.connTimeout);
  int errno_after_poll;

  if (isOpen()) {
    throw TTransportException(TTransportException::ALREADY_OPEN);
  }

  if (!path_.empty()) {
    socket_ = fsp::socket(PF_UNIX, SOCK_STREAM, IPPROTO_IP);
  } else {
    socket_ = fsp::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  }
  if (socket_ == -1) {
    int errno_copy = errno;
    GlobalOutput.perror(
        "TSocket::open() socket() " + getSocketInfo(), errno_copy);
    throw TTransportException(
        TTransportException::NOT_OPEN, "socket()", errno_copy);
  }

  setSocketOptions(options_);

  // Uses a low min RTO if asked to.
#ifdef TCP_LOW_MIN_RTO
  if (getUseLowMinRto()) {
    int one = 1;
    setsockopt(socket_, IPPROTO_TCP, TCP_LOW_MIN_RTO, &one, sizeof(one));
  }
#endif

  // Set the socket to be non blocking for connect if a timeout exists
  int flags = fcntl(socket_, F_GETFL, 0);
  if (options_.connTimeout > 0) {
    if (-1 == fcntl(socket_, F_SETFL, flags | O_NONBLOCK)) {
      int errno_copy = errno;
      GlobalOutput.perror(
          "TSocket::open() fcntl() " + getSocketInfo(), errno_copy);
      throw TTransportException(
          TTransportException::NOT_OPEN, "fcntl() failed", errno_copy);
    }
  } else {
    if (-1 == fcntl(socket_, F_SETFL, flags & ~O_NONBLOCK)) {
      int errno_copy = errno;
      GlobalOutput.perror(
          "TSocket::open() fcntl " + getSocketInfo(), errno_copy);
      throw TTransportException(
          TTransportException::NOT_OPEN, "fcntl() failed", errno_copy);
    }
  }

  if (options_.reuseAddr) {
    int val = 1;
    if (-1 ==
        setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::open() setsockopt(SO_REUSEADDR) " + getSocketInfo(),
          errno_copy);
      // No need to throw.
    }
  }

  // Connect the socket
  int ret;
  if (!path_.empty()) {
    size_t len = path_.size() + 1;
    if (len > sizeof(((sockaddr_un*)nullptr)->sun_path)) {
      int errno_copy = errno;
      GlobalOutput.perror(
          "TSocket::open() Unix Domain socket path too long", errno_copy);
      throw TTransportException(
          TTransportException::NOT_OPEN, " Unix Domain socket path too long");
    }

    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, path_.c_str(), len);
    socklen_t structlen = static_cast<socklen_t>(sizeof(address));
    ret = connect(socket_, (struct sockaddr*)&address, structlen);
  } else {
    ret = connect(socket_, res->ai_addr, static_cast<int>(res->ai_addrlen));
  }

  // success case
  if (ret == 0) {
    goto done;
  }

  if (errno != EINPROGRESS) {
    int errno_copy = errno;
    GlobalOutput.perror(
        "TSocket::open() connect() " + getSocketInfo(), errno_copy);
    throw TTransportException(
        TTransportException::NOT_OPEN,
        "connect() failed " + getSocketInfo(),
        errno_copy);
  }

try_again:
  struct pollfd fds[1];
  std::memset(fds, 0, sizeof(fds));
  fds[0].fd = socket_;
  fds[0].events = POLLOUT;

  // When there is a poll timeout set, an EINTR will restart the
  // poll() and hence reset the timer. So if we receive EINTRs at a
  // faster rate than the timeout value, the timeout will never
  // trigger. Therefore, we keep track of the total amount of time
  // we've spend in poll(), and if this value exceeds the timeout then
  // we stop retrying on EINTR. Note that we might still exceed the
  // timeout, but by at most a factor of 2.
  pausableTimer.start();
  ret = poll(fds, 1, options_.connTimeout);
  errno_after_poll =
      errno; // gettimeofday, used by PausableTimer, can change errno
  pausableTimer.stop();

  if (ret > 0) {
    // Ensure the socket is connected and that there are no errors set
    int val;
    socklen_t lon;
    lon = sizeof(int);
    int ret2 = getsockopt(socket_, SOL_SOCKET, SO_ERROR, (void*)&val, &lon);
    if (ret2 == -1) {
      GlobalOutput.perror(
          "TSocket::open() getsockopt() " + getSocketInfo(), errno_after_poll);
      throw TTransportException(
          TTransportException::NOT_OPEN, "getsockopt()", errno_after_poll);
    }
    // no errors on socket, go to town
    if (val == 0) {
      goto done;
    }
    GlobalOutput.perror(
        "TSocket::open() error on socket (after poll) " + getSocketInfo(), val);
    throw TTransportException(
        TTransportException::NOT_OPEN, "socket open() error", val);
  } else if (ret == 0) {
    // socket timed out
    string errStr = "TSocket::open() timed out " + getSocketInfo();
    GlobalOutput(errStr.c_str());
    throw TTransportException(
        TTransportException::NOT_OPEN, "open() timed out " + getSocketInfo());
  } else {
    // If interrupted, try again, but only if we haven't exceeded the
    // timeout value yet.
    if (errno_after_poll == EINTR) {
      if (pausableTimer.hasExceededTimeLimit()) {
        GlobalOutput.perror(
            "TSocket::open() poll() (EINTRs, then timed out) " +
                getSocketInfo(),
            errno_after_poll);
        throw TTransportException(
            TTransportException::NOT_OPEN,
            "poll() failed (EINTRs, then timed out)",
            errno_after_poll);
      } else {
        goto try_again;
      }
    } else { // error on poll() other than EINTR
      GlobalOutput.perror(
          "TSocket::open() poll() " + getSocketInfo(), errno_after_poll);
      throw TTransportException(
          TTransportException::NOT_OPEN, "poll() failed", errno_after_poll);
    }
  }

done:
  // Set socket back to normal mode (blocking)
  fcntl(socket_, F_SETFL, flags);

  if (path_.empty()) {
    setCachedAddress(res->ai_addr, res->ai_addrlen);
  }
}

void TSocket::open() {
  if (isOpen()) {
    throw TTransportException(TTransportException::ALREADY_OPEN);
  }
  if (!path_.empty()) {
    unix_open();
  } else {
    local_open();
  }
}

void TSocket::unix_open() {
  if (!path_.empty()) {
    // Unix Domain Socket does not need addrinfo struct, so we pass nullptr
    openConnection(nullptr);
  }
}

void TSocket::local_open() {
  // Validate port number
  if (port_ < 0 || port_ > 0xFFFF) {
    throw TTransportException(
        TTransportException::NOT_OPEN, "Specified port is invalid");
  }

  struct addrinfo hints, *res, *res0;
  res = nullptr;
  res0 = nullptr;
  int error;
  char port[sizeof("65535")];
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  sprintf(port, "%d", port_);

  error = getaddrinfo(host_.c_str(), port, &hints, &res0);

  if (error) {
    string errStr = "TSocket::open() getaddrinfo() " + maybeGetSocketInfo() +
        string(gai_strerror(error));
    GlobalOutput(errStr.c_str());
    close();
    throw TTransportException(
        TTransportException::NOT_OPEN,
        "Could not resolve host '" + host_ + "' for client socket " +
            maybeGetSocketInfo());
  }

  // Cycle through all the returned addresses until one
  // connects or push the exception up.
  for (res = res0; res; res = res->ai_next) {
    try {
      openConnection(res);
      break;
    } catch (TTransportException&) {
      if (res->ai_next) {
        close();
      } else {
        close();
        freeaddrinfo(res0); // cleanup on failure
        throw;
      }
    }
  }

  // Free address structure memory
  freeaddrinfo(res0);
}

void TSocket::close() {
  if (socket_ >= 0) {
    shutdown(socket_, SHUT_RDWR);
    ::close(socket_);
  }
  socket_ = -1;
  peerHost_.clear();
  peerAddressStr_.clear();
  cachedPeerAddr_.reset();
}

int TSocket::stealSocketFD() {
  int stolen = socket_;
  socket_ = -1;
  close(); // clear cached ivars
  return stolen;
}

uint32_t TSocket::read(uint8_t* buf, uint32_t len) {
  if (socket_ < 0) {
    throw TTransportException(
        TTransportException::NOT_OPEN, "Called read on non-open socket");
  }

  int32_t retries = 0;

  // EAGAIN can be signaled both when a timeout has occurred and when
  // the system is out of resources (an awesome undocumented feature).
  // The following is an approximation of the time interval under which
  // EAGAIN is taken to indicate an out of resources error.
  uint32_t eagainThresholdMicros = 0;
  if (options_.recvTimeout) {
    // if a readTimeout is specified along with a max number of recv retries,
    // then the threshold will ensure that the read timeout is not exceeded even
    // in the case of resource errors
    eagainThresholdMicros = (options_.recvTimeout * 1000) /
        ((maxRecvRetries_ > 0) ? maxRecvRetries_ : 2);
  }

  apache::thrift::util::PausableTimer pausableTimer(options_.recvTimeout);

try_again:
  // When there is a recv timeout set, an EINTR will restart the
  // recv() and hence reset the timer.  So if we receive EINTRs at a
  // faster rate than the timeout value, the timeout will never
  // trigger.  Therefore, we keep track of the total amount of time
  // we've spend in recv(), and if this value exceeds the timeout then
  // we stop retrying on EINTR.  Note that we might still exceed the
  // timeout, but by at most a factor of 2.
  pausableTimer.start();
  int got = folly::to<int>(recv(socket_, buf, size_t(len), 0));
  int errno_after_recv =
      errno; // gettimeofday, used by PausableTimer, can change errno
  pausableTimer.stop();
  ++g_socket_syscalls;

  // Check for error on read
  if (got < 0) {
    if (errno_after_recv == EAGAIN) {
      // if no timeout we can assume that resource exhaustion has occurred.
      if (options_.recvTimeout == 0) {
        throw TTransportException(
            TTransportException::TIMED_OUT, "EAGAIN (unavailable resources)");
      }
      // check if this is the lack of resources or timeout case
      if (pausableTimer.didLastRunningTimeExceedLimit(eagainThresholdMicros)) {
        // infer that timeout has been hit
        throw TTransportException(
            TTransportException::TIMED_OUT,
            "EAGAIN (timed out) " + getSocketInfo());
      } else {
        if (retries++ < maxRecvRetries_) {
          this_thread::sleep_for(chrono::microseconds(50));
          goto try_again;
        } else {
          throw TTransportException(
              TTransportException::TIMED_OUT, "EAGAIN (unavailable resources)");
        }
      }
    }

    // If interrupted, try again, but only if we haven't exceeded the
    // timeout value yet.
    if (errno_after_recv == EINTR) {
      if (pausableTimer.hasExceededTimeLimit()) {
        // Exceeded the timeout value, this is a real retry now.
        if (retries++ < maxRecvRetries_) {
          pausableTimer.reset();
          goto try_again;
        } else {
          throw TTransportException(
              TTransportException::TIMED_OUT,
              "recv() failed (EINTRs, then timed out)",
              errno_after_recv);
        }
      } else {
        goto try_again;
      }
    }

#if defined __FreeBSD__ || defined __MACH__
    if (errno_after_recv == ECONNRESET) {
      /* shigin: freebsd doesn't follow POSIX semantic of recv and fails with
       * ECONNRESET if peer performed shutdown
       * edhall: eliminated close() since we do that in the destructor.
       */
      return 0;
    }
#endif

    // Now it's not a try again case, but a real probblez
    GlobalOutput.perror(
        "TSocket::read() recv() " + getSocketInfo(), errno_after_recv);

    // If we disconnect with no linger time
    if (errno_after_recv == ECONNRESET) {
      throw TTransportException(
          TTransportException::NOT_OPEN, "ECONNRESET " + getSocketInfo());
    }

    // This ish isn't open
    if (errno_after_recv == ENOTCONN) {
      throw TTransportException(
          TTransportException::NOT_OPEN, "ENOTCONN " + getSocketInfo());
    }

    // Timed out!
    if (errno_after_recv == ETIMEDOUT) {
      throw TTransportException(
          TTransportException::TIMED_OUT, "ETIMEDOUT " + getSocketInfo());
    }

    // Some other error, whatevz
    throw TTransportException(
        TTransportException::UNKNOWN,
        "Unknown " + getSocketInfo(),
        errno_after_recv);
  }

  // The remote host has closed the socket
  if (got == 0) {
    // edhall: we used to call close() here, but our caller may want to deal
    // with the socket fd and we'll close() in our destructor in any case.
    return 0;
  }

  // Pack data into string
  return got;
}

void TSocket::write(const uint8_t* buf, uint32_t len) {
  uint32_t sent = 0;

  while (sent < len) {
    uint32_t b = write_partial(buf + sent, len - sent);
    if (b == 0) {
      // This should only happen if the timeout set with SO_SNDTIMEO expired.
      // Raise an exception.
      // However, first set SO_LINGER to 0 seconds for this socket.  We think
      // the remote endpoint is not responding, so when we call close() we just
      // want the server to drop any untransmitted data immediately, instead of
      // moving into the TCP_FIN_WAIT1 state and continuing to try and send it.
      setLinger(true, 0);
      throw TTransportException(
          TTransportException::TIMED_OUT,
          "send timeout expired " + getSocketInfo());
    }
    sent += b;
  }
}

uint32_t TSocket::write_partial(const uint8_t* buf, uint32_t len) {
  if (socket_ < 0) {
    throw TTransportException(
        TTransportException::NOT_OPEN, "Called write on non-open socket");
  }

  uint32_t sent = 0;

  int flags = 0;
#ifdef MSG_NOSIGNAL
  // Note the use of MSG_NOSIGNAL to suppress SIGPIPE errors, instead we
  // check for the EPIPE return condition and close the socket in that case
  flags |= MSG_NOSIGNAL;
#endif // ifdef MSG_NOSIGNAL

  int b = folly::to<int>(send(socket_, buf + sent, size_t(len - sent), flags));
  ++g_socket_syscalls;

  if (b < 0) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      return 0;
    }
    // Fail on a send error
    int errno_copy = errno;
    GlobalOutput.perror(
        "TSocket::write_partial() send() " + getSocketInfo(), errno_copy);

    if (errno_copy == EPIPE || errno_copy == ECONNRESET ||
        errno_copy == ENOTCONN) {
      close();
      throw TTransportException(
          TTransportException::NOT_OPEN,
          "write() send() " + getSocketInfo(),
          errno_copy);
    }

    throw TTransportException(
        TTransportException::UNKNOWN,
        "write() send() " + getSocketInfo(),
        errno_copy);
  }

  // Fail on blocked send
  if (b == 0) {
    throw TTransportException(
        TTransportException::NOT_OPEN, "Socket send returned 0.");
  }
  return b;
}

std::string TSocket::getHost() {
  return host_;
}

int TSocket::getPort() {
  return port_;
}

void TSocket::setHost(string host) {
  host_ = host;
}

void TSocket::setPort(int port) {
  port_ = port;
}

void TSocket::setSocketOptions(const Options& options) {
  // Connect timeout
  options_.connTimeout = options.connTimeout;

  // Send timeout
  if (options.sendTimeout >= 0) {
    setSendTimeout(options.sendTimeout);
  }

  // Recv timeout
  if (options.recvTimeout >= 0) {
    setRecvTimeout(options.recvTimeout);
  }

  // Send Buffer Size
  if (options.sendBufSize > 0) {
    setSendBufSize(options.sendBufSize);
  }

  // Recv Buffer Size
  if (options.recvBufSize > 0) {
    setRecvBufSize(options.recvBufSize);
  }

  // Linger
  setLinger(options.lingerOn, options.lingerVal);

  // No delay
  setNoDelay(options.noDelay);

  setReuseAddress(options.reuseAddr);
}

TSocket::Options TSocket::getSocketOptions() {
  return options_;
}

TSocket::Options TSocket::getCurrentSocketOptions() {
  TSocket::Options ro;
  socklen_t ol;
  socklen_t* optlen = &ol;
  struct timeval s;
  size_t bufSize;
  struct linger l;
  int ret = 0;

  // ConnTimeout
  ro.connTimeout = options_.connTimeout;

  // sendTimeout
  s.tv_sec = s.tv_usec = 0;
  ret = 0;
  *optlen = sizeof(s);
  ret = getsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &s, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::SendTimeout getsockopt() " + getSocketInfo(), errno_copy);
  } else {
    ro.sendTimeout = msTimeFromTimeval(s);
  }

  // recvTimeout
  s.tv_sec = s.tv_usec = 0;
  ret = 0;
  *optlen = sizeof(s);
  ret = getsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &s, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::RecvTimeout getsockopt() " + getSocketInfo(), errno_copy);
  } else {
    ro.recvTimeout = msTimeFromTimeval(s);
  }

  // sendBufSize
  ret = 0;
  bufSize = 0;
  *optlen = sizeof(bufSize);
  ret = getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &bufSize, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::getSendBufSize() setsockopt() " + getSocketInfo(),
        errno_copy);
  } else {
    ro.sendBufSize = bufSize;
  }

  // recvBufSize
  ret = 0;
  bufSize = 0;
  *optlen = sizeof(bufSize);
  ret = getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &bufSize, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::getRecvBufSize() setsockopt() " + getSocketInfo(),
        errno_copy);
  } else {
    ro.recvBufSize = bufSize;
  }

  // linger
  ret = 0;
  *optlen = sizeof(l);
  ret = getsockopt(socket_, SOL_SOCKET, SO_LINGER, &l, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::getLinger() setsockopt() " + getSocketInfo(), errno_copy);
  } else {
    ro.lingerOn = l.l_onoff;
    ro.lingerVal = l.l_linger;
  }

  // NODELAY
  int v = 0;
  ret = 0;
  *optlen = sizeof(v);
  ret = getsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, &v, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::getNoDelay() setsockopt() " + getSocketInfo(), errno_copy);
  } else {
    ro.noDelay = v;
  }

  // REUSEADDR
  v = 0;
  ret = 0;
  *optlen = sizeof(v);
  ret = getsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &v, optlen);
  if (ret == -1) {
    int errno_copy = errno; // Copy errno because we're allocating memory.
    GlobalOutput.perror(
        "TSocket::getCurrentSocketOptions() SO_REUSEADDR " + getSocketInfo(),
        errno_copy);
  } else {
    ro.reuseAddr = v;
  }

  return ro;
}

void TSocket::setLinger(bool on, int linger) {
  if (socket_ >= 0) {
    struct linger l = {(on ? 1 : 0), linger};
    int ret = setsockopt(socket_, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setLinger() setsockopt() " + getSocketInfo(), errno_copy);
      return;
    }
  }

  options_.lingerOn = on;
  options_.lingerVal = linger;
}

void TSocket::setNoDelay(bool noDelay) {
  if (socket_ >= 0 && path_.empty()) {
    // Set socket to NODELAY
    int v = noDelay ? 1 : 0;
    int ret = setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setNoDelay() setsockopt() " + getSocketInfo(), errno_copy);
      return;
    }
  }

  options_.noDelay = noDelay;
}

void TSocket::setConnTimeout(int ms) {
  options_.connTimeout = ms;
}

void TSocket::setRecvTimeout(int ms) {
  if (ms < 0) {
    char errBuf[512];
    sprintf(errBuf, "TSocket::setRecvTimeout with negative input: %d\n", ms);
    GlobalOutput(errBuf);
    return;
  }

  if (socket_ >= 0) {
    struct timeval r = {(int)(ms / 1000), (int)((ms % 1000) * 1000)};
    int ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setRecvTimeout() setsockopt() " + getSocketInfo(),
          errno_copy);
      return;
    }
  }

  options_.recvTimeout = ms;
}

void TSocket::setSendBufSize(size_t bufsize) {
  if (isOpen()) {
    // It is undesirable to reduce the send buffer at runtime.
    // The kernel may prevent the applicaton from sending new data
    // as it reduces the buffer
    if (bufsize < options_.sendBufSize) {
      GlobalOutput.printf(
          "Error cannot reduce send buffer size of \
          open socket old: %zu new: %zu",
          options_.sendBufSize,
          bufsize);
      return;
    }

    int ret =
        setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setSendBufSize() setsockopt() " + getSocketInfo(),
          errno_copy);
      return;
    }
  }

  options_.sendBufSize = bufsize;
}

void TSocket::setRecvBufSize(size_t bufsize) {
  if (isOpen()) {
    // It is very undesirable to decrease buffer after the socket is open
    // if we shrink the buffer usage, the TCP connection
    // needs to throw away some buffered packets and "renegade"
    // the advertised receiving window it announced to the sender.
    // This will cause heavy retransmission from the sender.
    if (bufsize < options_.recvBufSize) {
      GlobalOutput.printf(
          "Error cannot reduce Recv buffer size of \
          open socket old: %zu new: %zu",
          options_.recvBufSize,
          bufsize);
      return;
    }

    int ret =
        setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setRecvBufSize() setsockopt() " + getSocketInfo(),
          errno_copy);
      return;
    }
  }

  options_.recvBufSize = bufsize;
}

void TSocket::setSendTimeout(int ms) {
  if (ms < 0) {
    char errBuf[512];
    sprintf(errBuf, "TSocket::setSendTimeout with negative input: %d\n", ms);
    GlobalOutput(errBuf);
    return;
  }

  if (socket_ >= 0) {
    struct timeval s = {(int)(ms / 1000), (int)((ms % 1000) * 1000)};
    int ret = setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &s, sizeof(s));
    if (ret == -1) {
      int errno_copy = errno; // Copy errno because we're allocating memory.
      GlobalOutput.perror(
          "TSocket::setSendTimeout() setsockopt() " + getSocketInfo(),
          errno_copy);
      return;
    }
  }

  options_.sendTimeout = ms;
}

void TSocket::setMaxRecvRetries(int maxRecvRetries) {
  maxRecvRetries_ = maxRecvRetries;
}

void TSocket::setReuseAddress(bool reuseAddr) {
  // Don't bother setting on the open socket since the option has no effect
  // on an open socket.
  options_.reuseAddr = reuseAddr;
}

// get socket info, but only if we have a socket
string TSocket::maybeGetSocketInfo() {
  if (socket_ < 0) {
    return "(closed)";
  } else {
    return getSocketInfo();
  }
}

string TSocket::getSocketInfo() {
  std::ostringstream oss;
  if (host_.empty() || port_ == 0) {
    oss << "<Host: " << getPeerAddressStr();
    oss << " Port: " << getPeerPort() << ">";
  } else {
    oss << "<Host: " << host_ << " Port: " << port_ << ">";
  }
  return oss.str();
}

const folly::SocketAddress* TSocket::getPeerAddress() {
  if (socket_ < 0) {
    throw TTransportException(
        TTransportException::NOT_OPEN,
        "attempted to get address of a non-open TSocket");
  }

  if (!cachedPeerAddr_.isInitialized()) {
    cachedPeerAddr_.setFromPeerAddress(folly::NetworkSocket::fromFd(socket_));
  }

  return &cachedPeerAddr_;
}

std::string TSocket::getPeerHost() {
  if (peerHost_.empty() && path_.empty()) {
    peerHost_ = getPeerAddress()->getHostStr();
  }
  return peerHost_;
}

std::string TSocket::getPeerAddressStr() {
  if (peerAddressStr_.empty() && path_.empty()) {
    peerAddressStr_ = getPeerAddress()->getAddressStr();
  }
  return peerAddressStr_;
}

uint16_t TSocket::getPeerPort() {
  return getPeerAddress()->getPort();
}

void TSocket::setCachedAddress(const sockaddr* addr, socklen_t len) {
  if (path_.empty()) {
    cachedPeerAddr_.setFromSockaddr(addr, len);
  }
}

bool TSocket::useLowMinRto_ = false;
void TSocket::setUseLowMinRto(bool useLowMinRto) {
  useLowMinRto_ = useLowMinRto;
}
bool TSocket::getUseLowMinRto() {
  return useLowMinRto_;
}

} // namespace transport
} // namespace thrift
} // namespace apache
