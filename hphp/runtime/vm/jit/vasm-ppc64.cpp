/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

#include "hphp/util/arch.h"

#include <algorithm>
#include <tuple>

// TODO(lbianc): This is a temporary solution for compiling in different
// archs. The correct way of doing that is do not compile arch specific
// files of a different one, but currently it is not possible, since
// some common files use arch specific methods without eclusion.
// The macro bellow returns the dummy field for x86 as it will not be used
// in x64 arch.
#ifdef __powerpc64__
#define SAVED_TOC() m_savedToc
#else
#define SAVED_TOC() _dummyA
#endif

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

using namespace ppc64;
using namespace ppc64_asm;
using Assembler = ppc64_asm::Assembler;

namespace {

struct Vgen {
  explicit Vgen(Venv& env)
    : env(env)
    , text(env.text)
    , a(*env.cb)
    , current(env.current)
    , next(env.next)
    , jmps(env.jmps)
    , jccs(env.jccs)
    , catches(env.catches)
  {}

  static void patch(Venv& env) {
    for (auto& p : env.jmps) {
      assertx(env.addrs[p.target]);
      Assembler::patchBranch(p.instr, env.addrs[p.target]);
    }
    for (auto& p : env.jccs) {
      assertx(env.addrs[p.target]);
      Assembler::patchBranch(p.instr, env.addrs[p.target]);
    }
  }

  static void pad(CodeBlock& cb) {
    Assembler a {cb};
    while (a.available() >= 4) a.trap();
    assertx(a.available() == 0);
  }

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(const Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }
  void copyCR0toCR1(Assembler a, Reg64 raux)
  {
    a.mfcr(raux);
    a.sradi(raux,raux,4);
    a.mtocrf(0x40,raux);
  }

  // intrinsics
  void emit(const copy& i) {
    if (i.s == i.d) return;
    if (i.s.isGP()) {
      if (i.d.isGP()) {                     // GP => GP
        a.mr(i.d, i.s);
      } else {                              // GP => XMM
        assertx(i.d.isSIMD());
        a.std(i.s, rsp()[-8]);
        a.lfd(i.d, rsp()[-8]);
      }
    } else {
      assertx(i.s.isSIMD());
      if (i.d.isGP()) {                     // XMM => GP
        a.stfd(i.s, rsp()[-8]);
        a.ld(i.d, rsp()[-8]);
      } else {                              // XMM => XMM
        assertx(i.d.isSIMD());
        a.fmr(i.d, i.s);
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
        a.mr(rAsm, s1);
        a.mr(d0, s0);
        a.mr(d1, rAsm);
      } else {
        // could do this in a simplify pass
        if (s1 != d1) a.mr(d1, s1); // save s1 first; d1 != s0
        if (s0 != d0) a.mr(d0, s0);
      }
    } else {
      // could do this in a simplify pass
      if (s0 != d0) a.mr(d0, s0);
      if (s1 != d1) a.mr(d1, s1);
    }
  }
  void emit(const addl& i) {
    a.addo(Reg64(i.d), Reg64(i.s1), Reg64(i.s0), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const ldimmqs& i) {
    emitSmashableMovq(a.code(), env.meta, i.s.q(), i.d);
  }
  void emit(const nothrow& i) {
    // skip the "ld 2,24(1)" or "nop" emitted by "Assembler::call" at the end
    TCA saved_pc = a.frontier() - call_skip_bytes_for_ret;
    env.meta.catches.emplace_back(saved_pc, nullptr);
  }

  // instructions
  void emit(const addq& i) {
    a.addo(i.d, i.s0, i.s1, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const addsd& i) { a.fadd(i.d, i.s0, i.s1); }
  void emit(const andq& i) {
    a.and(i.d, i.s0, i.s1, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const andqi& i) {
    a.andi(i.d, i.s1, i.s0);
    copyCR0toCR1(a, rAsm);
  } // andi changes CR0
  void emit(const cmpl& i) {
    a.cmpw(Reg64(i.s1), Reg64(i.s0));
    a.cmplw(Reg64(i.s1), Reg64(i.s0), Assembler::CR::CR1);
  }
  void emit(const cmpli& i) {
    a.cmpwi(Reg64(i.s1), i.s0);
    a.cmplwi(Reg64(i.s1), i.s0, Assembler::CR::CR1);
  }
  void emit(const cmpq& i) {
    a.cmpd(i.s1, i.s0);
    a.cmpld(i.s1, i.s0, Assembler::CR::CR1);
  }
  void emit(const cmpqi& i) {
    a.cmpdi(i.s1, i.s0);
    a.cmpldi(i.s1, i.s0, Assembler::CR::CR1);
  }
  void emit(const decl& i) {
    a.subfo(Reg64(i.d), rone(), Reg64(i.s), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const decq& i) {
    a.subfo(i.d, rone(), i.s, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const divint& i) { a.divd(i.d,  i.s0, i.s1, false); }
  void emit(const divsd& i) { a.fdiv(i.d, i.s1, i.s0); }
  void emit(const extrb& i ) {
    int8_t sh = CHAR_BIT;
    a.rlwinm(Reg64(i.d), Reg64(i.s), 0, 32-sh, 31); // extract lower byte
    a.extsb(Reg64(i.d), Reg64(i.d));                // extend sign
  }
  void emit(const extsb& i) { a.extsb(i.d, i.s); }
  void emit(const extsw& i) { a.extsw(i.d, i.s); }
  void emit(const fabs& i) { a.fabs(i.d, i.s, false); }
  void emit(const fallthru& i) {}
  void emit(const fcmpo& i) {
    a.fcmpo(i.sf, i.s0, i.s1);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const fcmpu& i) {
    a.fcmpu(i.sf, i.s0, i.s1);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const imul& i) {
    a.mulldo(i.d, i.s1, i.s0, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const incl& i) {
    a.addo(Reg64(i.d), Reg64(i.s), rone(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const incq& i) {
    a.addo(i.d, i.s, rone(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const incw& i) {
    a.addo(Reg64(i.d), Reg64(i.s), rone(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const jmpi& i) { a.branchAuto(i.target); }
  void emit(const landingpad&) { }
  void emit(const ldimmw& i) { a.li(Reg64(i.d), i.s); }
  void emit(const ldarx& i) { a.ldarx(i.d, i.s); }
  void emit(const leap& i) { a.li64(i.d, i.s.r.disp, false); }
  void emit(const lead& i) { a.li64(i.d, (int64_t)i.s.get(), false); }
  void emit(const mfcr& i) { a.mfcr(i.d); }
  void emit(const mflr& i) { a.mflr(i.d); }
  void emit(const mfvsrd& i) { a.mfvsrd(i.d, i.s); }
  void emit(const mtlr& i) { a.mtlr(i.s); }
  void emit(const mtvsrd& i) { a.mtvsrd(i.d, i.s); }
  void emit(const mulsd& i) { a.fmul(i.d, i.s1, i.s0); }
  void emit(const neg& i) {
    a.neg(i.d, i.s, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const nop& i) { a.nop(); } // no-op form
  void emit(const not& i) { a.nor(i.d, i.s, i.s, false); }
  void emit(const orq& i) {
    a.or(i.d, i.s0, i.s1, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const ret& i) { a.blr(); }
  void emit(const roundsd& i) { a.xsrdpi(i.d, i.s); }
  void emit(const sar& i) {
    a.srad(i.d, i.s1, i.s0, true);
    copyCR0toCR1(a,rAsm);
  }
  void emit(const sarqi& i) {
    a.sradi(i.d, i.s1, i.s0.b(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const shl& i) {
    a.sld(i.d, i.s1, i.s0, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const shlli& i) {
    a.slwi(Reg64(i.d), Reg64(i.s1), i.s0.b(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const shlqi& i) {
    a.sldi(i.d, i.s1, i.s0.b(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const shrli& i) {
    a.srwi(Reg64(i.d), Reg64(i.s1), i.s0.b(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const shrqi& i) {
    a.srdi(i.d, i.s1, i.s0.b(), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const sqrtsd& i) { a.xssqrtdp(i.d,i.s); }

  void emit(const stdcx& i) { a.stdcx(i.s, i.d); }

  void emit(const storeups& i) { a.stxvw4x(i.s,i.m); }
  // Subtractions: d = s1 - s0
  void emit(const subq& i) {
    a.subfo(i.d, i.s0, i.s1, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const subsd& i) { a.fsub(i.d, i.s1, i.s0, false); }
  void emit(const ud2& i) { a.trap(); }
  void emit(const xorb& i) {
    a.xor(Reg64(i.d), Reg64(i.s0), Reg64(i.s1), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const xorl& i) {
    a.xor(Reg64(i.d), Reg64(i.s0), Reg64(i.s1), true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const xorq& i) {
    a.xor(i.d, i.s0, i.s1, true);
    copyCR0toCR1(a, rAsm);
  }
  void emit(const xscvdpsxds& i) { a.xscvdpsxds(i.d, i.s); }
  void emit(const xscvsxddp& i) { a.xscvsxddp(i.d, i.s); }
  void emit(const xxlxor& i) { a.xxlxor(i.d, i.s1, i.s0); }
  void emit(const xxpermdi& i) { a.xxpermdi(i.d, i.s1, i.s0); }

  void emit(const fctidz& i) {
    a.mtfsb0(23); // clear VXCVI
    a.fctidz(i.d, i.s, false);
    a.mcrfs(0,5);
  }
  // TODO: this vasm must be lowered.
  void emit(const loadups& i) {
    Vptr p = i.s;
    Vptr tmp(p);
    switch(p.scale){
      case 8:
        tmp.index = rfuncln();
        a.sldi(tmp.index, p.index, 3);
        break;
      case 4:
        tmp.index = rfuncln();
        a.sldi(tmp.index, p.index, 2);
        break;
      case 2:
        tmp.index = rfuncln();
        a.sldi(tmp.index, p.index, 1);
        break;
      default:
        break;
    }
    if(p.disp != 0) {
      tmp.base = rAsm;
      a.li64(tmp.base, static_cast<int64_t>(p.disp), false);
      a.add(tmp.base, p.base, tmp.base);
      a.lxvd2x(i.d, tmp);
    } else {
      a.lxvd2x(i.d, p);
    }
  }
  void emit(const movl& i) {
    int8_t sh = sizeof(int) * CHAR_BIT;
    a.rlwinm(Reg64(i.d), Reg64(i.s), 0, 32-sh, 31); // extract lowest 32 bits
    a.extsw(Reg64(i.d), Reg64(i.d));                // extend sign
  }
  void emit(const movw& i) {
    int8_t sh = sizeof(int) * (CHAR_BIT * 2);       // 16 bits
    a.rlwinm(Reg64(i.d), Reg64(i.s), 0, 32-sh, 31); // extract lowest 16 bits
    a.extsh(Reg64(i.d), Reg64(i.d));                // extend sign
  }
  void emit(const movb& i) {
    int8_t sh = CHAR_BIT;
    a.rlwinm(Reg64(i.d), Reg64(i.s), 0, 32-sh, 31); // extract lower byte
  }
  void emit(const orqi& i) {
    a.li64(rAsm, i.s0.l(), false);
    a.or(i.d, i.s1, rAsm, true /** or. implies Rc = 1 **/);
    copyCR0toCR1(a, rAsm);
  }
  // macro for commonlizing X-/D-form of load/store instructions
#define X(instr, dst, ptr)                                \
  do {                                                    \
    if (ptr.index.isValid()) {                            \
      a.instr##x(dst, ptr);                               \
    } else {                                              \
      a.instr   (dst, ptr);                               \
    }                                                     \
  } while(0)

  // As all registers are 64-bits wide, a smaller number from the memory should
  // have its sign extended after loading except for the 'z' vasm variants
  void emit(const loadw& i) {
    X(lhz,  Reg64(i.d), i.s);
    a.extsh(Reg64(i.d), Reg64(i.d));
  }
  void emit(const loadl& i) {
    X(lwz,  Reg64(i.d), i.s);
    a.extsw(Reg64(i.d), Reg64(i.d));
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

  void emit(const testq& i) {
    // More information on:
    // https://goo.gl/F1wrbO
    if (i.s0 != i.s1) {
      a.and(rAsm, i.s0, i.s1, true);   // result is not used, only flags
      copyCR0toCR1(a, rAsm);
    } else {
      a.cmpdi(i.s0, Immed(0));
      a.cmpldi(i.s0, Immed(0));
    }
  }
  void emit(const xorqi& i) {
    a.li64(rAsm, i.s0.l(), false);
    a.xor(i.d, i.s1, rAsm, true /** xor. implies Rc = 1 **/);
    copyCR0toCR1(a, rAsm);
  }

  void emit(const conjure& i) { always_assert(false); }
  void emit(const conjureuse& i) { always_assert(false); }

  // The following vasms reemit other vasms. They are implemented afterwards in
  // order to guarantee that the desired vasm is already defined or else it'll
  // fallback to the templated emit function.
  void emit(const call& i);
  void emit(const callarray& i);
  void emit(const callfaststub& i);
  void emit(const callphp&);
  void emit(const callr& i);
  void emit(const calls& i);
  void emit(const callstub& i);
  void emit(const calltc& i);
  void emit(const cmovq&);
  void emit(const contenter&);
  void emit(const cvtsi2sd& i);
  void emit(const decqmlock& i);
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const jmpr& i);
  void emit(const inittc&);
  void emit(const ldimmb& i);
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const lea&);
  void emit(const leavetc&);
  void emit(const load& i);
  void emit(const loadqd& i);
  void emit(const loadstubret& i);
  void emit(const mcprep&);
  void emit(const pop& i);
  void emit(const push& i);
  void emit(const resumetc& i);
  void emit(const store& i);
  void emit(const stublogue& i);
  void emit(const stubret& i);
  void emit(const stubtophp& i);
  void emit(const stubunwind& i);
  void emit(const syncpoint& i);
  void emit(const tailcallstub& i);
  void emit(const testqi& i);
  void emit(const ucomisd& i);
  void emit(const unwind& i);

  void emit_nop() {
    a.addi(rAsm, rAsm, 16);
    a.addi(rAsm, rAsm, -16);
  }

private:
  CodeBlock& frozen() { return text.frozen().code; }

  Venv& env;
  Vtext& text;
  Assembler a;

  const Vlabel current;
  const Vlabel next;
  jit::vector<Venv::LabelPatch>& jmps;
  jit::vector<Venv::LabelPatch>& jccs;
  jit::vector<Venv::LabelPatch>& catches;
};

void Vgen::emit(const cvtsi2sd& i) {
  // As described on ISA page 727, F.2.6
  emit(copy{i.s, i.d});
  a.fcfid(i.d, i.d);
}

void Vgen::emit(const decqmlock& i) {
   ppc64_asm::Label loop;
   loop.asm_label(a);
   {
     // Using rfuncln because rAsm scratch register
     // will be used by decq to save CR0 to CR1
     emit(ldarx{i.m, rfuncln()});
     emit(decq{rfuncln(), rfuncln(), i.sf});
     emit(stdcx{rfuncln(), i.m});
   }
   //TODO(racardoso): bne- (missing branch hint)
   a.bc(loop, BranchConditions::CR0_NotEqual);
}

void Vgen::emit(const ucomisd& i) {
  ppc64_asm::Label notNAN;
  emit(fcmpu{i.s0, i.s1, i.sf});
  a.bc(notNAN, BranchConditions::CR0_NoOverflow);
  {
    // Set "negative" bit if "Overflow" bit is set. Also, keep overflow bit set
    a.li64(rAsm, 0x99000000, false);
    a.mtcrf(0xC0, rAsm);
    copyCR0toCR1(a, rAsm);
  }
  notNAN.asm_label(a);
}

void Vgen::emit(const ldimmb& i) {
  if (i.d.isGP()) {
    a.li(i.d, i.s); // should be only 8bits available
  } else {
    assertx(i.d.isSIMD());
    a.li(rAsm, i.s);
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const ldimml& i) {
  if (i.d.isGP()) {
    a.li32(i.d, i.s.l());
  } else {
    assertx(i.d.isSIMD());
    a.li32(rAsm, i.s.l());
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const ldimmq& i) {
  auto val = i.s.q();
  if (i.d.isGP()) {
    if (val == 0) {
      a.xor(i.d, i.d, i.d);
      // emit nops to fill a standard li64 instruction block
      // this will be useful on patching and smashable operations
      a.emitNop(Assembler::kLi64InstrLen - 1 * instr_size_in_bytes);
    } else {
      a.li64(i.d, val);
    }
  } else {
    assertx(i.d.isSIMD());
    a.li64(rAsm, i.s.q());
    // no conversion necessary. The i.s already comes converted to FP
    emit(copy{rAsm, i.d});
  }
}

void Vgen::emit(const contenter& i) {
  ppc64_asm::Label stub, end;
  Reg64 fp = i.fp;

  a.b(end);
  stub.asm_label(a);
  {
    // The following two lines are equivalent to
    // pop(fp[AROFF(m_savedRip)]) on x64.
    // rAsm is a scratch register.
    a.mflr(rAsm);
    a.std(rAsm, fp[AROFF(m_savedRip)]);

    emit(jmpr{i.target,i.args});
  }
  end.asm_label(a);
  {
    a.call(stub);
    emit(unwind{{i.targets[0], i.targets[1]}});
  }
}

void Vgen::emit(const syncpoint& i) {
  // skip the "ld 2,24(1)" or "nop" emitted by "Assembler::call" at the end
  TCA saved_pc = a.frontier() - call_skip_bytes_for_ret;
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", saved_pc,
         i.fix.pcOffset, i.fix.spOffset);
  env.meta.fixups.emplace_back(saved_pc, i.fix);
}

/*
 * Push/pop mechanism is as simple as X64: it stores 8 bytes below the RSP.
 */
void Vgen::emit(const pop& i) {
  a.ld(i.d, rsp()[0]);              // popped element
  a.addi(rsp(), rsp(), 8);          // recover stack
}
void Vgen::emit(const push& i) {
  a.stdu(i.s, rsp()[-8]);           // pushed element
}

void Vgen::emit(const load& i) {
  if (i.d.isGP()) {
    if (i.s.index.isValid()){
      a.ldx(i.d, i.s);
    } else if (i.s.disp & 0x3) {     // Unaligned memory access
      Vptr p = i.s;
      a.li64(rAsm, (int64_t)p.disp); // Load disp to reg
      p.disp = 0;                    // Remove disp
      p.index = rAsm;                // Set disp reg as index
      a.ldx(i.d, p);                 // Use ldx for unaligned memory access
    } else {
      a.ld(i.d, i.s);
    }
  } else {
    assertx(i.d.isSIMD());
    a.lfd(i.d, i.s);
  }
}

// This function can't be lowered as i.get() may not be bound that early.
void Vgen::emit(const loadqd& i) {
  a.li64(rAsm, (int64_t)i.s.get());
  a.ld(i.d, rAsm[0]);
}

void Vgen::emit(const unwind& i) {
  // skip the "ld 2,24(1)" or "nop" emitted by "Assembler::call" at the end
  TCA saved_pc = a.frontier() - call_skip_bytes_for_ret;
  catches.push_back({saved_pc, i.targets[1]});
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a.frontier(), i.target});

  // offset to be determined by a.patchBranch
  a.branchAuto(a.frontier());
}

void Vgen::emit(const jmpr& i) {
  a.mr(rfuncentry(), i.target.asReg());
  a.mtctr(rfuncentry());
  a.bctr();
}

void Vgen::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a.frontier(), taken});

    // offset to be determined by a.patchBranch
    a.branchAuto(a.frontier(), i.cc);
  }
  emit(jmp{i.targets[0]});
}
void Vgen::emit(const jcci& i) {
  a.branchAuto(i.taken, i.cc);
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
  a.isel(i.d, t, f, (4 * int(i.sf.asReg())) + bp.bi());
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    if (i.d.index.isValid()){
      a.stdx(i.s, i.d);
    } else {
      a.std(i.s, i.d);
    }
  } else {
    assertx(i.s.isSIMD());
    a.stfd(i.s, i.d);
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
  auto const mov_addr = emitSmashableMovq(a.code(), env.meta, 0, r64(i.d));
  auto const imm = reinterpret_cast<uint64_t>(mov_addr);
  smashMovq(mov_addr, (imm << 1) | 1);

  env.meta.addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

void Vgen::emit(const inittc&) {
  // initialize our rone register
  a.li(ppc64::rone(), 1);
}

void Vgen::emit(const leavetc&) {
  // should read enterTCExit address that was pushed by calltc/resumetc
  emit(pop{rAsm});
  a.mtlr(rAsm);
  a.blr();
}

void Vgen::emit(const calltc& i) {
  // Dummy call for branch predictor's sake:
  // the link stack would be wrong otherwise and mispredictions would occur
  a.bl(instr_size_in_bytes);  // jump to next instruction

  // this will be verified by emitCallToExit
  a.li64(rAsm, reinterpret_cast<int64_t>(i.exittc), false);
  emit(push{rAsm});

  // keep the return address as initialized by the vm frame
  a.ld(rfuncln(), i.fp[AROFF(m_savedRip)]);
  a.mtlr(rfuncln());

  // and jump. When it returns, it'll be to enterTCExit
  a.mr(rfuncentry(), i.target.asReg());
  a.mtctr(rfuncentry());
  a.bctr();
}

void Vgen::emit(const resumetc& i) {
  // Dummy call for branch predictor's sake:
  // the link stack would be wrong otherwise and mispredictions would occur
  a.bl(instr_size_in_bytes);  // jump to next instruction

  // this will be verified by emitCallToExit
  a.li64(rAsm, reinterpret_cast<int64_t>(i.exittc), false);
  emit(push{rAsm});

  // and jump. When it returns, it'll be to enterTCExit
  a.mr(rfuncentry(), i.target.asReg());
  a.mtctr(rfuncentry());
  a.bctr();
}

void Vgen::emit(const lea& i) {
  // could do this in a simplify pass
  if (i.s.disp == 0 && i.s.base.isValid() && !i.s.index.isValid()) {
    emit(copy{i.s.base, i.d});
  } else {
    // don't reuse addq and addqi as they need a SF
    Vptr p = i.s;
    if (p.index.isValid()) {
      a.add(i.d, p.base, p.index);
    } else {
      a.addi(i.d, p.base, p.disp);
    }
  }
}

void Vgen::emit(const call& i) {
  // Setup r1 with a valid frame in order to allow LR save by callee's prologue.
  a.addi(rsfp(), rsp(), -min_frame_size);
  a.std(rvmfp(), rsfp()[AROFF(m_sfp)]);
  // TOC save/restore is required by ABI for external functions.
  a.std(rtoc(), rsfp()[AROFF(SAVED_TOC())]);
  a.call(i.target, Assembler::CallArg::External);
  if (i.watch) {
    // skip the "ld 2,24(1)" or "nop" emitted by "Assembler::call" at the end
    *i.watch = a.frontier() - call_skip_bytes_for_ret;
    env.meta.watchpoints.push_back(i.watch);
  }
}

void Vgen::emit(const callr& i) {
  // Setup r1 with a valid frame in order to allow LR save by callee's prologue.
  a.addi(rsfp(), rsp(), -min_frame_size);
  a.std(rvmfp(), rsfp()[AROFF(m_sfp)]);
  // TOC save/restore is required by ABI for external functions.
  a.std(rtoc(), rsfp()[AROFF(SAVED_TOC())]);
  a.call(i.target.asReg(), Assembler::CallArg::External);
}

void Vgen::emit(const calls& i) {
  // calls is used to call c++ function like handlePrimeCacheInit so setup the
  // r1 pointer to a valid frame in order to allow LR save by callee's
  // prologue.
  a.addi(rsfp(), rsp(), -min_frame_size);
  a.std(rvmfp(), rsfp()[AROFF(m_sfp)]);
  emitSmashableCall(a.code(), env.meta, i.target);
}

void Vgen::emit(const callphp& i) {
  emitSmashableCall(a.code(), env.meta, i.stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const callarray& i) {
  // callarray target can indeed start with stublogue, therefore reuse callstub
  // approach
  emit(callstub{i.target});
}

void Vgen::emit(const callstub& i) {
  // Build a minimal call frame in order to save the LR but avoid writing to
  // rsp() directly as there are stubs that are called that doesn't have a
  // stublogue/stubret (e.g: freeLocalsHelpers)
  a.addi(rsfp(), rsp(), -min_frame_size);
  a.std(rvmfp(), rsfp()[AROFF(m_sfp)]);
  a.call(i.target);
}

void Vgen::emit(const callfaststub& i) {
  emit(callstub{i.target});
  emit(syncpoint{i.fix});
}

/////////////////////////////////////////////////////////////////////////////
/*
 * Stub function ABI
 */
void Vgen::emit(const stublogue& i) {
  // Recover the frame created on callstub.
  a.mr(rsp(), rsfp());
  if (i.saveframe) a.std(rvmfp(), rsp()[AROFF(m_sfp)]);

  // save return address on this frame
  a.mflr(rfuncln());
  a.std(rfuncln(), rsfp()[AROFF(m_savedRip)]);
}

void Vgen::emit(const stubret& i) {
  // rvmfp, if necessary.
  if (i.saveframe) a.ld(rvmfp(), rsp()[AROFF(m_sfp)]);

  // restore return address.
  a.ld(rfuncln(), rsp()[AROFF(m_savedRip)]);
  a.mtlr(rfuncln());

  // pop this frame as created by stublogue and return
  a.addi(rsp(), rsp(), min_frame_size);
  a.blr();
}

void Vgen::emit(const tailcallstub& i) {
  // tail call: perform a jmp instead of a call. Use current return address on
  // frame and undo stublogue allocation.
  a.ld(rfuncln(), rsp()[AROFF(m_savedRip)]);
  a.mtlr(rfuncln());
  a.mr(rsfp(), rsp());
  emit(jmpi{i.target, i.args});
}

void Vgen::emit(const testqi& i) {
  if (i.s0.fits(sz::word)) {
    a.li(rAsm, i.s0);
  } else {
    a.li32(rAsm, i.s0.l());
  }
  emit(testq{rAsm, i.s1, i.sf});
}

void Vgen::emit(const loadstubret& i) {
  // grab the return address and store this return address for phplogue
  a.ld(i.d, rsp()[AROFF(m_savedRip)]);
}

void Vgen::emit(const stubunwind& i) {
  // reset the return address from native frame due to call to the vm frame
  a.ld(rfuncln(), rsp()[AROFF(m_savedRip)]);
  a.mtlr(rfuncln());
  // pop this frame as created by stublogue
  a.addi(rsp(), rsp(), min_frame_size);
}

void Vgen::emit(const stubtophp& i) {
  emit(stubunwind{});
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

/* Fallback, when a vasm is not lowered */
template <typename Inst>
void lowerForPPC64(Vout& v, Inst& inst) {}

/*
 * Using macro to commonlize vasms lowering
 */

// Auxiliary macros to handle vasms with different attributes
#define NONE
#define ONE(attr_1)             inst.attr_1,
#define TWO(attr_1, attr_2)     inst.attr_1, inst.attr_2,
#define ONE_R64(attr_1)         Reg64(inst.attr_1),
#define TWO_R64(attr_1, attr_2) Reg64(inst.attr_1), Reg64(inst.attr_2),

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
X(lea,      s, s, d);

#undef X

// Load the Immed to a register in order to be able to use ppc64 CR instructions
#define X(vasm_src, vasm_dst, attr_imm, attrs)                          \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp;                                                             \
  lowerImm(inst.attr_imm, v, tmp);                                      \
  v << vasm_dst{tmp, attrs inst.sf};                                    \
}

X(addli,  addl,  s0, TWO(s1, d))
X(cmpqi,  cmpq,  s0, ONE(s1))
X(addqi,  addq,  s0, TWO(s1, d))
X(subqi,  subq,  s0, TWO(s1, d))

#undef X

// If it patches the Immed, replace the vasm for its non-immediate variant
#define X(vasm_src, vasm_dst_reg, attr_imm, attrs)                      \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp;                                                             \
  if (patchImm(inst.attr_imm, v, tmp))                                  \
    v << vasm_dst_reg{tmp, attrs inst.sf};                              \
}

X(andqi,  andq,  s0, TWO(s1, d))

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
X(addlim,  addl, load,  store,  m, s0)
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

// Handles MemoryRef arguments, load the data input from memory and
// filter the reg og 16bits to 64bits. These ones have no output other
// than the sign flag register update (SF)
#define X(vasm_src, vasm_filter, vasm_dst, vasm_load, attr_addr, attr)  \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp1 = v.makeReg(), tmp2 = v.makeReg();                          \
  Vptr p = inst.attr_addr;                                              \
  v << vasm_filter{inst.attr, tmp1};                                    \
  patchVptr(p, v);                                                      \
  v << vasm_load{p, tmp2};                                              \
  v << vasm_dst{tmp1, tmp2, inst.sf};                                   \
}

X(cmpwm, movw, cmpq, loadw, s1, s0)
X(cmpbm, movb, cmpq, loadb, s1, s0)

#undef X


/*
 * For PPC64 there are no instructions to perform operations with only part of
 * a register, differently of x86_64.
 *
 * The VASMs that are composed of these kind of operations, must be lowered to
 * first extract the operator into a register and later perform operation using
 * the entire register, i. e., 64bits. If it needs a result, like subtract
 * operations, the destination register must be filled considering the size of
 * operation. For example:
 *
 * Consider d register with the value "0x44582C24A50CAD2".
 * For a subl instruction (l means 32bits), the result must be placed in the
 * lower 32bits (X means the result value and S the sign extension):
 * "0xSSSSSSSXXXXXXXX".
 *
 * For operands the treatment is analogous, only the lower 32bits are
 * considered to perform the operation.
 *
 * PS: the core vasm of these lowerings may be further lowered, but as the
 * first one is a vasm that is not lowered (movb), there will be no issues as
 * only the first emitted vasm is not lowered on "lowerForPPC64(Vunit&)"
 */

// Lower byte/long to quadword variant, but extract only the bits needed
// through the vasm_filter macro argument
#define X(vasm_src, vasm_filter, vasm_dst, attr_dest)                   \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp1 = v.makeReg(), tmp2 = v.makeReg();                          \
  v << vasm_filter{inst.s0, tmp1};                                      \
  v << vasm_filter{inst.s1, tmp2};                                      \
  v << vasm_dst{tmp1, tmp2, attr_dest inst.sf};                         \
}

X(cmpb,  extrb, cmpq,  NONE)
X(testb, extrb, testq, NONE)
X(testl, movl, testq, NONE)
X(subl,  movl, subq,  ONE_R64(d))
X(xorb,  extrb, xorq,  ONE_R64(d))
X(xorl,  movl, xorq,  ONE_R64(d))
X(andb,  extrb, andq,  ONE_R64(d))
X(andl,  movl, andq,  ONE_R64(d))

#undef X

// Lower byte/long to quadword variant with immediate. As above, only extract
// the bits needed through the vasm_filter macro argument
#define X(vasm_src, vasm_filter, vasm_dst, attr_dest)                   \
void lowerForPPC64(Vout& v, vasm_src& inst) {                           \
  Vreg tmp = v.makeReg();                                               \
  v << vasm_filter{inst.s1, tmp};                                       \
  v << vasm_dst{inst.s0, tmp, attr_dest inst.sf};                       \
}

X(cmpbi,  extrb, cmpqi,  NONE)
X(testbi, extrb, testqi, NONE)
X(testli, movl, testqi, NONE)
X(subbi,  extrb, subqi,  ONE_R64(d))
X(subli,  movl, subqi,  ONE_R64(d))
X(xorbi,  extrb, xorqi,  ONE_R64(d))
X(andbi,  extrb, andqi,  ONE_R64(d))
X(andli,  movl, andqi,  ONE_R64(d))

#undef X

#undef NONE
#undef ONE
#undef TWO
#undef ONE_R64
#undef TWO_R64

/////////////////////////////////////////////////////////////////////////////
/*
 * PHP function ABI
 */
// phplogue and phpret doesn't save toc as it's saved on call.
void lowerForPPC64(Vout& v, phplogue& inst) {
  // store basic info on vm frame
  v << mflr{rfuncln()};
  v << store{rfuncln(), inst.fp[AROFF(m_savedRip)]};
}

void lowerForPPC64(Vout& v, phpret& inst) {
  v << load{inst.fp[AROFF(m_savedRip)], rfuncln()};
  if (!inst.noframe) v << load{inst.fp[AROFF(m_sfp)], inst.d};

  // for balancing the link stack (branch predictor), this should perform blr
  v << mtlr{rfuncln()};
  v << ret{RegSet()};
}

/*
 * Tail call elimination on ppc64: call without creating a stack and keep LR
 * contents as prior to the call.
 */
void lowerForPPC64(Vout& v, tailcallphp& inst) {
  v << load{inst.fp[AROFF(m_savedRip)], rfuncln()};
  v << mtlr{rfuncln()};
  v << jmpr{inst.target, inst.args};
}

/////////////////////////////////////////////////////////////////////////////

// Lower movs to copy
void lowerForPPC64(Vout& v, movtqb& inst) { v << copy{inst.s, inst.d}; }
void lowerForPPC64(Vout& v, movtql& inst) { v << copy{inst.s, inst.d}; }
void lowerForPPC64(Vout& v, movtdb& inst) { v << copy{inst.s, inst.d}; }
void lowerForPPC64(Vout& v, movtdq& inst) { v << copy{inst.s, inst.d}; }

// Lower all movzb* to extrb as ppc64 always sign extend the unused bits of reg.
void lowerForPPC64(Vout& v, movzbl& i)    { v << extrb{i.s, Reg8(i.d)}; }
void lowerForPPC64(Vout& v, movzbq& i)    { v << extrb{i.s, Reg8(i.d)}; }
void lowerForPPC64(Vout& v, movzlq& i)    { v << movl{i.s, Reg32(i.d)}; }

void lowerForPPC64(Vout& v, srem& i) {
  // remainder as described on divd documentation:
  auto tmpSf = v.makeReg(), tmpSf2 = v.makeReg();
  auto quotient = v.makeReg();
  auto quoxdiv = v.makeReg();
  v << divint{i.s0, i.s1, quotient};         // i.d = quotient
  v << imul{quotient, i.s1, quoxdiv, tmpSf}; // i.d = quotient*divisor
  v << subq{quoxdiv, i.s0, i.d, tmpSf2};     // i.d = remainder
}

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

void lowerForPPC64(Vout& v, pushm& inst) {
  auto tmp = v.makeReg();
  patchVptr(inst.s, v);
  v << load{inst.s, tmp};
  v << push{tmp};
}

void lowerForPPC64(Vout& v, popm& inst) {
  auto tmp = v.makeReg();
  patchVptr(inst.d, v);
  v << pop{tmp};
  v << store{tmp, inst.d};
}

void lowerForPPC64(Vout& v, setcc& inst) {
  auto zero = v.makeReg();
  v << ldimmq{0, zero};
  v << cmovq{inst.cc, inst.sf, zero, rone(), Vreg64{Reg64(inst.d)}};
}

void lowerForPPC64(Vout& v, cvttsd2siq& inst) {
  //  In order to fctidz be x64 compliant, it is necessary to return  the value
  //  0x8000000000000000 when the bit VXCVI is set. See fctidz instruction on
  //  ISA page 152, 4.6.7
  auto sfTmp = v.makeReg();
  auto tmpRaw = v.makeReg();
  auto tmpInt = v.makeReg();
  auto tmpReturnError = v.makeReg();
  Immed64 ErrorVal = 0x8000000000000000;

  // Convert value. If an error occurs, the overflow bit in CR0 is set.
  v << fctidz{inst.s, tmpRaw, sfTmp};
  // Load error code.
  v << ldimmq{ErrorVal, tmpReturnError};
  // Copy the float register to a general purpose register.
  v << copy{tmpRaw, tmpInt};

  // If the overflow bit is set, return the ErrorVal. Else, return the
  // value converted by fctidz.
  v << cmovq {ConditionCode::CC_O, sfTmp, tmpInt, tmpReturnError, inst.d};
}

/*
 * cmplim and cmpli can't allocate registers due to callfaststub on
 * emitDecRefWork! (it can't allocate temporary register through v.makeReg())
 *
 * Special handling is done below for them
 */
void lowerForPPC64(Vout& v, cmplim& inst) {
  Vptr p = inst.s1;
  patchVptr(p, v);                          // safe for emitDecRefWork

  // this temp reg is always needed. Use one of our scratches.
  Vreg tmp2 = Vreg32(PhysReg(rAsm));
  v << loadl{p, tmp2};

  // This temp reg is not always needed. It's also not used by emitDecRefWork
  Vreg tmp;
  if (patchImm(inst.s0, v, tmp)) v << cmpl {tmp,     tmp2, inst.sf};
  else                           v << cmpli{inst.s0, tmp2, inst.sf};
}
void lowerForPPC64(Vout& v, cmpli& inst) {
  Vreg tmp;
  if (patchImm(inst.s0, v, tmp)) v << cmpl {tmp,     inst.s1, inst.sf};
  else                           v << cmpli{inst.s0, inst.s1, inst.sf};
}

void lowerForPPC64(Vout& v, cmovb& inst) {
  auto t_64 = v.makeReg(), f_64 = v.makeReg();
  v << extrb{inst.f, f_64};
  v << extrb{inst.t, t_64};
  v << cmovq{inst.cc, inst.sf, f_64, t_64, Reg64(inst.d)};
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
  Vreg equal = v.makeReg();;
  Vreg nequal = v.makeReg();
  v << ldimmq{0, nequal};
  v << ldimmq{0xffffffffffffffff,equal};

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

void lowerForPPC64(Vout& v, absdbl& inst) {
  // parameters are in FP format but in Vreg, so first copy it to a VregDbl type
  auto before_conv = v.makeReg(), after_conv = v.makeReg(); // VregDbl register
  v << copy{inst.s, before_conv};
  v << fabs{before_conv, after_conv};
  // now move it back to Vreg
  v << copy{after_conv, inst.d};
}

void lowerForPPC64(Vout& v, decqmlock& inst) {
  Vptr p = inst.m;
  patchVptr(p, v);
  // decqmlock uses ldarx and stdcx instructions that are X-Form instruction
  // and support only index+base address mode. In case decqmlock uses base or
  // base + displacement with displacement less than 16 bits patchVptr will
  // not work so we need to copy the displacement to index register.
  if (!p.index.isValid()) {
    p.index = v.makeReg();
    v <<  ldimml{Immed(p.disp), p.index};
  }
  v << decqmlock{p, inst.sf};
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

void optimizePPC64(Vunit& unit, const Abi& abi, bool regalloc) {
  Timer timer(Timer::vasm_optimize);

  removeTrivialNops(unit);
  optimizePhis(unit);
  fuseBranches(unit);
  optimizeJmps(unit);
  optimizeExits(unit);

  lowerForPPC64(unit);

  simplify(unit);

  optimizeCopies(unit, abi);

  if (unit.needsRegAlloc()) {
    removeDeadCode(unit);
    if (regalloc) allocateRegisters(unit, abi);
  }
  if (unit.blocks.size() > 1) {
    optimizeJmps(unit);
  }
}

void emitPPC64(const Vunit& unit, Vtext& text, CGMeta& fixups,
               AsmInfo* asmInfo) {
  vasm_emit<Vgen>(unit, text, fixups, asmInfo);
}

///////////////////////////////////////////////////////////////////////////////
}}
