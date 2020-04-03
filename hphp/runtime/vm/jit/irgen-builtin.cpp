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
#include "hphp/runtime/vm/jit/irgen-builtin.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vm-protect.h"

#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen-ret.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/hh/ext_hh.h"

#include "hphp/util/text-util.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

struct ParamPrep {
  explicit ParamPrep(size_t count, const Func* callee) : info{count} {}

  void decRefParams(IRGS& env) const {
    if (forNativeImpl) return;
    if (ctx && ctx->type() <= TObj) {
      decRef(env, ctx);
    }
    for (auto i = size(); i--;) {
      decRef(env, info[i].value);
    }
  }

  struct Info {
    SSATmp* value{nullptr};
    bool passByAddr{false};
    bool needsConversion{false};
    bool isInOut{false};
  };

  const Info& operator[](size_t idx) const { return info[idx]; }
  Info& operator[](size_t idx) { return info[idx]; }
  size_t size() const { return info.size(); }

  // For free/class/instance methods, ctx is null/Class*/Object* respectively.
  SSATmp* ctx{nullptr};
  jit::vector<Info> info;
  uint32_t numByAddr{0};

  bool forNativeImpl{false};
};

//////////////////////////////////////////////////////////////////////

// Will turn into either an int or a double in zend_convert_scalar_to_number.
bool type_converts_to_number(Type ty) {
  return ty.subtypeOfAny(
    TDbl,
    TInt,
    TNull,
    TObj,
    TRes,
    TStr,
    TBool
  );
}

//////////////////////////////////////////////////////////////////////

Block* make_opt_catch(IRGS& env, const ParamPrep& params) {
  // The params have been popped and if we're inlining the ActRec is gone
  env.irb->setCurMarker(makeMarker(env, nextBcOff(env)));
  env.irb->exceptionStackBoundary();

  assertx(!env.irb->fs().stublogue());
  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, nextBcOff(env)), exit);
  gen(env, BeginCatch);
  params.decRefParams(env);
  auto const data = EndCatchData {
    spOffBCFromIRSP(env),
    EndCatchData::CatchMode::UnwindOnly,
    EndCatchData::FrameMode::Phplogue,
    EndCatchData::Teardown::Full
  };
  gen(env, EndCatch, data, fp(env), sp(env));
  return exit;
}

SSATmp* is_a_impl(IRGS& env, const ParamPrep& params, bool subclassOnly) {
  if (params.size() != 3) return nullptr;

  auto const allowString = params[2].value;
  auto const classname   = params[1].value;
  auto const obj         = params[0].value;

  if (!obj->isA(TObj) ||
      !classname->hasConstVal(TStr) ||
      !allowString->isA(TBool)) {
    return nullptr;
  }

  auto const objCls = gen(env, LdObjClass, obj);

  auto const cls = lookupUniqueClass(env, classname->strVal());
  if (!cls) return nullptr;

  auto const testCls = cns(env, cls);

  // is_a() finishes here.
  if (!subclassOnly) return gen(env, InstanceOf, objCls, testCls);

  // is_subclass_of() needs to check that the LHS doesn't have the same class as
  // as the RHS.
  return cond(
    env,
    [&] (Block* taken) {
      auto const eq = gen(env, EqCls, objCls, testCls);
      gen(env, JmpNZero, taken, eq);
    },
    [&] {
      return gen(env, InstanceOf, objCls, testCls);
    },
    [&] {
      return cns(env, false);
    }
  );
}

SSATmp* opt_is_a(IRGS& env, const ParamPrep& params) {
  return is_a_impl(env, params, false /* subclassOnly */);
}

SSATmp* opt_is_subclass_of(IRGS& env, const ParamPrep& params) {
  return is_a_impl(env, params, true /* subclassOnly */);
}

SSATmp* opt_method_exists(IRGS& env, const ParamPrep& params) {
  if (params.size() != 2) return nullptr;

  auto const meth = params[1].value;
  auto const obj  = params[0].value;

  if (!obj->isA(TObj) || !meth->isA(TStr)) return nullptr;

  auto const cls = gen(env, LdObjClass, obj);
  return gen(env, MethodExists, cls, meth);
}

const StaticString
  s_conv_clsmeth_to_varray("Implicit clsmeth to varray conversion"),
  s_conv_clsmeth_to_vec("Implicit clsmeth to vec conversion");

void raiseClsMethToVecWarningHelper(IRGS& env, const ParamPrep& params) {
  if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
    gen(
      env,
      RaiseNotice,
      make_opt_catch(env, params),
      cns(env, RuntimeOption::EvalHackArrDVArrs ?
        s_conv_clsmeth_to_vec.get() : s_conv_clsmeth_to_varray.get())
    );
  }
}

SSATmp* opt_count(IRGS& env, const ParamPrep& params) {
  if (params.size() != 2) return nullptr;

  auto const mode = params[1].value;
  auto const val = params[0].value;

  if (val->isA(TClsMeth)) {
    raiseClsMethToVecWarningHelper(env, params);
    return cns(env, 2);
  }

  // Bail if we're trying to do a recursive count()
  if (!mode->hasConstVal(0)) return nullptr;

  // Count may throw
  return gen(env, Count, make_opt_catch(env, params), val);
}

SSATmp* opt_sizeof(IRGS& env, const ParamPrep& params) {
  return opt_count(env, params);
}

SSATmp* opt_ord(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const arg = params[0].value;
  auto const arg_type = arg->type();
  if (arg_type <= TStr) {
    return gen(env, OrdStr, arg);
  }

  return nullptr;
}

SSATmp* opt_chr(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const arg = params[0].value;
  auto const arg_type = arg->type();
  if (arg_type <= TInt) {
    return gen(env, ChrInt, arg);
  }

  return nullptr;
}

SSATmp* opt_ini_get(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  auto const argType = params[0].value->type();
  if (!(argType.hasConstVal(TStaticStr))) {
    return nullptr;
  }

  // We can only optimize settings that are system wide since user level
  // settings can be overridden during the execution of a request.
  //
  // TODO: the above is true for settings whose value we burn directly into the
  // TC, but for non-system settings, we can optimize them as a load from the
  // known static address or thread-local address of where the setting lives.
  // This might be worth doing specifically for the zend.assertions setting,
  // for which the emitter emits an ini_get around every call to assertx().
  auto const settingName = params[0].value->strVal()->toCppString();
  IniSetting::Mode mode = IniSetting::PHP_INI_NONE;
  if (!IniSetting::GetMode(settingName, mode)) {
    return nullptr;
  }
  if (mode & ~IniSetting::PHP_INI_SYSTEM) {
    return nullptr;
  }
  if (mode == IniSetting::PHP_INI_ALL) {  /* PHP_INI_ALL has a weird encoding */
    return nullptr;
  }

  Variant value;
  IniSetting::Get(settingName, value);
  // All scalar values are cast to a string before being returned.
  if (value.isString()) {
    return cns(env, makeStaticString(value.toString()));
  }
  if (value.isInteger()) {
    return cns(env, makeStaticString(folly::to<std::string>(value.toInt64())));
  }
  if (value.isBoolean()) {
    static auto const s_one = makeStaticString("1");
    return cns(env, value.toBoolean() ? s_one : staticEmptyString());
  }
  // ini_get() is now enhanced to return more than strings.
  // Get out of here if we are something else like an array.
  return nullptr;
}

/*
 * Transforms in_array with a static haystack argument into an AKExistsKeyset.
 */
SSATmp* opt_in_array(IRGS& env, const ParamPrep& params) {
  if (params.size() != 3 && params.size() != 2) return nullptr;

  // We will restrict this optimization to needles that are strings, and
  // haystacks that have only non-numeric string keys. This avoids a bunch of
  // complication around numeric-string array-index semantics.
  auto const needle = params[0].value;
  if (!(needle->type() <= TStr)) {
    return nullptr;
  }

  auto const haystackType = params[1].value->type();
  if (!haystackType.hasConstVal(TStaticArr)) {
    // Haystack isn't statically known
    return nullptr;
  }

  auto const haystack = haystackType.arrVal();
  if (haystack->size() == 0) {
    return cns(env, false);
  }

  KeysetInit flipped{haystack->size()};
  bool failed{false};
  IterateVNoInc(
    haystack,
    [&](TypedValue key) {

      if (!isStringType(type(key)) || val(key).pstr->isNumeric()) {
        // Numeric strings will complicate matters because the loose comparisons
        // done with array keys are not quite the same as loose comparisons done
        // by in_array. For example: in_array('0', array('0000')) is true, but
        // doing array('0000' => true)['0'] will say "undefined index".
        // This seems unlikely to affect real-world usage.
        failed = true;
        return true;
      }

      flipped.add(val(key).pstr);
      return false;
    }
  );
  if (failed) {
    return nullptr;
  }

  return gen(
    env,
    AKExistsKeyset,
    cns(env, ArrayData::GetScalarArray(flipped.toArray())),
    needle
  );
}

SSATmp* opt_get_class(IRGS& env, const ParamPrep& params) {
  auto const curCls = !params.forNativeImpl ? curClass(env) : nullptr;
  auto const curName = [&] {
    return curCls != nullptr ? cns(env, curCls->name()) : nullptr;
  };
  if (params.size() == 0 && RuntimeOption::EvalGetClassBadArgument == 0) {
    return curName();
  }
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;
  auto const ty  = val->type();
  if (ty <= TNull && RuntimeOption::EvalGetClassBadArgument == 0) {
    return curName();
  }
  if (ty <= TObj) {
    auto const cls = gen(env, LdObjClass, val);
    return gen(env, LdClsName, cls);
  }

  return nullptr;
}

SSATmp* opt_sqrt(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;
  auto const ty  = val->type();
  if (ty <= TDbl) return gen(env, Sqrt, val);
  return nullptr;
}

SSATmp* opt_strlen(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;
  auto const ty  = val->type();

  if (ty <= TStr) {
    return gen(env, LdStrLen, val);
  }

  return nullptr;
}

SSATmp* opt_clock_gettime_ns(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;

  // CLOCK_THREAD_CPUTIME_ID needs special handling
  if (val->hasConstVal(TInt) && val->intVal() != CLOCK_THREAD_CPUTIME_ID) {
    return gen(env, GetTimeNs, val);
  }

  return nullptr;
}

SSATmp* opt_microtime(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;

  if (val->hasConstVal(true)) {
    return gen(env, GetTime);
  }

  return nullptr;
}

SSATmp* minmax(IRGS& env, const ParamPrep& params, const bool is_max) {
  auto const val1 = params[1].value;
  auto const ty1 = val1->type();
  auto const val2 = params[0].value;
  auto const ty2 = val2->type();

  // this optimization is only for 2 ints/doubles
  if (!(ty1 <= TInt || ty1 <= TDbl) ||
      !(ty2 <= TInt || ty2 <= TDbl)) return nullptr;

  auto const cmp = [&]{
    if (ty1 <= TInt && ty2 <= TInt) {
      return gen(env, is_max ? GtInt : LtInt, val1, val2);
    } else {
      auto conv1 = (ty1 <= TDbl) ? val1 : gen(env, ConvIntToDbl, val1);
      auto conv2 = (ty2 <= TDbl) ? val2 : gen(env, ConvIntToDbl, val2);
      return gen(env, is_max ? GtDbl : LtDbl, conv1, conv2);
    }
  }();
  return gen(env, Select, cmp, val1, val2);
}

SSATmp* opt_max2(IRGS& env, const ParamPrep& params) {
  // max2 is only called for 2 operands
  return params.size() == 2 ? minmax(env, params, true) : nullptr;
}

SSATmp* opt_min2(IRGS& env, const ParamPrep& params) {
  // min2 is only called for 2 operands
  return params.size() == 2 ? minmax(env, params, false) : nullptr;
}

SSATmp* opt_ceil(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = params[0].value;
  if (!type_converts_to_number(val->type())) return nullptr;
  // May throw
  auto const dbl = gen(env, ConvTVToDbl, make_opt_catch(env, params), val);
  return gen(env, Ceil, dbl);
}

SSATmp* opt_floor(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = params[0].value;
  if (!type_converts_to_number(val->type())) return nullptr;
  // May throw
  auto const dbl = gen(env, ConvTVToDbl, make_opt_catch(env, params), val);
  return gen(env, Floor, dbl);
}

SSATmp* opt_abs(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const value = params[0].value;
  if (value->type() <= TInt) {
    // compute integer absolute value ((src>>63) ^ src) - (src>>63)
    auto const t1 = gen(env, Shr, value, cns(env, 63));
    auto const t2 = gen(env, XorInt, t1, value);
    return gen(env, SubInt, t2, t1);
  }

  if (value->type() <= TDbl) return gen(env, AbsDbl, value);
  if (value->type() <= TArrLike) return cns(env, false);

  return nullptr;
}

SSATmp* opt_array_key_cast(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;

  if (value->isA(TInt))  return value;
  if (value->isA(TNull)) return cns(env, staticEmptyString());
  if (value->isA(TBool)) return gen(env, ConvBoolToInt, value);
  if (value->isA(TDbl))  return gen(env, ConvDblToInt, value);
  if (value->isA(TRes))  return gen(env, ConvResToInt, value);
  if (value->isA(TStr))  return gen(env, StrictlyIntegerConv, value);

  return nullptr;
}

SSATmp* impl_opt_type_structure(IRGS& env, const ParamPrep& params,
                                bool getName) {
  if (params.size() != 2) return nullptr;
  auto const clsNameTmp = params[0].value;
  auto const cnsNameTmp = params[1].value;

  if (!clsNameTmp->isA(TStr)) return nullptr;
  if (!cnsNameTmp->hasConstVal(TStaticStr)) return nullptr;
  auto const cnsName = cnsNameTmp->strVal();

  auto const clsTmp = [&] () -> SSATmp* {
    if (clsNameTmp->inst()->is(LdClsName)) {
      return clsNameTmp->inst()->src(0);
    }
    return ldCls(env, clsNameTmp, make_opt_catch(env, params));
  }();

  if (!clsTmp->type().clsSpec()) return nullptr;
  auto const cls = clsTmp->type().clsSpec().cls();

  auto const cnsSlot = cls->clsCnsSlot(cnsName, true, true);
  if (cnsSlot == kInvalidSlot) return nullptr;

  auto const data = LdSubClsCnsData { cnsName, cnsSlot };
  if (!getName) {
    auto const ptr = gen(env, LdSubClsCns, data, clsTmp);
    return cond(
      env,
      [&] (Block* taken) {
        gen(env, CheckTypeMem, TUncountedInit, taken, ptr);
        return gen(env, LdTypeCns, taken, gen(env, LdMem, TUncountedInit, ptr));
      },
      [&] (SSATmp* cns) { return cns; },
      [&] /* taken */ {
        return gen(
          env, LdClsTypeCns, make_opt_catch(env, params), clsTmp, cnsNameTmp
        );
      }
    );
  }
  return cond(
    env,
    [&] (Block* taken) {
      auto const clsNameFromTS = gen(env, LdSubClsCnsClsName, data, clsTmp);
      return gen(env, CheckNonNull, taken, clsNameFromTS);
    },
    [&] (SSATmp* s) { return s; },
    [&] {
      return gen(
        env,
        LdClsTypeCnsClsName,
        make_opt_catch(env, params),
        clsTmp,
        cnsNameTmp
      );
    }
  );
}

SSATmp* opt_type_structure(IRGS& env, const ParamPrep& params) {
  return impl_opt_type_structure(env, params, false);
}
SSATmp* opt_type_structure_classname(IRGS& env, const ParamPrep& params) {
  return impl_opt_type_structure(env, params, true);
}

SSATmp* opt_is_list_like(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const type = params[0].value->type();
  // Type might be a Ptr here, so the maybe() below will go wrong if we don't
  // bail out here.
  if (!(type <= TInitCell)) return nullptr;
  if (type <= TClsMeth) {
    raiseClsMethToVecWarningHelper(env, params);
    return cns(env, true);
  }
  if (!type.maybe(TArrLike)) return cns(env, false);
  if (type.subtypeOfAny(TVec, TPackedArr)) return cns(env, true);
  return nullptr;
}

SSATmp* opt_foldable(IRGS& env,
                     const Func* func,
                     const ParamPrep& params,
                     uint32_t numNonDefaultArgs) {
  ARRPROV_USE_RUNTIME_LOCATION();
  if (!func->isFoldable()) return nullptr;

  const Class* cls = nullptr;
  if (func->isMethod()) {
    if (!params.ctx || !func->isStatic()) return nullptr;
    cls = params.ctx->type().clsSpec().exactCls();
    if (!cls) return nullptr;
  }

  ArrayData* variadicArgs = nullptr;
  uint32_t numVariadicArgs = 0;
  if (numNonDefaultArgs > func->numNonVariadicParams()) {
    assertx(params.size() == func->numParams());
    auto const variadic = params.info.back().value;
    auto const ty = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
    if (!variadic->type().hasConstVal(ty)) return nullptr;

    variadicArgs = variadic->variantVal().asCArrRef().get();
    numVariadicArgs = variadicArgs->size();

    if (numVariadicArgs && !variadicArgs->isVecOrVArray()) return nullptr;

    assertx(variadicArgs->isStatic());
    numNonDefaultArgs = func->numNonVariadicParams();
  }

  // Don't pop the args yet---if the builtin throws at compile time (because
  // it would raise a warning or something at runtime) we're going to leave
  // the call alone.
  VArrayInit args(numNonDefaultArgs + numVariadicArgs);
  for (auto i = 0; i < numNonDefaultArgs; ++i) {
    auto const t = params[i].value->type();
    if (!t.hasConstVal() && !t.subtypeOfAny(TUninit, TInitNull, TNullptr)) {
      return nullptr;
    } else {
      args.append(params[i].value->variantVal());
    }
  }
  if (variadicArgs) {
    for (auto i = 0; i < numVariadicArgs; i++) {
      args.append(variadicArgs->get(i).tv());
    }
  }

  try {
    // We don't know if notices would be enabled or not when this function
    // would normally get called, so be safe and don't optimize any calls that
    // COULD generate notices.
    ThrowAllErrorsSetter taes;

    VMProtect::Pause deprot;
    always_assert(tl_regState == VMRegState::CLEAN);

    // Even though tl_regState is marked clean, vmpc() has not necessarily been
    // set to anything valid, so we need to do so here (for assertions and
    // backtraces in the invocation, among other things).
    auto const savedPC = vmpc();
    vmpc() = vmfp() ? vmfp()->m_func->getEntry() : nullptr;
    SCOPE_EXIT{ vmpc() = savedPC; };

    assertx(!RID().getJitFolding());
    RID().setJitFolding(true);
    SCOPE_EXIT{ RID().setJitFolding(false); };

    auto retVal = g_context->invokeFunc(func, args.toArray(),
                                        nullptr, const_cast<Class*>(cls),
                                        nullptr, false);
    SCOPE_EXIT { tvDecRefGen(retVal); };
    assertx(tvIsPlausible(retVal));

    auto scalar_array = [&] {
      return ArrayData::GetScalarArray(std::move(tvAsVariant(&retVal)));
    };

    switch (retVal.m_type) {
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
        return cns(env, retVal);
      case KindOfPersistentString:
      case KindOfString:
        return cns(env, makeStaticString(retVal.m_data.pstr));
      case KindOfPersistentVec:
      case KindOfVec:
        return cns(
          env,
          make_tv<KindOfPersistentVec>(scalar_array())
        );
      case KindOfPersistentDict:
      case KindOfDict:
        return cns(
          env,
          make_tv<KindOfPersistentDict>(scalar_array())
        );
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        return cns(
          env,
          make_tv<KindOfPersistentKeyset>(scalar_array())
        );
      case KindOfPersistentDArray:
      case KindOfDArray:
      case KindOfPersistentVArray:
      case KindOfVArray:
      case KindOfPersistentArray:
      case KindOfArray:
        return cns(
          env,
          make_persistent_array_like_tv(scalar_array())
        );
      case KindOfUninit:
      case KindOfObject:
      case KindOfResource:
      // TODO (T29639296)
      case KindOfFunc:
      case KindOfClass:
      case KindOfClsMeth:
      case KindOfRecord: // TODO(arnabde)
        return nullptr;
    }
  } catch (...) {
    // If an exception or notice occurred, don't optimize
  }
  return nullptr;
}

/*
* Container intrinsic for HH\traversable
*/
SSATmp* opt_container_first(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) {
    return nullptr;
  }
  auto const value = params[0].value;
  auto const type = value->type();
  if (type.subtypeOfAny(TVec, TPackedArr)) {
    auto const r = gen(env, VecFirst, value);
    gen(env, IncRef, r);
    return r;
  } else if (type.subtypeOfAny(TDict, TMixedArr)) {
    auto const r = gen(env, DictFirst, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TKeyset) {
    auto const r = gen(env, KeysetFirst, value);
    gen(env, IncRef, r);
    return r;
  }
  return nullptr;
}

SSATmp* opt_container_last(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) {
    return nullptr;
  }
  auto const value = params[0].value;
  auto const type = value->type();
  if (type.subtypeOfAny(TVec, TPackedArr)) {
    auto const r = gen(env, VecLast, value);
    gen(env, IncRef, r);
    return r;
  } else if (type.subtypeOfAny(TDict, TMixedArr)) {
    auto const r = gen(env, DictLast, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TKeyset) {
    auto const r = gen(env, KeysetLast, value);
    gen(env, IncRef, r);
    return r;
  }
  return nullptr;
}

SSATmp* opt_container_first_key(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) {
    return nullptr;
  }
  auto const value = params[0].value;
  auto const type = value->type();

  if (type.subtypeOfAny(TVec, TPackedArr)) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = type <= TVec ?
          gen(env, CountVec, value) : gen(env, CountArray, value);
        gen(env, JmpZero, taken, length);
      },
      [&] { return cns(env, 0); },
      [&] { return cns(env, TInitNull); }
    );
  } else if (type.subtypeOfAny(TDict, TMixedArr)) {
    auto const r = gen(env, DictFirstKey, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TKeyset) {
    auto const r = gen(env, KeysetFirst, value);
    gen(env, IncRef, r);
    return r;
  }
  return nullptr;
}

SSATmp* opt_container_last_key(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) {
    return nullptr;
  }
  auto const value = params[0].value;
  auto const type = value->type();

  if (type.subtypeOfAny(TVec, TPackedArr)) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = type <= TVec ?
          gen(env, CountVec, value) : gen(env, CountArray, value);
        gen(env, JmpZero, taken, length);
        return length;
      },
      [&] (SSATmp* next) { return gen(env, SubInt, next, cns(env, 1)); },
      [&] { return cns(env, TInitNull); }
    );
  } else if (type.subtypeOfAny(TDict, TMixedArr)) {
    auto const r = gen(env, DictLastKey, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TKeyset) {
    auto const r = gen(env, KeysetLast, value);
    gen(env, IncRef, r);
    return r;
  }
  return nullptr;
}

namespace {
const StaticString
  s_MCHELPER_ON_GET_CLS("MethCallerHelper is used on meth_caller_get_class()"),
  s_MCHELPER_ON_GET_METH(
    "MethCallerHelper is used on meth_caller_get_method()"),
  s_BAD_ARG_ON_MC_GET_CLS(
    "Argument 1 passed to meth_caller_get_class() must be a MethCaller"),
  s_BAD_ARG_ON_MC_GET_METH(
    "Argument 1 passed to meth_caller_get_method() must be a MethCaller"),
  s_meth_caller_cls("__SystemLib\\MethCallerHelper"),
  s_cls_prop("class"),
  s_meth_prop("method");
const Slot s_cls_idx{0};
const Slot s_meth_idx{1};

SSATmp* opt_fun_get_function(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  auto const type = value->type();
  if (type <= TFunc) {
    return gen(env, LdFuncName, value);
  }
  return nullptr;
}

DEBUG_ONLY bool meth_caller_has_expected_prop(const Class *mcCls) {
  return mcCls->lookupDeclProp(s_cls_prop.get()) == s_cls_idx &&
        mcCls->lookupDeclProp(s_meth_prop.get()) == s_meth_idx &&
        mcCls->declPropTypeConstraint(s_cls_idx).isString() &&
        mcCls->declPropTypeConstraint(s_meth_idx).isString();
}

template<bool isCls>
SSATmp* meth_caller_get_name(IRGS& env, SSATmp *value) {
  if (value->isA(TFunc)) {
    return cond(
        env,
        [&] (Block* taken) {
          auto const attr = AttrData {static_cast<int32_t>(AttrIsMethCaller)};
          auto isMC = gen(env, FuncHasAttr, attr, value);
          gen(env, JmpZero, taken, isMC);
        },
        [&] {
          return gen(env, LdMethCallerName, MethCallerData{isCls}, value);
        },
        [&] { // Taken: src is not a meth_caller
          hint(env, Block::Hint::Unlikely);
          updateMarker(env);
          env.irb->exceptionStackBoundary();
          gen(env, RaiseError, cns(env, isCls ?
            s_BAD_ARG_ON_MC_GET_CLS.get() : s_BAD_ARG_ON_MC_GET_METH.get()));
          // Dead-code, but needed to satisfy cond().
          return cns(env, staticEmptyString());
        }
      );
  }
  if (value->isA(TObj)) {
    auto loadProp = [&] (Class* cls, bool isGetCls, SSATmp* obj) {
      auto const slot = isGetCls ? s_cls_idx : s_meth_idx;
      auto const idx = cls->propSlotToIndex(slot);
      auto const prop = gen(
        env, LdPropAddr, IndexData{idx}, TStr.lval(Ptr::Prop), obj);
      auto const ret = gen(env, LdMem, TStr, prop);
      gen(env, IncRef, ret);
      return ret;
    };

    auto const mcCls = Unit::lookupClass(s_meth_caller_cls.get());
    assertx(mcCls && meth_caller_has_expected_prop(mcCls));
    return cond(
      env,
      [&] (Block* taken) {
        auto isMC = gen(
          env, EqCls, cns(env, mcCls), gen(env, LdObjClass, value));
        gen(env, JmpZero, taken, isMC);
      },
      [&] {
        if (RuntimeOption::EvalEmitMethCallerFuncPointers &&
            RuntimeOption::EvalNoticeOnMethCallerHelperUse) {
          updateMarker(env);
          env.irb->exceptionStackBoundary();
          auto const msg = cns(env, isCls ?
            s_MCHELPER_ON_GET_CLS.get() : s_MCHELPER_ON_GET_METH.get());
          gen(env, RaiseNotice, msg);
        }
        return loadProp(mcCls, isCls, value);
      },
      [&] { // Taken: src is not a meth_caller
        hint(env, Block::Hint::Unlikely);
        updateMarker(env);
        env.irb->exceptionStackBoundary();
        gen(env, RaiseError, cns(env, isCls ?
          s_BAD_ARG_ON_MC_GET_CLS.get() : s_BAD_ARG_ON_MC_GET_METH.get()));
        // Dead-code, but needed to satisfy cond().
        return cns(env, staticEmptyString());
      }
    );
  }
  return nullptr;
}
}

SSATmp* opt_class_meth_get_class(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->type() <= TClsMeth) {
    return gen(env, LdClsName, gen(env, LdClsFromClsMeth, value));
  }
  return nullptr;
}

SSATmp* opt_class_meth_get_method(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->type() <= TClsMeth) {
    return gen(env, LdFuncName, gen(env, LdFuncFromClsMeth, value));
  }
  return nullptr;
}

SSATmp* opt_shapes_idx(IRGS& env, const ParamPrep& params) {
  // We first check the number and types of each argument. If any check fails,
  // we'll fall back to the native code which will raise an appropriate error.
  auto const nparams = params.size();
  if (nparams != 2 && nparams != 3) return nullptr;

  // params[0] is a darray, which may be a Dict or an Arr based on options.
  // If the Hack typehint check flag is on, then we fall back to the native
  // implementation of this method, which checks the DVArray bit in ArrayData.
  bool is_dict;
  auto const arrType = params[0].value->type();
  if (RuntimeOption::EvalHackArrDVArrs && arrType <= TDict) {
    is_dict = true;
  } else if (!RuntimeOption::EvalHackArrDVArrs && arrType <= TArr) {
    is_dict = false;
  } else {
    return nullptr;
  }

  // params[1] is an arraykey. We only optimize if it's narrowed to int or str.
  auto const keyType = params[1].value->type();
  if (!(keyType <= TInt || keyType <= TStr)) return nullptr;

  // params[2] is an optional argument. If it's uninit, we convert it to null.
  // We only optimize if we can distinguish between uninit and other types.
  auto const defType = nparams == 3 ? params[2].value->type() : TUninit;
  if (!(defType <= TUninit) && defType.maybe(TUninit)) return nullptr;
  auto const def = defType <= TUninit ? cns(env, TInitNull) : params[2].value;

  // params[0] is the array, for which we may need to do a dvarray check.
  auto const arr = [&]() -> SSATmp* {
    auto const val = params[0].value;
    if (!(RO::EvalHackArrCompatTypeHintNotices && val->isA(TArr))) return val;

    // Rather than just emitting a notice here, we interp and side-exit for
    // non-darrays so that we can use layout information in the main trace.
    //
    // To interp, we must restore the stack offset from the start of the HHBC.
    // It's easy to undo the stack offset for FCallBuiltin, but it's too hard
    // to do the same for inlined NativeImpls (since they end inlined traces).
    if (curSrcKey(env).op() != Op::FCallBuiltin) return nullptr;
    env.irb->fs().incBCSPDepth(nparams);
    auto const result = gen(env, CheckDArray, makeExitSlow(env), val);
    env.irb->fs().decBCSPDepth(nparams);
    return result;
  }();
  if (!arr) return nullptr;

  // Do the array access, using array offset profiling to optimize it.
  auto const key = params[1].value;
  auto const elm = profiledArrayAccess(env, arr, key,
    [&] (SSATmp* arr, SSATmp* key, uint32_t pos) {
      auto const op = is_dict ? DictGetK : MixedArrayGetK;
      return gen(env, op, IndexData { pos }, arr, key);
    },
    [&] (SSATmp* key, SizeHintData data) {
      auto const op = is_dict ? DictIdx : ArrayIdx;
      return gen(env, op, data, arr, key, def);
    }
  );

  auto const finish = [&](SSATmp* val){
    gen(env, IncRef, val);
    return val;
  };
  return finish(profiledType(env, elm, [&] {
    auto const cell = finish(elm);
    params.decRefParams(env);
    push(env, cell);
  }));
}

const EnumValues* getEnumValues(IRGS& env, const ParamPrep& params) {
  if (RO::EvalArrayProvenance) return nullptr;
  if (!(params.ctx && params.ctx->hasConstVal(TCls))) return nullptr;
  auto const cls = params.ctx->clsVal();
  if (!(isEnum(cls) && classHasPersistentRDS(cls))) return nullptr;
  return EnumCache::getValuesStatic(cls);
}

SSATmp* opt_enum_names(IRGS& env, const ParamPrep& params) {
  if (params.size() != 0) return nullptr;
  auto const enum_values = getEnumValues(env, params);
  return enum_values ? cns(env, enum_values->names.get()) : nullptr;
}

SSATmp* opt_enum_values(IRGS& env, const ParamPrep& params) {
  if (params.size() != 0) return nullptr;
  auto const enum_values = getEnumValues(env, params);
  return enum_values ? cns(env, enum_values->values.get()) : nullptr;
}

SSATmp* opt_enum_is_valid(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (!value->type().isKnownDataType()) return nullptr;
  auto const enum_values = getEnumValues(env, params);
  if (!enum_values) return nullptr;
  auto const ad = MixedArray::asMixed(enum_values->names.get());
  auto const op = ad->isDictType() ? AKExistsDict : AKExistsArr;
  if (value->isA(TInt)) {
    if (ad->keyTypes().mustBeStrs()) return cns(env, false);
    return gen(env, op, cns(env, ad->asArrayData()), value);
  } else if (value->isA(TStr)) {
    // We're not doing intish-casts here, so we bail if ad has any int keys.
    if (!ad->keyTypes().mustBeStrs()) return nullptr;
    return gen(env, op, cns(env, ad->asArrayData()), value);
  }
  return cns(env, false);
}

SSATmp* opt_enum_coerce(IRGS& env, const ParamPrep& params) {
  auto const valid = opt_enum_is_valid(env, params);
  if (!valid) return nullptr;
  return cond(env,
    [&](Block* taken) { gen(env, JmpZero, taken, valid); },
    [&]{
      // We never need to coerce strs to ints here, but we may need to coerce
      // ints to strs if the enum is a string type with intish values.
      auto const value = params[0].value;
      auto const isstr = isStringType(params.ctx->clsVal()->enumBaseTy());
      if (value->isA(TInt) && isstr) return gen(env, ConvIntToStr, value);
      gen(env, IncRef, value);
      return value;
    },
    [&]{ return cns(env, TInitNull); }
  );
}

SSATmp* opt_tag_provenance_here(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  if (RO::EvalLogArrayProvenance) return nullptr;
  auto const result = params[0].value;
  gen(env, IncRef, result);
  return result;
}

StaticString s_ARRAY_MARK_LEGACY_VEC(Strings::ARRAY_MARK_LEGACY_VEC);
StaticString s_ARRAY_MARK_LEGACY_DICT(Strings::ARRAY_MARK_LEGACY_DICT);

SSATmp* opt_array_mark_legacy(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (!RO::EvalHackArrDVArrs) {
    if (value->isA(TVec)) {
      gen(env,
          RaiseWarning,
          make_opt_catch(env, params),
          cns(env, s_ARRAY_MARK_LEGACY_VEC.get()));
    } else if (value->isA(TDict)) {
      gen(env,
          RaiseWarning,
          make_opt_catch(env, params),
          cns(env, s_ARRAY_MARK_LEGACY_DICT.get()));
    }
  }
  if (value->isA(TVec)) {
    return gen(env, SetLegacyVec, value);
  } else if (value->isA(TDict)) {
    return gen(env, SetLegacyDict, value);
  }
  return nullptr;
}

SSATmp* opt_is_meth_caller(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->isA(TFunc)) {
    return gen(
      env,
      FuncHasAttr,
      AttrData {static_cast<int32_t>(AttrIsMethCaller)},
      value);
  }
  if (value->isA(TObj)) {
    auto const mcCls = Unit::lookupClass(s_meth_caller_cls.get());
    assertx(mcCls);
    return gen(env, EqCls, cns(env, mcCls), gen(env, LdObjClass, value));
  }
  return nullptr;
}

SSATmp* opt_meth_caller_get_class(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  return meth_caller_get_name<true>(env, params[0].value);
}

SSATmp* opt_meth_caller_get_method(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  return meth_caller_get_name<false>(env, params[0].value);
}

//////////////////////////////////////////////////////////////////////

// Whitelists of builtins that we have optimized HHIR emitters for.
// The first whitelist here simply lets us look up the functions above.

using OptEmitFn = SSATmp* (*)(IRGS& env, const ParamPrep& params);

const hphp_fast_string_imap<OptEmitFn> s_opt_emit_fns{
  {"is_a", opt_is_a},
  {"is_subclass_of", opt_is_subclass_of},
  {"method_exists", opt_method_exists},
  {"count", opt_count},
  {"sizeof", opt_sizeof},
  {"ini_get", opt_ini_get},
  {"in_array", opt_in_array},
  {"get_class", opt_get_class},
  {"sqrt", opt_sqrt},
  {"strlen", opt_strlen},
  {"clock_gettime_ns", opt_clock_gettime_ns},
  {"microtime", opt_microtime},
  {"__SystemLib\\max2", opt_max2},
  {"__SystemLib\\min2", opt_min2},
  {"ceil", opt_ceil},
  {"floor", opt_floor},
  {"abs", opt_abs},
  {"ord", opt_ord},
  {"chr", opt_chr},
  {"hh\\array_key_cast", opt_array_key_cast},
  {"hh\\type_structure", opt_type_structure},
  {"hh\\type_structure_classname", opt_type_structure_classname},
  {"hh\\is_list_like", opt_is_list_like},
  {"HH\\Lib\\_Private\\Native\\first", opt_container_first},
  {"HH\\Lib\\_Private\\Native\\last", opt_container_last},
  {"HH\\Lib\\_Private\\Native\\first_key", opt_container_first_key},
  {"HH\\Lib\\_Private\\Native\\last_key", opt_container_last_key},
  {"HH\\fun_get_function", opt_fun_get_function},
  {"HH\\class_meth_get_class", opt_class_meth_get_class},
  {"HH\\class_meth_get_method", opt_class_meth_get_method},
  {"HH\\Shapes::idx", opt_shapes_idx},
  {"HH\\BuiltinEnum::getNames", opt_enum_names},
  {"HH\\BuiltinEnum::getValues", opt_enum_values},
  {"HH\\BuiltinEnum::coerce", opt_enum_coerce},
  {"HH\\BuiltinEnum::isValid", opt_enum_is_valid},
  {"HH\\is_meth_caller", opt_is_meth_caller},
  {"HH\\tag_provenance_here", opt_tag_provenance_here},
  {"HH\\array_mark_legacy", opt_array_mark_legacy},
  {"HH\\meth_caller_get_class", opt_meth_caller_get_class},
  {"HH\\meth_caller_get_method", opt_meth_caller_get_method},
};

// This second whitelist, a subset of the first, records which parameter
// (if any) we need a vanilla input for to generate optimized HHIR.

const hphp_fast_string_imap<int> s_vanilla_params{
  {"count", 0},
  {"sizeof", 0},
  {"HH\\Shapes::idx", 0},
  {"HH\\Lib\\_Private\\Native\\first", 0},
  {"HH\\Lib\\_Private\\Native\\last", 0},
  {"HH\\Lib\\_Private\\Native\\first_key", 0},
  {"HH\\Lib\\_Private\\Native\\last_key", 0},
};

//////////////////////////////////////////////////////////////////////

SSATmp* optimizedFCallBuiltin(IRGS& env,
                              const Func* func,
                              const ParamPrep& params,
                              uint32_t numNonDefault) {
  auto const result = [&]() -> SSATmp* {

    auto const fname = func->fullName();

    if (auto const retVal = opt_foldable(env, func, params, numNonDefault)) {
      // Check if any of the parameters are in-out. If not, we don't
      // need any special handling.
      auto const numInOut = std::count_if(
        params.info.begin(), params.info.end(),
        [] (const ParamPrep::Info& i) { return i.isInOut; }
      );
      if (!numInOut) return retVal;

      // Otherwise, the return value is actually a tuple containing
      // all of the results. We need to unpack the tuple and write the
      // contents to their proper place on the stack.
      auto const ad = retVal->arrLikeVal();
      assertx(ad->isStatic());
      assertx(ad->hasVanillaPackedLayout());
      assertx(ad->size() == numInOut + 1);

      size_t inOutIndex = 0;
      for (auto const& param : params.info) {
        if (!param.isInOut) continue;
        // NB: The parameters to the builtin have already been popped
        // at this point, so we don't need to account for them when
        // calculating the stack offset.
        auto const val = cns(env, ad->nvGetVal(inOutIndex + 1));
        auto const offset = offsetFromIRSP(
          env,
          BCSPRelOffset{safe_cast<int32_t>(inOutIndex)}
        );
        gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), val);
        ++inOutIndex;
      }

      // The first element of the tuple is always the actual function
      // return.
      return cns(env, ad->nvGetVal(0));
    }

    auto const it = s_opt_emit_fns.find(fname->data());
    if (it != s_opt_emit_fns.end()) return it->second(env, params);
    return nullptr;
  }();

  if (result == nullptr) return nullptr;

  // We don't constrain types when loading parameters whose typehints don't
  // imply any checks. However, optimized codegen for a builtin generally
  // requires specific input types (and uses them to produce specific outputs).
  //
  // As a result, if we're returning optimized builtin codegen, we also need
  // to constrain our input parameters as well.
  //
  // To play well with assumptions in tracelet region selection, our optimized
  // codegen must obey the following restriction:
  //   - IF we relax the inputs for an optimized builtin to DataTypeSpecific
  //   - THEN the output must have its current type relaxed to DataTypeSpecific
  //
  // Here's a breaking example: a builtin that returns an int for one string
  // input and null for a different string input. If any builtin behaves like
  // this, it should place additional constraints on its inputs. (No current
  // builtins need to do so - DataTypeSpecific is a good default.)
  for (auto const& param : params.info) {
    env.irb->constrainValue(param.value, DataTypeSpecific);
  }
  params.decRefParams(env);
  return result;
}

//////////////////////////////////////////////////////////////////////

/*
 * Return the target type of  a parameter to a builtin function.
 *
 * If the builtin parameter has no type hints to cause coercion, this function
 * returns TBottom.
 */
Type param_target_type(const Func* callee, uint32_t paramIdx) {
  auto const& pi = callee->params()[paramIdx];
  auto const& tc = pi.typeConstraint;
  if (tc.isNullable()) {
    auto const dt = tc.underlyingDataType();
    if (!dt) return TBottom;
    return TNull | Type(*dt);
  }
  if (!pi.builtinType) return tc.isVArrayOrDArray() ? TArr : TBottom;
  if (pi.builtinType == KindOfObject &&
      pi.defaultValue.m_type == KindOfNull) {
    return TNullableObj;
  }
  return Type(*pi.builtinType);
}

//////////////////////////////////////////////////////////////////////

/*
 * Collect parameters for a call to a builtin.  Also determine which ones will
 * need to be passed through the eval stack, and which ones will need
 * conversions.
 */
template <class LoadParam>
ParamPrep
prepare_params(IRGS& /*env*/, const Func* callee, SSATmp* ctx,
               uint32_t numArgs, uint32_t numNonDefault, bool forNativeImpl,
               LoadParam loadParam) {
  auto ret = ParamPrep{numArgs, callee};
  ret.ctx = ctx;
  ret.forNativeImpl = forNativeImpl;

  // Fill in in reverse order, since they may come from popC's (depending on
  // what loadParam wants to do).
  for (auto offset = uint32_t{numArgs}; offset-- > 0;) {
    auto const ty = param_target_type(callee, offset);
    auto& cur = ret[offset];
    auto& pi = callee->params()[offset];

    cur.value = loadParam(offset, ty);
    // If ty > TBottom, it had some kind of type hint.
    cur.needsConversion = (offset < numNonDefault && ty > TBottom);
    cur.isInOut = callee->isInOut(offset);
    // We do actually mean exact type equality here.  We're only capable of
    // passing the following primitives through registers; everything else goes
    // by address unless its flagged "NativeArg".
    if (ty == TBool || ty == TInt || ty == TDbl || pi.isNativeArg()) {
      continue;
    }
    if (cur.isInOut) continue;

    ++ret.numByAddr;
    cur.passByAddr = true;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

/*
 * CatchMaker makes catch blocks for calling builtins.  There's a fair bit of
 * complexity here right now, for these reasons:
 *
 *    o Sometimes we're 'logically' inlining a php-level call to a function
 *      that contains a NativeImpl opcode.
 *
 *      But we implement this by generating all the relevant NativeImpl code
 *      after the InlineReturn for the callee, to make it easier for DCE to
 *      eliminate the code that constructs the callee's activation record.
 *      This means the unwinder is going to see our PC as equal to the FCall*
 *      for the call to the function. We produce consistent state for unwinder
 *      by decrefing and popping the arguments.
 *
 *    o HNI-style param coerce modes can force the entire function to return
 *      false or null if the coersions fail.  This is implemented via a
 *      TVCoercionException, which is not a user-visible exception.  So our
 *      catch blocks are sometimes handling a PHP exception, and sometimes a
 *      failure to coerce.
 *
 *    o Both of these things may be relevant to the same catch block.
 *
 * Also, note that the CatchMaker keeps a pointer to the builtin call's
 * ParamPrep, which will have its values mutated by realize_params as it's
 * making coersions, so that we can see what's changed so far (and what to
 * clean up on the offramps).  Some values that were refcounted may become
 * non-refcounted after conversions, and we can't DecRef things twice.
 */
struct CatchMaker {
  enum class Kind { NotInlining, Inlining };

  explicit CatchMaker(IRGS& env, Kind kind, const ParamPrep* params)
    : env(env)
    , m_kind(kind)
    , m_params(*params)
  {
    // Native instance method calls are allowed from NativeImpl or in inlining.
    // Native static method calls are *additionally* allowed from FCallBuiltin.
    if (m_params.ctx == nullptr) return;
    DEBUG_ONLY auto const this_type = m_params.ctx->type();
    assertx(this_type <= TCls || this_type <= TObj);
    assertx(this_type <= TCls || m_params.forNativeImpl || inlining());
  }

  CatchMaker(const CatchMaker&) = delete;
  CatchMaker(CatchMaker&&) = default;

  bool inlining() const {
    switch (m_kind) {
    case Kind::NotInlining:      return false;
    case Kind::Inlining:         return true;
    }
    not_reached();
  }

  Block* makeUnusualCatch() const {
    assertx(!env.irb->fs().stublogue());
    auto const exit = defBlock(env, Block::Hint::Unlikely);
    BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
    gen(env, BeginCatch);
    decRefParams();
    prepareForCatch();
    gen(env,
        EndCatch,
        EndCatchData {
          spOffBCFromIRSP(env),
          EndCatchData::CatchMode::UnwindOnly,
          EndCatchData::FrameMode::Phplogue,
          EndCatchData::Teardown::Full
        },
        fp(env), sp(env));
    return exit;
  }

  /*
   * DecRef the params in preparation for an exception or side
   * exit. Parameters that are not being passed through the stack
   * still may need to be decref'd, because they may have been a
   * reference counted type that was going to be converted to a
   * non-reference counted type that we'd pass in a register.  As we
   * do the coersions, params.value gets updated so whenever we call
   * these catch block creation functions it will only decref things
   * that weren't yet converted.
   */
  void decRefParams() const {
    if (m_params.forNativeImpl) return;
    for (auto i = m_params.size(); i--; ) {
      auto const &pi = m_params[i];
      if (pi.passByAddr) {
        popDecRef(env);
      } else {
        decRef(env, pi.value);
      }
    }
  }

private:
  void prepareForCatch() const {
    if (inlining() && m_params.ctx) {
      decRef(env, m_params.ctx);
    }
    /*
     * We're potentially spilling to a different depth than the unwinder
     * would've expected, so we need an eager sync.  Even if we aren't inlining
     * this can happen, because before doing the CallBuiltin we set the marker
     * stack offset to only include the passed-through-stack args.
     *
     * So before we leave, update the marker to placate EndCatch assertions,
     * which is trying to detect failure to do this properly.
     */
    auto const spOff = IRSPRelOffsetData { spOffBCFromIRSP(env) };
    gen(env, EagerSyncVMRegs, spOff, fp(env), sp(env));
    updateMarker(env);  // Mark the EndCatch safe, since we're eager syncing.
  }

private:
  IRGS& env;
  Kind const m_kind;
  const ParamPrep& m_params;
};

//////////////////////////////////////////////////////////////////////

/*
 * Take the value in param, apply any needed conversions
 * and return the value to be passed to CallBuiltin.
 *
 * checkType(ty, fail):
 *    verify that the param is of type ty, and branch to fail
 *    if not. If it results in a new SSATmp*, (as eg CheckType
 *    would), then that should be returned; otherwise it should
 *    return nullptr;
 * convertParam(ty):
 *    convert the param to ty; failure should be handled by
 *    CatchMaker::makeParamCoerceCatch, and it should return
 *    a new SSATmp* (if appropriate) or nullptr.
 * realize():
 *    return the SSATmp* needed by CallBuiltin for this parameter.
 *    if checkType and convertParam returned non-null values,
 *    param.value will have been updated with a phi of their results.
 */
template<class V, class C, class R>
SSATmp* realize_param(IRGS& env,
                      ParamPrep::Info& param,
                      const Func* callee,
                      Type targetTy,
                      V checkType,
                      C convertParam,
                      R realize) {
  if (param.needsConversion) {
    auto const baseTy = targetTy - TNull;
    assertx(baseTy.isKnownDataType());
    auto const convertTy = baseTy;

    if (auto const value = cond(
          env,
          [&] (Block* convert) -> SSATmp* {
            if (targetTy == baseTy) {
              return checkType(baseTy, convert);
            }
            return cond(
              env,
              [&] (Block* fail) { return checkType(baseTy, fail); },
              [&] (SSATmp* v) { return v; },
              [&] {
                return checkType(TInitNull, convert);
              });
          },
          [&] (SSATmp* v) { return v; },
          [&] () -> SSATmp* {
            return convertParam(convertTy);
          }
        )) {
      // Heads up on non-local state here: we have to update
      // the values inside ParamPrep so that the CatchMaker
      // functions know about new potentially refcounted types
      // to decref, or values that were already decref'd and
      // replaced with things like ints.
      param.value = value;
    }
  }

  return realize();
}

template<class U, class F>
SSATmp* maybeCoerceValue(
  IRGS& env,
  SSATmp* val,
  Type target,
  uint32_t id,
  const Func* func,
  U update,
  F fail
) {
  auto bail = [&] { fail(); return cns(env, TBottom); };
  if (target <= TStr) {
    if (!val->type().maybe(TFunc|TCls)) return bail();

    auto castW = [&] (SSATmp* val, bool isCls){
      if (RuntimeOption::EvalStringHintNotices) {
        gen(
          env,
          RaiseNotice,
          cns(
            env,
            makeStaticString(
              isCls ? Strings::CLASS_TO_STRING_IMPLICIT
                    : Strings::FUNC_TO_STRING_IMPLICIT
            )
          )
        );
      }
      return update(val);
    };

    return cond(
      env,
      [&] (Block* f) { return gen(env, CheckType, TFunc, f, val); },
      [&] (SSATmp* fval) { return castW(gen(env, LdFuncName, fval), false); },
      [&] {
        hint(env, Block::Hint::Unlikely);
        return cond(
          env,
          [&] (Block* f) { return gen(env, CheckType, TCls, f, val); },
          [&] (SSATmp* cval) { return castW(gen(env, LdClsName, cval), true); },
          [&] {
            hint(env, Block::Hint::Unlikely);
            return bail();
          }
        );
      }
    );
  }

  if (target <= (RuntimeOption::EvalHackArrDVArrs ? TVec : TArr)) {
    if (!val->type().maybe(TClsMeth)) return bail();
    return cond(
      env,
      [&] (Block* f) { return gen(env, CheckType, TClsMeth, f, val); },
      [&] (SSATmp* methVal) {
        if (RuntimeOption::EvalVecHintNotices) {
          raiseClsmethCompatTypeHint(
            env, id, func, func->params()[id].typeConstraint);
        }
        auto const ret = update(convertClsMethToVec(env, methVal));
        decRef(env, methVal);
        return ret;
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        return bail();
      }
    );
  }

  return bail();
}

/*
 * Prepare the actual arguments to the CallBuiltin instruction, by converting a
 * ParamPrep into a vector of SSATmps to pass to CallBuiltin.  If any of the
 * parameters needed type conversions, we need to do that here too.
 */
jit::vector<SSATmp*> realize_params(IRGS& env,
                                    const Func* callee,
                                    ParamPrep& params,
                                    const CatchMaker& maker) {
  auto const cbNumArgs = 2 + params.size() + (params.ctx ? 1 : 0);
  auto ret = jit::vector<SSATmp*>(cbNumArgs);
  auto argIdx = uint32_t{0};
  ret[argIdx++] = fp(env);
  ret[argIdx++] = sp(env);
  if (params.ctx) ret[argIdx++] = params.ctx;

  auto const needDVCheck = [&](uint32_t param, const Type& ty) {
    if (!RuntimeOption::EvalHackArrCompatTypeHintNotices) return false;
    if (!callee->params()[param].typeConstraint.isArray()) return false;
    return ty <= TArr;
  };

  auto const dvCheck = [&](uint32_t param, SSATmp* val) {
    assertx(needDVCheck(param, val->type()));
    auto const& tc = callee->params()[param].typeConstraint;

    gen(
      env,
      RaiseHackArrParamNotice,
      RaiseHackArrParamNoticeData { tc, int32_t(param), false },
      maker.makeUnusualCatch(),
      val,
      cns(env, callee)
    );
  };

  DEBUG_ONLY auto seenBottom = false;
  DEBUG_ONLY auto usedStack = false;
  auto stackIdx = uint32_t{0};
  for (auto paramIdx = uint32_t{0}; paramIdx < params.size(); ++paramIdx) {
    auto& param = params[paramIdx];
    auto const targetTy = param_target_type(callee, paramIdx);

    seenBottom |= (param.value->type() == TBottom);

    if (param.value->type() <= TPtrToCell) {
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) -> SSATmp* {
          gen(env, CheckTypeMem, ty, fail, param.value);
          if (needDVCheck(paramIdx, ty)) {
            dvCheck(paramIdx, gen(env, LdMem, ty, param.value));
          }
          return nullptr;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          auto val = gen(env, LdMem, TCell, param.value);
          assertx(ty.isKnownDataType());
          maybeCoerceValue(
            env,
            val,
            ty,
            paramIdx,
            callee,
            [&] (SSATmp* val) {
              gen(env, StLoc, LocalId{paramIdx}, fp(env), val);
              return val;
            },
            [&] {
              gen(env, ThrowParameterWrongType,
                  FuncArgTypeData { callee, paramIdx + 1, ty.toDataType() },
                  maker.makeUnusualCatch(), val);
            }
          );
          return nullptr;
        },
        [&] {
          if (!param.passByAddr) {
            assertx(targetTy == TBool ||
                    targetTy == TInt ||
                    targetTy == TDbl ||
                    callee->params()[paramIdx].isNativeArg() ||
                    callee->isInOut(paramIdx));
            return gen(env, LdMem,
                       targetTy == TBottom ? TCell : targetTy,
                       param.value);
          }
          return param.value;
        });
      continue;
    }

    if (!param.passByAddr) {
      auto const oldVal = params[paramIdx].value;
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) {
          auto ret = gen(env, CheckType, ty, fail, param.value);
          env.irb->constrainValue(ret, DataTypeSpecific);
          if (needDVCheck(paramIdx, ty)) dvCheck(paramIdx, ret);
          return ret;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          assert(ty.isKnownDataType());
          return maybeCoerceValue(
            env,
            param.value,
            ty,
            paramIdx,
            callee,
            [&] (SSATmp* val) { return val; },
            [&] {
              gen(env, ThrowParameterWrongType,
                  FuncArgTypeData { callee, paramIdx + 1, ty.toDataType() },
                  maker.makeUnusualCatch(), oldVal);
            }
          );
        },
        [&] {
          /*
           * This gets tricky:
           *   - if we had a ref-counted type, and it was converted
           *     to a Bool, Int or Dbl above, we explicitly DecReffed it
           *     (in coerce_value).
           *   - if we did a CoerceMem which implicitly DecReffed the old value
           * In either case, the old value is taken care of, and any future
           * DecRefs (from exceptions, or after the call on the normal flow
           * of execution) should DecRef param.value (ie the post-coercion
           * value).
           */
          auto v = param.value;
          return v;
        });
      continue;
    }

    usedStack = true;
    auto const offset = BCSPRelOffset{safe_cast<int32_t>(
        params.numByAddr - stackIdx - 1)};

    ret[argIdx++] = realize_param(
      env, param, callee, targetTy,
      [&] (const Type& ty, Block* fail) -> SSATmp* {
        auto irSPRel = offsetFromIRSP(env, offset);
        gen(env, CheckStk, IRSPRelOffsetData { irSPRel }, ty, fail, sp(env));
        env.irb->constrainStack(irSPRel, DataTypeSpecific);
        if (needDVCheck(paramIdx, ty)) {
          dvCheck(
            paramIdx,
            gen(env, LdStk, ty, IRSPRelOffsetData { irSPRel }, sp(env))
          );
        }
        return nullptr;
      },
      [&] (const Type& ty) -> SSATmp* {
        always_assert(ty.isKnownDataType());
        hint(env, Block::Hint::Unlikely);
        auto const off = IRSPRelOffsetData{ offsetFromIRSP(env, offset) };
        auto const tv = gen(env, LdStk, TCell, off, sp(env));

        maybeCoerceValue(
          env,
          tv,
          ty,
          paramIdx,
          callee,
          [&] (SSATmp* val) {
            gen(env, StStk, off, sp(env), val);
            env.irb->exceptionStackBoundary();
            return val;
          },
          [&] {
            gen(env, ThrowParameterWrongType,
                FuncArgTypeData { callee, paramIdx + 1, ty.toDataType() },
                maker.makeUnusualCatch(), tv);
          }
        );
        return nullptr;
      },
      [&] {
        return ldStkAddr(env, offset);
      });
    ++stackIdx;
  }

  assertx(seenBottom || !usedStack || stackIdx == params.numByAddr);
  assertx(argIdx == cbNumArgs);

  return ret;
}

//////////////////////////////////////////////////////////////////////

SSATmp* builtinInValue(IRGS& env, const Func* builtin, uint32_t i) {
  auto const tv = Native::builtinInValue(builtin, i);
  if (!tv) return nullptr;
  return cns(env, *tv);
}

SSATmp* builtinCall(IRGS& env,
                    const Func* callee,
                    ParamPrep& params,
                    int32_t numNonDefault,
                    const CatchMaker& catchMaker) {
  assertx(callee->nativeFuncPtr());

  if (!params.forNativeImpl) {
    // For FCallBuiltin, params are TypedValues, while for NativeImpl, they're
    // pointers to these values on the frame. We only optimize native calls
    // when we have the values.
    auto const opt = optimizedFCallBuiltin(env, callee, params, numNonDefault);
    if (opt) return opt;

    /*
     * Everything that needs to be on the stack gets spilled now.
     *
     * If we're not inlining, the reason we do this even when numByAddr is
     * zero is to make it so that in either case the stack depth when we enter
     * our catch blocks is always the same as the numByAddr value, in all
     * situations.  If we didn't do this, then when we aren't inlining, and
     * numByAddr is zero, we'd have the stack depth be the total num
     * params (the depth before the FCallBuiltin), which would add more cases
     * to handle in the catch blocks.
     */
    if (params.numByAddr != 0 || !catchMaker.inlining()) {
      for (auto i = uint32_t{0}; i < params.size(); ++i) {
        if (params[i].passByAddr) {
          push(env, params[i].value);
        }
      }
      /*
       * This marker update is to make sure rbx points to the bottom of our
       * stack if we enter a catch trace.  It's also necessary because we might
       * run destructors as part of parameter coersions, which we don't want to
       * clobber our spilled stack.
       */
      updateMarker(env);
    }

    // If we are inlining, we've done various DefInlineFP-type stuff that can
    // affect stack depth.
    env.irb->exceptionStackBoundary();
  }

  // Collect the realized parameters.
  auto realized = realize_params(env, callee, params, catchMaker);

  // Store the inout parameters into their out locations.
  if (callee->takesInOutParams()) {
    int32_t idx = 0;
    uint32_t aoff = params.ctx ? 3 : 2;
    for (auto i = uint32_t{0}; i < params.size(); ++i) {
      if (!params[i].isInOut) continue;
      auto ty = [&] () -> folly::Optional<Type> {
        auto const r = builtinOutType(callee, i);
        if (r.isKnownDataType()) return r;
        return {};
      }();
      if (auto const iv = builtinInValue(env, callee, i)) {
        decRef(env, realized[i + aoff]);
        realized[i + aoff] = iv;
        ty = iv->type();
        if (ty->maybe(TPersistentArr)) *ty |= TArr;
        if (ty->maybe(TPersistentVec)) *ty |= TVec;
        if (ty->maybe(TPersistentDict)) *ty |= TDict;
        if (ty->maybe(TPersistentKeyset)) *ty |= TKeyset;
        if (ty->maybe(TPersistentStr)) *ty |= TStr;
      }
      if (params.forNativeImpl) {
        // Move the value to the caller stack to avoid an extra ref-count
        gen(env, StLoc, LocalId{i}, fp(env), cns(env, TInitNull));
        auto const addr = gen(env, LdOutAddr, IndexData(idx++), fp(env));
        gen(env, StMem, ty, addr, realized[i + aoff]);
        realized[i + aoff] = addr;
        continue;
      }
      auto const offset =
        BCSPRelOffset{safe_cast<int32_t>(params.numByAddr + idx++)};
      auto const out = offsetFromIRSP(env, offset);
      gen(env, StStk, IRSPRelOffsetData{out}, ty, sp(env), realized[i + aoff]);
      params[i].value = cns(env, TInitNull);
      realized[i + aoff] = gen(env, LdStkAddr, IRSPRelOffsetData{out}, sp(env));
    }
    env.irb->exceptionStackBoundary();
  }

  auto const retOff =
    offsetFromIRSP(env, BCSPRelOffset{safe_cast<int32_t>(params.numByAddr)});

  // Make the actual call.
  SSATmp** const decayedPtr = &realized[0];
  auto const ret = gen(
    env,
    CallBuiltin,
    CallBuiltinData {
      spOffBCFromIRSP(env),
      retOff,
      callee,
      numNonDefault
    },
    catchMaker.makeUnusualCatch(),
    std::make_pair(realized.size(), decayedPtr)
  );

  if (!params.forNativeImpl) {
    if (params.ctx && params.ctx->type() <= TObj) {
      decRef(env, params.ctx);
    }
    catchMaker.decRefParams();
  }

  return ret;
}

/*
 * When we're inlining a NativeImpl opcode, we know this is the only opcode in
 * the callee method body aside from AssertRATs (bytecode invariant).  So in
 * order to make sure we can eliminate the DefInlineFP, we do the CallBuiltin
 * instruction after we've left the inlined frame.
 *
 * We may need to pass some arguments to the builtin through the stack (e.g. if
 * it takes const Variant&'s)---these are spilled to the stack after leaving
 * the callee.
 *
 * To make this work, we need to do some weird things in the catch trace.  ;)
 */
void nativeImplInlined(IRGS& env) {
  auto const callee = curFunc(env);
  assertx(callee->nativeFuncPtr());

  auto const numArgs = callee->numParams();
  auto const paramThis = callee->isMethod() ? ldCtx(env) : nullptr;

  auto numNonDefault = fp(env)->inst()->extra<DefInlineFP>()->numArgs;
  auto params = prepare_params(
    env,
    callee,
    paramThis,
    numArgs,
    numNonDefault,
    false,
    [&] (uint32_t i, const Type) {
      return ldLoc(env, i, nullptr, DataTypeSpecific);
    }
  );

  implInlineReturn(env);

  auto const catcher = CatchMaker {
    env,
    CatchMaker::Kind::Inlining,
    &params
  };

  push(env, builtinCall(env, callee, params, numNonDefault, catcher));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

int getBuiltinVanillaParam(const char* name) {
  auto const it = s_vanilla_params.find(name);
  return it != s_vanilla_params.end() ? it->second : -1;
}

SSATmp* optimizedCallIsObject(IRGS& env, SSATmp* src) {
  if (src->isA(TObj) && src->type().clsSpec()) {
    auto const cls = src->type().clsSpec().cls();
    if (!env.irb->constrainValue(src, GuardConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard
      // any further, use it.
      return cns(env, cls != SystemLib::s___PHP_Incomplete_ClassClass);
    }
  }

  if (!src->type().maybe(TObj)) {
    return cns(env, false);
  }

  auto checkClass = [&] (SSATmp* obj) {
    auto cls = gen(env, LdObjClass, obj);
    auto testCls = SystemLib::s___PHP_Incomplete_ClassClass;
    auto eq = gen(env, EqCls, cls, cns(env, testCls));
    return gen(env, XorBool, eq, cns(env, true));
  };

  return cond(
    env,
    [&] (Block* taken) {
      auto isObj = gen(env, IsType, TObj, src);
      gen(env, JmpZero, taken, isObj);
    },
    [&] { // Next: src is an object
      auto obj = gen(env, AssertType, TObj, src);
      return checkClass(obj);
    },
    [&] { // Taken: src is not an object
      return cns(env, false);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void emitFCallBuiltin(IRGS& env,
                      uint32_t numArgs,
                      uint32_t numNonDefault,
                      uint32_t numOut,
                      const StringData* funcName) {
  auto const callee = Unit::lookupBuiltin(funcName);
  if (!callee) PUNT(Missing-builtin);

  if (callee->numInOutParams() != numOut) PUNT(bad-inout);

  emitCallerRxChecksKnown(env, callee);
  assertx(!callee->isMethod() || (callee->isStatic() && callee->cls()));
  auto const ctx = callee->isStatic() ? cns(env, callee->cls()) : nullptr;

  auto params = prepare_params(
    env, callee, ctx,
    numArgs, numNonDefault, false, [&](uint32_t /*i*/, const Type ty) {
      auto specificity =
        ty == TBottom ? DataTypeGeneric : DataTypeSpecific;
      return pop(env, specificity);
    });

  auto const catcher = CatchMaker {
    env,
    CatchMaker::Kind::NotInlining,
    &params
  };

  push(env, builtinCall(env, callee, params, numNonDefault, catcher));
}

void emitNativeImpl(IRGS& env) {
  if (isInlining(env)) return nativeImplInlined(env);

  auto const callee = curFunc(env);

  auto genericNativeImpl = [&]() {
    gen(env, NativeImpl, fp(env), sp(env));
    auto const retVal = gen(env, LdRetVal, callReturnType(callee), fp(env));
    auto const spAdjust = offsetToReturnSlot(env);
    auto const data = RetCtrlData { spAdjust, false, AuxUnion{0} };
    gen(env, RetCtrl, data, sp(env), fp(env), retVal);
  };

  if (!callee->nativeFuncPtr()) {
    genericNativeImpl();
    return;
  }

  auto ctx = callee->isMethod() ? ldCtx(env) : nullptr;

  auto params = prepare_params(
    env,
    callee,
    ctx,
    callee->numParams(),
    callee->numParams(),
    true,
    [&] (uint32_t i, const Type) {
      return gen(env, LdLocAddr, LocalId(i), fp(env));
    }
  );
  auto const catcher = CatchMaker {
    env,
    CatchMaker::Kind::NotInlining,
    &params
  };

  push(env, builtinCall(env, callee, params, callee->numParams(), catcher));
  emitRetC(env);
}

//////////////////////////////////////////////////////////////////////

namespace {

const StaticString s_add("add");
const StaticString s_addall("addall");
const StaticString s_append("append");
const StaticString s_clear("clear");
const StaticString s_remove("remove");
const StaticString s_removeall("removeall");
const StaticString s_removekey("removekey");
const StaticString s_set("set");
const StaticString s_setall("setall");

// Whitelist of known collection methods that always return $this (ignoring
// parameter coercion failure issues).
bool collectionMethodReturnsThis(const Func* callee) {
  auto const cls = callee->implCls();

  if (cls == c_Vector::classof()) {
    return
      callee->name()->isame(s_add.get()) ||
      callee->name()->isame(s_addall.get()) ||
      callee->name()->isame(s_append.get()) ||
      callee->name()->isame(s_clear.get()) ||
      callee->name()->isame(s_removekey.get()) ||
      callee->name()->isame(s_set.get()) ||
      callee->name()->isame(s_setall.get());
  }

  if (cls == c_Map::classof()) {
    return
      callee->name()->isame(s_add.get()) ||
      callee->name()->isame(s_addall.get()) ||
      callee->name()->isame(s_clear.get()) ||
      callee->name()->isame(s_remove.get()) ||
      callee->name()->isame(s_set.get()) ||
      callee->name()->isame(s_setall.get());
  }

  if (cls == c_Set::classof()) {
    return
      callee->name()->isame(s_add.get()) ||
      callee->name()->isame(s_addall.get()) ||
      callee->name()->isame(s_clear.get()) ||
      callee->name()->isame(s_remove.get()) ||
      callee->name()->isame(s_removeall.get());
  }

  return false;
}

}

Type builtinOutType(const Func* builtin, uint32_t i) {
  assertx(builtin->isCPPBuiltin());
  assertx(builtin->isInOut(i));

  if (auto const dt = Native::builtinOutType(builtin, i)) return Type{*dt};

  auto const& tc = builtin->params()[i].typeConstraint;

  if (tc.isSoft() || tc.isMixed()) return TInitCell;

  auto ty = [&] () -> Type {
    switch (tc.metaType()) {
    case AnnotMetaType::Precise:
      return Type{*tc.underlyingDataType()};
    case AnnotMetaType::Mixed:
      return TInitCell;
    case AnnotMetaType::Self:
      return TObj;
    case AnnotMetaType::Parent:
      return TObj;
    case AnnotMetaType::Callable:
      return TInitCell;
    case AnnotMetaType::Number:
      return TInt | TDbl;
    case AnnotMetaType::ArrayKey:
      return TInt | TStr;
    case AnnotMetaType::This:
      return TObj;
    case AnnotMetaType::VArray:
      return RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
    case AnnotMetaType::DArray:
      return RuntimeOption::EvalHackArrDVArrs ? TDict : TArr;
    case AnnotMetaType::VArrOrDArr:
      return RuntimeOption::EvalHackArrDVArrs ? TVec | TDict : TArr;
    case AnnotMetaType::VecOrDict:
      return TVec | TDict;
    case AnnotMetaType::ArrayLike:
      return TArrLike;
    case AnnotMetaType::Nonnull:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Nothing:
      return TInitCell;
    }
    not_reached();
  }();

  return tc.isNullable() ? ty | TInitNull : ty;
}

Type builtinReturnType(const Func* builtin) {
  // Why do we recalculate the type here than just using HHBBC's inferred type?
  // Unlike for regular PHP functions, we have access to all the same
  // information that HHBBC does, and the JIT type-system is slightly more
  // expressive. So, by doing it ourself, we can derive a slightly more precise
  // type.
  assertx(builtin->isCPPBuiltin());

  // NB: It is *not* safe to be pessimistic here and return TCell (or any other
  // approximation). The builtin's return type inferred here is used to control
  // code-gen when lowering the builtin call to vasm and must be no more general
  // than the HNI declaration (if present).
  auto type = [&]{
    // If this is a collection method which returns $this, use that fact to
    // infer the exact returning type. Otherwise try to use HNI declaration.
    if (collectionMethodReturnsThis(builtin)) {
      assertx(builtin->hniReturnType() == KindOfObject);
      return Type::ExactObj(builtin->implCls());
    }
    if (auto const hniType = builtin->hniReturnType()) {
      if (isArrayType(*hniType)) {
        auto const& constraint = builtin->returnTypeConstraint();
        if (constraint.isVArray()) return TVArr;
        if (constraint.isDArray()) return TDArr;
      }
      return Type{*hniType};
    }
    return TInitCell;
  }();

  // We're not sure what kind of array-likes builtins will produce.
  if (RO::EvalAllowBespokeArrayLikes) type = type.widenToBespoke();

  // "Reference" types (types represented by a pointer) can always be null.
  if (type.isReferenceType()) {
    type |= TInitNull;
  } else {
    assertx(type == TInitCell || type.isSimpleType());
  }

  return type & TInitCell;
}

/////////////////////////////////////////////////////////////////////

namespace {

void implArrayIdx(IRGS& env) {
  // These types are just used to decide what to do; once we know what we're
  // actually doing we constrain the values with the popC()s later on in this
  // function.
  auto const keyType = topC(env, BCSPRelOffset{1}, DataTypeGeneric)->type();

  if (keyType <= TNull) {
    auto const def = popC(env, DataTypeGeneric);
    auto const key = popC(env);
    auto const base = popC(env);

    // if the key is null it will not be found so just return the default
    push(env, def);
    decRef(env, base);
    decRef(env, key);
    return;
  }
  if (!(keyType <= TInt || keyType <= TStr)) {
    interpOne(env, TCell, 3);
    return;
  }

  auto const def = popC(env, DataTypeGeneric); // a helper will decref it but
                                               // the translated code doesn't
                                               // care about the type
  auto const key = popC(env);
  auto const base = popC(env);

  auto const elem = profiledArrayAccess(env, base, key,
    [&] (SSATmp* arr, SSATmp* key, uint32_t pos) {
      return gen(env, MixedArrayGetK, IndexData { pos }, arr, key);
    },
    [&] (SSATmp* key, SizeHintData data) {
      return gen(env, ArrayIdx, data, base, key, def);
    }
  );

  auto finish = [&](SSATmp* tmp) {
    pushIncRef(env, tmp);
    decRef(env, base);
    decRef(env, key);
    decRef(env, def);
  };

  auto const pelem = profiledType(env, elem, [&] { finish(elem); });
  finish(pelem);
}

void implVecIdx(IRGS& env, SSATmp* loaded_collection_vec) {
  auto const def = popC(env);
  auto const key = popC(env);
  auto const stack_base = popC(env);

  auto const finish = [&](SSATmp* elem) {
    pushIncRef(env, elem);
    decRef(env, def);
    decRef(env, key);
    decRef(env, stack_base);
  };

  if (key->isA(TNull | TStr)) return finish(def);

  if (!key->isA(TInt)) {
    // TODO(T11019533): Fix the underlying issue with unreachable code rather
    // than papering over it by pushing an unused value here.
    finish(def);
    updateMarker(env);
    env.irb->exceptionStackBoundary();
    gen(env, ThrowInvalidArrayKey, stack_base, key);
    return;
  }

  auto const use_base = loaded_collection_vec
    ? loaded_collection_vec
    : stack_base;
  assertx(use_base->isA(TVec));

  auto const elem = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayDataBounds, taken, use_base, key);
    },
    [&] { return gen(env, LdVecElem, use_base, key); },
    [&] { return def; }
  );

  auto const pelem = profiledType(env, elem, [&] { finish(elem); } );
  finish(pelem);
}

void implDictKeysetIdx(IRGS& env,
                       bool is_dict,
                       SSATmp* loaded_collection_dict) {
  auto const def = popC(env);
  auto const key = popC(env);
  auto const stack_base = popC(env);

  auto const finish = [&](SSATmp* elem) {
    pushIncRef(env, elem);
    decRef(env, def);
    decRef(env, key);
    decRef(env, stack_base);
  };

  if (key->isA(TNull)) return finish(def);

  if (!key->isA(TInt) && !key->isA(TStr)) {
    // TODO(T11019533): Fix the underlying issue with unreachable code rather
    // than papering over it by pushing an unused value here.
    finish(def);
    updateMarker(env);
    env.irb->exceptionStackBoundary();
    gen(env, ThrowInvalidArrayKey, stack_base, key);
    return;
  }

  assertx(is_dict || !loaded_collection_dict);
  auto const use_base = loaded_collection_dict
    ? loaded_collection_dict
    : stack_base;
  assertx(use_base->isA(is_dict ? TDict : TKeyset));

  auto const elem = profiledArrayAccess(env, use_base, key,
    [&] (SSATmp* base, SSATmp* key, uint32_t pos) {
      return gen(env, is_dict ? DictGetK : KeysetGetK, IndexData { pos },
                 base, key);
    },
    [&] (SSATmp* key, SizeHintData data) {
      return is_dict ? gen(env, DictIdx, data, use_base, key, def)
                     : gen(env, KeysetIdx, use_base, key, def);
    }
  );

  auto const pelem = profiledType(env, elem, [&] { finish(elem); });
  finish(pelem);
}

/*
 * Return the GuardConstraint that should be used to constrain baseType for an
 * Idx bytecode.
 */
GuardConstraint idxBaseConstraint(Type baseType, Type keyType,
                                 bool& useVec, bool& useDict) {
  if (baseType < TObj && baseType.clsSpec()) {
    auto const cls = baseType.clsSpec().cls();

    // Vector is only usable with int keys, so we can only optimize for
    // Vector if the key is an Int
    useVec = (collections::isType(cls, CollectionType::Vector) ||
              collections::isType(cls, CollectionType::ImmVector)) &&
             keyType <= TInt;

    useDict = collections::isType(cls, CollectionType::Map) ||
              collections::isType(cls, CollectionType::ImmMap) ||
              collections::isType(cls, CollectionType::Set) ||
              collections::isType(cls, CollectionType::ImmSet);

    if (useVec || useDict) return GuardConstraint(cls);
  }

  useVec = useDict = false;
  return DataTypeSpecific;
}

//////////////////////////////////////////////////////////////////////

}

void emitArrayIdx(IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{2}, DataTypeGeneric)->type();
  if (arrType <= TVec) return implVecIdx(env, nullptr);
  if (arrType <= TDict) return implDictKeysetIdx(env, true, nullptr);
  if (arrType <= TKeyset) return implDictKeysetIdx(env, false, nullptr);
  if (arrType <= TClsMeth) PUNT(ArrayIdx_clsmeth);

  if (!(arrType <= TArr)) {
    // raise fatal
    interpOne(env, TCell, 3);
    return;
  }

  implArrayIdx(env);
}

void emitIdx(IRGS& env) {
  auto const key      = topC(env, BCSPRelOffset{1}, DataTypeGeneric);
  auto const base     = topC(env, BCSPRelOffset{2}, DataTypeGeneric);
  auto const keyType  = key->type();
  auto const baseType = base->type();

  if (baseType <= TVec) return implVecIdx(env, nullptr);
  if (baseType <= TDict) return implDictKeysetIdx(env, true, nullptr);
  if (baseType <= TKeyset) return implDictKeysetIdx(env, false, nullptr);

  if (keyType <= TNull || !baseType.maybe(TArr | TObj | TStr)) {
    auto const def = popC(env, DataTypeGeneric);
    popC(env, keyType <= TNull ? DataTypeSpecific : DataTypeGeneric);
    popC(env, keyType <= TNull ? DataTypeGeneric : DataTypeSpecific);
    push(env, def);
    decRef(env, base);
    decRef(env, key);
    return;
  }

  if (!(keyType <= TInt || keyType <= TStr)) {
    interpOne(env, TCell, 3);
    return;
  }

  if (baseType <= TArr) {
    implArrayIdx(env);
    return;
  }

  bool useVec, useDict;
  auto const tc = idxBaseConstraint(baseType, keyType, useVec, useDict);
  if (useVec || useDict) {
    env.irb->constrainValue(base, tc);
    env.irb->constrainValue(key, DataTypeSpecific);

    if (useVec) {
      auto const vec = gen(env, LdColVec, base);
      implVecIdx(env, vec);
    } else {
      auto const dict = gen(env, LdColDict, base);
      implDictKeysetIdx(env, true, dict);
    }
    return;
  }

  interpOne(env, TCell, 3);
}

void emitAKExists(IRGS& env) {
  auto const arr = popC(env);
  auto key = popC(env);
  if (key->isA(TFunc) || key->isA(TCls)) PUNT(AKExists_func_cls_key);

  auto throwBadKey = [&] {
    // TODO(T11019533): Fix the underlying issue with unreachable code rather
    // than papering over it by pushing an unused value here.
    push(env, cns(env, false));
    decRef(env, arr);
    decRef(env, key);
    updateMarker(env);
    env.irb->exceptionStackBoundary();
    gen(env, ThrowInvalidArrayKey, arr, key);
  };

  auto const check_packed = [&] {
    assertx(key->isA(TInt));

    auto const result = cond(
      env,
      [&](Block* taken) {
        gen(env, CheckPackedArrayDataBounds, taken, arr, key);
      },
      [&] { return cns(env, true); },
      [&] { return cns(env, false); }
    );
    push(env, result);
    decRef(env, arr);
  };

  if (arr->isA(TVec)) {
    if (key->isA(TStr)) {
      push(env, cns(env, false));
      decRef(env, arr);
      decRef(env, key);
      return;
    }
    if (key->isA(TInt)) {
      return check_packed();
    }
    return throwBadKey();
  }

  if (arr->isA(TDict) || arr->isA(TKeyset)) {
    if (!key->isA(TInt) && !key->isA(TStr)) {
      return throwBadKey();
    }
    auto const val = gen(
      env,
      arr->isA(TDict) ? AKExistsDict : AKExistsKeyset,
      arr,
      key
    );
    push(env, val);
    decRef(env, arr);
    decRef(env, key);
    return;
  }

  if (!arr->isA(TArr) && !arr->isA(TObj)) PUNT(AKExists_badArray);

  if (key->isA(TInitNull) && arr->isA(TArr)) {
    if (checkHACArrayKeyCast()) {
      gen(
        env,
        RaiseHackArrCompatNotice,
        cns(
          env,
          makeStaticString(
            makeHackArrCompatImplicitArrayKeyMsg(uninit_variant.asTypedValue())
          )
        )
      );
    }

    key = cns(env, staticEmptyString());
  }

  if (!key->isA(TStr) && !key->isA(TInt)) PUNT(AKExists_badKey);

  if (arr->isA(TObj) && key->isA(TInt) &&
      collections::isType(arr->type().clsSpec().cls(), CollectionType::Vector,
                          CollectionType::ImmVector)) {
    auto const val =
      gen(env, CheckRange, key, gen(env, CountCollection, arr));
    push(env, val);
    decRef(env, arr);
    return;
  }
  if (arr->isA(TArr) && key->isA(TInt) &&
      arr->type().arrSpec().kind() == ArrayData::kPackedKind) {
    return check_packed();
  }

 auto const val =
    gen(env, arr->isA(TArr) ? AKExistsArr : AKExistsObj, arr, key);
  push(env, val);
  decRef(env, arr);
  decRef(env, key);
}

//////////////////////////////////////////////////////////////////////

void emitGetMemoKeyL(IRGS& env, NamedLocal loc) {
  DEBUG_ONLY auto const func = curFunc(env);
  assertx(func->isMemoizeWrapper());

  auto const value = ldLocWarn(
    env,
    loc,
    nullptr,
    DataTypeSpecific
  );

  // Use the generic scheme, which is implemented by GetMemoKey. The simplifier
  // will catch any additional special cases.
  push(env, gen(env, GetMemoKey, value));
}

namespace {

void memoGetImpl(IRGS& env,
                 Offset notfoundOff,
                 Offset suspendedOff,
                 LocalRange keys) {
  assertx(curFunc(env)->isMemoizeWrapper());
  assertx(keys.first + keys.count <= curFunc(env)->numLocals());
  assertx(suspendedOff == kInvalidOffset || curFunc(env)->isAsyncFunction());

  CompactVector<bool> types;
  for (auto i = keys.count; i > 0; --i) {
    auto const type = env.irb->local(keys.first + i - 1, DataTypeSpecific).type;
    if (type <= TStr) {
      types.emplace_back(true);
    } else if (type <= TInt) {
      types.emplace_back(false);
    } else {
      // Let it fatal from the interpreter
      PUNT(MemoGet);
    }
  }

  auto const notFound = getBlock(env, bcOff(env) + notfoundOff);
  assertx(notFound != nullptr);

  auto const func = curFunc(env);

  auto const loadAux = suspendedOff != kInvalidOffset;

  auto const val = [&]{
    // Any value we get from memoization must be the same type we return from
    // this function. If we need to load the aux field, force the type to be
    // InitCell so that we actually load the type. We'll assert the proper type
    // once we've checked aux.
    auto const retTy = loadAux
      ? TInitCell
      : typeFromRAT(func->repoReturnType(), curClass(env)) & TInitCell;

    if (func->isMethod() && !func->isStatic()) {
      auto const cls = func->cls();
      assertx(cls != nullptr);
      assertx(cls->hasMemoSlots());

      auto const this_ = checkAndLoadThis(env);
      if (!this_->isA(Type::SubObj(cls))) PUNT(MemoGet);

      auto const memoInfo = cls->memoSlotForFunc(func->getFuncId());

      if (keys.count == 0 && !memoInfo.second) {
        return gen(
          env,
          MemoGetInstanceValue,
          MemoValueInstanceData { memoInfo.first, func, folly::none, loadAux },
          notFound,
          retTy,
          this_
        );
      }

      return gen(
        env,
        MemoGetInstanceCache,
        MemoCacheInstanceData {
          memoInfo.first,
          keys,
          types.data(),
          func,
          memoInfo.second,
          folly::none,
          loadAux
        },
        notFound,
        retTy,
        fp(env),
        this_
      );
    }

    if (func->isMemoizeWrapperLSB()) {
      /* For LSB memoization, we need the LSB class */
      auto const lsbCls = ldCtxCls(env);
      if (keys.count > 0) {
        return gen(
          env,
          MemoGetLSBCache,
          MemoCacheStaticData {
            func,
            keys,
            types.data(),
            folly::none,
            loadAux
          },
          notFound,
          retTy,
          fp(env),
          lsbCls
        );
      }
      return gen(
        env,
        MemoGetLSBValue,
        MemoValueStaticData { func, folly::none, loadAux },
        notFound,
        retTy,
        lsbCls
      );
    }

    /* Static (non-LSB) Memoization */
    if (keys.count > 0) {
      return gen(
        env,
        MemoGetStaticCache,
        MemoCacheStaticData { func, keys, types.data(), folly::none, loadAux },
        notFound,
        retTy,
        fp(env)
      );
    }
    return gen(
      env,
      MemoGetStaticValue,
      MemoValueStaticData { func, folly::none, loadAux },
      notFound,
      retTy
    );
  }();

  if (!loadAux) {
    pushIncRef(env, val);
    return;
  }

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const aux = gen(env, LdTVAux, LdTVAuxData {}, val);
      auto const tst = gen(env, AndInt, aux, cns(env, 1u << 31));
      gen(env, JmpZero, taken, tst);
    },
    [&] {
      pushIncRef(
        env,
        gen(
          env,
          AssertType,
          typeFromRAT(func->repoAwaitedReturnType(), curClass(env)) & TInitCell,
          val
        )
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      pushIncRef(
        env,
        gen(
          env,
          AssertType,
          typeFromRAT(func->repoReturnType(), curClass(env)) & TInitCell,
          val
        )
      );
      jmpImpl(env, bcOff(env) + suspendedOff);
    }
  );
}

}

void emitMemoGet(IRGS& env, Offset notfoundOff, LocalRange keys) {
  memoGetImpl(env, notfoundOff, kInvalidOffset, keys);
}

void emitMemoGetEager(IRGS& env,
                      Offset notfoundOff,
                      Offset suspendedOff,
                      LocalRange keys) {
  assertx(curFunc(env)->isAsyncFunction());
  assertx(resumeMode(env) == ResumeMode::None);
  memoGetImpl(env, notfoundOff, suspendedOff, keys);
}

namespace {

void memoSetImpl(IRGS& env, LocalRange keys, bool eager) {
  assertx(curFunc(env)->isMemoizeWrapper());
  assertx(keys.first + keys.count <= curFunc(env)->numLocals());
  assertx(!eager || curFunc(env)->isAsyncFunction());

  CompactVector<bool> types;
  for (auto i = keys.count; i > 0; --i) {
    auto const type = env.irb->local(keys.first + i - 1, DataTypeSpecific).type;
    if (type <= TStr) {
      types.emplace_back(true);
    } else if (type <= TInt) {
      types.emplace_back(false);
    } else {
      // Let it fatal from the interpreter
      PUNT(MemoSet);
    }
  }

  auto const ldVal = [&] (DataTypeCategory tc) {
    return gen(
      env,
      AssertType,
      TInitCell,
      topC(env, BCSPRelOffset{ 0 }, tc)
    );
  };

  auto const func = curFunc(env);

  auto const asyncEager = [&] () -> folly::Optional<bool> {
    if (!func->isAsyncFunction()) return folly::none;
    return eager;
  }();

  if (func->isMethod() && !func->isStatic()) {
    auto const cls = func->cls();
    assertx(cls != nullptr);
    assertx(cls->hasMemoSlots());

    auto const this_ = checkAndLoadThis(env);
    if (!this_->isA(Type::SubObj(cls))) PUNT(MemoSet);

    auto const memoInfo = cls->memoSlotForFunc(func->getFuncId());

    if (keys.count == 0 && !memoInfo.second) {
      gen(
        env,
        MemoSetInstanceValue,
        MemoValueInstanceData { memoInfo.first, func, asyncEager, false },
        this_,
        ldVal(DataTypeCountness)
      );
      return;
    }

    gen(
      env,
      MemoSetInstanceCache,
      MemoCacheInstanceData {
        memoInfo.first,
        keys,
        types.data(),
        func,
        memoInfo.second,
        asyncEager,
        false
      },
      fp(env),
      this_,
      ldVal(DataTypeGeneric)
    );
    return;
  }

  if (func->isMemoizeWrapperLSB()) {
    /* For LSB memoization, we need the LSB class */
    auto const lsbCls = ldCtxCls(env);
    if (keys.count > 0) {
      gen(
        env,
        MemoSetLSBCache,
        MemoCacheStaticData { func, keys, types.data(), asyncEager, false },
        fp(env),
        lsbCls,
        ldVal(DataTypeGeneric)
      );
      return;
    }

    gen(
      env,
      MemoSetLSBValue,
      MemoValueStaticData { func, asyncEager, false },
      ldVal(DataTypeCountness),
      lsbCls
    );
    return;
  }

  if (keys.count > 0) {
    gen(
      env,
      MemoSetStaticCache,
      MemoCacheStaticData { func, keys, types.data(), asyncEager, false },
      fp(env),
      ldVal(DataTypeGeneric)
    );
    return;
  }

  gen(
    env,
    MemoSetStaticValue,
    MemoValueStaticData { func, asyncEager, false },
    ldVal(DataTypeCountness)
  );
}

}

void emitMemoSet(IRGS& env, LocalRange keys) {
  memoSetImpl(env, keys, false);
}

void emitMemoSetEager(IRGS& env, LocalRange keys) {
  assertx(curFunc(env)->isAsyncFunction());
  assertx(resumeMode(env) == ResumeMode::None);
  memoSetImpl(env, keys, true);
}

//////////////////////////////////////////////////////////////////////

void emitSilence(IRGS& env, Id localId, SilenceOp subop) {
  // We can't generate direct StLoc and LdLocs in pseudomains (violates an IR
  // invariant).
  if (curFunc(env)->isPseudoMain()) PUNT(PseudoMain-Silence);

  switch (subop) {
  case SilenceOp::Start:
    // We assume that whatever is in the local is dead and doesn't need to be
    // refcounted before being overwritten.
    gen(env, AssertLoc, TUncounted, LocalId(localId), fp(env));
    gen(env, StLoc, LocalId(localId), fp(env), gen(env, ZeroErrorLevel));
    break;
  case SilenceOp::End:
    {
      gen(env, AssertLoc, TInt, LocalId(localId), fp(env));
      auto const level = ldLoc(env, localId, makeExit(env), DataTypeGeneric);
      gen(env, RestoreErrorLevel, level);
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////
}}}
