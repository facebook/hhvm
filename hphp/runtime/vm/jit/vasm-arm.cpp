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
#include "hphp/runtime/vm/jit/back-end-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
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

struct Vgen {
  Vgen(Vunit& u, jit::vector<Vasm::Area>& areas, AsmInfo* asmInfo)
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
  void emit(bindcall& i);
  void emit(bindjcc& i);
  void emit(bindjmp& i);
  void emit(copy& i);
  void emit(copy2& i);
  void emit(debugtrap& i) { a->Brk(0); }
  void emit(fallbackcc i);
  void emit(fallback& i);
  void emit(hcsync& i);
  void emit(hcnocatch& i);
  void emit(hcunwind& i);
  void emit(hostcall& i);
  void emit(ldimmq& i);
  void emit(ldimml& i);
  void emit(ldimmb& i);
  void emit(ldimmqs& i) { not_implemented(); }
  void emit(load& i);
  void emit(store& i);
  void emit(syncpoint& i);

  // instructions
  void emit(addli& i) { a->Add(W(i.d), W(i.s1), i.s0.l(), vixl::SetFlags); }
  void emit(addq& i) { a->Add(X(i.d), X(i.s1), X(i.s0), vixl::SetFlags); }
  void emit(addqi& i) { a->Add(X(i.d), X(i.s1), i.s0.l(), vixl::SetFlags); }
  void emit(andq& i) { a->And(X(i.d), X(i.s1), X(i.s0) /* xxx flags */); }
  void emit(andqi& i) { a->And(X(i.d), X(i.s1), i.s0.l() /* xxx flags */); }
  void emit(asrv& i) { a->asrv(X(i.d), X(i.sl), X(i.sr)); }
  void emit(brk& i) { a->Brk(i.code); }
  void emit(cbcc& i);
  void emit(callr& i) { a->Blr(X(i.target)); }
  void emit(cmpl& i) { a->Cmp(W(i.s1), W(i.s0)); }
  void emit(cmpli& i) { a->Cmp(W(i.s1), i.s0.l()); }
  void emit(cmpq& i) { a->Cmp(X(i.s1), X(i.s0)); }
  void emit(cmpqi& i) { a->Cmp(X(i.s1), i.s0.l()); }
  void emit(cmpqims& i) { not_implemented(); }
  void emit(decq& i) { a->Sub(X(i.d), X(i.s), 1LL, vixl::SetFlags); }
  void emit(incq& i) { a->Add(X(i.d), X(i.s), 1LL, vixl::SetFlags); }
  void emit(jcc& i);
  void emit(jmp i);
  void emit(lea& i) { a->Add(X(i.d), X(i.s.base), i.s.disp); }
  void emit(loadl& i) { a->Ldr(W(i.d), M(i.s)); /* assume 0-extends */ }
  void emit(loadzbl& i) { a->Ldrb(W(i.d), M(i.s)); }
  void emit(lslv& i) { a->lslv(X(i.d), X(i.sl), X(i.sr)); }
  void emit(movzbl& i) { a->Uxtb(W(i.d), W(i.s)); }
  void emit(movzbq& i) { a->Uxtb(W(Vreg32(size_t(i.d))), W(i.s)); }
  void emit(mul& i) { a->Mul(X(i.d), X(i.s0), X(i.s1)); }
  void emit(neg& i) { a->Neg(X(i.d), X(i.s), vixl::SetFlags); }
  void emit(not& i) { a->Mvn(X(i.d), X(i.s)); }
  void emit(orq& i) { a->Orr(X(i.d), X(i.s1), X(i.s0) /* xxx flags? */); }
  void emit(orqi& i) { a->Orr(X(i.d), X(i.s1), i.s0.l() /* xxx flags? */); }
  void emit(ret& i) { a->Ret(); }
  void emit(storeb& i) { a->Strb(W(i.s), M(i.m)); }
  void emit(storel& i) { a->Str(W(i.s), M(i.m)); }
  void emit(setcc& i) { PhysReg r(i.d.asReg()); a->Cset(X(r), C(i.cc)); }
  void emit(subli& i) { a->Sub(W(i.d), W(i.s1), i.s0.l(), vixl::SetFlags); }
  void emit(subq& i) { a->Sub(X(i.d), X(i.s1), X(i.s0), vixl::SetFlags); }
  void emit(subqi& i) { a->Sub(X(i.d), X(i.s1), i.s0.l(), vixl::SetFlags); }
  void emit(tbcc& i);
  void emit(testl& i) { a->Tst(W(i.s1), W(i.s0)); }
  void emit(testli& i) { a->Tst(W(i.s1), i.s0.l()); }
  void emit(ud2& i) { a->Brk(1); }
  void emit(xorq& i) { a->Eor(X(i.d), X(i.s1), X(i.s0) /* xxx flags */); }
  void emit(xorqi& i) { a->Eor(X(i.d), X(i.s1), i.s0.l() /* xxx flags */); }

  CodeAddress start(Vlabel b) {
    auto area = unit.blocks[b].area;
    return areas[(int)area].start;
  }
  CodeBlock& main() { return area(AreaIndex::Main).code; }
  CodeBlock& cold() { return area(AreaIndex::Cold).code; }
  CodeBlock& frozen() { return area(AreaIndex::Frozen).code; }

private:
  Vasm::Area& area(AreaIndex i) {
    assertx((unsigned)i < areas.size());
    return areas[(unsigned)i];
  }

private:
  struct LabelPatch { CodeAddress instr; Vlabel target; };
  struct PointPatch { CodeAddress instr; Vpoint pos; Vreg d; };
  Vunit& unit;
  BackEnd& backend;
  jit::vector<Vasm::Area>& areas;
  AsmInfo* m_asmInfo;
  vixl::MacroAssembler* a;
  CodeBlock* codeBlock;
  Vlabel current{0}, next{0}; // in linear order
  jit::vector<CodeAddress> addrs, points;
  jit::vector<LabelPatch> jccs, jmps, bccs, catches;
  jit::vector<PointPatch> ldpoints;
};

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
  bool shouldUpdateAsmInfo = !!m_asmInfo
    && Trace::moduleEnabledRelease(HPHP::Trace::printir, kCodeGenLevel);

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
    codeBlock = &area(block.area).code;
    vixl::MacroAssembler as { *codeBlock };
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
    }

    const IRInstruction* currentOrigin = nullptr;
    auto blockInfo = shouldUpdateAsmInfo
      ? &areaToBlockInfos[unsigned(block.area)][b]
      : nullptr;
    auto start_snippet = [&](Vinstr& inst) {
      if (!shouldUpdateAsmInfo) return;

      blockInfo->snippets.push_back(
        Snippet { inst.origin, TcaRange { codeBlock->frontier(), nullptr } }
      );
    };
    auto finish_snippet = [&] {
      if (!shouldUpdateAsmInfo) return;

      if (!blockInfo->snippets.empty()) {
        auto& snip = blockInfo->snippets.back();
        snip.range = TcaRange { snip.range.start(), codeBlock->frontier() };
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
    assertx(addrs[p.target]);
    backend.smashJcc(p.instr, addrs[p.target]);
  }
  for (auto& p : bccs) {
    assertx(addrs[p.target]);
    auto link = (Instruction*) p.instr;
    link->SetImmPCOffsetTarget(Instruction::Cast(addrs[p.target]));
  }
  for (auto& p : jmps) {
    assertx(addrs[p.target]);
    backend.smashJmp(p.instr, addrs[p.target]);
  }
  for (auto& p : catches) {
    mcg->registerCatchBlock(p.instr, addrs[p.target]);
  }
  for (auto& p : ldpoints) {
    CodeCursor cc(main(), p.instr);
    MacroAssembler a{main()};
    a.Mov(X(p.d), points[p.pos]);
  }

  if (!shouldUpdateAsmInfo) {
    return;
  }

  for (auto i = 0; i < areas.size(); ++i) {
    const IRInstruction* currentOrigin = nullptr;
    auto& blockInfos = areaToBlockInfos[i];
    for (auto const blockID : labels) {
      auto const& blockInfo = blockInfos[static_cast<size_t>(blockID)];
      if (blockInfo.snippets.empty()) continue;

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

void Vgen::emit(bindcall& i) {
  mcg->backEnd().emitSmashableCall(*codeBlock, i.stub);
}

void Vgen::emit(bindjcc& i) {
  emitBindJ(*codeBlock, frozen(), i.cc, i.target);
}

void Vgen::emit(bindjmp& i) {
  // XXX what about trflags
  emitBindJ(*codeBlock, frozen(), CC_None, i.target);
}

void Vgen::emit(copy& i) {
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

void Vgen::emit(copy2& i) {
  PhysReg::Map<PhysReg> moves;
  Reg64 d0 = i.d0, d1 = i.d1, s0 = i.s0, s1 = i.s1;
  moves[d0] = s0;
  moves[d1] = s1;
  auto howTo = doRegMoves(moves, rAsm); // rAsm isn't used.
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      a->Mov(X(how.m_dst), X(how.m_src));
    } else {
      emitXorSwap(*a, X(how.m_dst), X(how.m_src));
    }
  }
}

void Vgen::emit(fallbackcc i) {
  auto const destSR = mcg->tx().getSrcRec(i.dest);
  if (!i.trflags.packed) {
    destSR->emitFallbackJump(*codeBlock, i.cc);
  } else {
    destSR->emitFallbackJumpCustom(*codeBlock, frozen(), i.dest, i.trflags);
  }
}

void Vgen::emit(fallback& i) {
  emit(fallbackcc{CC_None, InvalidReg, i.dest, i.trflags, i.args});
}

void Vgen::emit(hcsync& i) {
  assertx(points[i.call]);
  mcg->recordSyncPoint(points[i.call], i.fix);
}

void Vgen::emit(hcnocatch& i) {
  // register a null catch trace at the position of the call
  mcg->registerCatchBlock(points[i.call], nullptr);
}

void Vgen::emit(hcunwind& i) {
  catches.push_back({points[i.call], i.targets[1]});
  emit(jmp{i.targets[0]});
}

void Vgen::emit(hostcall& i) {
  points[i.syncpoint] = a->frontier();
  a->HostCall(i.argc);
}

void Vgen::emit(ldimmq& i) {
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

static void emitSimdImmInt(vixl::MacroAssembler* a, int64_t val, Vreg d) {
  if (val == 0) {
    a->Fmov(D(d), vixl::xzr);
  } else {
    a->Mov(rAsm, val); // XXX avoid scratch register somehow.
    a->Fmov(D(d), rAsm);
  }
}

void Vgen::emit(ldimml& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.q(), i.d);
  } else {
    Vreg32 d = i.d;
    a->Mov(W(d), i.s.l());
  }
}

void Vgen::emit(ldimmb& i) {
  if (i.d.isSIMD()) {
    emitSimdImmInt(a, i.s.q(), i.d);
  } else {
    Vreg8 d = i.d;
    a->Mov(W(d), i.s.b());
  }
}

void Vgen::emit(load& i) {
  if (i.d.isGP()) {
    a->Ldr(X(i.d), M(i.s));
  } else {
    a->Ldr(D(i.d), M(i.s));
  }
}

void Vgen::emit(store& i) {
  if (i.s.isGP()) {
    a->Str(X(i.s), M(i.d));
  } else {
    a->Str(D(i.s), M(i.d));
  }
}

void Vgen::emit(syncpoint& i) {
  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", a->frontier(),
         i.fix.pcOffset, i.fix.spOffset);
  mcg->recordSyncPoint(a->frontier(), i.fix);
}

void Vgen::emit(jmp i) {
  if (next == i.target) return;
  jmps.push_back({a->frontier(), i.target});
  // B range is +/- 128MB but this uses BR
  backend.emitSmashableJump(*codeBlock, kEndOfTargetChain, CC_None);
}

void Vgen::emit(jcc& i) {
  assertx(i.cc != CC_None);
  if (i.targets[1] != i.targets[0]) {
    if (next == i.targets[1]) {
      // the taken branch is the fall-through block, invert the branch.
      i = jcc{ccNegate(i.cc), i.sf, {i.targets[1], i.targets[0]}};
    }
    jccs.push_back({a->frontier(), i.targets[1]});
    // B.cond range is +/- 1MB but this uses BR
    backend.emitSmashableJump(*codeBlock, kEndOfTargetChain, i.cc);
  }
  emit(jmp{i.targets[0]});
}

void Vgen::emit(cbcc& i) {
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

void Vgen::emit(tbcc& i) {
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

// Lower svcreq{} by making copies to abi registers explicit, saving
// vm regs, and returning to the VM. svcreq{} is guaranteed to be
// at the end of a block, so we can just keep appending to the same block.
void lower_svcreq(Vunit& unit, Vlabel b, Vinstr& inst) {
  auto svcreq = inst.svcreq_; // copy it
  auto origin = inst.origin;
  auto& argv = unit.tuples[svcreq.extraArgs];
  unit.blocks[b].code.pop_back(); // delete the svcreq instruction
  Vout v(unit, b, origin);

  RegSet arg_regs;
  VregList arg_dests;
  for (int i = 0, n = argv.size(); i < n; ++i) {
    PhysReg d{serviceReqArgReg(i)};
    arg_dests.push_back(d);
    arg_regs |= d;
  }
  v << copyargs{svcreq.extraArgs, v.makeTuple(arg_dests)};
  // Save VM regs
  PhysReg vmfp{rVmFp}, vmsp{rVmSp}, sp{vixl::sp}, rdsp{rVmTl};
  v << store{vmfp, rdsp[rds::kVmfpOff]};
  v << store{vmsp, rdsp[rds::kVmspOff]};
  if (svcreq.stub_block) {
    always_assert(false && "use rip-rel addr to get ephemeral stub addr");
  } else {
    v << ldimmq{0, PhysReg{arm::rAsm}}; // because persist flag
  }
  v << ldimmq{svcreq.req, PhysReg{argReg(0)}};
  arg_regs |= arm::rAsm | argReg(0);

  // Weird hand-shaking with enterTC: reverse-call a service routine.
  // In the case of some special stubs (m_callToExit, m_retHelper), we
  // have already unbalanced the return stack by doing a ret to
  // something other than enterTCHelper.  In that case
  // SRJmpInsteadOfRet indicates to fake the return.
  v << load{sp[0], PhysReg{rLinkReg}};
  v << lea{sp[16], sp}; // fake postindexing
  arg_regs |= rLinkReg; // arm ret{} implicitly uses LR
  v << ret{arg_regs};
}

// Lower svcreq
void lower(Vunit& unit) {
  Timer _t(Timer::vasm_lower);
  for (size_t b = 0; b < unit.blocks.size(); ++b) {
    auto& code = unit.blocks[b].code;
    if (code.empty()) continue;
    if (code.back().op == Vinstr::svcreq) {
      lower_svcreq(unit, Vlabel{b}, code.back());
    }
    for (size_t i = 0; i < unit.blocks[b].code.size(); ++i) {
      auto& inst = unit.blocks[b].code[i];
      switch (inst.op) {
        case Vinstr::defvmsp:
          inst = copy{PhysReg{arm::rVmSp}, inst.defvmsp_.d};
          break;
        case Vinstr::syncvmsp:
          inst = copy{inst.syncvmsp_.s, PhysReg{arm::rVmSp}};
          break;
        default:
          break;
      }
    }
  }
}

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

void finishARM(Vunit& unit, Vasm::AreaList& areas,
               const Abi& abi, AsmInfo* asmInfo) {
  optimizeExits(unit);
  lower(unit);
  simplify(unit);
  if (!unit.constToReg.empty()) {
    foldImms<arm::ImmFolder>(unit);
  }
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
  auto blocks = layoutBlocks(unit);
  Vgen(unit, areas, asmInfo).emit(blocks);
}

///////////////////////////////////////////////////////////////////////////////
}}
