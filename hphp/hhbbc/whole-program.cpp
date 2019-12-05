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
#include "hphp/hhbbc/hhbbc.h"

#include <vector>
#include <algorithm>
#include <atomic>
#include <memory>
#include <set>

#include <folly/Memory.h>
#include <folly/ScopeGuard.h>

#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/debug.h"
#include "hphp/hhbbc/emit.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/parallel.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/stats.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_invoke("__invoke");

//////////////////////////////////////////////////////////////////////

enum class AnalyzeMode { NormalPass, ConstPass };
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

  explicit WorkResult(FuncAnalysisResult func)
    : type(WorkType::Func)
    , func(std::move(func))
  {}

  WorkResult(WorkResult&& wr) noexcept
    : type(wr.type)
  {
    switch (type) {
    case WorkType::Class:
      new (&cls) ClassAnalysis(std::move(wr.cls));
      break;
    case WorkType::Func:
      new (&func) FuncAnalysisResult(std::move(wr.func));
      break;
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
      func.~FuncAnalysisResult();
      break;
    }
  }

  WorkType type;
  union {
    ClassAnalysis cls;
    FuncAnalysisResult func;
  };
};

//////////////////////////////////////////////////////////////////////

template<typename F>
void all_unit_contexts(const php::Unit* u, F&& fun) {
  for (auto& c : u->classes) {
    for (auto& m : c->methods) {
      fun(Context { u, m.get(), c.get()});
    }
  }
  for (auto& f : u->funcs) {
    fun(Context { u, f.get() });
  }
  if (options.AnalyzePseudomains) {
    fun(Context { u, u->pseudomain.get() });
  }
}

std::vector<Context> const_pass_contexts(const php::Program& program) {
  std::vector<Context> ret;
  ret.reserve(program.constInits.size());
  for (auto func : program.constInits) {
    assertx(func);
    ret.push_back(Context { func->unit, func, func->cls });
  }
  return ret;
}

// Return all the WorkItems we'll need to start analyzing this
// program.
std::vector<WorkItem> initial_work(const php::Program& program,
                                   AnalyzeMode mode) {
  std::vector<WorkItem> ret;

  if (mode == AnalyzeMode::ConstPass) {
    auto const ctxs = const_pass_contexts(program);
    std::transform(begin(ctxs), end(ctxs), std::back_inserter(ret),
      [&] (Context ctx) { return WorkItem { WorkType::Func, ctx }; }
    );
    return ret;
  }

  for (auto& u : program.units) {
    /*
     * If we're not doing private property inference, schedule only
     * function-at-a-time work items.
     */
    if (!options.HardPrivatePropInference) {
      all_unit_contexts(u.get(), [&] (Context&& c) {
          ret.emplace_back(WorkType::Func, std::move(c));
        }
      );
      continue;
    }

    for (auto& c : u->classes) {
      if (c->closureContextCls) {
        // For class-at-a-time analysis, closures that are associated
        // with a class context are analyzed as part of that context.
        continue;
      }
      if (is_used_trait(*c)) {
        for (auto& f : c->methods) {
          ret.emplace_back(WorkType::Func,
                           Context { u.get(), f.get(), f->cls });
        }
      } else {
        ret.emplace_back(WorkType::Class,
                         Context { u.get(), nullptr, c.get() });
      }
    }
    for (auto& f : u->funcs) {
      ret.emplace_back(WorkType::Func, Context { u.get(), f.get() });
    }
    if (options.AnalyzePseudomains) {
      ret.emplace_back(
        WorkType::Func,
        Context { u.get(), u->pseudomain.get() }
      );
    }
  }
  return ret;
}

std::vector<Context> opt_prop_type_hints_contexts(const php::Program& program) {
  std::vector<Context> ret;
  for (auto& u : program.units) {
    for (auto& c : u->classes) {
      ret.emplace_back(Context { u.get(), nullptr, c.get() });
    }
  }
  return ret;
}

WorkItem work_item_for(const DependencyContext& d, AnalyzeMode mode) {
  switch (d.tag()) {
    case DependencyContextType::Class: {
      auto const cls = (const php::Class*)d.ptr();
      assertx(mode != AnalyzeMode::ConstPass &&
              options.HardPrivatePropInference &&
              !is_used_trait(*cls));
      return WorkItem { WorkType::Class, Context { cls->unit, nullptr, cls } };
    }
    case DependencyContextType::Func: {
      auto const func = (const php::Func*)d.ptr();
      auto const cls = !func->cls ? nullptr :
        func->cls->closureContextCls ?
        func->cls->closureContextCls : func->cls;
      assertx(!cls ||
              mode == AnalyzeMode::ConstPass ||
              !options.HardPrivatePropInference ||
              is_used_trait(*cls));
      return WorkItem {
        WorkType::Func,
        Context { func->unit, const_cast<php::Func*>(func), cls }
      };
    }
    case DependencyContextType::PropName:
      // We only record dependencies on static property names. We don't schedule
      // any work on their behalf.
      break;
  }
  always_assert(false);
}

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
 *
 */
void analyze_iteratively(Index& index, php::Program& program,
                         AnalyzeMode mode) {
  trace_time tracer(mode == AnalyzeMode::ConstPass ?
                    "analyze constants" : "analyze iteratively");

  // Counters, just for debug printing.
  std::atomic<uint32_t> total_funcs{0};
  std::atomic<uint32_t> total_classes{0};
  auto round = uint32_t{0};

  SCOPE_EXIT {
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease("total class visits %u\n", total_classes.load());
      Trace::traceRelease("total function visits %u\n", total_funcs.load());
    }
  };

  auto work = initial_work(program, mode);
  while (!work.empty()) {
    auto results = [&] {
      trace_time trace(
        "analyzing",
        folly::format("round {} -- {} work items", round, work.size()).str()
      );
      return parallel::map(
        work,
        // We have a folly::Optional just to keep the result type
        // DefaultConstructible.
        [&] (const WorkItem& wi) -> folly::Optional<WorkResult> {
          switch (wi.type) {
          case WorkType::Func:
            ++total_funcs;
            return WorkResult {
              analyze_func(index, wi.ctx, CollectionOpts{})
            };
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

    std::vector<DependencyContextSet> deps_vec{parallel::num_threads};

    auto update_func = [&] (FuncAnalysisResult& fa,
                            DependencyContextSet& deps) {
      SCOPE_ASSERT_DETAIL("update_func") {
        return "Updating Func: " + show(fa.ctx);
      };
      index.refine_return_info(fa, deps);
      index.refine_constants(fa, deps);
      update_bytecode(fa.ctx.func, std::move(fa.blockUpdates));

      if (options.AnalyzePublicStatics && mode == AnalyzeMode::NormalPass) {
        index.record_public_static_mutations(
          *fa.ctx.func,
          std::move(fa.publicSPropMutations)
        );
      }

      if (fa.resolvedConstants.size()) {
        index.refine_class_constants(fa.ctx,
                                     fa.resolvedConstants,
                                     deps);
      }
      for (auto& kv : fa.closureUseTypes) {
        assert(is_closure(*kv.first));
        if (index.refine_closure_use_vars(kv.first, kv.second)) {
          auto const func = find_method(kv.first, s_invoke.get());
          always_assert_flog(
            func != nullptr,
            "Failed to find __invoke on {} during index update\n",
            kv.first->name->data()
          );
          auto const ctx = Context { func->unit, func, kv.first };
          deps.insert(index.dependency_context(ctx));
        }
      }
    };

    auto update_class = [&] (ClassAnalysis& ca,
                             DependencyContextSet& deps) {
      {
        SCOPE_ASSERT_DETAIL("update_class") {
          return "Updating Class: " + show(ca.ctx);
        };
        index.refine_private_props(ca.ctx.cls,
                                   ca.privateProperties);
        index.refine_private_statics(ca.ctx.cls,
                                     ca.privateStatics);
        index.refine_bad_initial_prop_values(ca.ctx.cls,
                                             ca.badPropInitialValues,
                                             deps);
      }
      for (auto& fa : ca.methods)  update_func(fa, deps);
      for (auto& fa : ca.closures) update_func(fa, deps);
    };

    parallel::for_each(
      results,
      [&] (auto& result, size_t worker) {
        assertx(worker < deps_vec.size());
        switch (result->type) {
          case WorkType::Func:
            update_func(result->func, deps_vec[worker]);
            break;
          case WorkType::Class:
            update_class(result->cls, deps_vec[worker]);
            break;
        }
      }
    );

    {
      trace_time _("merging deps");
      for (auto& deps : deps_vec) {
        if (&deps == &deps_vec[0]) continue;
        for (auto& d : deps) deps_vec[0].insert(d);
        deps.clear();
      }
    }

    auto& deps = deps_vec[0];

    if (options.AnalyzePublicStatics && mode == AnalyzeMode::NormalPass) {
      index.refine_public_statics(deps);
    }

    work.clear();
    work.reserve(deps.size());
    for (auto& d : deps) work.push_back(work_item_for(d, mode));
  }
}

void prop_type_hint_pass(Index& index, php::Program& program) {
  trace_time tracer("optimize prop type-hints");

  auto const contexts = opt_prop_type_hints_contexts(program);
  parallel::for_each(
    contexts,
    [&] (Context ctx) { optimize_class_prop_type_hints(index, ctx); }
  );

  parallel::for_each(
    contexts,
    [&] (Context ctx) {
      index.mark_no_bad_redeclare_props(const_cast<php::Class&>(*ctx.cls));
    }
  );
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
template<typename F>
void final_pass(Index& index,
                php::Program& program,
                const StatsHolder& stats,
                F emitUnit) {
  trace_time final_pass("final pass");
  LitstrTable::fini();
  LitstrTable::init();
  LitstrTable::get().setWriting();
  index.freeze();
  auto const dump_dir = debug_dump_to();
  parallel::for_each(
    program.units,
    [&] (std::unique_ptr<php::Unit>& unit) {
      // optimize_func can remove 86*init methods from classes, so we
      // have to save the contexts for now.
      std::vector<Context> contexts;
      all_unit_contexts(unit.get(), [&] (Context&& ctx) {
          contexts.push_back(std::move(ctx));
        }
      );
      for (auto const& ctx : contexts) {
        optimize_func(index,
                      analyze_func(index, ctx, CollectionOpts{}),
                      true);
      }
      assert(check(*unit));
      state_after("optimize", *unit);
      if (!dump_dir.empty()) {
        if (Trace::moduleEnabledRelease(Trace::hhbbc_dump, 2)) {
          dump_representation(dump_dir, unit.get());
        }
        dump_index(dump_dir, index, unit.get());
      }
      collect_stats(stats, index, unit.get());
      emitUnit(*unit);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void UnitEmitterQueue::push(std::unique_ptr<UnitEmitter> ue) {
  assertx(!m_done.load(std::memory_order_relaxed));
  Lock lock(this);
  if (!ue) {
    m_done.store(true, std::memory_order_relaxed);
  } else {
    m_ues.push_back(std::move(ue));
  }
  notify();
}

std::unique_ptr<UnitEmitter> UnitEmitterQueue::pop() {
  Lock lock(this);
  while (m_ues.empty()) {
    if (m_done.load(std::memory_order_relaxed)) return nullptr;
    wait();
  }
  assertx(m_ues.size() > 0);
  auto ue = std::move(m_ues.front());
  assertx(ue != nullptr);
  m_ues.pop_front();
  return ue;
}

void UnitEmitterQueue::fetch(std::vector<std::unique_ptr<UnitEmitter>>& ues) {
  assertx(m_done.load(std::memory_order_relaxed));
  std::move(m_ues.begin(), m_ues.end(), std::back_inserter(ues));
  m_ues.clear();
}

void UnitEmitterQueue::reset() {
  m_ues.clear();
  m_done.store(false, std::memory_order_relaxed);
}

//////////////////////////////////////////////////////////////////////

namespace php {

void ProgramPtr::clear() { delete m_program; }

}

php::ProgramPtr make_program() {
  return php::ProgramPtr{ new php::Program };
}

void add_unit_to_program(const UnitEmitter* ue, php::Program& program) {
  parse_unit(program, ue);
}

void whole_program(php::ProgramPtr program,
                   UnitEmitterQueue& ueq,
                   std::unique_ptr<ArrayTypeTable::Builder>& arrTable,
                   int num_threads) {
  trace_time tracer("whole program");

  RuntimeOption::EvalLowStaticArrays = false;

  if (num_threads > 0) {
    parallel::num_threads = num_threads;
  }

  state_after("parse", *program);

  Index index(program.get());
  auto stats = allocate_stats();
  auto freeFuncMem = [&] (php::Func* fun) {
    fun->blocks = {};
  };
  auto emitUnit = [&] (php::Unit& unit) {
    auto ue = emit_unit(index, unit);
    if (RuntimeOption::EvalAbortBuildOnVerifyError && !ue->check(false)) {
      fprintf(
        stderr,
        "The optimized unit for %s did not pass verification, "
        "bailing because Eval.AbortBuildOnVerifyError is set\n",
        ue->m_filepath->data()
      );
      _Exit(1);
    }
    ueq.push(std::move(ue));
    for (auto& c : unit.classes) {
      for (auto& m : c->methods) {
        freeFuncMem(m.get());
      }
    }
    for (auto& f : unit.funcs) {
      freeFuncMem(f.get());
    }
    freeFuncMem(unit.pseudomain.get());
  };

  std::thread cleanup_pre;
  if (!options.NoOptimizations) {
    assert(check(*program));
    prop_type_hint_pass(index, *program);
    index.rewrite_default_initial_values(*program);
    index.use_class_dependencies(false);
    analyze_iteratively(index, *program, AnalyzeMode::ConstPass);
    // Defer initializing public static property types until after the
    // constant pass, to try to get better initial values.
    index.init_public_static_prop_types();
    index.use_class_dependencies(options.HardPrivatePropInference);
    analyze_iteratively(index, *program, AnalyzeMode::NormalPass);
    cleanup_pre = std::thread([&] { index.cleanup_for_final(); });
    index.mark_persistent_types_and_functions(*program);
    index.join_iface_vtable_thread();
    if (parallel::num_threads > parallel::final_threads) {
      parallel::num_threads = parallel::final_threads;
    }
    final_pass(index, *program, stats, emitUnit);
  } else {
    debug_dump_program(index, *program);
    index.join_iface_vtable_thread();
    parallel::for_each(
      program->units,
      [&] (const std::unique_ptr<php::Unit>& unit) {
        collect_stats(stats, index, unit.get());
        emitUnit(*unit);
      }
    );
  }

  auto const logging = Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
  // running cleanup_for_emit can take a while... start it as early as
  // possible, and run in its own thread.
  auto cleanup_post = std::thread([&] {
      auto const enable =
        logging && !Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
      Trace::BumpRelease bumper(Trace::hhbbc_time, -1, enable);
      index.cleanup_post_emit();
    }
  );

  print_stats(stats);

  arrTable = std::move(index.array_table_builder());
  ueq.push(nullptr);
  cleanup_pre.join();
  cleanup_post.join();
}

//////////////////////////////////////////////////////////////////////

}}
