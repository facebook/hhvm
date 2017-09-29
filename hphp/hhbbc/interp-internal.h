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

#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/func-util.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbbc);

const StaticString s_assert("assert");
const StaticString s_set_frame_metadata("HH\\set_frame_metadata");
const StaticString s_86metadata("86metadata");
const StaticString s_func_num_args("func_num_args");
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
    , blk(*bag.blk)
    , state(bag.state)
    , flags(flags)
    , propagate(propagate)
  {}

  const Index& index;
  const Context ctx;
  CollectedInfo& collect;
  const php::Block& blk;
  State& state;
  StepFlags& flags;
  PropagateFn propagate;
};

void impl_vec(ISS& env, bool reduce, std::vector<Bytecode>&& bcs);

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
void reduce(ISS& env, std::vector<Bytecode>&& bcs) {
  impl_vec(env, true, std::move(bcs));
}

template<class... Bytecodes>
void reduce(ISS& env, Bytecodes&&... hhbc) {
  reduce(env, { std::forward<Bytecodes>(hhbc)... });
}

bool fpassCanThrow(ISS& env, PrepKind kind, FPassHint hint) {
  switch (kind) {
  case PrepKind::Unknown: return hint != FPassHint::Any;
  case PrepKind::Val:     return hint == FPassHint::Ref;
  case PrepKind::Ref:     return hint == FPassHint::Cell;
  }
  not_reached();
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

void jmp_setdest(ISS& env, BlockId blk) {
  env.flags.jmpDest = blk;
}
void jmp_nevertaken(ISS& env) {
  jmp_setdest(env, env.blk.fallthrough);
}

void readUnknownParams(ISS& env) {
  for (LocalId p = 0; p < env.ctx.func->params.size(); p++) {
    if (p == env.flags.mayReadLocalSet.size()) break;
    env.flags.mayReadLocalSet.set(p);
  }
}

void readUnknownLocals(ISS& env) { env.flags.mayReadLocalSet.set(); }
void readAllLocals(ISS& env)     { env.flags.mayReadLocalSet.set(); }

void modifyLocalStatic(ISS& env, LocalId id, const Type& t) {
  auto modifyOne = [&] (LocalId lid) {
    if (is_volatile_local(env.ctx.func, lid)) return;
    if (env.state.localStaticBindings.size() <= lid) return;
    if (env.state.localStaticBindings[lid] == LocalStaticBinding::None) return;
    if (t.subtypeOf(TUninit) && !t.subtypeOf(TBottom)) {
      // Uninit means we are unbinding.
      env.state.localStaticBindings[lid] = id == NoLocalId ?
        LocalStaticBinding::None : LocalStaticBinding::Maybe;
      return;
    }
    if (lid >= env.collect.localStaticTypes.size()) {
      env.collect.localStaticTypes.resize(lid + 1, TBottom);
    }
    env.collect.localStaticTypes[lid] = t.subtypeOf(TCell) ?
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

void killLocals(ISS& env) {
  FTRACE(2, "    killLocals\n");
  readUnknownLocals(env);
  modifyLocalStatic(env, NoLocalId, TGen);
  for (auto& l : env.state.locals) l = TGen;
  for (auto& e : env.state.stack) e.equivLoc = NoLocalId;
  env.state.equivLocals.clear();
}

void doRet(ISS& env, Type t, bool hasEffects) {
  readAllLocals(env);
  assert(env.state.stack.empty());
  if (!hasEffects) {
    for (auto const& l : env.state.locals) {
      if (could_run_destructor(l)) {
        hasEffects = true;
        break;
      }
    }
    if (!hasEffects) effect_free(env);
  }
  env.flags.returned = t;
}

void mayUseVV(ISS& env) {
  env.collect.mayUseVV = true;
}

//////////////////////////////////////////////////////////////////////
// eval stack

Type popT(ISS& env) {
  assert(!env.state.stack.empty());
  auto const ret = std::move(env.state.stack.back().type);
  FTRACE(2, "    pop:  {}\n", show(ret));
  assert(ret.subtypeOf(TGen));
  env.state.stack.pop_back();
  return ret;
}

Type popC(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TInitCell));
  return v;
}

Type popV(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TRef));
  return v;
}

Type popU(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TUninit));
  return v;
}

Type popCU(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TCell));
  return v;
}

Type popR(ISS& env)  { return popT(env); }
Type popF(ISS& env)  { return popT(env); }
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
  assert(topT(env, i).subtypeOf(TInitCell));
  return topT(env, i);
}

Type& topR(ISS& env, uint32_t i = 0) { return topT(env, i); }

Type& topV(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(TRef));
  return topT(env, i);
}

void push(ISS& env, Type t) {
  FTRACE(2, "    push: {}\n", show(t));
  env.state.stack.push_back(StackElem {std::move(t), NoLocalId});
}

void push(ISS& env, Type t, LocalId l) {
  if (l != StackDupId) {
    if (l == NoLocalId || peekLocRaw(env, l).couldBe(TRef)) {
      return push(env, t);
    }
    assertx(!is_volatile_local(env.ctx.func, l)); // volatiles are TGen
  }
  FTRACE(2, "    push: {} (={})\n",
         show(t), l == StackDupId ? "Dup" : local_string(*env.ctx.func, l));
  env.state.stack.push_back(StackElem {std::move(t), l});
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
bool fpiPush(ISS& env, ActRec ar, int32_t nArgs) {
  auto foldable = [&] {
    if (nArgs < 0 ||
        ar.kind == FPIKind::Ctor ||
        ar.kind == FPIKind::Builtin ||
        !ar.func || ar.fallbackFunc) {
      return false;
    }
    auto const func = ar.func->exactFunc();
    if (!func) return false;
    if (env.collect.unfoldableFuncs.count(func)) return false;
    // Foldable builtins are always worth trying
    if (ar.func->isFoldable()) return true;
    // Any native functions at this point are known to be
    // non-foldable, but other builtins might be, even if they
    // don't have the __Foldable attribute.
    if (func->nativeInfo) return false;
    if (func->params.size()) {
      // Not worth trying if we're going to warn due to missing args
      return check_nargs_in_range(func, nArgs);
    }
    // If the function has no args, we can simply check that its effect free
    // and returns a literal.
    return env.index.is_effect_free(*ar.func) &&
    is_scalar(env.index.lookup_return_type_raw(func));
  }();
  if (foldable) effect_free(env);
  ar.foldable = foldable;
  ar.pushBlk = env.blk.id;

  FTRACE(2, "    fpi+: {}\n", show(ar));
  env.state.fpiStack.push_back(std::move(ar));
  return foldable;
}

void fpiPush(ISS& env, ActRec ar) {
  fpiPush(env, std::move(ar), -1);
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
  env.collect.unfoldableFuncs.emplace(func);
  env.propagate(ar.pushBlk, nullptr);
  ar.foldable = false;
  // we're going to reprocess the whole fpi region; any results we've
  // got so far are bogus, so stop prevent further useless work by
  // marking the next bytecode unreachable
  unreachable(env);
  FTRACE(2, "     fpi: not foldable\n");
}

PrepKind prepKind(ISS& env, uint32_t paramId) {
  auto& ar = fpiTop(env);
  if (ar.func && !ar.fallbackFunc) {
    auto const ret = env.index.lookup_param_prep(env.ctx, *ar.func, paramId);
    if (ar.foldable && ret != PrepKind::Val) {
      fpiNotFoldable(env);
    }
    if (ret != PrepKind::Unknown) {
      return ret;
    }
  }
  assertx(ar.kind != FPIKind::Builtin && !ar.foldable);
  return PrepKind::Unknown;
}

template<class... Bytecodes>
void killFPass(ISS& env, PrepKind kind, FPassHint hint, uint32_t arg,
               Bytecodes&&... bcs) {
  assert(kind != PrepKind::Unknown);

  // Since PrepKind is never Unknown for buildins or foldables we
  // should know statically if we will throw or not at runtime
  // (PrepKind and FPassHint don't match).
  if (fpassCanThrow(env, kind, hint)) {
    auto& ar = fpiTop(env);
    assertx(ar.foldable || ar.kind == FPIKind::Builtin);
    assertx(ar.func && !ar.fallbackFunc);
    return reduce(
      env,
      std::forward<Bytecodes>(bcs)...,
      bc::RaiseFPassWarning { hint, ar.func->name(), arg }
    );
  }
  return reduce(env, std::forward<Bytecodes>(bcs)...);
}

bool shouldKillFPass(ISS& env, FPassHint hint, uint32_t param) {
  auto& ar = fpiTop(env);
  if (ar.kind == FPIKind::Builtin) return true;
  if (!ar.foldable) return false;
  prepKind(env, param);
  if (!ar.foldable) return false;
  auto const ok = [&] {
    if (hint == FPassHint::Ref) return false;
    auto const &t = topT(env);
    if (!is_scalar(t)) return false;
    auto const callee = ar.func->exactFunc();
    if (param >= callee->params.size() ||
        (param + 1 == callee->params.size() &&
         callee->params.back().isVariadic)) {
      return true;
    }
    auto const constraint = callee->params[param].typeConstraint;
    if (!constraint.hasConstraint() ||
        constraint.isTypeVar() ||
        constraint.isTypeConstant()) {
      return true;
    }
    return env.index.satisfies_constraint(
      Context { callee->unit, const_cast<php::Func*>(callee), callee->cls },
      t, constraint);
  }();
  if (ok) return true;
  fpiNotFoldable(env);
  return false;
}

//////////////////////////////////////////////////////////////////////
// locals

void useLocalStatic(ISS& env, LocalId l) {
  assert(env.collect.localStaticTypes.size() > l);
  if (!env.flags.usedLocalStatics) {
    env.flags.usedLocalStatics = std::make_shared<std::map<LocalId,Type>>();
  }
  // Ignore the return value, since we only want the first type used,
  // as that will be the narrowest.
  env.flags.usedLocalStatics->emplace(l, env.collect.localStaticTypes[l]);
}

void mayReadLocal(ISS& env, uint32_t id) {
  if (id < env.flags.mayReadLocalSet.size()) {
    env.flags.mayReadLocalSet.set(id);
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
  always_assert(!is_volatile_local(env.ctx.func, from));
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
LocalId topStkLocal(ISS& env, uint32_t idx = 0) {
  assert(idx < env.state.stack.size());
  auto const equiv = env.state.stack[env.state.stack.size() - idx - 1].equivLoc;
  return equiv == StackDupId ? NoLocalId : equiv;
}

// Obtain a location which is equivalent to the given stack value
LocalId topStkEquiv(ISS& env, uint32_t idx = 0) {
  assert(idx < env.state.stack.size());
  return env.state.stack[env.state.stack.size() - idx - 1].equivLoc;
}

void setStkLocal(ISS& env, LocalId loc, uint32_t idx = 0) {
  assertx(loc <= MaxLocalId);
  while (true) {
    auto equiv = topStkEquiv(env, idx);
    if (equiv != StackDupId) {
      if (equiv <= MaxLocalId) {
        if (loc == equiv || locsAreEquiv(env, loc, equiv)) return;
        addLocEquiv(env, loc, equiv);
        return;
      }
      env.state.stack[env.state.stack.size() - idx - 1].equivLoc = loc;
      return;
    }
    idx++;
  }
}

// Kill all equivalencies involving the given local to stack values
void killStkEquiv(ISS& env, LocalId l) {
  for (auto& e : env.state.stack) {
    if (e.equivLoc == l) e.equivLoc = NoLocalId;
  }
}

void killAllStkEquiv(ISS& env) {
  for (auto& e : env.state.stack) {
    if (e.equivLoc != StackDupId) e.equivLoc = NoLocalId;
  }
}

Type peekLocRaw(ISS& env, LocalId l) {
  auto ret = env.state.locals[l];
  if (is_volatile_local(env.ctx.func, l)) {
    always_assert_flog(ret == TGen, "volatile local was not TGen");
  }
  return ret;
}

Type peekLocation(ISS& env, LocalId l, uint32_t idx = 0) {
  return l == StackDupId ? topT(env, idx) : peekLocRaw(env, l);
}

Type locRaw(ISS& env, LocalId l) {
  mayReadLocal(env, l);
  return peekLocRaw(env, l);
}

void setLocRaw(ISS& env, LocalId l, Type t) {
  mayReadLocal(env, l);
  killLocEquiv(env, l);
  killStkEquiv(env, l);
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
      if (t.subtypeOf(TBottom)) t = TInitNull;
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
  auto t = locRaw(env, l);
  return !t.subtypeOf(TCell) ? TInitCell :
          t.subtypeOf(TUninit) ? TInitNull :
          remove_uninit(std::move(t));
}

// Read a local type, dereferencing refs, but without converting
// potential TUninits to TInitNull.
Type derefLoc(ISS& env, LocalId l) {
  if (auto s = staticLocType(env, l, TCell)) {
    return std::move(*s);
  }
  auto v = locRaw(env, l);
  if (v.subtypeOf(TCell)) return v;
  return v.couldBe(TUninit) ? TCell : TInitCell;
}

bool locCouldBeUninit(ISS& env, LocalId l) {
  return locRaw(env, l).couldBe(TUninit);
}

bool locCouldBeRef(ISS& env, LocalId l) {
  return locRaw(env, l).couldBe(TRef);
}

/*
 * Update the known type of a local, based on assertions
 * (VerifyParamType; or IsType/JmpCC), rather than an actual
 * modification to the local.
 */
void refineLocHelper(ISS& env, LocalId l, Type t) {
  auto v = peekLocRaw(env, l);
  if (is_volatile_local(env.ctx.func, l)) {
    always_assert_flog(v == TGen, "volatile local was not TGen");
    return;
  }
  if (v.subtypeOf(TCell)) env.state.locals[l] = std::move(t);
}

template<typename F>
void refineLocation(ISS& env, LocalId l, F fun) {
  if (l == StackDupId) {
    auto stk = &env.state.stack.back();
    while (true) {
      stk->type = fun(std::move(stk->type));
      if (stk->equivLoc != StackDupId) break;
      assertx(stk != &env.state.stack.front());
      --stk;
    }
    l = stk->equivLoc;
  }
  if (l == NoLocalId) return;
  auto equiv = findLocEquiv(env, l);
  if (equiv != NoLocalId) {
    do {
      refineLocHelper(env, equiv, fun(peekLocRaw(env, equiv)));
      equiv = findLocEquiv(env, equiv);
    } while (equiv != l);
  }
  refineLocHelper(env, l, fun(peekLocRaw(env, l)));
}

template<typename PreFun, typename PostFun>
void refineLocation(ISS& env, LocalId l,
                    PreFun pre, BlockId target, PostFun post) {
  auto state = env.state;
  refineLocation(env, l, pre);
  env.propagate(target, &env.state);
  env.state = std::move(state);
  refineLocation(env, l, post);
}

/*
 * Set a local type in the sense of tvSet.  If the local is boxed or
 * not known to be not boxed, we can't change the type.  May be used
 * to set locals to types that include Uninit.
 */
void setLoc(ISS& env, LocalId l, Type t) {
  killLocEquiv(env, l);
  killStkEquiv(env, l);
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
    if (l.subtypeOf(TCell)) l = TCell;
  }
  killAllLocEquiv(env);
  killAllStkEquiv(env);
  modifyLocalStatic(env, NoLocalId, TCell);
}

void boxUnknownLocal(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "   boxUnknownLocal\n");
  for (auto& l : env.state.locals) {
    if (!l.subtypeOf(TRef)) l = TGen;
  }
  killAllLocEquiv(env);
  killAllStkEquiv(env);
  // Don't update the local statics here; this is called both for
  // boxing and binding, and the effects on local statics are
  // different.
}

void unsetUnknownLocal(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "  unsetUnknownLocal\n");
  for (auto& l : env.state.locals) l |= TUninit;
  killAllLocEquiv(env);
  killAllStkEquiv(env);
  unbindLocalStatic(env, NoLocalId);
}

//////////////////////////////////////////////////////////////////////
// Special functions

void specialFunctionEffects(ISS& env, const res::Func& func) {
  if (func.name()->isame(s_set_frame_metadata.get())) {
    /*
     * HH\set_frame_metadata can write to the local named 86metadata,
     * but doesn't require a VV.
     */
    auto const l = findLocal(env, s_86metadata.get());
    if (l != NoLocalId) setLoc(env, l, TInitCell);
    return;
  }

  if (func.name()->isame(s_assert.get())) {
    /*
     * Assert is somewhat special. In the most general case, it can read and
     * write to the caller's frame (and is marked as such). The first parameter,
     * if a string, will be evaled and can have arbitrary effects. Luckily this
     * is forbidden in RepoAuthoritative mode, so we can ignore that here. If
     * the assert fails, it may execute an arbitrary pre-registered callback
     * which still might try to write to the assert caller's frame. This can't
     * happen if calling such frame accessing functions dynamically is
     * forbidden.
     */
    if (RuntimeOption::DisallowDynamicVarEnvFuncs == HackStrictOption::ON) {
      return;
    }
  }

  /*
   * Skip-frame functions won't write or read to the caller's frame, but they
   * might dynamically call a function which can. So, skip-frame functions kill
   * our locals unless they can't call such functions.
   */
  if (func.mightWriteCallerFrame() ||
      (RuntimeOption::DisallowDynamicVarEnvFuncs != HackStrictOption::ON &&
       func.mightBeSkipFrame())) {
    readUnknownLocals(env);
    killLocals(env);
    mayUseVV(env);
    return;
  }

  if (func.mightReadCallerFrame()) {
    if (func.name()->isame(s_func_num_args.get())) return;
    if (func.name()->isame(s_func_get_args.get()) ||
        func.name()->isame(s_func_get_arg.get()) ||
        func.name()->isame(s_func_slice_args.get())) {
      readUnknownParams(env);
    } else {
      readUnknownLocals(env);
    }
    mayUseVV(env);
    return;
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
        killLocals(env);
        mayUseVV(env);
      }
      return;
    }
  case FPIKind::Builtin:
    specialFunctionEffects(env, *ar.func);
    if (ar.fallbackFunc) specialFunctionEffects(env, *ar.fallbackFunc);
    break;
  case FPIKind::Ctor:
  case FPIKind::ObjMeth:
  case FPIKind::ClsMeth:
  case FPIKind::ObjInvoke:
  case FPIKind::CallableArr:
    /*
     * Methods cannot read or write to the caller's frame, but they can be
     * skip-frame (if they're a builtin). So, its possible they'll dynamically
     * call a function which reads or writes to the caller's frame. If we don't
     * forbid this, we have to be pessimistic. Imagine something like
     * Vector::map calling assert.
     */
    if (RuntimeOption::DisallowDynamicVarEnvFuncs != HackStrictOption::ON &&
        (!ar.func || ar.func->mightBeSkipFrame())) {
      readUnknownLocals(env);
      killLocals(env);
      mayUseVV(env);
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////
// class-ref slots

// Read the specified class-ref slot without discarding the stored value.
const Type& peekClsRefSlot(ISS& env, ClsRefSlotId slot) {
  assert(slot != NoClsRefSlotId);
  always_assert_flog(env.state.clsRefSlots[slot].subtypeOf(TCls),
                     "class-ref slot contained non-TCls");
  return env.state.clsRefSlots[slot];
}

// Read the specified class-ref slot and discard the stored value.
Type takeClsRefSlot(ISS& env, ClsRefSlotId slot) {
  assert(slot != NoClsRefSlotId);
  auto ret = std::move(env.state.clsRefSlots[slot]);
  FTRACE(2, "    read class-ref: {} -> {}\n", slot, show(ret));
  always_assert_flog(ret.subtypeOf(TCls), "class-ref slot contained non-TCls");
  env.state.clsRefSlots[slot] = TCls;
  return ret;
}

void putClsRefSlot(ISS& env, ClsRefSlotId slot, Type ty) {
  assert(slot != NoClsRefSlotId);
  always_assert_flog(ty.subtypeOf(TCls),
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
  env.state.iters[iter] = UnknownIter {};
}

//////////////////////////////////////////////////////////////////////
// $this

void setThisAvailable(ISS& env) {
  FTRACE(2, "    setThisAvailable\n");
  env.state.thisAvailable = true;
}

bool thisAvailable(ISS& env) { return env.state.thisAvailable; }

// Returns the type $this would have if it's not null.  Generally
// you have to check thisIsAvailable() before assuming it can't be
// null.
folly::Optional<Type> thisType(ISS& env) {
  if (!env.ctx.cls) return folly::none;

  // Due to `bindTo`, we can't conclude the type of $this.
  if (RuntimeOption::EvalAllowScopeBinding && env.ctx.func->isClosureBody) {
    return folly::none;
  }

  // Due to unflattened traits in non-repo mode, we can't conclude $this type.
  if (!RuntimeOption::RepoAuthoritative && env.ctx.cls->attrs & AttrTrait) {
    return folly::none;
  }

  return subObj(env.index.resolve_class(env.ctx.cls));
}

folly::Optional<Type> selfCls(ISS& env) {
  if (auto rcls = env.index.selfCls(env.ctx)) return subCls(*rcls);
  return folly::none;
}

folly::Optional<Type> selfClsExact(ISS& env) {
  if (auto rcls = env.index.selfCls(env.ctx)) return clsExact(*rcls);
  return folly::none;
}

folly::Optional<Type> parentClsExact(ISS& env) {
  if (auto rcls = env.index.parentCls(env.ctx)) return clsExact(*rcls);
  return folly::none;
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

Type* thisPropRaw(ISS& env, SString name) {
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

bool isNonSerializedThisProp(ISS& env, SString name) {
  return env.collect.props.isNonSerialized(name);
}

void killThisProps(ISS& env) {
  FTRACE(2, "    killThisProps\n");
  for (auto& kv : env.collect.props.privateProperties()) {
    kv.second = TGen;
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
  auto const t = thisPropRaw(env, name);
  if (!t) return folly::none;
  if (t->couldBe(TUninit)) {
    auto const rthis = thisType(env);
    if (!rthis || dobj_of(*rthis).cls.couldHaveMagicGet()) {
      return TInitCell;
    }
  }
  return !t->subtypeOf(TCell) ? TInitCell :
          t->subtypeOf(TUninit) ? TInitNull :
          remove_uninit(*t);
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
  auto const t = thisPropRaw(env, name);
  if (!t) return;
  *t |= (isNonSerializedThisProp(env, name) ? type : loosen_all(type));
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
    mergeThisProp(env, kv.first, fn(kv.second));
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
  auto const t = thisPropRaw(env, name);
  if (!t) return;
  *t |= TRef;
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
    if (kv.second.subtypeOf(TCell)) kv.second = TCell;
  }
}

//////////////////////////////////////////////////////////////////////
// properties on self::

// Similar to $this properties above, we only track control-flow
// insensitive types for these.

Type* selfPropRaw(ISS& env, SString name) {
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
    kv.second = TGen;
  }
}

void killSelfProp(ISS& env, SString name) {
  FTRACE(2, "    killSelfProp {}\n", name->data());
  if (auto t = selfPropRaw(env, name)) *t = TGen;
}

// TODO(#3684136): self::$foo can't actually ever be uninit.  Right
// now uninits may find their way into here though.
folly::Optional<Type> selfPropAsCell(ISS& env, SString name) {
  auto const t = selfPropRaw(env, name);
  if (!t) return folly::none;
  return !t->subtypeOf(TCell) ? TInitCell :
          t->subtypeOf(TUninit) ? TInitNull :
          remove_uninit(*t);
}

/*
 * Merges a type into tracked static properties on self, in the
 * sense of tvSet (i.e. setting the inner type on possible refs).
 */
void mergeSelfProp(ISS& env, SString name, Type type) {
  auto const t = selfPropRaw(env, name);
  if (!t) return;
  *t |= type;
}

/*
 * Similar to mergeEachThisPropRaw, but for self props.
 */
template<class MapFn>
void mergeEachSelfPropRaw(ISS& env, MapFn fn) {
  for (auto& kv : env.collect.props.privateStatics()) {
    mergeSelfProp(env, kv.first, fn(kv.second));
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
    if (kv.second.subtypeOf(TInitCell)) kv.second = TCell;
  }
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

//////////////////////////////////////////////////////////////////////

}}

#endif
