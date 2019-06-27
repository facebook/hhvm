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
#ifndef incl_HHBBC_PARALLEL_H_
#define incl_HHBBC_PARALLEL_H_

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

#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/base/program-functions.h"

namespace HPHP { namespace HHBBC {

namespace parallel {

//////////////////////////////////////////////////////////////////////

/*
 * Before using the parallel module, you can configure these to change
 * how much parallelism is used.
 */
extern size_t num_threads;
extern size_t final_threads;
extern size_t work_chunk;

//////////////////////////////////////////////////////////////////////

namespace detail {

template<class Items>
auto size_info(Items&& items) {
  auto const size = items.size();
  if (!size) return std::make_tuple(size, size, size);
  // If work_chunk is too big to use all the threads, reduce it. Round
  // down to avoid reducing num_threads in the next step.
  auto const chunk = std::min(
    std::max(size_t{1}, size / num_threads), work_chunk
  );
  // If we still don't have enough chunks to use all the threads,
  // reduce the number of threads
  auto const threads = std::min((size + chunk - 1) / chunk, num_threads);
  return std::make_tuple(size, threads, chunk);
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

}

/*
 * Call a function on each element of `inputs', in parallel.
 *
 * If `func' throws an exception, some of the work will not be
 * attempted.
 */
template<class Func, class Items>
void for_each(Items&& inputs, Func func) {
  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};
  auto const info = detail::size_info(inputs);
  auto const size = std::get<0>(info);
  if (!size) return;
  auto const threads = std::get<1>(info);
  auto const chunk = std::get<2>(info);

  std::vector<std::thread> workers;
  for (auto worker = size_t{0}; worker < threads; ++worker) {
    workers.push_back(std::thread([&, worker] {
      try {
        HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};

        for (;;) {
          auto start = index.fetch_add(chunk);
          auto const stop = std::min(start + chunk, size);
          if (start >= stop) break;
          for (auto i = start; i != stop; ++i) {
            detail::caller(func,
                           std::forward<Items>(inputs)[i],
                           worker);
          }
        }
      } catch (const std::exception& e) {
        std::fprintf(stderr,
          "worker thread exited with exception: %s\n", e.what());
        failed = true;
      }
    }));
  }

  for (auto& t : workers) t.join();

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
 * If `func' throws an exception, the results of the output vector
 * will contain some default-constructed values.
 */
template<class Func, class Items>
auto map(Items&& inputs, Func func) -> std::vector<decltype(func(inputs[0]))> {
  auto const info = detail::size_info(inputs);
  auto const size = std::get<0>(info);
  std::vector<decltype(func(inputs[0]))> retVec(size);
  if (!size) return retVec;
  auto const threads = std::get<1>(info);
  auto const chunk = std::get<2>(info);

  auto const retMem = &retVec[0];

  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};

  std::vector<std::thread> workers;
  for (auto worker = size_t{0}; worker < threads; ++worker) {
    workers.push_back(std::thread([&] {
      try {
        HphpSessionAndThread _{Treadmill::SessionKind::HHBBC};

        for (;;) {
          auto start = index.fetch_add(chunk);
          auto const stop = std::min(start + chunk, inputs.size());
          if (start >= stop) break;

          std::transform(
            begin(inputs) + start, begin(inputs) + stop,
            retMem + start,
            func
          );
        }
      } catch (const std::runtime_error& e) {
        std::fprintf(stderr,
          "worker thread exited with exception: %s\n", e.what());
        failed = true;
      }
    }));
  }

  for (auto& t : workers) t.join();
  if (failed) throw std::runtime_error("parallel::map failed");

  return retVec;
}

//////////////////////////////////////////////////////////////////////

}

}}

#endif
