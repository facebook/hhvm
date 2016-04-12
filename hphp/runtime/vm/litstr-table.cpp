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

#include "hphp/runtime/vm/litstr-table.h"

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-helpers.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

LitstrTable* LitstrTable::s_litstrTable = nullptr;

///////////////////////////////////////////////////////////////////////////////

Id LitstrTable::mergeLitstr(const StringData* litstr) {
  std::lock_guard<Mutex> g(mutex());
  assert(!m_safeToRead);
  auto it = m_litstr2id.find(litstr);

  if (it == m_litstr2id.end()) {
    const StringData* sd = makeStaticString(litstr);
    Id id = numLitstrs();

    m_litstr2id[sd] = id;
    m_namedInfo.emplace_back(sd, nullptr);

    return id;
  } else {
    return it->second;
  }
}

void LitstrTable::insert(RepoTxn& txn) {
  Repo& repo = Repo::get();
  LitstrRepoProxy& lsrp = repo.lsrp();
  int repoId = Repo::get().repoIdForNewUnit(UnitOrigin::File);
  for (int i = 0; i < m_namedInfo.size(); ++i) {
    lsrp.insertLitstr(repoId).insert(txn, i, m_namedInfo[i].first);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
