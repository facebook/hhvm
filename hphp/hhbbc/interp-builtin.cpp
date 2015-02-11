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
X(chr)
X(count)
X(decbin)
X(dechex)
X(decoct)
X(explode)
X(gettype)
X(hexdec)
X(implode)
X(in_array)
X(log)
X(log10)
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
X(floor)
X(ceil)
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
  X(s_chr)
  X(s_count)
  X(s_decbin)
  X(s_dechex)
  X(s_decoct)
  X(s_explode)
  X(s_gettype)
  X(s_hexdec)
  X(s_implode)
  X(s_in_array)
  X(s_log)
  X(s_log10)
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
  X(s_floor)
  X(s_ceil)
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

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x) if (op.str3->isame(s_##x.get())) return builtin_##x(env, op);

  X(get_class)
  X(abs)

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

  // Fall back to generic version.  (This can at least push some return type
  // information from HNI, but it won't be great in general.)
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popT(env);
  specialFunctionEffects(env, op.str3);
  push(env, TInitGen);
}

//////////////////////////////////////////////////////////////////////

}}
