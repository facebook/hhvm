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

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_cfg);

//////////////////////////////////////////////////////////////////////

void remove_unreachable_blocks(const Index& index, const FuncAnalysis& ainfo) {
  auto reachable = [&](BlockId id) {
    return ainfo.bdata[id].stateIn.initialized;
  };

  for (auto& blk : ainfo.rpoBlocks) {
    if (reachable(blk->id)) continue;
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = NoBlockId;
  }

  if (!options.RemoveDeadBlocks) return;

  for (auto& blk : ainfo.rpoBlocks) {
    auto reachableTargets = false;
    forEachTakenEdge(blk->hhbcs.back(), [&] (BlockId id) {
        if (reachable(id)) reachableTargets = true;
      });
    if (reachableTargets) continue;
    switch (blk->hhbcs.back().op) {
    case Op::JmpNZ:
    case Op::JmpZ:
      blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
      break;
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

  // Block contains a sequence that could be part of a switch
  uint8_t couldBeSwitch    : 1;
  // Block contains a sequence that could be part of a switch, and nothing else
  uint8_t onlySwitch       : 1;
  // Block follows the "default" of a prior switch sequence
  uint8_t followsSwitch    : 1;
};

struct SwitchInfo {
  union Case { SString s; int64_t i; };
  std::vector<std::pair<Case,BlockId>> cases;
  BlockId defaultBlock = NoBlockId;
  LocalId switchLoc    = NoLocalId;
  DataType kind;
};

bool analyzeSwitch(const php::Block& blk,
                   std::vector<MergeBlockInfo>& blkInfos,
                   SwitchInfo* switchInfo) {
  auto const jmp = &blk.hhbcs.back();
  auto& blkInfo = blkInfos[blk.id];

  switch (jmp->op) {
    case Op::JmpZ:
    case Op::JmpNZ: {
      if (blk.hhbcs.size() < 4) return false;
      auto const& cmp = jmp[-1];
      if (cmp.op != Op::Eq && cmp.op != Op::Neq) return false;
      auto check = [&] (const Bytecode& arg1, const Bytecode& arg2) -> bool {
        LocalId loc;
        if (arg2.op == Op::CGetL) {
          loc = arg2.CGetL.loc1;
        } else if (arg2.op == Op::CGetL2 && &arg2 == &arg1 + 1) {
          loc = arg2.CGetL2.loc1;
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
          jmp->JmpNZ.target : jmp->JmpZ.target;
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
      if (cgetl.op != Op::CGetL) return false;
      auto const dt = jmp->op == Op::Switch ? KindOfInt64 : KindOfString;
      if (switchInfo) {
        if (switchInfo->cases.size()) {
          if (cgetl.CGetL.loc1 != switchInfo->switchLoc) return false;
          if (dt != switchInfo->kind) return false;
        } else {
          switchInfo->switchLoc = cgetl.CGetL.loc1;
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
  std::set<SString> seen;
  SSwitchTab sswitchTab;
  for (auto& c : switchInfo.cases) {
    if (seen.insert(c.first.s).second) {
      sswitchTab.emplace_back(c.first.s, c.second);
    }
  }
  sswitchTab.emplace_back(nullptr, switchInfo.defaultBlock);
  return { bc::SSwitch { std::move(sswitchTab) } };
}

bool buildSwitches(php::Func& func,
                   borrowed_ptr<php::Block> blk,
                   std::vector<MergeBlockInfo>& blkInfos) {
  SwitchInfo switchInfo;
  std::vector<BlockId> blocks;
  if (!analyzeSwitch(*blk, blkInfos, &switchInfo)) return false;
  blkInfos[blk->id].couldBeSwitch = false;
  blkInfos[blk->id].onlySwitch = false;
  while (true) {
    auto const& bInfo = blkInfos[switchInfo.defaultBlock];
    auto const nxt = borrow(func.blocks[switchInfo.defaultBlock]);
    if (bInfo.onlySwitch && !bInfo.multiplePreds &&
        analyzeSwitch(*nxt, blkInfos, &switchInfo)) {
      blocks.push_back(switchInfo.defaultBlock);
      continue;
    }
    bool ret = false;
    auto const minSize = switchInfo.kind == KindOfInt64 ? 1 : 8;
    if (switchInfo.cases.size() >= minSize && blocks.size()) {
      while (is_single_nop(*func.blocks[switchInfo.defaultBlock])) {
        switchInfo.defaultBlock =
          func.blocks[switchInfo.defaultBlock]->fallthrough;
      }
      auto bc = switchInfo.kind == KindOfInt64 ?
        buildIntSwitch(switchInfo) : buildStringSwitch(switchInfo);
      if (bc.op != Op::Nop) {
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
        blkInfos[switchInfo.defaultBlock].multiplePreds = true;
        blk->hhbcs.erase(it, blk->hhbcs.end());
        blk->hhbcs.emplace_back(bc::CGetL { switchInfo.switchLoc });
        blk->hhbcs.push_back(std::move(bc));
        blk->fallthrough = NoBlockId;
        for (auto id : blocks) {
          if (blkInfos[id].multiplePreds) continue;
          auto const removed = borrow(func.blocks[id]);
          removed->id = NoBlockId;
          removed->hhbcs = { bc::Nop {} };
          removed->fallthrough = NoBlockId;
          removed->factoredExits = {};
        }
        ret = true;
      }
    }
    return (bInfo.couldBeSwitch && buildSwitches(func, nxt, blkInfos)) || ret;
  }
}

}

bool control_flow_opts(const FuncAnalysis& ainfo) {
  auto& func = *ainfo.ctx.func;
  FTRACE(2, "control_flow_opts: {}\n", func.name);

  std::vector<MergeBlockInfo> blockInfo(func.blocks.size(), MergeBlockInfo {});

  bool removedAny = false;

  auto reachable = [&](BlockId id) {
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };
  // find all the blocks with multiple preds; they can't be merged
  // into their predecessors
  for (auto const& blk : func.blocks) {
    if (blk->id == NoBlockId) continue;
    auto& bbi = blockInfo[blk->id];
    int numSucc = 0;
    if (!reachable(blk->id)) {
      bbi.multiplePreds = true;
      bbi.multipleSuccs = true;
      continue;
    } else {
      analyzeSwitch(*blk, blockInfo, nullptr);
    }
    auto handleSucc = [&] (BlockId succId) {
      auto& bsi = blockInfo[succId];
      if (bsi.hasPred) {
        bsi.multiplePreds = true;
      } else {
        bsi.hasPred = true;
      }
      numSucc++;
    };
    forEachNormalSuccessor(*blk, [&](const BlockId& succId) {
        auto succ = borrow(func.blocks[succId]);
        while (succ->id != NoBlockId &&
               is_single_nop(*succ) &&
               succId < succ->fallthrough) {
          always_assert(succ->fallthrough != NoBlockId);
          const_cast<BlockId&>(succId) = succ->fallthrough;
          succ = borrow(func.blocks[succId]);
          removedAny = true;
        }
        handleSucc(succId);
      });
    for (auto& ex : blk->factoredExits) handleSucc(ex);
    if (numSucc > 1) bbi.multipleSuccs = true;
  }
  blockInfo[func.mainEntry].multiplePreds = true;
  for (auto const blkId: func.dvEntries) {
    if (blkId != NoBlockId) {
      blockInfo[blkId].multiplePreds = true;
    }
  }

  for (auto& blk : func.blocks) {
    if (blk->id == NoBlockId) continue;
    while (blk->fallthrough != NoBlockId) {
      auto nxt = borrow(func.blocks[blk->fallthrough]);
      if (blockInfo[blk->id].multipleSuccs ||
          blockInfo[nxt->id].multiplePreds ||
          blk->exnNode != nxt->exnNode ||
          blk->section != nxt->section) {
        break;
      }

      FTRACE(1, "merging: {} into {}\n", (void*)nxt, (void*)blk.get());
      auto& bInfo = blockInfo[blk->id];
      auto const& nInfo = blockInfo[nxt->id];
      bInfo.multipleSuccs = nInfo.multipleSuccs;
      bInfo.couldBeSwitch = nInfo.couldBeSwitch;
      bInfo.onlySwitch = false;

      blk->fallthrough = nxt->fallthrough;
      blk->fallthroughNS = nxt->fallthroughNS;
      if (nxt->factoredExits.size()) {
        if (blk->factoredExits.size()) {
          std::set<BlockId> exitSet;
          std::copy(begin(blk->factoredExits), end(blk->factoredExits),
                    std::inserter(exitSet, begin(exitSet)));
          std::copy(nxt->factoredExits.begin(), nxt->factoredExits.end(),
                    std::inserter(exitSet, begin(exitSet)));
          blk->factoredExits.resize(exitSet.size());
          std::copy(begin(exitSet), end(exitSet), blk->factoredExits.begin());
          nxt->factoredExits = decltype(nxt->factoredExits) {};
        } else {
          blk->factoredExits = std::move(nxt->factoredExits);
        }
      }
      std::copy(nxt->hhbcs.begin(), nxt->hhbcs.end(),
                std::back_inserter(blk->hhbcs));
      nxt->fallthrough = NoBlockId;
      nxt->id = NoBlockId;
      nxt->hhbcs = { bc::Nop {} };
      removedAny = true;
    }
    auto const& bInfo = blockInfo[blk->id];
    if (bInfo.couldBeSwitch &&
        (bInfo.multiplePreds || !bInfo.onlySwitch || !bInfo.followsSwitch)) {
      // This block looks like it could be part of a switch, and it's
      // not in the middle of a sequence of such blocks.
      if (buildSwitches(func, borrow(blk), blockInfo)) {
        removedAny = true;
      }
    }
  }

  return removedAny;
}

//////////////////////////////////////////////////////////////////////

}}
