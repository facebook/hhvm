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

#include "hphp/runtime/vm/jit/vasm-emit.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/asm-info.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/arch.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(vasm);

namespace {

struct BlockInfo {
  uintptr_t base;
  size_t size;
};
using DataBlockMap = jit::flat_map<uintptr_t, BlockInfo>;

struct DataPtrVisitor {
  explicit DataPtrVisitor(const DataBlockMap& blocks) : m_blocks(blocks) {}

  template<typename T> void imm(T) {}
  template<typename T> void imm(VdataPtr<T>& ptr) {
    if (ptr.bound()) return;

    auto const unboundPtr = (uintptr_t)ptr.getRaw();
    auto it = m_blocks.upper_bound(unboundPtr);
    assertx(it != m_blocks.begin());
    it--;
    auto const boundPtr = it->second.base + (unboundPtr - it->first);
    assertx(boundPtr < it->second.base + it->second.size);
    ptr.bind((T*)boundPtr);
  }

  template<typename T> void use(T) {}
  template<typename T> void use(VdataPtr<T>& ptr) = delete;
  template<typename R, typename H> void useHint(R r, H) { use(r); }

  template<typename T> void across(T) {}
  template<typename T> void across(VdataPtr<T>&) = delete;

  template<typename T> void def(T) {}
  template<typename T> void def(VdataPtr<T>&) = delete;
  template<typename R, typename H> void defHint(R r, H) { def(r); }

private:
  const DataBlockMap& m_blocks;
};


void bindDataPtrs(Vunit& vunit, DataBlock& data) {
  if (vunit.dataBlocks.empty()) return;

  tc::assertOwnsCodeLock();
  Timer timer(Timer::vasm_bind_ptrs);
  FTRACE(1, "{:-^80}\n", "binding VdataPtrs");

  DataBlockMap blocks;
  for (auto& dataBlock : vunit.dataBlocks) {
    auto oldPtr = dataBlock.data.get();
    auto newPtr = data.allocRaw(dataBlock.size, dataBlock.align);
    std::memcpy(newPtr, oldPtr, dataBlock.size);
    FTRACE(2, "  allocated {} bytes at {:#x}, moving from {:#x}\n",
           dataBlock.size, (uintptr_t)newPtr, (uintptr_t)oldPtr);
    blocks.emplace((uintptr_t)oldPtr,
                   BlockInfo{(uintptr_t)newPtr, dataBlock.size});
  }

  DataPtrVisitor ptrVisitor{blocks};

  PostorderWalker{vunit}.dfs([&](Vlabel b) {
    auto& block = vunit.blocks[b];
    for (auto& inst : block.code) {
      visitOperands(inst, ptrVisitor);
    }
  });
}

void emit(const Vunit& vunit, Vtext& vtext, CGMeta& meta, AsmInfo* ai) {
  switch (arch()) {
    case Arch::X64:
      emitX64(vunit, vtext, meta, ai);
      break;
    case Arch::ARM:
      emitARM(vunit, vtext, meta, ai);
      break;
    case Arch::PPC64:
      emitPPC64(vunit, vtext, meta, ai);
      break;
  }
}

}

void emitVunit(Vunit& vunit, const IRUnit& unit,
               CodeCache::View code, CGMeta& meta, Annotations* annotations) {
  Timer _t(Timer::vasm_emit);
  SCOPE_ASSERT_DETAIL("vasm unit") { return show(vunit); };
  tc::assertOwnsCodeLock();

  CodeBlock& main_in = code.main();
  CodeBlock& cold_in = code.cold();

  CodeBlock main;
  CodeBlock cold;
  CodeBlock* frozen = &code.frozen();

  auto const do_relocate = arch() == Arch::X64 &&
    !RuntimeOption::EvalEnableReusableTC &&
    RuntimeOption::EvalJitRelocationSize &&
    cold_in.canEmit(RuntimeOption::EvalJitRelocationSize * 3);

  // If code relocation is supported and enabled, set up temporary code blocks.
  if (do_relocate) {
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
  } else {
    // Use separate code blocks, so that attempts to use code's blocks
    // directly will fail (e.g., by overwriting the same memory being written
    // through these locals).
    cold.init(cold_in.frontier(), cold_in.available(), cold_in.name().c_str());
    main.init(main_in.frontier(), main_in.available(), main_in.name().c_str());
  }

  if (frozen == &cold_in) frozen = &cold;

  DEBUG_ONLY auto main_start = main_in.frontier();
  DEBUG_ONLY auto cold_start = cold_in.frontier();
  auto frozen_start = frozen->frontier();

  folly::Optional<AsmInfo> optAI;
  if (dumpIREnabled()) optAI.emplace(unit);
  auto ai = optAI.get_pointer();

  Vtext vtext{main, cold, *frozen, code.data()};
  bindDataPtrs(vunit, vtext.data());
  emit(vunit, vtext, meta, ai);

  assertx(cold_in.frontier() == cold_start);
  assertx(main_in.frontier() == main_start);

  if (do_relocate) {
    tc::relocateTranslation(unit,
                            main, main_in, main_start,
                            cold, cold_in, cold_start,
                            *frozen, frozen_start, ai, meta);
  } else {
    cold_in.skip(cold.frontier() - cold_in.frontier());
    main_in.skip(main.frontier() - main_in.frontier());
  }

  if (ai) {
    printUnit(kCodeGenLevel, unit, " after code gen ",
              ai, nullptr, annotations);
  }
}

}}
