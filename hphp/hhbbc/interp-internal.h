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
#ifndef incl_HPHP_INTERP_INTERNAL_H_
#define incl_HPHP_INTERP_INTERNAL_H_

#include <algorithm>

#include <folly/Optional.h>

#include "hphp/runtime/base/type-string.h"

#include "hphp/hhbbc/class-util.h"
#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

struct LocalRange;

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbbc);

const StaticString s_func_get_args("func_get_args");
const StaticString s_func_get_arg("func_get_arg");
const StaticString s_func_slice_args("__SystemLib\\func_slice_args");

//////////////////////////////////////////////////////////////////////

/*
 * Interpreter Step State.
 *
 * This struct gives interpreter functions access to shared state.  It's not in
 * interp-state.h because it's part of the internal implementation of
 * interpreter routines.  The publicized state as results of interpretation are
 * in that header and interp.h.
 */
struct ISS {
  explicit ISS(Interp& bag,
               StepFlags& flags,
               PropagateFn propagate)
    : index(bag.index)
    , ctx(bag.ctx)
    , collect(bag.collect)
    , bid(bag.bid)
    , blk(*bag.blk)
    , state(bag.state)
    , flags(flags)
    , propagate(propagate)
  {}

  const Index& index;
  const Context ctx;
  CollectedInfo& collect;
  const BlockId bid;
  const php::Block& blk;
  State& state;
  StepFlags& flags;
  PropagateFn propagate;
  bool recordUsedParams{true};
};

void impl_vec(ISS& env, bool reduce, BytecodeVec&& bcs);

//////////////////////////////////////////////////////////////////////

namespace interp_step {

/*
 * An interp_step::in(ISS&, const bc::op&) function exists for every
 * bytecode. Most are defined in interp.cpp, but some (like FCallBuiltin and
 * member instructions) are defined elsewhere.
 */
#define O(opcode, ...) void in(ISS&, const bc::opcode&);
OPCODES
#undef O

}

/*
 * Find a contiguous local range which is equivalent to the given range and has
 * a smaller starting id. Only returns the equivalent first local because the
 * size doesn't change.
 */
LocalId equivLocalRange(ISS& env, const LocalRange& range);

namespace {

Type peekLocRaw(ISS& env, LocalId l);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

/*
 * impl(...)
 *
 * Utility for chaining one bytecode implementation to a series of a few
 * others.  Use reduce() if you also want to enable strength reduction
 * (i.e. the bytecode can be replaced by some other bytecode as an
 * optimization).
 *
 * The chained-to bytecodes should not take branches.  For impl, the
 * canConstProp flag will only be set if it was set for all the
 * bytecodes.
 */

template<class... Ts>
void impl(ISS& env, Ts&&... ts) {
  impl_vec(env, false, { std::forward<Ts>(ts)... });
}

/*
 * Reduce means that (given some situation in the execution state),
 * a given bytecode could be replaced by some other bytecode
 * sequence.  Ensure that if you call reduce(), it is before any
 * state-affecting operations (like popC()).
 *
 * If env.collect.propagate_constants is set, the reduced bytecodes
 * will have been constant-propagated, and the canConstProp flag will
 * be clear; otherwise canConstProp will be set as for impl.
 */
void reduce(ISS& env, BytecodeVec&& bcs) {
  impl_vec(env, true, std::move(bcs));
}

template<class... Bytecodes>
void reduce(ISS& env, Bytecodes&&... hhbc) {
  reduce(env, { std::forward<Bytecodes>(hhbc)... });
}

void nothrow(ISS& env) {
  FTRACE(2, "    nothrow\n");
  env.flags.wasPEI = false;
}

void unreachable(ISS& env) {
  FTRACE(2, "    unreachable\n");
  env.state.unreachable = true;
}

void constprop(ISS& env) {
  FTRACE(2, "    constprop\n");
  env.flags.canConstProp = true;
}

void effect_free(ISS& env) {
  FTRACE(2, "    effect_free\n");
  nothrow(env);
  env.flags.effectFree = true;
}

/*
 * Mark the current block as unconditionally jumping to target. The
 * caller must arrange for env.state to reflect the state that needs
 * to be propagated to the target, but it should not propagate that
 * state.
 */
void jmp_setdest(ISS& env, BlockId target) {
  env.flags.jmpDest = target;
}
void jmp_nevertaken(ISS& env) {
  jmp_setdest(env, env.blk.fallthrough);
}

struct IgnoreUsedParams {
  explicit IgnoreUsedParams(ISS& env) :
      env{env}, record{env.recordUsedParams} {
    env.recordUsedParams = false;
  }

  ~IgnoreUsedParams() {
    env.recordUsedParams = record;
  }

  ISS& env;
  const bool record;
};

void readUnknownParams(ISS& env) {
  for (LocalId p = 0; p < env.ctx.func->params.size(); p++) {
    if (p == env.flags.mayReadLocalSet.size()) break;
    env.flags.mayReadLocalSet.set(p);
  }
  if (env.recordUsedParams) env.collect.usedParams.set();
}

void readUnknownLocals(ISS& env) {
  env.flags.mayReadLocalSet.set();
  if (env.recordUsedParams) env.collect.usedParams.set();
}

void readAllLocals(ISS& env) {
  env.flags.mayReadLocalSet.set();
  if (env.recordUsedParams) env.collect.usedParams.set();
}

void modifyLocalStatic(ISS& env, LocalId id, const Type& t) {
  auto modifyOne = [&] (LocalId lid) {
    if (is_volatile_local(env.ctx.func, lid)) return;
    if (env.state.localStaticBindings.size() <= lid) return;
    if (env.state.localStaticBindings[lid] == LocalStaticBinding::None) return;
    if (t.subtypeOf(BUninit) && !t.subtypeOf(BBottom)) {
      // Uninit means we are unbinding.
      env.state.localStaticBindings[lid] = id == NoLocalId ?
        LocalStaticBinding::None : LocalStaticBinding::Maybe;
      return;
    }
    if (lid >= env.collect.localStaticTypes.size()) {
      env.collect.localStaticTypes.resize(lid + 1, TBottom);
    }
    env.collect.localStaticTypes[lid] = t.subtypeOf(BCell) ?
      union_of(std::move(env.collect.localStaticTypes[lid]), t) :
      TGen;
  };
  if (id != NoLocalId) {
    return modifyOne(id);
  }
  for (LocalId i = 0; i < env.state.localStaticBindings.size(); i++) {
    modifyOne(i);
  }
}

void maybeBindLocalStatic(ISS& env, LocalId id) {
  if (is_volatile_local(env.ctx.func, id)) return;
  if (env.state.localStaticBindings.size() <= id) return;
  if (env.state.localStaticBindings[id] != LocalStaticBinding::None) return;
  env.state.localStaticBindings[id] = LocalStaticBinding::Maybe;
  return;
}

void unbindLocalStatic(ISS& env, LocalId id) {
  modifyLocalStatic(env, id, TUninit);
}

void bindLocalStatic(ISS& env, LocalId id, const Type& t) {
  if (is_volatile_local(env.ctx.func, id)) return;
  if (env.state.localStaticBindings.size() <= id) {
    env.state.localStaticBindings.resize(id + 1);
  }
  env.state.localStaticBindings[id] = LocalStaticBinding::Bound;
  modifyLocalStatic(env, id, t);
}

void doRet(ISS& env, Type t, bool hasEffects) {
  IgnoreUsedParams _{env};

  readAllLocals(env);
  assert(env.state.stack.empty());
  env.flags.retParam = NoLocalId;
  env.flags.returned = t;
  if (!hasEffects) {
    effect_free(env);
  }
}

void mayUseVV(ISS& env) {
  env.collect.mayUseVV = true;
}

void hasInvariantIterBase(ISS& env) {
  env.collect.hasInvariantIterBase = true;
}

//////////////////////////////////////////////////////////////////////
// eval stack

Type popT(ISS& env) {
  assert(!env.state.stack.empty());
  auto const ret = std::move(env.state.stack.back().type);
  FTRACE(2, "    pop:  {}\n", show(ret));
  assert(ret.subtypeOf(BGen));
  env.state.stack.pop_back();
  return ret;
}

Type popC(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(BInitCell));
  return v;
}

Type popV(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(BRef));
  return v;
}

Type popU(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(BUninit));
  return v;
}

Type popCU(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(BCell));
  return v;
}

Type popCV(ISS& env) { return popT(env); }

void discard(ISS& env, int n) {
  for (auto i = 0; i < n; ++i) {
    popT(env);
  }
}

Type& topT(ISS& env, uint32_t idx = 0) {
  assert(idx < env.state.stack.size());
  return env.state.stack[env.state.stack.size() - idx - 1].type;
}

Type& topC(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(BInitCell));
  return topT(env, i);
}

Type& topCV(ISS& env, uint32_t i = 0) { return topT(env, i); }

Type& topV(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(BRef));
  return topT(env, i);
}

void push(ISS& env, Type t) {
  FTRACE(2, "    push: {}\n", show(t));
  env.state.stack.push_back(StackElem {std::move(t), NoLocalId});
}

void push(ISS& env, Type t, LocalId l) {
  if (l == NoLocalId) return push(env, t);
  if (l <= MaxLocalId) {
    if (peekLocRaw(env, l).couldBe(BRef)) {
      return push(env, t);
    }
    assertx(!is_volatile_local(env.ctx.func, l)); // volatiles are TGen
  }
  FTRACE(2, "    push: {} (={})\n", show(t), local_string(*env.ctx.func, l));
  env.state.stack.push_back(StackElem {std::move(t), l});
}

//////////////////////////////////////////////////////////////////////
// $this

void setThisAvailable(ISS& env) {
  FTRACE(2, "    setThisAvailable\n");
  if (env.ctx.cls ?
      is_unused_trait(*env.ctx.cls) || (env.ctx.func->attrs & AttrStatic) :
      !is_pseudomain(env.ctx.func)) {
    return unreachable(env);
  }
  if (!env.state.thisType.couldBe(BObj) ||
      !env.state.thisType.subtypeOf(BOptObj)) {
    return unreachable(env);
  }
  if (is_opt(env.state.thisType)) {
    env.state.thisType = unopt(env.state.thisType);
  }
}

bool thisAvailable(ISS& env) {
  assertx(!env.state.thisType.subtypeOf(BBottom));
  return env.state.thisType.subtypeOf(BObj);
}

// Returns the type $this would have if it's not null.  Generally
// you have to check thisAvailable() before assuming it can't be
// null.
folly::Optional<Type> thisTypeFromContext(const Index& index, Context ctx) {
  // Due to `bindTo`, we can't conclude the type of $this.
  if (RuntimeOption::EvalAllowScopeBinding && ctx.func->isClosureBody) {
    return folly::none;
  }

  if (auto rcls = index.selfCls(ctx)) return setctx(subObj(*rcls));
  return folly::none;
}

folly::Optional<Type> thisType(ISS& env) {
  if (!is_specialized_obj(env.state.thisType)) return folly::none;
  return is_opt(env.state.thisType) ?
    unopt(env.state.thisType) : env.state.thisType;
}

folly::Optional<Type> selfCls(ISS& env) {
  if (auto rcls = env.index.selfCls(env.ctx)) return subCls(*rcls);
  return folly::none;
}

folly::Optional<Type> selfClsExact(ISS& env) {
  if (auto rcls = env.index.selfCls(env.ctx)) return clsExact(*rcls);
  return folly::none;
}

folly::Optional<Type> parentCls(ISS& env) {
  if (auto rcls = env.index.parentCls(env.ctx)) return subCls(*rcls);
  return folly::none;
}

folly::Optional<Type> parentClsExact(ISS& env) {
  if (auto rcls = env.index.parentCls(env.ctx)) return clsExact(*rcls);
  return folly::none;
}

//////////////////////////////////////////////////////////////////////
// fpi

/*
 * Push an ActRec.
 *
 * nArgs should either be the number of parameters that will be passed
 * to the call, or -1 for unknown. We only need the number of args
 * when we know the exact function being called, in order to determine
 * eligibility for folding.
 *
 * returns the foldable flag as a convenience.
 */
bool fpiPush(ISS& env, ActRec ar, int32_t nArgs, bool maybeDynamic) {
  auto foldable = [&] {
    if (!options.ConstantFoldBuiltins) return false;
    if (any(env.collect.opts & CollectionOpts::Speculating)) return false;
    if (!env.collect.propagate_constants &&
        any(env.collect.opts & CollectionOpts::Optimizing)) {
      // we're in the optimization phase, but we're not folding constants
      return false;
    }
    if (nArgs < 0 ||
        ar.kind == FPIKind::Ctor ||
        ar.kind == FPIKind::Builtin ||
        !ar.func || ar.fallbackFunc) {
      return false;
    }
    if (maybeDynamic && ar.func->mightCareAboutDynCalls()) return false;
    // Reified functions may have a mismatch of arity or reified generics
    // so we cannot fold them
    // TODO(T31677864): Detect the arity mismatch at HHBBC and enable them to
    // be foldable
    if (ar.func->couldHaveReifiedGenerics()) return false;
    auto const func = ar.func->exactFunc();
    if (!func) return false;
    if (func->attrs & AttrTakesInOutParams) return false;
    if (env.collect.unfoldableFuncs.count(std::make_pair(func, env.bid))) {
      return false;
    }
    // Foldable builtins are always worth trying
    if (ar.func->isFoldable()) return true;
    // Any native functions at this point are known to be
    // non-foldable, but other builtins might be, even if they
    // don't have the __Foldable attribute.
    if (func->nativeInfo) return false;

    // Don't try to fold functions which aren't guaranteed to be accessible at
    // this call site.
    if (func->attrs & AttrPrivate) {
      if (env.ctx.cls != func->cls) return false;
    } else if (func->attrs & AttrProtected) {
      if (!env.ctx.cls) return false;
      if (!env.index.must_be_derived_from(env.ctx.cls, func->cls) &&
          !env.index.must_be_derived_from(func->cls, env.ctx.cls)) return false;
    }

    if (func->params.size()) {
      // Not worth trying if we're going to warn due to missing args
      return check_nargs_in_range(func, nArgs);
    }

    auto has_better_this = [&] {
      if (!is_specialized_obj(env.state.thisType)) return false;
      auto const dobj = dobj_of(env.state.thisType);
      return dobj.type == DObj::Exact || dobj.cls.cls() != func->cls;
    };

    if (!(func->attrs & AttrStatic) && func->cls) {
      return thisAvailable(env) &&
        (has_better_this() ||
         is_scalar(env.index.lookup_return_type_raw(func)));
    }

    // The function has no args. Just check that it's effect free
    // and returns a literal.
    return env.index.is_effect_free(*ar.func) &&
      is_scalar(env.index.lookup_return_type_raw(func));
  }();
  if (foldable) effect_free(env);
  ar.foldable = foldable;
  ar.pushBlk = env.bid;

  FTRACE(2, "    fpi+: {}\n", show(ar));
  env.state.fpiStack.push_back(std::move(ar));
  return foldable;
}

void fpiPushNoFold(ISS& env, ActRec ar) {
  ar.foldable = false;
  ar.pushBlk = env.bid;

  FTRACE(2, "    fpi+: {}\n", show(ar));
  env.state.fpiStack.push_back(std::move(ar));
}

ActRec fpiPop(ISS& env) {
  assert(!env.state.fpiStack.empty());
  auto const ret = env.state.fpiStack.back();
  FTRACE(2, "    fpi-: {}\n", show(ret));
  env.state.fpiStack.pop_back();
  return ret;
}

ActRec& fpiTop(ISS& env) {
  assert(!env.state.fpiStack.empty());
  return env.state.fpiStack.back();
}

void fpiNotFoldable(ISS& env) {
  // By the time we're optimizing, we should know up front which funcs
  // are foldable (the analyze phase iterates to convergence, the
  // optimize phase does not - so its too late to fix now).
  assertx(!any(env.collect.opts & CollectionOpts::Optimizing));

  auto& ar = fpiTop(env);
  assertx(ar.func && ar.foldable);
  auto const func = ar.func->exactFunc();
  assertx(func);
  env.collect.unfoldableFuncs.emplace(func, ar.pushBlk);
  env.propagate(ar.pushBlk, nullptr);
  ar.foldable = false;
  // we're going to reprocess the whole fpi region; any results we've
  // got so far are bogus, so stop prevent further useless work by
  // marking the next bytecode unreachable
  unreachable(env);
  FTRACE(2, "     fpi: not foldable\n");
}

//////////////////////////////////////////////////////////////////////
// locals

void useLocalStatic(ISS& env, LocalId l) {
  assert(env.collect.localStaticTypes.size() > l);
  if (!env.flags.usedLocalStatics) {
    env.flags.usedLocalStatics =
      std::make_shared<hphp_fast_map<LocalId,Type>>();
  }
  // Ignore the return value, since we only want the first type used,
  // as that will be the narrowest.
  env.flags.usedLocalStatics->emplace(l, env.collect.localStaticTypes[l]);
}

void mayReadLocal(ISS& env, uint32_t id) {
  if (id < env.flags.mayReadLocalSet.size()) {
    env.flags.mayReadLocalSet.set(id);
  }
  if (env.recordUsedParams && id < env.collect.usedParams.size()) {
    env.collect.usedParams.set(id);
  }
}

// Find a local which is equivalent to the given local
LocalId findLocEquiv(ISS& env, LocalId l) {
  if (l >= env.state.equivLocals.size()) return NoLocalId;
  assert(env.state.equivLocals[l] == NoLocalId ||
         !is_volatile_local(env.ctx.func, l));
  return env.state.equivLocals[l];
}

// Determine whether two locals are equivalent
bool locsAreEquiv(ISS& env, LocalId l1, LocalId l2) {
  if (l1 >= env.state.equivLocals.size() ||
      l2 >= env.state.equivLocals.size() ||
      env.state.equivLocals[l1] == NoLocalId ||
      env.state.equivLocals[l2] == NoLocalId) {
    return false;
  }

  auto l = l1;
  while ((l = env.state.equivLocals[l]) != l1) {
    if (l == l2) return true;
  }
  return false;
}

bool locIsThis(ISS& env, LocalId l) {
  assertx(l <= MaxLocalId);
  return l == env.state.thisLoc ||
    (env.state.thisLoc <= MaxLocalId &&
     locsAreEquiv(env, l, env.state.thisLoc));
}

void killLocEquiv(State& state, LocalId l) {
  if (l >= state.equivLocals.size()) return;
  if (state.equivLocals[l] == NoLocalId) return;
  auto loc = l;
  do {
    loc = state.equivLocals[loc];
  } while (state.equivLocals[loc] != l);
  assert(loc != l);
  if (state.equivLocals[l] == loc) {
    state.equivLocals[loc] = NoLocalId;
  } else {
    state.equivLocals[loc] = state.equivLocals[l];
  }
  state.equivLocals[l] = NoLocalId;
}

void killLocEquiv(ISS& env, LocalId l) {
  killLocEquiv(env.state, l);
}

void killAllLocEquiv(ISS& env) {
  env.state.equivLocals.clear();
}

// Add from to to's equivalency set.
void addLocEquiv(ISS& env,
                 LocalId from,
                 LocalId to) {
  always_assert(peekLocRaw(env, from).subtypeOf(BCell));
  always_assert(!is_volatile_local(env.ctx.func, to));
  always_assert(from != to && findLocEquiv(env, from) == NoLocalId);

  auto m = std::max(to, from);
  if (env.state.equivLocals.size() <= m) {
    env.state.equivLocals.resize(m + 1, NoLocalId);
  }

  if (env.state.equivLocals[to] == NoLocalId) {
    env.state.equivLocals[from] = to;
    env.state.equivLocals[to] = from;
  } else {
    env.state.equivLocals[from] = env.state.equivLocals[to];
    env.state.equivLocals[to] = from;
  }
}

// Obtain a local which is equivalent to the given stack value
LocalId topStkLocal(const State& state, uint32_t idx = 0) {
  assert(idx < state.stack.size());
  auto const equiv = state.stack[state.stack.size() - idx - 1].equivLoc;
  return equiv > MaxLocalId ? NoLocalId : equiv;
}
LocalId topStkLocal(ISS& env, uint32_t idx = 0) {
  return topStkLocal(env.state, idx);
}

// Obtain a location which is equivalent to the given stack value
LocalId topStkEquiv(ISS& env, uint32_t idx = 0) {
  assert(idx < env.state.stack.size());
  return env.state.stack[env.state.stack.size() - idx - 1].equivLoc;
}

void setStkLocal(ISS& env, LocalId loc, uint32_t idx = 0) {
  assertx(loc <= MaxLocalId);
  always_assert(peekLocRaw(env, loc).subtypeOf(BCell));
  auto const equiv = [&] {
    while (true) {
      auto const e = topStkEquiv(env, idx);
      if (e != StackDupId) return e;
      idx++;
    }
  }();

  if (equiv <= MaxLocalId) {
    if (loc == equiv || locsAreEquiv(env, loc, equiv)) return;
    addLocEquiv(env, loc, equiv);
    return;
  }
  env.state.stack[env.state.stack.size() - idx - 1].equivLoc = loc;
}

void killThisLoc(ISS& env, LocalId l) {
  if (l != NoLocalId ?
      env.state.thisLoc == l : env.state.thisLoc != NoLocalId) {
    FTRACE(2, "Killing thisLoc: {}\n", env.state.thisLoc);
    env.state.thisLoc = NoLocalId;
  }
}

// Kill all equivalencies involving the given local to stack values
void killStkEquiv(ISS& env, LocalId l) {
  for (auto& e : env.state.stack) {
    if (e.equivLoc != l) continue;
    e.equivLoc = findLocEquiv(env, l);
    assertx(e.equivLoc != l);
  }
}

void killAllStkEquiv(ISS& env) {
  for (auto& e : env.state.stack) {
    if (e.equivLoc <= MaxLocalId) e.equivLoc = NoLocalId;
  }
}

void killIterEquivs(ISS& env, LocalId l, LocalId key = NoLocalId) {
  for (auto& i : env.state.iters) {
    match<void>(
      i,
      []  (DeadIter) {},
      [&] (LiveIter& iter) {
        if (iter.keyLocal == l) iter.keyLocal = NoLocalId;
        if (iter.baseLocal == l) {
          if (key == NoLocalId || key != iter.keyLocal) {
            iter.baseLocal = NoLocalId;
          }
        }
      }
    );
  }
}

void killAllIterEquivs(ISS& env) {
  for (auto& i : env.state.iters) {
    match<void>(
      i,
      [] (DeadIter) {},
      [] (LiveIter& iter) {
        iter.baseLocal = NoLocalId;
        iter.keyLocal = NoLocalId;
      }
    );
  }
}

void setIterKey(ISS& env, IterId id, LocalId key) {
  match<void>(
    env.state.iters[id],
    []  (DeadIter) {},
    [&] (LiveIter& iter) { iter.keyLocal = key; }
  );
}

Type peekLocRaw(ISS& env, LocalId l) {
  auto ret = env.state.locals[l];
  if (is_volatile_local(env.ctx.func, l)) {
    always_assert_flog(ret == TGen, "volatile local was not TGen");
  }
  return ret;
}

Type locRaw(ISS& env, LocalId l) {
  mayReadLocal(env, l);
  return peekLocRaw(env, l);
}

void setLocRaw(ISS& env, LocalId l, Type t) {
  mayReadLocal(env, l);
  killStkEquiv(env, l);
  killLocEquiv(env, l);
  killIterEquivs(env, l);
  killThisLoc(env, l);
  if (is_volatile_local(env.ctx.func, l)) {
    auto current = env.state.locals[l];
    always_assert_flog(current == TGen, "volatile local was not TGen");
    return;
  }
  modifyLocalStatic(env, l, t);
  env.state.locals[l] = std::move(t);
}

folly::Optional<Type> staticLocType(ISS& env, LocalId l, const Type& super) {
  mayReadLocal(env, l);
  if (env.state.localStaticBindings.size() > l &&
      env.state.localStaticBindings[l] == LocalStaticBinding::Bound) {
    assert(env.collect.localStaticTypes.size() > l);
    auto t = env.collect.localStaticTypes[l];
    if (t.subtypeOf(super)) {
      useLocalStatic(env, l);
      if (t.subtypeOf(BBottom)) t = TInitNull;
      return std::move(t);
    }
  }
  return folly::none;
}

// Read a local type in the sense of CGetL.  (TUninits turn into
// TInitNull, and potentially reffy types return the "inner" type,
// which is always a subtype of InitCell.)
Type locAsCell(ISS& env, LocalId l) {
  if (auto s = staticLocType(env, l, TInitCell)) {
    return std::move(*s);
  }
  return to_cell(locRaw(env, l));
}

// Read a local type, dereferencing refs, but without converting
// potential TUninits to TInitNull.
Type derefLoc(ISS& env, LocalId l) {
  if (auto s = staticLocType(env, l, TCell)) {
    return std::move(*s);
  }
  auto v = locRaw(env, l);
  if (v.subtypeOf(BCell)) return v;
  return v.couldBe(BUninit) ? TCell : TInitCell;
}

bool locCouldBeUninit(ISS& env, LocalId l) {
  return locRaw(env, l).couldBe(BUninit);
}

bool locCouldBeRef(ISS& env, LocalId l) {
  return locRaw(env, l).couldBe(BRef);
}

/*
 * Update the known type of a local, based on assertions
 * (VerifyParamType; or IsType/JmpCC), rather than an actual
 * modification to the local.
 */
void refineLocHelper(ISS& env, LocalId l, Type t) {
  auto v = peekLocRaw(env, l);
  if (v.subtypeOf(BCell)) env.state.locals[l] = std::move(t);
}

template<typename F>
bool refineLocation(ISS& env, LocalId l, F fun) {
  bool ok = true;
  auto refine = [&] (Type t) {
    always_assert(t.subtypeOf(BCell));
    auto r1 = fun(t);
    auto r2 = intersection_of(r1, t);
    // In unusual edge cases (mainly intersection of two unrelated
    // interfaces) the intersection may not be a subtype of its inputs.
    // In that case, always choose fun's type.
    if (r2.subtypeOf(r1)) {
      if (r2.subtypeOf(BBottom)) ok = false;
      return r2;
    }
    if (r1.subtypeOf(BBottom)) ok = false;
    return r1;
  };
  if (l == StackDupId) {
    auto stk = &env.state.stack.back();
    while (true) {
      stk->type = refine(std::move(stk->type));
      if (stk->equivLoc != StackDupId) break;
      assertx(stk != &env.state.stack.front());
      --stk;
    }
    l = stk->equivLoc;
  }
  if (l == StackThisId) {
    if (env.state.thisLoc != NoLocalId) {
      l = env.state.thisLoc;
    }
  }
  if (l > MaxLocalId) return ok;
  auto equiv = findLocEquiv(env, l);
  if (equiv != NoLocalId) {
    do {
      refineLocHelper(env, equiv, refine(peekLocRaw(env, equiv)));
      equiv = findLocEquiv(env, equiv);
    } while (equiv != l);
  }
  refineLocHelper(env, l, refine(peekLocRaw(env, l)));
  return ok;
}

template<typename PreFun, typename PostFun>
void refineLocation(ISS& env, LocalId l,
                    PreFun pre, BlockId target, PostFun post) {
  auto state = env.state;
  auto const target_reachable = refineLocation(env, l, pre);
  if (!target_reachable) jmp_nevertaken(env);
  // swap, so we can restore this state if the branch is always taken.
  std::swap(env.state, state);
  if (!refineLocation(env, l, post)) {
    jmp_setdest(env, target);
    env.state = std::move(state);
  } else if (target_reachable) {
    env.propagate(target, &state);
  }
}

/*
 * Set a local type in the sense of tvSet.  If the local is boxed or
 * not known to be not boxed, we can't change the type.  May be used
 * to set locals to types that include Uninit.
 */
void setLoc(ISS& env, LocalId l, Type t, LocalId key = NoLocalId) {
  killStkEquiv(env, l);
  killLocEquiv(env, l);
  killIterEquivs(env, l, key);
  killThisLoc(env, l);
  modifyLocalStatic(env, l, t);
  mayReadLocal(env, l);
  refineLocHelper(env, l, std::move(t));
}

LocalId findLocal(ISS& env, SString name) {
  for (auto& l : env.ctx.func->locals) {
    if (l.name->same(name)) {
      mayReadLocal(env, l.id);
      return l.id;
    }
  }
  return NoLocalId;
}

// Force non-ref locals to TCell.  Used when something modifies an
// unknown local's value, without changing reffiness.
void loseNonRefLocalTypes(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "    loseNonRefLocalTypes\n");
  for (auto& l : env.state.locals) {
    if (l.subtypeOf(BCell)) l = TCell;
  }
  killAllLocEquiv(env);
  killAllStkEquiv(env);
  killAllIterEquivs(env);
  killThisLoc(env, NoLocalId);
  modifyLocalStatic(env, NoLocalId, TCell);
}

void killLocals(ISS& env) {
  FTRACE(2, "    killLocals\n");
  readUnknownLocals(env);
  modifyLocalStatic(env, NoLocalId, TGen);
  for (auto& l : env.state.locals) l = TGen;
  killAllLocEquiv(env);
  killAllStkEquiv(env);
  killAllIterEquivs(env);
  killThisLoc(env, NoLocalId);
}

//////////////////////////////////////////////////////////////////////
// Special functions

void specialFunctionEffects(ISS& env, const res::Func& func) {
  /*
   * Skip-frame functions won't read from the caller's frame, but they might
   * dynamically call a function which can. So, skip-frame functions read our
   * locals unless they can't call such functions.
   */
  if ((RuntimeOption::DisallowDynamicVarEnvFuncs != HackStrictOption::ON &&
       func.mightBeSkipFrame())) {
    readUnknownLocals(env);
  }

  if (func.mightReadCallerFrame()) {
    if (func.name()->isame(s_func_get_args.get()) ||
        func.name()->isame(s_func_get_arg.get()) ||
        func.name()->isame(s_func_slice_args.get())) {
      readUnknownParams(env);
    } else {
      readUnknownLocals(env);
    }
    mayUseVV(env);
  }
}

void specialFunctionEffects(ISS& env, ActRec ar) {
  switch (ar.kind) {
  case FPIKind::Unknown:
    // fallthrough
  case FPIKind::Func:
    if (!ar.func) {
      if (RuntimeOption::DisallowDynamicVarEnvFuncs != HackStrictOption::ON) {
        readUnknownLocals(env);
      }
      return;
    }
  case FPIKind::Builtin:
    specialFunctionEffects(env, *ar.func);
    if (ar.fallbackFunc) specialFunctionEffects(env, *ar.fallbackFunc);
    break;
  case FPIKind::Ctor:
  case FPIKind::ObjMeth:
  case FPIKind::ObjMethNS:
  case FPIKind::ClsMeth:
  case FPIKind::ObjInvoke:
  case FPIKind::CallableArr:
    /*
     * Methods cannot read or write to the caller's frame, but they can be
     * skip-frame (if they're a builtin). So, its possible they'll dynamically
     * call a function which reads from the caller's frame. If we don't
     * forbid this, we have to be pessimistic. Imagine something like
     * Vector::map calling assert.
     */
    if (RuntimeOption::DisallowDynamicVarEnvFuncs != HackStrictOption::ON &&
        (!ar.func || ar.func->mightBeSkipFrame())) {
      readUnknownLocals(env);
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////
// class-ref slots

// Read the specified class-ref slot without discarding the stored value.
const Type& peekClsRefSlot(ISS& env, ClsRefSlotId slot) {
  assert(slot != NoClsRefSlotId);
  always_assert_flog(env.state.clsRefSlots[slot].subtypeOf(BCls),
                     "class-ref slot contained non-TCls");
  return env.state.clsRefSlots[slot];
}

// Read the specified class-ref slot and discard the stored value.
Type takeClsRefSlot(ISS& env, ClsRefSlotId slot) {
  assert(slot != NoClsRefSlotId);
  auto ret = std::move(env.state.clsRefSlots[slot]);
  FTRACE(2, "    read class-ref: {} -> {}\n", slot, show(ret));
  always_assert_flog(ret.subtypeOf(BCls), "class-ref slot contained non-TCls");
  env.state.clsRefSlots[slot] = TCls;
  return ret;
}

void putClsRefSlot(ISS& env, ClsRefSlotId slot, Type ty) {
  assert(slot != NoClsRefSlotId);
  always_assert_flog(ty.subtypeOf(BCls),
                     "attempted to set class-ref slot to non-TCls");
  FTRACE(2, "    write class-ref: {} -> {}\n", slot, show(ty));
  env.state.clsRefSlots[slot] = std::move(ty);
}

//////////////////////////////////////////////////////////////////////
// iterators

void setIter(ISS& env, IterId iter, Iter iterState) {
  env.state.iters[iter] = std::move(iterState);
}
void freeIter(ISS& env, IterId iter) {
  env.state.iters[iter] = DeadIter {};
}

bool iterIsDead(ISS& env, IterId iter) {
  return match<bool>(
    env.state.iters[iter],
    [] (DeadIter) { return true; },
    [] (const LiveIter&) { return false; }
  );
}

//////////////////////////////////////////////////////////////////////
// properties on $this

/*
 * Note: we are only tracking control-flow insensitive types for
 * object properties, because it can be pretty rough to try to track
 * all cases that could re-enter the VM, run arbitrary code, and
 * potentially change the type of a property.
 *
 * Because of this, the various "setter" functions for thisProps
 * here actually just union the new type into what we already had.
 */

PropStateElem<>* thisPropRaw(ISS& env, SString name) {
  auto& privateProperties = env.collect.props.privateProperties();
  auto const it = privateProperties.find(name);
  if (it != end(privateProperties)) {
    return &it->second;
  }
  return nullptr;
}

bool isTrackedThisProp(ISS& env, SString name) {
  return thisPropRaw(env, name);
}

bool isMaybeLateInitThisProp(ISS& env, SString name) {
  if (!env.ctx.cls) return false;
  for (auto const& prop : env.ctx.cls->properties) {
    if (prop.name == name &&
        (prop.attrs & AttrPrivate) &&
        !(prop.attrs & AttrStatic)
       ) {
      return prop.attrs & AttrLateInit;
    }
  }
  // Prop either doesn't exist, or is on an unflattened trait. Be conservative.
  return true;
}

void killThisProps(ISS& env) {
  FTRACE(2, "    killThisProps\n");
  for (auto& kv : env.collect.props.privateProperties()) {
    kv.second.ty |=
      adjust_type_for_prop(env.index, *env.ctx.cls, kv.second.tc, TGen);
  }
}

/*
 * This function returns a type that includes all the possible types
 * that could result from reading a property $this->name.
 *
 * Note that this may include types that the property itself cannot
 * actually contain, due to the effects of a possible __get function.
 */
folly::Optional<Type> thisPropAsCell(ISS& env, SString name) {
  auto const elem = thisPropRaw(env, name);
  if (!elem) return folly::none;
  if (elem->ty.couldBe(BUninit)) {
    auto const rthis = thisType(env);
    if (!rthis || dobj_of(*rthis).cls.couldHaveMagicGet()) {
      return TInitCell;
    }
  }
  return to_cell(elem->ty);
}

/*
 * Merge a type into the tracked property types on $this, in the sense
 * of tvSet (i.e. setting the inner type on possible refs).
 *
 * Note that all types we see that could go into an object property have to
 * loosen_all.  This is because the object could be serialized and then
 * deserialized, losing the static-ness of a string or array member, and we
 * don't guarantee deserialization would preserve a constant value object
 * property type.
 */
void mergeThisProp(ISS& env, SString name, Type type) {
  auto const elem = thisPropRaw(env, name);
  if (!elem) return;
  auto const adjusted =
    adjust_type_for_prop(env.index, *env.ctx.cls, elem->tc, loosen_all(type));
  elem->ty |= adjusted;
}

/*
 * Merge something into each this prop.  Usually MapFn will be a
 * predicate that returns TBottom when some condition doesn't hold.
 *
 * The types given to the map function are the raw tracked types
 * (i.e. could be TRef or TUninit).
 */
template<class MapFn>
void mergeEachThisPropRaw(ISS& env, MapFn fn) {
  for (auto& kv : env.collect.props.privateProperties()) {
    mergeThisProp(env, kv.first, fn(kv.second.ty));
  }
}

void unsetThisProp(ISS& env, SString name) {
  mergeThisProp(env, name, TUninit);
}

void unsetUnknownThisProp(ISS& env) {
  for (auto& kv : env.collect.props.privateProperties()) {
    mergeThisProp(env, kv.first, TUninit);
  }
}

void boxThisProp(ISS& env, SString name) {
  auto const elem = thisPropRaw(env, name);
  if (!elem) return;
  elem->ty |=
    adjust_type_for_prop(env.index, *env.ctx.cls, elem->tc, TRef);
}

/*
 * Forces non-ref property types up to TCell.  This is used when an
 * operation affects an unknown property on $this, but can't change
 * its reffiness.  This could only do TInitCell, but we're just
 * going to gradually get rid of the callsites of this.
 */
void loseNonRefThisPropTypes(ISS& env) {
  FTRACE(2, "    loseNonRefThisPropTypes\n");
  for (auto& kv : env.collect.props.privateProperties()) {
    if (kv.second.ty.subtypeOf(BCell)) {
      kv.second.ty |=
        adjust_type_for_prop(env.index, *env.ctx.cls, kv.second.tc, TCell);
    }
  }
}

//////////////////////////////////////////////////////////////////////
// properties on self::

// Similar to $this properties above, we only track control-flow
// insensitive types for these.

PropStateElem<>* selfPropRaw(ISS& env, SString name) {
  auto& privateStatics = env.collect.props.privateStatics();
  auto it = privateStatics.find(name);
  if (it != end(privateStatics)) {
    return &it->second;
  }
  return nullptr;
}

void killSelfProps(ISS& env) {
  FTRACE(2, "    killSelfProps\n");
  for (auto& kv : env.collect.props.privateStatics()) {
    kv.second.ty |=
      adjust_type_for_prop(env.index, *env.ctx.cls, kv.second.tc, TGen);
  }
}

void killSelfProp(ISS& env, SString name) {
  FTRACE(2, "    killSelfProp {}\n", name->data());
  if (auto elem = selfPropRaw(env, name)) {
    elem->ty |= adjust_type_for_prop(env.index, *env.ctx.cls, elem->tc, TGen);
  }
}

// TODO(#3684136): self::$foo can't actually ever be uninit.  Right
// now uninits may find their way into here though.
folly::Optional<Type> selfPropAsCell(ISS& env, SString name) {
  auto const elem = selfPropRaw(env, name);
  if (!elem) return folly::none;
  return to_cell(elem->ty);
}

/*
 * Merges a type into tracked static properties on self, in the
 * sense of tvSet (i.e. setting the inner type on possible refs).
 */
void mergeSelfProp(ISS& env, SString name, Type type) {
  auto const elem = selfPropRaw(env, name);
  if (!elem) return;
  // Context types might escape to other contexts here.
  auto const adjusted =
    adjust_type_for_prop(env.index, *env.ctx.cls, elem->tc, unctx(type));
  elem->ty |= adjusted;
}

/*
 * Similar to mergeEachThisPropRaw, but for self props.
 */
template<class MapFn>
void mergeEachSelfPropRaw(ISS& env, MapFn fn) {
  for (auto& kv : env.collect.props.privateStatics()) {
    mergeSelfProp(env, kv.first, fn(kv.second.ty));
  }
}

void boxSelfProp(ISS& env, SString name) {
  mergeSelfProp(env, name, TRef);
}

/*
 * Forces non-ref static properties up to TCell.  This is used when
 * an operation affects an unknown static property on self::, but
 * can't change its reffiness.
 *
 * This could only do TInitCell because static properties can never
 * be unset.  We're just going to get rid of the callers of this
 * function over a few more changes, though.
 */
void loseNonRefSelfPropTypes(ISS& env) {
  FTRACE(2, "    loseNonRefSelfPropTypes\n");
  for (auto& kv : env.collect.props.privateStatics()) {
    if (kv.second.ty.subtypeOf(BInitCell)) {
      kv.second.ty |=
        adjust_type_for_prop(env.index, *env.ctx.cls, kv.second.tc, TCell);
    }
  }
}

bool isMaybeLateInitSelfProp(ISS& env, SString name) {
  if (!env.ctx.cls) return false;
  for (auto const& prop : env.ctx.cls->properties) {
    if (prop.name == name &&
        (prop.attrs & AttrPrivate) &&
        (prop.attrs & AttrStatic)
       ) {
      return prop.attrs & AttrLateInit;
    }
  }
  // Prop either doesn't exist, or is on an unflattened trait. Be conservative.
  return true;
}

//////////////////////////////////////////////////////////////////////
// misc

/*
 * Check whether the class given by the type might raise when initialized.
 */
bool classInitMightRaise(ISS& env, const Type& cls) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return false;
  if (!is_specialized_cls(cls)) return true;
  auto const dcls = dcls_of(cls);
  if (dcls.type != DCls::Exact) return true;
  return env.index.lookup_class_init_might_raise(env.ctx, dcls.cls);
}

void badPropInitialValue(ISS& env) {
  FTRACE(2, "    badPropInitialValue\n");
  env.collect.props.setBadPropInitialValues();
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

//////////////////////////////////////////////////////////////////////

}}

#endif
