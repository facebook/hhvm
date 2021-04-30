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

#include "hphp/util/match.h"
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

const Type getArg(ISS& env, const php::Func* func, const FCallArgs& fca,
                  uint32_t idx) {
  assertx(idx < func->params.size());
  if (idx < fca.numArgs()) return topC(env, fca.numArgs() - idx - 1);
  auto const& pi = func->params[idx];
  assertx(pi.defaultValue.m_type != KindOfUninit);
  return from_cell(pi.defaultValue);
}

//////////////////////////////////////////////////////////////////////

struct Reduced {};
struct NoReduced {};
using TypeOrReduced = boost::variant<Type, Reduced, NoReduced>;

//////////////////////////////////////////////////////////////////////

TypeOrReduced builtin_get_class(ISS& env, const php::Func* func,
                                const FCallArgs& fca) {
  assertx(fca.numArgs() >= 0 && fca.numArgs() <= 1);
  auto const ty = getArg(env, func, fca, 0);

  if (!ty.subtypeOf(BObj)) return NoReduced{};

  if (!is_specialized_obj(ty)) return TStr;
  auto const d = dobj_of(ty);
  switch (d.type) {
  case DObj::Sub:   return TStr;
  case DObj::Exact: break;
  }

  constprop(env);
  return sval(d.cls.name());
}

TypeOrReduced builtin_abs(ISS& env, const php::Func* func,
                          const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);
  return ty.subtypeOf(BInt) ? TInt :
            ty.subtypeOf(BDbl) ? TDbl :
            TInitUnc;
}

/**
 * if the input to these functions is known to be integer or double,
 * the result will be a double. Otherwise, the result is conditional
 * on a successful conversion and an accurate number of arguments.
 */
TypeOrReduced floatIfNumeric(ISS& env, const php::Func* func,
                             const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);
  return ty.subtypeOf(BNum) ? TDbl : TInitUnc;
}
TypeOrReduced builtin_ceil(ISS& env, const php::Func* func,
                           const FCallArgs& fca) {
  return floatIfNumeric(env, func, fca);
}
TypeOrReduced builtin_floor(ISS& env, const php::Func* func,
                            const FCallArgs& fca) {
  return floatIfNumeric(env, func, fca);
}

/**
 * The compiler specializes the two-arg version of min() and max()
 * into an HNI provided helper. If both arguments are an integer
 * or both arguments are a double, we know the exact type of the
 * return value. If they're both numeric, the result is at least
 * numeric.
 */
TypeOrReduced minmax2(ISS& env, const php::Func* func, const FCallArgs& fca) {
  assertx(fca.numArgs() == 2);
  auto const t0 = getArg(env, func, fca, 0);
  auto const t1 = getArg(env, func, fca, 1);
  if (!t0.subtypeOf(BNum) || !t1.subtypeOf(BNum)) return NoReduced{};
  return t0 == t1 ? t0 : TNum;
}
TypeOrReduced builtin_max2(ISS& env, const php::Func* func,
                           const FCallArgs& fca) {
  return minmax2(env, func, fca);
}
TypeOrReduced builtin_min2(ISS& env, const php::Func* func,
                           const FCallArgs& fca) {
  return minmax2(env, func, fca);
}

TypeOrReduced builtin_strlen(ISS& env, const php::Func* func,
                             const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);
  // Returns null and raises a warning when input is an array, resource, or
  // object.
  if (ty.subtypeOf(BPrim | BStr)) effect_free(env);
  return ty.subtypeOf(BPrim | BStr) ? TInt : TOptInt;
}

TypeOrReduced builtin_function_exists(ISS& env, const php::Func* func,
                                      const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  if (!handle_function_exists(env, getArg(env, func, fca, 0))) {
    return NoReduced{};
  }
  constprop(env);
  return TTrue;
}

TypeOrReduced handle_oodecl_exists(ISS& env,
                                   const php::Func* func, const FCallArgs& fca,
                                   OODeclExistsOp subop) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  auto const name = getArg(env, func, fca, 0);
  auto const autoload = getArg(env, func, fca, 1);
  if (name.subtypeOf(BStr)) {
    if (fca.numArgs() == 1) {
        reduce(env, bc::True {});
    } else if (!autoload.subtypeOf(BBool)) {
        reduce(env, bc::CastBool {});
    }
    reduce(env, bc::OODeclExists { subop });
    return Reduced{};
  }
  if (!autoload.strictSubtypeOf(TBool)) return NoReduced{};
  auto const v = tv(autoload);
  assertx(v);
  if (fca.numArgs() == 2) reduce(env, bc::PopC {});
  reduce(env,
         bc::CastString {},
         gen_constant(*v),
         bc::OODeclExists { subop });
  return Reduced{};
}

TypeOrReduced builtin_class_exists(ISS& env, const php::Func* func,
                                   const FCallArgs& fca) {
  return handle_oodecl_exists(env, func, fca, OODeclExistsOp::Class);
}

TypeOrReduced builtin_interface_exists(ISS& env, const php::Func* func,
                                       const FCallArgs& fca) {
  return handle_oodecl_exists(env, func, fca, OODeclExistsOp::Interface);
}

TypeOrReduced builtin_trait_exists(ISS& env, const php::Func* func,
                                   const FCallArgs& fca) {
  return handle_oodecl_exists(env, func, fca, OODeclExistsOp::Trait);
}

TypeOrReduced builtin_array_key_cast(ISS& env, const php::Func* func,
                                     const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);

  if (ty.subtypeOf(BNum) || ty.subtypeOf(BBool) || ty.subtypeOf(BRes)) {
    reduce(env, bc::CastInt {});
    return Reduced{};
  }

  auto retTy = TBottom;
  if (ty.couldBe(BNull)) {
    retTy |= sval(staticEmptyString());
  }
  if (ty.couldBe(BNum | BBool | BRes)) {
    retTy |= TInt;
  }
  if (ty.couldBe(BCls)) {
    if (is_specialized_cls(ty)) {
      auto const dcls = dcls_of(ty);
      if (dcls.type == DCls::Exact) {
        auto cname = dcls_of(ty).cls.name();
        retTy |= sval(cname);
      } else {
        retTy |= TSStr;
      }
    } else {
      retTy |= TSStr;
    }
  }
  if (ty.couldBe(BLazyCls)) {
    if (is_specialized_lazycls(ty)) {
      auto cname = lazyclsval_of(ty);
      retTy |= sval(cname);
    } else {
      retTy |= TSStr;
    }
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

  if (!ty.couldBe(BObj | BArrLike)) {
    constprop(env);
    effect_free(env);
  }

  if (retTy == TBottom) unreachable(env);
  return retTy;
}

TypeOrReduced builtin_is_callable(ISS& env, const php::Func* func,
                                  const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  // Do not handle syntax-only checks or name output.
  if (getArg(env, func, fca, 1) != TFalse) return NoReduced{};

  auto const ty = getArg(env, func, fca, 0);
  if (ty == TInitCell) return NoReduced{};
  auto const res = [&]() -> folly::Optional<bool> {
    if (ty.subtypeOf(BClsMeth | BFunc)) return true;
    if (ty.subtypeOf(BArrLikeE | BKeyset) ||
        !ty.couldBe(BClsMeth | BFunc | BVec | BDict | BObj | BStr)) {
      return false;
    }
    return {};
  }();
  if (!res) return NoReduced{};
  if (fca.numArgs() == 2) reduce(env, bc::PopC {});
  reduce(env, bc::PopC {});
  *res ? reduce(env, bc::True {}) : reduce(env, bc::False {});
  return Reduced{};
}

TypeOrReduced builtin_is_list_like(ISS& env, const php::Func* func,
                                   const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);

  if (!ty.couldBe(BClsMeth)) {
    constprop(env);
    effect_free(env);
  }

  if (!ty.couldBe(BArrLike | BClsMeth)) return TFalse;
  if (ty.subtypeOf(BVec | BClsMeth)) return TTrue;

  switch (categorize_array(ty).cat) {
    case Type::ArrayCat::Empty:
    case Type::ArrayCat::Packed:
      return TTrue;
    case Type::ArrayCat::Mixed:
    case Type::ArrayCat::Struct:
      return TFalse;
    case Type::ArrayCat::None:
      return NoReduced{};
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
ArrayData* impl_type_structure_opts(ISS& env,
                                    const php::Func* func, const FCallArgs& fca,
                                    bool& will_fail) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  auto const fail = [&] {
    will_fail = true;
    return nullptr;
  };
  auto const result = [&](res::Class rcls, const StringData* cns,
                          bool check_lsb) -> ArrayData* {
    auto const cnst =
      env.index.lookup_class_const_ptr(env.ctx, rcls, cns, true);
    if (!cnst || !cnst->val || cnst->kind != ConstModifiers::Kind::Type) {
      if (check_lsb && (!cnst || !cnst->val)) return nullptr;
      // Either the const does not exist, it is abstract or is not a type const
      return fail();
    }
    if (check_lsb && !cnst->isNoOverride) return nullptr;
    auto const typeCns = cnst->val;
    if (!tvIsDict(&*typeCns)) return nullptr;
    return resolveTSStatically(env, typeCns->m_data.parr, env.ctx.cls);
  };
  auto const cns_name = tv(getArg(env, func, fca, 1));
  auto const cls_or_obj = tv(getArg(env, func, fca, 0));
  if (!cns_name || !tvIsString(&*cns_name)) return nullptr;
  auto const cns_sd = cns_name->m_data.pstr;
  if (!cls_or_obj) {
    if (auto const last = op_from_slot(env, 1)) {
      if (last->op == Op::ClassName || last->op == Op::LazyClassFromClass) {
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
  if (tvIsString(&*cls_or_obj) || tvIsLazyClass(&*cls_or_obj)) {
    auto const rcls =
      env.index.resolve_class(env.ctx,
                              tvIsString(&*cls_or_obj) ?
                              cls_or_obj->m_data.pstr :
                              cls_or_obj->m_data.plazyclass.name());
    if (!rcls || !rcls->resolved()) return nullptr;
    return result(*rcls, cns_sd, false);
  } else if (!tvIsObject(&*cls_or_obj) && !tvIsClass(&*cls_or_obj)) {
    return fail();
  }
  return nullptr;
}

TypeOrReduced impl_builtin_type_structure(ISS& env, const php::Func* func,
                                          const FCallArgs& fca, bool no_throw) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  bool fail = false;
  auto const ts = impl_type_structure_opts(env, func, fca, fail);
  if (fail && !no_throw) {
    unreachable(env);
    return TBottom;
  }
  if (!ts) return NoReduced{};
  if (fca.numArgs() == 2) reduce(env, bc::PopC {});
  reduce(env, bc::PopC {});
  reduce(env, bc::Dict { ts });
  return Reduced{};
}

TypeOrReduced builtin_type_structure(ISS& env, const php::Func* func,
                                     const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  return impl_builtin_type_structure(env, func, fca, false);
}

TypeOrReduced builtin_type_structure_no_throw(ISS& env, const php::Func* func,
                                              const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  return impl_builtin_type_structure(env, func, fca, true);
}

const StaticString s_classname("classname");

TypeOrReduced builtin_type_structure_classname(ISS& env, const php::Func* func,
                                               const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);
  bool fail = false;
  auto const ts = impl_type_structure_opts(env, func, fca, fail);
  if (fail) {
    unreachable(env);
    return TBottom;
  }
  if (ts) {
    auto const classname_field = ts->get(s_classname.get());
    if (isStringType(classname_field.type())) {
      if (fca.numArgs() == 2) reduce(env, bc::PopC {});
      reduce(env, bc::PopC {},
             bc::String { classname_field.val().pstr });
      return Reduced{};
    }
  }
  return TSStr;
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

#define X(x, y)    const StaticString s_##x(#y);
  SPECIAL_BUILTINS
#undef X

bool handle_builtin(ISS& env, const php::Func* func, const FCallArgs& fca) {
  auto const name = func->cls
    ? makeStaticString(folly::sformat("{}::{}", func->cls->name, func->name))
    : func->name;
  auto result = [&]() -> TypeOrReduced {
#define X(x, y) if (name->isame(s_##x.get())) return builtin_##x(env, func, fca);
    SPECIAL_BUILTINS
    return NoReduced{};
#undef X
  }();
  return match<bool>(
    result,
    [&] (NoReduced) { return false; },
    [&] (Reduced) {
      for (int i = 0; i < kNumActRecCells; ++i) reduce(env, bc::PopU2 {});
      return true;
    },
    [&] (Type retType) {
      for (int i = 0; i < fca.numArgs(); ++i) popT(env);
      for (int i = 0; i < kNumActRecCells; ++i) popU(env);
      push(env, std::move(retType));
      return true;
    }
  );
}

//////////////////////////////////////////////////////////////////////

} // namespace

bool optimize_builtin(ISS& env, const php::Func* func, const FCallArgs& fca) {
  if (!will_reduce(env) ||
      any(env.collect.opts & CollectionOpts::Speculating) ||
      func->attrs & AttrNoFCallBuiltin ||
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
      fca.enforceInOut() ||
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

  return handle_builtin(env, func, fca);
}

bool handle_function_exists(ISS& env, const Type& name) {
  if (!name.strictSubtypeOf(BStr)) return false;
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
  assertx(phpFunc.attrs & AttrIsFoldable);

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
    if (!isVecType(args.back().m_type)) return folly::none;
    auto const variadic = args.back();
    args.pop_back();
    IterateV(
      variadic.m_data.parr,
      [&](TypedValue v) { args.emplace_back(v); }
    );
  }

  FTRACE(1, "invoking: {}\n", func->fullName()->data());

  assertx(!RuntimeOption::EvalJit);
  return eval_cell(
    [&] {
      auto const retVal = g_context->invokeFuncFew(
        func, cls, args.size(), args.data(), RuntimeCoeffects::fixme(),
        false, false);
      assertx(tvIsPlausible(retVal));
      return retVal;
    }
  );
}

//////////////////////////////////////////////////////////////////////

}}
