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
#include "hphp/hhbbc/interp.h"

#include <vector>
#include <algorithm>
#include <string>
#include <utility>

#include <folly/Optional.h>
#include <folly/Format.h>

#include "hphp/util/trace.h"

#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/type-ops.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

// Represents the "effectful"-ness of a particular member instruction
// operation. This includes operations which are visible outside of
// the function (writing to a static property), but also potential
// throwing. Many different aspects of a member instruction operation
// can have effects, so we combine the effects from the various
// portions and use the final result to determine how to mark the
// bytecode.
enum class Effects {
  None,         // Effect-free
  SideEffect,   // Cannot throw, but has some side-effect
  Throws,       // Might throw an exception
  AlwaysThrows  // Always throws an exception
};

//////////////////////////////////////////////////////////////////////

/*
 * A note about bases.
 *
 * Generally type inference needs to know two kinds of things about the base to
 * handle effects on tracked locations:
 *
 *   - Could the base be a location we're tracking deeper structure on, so the
 *     next operation actually affects something inside of it.  For example,
 *     could the base be an object with the same type as $this, or an array in a
 *     local variable.
 *
 *   - Could the base be something (regardless of type) that is inside one of
 *     the things we're tracking.  I.e., the base might be whatever (an array or
 *     a bool or something), but living inside a property inside an object with
 *     the same type as $this, or living inside of an array in the local frame.
 *
 * The first cases apply because final operations are going to directly affect
 * the type of these elements.  The second case is because member operations may
 * change the base at each step if it is a defining instruction.
 *
 * Note that both of these cases can apply to the same base in some cases: you
 * might have an object property on $this that could be an object of the type of
 * $this.
 */

//////////////////////////////////////////////////////////////////////

bool couldBeThisObj(ISS& env, const Base& b) {
  if (b.loc == BaseLoc::This) return true;
  auto const thisTy = thisTypeFromContext(env.index, env.ctx);
  return b.type.couldBe(thisTy ? *thisTy : TObj);
}

bool mustBeThisObj(ISS& env, const Base& b) {
  if (b.loc == BaseLoc::This) return true;
  if (auto const ty = thisTypeFromContext(env.index, env.ctx)) {
    return b.type.subtypeOf(*ty);
  }
  return false;
}

bool mustBeInLocal(const Base& b) {
  return b.loc == BaseLoc::Local;
}

bool mustBeInStack(const Base& b) {
  return b.loc == BaseLoc::Stack;
}

bool mustBeInStatic(const Base& b) {
  return b.loc == BaseLoc::StaticProp;
}

//////////////////////////////////////////////////////////////////////

// Base locations that only occur at the start of a minstr sequence.
bool isInitialBaseLoc(BaseLoc loc) {
  return
    loc == BaseLoc::Local ||
    loc == BaseLoc::Stack ||
    loc == BaseLoc::StaticProp ||
    loc == BaseLoc::This ||
    loc == BaseLoc::Global;
}

// Base locations that only occur after the start of a minstr sequence.
bool isDimBaseLoc(BaseLoc loc) {
  return loc == BaseLoc::Elem || loc == BaseLoc::Prop;
}

//////////////////////////////////////////////////////////////////////

/*
 * If the current base is an array-like, update it via *_set, and
 * return whether the operation can possibly throw. Otherwise, return
 * folly::none;
 */
folly::Optional<TriBool> array_do_set(ISS& env,
                                      const Type& key,
                                      const Type& value) {
  auto& base = env.collect.mInstrState.base.type;
  if (!base.subtypeOf(BOptArrLike)) return folly::none;
  if (base.subtypeOf(BInitNull)) return folly::none;

  auto const tag = provTagHere(env);
  auto set = array_like_set(base, key, value, tag);
  if (set.first.subtypeOf(BBottom)) return TriBool::Yes;
  base = std::move(set.first);
  return maybeOrNo(set.second);
}

/*
 * If the current base is an array-like, return the best known type
 * for base[key] and whether the operation can possibly
 * throw. Otherwise, return folly::none;
 */
folly::Optional<std::pair<Type, TriBool>> array_do_elem(ISS& env,
                                                        bool nullOnMissing,
                                                        const Type& key) {
  auto const& base = env.collect.mInstrState.base.type;
  if (!base.subtypeOf(BOptArrLike)) return folly::none;
  if (base.subtypeOf(BInitNull)) return folly::none;

  auto elem = array_like_elem(base, key);
  auto const throws = [&] {
    switch (elem.second) {
      case ThrowMode::None:
        return TriBool::No;
      case ThrowMode::MaybeMissingElement:
      case ThrowMode::MissingElement:
        if (nullOnMissing) {
          elem.first |= TInitNull;
          return TriBool::No;
        }
        return TriBool::Maybe;
      case ThrowMode::MaybeBadKey:
        if (nullOnMissing) elem.first |= TInitNull;
        return TriBool::Maybe;
      case ThrowMode::BadOperation:
        // isset($a["abc"]) does not throw if $a is a vec, so we need to be
        // conservative here.
        if (nullOnMissing) {
          if (base.couldBe(BVecish) && key.couldBe(BStr)) {
            elem.first |= TInitNull;
            if (base.subtypeOf(BOptVecish) && key.subtypeOf(BStr)) {
              return TriBool::No;
            }
          }
        }
        return TriBool::Maybe;
    }
    always_assert(false);
  }();

  if (elem.first.subtypeOf(BBottom)) {
    return std::make_pair(TBottom, TriBool::Yes);
  }
  return std::make_pair(std::move(elem.first), throws);
}

/*
 * If the current base is an array-like, update it via *_newelem, and
 * return whether the newelem can throw; otherwise return folly::none.
 */
folly::Optional<TriBool> array_do_newelem(ISS& env, const Type& value) {
  auto& base = env.collect.mInstrState.base.type;
  if (!base.subtypeOf(BOptArrLike)) return folly::none;
  if (base.subtypeOf(BInitNull)) return folly::none;

  auto const tag = provTagHere(env);
  auto update = array_like_newelem(base, value, tag);
  if (update.first.subtypeOf(BBottom)) return TriBool::Yes;
  base = std::move(update.first);
  return maybeOrNo(update.second);
}

//////////////////////////////////////////////////////////////////////

void setLocalForBase(ISS& env, Type ty, LocalId firstKeyLoc) {
  assert(mustBeInLocal(env.collect.mInstrState.base));
  if (env.collect.mInstrState.base.locLocal == NoLocalId) {
    return killLocals(env);
  }
  FTRACE(4, "      ${} := {}\n",
    env.collect.mInstrState.base.locName
     ? env.collect.mInstrState.base.locName->data()
     : "$<unnamed>",
    show(ty)
  );
  setLoc(
    env,
    env.collect.mInstrState.base.locLocal,
    std::move(ty),
    firstKeyLoc
  );
}

void setStackForBase(ISS& env, Type ty) {
  assert(mustBeInStack(env.collect.mInstrState.base));

  auto const locSlot = env.collect.mInstrState.base.locSlot;
  FTRACE(4, "      stk[{:02}] := {}\n", locSlot, show(ty));
  assert(locSlot < env.state.stack.size());

  env.state.stack[locSlot] = StackElem { std::move(ty), NoLocalId };
}

void setStaticForBase(ISS& env, Type ty) {
  auto const& base = env.collect.mInstrState.base;
  assertx(mustBeInStatic(base));

  auto const nameTy = base.locName ? sval(base.locName) : TStr;
  FTRACE(
    4, "      ({})::$({}) |= {}\n",
    show(base.locTy), show(nameTy), show(ty)
  );

  env.index.merge_static_type(
    env.ctx,
    env.collect.publicSPropMutations,
    env.collect.props,
    base.locTy,
    nameTy,
    std::move(ty)
  );
}

// Run backwards through an array chain doing array_set operations
// to produce the array type that incorporates the effects of any
// intermediate defining dims.
Type currentChainType(ISS& env, Type val) {
  auto it = env.collect.mInstrState.arrayChain.end();
  auto const tag = provTagHere(env);
  while (it != env.collect.mInstrState.arrayChain.begin()) {
    --it;
    assertx(it->base.subtypeOf(BArrLike));
    val = array_like_set(it->base, it->key, val, tag).first;
    if (val.subtypeOf(BBottom)) return TBottom;
  }
  return val;
}

Type resolveArrayChain(ISS& env, Type val) {
  static UNUSED const char prefix[] = "              ";
  FTRACE(5, "{}chain {}\n", prefix, show(val));
  auto const tag = provTagHere(env);
  do {
    auto arr = std::move(env.collect.mInstrState.arrayChain.back().base);
    auto key = std::move(env.collect.mInstrState.arrayChain.back().key);
    env.collect.mInstrState.arrayChain.pop_back();
    FTRACE(5, "{}  | {} := {} in {}\n", prefix,
      show(key), show(val), show(arr));
    assertx(arr.subtypeOf(BArrLike));
    val = array_like_set(std::move(arr), key, val, tag).first;
    if (val.subtypeOf(BBottom)) env.collect.mInstrState.arrayChain.clear();
  } while (!env.collect.mInstrState.arrayChain.empty());
  FTRACE(5, "{}  = {}\n", prefix, show(val));
  return val;
}

// Returns true if the base update can be considered "effect-free"
// (IE, updating a static prop base is a side-effect because its
// visible elsewhere. Updating a local or stack slot is not).
bool updateBaseWithType(ISS& env,
                        const Type& ty,
                        LocalId firstKeyLoc = NoLocalId) {
  FTRACE(6, "    updateBaseWithType: {}\n", show(ty));

  if (ty.subtypeOf(BBottom)) return true;

  auto const& base = env.collect.mInstrState.base;

  if (mustBeInLocal(base)) {
    setLocalForBase(env, ty, firstKeyLoc);
    // If we're speculating, a local update is considered a
    // side-effect.
    return !any(env.collect.opts & CollectionOpts::Speculating);
  }
  if (mustBeInStack(base)) {
    setStackForBase(env, ty);
    return true;
  }
  if (mustBeInStatic(base)) {
    setStaticForBase(env, ty);
    return false;
  }

  return true;
}

void startBase(ISS& env, Base base) {
  auto& oldState = env.collect.mInstrState;
  assert(oldState.base.loc == BaseLoc::None);
  assert(oldState.arrayChain.empty());
  assert(isInitialBaseLoc(base.loc));
  assert(!base.type.subtypeOf(TBottom));

  oldState.effectFree = env.flags.effectFree;
  oldState.extraPop = false;
  oldState.base = std::move(base);
  FTRACE(5, "    startBase: {}\n", show(*env.ctx.func, oldState.base));
}

// Return true if the base is updated and that update is considered
// "effect-free" (see updateBaseWithType).
bool endBase(ISS& env, bool update = true, LocalId keyLoc = NoLocalId) {
  auto& state = env.collect.mInstrState;
  assert(state.base.loc != BaseLoc::None);

  FTRACE(5, "    endBase: {}\n", show(*env.ctx.func, state.base));

  auto const firstKeyLoc = state.arrayChain.empty()
    ? keyLoc
    : state.arrayChain.data()->keyLoc;
  auto const& ty = state.arrayChain.empty()
    ? state.base.type
    : resolveArrayChain(env, state.base.type);

  auto const effectFree = update
    ? updateBaseWithType(env, ty, firstKeyLoc)
    : true;
  state.base.loc = BaseLoc::None;
  return effectFree;
}

// Return true if the base is updated and that update is considered
// "effect-free" (see updateBaseWithType).
bool moveBase(ISS& env,
              Base newBase,
              bool update = true,
              LocalId keyLoc = NoLocalId) {
  auto& state = env.collect.mInstrState;
  assert(state.base.loc != BaseLoc::None);
  assert(isDimBaseLoc(newBase.loc));
  assert(!state.base.type.subtypeOf(BBottom));

  FTRACE(5, "    moveBase: {} -> {}\n",
         show(*env.ctx.func, state.base),
         show(*env.ctx.func, newBase));

  auto const firstKeyLoc = state.arrayChain.empty()
    ? keyLoc
    : state.arrayChain.data()->keyLoc;
  auto const& ty = state.arrayChain.empty()
    ? state.base.type
    : resolveArrayChain(env, state.base.type);

  auto const effectFree = update
    ? updateBaseWithType(env, ty, firstKeyLoc)
    : true;
  state.base = std::move(newBase);
  return effectFree;
}

// Return true if the base is updated and that update is considered
// "effect-free" (see updateBaseWithType).
bool extendArrChain(ISS& env, Type key, Type arr,
                    Type val, bool update = true,
                    LocalId keyLoc = NoLocalId) {
  auto& state = env.collect.mInstrState;
  assertx(state.base.loc != BaseLoc::None);
  // NB: The various array operation functions can accept Opt arrays
  // (they just ignore the Opt portion). However we still do not allow
  // putting Opt arrays in the chain, since (in general) its not clear
  // how to deal with the Optness with resolving the chain. Currently
  // the only case where we set up chains with possible Optness is for
  // ElemD, and there we'd fatal, so we remove the Optness before
  // calling this.
  assertx(arr.subtypeOf(BArrLike));
  assertx(!state.base.type.subtypeOf(BBottom));
  assertx(!val.subtypeOf(BBottom));

  state.arrayChain.emplace_back(
    CollectedInfo::MInstrState::ArrayChainEnt{
      std::move(arr),
      std::move(key),
      keyLoc
    }
  );
  state.base.type = std::move(val);

  auto const firstKeyLoc = state.arrayChain.data()->keyLoc;

  FTRACE(5, "    extendArrChain: {}\n", show(*env.ctx.func, state));
  if (update) {
    return updateBaseWithType(
      env,
      currentChainType(env, state.base.type),
      firstKeyLoc
    );
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

// Returns nullptr if it's an unknown key or not a string.
SString mStringKey(const Type& key) {
  auto const v = tv(key);
  return v && v->m_type == KindOfPersistentString ? v->m_data.pstr : nullptr;
}

template<typename Op>
auto update_mkey(const Op& op) { return false; }

template<typename Op>
auto update_mkey(Op& op) -> decltype(op.mkey, true) {
  switch (op.mkey.mcode) {
    case MEC: case MPC: {
      op.mkey.idx++;
      return true;
    }
    default:
      return false;
  }
}

template<typename Op>
auto update_discard(const Op& op) { return false; }

template<typename Op>
auto update_discard(Op& op) -> decltype(op.arg1, true) {
  op.arg1++;
  return true;
}

/*
 * Return the type of the key and whether any promotions happened, or
 * reduce op and return folly::none.  Note that when folly::none is
 * returned, there is nothing further to do.
 */
template<typename Op>
folly::Optional<std::pair<Type,Promotion>> key_type_or_fixup(ISS& env, Op op) {
  if (env.collect.mInstrState.extraPop) {
    auto const mkey = update_mkey(op);
    if (update_discard(op) || mkey) {
      env.collect.mInstrState.extraPop = false;
      reduce(env, op);
      env.collect.mInstrState.extraPop = true;
      return folly::none;
    }
  }
  auto const fixup = [&] (Type ty, bool isProp, bool couldBeUninit)
    -> folly::Optional<std::pair<Type,Promotion>> {

    // Handle any classlike key promotions
    auto promoted = promote_classlike_to_key(std::move(ty));
    // We could also promote and potentially throw if we had an uninit
    // local.
    if (couldBeUninit) promoted.second = Promotion::YesMightThrow;
    // If we might throw, we don't want to const prop the key
    if (promoted.second == Promotion::YesMightThrow) return promoted;

    if (auto const val = tv(promoted.first)) {
      if (isStringType(val->m_type)) {
        op.mkey.mcode = isProp ? MPT : MET;
        op.mkey.litstr = val->m_data.pstr;
        reduce(env, op);
        return folly::none;
      }
      if (!isProp && val->m_type == KindOfInt64) {
        op.mkey.mcode = MEI;
        op.mkey.int64 = val->m_data.num;
        reduce(env, op);
        return folly::none;
      }
    }
    return promoted;
  };
  switch (op.mkey.mcode) {
    case MEC: case MPC:
      return fixup(topC(env, op.mkey.idx), op.mkey.mcode == MPC, false);
    case MEL: case MPL: {
      auto couldBeUninit = true;
      if (!peekLocCouldBeUninit(env, op.mkey.local.id)) {
        couldBeUninit = false;
        auto const minLocEquiv = findMinLocEquiv(env, op.mkey.local.id, false);
        if (minLocEquiv != NoLocalId) {
          op.mkey.local = NamedLocal { kInvalidLocalName, minLocEquiv };
          reduce(env, op);
          return folly::none;
        }
      }
      return fixup(
        locAsCell(env, op.mkey.local.id),
        op.mkey.mcode == MPL,
        couldBeUninit
      );
    }
    case MW:
      return std::make_pair(TBottom, Promotion::No);
    case MEI:
      return std::make_pair(ival(op.mkey.int64), Promotion::No);
    case MET: case MPT: case MQT:
      return std::make_pair(sval(op.mkey.litstr), Promotion::No);
  }
  not_reached();
}

template<typename Op>
LocalId key_local(ISS& env, Op op) {
  switch (op.mkey.mcode) {
    case MEC: case MPC:
      return topStkLocal(env, op.mkey.idx);
    case MEL: case MPL:
      return op.mkey.local.id;
    case MW:
    case MEI:
    case MET: case MPT: case MQT:
      return NoLocalId;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

// Handle the promotions that can happen to the base for ElemU or
// ElemD operations (including mutating final operations). Return
// whether any promotion happened.
Promotion handleElemUDBasePromos(ISS& env) {
  auto& base = env.collect.mInstrState.base.type;
  Promotion promotion;
  std::tie(base, promotion) = promote_clsmeth_to_vecish(std::move(base));
  return promotion;
}

// Handle the general effects that can happen to the base for ElemU or
// ElemD operations. This operations will COW the base. This is only
// the general effects. It is up to the caller to also incorporate any
// changes specific to the operation (if one cannot predict the
// changes, use pessimizeBaseForElemUD instead).
void handleElemUDEffects(ISS& env) {
  auto& base = env.collect.mInstrState.base.type;
  // Remove staticness from the types which will get COWed.
  base = loosen_aggregate_staticness(std::move(base));
}

// Handle the effects that can happen to the base for ElemU or ElemD
// operations in a pessimistic manner. This is used when one cannot or
// doesn't wish to model the effects of that particular operation
// precisely.
void pessimizeBaseForElemUD(ISS& env) {
  auto& base = env.collect.mInstrState.base.type;
  // We cannot predict what will happen to the base in further member
  // operations, so remove what we know about the values. We also need
  // to remove staticness to account for COW.
  base = loosen_aggregate_staticness(loosen_array_values(std::move(base)));
}

//////////////////////////////////////////////////////////////////////

// Helper function for ending the base. Takes the current aggregated
// effects up until this point, and whether the base underwent any
// promotions. Returns an updated aggregated effects, taking into
// account any effects from the base write-back.
Effects endBaseWithEffects(ISS& env, Effects effects, bool update,
                           Promotion basePromo, LocalId keyLoc = NoLocalId) {
  // Use endUnreachableBase for always throwing cases
  assertx(effects != Effects::AlwaysThrows);
  // End the base. We request a base update if the caller requested,
  // or if a base promotion happened (if the base promoted, we need to
  // record the new type into it).
  auto const effectFree =
    endBase(env, update || basePromo != Promotion::No, keyLoc);
  // If we might throw because of base promotion, it doesn't matter
  // what other effects are.
  if (basePromo == Promotion::YesMightThrow) return Effects::Throws;
  // Special case: if we don't have any other effects, but writing to
  // the base is side-effectful, then we can be nothrow (but not
  // effect_free).
  if (!effectFree && effects == Effects::None) return Effects::SideEffect;
  return effects;
}

// Helper function for ending the base when we know this member
// instruction will always throw.
Effects endUnreachableBase(ISS& env, Promotion basePromo,
                           LocalId keyLoc = NoLocalId) {
  // If we promoted the base, we still need to reflect that
  if (basePromo != Promotion::No) endBase(env, true, keyLoc);
  return Effects::AlwaysThrows;
}

//////////////////////////////////////////////////////////////////////

// Helper function for Elem (not ElemD or ElemU) operations. Luckily
// the logic for the Dim portion, and the final operation are
// identical, so we can treat them as the same. Return the elem type
// and what effects the access has.
std::pair<Type, Effects> elemHelper(ISS& env, MOpMode mode, Type key) {
  assertx(mode == MOpMode::None ||
          mode == MOpMode::Warn ||
          mode == MOpMode::InOut);

  auto const& base = env.collect.mInstrState.base.type;

  // If we're using MOpMode::InOut, then the base has to be a ArrLike
  // or we'll throw.
  auto inOutFail = false;
  if (mode == MOpMode::InOut) {
    if (!base.couldBe(BArrLike)) return { TBottom, Effects::AlwaysThrows };
    if (!base.subtypeOf(BArrLike)) inOutFail = true;
  }

  // Null and False silently push Null. True, Int, Dbl, Resources,
  // Func-likes, and RClsMeth push Null but warn first. ClsMeth will
  // behave the same as RClsMeth with the right setting.
  auto const warnsWithNull =
    BTrue | BNum | BRes | BFuncLike | BRClsMeth |
    (!RO::EvalIsCompatibleClsMethType ? BClsMeth : BBottom);
  auto const justNull = BNull | BFalse;
  if (base.subtypeOf(warnsWithNull | justNull)) {
    return {
      TInitNull,
      (base.subtypeOf(justNull) && !inOutFail)
        ? Effects::None
        : Effects::Throws
    };
  }

  // Strings will return a static string (a character from itself or
  // an empty string). ClsLikes will convert to its equivalent string
  // first.
  if (base.subtypeOf(BOptStr | BOptClsLike)) {
    auto const isNoThrow =
      !inOutFail &&
      mode != MOpMode::Warn &&
      key.subtypeOf(BArrKey) &&
      (!base.couldBe(BClsLike) ||
       !RuntimeOption::EvalRaiseClassConversionWarning);
    return {
      base.couldBe(BNull) ? TOptSStr : TSStr,
      isNoThrow ? Effects::None : Effects::Throws
    };
  }

  // With the right setting, ClsMeth will behave like an equivalent
  // varray/vec (but will not actually promote). This is the same as a
  // 2 element vec/varray containing static strings.
  if (RO::EvalIsCompatibleClsMethType && base.subtypeOf(BOptClsMeth)) {
    auto const isNoThrow =
      !inOutFail &&
      mode == MOpMode::None &&
      !RuntimeOption::EvalRaiseClsMethConversionWarning;
    return {
      TOptSStr,
      isNoThrow ? Effects::None : Effects::Throws
    };
  }

  // If its an OptArrLike, we can determine the element type. If we're
  // in MOpMode::None, array_do_elem will automatically add TInitNull
  // for a misssing key.
  if (auto elem = array_do_elem(env, mode == MOpMode::None, key)) {
    if (elem->second == TriBool::Yes) {
      // The element definitely doesn't exist. If the base could be
      // null, then we might throw or get InitNull. Otherwise we know
      // we'll throw.
      if (!base.couldBe(BNull)) return { TBottom, Effects::AlwaysThrows };
      return { TInitNull, Effects::Throws };
    }
    // Otherwise the element might exist (or definitely doesn't exist
    // and array_do_elem added TInitNull).
    return {
      base.couldBe(BNull)
        ? union_of(TInitNull, std::move(elem->first))
        : std::move(elem->first),
      (elem->second == TriBool::No && !inOutFail)
        ? Effects::None
        : Effects::Throws
    };
  }

  // Otherwise its some mix of things we cannot disentangle. Be
  // pessimistic.
  return { TInitCell, Effects::Throws };
}

// Helper function for SetOpElem/IncDecElem final operations. Reads
// the element in the base given by the key, performs the operation
// using `op', then writes the new value back to the base. Returns the
// aggregates effects of the entire operation.
template <typename F>
Effects setOpElemHelper(ISS& env, int32_t nDiscard, const Type& key,
                        LocalId keyLoc, F op) {
  // Before anything else, the base might promote to a different type
  auto const promo = handleElemUDBasePromos(env);

  auto& base = env.collect.mInstrState.base.type;

  auto const stack = [&] (Type ty, Effects effects) {
    discard(env, nDiscard);
    push(env, std::move(ty));
    return effects;
  };

  auto const end = [&] (Type ty, bool update, Effects effects) {
    return stack(
      std::move(ty),
      endBaseWithEffects(env, effects, update, promo, keyLoc)
    );
  };
  auto const unreachable = [&] {
    return stack(
      TBottom,
      endUnreachableBase(env, promo, keyLoc)
    );
  };

  // Invalid bases which will either always throw, or warn and push a
  // null value onto the stack.
  auto const throws = BNull | BFalse | BStr | BKeyset;
  auto const warnsAndNull =
    BTrue | BNum | BRes | BFuncLike | BClsLike | BClsMethLike;
  if (base.subtypeOf(throws | warnsAndNull)) {
    if (base.subtypeOf(throws)) return unreachable();
    return end(TInitNull, false, Effects::Throws);
  }

  // If its an OptArrLike, we can attempt to analyze the effects
  // precisely
  if (auto elem = array_do_elem(env, false, key)) {
    // Element doesn't exist. Always throws
    if (elem->second == TriBool::Yes) return unreachable();

    // If the op throws, we'll have already have COWed the array, so
    // we need to manually remove staticness.
    handleElemUDEffects(env);

    // If the base is actually null, we'll throw. Therefore we can
    // remove the null when analyzing the result.
    auto const couldBeNull = base.couldBe(BNull);
    if (couldBeNull) base = unopt(std::move(base));

    // Perform the op
    auto [toSet, toPush, opEffects] = op(std::move(elem->first));
    if (opEffects == Effects::AlwaysThrows) {
      // The op will always throw. We've already COWed the base, so we
      // still need to update the base type.
      endBase(env, true, keyLoc);
      return stack(TBottom, Effects::AlwaysThrows);
    }

    // Write the element back into the array
    auto const set = array_do_set(env, key, toSet);
    assertx(set);
    if (*set == TriBool::Yes) {
      // The set will always throw. In theory this can happen if do
      // something like read a string out of a keyset, turn it into
      // something that's not an array key, then try to write it back.
      endBase(env, true, keyLoc);
      return stack(TBottom, Effects::AlwaysThrows);
    }

    auto const maybeThrows =
      couldBeNull ||
      (elem->second == TriBool::Maybe) ||
      (*set == TriBool::Maybe);

    return end(
      std::move(toPush),
      true,
      maybeThrows ? Effects::Throws : opEffects
    );
  }

  // Otherwise its something we cannot analyze. Pessimize the base
  // (because we don't know what will be written back) and assume we
  // can throw and anything can be pushed.
  pessimizeBaseForElemUD(env);
  return end(TInitCell, true, Effects::Throws);
}

// Helper function for SetOpNewElem/IncDecNewElemm final
// operations. These all either throw or push null.
Effects setOpNewElemHelper(ISS& env, int32_t nDiscard) {
  auto const& base = env.collect.mInstrState.base.type;

  // These always throw
  if (base.subtypeOf(BNull | BFalse | BStr | BArrLike | BObj |
                     BClsMeth | BRecord)) {
    discard(env, nDiscard);
    push(env, TBottom);
    return Effects::AlwaysThrows;
  }

  // And these raise a warning and push null.
  endBase(env, false);
  discard(env, nDiscard);
  push(env, TInitNull);
  return Effects::Throws;
}

//////////////////////////////////////////////////////////////////////
// intermediate ops

Effects miProp(ISS& env, bool, MOpMode mode, Type key) {
  auto const name     = mStringKey(key);
  auto const isDefine = mode == MOpMode::Define;
  auto const isUnset  = mode == MOpMode::Unset;
  // PHP5 doesn't modify an unset local if you unset a property or
  // array elem on it, but hhvm does (it promotes it to init-null).
  auto const update = isDefine || isUnset;
  /*
   * MOpMode::Unset Props doesn't promote "emptyish" things to stdClass, or
   * affect arrays, however it can define a property on an object base.  This
   * means we don't need any couldBeInFoo logic, but if the base could actually
   * be $this, and a declared property could be Uninit, we need to merge
   * InitNull.
   *
   * We're trying to handle this case correctly as far as the type inference
   * here is concerned, but the runtime doesn't actually behave this way right
   * now for declared properties.  Note that it never hurts to merge more types
   * than a thisProp could actually be, so this is fine.
   *
   * See TODO(#3602740): unset with intermediate dims on previously declared
   * properties doesn't define them to null.
   */
  if (isUnset && couldBeThisObj(env, env.collect.mInstrState.base)) {
    if (name) {
      auto const elem = thisPropRaw(env, name);
      if (elem && elem->ty.couldBe(BUninit)) {
        mergeThisProp(env, name, TInitNull);
      }
    } else {
      mergeEachThisPropRaw(env, [&] (const Type& ty) {
        return ty.couldBe(BUninit) ? TInitNull : TBottom;
      });
    }
  }

  if (mustBeThisObj(env, env.collect.mInstrState.base)) {
    auto const optThisTy = thisTypeFromContext(env.index, env.ctx);
    auto const thisTy    = optThisTy ? *optThisTy : TObj;
    if (name) {
      auto const ty = [&] {
        if (update) {
          if (auto const elem = thisPropRaw(env, name)) return elem->ty;
        } else {
          if (auto const propTy = thisPropAsCell(env, name)) return *propTy;
        }
        auto const raw =
          env.index.lookup_public_prop(objcls(thisTy), sval(name));
        return update ? raw : to_cell(raw);
      }();

      if (ty.subtypeOf(BBottom)) return Effects::AlwaysThrows;
      moveBase(
        env,
        Base { ty, BaseLoc::Prop, thisTy, name },
        update
      );
    } else {
      moveBase(env,
               Base { TInitCell, BaseLoc::Prop, thisTy },
               update);
    }
    return Effects::Throws;
  }

  // We know for sure we're going to be in an object property.
  if (env.collect.mInstrState.base.type.subtypeOf(BObj)) {
    auto const raw =
      env.index.lookup_public_prop(
        objcls(env.collect.mInstrState.base.type),
        name ? sval(name) : TStr
      );
    auto const ty = update ? raw : to_cell(raw);
    if (ty.subtypeOf(BBottom)) return Effects::AlwaysThrows;
    moveBase(env,
             Base { ty,
                    BaseLoc::Prop,
                    env.collect.mInstrState.base.type,
                    name },
             update);
    return Effects::Throws;
  }

  /*
   * Otherwise, intermediate props with define can promote a null, false, or ""
   * to stdClass.  Those cases, and others, if it's MOpMode::Define, will set
   * the base to a null value in tvScratch.  The base may also legitimately be
   * an object and our next base is in an object property. Conservatively treat
   * all these cases as "possibly" being inside of an object property with
   * "Prop" with locType TTop.
   */
  moveBase(env,
           Base { TInitCell, BaseLoc::Prop, TTop, name },
           update);
  return Effects::Throws;
}

Effects miElem(ISS& env, MOpMode mode, Type key, LocalId keyLoc) {
  auto const isDefine = mode == MOpMode::Define;
  auto const isUnset  = mode == MOpMode::Unset;

  if (!isDefine && !isUnset) {
    // An Elem operation which doesn't mutate the base at all
    auto elem = elemHelper(env, mode, std::move(key));
    if (elem.second != Effects::AlwaysThrows) {
      // Since we're not mutating the base, we don't need to use an
      // array chain here.
      moveBase(
        env, Base { std::move(elem.first), BaseLoc::Elem }, false, keyLoc
      );
    }
    return elem.second;
  }

  // ElemD or ElemU. The base might mutate here. First handle any base
  // promotions.
  auto const promo = handleElemUDBasePromos(env);

  auto& base = env.collect.mInstrState.base.type;

  // These are similar to endBaseWithEffects, but moves the base or
  // extends the array chain instead.
  auto const move = [&] (Type ty, bool update, Effects effects) {
    assertx(effects != Effects::AlwaysThrows);
    auto const effectFree = moveBase(
      env,
      Base { std::move(ty), BaseLoc::Elem },
      update || promo != Promotion::No,
      keyLoc
    );
    if (promo == Promotion::YesMightThrow) return Effects::Throws;
    if (!effectFree && effects == Effects::None) return Effects::SideEffect;
    return effects;
  };
  auto const extend = [&] (Type ty, Effects effects) {
    assertx(effects != Effects::AlwaysThrows);
    auto const effectFree =
      extendArrChain(env, std::move(key), base, std::move(ty), true, keyLoc);
    if (promo == Promotion::YesMightThrow) return Effects::Throws;
    if (!effectFree && effects == Effects::None) return Effects::SideEffect;
    return effects;
  };
  auto const unreachable = [&] {
    return endUnreachableBase(env, promo, keyLoc);
  };

  if (isUnset) {
    // ElemU. These types either always throw, or set the base to
    // Uninit.
    auto const alwaysThrows =
      BClsLike | BFuncLike | BStr | BClsMethLike | BRecord | BKeyset;
    auto const movesToUninit = BPrim | BRes;

    if (base.subtypeOf(alwaysThrows | movesToUninit)) {
      if (base.subtypeOf(alwaysThrows)) return unreachable();
      return move(
        TUninit, false,
        base.couldBe(alwaysThrows) ? Effects::Throws : Effects::None
      );
    }

    // Normally we allow OptArrLike for array_do_elem, but only
    // ArrLike here. We don't want to deal with potential nulls in the
    // array chain.
    if (!base.couldBe(BNull)) {
      if (auto elem = array_do_elem(env, false, key)) {
        if (elem->second == TriBool::Yes) return unreachable();
        handleElemUDEffects(env); // Handle COW
        return extend(
          std::move(elem->first),
          elem->second == TriBool::Maybe ? Effects::Throws : Effects::None
        );
      }
    }
  } else {
    assertx(isDefine);

    // ElemD. These types either always throw, or emit a warning and
    // push InitNull.
    auto const alwaysThrows = BNull | BFalse | BStr | BKeyset;
    auto const warnsAndNull =
      BTrue | BNum | BRes | BFuncLike | BClsLike | BClsMethLike;

    if (base.subtypeOf(alwaysThrows | warnsAndNull)) {
      if (base.subtypeOf(alwaysThrows)) return unreachable();
      return move(TInitNull, false, Effects::Throws);
    }

    // We allow OptArrLike here because we'll remove the optness
    // anyways (see below).
    if (auto elem = array_do_elem(env, false, key)) {
      if (elem->second == TriBool::Yes) return unreachable();
      auto const couldBeNull = base.couldBe(BNull);
      // If it was null, we'd have fataled, so it cannot be null
      // afterwards.
      if (couldBeNull) base = unopt(std::move(base));
      handleElemUDEffects(env); // Handle COW
      return extend(
        std::move(elem->first),
        (couldBeNull || elem->second == TriBool::Maybe)
          ? Effects::Throws
          : Effects::None
      );
    }
  }

  // Something else. Be pessimistic.
  pessimizeBaseForElemUD(env);
  return move(TInitCell, true, Effects::Throws);
}

Effects miNewElem(ISS& env) {
  // NewElem. These all either throw or raise a warning and set the
  // base to Uninit.
  auto const& base = env.collect.mInstrState.base.type;
  if (base.subtypeOf(BNull | BFalse | BArrLike | BObj | BClsMeth | BRecord) ||
      base.subtypeOf(sempty())) {
    return Effects::AlwaysThrows;
  }
  moveBase(env, Base { TUninit, BaseLoc::Elem }, false);
  return Effects::Throws;
}

//////////////////////////////////////////////////////////////////////
// final prop ops

Effects miFinalIssetProp(ISS& env, int32_t nDiscard, const Type& key) {
  auto const name = mStringKey(key);
  discard(env, nDiscard);

  if (name && mustBeThisObj(env, env.collect.mInstrState.base)) {
    if (auto const pt = thisPropAsCell(env, name)) {
      if (isMaybeLateInitThisProp(env, name)) {
        // LateInit props can always be maybe unset, except if its never set at
        // all.
        push(env, pt->subtypeOf(BBottom) ? TFalse : TBool);
      } else if (pt->subtypeOf(BNull)) {
        push(env, TFalse);
      } else if (!pt->couldBe(BNull)) {
        push(env, TTrue);
      } else {
        push(env, TBool);
      }
      return Effects::None;
    }
  }

  push(env, TBool);
  return Effects::Throws;
}

Effects miFinalCGetProp(ISS& env, int32_t nDiscard,
                        const Type& key, bool quiet) {
  auto const name = mStringKey(key);
  discard(env, nDiscard);

  if (name) {
    if (mustBeThisObj(env, env.collect.mInstrState.base)) {
      if (auto const t = thisPropAsCell(env, name)) {
        push(env, *t);
        if (t->subtypeOf(BBottom)) return Effects::AlwaysThrows;
        if (isMaybeLateInitThisProp(env, name)) return Effects::Throws;
        if (quiet) return Effects::None;
        auto const elem = thisPropRaw(env, name);
        assertx(elem);
        return elem->ty.couldBe(BUninit) ? Effects::Throws : Effects::None;
      }
    }
    auto ty = to_cell(
      env.index.lookup_public_prop(
        objcls(env.collect.mInstrState.base.type), sval(name)
      )
    );
    push(env, std::move(ty));
    return ty.subtypeOf(BBottom) ? Effects::AlwaysThrows : Effects::Throws;
  }

  push(env, TInitCell);
  return Effects::Throws;
}

Effects miFinalSetProp(ISS& env, int32_t nDiscard, const Type& key) {
  auto const name = mStringKey(key);
  auto const t1 = unctx(popC(env));

  auto const finish = [&](Type ty) {
    endBase(env);
    discard(env, nDiscard);
    push(env, std::move(ty));
    return Effects::Throws;
  };

  if (couldBeThisObj(env, env.collect.mInstrState.base)) {
    if (!name) {
      mergeEachThisPropRaw(
        env,
        [&] (Type propTy) {
          return propTy.couldBe(BInitCell) ? t1 : TBottom;
        }
      );
    } else {
      mergeThisProp(env, name, t1);
    }
  }

  if (env.collect.mInstrState.base.type.subtypeOf(BObj)) {
    if (t1.subtypeOf(BBottom)) {
      discard(env, nDiscard);
      push(env, TBottom);
      return Effects::AlwaysThrows;
    }
    moveBase(
      env,
      Base { t1, BaseLoc::Prop, env.collect.mInstrState.base.type, name }
    );
    return finish(t1);
  }

  moveBase(env, Base { TInitCell, BaseLoc::Prop, TTop, name });
  return finish(TInitCell);
}

Effects miFinalSetOpProp(ISS& env, int32_t nDiscard,
                      SetOpOp subop, const Type& key) {
  auto const name = mStringKey(key);
  auto const rhsTy = popC(env);

  auto const lhsTy = [&] {
    if (name) {
      if (mustBeThisObj(env, env.collect.mInstrState.base)) {
        if (auto const t = thisPropAsCell(env, name)) return *t;
      }
      return to_cell(
        env.index.lookup_public_prop(
          objcls(env.collect.mInstrState.base.type), sval(name)
        )
      );
    }
    return TInitCell;
  }();
  if (lhsTy.subtypeOf(BBottom)) {
    discard(env, nDiscard);
    push(env, TBottom);
    return Effects::AlwaysThrows;
  }

  auto const resultTy = env.collect.mInstrState.base.type.subtypeOf(TObj)
    ? typeSetOp(subop, lhsTy, rhsTy)
    : TInitCell;

  if (resultTy.subtypeOf(BBottom)) {
    discard(env, nDiscard);
    push(env, TBottom);
    return Effects::AlwaysThrows;
  }

  if (couldBeThisObj(env, env.collect.mInstrState.base)) {
    if (name) {
      mergeThisProp(env, name, resultTy);
    } else {
      killThisProps(env);
    }
  }

  endBase(env);
  discard(env, nDiscard);
  push(env, resultTy);
  return Effects::Throws;
}

Effects miFinalIncDecProp(ISS& env, int32_t nDiscard,
                          IncDecOp subop, const Type& key) {
  auto const name = mStringKey(key);

  auto const postPropTy = [&] {
    if (name) {
      if (mustBeThisObj(env, env.collect.mInstrState.base)) {
        if (auto const t = thisPropAsCell(env, name)) return *t;
      }
      return to_cell(
        env.index.lookup_public_prop(
          objcls(env.collect.mInstrState.base.type), sval(name)
        )
      );
    }
    return TInitCell;
  }();
  if (postPropTy.subtypeOf(BBottom)) {
    discard(env, nDiscard);
    push(env, TBottom);
    return Effects::AlwaysThrows;
  }

  auto const prePropTy = env.collect.mInstrState.base.type.subtypeOf(TObj)
    ? typeIncDec(subop, postPropTy)
    : TInitCell;
  if (prePropTy.subtypeOf(BBottom)) {
    discard(env, nDiscard);
    push(env, TBottom);
    return Effects::AlwaysThrows;
  }

  if (couldBeThisObj(env, env.collect.mInstrState.base)) {
    if (name) {
      mergeThisProp(env, name, prePropTy);
    } else {
      killThisProps(env);
    }
  }

  endBase(env);
  discard(env, nDiscard);
  push(env, isPre(subop) ? prePropTy : postPropTy);
  return Effects::Throws;
}

Effects miFinalUnsetProp(ISS& env, int32_t nDiscard, const Type& key) {
  if (couldBeThisObj(env, env.collect.mInstrState.base)) {
    if (auto const name = mStringKey(key)) {
      unsetThisProp(env, name);
    } else {
      unsetUnknownThisProp(env);
    }
  }

  endBase(env);
  discard(env, nDiscard);
  return Effects::Throws;
}

//////////////////////////////////////////////////////////////////////
// Final elem ops

Effects miFinalCGetElem(ISS& env, int32_t nDiscard,
                        const Type& key, MOpMode mode) {
  auto elem = elemHelper(env, mode, key);
  discard(env, nDiscard);
  push(
    env,
    elem.second == Effects::AlwaysThrows
      ? TBottom
      : std::move(elem.first)
  );
  return elem.second;
}

Effects miFinalIssetElem(ISS& env, int32_t nDiscard, const Type& key) {
  auto const& base = env.collect.mInstrState.base.type;

  auto const finish = [&] (Type ty, Effects effects) {
    discard(env, nDiscard);
    push(env, std::move(ty));
    return effects;
  };

  auto const pushesFalse = BNull | BBool | BNum | BRes | BFuncLike | BRClsMeth |
    (!RO::EvalIsCompatibleClsMethType ? BClsMeth : BBottom);
  if (base.subtypeOf(pushesFalse)) return finish(TFalse, Effects::None);

  if (auto elem = array_do_elem(env, true, key)) {
    if (elem->second == TriBool::Yes) {
      if (base.couldBe(BNull)) return finish(TFalse, Effects::Throws);
      return finish(TBottom, Effects::AlwaysThrows);
    }
    auto const effects = (elem->second == TriBool::Maybe)
      ? Effects::Throws
      : Effects::None;

    if (elem->first.subtypeOf(BNull)) return finish(TFalse, effects);
    if (!elem->first.couldBe(BNull) && !base.couldBe(BNull)) {
      return finish(TTrue, effects);
    }
    return finish(TBool, effects);
  }

  return finish(TBool, Effects::Throws);
}

Effects miFinalSetElem(ISS& env,
                       int32_t nDiscard,
                       const Type& key,
                       LocalId keyLoc) {
  auto rhs = popC(env);

  // First handle base promotions
  auto const promo = handleElemUDBasePromos(env);

  auto& base = env.collect.mInstrState.base.type;

  auto const stack = [&] (Type ty, Effects effects) {
    discard(env, nDiscard);
    push(env, std::move(ty));
    return effects;
  };

  auto const end = [&] (Type ty, bool update, Effects effects) {
    return stack(
      std::move(ty),
      endBaseWithEffects(env, effects, update, promo, keyLoc)
    );
  };
  auto const unreachable = [&] {
    return stack(
      TBottom,
      endUnreachableBase(env, promo, keyLoc)
    );
  };

  // These will either always throw, or raise a warning and push null.
  auto const alwaysThrows = BNull | BFalse | BKeyset;
  auto const pushesNull =
    BTrue | BNum | BRes | BFuncLike | BClsLike | BClsMethLike;
  if (base.subtypeOf(alwaysThrows | pushesNull)) {
    if (base.subtypeOf(alwaysThrows)) return unreachable();
    return end(TInitNull, false, Effects::Throws);
  }

  // String is a special case here. It will throw on empty strings,
  // and otherwise can return null or a static string (and possibly
  // warn). We don't bother predicting the effects of the set on the
  // string here, so be pessimistic and forget what we know about the
  // base in regards to staticness and values.
  if (base.subtypeOf(BOptStr)) {
    if (base.subtypeOf(sempty())) return unreachable();
    base = loosen_staticness(loosen_values(std::move(base)));
    return end(TOptSStr, true, Effects::Throws);
  }

  if (auto const set = array_do_set(env, key, rhs)) {
    if (*set == TriBool::Yes) return unreachable();
    auto const couldBeNull = base.couldBe(BNull);
    // If it was null, we'd have fataled, so it cannot be null
    // afterwards.
    if (couldBeNull) base = unopt(std::move(base));
    return end(
      std::move(rhs),
      true,
      (couldBeNull || *set == TriBool::Maybe) ? Effects::Throws : Effects::None
    );
  }

  // In most cases, the right hand side of the set is what's pushed
  // onto the stack. The exceptions are the special types above which
  // push null, or the string case described above.
  auto lhs = std::move(rhs);
  if (base.couldBe(pushesNull)) lhs = union_of(std::move(lhs), TInitNull);
  if (base.couldBe(BStr))       lhs = union_of(std::move(lhs), TOptSStr);

  // We can't determine how the base is changing here, so be
  // pessimistic and drop values and staticness.
  base = loosen_staticness(loosen_values(std::move(base)));
  return end(std::move(lhs), true, Effects::Throws);
}

Effects miFinalSetOpElem(ISS& env, int32_t nDiscard,
                         SetOpOp subop, const Type& key,
                         LocalId keyLoc) {
  auto const rhsTy = popC(env);
  return setOpElemHelper(
    env, nDiscard, key, keyLoc,
    [&] (const Type& lhsTy) {
      auto const result = typeSetOp(subop, lhsTy, rhsTy);
      return std::make_tuple(result, result, Effects::Throws);
    }
  );
}

Effects miFinalIncDecElem(ISS& env, int32_t nDiscard,
                          IncDecOp subop, const Type& key,
                          LocalId keyLoc) {
  return setOpElemHelper(
    env, nDiscard, key, keyLoc,
    [&] (const Type& before) {
      auto const after = typeIncDec(subop, before);
      return std::make_tuple(
        after,
        isPre(subop) ? after : before,
        before.subtypeOf(BNum) ? Effects::None : Effects::Throws
      );
    }
  );
}

Effects miFinalUnsetElem(ISS& env, int32_t nDiscard, const Type& key) {
  // First handle base promotions
  auto const promo = handleElemUDBasePromos(env);

  auto& base = env.collect.mInstrState.base.type;

  auto const end = [&] (bool update, Effects effects) {
    auto const e = endBaseWithEffects(env, effects, update, promo);
    discard(env, nDiscard);
    return e;
  };

  // These either always throw, or silently does nothing (ElemUnset
  // doesn't push anything and ends the base, so there's nothing else
  // to do).
  auto const doesNothing = BNull | BBool | BNum | BRes;
  auto const alwaysThrows =
    BFuncLike | BClsLike | BStr | BRecord | BClsMethLike;
  if (base.subtypeOf(doesNothing | alwaysThrows)) {
    if (base.subtypeOf(alwaysThrows)) {
      auto const e = endUnreachableBase(env, promo);
      discard(env, nDiscard);
      return e;
    }
    return end(
      false,
      base.subtypeOf(doesNothing) ? Effects::None : Effects::Throws
    );
  }

  // Special case for varray or vec with string keys. These silently
  // do nothing.
  if (base.subtypeOf(BOptVecish)) {
    if (key.subtypeOf(BStr)) return end(false, Effects::None);
  }

  // Unset doesn't throw for a missing element on dicts, keysets, or
  // darrays, only if the key is invalid. Vecs and varrays silently do
  // nothing for string keys, but can throw with int keys.
  auto const noThrow = base.subtypeOf(
    doesNothing |
    (key.subtypeOf(BArrKey) ? (BDictish | BKeyset) : BBottom) |
    (key.subtypeOf(BStr) ? BVecish : BBottom)
  );

  // We purposefully do not model the effects of unset on array
  // structure. This lets us assume that if we have array structure,
  // we also have no tombstones. Pessimize the base which drops array
  // structure and also remove emptiness information.
  pessimizeBaseForElemUD(env);
  base = loosen_emptiness(std::move(base));
  return end(true, noThrow ? Effects::None : Effects::Throws);
}

//////////////////////////////////////////////////////////////////////
// Final new elem ops

Effects miFinalSetNewElem(ISS& env, int32_t nDiscard) {
  auto rhs = popC(env);

  // First handle base promotions
  auto const promo = handleElemUDBasePromos(env);

  auto& base = env.collect.mInstrState.base.type;

  auto const stack = [&] (Type ty, Effects effects) {
    discard(env, nDiscard);
    push(env, std::move(ty));
    return effects;
  };

  // These all either throw or warn and push null.
  auto const pushesNull =
    BTrue | BNum | BRes | BFuncLike | BClsLike | BClsMethLike;
  auto const throws = BNull | BStr | BRecord | BFalse;

  if (base.subtypeOf(pushesNull | throws)) {
    if (base.subtypeOf(throws)) {
      return stack(TBottom, endUnreachableBase(env, promo));
    }
    return stack(
      TInitNull,
      endBaseWithEffects(env, Effects::Throws, false, promo)
    );
  }

  // Arrays will add a new element and push the right hand side of the
  // assignment.
  auto const couldBeNull = base.couldBe(BInitNull);
  if (auto doesThrow = array_do_newelem(env, rhs)) {
    if (*doesThrow == TriBool::Yes) {
      return stack(TBottom, endUnreachableBase(env, promo));
    }
    // If it was null, we'd have fataled, so it cannot be null
    // afterwards.
    if (couldBeNull) base = unopt(std::move(base));

    return stack(
      std::move(rhs),
      endBaseWithEffects(
        env,
        *doesThrow == TriBool::No && !couldBeNull
          ? Effects::None
          : Effects::Throws,
        true,
        promo
      )
    );
  }

  // We'll push the right hande side of the assignment in every case,
  // except for the special cases above which will push null instead,
  // so take that into account.
  auto ty = base.couldBe(pushesNull)
    ? union_of(std::move(rhs), TInitNull)
    : std::move(rhs);

  // We don't know what we're doing to the base, so pessimize it. In
  // addition, we're potentially adding new elements, so we need to
  // add non-emptiness bits.
  pessimizeBaseForElemUD(env);
  base = add_nonemptiness(std::move(base));

  return stack(
    std::move(ty),
    endBaseWithEffects(env, Effects::Throws, true, promo)
  );
}

Effects miFinalSetOpNewElem(ISS& env, int32_t nDiscard) {
  popC(env);
  return setOpNewElemHelper(env, nDiscard);
}

Effects miFinalIncDecNewElem(ISS& env, int32_t nDiscard) {
  return setOpNewElemHelper(env, nDiscard);
}

//////////////////////////////////////////////////////////////////////

// Translate the aggregated effects of the instruction into the
// appropriate interp-state actions. Returns true if the instruction
// is totally effect-free.
bool handleEffects(ISS& env, Effects effects, Promotion keyPromotion) {
  switch (effects) {
    case Effects::None:
      if (keyPromotion == Promotion::YesMightThrow) return false;
      effect_free(env);
      return true;
    case Effects::SideEffect:
      if (keyPromotion != Promotion::YesMightThrow) nothrow(env);
      return false;
    case Effects::Throws:
      return false;
    case Effects::AlwaysThrows:
      unreachable(env);
      return false;
  }
  always_assert(false);
}

}

namespace interp_step {

//////////////////////////////////////////////////////////////////////
// Base operations

void in(ISS& env, const bc::BaseGC& op) {
  startBase(env, Base{TInitCell, BaseLoc::Global});
}

void in(ISS& env, const bc::BaseGL& op) {
  mayReadLocal(env, op.loc1);
  startBase(env, Base{TInitCell, BaseLoc::Global});
}

void in(ISS& env, const bc::BaseSC& op) {
  auto tcls = topC(env, op.arg2);
  auto const tname = topC(env, op.arg1);

  // We'll raise an error if its not a class
  if (!tcls.couldBe(BCls)) return unreachable(env);

  // Lookup what we know about the property
  auto lookup = env.index.lookup_static(
    env.ctx,
    env.collect.props,
    tcls,
    tname
  );

  // If we definitely didn't find anything, we'll definitely throw
  if (lookup.found == TriBool::No || lookup.ty.subtypeOf(BBottom)) {
    return unreachable(env);
  }

  // Whether we might potentially throw because of AttrConst
  auto mightConstThrow = false;
  switch (op.subop3) {
    case MOpMode::Define:
    case MOpMode::Unset:
    case MOpMode::InOut:
      // If its definitely const, we'll always throw. Otherwise we'll
      // potentially throw if there's a chance its AttrConst.
      if (lookup.isConst == TriBool::Yes) return unreachable(env);
      mightConstThrow = lookup.isConst == TriBool::Maybe;
      break;
    case MOpMode::None:
    case MOpMode::Warn:
      // These don't mutate the base, so AttrConst does not apply
      break;
  }

  // Loading the base from a static property can be considered
  // effect_free if there's no possibility of throwing. This requires
  // a definitely found, non-AttrLateInit property with normal class
  // initialization, and both the class and name have to be the normal
  // types.
  if (lookup.found == TriBool::Yes &&
      lookup.lateInit == TriBool::No &&
      !lookup.classInitMightRaise &&
      !mightConstThrow &&
      tcls.subtypeOf(BCls) &&
      tname.subtypeOf(BStr)) {

    // If we're not mutating the base, and the base is a constant,
    // turn it into a BaseC with the appropriate constant on the
    // stack.
    if (op.subop3 == MOpMode::Warn || op.subop3 == MOpMode::None) {
      if (auto const v = tv(lookup.ty)) {
        reduce(env, gen_constant(*v), bc::BaseC { 0, op.subop3 });
        env.collect.mInstrState.extraPop = true;
        return;
      }
    }

    effect_free(env);
  }

  return startBase(
    env,
    Base {
      std::move(lookup.ty),
      BaseLoc::StaticProp,
      std::move(tcls),
      lookup.name
    }
  );
}

void in(ISS& env, const bc::BaseL& op) {
  auto ty = peekLocRaw(env, op.nloc1.id);

  // An Uninit local base can raise a notice.
  if (!ty.couldBe(BUninit)) {
    // If we're not mutating the base, and the base is a constant,
    // turn it into a BaseC with the appropriate constant on the
    // stack.
    if (op.subop2 == MOpMode::Warn || op.subop2 == MOpMode::None) {
      if (auto const v = tv(ty)) {
        reduce(env, gen_constant(*v), bc::BaseC { 0, op.subop2 });
        env.collect.mInstrState.extraPop = true;
        return;
      }

      // Try to find an equivalent local to use instead
      auto const minLocEquiv = findMinLocEquiv(env, op.nloc1.id, false);
      if (minLocEquiv != NoLocalId) {
        return reduce(
          env,
          bc::BaseL {
            NamedLocal { kInvalidLocalName, minLocEquiv },
            op.subop2
          }
        );
      }
    }

    effect_free(env);
  } else if (op.subop2 != MOpMode::Warn) {
    // The local could be Uninit, but we won't warn about it anyways.
    effect_free(env);
  }

  mayReadLocal(env, op.nloc1.id);
  if (ty.subtypeOf(BBottom)) return unreachable(env);

  startBase(
    env,
    Base {
      std::move(ty),
      BaseLoc::Local,
      TBottom,
      op.nloc1.name != kInvalidLocalName
        ? env.ctx.func->locals[op.nloc1.name].name
        : nullptr,
      op.nloc1.id
    }
  );
}

void in(ISS& env, const bc::BaseC& op) {
  assert(op.arg1 < env.state.stack.size());
  auto ty = topC(env, op.arg1);
  if (ty.subtypeOf(BBottom)) return unreachable(env);
  effect_free(env);
  startBase(
    env,
    Base {
      std::move(ty),
      BaseLoc::Stack,
      TBottom,
      SString{},
      NoLocalId,
      (uint32_t)env.state.stack.size() - op.arg1 - 1
    }
  );
}

void in(ISS& env, const bc::BaseH&) {
  auto const ty = thisTypeNonNull(env);
  if (ty.subtypeOf(BBottom)) return unreachable(env);
  effect_free(env);
  startBase(env, Base{ty, BaseLoc::This});
}

//////////////////////////////////////////////////////////////////////
// Intermediate operations

void in(ISS& env, const bc::Dim& op) {
  auto key = key_type_or_fixup(env, op);
  if (!key) return;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      return miProp(
        env, op.mkey.mcode == MQT, op.subop1, std::move(key->first)
      );
    } else if (mcodeIsElem(op.mkey.mcode)) {
      return miElem(env, op.subop1, std::move(key->first), key_local(env, op));
    } else {
      return miNewElem(env);
    }
  }();

  if (effects != Effects::None) {
    env.collect.mInstrState.effectFree = false;
  }
  if (!handleEffects(env, effects, key->second)) return;

  // This instruction must be effect free
  assertx(env.flags.effectFree);
  assertx(!env.state.unreachable);

  // If the base is a constant, and we're not mutating the base, and
  // if the entire minstr sequence up until now has been effect-free,
  // we can remove the entire sequence up until now. We replace it
  // with just a BaseC on the constant pushed onto the stack.
  if ((op.subop1 == MOpMode::None || op.subop1 == MOpMode::Warn) &&
      env.collect.mInstrState.effectFree &&
      will_reduce(env) &&
      is_scalar(env.collect.mInstrState.base.type)) {
    // Find the base instruction which started the sequence.
    for (int i = 0; ; i++) {
      auto const last = last_op(env, i);
      if (!last) break;
      if (isMemberBaseOp(last->op)) {
        auto const base = *last;
        rewind(env, i + 1);
        // We'll need to push the constant onto the stack. If the
        // sequence originally started with a BaseC (or BaseGC)
        // instruction, we can just pop off the original value and
        // replace it with the constant. This leaves all offsets the
        // same. If not, we push the constant and set extraPop, which
        // makes us increment all of the offsets when we reprocess
        // them.
        auto const reuseStack =
          [&] {
            switch (base.op) {
              case Op::BaseGC: return base.BaseGC.arg1 == 0;
              case Op::BaseC:  return base.BaseC.arg1 == 0;
              default: return false;
            }
          }();
        assertx(!env.collect.mInstrState.extraPop || reuseStack);
        auto const extraPop = !reuseStack || env.collect.mInstrState.extraPop;
        env.collect.mInstrState.clear();
        if (reuseStack) reduce(env, bc::PopC {});
        auto const v = tv(env.collect.mInstrState.base.type);
        assertx(v);
        reduce(env, gen_constant(*v), bc::BaseC { 0, op.subop1 });
        env.collect.mInstrState.extraPop = extraPop;
        return;
      }
      if (!isMemberDimOp(last->op)) break;
    }
  }
}

//////////////////////////////////////////////////////////////////////
// Final operations

const StaticString s_classname("classname");
const StaticString s_type_structure("HH\\type_structure");
const StaticString s_type_structure_classname("HH\\type_structure_classname");

void in(ISS& env, const bc::QueryM& op) {
  auto const key = key_type_or_fixup(env, op);
  if (!key) return;
  auto const nDiscard = op.arg1;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      // We don't currently do anything different for nullsafe query ops.
      switch (op.subop2) {
        case QueryMOp::CGet:
        case QueryMOp::CGetQuiet:
          return miFinalCGetProp(env, nDiscard, key->first,
                                 op.subop2 == QueryMOp::CGetQuiet);
        case QueryMOp::Isset:
          return miFinalIssetProp(env, nDiscard, key->first);
        case QueryMOp::InOut:
          always_assert(false);
      }
      always_assert(false);
    } else if (mcodeIsElem(op.mkey.mcode)) {
      switch (op.subop2) {
        case QueryMOp::InOut:
        case QueryMOp::CGet:
        case QueryMOp::CGetQuiet:
          return miFinalCGetElem(
            env, nDiscard, key->first, getQueryMOpMode(op.subop2)
          );
        case QueryMOp::Isset:
          return miFinalIssetElem(env, nDiscard, key->first);
      }
      always_assert(false);
    } else {
      // QueryMNewElem will always throw without doing any work.
      discard(env, nDiscard);
      push(env, TBottom);
      return Effects::AlwaysThrows;
    }
  }();

  // Try to detect type_structure(cls_name, cns_name)['classname'] and
  // reduce this to type_structure_classname(cls_name, cns_name)
  if (mcodeIsElem(op.mkey.mcode) &&
      op.subop2 == QueryMOp::CGet &&
      nDiscard == 1 &&
      op.mkey.mcode == MemberCode::MET &&
      op.mkey.litstr->isame(s_classname.get())) {
    if (auto const last = last_op(env, 0)) {
      if (last->op == Op::BaseC) {
        if (auto const prev = last_op(env, 1)) {
          if (prev->op == Op::FCallFuncD &&
              prev->FCallFuncD.str2->isame(s_type_structure.get()) &&
              prev->FCallFuncD.fca.numArgs() == 2) {
            auto const params = prev->FCallFuncD.fca.numArgs();
            rewind(env, op); // querym
            rewind(env, 2);  // basec + fcallfuncd
            env.collect.mInstrState.clear();
            return reduce(
              env,
              bc::FCallFuncD {
                FCallArgs(params),
                s_type_structure_classname.get()
              }
            );
          }
        }
      }
    }
  }

  if (effects != Effects::None) env.collect.mInstrState.effectFree = false;

  // For the QueryM ops, its our responsibility to call endBase()
  // (unless we'll always throw).

  if (!handleEffects(env, effects, key->second)) {
    if (effects != Effects::AlwaysThrows) endBase(env, false);
    return;
  }

  assertx(env.flags.effectFree);
  assertx(!env.state.unreachable);

  // If the QueryM produced a constant without any possible
  // side-ffects, we can replace the entire thing with the constant.
  if (env.collect.mInstrState.effectFree && is_scalar(topC(env))) {
    for (int i = 0; ; i++) {
      auto const last = last_op(env, i);
      if (!last) break;
      if (isMemberBaseOp(last->op)) {
        auto const v = tv(topC(env));
        rewind(env, op);
        rewind(env, i + 1);
        env.collect.mInstrState.clear();
        BytecodeVec bcs{nDiscard, bc::PopC{}};
        bcs.push_back(gen_constant(*v));
        return reduce(env, std::move(bcs));
      }
      if (!isMemberDimOp(last->op)) break;
    }
  }
  endBase(env, false);
}

void in(ISS& env, const bc::SetM& op) {
  auto const key = key_type_or_fixup(env, op);
  if (!key) return;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      return miFinalSetProp(env, op.arg1, key->first);
    } else if (mcodeIsElem(op.mkey.mcode)) {
      return miFinalSetElem(env, op.arg1, key->first, key_local(env, op));
    } else {
      return miFinalSetNewElem(env, op.arg1);
    }
  }();
  handleEffects(env, effects, key->second);
}

void in(ISS& env, const bc::SetRangeM& op) {
  popC(env);
  popC(env);
  popC(env);
  discard(env, op.arg1);
  auto& base = env.collect.mInstrState.base.type;
  if (!base.couldBe(BStr)) return unreachable(env);
  base = loosen_staticness(loosen_values(std::move(base)));
  endBase(env);
}

void in(ISS& env, const bc::IncDecM& op) {
  auto const key = key_type_or_fixup(env, op);
  if (!key) return;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      return miFinalIncDecProp(env, op.arg1, op.subop2, key->first);
    } else if (mcodeIsElem(op.mkey.mcode)) {
      return miFinalIncDecElem(
        env, op.arg1, op.subop2, key->first, key_local(env, op)
      );
    } else {
      return miFinalIncDecNewElem(env, op.arg1);
    }
  }();
  handleEffects(env, effects, key->second);
}

void in(ISS& env, const bc::SetOpM& op) {
  auto const key = key_type_or_fixup(env, op);
  if (!key) return;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      return miFinalSetOpProp(env, op.arg1, op.subop2, key->first);
    } else if (mcodeIsElem(op.mkey.mcode)) {
      return miFinalSetOpElem(
        env, op.arg1, op.subop2, key->first, key_local(env, op)
      );
    } else {
      return miFinalSetOpNewElem(env, op.arg1);
    }
  }();
  handleEffects(env, effects, key->second);
}

void in(ISS& env, const bc::UnsetM& op) {
  auto const key = key_type_or_fixup(env, op);
  if (!key) return;

  auto const effects = [&] {
    if (mcodeIsProp(op.mkey.mcode)) {
      return miFinalUnsetProp(env, op.arg1, key->first);
    } else {
      assert(mcodeIsElem(op.mkey.mcode));
      return miFinalUnsetElem(env, op.arg1, key->first);
    }
  }();
  handleEffects(env, effects, key->second);
}

}

//////////////////////////////////////////////////////////////////////

}}
