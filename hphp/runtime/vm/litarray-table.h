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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/vm/containers.h"

#include <tbb/concurrent_hash_map.h>

namespace HPHP {

using LitarrayTableData = VMCompactVector<UnsafeLockFreePtrWrapper<const ArrayData*>>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Singleton global table of literal arrays
 *
 * This can only be safely used when the repo is built in WholeProgram mode and
 * run in RepoAuthoritative mode.
 */
struct LitarrayTable {

  /////////////////////////////////////////////////////////////////////////////
  // Singleton init and get.                                           [static]

  /*
   * Create the singleton LitarrayTable.
   *
   * Must not be called in concurrent contexts---the table pointer is not
   * atomic, and init() does not check if a table already exists.
   */
  static void init();

  /*
   * Destroy the singleton LitarrayTable.
   *
   * Must not be called in concurrent contexts---the table pointer is not
   * atomic.
   */
  static void fini();

  /*
   * Get the singleton LitarrayTable.
   */
  static LitarrayTable& get();


  /////////////////////////////////////////////////////////////////////////////
  // Main API.

  /*
   * Size of the table.
   */
  size_t numLitarrays() const;

  bool contains(Id id) const;
  ArrayData* lookupLitarrayId(Id id) const;

  static bool canRead() {
    return !s_litarrayTable || s_litarrayTable->m_safeToRead;
  }

  /*
   * Set up the table.  Not thread-safe.
   */
  void setTable(LitarrayTableData&& arrays);

  /*
   * Set an entry, used for lazy loading.
   */
  void setLitarray(Id id, const ArrayData* str);

  /*
   * Add an entry for `litarray' to the table.
   *
   * The "merge" terminology is inherited from Unit.
   */
  Id mergeLitarray(const ArrayData* litarray);

  /*
   * Call onItem() for each item in the table.
   */
  void forEachLitarray(
    std::function<void (int i, const ArrayData* ad)> onItem);


  /////////////////////////////////////////////////////////////////////////////
  // Concurrency control.

  /*
   * LitarrayTable reader/writer state.
   *
   * Setting the reader state will update m_namedInfo from m_litarray2id.
   */
  void setReading();
  void setWriting();


  /////////////////////////////////////////////////////////////////////////////
  // Private constructor.

private:
  LitarrayTable() {}


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  static LitarrayTable* s_litarrayTable;

  using LitarrayMap = tbb::concurrent_hash_map<
    const ArrayData*,
    Id,
    pointer_hash<ArrayData>
  >;

  LitarrayMap m_litarray2id;
  LitarrayTableData m_arrays;

  std::atomic<Id> m_nextId{0};
  std::atomic<bool> m_safeToRead{true};
};

/*
 * Lazy load helper that is safe to call concurrently.
 */
ArrayData* loadLitarrayById(Id id);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_LITARRAY_TABLE_INL_H_
#include "hphp/runtime/vm/litarray-table-inl.h"
#undef incl_HPHP_LITARRAY_TABLE_INL_H_

