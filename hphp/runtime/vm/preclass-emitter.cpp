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
#include "hphp/runtime/vm/preclass-emitter.h"

#include <limits>

#include <folly/Memory.h>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

//=============================================================================
// PreClassEmitter::Prop.

PreClassEmitter::Prop::Prop(const PreClassEmitter* pce,
                            const StringData* n,
                            Attr attrs,
                            const StringData* userType,
                            const TypeConstraint& typeConstraint,
                            const UpperBoundVec& ubs,
                            const StringData* docComment,
                            const TypedValue* val,
                            RepoAuthType repoAuthType,
                            UserAttributeMap userAttributes)
  : m_name(n)
  , m_attrs(attrs)
  , m_userType(userType)
  , m_docComment(docComment)
  , m_repoAuthType(repoAuthType)
  , m_typeConstraint(typeConstraint)
  , m_ubs(ubs)
  , m_userAttributes(userAttributes)
{
  memcpy(&m_val, val, sizeof(TypedValue));
}

PreClassEmitter::Prop::~Prop() {
}

//=============================================================================
// PreClassEmitter.

PreClassEmitter::PreClassEmitter(UnitEmitter& ue,
                                 Id id,
                                 const std::string& n)
  : m_ue(ue)
  , m_name(makeStaticString(n))
  , m_id(id) {}

void PreClassEmitter::init(int line1, int line2, Attr attrs,
                           const StringData* parent,
                           const StringData* docComment) {
  m_line1 = line1;
  m_line2 = line2;
  m_attrs = attrs;
  m_parent = parent;
  m_docComment = docComment;
  if (!SystemLib::s_inited) {
    m_attrs = m_attrs | AttrBuiltin;
  }
}

PreClassEmitter::~PreClassEmitter() {
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    delete *it;
  }
}

void PreClassEmitter::addInterface(const StringData* n) {
  m_interfaces.push_back(n);
}

void PreClassEmitter::addEnumInclude(const StringData* n) {
  m_enumIncludes.push_back(n);
}

bool PreClassEmitter::addMethod(FuncEmitter* method) {
  MethodMap::const_iterator it = m_methodMap.find(method->name);
  if (it != m_methodMap.end()) {
    return false;
  }
  m_methods.push_back(method);
  m_methodMap[method->name] = method;
  return true;
}

void PreClassEmitter::renameMethod(const StringData* oldName,
                                   const StringData* newName) {
  assertx(m_methodMap.count(oldName));
  auto it = m_methodMap.find(oldName);
  auto fe = it->second;
  m_methodMap.erase(it);
  fe->name = newName;
  m_methodMap[newName] = fe;
}

bool PreClassEmitter::addProperty(const StringData* n, Attr attrs,
                                  const StringData* userType,
                                  const TypeConstraint& typeConstraint,
                                  const UpperBoundVec& ubs,
                                  const StringData* docComment,
                                  const TypedValue* val,
                                  RepoAuthType repoAuthType,
                                  UserAttributeMap userAttributes) {
  assertx(typeConstraint.validForProp());
  PropMap::Builder::const_iterator it = m_propMap.find(n);
  if (it != m_propMap.end()) {
    return false;
  }
  PreClassEmitter::Prop prop{
    this,
    n,
    attrs,
    userType,
    typeConstraint,
    ubs,
    docComment,
    val,
    repoAuthType,
    userAttributes
  };
  m_propMap.add(prop.name(), prop);
  return true;
}

bool PreClassEmitter::addAbstractConstant(const StringData* n,
                                          ConstModifiers::Kind kind,
                                          bool fromTrait) {
  assertx(kind == ConstModifiers::Kind::Value ||
          kind == ConstModifiers::Kind::Type);

  auto it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const cns(n, nullptr, nullptr,
                             {}, Array{}, kind,
                             Const::Invariance::None,
                             true, fromTrait);
  m_constMap.add(cns.name(), cns);
  return true;
}

bool PreClassEmitter::addContextConstant(
    const StringData* n,
    PreClassEmitter::Const::CoeffectsVec coeffects,
    bool isAbstract,
    bool fromTrait) {
  auto it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const cns(n, nullptr, nullptr,
                             std::move(coeffects),
                             Array{},
                             ConstModifiers::Kind::Context,
                             Const::Invariance::None,
                             isAbstract, fromTrait);
  m_constMap.add(cns.name(), cns);
  return true;
}

bool PreClassEmitter::addConstant(const StringData* n,
                                  const StringData* cls,
                                  const TypedValue* val,
                                  Array resolvedTypeStructure,
                                  ConstModifiers::Kind kind,
                                  Const::Invariance invariance,
                                  bool fromTrait,
                                  bool isAbstract) {
  assertx(kind == ConstModifiers::Kind::Value ||
          kind == ConstModifiers::Kind::Type);
  assertx(IMPLIES(kind == ConstModifiers::Kind::Value, !cls));
  assertx(IMPLIES(kind == ConstModifiers::Kind::Value,
                  resolvedTypeStructure.isNull()));
  assertx(IMPLIES(!resolvedTypeStructure.isNull(),
                  resolvedTypeStructure.isDict() &&
                  !resolvedTypeStructure.empty() &&
                  resolvedTypeStructure->isStatic()));
  assertx(IMPLIES(invariance != Const::Invariance::None,
                  !resolvedTypeStructure.isNull()));
  assertx(val);

  ConstMap::Builder::const_iterator it = m_constMap.find(n);
  if (it != m_constMap.end()) {
    return false;
  }
  PreClassEmitter::Const cns(n, cls, val, {}, std::move(resolvedTypeStructure),
                             kind, invariance, isAbstract, fromTrait);
  m_constMap.add(cns.name(), cns);
  return true;
}

void PreClassEmitter::addUsedTrait(const StringData* traitName) {
  m_usedTraits.push_back(traitName);
}

void PreClassEmitter::addTraitPrecRule(
    const PreClass::TraitPrecRule &rule) {
  m_traitPrecRules.push_back(rule);
}

void PreClassEmitter::addTraitAliasRule(
    const PreClass::TraitAliasRule &rule) {
  m_traitAliasRules.push_back(rule);
}

const StaticString
  s_nativedata("__nativedata"),
  s_DynamicallyConstructible("__DynamicallyConstructible"),
  s_invoke("__invoke"),
  s_coeffectsProp("86coeffects");

PreClass* PreClassEmitter::create(Unit& unit) const {
  Attr attrs = m_attrs;
  if (attrs & AttrPersistent &&
      !RuntimeOption::RepoAuthoritative && SystemLib::s_inited) {
    attrs = Attr(attrs & ~AttrPersistent);
  }

  auto const dynConstructSampleRate = [&] () -> int64_t {
    if (!(attrs & AttrDynamicallyConstructible)) return -1;

    auto const it = m_userAttributes.find(s_DynamicallyConstructible.get());
    if (it == m_userAttributes.end()) return -1;

    assertx(isArrayLikeType(type(it->second)));
    auto const rate = val(it->second).parr->get(int64_t(0));
    if (!isIntType(type(rate)) || val(rate).num < 0) return -1;

    attrs = Attr(attrs & ~AttrDynamicallyConstructible);
    return val(rate).num;
  }();

  auto const invoke = lookupMethod(s_invoke.get());
  if (invoke && invoke->isClosureBody) {
    attrs |= AttrIsClosureClass;
    if (!invoke->coeffectRules.empty()) {
      assertx(invoke->coeffectRules.size() == 1);
      if (invoke->coeffectRules[0].isClosureParentScope()) {
        attrs |= AttrHasClosureCoeffectsProp;
      } else {
        assertx(invoke->coeffectRules[0].isCaller());
      }
    }
  }

  assertx(attrs & AttrPersistent || SystemLib::s_inited);

  auto pc = std::make_unique<PreClass>(
    &unit, m_line1, m_line2, m_name,
    attrs, m_parent, m_docComment, m_id);
  pc->m_interfaces = m_interfaces;
  pc->m_includedEnums = m_enumIncludes;
  pc->m_usedTraits = m_usedTraits;
  pc->m_requirements = m_requirements;
  pc->m_traitPrecRules = m_traitPrecRules;
  pc->m_traitAliasRules = m_traitAliasRules;
  pc->m_enumBaseTy = m_enumBaseTy;
  pc->m_numDeclMethods = -1;
  pc->m_ifaceVtableSlot = m_ifaceVtableSlot;
  pc->m_dynConstructSampleRate = dynConstructSampleRate;

  // Set user attributes.
  [&] {
    pc->m_userAttributes = m_userAttributes;
    pc->m_nativeDataInfo = nullptr;
    if (!m_userAttributes.size()) return;

    // Check for <<__NativeData("Type")>>.
    auto it = m_userAttributes.find(s_nativedata.get());
    if (it == m_userAttributes.end()) return;

    TypedValue ndiInfo = it->second;
    if (!isArrayLikeType(ndiInfo.m_type)) return;

    // Use the first string label which references a registered type.  In
    // practice, there should generally only be one item and it should be a
    // string, but maybe that'll be extended...
    for (ArrayIter it(ndiInfo.m_data.parr); it; ++it) {
      Variant val = it.second();
      if (!val.isString()) continue;

      pc->m_nativeDataInfo = Native::getNativeDataInfo(val.toString().get());
      if (pc->m_nativeDataInfo) break;
    }
  }();

  PreClass::MethodMap::Builder methodBuild;
  for (MethodVec::const_iterator it = m_methods.begin();
       it != m_methods.end(); ++it) {
    Func* f = (*it)->create(unit, pc.get());
    if (f->attrs() & AttrTrait) {
      if (pc->m_numDeclMethods == -1) {
        pc->m_numDeclMethods = it - m_methods.begin();
      }
    } else if (!f->isGenerated()) {
      assertx(pc->m_numDeclMethods == -1);
    }
    methodBuild.add(f->name(), f);
  }
  pc->m_methods.create(methodBuild);

  PreClass::PropMap::Builder propBuild;
  if (pc->attrs() & AttrHasClosureCoeffectsProp) {
    TypedValue tvInit;
    tvWriteUninit(tvInit);

    propBuild.add(s_coeffectsProp.get(), PreClass::Prop(pc.get(),
      s_coeffectsProp.get(),
      AttrPrivate|AttrSystemInitialValue,
      staticEmptyString(),
      TypeConstraint(),
      CompactVector<TypeConstraint>{},
      staticEmptyString(),
      tvInit,
      RepoAuthType{},
      UserAttributeMap{}
    ));
  }
  for (unsigned i = 0; i < m_propMap.size(); ++i) {
    const Prop& prop = m_propMap[i];
    propBuild.add(prop.name(), PreClass::Prop(pc.get(),
                                              prop.name(),
                                              prop.attrs(),
                                              prop.userType(),
                                              prop.typeConstraint(),
                                              prop.upperBounds(),
                                              prop.docComment(),
                                              prop.val(),
                                              prop.repoAuthType(),
                                              prop.userAttributes()));
  }
  pc->m_properties.create(propBuild);

  PreClass::ConstMap::Builder constBuild;
  for (unsigned i = 0; i < m_constMap.size(); ++i) {
    const Const& const_ = m_constMap[i];
    TypedValueAux tvaux;
    tvaux.constModifiers() = {};
    tvaux.constModifiers().setIsAbstract(const_.isAbstract());
    if (const_.kind() == ConstModifiers::Kind::Context) {
      auto const coeffects = [&] {
        if (const_.coeffects().empty()) return StaticCoeffects::defaults();
        auto coeffects =
          CoeffectsConfig::fromName(const_.coeffects()[0]->toCppString());
        for (auto const& coeffect : const_.coeffects()) {
          coeffects |= CoeffectsConfig::fromName(coeffect->toCppString());
        }
        return coeffects;
      }();
      tvaux.constModifiers().setCoeffects(coeffects);
      if (!const_.coeffects().empty()) {
        tvCopy(make_tv<KindOfInt64>(0), tvaux); // dummy value for m_data
      } else {
        tvWriteConstValMissing(tvaux);
      }
    } else {
      if (const_.valOption()) {
        tvCopy(const_.val(), tvaux);
      } else {
        tvWriteConstValMissing(tvaux);
      }
    }

    tvaux.constModifiers().setKind(const_.kind());

    assertx(
      IMPLIES(const_.kind() != ConstModifiers::Kind::Type, !const_.cls())
    );
    assertx(
      IMPLIES(const_.kind() != ConstModifiers::Kind::Type,
              const_.resolvedTypeStructure().isNull())
    );
    assertx(
      IMPLIES(!const_.resolvedTypeStructure().isNull(),
              const_.resolvedTypeStructure().isDict() &&
              !const_.resolvedTypeStructure().empty())
    );
    assertx(
      IMPLIES(const_.invariance() != Const::Invariance::None,
              !const_.resolvedTypeStructure().isNull())
    );

    constBuild.add(
      const_.name(),
      PreClass::Const(
        const_.name(),
        const_.cls(),
        tvaux,
        const_.resolvedTypeStructure(),
        const_.invariance(),
        const_.isFromTrait()
      )
    );
  }
  if (auto nativeConsts = Native::getClassConstants(m_name)) {
    for (auto cnsMap : *nativeConsts) {
      TypedValueAux tvaux;
      tvCopy(cnsMap.second, tvaux);
      tvaux.constModifiers() = {};
      constBuild.add(cnsMap.first, PreClass::Const(cnsMap.first,
                                                   nullptr,
                                                   tvaux,
                                                   Array{},
                                                   Const::Invariance::None,
                                                   false));
    }
  }

  pc->m_constants.create(constBuild);
  return pc.release();
}

template<class SerDe> void PreClassEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name and a few other fields currently
  // serialized outside of this.
  sd(m_line1)
    (m_line2)
    (m_attrs)
    (m_parent)
    (m_docComment)
    (m_ifaceVtableSlot)

    (m_interfaces)
    (m_enumIncludes)
    (m_usedTraits)
    (m_requirements)
    (m_traitPrecRules)
    (m_traitAliasRules)
    (m_userAttributes)
    (m_propMap, [](Prop p) { return p.name(); })
    (m_constMap, [](Const c) { return c.name(); })
    (m_enumBaseTy)
    ;

    if (SerDe::deserializing) {
      for (unsigned i = 0; i < m_propMap.size(); ++i) {
        m_propMap[i].updateAfterDeserialize(this);
      }
    }
}

template void PreClassEmitter::serdeMetaData<>(BlobDecoder&);
template void PreClassEmitter::serdeMetaData<>(BlobEncoder&);

} // HPHP
