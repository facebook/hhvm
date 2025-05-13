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

#pragma once

#include "hphp/util/thread-local.h"
#include "folly/File.h"

namespace HPHP {

/**
 * Thread hinting allows influencing kernel scheduling decisions. This is done
 * by writing out a hint [0, 1024] per thread to a BPF map. This map is then
 * read by sched_ext and used to prioritize threads that have a hint value.
 * Lower hint values will be prioritized over higher ones. Threads without a
 * hint (default) are treated with a priority of 1024. This allows the scheduler
 * to prioritize threads that are processing latency sensitive work
 * (e.g. pre-psp) over other threads.
 *
 * Thread hints are set via config, Server.ScxThreadHintIdle, etc.
 */
struct ThreadHint {
  enum class Priority {
    Idling = 0,
    Processing,
    PostProcessing
  };

public:
  static ThreadHint &getInstance();

  /**
   * Update the thread hint for the current thread based on ThreadMode, will
   * persist a hint in the thread hint map and influence kernel scheduling
   * decisions.
   */
  void updateThreadHint(Priority state);

private:
  ThreadHint();
  ~ThreadHint() = default;
  ThreadHint(const ThreadHint&) = delete;
  ThreadHint(ThreadHint&&) = delete;
  ThreadHint& operator=(const ThreadHint&) = delete;
  ThreadHint& operator=(ThreadHint&&) = delete;

  void initHintMap(const std::string_view path);

  folly::File m_threadHint;

  struct ThreadData {
    pid_t tid{-1};
    folly::File pidfd;
  };
  static THREAD_LOCAL_NO_CHECK(ThreadData, s_threadData);
};

} // namespace HPHP
