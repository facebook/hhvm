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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

RDS::Link<uint64_t> g_bytecodesLLVM{RDS::kInvalidHandle};
RDS::Link<uint64_t> g_bytecodesVasm{RDS::kInvalidHandle};

///////////////////////////////////////////////////////////////////////////////

folly::Range<Vlabel*> succs(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::jcc:       return {inst.jcc_.targets, 2};
    case Vinstr::jmp:       return {&inst.jmp_.target, 1};
    case Vinstr::phijmp:    return {&inst.phijmp_.target, 1};
    case Vinstr::phijcc:    return {inst.phijcc_.targets, 2};
    case Vinstr::unwind:    return {inst.unwind_.targets, 2};
    case Vinstr::vinvoke:   return {inst.vinvoke_.targets, 2};
    case Vinstr::cbcc:      return {inst.cbcc_.targets, 2};
    case Vinstr::tbcc:      return {inst.tbcc_.targets, 2};
    case Vinstr::hcunwind:  return {inst.hcunwind_.targets, 2};
    default:                return {nullptr, nullptr};
  }
}

folly::Range<Vlabel*> succs(Vblock& block) {
  if (block.code.empty()) return {nullptr, nullptr};
  return succs(block.code.back());
}

folly::Range<const Vlabel*> succs(const Vinstr& inst) {
  return succs(const_cast<Vinstr&>(inst)).castToConst();
}

folly::Range<const Vlabel*> succs(const Vblock& block) {
  return succs(const_cast<Vblock&>(block)).castToConst();
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Sort blocks in reverse postorder, and try to arrange fall-through blocks in
 * the same area to be close together.
 */
struct BlockSorter {
  explicit BlockSorter(const Vunit& unit)
    : unit(unit)
    , visited(unit.blocks.size())
  {
    blocks.reserve(unit.blocks.size());
  }

  unsigned area(Vlabel b) {
    return (unsigned)unit.blocks[b].area;
  }

  void dfs(Vlabel b) {
    assert(size_t(b) < unit.blocks.size() &&
           !unit.blocks[b].code.empty());

    if (visited.test(b)) return;
    visited.set(b);

    if (area(b) == 0) {
      for (auto s : succs(unit.blocks[b])) {
        // visit colder
        if (area(s) > area(b)) dfs(s);
      }
      for (auto s : succs(unit.blocks[b])) {
        if (area(s) <= area(b)) dfs(s);
      }
    } else {
      for (auto s : succs(unit.blocks[b])) dfs(s);
    }
    blocks.push_back(b);
  }

  /*
   * Data members.
   */
  const Vunit& unit;
  jit::vector<Vlabel> blocks;
  boost::dynamic_bitset<> visited;
};

jit::vector<Vlabel> sortBlocks(const Vunit& unit) {
  BlockSorter s(unit);
  s.dfs(unit.entry);
  std::reverse(s.blocks.begin(), s.blocks.end());

  // Put the blocks containing "fallthru" last; expect at most one per Vunit.
  std::stable_partition(s.blocks.begin(), s.blocks.end(), [&] (Vlabel b) {
    auto& block = unit.blocks[b];
    auto& code = block.code;
    return code.back().op != Vinstr::fallthru;
  });

  return s.blocks;
}

jit::vector<Vlabel> layoutBlocks(const Vunit& unit) {
  auto blocks = sortBlocks(unit);
  // Partition into main/cold/frozen areas without changing relative order, and
  // the end{} block will be last.
  auto coldIt = std::stable_partition(blocks.begin(), blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Main &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  std::stable_partition(coldIt, blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Cold &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  return blocks;
}

///////////////////////////////////////////////////////////////////////////////
}}
