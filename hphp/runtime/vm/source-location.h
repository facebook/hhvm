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

#include "hphp/parser/location.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Unit;

///////////////////////////////////////////////////////////////////////////////
// Location tables.

/*
 * Delimiter pairs for a location in the source code.
 */
struct SourceLoc {
  /*
   * Constructors.
   */
  SourceLoc() {}
  explicit SourceLoc(const Location::Range& l);

  /*
   * Reset to, or check for, the invalid state.
   */
  void reset();
  bool valid() const;

  /*
   * Set to a parser Location.
   */
  void setLoc(const Location::Range* l);

  /*
   * Equality.
   */
  bool same(const SourceLoc* l) const;
  bool operator==(const SourceLoc& l) const;

  /*
   * Start and end lines and characters.
   *
   * The default {1, 1, 1, 1} is an invalid sentinel value.
   */
  int line0{1};
  int char0{1};
  int line1{1};
  int char1{1};

  template <typename SerDes> void serde(SerDes& sd) {
    sd(line0);
    sd(char0);
    sd(line1);
    sd(char1);
  }
};

/*
 * Pair of (base, past) offsets.
 */
struct OffsetRange {
  OffsetRange() {}

  OffsetRange(Offset base, Offset past)
    : base(base)
    , past(past)
  {}

  Offset base{0};
  Offset past{0};
};

using OffsetRangeVec = std::vector<OffsetRange>;
using LineToOffsetRangeVecMap = std::map<int, OffsetRangeVec>;

/*
 * Generic entry for representing many-to-one mappings of Offset -> T.
 *
 * Each entry's `pastOffset' is expected to be the offset just past the range
 * of offsets which logically map to its `val'.  In this way, by maintaining a
 * relatively sparse set of entries in a vector, we can use least upper bound
 * searches on an offset key to find its corresponding T.
 *
 * The values of `pastOffset' in such a table are expected to be sorted and
 * unique, but the values of `val' need not be.
 */
template<typename T>
struct TableEntry {
  /*
   * Constructors.
   */
  TableEntry()
    : m_pastOffset()
    , m_val()
  {}

  TableEntry(Offset pastOffset, T val)
    : m_pastOffset(pastOffset)
    , m_val(val)
  {}

  /*
   * Accessors.
   */
  Offset pastOffset() const;
  T val() const;

  /*
   * Comparison.
   */
  bool operator<(const TableEntry& other) const;

  template<class SerDe> void serde(SerDe& sd);

private:
  Offset m_pastOffset{0};
  T m_val;
};

/*
 * Table specializations.
 */
using LineEntry      = TableEntry<int>;
using SourceLocEntry = TableEntry<SourceLoc>;
using LineInfo       = std::pair<OffsetRange, int>;

using LineTable      = std::vector<LineEntry>;
using SourceLocTable = std::vector<SourceLocEntry>;

///////////////////////////////////////////////////////////////////////////////

namespace SourceLocation {

/*
 * Get the line number or SourceLoc for Offset `pc' in `table'.
 */
int getLineNumber(const LineTable& table, Offset pc);
bool getLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc);
void stashLineTable(const Unit* unit, LineTable table);
void stashExtendedLineTable(const Unit* unit, SourceLocTable table);

const SourceLocTable& getLocTable(const Unit*);

void removeUnit(const Unit* unit);
const LineTable& loadLineTable(const Unit* unit);
const LineTable* getLineTable(const Unit* unit);
LineInfo getLineInfo(const LineTable& table, Offset pc);

LineToOffsetRangeVecMap getLineToOffsetRangeVecMap(const Unit* unit);

}

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_SOURCE_LOCATION_INL_H_
#include "hphp/runtime/vm/source-location-inl.h"
#undef incl_HPHP_VM_SOURCE_LOCATION_INL_H_

