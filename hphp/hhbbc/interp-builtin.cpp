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

//////////////////////////////////////////////////////////////////////

bool builtin_get_class(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = topT(env);
  if (op.arg2 == 0) {
    // Support for get_class() is being removed.
    return false;
  }

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
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
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
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
  push(env, ty.subtypeOf(BNum) ? TDbl : TInitUnc);
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
    return topT(env, 0).subtypeOf(BNum) ? success() : false;
  case 2:
    if (topT(env, 0).subtypeOf(BNum) &&
        topT(env, 1).subtypeOf(BNum)) {
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
  if (op.arg1 != 1) return false;
  auto const ty = popC(env);
  // Returns null and raises a warning when input is an array, resource, or
  // object.
  if (ty.subtypeOfAny(TPrim, TStr)) nothrow(env);
  push(env, ty.subtypeOfAny(TPrim, TStr) ? TInt : TOptInt);
  return true;
}

bool builtin_function_exists(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 < 1 || op.arg1 > 2) return false;
  if (!handle_function_exists(env, topT(env, op.arg1 - 1))) return false;

  constprop(env);
  for (int i = 0; i < op.arg1; i++) popC(env);
  push(env, TTrue);
  return true;
}

bool handle_oodecl_exists(ISS& env,
                          const bc::FCallBuiltin& op,
                          OODeclExistsOp subop) {
  if (op.arg1 != 2) return false;
  auto const& name = topT(env, 1);
  if (name.subtypeOf(BStr)) {
    if (!topT(env).subtypeOf(BBool)) {
      reduce(env,
             bc::CastBool {},
             bc::OODeclExists { subop });
      return true;
    }
    reduce(env, bc::OODeclExists { subop });
    return true;
  }
  if (!topT(env).strictSubtypeOf(TBool)) return false;
  auto const v = tv(topT(env));
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
  if (op.arg1 != 1) return false;
  auto const ty = topC(env);

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

bool builtin_is_list_like(ISS& env, const bc::FCallBuiltin& op) {
  if (op.arg1 != 1) return false;
  auto const ty = topC(env);

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
    if (!tvIsDictOrDArray(&*typeCns)) return nullptr;
    return resolveTSStatically(env, typeCns->m_data.parr, env.ctx.cls);
  };
  if (op.arg1 != 2) return nullptr;
  auto const cns_name = tv(topT(env));
  auto const cls_or_obj = tv(topT(env, 1));
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

bool builtin_type_structure(ISS& env, const bc::FCallBuiltin& op) {
  bool fail = false;
  auto const ts = impl_type_structure_opts(env, op, fail);
  if (fail) {
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
    auto const classname_field = ts->rval(s_classname.get());
    if (classname_field != nullptr && isStringType(classname_field.type())) {
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
  if (op.arg1 != 3) return false;

  auto const def = to_cell(topT(env));
  auto const key = topC(env, 1);
  auto const base = topC(env, 2);

  if (!base.couldBe(RuntimeOption::EvalHackArrDVArrs ? BDict : BArr) ||
      !key.couldBe(BArrKey)) {
    unreachable(env);
    discard(env, 3);
    push(env, TBottom);
    return true;
  }
  if (!base.subtypeOf(RuntimeOption::EvalHackArrDVArrs ? BOptDict : BOptDArr) ||
      !key.subtypeOf(BOptArrKey)) {
    return false;
  }

  auto const unoptBase = is_opt(base) ? unopt(base) : base;
  auto const unoptKey = is_opt(key) ? unopt(key) : key;
  auto mightThrow = is_opt(base) || is_opt(key);

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
  discard(env, 3);
  push(env, std::move(elem.first));
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
  X(function_exists, function_exists)                                   \
  X(class_exists, class_exists)                                         \
  X(interface_exists, interface_exists)                                 \
  X(trait_exists, trait_exists)                                         \
  X(array_key_cast, HH\\array_key_cast)                                 \
  X(is_list_like, HH\\is_list_like)                                     \
  X(type_structure, HH\\type_structure)                                 \
  X(type_structure_classname, HH\\type_structure_classname)             \
  X(shapes_idx, HH\\Shapes::idx)                                        \

#define X(x, y)    const StaticString s_##x(#y);
  SPECIAL_BUILTINS
#undef X

bool handle_builtin(ISS& env, const bc::FCallBuiltin& op) {
#define X(x, y) if (op.str4->isame(s_##x.get())) return builtin_##x(env, op);
  SPECIAL_BUILTINS
#undef X

  return false;
}

//////////////////////////////////////////////////////////////////////

}

namespace interp_step {

void in(ISS& env, const bc::FCallBuiltin& op) {
  auto const name = op.str4;
  auto const func = env.index.resolve_func(env.ctx, name);

  if (options.ConstantFoldBuiltins && func.isFoldable()) {
    assertx(func.exactFunc());
    if (auto val = const_fold(env, op.arg1, 0, *func.exactFunc(), true)) {
      constprop(env);

      // If there's no in-out parameters, we're done and can just push
      // the result.
      if (!op.arg3) {
        discard(env, op.arg1);
        return push(env, std::move(*val));
      }

      // Otherwise what we got from evaluating the function is
      // actually a tuple of all the results. We need to extract the
      // values out of the tuple and push them onto the stack in their
      // proper order.
      auto const v = tv(*val);
      assertx(v);

      assertx(tvIsVecOrVArray(*v));
      auto const ad = v->m_data.parr;
      assertx(ad->isStatic());
      assertx(ad->size() == op.arg3 + 1);

      BytecodeVec repl;
      // Pop the arguments
      for (uint32_t i = 0; i < op.arg1; ++i) {
        // FCallBuiltin has CU flavored args, so we need to check the
        // type to know how to pop them.
        auto const& popped = topT(env, i);
        if (popped.subtypeOf(BUninit)) {
          repl.push_back(bc::PopU {});
        } else {
          assertx(popped.subtypeOf(BInitCell));
          repl.push_back(bc::PopC {});
        }
      }
      // Pop the placeholder uninit values
      for (uint32_t i = 0; i < op.arg3; ++i) {
        repl.push_back(bc::PopU {});
      }
      // Push the in-out returns
      for (uint32_t i = 0; i < op.arg3; ++i) {
        repl.push_back(gen_constant(ad->nvGetVal(op.arg3 - i)));
      }
      // And push the actual function return
      repl.push_back(gen_constant(ad->nvGetVal(0)));

      return reduce(env, std::move(repl));
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
      if (exact->attrs & AttrVariadicParam) {
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
      return native_function_return_type(exact);
    }();
    if (!precise_ty) return env.index.lookup_return_type(env.ctx, func);
    return *precise_ty;
  }();

  auto const num_out = op.arg3;
  for (auto i = uint32_t{0}; i < num_args + num_out; ++i) popT(env);

  for (auto i = num_out; i > 0; --i) {
    if (auto const f = func.exactFunc()) {
      push(env, native_function_out_type(f, i - 1));
    } else {
      push(env, TInitCell);
    }
  }

  push(env, rt);
}

}

bool can_emit_builtin(ISS& env, const php::Func* func, const FCallArgs& fca) {
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
  if (!options.StrengthReduce) {
    return false;
  }

  auto const variadic = func->params.size() && func->params.back().isVariadic;
  auto const concrete_params = func->params.size() - (variadic ? 1 : 0);

  // Only allowed to overrun the signature if we have somewhere to put it
  if (!variadic && (fca.hasUnpack() || fca.numArgs() > concrete_params)) {
    return false;
  }

  // Don't convert an FCall* with unpack unless we're calling a variadic
  // function with the unpack in the right place to pass it directly.
  if (variadic && fca.hasUnpack() && fca.numArgs() != concrete_params) {
    return false;
  }

  // Don't convert to FCallBuiltin if there are too many variadic args.
  if (variadic && !fca.hasUnpack() &&
      fca.numArgs() - concrete_params > ArrayData::MaxElemsOnStack) {
    return false;
  }

  if (fca.enforceInOut()) {
    // Don't convert functions with mis-annotated inout arguments
    for (int i = 0; i < func->params.size(); ++i) {
      auto const isInOut = i < fca.numArgs() && fca.isInOut(i);
      if (func->params[i].inout != isInOut) return false;
    }

    // Don't convert functions with extra inout arguments
    for (int i = func->params.size(); i < fca.numArgs(); ++i) {
      if (fca.isInOut(i)) return false;
    }
  }

  // Check for missing non-optional arguments.
  for (int i = fca.numArgs(); i < concrete_params; i++) {
    auto const& pi = func->params[i];
    if (pi.defaultValue.m_type == KindOfUninit) {
      return false;
    }
  }

  return true;
}

void finish_builtin(ISS& env, const php::Func* func, const FCallArgs& fca) {
  BytecodeVec repl;
  assertx(!fca.hasGenerics());
  assertx(!fca.hasUnpack() ||
          (fca.numArgs() + 1 == func->params.size() &&
           func->params.back().isVariadic));

  auto numArgs = fca.numInputs();
  if (!fca.hasUnpack()) {
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

    if (func->params.size() &&
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
  }

  assert(numArgs <= func->params.size());

  auto const numParams = static_cast<uint32_t>(func->params.size());
  if (func->cls == nullptr) {
    repl.emplace_back(
      bc::FCallBuiltin { numParams, numArgs, fca.numRets() - 1, func->name });
  } else {
    assertx(func->attrs & AttrStatic);
    auto const fullname =
      makeStaticString(folly::sformat("{}::{}", func->cls->name, func->name));
    repl.emplace_back(
      bc::FCallBuiltin { numParams, numArgs, fca.numRets() - 1, fullname });
  }
  if (fca.numRets() != 1) {
    repl.emplace_back(bc::PopFrame { fca.numRets() });
  } else {
    repl.emplace_back(bc::PopU2 {});
    repl.emplace_back(bc::PopU2 {});
    repl.emplace_back(bc::PopU2 {});
  }

  reduce(env, std::move(repl));
}

bool handle_function_exists(ISS& env, const Type& name) {
  if (!name.strictSubtypeOf(TStr)) return false;
  auto const v = tv(name);
  if (!v) return false;
  auto const rfunc = env.index.resolve_func(env.ctx, v->m_data.pstr);
  if (auto const func = rfunc.exactFunc()) {
    if (is_systemlib_part(*func->unit)) return true;
    if (!any(env.collect.opts & CollectionOpts::Inlining)) {
      func->unit->persistent.store(false, std::memory_order_relaxed);
    }
  }
  return false;
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
      cls = Unit::lookupClass(phpFunc.cls->name);
      if (!cls || !(cls->attrs() & AttrBuiltin)) return nullptr;
      auto const f = cls->lookupMethod(phpFunc.name);
      if (!f->isStatic()) return nullptr;
      return f;
    }
    return Unit::lookupBuiltin(phpFunc.name);
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
        func, cls, nullptr,
        args.size(), args.data(),
        false, false
      );

      assert(tvIsPlausible(retVal));
      return retVal;
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}
