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
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-arm.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-internal.h"
#include "hphp/runtime/vm/jit/vasm-lower.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

using namespace arm;
using namespace vixl;

namespace arm { struct ImmFolder; }

namespace {
///////////////////////////////////////////////////////////////////////////////

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
  return x2f(r);
}

vixl::VRegister V(Vreg r) {
  return x2v(r);
}

uint8_t Log2(uint8_t value) {
  switch (value) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    default:
      always_assert(false);
  }
}

int64_t MSKTOP(int64_t value) {
  // Make sure that top 32 bits are consistent
  assertx(((value >> 32) == 0) || ((value >> 32) == -1));
  return value & ~0u;
}

vixl::MemOperand M(Vptr p) {
  assertx(p.base.isValid());
  if (p.index.isValid()) {
    assertx(p.disp == 0);
    return MemOperand(X(p.base), X(p.index), LSL, Log2(p.scale));
  }
  assertx(p.disp >= -256 && p.disp <= 255);
  return MemOperand(X(p.base), p.disp);
}

vixl::Condition C(ConditionCode cc) {
  return arm::convertCC(cc);
}

///////////////////////////////////////////////////////////////////////////////

struct Vgen {
  explicit Vgen(Venv& env)
    : env(env)
    , assem(*env.cb)
    , a(&assem)
    , base(a->frontier())
    , current(env.current)
    , next(env.next)
    , jmps(env.jmps)
    , jccs(env.jccs)
    , catches(env.catches)
  {}
  ~Vgen() {
    auto begin = reinterpret_cast<char*>(base);
    auto end = reinterpret_cast<char*>(a->frontier());
    __builtin___clear_cache(begin, end);
  }

  static void patch(Venv& env);

  static void pad(CodeBlock& cb) {
    vixl::MacroAssembler a { cb };
    auto const begin = reinterpret_cast<char*>(cb.frontier());
    while (cb.available() >= 4) a.Brk(1);
    assertx(cb.available() == 0);
    auto const end = reinterpret_cast<char*>(cb.frontier());
    __builtin___clear_cache(begin, end);
  }

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(const Inst& i) {
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

  // vm entry abi
  void emit(const inittc& i) {}
  void emit(const leavetc&);

  // exceptions
  void emit(const landingpad& i) {}
  void emit(const nothrow& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);

  // instructions
  void emit(const addl& i) { a->Add(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const addli& i) { a->Add(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const addq& i) { a->Add(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const addqi& i) { a->Add(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const addsd& i) { a->Fadd(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const andb& i) { a->And(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const andbi& i);
  void emit(const andl& i) { a->And(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const andli& i);
  void emit(const andq& i) { a->And(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const andqi& i) { a->And(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const cloadq& i);
  void emit(const cmovb& i) { a->Csel(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const cmovq& i) { a->Csel(X(i.d), X(i.t), X(i.f), C(i.cc)); }
  void emit(const cmpl& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpli& i) { a->Cmp(W(i.s1), i.s0.l()); }
  void emit(const cmpq& i) { a->Cmp(X(i.s1), X(i.s0)); }
  void emit(const cmpqi& i) { a->Cmp(X(i.s1), i.s0.q()); }
  void emit(const cvtsi2sd& i) { a->Scvtf(D(i.d), X(i.s)); }
  void emit(const decl& i) { a->Sub(W(i.d), W(i.s), 1, SetFlags); }
  void emit(const decq& i) { a->Sub(X(i.d), X(i.s), 1, SetFlags); }
  void emit(const decqmlock& i);
  void emit(const divint& i) { a->Sdiv(X(i.d), X(i.s0), X(i.s1)); }
  void emit(const divsd& i) { a->Fdiv(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const imul& i);
  void emit(const incl& i) { a->Add(W(i.d), W(i.s), 1, SetFlags); }
  void emit(const incq& i) { a->Add(X(i.d), X(i.s), 1, SetFlags); }
  void emit(const incqmlock& i);
  void emit(const incw& i) { a->Add(W(i.d), W(i.s), 1, SetFlags); }
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const jmpi& i);
  void emit(const jmpm& i);
  void emit(const jmpr& i) { a->Br(X(i.target)); }
  void emit(const lea& i);
  void emit(const leap& i) { a->Mov(X(i.d), i.s.r.disp); }
  void emit(const lead& i) { a->Mov(X(i.d), i.s.get()); }
  void emit(const loadb& i) { a->Ldrsb(W(i.d), M(i.s)); }
  void emit(const loadl& i) { a->Ldr(W(i.d), M(i.s)); }
  void emit(const loadqp& i);
  void emit(const loadqd& i);
  void emit(const loadsd& i) { a->Ldr(D(i.d), M(i.s)); }
  void emit(const loadtqb& i) { a->Ldrsb(W(i.d), M(i.s)); }
  void emit(const loadups& i);
  void emit(const loadw& i) { a->Ldrsh(W(i.d), M(i.s)); }
  void emit(const loadzbl& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadzbq& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadzlq& i) { a->Ldr(W(i.d), M(i.s)); }
  void emit(const movb& i) { a->Mov(W(i.d), W(i.s)); }
  void emit(const movl& i) { a->Mov(W(i.d), W(i.s)); }
  void emit(const movtqb& i) { a->Sxtb(W(i.d), W(i.s)); }
  void emit(const movtql& i) { a->Mov(W(i.d), W(i.s)); }
  void emit(const movzbw& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(const movzbl& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(const movzbq& i) { a->Uxtb(X(i.d), W(i.s).X()); }
  void emit(const movzwl& i) { a->Uxth(W(i.d), W(i.s)); }
  void emit(const movzwq& i) { a->Uxth(X(i.d), W(i.s).X()); }
  void emit(const movzlq& i) { a->Uxtw(X(i.d), W(i.s).X()); }
  void emit(const mulsd& i) { a->Fmul(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const neg& i) { a->Neg(X(i.d), X(i.s), SetFlags); }
  void emit(const nop& i) { a->Nop(); }
  void emit(const notb& i) { a->Mvn(W(i.d), W(i.s)); }
  void emit(const not& i) { a->Mvn(X(i.d), X(i.s)); }
  void emit(const orq& i);
  void emit(const orqi& i);
  void emit(const pop& i);
  void emit(const psllq& i);
  void emit(const psrlq& i);
  void emit(const push& i);
  void emit(const roundsd& i);
  void emit(const sar& i);
  void emit(const setcc& i) { a->Cset(W(i.d), C(i.cc)); }
  void emit(const shl& i);
  void emit(const sqrtsd& i) { a->Fsqrt(D(i.d), D(i.s)); }
  void emit(const srem& i);
  void emit(const storeb& i) { a->Strb(W(i.s), M(i.m)); }
  void emit(const storel& i) { a->Str(W(i.s), M(i.m)); }
  void emit(const storesd& i) { emit(store{i.s, i.m}); }
  void emit(const storeups& i);
  void emit(const storew& i) { a->Strh(W(i.s), M(i.m)); }
  void emit(const subbi& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const subl& i) { a->Sub(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const subli& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const subq& i) { a->Sub(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const subqi& i) { a->Sub(X(i.d), X(i.s1), i.s0.q(), SetFlags); }
  void emit(const subsd& i) { a->Fsub(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const testl& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testli& i);
  void emit(const testq& i) { a->Tst(X(i.s1), X(i.s0)); }
  void emit(const testqi& i) { a->Tst(X(i.s1), i.s0.q()); }
  void emit(const ucomisd& i) { a->Fcmp(D(i.s0), D(i.s1)); }
  void emit(const ud2& i) { a->Brk(1); }
  void emit(const unpcklpd&);
  void emit(const xorb& i);
  void emit(const xorbi& i);
  void emit(const xorl& i);
  void emit(const xorq& i);
  void emit(const xorqi& i);

  // arm intrinsics
  void emit(const addxi& i) { a->Add(X(i.d), X(i.s1), i.s0.q()); }
  void emit(const asrxi& i);
  void emit(const asrxis& i);
  void emit(const bln& i);
  void emit(const cmplims& i);
  void emit(const cmpsds& i);
  void emit(const fabs& i) { a->Fabs(D(i.d), D(i.s)); }
  void emit(const fcvtzs& i) {a->Fcvtzs(X(i.d), D(i.s));}
  void emit(const lslwi& i);
  void emit(const lslwis& i);
  void emit(const lslxi& i);
  void emit(const lslxis& i);
  void emit(const lsrwi& i);
  void emit(const lsrwis& i);
  void emit(const lsrxi& i);
  void emit(const lsrxis& i);
  void emit(const mrs& i) { a->Mrs(X(i.r), vixl::SystemRegister(i.s.l())); }
  void emit(const msr& i) { a->Msr(vixl::SystemRegister(i.s.l()), X(i.r)); }
  void emit(const orsw& i);
  void emit(const orswi& i);
  void emit(const popp& i);
  void emit(const pushp& i);
  void emit(const subsb& i) { a->Sub(W(i.d), W(i.s1), W(i.s0), SetFlags); }
  void emit(const uxth& i) { a->Uxth(W(i.d), W(i.s)); }

  void emit_nop() { a->Nop(); }

private:
  CodeBlock& frozen() { return env.text.frozen().code; }

private:
  Venv& env;
  vixl::MacroAssembler assem;
  vixl::MacroAssembler* a;
  Address base;

  const Vlabel current;
  const Vlabel next;
  jit::vector<Venv::LabelPatch>& jmps;
  jit::vector<Venv::LabelPatch>& jccs;
  jit::vector<Venv::LabelPatch>& catches;
};

///////////////////////////////////////////////////////////////////////////////

void Vgen::patch(Venv& env) {
  for (auto& p : env.jmps) {
    assertx(env.addrs[p.target]);
    // 'jmp' is 2 instructions, load followed by branch
    auto const begin = reinterpret_cast<char*>(p.instr + 2 * 4);
    auto const end = begin + sizeof(env.addrs[p.target]);
    *reinterpret_cast<TCA*>(begin) = env.addrs[p.target];
    __builtin___clear_cache(begin, end);
  }
  for (auto& p : env.jccs) {
    assertx(env.addrs[p.target]);
    // 'jcc' is 3 instructions, b.!cc + load followed by branch
    auto const begin = reinterpret_cast<char*>(p.instr + 3 * 4);
    auto const end = begin + sizeof(env.addrs[p.target]);
    *reinterpret_cast<TCA*>(begin) = env.addrs[p.target];
    __builtin___clear_cache(begin, end);
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
  // Assembler::fmov emits a ldr from a literal pool if IsImmFP64 is false.
  // In that case, emit the raw bits into a GPR first and then move them
  // unmodified into destination SIMD
  union { double dval; int64_t ival; };
  ival = val;
  if (vixl::Assembler::IsImmFP64(dval)) {
    a->Fmov(D(d), dval);
  } else if (ival == 0) {
    a->Fmov(D(d), vixl::xzr);
  } else {
    a->Mov(rAsm, ival);
    a->Fmov(D(d), rAsm);
  }
}

#define Y(vasm_opc, simd_w, vr_w, gpr_w, imm) \
void Vgen::emit(const vasm_opc& i) {          \
  if (i.d.isSIMD()) {                         \
    emitSimdImmInt(a, i.s.simd_w(), i.d);     \
  } else {                                    \
    Vreg##vr_w d = i.d;                       \
    a->Mov(gpr_w(d), imm);                    \
  }                                           \
}

Y(ldimmb, ub, 8, W, MSKTOP(i.s.l()))
Y(ldimmw, w, 16, W, MSKTOP(i.s.l()))
Y(ldimml, l, 32, W, MSKTOP(i.s.l()))
Y(ldimmq, q, 64, X, i.s.q())

#undef Y

void Vgen::emit(const ldimmqs& i) {
  emitSmashableMovq(a->code(), env.meta, i.s.q(), i.d);
}

void Vgen::emit(const load& i) {
  if (i.d.isGP()) {
    a->Ldr(X(i.d), M(i.s));
  } else {
    a->Ldr(D(i.d), M(i.s));
  }
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    if (i.s == rsp()) {
      a->Mov(rAsm, X(i.s));
      a->Str(rAsm, M(i.d));
    } else {
      a->Str(X(i.s), M(i.d));
    }
  } else {
    a->Str(D(i.s), M(i.d));
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
  auto const mov_addr = emitSmashableMovq(a->code(), env.meta, 0, r64(i.d));
  auto const imm = reinterpret_cast<uint64_t>(mov_addr);
  smashMovq(mov_addr, (imm << 1) | 1);

  env.meta.addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const call& i) {
  a->Mov(rAsm, reinterpret_cast<uint64_t>(i.target));
  a->Blr(rAsm);
  if (i.watch) {
    *i.watch = a->frontier();
    env.meta.watchpoints.push_back(i.watch);
  }
}

void Vgen::emit(const callm& i) {
  a->Ldr(rAsm, M(i.target));
  a->Blr(rAsm);
}

void Vgen::emit(const calls& i) {
  emitSmashableCall(a->code(), env.meta, i.target);
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const stublogue& i) {
  // Push FP, LR always regardless of i.saveframe (makes SP 16B aligned)
  emit(pushp{rfp(), rlr()});
}

void Vgen::emit(const stubret& i) {
  if (i.saveframe) {
    emit(popp{rfp(), rlr()});
  } else {
    emit(popp{PhysReg(rAsm), rlr()});
  }
  emit(ret{});
}

void Vgen::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}

void Vgen::emit(const tailcallstub& i) {
  // Pop off FP/LR pair and jump to target
  emit(popp{rfp(), rlr()});
  emit(jmpi{i.target, i.args});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const phplogue& i) {
  // Save LR in m_savedRip on the current VM frame pointed by 'i.fp'
  a->Str(X(rlr()), X(i.fp)[AROFF(m_savedRip)]);
}

void Vgen::emit(const phpret& i) {
  a->Ldr(X(rlr()), X(i.fp)[AROFF(m_savedRip)]);
  if (!i.noframe) {
    a->Ldr(X(i.d), X(i.fp)[AROFF(m_sfp)]);
  }
  emit(ret{});
}

void Vgen::emit(const callphp& i) {
  emitSmashableCall(a->code(), env.meta, i.stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const tailcallphp& i) {
  // To make callee's return as caller's return, load the return address at
  // i.fp[AROFF(m_savedRip)] into LR and jmp to target
  a->Ldr(X(rlr()), X(i.fp)[AROFF(m_savedRip)]);
  emit(jmpr{i.target, i.args});
}

void Vgen::emit(const callarray& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const contenter& i) {
  vixl::Label End;

  a->Adr(rAsm, &End);
  a->Str(rAsm, X(i.fp)[AROFF(m_savedRip)]);
  a->Blr(X(i.target));
  a->bind(&End);
  // m_savedRip will point here.
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const leavetc& i) {
  emit(popp{PhysReg(rAsm), rlr()});
  emit(ret{});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const nothrow& i) {
  env.meta.catches.emplace_back(a->frontier(), nullptr);
}

void Vgen::emit(const syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  env.meta.fixups.emplace_back(a->frontier(), i.fix);
}

void Vgen::emit(const unwind& i) {
  catches.push_back({a->frontier(), i.targets[1]});
  emit(jmp{i.targets[0]});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const andbi& i) {
  a->And(W(i.d), W(i.s1), MSKTOP(i.s0.l()), SetFlags);
}

void Vgen::emit(const andli& i) {
  a->And(W(i.d), W(i.s1), MSKTOP(i.s0.l()), SetFlags);
}

void Vgen::emit(const testli& i) {
  a->Tst(W(i.s1), MSKTOP(i.s0.l()));
}

void Vgen::emit(const cloadq& i) {
  a->Ldr(rAsm, M(i.t));
  a->Csel(X(i.d), rAsm, X(i.f), C(i.cc));
}

/*
 * Flags
 *   SF should be set to MSB of the result
 *   CF, OF should be set to (1, 1) if the result is truncated, (0, 0) otherwise
 *   ZF, AF, PF are undefined
 *
 * In the following implementation,
 *   N, Z, V are updated according to result
 *   C is cleared (FIXME)
 *   PF, AF are not available
 */
void Vgen::emit(const imul& i) {

  // Do the multiplication
  a->Mul(X(i.d), X(i.s0), X(i.s1));

  vixl::Label after;
  vixl::Label checkSign;
  vixl::Label Overflow;

  // Do the multiplication for the upper 64 bits of a 128 bit result.
  // If the result is not all zeroes or all ones, then we have overflow.
  // If the result is all zeroes or all ones, and the sign is the same,
  // for both hi and low, then there is no overflow.
  a->smulh(rAsm, X(i.s0), X(i.s1));

  // If hi is all 0's or 1's, then check the sign, else overflow (fallthrough)
  a->Cmp(rAsm, 0);
  a->B(&checkSign, vixl::eq);
  a->Cmp(rAsm, -1);
  a->B(&checkSign, vixl::eq);

  // Overflow, so conditionally set N and Z bits and then or in V bit.
  a->Bind(&Overflow);
  a->Bic(vixl::xzr, X(i.d), vixl::xzr, SetFlags);
  a->Mrs(rAsm, NZCV);
  a->Orr(rAsm, rAsm, 1<<28);
  a->Msr(NZCV, rAsm);
  a->B(&after);

  // Check the signs of hi and lo.
  a->Bind(&checkSign);
  a->Eor(rAsm, rAsm, X(i.d));
  a->Tbnz(rAsm, 63, &Overflow);

  // No Overflow, so conditionally set the N and Z only
  a->Bic(vixl::xzr, X(i.d), vixl::xzr, SetFlags);

  a->bind(&after);
}

#define Y(vasm_opc, arm_opc)           \
void Vgen::emit(const vasm_opc& i) {   \
  auto adr = M(i.m);                   \
  vixl::Label again;                   \
  a->bind(&again);                     \
  a->ldxr(rAsm, adr);                  \
  a->arm_opc(rAsm, rAsm, 1, SetFlags); \
  a->stxr(rAsm.W(), rAsm, adr);        \
  a->Cbnz(rAsm.W(), &again);           \
}

Y(incqmlock, Add)
Y(decqmlock, Sub)

#undef Y

void Vgen::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});
    vixl::Label skip, data;

    a->B(&skip, vixl::InvertCondition(C(i.cc)));
    a->Ldr(rAsm, &data);
    a->Br(rAsm);
    a->bind(&data);
    a->dc64(a->frontier());
    a->bind(&skip);
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const jcci& i) {
  vixl::Label skip;

  a->B(&skip, vixl::InvertCondition(C(i.cc)));
  emit(jmpi{i.taken});
  a->bind(&skip);
  emit(jmp{i.target});
}

void Vgen::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  vixl::Label data;
  a->Ldr(rAsm, &data);
  a->Br(rAsm);
  a->bind(&data);
  a->dc64(a->frontier());
}

void Vgen::emit(const jmpi& i) {
  vixl::Label data;

  // If target can be addressed by pc relative offset (signed 26 bits), emit
  // PC relative jump. Else, emit target address into code and load from there
  auto diff = (i.target - a->frontier()) >> vixl::kInstructionSizeLog2;
  if (vixl::is_int26(diff)) {
    a->b(diff);
  } else {
    a->Ldr(rAsm, &data);
    a->Br(rAsm);
    a->bind(&data);
    a->dc64(i.target);
  }
}

void Vgen::emit(const jmpm& i) {
  a->Ldr(rAsm, M(i.target));
  a->Br(rAsm);
}

void Vgen::emit(const lea& i) {
  auto p = i.s;
  assertx(p.base.isValid());
  if (p.index.isValid()) {
    assertx(p.disp == 0);
    a->Add(X(i.d), X(p.base), Operand(X(p.index), LSL, Log2(p.scale)));
  } else {
    assertx(p.disp >= -256 && p.disp <= 255);
    a->Add(X(i.d), X(p.base), p.disp);
  }
}

void Vgen::emit(const loadqp& i) {
  a->Mov(X(i.d), i.s.r.disp);
  a->Ldr(X(i.d), X(i.d)[0]);
}

void Vgen::emit(const loadqd& i) {
  a->Mov(X(i.d), reinterpret_cast<uint64_t>(i.s.get()));
  a->Ldr(X(i.d), X(i.d)[0]);
}

#define Y(vasm_opc, arm_opc, src_dst, m)                             \
void Vgen::emit(const vasm_opc& i) {                                 \
  assertx(i.m.base.isValid());                                       \
  a->Mov(rAsm, X(i.m.base));                                         \
  if (i.m.index.isValid()) {                                         \
    a->Add(rAsm, rAsm, Operand(X(i.m.index), LSL, Log2(i.m.scale))); \
  }                                                                  \
  if (i.m.disp != 0) {                                               \
    a->Add(rAsm, rAsm, i.m.disp);                                    \
  }                                                                  \
  a->arm_opc(V(i.src_dst), MemOperand(rAsm));                        \
}

Y(loadups, ld1, d, s)
Y(storeups, st1, s, m)

#undef Y

/*
 * Flags
 *   SF, ZF, PF should be updated according to result
 *   CF, OF should be cleared
 *   AF is undefined
 *
 * In the following implementation,
 *   N, Z are updated according to result
 *   C, V are cleared
 *   PF, AF are not available
 */
#define Y(vasm_opc, arm_opc, gpr_w, s0, zr)         \
void Vgen::emit(const vasm_opc& i) {                \
  a->arm_opc(gpr_w(i.d), gpr_w(i.s1), s0);          \
  a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags); \
}

Y(orqi, Orr, X, i.s0.q(), xzr);
Y(orq, Orr, X, X(i.s0), xzr);
Y(orswi, Orr, W, MSKTOP(i.s0.l()), wzr);
Y(orsw, Orr, W, W(i.s0), wzr);
Y(xorb, Eor, W, W(i.s0), wzr);
Y(xorbi, Eor, W, MSKTOP(i.s0.l()), wzr);
Y(xorl, Eor, W, W(i.s0), wzr);
Y(xorq, Eor, X, X(i.s0), xzr);
Y(xorqi, Eor, X, i.s0.q(), xzr);

#undef Y

void Vgen::emit(const pop& i) {
  // SP access must be 8 byte aligned. Use rAsm instead
  a->Mov(rAsm, sp);
  a->Ldr(X(i.d), MemOperand(rAsm, 8, PostIndex));
  a->Mov(sp, rAsm);
}

void Vgen::emit(const psllq& i) {
  // TODO: Add simd shift support in vixl
  a->Fmov(rAsm, D(i.s1));
  a->Lsl(rAsm, rAsm, i.s0.l());
  a->Fmov(D(i.d), rAsm);
}

void Vgen::emit(const psrlq& i) {
  // TODO: Needs simd shift support in vixl
  a->Fmov(rAsm, D(i.s1));
  a->Lsr(rAsm, rAsm, i.s0.l());
  a->Fmov(D(i.d), rAsm);
}

void Vgen::emit(const push& i) {
  // SP access must be 8 byte aligned. Use rAsm instead
  a->Mov(rAsm, sp);
  a->Str(X(i.s), MemOperand(rAsm, -8, PreIndex));
  a->Mov(sp, rAsm);
}

void Vgen::emit(const roundsd& i) {
  switch (i.dir) {
    case RoundDirection::nearest: {
      a->frintn(D(i.d), D(i.s));
      break;
    }

    case RoundDirection::floor: {
      a->frintm(D(i.d), D(i.s));
      break;
    }

    case RoundDirection:: ceil: {
      a->frintp(D(i.d), D(i.s));
      break;
    }

    default: {
      assertx(i.dir == RoundDirection::truncate);
      a->frintz(D(i.d), D(i.s));
    }
  }
}

/*
 * N, Z are updated according to result
 * C, V are cleared (FIXME)
 * PF, AF are not available
 */
#define Y(vasm_opc, arm_opc)                      \
void Vgen::emit(const vasm_opc& i) {              \
  a->arm_opc(X(i.d), X(i.s1), X(i.s0));           \
  a->Bic(vixl::xzr, X(i.d), vixl::xzr, SetFlags); \
}

Y(sar, Asr)
Y(shl, Lsl)

#undef Y

void Vgen::emit(const srem& i) {
  a->Sdiv(rAsm, X(i.s0), X(i.s1));
  a->Msub(X(i.d), rAsm, X(i.s1), X(i.s0));
}

void Vgen::emit(const unpcklpd& i) {
  // i.d and i.s1 can be same, i.s0 is unique
  if (i.d != i.s1) a->fmov(D(i.d), D(i.s1));
  a->fmov(rAsm, D(i.s0));
  a->fmov(D(i.d), 1, rAsm);
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const bln& i) {
  vixl::Label stub;

  a->Bl(&stub);
  a->bind(&stub);
}

/*
 * 'cmplims' instruction is intended for use where temporary Vregs can't be
 * allocated, E.g., in stubs called via callfaststub{}. As the memory
 * operand is not lowered, make sure that it is in one of the following
 * formats
 *  [base]
 *  [base, index]
 *  [base, #imm] where #imm is in the range [-256, 255]
 */
void Vgen::emit(const cmplims& i) {
  enum {
    BASE = 1,
    INDEX = 2,
    DISP = 4
  };

  // TODO: Allow TLS addresses if possible
  assertx(i.s1.seg == Vptr::DS);

  uint8_t mode = (((i.s1.base.isValid()  & 0x1) << 0) |
                  ((i.s1.index.isValid() & 0x1) << 1) |
                  (((i.s1.disp != 0)     & 0x1) << 2));
  switch (mode) {
    case BASE:
    case BASE | INDEX:
      break;
    case BASE | DISP:
      assertx(i.s1.disp >= -256 && i.s1.disp <= 255);
      break;
    default:
      always_assert(false);
  }
  a->Ldr(rAsm.W(), M(i.s1));
  a->Cmp(rAsm.W(), i.s0.l());
}

void Vgen::emit(const cmpsds& i) {
  // Updates flags
  a->Fcmp(D(i.s0), D(i.s1));
  switch (i.pred) {
    case ComparisonPred::eq_ord: {
      a->Csetm(rAsm, C(jit::CC_E));
      break;
    }

    case ComparisonPred::ne_unord: {
      a->Csetm(rAsm, C(jit::CC_NE));
      break;
    }

    default: {
      always_assert(false);
    }
  }
  a->Fmov(D(i.d), rAsm);
}

#define Y(vasm_opc, arm_opc, gpr_w)              \
void Vgen::emit(const vasm_opc& i) {             \
  a->arm_opc(gpr_w(i.d), gpr_w(i.s1), i.s0.l()); \
}

Y(asrxi, Asr, X)
Y(lslwi, Lsl, W)
Y(lslxi, Lsl, X)
Y(lsrwi, Lsr, W)
Y(lsrxi, Lsr, X)

#undef Y

/*
 * Flags for shift instructions
 *   SF, ZF, PF should be updated according to result
 *   CF should be the last bit shifted out of the operand
 *   OF is defined only if 'count' is 1
 *     For left shifts, OF should be set to 0 if the MSB of result is same as CF
 *     (i.e., the top 2 bits of the operand are same). OF is set to 1 otherwise.
 *     For SAR, OF should be set to 0. For SHR, OF should be set to MSB of
 *     original operand
 *   AF is undefined
 *
 * In the following implementation,
 *   N, Z are updated according to result
 *   C is updated with the shifted out bit flipped
 *     Note: Flipping is needed to make jcc{CC_BE} and jcc{CC_A} work
 *   V is cleared (FIXME)
 *   PF, AF are not available
 */
#define Y(vasm_opc, arm_opc, gpr_w, zr)                 \
void Vgen::emit(const vasm_opc& i) {                    \
  a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags);     \
  a->Mrs(rAsm, NZCV);                                   \
  auto d = i.s1;                                        \
  if (i.s0.l() > 1) {                                   \
    a->arm_opc(gpr_w(i.df), gpr_w(i.s1), i.s0.l() - 1); \
    d = i.df;                                           \
  }                                                     \
  a->mvn(W(d).X(), W(d).X());                           \
  a->bfm(rAsm, W(d).X(), 35, 0);                        \
  a->Msr(NZCV, rAsm);                                   \
}

Y(asrxis, Asr, X, xzr)
Y(lsrwis, Lsr, W, wzr)
Y(lsrxis, Lsr, X, xzr)

#undef Y

#define Y(vasm_opc, gpr_w, zr, sz)                  \
void Vgen::emit(const vasm_opc& i) {                \
  a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags); \
  a->Mrs(rAsm, NZCV);                               \
  a->Lsr(gpr_w(i.df), gpr_w(i.s1), sz - i.s0.l());  \
  a->mvn(W(i.df).X(), W(i.df).X());                 \
  a->bfm(rAsm, W(i.df).X(), 35, 0);                 \
  a->Msr(NZCV, rAsm);                               \
}

Y(lslwis, W, wzr, 32)
Y(lslxis, X, xzr, 64)

#undef Y

void Vgen::emit(const popp& i) {
  a->Ldp(X(i.d0), X(i.d1), MemOperand(sp, 16, PostIndex));
}

void Vgen::emit(const pushp& i) {
  a->Stp(X(i.s0), X(i.s1), MemOperand(sp, -16, PreIndex));
}

///////////////////////////////////////////////////////////////////////////////

template<typename Lower>
void lower_impl(Vunit& unit, Vlabel b, size_t i, Lower lower) {
  vmodify(unit, b, i, [&] (Vout& v) { lower(v); return 1; });
}

template<typename Inst>
void lower(Vunit& unit, Inst& inst, Vlabel b, size_t i) {}

///////////////////////////////////////////////////////////////////////////////

/*
 * TODO: Using load size (ldr[bh]?), apply scaled address if 'disp' is unsigned
 */
void lowerVptr(Vptr& p, Vout& v) {
  enum {
    BASE = 1,
    INDEX = 2,
    DISP = 4
  };

  uint8_t mode = (((p.base.isValid()  & 0x1) << 0) |
                  ((p.index.isValid() & 0x1) << 1) |
                  (((p.disp != 0)     & 0x1) << 2));
  switch (mode) {
    case BASE:
    case BASE | INDEX:
      // ldr/str allow [base] and [base, index], nothing to lower
      break;

    case INDEX:
      // Not supported, convert to [base]
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << lslxi{Log2(p.scale), p.index, t};
        p.base = t;
      } else {
        p.base = p.index;
      }
      p.index = Vreg{};
      p.scale = 1;
      break;

    case BASE | DISP: {
      // ldr/str allow [base, #imm], where #imm is [-256 .. 255]
      if (p.disp >= -256 && p.disp <= 255)
        break;

      // #imm is out of range, convert to [base, index]
      auto index = v.makeReg();
      v << ldimmq{Immed64(p.disp), index};
      p.index = index;
      p.scale = 1;
      p.disp = 0;
      break;
    }

    case DISP: {
      // Not supported, convert to [base]
      auto base = v.makeReg();
      v << ldimmq{Immed64(p.disp), base};
      p.base = base;
      p.index = Vreg{};
      p.scale = 1;
      p.disp = 0;
      break;
    }

    case INDEX | DISP:
      // Not supported, convert to [base, #imm] or [base, index]
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << lslxi{Log2(p.scale), p.index, t};
        p.base = t;
      } else {
        p.base = p.index;
      }
      if (p.disp >= -256 && p.disp <= 255) {
        p.index = Vreg{};
        p.scale = 1;
      } else {
        auto index = v.makeReg();
        v << ldimmq{Immed64(p.disp), index};
        p.index = index;
        p.scale = 1;
        p.disp = 0;
      }
      break;

    case BASE | INDEX | DISP: {
      // Not supported, convert to [base, index]
      auto index = v.makeReg();
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << lslxi{Log2(p.scale), p.index, t};
        v << addxi{p.disp, t, index};
      } else {
        v << addxi{p.disp, p.index, index};
      }
      p.index = index;
      p.scale = 1;
      p.disp = 0;
      break;
    }
  }
}

#define Y(vasm_opc, m)                                  \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    lowerVptr(i.m, v);                                  \
    v << i;                                             \
  });                                                   \
}

Y(callm, target)
Y(cloadq, t)
Y(decqmlock, m)
Y(incqmlock, m)
Y(jmpm, target)
Y(lea, s)
Y(loadb, s)
Y(loadl, s)
Y(load, s)
Y(loadsd, s)
Y(loadtqb, s)
Y(loadups, s)
Y(loadw, s)
Y(loadzbl, s)
Y(loadzbq, s)
Y(loadzlq, s)
Y(storeb, m)
Y(store, d)
Y(storel, m)
Y(storesd, m)
Y(storeups, m)
Y(storew, m)

#undef Y

void lower(Vunit& u, absdbl& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto s = v.makeReg(), d = v.makeReg();
    v << copy{i.s, s};
    v << fabs{s, d};
    v << copy{d, i.d};
  });
}

#define ISAR vixl::Assembler::IsImmArithmetic(value)
#define ISLG(w) vixl::Assembler::IsImmLogical(value, w)
#define U2(v0, v1) v0, v1

#define Y(vasm_opc, lower_opc, load_opc, chk, imm, use) \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    auto value = safe_cast<int64_t>(i.s0.q());          \
    if (chk) {                                          \
      v << i;                                           \
    } else {                                            \
      auto s0 = v.makeReg();                            \
      v << load_opc{imm, s0};                           \
      v << lower_opc{s0, use, i.sf};                    \
    }                                                   \
  });                                                   \
}

Y(addli, addl, ldimml, ISAR, i.s0, U2(i.s1, i.d))
Y(addqi, addq, ldimmq, ISAR, Immed64(value), U2(i.s1, i.d))
Y(andbi, andb, ldimmb, ISLG(32), i.s0, U2(i.s1, i.d))
Y(andli, andl, ldimml, ISLG(32), i.s0, U2(i.s1, i.d))
Y(andqi, andq, ldimmq, ISLG(64), Immed64(value), U2(i.s1, i.d))
Y(cmpli, cmpl, ldimml, ISAR, i.s0, i.s1)
Y(cmpqi, cmpq, ldimmq, ISAR, Immed64(value), i.s1)
Y(orqi, orq, ldimmq, ISLG(64), Immed64(value), U2(i.s1, i.d))
Y(orswi, orsw, ldimml, ISLG(32), i.s0, U2(i.s1, i.d))
Y(subbi, subsb, ldimmb, ISAR, i.s0, U2(i.s1, i.d))
Y(subli, subl, ldimml, ISAR, i.s0, U2(i.s1, i.d))
Y(subqi, subq, ldimmq, ISAR, Immed64(value), U2(i.s1, i.d))
Y(testli, testl, ldimml, ISLG(32), i.s0, i.s1)
Y(testqi, testq, ldimmq, ISLG(64), Immed64(value), i.s1)
Y(xorbi, xorb, ldimmb, ISLG(32), i.s0, U2(i.s1, i.d))
Y(xorqi, xorq, ldimmq, ISLG(64), Immed64(value), U2(i.s1, i.d))

#undef Y

#undef U2
#undef ISLG
#undef ISAR

#define Y(vasm_opc, lower_opc, load_opc, store_opc, s0, m) \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) {    \
  lower_impl(u, b, z, [&] (Vout& v) {                      \
    lowerVptr(i.m, v);                                     \
    auto r0 = v.makeReg(), r1 = v.makeReg();               \
    v << load_opc{i.m, r0};                                \
    v << lower_opc{i.s0, r0, r1, i.sf};                    \
    v << store_opc{r1, i.m};                               \
  });                                                      \
}

Y(addlim, addli, loadl, storel, s0, m)
Y(addlm, addl, loadl, storel, s0, m)
Y(addqim, addqi, load, store, s0, m)
Y(andbim, andbi, loadb, storeb, s, m)
Y(orbim, orswi, loadb, storeb, s0, m)
Y(orqim, orqi, load, store, s0, m)
Y(orwim, orswi, loadw, storew, s0, m)

#undef Y

#define Y(vasm_opc, lower_opc, load_opc)                \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    lowerVptr(i.s1, v);                                 \
    auto r = v.makeReg();                               \
    v << load_opc{i.s1, r};                             \
    v << lower_opc{i.s0, r, i.sf};                      \
  });                                                   \
}

Y(cmpbim, cmpli, loadb)
Y(cmplim, cmpli, loadl)
Y(cmplm, cmpl, loadl)
Y(cmpqim, cmpqi, load)
Y(cmpqm, cmpq, load)
Y(cmpwim, cmpli, loadw)
Y(testbim, testli, loadb)
Y(testlim, testli, loadl)
Y(testqim, testqi, load)
Y(testqm, testq, load)
Y(testwim, testli, loadw)

#undef Y

void lower(Vunit& u, cmpsd& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    // Save and restore the flags register
    auto r = v.makeReg();
    v << mrs{NZCV, r};
    v << cmpsds {i.pred, i.s0, i.s1, i.d, v.makeReg()};
    v << msr{r, NZCV};
  });
}

void lower(Vunit& u, cvtsi2sdm& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    lowerVptr(i.s, v);
    auto r = v.makeReg();
    v << load{i.s, r};
    v << cvtsi2sd{r, i.d};
  });
}

#define Y(vasm_opc, lower_opc, load_opc, store_opc, m)  \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    lowerVptr(i.m, v);                                  \
    auto r0 = v.makeReg(), r1 = v.makeReg();            \
    v << load_opc{i.m, r0};                             \
    v << lower_opc{r0, r1, i.sf};                       \
    v << store_opc{r1, i.m};                            \
  });                                                   \
}

Y(declm, decl, loadl, storel, m)
Y(decqm, decq, load, store, m)
Y(inclm, incl, loadl, storel, m)
Y(incqm, incq, load, store, m)
Y(incwm, incw, loadw, storew, m)

#undef Y

void lower(Vunit& unit, cvttsd2siq& i, Vlabel b, size_t idx) {
  lower_impl(unit, b, idx, [&] (Vout& v) {
    // Clear FPSR IOC flag.
    auto const tmp1 = v.makeReg();
    auto const tmp2 = v.makeReg();
    v << mrs{FPSR, tmp1};
    v << andqi{~0x01, tmp1, tmp2, v.makeReg()};
    v << msr{tmp2, FPSR};

    // Load error value
    auto const err = v.makeReg();
    v << ldimmq{0x8000000000000000, err};

    // Do ARM64's double to signed int64 conversion.
    auto const res = v.makeReg();
    v << fcvtzs{i.s, res};

    // Check if there was a conversion error.
    auto const fpsr = v.makeReg();
    auto const sf = v.makeReg();
    v << mrs{FPSR, fpsr};
    v << testqi{1, fpsr, sf};

    // Move converted value or error.
    v << cmovq{CC_NZ, sf, res, err, i.d};
  });
}

void lower(Vunit& u, stubunwind& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    v << lea{rsp()[16], rsp()};
  });
}

void lower(Vunit& u, stubtophp& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    v << lea{rsp()[16], rsp()};
  });
}

void lower(Vunit& u, loadstubret& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    v << load{rsp()[8], i.d};
  });
}

void lower(Vunit& u, calltc& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    // Push FP, LR for callToExit(..)
    auto r0 = v.makeReg();
    auto r1 = v.makeReg();
    v << load{i.fp[AROFF(m_savedRip)], r0};
    v << ldimmq{i.exittc, r1};
    v << pushp{r0, r1};

    // Emit call to next instruction to balance predictor's stack
    v << bln{};

    // Set the return address to savedRip and jump to target
    v << copy{r0, rlr()};
    v << jmpr{i.target, i.args};
  });
}

void lower(Vunit& u, resumetc& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    // Push FP, LR for callToExit(..)
    auto r = v.makeReg();
    v << ldimmq{i.exittc, r};
    v << pushp{rfp(), r};

    // Call the helper
    v << callr{i.target, i.args};
    v << jmpi{i.exittc};
  });
}

void lower(Vunit& u, popm& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto r = v.makeReg();
    v << pop{r};
    lowerVptr(i.d, v);
    v << store{r, i.d};
  });
}

void lower(Vunit& u, pushm& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto r = v.makeReg();
    lowerVptr(i.s, v);
    v << load{i.s, r};
    v << push{r};
  });
}

void lower(Vunit& u, movtdb& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto d = v.makeReg();
    v << copy{i.s, d};
    v << movtqb{d, i.d};
  });
}

void lower(Vunit& u, movtdq& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    v << copy{i.s, i.d};
  });
}

void lower(Vunit& u, cmpb& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto s0 = v.makeReg();
    auto s1 = v.makeReg();
    v << movzbl{i.s0, s0};
    v << movzbl{i.s1, s1};
    v << cmpl{s0, s1, i.sf};
  });
}

void lower(Vunit& u, cmpbi& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto s1 = v.makeReg();
    v << movzbl{i.s1, s1};
    v << cmpli{i.s0, s1, i.sf};
  });
}

#define Y(vasm_opc, conv_opc, load_opc, cmp_opc)        \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    auto s0 = v.makeReg();                              \
    v << conv_opc{i.s0, s0};                            \
    lowerVptr(i.s1, v);                                 \
    auto s1 = v.makeReg();                              \
    v << load_opc{i.s1, s1};                            \
    v << cmp_opc{s0, s1, i.sf};                         \
  });                                                   \
}

Y(cmpbm, movzbl, loadzbl, cmpl)
Y(cmpwm, uxth, loadw, cmpl)

#undef Y

void lower(Vunit& u, testb& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto s0 = v.makeReg();
    auto s1 = v.makeReg();
    v << movzbl{i.s0, s0};
    v << movzbl{i.s1, s1};
    v << testl{s0, s1, i.sf};
  });
}

void lower(Vunit& u, testbi& i, Vlabel b, size_t z) {
  lower_impl(u, b, z, [&] (Vout& v) {
    auto s1 = v.makeReg();
    v << movzbl{i.s1, s1};
    v << testli{i.s0, s1, i.sf};
  });
}

/*
 * Shift instructions were split into 2, one does the actual shifting and the
 * other updates the flags. Following code makes a copy of the original
 * register, outputs the shift instruction followed by flags update instruction.
 * If the 'SF' register is not used by subsequent code, the copy and flags
 * update instructions are removed by the dead code elimination pass
 */
#define Y(vasm_opc, lower_opc, sf_opc)                  \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    auto r = v.makeReg(), d = v.makeReg();              \
    v << copy{i.s1, r};                                 \
    v << lower_opc{i.s0, i.s1, i.d};                    \
    v << sf_opc{i.s0, r, i.d, d, i.sf};                 \
  });                                                   \
}

Y(sarqi, asrxi, asrxis)
Y(shlli, lslwi, lslwis)
Y(shlqi, lslxi, lslxis)
Y(shrli, lsrwi, lsrwis)
Y(shrqi, lsrxi, lsrxis)

#undef Y

#define Y(vasm_opc, lower_opc, load_opc, imm)           \
void lower(Vunit& u, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(u, b, z, [&] (Vout& v) {                   \
    lowerVptr(i.m, v);                                  \
    auto r = v.makeReg();                               \
    v << load_opc{imm, r};                              \
    v << lower_opc{r, i.m};                             \
  });                                                   \
}

Y(storebi, storeb, ldimmb, i.s)
Y(storeli, storel, ldimml, i.s)
Y(storeqi, store, ldimmq, Immed64(i.s.q()))
Y(storewi, storew, ldimmw, i.s)

#undef Y

///////////////////////////////////////////////////////////////////////////////

void lower_vcallarray(Vunit& unit, Vlabel b) {
  auto& code = unit.blocks[b].code;
  // vcallarray can only appear at the end of a block.
  auto const inst = code.back().get<vcallarray>();
  auto const irctx = code.back().irctx();

  auto argRegs = inst.args;
  auto const& srcs = unit.tuples[inst.extraArgs];
  jit::vector<Vreg> dsts;
  for (int i = 0; i < srcs.size(); ++i) {
    dsts.emplace_back(rarg(i));
    argRegs |= rarg(i);
  }

  code.back() = copyargs{unit.makeTuple(srcs), unit.makeTuple(std::move(dsts))};
  code.emplace_back(callarray{inst.target, argRegs}, irctx);
  code.emplace_back(unwind{{inst.targets[0], inst.targets[1]}}, irctx);
}

///////////////////////////////////////////////////////////////////////////////

void lowerForARM(Vunit& unit) {
  Timer timer(Timer::vasm_lower);

  // This pass relies on having no critical edges in the unit.
  splitCriticalEdges(unit);

  // Scratch block can change blocks allocation, hence cannot use regular
  // iterators.
  auto& blocks = unit.blocks;

  PostorderWalker{unit}.dfs([&] (Vlabel ib) {
    assertx(!blocks[ib].code.empty());

    auto& back = blocks[ib].code.back();
    if (back.op == Vinstr::vcallarray) {
      lower_vcallarray(unit, Vlabel{ib});
    }

    for (size_t ii = 0; ii < blocks[ib].code.size(); ++ii) {
      vlower(unit, ib, ii);

      auto& inst = blocks[ib].code[ii];
      switch (inst.op) {
#define O(name, ...)                          \
        case Vinstr::name:                    \
          lower(unit, inst.name##_, ib, ii);  \
          break;

        VASM_OPCODES
#undef O
      }
    }
  });

  printUnit(kVasmLowerLevel, "after lower for ARM", unit);
}

///////////////////////////////////////////////////////////////////////////////
}

void optimizeARM(Vunit& unit, const Abi& abi, bool regalloc) {
  Timer timer(Timer::vasm_optimize);

  removeTrivialNops(unit);
  optimizePhis(unit);
  fuseBranches(unit);
  optimizeJmps(unit);
  optimizeExits(unit);

  assertx(checkWidths(unit));

  lowerForARM(unit);
  simplify(unit);

  if (!unit.constToReg.empty()) {
    foldImms<arm::ImmFolder>(unit);
  }

  optimizeCopies(unit, abi);

  if (unit.needsRegAlloc()) {
    removeDeadCode(unit);
    if (regalloc) allocateRegisters(unit, abi);
  }
  if (unit.blocks.size() > 1) {
    optimizeJmps(unit);
  }
}

void emitARM(Vunit& unit, Vtext& text, CGMeta& fixups,
             AsmInfo* asmInfo) {
  vasm_emit<Vgen>(unit, text, fixups, asmInfo);
}

///////////////////////////////////////////////////////////////////////////////
}}
