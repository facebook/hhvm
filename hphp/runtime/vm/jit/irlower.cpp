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

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-text.h"

#include "hphp/util/data-block.h"
#include "hphp/util/disasm.h"
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
size_t genBlock(IRLS& env, Vout& v, Vout& vc, Block& block) {
  FTRACE(6, "genBlock: {}\n", block.id());

  env.vmain = &v;
  env.vcold = &vc;
  CodeGenerator cg(env);

  size_t hhir_count{0};
  for (auto& inst : block) {
    hhir_count++;
    v.setOrigin(&inst);
    vc.setOrigin(&inst);
    cg.cgInst(&inst);
  }
  return hhir_count;
}

///////////////////////////////////////////////////////////////////////////////

void genArch(Vunit& vunit, Vtext& vtext, CGMeta& meta,
             CodeKind kind, AsmInfo* ai) {
  switch (arch()) {
    case Arch::X64:
      optimizeX64(vunit, abi(kind));
      emitX64(vunit, vtext, meta, ai);
      return;

    case Arch::ARM:
      finishARM(vunit, vtext, meta, abi(kind), ai);
      return;

    case Arch::PPC64:
      not_implemented();
      return;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

void relocateCode(const IRUnit& unit, size_t hhir_count,
                  CodeBlock& main, CodeBlock& main_in, CodeAddress main_start,
                  CodeBlock& cold, CodeBlock& cold_in, CodeAddress cold_start,
                  CodeBlock& frozen, CodeAddress frozen_start,
                  AsmInfo* ai, CGMeta& meta) {
  auto const& bc_map = meta.bcMap;
  if (!bc_map.empty()) {
    TRACE(1, "bcmaps before relocation\n");
    for (UNUSED auto const& map : bc_map) {
      TRACE(1, "%s %-6d %p %p %p\n",
            map.md5.toString().c_str(),
            map.bcStart,
            map.aStart,
            map.acoldStart,
            map.afrozenStart);
    }
  }
  if (ai) printUnit(kRelocationLevel, unit, " before relocation ", ai);

  RelocationInfo rel;
  size_t asm_count{0};

  asm_count += x64::relocate(rel, main_in,
                             main.base(), main.frontier(),
                             meta, nullptr);
  asm_count += x64::relocate(rel, cold_in,
                             cold.base(), cold.frontier(),
                             meta, nullptr);

  TRACE(1, "hhir-inst-count %ld asm %ld\n", hhir_count, asm_count);

  if (&frozen != &cold) {
    rel.recordRange(frozen_start, frozen.frontier(),
                    frozen_start, frozen.frontier());
  }
  x64::adjustForRelocation(rel);
  x64::adjustMetaDataForRelocation(rel, ai, meta);
  x64::adjustCodeForRelocation(rel, meta);

  if (ai) {
    static int64_t mainDeltaTotal = 0, coldDeltaTotal = 0;
    int64_t mainDelta = (main_in.frontier() - main_start) -
                        (main.frontier() - main.base());
    int64_t coldDelta = (cold_in.frontier() - cold_start) -
                        (cold.frontier() - cold.base());

    mainDeltaTotal += mainDelta;
    coldDeltaTotal += coldDelta;

    if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, 1)) {
      HPHP::Trace::traceRelease("main delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                mainDelta, mainDeltaTotal);
      HPHP::Trace::traceRelease("cold delta after relocation: "
                                "%" PRId64 " (%" PRId64 ")\n",
                                coldDelta, coldDeltaTotal);
    }
  }

#ifndef NDEBUG
  auto& ip = meta.inProgressTailJumps;
  for (size_t i = 0; i < ip.size(); ++i) {
    const auto& ib = ip[i];
    assertx(!main.contains(ib.toSmash()));
    assertx(!cold.contains(ib.toSmash()));
  }
  memset(main.base(), 0xcc, main.frontier() - main.base());
  memset(cold.base(), 0xcc, cold.frontier() - cold.base());
#endif
}

///////////////////////////////////////////////////////////////////////////////

void genCodeImpl(const IRUnit& unit, CodeCache::View code,
                 CGMeta& meta, CodeKind kind, AsmInfo* ai) {
  Timer _t(Timer::codeGen);
  CodeBlock& main_in = code.main();
  CodeBlock& cold_in = code.cold();

  CodeBlock main;
  CodeBlock cold;
  CodeBlock* frozen = &code.frozen();

  bool do_relocate = false;

  if (!RuntimeOption::EvalEnableReusableTC &&
      RuntimeOption::EvalJitRelocationSize &&
      cold_in.canEmit(RuntimeOption::EvalJitRelocationSize * 3)) {
    // This is mainly to exercise the relocator, and ensure that its not broken
    // by new non-relocatable code.  Later, it will be used to do some peephole
    // optimizations, such as reducing branch sizes.
    //
    // Allocate enough space that the relocated cold code doesn't overlap the
    // emitted cold code.
    static unsigned seed = 42;
    auto off = rand_r(&seed) & (cache_line_size() - 1);

    cold.init(cold_in.frontier() +
              RuntimeOption::EvalJitRelocationSize + off,
              RuntimeOption::EvalJitRelocationSize - off, "cgRelocCold");
    main.init(cold.frontier() +
              RuntimeOption::EvalJitRelocationSize + off,
              RuntimeOption::EvalJitRelocationSize - off, "cgRelocMain");

    do_relocate = true;
  } else {
    // Use separate code blocks, so that attempts to use the mcg's code blocks
    // directly will fail (e.g., by overwriting the same memory being written
    // through these locals).
    cold.init(cold_in.frontier(), cold_in.available(), cold_in.name().c_str());
    main.init(main_in.frontier(), main_in.available(), main_in.name().c_str());
  }

  if (frozen == &cold_in) frozen = &cold;

  DEBUG_ONLY auto main_start = main_in.frontier();
  DEBUG_ONLY auto cold_start = cold_in.frontier();
  auto frozen_start = frozen->frontier();

  size_t hhir_count{0};

  { Vasm vasm;
    auto& vunit = vasm.unit();
    vunit.transKind = unit.context().kind;
    SCOPE_ASSERT_DETAIL("vasm unit") { return show(vunit); };

    IRLS env(unit);
    auto const blocks = rpoSortCfg(unit);

    // Create the initial set of vasm blocks, numbered the same as the
    // corresponding HHIR blocks.
    for (uint32_t i = 0, n = unit.numBlocks(); i < n; ++i) {
      env.labels[i] = vunit.makeBlock(AreaIndex::Main);
    }

    // Create Vregs for all relevant SSATmps.
    assignRegs(unit, vunit, env, blocks);

    vunit.entry = env.labels[unit.entry()];
    Vtext vtext { main, cold, *frozen };

    for (auto block : blocks) {
      auto& v = block->hint() == Block::Hint::Unlikely ? vasm.cold() :
                block->hint() == Block::Hint::Unused ? vasm.frozen() :
                vasm.main();
      FTRACE(6, "genBlock {} on {}\n", block->id(),
             area_names[(unsigned)v.area()]);

      auto b = env.labels[block];
      vunit.blocks[b].area_idx = v.area();
      v.use(b);

      hhir_count += genBlock(env, v, vasm.cold(), *block);

      assertx(v.closed());
      assertx(vasm.main().empty() || vasm.main().closed());
      assertx(vasm.cold().empty() || vasm.cold().closed());
      assertx(vasm.frozen().empty() || vasm.frozen().closed());
    }
    printUnit(kInitialVasmLevel, "after initial vasm generation", vunit);
    assertx(check(vunit));

    genArch(vunit, vtext, meta, kind, ai);
  }

  assertx(cold_in.frontier() == cold_start);
  assertx(main_in.frontier() == main_start);

  if (do_relocate) {
    relocateCode(unit, hhir_count,
                 main, main_in, main_start,
                 cold, cold_in, cold_start,
                 *frozen, frozen_start, ai, meta);
  } else {
    cold_in.skip(cold.frontier() - cold_in.frontier());
    main_in.skip(main.frontier() - main_in.frontier());
  }

  if (ai) printUnit(kCodeGenLevel, unit, " after code gen ", ai);
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void genCode(const IRUnit& unit, CodeCache::View code, CGMeta& meta,
             CodeKind kind /* = CodeKind::Trace */) {
  if (dumpIREnabled()) {
    AsmInfo ai(unit);
    genCodeImpl(unit, code, meta, kind, &ai);
  } else {
    genCodeImpl(unit, code, meta, kind, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
