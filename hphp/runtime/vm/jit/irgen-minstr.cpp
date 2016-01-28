/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <type_traits>
#include <sstream>

#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/collections.h"

#include "hphp/runtime/vm/native-prop-handler.h"

#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_PackedArray("PackedArray");
const StaticString s_StructArray("StructArray");

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
  ProfiledStructArray,
  PackedArray,
  StructArray,
  String,
  Vector, // c_Vector* or c_ImmVector*
  Map,    // c_Map*
  Pair,   // c_Pair*
};

//////////////////////////////////////////////////////////////////////
// Property information.

struct PropInfo {
  PropInfo()
    : offset{-1}
    , repoAuthType{}
    , baseClass{nullptr}
  {}

  explicit PropInfo(int offset,
                    RepoAuthType repoAuthType,
                    const Class* baseClass)
    : offset{offset}
    , repoAuthType{repoAuthType}
    , baseClass{baseClass}
  {}

  int offset;
  RepoAuthType repoAuthType;
  const Class* baseClass;
};

/*
 * Try to find a property offset for the given key in baseClass. Will return a
 * PropInfo with an offset of -1 if the mapping from baseClass's name to the
 * Class* can change (which happens in sandbox mode when the ctx class is
 * unrelated to baseClass).
 */
PropInfo getPropertyOffset(IRGS& env,
                           const Class* ctx,
                           const Class* baseClass,
                           Type keyType) {
  if (!baseClass) return PropInfo();

  if (!keyType.hasConstVal(TStr)) return PropInfo();
  auto const name = keyType.strVal();

  // If we are not in repo-authoriative mode, we need to check that baseClass
  // cannot change in between requests.
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
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
  auto const idx = lookup.prop;

  // If we couldn't find a property that is accessible in the current context,
  // bail out
  if (idx == kInvalidSlot || !lookup.accessible) return PropInfo();

  // If it's a declared property we're good to go: even if a subclass redefines
  // an accessible property with the same name it's guaranteed to be at the same
  // offset.
  return PropInfo(
    baseClass->declPropOffset(idx),
    baseClass->declPropRepoAuthType(idx),
    baseClass
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

  // If the property name is known, try to look it up and get its RAT.
  if (key->hasConstVal(TStr)) {
    auto const keyStr = key->strVal();
    auto const ctx = curClass(env);
    auto const lookup = cls->getDeclPropIndex(ctx, keyStr);
    if (lookup.prop != kInvalidSlot) {
      isDeclared = true;
      if (RuntimeOption::RepoAuthoritative) {
        propType = typeFromRAT(cls->declPropRepoAuthType(lookup.prop));
      }
    }
  }

  // Magic getters/setters use tvRef if the property is unset.
  if (classMayHaveMagicPropMethods(cls) && propType.maybe(TUninit)) {
    return false;
  }

  // Native prop handlers never kick in for declared properties, even if
  // they're unset.
  if (!isDeclared && cls->hasNativePropHandler()) return false;

  env.irb->constrainValue(base, TypeConstraint(cls));
  return true;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<TypeConstraint> simpleOpConstraint(SimpleOp op) {
  switch (op) {
    case SimpleOp::None:
      return folly::none;

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::ProfiledStructArray:
    case SimpleOp::String:
      return TypeConstraint(DataTypeSpecific);

    case SimpleOp::PackedArray:
      return TypeConstraint(DataTypeSpecialized).setWantArrayKind();

    case SimpleOp::StructArray:
      return TypeConstraint(DataTypeSpecialized).setWantArrayShape();

    case SimpleOp::Vector:
      return TypeConstraint(c_Vector::classof());

    case SimpleOp::Map:
      return TypeConstraint(c_Map::classof());

    case SimpleOp::Pair:
      return TypeConstraint(c_Pair::classof());
  }

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

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
  return cns(env, Type::cns(&init_null_variant, TPtrToOtherInitNull));
}

SSATmp* ptrToUninit(IRGS& env) {
  // Nothing can write to the uninit null variant either, so the inner type
  // here is also always true.
  return cns(env, Type::cns(&null_variant, TPtrToOtherUninit));
}

bool mightCallMagicPropMethod(MInstrAttr mia, PropInfo propInfo) {
  if (!typeFromRAT(propInfo.repoAuthType).maybe(TUninit)) {
    return false;
  }
  auto const cls = propInfo.baseClass;
  if (!cls) return true;
  // NB: this function can't yet be used for unset or isset contexts.  Just get
  // and set.
  auto const relevant_attrs =
    // Magic getters can be invoked both in define contexts and non-define
    // contexts.
    AttrNoOverrideMagicGet |
    // But magic setters are only possible in define contexts.
    ((mia & MIA_define) ? AttrNoOverrideMagicSet : AttrNone);
  bool const no_override_magic =
    (cls->attrs() & relevant_attrs) == relevant_attrs;
  return !no_override_magic;
}

//////////////////////////////////////////////////////////////////////

/*
 * Punt if the given base type isn't known to be boxed or unboxed.
 */
void checkGenBase(Type baseType) {
  if (baseType.maybe(TCell) && baseType.maybe(TBoxedCell)) {
    PUNT(MInstr-GenBase);
  }
}

/*
 * This is called in a few places to be consistent with old minstrs, and should
 * be revisited once they're gone. It probably doesn't make sense to always
 * guard on an object class when we have one.
 */
void specializeObjBase(IRGS& env, SSATmp* base) {
  if (base && base->isA(TObj) && base->type().clsSpec().cls()) {
    env.irb->constrainValue(base, TypeConstraint(base->type().clsSpec().cls()));
  }
}

//////////////////////////////////////////////////////////////////////
// Intermediate ops

PropInfo getCurrentPropertyOffset(IRGS& env, SSATmp* base,
                                  Type baseType, Type keyType) {
  // We allow the use of clases from nullable objects because
  // emitPropSpecialized() explicitly checks for null (when needed) before
  // doing the property access.
  baseType = baseType.derefIfPtr();
  if (!(baseType < (TObj | TInitNull) && baseType.clsSpec())) return PropInfo{};

  auto const baseCls = baseType.clsSpec().cls();
  auto const info = getPropertyOffset(env, curClass(env), baseCls, keyType);
  if (info.offset == -1) return info;

  if (env.irb->constrainValue(base,
                              TypeConstraint(baseCls).setWeak())) {
    // We can't use this specialized class without making a guard more
    // expensive, so don't do it.
    return PropInfo{};
  }

  return info;
}

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It returns
 * a pointer to the property's address, or init_null_variant if the property
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
  assertx(propAddr->type() <= TPtrToGen);
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
    [&] { // Taken: Property is Uninit. Raise a warning and return a pointer to
          // init_null_variant.
      hint(env, Block::Hint::Unlikely);
      if (wantPropSpecializedWarnings()) {
        gen(env, RaiseUndefProp, baseAsObj, key);
      }
      return ptrToInitNull(env);
    }
  );
}

SSATmp* emitPropSpecialized(
  IRGS& env,
  SSATmp* base,
  Type baseType,
  SSATmp* key,
  bool nullsafe,
  const MInstrAttr mia,
  PropInfo propInfo
) {
  assertx(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  auto const initNull = ptrToInitNull(env);

  /*
   * Normal case, where the base is an object (and not a pointer to
   * something)---just do a lea with the type information we got from static
   * analysis.  The caller of this function will use it to know whether it can
   * avoid a generic incref, unbox, etc.
   */
  if (baseType <= TObj) {
    auto const propAddr = gen(
      env,
      LdPropAddr,
      PropOffset { propInfo.offset },
      typeFromRAT(propInfo.repoAuthType).ptr(Ptr::Prop),
      base
    );
    return checkInitProp(env, base, propAddr, key, doWarn, doDefine);
  }

  /*
   * We also support nullable objects for the base.  This is a frequent result
   * of static analysis on multi-dim property accesses ($foo->bar->baz), since
   * hhbbc doesn't try to prove __construct must be run or that sort of thing
   * (so every object-holding object property can also be null).
   *
   * After a null check, if it's actually an object we can just do LdPropAddr,
   * otherwise we just give out a pointer to the init_null_variant (after
   * raising the appropriate warnings).
   */
  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, TObj, taken, base);
    },
    [&] {
      // Next: Base is an object. Load property and check for uninit.
      auto const obj = gen(env, LdMem, baseType.deref() & TObj, base);
      auto const propAddr = gen(
        env,
        LdPropAddr,
        PropOffset { propInfo.offset },
        typeFromRAT(propInfo.repoAuthType).ptr(Ptr::Prop),
        obj
      );
      return checkInitProp(env, obj, propAddr, key, doWarn, doDefine);
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
}

//////////////////////////////////////////////////////////////////////
// "Simple op" handlers.

SSATmp* emitPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TArr) &&
          base->type().arrSpec().kind() == ArrayData::kPackedKind &&
          key->isA(TInt));

  auto doLdElem = [&] {
    auto const type = packedArrayElemType(base, key).ptr(Ptr::Elem);
    auto addr = gen(env, LdPackedArrayElemAddr, type, base, key);
    auto res = gen(env, LdMem, type.deref(), addr);
    auto unboxed = unbox(env, res, nullptr);
    gen(env, IncRef, unboxed);
    return unboxed;
  };

  if (key->hasConstVal()) {
    int64_t idx = key->intVal();
    if (base->hasConstVal()) {
      const ArrayData* arr = base->arrVal();
      if (idx < 0 || idx >= arr->size()) {
        gen(env, RaiseArrayIndexNotice, key);
        return cns(env, TInitNull);
      }
      auto const value = arr->nvGet(idx);
      return cns(env, *value);
    }

    switch (packedArrayBoundsStaticCheck(base->type(), idx)) {
    case PackedBounds::In:
      return doLdElem();
    case PackedBounds::Out:
      gen(env, RaiseArrayIndexNotice, key);
      return cns(env, TInitNull);
    case PackedBounds::Unknown:
      break;
    }
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      return doLdElem();
    },
    [&] { // Taken:
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseArrayIndexNotice, key);
      return cns(env, TInitNull);
    }
  );
}

SSATmp* emitStructArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TArr));
  assertx(base->type().arrSpec().kind() == ArrayData::kStructKind);
  assertx(base->type().arrSpec().shape());
  assertx(key->hasConstVal(TStr));
  assertx(key->strVal()->isStatic());

  const auto keyStr = key->strVal();
  const auto shape = base->type().arrSpec().shape();
  auto offset = shape->offsetFor(keyStr);

  if (offset == PropertyTable::kInvalidOffset) {
    gen(env, RaiseArrayKeyNotice, key);
    return cns(env, TInitNull);
  }

  auto res = gen(env, LdStructArrayElem, base, key);
  auto unboxed = unbox(env, res, nullptr);
  gen(env, IncRef, unboxed);
  return unboxed;
}

SSATmp* emitArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  auto elem = unbox(env, gen(env, ArrayGet, base, key), nullptr);
  gen(env, IncRef, elem);
  return elem;
}

SSATmp* emitProfiledPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  TargetProfile<NonPackedArrayProfile> prof(env.context,
                                            env.irb->curMarker(),
                                            s_PackedArray.get());
  if (prof.profiling()) {
    gen(env, ProfilePackedArray, RDSHandleData{prof.handle()}, base);
    return emitArrayGet(env, base, key);
  }

  if (prof.optimizing()) {
    auto const data = prof.data(NonPackedArrayProfile::reduce);
    // NonPackedArrayProfile data counts how many times a non-packed array was
    // observed.  Zero means it was monomorphic (or never executed).
    auto const typePackedArr = Type::Array(ArrayData::kPackedKind);
    if (base->type().maybe(typePackedArr) &&
        (data.count == 0 || RuntimeOption::EvalJitPGOArrayGetStress)) {
      // It's safe to side-exit still because we only do these profiled array
      // gets on the first element, with simple bases and single-element dims.
      // See computeSimpleCollectionOp.
      auto const exit = makeExit(env);
      base = gen(env, CheckType, typePackedArr, exit, base);
      env.irb->constrainValue(
        base,
        TypeConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      return emitPackedArrayGet(env, base, key);
    }
  }

  // Fall back to a generic array get.
  return emitArrayGet(env, base, key);
}

SSATmp* emitProfiledStructArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  TargetProfile<StructArrayProfile> prof(env.context,
                                         env.irb->curMarker(),
                                         s_StructArray.get());
  if (prof.profiling()) {
    gen(env, ProfileStructArray, RDSHandleData{prof.handle()}, base);
    return emitArrayGet(env, base, key);
  }

  if (prof.optimizing()) {
    auto const data = prof.data(StructArrayProfile::reduce);
    // StructArrayProfile data counts how many times a non-struct array was
    // observed.  Zero means it was monomorphic (or never executed).
    //
    // It also records how many Shapes it saw. The possible values are:
    //  0 (never executed)
    //  1 (monomorphic)
    //  many (polymorphic)
    //
    // If we never executed then we fall back to generic get. If we're
    // monomorphic, we'll emit a check for that specific Shape. If we're
    // polymorphic, we'll also fall back to generic get. Eventually we'd like
    // to emit an inline cache, which should be faster than calling out of line.
    if (base->type().maybe(Type::Array(ArrayData::kStructKind))) {
      if (data.nonStructCount == 0 && data.isMonomorphic()) {
        // It's safe to side-exit still because we only do these profiled array
        // gets on the first element, with simple bases and single-element dims.
        // See computeSimpleCollectionOp.
        auto const exit = makeExit(env);
        base = gen(env, CheckType, Type::Array(data.getShape()), exit, base);
        env.irb->constrainValue(
          base,
          TypeConstraint(DataTypeSpecialized).setWantArrayShape()
        );
        return emitStructArrayGet(env, base, key);
      }
    }
  }

  // Fall back to a generic array get.
  return emitArrayGet(env, base, key);
}

void checkBounds(IRGS& env, SSATmp* idx, SSATmp* limit) {
  ifThen(
    env,
    [&](Block* taken) {
      auto ok = gen(env, CheckRange, idx, limit);
      gen(env, JmpZero, taken, ok);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowOutOfBounds, idx);
    }
  );
}

SSATmp* emitVectorGet(IRGS& env, SSATmp* base, SSATmp* key) {
  auto const size = gen(env, LdVectorSize, base);
  checkBounds(env, key, size);
  base = gen(env, LdVectorBase, base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(env, Shl, key, cns(env, 4));
  auto result = gen(env, LdElem, base, idx);
  gen(env, IncRef, result);
  return result;
}

SSATmp* emitPairGet(IRGS& env, SSATmp* base, SSATmp* key) {
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
    checkBounds(env, key, cns(env, 2));
    return gen(env, Shl, key, cns(env, 4));
  }();

  auto const pairBase = gen(env, LdPairBase, base);
  auto const result = gen(env, LdElem, pairBase, idx);
  gen(env, IncRef, result);
  return result;
}

SSATmp* emitPackedArrayIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->type().arrSpec().kind() == ArrayData::kPackedKind);

  auto const type = packedArrayElemType(base, key);
  if (type <= TNull) return cns(env, false);

  if (key->hasConstVal()) {
    auto const idx = key->intVal();
    switch (packedArrayBoundsStaticCheck(base->type(), idx)) {
    case PackedBounds::In: {
      if (!type.maybe(TNull)) return cns(env, true);

      auto const elemAddr = gen(env, LdPackedArrayElemAddr,
                                type.ptr(Ptr::Elem), base, key);
      return gen(env, IsNTypeMem, TNull, elemAddr);
    }
    case PackedBounds::Out:
      return cns(env, false);
    case PackedBounds::Unknown:
      break;
    }
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      auto const elemAddr = gen(env, LdPackedArrayElemAddr,
                                type.ptr(Ptr::Elem), base, key);
      return gen(env, IsNTypeMem, TNull, elemAddr);
    },
    [&] { // Taken:
      return cns(env, false);
    }
  );
}

void emitVectorSet(IRGS& env, SSATmp* base, SSATmp* key, SSATmp* value) {
  auto const size = gen(env, LdVectorSize, base);
  checkBounds(env, key, size);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, VectorHasImmCopy, taken, base);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, VectorDoCow, base);
    }
  );

  gen(env, IncRef, value);
  auto const vecBase = gen(env, LdVectorBase, base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto const idx = gen(env, Shl, key, cns(env, 4));
  auto const oldVal = gen(env, LdElem, vecBase, idx);
  gen(env, StElem, vecBase, idx, value);
  decRef(env, oldVal);
}

//////////////////////////////////////////////////////////////////////

SSATmp* emitIncDecProp(IRGS& env, IncDecOp op, SSATmp* base, SSATmp* key) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());

  if (RuntimeOption::RepoAuthoritative &&
      propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, propInfo) &&
      !mightCallMagicPropMethod(MIA_define, propInfo)) {

    // Special case for when the property is known to be an int.
    if (base->isA(TObj) &&
        propInfo.repoAuthType.tag() == RepoAuthType::Tag::Int) {
      base = emitPropSpecialized(env, base, base->type(), key, false,
                                 MIA_define, propInfo);
      auto const prop = gen(env, LdMem, TInt, base);
      auto const result = incDec(env, op, prop);
      assertx(result != nullptr);
      gen(env, StMem, base, result);
      return isPre(op) ? result : prop;
    }
  }

  return gen(env, IncDecProp, IncDecData{op}, base, key);
}

SSATmp* emitCGetElem(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpFlags flags, SimpleOp simpleOp) {
  switch (simpleOp) {
    case SimpleOp::Array:
      return emitArrayGet(env, base, key);
    case SimpleOp::PackedArray:
      return emitPackedArrayGet(env, base, key);
    case SimpleOp::StructArray:
      return emitStructArrayGet(env, base, key);
    case SimpleOp::ProfiledPackedArray:
      return emitProfiledPackedArrayGet(env, base, key);
    case SimpleOp::ProfiledStructArray:
      return emitProfiledStructArrayGet(env, base, key);
    case SimpleOp::String:
      return gen(env, StringGet, base, key);
    case SimpleOp::Vector:
      return emitVectorGet(env, base, key);
    case SimpleOp::Pair:
      return emitPairGet(env, base, key);
    case SimpleOp::Map:
      return gen(env, MapGet, base, key);
    case SimpleOp::None:
      return gen(env, CGetElem, MInstrAttrData{mOpFlagsToAttr(flags)},
                 base, key);
  }
  always_assert(false);
}

SSATmp* emitIssetElem(IRGS& env, SSATmp* base, SSATmp* key, SimpleOp simpleOp) {
  switch (simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::StructArray:
  case SimpleOp::ProfiledPackedArray:
  case SimpleOp::ProfiledStructArray:
    return gen(env, ArrayIsset, base, key);
  case SimpleOp::PackedArray:
    return emitPackedArrayIsset(env, base, key);
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

void setWithRefImpl(IRGS& env, int32_t keyLoc, SSATmp* value) {
  auto const key = ldLoc(env, keyLoc, nullptr, DataTypeGeneric);
  gen(env, SetWithRefElem, gen(env, LdMBase, TPtrToGen), key, value);
}

/*
 * Determine which simple collection op to use for the given base and key
 * types.
 */
SimpleOp simpleCollectionOp(Type baseType, Type keyType, bool readInst) {
  if (baseType <= TArr) {
    auto isPacked = false;
    auto isStruct = false;
    if (auto arrSpec = baseType.arrSpec()) {
      isPacked = arrSpec.kind() == ArrayData::kPackedKind;
      isStruct = arrSpec.kind() == ArrayData::kStructKind &&
                 arrSpec.shape() != nullptr;
    }
    if (keyType <= TInt || keyType <= TStr) {
      if (readInst) {
        if (keyType <= TInt) {
          return isPacked ? SimpleOp::PackedArray
                          : SimpleOp::ProfiledPackedArray;
        } else if (keyType.hasConstVal(TStaticStr)) {
          if (!isStruct || !baseType.arrSpec().shape()) {
            return SimpleOp::ProfiledStructArray;
          }
          return SimpleOp::StructArray;
        }
      }
      return SimpleOp::Array;
    }
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
    [&](Block* taken) {
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
      return tvRef2Ptr(env);
    }
  );
}

void baseGImpl(IRGS& env, SSATmp* name, MOpFlags flags) {
  if (!name->isA(TStr)) PUNT(BaseG-non-string-name);
  auto gblPtr = gen(env, BaseG, MInstrAttrData{mOpFlagsToAttr(flags)}, name);
  gen(env, StMBase, gblPtr);
}

void baseSImpl(IRGS& env, SSATmp* name, int32_t clsIdx) {
  auto cls = topA(env, BCSPOffset{clsIdx});
  auto spropPtr = ldClsPropAddr(env, cls, name, true);
  gen(env, StMBase, spropPtr);

  if (clsIdx == 1) {
    auto rhs = pop(env, DataTypeGeneric);
    popA(env);
    push(env, rhs);
  } else {
    popA(env);
  }
}

void simpleBaseImpl(IRGS& env, SSATmp* base, Type innerTy) {
  checkGenBase(base->type());

  if (base->isA(TBoxedCell)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    gen(env, CheckRefInner, innerTy, makeExit(env), base);
    base = gen(env, LdRef, innerTy, base);
  }

  env.irb->fs().setMemberBaseValue(base);

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  env.irb->constrainValue(base, DataTypeSpecific);
}

SSATmp* propGenericImpl(IRGS& env, MInstrAttr mia, SSATmp* base, SSATmp* key,
                        bool nullsafe) {
  auto const miaData = MInstrAttrData{mia};
  auto const define = bool(mia & MIA_define);

  if (define && nullsafe) {
    gen(env, RaiseError,
        cns(env, makeStaticString(Strings::NULLSAFE_PROP_WRITE_ERROR)));
    return ptrToInitNull(env);
  }

  auto const tvRef = propTvRefPtr(env, base, key);
  return nullsafe
    ? gen(env, PropQ, base, key, tvRef)
    : gen(env, define ? PropDX : PropX, miaData, base, key, tvRef);
}

SSATmp* propImpl(IRGS& env, MOpFlags flags, SSATmp* key, bool nullsafe) {
  auto base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const baseType = base ? base->type() : basePtr->type().deref();

  if ((flags & MOpFlags::Unset) && !baseType.maybe(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    return ptrToInitNull(env);
  }

  if (base && base->isA(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
  } else {
    base = basePtr;
  }

  specializeObjBase(env, base);

  auto const mia = MInstrAttr(mOpFlagsToAttr(flags) & MIA_intermediate_prop);
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());
  if (propInfo.offset == -1 || (flags & MOpFlags::Unset) ||
      mightCallMagicPropMethod(mia, propInfo)) {
    return propGenericImpl(env, mia, base, key, nullsafe);
  }

  return emitPropSpecialized(env, base, base->type(), key,
                             nullsafe, mia, propInfo);
}

SSATmp* elemImpl(IRGS& env, MOpFlags flags, SSATmp* key) {
  auto const warn = flags & MOpFlags::Warn;
  auto const unset = flags & MOpFlags::Unset;
  auto const define = flags & MOpFlags::Define;
  auto const base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const baseType = base ? base->type() : basePtr->type().deref();

  assertx(!define || !unset);
  assertx(!define || !warn);

  if (base && base->isA(TArr) && key->type().subtypeOfAny(TInt, TStr)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    if (define || unset) {
      return gen(env, unset ? ElemArrayU : ElemArrayD, basePtr, key);
    }
    return gen(env, warn ? ElemArrayW : ElemArray, base, key);
  }

  if (unset) {
    env.irb->constrainValue(base, DataTypeSpecific);
    if (baseType <= TStr) {
      gen(env, RaiseError,
          cns(env, makeStaticString(Strings::OP_NOT_SUPPORTED_STRING)));
      return ptrToUninit(env);
    }

    if (!baseType.maybe(TArr | TObj)) {
      return ptrToUninit(env);
    }
  }

  auto const miaData = MInstrAttrData{mOpFlagsToAttr(flags)};
  auto const op = define ? ElemDX : unset ? ElemUX : ElemX;
  return gen(env, op, miaData, basePtr, key, tvRefPtr(env));
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

SSATmp* cGetPropImpl(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpFlags flags, bool nullsafe) {
  specializeObjBase(env, base);
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());
  auto const mia = mOpFlagsToAttr(flags);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, propInfo)) {
    auto propAddr = emitPropSpecialized(
      env, base, base->type(), key,
      nullsafe, mia, propInfo
    );

    if (!RuntimeOption::RepoAuthoritative) {
      auto const cellPtr = gen(env, UnboxPtr, base);
      auto const result = gen(env, LdMem, TCell, cellPtr);
      gen(env, IncRef, result);
      return result;
    }

    auto const ty = propAddr->type().deref();
    auto const cellPtr = ty.maybe(TBoxedCell)
      ? gen(env, UnboxPtr, propAddr)
      : propAddr;

    auto const result = gen(env, LdMem, ty.unbox(), cellPtr);
    gen(env, IncRef, result);
    return result;
  }

  // No warning takes precedence over nullsafe
  if (!nullsafe || !(mia & MIA_warn)) {
    return gen(env, CGetProp, MInstrAttrData{mia}, base, key);
  }
  return gen(env, CGetPropQ, base, key);
}

Block* makeCatchSet(IRGS& env, bool isSetWithRef = false) {
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
      gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
        fp(env), sp(env));
    }
  );

  // Fallthrough from here on is side-exiting due to an InvalidSetMException.
  hint(env, Block::Hint::Unused);

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  if (!isSetWithRef) {
    popDecRef(env, DataTypeGeneric);
  }
  auto const nDiscard = env.currentNormalizedInstruction->imm[0].u_IVA;
  for (int i = 0; i < nDiscard; ++i) {
    popDecRef(env, DataTypeGeneric);
  }
  cleanTvRefs(env);
  if (!isSetWithRef) {
    auto const val = gen(env, LdUnwinderValue, TCell);
    push(env, val);
  }

  // The minstr is done here, so we want to drop a FinishMemberOp to kill off
  // stores to MIState.
  gen(env, FinishMemberOp);

  gen(env, Jmp, makeExit(env, nextBcOff(env)));
  return block;
}

SSATmp* setPropImpl(IRGS& env, SSATmp* key) {
  auto const value = topC(env, BCSPOffset{0}, DataTypeGeneric);
  auto base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);

  if (base && base->isA(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
  } else {
    base = basePtr;
  }

  specializeObjBase(env, base);

  auto const mia = MIA_define;
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_define, propInfo)) {
    auto propPtr =
      emitPropSpecialized(env, base, base->type(), key, false, mia, propInfo);
    auto propTy = propPtr->type().deref();

    if (propTy.maybe(TBoxedCell)) {
      propTy = propTy.unbox();
      propPtr = gen(env, UnboxPtr, propPtr);
    }

    env.irb->constrainValue(value, DataTypeCountness);
    auto const oldVal = gen(env, LdMem, propTy, propPtr);
    gen(env, IncRef, value);
    gen(env, StMem, propPtr, value);
    decRef(env, oldVal);
  } else {
    gen(env, SetProp, makeCatchSet(env), base, key, value);
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

SSATmp* emitArraySet(IRGS& env, SSATmp* key, SSATmp* value) {
  // We need to store to a local after doing some user-visible operations, so
  // don't go down this path for pseudomains.
  if (curFunc(env)->isPseudoMain()) return nullptr;

  auto const base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const ptrInst = basePtr->inst();
  Location baseLoc;
  if (ptrInst->is(LdLocAddr)) {
    auto const id = ptrInst->extra<LocalId>()->locId;
    baseLoc = Location{Location::Local, id};
  } else if (ptrInst->is(LdStkAddr)) {
    auto const irOff = ptrInst->extra<IRSPOffsetData>()->offset;
    baseLoc = Location{offsetFromBCSP(env, irOff)};
  } else {
    return nullptr;
  }

  // base may be from inside a RefData inside a stack/local, so to determine
  // setRef we must check the actual value of the stack/local.
  auto const rawBaseType = provenTypeFromLocation(env, baseLoc);
  auto const setRef = rawBaseType <= TBoxedCell;

  if (setRef) {
    auto const box = baseLoc.space == Location::Local ?
      ldLoc(env, baseLoc.offset, nullptr, DataTypeSpecific) :
      top(env, baseLoc.bcRelOffset, DataTypeSpecific);
    gen(env, ArraySetRef, base, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack/local
    // because any load of the box will be guarded.
    return value;
  }

  auto const newArr = gen(env, ArraySet, base, key, value);

  // Update the base's location with the new array
  if (baseLoc.space == Location::Local) {
    // We know it's not boxed (setRef above handles that), and the helper has
    // already decref'd the old array and incref'd newArr.
    gen(env, StLoc, LocalId(baseLoc.offset), fp(env), newArr);
  } else if (baseLoc.space == Location::Stack) {
    auto const offset = offsetFromIRSP(env, baseLoc.bcRelOffset);
    gen(env, StStk, IRSPOffsetData{offset}, sp(env), newArr);
  } else {
    always_assert(false);
  }

  return value;
}

SSATmp* setNewElemImpl(IRGS& env) {
  auto const value = topC(env);
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const base = env.irb->fs().memberBaseValue();

  if (base && base->isA(TArr)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    gen(env, SetNewElemArray, makeCatchSet(env), basePtr, value);
  } else {
    gen(env, SetNewElem, makeCatchSet(env), basePtr, value);
  }
  return value;
}

SSATmp* setElemImpl(IRGS& env, SSATmp* key) {
  auto value = topC(env, BCSPOffset{0}, DataTypeGeneric);
  auto const base = env.irb->fs().memberBaseValue();
  auto const simpleOp =
    base ? simpleCollectionOp(base->type(), key->type(), false)
         : SimpleOp::None;

  if (auto tc = simpleOpConstraint(simpleOp)) {
    env.irb->constrainValue(base, *tc);
  }

  switch (simpleOp) {
    case SimpleOp::PackedArray:
    case SimpleOp::StructArray:
    case SimpleOp::String:
      always_assert(false && "Bad SimpleOp in setElemImpl");
      break;

    case SimpleOp::Vector:
      emitVectorSet(env, base, key, value);
      break;

    case SimpleOp::Map:
      gen(env, MapSet, base, key, value);
      break;

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::ProfiledStructArray:
      if (auto result = emitArraySet(env, key, value)) {
        return result;
      }
      // If we couldn't emit ArraySet, fall through to the generic path.

    case SimpleOp::Pair:
    case SimpleOp::None:
      auto const result =
        gen(env, SetElem, makeCatchSet(env), gen(env, LdMBase, TPtrToGen),
            key, value);
      auto const t = result->type();
      if (t == TNullptr) {
        // Base is not a string. Result is always value.
      } else if (t == TCountedStr) {
        // Base is a string. Stack result is a new string so we're responsible
        // for decreffing value.
        env.irb->constrainValue(value, DataTypeCountness);
        decRef(env, value);
        value = result;
      } else {
        assertx(t == (TCountedStr | TNullptr));
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
      return topC(env, BCSPOffset{int32_t(mk.iva)});
    case MEI:
      return cns(env, mk.int64);
    case MET: case MPT: case MQT:
      return cns(env, mk.litstr);
  }
  not_reached();
}

MOpFlags fpassFlags(IRGS& env, int32_t idx) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    return MOpFlags::DefineReffy;
  }
  return MOpFlags::Warn;
}

}

void emitBaseNC(IRGS& env, int32_t idx, MOpFlags flags) {
  interpOne(env, *env.currentNormalizedInstruction);
}

void emitBaseNL(IRGS& env, int32_t locId, MOpFlags flags) {
  interpOne(env, *env.currentNormalizedInstruction);
}

void emitFPassBaseNC(IRGS& env, int32_t arg, int32_t idx) {
  emitBaseNC(env, idx, fpassFlags(env, arg));
}

void emitFPassBaseNL(IRGS& env, int32_t arg, int32_t locId) {
  emitBaseNL(env, locId, fpassFlags(env, arg));
}

void emitBaseGC(IRGS& env, int32_t idx, MOpFlags flags) {
  initTvRefs(env);
  auto name = top(env, BCSPOffset{idx});
  baseGImpl(env, name, flags);
}

void emitBaseGL(IRGS& env, int32_t locId, MOpFlags flags) {
  initTvRefs(env);
  auto name = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                         DataTypeSpecific);
  baseGImpl(env, name, flags);
}

void emitFPassBaseGC(IRGS& env, int32_t arg, int32_t idx) {
  emitBaseGC(env, idx, fpassFlags(env, arg));
}

void emitFPassBaseGL(IRGS& env, int32_t arg, int32_t locId) {
  emitBaseGL(env, locId, fpassFlags(env, arg));
}

void emitBaseSC(IRGS& env, int32_t propIdx, int32_t clsIdx) {
  initTvRefs(env);
  auto name = top(env, BCSPOffset{propIdx});
  baseSImpl(env, name, clsIdx);
}

void emitBaseSL(IRGS& env, int32_t locId, int32_t clsIdx) {
  initTvRefs(env);
  auto name = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                         DataTypeSpecific);
  baseSImpl(env, name, clsIdx);
}

void emitBaseL(IRGS& env, int32_t locId, MOpFlags flags) {
  initTvRefs(env);
  gen(env, StMBase, ldLocAddr(env, locId));

  auto base = ldLoc(env, locId, makePseudoMainExit(env), DataTypeGeneric);

  if (base->isA(TUninit) && (flags & MOpFlags::Warn)) {
    env.irb->constrainLocal(locId, DataTypeSpecific,
                            "emitBaseL: Uninit base local");
    gen(env, RaiseUninitLoc, cns(env, curFunc(env)->localVarName(locId)));
  }

  auto innerTy = base->isA(TBoxedCell) ? env.irb->predictedInnerType(locId)
                                       : TTop;
  simpleBaseImpl(env, base, innerTy);
}

void emitFPassBaseL(IRGS& env, int32_t arg, int32_t locId) {
  emitBaseL(env, locId, fpassFlags(env, arg));
}

void emitBaseC(IRGS& env, int32_t idx) {
  initTvRefs(env);

  auto const bcOff = BCSPOffset{idx};
  auto const irOff = offsetFromIRSP(env, bcOff);
  gen(env, StMBase, ldStkAddr(env, bcOff));

  auto base = top(env, bcOff);
  auto innerTy = base->isA(TBoxedCell) ? env.irb->predictedStackInnerType(irOff)
                                       : TTop;
  simpleBaseImpl(env, top(env, bcOff), innerTy);
}

void emitBaseR(IRGS& env, int32_t idx) {
  emitBaseC(env, idx);
}

void emitBaseH(IRGS& env) {
  if (!curClass(env)) return interpOne(env, *env.currentNormalizedInstruction);

  initTvRefs(env);
  auto base = ldThis(env);
  auto scratchPtr = misLea(env, offsetof(MInstrState, tvTempBase));
  gen(env, StMem, scratchPtr, base);
  gen(env, StMBase, scratchPtr);
  env.irb->fs().setMemberBaseValue(base);
}

void emitDim(IRGS& env, MOpFlags flags, MemberKey mk) {
  // Eagerly mark us as not needing ratchets.  If the intermediate operation
  // ends up calling misLea(), this will be set to true.
  env.irb->fs().setNeedRatchet(false);

  auto key = memberKey(env, mk);
  auto newBase = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return propImpl(env, flags, key, mk.mcode == MQT);
    }
    if (mcodeIsElem(mk.mcode)) {
      return elemImpl(env, flags, key);
    }
    PUNT(DimNewElem);
  }();

  newBase = ratchetRefs(env, newBase);
  gen(env, StMBase, newBase);
}

void emitFPassDim(IRGS& env, int32_t arg, MemberKey mk) {
  emitDim(env, fpassFlags(env, arg), mk);
}

void emitQueryM(IRGS& env, int32_t nDiscard, QueryMOp query, MemberKey mk) {
  if (mk.mcode == MW) PUNT(QueryNewElem);

  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto objBase = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);
  auto simpleOp = SimpleOp::None;

  if (base && mcodeIsElem(mk.mcode) &&
      query != QueryMOp::Empty && query != QueryMOp::CGetQuiet) {
    simpleOp = simpleCollectionOp(base->type(), key->type(), true);

    if (auto tc = simpleOpConstraint(simpleOp)) {
      env.irb->constrainValue(base, *tc);
    }
  }

  auto const result = [&] {
    switch (query) {
      case QueryMOp::CGet:
      case QueryMOp::CGetQuiet: {
        auto const flags = getQueryMOpFlags(query);
        if (mcodeIsProp(mk.mcode)) {
          return cGetPropImpl(env, objBase, key, flags, mk.mcode == MQT);
        }
        auto const realBase = simpleOp == SimpleOp::None ? basePtr : base;
        return emitCGetElem(env, realBase, key, flags, simpleOp);
      }

      case QueryMOp::Isset: {
        if (mcodeIsProp(mk.mcode)) {
          return gen(env, IssetProp, objBase, key);
        }
        auto const realBase = simpleOp == SimpleOp::None ? basePtr : base;
        return emitIssetElem(env, realBase, key, simpleOp);
      }

      case QueryMOp::Empty:
        return mcodeIsProp(mk.mcode) ? gen(env, EmptyProp, objBase, key)
                                     : gen(env, EmptyElem, basePtr, key);
    }
    not_reached();
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitVGetM(IRGS& env, int32_t nDiscard, MemberKey mk) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto baseObj = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);

  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      if (mk.mcode == MQT) {
        gen(env, RaiseError,
            cns(env, makeStaticString(Strings::NULLSAFE_PROP_WRITE_ERROR)));
      }
      return gen(env, VGetProp, baseObj, key);
    }
    if (mcodeIsElem(mk.mcode)) {
      return gen(env, VGetElem, basePtr, key);
    }
    PUNT(VGetNewElem);
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitFPassM(IRGS& env, int32_t arg, int32_t nDiscard, MemberKey mk) {
  if (fpassFlags(env, arg) == MOpFlags::Warn) {
    return emitQueryM(env, nDiscard, QueryMOp::CGet, mk);
  }
  emitVGetM(env, nDiscard, mk);
}

void emitSetM(IRGS& env, int32_t nDiscard, MemberKey mk) {
  auto const key = memberKey(env, mk);
  auto const result =
    mk.mcode == MW        ? setNewElemImpl(env) :
    mcodeIsElem(mk.mcode) ? setElemImpl(env, key) :
                            setPropImpl(env, key);

  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

void emitIncDecM(IRGS& env, int32_t nDiscard, IncDecOp incDec, MemberKey mk) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto baseObj = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);

  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return emitIncDecProp(env, incDec, baseObj, key);
    }
    if (mcodeIsElem(mk.mcode)) {
      return gen(env, IncDecElem, IncDecData{incDec}, basePtr, key);
    }
    PUNT(IncDecNewElem);
  }();

  mFinalImpl(env, nDiscard, result);
}

void emitSetOpM(IRGS& env, int32_t nDiscard, SetOpOp op, MemberKey mk) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto baseObj = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);
  auto rhs = topC(env);

  auto const result = [&] {
    if (mcodeIsProp(mk.mcode)) {
      return gen(env, SetOpProp, SetOpData{op}, baseObj, key, rhs);
    }
    if (mcodeIsElem(mk.mcode)) {
      return gen(env, SetOpElem, SetOpData{op}, basePtr, key, rhs);
    }
    PUNT(SetOpNewElem);
  }();

  popDecRef(env);
  mFinalImpl(env, nDiscard, result);
}

void emitBindM(IRGS& env, int32_t nDiscard, MemberKey mk) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto baseObj = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);
  auto rhs = topV(env);

  if (mcodeIsProp(mk.mcode)) {
    gen(env, BindProp, baseObj, key, rhs);
  } else if (mcodeIsElem(mk.mcode)) {
    gen(env, BindElem, basePtr, key, rhs);
  } else {
    gen(env, BindNewElem, basePtr, rhs);
  }

  popV(env);
  mFinalImpl(env, nDiscard, rhs);
}

void emitUnsetM(IRGS& env, int32_t nDiscard, MemberKey mk) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto baseObj = base && base->isA(TObj) ? base : basePtr;
  auto key = memberKey(env, mk);

  if (mcodeIsProp(mk.mcode)) {
    gen(env, UnsetProp, baseObj, key);
  } else {
    assert(mcodeIsElem(mk.mcode));
    gen(env, UnsetElem, basePtr, key);
  }

  mFinalImpl(env, nDiscard, nullptr);
}

void emitSetWithRefLML(IRGS& env, int32_t keyLoc, int32_t valLoc) {
  setWithRefImpl(env, keyLoc, ldLoc(env, valLoc, nullptr, DataTypeGeneric));
  mFinalImpl(env, 0, nullptr);
}

void emitSetWithRefRML(IRGS& env, int32_t keyLoc) {
  setWithRefImpl(env, keyLoc, top(env, BCSPOffset{0}, DataTypeGeneric));
  popDecRef(env);
  mFinalImpl(env, 0, nullptr);
}

//////////////////////////////////////////////////////////////////////

}}}
