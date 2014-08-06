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
#include "hphp/runtime/base/thread-hooks.h"
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>

#include "hphp/runtime/base/extended-logger.h"
#include "hphp/util/assertions.h"
#include "hphp/util/mutex.h"

extern "C" {

int __wrap_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                           void *(*start_routine) (void *), void* arg) {
  int ret = __real_pthread_create(thread, attr,
                                  HPHP::start_routine_wrapper, arg);
  log_pthread_event(HPHP::PTHREAD_CREATE, thread, start_routine);
  return ret;
}

void __wrap_pthread_exit(void* retval) {
  pthread_t self = pthread_self();
  log_pthread_event(HPHP::PTHREAD_EXIT, &self);
  __real_pthread_exit(retval);
}

int __wrap_pthread_join(pthread_t thread, void **retval) {
  log_pthread_event(HPHP::PTHREAD_JOIN, &thread);
  return __real_pthread_join(thread, retval);
}

}

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ThreadMap threadMap;
static Mutex m_threadmap_lock;

std::string get_thread_mem_usage() {
  std::string result;
  Lock lock(m_threadmap_lock);

  result = "Thread memory usage:\n\tProcess Id\tThread Id"
           "\t\tBytes allocated\t\tThread Name\n";
  for (auto it : threadMap) {
    if (!it.second.mm) continue;
    auto& stats = it.second.mm->getStats();
    result += folly::format("\t{:10}\t{:9}\t{:13}\t\t{}\n", it.second.pid,
                            it.second.tid, stats.usage,
                            *it.second.start_name_ptr).str();
  }
  return result;
}

/*
 * We need to intercept the start routine in order to make sure each
 * thread has a MemoryManager.
 */

void* start_routine_wrapper(void *arg) {
  pthread_t self = pthread_self();
  start_routine_t start_routine;

retry:
  {
  Lock lock(m_threadmap_lock);

  auto it = threadMap.find(self);
  if (it == threadMap.end()) {
    goto sleep_wait;
  }

  auto& info = it->second;
  MemoryManager::TlsWrapper::getCheck();
  info.mm = &MM();
  assert(info.mm);
  info.mm->resetExternalStats();
#ifdef __linux__
  info.tid = syscall(SYS_gettid);
#else
  info.tid = getpid();
#endif
  info.pid = getpid();
  start_routine = info.start_routine;
  goto done;
  }

sleep_wait:
  usleep(100);
  goto retry;

done:
  auto ret = start_routine(arg);
  log_pthread_event(PTHREAD_EXIT, &self);
  return ret;
}

void log_pthread_event(pthread_event event, pthread_t* thread,
                       start_routine_t start_routine) {
  struct PthreadInfo info;

  memset(&info, 0, sizeof(info));
  switch (event) {
  case PTHREAD_CREATE: {
    info.num_frames = backtrace(reinterpret_cast<void **>(&info.parent_bt),
                                max_num_frames);
    info.parent_bt_names = backtrace_symbols(
                             reinterpret_cast<void *const*>(&info.parent_bt),
                             info.num_frames);
    if (!info.parent_bt_names) {
      Logger::Error("pthread_create: unable to get backtrace symbols");
    }
    assert(start_routine);
    info.start_routine = start_routine;
    info.start_name_ptr = backtrace_symbols(
                            reinterpret_cast<void *const *>(&start_routine), 1);
    if (!info.start_name_ptr) {
      Logger::Error("pthread_create: unable to get start_routine name");
    }
    break;
  }
  case PTHREAD_EXIT:
    break;
  case PTHREAD_JOIN:
    break;
  }

  {
  Lock lock(m_threadmap_lock);
  auto it = threadMap.find(*thread);

  switch (event) {
  case PTHREAD_CREATE: {
    if (it != threadMap.end()) {
      Logger::Error("pthread_create: thread already exists");
    }
    threadMap[*thread] = info;
    break;
  }
  case PTHREAD_EXIT: {
    if (it == threadMap.end()) {
      Logger::Warning("pthread_exit: thread does not exist");
      return;
    }
    threadMap.erase(it);
    break;
  }
  case PTHREAD_JOIN: {
    // Thread might have already exited. So, it is not guaranteed to be
    // found in the thread-map.
    break;
  }
  }
  }
}

///////////////////////////////////////////////////////////////////////////////

}
