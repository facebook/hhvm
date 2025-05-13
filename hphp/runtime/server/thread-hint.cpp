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

#include <fcntl.h>
#include <bpf/bpf.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <folly/File.h>
#include <folly/SocketAddress.h>
#include <folly/net/NetworkSocket.h>
#include <folly/portability/Sockets.h>

#include "hphp/runtime/server/thread-hint.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(thread_sched)

namespace HPHP {

THREAD_LOCAL_NO_CHECK(ThreadHint::ThreadData, ThreadHint::s_threadData);

ThreadHint &ThreadHint::getInstance() {
  static ThreadHint instance;
  return instance;
}

ThreadHint::ThreadHint() {
  if (!Cfg::Server::ScxThreadHintUdsPath.empty()) {
    initHintMap(Cfg::Server::ScxThreadHintUdsPath);
  }
}

/**
 * Receive BPF map FD from UDS at the provided `path`. This circumvents some
 * container bpffs limitations when trying to open the map directly. It's a
 * temporary measure and will be replaced with more official support in a later
 * release.
 */
static int receiveFd(const std::string_view path) {
  folly::NetworkSocket sock(folly::netops::socket(AF_UNIX, SOCK_STREAM, 0));
  if (sock == folly::NetworkSocket()) {
    FTRACE(1, "ThreadHint: Failed to create socket: {}\n", folly::errnoStr(errno));
    return -1;
  }
  SCOPE_EXIT { folly::netops::close(sock); };

  folly::SocketAddress addr;
  try {
    addr.setFromPath(path);
    sockaddr_storage addrStorage{};
    socklen_t addrLen = addr.getAddress(&addrStorage);
    if (folly::netops::connect(sock, reinterpret_cast<struct sockaddr*>(&addrStorage), addrLen) == -1) {
      FTRACE(1, "ThreadHint: Failed to connect to {}: {}\n", path, folly::errnoStr(errno));
      return -1;
    }
  } catch (const std::exception& ex) {
    FTRACE(1, "ThreadHint: Failed to set socket path {}: {}\n", path, ex.what());
    return -1;
  }

  char buf[CMSG_SPACE(sizeof(int))] = {0};
  char c;
  struct iovec iov = {
    .iov_base = &c,
    .iov_len = 1
  };

  struct msghdr msg = {
    .msg_iov = &iov,
    .msg_iovlen = 1,
    .msg_control = buf,
    .msg_controllen = sizeof(buf)
  };

  if (folly::netops::recvmsg(sock, &msg, 0) <= 0) {
    FTRACE(1, "ThreadHint: Failed to receive message: {}\n", folly::errnoStr(errno));
    return -1;
  }

  struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
  if (!cmsg || cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS) {
    FTRACE(1, "ThreadHint: No fd received\n");
    return -1;
  }

  return *((int*)CMSG_DATA(cmsg));
}

void ThreadHint::initHintMap(const std::string_view path) {
  FTRACE(1, "ThreadHint: Attempting to get fd from {}\n", path);
  int uds_fd = receiveFd(path);
  if (uds_fd < 0) {
    FTRACE(1, "ThreadHint: Failed to get fd from {}, errno={}\n", path, errno);
    return;
  }
  FTRACE(1, "ThreadHint: Successfully received fd {}\n", uds_fd);
  m_threadHint = folly::File(uds_fd, false);
}

static constexpr uint16_t getHint(ThreadHint::Priority priority) {
  switch (priority) {
  case ThreadHint::Priority::Idling:
    return Cfg::Server::ScxThreadHintIdle;
  case ThreadHint::Priority::Processing:
    return Cfg::Server::ScxThreadHintProcessing;
  case ThreadHint::Priority::PostProcessing:
    return Cfg::Server::ScxThreadHintPostProcessing;
  }
  not_reached();
}

const char* priorityToString(ThreadHint::Priority priority) {
  switch (priority) {
    case ThreadHint::Priority::Idling:
      return "Idling";
    case ThreadHint::Priority::Processing:
      return "Processing";
    case ThreadHint::Priority::PostProcessing:
      return "PostProcessing";
    }
    not_reached();
}

void ThreadHint::updateThreadHint(Priority priority) {
  if (m_threadHint.fd() == -1) {
    FTRACE(1, "No thread hint file, returning\n");
    return;
  }

  // Init thread local data
  ThreadHint::s_threadData.getCheck();
  auto &threadData = *ThreadHint::s_threadData;

  if (threadData.tid < 0) {
    threadData.tid = Process::GetThreadPid();
    if (threadData.tid < 0) {
      FTRACE(1, "Failed to get tid\n");
      return;
    }
  }
  if (threadData.pidfd.fd() == -1) {
    auto const fd = Process::PidfdOpen(threadData.tid, O_EXCL /* PIDFD_THREAD from linux/pidfd.h */);
    if (fd < 0) {
      FTRACE(1, "Failed to open pidfd for {}\n", threadData.tid);
      return;
    }
    threadData.pidfd = folly::File(fd, false);
  }

  // Only the first value is read by scx, remaining are reserved
  uint64_t threadHint[4] = {getHint(priority)};
  auto const pidfd = threadData.pidfd.fd();

  auto const res =
    bpf_map_update_elem(m_threadHint.fd(), &pidfd, threadHint, BPF_ANY);

  if (res < 0) {
    FTRACE(1, "Failed to update thread hint for {}\n", threadData.tid);
    return;
  }

  FTRACE(2, "ThreadHint: tid={} priority={} hint={}\n", threadData.tid,
    priorityToString(priority), threadHint[0]);
}

} // namespace HPHP
