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

#include "hphp/runtime/vm/jit/guard-constraint.h"
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
  s_Stringish("Stringish"),
  s_Awaitable("HH\\Awaitable");

//////////////////////////////////////////////////////////////////////

/*
 * Returns a {Cls|Nullptr} suitable for use in instance checks. If knownCls is
 * not null and is safe to use, that will be returned. Otherwise, className
 * will be used to look up a class.
 */
SSATmp* ldClassSafe(IRGS& env, const StringData* className,
                    const Class* knownCls = nullptr) {
  if (!knownCls) {
    knownCls = Unit::lookupUniqueClassInContext(className, curClass(env));
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
  if (s_Stringish.get()->isame(className)) {
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
      return gen(env, InstanceOfIfaceVtable, ClassData{knownCls}, objClass);
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
 * - PredInner: When the value is a BoxedInitCell, return the predicted inner
 *              type of the value.
 * - FuncToStr: Emit code to deal with any func to string conversions.
 * - ClsMethToVec: Emit code to deal with any ClsMeth to array conversions
 * - Fail:      Emit code to deal with the type check failing.
 * - HackArr:   Emit code to deal with a d/varray mismatch.
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
          typename PredInner,
          typename FuncToStr,
          typename ClassToStr,
          typename ClsMethToVec,
          typename Fail,
          typename HackArr,
          typename Callable,
          typename VerifyCls,
          typename Giveup>
void verifyTypeImpl(IRGS& env,
                    const TypeConstraint& tc,
                    bool onlyCheckNullability,
                    SSATmp* propCls,
                    GetVal getVal,
                    PredInner predInner,
                    FuncToStr funcToStr,
                    ClassToStr classToStr,
                    ClsMethToVec clsMethToVec,
                    Fail fail,
                    HackArr hackArr,
                    Callable callable,
                    VerifyCls verifyCls,
                    Giveup giveup) {
  if (!tc.isCheckable() || (RuntimeOption::EvalThisTypeHintLevel == 0
                            && !propCls && tc.isThis())) {
    return;
  }

  auto val = getVal();
  assertx(val->type() <= TCell || val->type() <= TBoxedCell);

  auto const valType = [&]() -> Type {
    if (val->type() <= TCell) return val->type();
    auto const pred = predInner(val);
    gen(env, CheckRefInner, pred, makeExit(env), val);
    val = gen(env, LdRef, pred, val);
    return pred;
  }();

  if (!valType.isKnownDataType()) return giveup();

  if (tc.isNullable() && valType <= TInitNull) return;

  auto const genFail = [&] {
    auto const thisFailsHard = [&] {
      if (propCls) return !tc.couldSeeMockObject();
      switch (RuntimeOption::EvalThisTypeHintLevel) {
        case 0:
          // We are not checking this typehints.
        case 2:
          // We are warning on this typehint failures.
          return false;
        case 1:
          // We are checking this typehints like self typehints.
          return true;
        case 3:
          // If we know there are no mock classes for the current class, it is
          // okay to fail hard.  Otherwise, mock objects may still pass, and we
          // have to be ready for execution to resume.
          return !tc.couldSeeMockObject();
      }
      always_assert(false);
    };

    auto const strictTypes = RuntimeOption::EnableHipHopSyntax ||
      curUnit(env)->isHHFile() ||
      !RuntimeOption::PHP7_ScalarTypes;
    auto const failHard = strictTypes
      && RuntimeOption::RepoAuthoritative
      && !tc.isSoft()
      && (!tc.isThis() || thisFailsHard());
    return fail(valType, failHard);
  };

  auto const genDVArrFail = [&]{
    hint(env, Block::Hint::Unlikely);
    hackArr(val);
  };

  auto const result =
    annotCompat(valType.toDataType(), tc.type(), tc.typeName());
  switch (result) {
    case AnnotAction::Pass: return;
    case AnnotAction::Fail: return genFail();
    case AnnotAction::CallableCheck:
      return callable(val);
    case AnnotAction::ObjectCheck:
      break;
    case AnnotAction::VArrayCheck:
      assertx(valType <= TArr);
      ifThen(
        env,
        [&] (Block* taken) { gen(env, CheckVArray, taken, val); },
        genDVArrFail
      );
      return;
    case AnnotAction::DArrayCheck:
      assertx(valType <= TArr);
      ifThen(
        env,
        [&] (Block* taken) { gen(env, CheckDArray, taken, val); },
        genDVArrFail
      );
      return;
    case AnnotAction::VArrayOrDArrayCheck:
      assertx(valType <= TArr);
      ifThen(
        env,
        [&] (Block* taken) {
          gen(env, JmpZero, taken, gen(env, IsDVArray, val));
        },
        genDVArrFail
      );
      return;
    case AnnotAction::NonVArrayOrDArrayCheck:
      assertx(valType <= TArr);
      ifThen(
        env,
        [&] (Block* taken) {
          gen(env, JmpNZero, taken, gen(env, IsDVArray, val));
        },
        genDVArrFail
      );
      return;
    case AnnotAction::WarnFunc:
      assertx(valType <= TFunc);
      gen(
        env,
        RaiseNotice,
        cns(
          env,
          makeStaticString("Implicit Func to string conversion for type-hint")
        )
      );

    case AnnotAction::ConvertFunc:
      assertx(valType <= TFunc);
      if (!funcToStr(val)) return genFail();
      return;

    case AnnotAction::WarnClass:
      assertx(valType <= TCls);
      gen(
        env,
        RaiseNotice,
        cns(
          env,
          makeStaticString("Implicit Class to string conversion for type-hint")
        )
      );

    case AnnotAction::ConvertClass:
      assertx(valType <= TCls);
      if (!classToStr(val)) return genFail();
      return;
    case AnnotAction::ClsMethCheck:
      assertx(valType <= TClsMeth);
      if (!clsMethToVec(val)) return genFail();
      return;
  }
  assertx(result == AnnotAction::ObjectCheck);
  if (onlyCheckNullability) return;

  if (!(valType <= TObj)) {
    if (tc.isResolved()) return genFail();
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
      auto const cachedClass = tc.namedEntity()->getCachedClass();
      if (cachedClass && classHasPersistentRDS(cachedClass) &&
          cachedClass->enumBaseTy() &&
          annotCompat(valType.toDataType(),
                      dataTypeToAnnotType(*cachedClass->enumBaseTy()),
                      nullptr) == AnnotAction::Pass) {
        env.irb->constrainValue(val, DataTypeSpecific);
        return;
      }
    }
    return giveup();
  }

  // At this point we know valType is Obj.
  if (tc.isThis() && (propCls || RuntimeOption::EvalThisTypeHintLevel >= 2)) {
    // For this type checks, the class needs to be an exact match.
    auto const ctxCls = propCls ? propCls : gen(env, LdClsCtx, ldCtx(env));
    auto const objClass = gen(env, LdObjClass, val);
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, JmpZero, taken, gen(env, EqCls, ctxCls, objClass));
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        genFail();
      }
    );
    return;
  }
  assertx(IMPLIES(tc.isThis(), RuntimeOption::EvalThisTypeHintLevel == 1));
  assertx(IMPLIES(tc.isThis(), !propCls));

  // If we reach here then valType is Obj and tc is Object, Self, or Parent
  const StringData* clsName;
  const Class* knownConstraint = nullptr;
  if (tc.isObject()) {
    auto const td = tc.namedEntity()->getCachedTypeAlias();
    if (RuntimeOption::RepoAuthoritative && td &&
        tc.namedEntity()->isPersistentTypeAlias() &&
        td->klass) {
      assertx(classHasPersistentRDS(td->klass));
      clsName = td->klass->name();
      knownConstraint = td->klass;
    } else {
      clsName = tc.typeName();
    }
  } else {
    if (tc.isSelf()
        || (tc.isThis() && RuntimeOption::EvalThisTypeHintLevel == 1)) {
      assertx(!propCls);
      knownConstraint = curFunc(env)->cls();
    } else {
      assertx(tc.isParent());
      assertx(!propCls);
      if (auto cls = curFunc(env)->cls()) knownConstraint = cls->parent();
    }
    if (!knownConstraint) {
      // The hint was self or parent and there's no corresponding
      // class for the current func. This typehint will always fail.
      return genFail();
    }
    clsName = knownConstraint->preClass()->name();
  }

  // For "self" and "parent", knownConstraint should always be
  // non-null at this point
  assertx(IMPLIES(tc.isSelf() || tc.isParent(), knownConstraint != nullptr));
  assertx(IMPLIES(tc.isSelf() || tc.isParent(), clsName != nullptr));
  assertx(IMPLIES(tc.isSelf() || tc.isParent(), !propCls));

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
  case IsTypeOp::Arr:     return TArr;
  case IsTypeOp::Keyset:  return TKeyset;
  case IsTypeOp::Obj:     return TObj;
  case IsTypeOp::ArrLike: return TArrLike;
  case IsTypeOp::Res:     return TRes;
  case IsTypeOp::ClsMeth: return TClsMeth;
  case IsTypeOp::Vec:
  case IsTypeOp::Dict:
  case IsTypeOp::VArray:
  case IsTypeOp::DArray:
  case IsTypeOp::Scalar: not_reached();
  }
  not_reached();
}

SSATmp* isScalarImpl(IRGS& env, SSATmp* val) {
  // The simplifier works fine when val has a known DataType, but do some
  // checks first in case val has a type like {Int|Str}.
  auto const scalar = TBool | TInt | TDbl | TStr;
  if (val->isA(scalar)) return cns(env, true);
  if (!val->type().maybe(scalar)) return cns(env, false);

  SSATmp* result = nullptr;
  for (auto t : {TBool, TInt, TDbl, TStr}) {
    auto const is_t = gen(env, ConvBoolToInt, gen(env, IsType, t, val));
    result = result ? gen(env, OrInt, result, is_t) : is_t;
  }
  return gen(env, ConvIntToBool, result);
}

SSATmp* isDVArrayImpl(IRGS& env, SSATmp* val, IsTypeOp op) {
  return cond(
    env,
    [&] (Block* taken) {
      auto const arr = gen(env, CheckType, TArr, taken, val);
      return gen(
        env,
        op == IsTypeOp::VArray ? CheckVArray : CheckDArray,
        taken,
        arr
      );
    },
    [&](SSATmp*) { return cns(env, true); },
    [&]{
      if (RuntimeOption::EvalHackArrCompatIsVecDictNotices) {
        ifElse(
          env,
          [&] (Block* taken) {
            gen(
              env,
              CheckType,
              op == IsTypeOp::VArray ? TVec : TDict,
              taken,
              val
            );
          },
          [&] {
            gen(
              env,
              RaiseHackArrCompatNotice,
              cns(
                env,
                makeStaticString(
                  op == IsTypeOp::VArray
                  ? Strings::HACKARR_COMPAT_VEC_IS_VARR
                  : Strings::HACKARR_COMPAT_DICT_IS_DARR
                )
              )
            );
          }
        );
      }

      // check if type is TClsMeth and raise notice
      if (op == IsTypeOp::VArray) {
        return cond(
          env,
          [&] (Block* taken) {
            return gen(env, CheckType, TClsMeth, taken, val);
          },
          [&] (SSATmp*) {
            if (!RuntimeOption::EvalHackArrDVArrs) {
              if (RuntimeOption::EvalIsVecNotices) {
                gen(env, RaiseNotice, cns(env,
                  makeStaticString(Strings::CLSMETH_COMPAT_IS_VARR)));
              }
              return cns(env, true);
            }
            return cns(env, false);
          },
          [&] { return cns(env, false); }
        );
      }
      return cns(env, false);
    }
  );
}

SSATmp* isVecImpl(IRGS& env, SSATmp* src) {
  if (!RuntimeOption::EvalHackArrCompatIsVecDictNotices) {
    return cond(
      env,
      [&] (Block* taken) {
        return gen(env, CheckType, TClsMeth, taken, src);
      },
      [&] (SSATmp*) {
        if (RuntimeOption::EvalHackArrDVArrs) {
          if (RuntimeOption::EvalIsVecNotices) {
            gen(env, RaiseNotice, cns(env,
              makeStaticString(Strings::CLSMETH_COMPAT_IS_VEC)));
          }
          return cns(env, true);
        }
        return cns(env, false);
      },
      [&] { return gen(env, IsType, TVec, src); }
    );
  }

  auto const varrCheck = [&]{
    cond(
      env,
      [&](Block* taken) { return gen(env, CheckType, TArr, taken, src); },
      [&](SSATmp* arr) {
        ifElse(
          env,
          [&](Block* taken) { gen(env, CheckVArray, taken, arr); },
          [&]{
            gen(
              env,
              RaiseHackArrCompatNotice,
              cns(env, makeStaticString(Strings::HACKARR_COMPAT_VARR_IS_VEC))
            );
          }
        );
        return nullptr;
      },
      [&]{ return nullptr; }
    );
  };

  return cond(
    env,
    [&](Block* taken) { gen(env, CheckType, TVec, taken, src); },
    [&]{ return cns(env, true); },
    [&]{
      varrCheck();
      return cond(
        env,
        [&] (Block* taken) {
          return gen(env, CheckType, TClsMeth, taken, src);
        },
        [&] (SSATmp*) {
          if (RuntimeOption::EvalHackArrDVArrs) {
            if (RuntimeOption::EvalIsVecNotices) {
              gen(env, RaiseNotice, cns(env,
                makeStaticString(Strings::CLSMETH_COMPAT_IS_VEC)));
            }
            return cns(env, true);
          }
          return cns(env, false);
        },
        [&] { return cns(env, false); }
      );
    }
  );
}

const StaticString s_FUNC_CONVERSION("Func to string conversion");
const StaticString s_FUNC_IS_STRING("Func used in is_string");
const StaticString s_CLASS_IS_STRING("Class used in is_string");
const StaticString s_CLASS_CONVERSION("Class to string conversion");

SSATmp* isStrImpl(IRGS& env, SSATmp* src) {
  return cond(
    env,
    [&] (Block* taken) { gen(env, CheckType, TStr, taken, src); },
    [&] { return cns(env, true); },
    [&] {
      return cond(
        env,
        [&] (Block* taken) { gen(env, CheckType, TFunc, taken, src); },
        [&] {
          if (RuntimeOption::EvalIsStringNotices) {
            gen(env, RaiseNotice, cns(env, s_FUNC_IS_STRING.get())
            );
          }
          return cns(env, true);
        },
        [&] {
          return cond(
            env,
            [&] (Block* taken) { gen(env, CheckType, TCls, taken, src); },
            [&] {
              if (RuntimeOption::EvalIsStringNotices) {
                gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get())
                );
              }
              return cns(env, true);
            },
            [&] { return cns(env, false); }
          );
        }
      );
    }
  );
}

SSATmp* isDictImpl(IRGS& env, SSATmp* src) {
  if (!RuntimeOption::EvalHackArrCompatIsVecDictNotices) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      return cond(
        env,
        [&](Block* taken) { gen(env, CheckType, TDict, taken, src); },
        [&]{ return cns(env, true); },
        [&]{ return gen(env, IsType, TShape, src); }
      );
    } else {
      return gen(env, IsType, TDict, src);
    }
  }

  auto const darrCheck = [&]{
    cond(
      env,
      [&](Block* taken) { return gen(env, CheckType, TArr, taken, src); },
      [&](SSATmp* arr) {
        ifElse(
          env,
          [&](Block* taken) { gen(env, CheckDArray, taken, arr); },
          [&]{
            gen(
              env,
              RaiseHackArrCompatNotice,
              cns(env, makeStaticString(Strings::HACKARR_COMPAT_DARR_IS_DICT))
            );
          }
        );
        return nullptr;
      },
      [&]{ return nullptr; }
    );
  };

  return cond(
    env,
    [&](Block* taken) { gen(env, CheckType, TDict, taken, src); },
    [&]{ return cns(env, true); },
    [&]{
      if (RuntimeOption::EvalHackArrDVArrs) {
        return cond(
          env,
          [&](Block* taken) { gen(env, CheckType, TShape, taken, src); },
          [&]{ return cns(env, true); },
          [&]{ darrCheck(); return cns(env, false); }
        );
      } else {
        darrCheck(); return cns(env, false);
      }
     }
  );
}

SSATmp* isArrayImpl(IRGS& env, SSATmp* src) {
  if (!RuntimeOption::EvalHackArrCompatIsArrayNotices ||
      curFunc(env)->isBuiltin()) {
    return cond(
      env,
      [&](Block* taken) { gen(env, CheckType, TArr, taken, src); },
      [&]{ return cns(env, true); },
      [&]{
        return cond(
          env,
          [&] (Block* taken) {
            return gen(env, CheckType, TClsMeth, taken, src);
          },
          [&] (SSATmp*) {
            if (!RuntimeOption::EvalHackArrDVArrs) {
              if (RuntimeOption::EvalIsVecNotices) {
                gen(env, RaiseNotice, cns(env,
                  makeStaticString(Strings::CLSMETH_COMPAT_IS_ARR)));
              }
              return cns(env, true);
            }
            return cns(env, false);
          },
          [&] {
            if (RuntimeOption::EvalHackArrDVArrs) {
              return cns(env, false);
            } else {
              return gen(env, IsType, TShape, src);
            }
          }
        );
      }
    );
  }

#define X(name, type, msg, next)                                        \
  auto const name = [&]{                                                \
    ifThenElse(                                                         \
      env,                                                              \
      [&](Block* taken) { gen(env, CheckType, type, taken, src); },     \
      [&]{                                                              \
        gen(                                                            \
          env,                                                          \
          RaiseHackArrCompatNotice,                                     \
          cns(env, makeStaticString(Strings::HACKARR_COMPAT_##msg##_IS_ARR)) \
        );                                                              \
      },                                                                \
      [&]{ next; }                                                      \
    );                                                                  \
  }

  X(keysetCheck, TKeyset, KEYSET,);
  X(dictCheck, TDict, DICT, keysetCheck());
  X(vecCheck, TVec, VEC, dictCheck());

#undef X

  return cond(
    env,
    [&](Block* taken) { gen(env, CheckType, TArr, taken, src); },
    [&]{ return cns(env, true); },
    [&]{
      return cond(
        env,
        [&] (Block* taken) {
          return gen(env, CheckType, TClsMeth, taken, src);
        },
        [&] (SSATmp*) {
          if (!RuntimeOption::EvalHackArrDVArrs) {
            if (RuntimeOption::EvalIsVecNotices) {
              gen(env, RaiseNotice, cns(env,
                makeStaticString(Strings::CLSMETH_COMPAT_IS_ARR)));
            }
            return cns(env, true);
          }
          return cns(env, false);
        },
        [&] {
          if (RuntimeOption::EvalHackArrDVArrs) {
            vecCheck();
            return cns(env, false);
          } else {
            return cond(
              env,
              [&](Block* taken) { gen(env, CheckType, TShape, taken, src); },
              [&]{ return cns(env, true); },
              [&]{
                vecCheck();
                if (!RuntimeOption::EvalHackArrDVArrs) {
                  return gen(env, IsType, TClsMeth, src);
                }
                return cns(env, false);
              }
            );
          }
        }
      );
    }
  );
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
    bool res = ((src->isA(TArr) && interface_supports_array(className))) ||
      (src->isA(TVec) && interface_supports_vec(className)) ||
      (src->isA(TDict) && interface_supports_dict(className)) ||
      (src->isA(TKeyset) && interface_supports_keyset(className)) ||
      (src->isA(TStr) && interface_supports_string(className)) ||
      (src->isA(TInt) && interface_supports_int(className)) ||
      (src->isA(TDbl) && interface_supports_double(className)) ||
      (src->isA(TClsMeth) && (RuntimeOption::EvalHackArrDVArrs ?
        interface_supports_vec(className) :
        interface_supports_array(className)));
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
    if (t2->isA(TArr))    return gen(env, InterfaceSupportsArr, t1);
    if (t2->isA(TVec))    return gen(env, InterfaceSupportsVec, t1);
    if (t2->isA(TDict))   return gen(env, InterfaceSupportsDict, t1);
    if (t2->isA(TKeyset)) return gen(env, InterfaceSupportsKeyset, t1);
    if (t2->isA(TInt))    return gen(env, InterfaceSupportsInt, t1);
    if (t2->isA(TStr))    return gen(env, InterfaceSupportsStr, t1);
    if (t2->isA(TDbl))    return gen(env, InterfaceSupportsDbl, t1);
    if (!t2->type().maybe(TObj|TArr|TVec|TDict|TKeyset|
                          TInt|TStr|TDbl)) return cns(env, false);
    return nullptr;
  }();

  if (!res) PUNT(InstanceOf-Unknown);

  push(env, res);
  decRef(env, t2);
  decRef(env, t1);
}

namespace {

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
      ? gen(env, LdClsCtx, ldCtx(env))
      : cns(env, nullptr);
  auto const result = gen(
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
  try {
    auto newTS = TypeStructure::resolvePartial(
      ArrNR(ts), nullptr, declaringCls, persistent, partial, invalidType);
    if (persistent) return ArrayData::GetScalarArray(std::move(newTS));
  } catch (Exception& e) {}
  // We are here because either we threw in the resolution or it wasn't
  // persistent resolution which means we didn't really resolve it
  partial = true;
  return ts;
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
 * This function tries to emit is/as type struct operations without resolving
 * the type structure when that's possible.
 * When it returns true, it has popped two values from the stack, namely the
 * type structure and the cell, and pushed one value back to stack, namely
 * true/false if it is an is-operation or the cell if it is an as operation.
 * This function does not modify the reference counts of these stack values,
 * leaving that responsibility to the caller.
 * When it returns false, it does not modify anything.
 */
bool emitIsAsTypeStructWithoutResolvingIfPossible(
  IRGS& env,
  const ArrayData* ts,
  bool asExpr
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

  // For as expressions, if the check succeeds, we want to return true without
  // doing anything whereas if it fails, we want the full asTypeStruct to run
  auto const success = [&] { return asExpr ? true : cnsResult(true); };
  auto const fail = [&] { return asExpr ? false : cnsResult(false); };

  auto const primitive = [&] (Type ty, bool should_negate = false) {
    auto const nty = is_nullable_ts ? ty|TNull : ty;
    if (t->isA(nty)) return should_negate ? fail() : success();
    if (!t->type().maybe(nty)) return should_negate ? success() : fail();
    if (asExpr) return false;
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
    if (asExpr) return false;

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
      if (t->isA(TFunc) && RuntimeOption::EvalRaiseFuncConversionWarning) {
        gen(env, RaiseWarning, cns(env, s_FUNC_IS_STRING.get()));
      } else if (t->isA(TCls) &&
        RuntimeOption::EvalRaiseClassConversionWarning) {
        gen(env, RaiseWarning, cns(env, s_CLASS_IS_STRING.get()));
      }
      return unionOf(TStr, TFunc, TCls);
    }
    case TypeStructure::Kind::T_null:        return primitive(TNull);
    case TypeStructure::Kind::T_void:        return primitive(TNull);
    case TypeStructure::Kind::T_keyset:      return primitive(TKeyset);
    case TypeStructure::Kind::T_nonnull:     return primitive(TNull, true);
    case TypeStructure::Kind::T_mixed:       return success();
    case TypeStructure::Kind::T_num:         return unionOf(TInt, TDbl);
    case TypeStructure::Kind::T_arraykey:    return unionOf(TInt, TStr);
    case TypeStructure::Kind::T_vec_or_dict:
      if (t->type().maybe(TClsMeth)) {
        if (t->isA(TClsMeth)) {
          if (RuntimeOption::EvalHackArrDVArrs) {
            if (RuntimeOption::EvalIsVecNotices) {
              gen(env, RaiseNotice,
                cns(env, makeStaticString(Strings::CLSMETH_COMPAT_IS_VEC)));
            }
            return success();
          } else {
            return fail();
          }
        } else {
          PUNT(TypeStructC-MaybeClsMeth);
        }
      }
      return unionOf(TVec, TDict);
    case TypeStructure::Kind::T_arraylike:
      if (t->type().maybe(TClsMeth)) {
        if (t->isA(TClsMeth)) {
          if (RuntimeOption::EvalIsVecNotices) {
            gen(env, RaiseNotice,
              cns(env, makeStaticString(Strings::CLSMETH_COMPAT_IS_ANY_ARR)));
          }
          return success();
        } else {
          PUNT(TypeStructC-MaybeClsMeth);
        }
      }
      return unionOf(TArr, TVec, TDict, TKeyset);
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec: {
      if (asExpr) return false;
      popC(env); // pop the ts that's on the stack
      auto const c = popC(env);
      auto const res = kind == TypeStructure::Kind::T_dict
        ? isDictImpl(env, c)
        : isVecImpl(env, c);
      push(env, is_nullable_ts ? check_nullable(env, res, c) : res);
      return true;
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      if (asExpr) return false;
      auto const clsname = get_ts_classname(ts);
      auto cls = Unit::lookupUniqueClassInContext(clsname, curClass(env));
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
    case TypeStructure::Kind::T_noreturn:
      return fail();
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
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
SSATmp* handleIsAsResolutionAndCommonOpts(
  IRGS& env,
  TypeStructResolveOp op,
  bool asExpr,
  bool& done,
  bool& shouldDecRef
) {
  auto const a = topC(env);
  auto const required_ts_type = RuntimeOption::EvalHackArrDVArrs ? TDict : TArr;
  if (!a->isA(required_ts_type)) PUNT(TypeStructC-NotArrayTypeStruct);
  if (!a->hasConstVal(required_ts_type)) {
    if (op == TypeStructResolveOp::Resolve) {
      return resolveTypeStructImpl(env, true, !asExpr, 1, true);
    }
    shouldDecRef = false;
    return popC(env);
  }
  auto const ts =
    RuntimeOption::EvalHackArrDVArrs ? a->dictVal() : a->arrVal();
  auto maybe_resolved = ts;
  bool partial = true;
  bool invalidType = true;
  if (op == TypeStructResolveOp::Resolve) {
    maybe_resolved =
      staticallyResolveTypeStructure(env, ts, partial, invalidType);
    shouldDecRef = maybe_resolved != ts;
  }
  if (emitIsAsTypeStructWithoutResolvingIfPossible(env, maybe_resolved,
                                                   asExpr)) {
    done = true;
    return nullptr;
  }
  if (op == TypeStructResolveOp::Resolve && (partial || invalidType)) {
    shouldDecRef = true;
    return resolveTypeStructImpl(
      env, typeStructureCouldBeNonStatic(ArrNR(ts)), !asExpr, 1, true);
  }
  popC(env);
  return cns(env, maybe_resolved);
}

} // namespace

void emitIsTypeStructC(IRGS& env, TypeStructResolveOp op) {
  auto const a = topC(env);
  auto const c = topC(env, BCSPRelOffset { 1 });
  bool done = false, shouldDecRef = true;
  SSATmp* tc =
    handleIsAsResolutionAndCommonOpts(env, op, false, done, shouldDecRef);
  if (done) {
    decRef(env, c);
    decRef(env, a);
    return;
  }
  popC(env);
  auto block = opcodeMayRaise(IsTypeStruct) && shouldDecRef
    ? create_catch_block(env, [&]{ decRef(env, tc); })
    : nullptr;
  push(env, gen(env, IsTypeStruct, block, tc, c));
  decRef(env, c);
  decRef(env, a);
}

void emitAsTypeStructC(IRGS& env, TypeStructResolveOp op) {
  /*
   * Expecting as-check to fail rarely and since is-check is cheaper,
   * run is-check first and if it fails run the as-check to generate the
   * exception
   */
  auto const a = topC(env);
  bool done = false, shouldDecRef = true;
  SSATmp* tc =
    handleIsAsResolutionAndCommonOpts(env, op, true, done, shouldDecRef);
  if (done) return decRef(env, a);
  auto const c = topC(env);
  ifThen(
    env,
    [&](Block* taken) {
      auto block = opcodeMayRaise(IsTypeStruct) && shouldDecRef
        ? create_catch_block(env, [&]{ decRef(env, tc); })
        : nullptr;
      auto const res = gen(env, IsTypeStruct, block, tc, c);
      gen(env, JmpZero, taken, res);
    },
    [&]{
      auto block = shouldDecRef
        ? create_catch_block(env, [&]{ decRef(env, tc); }) : nullptr;
      gen(env, AsTypeStruct, block, tc, c);
    }
  );
  decRef(env, a);
}

void emitRecordReifiedGeneric(IRGS& env, uint32_t n) {
  assertx(n != 0);
  auto const result = gen(
    env,
    RecordReifiedGenericsAndGetTSList,
    StackRangeData { spOffBCFromIRSP(env), static_cast<uint32_t>(n) },
    sp(env)
  );
  discard(env, n);
  push(env, result);
}

void emitReifiedName(IRGS& env, uint32_t n, const StringData* name) {
  assertx(n != 0);
  auto const result = gen(
    env,
    RecordReifiedGenericsAndGetName,
    StackRangeData { spOffBCFromIRSP(env), static_cast<uint32_t>(n) },
    sp(env)
  );
  auto const mangledName = gen(env, MangleReifiedName, cns(env, name), result);
  discard(env, n);
  push(env, mangledName);
}

void emitCombineAndResolveTypeStruct(IRGS& env, uint32_t n) {
  push(env, resolveTypeStructImpl(env, true, false, n, false));
}

namespace {

void raiseClsmethCompatTypeHint(
  IRGS& env, int32_t id, const Func* func, const TypeConstraint& tc) {
  auto name = tc.displayName(func->cls());
  if (id == TypeConstraint::ReturnId) {
    gen(env, RaiseNotice, cns(env, makeStaticString(
      folly::sformat("class_meth Compat: Value returned from function {}() "
      "must be of type {}, clsmeth given",
        func->fullDisplayName(), name))));
  } else {
    gen(env, RaiseNotice, cns(env, makeStaticString(
      folly::sformat("class_meth Compat: Argument {} passed to {}() "
      "must be of type {}, clsmeth given",
        id + 1, func->fullDisplayName(), name))));
  }
}

void verifyRetTypeImpl(IRGS& env, int32_t id, int32_t ind,
                       bool onlyCheckNullability) {
  if (!RuntimeOption::EvalCheckReturnTypeHints) return;

  auto const func = curFunc(env);
  auto const& tc = (id == TypeConstraint::ReturnId)
    ? func->returnTypeConstraint()
    : func->params()[id].typeConstraint;
  assertx(ind >= 0);

  verifyTypeImpl(
    env,
    tc,
    onlyCheckNullability,
    nullptr,
    [&] { // Get value to test
      return topC(env, BCSPRelOffset { ind });
    },
    [] (SSATmp*) -> Type { // Get boxed inner value
      PUNT(VerifyReturnTypeBoxed);
    },
    [&] (SSATmp* val) { // func to string conversions
      auto const str = gen(env, LdFuncName, val);
      discard(env, 1);
      push(env, str);
      return true;
    },
    [&] (SSATmp* val) { // class to string conversions
      auto const str = gen(env, LdClsName, val);
      discard(env, 1);
      push(env, str);
      return true;
    },
    [&] (SSATmp* val) { // clsmeth to varray/vec conversions
      if (RuntimeOption::EvalVecHintNotices) {
        raiseClsmethCompatTypeHint(env, id, func, tc);
      }
      auto clsMethArr = convertClsMethToVec(env, val);
      discard(env, 1);
      push(env, clsMethArr);
      decRef(env, val);
      return true;
    },
    [&] (Type, bool hard) { // Check failure
      updateMarker(env);
      env.irb->exceptionStackBoundary();
      auto const failHard =
        hard && RuntimeOption::EvalCheckReturnTypeHints >= 3;
      gen(
        env,
        failHard ? VerifyRetFailHard : VerifyRetFail,
        ParamData { id },
        ldStkAddr(env, BCSPRelOffset { ind })
      );
    },
    [&] (SSATmp* val) { // d/varray mismatch notice
      gen(
        env,
        RaiseHackArrParamNotice,
        RaiseHackArrParamNoticeData { tc.type(), id, true },
        val,
        cns(env, func)
      );
    },
    [&] (SSATmp* val) { // Callable check
      gen(
        env,
        VerifyRetCallable,
        ParamData { id },
        val
      );
    },
    [&] (SSATmp* val, SSATmp* objClass, SSATmp* checkCls) {
      // Class/type-alias check
      gen(
        env,
        VerifyRetCls,
        ParamData { id },
        objClass,
        checkCls,
        cns(env, uintptr_t(&tc)),
        val
      );
    },
    [] { // Giveup
      PUNT(VerifyReturnType);
    }
  );
}

void verifyParamTypeImpl(IRGS& env, int32_t id) {
  auto const func = curFunc(env);
  auto const& tc = func->params()[id].typeConstraint;
  verifyTypeImpl(
    env,
    tc,
    false,
    nullptr,
    [&] { // Get value to test
      auto const ldPMExit = makePseudoMainExit(env);
      return ldLoc(env, id, ldPMExit, DataTypeSpecific);
    },
    [&] (SSATmp* val) { // Get boxed inner type
      return env.irb->predictedLocalInnerType(id);
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
    [&] (SSATmp* val) { // clsmeth to varray/vec conversions
      if (RuntimeOption::EvalVecHintNotices) {
        raiseClsmethCompatTypeHint(env, id, func, tc);
      }
      auto clsMethArr = convertClsMethToVec(env, val);
      stLocRaw(env, id, fp(env), clsMethArr);
      decRef(env, val);
      return true;
    },
    [&] (Type valType, bool hard) { // Check failure
      auto const failHard = hard && RuntimeOption::EvalHardTypeHints &&
        !(tc.isArray() && valType.maybe(TObj)) &&
        !verify_fail_may_coerce(func);
      gen(
        env,
        failHard ? VerifyParamFailHard : VerifyParamFail,
        cns(env, id)
      );
    },
    [&] (SSATmp* val) { // d/varray mismatch
      gen(
        env,
        RaiseHackArrParamNotice,
        RaiseHackArrParamNoticeData { tc.type(), id, false },
        val,
        cns(env, func)
      );
    },
    [&] (SSATmp* val) { // Callable check
      gen(
        env,
        VerifyParamCallable,
        val,
        cns(env, id)
      );
    },
    [&] (SSATmp*, SSATmp* objClass, SSATmp* checkCls) {
      // Class/type-alias check
      gen(
        env,
        VerifyParamCls,
        objClass,
        checkCls,
        cns(env, uintptr_t(&tc)),
        cns(env, id)
      );
    },
    [] { // Giveup
      PUNT(VerifyParamType);
    }
  );
}

}

void verifyPropType(IRGS& env,
                    SSATmp* cls,
                    const HPHP::TypeConstraint* tc,
                    Slot slot,
                    SSATmp* val,
                    SSATmp* name,
                    bool isSProp) {
  assertx(cls->isA(TCls));
  assertx(val->isA(TCell));

  if (RuntimeOption::EvalCheckPropTypeHints <= 0) return;
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
    [&] (SSATmp*) -> Type { // Get boxed inner type
      // We've already asserted that the value is a Cell.
      always_assert(false);
    },
    [&] (SSATmp*) { return false; }, // No func to string automatic conversions
    [&] (SSATmp*) { return false; }, // No class to string automatic conversions
    [&] (SSATmp*) { return false; }, // No clsmeth to vec automatic conversions
    [&] (Type, bool hard) { // Check failure
      auto const failHard =
        hard && RuntimeOption::EvalCheckPropTypeHints >= 3;
      gen(
        env,
        failHard ? VerifyPropFailHard : VerifyPropFail,
        cls,
        cns(env, slot),
        val,
        cns(env, isSProp)
      );
    },
    [&] (SSATmp* val) { // d/varray mismatch
      gen(
        env,
        RaiseHackArrPropNotice,
        RaiseHackArrNoticeData { tc->type(), },
        cls,
        val,
        cns(env, slot),
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
      // verifyPropType without us worrying about it punting the entire
      // operation.
      gen(env, VerifyProp, cls, cns(env, slot), val, cns(env, isSProp));
    }
  );
}

void emitVerifyRetTypeC(IRGS& env) {
  verifyRetTypeImpl(env, TypeConstraint::ReturnId, 0, false);
}

void emitVerifyRetTypeTS(IRGS& env) {
  verifyRetTypeImpl(env, TypeConstraint::ReturnId, 1, false);
  auto const ts = popC(env);
  auto const cell = topC(env);
  gen(env, VerifyReifiedReturnType, cell, ts);
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
  gen(env, VerifyReifiedLocalType, ParamData { paramId }, ts);
}

void emitOODeclExists(IRGS& env, OODeclExistsOp subop) {
  auto const tAutoload = topC(env);
  auto const tCls = topC(env);

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
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (ld->isA(TClsMeth)) {
    PUNT(IssetL_is_ClsMeth);
  }
  push(env, gen(env, IsNType, TNull, ld));
}

void emitEmptyL(IRGS& env, int32_t id) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);
  if (ld->isA(TClsMeth)) {
    PUNT(EmptyL_is_ClsMeth);
  }
  push(
    env,
    gen(env, XorBool, gen(env, ConvCellToBool, ld), cns(env, true))
  );
}

void emitIsTypeC(IRGS& env, IsTypeOp subop) {
  auto const src = popC(env, DataTypeSpecific);

  if (subop == IsTypeOp::VArray || subop == IsTypeOp::DArray) {
    push(env, isDVArrayImpl(env, src, subop));
  } else if (subop == IsTypeOp::Arr) {
    push(env, isArrayImpl(env, src));
  } else if (subop == IsTypeOp::Vec) {
    push(env, isVecImpl(env, src));
  } else if (subop == IsTypeOp::Dict) {
    push(env, isDictImpl(env, src));
  } else if (subop == IsTypeOp::Scalar) {
    push(env, isScalarImpl(env, src));
  } else if (subop == IsTypeOp::Str) {
    push(env, isStrImpl(env, src));
  } else {
    if (subop == IsTypeOp::ArrLike && src->isA(TClsMeth)) {
      // To make ClsMeth compatiable with arraylike tentitively
      if (RuntimeOption::EvalIsVecNotices) {
        gen(env, RaiseNotice,
          cns(env, makeStaticString(Strings::CLSMETH_COMPAT_IS_ANY_ARR)));
      }
      push(env, cns(env, true));
    } else {
      auto const t = typeOpToType(subop);
      if (t <= TObj) {
        push(env, optimizedCallIsObject(env, src));
      } else {
        push(env, gen(env, IsType, t, src));
      }
    }
  }
  decRef(env, src);
}

void emitIsTypeL(IRGS& env, int32_t id, IsTypeOp subop) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
  auto const val =
    ldLocInnerWarn(env, id, ldrefExit, ldPMExit, DataTypeSpecific);

  if (subop == IsTypeOp::VArray || subop == IsTypeOp::DArray) {
    push(env, isDVArrayImpl(env, val, subop));
  } else if (subop == IsTypeOp::Arr) {
    push(env, isArrayImpl(env, val));
  } else if (subop == IsTypeOp::Vec) {
    push(env, isVecImpl(env, val));
  } else if (subop == IsTypeOp::Dict) {
    push(env, isDictImpl(env, val));
  } else if (subop == IsTypeOp::Scalar) {
    push(env, isScalarImpl(env, val));
  } else if (subop == IsTypeOp::Str) {
    push(env, isStrImpl(env, val));
  } else {
    if (subop == IsTypeOp::ArrLike && val->isA(TClsMeth)) {
      // To make ClsMeth compatiable with arraylike tentitively
      if (RuntimeOption::EvalIsVecNotices) {
        gen(env, RaiseNotice,
          cns(env, makeStaticString(Strings::CLSMETH_COMPAT_IS_ANY_ARR)));
      }
      push(env, cns(env, true));
    } else {
      auto const t = typeOpToType(subop);
      if (t <= TObj) {
        push(env, optimizedCallIsObject(env, val));
      } else {
        push(env, gen(env, IsType, t, val));
      }
    }
  }
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
