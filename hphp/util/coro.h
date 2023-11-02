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

#include "hphp/util/hash-map.h"
#include "hphp/util/lock-free-lazy.h"

#include <folly/Try.h>
#include <folly/Unit.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/AsyncScope.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Collect.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/Task.h>

#include <cstddef>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

//////////////////////////////////////////////////////////////////////

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// Calculate a value asynchronously on the given executor. This has an
// advantage over a bare coroutine because normally a coroutine will
// only start execution when you await it (and then you block until
// its done). This starts execution before you request the value,
// meaning it can run in the background.
template <typename T> struct CoroAsyncValue {
  template <typename F>
  CoroAsyncValue(F f, folly::Executor::KeepAlive<> executor) {
    auto work = folly::coro::co_invoke(
      [this, f = std::move(f)] () mutable -> folly::coro::Task<void> {
        try {
          m_try.emplace(co_await f());
        } catch (...) {
          m_try.emplaceException(
            folly::exception_wrapper(std::current_exception())
          );
        }
        m_baton.post();
        co_return;
      }
    );
    m_scope.add(std::move(work).scheduleOn(std::move(executor)));
  }

  ~CoroAsyncValue() {
    // NB: It might seem useful here to access the m_try after
    // waiting, to ensure that any exception is rethrown. However, we
    // can't throw from a dtor anyways, so its pointless to check.
    folly::coro::blockingWait(m_scope.joinAsync());
  }

  CoroAsyncValue(const CoroAsyncValue&) = delete;
  CoroAsyncValue(CoroAsyncValue&&) = delete;
  CoroAsyncValue& operator=(const CoroAsyncValue&) = delete;
  CoroAsyncValue& operator=(CoroAsyncValue&&) = delete;

  // Coroutines which return either a pointer to the value, or a copy
  // of the value. If the calculation resulted in an exception, that
  // exception will be thrown.

  folly::coro::Task<const T*> operator*() const {
    co_await m_baton;
    co_return &*m_try;
  }

  folly::coro::Task<T> getCopy() const {
    co_await m_baton;
    co_return *m_try;
  }

private:
  folly::Try<T> m_try;
  folly::coro::Baton m_baton;
  folly::coro::AsyncScope m_scope;
};

//////////////////////////////////////////////////////////////////////

// Maps a key to an asynchronously calculated value. This ensures that
// for any key, the value will be calculated precisely once.
template <typename K, typename V>
struct CoroAsyncMap {
  // If the key is present, return a coroutine which can be awaited to
  // get the associated value. If the key isn't present, then the
  // callable is invoked on the given executor and a coroutine is
  // returned representing that calculation. Any other concurrent (or
  // later) calls will wait on the same value.
  template <typename F>
  folly::coro::Task<V> get(const K& key,
                           F&& f,
                           folly::Executor::KeepAlive<> executor) {
    // AsyncValue is responsible for doing the actual async
    // calculation. We wrap that in a LockFreeLazy to ensure we only
    // ever create one of them for each key. The LockFreeLazy is
    // inside an unique_ptr since the LockFreeLazy is not
    // moveable. The folly_concurrent_hash_map_simd ensures we only
    // ever get one LockFreeLazy per key.
    auto& lazy = [&] () -> Lazy& {
      // Look up the key. If there's an entry, return it. Otherwise
      // create one and return it. If there's a race with the
      // insertion, one thread will actually insert it, and the rest
      // will just free their unique_ptrs (they'll all return the one
      // actually inserted).
      auto const it = m_map.find(key);
      if (it != m_map.end()) return *it->second;
      return *m_map.insert(
        key, std::make_unique<Lazy>()
      ).first->second;
    }();

    // We got the appropriate LockFreeLazy, which might be new or
    // old. Either way, get its value, which ensures exactly one
    // thread creates the AsyncValue and starts its calculation.
    return lazy.get(
      [&] { return Async{f(), std::move(executor)}; }
    ).getCopy();
  }

private:
  using Async = CoroAsyncValue<V>;
  using Lazy = LockFreeLazy<Async>;
  folly_concurrent_hash_map_simd<K, std::unique_ptr<Lazy>> m_map;
};

//////////////////////////////////////////////////////////////////////

// Block coro task until the latch has been signalled N times.
struct CoroLatch {
  explicit CoroLatch(uint64_t c) : m_count{c} {}
  folly::coro::Task<void> wait() { co_await m_baton; }
  void count_down() {
    if (!--m_count) m_baton.post();
  }

  CoroLatch(const CoroLatch&) = delete;
  CoroLatch(CoroLatch&&) = delete;
  CoroLatch& operator=(const CoroLatch&) = delete;
  CoroLatch& operator=(CoroLatch&&) = delete;
private:
  std::atomic<uint64_t> m_count;
  folly::coro::Baton m_baton;
};

//////////////////////////////////////////////////////////////////////

/*
 * A CPUThreadPoolExecutor with stronger forward progress
 * guarantees. Using the standard Executor interface it behaves
 * exactly as a CPUThreadPoolExecutor. However, one can add tasks with
 * an associated "Ticket". Runnable tasks are always prioritized by
 * their tickets (lower tickets are higher priority). So far this is
 * pretty standard, but one can obtain a "sticky" executor, which
 * remembers the assigned ticket. The same ticket will be used no
 * matter how many times the task is run then re-added to the
 * executor.
 *
 * This is useful for coroutines. You can assign a coroutine a sticky
 * executor and that coroutine will always be prioritized over later
 * coroutines. This ensures that earlier coroutines will finish before
 * later coroutines.
 */

struct TicketExecutor : public folly::CPUThreadPoolExecutor {
  // Create a TicketExecutor with the given number of threads. Each
  // thread created will call threadInit before running, and
  // threadFini before exiting. Any idle threads will exit after
  // threadTimeout passes without work. The created threads will set
  // to their name to threadPrefix plus some numeric identifier.
  TicketExecutor(const std::string& threadPrefix,
                 size_t minThreads,
                 size_t maxThreads,
                 std::function<void()> threadInit,
                 std::function<void()> threadFini,
                 std::chrono::milliseconds threadTimeout);
  ~TicketExecutor();

  using Ticket = int64_t;

  // Add work to this executor with the specified ticket.
  void addWithTicket(folly::Func, Ticket);

  // Obtain a new ticket. Tickets are monotonically increasing. Each
  // stamp() returns an unique new ticket higher than all previous
  // ones.
  Ticket stamp();

  // Create a new Executor (of some unnamed type) which will schedule
  // work on this TicketExecutor, but using addWithTicket with the
  // same ticket.
  folly::Executor::KeepAlive<> sticky() { return sticky(stamp()); }
  folly::Executor::KeepAlive<> sticky(Ticket);
};

//////////////////////////////////////////////////////////////////////

}
