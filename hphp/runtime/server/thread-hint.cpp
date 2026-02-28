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
#include <sys/un.h>

#include <folly/File.h>

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
  int fd = getThreadHintFd();
  if (fd >= 0) {
    m_threadHint = folly::File(fd, false);
    FTRACE(1, "ThreadHint: Thread hint fd set to {}\n", m_threadHint.fd());
  }
}

int ThreadHint::getThreadHintFd() {
  if (auto const& path = Cfg::Server::ScxThreadHintPath; !path.empty()) {
    FTRACE(1, "ThreadHint: Setting thread hint path to {}\n", path);
    int fd = bpf_obj_get(path.c_str());
    if (fd < 0) {
      FTRACE(1, "ThreadHint: Failed to open BPF map at {}: {}\n",
             path, folly::errnoStr(errno));
    }
    return fd;
  }

  return -1;
}

static constexpr uint16_t getHint(ThreadHint::Priority priority) {
  switch (priority) {
  case ThreadHint::Priority::Idling:
    return Cfg::Server::ScxThreadHintIdle;
  case ThreadHint::Priority::FirstFlush:
    return Cfg::Server::ScxThreadHintFirstFlush;
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
    case ThreadHint::Priority::FirstFlush:
      return "FirstFlush";
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
