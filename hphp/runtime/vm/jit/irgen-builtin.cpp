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
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/analysis.h"
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
  explicit ParamPrep(size_t count) : info(count) {}

  struct Info {
    SSATmp* value{nullptr};
    bool passByAddr{false};
    bool needsConversion{false};
    bool isOutputArg{false};
  };

  const Info& operator[](size_t idx) const { return info[idx]; }
  Info& operator[](size_t idx) { return info[idx]; }
  size_t size() const { return info.size(); }

  SSATmp* thiz{nullptr};       // may be null if call is not a method
  SSATmp* count{nullptr};      // if non-null, the count of arguments
  jit::vector<Info> info;
  uint32_t numByAddr{0};

  bool forNativeImpl{false};
};

//////////////////////////////////////////////////////////////////////

const StaticString
  s_is_a("is_a"),
  s_is_subclass_of("is_subclass_of"),
  s_method_exists("method_exists"),
  s_count("count"),
  s_sizeof("sizeof"),
  s_ini_get("ini_get"),
  s_in_array("in_array"),
  s_get_class("get_class"),
  s_sqrt("sqrt"),
  s_strlen("strlen"),
  s_clock_gettime_ns("clock_gettime_ns"),
  s_microtime("microtime"),
  s_max2("__SystemLib\\max2"),
  s_min2("__SystemLib\\min2"),
  s_ceil("ceil"),
  s_floor("floor"),
  s_abs("abs"),
  s_ord("ord"),
  s_chr("chr"),
  s_array_key_cast("hh\\array_key_cast"),
  s_type_structure("hh\\type_structure"),
  s_is_list_like("hh\\is_list_like"),
  s_one("1"),
  s_empty(""),
  s_container_first("HH\\Lib\\_Private\\Native\\first"),
  s_container_last("HH\\Lib\\_Private\\Native\\last"),
  s_container_first_key("HH\\Lib\\_Private\\Native\\first_key"),
  s_container_last_key("HH\\Lib\\_Private\\Native\\last_key"),
  s_class_meth_get_class("HH\\class_meth_get_class"),
  s_class_meth_get_method("HH\\class_meth_get_method");

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

  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, nextBcOff(env)), exit);
  gen(env, BeginCatch);
  for (auto i = params.size(); i--; ) {
    decRef(env, params[i].value);
  }
  gen(env, EndCatch,
      IRSPRelOffsetData { spOffBCFromIRSP(env) },
      fp(env), sp(env));
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

  auto const cls = Unit::lookupUniqueClassInContext(classname->strVal(),
                                                    curClass(env));
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

SSATmp* opt_count(IRGS& env, const ParamPrep& params) {
  if (params.size() != 2) return nullptr;

  auto const mode = params[1].value;
  auto const val = params[0].value;

  if (val->isA(TClsMeth)) return cns(env, 2);

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

  if (params.forNativeImpl) return nullptr;

  // In strict mode type mismatches won't be coerced (for legacy reasons in HH
  // files builtins are always weak).
  if (curFunc(env)->unit()->useStrictTypes() &&
      !curFunc(env)->unit()->isHHFile() &&
      !RuntimeOption::EnableHipHopSyntax) {
    return nullptr;
  }

  // intercept constant, non-string ord() here instead of OrdStr simplify stage.
  // OrdStr depends on a string as input for its vasm implementation.
  if (arg->hasConstVal(TBool)) {
    // ord((string)true)===ord("1"), ord((string)false)===ord("")
    return cns(env, int64_t{arg_type.boolVal() ? '1' : 0});
  }
  if (arg_type <= TNull) {
    return cns(env, int64_t{0});
  }
  if (arg->hasConstVal(TInt)) {
    const auto conv = folly::to<std::string>(arg_type.intVal());
    return cns(env, int64_t{conv[0]});
  }
  if (arg->hasConstVal(TDbl)) {
    const auto conv = folly::to<std::string>(arg_type.dblVal());
    return cns(env, int64_t{conv[0]});
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
    return cns(
      env,
      value.toBoolean() ? s_one.get() : s_empty.get()
    );
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
  if (params.size() == 0) return curName();
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;
  auto const ty  = val->type();
  if (ty <= TNull) return curName();
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
  if (ty <= TInt) {
    auto const conv = gen(env, ConvIntToDbl, val);
    return gen(env, Sqrt, conv);
  }
  return nullptr;
}

SSATmp* opt_strlen(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const val = params[0].value;
  auto const ty  = val->type();

  if (ty <= TStr) {
    return gen(env, LdStrLen, val);
  }

  if (RuntimeOption::EvalWarnOnCoerceBuiltinParams) {
    return nullptr;
  }

  if (ty <= TNull) return cns(env, 0);
  if (ty <= TBool) return gen(env, ConvBoolToInt, val);

  if (ty.subtypeOfAny(TInt, TDbl)) {
    auto str = ty <= TInt
      ? gen(env, ConvIntToStr, val)
      : gen(env, ConvDblToStr, val);
    auto len = gen(env, LdStrLen, str);
    decRef(env, str);
    return len;
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
  auto const dbl = gen(env, ConvCellToDbl, make_opt_catch(env, params), val);
  return gen(env, Ceil, dbl);
}

SSATmp* opt_floor(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = params[0].value;
  if (!type_converts_to_number(val->type())) return nullptr;
  // May throw
  auto const dbl = gen(env, ConvCellToDbl, make_opt_catch(env, params), val);
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

  env.irb->constrainValue(value, DataTypeSpecific);

  if (value->isA(TInt))  return value;
  if (value->isA(TNull)) return cns(env, staticEmptyString());
  if (value->isA(TBool)) return gen(env, ConvBoolToInt, value);
  if (value->isA(TDbl))  return gen(env, ConvDblToInt, value);
  if (value->isA(TRes))  return gen(env, ConvResToInt, value);
  if (value->isA(TStr))  return gen(env, StrictlyIntegerConv, value);

  return nullptr;
}

SSATmp* opt_type_structure(IRGS& env, const ParamPrep& params) {
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

SSATmp* opt_is_list_like(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const type = params[0].value->type();
  // Type might be a Ptr here, so the maybe() below will go wrong if we don't
  // bail out here.
  if (!(type <= TInitCell)) return nullptr;
  if (type <= TClsMeth) return cns(env, true);
  if (!type.maybe(TArrLike)) return cns(env, false);
  if (type <= TVec || type <= Type::Array(ArrayData::kPackedKind)) {
    return cns(env, true);
  }

  return nullptr;
}

SSATmp* opt_foldable(IRGS& env,
                     const Func* func,
                     const ParamPrep& params,
                     uint32_t numNonDefaultArgs) {
  if (!func->isFoldable()) return nullptr;

  const Class* cls = nullptr;
  if (func->isMethod()) {
    if (!params.thiz || !func->isStatic()) return nullptr;
    cls = params.thiz->type().clsSpec().exactCls();
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
                                        nullptr, nullptr,
                                        ExecutionContext::InvokeNormal,
                                        !func->unit()->useStrictTypes(),
                                        false);
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
      case KindOfPersistentShape:
      case KindOfShape:
        return cns(
          env,
          make_tv<KindOfPersistentShape>(scalar_array())
        );
      case KindOfPersistentArray:
      case KindOfArray:
        return cns(
          env,
          make_tv<KindOfPersistentArray>(scalar_array())
        );
      case KindOfUninit:
      case KindOfObject:
      case KindOfResource:
      case KindOfRef:
      // TODO (T29639296)
      case KindOfFunc:
      case KindOfClass:
      case KindOfClsMeth:
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
  if (type <= TVec || type <= Type::Array(ArrayData::kPackedKind)) {
    auto const r = gen(env, VecFirst, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TDict || type <= Type::Array(ArrayData::kMixedKind)) {
    auto const r = gen(env, DictFirst, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TKeyset) {
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
  if (type <= TVec || type <= Type::Array(ArrayData::kPackedKind)) {
    auto const r = gen(env, VecLast, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TDict || type <= Type::Array(ArrayData::kMixedKind)) {
    auto const r = gen(env, DictLast, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TKeyset) {
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

  if (type <= TVec || type <= Type::Array(ArrayData::kPackedKind)) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = type <= TVec ?
          gen(env, CountVec, value) : gen(env, CountArray, value);
        gen(env, JmpZero, taken, length);
      },
      [&] {
        return cns(env, 0);
       },
      [&] {
        return cns(env, TInitNull);
       }
    );
  }
  if (type <= TDict || type <= Type::Array(ArrayData::kMixedKind)) {
    auto const r = gen(env, DictFirstKey, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TKeyset) {
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

  if (type <= TVec || type <= Type::Array(ArrayData::kPackedKind)) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = type <= TVec ?
          gen(env, CountVec, value) : gen(env, CountArray, value);
        gen(env, JmpZero, taken, length);
        return length;
      },
      [&] (SSATmp* next) {
        return gen(env, SubInt, next, cns(env, 1));
       },
      [&] {
        return cns(env, TInitNull);
       }
    );
  }
  if (type <= TDict || type <= Type::Array(ArrayData::kMixedKind)) {
    auto const r = gen(env, DictLastKey, value);
    gen(env, IncRef, r);
    return r;
  }
  if (type <= TKeyset) {
    auto const r = gen(env, KeysetLast, value);
    gen(env, IncRef, r);
    return r;
  }
  return nullptr;
}

namespace {
const StaticString s_CLASS_CONVERSION("Class to string conversion");
const StaticString s_FUNC_CONVERSION("Func to string conversion");
}

SSATmp* opt_class_meth_get_class(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  auto const type = value->type();
  if (type <= TClsMeth) {
    if (RuntimeOption::EvalRaiseClassConversionWarning) {
      gen(env, RaiseNotice, cns(env, s_CLASS_CONVERSION.get()));
    }
    return gen(env, LdClsName, gen(env, LdClsFromClsMeth, value));
  }
  return nullptr;
}

SSATmp* opt_class_meth_get_method(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  auto const type = value->type();
  if (type <= TClsMeth) {
    if (RuntimeOption::EvalRaiseFuncConversionWarning) {
      gen(env, RaiseNotice, cns(env, s_FUNC_CONVERSION.get()));
    }
    return gen(env, LdFuncName, gen(env, LdFuncFromClsMeth, value));
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

SSATmp* optimizedFCallBuiltin(IRGS& env,
                              const Func* func,
                              const ParamPrep& params,
                              uint32_t numNonDefault) {
  auto const result = [&]() -> SSATmp* {

    auto const fname = func->fullName();

    if (auto const retVal = opt_foldable(env, func, params, numNonDefault)) {
      return retVal;
    }
#define X(x) \
    if (fname->isame(s_##x.get())) return opt_##x(env, params);

    X(get_class)
    X(in_array)
    X(ini_get)
    X(count)
    X(sizeof)
    X(is_a)
    X(is_subclass_of)
    X(method_exists)
    X(sqrt)
    X(strlen)
    X(clock_gettime_ns)
    X(microtime)
    X(max2)
    X(ceil)
    X(floor)
    X(abs)
    X(ord)
    X(chr)
    X(min2)
    X(array_key_cast)
    X(type_structure)
    X(is_list_like)
    X(container_first)
    X(container_last)
    X(container_first_key)
    X(container_last_key)
    X(class_meth_get_class)
    X(class_meth_get_method)

#undef X

    return nullptr;
  }();

  if (result == nullptr) return nullptr;

  // NativeImpl will do a RetC
  if (!params.forNativeImpl) {
    if (params.thiz && params.thiz->type() <= TObj) {
      decRef(env, params.thiz);
    }

    // Decref and free args
    for (int i = numNonDefault - 1; i >= 0; --i) {
      decRef(env, params[i].value);
    }
  }

  return result;
}

//////////////////////////////////////////////////////////////////////

/*
 * Return the type that a parameter to a builtin function is supposed to be
 * coerced to.  What this means depends on how the builtin is dealing with
 * parameter coersion: new-style HNI builtins try to do a tvCoerceParamTo*,
 * while older ones use tvCastTo* semantics.
 *
 * If the builtin parameter has no type hints to cause coercion, this function
 * returns TBottom.
 */
Type param_coerce_type(const Func* callee, uint32_t paramIdx) {
  auto const& pi = callee->params()[paramIdx];
  auto const& tc = pi.typeConstraint;
  if (tc.isNullable() && !callee->byRef(paramIdx)) {
    auto const dt = tc.underlyingDataType();
    if (!dt) return TBottom;
    return TNull | Type(*dt);
  }
  if (callee->byRef(paramIdx) && pi.nativeArg) {
    return TBoxedCell;
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
prepare_params(IRGS& /*env*/, const Func* callee, SSATmp* thiz,
               SSATmp* numArgsExpr, uint32_t numArgs, uint32_t numNonDefault,
               bool forNativeImpl, LoadParam loadParam) {
  auto ret = ParamPrep(numArgs);
  ret.thiz = thiz;
  ret.count = numArgsExpr;
  ret.forNativeImpl = forNativeImpl;

  // Fill in in reverse order, since they may come from popC's (depending on
  // what loadParam wants to do).
  for (auto offset = uint32_t{numArgs}; offset-- > 0;) {
    auto const ty = param_coerce_type(callee, offset);
    auto& cur = ret[offset];
    auto& pi = callee->params()[offset];

    cur.value = loadParam(offset, ty);
    cur.isOutputArg = pi.nativeArg && ty == TBoxedCell;
    // If ty > TBottom, it had some kind of type hint.
    // A by-reference parameter thats defaulted will get a plain
    // value (typically null), rather than a BoxedCell; so we still
    // need to apply a conversion there.
    cur.needsConversion = cur.isOutputArg ||
      (offset < numNonDefault && ty > TBottom);
    // We do actually mean exact type equality here.  We're only capable of
    // passing the following primitives through registers; everything else goes
    // by address unless its flagged "nativeArg".
    if (ty == TBool || ty == TInt || ty == TDbl || pi.nativeArg) {
      continue;
    }

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
 *      This means the unwinder is going to see our PC as equal to the FCall
 *      for the call to the function, which will be inside the FPI region for
 *      the call, so it'll try to pop an ActRec, so we'll need to reconstruct
 *      one for it during unwinding.
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

  explicit CatchMaker(IRGS& env,
                      Kind kind,
                      const Func* callee,
                      const ParamPrep* params)
    : env(env)
    , m_kind(kind)
    , m_callee(callee)
    , m_params(*params)
  {
    assertx(!m_params.thiz || m_params.forNativeImpl || inlining());
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
    auto const exit = defBlock(env, Block::Hint::Unlikely);
    BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
    gen(env, BeginCatch);
    decRefParams();
    prepareForCatch();
    gen(env, EndCatch,
        IRSPRelOffsetData { spOffBCFromIRSP(env) },
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
    if (inlining()) {
      fpushActRec(env,
                  cns(env, m_callee),
                  m_params.thiz ? m_params.thiz : cns(env, TNullptr),
                  m_params.size(),
                  nullptr,
                  /* This can be a lie, but we only care about the dynamic flag
                   * in prologues, so its value doesn't matter here. */
                  cns(env, false));
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
  const Func* m_callee;
  const ParamPrep& m_params;
};

//////////////////////////////////////////////////////////////////////

SSATmp* coerce_value(IRGS& env,
                     const Type& ty,
                     const Func* callee,
                     SSATmp* oldVal,
                     uint32_t paramIdx,
                     const CatchMaker& maker) {
  auto const result = [&] () -> SSATmp* {
    if (!callee->isParamCoerceMode()) {
      if (ty <= TInt) {
        return gen(env, ConvCellToInt, maker.makeUnusualCatch(), oldVal);
      }
      if (ty <= TDbl) {
        return gen(env, ConvCellToDbl, maker.makeUnusualCatch(), oldVal);
      }
      if (ty <= TBool) {
        return gen(env, ConvCellToBool, oldVal);
      }

      always_assert(false);
    }

    if (ty <= TInt) {
      return gen(env,
                 CoerceCellToInt,
                 FuncArgData(callee, paramIdx + 1),
                 maker.makeUnusualCatch(),
                 oldVal);
    }
    if (ty <= TDbl) {
      return gen(env,
                 CoerceCellToDbl,
                 FuncArgData(callee, paramIdx + 1),
                 maker.makeUnusualCatch(),
                 oldVal);
    }
    if (ty <= TBool) {
      return gen(env,
                 CoerceCellToBool,
                 FuncArgData(callee, paramIdx + 1),
                 maker.makeUnusualCatch(),
                 oldVal);
    }

    return nullptr;
  }();

  if (result) {
    decRef(env, oldVal);
    return result;
  }

  always_assert(ty.subtypeOfAny(TArr, TStr, TObj, TRes, TDict, TKeyset, TVec) &&
                callee->params()[paramIdx].nativeArg);
  auto const misAddr = gen(env, LdMIStateAddr,
                           cns(env, offsetof(MInstrState, tvBuiltinReturn)));
  gen(env, StMem, misAddr, oldVal);
  gen(env, CoerceMem, ty,
      CoerceMemData { callee, paramIdx + 1 },
      maker.makeUnusualCatch(), misAddr);
  return gen(env, LdMem, ty, misAddr);
}

void coerce_stack(IRGS& env,
                  const Type& ty,
                  const Func* callee,
                  uint32_t paramIdx,
                  BCSPRelOffset offset,
                  const CatchMaker& maker) {
  if (callee->isParamCoerceMode()) {
    always_assert(ty.isKnownDataType());
    gen(env,
        CoerceStk,
        ty,
        CoerceStkData { offsetFromIRSP(env, offset), callee, paramIdx + 1 },
        maker.makeUnusualCatch(),
        sp(env));
  } else {
    always_assert(ty.isKnownDataType() || ty <= TNullableObj);
    gen(env,
        CastStk,
        ty,
        IRSPRelOffsetData { offsetFromIRSP(env, offset) },
        maker.makeUnusualCatch(),
        sp(env));
  }

  /*
   * We can throw after writing to the stack above; inform IRBuilder about it.
   * This is basically just for assertions right now.
   */
  env.irb->exceptionStackBoundary();
}

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
    auto const convertTy =
      (!callee->isParamCoerceMode() &&
       targetTy == TNullableObj) ? targetTy : baseTy;

    if (auto const value = cond(
          env,
          [&] (Block* convert) -> SSATmp* {
            if (targetTy == baseTy ||
                !callee->isParamCoerceMode()) {
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

/*
 * Prepare the actual arguments to the CallBuiltin instruction, by converting a
 * ParamPrep into a vector of SSATmps to pass to CallBuiltin.  If any of the
 * parameters needed type conversions, we need to do that here too.
 */
jit::vector<SSATmp*> realize_params(IRGS& env,
                                    const Func* callee,
                                    ParamPrep& params,
                                    const CatchMaker& maker) {
  auto const cbNumArgs = 2 + params.size() +
    (params.thiz ? 1 : 0) + (params.count ? 1 : 0);
  auto ret = jit::vector<SSATmp*>(cbNumArgs);
  auto argIdx = uint32_t{0};
  ret[argIdx++] = fp(env);
  ret[argIdx++] = sp(env);
  if (params.thiz) ret[argIdx++] = params.thiz;
  if (params.count) ret[argIdx++] = params.count;

  assertx(!params.count || callee->takesNumArgs());

  auto const needDVCheck = [&](uint32_t param, const Type& ty) {
    if (!RuntimeOption::EvalHackArrCompatTypeHintNotices) return false;
    if (!callee->params()[param].typeConstraint.isArray()) return false;
    return ty <= TArr;
  };

  auto const dvCheck = [&](uint32_t param, SSATmp* val) {
    assertx(needDVCheck(param, val->type()));
    auto const& tc = callee->params()[param].typeConstraint;

    auto const check = [&](Block* taken) {
      if (tc.isVArray()) return gen(env, CheckVArray, taken, val);
      if (tc.isDArray()) return gen(env, CheckDArray, taken, val);
      if (tc.isVArrayOrDArray()) {
        return gen(env, JmpZero, taken, gen(env, IsDVArray, val));
      }
      return gen(env, JmpNZero, taken, gen(env, IsDVArray, val));
    };

    ifThen(
      env,
      check,
      [&]{
        gen(
          env,
          RaiseHackArrParamNotice,
          RaiseHackArrParamNoticeData { tc.type(), int32_t(param), false },
          maker.makeUnusualCatch(),
          val,
          cns(env, callee)
        );
      }
    );
  };

  DEBUG_ONLY auto seenBottom = false;
  DEBUG_ONLY auto usedStack = false;
  auto stackIdx = uint32_t{0};
  for (auto paramIdx = uint32_t{0}; paramIdx < params.size(); ++paramIdx) {
    auto& param = params[paramIdx];
    auto const targetTy = param_coerce_type(callee, paramIdx);

    seenBottom |= (param.value->type() == TBottom);

    if (param.value->type() <= TPtrToGen) {
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) -> SSATmp* {
          gen(env, CheckTypeMem, ty, fail, param.value);
          if (needDVCheck(paramIdx, ty)) {
            dvCheck(paramIdx, gen(env, LdMem, ty, param.value));
          }
          return param.isOutputArg ?
            gen(env, LdMem, TBoxedCell, param.value) : nullptr;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          if (param.isOutputArg) {
            return cns(env, TNullptr);
          }
          if (callee->isParamCoerceMode()) {
            gen(env, CoerceMem, ty, CoerceMemData { callee, paramIdx + 1 },
                maker.makeUnusualCatch(), param.value);
          } else {
            gen(env, CastMem, ty, maker.makeUnusualCatch(), param.value);
          }
          return nullptr;
        },
        [&] {
          if (!param.passByAddr && !param.isOutputArg) {
            assertx(targetTy == TBool ||
                    targetTy == TInt ||
                    targetTy == TDbl ||
                    callee->params()[paramIdx].nativeArg);
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
        [&] (const Type& ty) {
          if (param.isOutputArg) return cns(env, TNullptr);
          return coerce_value(env, ty, callee, oldVal, paramIdx, maker);
        },
        [&] {
          /*
           * This gets tricky:
           *   - if we had a ref-counted type, and it was converted
           *     to a Bool, Int or Dbl above, we explicitly DecReffed it
           *     (in coerce_value).
           *   - if we had a non-RefData nativeArg, we did a CoerceMem
           *     which implicitly DecReffed the old value
           * In either case, the old value is taken care of, and any future
           * DecRefs (from exceptions, or after the call on the normal flow
           * of execution) should DecRef param.value (ie the post-coercion
           * value).
           *
           * But if we had an OutputArg, we did not DecRef the old value,
           * and the post-coercion value is a RefData* or nullptr.
           * If its a RefData*, we need to DecRef that - but in that case
           * the new value is the same as the old.
           * If its Nullptr, we need to DecRef the old value.
           *
           * So in both cases we actually want to DecRef the *old* value, so
           * we have to restore it here (because realize_param replaced it
           * with the new value).
           */
          auto v = param.value;
          if (param.isOutputArg) {
            param.value = oldVal;
          }
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
        coerce_stack(env, ty, callee, paramIdx, offset, maker);
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

SSATmp* builtinCall(IRGS& env,
                    const Func* callee,
                    ParamPrep& params,
                    int32_t numNonDefault,
                    const CatchMaker& catchMaker) {
  // Try to replace the builtin call with a specialized implementation of the
  // builtin
  auto optRet = optimizedFCallBuiltin(env, callee, params, numNonDefault);
  if (optRet) return optRet;

  if (!params.forNativeImpl) {
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

  // Make the actual call.
  auto realized = realize_params(env, callee, params, catchMaker);
  SSATmp** const decayedPtr = &realized[0];
  auto const ret = gen(
    env,
    CallBuiltin,
    CallBuiltinData {
      spOffBCFromIRSP(env),
      callee,
      params.count ? -1 : numNonDefault,
      funcReadsLocals(callee),
      funcNeedsCallerFrame(callee)
    },
    catchMaker.makeUnusualCatch(),
    std::make_pair(realized.size(), decayedPtr)
  );

  if (!params.forNativeImpl) {
    if (params.thiz && params.thiz->type() <= TObj) {
      decRef(env, params.thiz);
    }
    catchMaker.decRefParams();
  }

  return ret;
}

/*
 * When we're inlining a NativeImpl opcode, we know this is the only opcode in
 * the callee method body aside from AssertRATs (bytecode invariant).  So in
 * order to make sure we can eliminate the SpillFrame, we do the CallBuiltin
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
  auto const paramThis = [&] () -> SSATmp* {
    if (!callee->isMethod()) return nullptr;
    auto ctx = ldCtx(env);
    if (callee->isStatic()) return gen(env, LdClsCtx, ctx);
    return castCtxThis(env, ctx);
  }();

  auto numNonDefault = fp(env)->inst()->extra<DefInlineFP>()->numNonDefault;
  auto params = prepare_params(
    env,
    callee,
    paramThis,
    nullptr,
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
    callee,
    &params
  };

  push(env, builtinCall(env, callee, params, numNonDefault, catcher));
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

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
                      const StringData* funcName) {
  auto const callee = Unit::lookupBuiltin(funcName);

  if (!callee) PUNT(Missing-builtin);
  emitCallerRxChecks(env, callee, /* unused, known callee */ IRSPRelOffset {});

  auto params = prepare_params(
    env, callee,
    nullptr, // no $this; FCallBuiltin never happens for methods
    nullptr, // count is constant numNonDefault
    numArgs, numNonDefault, false, [&](uint32_t /*i*/, const Type ty) {
      auto specificity =
        ty == TBottom ? DataTypeGeneric : DataTypeSpecific;
      return pop(env, specificity);
    });

  auto const catcher = CatchMaker {
    env,
    CatchMaker::Kind::NotInlining,
    callee,
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
    auto const data = RetCtrlData { offsetToReturnSlot(env), false };
    gen(env, RetCtrl, data, sp(env), fp(env), retVal);
  };

  if (!callee->nativeFuncPtr()) {
    genericNativeImpl();
    return;
  }

  auto thiz = callee->isMethod() ? ldCtx(env) : nullptr;
  auto const numParams = gen(env, LdARNumParams, fp(env));

  ifThenElse(
    env,
    [&] (Block* fallback) {
      if (thiz) {
        if (!hasThis(env)) {
          thiz = gen(env, LdClsCtx, thiz);
        } else {
          thiz = castCtxThis(env, thiz);
        }
      }

      auto maxArgs = callee->numParams();
      auto minArgs = callee->numNonVariadicParams();
      while (minArgs) {
        auto const& pi = callee->params()[minArgs - 1];
        if (pi.funcletOff == InvalidAbsoluteOffset) {
          break;
        }
        --minArgs;
      }
      if (callee->hasVariadicCaptureParam()) {
        if (minArgs) {
          auto const check = gen(env, LtInt, numParams, cns(env, minArgs));
          gen(env, JmpNZero, fallback, check);
        }
      } else {
        if (minArgs == maxArgs) {
          auto const check = gen(env, EqInt, numParams, cns(env, minArgs));
          gen(env, JmpZero, fallback, check);
        } else {
          if (minArgs) {
            auto const checkMin = gen(env, LtInt,
                                      numParams, cns(env, minArgs));
            gen(env, JmpNZero, fallback, checkMin);
          }
          auto const checkMax = gen(env, GtInt, numParams, cns(env, maxArgs));
          gen(env, JmpNZero, fallback, checkMax);
        }
      }
    },
    [&] {
      auto params = prepare_params(
        env,
        callee,
        thiz,
        callee->takesNumArgs() ? numParams : nullptr,
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
        callee,
        &params
      };

      auto const ret = builtinCall(env, callee, params,
                                   callee->numParams(), catcher);
      push(env, ret);
      emitRetC(env);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      genericNativeImpl();
    }
  );
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

Type builtinReturnType(const Func* builtin) {
  // Why do we recalculate the type here than just using HHBBC's inferred type?
  // Unlike for regular PHP functions, we have access to all the same
  // information that HHBBC does, and the JIT type-system is slightly more
  // expressive. So, by doing it ourself, we can derive a slightly more precise
  // type.
  assertx(builtin->isCPPBuiltin());

  // NB: It is *not* safe to be pessimistic here and return TGen (or any other
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
        if (constraint.isVArray()) return Type::Array(ArrayData::kPackedKind);
        if (constraint.isDArray()) return Type::Array(ArrayData::kMixedKind);
      }
      return Type{*hniType};
    }
    return TInitCell;
  }();

  // "Reference" types (not boxed, types represented by a pointer) can always be
  // null.
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
    [&] (SSATmp* key) {
      return gen(env, ArrayIdx, base, key, def);
    }
  );

  auto finish = [&](SSATmp* tmp) {
    auto const value = unbox(env, tmp, nullptr);
    pushIncRef(env, value);
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
    [&] (SSATmp* key) {
      return gen(env, is_dict ? DictIdx : KeysetIdx, use_base, key, def);
    }
  );

  auto const pelem = profiledType(env, elem, [&] { finish(elem); });
  finish(pelem);
}

const StaticString s_idx("hh\\idx");

void implGenericIdx(IRGS& env) {
  auto const def = popC(env, DataTypeSpecific);
  auto const key = popC(env, DataTypeSpecific);
  auto const base = popC(env, DataTypeSpecific);

  SSATmp* const args[] = { base, key, def };

  static auto func = Unit::lookupBuiltin(s_idx.get());
  assertx(func && func->numParams() == 3);

  emitDirectCall(env, func, 3, args);
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

  auto const simple_key =
    keyType <= TInt || keyType <= TStr;

  if (!simple_key) {
    implGenericIdx(env);
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

  implGenericIdx(env);
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
    if (key->isA(TNull | TStr)) {
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
    if (key->isA(TNull)) {
      push(env, cns(env, false));
      decRef(env, arr);
      decRef(env, key);
      return;
    }
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

void emitGetMemoKeyL(IRGS& env, int32_t locId) {
  DEBUG_ONLY auto const func = curFunc(env);
  assertx(func->isMemoizeWrapper());
  assertx(!func->anyByRef());

  auto const value = ldLocInnerWarn(
    env,
    locId,
    makeExit(env),
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
      auto const lsbCls = gen(env, LdClsCtx, ldCtx(env));
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
      gen(env, JmpNZero, taken, tst);
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
    auto const lsbCls = gen(env, LdClsCtx, ldCtx(env));
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
