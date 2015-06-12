/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-guards.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_WaitHandle("HH\\WaitHandle");

//////////////////////////////////////////////////////////////////////

/*
 * Returns a {Cls|Nullptr} suitable for use in instance checks. If knownCls is
 * not null and is safe to use, that will be returned. Otherwise, className
 * will be used to look up a class.
 */
SSATmp* ldClassSafe(IRGS& env, const StringData* className,
                    const Class* knownCls = nullptr) {
  if (!knownCls) knownCls = Unit::lookupClassOrUniqueClass(className);

  // We can only burn in the Class* if it's unique or in the inheritance
  // hierarchy of our context. If we can't burn in the class, use
  // LdClsCachedSafe - InstanceOfD and Verify(Ret|Param)Type don't invoke
  // autoload.
  return classIsUniqueOrCtxParent(env, knownCls)
    ? cns(env, knownCls)
    : gen(env, LdClsCachedSafe, cns(env, className));
}

/*
 * Returns a Bool value indicating if src (which must be <= TObj) is an
 * instance of the class given in className, or nullptr if we don't have an
 * efficient translation of the required check. checkCls must be the TCls for
 * className (but it doesn't have to be constant).
 */
SSATmp* implInstanceCheck(IRGS& env, SSATmp* src, const StringData* className,
                          SSATmp* checkCls) {
  assert(src->isA(TObj));
  if (s_WaitHandle.get()->isame(className)) {
    return gen(env, IsWaitHandle, src);
  }

  auto knownCls = checkCls->hasConstVal(TCls) ? checkCls->clsVal() : nullptr;
  assert(IMPLIES(knownCls, classIsUniqueOrCtxParent(env, knownCls)));
  assert(IMPLIES(knownCls, knownCls->name()->isame(className)));

  auto const srcType = src->type();

  /*
   * If the value is a specialized object type and we don't have to constrain a
   * guard to get it, we can avoid emitting runtime checks if we know the
   * result is true. If we don't know, we still have to emit a runtime check
   * because src might be a subtype of the specialized type.
   */
  if (srcType < TObj && srcType.clsSpec()) {
    auto const cls = srcType.clsSpec().cls();
    if (!env.irb->constrainValue(src, TypeConstraint(cls).setWeak()) &&
        ((knownCls && cls->classof(knownCls)) ||
         cls->name()->isame(className))) {
      return cns(env, true);
    }
  }

  // Every case after this point requires knowing things about knownCls.
  if (knownCls == nullptr) return nullptr;

  auto const ssaClassName = cns(env, className);
  auto const objClass     = gen(env, LdObjClass, src);

  InstanceBits::init();
  if (InstanceBits::lookup(className) != 0) {
    return gen(env, InstanceOfBitmask, objClass, ssaClassName);
  }

  // If the class is an interface, we can just hit the class's vtable or
  // interface map and call it a day.
  if (isInterface(knownCls)) {
    auto const slot = knownCls->preClass()->ifaceVtableSlot();
    if (slot != kInvalidSlot && RuntimeOption::RepoAuthoritative) {
      return gen(env, InstanceOfIfaceVtable, ClassData{knownCls}, objClass);
    }

    return gen(env, InstanceOfIface, objClass, ssaClassName);
  }

  // If knownCls isn't a normal class, our caller may want to do something
  // different.
  return isNormalClass(knownCls) ? gen(env, ExtendsClass, objClass, checkCls)
                                 : nullptr;
}

void verifyTypeImpl(IRGS& env, int32_t const id) {
  const bool isReturnType = (id == HPHP::TypeConstraint::ReturnId);
  if (isReturnType && !RuntimeOption::EvalCheckReturnTypeHints) return;

  auto func = curFunc(env);
  auto const& tc = isReturnType ? func->returnTypeConstraint()
                                : func->params()[id].typeConstraint;
  if (tc.isMixed()) return;

  auto const ldPMExit = makePseudoMainExit(env);
  auto val = isReturnType ? topR(env)
                          : ldLoc(env, id, ldPMExit, DataTypeSpecific);
  assertx(val->type() <= TCell || val->type() <= TBoxedCell);

  auto const valType = [&]() -> Type {
    if (val->type() <= TCell) return val->type();
    if (isReturnType) PUNT(VerifyReturnTypeBoxed);
    auto const pred = env.irb->predictedInnerType(id);
    gen(env, CheckRefInner, pred, makeExit(env), val);
    val = gen(env, LdRef, pred, val);
    return pred;
  }();

  if (!valType.isKnownDataType()) {
    interpOne(env, 0);
    return;
  }

  if (tc.isNullable() && valType <= TInitNull) return;

  if (!isReturnType && tc.isArray() && !tc.isSoft() && !func->mustBeRef(id) &&
      valType <= TObj) {
    PUNT(VerifyParamType-collectionToArray);
    return;
  }

  auto result = annotCompat(valType.toDataType(), tc.type(), tc.typeName());
  switch (result) {
    case AnnotAction::Pass:
      return;
    case AnnotAction::Fail:
      if (isReturnType) {
        gen(env, VerifyRetFail, val);
      } else {
        gen(env, VerifyParamFail, cns(env, id));
      }
      return;
    case AnnotAction::CallableCheck:
      if (isReturnType) {
        gen(env, VerifyRetCallable, val);
      } else {
        gen(env, VerifyParamCallable, val, cns(env, id));
      }
      return;
    case AnnotAction::ObjectCheck:
      break;
  }
  assertx(result == AnnotAction::ObjectCheck);

  if (!(valType <= TObj)) {
    // For RepoAuthoritative mode, if tc is a type alias we can optimize in
    // some cases
    if (tc.isObject() && RuntimeOption::RepoAuthoritative) {
      auto const td = tc.namedEntity()->getCachedTypeAlias();
      if (tc.namedEntity()->isPersistentTypeAlias() && td &&
          ((td->nullable && valType <= TNull) ||
           annotCompat(valType.toDataType(), td->type,
             td->klass ? td->klass->name() : nullptr) == AnnotAction::Pass)) {
        env.irb->constrainValue(val, DataTypeSpecific);
        return;
      }
      auto cachedClass = tc.namedEntity()->getCachedClass();
      if (cachedClass && classHasPersistentRDS(cachedClass) &&
          cachedClass->enumBaseTy() &&
          annotCompat(valType.toDataType(),
                      dataTypeToAnnotType(*cachedClass->enumBaseTy()),
                      nullptr) == AnnotAction::Pass) {
        env.irb->constrainValue(val, DataTypeSpecific);
        return;
      }
    }
    // Give up and call the interpreter
    interpOne(env, 0);
    return;
  }

  // If we reach here then valType is Obj and tc is Object, Self, or Parent
  const StringData* clsName;
  const Class* knownConstraint = nullptr;
  if (tc.isObject()) {
    auto const td = tc.namedEntity()->getCachedTypeAlias();
    if (RuntimeOption::RepoAuthoritative && td &&
        tc.namedEntity()->isPersistentTypeAlias() &&
        td->klass) {
      clsName = td->klass->name();
      knownConstraint = td->klass;
    } else {
      clsName = tc.typeName();
      knownConstraint = Unit::lookupClassOrUniqueClass(clsName);
    }
  } else {
    if (tc.isSelf()) {
      tc.selfToClass(curFunc(env), &knownConstraint);
    } else {
      assertx(tc.isParent());
      tc.parentToClass(curFunc(env), &knownConstraint);
    }
    if (!knownConstraint) {
      // The hint was self or parent and there's no corresponding
      // class for the current func. This typehint will always fail.
      if (isReturnType) {
        gen(env, VerifyRetFail, val);
      } else {
        gen(env, VerifyParamFail, cns(env, id));
      }
      return;
    }
    clsName = knownConstraint->preClass()->name();
  }
  assertx(clsName);

  // For "self" and "parent", knownConstraint should always be
  // non-null at this point
  assertx(IMPLIES(tc.isSelf() || tc.isParent(), knownConstraint != nullptr));

  auto const checkCls = ldClassSafe(env, clsName, knownConstraint);
  auto const fastIsInstance = implInstanceCheck(env, val, clsName, checkCls);
  if (fastIsInstance) {
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, JmpZero, taken, fastIsInstance);
      },
      [&] { // taken: the param type does not match
        hint(env, Block::Hint::Unlikely);
        if (isReturnType) gen(env, VerifyRetFail, val);
        else              gen(env, VerifyParamFail, cns(env, id));
      }
    );
    return;
  }

  auto const objClass = gen(env, LdObjClass, val);
  if (isReturnType) {
    gen(env, VerifyRetCls, objClass, checkCls,
        cns(env, uintptr_t(&tc)), val);
  } else {
    gen(env, VerifyParamCls, objClass, checkCls,
        cns(env, uintptr_t(&tc)), cns(env, id));
  }
}

DataType typeOpToDataType(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:  return KindOfNull;
  case IsTypeOp::Int:   return KindOfInt64;
  case IsTypeOp::Dbl:   return KindOfDouble;
  case IsTypeOp::Bool:  return KindOfBoolean;
  case IsTypeOp::Str:   return KindOfString;
  case IsTypeOp::Arr:   return KindOfArray;
  case IsTypeOp::Obj:   return KindOfObject;
  case IsTypeOp::Scalar: not_reached();
  }
  not_reached();
}

void implIsScalarL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(env, gen(env, IsScalarType, src));
}

void implIsScalarC(IRGS& env) {
  auto const src = popC(env);
  push(env, gen(env, IsScalarType, src));
  gen(env, DecRef, src);
}

//////////////////////////////////////////////////////////////////////

}

/*
 * Note: this is currently separate from typeFromRAT for now, just because we
 * don't want to enable every single type for assertions yet.
 *
 * (Some of them currently regress performance, presumably because the IR
 * doesn't always handle the additional type information very well.  It is
 * possibly a compile-time slowdown only, but we haven't investigated yet.)
 */
folly::Optional<Type> ratToAssertType(IRGS& env, RepoAuthType rat) {
  using T = RepoAuthType::Tag;

  switch (rat.tag()) {
    case T::Uninit:
    case T::InitNull:
    case T::Null:
    case T::Bool:
    case T::Int:
    case T::Dbl:
    case T::Res:
    case T::SStr:
    case T::Str:
    case T::Obj:
    case T::SArr:
    case T::Arr:
    case T::Cell:
    case T::Ref:
    case T::InitUnc:
    case T::Unc:
      return typeFromRAT(rat);

    case T::OptExactObj:
    case T::OptSubObj:
    case T::ExactObj:
    case T::SubObj: {
      auto ty = typeFromRAT(rat);
      auto const cls = Unit::lookupClassOrUniqueClass(rat.clsName());

      if (!classIsUniqueOrCtxParent(env, cls)) {
        ty |= TObj; // Kill specialization.
      }
      return ty;
    }

    // Type assertions can't currently handle Init-ness.
    case T::InitCell:
      return TCell;
    case T::InitGen:
      return folly::none;

    case T::Gen:
      return folly::none;

    case T::OptInt:
    case T::OptObj:
    case T::OptDbl:
    case T::OptBool:
    case T::OptSStr:
    case T::OptStr:
    case T::OptRes:
      return folly::none;

    case T::OptSArr:
    case T::OptArr:
      // TODO(#4205897): optional array types.
      return folly::none;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

SSATmp* implInstanceOfD(IRGS& env, SSATmp* src, const StringData* className) {
  /*
   * InstanceOfD is always false if it's not an object.
   *
   * We're prepared to generate translations for known non-object types, but if
   * it's Gen/Cell we're going to PUNT because it's natural to translate that
   * case with control flow TODO(#2020251)
   */
  if (TObj < src->type()) {
    PUNT(InstanceOfD_MaybeObj);
  }
  if (!src->isA(TObj)) {
    bool res = ((src->isA(TArr) && interface_supports_array(className))) ||
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
  gen(env, DecRef, src);
}

void emitInstanceOf(IRGS& env) {
  auto const t1 = popC(env);
  auto const t2 = popC(env); // t2 instanceof t1

  if (t1->isA(TObj) && t2->isA(TObj)) {
    auto const c2 = gen(env, LdObjClass, t2);
    auto const c1 = gen(env, LdObjClass, t1);
    push(env, gen(env, InstanceOf, c2, c1));
    gen(env, DecRef, t2);
    gen(env, DecRef, t1);
    return;
  }

  if (!t1->isA(TStr)) PUNT(InstanceOf-NotStr);

  if (t2->isA(TObj)) {
    auto const rds = gen(env, LookupClsRDSHandle, t1);
    auto const c1  = gen(env, DerefClsRDSHandle, rds);
    auto const c2  = gen(env, LdObjClass, t2);
    push(env, gen(env, InstanceOf, c2, c1));
    gen(env, DecRef, t2);
    gen(env, DecRef, t1);
    return;
  }

  push(
    env,
    t2->isA(TArr) ? gen(env, InterfaceSupportsArr, t1) :
    t2->isA(TInt) ? gen(env, InterfaceSupportsInt, t1) :
    t2->isA(TStr) ? gen(env, InterfaceSupportsStr, t1) :
    t2->isA(TDbl) ? gen(env, InterfaceSupportsDbl, t1) :
    cns(env, false)
  );
  gen(env, DecRef, t2);
  gen(env, DecRef, t1);
}

void emitVerifyRetTypeC(IRGS& env) {
  verifyTypeImpl(env, HPHP::TypeConstraint::ReturnId);
}

void emitVerifyRetTypeV(IRGS& env) {
  verifyTypeImpl(env, HPHP::TypeConstraint::ReturnId);
}

void emitVerifyParamType(IRGS& env, int32_t paramId) {
  verifyTypeImpl(env, paramId);
}

void emitOODeclExists(IRGS& env, OODeclExistsOp subop) {
  auto const tAutoload = popC(env);
  auto const tCls = popC(env);

  assertx(tCls->isA(TStr)); // result of CastString
  assertx(tAutoload->isA(TBool)); // result of CastBool

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
  push(env, val);
  gen(env, DecRef, tCls);
}

void emitIssetL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(env, gen(env, IsNType, TNull, ld));
}

void emitEmptyL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(
    env,
    gen(env, XorBool, gen(env, ConvCellToBool, ld), cns(env, true))
  );
}

void emitIsTypeC(IRGS& env, IsTypeOp subop) {
  if (subop == IsTypeOp::Scalar) return implIsScalarC(env);
  auto const t = typeOpToDataType(subop);
  auto const src = popC(env, DataTypeSpecific);
  if (t == KindOfObject) {
    push(env, optimizedCallIsObject(env, src));
  } else {
    push(env, gen(env, IsType, Type(t), src));
  }
  gen(env, DecRef, src);
}

void emitIsTypeL(IRGS& env, int32_t id, IsTypeOp subop) {
  if (subop == IsTypeOp::Scalar) return implIsScalarL(env, id);
  auto const t = typeOpToDataType(subop);
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const val =
    ldLocInnerWarn(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (t == KindOfObject) {
    push(env, optimizedCallIsObject(env, val));
  } else {
    push(env, gen(env, IsType, Type(t), val));
  }
}

void emitAssertRATL(IRGS& env, int32_t loc, RepoAuthType rat) {
  if (auto const t = ratToAssertType(env, rat)) {
    assertTypeLocal(env, loc, *t);
  }
}

void emitAssertRATStk(IRGS& env, int32_t offset, RepoAuthType rat) {
  if (auto const t = ratToAssertType(env, rat)) {
    assertTypeStack(env, BCSPOffset{offset}, *t);
  }
}

//////////////////////////////////////////////////////////////////////

}}}
