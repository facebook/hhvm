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

#include "hphp/runtime/vm/jit/vasm-emit.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-internal.h"
#include "hphp/runtime/vm/jit/vasm-lower.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

using namespace arm;
using namespace vixl;

namespace arm { struct ImmFolder; }

namespace {
///////////////////////////////////////////////////////////////////////////////

const TCA kEndOfTargetChain = reinterpret_cast<TCA>(0xf00ffeeffaaff11f);

vixl::Register X(Vreg64 r) {
  PhysReg pr(r.asReg());
  return x2a(pr);
}

vixl::Register W(Vreg64 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::Register W(Vreg32 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::Register W(Vreg16 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::Register W(Vreg8 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::FPRegister D(Vreg r) {
  return x2simd(r);
}

/*
 * Convert a Vptr to a MemOperand.
 *
 * If the Vptr is too fancy, this will emit instructions.
 *
 * FIXME: Following can be optimized as load/store take index+shift
 *
 */
vixl::MemOperand M(vixl::MacroAssembler* a, Vptr p) {
  auto shift = p.scale == 2 ? 1 :
               p.scale == 4 ? 2 :
               p.scale == 8 ? 3 : 0;
  if (p.base.isValid()) {
    // TLS addresses are baseless. If base is valid, p.seg must be Vptr::DS
    assertx(p.seg == Vptr::DS);

    if (!p.index.isValid()) return X(p.base)[p.disp];
    a->Lsl(rAsm2, X(p.index), shift);
    if (!p.disp) return X(p.base)[rAsm2];
    a->Add(rAsm2, X(p.base), rAsm2 /* Don't set flags */);
    return rAsm2[p.disp];
  }
  // Read TPIDR_EL0, if it is a TLS address
  switch(p.seg) {
  case Vptr::DS:
    a->Mov(rAsm2, vixl::xzr);
    break;
  case Vptr::FS:
  case Vptr::GS:
    a->Mrs(rAsm2, TPIDR_EL0);
    break;
  default:
    always_assert(false);
  }
  // no base, but index,scale,disp can be valid
  if (p.index.isValid()) {
    a->Add(rAsm2, rAsm2, Operand(X(p.index), LSL, shift));
    return rAsm2[p.disp];
  }
  // no base, no index.
  a->Add(rAsm2, rAsm2, p.disp);
  return rAsm2[0];
}

vixl::Condition C(ConditionCode cc) {
  return arm::convertCC(cc);
}

///////////////////////////////////////////////////////////////////////////////

struct Vgen {
  explicit Vgen(Venv& env)
    : text(env.text)
    , codeBlock(env.cb)
    , assem(*codeBlock)
    , a(&assem)
    , current(env.current)
    , next(env.next)
    , jmps(env.jmps)
    , jccs(env.jccs)
    , bccs(env.bccs)
    , catches(env.catches)
  {}

  static void patch(Venv& env);
  static void pad(CodeBlock& cb) {}

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }

  // intrinsics
  void emit(const copy& i);
  void emit(const copy2& i);
  void emit(const debugtrap& i) { a->Brk(0); }
  void emit(const fallthru& i) {}
  void emit(const ldimmb& i);
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const ldimmqs& i);
  void emit(const ldimmw& i);
  void emit(const load& i);
  void emit(const store& i);
  void emit(const mcprep& i);

  // native function abi
  void emit(const call& i);
  void emit(const callm& i);
  void emit(const callr& i) { a->Blr(X(i.target)); }
  void emit(const calls& i);
  void emit(const ret& i) { a->Ret(); }

  // stub function abi
  void emit(const stublogue& i);
  void emit(const stubret& i);
  void emit(const callstub& i);
  void emit(const callfaststub& i);
  void emit(const tailcallstub& i);

  // php function abi
  void emit(const phplogue& i);
  void emit(const phpret& i);
  void emit(const callphp& i);
  void emit(const tailcallphp& i);
  void emit(const callarray& i);
  void emit(const contenter& i);
  void emit(const leavetc&) { a->Ret(); }

  // exceptions
  void emit(const landingpad& i) {}
  void emit(const nothrow& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);

  // intrinsics
  void emit(const absdbl& i) { a->Fabs(D(i.d), D(i.s)); }
  void emit(const sar& i) { a->Asr(X(i.d), X(i.s1), X(i.s0)); }
  void emit(const shl& i) { a->Lsl(X(i.d), X(i.s1), X(i.s0)); }
  void emit(const srem& i);
  void emit(const divint& i) { a->Sdiv(X(i.d), X(i.s0), X(i.s1)); }

  // instructions
  void emit(const addl& i) { a->Add(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const addli& i) { a->Add(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const addlm& i);
  void emit(const addlim& i);
  void emit(const addq& i) { a->Add(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const addqi& i) { a->Add(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const addqim& i);
  void emit(const addsd i) { a->Fadd(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const andb& i) { a->And(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const andbi& i);
  void emit(const andbim& i);
  void emit(const andl& i) { a->And(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const andli& i) { a->And(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const andq& i) { a->And(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const andqi& i) { a->And(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const cloadq& i);
  void emit(const cmovq& i);
  void emit(const cmpwim& i);
  void emit(const cmpl& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpli& i) { a->Cmp(W(i.s1), i.s0.l()); }
  void emit(const cmplim& i);
  void emit(const cmplm& i);
  void emit(const cmpq& i) { a->Cmp(X(i.s1), X(i.s0)); }
  void emit(const cmpqi& i) { a->Cmp(X(i.s1), i.s0.l()); }
  void emit(const cmpqim& i);
  void emit(const cmpqm& i);
  void emit(const cmpsd& i);
  void emit(const cqo& i);
  void emit(const cvttsd2siq& i);
  void emit(const cvtsi2sd& i);
  void emit(const cvtsi2sdm& i);
  void emit(const decl& i) { a->Sub(W(i.d), W(i.s), 1, SetFlags); }
  void emit(const declm& i);
  void emit(const decq& i) { a->Sub(X(i.d), X(i.s), 1, SetFlags); }
  void emit(const decqm& i);
  void emit(const divsd i) { a->Fdiv(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const idiv& i);
  void emit(const imul& i);
  void emit(const incl& i) { a->Add(W(i.d), W(i.s), 1LL, SetFlags); }
  void emit(const inclm& i);
  void emit(const incq& i) { a->Add(X(i.d), X(i.s), 1LL, SetFlags); }
  void emit(const incqm& i);
  void emit(const incqmlock& i);
  void emit(const incw& i) { a->Add(W(i.d), W(i.s), 1LL, SetFlags); }
  void emit(const incwm& i);
  void emit(const jcc i);
  void emit(const jcci& i);
  void emit(const jmp i);
  void emit(const jmpr& i) { a->Br(X(i.target)); }
  void emit(const jmpm& i);
  void emit(const jmpi i);
  void emit(const lea& i);
  void emit(const leap& i);
  void emit(const loadups& i);
  void emit(const loadtqb& i) { a->Ldrsb(W(i.d), M(a, i.s)); }
  void emit(const loadb& i) { a->Ldrsb(W(i.d), M(a, i.s)); }
  void emit(const loadl& i) { a->Ldr(W(i.d), M(a, i.s)); }
  void emit(const loadqp& i);
  void emit(const loadsd& i);
  void emit(const loadzbl& i) { a->Ldrb(W(i.d), M(a, i.s)); }
  void emit(const loadzbq& i) { a->Ldrb(W(i.d), M(a, i.s)); /* FIXME */ }
  void emit(const loadzlq& i) { a->Ldr(W(i.d), M(a, i.s)); /* FIXME */ }
  void emit(const movb& i) { a->Mov(W(i.d), W(i.s)); }
  void emit(const movl& i) { a->Mov(W(i.d), W(i.s)); }
  void emit(const movzbl& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(const movzbq& i) { a->Uxtb(X(i.d), W(i.s).X()); }
  void emit(const movtqb& i);
  void emit(const movtql& i);
  void emit(const mulsd& i);
  void emit(const neg& i) { a->Neg(X(i.d), X(i.s), SetFlags); }
  void emit(const nop& i) { a->Nop(); }
  void emit(const not& i) { a->Mvn(X(i.d), X(i.s)); }
  void emit(const notb& i) { a->Mvn(W(i.d), W(i.s)); }
  void emit(const orbim& i);
  void emit(const orwim& i);
  void emit(const orq& i) { a->Orr(X(i.d), X(i.s1), X(i.s0)); /* FIXME: */ }
  void emit(const orqi& i) { a->Orr(X(i.d), X(i.s1), i.s0.l()); /* FIXME */ }
  void emit(const orqim& i);
  void emit(const pop& i);
  void emit(const popm& i);
  void emit(const psllq& i);
  void emit(const psrlq& i);
  void emit(const push& i);
  void emit(const pushm& i);
  void emit(const roundsd& i);
  void emit(const setcc& i);
  void emit(const sarq& i);
  void emit(const sarqi& i);
  void emit(const shlli& i);
  void emit(const shlq& i);
  void emit(const shlqi& i);
  void emit(const shrli& i) { a->Lsr(W(i.d), W(i.s1), i.s0.l()); /* FIXME */ }
  void emit(const shrqi& i);
  void emit(const sqrtsd& i);
  void emit(const storeb& i) { a->Strb(W(i.s), M(a, i.m)); }
  void emit(const storebi& i);
  void emit(const storeups& i);
  void emit(const storel& i) { a->Str(W(i.s), M(a, i.m)); }
  void emit(const storeli& i);
  void emit(const storeqi& i);
  void emit(const storesd& i);
  void emit(const storew& i) { a->Str(W(i.s), M(a, i.m)); }
  void emit(const storewi& i);
  void emit(const subbi& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const subl& i) { a->Sub(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const subli& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const subq& i) { a->Sub(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const subqi& i) { a->Sub(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const subsd& i) { a->Fsub(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const testb& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testbi& i) { a->Tst(W(i.s1), i.s0.l()); }
  void emit(const testbim&);
  void emit(const testwim&);
  void emit(const testl& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testli& i) { a->Tst(W(i.s1), i.s0.l()); }
  void emit(const testlim&);
  void emit(const testq& i) { a->Tst(X(i.s1), X(i.s0)); }
  void emit(const testqi& i) { a->Tst(X(i.s1), i.s0.q()); }
  void emit(const testqm&);
  void emit(const testqim&);
  void emit(const ucomisd& i) { a->Fcmp(D(i.s0), D(i.s1)); }
  void emit(const ud2& i) { a->Brk(1); }
  void emit(const unpcklpd&);
  void emit(const xorb& i) { a->Eor(W(i.d), W(i.s1), W(i.s0)); /* FIXME */ }
  void emit(const xorbi& i) { a->Eor(W(i.d), W(i.s1), i.s0.l()); /* FIXME */ }
  void emit(const xorl& i) { a->Eor(W(i.d), W(i.s1), W(i.s0)); /* FIXME */ }
  void emit(const xorq& i) { a->Eor(X(i.d), X(i.s1), X(i.s0)); /* FIXME */ }
  void emit(const xorqi& i) { a->Eor(X(i.d), X(i.s1), i.s0.l()); /* FIXME */ }

private:
  CodeBlock& frozen() { return text.frozen().code; }

private:
  Vtext& text;
  CodeBlock* codeBlock;
  vixl::MacroAssembler assem;
  vixl::MacroAssembler* a;

  const Vlabel current;
  const Vlabel next;
  jit::vector<Venv::LabelPatch>& jmps;
  jit::vector<Venv::LabelPatch>& jccs;
  jit::vector<Venv::LabelPatch>& bccs;
  jit::vector<Venv::LabelPatch>& catches;
};

///////////////////////////////////////////////////////////////////////////////

void Vgen::patch(Venv& env) {
  for (auto& p : env.jmps) {
    assertx(env.addrs[p.target]);
    *reinterpret_cast<TCA*>(p.instr + 8) = env.addrs[p.target];
  }
  for (auto& p : env.jccs) {
    assertx(env.addrs[p.target]);
    *reinterpret_cast<TCA*>(p.instr + 16) = env.addrs[p.target];
  }
  for (auto& p : env.bccs) {
    assertx(env.addrs[p.target]);
    auto link = (Instruction*) p.instr;
    link->SetImmPCOffsetTarget(Instruction::Cast(env.addrs[p.target]));
  }
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const copy& i) {
  if (i.s.isGP() && i.d.isGP()) {
    a->Mov(X(i.d), X(i.s));
  } else if (i.s.isSIMD() && i.d.isGP()) {
    a->Fmov(X(i.d), D(i.s));
  } else if (i.s.isGP() && i.d.isSIMD()) {
    a->Fmov(D(i.d), X(i.s));
  } else {
    assertx(i.s.isSIMD() && i.d.isSIMD());
    a->Fmov(D(i.d), D(i.s));
  }
}

void Vgen::emit(const copy2& i) {
  MovePlan moves;
  Reg64 d0 = i.d0, d1 = i.d1, s0 = i.s0, s1 = i.s1;
  moves[d0] = s0;
  moves[d1] = s1;
  auto howTo = doRegMoves(moves, rAsm); // rAsm isn't used.
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      a->Mov(X(how.m_dst), X(how.m_src));
    } else {
      auto const d = X(how.m_dst);
      auto const s = X(how.m_src);
      a->Eor(d, d, s);
      a->Eor(s, d, s);
      a->Eor(d, d, s);
    }
  }
}

void emitSimdImmInt(vixl::MacroAssembler* a, int64_t val, Vreg d) {
  // FIXME: Same Fmov() for all 'ldimm[bwl]' instructions below
  if (val == 0) {
    a->Fmov(D(d), vixl::xzr);
  } else {
    union { double dval; int64_t ival; };
    ival = val;
    a->Fmov(D(d), dval);
  }
}

void Vgen::emit(const ldimmb& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.b(), i.d);
  } else {
    Vreg8 d = i.d;
    a->Mov(W(d), i.s.l());
  }
}

void Vgen::emit(const ldimml& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.l(), i.d);
  } else {
    Vreg32 d = i.d;
    a->Mov(W(d), i.s.l());
  }
}

void Vgen::emit(const ldimmq& i) {
  union { double dval; int64_t ival; };
  ival = i.s.q();
  if (i.d.isSIMD()) {
    // FIXME:
    // Assembler::fmov (which you'd think shouldn't be a macro instruction)
    // will emit a ldr from a literal pool if IsImmFP64 is false. vixl's
    // literal pools don't work well with our codegen pattern, so if that
    // would happen, emit the raw bits into a GPR first and then move them
    // unmodified into a SIMD.
    if (vixl::Assembler::IsImmFP64(dval)) {
      a->Fmov(D(i.d), dval);
    } else if (ival == 0) { // careful: dval==0.0 is true for -0.0
      // 0.0 is not encodeable as an immediate to Fmov, but this works.
      a->Fmov(D(i.d), vixl::xzr);
    } else {
      a->Mov(rAsm, ival);
      a->Fmov(D(i.d), rAsm);
    }
  } else {
    a->Mov(X(i.d), ival);
  }
}

void Vgen::emit(const ldimmqs& i) {
  emitSmashableMovq(*codeBlock, i.s.q(), i.d);
}

void Vgen::emit(const ldimmw& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.l(), i.d);
  } else {
    a->movk(W(i.d), i.s.l(), 0);
  }
}

void Vgen::emit(const load& i) {
  if (i.d.isGP()) {
    a->Ldr(X(i.d), M(a, i.s));
  } else {
    a->Ldr(D(i.d), M(a, i.s));
  }
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    a->Str(X(i.s), M(a, i.d));
  } else {
    a->Str(D(i.s), M(a, i.d));
  }
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const mcprep& i) {
  /*
   * Initially, we set the cache to hold (addr << 1) | 1 (where `addr' is the
   * address of the movq) so that we can find the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * handlePrimeCacheInit can tell it's not been smashed yet
   */
  auto const mov_addr = emitSmashableMovq(*codeBlock, 0, r64(i.d));
  auto const imm = reinterpret_cast<uint64_t>(mov_addr);
  smashMovq(mov_addr, (imm << 1) | 1);

  mcg->cgFixups().addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const call& i) {
  a->Mov(rAsm, reinterpret_cast<uint64_t>(i.target));
  a->Blr(rAsm);
}

void Vgen::emit(const callm& i) {
  a->Ldr(rAsm, M(a, i.target));
  a->Blr(rAsm);
}

void Vgen::emit(const calls& i) {
  emitSmashableCall(*codeBlock, i.target);
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const stublogue& i) {
  // Save x29 and x30 always, regardless of i.saveframe (makes sp 16B aligned)
  a->Stp(x29, x30, MemOperand(sp, -16, PreIndex));
}

void Vgen::emit(const stubret& i) {
  if(i.saveframe)
    a->Ldp(x29, x30, MemOperand(sp, 16, PostIndex));
  else
    a->Ldp(rAsm, x30, MemOperand(sp, 16, PostIndex));
  a->Ret();
}

void Vgen::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}

void Vgen::emit(const tailcallstub& i) {
  // sp is already aligned, just jmp
  emit(jmpi{i.target, i.args});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const phplogue& i) {
  // Save link register in m_savedRip on the current VM frame pointed by 'i.fp'
  a->Str(rLinkReg, X(i.fp)[AROFF(m_savedRip)]);
}

void Vgen::emit(const phpret& i) {
  a->Ldr(rLinkReg, X(i.fp)[AROFF(m_savedRip)]);
  if (!i.noframe) {
    a->Ldr(X(i.d), X(i.fp)[AROFF(m_sfp)]);
  }
  a->Ret();
}

void Vgen::emit(const callphp& i) {
  emitSmashableCall(*codeBlock, i.stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const tailcallphp& i) {
  // To make callee's return as caller's return, load the return address at
  // i.fp[AROFF(m_savedRip)] into link register and jmp to target
  a->Ldr(rLinkReg, X(i.fp)[AROFF(m_savedRip)]);
  emit(jmpr{i.target, i.args});
}

void Vgen::emit(const callarray& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const contenter& i) {
  // FIXME: Mimicking x64. Verify
  vixl::Label Stub, End;

  a->B(&End);
  a->bind(&Stub);

  // FIXME: x64 has pop() here. Needs sp adjustment?
  a->Ldr(rAsm, sp[0]);
  a->Str(rAsm, X(i.fp)[AROFF(m_savedRip)]);

  a->Br(X(i.target));
  a->bind(&End);
  a->Ldr(rAsm, &Stub);
  a->Blr(rAsm);
  // m_savedRip will point here.
  emit(unwind{{i.targets[0], i.targets[1]}});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const nothrow& i) {
  mcg->registerCatchBlock(a->frontier(), nullptr);
}

void Vgen::emit(const syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(a->frontier(), i.fix);
}

void Vgen::emit(const unwind& i) {
  catches.push_back({a->frontier(), i.targets[1]});
  emit(jmp{i.targets[0]});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const srem& i) {
  a->Sdiv(rAsm, X(i.s0), X(i.s1));
  a->Msub(X(i.d), rAsm, X(i.s1), X(i.s0));
}

void Vgen::emit(const addlm& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm.W(), adr);
  a->Add(rAsm.W(), rAsm.W(), W(i.s0), SetFlags);
  a->Str(rAsm.W(), adr);
}

void Vgen::emit(const addlim& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm.W(), adr);
  a->Add(rAsm.W(), rAsm.W(), i.s0.l(), SetFlags);
  a->Str(rAsm.W(), adr);
}

void Vgen::emit(const addqim& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm, adr);
  a->Add(rAsm, rAsm, i.s0.q(), SetFlags);
  a->Str(rAsm, adr);
}

void Vgen::emit(const andbi& i) {
  a->Mov(rAsm.W(), i.s0.l());
  a->And(W(i.d), W(i.s1), rAsm.W(), SetFlags);
}

void Vgen::emit(const andbim& i) {
  auto adr = M(a, i.m);
  a->Ldrsb(rAsm.W(), adr);
  a->And(rAsm.W(), rAsm.W(), i.s.l(), SetFlags);
  a->Strb(rAsm.W(), adr);
}

void Vgen::emit(const cloadq& i) {
  a->Ldr(rAsm, M(a, i.t));
  a->Csel(X(i.d), rAsm, X(i.f), C(i.cc));
}

void Vgen::emit(const cmovq& i) {
  a->Csel(X(i.d), X(i.t), X(i.f), C(i.cc));
}

void Vgen::emit(const cmpwim& i) {
  a->Ldrsh(rAsm.W(), M(a, i.s1));
  a->Cmp(rAsm.W(), i.s0.l());
}

void Vgen::emit(const cmplim& i) {
  a->Ldr(rAsm.W(), M(a, i.s1));
  a->Cmp(rAsm.W(), i.s0.l());
}

void Vgen::emit(const cmplm& i) {
  a->Ldr(rAsm.W(), M(a, i.s1));
  a->Cmp(rAsm.W(), W(i.s0));
}

void Vgen::emit(const cmpqim& i) {
  a->Ldr(rAsm, M(a, i.s1));
  a->Cmp(rAsm, i.s0.q());
}

void Vgen::emit(const cmpqm& i) {
  a->Ldr(rAsm, M(a, i.s1));
  a->Cmp(rAsm, X(i.s0));
}

void Vgen::emit(const cmpsd& i) {
  // FIXME: Mimicking x64 'cmpsd' (flags are modified). change ir->vasm?
  a->Fcmp(D(i.s0), D(i.s1));
  switch(i.pred) {
  case ComparisonPred::eq_ord:
    a->Csetm(rAsm, C(jit::CC_E));
    break;
  case ComparisonPred::ne_unord:
    a->Csetm(rAsm, C(jit::CC_NE));
    break;
  default:
    always_assert(false);
  }
  a->Fmov(D(i.d), rAsm);
}

void Vgen::emit(const cqo& i) {
  // FIXME: Sign-Extend RAX
  not_implemented();
}

void Vgen::emit(const cvttsd2siq& i) {
  a->Fcvtzs(X(i.d), D(i.s));
}

void Vgen::emit(const cvtsi2sd& i) {
  a->Scvtf(D(i.d), X(i.s));
}

void Vgen::emit(const cvtsi2sdm& i) {
  a->Ldr(rAsm, M(a, i.s));
  a->Scvtf(D(i.d), rAsm);
}

void Vgen::emit(const declm& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm.W(), adr);
  a->Sub(rAsm.W(), rAsm.W(), 1, SetFlags);
  a->Str(rAsm.W(), adr);
}

void Vgen::emit(const decqm& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm, adr);
  a->Sub(rAsm, rAsm, 1, SetFlags);
  a->Str(rAsm, adr);
}

void Vgen::emit(const idiv& i) {
  // FIXME: (rdx:rax)/i.s
  not_implemented();
}

void Vgen::emit(const imul& i) {
  // FIXME: SetFlags?
  a->Mul(X(i.d), X(i.s0), X(i.s1));
}

void Vgen::emit(const inclm& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm.W(), adr);
  a->Add(rAsm.W(), rAsm.W(), 1, SetFlags);
  a->Str(rAsm.W(), adr);
}

void Vgen::emit(const incqm& i) {
  auto adr = M(a, i.m);
  a->Ldr(rAsm, adr);
  a->Add(rAsm, rAsm, 1, SetFlags);
  a->Str(rAsm, adr);
}

void Vgen::emit(const incqmlock& i) {
  auto adr = M(a, i.m);
  vixl::Label again;
  a->bind(&again);
  a->ldxr(rAsm, adr);
  a->Add(rAsm, rAsm, 1, SetFlags);
  a->stxr(rAsm.W(), rAsm, adr);
  a->Cbnz(rAsm.W(), &again);
}

void Vgen::emit(const incwm& i) {
  auto adr = M(a, i.m);
  a->Ldrh(rAsm.W(), adr);
  a->Add(rAsm.W(), rAsm.W(), 1, SetFlags);
  a->Strh(rAsm.W(), adr);
}

void Vgen::emit(const jcc i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});
    vixl::Label truecase, falsecase, data;
    // Vgen::patch() should work for all 'jcc{,i,s}' instructions. So, using
    // 4 instructions though it can be done in 3 as jccs needs 4
    a->B(&truecase, C(i.cc));
    a->B(&falsecase);
    a->bind(&truecase);
    a->Ldr(rAsm, &data);
    a->Br(rAsm);
    a->bind(&data);
    a->dc64(reinterpret_cast<int64_t>(a->frontier()));
    a->bind(&falsecase);
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const jcci& i) {
  vixl::Label truecase, falsecase, data;

  // Vgen::patch() should work for all 'jcc{,i,s}' instructions. So, using
  // 4 instructions though it can be done in 3 as jccs needs 4
  a->B(&truecase, C(i.cc));
  a->B(&falsecase);
  a->bind(&truecase);
  a->Ldr(rAsm, &data);
  a->Br(rAsm);
  a->bind(&data);
  a->dc64(reinterpret_cast<int64_t>(i.taken));
  a->bind(&falsecase);
  emit(jmp{i.target});
}

void Vgen::emit(jmp i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  vixl::Label data;
  a->Ldr(rAsm, &data);
  a->Br(rAsm);
  a->bind(&data);
  a->dc64(reinterpret_cast<int64_t>(a->frontier()));
}

void Vgen::emit(const jmpm& i) {
  a->Ldr(rAsm, M(a, i.target));
  a->Br(rAsm);
}

void Vgen::emit(const jmpi i) {
  vixl::Label data;
  a->Ldr(rAsm, &data);
  a->Br(rAsm);
  a->bind(&data);
  a->dc64(reinterpret_cast<int64_t>(i.target));
}

void Vgen::emit(const lea& i) {
  auto adr = M(a, i.s);
  auto offset = reinterpret_cast<int64_t>(adr.offset());
  a->Add(X(i.d), adr.base(), offset /* Don't set flags */);
}

void Vgen::emit(const leap& i) {
  // FIXME: PC relative?
  a->Mov(X(i.d), i.s.r.disp);
}

void Vgen::emit(const loadups& i) {
  // FIXME: Needs simd support in vixl
  not_implemented();
}

void Vgen::emit(const loadqp& i) {
  // FIXME: PC relative?
  a->Mov(X(i.d), i.s.r.disp);
  a->Ldr(X(i.d), X(i.d)[0]);
}

void Vgen::emit(const loadsd& i) {
  a->Ldr(rAsm, M(a, i.s));
  a->Fmov(D(i.d), rAsm);
}

void Vgen::emit(const movtqb& i) {
  a->Mov(W(i.d), X(i.s));
}

void Vgen::emit(const movtql& i) {
  a->Mov(W(i.d), X(i.s));
}

void Vgen::emit(const mulsd& i) {
  a->Fmul(D(i.d), D(i.s0), D(i.s1));
}

void Vgen::emit(const orbim& i) {
  // FIXME: SetFlags?
  a->Ldrsb(rAsm.W(), M(a, i.m));
  a->Orr(rAsm.W(), rAsm.W(), i.s0.l());
  a->Strb(rAsm.W(), M(a, i.m));
}

void Vgen::emit(const orwim& i) {
  // FIXME: SetFlags?
  a->Ldrsh(rAsm.W(), M(a, i.m));
  a->Orr(rAsm.W(), rAsm.W(), i.s0.l());
  a->Strh(rAsm.W(), M(a, i.m));
}

void Vgen::emit(const orqim& i) {
  // FIXME: SetFlags?
  a->Ldr(rAsm, M(a, i.m));
  a->Orr(rAsm, rAsm, i.s0.q());
  a->Str(rAsm, M(a, i.m));
}

void Vgen::emit(const pop& i) {
  a->Ldr(X(i.d), MemOperand(sp, 8, PostIndex));
}

void Vgen::emit(const popm& i) {
  a->Ldr(rAsm, MemOperand(sp, 8, PostIndex));
  a->Str(rAsm, M(a, i.d));
}

void Vgen::emit(const psllq& i) {
  // FIXME: Needs simd support in vixl
  not_implemented();
}

void Vgen::emit(const psrlq& i) {
  // FIXME: Needs simd support in vixl
  not_implemented();
}

void Vgen::emit(const push& i) {
  a->Str(X(i.s), MemOperand(sp, -8, PreIndex));
}

void Vgen::emit(const pushm& i) {
  a->Ldr(rAsm, M(a, i.s));
  a->Str(rAsm, MemOperand(sp, -8, PreIndex));
}

void Vgen::emit(const roundsd& i) {
  switch(i.dir) {
  case RoundDirection::nearest:
    a->frintn(D(i.d), D(i.s));
    break;
  case RoundDirection::floor:
    a->frintm(D(i.d), D(i.s));
    break;
  case RoundDirection:: ceil:
    a->frintp(D(i.d), D(i.s));
    break;
  default:
    assertx(i.dir == RoundDirection::truncate);
    a->frintz(D(i.d), D(i.s));
  }
}

void Vgen::emit(const setcc& i) {
  PhysReg r(i.d.asReg());
  a->Cset(X(r), C(i.cc));
}

void Vgen::emit(const sarq& i) {
  // FIXME: SetFlags?
  a->Asr(X(i.d), X(i.s), 1);
}

void Vgen::emit(const sarqi& i) {
  // FIXME: SetFlags?
  a->Asr(X(i.d), X(i.s1), i.s0.l());
}

void Vgen::emit(const shlli& i) {
  // FIXME: SetFlags?
  a->Lsl(W(i.d), W(i.s1), i.s0.l());
}

void Vgen::emit(const shlq& i) {
  // FIXME: SetFlags?
  a->Lsl(X(i.d), X(i.s), 1);
}

void Vgen::emit(const shlqi& i) {
  // FIXME: SetFlags?
  a->Lsl(X(i.d), X(i.s1), i.s0.l());
}

void Vgen::emit(const shrqi& i) {
  // FIXME: SetFlags?
  a->Lsr(X(i.d), X(i.s1), i.s0.l());
}

void Vgen::emit(const sqrtsd& i) {
  a->Fsqrt(D(i.d), D(i.s));
}

void Vgen::emit(const storebi& i) {
  a->Mov(rAsm.W(), i.s.l());
  a->Strb(rAsm.W(), M(a, i.m));
}

void Vgen::emit(const storeups& i) {
  // FIXME: Needs simd support in vixl
  not_implemented();
}

void Vgen::emit(const storeli& i) {
  a->Mov(rAsm.W(), i.s.l());
  a->Str(rAsm.W(), M(a, i.m));
}

void Vgen::emit(const storeqi& i) {
  a->Mov(rAsm, i.s.q());
  a->Str(rAsm, M(a, i.m));
}

void Vgen::emit(const storesd& i) {
  a->Fmov(rAsm, D(i.s));
  a->Str(rAsm, M(a, i.m));
}

void Vgen::emit(const storewi& i) {
  a->Mov(rAsm.W(), i.s.w());
  a->Strh(rAsm.W(), M(a, i.m));
}

void Vgen::emit(const testbim& i) {
  a->Ldrsb(rAsm.W(), M(a, i.s1));
  a->Tst(rAsm.W(), i.s0.l());
}

void Vgen::emit(const testwim& i) {
  a->Ldrsh(rAsm.W(), M(a, i.s1));
  a->Tst(rAsm.W(), i.s0.l());
}

void Vgen::emit(const testlim& i) {
  a->Ldr(rAsm.W(), M(a, i.s1));
  a->Tst(rAsm.W(), i.s0.l());
}

void Vgen::emit(const testqm& i) {
  a->Ldr(rAsm, M(a, i.s1));
  a->Tst(X(i.s0), rAsm);
}

void Vgen::emit(const testqim& i) {
  a->Ldr(rAsm, M(a, i.s1));
  a->Tst(rAsm, i.s0.q());
}

void Vgen::emit(const unpcklpd& i) {
  // FIXME: Needs simd support in vixl
  not_implemented();
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Some vasm opcodes don't have equivalent single instructions on ARM, and the
 * equivalent instruction sequences require scratch registers.  We have to
 * lower these to ARM-suitable vasm opcodes before register allocation.
 */
template<typename Inst>
void lower(Inst& i, Vout& v) {
  v << i;
}

void lower(countbytecode& i, Vout& v) {
  v << incqm{i.base[g_bytecodesVasm.handle()], i.sf};
}

void lower(stubtophp& i, Vout& v) {
  v << addqi{8, reg::rsp, reg::rsp, v.makeReg()};
  v << popm{i.fp[AROFF(m_savedRip)]};
}

///////////////////////////////////////////////////////////////////////////////

void lower(cmpb& i, Vout& v) {
  auto s0 = v.makeReg();
  auto s1 = v.makeReg();
  v << movzbl{i.s0, s0};
  v << movzbl{i.s1, s1};
  v << cmpl{s0, s1, i.sf};
}

void lower(cmpbi& i, Vout& v) {
  auto s1 = v.makeReg();
  v << movzbl{i.s1, s1};
  v << cmpli{i.s0, s1, i.sf};
}

void lower(cmpbim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadzbl{i.s1, scratch};
  v << cmpli{i.s0, scratch, i.sf};
}

void lowerForARM(Vunit& unit) {
  assertx(check(unit));

  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);

  for (auto b : blocks) {
    auto oldCode = std::move(unit.blocks[b].code);
    Vout v{unit, b};

    for (auto& inst : oldCode) {
      v.setOrigin(inst.origin);

      switch (inst.op) {
#define O(nm, imm, use, def) \
        case Vinstr::nm: \
          lower(inst.nm##_, v); \
          break;

        VASM_OPCODES
#undef O
      }
    }
  }

  assertx(check(unit));
  printUnit(kVasmARMFoldLevel, "after lowerForARM", unit);
}

///////////////////////////////////////////////////////////////////////////////
}

void finishARM(Vunit& unit, Vtext& text, const Abi& abi, AsmInfo* asmInfo) {
  Timer timer(Timer::vasm_optimize);

  removeTrivialNops(unit);
  optimizePhis(unit);
  fuseBranches(unit);
  optimizeJmps(unit);
  optimizeExits(unit);
  vlower(unit);
  lowerForARM(unit);
  simplify(unit);
  if (!unit.constToReg.empty()) {
    foldImms<arm::ImmFolder>(unit);
  }
  {
    Timer timer(Timer::vasm_copy);
    optimizeCopies(unit, abi);
  }
  if (unit.needsRegAlloc()) {
    Timer _t(Timer::vasm_xls);
    removeDeadCode(unit);
    allocateRegisters(unit, abi);
  }
  if (unit.blocks.size() > 1) {
    Timer _t(Timer::vasm_jumps);
    optimizeJmps(unit);
  }

  Timer _t(Timer::vasm_gen);
  vasm_emit<Vgen>(unit, text, asmInfo);
}

///////////////////////////////////////////////////////////////////////////////
}}
