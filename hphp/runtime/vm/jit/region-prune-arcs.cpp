/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/region-prune-arcs.h"

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#include <folly/Format.h>

#include "hphp/util/assertions.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(pgo);

namespace {

//////////////////////////////////////////////////////////////////////

struct State {
  bool initialized = false;
  std::vector<Type> locals;
};

struct BlockInfo {
  RegionDesc::BlockId blockID;
  State in;
  State out;
};

//////////////////////////////////////////////////////////////////////

std::string DEBUG_ONLY show(const State& state) {
  if (!state.initialized) return "<uninit>\n";
  auto ret = std::string{};
  for (auto locID = uint32_t{0}; locID < state.locals.size(); ++locID) {
    auto const ty = state.locals[locID];
    if (ty < TGen) folly::format(&ret, "  L{}: {}\n", locID, ty.toString());
  }
  return ret;
}

State entry_state(const RegionDesc& region) {
  auto const numLocals = region.start().func()->numLocals();
  auto ret = State{};
  ret.initialized = true;
  ret.locals.resize(numLocals, TGen);
  return ret;
}

bool merge_into(State& dst, const State& src) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  always_assert(dst.locals.size() == src.locals.size());

  auto changed = false;
  for (auto loc = uint32_t{0}; loc < src.locals.size(); ++loc) {
    auto const old_ty = dst.locals[loc];
    dst.locals[loc] |= src.locals[loc];
    if (old_ty != dst.locals[loc]) changed = true;
  }

  return changed;
}

//////////////////////////////////////////////////////////////////////

bool preconds_may_pass(const RegionDesc::Block& block,
                       const State& state) {
  // Return false if the type of any local is bottom, which can only
  // happen in unreachable paths.
  for (auto& locType : state.locals) {
    if (locType == TBottom) return false;
  }

  auto const& preConds = block.typePreConditions();
  auto preCond_it = preConds.find(block.start());
  for (;
      preCond_it != end(preConds) && preCond_it->first == block.start();
      ++preCond_it) {
    auto const preCond = preCond_it->second;
    using L = RegionDesc::Location::Tag;
    switch (preCond.location.tag()) {
    case L::Stack: break;
    case L::Local:
      {
        auto const loc = preCond.location.localId();
        assertx(loc < state.locals.size());
        if (!state.locals[loc].maybe(preCond.type)) {
          FTRACE(6, "  x B{}'s precond {} fails (Local{} is {})\n",
                 block.id(), show(preCond), loc, state.locals[loc]);
          return false;
        }
      }
      break;
    }
  }
  return true;
}

// PostConditions of a block are our local transfer functions.
// Changed types are overwritten, while refined types are intersected
// with the current type.
void apply_transfer_function(State& dst, const PostConditions& postConds) {
  for (auto& p : postConds.refined) {
    using L = RegionDesc::Location::Tag;
    switch (p.location.tag()) {
      case L::Stack:
        break;
      case L::Local: {
        const auto locId = p.location.localId();
        assert(locId < dst.locals.size());
        dst.locals[locId] = dst.locals[locId] & p.type;
        break;
      }
    }
  }

  for (auto& p : postConds.changed) {
    using L = RegionDesc::Location::Tag;
    switch (p.location.tag()) {
      case L::Stack:
        break;
      case L::Local: {
        const auto locId = p.location.localId();
        assert(locId < dst.locals.size());
        dst.locals[locId] = p.type;
        break;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

void region_prune_arcs(RegionDesc& region) {
  FTRACE(4, "region_prune_arcs\n");

  region.sortBlocks();
  auto const sortedBlocks = region.blocks();

  // Maps region block ids to their RPO ids.
  auto blockToRPO = std::unordered_map<RegionDesc::BlockId,uint32_t>{};

  auto blockInfos = std::vector<BlockInfo>(sortedBlocks.size());
  auto workQ = dataflow_worklist<uint32_t>(sortedBlocks.size());
  for (auto rpoID = uint32_t{0}; rpoID < sortedBlocks.size(); ++rpoID) {
    auto const& b = sortedBlocks[rpoID];
    auto& binfo = blockInfos[rpoID];
    binfo.blockID = b->id();
    blockToRPO[binfo.blockID] = rpoID;
  }
  workQ.push(0);
  blockInfos[0].in = entry_state(region);

  FTRACE(4, "Iterating:\n");
  do {
    auto const rpoID = workQ.pop();
    auto& binfo = blockInfos[rpoID];
    FTRACE(4, "B{}\n", binfo.blockID);

    /*
     * This code currently assumes inlined functions were entirely contained
     * within a single profiling translation, and will need updates if we
     * inline bigger things in a way visible to region selection.
     *
     * Note: inlined blocks /may/ have postConditions, if they are the last
     * blocks from profiling translations.  Currently any locations referred to
     * in postconditions for these blocks are for the outermost caller, so this
     * code handles that correctly.
     */
    if (region.block(binfo.blockID)->inlineLevel() != 0) {
      assertx(region.block(binfo.blockID)->typePreConditions().empty());
    }

    binfo.out = binfo.in;
    apply_transfer_function(
      binfo.out,
      region.block(binfo.blockID)->postConds()
    );

    for (auto& succ : region.succs(binfo.blockID)) {
      auto const succRPO = blockToRPO.find(succ);
      assertx(succRPO != end(blockToRPO));
      auto& succInfo = blockInfos[succRPO->second];
      if (preconds_may_pass(*region.block(succInfo.blockID), binfo.out)) {
        if (merge_into(succInfo.in, binfo.out)) {
          FTRACE(5, "  -> {}\n", succInfo.blockID);
          workQ.push(succRPO->second);
        }
      }
    }
  } while (!workQ.empty());

  FTRACE(2, "\nPostConds fixed point:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& s : blockInfos) {
        folly::format(&ret, "B{}:\n{}", s.blockID, show(s.in));
      }
      return ret;
    }()
  );

  // Now remove any edge that looks like it will unconditionally fail type
  // predictions, and completely remove any block that can't be reached.
  using ArcIDs = std::pair<RegionDesc::BlockId,RegionDesc::BlockId>;
  auto toRemove = std::vector<ArcIDs>{};
  for (auto rpoID = uint32_t{0}; rpoID < sortedBlocks.size(); ++rpoID) {
    auto const& binfo = blockInfos[rpoID];

    for (auto& succ : region.succs(binfo.blockID)) {
      auto const succRPO = blockToRPO.find(succ);
      assertx(succRPO != end(blockToRPO));
      auto const& succInfo = blockInfos[succRPO->second];
      if (!binfo.in.initialized ||
          !succInfo.in.initialized ||
          !preconds_may_pass(*region.block(succInfo.blockID), binfo.out)) {
        FTRACE(2, "Pruning arc: B{} -> B{}\n",
               binfo.blockID,
               succInfo.blockID);
        toRemove.emplace_back(binfo.blockID, succInfo.blockID);
      }
    }

    for (auto& r : toRemove) region.removeArc(r.first, r.second);
    toRemove.clear();
  }

  // Get rid of the completely unreachable blocks, now that any arcs to/from
  // them are gone.
  for (auto rpoID = uint32_t{0}; rpoID < sortedBlocks.size(); ++rpoID) {
    auto const& binfo = blockInfos[rpoID];
    if (!binfo.in.initialized) {
      FTRACE(2, "Pruning block: B{}\n", binfo.blockID);
      region.deleteBlock(binfo.blockID);
    }
  }
  FTRACE(2, "\n");
}

}}
