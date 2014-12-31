/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/types.h"
#include <algorithm>

namespace HPHP { namespace jit {

// Information about where code was generated, for pretty-printing.
struct AsmInfo {
  explicit AsmInfo(const IRUnit& unit)
    : asmInstRanges(unit, TcaRange(nullptr, nullptr))
    , coldInstRanges(unit, TcaRange(nullptr, nullptr))
    , frozenInstRanges(unit, TcaRange(nullptr, nullptr))
    , asmBlockRanges(unit, TcaRange(nullptr, nullptr))
    , coldBlockRanges(unit, TcaRange(nullptr, nullptr))
    , frozenBlockRanges(unit, TcaRange(nullptr, nullptr))
  {}

  // Asm address info for each instruction and block
  typedef StateVector<IRInstruction, TcaRange> InstRanges;
  typedef StateVector<Block, TcaRange> BlockRanges;

  InstRanges asmInstRanges;
  InstRanges coldInstRanges;
  InstRanges frozenInstRanges;
  BlockRanges asmBlockRanges;
  BlockRanges coldBlockRanges;
  BlockRanges frozenBlockRanges;

  InstRanges& instRangesForArea(AreaIndex area) {
    switch (area) {
    case AreaIndex::Main:
      return asmInstRanges;
    case AreaIndex::Cold:
      return coldInstRanges;
    case AreaIndex::Frozen:
      return frozenInstRanges;
    default:
      not_reached();
      return asmInstRanges;
    }
  }

  const InstRanges& instRangesForArea(AreaIndex area) const {
    return const_cast<AsmInfo*>(this)->instRangesForArea(area);
  }

  BlockRanges& blockRangesForArea(AreaIndex area) {
    switch (area) {
    case AreaIndex::Main:
      return asmBlockRanges;
    case AreaIndex::Cold:
      return coldBlockRanges;
    case AreaIndex::Frozen:
      return frozenBlockRanges;
    default:
      not_reached();
      return asmBlockRanges;
    }
  }

  const BlockRanges& blockRangesForArea(AreaIndex area) const {
    return const_cast<AsmInfo*>(this)->blockRangesForArea(area);
  }

  void updateForInstruction(const IRInstruction* inst,
                            AreaIndex area,
                            TCA start, TCA end) {
    auto& instRanges = instRangesForArea(area);
    auto oldRange = instRanges[inst];

    start = oldRange.start() ? std::min(oldRange.start(), start) : start;
    end = oldRange.end() ? std::max(oldRange.end(), end) : end;

    instRanges[inst] = TcaRange { start, end };
    updateForBlock(inst->block(), area, start, end);
  }

  void updateForBlock(Block* block, AreaIndex area, TCA start, TCA end) {
    auto& blockRanges = blockRangesForArea(area);
    auto oldRange = blockRanges[block];

    start = oldRange.start() ? std::min(oldRange.start(), start) : start;
    end = oldRange.end() ? std::max(oldRange.end(), end) : end;

    blockRanges[block] = TcaRange { start, end };
  }
};

}}

#endif
