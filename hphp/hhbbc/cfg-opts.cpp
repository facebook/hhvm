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
#include "hphp/hhbbc/cfg-opts.h"

#include <vector>
#include <string>

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/dce.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/wide-func.h"

namespace HPHP::HHBBC {

TRACE_SET_MOD(hhbbc_cfg);

//////////////////////////////////////////////////////////////////////

static bool is_dead(const php::Block* blk) {
  return blk->dead;
}

namespace {

/*
 * Remove exception handlers that has no effect, either because they just
 * immediately rethrow the received exception, or they only unset locals
 * before rethrowing at the top level, which is redundant with the unwinder.
 */
void remove_unnecessary_exception_handlers(const FuncAnalysis& ainfo,
                                           php::WideFunc& func) {
  auto& blocks = func.blocks();
  auto remap = std::vector<Optional<std::tuple<BlockId, ExnNodeId>>>(
    blocks.size(), std::nullopt);

  // Visit exception handlers before the blocks using them.
  for (auto it = ainfo.rpoBlocks.rbegin(); it != ainfo.rpoBlocks.rend(); ++it) {
    auto& blk = blocks[*it];

    // Apply the remap if this block handles exceptions with a remapped
    // exception handler.
    if (blk->throwExit != NoBlockId) {
      if (auto const r = remap[blk->throwExit]) {
        auto const mblk = blk.mutate();
        std::tie(mblk->throwExit, mblk->exnNodeId) = *r;
      }
    }

    // If this block looks like an exception handler that can be optimized away,
    // add it to the remap.
    if (is_single_throw(*blk) ||
        (blk->throwExit == NoBlockId && is_unsetl_throw(*blk))) {
      remap[*it] = std::make_tuple(blk->throwExit, blk->exnNodeId);
    }
  }
}

}

void remove_unreachable_blocks(const FuncAnalysis& ainfo, php::WideFunc& func) {
  auto done_header = false;
  auto header = [&] {
    if (done_header) return;
    done_header = true;
    FTRACE(2, "Remove unreachable blocks: {}\n", func->name);
  };

  auto& blocks = func.blocks();

  auto make_unreachable = [&](BlockId bid) {
    auto const blk = blocks[bid].get();
    if (is_dead(blk)) return false;
    auto const& state = ainfo.bdata[bid].stateIn;
    if (!state.initialized) return true;
    if (!state.unreachable) return false;
    return blk->hhbcs.size() != 2 ||
           blk->hhbcs.back().op != Op::Fatal;
  };

  for (auto bid : func.blockRange()) {
    if (!make_unreachable(bid)) continue;
    header();
    FTRACE(2, "Marking {} unreachable\n", bid);
    auto const blk = func.blocks()[bid].mutate();
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = NoBlockId;
    blk->throwExit = NoBlockId;
    blk->exnNodeId = NoExnNodeId;
  }

  remove_unnecessary_exception_handlers(ainfo, func);

  auto reachable = [&](BlockId id) {
    if (id == NoBlockId) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };

  for (auto const bid : ainfo.rpoBlocks) {
    if (!reachable(bid)) continue;
    auto reachableTarget = NoBlockId;
    auto hasUnreachableTargets = false;
    forEachNormalSuccessor(
      *blocks[bid],
      [&] (BlockId id) {
        if (reachable(id)) {
          reachableTarget = id;
        } else {
          hasUnreachableTargets = true;
        }
      }
    );

    if (!hasUnreachableTargets || reachableTarget == NoBlockId) continue;
    header();
    switch (blocks[bid]->hhbcs.back().op) {
      case Op::JmpNZ:
      case Op::JmpZ: {
        FTRACE(2, "blk: {} - jcc -> jmp {}\n", bid, reachableTarget);
        auto const blk = func.blocks()[bid].mutate();
        blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
        blk->fallthrough = reachableTarget;
        break;
      }
      case Op::Switch:
      case Op::SSwitch: {
        FTRACE(2, "blk: {} -", bid);
        auto const blk = func.blocks()[bid].mutate();
        forEachNormalSuccessor(
          *blk,
          [&] (BlockId& id) {
            if (!reachable(id)) {
              FTRACE(2, " {}->{}", id, reachableTarget);
              id = reachableTarget;
            }
          }
        );
        FTRACE(2, "\n");
        break;
      }
      default:
        break;
    }
  }
}

namespace {

struct MergeBlockInfo {
  // This block has a predecessor; used to set the multiplePreds flag
  uint8_t hasPred          : 1;
  // Block has more than one pred, or is an entry block
  uint8_t multiplePreds    : 1;
  // Block has more than one successor
  uint8_t multipleSuccs    : 1;
  // Block has simple Nop successor.
  uint8_t followSucc       : 1;

  // Block contains a sequence that could be part of a switch
  uint8_t couldBeSwitch    : 1;
  // Block contains a sequence that could be part of a switch, and nothing else
  uint8_t onlySwitch       : 1;
  // Block follows the "default" of a prior switch sequence
  uint8_t followsSwitch    : 1;

  // Block will not throw
  uint8_t noThrow          : 1;
};

struct SwitchInfo {
  union Case { SString s; int64_t i; };
  std::vector<std::pair<Case,BlockId>> cases;
  BlockId defaultBlock = NoBlockId;
  NamedLocal switchLoc{};
  DataType kind;
};

bool analyzeSwitch(const php::WideFunc& func, BlockId bid,
                   std::vector<MergeBlockInfo>& blkInfos,
                   SwitchInfo* switchInfo) {
  auto const& blk = *func.blocks()[bid];
  auto const jmp = &blk.hhbcs.back();
  auto& blkInfo = blkInfos[bid];

  switch (jmp->op) {
    case Op::JmpZ:
    case Op::JmpNZ: {
      if (blk.hhbcs.size() < 4) return false;
      auto const& cmp = jmp[-1];
      if (cmp.op != Op::Eq && cmp.op != Op::Neq) return false;
      auto check = [&] (const Bytecode& arg1, const Bytecode& arg2) -> bool {
        NamedLocal loc;
        if (arg2.op == Op::CGetL) {
          loc = arg2.CGetL.nloc1;
        } else if (arg2.op == Op::CGetQuietL) {
          loc = NamedLocal{kInvalidLocalName, arg2.CGetQuietL.loc1};
        } else if (arg2.op == Op::CGetL2 && &arg2 == &arg1 + 1) {
          loc = arg2.CGetL2.nloc1;
        } else {
          return false;
        }
        SwitchInfo::Case c;
        if (arg1.op == Op::Int) {
          c.i = arg1.Int.arg1;
        } else if (arg1.op == Op::String) {
          c.s = arg1.String.str1;
        } else {
          return false;
        }
        if (switchInfo) {
          auto const dt = arg1.op == Op::Int ? KindOfInt64 : KindOfString;
          if (switchInfo->cases.size()) {
            if (loc != switchInfo->switchLoc) return false;
            if (dt != switchInfo->kind) return false;
          } else {
            switchInfo->switchLoc = loc;
            switchInfo->kind = dt;
          }
        }
        auto const jmpTarget = jmp->op == Op::JmpNZ ?
          jmp->JmpNZ.target1 : jmp->JmpZ.target1;
        BlockId caseTarget, defaultBlock;
        if ((jmp->op == Op::JmpNZ) == (cmp.op == Op::Eq)) {
          defaultBlock = blk.fallthrough;
          caseTarget = jmpTarget;
        } else {
          defaultBlock = jmpTarget;
          caseTarget = blk.fallthrough;
        }
        blkInfo.couldBeSwitch = true;
        blkInfo.onlySwitch = blk.hhbcs.size() == 4;
        blkInfos[defaultBlock].followsSwitch = true;
        if (switchInfo) {
          switchInfo->cases.emplace_back(c, caseTarget);
          switchInfo->defaultBlock = defaultBlock;
        }
        return true;
      };
      return check(jmp[-2], jmp[-3]) || check(jmp[-3], jmp[-2]);
    }
    case Op::Switch:
    case Op::SSwitch: {
      if (blk.hhbcs.size() < 2) return false;
      auto const& cgetl = jmp[-1];
      if (cgetl.op != Op::CGetL && cgetl.op != Op::CGetQuietL) return false;
      auto const loc = cgetl.op == Op::CGetQuietL
                       ? NamedLocal{kInvalidLocalName, cgetl.CGetQuietL.loc1}
                       : cgetl.CGetL.nloc1;
      auto const dt = jmp->op == Op::Switch ? KindOfInt64 : KindOfString;
      if (switchInfo) {
        if (switchInfo->cases.size()) {
          if (loc != switchInfo->switchLoc) return false;
          if (dt != switchInfo->kind) return false;
        } else {
          switchInfo->switchLoc = loc;
          switchInfo->kind = dt;
        }
      }
      if (jmp->op == Op::Switch) {
        if (jmp->Switch.subop1 != SwitchKind::Bounded) return false;
        auto const db = jmp->Switch.targets.back();
        auto const min = jmp->Switch.arg2;
        blkInfos[db].followsSwitch = true;
        if (switchInfo) {
          switchInfo->defaultBlock = db;
          for (size_t i = 0; i < jmp->Switch.targets.size() - 2; i++) {
            auto const t = jmp->Switch.targets[i];
            if (t == db) continue;
            SwitchInfo::Case c;
            c.i = i + min;
            switchInfo->cases.emplace_back(c, t);
          }
        }
      } else {
        auto const db = jmp->SSwitch.targets.back().second;
        blkInfos[db].followsSwitch = true;
        if (switchInfo) {
          switchInfo->defaultBlock = db;
          for (auto& kv : jmp->SSwitch.targets) {
            if (kv.second == db) continue;
            SwitchInfo::Case c;
            c.s = kv.first;
            switchInfo->cases.emplace_back(c, kv.second);
          }
        }
      }
      blkInfo.couldBeSwitch = true;
      blkInfo.onlySwitch = blk.hhbcs.size() == 2;
      return true;
    }
    default:
      return false;
  }
}

Bytecode buildIntSwitch(SwitchInfo& switchInfo) {
  auto min = switchInfo.cases[0].first.i;
  auto max = min;
  for (size_t i = 1; i < switchInfo.cases.size(); ++i) {
    auto v = switchInfo.cases[i].first.i;
    if (v < min) min = v;
    if (v > max) max = v;
  }
  if (switchInfo.cases.size() / ((double)max - (double)min + 1) < .5) {
    return { bc::Nop {} };
  }
  CompactVector<BlockId> switchTab;
  switchTab.resize(max - min + 3, switchInfo.defaultBlock);
  for (auto i = switchInfo.cases.size(); i--; ) {
    auto const& c = switchInfo.cases[i];
    switchTab[c.first.i - min] = c.second;
    if (c.first.i) switchTab[max - min + 1] = c.second;
  }
  return { bc::Switch { SwitchKind::Bounded, min, std::move(switchTab) } };
}

Bytecode buildStringSwitch(SwitchInfo& switchInfo) {
  hphp_fast_set<SString> seen;
  SSwitchTab sswitchTab;
  for (auto& c : switchInfo.cases) {
    if (seen.insert(c.first.s).second) {
      sswitchTab.emplace_back(c.first.s, c.second);
    }
  }
  sswitchTab.emplace_back(nullptr, switchInfo.defaultBlock);
  return { bc::SSwitch { std::move(sswitchTab) } };
}

bool buildSwitches(php::WideFunc& func, BlockId bid,
                   std::vector<MergeBlockInfo>& blkInfos) {
  SwitchInfo switchInfo;
  std::vector<BlockId> blocks;
  if (!analyzeSwitch(func, bid, blkInfos, &switchInfo)) return false;
  blkInfos[bid].couldBeSwitch = false;
  blkInfos[bid].onlySwitch = false;
  while (true) {
    auto const& bInfo = blkInfos[switchInfo.defaultBlock];
    auto const nxtId = switchInfo.defaultBlock;
    if (bInfo.onlySwitch && !bInfo.multiplePreds &&
        analyzeSwitch(func, nxtId, blkInfos, &switchInfo)) {
      blocks.push_back(switchInfo.defaultBlock);
      continue;
    }
    bool ret = false;
    auto const minSize = switchInfo.kind == KindOfInt64 ? 1 : 8;
    if (switchInfo.cases.size() >= minSize && blocks.size()) {
      switchInfo.defaultBlock = next_real_block(func, switchInfo.defaultBlock);
      auto bc = switchInfo.kind == KindOfInt64 ?
        buildIntSwitch(switchInfo) : buildStringSwitch(switchInfo);
      if (bc.op != Op::Nop) {
        auto const blk = func.blocks()[bid].mutate();
        auto it = blk->hhbcs.end();
        // blk->fallthrough implies it was a JmpZ JmpNZ block,
        // which means we have exactly 4 instructions making up
        // the switch (see analyzeSwitch). Otherwise it was a
        // [S]Switch, and there were exactly two instructions.
        if (blk->fallthrough != NoBlockId) {
          it -= 4;
        } else {
          it -= 2;
        }
        bc.srcLoc = it->srcLoc;
        blkInfos[switchInfo.defaultBlock].multiplePreds = true;
        blk->hhbcs.erase(it, blk->hhbcs.end());
        blk->hhbcs.emplace_back(bc::CGetL { switchInfo.switchLoc });
        blk->hhbcs.back().srcLoc = bc.srcLoc;
        blk->hhbcs.push_back(std::move(bc));
        blk->fallthrough = NoBlockId;
        for (auto id : blocks) {
          if (blkInfos[id].multiplePreds) continue;
          auto const removed = func.blocks()[id].mutate();
          removed->dead = true;
          removed->hhbcs = { bc::Nop {} };
          removed->fallthrough = NoBlockId;
          removed->throwExit = NoBlockId;
          removed->exnNodeId = NoExnNodeId;
        }
        ret = true;
      }
    }
    if (bInfo.couldBeSwitch && buildSwitches(func, nxtId, blkInfos)) {
      ret = true;
    }
    return ret;
  }
}

}

bool control_flow_opts(const FuncAnalysis& ainfo, php::WideFunc& func) {
  FTRACE(2, "control_flow_opts: {}\n", func->name);

  std::vector<MergeBlockInfo> blockInfo(
      func.blocks().size(), MergeBlockInfo {});

  bool anyChanges = false;

  auto reachable = [&](BlockId id) {
    if (is_dead(func.blocks()[id].get())) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };

  // find all the blocks with multiple preds; they can't be merged
  // into their predecessors
  for (auto bid : func.blockRange()) {
    auto& cblk = func.blocks()[bid];
    if (is_dead(cblk.get())) continue;
    auto& bbi = blockInfo[bid];
    bbi.noThrow = ainfo.bdata[bid].noThrow;
    int numSucc = 0;
    if (!reachable(bid)) {
      bbi.multiplePreds = true;
      bbi.multipleSuccs = true;
      continue;
    } else {
      analyzeSwitch(func, bid, blockInfo, nullptr);
    }
    auto handleSucc = [&] (BlockId succId) {
      auto& bsi = blockInfo[succId];
      if (bsi.hasPred) {
        bsi.multiplePreds = true;
      } else {
        bsi.hasPred = true;
      }
    };
    forEachNormalSuccessor(
      *cblk,
      [&] (BlockId succId) {
        if (cblk->hhbcs.back().op == Op::Enter) {
          // Enter must always point to the main func entry.
          handleSucc(succId);
        } else {
          auto const realSucc = next_real_block(func, succId);
          if (succId != realSucc) bbi.followSucc = true;
          handleSucc(realSucc);
        }
        numSucc++;
      }
    );
    // Attempt to jump thread the catch blocks again, in the hope of
    // having two adjacent blocks end up with the same catch block.
    auto const nextCatch =
      next_catch_block(func, cblk->throwExit, cblk->exnNodeId);
    if (nextCatch.first != cblk->throwExit ||
        nextCatch.second != cblk->exnNodeId) {
      anyChanges = true;
      auto const blk = cblk.mutate();
      blk->throwExit = nextCatch.first;
      blk->exnNodeId = nextCatch.second;
    }
    if (cblk->throwExit != NoBlockId) handleSucc(cblk->throwExit);
    if (numSucc > 1) bbi.multipleSuccs = true;
  }
  blockInfo[func->mainEntry].multiplePreds = true;
  for (auto const blkId: func->dvEntries) {
    if (blkId != NoBlockId) {
      blockInfo[blkId].multiplePreds = true;
    }
  }
  for (auto bid : func.blockRange()) {
    if (blockInfo[bid].followSucc) {
      auto const blk = func.blocks()[bid].mutate();
      assertx(blk->hhbcs.back().op != Op::Enter);
      forEachNormalSuccessor(
        *blk,
        [&] (BlockId& succId) {
          auto skip = next_real_block(func, succId);
          auto const criticalEdge = blockInfo[bid].multipleSuccs
                                    && blockInfo[skip].multiplePreds;
          if (skip != succId) {
            if (!criticalEdge) anyChanges = true;
            succId = skip;
          }
        }
      );
    }
  }

  for (auto bid : func.blockRange()) {
    auto& cblk = func.blocks()[bid];
    if (is_dead(cblk.get())) continue;

    while (cblk->fallthrough != NoBlockId) {
      auto const nid = cblk->fallthrough;
      auto& cnxt = func.blocks()[nid];
      if (blockInfo[bid].multipleSuccs || blockInfo[nid].multiplePreds) break;

      // If the two blocks have different catch blocks, we might still
      // be able to combine them. If one (or both) of the blocks are
      // no throw, it doesn't matter what the catch block is, because
      // its unreachable.
      auto useNextCatch = false;
      if (cblk->exnNodeId != cnxt->exnNodeId ||
          cblk->throwExit != cnxt->throwExit) {
        auto const noThrow = blockInfo[bid].noThrow;
        auto const nextNoThrow = blockInfo[cblk->fallthrough].noThrow;
        if (!nextNoThrow && !noThrow) break;

        // Don't merge blocks containing these instructions because it
        // can produce bytecode which will fail the verifier.
        auto const unsafe = [] (const Bytecode& bc) {
          switch (bc.op) {
            case Op::IterFree:
            case Op::LIterFree:
            case Op::Silence:
              return true;
            default:
              return false;
          }
        };
        if (std::any_of(begin(cblk->hhbcs), end(cblk->hhbcs), unsafe)) break;
        if (std::any_of(begin(cnxt->hhbcs), end(cnxt->hhbcs), unsafe)) break;

        useNextCatch = noThrow;
      }

      FTRACE(2, "   merging: {} into {}\n", nid, bid);
      auto& bInfo = blockInfo[bid];
      auto const& nInfo = blockInfo[nid];
      bInfo.multipleSuccs = nInfo.multipleSuccs;
      bInfo.couldBeSwitch = nInfo.couldBeSwitch;
      bInfo.onlySwitch = false;
      bInfo.noThrow &= nInfo.noThrow;

      auto const blk = func.blocks()[bid].mutate();
      blk->fallthrough = cnxt->fallthrough;
      if (useNextCatch) {
        blk->throwExit = cnxt->throwExit;
        blk->exnNodeId = cnxt->exnNodeId;
      }
      std::copy(cnxt->hhbcs.begin(), cnxt->hhbcs.end(),
                std::back_inserter(blk->hhbcs));
      auto const nxt = func.blocks()[nid].mutate();
      nxt->fallthrough = NoBlockId;
      nxt->throwExit = NoBlockId;
      nxt->exnNodeId = NoExnNodeId;
      nxt->dead = true;
      nxt->hhbcs = { bc::Nop {} };
      anyChanges = true;
    }

    auto& bInfo = blockInfo[bid];

    if (bInfo.couldBeSwitch &&
        (bInfo.multiplePreds || !bInfo.onlySwitch || !bInfo.followsSwitch)) {
      // This block looks like it could be part of a switch, and it's
      // not in the middle of a sequence of such blocks.
      if (buildSwitches(func, bid, blockInfo)) {
        anyChanges = true;
      }
    }

    // Turn a conditional jump to the same block into an unconditional
    // jump. This can make the source of the conditional jump dead.
    auto const jmpTarget = [&] () -> Optional<BlockId> {
      auto const& lastOp = cblk->hhbcs.back();
      if (lastOp.op == Op::JmpZ)  return lastOp.JmpZ.target1;
      if (lastOp.op == Op::JmpNZ) return lastOp.JmpNZ.target1;
      return std::nullopt;
    }();

    if (jmpTarget && cblk->fallthrough == *jmpTarget) {
      FTRACE(2, "   removing redundant conditional jmp in {}\n", bid);
      auto const blk = func.blocks()[bid].mutate();
      blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
      bInfo.multipleSuccs = false;
      anyChanges = true;
    }
  }

  return anyChanges;
}

void split_critical_edges(const Index& index, FuncAnalysis& ainfo,
                          php::WideFunc& func) {
  // Changed tracks if we need to recompute RPO.
  auto changed = false;
  assertx(func.blocks().size() == ainfo.bdata.size());

  // Makes an empty block and inserts it between src and dst.  Since we can't
  // have empty blocks it actually stores a Nop in it to keep everyone happy.
  // The block will have the same exception node id, and throw exit block as
  // the src block.
  //
  // ainfo is not updated with the correct rpo info by this helper, but is
  // updated with the correct state info.
  auto const split_edge = [&](BlockId srcBid, php::Block* srcBlk,
                              BlockId dstBid) {
    assertx(srcBlk->hhbcs.back().op != Op::Enter);
    auto const srcLoc = srcBlk->hhbcs.back().srcLoc;
    auto const newBid = make_block(func, srcBlk);
    auto const newBlk = func.blocks()[newBid].mutate();
    newBlk->hhbcs = { bc_with_loc(srcLoc, bc::Nop{}) };
    newBlk->fallthrough = dstBid;

    UNUSED bool replacedDstTarget = false;
    forEachNormalSuccessor(*srcBlk, [&] (BlockId& bid) {
      if (bid == dstBid) {
        bid = newBid;
        replacedDstTarget = true;
      }
    });
    assertx(replacedDstTarget);

    IndexAdaptor adaptor{index};
    auto collect = CollectedInfo {
      adaptor, ainfo.ctx, nullptr,
      CollectionOpts{}, nullptr, &ainfo
    };
    auto const ctx = AnalysisContext { ainfo.ctx.unit, func, ainfo.ctx.cls };
    ainfo.bdata.push_back({
      0, // We renumber the blocks at the end of the function.
      locally_propagated_bid_state(index, ctx, collect, srcBid,
                                   ainfo.bdata[srcBid].stateIn,
                                   newBid)
    });
    assertx(func.blocks().size() == ainfo.bdata.size());

    changed = true;
  };

  boost::dynamic_bitset<> haveSeenPred(func.blocks().size());
  boost::dynamic_bitset<> hasMultiplePreds(func.blocks().size());
  boost::dynamic_bitset<> hasMultipleSuccs(func.blocks().size());
  // Switches can have the same successor for multiple cases, and we only want
  // to split the edge once.
  boost::dynamic_bitset<> seenSuccs(func.blocks().size());
  for (auto bid : func.blockRange()) {
    auto& blk = func.blocks()[bid];

    int succCount = 0;
    seenSuccs.reset();
    forEachNormalSuccessor(*blk, [&] (BlockId succBid) {
      if (seenSuccs[succBid]) return;
      seenSuccs[succBid] = true;
      if (haveSeenPred[succBid]) {
        hasMultiplePreds[succBid] = true;
      } else {
        haveSeenPred[succBid] = true;
      }
      succCount++;
    });

    hasMultipleSuccs[bid] = succCount > 1;
  }

  for (auto bid : func.blockRange()) {
    if (!hasMultipleSuccs[bid]) continue;
    auto blk = func.blocks()[bid];
    seenSuccs.reset();
    forEachNormalSuccessor(*blk, [&] (BlockId succBid) {
      if (hasMultiplePreds[succBid] && !seenSuccs[succBid]) {
        seenSuccs[succBid] = true;
        split_edge(bid, func.blocks()[bid].mutate(), succBid);
      }
    });
  }

  if (changed) {
    // Recompute the rpo ids.
    assertx(func.blocks().size() == ainfo.bdata.size());
    ainfo.rpoBlocks = rpoSortAddDVs(func);
    assertx(ainfo.rpoBlocks.size() <= ainfo.bdata.size());
    for (size_t rpoId = 0; rpoId < ainfo.rpoBlocks.size(); ++rpoId) {
      ainfo.bdata[ainfo.rpoBlocks[rpoId]].rpoId = rpoId;
    }
  }
}

//////////////////////////////////////////////////////////////////////

}
