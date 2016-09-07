/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/abi-cxx.h"
#include "hphp/util/assertions.h"
#include "hphp/util/dwarf-reg.h"
#include "hphp/util/eh-frame.h"
#include "hphp/util/trace.h"
#include "hphp/util/unwind-itanium.h"

#include <folly/ScopeGuard.h>

#include <memory>
#include <typeinfo>

#ifndef _MSC_VER
#include <cxxabi.h>
#include <unwind.h>
#endif

TRACE_SET_MOD(unwind);

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

rds::Link<UnwindRDS,true> g_unwind_rds(rds::kInvalidHandle);

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Sync VM regs for the TC frame represented by `context'.
 */
void sync_regstate(_Unwind_Context* context) {
  assertx(tl_regState == VMRegState::DIRTY);

  uintptr_t fp = _Unwind_GetGR(context, dw_reg::FP);
  uintptr_t ip = _Unwind_GetIP(context);
  FTRACE(2, "syncing regstate for: fp {:#x}, ip {:#x}\n", fp, ip);

  // fixupWork() takes an `ar' argument and syncs VM regs for the first TC
  // frame it finds in the call chain for `ar'.  We can make it sync for the fp
  // and rip in `context' by putting them in a fake ActRec here on the native
  // stack, and passing a pointer to it.
  //
  // NB: This doesn't work for IndirectFixup situations.  However, currently
  // IndirectFixup is only used for destructors, which aren't allowed to throw,
  // so this is ok.
  ActRec fakeAR;
  fakeAR.m_sfp = reinterpret_cast<ActRec*>(fp);
  fakeAR.m_savedRip = ip;

  Stats::inc(Stats::TC_SyncUnwind);
  FixupMap::fixupWork(g_context.getNoCheck(), &fakeAR);
  tl_regState = VMRegState::CLEAN;
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
TCA lookup_catch_trace(TCA rip, _Unwind_Exception* exn) {
  if (auto catchTraceOpt = getCatchTrace(rip)) {
    if (auto catchTrace = *catchTraceOpt) return catchTrace;

    // FIXME: This assumes that smashable calls and regular calls look the
    // same, which is probably not true on non-x64 platforms.
    auto const target = smashableCallTarget(smashableCallFromRet(rip));

    always_assert_flog(
      false, "Translated call to {} threw '{}' without catch block; "
             "return address: {}\n",
      getNativeFunctionName(target), typeinfoFromUE(exn).name(), rip
    );
  }

  return nullptr;
}

/*
 * Look up the catch trace for the return address in `ctx', and install it by
 * updating the unwind RDS info, as well as the IP in `ctx'.
 */
bool install_catch_trace(_Unwind_Context* ctx, _Unwind_Exception* exn,
                         bool do_side_exit, TypedValue unwinder_tv) {
  auto const rip = (TCA)_Unwind_GetIP(ctx);
  auto catchTrace = lookup_catch_trace(rip, exn);
  if (!catchTrace) {
    FTRACE(1, "no catch trace entry for ip {}; bailing\n", rip);
    return false;
  }

  FTRACE(1, "installing catch trace {} for call {} with tv {}, "
         "returning _URC_INSTALL_CONTEXT\n",
         catchTrace, rip, unwinder_tv.pretty());

  // If the catch trace isn't going to finish by calling _Unwind_Resume, we
  // consume the exception here. Otherwise, we leave a pointer to it in RDS so
  // endCatchHelper can pass it to _Unwind_Resume when it's done.
  //
  // In theory, the unwind API will let us set registers in the frame before
  // executing our landing pad. In practice, trying to use their recommended
  // scratch registers results in a SEGV inside _Unwind_SetGR, so we pass
  // things to the handler using the RDS. This also simplifies the handler code
  // because it doesn't have to worry about saving its arguments somewhere
  // while executing the exit trace.
  assert(g_unwind_rds.isInit());
  if (do_side_exit) {
    g_unwind_rds->exn = nullptr;
#ifndef _MSC_VER
    __cxxabiv1::__cxa_begin_catch(exn);
    __cxxabiv1::__cxa_end_catch();
#endif
    g_unwind_rds->tv = unwinder_tv;
  } else {
    g_unwind_rds->exn = exn;
  }
  g_unwind_rds->doSideExit = do_side_exit;

  _Unwind_SetIP(ctx, (uint64_t)catchTrace);
  tl_regState = VMRegState::DIRTY;

  return true;
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
  auto const std_exception = exceptionFromUE(ue);
  InvalidSetMException* ism = nullptr;
  TVCoercionException* tce = nullptr;
  if (ti == typeid(InvalidSetMException)) {
    ism = static_cast<InvalidSetMException*>(std_exception);
  } else if (ti == typeid(TVCoercionException)) {
    tce = static_cast<TVCoercionException*>(std_exception);
  }

  if (Trace::moduleEnabled(TRACEMOD, 1)) {
    DEBUG_ONLY auto const* unwindType =
      (actions & _UA_SEARCH_PHASE) ? "search" : "cleanup";
#ifndef _MSC_VER
    int status;
    auto* exnType = abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status);
    SCOPE_EXIT { free(exnType); };
    assertx(status == 0);
#else
    auto* exnType = ti.name();
#endif
    FTRACE(1, "unwind {} exn {}: regState {}, ip {}, type {}\n",
           unwindType, ue,
           tl_regState == VMRegState::DIRTY ? "dirty" :
           tl_regState == VMRegState::CLEAN ? "clean" : "guarded",
           (TCA)_Unwind_GetIP(context), exnType);
  }

  /*
   * We don't do anything during the search phase---before attempting cleanup,
   * we want all deeper frames to have run their object destructors (which can
   * have side effects like setting tl_regState) and spilled any values they
   * may have been holding in callee-saved regs.
   */
  if (actions & _UA_SEARCH_PHASE) {
    if (ism) {
      FTRACE(1, "thrown value: {} returning _URC_HANDLER_FOUND\n ",
             ism->tv().pretty());
      return _URC_HANDLER_FOUND;
    }
    if (tce) {
      FTRACE(1, "TVCoercionException thrown, returning _URC_HANDLER_FOUND\n");
      return _URC_HANDLER_FOUND;
    }
  }

  /*
   * During the cleanup phase, we can either use a landing pad to perform
   * cleanup (with _Unwind_SetIP and _URC_INSTALL_CONTEXT), or we can do it
   * here. We sync the VM registers here, then optionally use a landing pad,
   * which is an exit trace from hhir with a few special instructions.
   */
  else if (actions & _UA_CLEANUP_PHASE) {
    TypedValue tv = ism ? ism->tv() : tce ? tce->tv() : TypedValue();
    if (tl_regState == VMRegState::DIRTY) {
      sync_regstate(context);
    }

    // If we have a catch trace at the IP in the frame given by `context',
    // install it.
    if (install_catch_trace(context, ue, ism || tce, tv)) {
      // Note that we should always have a catch trace for the special runtime
      // helper exceptions above.
      always_assert((ism || tce) == bool(actions & _UA_HANDLER_FRAME));
      return _URC_INSTALL_CONTEXT;
    }
    always_assert(!(actions & _UA_HANDLER_FRAME));

    auto const ip = TCA(_Unwind_GetIP(context));

    assert(g_unwind_rds.isInit());

    auto& stubs = tc::ustubs();
    if (ip == stubs.endCatchHelperPast) {
      FTRACE(1, "rip == endCatchHelperPast, continuing unwind\n");
      return _URC_CONTINUE_UNWIND;
    }

    FTRACE(1, "unwinder hit normal TC frame, going to tc_unwind_resume\n");
    g_unwind_rds->exn = ue;
    _Unwind_SetIP(context, uint64_t(stubs.endCatchHelper));
    return _URC_INSTALL_CONTEXT;
  }

  always_assert(!(actions & _UA_HANDLER_FRAME));

  FTRACE(1, "returning _URC_CONTINUE_UNWIND\n");
  return _URC_CONTINUE_UNWIND;
}

TCUnwindInfo tc_unwind_resume(ActRec* fp) {
  while (true) {
    auto const sfp = fp->m_sfp;

    ITRACE(1, "tc_unwind_resume: fp {}, saved rip {:#x}, saved fp {}\n",
           fp, fp->m_savedRip, sfp);
    Trace::Indent indent;

    // We should only ever be unwinding VM or unique stub frames.
    always_assert_flog(isVMFrame(fp),
                       "Unwinder got non-VM frame {} with saved rip {:#x}\n",
                       fp, fp->m_savedRip);
    auto savedRip = reinterpret_cast<TCA>(fp->m_savedRip);

    if (savedRip == tc::ustubs().callToExit) {
      // If we're the top VM frame, there's nothing we need to do; we can just
      // let the native C++ unwinder take over.
      ITRACE(1, "top VM frame, passing back to _Unwind_Resume\n");
      return {nullptr, sfp};
    }

    // When we're unwinding through a TC frame (as opposed to stopping at a
    // handler frame, or unwinding through a stub frame), we need to make sure
    // that if we later return from this VM frame in translated code, we don't
    // resume after the PHP call that may be expecting things to still live in
    // its spill space.
    //
    // (Note that we can't do this if we're in the top VM frame, since it's not
    // actually an ActRec, so it's actually required that we skip it above).
    unwindPreventReturnToTC(fp);

    assert(g_unwind_rds.isInit());
    auto catchTrace = lookup_catch_trace(savedRip, g_unwind_rds->exn);

    if (isDebuggerReturnHelper(savedRip)) {
      // If this frame had its return address smashed by the debugger, the real
      // catch trace is saved in a side table.
      assertx(catchTrace == nullptr);
      catchTrace = unstashDebuggerCatch(fp);
    }

    if (fp->m_savedRip != reinterpret_cast<uint64_t>(savedRip)) {
      ITRACE(1, "Smashed m_savedRip of fp {} from {} to {:#x}\n",
             fp, savedRip, fp->m_savedRip);
    }

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
  }
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
void write_tc_cie(EHFrameWriter& ehfw) {
  ehfw.begin_cie(dw_reg::IP,
                 reinterpret_cast<const void*>(tc_unwind_personality));

  // The part of the ActRec that mirrors the native frame record is the part
  // below ActRec::m_func, which at a minimum includes the saved frame pointer
  // and the return address.
  constexpr auto record_size = AROFF(m_func);
  ehfw.def_cfa(dw_reg::FP, record_size);

  // The following calculation is related to the "top" of the record.  There is
  // an implicit -8 factor as defined by EHFrameWriter::m_cie.data_align.
  ehfw.offset_extended_sf(dw_reg::IP, (record_size - AROFF(m_savedRip)) / 8);
  ehfw.offset_extended_sf(dw_reg::FP, (record_size - AROFF(m_sfp)) / 8);

#if defined(__powerpc64__)
  ehfw.offset_extended_sf(dw_reg::TOC, (record_size - AROFF(m_savedToc)) / 8);
#endif

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

void initUnwinder(TCA base, size_t size) {
  EHFrameWriter ehfw;
  write_tc_cie(ehfw);
  ehfw.begin_fde(base);
  ehfw.end_fde(size);
  ehfw.null_fde();
  s_ehFrames.push_back(ehfw.register_and_release());
}

///////////////////////////////////////////////////////////////////////////////

}}
