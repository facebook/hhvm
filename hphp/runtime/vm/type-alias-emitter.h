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

#pragma once

#include "hphp/runtime/base/annot-type.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-array-like.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/user-attributes.h"

#include "hphp/runtime/vm/type-alias.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Unit;
struct UnitEmitter;

///////////////////////////////////////////////////////////////////////////////

struct TypeAliasEmitter {

  TypeAliasEmitter(UnitEmitter& ue, Id id, const std::string& name);

  void init(int line0, int line1, Attr attrs,
            TypeConstraint value,
            AliasKind kind, Array typeStructure,
            Array resolvedTypeStructure);

  UnitEmitter& ue() const { return m_ue; }
  const StringData* name() const { return m_name; }
  const TypeConstraint& value() const { return m_value; }
  Attr attrs() const { return m_attrs; }
  void setAttrs(Attr attrs) { m_attrs = attrs; }
  AliasKind kind() const { return m_kind; }
  UserAttributeMap userAttributes() const { return m_userAttributes; }
  void setUserAttributes(UserAttributeMap map) {
    m_userAttributes = std::move(map);
  }
  const Array& typeStructure() const { return m_typeStructure; }
  const Array& resolvedTypeStructure() const { return m_resolvedTypeStructure; }

  Id id() const { return m_id; }

  PreTypeAlias create(Unit& unit) const;

  template<class SerDe> void serdeMetaData(SerDe&);

  std::pair<int,int> getLocation() const {
    return std::make_pair(m_line0, m_line1);
  }

private:
  UnitEmitter& m_ue;
  LowStringPtr m_name;
  Attr m_attrs;
  TypeConstraint m_value;
  int m_line0;
  int m_line1;
  AliasKind m_kind;
  UserAttributeMap m_userAttributes;
  Array m_typeStructure{ArrayData::CreateDict()};
  // If !isNull(), contains m_typeStructure in post-resolved form from
  // HHBBC.
  Array m_resolvedTypeStructure;
  Id m_id;
};

///////////////////////////////////////////////////////////////////////////////
}
