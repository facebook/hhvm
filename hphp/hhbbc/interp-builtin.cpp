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

folly::Optional<Type> const_fold(ISS& env,
                                 const bc::FCallBuiltin& op,
                                 const res::Func& rfunc) {
  assert(rfunc.isFoldable());

  // Don't pop the args yet---if the builtin throws at compile time (because
  // it would raise a warning or something at runtime) we're going to leave
  // the call alone.
  std::vector<Cell> args(op.arg1);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) {
    auto const val = tv(topT(env, i));
    if (!val || val->m_type == KindOfUninit) return folly::none;
    args[op.arg1 - i - 1] = *val;
  }

  auto const func = Unit::lookupFunc(rfunc.name());
  always_assert_flog(
    func,
    "func not found for builtin {}\n",
    rfunc.name()->data()
  );

  // If the function is variadic, all the variadic parameters are already packed
  // into an array as the last parameter. invokeFuncFew, however, expects them
  // to be unpacked, so do so here.
  if (func->hasVariadicCaptureParam()) {
    if (args.empty()) return folly::none;
    if (!isArrayType(args.back().m_type)) return folly::none;
    auto const variadic = args.back();
    args.pop_back();
    IterateV(
      variadic.m_data.parr,
      [&](const TypedValue* v) { args.emplace_back(*v); }
    );
  }

  FTRACE(1, "invoking: {}\n", func->fullName()->data());

  assert(!RuntimeOption::EvalJit);
  return eval_cell(
    [&] {
      auto retVal = g_context->invokeFuncFew(
        func, nullptr, nullptr,
        args.size(), args.data(), !env.ctx.unit->useStrictTypes
      );

      // If we got here, we didn't throw, so we can pop the inputs.
      for (auto i = uint32_t{0}; i < op.arg1; ++i) popT(env);

      assert(cellIsPlausible(retVal));
      return retVal;
    }
  );
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

bool builtin_strlen(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
  // Returns null and raises a warning when input is an array, resource, or
  // object.
  if (ty.subtypeOfAny(TPrim, TStr)) nothrow(env);
  push(env, ty.subtypeOfAny(TPrim, TStr) ? TInt : TOptInt);
  return true;
}

const StaticString
  s_abs("abs"),
  s_ceil("ceil"),
  s_floor("floor"),
  s_max2("__SystemLib\\max2"),
  s_min2("__SystemLib\\min2"),
  s_mt_rand("mt_rand"),
  s_strlen("mt_strlen");

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x) if (op.str3->isame(s_##x.get())) return builtin_##x(env, op);

  X(abs)
  X(ceil)
  X(floor)
  X(get_class)
  X(max2)
  X(min2)
  X(mt_rand)
  X(strlen)

#undef X

  return false;
}

//////////////////////////////////////////////////////////////////////

}

namespace interp_step {

void in(ISS& env, const bc::FCallBuiltin& op) {
  auto const name = op.str3;
  auto const func = env.index.resolve_func(env.ctx, name);

  if (options.ConstantFoldBuiltins && func.isFoldable()) {
    if (auto const val = const_fold(env, op, func)) {
      constprop(env);
      return push(env, *val);
    }
  }

  // Try to handle the builtin at the type level.
  if (handle_builtin(env, op)) return;

  auto const rt = env.index.lookup_return_type(env.ctx, func);
  for (auto i = uint32_t{0}; i < op.arg1; ++i) popT(env);
  specialFunctionEffects(env, func);
  push(env, rt);
}

}

//////////////////////////////////////////////////////////////////////

}}
