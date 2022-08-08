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

std::vector<Context> all_unit_contexts(const Index& index,
                                       const php::Unit& u) {
  std::vector<Context> ret;
  index.for_each_unit_class(
    u,
    [&] (const php::Class& c) {
      for (auto const& m : c.methods) {
        ret.emplace_back(Context { &u, m.get(), &c });
      }
    }
  );
  index.for_each_unit_func(
    u,
    [&] (const php::Func& f) { ret.emplace_back(Context { &u, &f }); }
  );
  return ret;
}

std::vector<Context> const_pass_contexts(const Index& index) {
  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * Must include every function with a DefCns for correctness; cinit,
   * pinit and sinit functions are added to improve overall
   * performance.
   */
  std::vector<Context> ret;
  for (auto const& c : index.program().classes) {
    for (auto const& m : c->methods) {
      if (m->name == s_86cinit.get() ||
          m->name == s_86pinit.get() ||
          m->name == s_86sinit.get() ||
          m->name == s_86linit.get()) {
        ret.emplace_back(
          Context {
            index.lookup_class_unit(*c),
            m.get(),
            c.get()
          }
        );
      }
    }
  }
  return ret;
}

// Return all the WorkItems we'll need to start analyzing this
// program.
std::vector<WorkItem> initial_work(const Index& index,
                                   AnalyzeMode mode) {
  std::vector<WorkItem> ret;

  if (mode == AnalyzeMode::ConstPass) {
    auto const ctxs = const_pass_contexts(index);
    std::transform(begin(ctxs), end(ctxs), std::back_inserter(ret),
      [&] (Context ctx) { return WorkItem { WorkType::Func, ctx }; }
    );
    return ret;
  }

  auto const& program = index.program();
  for (auto const& c : program.classes) {
    if (c->closureContextCls) {
      // For class-at-a-time analysis, closures that are associated
      // with a class context are analyzed as part of that context.
      continue;
    }
    auto const unit = index.lookup_class_unit(*c);
    if (is_used_trait(*c)) {
      for (auto const& f : c->methods) {
        ret.emplace_back(
          WorkType::Func,
          Context { unit, f.get(), f->cls }
        );
      }
    } else {
      ret.emplace_back(
        WorkType::Class,
        Context { unit, nullptr, c.get() }
      );
    }
  }
  for (auto const& f : program.funcs) {
    ret.emplace_back(
      WorkType::Func,
      Context { index.lookup_func_unit(*f), f.get() }
    );
  }
  return ret;
}

std::vector<Context> opt_prop_type_hints_contexts(const Index& index) {
  std::vector<Context> ret;
  for (auto const& c : index.program().classes) {
    ret.emplace_back(
      Context { index.lookup_class_unit(*c), nullptr, c.get() }
    );
  }
  return ret;
}

WorkItem work_item_for(const DependencyContext& d,
                       AnalyzeMode mode,
                       const Index& index) {
  switch (d.tag()) {
    case DependencyContextType::Class: {
      auto const cls = (const php::Class*)d.ptr();
      assertx(mode != AnalyzeMode::ConstPass &&
              !is_used_trait(*cls));
      return WorkItem {
        WorkType::Class,
        Context { index.lookup_class_unit(*cls), nullptr, cls }
      };
    }
    case DependencyContextType::Func: {
      auto const func = (const php::Func*)d.ptr();
      auto const cls = func->cls
        ? index.lookup_closure_context(*func->cls)
        : nullptr;
      assertx(!cls ||
              mode == AnalyzeMode::ConstPass ||
              is_used_trait(*cls));
      return WorkItem {
        WorkType::Func,
        Context { index.lookup_func_unit(*func), func, cls }
      };
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
void analyze_iteratively(Index& index, AnalyzeMode mode) {
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

  auto work = initial_work(index, mode);
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
      for (auto const& [cls, vars] : fa.closureUseTypes) {
        assertx(is_closure(*cls));
        if (index.refine_closure_use_vars(cls, vars)) {
          auto const func = find_method(cls, s_invoke.get());
          always_assert_flog(
            func != nullptr,
            "Failed to find __invoke on {} during index update\n",
            cls->name
          );
          auto const ctx =
            Context { index.lookup_func_unit(*func), func, cls };
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
    for (auto& d : deps) work.push_back(work_item_for(d, mode, index));
    deps.clear();
  }
}

void prop_type_hint_pass(Index& index) {
  trace_time tracer("optimize prop type-hints");

  auto const contexts = opt_prop_type_hints_contexts(index);
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
                const StatsHolder& stats,
                F emitUnit) {
  trace_time final_pass("final pass");
  index.freeze();
  auto const dump_dir = debug_dump_to();
  parallel::for_each(
    index.program().units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      // optimize_func can remove 86*init methods from classes, so we
      // have to save the contexts for now.
      for (auto const& context : all_unit_contexts(index, *unit)) {
        // This const_cast is safe since no two threads update the same Func.
        auto func = php::WideFunc::mut(const_cast<php::Func*>(context.func));
        auto const ctx = AnalysisContext { context.unit, func, context.cls };
        optimize_func(index, analyze_func(index, ctx, CollectionOpts{}), func);
      }
      state_after("optimize", *unit, index);
      if (!dump_dir.empty()) {
        if (Trace::moduleEnabledRelease(Trace::hhbbc_dump, 2)) {
          dump_representation(dump_dir, index, *unit);
        }
        dump_index(dump_dir, index, *unit);
      }
      collect_stats(stats, index, *unit);
      emitUnit(*unit);
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

// Right now Key is nothing, and Value is the UnitEmitter.

struct WholeProgramInput::Key::Impl {
  enum class Type {
    Fail,
    Unit,
    Func,
    Class
  };
  Type type;
  LSString name;
  std::vector<SString> dependencies;

  Impl(Type type, SString name, std::vector<SString> dependencies)
    : type{type}, name{name}, dependencies{std::move(dependencies)} {}
};
struct WholeProgramInput::Value::Impl {
  std::unique_ptr<php::Func> func;
  std::unique_ptr<php::Class> cls;
  std::unique_ptr<php::Unit> unit;

  explicit Impl(std::nullptr_t) {}
  explicit Impl(std::unique_ptr<php::Func> func) : func{std::move(func)} {}
  explicit Impl(std::unique_ptr<php::Class> cls) : cls{std::move(cls)} {}
  explicit Impl(std::unique_ptr<php::Unit> unit) : unit{std::move(unit)} {}
};
struct WholeProgramInput::Impl {
  folly::AtomicLinkedList<std::pair<Key, Ref<Value>>> values;
};

WholeProgramInput::WholeProgramInput() : m_impl{new Impl} {}

void WholeProgramInput::add(Key k, extern_worker::Ref<Value> v) {
  assertx(m_impl);
  m_impl->values.insertHead(std::make_pair(std::move(k), std::move(v)));
}

std::vector<std::pair<WholeProgramInput::Key, WholeProgramInput::Value>>
WholeProgramInput::make(std::unique_ptr<UnitEmitter> ue) {
  assertx(ue);

  auto parsed = parse_unit(*ue);

  std::vector<std::pair<Key, Value>> out;

  auto const add = [&] (Key::Impl::Type type,
                        SString name,
                        auto v,
                        std::vector<SString> deps = {}) {
    Key key;
    Value value;
    key.m_impl.reset(new Key::Impl{type, name, std::move(deps)});
    value.m_impl.reset(new Value::Impl{std::move(v)});
    out.emplace_back(std::move(key), std::move(value));
  };

  if (parsed.unit) {
    if (auto const& fi = parsed.unit->fatalInfo) {
      if (!fi->fatalLoc) {
        add(Key::Impl::Type::Fail, makeStaticString(fi->fatalMsg), nullptr);
        return out;
      }
    }
    auto const name = parsed.unit->filename;
    add(Key::Impl::Type::Unit, name, std::move(parsed.unit));
  }
  for (auto& c : parsed.classes) {
    auto const name = c->name;
    auto deps = Index::Input::makeDeps(*c);
    add(
      Key::Impl::Type::Class,
      name,
      std::move(c),
      std::move(deps)
    );
  }
  for (auto& f : parsed.funcs) {
    auto const name = f->name;
    add(Key::Impl::Type::Func, name, std::move(f));
  }
  return out;
}

void WholeProgramInput::Key::serde(BlobEncoder& sd) const {
  assertx(m_impl);
  sd(m_impl->type)(m_impl->name);
  if (m_impl->type == Impl::Type::Class) sd(m_impl->dependencies);
}
void WholeProgramInput::Key::serde(BlobDecoder& sd) {
  Key::Impl::Type type;
  SString name;
  std::vector<SString> dependencies;
  sd(type)(name);
  if (type == Impl::Type::Class) sd(dependencies);
  m_impl.reset(new Impl{type, name, std::move(dependencies)});
}

void WholeProgramInput::Value::serde(BlobEncoder& sd) const {
  assertx(m_impl);
  assertx(
    (bool)m_impl->func + (bool)m_impl->cls + (bool)m_impl->unit <= 1
  );
  if (m_impl->func) {
    sd(m_impl->func, nullptr);
  } else if (m_impl->cls) {
    sd(m_impl->cls);
  } else if (m_impl->unit) {
    sd(m_impl->unit);
  }
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

Index::Input make_index_input(WholeProgramInput input) {
  Index::Input out;

  using WPI = WholeProgramInput;
  using Key = WPI::Key::Impl;

  input.m_impl->values.sweep(
    [&] (std::pair<WPI::Key, Ref<WPI::Value>>&& p) {
      switch (p.first.m_impl->type) {
        case Key::Type::Fail:
          // An unit which failed the verifier. This causes us
          // to exit immediately with an error.
           fprintf(stderr, "%s", p.first.m_impl->name->data());
           _Exit(1);
           break;
         case Key::Type::Class:
           out.classes.emplace_back(
             Index::Input::ClassMeta{
               p.second.cast<std::unique_ptr<php::Class>>(),
               p.first.m_impl->name,
               std::move(p.first.m_impl->dependencies)
             }
           );
           break;
         case Key::Type::Func:
           out.funcs.emplace_back(
             p.first.m_impl->name,
             p.second.cast<std::unique_ptr<php::Func>>()
           );
           break;
         case Key::Type::Unit:
           out.units.emplace_back(
             p.first.m_impl->name,
             p.second.cast<std::unique_ptr<php::Unit>>()
           );
           break;
      }
    }
  );

  return out;
}

}

//////////////////////////////////////////////////////////////////////

void whole_program(WholeProgramInput inputs,
                   std::unique_ptr<coro::TicketExecutor> executor,
                   std::unique_ptr<Client> client,
                   const EmitCallback& callback,
                   DisposeCallback dispose,
                   Optional<StructuredLogEntry> sample,
                   int num_threads) {
  trace_time tracer("whole program");

  if (num_threads > 0) {
    parallel::num_threads = num_threads;
    // Leave a thread free for cleanup
    parallel::final_threads = (num_threads > 1) ? (num_threads - 1) : 1;
  }

  Index index{
    make_index_input(std::move(inputs)),
    std::move(executor),
    std::move(client),
    std::move(dispose)
  };

  auto stats = allocate_stats();
  auto const emitUnit = [&] (php::Unit& unit) {
    auto ue = emit_unit(index, unit);
    if (RO::EvalAbortBuildOnVerifyError && !ue->check(false)) {
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

  auto cleanup_pre_analysis =
    std::thread([&] { index.cleanup_pre_analysis(); });
  assertx(check(index.program()));
  prop_type_hint_pass(index);
  index.rewrite_default_initial_values();
  index.use_class_dependencies(false);
  analyze_iteratively(index, AnalyzeMode::ConstPass);
  // Defer preresolve type-structures and initializing public static
  // property types until after the constant pass, to try to get
  // better initial values.
  index.preresolve_type_structures();
  index.init_public_static_prop_types();
  index.preinit_bad_initial_prop_values();
  index.use_class_dependencies(true);
  analyze_iteratively(index, AnalyzeMode::NormalPass);
  auto cleanup_for_final = std::thread([&] { index.cleanup_for_final(); });
  index.join_iface_vtable_thread();
  parallel::num_threads = parallel::final_threads;
  final_pass(index, stats, emitUnit);
  cleanup_pre_analysis.join();
  cleanup_for_final.join();

  print_stats(stats);

  auto const numUnits = index.program().units.size();
  index.cleanup_post_emit();

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
