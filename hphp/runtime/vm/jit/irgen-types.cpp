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

void verifyTypeImpl(HTS& env, int32_t const id) {
  const bool isReturnType = (id == HPHP::TypeConstraint::ReturnId);
  if (isReturnType && !RuntimeOption::EvalCheckReturnTypeHints) return;

  auto const ldPMExit = makePseudoMainExit(env);
  auto func = curFunc(env);
  auto const& tc = isReturnType ? func->returnTypeConstraint()
                                : func->params()[id].typeConstraint;
  auto val = isReturnType ? topR(env)
                          : ldLoc(env, id, ldPMExit, DataTypeSpecific);
  assert(val->type().isBoxed() || val->type().notBoxed());

  auto const valType = [&]() -> Type {
    if (!val->type().isBoxed()) return val->type();
    if (isReturnType) PUNT(VerifyReturnTypeBoxed);
    auto const pred = env.irb->predictedInnerType(id);
    gen(env, CheckRefInner, pred, makeExit(env), val);
    val = gen(env, LdRef, pred, val);
    return pred;
  }();

  if (!valType.isKnownDataType()) {
    if (!isReturnType) {
      // This is supposed to be impossible, but it does happen in a rare case
      // with the legacy region selector. Until it's figured out, punt in
      // release builds. t3412704
      assert_log(
        false,
        [&] {
          return folly::format("Bad type {} for local {}:\n\n{}\n",
                               valType, id, env.irb->unit().toString()).str();
        }
      );
    }
    interpOne(env, 0);
    return;
  }

  if (tc.isTypeVar()) return;
  if (tc.isNullable() && valType.subtypeOf(Type::InitNull)) return;

  if (!isReturnType && tc.isArray() && !tc.isSoft() && !func->mustBeRef(id) &&
      valType <= Type::Obj) {
    PUNT(VerifyParamType-collectionToArray);
    return;
  }
  if (tc.isCallable()) {
    if (isReturnType) {
      gen(env, VerifyRetCallable, val);
    } else {
      gen(env, VerifyParamCallable, val, cns(env, id));
    }
    return;
  }

  // For non-object guards, we rely on what we know from the tracelet
  // guards and never have to do runtime checks.
  if (!tc.isObjectOrTypeAlias()) {
    if (!tc.checkPrimitive(valType.toDataType())) {
      if (isReturnType) {
        gen(env, VerifyRetFail, val);
      } else {
        gen(env, VerifyParamFail, cns(env, id));
      }
    }
    return;
  }
  // If val is not an object, it still might pass the type constraint
  // if the constraint is a typedef. For now we just interp that case.
  auto const typeName = tc.typeName();
  if (valType <= Type::Arr && interface_supports_array(typeName)) {
    return;
  }
  if (valType <= Type::Str && interface_supports_string(typeName)) {
    return;
  }
  if (valType <= Type::Int && interface_supports_int(typeName)) {
    return;
  }
  if (valType <= Type::Dbl && interface_supports_double(typeName)) {
    return;
  }
  if (!(valType <= Type::Obj)) {
    if (tc.isObjectOrTypeAlias()
        && RuntimeOption::RepoAuthoritative
        && !tc.isCallable()
        && tc.isPrecise()) {
      auto const td = tc.namedEntity()->getCachedTypeAlias();
      if (tc.namedEntity()->isPersistentTypeAlias() && td) {
        if ((td->nullable && valType <= Type::Null)
            || td->any
            || equivDataTypes(td->kind, valType.toDataType())) {
          env.irb->constrainValue(val, TypeConstraint(DataTypeSpecific));
          return;
        }
      }
    }
    interpOne(env, 0);
    return;
  }

  const StringData* clsName;
  const Class* knownConstraint = nullptr;
  if (!tc.isSelf() && !tc.isParent()) {
    clsName = tc.typeName();
    knownConstraint = Unit::lookupClass(clsName);
  } else {
    if (tc.isSelf()) {
      tc.selfToClass(curFunc(env), &knownConstraint);
    } else if (tc.isParent()) {
      tc.parentToClass(curFunc(env), &knownConstraint);
    }
    if (knownConstraint) {
      clsName = knownConstraint->preClass()->name();
    } else {
      // The hint was self or parent and there's no corresponding
      // class for the current func. This typehint will always fail.
      if (isReturnType) {
        gen(env, VerifyRetFail, val);
      } else {
        gen(env, VerifyParamFail, cns(env, id));
      }
      return;
    }
  }
  assert(clsName);

  // We can only burn in the Class* if it's unique or in the
  // inheritance hierarchy of our context. It's ok if the class isn't
  // defined yet - all paths below are tolerant of a null constraint.
  if (!classIsUniqueOrCtxParent(env, knownConstraint)) {
    knownConstraint = nullptr;
  }

  /*
   * If the local is a specialized object type and we don't have to constrain a
   * guard to get it, we can avoid emitting runtime checks if we know the thing
   * would pass. If we don't know, we still have to emit them because valType
   * might be a subtype of its specialized object type.
   */
  if (valType < Type::Obj) {
    auto const cls = valType.getClass();
    if (!env.irb->constrainValue(val, TypeConstraint(cls).setWeak()) &&
        ((knownConstraint && cls->classof(knownConstraint)) ||
         cls->name()->isame(clsName))) {
      return;
    }
  }

  InstanceBits::init();
  auto const haveBit = InstanceBits::lookup(clsName) != 0;
  auto const constraint = knownConstraint
    ? cns(env, knownConstraint)
    : gen(env, LdClsCachedSafe, cns(env, clsName));
  auto const objClass = gen(env, LdObjClass, val);
  if (haveBit || classIsUniqueNormalClass(knownConstraint)) {
    auto const isInstance = haveBit
      ? gen(env, InstanceOfBitmask, objClass, cns(env, clsName))
      : gen(env, ExtendsClass, objClass, constraint);
    env.irb->ifThen(
      [&](Block* taken) {
        gen(env, JmpZero, taken, isInstance);
      },
      [&] { // taken: the param type does not match
        env.irb->hint(Block::Hint::Unlikely);
        if (isReturnType) {
          gen(env, VerifyRetFail, val);
        } else {
          gen(env, VerifyParamFail, cns(env, id));
        }
      }
    );
  } else {
    if (isReturnType) {
      gen(env, VerifyRetCls, objClass, constraint,
          cns(env, uintptr_t(&tc)), val);
    } else {
      gen(env, VerifyParamCls, objClass, constraint,
          cns(env, uintptr_t(&tc)), cns(env, id));
    }
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

void implIsScalarL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const src = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(env, gen(env, IsScalarType, src));
}

void implIsScalarC(HTS& env) {
  auto const src = popC(env);
  push(env, gen(env, IsScalarType, src));
  gen(env, DecRef, src);
}

/*
 * Note: this is currently separate from convertToType(RepoAuthType) for now,
 * just because we don't want to enable every single type for assertions yet.
 *
 * (Some of them currently regress performance, presumably because the IR
 * doesn't always handle the additional type information very well.  It is
 * possibly a compile-time slowdown only, but we haven't investigated yet.)
 */
folly::Optional<Type> ratToAssertType(HTS& env, RepoAuthType rat) {
  using T = RepoAuthType::Tag;
  switch (rat.tag()) {
  case T::Uninit:     return Type::Uninit;
  case T::InitNull:   return Type::InitNull;
  case T::Int:        return Type::Int;
  case T::Dbl:        return Type::Dbl;
  case T::Res:        return Type::Res;
  case T::Null:       return Type::Null;
  case T::Bool:       return Type::Bool;
  case T::Str:        return Type::Str;
  case T::Obj:        return Type::Obj;
  case T::SStr:       return Type::StaticStr;

  // These aren't enabled yet:
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

  case T::SArr:
    if (auto const arr = rat.array()) {
      return Type::StaticArr.specialize(arr);
    }
    return Type::StaticArr;
  case T::Arr:
    if (auto const arr = rat.array()) {
      return Type::Arr.specialize(arr);
    }
    return Type::Arr;

  case T::OptExactObj:
  case T::OptSubObj:
  case T::ExactObj:
  case T::SubObj:
    {
      auto ty = Type::Obj;
      auto const cls = Unit::lookupUniqueClass(rat.clsName());
      if (classIsUniqueOrCtxParent(env, cls)) {
        if (rat.tag() == T::OptExactObj || rat.tag() == T::ExactObj) {
          ty = ty.specializeExact(cls);
        } else {
          ty = ty.specialize(cls);
        }
      }
      if (rat.tag() == T::OptExactObj || rat.tag() == T::OptSubObj) {
        ty = ty | Type::InitNull;
      }
      return ty;
    }

  case T::Cell:       return Type::Cell;
  case T::Ref:        return Type::BoxedCell;

  case T::InitGen:
    // Should ideally be able to remove Uninit here.
    return folly::none;
  case T::Gen:
    return folly::none;

  case T::InitUnc:    return Type::UncountedInit;
  case T::Unc:        return Type::Uncounted;
  // The JIT can't currently handle the exact information in this type
  // assertion in some cases:
  case T::InitCell:   return Type::Cell; // - Type::Uninit
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

SSATmp* implInstanceOfD(HTS& env, SSATmp* src, const StringData* className) {
  /*
   * InstanceOfD is always false if it's not an object.
   *
   * We're prepared to generate translations for known non-object
   * types, but if it's Gen/Cell we're going to PUNT because it's
   * natural to translate that case with control flow TODO(#2020251)
   */
  if (Type::Obj.strictSubtypeOf(src->type())) {
    PUNT(InstanceOfD_MaybeObj);
  }
  if (!src->isA(Type::Obj)) {
    bool res = ((src->isA(Type::Arr) && interface_supports_array(className))) ||
      (src->isA(Type::Str) && interface_supports_string(className)) ||
      (src->isA(Type::Int) && interface_supports_int(className)) ||
      (src->isA(Type::Dbl) && interface_supports_double(className));
    return cns(env, res);
  }

  if (s_WaitHandle.get()->isame(className)) {
    return gen(env, IsWaitHandle, src);
  }

  auto const objClass     = gen(env, LdObjClass, src);
  auto const ssaClassName = cns(env, className);

  InstanceBits::init();
  const bool haveBit = InstanceBits::lookup(className) != 0;

  auto const maybeCls = Unit::lookupUniqueClass(className);
  const bool isNormalClass = classIsUniqueNormalClass(maybeCls);
  const bool isUnique = classIsUnique(maybeCls);

  /*
   * If the class is a unique interface, we can just hit the class's
   * interfaces map and call it a day.
   */
  if (!haveBit && classIsUniqueInterface(maybeCls)) {
    return gen(env, InstanceOfIface, objClass, ssaClassName);
  }

  /*
   * If the class is unique or a parent of the current context, we
   * don't need to load it out of RDS because it must already exist
   * and be defined.
   *
   * Otherwise, we only use LdClsCachedSafe---instanceof with an
   * undefined class doesn't invoke autoload.
   */
  auto const checkClass =
    isUnique || (maybeCls && curClass(env) && curClass(env)->classof(maybeCls))
      ? cns(env, maybeCls)
      : gen(env, LdClsCachedSafe, ssaClassName);

  return
    haveBit ? gen(env, InstanceOfBitmask, objClass, ssaClassName) :
    isUnique && isNormalClass ? gen(env, ExtendsClass, objClass, checkClass) :
    gen(env, InstanceOf, objClass, checkClass);
}

//////////////////////////////////////////////////////////////////////

void emitInstanceOfD(HTS& env, const StringData* className) {
  auto const src = popC(env);
  push(env, implInstanceOfD(env, src, className));
  gen(env, DecRef, src);
}

void emitInstanceOf(HTS& env) {
  auto const t1 = popC(env);
  auto const t2 = popC(env); // t2 instanceof t1

  if (t1->isA(Type::Obj) && t2->isA(Type::Obj)) {
    auto const c2 = gen(env, LdObjClass, t2);
    auto const c1 = gen(env, LdObjClass, t1);
    push(env, gen(env, InstanceOf, c2, c1));
    gen(env, DecRef, t2);
    gen(env, DecRef, t1);
    return;
  }

  if (!t1->isA(Type::Str)) PUNT(InstanceOf-NotStr);

  if (t2->isA(Type::Obj)) {
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
    t2->isA(Type::Arr) ? gen(env, InterfaceSupportsArr, t1) :
    t2->isA(Type::Int) ? gen(env, InterfaceSupportsInt, t1) :
    t2->isA(Type::Str) ? gen(env, InterfaceSupportsStr, t1) :
    t2->isA(Type::Dbl) ? gen(env, InterfaceSupportsDbl, t1) :
    cns(env, false)
  );
  gen(env, DecRef, t2);
  gen(env, DecRef, t1);
}

void emitVerifyRetTypeC(HTS& env) {
  verifyTypeImpl(env, HPHP::TypeConstraint::ReturnId);
}

void emitVerifyRetTypeV(HTS& env) {
  verifyTypeImpl(env, HPHP::TypeConstraint::ReturnId);
}

void emitVerifyParamType(HTS& env, int32_t paramId) {
  verifyTypeImpl(env, paramId);
}

void emitOODeclExists(HTS& env, OODeclExistsOp subop) {
  auto const tAutoload = popC(env);
  auto const tCls = popC(env);

  assert(tCls->isA(Type::Str)); // result of CastString
  assert(tAutoload->isA(Type::Bool)); // result of CastBool

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

void emitIssetL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(env, gen(env, IsNType, Type::Null, ld));
}

void emitEmptyL(HTS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  push(
    env,
    gen(env, XorBool, gen(env, ConvCellToBool, ld), cns(env, true))
  );
}

void emitIsTypeC(HTS& env, IsTypeOp subop) {
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

void emitIsTypeL(HTS& env, int32_t id, IsTypeOp subop) {
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

void emitAssertRATL(HTS& env, int32_t loc, RepoAuthType rat) {
  if (auto const t = ratToAssertType(env, rat)) {
    assertTypeLocal(env, loc, *t);
  }
}

void emitAssertRATStk(HTS& env, int32_t offset, RepoAuthType rat) {
  if (auto const t = ratToAssertType(env, rat)) {
    assertTypeStack(env, offset, *t);
  }
}

//////////////////////////////////////////////////////////////////////

}}}

