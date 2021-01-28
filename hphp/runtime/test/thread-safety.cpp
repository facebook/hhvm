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
#include <gtest/gtest.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/ldap/ext_ldap.h"
#include "hphp/util/process.h"

#include <atomic>
#include <chrono>
#include <thread>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {
// Certain functions may not be safe in multithreaded environments, such as in
// server mode. Here we introduce a way to simultaneously run a function in many
// threads.
template <typename Func>
void runInThread(std::atomic_bool& flag, Func f) {
  hphp_thread_init();
  rds::local::init();
  g_context.getCheck();
  // wait until external
  while (!flag.load(std::memory_order_acquire)) {
#ifdef __x86_64__
    __asm__("pause");
#endif
  }
  f();
  rds::local::fini();
  hphp_thread_exit();
}

template <typename Func>
void runInManyThreads(unsigned nThreads, Func f) {
  std::atomic_bool flag = false;
  std::vector<std::unique_ptr<std::thread>> threads;
  for (unsigned i = 0; i < nThreads; ++i) {
    threads.emplace_back(std::make_unique<std::thread>([&] {
      runInThread(flag, f);
    }));
  }
  /* sleep override */ usleep(2000);
  flag.store(true, std::memory_order_release);
  for (auto& t : threads) {
    t->join();
  }
}

}
//////////////////////////////////////////////////////////////////////

TEST(MultiThread, LDAP) {
  runInManyThreads(Process::GetCPUCount(), [] {
    f_ldap_connect("localhost");
  });
}

//////////////////////////////////////////////////////////////////////

}
