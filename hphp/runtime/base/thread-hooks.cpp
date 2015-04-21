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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ThreadMap threadMap;
static Mutex threadMap_lock;

PthreadInfo::PthreadInfo(start_routine_t start, void* arg) :
    start_routine(start), start_routine_arg(arg) {
  pid = getpid();

  if (RuntimeOption::EvalLogThreadCreateBacktraces) {
    num_frames = backtrace(reinterpret_cast<void **>(&parent_bt),
                           max_num_frames);
    parent_bt_names = backtrace_symbols(
      reinterpret_cast<void *const*>(&parent_bt),
      num_frames);
    if (!parent_bt_names) {
      Logger::Error("pthread_create: unable to get backtrace symbols");
    }
    start_name_ptr = backtrace_symbols(
      reinterpret_cast<void *const *>(&start_routine), 1);
    if (!start_name_ptr) {
      Logger::Error("pthread_create: unable to get start_routine name");
    }
  }
}

PthreadInfo::~PthreadInfo() {
  free(parent_bt_names);
  free(start_name_ptr);
}

std::string get_thread_mem_usage() {
  std::string result;
  Lock lock(threadMap_lock);

  result = "Thread memory usage:\n\tProcess Id\tThread Id"
           "\t\tBytes allocated\t\tThread Name\n";
  for (auto it : threadMap) {
    if (!it.second->mm) continue;
    auto& stats = it.second->mm->getStats();
    result += folly::sformat("\t{:10}\t{:9}\t{:13}\t\t{}\n", it.second->pid,
                             it.second->tid, stats.usage,
                             *it.second->start_name_ptr);
  }
  return result;
}

/*
 * We need to intercept the start routine in order to make sure each
 * thread has a MemoryManager.
 */

void* start_routine_wrapper(void *arg) {
  pthread_t self = pthread_self();
  auto& info = *reinterpret_cast<PthreadInfo*>(arg);

  MemoryManager::TlsWrapper::getCheck();
  info.mm = &MM();
  assert(info.mm);
  info.mm->resetExternalStats();
#ifdef __linux__
  info.tid = syscall(SYS_gettid);
#else
  info.tid = getpid();
#endif

  auto ret = info.start_routine(info.start_routine_arg);
  log_pthread_event(PTHREAD_EXIT, &self);
  return ret;
}

void log_pthread_event(pthread_event event, pthread_t* thread) {
  Lock lock(threadMap_lock);
  auto it = threadMap.find(*thread);

  switch (event) {
    case PTHREAD_CREATE: {
      always_assert(!"All logging handled elsewhere");
      break;
    }
    case PTHREAD_EXIT: {
      if (it == threadMap.end()) {
        Logger::Warning("pthread_exit: thread does not exist");
        return;
      }
      delete it->second;
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

#ifdef __linux__
extern "C" {

int __wrap_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                           void *(*start_routine) (void *), void* arg) {
  auto info = new HPHP::PthreadInfo(start_routine, arg);
  int ret = __real_pthread_create(thread, attr,
                                  HPHP::start_routine_wrapper, info);
  {
    HPHP::Lock lock(HPHP::threadMap_lock);
    auto res = HPHP::threadMap.emplace(*thread, info);
    if (!res.second) {
      HPHP::Logger::Error("pthread_create: thread already exists");
    }
  }
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
#endif // __linux__
