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

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <algorithm>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

using namespace reg;
using namespace x64;

namespace x64 { struct ImmFolder; }

namespace {
///////////////////////////////////////////////////////////////////////////////

struct Vgen {
  Vgen(const Vunit& u, Vasm::AreaList& areas, AsmInfo* asmInfo)
    : unit(u)
    , backend(mcg->backEnd())
    , areas(areas)
    , m_asmInfo(asmInfo) {
    addrs.resize(u.blocks.size());
    points.resize(u.next_point);
  }
  void emit(jit::vector<Vlabel>&);

 private:
  template<class Inst> void emit(const Inst& i) {
    always_assert_flog(false, "unimplemented instruction: {} in B{}\n",
                       vinst_names[Vinstr(i).op], size_t(current));
  }
  // intrinsics
  void emit(const bindaddr& i);
  void emit(const bindcall& i);
  void emit(const bindjcc1st& i);
  void emit(const bindjcc& i);
  void emit(const bindjmp& i);
  void emit(const callstub& i);
  void emit(const callfaststub& i);
  void emit(const contenter& i);
  void emit(const copy& i);
  void emit(const copy2& i);
  void emit(const debugtrap& i) { a->int3(); }
  void emit(const fallthru& i) {}
  void emit(const ldimmb& i);
  void emit(const ldimml& i);
  void emit(const ldimmq& i);
  void emit(const ldimmqs& i);
  void emit(const fallback& i);
  void emit(const fallbackcc& i);
  void emit(const load& i);
  void emit(const mccall& i);
  void emit(const mcprep& i);
  void emit(const nothrow& i);
  void emit(const store& i);
  void emit(const syncpoint& i);
  void emit(const unwind& i);
  void emit(const landingpad& i) {}
  void emit(const vretm& i);
  void emit(const vret& i);
  void emit(const leavetc&) { a->ret(); }

  // instructions
  void emit(andb i) { commuteSF(i); a->andb(i.s0, i.d); }
  void emit(andbi i) { binary(i); a->andb(i.s0, i.d); }
  void emit(const andbim& i) { a->andb(i.s, i.m); }
  void emit(andl i) { commuteSF(i); a->andl(i.s0, i.d); }
  void emit(andli i) { binary(i); a->andl(i.s0, i.d); }
  void emit(andq i) { commuteSF(i); a->andq(i.s0, i.d); }
  void emit(andqi i) { binary(i); a->andq(i.s0, i.d); }
  void emit(addli i) { binary(i); a->addl(i.s0, i.d); }
  void emit(const addlm& i) { a->addl(i.s0, i.m); }
  void emit(addq i) { commuteSF(i); a->addq(i.s0, i.d); }
  void emit(addqi i) { binary(i); a->addq(i.s0, i.d); }
  void emit(const addqim& i);
  void emit(addsd i) { commute(i); a->addsd(i.s0, i.d); }
  void emit(const call& i);
  void emit(const callm& i) { a->call(i.target); }
  void emit(const callr& i) { a->call(i.target); }
  void emit(const cloadq& i);
  void emit(const cmovq& i);
  void emit(const cmpb& i) { a->cmpb(i.s0, i.s1); }
  void emit(const cmpbi& i) { a->cmpb(i.s0, i.s1); }
  void emit(const cmpbim& i) { a->cmpb(i.s0, i.s1); }
  void emit(const cmpwim& i) { a->cmpw(i.s0, i.s1); }
  void emit(const cmpl& i) { a->cmpl(i.s0, i.s1); }
  void emit(const cmpli& i) { a->cmpl(i.s0, i.s1); }
  void emit(const cmplim& i) { a->cmpl(i.s0, i.s1); }
  void emit(const cmplm& i) { a->cmpl(i.s0, i.s1); }
  void emit(const cmpq& i) { a->cmpq(i.s0, i.s1); }
  void emit(const cmpqi& i) { a->cmpq(i.s0, i.s1); }
  void emit(const cmpqim& i) { a->cmpq(i.s0, i.s1); }
  void emit(const cmpqims& i);
  void emit(const cmpqm& i) { a->cmpq(i.s0, i.s1); }
  void emit(cmpsd i) { noncommute(i); a->cmpsd(i.s0, i.d, i.pred); }
  void emit(const cqo& i) { a->cqo(); }
  void emit(const cvttsd2siq& i) { a->cvttsd2siq(i.s, i.d); }
  void emit(const cvtsi2sd& i);
  void emit(const cvtsi2sdm& i);
  void emit(decl i) { unary(i); a->decl(i.d); }
  void emit(const declm& i) { a->decl(i.m); }
  void emit(decq i) { unary(i); a->decq(i.d); }
  void emit(const decqm& i) { a->decq(i.m); }
  void emit(divsd i) { noncommute(i); a->divsd(i.s0, i.d); }
  void emit(imul i) { commuteSF(i); a->imul(i.s0, i.d); }
  void emit(const idiv& i) { a->idiv(i.s); }
  void emit(incl i) { unary(i); a->incl(i.d); }
  void emit(const inclm& i) { a->incl(i.m); }
  void emit(incq i) { unary(i); a->incq(i.d); }
  void emit(const incqm& i) { a->incq(i.m); }
  void emit(const incqmlock& i) { a->lock(); a->incq(i.m); }
  void emit(const incwm& i) { a->incw(i.m); }
  void emit(const jcc& i);
  void emit(const jcci& i);
  void emit(const jmp& i);
  void emit(const jmpr& i) { a->jmp(i.target); }
  void emit(const jmpm& i) { a->jmp(i.target); }
  void emit(const jmpi& i) { a->jmp(i.target); }
  void emit(const lea& i);
  void emit(const leap& i) { a->lea(i.s, i.d); }
  void emit(const loadups& i) { a->movups(i.s, i.d); }
  void emit(const loadtqb& i) { a->loadb(i.s, i.d); }
  void emit(const loadl& i) { a->loadl(i.s, i.d); }
  void emit(const loadqp& i) { a->loadq(i.s, i.d); }
  void emit(const loadsd& i) { a->movsd(i.s, i.d); }
  void emit(const loadzbl& i) { a->loadzbl(i.s, i.d); }
  void emit(const loadzbq& i) { a->loadzbl(i.s, Reg32(i.d)); }
  void emit(const loadzlq& i) { a->loadl(i.s, Reg32(i.d)); }
  void emit(const movb& i) { a->movb(i.s, i.d); }
  void emit(const movl& i) { a->movl(i.s, i.d); }
  void emit(const movzbl& i) { a->movzbl(i.s, i.d); }
  void emit(const movzbq& i) { a->movzbl(i.s, Reg32(i.d)); }
  void emit(mulsd i) { commute(i); a->mulsd(i.s0, i.d); }
  void emit(neg i) { unary(i); a->neg(i.d); }
  void emit(const nop& i) { a->nop(); }
  void emit(not i) { unary(i); a->not(i.d); }
  void emit(notb i) { unary(i); a->notb(i.d); }
  void emit(const orwim& i) { a->orw(i.s0, i.m); }
  void emit(orq i) { commuteSF(i); a->orq(i.s0, i.d); }
  void emit(orqi i) { binary(i); a->orq(i.s0, i.d); }
  void emit(const orqim& i) { a->orq(i.s0, i.m); }
  void emit(const pop& i) { a->pop(i.d); }
  void emit(const popm& i) { a->pop(i.d); }
  void emit(psllq i) { binary(i); a->psllq(i.s0, i.d); }
  void emit(psrlq i) { binary(i); a->psrlq(i.s0, i.d); }
  void emit(const push& i) { a->push(i.s); }
  void emit(const roundsd& i) { a->roundsd(i.dir, i.s, i.d); }
  void emit(const ret& i) { a->ret(); }
  void emit(const sarq& i) { unary(i); a->sarq(i.d); }
  void emit(sarqi i) { binary(i); a->sarq(i.s0, i.d); }
  void emit(const setcc& i) { a->setcc(i.cc, i.d); }
  void emit(shlli i) { binary(i); a->shll(i.s0, i.d); }
  void emit(shlq i) { unary(i); a->shlq(i.d); }
  void emit(shlqi i) { binary(i); a->shlq(i.s0, i.d); }
  void emit(shrli i) { binary(i); a->shrl(i.s0, i.d); }
  void emit(shrqi i) { binary(i); a->shrq(i.s0, i.d); }
  void emit(const sqrtsd& i) { a->sqrtsd(i.s, i.d); }
  void emit(const storeups& i) { a->movups(i.s, i.m); }
  void emit(const storeb& i) { a->storeb(i.s, i.m); }
  void emit(const storebi& i);
  void emit(const storel& i) { a->storel(i.s, i.m); }
  void emit(const storeli& i) { a->storel(i.s, i.m); }
  void emit(const storeqi& i) { a->storeq(i.s, i.m); }
  void emit(const storesd& i) { a->movsd(i.s, i.m); }
  void emit(const storew& i) { a->storew(i.s, i.m); }
  void emit(const storewi& i) { a->storew(i.s, i.m); }
  void emit(subbi i) { binary(i); a->subb(i.s0, i.d); }
  void emit(subl i) { noncommute(i); a->subl(i.s0, i.d); }
  void emit(subli i) { binary(i); a->subl(i.s0, i.d); }
  void emit(subq i) { noncommute(i); a->subq(i.s0, i.d); }
  void emit(subqi i) { binary(i); a->subq(i.s0, i.d); }
  void emit(subsd i) { noncommute(i); a->subsd(i.s0, i.d); }
  void emit(const testb& i) { a->testb(i.s0, i.s1); }
  void emit(const testbi& i) { a->testb(i.s0, i.s1); }
  void emit(const testbim& i) { a->testb(i.s0, i.s1); }
  void emit(const testwim& i);
  void emit(const testl& i) { a->testl(i.s0, i.s1); }
  void emit(const testli& i) { a->testl(i.s0, i.s1); }
  void emit(const testlim& i);
  void emit(const testq& i) { a->testq(i.s0, i.s1); }
  void emit(const testqm& i) { a->testq(i.s0, i.s1); }
  void emit(const testqim& i);
  void emit(const ucomisd& i) { a->ucomisd(i.s0, i.s1); }
  void emit(const ud2& i) { a->ud2(); }
  void emit(unpcklpd i) { noncommute(i); a->unpcklpd(i.s0, i.d); }
  void emit(xorb i) { commuteSF(i); a->xorb(i.s0, i.d); }
  void emit(xorbi i) { binary(i); a->xorb(i.s0, i.d); }
  void emit(xorl i) { commuteSF(i); a->xorl(i.s0, i.d); }
  void emit(xorq i) { commuteSF(i); a->xorq(i.s0, i.d); }
  void emit(xorqi i) { binary(i); a->xorq(i.s0, i.d); }

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
    assertx((unsigned)i < areas.size());
    return areas[(unsigned)i];
  }

private:
  struct LabelPatch { CodeAddress instr; Vlabel target; };
  struct PointPatch { CodeAddress instr; Vpoint pos; };
  const Vunit& unit;
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

/*
 * Prepare a binary op that is not commutative.
 *
 * s0 must be a different register than s1 so we don't clobber it.
 */
template<class Inst> void Vgen::noncommute(Inst& i) {
  assertx(i.s1 == i.d || i.s0 != i.d); // do not clobber s0
  binary(i);
}

/*
 * Prepare a binary op that is commutative.
 *
 * Swap operands if the dest is s0.
 */
template<class Inst> void Vgen::commuteSF(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d, i.sf};
  } else {
    binary(i);
  }
}

template<class Inst> void Vgen::commute(Inst& i) {
  if (i.s1 != i.d && i.s0 == i.d) {
    i = Inst{i.s1, i.s0, i.d};
  } else {
    binary(i);
  }
}

///////////////////////////////////////////////////////////////////////////////

bool is_empty_catch(const Vblock& block) {
  return block.code.size() == 2 &&
         block.code[0].op == Vinstr::landingpad &&
         block.code[1].op == Vinstr::jmpi &&
         block.code[1].jmpi_.target == mcg->tx().uniqueStubs.endCatchHelper;
}

/*
 * Toplevel emitter.
 */
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
    assertx(checkBlockEnd(unit, labels[i]));

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

    auto const start_snippet = [&] (const Vinstr& inst) {
      if (!shouldUpdateAsmInfo) return;

      blockInfo->snippets.push_back(
        Snippet { inst.origin, TcaRange { a->code().frontier(), nullptr } }
      );
    };
    auto const finish_snippet = [&] {
      if (!shouldUpdateAsmInfo) return;

      if (!blockInfo->snippets.empty()) {
        auto& snip = blockInfo->snippets.back();
        snip.range = TcaRange { snip.range.start(), a->code().frontier() };
      }
    };

    // We'll replace exception edges to empty catch blocks with the catch
    // helper unique stub.
    if (is_empty_catch(block)) continue;

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
    assertx(addrs[p.target]);
    X64Assembler::patchJcc(p.instr, addrs[p.target]);
  }
  for (auto& p : jmps) {
    assertx(addrs[p.target]);
    X64Assembler::patchJmp(p.instr, addrs[p.target]);
  }
  for (auto& p : calls) {
    assertx(addrs[p.target]);
    X64Assembler::patchCall(p.instr, addrs[p.target]);
  }
  for (auto& p : catches) {
    auto const catch_target = is_empty_catch(unit.blocks[p.target])
      ? mcg->tx().uniqueStubs.endCatchHelper
      : addrs[p.target];
    mcg->registerCatchBlock(p.instr, catch_target);
  }
  for (auto& p : ldpoints) {
    auto after_lea = p.instr + 7;
    auto d = points[p.pos] - after_lea;
    assertx(deltaFits(d, sz::dword));
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

///////////////////////////////////////////////////////////////////////////////

void Vgen::emit(const addqim& i) {
  if (i.m.seg == Vptr::FS) a->fs();
  a->addq(i.s0, i.m.mr());
}

void Vgen::emit(const call& i) {
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

void Vgen::emit(const cloadq& i) {
  auto m = i.t;
  always_assert(!m.index.isValid()); // not supported, but could be later.
  if (i.f != i.d) {
    if (i.d == m.base) {
      // We can't move f over d or we'll clobber the Vptr we need to load from.
      // Since cload does the load unconditionally anyway, we can just load and
      // cmov.
      a->loadq(i.t, i.d);
      a->cmov_reg64_reg64(ccNegate(i.cc), i.f, i.d);
      return;
    }
    a->movq(i.f, i.d);
  }
  a->cload_reg64_disp_reg64(i.cc, m.base, m.disp, i.d);
}

// add s0 s1 d => mov s1->d; d += s0
// cmov cc s d => if cc { mov s->d }
void Vgen::emit(const cmovq& i) {
  if (i.f != i.d && i.t == i.d) {
    // negate the condition and swap t/f operands so we dont clobber i.t
    return emit(cmovq{ccNegate(i.cc), i.sf, i.t, i.f, i.d});
  } else {
    prep(i.f, i.d);
  }
  a->cmov_reg64_reg64(i.cc, i.t, i.d);
}

void Vgen::emit(const contenter& i) {
  Label Stub, End;
  Reg64 fp = i.fp, target = i.target;
  a->jmp8(End);

  asm_label(*a, Stub);
  a->pop(fp[AROFF(m_savedRip)]);
  a->jmp(target);

  asm_label(*a, End);
  a->call(Stub);
  // m_savedRip will point here.
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const copy& i) {
  if (i.s == i.d) return;
  if (i.s.isGP()) {
    if (i.d.isGP()) {                 // GP => GP
      a->movq(i.s, i.d);
    } else {                             // GP => XMM
      assertx(i.d.isSIMD());
      // This generates a movq x86 instruction, which zero extends
      // the 64-bit value in srcReg into a 128-bit XMM register
      a->movq_rx(i.s, i.d);
    }
  } else {
    if (i.d.isGP()) {                 // XMM => GP
      a->movq_xr(i.s, i.d);
    } else {                             // XMM => XMM
      assertx(i.d.isSIMD());
      // This copies all 128 bits in XMM,
      // thus avoiding partial register stalls
      a->movdqa(i.s, i.d);
    }
  }
}

void Vgen::emit(const copy2& i) {
  assertx(i.s0.isValid() && i.s1.isValid() && i.d0.isValid() && i.d1.isValid());
  auto s0 = i.s0, s1 = i.s1, d0 = i.d0, d1 = i.d1;
  assertx(d0 != d1);
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

void Vgen::emit(const bindaddr& i) {
  *i.dest = emitBindAddr(a->code(), frozen(), i.dest, i.sk, i.spOff);
  mcg->setJmpTransID(TCA(i.dest));
}

void Vgen::emit(const bindcall& i) {
  mcg->backEnd().prepareForSmash(a->code(), kCallLen);
  a->call(i.stub);
  emit(unwind{{i.targets[0], i.targets[1]}});
}

void Vgen::emit(const bindjcc1st& i) {
  emitBindJmpccFirst(a->code(), frozen(), i.cc, i.targets[0], i.targets[1],
                     i.spOff);
}

void Vgen::emit(const bindjcc& i) {
  emitBindJ(
    a->code(),
    frozen(),
    i.cc,
    i.target,
    i.spOff,
    i.trflags
  );
}

void Vgen::emit(const bindjmp& i) {
  emitBindJ(
    a->code(),
    frozen(),
    CC_None,
    i.target,
    i.spOff,
    i.trflags
  );
}

void Vgen::emit(const callstub& i) {
  emit(call{i.target, i.args});
}

void Vgen::emit(const callfaststub& i) {
  emit(call{i.target, i.args});
  emit(syncpoint{i.fix});
}

void Vgen::emit(const cmpqims& i) {
  backend.prepareForSmash(a->code(), kCmpLen);
  a->cmpq(i.s0, i.s1);
}

void Vgen::emit(const fallback& i) {
  emit(fallbackcc{CC_None, InvalidReg, i.dest, i.trflags, i.args});
}

void Vgen::emit(const fallbackcc& i) {
  auto const destSR = mcg->tx().getSrcRec(i.dest);
  if (!i.trflags.packed) {
    destSR->emitFallbackJump(a->code(), i.cc);
  } else {
    destSR->emitFallbackJumpCustom(a->code(), frozen(), i.dest, i.trflags);
  }
}

void emitSimdImm(X64Assembler* a, int64_t val, Vreg d) {
  if (val == 0) {
    a->pxor(d, d); // does not modify flags
  } else {
    auto addr = mcg->allocLiteral(val);
    a->movsd(rip[(intptr_t)addr], d);
  }
}

void Vgen::emit(const ldimmb& i) {
  // ldimmb is for Vconst::Byte, which is treated as unsigned uint8_t
  auto val = i.s.b();
  if (i.d.isGP()) {
    Vreg8 d8 = i.d;
    a->movb(val, d8);
  } else {
    emitSimdImm(a, uint8_t(val), i.d);
  }
}

void Vgen::emit(const ldimml& i) {
  // ldimml is for Vconst::Long, which is treated as unsigned uint32_t
  auto val = i.s.l();
  if (i.d.isGP()) {
    Vreg32 d32 = i.d;
    a->movl(val, d32);
  } else {
    emitSimdImm(a, uint32_t(val), i.d);
  }
}

void Vgen::emit(const ldimmq& i) {
  auto val = i.s.q();
  if (i.d.isGP()) {
    if (val == 0) {
      Vreg32 d32 = i.d;
      a->movl(0, d32); // because emitImmReg tries the xor optimization
    } else {
      a->emitImmReg(i.s, i.d);
    }
  } else {
    emitSimdImm(a, val, i.d);
  }
}

void Vgen::emit(const ldimmqs& i) {
  backend.prepareForSmash(a->code(), kMovLen);
  a->movq(0xdeadbeeffeedface, i.d);

  auto immp = reinterpret_cast<uintptr_t*>(a->frontier()) - 1;
  *immp = i.s.q();
}

void Vgen::emit(const load& i) {
  if (i.s.seg == Vptr::FS) a->fs();
  auto mref = i.s.mr();
  if (i.d.isGP()) {
    a->loadq(mref, i.d);
  } else {
    assertx(i.d.isSIMD());
    a->movsd(mref, i.d);
  }
}

void Vgen::emit(const mccall& i) {
  backend.prepareForSmash(a->code(), kCallLen);
  a->call(i.target);
}

// emit smashable mov as part of method cache callsite
void Vgen::emit(const mcprep& i) {
  /*
   * For the first time through, set the cache to hold the address
   * of the movq (*2 + 1), so we can find the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * handlePrimeCacheMiss can tell it's not been smashed yet
   */
  emit(ldimmqs{0x8000000000000000u, i.d});

  auto movAddr = reinterpret_cast<uintptr_t>(a->frontier()) - x64::kMovLen;
  auto immAddr = reinterpret_cast<uintptr_t*>(movAddr + x64::kMovImmOff);

  *immAddr = (movAddr << 1) | 1;
  mcg->cgFixups().m_addressImmediates.insert(reinterpret_cast<TCA>(~movAddr));
}

void Vgen::emit(const storebi& i) {
  if (i.m.seg == Vptr::FS) a->fs();
  a->storeb(i.s, i.m.mr());
}

void Vgen::emit(const store& i) {
  if (i.s.isGP()) {
    a->storeq(i.s, i.d);
  } else {
    assertx(i.s.isSIMD());
    a->movsd(i.s, i.d);
  }
}

void Vgen::emit(const syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(a->frontier(), i.fix);
}

void Vgen::emit(const testwim& i) {
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

void Vgen::emit(const testlim& i) {
  a->testl(i.s0, i.s1);
}

void Vgen::emit(const testqim& i) {
  // The immediate is 32 bits, sign-extended to 64. If the sign bit isn't set,
  // we can get the same results by emitting a testlim.
  if (i.s0.l() < 0) {
    a->testq(i.s0, i.s1);
  } else {
    emit(testlim{i.s0, i.s1, i.sf});
  }
}

void Vgen::emit(const nothrow& i) {
  // register a null catch trace at this position, telling the unwinder that
  // the function call returning to here isn't allowed to throw.
  mcg->registerCatchBlock(a->frontier(), nullptr);
}

void Vgen::emit(const unwind& i) {
  // Unwind instructions terminate blocks with calls that can throw, and have
  // the edges to catch (unwinder) blocks and fall-through blocks.
  catches.push_back({a->frontier(), i.targets[1]});
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const vretm& i) {
  a->push(i.retAddr);
  a->loadq(i.prevFp, i.d);
  a->ret();
}

void Vgen::emit(const vret& i) {
  a->push(i.retAddr);
  a->ret();
}

void Vgen::emit(const cvtsi2sd& i) {
  a->pxor(i.d, i.d);
  a->cvtsi2sd(i.s, i.d);
}

void Vgen::emit(const cvtsi2sdm& i) {
  a->pxor(i.d, i.d);
  a->cvtsi2sd(i.s, i.d);
}

void Vgen::emit(const jcc& i) {
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      return emit(jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}});
    }
    auto taken = i.targets[1];
    jccs.push_back({a->frontier(), taken});
    a->jcc(i.cc, a->frontier());
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(const jcci& i) {
  a->jcc(i.cc, i.taken);
  emit(jmp{i.target});
}

void Vgen::emit(const jmp& i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  a->jmp(a->frontier());
}

void Vgen::emit(const lea& i) {
  // could do this in a simplify pass
  if (i.s.disp == 0 && i.s.base.isValid() && !i.s.index.isValid()) {
    emit(copy{i.s.base, i.d});
  } else {
    a->lea(i.s, i.d);
  }
}

///////////////////////////////////////////////////////////////////////////////

// Lower svcreq{} by making copies to abi registers explicit, saving
// vm regs, and returning to the VM. svcreq{} is guaranteed to be
// at the end of a block, so we can just keep appending to the same
// block.
void lower_svcreq(Vunit& unit, Vlabel b, const Vinstr& inst) {
  assertx(unit.tuples[inst.svcreq_.extraArgs].size() < kNumServiceReqArgRegs);
  auto svcreq = inst.svcreq_; // copy it
  auto origin = inst.origin;
  auto& argv = unit.tuples[svcreq.extraArgs];
  unit.blocks[b].code.pop_back(); // delete the svcreq instruction
  Vout v(unit, b, origin);

  RegSet arg_regs = svcreq.args;
  VregList arg_dests;
  for (int i = 0, n = argv.size(); i < n; ++i) {
    PhysReg d{serviceReqArgRegs[i]};
    arg_dests.push_back(d);
    arg_regs |= d;
  }
  v << copyargs{svcreq.extraArgs, v.makeTuple(arg_dests)};
  if (svcreq.stub_block) {
    v << leap{rip[(int64_t)svcreq.stub_block], rAsm};
  } else {
    v << ldimmq{0, rAsm}; // because persist flag
  }
  v << ldimmq{svcreq.req, rdi};
  arg_regs |= rAsm | rdi | rVmFp | rVmSp;

  v << jmpi{TCA(handleSRHelper), arg_regs};
}

void lowerSrem(Vunit& unit, Vlabel b, size_t iInst) {
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
void lowerShift(Vunit& unit, Vlabel b, size_t iInst) {
  auto const& inst = unit.blocks[b].code[iInst];
  auto const& shift = inst.get<FromOp>();
  auto scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, inst.origin);
  v << copy{shift.s0, rcx};
  v << ToOp{shift.s1, shift.d, shift.sf};

  vector_splice(unit.blocks[b].code, iInst, 1, unit.blocks[scratch].code);
}

void lowerAbsdbl(Vunit& unit, Vlabel b, size_t iInst) {
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

void lowerVcall(Vunit& unit, Vlabel b, size_t iInst) {
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
      assertx(taken.front().op == Vinstr::landingpad ||
             taken.front().op == Vinstr::jmp);
      Vinstr v{lea{rsp[rspOffset], rsp}};
      v.origin = taken.front().origin;
      if (taken.front().op == Vinstr::jmp) {
        taken.insert(taken.begin(), v);
      } else {
        taken.insert(taken.begin() + 1, v);
      }
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
      static_assert(offsetof(TypedValue, m_data) == 0, "");
      static_assert(offsetof(TypedValue, m_type) == 8, "");
      if (dests.size() == 2) {
        v << copy2{reg::rax, reg::rdx, dests[0], dests[1]};
      } else {
        // We have cases where we statically know the type but need the value
        // from native call. Even if the type does not really need a register
        // (e.g., InitNull), a Vreg is still allocated in assignRegs(), so the
        // following assertion holds.
        assertx(dests.size() == 1);
        v << copy{reg::rax, dests[0]};
      }
      break;
    }
    case DestType::SIMD: {
      // rax contains m_type and m_aux but we're expecting just the type in
      // the lower bits, so shift the type result register.
      static_assert(offsetof(TypedValue, m_data) == 0, "");
      static_assert(offsetof(TypedValue, m_type) == 8, "");
      assertx(dests.size() == 1);
      pack2(v, reg::rax, reg::rdx, dests[0]);
      break;
    }
    case DestType::SSA:
    case DestType::Byte:
      // copy the single-register result to dests[0]
      assertx(dests.size() == 1);
      assertx(dests[0].isValid());
      v << copy{reg::rax, dests[0]};
      break;
    case DestType::None:
      assertx(dests.empty());
      break;
    case DestType::Dbl:
      // copy the single-register result to dests[0]
      assertx(dests.size() == 1);
      assertx(dests[0].isValid());
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

void lower_vcallstub(Vunit& unit, Vlabel b) {
  auto& code = unit.blocks[b].code;
  // vcallstub can only appear at the end of a block.
  auto const inst = code.back().get<vcallstub>();
  auto const origin = code.back().origin;

  auto argRegs = inst.args;
  auto const& srcs = unit.tuples[inst.extraArgs];
  jit::vector<Vreg> dsts;
  for (int i = 0; i < srcs.size(); ++i) {
    dsts.emplace_back(argNumToRegName[i]);
    argRegs |= argNumToRegName[i];
  }

  code.back() = copyargs{unit.makeTuple(srcs), unit.makeTuple(std::move(dsts))};
  code.emplace_back(callstub{inst.target, argRegs});
  code.back().origin = origin;
  code.emplace_back(unwind{{inst.targets[0], inst.targets[1]}});
  code.back().origin = origin;
}

/*
 * Lower a few abstractions to facilitate straightforward x64 codegen.
 */
void lowerForX64(Vunit& unit, const Abi& abi) {
  Timer _t(Timer::vasm_lower);

  // This pass relies on having no critical edges in the unit.
  splitCriticalEdges(unit);

  // Scratch block can change blocks allocation, hence cannot use regular
  // iterators.
  auto& blocks = unit.blocks;

  PostorderWalker{unit}.dfs([&](Vlabel ib) {
    assertx(!blocks[ib].code.empty());
    auto& back = blocks[ib].code.back();
    if (back.op == Vinstr::svcreq) {
      lower_svcreq(unit, Vlabel{ib}, blocks[ib].code.back());
    } else if (back.op == Vinstr::vcallstub) {
      lower_vcallstub(unit, Vlabel{ib});
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

        case Vinstr::countbytecode:
          inst = incqm{inst.countbytecode_.base[g_bytecodesVasm.handle()],
                       inst.countbytecode_.sf};
          break;

        default:
          break;
      }
    }
  });

  printUnit(kVasmLowerLevel, "after lower for X64", unit);
}

///////////////////////////////////////////////////////////////////////////////
}

void optimizeX64(Vunit& unit, const Abi& abi) {
  Timer timer(Timer::vasm_optimize);

  removeTrivialNops(unit);
  optimizePhis(unit);
  fuseBranches(unit);
  optimizeJmps(unit);
  optimizeExits(unit);

  lowerForX64(unit, abi);

  simplify(unit);

  if (!unit.constToReg.empty()) {
    foldImms<x64::ImmFolder>(unit);
  }
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

void emitX64(const Vunit& unit, Vasm::AreaList& areas, AsmInfo* asmInfo) {
  static thread_local bool busy;
  always_assert(!busy);
  busy = true;
  SCOPE_EXIT { busy = false; };

  Timer timer(Timer::vasm_gen);
  auto blocks = layoutBlocks(unit);
  Vgen(unit, areas, asmInfo).emit(blocks);
}

///////////////////////////////////////////////////////////////////////////////
}}
