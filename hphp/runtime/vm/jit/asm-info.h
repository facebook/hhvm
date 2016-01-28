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

#ifndef incl_HPHP_JIT_ASM_INFO_H_
#define incl_HPHP_JIT_ASM_INFO_H_

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/state-multi-map.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/types.h"
#include <algorithm>

namespace HPHP { namespace jit {

// Information about where code was generated, for pretty-printing.
struct AsmInfo {
  explicit AsmInfo(const IRUnit& unit)
    : mainInstBlockMap(unit, 0)
    , coldInstBlockMap(unit, 0)
    , frozenInstBlockMap(unit, 0)
  {}

  // Asm address info for each instruction and block
  typedef StateMultiMap<IRInstruction, TcaRange> InstRanges;
  typedef StateMultiMap<Block, TcaRange> BlockRanges;
  // Map from IRInstruction to Block id.
  typedef StateVector<IRInstruction, uint32_t> InstBlockMap;

  InstRanges mainInstRanges;
  InstRanges coldInstRanges;
  InstRanges frozenInstRanges;
  BlockRanges mainBlockRanges;
  BlockRanges coldBlockRanges;
  BlockRanges frozenBlockRanges;
  InstBlockMap mainInstBlockMap;
  InstBlockMap coldInstBlockMap;
  InstBlockMap frozenInstBlockMap;

  template<typename T>
  bool instRangeExists(const T& haystack, const TcaRange& needle) const {
    for (const auto& rng : haystack) {
      if (contains(needle, rng.second)) return true;
    }
    return false;
  }

  bool instRangeExists(AreaIndex area, const TcaRange& rng) const {
    return (instRangeExists(instRangesForArea(AreaIndex::Main), rng) ||
            instRangeExists(instRangesForArea(AreaIndex::Cold), rng) ||
            instRangeExists(instRangesForArea(AreaIndex::Frozen), rng));
  }

  InstRanges& instRangesForArea(AreaIndex area) {
    switch (area) {
    case AreaIndex::Main:
      return mainInstRanges;
    case AreaIndex::Cold:
      return coldInstRanges;
    case AreaIndex::Frozen:
      return frozenInstRanges;
    }
    not_reached();
  }

  const InstRanges& instRangesForArea(AreaIndex area) const {
    return const_cast<AsmInfo*>(this)->instRangesForArea(area);
  }

  BlockRanges& blockRangesForArea(AreaIndex area) {
    switch (area) {
    case AreaIndex::Main:
      return mainBlockRanges;
    case AreaIndex::Cold:
      return coldBlockRanges;
    case AreaIndex::Frozen:
      return frozenBlockRanges;
    }
    not_reached();
  }

  const BlockRanges& blockRangesForArea(AreaIndex area) const {
    return const_cast<AsmInfo*>(this)->blockRangesForArea(area);
  }

  void updateForInstruction(const IRInstruction* inst,
                            AreaIndex area,
                            TCA start,
                            TCA end) {
    if (start != end) {
      auto newRange = updateRange(instRangesForArea(area),
                                  inst->id(),
                                  area,
                                  start,
                                  end,
                                  false);
      instBlockMapForArea(area)[inst] = inst->block()->id();
      updateRange(blockRangesForArea(area),
                  inst->block()->id(),
                  area,
                  newRange.begin(),
                  newRange.end(),
                  true);
    }
  }

  void updateForBlock(AreaIndex area,
                      uint32_t instId,
                      const TcaRange& instRange) {
    if (!instRange.empty()) {
      updateRange(blockRangesForArea(area),
                  instBlockMapForArea(area)[instId],
                  area,
                  instRange.begin(),
                  instRange.end(),
                  true);
    }
  }

  void clearBlockRangesForArea(AreaIndex area) {
    blockRangesForArea(area).clear();
  }

  bool validate() const {
    return (validateInstructionRanges(AreaIndex::Main, AreaIndex::Main) &&
            validateInstructionRanges(AreaIndex::Main, AreaIndex::Cold) &&
            validateInstructionRanges(AreaIndex::Main, AreaIndex::Frozen) &&
            validateInstructionRanges(AreaIndex::Cold, AreaIndex::Cold) &&
            validateInstructionRanges(AreaIndex::Cold, AreaIndex::Frozen) &&
            validateInstructionRanges(AreaIndex::Frozen, AreaIndex::Frozen) &&
            validateBlockRanges(AreaIndex::Main) &&
            validateBlockRanges(AreaIndex::Cold) &&
            validateBlockRanges(AreaIndex::Frozen) &&
            validateBlocks(AreaIndex::Main, AreaIndex::Main) &&
            validateBlocks(AreaIndex::Main, AreaIndex::Cold) &&
            validateBlocks(AreaIndex::Main, AreaIndex::Frozen) &&
            validateBlocks(AreaIndex::Cold, AreaIndex::Cold) &&
            validateBlocks(AreaIndex::Cold, AreaIndex::Frozen) &&
            validateBlocks(AreaIndex::Frozen, AreaIndex::Frozen));
  }

  void dump() const {
    dumpBlockRanges("Main", mainBlockRanges);
    dumpInstructionRanges("Main", mainInstRanges);
    dumpBlockRanges("Cold", coldBlockRanges);
    dumpInstructionRanges("Cold", coldInstRanges);
    dumpBlockRanges("Frozen", frozenBlockRanges);
    dumpInstructionRanges("Frozen", frozenInstRanges);
  }

 private:
  InstBlockMap& instBlockMapForArea(AreaIndex area) {
    switch (area) {
    case AreaIndex::Main:
      return mainInstBlockMap;
    case AreaIndex::Cold:
      return coldInstBlockMap;
    case AreaIndex::Frozen:
      return frozenInstBlockMap;
    }
    not_reached();
  }

  const InstBlockMap& instBlockMapForArea(AreaIndex area) const {
    return const_cast<AsmInfo*>(this)->instBlockMapForArea(area);
  }

  template <typename MM>
  TcaRange updateRange(MM& stateMap,
                       uint32_t id,
                       AreaIndex area,
                       TCA start,
                       TCA end,
                       bool merge) {
    auto ranges = stateMap[id];
    auto itr = ranges.first;

    if (merge) {
      // update range multi-map for the given id.  If the new (start,end)
      // range overlaps an existing range, just grow the existing entry
      // rather than adding a new one.

      while (itr != ranges.second) {
        if (overlappingOrAdjacent(itr->second, TcaRange{start, end})) {
          break;
        }
        ++itr;
      }

      if (itr != ranges.second) {
        auto& oldRange = itr->second;
        start = oldRange.start() ? std::min(oldRange.start(), start) : start;
        end = oldRange.end() ? std::max(oldRange.end(), end) : end;
        oldRange = TcaRange { start, end };
        return oldRange;
      }
    }

    auto newRange = TcaRange{start, end};
    if (itr == ranges.second || itr->second != newRange) {
      stateMap.insert(id, newRange);
    }
    return newRange;
  }

  bool validateInstructionRanges(AreaIndex area_a, AreaIndex area_b) const {
    const auto& arngs = instRangesForArea(area_a);
    const auto& brngs = instRangesForArea(area_b);
    bool sawBadInst = false;

    for (auto irnga = arngs.begin(); irnga != arngs.end(); ++irnga) {
      for (auto irngb = area_a == area_b ? std::next(irnga) : brngs.begin();
          irngb != brngs.end(); ++irngb) {
        if (!disjoint(irnga->second, irngb->second)) {
          std::cout << areaAsString(area_a) << " instructions: "
                    << irnga->first << " ("
                    << (void*)irnga->second.begin() << ", "
                    << (void*)irnga->second.end()
                    << ") and " << areaAsString(area_b) << " "
                    << irngb->first << " ("
                    << (void*)irngb->second.begin() << ", "
                    << (void*)irngb->second.end()
                    << ") are not disjoint.\n";
          sawBadInst = true;
        }
      }
    }
    return !sawBadInst;
  }

  bool validateBlockRanges(AreaIndex area) const {
    bool sawBadInst = false;
    for (auto& irng : instRangesForArea(area)) {
      bool ok = false;
      for (auto& brng : blockRangesForArea(area)) {
        if (contains(brng.second, irng.second)) {
          ok = true;
          break;
        }
      }
      if (!ok) {
        sawBadInst = true;
        std::cout << areaAsString(area) << " instruction: "
                  << irng.first << " ("
                  << (void*)irng.second.begin() << ", "
                  << (void*)irng.second.end()
                  << ") is not properly contained in any block.\n";
      }
    }
    return !sawBadInst;
  }

  bool validateBlocks(AreaIndex area_a, AreaIndex area_b) const {
    auto& arng = blockRangesForArea(area_a);
    auto& brng = blockRangesForArea(area_b);
    bool sawBadBlock = false;
    for (auto itra = arng.begin(); itra != arng.end(); ++itra) {
      for (auto itrb = area_a == area_b ? std::next(itra) : brng.begin();
          itrb != brng.end(); ++itrb) {
        if (!disjoint(itra->second, itrb->second)) {
          std::cout << areaAsString(area_a) << " block: " << itra->first << " ("
                    << (void*)itra->second.begin() << ", "
                    << (void*)itra->second.end()
                    << ") and " << areaAsString(area_b) << " "
                    << itrb->first << " ("
                    << (void*)itrb->second.begin() << ", "
                    << (void*)itrb->second.end()
                    << ") are not disjoint.\n";
          sawBadBlock = true;
        }
      }
    }
    return !sawBadBlock;
  }

  void dumpInstructionRanges(const char* area, const InstRanges& rngs) const {
    std::cout << area << " instructions:\n";
    for (auto& rng : rngs) {
      std::cout << rng.first << " = (" << (void*)rng.second.begin()
                << ", " << (void*)rng.second.end() << ")\n";
    }
    std::cout << "\n";
  }

  void dumpBlockRanges(const char* area, const BlockRanges& rngs) const {
    std::cout << area << " blocks:\n";
    for (auto itr = rngs.begin(); itr != rngs.end(); ++itr) {
      if (!itr->second.empty()) {
        std::cout << itr->first << " = (" << (void*)itr->second.begin()
                  << ", " << (void*)itr->second.end() << ")\n";
      }
    }
    std::cout << "\n";
  }

  static bool overlappingOrAdjacent(const TcaRange& a, const TcaRange& b) {
    return ((a.begin() >= b.begin() && a.begin() <= b.end()) ||
            (a.end() >= b.begin() && a.end() <= b.end()) ||
            (b.begin() >= a.begin() && b.begin() <= a.end()) ||
            (b.end() >= a.begin() && b.end() <= a.end()));
  }

  // does a contain b?
  static bool contains(const TcaRange& a, const TcaRange& b) {
    return b.begin() >= a.begin() && b.end() <= a.end();
  }

  static bool disjoint(const TcaRange& a, const TcaRange& b) {
    return a.begin() >= b.end() || a.end() <= b.begin();
  }
};

}}

#endif
