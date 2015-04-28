/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_VASM_EMIT_H_
#define incl_HPHP_JIT_VASM_EMIT_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Abi;
struct AsmInfo;
struct IRInstruction;
struct Vinstr;
struct Vreg;
struct Vunit;
struct X64Assemblers;

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
  Vpoint makePoint();
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
 * Similar to X64Assembler, but buffers instructions as they are written, then
 * generates code all at once at the end.
 *
 * Areas represent the separate sections we generate code into.
 */
struct Vasm {
  struct Area {
    Vout out;
    CodeBlock& code;
    CodeAddress start;
  };
  using AreaList = jit::vector<Area>;

  Vasm() {
    m_areas.reserve(kNumAreas);
  }

  /*
   * Accessors.
   */
  Vunit& unit() { return m_unit; }
  AreaList& areas() { return m_areas; }

  /*
   * Get an existing area.
   */
  Vout& main() { return area(AreaIndex::Main).out; }
  Vout& cold() { return area(AreaIndex::Cold).out; }
  Vout& frozen() { return area(AreaIndex::Frozen).out; }

  /*
   * Create areas.
   */
  Vout& main(CodeBlock& cb) { return add(cb, AreaIndex::Main); }
  Vout& cold(CodeBlock& cb) { return add(cb, AreaIndex::Cold); }
  Vout& frozen(CodeBlock& cb) { return add(cb, AreaIndex::Frozen); }
  Vout& main(X64Assembler& a) { return main(a.code()); }
  Vout& cold(X64Assembler& a) { return cold(a.code()); }
  Vout& frozen(X64Assembler& a) { return frozen(a.code()); }

private:
  Area& area(AreaIndex i);
  Vout& add(CodeBlock &cb, AreaIndex area);

private:
  Vunit m_unit;
  AreaList m_areas; // indexed by AreaIndex
};

/*
 * Optimize, lower for x64, register allocator, and perform more optimizations
 * on unit.
 */
void optimizeX64(Vunit& unit, const Abi&);

/*
 * Emit code for the given unit using the given code areas. The unit should
 * have already been through optimizeX64().
 */
void emitX64(const Vunit&, Vasm::AreaList&, AsmInfo*);

/*
 * Optimize, register allocate, and emit ARM code for the given unit.
 */
void finishARM(Vunit&, Vasm::AreaList&, const Abi&, AsmInfo* asmInfo);

/*
 * Vauto is a convenience helper for emitting small amounts of machine code
 * using vasm.
 *
 * A Vauto always has a main code block; cold and frozen blocks may be added
 * using the normal vasm API after creation.  When the Vauto goes out of scope,
 * it will finalize and emit any code it contains.
 */
struct Vauto : Vasm {
  explicit Vauto(CodeBlock& code) {
    unit().entry = Vlabel(main(code));
  }
  ~Vauto();
};

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/vasm-emit-inl.h"

#endif
