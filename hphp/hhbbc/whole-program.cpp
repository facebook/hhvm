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
const StaticString s_86cinit("86cinit");

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

// Return a Context for every function in the Program.
std::vector<Context> all_function_contexts(const php::Program& program) {
  std::vector<Context> ret;

  for (auto& u : program.units) {
    for (auto& c : u->classes) {
      for (auto& m : c->methods) {
        ret.push_back(Context { u.get(), m.get(), c.get()});
      }
    }
    for (auto& f : u->funcs) {
      ret.push_back(Context { u.get(), f.get() });
    }
    if (options.AnalyzePseudomains) {
      ret.push_back(Context { u.get(), u->pseudomain.get() });
    }
  }
  return ret;
}

std::vector<Context> const_pass_contexts(const php::Program& program,
                                         php::Program::CInit ci) {
  std::vector<Context> ret;
  ret.reserve(program.constInits.size());
  program.constInits.foreach([&](uintptr_t f) {
      if (f & ci) {
        auto const func = static_cast<php::Func*>(
          reinterpret_cast<void*>(f & ~php::Program::ForAll));
        ret.push_back(Context { func->unit, func, func->cls });
      }
    });
  return ret;
}

// Return all the WorkItems we'll need to start analyzing this
// program.
std::vector<WorkItem> initial_work(const php::Program& program,
                                   AnalyzeMode mode) {
  std::vector<WorkItem> ret;

  if (mode == AnalyzeMode::ConstPass) {
    auto const ctxs = const_pass_contexts(program, php::Program::ForAnalyze);
    std::transform(begin(ctxs), end(ctxs), std::back_inserter(ret),
      [&] (Context ctx) { return WorkItem { WorkType::Func, ctx }; }
    );
    return ret;
  }
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
              analyze_func(index, wi.ctx,
                           mode == AnalyzeMode::ConstPass ?
                           CollectionOpts{} :
                           CollectionOpts::TrackConstantArrays)
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

    index.update_class_aliases();
    work.clear();
    work.reserve(deps.size());
    for (auto& d : deps) work.push_back(work_item_for(d, mode));
  }
}

void constant_pass(Index& index, php::Program& program) {
  if (!options.HardConstProp) return;
  index.use_class_dependencies(false);
  analyze_iteratively(index, program, AnalyzeMode::ConstPass);

  auto save = options.InsertAssertions;
  options.InsertAssertions = false;
  index.freeze();

  trace_time optimize_constants("optimize constants");
  parallel::for_each(
    const_pass_contexts(program, php::Program::ForAll),
    [&] (Context ctx) {
      optimize_func(index, analyze_func(index, ctx, CollectionOpts{}), false);
    }
  );

  index.thaw();
  options.InsertAssertions = save;
}

void mark_persistent_static_properties(const Index& index,
                                       php::Program& program) {
  trace_time update("mark persistent static properties");
  for (auto& unit : program.units) {
    for (auto& cls : unit->classes) {
      for (auto& prop : cls->properties) {
        if (index.lookup_public_static_immutable(cls.get(), prop.name)) {
          prop.attrs |= AttrPersistent;
        }
      }
    }
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
void final_pass(Index& index, php::Program& program) {
  trace_time final_pass("final pass");
  index.freeze();
  parallel::for_each(
    all_function_contexts(program),
    [&] (Context ctx) {
      optimize_func(index,
                    analyze_func(index,
                                 ctx,
                                 CollectionOpts::TrackConstantArrays),
                    true);
    }
  );
}

//////////////////////////////////////////////////////////////////////

template<class Container>
std::unique_ptr<php::Program> parse_program(Container units) {
  trace_time tracer("parse");
  auto ret = std::make_unique<php::Program>(units.size());
  ret->units = parallel::map(
    units,
    [&] (std::unique_ptr<UnitEmitter>& ue) {
      return parse_unit(*ret, std::move(ue));
    }
  );
  return ret;
}

template<typename F>
void make_unit_emitters(
  const Index& index,
  const php::Program& program,
  F outFunc) {
  trace_time trace("make_unit_emitters");
  return parallel::for_each(
    program.units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      outFunc(emit_unit(index, *unit));
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

void whole_program(std::vector<std::unique_ptr<UnitEmitter>> ues,
                   UnitEmitterQueue& ueq,
                   std::unique_ptr<ArrayTypeTable::Builder>& arrTable,
                   int num_threads) {
  trace_time tracer("whole program");

  RuntimeOption::EvalLowStaticArrays = false;

  if (num_threads > 0) {
    parallel::num_threads = num_threads;
  }

  auto program = parse_program(std::move(ues));

  state_after("parse", *program);

  folly::Optional<Index> index;
  index.emplace(program.get());
  if (!options.NoOptimizations) {
    while (true) {
      try {
        assert(check(*program));
        prop_type_hint_pass(*index, *program);
        index->rewrite_default_initial_values(*program);
        constant_pass(*index, *program);
        // Defer initializing public static property types until after the
        // constant pass, to try to get better initial values.
        index->init_public_static_prop_types();
        index->use_class_dependencies(options.HardPrivatePropInference);
        analyze_iteratively(*index, *program, AnalyzeMode::NormalPass);
        final_pass(*index, *program);
        index->mark_persistent_classes_and_functions(*program);
        state_after("optimize", *program);
        assert(check(*program));
        break;
      } catch (Index::rebuild& rebuild) {
        FTRACE(1, "whole_program: rebuilding index\n");
        index.emplace(program.get(), &rebuild);
        continue;
      }
    }
  }

  if (options.AnalyzePublicStatics) {
    mark_persistent_static_properties(*index, *program);
  }

  debug_dump_program(*index, *program);
  print_stats(*index, *program);

  // running cleanup_for_emit can take a while... do it in parallel
  // with making the unit emitters.
  folly::Baton<> done;
  auto cleanup_thread = std::thread([&] { index->cleanup_for_emit(&done); });

  LitstrTable::fini();
  LitstrTable::init();
  LitstrTable::get().setWriting();
  make_unit_emitters(*index, *program, [&] (std::unique_ptr<UnitEmitter> ue) {
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
  });

  arrTable = std::move(index->array_table_builder());
  done.post();
  cleanup_thread.join();
  ueq.push(nullptr);
}

//////////////////////////////////////////////////////////////////////

}}
