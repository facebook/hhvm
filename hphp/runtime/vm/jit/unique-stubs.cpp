/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h" // TODO(#7728856)
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/phys-reg-saver.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/unique-stubs-x64.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/disasm.h"
#include "hphp/util/trace.h"

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void alignJmpTarget(CodeBlock& cb) {
  align(cb, Alignment::JmpTarget, AlignContext::Dead);
}

void assertNativeStackAligned(Vout& v) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << call{TCA(assert_native_stack_aligned)};
  }
}

void loadVMRegs(Vout& v) {
  v << load{rvmtl()[rds::kVmfpOff], rvmfp()};
  v << load{rvmtl()[rds::kVmspOff], rvmsp()};
}

void storeVMRegs(Vout& v) {
  v << store{rvmfp(), rvmtl()[rds::kVmfpOff]};
  v << store{rvmsp(), rvmtl()[rds::kVmspOff]};
}

void loadMCG(Vout& v, Vreg d) {
  // TODO(#8060678): Why does this need to be RIP-relative?
  auto const imcg = reinterpret_cast<uintptr_t>(&mcg);
  v << loadqp{reg::rip[imcg], d};
}

/*
 * Convenience wrapper around a simple vcall to `helper', with a single `arg'
 * and a return value in `d'.
 */
template<class F>
Vinstr simplecall(Vout& v, F helper, Vreg arg, Vreg d) {
  return vcall{
    CppCall::direct(helper),
    v.makeVcallArgs({{arg}}),
    v.makeTuple({d}),
    Fixup{},
    DestType::SSA
  };
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Set up any registers we want live going into an LLVM catch block.
 *
 * Return the set of live registers.
 */
RegSet syncForLLVMCatch(Vout& v) {
  if (arch() != Arch::X64) return RegSet{};
  return x64::syncForLLVMCatch(v);
}

/*
 * Load RIP from wherever it's stashed at the beginning of a new native frame.
 */
void loadSavedRIP(Vout& v, Vreg d) {
  if (arch() != Arch::X64) not_implemented();
  return x64::loadSavedRIP(v, d);
}

/*
 * Stash the saved RIP of the native frame in the VM frame's m_savedRip, and
 * also align the native stack.
 *
 * TODO(#7728856): Use this for EnterFrame as well.
 */
void stashSavedRIP(Vout& v, Vreg fp) {
  if (arch() != Arch::X64) not_implemented();
  return x64::stashSavedRIP(v, fp);
}

/*
 * Align the native stack in an architecture-specific way.
 */
template<class GenFunc>
void alignNativeStack(Vout& v, GenFunc gen) {
  if (arch() != Arch::X64) return gen(v);
  return x64::alignNativeStack(v, gen);
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFunctionEnterHelper(CodeBlock& cb, UniqueStubs& us) {
  if (arch() != Arch::X64) not_implemented();
  return x64::emitFunctionEnterHelper(cb, us);
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, UniqueStubs& us) {
  if (arch() != Arch::X64) not_implemented();
  return x64::emitFreeLocalsHelpers(cb, us);
}

TCA emitCallToExit(CodeBlock& cb) {
  if (arch() != Arch::X64) not_implemented();
  return x64::emitCallToExit(cb);
}

TCA emitEndCatchHelper(CodeBlock& cb, UniqueStubs& us) {
  if (arch() != Arch::X64) not_implemented();
  return x64::emitEndCatchHelper(cb, us);
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFuncPrologueRedispatch(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    auto const func = v.makeReg();
    v << load{rvmfp()[AROFF(m_func)], func};

    auto const argc = v.makeReg();
    auto const naaf = v.makeReg();
    v << loadl{rvmfp()[AROFF(m_numArgsAndFlags)], naaf};
    v << andli{ActRec::kNumArgsMask, naaf, argc, v.makeReg()};

    auto const nparams = v.makeReg();
    auto const pcounts = v.makeReg();
    v << loadl{func[Func::paramCountsOff()], pcounts};
    v << shrli{0x1, pcounts, nparams, v.makeReg()};

    auto const sf = v.makeReg();
    v << cmpl{argc, nparams, sf};

    auto const pTabOff = safe_cast<int32_t>(Func::prologueTableOff());

    // If we passed more args than declared, we might need to dispatch to the
    // "too many arguments" prologue.
    ifThen(v, CC_L, sf, [&] (Vout& v) {
      auto const sf = v.makeReg();

      // If we passed fewer than kNumFixedPrologues, argc is still a valid
      // index into the prologue table.
      v << cmpli{kNumFixedPrologues, argc, sf};

      ifThen(v, CC_NL, sf, [&] (Vout& v) {
        auto const dest = v.makeReg();

        // Too many gosh-darned arguments passed.  Go to the (nparams + 1)-th
        // prologue, which is always the "too many args" entry point.
        v << load{func[nparams * 8 + (pTabOff + int32_t(sizeof(TCA)))], dest};
        v << jmpr{dest};
      });
    });

    auto const dest = v.makeReg();
    v << load{func[argc * 8 + pTabOff], dest};
    v << jmpr{dest};
  });
}

TCA emitFCallHelperThunk(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap2(cb, [] (Vout& v, Vout& vcold) {
    auto const dest = v.makeReg();

    v << popm{rvmfp()[AROFF(m_savedRip)]};
    // fcallHelper asserts native stack alignment for us.
    TCA (*helper)(ActRec*) = &fcallHelper;
    v << simplecall(v, helper, rvmfp(), dest);
    v << pushm{rvmfp()[AROFF(m_savedRip)]};

    // Clobber rvmsp in debug builds.
    if (debug) v << copy{v.cns(0x1), rvmsp()};

    auto const sf = v.makeReg();
    v << testq{dest, dest, sf};

    unlikelyIfThen(v, vcold, CC_Z, sf, [&] (Vout& v) {
      // A nullptr dest means the callee was intercepted and should be skipped.
      // Just return to the caller after syncing VM regs.
      loadVMRegs(v);
      v << ret{};
    });

    // Jump to the func prologue.
    v << jmpr{dest};
  });
}

TCA emitFuncBodyHelperThunk(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    TCA (*helper)(ActRec*) = &funcBodyHelper;
    auto const dest = v.makeReg();

    // The funcBodyHelperThunk stub is reached via a jmp from the TC, so the
    // stack parity is already correct.
    v << simplecall(v, helper, rvmfp(), dest);

    v << jmpr{dest};
  });
}

TCA emitFunctionSurprisedOrStackOverflow(CodeBlock& cb,
                                         const UniqueStubs& us) {
  alignJmpTarget(cb);

  return vwrap(cb, [&] (Vout& v) {
    alignNativeStack(v, [&] (Vout& v) {
      v << vcall{CppCall::direct(handlePossibleStackOverflow),
                 v.makeVcallArgs({{rvmfp()}}), v.makeTuple({})};
    });
    v << jmpi{us.functionEnterHelper};
  });
}

///////////////////////////////////////////////////////////////////////////////

template<bool async>
void loadGenFrame(Vout& v, Vreg d) {
  auto const arOff = BaseGenerator::arOff() -
    (async ? AsyncGenerator::objectOff() : Generator::objectOff());

  auto const gen = v.makeReg();

  // We have to get the Generator object from the current frame's $this, then
  // load the embedded frame.
  v << load{rvmfp()[AROFF(m_this)], gen};
  v << lea{gen[arOff], d};
}

void debuggerRetImpl(Vout& v, Vreg ar) {
  auto const soff = v.makeReg();

  v << loadl{ar[AROFF(m_soff)], soff};
  v << storel{soff, rvmtl()[unwinderDebuggerReturnOffOff()]};
  v << store{rvmsp(), rvmtl()[unwinderDebuggerReturnSPOff()]};

  auto const ret = v.makeReg();
  v << simplecall(v, popDebuggerCatch, ar, ret);

  auto const args = syncForLLVMCatch(v);
  v << jmpr{ret, args};
}

TCA emitInterpRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, [] (Vout& v) {
    assertNativeStackAligned(v);
    v << lea{rvmsp()[-AROFF(m_r)], r_svcreq_arg(0)};
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_INTERP_RET);
  return start;
}

template<bool async>
TCA emitInterpGenRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  auto const start = vwrap(cb, [] (Vout& v) {
    assertNativeStackAligned(v);
    loadGenFrame<async>(v, r_svcreq_arg(0));
    v << copy{rvmfp(), r_svcreq_arg(1)};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_INTERP_RET);
  return start;
}

TCA emitDebuggerInterpRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    assertNativeStackAligned(v);

    auto const ar = v.makeReg();
    v << lea{rvmsp()[-AROFF(m_r)], ar};
    debuggerRetImpl(v, ar);
  });
}

template<bool async>
TCA emitDebuggerInterpGenRet(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    assertNativeStackAligned(v);

    auto const ar = v.makeReg();
    loadGenFrame<async>(v, ar);
    debuggerRetImpl(v, ar);
  });
}

///////////////////////////////////////////////////////////////////////////////

template<bool immutable>
TCA emitBindCallStub(CodeBlock& cb) {
  return vwrap(cb, [] (Vout& v) {
    auto args = VregList { v.makeReg(), v.makeReg(),
                           v.makeReg(), v.makeReg() };
    loadMCG(v, args[0]);

    // Reconstruct the address of the call from the saved RIP.
    auto const savedRIP = v.makeReg();
    auto const callLen = safe_cast<int>(smashableCallLen());
    loadSavedRIP(v, savedRIP);
    v << subqi{callLen, savedRIP, args[1], v.makeReg()};

    v << copy{rvmfp(), args[2]};
    v << movb{v.cns(immutable), args[3]};

    auto const ret = v.makeReg();

    alignNativeStack(v, [&] (Vout& v) {
      auto const handler = reinterpret_cast<void (*)()>(
        getMethodPtr(&MCGenerator::handleBindCall)
      );
      v << vcall{
        CppCall::direct(handler),
        v.makeVcallArgs({args}),
        v.makeTuple({ret}),
        Fixup{},
        DestType::SSA
      };
    });

    v << jmpr{ret};
  });
}

TCA emitFCallArrayHelper(CodeBlock& cb) {
  align(cb, Alignment::CacheLine, AlignContext::Dead);

  return vwrap2(cb, [] (Vout& v, Vout& vcold) {
    storeVMRegs(v);

    auto const func = v.makeReg();
    auto const unit = v.makeReg();
    auto const bc = v.makeReg();

    // Load fp->m_func->m_unit->m_bc.
    v << load{rvmfp()[AROFF(m_func)], func};
    v << load{func[Func::unitOff()], unit};
    v << load{unit[Unit::bcOff()], bc};

    auto const pc = v.makeReg();
    auto const next = v.makeReg();

    // Convert offsets into PCs, and sync the PC.
    v << addq{bc, rarg(0), pc, v.makeReg()};
    v << store{pc, rvmtl()[rds::kVmpcOff]};
    v << addq{bc, rarg(1), next, v.makeReg()};

    auto const res = v.makeReg();

    alignNativeStack(v, [&] (Vout& v) {
      bool (*helper)(PC) = &doFCallArrayTC;
      v << simplecall(v, helper, next, res);
    });
    v << load{rvmtl()[rds::kVmspOff], rvmsp()};

    auto const sf = v.makeReg();
    v << testb{res, res, sf};

    unlikelyIfThen(v, vcold, CC_Z, sf, [&] (Vout& v) {
      // If false was returned, we should skip the callee.  The interpreter
      // will have popped the pre-live ActRec already, so we can just return to
      // the caller.
      v << ret{};
    });
    v << load{rvmtl()[rds::kVmfpOff], rvmfp()};

    // If true was returned, head to the func body.  Stashing the saved RIP
    // here performs the same work as EnterFrame in func prologues.
    stashSavedRIP(v, rvmfp());

    auto const callee = v.makeReg();
    auto const body = v.makeReg();

    v << load{rvmfp()[AROFF(m_func)], callee};
    v << load{callee[Func::funcBodyOff()], body};

    // We jmp directly to the func body---this keeps the return stack buffer
    // balanced between the call to this stub and the ret from the callee.
    v << jmpr{body};
  });
}

///////////////////////////////////////////////////////////////////////////////

struct ResumeHelper {
  TCA handle_resume;
  TCA reenter_tc;
};

ResumeHelper resumeHelperBody(CodeBlock& cb) {
  auto const handle_resume = vwrap(cb, [] (Vout& v) {
    v << load{rvmtl()[rds::kVmfpOff], rvmfp()};
    loadMCG(v, rarg(0));

    auto const handler = reinterpret_cast<TCA>(
      getMethodPtr(&MCGenerator::handleResume)
    );
    v << call{handler, arg_regs(2)};
  });

  auto const reenter_tc = vwrap(cb, [] (Vout& v) {
    loadVMRegs(v);
    auto const args = syncForLLVMCatch(v);
    v << jmpr{rret(), args};
  });

  return ResumeHelper { handle_resume, reenter_tc };
}

TCA emitResumeInterpHelpers(CodeBlock& cb, UniqueStubs& us) {
  alignJmpTarget(cb);

  us.resumeHelperRet = vwrap(cb, [] (Vout& v) {
    stashSavedRIP(v, rvmfp());
  });
  us.resumeHelper = vwrap(cb, [] (Vout& v) {
    v << ldimmb{0, rarg(1)};
  });
  auto const body = resumeHelperBody(cb).handle_resume;

  us.interpHelper = vwrap(cb, [] (Vout& v) {
    v << store{rarg(0), rvmtl()[rds::kVmpcOff]};
  });
  us.interpHelperSyncedPC = vwrap(cb, [&] (Vout& v) {
    storeVMRegs(v);
    v << ldimmb{1, rarg(1)};
    v << jmpi{body, RegSet(rarg(1))};
  });

  return us.resumeHelperRet;
}

TCA emitInterpOneCFHelper(CodeBlock& cb, Op op, ResumeHelper rh) {
  alignJmpTarget(cb);

  return vwrap(cb, [&] (Vout& v) {
    v << copy2{rvmfp(), rvmsp(), rarg(0), rarg(1)};
    // rarg(2) is set at the stub callsite.

    auto const handler = reinterpret_cast<TCA>(
      interpOneEntryPoints[static_cast<size_t>(op)]
    );
    v << call{handler, arg_regs(3)};

    auto const sf = v.makeReg();
    auto const next = v.makeBlock();

    v << testq{rret(), rret(), sf};
    v << jcci{CC_NZ, sf, next, rh.reenter_tc};
    v = next;
    v << jmpi{rh.handle_resume};
  });
}

void emitInterpOneCFHelpers(CodeBlock& cb, UniqueStubs& us) {
  alignJmpTarget(cb);
  auto const rh = resumeHelperBody(cb);

  auto const emit = [&] (Op op, const char* name) {
    auto const stub = emitInterpOneCFHelper(cb, op, rh);
    us.interpOneCFHelpers[op] = stub;
    us.add(name, stub);
  };

#define O(name, imm, in, out, flags)          \
  if (((flags) & CF) || ((flags) & TF)) {     \
    emit(Op::name, "interpOneCFHelper"#name); \
  }
  OPCODES
#undef O

  // Exit is a very special snowflake.  Because it can appear in PHP
  // expressions, the emitter pretends that it pushed a value on the eval stack
  // (and iopExit actually does push Null right before throwing).  Marking it
  // as TF would mess up any bytecodes that want to consume its output value,
  // so we can't do that.  But we also don't want to extend regions past it, so
  // the JIT treats it as terminal and uses InterpOneCF to execute it.
  emit(Op::Exit, "interpOneCFHelperExit");
}

///////////////////////////////////////////////////////////////////////////////

TCA emitDecRefGeneric(CodeBlock& cb) {
  return vwrap(cb, [] (Vout& v) {
    auto const rdata = rarg(0);
    auto const rtype = rarg(1);

    auto const destroy = [&] (Vout& v) {
      // decRefGeneric is called via callfaststub, whose ABI claims that all
      // registers are preserved.  This is true in the fast path, but in the
      // slow path we need to manually save caller-saved registers.
      auto const callerSaved = abi().gpUnreserved - abi().calleeSaved;
      PhysRegSaver prs{v, callerSaved, false /* aligned */};

      // As a consequence of being called via callfaststub, we can't safely use
      // any Vregs here except for status flags registers, at least not with
      // the default vwrap() ABI.  Just use the argument registers instead.
      assertx(callerSaved.contains(rdata));
      assertx(callerSaved.contains(rtype));

      v << movzbq{rtype, rtype};
      v << shrli{kShiftDataTypeToDestrIndex, rtype, rtype, v.makeReg()};

      auto const dtor_table =
        safe_cast<int>(reinterpret_cast<intptr_t>(g_destructors));
      v << callm{baseless(rtype * 8 + dtor_table), arg_regs(1)};
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
    };

    // TODO(#7728856): Pull this stuff out of x64 namespace; it's not actually
    // x64-dependent.
    x64::emitDecRefWork(v, v, rdata, destroy, false);
    v << ret{};
  });
}

///////////////////////////////////////////////////////////////////////////////

TCA emitThrowSwitchMode(CodeBlock& cb) {
  alignJmpTarget(cb);

  return vwrap(cb, [] (Vout& v) {
    v << call{TCA(throwSwitchMode)};
    v << ud2{};
  });
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void UniqueStubs::emitAll() {
  auto& main = mcg->code.main();
  auto& cold = mcg->code.cold();
  auto& frozen = mcg->code.frozen();

  auto const hot = [&]() -> CodeBlock& {
    auto& hot = const_cast<CodeBlock&>(mcg->code.hot());
    return hot.available() > 512 ? hot : main;
  };

#define ADD(name, stub) name = add(#name, (stub))
  ADD(funcPrologueRedispatch, emitFuncPrologueRedispatch(hot()));
  ADD(fcallHelperThunk,       emitFCallHelperThunk(cold));
  ADD(funcBodyHelperThunk,    emitFuncBodyHelperThunk(cold));
  ADD(functionEnterHelper,    emitFunctionEnterHelper(cold, *this));
  ADD(functionSurprisedOrStackOverflow,
      emitFunctionSurprisedOrStackOverflow(cold, *this));

  ADD(retHelper,                  emitInterpRet(cold));
  ADD(genRetHelper,               emitInterpGenRet<false>(cold));
  ADD(asyncGenRetHelper,          emitInterpGenRet<true>(cold));
  ADD(retInlHelper,               emitInterpRet(cold));
  ADD(debuggerRetHelper,          emitDebuggerInterpRet(cold));
  ADD(debuggerGenRetHelper,       emitDebuggerInterpGenRet<false>(cold));
  ADD(debuggerAsyncGenRetHelper,  emitDebuggerInterpGenRet<true>(cold));

  ADD(bindCallStub,           emitBindCallStub<false>(cold));
  ADD(immutableBindCallStub,  emitBindCallStub<true>(cold));
  ADD(fcallArrayHelper,       emitFCallArrayHelper(hot()));

  ADD(decRefGeneric,  emitDecRefGeneric(cold));

  ADD(callToExit,       emitCallToExit(main));
  ADD(endCatchHelper,   emitEndCatchHelper(frozen, *this));
  ADD(throwSwitchMode,  emitThrowSwitchMode(frozen));
#undef ADD

  add("freeLocalsHelpers",  emitFreeLocalsHelpers(hot(), *this));
  add("resumeInterpHelpers",  emitResumeInterpHelpers(main, *this));
  emitInterpOneCFHelpers(cold, *this);
}

///////////////////////////////////////////////////////////////////////////////

TCA UniqueStubs::add(const char* name, TCA start) {
  auto& cb = mcg->code.blockFor(start);
  auto const end = cb.frontier();

  FTRACE(1, "unique stub: {} @ {} -- {:4} bytes: {}\n",
         cb.name(),
         static_cast<void*>(start),
         static_cast<size_t>(end - start),
         name);

  ONTRACE(2,
          [&]{
            Disasm dasm(Disasm::Options().indent(4));
            std::ostringstream os;
            dasm.disasm(os, start, end);
            FTRACE(2, "{}\n", os.str());
          }()
         );

  mcg->recordGdbStub(cb, start, folly::sformat("HHVM::{}", name));

  auto const newStub = StubRange{name, start, end};
  auto lower = std::lower_bound(m_ranges.begin(), m_ranges.end(), newStub);

  // We assume ranges are non-overlapping.
  assertx(lower == m_ranges.end() || newStub.end <= lower->start);
  assertx(lower == m_ranges.begin() || (lower - 1)->end <= newStub.start);
  m_ranges.insert(lower, newStub);
  return start;
}

std::string UniqueStubs::describe(TCA address) {
  auto raw = [address] { return folly::sformat("{}", address); };
  if (m_ranges.empty()) return raw();

  auto const dummy = StubRange{"", address, nullptr};
  auto lower = std::upper_bound(m_ranges.begin(), m_ranges.end(), dummy);
  if (lower == m_ranges.begin()) return raw();

  --lower;
  if (lower->contains(address)) {
    return folly::sformat("{}+{:#x}", lower->name, address - lower->start);
  }
  return raw();
}

///////////////////////////////////////////////////////////////////////////////

RegSet interp_one_cf_regs() {
  return vm_regs_with_sp() | rarg(2);
}

///////////////////////////////////////////////////////////////////////////////

}}
