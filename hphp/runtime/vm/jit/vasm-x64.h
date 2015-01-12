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

#ifndef incl_HPHP_JIT_VASM_X64_H_
#define incl_HPHP_JIT_VASM_X64_H_

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cpp-call.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"

#include <bitset>
#include <folly/Range.h>

namespace HPHP { namespace jit {
struct IRInstruction;
struct AsmInfo;
}}

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vblock {
  explicit Vblock(AreaIndex area) : area(area) {}
  AreaIndex area;
  jit::vector<Vinstr> code;
};

/*
 * Source operands for vcall/vinvoke instructions, packed into a struct for
 * convenience and to keep the instructions compact.
 */
struct VcallArgs {
  VregList args, simdArgs, stkArgs;
};

// Vasm constant: 1, 4, or 8 byte unsigned value, or the disp32 part of a
// thread-local address of an immutable constant that varies by thread.
struct Vconst {
  enum Kind { Quad, Long, Byte, ThreadLocal };
  struct Hash {
    size_t operator()(Vconst c) const {
      return std::hash<uint64_t>()(c.val) ^ std::hash<int>()(c.kind);
    }
  };
  Vconst() : kind(Quad), val(0) {}
  /* implicit */ Vconst(bool b) : kind(Byte), val(b) {}
  /* implicit */ Vconst(uint8_t b) : kind(Byte), val(b) {}
  /* implicit */ Vconst(uint32_t i) : kind(Long), val(i) {}
  /* implicit */ Vconst(uint64_t i) : kind(Quad), val(i) {}
  /* implicit */ Vconst(Vptr tl) : kind(ThreadLocal), disp(tl.disp) {
    assert(!tl.base.isValid() && !tl.index.isValid() && tl.seg == Vptr::FS);
  }

  bool operator==(Vconst other) const {
    return kind == other.kind && val == other.val;
  }

  Kind kind;
  union {
    uint64_t val;
    int64_t disp; // really, int32 offset from %fs
  };
};

/*
 * A Vunit contains all the assets that make up a vasm compilation unit. It is
 * responsible for allocating new blocks, Vregs, and tuples.
 */
struct Vunit {
  /*
   * Create a new block in the given area, returning its id.
   */
  Vlabel makeBlock(AreaIndex area);

  /*
   * Create a block intended to be used temporarily, as part of modifying
   * existing code. Although not necessary for correctness, the block may be
   * freed with freeScratchBlock when finished.
   */
  Vlabel makeScratchBlock();

  /*
   * Free a scratch block when finished with it. There must be no references to
   * this block in reachable code.
   */
  void freeScratchBlock(Vlabel);

  Vreg makeReg() { return Vreg{next_vr++}; }
  Vtuple makeTuple(VregList&& regs);
  Vtuple makeTuple(const VregList& regs);
  VcallArgsId makeVcallArgs(VcallArgs&& args);

  Vreg makeConst(bool);
  Vreg makeConst(uint32_t);
  Vreg makeConst(uint64_t);
  Vreg makeConst(double);
  Vreg makeConst(Vptr);
  Vreg makeConst(const void* p) { return makeConst(uint64_t(p)); }
  Vreg makeConst(int64_t v) { return makeConst(uint64_t(v)); }
  Vreg makeConst(int32_t v) { return makeConst(int64_t(v)); }
  Vreg makeConst(DataType t) { return makeConst(uint64_t(t)); }
  Vreg makeConst(Immed64 v) { return makeConst(uint64_t(v.q())); }

  template<class R, class... Args>
  Vreg makeConst(R (*fn)(Args...)) { return makeConst(CTCA(fn)); }

  template<class T>
  typename std::enable_if<std::is_integral<T>::value, Vreg>::type
  makeConst(T l) { return makeConst(uint64_t(l)); }

  /*
   * Returns true iff this Vunit needs register allocation before it can be
   * emitted, either because it uses virtual registers or contains instructions
   * that must be lowered by xls.
   */
  bool needsRegAlloc() const;

  unsigned next_vr{Vreg::V0};
  unsigned next_point{0};
  Vlabel entry;
  jit::vector<Vblock> blocks;

  jit::hash_map<Vconst,Vreg,Vconst::Hash> constants;
  jit::vector<VregList> tuples;
  jit::vector<VcallArgs> vcallArgs;
};

// writer stream to add instructions to a block
struct Vout {
  Vout(Vunit& u, Vlabel b, const IRInstruction* origin = nullptr)
    : m_unit(u), m_block(b), m_origin(origin)
  {}

  Vout(Vout&&) = default;
  Vout(const Vout&) = delete;

  Vout& operator=(const Vout& v) {
    assert(&v.m_unit == &m_unit);
    m_block = v.m_block;
    m_origin = v.m_origin;
    return *this;
  }

  Vout& operator=(Vlabel b) {
    m_block = b;
    return *this;
  }

  // implicit cast to label for initializing branch instructions
  /* implicit */ operator Vlabel() const;
  bool empty() const;
  bool closed() const;

  Vout makeBlock(); // create a stream connected to a new empty block

  // instruction emitter
  Vout& operator<<(const Vinstr& inst);

  Vpoint makePoint() { return Vpoint{m_unit.next_point++}; }
  Vunit& unit() { return m_unit; }
  template<class T> Vreg cns(T v) { return m_unit.makeConst(v); }
  void use(Vlabel b) { m_block = b; }
  void setOrigin(const IRInstruction* i) { m_origin = i; }
  Vreg makeReg() { return m_unit.makeReg(); }
  AreaIndex area() const { return m_unit.blocks[m_block].area; }
  Vtuple makeTuple(const VregList& regs) const {
    return m_unit.makeTuple(regs);
  }
  Vtuple makeTuple(VregList&& regs) const {
    return m_unit.makeTuple(std::move(regs));
  }
  VcallArgsId makeVcallArgs(VcallArgs&& args) const {
    return m_unit.makeVcallArgs(std::move(args));
  }

private:
  Vunit& m_unit;
  Vlabel m_block;
  const IRInstruction* m_origin;
};

// Similar to X64Assembler, but buffers instructions as they
// are written, then generates code all at once at the end.
// Areas represent the separate sections we generate code into;
struct Vasm {
  struct Area {
    Vout out;
    CodeBlock& code;
    CodeAddress start;
  };
  using AreaList = jit::vector<Area>;

  explicit Vasm() {
    m_areas.reserve(size_t(AreaIndex::Max));
  }

  void optimizeX64();
  void finishX64(const Abi&, AsmInfo* asmInfo);
  void finishARM(const Abi&, AsmInfo* asmInfo);

  // get an existing area
  Vout& main() { return area(AreaIndex::Main).out; }
  Vout& cold() { return area(AreaIndex::Cold).out; }
  Vout& frozen() { return area(AreaIndex::Frozen).out; }

  // create areas
  Vout& main(CodeBlock& cb) { return add(cb, AreaIndex::Main); }
  Vout& cold(CodeBlock& cb) { return add(cb, AreaIndex::Cold); }
  Vout& frozen(CodeBlock& cb) { return add(cb, AreaIndex::Frozen); }
  Vout& main(X64Assembler& a) { return main(a.code()); }
  Vout& cold(X64Assembler& a) { return cold(a.code()); }
  Vout& frozen(X64Assembler& a) { return frozen(a.code()); }
  Vunit& unit() { return m_unit; }
  AreaList& areas() { return m_areas; }

private:
  Vout& add(CodeBlock &cb, AreaIndex area);
  Area& area(AreaIndex i) {
    assert((unsigned)i < m_areas.size());
    return m_areas[(unsigned)i];
  }

private:
  Vunit m_unit;
  AreaList m_areas; // indexed by AreaIndex
};

/*
 * Vauto is a convenience helper for emitting small amounts of machine code
 * using vasm. It always has a main code block; cold and frozen blocks may be
 * added using the normal Vasm API after creation. When the Vauto goes out of
 * scope, it will finalize and emit any code it contains.
 */
struct Vauto : Vasm {
  explicit Vauto(CodeBlock& code) {
    unit().entry = Vlabel(main(code));
  }
  ~Vauto();
};

template<class F> void visit(const Vunit&, Vreg v, F f) {
  f(v);
}
template<class F> void visit(const Vunit&, Vptr p, F f) {
  if (p.base.isValid()) f(p.base);
  if (p.index.isValid()) f(p.index);
}
template<class F> void visit(const Vunit& unit, Vtuple t, F f) {
  for (auto r : unit.tuples[t]) f(r);
}
template<class F> void visit(const Vunit& unit, VcallArgsId a, F f) {
  auto& args = unit.vcallArgs[a];
  for (auto r : args.args) f(r);
  for (auto r : args.simdArgs) f(r);
  for (auto r : args.stkArgs) f(r);
}
template<class F> void visit(const Vunit& unit, RegSet regs, F f) {
  regs.forEach([&](Vreg r) { f(r); });
}

template<class Use>
void visitUses(const Vunit& unit, Vinstr& inst, Use use) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      uses \
      break; \
    }
#define U(s) visit(unit, i.s, use);
#define UA(s) visit(unit, i.s, use);
#define UH(s,h) visit(unit, i.s, use);
#define Un
    VASM_OPCODES
#undef Un
#undef UH
#undef UA
#undef U
#undef O
  }
}

template<class Def>
void visitDefs(const Vunit& unit, const Vinstr& inst, Def def) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      defs \
      break; \
    }
#define D(d) visit(unit, i.d, def);
#define DH(d,h) visit(unit, i.d, def);
#define Dn
    VASM_OPCODES
#undef Dn
#undef DH
#undef D
#undef O
  }
}

/*
 * visitOperands visits all operands of the given instruction, calling
 * visitor.imm(), visitor.use(), visitor.across(), and visitor.def() as defined
 * in the VASM_OPCODES macro.
 *
 * The template spew is necessary to support callers that only have a const
 * Vinstr& as well as callers with a Vinstr& that wish to mutate the
 * instruction in the visitor.
 */
template<class MaybeConstVinstr, class Visitor>
typename std::enable_if<
  std::is_same<MaybeConstVinstr, Vinstr>::value ||
  std::is_same<MaybeConstVinstr, const Vinstr>::value
>::type
visitOperands(MaybeConstVinstr& inst, Visitor& visitor) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      imms \
      uses \
      defs \
      break; \
    }
#define I(f) visitor.imm(i.f);
#define U(s) visitor.use(i.s);
#define UA(s) visitor.across(i.s);
#define UH(s,h) visitor.useHint(i.s, i.h);
#define D(d) visitor.def(i.d);
#define DH(d,h) visitor.defHint(i.d, i.h);
#define Inone
#define Un
#define Dn
    VASM_OPCODES
#undef Dn
#undef Un
#undef Inone
#undef DH
#undef D
#undef UH
#undef UA
#undef U
#undef I
#undef O
  }
}

extern const char* vinst_names[];

// search for the phidef in block b, then return its dest tuple
Vtuple findDefs(const Vunit& unit, Vlabel b);

typedef jit::vector<jit::vector<Vlabel>> PredVector;
PredVector computePreds(const Vunit& unit);

}}
#endif
