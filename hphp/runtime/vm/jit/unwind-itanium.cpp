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

#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/unwind.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/arch.h"
#include "hphp/util/assertions.h"
#include "hphp/util/dwarf-reg.h"
#include "hphp/util/eh-frame.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"
#include "hphp/util/unwind-itanium.h"

#include <folly/ScopeGuard.h>

#include <memory>
#include <typeinfo>

#include <cxxabi.h>
#include <unwind.h>

TRACE_SET_MOD(unwind);

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

rds::Link<UnwindRDS,rds::Mode::Normal> g_unwind_rds;

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Sync VM regs for the TC frame represented by `context'.
 */
void sync_regstate(TCA ip, _Unwind_Context* context) {
  assertx(regState() == VMRegState::DIRTY);
  Stats::inc(Stats::TC_SyncUnwind);

  auto const frame = [&] () -> VMFrame {
    // `actRec` points to the leaf frame of the VM.
    auto const actRec =
      reinterpret_cast<ActRec*>(_Unwind_GetGR(context, dw_reg::FP));
    if (isVMFrame(actRec)) {
      auto const cfa = _Unwind_GetCFA(context);
      return {actRec, TCA(ip), cfa};
    }

    // `actRec` points to a synthetic frame in a stub. The next frame is the
    // first true VM frame. It is safe to deduce the CFA from the rbp, as VM
    // frames do not perform CFA padding.
    auto const cfa = uintptr_t(actRec) + kNativeFrameSize;
    return {actRec->m_sfp, TCA(actRec->m_savedRip), cfa};
  }();

  FTRACE(2, "syncing regstate for: cfa {}, sfp {}, ip {}\n",
         (void*)frame.m_prevCfa, (void*)frame.m_actRec, (void*)frame.m_rip);
  always_assert(FixupMap::processFixupForVMFrame(frame));

  regState() = VMRegState::CLEAN;
  FTRACE(2, "synced vmfp {}, vmsp {}, vmpc {}\n", vmfp(), vmsp(), vmpc());
}

/*
 * Look up a catch trace for `rip', returning nullptr if none was found.
 *
 * A present-but-nullptr catch trace indicates that a call is explicitly not
 * allowed to throw.  A few of our optimization passes must be aware of every
 * path out of a region, so throwing through jitted code without a catch block
 * is very bad---and we abort in this case.
 */
TCA lookup_catch_trace(TCA rip) {
  if (auto catchTraceOpt = getCatchTrace(rip)) {
    if (auto catchTrace = *catchTraceOpt) return catchTrace;

    // FIXME: This assumes that smashable calls and regular calls look the
    // same, which is probably not true on non-x64 platforms.
    auto const target = smashableCallTarget(smashableCallFromRet(rip));

    always_assert_flog(
      false, "Translated call to {} threw '{}' without catch block; "
             "return address: {}\n",
      getNativeFunctionName(target),
      current_exception_name(),
      rip
    );
  }

  return nullptr;
}

/*
 * Look up the catch trace for the return address in `ctx', and install it by
 * updating the unwind RDS info, as well as the IP in `ctx'.
 */
void install_catch_trace(_Unwind_Context* ctx, TCA rip) {
  auto catchTrace = lookup_catch_trace(rip);
  if (!catchTrace) {
    FTRACE(1, "no catch trace entry for ip {}; installing default catch trace "
              "and going to tc_unwind_resume\n", rip);
    _Unwind_SetIP(ctx, uint64_t(tc::ustubs().endCatchHelper));
    return;
  }

  FTRACE(1, "installing catch trace {} for call {} with tv {}, "
         "returning _URC_INSTALL_CONTEXT\n",
         catchTrace, rip, g_unwind_rds->tv.pretty());
  _Unwind_SetIP(ctx, (uint64_t)catchTrace);
  regState() = VMRegState::DIRTY;
}

///////////////////////////////////////////////////////////////////////////////

}

_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exn_cls,
                      _Unwind_Exception* ue,
                      _Unwind_Context* context) {
  // Exceptions thrown by g++-generated code will have the class "GNUCC++"
  // packed into a 64-bit int. libc++ has the class "CLNGC++". For now we
  // shouldn't be seeing exceptions from any other runtimes but this may
  // change in the future.
  DEBUG_ONLY constexpr uint64_t kMagicClass = 0x474e5543432b2b00;
  DEBUG_ONLY constexpr uint64_t kMagicDependentClass = 0x474e5543432b2b01;
  DEBUG_ONLY constexpr uint64_t kLLVMMagicClass = 0x434C4E47432B2B00;
  DEBUG_ONLY constexpr uint64_t kLLVMMagicDependentClass = 0x434C4E47432B2B01;
  assertx(exn_cls == kMagicClass ||
          exn_cls == kMagicDependentClass ||
          exn_cls == kLLVMMagicClass ||
          exn_cls == kLLVMMagicDependentClass);
  assertx(version == 1);

  auto const& ti = typeinfoFromUE(ue);

  if (Trace::moduleEnabled(TRACEMOD, 1)) {
    DEBUG_ONLY auto const* unwindType =
      (actions & _UA_SEARCH_PHASE) ? "search" : "cleanup";
    int status;
    auto* exnType = abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status);
    SCOPE_EXIT { free(exnType); };
    assertx(status == 0);
    DEBUG_ONLY auto const fp =
      reinterpret_cast<ActRec*>(_Unwind_GetGR(context, dw_reg::FP));
    FTRACE(1, "unwind {} exn {}: regState {}, ip {}, type {}, {} {}\n",
           unwindType, ue,
           regState() == VMRegState::DIRTY ? "dirty" :
           regState() == VMRegState::CLEAN ? "clean" : "guarded",
           (TCA)_Unwind_GetIP(context), exnType,
           isVMFrame(fp) ? fp->func()->fullName()->data() : "non-vm", fp);
  }

  auto ip = TCA(_Unwind_GetIP(context));
  if (ip == tc::ustubs().endCatchHelperPast) {
    FTRACE(1, "rip == endCatchHelperPast, continuing unwind\n");
    // Use search phase to indicate that this is the first time we enter the
    // personality after running __cxa_rethrow()
    if (actions & _UA_SEARCH_PHASE) __cxxabiv1::__cxa_end_catch();
    return _URC_CONTINUE_UNWIND;
  }

  /*
   * We don't do anything during the search phase---before attempting cleanup,
   * we want all deeper frames to have run their object destructors (which can
   * have side effects like setting regState()) and spilled any values they
   * may have been holding in callee-saved regs.
   */
  if (actions & _UA_SEARCH_PHASE) return _URC_HANDLER_FOUND;

  /*
   * During the cleanup phase, we can either use a landing pad to perform
   * cleanup (with _Unwind_SetIP and _URC_INSTALL_CONTEXT), or we can do it
   * here. We sync the VM registers here, then optionally use a landing pad,
   * which is an exit trace from hhir with a few special instructions.
   */
  assertx(actions & _UA_CLEANUP_PHASE && actions & _UA_HANDLER_FRAME);
  auto exn = exceptionFromUE(ue);
  auto const ism = ti != typeid(InvalidSetMException) ? nullptr :
    static_cast<InvalidSetMException*>(exn);
  __cxxabiv1::__cxa_begin_catch(ue);
  if (regState() == VMRegState::DIRTY) sync_regstate(ip, context);
  g_unwind_rds->exn = [&]() -> Either<ObjectData*, Exception*> {
    if (ti == typeid(Object)) return static_cast<Object*>(exn)->get();
    if (ti == typeid(req::root<Object>)) {
      return static_cast<req::root<Object>*>(exn)->get();
    }
    if (!ism) return static_cast<Exception*>(exn);
    return nullptr;
  }();
  assertx((g_unwind_rds->exn.left() &&
           g_unwind_rds->exn.left()->kindIsValid()) ||
          g_unwind_rds->exn.right() ||
          (g_unwind_rds->exn.isNull() && ism));
  g_unwind_rds->isFirstFrame = true;

  // In theory, the unwind API will let us set registers in the frame before
  // executing our landing pad. In practice, trying to use their recommended
  // scratch registers results in a SEGV inside _Unwind_SetGR, so we pass
  // things to the handler using the RDS. This also simplifies the handler code
  // because it doesn't have to worry about saving its arguments somewhere
  // while executing the exit trace.
  g_unwind_rds->doSideExit = ism;
  g_unwind_rds->tv = ism ? ism->tv() : TypedValue{};

  if (ism) {
    // If the catch trace is going to side exit, we won't have an opportunity
    // to consume the exception later, so consume it here.
    assertx(lookup_catch_trace(ip));
    __cxxabiv1::__cxa_end_catch();
  } else if (g_unwind_rds->exn.left()) {
    // We consume Hack exceptions early and convert them to the situation
    // equivalent to a side entry. This unifies the logic for both situations
    // and gives us flexibility to handle Hack exceptions from catch blocks.
    g_unwind_rds->exn.left()->incRefCount();
    __cxxabiv1::__cxa_end_catch();
  }


  // If we have a catch trace at the IP in the frame given by `context',
  // install it otherwise install the default catch trace.
  install_catch_trace(context, ip);
  return _URC_INSTALL_CONTEXT;
}

TCUnwindInfo tc_unwind_resume(ActRec* fp, bool teardown) {
  while (true) {
    auto const sfp = fp->m_sfp;

    ITRACE(1, "tc_unwind_resume: fp {}, saved rip {:#x}, saved fp {},"
              " teardown {}\n",
           fp, fp->m_savedRip, sfp, teardown);
    Trace::Indent indent;

    // We should only ever be unwinding VM or unique stub frames.
    always_assert_flog(isVMFrame(fp),
                       "Unwinder got non-VM frame {} with saved rip {:#x}\n",
                       fp, fp->m_savedRip);

    auto savedRip = reinterpret_cast<TCA>(fp->m_savedRip);

    regState() = VMRegState::CLEAN;
    if (!g_unwind_rds->exn.isNull() && !g_unwind_rds->isFirstFrame) {
      lockObjectWhileUnwinding(vmpc(), vmStack());
    }
    g_unwind_rds->isFirstFrame = false;

    if (isVMFrame(fp)) {
      ITRACE(2, "fp {} {}, sfp {} {}\n",
        fp, fp->func()->fullName(), sfp,
        isVMFrame(sfp, true) ? sfp->func()->fullName()->data() : "[non-vm]");

      // Unwind vm stack to sfp
      if (!g_unwind_rds->exn.isNull()) {
        auto phpException = g_unwind_rds->exn.left();
        auto const result = unwindVM(g_unwind_rds->exn, sfp, teardown);
        if (!(result & UnwindReachedGoal)) {
          assertx(phpException);
          phpException->decReleaseCheck();
          g_unwind_rds->doSideExit = true;

          if (result & UnwindFSWH) {
            auto const vmfp_ = vmfp();
            if (!vmfp_ || vmfp_ == sfp) {
              g_unwind_rds->savedRip = savedRip;
              ITRACE(3, "vmsp() is {}\n", vmsp());
              regState() = VMRegState::DIRTY;
              auto const ret = g_unwind_rds->fswh
                ? tc::ustubs().unwinderAsyncRet
                : tc::ustubs().unwinderAsyncNullRet;
              ITRACE(1, "tc_unwind_resume returning to {} with fp {}\n",
                        tc::ustubs().describe(ret), sfp);
              return {ret, sfp};
            }
            // we haven't fully unwound but we need to resume execution since
            // we have a FSWH on our hand
            ITRACE(2, "Got FSWH but vmfp: {}, sfp: {}\n", vmfp_, sfp);
          }
          regState() = VMRegState::DIRTY;
          FTRACE(1, "Resuming from resumeHelper with fp {}\n", fp);
          return {tc::ustubs().resumeHelperFromInterp, fp};
        }
      }
    }

    if (savedRip == tc::ustubs().callToExit) {
      // If we're the top VM frame, there's nothing we need to do; we can just
      // let the native C++ unwinder take over.
      if (g_unwind_rds->exn.left()) {
        ITRACE(1, "top VM frame, Hack exception, enter itanium unwinder "
                  "by throwing it\n");
        return {tc::ustubs().throwExceptionWhileUnwinding, sfp};
      }

      ITRACE(1, "top VM frame, passing back to _Unwind_Resume\n");
      return {nullptr, sfp};
    }

    auto catchTrace = lookup_catch_trace(savedRip);

    fp = sfp;

    // If there's a catch trace for this block, return it.  Otherwise, keep
    // going up the VM stack for this nesting level.
    if (catchTrace) {
      ITRACE(1, "tc_unwind_resume returning catch trace {} with fp: {}\n",
             catchTrace, fp);
      return {catchTrace, fp};
    }

    ITRACE(1, "No catch trace entry for {}; continuing\n",
           tc::ustubs().describe(savedRip));
    teardown = true;
  }
}

TCUnwindInfo tc_unwind_resume_stublogue(ActRec* fp, TCA savedRip) {
  // We can't consume the current `fp' when unwinding from a stublogue context,
  // as that would skip the Catch of the instruction that called the stublogue.
  // Instead, try to use the savedRip from stublogue to continue unwinding the
  // same logical VM frame, but in the context before entering this stub.
  ITRACE(1, "tc_unwind_resume_stublogue: fp {}, saved rip {}\n",
         fp, savedRip);
  auto catchTrace = lookup_catch_trace(savedRip);
  if (catchTrace) {
    ITRACE(1,
           "tc_unwind_resume_stublogue returning catch trace {} with fp: {}\n",
           catchTrace, fp);
    return {catchTrace, fp};
  }

  // The caller of this stublogue doesn't have a catch trace. Continue with
  // tc_unwind_resume().
  return tc_unwind_resume(fp, true);
}

///////////////////////////////////////////////////////////////////////////////

namespace {
/*
 * Write a CIE for the TC using `ehfw'.
 *
 * This sets tc_unwind_personality() as the personality routine, and includes
 * basic instructions to the unwinder for rematerializing the call frame
 * registers.
 */
void write_tc_cie(EHFrameWriter& ehfw, PersonalityFunc personality) {
  ehfw.begin_cie(dw_reg::IP, reinterpret_cast<const void*>(personality));

  // The part of the ActRec that mirrors the native frame record is the part
  // below ActRec::m_funcId, which at a minimum includes the saved frame pointer
  // and the return address.
  constexpr auto record_size = kNativeFrameSize;
  ehfw.def_cfa(dw_reg::FP, record_size);

  // The following calculation is related to the "top" of the record.  There is
  // an implicit -8 factor as defined by EHFrameWriter::m_cie.data_align.

  switch (arch()) {
    case Arch::ARM:
    case Arch::X64:
      ehfw.offset_extended_sf(dw_reg::IP,
                              (record_size - AROFF(m_savedRip)) / 8);
      break;
  }

  ehfw.offset_extended_sf(dw_reg::FP, (record_size - AROFF(m_sfp)) / 8);

  // It's not actually the case that %rsp keeps the same value across all TC
  // frames---in particular, we use the native stack for spill space.  However,
  // the responsibility of restoring it properly falls to TC catch traces, so
  // this .eh_frame entry need only preserve it.
  ehfw.same_value(dw_reg::SP);

  ehfw.end_cie();
}

// Handles to registered .eh_frame sections.
std::vector<EHFrameDesc> s_ehFrames;
}

void initUnwinder(TCA base, size_t size, PersonalityFunc personality) {
  EHFrameWriter ehfw;
  write_tc_cie(ehfw, personality);
  ehfw.begin_fde(base);
  ehfw.end_fde(size);
  ehfw.null_fde();
  s_ehFrames.push_back(ehfw.register_and_release());
}

///////////////////////////////////////////////////////////////////////////////

}
