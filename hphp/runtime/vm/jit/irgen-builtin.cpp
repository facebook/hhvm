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
#include "hphp/runtime/vm/jit/irgen-builtin.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/func-effects.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-ret.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-types.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"
#include "hphp/runtime/base/file-util.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString
  s_is_a("is_a"),
  s_is_subclass_of("is_subclass_of"),
  s_method_exists("method_exists"),
  s_count("count"),
  s_ini_get("ini_get"),
  s_dirname("dirname"),
  s_86metadata("86metadata"),
  s_set_frame_metadata("hh\\set_frame_metadata"),
  s_in_array("in_array"),
  s_get_class("get_class"),
  s_get_called_class("get_called_class"),
  s_sqrt("sqrt"),
  s_strlen("strlen"),
  s_max2("__SystemLib\\max2"),
  s_min2("__SystemLib\\min2"),
  s_ceil("ceil"),
  s_floor("floor"),
  s_abs("abs"),
  s_ord("ord"),
  s_func_num_args("__SystemLib\\func_num_arg_"),
  s_one("1"),
  s_empty("");

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

SSATmp* is_a_impl(IRGS& env, uint32_t numArgs, bool subclassOnly) {
  if (numArgs != 3) return nullptr;

  auto const allowString = topC(env, BCSPOffset{0});
  auto const classname   = topC(env, BCSPOffset{1});
  auto const obj         = topC(env, BCSPOffset{2});

  if (!obj->isA(TObj) ||
      !classname->hasConstVal(TStr) ||
      !allowString->isA(TBool)) {
    return nullptr;
  }

  auto const objCls = gen(env, LdObjClass, obj);

  SSATmp* testCls = nullptr;
  if (auto const cls = Unit::lookupClassOrUniqueClass(classname->strVal())) {
    if (classIsUniqueOrCtxParent(env, cls)) testCls = cns(env, cls);
  }
  if (testCls == nullptr) return nullptr;

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

SSATmp* opt_is_a(IRGS& env, uint32_t numArgs) {
  return is_a_impl(env, numArgs, false /* subclassOnly */);
}

SSATmp* opt_is_subclass_of(IRGS& env, uint32_t numArgs) {
  return is_a_impl(env, numArgs, true /* subclassOnly */);
}

SSATmp* opt_method_exists(IRGS& env, uint32_t numArgs) {
  if (numArgs != 2) return nullptr;

  auto const meth = topC(env, BCSPOffset{0});
  auto const obj  = topC(env, BCSPOffset{1});

  if (!obj->isA(TObj) || !meth->isA(TStr)) return nullptr;

  auto const cls = gen(env, LdObjClass, obj);
  return gen(env, MethodExists, cls, meth);
}

SSATmp* opt_count(IRGS& env, uint32_t numArgs) {
  if (numArgs != 2) return nullptr;

  auto const mode = topC(env, BCSPOffset{0});
  auto const val = topC(env, BCSPOffset{1});

  // Bail if we're trying to do a recursive count()
  if (!mode->hasConstVal(0)) return nullptr;

  return gen(env, Count, val);
}

SSATmp* opt_ord(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  auto const arg = topC(env, BCSPOffset{0});
  auto const arg_type = arg->type();
  if (arg_type <= TStr) {
    return gen(env, OrdStr, arg);
  }

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

SSATmp* opt_func_num_args(IRGS& env, uint32_t numArgs) {
  if (numArgs != 0 || curFunc(env)->isPseudoMain()) return nullptr;
  return gen(env, LdARNumParams, fp(env));
}

SSATmp* opt_ini_get(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  auto const argType = topType(env, BCSPOffset{0});
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
  // for which the emitter emits an ini_get around every call to assert().
  auto const settingName = top(env,
                               BCSPOffset{0})->strVal()->toCppString();
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

SSATmp* opt_dirname(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  // Only generate the optimized version if the argument passed in is a
  // static string with a constant literal value so we can get the string value
  // at JIT time.
  auto const argType = topType(env, BCSPOffset{0});
  if (!(argType.hasConstVal(TStaticStr))) {
    return nullptr;
  }

  // Return the directory portion of the path
  auto path = top(env, BCSPOffset{0})->strVal();
  auto psize = path->size();
  // Make a mutable copy for dirname_helper to modify
  char *buf = strndup(path->data(), psize);
  int len = FileUtil::dirname_helper(buf, psize);
  SSATmp *ret = cns(env, makeStaticString(buf, len));
  free(buf);
  return ret;
}

/*
 * Transforms in_array with a static haystack argument into an AKExistsArr with
 * the haystack flipped.
 */
SSATmp* opt_in_array(IRGS& env, uint32_t numArgs) {
  if (numArgs != 3) return nullptr;

  // We will restrict this optimization to needles that are strings, and
  // haystacks that have only non-numeric string keys. This avoids a bunch of
  // complication around numeric-string array-index semantics.
  if (!(topType(env, BCSPOffset{2}) <= TStr)) {
    return nullptr;
  }

  auto const haystackType = topType(env, BCSPOffset{1});
  if (!haystackType.hasConstVal(TStaticArr)) {
    // Haystack isn't statically known
    return nullptr;
  }

  auto const haystack = haystackType.arrVal();
  if (haystack->size() == 0) {
    return cns(env, false);
  }

  ArrayInit flipped{haystack->size(), ArrayInit::Map{}};

  for (auto iter = ArrayIter{haystack}; iter; ++iter) {
    auto const& key = iter.secondRef();
    int64_t ignoredInt;
    double ignoredDbl;

    if (!key.isString() ||
        key.asCStrRef().get()
          ->isNumericWithVal(ignoredInt, ignoredDbl, false) != KindOfNull) {
      // Numeric strings will complicate matters because the loose comparisons
      // done with array keys are not quite the same as loose comparisons done
      // by in_array. For example: in_array('0', array('0000')) is true, but
      // doing array('0000' => true)['0'] will say "undefined index". This seems
      // unlikely to affect real-world usage.
      return nullptr;
    }

    flipped.set(key.asCStrRef(), init_null_variant);
  }

  auto const needle = topC(env, BCSPOffset{2});
  auto const array = flipped.toArray();
  return gen(
    env,
    AKExistsArr,
    cns(env, ArrayData::GetScalarArray(array.get())),
    needle
  );
}

SSATmp* opt_get_class(IRGS& env, uint32_t numArgs) {
  auto const curCls = curClass(env);
  auto const curName = [&] {
    return curCls != nullptr ? cns(env, curCls->name()) : nullptr;
  };
  if (numArgs == 0) return curName();
  if (numArgs != 1) return nullptr;

  auto const val = topC(env, BCSPOffset{0});
  auto const ty  = val->type();
  if (ty <= TNull) return curName();
  if (ty <= TObj) {
    auto const cls = gen(env, LdObjClass, val);
    return gen(env, LdClsName, cls);
  }

  return nullptr;
}

SSATmp* opt_get_called_class(IRGS& env, uint32_t numArgs) {
  if (numArgs != 0) return nullptr;
  if (!curClass(env)) return nullptr;
  auto const ctx = ldCtx(env);
  auto const cls = gen(env, LdClsCtx, ctx);
  return gen(env, LdClsName, cls);
}

SSATmp* opt_sqrt(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  auto const val = topC(env);
  auto const ty  = val->type();
  if (ty <= TDbl) return gen(env, Sqrt, val);
  if (ty <= TInt) {
    auto const conv = gen(env, ConvIntToDbl, val);
    return gen(env, Sqrt, conv);
  }
  return nullptr;
}

SSATmp* opt_strlen(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  auto const val = topC(env);
  auto const ty  = val->type();

  if (ty <= TStr) {
    return gen(env, LdStrLen, val);
  }

  if (ty.subtypeOfAny(TNull, TBool)) {
    return gen(env, ConvCellToInt, val);
  }

  if (ty.subtypeOfAny(TInt, TDbl)) {
    auto str = gen(env, ConvCellToStr, val);
    auto len = gen(env, LdStrLen, str);
    decRef(env, str);
    return len;
  }

  return nullptr;
}

SSATmp* minmax(IRGS& env, const bool is_max) {
  auto const val1 = topC(env, BCSPOffset{0});
  auto const ty1 = val1->type();
  auto const val2 = topC(env, BCSPOffset{1});
  auto const ty2 = val2->type();

  // this optimization is only for 2 ints/doubles
  if (!(ty1 <= TInt || ty1 <= TDbl) ||
      !(ty2 <= TInt || ty2 <= TDbl)) return nullptr;

  return cond(
    env,
    [&] (Block* taken) {
      SSATmp* cmp;
      if (ty1 <= TInt && ty2 <= TInt) {
        cmp = gen(env, is_max ? GteInt : LteInt, val1, val2);
      } else {
        auto conv1 = (ty1 <= TDbl) ? val1 : gen(env, ConvIntToDbl, val1);
        auto conv2 = (ty2 <= TDbl) ? val2 : gen(env, ConvIntToDbl, val2);
        cmp = gen(env, is_max ? GteDbl : LteDbl, conv1, conv2);
      }
      gen(env, JmpZero, taken, cmp);
    },
    [&] {
      return val1;
    },
    [&] {
      return val2;
    }
  );
}

SSATmp* opt_max2(IRGS& env, uint32_t numArgs) {
  // max2 is only called for 2 operands
  return numArgs == 2 ? minmax(env, true) : nullptr;
}

SSATmp* opt_min2(IRGS& env, uint32_t numArgs) {
  // min2 is only called for 2 operands
  return numArgs == 2 ? minmax(env, false) : nullptr;
}

SSATmp* opt_ceil(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = topC(env);
  if (!type_converts_to_number(val->type())) return nullptr;
  auto const dbl = gen(env, ConvCellToDbl, val);
  return gen(env, Ceil, dbl);
}

SSATmp* opt_floor(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = topC(env);
  if (!type_converts_to_number(val->type())) return nullptr;
  auto const dbl = gen(env, ConvCellToDbl, val);
  return gen(env, Floor, dbl);
}

SSATmp* opt_abs(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;

  auto const value = topC(env);
  if (value->type() <= TInt) {
    // compute integer absolute value ((src>>63) ^ src) - (src>>63)
    auto const t1 = gen(env, Shr, value, cns(env, 63));
    auto const t2 = gen(env, XorInt, t1, value);
    return gen(env, SubInt, t2, t1);
  }

  if (value->type() <= TDbl) return gen(env, AbsDbl, value);
  if (value->type() <= TArr) return cns(env, false);

  return nullptr;
}

SSATmp* opt_set_frame_metadata(IRGS& env, uint32_t numArgs) {
  if (numArgs != 1) return nullptr;
  auto func = curFunc(env);
  if (func->isPseudoMain() || (func->attrs() & AttrMayUseVV)) return nullptr;
  auto const local = func->lookupVarId(s_86metadata.get());
  if (local == kInvalidId) return nullptr;
  auto oldVal = ldLoc(env, local, nullptr, DataTypeCountness);
  auto newVal = topC(env);
  stLocRaw(env, local, fp(env), newVal);
  decRef(env, oldVal);
  gen(env, IncRef, newVal);
  return cns(env, TInitNull);
}

//////////////////////////////////////////////////////////////////////

bool optimizedFCallBuiltin(IRGS& env,
                           const Func* func,
                           uint32_t numArgs,
                           uint32_t numNonDefault) {
  auto const result = [&]() -> SSATmp* {

    auto const fname = func->name();
#define X(x) \
    if (fname->isame(s_##x.get())) return opt_##x(env, numArgs);

    X(get_called_class)
    X(get_class)
    X(in_array)
    X(ini_get)
    X(count)
    X(is_a)
    X(is_subclass_of)
    X(method_exists)
    X(sqrt)
    X(strlen)
    X(max2)
    X(ceil)
    X(floor)
    X(abs)
    X(ord)
    X(func_num_args)
    X(max2)
    X(min2)
    X(dirname)
    X(set_frame_metadata)

#undef X

    return nullptr;
  }();

  if (result == nullptr) return false;

  // Decref and free args
  for (int i = 0; i < numArgs; i++) {
    auto const arg = popR(env);
    if (i >= numArgs - numNonDefault) {
      decRef(env, arg);
    }
  }

  push(env, result);
  return true;
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
  if (!pi.builtinType) return TBottom;
  if (pi.builtinType == KindOfObject &&
      pi.defaultValue.m_type == KindOfNull) {
    return TNullableObj;
  }
  return pi.builtinType ? Type(*pi.builtinType) : TBottom;
}

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

  // if set, coerceFailure determines the target of a failed coercion;
  // if not set, we side-exit to the next byte-code instruction (only
  //   applies to an inlined NativeImpl, or an FCallBuiltin).
  Block* coerceFailure{nullptr};
  bool forNativeImpl{false};
};

/*
 * Collect parameters for a call to a builtin.  Also determine which ones will
 * need to be passed through the eval stack, and which ones will need
 * conversions.
 */
template<class LoadParam>
ParamPrep prepare_params(IRGS& env,
                         const Func* callee,
                         SSATmp* thiz,
                         SSATmp* numArgsExpr,
                         uint32_t numArgs,
                         uint32_t numNonDefault,
                         Block* coerceFailure,
                         LoadParam loadParam) {
  auto ret = ParamPrep(numArgs);
  ret.thiz = thiz;
  ret.count = numArgsExpr;
  ret.coerceFailure = coerceFailure;
  ret.forNativeImpl = coerceFailure != nullptr;

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
 *      This means the unwinder is going to see our PC as equal to the FCallD
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
    decRefForUnwind();
    prepareForCatch();
    gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
      fp(env), sp(env));
    return exit;
  }

  Block* makeParamCoerceCatch() const {
    auto const exit = defBlock(env, Block::Hint::Unlikely);

    BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
    gen(env, BeginCatch);

    // Determine whether we're dealing with a TVCoercionException or a php
    // exception.  If it's a php-exception, we'll go to the taken block.
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
      },
      [&] {
        hint(env, Block::Hint::Unused);
        decRefForUnwind();
        prepareForCatch();
        gen(env, EndCatch,
          IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
          fp(env), sp(env));
      }
    );

    // prepareForCatch() in the ifThen() above messed with irb's marker, so we
    // have to update it on the fallthru path here.
    updateMarker(env);

    if (m_params.coerceFailure) {
      gen(env, Jmp, m_params.coerceFailure);
    } else {
      // From here on we're on the side-exit path, due to a failure to coerce.
      // We need to push the unwinder value and then side-exit to the next
      // instruction.
      hint(env, Block::Hint::Unlikely);
      decRefForSideExit();
      if (m_params.thiz && m_params.thiz->type() <= TObj) {
        decRef(env, m_params.thiz);
      }

      auto const val = gen(env, LdUnwinderValue, TCell);
      push(env, val);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }

    return exit;
  }

  void decRefByPopping() const {
    decRefForUnwind();
  }

private:
  void prepareForCatch() const {
    if (inlining()) {
      fpushActRec(env,
                  cns(env, m_callee),
                  m_params.thiz ? m_params.thiz : cns(env, TNullptr),
                  m_params.size(),
                  nullptr);
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
    auto const spOff = IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) };
    gen(env, EagerSyncVMRegs, spOff, fp(env), sp(env));
    updateMarker(env);  // Mark the EndCatch safe, since we're eager syncing.
  }

  /*
   * For consistency with the interpreter, we need to decref things in a
   * different order depending on whether we are unwinding, or planning to side
   * exit.
   *
   * In either case, parameters that are not being passed through the stack
   * still may need to be decref'd, because they may have been a reference
   * counted type that was going to be converted to a non-reference counted
   * type that we'd pass in a register.  As we do the coersions, params.value
   * gets updated so whenever we call these catch block creation functions it
   * will only decref things that weren't yet converted.
   */

  void decRefForUnwind() const {
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

  // Same work as above, but opposite order.
  void decRefForSideExit() const {
    assertx(!m_params.forNativeImpl);
    int32_t stackIdx = safe_cast<int32_t>(m_params.numByAddr);

    // Make sure we have loads for all of the stack elements.  We need to do
    // this in forward order before we decref in backward order because
    // extendStack will end up with values that are of type StkElem
    // TODO(#6156498).
    for (auto i = 0; i < stackIdx; ++i) {
      top(env, BCSPOffset{i}, DataTypeGeneric);
    }

    for (auto i = m_params.size(); i-- > 0;) {
      if (m_params[i].passByAddr) {
        --stackIdx;
        auto const val = top(env, BCSPOffset{stackIdx}, DataTypeGeneric);
        decRef(env, val);
      } else {
        decRef(env, m_params[i].value);
      }
    }
    discard(env, m_params.numByAddr);
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
                 maker.makeParamCoerceCatch(),
                 oldVal);
    }
    if (ty <= TDbl) {
      return gen(env,
                 CoerceCellToDbl,
                 FuncArgData(callee, paramIdx + 1),
                 maker.makeParamCoerceCatch(),
                 oldVal);
    }
    if (ty <= TBool) {
      return gen(env,
                 CoerceCellToBool,
                 FuncArgData(callee, paramIdx + 1),
                 maker.makeParamCoerceCatch(),
                 oldVal);
    }

    return nullptr;
  }();

  if (result) {
    decRef(env, oldVal);
    return result;
  }

  always_assert(ty.subtypeOfAny(TArr, TStr, TObj, TRes) &&
                callee->params()[paramIdx].nativeArg);
  auto const misAddr = gen(env, LdMIStateAddr,
                           cns(env, offsetof(MInstrState, tvBuiltinReturn)));
  gen(env, StMem, misAddr, oldVal);
  gen(env, CoerceMem, ty,
      CoerceMemData { callee, paramIdx + 1 },
      maker.makeParamCoerceCatch(), misAddr);
  return gen(env, LdMem, ty, misAddr);
}

void coerce_stack(IRGS& env,
                  const Type& ty,
                  const Func* callee,
                  uint32_t paramIdx,
                  BCSPOffset offset,
                  const CatchMaker& maker) {
  if (callee->isParamCoerceMode()) {
    always_assert(ty.isKnownDataType());
    gen(env,
        CoerceStk,
        ty,
        CoerceStkData { offsetFromIRSP(env, offset), callee, paramIdx + 1 },
        maker.makeParamCoerceCatch(),
        sp(env));
  } else {
    always_assert(ty.isKnownDataType() || ty <= TNullableObj);
    gen(env,
        CastStk,
        ty,
        IRSPOffsetData { offsetFromIRSP(env, offset) },
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
    assert(baseTy.isKnownDataType());
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

  assertx(!params.count || callee->attrs() & AttrNumArgs);

  DEBUG_ONLY auto usedStack = false;
  auto stackIdx = uint32_t{0};
  for (auto paramIdx = uint32_t{0}; paramIdx < params.size(); ++paramIdx) {
    auto& param = params[paramIdx];
    auto const targetTy = param_coerce_type(callee, paramIdx);

    if (param.value->type() <= TPtrToGen) {
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) -> SSATmp* {
          gen(env, CheckTypeMem, ty, fail, param.value);
          return param.isOutputArg ?
            gen(env, LdMem, TBoxedCell, param.value) : nullptr;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          if (param.isOutputArg) {
            return cns(env, TNullptr);
          }
          if (callee->isParamCoerceMode()) {
            gen(env,
                CoerceMem,
                ty,
                CoerceMemData { callee, paramIdx + 1 },
                maker.makeParamCoerceCatch(),
                param.value);
          } else {
            gen(env,
                CastMem,
                ty,
                maker.makeUnusualCatch(),
                param.value);
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
                       targetTy == TBool || targetTy == TBoxedCell ?
                       TInt : targetTy == TBottom ? TCell : targetTy,
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
          return ret;
        },
        [&] (const Type& ty) {
          if (param.isOutputArg) return cns(env, TNullptr);
          return coerce_value(
              env,
              ty,
              callee,
              oldVal,
              paramIdx,
              maker
            );
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
    auto const offset = BCSPOffset{safe_cast<int32_t>(
        params.numByAddr - stackIdx - 1)};

    ret[argIdx++] = realize_param(
      env, param, callee, targetTy,
      [&] (const Type& ty, Block* fail) -> SSATmp* {
        auto irspOff = offsetFromIRSP(env, offset);
        gen(env, CheckStk,
            RelOffsetData { offset, irspOff },
            ty, fail, sp(env));
        env.irb->constrainStack(irspOff, DataTypeSpecific);
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

  assertx(!usedStack || stackIdx == params.numByAddr);
  assertx(argIdx == cbNumArgs);

  return ret;
}

//////////////////////////////////////////////////////////////////////

SSATmp* builtinCall(IRGS& env,
                    const Func* callee,
                    ParamPrep& params,
                    int32_t numNonDefault,
                    const CatchMaker& catchMaker) {
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

  auto const retType = [&]() -> Type {
    auto const retDT = callee->returnType();
    auto const ret = retDT ? Type(*retDT) : TCell;
    if (callee->attrs() & AttrReference) {
      return ret.box() & TBoxedInitCell;
    }
    return ret;
  }();

  // Make the actual call.
  auto realized = realize_params(env, callee, params, catchMaker);
  SSATmp** const decayedPtr = &realized[0];
  auto const ret = gen(
    env,
    CallBuiltin,
    retType,
    CallBuiltinData {
      offsetFromIRSP(env, BCSPOffset{0}),
      callee,
      params.count ? -1 : numNonDefault,
      builtinFuncDestroysLocals(callee),
      builtinFuncNeedsCallerFrame(callee)
    },
    catchMaker.makeUnusualCatch(),
    std::make_pair(realized.size(), decayedPtr)
  );

  if (!params.forNativeImpl) {
    if (params.thiz && params.thiz->type() <= TObj) {
      decRef(env, params.thiz);
    }
    catchMaker.decRefByPopping();
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
    if (callee->isStatic() && !callee->isNative()) return nullptr;
    auto ctx = gen(env, LdCtx, fp(env));
    if (callee->isStatic()) return gen(env, LdClsCtx, ctx);
    return gen(env, CastCtxThis, ctx);
  }();

  auto numNonDefault = fp(env)->inst()->extra<DefInlineFP>()->numNonDefault;
  auto params = prepare_params(
    env,
    callee,
    paramThis,
    nullptr,
    numArgs,
    numNonDefault,
    nullptr,
    [&] (uint32_t i, const Type) {
      auto ret = ldLoc(env, i, nullptr, DataTypeSpecific);
      // These IncRefs must be 'inside' the callee: it may own the only
      // reference to the parameters.  Normally they will cancel with the
      // DecRefs that we'll do in endInlinedCommon.
      gen(env, IncRef, ret);
      return ret;
    }
  );

  // For the same reason that we have to IncRef the locals above, we
  // need to grab one on the $this.
  if (paramThis && paramThis->type() <= TObj) gen(env, IncRef, paramThis);

  endInlinedCommon(env);

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
    if (!env.irb->constrainValue(src, TypeConstraint(cls).setWeak())) {
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
                      int32_t numArgs,
                      int32_t numNonDefault,
                      const StringData* funcName) {
  auto const callee = Unit::lookupFunc(funcName);

  if (optimizedFCallBuiltin(env, callee, numArgs, numNonDefault)) return;

  auto params = prepare_params(
    env,
    callee,
    nullptr,  // no $this; FCallBuiltin never happens for methods
    nullptr,  // count is constant numNonDefault
    numArgs,
    numNonDefault,
    nullptr,
    [&] (uint32_t i, const Type ty) {
      auto specificity =
        ty == TBottom ? DataTypeGeneric : DataTypeSpecific;
      return pop(env, specificity);
    }
  );

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

  auto genericNativeImpl = [&]() {
    gen(env, NativeImpl, fp(env), sp(env));
    auto const data = RetCtrlData { offsetToReturnSlot(env), false };
    gen(env, RetCtrl, data, sp(env), fp(env));
  };

  auto callee = curFunc(env);
  if (!callee->nativeFuncPtr() || callee->builtinFuncPtr() == zend_wrap_func) {
    genericNativeImpl();
    return;
  }

  // CallBuiltin doesn't understand IDL instance methods that have variable
  // arguments.
  if (auto const info = callee->methInfo()) {
    if (info->attribute & (ClassInfo::VariableArguments |
                           ClassInfo::RefVariableArguments)) {
      genericNativeImpl();
      return;
    }
  }

  auto thiz = callee->isMethod() && (!callee->isStatic() || callee->isNative())
    ? gen(env, LdCtx, fp(env)) : nullptr;
  auto const numParams = gen(env, LdARNumParams, fp(env));

  ifThenElse(
    env,
    [&] (Block* fallback) {
      if (thiz) {
        if (callee->isStatic()) {
          thiz = gen(env, LdClsCtx, thiz);
        } else {
          gen(env, CheckCtxThis, fallback, thiz);
          thiz = gen(env, CastCtxThis, thiz);
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
      auto const ret = cond(
        env,
        [&] (Block* fail) {
          auto params = prepare_params(
            env,
            callee,
            thiz,
            callee->attrs() & AttrNumArgs ? numParams : nullptr,
            callee->numParams(),
            callee->numParams(),
            fail,
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

          return builtinCall(env, callee, params,
                             callee->numParams(), catcher);
        },
        [&] (SSATmp* ret) { return ret; },
        [&] {
          return callee->attrs() & AttrParamCoerceModeFalse ?
            cns(env, false) : cns(env, TInitNull);
        });
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

// Helper for doing array-style Idx translations, even if we're dealing with a
// collection.  The stack will still contain the collection in that case, and
// loaded_collection_array will be non-nullptr.  If we're really doing
// ArrayIdx, it's nullptr.
void implArrayIdx(IRGS& env, SSATmp* loaded_collection_array) {
  // These types are just used to decide what to do; once we know what we're
  // actually doing we constrain the values with the popC()s later on in this
  // function.
  auto const keyType = topC(env, BCSPOffset{1}, DataTypeGeneric)->type();

  if (keyType <= TNull) {
    auto const def = popC(env, DataTypeGeneric);
    auto const key = popC(env);
    auto const stack_base = popC(env);

    // if the key is null it will not be found so just return the default
    push(env, def);
    decRef(env, stack_base);
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
  auto const stack_base = popC(env);
  auto const use_base = loaded_collection_array
    ? loaded_collection_array
    : stack_base;
  auto const value = gen(env, ArrayIdx, use_base, key, def);
  push(env, value);
  decRef(env, stack_base);
  decRef(env, key);
  decRef(env, def);
}

void implMapIdx(IRGS& env) {
  auto const def = popC(env);
  auto const key = popC(env);
  auto const map = popC(env);
  auto const val = gen(env, MapIdx, map, key, def);
  push(env, val);
  decRef(env, map);
  decRef(env, key);
  decRef(env, def);
}

void implGenericIdx(IRGS& env) {
  auto const stkptr = sp(env);
  auto const spOff = IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) };
  auto const def = popC(env, DataTypeSpecific);
  auto const key = popC(env, DataTypeSpecific);
  auto const arr = popC(env, DataTypeSpecific);
  push(env, gen(env, GenericIdx, spOff, arr, key, def, stkptr));
  decRef(env, arr);
  decRef(env, key);
  decRef(env, def);
}

TypeConstraint idxBaseConstraint(Type baseType, Type keyType,
                                 bool& useCollection, bool& useMap) {
  if (baseType < TObj && baseType.clsSpec()) {
    auto const cls = baseType.clsSpec().cls();

    // To use ArrayIdx, we require either constant non-int keys or known
    // integer keys for Map, because integer-like strings behave differently.
    auto const isMap      = collections::isType(cls, CollectionType::Map) ||
                            collections::isType(cls, CollectionType::ImmMap);
    auto const isMapOrSet = isMap ||
                            collections::isType(cls, CollectionType::Set) ||
                            collections::isType(cls, CollectionType::ImmSet);
    auto const okMapOrSet = [&]() {
      if (!isMapOrSet) return false;
      if (keyType <= TInt) return true;
      if (!keyType.hasConstVal(TStr)) return false;
      int64_t dummy;
      if (keyType.strVal()->isStrictlyInteger(dummy)) return false;
      return true;
    }();
    // Similarly, Vector is only usable with int keys, so we can only do this
    // for Vector if it's an Int.
    auto const isVector = collections::isType(cls, CollectionType::Vector) ||
                          collections::isType(cls, CollectionType::ImmVector);
    auto const okVector = isVector && keyType <= TInt;

    useCollection = okMapOrSet || okVector;
    // If it's a map with a non-static string key, we can do a map-specific
    // optimization.
    useMap = isMap && keyType <= TStr;

    if (useCollection || useMap) return TypeConstraint(cls);
  }

  useCollection = useMap = false;
  return DataTypeSpecific;
}

//////////////////////////////////////////////////////////////////////

}

TypeConstraint idxBaseConstraint(Type baseType, Type keyType) {
  bool collection, map;
  return idxBaseConstraint(baseType, keyType, collection, map);
}

void emitArrayIdx(IRGS& env) {
  auto const arrType = topC(env, BCSPOffset{2}, DataTypeGeneric)->type();
  if (!(arrType <= TArr)) {
    // raise fatal
    interpOne(env, TCell, 3);
    return;
  }

  implArrayIdx(env, nullptr);
}

void emitIdx(IRGS& env) {
  auto const key      = topC(env, BCSPOffset{1}, DataTypeGeneric);
  auto const base     = topC(env, BCSPOffset{2}, DataTypeGeneric);
  auto const keyType  = key->type();
  auto const baseType = base->type();

  auto const simple_key =
    keyType <= TInt || keyType <= TStr;

  if (!simple_key) {
    implGenericIdx(env);
    return;
  }

  if (baseType <= TArr) {
    emitArrayIdx(env);
    return;
  }

  bool useCollection, useMap;
  auto const tc = idxBaseConstraint(baseType, keyType, useCollection, useMap);
  if (useCollection || useMap) {
    env.irb->constrainValue(base, tc);
    env.irb->constrainValue(key, DataTypeSpecific);

    if (useCollection) {
      auto const arr = gen(env, LdColArray, base);
      implArrayIdx(env, arr);
    } else {
      implMapIdx(env);
    }
    return;
  }

  implGenericIdx(env);
}

void emitAKExists(IRGS& env) {
  auto const arr = popC(env);
  auto key = popC(env);

  if (!arr->isA(TArr) && !arr->isA(TObj)) PUNT(AKExists_badArray);

  if (key->isA(TInitNull)) {
    if (arr->isA(TObj)) {
      push(env, cns(env, false));
      decRef(env, arr);
      return;
    }

    key = cns(env, staticEmptyString());
  }

  if (!key->isA(TStr) && !key->isA(TInt)) PUNT(AKExists_badKey);

  auto const val =
    gen(env, arr->isA(TArr) ? AKExistsArr : AKExistsObj, arr, key);
  push(env, val);
  decRef(env, arr);
  decRef(env, key);
}

void emitGetMemoKey(IRGS& env) {
  auto const inTy = topC(env)->type();
  if (inTy <= TInt) {
    // An int is already a valid key. No-op.
    return;
  }
  if (inTy <= TNull) {
    auto input = popC(env);
    push(env, cns(env, s_empty.get()));
    decRef(env, input);
    return;
  }

  auto const obj = popC(env);
  auto const key = gen(env, GetMemoKey, obj);
  push(env, key);
  decRef(env, obj);
}

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
