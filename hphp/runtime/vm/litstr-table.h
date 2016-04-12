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

#ifndef incl_HPHP_LITSTR_TABLE_H_
#define incl_HPHP_LITSTR_TABLE_H_

#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/named-entity-pair-table.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/mutex.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct RepoTxn;
struct StringData;

///////////////////////////////////////////////////////////////////////////////

/*
 * Global litstr Id's are all above this mark.
 */
constexpr int kGlobalLitstrOffset = 0x40000000;

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
  const NamedEntityPair& lookupNamedEntityPairId(Id id) const;

  /*
   * Set up the named info table.  Not thread-safe.
   */
  void setNamedEntityPairTable(NamedEntityPairTable&& namedInfo);

  /*
   * Add an entry for `litstr' to the table.
   *
   * The "merge" terminology is inherited from Unit.
   */
  Id mergeLitstr(const StringData* litstr);

  /*
   * Insert the table into the repo.
   */
  void insert(RepoTxn& txn);


  /////////////////////////////////////////////////////////////////////////////
  // Concurrency control.

  /*
   * Write lock.
   *
   * @requires: !m_safeToRead.
   */
  Mutex& mutex();

  /*
   * LitstrTable reader/writer state.
   *
   * Used for debugging asserts only, but the flags are always set since these
   * are called rarely.
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

  using LitstrMap = hphp_hash_map<
    const StringData*,
    Id,
    string_data_hash,
    string_data_same
  >;

  NamedEntityPairTable m_namedInfo;
  LitstrMap m_litstr2id;

  Mutex m_mutex;
  std::atomic<bool> m_safeToRead;
};

///////////////////////////////////////////////////////////////////////////////
// ID helpers.

/*
 * Functions for differentiating global litstrId's from unit-local Id's.
 */
bool isGlobalLitstrId(Id id);
Id encodeGlobalLitstrId(Id id);
Id decodeGlobalLitstrId(Id id);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_LITSTR_TABLE_INL_H_
#include "hphp/runtime/vm/litstr-table-inl.h"
#undef incl_HPHP_LITSTR_TABLE_INL_H_

#endif // incl_HPHP_LITSTR_TABLE_H_
