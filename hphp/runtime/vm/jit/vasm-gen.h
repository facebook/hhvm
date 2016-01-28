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

#ifndef incl_HPHP_JIT_VASM_GEN_H_
#define incl_HPHP_JIT_VASM_GEN_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-text.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

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
  Vout(Vunit& u, Vlabel b, const IRInstruction* origin = nullptr)
    : m_unit(u)
    , m_block(b)
    , m_origin(origin)
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
  void setOrigin(const IRInstruction* i) { m_origin = i; }

  /*
   * Vunit delegations.
   */
  Vreg makeReg();
  Vtuple makeTuple(const VregList& regs) const;
  Vtuple makeTuple(VregList&& regs) const;
  VcallArgsId makeVcallArgs(VcallArgs&& args) const;
  template<class T> Vreg cns(T v);

private:
  Vunit& m_unit;
  Vlabel m_block;
  const IRInstruction* m_origin;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Vasm assembler.
 *
 * A Vasm manages Vout streams for a Vunit.
 */
struct Vasm {
  Vasm() { m_outs.reserve(kNumAreas); }

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
  Vunit m_unit;
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
  explicit Vauto(CodeBlock& code, CodeKind kind = CodeKind::Helper)
    : m_kind{kind}
    , m_text{code, code}
    , m_main{m_unit, m_unit.makeBlock(AreaIndex::Main)}
    , m_cold{m_unit, m_unit.makeBlock(AreaIndex::Cold)}
  {
    m_unit.entry = Vlabel(main());
  }

  Vunit& unit() { return m_unit; }
  Vout& main() { return m_main; }
  Vout& cold() { return m_cold; }

  ~Vauto();

private:
  CodeKind m_kind;
  Vunit m_unit;
  Vtext m_text;
  Vout m_main;
  Vout m_cold;
};

/*
 * Convenience wrappers around Vauto for cross-trace code.
 */
template<class GenFunc>
TCA vwrap(CodeBlock& cb, GenFunc gen,
          CodeKind kind = CodeKind::CrossTrace) {
  auto const start = cb.frontier();
  Vauto vauto { cb, kind };
  gen(vauto.main());
  return start;
}
template<class GenFunc>
TCA vwrap2(CodeBlock& cb, GenFunc gen,
           CodeKind kind = CodeKind::CrossTrace) {
  auto const start = cb.frontier();
  Vauto vauto { cb, kind };
  gen(vauto.main(), vauto.cold());
  return start;
}

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/vasm-gen-inl.h"

#endif
