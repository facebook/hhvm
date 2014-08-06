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
#ifndef incl_HPHP_THREAD_HOOKS_H_
#define incl_HPHP_THREAD_HOOKS_H_

#include <pthread.h>
#include <sys/types.h>
#include <unordered_map>

#include "hphp/runtime/base/memory-manager.h"

extern "C" {
  int __wrap_pthread_create (pthread_t* thread, const pthread_attr_t* attr,
                             void *(*start_routine) (void *), void* arg);
  int __real_pthread_create (pthread_t* thread, const pthread_attr_t* attr,
                             void *(*start_routine) (void *), void* arg);

  void __wrap_pthread_exit (void* retval);
  void __real_pthread_exit (void* retval);

  int __wrap_pthread_join (pthread_t thread, void **retval);
  int __real_pthread_join (pthread_t thread, void **retval);
}

namespace HPHP {

//////////////////////////////////////////////////////////////////////

enum pthread_event {
  PTHREAD_CREATE,
  PTHREAD_EXIT,
  PTHREAD_JOIN
};

constexpr int max_num_frames = 20;

typedef void* (*start_routine_t) (void *);

struct PthreadInfo {
  int num_frames;
  void* parent_bt[max_num_frames];
  char** parent_bt_names;
  start_routine_t start_routine;
  pid_t pid;
  pid_t tid;
  char** start_name_ptr;
  struct MemoryManager* mm;
};

typedef std::unordered_map<pthread_t, PthreadInfo> ThreadMap;

std::string get_thread_mem_usage();
void* start_routine_wrapper(void *);
void log_pthread_event(pthread_event event, pthread_t* thread,
                       start_routine_t start = nullptr);

//////////////////////////////////////////////////////////////////////

}

#endif
