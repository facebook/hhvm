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

#include <stdexcept>
#include <iterator>
#include <cstdio>
#include <thread>
#include <vector>
#include <type_traits>
#include <string>
#include <atomic>
#include <algorithm>
#include <exception>

#include <folly/ScopeGuard.h>
#include <folly/synchronization/LifoSem.h>

#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/base/program-functions.h"

namespace HPHP::HHBBC {

namespace parallel {

//////////////////////////////////////////////////////////////////////

/*
 * Before using the parallel module, you can configure these to change
 * how much parallelism is used.
 */
extern size_t num_threads;
extern size_t final_threads;

//////////////////////////////////////////////////////////////////////

/*
 * Track if the current thread is within a parallel call. If so, any
 * inner calls to the parallel functions will instead operate
 * serially. This prevents a huge blowup in threads from recursively
 * spawning.
 */
extern thread_local bool tl_inParallel;

//////////////////////////////////////////////////////////////////////

namespace detail {

//////////////////////////////////////////////////////////////////////

template<class Items>
auto size_info(Items&& items) {
  auto const size = items.size();
  if (!size) return std::make_tuple(size, size);
  auto const threads = std::min(num_threads, size);
  return std::make_tuple(size, threads);
}

template<class Func, class Item>
auto caller(const Func& func, Item&& item, size_t worker) ->
  decltype(func(std::forward<Item>(item), worker)) {
  return func(std::forward<Item>(item), worker);
}

template<class Func, class Item>
auto caller(const Func& func, Item&& item, size_t worker) ->
  decltype(func(std::forward<Item>(item))) {
  return func(std::forward<Item>(item));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

/*
 * If you need to call one of the parallel functions often (for
 * example, in an inner loop), the time spent creating and destroying
 * the threads can be significant. This class allows you to create a
 * fixed thread pool, which can be created once, then re-used within
 * the loop.
 */
struct thread_pool {
  thread_pool();
  ~thread_pool();
  template <typename F> void run(size_t, F);
private:
  void thread_run(size_t);

  folly::Function<void(size_t)> func;
  std::vector<std::thread> threads;

  folly::LifoSem active;
  folly::LifoSem wait;
};

template <typename F>
void thread_pool::run(size_t items, F f) {
  func = std::move(f);
  auto const num = std::min(items, threads.size());
  active.post(num);
  for (size_t i = 0; i < num; ++i) wait.wait();
  func = nullptr;
}

//////////////////////////////////////////////////////////////////////

/*
 * Call a function on each element of `inputs', in parallel.
 *
 * If a thread pool has been provided, that will be used. Otherwise it
 * will spin up it's own threads.
 *
 * If `func' throws an exception, some of the work will not be
 * attempted.
 */
template<class Func, class Items>
void for_each(Items&& inputs, Func func, thread_pool* pool = nullptr) {
  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};
  auto const info = detail::size_info(inputs);
  auto const size = std::get<0>(info);
  if (!size) return;
  auto const threads = std::get<1>(info);

  auto const work = [size, &index, &inputs, &func, &failed] (size_t worker) {
    try {
      while (true) {
        auto const i = index++;
        if (i >= size) break;
        detail::caller(
          func,
          std::forward<Items>(inputs)[i],
          worker
        );
      }
    } catch (const std::exception& e) {
      std::fprintf(
        stderr,
        "worker thread exited with exception: %s\n",
        e.what()
      );
      failed = true;
    }
  };

  if (tl_inParallel) {
    work(0);
  } else if (!pool) {
    std::vector<std::thread> workers;
    for (auto worker = size_t{0}; worker < threads; ++worker) {
      workers.emplace_back(
        [worker, &work] {
          tl_inParallel = true;
          SCOPE_EXIT { tl_inParallel = false; };
          HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
          work(worker);
        }
      );
    }
    for (auto& t : workers) t.join();
  } else {
    pool->run(inputs.size(), work);
  }

  if (failed) throw std::runtime_error("parallel::for_each failed");
}

//////////////////////////////////////////////////////////////////////

/*
 * Call a function that produces a return value for each element of
 * `inputs' in parallel, and collect the results.
 *
 * Requires: the type returned from the function call must be
 * DefaultConstructible, and either MoveAssignable or Assignable.
 *
 * If a thread pool has been provided, that will be used. Otherwise it
 * will spin up it's own threads.
 *
 * If `func' throws an exception, the results of the output vector
 * will contain some default-constructed values.
 */
template<class Func, class Items>
auto map(Items&& inputs, Func func, thread_pool* pool = nullptr)
  -> std::vector<decltype(func(inputs[0]))> {
  auto const info = detail::size_info(inputs);
  auto const size = std::get<0>(info);
  std::vector<decltype(func(inputs[0]))> retVec(size);
  if (!size) return retVec;
  auto const threads = std::get<1>(info);

  auto const retMem = &retVec[0];
  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};

  auto const work = [size, &index, &inputs, &func, &failed, &retMem] (size_t) {
    try {
      while (true) {
        auto const i = index++;
        if (i >= size) break;
        retMem[i] = func(std::forward<Items>(inputs)[i]);
      }
    } catch (const std::runtime_error& e) {
      std::fprintf(
        stderr,
        "worker thread exited with exception: %s\n",
        e.what()
      );
      failed = true;
    }
  };

  if (tl_inParallel) {
    work(0);
  } else if (!pool) {
    std::vector<std::thread> workers;
    for (auto worker = size_t{0}; worker < threads; ++worker) {
      workers.emplace_back(
        [worker, &work] {
          tl_inParallel = true;
          SCOPE_EXIT { tl_inParallel = false; };
          HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
          work(worker);
        }
      );
    }
    for (auto& t : workers) t.join();
  } else {
    pool->run(inputs.size(), work);
  }

  if (failed) throw std::runtime_error("parallel::map failed");
  return retVec;
}

//////////////////////////////////////////////////////////////////////

/*
 * Call a function that produces a return value for each integer from
 * 0 to count-1 in parallel, and collect the results.
 *
 * Requires: the type returned from the function call must be
 * DefaultConstructible, and either MoveAssignable or Assignable.
 *
 * If a thread pool has been provided, that will be used. Otherwise it
 * will spin up it's own threads.
 *
 * If `func' throws an exception, the results of the output vector
 * will contain some default-constructed values.
 */
template<typename Func>
auto gen(size_t count, Func func, thread_pool* pool = nullptr)
  -> std::vector<decltype(detail::caller(func,0,0))> {
  std::vector<decltype(detail::caller(func,0,0))> retVec(count);
  if (!count) return retVec;

  auto const threads = std::min(num_threads, count);

  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};

  auto const work = [count, &index, &func, &failed, &retVec] (size_t worker) {
    try {
      while (true) {
        auto const i = index++;
        if (i >= count) break;
        retVec[i] = detail::caller(func, i, worker);
      }
    } catch (const std::runtime_error& e) {
      std::fprintf(
        stderr,
        "worker thread exited with exception: %s\n",
        e.what()
      );
      failed = true;
    }
  };

  if (tl_inParallel) {
    work(0);
  } else if (!pool) {
    std::vector<std::thread> workers;
    for (auto worker = size_t{0}; worker < threads; ++worker) {
      workers.emplace_back(
        [worker, &work] {
          tl_inParallel = true;
          SCOPE_EXIT { tl_inParallel = false; };
          HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
          work(worker);
        }
      );
    }
    for (auto& t : workers) t.join();
  } else {
    pool->run(count, work);
  }

  if (failed) throw std::runtime_error("parallel::gen failed");
  return retVec;
}

//////////////////////////////////////////////////////////////////////

namespace detail {

template<size_t Total>
void populateThreads(std::array<std::thread, Total>&,
                     std::atomic<bool>&,
                     size_t) {}

template<size_t Total, typename Func, typename... Funcs>
void populateThreads(std::array<std::thread, Total>& threads,
                     std::atomic<bool>& failed,
                     size_t idx,
                     Func&& func,
                     Funcs&&... funcs) {
  auto const work = [&] {
    try {
      func();
    } catch (const std::runtime_error& e) {
      std::fprintf(
        stderr, "worker thread exited with exception: %s\n", e.what()
      );
      failed = true;
    }
  };

  if (tl_inParallel) {
    work();
  } else {
    threads[idx] = std::thread(
      [&] {
        tl_inParallel = true;
        SCOPE_EXIT { tl_inParallel = false; };
        HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};
        work();
      }
    );
  }
  populateThreads(threads, failed, idx+1, std::forward<Funcs>(funcs)...);
}

}

// Run N callables in parallel
template<typename... Funcs>
void parallel(Funcs&&... funcs) {
  std::array<std::thread, sizeof...(Funcs)> threads;
  std::atomic<bool> failed{false};
  detail::populateThreads(threads, failed, 0, std::forward<Funcs>(funcs)...);
  if (!tl_inParallel) {
    for (auto& t : threads) t.join();
  }
  if (failed) throw std::runtime_error("parallel::parallel failed");
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}
