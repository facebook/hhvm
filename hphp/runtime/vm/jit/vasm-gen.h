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

#pragma once

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/data-block.h"

#include <boost/type_traits.hpp>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct IRInstruction;
struct Vinstr;
struct Vreg;

///////////////////////////////////////////////////////////////////////////////

/*
 * Writer stream for adding instructions to a Vblock.
 */
struct Vout {
  Vout(Vunit& u, Vlabel b,
       Optional<Vinstr::ir_context> irctx = std::nullopt)
    : m_unit(u)
    , m_block(b)
    , m_irctx ( irctx ? *irctx
                      : Vinstr::ir_context { nullptr, Vinstr::kInvalidVoff } )
  {}

  Vout(Vout&&) = default;
  Vout(const Vout&) = delete;

  Vout& operator=(const Vout&);
  Vout& operator=(Vlabel);

  /*
   * Implicit cast to label for initializing branch instructions.
   */
  /* implicit */ operator Vlabel() const;

  /*
   * Instruction emitter.
   */
  Vout& operator<<(const Vinstr& inst);

  /*
   * Create a stream connected to a new empty block.
   */
  Vout makeBlock();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Whether the stream is empty or terminated (because a block-terminating
   * instruction has been emitted).
   */
  bool empty() const;
  bool closed() const;

  /*
   * Return the managed Vunit.
   */
  Vunit& unit() { return m_unit; }

  /*
   * Return the code area of the currently-managed block.
   */
  AreaIndex area() const;

  /*
   * Set the current block and Vinstr origin.
   */
  void use(Vlabel b)                     { m_block = b; }
  void setOrigin(const IRInstruction* i) { m_irctx.origin = i; }

  /*
   * Add a scaling factor. The weight of the current block and all
   * future blocks created from this Vout will be multiplied by the
   * total scaling factor.
   */
  void addWeightScale(uint64_t);

  /*
   * Vunit delegations.
   */
  Vreg makeReg();
  Vaddr makeAddr();
  Vtuple makeTuple(const VregList& regs) const;
  Vtuple makeTuple(VregList&& regs) const;
  VcallArgsId makeVcallArgs(VcallArgs&& args) const;
  template<class T> Vreg cns(T v);
  template<class T, class... Args> T* allocData(Args&&... args);

private:
  Vout(Vunit& u, Vlabel b, Vinstr::ir_context irctx, uint64_t scale)
    : m_unit{u}
    , m_block{b}
    , m_irctx{irctx}
    , m_weight_scale{scale} {}

  Vunit& m_unit;
  Vlabel m_block;
  Vinstr::ir_context m_irctx;
  uint64_t m_weight_scale = 1;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Vasm assembler.
 *
 * A Vasm manages Vout streams for a Vunit.
 */
struct Vasm {
  explicit Vasm(Vunit& unit) : m_unit(unit) { m_outs.reserve(kNumAreas); }

  /*
   * Obtain the managed Vunit.
   */
  Vunit& unit() { return m_unit; }

  /*
   * Get or create the stream for the corresponding Varea.
   */
  Vout& main() { return out(AreaIndex::Main); }
  Vout& cold() { return out(AreaIndex::Cold); }
  Vout& frozen() { return out(AreaIndex::Frozen); }

private:
  Vout& out(AreaIndex i);

private:
  Vunit& m_unit;
  jit::vector<Vout> m_outs; // one for each AreaIndex
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Helper for emitting small amounts of machine code using vasm.
 *
 * Vauto manages a Vunit with a single area.  When the Vauto goes out of scope,
 * it will finalize and emit any code it contains.
 */
struct Vauto {
  explicit Vauto(CodeBlock& main, CodeBlock& cold, DataBlock& data,
                 CGMeta& fixups, CodeKind kind = CodeKind::Helper,
                 bool relocate = false, bool* wasFull = nullptr)
    : m_text{main, cold, data}
    , m_fixups(fixups)
    , m_main{m_unit, m_unit.makeBlock(AreaIndex::Main)}
    , m_cold{m_unit, m_unit.makeBlock(AreaIndex::Cold)}
    , m_kind{kind}
    , m_relocate{relocate}
    , m_wasFull{wasFull}
  {
    m_unit.entry = Vlabel(this->main());
  }

  Vunit& unit() { return m_unit; }
  Vout& main() { return m_main; }
  Vout& cold() { return m_cold; }

  ~Vauto();

private:
  Vunit m_unit;
  Vtext m_text;
  CGMeta& m_fixups;
  Vout m_main;
  Vout m_cold;
  CodeKind m_kind;
  bool m_relocate;
  bool* m_wasFull;
};

namespace detail {
  template<class GenFunc>
  TCA vwrap_impl(CodeBlock& main, CodeBlock& cold, DataBlock& data,
                 CGMeta* meta, GenFunc gen,
                 CodeKind kind = CodeKind::CrossTrace,
                 bool relocate = true,
                 bool nullOnFull = false);
}

/*
 * Convenience wrappers around Vauto for cross-trace or helper code.
 *
 * The relocate parameter attempts to generate code into a temporary
 * buffer allocated from cb, and then relocate it to the start of cb -
 * which can reduce the code size. If there isn't enough room in cb,
 * the parameter will be ignored.
 */
template<class GenFunc>
TCA vwrap(CodeBlock& cb, DataBlock& data, CGMeta& meta, GenFunc gen,
          CodeKind kind = CodeKind::CrossTrace, bool relocate = true) {
  return detail::vwrap_impl(cb, cb, data, &meta,
                            [&] (Vout& v, Vout&) { gen(v); }, kind, relocate);
}
template<class GenFunc>
TCA vwrap(CodeBlock& cb, DataBlock& data, GenFunc gen,
          bool relocate = true, bool nullOnFull = false) {
  return detail::vwrap_impl(cb, cb, data, nullptr,
                            [&] (Vout& v, Vout&) { gen(v); },
                            CodeKind::CrossTrace, relocate, nullOnFull);
}
template<class GenFunc>
TCA vwrap2(CodeBlock& main, CodeBlock& cold, DataBlock& data,
           CGMeta& meta, GenFunc gen) {
  return detail::vwrap_impl(main, cold, data, &meta, gen);
}
template<class GenFunc>
TCA vwrap2(CodeBlock& main, CodeBlock& cold, DataBlock& data, GenFunc gen) {
  return detail::vwrap_impl(main, cold, data, nullptr, gen);
}

/*
 * Multiplying factors used to compute the block weights for each code area.
 * We multiply the corresponding IR block's profile counter by the following
 * factors, depending on the code area the block is assigned to.
 */
uint64_t areaWeightFactor(AreaIndex area);

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/vasm-gen-inl.h"
