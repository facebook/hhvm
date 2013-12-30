/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/hhbbc.h"

#include <vector>
#include <algorithm>

#include "folly/Memory.h"
#include "folly/ScopeGuard.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/emit.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/debug.h"
#include "hphp/hhbbc/abstract-interp.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

template<class Container>
std::unique_ptr<php::Program> parse_program(const Container& units) {
  trace_time tracer("parse");
  auto ret = folly::make_unique<php::Program>();
  ret->units = parallel_map(
    units,
    [&] (const std::unique_ptr<UnitEmitter>& ue) {
      return parse_unit(*ue);
    }
  );
  return ret;
}

std::vector<std::unique_ptr<UnitEmitter>>
make_unit_emitters(const php::Program& program) {
  trace_time trace("make_unit_emitters");
  return parallel_map(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      return emit_unit(*unit);
    }
  );
}

// Return a Context for every function in the Program.
std::vector<Context> initial_work(const php::Program& program) {
  std::vector<Context> ret;

  for (auto& u : program.units) {
    for (auto& c : u->classes) {
      for (auto& m : c->methods) {
        ret.push_back(Context { borrow(u), borrow(m), borrow(c)});
      }
    }
    for (auto& f : u->funcs) {
      ret.push_back(Context { borrow(u), borrow(f) });
    }
  }
  std::random_shuffle(begin(ret), end(ret)); // heh
  return ret;
}

void optimize(Index& index, php::Program& program) {
  assert(check(program));
  trace_time tracer("optimize");
  SCOPE_EXIT { state_after("optimize", program); };

  // Counters, just for debug printing.
  std::atomic<uint32_t> total_funcs{0};
  auto round = uint32_t{0};

  /*
   * Algorithm:
   *
   * Start by running an analyze pass on every function.  During
   * analysis, information about functions or classes will be
   * requested from the Index, which initially won't really know much,
   * but will record a dependency.  This part is done in parallel: no
   * passes are mutating anything, just reading from the Index.
   *
   * After a pass, we do a single-threaded "update" step to prepare
   * for the next pass: for each function that was analyzed, note the
   * facts we learned that may aid analyzing other functions in the
   * program, and register them in the index.  At this point, if any
   * of these facts are more useful than they used to be, add all the
   * Contexts that had a dependency on the new information to the work
   * list again, in case they can do better based on the new fact.
   *
   * Repeat until the work list is empty.
   */
  auto work = initial_work(program);
  while (!work.empty()) {
    auto const results = [&] {
      trace_time trace(
        "analyzing",
        folly::format("round {} -- {} work items", round, work.size()).str()
      );
      return parallel_map(
        work,
        [&] (const Context& ctx) -> folly::Optional<FuncAnalysis> {
          total_funcs.fetch_add(1, std::memory_order_relaxed);
          return analyze_func(index, ctx);
        }
      );
    }();
    work.clear();

    ++round;
    trace_time update_time("updating");

    std::set<Context> revisit;
    for (auto i = size_t{0}; i < results.size(); ++i) {
      auto& result = *results[i];

      assert(result.ctx.func == work[i].func);
      assert(result.ctx.cls == work[i].cls);
      assert(result.ctx.unit == work[i].unit);

      auto deps = index.refine_return_type(
        result.ctx.func, result.inferredReturn
      );
      for (auto& d : deps) revisit.insert(d);
    }

    std::copy(begin(revisit), end(revisit), std::back_inserter(work));
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
    Trace::traceRelease("total function visits %u\n", total_funcs.load());
  }

  /*
   * Finally, use the results of all these iterations to perform
   * optimization.  This reanalyzes every function using our
   * now-very-updated Index, and then runs optimize_func with the
   * results.
   *
   * We do this in parallel: all the shared information is queried out
   * of the index, and each thread is allowed to modify the bytecode
   * for the function it is looking at.
   *
   * NOTE: currently they can't modify anything other than the
   * bytecode/Blocks, because other threads may be doing unlocked
   * queries to php::Func and php::Class structures.
   */
  trace_time final_pass("final pass");
  work = initial_work(program);
  parallel_for_each(
    initial_work(program),
    [&] (Context ctx) { optimize_func(index, analyze_func(index, ctx)); }
  );
}

}

//////////////////////////////////////////////////////////////////////

std::vector<std::unique_ptr<UnitEmitter>>
whole_program(std::vector<std::unique_ptr<UnitEmitter>> ues,
              const Options& opts) {
  trace_time tracer("whole program");

  LitstrTable::get().setReading();

  // TODO(#3343796): hphpc will give us unit emitters for systemlib,
  // but currently we can't handle it.
  static const char systemlib_name[] = "/:systemlib";
  ues.erase(
    std::remove_if(begin(ues), end(ues),
      [&] (const std::unique_ptr<UnitEmitter>& ue) {
        return !strncmp(systemlib_name,
                        ue->getFilepath()->data(),
                        sizeof systemlib_name - 1);
      }
    ),
    end(ues)
  );

  auto program = parse_program(ues);
  ues.clear();

  state_after("parse", *program);

  Index index{borrow(program), opts};
  optimize(index, *program);

  if (Trace::moduleEnabledRelease(Trace::hhbbc_dump, 1)) {
    debug_dump_program(*program);
  }

  LitstrTable::get().setWriting();
  ues = make_unit_emitters(*program);

  return ues;
}

//////////////////////////////////////////////////////////////////////

}}
