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
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/timer.h"

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit { namespace x64 {
using namespace reg;

const char* vinst_names[] = {
#define O(name, imms, uses, defs) #name,
  X64_OPCODES
#undef O
};

bool isBlockEnd(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::bindaddr:
    case Vinstr::bindjcc1:
    case Vinstr::bindjmp:
    case Vinstr::end:
    case Vinstr::fallback:
    case Vinstr::jcc:
    case Vinstr::jmp:
    case Vinstr::jmpr:
    case Vinstr::jmpm:
    case Vinstr::phijmp:
    case Vinstr::resume:
    case Vinstr::ud2:
    case Vinstr::unwind:
    case Vinstr::retransopt:
    case Vinstr::ret:
      return true;
    default:
      return false;
  }
}

Vlabel Vunit::makeBlock(AreaIndex area) {
  auto i = blocks.size();
  blocks.emplace_back(area);
  return Vlabel{i};
}

Vreg Vunit::makeConst(uint64_t v) {
  auto it = cpool.find(v);
  if (it != cpool.end()) return it->second;
  auto r = makeReg();
  cpool[v] = r;
  return r;
}

Vreg Vunit::makeConst(double d) {
  union { double d; uint64_t i; } u;
  u.d = d;
  return makeConst(u.i);
}

Vtuple Vunit::makeTuple(const VregList& regs) {
  auto i = tuples.size();
  tuples.emplace_back(regs);
  return Vtuple{i};
}

Vout& Vout::operator<<(Vinstr inst) {
  assert(!closed());
  inst.sk = m_sk;
  m_unit.blocks[m_block].code.push_back(inst);
  return *this;
}

Vout Vout::makeBlock() {
  return {m_meta, m_unit, m_unit.makeBlock(m_area), m_area, m_sk};
}

Vout Vout::makeEntry() {
  auto label = m_unit.makeBlock(m_area);
  m_unit.roots.push_back(label); // save entry label
  return {m_meta, m_unit, label, m_area, m_sk};
}

// implicit cast to label for initializing branch instructions
Vout::operator Vlabel() const {
  assert(empty());
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
  Vgen(Vunit& u, jit::vector<Vasm::Area>& areas, Vmeta* meta)
    : unit(u)
    , backend(mcg->backEnd())
    , areas(areas)
    , meta(meta)
    , addrs(u.blocks.size(), nullptr)
  {}
  void emit(jit::vector<Vlabel>&);

private:
  // intrinsics
  void emit(bindaddr& i);
  void emit(bindcall& i);
  void emit(bindexit& i);
  void emit(bindjcc1& i);
  void emit(bindjcc2& i);
  void emit(bindjmp& i);
  void emit(callstub& i);
  void emit(contenter& i);
  void emit(copy i);
  void emit(copy2& i);
  void emit(copyargs& i) { always_assert(false); }
  void emit(end& i) {}
  void emit(ldimm& i);
  void emit(fallback& i);
  void emit(fallbackcc i);
  void emit(incstat& i) { emitIncStat(a->code(), i.stat, i.n, i.force); }
  void emit(kpcall& i);
  void emit(ldpoint& i);
  void emit(load& i);
  void emit(mccall& i);
  void emit(mcprep& i);
  void emit(nocatch& i);
  void emit(nop& i) { a->nop(); }
  void emit(phidef& i) { always_assert(false); }
  void emit(phijmp& i) { always_assert(false); }
  void emit(point& i) { meta->points[i.p] = a->frontier(); }
  void emit(resume& i) { emitServiceReq(a->code(), REQ_RESUME); }
  void emit(retransopt& i);
  void emit(store& i);
  void emit(syncpoint i);
  void emit(unwind& i);

  // instructions
  void emit(andb& i) { commute(i); a->andb(i.s0, i.d); }
  void emit(andbi& i) { binary(i); a->andb(i.s0, i.d); }
  void emit(andbim& i) { a->andb(i.s, i.m); }
  void emit(andl& i) { commute(i); a->andl(i.s0, i.d); }
  void emit(andli& i) { binary(i); a->andl(i.s0, i.d); }
  void emit(andq& i) { commute(i); a->andq(i.s0, i.d); }
  void emit(andqi& i) { binary(i); a->andq(i.s0, i.d); }
  void emit(addlm& i) { a->addl(i.s0, i.m); }
  void emit(addq& i) { commute(i); a->addq(i.s0, i.d); }
  void emit(addqi& i) { binary(i); a->addq(i.s0, i.d); }
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
  void emit(imul& i) { commute(i); a->imul(i.s0, i.d); }
  void emit(incwm& i) { a->incw(i.m); }
  void emit(idiv& i) { a->idiv(i.s); }
  void emit(incl& i) { unary(i); a->incl(i.d); }
  void emit(inclm& i) { a->incl(i.m); }
  void emit(incq& i) { unary(i); a->incq(i.d); }
  void emit(incqm& i) { a->incq(i.m); }
  void emit(incqmlock& i) { a->lock(); a->incq(i.m); }
  void emit(jcc& i);
  void emit(jmp i);
  void emit(jmpr& i) { a->jmp(i.target); }
  void emit(jmpm& i) { a->jmp(i.target); }
  void emit(lea& i);
  void emit(leap& i) { a->lea(i.s, i.d); }
  void emit(loaddqu& i) { a->movdqu(i.s, i.d); }
  void emit(loadl& i) { a->loadl(i.s, i.d); }
  void emit(loadq& i);
  void emit(loadqp& i) { a->loadq(i.s, i.d); }
  void emit(loadsd& i) { a->movsd(i.s, i.d); }
  void emit(loadzbl& i) { a->loadzbl(i.s, i.d); }
  void emit(movb& i) { a->movb(i.s, i.d); }
  void emit(movbi& i) { a->movb(i.s, i.d); }
  void emit(movdqa& i) { a->movdqa(i.s, i.d); }
  void emit(movl& i) { a->movl(i.s, i.d); }
  void emit(movq& i) { a->movq(i.s, i.d); }
  void emit(movqrx& i) { a->movq_rx(i.s, i.d); }
  void emit(movqxr& i) { a->movq_xr(i.s, i.d); }
  void emit(movzbl& i) { a->movzbl(i.s, i.d); }
  void emit(movsbl& i) { a->movsbl(i.s, i.d); }
  void emit(mulsd& i) { commute(i); a->mulsd(i.s0, i.d); }
  void emit(neg& i) { unary(i); a->neg(i.d); }
  void emit(not& i) { unary(i); a->not(i.d); }
  void emit(orq& i) { commute(i); a->orq(i.s0, i.d); }
  void emit(orqi& i) { binary(i); a->orq(i.s0, i.d); }
  void emit(orqim& i) { a->orq(i.s0, i.m); }
  void emit(pop& i) { a->pop(i.d); }
  void emit(popm& i) { a->pop(i.m); }
  void emit(psllq& i) { binary(i); a->psllq(i.s0, i.d); }
  void emit(psrlq& i) { binary(i); a->psrlq(i.s0, i.d); }
  void emit(push& i) { a->push(i.s); }
  void emit(pushl& i) { a->pushl(i.s); }
  void emit(pushm& i) { a->push(i.s); }
  void emit(roundsd& i) { unary(i); a->roundsd(i.dir, i.s, i.d); }
  void emit(ret& i) { a->ret(); }
  void emit(rorqi& i) { binary(i); a->rorq(i.s0, i.d); }
  void emit(sarq& i) { unary(i); a->sarq(i.d); }
  void emit(sarqi& i) { binary(i); a->sarq(i.s0, i.d); }
  void emit(sbbl& i) { noncommute(i); a->sbbl(i.s0, i.d); }
  void emit(setcc& i) { a->setcc(i.cc, i.d); }
  void emit(shlli& i) { binary(i); a->shll(i.s0, i.d); }
  void emit(shlq& i) { unary(i); a->shlq(i.d); }
  void emit(shlqi& i) { binary(i); a->shlq(i.s0, i.d); }
  void emit(shrli& i) { binary(i); a->shrl(i.s0, i.d); }
  void emit(shrqi& i) { binary(i); a->shrq(i.s0, i.d); }
  void emit(sqrtsd& i) { a->sqrtsd(i.s, i.d); }
  void emit(storedqu& i) { a->movdqu(i.s, i.m); }
  void emit(storeb& i) { a->storeb(i.s, i.m); }
  void emit(storebim& i) { a->storeb(i.s, i.m); }
  void emit(storel& i) { a->storel(i.s, i.m); }
  void emit(storelim& i) { a->storel(i.s, i.m); }
  void emit(storeq& i) { a->storeq(i.s, i.m); }
  void emit(storeqim& i) { a->storeq(i.s, i.m); }
  void emit(storesd& i) { a->movsd(i.s, i.m); }
  void emit(storew& i) { a->storew(i.s, i.m); }
  void emit(storewim& i) { a->storew(i.s, i.m); }
  void emit(subl& i) { noncommute(i); a->subl(i.s0, i.d); }
  void emit(subli& i) { binary(i); a->subl(i.s0, i.d); }
  void emit(subq& i) { noncommute(i); a->subq(i.s0, i.d); }
  void emit(subqi& i) { binary(i); a->subq(i.s0, i.d); }
  void emit(subsd& i) { noncommute(i); a->subsd(i.s0, i.d); }
  void emit(testb& i) { a->testb(i.s0, i.s1); }
  void emit(testbi& i) { a->testb(i.s0, i.s1); }
  void emit(testbim& i) { a->testb(i.s0, i.s1); }
  void emit(testl& i) { a->testl(i.s0, i.s1); }
  void emit(testli& i) { a->testl(i.s0, i.s1); }
  void emit(testlim& i) { a->testl(i.s0, i.s1); }
  void emit(testq& i) { a->testq(i.s0, i.s1); }
  void emit(testqm& i) { a->testq(i.s0, i.s1); }
  void emit(testqim& i) { a->testq(i.s0, i.s1); }
  void emit(ucomisd& i) { a->ucomisd(i.s0, i.s1); }
  void emit(ud2& i) { a->ud2(); }
  void emit(unpcklpd& i) { noncommute(i); a->unpcklpd(i.s0, i.d); }
  void emit(xorb& i) { commute(i); a->xorb(i.s0, i.d); }
  void emit(xorbi& i) { binary(i); a->xorb(i.s0, i.d); }
  void emit(xorq& i) { commute(i); a->xorq(i.s0, i.d); }
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
  bool check(Vblock& block);
  CodeBlock& main() { return area(AreaIndex::Main).code; }
  CodeBlock& cold() { return area(AreaIndex::Cold).code; }
  CodeBlock& frozen() { return area(AreaIndex::Frozen).code; }
  template<class Inst> void unary(Inst& i) { prep(i.s, i.d); }
  template<class Inst> void binary(Inst& i) { prep(i.s1, i.d); }
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
  jit::vector<Vasm::Area>& areas;
  Vmeta* meta;
  X64Assembler* a;
  Vlabel next{0}; // in linear order
  jit::vector<CodeAddress> addrs;
  jit::vector<LabelPatch> jccs, jmps, calls, catches;
  jit::vector<PointPatch> ldpoints;
  jit::hash_map<uint64_t,uint64_t*> cpool;
};

// prepare a binary op that is not commutative.  s0 must be a different
// register than s1 so we don't clobber it.
template<class Inst> void Vgen::noncommute(Inst& i) {
  assert(i.s1 == i.d || i.s0 != i.d); // do not clobber s0
  binary(i);
}

// prepare a binary op that is commutative. Swap operands if the dest is s0.
template<class Inst> void Vgen::commute(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d};
  } else {
    binary(i);
  }
}

void Vgen::emit(call i) {
  if (a->jmpDeltaFits(i.target) && !Stats::enabled()) {
    a->call(i.target);
  } else {
    // can't do a near call; store address in data section.
    // call by loading the address using rip-relative addressing.  This
    // assumes the data section is near the current code section.  Since
    // this sequence is directly in-line, rip-relative like this is
    // more compact than loading a 64-bit immediate.
    auto addr = mcg->allocLiteral((uint64_t)i.target);
    a->call(rip[(intptr_t)addr]);
    assert(((int32_t*)a->frontier())[-1] + a->frontier() == (TCA)addr);
  }
}

void Vgen::emit(cloadq& i) {
  a->cload_reg64_disp_reg64(i.cc, i.s.base, i.s.disp, i.d);
}

// add s0 s1 d => mov s1->d; d += s0
// cmov cc s d => if cc { mov s->d }
void Vgen::emit(cmovq i) {
  if (i.f != i.d && i.t == i.d) {
    // negate the condition and swap t/f operands so we dont clobber i.t
    i = {ccNegate(i.cc), i.t, i.f, i.d};
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
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      a->movq_rx(i.s, i.d);
    }
  } else {
    if (i.d.isGP()) {                 // XMM => GP
      a->movq_xr(i.s, i.d);
    } else {                             // XMM => XMM
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
  *i.dest = emitEphemeralServiceReq(a->code(),
                           mcg->getFreeStub(a->code(), &mcg->cgFixups()),
                           REQ_BIND_ADDR,
                           i.dest,
                           i.sk.toAtomicInt(),
                           TransFlags{}.packed);
  mcg->cgFixups().m_codePointers.insert(i.dest);
}

void Vgen::emit(bindcall& i) {
  emitBindCall(a->code(), cold(), frozen(), i.sk, i.callee, i.argc);
}

void Vgen::emit(bindexit& i) {
  emitBindSideExit(a->code(), frozen(), i.cc, i.target, i.trflags);
}

void Vgen::emit(bindjcc1& i) {
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

void Vgen::emit(bindjcc2& i) {
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
  emit(fallbackcc{CC_None, i.dest, i.trflags});
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

void Vgen::emit(retransopt& i) {
  emitServiceReq(a->code(), REQ_RETRANSLATE_OPT,
                 i.sk.toAtomicInt(), i.id);
}

void Vgen::emit(store& i) {
  if (i.s.isGP()) {
    a->storeq(i.s, i.d);
  } else {
    a->movsd(i.s, i.d);
  }
}

void Vgen::emit(syncpoint i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(a->frontier(), i.fix.pcOffset,
                       i.fix.spOffset);
}

void Vgen::emit(nocatch& i) {
  // register a null catch trace at this position.
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
  std::vector<TransBCMapping>* bcmap = nullptr;
  if (mcg->tx().isTransDBEnabled() || RuntimeOption::EvalJitUseVtuneAPI) {
    bcmap = &mcg->cgFixups().m_bcMap;
  }
  jit::vector<jit::vector<TcaRange>> block_ranges(areas.size());
  for (auto& r : block_ranges) r.resize(unit.blocks.size());
  for (int i = 0, n = labels.size(); i < n; ++i) {
    assert(check(unit.blocks[labels[i]]));
    auto b = labels[i];
    auto& block = unit.blocks[b];
    X64Assembler as { area(block.area).code };
    a = &as;
    addrs[b] = a->frontier();
    for (int j = 0; j < areas.size(); j++) {
      block_ranges[j][b] = TcaRange{areas[j].code.frontier(), nullptr};
    }
    {
      // Compute the next block we will emit into the current area.
      auto cur_start = start(labels[i]);
      auto j = i + 1;
      while (j < labels.size() && cur_start != start(labels[j])) {
        j++;
      }
      next = j < labels.size() ? labels[j] : Vlabel(unit.blocks.size());
    }
    for (auto& inst : block.code) {
      if (bcmap && inst.sk.valid() && (bcmap->empty() ||
          bcmap->back().md5 != inst.sk.unit()->md5() ||
          bcmap->back().bcStart != inst.sk.offset())) {
        bcmap->push_back(TransBCMapping{
          inst.sk.unit()->md5(),
          inst.sk.offset(),
          main().frontier(),
          cold().frontier(),
          frozen().frontier()
        });
      }
      switch (inst.op) {
#define O(name, imms, uses, defs)\
        case Vinstr::name: emit(inst.name##_); break;
        X64_OPCODES
#undef O
      }
    }
    for (int j = 0; j < areas.size(); j++) {
      auto start = block_ranges[j][b].start();
      block_ranges[j][b] = TcaRange{start, areas[j].code.frontier()};
    }
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
    auto d = meta->points[p.pos] - after_lea;
    assert(deltaFits(d, sz::dword));
    ((int32_t*)after_lea)[-1] = d;
  }
  if (dumpIREnabled(kCodeGenLevel+1)) {
    std::ostringstream str;
    for (auto b : labels) {
      str << "B" << size_t(b) << "\n";
      for (int j = 0; j < areas.size(); j++) {
        if (!block_ranges[j][b].empty()) {
          disasmRange(str, block_ranges[j][b]);
          str << "\n";
        }
      }
    }
    HPHP::Trace::traceRelease("%s\n", str.str().c_str());
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
      i = jcc{ccNegate(i.cc), {i.targets[1], i.targets[0]}};
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

void Vgen::emit(loadq& i) {
  if (i.s.seg == Vptr::FS) a->fs();
  a->loadq(i.s.mr(), i.d);
}

// check that each block has exactly one terminal instruction at the end.
bool Vgen::check(Vblock& block) {
  assert(!block.code.empty());
  auto n = block.code.size();
  for (size_t i = 0; i < n - 1; ++i) {
    assert(!isBlockEnd(block.code[i]));
  }
  assert(isBlockEnd(block.code[n - 1]));
  return true;
}
}

Vout& Vasm::add(CodeBlock& cb, AreaIndex area) {
  assert(size_t(area) == m_areas.size());
  auto b = m_unit.makeBlock(area);
  if (size_t(b) == 0) m_unit.roots.push_back(b);
  Vout v{m_meta, m_unit, b, area};
  m_areas.push_back(Area{v, cb, cb.frontier()});
  return m_areas.back().out;
}

// copy of layoutBlocks in layout.cpp
jit::vector<Vlabel> layoutBlocks(Vunit& m_unit) {
  auto blocks = sortBlocks(m_unit);
  // partition into main/cold/frozen areas without changing relative order,
  // and the end{} block will be last.
  auto coldIt = std::stable_partition(blocks.begin(), blocks.end(),
    [&](Vlabel b) {
      return m_unit.blocks[b].area == AreaIndex::Main &&
             m_unit.blocks[b].code.back().op != Vinstr::end;
    });
  std::stable_partition(coldIt, blocks.end(),
    [&](Vlabel b) {
      return m_unit.blocks[b].area == AreaIndex::Cold &&
             m_unit.blocks[b].code.back().op != Vinstr::end;
    });
  return blocks;
}

void Vasm::finish(const Abi& abi) {
  if (m_unit.hasVrs()) {
    Timer _t(Timer::vasm_xls);
    allocateRegisters(m_unit, abi);
  }
  if (m_unit.blocks.size() > 1) {
    Timer _t(Timer::vasm_jumps);
    optimizeJmps(m_unit);
  }
  Timer _t(Timer::vasm_gen);
  auto blocks = layoutBlocks(m_unit);
  Vgen(m_unit, m_areas, m_meta).emit(blocks);
}

auto const vauto_gp = RegSet(rAsm).add(reg::r11);
auto const vauto_simd = RegSet(reg::xmm5).add(reg::xmm6).add(reg::xmm7);
UNUSED const Abi vauto_abi {
  .gpUnreserved = vauto_gp,
  .gpReserved = x64::abi.gp() - vauto_gp,
  .simdUnreserved = vauto_simd,
  .simdReserved = x64::abi.simd() - vauto_simd,
  .calleeSaved = x64::abi.calleeSaved
};

Vauto::~Vauto() {
  for (auto& b : unit().blocks) {
    if (!b.code.empty()) {
      // found at least one nonempty block. finish up.
      if (!main().closed()) main() << end{};
      assert(m_areas.size() < 2 || cold().empty() || cold().closed());
      assert(m_areas.size() < 3 || frozen().empty() || frozen().closed());
      printUnit("after vasm-auto", unit());
      finish(vauto_abi);
      return;
    }
  }
}

std::string format(Vreg r) {
  if (r.isPhys()) {
    if (r.isGP()) {
      Reg64 r64 = r;
      return regname(r64);
    } else {
      RegXMM rxmm = r;
      return regname(rxmm);
    }
  }
  std::ostringstream str;
  str << "%" << size_t(r);
  return str.str();
}

Vtuple findDefs(const Vunit& unit, Vlabel b) {
  assert(!unit.blocks[b].code.empty() &&
         unit.blocks[b].code.front().op == Vinstr::phidef);
  return unit.blocks[b].code.front().phidef_.defs;
}

}}}
