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

#ifndef incl_HPHP_RUNTIME_VM_WORKER_H
#define incl_HPHP_RUNTIME_VM_WORKER_H

#include "hphp/util/async-func.h"
#include <functional>

namespace HPHP {

struct WorkerSpec {
  int numaNode{-1};
  unsigned hugeStackKb{0};
  unsigned tlExtraKb{0};
};

// Run arbitrary code in a thread/fiber with VM context.
struct VMWorker : private std::function<void(void)>
                , private AsyncFuncImpl {
  template<class F>
  explicit VMWorker(F&& f)
    : std::function<void(void)>(f)
    , AsyncFuncImpl(this, run_, -1, 0, 0)
  {}

  template<class F>
  VMWorker(WorkerSpec s, F&& f)
    : std::function<void(void)>(f)
    , AsyncFuncImpl(this, run_, s.numaNode, s.hugeStackKb, s.tlExtraKb)
  {}

  VMWorker(const VMWorker&) = delete;

  using AsyncFuncImpl::start;
  using AsyncFuncImpl::run;
  using AsyncFuncImpl::waitForEnd;

 private:
  static void run_(void* obj) {
    // hphp_thread_init/exit are called as hooks of AsyncFuncImpl, see struct
    // SetThreadInitFini.
    auto p = static_cast<VMWorker*>(obj);
    (*p)();
  }
};

}

#endif
