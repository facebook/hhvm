/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

/*
 * The HHVM's ARM64 backend works with an early-truncation policy.
 * That means that:
 *
 *  A Vreg8 is an extended W-register with a u8 value.
 *  A Vreg16 is an extended W-register with a u16 value.
 *  A Vreg32 is a W-register with a u32 value.
 *  A Vreg64 is a X-register with a u64 value.
 *
 * This allows to omit truncation instructions for sub-32-bit
 * operations. E.g. a testb{Vreg8 s0, Vreg8 s1} has to truncate
 * s0 and s1 before emitting a tst instruction. When using the
 * early-truncation policy, the testb{} emitter can rely on the
 * fact, that s0 and s1 are already truncated and can emit a
 * cmp instruction without preceding uxtb's.
 *
 * Conversely any arithmetic instruction has to sign extend any
 * Vreg8 before operating on it. Vasm is light on these instructions,
 * with only the following, currently: csinc[bw]{} and cmp[bw][i]{}.
 *
 * Early-truncation has also consequences to extension/truncation
 * vasm instructions. The following list shows how to use them:
 *
 * movzbw: Vreg8 -> Vreg16: mov w0, w0 #nop if s==d
 * movzbl: Vreg8 -> Vreg32: mov w0, w0 #nop if s==d
 * movzbq: Vreg8 -> Vreg64: uxtb x0, x0
 * movzwl: Vreg16 -> Vreg32 mov w0, w0 #nop if s==d
 * movzwq: Vreg16 -> Vreg64 uxth x0, x0
 * movzlq: Vreg32 -> Vreg64 uxtw x0, x0
 * movtqb: Vreg64 -> Vreg8:  uxtb w0, w0
 * movtql: Vreg64 -> Vreg32: uxtw w0, w0
 *
 * Early-truncation also implies, that instructions have to truncate
 * after performing the actual operation if it cannot guarantee that
 * the resulting VregN type matches. E.g. emitting code for the vasm
 * instruction andbi{Immed imm, Vreg8 s, Vreg8 d} has to truncate the
 * result to guarantee that register d indeed holds a u8 value.
 *
 * Note, that the early-truncation policy allows aarch64 specific
 * optimizations, which are not relevant on other architectures.
 * E.g. the x86_64 does not need this policy as the ISA allows
 * direct register accesses for Vreg8, Vreg16, Vreg32 and Vreg64
 * (e.g. AL, AX, EAX, RAX).
 *
 * The early-truncation policy relies on the following
 * requirements of the Vreg type-system:
 *
 *  * All VregNs are created for values of up to N bits
 *  * All conversions between VregNs are done via movz/movt vasm instructions
 */

#include "hphp/runtime/vm/jit/vasm-emit.h"

#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
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

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

using namespace arm;
using namespace vixl;

namespace arm { struct ImmFolder; }

namespace {
///////////////////////////////////////////////////////////////////////////////

static_assert(folly::kIsLittleEndian,
  "Code contains little-endian specific optimizations.");

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

vixl::MemOperand M(Vptr p) {
  assertx(p.base.isValid());
  if (p.index.isValid()) {
    assertx(p.disp == 0);
    return MemOperand(X(p.base), X(p.index), LSL, Log2(p.scale));
  }
  return MemOperand(X(p.base), p.disp);
}

vixl::Condition C(ConditionCode cc) {
  return arm::convertCC(cc);
}

/*
 * Uses the flags from the Vinstr which defs SF to determine
 * whether or not the Vixl assembler should emit code which
 * sets the status flags.
 */
vixl::FlagsUpdate UF(Vflags flags) {
  return flags ? SetFlags : LeaveFlags;
}

/*
 * There are numerous ARM instructions that don't set status flags, and
 * therefore those flags must be set synthetically in the emitters. This
 * assertion is applied to the emitters which don't set all of the status
 * flags required by the Vinstr which defs SF. The flags field of the
 * Vinstr is used to determine which bits are required. Those required
 * bits are compared against the bits which are actually set by the
 * implementation.
 */
template<class Inst> void checkSF(const Inst& i, StatusFlags s) {
  Vflags required = i.fl;
  Vflags set = static_cast<Vflags>(s);
  always_assert_flog((required & set) == required,
                     "should def SF but does not: {}\n",
                     vinst_names[Vinstr(i).op]);
}

template<class Inst> void checkSF(const Inst& i) {
  checkSF(i, StatusFlags::None);
}

/*
 * Returns true if the queried flag(s) is in the set of required flags.
 */
bool flagRequired(Vflags flags, StatusFlags flag) {
  return (flags & static_cast<Vflags>(flag));
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
    env.cb->sync(base);
  }

  static void emitVeneers(Venv& env);
  static void handleLiterals(Venv& env);
  static void retargetBinds(Venv& env);
  static void patch(Venv& env);

  static void pad(CodeBlock& cb) {
    vixl::MacroAssembler a { cb };
    auto const begin = cb.frontier();
    while (cb.available() >= 4) a.Brk(1);
    assertx(cb.available() == 0);
    cb.sync(begin);
  }

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(const Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }

  // intrinsics
  void emit(const copy& i);
  void emit(const copy2& i);
  void emit(const debugtrap& /*i*/) { a->Brk(0); }
  void emit(const fallthru& /*i*/);
  void emit(const killeffects& /*i*/) {}
  void emit(const ldimmb& i);
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const ldimmw& i);
  void emit(const ldundefq& /*i*/) {}
  void emit(const load& i);
  void emit(const store& i);
  void emit(const mcprep& i);

  // native function abi
  void emit(const call& i);
  void emit(const callr& i) { a->Blr(X(i.target)); }
  void emit(const calls& i);
  void emit(const ret& /*i*/) { a->Ret(); }

  // stub function abi
  void emit(const callstub& i);
  void emit(const callfaststub& i);

  // php function abi
  void emit(const callphp& i) {
    emit(call{i.target, i.args});
    setCallFuncId(env, a->frontier());
  }
  void emit(const callphpr& i) {
    emit(callr{i.target, i.args});
    setCallFuncId(env, a->frontier());
  }
  void emit(const inlinesideexit& i) {
    emit(call{tc::ustubs().inlineSideExit, i.args});
  }
  void emit(const contenter& i);
  void emit(const phpret& i);

  // vm entry abi
  void emit(const inittc& /*i*/) {}

  // exceptions
  void emit(const landingpad& /*i*/) {}
  void emit(const nothrow& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);

  // instructions
  void emit(const absdbl& i) { a->Fabs(D(i.d), D(i.s)); }
  void emit(const addl& i) { a->Add(W(i.d), W(i.s1), W(i.s0), UF(i.fl)); }
  void emit(const addli& i) { a->Add(W(i.d), W(i.s1), i.s0.l(), UF(i.fl)); }
  void emit(const addq& i) { a->Add(X(i.d), X(i.s1), X(i.s0), UF(i.fl));}
  void emit(const addqi& i) { a->Add(X(i.d), X(i.s1), i.s0.q(), UF(i.fl)); }
  void emit(const addsd& i) { a->Fadd(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const andb& i) { a->And(W(i.d), W(i.s1), W(i.s0), UF(i.fl)); }
  void emit(const andbi& i) { a->And(W(i.d), W(i.s1), i.s0.ub(), UF(i.fl)); }
  void emit(const andw& i) { a->And(W(i.d), W(i.s1), W(i.s0), UF(i.fl)); }
  void emit(const andwi& i) { a->And(W(i.d), W(i.s1), i.s0.uw(), UF(i.fl)); }
  void emit(const andl& i) { a->And(W(i.d), W(i.s1), W(i.s0), UF(i.fl)); }
  void emit(const andli& i) { a->And(W(i.d), W(i.s1), i.s0.l(), UF(i.fl)); }
  void emit(const andq& i) { a->And(X(i.d), X(i.s1), X(i.s0), UF(i.fl)); }
  void emit(const andqi& i) { a->And(X(i.d), X(i.s1), i.s0.q(), UF(i.fl)); }
  void emit(const andqi64& i) { a->And(X(i.d), X(i.s1), i.s0.q(), UF(i.fl)); }
  void emit(const btrq& i) {
    // NB: We can't directly store the result to i.d because, in case i.s1 is
    // dead after the btrq, the register allocator may allocated both i.d and
    // i.s1 to the same physical register, thus breaking the comparison
    // (subtraction) that is done below.
    a->Bic(rVixlScratch0, X(i.s1), 1 << i.s0.q());

    // Subtract the original value from the result to set the carry bit based on
    // whether they differ.
    a->Sub(vixl::xzr, rVixlScratch0, X(i.s1), UF(i.fl));
    a->Mov(X(i.d), rVixlScratch0);
  }
  void emit(const cmovb& i) { a->Csel(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const cmovw& i) { a->Csel(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const cmovl& i) { a->Csel(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const cmovq& i) { a->Csel(X(i.d), X(i.t), X(i.f), C(i.cc)); }
  // note: cmp{bw}[i] are emitted only for narrow comparisons and _do not_ sign
  // extend their arguments--these instructions are lowered to cmp{lq}[i] if
  // the comparison is not narrow or not equality/inequality
  void emit(const cmpb& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpbi& i) { a->Cmp(W(i.s1), i.s0.ub()); }
  void emit(const cmpw& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpwi& i) { a->Cmp(W(i.s1), i.s0.uw()); }
  void emit(const cmpl& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(const cmpli& i) { a->Cmp(W(i.s1), i.s0.l()); }
  void emit(const cmpq& i) { a->Cmp(X(i.s1), X(i.s0)); }
  void emit(const cmpqi& i) { a->Cmp(X(i.s1), i.s0.q()); }
  void emit(const cmpsd& i);
  // TODO(CDE): csinc[bw]{} Should a) sign extend and b) set SF for overflow
  void emit(const csincb& i) { a->Csinc(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const csincw& i) { a->Csinc(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const csincl& i) { a->Csinc(W(i.d), W(i.t), W(i.f), C(i.cc)); }
  void emit(const csincq& i) { a->Csinc(X(i.d), X(i.t), X(i.f), C(i.cc)); }
  void emit(const cvtsi2sd& i) { a->Scvtf(D(i.d), X(i.s)); }
  void emit(const decl& i) { a->Sub(W(i.d), W(i.s), 1, UF(i.fl)); }
  void emit(const decq& i) { a->Sub(X(i.d), X(i.s), 1, UF(i.fl)); }
  void emit(const decqmlock& i);
  void emit(const divint& i) { a->Sdiv(X(i.d), X(i.s0), X(i.s1)); }
  void emit(const divsd& i) { a->Fdiv(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const imul& i);
  void emit(const incl& i) { a->Add(W(i.d), W(i.s), 1, UF(i.fl)); }
  void emit(const incq& i) { a->Add(X(i.d), X(i.s), 1, UF(i.fl)); }
  void emit(const incw& i) { a->Add(W(i.d), W(i.s), 1, UF(i.fl)); }
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const jmpi& i);
  void emit(const jmpr& i) { a->Br(X(i.target)); }
  void emit(const ldbindretaddr& i);
  void emit(const lea& i);
  void emit(const leap& i);
  void emit(const lead& i);
  void emit(const loadb& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadl& i) { a->Ldr(W(i.d), M(i.s)); }
  void emit(const loadsd& i) { a->Ldr(D(i.d), M(i.s)); }
  void emit(const loadtqb& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadtql& i) { a->Ldr(W(i.d), M(i.s)); }
  void emit(const loadups& i);
  void emit(const loadw& i) { a->Ldrh(W(i.d), M(i.s)); }
  void emit(const loadzbl& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadzbq& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(const loadsbq& i) { a->Ldrsb(X(i.d), M(i.s)); }
  void emit(const loadsbl& i) { a->Ldrsb(W(i.d), M(i.s)); }
  void emit(const loadzwq& i) { a->Ldrh(W(i.d), M(i.s)); }
  void emit(const loadzlq& i) { a->Ldr(W(i.d), M(i.s)); }
  void emit(const movb& i) { if (i.d != i.s) a->Mov(W(i.d), W(i.s)); }
  void emit(const movw& i) { if (i.d != i.s) a->Mov(W(i.d), W(i.s)); }
  void emit(const movl& i) { if (i.d != i.s) a->Mov(W(i.d), W(i.s)); }
  void emit(const movsbl& i) { a->Sxtb(W(i.d), W(i.s)); }
  void emit(const movsbq& i) { a->Sxtb(X(i.d), W(i.s).X()); }
  void emit(const movswl& i) { a->Sxth(W(i.d), W(i.s)); }
  void emit(const movtqb& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(const movtqw& i) { a->Uxth(W(i.d), W(i.s)); }
  void emit(const movtql& i) { a->Uxtw(W(i.d), W(i.s)); }
  void emit(const movzbq& i) { a->Uxtb(X(i.d), W(i.s).X()); }
  void emit(const movzwq& i) { a->Uxth(X(i.d), W(i.s).X()); }
  void emit(const movzlq& i) { a->Uxtw(X(i.d), W(i.s).X()); }
  void emit(const mulsd& i) { a->Fmul(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const neg& i) { a->Neg(X(i.d), X(i.s), UF(i.fl)); }
  void emit(const nop& /*i*/) { a->Nop(); }
  void emit(const notb& i) { a->Mvn(W(i.d), W(i.s)); }
  void emit(const not& i) { a->Mvn(X(i.d), X(i.s)); }
  void emit(const orbi& i);
  void emit(const orq& i);
  void emit(const orwi& i);
  void emit(const orli& i);
  void emit(const orqi& i);
  void emit(const pop& i);
  void emit(const popp& i);
  void emit(const push& i);
  void emit(const pushp& i);
  void emit(const roundsd& i);
  void emit(const sar& i);
  void emit(const sarqi& i);
  void emit(const setcc& i) { a->Cset(W(i.d), C(i.cc)); }
  void emit(const shl& i);
  void emit(const shlli& i);
  void emit(const shlqi& i);
  void emit(const shrli& i);
  void emit(const shrqi& i);
  void emit(const sqrtsd& i) { a->Fsqrt(D(i.d), D(i.s)); }
  void emit(const srem& i);
  void emit(const storeb& i) { a->Strb(W(i.s), M(i.m)); }
  void emit(const storel& i) { a->Str(W(i.s), M(i.m)); }
  void emit(const storesd& i) { emit(store{i.s, i.m}); }
  void emit(const storeups& i);
  void emit(const storew& i) { a->Strh(W(i.s), M(i.m)); }
  void emit(const subl& i) { a->Sub(W(i.d), W(i.s1), W(i.s0), UF(i.fl)); }
  void emit(const subli& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), UF(i.fl)); }
  void emit(const subq& i) { a->Sub(X(i.d), X(i.s1), X(i.s0), UF(i.fl)); }
  void emit(const subqi& i) { a->Sub(X(i.d), X(i.s1), i.s0.q(), UF(i.fl)); }
  void emit(const subsd& i) { a->Fsub(D(i.d), D(i.s1), D(i.s0)); }
  void emit(const testb& i){ a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testbi& i){ a->Tst(W(i.s1), i.s0.ub()); }
  void emit(const testw& i){ a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testwi& i){ a->Tst(W(i.s1), i.s0.uw()); }
  void emit(const testl& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(const testli& i) { a->Tst(W(i.s1), i.s0.l()); }
  void emit(const testq& i) { a->Tst(X(i.s1), X(i.s0)); }
  void emit(const testqi& i) { a->Tst(X(i.s1), i.s0.q()); }
  void emit(const trap& /*i*/);
  void emit(const ucomisd& i) { a->Fcmp(D(i.s0), D(i.s1)); }
  void emit(const unpcklpd&);
  void emit(const xorb& i);
  void emit(const xorbi& i);
  void emit(const xorw& i);
  void emit(const xorwi& i);
  void emit(const xorl& i);
  void emit(const xorq& i);
  void emit(const xorqi& i);

  // arm intrinsics
  void emit(const prefetch& /*i*/) { /* ignored */ }
  void emit(const fcvtzs& i) { a->Fcvtzs(X(i.d), D(i.s)); }
  void emit(const mrs& i) { a->Mrs(X(i.r), vixl::SystemRegister(i.s.l())); }
  void emit(const msr& i) { a->Msr(vixl::SystemRegister(i.s.l()), X(i.r)); }
  void emit(const ubfmli& i) { a->ubfm(W(i.d), W(i.s), i.mr.w(), i.ms.w()); }

  void emit_nop() { a->Nop(); }

private:
  CodeBlock& frozen() { return env.text.frozen().code; }
  static void recordAddressImmediate(Venv& env, TCA addr) {
    env.meta.addressImmediates.insert(addr);
  }
  void recordAddressImmediate() {
    env.meta.addressImmediates.insert(env.cb->frontier());
  }

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

static CodeBlock* getBlock(Venv& env, CodeAddress a) {
  for (auto const& area : env.text.areas()) {
    if (area.code.contains(a)) {
      return &area.code;
    }
  }
  return nullptr;
}

void Vgen::emitVeneers(Venv& env) {
  auto& meta = env.meta;
  decltype(env.meta.veneers) notEmitted;

  for (auto const& veneer : meta.veneers) {
    auto cb = getBlock(env, veneer.source);
    if (!cb) {
      // If we can't find the code block, it must have been emitted by a Vunit
      // wrapping this one (bindjmp emits a Vunit within a Vunit).
      notEmitted.push_back(veneer);
      continue;
    }
    auto const vaddr = cb->frontier();

    FTRACE(1, "emitVeneers: source = {}, target = {}, veneer at {}\n",
           veneer.source, veneer.target, vaddr);

    // Emit the veneer code: LDR + BR.
    meta.veneerAddrs.insert(vaddr);
    MacroAssembler av{*cb};
    vixl::Label target_data;
    meta.addressImmediates.insert(vaddr);
    poolLiteral(*cb, meta, (uint64_t)makeTarget32(veneer.target), 32, true);
    av.bind(&target_data);
    av.Ldr(rAsm_w, &target_data);
    av.Br(rAsm);

    // Update the veneer source instruction to jump/call the veneer.
    auto const realSource = env.text.toDestAddress(veneer.source);
    CodeBlock tmpBlock;
    tmpBlock.init(realSource, kInstructionSize, "emitVeneers");
    MacroAssembler at{tmpBlock};
    int64_t offset = vaddr - veneer.source;
    auto sourceInst = Instruction::Cast(realSource);

    if (sourceInst->Mask(UnconditionalBranchMask) == B) {
      always_assert(is_int28(offset));
      at.b(offset >> kInstructionSizeLog2);

    } else if (sourceInst->Mask(UnconditionalBranchMask) == BL) {
      always_assert(is_int28(offset));
      at.bl(offset >> kInstructionSizeLog2);

    } else if (sourceInst->IsCondBranchImm()) {
      auto const cond = static_cast<Condition>(sourceInst->ConditionBranch());
      if (is_int21(offset)) {
        at.b(offset >> kInstructionSizeLog2, cond);
      } else {
        // The offset doesn't fit in a conditional jump. Hopefully it still fits
        // in an unconditional jump, in which case we add an appendix to the
        // veneer.
        offset += 2 * kInstructionSize;
        always_assert(is_int28(offset));
        // Add an appendix to the veneer, and jump to it instead.  The full
        // veneer in this case looks like:
        //   VENEER:
        //      LDR RX, LITERAL_ADDR
        //      BR  RX
        //   APPENDIX:
        //      B.CC VENEER
        //      B NEXT
        // And the conditional jump into the veneer is turned into a jump to the
        // appendix:
        //      B APPENDIX
        //   NEXT:

        // Turn the original conditional branch into an unconditional one.
        at.b(offset >> kInstructionSizeLog2);

        // Emit appendix.
        auto const appendix = cb->frontier();
        av.b(-2 /* veneer starts 2 instructions before the appendix */, cond);
        const int64_t nextOffset = (veneer.source + kInstructionSize) - // NEXT
          (vaddr + 3 * kInstructionSize); // addr of "B NEXT"
        always_assert(is_int28(nextOffset));
        av.b(nextOffset >> kInstructionSizeLog2);

        // Replace veneer.source with appendix in the relevant metadata.
        meta.smashableLocations.erase(veneer.source);
        meta.smashableLocations.insert(appendix);
        for (auto& tj : meta.inProgressTailJumps) {
          if (tj.toSmash() == veneer.source) tj.adjust(appendix);
        }
        for (auto& bind : meta.smashableBinds) {
          if (bind.smashable.toSmash() == veneer.source) {
            bind.smashable.adjust(appendix);
          }
        }
      }
    } else {
      always_assert_flog(0, "emitVeneers: invalid source instruction at source"
                         " {} (realSource = {})",
                         veneer.source, realSource);
    }
  }

  env.meta.veneers.swap(notEmitted);
}

void Vgen::handleLiterals(Venv& env) {
  decltype(env.meta.literalsToPool) notEmitted;
  for (auto const& pl : env.meta.literalsToPool) {
    auto const cb = getBlock(env, pl.patchAddress);
    if (!cb) {
      // If we can't find the code block it must have been emitted by a Vunit
      // wrapping this one.  (bindjmp emits a Vunit within a Vunit)
      notEmitted.push_back(pl);
      continue;
    }

    // Emit the literal.
    auto literalAddress = cb->frontier();
    if (pl.width == 32) {
      cb->dword(static_cast<uint32_t>(pl.value));
    } else if (pl.width == 64) {
      if (pl.smashable) {
        // Although the region is actually dead, we mark it as live, so that
        // the relocator can remove the padding.
        align(*cb, &env.meta, Alignment::QuadWordSmashable, AlignContext::Live);
        literalAddress = cb->frontier();
      }
      cb->qword(pl.value);
    } else {
      not_reached();
    }

    // Patch the LDR.
    auto const patchAddressActual =
      Instruction::Cast(env.text.toDestAddress(pl.patchAddress));
    assertx(patchAddressActual->IsLoadLiteral());
    patchAddressActual->SetImmPCOffsetTarget(
      Instruction::Cast(literalAddress),
      Instruction::Cast(pl.patchAddress));
  }

  if (env.meta.fallthru) {
    auto const fallthru = *env.meta.fallthru;
    auto const cb = getBlock(env, fallthru);
    if (!cb) {
      always_assert_flog(false,
                         "Fallthrus shouldn't be used in nested Vunits.");
    }
    auto const blockEndAddr = cb->frontier();
    auto const startAddr = cb->toDestAddress(fallthru);
    CodeBlock tmp;
    tmp.init(startAddr, kInstructionSize, "Tmp");
    // Write the jmp.
    Assembler a { tmp };
    recordAddressImmediate(env, fallthru);
    a.b((blockEndAddr - fallthru) >> kInstructionSizeLog2);
  }

  env.meta.literalsToPool.swap(notEmitted);
}

void Vgen::retargetBinds(Venv& env) {
}

void Vgen::patch(Venv& env) {
  // Patch the 32 bit target of the LDR
  auto patch = [&env](TCA instr, TCA target) {
    // The LDR loading the address to branch to.
    auto ldr = Instruction::Cast(instr);
    auto const DEBUG_ONLY br = ldr->NextInstruction();
    assertx(ldr->Mask(LoadLiteralMask) == LDR_w_lit &&
            br->Mask(UnconditionalBranchToRegisterMask) == BR &&
            ldr->Rd() == br->Rn());
    // The address the LDR loads.
    auto targetAddr = ldr->LiteralAddress();
    // Patch the 32 bit target following the LDR and BR
    patchTarget32(targetAddr, target);
  };

  for (auto const& p : env.jmps) {
    auto addr = env.text.toDestAddress(p.instr);
    auto const target = env.addrs[p.target];
    assertx(target);
    if (env.meta.smashableLocations.count(p.instr)) {
      assertx(possiblySmashableJmp(addr));
      // Update `addr' to point to the veneer.
      addr = TCA(vixl::Instruction::Cast(addr)->ImmPCOffsetTarget());
    }
    // Patch the address we are jumping to.
    patch(addr, target);
  }
  for (auto const& p : env.jccs) {
    auto addr = env.text.toDestAddress(p.instr);
    auto const target = env.addrs[p.target];
    assertx(target);
    if (env.meta.smashableLocations.count(p.instr)) {
      assertx(possiblySmashableJcc(addr));
      // Update `addr' to point to the veneer.
      addr = TCA(vixl::Instruction::Cast(addr)->ImmPCOffsetTarget());
    } else {
      assertx(Instruction::Cast(addr)->IsCondBranchImm());
      // If the jcc starts with a conditional jump, patch the next instruction
      // (which should start with a LDR).
      addr += kInstructionSize;
    }
    patch(addr, target);
  }
  for (auto const& p : env.leas) {
    auto addr = env.text.toDestAddress(p.instr);
    auto const target = env.vaddrs[p.target];

    // Get the address the LDR loads.
    auto const ldr = Instruction::Cast(addr);
    assertx(ldr->Mask(LoadLiteralMask) == LDR_w_lit);
    auto literalAddr = ldr->LiteralAddress();

    // Patch it to target
    patchTarget32(literalAddr, target);
  }
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const copy& i) {
  if (i.s == i.d) return;
  if (i.s.isGP() && i.d.isGP()) {
    a->Mov(X(i.d), X(i.s));
  } else if (i.s.isSIMD() && i.d.isGP()) {
    a->Fmov(X(i.d), D(i.s));
  } else if (i.s.isGP() && i.d.isSIMD()) {
    a->Fmov(D(i.d), X(i.s));
  } else {
    assertx(i.s.isSIMD() && i.d.isSIMD());
    a->mov(V(i.d), V(i.s));
  }
}

void Vgen::emit(const copy2& i) {
  assertx(i.s0.isValid() && i.s1.isValid() && i.d0.isValid() && i.d1.isValid());
  auto s0 = i.s0, s1 = i.s1, d0 = i.d0, d1 = i.d1;
  assertx(d0 != d1);
  if (d0 == s1) {
    if (d1 == s0) {
      a->Eor(X(d0), X(d0), X(s0));
      a->Eor(X(s0), X(d0), X(s0));
      a->Eor(X(d0), X(d0), X(s0));
    } else {
      // could do this in a simplify pass
      if (s1 != d1) a->Mov(X(s1), X(d1)); // save s1 first; d1 != s0
      if (s0 != d0) a->Mov(X(s0), X(d0));
    }
  } else {
    // could do this in a simplify pass
    if (s0 != d0) a->Mov(X(s0), X(d0));
    if (s1 != d1) a->Mov(X(s1), X(d1));
  }
}

void emitSimdImmInt(vixl::MacroAssembler* a, uint64_t val, Vreg d) {
  // Assembler::fmov emits a ldr from a literal pool if IsImmFP64 is false.
  // In that case, emit the raw bits into a GPR first and then move them
  // unmodified into destination SIMD
  union { double dval; uint64_t ival; };
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
void Vgen::emit(const fallthru& /*i*/) {
  always_assert(!env.meta.fallthru);
  env.meta.fallthru = a->frontier();
  a->nop();
}

#define Y(vasm_opc, simd_w, vr_w, gpr_w, imm) \
void Vgen::emit(const vasm_opc& i) {          \
  if (i.d.isSIMD()) {                         \
    emitSimdImmInt(a, static_cast<uint##vr_w##_t>(i.s.simd_w()), i.d);     \
  } else {                                    \
    Vreg##vr_w d = i.d;                       \
    a->Mov(gpr_w(d), imm);                    \
  }                                           \
}

Y(ldimmb, ub, 8, W, i.s.ub())
Y(ldimmw, uw, 16, W, i.s.uw())
Y(ldimml, l, 32, W, i.s.l())
Y(ldimmq, q, 64, X, i.s.q())

#undef Y

void Vgen::emit(const load& i) {
  if (i.d.isGP()) {
    a->Ldr(X(i.d), M(i.s));
  } else {
    a->Ldr(D(i.d), M(i.s));
  }
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    if (i.s == arm::rsp()) {
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
   * MethodCache::handleStaticCall can tell it's not been smashed yet
   */

  align(*env.cb, &env.meta, Alignment::SmashMovq, AlignContext::Live);
  auto const imm = reinterpret_cast<uint64_t>(a->frontier());
  emitSmashableMovq(*env.cb, env.meta, (imm << 1) | 1, r64(i.d));

  env.meta.addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const call& i) {
  recordAddressImmediate();
  a->Mov(rAsm, i.target);
  a->Blr(rAsm);
  if (i.watch) {
    *i.watch = a->frontier();
    env.meta.watchpoints.push_back(i.watch);
  }
}

void Vgen::emit(const calls& i) {
  emitSmashableCall(*env.cb, env.meta, i.target);
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const phpret& i) {
  // prefer load-pair instruction
  if (!i.noframe) {
    a->ldp(X(arm::rvmfp()), X(rlr()), X(i.fp)[AROFF(m_sfp)]);
  } else {
    a->Ldr(X(rlr()), X(i.fp)[AROFF(m_savedRip)]);
  }
  emit(ret{});
}

void Vgen::emit(const contenter& i) {
  vixl::Label stub, end;

  // Jump past the stub below.
  recordAddressImmediate();
  a->B(&end);

  // We call into this stub from the end below. Take that LR and store it in
  // m_savedRip. Then jump to the target.
  a->bind(&stub);
  a->Str(X(rlr()), M(i.fp[AROFF(m_savedRip)]));
  a->Br(X(i.target));

  // Call to stub above and then unwind.
  a->bind(&end);
  recordAddressImmediate();
  a->Bl(&stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const nothrow& /*i*/) {
  env.meta.catches.emplace_back(a->frontier(), nullptr);
}

void Vgen::emit(const syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {}\n", a->frontier(), i.fix.show());
  env.meta.fixups.emplace_back(a->frontier(), i.fix);
  env.record_inline_stack(a->frontier());
}

void Vgen::emit(const unwind& i) {
  catches.push_back({a->frontier(), i.targets[1]});
  env.record_inline_stack(a->frontier());
  emit(jmp{i.targets[0]});
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Flags
 *   SF should be set to MSB of the result
 *   CF, OF should be set to (1, 1) if the result is truncated, (0, 0) otherwise
 *   ZF, AF, PF are undefined
 *
 * In the following implementation,
 *   N, Z, V are updated according to result
 *   C is cleared (FIXME)
 */
void Vgen::emit(const imul& i) {

  // Do the multiplication
  a->Mul(X(i.d), X(i.s0), X(i.s1));

  // If we have to set any flags, then always set N and Z since it's cheap.
  // Only set V when absolutely necessary. C is not supported.
  if (i.fl) {
    vixl::Label after;

    checkSF(i, StatusFlags::NotC);

    if (flagRequired(i.fl, StatusFlags::V)) {
      vixl::Label checkSign;
      vixl::Label Overflow;

      // Do the multiplication for the upper 64 bits of a 128 bit result.
      // If the result is not all zeroes or all ones, then we have overflow.
      // If the result is all zeroes or all ones, and the sign is the same,
      // for both hi and low, then there is no overflow.
      a->smulh(rAsm, X(i.s0), X(i.s1));

      // If hi is all 0's or 1's, then check the sign, else overflow
      // (fallthrough).
      recordAddressImmediate();
      a->Cbz(rAsm, &checkSign);
      a->Cmp(rAsm, -1);
      recordAddressImmediate();
      a->B(&checkSign, vixl::eq);

      // Overflow, so conditionally set N and Z bits and then or in V bit.
      a->Bind(&Overflow);
      a->Bic(vixl::xzr, X(i.d), vixl::xzr, SetFlags);
      a->Mrs(rAsm, NZCV);
      a->Orr(rAsm, rAsm, 1<<28);
      a->Msr(NZCV, rAsm);
      recordAddressImmediate();
      a->B(&after);

      // Check the signs of hi and lo.
      a->Bind(&checkSign);
      a->Eor(rAsm, rAsm, X(i.d));
      recordAddressImmediate();
      a->Tbnz(rAsm, 63, &Overflow);
    }

    // No Overflow, so conditionally set the N and Z only
    a->Bic(vixl::xzr, X(i.d), vixl::xzr, SetFlags);

    a->bind(&after);
  }
}

void Vgen::emit(const decqmlock& i) {
  auto adr = M(i.m);
  /* Use VIXL's macroassembler scratch regs. */
  a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);
  if (Cfg::Jit::ArmLse) {
    a->Mov(rVixlScratch0, -1);
    a->ldaddal(rVixlScratch0, rVixlScratch0, adr);
    a->Sub(rAsm, rVixlScratch0, 1, SetFlags);
  } else {
    vixl::Label again;
    a->bind(&again);
    a->ldxr(rAsm, adr);
    a->Sub(rAsm, rAsm, 1, SetFlags);
    a->stxr(rVixlScratch0, rAsm, adr);
    recordAddressImmediate();
    a->Cbnz(rVixlScratch0, &again);
  }
  /* Restore VIXL's scratch regs. */
  a->SetScratchRegisters(rVixlScratch0, rVixlScratch1);
}

void Vgen::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});
    vixl::Label skip, data;

    // Emit a "far JCC" sequence for easy patching later.  Static relocation
    // might be able to simplify this later (see optimizeFarJcc()).
    recordAddressImmediate();
    a->B(&skip, vixl::InvertCondition(C(i.cc)));
    recordAddressImmediate();
    poolLiteral(*env.cb, env.meta, (uint64_t)makeTarget32(a->frontier()),
                32, false);
    a->bind(&data);  // This will be remmaped during the handleLiterals phase.
    a->Ldr(rAsm_w, &data);
    a->Br(rAsm);
    a->bind(&skip);
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const jcci& i) {
  vixl::Label skip;

  recordAddressImmediate();
  a->B(&skip, vixl::InvertCondition(C(i.cc)));
  emit(jmpi{i.taken});
  a->bind(&skip);
}

void Vgen::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  vixl::Label data;

  // Emit a "far JMP" sequence for easy patching later.  Static relocation
  // might be able to simplify this (see optimizeFarJmp()).
  recordAddressImmediate();
  poolLiteral(*env.cb, env.meta, (uint64_t)a->frontier(), 32, false);
  a->bind(&data); // This will be remapped during the handleLiterals phase.
  a->Ldr(rAsm_w, &data);
  a->Br(rAsm);
}

void Vgen::emit(const jmpi& i) {
  vixl::Label data;

  // If target can be addressed by pc relative offset (signed 26 bits), emit
  // PC relative jump. Else, emit target address into code and load from there.
  auto diff = (i.target - a->frontier()) >> vixl::kInstructionSizeLog2;
  if (vixl::is_int26(diff)) {
    recordAddressImmediate();
    a->b(diff);
  } else {
    // Cannot use simple a->Mov() since such a sequence cannot be
    // adjusted while live following a relocation.
    recordAddressImmediate();
    poolLiteral(*env.cb, env.meta, (uint64_t)i.target, 32, false);
    a->bind(&data); // This will be remapped during the handleLiterals phase.
    a->Ldr(rAsm_w, &data);
    a->Br(rAsm);
  }
}

void Vgen::emit(const ldbindretaddr& i) {
  auto const addr = a->frontier();
  emit(leap{reg::rip[(intptr_t)addr], i.d});
  env.ldbindretaddrs.push_back({addr, i.target, i.spOff});
}

void Vgen::emit(const lea& i) {
  auto p = i.s;
  assertx(p.base.isValid());
  if (p.index.isValid()) {
    assertx(p.disp == 0);
    a->Add(X(i.d), X(p.base), Operand(X(p.index), LSL, Log2(p.scale)));
  } else {
    a->Add(X(i.d), X(p.base), p.disp);
  }
}

void Vgen::emit(const leap& i) {
  vixl::Label imm_data;
  vixl::Label after_data;

  // Cannot use simple a->Mov() since such a sequence cannot be
  // adjusted while live following a relocation.
  recordAddressImmediate();
  poolLiteral(*env.cb, env.meta, (uint64_t)makeTarget32(i.s.r.disp),
              32, false);
  a->bind(&imm_data);  // This will be remapped during the handleLiterals phase.
  a->Ldr(W(i.d), &imm_data);
}

void Vgen::emit(const lead& i) {
  recordAddressImmediate();
  a->Mov(X(i.d), i.s.get());
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
 */
#define Y(vasm_opc, arm_opc, gpr_w, s0, zr)           \
void Vgen::emit(const vasm_opc& i) {                  \
  a->arm_opc(gpr_w(i.d), gpr_w(i.s1), s0);            \
  if (i.fl) {                                         \
    a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags); \
  }                                                   \
}

Y(orbi, Orr, W, i.s0.ub(), wzr);
Y(orwi, Orr, W, i.s0.uw(), xzr);
Y(orli, Orr, W, i.s0.l(), xzr);
Y(orqi, Orr, X, i.s0.q(), xzr);
Y(orq, Orr, X, X(i.s0), xzr);
Y(xorb, Eor, W, W(i.s0), wzr);
Y(xorbi, Eor, W, i.s0.ub(), wzr);
Y(xorw, Eor, W, W(i.s0), wzr);
Y(xorwi, Eor, W, i.s0.uw(), wzr);
Y(xorl, Eor, W, W(i.s0), wzr);
Y(xorq, Eor, X, X(i.s0), xzr);
Y(xorqi, Eor, X, i.s0.q(), xzr);

#undef Y

void Vgen::emit(const pop& i) {
  // SP access must be 8 byte aligned. Use rAsm instead.
  a->Mov(rAsm, sp);
  a->Ldr(X(i.d), MemOperand(rAsm, 8, PostIndex));
  a->Mov(sp, rAsm);
}

void Vgen::emit(const push& i) {
  // SP access must be 8 byte aligned. Use rAsm instead.
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

void Vgen::emit(const srem& i) {
  a->Sdiv(rAsm, X(i.s0), X(i.s1));
  a->Msub(X(i.d), rAsm, X(i.s1), X(i.s0));
}

void Vgen::emit(const trap& i) {
  env.meta.trapReasons.emplace_back(a->frontier(), i.reason);
  a->Brk(1);
}

void Vgen::emit(const unpcklpd& i) {
  // i.d and i.s1 can be same, i.s0 is unique.
  if (i.d != i.s1) a->fmov(D(i.d), D(i.s1));
  a->fmov(rAsm, D(i.s0));
  a->fmov(D(i.d), 1, rAsm);
}

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const cmpsd& i) {
  /*
   * cmpsd doesn't update SD, so read the flags into a temp.
   * Use one of the macroassembler scratch regs .
   */
  a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);
  a->Mrs(rVixlScratch0, NZCV);

  a->Fcmp(D(i.s0), D(i.s1));
  switch (i.pred) {
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

  /* Copy the flags back to the system register. */
  a->Msr(NZCV, rVixlScratch0);
  a->SetScratchRegisters(rVixlScratch0, rVixlScratch1);
}


///////////////////////////////////////////////////////////////////////////////

/*
 * For the shifts:
 *
 * C is set through inspection
 * N, Z are updated according to result
 * V is cleared (FIXME)
 * PF, AF are not available
 *
 * Only set the flags if there are any required flags (i.fl).
 * Setting the C flag is particularly expensive, so when setting
 * flags check this flag specifically.
 */
#define Y(vasm_opc, arm_opc, gpr_w, zr)                      \
void Vgen::emit(const vasm_opc& i) {                         \
  if (!i.fl) {                                               \
    /* Just perform the shift. */                            \
    a->arm_opc(gpr_w(i.d), gpr_w(i.s1), gpr_w(i.s0));        \
  } else {                                                   \
    checkSF(i, StatusFlags::NotV);                           \
    if (!flagRequired(i.fl, StatusFlags::C)) {               \
      /* Perform the shift and set N and Z. */               \
      a->arm_opc(gpr_w(i.d), gpr_w(i.s1), gpr_w(i.s0));      \
      a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags);      \
    } else {                                                 \
      /* Use VIXL's macroassembler scratch regs. */          \
      a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);      \
      /* Perform the shift using temp and set N and Z. */    \
      a->arm_opc(rVixlScratch0, gpr_w(i.s1), gpr_w(i.s0));   \
      a->Bic(vixl::zr, rVixlScratch0, vixl::zr, SetFlags);   \
      /* Read the flags into a temp. */                      \
      a->Mrs(rAsm, NZCV);                                    \
      /* Reshift right leaving the last bit as bit 0. */     \
      a->Sub(rVixlScratch1, gpr_w(i.s0), 1);                 \
      a->Lsr(rVixlScratch1, gpr_w(i.s1), rVixlScratch1);     \
      /* Negate the bits, including bit 0 to match X64. */   \
      a->Mvn(rVixlScratch1, rVixlScratch1);                  \
      /* Copy bit zero into bit 29 of the flags. */          \
      a->bfm(rAsm, rVixlScratch1, 35, 0);                    \
      /* Copy the flags back to the system register. */      \
      a->Msr(NZCV, rAsm);                                    \
      /* Copy the result to the destination. */              \
      a->Mov(gpr_w(i.d), rVixlScratch0);                     \
      /* Restore VIXL's scratch regs. */                     \
      a->SetScratchRegisters(rVixlScratch0, rVixlScratch1);  \
    }                                                        \
  }                                                          \
}

Y(sar, Asr, X, xzr)

#undef Y

#define Y(vasm_opc, arm_opc, gpr_w, sz, zr)                 \
void Vgen::emit(const vasm_opc& i) {                        \
  if (!i.fl) {                                              \
    /* Just perform the shift. */                           \
    a->arm_opc(gpr_w(i.d), gpr_w(i.s1), gpr_w(i.s0));       \
  } else {                                                  \
    checkSF(i, StatusFlags::NotV);                          \
    if (!flagRequired(i.fl, StatusFlags::C)) {              \
      /* Perform the shift and set N and Z. */              \
      a->arm_opc(gpr_w(i.d), gpr_w(i.s1), gpr_w(i.s0));     \
      a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags);     \
    } else {                                                \
      /* Use VIXL's macroassembler scratch regs. */         \
      a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);     \
      /* Perform the shift using temp and set N and Z. */   \
      a->arm_opc(rVixlScratch0, gpr_w(i.s1), gpr_w(i.s0));  \
      a->Bic(vixl::zr, rVixlScratch0, vixl::zr, SetFlags);  \
      /* Read the flags into a temp. */                     \
      a->Mrs(rAsm, NZCV);                                   \
      /* Reshift right leaving the last bit as bit 0. */    \
      a->Mov(rVixlScratch1, sz);                            \
      a->Sub(rVixlScratch1, rVixlScratch1, gpr_w(i.s0));    \
      a->Lsr(rVixlScratch1, gpr_w(i.s1), rVixlScratch1);    \
      /* Negate the bits, including bit 0 to match X64. */  \
      a->Mvn(rVixlScratch1, rVixlScratch1);                 \
      /* Copy bit zero into bit 29 of the flags. */         \
      a->bfm(rAsm, rVixlScratch1, 35, 0);                   \
      /* Copy the flags back to the system register. */     \
      a->Msr(NZCV, rAsm);                                   \
      /* Copy the result to the destination. */             \
      a->Mov(gpr_w(i.d), rVixlScratch0);                    \
      /* Restore VIXL's scratch regs. */                    \
      a->SetScratchRegisters(rVixlScratch0, rVixlScratch1); \
    }                                                       \
  }                                                         \
}

Y(shl, Lsl, X, 64, xzr)

#undef Y

#define Y(vasm_opc, arm_opc, gpr_w, zr)                     \
void Vgen::emit(const vasm_opc& i) {                        \
  if (!i.fl) {                                              \
    /* Just perform the shift. */                           \
    a->arm_opc(gpr_w(i.d), gpr_w(i.s1), i.s0.l());          \
  } else {                                                  \
    checkSF(i, StatusFlags::NotV);                          \
    if (!flagRequired(i.fl, StatusFlags::C)) {              \
      /* Perform the shift and set N and Z. */              \
      a->arm_opc(gpr_w(i.d), gpr_w(i.s1), i.s0.l());        \
      a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags);     \
    } else {                                                \
      /* Use VIXL's macroassembler scratch regs. */         \
      a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);     \
      /* Perform the shift using temp and set N and Z. */   \
      a->arm_opc(rVixlScratch0, gpr_w(i.s1), i.s0.l());     \
      a->Bic(vixl::zr, rVixlScratch0, vixl::zr, SetFlags);  \
      /* Read the flags into a temp. */                     \
      a->Mrs(rAsm, NZCV);                                   \
      /* Reshift right leaving the last bit as bit 0. */    \
      a->Lsr(rVixlScratch1, gpr_w(i.s1), i.s0.l() - 1);     \
      /* Negate the bits, including bit 0 to match X64. */  \
      a->Mvn(rVixlScratch1, rVixlScratch1);                 \
      /* Copy bit zero into bit 29 of the flags. */         \
      a->bfm(rAsm, rVixlScratch1, 35, 0);                   \
      /* Copy the flags back to the system register. */     \
      a->Msr(NZCV, rAsm);                                   \
      /* Copy the result to the destination. */             \
      a->Mov(gpr_w(i.d), rVixlScratch0);                    \
      /* Restore VIXL's scratch regs. */                    \
      a->SetScratchRegisters(rVixlScratch0, rVixlScratch1); \
    }                                                       \
  }                                                         \
}

Y(sarqi, Asr, X, xzr)
Y(shrli, Lsr, W, wzr)
Y(shrqi, Lsr, X, xzr)

#undef Y

#define Y(vasm_opc, arm_opc, gpr_w, sz, zr)                  \
void Vgen::emit(const vasm_opc& i) {                         \
  if (!i.fl) {                                               \
    /* Just perform the shift. */                            \
    a->arm_opc(gpr_w(i.d), gpr_w(i.s1), i.s0.l());           \
  } else {                                                   \
    checkSF(i, StatusFlags::NotV);                           \
    if (!flagRequired(i.fl, StatusFlags::C)) {               \
      /* Perform the shift and set N and Z. */               \
      a->arm_opc(gpr_w(i.d), gpr_w(i.s1), i.s0.l());         \
      a->Bic(vixl::zr, gpr_w(i.d), vixl::zr, SetFlags);      \
    } else {                                                 \
      /* Use VIXL's macroassembler scratch regs. */          \
      a->SetScratchRegisters(vixl::NoReg, vixl::NoReg);      \
      /* Perform the shift using temp and set N and Z. */    \
      a->arm_opc(rVixlScratch0, gpr_w(i.s1), i.s0.l());      \
      a->Bic(vixl::zr, rVixlScratch0, vixl::zr, SetFlags);   \
      /* Read the flags into a temp. */                      \
      a->Mrs(rAsm, NZCV);                                    \
      /* Reshift right leaving the last bit as bit 0. */     \
      a->Lsr(rVixlScratch1, gpr_w(i.s1), sz - i.s0.l());     \
      /* Negate the bits, including bit 0 to match X64. */   \
      a->Mvn(rVixlScratch1, rVixlScratch1);                  \
      /* Copy bit zero into bit 29 of the flags. */          \
      a->bfm(rAsm, rVixlScratch1, 35, 0);                    \
      /* Copy the flags back to the system register. */      \
      a->Msr(NZCV, rAsm);                                    \
      /* Copy the result to the destination. */              \
      a->Mov(gpr_w(i.d), rVixlScratch0);                     \
      /* Restore VIXL's scratch regs. */                     \
      a->SetScratchRegisters(rVixlScratch0, rVixlScratch1);  \
    }                                                        \
  }                                                          \
}

Y(shlli, Lsl, W, 32, wzr)
Y(shlqi, Lsl, X, 64, xzr)

#undef Y

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const popp& i) {
  a->Ldp(X(i.d0), X(i.d1), MemOperand(sp, 16, PostIndex));
}

void Vgen::emit(const pushp& i) {
  a->Stp(X(i.s1), X(i.s0), MemOperand(sp, -16, PreIndex));
}

///////////////////////////////////////////////////////////////////////////////

template<typename Lower>
void lower_impl(Vunit& unit, Vlabel b, size_t i, Lower lower) {
  vmodify(unit, b, i, [&] (Vout& v) { lower(v); return 1; });
}

template <typename Inst>
void lower(const VLS& /*env*/, Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {}

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
      // ldr/str allow [base], nothing to lower.
      break;

    case BASE | INDEX:
      if (p.scale != 1 && p.scale != uint8_t(p.width)) {
        auto t = v.makeReg();
        v << shlqi{Log2(p.scale), p.index, t, v.makeReg()};
        p.index = t;
        p.scale = 1;
      }
      break;

    case INDEX:
      // Not supported, convert to [base].
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << shlqi{Log2(p.scale), p.index, t, v.makeReg()};
        p.base = t;
      } else {
        p.base = p.index;
      }
      p.index = Vreg{};
      p.scale = 1;
      break;

    case BASE | DISP: {
      // ldr/str allow [base, #imm], where #imm is [-256 .. 255].
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
      // Not supported, convert to [base].
      auto base = v.makeReg();
      v << ldimmq{Immed64(p.disp), base};
      p.base = base;
      p.index = Vreg{};
      p.scale = 1;
      p.disp = 0;
      break;
    }

    case INDEX | DISP:
      // Not supported, convert to [base, #imm] or [base, index].
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << shlqi{Log2(p.scale), p.index, t, v.makeReg()};
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
      // Not supported, convert to [base, index].
      auto index = v.makeReg();
      if (p.scale > 1) {
        auto t = v.makeReg();
        v << shlqi{Log2(p.scale), p.index, t, v.makeReg()};
        v << addqi{p.disp, t, index, v.makeReg()};
      } else {
        v << addqi{p.disp, p.index, index, v.makeReg()};
      }
      p.index = index;
      p.scale = 1;
      p.disp = 0;
      break;
    }
  }
}

#define Y(vasm_opc, m)                                      \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(e.unit, b, z, [&] (Vout& v) {                  \
    lowerVptr(i.m, v);                                      \
    v << i;                                                 \
  });                                                       \
}

Y(decqmlock, m)
Y(lea, s)
Y(load, s)
Y(loadb, s)
Y(loadl, s)
Y(loadsd, s)
Y(loadtqb, s)
Y(loadtql, s)
Y(loadups, s)
Y(loadw, s)
Y(loadzbl, s)
Y(loadzbq, s)
Y(loadzwq, s)
Y(loadzlq, s)
Y(store, d)
Y(storeb, m)
Y(storel, m)
Y(storesd, m)
Y(storeups, m)
Y(storew, m)

#undef Y

#define Y(vasm_opc, lower_opc, load_opc, store_opc, arg, m) \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(e.unit, b, z, [&] (Vout& v) {                  \
    lowerVptr(i.m, v);                                      \
    auto r0 = v.makeReg(), r1 = v.makeReg();                \
    v << load_opc{i.m, r0};                                 \
    v << lower_opc{arg, r0, r1, i.sf, i.fl};                \
    v << store_opc{r1, i.m};                                \
  });                                                       \
}

Y(addlim, addli, loadl, storel, i.s0, m)
Y(addlm, addl, loadl, storel, i.s0, m)
Y(addwm, addl, loadw, storew, Reg32(i.s0), m)
Y(addqim, addqi, load, store, i.s0, m)
Y(andbim, andbi, loadb, storeb, i.s, m)
Y(subqim, subqi, load, store, i.s0, m)
Y(orbim, orqi, loadb, storeb, i.s0, m)
Y(orqim, orqi, load, store, i.s0, m)
Y(orwim, orqi, loadw, storew, i.s0, m)
Y(orlim, orqi, loadl, storel, i.s0, m)

#undef Y

#define Y(vasm_opc, lower_opc, movs_opc)                                \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) {             \
  if (!i.fl || (i.fl & static_cast<Vflags>(StatusFlags::NV))) {         \
    lower_impl(e.unit, b, z, [&] (Vout& v) {                            \
      auto r0 = v.makeReg(), r1 = v.makeReg();                          \
      v << movs_opc{i.s0, r0};                                          \
      v << movs_opc{i.s1, r1};                                          \
      v << lower_opc{r0, r1, i.sf, i.fl};                               \
    });                                                                 \
  }                                                                     \
}

Y(cmpb, cmpl, movsbl)
Y(cmpw, cmpl, movswl)

#undef Y

#define Y(vasm_opc, lower_opc, movs_opc)                                \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) {             \
  if (!i.fl || (i.fl & static_cast<Vflags>(StatusFlags::NV))) {         \
    lower_impl(e.unit, b, z, [&] (Vout& v) {                            \
      auto r = v.makeReg();                                             \
      v << movs_opc{i.s1, r};                                           \
      v << lower_opc{i.s0, r, i.sf, i.fl};                              \
    });                                                                 \
  }                                                                     \
}

Y(cmpbi, cmpli, movsbl)
Y(cmpwi, cmpli, movswl)

#undef Y

#define Y(vasm_opc, lower_opc, load_opc)                         \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) {      \
  lower_impl(e.unit, b, z, [&] (Vout& v) {                       \
    lowerVptr(i.s1, v);                                          \
    auto r = e.allow_vreg() ? v.makeReg() : Vreg(PhysReg(rAsm)); \
    v << load_opc{i.s1, r};                                      \
    v << lower_opc{i.s0, r, i.sf, i.fl};                         \
  });                                                            \
}

Y(cmpbim, cmpbi, loadb)
Y(cmplim, cmpli, loadl)
Y(cmpbm, cmpb, loadb)
Y(cmpwm, cmpw, loadb)
Y(cmplm, cmpl, loadl)
Y(cmpqim, cmpqi, load)
Y(cmpqm, cmpq, load)
Y(cmpwim, cmpwi, loadw)
Y(testbim, testli, loadb)
Y(testlim, testli, loadl)
Y(testqim, testqi, load)
Y(testbm, testb, loadb)
Y(testwm, testw, loadw)
Y(testlm, testl, loadl)
Y(testqm, testq, load)
Y(testwim, testli, loadw)

#undef Y

void lower(const VLS& e, cvtsi2sdm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    lowerVptr(i.s, v);
    auto r = v.makeReg();
    v << load{i.s, r};
    v << cvtsi2sd{r, i.d};
  });
}

#define Y(vasm_opc, lower_opc, load_opc, store_opc, m)            \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) {       \
  lower_impl(e.unit, b, z, [&] (Vout& v) {                        \
    lowerVptr(i.m, v);                                            \
    auto r0 = e.allow_vreg() ? v.makeReg() : Vreg(PhysReg(rAsm)); \
    auto r1 = e.allow_vreg() ? v.makeReg() : Vreg(PhysReg(rAsm)); \
    v << load_opc{i.m, r0};                                       \
    v << lower_opc{r0, r1, i.sf, i.fl};                           \
    v << store_opc{r1, i.m};                                      \
  });                                                             \
}

Y(declm, decl, loadl, storel, m)
Y(decqm, decq, load, store, m)
Y(inclm, incl, loadl, storel, m)
Y(incqm, incq, load, store, m)
Y(incwm, incw, loadw, storew, m)

#undef Y

void lower(const VLS& e, cvttsd2siq& i, Vlabel b, size_t idx) {
  lower_impl(e.unit, b, idx, [&] (Vout& v) {
    // Clear FPSR IOC flag.
    auto const tmp1 = v.makeReg();
    auto const tmp2 = v.makeReg();
    v << mrs{FPSR, tmp1};
    v << andqi{~0x01, tmp1, tmp2, v.makeReg()};
    v << msr{tmp2, FPSR};

    // Load error value.
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

void lower(const VLS& e, callm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    lowerVptr(i.target, v);

    auto const scratch = v.makeReg();

    // Load the target from memory and then call it.
    v << load{i.target, scratch};
    v << callr{scratch, i.args};
  });
}

void lower(const VLS& e, jmpm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    lowerVptr(i.target, v);

    auto const scratch = v.makeReg();

    v << load{i.target, scratch};
    v << jmpr{scratch, i.args};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, restoreripm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    lowerVptr(i.s, v);
    v << load{i.s, rlr()};
  });
}

void lower(const VLS& e, saverips& /*i*/, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Push LR twice to keep stack aligned.
    v << pushp{rlr(), rlr()};
  });
}

void lower(const VLS& e, restorerips& /*i*/, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Pop LR and the stack alignment padding.
    v << popp{PhysReg(rAsm), rlr()};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, stublogue& /*i*/, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Push both the LR and FP regardless of i.saveframe to align SP.
    v << pushp{rlr(), arm::rvmfp()};
  });
}

void lower(const VLS& e, unstublogue& /*i*/, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Pop LR and remove FP from the stack.
    v << popp{PhysReg(rAsm), rlr()};
  });
}

void lower(const VLS& e, stubret& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Pop LR and (optionally) FP.
    if (i.saveframe) {
      v << popp{arm::rvmfp(), rlr()};
    } else {
      v << popp{PhysReg(rAsm), rlr()};
    }

    v << ret{i.args};
  });
}

void lower(const VLS& e, tailcallstub& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Restore LR from native stack and adjust SP.
    v << popp{PhysReg(rAsm), rlr()};

    // Then directly jump to the target.
    v << jmpi{i.target, i.args};
  });
}

void lower(const VLS& e, tailcallstubr& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Restore LR from native stack and adjust SP.
    v << popp{PhysReg(rAsm), rlr()};

    v << jmpr{i.target, i.args};
  });
}

void lower(const VLS& e, stubunwind& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Pop the call frame.
    v << popp{PhysReg(rAsm), i.d};
  });
}

void lower(const VLS& e, stubtophp& /*i*/, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Pop the call frame
    v << lea{arm::rsp()[16], arm::rsp()};
  });
}

void lower(const VLS& e, loadstubret& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Load the LR to the destination.
    v << load{arm::rsp()[AROFF(m_savedRip)], i.d};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, phplogue& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    v << store{rlr(), i.fp[AROFF(m_savedRip)]};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, resumetc& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    // Jump to the translation target.
    v << jmpr{i.target, i.args};
  });
}

  ///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, leavetc& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    v << jmpi{i.exittc};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(const VLS& e, popm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto r = v.makeReg();
    v << pop{r};
    lowerVptr(i.d, v);
    v << store{r, i.d};
  });
}

void lower(const VLS& e, poppm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto r0 = v.makeReg();
    auto r1 = v.makeReg();
    v << popp{r0, r1};
    lowerVptr(i.d0, v);
    lowerVptr(i.d1, v);
    v << store{r0, i.d0};
    v << store{r1, i.d1};
  });
}

void lower(const VLS& e, pushm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto r = v.makeReg();
    lowerVptr(i.s, v);
    v << load{i.s, r};
    v << push{r};
  });
}

void lower(const VLS& e, pushpm& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto r0 = v.makeReg();
    auto r1 = v.makeReg();
    lowerVptr(i.s0, v);
    lowerVptr(i.s1, v);
    v << load{i.s0, r0};
    v << load{i.s1, r1};
    v << pushp{r0, r1};
  });
}

template<typename movz>
void lower_movz(const VLS& e, movz& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    v << copy{i.s, i.d};
  });
}

void lower(const VLS& e, movzbw& i, Vlabel b, size_t z) {
  lower_movz(e, i, b, z);
}

void lower(const VLS& e, movzbl& i, Vlabel b, size_t z) {
  lower_movz(e, i, b, z);
}

void lower(const VLS& e, movzwl& i, Vlabel b, size_t z) {
  lower_movz(e, i, b, z);
}

void lower(const VLS& e, movtdb& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto d = v.makeReg();
    v << copy{i.s, d};
    v << movtqb{d, i.d};
  });
}

void lower(const VLS& e, movtdq& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    v << copy{i.s, i.d};
  });
}

#define Y(vasm_opc, lower_opc, load_opc, imm, zr, sz)   \
void lower(const VLS& e, vasm_opc& i, Vlabel b, size_t z) { \
  lower_impl(e.unit, b, z, [&] (Vout& v) {                   \
    lowerVptr(i.m, v);                                  \
    if (imm.sz() == 0u) {                               \
      v << lower_opc{PhysReg(vixl::zr), i.m};           \
    } else {                                            \
      auto r = v.makeReg();                             \
      v << load_opc{imm, r};                            \
      v << lower_opc{r, i.m};                           \
    }                                                   \
  });                                                   \
}

Y(storebi, storeb, ldimmb, i.s, wzr, b)
Y(storewi, storew, ldimmw, i.s, wzr, w)
Y(storeli, storel, ldimml, i.s, wzr, l)
//storeqi only supports 32-bit immediates
Y(storeqi, store, ldimmq, Immed64(i.s.l()), wzr, q)

#undef Y

void lower(const VLS& e, cloadq& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto const scratch = v.makeReg();

    lowerVptr(i.t, v);

    v << load{i.t, scratch};
    v << cmovq{i.cc, i.sf, i.f, scratch, i.d};
  });
}

void lower(const VLS& e, loadqp& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto const scratch = v.makeReg();

    v << leap{i.s, scratch};
    v << load{scratch[0], i.d};
  });
}

void lower(const VLS& e, loadqd& i, Vlabel b, size_t z) {
  lower_impl(e.unit, b, z, [&] (Vout& v) {
    auto const scratch = v.makeReg();

    v << lead{i.s.getRaw(), scratch};
    v << load{scratch[0], i.d};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lowerForARM(Vunit& unit) {
  vasm_lower(unit, [&] (const VLS& env, Vinstr& inst, Vlabel b, size_t i) {
    switch (inst.op) {
#define O(name, ...)                      \
      case Vinstr::name:                  \
        lower(env, inst.name##_, b, i);   \
        break;

      VASM_OPCODES
#undef O
    }
  });
}

///////////////////////////////////////////////////////////////////////////////
}

void optimizeARM(Vunit& unit, const Abi& abi, bool regalloc) {
  Timer timer(Timer::vasm_optimize, unit.log_entry);

  removeTrivialNops(unit);
  optimizePhis(unit);
  fuseBranches(unit);
  optimizeJmps(unit, false, true);

  assertx(checkWidths(unit));

  simplify(unit);

  annotateSFUses(unit);
  lowerForARM(unit);

  simplify(unit);

  if (!unit.constToReg.empty()) {
    foldImms<arm::ImmFolder>(unit);
  }
  reuseImmq(unit);

  optimizeCopies(unit, abi);

  annotateSFUses(unit);
  if (unit.needsRegAlloc()) {
    removeDeadCode(unit);
    if (regalloc) {
      splitCriticalEdges(unit);

      if (RuntimeOption::EvalUseGraphColor &&
          unit.context &&
          (unit.context->kind == TransKind::Optimize ||
           unit.context->kind == TransKind::OptPrologue)) {
        allocateRegistersWithGraphColor(unit, abi);
      } else {
        allocateRegistersWithXLS(unit, abi);
      }
    }
  }

  optimizeExits(unit);
  optimizeJmps(unit, true, false);
}

void emitARM(Vunit& unit, Vtext& text, CGMeta& fixups,
             AsmInfo* asmInfo) {
  vasm_emit<Vgen>(unit, text, fixups, asmInfo);
}

///////////////////////////////////////////////////////////////////////////////
}
