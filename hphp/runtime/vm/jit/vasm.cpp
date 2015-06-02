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

rds::Link<uint64_t> g_bytecodesLLVM{rds::kInvalidHandle};
rds::Link<uint64_t> g_bytecodesVasm{rds::kInvalidHandle};

///////////////////////////////////////////////////////////////////////////////

folly::Range<Vlabel*> succs(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::bindcall:  return {inst.bindcall_.targets, 2};
    case Vinstr::contenter: return {inst.contenter_.targets, 2};
    case Vinstr::jcc:       return {inst.jcc_.targets, 2};
    case Vinstr::jcci:      return {&inst.jcci_.target, 1};
    case Vinstr::jmp:       return {&inst.jmp_.target, 1};
    case Vinstr::phijmp:    return {&inst.phijmp_.target, 1};
    case Vinstr::phijcc:    return {inst.phijcc_.targets, 2};
    case Vinstr::unwind:    return {inst.unwind_.targets, 2};
    case Vinstr::vcallstub: return {inst.vcallstub_.targets, 2};
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

boost::dynamic_bitset<> backedgeTargets(const Vunit& unit,
                                        const jit::vector<Vlabel>& rpoBlocks) {
  boost::dynamic_bitset<> ret(unit.blocks.size());
  boost::dynamic_bitset<> seen(unit.blocks.size());

  for (auto label : rpoBlocks) {
    seen.set(label);
    for (auto target : succs(unit.blocks[label])) {
      if (seen.test(target)) ret.set(target);
    }
  }

  return ret;
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
    assert_no_log(size_t(b) < unit.blocks.size());
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
    return !code.empty() && code.back().op != Vinstr::fallthru;
  });

  return s.blocks;
}

///////////////////////////////////////////////////////////////////////////////

}}
