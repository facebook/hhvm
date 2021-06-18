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

#include "hphp/runtime/vm/source-location.h"

#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/util/functional.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace SourceLocation {

//////////////////////////////////////////////////////////////////////

/**
 * Generate line->vector<OffsetRange> reverse map from SourceLocTable.
 *
 * Algorithm:
 * We first generate the OffsetRange for each SourceLoc,
 * then sort the pair<SourceLoc, OffsetRange> in most nested to outward order
 * so that we can add vector<OffsetRange> for nested lines first.
 * After merging continuous duplicate line ranges into one we build the final
 * map by adding vector<OffsetRange> for each line in the LineRange only if
 * it hasn't got any vector<OffsetRange> from inner LineRange yet.
 * By doing this we ensure the outer LineRange's vector<OffsetRange> will not be
 * added for inner lines.
 */
void generateLineToOffsetRangesMap(
  const SourceLocTable& srcLocTable,
  LineToOffsetRangeVecMap& map
) {
  // First generate an OffsetRange for each SourceLoc.
  struct LineRange {
    LineRange(int start, int end)
    : line0(start), line1(end)
    {}
    int line0;
    int line1;

    bool operator!=(const LineRange& other) const {
      return this->line0 != other.line0 || this->line1 != other.line1;
    }
  };

  using LineRangeOffsetRangePair = std::pair<LineRange, OffsetRange>;
  std::vector<LineRangeOffsetRangePair> lineRangesTable;
  Offset baseOff = 0;
  for (const auto& sourceLoc: srcLocTable) {
    Offset pastOff = sourceLoc.pastOffset();
    OffsetRange offsetRange(baseOff, pastOff);
    LineRange lineRange(sourceLoc.val().line0, sourceLoc.val().line1);
    lineRangesTable.emplace_back(lineRange, offsetRange);
    baseOff = pastOff;
  }

  // Sort the line ranges in most nested to outward order:
  // First sort them in ascending order by line range end;
  // if range end ties, sort in descending order by line range start.
  std::sort(
    lineRangesTable.begin(),
    lineRangesTable.end(),
    [](const LineRangeOffsetRangePair& a, const LineRangeOffsetRangePair& b) {
      return a.first.line1 == b.first.line1 ?
        a.first.line0 > b.first.line0 :
        a.first.line1 < b.first.line1;
    }
  );

  // Merge continuous duplicate line ranges into one.
  using LineRangeToOffsetRangesTable =
    std::vector<std::pair<LineRange, std::vector<OffsetRange>>>;
  LineRangeToOffsetRangesTable lineRangeToOffsetRangesTable;
  for (auto i = 0; i < lineRangesTable.size(); ++i) {
    if (i == 0 || lineRangesTable[i].first != lineRangesTable[i-1].first) {
      // New line range starts.
      std::vector<OffsetRange> offsetRanges;
      offsetRanges.emplace_back(lineRangesTable[i].second);
      const auto& lineRange = lineRangesTable[i].first;
      lineRangeToOffsetRangesTable.emplace_back(lineRange, offsetRanges);
    } else {
      // Duplicate LineRange.
      assertx(lineRangeToOffsetRangesTable.size() > 0);
      auto& offsetRanges = lineRangeToOffsetRangesTable.back().second;
      offsetRanges.emplace_back(lineRangesTable[i].second);
    }
  }

  // Generate the final line to offset ranges map.
  for (auto& entry: lineRangeToOffsetRangesTable) {
    // Sort the offset ranges of each line range.
    std::sort(
      entry.second.begin(),
      entry.second.end(),
      [](const OffsetRange& a, const OffsetRange& b) {
        return a.base == b.base ? a.past < b.past : a.base < b.base;
      }
    );

    const auto& offsetRanges = entry.second;
    auto line0 = entry.first.line0;
    auto line1 = entry.first.line1;
    for (auto line = line0; line <= line1; ++line) {
      // Only add if not added by inner LineRange yet.
      if (map.find(line) == map.end()) {
        map[line] = offsetRanges;
      }
    }
  }
}

LineInfo getLineInfo(const LineTable& table, Offset pc) {
  auto const it =
    std::upper_bound(begin(table), end(table), LineEntry{ pc, -1 });

  auto const e = end(table);
  if (it != e) {
    auto const line = it->val();
    if (line > 0) {
      auto const pastOff = it->pastOffset();
      auto const baseOff = it == begin(table) ?
        pc : std::prev(it)->pastOffset();
      assertx(baseOff <= pc && pc < pastOff);
      return { { baseOff, pastOff }, line };
    }
  }
  return LineInfo{ { pc, pc + 1 }, -1 };
}

int getLineNumber(const LineTable& table, Offset pc) {
  auto const key = LineEntry(pc, -1);
  auto it = std::upper_bound(begin(table), end(table), key);
  if (it != end(table)) {
    assertx(pc < it->pastOffset());
    return it->val();
  }
  return -1;
}

bool getLoc(const SourceLocTable& table, Offset pc, SourceLoc& sLoc) {
  SourceLocEntry key(pc, sLoc);
  auto it = std::upper_bound(table.begin(), table.end(), key);
  if (it != table.end()) {
    assertx(pc < it->pastOffset());
    sLoc = it->val();
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}
