/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

inline bool NamedEntityPairTable::contains(Id id) const {
  return id >= 0 && id < Id(size());
}

inline StringData* NamedEntityPairTable::lookupLitstr(Id id) const {
  assert(contains(id));
  return const_cast<StringData*>((*this)[id].first.get());
}

inline const NamedEntity*
NamedEntityPairTable::lookupNamedEntity(Id id) const {
  return lookupNamedEntityPair(id).second;
}

inline const NamedEntityPair&
NamedEntityPairTable::lookupNamedEntityPair(Id id) const {
  assert(contains(id));
  auto const& nep = (*this)[id];

  // Check that the name exists and is normalized.
  assert(nep.first);
  assert(nep.first->data()[nep.first->size()] == 0);
  assert(nep.first->data()[0] != '\\');

  // Create the NamedEntity if necessary.
  if (UNLIKELY(!nep.second)) {
    const_cast<LowPtr<const NamedEntity>&>(nep.second) =
      NamedEntity::get(nep.first);
  }
  return nep;
}

///////////////////////////////////////////////////////////////////////////////
}
