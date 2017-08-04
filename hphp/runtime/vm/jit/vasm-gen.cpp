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

#include "hphp/runtime/vm/jit/vasm-gen.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#include "hphp/util/arch.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////
// Vout.

Vout& Vout::operator<<(const Vinstr& inst) {
  assertx(!closed());

  auto& code = m_unit.blocks[m_block].code;
  code.emplace_back(inst);
  code.back().origin = m_irctx.origin;
  code.back().voff = m_irctx.voff == Vinstr::kInvalidVoff
    ? m_unit.cur_voff++
    : m_irctx.voff;

  // Don't let `voff' overflow.  It's only used for debugging and logging, so a
  // value of UINT_MAX - 1 can be considered to have an implicit "or greater"
  // qualifier attached.
  if (UNLIKELY(m_unit.cur_voff == Vinstr::kInvalidVoff)) --m_unit.cur_voff;

  FTRACE(6, "Vout << {}\n", show(m_unit, inst));
  return *this;
}

bool Vout::empty() const {
  return m_unit.blocks[m_block].code.empty();
}

bool Vout::closed() const {
  return !empty() && isBlockEnd(m_unit.blocks[m_block].code.back());
}

Vout Vout::makeBlock() {
  uint64_t weight = 0;
  if (m_irctx.origin != nullptr) {
    weight = m_irctx.origin->block()->profCount();
  }
  weight = weight * areaWeightFactor(area());
  return {m_unit, m_unit.makeBlock(area(), weight), m_irctx};
}

///////////////////////////////////////////////////////////////////////////////
// Vasm.

Vout& Vasm::out(AreaIndex area) {
  auto const a = static_cast<size_t>(area);

  // Initialize all streams up through `area'.
  for (auto i = m_outs.size(); i <= a; ++i) {
    auto b = m_unit.makeBlock(static_cast<AreaIndex>(i), 0);
    m_outs.emplace_back(m_unit, b);
  }
  return m_outs[a];
}

///////////////////////////////////////////////////////////////////////////////
// Vauto.

Vauto::~Vauto() {
  for (auto& b : unit().blocks) {
    if (!b.code.empty()) {
      // Found at least one nonempty block.  Finish up.
      if (!main().closed()) main() << fallthru{};
      if (!cold().closed()) cold() << fallthru{};

      // Prevent spurious printir traces.
      Trace::Bump bumper{Trace::printir, 10};

      CodeBlock mainTmp, coldTmp;
      auto text = [&] {
        if (m_relocate) {
          auto& cold = m_text.cold().code;
          if (cold.canEmit(0x10000)) {
            mainTmp.init(cold.frontier() + 0x8000, 0x4000, "vauto main tmp");
            coldTmp.init(cold.frontier() + 0xc000, 0x4000, "vauto cold tmp");
            return Vtext(mainTmp, coldTmp, m_text.data());
          }
          m_relocate = false;
        }
        return std::move(m_text);
      }();

      auto const abi = jit::abi(m_kind);
      switch (arch()) {
        case Arch::X64:
          optimizeX64(unit(), abi, true /* regalloc */);
          emitX64(unit(), text, m_fixups, nullptr);
          break;
        case Arch::ARM:
          optimizeARM(unit(), abi, true /* regalloc */);
          emitARM(unit(), text, m_fixups, nullptr);
          break;
        case Arch::PPC64:
          optimizePPC64(unit(), abi, true /* regalloc */);
          emitPPC64(unit(), text, m_fixups, nullptr);
          break;
      }

      if (m_relocate) {
        tc::relocateTranslation(
          nullptr,
          mainTmp, m_text.main().code, m_text.main().code.frontier(),
          coldTmp, m_text.cold().code, m_text.cold().code.frontier(),
          coldTmp, coldTmp.frontier(),
          nullptr, m_fixups);
      }
      return;
    }
  }

  if (unit().padding) {
    // Force emission due to padding
    if (!main().closed()) main() << fallthru{};
    if (!cold().closed()) cold() << fallthru{};

    switch (arch()) {
      case Arch::X64:
        emitX64(unit(), m_text, m_fixups, nullptr);
        break;
      case Arch::ARM:
        emitARM(unit(), m_text, m_fixups, nullptr);
        break;
      case Arch::PPC64:
        emitPPC64(unit(), m_text, m_fixups, nullptr);
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
