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

#ifndef incl_HPHP_THREAD_INIT_FINI_H_
#define incl_HPHP_THREAD_INIT_FINI_H_

#include "hphp/util/portability.h"
#include "hphp/util/assertions.h"
#include "hphp/util/async-job.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct InitFiniNode;

struct IFJob {
  explicit IFJob(const InitFiniNode& n) : node(n) {}
  const InitFiniNode& node;
};
template <> struct WorkerInfo<IFJob> {
  enum { DoInitFini = false };
};

struct InitFiniNode {
  enum class When {
    RequestStart,            // before executing PHP code in a request
      RequestFini,
      ThreadInit,
      ThreadFini,
      // ProcessInitConcurrent should only be used for thread-safe code with few
      // dependencies (e.g., runtime options, logging).
      PostRuntimeOptions,    // after runtime options are set, single-threaded
      ProcessPreInit,        // after pthread initialization and config parsing
      ProcessInit,           // after PreInit
      ProcessInitConcurrent, // after PreInit, concurrently with Init and others
      ProcessExit,           // after Init and InitConcurrent
      ServerPreInit,
      ServerInit,
      WarmupConcurrent,      // concurrent with OS file cache warmup, optional
      ServerExit,

      Sentinel
  };

  const static unsigned NumNodes = static_cast<unsigned>(When::Sentinel);

  InitFiniNode(void(*f)(), When when, const char* what = nullptr) {
    InitFiniNode*& n = node(when);
    func = f;
    next = n;
    name = what;
    n = this;
  }

  static void RequestStart()   { iterate(When::RequestStart);   }
  static void RequestFini()    { iterate(When::RequestFini);    }
  static void ThreadInit()     { iterate(When::ThreadInit);     }
  static void ThreadFini()     { iterate(When::ThreadFini);     }
  static void ProcessPostRuntimeOptions() {
    iterate(When::PostRuntimeOptions);
  }
  static void ProcessPreInit() { iterate(When::ProcessPreInit); }
  static void ProcessInit()    { iterate(When::ProcessInit);    }
  // Use maxWorkers == 0 to run synchronously on current thread.
  static void ProcessInitConcurrentStart(uint32_t maxWorkers);
  static void ProcessInitConcurrentWaitForEnd();
  static void ProcessFini()    { iterate(When::ProcessExit);    }
  static void ServerPreInit()  { iterate(When::ServerPreInit);  }
  static void ServerInit()     { iterate(When::ServerInit);     }
  static void WarmupConcurrentStart(uint32_t maxWorkers);
  static void WarmupConcurrentWaitForEnd(); // No-op if not started.
  static void ServerFini()     { iterate(When::ServerExit);     }

  struct IFWorker {
    void onThreadEnter() {}
    void doJob(std::shared_ptr<IFJob> job);
    void onThreadExit() {}
  };
 private:
  using IFDispatcher = JobDispatcher<IFJob, IFWorker>;

  static InitFiniNode*& node(When when) {
    auto idx = static_cast<unsigned>(when);
    assertx(idx < NumNodes);
    return s_nodes[idx];
  }
  static IFDispatcher*& dispatcher(When when) {
    auto idx = static_cast<unsigned>(when);
    assertx(idx < NumNodes);
    return s_dispatcher[idx];
  }
  void (*func)();
  InitFiniNode* next;
  const char* name;

  static void iterate(When when) { iterate(node(when)); }
  static void iterate(InitFiniNode* node);
  static void concurrentStart(uint32_t maxWorkers, When when);
  static void concurrentWaitForEnd(When when);

  static InitFiniNode* s_nodes[NumNodes];
  static IFDispatcher* s_dispatcher[NumNodes];
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_THREAD_INIT_FINI_H_
