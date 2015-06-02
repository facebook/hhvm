/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

PreClass::PreClass(Unit* unit, int line1, int line2, Offset o,
                   const StringData* n, Attr attrs, const StringData* parent,
                   const StringData* docComment, Id id, Hoistable hoistable)
  : m_unit(unit)
  , m_namedEntity(NamedEntity::get(n))
  , m_line1(line1)
  , m_line2(line2)
  , m_offset(o)
  , m_id(id)
  , m_attrs(attrs)
  , m_hoistable(hoistable)
  , m_name(n)
  , m_parent(parent)
  , m_docComment(docComment)
{}

PreClass::~PreClass() {
  std::for_each(methods(), methods() + numMethods(), Func::destroy);
}

void PreClass::atomicRelease() {
  delete this;
}

const StringData* PreClass::manglePropName(const StringData* className,
                                           const StringData* propName,
                                           Attr attrs) {
  switch (attrs & (AttrPublic|AttrProtected|AttrPrivate)) {
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
    default: not_reached();
  }
}

void PreClass::prettyPrint(std::ostream &out) const {
  out << "Class ";
  if (m_attrs & AttrAbstract) { out << "abstract "; }
  if (m_attrs & AttrFinal) { out << "final "; }
  if (m_attrs & AttrInterface) { out << "interface "; }
  out << m_name->data() << " at " << m_offset;
  if (m_hoistable == MaybeHoistable) {
    out << " (maybe-hoistable)";
  } else if (m_hoistable == AlwaysHoistable) {
    out << " (always-hoistable)";
  }
  if (m_attrs & AttrNoOverride){ out << " (nooverride)"; }
  if (m_attrs & AttrUnique)     out << " (unique)";
  if (m_attrs & AttrPersistent) out << " (persistent)";
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

///////////////////////////////////////////////////////////////////////////////
// PreClass::Prop.

PreClass::Prop::Prop(PreClass* preClass,
                     const StringData* name,
                     Attr attrs,
                     const StringData* typeConstraint,
                     const StringData* docComment,
                     const TypedValue& val,
                     RepoAuthType repoAuthType)
  : m_name(name)
  , m_mangledName(manglePropName(preClass->name(), name, attrs))
  , m_attrs(attrs)
  , m_typeConstraint(typeConstraint)
  , m_docComment(docComment)
  , m_val(val)
  , m_repoAuthType{repoAuthType}
{}

void PreClass::Prop::prettyPrint(std::ostream& out,
                                 const PreClass* preClass) const {
  out << "Property ";
  if (m_attrs & AttrStatic) { out << "static "; }
  if (m_attrs & AttrPublic) { out << "public "; }
  if (m_attrs & AttrProtected) { out << "protected "; }
  if (m_attrs & AttrPrivate) { out << "private "; }
  if (m_attrs & AttrPersistent) { out << "(persistent) "; }
  out << preClass->name()->data() << "::" << m_name->data() << " = ";
  if (m_val.m_type == KindOfUninit) {
    out << "<non-scalar>";
  } else {
    std::stringstream ss;
    staticStreamer(&m_val, ss);
    out << ss.str();
  }
  out << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::Const.

PreClass::Const::Const(const StringData* name,
                       const TypedValueAux& val,
                       const StringData* phpCode)
  : m_name(name)
  , m_val(val)
  , m_phpCode(phpCode)
{}

void PreClass::Const::prettyPrint(std::ostream& out,
                                  const PreClass* preClass) const {
  if (isType()) {
    out << "Type ";
  }
  if (isAbstract()) {
    out << "Constant (abstract) "
        << preClass->name()->data() << "::" << m_name->data()
        << std::endl;
    return;
  }
  out << "Constant " << preClass->name()->data() << "::" << m_name->data();
  if (m_val.m_type == KindOfUninit) {
    out << " = " << "<non-scalar>";
  } else {
    std::stringstream ss;
    staticStreamer(&m_val, ss);
    out << " = " << ss.str();
  }
  out << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// PreClass::TraitAliasRule.

PreClass::TraitAliasRule::NamePair
PreClass::TraitAliasRule::asNamePair() const {
  char buf[traitName()->size() + origMethodName()->size() + 9];
  sprintf(buf, "%s::%s",
          traitName()->empty() ? "(null)" : traitName()->data(),
          origMethodName()->data());

  auto origName = makeStaticString(buf);
  return std::make_pair(newMethodName(), origName);
}

///////////////////////////////////////////////////////////////////////////////
}
