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

#include "hphp/runtime/vm/jit/vasm-emit.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-x64.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-internal.h"
#include "hphp/runtime/vm/jit/vasm-lower.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-prof.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <algorithm>
#include <tuple>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

using namespace reg;
using namespace x64;

namespace x64 { struct ImmFolder; }

namespace {
///////////////////////////////////////////////////////////////////////////////

static_assert(folly::kIsLittleEndian,
  "Code contains little-endian specific optimizations.");

template<class X64Asm>
struct Vgen {
  explicit Vgen(Venv& env)
    : env(env)
    , a(*env.cb)
    , current(env.current)
    , next(env.next)
    , jmps(env.jmps)
    , jccs(env.jccs)
    , catches(env.catches)
  {}

  static void emitVeneers(Venv& env) {}
  static void handleLiterals(Venv& env) {}
  static void patch(Venv& env);
  static void pad(CodeBlock& cb);

  /////////////////////////////////////////////////////////////////////////////

  template<class Inst> void emit(const Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }

  // intrinsics
  void emit(const copy& i);
  void emit(const copy2& i);
  void emit(const debugtrap& /*i*/) { a.int3(); }
  void emit(const fallthru& /*i*/) {}
  void emit(const ldimmb& i);
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const load& i);
  void emit(const store& i);
  void emit(const mcprep& i);

  // native function abi
  void emit(const call& i);
  void emit(const callm& i) { a.call(i.target); }
  void emit(const callr& i) { a.call(i.target); }
  void emit(const calls& i);
  void emit(const ret& /*i*/) { a.ret(); }

  // stub function abi
  void emit(const stubret& i);
  void emit(const callstub& i);
  void emit(const callfaststub& i);
  void emit(const tailcallstub& i);

  // php function abi
  void emit(const phpret& i);
  void emit(const tailcallphp& i);
  void emit(const callunpack& i);
  void emit(const contenter& i);

  // vm entry abi
  void emit(const inittc& /*i*/) {}
  void emit(const calltc&);
  void emit(const leavetc&) { a.ret(); }

  // exceptions
  void emit(const landingpad& /*i*/) {}
  void emit(const nothrow& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);

  // instructions
  void emit(absdbl i) { unary(i); a.psllq(1, i.d); a.psrlq(1, i.d); }
  void emit(andb i) { commuteSF(i); a.andb(i.s0, i.d); }
  void emit(andbi i) { binary(i); a.andb(i.s0, i.d); }
  void emit(const andbim& i) { a.andb(i.s, i.m); }
  void emit(andw i) { commuteSF(i); a.andw(i.s0, i.d); }
  void emit(andwi i) { binary(i); a.andw(i.s0, i.d); }
  void emit(andl i) { commuteSF(i); a.andl(i.s0, i.d); }
  void emit(andli i) { binary(i); a.andl(i.s0, i.d); }
  void emit(andq i) { commuteSF(i); a.andq(i.s0, i.d); }
  void emit(andqi i);
  void emit(const addwm& i) { a.addw(i.s0, i.m); }
  void emit(addli i) { binary(i); a.addl(i.s0, i.d); }
  void emit(const addlm& i) { a.addl(i.s0, i.m); }
  void emit(const addlim& i);
  void emit(addq i) { commuteSF(i); a.addq(i.s0, i.d); }
  void emit(addqi i) { binary(i); a.addq(i.s0, i.d); }
  void emit(const addqmr& i) { binary(i); a.addq(i.m, i.d); }
  void emit(const addqrm& i) { a.addq(i.s1, i.m); }
  void emit(const addqim& i);
  void emit(addsd i) { commute(i); a.addsd(i.s0, i.d); }
  void emit(const cloadq& i);
  template<class cmov> void emit_cmov(const cmov& i);
  void emit(const cmovb& i) { emit_cmov(i); }
  void emit(const cmovw& i) { emit_cmov(i); }
  void emit(const cmovl& i) { emit_cmov(i); }
  void emit(const cmovq& i) { emit_cmov(i); }
  void emit(const cmpb& i) { a.cmpb(i.s0, i.s1); }
  void emit(const cmpbi& i) { a.cmpb(i.s0, i.s1); }
  void emit(const cmpbim& i) { a.cmpb(i.s0, i.s1); }
  void emit(const cmpbm& i) { a.cmpb(i.s0, i.s1); }
  void emit(const cmpw& i) { a.cmpw(i.s0, i.s1); }
  void emit(const cmpwi& i) { a.cmpw(i.s0, i.s1); }
  void emit(const cmpwim& i) { a.cmpw(i.s0, i.s1); }
  void emit(const cmpwm& i) { a.cmpw(i.s0, i.s1); }
  void emit(const cmpl& i) { a.cmpl(i.s0, i.s1); }
  void emit(const cmpli& i) { a.cmpl(i.s0, i.s1); }
  void emit(const cmplim& i) { a.cmpl(i.s0, i.s1); }
  void emit(const cmplm& i) { a.cmpl(i.s0, i.s1); }
  void emit(const cmpq& i) { a.cmpq(i.s0, i.s1); }
  void emit(const cmpqi& i) { a.cmpq(i.s0, i.s1); }
  void emit(const cmpqim& i) { a.cmpq(i.s0, i.s1); }
  void emit(const cmpqm& i) { a.cmpq(i.s0, i.s1); }
  void emit(cmpsd i) { noncommute(i); a.cmpsd(i.s0, i.d, i.pred); }
  void emit(const cqo& /*i*/) { a.cqo(); }
  void emit(const cvttsd2siq& i) { a.cvttsd2siq(i.s, i.d); }
  void emit(const cvtsi2sd& i);
  void emit(const cvtsi2sdm& i);
  void emit(decl i) { unary(i); a.decl(i.d); }
  void emit(const declm& i) { a.decl(i.m); }
  void emit(decq i) { unary(i); a.decq(i.d); }
  void emit(const decqm& i) { a.decq(i.m); }
  void emit(const decqmlock& i) { a.decqlock(i.m); }
  void emit(divsd i) { noncommute(i); a.divsd(i.s0, i.d); }
  void emit(imul i) { commuteSF(i); a.imul(i.s0, i.d); }
  void emit(const idiv& i) { a.idiv(i.s); }
  void emit(incl i) { unary(i); a.incl(i.d); }
  void emit(const inclm& i) { a.incl(i.m); }
  void emit(incq i) { unary(i); a.incq(i.d); }
  void emit(const incqm& i) { a.incq(i.m); }
  void emit(const incwm& i) { a.incw(i.m); }
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const jmpr& i) { a.jmp(i.target); }
  void emit(const jmpm& i) { a.jmp(i.target); }
  void emit(const jmpi& i);
  void emit(const lea& i);
  void emit(const leap& i) { a.lea(i.s, i.d); }
  void emit(const leav& i);
  void emit(const lead& i) { a.lea(rip[(intptr_t)i.s.get()], i.d); }
  void emit(const loadups& i) { a.movups(i.s, i.d); }
  void emit(const loadtqb& i) { a.loadb(i.s, i.d); }
  void emit(const loadb& i) { a.loadb(i.s, i.d); }
  void emit(const loadw& i) { a.loadw(i.s, i.d); }
  void emit(const loadtql& i) { a.loadl(i.s, i.d); }
  void emit(const loadl& i) { a.loadl(i.s, i.d); }
  void emit(const loadqp& i) { a.loadq(i.s, i.d); }
  void emit(const loadqd& i) { a.loadq(rip[(intptr_t)i.s.get()], i.d); }
  void emit(const loadsd& i) { a.movsd(i.s, i.d); }
  void emit(const loadzbl& i) { a.loadzbl(i.s, i.d); }
  void emit(const loadzbq& i) { a.loadzbl(i.s, Reg32(i.d)); }
  void emit(const loadsbq& i) { a.loadsbq(i.s, i.d); }
  void emit(const loadzlq& i) { a.loadl(i.s, Reg32(i.d)); }
  void emit(const movb& i) { a.movb(i.s, i.d); }
  void emit(const movl& i) { a.movl(i.s, i.d); }
  void emit(const movzbw& i) { a.movzbl(i.s, Reg32(i.d)); }
  void emit(const movzbl& i) { a.movzbl(i.s, i.d); }
  void emit(const movzbq& i) { a.movzbl(i.s, Reg32(i.d)); }
  void emit(const movzwl& i) { a.movzwl(i.s, i.d); }
  void emit(const movzwq& i) { a.movzwl(i.s, Reg32(i.d)); }
  void emit(const movzlq& i) { a.movl(i.s, Reg32(i.d)); }
  void emit(const movsbq& i) { a.movsbq(i.s, i.d); }
  void emit(mulsd i) { commute(i); a.mulsd(i.s0, i.d); }
  void emit(neg i) { unary(i); a.neg(i.d); }
  void emit(const nop& /*i*/) { a.nop(); }
  void emit(not i) { unary(i); a.not(i.d); }
  void emit(notb i) { unary(i); a.notb(i.d); }
  void emit(orbi i) { binary(i); a.orb(i.s0, i.d); }
  void emit(const orbim& i) { a.orb(i.s0, i.m); }
  void emit(const orwim& i) { a.orw(i.s0, i.m); }
  void emit(const orlim& i) { a.orl(i.s0, i.m); }
  void emit(orq i) { commuteSF(i); a.orq(i.s0, i.d); }
  void emit(orwi i) { binary(i); a.orw(i.s0, i.d); }
  void emit(orli i) { binary(i); a.orl(i.s0, i.d); }
  void emit(orqi i) { binary(i); a.orq(i.s0, i.d); }
  void emit(const orqim& i) { a.orq(i.s0, i.m); }
  void emit(const pop& i) { a.pop(i.d); }
  void emit(const popm& i) { a.pop(i.d); }
  void emit(const popf& i) { assertx(i.d == RegSF{0}); a.popf(); }
  void emit(const push& i) { a.push(i.s); }
  void emit(const pushm& i) { a.push(i.s); }
  void emit(const pushf& i) { assertx(i.s == RegSF{0}); a.pushf(); }
  void emit(const roundsd& i) { a.roundsd(i.dir, i.s, i.d); }
  void emit(const sarq& i) { unary(i); a.sarq(i.d); }
  void emit(sarqi i) { binary(i); a.sarq(i.s0, i.d); }
  void emit(const setcc& i) { a.setcc(i.cc, i.d); }
  void emit(shlli i) { binary(i); a.shll(i.s0, i.d); }
  void emit(shlq i) { unary(i); a.shlq(i.d); }
  void emit(shrq i) { unary(i); a.shrq(i.d); }
  void emit(shlqi i) { binary(i); a.shlq(i.s0, i.d); }
  void emit(shrli i) { binary(i); a.shrl(i.s0, i.d); }
  void emit(shrqi i) { binary(i); a.shrq(i.s0, i.d); }
  void emit(const sqrtsd& i) { a.sqrtsd(i.s, i.d); }
  void emit(const storeups& i) { a.movups(i.s, i.m); }
  void emit(const storeb& i) { a.storeb(i.s, i.m); }
  void emit(const storebi& i);
  void emit(const storel& i) { a.storel(i.s, i.m); }
  void emit(const storeli& i) { a.storel(i.s, i.m); }
  void emit(const storeqi& i);
  void emit(const storesd& i) { a.movsd(i.s, i.m); }
  void emit(const storew& i) { a.storew(i.s, i.m); }
  void emit(const storewi& i) { a.storew(i.s, i.m); }
  void emit(subl i) { noncommute(i); a.subl(i.s0, i.d); }
  void emit(subli i) { binary(i); a.subl(i.s0, i.d); }
  void emit(subq i) { noncommute(i); a.subq(i.s0, i.d); }
  void emit(subqi i) { binary(i); a.subq(i.s0, i.d); }
  void emit(subsd i) { noncommute(i); a.subsd(i.s0, i.d); }
  void emit(const testb& i) { a.testb(i.s0, i.s1); }
  void emit(const testbi& i) { a.testb(i.s0, i.s1); }
  void emit(const testbm& i) { a.testb(i.s0, i.s1); }
  void emit(const testbim& i) { a.testb(i.s0, i.s1); }
  void emit(const testw& i) { a.testw(i.s0, i.s1); }
  void emit(const testwi& i);
  void emit(const testwm& i) { a.testw(i.s0, i.s1); }
  void emit(const testwim& i);
  void emit(const testl& i) { a.testl(i.s0, i.s1); }
  void emit(const testli& i);
  void emit(const testlm& i) { a.testl(i.s0, i.s1); }
  void emit(const testlim& i);
  void emit(const testq& i) { a.testq(i.s0, i.s1); }
  void emit(const testqi& i);
  void emit(const testqm& i) { a.testq(i.s0, i.s1); }
  void emit(const testqim& i);
  void emit(const trap& i);
  void emit(const ucomisd& i) { a.ucomisd(i.s0, i.s1); }
  void emit(unpcklpd i) { noncommute(i); a.unpcklpd(i.s0, i.d); }
  void emit(xorb i) { commuteSF(i); a.xorb(i.s0, i.d); }
  void emit(xorbi i) { binary(i); a.xorb(i.s0, i.d); }
  void emit(xorl i) { commuteSF(i); a.xorl(i.s0, i.d); }
  void emit(xorq i);
  void emit(xorqi i) { binary(i); a.xorq(i.s0, i.d); }
  void emit(const conjure& /*i*/) { always_assert(false); }
  void emit(const conjureuse& /*i*/) { always_assert(false); }

  void emit_nop() {
    emit(lea{rax[8], rax});
    emit(lea{rax[-8], rax});
  }

private:
  // helpers
  void prep(Reg8 s, Reg8 d) { if (s != d) a.movb(s, d); }
  void prep(Reg16 s, Reg16 d) { if (s != d) a.movw(s, d); }
  void prep(Reg32 s, Reg32 d) { if (s != d) a.movl(s, d); }
  void prep(Reg64 s, Reg64 d) { if (s != d) a.movq(s, d); }
  void prep(RegXMM s, RegXMM d) { if (s != d) a.movdqa(s, d); }
  void emit_simd_imm(int64_t, Vreg);

  template<class Inst> void unary(Inst& i) { prep(i.s, i.d); }
  template<class Inst> void binary(Inst& i) { prep(i.s1, i.d); }

  template<class Inst> void commuteSF(Inst&);
  template<class Inst> void commute(Inst&);
  template<class Inst> void noncommute(Inst&);

  CodeBlock& frozen() { return env.text.frozen().code; }

private:
  Venv& env;
  X64Asm a;

  const Vlabel current;
  const Vlabel next;
  jit::vector<Venv::LabelPatch>& jmps;
  jit::vector<Venv::LabelPatch>& jccs;
  jit::vector<Venv::LabelPatch>& catches;
};

///////////////////////////////////////////////////////////////////////////////
/*
 * Prepare a binary op that is not commutative.
 *
 * s0 must be a different register than s1 so we don't clobber it.
 */
template<class X64Asm>
template<class Inst> void Vgen<X64Asm>::noncommute(Inst& i) {
  assertx(i.s1 == i.d || i.s0 != i.d); // do not clobber s0
  binary(i);
}

/*
 * Prepare a binary op that is commutative.
 *
 * Swap operands if the dest is s0.
 */
template<class X64Asm>
template<class Inst> void Vgen<X64Asm>::commuteSF(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d, i.sf};
  } else {
    binary(i);
  }
}

template<class X64Asm>
template<class Inst> void Vgen<X64Asm>::commute(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d};
  } else {
    binary(i);
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns true iff the status flags necessary to take a j<a> imply that a j<b>
 * will also be taken.
 */
bool ccImplies(ConditionCode a, ConditionCode b) {
  if (a == b) return true;

  switch (a) {
    case CC_None:
    case CC_O:  case CC_NO:
    case CC_AE: case CC_BE:
    case CC_NE:
    case CC_S:  case CC_NS:
    case CC_P:  case CC_NP:
    case CC_GE: case CC_LE:
      return false;

    case CC_B: return b == CC_BE;
    case CC_E: return b == CC_BE || b == CC_LE;
    case CC_A: return b == CC_AE || b == CC_NE;
    case CC_L: return b == CC_LE;
    case CC_G: return b == CC_NE || b == CC_GE;
  }
  always_assert(false);
}

static CodeAddress toReal(Venv& env, CodeAddress a) {
  if (env.text.main().code.contains(a)) {
    return env.text.main().code.toDestAddress(a);
  }
  if (env.text.cold().code.contains(a)) {
    return env.text.cold().code.toDestAddress(a);
  }
  if (env.text.frozen().code.contains(a)) {
    return env.text.frozen().code.toDestAddress(a);
  }
  return a;
}

/*
 * When two jccs go to the same destination, the cc of the first is compatible
 * with the cc of the second, and they're within a one-byte offset of each
 * other, retarget the first to jump to the second. This will allow the
 * relocator to shrink the first one, and the extra jmp shouldn't matter since
 * we try to only do this to rarely taken jumps.
 */
void retargetJumps(Venv& env,
                   const jit::hash_map<TCA, jit::vector<TCA>>& jccs) {
  jit::hash_set<TCA> retargeted;
  for (auto& pair : jccs) {
    auto const& jmps = pair.second;
    if (jmps.size() < 2) continue;

    for (size_t i = 0; i < jmps.size(); ++i) {
      DecodedInstruction di(toReal(env, jmps[i]), jmps[i]);
      // Don't bother if the jump is already a short jump.
      if (di.size() != 6) continue;

      for (size_t j = jmps.size() - 1; j > i; --j) {
        auto const delta = jmps[j] - jmps[i] + 2;
        // Backwards jumps are probably not guards, and don't retarget to a
        // dest that's more than a one-byte offset away.
        if (delta < 0 || !deltaFits(delta, sz::byte)) continue;

        DecodedInstruction dj(toReal(env, jmps[j]), jmps[j]);
        if (!ccImplies(di.jccCondCode(), dj.jccCondCode())) continue;

        di.setPicAddress(jmps[j]);
        retargeted.insert(jmps[i]);

        // We might've converted a smashable jump to a regular in-unit jump, so
        // remove any smashable alignments.
        auto range = env.meta.alignments.equal_range(jmps[i]);
        while (range.first != range.second) {
          auto iter = range.first;
          ++range.first;

          auto& align = iter->second;
          if (align.first == Alignment::SmashJcc &&
              align.second == AlignContext::Live) {
            env.meta.alignments.erase(iter);
          }
        }

        break;
      }
    }
  }

  // Finally, remove any retargeted jmps from inProgressTailJumps.
  if (!retargeted.empty()) {
    GrowableVector<IncomingBranch> newTailJumps;
    for (auto& jmp : env.meta.inProgressTailJumps) {
      if (retargeted.count(jmp.toSmash()) == 0) {
        newTailJumps.push_back(jmp);
      }
    }
    env.meta.inProgressTailJumps.swap(newTailJumps);
  }
  // If the retarged jumps were smashable, now they aren't anymore, so remove
  // them from smashableJumpData.
  for (auto jmp : retargeted) {
    if (env.meta.smashableJumpData.erase(jmp) > 0) {
      FTRACE(3, "retargetJumps: removed {} from smashableJumpData\n", jmp);
    }
  }
}

template<class X64Asm>
void Vgen<X64Asm>::patch(Venv& env) {
  for (auto const& p : env.jmps) {
    assertx(env.addrs[p.target]);
    X64Asm::patchJmp(toReal(env, p.instr), p.instr, env.addrs[p.target]);
  }

  auto const optLevel = RuntimeOption::EvalJitRetargetJumps;
  jit::hash_map<TCA, jit::vector<TCA>> jccs;
  for (auto const& p : env.jccs) {
    assertx(env.addrs[p.target]);
    X64Asm::patchJcc(toReal(env, p.instr), p.instr, env.addrs[p.target]);
    if (optLevel >= 2 ||
        (optLevel == 1 && p.target >= env.unit.blocks.size())) {
      jccs[env.addrs[p.target]].emplace_back(p.instr);
    }
  }

  if (!jccs.empty()) retargetJumps(env, jccs);

  for (auto const& p : env.leas) {
    assertx(env.vaddrs[p.target]);
    DecodedInstruction di(toReal(env, p.instr), p.instr);
    assertx(di.hasPicOffset());
    di.setPicAddress(env.vaddrs[p.target]);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::pad(CodeBlock& cb) {
  X64Asm a { cb };
  a.pad();
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const copy& i) {
  if (i.s == i.d) return;
  if (i.s.isGP()) {
    if (i.d.isGP()) {                 // GP => GP
      a.movq(i.s, i.d);
    } else {                             // GP => XMM
      assertx(i.d.isSIMD());
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      a.movq_rx(i.s, i.d);
    }
  } else {
    if (i.d.isGP()) {                 // XMM => GP
      a.movq_xr(i.s, i.d);
    } else {                             // XMM => XMM
      assertx(i.d.isSIMD());
      // This copies all 128 bits in XMM,
      // thus avoiding partial register stalls
      a.movdqa(i.s, i.d);
    }
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const copy2& i) {
  assertx(i.s0.isValid() && i.s1.isValid() && i.d0.isValid() && i.d1.isValid());
  auto s0 = i.s0, s1 = i.s1, d0 = i.d0, d1 = i.d1;
  assertx(d0 != d1);
  if (d0 == s1) {
    if (d1 == s0) {
      a.xchgq(d0, d1);
    } else {
      // could do this in a simplify pass
      if (s1 != d1) a.movq(s1, d1); // save s1 first; d1 != s0
      if (s0 != d0) a.movq(s0, d0);
    }
  } else {
    // could do this in a simplify pass
    if (s0 != d0) a.movq(s0, d0);
    if (s1 != d1) a.movq(s1, d1);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit_simd_imm(int64_t val, Vreg d) {
  if (val == 0) {
    a.pxor(d, d); // does not modify flags
  } else {
    auto addr = alloc_literal(env, val);
    a.movsd(rip[(intptr_t)addr], d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const ldimmb& i) {
  // ldimmb is for Vconst::Byte, which is treated as unsigned uint8_t
  auto val = i.s.ub();
  if (i.d.isGP()) {
    Vreg8 d8 = i.d;
    a.movb(static_cast<int8_t>(val), d8);
  } else {
    emit_simd_imm(val, i.d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const ldimml& i) {
  // ldimml is for Vconst::Long, which is treated as unsigned uint32_t
  auto val = i.s.l();
  if (i.d.isGP()) {
    Vreg32 d32 = i.d;
    a.movl(val, d32);
  } else {
    emit_simd_imm(uint32_t(val), i.d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const ldimmq& i) {
  auto val = i.s.q();
  if (i.d.isGP()) {
    if (val == 0) {
      Vreg32 d32 = i.d;
      a.movl(0, d32); // because emitImmReg tries the xor optimization
    } else {
      a.emitImmReg(i.s, i.d);
    }
  } else {
    emit_simd_imm(val, i.d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const load& i) {
  auto mref = i.s.mr();
  a.prefix(mref);
  if (i.d.isGP()) {
    a.loadq(mref, i.d);
  } else {
    assertx(i.d.isSIMD());
    a.movsd(mref, i.d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const store& i) {
  if (i.s.isGP()) {
    a.storeq(i.s, i.d);
  } else {
    assertx(i.s.isSIMD());
    a.movsd(i.s, i.d);
  }
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const mcprep& i) {
  /*
   * Initially, we set the cache to hold (addr << 1) | 1 (where `addr' is the
   * address of the movq) so that we can find the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * MethodCache::handleStaticCall can tell it's not been smashed yet
   */
  auto const mov_addr = emitSmashableMovq(a.code(), env.meta, 0, r64(i.d));
  auto const imm = reinterpret_cast<uint64_t>(mov_addr);
  smashMovq(a.toDestAddress(mov_addr), (imm << 1) | 1);

  env.meta.addressImmediates.insert(reinterpret_cast<TCA>(~imm));
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const call& i) {
  if (a.jmpDeltaFits(i.target)) {
    a.call(i.target);
  } else {
    // can't do a near call; store address in data section.
    // call by loading the address using rip-relative addressing.  This
    // assumes the data section is near the current code section.  Since
    // this sequence is directly in-line, rip-relative like this is
    // more compact than loading a 64-bit immediate.
    auto addr = alloc_literal(env, (uint64_t)i.target);
    a.call(rip[(intptr_t)addr]);
  }
  if (i.watch) {
    *i.watch = a.frontier();
    env.meta.watchpoints.push_back(i.watch);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const calls& i) {
  emitSmashableCall(a.code(), env.meta, i.target);
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const stubret& i) {
  if (i.saveframe) {
    a.pop(rvmfp());
  } else {
    a.addq(8, reg::rsp);
  }
  a.ret();
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const tailcallstub& i) {
  a.addq(8, reg::rsp);
  emit(jmpi{i.target, i.args});
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const phpret& i) {
  a.push(i.fp[AROFF(m_savedRip)]);
  if (!i.noframe) {
    a.loadq(i.fp[AROFF(m_sfp)], i.d);
  }
  a.ret();
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const tailcallphp& i) {
  emit(pushm{i.fp[AROFF(m_savedRip)]});
  emit(jmpr{i.target, i.args});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const callunpack& i) {
  emit(call{i.target, i.args});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const contenter& i) {
  Label Stub, End;
  Reg64 fp = i.fp, target = i.target;
  a.jmp8(End);

  asm_label(a, Stub);
  a.pop(fp[AROFF(m_savedRip)]);
  a.jmp(target);

  asm_label(a, End);
  a.call(Stub);
  // m_savedRip will point here.
  emit(unwind{{i.targets[0], i.targets[1]}});
}

///////////////////////////////////////////////////////////////////////////////
template<class X64Asm>
void Vgen<X64Asm>::emit(const calltc& i) {
  a.push(i.exittc);
  a.push(i.fp[AROFF(m_savedRip)]);

  Label stub;
  a.call(stub);

  asm_label(a, stub);
  assertx(!i.args.contains(reg::rax));
  a.pop(reg::rax);  // unused
  a.jmp(i.target);
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(const nothrow& /*i*/) {
  env.meta.catches.emplace_back(a.frontier(), nullptr);
  env.record_inline_stack(a.frontier());
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a.frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  env.meta.fixups.emplace_back(a.frontier(), i.fix);
  env.record_inline_stack(a.frontier());
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const unwind& i) {
  catches.push_back({a.frontier(), i.targets[1]});
  env.record_inline_stack(a.frontier());
  emit(jmp{i.targets[0]});
}

///////////////////////////////////////////////////////////////////////////////

template<class X64Asm>
void Vgen<X64Asm>::emit(andqi i) {
  if (magFits(i.s0.q(), sz::dword)) {
    emit(andli{int32_t(i.s0.q()), Reg32(i.s1), Reg32(i.d), i.sf});
    return;
  }

  binary(i);
  a.andq(i.s0, i.d);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const addlim& i) {
  auto mref = i.m.mr();
  a.prefix(mref).addl(i.s0, mref);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const addqim& i) {
  auto mref = i.m.mr();
  a.prefix(mref).addq(i.s0, mref);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const cloadq& i) {
  auto m = i.t;
  always_assert(!m.index.isValid()); // not supported, but could be later.
  if (i.f != i.d) {
    if (i.d == m.base) {
      // We can't move f over d or we'll clobber the Vptr we need to load from.
      // Since cload does the load unconditionally anyway, we can just load and
      // cmov.
      a.loadq(i.t, i.d);
      a.cmov_reg64_reg64(ccNegate(i.cc), i.f, i.d);
      return;
    }
    a.movq(i.f, i.d);
  }
  a.cload_reg64_disp_reg64(i.cc, m.base, m.disp, i.d);
}

// add s0 s1 d => mov s1->d; d += s0
// cmov cc s d => if cc { mov s->d }
template<class X64Asm>
template<class cmov>
void Vgen<X64Asm>::emit_cmov(const cmov& i) {
  if (i.f != i.d && i.t == i.d) {
    // negate the condition and swap t/f operands so we dont clobber i.t
    return emit(cmov{ccNegate(i.cc), i.sf, i.t, i.f, i.d});
  } else {
    prep(i.f, i.d);
  }
  a.cmov_reg64_reg64(i.cc, r64(i.t), r64(i.d));
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const cvtsi2sd& i) {
  a.pxor(i.d, i.d);
  a.cvtsi2sd(i.s, i.d);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const cvtsi2sdm& i) {
  a.pxor(i.d, i.d);
  a.cvtsi2sd(i.s, i.d);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a.frontier(), taken});
    a.jcc(i.cc, a.frontier());
  }
  emit(jmp{i.targets[0]});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const jcci& i) {
  a.jcc(i.cc, i.taken);
  emit(jmp{i.target});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a.frontier(), i.target});
  a.jmp(a.frontier());
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const jmpi& i) {
  if (a.jmpDeltaFits(i.target)) {
    a.jmp(i.target);
  } else {
    // can't do a near jmp - use rip-relative addressing
    auto addr = alloc_literal(env, (uint64_t)i.target);
    a.jmp(rip[(intptr_t)addr]);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const lea& i) {
  // could do this in a simplify pass
  if (i.s.disp == 0 && i.s.base.isValid() && !i.s.index.isValid()) {
    emit(copy{i.s.base, i.d});
  } else {
    a.lea(i.s, i.d);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const leav& i) {
  auto const addr = a.frontier();
  emit(leap{reg::rip[0xdeadbeef], i.d});
  env.leas.push_back({addr, i.s});
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const storebi& i) {
  auto mref = i.m.mr();
  a.prefix(mref).storeb(i.s, mref);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const storeqi& i) {
  auto mref = i.m.mr();
  a.prefix(mref).storeq(i.s, mref);
}

template<class VgenImpl, typename Inst>
bool testimHelper(VgenImpl& env, const Inst& i, uint64_t mask) {
  // If there's only 1 byte of meaningful bits in the mask, we can adjust the
  // pointer offset and use testbim instead.
  int off = 0;
  while (mask > 0xff && !(mask & 0xff)) {
    off++;
    mask >>= 8;
  }

  if (mask > 0xff) return false;

  env.emit(testbim{int8_t(mask), i.s1 + off, i.sf});
  return true;
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const testwi& i) {
  if (i.s0.w() == -1) {
    return emit(testw{i.s1, i.s1, i.sf});
  }
  a.testw(i.s0, i.s1);
}

template<class X64Asm>
void Vgen<X64Asm>::Vgen::emit(const testwim& i) {
  if (testimHelper(*this, i, i.s0.w())) return;
  a.testw(i.s0, i.s1);
}

template<class X64Asm>
void Vgen<X64Asm>::Vgen::emit(const testlim& i) {
  if (testimHelper(*this, i, i.s0.l())) return;
  a.testl(i.s0, i.s1);
}

template<class X64Asm>
void Vgen<X64Asm>::Vgen::emit(const testli& i) {
  if (i.s0.l() == -1) {
    return emit(testl{i.s1, i.s1, i.sf});
  }
  a.testl(i.s0, i.s1);
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const testqi& i) {
  auto const imm = i.s0.q();
  if (magFits(imm, sz::byte)) {
    a.testb(int8_t(imm), rbyte(i.s1));
  } else if (magFits(imm, sz::dword)) {
    emit(testli{int32_t(imm), Reg32(i.s1), i.sf});
  } else if (imm == -1) {
    emit(testq{i.s1, i.s1, i.sf});
  } else {
    a.testq(i.s0, i.s1);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const testqim& i) {
  if (testimHelper(*this, i, i.s0.q())) return;
  if (magFits(i.s0.q(), sz::dword)) {
    // For an unsigned 32 bit immediate, we can get the same results
    // by emitting a testlim.
    emit(testlim{int32_t(i.s0.q()), i.s1, i.sf});
  } else {
    a.testq(i.s0, i.s1);
  }
}

template<class X64Asm>
void Vgen<X64Asm>::emit(const trap& i) {
  env.meta.trapReasons.emplace_back(a.frontier(), i.reason);
  a.ud2();
}

template<class X64Asm>
void Vgen<X64Asm>::emit(xorq i) {
  if (i.s0 == i.s1) {
    // 32-bit xor{s, s, d} zeroes the upper bits of `d'.
    return emit(xorl{r32(i.s0), r32(i.s1), r32(i.d), i.sf});
  }
  commuteSF(i);
  a.xorq(i.s0, i.d);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Lower>
void lower_impl(Vunit& unit, Vlabel b, size_t i, Lower lower) {
  vmodify(unit, b, i, [&] (Vout& v) { lower(v); return 1; });
}

template <typename Inst>
void lower(Vunit& /*unit*/, Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {}

///////////////////////////////////////////////////////////////////////////////

void lower(Vunit& unit, popp& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << pop{inst.d0};
    v << pop{inst.d1};
  });
}

void lower(Vunit& unit, poppm& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << popm{inst.d0};
    v << popm{inst.d1};
  });
}

void lower(Vunit& unit, pushp& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << push{inst.s0};
    v << push{inst.s1};
  });
}

void lower(Vunit& unit, pushpm& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << pushm{inst.s0};
    v << pushm{inst.s1};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(Vunit& unit, stublogue& inst, Vlabel b, size_t i) {
  if (inst.saveframe) {
    unit.blocks[b].code[i] = push{rvmfp()};
  } else {
    unit.blocks[b].code[i] = lea{reg::rsp[-8], reg::rsp};
  }
}

void lower(Vunit& unit, stubunwind& /*inst*/, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = lea{reg::rsp[16], reg::rsp};
}

void lower(Vunit& unit, stubtophp& /*inst*/, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = lea{reg::rsp[16], reg::rsp};
}

void lower(Vunit& unit, loadstubret& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = load{reg::rsp[8], inst.d};
}

void lower(Vunit& unit, phplogue& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = popm{inst.fp[AROFF(m_savedRip)]};
}

void lower(Vunit& unit, resumetc& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << callr{inst.target, inst.args};
    v << jmpi{inst.exittc};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(Vunit& unit, sar& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << copy{inst.s0, rcx};
    v << sarq{inst.s1, inst.d, inst.sf};
  });
}

void lower(Vunit& unit, shl& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << copy{inst.s0, rcx};
    v << shlq{inst.s1, inst.d, inst.sf};
  });
}

void lower(Vunit& unit, shr& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << copy{inst.s0, rcx};
    v << shrq{inst.s1, inst.d, inst.sf};
  });
}

void lower(Vunit& unit, srem& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << copy{inst.s0, rax};
    v << cqo{};                      // sign-extend rax => rdx:rax
    v << idiv{inst.s1, v.makeReg()}; // rdx:rax/divisor => quot:rax, rem:rdx
    v << copy{rdx, inst.d};
  });
}

void lower(Vunit& unit, divint& inst, Vlabel b, size_t i) {
  lower_impl(unit, b, i, [&] (Vout& v) {
    v << copy{inst.s0, rax};
    v << cqo{};                      // sign-extend rax => rdx:rax
    v << idiv{inst.s1, v.makeReg()}; // rdx:rax/divisor => quot:rax, rem:rdx
    v << copy{rax, inst.d};
  });
}

///////////////////////////////////////////////////////////////////////////////

void lower(Vunit& unit, movtqb& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, inst.d};
}
void lower(Vunit& unit, movtdb& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, inst.d};
}
void lower(Vunit& unit, movtdq& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, inst.d};
}
void lower(Vunit& unit, movtqw& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, inst.d};
}
void lower(Vunit& unit, movtql& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, inst.d};
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Lower a few abstractions to facilitate straightforward x64 codegen.
 */
void lowerForX64(Vunit& unit) {
  vasm_lower(unit, [&](const VLS& /*env*/, Vinstr& inst, Vlabel b, size_t i) {
    switch (inst.op) {
#define O(name, ...)                      \
      case Vinstr::name:                  \
        lower(unit, inst.name##_, b, i);  \
        break;

      VASM_OPCODES
#undef O
    }
  });
}

///////////////////////////////////////////////////////////////////////////////
}

void optimizeX64(Vunit& unit, const Abi& abi, bool regalloc) {
  Timer timer(Timer::vasm_optimize, unit.log_entry);

  auto const doPass = [&] (const char* name, auto fun) {
    rqtrace::EventGuard trace{name};
    fun(unit);
  };

  doPass("VOPT_NOP",    removeTrivialNops);
  doPass("VOPT_PHI",    optimizePhis);
  doPass("VOPT_BRANCH", fuseBranches);
  doPass("VOPT_JMP",    [] (Vunit& u) { optimizeJmps(u); });
  doPass("VOPT_EXIT",   [] (Vunit& u) { optimizeExits(u); });

  assertx(checkWidths(unit));

  if (unit.context && !isProfiling(unit.context->kind) && abi.canSpill &&
      RuntimeOption::EvalProfBranchSampleFreq > 0) {
    // Even when branch profiling is on, we still only want to profile
    // non-profiling translations of PHP functions.  We also require that we
    // can spill, so that we can generate arbitrary profiling code, and also to
    // ensure we don't profile unique stubs and such.
    doPass("VOPT_PROF_BRANCH", profile_branches);
  }

  doPass("VOPT_X64",      lowerForX64);
  doPass("VOPT_SIMPLIFY", simplify);
  doPass("VOPT_X64",      lowerForX64);

  if (!unit.constToReg.empty()) {
    doPass("VOPT_FOLD_IMM", foldImms<x64::ImmFolder>);
  }

  doPass("VOPT_COPY", [&] (Vunit& u) { optimizeCopies(u, abi); });

  if (unit.needsRegAlloc()) {
    doPass("VOPT_DCE", [] (Vunit& u) { removeDeadCode(u); });
    doPass("VOPT_JMP", [] (Vunit& u) { optimizeJmps(u); });
    doPass("VOPT_DCE", [] (Vunit& u) { removeDeadCode(u); });
    if (regalloc) {
      if (RuntimeOption::EvalUseGraphColor &&
          unit.context &&
          (unit.context->kind == TransKind::Optimize ||
           unit.context->kind == TransKind::OptPrologue)) {
        rqtrace::EventGuard trace{"VOPT_GRAPH_COLOR"};
        allocateRegistersWithGraphColor(unit, abi);
      } else {
        rqtrace::EventGuard trace{"VOPT_XLS"};
        allocateRegistersWithXLS(unit, abi);
      }
      doPass("VOPT_SF_PEEPHOLES", [&] (Vunit& u) { sfPeepholes(u, abi); });
      doPass("VOPT_POST_RA_SIMPLIFY", postRASimplify);
    }
  }
  if (unit.blocks.size() > 1) {
    doPass("VOPT_JMP", [] (Vunit& u) { optimizeJmps(u); });
  }
}

void emitX64(Vunit& unit, Vtext& text, CGMeta& fixups,
             AsmInfo* asmInfo) {
#ifdef HAVE_LIBXED
  if (RuntimeOption::EvalUseXedAssembler) {
    return vasm_emit<Vgen<XedAssembler>>(unit, text, fixups, asmInfo);
  }
#endif
  vasm_emit<Vgen<X64Assembler>>(unit, text, fixups, asmInfo);
}

///////////////////////////////////////////////////////////////////////////////
}}
