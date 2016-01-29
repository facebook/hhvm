/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015                                   |
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

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/func-guard-ppc64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-internal.h"
#include "hphp/runtime/vm/jit/vasm-lower.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <algorithm>
#include <tuple>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

using namespace ppc64;
using namespace ppc64_asm;

namespace {

///////////////////////////////////////////////////////////////////////////////

/* Parameters for push/pop and keep stack aligned */
constexpr int push_pop_position           = 8;

///////////////////////////////////////////////////////////////////////////////

struct Vgen {
  explicit Vgen(Venv& env)
    : text(env.text)
    , assem(*env.cb)
    , a(&assem)
    , current(env.current)
    , next(env.next)
    , jmps(env.jmps)
    , jccs(env.jccs)
    , catches(env.catches)
  {}

  static void patch(Venv& env);
  static void pad(CodeBlock& cb);

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(const Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }

  // intrinsics
  void emit(const copy& i) {
    if (i.s == i.d) return;
    if (i.s.isGP()) {
      if (i.d.isGP()) {                     // GP => GP
        a->mr(i.d, i.s);
      } else {                              // GP => XMM
        assertx(i.d.isSIMD());
        a->std(i.s, rsp()[-8]);
        a->lfd(i.d, rsp()[-8]);
      }
    } else {
      assertx(i.s.isSIMD());
      if (i.d.isGP()) {                     // XMM => GP
        a->stfd(i.s, rsp()[-8]);
        a->ld(i.d, rsp()[-8]);
      } else {                              // XMM => XMM
        assertx(i.d.isSIMD());
        a->fmr(i.d, i.s);
      }
    }
  }
  void emit(const copy2& i) {
    assertx(i.s0.isValid() && i.s1.isValid() &&
            i.d0.isValid() && i.d1.isValid());
    auto s0 = i.s0, s1 = i.s1, d0 = i.d0, d1 = i.d1;
    assertx(d0 != d1);
    if (d0 == s1) {
      if (d1 == s0) {
        a->mr(rAsm, s1);
        a->mr(d0, s0);
        a->mr(d1, rAsm);
      } else {
        // could do this in a simplify pass
        if (s1 != d1) a->mr(d1, s1); // save s1 first; d1 != s0
        if (s0 != d0) a->mr(d0, s0);
      }
    } else {
      // could do this in a simplify pass
      if (s0 != d0) a->mr(d0, s0);
      if (s1 != d1) a->mr(d1, s1);
    }
  }
  void emit(const fallthru& i) {}
  void emit(const ldimmb& i);
  void emit(const ldimmw& i) {
    // PPC64 specific. i.d is not Vreg but Vreg16, so it can't be a SIMD.
    a->li(Reg64(i.d), i.s); // should be only 16bits available
  }
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const ldimmqs& i) { emitSmashableMovq(a->code(), i.s.q(), i.d); }
  void emit(const nothrow& i) {
    mcg->registerCatchBlock(a->frontier(), nullptr);
  }
  void emit(const landingpad& i) {}

  // instructions
  void emit(const absdbl& i) {
    a->fabs(RegXMM(int(i.d)), RegXMM(int(i.s)), false);
  }
  void emit(const addl& i) {
    a->addo(Reg64(i.d), Reg64(i.s1), Reg64(i.s0), true);
  }
  void emit(const addq& i) { a->addo(i.d, i.s0, i.s1, true); }

  // Addqi and subqi can't be lowered to addq and subq if the destiny is rsp().
  // To avoid this issue, addqi and subqi are the only vasms that will be
  // lowered directly by using rAsm on its emission.
  //
  // TODO(gut): move them to lower if rsp() manipulation is exclusively allowed
  // in lea and not anymore in addqi and subqi
  void emit(const addqi& i) {
    if (i.s0.fits(sz::word)) {
      a->li(rAsm, i.s0);
    } else {
      a->li32(rAsm, i.s0.l());
    }
    a->addo(i.d, i.s1, rAsm, true);
  }
  void emit(const subqi& i) {
    if (i.s0.fits(sz::word)) {
      a->li(rAsm, i.s0);
    } else {
      a->li32(rAsm, i.s0.l());
    }
    a->subo(i.d, i.s1, rAsm, true);
  }
  void emit(const addsd& i) { a->fadd(i.d, i.s0, i.s1); }
  void emit(const andq& i) { a->and(i.d, i.s0, i.s1, true); }
  void emit(const andqi& i) { a->andi(i.d, i.s1, i.s0); } // andi changes CR0
  void emit(const cmpl& i) { a->cmpw(Reg64(i.s1), Reg64(i.s0)); }
  void emit(const cmpli& i) { a->cmpwi(Reg64(i.s1), i.s0); }
  void emit(const cmpq& i) { a->cmpd(i.s1, i.s0); }
  void emit(const cmpqi& i) { a->cmpdi(i.s1, i.s0); }
  void emit(const xscvdpsxds& i) { a->xscvdpsxds(i.d, i.s); }
  void emit(const xscvsxddp& i) { a->xscvsxddp(i.d, i.s); }
  void emit(const xxlxor& i) { a->xxlxor(i.d, i.s1, i.s0); }
  void emit(const xxpermdi& i) { a->xxpermdi(i.d, i.s1, i.s0); }
  void emit(const mfvsrd& i) { a->mfvsrd(i.d, i.s); }
  void emit(const mtvsrd& i) { a->mtvsrd(i.d, i.s); }
  void emit(const decl& i) { a->subfo(Reg64(i.d), rone(), Reg64(i.s), true); }
  void emit(const decq& i) { a->subfo(i.d, rone(), i.s, true); }
  void emit(const imul& i) { a->mulld(i.d, i.s1, i.s0, true); }
  void emit(const srem& i) {
    // remainder as described on divd documentation:
    a->divd(i.d, i.s0, i.s1);   // i.d = quotient
    a->mulld(i.d, i.d, i.s1);   // i.d = quotient*divisor
    a->subf(i.d, i.d, i.s0);    // i.d = remainder
  }
  void emit(const divint& i) { a->divd(i.d,  i.s0, i.s1, false); }
  void emit(const mulsd& i) { a->fmul(i.d, i.s1, i.s0); }
  void emit(const divsd& i) { a->fdiv(i.d, i.s1, i.s0); }
  void emit(const fcmpo& i) { a->fcmpo(i.sf, i.s1, i.s0); }
  void emit(const fcmpu& i) { a->fcmpu(i.sf, i.s1, i.s0); }
  void emit(const incw& i) { a->addo(Reg64(i.d), Reg64(i.s), rone(), true); }
  void emit(const incl& i) { a->addo(Reg64(i.d), Reg64(i.s), rone(), true); }
  void emit(const incq& i) { a->addo(i.d, i.s, rone(), true); }
  void emit(const jmpi& i) {
    a->branchAuto(i.target, BranchConditions::Always, LinkReg::DoNotTouch);
  }
  void emit(const jmpr& i) {
    a->mtctr(i.target);
    a->bctr();
  }
  void emit(const leap& i) { a->li64(i.d, i.s.r.disp); }
  void emit(const loadups& i) { a->lxvw4x(i.d,i.s); }
  void emit(const mfcr& i) { a->mfcr(i.d); }
  void emit(const mflr& i) { a->mflr(i.d); }
  void emit(const mtlr& i) { a->mtlr(i.s); }
  void emit(const movl& i) {
    int8_t sh = sizeof(int) * CHAR_BIT;
    a->rlwinm(rAsm, Reg64(i.s), 0, 32-sh, 31);
    a->mr(Reg64(i.d), rAsm);
  }
  void emit(const movlk& i) {
    int8_t sh = sizeof(int) * CHAR_BIT;
    a->rlwinm(rAsm, i.s, 0, 32-sh, 31); // extract lower 32bits
    a->clrrwi(i.d, i.d, sh); // clear lower 32bits on destination
    // move lower 32bits to destination and keep the higher 32bits
    a->or(i.d, i.d, rAsm);
  }
  void emit(const movb& i) {
    int8_t sh = CHAR_BIT;
    a->rlwinm(rAsm, Reg64(i.s), 0, 32-sh, 31); // extract lower byte
    a->clrrwi(Reg64(i.d), Reg64(i.d), sh); // clear lower byte on destination
    // move lower byte to destination and keep the other 56 bits
    a->or(Reg64(i.d), Reg64(i.d), rAsm);
  }
  void emit(const movzbl& i) {
    int8_t sh_32 = sizeof(int) * CHAR_BIT;
    int8_t sh_8 = CHAR_BIT;
    a->rlwinm(rAsm, Reg64(i.s), 0, 32-sh_8, 31); // extract lower byte
    // clear lower 32bits on destination
    a->clrrwi(Reg64(i.d), Reg64(i.d), sh_32);
    // move lower byte to destination and keep the other 56 bits
    a->or(Reg64(i.d), Reg64(i.d), rAsm);
  }
  void emit(const movzbq& i) {
    int8_t sh_8 = CHAR_BIT;
    a->rlwinm(rAsm, Reg64(i.s), 0, 32-sh_8, 31); // extract lower byte
    a->mr(i.d, rAsm); // move entire register to reset higher 56bits
  }
  void emit(const extsb& i) {
    a->extsb(i.d, i.s);
  }
  void emit(const extsw& i) {
    a->extsw(i.d, i.s);
  }
  void emit(const neg& i) { a->neg(i.d, i.s, true); }
  void emit(const nop& i) { a->ori(Reg64(0), Reg64(0), 0); } // no-op form
  void emit(const not& i) { a->nor(i.d, i.s, i.s, false); }
  void emit(const orq& i) { a->or(i.d, i.s0, i.s1, true); }
  void emit(const orqi& i) {
    if (i.s0.fits(sz::word)) {
      a->li(rAsm,i.s0);
    } else {
      a->li32(rAsm,i.s0.l());
    }

    a->or(i.d, i.s1, rAsm, true /** or. implies Rc = 1 **/);
  }
  void emit(const roundsd& i) { a->xsrdpi(i.d, i.s); }
  void emit(const ret& i) {
    a->blr();
  }
  /*Immediate-form logical (unsigned) shift operations are
    obtained by specifying appropriate masks and shift values for
    certain Rotate instructions.
  */
  void emit(const sar& i) { a->srad(i.d, i.s1, i.s0, true); }
  void emit(const sarqi& i) { a->sradi(i.d, i.s1, i.s0.b(), true); }
  void emit(const setcc& i) {
    ppc64_asm::Label l_true, l_end;
    Reg64 d(i.d);

    a->bc(l_true, i.cc);
    a->xor(d, d, d, false);   /* set output to 0 */
    a->b(l_end);

    l_true.asm_label(*a);
    a->li(d, 1);        /* set output to 1 */

    l_end.asm_label(*a);
  }
  void emit(const shlli& i) {
    a->slwi(Reg64(i.d), Reg64(i.s1), i.s0.b(), true);
  }
  void emit(const shl& i) { a->sld(i.d, i.s1, i.s0, true); }
  void emit(const shlqi& i) { a->sldi(i.d, i.s1, i.s0.b(), true); }
  void emit(const shrli& i) {
    a->srwi(Reg64(i.d), Reg64(i.s1), i.s0.b(), true);
  }
  void emit(const shrqi& i) { a->srdi(i.d, i.s1, i.s0.b(), true); }
  void emit(const sqrtsd& i) { a->xssqrtdp(i.d,i.s); }
  void emit(const storeups& i) { a->stxvw4x(i.s,i.m); }

  // macro for commonlizing X-/D-form of load/store instructions
#define X(instr, dst, ptr)                                \
  do {                                                    \
    if (ptr.index.isValid()) {                            \
      a->instr##x(dst, ptr);                              \
    } else {                                              \
      a->instr   (dst, ptr);                              \
    }                                                     \
  } while(0)

  // As all registers are 64-bits wide, a smaller number from the memory should
  // have its sign extended after loading except for the 'z' vasm variants
  void emit(const loadw& i) {
    X(lhz,  Reg64(i.d), i.s);
    a->extsh(Reg64(i.d), Reg64(i.d));
  }
  void emit(const loadl& i) {
    X(lwz,  Reg64(i.d), i.s);
    a->extsw(Reg64(i.d), Reg64(i.d));
  }

  void emit(const loadb& i)   { X(lbz, Reg64(i.d),  i.s); }
  void emit(const loadtqb& i) { X(lbz, Reg64(i.d),  i.s); }
  void emit(const loadzbl& i) { X(lbz,  Reg64(i.d), i.s); }
  void emit(const loadzbq& i) { X(lbz,  i.d,        i.s); }
  void emit(const loadzlq& i) { X(lwz,  i.d,        i.s); }
  void emit(const storeb& i)  { X(stb,  Reg64(i.s), i.m); }
  void emit(const storel& i)  { X(stw,  Reg64(i.s), i.m); }
  void emit(const storew& i)  { X(sth,  Reg64(i.s), i.m); }
  void emit(const loadsd& i)  { X(lfd,  i.d,        i.s); }
  void emit(const storesd& i) { X(stfd, i.s,        i.m); }

#undef X

  /* Subtractions: d = s1 - s0 */
  void emit(const subq& i) { a->sub(i.d, i.s1, i.s0, true); }
  void emit(const subsd& i) { a->fsub(i.d, i.s1, i.s0, false); }
  void emit(const testq& i) {
    // More information on:
    // https://goo.gl/F1wrbO
    if (i.s0 != i.s1) {
      a->and(rAsm, i.s0, i.s1, true);   // result is not used, only flags
    } else {
      a->cmpdi(i.s0, Immed(0));
    }
  }
  void emit(const ucomisd& i) { a->dcmpu(i.s1,i.s0); }              // needs SF
  void emit(const ud2& i) { a->trap(); }
  void emit(const xorb& i) {
    a->xor(Reg64(i.d), Reg64(i.s0), Reg64(i.s1), true);
  }
  void emit(const xorl& i) {
    a->xor(Reg64(i.d), Reg64(i.s0), Reg64(i.s1), true);
  }
  void emit(const xorq& i) { a->xor(i.d, i.s0, i.s1, true); }
  void emit(const xorqi& i) {
    if (i.s0.fits(sz::word)) {
      a->li(rAsm, i.s0);
    } else {
      a->li32(rAsm, i.s0.l());
    }

    a->xor(i.d, i.s1, rAsm, true /** xor. implies Rc = 1 **/);
  }


  // The following vasms reemit other vasms. They are implemented afterwards in
  // order to guarantee that the desired vasm is already defined or else it'll
  // fallback to the templated emit function.
  void emit(const cvtsi2sd& i);
  void emit(const cvttsd2siq& i);
  void emit(const callfaststub& i);
  void emit(const callstub& i);
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const load& i);
  void emit(const pop& i);
  void emit(const push& i);
  void emit(const store& i);
  void emit(const mcprep&);
  void emit(const syncpoint& i);
  void emit(const unwind& i);
  void emit(const callphp&);
  void emit(const cmovq&);
  void emit(const leavetc&);
  void emit(const lea&);
  void emit(const contenter&);

  // auxiliary for emit(call) and emit(callr)
  template<class Func>
  void callExtern(Func func);
  void emit(const call& i);
  void emit(const callr& i);
  void emit(const calls& i);
  void emit(const stublogue& i);
  void emit(const stubret& i);
  void emit(const tailcallstub& i);
  void emit(const callarray& i);

private:
  CodeBlock& frozen() { return text.frozen().code; }

  Vtext& text;
  ppc64_asm::Assembler assem;
  ppc64_asm::Assembler* a;

  const Vlabel current;
  const Vlabel next;
  jit::vector<Venv::LabelPatch>& jmps;
  jit::vector<Venv::LabelPatch>& jccs;
  jit::vector<Venv::LabelPatch>& catches;
};

void Vgen::emit(const cvtsi2sd& i) {
  // As described on ISA page 727, F.2.6
  emit(copy{i.s, i.d});
  a->fcfids(i.d, i.d);
}

void Vgen::emit(const cvttsd2siq& i) {
  // As described on ISA page 726, F.2.2
  a->fctid(rFasm, i.s);
  emit(copy{rFasm, i.d});
}

void Vgen::emit(const ldimmb& i) {
  if (i.d.isGP()) {
    // This is necessary since x86_64 can load only the byte and do not
    // touch the other bits of destination register
    a->li(rAsm, i.s); // should be only 8bits available
    a->clrrwi(i.d, i.d, CHAR_BIT); // Clear lower byte
    // Place only the byte into the register, keeping the higher 56bits
    a->or(i.d, i.d, rAsm);
  } else {
    assertx(i.d.isSIMD());
    a->li(rAsm, i.s);
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const ldimml& i) {
  if (i.d.isGP()) {
    a->li32(i.d, i.s.l());
  } else {
    assertx(i.d.isSIMD());
    a->li32(rAsm, i.s.l());
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const ldimmq& i) {
  auto val = i.s.q();
  if (i.d.isGP()) {
    if (val == 0) {
      a->xor(i.d, i.d, i.d);
      // emit nops to fill a standard li64 instruction block
      // this will be useful on patching and smashable operations
      a->emitNop(ppc64_asm::Assembler::kLi64InstrLen -
          1 * ppc64_asm::Assembler::kBytesPerInstr);
    } else {
      a->li64(i.d, val);
    }
  } else {
    assertx(i.d.isSIMD());
    a->li64(rAsm, i.s.q());
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const contenter& i) {
 ppc64_asm::Label stub, end;
 Reg64 fp = i.fp;
 a->ba(end); // jmp in x64

 stub.asm_label(*a);
 // The following two lines are equivalent to popm lower.
 // Since we can't call popm lower function here at the
 // moment it seems sufficient.
 // rAsm is a scratch register.
 emit(pop{rAsm});
 emit(store{rAsm, fp[AROFF(m_savedRip)]});
 emit(jmpr{i.target, i.args});

 end.asm_label(*a);
 a->bla(stub); // call in x64

 emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const syncpoint& i) {
  // As the syncpoint intends to store the return address of the last call vasm,
  // it's necessary to subtract 3 instructions from the callExtern's "epilogue"
  // as mtlr, ld, addi exist before current a->frontier().
  TCA saved_pc = a->frontier() - 3 * ppc64_asm::Assembler::kBytesPerInstr;
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", saved_pc,
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(saved_pc, i.fix);
}

/*
 * Push/pop mechanism is as simple as X64: it stores 8 bytes below the SP.
 */
void Vgen::emit(const pop& i) {
  a->ld(i.d, rsp()[0]);                         // popped element
  a->addi(rsp(), rsp(), push_pop_position);     // recover stack
}
void Vgen::emit(const push& i) {
  a->stdu(i.s, rsp()[-push_pop_position]);      // pushed element
}

void Vgen::emit(const load& i) {
  if (i.d.isGP()) {
    if (i.s.index.isValid()){
      a->ldx(i.d, i.s);
    } else {
      a->ld(i.d, i.s);
    }
  } else {
    assertx(i.d.isSIMD());
    a->lfd(i.d, i.s);
  }
}

void Vgen::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}
void Vgen::emit(const unwind& i) {
  catches.push_back({a->frontier(), i.targets[1]});
  emit(jmp{i.targets[0]});
}
void Vgen::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});

  // offset to be determined by a->patchBctr
  a->branchAuto(a->frontier());
}
void Vgen::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});

    // offset to be determined by a->patchBctr
    a->branchAuto(a->frontier(), i.cc);
  }
  emit(jmp{i.targets[0]});
}
void Vgen::emit(const jcci& i) {
  a->branchAuto(i.taken, i.cc);
  emit(jmp{i.target});
}

void Vgen::emit(const cmovq& i) {
  // A CR bit parameter in ppc64 is a combination of X64's cc and sf variables:
  // CR group (4 bits per group) is a sf and the CR bit (1 of the 4) is the cc
  BranchParams bp (i.cc);
  auto t = i.t;
  auto f = i.f;
  if (static_cast<uint8_t>(BranchParams::BO::CRNotSet) == bp.bo()) {
    // invert the true/false parameters, as only the bp.bi field is used
    std::swap(t,f);
  }
  a->isel(i.d, t, f, (4 * int(i.sf.asReg())) + bp.bi());
}

void Vgen::patch(Venv& env) {
  for (auto& p : env.jmps) {
    assertx(env.addrs[p.target]);
    ppc64_asm::Assembler::patchBctr(p.instr, env.addrs[p.target]);
  }
  for (auto& p : env.jccs) {
    assertx(env.addrs[p.target]);
    ppc64_asm::Assembler::patchBctr(p.instr, env.addrs[p.target]);
  }
  assertx(env.bccs.empty());
}

void Vgen::pad(CodeBlock& cb) {
  ppc64_asm::Assembler a {cb};
  while (a.available() >= 4) a.trap();
  assertx(a.available() == 0);
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    if (i.d.index.isValid()){
      a->stdx(i.s, i.d);
    } else {
      a->std(i.s, i.d);
    }
  } else {
    assertx(i.s.isSIMD());
    a->stfd(i.s, i.d);
  }
}

void Vgen::emit(const mcprep& i) {
  /*
   * Initially, we set the cache to hold (addr << 1) | 1 (where `addr' is the
   * address of the movq) so that we can find the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * handlePrimeCacheInit can tell it's not been smashed yet
   */
  auto const mov_addr = emitSmashableMovq(a->code(), 0, r64(i.d));
  auto const imm = reinterpret_cast<uint64_t>(mov_addr);
  smashMovq(mov_addr, (imm << 1) | 1);

  mcg->cgFixups().m_addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

void Vgen::emit(const callphp& i) {
  emitSmashableCall(a->code(), i.stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const leavetc&) {
  emit(ret{});
}

void Vgen::emit(const lea& i) {
  // could do this in a simplify pass
  if (i.s.disp == 0 && i.s.base.isValid() && !i.s.index.isValid()) {
    emit(copy{i.s.base, i.d});
  } else {
    // don't reuse addq and addqi as they need a SF
    Vptr p = i.s;
    if (p.index.isValid()) {
      a->add(i.d, p.base, p.index);
    } else {
      a->addi(i.d, p.base, p.disp);
    }
  }
}

template<class Func>
void Vgen::callExtern(Func func) {
  /*
   * REMEMBER to update the smashable parameters for call on
   * smashable-instr-ppc64.h when this function is updated!
   */
  a->prologue(rsp(), rfuncln(), rvmfp());
  func(); // branch
  a->epilogue(rsp(), rfuncln());
}

void Vgen::emit(const call& i) {
  callExtern([&]() {
      a->branchAuto(i.target, BranchConditions::Always, LinkReg::Save);
  });
}

void Vgen::emit(const callr& i) {
  callExtern([&]() {
      a->mr(rfuncentry(), i.target);
      a->mtctr(rfuncentry());
      a->bctrl();
  });
}

void Vgen::emit(const calls& i) {
  emitSmashableCall(a->code(), i.target);
}

void Vgen::emit(const callarray& i) {
  emit(call{i.target, i.args});
}

/////////////////////////////////////////////////////////////////////////////
/*
 * Stub function ABI
 *
 * The code below is Call-stub ABI compliant, not PPC64 ABI.
 */
void Vgen::emit(const stublogue& i) {
  // Return address.
  auto tmp = rfuncln();
  a->mflr(tmp);
  a->std(tmp, rsp()[-8]);

  // rvmfp, if necessary.
  if (i.saveframe) a->std(rvmfp(), rsp()[-16]);

  // Update native stack pointer.
  a->addi(rsp(), rsp(), -16);
}

void Vgen::emit(const stubret& i) {
  // Update native stack pointer.
  a->addi(rsp(), rsp(), 16);

  // rvmfp, if necessary.
  if (i.saveframe) a->ld(rvmfp(), rsp()[-16]);

  // Return address.
  auto tmp = rfuncln();
  a->ld(tmp, rsp()[-8]);
  a->mtlr(tmp);
  emit(ret{});
}

void Vgen::emit(const tailcallstub& i) {
  // Return address.
  auto tmp = rfuncln();
  a->mflr(tmp);
  a->std(tmp, rsp()[-8]);

  // Update native stack pointer.
  a->addi(rsp(), rsp(), -16);

  emit(jmpi{i.target, i.args});
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Lower the immediate to a register in order to use ppc64 instructions that
 * can change required Condition Register (on vasm world, that's the SF
 * register).
 */
void lowerImm(Immed imm, Vout& v, Vreg& tmpRegister) {
  tmpRegister = v.makeReg();
  if (imm.fits(sz::word)) {
    v << ldimmw{imm, tmpRegister};
  } else {
    v << ldimml{imm, tmpRegister};
  }
}

/*
 * Native ppc64 instructions can't handle an immediate bigger than 16 bits and
 * need to be loaded into a register in order to be used.
 */
bool patchImm(Immed imm, Vout& v, Vreg& tmpRegister) {
  if (!imm.fits(sz::word)) {
    tmpRegister = v.makeReg();
    v << ldimml{imm, tmpRegister};
    return true;
  } else {
    return false;
  }
}

/*
 * Vptr struct supports fancy x64 addressing modes.
 * So we need to patch it to avoid ppc64el unsuported address modes.
 *
 * After patching, the Vptr @p will only have either base and index or base and
 * displacement.
 */
void patchVptr(Vptr& p, Vout& v) {
  // Map all address modes that Vptr can be so it can be handled.
  enum class AddressModes {
    Invalid        = 0,
    Disp           = 1, // Displacement
    Base           = 2, // Base
    BaseDisp       = 3, // Base+Displacement
    Index          = 4, // Index
    IndexDisp      = 5, // Index+Dispacement
    IndexBase      = 6, // Index+Base
    IndexBaseDisp  = 7  // Index+Base+Displacement
  };

  AddressModes mode = static_cast<AddressModes>(
                    (((p.disp != 0)     & 0x1) << 0) |
                    ((p.base.isValid()  & 0x1) << 1) |
                    ((p.index.isValid() & 0x1) << 2));

  // Index can never be used directly if shifting is necessary. Handling it here
  uint8_t shift = p.scale == 2 ? 1 :
                  p.scale == 4 ? 2 :
                  p.scale == 8 ? 3 : 0;

  if (p.index.isValid() && shift) {
    Vreg shifted_index_reg = v.makeReg();
    v << shlqi{shift, p.index, shifted_index_reg, VregSF(RegSF{0})};
    p.index = shifted_index_reg;
    p.scale = 1;  // scale is now normalized.
  }

  // taking care of the displacement, in case it is > 16bits
  Vreg disp_reg;
  bool patched_disp = patchImm(Immed(p.disp), v, disp_reg);
  switch (mode) {
    case AddressModes::Base:
    case AddressModes::IndexBase:
      // ppc64 can handle these address modes. Nothing to do here.
      break;

    case AddressModes::BaseDisp:
      // ppc64 can handle this address mode if displacement < 16bits
      if (patched_disp) {
        // disp is loaded on a register. Change address mode to kBase_Index
        p.index = disp_reg;
        p.disp = 0;
      }
      break;

    case AddressModes::Index:
        // treat it as kBase to avoid a kIndex_Disp asm handling.
        std::swap(p.base, p.index);
      break;

    case AddressModes::Disp:
    case AddressModes::IndexDisp:
      if (patched_disp) {
        // disp is loaded on a register. Change address mode to kBase_Index
        p.base = disp_reg;
        p.disp = 0;
      }
      else {
        // treat it as kBase_Disp to avoid a kIndex_Disp asm handling.
        p.base = p.index;
      }
      break;

    case AddressModes::IndexBaseDisp: {
      // This mode is not supported: Displacement will be embedded on Index
      Vreg index_disp_reg = v.makeReg();
      if (patched_disp) {
        v << addq{disp_reg, p.index, index_disp_reg, VregSF(RegSF{0})};
      } else {
        v << addqi{p.disp, p.index, index_disp_reg, VregSF(RegSF{0})};
      }
      p.index = index_disp_reg;
      p.disp = 0;
      break;
    }

    case AddressModes::Invalid:
    default:
      assert(false && "Invalid address mode");
      break;
  }
}


/*
 * Rules for the lowering of these vasms:
 * 1) All vasms emitted in lowering are already adjusted/patched.
 *   In other words, it will not be lowered afterwards.
 * 2) If a vasm has a Vptr that can be removed by emitting load/store, do it!
 *
 * Parameter description for every lowering:
 * Vout& v : the Vout instance so vasms can be emitted
 * <Type> inst : the current vasm to be lowered
 */

/* fallback, when a vasm is not lowered */
template <typename Inst>
void lowerForPPC64(Vout& v, Inst& inst) {}

/*
 * Using macro to commonlize vasms lowering
 */

// Patches the Vptr, retrieve the immediate and emmit a related direct vasm
#define X(vasm_src, attr_data, vasm_dst, attr_addr, vasm_imm)           \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp = v.makeReg();                                               \
  Vptr p = inst.attr_addr;                                              \
  patchVptr(p, v);                                                      \
  v << vasm_imm{inst.attr_data, tmp};                                   \
  v << vasm_dst{tmp, p};                                                \
}

X(storebi, s, storeb, m, ldimmb)
X(storewi, s, storew, m, ldimmw)
X(storeli, s, storel, m, ldimml)
X(storeqi, s, store,  m, ldimml)

#undef X

// Simply take care of the vasm's Vptr, reemmiting it if patch occured
#define X(vasm, attr_addr, attr_1, attr_2)                              \
void lowerForPPC64(Vout& v, vasm& inst) {                               \
  patchVptr(inst.attr_addr, v);                                         \
  if (!v.empty()) v << vasm{inst.attr_1, inst.attr_2};                  \
}

X(storeb,   m, s, m);
X(storew,   m, s, m);
X(storel,   m, s, m);
X(store,    d, s, d);
X(storeups, m, s, m);
X(storesd,  m, s, m);
X(load,     s, s, d);
X(loadb,    s, s, d);
X(loadl,    s, s, d);
X(loadzbl,  s, s, d);
X(loadzbq,  s, s, d);
X(loadzlq,  s, s, d);
X(loadups,  s, s, d);
X(lea,      s, s, d);

#undef X

// Auxiliary macros to handle vasms with different attributes
#define NONE
#define ONE(attr_1)             inst.attr_1,
#define TWO(attr_1, attr_2)     inst.attr_1, inst.attr_2,
#define ONE_R64(attr_1)         Reg64(inst.attr_1),
#define TWO_R64(attr_1, attr_2) Reg64(inst.attr_1), Reg64(inst.attr_2),

// If it patches the Immed, replace the vasm for its non-immediate variant
#define X(vasm_src, vasm_dst, attr_imm, attrs)                          \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp;                                                             \
  lowerImm(inst.attr_imm, v, tmp);                                      \
  v << vasm_dst{tmp, attrs inst.sf};                                    \
}

X(addli,  addl,  s0, TWO(s1, d))
X(andli,  andl,  s0, TWO(s1, d))    // could use patchImm
X(andqi,  andq,  s0, TWO(s1, d))    // could use patchImm
X(testqi, testq, s0, ONE(s1))
X(cmpqi,  cmpq,  s0, ONE(s1))

#undef X

// Simplify MemoryRef vasm types by their direct variant as ppc64 can't
// change data directly in memory. Patches the Vptr, grab and save the data.
#define X(vasm_src, vasm_dst, vasm_load, vasm_store, attr_addr, attrs)  \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp = v.makeReg(), tmp2 = v.makeReg();                           \
  Vptr p = inst.attr_addr;                                              \
  patchVptr(p, v);                                                      \
  v << vasm_load{p, tmp};                                               \
  v << vasm_dst{attrs tmp, tmp2, inst.sf};                              \
  v << vasm_store{tmp2, p};                                             \
}

X(incwm, incw, loadw, storew, m, NONE)
X(inclm, incl, loadl, storel, m, NONE)
X(incqm, incq, load,  store,  m, NONE)
X(declm, decl, loadl, storel, m, NONE)
X(decqm, decq, load,  store,  m, NONE)
X(addlm, addl, loadw, storew, m, ONE(s0))

#undef X

#undef NONE
#undef ONE
#undef TWO
#undef ONE_R64
#undef TWO_R64

// Also deals with MemoryRef vasms like above but these ones have Immed data
// too. Load data and emit a new vasm depending if the Immed fits a direct
// ppc64 instruction.
#define X(vasm_src, vasm_dst_reg, vasm_dst_imm, vasm_load,              \
                  attr_addr, attr_data)                                 \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vptr p = inst.attr_addr;                                              \
  patchVptr(p, v);                                                      \
  Vreg tmp2 = v.makeReg(), tmp;                                         \
  v << vasm_load{p, tmp2};                                              \
  if (patchImm(inst.attr_data, v, tmp))                                 \
    v << vasm_dst_reg{tmp, tmp2, inst.sf};                              \
  else v << vasm_dst_imm{inst.attr_data, tmp2, inst.sf};                \
}

X(cmpbim,  cmpl,  cmpli,  loadb, s1, s0)
X(cmplim,  cmpl,  cmpli,  loadl, s1, s0)
X(cmpqim,  cmpq,  cmpqi,  load,  s1, s0)
X(testbim, testq, testqi, loadb, s1, s0)
X(testwim, testq, testqi, loadw, s1, s0)
X(testlim, testq, testqi, loadl, s1, s0)
X(testqim, testq, testqi, load,  s1, s0)

#undef X

// Very similar with the above case: handles MemoryRef and Immed, but also
// stores the result in the memory.
#define X(vasm_src, vasm_dst, vasm_load, vasm_store,                    \
                  attr_addr, attr_data)                                 \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp = v.makeReg(), tmp3 = v.makeReg(), tmp2;                     \
  Vptr p = inst.attr_addr;                                              \
  patchVptr(p, v);                                                      \
  v << vasm_load{p, tmp};                                               \
  lowerImm(inst.attr_data, v, tmp2);                                    \
  v << vasm_dst{tmp2, tmp, tmp3, inst.sf};                              \
  v << vasm_store{tmp3, p};                                             \
}

X(orwim,   orq,  loadw, storew, m, s0)
X(orqim,   orq,  load,  store,  m, s0)
X(addqim,  addq, load,  store,  m, s0)

#undef X

// Handles MemoryRef arguments and load the data input from memory, but these
// ones have no output other than the sign flag register update (SF)
#define X(vasm_src, vasm_dst, vasm_load, attr_addr, attr)               \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vptr p = inst.attr_addr;                                              \
  patchVptr(p, v);                                                      \
  Vreg tmp = v.makeReg();                                               \
  v << vasm_load{p, tmp};                                               \
  v << vasm_dst{inst.attr, tmp, inst.sf};                               \
}

X(testqm, testq, load,  s1, s0)
X(cmplm,  cmpl,  loadl, s1, s0)
X(cmpqm,  cmpq,  load,  s1, s0)

#undef X

// Other lowers that didn't fit the macros above or are not so numerous.
void lowerForPPC64(Vout& v, jmpm& inst) {
  Vptr p = inst.target;
  patchVptr(p, v);
  Vreg tmp = v.makeReg();
  v << load{p, tmp};
  v << jmpr{tmp, inst.args};
}

void lowerForPPC64(Vout& v, callm& inst) {
  Vptr p = inst.target;
  patchVptr(p, v);
  auto d = v.makeReg();
  v << load{p, d};
  v << callr{d, inst.args};
}

void lowerForPPC64(Vout& v, loadqp& inst) {
  // in PPC we don't have anything like a RIP register
  // RIP register uses an absolute displacement address. Load it to a register
  Vptr p(v.cns(inst.s.r.disp), 0); // this Vptr doesn't need patch
  v << load{p, inst.d};
}

void lowerForPPC64(Vout& v, popm& inst) {
  auto tmp = v.makeReg();
  patchVptr(inst.d, v);
  v << pop{tmp};
  v << store{tmp, inst.d};
}

void lowerForPPC64(Vout& v, countbytecode& inst) {
  v << incqm{inst.base[g_bytecodesVasm.handle()], inst.sf};
}

/////////////////////////////////////////////////////////////////////////////
/*
 * PHP function ABI
 */
void lowerForPPC64(Vout& v, phplogue& inst) {
  Vreg ret_address = v.makeReg();
  v << mflr{ret_address};
  v << store{ret_address, inst.fp[AROFF(m_savedRip)]};
}

void lowerForPPC64(Vout& v, phpret& inst) {
  Vreg tmp = v.makeReg();
  v << load{inst.fp[AROFF(m_savedRip)], tmp};
  if (!inst.noframe) {
    v << load{inst.fp[AROFF(m_sfp)], inst.d};
  }
  v << jmpr{tmp};
}

/*
 * Tail call elimination on ppc64: call without creating a stack and keep LR
 * contents as prior to the call.
 */
void lowerForPPC64(Vout& v, tailcallphp& inst) {
  Vreg new_return = v.makeReg();
  v << load{inst.fp[AROFF(m_savedRip)], new_return};
  v << mtlr{new_return};
  v << jmpr{inst.target, inst.args};
}

/////////////////////////////////////////////////////////////////////////////

// Lower movs to copy
void lowerForPPC64(Vout& v, movtqb& inst) { v << copy{inst.s, inst.d}; }
void lowerForPPC64(Vout& v, movtql& inst) { v << copy{inst.s, inst.d}; }

// For PPC64 there are no instructions to perform operations with only part of
// a register, differently of x86_64.
// The VASMs that are composed of these kind of operations, must be lowered to
// first extract the operator into a register and later perform operation using
// the entire register, i. e., 64bits. If it needs a result, like subtract
// operations, the destination register must be filled considering the size of
// operation. For example:
// Consider d register with the value "0x44582C24A50CAD2".
// For a subl instruction (l means 32bits), the result must be placed in the
// lower 32bits (X means the result value):
// "0x44582C2XXXXXXXX" and the higher 32bits should not be touched.
// For operands the treatment is analogous, only the lower 32bits are
// considered to perform the operation.
// Lower comparison to cmpq
void lowerForPPC64(Vout& v, cmpb& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg(),
      tmp4 = v.makeReg();

  v << movb{inst.s0, tmp1}; // Extract s0
  v << extsb {tmp1, tmp2, VregSF(RegSF{0})}; // Extend byte sign
  v << movb{inst.s1, tmp3}; // Extract s1
  v << extsb {tmp3, tmp4, VregSF(RegSF{0})}; // Extend byte sign
  v << cmpq{tmp2, tmp4, inst.sf}; // Compare values
}

void lowerForPPC64(Vout& v, cmpl& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg(),
      tmp4 = v.makeReg();

  v << movl{inst.s0, tmp1}; // Extract byte from s0
  v << extsw{tmp1, tmp2, VregSF(RegSF{0})}; // Extend word sign
  v << movl{inst.s1, tmp3}; // extract byte from s1
  v << extsw{tmp3, tmp4, VregSF(RegSF{0})}; // Extend word sign
  v << cmpq{tmp3, tmp4, inst.sf}; // Compare the extracted values
}

// Lower comparison with immediate to cmpqi
void lowerForPPC64(Vout& v, cmpbi& inst) {
  Vreg tmp1 = v.makeReg(), tmp2 = v.makeReg();

  v << movb{inst.s1, tmp1}; // extract s0
  v << extsb {tmp1, tmp2, VregSF(RegSF{0})}; // extend byte sign
  // compare immed only with the byte extracted
  v << cmpqi{inst.s0, tmp2, inst.sf};
}

void lowerForPPC64(Vout& v, cmpli& inst) {
  // convert cmpli to cmpqi or ldimmq + cmpq by cmpqi's lowering
  Vreg tmp1 = v.makeReg(), tmp2 = v.makeReg(), tmp3 = v.makeReg();

  v << movl{inst.s1, tmp1}; // Extract s1
  v << extsw{tmp1, tmp2, VregSF(RegSF{0})}; // Extend word sign
  // Lowering for cmpqi removed, since movl was emitted, then the empty
  // will always return false
  if (patchImm(inst.s0, v, tmp3)) v << cmpq{tmp3, tmp2, inst.sf};
  else v << cmpqi{inst.s0, tmp2, inst.sf};
}

// Lower subtraction to subq
void lowerForPPC64(Vout& v, subl& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg(),
        tmp3 = v.makeReg(),
        tmp4 = v.makeReg(),
        tmp5 = v.makeReg();

  v << movl{inst.s0, tmp1}; // Extract s0
  v << movl{inst.s1, tmp2}; // Extract s1
  v << extsw{tmp1, tmp3, VregSF(RegSF{0})}; // Extend word sign
  v << extsw{tmp2, tmp4, VregSF(RegSF{0})}; // Extend word sign
  v << subq{tmp3, tmp4, tmp5, inst.sf}; // subtract
  // Move word to d and do not touch the higher 32bits
  v << movlk{tmp5, Reg64(inst.d)};
}

void lowerForPPC64(Vout& v, subbi& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg(),
        tmp3 = v.makeReg();

  v << movb{inst.s1, tmp1}; // Extract s1
  v << extsb{tmp1, tmp2, VregSF(RegSF{0})}; // Extend byte sign
  v << subqi{inst.s0, tmp2, tmp3, inst.sf}; // subtract immediate
  // move byte to destiny and do not touch higher 56bits
  v << movb{tmp3, inst.d};
}

void lowerForPPC64(Vout& v, subli& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg(),
        tmp3 = v.makeReg();

  v << movl{inst.s1, tmp1}; // Extract s1
  v << extsw{tmp1, tmp2, VregSF(RegSF{0})}; // Extend word sign
  v << subqi{inst.s0, tmp2, tmp3, inst.sf}; // Subtract immediate
  // move word to destiny and do not touch the higher 32bits
  v << movlk{tmp3, Reg64(inst.d)};
}

// Lower test to testq
void lowerForPPC64(Vout& v, testb& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg();

  v << movb{inst.s0, tmp1};// Extract s0
  v << movb{inst.s1, tmp2};// Extract s1
  v << testq{tmp1, tmp2, inst.sf};// Compare lower byte of each source
}

void lowerForPPC64(Vout& v, testl& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg();

  v << movl{inst.s0, tmp1}; // Extract s0
  v << movl{inst.s1, tmp2}; // Extract s1
  v << testq{tmp1, tmp2, inst.sf}; // Compare low 32bits of each source
}

void lowerForPPC64(Vout& v, testbi& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg();

  v << movb{inst.s1, tmp1}; // Extract byte
  // convert testbi to testqi or ldimmq + testq by testqi's lowering
  if (patchImm(inst.s0, v, tmp2)) v << testq{tmp2, tmp1, inst.sf};
  else v << testqi{inst.s0, tmp1, inst.sf}; // perform test operation
}

void lowerForPPC64(Vout& v, testli& inst) {
  Vreg tmp1 = v.makeReg(),
        tmp2 = v.makeReg();

  v << movl{inst.s1, tmp1}; // Extract word
  // convert testli to testqi or ldimmq + testq by testqi's lowering
  if (patchImm(inst.s0, v, tmp2)) v << testq{tmp2, tmp1, inst.sf};
  else v << testqi{inst.s0, tmp1, inst.sf};
}

// Lower xor to xorq
void lowerForPPC64(Vout& v, xorb& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg();

  v << movb{inst.s0, tmp1}; // Extract byte
  v << movb{inst.s1, tmp2}; // Extract byte
  v << xorq{tmp1, tmp2, tmp3, inst.sf}; // Perform xorq
  // Place only byte result into d and do not touch higher 56bits
  v << movb{tmp3, inst.d};
}
void lowerForPPC64(Vout& v, xorl& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg();

  v << movl{inst.s0, tmp1}; // Extract word
  v << movl{inst.s1, tmp2}; // Extract word
  v << xorq{tmp1, tmp2, tmp3, inst.sf}; // Perform xorq
  // Place only byte result into d and do not touch higher 32bits
  v << movlk{tmp3, Reg64(inst.d)};
}

// Lower xor with immediate to xorqi
void lowerForPPC64(Vout& v, xorbi& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg();

  v << movb {inst.s1, tmp1}; // Extract byte
  v << xorqi {inst.s0, tmp1, tmp2, VregSF(RegSF{0})}; // Perform xorqi
  // move only the byte result to the destination, keeping the higher 56bits
  v << movb {inst.d, tmp1};
}

// Lower and to andq
void lowerForPPC64(Vout& v, andb& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg();

  v << movb{inst.s0, tmp1}; // Extract s0
  v << movb{inst.s1, tmp2}; // Extract s1
  v << andq{tmp1, tmp2, tmp3, inst.sf};
  // move only the byte result to the destination, keeping the higher 56bits
  v << movb{tmp3, inst.d};
}

void lowerForPPC64(Vout& v, andl& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg(),
      tmp3 = v.makeReg();

  v << movl{inst.s0, tmp1}; // extract s0
  v << movl{inst.s1, tmp2}; // extract s1
  v << andq{tmp1, tmp2, tmp3, inst.sf};
  v << movlk{tmp3, Reg64(inst.d)}; // Move result keeping the higher 32bits
}

void lowerForPPC64(Vout& v, andbi& inst) {
  Vreg tmp1 = v.makeReg(),
      tmp2 = v.makeReg();

  v << movb{inst.s1, tmp1}; // Extract s1
  // patchImm doesn't need to be called as it should be < 8 bits
  v << andqi{inst.s0, tmp1, tmp2, inst.sf};
  // move only the byte result to the destination, keeping the higher 56bits
  v << movb{tmp2, inst.d};
}

void lowerForPPC64(Vout& v, cloadq& inst) {
  auto m = inst.t;
  patchVptr(m, v);
  auto tmp = v.makeReg();
  v << load{m, tmp};
  v << cmovq{inst.cc, inst.sf, inst.f, tmp, inst.d};
}

void lowerForPPC64(Vout& v, cmpsd& inst) {
  auto sf = v.makeReg();
  auto r64_d = v.makeReg(); // auxiliary for GP -> FP conversion

  // assume standard ComparisonPred as eq_ord
  Vreg equal = rone();
  Vreg nequal = v.makeReg();
  v << ldimmb{0, nequal};

  switch (inst.pred) {
  case ComparisonPred::eq_ord: { // scope for the zero variable initialization
    v << fcmpo{inst.s0, inst.s1, sf};
    // now set the inst.d if the CR's EQ bit is set, else clear it
    break;
  }
  case ComparisonPred::ne_unord:
    v << fcmpu{inst.s0, inst.s1, sf};
    // now clear the inst.d if the CR's EQ bit is set, else set it
    std::swap(equal, nequal);
    break;
  default:
    assert(false && "Invalid ComparisonPred for cmpsd");
  }
  v << cmovq{CC_E, sf, nequal, equal, r64_d};
  v << copy{r64_d, inst.d}; // GP -> FP
}

void lower_vcallarray(Vunit& unit, Vlabel b) {
  auto& code = unit.blocks[b].code;
  // vcallarray can only appear at the end of a block.
  auto const inst = code.back().get<vcallarray>();
  auto const origin = code.back().origin;

  auto argRegs = inst.args;
  auto const& srcs = unit.tuples[inst.extraArgs];
  jit::vector<Vreg> dsts;
  for (int i = 0; i < srcs.size(); ++i) {
    dsts.emplace_back(rarg(i));
    argRegs |= rarg(i);
  }

  code.back() = copyargs{unit.makeTuple(srcs), unit.makeTuple(std::move(dsts))};
  code.emplace_back(callarray{inst.target, argRegs});
  code.back().origin = origin;
  code.emplace_back(unwind{{inst.targets[0], inst.targets[1]}});
  code.back().origin = origin;
}


/*
 * Lower a few abstractions to facilitate straightforward PPC64 codegen.
 * PPC64 doesn't have instructions for operating on less than 64 bits data
 * (except the memory related load/store), therefore all arithmetic vasms
 * that intend to deal with smaller data will actually operate on 64bits
 */
void lowerForPPC64(Vunit& unit) {
  Timer _t(Timer::vasm_lower);

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

      auto scratch = unit.makeScratchBlock();
      auto& inst = blocks[ib].code[ii];
      SCOPE_EXIT {unit.freeScratchBlock(scratch);};
      Vout v(unit, scratch, inst.origin);

      switch (inst.op) {

        /*
         * Call every lowering and provide only what is necessary:
         * Vout& v : the Vout instance so vasms can be emitted
         * <Type> inst : the current vasm to be lowered
         *
         * If any vasm is emitted inside of the lower, then the current vasm
         * will be replaced by the vector_splice call below.
         */

#define O(name, imms, uses, defs)                         \
        case Vinstr::name:                                \
          lowerForPPC64(v, inst.name##_);                 \
          if (!v.empty()) {                               \
            vector_splice(unit.blocks[ib].code, ii, 1,    \
                          unit.blocks[scratch].code);     \
          }                                               \
          break;

          VASM_OPCODES

#undef O

      }
    }
  });

  printUnit(kVasmLowerLevel, "after lower for PPC64", unit);
}

///////////////////////////////////////////////////////////////////////////////
} // anonymous namespace

void finishPPC64(Vunit& unit, Vtext& text, const Abi& abi, AsmInfo* asmInfo) {
  // The Timer instances calculate the time while they are alive, hence the
  // additional scope in order to limit them.
  {
    Timer timer(Timer::vasm_optimize);

    removeTrivialNops(unit);
    optimizePhis(unit);
    fuseBranches(unit);
    optimizeJmps(unit);
    optimizeExits(unit);

    lowerForPPC64(unit);

    simplify(unit);

#if 0 // TODO(gut): not needed?
    if (!unit.constToReg.empty()) {
      foldImms<x64::ImmFolder>(unit);
    }
#endif
    {
      Timer timer(Timer::vasm_copy);
      optimizeCopies(unit, abi);
    }
    if (unit.needsRegAlloc()) {
      Timer timer(Timer::vasm_xls);
      removeDeadCode(unit);
      allocateRegisters(unit, abi);
    }
    if (unit.blocks.size() > 1) {
      Timer timer(Timer::vasm_jumps);
      optimizeJmps(unit);
    }
  }

  {
    Timer timer(Timer::vasm_gen);
    vasm_emit<Vgen>(unit, text, asmInfo);
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
