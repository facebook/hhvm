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

#include "folly/ScopeGuard.h"

#include "hphp/runtime/base/program-functions.h"

namespace HPHP { namespace HHBBC {

namespace parallel {

//////////////////////////////////////////////////////////////////////

/*
 * Before using the parallel module, you can configure these to change
 * how much parallelism is used.
 */
extern size_t num_threads;
extern size_t work_chunk;

//////////////////////////////////////////////////////////////////////

/*
 * Call a function on each element of `inputs', in parallel.
 *
 * If `func' throws an exception, some of the work will not be
 * attempted.
 */
template<class Func, class Item>
void for_each(const std::vector<Item>& inputs, Func func) {
  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};

  std::vector<std::thread> workers;
  for (auto worker = size_t{0}; worker < num_threads; ++worker) {
    workers.push_back(std::thread([&] {
      try {
        hphp_session_init();
        auto const context = hphp_context_init();
        SCOPE_EXIT {
          hphp_context_exit(context, false /* psp */);
          hphp_session_exit();
          hphp_thread_exit();
        };

        for (;;) {
          auto start = index.fetch_add(work_chunk);
          auto const stop = std::min(start + work_chunk, inputs.size());
          if (start >= stop) break;
          for (auto i = start; i != stop; ++i) func(inputs[i]);
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
template<class Func, class Item>
std::vector<typename std::result_of<Func (Item)>::type>
map(const std::vector<Item>& inputs, Func func) {
  using RetT = typename std::result_of<Func (Item)>::type;

  std::vector<RetT> retVec(inputs.size());
  auto const retMem = &retVec[0];

  std::atomic<bool> failed{false};
  std::atomic<size_t> index{0};

  std::vector<std::thread> workers;
  for (auto worker = size_t{0}; worker < num_threads; ++worker) {
    workers.push_back(std::thread([&] {
      try {
        hphp_session_init();
        auto const context = hphp_context_init();
        SCOPE_EXIT {
          hphp_context_exit(context, false /* psp */);
          hphp_session_exit();
          hphp_thread_exit();
        };

        for (;;) {
          auto start = index.fetch_add(work_chunk);
          auto const stop = std::min(start + work_chunk, inputs.size());
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
