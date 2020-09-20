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

#ifndef incl_HPHP_NAMED_ENTITY_PAIR_TABLE_INL_H_
#error "named-entity-pair-table-inl.h should only be included by " \
       "named-entity-pair-table.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern StringData* loadLitstrById(Id id);

///////////////////////////////////////////////////////////////////////////////

inline bool NamedEntityPairTable::contains(Id id) const {
  return id >= 0 && id < Id(size());
}

inline StringData* NamedEntityPairTable::lookupLitstr(Id id) const {
  assertx(contains(id));
  if (auto const ret = (*this)[id].get()) {
    return const_cast<StringData*>(ret);
  }
  return loadLitstrById(id);
}

inline const NamedEntity*
NamedEntityPairTable::lookupNamedEntity(Id id) const {
  return lookupNamedEntityPair(id).second;
}

inline NamedEntityPair
NamedEntityPairTable::lookupNamedEntityPair(Id id) const {
  assertx(contains(id));
  auto const name = lookupLitstr(id);

  assertx(name);
  assertx(name->data()[0] != '\\');

  return { name, NamedEntity::get(name) };
}

///////////////////////////////////////////////////////////////////////////////
}
