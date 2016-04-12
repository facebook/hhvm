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

#ifndef incl_HPHP_NAMED_ENTITY_PAIR_TABLE_H_
#define incl_HPHP_NAMED_ENTITY_PAIR_TABLE_H_

#include "hphp/runtime/vm/named-entity.h"

#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct StringData;

///////////////////////////////////////////////////////////////////////////////

/*
 * Vector of NamedEntityPairs, used as a map from Id to NEP.
 */
struct NamedEntityPairTable : std::vector<NamedEntityPair> {
  /*
   * Is `id' valid in this table?
   */
  bool contains(Id id) const;

  /*
   * Look up the litstr given by `id'.
   *
   * @requires: contains(id)
   */
  StringData* lookupLitstr(Id id) const;

  /*
   * Look up the NamedEntity corresponding to the litstr given by `id'.
   *
   * @returns: lookupNamedEntityPair(id).second
   */
  const NamedEntity* lookupNamedEntity(Id id) const;

  /*
   * Look up the NamedEntityPair corresponding to the litstr given by `id'.
   *
   * This will lazily populate the NamedEntity table, creating the NamedEntity
   * for the litstr if it does not yet exist.
   *
   * @requires: contains(id)
   */
  const NamedEntityPair& lookupNamedEntityPair(Id id) const;
};

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_NAMED_ENTITY_PAIR_TABLE_INL_H_
#include "hphp/runtime/vm/named-entity-pair-table-inl.h"
#undef incl_HPHP_NAMED_ENTITY_PAIR_TABLE_INL_H_

#endif // incl_HPHP_NAMED_ENTITY_PAIR_TABLE_H_
