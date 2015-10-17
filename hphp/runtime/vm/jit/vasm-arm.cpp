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

vixl::Register W(Vreg32 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::Register W(Vreg8 r) {
  PhysReg pr(r.asReg());
  return x2a(pr).W();
}

vixl::Register X(Vreg64 r) {
  PhysReg pr(r.asReg());
  return x2a(pr);
}

vixl::FPRegister D(Vreg r) {
  return x2simd(r);
}

// convert Vptr to MemOperand
vixl::MemOperand M(Vptr p) {
  assertx(p.base.isValid() && !p.index.isValid());
  return X(p.base)[p.disp];
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
  void emit(const hostcall& i) { a->HostCall(i.argc); }
  void emit(const ldimmq& i);
  void emit(const ldimml& i);
  void emit(const ldimmb& i);
  void emit(const ldimmqs& i) { not_implemented(); }
  void emit(const load& i);
  void emit(const store& i);

  // functions
  void emit(const callr& i) { a->Blr(X(i.target)); }
  void emit(const ret& i) { a->Ret(); }
  void emit(const callphp& i);

  // exceptions
  void emit(const nothrow& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);

  // instructions
  void emit(const addli& i) { a->Add(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const addq& i) { a->Add(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const addqi& i) { a->Add(X(i.d), X(i.s1), i.s0.l(), SetFlags); }
  void emit(const andq& i) { a->And(X(i.d), X(i.s1), X(i.s0) /* flags? */); }
  void emit(const andqi& i) { a->And(X(i.d), X(i.s1), i.s0.l() /* flags? */); }
  void emit(const sar& i) { a->asrv(X(i.d), X(i.s0), X(i.s1)); }
  void emit(const brk& i) { a->Brk(i.code); }
  void emit(cbcc i);
  void emit(const cmpl& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpli& i) { a->Cmp(W(i.s1), i.s0.l()); }
  void emit(const cmpq& i) { a->Cmp(X(i.s1), X(i.s0)); }
  void emit(const cmpqi& i) { a->Cmp(X(i.s1), i.s0.l()); }
  void emit(const decq& i) { a->Sub(X(i.d), X(i.s), 1LL, SetFlags); }
  void emit(const incq& i) { a->Add(X(i.d), X(i.s), 1LL, SetFlags); }
  void emit(jcc i);
  void emit(jmp i);
  void emit(const lea& i);
  void emit(const loadl& i) { a->Ldr(W(i.d), M(i.s)); /* 0-extends? */ }
  void emit(const loadzbl& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const shl& i) { a->lslv(X(i.d), X(i.s0), X(i.s1)); }
  void emit(const movzbl& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(const movzbq& i) { a->Uxtb(W(Vreg32(size_t(i.d))), W(i.s)); }
  void emit(const imul& i) { a->Mul(X(i.d), X(i.s0), X(i.s1)); }
  void emit(const neg& i) { a->Neg(X(i.d), X(i.s), vixl::SetFlags); }
  void emit(const not& i) { a->Mvn(X(i.d), X(i.s)); }
  void emit(const orq& i) { a->Orr(X(i.d), X(i.s1), X(i.s0) /* flags? */); }
  void emit(const orqi& i) { a->Orr(X(i.d), X(i.s1), i.s0.l() /* flags? */); }
  void emit(const storeb& i) { a->Strb(W(i.s), M(i.m)); }
  void emit(const storel& i) { a->Str(W(i.s), M(i.m)); }
  void emit(const setcc& i) { PhysReg r(i.d.asReg()); a->Cset(X(r), C(i.cc)); }
  void emit(const subli& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), SetFlags); }
  void emit(const subq& i) { a->Sub(X(i.d), X(i.s1), X(i.s0), SetFlags); }
  void emit(const subqi& i) { a->Sub(X(i.d), X(i.s1), i.s0.l(), SetFlags); }
  void emit(tbcc i);
  void emit(const testl& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testli& i) { a->Tst(W(i.s1), i.s0.l()); }
  void emit(const ud2& i) { a->Brk(1); }
  void emit(const xorq& i) { a->Eor(X(i.d), X(i.s1), X(i.s0) /* flags? */); }
  void emit(const xorqi& i) { a->Eor(X(i.d), X(i.s1), i.s0.l() /* flags? */); }

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
    smashJmp(p.instr, env.addrs[p.target]);
  }
  for (auto& p : env.jccs) {
    assertx(env.addrs[p.target]);
    smashJcc(p.instr, env.addrs[p.target]);
  }
  for (auto& p : env.bccs) {
    assertx(env.addrs[p.target]);
    auto link = (Instruction*) p.instr;
    link->SetImmPCOffsetTarget(Instruction::Cast(env.addrs[p.target]));
  }
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const callphp& i) {
  emitSmashableCall(*codeBlock, i.stub);
}

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

void Vgen::emit(const ldimmq& i) {
  union { double dval; int64_t ival; };
  ival = i.s.q();
  if (i.d.isSIMD()) {
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
      a->Mov(rAsm, ival); // XXX avoid scratch register somehow.
      a->Fmov(D(i.d), rAsm);
    }
  } else {
    a->Mov(X(i.d), ival);
  }
}

void emitSimdImmInt(vixl::MacroAssembler* a, int64_t val, Vreg d) {
  if (val == 0) {
    a->Fmov(D(d), vixl::xzr);
  } else {
    a->Mov(rAsm, val); // XXX avoid scratch register somehow.
    a->Fmov(D(d), rAsm);
  }
}

void Vgen::emit(const ldimml& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.q(), i.d);
  } else {
    Vreg32 d = i.d;
    a->Mov(W(d), i.s.l());
  }
}

void Vgen::emit(const ldimmb& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.q(), i.d);
  } else {
    Vreg8 d = i.d;
    a->Mov(W(d), i.s.b());
  }
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
    a->Str(X(i.s), M(i.d));
  } else {
    a->Str(D(i.s), M(i.d));
  }
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

void Vgen::emit(jmp i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  // B range is +/- 128MB but this uses BR
  emitSmashableJmp(*codeBlock, kEndOfTargetChain);
}

void Vgen::emit(jcc i) {
  assertx(i.cc != CC_None);
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      // the taken branch is the fall-through block, invert the branch.
      i = jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}};
    }
    jccs.push_back({a->frontier(), i.targets[1]});
    // B.cond range is +/- 1MB but this uses BR
    emitSmashableJcc(*codeBlock, kEndOfTargetChain, i.cc);
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const lea& i) {
  assertx(!i.s.index.isValid());
  assertx(i.s.scale == 1);
  a->Add(X(i.d), X(i.s.base), i.s.disp);
}

void Vgen::emit(cbcc i) {
  assertx(i.cc == vixl::ne || i.cc == vixl::eq);
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      // the taken branch is the fall-through block, invert the branch.
      i = cbcc{i.cc == vixl::ne ? vixl::eq : vixl::ne, i.s,
               {i.targets[1], i.targets[0]}};
    }
    bccs.push_back({a->frontier(), i.targets[1]});
    // offset range +/- 1MB
    if (i.cc == vixl::ne) {
      a->cbnz(X(i.s), 0);
    } else {
      a->cbz(X(i.s), 0);
    }
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(tbcc i) {
  assertx(i.cc == vixl::ne || i.cc == vixl::eq);
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      // the taken branch is the fall-through block, invert the branch.
      i = tbcc{i.cc == vixl::ne ? vixl::eq : vixl::ne, i.bit, i.s,
               {i.targets[1], i.targets[0]}};
    }
    bccs.push_back({a->frontier(), i.targets[1]});
    // offset range +/- 32KB
    if (i.cc == vixl::ne) {
      a->tbnz(X(i.s), i.bit, 0);
    } else {
      a->tbz(X(i.s), i.bit, 0);
    }
  }
  emit(jmp{i.targets[0]});
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

void lower(cmpbim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadzbl{i.s1, scratch};
  v << cmpli{i.s0, scratch, i.sf};
}

void lower(cmplim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadl{i.s1, scratch};
  v << cmpli{i.s0, scratch, i.sf};
}

void lower(cmpqm& i, Vout& v) {
  auto scratch = v.makeReg();
  v << load{i.s1, scratch};
  v << cmpq{i.s0, scratch, i.sf};
}

void lower(testbim& i, Vout& v) {
  auto scratch = v.makeReg();
  v << loadzbl{i.s1, scratch};
  v << testli{i.s0, scratch, i.sf};
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
  optimizeExits(unit);
  simplify(unit);
  if (!unit.constToReg.empty()) {
    foldImms<arm::ImmFolder>(unit);
  }
  vlower(unit);
  lowerForARM(unit);
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
