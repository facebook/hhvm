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

SSATmp* ldRecDescSafe(IRGS& env, const StringData* recName) {
  return cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdRecDescCachedSafe, RecNameData{recName}, taken);
    },
    [&] (SSATmp* rec) { // next
      return rec;
    },
    [&] { // taken
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
 * - DVArr:     Emit code to deal with a dvarray mismatch.
 * - Callable:  Emit code to verify that the given value is callable.
 * - VerifyCls: Emit code to verify that the given value is an instance of the
 *              given Class.
 * - VerifyRecordDesc: Emit code to verify that the given value is an instance
 *              of the given record.
 * - Giveup:    Called when the type check cannot be resolved statically. Either
 *              PUNT or call a runtime helper to do the check.
 *
 * `propCls' should only be non-null for property type-hints, and represents the
 * runtime class of the object the property belongs to.
 */
template <typename GetVal,
          typename FuncToStr,
          typename ClassToStr,
          typename ClsMethToVec,
          typename Fail,
          typename DVArr,
          typename Callable,
          typename VerifyCls,
          typename VerifyRecordDesc,
          typename Giveup>
void verifyTypeImpl(IRGS& env,
                    const TypeConstraint& tc,
                    bool onlyCheckNullability,
                    SSATmp* propCls,
                    GetVal getVal,
                    FuncToStr funcToStr,
                    ClassToStr classToStr,
                    ClsMethToVec clsMethToVec,
                    Fail fail,
                    DVArr dvArr,
                    Callable callable,
                    VerifyCls verifyCls,
                    VerifyRecordDesc verifyRecDesc,
                    Giveup giveup) {

  if (!tc.isCheckable()) return;
  assertx(!tc.isUpperBound() || RuntimeOption::EvalEnforceGenericsUB != 0);

  auto val = getVal();
  assertx(val->type() <= TCell);

  auto const valType = val->type();

  if (!valType.isKnownDataType()) return giveup();

  if (tc.isNullable() && valType <= TInitNull) return;

  auto const genFail = [&] {
    // If we know there are no mock classes for the current class, it is
    // okay to fail hard.  Otherwise, mock objects may still pass, and we
    // have to be ready for execution to resume.
    auto const thisFailsHard = !tc.couldSeeMockObject();

    auto const failHard = RuntimeOption::RepoAuthoritative
      && !tc.isSoft()
      && (!tc.isThis() || thisFailsHard)
      && (!tc.isUpperBound() || RuntimeOption::EvalEnforceGenericsUB >= 2);
    return fail(valType, failHard);
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
    case AnnotAction::DArrayCheck:
    case AnnotAction::VArrayOrDArrayCheck:
    case AnnotAction::NonVArrayOrDArrayCheck: {
      assertx(valType <= TArr);
      auto const gc = GuardConstraint(DataTypeSpecialized).setWantArrayKind();
      env.irb->constrainValue(val, gc);
      return dvArr(val);
    }
    case AnnotAction::WarnFunc:
      assertx(valType <= TFunc);
      if (!funcToStr(val)) return genFail();
      gen(
        env,
        RaiseNotice,
        cns(
          env,
          makeStaticString(Strings::FUNC_TO_STRING_IMPLICIT)
        )
      );
      return;

    case AnnotAction::ConvertFunc:
      assertx(valType <= TFunc);
      if (!funcToStr(val)) return genFail();
      return;

    case AnnotAction::WarnClass:
      assertx(valType <= TCls);
      if (!classToStr(val)) return genFail();
      gen(
        env,
        RaiseNotice,
        cns(
          env,
          makeStaticString(Strings::CLASS_TO_STRING_IMPLICIT)
        )
      );
      return;

    case AnnotAction::ConvertClass:
      assertx(valType <= TCls);
      if (!classToStr(val)) return genFail();
      return;
    case AnnotAction::ClsMethCheck:
      assertx(valType <= TClsMeth);
      if (!clsMethToVec(val)) return genFail();
      return;
    case AnnotAction::RecordCheck:
      assertx(valType <= TRecord);
      auto const rec = Unit::lookupUniqueRecDesc(tc.typeName());
      auto const isPersistent = recordHasPersistentRDS(rec);
      auto const checkRecDesc = isPersistent ?
        cns(env, rec) : ldRecDescSafe(env, tc.typeName());
      verifyRecDesc(gen(env, LdRecDesc, val), checkRecDesc, val);
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
             td->klass ?
             td->klass->name() :
             (td->rec ? td->rec->name() : nullptr)) == AnnotAction::Pass)) {
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
        genFail();
      }
    );
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
      assertx(classHasPersistentRDS(td->klass));
      clsName = td->klass->name();
      knownConstraint = td->klass;
    } else {
      clsName = tc.typeName();
    }
  } else {
    assertx(!propCls);
    if (tc.isSelf()) {
      knownConstraint = curFunc(env)->cls();
    } else {
      assertx(tc.isParent());
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
  case IsTypeOp::PHPArr:
  case IsTypeOp::Arr:     return TArr;
  case IsTypeOp::Keyset:  return TKeyset;
  case IsTypeOp::Obj:     return TObj;
  case IsTypeOp::ArrLike: return TArrLike;
  case IsTypeOp::Res:     return TRes;
  case IsTypeOp::ClsMeth: return TClsMeth;
  case IsTypeOp::Func:    return TFunc;
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

const StaticString s_FUNC_CONVERSION(Strings::FUNC_TO_STRING);
const StaticString s_FUNC_IS_STRING("Func used in is_string");
const StaticString s_CLASS_CONVERSION(Strings::CLASS_TO_STRING);
const StaticString s_CLASS_IS_STRING("Class used in is_string");

SSATmp* isStrImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TStr, [&](SSATmp*) { return cns(env, true); });

  mc.ifTypeThen(src, TFunc, [&](SSATmp*) {
    if (RuntimeOption::EvalIsStringNotices) {
      gen(env, RaiseNotice, cns(env, s_FUNC_IS_STRING.get()));
    }
    return cns(env, true);
  });

  mc.ifTypeThen(src, TCls, [&](SSATmp*) {
    if (RuntimeOption::EvalIsStringNotices) {
      gen(env, RaiseNotice, cns(env, s_CLASS_IS_STRING.get()));
    }
    return cns(env, true);
  });

  return mc.elseDo([&]{ return cns(env, false); });
}

// Checks if `arr` is a darray, varray, or varray (based on op). Executes the
// code in `next` if that is the case.
//
// The JIT's type system doesn't track dvarray-ness, and it's not particularly
// useful in general, but we implement a simple optimization here based on the
// instruction that created src. This optimization is valuable because it lets
// us check dvarray-ness for other reasons and then call maybeLogSerialization,
// which is the canonical safe way to do provenance-based logging.
template <typename Next>
void ifDVArray(IRGS& env, Opcode op, SSATmp* arr, Next next) {
  assertx(arr->isA(TArr));
  assertx(op == CheckDArray || op == CheckVArray || op == CheckDVArray);

  auto const gc = GuardConstraint(DataTypeSpecialized).setWantArrayKind();
  env.irb->constrainValue(arr, gc);

  if (arr->inst()->is(CheckDArray, CheckVArray, CheckDVArray)) {
    next(arr);
  } else {
    ifThen(env, [&](Block* taken) { next(gen(env, op, taken, arr)); }, [&]{});
  }
}

// Checks if the `arr` has provenance, implementing the same logic as in the
// runtume helper arrprov::arrayWantsTag. If so, logs a serialization notice.
void maybeLogSerialization(IRGS& env, SSATmp* arr, SerializationSite site) {
  assertx(arr->type().isKnownDataType());

  if (!RO::EvalLogArrayProvenance) return;

  if (arr->isA(TArr) && RO::EvalArrProvDVArrays) {
    ifDVArray(env, CheckDVArray, arr, [&](SSATmp* arr) {
      gen(env, RaiseArraySerializeNotice, cns(env, site), arr);
    });
  } else if ((arr->isA(TVec) || arr->isA(TDict)) && RO::EvalArrProvHackArrays) {
    gen(env, RaiseArraySerializeNotice, cns(env, site), arr);
  }
}

SSATmp* isArrayImpl(IRGS& env, SSATmp* src, bool log_on_hack_arrays) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TArr, [&](SSATmp* src) {
    maybeLogSerialization(env, src, SerializationSite::IsArray);
    return cns(env, true);
  });

  if (!RO::EvalHackArrDVArrs && RO::EvalIsCompatibleClsMethType) {
    mc.ifTypeThen(src, TClsMeth, [&](SSATmp*) {
      if (RO::EvalIsVecNotices) {
        auto const msg = makeStaticString(Strings::CLSMETH_COMPAT_IS_ARR);
        gen(env, RaiseNotice, cns(env, msg));
      }
      return cns(env, true);
    });
  }

  auto const hacLogging = [&](const char* msg) {
    if (!RO::EvalHackArrCompatIsArrayNotices) return;
    gen(env, RaiseHackArrCompatNotice, cns(env, makeStaticString(msg)));
  };

  if (!curFunc(env)->isBuiltin() &&
      (RO::EvalHackArrCompatIsArrayNotices ||
      (RO::EvalLogArrayProvenance && RO::EvalArrProvHackArrays)) &&
      log_on_hack_arrays) {
    mc.ifTypeThen(src, TVec, [&](SSATmp* src) {
      hacLogging(Strings::HACKARR_COMPAT_VEC_IS_ARR);
      maybeLogSerialization(env, src, SerializationSite::IsArray);
      return cns(env, false);
    });
    mc.ifTypeThen(src, TDict, [&](SSATmp* src) {
      hacLogging(Strings::HACKARR_COMPAT_DICT_IS_ARR);
      maybeLogSerialization(env, src, SerializationSite::IsArray);
      return cns(env, false);
    });
    mc.ifTypeThen(src, TKeyset, [&](SSATmp* src) {
      hacLogging(Strings::HACKARR_COMPAT_KEYSET_IS_ARR);
      return cns(env, false);
    });
  }

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isVecImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};


  mc.ifTypeThen(src, TVec, [&](SSATmp* src) {
    maybeLogSerialization(env, src, SerializationSite::IsVec);
    return cns(env, true);
  });

  auto const hacLogging = [&](const char* msg) {
    if (!RO::EvalHackArrCompatIsVecDictNotices) return;
    gen(env, RaiseHackArrCompatNotice, cns(env, makeStaticString(msg)));
  };

  if (RO::EvalIsCompatibleClsMethType) {
    if (RO::EvalHackArrDVArrs) {
      mc.ifTypeThen(src, TClsMeth, [&](SSATmp* src) {
        if (RO::EvalIsVecNotices) {
          gen(env, RaiseNotice,
              cns(env, makeStaticString(Strings::CLSMETH_COMPAT_IS_VEC)));
        }
        return cns(env, true);
      });
    } else {
      mc.ifTypeThen(src, TClsMeth, [&](SSATmp* src) {
        hacLogging(Strings::HACKARR_COMPAT_VARR_IS_VEC);
        return cns(env, false);
      });
    }
  }

  if (RO::EvalHackArrCompatIsVecDictNotices ||
      (RO::EvalLogArrayProvenance && RO::EvalArrProvDVArrays)) {
    mc.ifTypeThen(src, TArr, [&](SSATmp* src) {
      ifDVArray(env, CheckVArray, src, [&](SSATmp* src) {
        hacLogging(Strings::HACKARR_COMPAT_VARR_IS_VEC);
        maybeLogSerialization(env, src, SerializationSite::IsVec);
      });
      return cns(env, false);
    });
  }

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isDictImpl(IRGS& env, SSATmp* src) {
  MultiCond mc{env};

  mc.ifTypeThen(src, TDict, [&](SSATmp* src) {
    maybeLogSerialization(env, src, SerializationSite::IsDict);
    return cns(env, true);
  });

  auto const hacLogging = [&](const char* msg) {
    if (!RO::EvalHackArrCompatIsVecDictNotices) return;
    gen(env, RaiseHackArrCompatNotice, cns(env, makeStaticString(msg)));
  };

  if (RO::EvalHackArrCompatIsVecDictNotices ||
      (RO::EvalLogArrayProvenance && RO::EvalArrProvDVArrays)) {
    mc.ifTypeThen(src, TArr, [&](SSATmp* src) {
      ifDVArray(env, CheckDArray, src, [&](SSATmp* src) {
        hacLogging(Strings::HACKARR_COMPAT_DARR_IS_DICT);
        maybeLogSerialization(env, src, SerializationSite::IsDict);
      });
      return cns(env, false);
    });
  }

  return mc.elseDo([&]{ return cns(env, false); });
}

SSATmp* isDVArrayImpl(IRGS& env, SSATmp* src, IsTypeOp subop) {
  MultiCond mc{env};

  if (src->type().maybe(TArr)) {
    auto const gc = GuardConstraint(DataTypeSpecialized).setWantArrayKind();
    env.irb->constrainValue(src, gc);
  }

  assertx(subop == IsTypeOp::VArray || subop == IsTypeOp::DArray);
  auto const varray = subop == IsTypeOp::VArray;
  auto const check = varray ? CheckVArray : CheckDArray;
  auto const site = varray ? SerializationSite::IsVArray
                           : SerializationSite::IsDArray;

  mc.ifTypeThen(src, TArr, [&](SSATmp* src) {
    return cond(env,
      [&](Block* taken) { return gen(env, check, taken, src); },
      [&](SSATmp* src) {
        maybeLogSerialization(env, src, site);
        return cns(env, true);
      },
      [&]{ return cns(env, false); }
    );
  });

  if (varray && RO::EvalIsCompatibleClsMethType) {
    mc.ifTypeThen(src, TClsMeth, [&](SSATmp*) {
      if (RO::EvalIsVecNotices) {
        auto const msg = makeStaticString(Strings::CLSMETH_COMPAT_IS_VARR);
        gen(env, RaiseNotice, cns(env, msg));
      }
      return cns(env, true);
    });
  }


  auto const hacLogging = [&](const char* msg) {
    if (!RO::EvalHackArrCompatIsVecDictNotices) return;
    gen(env, RaiseHackArrCompatNotice, cns(env, makeStaticString(msg)));
  };

  mc.ifTypeThen(src, varray ? TVec : TDict, [&](SSATmp* src) {
    hacLogging(varray ? Strings::HACKARR_COMPAT_VEC_IS_VARR
                      : Strings::HACKARR_COMPAT_DICT_IS_DARR);
    maybeLogSerialization(env, src, site);
    return cns(env, false);
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
    if (src->type().subtypeOfAny(TCls, TFunc)) {
      if (!interface_supports_string(className)) return cns(env, false);
      if (RuntimeOption::EvalIsStringNotices) {
        gen(
          env,
          RaiseNotice,
          cns(
            env,
            src->isA(TFunc) ? s_FUNC_IS_STRING.get() : s_CLASS_IS_STRING.get()
          )
        );
      }
      return cns(env, true);
    }

    if (src->isA(TClsMeth)) {
      if (RuntimeOption::EvalHackArrDVArrs
          ? !interface_supports_vec(className)
          : !interface_supports_array(className)) {
        return cns(env, false);
      }

      if (RO::EvalIsVecNotices) {
        gen(
          env,
          RaiseNotice,
          cns(
            env,
            makeStaticString(folly::sformat(
              "Implicit clsmeth to {} conversion", className->data()
            ))
          )
        );
      }

      return cns(env, true);
    }

    bool res = ((src->isA(TArr) && interface_supports_array(className))) ||
      (src->isA(TVec) && interface_supports_vec(className)) ||
      (src->isA(TDict) && interface_supports_dict(className)) ||
      (src->isA(TKeyset) && interface_supports_keyset(className)) ||
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
    if (t2->isA(TArr))    return gen(env, InterfaceSupportsArr, t1);
    if (t2->isA(TVec))    return gen(env, InterfaceSupportsVec, t1);
    if (t2->isA(TDict))   return gen(env, InterfaceSupportsDict, t1);
    if (t2->isA(TKeyset)) return gen(env, InterfaceSupportsKeyset, t1);
    if (t2->isA(TInt))    return gen(env, InterfaceSupportsInt, t1);
    if (t2->isA(TStr))    return gen(env, InterfaceSupportsStr, t1);
    if (t2->isA(TDbl))    return gen(env, InterfaceSupportsDbl, t1);
    if (t2->isA(TFunc) || t2->isA(TCls)) {
      auto const warn =
        (t2->isA(TCls) && RuntimeOption::EvalRaiseClassConversionWarning) ||
        (t2->isA(TFunc) && RuntimeOption::EvalRaiseFuncConversionWarning);

      if (!warn) return gen(env, InterfaceSupportsStr, t1);
      return cond(
        env,
        [&] (Block* taken) {
          gen(env, JmpZero, taken, gen(env, InterfaceSupportsStr, t1));
        },
        [&] {
          auto const m = t2->isA(TCls) ? s_CLASS_CONVERSION : s_FUNC_CONVERSION;
          gen(env, RaiseNotice, cns(env, m.get()));
          return cns(env, true);
        },
        [&] { return cns(env, false); }
      );
    }
    if (!t2->type().maybe(TObj|TArr|TVec|TDict|TKeyset|
                          TInt|TStr|TDbl)) return cns(env, false);
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
  auto const handle = RDSHandleData { rds::alloc<ArrayData*>().handle() };
  auto const ptrType = RuntimeOption::EvalHackArrDVArrs
    ? TPtrToOtherDict
    : TPtrToOtherArr;
  auto const addr = gen(env, LdRDSAddr, handle, ptrType);
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
  return gen(env, LdMem, RuntimeOption::EvalHackArrDVArrs ? TDict : TArr, addr);
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
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
      return success();
    case TypeStructure::Kind::T_num:         return unionOf(TInt, TDbl);
    case TypeStructure::Kind::T_arraykey:    return unionOf(TInt, TStr);
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
      // fallthrough
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec: {
      popC(env); // pop the ts that's on the stack
      auto const c = popC(env);
      auto const res = [&]{
        if (kind == TypeStructure::Kind::T_dict) {
          return isDictImpl(env, c);
        } else if (kind == TypeStructure::Kind::T_vec) {
          return isVecImpl(env, c);
        } else if (kind == TypeStructure::Kind::T_vec_or_dict) {
          return cond(
            env,
            [&](Block* taken) {
              auto vec = isVecImpl(env, c);
              gen(env, JmpZero, taken, vec);
            },
            [&] {
              return cns(env, true);
            },
            [&] {
              return isDictImpl(env, c);
            }
          );
        } else {
          not_reached();
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
SSATmp* handleIsResolutionAndCommonOpts(
  IRGS& env,
  TypeStructResolveOp op,
  bool& done,
  bool& shouldDecRef
) {
  auto const a = topC(env);
  auto const required_ts_type = RuntimeOption::EvalHackArrDVArrs ? TDict : TArr;
  if (!a->isA(required_ts_type)) PUNT(IsTypeStructC-NotArrayTypeStruct);
  if (!a->hasConstVal(required_ts_type)) {
    if (op == TypeStructResolveOp::Resolve) {
      return resolveTypeStructImpl(env, true, true, 1, true);
    }
    shouldDecRef = false;
    return gen(env, RaiseErrorOnInvalidIsAsExpressionType, popC(env));
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
  auto const result = cns(env, maybe_resolved);
  if (op == TypeStructResolveOp::DontResolve) {
    return gen(env, RaiseErrorOnInvalidIsAsExpressionType, result);
  }
  return result;
}

} // namespace

void emitIsTypeStructC(IRGS& env, TypeStructResolveOp op) {
  auto const a = topC(env);
  auto const c = topC(env, BCSPRelOffset { 1 });
  bool done = false, shouldDecRef = true;
  SSATmp* tc = handleIsResolutionAndCommonOpts(env, op, done, shouldDecRef);
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

void emitThrowAsTypeStructException(IRGS& env) {
  auto const arr = topC(env);
  auto const c = topC(env, BCSPRelOffset { 1 });
  auto const tsAndBlock = [&]() -> std::pair<SSATmp*, Block*> {
    if (arr->hasConstVal(RuntimeOption::EvalHackArrDVArrs ? TDict : TArr)) {
      auto const ts = arr->arrLikeVal();
      auto maybe_resolved = ts;
      bool partial = true, invalidType = true;
      maybe_resolved =
        staticallyResolveTypeStructure(env, ts, partial, invalidType);
      if (!ts->equal(maybe_resolved, true)) {
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
  if (!ts->isA(RuntimeOption::EvalHackArrDVArrs ? TVec : TArr)) {
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
      [&] (SSATmp* val) { // clsmeth to varray/vec conversions
        if (RuntimeOption::EvalVecHintNotices) {
          raiseClsmethCompatTypeHint(env, id, func, tc);
        }
        if (RuntimeOption::EvalHackArrCompatTypeHintNotices) {
          if (getAnnotMetaType(tc.type()) == AnnotMetaType::DArray) {
            gen(
              env,
              RaiseHackArrParamNotice,
              RaiseHackArrParamNoticeData { tc, id, true },
              cns(env, staticEmptyVArray()),
              cns(env, func)
            );
          }
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
          ParamWithTCData { id, &tc },
          ldStkAddr(env, BCSPRelOffset { ind })
        );
      },
      [&] (SSATmp* val) { // dvarray mismatch notice
        gen(
          env,
          RaiseHackArrParamNotice,
          RaiseHackArrParamNoticeData { tc, id, true },
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
      [&] (SSATmp* valRecDesc, SSATmp* checkRec, SSATmp* val) {
        // Record/type-alias check
        gen(
          env,
          VerifyRetRecDesc,
          ParamData { id },
          valRecDesc,
          checkRec,
          cns(env, uintptr_t(&tc)),
          val
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
    for (auto const& ub : func->returnUBs()) verifyFunc(ub);
  } else if (func->hasParamsWithMultiUBs()) {
    auto const& ubs = func->paramUBs();
    auto it = ubs.find(id);
    if (it != ubs.end()) {
      for (auto const& ub : it->second) verifyFunc(ub);
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
        auto const ldPMExit = makePseudoMainExit(env);
        return ldLoc(env, id, ldPMExit, DataTypeSpecific);
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
        if (RuntimeOption::EvalHackArrCompatTypeHintNotices) {
          if (getAnnotMetaType(tc.type()) == AnnotMetaType::DArray) {
            gen(
              env,
              RaiseHackArrParamNotice,
              RaiseHackArrParamNoticeData { tc, id, false },
              cns(env, staticEmptyVArray()),
              cns(env, func)
            );
          }
        }
        auto clsMethArr = convertClsMethToVec(env, val);
        stLocRaw(env, id, fp(env), clsMethArr);
        decRef(env, val);
        return true;
      },
      [&] (Type valType, bool hard) { // Check failure
        auto const failHard = hard &&
          !(tc.isArray() && valType.maybe(TObj));
        gen(
          env,
          failHard ? VerifyParamFailHard : VerifyParamFail,
          ParamWithTCData { id, &tc }
        );
      },
      [&] (SSATmp* val) { // dvarray mismatch
        gen(
          env,
          RaiseHackArrParamNotice,
          RaiseHackArrParamNoticeData { tc, id, false },
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
      [&] (SSATmp* valRecDesc, SSATmp* checkRec, SSATmp*) {
        // Record/type-alias check
        gen(
          env,
          VerifyParamRecDesc,
          valRecDesc,
          checkRec,
          cns(env, uintptr_t(&tc)),
          cns(env, id)
        );
      },
      [] { // Giveup
        PUNT(VerifyParamType);
      }
    );
  };

  verifyFunc(func->params()[id].typeConstraint);
  if (func->hasParamsWithMultiUBs()) {
    auto const& ubs = func->paramUBs();
    auto it = ubs.find(id);
    if (it != ubs.end()) {
      for (auto const& ub : it->second) verifyFunc(ub);
    }
  }
}

}

void verifyPropType(IRGS& env,
                    SSATmp* cls,
                    const HPHP::TypeConstraint* tc,
                    Slot slot,
                    SSATmp* val,
                    SSATmp* name,
                    bool isSProp,
                    SSATmp** coerce /* = nullptr */) {
  assertx(cls->isA(TCls));
  assertx(val->isA(TCell));

  if (coerce) *coerce = val;
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
    [&] (SSATmp*) { return false; }, // No func to string automatic conversions
    [&] (SSATmp*) { return false; }, // No class to string automatic conversions
    [&] (SSATmp* val) {
      if (!coerce) return false;
      // If we're not hard enforcing property type mismatches don't coerce
      if (RO::EvalCheckPropTypeHints < 3) return false;
      if (RuntimeOption::EvalVecHintNotices) {
        if (cls->hasConstVal(TCls) && name->hasConstVal(TStr)) {
          auto const msg = makeStaticString(folly::sformat(
            "class_meth Compat: {} '{}::{}' declared as type {}, clsmeth "
            "assigned",
            isSProp ? "Static property" : "Property",
            cls->clsVal()->name()->data(),
            name->strVal()->data(),
            tc->displayName().c_str()
          ));
          gen(env, RaiseNotice, cns(env, msg));
        } else {
          gen(
            env,
            RaiseClsMethPropConvertNotice,
            RaiseClsMethPropConvertNoticeData{tc, isSProp},
            cls,
            name
          );
        }
      }
      if (RuntimeOption::EvalHackArrCompatTypeHintNotices) {
        if (getAnnotMetaType(tc->type()) == AnnotMetaType::DArray) {
          ARRPROV_USE_RUNTIME_LOCATION();
          gen(
            env,
            RaiseHackArrPropNotice,
            RaiseHackArrTypehintNoticeData { *tc },
            cls,
            cns(env, empty_varray().get()),
            cns(env, slot),
            cns(env, isSProp)
          );
        }
      }
      *coerce = convertClsMethToVec(env, val);
      return true;
    },
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
    [&] (SSATmp* val) { // dvarray mismatch
      gen(
        env,
        RaiseHackArrPropNotice,
        RaiseHackArrTypehintNoticeData { *tc },
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
    [&] (SSATmp*, SSATmp* checkRec, SSATmp* val) { // Record/type-alias check
      gen(
        env,
        VerifyPropRecDesc,
        cls,
        cns(env, slot),
        checkRec,
        val,
        cns(env, isSProp)
      );
    },
    [&] {
      // Unlike the other type-hint checks, we don't punt here. We instead do
      // the check using a runtime helper. This gives us the freedom to call
      // verifyPropType without us worrying about it punting the entire
      // operation.
      if (coerce && (tc->isArray() || (tc->isObject() && !tc->isResolved()))) {
        *coerce = gen(
          env,
          VerifyPropCoerce,
          cls,
          cns(env, slot),
          val,
          cns(env, isSProp)
        );
      } else {
        gen(env, VerifyProp, cls, cns(env, slot), val, cns(env, isSProp));
      }
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
  auto const reified = tcCouldBeReified(curFunc(env), TypeConstraint::ReturnId);
  if (reified || cell->isA(TObj)) {
    gen(env, VerifyReifiedReturnType, cell, ts);
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
  auto const ldPMExit = makePseudoMainExit(env);
  auto const cell = ldLoc(env, paramId, ldPMExit, DataTypeSpecific);
  auto const reified = tcCouldBeReified(curFunc(env), paramId);
  if (cell->isA(TObj) || reified) {
    gen(env, VerifyReifiedLocalType, ParamData { paramId }, ts);
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
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLoc(env, id, ldPMExit, DataTypeSpecific);
  if (ld->isA(TClsMeth)) {
    PUNT(IssetL_is_ClsMeth);
  }
  push(env, gen(env, IsNType, TNull, ld));
}

void emitIsUnsetL(IRGS& env, int32_t id) {
  auto const ldPMExit = makePseudoMainExit(env);
  auto const ld = ldLoc(env, id, ldPMExit, DataTypeSpecific);
  push(env, gen(env, IsType, TUninit, ld));
}

SSATmp* isTypeHelper(IRGS& env, IsTypeOp subop, SSATmp* val) {
  switch (subop) {
    case IsTypeOp::VArray: /* intentional fallthrough */
    case IsTypeOp::DArray: return isDVArrayImpl(env, val, subop);
    case IsTypeOp::PHPArr:
      return isArrayImpl(env, val, /*log_on_hack_arrays=*/false);
    case IsTypeOp::Arr:
      return isArrayImpl(env, val, /*log_on_hack_arrays=*/true);
    case IsTypeOp::Vec:    return isVecImpl(env, val);
    case IsTypeOp::Dict:   return isDictImpl(env, val);
    case IsTypeOp::Scalar: return isScalarImpl(env, val);
    case IsTypeOp::Str:    return isStrImpl(env, val);
    default: break;
  }

  // We eventually want ClsMeth to be its own DataType, so we must log.
  if (RO::EvalIsCompatibleClsMethType && subop == IsTypeOp::ArrLike) {
    if (val->isA(TClsMeth)) {
      if (RuntimeOption::EvalIsVecNotices) {
        auto const msg = makeStaticString(Strings::CLSMETH_COMPAT_IS_ANY_ARR);
        gen(env, RaiseNotice, cns(env, msg));
      }
      return cns(env, true);
    }
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
  auto const ldPMExit = makePseudoMainExit(env);
  auto const val = ldLocWarn(env, loc, ldPMExit, DataTypeSpecific);
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
