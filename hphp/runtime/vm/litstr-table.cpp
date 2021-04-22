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

#include "hphp/runtime/vm/litstr-table.h"

#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

LitstrTable* LitstrTable::s_litstrTable = nullptr;

///////////////////////////////////////////////////////////////////////////////

Id LitstrTable::mergeLitstr(const StringData* litstr) {
  if (!litstr) {
    return 0;
  }

  {
    LitstrMap::const_accessor acc;
    if (m_litstr2id.find(acc, litstr)) {
      return acc->second;
    }
  }

  auto const sd = makeStaticString(litstr);
  LitstrMap::accessor acc;
  if (m_litstr2id.insert(acc, sd)) {
    acc->second = m_nextId.fetch_add(1, std::memory_order_relaxed);
  }

  return acc->second;
}

void LitstrTable::setReading() {
  always_assert(!m_safeToRead);
  always_assert(!m_namedInfo.size());

  m_namedInfo.resize(m_litstr2id.size() + 1);
  m_namedInfo.shrink_to_fit();
  for (auto const& strId : m_litstr2id) {
    m_namedInfo[strId.second] = LowStringPtr{strId.first};
  }

  m_safeToRead = true;
}

void LitstrTable::forEachLitstr(
  std::function<void (int, const StringData*)> onItem) {
  assertx(m_safeToRead);
  auto i = 0;
  for (auto& s : m_namedInfo) {
    if (i != 0) {
      onItem(i, s.get());
    }
    i++;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Lazy loading.

StringData* loadLitstrById(Id id) {
  if (RuntimeOption::RepoAuthoritative) return RepoFile::loadLitstr(id);
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
