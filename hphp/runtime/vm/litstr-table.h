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

#pragma once

#include "hphp/runtime/base/string-functors.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-pair-table.h"
#include "hphp/util/alloc.h"

#include <tbb/concurrent_hash_map.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Singleton global table of literal strings and NamedEntitys.
 *
 * This can only be safely used when the repo is built in WholeProgram mode and
 * run in RepoAuthoritative mode.
 */
struct LitstrTable {

  /////////////////////////////////////////////////////////////////////////////
  // Singleton init and get.                                           [static]

  /*
   * Create the singleton LitstrTable.
   *
   * Must not be called in concurrent contexts---the table pointer is not
   * atomic, and init() does not check if a table already exists.
   */
  static void init();

  /*
   * Destroy the singleton LitstrTable.
   *
   * Must not be called in concurrent contexts---the table pointer is not
   * atomic.
   */
  static void fini();

  /*
   * Get the singleton LitstrTable.
   */
  static LitstrTable& get();


  /////////////////////////////////////////////////////////////////////////////
  // Main API.

  /*
   * Size of the table.
   */
  size_t numLitstrs() const;

  /*
   * Dispatch to corresponding NamedEntityPairTable methods, sans `Id' suffix.
   */
  bool contains(Id id) const;
  StringData* lookupLitstrId(Id id) const;
  const NamedEntity* lookupNamedEntityId(Id id) const;
  NamedEntityPair lookupNamedEntityPairId(Id id) const;

  static bool canRead() {
    return !s_litstrTable || s_litstrTable->m_safeToRead;
  }

  /*
   * Set up the named info table.  Not thread-safe.
   */
  void setNamedEntityPairTable(NamedEntityPairTable&& namedInfo);

  /*
   * Set an entry, used for lazy loading.
   */
  void setLitstr(Id id, const StringData* str);

  /*
   * Add an entry for `litstr' to the table.
   *
   * The "merge" terminology is inherited from Unit.
   */
  Id mergeLitstr(const StringData* litstr);

  /*
   * Call onItem() for each item in the table.
   */
  void forEachLitstr(
    std::function<void (int i, const StringData* name)> onItem);


  /////////////////////////////////////////////////////////////////////////////
  // Concurrency control.

  /*
   * LitstrTable reader/writer state.
   *
   * Setting the reader state will update m_namedInfo from m_litstr2id.
   */
  void setReading();
  void setWriting();


  /////////////////////////////////////////////////////////////////////////////
  // Private constructor.

private:
  LitstrTable() {}


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  static LitstrTable* s_litstrTable;

  using LitstrMap = tbb::concurrent_hash_map<
    const StringData*,
    Id,
    StringDataHashCompare,
    VMAllocator<char>
  >;

  NamedEntityPairTable m_namedInfo;
  LitstrMap m_litstr2id;

  std::atomic<Id> m_nextId{1};
  std::atomic<bool> m_safeToRead{true};
};

/*
 * Lazy load helper that is safe to call concurrently.
 */
StringData* loadLitstrById(Id id);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_LITSTR_TABLE_INL_H_
#include "hphp/runtime/vm/litstr-table-inl.h"
#undef incl_HPHP_LITSTR_TABLE_INL_H_

