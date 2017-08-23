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
#include "hphp/hhbbc/analyze.h"

#include <cstdint>
#include <cstdio>
#include <set>
#include <algorithm>
#include <string>
#include <vector>


#include "hphp/util/trace.h"
#include "hphp/util/dataflow-worklist.h"

#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options-util.h"

namespace HPHP { namespace HHBBC {

namespace {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

const StaticString s_86pinit("86pinit");
const StaticString s_86sinit("86sinit");
const StaticString s_AsyncGenerator("HH\\AsyncGenerator");
const StaticString s_Generator("Generator");
const StaticString s_Closure("Closure");

//////////////////////////////////////////////////////////////////////

/*
 * Short-hand to get the rpoId of a block in a given FuncAnalysis.  (The RPO
 * ids are re-assigned per analysis.)
 */
uint32_t rpoId(const FuncAnalysis& ai, BlockId blk) {
  return ai.bdata[blk].rpoId;
}

State pseudomain_entry_state(borrowed_ptr<const php::Func> func) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisAvailable = false;
  ret.locals.resize(func->locals.size());
  ret.iters.resize(func->numIters);
  ret.clsRefSlots.resize(func->numClsRefSlots);
  for (auto& l : ret.locals) l = TGen;
  for (auto& s : ret.clsRefSlots) s = TCls;
  return ret;
}

State entry_state(const Index& index, Context const ctx,
                  ClassAnalysis* /*clsAnalysis*/,
                  const std::vector<Type>* knownArgs) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisAvailable = index.lookup_this_available(ctx.func);
  ret.locals.resize(ctx.func->locals.size());
  ret.iters.resize(ctx.func->numIters);
  ret.clsRefSlots.resize(ctx.func->numClsRefSlots);

  for (auto& s : ret.clsRefSlots) s = TCls;

  // TODO(#3788877): when we're doing a context sensitive analyze_func_inline,
  // thisAvailable and specific type of $this should be able to come from the
  // call context.

  auto locId = uint32_t{0};
  for (; locId < ctx.func->params.size(); ++locId) {
    // Parameters may be Uninit (i.e. no InitCell).  Also note that if
    // a function takes a param by ref, it might come in as a Cell
    // still if FPassC was used.
    if (knownArgs) {
      if (locId < knownArgs->size()) {
        ret.locals[locId] = (*knownArgs)[locId];
      } else {
        ret.locals[locId] = ctx.func->params[locId].isVariadic ? TArr : TUninit;
      }
      continue;
    }
    auto const& param = ctx.func->params[locId];
    if (ctx.func->isMemoizeImpl &&
        !param.byRef &&
        options.HardTypeHints) {
      auto const& constraint = param.typeConstraint;
      if (constraint.hasConstraint() && !constraint.isTypeVar() &&
          !constraint.isTypeConstant()) {
        ret.locals[locId] = index.lookup_constraint(ctx, constraint);
        continue;
      }
    }
    ret.locals[locId] = param.byRef ? TGen : TCell;
  }

  /*
   * Closures have a hidden local that's always the first (non-parameter)
   * local, which stores the closure itself. Due to Class rescoping in the
   * runtime, the strongest type we can assert here is <= Closure. We also need
   * to look up the types of use vars from the index.
   */
  if (ctx.func->isClosureBody) {
    assert(locId < ret.locals.size());
    assert(ctx.func->cls);
    auto const rcls = index.builtin_class(s_Closure.get());
    ret.locals[locId++] = subObj(rcls);
  }
  auto const useVars = ctx.func->isClosureBody
    ? index.lookup_closure_use_vars(ctx.func)
    : std::vector<Type>{};

  auto afterParamsLocId = uint32_t{0};
  for (; locId < ctx.func->locals.size(); ++locId, ++afterParamsLocId) {
    /*
     * Some of the closure locals are mapped to used variables or static
     * locals.  The types of use vars are looked up from the index, but we
     * don't currently do anything to try to track closure static local types.
     */
    if (ctx.func->isClosureBody) {
      if (afterParamsLocId < useVars.size()) {
        ret.locals[locId] = useVars[afterParamsLocId];
        continue;
      }
      if (afterParamsLocId < ctx.func->staticLocals.size()) {
        ret.locals[locId] = TGen;
        continue;
      }
    }

    // Otherwise the local will start uninitialized, like normal.
    ret.locals[locId] = TUninit;
  }

  // Finally, make sure any volatile locals are set to Gen, even if they are
  // parameters.
  for (auto locId = uint32_t{0}; locId < ctx.func->locals.size(); ++locId) {
    if (is_volatile_local(ctx.func, locId)) {
      ret.locals[locId] = TGen;
    }
  }

  return ret;
}

/*
 * Helper for do_analyze to initialize the states for all function entries
 * (i.e. each dv init and the main entry), and all of them count as places the
 * function could be entered, so they all must be visited at least once.
 *
 * If we're entering at a DV-init, all higher parameter locals must be
 * Uninit, with the possible exception of a final variadic param
 * (which will be an array). It is also possible that the DV-init is
 * reachable from within the function with these parameter locals
 * already initialized (although the normal php emitter can't do
 * this), but that case will be discovered when iterating.
 */
dataflow_worklist<uint32_t>
prepare_incompleteQ(const Index& index,
                    FuncAnalysis& ai,
                    ClassAnalysis* clsAnalysis,
                    const std::vector<Type>* knownArgs) {
  auto incompleteQ     = dataflow_worklist<uint32_t>(ai.rpoBlocks.size());
  auto const ctx       = ai.ctx;
  auto const numParams = ctx.func->params.size();

  auto const entryState = [&] {
    if (!is_pseudomain(ctx.func)) {
      return entry_state(index, ctx, clsAnalysis, knownArgs);
    }

    assert(!knownArgs && !clsAnalysis);
    assert(numParams == 0);
    return pseudomain_entry_state(ctx.func);
  }();

  if (knownArgs) {
    // When we have known args, we only need to add one of the entry points to
    // the initial state, since we know how many arguments were passed.
    auto const useDvInit = [&] {
      if (knownArgs->size() >= numParams) return false;
      for (auto i = knownArgs->size(); i < numParams; ++i) {
        auto const dv = ctx.func->params[i].dvEntryPoint;
        if (dv != NoBlockId) {
          ai.bdata[dv].stateIn = entryState;
          incompleteQ.push(rpoId(ai, dv));
          return true;
        }
      }
      return false;
    }();

    if (!useDvInit) {
      ai.bdata[ctx.func->mainEntry].stateIn = entryState;
      incompleteQ.push(rpoId(ai, ctx.func->mainEntry));
    }

    return incompleteQ;
  }

  for (auto paramId = uint32_t{0}; paramId < numParams; ++paramId) {
    auto const dv = ctx.func->params[paramId].dvEntryPoint;
    if (dv != NoBlockId) {
      ai.bdata[dv].stateIn = entryState;
      incompleteQ.push(rpoId(ai, dv));
      for (auto locId = paramId; locId < numParams; ++locId) {
        ai.bdata[dv].stateIn.locals[locId] =
          ctx.func->params[locId].isVariadic ? TArr : TUninit;
      }
    }
  }

  ai.bdata[ctx.func->mainEntry].stateIn = entryState;
  incompleteQ.push(rpoId(ai, ctx.func->mainEntry));

  return incompleteQ;
}

/*
 * Closures inside of classes are analyzed in the context they are
 * created in (this affects accessibility rules, access to privates,
 * etc).
 *
 * Note that in the interpreter code, ctx.func->cls is not
 * necessarily the same as ctx.cls because of closures.
 */
Context adjust_closure_context(Context ctx) {
  if (ctx.cls && ctx.cls->closureContextCls) {
    ctx.cls = ctx.cls->closureContextCls;
  }
  return ctx;
}

FuncAnalysis do_analyze_collect(const Index& index,
                                Context const inputCtx,
                                CollectedInfo& collect,
                                ClassAnalysis* clsAnalysis,
                                const std::vector<Type>* knownArgs) {
  auto const ctx = adjust_closure_context(inputCtx);
  FuncAnalysis ai{ctx};

  auto const bump = trace_bump_for(ctx.cls, ctx.func);
  Trace::Bump bumper1{Trace::hhbbc, bump};
  Trace::Bump bumper2{Trace::hhbbc_cfg, bump};

  if (knownArgs) {
    FTRACE(2, "{:.^70}\n", "Inline Interp");
  }
  SCOPE_EXIT {
    if (knownArgs) {
      FTRACE(2, "{:.^70}\n", "End Inline Interp");
    }
  };

  FTRACE(2, "{:-^70}\n-- {}\n", "Analyze", show(ctx));

  /*
   * Set of RPO ids that still need to be visited.
   *
   * Initially, we need each entry block in this list.  As we visit
   * blocks, we propagate states to their successors and across their
   * back edges---when state merges cause a change to the block
   * stateIn, we will add it to this queue so it gets visited again.
   */
  auto incompleteQ = prepare_incompleteQ(index, ai, clsAnalysis, knownArgs);

  /*
   * There are potentially infinitely growing types when we're using
   * union_of to merge states, so occasonially we need to apply a
   * widening operator.
   *
   * Currently this is done by having a straight-forward hueristic: if
   * you visit a block too many times, we'll start doing all the
   * merges with the widening operator until we've had a chance to
   * visit the block again.  We must then continue iterating in case
   * the actual fixed point is higher than the result of widening.
   *
   * Terminiation is guaranteed because the widening operator has only
   * finite chains in the type lattice.
   */
  auto nonWideVisits = std::vector<uint32_t>(ctx.func->blocks.size());

  // For debugging, count how many times basic blocks get interpreted.
  auto interp_counter = uint32_t{0};

  // Used to force blocks that depended on the types of local statics
  // to be re-analyzed when the local statics change.
  std::unordered_map<borrowed_ptr<const php::Block>, std::map<LocalId, Type>>
    usedLocalStatics;

  /*
   * Iterate until a fixed point.
   *
   * Each time a stateIn for a block changes, we re-insert the block's
   * rpo ID in incompleteQ.  Since incompleteQ is ordered, we'll
   * always visit blocks with earlier RPO ids first, which hopefully
   * means less iterations.
   */
  do {
    while (!incompleteQ.empty()) {
      auto const blk = ai.rpoBlocks[incompleteQ.pop()];

      if (nonWideVisits[blk->id]++ > options.analyzeFuncWideningLimit) {
        nonWideVisits[blk->id] = 0;
      }

      FTRACE(2, "block #{}\nin {}{}", blk->id,
             state_string(*ctx.func, ai.bdata[blk->id].stateIn, collect),
             property_state_string(collect.props));
      ++interp_counter;

      auto propagate = [&] (BlockId target, const State& st) {
        auto const needsWiden =
          nonWideVisits[target] >= options.analyzeFuncWideningLimit;

        // We haven't optimized the widening operator much, because it
        // doesn't happen in practice right now.  We want to know when
        // it starts happening:
        if (needsWiden) {
          std::fprintf(stderr, "widening in %s on %s\n",
            ctx.unit->filename->data(),
            ctx.func->name->data());
        }

        FTRACE(2, "     {}-> {}\n", needsWiden ? "widening " : "", target);
        FTRACE(4, "target old {}",
               state_string(*ctx.func, ai.bdata[target].stateIn, collect));

        auto const changed =
          needsWiden ? widen_into(ai.bdata[target].stateIn, st)
                     : merge_into(ai.bdata[target].stateIn, st);
        if (changed) {
          incompleteQ.push(rpoId(ai, target));
        }
        FTRACE(4, "target new {}",
               state_string(*ctx.func, ai.bdata[target].stateIn, collect));
      };

      auto stateOut = ai.bdata[blk->id].stateIn;
      auto interp   = Interp { index, ctx, collect, blk, stateOut };
      auto flags    = run(interp, propagate);
      // We only care about the usedLocalStatics from the last visit
      if (flags.usedLocalStatics) {
        usedLocalStatics[blk] = std::move(*flags.usedLocalStatics);
      } else {
        usedLocalStatics.erase(blk);
      }

      if (flags.returned) {
        ai.inferredReturn |= std::move(*flags.returned);
      }
    }
    // maybe some local statics changed type since the last time their
    // blocks were visited.
    for (auto const& elm : usedLocalStatics) {
      for (auto const& ls : elm.second) {
        if (collect.localStaticTypes[ls.first] != ls.second) {
          incompleteQ.push(rpoId(ai, elm.first->id));
          break;
        }
      }
    }
  } while (!incompleteQ.empty());

  ai.closureUseTypes = std::move(collect.closureUseTypes);
  ai.cnsMap = std::move(collect.cnsMap);
  ai.readsUntrackedConstants = collect.readsUntrackedConstants;
  ai.mayUseVV = collect.mayUseVV;

  if (ctx.func->isGenerator) {
    if (ctx.func->isAsync) {
      // Async generators always return AsyncGenerator object.
      ai.inferredReturn = objExact(index.builtin_class(s_AsyncGenerator.get()));
    } else {
      // Non-async generators always return Generator object.
      ai.inferredReturn = objExact(index.builtin_class(s_Generator.get()));
    }
  } else if (ctx.func->isAsync) {
    // Async functions always return WaitH<T>, where T is the type returned
    // internally.
    ai.inferredReturn = wait_handle(index, ai.inferredReturn);
  }

  /*
   * If inferredReturn is TBottom, the callee didn't execute a return
   * at all.  (E.g. it unconditionally throws, or is an abstract
   * function body.)
   *
   * In this case, we leave the return type as TBottom, to indicate
   * the same to callers.
   */
  assert(ai.inferredReturn.subtypeOf(TGen));

  // For debugging, print the final input states for each block.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '=') + "\n";
    auto const sep = std::string(60, '-') + "\n";
    auto ret = folly::format(
      "{}function {} ({} block interps):\n{}",
      bsep,
      show(ctx),
      interp_counter,
      bsep
    ).str();
    for (auto& bd : ai.bdata) {
      folly::format(
        &ret,
        "{}block {}:\nin {}",
        sep,
        ai.rpoBlocks[bd.rpoId]->id,
        state_string(*ctx.func, bd.stateIn, collect)
      );
    }
    ret += sep + bsep;
    folly::format(&ret, "Inferred return type: {}\n", show(ai.inferredReturn));
    ret += bsep;
    return ret;
  }());

  // Do this after the tracing above
  ai.localStaticTypes = std::move(collect.localStaticTypes);
  return ai;
}

FuncAnalysis do_analyze(const Index& index,
                        Context const ctx,
                        ClassAnalysis* clsAnalysis,
                        const std::vector<Type>* knownArgs,
                        bool trackConstantArrays) {
  CollectedInfo collect {
    index, ctx, clsAnalysis, nullptr, trackConstantArrays
  };

  return do_analyze_collect(index, ctx, collect, clsAnalysis, knownArgs);
}

//////////////////////////////////////////////////////////////////////

/*
 * In the case of HNI builtin classes, private properties are
 * allowed to be mutated by native code, so we may not see all the
 * modifications.
 *
 * We are allowed to assume the type annotation on the property is
 * accurate, although nothing is currently checking that this is the
 * case.  We handle this right now by doing inference as if it
 * couldn't be affected by native code, then assert the inferred
 * type is at least a subtype of the annotated type, and expanding
 * it to be the annotated type if it is bigger.
 */
void expand_hni_prop_types(ClassAnalysis& clsAnalysis) {
  auto relax_prop = [&] (const php::Prop& prop, PropState& propState) {
    auto it = propState.find(prop.name);
    if (it == end(propState)) return;

    /*
     * When HardTypeHints isn't on, DisallowDynamicVarEnvFuncs isn't on, or any
     * functions are interceptable, we don't require the constraints to actually
     * match, and relax all the HNI types to Gen.
     *
     * This is because extensions may wish to assign to properties
     * after a typehint guard, which is going to fail without
     * HardTypeHints.  Or, with any interceptable functions, it's
     * quite possible that some function calls in systemlib might not
     * be known to return things matching the property type hints for
     * some properties, or not to take their arguments by reference.
     */
    auto const hniTy =
      !options.HardTypeHints ||
      !options.DisallowDynamicVarEnvFuncs ||
      clsAnalysis.anyInterceptable
        ? TGen
        : from_hni_constraint(prop.typeConstraint);
    if (it->second.subtypeOf(hniTy)) {
      it->second = hniTy;
      return;
    }

    std::fprintf(
      stderr,
      "HNI class %s::%s inferred property type (%s) doesn't "
        "match annotation (%s)\n",
      clsAnalysis.ctx.cls->name->data(),
      prop.name->data(),
      show(it->second).c_str(),
      show(hniTy).c_str()
    );
    always_assert(!"HNI property type annotation was wrong");
  };

  for (auto& prop : clsAnalysis.ctx.cls->properties) {
    relax_prop(prop, clsAnalysis.privateProperties);
    relax_prop(prop, clsAnalysis.privateStatics);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

FuncAnalysis::FuncAnalysis(Context ctx)
  : ctx(ctx)
  , rpoBlocks(rpoSortAddDVs(*ctx.func))
  , bdata(ctx.func->blocks.size())
  , inferredReturn(TBottom)
{
  for (auto rpoId = size_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    bdata[rpoBlocks[rpoId]->id].rpoId = rpoId;
  }
}

FuncAnalysis analyze_func(const Index& index, Context const ctx,
                          bool trackConstantArrays) {
  while (true) {
    auto ret = do_analyze(index, ctx, nullptr, nullptr, trackConstantArrays);
    if (!rebuild_exn_tree(ret)) return ret;
  }
}

FuncAnalysis analyze_func_collect(const Index& index,
                                  Context const ctx,
                                  CollectedInfo& collect) {
  return do_analyze_collect(index, ctx, collect, nullptr, nullptr);
}

FuncAnalysis analyze_func_inline(const Index& index,
                                 Context const ctx,
                                 std::vector<Type> args) {
  assert(!ctx.func->isClosureBody);
  return do_analyze(index, ctx, nullptr, &args, true);
}

ClassAnalysis analyze_class(const Index& index, Context const ctx) {

  assert(ctx.cls && !ctx.func);
  {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
        is_systemlib_part(*ctx.unit)};
    FTRACE(2, "{:#^70}\n", "Class");
  }

  ClassAnalysis clsAnalysis(ctx, index.any_interceptable_functions());
  auto const associatedClosures = index.lookup_closures(ctx.cls);
  auto const isHNIBuiltin       = ctx.cls->attrs & AttrBuiltin;

  /*
   * Initialize inferred private property types to their in-class
   * initializers.
   *
   * We need to loosen_all on instance properties, because the class could be
   * unserialized, which we don't guarantee preserves those aspects of the
   * type. The exception is properties marked as AttrNoSerialize, which won't be
   * unserialized.
   *
   * Also, set Uninit properties to TBottom, so that analysis
   * of 86pinit methods sets them to the correct type.
   */
  for (auto& prop : ctx.cls->properties) {
    if (!(prop.attrs & AttrPrivate)) continue;

    auto const cellTy = from_cell(prop.val);
    if (isHNIBuiltin) {
      auto const hniTy = from_hni_constraint(prop.typeConstraint);
      if (!cellTy.subtypeOf(hniTy)) {
        std::fprintf(stderr, "hni %s::%s has impossible type. "
                     "The annotation says it is type (%s) "
                     "but the default value is type (%s).\n",
                     ctx.cls->name->data(),
                     prop.name->data(),
                     show(hniTy).c_str(),
                     show(cellTy).c_str()
                     );
        always_assert(0 && "HNI systemlib has invalid type annotations");
      }
    }

    if (!(prop.attrs & AttrStatic)) {
      auto t = (prop.attrs & AttrNoSerialize) ? cellTy : loosen_all(cellTy);
      if (!is_closure(*ctx.cls) && t.subtypeOf(TUninit)) {
        /*
         * For non-closure classes, a property of type KindOfUninit
         * means that it has non-scalar initializer which will be set
         * by a 86pinit method.  For these classes, we want the
         * initial type of the property to be the type set by the
         * 86pinit method, so we set the type to TBottom.
         *
         * Closures will not have an 86pinit body, but still may have
         * properties of kind KindOfUninit (they will later contain
         * used variables or static locals for the closure body).  We
         * don't want to touch those.
         */
        t = TBottom;
      }
      clsAnalysis.privateProperties[prop.name] = t;
    } else {
      // Same thing as the above regarding TUninit and TBottom.
      // Static properties don't need to exclude closures for this,
      // though---we use instance properties for the closure
      // 86static_* properties.
      auto t = cellTy;
      if (t.subtypeOf(TUninit)) {
        t = TBottom;
      }
      clsAnalysis.privateStatics[prop.name] = t;
    }
  }

  /*
   * For classes with non-scalar initializers, the 86pinit and 86sinit
   * methods are guaranteed to run before any other method, and
   * are never called afterwards. Thus, we can analyze these
   * methods first to determine the initial types of properties with
   * non-scalar initializers, and these need not be be run again as part
   * of the fixedpoint computation.
   */
  if (auto f = find_method(ctx.cls, s_86pinit.get())) {
    do_analyze(
      index,
      Context { ctx.unit, f, ctx.cls },
      &clsAnalysis,
      nullptr,
      true
    );
  }
  if (auto f = find_method(ctx.cls, s_86sinit.get())) {
    do_analyze(
      index,
      Context { ctx.unit, f, ctx.cls },
      &clsAnalysis,
      nullptr,
      true
    );
  }

  // Verify that none of the class properties are TBottom, i.e.
  // any property of type KindOfUninit has been initialized (by
  // 86pinit or 86sinit).
  for (auto& prop : ctx.cls->properties) {
    if (!(prop.attrs & AttrPrivate)) continue;
    if (prop.attrs & AttrStatic) {
      assert(!clsAnalysis.privateStatics[prop.name].subtypeOf(TBottom));
    } else {
      assert(!clsAnalysis.privateProperties[prop.name].subtypeOf(TBottom));
    }
  }

  /*
   * Similar to the function case in do_analyze, we have to handle the
   * fact that there are infinitely growing chains in our type lattice
   * under union_of.
   *
   * So if we've visited the whole class some number of times and
   * still aren't at a fixed point, we'll set the property state to
   * the result of widening the old state with the new state, and then
   * reset the counter.  This guarantees eventual termination.
   */
  auto nonWideVisits = uint32_t{0};

  for (;;) {
    auto const previousProps   = clsAnalysis.privateProperties;
    auto const previousStatics = clsAnalysis.privateStatics;

    std::vector<FuncAnalysis> methodResults;
    std::vector<FuncAnalysis> closureResults;

    // Analyze every method in the class until we reach a fixed point
    // on the private property states.
    for (auto& f : ctx.cls->methods) {
      if (f->name->isame(s_86pinit.get()) ||
          f->name->isame(s_86sinit.get())) {
        continue;
      }

      methodResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, borrow(f), ctx.cls },
          &clsAnalysis,
          nullptr,
          true
        )
      );
    }

    if (associatedClosures) {
      for (auto& c : *associatedClosures) {
        auto const invoke = borrow(c->methods[0]);
        closureResults.push_back(
          do_analyze(
            index,
            Context { ctx.unit, invoke, c },
            &clsAnalysis,
            nullptr,
            true
          )
        );
      }
    }

    auto noExceptionalChanges = [&] {
      auto changes = false;
      for (auto& fa : methodResults) {
        if (rebuild_exn_tree(fa)) changes = true;
      }
      for (auto& fa : closureResults) {
        if (rebuild_exn_tree(fa)) changes = true;
      }
      return !changes;
    };

    // Check if we've reached a fixed point yet.
    if (previousProps   == clsAnalysis.privateProperties &&
        previousStatics == clsAnalysis.privateStatics &&
        noExceptionalChanges()) {
      clsAnalysis.methods  = std::move(methodResults);
      clsAnalysis.closures = std::move(closureResults);
      break;
    }

    if (nonWideVisits++ > options.analyzeClassWideningLimit) {
      auto const a = widen_into(clsAnalysis.privateProperties, previousProps);
      auto const b = widen_into(clsAnalysis.privateStatics, previousStatics);
      always_assert(a || b);
      nonWideVisits = 0;
    }
  }

  if (isHNIBuiltin) expand_hni_prop_types(clsAnalysis);

  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
      is_systemlib_part(*ctx.unit)};

  // For debugging, print the final state of the class analysis.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '+') + "\n";
    auto ret = folly::format(
      "{}class {}:\n{}",
      bsep,
      ctx.cls->name,
      bsep
    ).str();
    for (auto& kv : clsAnalysis.privateProperties) {
      ret += folly::format(
        "private ${: <14} :: {}\n",
        kv.first,
        show(kv.second)
      ).str();
    }
    for (auto& kv : clsAnalysis.privateStatics) {
      ret += folly::format(
        "private static ${: <14} :: {}\n",
        kv.first,
        show(kv.second)
      ).str();
    }
    ret += bsep;
    return ret;
  }());

  return clsAnalysis;
}

//////////////////////////////////////////////////////////////////////

std::vector<std::pair<State,StepFlags>>
locally_propagated_states(const Index& index,
                          const FuncAnalysis& fa,
                          borrowed_ptr<const php::Block> blk,
                          State state) {
  Trace::Bump bumper{Trace::hhbbc, 10};

  std::vector<std::pair<State,StepFlags>> ret;
  ret.reserve(blk->hhbcs.size() + 1);

  CollectedInfo collect { index, fa.ctx, nullptr, nullptr, true, &fa };
  auto interp = Interp { index, fa.ctx, collect, blk, state };

  for (auto& op : blk->hhbcs) {
    ret.emplace_back(state, StepFlags{});
    ret.back().second = step(interp, op);
  }

  ret.emplace_back(std::move(state), StepFlags{});
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
