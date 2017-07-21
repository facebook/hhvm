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
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_cfg);

//////////////////////////////////////////////////////////////////////

void remove_unreachable_blocks(const FuncAnalysis& ainfo) {
  auto done_header = false;
  auto header = [&] {
    if (done_header) return;
    done_header = true;
    FTRACE(2, "Remove unreachable blocks: {}\n", ainfo.ctx.func->name);
  };

  auto make_unreachable = [&](borrowed_ptr<php::Block> blk) {
    if (blk->id == NoBlockId) return false;
    auto const& state = ainfo.bdata[blk->id].stateIn;
    if (!state.initialized) return true;
    if (!state.unreachable) return false;
    return blk->hhbcs.size() != 2 ||
           blk->hhbcs.back().op != Op::Fatal;
  };

  for (auto const& blk : ainfo.ctx.func->blocks) {
    if (!make_unreachable(borrow(blk))) continue;
    header();
    FTRACE(2, "Marking {} unreachable\n", blk->id);
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = NoBlockId;
    blk->exnNode = nullptr;
  }

  if (!options.RemoveDeadBlocks) return;

  auto reachable = [&](BlockId id) {
    if (id == NoBlockId) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };

  for (auto const& blk : ainfo.rpoBlocks) {
    if (!reachable(blk->id)) continue;
    auto reachableTarget = NoBlockId;
    auto hasUnreachableTargets = false;
    forEachNormalSuccessor(*blk, [&] (BlockId id) {
        if (reachable(id)) {
          reachableTarget = id;
        } else {
          hasUnreachableTargets = true;
        }
      });
    if (!hasUnreachableTargets || reachableTarget == NoBlockId) continue;
    header();
    switch (blk->hhbcs.back().op) {
      case Op::JmpNZ:
      case Op::JmpZ:
        FTRACE(2, "blk: {} - jcc -> jmp {}\n", blk->id, reachableTarget);
        blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
        blk->fallthrough = reachableTarget;
        break;
      default:
        FTRACE(2, "blk: {} -", blk->id, reachableTarget);
        forEachNormalSuccessor(*blk, [&] (const BlockId& id) {
            if (!reachable(id)) {
              FTRACE(2, " {}->{}", id, reachableTarget);
              const_cast<BlockId&>(id) = reachableTarget;
            }
          });
        FTRACE(2, "\n");
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
      switchInfo.defaultBlock = next_real_block(func, switchInfo.defaultBlock);
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
        bc.srcLoc = it->srcLoc;
        blkInfos[switchInfo.defaultBlock].multiplePreds = true;
        blk->hhbcs.erase(it, blk->hhbcs.end());
        blk->hhbcs.emplace_back(bc::CGetL { switchInfo.switchLoc });
        blk->hhbcs.back().srcLoc = bc.srcLoc;
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

template<typename S>
bool strip_exn_tree(const php::Func& func,
                    CompactVector<std::unique_ptr<php::ExnNode>>& nodes,
                    const std::set<borrowed_ptr<php::ExnNode>> &seenExnNodes,
                    uint32_t& nextId,
                    S& sectionExits) {
  auto it = std::remove_if(nodes.begin(), nodes.end(),
                           [&] (const std::unique_ptr<php::ExnNode>& node) {
                             if (seenExnNodes.count(borrow(node))) return false;
                             FTRACE(2, "Stripping ExnNode {}\n", node->id);
                             return true;
                           });
  auto ret = false;
  if (it != nodes.end()) {
    nodes.erase(it, nodes.end());
    ret = true;
  }
  for (auto& n : nodes) {
    n->id = nextId++;
    if (n->parent) {
      match<void>(
        n->info, [&](const php::CatchRegion& /*cr*/) {},
        [&](const php::FaultRegion& fr) {
          auto pentry = match<BlockId>(
            n->parent->info,
            [&] (const php::CatchRegion& cr2) { return cr2.catchEntry; },
            [&] (const php::FaultRegion& fr2) { return fr2.faultEntry; }
          );
          auto const sectionId =
            static_cast<size_t>(func.blocks[fr.faultEntry]->section);
          sectionExits[sectionId].insert(pentry);
        });
    }
    if (strip_exn_tree(func, n->children, seenExnNodes, nextId, sectionExits)) {
      ret = true;
    }
  }

  return ret;
}

}

bool rebuild_exn_tree(const FuncAnalysis& ainfo) {
  auto& func = *ainfo.ctx.func;
  Trace::Bump bumper{
    Trace::hhbbc_cfg, kSystemLibBump, is_systemlib_part(*func.unit)
  };
  FTRACE(4, "Rebuild exn tree: {}\n", func.name);

  auto reachable = [&](BlockId id) {
    if (id == NoBlockId) return false;
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };
  std::unordered_map<BlockId,std::set<BlockId>> factoredExits;
  std::unordered_map<size_t, std::set<BlockId>> sectionExits;
  std::set<borrowed_ptr<php::ExnNode>> seenExnNodes;

  for (auto const& blk : ainfo.rpoBlocks) {
    if (!reachable(blk->id)) {
      FTRACE(4, "Unreachable: {}\n", blk->id);
      continue;
    }
    if (auto node = blk->exnNode) {
      auto entry = match<BlockId>(
        node->info,
        [&] (const php::CatchRegion& cr) { return cr.catchEntry; },
        [&] (const php::FaultRegion& fr) { return fr.faultEntry; }
      );
      factoredExits[blk->id].insert(entry);
      do {
        if (!seenExnNodes.insert(node).second) break;
      } while ((node = node->parent) != nullptr);
    }
  }

  uint32_t nextId = 0;
  if (!strip_exn_tree(func, func.exnNodes,
                      seenExnNodes, nextId, sectionExits)) {
    return false;
  }

  for (auto const& blk : func.blocks) {
    if (!reachable(blk->id)) {
      blk->exnNode = nullptr;
      continue;
    }
    auto &fe = factoredExits[blk->id];
    auto it = sectionExits.find(static_cast<size_t>(blk->section));
    if (it != sectionExits.end()) {
      fe.insert(it->second.begin(), it->second.end());
    }
    auto update = false;
    if (blk->factoredExits.size() != fe.size()) {
      update = true;
      FTRACE(2, "Old factored edges: blk:{} -", blk->id);
      for (auto DEBUG_ONLY id : blk->factoredExits) FTRACE(2, " {}", id);
      FTRACE(2, "\n");
      blk->factoredExits.resize(fe.size());
    }

    size_t i = 0;
    for (auto b : fe) {
      if (blk->factoredExits[i] != b) {
        assert(update);
        blk->factoredExits[i] = b;
      }
      i++;
    }
    if (update) {
      FTRACE(2, "New factored edges: blk:{} -", blk->id);
      for (auto DEBUG_ONLY id : blk->factoredExits) FTRACE(2, " {}", id);
      FTRACE(2, "\n");
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
    };
    forEachNormalSuccessor(*blk, [&](const BlockId& succId) {
        auto skip = next_real_block(func, succId);
        if (skip != succId) {
          const_cast<BlockId&>(succId) = skip;
          anyChanges = true;
        }
        handleSucc(succId);
        numSucc++;
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
      // The blocks have the same exnNode, and the same section
      // so they must have the same factoredExits.
      assert(blk->factoredExits == nxt->factoredExits);
      std::copy(nxt->hhbcs.begin(), nxt->hhbcs.end(),
                std::back_inserter(blk->hhbcs));
      nxt->fallthrough = NoBlockId;
      nxt->id = NoBlockId;
      nxt->hhbcs = { bc::Nop {} };
      anyChanges = true;
    }
    auto const& bInfo = blockInfo[blk->id];
    if (bInfo.couldBeSwitch &&
        (bInfo.multiplePreds || !bInfo.onlySwitch || !bInfo.followsSwitch)) {
      // This block looks like it could be part of a switch, and it's
      // not in the middle of a sequence of such blocks.
      if (buildSwitches(func, borrow(blk), blockInfo)) {
        anyChanges = true;
      }
    }
  }

  return anyChanges;
}

//////////////////////////////////////////////////////////////////////

}}
