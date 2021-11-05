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
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/is-type-struct-profile.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_StringishObject("StringishObject"),
  s_Awaitable("HH\\Awaitable"),
  s_CLASS_TO_STRING_IMPLICIT(Strings::CLASS_TO_STRING_IMPLICIT),
  s_CLASS_TO_CLASSNAME(Strings::CLASS_TO_CLASSNAME);

//////////////////////////////////////////////////////////////////////

/*
 * Returns a {Cls|Nullptr} suitable for use in instance checks. If knownCls is
 * not null and is safe to use, that will be returned. Otherwise, className
 * will be used to look up a class.
 */
SSATmp* ldClassSafe(IRGS& env, const StringData* className,
                    const Class* knownCls = nullptr) {
  if (!knownCls) {
    knownCls = lookupUniqueClass(env, className);
  }

  if (knownCls) {
    return cns(env, knownCls);
  }

  return cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdClsCachedSafe, taken, cns(env, className));
    },
    [&] (SSATmp* cls) { // next
      return cls;
    },
    [&] { // taken
      hint(env, Block::Hint::Unlikely);
      return cns(env, nullptr);
    }
  );
}

/*
 * Returns a Bool value indicating if src (which must be <= TObj) is an
 * instance of the class given in className, or nullptr if we don't have an
 * efficient translation of the required check. checkCls must be the TCls for
 * className (but it doesn't have to be constant).
 */
SSATmp* implInstanceCheck(IRGS& env, SSATmp* src, const StringData* className,
                          SSATmp* checkCls) {
  assertx(src->isA(TObj));
  if (s_Awaitable.get()->isame(className)) {
    return gen(env, IsWaitHandle, src);
  }
  if (s_StringishObject.get()->isame(className)) {
    return gen(env, HasToString, src);
  }

  auto knownCls = checkCls->hasConstVal(TCls) ? checkCls->clsVal() : nullptr;
  assertx(IMPLIES(knownCls, classIsUniqueOrCtxParent(env, knownCls)));
  assertx(IMPLIES(knownCls, knownCls->name()->isame(className)));

  auto const srcType = src->type();

  /*
   * If the value is a specialized object type and we don't have to constrain a
   * guard to get it, we can avoid emitting runtime checks if we know the
   * result is true. If we don't know, we still have to emit a runtime check
   * because src might be a subtype of the specialized type.
   */
  if (srcType < TObj && srcType.clsSpec()) {
    auto const cls = srcType.clsSpec().cls();
    if (!env.irb->constrainValue(src, GuardConstraint(cls).setWeak()) &&
        ((knownCls && cls->classof(knownCls)) ||
         cls->name()->isame(className))) {
      return cns(env, true);
    }
  }

  // Every case after this point requires knowing things about knownCls.
  if (knownCls == nullptr) return nullptr;

  auto const ssaClassName = cns(env, className);
  auto const objClass     = gen(env, LdObjClass, src);

  if (env.context.kind == TransKind::Profile && !InstanceBits::initted()) {
    gen(env, ProfileInstanceCheck, cns(env, className));
  } else if (env.context.kind == TransKind::Optimize ||
             InstanceBits::initted()) {
    InstanceBits::init();
    if (InstanceBits::lookup(className) != 0) {
      return gen(env, InstanceOfBitmask, objClass, ssaClassName);
    }
  }

  // If the class is an interface, we can just hit the class's vtable or
  // interface map and call it a day.
  if (isInterface(knownCls)) {
    auto const slot = knownCls->preClass()->ifaceVtableSlot();
    if (slot != kInvalidSlot && RuntimeOption::RepoAuthoritative) {
      return gen(env,
                 InstanceOfIfaceVtable,
                 InstanceOfIfaceVtableData{knownCls, true},
                 objClass);
    }

    return gen(env, InstanceOfIface, objClass, ssaClassName);
  }

  // If knownCls isn't a normal class, our caller may want to do something
  // different.
  return isNormalClass(knownCls) ?
    gen(env, ExtendsClass, ExtendsClassData{ knownCls }, objClass) : nullptr;
}

/*
 * Emit a type-check for the given type-constraint. Since the details can vary
 * quite a bit depending on what the type-constraint represents, this function
 * is heavily templatized.
 *
 * The lambda parameters are as follows:
 *
 * - GetVal:    Return the SSATmp of the value to test
 * - FuncToStr: Emit code to deal with any func to string conversions.
 * - ClsMethToVec: Emit code to deal with any ClsMeth to array conversions
 * - Fail:      Emit code to deal with the type check failing.
 * - Callable:  Emit code to verify that the given value is callable.
 * - VerifyCls: Emit code to verify that the given value is an instance of the
 *              given Class.
 * - Giveup:    Called when the type check cannot be resolved statically. Either
 *              PUNT or call a runtime helper to do the check.
 *
 * `propCls' should only be non-null for property type-hints, and represents the
 * runtime class of the object the property belongs to.
 */
template <typename GetVal,
          typename FuncToStr,
          typename ClassToStr,
          typename LazyClassToStr,
          typename Fail,
          typename Callable,
          typename VerifyCls,
          typename Giveup>
void verifyTypeImpl(IRGS& env,
                    const TypeConstraint& tc,
                    bool onlyCheckNullability,
                    SSATmp* propCls,
                    GetVal getVal,
                    FuncToStr funcToStr,
                    ClassToStr classToStr,
                    LazyClassToStr lazyClassToStr,
                    Fail fail,
                    Callable callable,
                    VerifyCls verifyCls,
                    Giveup giveup) {

  if (!tc.isCheckable()) return;
  assertx(!tc.isUpperBound() || RuntimeOption::EvalEnforceGenericsUB != 0);

  auto const val = getVal();
  assertx(val->type() <= TCell);
  auto const valType = val->type();

  if (!valType.isKnownDataType()) return giveup();

  if (tc.isNullable() && valType <= TInitNull) return;

  auto const genFail = [&](SSATmp* ctx = nullptr) {
    if (ctx == nullptr) {
      ctx = tc.isThis() ? propCls ? propCls : ldCtxCls(env)
                        : cns(env, nullptr);
    }

    auto const failHard = RuntimeOption::RepoAuthoritative
      && !tc.isSoft()
      && (!tc.isThis() || !tc.couldSeeMockObject())
      && (!tc.isUpperBound() || RuntimeOption::EvalEnforceGenericsUB >= 2);
    return fail(val, ctx, failHard);
  };

  auto const valDataType = valType.toDataType();
  auto const result = annotCompat(valDataType, tc.type(), tc.typeName());

  switch (result) {
    case AnnotAction::Pass:          return;
    case AnnotAction::Fail:          return genFail();
    case AnnotAction::CallableCheck: return callable(val);
    case AnnotAction::ObjectCheck:   break;

    case AnnotAction::WarnClass:
    case AnnotAction::ConvertClass:
      assertx(valType <= TCls);
      if (!classToStr(val)) return genFail();
      if (result == AnnotAction::WarnClass) {
        gen(env, RaiseNotice, cns(env, s_CLASS_TO_STRING_IMPLICIT.get()));
      }
      return;

    case AnnotAction::WarnLazyClass:
    case AnnotAction::ConvertLazyClass:
      assertx(valType <= TLazyCls);
      if (!lazyClassToStr(val)) return genFail();
      if (result == AnnotAction::WarnLazyClass) {
        gen(env, RaiseNotice, cns(env, s_CLASS_TO_STRING_IMPLICIT.get()));
      }
      return;

    case AnnotAction::WarnClassname:
      assertx(valType <= TCls || valType <= TLazyCls);
      gen(env, RaiseNotice, cns(env, s_CLASS_TO_CLASSNAME.get()));
      return;
  }
  assertx(result == AnnotAction::ObjectCheck);
  if (onlyCheckNullability) return;

  if (!(valType <= TObj)) {
    if (tc.isResolved()) return genFail();

    // In RepoAuth mode, we can optimize some type aliases and enum types.
    if (tc.isObject() && RuntimeOption::RepoAuthoritative) {
      auto const pass = [&]{
        auto const ne = tc.namedEntity();
        auto const td = ne->getCachedTypeAlias();
        if (ne->isPersistentTypeAlias() && td) {
          if (td->nullable && valType <= TNull) return true;
          auto const cls = td->klass ? td->klass->name() : nullptr;
          return annotCompat(valDataType, td->type, cls) == AnnotAction::Pass;
        }
        auto const cls = ne->getCachedClass();
        if (cls && classHasPersistentRDS(cls) && cls->enumBaseTy()) {
          auto const type = enumDataTypeToAnnotType(*cls->enumBaseTy());
          return annotCompat(valDataType, type, nullptr) == AnnotAction::Pass;
        }
        return false;
      }();
      if (pass) {
        env.irb->constrainValue(val, DataTypeSpecific);
        return;
      }
    }
    return giveup();
  }

  // At this point, we know that val is a TObj.
  if (tc.isThis()) {
    // For this type checks, the class needs to be an exact match.
    auto const ctxCls = propCls ? propCls : ldCtxCls(env);
    auto const objClass = gen(env, LdObjClass, val);
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, JmpZero, taken, gen(env, EqCls, ctxCls, objClass));
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        genFail(ctxCls);
      }
    );
    return;
  }

  // At this point, we know that val is a TObj and that tc is an Object.
  assertx(tc.isObject());
  const StringData* clsName = nullptr;
  const Class* knownConstraint = nullptr;
  auto const ne = tc.namedEntity();
  auto const td = ne->getCachedTypeAlias();
  if (RO::RepoAuthoritative && ne->isPersistentTypeAlias() &&
      td && td->klass) {
    assertx(classHasPersistentRDS(td->klass));
    clsName = td->klass->name();
    knownConstraint = td->klass;
  } else {
    clsName = tc.typeName();
  }

  auto const checkCls = ldClassSafe(env, clsName, knownConstraint);
  auto const fastIsInstance = implInstanceCheck(env, val, clsName, checkCls);
  if (fastIsInstance) {
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, JmpZero, taken, fastIsInstance);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        genFail();
      }
    );
    return;
  }

  verifyCls(val, gen(env, LdObjClass, val), checkCls);
}

Type typeOpToType(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:    return TInitNull;
  case IsTypeOp::Int:     return TInt;
  case IsTypeOp::Dbl:     return TDbl;
  case IsTypeOp::Bool:    return TBool;
  case IsTypeOp::Str:     return TStr;
  case IsTypeOp::Keyset:  return TKeyset;
  case IsTypeOp::Obj:     return TObj;
  case IsTypeOp::Res:     return TRes;
  case IsTypeOp::ClsMeth:
  case IsTypeOp::Func:
  case IsTypeOp::Class:
  case IsTypeOp::Vec:
  case IsTypeOp::Dict:
  case IsTypeOp::ArrLike:
  case IsTypeOp::LegacyArrLike:
  case IsTypeOp::Scalar: not_reached();
  }
  not_reached();
}

SSATmp* isScalarImpl(IRGS& env, SSATmp* val) {
  // The simplifier works fine when val has a known DataType, but do some
  // checks first in case val has a type like {Int|Str}.
  auto const scalar = TBool | TInt | TDbl | TStr | TCls | TLazyCls;
  if (val->isA(scalar)) return cns(env, true);
  if (!val->type().maybe(scalar)) return cns(env, false);

  SSATmp* result = nullptr;
  for (auto t : {TBool, TInt, TDbl, TStr, TCls, TLazyCls}) {
    auto const is_t = gen(env, ConvBoolToInt, gen(env, IsType, t, val));
    result = result ? gen(env, OrInt, result, is_t) : is_t;
  }
  return gen(env, ConvIntToBool, result);
}

const StaticString s_FUNC_CONVERSION(Strings::FUNC_TO_STRING);
const StaticString s_FUNC_IS_STRING("Func used in is_string");
const StaticString s_CLASS_CONVERSION(Strings::CLASS_TO_STRING);
const StaticString s_CLASS_IS_STRING("Class used in is_string");
const StaticString s_TYPE_STRUCT_NOT_DARR("Type-structure is not a darray");

SSATmp* isStrImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TStr, [&](SSATmp*) { return cns(env, true); });

  mc.ifTypeThen(src, TLazyCls, [&](SSATmp*) {
    if (RuntimeOption::EvalClassIsStringNotices) {
      gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
    }
    return cns(env, true);
  });

  mc.ifTypeThen(src, TCls, [&](SSATmp*) {
    if (RuntimeOption::EvalClassIsStringNotices) {
      gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
    }
    return cns(env, true);
  });

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isClassImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};
  mc.ifTypeThen(src, TLazyCls, [&](SSATmp*) { return cns(env, true); });
  mc.ifTypeThen(src, TCls, [&](SSATmp*) { return cns(env, true); });
  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isFuncImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};
  mc.ifTypeThen(src, TFunc, [&](SSATmp* func) {
    auto const attr = AttrData { AttrIsMethCaller };
    auto const isMC = gen(env, FuncHasAttr, attr, func);
    return gen(env, EqBool, isMC, cns(env, false));
  });
  mc.ifTypeThen(src, TRFunc, [&](SSATmp*) { return cns(env, true); });
  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isClsMethImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};
  mc.ifTypeThen(src, TClsMeth, [&](SSATmp*) { return cns(env, true); });
  mc.ifTypeThen(src, TRClsMeth, [&](SSATmp*) { return cns(env, true); });
  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isVecImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TVec, [&](SSATmp* src) {
    return cns(env, true);
  });

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isDictImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TDict, [&](SSATmp* src) {
    return cns(env, true);
  });

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isArrLikeImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};
  return mc.elseDo([&]{ return gen(env, IsType, TArrLike, src); });
}

SSATmp* isLegacyArrLikeImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TVec|TDict, [&](SSATmp* src) {
    return gen(env, IsLegacyArrLike, src);
  });

  return mc.elseDo([&]{ return cns(env, false); });
}

//////////////////////////////////////////////////////////////////////

}

SSATmp* implInstanceOfD(IRGS& env, SSATmp* src, const StringData* className) {
  /*
   * InstanceOfD is always false if it's not an object.
   *
   * We're prepared to generate translations for known non-object types, but if
   * it's Gen/Cell we're going to PUNT because it's natural to translate that
   * case with control flow TODO(#16781576)
   */
  if (TObj < src->type()) {
    PUNT(InstanceOfD_MaybeObj);
  }
  if (!src->isA(TObj)) {
    if (src->isA(TCls | TLazyCls)) {
      if (!interface_supports_string(className)) return cns(env, false);
      if (RuntimeOption::EvalClassIsStringNotices && src->isA(TCls)) {
        gen(
          env,
          RaiseNotice,
          cns(env, s_CLASS_IS_STRING.get())
        );
      }
      return cns(env, true);
    }

    auto const res =
      (src->isA(TArrLike) && interface_supports_arrlike(className)) ||
      (src->isA(TStr) && interface_supports_string(className)) ||
      (src->isA(TInt) && interface_supports_int(className)) ||
      (src->isA(TDbl) && interface_supports_double(className));
    return cns(env, res);
  }

  auto const checkCls = ldClassSafe(env, className);
  if (auto isInstance = implInstanceCheck(env, src, className, checkCls)) {
    return isInstance;
  }

  return gen(env, InstanceOf, gen(env, LdObjClass, src), checkCls);
}

//////////////////////////////////////////////////////////////////////

void emitInstanceOfD(IRGS& env, const StringData* className) {
  auto const src = popC(env);
  push(env, implInstanceOfD(env, src, className));
  decRef(env, src);
}

void emitInstanceOf(IRGS& env) {
  auto const t1 = popC(env);
  auto const t2 = popC(env); // t2 instanceof t1

  if (t1->isA(TObj) && t2->isA(TObj)) {
    auto const c2 = gen(env, LdObjClass, t2);
    auto const c1 = gen(env, LdObjClass, t1);
    push(env, gen(env, InstanceOf, c2, c1));
    decRef(env, t2);
    decRef(env, t1);
    return;
  }

  if (!t1->isA(TStr)) PUNT(InstanceOf-NotStr);

  if (t2->isA(TObj)) {
    auto const c1 = gen(env, LookupClsRDS, t1);
    auto const c2  = gen(env, LdObjClass, t2);
    push(env, gen(env, InstanceOf, c2, c1));
    decRef(env, t2);
    decRef(env, t1);
    return;
  }

  auto const res = [&]() -> SSATmp* {
    if (t2->isA(TArrLike)) return gen(env, InterfaceSupportsArrLike, t1);
    if (t2->isA(TInt))     return gen(env, InterfaceSupportsInt, t1);
    if (t2->isA(TStr))     return gen(env, InterfaceSupportsStr, t1);
    if (t2->isA(TDbl))     return gen(env, InterfaceSupportsDbl, t1);
    if (t2->isA(TCls)) {
      if (!RO::EvalRaiseClassConversionWarning) {
        return gen(env, InterfaceSupportsStr, t1);
      }
      return cond(
        env,
        [&] (Block* taken) {
          gen(env, JmpZero, taken, gen(env, InterfaceSupportsStr, t1));
        },
        [&] {
          gen(env, RaiseNotice, cns(env, s_CLASS_CONVERSION.get()));
          return cns(env, true);
        },
        [&] { return cns(env, false); }
      );
    }
    if (!t2->type().maybe(TObj|TArrLike|TInt|TStr|TDbl)) return cns(env, false);
    return nullptr;
  }();

  if (!res) PUNT(InstanceOf-Unknown);

  push(env, res);
  decRef(env, t2);
  decRef(env, t1);
}

void emitIsLateBoundCls(IRGS& env) {
  auto const cls = curClass(env);
  if (!cls) PUNT(IsLateBoundCls-NoClassContext);
  if (isTrait(cls)) PUNT(IsLateBoundCls-Trait);
  auto const obj = popC(env);
  if (obj->isA(TObj)) {
    auto const rhs = ldCtxCls(env);
    auto const lhs  = gen(env, LdObjClass, obj);
    push(env, gen(env, InstanceOf, lhs, rhs));
  } else if (!obj->type().maybe(TObj)) {
    push(env, cns(env, false));
  } else {
    PUNT(IsLateBoundCls-MaybeObject);
  }
  decRef(env, obj);
}

namespace {

template<typename F>
SSATmp* resolveTypeStructureAndCacheInRDS(
  IRGS& env,
  F resolveTypeStruct,
  bool typeStructureCouldBeNonStatic
) {
  if (typeStructureCouldBeNonStatic) return resolveTypeStruct();
  auto const handle = RDSHandleData { rds::alloc<TypedValue>().handle() };
  auto const type = TPtrToOtherDict;
  auto const addr = gen(env, LdRDSAddr, handle, type);
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckRDSInitialized, taken, handle);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, StMem, addr, resolveTypeStruct());
      gen(env, MarkRDSInitialized, handle);
    }
  );
  return gen(env, LdMem, TDict, addr);
}

SSATmp* resolveTypeStructImpl(
  IRGS& env,
  bool typeStructureCouldBeNonStatic,
  bool suppress,
  uint32_t n,
  bool isOrAsOp
) {
  auto const declaringCls = curFunc(env) ? curClass(env) : nullptr;
  auto const calledCls =
    declaringCls && typeStructureCouldBeNonStatic
      ? ldCtxCls(env)
      : cns(env, nullptr);
  auto const result = resolveTypeStructureAndCacheInRDS(
    env,
    [&] {
      return gen(
        env,
        ResolveTypeStruct,
        ResolveTypeStructData {
          declaringCls,
          suppress,
          spOffBCFromIRSP(env),
          static_cast<uint32_t>(n),
          isOrAsOp
        },
        sp(env),
        calledCls
      );
    },
    typeStructureCouldBeNonStatic
  );
  popC(env);
  discard(env, n - 1);
  return result;
}

const ArrayData* staticallyResolveTypeStructure(
  IRGS& env,
  const ArrayData* ts,
  bool& partial,
  bool& invalidType
) {
  auto const declaringCls = curFunc(env) ? curClass(env) : nullptr;
  bool persistent = false;
  // This shouldn't do a difference, but does on GCC 8.3 on Ubuntu 19.04;
  // if we take the catch then return `ts`, it's a bogus value and we
  // segfault... sometimes...
  const ArrayData* ts_copy = ts;
  try {
    auto newTS = TypeStructure::resolvePartial(
      ArrNR(ts), nullptr, declaringCls, persistent, partial, invalidType);
    if (persistent) return ArrayData::GetScalarArray(std::move(newTS));
  } catch (Exception& e) {}
  // We are here because either we threw in the resolution or it wasn't
  // persistent resolution which means we didn't really resolve it
  partial = true;
  return ts_copy;
}

SSATmp* check_nullable(IRGS& env, SSATmp* res, SSATmp* var) {
  return cond(
    env,
    [&] (Block* taken) { gen(env, JmpNZero, taken, res); },
    [&] { return gen(env, IsType, TNull, var); },
    [&] { return cns(env, true); }
  );
};

void chain_is_type(IRGS& env, SSATmp* c, bool nullable, Type ty) {
  always_assert(false);
}

template<typename... Types>
void chain_is_type(IRGS& env, SSATmp* c, bool nullable,
                 Type ty1, Type ty2, Types&&... rest) {
  ifThenElse(
    env,
    [&](Block* taken) {
      auto const res = gen(env, IsType, ty1, c);
      gen(env, JmpNZero, taken, res);
    },
    [&] {
      if (sizeof...(rest) == 0) {
        auto const res = gen(env, IsType, ty2, c);
        push(env, nullable ? check_nullable(env, res, c) : res);
      } else {
        chain_is_type(env, c, nullable, ty2, rest...);
      }
    },
    [&] { // taken block
      push(env, cns(env, true));
    }
  );
};

/*
 * This function tries to emit is type struct operations without resolving
 * the type structure when that's possible.
 * When it returns true, it has popped two values from the stack, namely the
 * type structure and the cell, and pushed one value back to stack, namely
 * true/false if it is an is-operation or the cell if it is an as operation.
 * This function does not modify the reference counts of these stack values,
 * leaving that responsibility to the caller.
 * When it returns false, it does not modify anything.
 */
bool emitIsTypeStructWithoutResolvingIfPossible(
  IRGS& env,
  const ArrayData* ts
) {
  // Top of the stack is the type structure, so the thing we are checking is
  // the next element
  auto const t = topC(env, BCSPRelOffset { 1 });
  auto const is_nullable_ts = is_ts_nullable(ts);

  auto const cnsResult = [&] (bool value) {
    popC(env); // pop the ts that's on the stack
    popC(env); // pop the cell
    push(env, cns(env, value));
    return true;
  };

  auto const success = [&] { return cnsResult(true); };
  auto const fail = [&] { return cnsResult(false); };

  auto const primitive = [&] (Type ty, bool should_negate = false) {
    auto const nty = is_nullable_ts ? ty|TNull : ty;
    if (t->isA(nty)) return should_negate ? fail() : success();
    if (!t->type().maybe(nty)) return should_negate ? success() : fail();
    popC(env); // pop the ts that's on the stack
    auto const c = popC(env);
    auto const res = gen(env, should_negate ? IsNType : IsType, ty, c);
    push(env, is_nullable_ts ? check_nullable(env, res, c) : res);
    return true;
  };

  // We explicitly bind is_nullable_ts because failing to do so causes a
  // spurious compiler error on some g++ versions.
  auto const unionOf = [&,is_nullable_ts] (Type ty1, Type ty2,
                                           auto&&... rest) {
    auto const ty = Type::unionAll(ty1, ty2, rest...) |
                    (is_nullable_ts ? TNull : TBottom);
    if (t->isA(ty)) return success();
    if (!t->type().maybe(ty)) return fail();

    popC(env); // pop the ts that's on the stack
    auto const c = popC(env);
    chain_is_type(env, c, is_nullable_ts, ty1, ty2, rest...);
    return true;
  };

  if (t->isA(TNull) && is_nullable_ts) return success();

  auto kind = get_ts_kind(ts);
  switch (kind) {
    case TypeStructure::Kind::T_int:         return primitive(TInt);
    case TypeStructure::Kind::T_bool:        return primitive(TBool);
    case TypeStructure::Kind::T_float:       return primitive(TDbl);
    case TypeStructure::Kind::T_string: {
      if (t->type().maybe(TLazyCls) &&
          RuntimeOption::EvalClassIsStringNotices) {
        ifElse(env,
          [&] (Block* taken) {
            gen(env, CheckType, TLazyCls, taken, t);
          },
          [&] {
            gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
          }
        );
      }
      if (t->type().maybe(TCls) &&
          RuntimeOption::EvalClassIsStringNotices) {
        ifElse(env,
          [&] (Block* taken) {
            gen(env, CheckType, TCls, taken, t);
          },
          [&] {
            gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
          }
        );
      }
      return unionOf(TStr, TLazyCls, TCls);
    }
    case TypeStructure::Kind::T_null:        return primitive(TNull);
    case TypeStructure::Kind::T_void:        return primitive(TNull);
    case TypeStructure::Kind::T_keyset:      return primitive(TKeyset);
    case TypeStructure::Kind::T_nonnull:     return primitive(TNull, true);
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
      return success();
    case TypeStructure::Kind::T_num:         return unionOf(TInt, TDbl);
    case TypeStructure::Kind::T_arraykey: {
      if (t->type().maybe(TLazyCls) &&
          RuntimeOption::EvalClassIsStringNotices) {
        ifElse(env,
          [&] (Block* taken) {
            gen(env, CheckType, TLazyCls, taken, t);
          },
          [&] {
            gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
          }
        );
      }
      if (t->type().maybe(TCls) &&
          RuntimeOption::EvalClassIsStringNotices) {
        ifElse(env,
          [&] (Block* taken) {
            gen(env, CheckType, TCls, taken, t);
          },
          [&] {
            gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
          }
        );
      }
      return unionOf(TInt, TStr, TLazyCls, TCls);
    }
    case TypeStructure::Kind::T_any_array:
      return unionOf(TVec, TDict, TKeyset);
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray: {
      popC(env); // pop the ts that's on the stack
      auto const c = popC(env);
      auto const res = [&]{
        if (kind == TypeStructure::Kind::T_dict ||
            kind == TypeStructure::Kind::T_darray) {
          return isDictImpl(env, c);
        } else if (kind == TypeStructure::Kind::T_vec ||
                   kind == TypeStructure::Kind::T_varray) {
          return isVecImpl(env, c);
        } else {
          assertx(kind == TypeStructure::Kind::T_vec_or_dict ||
                  kind == TypeStructure::Kind::T_varray_or_darray);
          return cond(
            env,
            [&](Block* taken) { gen(env, JmpZero, taken, isVecImpl(env, c)); },
            [&] { return cns(env, true); },
            [&] { return isDictImpl(env, c); }
          );
        }
      }();
      push(env, is_nullable_ts ? check_nullable(env, res, c) : res);
      return true;
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      auto const clsname = get_ts_classname(ts);
      auto cls = lookupUniqueClass(env, clsname);
      if (ts->exists(s_generic_types) &&
          ((classIsPersistentOrCtxParent(env, cls) &&
            cls->hasReifiedGenerics()) ||
           !isTSAllWildcards(ts))) {
        // If it is a reified class or has non wildcard generics,
        // we need to bail
        return false;
      }
      popC(env); // pop the ts that's on the stack
      auto const c = popC(env);
      auto const res = implInstanceOfD(env, c, clsname);
      push(env, is_nullable_ts ? check_nullable(env, res, c) : res);
      return true;
    }
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
      return fail();
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_trait:
      // Not supported, will throw an error on these at the resolution phase
      return false;
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_tuple:
    case TypeStructure::Kind::T_shape:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_reifiedtype:
      // TODO(T28423611): Implement these
      return false;
  }
  not_reached();
}

/*
 * shouldDefRef is set iff the resulting SSATmp is a newly allocated type
 * structure
 * This function does not modify the reference count of its inputs, leaving that
 * to the caller
 */
SSATmp* handleIsResolutionAndCommonOpts(
  IRGS& env,
  TypeStructResolveOp op,
  bool& done,
  bool& shouldDecRef,
  bool& checkValid
) {
  auto const a = topC(env);
  if (!a->isA(TDict)) PUNT(IsTypeStructC-NotArrayTypeStruct);
  if (!a->hasConstVal(TDict)) {
    if (op == TypeStructResolveOp::Resolve) {
      return resolveTypeStructImpl(env, true, true, 1, true);
    }
    shouldDecRef = false;
    checkValid = true;
    return popC(env);
  }
  auto const ts = a->arrLikeVal();
  auto maybe_resolved = ts;
  bool partial = true;
  bool invalidType = true;
  if (op == TypeStructResolveOp::Resolve) {
    maybe_resolved =
      staticallyResolveTypeStructure(env, ts, partial, invalidType);
    shouldDecRef = maybe_resolved != ts;
  }
  if (emitIsTypeStructWithoutResolvingIfPossible(env, maybe_resolved)) {
    done = true;
    return nullptr;
  }
  if (op == TypeStructResolveOp::Resolve && (partial || invalidType)) {
    shouldDecRef = true;
    return resolveTypeStructImpl(
      env, typeStructureCouldBeNonStatic(ts), true, 1, true);
  }
  popC(env);
  if (op == TypeStructResolveOp::DontResolve) checkValid = true;
  return cns(env, maybe_resolved);
}

} // namespace

void emitIsTypeStructC(IRGS& env, TypeStructResolveOp op) {
  auto const a = topC(env);
  auto const c = topC(env, BCSPRelOffset { 1 });
  bool done = false, shouldDecRef = true, checkValid = false;
  SSATmp* tc =
    handleIsResolutionAndCommonOpts(env, op, done, shouldDecRef, checkValid);
  if (done) {
    decRef(env, c);
    decRef(env, a);
    return;
  }
  popC(env);
  auto block = opcodeMayRaise(IsTypeStruct) && shouldDecRef
    ? create_catch_block(env, [&]{ decRef(env, tc); })
    : nullptr;
  auto const data = RDSHandleData { rds::bindTSCache(curFunc(env)).handle() };

  static const StaticString s_IsTypeStruct{"IsTypeStruct"};
  auto const profile = TargetProfile<IsTypeStructProfile> {
    env.context,
    env.irb->curMarker(),
    s_IsTypeStruct.get()
  };

  auto const generic = [&] {
    if (checkValid) gen(env, RaiseErrorOnInvalidIsAsExpressionType, tc);
    return gen(env, IsTypeStruct, block, data, tc, c);
  };

  auto const finish = [&] (SSATmp* result) {
    push(env, result);
    decRef(env, c);
    decRef(env, a);
  };

  if (profile.profiling()) {
    gen(env, ProfileIsTypeStruct, RDSHandleData { profile.handle() }, a);
    finish(generic());
    return;
  }

  if (!profile.optimizing() || !profile.data().shouldOptimize()) {
    finish(generic());
    return;
  }

  finish(cond(
    env,
    [&] (Block* taken) {
      return gen(env, IsTypeStructCached, taken, a, c);
    },
    [&] (SSATmp* result) { // next
      return result;
    },
    [&] { // taken
      hint(env, Block::Hint::Unlikely);
      return generic();
    }
  ));
}

void emitThrowAsTypeStructException(IRGS& env) {
  auto const arr = topC(env);
  auto const c = topC(env, BCSPRelOffset { 1 });
  auto const tsAndBlock = [&]() -> std::pair<SSATmp*, Block*> {
    if (arr->hasConstVal(TDict)) {
      auto const ts = arr->arrLikeVal();
      auto maybe_resolved = ts;
      bool partial = true, invalidType = true;
      maybe_resolved =
        staticallyResolveTypeStructure(env, ts, partial, invalidType);
      if (!ts->same(maybe_resolved)) {
        auto const inputTS = cns(env, maybe_resolved);
        return {inputTS, create_catch_block(env, [&]{ decRef(env, inputTS); })};
      }
    }
    auto const ts = resolveTypeStructImpl(env, true, false, 1, true);
    return {ts, nullptr};
  }();
  // No need to decref inputs as this instruction will throw
  gen(env, ThrowAsTypeStructException, tsAndBlock.second, tsAndBlock.first, c);
}

void emitRecordReifiedGeneric(IRGS& env) {
  auto const ts = popC(env);
  if (!ts->isA(TVec)) {
    PUNT(RecordReifiedGeneric-InvalidTS);
  }
  // RecordReifiedGenericsAndGetTSList decrefs the ts
  auto const result = gen(env, RecordReifiedGenericsAndGetTSList, ts);
  push(env, result);
}

void emitCombineAndResolveTypeStruct(IRGS& env, uint32_t n) {
  push(env, resolveTypeStructImpl(env, true, false, n, false));
}

void raiseClsmethCompatTypeHint(
  IRGS& env, int32_t id, const Func* func, const TypeConstraint& tc) {
  auto name = tc.displayName(func->cls());
  if (id == TypeConstraint::ReturnId) {
    gen(env, RaiseNotice, cns(env, makeStaticString(
      folly::sformat("class_meth Compat: Value returned from function {}() "
      "must be of type {}, clsmeth given",
        func->fullName(), name))));
  } else {
    gen(env, RaiseNotice, cns(env, makeStaticString(
      folly::sformat("class_meth Compat: Argument {} passed to {}() "
      "must be of type {}, clsmeth given",
        id + 1, func->fullName(), name))));
  }
}

namespace {

void verifyRetTypeImpl(IRGS& env, int32_t id, int32_t ind,
                       bool onlyCheckNullability) {
  auto const func = curFunc(env);
  auto const verifyFunc = [&] (const TypeConstraint& tc) {
    verifyTypeImpl(
      env,
      tc,
      onlyCheckNullability,
      nullptr,
      [&] { // Get value to test
        return topC(env, BCSPRelOffset { ind });
      },
      [&] (SSATmp* val) { // func to string conversions
        auto const str = gen(env, LdFuncName, val);
        auto const offset = offsetFromIRSP(env, BCSPRelOffset { ind });
        gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), str);
        env.irb->exceptionStackBoundary();
        return true;
      },
      [&] (SSATmp* val) { // class to string conversions
        auto const str = gen(env, LdClsName, val);
        auto const offset = offsetFromIRSP(env, BCSPRelOffset { ind });
        gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), str);
        env.irb->exceptionStackBoundary();
        return true;
      },
      [&] (SSATmp* val) { // lazy class to string conversions
        auto const str = gen(env, LdLazyClsName, val);
        auto const offset = offsetFromIRSP(env, BCSPRelOffset { ind });
        gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), str);
        env.irb->exceptionStackBoundary();
        return true;
      },
      [&] (SSATmp* val, SSATmp* ctx, bool hard) { // Check failure
        updateMarker(env);
        env.irb->exceptionStackBoundary();
        auto const updated = gen(
          env,
          hard ? VerifyRetFailHard : VerifyRetFail,
          FuncParamWithTCData { func, id, &tc },
          val,
          ctx
        );

        if (!hard) {
          auto const offset = offsetFromIRSP(env, BCSPRelOffset { ind });
          gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), updated);
          env.irb->exceptionStackBoundary();
        }
      },
      [&] (SSATmp* val) { // Callable check
        gen(
          env,
          VerifyRetCallable,
          FuncParamData { func, id },
          val
        );
      },
      [&] (SSATmp* val, SSATmp* objClass, SSATmp* checkCls) {
        // Class/type-alias check
        gen(
          env,
          VerifyRetCls,
          FuncParamWithTCData { func, id, &tc },
          val,
          objClass,
          checkCls
        );
      },
      [] { // Giveup
        PUNT(VerifyReturnType);
      }
    );
  };
  auto const& tc = (id == TypeConstraint::ReturnId)
    ? func->returnTypeConstraint()
    : func->params()[id].typeConstraint;
  assertx(ind >= 0);
  verifyFunc(tc);
  if (id == TypeConstraint::ReturnId && func->hasReturnWithMultiUBs()) {
    auto& ubs = const_cast<Func::UpperBoundVec&>(func->returnUBs());
    for (auto& ub : ubs) {
      applyFlagsToUB(ub, tc);
      verifyFunc(ub);
    }
  } else if (func->hasParamsWithMultiUBs()) {
    auto& ubs = const_cast<Func::ParamUBMap&>(func->paramUBs());
    auto it = ubs.find(id);
    if (it != ubs.end()) {
      for (auto& ub : it->second) {
        applyFlagsToUB(ub, tc);
        verifyFunc(ub);
      }
    }
  }
}

void verifyParamTypeImpl(IRGS& env, int32_t id) {
  auto const func = curFunc(env);
  auto const verifyFunc = [&](const TypeConstraint& tc) {
    verifyTypeImpl(
      env,
      tc,
      false,
      nullptr,
      [&] { // Get value to test
        return ldLoc(env, id, DataTypeSpecific);
      },
      [&] (SSATmp* val) { // func to string conversions
        auto const str = gen(env, LdFuncName, val);
        stLocRaw(env, id, fp(env), str);
        return true;
      },
      [&] (SSATmp* val) { // class to string conversions
        auto const str = gen(env, LdClsName, val);
        stLocRaw(env, id, fp(env), str);
        return true;
      },
      [&] (SSATmp* val) { // lazy class to string conversions
        auto const str = gen(env, LdLazyClsName, val);
        stLocRaw(env, id, fp(env), str);
        return true;
      },
      [&] (SSATmp* val, SSATmp* ctx, bool hard) { // Check failure
        auto const updated = gen(
          env,
          hard ? VerifyParamFailHard : VerifyParamFail,
          FuncParamWithTCData { func, id, &tc },
          val,
          ctx
        );

        if (!hard) {
          stLocRaw(env, id, fp(env), updated);
        }
      },
      [&] (SSATmp* val) { // Callable check
        gen(
          env,
          VerifyParamCallable,
          FuncParamData { func, id },
          val
        );
      },
      [&] (SSATmp* val, SSATmp* objClass, SSATmp* checkCls) {
        // Class/type-alias check
        gen(
          env,
          VerifyParamCls,
          FuncParamWithTCData { func, id, &tc },
          val,
          objClass,
          checkCls
        );
      },
      [] { // Giveup
        PUNT(VerifyParamType);
      }
    );
  };
  auto const& tc = func->params()[id].typeConstraint;
  verifyFunc(tc);
  if (func->hasParamsWithMultiUBs()) {
    auto& ubs = const_cast<Func::ParamUBMap&>(func->paramUBs());
    auto it = ubs.find(id);
    if (it != ubs.end()) {
      for (auto& ub : it->second) {
        applyFlagsToUB(ub, tc);
        verifyFunc(ub);
      }
    }
  }
}

}

void verifyPropType(IRGS& env,
                    SSATmp* cls,
                    const HPHP::TypeConstraint* tc,
                    const Class::UpperBoundVec* ubs,
                    Slot slot,
                    SSATmp* val,
                    SSATmp* name,
                    bool isSProp,
                    SSATmp** coerce /* = nullptr */) {
  assertx(cls->isA(TCls));
  assertx(val->isA(TCell));

  if (coerce) *coerce = val;
  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;

  auto const verifyFunc = [&](const TypeConstraint* tc) {
    if (!tc || !tc->isCheckable()) return;
    assertx(tc->validForProp());

    verifyTypeImpl(
      env,
      *tc,
      false,
      cls,
      [&] { // Get value to check
        env.irb->constrainValue(val, DataTypeSpecific);
        return val;
      },
      [&] (SSATmp*) { return false; }, // No func to string automatic conversions
      [&] (SSATmp*) {  // class to string automatic conversions
        if (!coerce) return false;
        if (RO::EvalCheckPropTypeHints < 3) return false;
        *coerce = gen(env, LdClsName, val);
        return true;
      },
      [&] (SSATmp*) {  // lazy class to string automatic conversions
        if (!coerce) return false;
        if (RO::EvalCheckPropTypeHints < 3) return false;
        *coerce = gen(env, LdLazyClsName, val);
        return true;
      },
      [&] (SSATmp*, SSATmp*, bool hard) { // Check failure
        auto const failHard =
          hard && RuntimeOption::EvalCheckPropTypeHints >= 3 &&
          (!tc->isUpperBound() || RuntimeOption::EvalEnforceGenericsUB >= 2);
        gen(
          env,
          failHard ? VerifyPropFailHard : VerifyPropFail,
          TypeConstraintData{ tc },
          cls,
          cns(env, slot),
          val,
          cns(env, isSProp)
        );
      },
      // We don't allow callable as a property type-hint, so we should never need
      // to check callability.
      [&] (SSATmp*) { always_assert(false); },
      [&] (SSATmp* v, SSATmp*, SSATmp* checkCls) { // Class/type-alias check
        gen(
          env,
          VerifyPropCls,
          TypeConstraintData{ tc },
          cls,
          cns(env, slot),
          checkCls,
          v,
          cns(env, isSProp)
        );
      },
      [&] {
        // Unlike the other type-hint checks, we don't punt here. We instead do
        // the check using a runtime helper. This gives us the freedom to call
        // verifyPropType without us worrying about it punting the whole set op.
        // This check is fragile - which type constraints coerce?

        if (coerce && (tc->isString() || (tc->isObject() && !tc->isResolved()))) {
          *coerce = gen(
            env,
            VerifyPropCoerce,
            TypeConstraintData { tc },
            cls,
            cns(env, slot),
            val,
            cns(env, isSProp)
          );
        } else {
          gen(
            env,
            VerifyProp,
            TypeConstraintData { tc },
            cls,
            cns(env, slot),
            val,
            cns(env, isSProp)
          );
        }
      }
    );
  };
  verifyFunc(tc);
  if (RuntimeOption::EvalEnforceGenericsUB > 0) {
    for (auto const& ub : *ubs) {
      verifyFunc(&ub);
    }
  }
}

void emitVerifyRetTypeC(IRGS& env) {
  verifyRetTypeImpl(env, TypeConstraint::ReturnId, 0, false);
}

void emitVerifyRetTypeTS(IRGS& env) {
  verifyRetTypeImpl(env, TypeConstraint::ReturnId, 1, false);
  auto const ts = popC(env);
  auto const cell = topC(env);
  auto const reified = tcCouldBeReified(curFunc(env), TypeConstraint::ReturnId);
  if (reified || cell->isA(TObj)) {
    auto const funcData = FuncData { curFunc(env) };
    gen(env, VerifyReifiedReturnType, funcData, cell, ts, ldCtxCls(env));
  } else if (cell->type().maybe(TObj) && !reified) {
    // Meaning we did not not guard on the stack input correctly
    PUNT(VerifyRetTypeTS-UnguardedObj);
  }
}

void emitVerifyRetNonNullC(IRGS& env) {
  auto const func = curFunc(env);
  auto const& tc = func->returnTypeConstraint();
  always_assert(!tc.isNullable());
  verifyRetTypeImpl(env, TypeConstraint::ReturnId, 0, true);
}

void emitVerifyOutType(IRGS& env, uint32_t paramId) {
  verifyRetTypeImpl(env, paramId, 0, false);
}

void emitVerifyParamType(IRGS& env, int32_t paramId) {
  verifyParamTypeImpl(env, paramId);
}

void emitVerifyParamTypeTS(IRGS& env, int32_t paramId) {
  verifyParamTypeImpl(env, paramId);
  auto const ts = popC(env);
  auto const cell = ldLoc(env, paramId, DataTypeSpecific);
  auto const reified = tcCouldBeReified(curFunc(env), paramId);
  if (cell->isA(TObj) || reified) {
    cond(
      env,
      [&] (Block* taken) {
        return gen(env, CheckType, TDict, taken, ts);
      },
      [&] (SSATmp* dts) {
        auto const fpData = FuncParamData { curFunc(env), paramId };
        gen(env, VerifyReifiedLocalType, fpData, cell, dts, ldCtxCls(env));
        return nullptr;
      },
      [&] {
        gen(env, RaiseError, cns(env, s_TYPE_STRUCT_NOT_DARR.get()));
        return nullptr;
      }
    );
  } else if (cell->type().maybe(TObj)) {
    // Meaning we did not not guard on the stack input correctly
    PUNT(VerifyReifiedLocalType-UnguardedObj);
  }
}

void emitOODeclExists(IRGS& env, OODeclExistsOp subop) {
  auto const tAutoload = topC(env);
  auto const tCls = topC(env, BCSPRelOffset{1});

  if (!tCls->isA(TStr) || !tAutoload->isA(TBool)){ // result of Cast
    PUNT(OODeclExists-BadTypes);
  }

  ClassKind kind;
  switch (subop) {
  case OODeclExistsOp::Class:     kind = ClassKind::Class; break;
  case OODeclExistsOp::Trait:     kind = ClassKind::Trait; break;
  case OODeclExistsOp::Interface: kind = ClassKind::Interface; break;
  }

  auto const val = gen(
    env,
    OODeclExists,
    ClassKindData { kind },
    tCls,
    tAutoload
  );
  discard(env, 2);
  push(env, val);
  decRef(env, tCls);
}

void emitIssetL(IRGS& env, int32_t id) {
  auto const ld = ldLoc(env, id, DataTypeSpecific);
  push(env, gen(env, IsNType, TNull, ld));
}

void emitIsUnsetL(IRGS& env, int32_t id) {
  auto const ld = ldLoc(env, id, DataTypeSpecific);
  push(env, gen(env, IsType, TUninit, ld));
}

SSATmp* isTypeHelper(IRGS& env, IsTypeOp subop, SSATmp* val) {
  switch (subop) {
    case IsTypeOp::Vec:           return isVecImpl(env, val);
    case IsTypeOp::Dict:          return isDictImpl(env, val);
    case IsTypeOp::Scalar:        return isScalarImpl(env, val);
    case IsTypeOp::Str:           return isStrImpl(env, val);
    case IsTypeOp::ArrLike:       return isArrLikeImpl(env, val);
    case IsTypeOp::LegacyArrLike: return isLegacyArrLikeImpl(env, val);
    case IsTypeOp::Class:         return isClassImpl(env, val);
    case IsTypeOp::Func:          return isFuncImpl(env, val);
    case IsTypeOp::ClsMeth:       return isClsMethImpl(env, val);
    default: break;
  }

  auto const t = typeOpToType(subop);
  return t <= TObj ? optimizedCallIsObject(env, val) : gen(env, IsType, t, val);
}

void emitIsTypeC(IRGS& env, IsTypeOp subop) {
  auto const val = popC(env, DataTypeSpecific);
  push(env, isTypeHelper(env, subop, val));
  decRef(env, val);
}

void emitIsTypeL(IRGS& env, NamedLocal loc, IsTypeOp subop) {
  auto const val = ldLocWarn(env, loc, DataTypeSpecific);
  push(env, isTypeHelper(env, subop, val));
}

//////////////////////////////////////////////////////////////////////

void emitAssertRATL(IRGS& env, int32_t loc, RepoAuthType rat) {
  assertTypeLocal(env, loc, typeFromRAT(rat, curClass(env)));
}

void emitAssertRATStk(IRGS& env, uint32_t offset, RepoAuthType rat) {
  assertTypeStack(
    env,
    BCSPRelOffset{safe_cast<int32_t>(offset)},
    typeFromRAT(rat, curClass(env))
  );
}

//////////////////////////////////////////////////////////////////////

}}}
