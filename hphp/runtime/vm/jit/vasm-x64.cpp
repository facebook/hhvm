/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-x64.h"

#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/base/arch.h"

#include <algorithm>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
using namespace reg;
using namespace x64;

namespace x64 { struct ImmFolder; }

#define O(name, ...)                                                    \
  static_assert(sizeof(name) <= 48, "vasm struct " #name " is too big");
VASM_OPCODES
#undef O
static_assert(sizeof(Vinstr) <= 64, "Vinstr should be <= 64 bytes");

const char* vinst_names[] = {
#define O(name, imms, uses, defs) #name,
  VASM_OPCODES
#undef O
};

Vlabel Vunit::makeBlock(AreaIndex area) {
  auto i = blocks.size();
  blocks.emplace_back(area);
  return Vlabel{i};
}

Vlabel Vunit::makeScratchBlock() {
  return makeBlock(AreaIndex::Main);
}

void Vunit::freeScratchBlock(Vlabel l) {
  // This will leak blocks if anything's been added since the corresponding
  // call to makeScratchBlock(), but it's harmless.
  if (l == blocks.size() - 1) blocks.pop_back();
}

Vreg Vunit::makeConst(uint64_t v) {
  auto it = constants.find(v);
  if (it != constants.end()) return it->second;
  return constants[v] = makeReg();
}

Vreg Vunit::makeConst(bool b) {
  auto it = constants.find(b);
  if (it != constants.end()) return it->second;
  return constants[b] = makeReg();
}

Vreg Vunit::makeConst(double d) {
  union { double d; uint64_t i; } u;
  u.d = d;
  return makeConst(u.i);
}

Vtuple Vunit::makeTuple(VregList&& regs) {
  auto i = tuples.size();
  tuples.emplace_back(std::move(regs));
  return Vtuple{i};
}

Vtuple Vunit::makeTuple(const VregList& regs) {
  auto i = tuples.size();
  tuples.emplace_back(regs);
  return Vtuple{i};
}

VcallArgsId Vunit::makeVcallArgs(VcallArgs&& args) {
  VcallArgsId i(vcallArgs.size());
  vcallArgs.emplace_back(std::move(args));
  return i;
}

bool Vunit::needsRegAlloc() const {
  if (next_vr > Vreg::V0) return true;

  for (auto& block : blocks) {
    for (auto& inst : block.code) {
      if (inst.op == Vinstr::copyargs) return true;
    }
  }

  return false;
}

Vout& Vout::operator<<(const Vinstr& inst) {
  assert(!closed());
  auto& code = m_unit.blocks[m_block].code;
  code.emplace_back(inst);
  code.back().origin = m_origin;
  FTRACE(6, "Vout << {}\n", show(m_unit, inst));
  return *this;
}

Vout Vout::makeBlock() {
  return {m_unit, m_unit.makeBlock(area()), m_origin};
}

// implicit cast to label for initializing branch instructions
Vout::operator Vlabel() const {
  return m_block;
}

bool Vout::empty() const {
  return m_unit.blocks[m_block].code.empty();
}

bool Vout::closed() const {
  return !empty() && isBlockEnd(m_unit.blocks[m_block].code.back());
}

namespace {
struct Vgen {
  Vgen(Vunit& u, Vasm::AreaList& areas, AsmInfo* asmInfo)
    : unit(u)
    , backend(mcg->backEnd())
    , areas(areas)
    , m_asmInfo(asmInfo) {
    addrs.resize(u.blocks.size());
    points.resize(u.next_point);
  }
  void emit(jit::vector<Vlabel>&);

private:
  template<class Inst> void emit(Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }
  // intrinsics
  void emit(bindaddr& i);
  void emit(bindcall& i);
  void emit(bindexit& i);
  void emit(bindjcc1st& i);
  void emit(bindjcc2nd& i);
  void emit(bindjmp& i);
  void emit(callstub& i);
  void emit(contenter& i);
  void emit(copy i);
  void emit(copy2& i);
  void emit(debugtrap& i) { a->int3(); }
  void emit(fallthru& i) {}
  void emit(ldimmb& i);
  void emit(ldimm& i);
  void emit(fallback& i);
  void emit(fallbackcc i);
  void emit(kpcall& i);
  void emit(ldpoint& i);
  void emit(load& i);
  void emit(mccall& i);
  void emit(mcprep& i);
  void emit(nothrow& i);
  void emit(point& i) { points[i.p] = a->frontier(); }
  void emit(store& i);
  void emit(syncpoint i);
  void emit(unwind& i);
  void emit(landingpad& i) {}

  // instructions
  void emit(andb& i) { commuteSF(i); a->andb(i.s0, i.d); }
  void emit(andbi& i) { binary(i); a->andb(i.s0, i.d); }
  void emit(andbim& i) { a->andb(i.s, i.m); }
  void emit(andl& i) { commuteSF(i); a->andl(i.s0, i.d); }
  void emit(andli& i) { binary(i); a->andl(i.s0, i.d); }
  void emit(andq& i) { commuteSF(i); a->andq(i.s0, i.d); }
  void emit(andqi& i) { binary(i); a->andq(i.s0, i.d); }
  void emit(addli& i) { binary(i); a->addl(i.s0, i.d); }
  void emit(addlm& i) { a->addl(i.s0, i.m); }
  void emit(addq& i) { commuteSF(i); a->addq(i.s0, i.d); }
  void emit(addqi& i) { binary(i); a->addq(i.s0, i.d); }
  void emit(addqim& i);
  void emit(addsd& i) { commute(i); a->addsd(i.s0, i.d); }
  void emit(call i);
  void emit(callm& i) { a->call(i.target); }
  void emit(callr& i) { a->call(i.target); }
  void emit(cloadq& i);
  void emit(cmovq i);
  void emit(cmpb& i) { a->cmpb(i.s0, i.s1); }
  void emit(cmpbi& i) { a->cmpb(i.s0, i.s1); }
  void emit(cmpbim& i) { a->cmpb(i.s0, i.s1); }
  void emit(cmpl& i) { a->cmpl(i.s0, i.s1); }
  void emit(cmpli& i) { a->cmpl(i.s0, i.s1); }
  void emit(cmplim& i) { a->cmpl(i.s0, i.s1); }
  void emit(cmplm& i) { a->cmpl(i.s0, i.s1); }
  void emit(cmpq& i) { a->cmpq(i.s0, i.s1); }
  void emit(cmpqi& i) { a->cmpq(i.s0, i.s1); }
  void emit(cmpqim& i) { a->cmpq(i.s0, i.s1); }
  void emit(cmpqm& i) { a->cmpq(i.s0, i.s1); }
  void emit(cmpsd& i) { noncommute(i); a->cmpsd(i.s0, i.d, i.pred); }
  void emit(cqo& i) { a->cqo(); }
  void emit(cvttsd2siq& i) { a->cvttsd2siq(i.s, i.d); }
  void emit(cvtsi2sd& i);
  void emit(cvtsi2sdm& i);
  void emit(decl& i) { unary(i); a->decl(i.d); }
  void emit(declm& i) { a->decl(i.m); }
  void emit(decq& i) { unary(i); a->decq(i.d); }
  void emit(decqm& i) { a->decq(i.m); }
  void emit(divsd& i) { noncommute(i); a->divsd(i.s0, i.d); }
  void emit(imul& i) { commuteSF(i); a->imul(i.s0, i.d); }
  void emit(idiv& i) { a->idiv(i.s); }
  void emit(incl& i) { unary(i); a->incl(i.d); }
  void emit(inclm& i) { a->incl(i.m); }
  void emit(incq& i) { unary(i); a->incq(i.d); }
  void emit(incqm& i) { a->incq(i.m); }
  void emit(incqmlock& i) { a->lock(); a->incq(i.m); }
  void emit(incwm& i) { a->incw(i.m); }
  void emit(jcc& i);
  void emit(jmp i);
  void emit(jmpr& i) { a->jmp(i.target); }
  void emit(jmpm& i) { a->jmp(i.target); }
  void emit(lea& i);
  void emit(leap& i) { a->lea(i.s, i.d); }
  void emit(loaddqu& i) { a->movdqu(i.s, i.d); }
  void emit(loadtqb& i) { a->loadb(i.s, i.d); }
  void emit(loadl& i) { a->loadl(i.s, i.d); }
  void emit(loadqp& i) { a->loadq(i.s, i.d); }
  void emit(loadsd& i) { a->movsd(i.s, i.d); }
  void emit(loadzbl& i) { a->loadzbl(i.s, i.d); }
  void emit(loadzbq& i) { a->loadzbl(i.s, Reg32(i.d)); }
  void emit(loadzlq& i) { a->loadl(i.s, Reg32(i.d)); }
  void emit(movb& i) { a->movb(i.s, i.d); }
  void emit(movl& i) { a->movl(i.s, i.d); }
  void emit(movzbl& i) { a->movzbl(i.s, i.d); }
  void emit(movzbq& i) { a->movzbl(i.s, Reg32(i.d)); }
  void emit(mulsd& i) { commute(i); a->mulsd(i.s0, i.d); }
  void emit(neg& i) { unary(i); a->neg(i.d); }
  void emit(nop& i) { a->nop(); }
  void emit(not& i) { unary(i); a->not(i.d); }
  void emit(orwim& i) { a->orw(i.s0, i.m); }
  void emit(orq& i) { commuteSF(i); a->orq(i.s0, i.d); }
  void emit(orqi& i) { binary(i); a->orq(i.s0, i.d); }
  void emit(orqim& i) { a->orq(i.s0, i.m); }
  void emit(pop& i) { a->pop(i.d); }
  void emit(popm& i) { a->pop(i.m); }
  void emit(psllq& i) { binary(i); a->psllq(i.s0, i.d); }
  void emit(psrlq& i) { binary(i); a->psrlq(i.s0, i.d); }
  void emit(push& i) { a->push(i.s); }
  void emit(pushm& i) { a->push(i.s); }
  void emit(roundsd& i) { a->roundsd(i.dir, i.s, i.d); }
  void emit(ret& i) { a->ret(); }
  void emit(sarq& i) { unary(i); a->sarq(i.d); }
  void emit(sarqi& i) { binary(i); a->sarq(i.s0, i.d); }
  void emit(setcc& i) { a->setcc(i.cc, i.d); }
  void emit(shlli& i) { binary(i); a->shll(i.s0, i.d); }
  void emit(shlq& i) { unary(i); a->shlq(i.d); }
  void emit(shlqi& i) { binary(i); a->shlq(i.s0, i.d); }
  void emit(shrli& i) { binary(i); a->shrl(i.s0, i.d); }
  void emit(shrqi& i) { binary(i); a->shrq(i.s0, i.d); }
  void emit(sqrtsd& i) { a->sqrtsd(i.s, i.d); }
  void emit(storedqu& i) { a->movdqu(i.s, i.m); }
  void emit(storeb& i) { a->storeb(i.s, i.m); }
  void emit(storebi& i) { a->storeb(i.s, i.m); }
  void emit(storel& i) { a->storel(i.s, i.m); }
  void emit(storeli& i) { a->storel(i.s, i.m); }
  void emit(storeqi& i) { a->storeq(i.s, i.m); }
  void emit(storesd& i) { a->movsd(i.s, i.m); }
  void emit(storew& i) { a->storew(i.s, i.m); }
  void emit(storewi& i) { a->storew(i.s, i.m); }
  void emit(subbi& i) { binary(i); a->subb(i.s0, i.d); }
  void emit(subl& i) { noncommute(i); a->subl(i.s0, i.d); }
  void emit(subli& i) { binary(i); a->subl(i.s0, i.d); }
  void emit(subq& i) { noncommute(i); a->subq(i.s0, i.d); }
  void emit(subqi& i) { binary(i); a->subq(i.s0, i.d); }
  void emit(subsd& i) { noncommute(i); a->subsd(i.s0, i.d); }
  void emit(testb& i) { a->testb(i.s0, i.s1); }
  void emit(testbi& i) { a->testb(i.s0, i.s1); }
  void emit(testbim i) { a->testb(i.s0, i.s1); }
  void emit(testwim& i);
  void emit(testl& i) { a->testl(i.s0, i.s1); }
  void emit(testli& i) { a->testl(i.s0, i.s1); }
  void emit(testlim i);
  void emit(testq& i) { a->testq(i.s0, i.s1); }
  void emit(testqm& i) { a->testq(i.s0, i.s1); }
  void emit(testqim& i);
  void emit(ucomisd& i) { a->ucomisd(i.s0, i.s1); }
  void emit(ud2& i) { a->ud2(); }
  void emit(unpcklpd& i) { noncommute(i); a->unpcklpd(i.s0, i.d); }
  void emit(xorb& i) { commuteSF(i); a->xorb(i.s0, i.d); }
  void emit(xorbi& i) { binary(i); a->xorb(i.s0, i.d); }
  void emit(xorq& i) { commuteSF(i); a->xorq(i.s0, i.d); }
  void emit(xorqi& i) { binary(i); a->xorq(i.s0, i.d); }

  // helpers
  void prep(Reg8 s, Reg8 d) { if (s != d) a->movb(s, d); }
  void prep(Reg32 s, Reg32 d) { if (s != d) a->movl(s, d); }
  void prep(Reg64 s, Reg64 d) { if (s != d) a->movq(s, d); }
  void prep(RegXMM s, RegXMM d) { if (s != d) a->movdqa(s, d); }
  CodeAddress start(Vlabel b) {
    auto area = unit.blocks[b].area;
    return areas[(int)area].start;
  }
  CodeBlock& main() { return area(AreaIndex::Main).code; }
  CodeBlock& cold() { return area(AreaIndex::Cold).code; }
  CodeBlock& frozen() { return area(AreaIndex::Frozen).code; }
  template<class Inst> void unary(Inst& i) { prep(i.s, i.d); }
  template<class Inst> void binary(Inst& i) { prep(i.s1, i.d); }
  template<class Inst> void commuteSF(Inst&);
  template<class Inst> void commute(Inst&);
  template<class Inst> void noncommute(Inst&);

private:
  Vasm::Area& area(AreaIndex i) {
    assert((unsigned)i < areas.size());
    return areas[(unsigned)i];
  }

private:
  struct LabelPatch { CodeAddress instr; Vlabel target; };
  struct PointPatch { CodeAddress instr; Vpoint pos; };
  Vunit& unit;
  BackEnd& backend;
  Vasm::AreaList& areas;
  AsmInfo* m_asmInfo;
  X64Assembler* a;
  Vlabel current{0}, next{0}; // in linear order
  jit::vector<CodeAddress> addrs, points;
  jit::vector<LabelPatch> jccs, jmps, calls, catches;
  jit::vector<PointPatch> ldpoints;
  jit::hash_map<uint64_t,uint64_t*> constants;
};

// prepare a binary op that is not commutative.  s0 must be a different
// register than s1 so we don't clobber it.
template<class Inst> void Vgen::noncommute(Inst& i) {
  assert(i.s1 == i.d || i.s0 != i.d); // do not clobber s0
  binary(i);
}

// prepare a binary op that is commutative. Swap operands if the dest is s0.
template<class Inst> void Vgen::commuteSF(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d, i.sf};
  } else {
    binary(i);
  }
}

// prepare a binary op that is commutative. Swap operands if the dest is s0.
template<class Inst> void Vgen::commute(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d};
  } else {
    binary(i);
  }
}

void Vgen::emit(addqim& i) {
  if (i.m.seg == Vptr::FS) a->fs();
  a->addq(i.s0, i.m.mr());
}

void Vgen::emit(call i) {
  // warning: this is a copy of emitCall(TCA) in code-gen-helpers-x64.cpp
  if (a->jmpDeltaFits(i.target)) {
    a->call(i.target);
  } else {
    // can't do a near call; store address in data section.
    // call by loading the address using rip-relative addressing.  This
    // assumes the data section is near the current code section.  Since
    // this sequence is directly in-line, rip-relative like this is
    // more compact than loading a 64-bit immediate.
    auto addr = mcg->allocLiteral((uint64_t)i.target);
    a->call(rip[(intptr_t)addr]);
  }
}

void Vgen::emit(cloadq& i) {
  auto m = i.t;
  always_assert(!m.index.isValid()); // not supported, but could be later.
  if (i.f != i.d) {
    always_assert(i.d != m.base); // don't clobber base
    a->movq(i.f, i.d);
  }
  a->cload_reg64_disp_reg64(i.cc, m.base, m.disp, i.d);
}

// add s0 s1 d => mov s1->d; d += s0
// cmov cc s d => if cc { mov s->d }
void Vgen::emit(cmovq i) {
  if (i.f != i.d && i.t == i.d) {
    // negate the condition and swap t/f operands so we dont clobber i.t
    i = {ccNegate(i.cc), i.sf, i.t, i.f, i.d};
  } else {
    prep(i.f, i.d);
  }
  a->cmov_reg64_reg64(i.cc, i.t, i.d);
}

void Vgen::emit(contenter& i) {
  Label Stub, End;
  Reg64 fp = i.fp, target = i.target;
  a->jmp8(End);

  asm_label(*a, Stub);
  a->pop(fp[AROFF(m_savedRip)]);
  a->jmp(target);

  asm_label(*a, End);
  a->call(Stub);
  // m_savedRip will point here.
}

void Vgen::emit(copy i) {
  if (i.s == i.d) return;
  if (i.s.isGP()) {
    if (i.d.isGP()) {                 // GP => GP
      a->movq(i.s, i.d);
    } else {                             // GP => XMM
      assert(i.d.isSIMD());
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      a->movq_rx(i.s, i.d);
    }
  } else {
    if (i.d.isGP()) {                 // XMM => GP
      a->movq_xr(i.s, i.d);
    } else {                             // XMM => XMM
      assert(i.d.isSIMD());
      // This copies all 128 bits in XMM,
      // thus avoiding partial register stalls
      a->movdqa(i.s, i.d);
    }
  }
}

void Vgen::emit(copy2& i) {
  assert(i.s0.isValid() && i.s1.isValid() && i.d0.isValid() && i.d1.isValid());
  auto s0 = i.s0, s1 = i.s1, d0 = i.d0, d1 = i.d1;
  assert(d0 != d1);
  if (d0 == s1) {
    if (d1 == s0) {
      a->xchgq(d0, d1);
    } else {
      // could do this in a simplify pass
      if (s1 != d1) a->movq(s1, d1); // save s1 first; d1 != s0
      if (s0 != d0) a->movq(s0, d0);
    }
  } else {
    // could do this in a simplify pass
    if (s0 != d0) a->movq(s0, d0);
    if (s1 != d1) a->movq(s1, d1);
  }
}

void Vgen::emit(bindaddr& i) {
  mcg->setJmpTransID((TCA)i.dest);
  *i.dest = emitEphemeralServiceReq(
    frozen(),
    mcg->getFreeStub(frozen(), &mcg->cgFixups()),
    REQ_BIND_ADDR,
    i.dest,
    i.sk.toAtomicInt(),
    TransFlags{}.packed
  );
  mcg->cgFixups().m_codePointers.insert(i.dest);
}

void Vgen::emit(bindcall& i) {
  mcg->backEnd().prepareForSmash(a->code(), kCallLen);
  a->call(i.stub);
}

void Vgen::emit(bindexit& i) {
  emitBindSideExit(a->code(), frozen(), i.cc, i.target, i.trflags);
}

void Vgen::emit(bindjcc1st& i) {
  backend.prepareForTestAndSmash(a->code(), 0,
                                 TestAndSmashFlags::kAlignJccAndJmp);
  auto const patchAddr = a->frontier();
  auto const jccStub =
    emitEphemeralServiceReq(frozen(),
                            mcg->getFreeStub(frozen(), &mcg->cgFixups()),
                            REQ_BIND_JMPCC_FIRST,
                            RipRelative(patchAddr),
                            i.targets[1],
                            i.targets[0],
                            i.cc,
                            ccServiceReqArgInfo(i.cc));

  mcg->setJmpTransID(a->frontier());
  a->jcc(i.cc, jccStub);
  mcg->setJmpTransID(a->frontier());
  a->jmp(jccStub);
}

void Vgen::emit(bindjcc2nd& i) {
  backend.prepareForSmash(a->code(), kJmpccLen);
  auto def = emitEphemeralServiceReq(frozen(),
                                     mcg->getFreeStub(frozen(),
                                                      &mcg->cgFixups()),
                                     REQ_BIND_JMPCC_SECOND,
                                     RipRelative(a->frontier()),
                                     i.target, i.cc);
  mcg->setJmpTransID(a->frontier());
  a->jcc(i.cc, def);
}

void Vgen::emit(bindjmp& i) {
  emitBindJmp(a->code(), frozen(), i.target, i.trflags);
}

void Vgen::emit(callstub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}

void Vgen::emit(fallback& i) {
  emit(fallbackcc{CC_None, InvalidReg, i.dest, i.trflags});
}

void Vgen::emit(fallbackcc i) {
  auto const destSR = mcg->tx().getSrcRec(i.dest);
  if (!i.trflags.packed) {
    destSR->emitFallbackJump(a->code(), i.cc);
  } else {
    destSR->emitFallbackJumpCustom(a->code(), frozen(), i.dest, i.trflags);
  }
}

void Vgen::emit(kpcall& i) {
  backend.prepareForSmash(a->code(), kCallLen);
  mcg->tx().profData()->addPrologueMainCaller(i.callee, i.prologIndex,
                                              a->frontier());
  always_assert(backend.isSmashable(a->frontier(), kCallLen));
  a->call(i.target);
}

void Vgen::emit(ldimmb& i) {
  auto val = i.s.b();
  assert_not_implemented(i.d.isGP());
  if (val == 0 && !i.saveflags) {
    a->xorb(i.d, i.d);
  } else {
    a->movb(val, i.d);
  }
}

void Vgen::emit(ldimm& i) {
  auto val = i.s.q();
  if (i.d.isGP()) {
    if (val == 0) {
      Reg64 d = i.d;
      if (i.saveflags) {
        a->movl(0, r32(d));
      } else {
        a->xorl(r32(d), r32(d));
      }
    } else {
      a->emitImmReg(i.s, i.d);
    }
  } else if (i.s.q() == 0) {
    a->pxor(i.d, i.d); // does not modify flags
  } else {
    auto addr = mcg->allocLiteral(i.s.q());
    a->movsd(rip[(intptr_t)addr], i.d);
  }
}

void Vgen::emit(ldpoint& i) {
  ldpoints.push_back({a->frontier(), i.s});
  a->lea(rip[0], i.d);
}

void Vgen::emit(load& i) {
  if (i.s.seg == Vptr::FS) a->fs();
  auto mref = i.s.mr();
  if (i.d.isGP()) {
    a->loadq(mref, i.d);
  } else {
    assert(i.d.isSIMD());
    a->movsd(mref, i.d);
  }
}

void Vgen::emit(mccall& i) {
  backend.prepareForSmash(a->code(), kCallLen);
  a->call(i.target);
}

// emit smashable mov as part of method cache callsite
void Vgen::emit(mcprep& i) {
  /*
   * For the first time through, set the cache to hold the address
   * of the movq (*2 + 1), so we can find the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * handlePrimeCacheMiss can tell it's not been smashed yet
   */
  backend.prepareForSmash(a->code(), MethodCache::kMovLen);
  auto movAddr = a->frontier();
  auto movAddrUInt = reinterpret_cast<uintptr_t>(movAddr);
  a->movq(0x8000000000000000u, i.d);
  auto after = reinterpret_cast<uintptr_t*>(a->frontier());
  after[-1] = (movAddrUInt << 1) | 1;
  mcg->cgFixups().m_addressImmediates.insert(
    reinterpret_cast<TCA>(~movAddrUInt));
}

void Vgen::emit(store& i) {
  if (i.s.isGP()) {
    a->storeq(i.s, i.d);
  } else {
    assert(i.s.isSIMD());
    a->movsd(i.s, i.d);
  }
}

void Vgen::emit(syncpoint i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(a->frontier(), i.fix.pcOffset,
                       i.fix.spOffset);
}

void Vgen::emit(testwim& i) {
  // If there's only 1 byte of meaningful bits in the mask, we can adjust the
  // pointer offset and use testbim instead.
  int off = 0;
  uint16_t newMask = i.s0.w();
  while (newMask > 0xff && !(newMask & 0xff)) {
    off++;
    newMask >>= 8;
  }

  if (newMask > 0xff) {
    a->testw(i.s0, i.s1);
  } else {
    emit(testbim{int8_t(newMask), i.s1 + off, i.sf});
  }
}

void Vgen::emit(testlim i) {
  a->testl(i.s0, i.s1);
}

void Vgen::emit(testqim& i) {
  // The immediate is 32 bits, sign-extended to 64. If the sign bit isn't set,
  // we can get the same results by emitting a testlim.
  if (i.s0.l() < 0) {
    a->testq(i.s0, i.s1);
  } else {
    emit(testlim{i.s0, i.s1, i.sf});
  }
}

void Vgen::emit(nothrow& i) {
  // register a null catch trace at this position, telling the unwinder that
  // the function call returning to here isn't allowed to throw.
  mcg->registerCatchBlock(a->frontier(), nullptr);
}

void Vgen::emit(unwind& i) {
  // Unwind instructions terminate blocks with calls that can throw, and have
  // the edges to catch (unwinder) blocks and fall-through blocks.
  catches.push_back({a->frontier(), i.targets[1]});
  emit(jmp{i.targets[0]});
}

// overall emitter
void Vgen::emit(jit::vector<Vlabel>& labels) {
  // Some structures here track where we put things just for debug printing.
  struct Snippet {
    const IRInstruction* origin;
    TcaRange range;
  };
  struct BlockInfo {
    jit::vector<Snippet> snippets;
  };

  // This is under the printir tracemod because it mostly shows you IR and
  // machine code, not vasm and machine code (not implemented).
  bool shouldUpdateAsmInfo = !!m_asmInfo;

  std::vector<TransBCMapping>* bcmap = nullptr;
  if (mcg->tx().isTransDBEnabled() || RuntimeOption::EvalJitUseVtuneAPI) {
    bcmap = &mcg->cgFixups().m_bcMap;
  }

  jit::vector<jit::vector<BlockInfo>> areaToBlockInfos;
  if (shouldUpdateAsmInfo) {
    areaToBlockInfos.resize(areas.size());
    for (auto& r : areaToBlockInfos) {
      r.resize(unit.blocks.size());
    }
  }

  for (int i = 0, n = labels.size(); i < n; ++i) {
    assert(checkBlockEnd(unit, labels[i]));

    auto b = labels[i];
    auto& block = unit.blocks[b];
    X64Assembler as { area(block.area).code };
    a = &as;
    auto blockStart = a->frontier();
    addrs[b] = blockStart;

    {
      // Compute the next block we will emit into the current area.
      auto cur_start = start(labels[i]);
      auto j = i + 1;
      while (j < labels.size() && cur_start != start(labels[j])) {
        j++;
      }
      next = j < labels.size() ? labels[j] : Vlabel(unit.blocks.size());
      current = b;
    }

    const IRInstruction* currentOrigin = nullptr;
    auto blockInfo = shouldUpdateAsmInfo
      ? &areaToBlockInfos[unsigned(block.area)][b]
      : nullptr;
    auto start_snippet = [&](Vinstr& inst) {
      if (!shouldUpdateAsmInfo) return;

      blockInfo->snippets.push_back(
        Snippet { inst.origin, TcaRange { a->code().frontier(), nullptr } }
      );
    };
    auto finish_snippet = [&] {
      if (!shouldUpdateAsmInfo) return;

      if (!blockInfo->snippets.empty()) {
        auto& snip = blockInfo->snippets.back();
        snip.range = TcaRange { snip.range.start(), a->code().frontier() };
      }
    };

    for (auto& inst : block.code) {
      if (currentOrigin != inst.origin) {
        finish_snippet();
        start_snippet(inst);
        currentOrigin = inst.origin;
      }

      if (bcmap && inst.origin) {
        auto sk = inst.origin->marker().sk();
        if (bcmap->empty() ||
            bcmap->back().md5 != sk.unit()->md5() ||
            bcmap->back().bcStart != sk.offset()) {
          bcmap->push_back(TransBCMapping{sk.unit()->md5(), sk.offset(),
                                          main().frontier(), cold().frontier(),
                                          frozen().frontier()});
        }
      }

      switch (inst.op) {
#define O(name, imms, uses, defs) \
        case Vinstr::name: emit(inst.name##_); break;
        VASM_OPCODES
#undef O
      }
    }

    finish_snippet();
  }

  for (auto& p : jccs) {
    assert(addrs[p.target]);
    X64Assembler::patchJcc(p.instr, addrs[p.target]);
  }
  for (auto& p : jmps) {
    assert(addrs[p.target]);
    X64Assembler::patchJmp(p.instr, addrs[p.target]);
  }
  for (auto& p : calls) {
    assert(addrs[p.target]);
    X64Assembler::patchCall(p.instr, addrs[p.target]);
  }
  for (auto& p : catches) {
    mcg->registerCatchBlock(p.instr, addrs[p.target]);
  }
  for (auto& p : ldpoints) {
    auto after_lea = p.instr + 7;
    auto d = points[p.pos] - after_lea;
    assert(deltaFits(d, sz::dword));
    ((int32_t*)after_lea)[-1] = d;
  }

  if (!shouldUpdateAsmInfo) {
    return;
  }

  for (auto i = 0; i < areas.size(); ++i) {
    auto& blockInfos = areaToBlockInfos[i];
    for (auto const blockID : labels) {
      auto const& blockInfo = blockInfos[static_cast<size_t>(blockID)];
      if (blockInfo.snippets.empty()) continue;

      const IRInstruction* currentOrigin = nullptr;
      for (auto const& snip : blockInfo.snippets) {
        if (currentOrigin != snip.origin && snip.origin) {
          currentOrigin = snip.origin;
        }

        m_asmInfo->updateForInstruction(
          currentOrigin,
          static_cast<AreaIndex>(i),
          snip.range.start(),
          snip.range.end());
      }
    }
  }
}

void Vgen::emit(cvtsi2sd& i) {
  a->pxor(i.d, i.d);
  a->cvtsi2sd(i.s, i.d);
}

void Vgen::emit(cvtsi2sdm& i) {
  a->pxor(i.d, i.d);
  a->cvtsi2sd(i.s, i.d);
}

void Vgen::emit(jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      i = jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}};
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});
    a->jcc(i.cc, a->frontier());
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(jmp i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  a->jmp(a->frontier());
}

void Vgen::emit(lea& i) {
  // could do this in a simplify pass
  if (i.s.disp == 0 && i.s.base.isValid() && !i.s.index.isValid()) {
    emit(copy{i.s.base, i.d});
  } else {
    a->lea(i.s, i.d);
  }
}

}

Vout& Vasm::add(CodeBlock& cb, AreaIndex area) {
  assert(size_t(area) == m_areas.size());
  auto b = m_unit.makeBlock(area);
  Vout v{m_unit, b};
  m_areas.push_back(Area{v, cb, cb.frontier()});
  return m_areas.back().out;
}

jit::vector<Vlabel> layoutBlocks(const Vunit& unit) {
  auto blocks = sortBlocks(unit);
  // partition into main/cold/frozen areas without changing relative order,
  // and the end{} block will be last.
  auto coldIt = std::stable_partition(blocks.begin(), blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Main &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  std::stable_partition(coldIt, blocks.end(),
    [&](Vlabel b) {
      return unit.blocks[b].area == AreaIndex::Cold &&
             unit.blocks[b].code.back().op != Vinstr::fallthru;
    });
  return blocks;
}

/*
 * Move all the elements of in into out, replacing count elements of out
 * starting at idx. in be will be cleared at the end.
 *
 * Example: vector_splice([1, 2, 3, 4, 5], 2, 1, [10, 11, 12]) will change out
 * to [1, 2, 10, 11, 12, 4, 5].
 */
template<typename V>
static void vector_splice(V& out, size_t idx, size_t count, V& in) {
  auto out_size = out.size();

  // Start by making room in out for the new elements.
  out.resize(out.size() + in.size() - count);

  // Move everything after the to-be-overwritten elements to the new end.
  std::move_backward(out.begin() + idx + count, out.begin() + out_size,
                     out.end());

  // Move the new elements in
  std::move(in.begin(), in.end(), out.begin() + idx);
  in.clear();
}

// Lower svcreq{} by making copies to abi registers explicit, saving
// vm regs, and returning to the VM. svcreq{} is guaranteed to be
// at the end of a block, so we can just keep appending to the same
// block.
static void lower_svcreq(Vunit& unit, Vlabel b, const Vinstr& inst) {
  assert(unit.tuples[inst.svcreq_.args].size() < kNumServiceReqArgRegs);
  auto svcreq = inst.svcreq_; // copy it
  auto origin = inst.origin;
  auto& argv = unit.tuples[svcreq.args];
  unit.blocks[b].code.pop_back(); // delete the svcreq instruction
  Vout v(unit, b, origin);

  RegSet arg_regs = kCrossTraceRegs;
  VregList arg_dests;
  for (int i = 0, n = argv.size(); i < n; ++i) {
    PhysReg d{serviceReqArgRegs[i]};
    arg_dests.push_back(d);
    arg_regs |= d;
  }
  v << copyargs{svcreq.args, v.makeTuple(arg_dests)};
  if (svcreq.stub_block) {
    v << leap{rip[(int64_t)svcreq.stub_block], rAsm};
  } else {
    v << ldimm{0, rAsm}; // because persist flag
  }
  v << ldimm{svcreq.req, rdi};
  arg_regs |= rAsm | rdi | rVmFp | rVmSp;

  // Weird hand-shaking with enterTC: reverse-call a service routine.
  // In the case of some special stubs (m_callToExit, m_retHelper), we
  // have already unbalanced the return stack by doing a ret to
  // something other than enterTCHelper.  In that case
  // SRJmpInsteadOfRet indicates to fake the return.
  v << ret{arg_regs};
}

static void lowerSrem(Vunit& unit, Vlabel b, size_t iInst) {
  auto const& inst = unit.blocks[b].code[iInst];
  auto const& srem = inst.srem_;
  auto scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, inst.origin);
  v << copy{srem.s0, rax};
  v << cqo{};                      // sign-extend rax => rdx:rax
  v << idiv{srem.s1, v.makeReg()}; // rdx:rax/divisor => quot:rax, rem:rdx
  v << copy{rdx, srem.d};

  vector_splice(unit.blocks[b].code, iInst, 1, unit.blocks[scratch].code);
}

template<typename FromOp, typename ToOp>
static void lowerShift(Vunit& unit, Vlabel b, size_t iInst) {
  auto const& inst = unit.blocks[b].code[iInst];
  auto const& shift = inst.get<FromOp>();
  auto scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, inst.origin);
  v << copy{shift.s0, rcx};
  v << ToOp{shift.s1, shift.d, shift.sf};

  vector_splice(unit.blocks[b].code, iInst, 1, unit.blocks[scratch].code);
}

static void lowerAbsdbl(Vunit& unit, Vlabel b, size_t iInst) {
  auto const& inst = unit.blocks[b].code[iInst];
  auto const& absdbl = inst.absdbl_;
  auto scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, inst.origin);

  // clear the high bit
  auto tmp = v.makeReg();
  v << psllq{1, absdbl.s, tmp};
  v << psrlq{1, tmp, absdbl.d};

  vector_splice(unit.blocks[b].code, iInst, 1, unit.blocks[scratch].code);
}

static void lowerVcall(Vunit& unit, Vlabel b, size_t iInst) {
  auto& blocks = unit.blocks;
  auto& inst = blocks[b].code[iInst];
  auto const is_vcall = inst.op == Vinstr::vcall;
  auto const vcall = inst.vcall_;
  auto const vinvoke = inst.vinvoke_;

  // Extract all the relevant information from the appropriate instruction.
  auto const is_smashable = !is_vcall && vinvoke.smashable;
  auto const call = is_vcall ? vcall.call : vinvoke.call;
  auto const& vargs = unit.vcallArgs[is_vcall ? vcall.args : vinvoke.args];
  auto const& stkArgs = vargs.stkArgs;
  auto const dests = unit.tuples[is_vcall ? vcall.d : vinvoke.d];
  auto const fixup = is_vcall ? vcall.fixup : vinvoke.fixup;
  auto const destType = is_vcall ? vcall.destType : vinvoke.destType;

  auto scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, inst.origin);

  int32_t const adjust = (stkArgs.size() & 0x1) ? sizeof(uintptr_t) : 0;
  if (adjust) v << subqi{adjust, reg::rsp, reg::rsp, v.makeReg()};

  // Push stack arguments, in reverse order.
  for (int i = stkArgs.size() - 1; i >= 0; --i) v << push{stkArgs[i]};

  // Get the arguments in the proper registers.
  RegSet argRegs;
  auto doArgs = [&](const VregList& srcs, const PhysReg argNames[]) {
    VregList argDests;
    for (size_t i = 0; i < srcs.size(); ++i) {
      auto reg = argNames[i];
      argDests.push_back(reg);
      argRegs |= reg;
    }
    if (argDests.size()) {
      v << copyargs{v.makeTuple(srcs),
                    v.makeTuple(std::move(argDests))};
    }
  };
  doArgs(vargs.args, argNumToRegName);
  doArgs(vargs.simdArgs, argNumToSIMDRegName);

  // Emit the call.
  if (is_smashable) v << mccall{(TCA)call.address(), argRegs};
  else              emitCall(v, call, argRegs);

  // Handle fixup and unwind information.
  if (fixup.isValid()) v << syncpoint{fixup};

  if (!is_vcall) {
    auto& targets = vinvoke.targets;
    v << unwind{{targets[0], targets[1]}};

    // Insert an lea fixup for any stack args at the beginning of the catch
    // block.
    if (auto rspOffset = ((stkArgs.size() + 1) & ~1) * sizeof(uintptr_t)) {
      auto& taken = unit.blocks[targets[1]].code;
      assert(taken.front().op == Vinstr::landingpad);
      Vinstr v{lea{rsp[rspOffset], rsp}};
      v.origin = taken.front().origin;
      taken.insert(taken.begin() + 1, v);
    }

    // Write out the code so far to the end of b. Remaining code will be
    // emitted to the next block.
    vector_splice(blocks[b].code, iInst, 1, blocks[scratch].code);
  } else if (vcall.nothrow) {
    v << nothrow{};
  }

  // Copy the call result to the destination register(s)
  switch (destType) {
    case DestType::TV: {
      // rax contains m_type and m_aux but we're expecting just the type in
      // the lower bits, so shift the type result register.
      auto rval = packed_tv ? reg::rdx : reg::rax;
      auto rtyp = packed_tv ? reg::rax : reg::rdx;
      if (kTypeShiftBits > 0) {
        v << shrqi{kTypeShiftBits, rtyp, rtyp, v.makeReg()};
      }
      assert(dests.size() == 2);
      v << copy2{rval, rtyp, dests[0], dests[1]};
      break;
    }
    case DestType::SIMD: {
      // rax contains m_type and m_aux but we're expecting just the type in
      // the lower bits, so shift the type result register.
      auto rval = packed_tv ? reg::rdx : reg::rax;
      auto rtyp = packed_tv ? reg::rax : reg::rdx;
      if (kTypeShiftBits > 0) {
        v << shrqi{kTypeShiftBits, rtyp, rtyp, v.makeReg()};
      }
      assert(dests.size() == 1);
      pack2(v, rval, rtyp, dests[0]);
      break;
    }
    case DestType::SSA:
    case DestType::Byte:
      // copy the single-register result to dests[0]
      assert(dests.size() == 1);
      assert(dests[0].isValid());
      v << copy{reg::rax, dests[0]};
      break;
    case DestType::None:
      assert(dests.empty());
      break;
    case DestType::Dbl:
      // copy the single-register result to dests[0]
      assert(dests.size() == 1);
      assert(dests[0].isValid());
      v << copy{reg::xmm0, dests[0]};
      break;
  }

  if (stkArgs.size() > 0) {
    v << addqi{safe_cast<int32_t>(stkArgs.size() * sizeof(uintptr_t)
                                  + adjust),
               reg::rsp,
               reg::rsp,
               v.makeReg()};
  }

  // Insert new instructions to the appropriate block
  if (is_vcall) {
    vector_splice(blocks[b].code, iInst, 1, blocks[scratch].code);
  } else {
    vector_splice(blocks[vinvoke.targets[0]].code, 0, 0,
                  blocks[scratch].code);
  }
}

/*
 * Lower a few abstractions to facilitate straightforward x64 codegen.
 */
static void lowerForX64(Vunit& unit, const Abi& abi) {
  Timer _t(Timer::vasm_lower);

  // Scratch block can change blocks allocation, hence cannot use regular
  // iterators.
  auto& blocks = unit.blocks;
  for (size_t ib = 0; ib < blocks.size(); ++ib) {
    if (blocks[ib].code.empty()) continue;
    if (blocks[ib].code.back().op == Vinstr::svcreq) {
      lower_svcreq(unit, Vlabel{ib}, blocks[ib].code.back());
    }
    for (size_t ii = 0; ii < blocks[ib].code.size(); ++ii) {
      auto& inst = blocks[ib].code[ii];
      switch (inst.op) {
        case Vinstr::vcall:
        case Vinstr::vinvoke:
          lowerVcall(unit, Vlabel{ib}, ii);
          break;

        case Vinstr::srem:
          lowerSrem(unit, Vlabel{ib}, ii);
          break;

        case Vinstr::sar:
          lowerShift<sar, sarq>(unit, Vlabel{ib}, ii);
          break;

        case Vinstr::shl:
          lowerShift<shl, shlq>(unit, Vlabel{ib}, ii);
          break;

        case Vinstr::absdbl:
          lowerAbsdbl(unit, Vlabel{ib}, ii);
          break;

        case Vinstr::defvmsp:
          inst = copy{rVmSp, inst.defvmsp_.d};
          break;

        case Vinstr::syncvmsp:
          inst = copy{inst.syncvmsp_.s, rVmSp};
          break;

        case Vinstr::movtqb:
          inst = copy{inst.movtqb_.s, inst.movtqb_.d};
          break;

        case Vinstr::movtql:
          inst = copy{inst.movtql_.s, inst.movtql_.d};
          break;

        case Vinstr::ldretaddr:
          inst = pushm{inst.ldretaddr_.s};
          break;

        case Vinstr::movretaddr:
          inst = load{*rsp, inst.movretaddr_.d};
          break;

        case Vinstr::retctrl:
          inst = ret{kCrossTraceRegs};
          break;

        default:
          break;
      }
    }
  }

  printUnit(kVasmLowerLevel, "after lower for X64", unit);
}

void Vasm::finishX64(const Abi& abi, AsmInfo* asmInfo) {
  static thread_local bool busy;
  always_assert(!busy);
  busy = true;
  SCOPE_EXIT { busy = false; };
  lowerForX64(m_unit, abi);

  if (!m_unit.constants.empty()) {
    foldImms<x64::ImmFolder>(m_unit);
  }
  if (m_unit.needsRegAlloc()) {
    Timer _t(Timer::vasm_xls);
    removeDeadCode(m_unit);
    allocateRegisters(m_unit, abi);
  }
  if (m_unit.blocks.size() > 1) {
    Timer _t(Timer::vasm_jumps);
    optimizeJmps(m_unit);
  }

  Timer _t(Timer::vasm_gen);
  auto blocks = layoutBlocks(m_unit);
  Vgen(m_unit, m_areas, asmInfo).emit(blocks);
}

auto const vauto_gp = rAsm | r11;
auto const vauto_simd = xmm5 | xmm6 | xmm7;
UNUSED const Abi vauto_abi {
  .gpUnreserved = vauto_gp,
  .gpReserved = x64::abi.gp() - vauto_gp,
  .simdUnreserved = vauto_simd,
  .simdReserved = x64::abi.simd() - vauto_simd,
  .calleeSaved = x64::abi.calleeSaved,
  .sf = x64::abi.sf
};

Vauto::~Vauto() {
  UNUSED auto& areas = this->areas();
  for (auto& b : unit().blocks) {
    if (!b.code.empty()) {
      // found at least one nonempty block. finish up.
      if (!main().closed()) main() << fallthru{};
      assert(areas.size() < 2 || cold().empty() || cold().closed());
      assert(areas.size() < 3 || frozen().empty() || frozen().closed());
      Trace::Bump bumper{Trace::printir, 10}; // prevent spurious printir
      switch (arch()) {
        case Arch::X64: finishX64(vauto_abi, nullptr); break;
        case Arch::ARM: finishARM(vauto_abi, nullptr); break;
      }
      return;
    }
  }
}

Vtuple findDefs(const Vunit& unit, Vlabel b) {
  assert(!unit.blocks[b].code.empty() &&
         unit.blocks[b].code.front().op == Vinstr::phidef);
  return unit.blocks[b].code.front().phidef_.defs;
}

}}
