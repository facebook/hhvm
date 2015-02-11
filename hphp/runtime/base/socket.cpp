/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <fcntl.h>
#include <poll.h>

#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct SocketState final : RequestEventHandler {
  SocketState() : m_lastErrno(0) {}
  void clear() { m_lastErrno = 0; }
  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }
  int m_lastErrno;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(SocketState, s_socket_state);

///////////////////////////////////////////////////////////////////////////////
// constructors and destructor

SocketData::SocketData(
  int port,
  int type
) : FileData(true), m_port(port), m_type(type) {
}

bool SocketData::closeImpl() {
  s_pcloseRet = 0;
  if (valid() && !isClosed()) {
    s_pcloseRet = ::close(getFd());
    setIsClosed(true);
    setFd(-1);
  }
  return FileData::closeImpl();
}

SocketData::~SocketData() {
  SocketData::closeImpl();
}

Socket::Socket()
: File(std::make_shared<SocketData>(0, -1)),
  m_data(static_cast<SocketData*>(getFileData()))
{
}

Socket::Socket(std::shared_ptr<SocketData> data)
: File(data),
  m_data(static_cast<SocketData*>(getFileData()))
{
  assert(data);
  inferStreamType();
}

Socket::Socket(std::shared_ptr<SocketData> data,
               int sockfd,
               int type,
               const char *address /* = NULL */,
               int port /* = 0 */,
               double timeout /* = 0 */,
               const StaticString& streamType /* = empty_string_ref */)
: File(data, null_string, streamType),
  m_data(data.get())
{
  if (address) m_data->m_address = address;
  setFd(sockfd);

  struct timeval tv;
  if (timeout <= 0) {
    tv.tv_sec = ThreadInfo::s_threadInfo.getNoCheck()->
      m_reqInjectionData.getSocketDefaultTimeout();
    tv.tv_usec = 0;
  } else {
    tv.tv_sec = (int)timeout;
    tv.tv_usec = (timeout - tv.tv_sec) * 1e6;
  }
  setsockopt(getFd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  s_socket_state->m_lastErrno = errno;
  setsockopt(getFd(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  s_socket_state->m_lastErrno = errno;
  setTimeout(tv);
  setIsLocal(type == AF_UNIX);

  // Attempt to infer stream type only if it was not explicitly specified.
  inferStreamType();
}

Socket::Socket(int sockfd, int type, const char *address /* = NULL */,
               int port /* = 0 */, double timeout /* = 0 */,
               const StaticString& streamType /* = empty_string_ref */)
: Socket(
    std::make_shared<SocketData>(port, type),
    sockfd, type, address, port, timeout, streamType)
{ }

Socket::~Socket() { }

void Socket::sweep() {
  Socket::closeImpl();
  File::sweep();
  m_data = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

void Socket::setError(int err) {
  s_socket_state->m_lastErrno = m_data->m_error = err;
}

int Socket::getLastError() {
  return s_socket_state->m_lastErrno;
}

bool Socket::open(const String& filename, const String& mode) {
  throw_not_supported(__func__, "cannot open socket this way");
}

bool Socket::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

void Socket::setTimeout(struct timeval &tv) {
  if (tv.tv_sec >= 0 && tv.tv_usec >= 0) {
    m_data->m_timeout = tv.tv_sec * 1000000 + tv.tv_usec;
  } else {
    m_data->m_timeout = 0;
  }
}

bool Socket::checkLiveness() {
  if (getFd() == -1 || m_data->m_timedOut) {
    return false;
  }

  pollfd p;
  p.fd = getFd();
  p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
  p.revents = 0;
  if (poll(&p, 1, 0) > 0 && p.revents > 0) {
    char buf;
    int64_t ret = recv(getFd(), &buf, sizeof(buf), MSG_PEEK);
    if (ret == 0 || (ret == -1 && errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR)) {
      return false;
    }
  }

  return true;
}

bool Socket::setBlocking(bool blocking) {
  int flags = fcntl(getFd(), F_GETFL, 0);
  if (blocking) {
    flags &= ~O_NONBLOCK;
  } else {
    flags |= O_NONBLOCK;
  }
  return fcntl(getFd(), F_SETFL, flags) != -1;
}

bool Socket::waitForData() {
  m_data->m_timedOut = false;
  while (true) {
    struct pollfd fds[1];
    fds[0].fd = getFd();
    fds[0].events = POLLIN|POLLERR|POLLHUP;
    if (poll(fds, 1, m_data->m_timeout / 1000)) {
      socklen_t lon = sizeof(int);
      int valopt;
      getsockopt(getFd(), SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
      if (valopt == EINTR) continue;
      return valopt == 0;
    } else {
      m_data->m_timedOut = true;
      return true;
    }
  }
  return false;
}

int64_t Socket::readImpl(char *buffer, int64_t length) {
  assert(getFd());
  assert(length > 0);

  IOStatusHelper io("socket::recv", m_data->m_address.c_str(), m_data->m_port);

  int recvFlags = 0;
  if (m_data->m_timeout > 0) {
    int flags = fcntl(getFd(), F_GETFL, 0);
    if ((flags & O_NONBLOCK) == 0) {
      if (!waitForData()) {
        setEof(true);
        return 0;
      }
      recvFlags = MSG_DONTWAIT; // polled, so no need to wait any more
    }
  }

  int64_t ret = recv(getFd(), buffer, length, recvFlags);
  if (ret == 0 || (ret == -1 && errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR)) {
    setEof(true);
  }
  return (ret < 0) ? 0 : ret;
}

int64_t Socket::writeImpl(const char *buffer, int64_t length) {
  assert(getFd());
  assert(length > 0);
  setEof(false);
  IOStatusHelper io("socket::send", m_data->m_address.c_str(), m_data->m_port);
  int64_t ret = send(getFd(), buffer, length, 0);
  if (ret >= 0) {
    m_data->m_bytesSent += ret;
  }
  return ret;
}

bool Socket::eof() {
  if (!getEof() && valid() && bufferedLen() == 0) {
    // Test if stream is EOF if the flag is not already set.
    // Attempt to peek at one byte from the stream, checking for:
    // i)  recv() closing gracefully, or
    // ii) recv() failed due to no waiting data on non-blocking socket.
    char ch;
    int64_t ret = recv(getFd(), &ch, 1, MSG_PEEK | MSG_DONTWAIT);
    if (ret == 0 || (ret == -1 && errno != EWOULDBLOCK && errno != EAGAIN && errno != EINTR)) {
      setEof(true);
    }
  }
  return getEof();
}

const StaticString
  s_timed_out("timed_out"),
  s_blocked("blocked");

Array Socket::getMetaData() {
  Array ret = File::getMetaData();
  ret.set(s_timed_out, m_data->m_timedOut);
  ret.set(s_blocked, (bool)(fcntl(getFd(), F_GETFL, 0) & O_NONBLOCK));
  return ret;
}

int64_t Socket::tell() {
  return 0;
}

// Tries to infer stream type based on getsockopt() values.
// If no conclusive match can be found, leaves m_streamType
// as its default value of empty_string_ref.
void Socket::inferStreamType() {
  if (empty_string() == getStreamType()) {
    int result, type;
    socklen_t len = sizeof(type);
    result = getsockopt(getFd(), SOL_SOCKET, SO_TYPE, &type, &len);
    if (result != 0) {
      // getsockopt error.
      // Nothing to do here because the default stream type is empty_string_ref.
      return;
    }

    if (m_data->m_type == AF_INET || m_data->m_type == AF_INET6) {
      if (type == SOCK_STREAM) {
        if (RuntimeOption::EnableHipHopSyntax) {
          setStreamType(StaticString("tcp_socket"));
        } else {
          // Note: PHP returns "tcp_socket/ssl" for this query,
          // even though the socket is clearly not an SSL socket.
          setStreamType(StaticString("tcp_socket/ssl"));
        }
      } else if (type == SOCK_DGRAM) {
        setStreamType(StaticString("udp_socket"));
      }
    } else if (m_data->m_type == AF_UNIX) {
      if (type == SOCK_STREAM) {
        setStreamType(StaticString("unix_socket"));
      } else if (type == SOCK_DGRAM) {
        setStreamType(StaticString("udg_socket"));
      }
    }

    // Nothing to do in default case because the default stream type is
    // empty_string_ref.
  }
}

///////////////////////////////////////////////////////////////////////////////
}
