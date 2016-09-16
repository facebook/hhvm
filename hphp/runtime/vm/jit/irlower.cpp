/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/perf-warning.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
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
#include "hphp/util/trace.h"

#include <folly/Format.h>
#include <folly/Range.h>

#include <cstdlib>
#include <cstring>
#include <sstream>

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Lower `block' from HHIR to vasm.
 */
void genBlock(IRLS& env, Vout& v, Vout& vc, Block& block) {
  FTRACE(6, "genBlock: {}\n", block.id());

  env.vmain = &v;
  env.vcold = &vc;
  CodeGenerator cg(env);

  for (auto& inst : block) {
    v.unit().cur_voff = 0;
    v.setOrigin(&inst);
    vc.setOrigin(&inst);
    cg.cgInst(&inst);
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

}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Vunit> lowerUnit(const IRUnit& unit, CodeKind kind,
                                 bool regAlloc /* = true */) noexcept {
  Timer timer(Timer::hhir_lower);
  SCOPE_ASSERT_DETAIL("hhir unit") { return show(unit); };

  auto vunit = folly::make_unique<Vunit>();
  vunit->context = unit.context();
  vunit->log_entry = unit.logEntry().get_pointer();
  Vasm vasm{*vunit};
  SCOPE_ASSERT_DETAIL("vasm unit") { return show(*vunit); };

  IRLS env{unit};
  auto const blocks = rpoSortCfg(unit);

  // Create the initial set of vasm blocks, numbered the same as the
  // corresponding HHIR blocks.
  for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
    env.labels[i] = vunit->makeBlock(AreaIndex::Main);
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

  try {
    optimize(*vunit, kind, regAlloc);
  } catch (const FailedTraceGen& e) {
    // vasm-xls can fail if it tries to allocate too many spill slots.
    logLowPriPerfWarning(
      "vasm-optimize punt",
      250,
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
