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

#include "hphp/runtime/base/array-iterator.h"
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

const Type& getArg(ISS& env, const bc::FCallBuiltin& op, uint32_t idx) {
  assertx(idx < op.arg1);
  return topC(env, op.arg1 - idx - 1);
}

//////////////////////////////////////////////////////////////////////

bool builtin_get_class(ISS& env, const bc::FCallBuiltin& op) {
  auto const ty = getArg(env, op, 0);

  if (!ty.subtypeOf(BObj)) return false;

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
  auto const ty = getArg(env, op, 0);
  popC(env);
  push(env, ty.subtypeOf(BInt) ? TInt :
            ty.subtypeOf(BDbl) ? TDbl :
            TInitUnc);
  return true;
}

/**
 * if the input to these functions is known to be integer or double,
 * the result will be a double. Otherwise, the result is conditional
 * on a successful conversion and an accurate number of arguments.
 */
bool floatIfNumeric(ISS& env, const bc::FCallBuiltin& op) {
  auto const ty = getArg(env, op, 0);
  popC(env);
  push(env, ty.subtypeOf(BNum) ? TDbl : TInitUnc);
  return true;
}
bool builtin_ceil(ISS& env, const bc::FCallBuiltin& op) {
  return floatIfNumeric(env, op);
}
bool builtin_floor(ISS& env, const bc::FCallBuiltin& op) {
  return floatIfNumeric(env, op);
}

/**
 * The compiler specializes the two-arg version of min() and max()
 * into an HNI provided helper. If both arguments are an integer
 * or both arguments are a double, we know the exact type of the
 * return value. If they're both numeric, the result is at least
 * numeric.
 */
bool minmax2(ISS& env, const bc::FCallBuiltin& op) {
  auto const t0 = getArg(env, op, 0);
  auto const t1 = getArg(env, op, 1);
  if (!t0.subtypeOf(BNum) || !t1.subtypeOf(BNum)) return false;
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
  auto const ty = getArg(env, op, 0);
  popC(env);
  // Returns null and raises a warning when input is an array, resource, or
  // object.
  if (ty.subtypeOfAny(TPrim, TStr)) nothrow(env);
  push(env, ty.subtypeOfAny(TPrim, TStr) ? TInt : TOptInt);
  return true;
}

bool builtin_function_exists(ISS& env, const bc::FCallBuiltin& op) {
  if (!handle_function_exists(env, getArg(env, op, 0))) return false;

  constprop(env);
  for (int i = 0; i < op.arg1; i++) popC(env);
  push(env, TTrue);
  return true;
}

bool handle_oodecl_exists(ISS& env,
                          const bc::FCallBuiltin& op,
                          OODeclExistsOp subop) {
  auto const name = getArg(env, op, 0);
  auto const autoload = getArg(env, op, 1);
  if (name.subtypeOf(BStr)) {
    if (!autoload.subtypeOf(BBool)) {
      reduce(env,
             bc::CastBool {},
             bc::OODeclExists { subop });
      return true;
    }
    reduce(env, bc::OODeclExists { subop });
    return true;
  }
  if (!autoload.strictSubtypeOf(TBool)) return false;
  auto const v = tv(autoload);
  assertx(v);
  reduce(env,
         bc::PopC {},
         bc::CastString {},
         gen_constant(*v),
         bc::OODeclExists { subop });
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

bool builtin_array_key_cast(ISS& env, const bc::FCallBuiltin& op) {
  auto const ty = getArg(env, op, 0);

  if (ty.subtypeOf(BNum) || ty.subtypeOf(BBool) || ty.subtypeOf(BRes)) {
    reduce(env, bc::CastInt {});
    return true;
  }

  auto retTy = TBottom;
  if (ty.couldBe(BNull)) {
    retTy |= sval(staticEmptyString());
  }
  if (ty.couldBe(BNum | BBool | BRes)) {
    retTy |= TInt;
  }
  if (ty.couldBe(BCls)) {
    if (ty.strictSubtypeOf(TCls)) {
      auto cname = dcls_of(ty).cls.name();
      retTy |= sval(cname);
    } else {
      retTy |= TStr;
    }
  }
  // TODO: T70712990: Specialize lazy class types
  if (ty.couldBe(BLazyCls)) retTy |= TStr;
  if (ty.couldBe(BStr)) {
    retTy |= [&] {
      if (ty.subtypeOf(BSStr)) {
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

  if (!ty.couldBe(BObj | BArr | BVec | BDict | BKeyset)) {
    constprop(env);
    nothrow(env);
  }

  popC(env);
  push(env, retTy);

  if (retTy == TBottom) unreachable(env);

  return true;
}

bool builtin_is_callable(ISS& env, const bc::FCallBuiltin& op) {
  // Do not handle syntax-only checks or name output.
  if (getArg(env, op, 1) != TFalse) return false;

  auto const ty = getArg(env, op, 0);
  if (ty == TInitCell) return false;
  auto const res = [&]() -> folly::Optional<bool> {
    auto constexpr BFuncPtr = BClsMeth | BFunc;
    if (ty.subtypeOf(BFuncPtr)) return true;
    if (ty.subtypeOf(BArrLikeE) ||
        ty.subtypeOf(BKeyset) ||
        !ty.couldBe(BFuncPtr | BArr | BVec | BDict | BObj | BStr)) {
      return false;
    }
    return {};
  }();
  if (!res) return false;
  reduce(env, bc::PopC {}, bc::PopC {});
  *res ? reduce(env, bc::True {}) : reduce(env, bc::False {});
  return true;
}

bool builtin_is_list_like(ISS& env, const bc::FCallBuiltin& op) {
  auto const ty = getArg(env, op, 0);

  if (!ty.couldBe(TClsMeth)) {
    constprop(env);
    nothrow(env);
  }

  if (!ty.couldBeAny(TArr, TVec, TDict, TKeyset, TClsMeth)) {
    popC(env);
    push(env, TFalse);
    return true;
  }

  if (ty.subtypeOfAny(TVec, TVArr, TClsMeth)) {
    popC(env);
    push(env, TTrue);
    return true;
  }

  switch (categorize_array(ty).cat) {
    case Type::ArrayCat::Empty:
    case Type::ArrayCat::Packed:
      popC(env);
      push(env, TTrue);
      return true;
    case Type::ArrayCat::Mixed:
    case Type::ArrayCat::Struct:
      popC(env);
      push(env, TFalse);
      return true;
    case Type::ArrayCat::None:
      return false;
  }
  always_assert(false);
}

/*
 * This function optimizes type_structure{,_classname} statically.
 * Does not modify stack or locals.
 * Sets will_fail to true if we statically determined that this function
 * will throw.
 * If the resulting darray/dict is statically determined, it returns it.
 * Otherwise returns nullptr.
 */
ArrayData* impl_type_structure_opts(ISS& env, const bc::FCallBuiltin& op,
                                    bool& will_fail) {
  auto const fail = [&] {
    will_fail = true;
    return nullptr;
  };
  auto const result = [&](res::Class rcls, const StringData* cns,
                          bool check_lsb) -> ArrayData* {
    auto const cnst =
      env.index.lookup_class_const_ptr(env.ctx, rcls, cns, true);
    if (!cnst || !cnst->val || !cnst->isTypeconst) {
      if (check_lsb && (!cnst || !cnst->val)) return nullptr;
      // Either the const does not exist, it is abstract or is not a type const
      return fail();
    }
    if (check_lsb && !cnst->isNoOverride) return nullptr;
    auto const typeCns = cnst->val;
    if (!tvIsHAMSafeDArray(&*typeCns)) return nullptr;
    return resolveTSStatically(env, typeCns->m_data.parr, env.ctx.cls);
  };
  auto const cns_name = tv(getArg(env, op, 1));
  auto const cls_or_obj = tv(getArg(env, op, 0));
  if (!cns_name || !tvIsString(&*cns_name)) return nullptr;
  auto const cns_sd = cns_name->m_data.pstr;
  if (!cls_or_obj) {
    if (auto const last = op_from_slot(env, 1)) {
      if (last->op == Op::ClassName) {
        if (auto const prev = op_from_slot(env, 1, 1)) {
          if (prev->op == Op::LateBoundCls) {
            if (!env.ctx.cls) return fail();
            return result(env.index.resolve_class(env.ctx.cls), cns_sd, true);
          }
        }
      }
    }
    return nullptr;
  }
  if (tvIsString(&*cls_or_obj)) {
    auto const rcls = env.index.resolve_class(env.ctx, cls_or_obj->m_data.pstr);
    if (!rcls || !rcls->resolved()) return nullptr;
    return result(*rcls, cns_sd, false);
  } else if (!tvIsObject(&*cls_or_obj) && !tvIsClass(&*cls_or_obj)) {
    return fail();
  }
  return nullptr;
}

bool impl_builtin_type_structure(ISS& env, const bc::FCallBuiltin& op,
                                 bool no_throw) {
  bool fail = false;
  auto const ts = impl_type_structure_opts(env, op, fail);
  if (fail && !no_throw) {
    unreachable(env);
    popT(env);
    popT(env);
    push(env, TBottom);
    return true;
  }
  if (!ts) return false;
  reduce(env, bc::PopC {}, bc::PopC {});
  RuntimeOption::EvalHackArrDVArrs
    ? reduce(env, bc::Dict { ts }) : reduce(env, bc::Array { ts });
  return true;
}

bool builtin_type_structure(ISS& env, const bc::FCallBuiltin& op) {
  return impl_builtin_type_structure(env, op, false);
}

bool builtin_type_structure_no_throw(ISS& env, const bc::FCallBuiltin& op) {
  return impl_builtin_type_structure(env, op, true);
}

const StaticString s_classname("classname");

bool builtin_type_structure_classname(ISS& env, const bc::FCallBuiltin& op) {
  bool fail = false;
  auto const ts = impl_type_structure_opts(env, op, fail);
  if (fail) {
    unreachable(env);
    popT(env);
    popT(env);
    push(env, TBottom);
    return true;
  }
  if (ts) {
    auto const classname_field = ts->get(s_classname.get());
    if (isStringType(classname_field.type())) {
      reduce(env, bc::PopC {}, bc::PopC {},
             bc::String { classname_field.val().pstr });
      return true;
    }
  }
  popT(env);
  popT(env);
  push(env, TSStr);
  return true;
}

bool builtin_shapes_idx(ISS& env, const bc::FCallBuiltin& op) {
  auto def = to_cell(getArg(env, op, 2));
  auto const key = getArg(env, op, 1);
  auto const base = getArg(env, op, 0);
  const auto optDArr = RuntimeOption::EvalHackArrDVArrs ? BOptDict : BOptArr;

  if (!base.couldBe(optDArr) ||
      !key.couldBe(BArrKeyCompat)) {
    unreachable(env);
    discard(env, 3);
    push(env, TBottom);
    return true;
  }
  if (!base.subtypeOf(optDArr) || !key.subtypeOf(BOptArrKeyCompat)) {
    return false;
  }

  auto mightThrow = is_opt(key);

  if (base.subtypeOf(BNull)) {
    if (!mightThrow) {
      constprop(env);
      effect_free(env);
    }
    discard(env, 3);
    push(env, std::move(def));
    return true;
  }

  auto const unoptBase = is_opt(base) ? unopt(base) : base;
  auto const unoptKey = is_opt(key) ? unopt(key) : key;

  auto elem = RuntimeOption::EvalHackArrDVArrs
    ? dict_elem(unoptBase, unoptKey, def)
    : array_elem(unoptBase, unoptKey, def);
  switch (elem.second) {
    case ThrowMode::None:
    case ThrowMode::MaybeMissingElement:
    case ThrowMode::MissingElement:
      break;
    case ThrowMode::MaybeBadKey:
      mightThrow = true;
      break;
    case ThrowMode::BadOperation:
      always_assert(false);
  }

  if (!mightThrow) {
    constprop(env);
    effect_free(env);
  }

  auto res = elem.first;
  if (is_opt(base)) res |= def;

  discard(env, 3);
  push(env, std::move(res));
  return true;
}

#define SPECIAL_BUILTINS                                                \
  X(abs, abs)                                                           \
  X(ceil, ceil)                                                         \
  X(floor, floor)                                                       \
  X(get_class, get_class)                                               \
  X(max2, max2)                                                         \
  X(min2, min2)                                                         \
  X(strlen, strlen)                                                     \
  X(function_exists, function_exists)                                   \
  X(class_exists, class_exists)                                         \
  X(interface_exists, interface_exists)                                 \
  X(trait_exists, trait_exists)                                         \
  X(array_key_cast, HH\\array_key_cast)                                 \
  X(is_callable, is_callable)                                           \
  X(is_list_like, HH\\is_list_like)                                     \
  X(type_structure, HH\\type_structure)                                 \
  X(type_structure_no_throw, HH\\type_structure_no_throw)               \
  X(type_structure_classname, HH\\type_structure_classname)             \
  X(shapes_idx, HH\\Shapes::idx)                                        \

#define X(x, y)    const StaticString s_##x(#y);
  SPECIAL_BUILTINS
#undef X

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x, y) if (op.str3->isame(s_##x.get())) return builtin_##x(env, op);
  SPECIAL_BUILTINS
#undef X

  return false;
}

bool is_optimizable_builtin(const php::Func* func) {
#define X(x, y) if (func->name->isame(s_##x.get())) return true;
  SPECIAL_BUILTINS
#undef X

  return false;
}

//////////////////////////////////////////////////////////////////////

}

namespace interp_step {

void in(ISS& env, const bc::FCallBuiltin& op) {
  always_assert(op.arg2 == 0);
  auto const name = op.str3;
  auto const func = env.index.resolve_func(env.ctx, name);
  auto const exactFunc = func.exactFunc();

  if (options.ConstantFoldBuiltins && func.isFoldable()) {
    assertx(exactFunc);
    if (auto val = const_fold(env, op.arg1, 0, *exactFunc, true)) {
      constprop(env);

      always_assert(!RO::EvalArrayProvenance ||
                    !(exactFunc->attrs & AttrProvenanceSkipFrame));
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
      if (!exactFunc) return folly::none;
      if (exactFunc->attrs & AttrVariadicParam) {
        return folly::none;
      }
      assert(num_args == exactFunc->params.size());
      // Check to see if all provided arguments are sub-types of what the
      // builtin expects. If so, we know there won't be any coercions.
      for (auto i = uint32_t{0}; i < num_args; ++i) {
        auto const& param = exactFunc->params[i];
        if (!param.builtinType || param.builtinType == KindOfUninit) {
          return folly::none;
        }
        auto const param_ty = from_DataType(*param.builtinType);
        if (!topT(env, num_args - i - 1).subtypeOf(param_ty)) {
          return folly::none;
        }
      }
      return native_function_return_type(exactFunc);
    }();
    if (!precise_ty) return env.index.lookup_return_type(env.ctx, func);
    return *precise_ty;
  }();

  for (auto i = uint32_t{0}; i < num_args; ++i) popT(env);
  push(env, rt);
}

}

bool optimize_builtin(ISS& env, const php::Func* func, const FCallArgs& fca) {
  if (!will_reduce(env) ||
      any(env.collect.opts & CollectionOpts::Speculating) ||
      func->attrs & (AttrInterceptable | AttrNoFCallBuiltin) ||
      (func->cls && !(func->attrs & AttrStatic))  ||
      !func->nativeInfo ||
      func->params.size() >= Native::maxFCallBuiltinArgs() ||
      fca.hasGenerics() ||
      !RuntimeOption::EvalEnableCallBuiltin) {
    return false;
  }

  // We rely on strength reduction to convert builtins, but if we do
  // the analysis on the assumption that builtins will be created, but
  // don't actually create them, all sorts of things can go wrong.
  if (!options.StrengthReduce) return false;

  // Do not allow for inout arguments, unpack and variadic arguments
  if (func->hasInOutArgs ||
      fca.hasUnpack() ||
      (func->params.size() && func->params.back().isVariadic)) {
    return false;
  }

  // Argument not allowed to overrun the signature
  if (fca.numArgs() > func->params.size()) return false;

  // Check for missing non-optional arguments
  for (int i = fca.numArgs(); i < func->params.size(); i++) {
    auto const& pi = func->params[i];
    assertx(!pi.isVariadic);
    if (pi.defaultValue.m_type == KindOfUninit) return false;
  }

  if (!is_optimizable_builtin(func)) return false;

  BytecodeVec repl;
  for (auto i = fca.numArgs(); i < func->params.size(); i++) {
    auto const& pi = func->params[i];
    assertx(pi.defaultValue.m_type != KindOfUninit);
    repl.emplace_back(gen_constant(pi.defaultValue));
  }

  auto const numParams = static_cast<uint32_t>(func->params.size());
  if (func->cls == nullptr) {
    repl.emplace_back(
      bc::FCallBuiltin { numParams, fca.numRets() - 1, func->name });
  } else {
    assertx(func->attrs & AttrStatic);
    auto const fullname =
      makeStaticString(folly::sformat("{}::{}", func->cls->name, func->name));
    repl.emplace_back(
      bc::FCallBuiltin { numParams, fca.numRets() - 1, fullname });
  }
  for (int i = 0; i < kNumActRecCells; ++i) {
    repl.emplace_back(bc::PopU2 {});
  }

  reduce(env, std::move(repl));
  return true;
}

bool handle_function_exists(ISS& env, const Type& name) {
  if (!name.strictSubtypeOf(TStr)) return false;
  auto const v = tv(name);
  if (!v) return false;
  auto const rfunc = env.index.resolve_func(env.ctx, v->m_data.pstr);
  return rfunc.exactFunc();
}

folly::Optional<Type> const_fold(ISS& env,
                                 uint32_t nArgs,
                                 uint32_t numExtraInputs,
                                 const php::Func& phpFunc,
                                 bool variadicsPacked) {
  assert(phpFunc.attrs & AttrIsFoldable);

  std::vector<TypedValue> args(nArgs);
  auto const firstArgPos = numExtraInputs + nArgs - 1;
  for (auto i = uint32_t{0}; i < nArgs; ++i) {
    auto const val = tv(topT(env, firstArgPos - i));
    if (!val || val->m_type == KindOfUninit) return folly::none;
    args[i] = *val;
  }

  Class* cls = nullptr;
  auto const func = [&] () -> HPHP::Func* {
    if (phpFunc.cls) {
      cls = Class::lookup(phpFunc.cls->name);
      if (!cls || !(cls->attrs() & AttrBuiltin)) return nullptr;
      auto const f = cls->lookupMethod(phpFunc.name);
      if (!f->isStatic()) return nullptr;
      return f;
    }
    return Func::lookupBuiltin(phpFunc.name);
  }();

  if (!func) return folly::none;

  // If the function is variadic and all the variadic parameters are already
  // packed into an array as the last parameter, we need to unpack them, as
  // invokeFuncFew expects them to be unpacked.
  if (func->hasVariadicCaptureParam() && variadicsPacked) {
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
        func, cls, args.size(), args.data(), false, false);

      if (tvIsArrayLike(retVal)) {
        if (auto const tag = provTagHere(env).get()) {
          arrprov::TagOverride _(tag);
          auto const tagged = arrprov::tagTvRecursively(retVal, /*flags=*/0);
          tvMove(tagged, retVal);
        }
      }

      assert(tvIsPlausible(retVal));
      return retVal;
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}
