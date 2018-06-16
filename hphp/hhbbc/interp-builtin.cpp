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
#include "hphp/hhbbc/interp.h"

#include "hphp/util/trace.h"

#include "hphp/runtime/base/execution-context.h"

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

bool builtin_get_class(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = topT(env);
  if (op.arg2 == 0) {
    // AttrNoOverride naturally handles classes - but it also handles
    // traits, because we only set it if there are no uses of the
    // trait in the program (this is after any trait flattening has
    // taken place).
    if (!env.ctx.cls || !(env.ctx.cls->attrs & AttrNoOverride)) return false;
    assertx(ty.subtypeOf(TUninit));
    reduce(env,
           bc::PopU {},
           bc::String { env.ctx.cls->name },
           bc::RGetCNop {});
    return true;
  }

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

bool builtin_defined(ISS& env, const bc::FCallBuiltin& op) {
  if (!options.HardConstProp || op.arg1 != 2) return false;
  if (auto const v = tv(topT(env, 1))) {
    if (isStringType(v->m_type) &&
        !env.index.lookup_constant(env.ctx, v->m_data.pstr)) {
      env.collect.cnsMap[v->m_data.pstr].m_type = kDynamicConstant;
    }
  }
  return false;
}

bool builtin_function_exists(ISS& env, const bc::FCallBuiltin& op) {
  return handle_function_exists(env, op.arg1, true);
}

bool handle_oodecl_exists(ISS& env,
                          const bc::FCallBuiltin& op,
                          OODeclExistsOp subop) {
  if (op.arg1 != 2) return false;
  auto const& name = topT(env, 1);
  if (name.subtypeOf(TStr)) {
    if (!topT(env).subtypeOf(TBool)) {
      reduce(env,
             bc::CastBool {},
             bc::OODeclExists { subop },
             bc::RGetCNop {});
      return true;
    }
    reduce(env, bc::OODeclExists { subop }, bc::RGetCNop {});
    return true;
  }
  if (!topT(env).strictSubtypeOf(TBool)) return false;
  auto const v = tv(topT(env));
  assertx(v);
  reduce(env,
         bc::PopC {},
         bc::CastString {},
         gen_constant(*v),
         bc::OODeclExists { subop },
         bc::RGetCNop {});
  return true;
}

bool builtin_class_exists(ISS& env, const bc::FCallBuiltin& op) {
  return handle_oodecl_exists(env, op, OODeclExistsOp::Class);
}

bool builtin_interface_exists(ISS& env, const bc::FCallBuiltin& op) {
  return handle_oodecl_exists(env, op, OODeclExistsOp::Interface);
}

bool builtin_trait_exists(ISS& env, const bc::FCallBuiltin& op) {
  return handle_oodecl_exists(env, op, OODeclExistsOp::Trait);
}

bool builtin_class_alias(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 3) return false;
  auto const& alias = topT(env, 1);
  auto const& orig  = topT(env, 2);
  auto const alias_tv = tv(alias);
  auto const orig_tv = tv(orig);
  if (!alias_tv || !orig_tv ||
      !isStringType(alias_tv->m_type) ||
      !isStringType(orig_tv->m_type) ||
      !env.index.register_class_alias(orig_tv->m_data.pstr,
                                      alias_tv->m_data.pstr)) {
    return false;
  }

  auto const aload = topT(env);
  if (aload != TTrue && aload != TFalse) return false;

  reduce(env, bc::PopC {}, bc::PopC {}, bc::PopC {},
         gen_constant(make_tv<KindOfBoolean>(aload == TTrue)),
         bc::AliasCls { orig_tv->m_data.pstr, alias_tv->m_data.pstr },
         bc::RGetCNop {});
  return true;
}

bool builtin_array_key_cast(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = topC(env);

  if (ty.subtypeOf(TNum) || ty.subtypeOf(TBool) || ty.subtypeOf(TRes)) {
    reduce(env, bc::CastInt {}, bc::RGetCNop {});
    return true;
  }

  auto retTy = TBottom;
  if (ty.couldBe(TNull)) {
    retTy |= sval(staticEmptyString());
  }
  if (ty.couldBe(TNum) || ty.couldBe(TBool) || ty.couldBe(TRes)) {
    retTy |= TInt;
  }
  if (ty.couldBe(TStr)) {
    retTy |= [&] {
      if (ty.subtypeOf(TSStr)) {
        auto const v = tv(ty);
        if (v) {
          int64_t i;
          if (v->m_data.pstr->isStrictlyInteger(i)) {
            return ival(i);
          }
          return ty;
        }
        return TUncArrKey;
      }
      return TArrKey;
    }();
  }

  if (!ty.couldBe(TObj) && !ty.couldBe(TArr) &&
      !ty.couldBe(TVec) && !ty.couldBe(TDict) &&
      !ty.couldBe(TKeyset)) {
    constprop(env);
    nothrow(env);
  }

  popC(env);
  push(env, retTy);

  if (retTy == TBottom) unreachable(env);

  return true;
}

#define SPECIAL_BUILTINS                                                \
  X(abs, abs)                                                           \
  X(ceil, ceil)                                                         \
  X(floor, floor)                                                       \
  X(get_class, get_class)                                               \
  X(max2, max2)                                                         \
  X(min2, min2)                                                         \
  X(mt_rand, mt_rand)                                                   \
  X(strlen, strlen)                                                     \
  X(defined, defined)                                                   \
  X(function_exists, function_exists)                                   \
  X(class_exists, class_exists)                                         \
  X(interface_exists, interface_exists)                                 \
  X(trait_exists, trait_exists)                                         \
  X(class_alias, class_alias)                                           \
  X(array_key_cast, HH\\array_key_cast)                                 \

#define X(x, y)    const StaticString s_##x(#y);
  SPECIAL_BUILTINS
#undef X

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x, y) if (op.str3->isame(s_##x.get())) return builtin_##x(env, op);
  SPECIAL_BUILTINS
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
    if (auto val = const_fold(env, op.arg1, func)) {
      constprop(env);
      discard(env, op.arg1);
      return push(env, std::move(*val));
    }
  }

  // Try to handle the builtin at the type level.
  if (handle_builtin(env, op)) return;

  auto const num_args = op.arg1;
  auto const rt = [&]{
    // If we know they'll be no parameter coercions, we can provide a slightly
    // more precise type by ignoring AttrParamCoerceNull and
    // AttrParamCoerceFalse. Since this is a FCallBuiltin, we already know the
    // parameter count is exactly what the builtin expects.
    auto const precise_ty = [&]() -> folly::Optional<Type> {
      auto const exact = func.exactFunc();
      if (!exact) return folly::none;
      if (exact->attrs & (AttrVariadicParam | AttrVariadicByRef)) {
        return folly::none;
      }
      assert(num_args == exact->params.size());
      // Check to see if all provided arguments are sub-types of what the
      // builtin expects. If so, we know there won't be any coercions.
      for (auto i = uint32_t{0}; i < num_args; ++i) {
        auto const& param = exact->params[i];
        if (!param.builtinType || param.builtinType == KindOfUninit) {
          return folly::none;
        }
        auto const param_ty = from_DataType(*param.builtinType);
        if (!topT(env, num_args - i - 1).subtypeOf(param_ty)) {
          return folly::none;
        }
      }
      return native_function_return_type(exact, false);
    }();
    if (!precise_ty) return env.index.lookup_return_type(env.ctx, func);
    return *precise_ty;
  }();

  for (auto i = uint32_t{0}; i < num_args; ++i) popT(env);
  specialFunctionEffects(env, func);
  push(env, rt);
}

}

bool can_emit_builtin(borrowed_ptr<const php::Func> func,
                      int numArgs, bool hasUnpack) {
  if (func->attrs & (AttrInterceptable | AttrNoFCallBuiltin |
                     AttrTakesInOutParams) ||
      func->cls ||
      !func->nativeInfo ||
      func->params.size() >= Native::maxFCallBuiltinArgs() ||
      !RuntimeOption::EvalEnableCallBuiltin) {
    return false;
  }

  // We rely on strength reduction to convert builtins, but if we do
  // the analysis on the assumption that builtins will be created, but
  // don't actually create them, all sorts of things can go wrong (eg
  // attempting to constprop the result will fail, because we have a
  // bunch of FPass results on the stack).
  if (!options.StrengthReduce) {
    return false;
  }

  auto variadic = func->params.size() && func->params.back().isVariadic;

  // Only allowed to overrun the signature if we have somewhere to put it
  if (numArgs > func->params.size() && !variadic) return false;

  // Don't convert an FCallUnpack unless we're calling a variadic function
  // with the unpack in the right place to pass it directly.
  if (hasUnpack &&
      (!variadic || numArgs != func->params.size())) {
    return false;
  }

  // Don't convert to FCallBuiltin if there are too many variadic args.
  if (variadic && !hasUnpack &&
      numArgs - func->params.size() + 1 > ArrayData::MaxElemsOnStack) {
    return false;
  }

  auto const allowDoubleArgs = Native::allowFCallBuiltinDoubles();

  if (!allowDoubleArgs && func->nativeInfo->returnType == KindOfDouble) {
    return false;
  }

  auto const concrete_params = func->params.size() - (variadic ? 1 : 0);

  for (int i = 0; i < concrete_params; i++) {
    auto const& pi = func->params[i];
    if (!allowDoubleArgs && pi.builtinType == KindOfDouble) {
      return false;
    }
    if (i >= numArgs) {
      if (pi.isVariadic) continue;
      if (pi.defaultValue.m_type == KindOfUninit) {
        return false;
      }
    }
  }

  return true;
}

void finish_builtin(ISS& env,
                    borrowed_ptr<const php::Func> func,
                    uint32_t numArgs,
                    bool unpack) {
  std::vector<Bytecode> repl;
  assert(!unpack ||
         (numArgs &&
          numArgs == func->params.size() &&
          func->params.back().isVariadic));

  for (auto i = numArgs; i < func->params.size(); i++) {
    auto const& pi = func->params[i];
    if (pi.isVariadic) {
      if (RuntimeOption::EvalHackArrDVArrs) {
        repl.emplace_back(bc::Vec { staticEmptyVecArray() });
      } else {
        repl.emplace_back(bc::Array { staticEmptyVArray() });
      }
      continue;
    }
    auto cell = pi.defaultValue.m_type == KindOfNull && !pi.builtinType ?
      make_tv<KindOfUninit>() : pi.defaultValue;
    repl.emplace_back(gen_constant(cell));
  }
  if (!unpack &&
      func->params.size() &&
      func->params.back().isVariadic &&
      numArgs >= func->params.size()) {

    const uint32_t numToPack = numArgs - func->params.size() + 1;
    if (RuntimeOption::EvalHackArrDVArrs) {
      repl.emplace_back(bc::NewVecArray { numToPack });
    } else {
      repl.emplace_back(bc::NewVArray { numToPack });
    }
    numArgs = func->params.size();
  }

  assert(numArgs <= func->params.size());

  repl.emplace_back(
    bc::FCallBuiltin {
      static_cast<uint32_t>(func->params.size()), numArgs, func->name }
  );

  reduce(env, std::move(repl));
  fpiPop(env);
}

void reduce_fpass_arg(ISS& env, const Bytecode& bc, uint32_t param, bool byRef,
                      FPassHint hint) {
  if (byRef) {
    return reduce(env, bc, bc::FPassVNop { param, hint });
  }

  return reduce(env, bc, bc::FPassC { param, hint });
}

bool handle_function_exists(ISS& env, int numArgs, bool allowConstProp) {
  if (numArgs < 1 || numArgs > 2) return false;
  auto const& name = topT(env, numArgs - 1);
  if (!name.strictSubtypeOf(TStr)) return false;
  auto const v = tv(name);
  if (!v) return false;
  auto const rfunc = env.index.resolve_func(env.ctx, v->m_data.pstr);
  if (auto const func = rfunc.exactFunc()) {
    if (is_systemlib_part(*func->unit)) {
      if (!allowConstProp) return false;
      constprop(env);
      for (int i = 0; i < numArgs; i++) popC(env);
      push(env, TTrue);
      return true;
    }
    if (!any(env.collect.opts & CollectionOpts::Inlining)) {
      func->unit->persistent.store(false, std::memory_order_relaxed);
    }
  }
  return false;
}

folly::Optional<Type> const_fold(ISS& env,
                                 uint32_t nArgs,
                                 const res::Func& rfunc) {
  assert(rfunc.isFoldable());

  std::vector<Cell> args(nArgs);
  for (auto i = uint32_t{0}; i < nArgs; ++i) {
    auto const val = tv(topT(env, i));
    if (!val || val->m_type == KindOfUninit) return folly::none;
    args[nArgs - i - 1] = *val;
  }

  auto phpFunc = rfunc.exactFunc();
  assertx(phpFunc);

  Class* cls = nullptr;
  auto const func = [&] () -> HPHP::Func* {
    if (phpFunc->cls) {
      cls = Unit::lookupClass(phpFunc->cls->name);
      if (!cls || !(cls->attrs() & AttrBuiltin)) return nullptr;
      auto const f = cls->lookupMethod(phpFunc->name);
      if (!f->isStatic()) return nullptr;
      return f;
    }
    return Unit::lookupBuiltin(phpFunc->name);
  }();

  if (!func) return folly::none;

  // If the function is variadic, all the variadic parameters are already packed
  // into an array as the last parameter. invokeFuncFew, however, expects them
  // to be unpacked, so do so here.
  if (func->hasVariadicCaptureParam()) {
    if (args.empty()) return folly::none;
    if (!isArrayType(args.back().m_type) && !isVecType(args.back().m_type)) {
      return folly::none;
    }
    auto const variadic = args.back();
    args.pop_back();
    IterateV(
      variadic.m_data.parr,
      [&](TypedValue v) { args.emplace_back(v); }
    );
  }

  FTRACE(1, "invoking: {}\n", func->fullName()->data());

  assert(!RuntimeOption::EvalJit);
  return eval_cell(
    [&] {
      auto retVal = g_context->invokeFuncFew(
        func, HPHP::ActRec::encodeClass(cls), nullptr,
        args.size(), args.data(),
        false
      );

      assert(cellIsPlausible(retVal));
      return retVal;
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}
