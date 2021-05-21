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

#include "hphp/runtime/vm/preclass.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/unit.h"

#include <ostream>
#include <sstream>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

PreClass::PreClass(Unit* unit, int line1, int line2, const StringData* n,
                   Attr attrs, const StringData* parent, const StringData* docComment,
                   Id id)
  : m_unit(unit)
  , m_namedEntity(NamedEntity::get(n))
  , m_line1(line1)
  , m_line2(line2)
  , m_id(id)
  , m_attrs(attrs)
  , m_name(n)
  , m_parent(parent)
  , m_docComment(docComment)
{
}

PreClass::~PreClass() {
  std::for_each(methods(), methods() + numMethods(), Func::destroy);
}

void PreClass::atomicRelease() {
  delete this;
}

const StringData* PreClass::manglePropName(const StringData* className,
                                           const StringData* propName,
                                           Attr attrs) {
  switch (attrs & VisibilityAttrs) {
    case AttrPublic: {
      return propName;
    }
    case AttrProtected: {
      std::string mangledName = "";
      mangledName.push_back('\0');
      mangledName.push_back('*');
      mangledName.push_back('\0');
      mangledName += propName->data();
      return makeStaticString(mangledName);
    }
    case AttrPrivate: {
      std::string mangledName = "";
      mangledName.push_back('\0');
      mangledName += className->data();
      mangledName.push_back('\0');
      mangledName += propName->data();
      return makeStaticString(mangledName);
    }
    default:
      //Failing here will cause the VM to crash before the Verifier runs, so we
      //defer the failure to runtime so the Verifier can report this problem to
      //the user.
      return staticEmptyString();
  }
}

void PreClass::prettyPrint(std::ostream &out) const {
  if (m_attrs & AttrSealed) { out << "<<__Sealed()>> "; }
  out << "Class ";
  if (m_attrs & AttrAbstract) { out << "abstract "; }
  if (m_attrs & AttrFinal) { out << "final "; }
  if (m_attrs & AttrInterface) { out << "interface "; }
  out << m_name->data();
  if (m_attrs & AttrNoOverride){ out << " (nooverride)"; }
  if (m_attrs & AttrUnique)     out << " (unique)";
  if (m_attrs & AttrPersistent) out << " (persistent)";
  if (m_attrs & AttrIsConst) {
    // AttrIsConst classes will always also have AttrForbidDynamicProps set,
    // so don't bother printing it
    out << " (const)";
  } else if (m_attrs & AttrForbidDynamicProps) {
    out << " (no-dynamic-props)";
  }
  if (m_attrs & AttrDynamicallyConstructible) out << " (dyn_constructible)";
  if (m_id != -1) {
    out << " (ID " << m_id << ")";
  }
  out << std::endl;

  for (Func* const* it = methods(); it != methods() + numMethods(); ++it) {
    out << " ";
    (*it)->prettyPrint(out);
  }
  for (const Prop* it = properties();
      it != properties() + numProperties();
      ++it) {
    out << " ";
    it->prettyPrint(out, this);
  }
  for (const Const* it = constants();
      it != constants() + numConstants();
      ++it) {
    out << " ";
    it->prettyPrint(out, this);
  }
}

const StaticString s___Sealed("__Sealed");
void PreClass::enforceInMaybeSealedParentWhitelist(
  const PreClass* parentPreClass) const {
  // if our parent isn't sealed, then we're fine. If we're a mock, YOLO
  if (!(parentPreClass->attrs() & AttrSealed) ||
      m_userAttributes.find(s___MockClass.get()) != m_userAttributes.end()) {
    return;
  }
  const UserAttributeMap& parent_attrs = parentPreClass->userAttributes();
  assertx(parent_attrs.find(s___Sealed.get()) != parent_attrs.end());
  const auto& parent_sealed_attr = parent_attrs.find(s___Sealed.get())->second;
  bool in_sealed_whitelist = false;
  IterateV(parent_sealed_attr.m_data.parr,
           [&in_sealed_whitelist, this](TypedValue v) -> bool {
             if (v.m_data.pstr->same(name())) {
               in_sealed_whitelist = true;
               return true;
             }
             return false;
           });
  if (!in_sealed_whitelist) {
    raise_error("Class %s may not inherit from sealed %s (%s) without "
                "being in the whitelist",
                name()->data(),
                parentPreClass->attrs() & AttrInterface ? "interface" : "class",
                parentPreClass->name()->data());
  }
}


///////////////////////////////////////////////////////////////////////////////
// PreClass::Prop.

PreClass::Prop::Prop(PreClass* preClass,
                     const StringData* name,
                     Attr attrs,
                     const StringData* userType,
                     const TypeConstraint& typeConstraint,
                     const CompactVector<TypeConstraint>& ubs,
                     const StringData* docComment,
                     const TypedValue& val,
                     RepoAuthType repoAuthType,
                     UserAttributeMap userAttributes)
  : m_name(name)
  , m_mangledName(manglePropName(preClass->name(), name, attrs))
  , m_attrs(attrs)
  , m_userType{userType}
  , m_docComment(docComment)
  , m_val(val)
  , m_repoAuthType{repoAuthType}
  , m_typeConstraint{typeConstraint}
  , m_userAttributes(userAttributes) {
  m_ubs.resize(ubs.size());
  std::copy(ubs.begin(), ubs.end(), m_ubs.begin());
}

void PreClass::Prop::prettyPrint(std::ostream& out,
                                 const PreClass* preClass) const {
  out << "Property ";
  if (m_attrs & AttrStatic) { out << "static "; }
  if (m_attrs & AttrPublic) { out << "public "; }
  if (m_attrs & AttrProtected) { out << "protected "; }
  if (m_attrs & AttrPrivate) { out << "private "; }
  if (m_attrs & AttrPersistent) { out << "(persistent) "; }
  if (m_attrs & AttrIsConst) { out << "(const) "; }
  if (m_attrs & AttrTrait) { out << "(trait) "; }
  if (m_attrs & AttrNoBadRedeclare) { out << "(no-bad-redeclare) "; }
  if (m_attrs & AttrNoOverride) { out << "(no-override) "; }
  if (m_attrs & AttrSystemInitialValue) { out << "(system-initial-val) "; }
  if (m_attrs & AttrNoImplicitNullable) { out << "(no-implicit-nullable) "; }
  if (m_attrs & AttrInitialSatisfiesTC) { out << "(initial-satisfies-tc) "; }
  if (m_attrs & AttrLSB) { out << "(lsb) "; }
  if (m_attrs & AttrLateInit) { out << "(late-init) "; }
  out << preClass->name()->data() << "::" << m_name->data() << " = ";
  if (m_val.m_type == KindOfUninit) {
    out << "<non-scalar>";
  } else {
    std::string ss;
    staticStreamer(&m_val, ss);
    out << ss;
  }
  out << " (RAT = " << show(m_repoAuthType) << ")";
  if (m_userType && !m_userType->empty()) {
    out << " (user-type = " << m_userType->data() << ")";
  }
  if (m_typeConstraint.hasConstraint()) {
    out << " (tc = " << m_typeConstraint.displayName(nullptr, true) << ")";
  }
  if (!m_ubs.empty()) {
    out << "(ubs = ";
    for (auto const& ub : m_ubs) {
      if (ub.hasConstraint()) {
        out << ub.displayName(nullptr, true);
        out << " ";
      }
    }
    out << ")";
  }
  out << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::Const.

PreClass::Const::Const(const StringData* name,
                       const TypedValueAux& val,
                       const StringData* phpCode,
                       const bool fromTrait)
  : m_name(name)
  , m_val(val)
  , m_phpCode(phpCode)
  , m_fromTrait(fromTrait)
{}

void PreClass::Const::prettyPrint(std::ostream& out,
                                  const PreClass* preClass) const {
  switch (kind()) {
    case ConstModifiers::Kind::Value:
      break;
    case ConstModifiers::Kind::Type:
      out << "Type ";
      break;
    case ConstModifiers::Kind::Context:
      out << "Context ";
      break;
  }
  if (isAbstractAndUninit()) {
    out << "Constant (abstract) "
        << preClass->name()->data() << "::" << m_name->data()
        << std::endl;
    return;
  }
  if (kind() == ConstModifiers::Kind::Context) {
    out << "Constant "
        << preClass->name()->data() << "::" << m_name->data()
        << " " << coeffects().toString()
        << std::endl;
    return;
  }
  out << "Constant " << preClass->name()->data() << "::" << m_name->data();
  if (m_val.m_type == KindOfUninit) {
    out << " = " << "<non-scalar>";
  } else {
    std::string ss;
    staticStreamer(&m_val, ss);
    out << " = " << ss;
  }
  out << std::endl;
}

StaticCoeffects PreClass::Const::coeffects() const {
  assertx(kind() == ConstModifiers::Kind::Context);
  return m_val.constModifiers().getCoeffects();
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::TraitAliasRule.

PreClass::TraitAliasRule::NamePair
PreClass::TraitAliasRule::asNamePair() const {
  auto const tmp = folly::sformat(
    "{}::{}",
    traitName()->empty() ? "(null)" : traitName()->data(),
    origMethodName());

  auto origName = makeStaticString(tmp);
  return std::make_pair(newMethodName(), origName);
}

///////////////////////////////////////////////////////////////////////////////
}
