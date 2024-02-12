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

#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/configs/hhir.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/runtime/base/isame-log.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/containers.h"
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
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"

#include "hphp/util/text-util.h"

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

struct ParamPrep {
  explicit ParamPrep(size_t count, const Func* callee) : info{count} {}

  struct Info {
    SSATmp* value{nullptr};
    // The value with the appropriate ref-iness to pass to the C++ builtin
    SSATmp* argValue{nullptr};
    bool passByAddr{false};
    bool isInOut{false};
  };

  const Info& operator[](size_t idx) const { return info[idx]; }
  Info& operator[](size_t idx) { return info[idx]; }
  size_t size() const { return info.size(); }

  // For free/class/instance methods, ctx is null/Class*/Object* respectively.
  SSATmp* ctx{nullptr};
  jit::vector<Info> info;
  uint32_t numByAddr{0};
};

//////////////////////////////////////////////////////////////////////

// Will turn into either an int or a double in zend_convert_scalar_to_number.
bool type_converts_to_number(Type ty) {
  return ty.subtypeOfAny(TDbl, TInt, TNull, TObj, TRes, TStr, TBool);
}

SSATmp* convToStr(IRGS& env, SSATmp* tmp, char c) {
	switch (c) {
		case 's':
			return gen(env, ConvTVToStr, ConvNoticeData{}, tmp);
		case 'd':
			return gen(
        env,
        ConvTVToStr,
        ConvNoticeData{},
        gen(env, ConvTVToInt, tmp)
			);
		default:
			always_assert(false && "convToStr can't handle non decimal/string format specifiers");
	}
}

/*
 * Warning: The logic for handling the tokens %%, %s in the format string
 * in this function should mimic one in the string_printf implementation in
 * zend-printf.cpp, which is invoked in the slow path. If you make any changes
 * here, please ensure that the two implementations will be in sync.
 */
jit::vector<SSATmp*> tokenize(
    IRGS& env, const StringData* format, SSATmp* args) {
  assertx(args->isA(TVec));

  jit::vector<SSATmp*> result;
  auto const rat = args->type().arrSpec().type();
  if (!rat || rat->tag() != RepoAuthType::Array::Tag::Tuple) {
    return result;
  }

  if (format->empty()) {
    return result;
  }

  const int size = 256 - kStringOverhead;
  static_assert(size >= 0);
  StringBuffer buf(size);
  jit::vector<std::pair<const StringData*, const HPHP::Optional<const char>>> tokens;
  size_t numStrTokens = 0;
  for (auto it = format->slice().begin(); it != format->slice().end(); ++it) {
    char ch = *it;
    if (ch != '%') {
      buf.append(ch);
      continue;
    }

    it++;
    if (it != format->slice().end()) {
      if (*it == '%') {
        buf.append('%');
        continue;
      }

      if (*it != 's' && *it != 'd') {
        return jit::vector<SSATmp*>();
      }

			char spec = *it;
      if (!buf.empty()) {
        tokens.emplace_back(makeStaticString(buf.detach()), std::nullopt);
      }
      // Use a nullptr placeholder that will be replaced with the corresponding
      // token from args
      ++numStrTokens;
      tokens.emplace_back(nullptr, spec);
    }
  }

  if (!buf.empty()) {
    tokens.emplace_back(makeStaticString(buf.detach()), std::nullopt);
  }

  if (rat->size() < numStrTokens) {
    return result;
  }

  result.reserve(tokens.size());
  size_t argIdx = 0;
  for (auto const& kvp : tokens) {
		auto const token = kvp.first;
		auto const& spec = kvp.second;
    if (!token) {
			assertx(spec);
      auto index = env.unit.cns(argIdx++);
      auto elem = gen(env, LdVecElem, args, index);
      result.push_back(convToStr(env, elem, spec.value()));
    } else {
      result.push_back(cns(env, token));
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

SSATmp* is_a_impl(IRGS& env, const ParamPrep& params, bool subclassOnly) {
  auto const nparams = params.size();
  if (nparams != 2 && nparams != 3) return nullptr;

  auto const obj = params[0].value;
  auto const cls = params[1].value;
  auto const allowClass = nparams == 3 ? params[2].value : cns(env, false);

  if (!obj->type().subtypeOfAny(TCls, TObj) ||
      !cls->type().subtypeOfAny(TCls, TLazyCls, TStr) ||
      !allowClass->isA(TBool)) {
    return nullptr;
  }

  auto const lhs = obj->isA(TObj) ? gen(env, LdObjClass, obj) : obj;
  auto const rhs = [&] {
    if (cls->isA(TStr)) {
      return gen(env, LookupClsRDS, cls);
    }
    if (cls->isA(TLazyCls)) {
      auto const cname = gen(env, LdLazyClsName, cls);
      return gen(env, LookupClsRDS, cname);
    }
    return cls;
  }();

  return cond(
    env,
    [&](Block* taken) {
      return gen(env, CheckNonNull, taken, rhs);
    },
    [&](SSATmp* rhs) {
      // is_a() finishes here.
      if (!subclassOnly) return gen(env, InstanceOf, lhs, rhs);

      // is_subclass_of() also needs to check that LHS and RHS don't match.
      return cond(
        env,
        [&](Block* match) {
          auto const eq = gen(env, EqCls, lhs, rhs);
          gen(env, JmpNZero, match, eq);
        },
        [&]{ return gen(env, InstanceOf, lhs, rhs); },
        [&]{ return cns(env, false); }
      );
    },
    [&]{ return cns(env, false); }
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

const StaticString s_conv_clsmeth_to_vec("Implicit clsmeth to vec conversion");

SSATmp* opt_count(IRGS& env, const ParamPrep& params) {
  if (params.size() != 2) return nullptr;

  auto const mode = params[1].value;
  auto const val = params[0].value;

  // Bail if we're trying to do a recursive count()
  if (!mode->hasConstVal(0)) return nullptr;

  // Count may throw
  return gen(env, Count, val);
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

SSATmp* opt_is_numeric(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;

  auto const arg = params[0].value;
  if (!arg->type().maybe(TInt | TDbl | TStr)) return cns(env, false);
  if (arg->isA(TInt | TDbl)) return cns(env, true);
  if (arg->hasConstVal(TStr)) {
    int64_t ival;
    double dval;
    auto const dt = arg->strVal()->toNumeric(ival, dval);
    return cns(env, dt == KindOfInt64 || dt == KindOfDouble);
  }
  return nullptr;
}

SSATmp* opt_hphp_debug_caller_info(IRGS& env, const ParamPrep& params) {
  if (params.size() != 0) return nullptr;

  Array result = empty_dict_array();
  auto skipped = false;
  auto found = false;

  for (auto i = inlineDepth(env); i > 0; i--) {
    auto const sk = env.inlineState.frames[i - 1].callerSk;
    found = hphp_debug_caller_info_impl(
        result, skipped, sk.func(), sk.offset());
    if (found) break;
  }

  if (!found) return nullptr;
  auto const layout = ArrayLayout::FromArray(result.get());
  auto const vad = ArrayData::GetScalarArray(std::move(result));
  auto const bad = layout.apply(vad);
  return cns(env, bad);
}

SSATmp* opt_hphp_debug_caller_identifier(IRGS& env, const ParamPrep& params) {
  if (params.size() != 0) return nullptr;

  String result = empty_string();
  auto skipped = false;
  auto found = false;

  for (auto i = inlineDepth(env); i > 0; i--) {
    auto const sk = env.inlineState.frames[i - 1].callerSk;
    found = hphp_debug_caller_identifier_impl(result, skipped, sk.func());
    if (found) break;
  }

  if (!found) return nullptr;
  return cns(env, makeStaticString(result));
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
  auto const settingName = params[0].value->strVal()->toCppString();
  auto mode = IniSetting::GetMode(settingName);
  if (!mode) {
    return nullptr;
  }
  // If the request can change the value we can't optimize it
  if (IniSetting::canSet(*mode, IniSetting::Mode::Request)) {
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
  if (!haystackType.hasConstVal(TArrLike)) {
    // Haystack isn't statically known
    return nullptr;
  }

  auto const haystack = haystackType.arrLikeVal();
  if (haystack->size() == 0) {
    return cns(env, false);
  }

  KeysetInit flipped{haystack->size()};
  bool failed{false};
  IterateV(
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


  auto const ad = ArrayData::GetScalarArray(flipped.toArray());
  return gen(env, AKExistsKeyset, cns(env, ad), needle);
}

SSATmp* opt_get_class(IRGS& env, const ParamPrep& params) {
  auto const curCls = curClass(env);
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

SSATmp* opt_class_get_class_name(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->type() <= TCls) {
    return gen(env, LdClsName, value);
  }
  if (value->type() <= TLazyCls) {
    return gen(env, LdLazyClsName, value);
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
  auto const dbl = gen(env, ConvTVToDbl, val);
  return gen(env, Ceil, dbl);
}

SSATmp* genConcat(IRGS& env, std::vector<SSATmp*>&& tokens) {
  if (tokens.empty()) {
    return nullptr;
  }

  auto lhs = tokens[0];
  size_t frontier = 1;
  while (frontier < tokens.size()) {
    auto const remaining = tokens.size() - frontier;
    if (remaining < 3) {
      switch(remaining) {
        case 0:
          always_assert(false);
        case 1:
          lhs = gen(env, ConcatStrStr, lhs, tokens[frontier]);
          break;
        case 2:
          lhs = gen(env, ConcatStr3, lhs, tokens[frontier], tokens[frontier+1]);
          break;
      }
      frontier += remaining;
    } else {
      lhs = gen(env, ConcatStr4, lhs, tokens[frontier], tokens[frontier+1], tokens[frontier+2]);
      frontier += 3;
    }
  }

  return lhs;
}

SSATmp* opt_vsprintf_l(IRGS& env, const ParamPrep& params) {
  auto const format = params[1].value;
  auto const args = params[2].value;
  if (!format->hasConstVal(TStr) || !args->isA(TVec)) {
    return nullptr;
  }

  auto tokens = tokenize(env, format->strVal(), args);
  if (tokens.empty()) {
    return nullptr;
  }

  return genConcat(env, std::move(tokens));
}

SSATmp* opt_floor(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  if (!folly::CpuId().sse41()) return nullptr;
  auto const val = params[0].value;
  if (!type_converts_to_number(val->type())) return nullptr;
  // May throw
  auto const dbl = gen(env, ConvTVToDbl, val);
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

  auto const op = "array_key_cast";
  if (value->isA(TInt))  return value;
  if (value->isA(TNull)) return cns(env, staticEmptyString());
  if (value->isA(TBool)) return gen(env, ConvBoolToInt, value);
  if (value->isA(TDbl))  return gen(env, ConvDblToInt, value);
  if (value->isA(TRes))  return gen(env, ConvResToInt, value);
  if (value->isA(TStr))  return gen(env, StrictlyIntegerConv, value);
  if (value->isA(TLazyCls))  {
		if (RO::EvalRaiseClassConversionNoticeSampleRate > 0) {
      std::string msg;
      // TODO(vmladenov) appears untested
      string_printf(msg, Strings::CLASS_TO_STRING_IMPLICIT, op);
      gen(env,
        RaiseNotice,
        SampleRateData { RO::EvalRaiseClassConversionNoticeSampleRate },
        cns(env, makeStaticString(msg)));
    }
    return gen(env, LdLazyClsName, value);
  }
  if (value->isA(TCls))  {
		if (RO::EvalRaiseClassConversionNoticeSampleRate > 0) {
      std::string msg;
      // TODO(vmladenov) appears untested
      string_printf(msg, Strings::CLASS_TO_STRING_IMPLICIT, op);
      gen(env,
          RaiseNotice,
          SampleRateData { RO::EvalRaiseClassConversionNoticeSampleRate },
          cns(env, makeStaticString(msg)));
    }
    return gen(env, LdClsName, value);
  }

  return nullptr;
}

SSATmp* impl_opt_type_structure(IRGS& env, const ParamPrep& params,
                                bool getName, bool noThrow) {
  if (params.size() != 2) return nullptr;
  auto const clsNameTmp = params[0].value;
  auto const cnsNameTmp = params[1].value;

  if (!cnsNameTmp->hasConstVal(TStaticStr)) return nullptr;
  auto const cnsName = cnsNameTmp->strVal();

  auto const clsTmp = [&] () -> SSATmp* {
    if (clsNameTmp->isA(TCls)) return clsNameTmp;
    if (clsNameTmp->isA(TObj)) return gen(env, LdObjClass, clsNameTmp);
    if (clsNameTmp->inst()->is(LdClsName) ||
        clsNameTmp->inst()->is(LdLazyCls)) {
      return clsNameTmp->inst()->src(0);
    }
    if (clsNameTmp->type().subtypeOfAny(TStr, TLazyCls)) {
      return ldCls(env, clsNameTmp);
    }
    return nullptr;
  }();
  if (!clsTmp) return nullptr;

  if (!clsTmp->type().clsSpec()) return nullptr;
  auto const cls = clsTmp->type().clsSpec().cls();
  // Slot indices aren't invariant for non-normal classes.
  if (!isNormalClass(cls)) return nullptr;

  auto const cnsSlot =
    cls->clsCnsSlot(cnsName, ConstModifiers::Kind::Type, true);
  if (cnsSlot == kInvalidSlot) return nullptr;

  if (!getName) {
    return cond(
      env,
      [&] (Block* taken) {
        return gen(
          env,
          LdResolvedTypeCns,
          taken,
          ClsCnsSlotData { cnsName, cnsSlot },
          clsTmp
        );
      },
      [&] (SSATmp* cns) { return cns; },
      [&] {
        hint(env, Block::Hint::Unlikely);
        return gen(
          env,
          noThrow ? LdTypeCnsNoThrow : LdTypeCns,
          clsTmp,
          cnsNameTmp
        );
      }
    );
  }

  assertx(!noThrow);

  return cond(
    env,
    [&] (Block* taken) {
      auto const classname = gen(
        env,
        LdResolvedTypeCnsClsName,
        ClsCnsSlotData { cnsName, cnsSlot },
        clsTmp
      );
      return gen(env, CheckNonNull, taken, classname);
    },
    [&] (SSATmp* s) { return s; },
    [&] {
      hint(env, Block::Hint::Unlikely);
      return gen(
        env,
        LdTypeCnsClsName,
        clsTmp,
        cnsNameTmp
      );
    }
  );
}

SSATmp* opt_type_structure(IRGS& env, const ParamPrep& params) {
  return impl_opt_type_structure(env, params, false, false);
}
SSATmp* opt_type_structure_no_throw(IRGS& env, const ParamPrep& params) {
  return impl_opt_type_structure(env, params, false, true);
}
SSATmp* opt_type_structure_classname(IRGS& env, const ParamPrep& params) {
  return impl_opt_type_structure(env, params, true, false);
}

SSATmp* opt_is_list_like(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const type = params[0].value->type();
  if (!type.maybe(TArrLike)) return cns(env, false);
  if (type <= TVec) return cns(env, true);
  return nullptr;
}

SSATmp* opt_is_vec_or_varray(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const type = params[0].value->type();

  if (type <= TVec) {
    return cns(env, true);
  }

  return nullptr;
}

SSATmp* opt_is_dict_or_darray(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const type = params[0].value->type();

  if (type <= TDict) {
    return cns(env, true);
  }

  if (!type.maybe(TDict)) {
    return cns(env, false);
  }

  return nullptr;
}

SSATmp* opt_foldable(IRGS& env,
                     const Func* func,
                     const ParamPrep& params) {
  if (!func->isFoldable()) return nullptr;

  const Class* cls = nullptr;
  if (func->isMethod()) {
    if (!params.ctx || !func->isStatic()) return nullptr;
    cls = params.ctx->type().clsSpec().exactCls();
    if (!cls) return nullptr;
  }

  ArrayData* variadicArgs = nullptr;
  uint32_t numVariadicArgs = 0;
  auto numNonDefaultArgs = params.size();
  if (numNonDefaultArgs > func->numNonVariadicParams()) {
    assertx(params.size() == func->numParams());
    auto const variadic = params.info.back().value;
    if (!variadic->type().hasConstVal(TVec)) return nullptr;

    variadicArgs = variadic->variantVal().asCArrRef().get();
    numVariadicArgs = variadicArgs->size();

    if (numVariadicArgs && !variadicArgs->isVecType()) return nullptr;

    assertx(variadicArgs->isStatic());
    numNonDefaultArgs = func->numNonVariadicParams();
  }

  // Don't pop the args yet---if the builtin throws at compile time (because
  // it would raise a warning or something at runtime) we're going to leave
  // the call alone.
  VecInit args(numNonDefaultArgs + numVariadicArgs);
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
      args.append(variadicArgs->get(i));
    }
  }

  VMProtect::Pause deprot;
  always_assert(regState() == VMRegState::CLEAN);

  // Even though regState() is marked clean, vmpc() has not necessarily been
  // set to anything valid, so we need to do so here (for assertions and
  // backtraces in the invocation, among other things).
  auto const savedPC = vmpc();
  vmpc() = vmfp() ? vmfp()->func()->entry() : nullptr;
  SCOPE_EXIT{ vmpc() = savedPC; };

  try {
    // We don't know if notices would be enabled or not when this function
    // would normally get called, so be safe and don't optimize any calls that
    // COULD generate notices.
    ThrowAllErrorsSetter taes;

    assertx(!RID().getJitFolding());
    RID().setJitFolding(true);
    SCOPE_EXIT{ RID().setJitFolding(false); };

    auto retVal = g_context->invokeFunc(func, args.toArray(), nullptr,
                                        const_cast<Class*>(cls),
                                        RuntimeCoeffects::fixme(), false);
    SCOPE_EXIT { tvDecRefGen(retVal); };
    assertx(tvIsPlausible(retVal));

    auto scalar_array = [&] {
      auto& a = val(retVal).parr;
      ArrayData::GetScalarArray(&a);
      return a;
    };

    switch (retVal.m_type) {
      case KindOfNull:
      case KindOfBoolean:
      case KindOfInt64:
      case KindOfDouble:
      case KindOfEnumClassLabel:
        return cns(env, retVal);
      case KindOfPersistentString:
      case KindOfString:
        return cns(env, makeStaticString(retVal.m_data.pstr));
      case KindOfPersistentVec:
      case KindOfVec:
        return cns(env, make_tv<KindOfPersistentVec>(scalar_array()));
      case KindOfPersistentDict:
      case KindOfDict:
        return cns(env, make_tv<KindOfPersistentDict>(scalar_array()));
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        return cns(env, make_tv<KindOfPersistentKeyset>(scalar_array()));
      case KindOfLazyClass:
        return cns(env, retVal.m_data.plazyclass.name());
      case KindOfUninit:
      case KindOfObject:
      case KindOfResource:
      case KindOfRFunc:
      // TODO (T29639296)
      case KindOfFunc:
      case KindOfClass:
      case KindOfClsMeth:
      case KindOfRClsMeth:
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
  if (type <= TVec) {
    auto const r = gen(env, VecFirst, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TDict) {
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
  if (type <= TVec) {
    auto const r = gen(env, VecLast, value);
    gen(env, IncRef, r);
    return r;
  } else if (type <= TDict) {
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

  if (type <= TVec) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = gen(env, CountVec, value);
        gen(env, JmpZero, taken, length);
      },
      [&] { return cns(env, 0); },
      [&] { return cns(env, TInitNull); }
    );
  } else if (type <= TDict) {
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

  if (type <= TVec) {
    return cond(
      env,
      [&](Block* taken) {
        auto const length = gen(env, CountVec, value);
        gen(env, JmpZero, taken, length);
        return length;
      },
      [&] (SSATmp* next) { return gen(env, SubInt, next, cns(env, 1)); },
      [&] { return cns(env, TInitNull); }
    );
  } else if (type <= TDict) {
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
  s_dyn_meth_caller_cls("__SystemLib\\DynMethCallerHelper"),
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
  } else if (type <= TRFunc) {
    return gen(env, LdFuncName, gen(env, LdFuncFromRFunc, value));
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
          auto const attr = AttrData { AttrIsMethCaller };
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
      auto const prop = ldPropAddr(env, obj, nullptr, cls, slot, TStr);
      auto const ret = gen(env, LdMem, TStr, prop);
      gen(env, IncRef, ret);
      return ret;
    };

    auto const check = [&] (const Class* cls, Block* taken) -> SSATmp* {
      auto isMC = gen(env, EqCls, cns(env, cls), gen(env, LdObjClass, value));
      gen(env, JmpZero, taken, isMC);
      return nullptr;
    };

    auto const mcCls = Class::lookup(s_meth_caller_cls.get());
    auto const dynMcCls = Class::lookup(s_dyn_meth_caller_cls.get());
    assertx(mcCls && meth_caller_has_expected_prop(mcCls));
    assertx(dynMcCls && meth_caller_has_expected_prop(dynMcCls));

    MultiCond mc{env};
    mc.ifThen(
      [&] (Block* taken) { return check(dynMcCls, taken); },
      [&] (SSATmp*) { return loadProp(dynMcCls, isCls, value); }
    );
    mc.ifThen(
      [&] (Block* taken) { return check(mcCls, taken); },
      [&] (SSATmp*) {
        if (RO::EvalEmitMethCallerFuncPointers &&
            RO::EvalNoticeOnMethCallerHelperUse) {
          updateMarker(env);
          env.irb->exceptionStackBoundary();
          auto const msg = cns(env, isCls ?
            s_MCHELPER_ON_GET_CLS.get() : s_MCHELPER_ON_GET_METH.get());
          gen(env, RaiseNotice, SampleRateData {}, msg);
        }
        return loadProp(mcCls, isCls, value);
      }
    );
    return mc.elseDo(
      [&] { // src is not a meth_caller
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
  } else if (value->type() <= TRClsMeth) {
    return gen(env, LdClsName, gen(env, LdClsFromRClsMeth, value));
  }
  return nullptr;
}

SSATmp* opt_class_meth_get_method(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->type() <= TClsMeth) {
    return gen(env, LdFuncName, gen(env, LdFuncFromClsMeth, value));
  } else if (value->type() <= TRClsMeth) {
    return gen(env, LdFuncName, gen(env, LdFuncFromRClsMeth, value));
  }
  return nullptr;
}

const EnumValues* getEnumValues(IRGS& env, const ParamPrep& params) {
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
  auto const origVal = params[0].value;
  if (!origVal->type().isKnownDataType()) return nullptr;
  auto const value = convertClassKey(env, origVal);
  auto const enum_values = getEnumValues(env, params);
  if (!enum_values) return nullptr;
  auto const ad = VanillaDict::as(enum_values->names.get());
  if (value->isA(TInt)) {
    if (ad->keyTypes().mustBeStrs()) return cns(env, false);
    return gen(env, AKExistsDict, cns(env, ad->asArrayData()), value);
  } else if (value->isA(TStr)) {
    // We're not doing intish-casts here, so we bail if ad has any int keys.
    if (!ad->keyTypes().mustBeStrs()) return nullptr;
    return gen(env, AKExistsDict, cns(env, ad->asArrayData()), value);
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

SSATmp* opt_is_meth_caller(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const value = params[0].value;
  if (value->isA(TFunc)) {
    return gen(env, FuncHasAttr, AttrData { AttrIsMethCaller }, value);
  }
  if (value->isA(TObj)) {
    auto const mcCls = Class::lookup(s_meth_caller_cls.get());
    auto const dynMcCls = Class::lookup(s_dyn_meth_caller_cls.get());
    assertx(mcCls);
    assertx(dynMcCls);
    auto const cls = gen(env, LdObjClass, value);
    auto const isMC = gen(env, EqCls, cns(env, mcCls), cls);
    auto const isDMC = gen(env, EqCls, cns(env, dynMcCls), cls);
    return gen(env, XorBool, isMC, isDMC);
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

SSATmp* opt_get_implicit_context_memo_key(IRGS& env, const ParamPrep& params) {
  if (params.size() != 0) return nullptr;
  return cond(
    env,
    [&] (Block* taken) {
      auto const ctx = gen(env, LdImplicitContext);
      return gen(env, CheckType, TObj, taken, ctx);
    },
    [&] (SSATmp* ctx) {
      auto const key = gen(env, LdImplicitContextMemoKey, ctx);
      gen(env, IncRef, key);
      return key;
    },
    [&] {
      return cns(env, staticEmptyString());
    }
  );
}

SSATmp* opt_class_to_classname(IRGS& env, const ParamPrep& params) {
  if (params.size() != 1) return nullptr;
  auto const c = params[0].value;

  if (c->isA(TStr)) {
    // See ext_hh.cpp. We inc-ref because the <<__Native>> PHP wrapper
    // owns its parameter and dec-refs it when it returns. We rely on
    // refcount-opts to drop the IncRef when the param is a static string.
    gen(env, IncRef, c);
    return c;
  }
  if (c->isA(TLazyCls)) return gen(env, LdLazyClsName, c);
  if (c->isA(TCls)) return gen(env, LdClsName, c);

  return nullptr;
}

//////////////////////////////////////////////////////////////////////

// Allowlists of builtins that we have optimized HHIR emitters for.
// The first allowlist here simply lets us look up the functions above.

using OptEmitFn = SSATmp* (*)(IRGS& env, const ParamPrep& params);

const hphp_fast_string_fmap<OptEmitFn> s_opt_emit_fns{
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
  {"is_numeric", opt_is_numeric},
  {"hphp_debug_caller_info", opt_hphp_debug_caller_info},
  {"HH\\array_key_cast", opt_array_key_cast},
  {"HH\\type_structure", opt_type_structure},
  {"HH\\type_structure_no_throw", opt_type_structure_no_throw},
  {"HH\\type_structure_classname", opt_type_structure_classname},
  {"HH\\is_list_like", opt_is_list_like},
  {"HH\\is_dict_or_darray", opt_is_dict_or_darray},
  {"HH\\is_vec_or_varray", opt_is_vec_or_varray},
  {"HH\\Lib\\_Private\\Native\\first", opt_container_first},
  {"HH\\Lib\\_Private\\Native\\last", opt_container_last},
  {"HH\\Lib\\_Private\\Native\\first_key", opt_container_first_key},
  {"HH\\Lib\\_Private\\Native\\last_key", opt_container_last_key},
  {"HH\\Lib\\_Private\\_Str\\vsprintf_l", opt_vsprintf_l},
  {"HH\\fun_get_function", opt_fun_get_function},
  {"HH\\class_meth_get_class", opt_class_meth_get_class},
  {"HH\\class_meth_get_method", opt_class_meth_get_method},
  {"HH\\class_get_class_name", opt_class_get_class_name},
  {"HH\\BuiltinEnum::getNames", opt_enum_names},
  {"HH\\BuiltinEnum::getValues", opt_enum_values},
  {"HH\\BuiltinEnum::coerce", opt_enum_coerce},
  {"HH\\BuiltinEnum::isValid", opt_enum_is_valid},
  {"HH\\is_meth_caller", opt_is_meth_caller},
  {"HH\\meth_caller_get_class", opt_meth_caller_get_class},
  {"HH\\meth_caller_get_method", opt_meth_caller_get_method},
  {"HH\\ImplicitContext\\_Private\\get_implicit_context_memo_key",
     opt_get_implicit_context_memo_key},
  {"HH\\class_to_classname", opt_class_to_classname},
  {"hphp_debug_caller_identifier", opt_hphp_debug_caller_identifier},
};

// This second allowlist, a subset of the first, records which parameter
// (if any) we need a vanilla input for to generate optimized HHIR.

const hphp_fast_string_fmap<int> s_vanilla_params{
  {"HH\\Lib\\_Private\\Native\\first", 0},
  {"HH\\Lib\\_Private\\Native\\last", 0},
  {"HH\\Lib\\_Private\\Native\\first_key", 0},
  {"HH\\Lib\\_Private\\Native\\last_key", 0},
};

//////////////////////////////////////////////////////////////////////

// If we encounter an inlined NativeImpls in an optimized region, we can't use
// a layout-sensitive implementation for the bytecode, today, because we don't
// support arbitrary control flow with an inlined NativeImpl.
bool skipLayoutSensitiveNativeImpl(IRGS& env, const StringData* fname) {
  return allowBespokeArrayLikes() &&
         s_vanilla_params.find(fname->data()) != s_vanilla_params.end();
}

SSATmp* ldOutAddr(IRGS& env, uint32_t inOutIndex) {
  if (isInlining(env)) {
    auto const func = curFunc(env);
    return gen(
      env,
      LdStkAddr,
      IRSPRelOffsetData {offsetFromIRSP(env, SBInvOffset {
        - func->numSlotsInFrame() - (int)kNumActRecCells - (int)inOutIndex
      })},
      sp(env)
    );
  }
  return gen(
    env,
    LdOutAddr,
    IndexData(inOutIndex),
    fp(env)
  );
}

SSATmp* optimizedFCallBuiltin(IRGS& env,
                              const Func* func,
                              const ParamPrep& params) {
  auto const result = [&]() -> SSATmp* {

    auto const fname = func->fullName();

    if (auto const retVal = opt_foldable(env, func, params)) {
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
      assertx(ad->isVecType());
      assertx(ad->size() == numInOut + 1);

      uint32_t inOutIndex = 0;
      for (auto const& param : params.info) {
        if (!param.isInOut) continue;
        // NB: The parameters to the builtin have already been popped
        // at this point, so we don't need to account for them when
        // calculating the stack offset.
        auto const val = cns(env, ad->nvGetVal(inOutIndex + 1));
        auto const addr = ldOutAddr(env, inOutIndex);
        gen(env, StMem, addr, val);
        ++inOutIndex;
      }

      // The first element of the tuple is always the actual function
      // return.
      return cns(env, ad->nvGetVal(0));
    }

    if (skipLayoutSensitiveNativeImpl(env, fname)) {
      return nullptr;
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
    env.irb->constrainValue(param.argValue, DataTypeSpecific);
  }
  return result;
}

//////////////////////////////////////////////////////////////////////

/*
 * Return the target type of  a parameter to a builtin function.
 *
 * If the builtin parameter has no type hints to cause coercion, this function
 * returns TBottom.
 */
Optional<Type> param_target_type(const Func* callee, uint32_t paramIdx) {
  auto const& pi = callee->params()[paramIdx];
  auto const& tc = pi.typeConstraint;
  if (tc.typeName() && interface_supports_arrlike(tc.typeName())) {
    // If we're dealing with an array-like interface, then there's no need to
    // check the input type: it's going to be mixed on the C++ side anyhow.
    return std::nullopt;
  }
  if (tc.isNullable()) {
    // FIXME(T116301380): native builtins don't resolve properly
    auto const dt = tc.isUnresolved() ? KindOfObject : tc.underlyingDataType();
    if (!dt) return std::nullopt;
    return TNull | Type(*dt);
  }
  if (!pi.builtinType()) {
    return tc.isVecOrDict() ? make_optional(TVec|TDict) : std::nullopt;
  }
  if (pi.builtinType() == KindOfObject &&
      pi.defaultValue.m_type == KindOfNull) {
    return TNullableObj;
  }
  return Type(*pi.builtinType());
}

//////////////////////////////////////////////////////////////////////

/*
 * Collect parameters for a call to a builtin.  Also determine which ones will
 * need to be passed through the eval stack, and which ones will need
 * conversions.
 */
ParamPrep
prepare_params(IRGS& env, const Func* callee, SSATmp* ctx,
               uint32_t numArgs) {
  auto ret = ParamPrep{numArgs, callee};
  ret.ctx = ctx;

  // Fill in in reverse order, since they may come from popC's (depending on
  // what loadParam wants to do).
  for (auto offset = uint32_t{numArgs}; offset-- > 0;) {
    auto const ty = param_target_type(callee, offset);
    auto& cur = ret[offset];
    auto& pi = callee->params()[offset];

    cur.isInOut = callee->isInOut(offset);
    cur.value = ldLoc(env, offset, DataTypeSpecific);
    // We do actually mean exact type equality here.  We're only capable of
    // passing the following primitives through registers; everything else goes
    // by address unless its flagged "NativeArg".
    auto const nonrefType = !pi.isTakenAsVariant() &&
      (ty == TBool || ty == TInt || ty == TDbl ||
       pi.isNativeArg() || pi.isTakenAsTypedValue());
    if (cur.isInOut || nonrefType) {
      cur.argValue = cur.value;
      continue;
    }

    cur.argValue = gen(env, LdLocAddr, LocalId(offset), fp(env));
    ++ret.numByAddr;
    cur.passByAddr = true;
  }

  return ret;
}

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
 *    param.argValue will have been updated with a phi of their results.
 */
template<class V, class C, class R>
SSATmp* realize_param(IRGS& env,
                      ParamPrep::Info& param,
                      const Func* callee,
                      Optional<Type> targetTy,
                      V checkType,
                      C convertParam,
                      R realize) {
  if (targetTy) {
    auto const baseTy = *targetTy - TNull;
    assertx(baseTy.isKnownDataType() || baseTy == (TVec|TDict));

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
            return convertParam(baseTy);
          }
        )) {
      // Heads up on non-local state here: we have to update
      // the values inside ParamPrep so that the CatchMaker
      // functions know about new potentially refcounted types
      // to decref, or values that were already decref'd and
      // replaced with things like ints.
      param.argValue = value;
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
  assertx(target.isKnownDataType() || (target == (TVec|TDict)));

  auto bail = [&] { fail(); return cns(env, TBottom); };
  if (target <= TStr) {
    if (!val->type().maybe(TLazyCls | TCls)) return bail();

    auto castW = [&] (SSATmp* val){
      if (RO::EvalClassStringHintNoticesSampleRate > 0) {
        auto tcInfo = folly::sformat(
          "argument {} passed to {}()", id + 1, func->fullName());
        std::string msg;
        string_printf(msg, Strings::CLASS_TO_STRING_IMPLICIT, tcInfo.c_str());
        gen(
          env,
          RaiseNotice,
          SampleRateData { RO::EvalClassStringHintNoticesSampleRate },
          cns(env, makeStaticString(msg))
        );
      }
      return update(val);
    };

    MultiCond mc{env};
    mc.ifTypeThen(val, TLazyCls, [&](SSATmp* lcval) {
      return castW(gen(env, LdLazyClsName, lcval));
    });
    mc.ifTypeThen(val, TCls, [&](SSATmp* cval) {
      return castW(gen(env, LdClsName, cval));
    });
    return mc.elseDo([&] {
      hint(env, Block::Hint::Unlikely);
      return bail();
    });
  }

  return bail();
}

StaticString
  s_varray_or_darray("varray_or_darray"),
  s_vec_or_dict("vec_or_dict");

/*
 * Prepare the actual arguments to the CallBuiltin instruction, by converting a
 * ParamPrep into a vector of SSATmps to pass to CallBuiltin.  If any of the
 * parameters needed type conversions, we need to do that here too.
 */
jit::vector<SSATmp*> realize_params(IRGS& env,
                                    const Func* callee,
                                    ParamPrep& params) {
  auto const cbNumArgs = 2 + params.size() + (params.ctx ? 1 : 0);
  auto ret = jit::vector<SSATmp*>(cbNumArgs);
  auto argIdx = uint32_t{0};
  ret[argIdx++] = fixupFP(env);
  ret[argIdx++] = sp(env);
  if (params.ctx) ret[argIdx++] = params.ctx;

  auto const genFail = [&](uint32_t param, SSATmp* val) {
    auto const expected_type = [&]{
      auto const& tc = callee->params()[param].typeConstraint;
      if (tc.isVecOrDict()) return s_vec_or_dict.get();
      auto const dt = param_target_type(callee, param).value_or(TUninit) - TNull;
      return getDataTypeString(dt.toDataType()).get();
    }();
    auto const data = FuncArgTypeData { callee, param + 1, expected_type };
    gen(env, ThrowParameterWrongType, data, val);
  };

  for (auto paramIdx = uint32_t{0}; paramIdx < params.size(); ++paramIdx) {
    auto& param = params[paramIdx];
    auto const targetTy = param_target_type(callee, paramIdx);

    if (param.passByAddr) {
      assertx(param.argValue->type() <= TMem);
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) -> SSATmp* {
          gen(env, CheckTypeMem, ty, fail, param.argValue);
          return nullptr;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          auto val = gen(env, LdMem, TCell, param.argValue);
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
            [&] { genFail(paramIdx, val); }
          );
          return nullptr;
        },
        [&] {
          return param.argValue;
        });
    } else {
      assertx(param.argValue->type() <= TCell);
      auto const oldVal = params[paramIdx].argValue;
      ret[argIdx++] = realize_param(
        env, param, callee, targetTy,
        [&] (const Type& ty, Block* fail) {
          auto ret = gen(env, CheckType, ty, fail, param.argValue);
          env.irb->constrainValue(ret, DataTypeSpecific);
          return ret;
        },
        [&] (const Type& ty) -> SSATmp* {
          hint(env, Block::Hint::Unlikely);
          return maybeCoerceValue(
            env,
            param.argValue,
            ty,
            paramIdx,
            callee,
            [&] (SSATmp* val) { return val; },
            [&] { genFail(paramIdx, oldVal); }
          );
        },
        [&] {
          return param.argValue;
        });
    }
  }

  assertx(argIdx == cbNumArgs);

  return ret;
}

//////////////////////////////////////////////////////////////////////

SSATmp* builtinInValue(IRGS& env, const Func* builtin, uint32_t i) {
  auto const tv = Native::builtinInValue(builtin, i);
  if (!tv) return nullptr;
  return cns(env, *tv);
}

const StaticString s_EagerVMSync("__EagerVMSync");

SSATmp* builtinCall(IRGS& env,
                    const Func* callee,
                    ParamPrep& params) {
  assertx(callee->nativeFuncPtr());

  auto const opt = optimizedFCallBuiltin(env, callee, params);
  if (opt) return opt;
  env.irb->exceptionStackBoundary();

  // Collect the realized parameters.
  auto realized = realize_params(env, callee, params);

  // Store the inout parameters into their out locations.
  if (callee->takesInOutParams()) {
    int32_t idx = 0;
    uint32_t aoff = params.ctx ? 3 : 2;
    for (auto i = uint32_t{0}; i < params.size(); ++i) {
      if (!params[i].isInOut) continue;
      auto ty = [&] () -> Optional<Type> {
        auto const r = builtinOutType(callee, i);
        if (r.isKnownDataType()) return r;
        return {};
      }();
      if (auto const iv = builtinInValue(env, callee, i)) {
        decRef(env, realized[i + aoff], static_cast<DecRefProfileId>(i));
        realized[i + aoff] = iv;
        ty = iv->type();
        if (ty->maybe(TPersistentVec)) *ty |= TVec;
        if (ty->maybe(TPersistentDict)) *ty |= TDict;
        if (ty->maybe(TPersistentKeyset)) *ty |= TKeyset;
        if (ty->maybe(TPersistentStr)) *ty |= TStr;
      }
      // Move the value to the caller stack to avoid an extra ref-count
      gen(env, StLoc, LocalId{i}, fp(env), cns(env, TInitNull));
      auto const addr = ldOutAddr(env, idx++);
      gen(env, StMem, ty, addr, realized[i + aoff]);
      realized[i + aoff] = addr;
    }
    env.irb->exceptionStackBoundary();
  }

  bool eagerSync = callee->userAttributes().count(LowStringPtr(s_EagerVMSync.get()));
  if (eagerSync) {
    auto const spOff = offsetFromIRSP(env, env.irb->curMarker().bcSPOff());
    eagerVMSync(env, spOff);
  }

  // Make the actual call.
  SSATmp** const decayedPtr = &realized[0];
  auto const ret = gen(
    env,
    CallBuiltin,
    CallBuiltinData {
      spOffBCFromIRSP(env),
      callee,
    },
    std::make_pair(realized.size(), decayedPtr)
  );

  if (eagerSync) gen(env, StVMRegState, cns(env, VMRegState::DIRTY));

  return ret;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

const StaticString s_isObjectMethCaller("is_object() called on MethCaller");

SSATmp* optimizedCallIsObject(IRGS& env, SSATmp* src) {
  auto notice = [&] (StringData* sd) {
    gen(env, RaiseNotice, SampleRateData {}, cns(env, sd));
  };
  if (src->isA(TObj) && src->type().clsSpec()) {
    auto const cls = src->type().clsSpec().cls();
    if (!env.irb->constrainValue(src, GuardConstraint(cls).setWeak())) {
      if (RO::EvalNoticeOnMethCallerHelperIsObject) {
        if (cls == SystemLib::getMethCallerHelperClass()) {
          notice(s_isObjectMethCaller.get());
        }
      }
      // If we know the class without having to specialize a guard
      // any further, use it.
      return cns(env, cls != SystemLib::get__PHP_Incomplete_ClassClass());
    }
  }

  if (!src->type().maybe(TObj)) {
    return cns(env, false);
  }

  auto checkClass = [&] (SSATmp* obj) {
    auto cls = gen(env, LdObjClass, obj);

    if (RO::EvalNoticeOnMethCallerHelperIsObject) {
      auto const c = SystemLib::getMethCallerHelperClass();
      ifThen(
        env,
        [&] (Block* taken) {
          gen(env, JmpNZero, taken, gen(env, EqCls, cls, cns(env, c)));
        },
        [&] { notice(s_isObjectMethCaller.get()); }
      );
    }

    auto testCls = SystemLib::get__PHP_Incomplete_ClassClass();
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

void emitNativeImpl(IRGS& env) {
  auto const callee = curFunc(env);

  auto genericNativeImpl = [&]() {
    bool eagerSync = callee->userAttributes().count(LowStringPtr(s_EagerVMSync.get()));
    if (eagerSync) {
      auto const spOff = offsetFromIRSP(env, env.irb->curMarker().bcSPOff());
      eagerVMSync(env, spOff);
    }
    gen(env, NativeImpl, fp(env), sp(env));
    if (eagerSync) gen(env, StVMRegState, cns(env, VMRegState::DIRTY));
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
    callee->numParams()
  );

  auto const ret = builtinCall(env, callee, params);

  push(env, ret);
  emitRetC(env);
}

//////////////////////////////////////////////////////////////////////

Type builtinOutType(const Func* builtin, uint32_t i) {
  assertx(builtin->isCPPBuiltin());
  assertx(builtin->isInOut(i));

  auto const& pinfo = builtin->params()[i];
  auto const& tc = pinfo.typeConstraint;
  if (auto const dt = Native::builtinOutType(tc, pinfo.userAttributes)) {
    const auto ty = Type{*dt};
    return tc.isNullable() ? ty | TInitNull : ty;
  }

  if (tc.isSoft() || tc.isMixed()) return TInitCell;

  auto ty = [&] () -> Type {
    switch (tc.metaType()) {
    case AnnotMetaType::Precise:
    case AnnotMetaType::SubObject:
      if (auto const dt = tc.underlyingDataType()) {
        return Type{*dt};
      }
      return TInitCell;
    case AnnotMetaType::Mixed:
      return TInitCell;
    case AnnotMetaType::Callable:
      return TInitCell;
    case AnnotMetaType::Number:
      return TInt | TDbl;
    case AnnotMetaType::ArrayKey:
      return TInt | TStr;
    case AnnotMetaType::This:
      return TObj;
    case AnnotMetaType::VecOrDict:
      return TVec|TDict;
    case AnnotMetaType::ArrayLike:
      return TArrLike;
    case AnnotMetaType::Classname:
      if (!RO::EvalClassPassesClassname) {
        return TStr;
      }
      return TStr | TCls | TLazyCls;
    case AnnotMetaType::Nonnull:
    case AnnotMetaType::NoReturn:
    case AnnotMetaType::Nothing:
      return TInitCell;
    case AnnotMetaType::Unresolved:
      // FIXME(T116301380): native builtins don't resolve properly
      return TObj;
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
  auto type = typeFromFuncReturn(builtin);

  // Allow builtins to return bespoke array likes if the flag is set.
  assertx(!type.arrSpec().vanilla());
  if (!allowBespokeArrayLikes()) type = type.narrowToVanilla();

  return type;
}

/////////////////////////////////////////////////////////////////////

namespace {

void implVecIdx(IRGS& env, SSATmp* loaded_collection_vec) {
  auto const def = topC(env, BCSPRelOffset{0});
  auto const key = topC(env, BCSPRelOffset{1});
  auto const stack_base = topC(env, BCSPRelOffset{2});

  auto const finish = [&](SSATmp* elem) {
    discard(env, 3);
    pushIncRef(env, elem);
    decRef(env, def, DecRefProfileId::IdxDef);
    decRef(env, key, DecRefProfileId::IdxKey);
    decRef(env, stack_base, DecRefProfileId::IdxBase);
  };

  if (key->isA(TNull | TStr)) return finish(def);

  if (!key->isA(TInt)) {
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
      gen(env, CheckVecBounds, taken, use_base, key);
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
  auto const def = topC(env, BCSPRelOffset{0});
  auto const origKey = topC(env, BCSPRelOffset{1});
  if (!origKey->type().isKnownDataType()) PUNT(Idx-KeyNotKnown);
  auto const key = convertClassKey(env, origKey);
  auto const stack_base = topC(env, BCSPRelOffset{2});

  auto const finish = [&](SSATmp* elem) {
    discard(env, 3);
    pushIncRef(env, elem);
    decRef(env, def, DecRefProfileId::IdxDef);
    decRef(env, key, DecRefProfileId::IdxKey);
    decRef(env, stack_base, DecRefProfileId::IdxBase);
  };

  if (key->isA(TNull)) return finish(def);

  if (!key->isA(TInt) && !key->isA(TStr)) {
    gen(env, ThrowInvalidArrayKey, stack_base, key);
    return;
  }

  assertx(is_dict || !loaded_collection_dict);
  auto const use_base = loaded_collection_dict
    ? loaded_collection_dict
    : stack_base;
  assertx(use_base->isA(is_dict ? TDict : TKeyset));

  auto const elem = profiledArrayAccess(
    env, use_base, key, MOpMode::None,
    [&] (SSATmp* base, SSATmp* key, SSATmp* pos) {
      return gen(env, is_dict ? DictGetK : KeysetGetK, base, key, pos);
    },
    [&] (SSATmp*) { return def; },
    [&] (SSATmp* key, SizeHintData data) {
      return is_dict ? gen(env, DictIdx, data, use_base, key, def)
                     : gen(env, KeysetIdx, use_base, key, def);
    },
    finish
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

namespace {
void implArrayMarkLegacy(IRGS& env, bool legacy) {
  auto const recursive = topC(env, BCSPRelOffset{0});
  auto const value     = topC(env, BCSPRelOffset{1});

  if (!recursive->isA(TBool)) {
    PUNT(ArrayMarkLegacy-RecursiveMustBeBool);
  } else if (!value->type().isKnownDataType()) {
    PUNT(ArrayMarkLegacy-ValueMustBeKnown);
  }


  if (!value->isA(TVec|TDict)) {
    discard(env);
    return;
  }

  auto const result = cond(
    env,
    [&](Block* taken) {
      gen(env, JmpZero, taken, recursive);
    },
    [&]{
      auto const op = legacy ? ArrayMarkLegacyRecursive
                             : ArrayUnmarkLegacyRecursive;
      return gen(env, op, value);
    },
    [&]{
      auto const op = legacy ? ArrayMarkLegacyShallow
                             : ArrayUnmarkLegacyShallow;
      return gen(env, op, value);
    }
  );

  discard(env, 2);
  push(env, result);
}
}

void emitArrayMarkLegacy(IRGS& env) {
  implArrayMarkLegacy(env, true);
}

void emitArrayUnmarkLegacy(IRGS& env) {
  implArrayMarkLegacy(env, false);
}

void emitArrayIdx(IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{2}, DataTypeGeneric)->type();
  if (arrType <= TVec) return implVecIdx(env, nullptr);
  if (arrType <= TDict) return implDictKeysetIdx(env, true, nullptr);
  if (arrType <= TKeyset) return implDictKeysetIdx(env, false, nullptr);
  if (arrType <= TClsMeth) PUNT(ArrayIdx_clsmeth);
  interpOne(env, TCell, 3);
}

void emitIdx(IRGS& env) {
  auto const key      = topC(env, BCSPRelOffset{1}, DataTypeGeneric);
  auto const base     = topC(env, BCSPRelOffset{2}, DataTypeGeneric);
  auto const keyType  = key->type();
  auto const baseType = base->type();

  if (baseType <= TVec) return implVecIdx(env, nullptr);
  if (baseType <= TDict) return implDictKeysetIdx(env, true, nullptr);
  if (baseType <= TKeyset) return implDictKeysetIdx(env, false, nullptr);

  if (keyType <= TNull || !baseType.maybe(TArrLike | TObj | TStr)) {
    auto const def = popC(env, DataTypeGeneric);
    popC(env, keyType <= TNull ? DataTypeSpecific : DataTypeGeneric);
    popC(env, keyType <= TNull ? DataTypeGeneric : DataTypeSpecific);
    push(env, def);
    decRef(env, base, DecRefProfileId::IdxDef);
    decRef(env, key, DecRefProfileId::IdxKey);
    return;
  }

  if (!(keyType <= TInt || keyType <= TStr)) {
    interpOne(env, TCell, 3);
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
  auto const arr = topC(env);
  auto const origKey = topC(env, BCSPRelOffset{ 1 });
  if (!origKey->type().isKnownDataType()) PUNT(AKExists-KeyNotKnown);
  auto const key = convertClassKey(env, origKey);
  if (key->isA(TFunc)) PUNT(AKExists_func_key);
  if (!arr->type().subtypeOfAny(TVec, TDict, TKeyset, TObj)) {
    PUNT(AKExists_unknown_array_or_obj_type);
  }

  auto const throwBadKey = [&] {
    gen(env, ThrowInvalidArrayKey, arr, key);
  };

  if (arr->isA(TVec)) {
    if (key->isA(TStr)) {
      discard(env, 2);
      push(env, cns(env, false));
      decRef(env, arr, DecRefProfileId::AKExistsArr);
      decRef(env, key, DecRefProfileId::AKExistsKey);
      return;
    }
    if (key->isA(TInt)) {
      auto const result = cond(
        env,
        [&](Block* taken) {
          gen(env, CheckVecBounds, taken, arr, key);
        },
        [&] { return cns(env, true); },
        [&] { return cns(env, false); }
      );
      discard(env, 2);
      push(env, result);
      decRef(env, arr, DecRefProfileId::AKExistsArr);
      return;
    }
    return throwBadKey();
  }

  if (arr->type().subtypeOfAny(TDict, TKeyset)) {
    if (!key->type().subtypeOfAny(TInt, TStr)) {
      return throwBadKey();
    }

    auto const finish = [&] (SSATmp* present) {
      discard(env, 2);
      push(env, present);
      decRef(env, arr, DecRefProfileId::AKExistsArr);
      decRef(env, key, DecRefProfileId::AKExistsKey);
    };

    auto const present = profiledArrayAccess(
      env, arr, key, MOpMode::None,
      [&] (SSATmp*, SSATmp*, SSATmp*) { return cns(env, true); },
      [&] (SSATmp*) { return cns(env, false); },
      [&] (SSATmp* key, SizeHintData) {
        auto const op = arr->isA(TKeyset) ? AKExistsKeyset : AKExistsDict;
        return gen(env, op, arr, key);
      },
      finish
    );
    finish(present);
    return;
  }

  if (!key->isA(TStr) && !key->isA(TInt)) PUNT(AKExists_badKey);

  if (arr->isA(TObj) && key->isA(TInt) &&
      collections::isType(arr->type().clsSpec().cls(), CollectionType::Vector,
                          CollectionType::ImmVector)) {
    auto const val =
      gen(env, CheckRange, key, gen(env, CountCollection, arr));
    discard(env, 2);
    push(env, val);
    decRef(env, arr, DecRefProfileId::AKExistsArr);
    return;
  }

  auto const val = gen(env, AKExistsObj, arr, key);
  discard(env, 2);
  push(env, val);
  decRef(env, arr, DecRefProfileId::AKExistsArr);
  decRef(env, key, DecRefProfileId::AKExistsKey);
}

//////////////////////////////////////////////////////////////////////
void emitGetMemoKeyL(IRGS& env, NamedLocal loc) {
  DEBUG_ONLY auto const func = curFunc(env);
  assertx(func->isMemoizeWrapper());

  auto const value = ldLocWarn(env, loc, DataTypeSpecific);

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
          MemoValueInstanceData { memoInfo.first, func, std::nullopt, loadAux },
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
          std::nullopt,
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
            std::nullopt,
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
        MemoValueStaticData { func, std::nullopt, loadAux },
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
        MemoCacheStaticData { func, keys, types.data(), std::nullopt, loadAux },
        notFound,
        retTy,
        fp(env)
      );
    }
    return gen(
      env,
      MemoGetStaticValue,
      MemoValueStaticData { func, std::nullopt, loadAux },
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

  auto const asyncEager = [&] () -> Optional<bool> {
    if (!func->isAsyncFunction()) return std::nullopt;
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
        ldVal(DataTypeGeneric)
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
      ldVal(DataTypeGeneric),
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
    ldVal(DataTypeGeneric)
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
      auto const level = ldLoc(env, localId, DataTypeGeneric);
      gen(env, RestoreErrorLevel, level);
    }
    break;
  }
}

void emitSetImplicitContextByValue(IRGS& env) {
  auto const tv = topC(env);
  ifThenElse(
    env,
    [&] (Block* taken) { gen(env, CheckType, TInitNull, taken, tv); },
    [&] {
      auto const prev = gen(env, LdImplicitContext);
      gen(env, StImplicitContext, cns(env, TInitNull));
      popC(env);
      pushIncRef(env, prev);
    },
    [&] {
      ifThenElse(
        env,
        [&] (Block* taken) { gen(env, CheckType, TObj, taken, tv); },
        [&] {
          auto const obj = gen(env, AssertType, TObj, tv);
          auto const prev = gen(env, LdImplicitContext);
          gen(env, StImplicitContext, obj);
          popC(env);
          pushIncRef(env, prev);
          // Decref after discarding so that if we are pushing the same object back,
          // avoid refcount going to zero
          decRef(env, obj);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          gen(env, AssertType, tv->type() - TObj - TInitNull, tv);
          interpOne(env);
        }
      );
    }
  );
}

void verifyImplicitContextState(IRGS& env, const Func* func) {
  assertx(!func->hasCoeffectRules());
  assertx(func->isMemoizeWrapper() || func->isMemoizeWrapperLSB());

  switch (func->memoizeICType()) {
    case Func::MemoizeICType::NoIC:
      if (providedCoeffectsKnownStatically(func).canCall(
          RuntimeCoeffects::leak_safe_shallow())) {
        // We are in a memoized that can call [defaults] code or any escape
        ifThen(
          env,
          [&] (Block* taken) {
            auto const ctx = gen(env, LdImplicitContext);
            gen(env, CheckType, TInitNull, taken, ctx);
          },
          [&] {
            hint(env, Block::Hint::Unlikely);
            gen(env, RaiseImplicitContextStateInvalid, FuncData { func });
            return cns(env, TBottom);
          }
        );
      }
      return;
    case Func::MemoizeICType::SoftMakeICInaccessible:
      ifThen(
        env,
        [&] (Block* taken) {
          auto const ctx = gen(env, LdImplicitContext);
          gen(env, CheckType, TInitNull, taken, ctx);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          gen(env, RaiseImplicitContextStateInvalid, FuncData { func });
        }
      );
      return;
    case Func::MemoizeICType::KeyedByIC:
    case Func::MemoizeICType::MakeICInaccessible:
      return;
  }

  not_reached();
}

void emitVerifyImplicitContextState(IRGS& env) {
  verifyImplicitContextState(env, curFunc(env));
}

void emitCreateSpecialImplicitContext(IRGS& env) {
  auto const memoKey = topC(env, BCSPRelOffset{0});
  auto const type = topC(env, BCSPRelOffset{1});
  if (!type->isA(TInt)) PUNT(CreateSpecialImplicitContext);
  if (!memoKey->isA(TStr) && !memoKey->isA(TInitNull)) {
    PUNT(CreateSpecialImplicitContext);
  }
  auto const func = curFunc(env);
  auto const obj =
    gen(env, CreateSpecialImplicitContext, type, memoKey, cns(env, func));
  decRef(env, memoKey);
  decRef(env, type);
  discard(env, 2);
  push(env, obj);
}

//////////////////////////////////////////////////////////////////////
}
