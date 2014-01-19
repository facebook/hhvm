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
#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

enum class WorkType { Class, Func };

struct WorkItem {
  explicit WorkItem(WorkType type, Context ctx)
    : type(type)
    , ctx(ctx)
  {}

  bool operator<(const WorkItem& o) const {
    return type < o.type ? true :
           o.type < type ? false :
           ctx < o.ctx;
  }

  bool operator==(const WorkItem& o) const {
    return type == o.type && ctx == o.ctx;
  }

  WorkType type;
  Context ctx;
};

struct WorkResult {
  explicit WorkResult(ClassAnalysis cls)
    : type(WorkType::Class)
    , cls(std::move(cls))
  {}

  explicit WorkResult(FuncAnalysis func)
    : type(WorkType::Func)
    , func(std::move(func))
  {}

  WorkResult(WorkResult&& wr) noexcept
    : type(wr.type)
  {
    switch (type) {
    case WorkType::Class: new (&cls) ClassAnalysis(std::move(wr.cls));  break;
    case WorkType::Func:  new (&func) FuncAnalysis(std::move(wr.func)); break;
    }
  }

  WorkResult& operator=(WorkResult&& o) noexcept {
    this->~WorkResult();
    switch (o.type) {
    case WorkType::Class: new (this) WorkResult(std::move(o.cls)); break;
    case WorkType::Func:  new (this) WorkResult(std::move(o.func)); break;
    }
    return *this;
  }

  ~WorkResult() {
    switch (type) {
    case WorkType::Class:
      cls.~ClassAnalysis();
      break;
    case WorkType::Func:
      func.~FuncAnalysis();
      break;
    }
  }

  WorkType type;
  union {
    ClassAnalysis cls;
    FuncAnalysis func;
  };
};

//////////////////////////////////////////////////////////////////////

// Return a Context for every function in the Program.
std::vector<Context> all_function_contexts(const php::Program& program) {
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
  return ret;
}

// Return all the WorkItems we'll need to start analyzing this
// program.
std::vector<WorkItem> initial_work(const php::Program& program) {
  std::vector<WorkItem> ret;

  /*
   * If we're not doing private property inference, schedule only
   * function-at-a-time work items.
   */
  if (!options.HardPrivatePropInference) {
    auto const ctxs = all_function_contexts(program);
    std::transform(begin(ctxs), end(ctxs), std::back_inserter(ret),
      [&] (Context ctx) { return WorkItem { WorkType::Func, ctx }; }
    );
    return ret;
  }

  for (auto& u : program.units) {
    for (auto& c : u->classes) {
      ret.emplace_back(WorkType::Class,
                       Context { borrow(u), nullptr, borrow(c) });
    }
    for (auto& f : u->funcs) {
      ret.emplace_back(WorkType::Func, Context { borrow(u), borrow(f) });
    }
  }
  return ret;
}

WorkItem work_item_for(Context ctx) {
  if (!options.HardPrivatePropInference) {
    return WorkItem { WorkType::Func, ctx };
  }

  return ctx.cls == nullptr
    ? WorkItem { WorkType::Func, ctx }
    : WorkItem { WorkType::Class, Context { ctx.unit, nullptr, ctx.cls } };
}

void optimize(Index& index, php::Program& program) {
  assert(check(program));
  trace_time tracer("optimize");
  SCOPE_EXIT { state_after("optimize", program); };

  // Counters, just for debug printing.
  std::atomic<uint32_t> total_funcs{0};
  std::atomic<uint32_t> total_classes{0};
  auto round = uint32_t{0};

  /*
   * Algorithm:
   *
   * Start by running an analyze pass on every class or free function.
   * During analysis, information about functions or classes will be
   * requested from the Index, which initially won't really know much,
   * but will record a dependency.  This part is done in parallel: no
   * passes are mutating anything, just reading from the Index.
   *
   * After a pass, we do a single-threaded "update" step to prepare
   * for the next pass: for each function or class that was analyzed,
   * note the facts we learned that may aid analyzing other functions
   * in the program, and register them in the index.
   *
   * If any of these facts are more useful than they used to be, add
   * all the Contexts that had a dependency on the new information to
   * the work list again, in case they can do better based on the new
   * fact.  (This only applies to function analysis information right
   * now.)
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
        // We have a folly::Optional just to keep the result type
        // DefaultConstructible.
        [&] (const WorkItem& wi) -> folly::Optional<WorkResult> {
          switch (wi.type) {
          case WorkType::Func:
            ++total_funcs;
            return WorkResult { analyze_func(index, wi.ctx) };
          case WorkType::Class:
            ++total_classes;
            return WorkResult { analyze_class(index, wi.ctx) };
          }
          not_reached();
        }
      );
    }();

    ++round;
    trace_time update_time("updating");

    std::set<WorkItem> revisit;

    auto update_func = [&] (const FuncAnalysis& fa) {
      auto deps = index.refine_return_type(fa.ctx.func, fa.inferredReturn);
      for (auto& d : deps) revisit.insert(work_item_for(d));
    };

    for (auto& result : results) {
      switch (result->type) {
      case WorkType::Func:
        update_func(result->func);
        break;
      case WorkType::Class:
        index.refine_private_props(result->cls.ctx.cls,
                                   result->cls.privateProperties);
        for (auto& fa : result->cls.methods) update_func(fa);
        break;
      }
    }

    work.assign(begin(revisit), end(revisit));
  }

  if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
    Trace::traceRelease("total class visits %u\n", total_classes.load());
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
  parallel_for_each(
    all_function_contexts(program),
    [&] (Context ctx) { optimize_func(index, analyze_func(index, ctx)); }
  );
}

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

std::vector<std::unique_ptr<UnitEmitter>>
whole_program(std::vector<std::unique_ptr<UnitEmitter>> ues) {
  trace_time tracer("whole program");

  LitstrTable::get().setReading();

  auto program = parse_program(ues);
  ues.clear();

  state_after("parse", *program);

  Index index{borrow(program)};
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
