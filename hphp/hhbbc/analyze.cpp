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

namespace HPHP { namespace HHBBC {

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

Type get_type_of_reified_list(const UserAttributeMap& ua) {
  auto const it = ua.find(s___Reified.get());
  assertx(it != ua.end());
  auto const tv = it->second;
  assertx(tvIsVecOrVArray(&tv));
  auto const info = extractSizeAndPosFromReifiedAttribute(tv.m_data.parr);
  auto const numGenerics = info.m_typeParamInfo.size();
  assertx(numGenerics > 0);
  std::vector<Type> types(numGenerics,
                          RuntimeOption::EvalHackArrDVArrs ? TDictN : TDArrN);
  return RO::EvalHackArrDVArrs ? vec(types, ProvTag::Top)
                               : arr_packed_varray(types, ProvTag::Top);
}

State pseudomain_entry_state(const php::Func* func) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisType = TOptObj;
  ret.locals.resize(func->locals.size());
  ret.iters.resize(func->numIters);
  for (auto i = 0; i < ret.locals.size(); ++i) {
    // Named pseudomain locals are bound to $GLOBALS.
    ret.locals[i] = func->locals[i].name ? TCell : TUninit;
  }
  return ret;
}

State entry_state(const Index& index, Context const ctx,
                  const KnownArgs* knownArgs) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisType = [&] {
    if (!ctx.cls) return TNull;
    if (knownArgs && !knownArgs->context.subtypeOf(BBottom)) {
      if (knownArgs->context.subtypeOf(BOptObj)) {
        return setctx(knownArgs->context);
      }
      if (is_specialized_cls(knownArgs->context)) {
        auto const dcls = dcls_of(knownArgs->context);
        return setctx(dcls.type == DCls::Exact ?
                      objExact(dcls.cls) : subObj(dcls.cls));
      }
    }
    auto const maybeThisType = thisType(index, ctx);
    auto const thisType = maybeThisType ? *maybeThisType : TObj;
    if (index.lookup_this_available(ctx.func)) return thisType;
    return opt(thisType);
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
          ret.locals[locId] = RuntimeOption::EvalHackArrDVArrs
            ? vec(std::move(pack), ProvTag::Top)
            : arr_packed_varray(std::move(pack), ProvTag::Top);
        } else {
          ret.locals[locId] = unctx(knownArgs->args[locId]);
        }
      } else {
        ret.locals[locId] = ctx.func->params[locId].isVariadic
          ? (RuntimeOption::EvalHackArrDVArrs ? TVec : TVArr)
          : TUninit;
      }
      continue;
    }
    auto const& param = ctx.func->params[locId];
    if (ctx.func->isMemoizeImpl) {
      auto const& constraint = param.typeConstraint;
      if (constraint.hasConstraint() && !constraint.isTypeVar() &&
          !constraint.isTypeConstant()) {
        ret.locals[locId] =
          loosen_dvarrayness(index.lookup_constraint(ctx, constraint));
        continue;
      }
    }
    // Because we throw a non-recoverable error for having fewer than the
    // required number of args, all function parameters must be initialized.
    ret.locals[locId] = ctx.func->params[locId].isVariadic
      ? (RuntimeOption::EvalHackArrDVArrs ? TVec : TVArr)
      : TInitCell;
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
    assert(!ctx.func->isClosureBody);
    assert(locId < ret.locals.size());
    ret.locals[locId++] = get_type_of_reified_list(ctx.func->userAttributes);
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
                    const KnownArgs* knownArgs) {
  auto incompleteQ     = dataflow_worklist<uint32_t>(ai.rpoBlocks.size());
  auto const ctx       = ai.ctx;
  auto const numParams = ctx.func->params.size();

  auto const entryState = [&] {
    if (!is_pseudomain(ctx.func)) {
      return entry_state(index, ctx, knownArgs);
    }

    assert(!knownArgs);
    assert(numParams == 0);
    return pseudomain_entry_state(ctx.func);
  }();

  if (knownArgs) {
    // When we have known args, we only need to add one of the entry points to
    // the initial state, since we know how many arguments were passed.
    auto const useDvInit = [&] {
      if (knownArgs->args.size() >= numParams) return false;
      for (auto i = knownArgs->args.size(); i < numParams; ++i) {
        auto const dv = ctx.func->params[i].dvEntryPoint;
        if (dv != NoBlockId) {
          ai.bdata[dv].stateIn.copy_from(entryState);
          incompleteQ.push(rpoId(ai, dv));
          return true;
        }
      }
      return false;
    }();

    if (!useDvInit) {
      ai.bdata[ctx.func->mainEntry].stateIn.copy_from(entryState);
      incompleteQ.push(rpoId(ai, ctx.func->mainEntry));
    }

    return incompleteQ;
  }

  for (auto paramId = uint32_t{0}; paramId < numParams; ++paramId) {
    auto const dv = ctx.func->params[paramId].dvEntryPoint;
    if (dv != NoBlockId) {
      ai.bdata[dv].stateIn.copy_from(entryState);
      incompleteQ.push(rpoId(ai, dv));
      for (auto locId = paramId; locId < numParams; ++locId) {
        ai.bdata[dv].stateIn.locals[locId] = ctx.func->params[locId].isVariadic
          ? (RuntimeOption::EvalHackArrDVArrs ? TVec : TVArr)
          : TUninit;
      }
    }
  }

  ai.bdata[ctx.func->mainEntry].stateIn.copy_from(entryState);
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
                                Context const ctx,
                                CollectedInfo& collect,
                                const KnownArgs* knownArgs) {
  assertx(ctx.cls == adjust_closure_context(ctx).cls);
  FuncAnalysis ai{ctx};

  SCOPE_ASSERT_DETAIL("do-analyze-collect-2") {
    std::string ret;
    for (auto bid : ctx.func->blockRange()) {
      folly::format(&ret,
                    "block #{}\nin-{}\n{}",
                    bid,
                    state_string(*ctx.func, ai.bdata[bid].stateIn, collect),
                    show(*ctx.func, *ctx.func->blocks[bid])
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
  auto incompleteQ = prepare_incompleteQ(index, ai, knownArgs);

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
  auto totalVisits = std::vector<uint32_t>(ctx.func->blocks.size());

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

      auto const blk = ctx.func->blocks[bid].get();
      auto stateOut = ai.bdata[bid].stateIn;
      auto interp   = Interp {
        index, ctx, collect, bid, blk, stateOut
      };
      auto flags    = run(interp, ai.bdata[bid].stateIn, propagate);
      if (any(collect.opts & CollectionOpts::EffectFreeOnly) &&
          !collect.effectFree) {
        break;
      }
      if (flags.updateInfo.replacedBcs.size() ||
          flags.updateInfo.unchangedBcs != blk->hhbcs.size() ||
          flags.updateInfo.fallthrough != blk->fallthrough) {
        blockUpdates[bid] = flags.updateInfo;
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
    }

    if (any(collect.opts & CollectionOpts::EffectFreeOnly) &&
        !collect.effectFree) {
      break;
    }
  } while (!incompleteQ.empty());

  ai.closureUseTypes = std::move(collect.closureUseTypes);
  ai.mayUseVV = collect.mayUseVV;
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
  assert(ai.inferredReturn.subtypeOf(BCell));

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
        ai.rpoBlocks[bd.rpoId],
        state_string(*ctx.func, bd.stateIn, collect)
      );
    }
    ret += sep + bsep;
    folly::format(&ret, "Inferred return type: {}\n", show(ai.inferredReturn));
    ret += bsep;
    return ret;
  }());
  return ai;
}

FuncAnalysis do_analyze(const Index& index,
                        Context const inputCtx,
                        ClassAnalysis* clsAnalysis,
                        const KnownArgs* knownArgs,
                        CollectionOpts opts) {
  auto const ctx = adjust_closure_context(inputCtx);
  CollectedInfo collect {
    index, ctx, clsAnalysis, opts
  };

  auto ret = do_analyze_collect(index, ctx, collect, knownArgs);
  if (ctx.func->name == s_86cinit.get() && !knownArgs) {
    // We need to try to resolve any dynamic constants
    size_t idx = 0;
    for (auto const& c : ctx.cls->constants) {
      if (c.val && c.val->m_type == KindOfUninit) {
        auto const fa = analyze_func_inline(index, ctx, TCls, { sval(c.name) });
        if (auto const val = tv(fa.inferredReturn)) {
          ret.resolvedConstants.emplace_back(idx, *val);
        }
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

    /*
     * When any functions are interceptable, we don't require the constraints to
     * actually match, and relax all the HNI types to Gen.
     *
     * This is because with any interceptable functions, it's quite
     * possible that some function calls in systemlib might not be
     * known to return things matching the property type hints for
     * some properties, or not to take their arguments by reference.
     */
    auto const hniTy =
      clsAnalysis.anyInterceptable
        ? TCell
        : from_hni_constraint(prop.userType);
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

FuncAnalysisResult::FuncAnalysisResult(Context ctx)
  : ctx(ctx)
  , inferredReturn(TBottom)
{
}

FuncAnalysis::FuncAnalysis(Context ctx)
  : FuncAnalysisResult{ctx}
  , rpoBlocks{rpoSortAddDVs(*ctx.func)}
  , bdata{ctx.func->blocks.size()}
{
  for (auto rpoId = size_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    bdata[rpoBlocks[rpoId]].rpoId = rpoId;
  }
}

FuncAnalysis analyze_func(const Index& index, Context const ctx,
                          CollectionOpts opts) {
  while (true) {
    auto ret = do_analyze(index, ctx, nullptr, nullptr, opts);
    if (!rebuild_exn_tree(ret)) return ret;
  }
}

FuncAnalysis analyze_func_collect(const Index& index,
                                  Context const ctx,
                                  CollectedInfo& collect) {
  return do_analyze_collect(index, ctx, collect, nullptr);
}

FuncAnalysis analyze_func_inline(const Index& index,
                                 Context const ctx,
                                 const Type& thisType,
                                 const CompactVector<Type>& args,
                                 CollectionOpts opts) {
  assert(!ctx.func->isClosureBody);
  KnownArgs knownArgs { thisType, args };
  return do_analyze(index, ctx, nullptr, &knownArgs,
                    opts | CollectionOpts::Inlining);
}

ClassAnalysis analyze_class(const Index& index, Context const ctx) {

  assertx(ctx.cls && !ctx.func && !is_used_trait(*ctx.cls));

  {
    Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
        is_systemlib_part(*ctx.unit)};
    FTRACE(2, "{:#^70}\n", "Class");
  }

  ClassAnalysis clsAnalysis(ctx, index.any_interceptable_functions());
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

    if (is_closure(*ctx.cls) ||
        (prop.attrs & (AttrSystemInitialValue | AttrLateInit)) ||
        (!cellTy.subtypeOf(TUninit) &&
         index.satisfies_constraint(ctx, cellTy, prop.typeConstraint))) {
      prop.attrs |= AttrInitialSatisfiesTC;
    } else {
      prop.attrs = (Attr)(prop.attrs & ~AttrInitialSatisfiesTC);
      // If Uninit, it will be determined in the 86[s,p]init function.
      if (!cellTy.subtypeOf(TUninit)) clsAnalysis.badPropInitialValues = true;
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
      auto t = loosen_all(cellTy);
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
      } else if (prop.name->isame(s_86reified_prop.get())) {
        t = get_type_of_reified_list(ctx.cls->userAttributes);
      }
      auto& elem = clsAnalysis.privateProperties[prop.name];
      elem.ty = std::move(t);
      elem.tc = &prop.typeConstraint;
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
    }
  }

  /*
   * For classes with non-scalar initializers, the 86pinit, 86sinit, and
   * 86linit methods are guaranteed to run before any other method, and
   * are never called afterwards. Thus, we can analyze these
   * methods first to determine the initial types of properties with
   * non-scalar initializers, and these need not be be run again as part
   * of the fixedpoint computation.
   */
  CompactVector<FuncAnalysis> initResults;
  auto analyze_86init = [&](const StaticString &name) {
    if (auto f = find_method(ctx.cls, name.get())) {
      initResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, f, ctx.cls },
          &clsAnalysis,
          nullptr,
          CollectionOpts{}
        )
      );
    }
  };
  analyze_86init(s_86pinit);
  analyze_86init(s_86sinit);
  analyze_86init(s_86linit);

  /*
   * The 86cinit is a little different from the other two, but
   * similarly can't play a role in the fixed point computation.
   */
  analyze_86init(s_86cinit);

  // NB: Properties can still be TBottom at this point if their initial values
  // cannot possibly satisfy their type-constraints. The classes of such
  // properties cannot be instantiated.

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
  auto totalVisits = uint32_t{0};

  for (;;) {
    auto const previousProps   = clsAnalysis.privateProperties;
    auto const previousStatics = clsAnalysis.privateStatics;

    CompactVector<FuncAnalysis> methodResults;
    CompactVector<FuncAnalysis> closureResults;

    // Analyze every method in the class until we reach a fixed point
    // on the private property states.
    for (auto& f : ctx.cls->methods) {
      if (f->name->isame(s_86pinit.get()) ||
          f->name->isame(s_86sinit.get()) ||
          f->name->isame(s_86linit.get()) ||
          f->name->isame(s_86cinit.get())) {
        continue;
      }

      methodResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, f.get(), ctx.cls },
          &clsAnalysis,
          nullptr,
          CollectionOpts{}
        )
      );
    }

    if (associatedClosures) {
      for (auto const c : *associatedClosures) {
        auto const invoke = c->methods[0].get();
        closureResults.push_back(
          do_analyze(
            index,
            Context { ctx.unit, invoke, c },
            &clsAnalysis,
            nullptr,
            CollectionOpts{}
          )
        );
      }
    }

    if (associatedMethods) {
      for (auto const m : *associatedMethods) {
        // We throw the results of the analysis away. We're only doing
        // this for the effects on the private properties, and the
        // results aren't meaningful outside of this context.
        do_analyze(
          index,
          Context { m->unit, m, ctx.cls },
          &clsAnalysis,
          nullptr,
          CollectionOpts{}
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
      clsAnalysis.methods.reserve(initResults.size() + methodResults.size());
      for (auto& m : initResults) {
        clsAnalysis.methods.push_back(std::move(m));
      }
      for (auto& m : methodResults) {
        clsAnalysis.methods.push_back(std::move(m));
      }
      clsAnalysis.closures.reserve(closureResults.size());
      for (auto& m : closureResults) {
        clsAnalysis.closures.push_back(std::move(m));
      }
      break;
    }

    if (totalVisits++ >= options.analyzeClassWideningLimit) {
      widen_props(clsAnalysis.privateProperties);
      widen_props(clsAnalysis.privateStatics);
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

  return clsAnalysis;
}

//////////////////////////////////////////////////////////////////////

std::vector<std::pair<State,StepFlags>>
locally_propagated_states(const Index& index,
                          const FuncAnalysis& fa,
                          CollectedInfo& collect,
                          BlockId bid,
                          State state) {
  Trace::Bump bumper{Trace::hhbbc, 10};

  std::vector<std::pair<State,StepFlags>> ret;

  auto const blk = fa.ctx.func->blocks[bid].get();
  ret.reserve(blk->hhbcs.size() + 1);

  auto interp = Interp { index, fa.ctx, collect, bid, blk, state };

  for (auto& op : blk->hhbcs) {
    ret.emplace_back(state, StepFlags{});
    ret.back().second = step(interp, op);
    state.stack.compact();
  }

  ret.emplace_back(std::move(state), StepFlags{});
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
