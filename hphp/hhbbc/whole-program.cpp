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

#include <folly/AtomicLinkedList.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>

#include "hphp/runtime/vm/repo-global-data.h"
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
#include "hphp/hhbbc/wide-func.h"

#include "hphp/util/extern-worker.h"
#include "hphp/util/struct-log.h"

namespace HPHP {

using namespace extern_worker;

namespace HHBBC {

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
}

std::vector<Context> const_pass_contexts(const php::Program& program) {
  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * Must include every function with a DefCns for correctness; cinit,
   * pinit and sinit functions are added to improve overall
   * performance.
   */
  std::vector<Context> ret;
  for (auto const& u : program.units) {
    for (auto const& c : u->classes) {
      for (auto const& m : c->methods) {
        if (m->name == s_86cinit.get() ||
            m->name == s_86pinit.get() ||
            m->name == s_86sinit.get() ||
            m->name == s_86linit.get()) {
          ret.emplace_back(Context { u.get(), m.get(), c.get() });
        }
      }
    }
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
              is_used_trait(*cls));
      return WorkItem { WorkType::Func, Context { func->unit, func, cls } };
    }
    case DependencyContextType::Prop:
    case DependencyContextType::FuncFamily:
      // We only record dependencies on these. We don't schedule any
      // work on their behalf.
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

  std::vector<DependencyContextSet> deps_vec{parallel::num_threads};

  auto work = initial_work(program, mode);
  while (!work.empty()) {
    auto results = [&] {
      trace_time trace(
        "analyzing",
        folly::format("round {} -- {} work items", round, work.size()).str()
      );
      return parallel::map(
        work,
        // We have a Optional just to keep the result type
        // DefaultConstructible.
        [&] (const WorkItem& wi) -> Optional<WorkResult> {
          switch (wi.type) {
          case WorkType::Func: {
            ++total_funcs;
            auto const wf = php::WideFunc::cns(wi.ctx.func);
            auto const ctx = AnalysisContext { wi.ctx.unit, wf, wi.ctx.cls };
            return WorkResult { analyze_func(index, ctx, CollectionOpts{}) };
          }
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

    auto update_func = [&] (FuncAnalysisResult& fa,
                            DependencyContextSet& deps) {
      SCOPE_ASSERT_DETAIL("update_func") {
        return "Updating Func: " + show(fa.ctx);
      };
      // This const_cast is safe since no two threads update the same Func.
      auto func = php::WideFunc::mut(const_cast<php::Func*>(fa.ctx.func));
      index.refine_return_info(fa, deps);
      index.refine_constants(fa, deps);
      update_bytecode(func, std::move(fa.blockUpdates));

      if (mode == AnalyzeMode::NormalPass) {
        index.record_public_static_mutations(
          *func,
          std::move(fa.publicSPropMutations)
        );
      }

      if (fa.resolvedConstants.size()) {
        index.refine_class_constants(fa.ctx, fa.resolvedConstants, deps);
      }
      for (auto& kv : fa.closureUseTypes) {
        assertx(is_closure(*kv.first));
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
        result.reset();
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

    if (mode == AnalyzeMode::NormalPass) {
      index.refine_public_statics(deps);
    }

    work.clear();
    work.reserve(deps.size());
    for (auto& d : deps) work.push_back(work_item_for(d, mode));
    deps.clear();
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
      for (auto const& context : contexts) {
        // This const_cast is safe since no two threads update the same Func.
        auto func = php::WideFunc::mut(const_cast<php::Func*>(context.func));
        auto const ctx = AnalysisContext { context.unit, func, context.cls };
        optimize_func(index, analyze_func(index, ctx, CollectionOpts{}), func);
      }
      assertx(check(*unit));
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

// Right now Key is nothing, and Value is the UnitEmitter.

struct WholeProgramInput::Key::Impl {};
struct WholeProgramInput::Value::Impl {
  std::unique_ptr<php::Unit> unit;
};
struct WholeProgramInput::Impl {
  folly::AtomicLinkedList<Ref<Value>> values;
};

WholeProgramInput::WholeProgramInput() : m_impl{new Impl} {}

void WholeProgramInput::add(Key, extern_worker::Ref<Value> v) {
  assertx(m_impl);
  m_impl->values.insertHead(std::move(v));
}

std::vector<std::pair<WholeProgramInput::Key, WholeProgramInput::Value>>
WholeProgramInput::make(std::unique_ptr<UnitEmitter> ue) {
  assertx(ue);
  Key key;
  Value value;
  value.m_impl.reset(new Value::Impl);
  value.m_impl->unit = parse_unit(*ue);
  std::vector<std::pair<Key, Value>> out;
  out.emplace_back(std::move(key), std::move(value));
  return out;
}

void WholeProgramInput::Key::serde(BlobEncoder&) const {}
void WholeProgramInput::Key::serde(BlobDecoder&) {}

void WholeProgramInput::Value::serde(BlobEncoder& sd) const {
  assertx(m_impl);
  sd(m_impl->unit);
}
void WholeProgramInput::Value::serde(BlobDecoder& sd) {
  m_impl.reset(new Impl);
  sd(m_impl->unit);
}

void WholeProgramInput::Key::Deleter::operator()(Impl* i) const {
  delete i;
}
void WholeProgramInput::Value::Deleter::operator()(Impl* i) const {
  delete i;
}
void WholeProgramInput::Deleter::operator()(Impl* i) const {
  delete i;
}

//////////////////////////////////////////////////////////////////////

namespace {

// Construct a php::Program from a WholeProgramInput. Right now this
// is just loading the Values and adding them to a php::Program. The
// units have already been constructed in extern-worker jobs.
std::unique_ptr<php::Program> make_program(
    WholeProgramInput input,
    std::unique_ptr<coro::TicketExecutor> executor,
    std::unique_ptr<Client> client,
    const DisposeCallback& dispose) {
  trace_time tracer("materialize inputs");

  // For speed, split up the unit loading into chunks.
  constexpr size_t kLoadChunkSize = 500;

  using V = WholeProgramInput::Value;
  std::vector<std::vector<Ref<V>>> chunked;
  std::vector<Ref<V>> current;
  input.m_impl->values.sweep(
    [&] (Ref<V>&& v) {
      current.emplace_back(std::move(v));
      if (current.size() >= kLoadChunkSize) {
        chunked.emplace_back(std::move(current));
      }
    }
  );
  if (!current.empty()) chunked.emplace_back(std::move(current));

  auto program = std::make_unique<php::Program>();
  auto const loadAndParse = [&] (std::vector<Ref<V>> r) -> coro::Task<void> {
    auto const values = HPHP_CORO_AWAIT(client->load(std::move(r)));

    // Add every parsed unit into the Program.
    for (auto& v : values) {
      // Check for any unit which failed the verifier. This causes us
      // to exit immediately with an error.
      if (auto const& fi = v.m_impl->unit->fatalInfo) {
        if (!fi->fatalLoc) {
          fprintf(stderr, "%s", fi->fatalMsg.c_str());
          _Exit(1);
        }
      }

      // Assign unique ids to each function:
      for (auto& f : v.m_impl->unit->funcs) {
        f->idx = program->nextFuncId++;
      }
      for (auto& c : v.m_impl->unit->classes) {
        for (auto& m : c->methods) {
          m->idx = program->nextFuncId++;
        }
      }
    }

    {
      std::scoped_lock<std::mutex> _{program->lock};
      for (auto& v : values) {
        program->units.emplace_back(std::move(v.m_impl->unit));
      }
    }
    HPHP_CORO_RETURN_VOID;
  };

  std::vector<coro::TaskWithExecutor<void>> tasks;
  tasks.reserve(chunked.size());
  for (auto& c : chunked) {
    tasks.emplace_back(
      loadAndParse(std::move(c)).scheduleOn(executor->sticky())
    );
  }
  coro::wait(coro::collectRange(std::move(tasks)));

  // Done with any extern-worker stuff at this point (for now).
  dispose(std::move(executor), std::move(client));
  return program;
}

}

//////////////////////////////////////////////////////////////////////

void whole_program(WholeProgramInput inputs,
                   std::unique_ptr<coro::TicketExecutor> executor,
                   std::unique_ptr<Client> client,
                   const EmitCallback& callback,
                   const DisposeCallback& dispose,
                   Optional<StructuredLogEntry> sample,
                   int num_threads) {
  trace_time tracer("whole program");

  if (num_threads > 0) {
    parallel::num_threads = num_threads;
    // Leave a thread free for cleanup
    parallel::final_threads = (num_threads > 1) ? (num_threads - 1) : 1;
  }

  auto program = make_program(
    std::move(inputs),
    std::move(executor),
    std::move(client),
    dispose
  );

  if (options.TestCompression || RO::EvalHHBBCTestCompression) {
    php::testCompression(*program);
  }

  state_after("parse", *program);

  Index index(program.get());
  auto stats = allocate_stats();
  auto const emitUnit = [&] (php::Unit& unit) {
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
    callback(std::move(ue));
  };

  auto cleanup_pre_analysis = std::thread([&] { index.cleanup_pre_analysis(); });
  assertx(check(*program));
  prop_type_hint_pass(index, *program);
  index.rewrite_default_initial_values(*program);
  index.use_class_dependencies(false);
  analyze_iteratively(index, *program, AnalyzeMode::ConstPass);
  // Defer preresolve type-structures and initializing public static
  // property types until after the constant pass, to try to get
  // better initial values.
  index.preresolve_type_structures(*program);
  index.init_public_static_prop_types();
  index.preinit_bad_initial_prop_values();
  index.use_class_dependencies(true);
  analyze_iteratively(index, *program, AnalyzeMode::NormalPass);
  auto cleanup_for_final = std::thread([&] { index.cleanup_for_final(); });
  index.join_iface_vtable_thread();
  parallel::num_threads = parallel::final_threads;
  final_pass(index, *program, stats, emitUnit);
  cleanup_pre_analysis.join();
  cleanup_for_final.join();

  print_stats(stats);

  auto const numUnits = program->units.size();
  {
    auto const logging = Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
    auto const enable =
      logging && !Trace::moduleEnabledRelease(Trace::hhbbc_time, 1);
    Trace::BumpRelease bumper(Trace::hhbbc_time, -1, enable);
    index.cleanup_post_emit(std::move(program));
  }

  summarize_memory(sample.get_pointer());
  if (sample && numUnits >= RO::EvalHHBBCMinUnitsToLog) {
    // num_units includes systemlib, around 200 units. Only log big builds.
    sample->setInt("num_units", numUnits);
    sample->setInt(tracer.label(), tracer.elapsed_ms());
    sample->force_init = true;
    StructuredLog::log("hhvm_whole_program", *sample);
  }
}

//////////////////////////////////////////////////////////////////////

}}
