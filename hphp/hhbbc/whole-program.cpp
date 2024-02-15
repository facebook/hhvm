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

#ifdef HHVM_FACEBOOK
#include <strobelight/strobemeta/strobemeta_frames.h>
#else
#define SET_FRAME_METADATA(...)
#endif

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
#include "hphp/hhbbc/options-util.h"
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
namespace coro = folly::coro;

namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_invoke("__invoke");

//////////////////////////////////////////////////////////////////////

enum class WorkType { Class, Func };

struct WorkItem {
  explicit WorkItem(WorkType type, Context ctx)
    : type(type)
    , ctx(ctx)
  {}

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
        ret.emplace_back(Context { u.filename, m.get(), &c });
      }
    }
  );
  index.for_each_unit_func(
    u,
    [&] (const php::Func& f) { ret.emplace_back(Context { u.filename, &f }); }
  );
  return ret;
}

std::vector<Context> const_pass_contexts(const Index& index) {
  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * None are needed for correctness. cinit, pinit, sinit, and linit
   * functions are processed to improve overall performance.
   */
  std::vector<Context> ret;
  for (auto const& c : index.program().classes) {
    for (auto const& m : c->methods) {
      if (!is_86init_func(*m)) continue;
      ret.emplace_back(
        Context {
          c->unit,
          m.get(),
          c.get()
        }
      );
    }
  }
  for (auto const& f : index.program().funcs) {
    if (!Constant::nameFromFuncName(f->name)) continue;
    ret.emplace_back(
      Context {
        f->unit,
        f.get(),
        nullptr
      }
    );
  }
  return ret;
}

// Return all the WorkItems we'll need to start analyzing this
// program.
std::vector<WorkItem> initial_work(const Index& index) {
  std::vector<WorkItem> ret;

  auto const& program = index.program();
  for (auto const& c : program.classes) {
    if (c->closureContextCls) {
      // For class-at-a-time analysis, closures that are associated
      // with a class context are analyzed as part of that context.
      continue;
    }
    if (is_used_trait(*c)) {
      for (auto const& f : c->methods) {
        ret.emplace_back(
          WorkType::Func,
          Context { c->unit, f.get(), f->cls }
        );
      }
    } else {
      ret.emplace_back(
        WorkType::Class,
        Context { c->unit, nullptr, c.get() }
      );
    }
  }
  for (auto const& f : program.funcs) {
    ret.emplace_back(
      WorkType::Func,
      Context { f->unit, f.get() }
    );
  }
  return ret;
}

WorkItem work_item_for(const DependencyContext& d,
                       const Index& index) {
  switch (d.tag()) {
    case DependencyContextType::Class: {
      auto const cls = (const php::Class*)d.ptr();
      assertx(!is_used_trait(*cls));
      return WorkItem {
        WorkType::Class,
        Context { cls->unit, nullptr, cls }
      };
    }
    case DependencyContextType::Func: {
      auto const func = (const php::Func*)d.ptr();
      auto const cls = func->cls
        ? index.lookup_closure_context(*func->cls)
        : nullptr;
      assertx(!cls || is_used_trait(*cls));
      return WorkItem {
        WorkType::Func,
        Context { func->unit, func, cls }
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
void analyze_iteratively(Index& index) {
  trace_time tracer("analyze iteratively", index.sample());

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

  auto work = initial_work(index);
  while (!work.empty()) {
    auto results = [&] {
      trace_time trace(
        "analyzing",
        folly::sformat("round {} -- {} work items", round, work.size())
      );
      return parallel::map(
        work,
        // We have a Optional just to keep the result type
        // DefaultConstructible.
        [&] (const WorkItem& wi) -> Optional<WorkResult> {
          SET_FRAME_METADATA(wi.ctx.unit->toCppString());
          switch (wi.type) {
          case WorkType::Func: {
            ++total_funcs;
            auto const wf = php::WideFunc::cns(wi.ctx.func);
            auto const ctx = AnalysisContext { wi.ctx.unit, wf, wi.ctx.cls };
            IndexAdaptor adaptor{ index };
            return WorkResult { analyze_func(adaptor, ctx, CollectionOpts{}) };
          }
          case WorkType::Class: {
            ++total_classes;
            IndexAdaptor adaptor{ index };
            return WorkResult { analyze_class(adaptor, wi.ctx) };
          }
          }
          not_reached();
        }
      );
    }();

    ++round;
    trace_time update_time("updating");

    auto const update_func = [&] (FuncAnalysisResult& fa,
                                  DependencyContextSet& deps) {
      SCOPE_ASSERT_DETAIL("update_func") {
        return "Updating Func: " + show(fa.ctx);
      };
      // This const_cast is safe since no two threads update the same Func.
      auto func = php::WideFunc::mut(const_cast<php::Func*>(fa.ctx.func));
      index.refine_return_info(fa, deps);
      index.refine_constants(fa, deps);
      update_bytecode(func, std::move(fa.blockUpdates));

      index.record_public_static_mutations(
        *func,
        std::move(fa.publicSPropMutations)
      );

      if (auto const l = fa.resolvedInitializers.left()) {
        index.refine_class_constants(fa.ctx, *l, deps);
      } else if (auto const r = fa.resolvedInitializers.right()) {
        index.update_prop_initial_values(fa.ctx, *r, deps);
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
            Context { func->unit, func, cls };
          deps.insert(index.dependency_context(ctx));
        }
      }
    };

    auto const update_class = [&] (ClassAnalysis& ca,
                                   DependencyContextSet& deps) {
      {
        SCOPE_ASSERT_DETAIL("update_class") {
          return "Updating Class: " + show(ca.ctx);
        };
        index.refine_private_props(ca.ctx.cls,
                                   ca.privateProperties);
        index.refine_private_statics(ca.ctx.cls,
                                     ca.privateStatics);
        index.update_prop_initial_values(ca.ctx, ca.resolvedProps, deps);
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

    index.refine_public_statics(deps);

    work.clear();
    work.reserve(deps.size());
    for (auto& d : deps) work.emplace_back(work_item_for(d, index));
    deps.clear();
  }
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
  trace_time final_pass("final pass", index.sample());
  index.freeze();
  auto const dump_dir = debug_dump_to();
  parallel::for_each(
    index.program().units,
    [&] (const std::unique_ptr<php::Unit>& unit) {
      SET_FRAME_METADATA(unit->filename->toCppString());
      // optimize_func can remove 86*init methods from classes, so we
      // have to save the contexts for now.
      for (auto const& context : all_unit_contexts(index, *unit)) {
        // This const_cast is safe since no two threads update the same Func.
        auto func = php::WideFunc::mut(const_cast<php::Func*>(context.func));
        auto const ctx = AnalysisContext { context.unit, func, context.cls };
        IndexAdaptor adaptor{ index };
        optimize_func(index, analyze_func(adaptor, ctx, CollectionOpts{}), func);
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

// Extern-worker job to analyze constants

struct AnalyzeConstantsJob {
  static std::string name() { return "hhbbc-analyze-constants"; }
  static void init(const Config& config) {
    process_init(config.o, config.gd, false);
    AnalysisIndex::start();
  }
  static void fini() { AnalysisIndex::stop(); }

  template<typename T> using V = Variadic<T>;
  template<typename T> using VU = V<std::unique_ptr<T>>;

  using Output = AnalysisIndex::Output;

  static Output run(VU<php::Class> classes,
                    VU<php::Func> funcs,
                    VU<php::Unit> units,
                    VU<php::ClassBytecode> clsBC,
                    VU<php::FuncBytecode> funcBC,
                    V<AnalysisIndexCInfo> cinfos,
                    V<AnalysisIndexFInfo> finfos,
                    V<AnalysisIndexMInfo> minfos,
                    VU<php::Class> depClasses,
                    VU<php::Func> depFuncs,
                    VU<php::Unit> depUnits,
                    AnalysisInput::Meta meta) {
    // Pre-populate the worklist with everything to begin with.
    AnalysisWorklist start;
    for (auto const& c : classes.vals) start.schedule(c.get());
    for (auto const& f : funcs.vals)   start.schedule(f.get());
    // Make a copy of it. We'll need the original list after the index
    // is frozen.
    auto worklist = start;

    AnalysisIndex index{
      worklist,
      std::move(classes.vals),
      std::move(funcs.vals),
      std::move(units.vals),
      std::move(clsBC.vals),
      std::move(funcBC.vals),
      std::move(cinfos.vals),
      std::move(finfos.vals),
      std::move(minfos.vals),
      std::move(depClasses.vals),
      std::move(depFuncs.vals),
      std::move(depUnits.vals),
      std::move(meta)
    };

    // Keep processing work until we reach a fixed-point (nothing new
    // gets put on the worklist).
    while (process(index, worklist)) {}
    // Freeze the index. Nothing is allowed to update the index after
    // this.
    index.freeze();
    // Now do a pass through the original work items again. Now that
    // the index is frozen, we'll gather up any dependencies from the
    // analysis. Since we already reached a fixed point, this should
    // not cause any updates (and if it does, we'll assert).
    while (process(index, start)) {}
    // Everything is analyzed and dependencies are recorded. Turn the
    // index data into AnalysisIndex::Output and return it from this
    // job.
    return index.finish();
  }

private:
  // Analyze the work item at the front of the worklist (returning
  // false if the list is empty).
  static bool process(AnalysisIndex& index,
                      AnalysisWorklist& worklist) {
    auto const w = worklist.next();
    if (auto const c = w.right()) {
      auto results = analyze(*c, index);
      for (auto& r : results) update(std::move(r), index);
    } else if (auto const f = w.left()) {
      update(analyze(*f, index), index);
    } else {
      return false;
    }
    return true;
  }

  static FuncAnalysisResult analyze(const php::Func& f, AnalysisIndex& index) {
    auto const wf = php::WideFunc::cns(&f);
    AnalysisContext ctx{ f.unit, wf, f.cls };
    return analyze_func(AnalysisIndexAdaptor{ index }, ctx, CollectionOpts{});
  }

  static std::vector<FuncAnalysisResult> analyze(const php::Class& c,
                                                 AnalysisIndex& index) {
    std::vector<FuncAnalysisResult> results;
    results.reserve(c.methods.size());
    for (auto const& m : c.methods) {
      if (!is_86init_func(*m)) continue;
      results.emplace_back(analyze(*m, index));
    }
    return results;
  }

  static void update(FuncAnalysisResult fa, AnalysisIndex& index) {
    SCOPE_ASSERT_DETAIL("update func") {
      return "Updating Func: " + show(fa.ctx);
    };
    AnalysisIndexAdaptor adaptor{index};
    ContextPusher _{adaptor, fa.ctx};
    index.refine_return_info(fa);
    index.refine_constants(fa);
    index.refine_class_constants(fa);
    index.update_prop_initial_values(fa);
    index.update_bytecode(fa);
  }
};

Job<AnalyzeConstantsJob> s_analyzeConstantsJob;

void analyze_constants(Index& index) {
  trace_time tracer{"analyze constants", index.sample()};

  constexpr size_t kBucketSize = 2000;

  using namespace folly::gen;

  // We'll only process classes with 86*init functions or top-level
  // 86cinits.
  AnalysisScheduler scheduler{index};
  for (auto const cls : index.classes_with_86inits()) {
    scheduler.registerClass(cls);
  }
  for (auto const func : index.constant_init_funcs()) {
    scheduler.registerFunc(func);
  }

  auto const run = [&] (AnalysisInput input) -> coro::Task<void> {
    co_await coro::co_reschedule_on_current_executor;

    if (input.empty()) co_return;

    Client::ExecMetadata metadata{
      .job_key = folly::sformat("analyze constants {}", input.key())
    };

    auto [inputMeta, config] = co_await coro::collectAll(
      index.client().store(input.takeMeta()),
      index.configRef().getCopy()
    );

    // Run the job
    auto outputs = co_await index.client().exec(
      s_analyzeConstantsJob,
      std::move(config),
      singleton_vec(input.toTuple(std::move(inputMeta))),
      std::move(metadata)
    );
    always_assert(outputs.size() == 1);
    auto& [clsRefs, funcRefs, unitRefs,
           clsBCRefs, funcBCRefs,
           cinfoRefs, finfoRefs,
           minfoRefs, metaRef] = outputs[0];

    auto meta = co_await index.client().load(std::move(metaRef));

    auto classNames = input.classNames();
    auto cinfoNames = input.cinfoNames();
    auto minfoNames = input.minfoNames();
    always_assert(clsRefs.size() == classNames.size());
    always_assert(clsBCRefs.size() == classNames.size());
    always_assert(cinfoRefs.size() == cinfoNames.size());
    always_assert(minfoRefs.size() == minfoNames.size());
    always_assert(meta.classDeps.size() == classNames.size());

    auto funcNames = from(input.funcNames())
      | filter([&] (SString n) { return !meta.removedFuncs.count(n); })
      | as<std::vector>();
    always_assert(funcRefs.size() == funcNames.size());
    always_assert(funcBCRefs.size() == funcNames.size());
    always_assert(finfoRefs.size() == funcNames.size());
    always_assert(meta.funcDeps.size() == funcNames.size());

    auto unitNames = input.unitNames();
    always_assert(unitRefs.size() == unitNames.size());

    // Inform the scheduler
    scheduler.record(
      AnalysisOutput{
        std::move(classNames),
        std::move(cinfoNames),
        std::move(minfoNames),
        std::move(clsRefs),
        std::move(clsBCRefs),
        std::move(cinfoRefs),
        std::move(funcNames),
        std::move(funcRefs),
        std::move(funcBCRefs),
        std::move(finfoRefs),
        std::move(minfoRefs),
        std::move(unitNames),
        std::move(unitRefs),
        std::move(meta)
      }
    );
    co_return;
  };

  size_t round{0};
  while (auto const workItems = scheduler.workItems()) {
    trace_time trace{
      "analyze constants round",
      folly::sformat("round {} -- {} work items", round, workItems)
    };
    // Get the work buckets from the scheduler.
    auto const work = [&] {
      trace_time trace2{
        "analyze constants schedule",
        folly::sformat("round {}", round)
      };
      trace2.ignore_client_stats();
      return scheduler.schedule(kBucketSize);
    }();
    // Work shouldn't be empty because we add non-zero work items this
    // round.
    assertx(!work.empty());

    {
      // Process the work buckets in individual analyze constants
      // jobs. These will record their results as each one finishes.
      trace_time trace2{
        "analyze constants run",
        folly::sformat("round {}", round)
      };
      trace2.ignore_client_stats();
      coro::blockingWait(coro::collectAllRange(
        from(work)
          | move
          | map([&] (AnalysisInput input) {
              return run(std::move(input)).scheduleOn(index.executor().sticky());
            })
          | as<std::vector>()
      ));
    }

    {
      // All the jobs recorded their results in the scheduler. Now let
      // the scheduler know that all jobs are done, so it can
      // determine what needs to be run in the next round.
      trace_time trace2{
        "analyze constants deps",
        folly::sformat("round {}", round)
      };
      trace2.ignore_client_stats();
      scheduler.recordingDone();
      ++round;
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

struct WholeProgramInput::Key::Impl {
  enum class Type {
    None,
    Fail,
    Unit,
    Func,
    FuncBytecode,
    Class,
    ClassBytecode
  };

  using UnresolvedTypes =
    hphp_fast_set<SString, string_data_hash, string_data_tsame>;

  struct FailInfo {
    LSString message;
    template <typename SerDe> void serde(SerDe& sd) { sd(message); }
  };
  struct UnitInfo {
    LSString name;
    std::vector<TypeMapping> typeMappings;
    std::vector<std::pair<SString, bool>> constants;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(name)
        (typeMappings)
        (constants)
        ;
    }
  };
  struct FuncInfo {
    LSString name;
    LSString unit;
    bool methCaller;
    UnresolvedTypes unresolvedTypes;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(name)
        (unit)
        (methCaller)
        (unresolvedTypes, string_data_lt_type{})
        ;
    }
  };
  struct FuncBytecodeInfo {
    LSString name;
    LSString unit;
    bool methCaller;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(name)(unit)(methCaller);
    }
  };
  struct ClassInfo {
    LSString name;
    LSString context;
    LSString unit;
    std::vector<SString> dependencies;
    bool isClosure;
    bool closureDeclInFunc;
    bool has86init;
    Optional<TypeMapping> typeMapping;
    UnresolvedTypes unresolvedTypes;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(name)
        (context)
        (unit)
        (dependencies)
        (isClosure)
        (closureDeclInFunc)
        (has86init)
        (typeMapping)
        (unresolvedTypes, string_data_lt_type{})
        ;
    }
  };
  struct ClassBytecodeInfo {
    LSString name;
    template <typename SerDe> void serde(SerDe& sd) {
      sd(name);
    }
  };

  Type type;
  union {
    FailInfo fail;
    UnitInfo unit;
    FuncInfo func;
    FuncBytecodeInfo funcBC;
    ClassInfo cls;
    ClassBytecodeInfo clsBC;
  };

  Impl() : type{Type::None} {}

  explicit Impl(FailInfo i)  : type{Type::Fail},  fail{std::move(i)} {}
  explicit Impl(UnitInfo i)  : type{Type::Unit},  unit{std::move(i)} {}
  explicit Impl(FuncInfo i)  : type{Type::Func},  func{std::move(i)} {}
  explicit Impl(ClassInfo i) : type{Type::Class}, cls{std::move(i)}  {}
  explicit Impl(FuncBytecodeInfo i)
    : type{Type::FuncBytecode}, funcBC{std::move(i)} {}
  explicit Impl(ClassBytecodeInfo i)
    : type{Type::ClassBytecode}, clsBC{std::move(i)} {}

  Impl(const Impl&) = delete;
  Impl(Impl&&) = delete;
  Impl& operator=(const Impl&) = delete;
  Impl& operator=(Impl&&) = delete;

  void destroyInfo() {
    switch (type) {
      case Type::None:  break;
      case Type::Fail:  fail.~FailInfo();  break;
      case Type::Unit:  unit.~UnitInfo();  break;
      case Type::Func:  func.~FuncInfo();  break;
      case Type::Class: cls.~ClassInfo();  break;
      case Type::FuncBytecode:
        funcBC.~FuncBytecodeInfo();
        break;
      case Type::ClassBytecode:
        clsBC.~ClassBytecodeInfo();
        break;
    }
  }

  ~Impl() { destroyInfo(); }

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      destroyInfo();
      sd(type);
      switch (type) {
        case Type::None:  break;
        case Type::Fail:  new (&fail) FailInfo();  break;
        case Type::Unit:  new (&unit) UnitInfo();  break;
        case Type::Func:  new (&func) FuncInfo();  break;
        case Type::Class: new (&cls)  ClassInfo(); break;
        case Type::FuncBytecode:
          new (&funcBC) FuncBytecodeInfo();
          break;
        case Type::ClassBytecode:
          new (&clsBC) ClassBytecodeInfo();
          break;
      }
    } else {
      sd(type);
    }

    switch (type) {
      case Type::None:  break;
      case Type::Fail:  sd(fail); break;
      case Type::Unit:  sd(unit); break;
      case Type::Func:  sd(func); break;
      case Type::Class: sd(cls);  break;
      case Type::FuncBytecode:
        sd(funcBC);
        break;
      case Type::ClassBytecode:
        sd(clsBC);
        break;
    }
  }
};

struct WholeProgramInput::Value::Impl {
  std::unique_ptr<php::Func> func;
  std::unique_ptr<php::Class> cls;
  std::unique_ptr<php::Unit> unit;
  std::unique_ptr<php::FuncBytecode> funcBC;
  std::unique_ptr<php::ClassBytecode> clsBC;

  explicit Impl(std::nullptr_t) {}
  explicit Impl(std::unique_ptr<php::Func> func) : func{std::move(func)} {}
  explicit Impl(std::unique_ptr<php::Class> cls) : cls{std::move(cls)} {}
  explicit Impl(std::unique_ptr<php::Unit> unit) : unit{std::move(unit)} {}
  explicit Impl(std::unique_ptr<php::FuncBytecode> b)  : funcBC{std::move(b)} {}
  explicit Impl(std::unique_ptr<php::ClassBytecode> b) : clsBC{std::move(b)} {}
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

  using KeyI = Key::Impl;
  using ValueI = Value::Impl;

  auto const add = [&] (auto k, auto v) {
    Key key;
    Value value;
    key.m_impl.reset(new KeyI{std::move(k)});
    value.m_impl.reset(new ValueI{std::move(v)});
    out.emplace_back(std::move(key), std::move(value));
  };

  auto const addType =
    [&] (KeyI::UnresolvedTypes& u,
         const TypeConstraint& tc,
         const php::Class* cls = nullptr,
         const TypeIntersectionConstraint* ubs = nullptr) {
    // Skip names which match the current class name. We don't need to
    // report these as it's implicit.
    if (tc.isUnresolved() && (!cls || !cls->name->tsame(tc.typeName()))) {
      u.emplace(tc.typeName());
    }
    if (!ubs) return;
    for (auto const& ub : ubs->m_constraints) {
      if (ub.isUnresolved() && (!cls || !cls->name->tsame(ub.typeName()))) {
        u.emplace(ub.typeName());
      }
    }
  };

  auto const addFuncTypes = [&] (KeyI::UnresolvedTypes& u,
                                 const php::Func& f,
                                 const php::Class* cls = nullptr) {
    for (auto const& p : f.params) {
      addType(u, p.typeConstraint, cls, &p.upperBounds);
    }
    addType(u, f.retTypeConstraint, cls, &f.returnUBs);
  };

  if (parsed.unit) {
    if (auto const& fi = parsed.unit->fatalInfo) {
      auto const msg = makeStaticString(fi->fatalMsg);
      if (!fi->fatalLoc) {
        add(KeyI::FailInfo{msg}, nullptr);
        return out;
      }
    }

    KeyI::UnitInfo info{parsed.unit->filename};
    for (auto const& typeAlias : parsed.unit->typeAliases) {
      info.typeMappings.emplace_back(
        TypeMapping{
          typeAlias->name,
          nullptr,
          typeAlias->value,
          true
        }
      );
    }
    for (auto const& cns : parsed.unit->constants) {
      info.constants.emplace_back(
        cns->name,
        type(cns->val) == KindOfUninit
      );
    }
    add(std::move(info), std::move(parsed.unit));
  }
  for (auto& c : parsed.classes) {
    auto const name = c->name;
    auto const context = c->closureContextCls;
    auto const isClosure = is_closure(*c);
    auto const declFunc = c->closureDeclFunc;
    auto const unit = c->unit;
    auto deps = Index::Input::makeDeps(*c);
    auto has86init = false;

    php::ClassBytecode bc{name};
    KeyI::UnresolvedTypes types;
    for (auto& m : c->methods) {
      addFuncTypes(types, *m, c.get());
      bc.methodBCs.emplace_back(m->name, std::move(m->rawBlocks));
      has86init |= is_86init_func(*m);
    }
    for (auto const& p : c->properties) {
      addType(types, p.typeConstraint, c.get(), &p.ubs);
    }

    Optional<TypeMapping> typeMapping;
    if (c->attrs & AttrEnum) {
      auto tc = c->enumBaseTy;
      assertx(!tc.isNullable());
      addType(types, tc, nullptr);
      if (tc.isMixed()) tc.setType(AnnotType::ArrayKey);
      typeMapping.emplace(TypeMapping{c->name, c->name, tc, false});
    }

    add(
      KeyI::ClassBytecodeInfo{name},
      std::make_unique<php::ClassBytecode>(std::move(bc))
    );
    add(
      KeyI::ClassInfo{
        name,
        declFunc ? declFunc : context,
        unit,
        std::move(deps),
        isClosure,
        (bool)declFunc,
        has86init,
        std::move(typeMapping),
        std::move(types)
      },
      std::move(c)
    );
  }
  for (auto& f : parsed.funcs) {
    auto const name = f->name;
    auto const unit = f->unit;
    auto const methCaller = bool(f->attrs & AttrIsMethCaller);

    KeyI::UnresolvedTypes types;
    addFuncTypes(types, *f);

    add(
      KeyI::FuncBytecodeInfo{name, unit, methCaller},
      std::make_unique<php::FuncBytecode>(name, std::move(f->rawBlocks))
    );
    add(
      KeyI::FuncInfo{name, unit, methCaller, std::move(types)},
      std::move(f)
    );
  }
  return out;
}

void WholeProgramInput::Key::serde(BlobEncoder& sd) const {
  assertx(m_impl);
  sd(*m_impl);
}

void WholeProgramInput::Key::serde(BlobDecoder& sd) {
  m_impl.reset(new Impl());
  sd(*m_impl);
}

void WholeProgramInput::Value::serde(BlobEncoder& sd) const {
  assertx(m_impl);
  assertx(
    (bool)m_impl->func + (bool)m_impl->cls + (bool)m_impl->unit +
    (bool)m_impl->funcBC + (bool)m_impl->clsBC <= 1
  );
  if (m_impl->func) {
    sd(m_impl->func, nullptr);
  } else if (m_impl->cls) {
    sd(m_impl->cls);
  } else if (m_impl->unit) {
    sd(m_impl->unit);
  } else if (m_impl->funcBC) {
    sd(m_impl->funcBC);
  } else if (m_impl->clsBC) {
    sd(m_impl->clsBC);
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
        case Key::Type::None:
          break;
        case Key::Type::Fail:
          // An unit which failed the verifier. This causes us
          // to exit immediately with an error.
          fprintf(stderr, "%s", p.first.m_impl->fail.message->data());
          _Exit(HPHP_EXIT_FAILURE);
          break;
        case Key::Type::Class:
          out.classes.emplace_back(
            Index::Input::ClassMeta{
              p.second.cast<std::unique_ptr<php::Class>>(),
              p.first.m_impl->cls.name,
              std::move(p.first.m_impl->cls.dependencies),
              p.first.m_impl->cls.context,
              p.first.m_impl->cls.unit,
              p.first.m_impl->cls.isClosure,
              p.first.m_impl->cls.closureDeclInFunc,
              p.first.m_impl->cls.has86init,
              std::move(p.first.m_impl->cls.typeMapping),
              std::vector<SString>{
                begin(p.first.m_impl->cls.unresolvedTypes),
                end(p.first.m_impl->cls.unresolvedTypes)
              }
            }
          );
          break;
        case Key::Type::Func:
          out.funcs.emplace_back(
            Index::Input::FuncMeta{
              p.second.cast<std::unique_ptr<php::Func>>(),
              p.first.m_impl->func.name,
              p.first.m_impl->func.unit,
              p.first.m_impl->func.methCaller,
              std::vector<SString>{
                begin(p.first.m_impl->func.unresolvedTypes),
                end(p.first.m_impl->func.unresolvedTypes)
              }
            }
          );
          break;
        case Key::Type::Unit:
          out.units.emplace_back(
            Index::Input::UnitMeta{
              p.second.cast<std::unique_ptr<php::Unit>>(),
              p.first.m_impl->unit.name,
              std::move(p.first.m_impl->unit.typeMappings),
              std::move(p.first.m_impl->unit.constants)
            }
          );
          break;
        case Key::Type::FuncBytecode:
          out.funcBC.emplace_back(
            Index::Input::FuncBytecodeMeta{
              p.second.cast<std::unique_ptr<php::FuncBytecode>>(),
              p.first.m_impl->funcBC.name,
              p.first.m_impl->funcBC.unit,
              p.first.m_impl->funcBC.methCaller
            }
          );
          break;
        case Key::Type::ClassBytecode:
          out.classBC.emplace_back(
            Index::Input::ClassBytecodeMeta{
              p.second.cast<std::unique_ptr<php::ClassBytecode>>(),
              p.first.m_impl->clsBC.name
            }
          );
          break;
      }
    }
  );

  return out;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void whole_program(WholeProgramInput inputs,
                   Config config,
                   std::unique_ptr<TicketExecutor> executor,
                   std::unique_ptr<Client> client,
                   const EmitCallback& callback,
                   DisposeCallback dispose,
                   StructuredLogEntry* sample,
                   int num_threads) {
  trace_time tracer("whole program", sample);

  if (sample) {
    sample->setInt("hhbbc_thread_count", executor->numThreads());
  }

  if (num_threads > 0) {
    parallel::num_threads = num_threads;
    // Leave a thread free for cleanup
    parallel::final_threads = (num_threads > 1) ? (num_threads - 1) : 1;
  }

  Index index{
    make_index_input(std::move(inputs)),
    std::move(config),
    std::move(executor),
    std::move(client),
    std::move(dispose),
    sample
  };

  analyze_constants(index);
  index.make_local();

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
      _Exit(HPHP_EXIT_FAILURE);
    }
    callback(std::move(ue));
  };

  assertx(check(index.program()));

  // Defer preresolve type-structures and initializing public static
  // property types until after the constant pass, to try to get
  // better initial values.
  index.use_class_dependencies(false);
  index.preresolve_type_structures();
  index.use_class_dependencies(true);
  analyze_iteratively(index);
  auto cleanup_for_final = std::thread([&] { index.cleanup_for_final(); });
  parallel::num_threads = parallel::final_threads;
  final_pass(index, stats, emitUnit);
  cleanup_for_final.join();

  print_stats(stats);

  if (sample) {
    sample->setInt("hhbbc_num_units", index.program().units.size());
    sample->setInt("hhbbc_num_classes", index.program().classes.size());
    sample->setInt("hhbbc_num_funcs", index.program().funcs.size());
  }

  index.cleanup_post_emit();
  summarize_memory(sample);
}

//////////////////////////////////////////////////////////////////////

}}
