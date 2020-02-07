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
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_cfg);

//////////////////////////////////////////////////////////////////////

static bool is_dead(const php::Block* blk) {
  return blk->dead;
}

void remove_unreachable_blocks(const FuncAnalysis& ainfo) {
  auto done_header = false;
  auto header = [&] {
    if (done_header) return;
    done_header = true;
    FTRACE(2, "Remove unreachable blocks: {}\n", ainfo.ctx.func->name);
  };

  auto& blocks = ainfo.ctx.func->blocks;

  auto make_unreachable = [&](BlockId bid) {
    auto const blk = blocks[bid].get();
    if (is_dead(blk)) return false;
    auto const& state = ainfo.bdata[bid].stateIn;
    if (!state.initialized) return true;
    if (!state.unreachable) return false;
    return blk->hhbcs.size() != 2 ||
           blk->hhbcs.back().op != Op::Fatal;
  };

  for (auto bid : ainfo.ctx.func->blockRange()) {
    if (!make_unreachable(bid)) continue;
    header();
    FTRACE(2, "Marking {} unreachable\n", bid);
    auto const blk = blocks[bid].mutate();
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = NoBlockId;
    blk->throwExit = NoBlockId;
    blk->exnNodeId = NoExnNodeId;
  }

  if (!options.RemoveDeadBlocks) return;

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
        auto const blk = blocks[bid].mutate();
        blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
        blk->fallthrough = reachableTarget;
        break;
      }
      case Op::Switch:
      case Op::SSwitch: {
        FTRACE(2, "blk: {} -", bid);
        auto const blk = blocks[bid].mutate();
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
  NamedLocal switchLoc{};
  DataType kind;
};

bool analyzeSwitch(const php::Func& func,
                   BlockId bid,
                   std::vector<MergeBlockInfo>& blkInfos,
                   SwitchInfo* switchInfo) {
  auto const& blk = *func.blocks[bid];
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

bool buildSwitches(php::Func& func,
                   BlockId bid,
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
        auto const blk = func.blocks[bid].mutate();
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
          auto const removed = func.blocks[id].mutate();
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

bool rebuild_exn_tree(const FuncAnalysis& ainfo) {
  auto& func = *ainfo.ctx.func;
  Trace::Bump bumper{
    Trace::hhbbc_cfg, kSystemLibBump, is_systemlib_part(*func.unit)
  };
  FTRACE(4, "Rebuild exn tree: {}\n", func.name);

  auto reachable = [&](BlockId id) {
    if (is_dead(func.blocks[id].get())) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };
  hphp_fast_set<ExnNodeId> seenNodes;

  for (auto const bid : ainfo.rpoBlocks) {
    if (!reachable(bid)) {
      FTRACE(4, "Unreachable: {}\n", bid);
      continue;
    }
    auto idx = func.blocks[bid]->exnNodeId;
    while (idx != NoExnNodeId) {
      if (!seenNodes.insert(idx).second) break;
      idx = func.exnNodes[idx].parent;
    }
  }

  auto changed = false;
  for (auto& n : func.exnNodes) {
    if (n.idx == NoExnNodeId) continue;
    if (!seenNodes.count(n.idx)) {
      n.idx = NoExnNodeId;
      n.depth = 0;
      n.children.clear();
      n.parent = NoExnNodeId;
      changed = true;
    } else {
      auto it = std::remove_if(n.children.begin(), n.children.end(),
                               [&] (ExnNodeId c) {
                                 if (seenNodes.count(c)) return false;
                                 FTRACE(2, "Stripping ExnNode {}\n", c);
                                 return true;
                               });
      if (it != n.children.end()) {
        n.children.erase(it, n.children.end());
        changed = true;
      }
    }
  }

  if (!changed) return false;

  for (auto bid : func.blockRange()) {
    if (!reachable(bid)) {
      auto const blk = func.blocks[bid].mutate();
      blk->exnNodeId = NoExnNodeId;
      blk->throwExit = NoBlockId;
      continue;
    }
  }

  return true;
}

bool control_flow_opts(const FuncAnalysis& ainfo) {
  auto& func = *ainfo.ctx.func;
  FTRACE(2, "control_flow_opts: {}\n", func.name);

  std::vector<MergeBlockInfo> blockInfo(func.blocks.size(), MergeBlockInfo {});

  bool anyChanges = false;

  auto reachable = [&](BlockId id) {
    if (is_dead(func.blocks[id].get())) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };
  // find all the blocks with multiple preds; they can't be merged
  // into their predecessors
  for (auto bid : func.blockRange()) {
    auto& cblk = func.blocks[bid];
    if (is_dead(cblk.get())) continue;
    auto& bbi = blockInfo[bid];
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
    auto followSucc = false;
    forEachNormalSuccessor(
      *cblk,
      [&] (BlockId succId) {
        auto const realSucc = next_real_block(func, succId);
        if (succId != realSucc) followSucc = true;
        handleSucc(realSucc);
        numSucc++;
      }
    );
    if (followSucc) {
      anyChanges = true;
      auto const blk = cblk.mutate();
      forEachNormalSuccessor(
        *blk,
        [&] (BlockId& succId) {
          auto skip = next_real_block(func, succId);
          if (skip != succId) {
            succId = skip;
          }
        }
      );
    }
    if (cblk->throwExit != NoBlockId) handleSucc(cblk->throwExit);
    if (numSucc > 1) bbi.multipleSuccs = true;
  }
  blockInfo[func.mainEntry].multiplePreds = true;
  for (auto const blkId: func.dvEntries) {
    if (blkId != NoBlockId) {
      blockInfo[blkId].multiplePreds = true;
    }
  }

  for (auto bid : func.blockRange()) {
    auto& cblk = func.blocks[bid];
    if (is_dead(cblk.get())) continue;
    while (cblk->fallthrough != NoBlockId) {
      auto& cnxt = func.blocks[cblk->fallthrough];
      if (blockInfo[bid].multipleSuccs ||
          blockInfo[cblk->fallthrough].multiplePreds ||
          cblk->exnNodeId != cnxt->exnNodeId ||
          cblk->throwExit != cnxt->throwExit) {
        break;
      }

      FTRACE(2, "   merging: {} into {}\n", cblk->fallthrough, bid);
      auto& bInfo = blockInfo[bid];
      auto const& nInfo = blockInfo[cblk->fallthrough];
      bInfo.multipleSuccs = nInfo.multipleSuccs;
      bInfo.couldBeSwitch = nInfo.couldBeSwitch;
      bInfo.onlySwitch = false;

      auto const blk = cblk.mutate();
      blk->fallthrough = cnxt->fallthrough;
      blk->fallthroughNS = cnxt->fallthroughNS;
      std::copy(cnxt->hhbcs.begin(), cnxt->hhbcs.end(),
                std::back_inserter(blk->hhbcs));
      auto const nxt = cnxt.mutate();
      nxt->fallthrough = NoBlockId;
      nxt->dead = true;
      nxt->hhbcs = { bc::Nop {} };
      anyChanges = true;
    }
    auto const& bInfo = blockInfo[bid];
    if (bInfo.couldBeSwitch &&
        (bInfo.multiplePreds || !bInfo.onlySwitch || !bInfo.followsSwitch)) {
      // This block looks like it could be part of a switch, and it's
      // not in the middle of a sequence of such blocks.
      if (buildSwitches(func, bid, blockInfo)) {
        anyChanges = true;
      }
    }
  }

  return anyChanges;
}

//////////////////////////////////////////////////////////////////////

}}
