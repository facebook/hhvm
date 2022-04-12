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

#ifndef incl_HPHP_LITSTR_TABLE_INL_H_
#error "litstr-table-inl.h should only be included by litstr-table.h"
#endif

#include "hphp/util/alloc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void LitstrTable::init() {
  assertx(!LitstrTable::s_litstrTable);
  LitstrTable::s_litstrTable =
    new (vm_malloc(sizeof(LitstrTable))) LitstrTable();
}

inline void LitstrTable::fini() {
  assertx(LitstrTable::s_litstrTable);
  vm_free(LitstrTable::s_litstrTable);
  LitstrTable::s_litstrTable = nullptr;
}

inline LitstrTable& LitstrTable::get() {
  return *LitstrTable::s_litstrTable;
}

///////////////////////////////////////////////////////////////////////////////
// Main API.

inline size_t LitstrTable::numLitstrs() const {
  assertx(m_safeToRead);
  return m_namedInfo.size();
}

inline bool LitstrTable::contains(Id id) const {
  return m_safeToRead
    ? m_namedInfo.contains(id)
    : 0 < id && id < m_nextId.load(std::memory_order_relaxed);
}

inline StringData* LitstrTable::lookupLitstrId(Id id) const {
  assertx(m_safeToRead);
  if (auto ret = m_namedInfo.lookupLitstr(id)) {
    return ret;
  }
  return loadLitstrById(id);
}

inline const NamedEntity* LitstrTable::lookupNamedEntityId(Id id) const {
  assertx(m_safeToRead);
  return m_namedInfo.lookupNamedEntity(id);
}

inline NamedEntityPair LitstrTable::lookupNamedEntityPairId(Id id) const {
  assertx(m_safeToRead);
  return m_namedInfo.lookupNamedEntityPair(id);
}

inline
void LitstrTable::setNamedEntityPairTable(NamedEntityPairTable&& namedInfo) {
  assertx(m_namedInfo.empty());
  m_namedInfo = std::move(namedInfo);
}

inline
void LitstrTable::setLitstr(Id id, const StringData* str) {
  assertx(contains(id));
  auto& elem = m_namedInfo[id];
  elem.lock_for_update();
  if (DEBUG_ONLY auto const curr = elem.get()) {
    assertx(str == curr);
    elem.unlock();
  } else {
    elem.update_and_unlock(LowStringPtr{str});
  }
}

///////////////////////////////////////////////////////////////////////////////
// Concurrency control.

inline void LitstrTable::setWriting() {
  always_assert(!m_litstr2id.size());
  m_safeToRead = false;
}

///////////////////////////////////////////////////////////////////////////////
}
