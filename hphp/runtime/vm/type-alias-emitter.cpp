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
#include "hphp/runtime/vm/type-alias-emitter.h"

#include <limits>
#include <sstream>


#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/repo.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TypeAliasEmitter::TypeAliasEmitter(UnitEmitter& ue, Id id, const std::string& name)
  : m_ue(ue)
  , m_name(makeStaticString(name))
  , m_id(id) {}

void TypeAliasEmitter::init(int line0, int line1, Attr attrs,
                            const StringData* value, AnnotType type, bool nullable) {
  m_line0 = line0;
  m_line1 = line1;
  m_attrs = attrs;
  m_value = value;
  m_type = type;
  m_nullable = nullable;
}

PreTypeAlias TypeAliasEmitter::create(Unit& unit) const {
  return PreTypeAlias {
      &unit, m_name, m_value, m_attrs, m_type, m_line0, m_line1, m_nullable, m_userAttributes, m_typeStructure };
}

template<class SerDe> void TypeAliasEmitter::serdeMetaData(SerDe& sd) {
  // NOTE: name is serialized outside of this.
  sd(m_line0)
    (m_line1)
    (m_attrs)
    (m_value)
    (m_type)
    (m_nullable)
    (m_userAttributes)
    ;

  if constexpr (SerDe::deserializing) {
    TypedValue tv;
    sd(tv);
    assertx(tvIsDict(tv));
    assertx(tvIsPlausible(tv));
    m_typeStructure = tv.m_data.parr;
  } else {
    auto tv = make_array_like_tv(m_typeStructure.get());
    sd(tv);
  }
}

template void TypeAliasEmitter::serdeMetaData<>(BlobDecoder&);
template void TypeAliasEmitter::serdeMetaData<>(BlobEncoder&);

///////////////////////////////////////////////////////////////////////////////
}
