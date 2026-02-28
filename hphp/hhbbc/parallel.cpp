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
#include <cstdlib>

#include "hphp/hhbbc/parallel.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace parallel {

//////////////////////////////////////////////////////////////////////

size_t num_threads = 31;
size_t final_threads = 31;

//////////////////////////////////////////////////////////////////////

thread_local bool tl_inParallel = false;

//////////////////////////////////////////////////////////////////////

thread_pool::thread_pool()
{
  for (size_t i = 0; i < parallel::num_threads; ++i) {
    threads.emplace_back(&thread_pool::thread_run, this, i);
  }
}

thread_pool::~thread_pool() {
  active.shutdown();
  for (auto& t : threads) t.join();
}

void thread_pool::thread_run(size_t worker) {
  tl_inParallel = true;
  SCOPE_EXIT { tl_inParallel = false; };
  HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
  try {
    while (true) {
      active.wait();
      func(worker);
      wait.post();
    }
  } catch (const folly::ShutdownSemError&) {
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}
