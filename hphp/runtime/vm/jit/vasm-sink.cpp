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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <cstdint>
#include <utility>

TRACE_SET_MOD(vasm)

namespace HPHP::jit {

namespace {

using UseBlocks = jit::vector<Vlabel>;
constexpr auto kInvalidRpoOrder = int32_t{-1};

struct SinkAnalysis {
  jit::vector<Vlabel> rpo;
  jit::vector<Vlabel> idoms;
  jit::vector<int32_t> rpoOrder;
  jit::vector<UseBlocks> useBlocks;
};

struct SinkMove {
  Vlabel src;
  size_t srcIdx;
  Vlabel dst;
};

bool isSinkableDef(const Vunit& unit,
                   const Abi& abi,
                   const Vinstr& inst,
                   Vreg& d) {
  if (!isPure(inst)) return false;
  if (isBlockEnd(inst)) return false;
  // ssaalias is a restoreSSA marker, not an ordinary movable value definition.
  if (inst.op == Vinstr::ssaalias) return false;

  auto defs = size_t{0};
  auto touchesPhys = false;
  auto touchesFlags = false;
  auto const noteOperand = [&] (Vreg r, Width w) {
    if (!r.isValid()) return;
    // isPure() still allows explicit flag consumers/producers such as cmov/test,
    // but sinkDefs only moves register-only definitions.
    if (w == Width::Flags) {
      touchesFlags = true;
      return;
    }
    if (r.isPhys()) touchesPhys = true;
  };

  visitUses(unit, inst, noteOperand);
  visitDefs(unit, inst, [&] (Vreg r, Width w) {
    noteOperand(r, w);
    if (!r.isValid() || w == Width::Flags || r.isPhys()) return;
    d = r;
    ++defs;
  });

  RegSet implicitUses, implicitAcross, implicitDefs;
  getEffects(abi, inst, implicitUses, implicitAcross, implicitDefs);
  if (!implicitUses.empty() || !implicitAcross.empty() || !implicitDefs.empty()) {
    touchesPhys = true;
  }

  return
    !touchesFlags &&
    !touchesPhys &&
    defs == 1 &&
    d.isValid() &&
    !d.isPhys() &&
    !d.isSF();
}

size_t blockInsertIndex(const Vblock& block) {
  auto idx = size_t{0};
  while (idx < block.code.size()) {
    auto const op = block.code[idx].op;
    // These are structural block-header ops, not ordinary insertion points:
    // landingpad marks catch-block entry and phidef models incoming-edge defs.
    if (op != Vinstr::landingpad && op != Vinstr::phidef) break;
    ++idx;
  }
  return idx;
}

jit::vector<UseBlocks> computeUseBlocks(const Vunit& unit,
                                        const jit::vector<Vlabel>& rpo) {
  auto uses = jit::vector<UseBlocks>(unit.next_vr);

  for (auto const b : rpo) {
    for (auto const& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&] (Vreg r) {
        if (!r.isValid() || r.isPhys()) return;
        assertx(r < uses.size());
        auto& blocks = uses[r];
        if (blocks.empty() || blocks.back() != b) blocks.push_back(b);
      });
    }
  }

  return uses;
}

SinkAnalysis analyzeSinks(const Vunit& unit) {
  SinkAnalysis analysis;
  analysis.rpo = sortBlocks(unit);
  if (analysis.rpo.empty()) return analysis;

  analysis.idoms = findDominators(unit, analysis.rpo);
  analysis.useBlocks = computeUseBlocks(unit, analysis.rpo);
  analysis.rpoOrder =
    jit::vector<int32_t>(unit.blocks.size(), kInvalidRpoOrder);
  for (size_t i = 0; i < analysis.rpo.size(); ++i) {
    analysis.rpoOrder[analysis.rpo[i]] = safe_cast<int32_t>(i);
  }

  return analysis;
}

jit::vector<SinkMove> collectSinkMoves(const Vunit& unit,
                                       const Abi& abi,
                                       const SinkAnalysis& analysis) {
  auto moves = jit::vector<SinkMove>{};

  for (auto it = analysis.rpo.rbegin(); it != analysis.rpo.rend(); ++it) {
    auto const b = *it;
    auto const& block = unit.blocks[b];
    for (auto i = block.code.size(); i != 0; --i) {
      auto const idx = i - 1;
      auto d = Vreg{};
      if (!isSinkableDef(unit, abi, block.code[idx], d)) {
        continue;
      }

      auto target = Vlabel{};
      for (auto const useBlock : analysis.useBlocks[d]) {
        target = commonDominator(
          target,
          useBlock,
          analysis.idoms,
          analysis.rpoOrder
        );
      }
      if (!target.isValid() || target == b) continue;
      if (!dominates(b, target, analysis.idoms)) continue;

      moves.push_back(SinkMove{b, idx, target});
    }
  }

  return moves;
}

bool sinkDefsBatch(Vunit& unit, const Abi& abi) {
  auto const analysis = analyzeSinks(unit);
  if (analysis.rpo.empty()) return false;

  // Collect a whole batch from a stable snapshot first. Mutating block.code
  // while scanning would invalidate indices, and moving users may enable a
  // deeper sink for their inputs in the next batch.
  auto const moves = collectSinkMoves(unit, abi, analysis);
  if (moves.empty()) return false;

  // We collect source blocks in reverse RPO and source instructions in reverse
  // order. Since each target is dominated by its source, any block we insert
  // into has already been handled as a source, so the saved source indices stay
  // valid while we move each instruction directly.
  for (auto const& move : moves) {
    auto& src = unit.blocks[move.src];
    auto inst = std::move(src.code[move.srcIdx]);
    src.code.erase(src.code.begin() + move.srcIdx);

    auto& dst = unit.blocks[move.dst];
    auto const insertIdx = blockInsertIndex(dst);
    dst.code.insert(dst.code.begin() + insertIdx, std::move(inst));
    FTRACE(kVasmSinkDefsLevel, "sinkDefs moved {} from {}:{} to {}:{}\n",
           vinst_names[dst.code[insertIdx].op],
           size_t(move.src),
           move.srcIdx,
           size_t(move.dst),
           insertIdx);
  }

  return true;
}

}

void sinkDefs(Vunit& unit, const Abi& abi) {
  assertx(check(unit));

  auto changed = false;
  VpassTracer tracer{&unit, Trace::vasm, "vasm-sink", &changed};

  while (sinkDefsBatch(unit, abi)) {
    changed = true;
  }

  assertx(check(unit));
  printUnit(kVasmSinkDefsLevel, "after vasm sink", unit);
}

}
