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
#include "hphp/hhbbc/type-structure.h"

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/bespoke/type-structure.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace TS = TypeStructure;

//////////////////////////////////////////////////////////////////////

/*
 * NB: The below resolution logic must match the runtime *EXACTLY*.
 *
 * This includes the exact order in which fields are inserted into the
 * array.
 */

namespace {

// Whether the kind represents a type-structure which is inherently
// resolved. We don't need to look at those at all.
bool kind_is_resolved(TS::Kind kind) {
  switch (kind) {
    case TS::Kind::T_int:
    case TS::Kind::T_bool:
    case TS::Kind::T_float:
    case TS::Kind::T_string:
    case TS::Kind::T_num:
    case TS::Kind::T_arraykey:
    case TS::Kind::T_void:
    case TS::Kind::T_null:
    case TS::Kind::T_nothing:
    case TS::Kind::T_noreturn:
    case TS::Kind::T_mixed:
    case TS::Kind::T_dynamic:
    case TS::Kind::T_nonnull:
    case TS::Kind::T_resource:
    case TS::Kind::T_class:
    case TS::Kind::T_interface:
    case TS::Kind::T_xhp:
    case TS::Kind::T_enum:
    case TS::Kind::T_trait:
      return true;
    case TS::Kind::T_typevar:
    case TS::Kind::T_darray:
    case TS::Kind::T_varray:
    case TS::Kind::T_varray_or_darray:
    case TS::Kind::T_dict:
    case TS::Kind::T_vec:
    case TS::Kind::T_keyset:
    case TS::Kind::T_vec_or_dict:
    case TS::Kind::T_any_array:
    case TS::Kind::T_tuple:
    case TS::Kind::T_shape:
    case TS::Kind::T_unresolved:
    case TS::Kind::T_typeaccess:
    case TS::Kind::T_fun:
    case TS::Kind::T_reifiedtype:
    case TS::Kind::T_union:
      return false;
  }
  always_assert(false);
}

using Resolution = TypeStructureResolution;

Resolution&& maybeMakeBespoke(Resolution&& r) {
  // convert type structure into bespoke versions if possible
  if (!RO::EvalEmitBespokeTypeStructures) return std::move(r);

  if (auto const t = tv(r.type)) {
    auto const ad = val(*t).parr;
    if (!bespoke::TypeStructure::isValidTypeStructure(ad)) return std::move(r);

    if (auto const ts = bespoke::TypeStructure::MakeFromVanillaStatic(ad, false)) {
      r.type = dict_val(ts);
    }
  }
  return std::move(r);
}

// Builder encapsulates the resolution result being built. The user
// can simply use the pre-defined mutations and not worry about
// dealing with type mutations.
struct Builder {
  static Builder dict() {
    return Builder{ Resolution{ dict_empty(), false }};
  }
  static Builder vec() {
    return Builder{ Resolution{ vec_empty(), false }};
  }

  static Builder copy(SArray from, TS::Kind kind) {
    assertx(from->isStatic());
    assertx(from->isDictType());
    auto b = dict()
      .copyModifiers(from)
      .set(s_kind, make_tv<KindOfInt64>((int64_t)kind));
    if (from->exists(s_allows_unknown_fields)) {
      b.set(s_allows_unknown_fields, make_tv<KindOfBoolean>(true));
    }
    return b;
  }

  static Builder attach(Resolution r) { return Builder{std::move(r)}; }

  bool alwaysFails() const { return r.type.is(BBottom); }

  Builder& copyModifiers(SArray from) {
    assertx(from->isStatic());
    assertx(from->isDictType());
    if (from->exists(s_nullable)) set(s_nullable, make_tv<KindOfBoolean>(true));
    if (from->exists(s_soft))     set(s_soft, make_tv<KindOfBoolean>(true));
    return *this;
  }

  Builder& optCopy(const StaticString& key, SArray from) {
    assertx(from->isStatic());
    assertx(from->isDictType());
    auto const v = from->get(key);
    if (!v.is_init()) return *this;
    set(key, v);
    return *this;
  }

  template <typename C, typename F>
  Builder& resolve(const StaticString& key, SArray v,
                   C& c, const F& f) {
    if (!v) return *this;
    if (r.type.is(BBottom)) return *this;
    set(key, f(c, v));
    return *this;
  }

  Builder& set(const StringData* key, const Resolution& o) {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(IMPLIES(o.type.is(BBottom), o.mightFail));
    assertx(r.type.subtypeOf(BDict));
    assertx(o.type.subtypeOf(BDict) || o.type.subtypeOf(BVec));
    assertx(key->isStatic());
    if (r.type.is(BBottom)) return *this;
    if (o.type.is(BBottom)) {
      r.type = TBottom;
      r.mightFail = true;
      return *this;
    }
    auto s = array_like_set(std::move(r.type), sval(key), o.type);
    assertx(!s.second);
    r.type = std::move(s.first);
    r.mightFail |= o.mightFail;
    return *this;
  }

  Builder& set(const StaticString& key, const Resolution& o) {
    return set(key.get(), o);
  }

  Builder& set(const Type& key, const Resolution& o) {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(IMPLIES(o.type.is(BBottom), o.mightFail));
    assertx(r.type.subtypeOf(BDict));
    assertx(o.type.subtypeOf(BDict) || o.type.subtypeOf(BVec));
    assertx(key.subtypeOf(BArrKey));
    if (r.type.is(BBottom)) return *this;
    if (key.is(BBottom) || o.type.is(BBottom)) {
      r.type = TBottom;
      r.mightFail = true;
      return *this;
    }
    auto s = array_like_set(std::move(r.type), key, o.type);
    assertx(!s.second);
    r.type = std::move(s.first);
    r.mightFail |= o.mightFail;
    return *this;
  }

  Builder& set(const StringData* key, TypedValue v) {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(r.type.subtypeOf(BDict));
    assertx(key->isStatic());
    if (r.type.is(BBottom)) return *this;
    auto s = array_like_set(std::move(r.type), sval(key), from_cell(v));
    assertx(!s.second);
    r.type = std::move(s.first);
    return *this;
  }

  Builder& set(const StaticString& key, TypedValue v) {
    return set(key.get(), v);
  }

  Builder& append(const Resolution& o) {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(IMPLIES(o.type.is(BBottom), o.mightFail));
    assertx(r.type.subtypeOf(BVec));
    assertx(o.type.subtypeOf(BDict) || o.type.subtypeOf(BVec));
    if (r.type.is(BBottom)) return *this;
    if (o.type.is(BBottom)) {
      r.type = TBottom;
      r.mightFail = true;
      return *this;
    }
    auto a = array_like_newelem(std::move(r.type), o.type);
    assertx(!a.second);
    r.type = std::move(a.first);
    r.mightFail |= o.mightFail;
    return *this;
  }

  Resolution&& finish() {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(r.type.subtypeOf(BDict) || r.type.subtypeOf(BVec));
    return std::move(r);
  }

  Resolution&& finishTS() {
    assertx(IMPLIES(r.type.is(BBottom), r.mightFail));
    assertx(r.type.subtypeOf(BDict));

    return maybeMakeBespoke(std::move(r));
  }

private:
  explicit Builder(Resolution r) : r{std::move(r)} {}
  Resolution r;
};

using GenericsMap = hphp_fast_map<std::string, Type>;

struct Cache;

struct ResolveCtx {
  ResolveCtx(Context ctx, const IIndex* index, Cache* cache)
    : ctx{ctx}, index{index}, cache{cache} {}
  Context ctx;
  const IIndex* index;
  Cache* cache;
  const CollectedInfo* collect = nullptr;
  const php::Class* selfCls = nullptr;
  const php::Class* thisCls = nullptr;
  const GenericsMap* generics = nullptr;
};

struct Cache {
  struct Key {
    SArray ts;
    const php::Class* selfCls;
    const php::Class* thisCls;
    Optional<GenericsMap> generics;
  };

  struct KeyProxy {
    SArray ts;
    const ResolveCtx& ctx;
  };

  struct GenericsHasher {
    size_t operator()(const std::pair<std::string, Type>& p) const {
      return folly::hash::hash_128_to_64(
        folly::hasher<std::string>{}(p.first),
        p.second.hash()
      );
    }
  };

  struct KeyHasher {
    using is_transparent = void;

    size_t hashGenericsMap(const GenericsMap* g) const {
      if (!g) return 0;
      return folly::hash::commutative_hash_combine_range_generic(
        1,
        GenericsHasher{},
        g->begin(),
        g->end()
      );
    }

    size_t operator()(const Key& k) const {
      return folly::hash::hash_combine(
        k.ts,
        k.selfCls,
        k.thisCls,
        hashGenericsMap(k.generics.get_pointer())
      );
    }

    size_t operator()(const KeyProxy& k) const {
      return folly::hash::hash_combine(
        k.ts,
        k.ctx.selfCls,
        k.ctx.thisCls,
        hashGenericsMap(k.ctx.generics)
      );
    }
  };

  struct KeyEquals {
    using is_transparent = void;
    bool operator()(const Key& k1, const Key& k2) const {
      return
        std::tie(k1.ts, k1.selfCls, k1.thisCls, k1.generics) ==
        std::tie(k2.ts, k2.selfCls, k2.thisCls, k2.generics);
    }
    bool operator()(const KeyProxy& k1, const Key& k2) const {
      if (std::tie(k1.ts, k1.ctx.selfCls, k1.ctx.thisCls) !=
          std::tie(k2.ts, k2.selfCls, k2.thisCls)) {
        return false;
      }
      if (!k1.ctx.generics) return !k2.generics.has_value();
      if (!k2.generics.has_value()) return false;
      return *k1.ctx.generics == *k2.generics;
    }
  };

  hphp_fast_map<Key, Resolution, KeyHasher, KeyEquals> m_cache;

  Optional<Resolution> enter(const ResolveCtx& ctx, SArray ts) {
    auto const insertion = m_cache.emplace(
      Key{
        ts, ctx.selfCls, ctx.thisCls,
        ctx.generics ? *ctx.generics : Optional<GenericsMap>{}
      },
      Resolution{ TDictN, true }
    );
    if (insertion.second) return std::nullopt;
    return insertion.first->second;
  }

  void exit(const ResolveCtx& ctx, SArray ts, Resolution r) {
    auto const it = m_cache.find(KeyProxy{ts, ctx});
    assertx(it != m_cache.end());
    it->second = std::move(r);
  }
};

// Map a name into an appropriate TCls specialization.
Type name_to_cls_type(ResolveCtx& ctx, SString name) {
  auto const resolveCls = [&] (const php::Class* cls) {
    auto const rcls = ctx.index->resolve_class(cls->name);
    if (!rcls) return TBottom;
    return clsExact(*rcls);
  };
  auto const resolveStr = [&] (const StringData* str) {
    auto const rcls = ctx.index->resolve_class(str);
    if (!rcls) return TBottom;
    return clsExact(*rcls);
  };

  if (ctx.selfCls) {
    if (name->tsame(s_hh_this.get())) {
      if (ctx.thisCls) return resolveCls(ctx.thisCls);
      auto const rcls = ctx.index->resolve_class(ctx.selfCls->name);
      if (!rcls) return TBottom;
      return subCls(*rcls);
    }

    if (name->tsame(s_self.get())) return resolveCls(ctx.selfCls);

    if (name->tsame(s_parent.get())) {
      if (!ctx.selfCls->parentName) return TBottom;
      return resolveStr(ctx.selfCls->parentName);
    }
  }

  while (true) {
    auto const lookup = ctx.index->lookup_class_or_type_alias(name);
    if (lookup.cls) {
      auto const rcls = ctx.index->resolve_class(*lookup.cls);
      if (!rcls) return TBottom;
      return clsExact(*rcls);
    }
    if (!lookup.typeAlias) return lookup.maybeExists ? TCls : TBottom;
    auto const typeAlias = lookup.typeAlias;
    assertx(typeAlias->typeStructure.exists(s_kind));
    if (!typeAlias->typeStructure.exists(s_classname)) return TBottom;
    auto const tv = typeAlias->typeStructure->get(s_classname);
    assertx(tvIsString(tv));
    name = val(tv).pstr;
  }

  not_reached();
}

Resolution resolve(ResolveCtx&, SArray ts);
Resolution resolveBespoke(ResolveCtx& ctx, SArray ts);

Resolution resolve_list(ResolveCtx& ctx, SArray ts) {
  assertx(ts->isStatic());
  assertx(ts->isVecType());
  auto const size = ts->size();
  auto b = Builder::vec();
  for (size_t i = 0; i < size; ++i) {
    auto const tv = ts->get(i);
    assertx(tvIsDict(tv));
    auto const r = resolveBespoke(ctx, val(tv).parr);
    b.append(r);
    if (b.alwaysFails()) break;
  }
  return b.finish();
}

Optional<std::vector<Resolution>> resolve_list_separate(ResolveCtx& ctx,
                                                        SArray ts) {
  assertx(ts->isStatic());
  assertx(ts->isVecType());
  auto const size = ts->size();
  std::vector<Resolution> out;
  out.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    auto const tv = ts->get(i);
    assertx(tvIsDict(tv));
    out.emplace_back(resolveBespoke(ctx, val(tv).parr));
    if (out.back().type.is(BBottom)) return std::nullopt;
  }
  return out;
}

Resolution resolve_fun(ResolveCtx& ctx, SArray ts) {
  return Builder::copy(ts, TS::Kind::T_fun)
    .resolve(s_return_type, get_ts_return_type(ts), ctx, resolveBespoke)
    .resolve(s_param_types, get_ts_param_types(ts), ctx, resolve_list)
    .resolve(s_variadic_type, get_ts_variadic_type_opt(ts), ctx, resolveBespoke)
    .optCopy(s_typevars, ts)
    .optCopy(s_alias, ts)
    .optCopy(s_case_type, ts)
    .finish();
}

Resolution resolve_tuple(ResolveCtx& ctx, SArray ts) {
  return Builder::copy(ts, TS::Kind::T_tuple)
    .resolve(s_elem_types, get_ts_elem_types(ts), ctx, resolve_list)
    .optCopy(s_typevars, ts)
    .optCopy(s_alias, ts)
    .optCopy(s_case_type, ts)
    .finish();
}

Resolution resolve_arraylike(ResolveCtx& ctx, TS::Kind kind, SArray ts) {
  return Builder::copy(ts, kind)
    .resolve(s_generic_types, get_ts_generic_types_opt(ts), ctx, resolve_list)
    .optCopy(s_typevars, ts)
    .optCopy(s_alias, ts)
    .optCopy(s_case_type, ts)
    .finish();
}

Resolution resolve_typevar(ResolveCtx& ctx, SArray ts) {
  auto const name = get_ts_name(ts);
  if (name->tsame(s_wildcard.get())) {
    return Resolution { dict_val(ts), false };
  }
  if (!ctx.generics) return Resolution { TDictN, false };
  auto const it = ctx.generics->find(name->toCppString());
  if (it == ctx.generics->end()) {
    return Resolution { dict_val(ts), false };
  }
  return Resolution { it->second, false };
}

Resolution resolve_shape(ResolveCtx& ctx, SArray ts) {
  auto const fields = get_ts_fields(ts);
  assertx(fields->isStatic());
  assertx(fields->isDictType());

  auto const size = fields->size();

  auto b = Builder::dict();
  for (size_t i = 0; i < size; ++i) {
    auto const fieldValue = fields->nvGetVal(i);
    assertx(tvIsArrayLike(fieldValue));
    auto const fieldValueArr = val(fieldValue).parr;

    auto const key = [&] {
      auto const fieldKey = fields->nvGetKey(i);
      assertx(tvIsString(fieldKey) || tvIsInt(fieldKey));

      if (!fieldValueArr->exists(s_is_cls_cns)) {
        return std::make_pair(from_cell(fieldKey), false);
      }

      assertx(tvIsString(fieldKey));
      auto const clsCns = val(fieldKey).pstr->toCppString();
      std::string clsNameStr, cnsNameStr;
      folly::split("::", clsCns, clsNameStr, cnsNameStr);

      auto const clsName = makeStaticString(clsNameStr);
      auto const cnsName = makeStaticString(cnsNameStr);

      auto const fails = [&] {
        return std::make_pair(TBottom, true);
      };

      auto const cls = name_to_cls_type(ctx, clsName);
      if (cls.is(BBottom)) return fails();

      auto lookup = lookupClsConstant(
        *ctx.index,
        ctx.ctx,
        ctx.collect,
        cls,
        sval(cnsName)
      );
      if (lookup.found == TriBool::No) return fails();
      if (!lookup.ty.couldBe(BArrKey | BCls | BLazyCls)) return fails();

      auto const noFail =
        lookup.found == TriBool::Yes &&
        !lookup.mightThrow &&
        lookup.ty.subtypeOf(BArrKey);

      return std::make_pair(
        intersection_of(loosen_likeness(std::move(lookup.ty)), TArrKey),
        !noFail
      );
    }();

    auto resolution = [&] {
      auto const toResolve = [&] () -> SArray {
        auto const v = fieldValueArr->get(s_value);
        if (!v.is_init()) return fieldValueArr;
        // Runtime will attempt to coerce this to an array if it's not.
        if (!tvIsDict(v)) return nullptr;
        return val(v).parr;
      }();
      if (!toResolve) return Resolution { TDictN, true };

      auto r = resolve(ctx, toResolve);
      if (!fieldValueArr->exists(s_optional_shape_field)) return r;

      return Builder::attach(std::move(r))
        .set(s_optional_shape_field, make_tv<KindOfBoolean>(true))
        .finish();
    }();
    resolution.mightFail |= key.second;

    b.set(key.first, maybeMakeBespoke(std::move(resolution)));
    if (b.alwaysFails()) break;
  }

  return Builder::copy(ts, TS::Kind::T_shape)
    .set(s_fields, b.finish())
    .optCopy(s_typevars, ts)
    .optCopy(s_alias, ts)
    .optCopy(s_case_type, ts)
    .finishTS();
}

Resolution resolve_type_access_list(ResolveCtx& ctx,
                                    const Type& clsType,
                                    SArray accessList,
                                    size_t idx) {
  assertx(!clsType.is(BBottom));
  assertx(accessList->isStatic());
  assertx(accessList->isVecType());
  assertx(accessList->size() > 0);

  auto const cnsName = accessList->at(idx);
  assertx(tvIsString(cnsName));

  // Use a custom resolver. Instead of merely returning the result, we
  // continue down the access-list and resolve the entire thing. This
  // produces *much* better types (since we're unioning together all
  // of the final results, not intermediate steps).
  auto lookup = ctx.index->lookup_class_type_constant(
    clsType, sval(val(cnsName).pstr),
    [&] (const php::Const& cns, const php::Class& thiz) {
      assertx(cns.kind == ConstModifiers::Kind::Type);
      assertx(cns.val.has_value());
      assertx(tvIsDict(*cns.val));
      assertx(val(*cns.val).parr->isStatic());

      auto r = [&] {
        if (cns.resolvedTypeStructure) {
          return Resolution { dict_val(cns.resolvedTypeStructure), false };
        }
        ResolveCtx newCtx{ctx.ctx, ctx.index, ctx.cache};
        newCtx.selfCls = ctx.index->lookup_const_class(cns);
        if (!newCtx.selfCls) return Resolution{ TDictN, true };
        newCtx.thisCls = &thiz;
        newCtx.generics = ctx.generics;
        return resolve(newCtx, val(*cns.val).parr);
      }();

      assertx(r.type.subtypeOf(BDictN));
      if (r.type.is(BBottom)) return Resolution{ TBottom, true };
      if (idx == accessList->size() - 1) return r;

      auto const [kindTy, kindPresent] =
        array_like_elem(r.type, sval(s_kind.get()));
      auto const kind = tv(kindTy);
      if (!kind || !tvIsInt(*kind)) return Resolution{ TDictN, true };

      switch ((TS::Kind)val(*kind).num) {
        case TS::Kind::T_class:
        case TS::Kind::T_interface:
          break;
        default:
          return Resolution{ TBottom, true };
      }

      auto const [clsNameTy, clsNamePresent] =
        array_like_elem(r.type, sval(s_classname.get()));
      auto const clsName = tv(clsNameTy);
      if (!clsName || !tvIsString(*clsName)) {
        return Resolution{ TDictN, true };
      }

      auto const rcls = ctx.index->resolve_class(val(*clsName).pstr);
      if (!rcls) return Resolution{ TBottom, true };

      auto next =
        resolve_type_access_list(ctx, clsExact(*rcls), accessList, idx+1);
      next.mightFail |= (!kindPresent || !clsNamePresent);
      return next;
    }
  );
  assertx(lookup.resolution.type.subtypeOf(BSDictN));
  if (lookup.found == TriBool::No) return Resolution{ TBottom, true };

  auto const mightFail =
    lookup.found != TriBool::Yes ||
    lookup.resolution.mightFail;
  return Resolution{ std::move(lookup.resolution.type), mightFail };
}

Resolution resolve_type_access(ResolveCtx& ctx, SArray ts) {
  auto const clsType = name_to_cls_type(ctx, get_ts_root_name(ts));
  if (clsType.is(BBottom)) return Resolution{ TBottom, true };
  return Builder::attach(
    resolve_type_access_list(ctx, clsType, get_ts_access_list(ts), 0)
  ).copyModifiers(ts)
   .optCopy(s_alias, ts)
   .optCopy(s_case_type, ts)
   .finish();
}

Resolution resolve_unresolved(ResolveCtx& ctx, SArray ts) {
  auto b = Builder::copy(ts, TS::Kind::T_unresolved);

  auto const clsName = get_ts_classname(ts);

  auto const setKindAndName = [&] (TS::Kind k, SString n) {
    return Builder::copy(ts, TS::Kind::T_unresolved)
      .set(s_kind, make_tv<KindOfInt64>((int64_t)k))
      .set(s_classname, make_tv<KindOfPersistentString>(n));
  };

  if (clsName->tsame(s_callable.get())) {
    return setKindAndName(TS::Kind::T_class, clsName)
      .optCopy(s_typevars, ts)
      .optCopy(s_alias, ts)
      .optCopy(s_case_type, ts)
      .finish();
  }

  auto const resolvedCls = [&] (SString name, Attr attrs, bool setExact) {
    auto const kind = [&] {
      if (attrs & AttrEnum)      return TypeStructure::Kind::T_enum;
      if (attrs & AttrTrait)     return TypeStructure::Kind::T_trait;
      if (attrs & AttrInterface) return TypeStructure::Kind::T_interface;
      return TypeStructure::Kind::T_class;
    }();

    auto b = setKindAndName(kind, name);
    if (setExact) b.set(s_exact, make_tv<KindOfBoolean>(true));
    return
      b.resolve(s_generic_types, get_ts_generic_types_opt(ts),
                ctx, resolve_list)
        .optCopy(s_typevars, ts)
        .optCopy(s_alias, ts)
        .optCopy(s_case_type, ts)
        .finish();
  };

  if (ctx.selfCls) {
    if (clsName->tsame(s_hh_this.get())) {
      if (ctx.thisCls) {
        return resolvedCls(ctx.thisCls->name, ctx.thisCls->attrs, true);
      }
      if (ctx.selfCls->attrs & AttrNoOverride) {
        return resolvedCls(ctx.selfCls->name, ctx.selfCls->attrs, true);
      }

      auto const rcls = ctx.index->resolve_class(ctx.selfCls->name);
      if (!rcls) return Resolution{ TBottom, true };

      Optional<Resolution> resolution;
      auto const knowsChildren = rcls->forEachSubclass(
        [&] (SString name, Attr attrs) {
          if (!resolution) {
            resolution.emplace(resolvedCls(name, attrs, true));
          } else {
            *resolution |= resolvedCls(name, attrs, true);
          }
        }
      );
      if (!knowsChildren) return Resolution{ TDictN, true };
      if (!resolution) return Resolution{ TBottom, true };
      return *resolution;
    }

    if (clsName->tsame(s_self.get())) {
      return resolvedCls(ctx.selfCls->name, ctx.selfCls->attrs, false);
    }

    if (clsName->tsame(s_parent.get())) {
      if (!ctx.selfCls->parentName) return Resolution { TBottom, true };
      auto const rcls = ctx.index->resolve_class(ctx.selfCls->parentName);
      if (!rcls) return Resolution { TBottom, true };
      if (auto const cls = rcls->cls()) {
        return resolvedCls(cls->name, cls->attrs, false);
      }
      return Resolution{ TDictN, true };
    }
  }

  auto const lookup = ctx.index->lookup_class_or_type_alias(clsName);
  if (lookup.cls) {
    return resolvedCls(lookup.cls->name, lookup.cls->attrs, false);
  }
  if (!lookup.typeAlias) {
    return Resolution { lookup.maybeExists ? TDictN : TBottom, true };
  }
  auto const typeAlias = lookup.typeAlias;
  assertx(!typeAlias->typeStructure.empty());
  assertx(typeAlias->typeStructure.isDict());

  using TypevarTypes = std::vector<std::pair<std::string, Type>>;

  auto const resolveTA =
    [&, typeAlias=typeAlias] (const GenericsMap& g = {},
                              const TypevarTypes* typevarTypes = nullptr) {
    auto b = [&, typeAlias=typeAlias] {
      auto const& preresolved = typeAlias->resolvedTypeStructure;
      if (!preresolved.isNull()) {
        assertx(preresolved.isDict());
        return Builder::attach(
          Resolution{ dict_val(preresolved.get()), false }
        );
      }

      ResolveCtx newCtx{ctx.ctx, ctx.index, ctx.cache};
      newCtx.generics = &g;

      auto const toResolve = [&] {
        if (!typevarTypes) return typeAlias->typeStructure.get();
        auto removed = typeAlias->typeStructure;
        removed.remove(s_typevars);
        removed.setEvalScalar();
        return removed.get();
      }();

      return Builder::attach(resolve(newCtx, toResolve))
        .set(typeAlias->caseType ? s_case_type : s_alias,
             make_tv<KindOfPersistentString>(clsName));
    }();

    if (typevarTypes) {
      auto d = Builder::dict();
      for (auto const& kv : *typevarTypes) {
        d.set(
          makeStaticString(kv.first),
          maybeMakeBespoke(Resolution { kv.second, false })
        );
      }
      b.set(s_typevar_types, d.finish());
    }
    return b.copyModifiers(ts).finish();
  };

  if (typeAlias->typeStructure.exists(s_typevars) &&
      ts->exists(s_generic_types)) {
    std::vector<std::string> typevars;
    folly::split(
      ',',
      typeAlias->typeStructure[s_typevars].asCStrRef().data(),
      typevars
    );

    auto rgenerics = resolve_list_separate(ctx, get_ts_generic_types(ts));
    if (!rgenerics) return Resolution { TBottom, true };

    auto mightFail = false;
    for (auto const& r : *rgenerics) {
      mightFail |= r.mightFail;
      if (mightFail) break;
    }

    auto const size = std::min<size_t>(typevars.size(), rgenerics->size());

    GenericsMap genericsMap;
    TypevarTypes typevarTypes;
    for (size_t i = 0; i < size; ++i) {
      typevarTypes.emplace_back(typevars[i], (*rgenerics)[i].type);
      genericsMap.insert_or_assign(typevars[i], std::move((*rgenerics)[i].type));
    }
    auto r = resolveTA(genericsMap, &typevarTypes);
    r.mightFail |= mightFail;
    return r;
  }
  return resolveTA();
}

Resolution resolve_union(ResolveCtx& ctx, SArray ts) {
  return Builder::copy(ts, TS::Kind::T_union)
    .resolve(s_union_types, get_ts_union_types(ts), ctx, resolve_list)
    .finish();
}

Resolution resolve_impl(ResolveCtx& ctx, TS::Kind kind, SArray ts) {
  assertx(ts->isStatic());
  assertx(ts->isDictType());

  switch (kind) {
    case TS::Kind::T_fun:
      return resolve_fun(ctx, ts);
    case TS::Kind::T_tuple:
      return resolve_tuple(ctx, ts);
    case TS::Kind::T_darray:
    case TS::Kind::T_varray:
    case TS::Kind::T_varray_or_darray:
    case TS::Kind::T_dict:
    case TS::Kind::T_vec:
    case TS::Kind::T_keyset:
    case TS::Kind::T_vec_or_dict:
    case TS::Kind::T_any_array:
      return resolve_arraylike(ctx, kind, ts);
    case TS::Kind::T_shape:
      return resolve_shape(ctx, ts);
    case TS::Kind::T_typevar:
      return resolve_typevar(ctx, ts);
    case TS::Kind::T_unresolved:
      return resolve_unresolved(ctx, ts);
    case TS::Kind::T_typeaccess:
      return resolve_type_access(ctx, ts);
    case TS::Kind::T_reifiedtype:
      return Resolution{ TDictN, false };
    case TS::Kind::T_union:
      return resolve_union(ctx, ts);
    case TS::Kind::T_int:
    case TS::Kind::T_bool:
    case TS::Kind::T_float:
    case TS::Kind::T_string:
    case TS::Kind::T_num:
    case TS::Kind::T_arraykey:
    case TS::Kind::T_void:
    case TS::Kind::T_null:
    case TS::Kind::T_nothing:
    case TS::Kind::T_noreturn:
    case TS::Kind::T_mixed:
    case TS::Kind::T_dynamic:
    case TS::Kind::T_nonnull:
    case TS::Kind::T_resource:
    case TS::Kind::T_class:
    case TS::Kind::T_interface:
    case TS::Kind::T_xhp:
    case TS::Kind::T_enum:
    case TS::Kind::T_trait:
      // These should be already caught by resolve().
      break;
  }
  always_assert(false);
}

Resolution resolve(ResolveCtx& ctx, SArray ts) {
  // If the type-structure is trivially resolved, we can return it
  // immediately and skip the cache check.
  auto const kind = get_ts_kind(ts);
  if (kind_is_resolved(kind)) {
    return Resolution{ dict_val(ts), false };
  }

  if (auto const r = ctx.cache->enter(ctx, ts)) return *r;
  auto const r = resolve_impl(ctx, kind, ts);
  assertx(r.type.subtypeOf(BDictN));
  ctx.cache->exit(ctx, ts, r);
  return r;
}

Resolution resolveBespoke(ResolveCtx& ctx, SArray ts) {
  return maybeMakeBespoke(resolve(ctx, ts));
}

} // namespace

//////////////////////////////////////////////////////////////////////

Resolution resolve_type_structure(const ISS& env, SArray ts) {
  assertx(ts->isStatic());
  assertx(ts->isDictType());

  Cache cache;
  ResolveCtx ctx{env.ctx, &env.index, &cache};
  ctx.selfCls = env.ctx.cls;
  ctx.collect = &env.collect;
  return resolveBespoke(ctx, ts);
}

Resolution resolve_type_structure(const IIndex& index,
                                  const php::Const& cns,
                                  const php::Class& thiz) {
  assertx(cns.kind == ConstModifiers::Kind::Type);
  assertx(cns.val.has_value());
  assertx(tvIsDict(*cns.val));
  assertx(val(*cns.val).parr->isStatic());

  if (cns.resolvedTypeStructure) {
    assertx(cns.resolvedTypeStructure->isDictType());
    return Resolution { dict_val(cns.resolvedTypeStructure), false };
  }

  Cache cache;
  ResolveCtx ctx{Context{}, &index, &cache};
  ctx.selfCls = index.lookup_const_class(cns);
  ctx.thisCls = &thiz;

  // If the self class isn't present we need to be pessimistic.
  if (!ctx.selfCls) return Resolution { TDictN, true };
  return resolveBespoke(ctx, val(*cns.val).parr);
}

Resolution resolve_type_structure(const IIndex& index,
                                  const CollectedInfo* collect,
                                  const php::TypeAlias& typeAlias) {
  auto const& preresolved = typeAlias.resolvedTypeStructure;
  if (!preresolved.isNull()) {
    assertx(preresolved.isDict());
    assertx(!preresolved.empty());
    return Resolution { dict_val(preresolved.get()), false };
  }

  Cache cache;
  ResolveCtx ctx{Context{}, &index, &cache};
  ctx.collect = collect;
  return Builder::attach(resolve(ctx, typeAlias.typeStructure.get()))
    .set(typeAlias.caseType ? s_case_type : s_alias,
         make_tv<KindOfPersistentString>(typeAlias.name))
    .finishTS();
}

//////////////////////////////////////////////////////////////////////

}
