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

#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/functional.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * We store 'detailed' line number information on a table on the side, because
 * in production modes for HHVM it's generally not useful (which keeps Unit
 * smaller in that case)---this stuff is only used for the debugger, where we
 * can afford the lookup here.  The normal Unit m_lineMap is capable of
 * producing enough line number information for things needed in production
 * modes (backtraces, warnings, etc).
 */

struct ExtendedLineInfo {
  SourceLocTable sourceLocTable;

  /*
   * Map from source lines to a collection of all the bytecode ranges the line
   * encompasses.
   *
   * The value type of the map is a list of offset ranges, so a single line
   * with several sub-statements may correspond to the bytecodes of all of the
   * sub-statements.
   *
   * May not be initialized.  Lookups need to check if it's empty() and if so
   * compute it from sourceLocTable.
   */
  LineToOffsetRangeVecMap lineToOffsetRange;
};

using ExtendedLineInfoCache = tbb::concurrent_hash_map<
  const Unit*,
  ExtendedLineInfo,
  pointer_hash<Unit>
>;
ExtendedLineInfoCache s_extendedLineInfo;

using LineTableStash = tbb::concurrent_hash_map<
  const Unit*,
  LineTable,
  pointer_hash<Unit>
>;
LineTableStash s_lineTables;

struct LineCacheEntry {
  LineCacheEntry(const Unit* unit, LineTable&& table)
    : unit{unit}
    , table{std::move(table)}
  {}
  const Unit* unit;
  LineTable table;
};
std::array<std::atomic<LineCacheEntry*>, 512> s_lineCache;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace SourceLocation {

//////////////////////////////////////////////////////////////////////

static SourceLocTable loadLocTable(const Unit* unit) {
  auto ret = SourceLocTable{};
  if (unit->repoID() == RepoIdInvalid) return ret;

  Lock lock(g_classesMutex);
  auto& urp = Repo::get().urp();
  urp.getSourceLocTab[unit->repoID()].get(unit->sn(), ret);
  return ret;
}

/*
 * Return the Unit's SourceLocTable, extracting it from the repo if
 * necessary.
 */
const SourceLocTable& getLocTable(const Unit* unit) {
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, unit)) {
      return acc->second.sourceLocTable;
    }
  }

  // Try to load it while we're not holding the lock.
  auto newTable = loadLocTable(unit);
  ExtendedLineInfoCache::accessor acc;
  if (s_extendedLineInfo.insert(acc, unit)) {
    acc->second.sourceLocTable = std::move(newTable);
  }
  return acc->second.sourceLocTable;
}

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
static void generateLineToOffsetRangesMap(
  const Unit* unit,
  LineToOffsetRangeVecMap& map
) {
  // First generate an OffsetRange for each SourceLoc.
  auto const& srcLocTable = getLocTable(unit);

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

/*
 * Return a copy of the Unit's line to OffsetRangeVec table.
 */
LineToOffsetRangeVecMap getLineToOffsetRangeVecMap(const Unit* unit) {
  {
    ExtendedLineInfoCache::const_accessor acc;
    if (s_extendedLineInfo.find(acc, unit)) {
      if (!acc->second.lineToOffsetRange.empty()) {
        return acc->second.lineToOffsetRange;
      }
    }
  }

  LineToOffsetRangeVecMap map;
  generateLineToOffsetRangesMap(unit, map);

  ExtendedLineInfoCache::accessor acc;
  if (!s_extendedLineInfo.find(acc, unit)) {
    always_assert_flog(0, "ExtendedLineInfoCache was not found when it should "
      "have been");
  }
  if (acc->second.lineToOffsetRange.empty()) {
    acc->second.lineToOffsetRange = std::move(map);
  }
  return acc->second.lineToOffsetRange;
}

const LineTable* getLineTable(const Unit* unit) {
  LineTableStash::accessor acc;
  if (s_lineTables.find(acc, unit)) {
    return &acc->second;
  }
  return nullptr;
}

const LineTable& loadLineTable(const Unit* unit) {
  assertx(unit->repoID() != RepoIdInvalid);
  if (!RO::RepoAuthoritative) {
    LineTableStash::const_accessor acc;
    if (s_lineTables.find(acc, unit)) {
      return acc->second;
    }
  }

  auto const hash = pointer_hash<Unit>{}(unit) % s_lineCache.size();
  auto& entry = s_lineCache[hash];
  if (auto const p = entry.load(std::memory_order_acquire)) {
    if (p->unit == unit) return p->table;
  }

  // We already hold a lock on the unit in Unit::getLineNumber below,
  // so nobody else is going to be reading the line table while we are
  // (this is only an efficiency concern).
  auto& urp = Repo::get().urp();
  auto table = LineTable{};
  urp.getUnitLineTable[unit->repoID()].get(unit->sn(), table);

  // Loading line tables for each unseen line while coverage is enabled can
  // cause the treadmill to to carry an enormous number of discarded
  // LineTables, so instead cache the table permanently in s_lineTables.
  if (UNLIKELY(g_context &&
               (unit->isCoverageEnabled() || RID().getCoverage()))) {
    LineTableStash::accessor acc;
    if (s_lineTables.insert(acc, unit)) {
      acc->second = std::move(table);
    }
    return acc->second;
  }

  auto const p = new LineCacheEntry(unit, std::move(table));
  if (auto const old = entry.exchange(p, std::memory_order_release)) {
    Treadmill::enqueue([old] { delete old; });
  }
  return p->table;
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

void stashLineTable(const Unit* unit, LineTable table) {
  LineTableStash::accessor acc;
  if (s_lineTables.insert(acc, unit)) {
    acc->second = std::move(table);
  }
}

void stashExtendedLineTable(const Unit* unit, SourceLocTable table) {
  ExtendedLineInfoCache::accessor acc;
  if (s_extendedLineInfo.insert(acc, unit)) {
    acc->second.sourceLocTable = std::move(table);
  }
}

void removeUnit(const Unit* unit) {
  s_extendedLineInfo.erase(unit);
  s_lineTables.erase(unit);

  auto const hash = pointer_hash<Unit>{}(unit) % s_lineCache.size();
  auto& entry = s_lineCache[hash];
  if (auto lce = entry.load(std::memory_order_acquire)) {
    if (lce->unit == unit &&
        entry.compare_exchange_strong(lce, nullptr,
                                      std::memory_order_release)) {
      Treadmill::enqueue([lce] { delete lce; });
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

}
