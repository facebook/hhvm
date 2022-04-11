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
#include "hphp/runtime/vm/hackc-translator.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit-gen-helpers.h"
#include "hphp/zend/zend-string.h"

#include <folly/Range.h>

namespace HPHP {

TRACE_SET_MOD(hackc_translate);

namespace {

using namespace HPHP::hackc;
using namespace HPHP::hackc::hhbc;

struct TranslationException : Exception {
  template<class... A>
  explicit TranslationException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

[[noreturn]] void error(const char* what) {
  throw TranslationException("{}: {}", what, folly::errnoStr(errno));
}

struct TranslationState {
  UnitEmitter* ue;
  PreClassEmitter* pce{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
// hhbc::Slice helpers

template <class T>
folly::Range<const T*> range(Slice<T> const& s) {
  return folly::range(s.data, s.data + s.len);
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Maybe helpers

template<typename T>
Optional<T> maybe(hackc::Maybe<T> m) {
  if (m.tag == hackc::Maybe<T>::Tag::Nothing) return std::nullopt;
  return m.just._0;
}

template<typename T, typename Fn, typename ElseFn>
auto maybeOrElse(hackc::Maybe<T> m, Fn fn, ElseFn efn) {
  auto opt = maybe(m);
  return opt ? fn(opt.value()) : efn();
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Str Helpers

std::string toString(const Str& str) {
  assertx(str.data != nullptr);
  return std::string{(const char*)str.data, str.len};
}

StringData* toStaticString(const Str& str) {
  return makeStaticString((char*)str.data, str.len);
}

StringData* makeDocComment(const Str& str) {
  if (RuntimeOption::EvalGenerateDocComments) return toStaticString(str);
  return staticEmptyString();
}

///////////////////////////////////////////////////////////////////////////////

using kind = hhbc::TypedValue::Tag;

// TODO make arrays static
HPHP::TypedValue toTypedValue(const hackc::hhbc::TypedValue& tv) {
  auto hphp_tv = [&]() {
    switch(tv.tag) {
      case kind::Uninit:
        return make_tv<KindOfUninit>();
      case kind::Int:
        return make_tv<KindOfInt64>(tv.int_._0);
      case kind::Bool:
        return make_tv<KindOfBoolean>(tv.bool_._0);
      case kind::Double: {
        return make_tv<KindOfDouble>(tv.double_._0);
      }
    case kind::String: {
      auto const s = toStaticString(tv.string._0);
      return make_tv<KindOfPersistentString>(s);
    }
    case kind::Null:
      return make_tv<KindOfNull>();
    case kind::Vec: {
      VecInit v(tv.vec._0.len);
      auto set = range(tv.vec._0);
      for (auto const& elt : set) {
        v.append(toTypedValue(elt));
      }
      return make_tv<KindOfVec>(v.create());
    }
    case kind::Dict: {
      DictInit d(tv.dict._0.len);
      auto set = range(tv.dict._0);
      for (auto const& elt : set) {
        switch (elt._0.tag) {
          case kind::Int:
            d.set(elt._0.int_._0, toTypedValue(elt._1));
            break;
          case kind::String: {
            auto const s = toStaticString(elt._0.string._0);
            d.set(s, toTypedValue(elt._1));
            break;
          }
          default:
            always_assert(false);
        }
      }
      return make_tv<KindOfDict>(d.create());
    }
    case kind::Keyset: {
      KeysetInit k(tv.keyset._0.len);
      auto set = range(tv.keyset._0);
      for (auto const& elt : set) {
        k.add(toTypedValue(elt));
      }
      return make_tv<KindOfKeyset>(k.create());
    }
    case kind::LazyClass: {
      auto const lc = LazyClassData::create(toStaticString(tv.lazy_class._0));
      return make_tv<KindOfLazyClass>(lc);
    }
    case kind::HhasAdata:
      error("toTypedValue unimplemented for HhasAdata");
    }
    not_reached();
  }();
  checkSize(hphp_tv, RuntimeOption::EvalAssemblerMaxScalarSize);
  return hphp_tv;
}

///////////////////////////////////////////////////////////////////////////////
// Field translaters

void translateUserAttributes(Slice<HhasAttribute> attributes, UserAttributeMap& userAttrs) {
  Trace::Indent indent;
  auto attrs = range(attributes);
  for (auto const& attr : attrs) {
    auto const name = toStaticString(attr.name);
    VecInit v(attr.arguments.len);
    auto args = range(attr.arguments);
    for (auto const& arg : args) {
      v.append(toTypedValue(arg));
    }
    auto tv = v.create();
    ArrayData::GetScalarArray(&tv);
    userAttrs[name] = make_tv<KindOfVec>(tv);
  };
}

template<bool isEnum=false>
std::pair<const StringData*, TypeConstraint> translateTypeInfo(const HhasTypeInfo& t) {
  auto const user_type = maybeOrElse(t.user_type,
    [&](Str& s) {return toStaticString(s);},
    [&]() {return staticEmptyString();});

  auto const type_name = isEnum
    ? user_type
    : maybeOrElse(t.type_constraint.name,
        [&](Str& s) {return toStaticString(s);},
        [&]() {return nullptr;});

  auto flags = t.type_constraint.flags;
  return std::make_pair(user_type, TypeConstraint{type_name, flags});
}

void translateTypedef(TranslationState& ts, const HhasTypedef& t) {
  UserAttributeMap userAttrs;
  translateUserAttributes(t.attributes, userAttrs);
  auto attrs = t.attrs;
  if (!SystemLib::s_inited) attrs |= AttrPersistent;
  auto const name = toStaticString(t.name._0);

  auto const ty = translateTypeInfo(t.type_info).second;
  auto tname = ty.typeName();
  if (!tname) tname = staticEmptyString();

  auto tys = toTypedValue(t.type_structure);
  assertx(isArrayLikeType(tys.m_type));
  tvAsVariant(&tys).setEvalScalar();

  auto te = ts.ue->newTypeAliasEmitter(name->toCppString());
  te->init(
    t.span.line_begin,
    t.span.line_end,
    attrs,
    tname,
    tname->empty() ? AnnotType::Mixed : ty.type(),
    (ty.flags() & TypeConstraintFlags::Nullable) != 0,
    ArrNR{tys.m_data.parr},
    Array{}
  );

  te->setUserAttributes(userAttrs);
}

template<bool isType>
void addConstant(TranslationState& ts,
                 const StringData* name,
                 Optional<hhbc::TypedValue> tv,
                 bool isAbstract) {
  auto const kind = isType
    ? ConstModifiers::Kind::Type
    : ConstModifiers::Kind::Value;

  if (!tv) {
    assertx(isAbstract);
    ts.pce->addAbstractConstant(name, kind, false);
    return;
  }

  auto tvInit = toTypedValue(tv.value());
  ts.pce->addConstant(name,
                      nullptr,
                      &tvInit,
                      Array{},
                      kind,
                      PreClassEmitter::Const::Invariance::None,
                      false,
                      isAbstract);
}

void translateClassConstant(TranslationState& ts, const HhasConstant& c) {
  auto const name = toStaticString(c.name._0);
  auto const tv = maybe(c.value);
  addConstant<false>(ts, name, tv, c.is_abstract);
}

void translateTypeConstant(TranslationState& ts, const HhasTypeConstant& c) {
  auto const name = toStaticString(c.name);
  auto const tv = maybe(c.initializer);
  addConstant<true>(ts, name, tv, c.is_abstract);
}

void translateCtxConstant(TranslationState& ts, const HhasCtxConstant& c) {
  auto const name = toStaticString(c.name);
  bool isAbstract = c.is_abstract;
  auto coeffects = PreClassEmitter::Const::CoeffectsVec{};

  auto recognized = range(c.recognized);
  for (auto const& r : recognized) {
    coeffects.push_back(toStaticString(r));
  }
  auto unrecognized = range(c.unrecognized);
  for (auto const& u : unrecognized) {
    coeffects.push_back(toStaticString(u));
  }

  // T112974443: temporarily drop the abstract ones until runtime is fixed
  if (isAbstract && !RuntimeOption::EvalEnableAbstractContextConstants) return;
  DEBUG_ONLY auto added =
    ts.pce->addContextConstant(name, std::move(coeffects), isAbstract);
  assertx(added);
}

void translateProperty(TranslationState& ts, const HhasProperty& p, const UpperBoundMap& class_ubs) {
  UserAttributeMap userAttributes;
  translateUserAttributes(p.attributes, userAttributes);

  auto const heredoc = maybeOrElse(p.doc_comment,
    [&](Str& s) {return makeDocComment(s);},
    [&]() {return staticEmptyString();});

  auto [userTy, typeConstraint] = translateTypeInfo(p.type_info);

  auto const hasReifiedGenerics =
    userAttributes.find(s___Reified.get()) != userAttributes.end();

  // T112889109: Passing in {} as the third argument here exposes a gcc compiler bug.
  auto ub = getRelevantUpperBounds(typeConstraint, class_ubs, class_ubs, {});

  auto needsMultiUBs = false;
  if (ub.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(ub[0], typeConstraint);
    typeConstraint = ub[0];
  } else if (!ub.empty()) {
    needsMultiUBs = true;
  }

  auto const tv = maybeOrElse(p.initial_value,
    [&](hhbc::TypedValue& s) {return toTypedValue(s);},
    [&]() {return make_tv<KindOfNull>();});

  auto const name = toStaticString(p.name._0);
  ITRACE(2, "Translating property {} {}\n", name, tv.pretty());

  ts.pce->addProperty(name,
                      p.flags,
                      userTy,
                      typeConstraint,
                      needsMultiUBs ? std::move(ub) : UpperBoundVec{},
                      heredoc,
                      &tv,
                      HPHP::RepoAuthType{},
                      userAttributes);
}

void translateClassBody(TranslationState& ts,
                        const HhasClass& c,
                        const UpperBoundMap& class_ubs) {
  auto props = range(c.properties);
  for (auto const& p : props) {
    translateProperty(ts, p, class_ubs);
  }
  auto constants = range(c.constants);
  for (auto const& cns : constants) {
    translateClassConstant(ts, cns);
  }
  auto tconstants = range(c.type_constants);
  for (auto const& t : tconstants) {
    translateTypeConstant(ts, t);
  }
  auto cconstants = range(c.ctx_constants);
  for (auto const& ctx : cconstants) {
    translateCtxConstant(ts, ctx);
  }
}

using TypeInfoPair = Pair<Str, Slice<HhasTypeInfo>>;

void translateUbs(const TypeInfoPair& ub, UpperBoundMap& ubs) {
  auto const& name = toStaticString(ub._0);
  CompactVector<TypeConstraint> ret;

  auto infos = range(ub._1);
  for (auto const& i : infos) {
    ubs[name].emplace_back(translateTypeInfo(i).second);
  }
}

void translateEnumType(TranslationState& ts, const Maybe<HhasTypeInfo>& t) {
  auto const tOpt = maybe(t);
  if (tOpt) {
    ts.pce->setEnumBaseTy(translateTypeInfo<true>(tOpt.value()).second);
  }
}

void translateRequirements(TranslationState& ts, Pair<ClassName, TraitReqKind> requirement) {
  auto const name = toStaticString(requirement._0._0);
  auto const isExtends = requirement._1 == TraitReqKind::MustExtend;
  ts.pce->addClassRequirement(PreClass::ClassRequirement(name, isExtends));
}

using UseAlias = Quadruple<
  Maybe<ClassName>,
  ClassName,
  Maybe<ClassName>,
  Attr
>;
void translateUseAlias(TranslationState& ts, UseAlias useAlias) {
  auto const identifier = toStaticString(useAlias._1._0);
  auto const traitName = maybeOrElse(useAlias._0,
    [&](ClassName& c) {return toStaticString(c._0);},
    [&]() {return staticEmptyString();});

  auto const alias = maybeOrElse(useAlias._2,
    [&](ClassName& c) { return toStaticString(c._0);},
    [&]() {return identifier;});

  ts.pce->addTraitAliasRule(
    PreClass::TraitAliasRule(traitName, identifier, alias, useAlias._3)
  );
}

using UsePrecedence = Triple<ClassName, ClassName, Slice<ClassName>>;
void translateUsePrecedence(TranslationState& ts, UsePrecedence usePrecedence) {
  auto const traitName = toStaticString(usePrecedence._0._0);
  auto const identifier = toStaticString(usePrecedence._1._0);

  PreClass::TraitPrecRule precRule(traitName, identifier);
  auto otherTraits = range(usePrecedence._2);
  for (auto const& t : otherTraits) {
    precRule.addOtherTraitName(toStaticString(t._0));
  }
  ts.pce->addTraitPrecRule(precRule);
}

void translateClass(TranslationState& ts, const HhasClass& c) {
  UpperBoundMap ubs;
  auto upper_bounds = range(c.upper_bounds);
  for (auto const& u : upper_bounds) {
    translateUbs(u, ubs);
  }

  std::string name = toString(c.name._0);
  ITRACE(1, "Translating class {}\n", name);
  ts.pce = ts.ue->newPreClassEmitter(name);

  UserAttributeMap userAttrs;
  ITRACE(2, "Translating attribute list {}\n", c.attributes.len);
  translateUserAttributes(c.attributes, userAttrs);
  auto attrs = c.flags;
  if (!SystemLib::s_inited) attrs |= AttrUnique | AttrPersistent | AttrBuiltin;

  auto const parentName = maybeOrElse(c.base,
    [&](ClassName& s) { return toStaticString(s._0); },
    [&]() { return staticEmptyString(); });

  ts.pce->init(c.span.line_begin,
               c.span.line_end,
               attrs,
               parentName,
               staticEmptyString());

  auto const dc = maybe(c.doc_comment);
  if (dc) ts.pce->setDocComment(makeDocComment(dc.value()));
  ts.pce->setUserAttributes(userAttrs);

  auto impls = range(c.implements);
  for (auto const& i : impls) {
    ts.pce->addInterface(toStaticString(i._0));
  }
  auto incl = range(c.enum_includes);
  for (auto const& in : incl) {
    ts.pce->addEnumInclude(toStaticString(in._0));
  }

  auto requirements = range(c.requirements);
  for (auto const& r : requirements) {
    translateRequirements(ts, r);
  }

  auto uses = range(c.uses);
  for (auto const& u : uses) {
    ts.pce->addUsedTrait(toStaticString(u));
  }

  auto useAliases = range(c.use_aliases);
  for (auto const& ua : useAliases) {
    translateUseAlias(ts, ua);
  }

  auto usePrecedences = range(c.use_precedences);
  for (auto const& up : usePrecedences) {
    translateUsePrecedence(ts, up);
  }

  translateEnumType(ts, c.enum_type);
  translateClassBody(ts, c, ubs);
}

void translateConstant(TranslationState& ts, const HhasConstant& c) {
  Constant constant;
  constant.name = toStaticString(c.name._0);
  constant.attrs = SystemLib::s_inited ? AttrNone : AttrPersistent;

  constant.val = maybeOrElse(c.value,
    [&](hhbc::TypedValue& tv) {return toTypedValue(tv);},
    [&]() {return make_tv<KindOfNull>();});

  // An uninit constant means its actually a "dynamic" constant whose value
  // is evaluated at runtime. We store the callback in m_data.pcnt and invoke
  // on lookup. (see constant.cpp) It's used for things like STDERR.
  if (type(constant.val) == KindOfUninit) {
    constant.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Constant::get);
  }
  ts.ue->addConstant(constant);
}

void translateModuleUse(TranslationState& ts, const Optional<Str>& name) {
  if (!name) return;
  ts.ue->m_moduleName = toStaticString(name.value());
}

void translate(TranslationState& ts, const HackCUnit& unit) {
  auto classes = range(unit.classes);
  for (auto const& c : classes) {
    translateClass(ts, c);
  }

  auto constants = range(unit.constants);
  for (auto const& c : constants) {
    translateConstant(ts, c);
  }

  auto typedefs = range(unit.typedefs);
  for (auto const& t : typedefs) {
    translateTypedef(ts, t);
  }
  translateModuleUse(ts, maybe(unit.module_use));
}
}

std::unique_ptr<UnitEmitter> unitEmitterFromHackCUnit(
  const HackCUnit& unit,
  const char* filename,
	const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  const std::string& hhasString
) {
  auto const bcSha1 = SHA1{string_sha1(hhasString)};
  auto ue = std::make_unique<UnitEmitter>(sha1, bcSha1, nativeFuncs);
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;

  TranslationState ts{};
  ts.ue = ue.get();
  translate(ts, unit);
  ue->finish();
  return ue;
}
}
