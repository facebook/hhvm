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
#include "thrift/lib/cpp/transport/TSocket.h"

#include "thrift/lib/cpp/config.h"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "thrift/lib/cpp/concurrency/Monitor.h"
#include "thrift/lib/cpp/transport/TTransportException.h"

namespace apache { namespace thrift { namespace transport {

using namespace std;

// Global var to track total socket sys calls
uint32_t g_socket_syscalls = 0;

// Global helper functions

int msTimeFromTimeval(struct timeval s) {
  return s.tv_sec*1000 + s.tv_usec/1000;
}

ostream& operator<<(ostream& os, const TSocket::Options& o) {
  os << "SOCKET OPTIONS"  << "\n";
  os << "connTimeout = " << o.connTimeout << "\n";
  os << "sendTimeout = " << o.sendTimeout << "\n";
  os << "recvTimeout = " << o.recvTimeout << "\n";
  os << "sendBufSize = " << o.sendBufSize << "\n";
  os << "recvBufSize = " << o.recvBufSize << "\n";
  os << "lingerOn    = " << o.lingerOn    << "\n";
  os << "lingerVal   = " << o.lingerVal   << "\n";
  os << "noDelay     = " << o.noDelay     << "\n";
  os << "reuseAddr   = " << o.reuseAddr   << "\n";
  return os;
}

/**
 * TSocket implementation.
 *
 */

TSocket::TSocket(string host, int port) :
  host_(host),
  port_(port),
  socket_(-1),
  maxRecvRetries_(5) {
}

TSocket::TSocket(const TSocketAddress* address) :
  host_(address->getAddressStr()),
  port_(address->getPort()),
  socket_(-1),
  maxRecvRetries_(5) {
  // For convenience with the existing code that resolves hostnames,
  // we just store the IP address as a string in host_, and convert it back to
  // an address in connect()
}

TSocket::TSocket(const TSocketAddress& address) :
  host_(address.getAddressStr()),
  port_(address.getPort()),
  socket_(-1),
  maxRecvRetries_(5) {
  // For convenience with the existing code that resolves hostnames,
  // we just store the IP address as a string in host_, and convert it back to
  // an address in connect()
}

TSocket::TSocket() :
  host_(""),
  port_(0),
  socket_(-1),
  maxRecvRetries_(5) {
}

TSocket::TSocket(int socket) :
  host_(""),
  port_(0),
  socket_(socket),
  maxRecvRetries_(5) {
}

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
  int r = recv(socket_, &buf, 1, MSG_PEEK);
  if (r == -1) {
    int errno_copy = errno;
    #if defined __FreeBSD__ || defined __MACH__
    /* shigin:
     * freebsd returns -1 and ECONNRESET if socket was closed by
     * the other side
     */
    if (errno_copy == ECONNRESET)
    {
      close();
      return false;
    }
    #endif
    GlobalOutput.perror("TSocket::peek() recv() " + getSocketInfo(), errno_copy);
    throw TTransportException(TTransportException::UNKNOWN, "recv()", errno_copy);
  }
  return (r > 0);
}

void TSocket::openConnection(struct addrinfo *res) {
  if (isOpen()) {
    throw TTransportException(TTransportException::ALREADY_OPEN);
  }

  socket_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_ == -1) {
    int errno_copy = errno;
    GlobalOutput.perror("TSocket::open() socket() " + getSocketInfo(), errno_copy);
    throw TTransportException(TTransportException::NOT_OPEN, "socket()", errno_copy);
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
      GlobalOutput.perror("TSocket::open() fcntl() " + getSocketInfo(), errno_copy);
      throw TTransportException(TTransportException::NOT_OPEN, "fcntl() failed", errno_copy);
    }
  } else {
    if (-1 == fcntl(socket_, F_SETFL, flags & ~O_NONBLOCK)) {
      int errno_copy = errno;
      GlobalOutput.perror("TSocket::open() fcntl " + getSocketInfo(), errno_copy);
      throw TTransportException(TTransportException::NOT_OPEN, "fcntl() failed", errno_copy);
    }
  }

  if (options_.reuseAddr) {
    int val = 1;
    if (-1 == setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &val,
                         sizeof(val))) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::open() setsockopt(SO_REUSEADDR) "
                          + getSocketInfo(), errno_copy);
      // No need to throw.
    }
  }

  // Connect the socket
  int ret = connect(socket_, res->ai_addr, res->ai_addrlen);

  // success case
  if (ret == 0) {
    goto done;
  }

  if (errno != EINPROGRESS) {
    int errno_copy = errno;
    GlobalOutput.perror("TSocket::open() connect() " + getSocketInfo(), errno_copy);
    throw TTransportException(TTransportException::NOT_OPEN, "connect() failed "
                              + getSocketInfo(), errno_copy);
  }


  struct pollfd fds[1];
  std::memset(fds, 0 , sizeof(fds));
  fds[0].fd = socket_;
  fds[0].events = POLLOUT;
  ret = poll(fds, 1, options_.connTimeout);

  if (ret > 0) {
    // Ensure the socket is connected and that there are no errors set
    int val;
    socklen_t lon;
    lon = sizeof(int);
    int ret2 = getsockopt(socket_, SOL_SOCKET, SO_ERROR, (void *)&val, &lon);
    if (ret2 == -1) {
      int errno_copy = errno;
      GlobalOutput.perror("TSocket::open() getsockopt() " + getSocketInfo(), errno_copy);
      throw TTransportException(TTransportException::NOT_OPEN, "getsockopt()", errno_copy);
    }
    // no errors on socket, go to town
    if (val == 0) {
      goto done;
    }
    GlobalOutput.perror("TSocket::open() error on socket (after poll) " + getSocketInfo(), val);
    throw TTransportException(TTransportException::NOT_OPEN, "socket open() error", val);
  } else if (ret == 0) {
    // socket timed out
    string errStr = "TSocket::open() timed out " + getSocketInfo();
    GlobalOutput(errStr.c_str());
    throw TTransportException(TTransportException::NOT_OPEN,
                              "open() timed out " + getSocketInfo());
  } else {
    // error on poll()
    int errno_copy = errno;
    GlobalOutput.perror("TSocket::open() poll() " + getSocketInfo(), errno_copy);
    throw TTransportException(TTransportException::NOT_OPEN, "poll() failed", errno_copy);
  }

 done:
  // Set socket back to normal mode (blocking)
  fcntl(socket_, F_SETFL, flags);

  setCachedAddress(res->ai_addr, res->ai_addrlen);
}

void TSocket::open() {
  if (isOpen()) {
    throw TTransportException(TTransportException::ALREADY_OPEN);
  }

  // Validate port number
  if (port_ < 0 || port_ > 0xFFFF) {
    throw TTransportException(TTransportException::NOT_OPEN, "Specified port is invalid");
  }

  struct addrinfo hints, *res, *res0;
  res = nullptr;
  res0 = nullptr;
  int error;
  char port[sizeof("65535")];
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  sprintf(port, "%d", port_);

  error = getaddrinfo(host_.c_str(), port, &hints, &res0);

  if (error) {
    string errStr = "TSocket::open() getaddrinfo() " + getSocketInfo() + string(gai_strerror(error));
    GlobalOutput(errStr.c_str());
    close();
    throw TTransportException(TTransportException::NOT_OPEN,
                              "Could not resolve host for client socket " +
                              getSocketInfo());
  }

  // Cycle through all the returned addresses until one
  // connects or push the exception up.
  for (res = res0; res; res = res->ai_next) {
    try {
      openConnection(res);
      break;
    } catch (TTransportException& ttx) {
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
}

void TSocket::setSocketFD(int socket) {
  if (socket_ >= 0) {
    close();
  }
  socket_ = socket;
}

uint32_t TSocket::read(uint8_t* buf, uint32_t len) {
  if (socket_ < 0) {
    throw TTransportException(TTransportException::NOT_OPEN, "Called read on non-open socket");
  }

  int32_t retries = 0;

  // EAGAIN can be signaled both when a timeout has occurred and when
  // the system is out of resources (an awesome undocumented feature).
  // The following is an approximation of the time interval under which
  // EAGAIN is taken to indicate an out of resources error.
  uint32_t eagainThresholdMicros = 0;
  if (options_.recvTimeout) {
    // if a readTimeout is specified along with a max number of recv retries, then
    // the threshold will ensure that the read timeout is not exceeded even in the
    // case of resource errors
    eagainThresholdMicros = (options_.recvTimeout*1000)/ ((maxRecvRetries_>0) ?
        maxRecvRetries_ : 2);
  }

  // When there is a recv timeout set, an EINTR will restart the
  // recv() and hence reset the timer.  So if we receive EINTRs at a
  // faster rate than the timeout value, the timeout will never
  // trigger.  Therefore, we keep track of the total amount of time
  // we've spend in recv(), and if this value exceeds the timeout then
  // we stop retrying on EINTR.  Note that we might still exceed the
  // timeout, but by at most a factor of 2.
  struct timeval waited;        // total time waiting so far
  waited.tv_sec = waited.tv_usec = 0;

 try_again:
  // Read from the socket
  struct timeval begin;
  if (options_.recvTimeout > 0) {
    gettimeofday(&begin, nullptr);
  } else {
    // if there is no read timeout we don't need the TOD to determine whether
    // an EAGAIN is due to a timeout or an out-of-resource condition.
    begin.tv_sec = begin.tv_usec = 0;
  }
  int got = recv(socket_, buf, len, 0);
  ++g_socket_syscalls;
  int errno_copy = errno; //gettimeofday can change errno

  // Check for error on read
  if (got < 0) {
    if (errno_copy == EAGAIN) {
      // if no timeout we can assume that resource exhaustion has occurred.
      if (options_.recvTimeout == 0) {
        throw TTransportException(TTransportException::TIMED_OUT,
                                    "EAGAIN (unavailable resources)");
      }
      // check if this is the lack of resources or timeout case
      struct timeval end;
      gettimeofday(&end, nullptr);
      uint32_t readElapsedMicros =  (((end.tv_sec - begin.tv_sec) * 1000 * 1000)
                                     + (((uint64_t)(end.tv_usec - begin.tv_usec))));
      if (!eagainThresholdMicros || (readElapsedMicros < eagainThresholdMicros)) {
        if (retries++ < maxRecvRetries_) {
          usleep(50);
          goto try_again;
        } else {
          throw TTransportException(TTransportException::TIMED_OUT,
                                    "EAGAIN (unavailable resources)");
        }
      } else {
        // infer that timeout has been hit
        throw TTransportException(TTransportException::TIMED_OUT,
                                  "EAGAIN (timed out) " + getSocketInfo());
      }
    }

    // If interrupted, try again, but only if we haven't exceeded the
    // timeout value yet.
    if (errno_copy == EINTR) {
      if (options_.recvTimeout == 0) {
        goto try_again;
      } else {
        // waited += (end - begin);
        struct timeval end;
        struct timeval duration;
        gettimeofday(&end, nullptr);
        timersub(&end, &begin, &duration);
        timeradd(&duration, &waited, &waited);

        struct timeval timeout = {
          static_cast<int>(options_.recvTimeout/1000),
          static_cast<int>((options_.recvTimeout%1000)*1000)
        };
        // if (waited < timeout) ...
        if (timercmp(&waited, &timeout, <)) {
          goto try_again;
        } else {
          // Exceeded the timeout value, this is a real retry now.
          if (retries++ < maxRecvRetries_) {
            // Reset the total waiting time
            waited.tv_sec = waited.tv_usec = 0;
            goto try_again;
          } else {
            throw TTransportException(TTransportException::TIMED_OUT,
                                      "EINTR (timed out)");
          }
        }
      }
    }

    #if defined __FreeBSD__ || defined __MACH__
    if (errno_copy == ECONNRESET) {
      /* shigin: freebsd doesn't follow POSIX semantic of recv and fails with
       * ECONNRESET if peer performed shutdown
       * edhall: eliminated close() since we do that in the destructor.
       */
      return 0;
    }
    #endif

    // Now it's not a try again case, but a real probblez
    GlobalOutput.perror("TSocket::read() recv() " + getSocketInfo(), errno_copy);

    // If we disconnect with no linger time
    if (errno_copy == ECONNRESET) {
      throw TTransportException(TTransportException::NOT_OPEN,
                                "ECONNRESET " + getSocketInfo());
    }

    // This ish isn't open
    if (errno_copy == ENOTCONN) {
      throw TTransportException(TTransportException::NOT_OPEN,
                                "ENOTCONN " + getSocketInfo());
    }

    // Timed out!
    if (errno_copy == ETIMEDOUT) {
      throw TTransportException(TTransportException::TIMED_OUT,
                                "ETIMEDOUT " + getSocketInfo());
    }

    // Some other error, whatevz
    throw TTransportException(TTransportException::UNKNOWN,
                              "Unknown " + getSocketInfo(), errno_copy);
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
      throw TTransportException(TTransportException::TIMED_OUT,
                                "send timeout expired " + getSocketInfo());
    }
    sent += b;
  }
}

uint32_t TSocket::write_partial(const uint8_t* buf, uint32_t len) {
  if (socket_ < 0) {
    throw TTransportException(TTransportException::NOT_OPEN, "Called write on non-open socket");
  }

  uint32_t sent = 0;

  int flags = 0;
#ifdef MSG_NOSIGNAL
  // Note the use of MSG_NOSIGNAL to suppress SIGPIPE errors, instead we
  // check for the EPIPE return condition and close the socket in that case
  flags |= MSG_NOSIGNAL;
#endif // ifdef MSG_NOSIGNAL

  int b = send(socket_, buf + sent, len - sent, flags);
  ++g_socket_syscalls;

  if (b < 0) {
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
      return 0;
    }
    // Fail on a send error
    int errno_copy = errno;
    GlobalOutput.perror("TSocket::write_partial() send() " + getSocketInfo(), errno_copy);

    if (errno_copy == EPIPE || errno_copy == ECONNRESET || errno_copy == ENOTCONN) {
      close();
      throw TTransportException(TTransportException::NOT_OPEN,
                               "write() send() " + getSocketInfo(), errno_copy);
    }

    throw TTransportException(TTransportException::UNKNOWN,
                              "write() send() " + getSocketInfo(), errno_copy);
  }

  // Fail on blocked send
  if (b == 0) {
    throw TTransportException(TTransportException::NOT_OPEN, "Socket send returned 0.");
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
  socklen_t *optlen = &ol;
  struct timeval s;
  size_t bufSize;
  struct linger l;
  int ret = 0;
  int errno_copy;

  // ConnTimeout
  ro.connTimeout = options_.connTimeout;

  //sendTimeout
  s.tv_sec = s.tv_usec = 0;
  ret = 0;
  *optlen = sizeof(s);
  ret = getsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &s, optlen);
  if (ret == -1) {
    errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::SendTimeout getsockopt() " +
        getSocketInfo(), errno_copy);
  } else {
    ro.sendTimeout = msTimeFromTimeval(s);
  }

  // recvTimeout
  s.tv_sec = s.tv_usec = 0;
  ret = 0;
  *optlen = sizeof(s);
  ret = getsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &s, optlen);
  if (ret == -1) {
    errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::RecvTimeout getsockopt() " +
        getSocketInfo(), errno_copy);
  } else {
    ro.recvTimeout = msTimeFromTimeval(s);
  }

  // sendBufSize
  ret = 0;
  bufSize = 0;
  *optlen = sizeof (bufSize);
  ret = getsockopt(socket_, SOL_SOCKET, SO_SNDBUF,
      &bufSize, optlen);
  if (ret == -1) {
    int errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::getSendBufSize() setsockopt() "
        + getSocketInfo(), errno_copy);
  } else {
    ro.sendBufSize = bufSize;
  }

  // recvBufSize
  ret = 0;
  bufSize = 0;
  *optlen = sizeof (bufSize);
  ret = getsockopt(socket_, SOL_SOCKET, SO_RCVBUF,
      &bufSize, optlen);
  if (ret == -1) {
    int errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::getRecvBufSize() setsockopt() "
        + getSocketInfo(), errno_copy);
  } else {
    ro.recvBufSize = bufSize;
  }

  // linger
  ret = 0;
  *optlen = sizeof (l);
  ret = getsockopt(socket_, SOL_SOCKET, SO_LINGER, &l, optlen);
  if (ret == -1) {
    int errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::getLinger() setsockopt() "
        + getSocketInfo(), errno_copy);
  } else {
    ro.lingerOn = l.l_onoff;
    ro.lingerVal = l.l_linger;
  }

  // NODELAY
  int v = 0;
  ret = 0;
  *optlen = sizeof (v);
  ret = getsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, &v, optlen);
  if (ret == -1) {
    int errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::getNoDelay() setsockopt() "
        + getSocketInfo(), errno_copy);
  } else {
    ro.noDelay = v;
  }

  // REUSEADDR
  v = 0;
  ret = 0;
  *optlen = sizeof(v);
  ret = getsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &v, optlen);
  if (ret == -1) {
    int errno_copy = errno;  // Copy errno because we're allocating memory.
    GlobalOutput.perror("TSocket::getCurrentSocketOptions() SO_REUSEADDR "
                        + getSocketInfo(), errno_copy);
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
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setLinger() setsockopt() " +
                          getSocketInfo(), errno_copy);
      return;
    }
  }

  options_.lingerOn = on;
  options_.lingerVal = linger;
}

void TSocket::setNoDelay(bool noDelay) {
  if (socket_ >= 0) {
    // Set socket to NODELAY
    int v = noDelay ? 1 : 0;
    int ret = setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
    if (ret == -1) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setNoDelay() setsockopt() " +
                          getSocketInfo(), errno_copy);
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
    struct timeval r = {(int)(ms/1000),
                        (int)((ms%1000)*1000)};
    int ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &r, sizeof(r));
    if (ret == -1) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setRecvTimeout() setsockopt() " +
                          getSocketInfo(), errno_copy);
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
      GlobalOutput.printf("Error cannot reduce send buffer size of \
          open socket old: %zu new: %zu",
          options_.sendBufSize, bufsize);
      return;
    }

    int ret = setsockopt(socket_, SOL_SOCKET, SO_SNDBUF,
        &bufsize, sizeof(bufsize));
    if (ret == -1) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setSendBufSize() setsockopt() "
          + getSocketInfo(), errno_copy);
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
      GlobalOutput.printf("Error cannot reduce Recv buffer size of \
          open socket old: %zu new: %zu",
          options_.recvBufSize, bufsize);
      return;
    }

    int ret = setsockopt(socket_, SOL_SOCKET, SO_RCVBUF,
        &bufsize, sizeof(bufsize));
    if (ret == -1) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setRecvBufSize() setsockopt() "
          + getSocketInfo(), errno_copy);
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
    struct timeval s = {(int)(ms/1000),
                        (int)((ms%1000)*1000)};
    int ret = setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &s, sizeof(s));
    if (ret == -1) {
      int errno_copy = errno;  // Copy errno because we're allocating memory.
      GlobalOutput.perror("TSocket::setSendTimeout() setsockopt() " +
                          getSocketInfo(), errno_copy);
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

const TSocketAddress* TSocket::getPeerAddress() {
  if (socket_ < 0) {
    throw TTransportException(TTransportException::NOT_OPEN,
                              "attempted to get address of a non-open TSocket");
  }

  if (!cachedPeerAddr_.isInitialized()) {
    cachedPeerAddr_.setFromPeerAddress(socket_);
  }

  return &cachedPeerAddr_;
}

std::string TSocket::getPeerHost() {
  if (peerHost_.empty()) {
    peerHost_ = getPeerAddress()->getHostStr();
  }
  return peerHost_;
}

std::string TSocket::getPeerAddressStr() {
  if (peerAddressStr_.empty()) {
    peerAddressStr_ = getPeerAddress()->getAddressStr();
  }
  return peerAddressStr_;
}

uint16_t TSocket::getPeerPort() {
  return getPeerAddress()->getPort();
}

void TSocket::setCachedAddress(const sockaddr* addr, socklen_t len) {
  cachedPeerAddr_.setFromSockaddr(addr, len);
}

bool TSocket::useLowMinRto_ = false;
void TSocket::setUseLowMinRto(bool useLowMinRto) {
  useLowMinRto_ = useLowMinRto;
}
bool TSocket::getUseLowMinRto() {
  return useLowMinRto_;
}

}}} // apache::thrift::transport
