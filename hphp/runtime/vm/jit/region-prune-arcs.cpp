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
#include "hphp/runtime/vm/jit/region-prune-arcs.h"

#include <string>
#include <utility>
#include <vector>

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
  bool initialized{false};
  std::vector<Type> locals;
  Type mbase{TCell};
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
    if (ty < TCell) folly::format(&ret, "  L{}: {}\n", locID, ty.toString());
  }
  auto const ty = state.mbase;
  if (ty < TCell) folly::format(&ret, "  M{{}}: {}\n", ty.toString());
  return ret;
}

State entry_state(const RegionDesc& region, std::vector<Type>* input) {
  auto ret = State{};
  ret.initialized = true;

  if (input) ret.locals = *input;
  auto const func = region.start().func();
  ret.locals.resize(func->numLocals(), TCell);

  return ret;
}

bool merge_type(Type& lhs, const Type& rhs) {
  auto const old = lhs;
  lhs |= rhs;
  return lhs != old;
}

bool merge_into(State& dst, const State& src) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }
  auto changed = false;

  always_assert(dst.locals.size() == src.locals.size());
  for (auto i = uint32_t{0}; i < src.locals.size(); ++i) {
    changed |= merge_type(dst.locals[i], src.locals[i]);
  }

  changed |= merge_type(dst.mbase, src.mbase);

  return changed;
}

//////////////////////////////////////////////////////////////////////

bool is_tracked(Location l) {
  switch (l.tag()) {
    case LTag::Local:
    case LTag::MBase:
      return true;
    case LTag::Stack:
      return false;
  }
  not_reached();
}

Type& type_of(State& state, Location l) {
  switch (l.tag()) {
    case LTag::Local: {
      auto const locID = l.localId();
      assertx(locID < state.locals.size());
      return state.locals[locID];
    }
    case LTag::Stack:
    case LTag::MBase:
      return state.mbase;
  }
  not_reached();
}

Type type_of(const State& state, Location l) {
  return type_of(const_cast<State&>(state), l);
}

bool preconds_may_pass(const RegionDesc::Block& block,
                       const State& state) {
  // Bail if any type is Bottom, which can only happen in unreachable paths.
  for (auto const& ty : state.locals) {
    if (ty == TBottom) return false;
  }

  auto const& preConds = block.typePreConditions();

  for (auto const& p : preConds) {
    if (!is_tracked(p.location)) continue;
    auto const ty = type_of(state, p.location);
    if (!ty.maybe(p.type)) {
      FTRACE(6, "  x B{}'s precond {} fails ({} is {})\n",
             block.id(), show(p), show(p.location), ty);
      return false;
    }
  }
  return true;
}

/*
 * PostConditions of a block are our local transfer functions.
 *
 * Changed types are overwritten, while refined types are intersected
 * with the current type.
 */
void apply_transfer_function(State& dst, const PostConditions& postConds) {
  for (auto& p : postConds.refined) {
    if (!is_tracked(p.location)) continue;
    auto& ty = type_of(dst, p.location);
    ty &= p.type;
  }
  for (auto& p : postConds.changed) {
    if (!is_tracked(p.location)) continue;
    auto& ty = type_of(dst, p.location);
    ty = p.type;
  }
}

//////////////////////////////////////////////////////////////////////

}

void region_prune_arcs(RegionDesc& region, std::vector<Type>* input) {
  FTRACE(4, "region_prune_arcs\n");

  region.sortBlocks();
  auto const sortedBlocks = region.blocks();

  // Maps region block ids to their RPO ids.
  auto blockToRPO = jit::fast_map<RegionDesc::BlockId,uint32_t>{};

  auto blockInfos = std::vector<BlockInfo>(sortedBlocks.size());
  auto workQ = dataflow_worklist<uint32_t>(sortedBlocks.size());
  for (auto rpoID = uint32_t{0}; rpoID < sortedBlocks.size(); ++rpoID) {
    auto const& b = sortedBlocks[rpoID];
    auto& binfo = blockInfos[rpoID];
    binfo.blockID = b->id();
    blockToRPO[binfo.blockID] = rpoID;
  }
  workQ.push(0);
  blockInfos[0].in = entry_state(region, input);

  FTRACE(4, "Iterating:\n");
  do {
    auto const rpoID = workQ.pop();
    auto& binfo = blockInfos[rpoID];
    FTRACE(4, "B{}\n", binfo.blockID);

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
