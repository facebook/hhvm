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

#ifndef incl_HPHP_LITARRAY_TABLE_INL_H_
#error "litarray-table-inl.h should only be included by litarray-table.h"
#endif

#include "hphp/util/alloc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void LitarrayTable::init() {
  assertx(!LitarrayTable::s_litarrayTable);
  LitarrayTable::s_litarrayTable =
    new (vm_malloc(sizeof(LitarrayTable))) LitarrayTable();
}

inline void LitarrayTable::fini() {
  assertx(LitarrayTable::s_litarrayTable);
  vm_free(LitarrayTable::s_litarrayTable);
  LitarrayTable::s_litarrayTable = nullptr;
}

inline LitarrayTable& LitarrayTable::get() {
  assertx(LitarrayTable::s_litarrayTable);
  return *LitarrayTable::s_litarrayTable;
}

///////////////////////////////////////////////////////////////////////////////
// Main API.

inline size_t LitarrayTable::numLitarrays() const {
  assertx(m_safeToRead);
  return m_arrays.size();
}

inline bool LitarrayTable::contains(Id id) const {
  return m_safeToRead
    ? 0 <= id && id < m_arrays.size()
    : 0 <= id && id < m_nextId.load(std::memory_order_relaxed);
}

inline ArrayData* LitarrayTable::lookupLitarrayId(Id id) const {
  assertx(m_safeToRead);
  if (auto const ret = m_arrays[id].get()) {
    return const_cast<ArrayData*>(ret);
  }
  return loadLitarrayById(id);
}

inline
void LitarrayTable::setTable(LitarrayTableData&& arrays) {
  assertx(m_arrays.empty());
  m_arrays = std::move(arrays);
}

inline
void LitarrayTable::setLitarray(Id id, const ArrayData* arr) {
  assertx(contains(id));
  auto& elem = m_arrays[id];
  elem.lock_for_update();
  if (DEBUG_ONLY auto const curr = elem.get()) {
    assertx(arr == curr);
    elem.unlock();
  } else {
    elem.update_and_unlock(std::move(arr));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Concurrency control.

inline void LitarrayTable::setWriting() {
  always_assert(!m_litarray2id.size());
  m_safeToRead = false;
}

///////////////////////////////////////////////////////////////////////////////
}
