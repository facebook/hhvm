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

#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/util/logger.h"
#include <fcntl.h>
#include <poll.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SocketData : public RequestEventHandler {
public:
  SocketData() : m_lastErrno(0) {}
  void clear() { m_lastErrno = 0; }
  virtual void requestInit() {
    clear();
  }
  virtual void requestShutdown() {
    clear();
  }
  int m_lastErrno;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(SocketData, s_socket_data);

///////////////////////////////////////////////////////////////////////////////
// constructors and destructor

Socket::Socket()
  : File(true), m_port(0), m_type(-1), m_error(0), m_eof(false), m_timeout(0),
    m_timedOut(false), m_bytesSent(0) {
}

Socket::Socket(int sockfd, int type, const char *address /* = NULL */,
               int port /* = 0 */, double timeout /* = 0 */)
  : File(true), m_port(port), m_type(type), m_error(0), m_eof(false),
    m_timeout(0), m_timedOut(false), m_bytesSent(0) {
  if (address) m_address = address;
  m_fd = sockfd;

  struct timeval tv;
  if (timeout <= 0) {
    tv.tv_sec = RuntimeOption::SocketDefaultTimeout;
    tv.tv_usec = 0;
  } else {
    tv.tv_sec = (int)timeout;
    tv.tv_usec = (timeout - tv.tv_sec) * 1e6;
  }
  setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  s_socket_data->m_lastErrno = errno;
  setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  s_socket_data->m_lastErrno = errno;
  setTimeout(tv);
  m_isLocal = (type == AF_UNIX);
}

Socket::~Socket() {
  closeImpl();
}

void Socket::sweep() {
  Socket::closeImpl();
  File::sweep();
  Socket::operator delete(this);
}

///////////////////////////////////////////////////////////////////////////////

void Socket::setError(int err) {
  s_socket_data->m_lastErrno = m_error = err;
}

int Socket::getLastError() {
  return s_socket_data->m_lastErrno;
}

bool Socket::open(const String& filename, const String& mode) {
  throw NotSupportedException(__func__, "cannot open socket this way");
}

bool Socket::close() {
  return closeImpl();
}

bool Socket::closeImpl() {
  s_file_data->m_pcloseRet = 0;
  if (valid() && !m_closed) {
    s_file_data->m_pcloseRet = ::close(m_fd);
    m_closed = true;
    m_fd = -1;
  }
  File::closeImpl();
  return true;
}

void Socket::setTimeout(struct timeval &tv) {
  if (tv.tv_sec >= 0 && tv.tv_usec >= 0) {
    m_timeout = tv.tv_sec * 1000000 + tv.tv_usec;
  } else {
    m_timeout = 0;
  }
}

bool Socket::checkLiveness() {
  if (m_fd == -1 || m_timedOut) {
    return false;
  }

  pollfd p;
  p.fd = m_fd;
  p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
  p.revents = 0;
  if (poll(&p, 1, 0) > 0 && p.revents > 0) {
    char buf;
    if (0 == recv(m_fd, &buf, sizeof(buf), MSG_PEEK) &&
        errno != EAGAIN) {
      return false;
    }
  }

  return true;
}

bool Socket::setBlocking(bool blocking) {
  int flags = fcntl(m_fd, F_GETFL, 0);
  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  return fcntl(m_fd, F_SETFL, flags) != -1;
}

bool Socket::waitForData() {
  m_timedOut = false;
  while (true) {
    struct pollfd fds[1];
    fds[0].fd = m_fd;
    fds[0].events = POLLIN|POLLERR|POLLHUP;
    if (poll(fds, 1, m_timeout / 1000)) {
      socklen_t lon = sizeof(int);
      int valopt;
      getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
      if (valopt == EINTR) continue;
      return valopt == 0;
    } else {
      m_timedOut = true;
      return true;
    }
  }
  return false;
}

int64_t Socket::readImpl(char *buffer, int64_t length) {
  assert(m_fd);
  assert(length > 0);

  IOStatusHelper io("socket::recv", m_address.c_str(), m_port);

  int recvFlags = 0;
  if (m_timeout > 0) {
    int flags = fcntl(m_fd, F_GETFL, 0);
    if ((flags & O_NONBLOCK) == 0) {
      if (!waitForData()) {
        m_eof = true;
        return 0;
      }
      recvFlags = MSG_DONTWAIT; // polled, so no need to wait any more
    }
  }

  int64_t ret = recv(m_fd, buffer, length, recvFlags);
  if (ret == 0 || (ret == -1 && errno != EWOULDBLOCK)) {
    m_eof = true;
  }
  return (ret < 0) ? 0 : ret;
}

int64_t Socket::writeImpl(const char *buffer, int64_t length) {
  assert(m_fd);
  assert(length > 0);
  m_eof = false;
  IOStatusHelper io("socket::send", m_address.c_str(), m_port);
  int64_t ret = send(m_fd, buffer, length, 0);
  if (ret >= 0) {
    m_bytesSent += ret;
  }
  return ret;
}

bool Socket::eof() {
  return m_eof;
}

const StaticString
  s_timed_out("timed_out"),
  s_blocked("blocked");

Array Socket::getMetaData() {
  Array ret = File::getMetaData();
  ret.set(s_timed_out, m_timedOut);
  ret.set(s_blocked, (bool)(fcntl(m_fd, F_GETFL, 0) & O_NONBLOCK));
  return ret;
}

int64_t Socket::tell() {
  return m_bytesSent;
}

///////////////////////////////////////////////////////////////////////////////
}
