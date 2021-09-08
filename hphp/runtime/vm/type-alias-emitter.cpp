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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TypeAliasEmitter::TypeAliasEmitter(UnitEmitter& ue,
                                   Id id,
                                   const std::string& name)
  : m_ue(ue)
  , m_name(makeStaticString(name))
  , m_id(id) {}

void TypeAliasEmitter::init(int line0, int line1, Attr attrs,
                            const StringData* value,
                            AnnotType type,
                            bool nullable,
                            Array typeStructure,
                            Array resolvedTypeStructure) {
  assertx(typeStructure.isDict() &&
          !typeStructure.empty() &&
          typeStructure->isStatic());
  assertx(resolvedTypeStructure.isNull() ||
          (resolvedTypeStructure.isDict() &&
           !resolvedTypeStructure.empty() &&
           resolvedTypeStructure->isStatic()));
  m_line0 = line0;
  m_line1 = line1;
  m_attrs = attrs;
  m_value = value;
  m_type = type;
  m_nullable = nullable;
  m_typeStructure = std::move(typeStructure);
  m_resolvedTypeStructure = std::move(resolvedTypeStructure);
}

PreTypeAlias TypeAliasEmitter::create(Unit& unit) const {
  return PreTypeAlias {
    &unit, m_name, m_value, m_attrs, m_type, m_line0, m_line1,
    m_nullable, m_userAttributes, m_typeStructure, m_resolvedTypeStructure
  };
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
    (m_typeStructure)
    (m_resolvedTypeStructure)
    ;

  assertx(m_typeStructure.isDict());
  assertx(!m_typeStructure.empty());
  assertx(IMPLIES(!m_resolvedTypeStructure.isNull(),
                  m_resolvedTypeStructure.isDict() &&
                  !m_resolvedTypeStructure.empty() &&
                  m_resolvedTypeStructure->isStatic()));
}

template void TypeAliasEmitter::serdeMetaData<>(BlobDecoder&);
template void TypeAliasEmitter::serdeMetaData<>(BlobEncoder&);

///////////////////////////////////////////////////////////////////////////////
}
