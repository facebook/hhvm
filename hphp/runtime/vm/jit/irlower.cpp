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

#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/arch.h"
#include "hphp/util/assertions.h"
#include "hphp/util/trace.h"

#include <folly/Format.h>
#include <folly/Range.h>

#include <sstream>

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

namespace {

///////////////////////////////////////////////////////////////////////////////

void cgInst(IRLS& env, const IRInstruction* inst){
  SCOPE_ASSERT_DETAIL("cgInst") { return inst->toString(); };

  switch (inst->op()) {
#define O(name, dsts, srcs, flags)  \
    case name:                      \
      FTRACE(7, "cg" #name "\n");   \
      cg##name(env, inst);          \
      break;
    IR_OPCODES
#undef O
    default:
      always_assert(false);
  }

  auto& v = vmain(env);
  if (inst->isBlockEnd() && !v.closed()) {
    if (auto const next = inst->next()) {
      v << jmp{label(env, next)};
    } else {
      v << ud2{}; // or end?
    }
  }
}

/*
 * Lower `block' from HHIR to vasm.
 */
void genBlock(IRLS& env, Vout& v, Vout& vc, Block& block) {
  FTRACE(6, "genBlock: {}\n", block.id());

  env.vmain = &v;
  env.vcold = &vc;

  for (auto& inst : block) {
    v.unit().cur_voff = 0;
    v.setOrigin(&inst);
    vc.setOrigin(&inst);
    cgInst(env, &inst);
  }
}

void optimize(Vunit& unit, CodeKind kind, bool regAlloc) {
  auto const abi = jit::abi(kind);
  switch (arch()) {
    case Arch::X64:
      optimizeX64(unit, abi, regAlloc);
      break;
    case Arch::ARM:
      optimizeARM(unit, abi, regAlloc);
      break;
    case Arch::PPC64:
      optimizePPC64(unit, abi, regAlloc);
      break;
  }
}

/*
 * Computes inline frames for each block in unit. Inline frames are dominated
 * by an inlinestart instruction and post-dominated by an inlineend instruction.
 * This function annotates Vblocks with their associated frame, and populates
 * the frame vector. Additionally, inlinestart and inlineend instructions are
 * replaced by jmp instructions.
 */
void computeFrames(Vunit& unit) {
  auto const topFunc = unit.context ? unit.context->func : nullptr;

  auto const rpo = sortBlocks(unit);

  unit.frames.emplace_back(topFunc, Vframe::Top, 0, unit.blocks[rpo[0]].weight);
  unit.blocks[rpo[0]].frame = 0;
  for (auto const b : rpo) {
    auto& block = unit.blocks[b];
    assert_flog(block.frame != -1, "Block frames cannot be uninitialized.");

    if (block.code.empty()) continue;

    auto const next_frame = [&] () -> int {
      auto frame = block.frame;
      for (auto& inst : block.code) {
        switch (inst.op) {
        case Vinstr::inlinestart:
          // Each inlined frame will have a single start but may have multiple
          // ends, and so we need to propagate this state here so that it only
          // happens once per frame.
          for (auto f = frame; f != Vframe::Top; f = unit.frames[f].parent) {
            unit.frames[f].inclusive_cost += inst.inlinestart_.cost;
            unit.frames[f].num_inner_frames++;
          }

          unit.frames.emplace_back(
            inst.inlinestart_.func,
            frame,
            inst.inlinestart_.cost,
            block.weight
          );
          frame = inst.inlinestart_.id = unit.frames.size() - 1;
          break;
        case Vinstr::inlineend:
          frame = unit.frames[frame].parent;
          break;
        default: break;
        }
      }
      return frame;
    }();

    for (auto const s : succs(block)) {
      auto& sblock = unit.blocks[s];
      assert_flog(
        sblock.frame == -1 || sblock.frame == next_frame,
        "Blocks must be dominated by a single inline frame {} cannot have "
        "frames {} ({}) and {} ({}).",
        s,
        sblock.frame,
        unit.frames[sblock.frame].func
          ? unit.frames[sblock.frame].func->fullName()->data()
          : "(null)",
        next_frame,
        unit.frames[next_frame].func
          ? unit.frames[next_frame].func->fullName()->data()
          : "(null)"
      );
      sblock.frame = next_frame;
    }
  }
}

}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Vunit> lowerUnit(const IRUnit& unit, CodeKind kind,
                                 bool regAlloc /* = true */) noexcept {
  Timer timer(Timer::hhir_lower, unit.logEntry().get_pointer());
  SCOPE_ASSERT_DETAIL("hhir unit") { return show(unit); };

  auto vunit = std::make_unique<Vunit>();
  vunit->context = unit.context();
  vunit->log_entry = unit.logEntry().get_pointer();
  vunit->profiling = true;
  Vasm vasm{*vunit};
  SCOPE_ASSERT_DETAIL("vasm unit") { return show(*vunit); };

  IRLS env{unit};
  auto const blocks = rpoSortCfg(unit);

  // Create the initial set of vasm blocks, numbered the same as the
  // corresponding HHIR blocks.
  // We initially create the blocks with 0 weight, and then set them below.
  for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
    env.labels[i] = vunit->makeBlock(AreaIndex::Main, 0);
  }
  vunit->entry = env.labels[unit.entry()];

  // Create Vregs for all relevant SSATmps.
  assignRegs(unit, *vunit, env, blocks);

  for (auto block : blocks) {
    auto& v = block->hint() == Block::Hint::Unlikely ? vasm.cold() :
              block->hint() == Block::Hint::Unused ? vasm.frozen() :
              vasm.main();
    FTRACE(6, "genBlock {} on {}\n", block->id(),
           area_names[(unsigned)v.area()]);

    auto b = env.labels[block];
    vunit->blocks[b].area_idx = v.area();
    vunit->blocks[b].weight = block->profCount() * areaWeightFactor(v.area());
    v.use(b);

    genBlock(env, v, vasm.cold(), *block);

    assertx(v.closed());
    assertx(vasm.main().empty() || vasm.main().closed());
    assertx(vasm.cold().empty() || vasm.cold().closed());
    assertx(vasm.frozen().empty() || vasm.frozen().closed());
  }

  printUnit(kInitialVasmLevel, "after initial vasm generation", *vunit);
  assertx(check(*vunit));
  timer.stop();

  // Lower inlinestart and inlineend instructions to jmps, and annotate blocks
  // with inlined function parents
  computeFrames(*vunit);

  try {
    optimize(*vunit, kind, regAlloc);
  } catch (const FailedTraceGen& e) {
    // vasm-xls can fail if it tries to allocate too many spill slots.
    logLowPriPerfWarning(
      "vasm-optimize punt",
      1000,
      [&](StructuredLogEntry& cols) {
        cols.setStr("punt_type", e.what());
        cols.setStr("vasm_unit", show(*vunit));
      }
    );
    FTRACE(1, "vasm-optimize failed with {}\n", e.what());
    return nullptr;
  }

  return vunit;
}

Vcost computeIRUnitCost(const IRUnit& unit) {
  auto const vunit = lowerUnit(unit, CodeKind::Trace, false /* regAlloc */);
  return computeVunitCost(*vunit);
}


///////////////////////////////////////////////////////////////////////////////

}}}
