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
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options-util.h"

#include "hphp/runtime/vm/reified-generics.h"

namespace HPHP::HHBBC {

namespace {

TRACE_SET_MOD(hhbbc);

struct KnownArgs {
  Type context;
  const CompactVector<Type>& args;
};

//////////////////////////////////////////////////////////////////////

const StaticString s_Closure("Closure");

//////////////////////////////////////////////////////////////////////

/*
 * Short-hand to get the rpoId of a block in a given FuncAnalysis.  (The RPO
 * ids are re-assigned per analysis.)
 */
uint32_t rpoId(const FuncAnalysis& ai, BlockId blk) {
  return ai.bdata[blk].rpoId;
}

const StaticString s_reified_generics_var("0ReifiedGenerics");
const StaticString s_coeffects_var("0Coeffects");

Optional<State> entry_state(const Index& index, CollectedInfo& collect,
                            const Context& ctx, const KnownArgs* knownArgs) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisType = [&] {
    if (!ctx.cls) return TNull;
    if (knownArgs && !knownArgs->context.subtypeOf(BBottom)) {
      if (knownArgs->context.subtypeOf(BOptObj)) {
        return setctx(knownArgs->context);
      }
      if (knownArgs->context.subtypeOf(BCls) &&
          is_specialized_cls(knownArgs->context)) {
        return setctx(toobj(knownArgs->context));
      }
    }
    auto const maybeThisType = thisType(index, ctx);
    auto thisType = maybeThisType ? *maybeThisType : TObj;
    if (index.lookup_this_available(ctx.func)) return thisType;
    return opt(std::move(thisType));
  }();
  ret.locals.resize(ctx.func->locals.size());
  ret.iters.resize(ctx.func->numIters);

  auto locId = uint32_t{0};
  for (; locId < ctx.func->params.size(); ++locId) {
    if (knownArgs) {
      if (locId < knownArgs->args.size()) {
        if (ctx.func->params[locId].isVariadic) {
          std::vector<Type> pack(knownArgs->args.begin() + locId,
                                 knownArgs->args.end());
          for (auto& p : pack) p = unctx(std::move(p));
          ret.locals[locId] = vec(std::move(pack));
        } else {
          auto [ty, _, effectFree] =
            index.verify_param_type(ctx, locId, unctx(knownArgs->args[locId]));

          if (ty.subtypeOf(BBottom)) {
            ret.unreachable = true;
            if (ctx.func->params[locId].dvEntryPoint == NoBlockId) {
              return std::nullopt;
            }
          }

          ret.locals[locId] = std::move(ty);
          collect.effectFree &= effectFree;
        }
      } else {
        ret.locals[locId] = ctx.func->params[locId].isVariadic ? TVec : TUninit;
      }
      continue;
    }

    if (ctx.func->params[locId].isVariadic) {
      ret.locals[locId] = TVec;
      continue;
    }

    // Because we throw a non-recoverable error for having fewer than the
    // required number of args, all function parameters must be initialized.

    auto [ty, _, effectFree] =
      index.verify_param_type(ctx, locId, TInitCell);

    if (ty.subtypeOf(BBottom)) {
      ret.unreachable = true;
      if (ctx.func->params[locId].dvEntryPoint == NoBlockId) {
        return std::nullopt;
      }
    }

    ret.locals[locId] = std::move(ty);
    collect.effectFree &= effectFree;
  }

  // Closures have use vars, we need to look up their types from the index.
  auto const useVars = ctx.func->isClosureBody
    ? index.lookup_closure_use_vars(ctx.func)
    : CompactVector<Type>{};

  /*
   * Reified functions have a hidden local that's always the first
   * (non-parameter) local, which stores reified generics.
   */
  if (ctx.func->isReified) {
    // Currently closures cannot be reified
    assertx(!ctx.func->isClosureBody);
    assertx(locId < ret.locals.size());
    assertx(ctx.func->locals[locId].name->same(s_reified_generics_var.get()));
    ret.locals[locId++] = get_type_of_reified_list(ctx.func->userAttributes);
  }

  /*
   * Functions with coeffect rules have a hidden local that's always the first
   * (non-parameter) local (after reified generics, if exists),
   * which stores the ambient coeffects.
   */
  if (has_coeffects_local(ctx.func)) {
    assertx(locId < ret.locals.size());
    assertx(ctx.func->locals[locId].name->same(s_coeffects_var.get()));
    ret.locals[locId++] = TInt;
  }

  auto afterParamsLocId = uint32_t{0};
  for (; locId < ctx.func->locals.size(); ++locId, ++afterParamsLocId) {
    /*
     * Some of the closure locals are mapped to used variables. The types of
     * use vars are looked up from the index.
     */
    if (ctx.func->isClosureBody) {
      if (afterParamsLocId < useVars.size()) {
        ret.locals[locId] = useVars[afterParamsLocId];
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
      ret.locals[locId] = TCell;
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
                    CollectedInfo& collect,
                    const KnownArgs* knownArgs) {
  auto incompleteQ     = dataflow_worklist<uint32_t>(ai.rpoBlocks.size());
  auto const ctx       = ai.ctx;
  auto const numParams = ctx.func->params.size();

  auto const entryState = entry_state(index, collect, ctx, knownArgs);
  if (!entryState) return incompleteQ;

  if (knownArgs) {
    // When we have known args, we only need to add one of the entry points to
    // the initial state, since we know how many arguments were passed.
    auto const useDvInit = [&] {
      if (knownArgs->args.size() >= numParams) return false;
      for (auto i = knownArgs->args.size(); i < numParams; ++i) {
        auto const dv = ctx.func->params[i].dvEntryPoint;
        if (dv != NoBlockId) {
          ai.bdata[dv].stateIn.copy_from(*entryState);
          ai.bdata[dv].stateIn.unreachable = false;
          incompleteQ.push(rpoId(ai, dv));
          return true;
        }
      }
      return false;
    }();

    if (!useDvInit && !entryState->unreachable) {
      ai.bdata[ctx.func->mainEntry].stateIn.copy_from(*entryState);
      incompleteQ.push(rpoId(ai, ctx.func->mainEntry));
    }

    return incompleteQ;
  }

  for (auto paramId = uint32_t{0}; paramId < numParams; ++paramId) {
    auto const dv = ctx.func->params[paramId].dvEntryPoint;
    if (dv != NoBlockId) {
      ai.bdata[dv].stateIn.copy_from(*entryState);
      ai.bdata[dv].stateIn.unreachable = false;
      incompleteQ.push(rpoId(ai, dv));
      for (auto locId = paramId; locId < numParams; ++locId) {
        ai.bdata[dv].stateIn.locals[locId] =
          ctx.func->params[locId].isVariadic ? TVec : TUninit;
      }
    }
  }

  if (!entryState->unreachable) {
    ai.bdata[ctx.func->mainEntry].stateIn.copy_from(*entryState);
    incompleteQ.push(rpoId(ai, ctx.func->mainEntry));
  }

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
AnalysisContext adjust_closure_context(const Index& index,
                                       AnalysisContext ctx) {
  if (ctx.cls) ctx.cls = index.lookup_closure_context(*ctx.cls);
  return ctx;
}

FuncAnalysis do_analyze_collect(const Index& index,
                                const AnalysisContext& ctx,
                                CollectedInfo& collect,
                                const KnownArgs* knownArgs) {
  assertx(ctx.cls == adjust_closure_context(index, ctx).cls);
  auto ai = FuncAnalysis { ctx };

  SCOPE_ASSERT_DETAIL("do-analyze-collect-2") {
    std::string ret;
    for (auto bid : ctx.func.blockRange()) {
      folly::format(&ret,
                    "block #{}\nin-{}\n{}",
                    bid,
                    state_string(*ctx.func, ai.bdata[bid].stateIn, collect),
                    show(*ctx.func, *ctx.func.blocks()[bid])
                   );
    }

    return ret;
  };

  SCOPE_ASSERT_DETAIL("do-analyze-collect-1") {
    return "Analyzing: " + show(ctx);
  };

  auto const bump = trace_bump_for(ctx.cls, ctx.func);
  Trace::Bump bumper1{Trace::hhbbc, bump};
  Trace::Bump bumper2{Trace::hhbbc_cfg, bump};
  Trace::Bump bumper3{Trace::hhbbc_index, bump};

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
  auto incompleteQ = prepare_incompleteQ(index, ai, collect, knownArgs);

  /*
   * There are potentially infinitely growing types when we're using union_of to
   * merge states, so occasionally we need to apply a widening operator.
   *
   * Currently this is done by having a straight-forward hueristic: if you visit
   * a block too many times, we'll start doing all the merges with the widening
   * operator. We must then continue iterating in case the actual fixed point is
   * higher than the result of widening. Likewise if we loop too much because of
   * local static types changing, we'll widen those.
   *
   * Termination is guaranteed because the widening operator has only finite
   * chains in the type lattice.
   */
  auto totalVisits = std::vector<uint32_t>(ctx.func.blocks().size());

  // For debugging, count how many times basic blocks get interpreted.
  auto interp_counter = uint32_t{0};

  hphp_fast_map<BlockId, BlockUpdateInfo> blockUpdates;

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
      auto const bid = ai.rpoBlocks[incompleteQ.pop()];

      totalVisits[bid]++;

      FTRACE(2, "block #{}\nin {}{}", bid,
             state_string(*ctx.func, ai.bdata[bid].stateIn, collect),
             property_state_string(collect.props));
      ++interp_counter;

      auto propagate = [&] (BlockId target, const State* st) {
        if (!st) {
          FTRACE(2, "     Force reprocess: {}\n", target);
          incompleteQ.push(rpoId(ai, target));
          return;
        }

        auto const needsWiden =
          totalVisits[target] >= options.analyzeFuncWideningLimit;

        FTRACE(2, "     {}-> {}\n", needsWiden ? "widening " : "", target);
        FTRACE(4, "target old {}",
               state_string(*ctx.func, ai.bdata[target].stateIn, collect));

        auto const changed =
          needsWiden ? widen_into(ai.bdata[target].stateIn, *st)
                     : merge_into(ai.bdata[target].stateIn, *st);
        if (changed) {
          incompleteQ.push(rpoId(ai, target));
        }
        FTRACE(4, "target new {}",
               state_string(*ctx.func, ai.bdata[target].stateIn, collect));
      };

      auto const blk = ctx.func.blocks()[bid].get();
      auto stateOut = ai.bdata[bid].stateIn;
      auto interp   = Interp { index, ctx, collect, bid, blk, stateOut };
      auto flags    = run(interp, ai.bdata[bid].stateIn, propagate);
      if (any(collect.opts & CollectionOpts::EffectFreeOnly) &&
          !collect.effectFree) {
        break;
      }
      if (flags.updateInfo.replacedBcs.size() ||
          flags.updateInfo.unchangedBcs != blk->hhbcs.size() ||
          flags.updateInfo.fallthrough != blk->fallthrough) {
        blockUpdates[bid] = std::move(flags.updateInfo);
      } else {
        blockUpdates.erase(bid);
      }

      if (flags.returned) {
        ai.inferredReturn |= std::move(*flags.returned);
        if (flags.retParam == NoLocalId) {
          ai.retParam = NoLocalId;
        } else if (ai.retParam != flags.retParam) {
          if (ai.retParam != MaxLocalId) {
            ai.retParam = NoLocalId;
          } else {
            ai.retParam = flags.retParam;
          }
        }
      }

      ai.bdata[bid].noThrow = flags.noThrow;
    }

    if (any(collect.opts & CollectionOpts::EffectFreeOnly) &&
        !collect.effectFree) {
      break;
    }
  } while (!incompleteQ.empty());

  ai.closureUseTypes = std::move(collect.closureUseTypes);
  ai.effectFree = collect.effectFree;
  ai.hasInvariantIterBase = collect.hasInvariantIterBase;
  ai.unfoldableFuncs = collect.unfoldableFuncs;
  ai.usedParams = collect.usedParams;
  ai.publicSPropMutations = std::move(collect.publicSPropMutations);
  for (auto& elm : blockUpdates) {
    ai.blockUpdates.emplace_back(elm.first, std::move(elm.second));
  }
  index.fixup_return_type(ctx.func, ai.inferredReturn);

  /*
   * If inferredReturn is TBottom, the callee didn't execute a return
   * at all.  (E.g. it unconditionally throws, or is an abstract
   * function body.)
   *
   * In this case, we leave the return type as TBottom, to indicate
   * the same to callers.
   */
  assertx(ai.inferredReturn.subtypeOf(BCell));

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
        "{}block {}{}:\nin {}",
        sep,
        ai.rpoBlocks[bd.rpoId],
        bd.noThrow ? " (no throw)" : "",
        state_string(*ctx.func, bd.stateIn, collect)
      );
    }
    ret += sep + bsep;
    folly::format(
      &ret,
      "Inferred return type: {}{}\n",
      show(ai.inferredReturn),
      ai.effectFree ? " (effect-free)" : ""
    );
    ret += bsep;
    return ret;
  }());
  return ai;
}

FuncAnalysis do_analyze(const Index& index,
                        const AnalysisContext& inputCtx,
                        ClassAnalysis* clsAnalysis,
                        const KnownArgs* knownArgs = nullptr,
                        CollectionOpts opts = CollectionOpts{}) {
  auto const ctx = adjust_closure_context(index, inputCtx);
  auto collect = CollectedInfo { index, ctx, clsAnalysis, opts };

  auto ret = do_analyze_collect(index, ctx, collect, knownArgs);
  if (ctx.func->name == s_86cinit.get() && !knownArgs) {
    // We need to try to resolve any dynamic constants
    size_t idx = 0;
    for (auto const& c : ctx.cls->constants) {
      if (c.val && c.val->m_type == KindOfUninit) {
        auto const fa = analyze_func_inline(index, ctx, TCls, { sval(c.name) });
        ret.resolvedConstants.emplace_back(idx, unctx(fa.inferredReturn));
      }
      ++idx;
    }
  }
  return ret;
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

    it->second.everModified = true;

    /*
     * When any functions are interceptable, we don't require the constraints to
     * actually match, and relax all the HNI types to Gen.
     *
     * This is because with any interceptable functions, it's quite
     * possible that some function calls in systemlib might not be
     * known to return things matching the property type hints for
     * some properties, or not to take their arguments by reference.
     */
    auto const hniTy = from_hni_constraint(prop.userType);
    if (it->second.ty.subtypeOf(hniTy)) {
      it->second.ty = hniTy;
      return;
    }

    always_assert_flog(
      false,
      "HNI class {}::{} inferred property type ({}) doesn't "
        "match annotation ({})\n",
      clsAnalysis.ctx.cls->name,
      prop.name,
      show(it->second.ty),
      show(hniTy)
    );
  };

  for (auto& prop : clsAnalysis.ctx.cls->properties) {
    relax_prop(prop, clsAnalysis.privateProperties);
    relax_prop(prop, clsAnalysis.privateStatics);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

const php::Func* ClassAnalysisWorklist::next() {
  if (worklist.empty()) return nullptr;
  auto f = worklist.front();
  inWorklist.erase(f);
  worklist.pop_front();
  return f;
}

bool ClassAnalysisWorklist::schedule(const php::Func& f) {
  auto const insert = inWorklist.emplace(&f);
  if (!insert.second) return false;
  worklist.emplace_back(&f);
  return true;
}

void ClassAnalysisWorklist::scheduleForProp(SString name) {
  auto const it = propDeps.find(name);
  if (it == propDeps.end()) return;
  for (auto const f : it->second) schedule(*f);
}

void ClassAnalysisWorklist::scheduleForPropMutate(SString name) {
  auto const it = propMutateDeps.find(name);
  if (it == propMutateDeps.end()) return;
  for (auto const f : it->second) schedule(*f);
}

void ClassAnalysisWorklist::scheduleForReturnType(const php::Func& callee) {
  auto const it = returnTypeDeps.find(&callee);
  if (it == returnTypeDeps.end()) return;
  for (auto const f : it->second) schedule(*f);
}

//////////////////////////////////////////////////////////////////////

FuncAnalysisResult::FuncAnalysisResult(AnalysisContext ctx)
  : ctx(ctx)
  , inferredReturn(TBottom)
{
}

FuncAnalysis::FuncAnalysis(AnalysisContext ctx)
  : FuncAnalysisResult{ctx}
  , rpoBlocks{rpoSortAddDVs(ctx.func)}
  , bdata{ctx.func.blocks().size()}
{
  for (auto rpoId = size_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    bdata[rpoBlocks[rpoId]].rpoId = rpoId;
  }
}

FuncAnalysis analyze_func(const Index& index, const AnalysisContext& ctx,
                          CollectionOpts opts) {
  return do_analyze(index, ctx, nullptr, nullptr, opts);
}

FuncAnalysis analyze_func_inline(const Index& index,
                                 const AnalysisContext& ctx,
                                 const Type& thisType,
                                 const CompactVector<Type>& args,
                                 CollectionOpts opts) {
  auto const knownArgs = KnownArgs {
    ctx.func->isClosureBody ? TBottom : thisType,
    args
  };

  return do_analyze(index, ctx, nullptr, &knownArgs,
                    opts | CollectionOpts::Inlining);
}

ClassAnalysis analyze_class(const Index& index, const Context& ctx) {

  assertx(ctx.cls && !ctx.func && !is_used_trait(*ctx.cls));

  {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
      is_systemlib_part(*ctx.unit)};
    FTRACE(2, "{:#^70}\n", "Class");
  }

  ClassAnalysis clsAnalysis(ctx);
  auto const associatedClosures = index.lookup_closures(ctx.cls);
  auto const associatedMethods  = index.lookup_extra_methods(ctx.cls);
  auto const isHNIBuiltin       = ctx.cls->attrs & AttrBuiltin;

  /*
   * Initialize inferred private property types to their in-class
   * initializers.
   *
   * We need to loosen_all on instance properties, because the class could be
   * unserialized, which we don't guarantee preserves those aspects of the
   * type.
   *
   * Also, set Uninit properties to TBottom, so that analysis
   * of 86pinit methods sets them to the correct type.
   */
  for (auto& prop : const_cast<php::Class*>(ctx.cls)->properties) {
    auto const cellTy = from_cell(prop.val);

    if (prop_might_have_bad_initial_value(index, *ctx.cls, prop)) {
      prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
      // If Uninit, it will be determined in the 86[s,p]init function.
      if (!cellTy.subtypeOf(BUninit)) clsAnalysis.badPropInitialValues = true;
    } else {
      prop.attrs |= AttrInitialSatisfiesTC;
    }

    if (!(prop.attrs & AttrPrivate)) continue;

    if (isHNIBuiltin) {
      auto const hniTy = from_hni_constraint(prop.userType);
      if (!cellTy.subtypeOf(hniTy)) {
        always_assert_flog(
          false,
          "hni {}::{} has impossible type. "
          "The annotation says it is type ({}) "
          "but the default value is type ({}).\n",
          ctx.cls->name,
          prop.name,
          show(hniTy),
          show(cellTy)
        );
      }
    }

    if (!(prop.attrs & AttrStatic)) {
      auto t = loosen_this_prop_for_serialization(*ctx.cls, prop.name, cellTy);

      if (!is_closure(*ctx.cls) && t.subtypeOf(BUninit)) {
        /*
         * For non-closure classes, a property of type KindOfUninit
         * means that it has non-scalar initializer which will be set
         * by a 86pinit method.  For these classes, we want the
         * initial type of the property to be the type set by the
         * 86pinit method, so we set the type to TBottom.
         *
         * Closures will not have an 86pinit body, but still may have
         * properties of kind KindOfUninit (they will later contain
         * used variables).  We don't want to touch those.
         */
        t = TBottom;
      } else if (!(prop.attrs & AttrSystemInitialValue)) {
        t = adjust_type_for_prop(index, *ctx.cls, &prop.typeConstraint, t);
      }
      auto& elem = clsAnalysis.privateProperties[prop.name];
      elem.ty = std::move(t);
      elem.tc = &prop.typeConstraint;
      elem.attrs = prop.attrs;
      elem.everModified = false;
    } else {
      // Same thing as the above regarding TUninit and TBottom.
      // Static properties don't need to exclude closures for this,
      // though---we use instance properties for the closure use vars.
      auto t = cellTy.subtypeOf(BUninit)
        ? TBottom
        : (prop.attrs & AttrSystemInitialValue)
          ? cellTy
          : adjust_type_for_prop(index, *ctx.cls, &prop.typeConstraint, cellTy);
      auto& elem = clsAnalysis.privateStatics[prop.name];
      elem.ty = std::move(t);
      elem.tc = &prop.typeConstraint;
      elem.attrs = prop.attrs;
      elem.everModified = false;
    }
  }

  /*
   * For builtins, we assume the runtime can write to the properties
   * in un-analyzable ways (but won't violate their type-hint). So,
   * expand the analyzed types to at least include the type-hint.
   */
  if (isHNIBuiltin) expand_hni_prop_types(clsAnalysis);

  /*
   * For classes with non-scalar initializers, the 86pinit, 86sinit,
   * 86linit, 86cinit, and 86reifiedinit methods are guaranteed to run
   * before any other method, and are never called afterwards. Thus,
   * we can analyze these methods first to determine the initial types
   * of properties with non-scalar initializers, and these need not be
   * be run again as part of the fixedpoint computation.
   */
  CompactVector<FuncAnalysis> initResults;
  auto analyze_86init = [&](const StaticString &name) {
    if (auto func = find_method(ctx.cls, name.get())) {
      auto const wf = php::WideFunc::cns(func);
      auto const context = AnalysisContext { ctx.unit, wf, ctx.cls };
      initResults.push_back(do_analyze(index, context, &clsAnalysis));
    }
  };
  analyze_86init(s_86pinit);
  analyze_86init(s_86sinit);
  analyze_86init(s_86linit);
  analyze_86init(s_86cinit);
  analyze_86init(s_86reifiedinit);

  // NB: Properties can still be TBottom at this point if their initial values
  // cannot possibly satisfy their type-constraints. The classes of such
  // properties cannot be instantiated.

  /*
   * Similar to the function case in do_analyze, we have to handle the
   * fact that there are infinitely growing chains in our type lattice
   * under union_of.
   *
   * So if we've visited a func some number of times and still aren't
   * at a fixed point, we'll set the property state to the result of
   * widening the old state with the new state, and then reset the
   * counter.  This guarantees eventual termination.
   */

  ClassAnalysisWork work;
  clsAnalysis.work = &work;

  clsAnalysis.methods.reserve(initResults.size() + ctx.cls->methods.size());
  for (auto& m : initResults) {
    clsAnalysis.methods.emplace_back(std::move(m));
  }
  if (associatedClosures) {
    clsAnalysis.closures.reserve(associatedClosures->size());
  }

  auto const startPrivateProperties = clsAnalysis.privateProperties;
  auto const startPrivateStatics = clsAnalysis.privateStatics;

  struct FuncMeta {
    const php::Unit* unit;
    const php::Class* cls;
    CompactVector<FuncAnalysisResult>* output;
    size_t startReturnRefinements;
    size_t localReturnRefinements = 0;
    int outputIdx = -1;
    size_t visits = 0;
  };
  hphp_fast_map<const php::Func*, FuncMeta> funcMeta;

  auto const getMeta = [&] (const php::Func& f) -> FuncMeta& {
    auto metaIt = funcMeta.find(&f);
    assertx(metaIt != funcMeta.end());
    return metaIt->second;
  };

  // Build up the initial worklist:
  for (auto const& f : ctx.cls->methods) {
    if (f->name == s_86pinit.get() ||
        f->name == s_86sinit.get() ||
        f->name == s_86linit.get() ||
        f->name == s_86cinit.get() ||
        f->name == s_86reifiedinit.get()) {
      continue;
    }
    auto const DEBUG_ONLY inserted = work.worklist.schedule(*f);
    assertx(inserted);
    auto [type, refinements] = index.lookup_return_type_raw(f.get());
    work.returnTypes.emplace(f.get(), std::move(type));
    funcMeta.emplace(
      f.get(),
      FuncMeta{ctx.unit, ctx.cls, &clsAnalysis.methods, refinements}
    );
  }

  if (associatedClosures) {
    for (auto const c : *associatedClosures) {
      auto const f = c->methods[0].get();
      auto const DEBUG_ONLY inserted = work.worklist.schedule(*f);
      assertx(inserted);
      auto [type, refinements] = index.lookup_return_type_raw(f);
      work.returnTypes.emplace(f, std::move(type));
      funcMeta.emplace(
        f, FuncMeta{ctx.unit, c, &clsAnalysis.closures, refinements}
      );
    }
  }
  if (associatedMethods) {
    for (auto const m : *associatedMethods) {
      auto const DEBUG_ONLY inserted = work.worklist.schedule(*m);
      assertx(inserted);
      funcMeta.emplace(
        m,
        FuncMeta{index.lookup_func_unit(*m), ctx.cls, nullptr, 0, 0}
      );
    }
  }

  // Keep analyzing until we have more functions scheduled (the fixed
  // point).
  while (!work.worklist.empty()) {
    // First analyze funcs until we hit a fixed point for the
    // properties. Until we reach that, the return types are *not*
    // guaranteed to be correct.
    while (auto const f = work.worklist.next()) {
      auto& meta = getMeta(*f);

      auto const wf = php::WideFunc::cns(f);
      auto const context = AnalysisContext { meta.unit, wf, meta.cls };
      auto results = do_analyze(index, context, &clsAnalysis);

      if (meta.output) {
        if (meta.outputIdx < 0) {
          meta.outputIdx = meta.output->size();
          meta.output->emplace_back(std::move(results));
        } else {
          (*meta.output)[meta.outputIdx] = std::move(results);
        }
      }

      if (meta.visits++ >= options.analyzeClassWideningLimit) {
        for (auto& prop : clsAnalysis.privateProperties) {
          auto wide = widen_type(prop.second.ty);
          if (prop.second.ty.strictlyMoreRefined(wide)) {
            prop.second.ty = std::move(wide);
            work.worklist.scheduleForProp(prop.first);
          }
        }
        for (auto& prop : clsAnalysis.privateStatics) {
          auto wide = widen_type(prop.second.ty);
          if (prop.second.ty.strictlyMoreRefined(wide)) {
            prop.second.ty = std::move(wide);
            work.worklist.scheduleForProp(prop.first);
          }
        }
      }
    }

    // We've hit a fixed point for the properties. Other local
    // information (such as return type information) is now correct
    // (but might not be optimal).

    auto bail = false;

    // Reflect any improved return types into the results. This will
    // make them available for local analysis and they'll eventually
    // be written back into the Index.
    for (auto& kv : funcMeta) {
      auto const f = kv.first;
      auto& meta = kv.second;
      if (!meta.output) continue;
      assertx(meta.outputIdx >= 0);
      auto& results = (*meta.output)[meta.outputIdx];

      auto const oldTypeIt = work.returnTypes.find(f);
      assertx(oldTypeIt != work.returnTypes.end());
      auto& oldType = oldTypeIt->second;

      // Heed the return type refinement limit
      if (results.inferredReturn.strictlyMoreRefined(oldType)) {
        if (meta.startReturnRefinements + meta.localReturnRefinements
            < options.returnTypeRefineLimit) {
          oldType = results.inferredReturn;
          work.worklist.scheduleForReturnType(*f);
        } else if (meta.localReturnRefinements > 0) {
          results.inferredReturn = oldType;
        }
        ++meta.localReturnRefinements;
      } else if (!results.inferredReturn.moreRefined(oldType)) {
        // If we have a monotonicity violation, bail out immediately
        // and let the Index complain.
        bail = true;
        break;
      }

      results.localReturnRefinements = meta.localReturnRefinements;
      if (results.localReturnRefinements > 0) --results.localReturnRefinements;
    }
    if (bail) break;

    hphp_fast_set<const php::Func*> changed;

    // We've made the return types available for local analysis. Now
    // iterate again and see if we can improve them.
    while (auto const f = work.worklist.next()) {
      auto& meta = getMeta(*f);

      auto const wf = php::WideFunc::cns(f);
      auto const context = AnalysisContext { meta.unit, wf, meta.cls };

      work.propsRefined = false;
      auto results = do_analyze(index, context, &clsAnalysis);
      assertx(!work.propsRefined);

      if (!meta.output) continue;

      auto returnTypeIt = work.returnTypes.find(f);
      assertx(returnTypeIt != work.returnTypes.end());

      auto& oldReturn = returnTypeIt->second;

      // Heed the return type refinement limit
      if (results.inferredReturn.strictlyMoreRefined(oldReturn)) {
        if (meta.startReturnRefinements + meta.localReturnRefinements
            < options.returnTypeRefineLimit) {
          oldReturn = results.inferredReturn;
          work.worklist.scheduleForReturnType(*f);
          changed.emplace(f);
        } else if (meta.localReturnRefinements > 0) {
          results.inferredReturn = oldReturn;
        }
        ++meta.localReturnRefinements;
      } else if (!results.inferredReturn.moreRefined(oldReturn)) {
        // If we have a monotonicity violation, bail out immediately
        // and let the Index complain.
        bail = true;
        break;
      }

      results.localReturnRefinements = meta.localReturnRefinements;
      if (results.localReturnRefinements > 0) --results.localReturnRefinements;

      assertx(meta.outputIdx >= 0);
      (*meta.output)[meta.outputIdx] = std::move(results);
    }
    if (bail) break;

    // Return types have reached a fixed point. However, this means
    // that we might be able to further improve property types. So, if
    // a method has an improved return return, examine the methods
    // which depend on that return type. Drop any property info for
    // properties those methods write to. Reschedule any methods which
    // or write to those properties. The idea is we want to re-analyze
    // all mutations of those properties again, since the refined
    // returned types may result in better property types. This
    // process may repeat multiple times, but will eventually reach a
    // fixed point.

    if (!work.propMutators.empty()) {
      auto const resetProp = [&] (SString name,
                                  const PropState& src,
                                  PropState& dst) {
        auto dstIt = dst.find(name);
        auto const srcIt = src.find(name);
        if (dstIt == dst.end()) {
          assertx(srcIt == src.end());
          return;
        }
        assertx(srcIt != src.end());
        dstIt->second.ty = srcIt->second.ty;
        dstIt->second.everModified = srcIt->second.everModified;
      };

      hphp_fast_set<SString> retryProps;
      for (auto const f : changed) {
        auto const deps = work.worklist.depsForReturnType(*f);
        if (!deps) continue;
        for (auto const dep : *deps) {
          auto const propsIt = work.propMutators.find(dep);
          if (propsIt == work.propMutators.end()) continue;
          for (auto const prop : propsIt->second) retryProps.emplace(prop);
        }
      }

      // Schedule the funcs which mutate the props before the ones
      // that read them.
      for (auto const prop : retryProps) {
        resetProp(prop, startPrivateProperties,
                  clsAnalysis.privateProperties);
        resetProp(prop, startPrivateStatics,
                  clsAnalysis.privateStatics);
        work.worklist.scheduleForPropMutate(prop);
      }
      for (auto const prop : retryProps) {
        work.worklist.scheduleForProp(prop);
      }
    }

    // This entire loop will eventually terminate when we cannot
    // improve properties nor return types.
  }

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
        show(kv.second.ty)
      ).str();
    }
    for (auto& kv : clsAnalysis.privateStatics) {
      ret += folly::format(
        "private static ${: <14} :: {}\n",
        kv.first,
        show(kv.second.ty)
      ).str();
    }
    ret += bsep;
    return ret;
  }());

  clsAnalysis.work = nullptr;
  return clsAnalysis;
}

//////////////////////////////////////////////////////////////////////

PropagatedStates::PropagatedStates(State&& state, StateMutationUndo undos)
  : m_locals{std::move(state.locals)}
  , m_undos{std::move(undos)}
{
  for (size_t i = 0; i < state.stack.size(); ++i) {
    m_stack.emplace_back(std::move(state.stack[i].type));
  }
}

void PropagatedStates::next() {
  // The undo log shouldn't be empty, and we should be at a mark
  // (which marks instruction boundary).
  assertx(!m_undos.events.empty());
  assertx(boost::get<StateMutationUndo::Mark>(&m_undos.events.back()));

  m_lastPush.reset();
  m_afterLocals.clear();
  m_undos.events.pop_back();

  // Use the undo log to "unwind" the current state.
  while (true) {
    assertx(!m_undos.events.empty());
    auto const stop = match<bool>(
      m_undos.events.back(),
      [] (const StateMutationUndo::Mark&) { return true; },
      [this] (StateMutationUndo::Push) {
        assertx(!m_stack.empty());
        if (!m_lastPush) m_lastPush.emplace(std::move(m_stack.back()));
        m_stack.pop_back();
        return false;
      },
      [this] (StateMutationUndo::Pop& p) {
        m_stack.emplace_back(std::move(p.t));
        return false;
      },
      [this] (StateMutationUndo::Stack& s) {
        assertx(s.idx < m_stack.size());
        m_stack[s.idx] = std::move(s.t);
        return false;
      },
      [this] (StateMutationUndo::Local& l) {
        assertx(l.id < m_locals.size());
        auto& old = m_locals[l.id];
        m_afterLocals.emplace_back(std::make_pair(l.id, std::move(old)));
        old = std::move(l.t);
        return false;
      }
    );
    if (stop) break;
    m_undos.events.pop_back();
  }
}

PropagatedStates
locally_propagated_states(const Index& index,
                          const AnalysisContext& ctx,
                          CollectedInfo& collect,
                          BlockId bid,
                          State state) {
  Trace::Bump bumper{Trace::hhbbc, 10};

  auto const blk = ctx.func.blocks()[bid].get();

  // Set up the undo log for the interp. We reserve it using this size
  // heuristic which captures typical undo log sizes.
  StateMutationUndo undos;
  undos.events.reserve((blk->hhbcs.size() + 1) * 4);

  auto interp = Interp { index, ctx, collect, bid, blk, state, &undos };

  for (auto const& op : blk->hhbcs) {
    auto const markIdx = undos.events.size();
    // Record instruction boundary
    undos.events.emplace_back(StateMutationUndo::Mark{});
    // Interpret it. This appends more info to the undo log.
    auto const stepFlags = step(interp, op);
    // Store the step flags in the mark we recorded before the
    // interpret.
    auto& mark = boost::get<StateMutationUndo::Mark>(undos.events[markIdx]);
    mark.wasPEI = stepFlags.wasPEI;
    mark.mayReadLocalSet = stepFlags.mayReadLocalSet;
    mark.unreachable = state.unreachable;
    state.stack.compact();
  }
  // Add a final mark to maintain invariants (this will be popped
  // immediately when the first next() is called).
  undos.events.emplace_back(StateMutationUndo::Mark{});
  return PropagatedStates{std::move(state), std::move(undos)};
}

State locally_propagated_bid_state(const Index& index,
                                   const AnalysisContext& ctx,
                                   CollectedInfo& collect,
                                   BlockId bid,
                                   State state,
                                   BlockId targetBid) {
  Trace::Bump bumper{Trace::hhbbc, 10};
  if (!state.initialized) return {};

  auto const originalState = state;
  auto const blk = ctx.func.blocks()[bid].get();
  auto interp = Interp { index, ctx, collect, bid, blk, state };

  State ret{};
  auto const propagate = [&] (BlockId target, const State* st) {
    if (target == targetBid) merge_into(ret, *st);
  };
  run(interp, originalState, propagate);

  ret.stack.compact();
  return ret;
}

//////////////////////////////////////////////////////////////////////

}
