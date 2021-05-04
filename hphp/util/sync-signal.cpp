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

#include "hphp/util/sync-signal.h"

#include "hphp/util/logger.h"
#include "hphp/util/process.h"

#include <atomic>
#include <map>

#include <pthread.h>
#include <signal.h>

#ifdef __linux__
#include <sched.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#endif

namespace HPHP {

/*
 * Handle some asynchronous signals in a syncrhonous way.
 *
 * It is hard to write a nontrivial signal handler. A signal can arrive any
 * time, including when a signal handler is already executing; it can be
 * delievered to any of the threads in the current process (except a few cases
 * when the signal is directed to a specific thread. As a result, the set of
 * things a singal handler can safely do is very limited, e.g., it cannot
 * allocate memory (thus not use lots of library functions including many in
 * STL), cannot use pthread mutexes and condition variables.
 *
 * In many cases, when signals arrive, the program is terminating anyway, and a
 * buggy signal handler works most of the time, and is thus good enough.  But if
 * we want to make use of signals in other scenarios, we cannot have unsafe
 * signal handlers.
 *
 * To make it easier to handle signals, we use a dedicated thread to handle some
 * signals one by one, by blocking the signals and use sigwait() in a loop. This
 * way, the actual handler won't have to worry about reentering, and we can do
 * anything we want as usual.
 */

namespace {

struct SyncSignals {
  ~SyncSignals() {
    // Ignore all signals after exit().
    ignore_sync_signals();

    // FIXME: gracefully shut down the m_handlerThread
  }

  pthread_t m_handlerThread{};
  std::atomic_bool m_handlerThreadStarted{false};

  // "Synchronous" signal handlers.  This must be initialized before the handler
  // thread starts, and cannot change afterwards.
  std::map<int, sighandler_sync_t> m_syncHandlers;
  // We could've called sigemptyset() to initialize it, but it just does zero
  // fill on both Linux and Mac.
  sigset_t m_syncSignals{};
};

SyncSignals g_state;

// A signal in `m_syncSignals` should be blocked in all threads. And its
// handler should be `block_and_raise()`.  Usually, the signal handler is never
// invoked (because the signal is blocked).  But in case thread changes its own
// mask, we change it back here.
void block_and_raise(int signo) {
#ifndef NDEBUG
  if (!g_state.m_syncHandlers.count(signo)) {
    abort();
  }
#endif
  pthread_sigmask(SIG_BLOCK, &g_state.m_syncSignals, nullptr);
  // Send the signal to the dedicated handling thread.
  if (g_state.m_handlerThreadStarted.load(std::memory_order_acquire)) {
    pthread_kill(g_state.m_handlerThread, signo);
  }
}

void* handle_signals(void*) {
  sigset_t all_signals;
  if (!sigfillset(&all_signals)) {
    pthread_sigmask(SIG_BLOCK, &all_signals, nullptr);
  }
  if (g_state.m_handlerThreadStarted.exchange(true)) {
    Logger::Error("Cannot register more signal handlers "
                  "after the handler thread starts");
    return nullptr;
  }
  SCOPE_EXIT {
    g_state.m_handlerThreadStarted.store(false, std::memory_order_release);
  };
  if (g_state.m_syncHandlers.empty()) return nullptr;
#ifdef __linux__
  prctl(PR_SET_NAME, "sig_handler");
  sched_param param{};
  param.sched_priority = sched_get_priority_min(SCHED_RR);
  sched_setscheduler(0, SCHED_RR | SCHED_RESET_ON_FORK, &param);
#endif
  int signo = 0;
  while (!sigwait(&g_state.m_syncSignals, &signo)) {
    // dispatch signo
    auto const iter = g_state.m_syncHandlers.find(signo);
    if (iter == g_state.m_syncHandlers.end()) {
      Logger::FError("Cannot find handler for signal {}, ignoring", signo);
      continue;
    }
    if (iter->second == (sighandler_sync_t)SIG_IGN) {
      continue;
    }
    // Invoke the handler, which can do anything, including manipulating signal
    // handlers/masks, raising another signal, and calling pthread_exit() to
    // stop this thread.
    iter->second(signo);
  }
  Logger::Error("sigwait() failed");
  reset_sync_signals();
  return nullptr;
}

// Clear state in child process after fork().
void postfork_clear() {
  reset_sync_signals();
  g_state.m_handlerThreadStarted.store(false, std::memory_order_release);
  g_state.m_handlerThread = pthread_t{};
}

// Block `m_syncSignals` and point handlers to block_and_raise().
void block_sync_signals() {
  pthread_sigmask(SIG_BLOCK, &g_state.m_syncSignals, nullptr);
  for (auto iter : g_state.m_syncHandlers) {
    signal(iter.first, block_and_raise);
  }
  // If we ever fork(), avoid affecting child processes.
  static std::atomic_flag flag ATOMIC_FLAG_INIT;
  if (!flag.test_and_set()) {
    pthread_atfork(nullptr, nullptr, postfork_clear);
  }
}

}

//////////////////////////////////////////////////////////////////////

bool sync_signal(int signo, sighandler_sync_t sync_handler) {
  if (g_state.m_handlerThreadStarted.load(std::memory_order_acquire)) {
    return false;
  }
  if (signo <= 0 || signo >= Process::kNSig) return false;
  // We don't support SIG_DFL here.  SIG_IGN is OK.
  if (sync_handler == (sighandler_sync_t)SIG_DFL) return false;
  g_state.m_syncHandlers[signo] = sync_handler;
  sigaddset(&g_state.m_syncSignals, signo);
  return true;
}

bool is_sync_signal(int signo) {
  return g_state.m_syncHandlers.count(signo);
}

void reset_sync_signals() {
  for (auto const iter : g_state.m_syncHandlers) {
    signal(iter.first, SIG_DFL);
  }
  pthread_sigmask(SIG_UNBLOCK, &g_state.m_syncSignals, nullptr);
}

void ignore_sync_signals() {
  for (auto const iter : g_state.m_syncHandlers) {
    signal(iter.first, SIG_IGN);
  }
  pthread_sigmask(SIG_UNBLOCK, &g_state.m_syncSignals, nullptr);
}

void block_sync_signals_and_start_handler_thread() {
  if (g_state.m_syncHandlers.empty()) return;
  block_sync_signals();
  if (g_state.m_handlerThreadStarted.load(std::memory_order_acquire)) {
    Logger::Error("sync signal handler already started");
    return;
  }
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&g_state.m_handlerThread, &attr, &handle_signals, nullptr);
}

void postfork_restart_handler_thread() {
  block_sync_signals_and_start_handler_thread();
}

}
