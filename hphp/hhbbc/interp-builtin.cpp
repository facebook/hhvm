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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/type-builtins.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP::HHBBC {

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
  auto const& d = dobj_of(ty);
  if (!d.isExact()) return TStr;
  constprop(env);
  return sval(d.cls().name());
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

TypeOrReduced builtin_is_numeric(ISS& env, const php::Func* func,
                                 const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto const ty = getArg(env, func, fca, 0);
  if (!ty.couldBe(BInt | BDbl | BStr)) return TFalse;
  if (ty.subtypeOf(BInt | BDbl)) return TTrue;
  if (ty.subtypeOf(BStr)) {
     if (auto const val = tv(ty)) {
       int64_t ival;
       double dval;
       auto const dt = val->m_data.pstr->toNumeric(ival, dval);
       return dt == KindOfInt64 || dt == KindOfDouble ? TTrue : TFalse;
     }
  }
  return TBool;
}

TypeOrReduced builtin_function_exists(ISS& env, const php::Func* func,
                                      const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);

  auto const name = getArg(env, func, fca, 0);
  if (!name.strictSubtypeOf(BStr)) return NoReduced{};
  auto const v = tv(name);
  if (!v) return NoReduced{};
  auto const rfunc = env.index.resolve_func(v->m_data.pstr);
  switch (rfunc.exists()) {
    case TriBool::Yes:
      constprop(env);
      return TTrue;
    case TriBool::No:
      constprop(env);
      return TFalse;
    case TriBool::Maybe:
      return NoReduced{};
  }
  not_reached();
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

TypeOrReduced builtin_package_exists(ISS& env, const php::Func* func,
                                     const FCallArgs& fca) {
  assertx(fca.numArgs() == 1);
  auto name = getArg(env, func, fca, 0);
  if (!name.strictSubtypeOf(BStr)) return NoReduced{};
  auto const v = tv(name);
  if (!v) return NoReduced{};
  auto const packageName = v->m_data.pstr;
  auto const unit = env.index.lookup_func_unit(*env.ctx.func);
  if (!unit) return NoReduced{};
  constprop(env);
  return unit->packageInfo.isPackageInActiveDeployment(packageName)
    ? TTrue : TFalse;
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
      auto const& dcls = dcls_of(ty);
      if (dcls.isExact()) {
        auto cname = dcls.cls().name();
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
  auto const res = [&]() -> Optional<bool> {
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
 * Optimize type_structure, type_structure_nothrow, and
 * type_structure_classname flavors. Returns the best known result
 * they produce, and whether they might throw. Returns std::nullopt if
 * no optimization should be done.
 */
Optional<std::pair<Type, TriBool>>
impl_builtin_type_structure(ISS& env, const php::Func* func,
                            const FCallArgs& fca,
                            bool no_throw_on_undefined) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);

  auto const name = getArg(env, func, fca, 1);

  if (name.subtypeOf(BInitNull)) {
    // Type-alias case:
    auto throws = TriBool::No;
    auto const clsStr = [&] () -> SString {
      auto const t = getArg(env, func, fca, 0);
      if (t.subtypeOf(BCls) && is_specialized_cls(t)) {
        auto const& dcls = dcls_of(t);
        if (!dcls.isExact()) return nullptr;
        if (RO::EvalRaiseClassConversionNoticeSampleRate > 0) {
          throws = TriBool::Maybe;
        }
        return dcls.cls().name();
      }
      if (t.subtypeOf(BStr) && is_specialized_string(t)) {
        return sval_of(t);
      }
      if (t.subtypeOf(BLazyCls) && is_specialized_lazycls(t)) {
        if (RO::EvalRaiseClassConversionNoticeSampleRate > 0) {
          throws = TriBool::Maybe;
        }
        return lazyclsval_of(t);
      }
      return nullptr;
    }();
    if (!clsStr) return std::nullopt;

    auto const [typeAlias, exists] = env.index.lookup_type_alias(clsStr);
    if (!exists) {
      // No type-alias with that name
      unreachable(env);
      return std::make_pair(TBottom, TriBool::Yes);
    }
    // It might exist, so be conservative.
    if (!typeAlias) return std::nullopt;

    // Found a type-alias, resolve it's type-structure.
    auto const r = resolve_type_structure(env.index, &env.collect, *typeAlias);
    if (r.type.is(BBottom)) {
      // Resolution will always fail
      unreachable(env);
      return std::make_pair(TBottom, TriBool::Yes);
    }

    return std::make_pair(
      r.type,
      noOrMaybe(throws == TriBool::No && !r.mightFail)
    );
  } else if (!name.subtypeOf(BStr)) {
    // Don't know whether it's a class or type-alias.
    return std::nullopt;
  }

  // Class constant case:

  auto throws = TriBool::No;

  auto const cls = [&] {
    auto const t = getArg(env, func, fca, 0);
    if (t.subtypeOf(BCls)) return t;
    if (t.subtypeOf(BObj)) return objcls(t);
    if (t.subtypeOf(BStr) && is_specialized_string(t)) {
      auto const str = sval_of(t);
      auto const rcls = env.index.resolve_class(str);
      if (!rcls) return TBottom;
      return clsExact(*rcls);
    }
    if (t.subtypeOf(BLazyCls) && is_specialized_lazycls(t)) {
      auto const str = lazyclsval_of(t);
      auto const rcls = env.index.resolve_class(str);
      if (!rcls) return TBottom;
      return clsExact(*rcls);
    }
    throws = TriBool::Maybe;
    return TCls;
  }();

  if (cls.is(BBottom)) {
    unreachable(env);
    return std::make_pair(TBottom, TriBool::Yes);
  }

  auto lookup = env.index.lookup_class_type_constant(cls, name);
  assertx(lookup.resolution.type.subtypeOf(BSDictN));

  // Match behavior of runtime and return "fake" resolution for
  // nothrow failure.
  auto const fake = [] {
    auto array = make_dict_array(
      s_kind,
      Variant(static_cast<uint8_t>(TypeStructure::Kind::T_class)),
      s_classname,
      Variant(s_type_structure_non_existant_class)
    );
    array.setEvalScalar();
    return dict_val(array.get());
  };

  switch (lookup.found) {
    case TriBool::No:
      assertx(!lookup.resolution.mightFail);
      assertx(lookup.resolution.type.is(BBottom));
      if (!no_throw_on_undefined || lookup.abstract == TriBool::No) {
        throws = TriBool::Yes;
      } else {
        if (lookup.abstract == TriBool::Maybe) throws = TriBool::Maybe;
        lookup.resolution.type = fake();
      }
      break;
    case TriBool::Maybe:
      if (!no_throw_on_undefined || lookup.abstract == TriBool::No) {
        throws = TriBool::Maybe;
      } else {
        if (lookup.abstract == TriBool::Maybe) throws = TriBool::Maybe;
        lookup.resolution.type |= fake();
      }
      break;
    case TriBool::Yes:
      assertx(lookup.abstract == TriBool::No);
      break;
  }

  if (lookup.resolution.mightFail) {
    if (no_throw_on_undefined) {
      lookup.resolution.type |= fake();
    } else if (throws != TriBool::Yes) {
      throws =
        lookup.resolution.type.is(BBottom) ? TriBool::Yes : TriBool::Maybe;
    }
  }

  return std::make_pair(lookup.resolution.type, throws);
}

TypeOrReduced builtin_type_structure(ISS& env, const php::Func* func,
                                     const FCallArgs& fca) {
  auto const r = impl_builtin_type_structure(env, func, fca, false);
  if (!r) return NoReduced{};
  switch (r->second) {
   case TriBool::Yes:
      unreachable(env);
      return TBottom;
    case TriBool::No:
      effect_free(env);
      constprop(env);
      return r->first;
    case TriBool::Maybe:
      return r->first;
  }
  always_assert(false);
}

TypeOrReduced builtin_type_structure_no_throw(ISS& env, const php::Func* func,
                                              const FCallArgs& fca) {
  auto const r = impl_builtin_type_structure(env, func, fca, true);
  if (!r) return NoReduced{};
  switch (r->second) {
   case TriBool::Yes:
      unreachable(env);
      return TBottom;
    case TriBool::No:
      effect_free(env);
      constprop(env);
      return r->first;
    case TriBool::Maybe:
      return r->first;
  }
  always_assert(false);
}

const StaticString s_classname("classname");

TypeOrReduced builtin_type_structure_classname(ISS& env, const php::Func* func,
                                               const FCallArgs& fca) {
  auto const r = impl_builtin_type_structure(env, func, fca, false);
  if (!r) return NoReduced{};
  if (r->second == TriBool::Yes) {
    unreachable(env);
    return TBottom;
  }
  assertx(r->first.subtypeOf(BDictN));

  auto const [classname, present] =
    array_like_elem(r->first, sval(s_classname.get()));
  if (classname.is(BBottom)) {
    unreachable(env);
    return TBottom;
  }
  if (!classname.couldBe(BSStr)) return NoReduced{};

  if (r->second == TriBool::No && present) {
    effect_free(env);
    constprop(env);
  }
  return intersection_of(classname, TSStr);
}

TypeOrReduced builtin_create_special_implicit_context(ISS& env,
                                                      const php::Func* func,
                                                      const FCallArgs& fca) {
  assertx(fca.numArgs() >= 1 && fca.numArgs() <= 2);

  auto const type = getArg(env, func, fca, 0);
  if (!type.subtypeOf(BInt)) return NoReduced{};

  if (fca.numArgs() == 1) {
    reduce(env, bc::Null {}, bc::CreateSpecialImplicitContext {});
  } else {
    if (!getArg(env, func, fca, 1).subtypeOf(BOptStr)) return NoReduced{};
    reduce(env, bc::CreateSpecialImplicitContext {});
  }
  return Reduced{};
}

#define SPECIAL_BUILTINS                                                \
  X(abs, abs)                                                           \
  X(ceil, ceil)                                                         \
  X(floor, floor)                                                       \
  X(get_class, get_class)                                               \
  X(max2, max2)                                                         \
  X(min2, min2)                                                         \
  X(strlen, strlen)                                                     \
  X(is_numeric, is_numeric)                                             \
  X(function_exists, function_exists)                                   \
  X(class_exists, class_exists)                                         \
  X(interface_exists, interface_exists)                                 \
  X(trait_exists, trait_exists)                                         \
  X(package_exists, HH\\package_exists)                                 \
  X(array_key_cast, HH\\array_key_cast)                                 \
  X(is_callable, is_callable)                                           \
  X(is_list_like, HH\\is_list_like)                                     \
  X(type_structure, HH\\type_structure)                                 \
  X(type_structure_no_throw, HH\\type_structure_no_throw)               \
  X(type_structure_classname, HH\\type_structure_classname)             \
  X(create_special_implicit_context, HH\\ImplicitContext\\_Private\\create_special_implicit_context)   \

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
  if (any(env.collect.opts & CollectionOpts::Speculating) ||
      func->attrs & AttrNoFCallBuiltin ||
      (func->cls && !(func->attrs & AttrStatic))  ||
      !func->isNative ||
      func->params.size() >= Native::maxFCallBuiltinArgs() ||
      fca.hasGenerics() ||
      !RuntimeOption::EvalEnableCallBuiltin) {
    return false;
  }

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

Optional<Type> const_fold(ISS& env,
                                 uint32_t nArgs,
                                 uint32_t numExtraInputs,
                                 const php::Func& phpFunc,
                                 bool variadicsPacked) {
  assertx(phpFunc.attrs & AttrIsFoldable);

  std::vector<TypedValue> args(nArgs);
  auto const firstArgPos = numExtraInputs + nArgs - 1;
  for (auto i = uint32_t{0}; i < nArgs; ++i) {
    auto const val = tv(topT(env, firstArgPos - i));
    if (!val || val->m_type == KindOfUninit) return std::nullopt;
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

  if (!func) return std::nullopt;

  // If the function is variadic and all the variadic parameters are already
  // packed into an array as the last parameter, we need to unpack them, as
  // invokeFuncFew expects them to be unpacked.
  if (func->hasVariadicCaptureParam() && variadicsPacked) {
    if (args.empty()) return std::nullopt;
    if (!isVecType(args.back().m_type)) return std::nullopt;
    auto const variadic = args.back();
    args.pop_back();
    IterateV(
      variadic.m_data.parr,
      [&](TypedValue v) { args.emplace_back(v); }
    );
  }

  FTRACE(1, "invoking: {}\n", func->fullName()->data());

  assertx(!RuntimeOption::EvalJit);
  // NB: Coeffects are already checked prior to here by `shouldAttemptToFold`
  return eval_cell(
    [&] {
      auto const retVal = g_context->invokeFuncFew(
        func, cls, args.size(), args.data(), RuntimeCoeffects::none(),
        false, false);
      assertx(tvIsPlausible(retVal));
      return retVal;
    }
  );
}

//////////////////////////////////////////////////////////////////////

}
