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
#include "hphp/util/afdt-util.h"
#include <afdt.h>
#include <errno.h>
#include <limits.h>

namespace HPHP {
namespace afdt {
///////////////////////////////////////////////////////////////////////////////

namespace detail {

namespace {
void append(std::vector<iovec>& iov, const void* addr, size_t size) {
  iovec io;
  io.iov_base = const_cast<void*>(addr);
  io.iov_len = size;
  iov.push_back(io);
}
}

void sappend(int afdt_fd,
             std::vector<iovec>& iov, const void* addr, size_t size) {
  if (iov.size() == IOV_MAX) {
    send(afdt_fd, iov);
    iov.clear();
  }
  append(iov, addr, size);
}

void rappend(int afdt_fd,
             std::vector<iovec>& iov, void* addr, size_t size) {
  if (iov.size() == IOV_MAX) {
    recv(afdt_fd, iov);
    iov.clear();
  }
  append(iov, addr, size);
}

void send(int afdt_fd, std::vector<iovec>& iov) {
  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &iov[0];
  msg.msg_iovlen = iov.size();
  ssize_t nwritten = sendmsg(afdt_fd, &msg, 0);
  if (nwritten < 0) throw std::runtime_error("send failed");
  for (auto& io : iov) {
    nwritten -= io.iov_len;
  }
  if (nwritten) throw std::runtime_error("sent wrong number of bytes");
}

void recv(int afdt_fd, std::vector<iovec>& iov) {
  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &iov[0];
  msg.msg_iovlen = iov.size();
  ssize_t nread = recvmsg(afdt_fd, &msg, 0);
  if (nread <= 0) throw std::runtime_error("recv failed");
  for (auto& io : iov) {
    nread -= io.iov_len;
  }
  if (nread) throw std::runtime_error("recv received wrong number of bytes");
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
