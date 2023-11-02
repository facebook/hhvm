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

#include <cstddef>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

//////////////////////////////////////////////////////////////////////

/*
 * C++ coroutine portability layer
 *
 * Not all of the compilers we support have coroutine support, and
 * some that do are buggy. Nonetheless, we still want to be able to
 * use coroutines. This file provides a portability layer to allow for
 * the usage of coroutines (with very similar syntax), but supports
 * older compilers.
 *
 * If the compiler supports coroutines, this is just a simple wrapper
 * around the coroutine functionality we need. If not, all computation
 * is performed *eagerly* and the coroutine syntax mainly desugars
 * into nothing. For the most part, one can write the code assuming
 * coroutines are supported and it will just work.
 *
 * NB: There is one important difference. Since "emulated" coroutines
 * calculate their values eagerly, they may throw exceptions from
 * different places than "native" coroutines. Native coroutines will
 * only throw when they are awaited. Emulated coroutines will throw
 * when they are first called. When writing code when using this
 * layer, one should be mindful of this and structure their
 * try/catches appropriately.
 *
 * Emulated coroutines may be (much) slower than native coroutines
 * since they are all calculated eagerly, thus there's no room for
 * parallelism. We expect to use native coroutines in situations where
 * performance matters.
 *
 * Emulated coroutines can be forced by defining HPHP_DISABLE_CORO.
 */

//////////////////////////////////////////////////////////////////////

// Co-routine support in GCC is currently broken....
#if FOLLY_HAS_COROUTINES && defined(__clang__) && !defined(HPHP_DISABLE_CORO)

// Native coroutines:

#include <folly/experimental/coro/AsyncScope.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Collect.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/Task.h>

//////////////////////////////////////////////////////////////////////

namespace HPHP::coro {

//////////////////////////////////////////////////////////////////////

constexpr const bool using_coros = true;

// Operators. They must be macros since they're part of the syntax.

// Await a coroutine, blocking execution until it returns a value.
#define HPHP_CORO_AWAIT(x) (co_await (x))
// Return a value from a coroutine
#define HPHP_CORO_RETURN(x) co_return (x)
// Return a value from a coroutine while moving it. This distinction
// doesn't matter for native coroutines.
#define HPHP_CORO_MOVE_RETURN(x) co_return (x)
// Return a void value from a coroutine. Due to syntactic issues, this
// must be a different macro than the value.
#define HPHP_CORO_RETURN_VOID co_return
// Return a Executor* representing the executor this coroutine is
// assigned to.
#define HPHP_CORO_CURRENT_EXECUTOR (co_await folly::coro::co_current_executor)
// Yield execution of the current coroutine and reschedule it to run
// on its assigned executor. The collect* functions can attempt to run
// a coroutine eagerly on the current executor. This forces them to
// run on their assigned executor.
#define HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR \
  (co_await folly::coro::co_reschedule_on_current_executor)
// Throw a folly::OperationCancelled if this coroutine has been cancelled
#define HPHP_CORO_SAFE_POINT (co_await folly::coro::co_safe_point)

// The actual arguments of these can be complex, so we just perfect
// forward everything rather than replicating it here.

// Await the given coroutine, blocking the thread until it has a
// value. This can be used to "await" a coroutine without become a
// coroutine yourself.
template <typename... Args>
auto wait(Args&&... args) {
  return folly::coro::blockingWait(std::forward<Args>(args)...);
}

// Takes a set of coroutines and returns a coroutine representing all
// of the coroutines. Awaiting that coroutine returns a tuple of the
// values of the passed coroutines. This lets you await on multiple
// coroutines simultaneously.
template <typename... Args>
auto collect(Args&&... args) {
  return folly::coro::collectAll(std::forward<Args>(args)...);
}

// Like collect, but accepts a std::vector of coroutines.
template <typename T>
auto collectRange(std::vector<T>&& ts) {
  return folly::coro::collectAllRange(std::move(ts));
}

// Like collectRange, but will only run up to a fixed number of tasks
// simultaneously.
template <typename T>
auto collectRangeWindowed(std::vector<T>&& ts, std::size_t window) {
  return folly::coro::collectAllWindowed(std::move(ts), window);
}

// Given a callable and a set of arguments, store the arguments, then
// invoke the callable. This is useful if you have a callable which
// takes params by reference. Passing a temporary will cause crashes
// because when the coroutine suspends and then runs later, the
// temporary will be gone, and the reference will point to
// nothing. This ensures that any such temporaries will be stored
// somewhere which lives as long as the coroutine. It is also needed
// with lambdas which are coroutines for similar reasons. If you call
// the lambda immediately, it will be gone when the coroutine resumes.
template <typename... Args>
auto invoke(Args&&... args) {
  return folly::coro::co_invoke(std::forward<Args>(args)...);
}

// Block the execution of this coroutine for the given amount of time.
inline auto sleep(std::chrono::milliseconds d) {
  return folly::coro::sleep(d);
}

// All coroutines return Task<T>, means they return T when they
// finish. TaskWithExecutor<T> is a Task which has been bound to an
// executor.
template <typename T> using Task = folly::coro::Task<T>;
template <typename T>
using TaskWithExecutor = folly::coro::TaskWithExecutor<T>;

// Calculate a value asynchronously on the given executor. This has an
// advantage over a bare coroutine because normally a coroutine will
// only start execution when you await it (and then you block until
// its done). This starts execution before you request the value,
// meaning it can run in the background.
template <typename T> struct AsyncValue {
  template <typename F>
  AsyncValue(F f, folly::Executor::KeepAlive<> executor) {
    auto work = folly::coro::co_invoke(
      [this, f = std::move(f)] () mutable -> Task<void> {
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

  ~AsyncValue() {
    // NB: It might seem useful here to access the m_try after
    // waiting, to ensure that any exception is rethrown. However, we
    // can't throw from a dtor anyways, so its pointless to check.
    folly::coro::blockingWait(m_scope.joinAsync());
  }

  AsyncValue(const AsyncValue&) = delete;
  AsyncValue(AsyncValue&&) = delete;
  AsyncValue& operator=(const AsyncValue&) = delete;
  AsyncValue& operator=(AsyncValue&&) = delete;

  // Coroutines which return either a pointer to the value, or a copy
  // of the value. If the calculation resulted in an exception, that
  // exception will be thrown.

  Task<const T*> operator*() const {
    co_await m_baton;
    co_return &*m_try;
  }

  Task<T> getCopy() const {
    co_await m_baton;
    co_return *m_try;
  }

private:
  folly::Try<T> m_try;
  folly::coro::Baton m_baton;
  folly::coro::AsyncScope m_scope;
};

// Coro aware semaphore (runs a different task when blocks)
struct Semaphore {
  explicit Semaphore(uint32_t tokens) : m_sem{tokens} {}
  void signal() { m_sem.signal(); }
  Task<void> wait() { return m_sem.co_wait(); }
private:
  folly::fibers::Semaphore m_sem;
};

// Block coro task until the latch has been signalled N times. NB: If
// coros are being emulated, this class will be a no-op. This is
// because in that situation tasks run synchronously and using a latch
// will typically cause deadlock. Be aware of this possibility when
// using it.
struct Latch {
  explicit Latch(uint64_t c) : m_count{c} {}
  Task<void> wait() { co_await m_baton; }
  void count_down() {
    if (!--m_count) m_baton.post();
  }

  Latch(const Latch&) = delete;
  Latch(Latch&&) = delete;
  Latch& operator=(const Latch&) = delete;
  Latch& operator=(Latch&&) = delete;
private:
  std::atomic<uint64_t> m_count;
  folly::coro::Baton m_baton;
};

// Allows you to run coroutines asynchronously. Assign an executor to
// a Task, then add it to the AsyncScope. The coroutine will start
// running and will be automatically cleaned up when done. Since you
// cannot await the coroutine after you add it, this is only really
// useful for side-effectful coroutines which return void. The
// coroutine should not throw.
using AsyncScope = folly::coro::AsyncScope;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

#else

#include <folly/synchronization/LifoSem.h>

// Emulated coroutines. The coroutine is just the value itself. The
// value is calculated eagerly when the coroutine is created (so
// basically like any other normal function call).

namespace HPHP::coro {

//////////////////////////////////////////////////////////////////////

constexpr const bool using_coros = false;

// Task and TaskWithExecutor are just simple wrappers around a value.

namespace detail {
template <typename T> struct DummyTask;

template <typename T>
struct DummyTask {
  explicit DummyTask(T v) : val{std::move(v)} {}

  DummyTask<T> scheduleOn(folly::Executor::KeepAlive<>) && {
    return DummyTask<T>{std::move(val)};
  }

  T take() && { return std::move(val); }
private:
  T val;
};

template<> struct DummyTask<void> {
  DummyTask<void> scheduleOn(folly::Executor::KeepAlive<>) && {
    return DummyTask<void>{};
  }
  void take() && {}
};

template <typename T> DummyTask<T> makeDummy(T v) {
  return DummyTask<T>{std::move(v)};
}
}

template <typename T> using Task = detail::DummyTask<T>;
template <typename T> using TaskWithExecutor = detail::DummyTask<T>;

// "Awaiting" is just taking the wrapper value
#define HPHP_CORO_AWAIT(x) ((x).take())
// Returning just wraps the value
#define HPHP_CORO_RETURN(x) return HPHP::coro::detail::makeDummy((x))
#define HPHP_CORO_MOVE_RETURN(x) \
  return HPHP::coro::detail::makeDummy(std::move(x))
#define HPHP_CORO_RETURN_VOID return HPHP::coro::detail::DummyTask<void>{}
// This isn't ideal but there's no real good notion of "current
// executor" if coroutines don't actually exist.
#define HPHP_CORO_CURRENT_EXECUTOR ((folly::Executor*)nullptr)
#define HPHP_CORO_RESCHEDULE_ON_CURRENT_EXECUTOR
#define HPHP_CORO_SAFE_POINT

template <typename T>
auto wait(Task<T>&& t) { return std::move(t).take(); }

inline void wait(Task<void>&&) {}

namespace detail {
template<typename T>
auto collectImpl(Task<T>&& t) {
  return std::move(t).take();
}
inline auto collectImpl(Task<void>&&) {
  return folly::Unit{};
}
}

template <typename... T>
auto collect(Task<T>&&... t) {
  return detail::makeDummy(
    std::make_tuple(detail::collectImpl(std::move(t))...)
  );
}

template <typename T>
auto collectRange(std::vector<Task<T>>&& ts) {
  std::vector<T> out;
  out.reserve(ts.size());
  for (auto& t : ts) out.emplace_back(std::move(t).take());
  return detail::makeDummy(std::move(out));
}

inline auto collectRange(std::vector<Task<void>>&&) {
  return Task<void>{};
}

template <typename T>
auto collectRangeWindowed(std::vector<Task<T>>&& ts, std::size_t) {
  return collectRange(std::move(ts));
}

template <typename F, typename... Args> auto invoke(F&& f, Args&&... args) {
  return f(std::forward<Args>(args)...);
}

inline Task<void> sleep(std::chrono::milliseconds d) {
  std::this_thread::sleep_for(d);
  HPHP_CORO_RETURN_VOID;
}

// Just calculate the value eagerly, fulfilling the interface.
template <typename T> struct AsyncValue {
  template <typename F>
  AsyncValue(F&& f, folly::Executor::KeepAlive<>) {
    try {
      m_try.emplace(f().take());
    } catch (...) {
      m_try.emplaceException(
        folly::exception_wrapper(std::current_exception())
      );
    }
  }

  AsyncValue(const AsyncValue&) = delete;
  AsyncValue(AsyncValue&&) = delete;
  AsyncValue& operator=(const AsyncValue&) = delete;
  AsyncValue& operator=(AsyncValue&&) = delete;

  Task<const T*> operator*() const {
    return Task<const T*>{&*m_try};
  }

  Task<T> getCopy() const {
    return Task<T>{*m_try};
  }
private:
  folly::Try<T> m_try;
};

// A Task<void> has by definition alreadly executed whatever
// side-effects it has, so there's nothing to do there.
struct AsyncScope {
  void add(TaskWithExecutor<void>&&) {}
  Task<void> joinAsync() { return Task<void>{}; }
};

struct Semaphore {
  explicit Semaphore(uint32_t tokens) : m_sem{tokens} {}
  void signal() { m_sem.post(); }
  Task<void> wait() { m_sem.wait(); return Task<void>{}; }
private:
  folly::LifoSem m_sem;
};

// No-op implementation. In coro emulation, all tasks run
// synchronously and thus using a latch to block a task to wait for
// another will just deadlock. Only use Latch in situations where this
// is acceptable behavior.
struct Latch {
  explicit Latch(uint64_t) {}
  Task<void> wait() { return Task<void>{}; }
  void count_down() {}

  Latch(const Latch&) = delete;
  Latch(Latch&&) = delete;
  Latch& operator=(const Latch&) = delete;
  Latch& operator=(Latch&&) = delete;
};

//////////////////////////////////////////////////////////////////////

}
#endif

//////////////////////////////////////////////////////////////////////

namespace HPHP::coro {

// Generic functionality for both modes:

//////////////////////////////////////////////////////////////////////

// Maps a key to an asynchronously calculated value. This ensures that
// for any key, the value will be calculated precisely once.
template <typename K, typename V>
struct AsyncMap {
  // If the key is present, return a coroutine which can be awaited to
  // get the associated value. If the key isn't present, then the
  // callable is invoked on the given executor and a coroutine is
  // returned representing that calculation. Any other concurrent (or
  // later) calls will wait on the same value.
  template <typename F>
  Task<V> get(const K& key,
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
  using Async = AsyncValue<V>;
  using Lazy = LockFreeLazy<Async>;
  folly_concurrent_hash_map_simd<K, std::unique_ptr<Lazy>> m_map;
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
