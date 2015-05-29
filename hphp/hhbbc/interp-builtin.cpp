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
#include "hphp/hhbbc/interp.h"

#include "hphp/util/trace.h"

#include "hphp/runtime/base/execution-context.h"

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/interp-internal.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

#define X(x) const StaticString s_##x(#x);

X(array_combine)
X(array_fill)
X(array_fill_keys)
X(array_flip)
X(array_reverse)
X(array_search)
X(array_slice)
X(array_sum)
X(array_values)
X(base64decode)
X(base64_encode)
X(base_convert)
X(bindec)
X(ceil)
X(chr)
X(count)
X(decbin)
X(dechex)
X(decoct)
X(explode)
X(floor)
X(getrandmax)
X(gettype)
X(hexdec)
X(implode)
X(in_array)
X(log)
X(log10)
X(max)
X(mt_rand)
X(mt_getrandmax)
X(octdec)
X(ord)
X(pow)
X(preg_quote)
X(range)
X(rawurldecode)
X(rawurlencode)
X(round)
X(serialize)
X(sha1)
X(str_repeat)
X(str_split)
X(substr)
X(trim)
X(urldecode)
X(urlencode)
X(utf8_encode)
X(version_compare)
X(sqrt)
X(abs)

#undef X

//////////////////////////////////////////////////////////////////////

folly::Optional<Type> const_fold(ISS& env,
                                 const bc::FCallBuiltin& op,
                                 const StaticString& name) {
  FTRACE(1, "invoking: {}\n", name.get()->data());

  assert(!RuntimeOption::EvalJit);
  return eval_cell([&] {
    // Don't pop the args yet---if the builtin throws at compile time (because
    // it would raise a warning or something at runtime) we're going to leave
    // the call alone.
    std::vector<Cell> args(op.arg1);
    for (auto i = uint32_t{0}; i < op.arg1; ++i) {
      args[op.arg1 - i - 1] = *tv(topT(env, i));
    }

    Cell retVal;
    auto const func = Unit::lookupFunc(name.get());
    always_assert_flog(func, "func not found for builtin {}\n", name.get());
    g_context->invokeFuncFew(&retVal, func, nullptr, nullptr,
      args.size(), &args[0]);

    // If we got here, we didn't throw, so we can pop the inputs.
    for (auto i = uint32_t{0}; i < op.arg1; ++i) popT(env);

    assert(cellIsPlausible(retVal));
    return retVal;
  });
}

//////////////////////////////////////////////////////////////////////

folly::Optional<Type> const_fold(ISS& env, const bc::FCallBuiltin& op) {
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    auto const val = tv(topT(env, i));
    if (!val || val->m_type == KindOfUninit) return folly::none;
  }

#define X(x) if (op.str3->isame(x.get())) return const_fold(env, op, x);

  X(s_array_combine)
  X(s_array_fill)
  X(s_array_flip)
  X(s_array_reverse)
  X(s_array_search)
  X(s_array_slice)
  X(s_array_sum)
  X(s_array_values)
  X(s_base64decode)
  X(s_base64_encode)
  X(s_base_convert)
  X(s_bindec)
  X(s_ceil)
  X(s_chr)
  X(s_count)
  X(s_decbin)
  X(s_dechex)
  X(s_decoct)
  X(s_explode)
  X(s_floor)
  X(s_getrandmax)
  X(s_gettype)
  X(s_hexdec)
  X(s_implode)
  X(s_in_array)
  X(s_log)
  X(s_log10)
  X(s_mt_getrandmax)
  X(s_octdec)
  X(s_ord)
  X(s_pow)
  X(s_preg_quote)
  X(s_range)
  X(s_rawurldecode)
  X(s_rawurlencode)
  X(s_round)
  X(s_sha1)
  X(s_str_repeat)
  X(s_str_split)
  X(s_substr)
  X(s_trim)
  X(s_urldecode)
  X(s_urlencode)
  X(s_utf8_encode)
  X(s_version_compare)
  X(s_sqrt)
  X(s_abs)

  // Note serialize can only run user-defined code if its argument is an
  // object, which will never be a constant type, so this is safe.
  X(s_serialize)

  // We turn this on in case we'd be omitting a warning after constant-folding
  // array_fill_keys.  Even if at runtime this option will be off, it's fine to
  // compile as if it were on: we just won't fold the calls that could've
  // raised this error.
  RuntimeOption::StrictArrayFillKeys = HackStrictOption::ON;
  X(s_array_fill_keys)

  // A few that you can't do:

  // These read the current locale, so they can't be constant folded:
  // X(s_ucfirst)
  // X(s_ucwords)
  // X(s_strtolower)

  // This sets the last json error code, so it has observable effects:
  // X(s_json_encode)

#undef X

  return folly::none;
}

//////////////////////////////////////////////////////////////////////

const StaticString s_get_class("get_class");

bool builtin_get_class(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = topT(env);

  if (!ty.subtypeOf(TObj)) return false;

  auto unknown_class = [&] {
    popT(env);
    push(env, TStr);
    return true;
  };

  if (!is_specialized_obj(ty)) return unknown_class();
  auto const d = dobj_of(ty);
  switch (d.type) {
  case DObj::Sub:   return unknown_class();
  case DObj::Exact: break;
  }

  constprop(env);
  popT(env);
  push(env, sval(d.cls.name()));
  return true;
}

bool builtin_abs(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
  push(env, ty.subtypeOf(TInt) ? TInt :
            ty.subtypeOf(TDbl) ? TDbl :
            TInitUnc);
  return true;
}

/**
 * if the input to these functions is known to be integer or double,
 * the result will be a double. Otherwise, the result is conditional
 * on a successful conversion and an accurate number of arguments.
 */
bool floatIfNumeric(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
  push(env, ty.subtypeOf(TNum) ? TDbl : TInitUnc);
  return true;
}
bool builtin_ceil(ISS& env, const bc::FCallBuiltin& op) {
  return floatIfNumeric(env, op);
}
bool builtin_floor(ISS& env, const bc::FCallBuiltin& op) {
  return floatIfNumeric(env, op);
}

bool builtin_mt_rand(ISS& env, const bc::FCallBuiltin& op) {
  // In PHP, the two arg version can return false on input failure, but we don't
  // behave the same as PHP. we allow 1-arg calls and we allow the params to
  // come in any order.
  auto success = [&] {
    popT(env);
    popT(env);
    push(env, TInt);
    return true;
  };

  switch (op.arg1) {
  case 0:
    return success();
  case 1:
    return topT(env, 0).subtypeOf(TNum) ? success() : false;
  case 2:
    if (topT(env, 0).subtypeOf(TNum) &&
        topT(env, 1).subtypeOf(TNum)) {
      return success();
    }
    break;
  }
  return false;
}

/**
 * The compiler specializes the two-arg version of min() and max()
 * into an HNI provided helper. If both arguments are an integer
 * or both arguments are a double, we know the exact type of the
 * return value. If they're both numeric, the result is at least
 * numeric.
 */
bool minmax2(ISS& env, const bc::FCallBuiltin& op) {
  // this version takes exactly two arguments.
  if (op.arg1 != 2) return false;

  auto const t0 = topT(env, 0);
  auto const t1 = topT(env, 1);
  if (!t0.subtypeOf(TNum) || !t1.subtypeOf(TNum)) return false;
  popC(env);
  popC(env);
  push(env, t0 == t1 ? t0 : TNum);
  return true;
}
bool builtin_max2(ISS& env, const bc::FCallBuiltin& op) {
  return minmax2(env, op);
}
bool builtin_min2(ISS& env, const bc::FCallBuiltin& op) {
  return minmax2(env, op);
}

const StaticString
  s_max2("__SystemLib\\max2"),
  s_min2("__SystemLib\\min2");

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x) if (op.str3->isame(s_##x.get())) return builtin_##x(env, op);

  X(abs)
  X(ceil)
  X(floor)
  X(get_class)
  X(max2)
  X(min2)
  X(mt_rand)

#undef X

  return false;
}

//////////////////////////////////////////////////////////////////////

}

void builtin(ISS& env, const bc::FCallBuiltin& op) {
  if (options.ConstantFoldBuiltins) {
    if (auto const val = const_fold(env, op)) {
      constprop(env);
      return push(env, *val);
    }
  }

  // Try to handle the builtin at the type level.
  if (handle_builtin(env, op)) return;

  auto const name = op.str3;
  auto const func = env.index.resolve_func(env.ctx, name);
  auto const rt = env.index.lookup_return_type(env.ctx, func);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popT(env);
  specialFunctionEffects(env, name);
  push(env, rt);
}

//////////////////////////////////////////////////////////////////////

}}
