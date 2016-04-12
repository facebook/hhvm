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

#ifndef incl_HPHP_LITSTR_TABLE_INL_H_
#error "litstr-table-inl.h should only be included by litstr-table.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void LitstrTable::init() {
  LitstrTable::s_litstrTable = new LitstrTable();
}

inline LitstrTable& LitstrTable::get() {
  return *LitstrTable::s_litstrTable;
}

///////////////////////////////////////////////////////////////////////////////
// Main API.

inline size_t LitstrTable::numLitstrs() const {
  return m_namedInfo.size();
}

inline bool LitstrTable::contains(Id id) const {
  return m_namedInfo.contains(id);
}

inline StringData* LitstrTable::lookupLitstrId(Id id) const {
  assert(m_safeToRead);
  return m_namedInfo.lookupLitstr(id);
}

inline const NamedEntity* LitstrTable::lookupNamedEntityId(Id id) const {
  assert(m_safeToRead);
  return m_namedInfo.lookupNamedEntity(id);
}

inline
const NamedEntityPair& LitstrTable::lookupNamedEntityPairId(Id id) const {
  assert(m_safeToRead);
  return m_namedInfo.lookupNamedEntityPair(id);
}

inline
void LitstrTable::setNamedEntityPairTable(NamedEntityPairTable&& namedInfo) {
  m_namedInfo = std::move(namedInfo);
}

///////////////////////////////////////////////////////////////////////////////
// Concurrency control.

inline Mutex& LitstrTable::mutex() {
  return m_mutex;
}

inline void LitstrTable::setReading() {
  m_safeToRead = true;
}

inline void LitstrTable::setWriting() {
  m_safeToRead = false;
}

///////////////////////////////////////////////////////////////////////////////
// ID helpers.

inline bool isGlobalLitstrId(Id id) {
  return id >= kGlobalLitstrOffset;
}

inline Id encodeGlobalLitstrId(Id id) {
  return id + kGlobalLitstrOffset;
}

inline Id decodeGlobalLitstrId(Id id) {
  return id - kGlobalLitstrOffset;
}

///////////////////////////////////////////////////////////////////////////////
}
