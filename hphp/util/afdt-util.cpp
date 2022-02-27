/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/util/afdt-util.h"
#include <afdt.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <system_error>

#include "hphp/util/assertions.h"

namespace HPHP {
namespace afdt {
///////////////////////////////////////////////////////////////////////////////

namespace detail {

namespace {
void append(std::vector<iovec>& iov, const void* addr, size_t size) {
  if (!size) return;

  iovec io;
  io.iov_base = const_cast<void*>(addr);
  io.iov_len = size;
  iov.push_back(io);
}
}

void sappend(std::vector<iovec>& iov, const void* addr, size_t size) {
  append(iov, addr, size);
}

void rappend(std::vector<iovec>& iov, void* addr, size_t size) {
  append(iov, addr, size);
}

namespace {
// Skip all fully read iovecs. This includes initially empty iovecs.
bool skip_processed(struct msghdr& msg, ssize_t nprocessed,
                    ssize_t& npending_iovs) {
  while (msg.msg_iovlen > 0 && msg.msg_iov[0].iov_len <= nprocessed) {
    nprocessed -= msg.msg_iov[0].iov_len;
    msg.msg_iov++;
    if (npending_iovs > 0) {
      npending_iovs--;
    } else {
      msg.msg_iovlen--;
    }
  }

  if (msg.msg_iovlen == 0) {
    // Nothing more to recv or send.
    always_assert(nprocessed == 0);
    return true;
  }

  reinterpret_cast<char*&>(msg.msg_iov[0].iov_base) += nprocessed;
  msg.msg_iov[0].iov_len -= nprocessed;
  return false;
}
}

void send(int afdt_fd, std::vector<iovec>& iov) {
  if (iov.size() == 0) return;

  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &iov[0];
  msg.msg_iovlen = std::min(iov.size(), size_t{IOV_MAX});

  ssize_t npending_iovs = iov.size() - msg.msg_iovlen;
  ssize_t nwritten = 0;

  while (true) {
    if (skip_processed(msg, nwritten, npending_iovs)) return;

    nwritten = sendmsg(afdt_fd, &msg, MSG_WAITALL);
    if (nwritten < 0) {
      if (errno == EINTR) {
        nwritten = 0;
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "send failed");
    }
  }
}

void recv(int afdt_fd, std::vector<iovec>& iov) {
  if (iov.size() == 0) return;

  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &iov[0];
  msg.msg_iovlen = std::min(iov.size(), size_t{IOV_MAX});

  ssize_t npending_iovs = iov.size() - msg.msg_iovlen;
  ssize_t nread = 0;

  while (true) {
    if (skip_processed(msg, nread, npending_iovs)) return;

    nread = recvmsg(afdt_fd, &msg, MSG_WAITALL);
    if (nread < 0) {
      if (errno == EINTR) {
        nread = 0;
        continue;
      }

      throw std::system_error(errno, std::generic_category(), "recv failed");
    }
  }
}

}

bool send_fd(int afdt_fd, int fd) {
  afdt_error_t err = AFDT_ERROR_T_INIT;
  errno = 0;
  int ret = afdt_send_fd_msg(afdt_fd, nullptr, 0, fd, &err);
  if (ret < 0 && errno == 0) {
    // Set non-empty errno if afdt_send_fd_msg doesn't set one on error
    errno = EPROTO;
  }
  return ret >= 0;
}

int recv_fd(int afdt_fd) {
#ifdef __APPLE__
  {
    errno = 0;
    int flags = fcntl(0, F_GETFD);
    always_assert(flags != -1 || errno != EBADF);
  }
#endif
  int fd;
  afdt_error_t err = AFDT_ERROR_T_INIT;
  uint32_t afdt_len = 0;
  errno = 0;
  if (afdt_recv_fd_msg(afdt_fd, nullptr, &afdt_len, &fd, &err) < 0) {
    if (errno == 0) {
      // Set non-empty errno if afdt_send_fd_msg doesn't set one on error
      errno = EPROTO;
    }
    return -1;
  }
  return fd;
}

///////////////////////////////////////////////////////////////////////////////
}}
