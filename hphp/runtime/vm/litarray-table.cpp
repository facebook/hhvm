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

#include "hphp/runtime/vm/litarray-table.h"

#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

LitarrayTable* LitarrayTable::s_litarrayTable = nullptr;

///////////////////////////////////////////////////////////////////////////////

Id LitarrayTable::mergeLitarray(const ArrayData* arr) {
  assertx(!m_safeToRead);
  assertx(arr);
  {
    LitarrayMap::const_accessor acc;
    if (m_litarray2id.find(acc, arr)) {
      return acc->second;
    }
  }

  auto mut_arr = const_cast<ArrayData*>(arr);
  ArrayData::GetScalarArray(&mut_arr);
  LitarrayMap::accessor acc;
  if (m_litarray2id.insert(acc, mut_arr)) {
    acc->second = m_nextId.fetch_add(1, std::memory_order_relaxed);
  }

  return acc->second;
}

void LitarrayTable::setReading() {
  always_assert(!m_safeToRead);
  always_assert(!m_arrays.size());

  m_arrays.resize(m_litarray2id.size());
  m_arrays.shrink_to_fit();
  for (auto const& arrId : m_litarray2id) {
    m_arrays[arrId.second] = arrId.first;
  }

  m_safeToRead = true;
}

void LitarrayTable::forEachLitarray(
  std::function<void (int, const ArrayData*)> onItem) {
  assertx(m_safeToRead);
  auto i = 0;
  for (auto& s : m_arrays) {
    onItem(i, s.get());
    i++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Lazy loading.

ArrayData* loadLitarrayById(Id id) {
  assertx(RuntimeOption::RepoAuthoritative);
  return RepoFile::loadLitarray(id);
}

///////////////////////////////////////////////////////////////////////////////
}
