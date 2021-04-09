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

#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen.h"

#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/vm/native-prop-handler.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-arith.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

#include "hphp/util/safe-cast.h"
#include "hphp/util/struct-log.h"

#include <folly/Optional.h>

#include <sstream>

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

enum class SimpleOp {
  None,
  Vec,
  Dict,
  Keyset,
  String,
  Vector, // c_Vector* or c_ImmVector*
  Map,    // c_Map*
  Pair,   // c_Pair*
};

//////////////////////////////////////////////////////////////////////
// Property information.

struct PropInfo {
  using UpperBoundVec = PreClass::UpperBoundVec;
  explicit PropInfo(Slot slot,
                    uint16_t index,
                    bool isConst,
                    bool readOnly,
                    bool lateInit,
                    bool lateInitCheck,
                    Type knownType,
                    const HPHP::TypeConstraint* typeConstraint,
                    const UpperBoundVec* ubs,
                    const Class* objClass,
                    const Class* propClass)
    : slot{slot}
    , index{index}
    , isConst{isConst}
    , readOnly{readOnly}
    , lateInit{lateInit}
    , lateInitCheck{lateInitCheck}
    , knownType{std::move(knownType)}
    , typeConstraint{typeConstraint}
    , ubs{ubs}
    , objClass{objClass}
    , propClass{propClass}
  {}

  Slot slot{kInvalidSlot};
  uint16_t index{0};
  bool isConst{false};
  bool readOnly{false};
  bool lateInit{false};
  bool lateInitCheck{false};
  Type knownType{TCell};
  const HPHP::TypeConstraint* typeConstraint{nullptr};
  const UpperBoundVec* ubs{nullptr};
  const Class* objClass{nullptr};
  const Class* propClass{nullptr};
};

Type knownTypeForProp(const Class::Prop& prop,
                      const Class* propCls,
                      const Class* ctx,
                      bool ignoreLateInit) {
  auto knownType = TCell;
  if (RuntimeOption::EvalCheckPropTypeHints >= 3 &&
      (!prop.typeConstraint.isUpperBound() ||
       RuntimeOption::EvalEnforceGenericsUB >= 2)) {
    knownType = typeFromPropTC(prop.typeConstraint, propCls, ctx, false);
    if (!(prop.attrs & AttrNoImplicitNullable)) knownType |= TInitNull;
  }
  knownType &= typeFromRAT(prop.repoAuthType, ctx);
  // Repo-auth-type doesn't include uninit for AttrLateInit props, so we need to
  // add it after intersecting with it.
  if (prop.attrs & AttrLateInit) {
    // If we're ignoring AttrLateInit, the prop might be uninit, but if we're
    // validating it, we'll never see uninit, so remove it.
    if (ignoreLateInit) {
      knownType |= TUninit;
    } else {
      knownType -= TUninit;
    }
  }
  return knownType;
}

/*
 * Try to find a property offset for the given key in baseClass. Will return
 * folly::none if the mapping from baseClass's name to the Class* can change
 * (which happens in sandbox mode when the ctx class is unrelated to baseClass).
 */
folly::Optional<PropInfo>
getPropertyOffset(IRGS& env,
                  const Class* baseClass,
                  Type keyType,
                  bool ignoreLateInit) {
  if (!baseClass) return folly::none;

  if (!keyType.hasConstVal(TStr)) return folly::none;
  auto const name = keyType.strVal();

  auto const ctx = curClass(env);

  // We need to check that baseClass cannot change between requests.
  if (!(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return folly::none;
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return folly::none;
      }
    }
  }

  // Lookup the index of the property based on ctx and baseClass
  auto const lookup = baseClass->getDeclPropSlot(ctx, name);
  auto const slot = lookup.slot;

  // If we couldn't find a property that is accessible in the current context,
  // bail out
  if (slot == kInvalidSlot || !lookup.accessible) return folly::none;

  // If it's a declared property we're good to go: even if a subclass redefines
  // an accessible property with the same name it's guaranteed to be at the same
  // offset.

  auto const& prop = baseClass->declProperties()[slot];

  // If we're going to serialize the profile data, we emit a ProfileProp here to
  // profile property accesses.  We only do this here, when we can resolve the
  // property at JIT time, in order to make profiling simpler and cheaper.  And
  // this should cover the vast majority of the property accesses anyway.
  if (env.context.kind == TransKind::Profile && isJitSerializing()) {
    gen(env, ProfileProp, cns(env, prop.baseCls->name()), cns(env, name));
  }

  return PropInfo(
    slot,
    baseClass->propSlotToIndex(slot),
    prop.attrs & AttrIsConst,
    prop.attrs & AttrIsReadOnly,
    prop.attrs & AttrLateInit,
    (prop.attrs & AttrLateInit) && !ignoreLateInit,
    knownTypeForProp(prop, baseClass, ctx, ignoreLateInit),
    &prop.typeConstraint,
    &prop.ubs,
    baseClass,
    prop.cls
  );
}

/*
 * Returns true iff a Prop{X,DX,Q} operation with the given base and key will
 * not write to its tvRef src, using the same set of conditions checked in
 * ObjectData::propImpl(). This allows us to skip the very expensive ratchet
 * operation after intermediate operations.
 */
bool prop_ignores_tvref(IRGS& env, SSATmp* base, const SSATmp* key) {
  // Make sure it's an object of a known class.
  if (!base->isA(TObj) || !base->type().clsSpec().cls()) return false;

  auto cls = base->type().clsSpec().cls();
  auto isDeclared = false;
  auto propClass = cls;

  // If the property name is known, try to look it up and get its RAT.
  if (key->hasConstVal(TStr)) {
    auto const keyStr = key->strVal();
    auto const ctx = curClass(env);
    auto const lookup = cls->getDeclPropSlot(ctx, keyStr);
    if (lookup.slot != kInvalidSlot && lookup.accessible) {
      isDeclared = true;
      auto const& prop = cls->declProperties()[lookup.slot];
      propClass = prop.cls;
    }
  }

  // Native prop handlers never kick in for declared properties, even if
  // they're unset.
  if (!isDeclared && cls->hasNativePropHandler()) return false;

  if (propClass == cls ||
      env.irb->constrainValue(base, GuardConstraint(propClass).setWeak())) {
    env.irb->constrainValue(base, GuardConstraint(cls));
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<GuardConstraint> simpleOpConstraint(SimpleOp op) {
  switch (op) {
    case SimpleOp::None:
      return folly::none;

    case SimpleOp::Vec:
    case SimpleOp::Dict:
    case SimpleOp::Keyset:
    case SimpleOp::String:
      return GuardConstraint(DataTypeSpecific);

    case SimpleOp::Vector:
      return GuardConstraint(c_Vector::classof());

    case SimpleOp::Map:
      return GuardConstraint(c_Map::classof());

    case SimpleOp::Pair:
      return GuardConstraint(c_Pair::classof());
  }

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

/*
 * Load or store the member base pointer.
 */
SSATmp* ldMBase(IRGS& env) {
  return gen(env, LdMBase, TLvalToCell);
}
void stMBase(IRGS& env, SSATmp* base) {
  if (base->isA(TPtrToCell)) base = gen(env, ConvPtrToLval, base);
  assert_flog(base->isA(TLvalToCell), "Unexpected mbase: {}", *base->inst());
  gen(env, StMBase, base);
}

/*
 * Get a pointer to the "tvTempBase" field used to materialize values that
 * are created on the fly during a member operation sequence.
 */
SSATmp* tvTempBasePtr(IRGS& env) {
  return gen(env, LdMIStateAddr, cns(env, offsetof(MInstrState, tvTempBase)));
}

SSATmp* propTvRefPtr(IRGS& env, SSATmp* base, const SSATmp* key) {
  return prop_ignores_tvref(env, base, key)
    ? cns(env, Type::cns(nullptr, TPtrToMISCell))
    : tvTempBasePtr(env);
}

SSATmp* ptrToUninit(IRGS& env) {
  // Nothing can write to the uninit null variant either, so the inner type
  // here is also always true.
  return cns(env, Type::cns(&immutable_uninit_base, TLvalToOtherUninit));
}

bool baseMightPromote(const SSATmp* base) {
  auto const ty = base->type().derefIfPtr();
  return
    ty.maybe(TNull) ||
    ty.maybe(Type::cns(false)) ||
    ty.maybe(Type::cns(staticEmptyString()));
}

//////////////////////////////////////////////////////////////////////

/*
 * This is called in a few places to be consistent with old minstrs, and should
 * be revisited once they're gone. It probably doesn't make sense to always
 * guard on an object class when we have one.
 */
void specializeObjBase(IRGS& env, SSATmp* base) {
  if (base && base->isA(TObj) && base->type().clsSpec().cls()) {
    env.irb->constrainValue(
      base, GuardConstraint(base->type().clsSpec().cls()));
  }
}

//////////////////////////////////////////////////////////////////////
// Intermediate ops

folly::Optional<PropInfo>
getCurrentPropertyOffset(IRGS& env, SSATmp* base, Type keyType,
                         bool ignoreLateInit) {
  // We allow the use of classes from nullable objects because
  // emitPropSpecialized() explicitly checks for null (when needed) before
  // doing the property access.
  auto const baseType = base->type().derefIfPtr();
  if (!(baseType < (TObj | TInitNull) && baseType.clsSpec())) {
    return folly::none;
  }

  auto const baseCls = baseType.clsSpec().cls();
  auto const info = getPropertyOffset(env, baseCls, keyType, ignoreLateInit);
  if (!info) return info;
  specializeObjBase(env, base);
  return info;
}

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It returns
 * a pointer to the property's address, or &immutable_null_base if the property
 * was Uninit and doWarn is true.
 *
 * We can omit the uninit check for properties that we know may not be uninit
 * due to the frontend's type inference.
 */
SSATmp* checkInitProp(IRGS& env,
                      SSATmp* baseAsObj,
                      SSATmp* propAddr,
                      SSATmp* key,
                      bool doWarn,
                      bool doDefine) {
  assertx(key->isA(TStaticStr));
  assertx(baseAsObj->isA(TObj));
  assertx(propAddr->type() <= TLvalToCell);
  assertx(!doWarn || !doDefine);

  auto const needsCheck = doWarn && propAddr->type().deref().maybe(TUninit);
  if (!needsCheck) return propAddr;

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckInitMem, taken, propAddr);
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return
          // &immutable_null_base.
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseUndefProp, baseAsObj, key);
      return ptrToInitNull(env);
    }
  );
}

/*
 * Returns a pointer to the property, and for MOpMode::Define, the object base
 * of the property.
 */
std::pair<SSATmp*, SSATmp*> emitPropSpecialized(
  IRGS& env,
  SSATmp* base,
  SSATmp* key,
  bool nullsafe,
  MOpMode mode,
  PropInfo propInfo
) {
  assertx(mode != MOpMode::Warn || mode != MOpMode::Unset);
  auto const doWarn   = mode == MOpMode::Warn;
  auto const doDefine = mode == MOpMode::Define || mode == MOpMode::Unset;

  auto const initNull = ptrToInitNull(env);
  auto const baseType = base->type();

  auto const getAddr = [&] (SSATmp* obj) {
    if (!propInfo.lateInitCheck) {
      assertx(!propInfo.lateInit || mode == MOpMode::Define);
      auto const addr = gen(
        env,
        LdPropAddr,
        IndexData { propInfo.index },
        propInfo.knownType.lval(Ptr::Prop),
        obj
      );
      return !propInfo.lateInit
        ? checkInitProp(env, obj, addr, key, doWarn, doDefine)
        : addr;
    }

    assertx(propInfo.lateInit);
    return cond(
      env,
      [&] (Block* taken) {
        return gen(
          env,
          LdInitPropAddr,
          IndexData { propInfo.index },
          taken,
          propInfo.knownType.lval(Ptr::Prop),
          obj
        );
      },
      [&] (SSATmp* addr) { return addr; },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(
          env,
          ThrowLateInitPropError,
          cns(env, propInfo.propClass),
          key,
          cns(env, false)
        );
        return cns(env, TBottom);
      }
    );
  };

  /*
   * Normal case, where the base is an object (and not a pointer to
   * something)---just do a lea with the type information we got from static
   * analysis.  The caller of this function will use it to know whether it can
   * avoid a generic incref, etc.
   */
  if (baseType <= TObj) {
    return {
      getAddr(base),
      mode == MOpMode::Define ? base : nullptr
    };
  }

  /*
   * We also support nullable objects for the base.  This is a frequent result
   * of static analysis on multi-dim property accesses ($foo->bar->baz), since
   * hhbbc doesn't try to prove __construct must be run or that sort of thing
   * (so every object-holding object property can also be null).
   *
   * After a null check, if it's actually an object we can just do LdPropAddr,
   * otherwise we just give out &immutable_null_base (after raising the
   * appropriate warnings).
   */

  SSATmp* obj = nullptr;
  auto const prop = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, TObj, taken, base);
    },
    [&] {
      // Next: Base is an object. Load property and check for uninit.
      obj = gen(env, LdMem, baseType.deref() & TObj, base);
      return getAddr(obj);
    },
    [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
      hint(env, Block::Hint::Unlikely);
      if (!nullsafe && doWarn) {
        auto const msg = makeStaticString(
            "Cannot access property on non-object");
        gen(env, RaiseNotice, cns(env, msg));
      }
      if (doDefine) {
        /*
         * This case is where we're logically supposed to do stdClass
         * promotion.  However, it's impossible that we're going to be asked to
         * do this with the way type inference works ahead of time right now:
         *
         *   o In defining minstrs, the only way hhbbc will know that an object
         *     type is nullable and also specialized is if the only type it can
         *     be is ?Obj=stdClass.  (This is because it does object property
         *     inference in a control flow insensitive way, so if null is
         *     possible stdClass must be added to the type, and there are no
         *     unions of multiple specialized object types.)
         *
         *   o On the other hand, if the type was really ?Obj=stdClass, we
         *     wouldn't have gotten a known property offset for any properties,
         *     because stdClass has no declared properties, so we can't be
         *     here.
         *
         * We could punt, but it's better to assert for now because if we
         * change this in hhbbc it will be on-purpose...
         */
        always_assert_flog(
          false,
          "Static analysis error: we would've needed to generate "
          "stdClass-promotion code in the JIT, which is unexpected."
        );
      }
      return initNull;
    }
  );

  // Note that normally using obj here is unsafe, as its only defined on one
  // side of the cond(). However, for MOpMode::Define, we'll assert if we emit
  // an actual branch, so its okay.
  assertx(mode != MOpMode::Define || obj);
  return { prop, mode == MOpMode::Define ? obj : nullptr };
}

//////////////////////////////////////////////////////////////////////
// "Simple op" handlers.

void checkCollectionBounds(IRGS& env, SSATmp* base,
                           SSATmp* idx, SSATmp* limit) {
  assertx(base->isA(TObj));

  ifThen(
    env,
    [&](Block* taken) {
      auto ok = gen(env, CheckRange, idx, limit);
      gen(env, JmpZero, taken, ok);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowOutOfBounds, base, idx);
    }
  );
}

void checkVecBounds(IRGS& env, SSATmp* base, SSATmp* idx) {
  assertx(base->isA(TVec));

  ifThen(
    env,
    [&](Block* taken) {
      gen(env, CheckVecBounds, taken, base, idx);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowOutOfBounds, base, idx);
    }
  );
}

template<class Finish>
SSATmp* emitVecGet(IRGS& env, SSATmp* base, SSATmp* key, Finish finish) {
  assertx(base->isA(TVec));

  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  checkVecBounds(env, base, key);

  auto finishMe = [&](SSATmp* elem) {
    gen(env, IncRef, elem);
    return elem;
  };

  auto const elem = gen(env, LdVecElem, base, key);
  auto const pelem = profiledType(env, elem, [&] { finish(finishMe(elem)); });
  return finishMe(pelem);
}

template<class Finish>
SSATmp* emitVecQuietGet(IRGS& env, SSATmp* base, SSATmp* key, Finish finish) {
  assertx(base->isA(TVec));

  if (key->isA(TStr)) return cns(env, TInitNull);
  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  auto const elem = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckVecBounds, taken, base, key);
    },
    [&] { return gen(env, LdVecElem, base, key); },
    [&] { return cns(env, TInitNull); }
  );

  auto finishMe = [&](SSATmp* element) {
    gen(env, IncRef, element);
    return element;
  };

  auto const pelem = profiledType(env, elem, [&] { finish(finishMe(elem)); });
  return finishMe(pelem);
}

template<class Finish>
SSATmp* emitDictKeysetGet(IRGS& env, SSATmp* base, SSATmp* key,
                          bool quiet, bool is_dict, Finish finish) {
  assertx(is_dict ? base->isA(TDict) : base->isA(TKeyset));

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  auto finishMe = [&](SSATmp* elem) {
    gen(env, IncRef, elem);
    return elem;
  };

  auto const mode = quiet ? MOpMode::None : MOpMode::Warn;
  auto const elem = profiledArrayAccess(
    env, base, key, mode,
    [&] (SSATmp* base, SSATmp* key, SSATmp* pos) {
      return gen(env, is_dict ? DictGetK : KeysetGetK, base, key, pos);
    },
    [&] (SSATmp* key) {
      if (quiet) return cns(env, TInitNull);
      gen(env, ThrowOutOfBounds, base, key);
      return cns(env, TBottom);
    },
    [&] (SSATmp* key, SizeHintData data) {
      return gen(
        env,
        is_dict
          ? (quiet ? DictGetQuiet : DictGet)
          : (quiet ? KeysetGetQuiet : KeysetGet),
        base,
        key
      );
    }
  );
  auto const pelem = profiledType(env, elem, [&] { finish(finishMe(elem)); });
  return finishMe(pelem);
}

template<class Finish>
SSATmp* emitVectorGet(IRGS& env, SSATmp* base, SSATmp* key, Finish finish) {
  auto const size = gen(env, LdVectorSize, base);
  checkCollectionBounds(env, base, key, size);

  auto vec = gen(env, LdColVec, base);
  auto result = gen(env, LdVecElem, vec, key);

  auto const profres = profiledType(env, result, [&] {
    gen(env, IncRef, result);
    finish(result);
  });
  gen(env, IncRef, profres);
  return profres;
}

template<class Finish>
SSATmp* emitPairGet(IRGS& env, SSATmp* base, SSATmp* key, Finish finish) {
  assertx(key->isA(TInt));

  auto const idx = [&] {
    if (key->hasConstVal()) {
      auto keyVal = key->intVal();
      if (keyVal < 0 || keyVal > 1) PUNT(emitPairGet);
    } else {
      checkCollectionBounds(env, base, key, cns(env, 2));
    }
    return key;
  }();

  auto const result = gen(env, LdPairElem, base, idx);
  auto const profres = profiledType(env, result, [&] {
    gen(env, IncRef, result);
    finish(result);
  });
  gen(env, IncRef, profres);
  return profres;
}

SSATmp* emitVecIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TVec));

  if (key->isA(TStr)) return cns(env, false);
  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckVecBounds, taken, base, key);
    },
    [&] {
      auto const elem = gen(env, LdVecElem, base, key);
      return gen(env, IsNType, TInitNull, elem);
    },
    [&] { return cns(env, false); }
  );
}

SSATmp* emitDictIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TDict));
  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  return gen(env, DictIsset, base, key);
}

SSATmp* emitKeysetIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TKeyset));
  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  return gen(env, KeysetIsset, base, key);
}

//////////////////////////////////////////////////////////////////////

SSATmp* emitIncDecProp(IRGS& env, IncDecOp op, SSATmp* base, SSATmp* key) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);

  if (propInfo && !propInfo->isConst) {
    // Special case for when the property is known to be an int.
    if (base->isA(TObj) && propInfo->knownType <= TInt) {
      base = emitPropSpecialized(env, base, key, false,
                                 MOpMode::Define, *propInfo).first;
      auto const prop = gen(env, LdMem, TInt, base);
      auto const result = incDec(env, op, prop);
      // No need for a property type-check because the input is an Int and the
      // result is always an Int.
      assertx(result != nullptr);
      assertx(result->isA(TInt));
      gen(env, StMem, base, result);
      return isPre(op) ? result : prop;
    }
  }

  return gen(env, IncDecProp, IncDecData{op}, base, key);
}

template<class Finish>
SSATmp* emitCGetElem(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpMode mode, SimpleOp simpleOp, Finish finish) {
  assertx(mode == MOpMode::Warn || mode == MOpMode::InOut);
  switch (simpleOp) {
    case SimpleOp::Vec:
      return emitVecGet(env, base, key, finish);
    case SimpleOp::Dict:
      return emitDictKeysetGet(env, base, key, false, true, finish);
    case SimpleOp::Keyset:
      return emitDictKeysetGet(env, base, key, false, false, finish);
    case SimpleOp::String:
      assertx(mode != MOpMode::InOut);
      return gen(env, StringGet, base, key);
    case SimpleOp::Vector:
      assertx(mode != MOpMode::InOut);
      return emitVectorGet(env, base, key, finish);
    case SimpleOp::Pair:
      assertx(mode != MOpMode::InOut);
      return emitPairGet(env, base, key, finish);
    case SimpleOp::Map:
      assertx(mode != MOpMode::InOut);
      return gen(env, MapGet, base, key);
    case SimpleOp::None:
      return gen(env, CGetElem, MOpModeData{mode}, base, key);
  }
  always_assert(false);
}

template<class Finish>
SSATmp* emitCGetElemQuiet(IRGS& env, SSATmp* base, SSATmp* key,
                          MOpMode mode, SimpleOp simpleOp, Finish finish) {
  assertx(mode != MOpMode::Warn);
  switch (simpleOp) {
    case SimpleOp::Vec:
      return emitVecQuietGet(env, base, key, finish);
    case SimpleOp::Dict:
      return emitDictKeysetGet(env, base, key, true, true, finish);
    case SimpleOp::Keyset:
      return emitDictKeysetGet(env, base, key, true, false, finish);
    case SimpleOp::String:
    case SimpleOp::Vector:
    case SimpleOp::Pair:
    case SimpleOp::Map:
      assertx(mode != MOpMode::InOut);
    case SimpleOp::None:
      return gen(env, CGetElem, MOpModeData{mode}, ldMBase(env), key);
  }
  always_assert(false);
}

SSATmp* emitIssetElem(IRGS& env, SSATmp* base, SSATmp* key, SimpleOp simpleOp) {
  switch (simpleOp) {
  case SimpleOp::Vec:
    return emitVecIsset(env, base, key);
  case SimpleOp::Dict:
    return emitDictIsset(env, base, key);
  case SimpleOp::Keyset:
    return emitKeysetIsset(env, base, key);
  case SimpleOp::String:
    return gen(env, StringIsset, base, key);
  case SimpleOp::Vector:
    return gen(env, VectorIsset, base, key);
  case SimpleOp::Pair:
    return gen(env, PairIsset, base, key);
  case SimpleOp::Map:
    return gen(env, MapIsset, base, key);
  case SimpleOp::None:
    return gen(env, IssetElem, base, key);
  }

  always_assert(false);
}

/*
 * Determine which simple collection op to use for the given base and key
 * types.
 */
SimpleOp simpleCollectionOp(Type baseType, Type keyType, bool readInst,
                            bool inOut) {
  if (inOut && !(baseType <= TArrLike)) return SimpleOp::None;

  if (baseType <= TVec)    return SimpleOp::Vec;
  if (baseType <= TDict)   return SimpleOp::Dict;
  if (baseType <= TKeyset) return SimpleOp::Keyset;

  if (baseType <= TStr) {
    if (keyType <= TInt) {
      // Don't bother with SetM on strings, because profile data shows it
      // basically never happens.
      if (readInst) return SimpleOp::String;
    }
  } else if (baseType < TObj && baseType.clsSpec()) {
    const Class* klass = baseType.clsSpec().cls();
    auto const isVector = collections::isType(klass, CollectionType::Vector);
    auto const isImmVector =
      collections::isType(klass, CollectionType::ImmVector);
    auto const isPair   = collections::isType(klass, CollectionType::Pair);
    auto const isMap    = collections::isType(klass, CollectionType::Map);
    auto const isImmMap = collections::isType(klass, CollectionType::ImmMap);

    if (isVector || isPair || (isImmVector && readInst)) {
      if (keyType <= TInt) {
        // We don't specialize setting pair elements.
        if (isPair && !readInst) return SimpleOp::None;

        return (isImmVector || isVector) ? SimpleOp::Vector : SimpleOp::Pair;
      }
    } else if ((isMap || (isImmMap && readInst)) &&
               (keyType <= TInt || keyType <= TStr)) {
      return SimpleOp::Map;
    }
  }

  return SimpleOp::None;
}

void baseGImpl(IRGS& env, SSATmp* name, MOpMode mode) {
  if (!name->isA(TStr)) PUNT(BaseG-non-string-name);
  auto base_mode = mode != MOpMode::Unset ? mode : MOpMode::None;
  auto gblPtr = gen(env, BaseG, MOpModeData{base_mode}, name);
  stMBase(env, gblPtr);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * We'd like to use value-type access rather than ref-type access for "lookup"
 * member-op sequences (that is, ones with mode Warn, None, or InOut).
 *
 * This helper converts value-type bases into the refs used by MInstrState.
 * store-elim will usually end up moving these ops off of the main trace.
 */
SSATmp* baseValueToLval(IRGS& env, SSATmp* base) {
  auto const temp = tvTempBasePtr(env);
  gen(env, StMem, temp, base);
  return gen(env, ConvPtrToLval, temp);
}

/*
 * Load and fully unpack---i.e., dereference---the member base.
 *
 * Also constrains the base value (and, if applicable, its inner value) to
 * DataTypeSpecific; it's expected that the caller only uses extractBase() when
 * it has a certain useful type.
 */
SSATmp* extractBase(IRGS& env) {
  auto const& mbase = env.irb->fs().mbase();

  env.irb->constrainLocation(Location::MBase{}, DataTypeSpecific);

  auto const base = mbase.value
    ? mbase.value
    : gen(env, LdMem, mbase.type, ldMBase(env));

  env.irb->constrainValue(base, DataTypeSpecific);

  return base;
}

/*
 * Constrain the member base's types.
 *
 * This does all the type constraint work of extractBase().  It's used in
 * situations where we need to know the fully-unpacked base's type, but only
 * need the base pointer.
 */
void constrainBase(IRGS& env) { extractBase(env); }

/*
 * Return the extracted object base if the predicted type is TObj, else just
 * return the base pointer.
 */
SSATmp* extractBaseIfObj(IRGS& env) {
  auto const baseType = env.irb->fs().mbase().type;
  return baseType <= TObj ? extractBase(env) : ldMBase(env);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_NULLSAFE_PROP_WRITE_ERROR(Strings::NULLSAFE_PROP_WRITE_ERROR);

SSATmp* propGenericImpl(IRGS& env, MOpMode mode, SSATmp* base, SSATmp* key,
                        bool nullsafe, ReadOnlyOp rop) {
  auto const define = mode == MOpMode::Define;
  if (define && nullsafe) {
    gen(env, RaiseError, cns(env, s_NULLSAFE_PROP_WRITE_ERROR.get()));
    return ptrToInitNull(env);
  }

  auto const tvRef = propTvRefPtr(env, base, key);
  if (nullsafe) return gen(env, PropQ, ReadOnlyData { rop }, base, key, tvRef);
  auto const op = define ? PropDX : PropX;
  return gen(env, op, PropData { mode, rop }, base, key, tvRef);
}

SSATmp* propImpl(IRGS& env, MOpMode mode, SSATmp* key, bool nullsafe, ReadOnlyOp op) {
  auto const baseType = env.irb->fs().mbase().type;

  if (mode == MOpMode::Unset && !baseType.maybe(TObj)) {
    constrainBase(env);
    return ptrToInitNull(env);
  }

  auto const base = extractBaseIfObj(env);

  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);
  if (!propInfo || propInfo->isConst || mode == MOpMode::Unset) {
    return propGenericImpl(env, mode, base, key, nullsafe, op);
  }
  if (propInfo->readOnly && op == ReadOnlyOp::Mutable) {
    gen(env, ThrowMustBeMutableException, cns(env, propInfo->propClass), key);
    return cns(env, TBottom);
  }

  SSATmp* propPtr;
  SSATmp* obj;
  std::tie(propPtr, obj) = emitPropSpecialized(
    env,
    base,
    key,
    nullsafe,
    mode,
    *propInfo
  );
  assertx(IMPLIES(mode == MOpMode::Define, obj != nullptr));
  return propPtr;
}

SSATmp* vecElemImpl(IRGS& env, MOpMode mode, Type baseType, SSATmp* key) {
  assertx(baseType <= TVec);
  assertx(key->isA(TInt) || key->isA(TStr) ||
          !key->type().maybe(TInt | TStr));

  auto const warn = mode == MOpMode::Warn || mode == MOpMode::InOut;
  auto const unset = mode == MOpMode::Unset;
  auto const define = mode == MOpMode::Define;

  auto const invalid_key = [&] {
    gen(env, ThrowInvalidArrayKey, extractBase(env), key);
    return cns(env, TBottom);
  };

  if (define) {
    return key->isA(TInt)
      ? gen(env, ElemVecD, baseType, ldMBase(env), key)
      : invalid_key();
  }

  if (unset) {
    return key->isA(TInt) ? gen(env, ElemVecU, baseType, ldMBase(env), key) :
           key->isA(TStr) ? ptrToInitNull(env)
           /* invalid */  : invalid_key();
  }

  if (warn) {
    if (key->isA(TInt)) {
      auto const base = extractBase(env);
      checkVecBounds(env, base, key);
      auto const elemType = arrLikeElemType(
        base->type(),
        key->type(),
        curClass(env)
      ).first.lval(Ptr::Elem);
      return gen(env, LdVecElemAddr, elemType, base, key);
    }
    return invalid_key();
  }

  if (key->isA(TInt)) {
    auto const base = extractBase(env);
    return cond(
      env,
      [&] (Block* taken) {
        gen(env, CheckVecBounds, taken, base, key);
      },
      [&] {
        auto const elemType = arrLikeElemType(
          base->type(),
          key->type(),
          curClass(env)
        ).first.lval(Ptr::Elem);
        return gen(env, LdVecElemAddr, elemType, base, key);
      },
      [&] { return ptrToInitNull(env); }
    );
  }

  if (key->isA(TStr)) return ptrToInitNull(env);

  return invalid_key();
}

SSATmp* dictElemImpl(IRGS& env, MOpMode mode, Type baseType, SSATmp* key) {
  assertx(baseType <= TDict);

  auto const unset = mode == MOpMode::Unset;
  auto const define = mode == MOpMode::Define;

  auto const base = extractBase(env);

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  return profiledArrayAccess(
    env, base, key, mode,
    [&] (SSATmp* dict, SSATmp* key, SSATmp* pos) {
      return gen(env, ElemDictK, dict, key, pos);
    },
    [&] (SSATmp* key) {
      if (unset || mode == MOpMode::None) return ptrToInitNull(env);
      assertx(mode == MOpMode::Warn || mode == MOpMode::InOut);
      gen(env, ThrowOutOfBounds, base, key);
      return cns(env, TBottom);
    },
    [&] (SSATmp* key, SizeHintData) {
      if (define || unset) {
        return gen(env, unset ? ElemDictU : ElemDictD,
                   baseType, ldMBase(env), key);
      }
      assertx(
        mode == MOpMode::Warn ||
        mode == MOpMode::None ||
        mode == MOpMode::InOut
      );
      auto const op = mode == MOpMode::None ? DictGetQuiet : DictGet;
      return baseValueToLval(env, gen(env, op, base, key));
    }
  );
}

SSATmp* keysetElemImpl(IRGS& env, MOpMode mode, Type baseType, SSATmp* key) {
  assertx(baseType <= TKeyset);

  auto const unset = mode == MOpMode::Unset;
  auto const define = mode == MOpMode::Define;

  auto const base = extractBase(env);

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  if (define || unset) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(env, s_InvalidKeysetOperationMsg.get())
    );
    return cns(env, TBottom);
  }

  return profiledArrayAccess(
    env, base, key, mode,
    [&] (SSATmp* keyset, SSATmp* key, SSATmp* pos) {
      return gen(env, ElemKeysetK, keyset, key, pos);
    },
    [&] (SSATmp* key) {
      if (mode == MOpMode::None) return ptrToInitNull(env);
      assertx(mode == MOpMode::Warn || mode == MOpMode::InOut);
      gen(env, ThrowOutOfBounds, base, key);
      return cns(env, TBottom);
    },
    [&] (SSATmp* key, SizeHintData) {
      assertx(
        mode == MOpMode::Warn ||
        mode == MOpMode::None ||
        mode == MOpMode::InOut
      );
      auto const op = mode == MOpMode::None ? KeysetGetQuiet : KeysetGet;
      return baseValueToLval(env, gen(env, op, base, key));
    }
  );
}

const StaticString s_OP_NOT_SUPPORTED_STRING(Strings::OP_NOT_SUPPORTED_STRING);

SSATmp* elemImpl(IRGS& env, MOpMode mode, SSATmp* key) {
  DEBUG_ONLY auto const warn = mode == MOpMode::Warn;
  auto const unset = mode == MOpMode::Unset;
  auto const define = mode == MOpMode::Define;

  auto const baseType = env.irb->fs().mbase().type;

  assertx(!define || !unset);
  assertx(!define || !warn);

  if (baseType <= TVec)    return vecElemImpl(env, mode, baseType, key);
  if (baseType <= TDict)   return dictElemImpl(env, mode, baseType, key);
  if (baseType <= TKeyset) return keysetElemImpl(env, mode, baseType, key);

  if (unset) {
    constrainBase(env);
    if (baseType <= TStr) {
      gen(env, RaiseError, cns(env, s_OP_NOT_SUPPORTED_STRING.get()));
      return ptrToUninit(env);
    }

    if (!baseType.maybe(TClsMeth | TArrLike | TObj)) {
      return ptrToUninit(env);
    }
  }

  auto const base = ldMBase(env);
  auto const data = MOpModeData { mode };
  if (define || unset) {
    auto const op = define ? ElemDX : ElemUX;
    return gen(env, op, data, base, key);
  }
  auto const value = gen(env, ElemX, data, base, key);
  return baseValueToLval(env, value);
}

template<class Finish>
SSATmp* cGetPropImpl(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpMode mode, bool nullsafe, Finish finish, ReadOnlyOp op) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);
  if (propInfo) {
    if (propInfo->readOnly && op == ReadOnlyOp::Mutable) {
      gen(env, ThrowMustBeMutableException, cns(env, propInfo->propClass), key);
      return cns(env, TBottom);
    }
    auto propAddr =
      emitPropSpecialized(env, base, key, nullsafe, mode, *propInfo).first;
    auto const ty = propAddr->type().deref();
    auto const result = gen(env, LdMem, ty, propAddr);
    auto const profres = profiledType(env, result, [&] {
      gen(env, IncRef, result);
      finish(result);
    });
    gen(env, IncRef, profres);
    return profres;
  }

  // No warning takes precedence over nullsafe.
  if (!nullsafe || mode != MOpMode::Warn) {
    return gen(env, CGetProp, PropData{mode, op}, base, key);
  }
  return gen(env, CGetPropQ, ReadOnlyData{op}, base, key);
}

Block* makeCatchSet(IRGS& env, uint32_t nDiscard) {
  auto block = defBlock(env, Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), block);
  gen(env, BeginCatch);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
    },
    [&] {
      assertx(!env.irb->fs().stublogue());
      hint(env, Block::Hint::Unused);
      auto const data = EndCatchData {
        spOffBCFromIRSP(env),
        EndCatchData::CatchMode::SideExit,
        EndCatchData::FrameMode::Phplogue,
        EndCatchData::Teardown::Full
      };
      gen(env, EndCatch, data, fp(env), sp(env));
    }
  );

  // Fallthrough from here on is side-exiting due to an InvalidSetMException.
  hint(env, Block::Hint::Unused);

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  popDecRef(env, DataTypeGeneric);
  for (int i = 0; i < nDiscard; ++i) {
    popDecRef(env, DataTypeGeneric);
  }
  auto const val = gen(env, LdUnwinderValue, TCell);
  push(env, val);

  // The minstr is done here, so we want to drop a FinishMemberOp to kill off
  // stores to MIState.
  gen(env, FinishMemberOp);

  gen(env, Jmp, makeExit(env, nextSrcKey(env)));
  return block;
}

SSATmp* setPropImpl(IRGS& env, uint32_t nDiscard, SSATmp* key, ReadOnlyOp op) {
  auto const value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);

  auto const base = extractBaseIfObj(env);

  auto const mode = MOpMode::Define;
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), true);

  if (propInfo && !propInfo->isConst) {
    if (!propInfo->readOnly && op == ReadOnlyOp::ReadOnly) {
      gen(env, ThrowMustBeReadOnlyException, cns(env, propInfo->propClass), key);
      return cns(env, TBottom);
    }
    SSATmp* propPtr;
    SSATmp* obj;
    std::tie(propPtr, obj) = emitPropSpecialized(
      env,
      base,
      key,
      false,
      mode,
      *propInfo
    );

    SSATmp* newVal = nullptr;
    assertx(obj != nullptr);
    verifyPropType(
      env,
      gen(env, LdObjClass, obj),
      propInfo->typeConstraint,
      propInfo->ubs,
      propInfo->slot,
      value,
      key,
      false,
      &newVal
    );

    auto const propTy = propPtr->type().deref();
    auto const oldVal = gen(env, LdMem, propTy, propPtr);
    gen(env, IncRef, newVal);
    gen(env, StMem, propPtr, newVal);
    decRef(env, oldVal);
  } else {
    gen(env, SetProp, makeCatchSet(env, nDiscard), ReadOnlyData{op}, base, key, value);
  }

  return value;
}

void handleStrTestResult(IRGS& env, uint32_t nDiscard, SSATmp* strTestResult) {
  // We expected SetElem's base to not be a Str but might be wrong. Make an
  // exit trace to side exit to the next instruction, replacing our guess with
  // the correct stack output.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckNullptr, taken, strTestResult);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const str = gen(env, AssertNonNull, strTestResult);
      popDecRef(env, DataTypeGeneric);
      for (int i = 0; i < nDiscard; ++i) {
        popDecRef(env);
      }
      push(env, str);
      gen(env, FinishMemberOp);
      gen(env, Jmp, makeExit(env, nextSrcKey(env)));
    }
  );
}

SSATmp* emitArrayLikeSet(IRGS& env, SSATmp* key, SSATmp* value) {
  auto const baseType = env.irb->fs().mbase().type;
  auto const base = extractBase(env);
  assertx(baseType <= TArrLike);

  auto const isVec = baseType <= TVec;
  auto const isDict = baseType <= TDict;
  auto const isKeyset = baseType <= TKeyset;
  assertx(isVec || isDict || isKeyset);

  if ((isVec && !key->isA(TInt)) ||
      (isDict && !key->isA(TInt | TStr))) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  if (isKeyset) {
    gen(env, ThrowInvalidOperation,
        cns(env, s_InvalidKeysetOperationMsg.get()));
    return cns(env, TBottom);
  }

  auto const baseLoc = ldMBase(env);
  if (!canUpdateCanonicalBase(baseLoc)) {
    gen(env, SetElem, baseLoc, key, value);
    return value;
  }

  auto const op = isVec ? VecSet : DictSet;
  auto const newArr = gen(env, op, base, key, value);

  // Update the base's location with the new array.
  updateCanonicalBase(env, baseLoc, newArr);
  gen(env, IncRef, value);
  return value;
}

void setNewElemVecImpl(IRGS& env, uint32_t nDiscard, SSATmp* basePtr,
                       Type baseType, SSATmp* value) {
  assertx(baseType <= TVec);
  auto const maybeCyclic = value->type().maybe(baseType);

  if (maybeCyclic) gen(env, IncRef, value);

  ifThen(
    env,
    [&](Block* taken) {
      auto const base = extractBase(env);

      gen(env, CheckArrayCOW, taken, base);
      auto const offset = gen(env, ReserveVecNewElem, taken, base);
      auto const elemPtr = gen(
        env,
        LdVecElemAddr,
        TLvalToElemUninit,
        base,
        offset
      );
      gen(env, StMem, elemPtr, value);
    },
    [&] {
      gen(env, SetNewElemVec, basePtr, value);
    }
  );

  if (!maybeCyclic) gen(env, IncRef, value);
}

SSATmp* setNewElemImpl(IRGS& env, uint32_t nDiscard) {
  auto value = topC(env);
  auto const baseType = env.irb->fs().mbase().type;

  // We load the member base pointer before calling makeCatchSet() to avoid
  // mismatched in-states for any catch block edges we emit later on.
  auto const basePtr = ldMBase(env);

  if (baseType <= TVec) {
    setNewElemVecImpl(env, nDiscard, basePtr, baseType, value);
  } else if (baseType <= TDict) {
    constrainBase(env);
    gen(env, IncRef, value);
    gen(env, SetNewElemDict, basePtr, value);
  } else if (baseType <= TKeyset) {
    constrainBase(env);
    if (!value->type().isKnownDataType()) {
      PUNT(SetM-NewElem-Keyset-ValueNotKnown);
    }
    value = convertClassKey(env, value);
    if (!value->isA(TInt | TStr)) {
      auto const base = extractBase(env);
      gen(env, ThrowInvalidArrayKey, base, value);
    } else {
      gen(env, IncRef, value);
      gen(env, SetNewElemKeyset, basePtr, value);
    }
  } else {
    gen(env, SetNewElem, makeCatchSet(env, nDiscard), basePtr, value);
  }
  return value;
}

SSATmp* setElemImpl(IRGS& env, uint32_t nDiscard, SSATmp* key) {
  auto value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);

  auto const baseType = env.irb->fs().mbase().type;
  auto const simpleOp = simpleCollectionOp(baseType, key->type(), false, false);

  if (auto gc = simpleOpConstraint(simpleOp)) {
    env.irb->constrainLocation(Location::MBase{}, *gc);
  }

  switch (simpleOp) {
    case SimpleOp::String:
      always_assert(false && "Bad SimpleOp in setElemImpl");
      break;

    case SimpleOp::Vector:
      gen(env, VectorSet, extractBase(env), key, value);
      gen(env, IncRef, value);
      break;

    case SimpleOp::Map:
      gen(env, MapSet, extractBase(env), key, value);
      gen(env, IncRef, value);
      break;

    case SimpleOp::Vec:
    case SimpleOp::Dict:
    case SimpleOp::Keyset:
      return emitArrayLikeSet(env, key, value);

    case SimpleOp::Pair:
    case SimpleOp::None:
      // We load the member base pointer before calling makeCatchSet() to avoid
      // mismatched in-states for any catch block edges we emit later on.
      auto const basePtr = ldMBase(env);
      auto const result = gen(env, SetElem, makeCatchSet(env, nDiscard),
                              basePtr, key, value);
      auto const t = result->type();
      if (!baseType.maybe(TStr) || t == TNullptr) {
        // Base is not a string. Result is always value.
      } else if (t == TStaticStr) {
        // Base is a string. Stack result is a new string so we're responsible
        // for decreffing value.
        decRef(env, value);
        value = result;
      } else {
        assertx(t == (TStaticStr | TNullptr));
        // Base might be a string. Emit a check to verify the result before
        // returning the optimistic result.
        handleStrTestResult(env, nDiscard, result);
      }
      break;
  }

  return value;
}

SSATmp* memberKey(IRGS& env, MemberKey mk) {
  auto const res = [&] () -> SSATmp* {
    switch (mk.mcode) {
      case MW:
        return nullptr;
      case MEL: case MPL:
        return ldLocWarn(env, mk.local, DataTypeSpecific);
      case MEC: case MPC:
        return topC(env, BCSPRelOffset{int32_t(mk.iva)});
      case MEI:
        return cns(env, mk.int64);
      case MET: case MPT: case MQT:
        return cns(env, mk.litstr);
    }
    not_reached();
  }();
  if (!res) return nullptr;

  if (!res->type().isKnownDataType()) PUNT(MInstr-KeyNotKnown);
  return convertClassKey(env, res);
}

}

//////////////////////////////////////////////////////////////////////

SSATmp* ptrToInitNull(IRGS& env) {
  // Nothing is allowed to write anything to the init null variant, so this
  // inner type is always true.
  return cns(env, Type::cns(&immutable_null_base, TLvalToOtherInitNull));
}

bool canUpdateCanonicalBase(SSATmp* baseLoc) {
  auto const canonicalBaseLoc = canonical(baseLoc);
  assertx(canonicalBaseLoc);
  switch (canonicalBaseLoc->inst()->op()) {
    case LdLocAddr:
    case LdStkAddr:
      break;
    default: {
      auto const t = baseLoc->type();
      if (t.maybe(TMemToFrameCell) ||
          t.maybe(TMemToStkCell)) {
        // We don't handle these, as anything simple would require killing all
        // frame and stack info in frame state (currently framestate asserts we
        // don't stmem to ptrs of this type for this reason).
        return false;
      }
      break;
    }
  }

  return true;
}

void updateCanonicalBase(IRGS& env, SSATmp* baseLoc, SSATmp* newArr) {
  auto const canonicalBaseLoc = canonical(baseLoc);
  assertx(canonicalBaseLoc);
  switch (canonicalBaseLoc->inst()->op()) {
    case LdLocAddr: {
      auto const locID = canonicalBaseLoc->inst()->extra<LocalId>()->locId;
      gen(env, StLoc, LocalId { locID }, fp(env), newArr);
      break;
    }
    case LdStkAddr: {
      auto const irSPRel =
        canonicalBaseLoc->inst()->extra<IRSPRelOffsetData>()->offset;
      gen(env, StStk, IRSPRelOffsetData{irSPRel}, sp(env), newArr);
      break;
    }
    default:
      gen(env, StMem, baseLoc, newArr);
      break;
  }
}

void mFinalImpl(IRGS& env, int32_t nDiscard, SSATmp* result) {
  for (auto i = 0; i < nDiscard; ++i) popDecRef(env);
  if (result) push(env, result);
  gen(env, FinishMemberOp);
}

//////////////////////////////////////////////////////////////////////

bool propertyMayBeCountable(const Class::Prop& prop) {
  // We can't call `knownTypeForProp` for type-hints that involve objects
  // here because the classes they refer to may not yet be defined. We return
  // `true` for these cases. Doing so doesn't cause unnecessary pessimization,
  // because subtypes of Object are going to be countable anyway.
  auto const& tc = prop.typeConstraint;
  if (tc.isObject() || tc.isThis()) return true;
  if (prop.repoAuthType.hasClassName()) return true;
  auto const type = knownTypeForProp(prop, nullptr, nullptr, true);
  return type.maybe(jit::TCounted);
}

//////////////////////////////////////////////////////////////////////

void emitBaseGC(IRGS& env, uint32_t idx, MOpMode mode) {
  auto name = top(env, BCSPRelOffset{safe_cast<int32_t>(idx)});
  baseGImpl(env, name, mode);
}

void emitBaseGL(IRGS& env, int32_t locId, MOpMode mode) {
  auto name = ldLoc(env, locId, DataTypeSpecific);
  baseGImpl(env, name, mode);
}

void emitBaseSC(IRGS& env,
                uint32_t propIdx,
                uint32_t clsIdx,
                MOpMode mode,
                ReadOnlyOp op) {
  auto const cls = topC(env, BCSPRelOffset{safe_cast<int32_t>(clsIdx)});
  if (!cls->isA(TCls)) PUNT(BaseSC-NotClass);

  auto const name = top(env, BCSPRelOffset{safe_cast<int32_t>(propIdx)});
  if (!name->isA(TStr)) PUNT(BaseS-non-string-name);

  assertx(mode != MOpMode::InOut);
  auto const writeMode = mode == MOpMode::Define || mode == MOpMode::Unset;

  const LdClsPropOptions opts { op, true, false, writeMode };
  auto const spropPtr = ldClsPropAddr(env, cls, name, opts).propPtr;
  stMBase(env, spropPtr);
}

void emitBaseL(IRGS& env, NamedLocal loc, MOpMode mode) {
  stMBase(env, ldLocAddr(env, loc.id));

  auto base = ldLoc(env, loc.id, DataTypeGeneric);

  if (!base->type().isKnownDataType()) PUNT(unknown-BaseL);

  if (base->isA(TUninit) && mode == MOpMode::Warn) {
    auto const baseName = curFunc(env)->localVarName(loc.name);
    env.irb->constrainLocal(loc.id, DataTypeSpecific,
                            "emitBaseL: Uninit base local");
    gen(env, ThrowUninitLoc, cns(env, baseName));
  }

  env.irb->fs().setMemberBase(base);
}

void emitBaseC(IRGS& env, uint32_t idx, MOpMode mode) {
  auto const bcOff = BCSPRelOffset{safe_cast<int32_t>(idx)};
  stMBase(env, ldStkAddr(env, bcOff));

  auto base = topC(env, bcOff);
  env.irb->fs().setMemberBase(base);
}

void emitBaseH(IRGS& env) {
  if (!curClass(env)) return interpOne(env);

  auto const base = ldThis(env);
  stMBase(env, baseValueToLval(env, base));
  env.irb->fs().setMemberBase(base);
}

void emitDim(IRGS& env, MOpMode mode, MemberKey mk) {
  auto const key = memberKey(env, mk);
  auto const base = [&] {
    if (mcodeIsProp(mk.mcode)) return propImpl(env, mode, key, mk.mcode == MQT, mk.rop);
    if (mcodeIsElem(mk.mcode)) return elemImpl(env, mode, key);
    PUNT(DimNewElem);
  }();

  stMBase(env, base);
}

void emitQueryM(IRGS& env, uint32_t nDiscard, QueryMOp query, MemberKey mk) {
  if (mk.mcode == MW) PUNT(QueryNewElem);
  auto key = memberKey(env, mk);
  auto const baseType = env.irb->fs().mbase().type;
  if (baseType <= TClsMeth) {
    PUNT(QueryM_is_ClsMeth);
  }
  auto simpleOp = SimpleOp::None;

  if (mcodeIsElem(mk.mcode)) {
    simpleOp = simpleCollectionOp(baseType, key->type(), true,
                                  query == QueryMOp::InOut);

    if (simpleOp != SimpleOp::None) {
      if (auto const tc = simpleOpConstraint(simpleOp)) {
        env.irb->constrainLocation(Location::MBase{}, *tc);
      }
    }
  }

  auto const maybeExtractBase = [simpleOp] (IRGS& environment) {
    return simpleOp == SimpleOp::None
      ? ldMBase(environment)
      : extractBase(environment);
  };

  auto const result = [&]() -> SSATmp* {
    switch (query) {
      case QueryMOp::InOut:
      case QueryMOp::CGet: {
        auto const mode = getQueryMOpMode(query);
        always_assert_flog(
          mode != MOpMode::InOut || mcodeIsElem(mk.mcode),
          "QueryOp InOut can only be used with Elem codes"
        );
        return mcodeIsProp(mk.mcode)
          ? cGetPropImpl(env, extractBaseIfObj(env), key,
                         mode, mk.mcode == MQT,
                         [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); },
                         mk.rop)
          : emitCGetElem(env, maybeExtractBase(env), key, mode, simpleOp,
                         [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); });
      }

      case QueryMOp::CGetQuiet: {
        auto const mode = getQueryMOpMode(query);
        return mcodeIsProp(mk.mcode)
          ? cGetPropImpl(
              env, extractBaseIfObj(env), key, mode, mk.mcode == MQT,
              [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); }, mk.rop)
          : emitCGetElemQuiet(
              env, maybeExtractBase(env), key, mode, simpleOp,
              [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); });
      }

      case QueryMOp::Isset:
        return mcodeIsProp(mk.mcode)
          ? gen(env, IssetProp, extractBaseIfObj(env), key)
          : emitIssetElem(env, maybeExtractBase(env), key, simpleOp);
    }
    not_reached();
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitSetM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto const key = memberKey(env, mk);
  auto const baseType = env.irb->fs().mbase().type;
  if (baseType <= TClsMeth) {
    PUNT(SetM_is_ClsMeth);
  }
  auto const result =
    mk.mcode == MW        ? setNewElemImpl(env, nDiscard) :
    mcodeIsElem(mk.mcode) ? setElemImpl(env, nDiscard, key) :
                            setPropImpl(env, nDiscard, key, mk.rop);

  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

void emitSetRangeM(IRGS& env,
                   uint32_t nDiscard,
                   uint32_t size,
                   SetRangeOp op,
                   ReadOnlyOp /*rop*/) {
  auto const count = gen(env, ConvTVToInt, ConvNoticeData{}, topC(env));
  auto const src = topC(env, BCSPRelOffset{1});
  auto const offset = gen(env, ConvTVToInt, ConvNoticeData{}, topC(env, BCSPRelOffset{2}));
  auto const reverse = op == SetRangeOp::Reverse;

  gen(
    env, reverse ? SetRangeRev : SetRange,
    ldMBase(env), offset, src, count, cns(env, size)
  );
  mFinalImpl(env, nDiscard + 3, nullptr);
}

void emitIncDecM(IRGS& env, uint32_t nDiscard, IncDecOp incDec, MemberKey mk) {
  auto key = memberKey(env, mk);

  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return emitIncDecProp(env, incDec, extractBaseIfObj(env), key);
    }
    if (mcodeIsElem(mk.mcode)) {
      auto const base = ldMBase(env);
      return gen(env, IncDecElem, IncDecData{incDec}, base, key);
    }
    PUNT(IncDecNewElem);
  }();

  mFinalImpl(env, nDiscard, result);
}

SSATmp* inlineSetOp(IRGS& env, SetOpOp op, SSATmp* lhs, SSATmp* rhs) {
  auto const maybeOp = [&]() -> folly::Optional<Op> {
    switch (op) {
    case SetOpOp::PlusEqual:   return Op::Add;
    case SetOpOp::MinusEqual:  return Op::Sub;
    case SetOpOp::MulEqual:    return Op::Mul;
    case SetOpOp::PlusEqualO:  return folly::none;
    case SetOpOp::MinusEqualO: return folly::none;
    case SetOpOp::MulEqualO:   return folly::none;
    case SetOpOp::DivEqual:    return folly::none;
    case SetOpOp::ConcatEqual: return folly::none;
    case SetOpOp::ModEqual:    return folly::none;
    case SetOpOp::PowEqual:    return folly::none;
    case SetOpOp::AndEqual:    return Op::BitAnd;
    case SetOpOp::OrEqual:     return Op::BitOr;
    case SetOpOp::XorEqual:    return Op::BitXor;
    case SetOpOp::SlEqual:     return folly::none;
    case SetOpOp::SrEqual:     return folly::none;
    }
    not_reached();
  }();

  if (!maybeOp) return nullptr;

  auto const bcOp = *maybeOp;
  if (!areBinaryArithTypesSupported(bcOp, lhs->type(), rhs->type())) {
    return nullptr;
  }

  bool isBitOp_ = isBitOp(bcOp);

  lhs = promoteBool(env, lhs, isBitOp_);
  rhs = promoteBool(env, rhs, isBitOp_);

  auto const hhirOp = isBitOp_ ? bitOp(bcOp)
                               : promoteBinaryDoubles(env, bcOp, lhs, rhs);
  auto result = gen(env, hhirOp, lhs, rhs);
  assertx(result->isA(TUncounted));
  return result;
}

template<class Finish>
SSATmp* setOpPropImpl(IRGS& env, SetOpOp op, SSATmp* base,
                      SSATmp* key, SSATmp* rhs, Finish finish) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);

  if (propInfo && !propInfo->isConst) {
    SSATmp* propPtr;
    SSATmp* obj;
    std::tie(propPtr, obj) = emitPropSpecialized(
      env,
      base,
      key,
      false,
      MOpMode::Define,
      *propInfo
    );
    assertx(obj != nullptr);

    auto const lhs = gen(env, LdMem, propPtr->type().deref(), propPtr);


    auto const handleShuffle = [&](SSATmp* result) {
      verifyPropType(
        env,
        gen(env, LdObjClass, obj),
        propInfo->typeConstraint,
        propInfo->ubs,
        propInfo->slot,
        result,
        key,
        false
      );
      gen(env, StMem, propPtr, result);
      auto const finishMe = [&] (SSATmp* oldVal) {
        gen(env, DecRef, DecRefData{}, oldVal);
        gen(env, IncRef, result);
        return result;
      };
      auto const plhs = profiledType(env, lhs, [&] {
        finishMe(lhs);
        finish(result);
      });
      return finishMe(plhs);
    };

    if (auto const result = inlineSetOp(env, op, lhs, rhs)) {
      assertx(!result->type().maybe(TClsMeth));
      return handleShuffle(result);
    }

    /*
     * If we statically know this SetOp cannot violate the type-hint, we can
     * skip the type-hint validation. Otherwise we need to use the (slower)
     * runtime helpers which also perform a type-hint validation. These might
     * need to perform the SetOp on a temporary, which can trigger COW where we
     * wouldn't otherwise.
     *
     * This is only really important for concat, so that's the only special case
     * we deal with right now. If the lhs is already a string (and therefore
     * must satisfy the type-hint), or the type-hint always allows strings
     * (concats will always produce a string, regardless of the rhs), we know a
     * check isn't necessary.
     */
    auto const fast = [&]{
      if (RuntimeOption::EvalCheckPropTypeHints <= 0) return true;
      if (!propInfo->typeConstraint ||
          !propInfo->typeConstraint->isCheckable()) {
        return true;
      }
      if (op != SetOpOp::ConcatEqual) return false;
      if (propPtr->type().deref() <= TStr) return true;
      return propInfo->typeConstraint->alwaysPasses(KindOfString);
    }();

    if (!fast) {
      return handleShuffle(gen(env, OutlineSetOp, SetOpData{op}, lhs, rhs));
    } else {
      gen(env, SetOpTV, SetOpData{op}, propPtr, rhs);
    }
    auto newVal = gen(env, LdMem, propPtr->type().deref(), propPtr);
    auto const pNewVal = profiledType(env, newVal, [&] {
      gen(env, IncRef, newVal);
      finish(newVal);
    });
    gen(env, IncRef, pNewVal);
    return pNewVal;
  }

  return gen(env, SetOpProp, SetOpData{op}, base, key, rhs);
}

void emitSetOpM(IRGS& env, uint32_t nDiscard, SetOpOp op, MemberKey mk) {
  auto key = memberKey(env, mk);
  auto rhs = topC(env);

  auto const finish = [&] (SSATmp* result) {
    popDecRef(env);
    mFinalImpl(env, nDiscard, result);
  };
  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return setOpPropImpl(env, op, extractBaseIfObj(env), key, rhs, finish);
    }
    if (mcodeIsElem(mk.mcode)) {
      auto const base = ldMBase(env);
      return gen(env, SetOpElem, SetOpData{op}, base, key, rhs);
    }
    PUNT(SetOpNewElem);
  }();

  finish(result);
}

void emitUnsetM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto key = memberKey(env, mk);

  if (mcodeIsProp(mk.mcode)) {
    gen(env, UnsetProp, extractBaseIfObj(env), key);
  } else {
    assertx(mcodeIsElem(mk.mcode));
    gen(env, UnsetElem, ldMBase(env), key);
  }

  mFinalImpl(env, nDiscard, nullptr);
}

//////////////////////////////////////////////////////////////////////

void logArrayAccessProfile(IRGS& env, SSATmp* arr, SSATmp* key,
                           MOpMode mode, const ArrayAccessProfile& profile) {
  // We generate code for many accesses each time we call retranslateAll.
  // We don't want production webservers to log when they do so.
  if (!RO::EvalLogArrayAccessProfile) return;
  if (env.inlineState.conjure) return;

  auto const marker  = makeMarker(env, curSrcKey(env));
  assertx(marker.hasFunc());
  auto const func = marker.func();

  std::vector<std::string> inline_state_string;
  std::vector<folly::StringPiece> inline_state;
  for (auto const& state : env.inlineState.bcStateStack) {
    inline_state_string.push_back(show(state));
    inline_state.push_back(inline_state_string.back());
  }

  StructuredLogEntry entry;
  entry.setStr("marker", marker.show());
  entry.setStr("profile", profile.toString());
  entry.setStr("source_func", func->fullName()->data());
  entry.setStr("source_file", func->filename()->data());
  entry.setInt("source_line", marker.sk().lineNumber());
  entry.setInt("prof_count", curProfCount(env));
  entry.setInt("inline_depth", env.inlineState.depth);
  entry.setVec("inline_state", inline_state);
  entry.setStr("arr_type", arr->type().toString());
  entry.setStr("key_type", key->type().toString());
  entry.setStr("hhbc", instrToString(marker.sk().pc(), func));
  entry.setStr("mode", subopToName(mode));

  StructuredLog::log("hhvm_array_accesses", entry);
}

void annotArrayAccessProfile(IRGS& env,
                             SSATmp* arr,
                             SSATmp* key,
                             const ArrayAccessProfile& profile,
                             const ArrayAccessProfile::Result& result) {
  if (!RuntimeOption::EvalDumpArrAccProf) return;

  auto const fnName = curFunc(env)->fullName()->data();

  env.unit.annotationData->add(
    "ArrAccProf",
    folly::sformat("BC={} FN={}: {}: {}: {}: {}\n",
                   bcOff(env), fnName, *arr, *key, profile, result)
  );
}

//////////////////////////////////////////////////////////////////////

}}}
