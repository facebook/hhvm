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
#include "hphp/runtime/vm/jit/array-kind-profile.h"
#include "hphp/runtime/vm/jit/array-offset-profile.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
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

#include <folly/Optional.h>

#include <sstream>

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_ArrayKindProfile("ArrayKindProfile");

//////////////////////////////////////////////////////////////////////

bool wantPropSpecializedWarnings() {
  return !RuntimeOption::RepoAuthoritative ||
    !RuntimeOption::EvalDisableSomeRepoAuthNotices;
}

//////////////////////////////////////////////////////////////////////

enum class SimpleOp {
  None,
  Array,
  ProfiledPackedArray,
  PackedArray,
  VecArray,
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
  PropInfo() = default;
  explicit PropInfo(int offset,
                    Slot slot,
                    bool immutable,
                    bool lateInit,
                    bool lateInitCheck,
                    Type knownType,
                    const HPHP::TypeConstraint* typeConstraint,
                    const Class* objClass,
                    const Class* propClass)
    : offset{offset}
    , slot{slot}
    , immutable{immutable}
    , lateInit{lateInit}
    , lateInitCheck{lateInitCheck}
    , knownType{std::move(knownType)}
    , typeConstraint{typeConstraint}
    , objClass{objClass}
    , propClass{propClass}
  {}

  int offset{-1};
  Slot slot{kInvalidSlot};
  bool immutable{false};
  bool lateInit{false};
  bool lateInitCheck{false};
  Type knownType{TGen};
  const HPHP::TypeConstraint* typeConstraint{nullptr};
  const Class* objClass{nullptr};
  const Class* propClass{nullptr};
};

Type knownTypeForProp(const Class::Prop& prop,
                      const Class* propCls,
                      const Class* ctx,
                      bool ignoreLateInit) {
  // The default value means that a AttrLateInitSoft property can potentially be
  // anything, so be pessimistic.
  if (prop.attrs & AttrLateInitSoft) return TGen;

  auto knownType = TGen;
  if (RuntimeOption::EvalCheckPropTypeHints >= 3) {
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
 * Try to find a property offset for the given key in baseClass. Will return a
 * PropInfo with an offset of -1 if the mapping from baseClass's name to the
 * Class* can change (which happens in sandbox mode when the ctx class is
 * unrelated to baseClass).
 */
PropInfo getPropertyOffset(IRGS& env,
                           const Class* baseClass,
                           Type keyType,
                           bool ignoreLateInit) {
  if (!baseClass) return PropInfo();

  if (!keyType.hasConstVal(TStr)) return PropInfo();
  auto const name = keyType.strVal();

  auto const ctx = curClass(env);

  // We need to check that baseClass cannot change between requests.
  if (!(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return PropInfo();
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return PropInfo();
      }
    }
  }

  // Lookup the index of the property based on ctx and baseClass
  auto const lookup = baseClass->getDeclPropIndex(ctx, name);
  auto const idx = lookup.slot;

  // If we couldn't find a property that is accessible in the current context,
  // bail out
  if (idx == kInvalidSlot || !lookup.accessible) return PropInfo();

  // If it's a declared property we're good to go: even if a subclass redefines
  // an accessible property with the same name it's guaranteed to be at the same
  // offset.

  auto const& prop = baseClass->declProperties()[idx];
  // If we want the AttrLateInitSoft default value behavior here, resort to the
  // runtime helpers to handle it.
  if ((prop.attrs & AttrLateInitSoft) && !ignoreLateInit) return PropInfo();

  return PropInfo(
    baseClass->declPropOffset(idx),
    idx,
    prop.attrs & AttrIsImmutable,
    prop.attrs & AttrLateInit,
    (prop.attrs & AttrLateInit) && !ignoreLateInit,
    knownTypeForProp(prop, baseClass, ctx, ignoreLateInit),
    &prop.typeConstraint,
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
  auto propType = TGen;
  auto isDeclared = false;
  auto propClass = cls;

  // If the property name is known, try to look it up and get its RAT.
  if (key->hasConstVal(TStr)) {
    auto const keyStr = key->strVal();
    auto const ctx = curClass(env);
    auto const lookup = cls->getDeclPropIndex(ctx, keyStr);
    if (lookup.slot != kInvalidSlot) {
      isDeclared = true;
      auto const& prop = cls->declProperties()[lookup.slot];
      propClass = prop.cls;
      propType = knownTypeForProp(prop, cls, curClass(env), true);
    }
  }

  // Magic getters/setters use tvRef if the property is unset.
  if (classMayHaveMagicPropMethods(cls) && propType.maybe(TUninit)) {
    return false;
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

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::VecArray:
    case SimpleOp::Dict:
    case SimpleOp::Keyset:
    case SimpleOp::String:
      return GuardConstraint(DataTypeSpecific);

    case SimpleOp::PackedArray:
      return GuardConstraint(DataTypeSpecialized).setWantArrayKind();

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
 *
 * Note that the LdMBase may get preOptimize'd away, or might have its type
 * refined, based on earlier tracked updates to the member base.
 */
SSATmp* ldMBase(IRGS& env) {
  return gen(env, LdMBase, TLvalToGen);
}
void stMBase(IRGS& env, SSATmp* base) {
  if (base->isA(TPtrToGen)) base = gen(env, ConvPtrToLval, base);
  assert_flog(base->isA(TLvalToGen), "Unexpected mbase: {}", *base->inst());

  gen(env, StMBase, base);
}

/*
 * Returns a pointer to a specific value in MInstrState.
 */
SSATmp* misLea(IRGS& env, int32_t offset) {
  env.irb->fs().setNeedRatchet(true);
  return gen(env, LdMIStateAddr, cns(env, offset));
}

SSATmp* tvRefPtr(IRGS& env) {
  return misLea(env, offsetof(MInstrState, tvRef));
}

SSATmp* propTvRefPtr(IRGS& env, SSATmp* base, const SSATmp* key) {
  return prop_ignores_tvref(env, base, key)
    ? cns(env, Type::cns(nullptr, TPtrToMISGen))
    : tvRefPtr(env);
}

SSATmp* tvRef2Ptr(IRGS& env) {
  return misLea(env, offsetof(MInstrState, tvRef2));
}

SSATmp* ptrToInitNull(IRGS& env) {
  // Nothing is allowed to write anything to the init null variant, so this
  // inner type is always true.
  return cns(env, Type::cns(&immutable_null_base, TLvalToOtherInitNull));
}

SSATmp* ptrToUninit(IRGS& env) {
  // Nothing can write to the uninit null variant either, so the inner type
  // here is also always true.
  return cns(env, Type::cns(&immutable_uninit_base, TLvalToOtherUninit));
}

bool baseMightPromote(const SSATmp* base) {
  auto const ty = base->type().strip();
  return
    ty.maybe(TNull) ||
    ty.maybe(Type::cns(false)) ||
    ty.maybe(Type::cns(staticEmptyString()));
}

SSATmp* propStatePtrDimProp(IRGS& env) {
  return (RuntimeOption::EvalCheckPropTypeHints <= 0)
    ? cns(env, TNullptr)
    : gen(env, LdMIPropStateAddr);
}

SSATmp* propStatePtrElem(IRGS& env, const SSATmp* base) {
  return RuntimeOption::EvalCheckPropTypeHints > 0 && baseMightPromote(base)
    ? gen(env, LdMIPropStateAddr)
    : cns(env, TNullptr);
}

SSATmp* propStatePtrFinalProp(IRGS& env, const SSATmp* base) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return cns(env, TNullptr);
  if (!RuntimeOption::EvalPromoteEmptyObject) return cns(env, TNullptr);
  return baseMightPromote(base)
    ? gen(env, LdMIPropStateAddr)
    : cns(env, TNullptr);
}

bool mightCallMagicPropMethod(MOpMode mode, PropInfo propInfo) {
  if (!propInfo.knownType.maybe(TUninit)) return false;
  auto const cls = propInfo.objClass;
  if (!cls) return true;
  // NB: this function can't yet be used for unset or isset contexts.  Just get
  // and set.
  auto const relevant_attrs =
    // Magic getters can be invoked both in define contexts and non-define
    // contexts.
    AttrNoOverrideMagicGet |
    // But magic setters are only possible in define contexts.
    (mode == MOpMode::Define ? AttrNoOverrideMagicSet : AttrNone);
  return (cls->attrs() & relevant_attrs) != relevant_attrs;
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

PropInfo getCurrentPropertyOffset(IRGS& env, SSATmp* base, Type keyType,
                                  bool ignoreLateInit) {
  // We allow the use of clases from nullable objects because
  // emitPropSpecialized() explicitly checks for null (when needed) before
  // doing the property access.
  auto const baseType = base->type().derefIfPtr();
  if (!(baseType < (TObj | TInitNull) && baseType.clsSpec())) return PropInfo{};

  auto const baseCls = baseType.clsSpec().cls();
  auto const info = getPropertyOffset(env, baseCls, keyType, ignoreLateInit);
  if (info.offset == -1) return info;
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
  assertx(propAddr->type() <= TLvalToGen);
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
      if (wantPropSpecializedWarnings()) {
        gen(env, RaiseUndefProp, baseAsObj, key);
      }
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
        ByteOffsetData { propInfo.offset },
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
          ByteOffsetData { propInfo.offset },
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
   * avoid a generic incref, unbox, etc.
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
      gen(env, CheckPackedArrayDataBounds, taken, base, idx);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowOutOfBounds, base, idx);
    }
  );
}

template<class Finish>
SSATmp* emitPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key, MOpMode mode,
                           Finish finish) {
  assertx(base->isA(TArr) &&
          base->type().arrSpec().kind() == ArrayData::kPackedKind &&
          key->isA(TInt));

  auto finishMe = [&](SSATmp* elem) {
    auto unboxed = unbox(env, elem, nullptr);
    gen(env, IncRef, unboxed);
    return unboxed;
  };

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, base, key);
    },
    [&] { // Next:
      auto const res = gen(env, LdPackedElem, base, key);
      auto const pres = profiledType(env, res, [&] { finish(finishMe(res)); });
      return finishMe(pres);
    },
    [&] { // Taken:
      hint(env, Block::Hint::Unlikely);
      if (mode == MOpMode::Warn || mode == MOpMode::InOut) {
        auto const inout = mode == MOpMode::InOut;
        gen(env, RaiseArrayIndexNotice,
            RaiseArrayIndexNoticeData { inout }, key);
      }
      return cns(env, TInitNull);
    }
  );
}

template<class Finish>
SSATmp* emitVecArrayGet(IRGS& env, SSATmp* base, SSATmp* key,
                        Finish finish) {
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
SSATmp* emitVecArrayQuietGet(IRGS& env, SSATmp* base, SSATmp* key,
                             Finish finish) {
  assertx(base->isA(TVec));

  if (key->isA(TStr)) return cns(env, TInitNull);
  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  auto const elem = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, base, key);
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
  assertx(base->isA(is_dict ? TDict : TKeyset));

  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  auto finishMe = [&](SSATmp* elem) {
    gen(env, IncRef, elem);
    return elem;
  };

  auto const elem = profiledArrayAccess(
    env, base, key,
    [&] (SSATmp* base, SSATmp* key, uint32_t pos) {
      return gen(env, is_dict ? DictGetK : KeysetGetK, IndexData { pos },
                 base, key);
    },
    [&] (SSATmp* key) {
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
SSATmp* emitArrayGet(IRGS& env, SSATmp* base, SSATmp* key, MOpMode mode,
                     Finish finish) {
  auto const elem = profiledArrayAccess(env, base, key,
    [&] (SSATmp* arr, SSATmp* key, uint32_t pos) {
      return gen(env, MixedArrayGetK, IndexData { pos }, arr, key);
    },
    [&] (SSATmp* key) {
      return gen(env, ArrayGet, MOpModeData { mode }, base, key);
    }
  );
  auto finishMe = [&](SSATmp* element) {
    auto const cell = unbox(env, element, nullptr);
    gen(env, IncRef, cell);
    return cell;
  };
  auto const pelem = profiledType(env, elem, [&] { finish(finishMe(elem)); });
  return finishMe(pelem);
}

template<class Finish>
SSATmp* emitProfiledPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key,
                                   MOpMode mode, Finish finish) {
  TargetProfile<ArrayKindProfile> prof(env.context,
                                       env.irb->curMarker(),
                                       s_ArrayKindProfile.get());
  if (prof.profiling()) {
    gen(env, ProfileArrayKind, RDSHandleData{prof.handle()}, base);
    return emitArrayGet(env, base, key, mode, finish);
  }

  if (prof.optimizing()) {
    auto const data = prof.data();
    auto const typePackedArr = Type::Array(ArrayData::kPackedKind);
    if (base->type().maybe(typePackedArr) &&
        (data.fraction(ArrayData::kPackedKind) == 1.0 ||
         RuntimeOption::EvalJitPGOArrayGetStress)) {
      // It's safe to side-exit still because we only do these profiled array
      // gets on the first element, with simple bases and single-element dims.
      // See computeSimpleCollectionOp.
      auto const exit = makeExit(env);
      base = gen(env, CheckType, typePackedArr, exit, base);
      env.irb->constrainValue(
        base,
        GuardConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      return emitPackedArrayGet(env, base, key, mode, finish);
    }
  }

  // Fall back to a generic array get.
  return emitArrayGet(env, base, key, mode, finish);
}

template<class Finish>
SSATmp* emitVectorGet(IRGS& env, SSATmp* base, SSATmp* key, Finish finish) {
  auto const size = gen(env, LdVectorSize, base);
  checkCollectionBounds(env, base, key, size);
  base = gen(env, LdVectorBase, base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(env, Shl, key, cns(env, 4));
  auto result = gen(env, LdElem, base, idx);
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

      // no reason to check bounds
      return cns(env, keyVal * sizeof(TypedValue));
    }

    static_assert(sizeof(TypedValue) == 16,
                  "TypedValue size expected to be 16 bytes");
    checkCollectionBounds(env, base, key, cns(env, 2));
    return gen(env, Shl, key, cns(env, 4));
  }();

  auto const pairBase = gen(env, LdPairBase, base);
  auto const result = gen(env, LdElem, pairBase, idx);
  auto const profres = profiledType(env, result, [&] {
    gen(env, IncRef, result);
    finish(result);
  });
  gen(env, IncRef, profres);
  return profres;
}

SSATmp* emitPackedArrayIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->type().arrSpec().kind() == ArrayData::kPackedKind);

  auto const elem = arrElemType(base->type(), key->type(), curClass(env));
  if (elem.first <= TNull) {
    return cns(env, false);
  } else if (elem.second) {
    return cns(env, true);
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, base, key);
    },
    [&] { // Next:
      auto const packedElem = gen(env, LdPackedElem, base, key);
      return gen(env, IsNType, TNull, packedElem);
    },
    [&] { // Taken:
      return cns(env, false);
    }
  );
}

SSATmp* emitVecArrayIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TVec));

  if (key->isA(TStr)) return cns(env, false);
  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, base, key);
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

SSATmp* emitVecArrayEmptyElem(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TVec));

  if (key->isA(TStr)) return cns(env, true);
  if (!key->isA(TInt)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, base, key);
    },
    [&] {
      auto const elem = gen(env, LdVecElem, base, key);
      auto const b = gen(env, ConvCellToBool, elem);
      return gen(env, XorBool, b, cns(env, true));
    },
    [&] { return cns(env, true); }
  );
}

SSATmp* emitDictEmptyElem(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TDict));
  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  return gen(env, DictEmptyElem, base, key);
}

SSATmp* emitKeysetEmptyElem(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TKeyset));
  if (!key->isA(TInt | TStr)) {
    gen(env, ThrowInvalidArrayKey, base, key);
    return cns(env, TBottom);
  }
  return gen(env, KeysetEmptyElem, base, key);
}

//////////////////////////////////////////////////////////////////////

SSATmp* emitIncDecProp(IRGS& env, IncDecOp op, SSATmp* base, SSATmp* key) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);

  if (RuntimeOption::RepoAuthoritative &&
      propInfo.offset != -1 &&
      !propInfo.immutable &&
      !mightCallMagicPropMethod(MOpMode::None, propInfo) &&
      !mightCallMagicPropMethod(MOpMode::Define, propInfo)) {

    // Special case for when the property is known to be an int.
    if (base->isA(TObj) && propInfo.knownType <= TInt) {
      base = emitPropSpecialized(env, base, key, false,
                                 MOpMode::Define, propInfo).first;
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

  return gen(
    env,
    IncDecProp,
    IncDecData{op},
    base,
    key,
    propStatePtrFinalProp(env, base)
  );
}

template<class Finish>
SSATmp* emitCGetElem(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpMode mode, SimpleOp simpleOp, Finish finish) {
  assertx(mode == MOpMode::Warn || mode == MOpMode::InOut);
  switch (simpleOp) {
    case SimpleOp::Array:
      return emitArrayGet(env, base, key, mode, finish);
    case SimpleOp::PackedArray:
      return emitPackedArrayGet(env, base, key, mode, finish);
    case SimpleOp::ProfiledPackedArray:
      return emitProfiledPackedArrayGet(env, base, key, mode, finish);
    case SimpleOp::VecArray:
      return emitVecArrayGet(env, base, key, finish);
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
    case SimpleOp::VecArray:
      return emitVecArrayQuietGet(env, base, key, finish);
    case SimpleOp::Dict:
      return emitDictKeysetGet(env, base, key, true, true, finish);
    case SimpleOp::Keyset:
      return emitDictKeysetGet(env, base, key, true, false, finish);
    case SimpleOp::Array:
      return emitArrayGet(env, base, key, mode, finish);
    case SimpleOp::PackedArray:
      return emitPackedArrayGet(env, base, key, mode, finish);
    case SimpleOp::ProfiledPackedArray:
      return emitProfiledPackedArrayGet(env, base, key, mode, finish);
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
  case SimpleOp::Array:
  case SimpleOp::ProfiledPackedArray:
    return gen(env, ArrayIsset, base, key);
  case SimpleOp::PackedArray:
    return emitPackedArrayIsset(env, base, key);
  case SimpleOp::VecArray:
    return emitVecArrayIsset(env, base, key);
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

SSATmp* emitEmptyElem(IRGS& env, SSATmp* base,
                      SSATmp* key, SimpleOp simpleOp) {
  switch (simpleOp) {
    case SimpleOp::VecArray:
      return emitVecArrayEmptyElem(env, base, key);
    case SimpleOp::Dict:
      return emitDictEmptyElem(env, base, key);
    case SimpleOp::Keyset:
      return emitKeysetEmptyElem(env, base, key);
    case SimpleOp::Array:
    case SimpleOp::PackedArray:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::String:
    case SimpleOp::Vector:
    case SimpleOp::Pair:
    case SimpleOp::Map:
    case SimpleOp::None:
      return gen(env, EmptyElem, ldMBase(env), key);
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

  if (baseType <= TArr) {
    auto isPacked = false;
    if (auto arrSpec = baseType.arrSpec()) {
      isPacked = arrSpec.kind() == ArrayData::kPackedKind;
    }
    if (keyType <= TInt || keyType <= TStr) {
      if (readInst && keyType <= TInt) {
        return isPacked ? SimpleOp::PackedArray : SimpleOp::ProfiledPackedArray;
      }
      return SimpleOp::Array;
    }
  } else if (baseType <= TVec) {
    return SimpleOp::VecArray;
  } else if (baseType <= TDict) {
    return SimpleOp::Dict;
  } else if (baseType <= TKeyset) {
    return SimpleOp::Keyset;
  } else if (baseType <= TStr) {
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

/*
 * Store Uninit to tvRef and tvRef2.
 */
void initTvRefs(IRGS& env) {
  gen(env, StMem, tvRefPtr(env), cns(env, TUninit));
  gen(env, StMem, tvRef2Ptr(env), cns(env, TUninit));
}

/*
 * DecRef tvRef and tvRef2.
 */
void cleanTvRefs(IRGS& env) {
  for (auto ptr : {tvRefPtr(env), tvRef2Ptr(env)}) {
    decRef(env, gen(env, LdMem, TGen, ptr));
  }
}

/*
 * If tvRef is not Uninit, DecRef tvRef2 and move tvRef's value to tvRef2,
 * storing Uninit to tvRef. Returns the adjusted base, which may point to
 * tvRef2.
 */
SSATmp* ratchetRefs(IRGS& env, SSATmp* base) {
  if (!env.irb->fs().needRatchet()) {
    return base;
  }

  auto tvRef = tvRefPtr(env);

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, taken, TUninit, tvRef);
    },
    [&] { // Next: tvRef is Uninit. Do nothing.
      return base;
    },
    [&] { // Taken: tvRef isn't Uninit. Ratchet the refs
      auto tvRef2 = tvRef2Ptr(env);
      // Clean up tvRef2 before overwriting it.
      auto const oldRef2 = gen(env, LdMem, TGen, tvRef2);
      decRef(env, oldRef2);

      // Copy tvRef to tvRef2.
      auto const tvRefVal = gen(env, LdMem, TGen, tvRef);
      gen(env, StMem, tvRef2, tvRefVal);
      // Reset tvRef.
      gen(env, StMem, tvRef, cns(env, TUninit));

      // Adjust base pointer.  Don't use 'tvRef2' here so that we don't reuse
      // the temp.  This will let us elide uses of the register for 'tvRef2',
      // until the Jmp we're going to emit here.
      return gen(env, ConvPtrToLval, tvRef2Ptr(env));
    }
  );
}

void setEmptyMIPropState(IRGS& env, const SSATmp* newBase, MOpMode mode) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  if (mode != MOpMode::Define) return;
  if (!baseMightPromote(newBase)) return;
  gen(env, StMIPropState, cns(env, TNullptr), cns(env, 0), cns(env, false));
}

void setObjMIPropState(IRGS& env, const SSATmp* newBase, MOpMode mode,
                       SSATmp* obj, Slot slot) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  if (mode != MOpMode::Define) return;
  if (!baseMightPromote(newBase)) return;
  auto const cls = gen(env, LdObjClass, obj);
  gen(env, StMIPropState, cls, cns(env, slot), cns(env, false));
}

void setClsMIPropState(IRGS& env, const SSATmp* newBase, MOpMode mode,
                       SSATmp* cls, SSATmp* name) {
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
  if (mode != MOpMode::Define) return;
  if (!baseMightPromote(newBase)) return;
  auto const slot = gen(env, LookupSPropSlot, cls, name);
  gen(env, StMIPropState, cls, slot, cns(env, true));
}

void baseGImpl(IRGS& env, SSATmp* name, MOpMode mode) {
  if (!name->isA(TStr)) PUNT(BaseG-non-string-name);
  auto base_mode = mode != MOpMode::Unset ? mode : MOpMode::None;
  auto gblPtr = gen(env, BaseG, MOpModeData{base_mode}, name);
  stMBase(env, gblPtr);
  setEmptyMIPropState(env, gblPtr, mode);
}

void baseSImpl(IRGS& env, SSATmp* name, uint32_t clsRefSlot, MOpMode mode) {
  if (!name->isA(TStr)) PUNT(BaseS-non-string-name);
  auto const cls = takeClsRefCls(env, clsRefSlot);
  auto const spropPtr = ldClsPropAddr(env, cls, name, true, false).propPtr;
  stMBase(env, spropPtr);
  setClsMIPropState(env, spropPtr, mode, cls, name);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Punt if the given base type isn't known to be boxed or unboxed.
 */
void puntGenBase(Type baseType) {
  if (baseType.maybe(TCell) && baseType.maybe(TBoxedCell)) {
    PUNT(MInstr-GenBase);
  }
}

/*
 * Update FrameState for a base at a known location.
 */
void simpleBaseImpl(IRGS& env, SSATmp* base, MOpMode mode, Location l) {
  puntGenBase(base->type());

  auto const predicted = base->isA(TBoxedCell)
    ? folly::make_optional(env.irb->fs().predictedTypeOf(l))
    : folly::none;
  env.irb->fs().setMemberBase(base, predicted);

  setEmptyMIPropState(env, base, mode);
}

/*
 * Load and fully unpack---i.e., dereference and unbox---the member base.
 *
 * Also constrains the base value (and, if applicable, its inner value) to
 * DataTypeSpecific; it's expected that the caller only uses extractBase() when
 * it has a certain useful type.
 */
SSATmp* extractBase(IRGS& env) {
  auto const& mbase = env.irb->fs().mbase();
  puntGenBase(mbase.type);

  env.irb->constrainLocation(Location::MBase{}, DataTypeSpecific);

  auto const base = mbase.value
    ? mbase.value
    : gen(env, LdMem, mbase.type, ldMBase(env));

  env.irb->constrainValue(base, DataTypeSpecific);

  if (base->isA(TBoxedCell)) {
    auto const innerTy = env.irb->predictedMBaseInnerType();
    gen(env, CheckRefInner, innerTy, makeExit(env), base);

    auto const inner = gen(env, LdRef, innerTy, base);
    env.irb->constrainValue(inner, DataTypeSpecific);
    return inner;
  }

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
 * Type of extractBase().
 *
 * Used to determine whether to actually unpack the member base (and thus
 * constrain types) for a given minstr implementation.
 */
Type predictedBaseType(const IRGS& env) {
  auto const baseType = env.irb->fs().mbase().type;

  return baseType <= TBoxedCell
    ? env.irb->predictedMBaseInnerType()
    : baseType;
}

/*
 * Return the extracted object base if the predicted type is TObj, else just
 * return the base pointer.
 */
SSATmp* extractBaseIfObj(IRGS& env) {
  auto const baseType = predictedBaseType(env);
  return baseType <= TObj ? extractBase(env) : ldMBase(env);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_NULLSAFE_PROP_WRITE_ERROR(Strings::NULLSAFE_PROP_WRITE_ERROR);

SSATmp* propGenericImpl(IRGS& env, MOpMode mode, SSATmp* base, SSATmp* key,
                        bool nullsafe) {
  auto const define = mode == MOpMode::Define;
  if (define && nullsafe) {
    gen(env, RaiseError, cns(env, s_NULLSAFE_PROP_WRITE_ERROR.get()));
    return ptrToInitNull(env);
  }

  auto const modeData = MOpModeData{mode};

  auto const tvRef = propTvRefPtr(env, base, key);
  return nullsafe
    ? gen(env, PropQ, base, key, tvRef)
    : define
      ? gen(env, PropDX, modeData, base, key, tvRef, propStatePtrDimProp(env))
      : gen(env, PropX, modeData, base, key, tvRef);
}

SSATmp* propImpl(IRGS& env, MOpMode mode, SSATmp* key, bool nullsafe) {
  auto const baseType = predictedBaseType(env);

  if (mode == MOpMode::Unset && !baseType.maybe(TObj)) {
    constrainBase(env);
    return ptrToInitNull(env);
  }

  auto const base = extractBaseIfObj(env);

  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);
  if (propInfo.offset == -1 ||
      propInfo.immutable ||
      mode == MOpMode::Unset ||
      mightCallMagicPropMethod(mode, propInfo)) {
    return propGenericImpl(env, mode, base, key, nullsafe);
  }

  SSATmp* propPtr;
  SSATmp* obj;
  std::tie(propPtr, obj) = emitPropSpecialized(
    env,
    base,
    key,
    nullsafe,
    mode,
    propInfo
  );
  if (mode == MOpMode::Define) {
    assertx(obj != nullptr);
    setObjMIPropState(env, propPtr, mode, obj, propInfo.slot);
  }
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
      auto const elemType = vecElemType(
        base->type(),
        key->type(),
        curClass(env)
      ).first.lval(Ptr::Elem);
      return gen(env, LdPackedArrayDataElemAddr, elemType, base, key);
    }
    return invalid_key();
  }

  if (key->isA(TInt)) {
    auto const base = extractBase(env);
    return cond(
      env,
      [&] (Block* taken) {
        gen(env, CheckPackedArrayDataBounds, taken, base, key);
      },
      [&] {
        auto const elemType = vecElemType(
          base->type(),
          key->type(),
          curClass(env)
        ).first.lval(Ptr::Elem);
        return gen(env, LdPackedArrayDataElemAddr, elemType, base, key);
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
    env, base, key,
    [&] (SSATmp* dict, SSATmp* key, uint32_t pos) {
      return gen(env, ElemDictK, IndexData { pos }, dict, key);
    },
    [&] (SSATmp* key) {
      if (define || unset) {
        return gen(env, unset ? ElemDictU : ElemDictD,
                   baseType, ldMBase(env), key);
      }
      assertx(
        mode == MOpMode::Warn ||
        mode == MOpMode::None ||
        mode == MOpMode::InOut
      );
      return gen(env, ElemDictX, MOpModeData { mode }, base, key);
    },
    define || unset // cow check
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

  if (define) {
    gen(
      env,
      ThrowInvalidOperation,
      cns(env, s_InvalidKeysetOperationMsg.get())
    );
    return cns(env, TBottom);
  }

  return profiledArrayAccess(
    env, base, key,
    [&] (SSATmp* keyset, SSATmp* key, uint32_t pos) {
      return gen(env, ElemKeysetK, IndexData { pos }, keyset, key);
    },
    [&] (SSATmp* key) {
      if (unset) return gen(env, ElemKeysetU, baseType, ldMBase(env), key);
      assertx(
        mode == MOpMode::Warn ||
        mode == MOpMode::None ||
        mode == MOpMode::InOut
      );
      return gen(env, ElemKeysetX, MOpModeData { mode }, base, key);
    },
    unset // cow check
  );
}

const StaticString s_OP_NOT_SUPPORTED_STRING(Strings::OP_NOT_SUPPORTED_STRING);

SSATmp* elemImpl(IRGS& env, MOpMode mode, SSATmp* key) {
  DEBUG_ONLY auto const warn = mode == MOpMode::Warn;
  auto const unset = mode == MOpMode::Unset;
  auto const define = mode == MOpMode::Define;

  auto const baseType = predictedBaseType(env);

  assertx(!define || !unset);
  assertx(!define || !warn);

  if (baseType <= TVec) return vecElemImpl(env, mode, baseType, key);
  if (baseType <= TDict) return dictElemImpl(env, mode, baseType, key);
  if (baseType <= TKeyset) return keysetElemImpl(env, mode, baseType, key);

  if (baseType <= TArr && key->type().subtypeOfAny(TInt, TStr)) {
    auto const base = extractBase(env);

    return profiledArrayAccess(env, base, key,
      [&] (SSATmp* arr, SSATmp* key, uint32_t pos) {
        return gen(env, ElemMixedArrayK, IndexData { pos }, arr, key);
      },
      [&] (SSATmp* key) {
        if (define || unset) {
          return gen(env, unset ? ElemArrayU : ElemArrayD,
                     base->type(), ldMBase(env), key);
        }
        assertx(
          mode == MOpMode::Warn ||
          mode == MOpMode::None ||
          mode == MOpMode::InOut
        );
        return gen(env, ElemArrayX, MOpModeData { mode }, base, key);
      },
      define || unset // cow check
    );
  }

  if (unset) {
    constrainBase(env);
    if (baseType <= TStr) {
      gen(env, RaiseError, cns(env, s_OP_NOT_SUPPORTED_STRING.get()));
      return ptrToUninit(env);
    }

    if (!baseType.maybe(TArrLike | TObj)) {
      return ptrToUninit(env);
    }
  }

  auto const base = ldMBase(env);
  if (define) {
    return gen(
      env,
      ElemDX,
      MOpModeData { mode },
      base,
      key,
      tvRefPtr(env),
      propStatePtrElem(env, base)
    );
  }
  auto const op = unset ? ElemUX : ElemX;
  return gen(env, op, MOpModeData { mode }, base, key, tvRefPtr(env));
}

/*
 * Pop nDiscard elements from the stack, push the result (if present), DecRef
 * tvRef(2), and mark the member operation as complete.
 */
void mFinalImpl(IRGS& env, int32_t nDiscard, SSATmp* result) {
  for (auto i = 0; i < nDiscard; ++i) popDecRef(env);
  cleanTvRefs(env);
  if (result) push(env, result);

  gen(env, FinishMemberOp);
}

template<class Finish>
SSATmp* cGetPropImpl(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpMode mode, bool nullsafe, Finish finish) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), false);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MOpMode::None, propInfo)) {

    auto propAddr =
      emitPropSpecialized(env, base, key, nullsafe, mode, propInfo).first;
    auto const ty = propAddr->type().deref();
    auto const cellPtr =
      ty.maybe(TBoxedCell) ? gen(env, UnboxPtr, propAddr) : propAddr;
    auto const result = gen(env, LdMem, ty.unbox(), cellPtr);
    auto const profres = profiledType(env, result, [&] {
      gen(env, IncRef, result);
      finish(result);
    });
    gen(env, IncRef, profres);
    return profres;
  }

  // No warning takes precedence over nullsafe.
  if (!nullsafe || mode != MOpMode::Warn) {
    return gen(env, CGetProp, MOpModeData{mode}, base, key);
  }
  return gen(env, CGetPropQ, base, key);
}

Block* makeCatchSet(IRGS& env) {
  auto block = defBlock(env, Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), block);
  gen(env, BeginCatch);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
    },
    [&] {
      hint(env, Block::Hint::Unused);
      gen(env, EndCatch,
          IRSPRelOffsetData { spOffBCFromIRSP(env) },
          fp(env), sp(env));
    }
  );

  // Fallthrough from here on is side-exiting due to an InvalidSetMException.
  hint(env, Block::Hint::Unused);

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  popDecRef(env, DataTypeGeneric);
  auto const nDiscard = env.currentNormalizedInstruction->imm[0].u_IVA;
  for (int i = 0; i < nDiscard; ++i) {
    popDecRef(env, DataTypeGeneric);
  }
  cleanTvRefs(env);
  auto const val = gen(env, LdUnwinderValue, TCell);
  push(env, val);

  // The minstr is done here, so we want to drop a FinishMemberOp to kill off
  // stores to MIState.
  gen(env, FinishMemberOp);

  gen(env, Jmp, makeExit(env, nextBcOff(env)));
  return block;
}

SSATmp* setPropImpl(IRGS& env, SSATmp* key) {
  auto const value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);

  auto const base = extractBaseIfObj(env);

  auto const mode = MOpMode::Define;
  auto const propInfo =
    getCurrentPropertyOffset(env, base, key->type(), true);

  if (propInfo.offset != -1 &&
      !propInfo.immutable &&
      !mightCallMagicPropMethod(mode, propInfo)) {

    SSATmp* propPtr;
    SSATmp* obj;
    std::tie(propPtr, obj) = emitPropSpecialized(
      env,
      base,
      key,
      false,
      mode,
      propInfo
    );

    assertx(obj != nullptr);
    verifyPropType(
      env,
      gen(env, LdObjClass, obj),
      propInfo.typeConstraint,
      propInfo.slot,
      value,
      key,
      false
    );

    auto propTy = propPtr->type().deref();
    if (propTy.maybe(TBoxedCell)) {
      propTy = propTy.unbox();
      propPtr = gen(env, UnboxPtr, propPtr);
    }

    env.irb->constrainValue(value, DataTypeBoxAndCountness);
    auto const oldVal = gen(env, LdMem, propTy, propPtr);
    gen(env, IncRef, value);
    gen(env, StMem, propPtr, value);
    decRef(env, oldVal);
  } else {
    gen(
      env,
      SetProp,
      makeCatchSet(env),
      base,
      key,
      value,
      propStatePtrFinalProp(env, base)
    );
  }

  return value;
}

void handleStrTestResult(IRGS& env, SSATmp* strTestResult) {
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
      popDecRef(env, DataTypeSpecific);
      auto const nDiscard = env.currentNormalizedInstruction->imm[0].u_IVA;
      for (int i = 0; i < nDiscard; ++i) {
        popDecRef(env, DataTypeSpecific);
      }
      cleanTvRefs(env);
      push(env, str);
      gen(env, FinishMemberOp);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );
}

SSATmp* emitArrayLikeSet(IRGS& env, SSATmp* key, SSATmp* value) {
  // We need to store to a local after doing some user-visible operations, so
  // don't go down this path for pseudomains.
  if (curFunc(env)->isPseudoMain()) return nullptr;

  auto const baseType = predictedBaseType(env);
  auto const base = extractBase(env);
  assertx(baseType <= TArrLike);

  auto const isVec = baseType <= TVec;
  auto const isDict = baseType <= TDict;
  auto const isKeyset = baseType <= TKeyset;

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

  auto const baseLoc = [&]() -> folly::Optional<Location> {
    auto const basePtr = ldMBase(env);
    auto const ptrInst = canonical(basePtr)->inst();

    switch (ptrInst->op()) {
      case LdLocAddr: {
        auto const locID = ptrInst->extra<LocalId>()->locId;
        return folly::make_optional<Location>(Location::Local { locID });
      }
      case LdStkAddr: {
        auto const irSPRel = ptrInst->extra<IRSPRelOffsetData>()->offset;
        auto const fpRel = irSPRel.to<FPInvOffset>(env.irb->fs().irSPOff());
        return folly::make_optional<Location>(Location::Stack { fpRel });
      }
      default:
        return folly::none;
    }
  }();
  if (!baseLoc) return nullptr;

  // base may be from inside a RefData inside a stack/local, so to determine
  // setRef we must check the actual value of the stack/local.
  auto const rawBaseType = provenType(env, *baseLoc);
  auto const setRef = rawBaseType <= TBoxedCell;

  if (setRef) {
    auto const box = [&] {
      switch (baseLoc->tag()) {
        case LTag::Local:
          return ldLoc(env, baseLoc->localId(), nullptr, DataTypeSpecific);
        case LTag::Stack:
          return top(env, offsetFromBCSP(env, baseLoc->stackIdx()));
        case LTag::MBase:
        case LTag::CSlotCls:
        case LTag::CSlotTS:
          always_assert(false);
      }
      not_reached();
    }();
    gen(env,
        isVec ? VecSetRef : isDict ? DictSetRef : ArraySetRef,
        base, key, value, box);

    // Unlike the non-ref case, we don't need to do anything to the stack/local
    // because any load of the box will be guarded.
    return value;
  }

  auto const newArr = gen(env,
                          isVec ? VecSet : isDict ? DictSet : ArraySet,
                          base, key, value);

  // Update the base's location with the new array.
  switch (baseLoc->tag()) {
    case LTag::Local:
      // We know it's not boxed (setRef above handles that), and the helper has
      // already decref'd the old array and incref'd newArr.
      gen(env, StLoc, LocalId { baseLoc->localId() }, fp(env), newArr);
      break;
    case LTag::Stack:
      gen(env, StStk,
          IRSPRelOffsetData { offsetFromIRSP(env, baseLoc->stackIdx()) },
          sp(env), newArr);
      break;
    case LTag::MBase:
    case LTag::CSlotCls:
    case LTag::CSlotTS:
      always_assert(false);
  }
  return value;
}

void setNewElemPackedArrayDataImpl(IRGS& env, SSATmp* basePtr, Type baseType,
                                   SSATmp* value) {
  ifThen(
    env,
    [&](Block* taken) {
      auto const base = extractBase(env);

      if ((baseType <= TArr && value->type() <= TArr) ||
          (baseType <= TVec && value->type() <= TVec)) {
        auto const appendToSelf = gen(env, EqArrayDataPtr, base, value);
        gen(env, JmpNZero, taken, appendToSelf);
      }
      gen(env, CheckArrayCOW, taken, base);
      auto const offset = gen(env, ReservePackedArrayDataNewElem, taken, base);
      auto const elemPtr = gen(
        env,
        LdPackedArrayDataElemAddr,
        TLvalToElemUninit,
        base,
        offset
      );
      gen(env, IncRef, value);
      gen(env, StMem, elemPtr, value);
    },
    [&] {
      if (baseType <= Type::Array(ArrayData::kPackedKind)) {
        gen(env, SetNewElemArray, makeCatchSet(env), basePtr, value);
      } else if (baseType <= TVec) {
        gen(env, SetNewElemVec, makeCatchSet(env), basePtr, value);
      } else {
        always_assert(false);
      }
    }
  );
}

SSATmp* setNewElemImpl(IRGS& env) {
  auto const value = topC(env);

  auto const baseType = predictedBaseType(env);

  // We load the member base pointer before calling makeCatchSet() to avoid
  // mismatched in-states for any catch block edges we emit later on.
  auto const basePtr = ldMBase(env);

  auto const gc = GuardConstraint(DataTypeSpecialized).setWantArrayKind();
  env.irb->constrainLocation(Location::MBase{}, gc);

  if (baseType <= Type::Array(ArrayData::kPackedKind) || baseType <= TVec) {
    setNewElemPackedArrayDataImpl(env, basePtr, baseType, value);
  } else if (baseType <= TArr) {
    constrainBase(env);
    gen(env, SetNewElemArray, makeCatchSet(env), basePtr, value);
  } else if (baseType <= TKeyset) {
    constrainBase(env);
    if (!value->isA(TInt | TStr)) {
      auto const base = extractBase(env);
      gen(env, ThrowInvalidArrayKey, makeCatchSet(env), base, value);
    } else {
      gen(env, SetNewElemKeyset, makeCatchSet(env), basePtr, value);
    }
  } else {
    gen(
      env,
      SetNewElem,
      makeCatchSet(env),
      basePtr,
      value,
      propStatePtrElem(env, basePtr)
    );
  }
  return value;
}

SSATmp* setElemImpl(IRGS& env, SSATmp* key) {
  auto value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);

  auto const baseType = predictedBaseType(env);
  auto const simpleOp = simpleCollectionOp(baseType, key->type(), false, false);

  if (auto gc = simpleOpConstraint(simpleOp)) {
    env.irb->constrainLocation(Location::MBase{}, *gc);
  }

  switch (simpleOp) {
    case SimpleOp::PackedArray:
    case SimpleOp::String:
      always_assert(false && "Bad SimpleOp in setElemImpl");
      break;

    case SimpleOp::Vector:
      gen(env, VectorSet, extractBase(env), key, value);
      break;

    case SimpleOp::Map:
      gen(env, MapSet, extractBase(env), key, value);
      break;

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::VecArray:
    case SimpleOp::Dict:
    case SimpleOp::Keyset:
      if (auto result = emitArrayLikeSet(env, key, value)) {
        return result;
      }
      // If we couldn't emit ArraySet, fall through to the generic path.

    case SimpleOp::Pair:
    case SimpleOp::None:
      // We load the member base pointer before calling makeCatchSet() to avoid
      // mismatched in-states for any catch block edges we emit later on.
      auto const basePtr = ldMBase(env);
      auto const result = gen(env, SetElem, makeCatchSet(env),
                              basePtr, key, value,
                              propStatePtrElem(env, basePtr));
      auto const t = result->type();
      if (t == TNullptr) {
        // Base is not a string. Result is always value.
      } else if (t == TStaticStr) {
        // Base is a string. Stack result is a new string so we're responsible
        // for decreffing value.
        env.irb->constrainValue(value, DataTypeBoxAndCountness);
        decRef(env, value);
        value = result;
      } else {
        assertx(t == (TStaticStr | TNullptr));
        // Base might be a string. Emit a check to verify the result before
        // returning the optimistic result.
        handleStrTestResult(env, result);
      }
      break;
  }

  return value;
}

SSATmp* memberKey(IRGS& env, MemberKey mk) {
  switch (mk.mcode) {
    case MW:
      return nullptr;
    case MEL: case MPL:
      return ldLocInnerWarn(env, mk.iva, makeExit(env),
                            makePseudoMainExit(env), DataTypeSpecific);
    case MEC: case MPC:
      return topC(env, BCSPRelOffset{int32_t(mk.iva)});
    case MEI:
      return cns(env, mk.int64);
    case MET: case MPT: case MQT:
      return cns(env, mk.litstr);
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

void emitBaseGC(IRGS& env, uint32_t idx, MOpMode mode) {
  initTvRefs(env);
  auto name = top(env, BCSPRelOffset{safe_cast<int32_t>(idx)});
  baseGImpl(env, name, mode);
}

void emitBaseGL(IRGS& env, int32_t locId, MOpMode mode) {
  initTvRefs(env);
  auto name = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                         DataTypeSpecific);
  baseGImpl(env, name, mode);
}

void emitBaseSC(IRGS& env, uint32_t propIdx, uint32_t slot, MOpMode mode) {
  initTvRefs(env);
  auto name = top(env, BCSPRelOffset{safe_cast<int32_t>(propIdx)});
  baseSImpl(env, name, slot, mode);
}

void emitBaseL(IRGS& env, int32_t locId, MOpMode mode) {
  initTvRefs(env);
  stMBase(env, ldLocAddr(env, locId));

  auto base = ldLoc(env, locId, makePseudoMainExit(env), DataTypeGeneric);

  if (base->isA(TUninit) && mode == MOpMode::Warn) {
    env.irb->constrainLocal(locId, DataTypeSpecific,
                            "emitBaseL: Uninit base local");
    gen(env, RaiseUninitLoc, cns(env, curFunc(env)->localVarName(locId)));
  }

  simpleBaseImpl(
    env, base, mode, Location::Local { safe_cast<uint32_t>(locId) }
  );
}

void emitBaseC(IRGS& env, uint32_t idx, MOpMode mode) {
  initTvRefs(env);

  auto const bcOff = BCSPRelOffset{safe_cast<int32_t>(idx)};
  auto const irOff = offsetFromIRSP(env, bcOff);
  stMBase(env, ldStkAddr(env, bcOff));

  auto base = topC(env, bcOff);
  simpleBaseImpl(env, base, mode, Location::Stack { offsetFromFP(env, irOff) });
}

void emitBaseH(IRGS& env) {
  if (!curClass(env)) return interpOne(env, *env.currentNormalizedInstruction);

  initTvRefs(env);
  auto base = ldThis(env);
  auto scratchPtr = misLea(env, offsetof(MInstrState, tvTempBase));
  gen(env, StMem, scratchPtr, base);
  stMBase(env, scratchPtr);
  env.irb->fs().setMemberBase(base);
  // Never write to MInstrPropState here since a base from BaseH will never
  // promote
}

void emitDim(IRGS& env, MOpMode mode, MemberKey mk) {
  // Eagerly mark us as not needing ratchets.  If the intermediate operation
  // ends up calling misLea(), this will be set to true.
  env.irb->fs().setNeedRatchet(false);

  auto key = memberKey(env, mk);
  auto newBase = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return propImpl(env, mode, key, mk.mcode == MQT);
    }
    if (mcodeIsElem(mk.mcode)) {
      auto const base = elemImpl(env, mode, key);
      setEmptyMIPropState(env, base, mode);
      return base;
    }
    PUNT(DimNewElem);
  }();

  newBase = ratchetRefs(env, newBase);
  stMBase(env, newBase);
}

void emitQueryM(IRGS& env, uint32_t nDiscard, QueryMOp query, MemberKey mk) {
  if (mk.mcode == MW) PUNT(QueryNewElem);

  auto const baseType = predictedBaseType(env);
  if (baseType <= TClsMeth) {
    PUNT(QueryM_is_ClsMeth);
  }
  auto key = memberKey(env, mk);
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
                         [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); })
          : emitCGetElem(env, maybeExtractBase(env), key, mode, simpleOp,
                         [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); });
      }

      case QueryMOp::CGetQuiet: {
        auto const mode = getQueryMOpMode(query);
        return mcodeIsProp(mk.mcode)
          ? cGetPropImpl(
              env, extractBaseIfObj(env), key, mode, mk.mcode == MQT,
              [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); }
            )
          : emitCGetElemQuiet(
              env, maybeExtractBase(env), key, mode, simpleOp,
              [&](SSATmp* el) { mFinalImpl(env, nDiscard, el); }
            );
      }

      case QueryMOp::Isset:
        return mcodeIsProp(mk.mcode)
          ? gen(env, IssetProp, extractBaseIfObj(env), key)
          : emitIssetElem(env, maybeExtractBase(env), key, simpleOp);

      case QueryMOp::Empty:
        return mcodeIsProp(mk.mcode)
          ? gen(env, EmptyProp, extractBaseIfObj(env), key)
          : emitEmptyElem(env, maybeExtractBase(env), key, simpleOp);
    }
    not_reached();
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitVGetM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto key = memberKey(env, mk);

  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      if (mk.mcode == MQT) {
        gen(env, RaiseError, cns(env, s_NULLSAFE_PROP_WRITE_ERROR.get()));
      }
      auto const base = extractBaseIfObj(env);
      return gen(env, VGetProp, base, key, propStatePtrFinalProp(env, base));
    }
    if (mcodeIsElem(mk.mcode)) {
      auto const base = ldMBase(env);
      return gen(env, VGetElem, base, key, propStatePtrElem(env, base));
    }
    PUNT(VGetNewElem);
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitSetM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto const baseType = predictedBaseType(env);
  if (baseType <= TClsMeth) {
    PUNT(SetM_is_ClsMeth);
  }

  auto const key = memberKey(env, mk);
  auto const result =
    mk.mcode == MW        ? setNewElemImpl(env) :
    mcodeIsElem(mk.mcode) ? setElemImpl(env, key) :
                            setPropImpl(env, key);

  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

void emitSetRangeM(IRGS& env, uint32_t nDiscard, SetRangeOp op, uint32_t size) {
  auto const count = gen(env, ConvCellToInt, topC(env));
  auto const src = topC(env, BCSPRelOffset{1});
  auto const offset = gen(env, ConvCellToInt, topC(env, BCSPRelOffset{2}));
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
      return gen(
        env,
        IncDecElem,
        IncDecData{incDec},
        base,
        key,
        propStatePtrElem(env, base)
      );
    }
    PUNT(IncDecNewElem);
  }();

  mFinalImpl(env, nDiscard, result);
}

/*
 * If the op and operand types are a supported combination, return the modified
 * value. Otherwise, return nullptr. The returned value always has an uncounted
 * type.
 */
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

  lhs = promoteBool(env, lhs);
  rhs = promoteBool(env, rhs);

  auto const hhirOp = isBitOp(bcOp) ? bitOp(bcOp)
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

  if (propInfo.offset != -1 &&
      !propInfo.immutable &&
      !mightCallMagicPropMethod(MOpMode::Define, propInfo)) {

    SSATmp* propPtr;
    SSATmp* obj;
    std::tie(propPtr, obj) = emitPropSpecialized(
      env,
      base,
      key,
      false,
      MOpMode::Define,
      propInfo
    );
    assertx(obj != nullptr);

    propPtr = gen(env, UnboxPtr, propPtr);

    auto const lhs = gen(env, LdMem, propPtr->type().deref(), propPtr);
    if (auto const result = inlineSetOp(env, op, lhs, rhs)) {
      verifyPropType(
        env,
        gen(env, LdObjClass, obj),
        propInfo.typeConstraint,
        propInfo.slot,
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
      if (!propInfo.typeConstraint || !propInfo.typeConstraint->isCheckable()) {
        return true;
      }
      if (op != SetOpOp::ConcatEqual) return false;
      if (propPtr->type().deref() <= TStr) return true;
      return propInfo.typeConstraint->alwaysPasses(KindOfString);
    }();

    if (!fast) {
      gen(
        env,
        SetOpCellVerify,
        SetOpData{op},
        propPtr,
        rhs,
        gen(env, LdObjClass, obj),
        cns(env, propInfo.slot)
      );
    } else {
      gen(env, SetOpCell, SetOpData{op}, propPtr, rhs);
    }
    auto newVal = gen(env, LdMem, propPtr->type().deref(), propPtr);
    auto const pNewVal = profiledType(env, newVal, [&] {
      gen(env, IncRef, newVal);
      finish(newVal);
    });
    gen(env, IncRef, pNewVal);
    return pNewVal;
  }

  return gen(
    env,
    SetOpProp,
    SetOpData{op},
    base,
    key,
    rhs,
    propStatePtrFinalProp(env, base)
  );
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
      return gen(
        env,
        SetOpElem,
        SetOpData{op},
        base,
        key,
        rhs,
        propStatePtrElem(env, base)
      );
    }
    PUNT(SetOpNewElem);
  }();

  finish(result);
}

void emitBindM(IRGS& env, uint32_t nDiscard, MemberKey mk) {
  auto key = memberKey(env, mk);
  auto rhs = topV(env);

  if (mcodeIsProp(mk.mcode)) {
    auto const base = extractBaseIfObj(env);
    gen(env, BindProp, base, key, rhs, propStatePtrFinalProp(env, base));
  } else if (mcodeIsElem(mk.mcode)) {
    auto const base = ldMBase(env);
    gen(env, BindElem, base, key, rhs, propStatePtrElem(env, base));
  } else {
    auto const base = ldMBase(env);
    gen(env, BindNewElem, base, rhs, propStatePtrElem(env, base));
  }

  popV(env);
  mFinalImpl(env, nDiscard, rhs);
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

}}}
