/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unwind-x64.h"

#include <vector>
#include <memory>
#include <cxxabi.h>
#include <boost/mpl/identity.hpp>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unwind.h"
#include "hphp/util/abi-cxx.h"

// on cygwin in 64 bit/SEH adding frame information needs to be
// handled with rtladdfunctiontable and rtldeletefunctiontable
// or use the rtlinstallfunctiontablecallback
// register_frame and deregister_frame do not exist
// this is a temp solution that provides empty placeholders for linking
#ifdef __CYGWIN__
void __register_frame(const void*) {}
void __deregister_frame(const void*) {}
#else

// libgcc exports this for registering eh information for
// dynamically-loaded objects.  The pointer is to data in the format
// you find in a .eh_frame section.
extern "C" void __register_frame(const void*);
extern "C" void __deregister_frame(const void*);
#endif

TRACE_SET_MOD(unwind);

namespace HPHP { namespace jit {

rds::Link<UnwindRDS> unwindRdsInfo(rds::kInvalidHandle);

namespace {

size_t fdeIdx;

template<class T>
void append_vec(std::vector<char>& v,
                // Prevent template argument deduction:
                typename boost::mpl::identity<T>::type t) {
  size_t idx = v.size();
  v.resize(idx + sizeof(T));
  char* caddr = &v[idx];
  std::memcpy(caddr, &t, sizeof t);
}

void sync_regstate(_Unwind_Context* context) {
  assertx(tl_regState == VMRegState::DIRTY);

  uintptr_t frameRbp = _Unwind_GetGR(context, Debug::RBP);
  uintptr_t frameRip = _Unwind_GetIP(context);
  FTRACE(2, "syncing regstate for rbp: {:#x} rip: {:#x}\n", frameRbp, frameRip);

  /*
   * fixupWork expects to be looking at the first frame that is out of
   * the TC.  We have RBP/RIP for the TC frame that called out here,
   * so we make a fake ActRec here to give it what it expects.
   *
   * Note: this doesn't work for IndirectFixup situations.  However,
   * currently IndirectFixup is only used for destructors, which
   * aren't allowed to throw, so this is ok.
   */
  ActRec fakeAr;
  fakeAr.m_sfp = reinterpret_cast<ActRec*>(frameRbp);
  fakeAr.m_savedRip = frameRip;

  Stats::inc(Stats::TC_SyncUnwind);
  mcg->fixupMap().fixupWork(g_context.getNoCheck(), &fakeAr);
  tl_regState = VMRegState::CLEAN;
  FTRACE(2, "synced vmfp: {} vmsp: {} vmpc: {}\n", vmfp(), vmsp(), vmpc());
}

/*
 * Lookup a catch trace for the given TCA, returning nullptr if none was
 * found. Will abort if a nullptr catch trace was registered, meaning this call
 * isn't allowed to throw.
 */
TCA lookup_catch_trace(TCA rip, _Unwind_Exception* exn) {
  if (auto catchTraceOpt = mcg->getCatchTrace(rip)) {
    if (auto catchTrace = *catchTraceOpt) return catchTrace;

    // A few of our optimization passes must be aware of every path out of
    // the trace, so throwing through jitted code without a catch block is
    // very bad. This is indicated with a present but nullptr entry in the
    // catch trace map.
    const size_t kCallSize = 5;
    const uint8_t kCallOpcode = 0xe8;

    auto callAddr = rip - kCallSize;
    TCA helperAddr = nullptr;
    if (*callAddr == kCallOpcode) {
      helperAddr = rip + *reinterpret_cast<int32_t*>(callAddr + 1);
    }

    always_assert_flog(false,
                       "Translated call to {} threw '{}' without "
                       "catch block, return address: {}\n",
                       getNativeFunctionName(helperAddr),
                       typeInfoFromUnwindException(exn).name(),
                       rip);
  }

  return nullptr;
}

bool install_catch_trace(_Unwind_Context* ctx, _Unwind_Exception* exn,
                         bool do_side_exit, TypedValue unwinder_tv) {
  auto const rip = (TCA)_Unwind_GetIP(ctx);
  auto catchTrace = lookup_catch_trace(rip, exn);
  if (!catchTrace) {
    FTRACE(1, "No catch trace entry for ip {}; bailing\n", rip);
    return false;
  }

  FTRACE(1, "installing catch trace {} for call {} with tv {}, "
         "returning _URC_INSTALL_CONTEXT\n",
         catchTrace, rip, unwinder_tv.pretty());

  // If the catch trace isn't going to finish by calling _Unwind_Resume, we
  // consume the exception here. Otherwise, we leave a pointer to it in RDS so
  // endCatchHelper can pass it to _Unwind_Resume when it's done.
  if (do_side_exit) {
    unwindRdsInfo->exn = nullptr;
    __cxxabiv1::__cxa_begin_catch(exn);
    __cxxabiv1::__cxa_end_catch();
  } else {
    unwindRdsInfo->exn = exn;
  }

  // In theory the unwind api will let us set registers in the frame before
  // executing our landing pad. In practice, trying to use their recommended
  // scratch registers results in a SEGV inside _Unwind_SetGR, so we pass
  // things to the handler using the RDS. This also simplifies the handler code
  // because it doesn't have to worry about saving its arguments somewhere
  // while executing the exit trace.
  unwindRdsInfo->doSideExit = do_side_exit;
  if (do_side_exit) unwindRdsInfo->unwinderTv = unwinder_tv;
  _Unwind_SetIP(ctx, (uint64_t)catchTrace);
  tl_regState = VMRegState::DIRTY;

  return true;
}

void deregister_unwind_region(std::vector<char>* p) {
  std::auto_ptr<std::vector<char> > del(p);
  __deregister_frame(&(*p)[fdeIdx]);
}

}

_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context) {
  using namespace abi;
  // Exceptions thrown by g++-generated code will have the class "GNUCC++"
  // packed into a 64-bit int. libc++ has the class "CLNGC++". For now we
  // shouldn't be seeing exceptions from any other runtimes but this may
  // change in the future.
  DEBUG_ONLY constexpr uint64_t kMagicClass = 0x474e5543432b2b00;
  DEBUG_ONLY constexpr uint64_t kMagicDependentClass = 0x474e5543432b2b01;
  DEBUG_ONLY constexpr uint64_t kLLVMMagicClass = 0x434C4E47432B2B00;
  DEBUG_ONLY constexpr uint64_t kLLVMMagicDependentClass = 0x434C4E47432B2B01;
  assertx(exceptionClass == kMagicClass ||
          exceptionClass == kMagicDependentClass ||
          exceptionClass == kLLVMMagicClass ||
          exceptionClass == kLLVMMagicDependentClass);
  assertx(version == 1);

  auto const& ti = typeInfoFromUnwindException(exceptionObj);
  auto const std_exception = exceptionFromUnwindException(exceptionObj);
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
    int status;
    auto* exnType = __cxa_demangle(ti.name(), nullptr, nullptr, &status);
    SCOPE_EXIT { free(exnType); };
    assertx(status == 0);
    FTRACE(1, "unwind {} exn {}: regState: {} ip: {} type: {}\n",
           unwindType, exceptionObj,
           tl_regState == VMRegState::DIRTY ? "dirty" : "clean",
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

    if (install_catch_trace(context, exceptionObj, ism || tce, tv)) {
      always_assert((ism || tce) == bool(actions & _UA_HANDLER_FRAME));
      return _URC_INSTALL_CONTEXT;
    }

    always_assert(!(actions & _UA_HANDLER_FRAME));

    auto ip = TCA(_Unwind_GetIP(context));
    auto& stubs = mcg->tx().uniqueStubs;
    if (ip == stubs.endCatchHelperPast) {
      FTRACE(1, "rip == endCatchHelperPast, continuing unwind\n");
      return _URC_CONTINUE_UNWIND;
    }
    if (ip == stubs.functionEnterHelperReturn) {
      FTRACE(1, "rip == functionEnterHelperReturn, continuing unwind\n");
      return _URC_CONTINUE_UNWIND;
    }

    FTRACE(1, "unwinder hit normal TC frame, going to tc_unwind_resume\n");
    unwindRdsInfo->exn = exceptionObj;
    _Unwind_SetIP(context, uint64_t(stubs.endCatchHelper));
    return _URC_INSTALL_CONTEXT;
  }

  always_assert(!(actions & _UA_HANDLER_FRAME));

  FTRACE(1, "returning _URC_CONTINUE_UNWIND\n");
  return _URC_CONTINUE_UNWIND;
}

TCUnwindInfo tc_unwind_resume(ActRec* fp) {
  while (true) {
    auto const newFp = fp->m_sfp;
    ITRACE(1, "tc_unwind_resume processing fp: {} savedRip: {:#x} newFp: {}\n",
           fp, fp->m_savedRip, newFp);
    Trace::Indent indent;
    always_assert_flog(isVMFrame(fp),
                       "Unwinder got non-VM frame {} with saved rip {:#x}\n",
                       fp, fp->m_savedRip);

    // When we're unwinding through a TC frame (as opposed to stopping at a
    // handler frame), we need to make sure that if we later return from this
    // VM frame in translated code, we don't resume after the bindcall that may
    // be expecting things to still live in its spill space. If the return
    // address is in functionEnterHelper or callToExit, rVmFp won't contain a
    // real VM frame, so we skip those.
    auto savedRip = reinterpret_cast<TCA>(fp->m_savedRip);
    if (savedRip == mcg->tx().uniqueStubs.callToExit) {
      ITRACE(1, "top VM frame, passing back to _Unwind_Resume\n");
      return {nullptr, newFp};
    }

    auto catchTrace = lookup_catch_trace(savedRip, unwindRdsInfo->exn);
    if (isDebuggerReturnHelper(savedRip)) {
      // If this frame had its return address smashed by the debugger, the real
      // catch trace is saved in a side table.
      assertx(catchTrace == nullptr);
      catchTrace = popDebuggerCatch(fp);
    }
    unwindPreventReturnToTC(fp);
    if (fp->m_savedRip != reinterpret_cast<uint64_t>(savedRip)) {
      ITRACE(1, "Smashed m_savedRip of fp {} from {} to {:#x}\n",
             fp, savedRip, fp->m_savedRip);
    }

    fp = newFp;

    // If there's a catch trace for this block, return it. Otherwise, keep
    // going up the VM stack for this nesting level.
    if (catchTrace) {
      ITRACE(1, "tc_unwind_resume returning catch trace {} with fp: {}\n",
             catchTrace, fp);
      return {catchTrace, fp};
    }

    ITRACE(1, "No catch trace entry for {}; continuing\n",
           mcg->tx().uniqueStubs.describe(savedRip));
  }
}

///////////////////////////////////////////////////////////////////////////////

UnwindInfoHandle
register_unwind_region(unsigned char* startAddr, size_t size) {
  FTRACE(1, "register_unwind_region: base {}, size {}\n", startAddr, size);
  // The first time we're called, this will dynamically link the data
  // we need in the request data segment.  All future JIT translations
  // of catch traces may use offsets based on this handle.
  unwindRdsInfo.bind();

  std::unique_ptr<std::vector<char>> bufferMem(new std::vector<char>);
  std::vector<char>& buffer = *bufferMem;

  {
    // This is a dwarf CIE header.  Looks the same as a fde except the
    // second field is zero.
    append_vec<uint32_t>(buffer, 0); // Room for length later
    append_vec<int32_t>(buffer, 0);  // CIE_id
    append_vec<uint8_t>(buffer, 1);  // version

    /*
     * Null-terminated "augmentation string" (defines what the rest of
     * this thing is going to have.
     */
    append_vec<char>(buffer, 'z');
    append_vec<char>(buffer, 'P');
    append_vec<char>(buffer, '\0');

    // Code and data alignment.
    append_vec<uint8_t>(buffer, 1);
    append_vec<uint8_t>(buffer, 8); // Multiplies offsets below.

    // Return address column (in version 1, this is a single byte).
    append_vec<uint8_t>(buffer, Debug::RIP);

    // Length of the augmentation data.
    const size_t augIdx = buffer.size();
    append_vec<uint8_t>(buffer, 9);

    // Pointer to the personality routine for the TC.
    append_vec<uint8_t>(buffer, DW_EH_PE_absptr);
    append_vec<uintptr_t>(buffer, uintptr_t(tc_unwind_personality));

    // Fixup the augmentation data length field.  Note that it doesn't include
    // the space for itself.
    void* vp = &buffer[augIdx];
    *static_cast<uint8_t*>(vp) = buffer.size() - augIdx - sizeof(uint8_t);

    /*
     * Define a program for the CIE.  This explains to the unwinder
     * how to figure out where the frame pointer was, etc.
     *
     * Arguments to some of these are encoded in LEB128, so we have to
     * clear the high bit for the signed values.
     */
    // Previous FP (CFA) is at rbp + 16.
    append_vec<uint8_t>(buffer, DW_CFA_def_cfa);
    append_vec<uint8_t>(buffer, Debug::RBP);
    append_vec<uint8_t>(buffer, 16);
    // rip is at CFA - 1 * data_align.
    append_vec<uint8_t>(buffer, DW_CFA_offset_extended_sf);
    append_vec<uint8_t>(buffer, Debug::RIP);
    append_vec<uint8_t>(buffer, -1u & 0x7f);
    // rbp is at CFA - 2 * data_align.
    append_vec<uint8_t>(buffer, DW_CFA_offset_extended_sf);
    append_vec<uint8_t>(buffer, Debug::RBP);
    append_vec<uint8_t>(buffer, -2u & 0x7f);
    /*
     * Leave rsp unchanged.
     *
     * Note that some things in the translator do actually change rsp,
     * but we assume they cannot throw so this is ok.  If rVmSp ever
     * changes to use rsp this code must change.
     */
    append_vec<uint8_t>(buffer, DW_CFA_same_value);
    append_vec<uint8_t>(buffer, Debug::RSP);

    // Fixup the length field.  Note that it doesn't include the space
    // for itself.
    vp = &buffer[0];
    *static_cast<uint32_t*>(vp) = buffer.size() - sizeof(uint32_t);
  }
  fdeIdx = buffer.size();
  {
    // Reserve space for FDE length.
    append_vec<uint32_t>(buffer, 0);

    // Negative offset to the CIE for this FDE---the offset is
    // relative to this field.
    append_vec<int32_t>(buffer, int32_t(buffer.size()));

    // We're using the addressing mode DW_EH_PE_absptr, which means it
    // wants a 8 byte pointer and a 8 byte size indicating the region
    // this FDE applies to.
    append_vec<unsigned char*>(buffer, startAddr);
    append_vec<size_t>(buffer, size);

    // Length of the augmentation data in this FDE. This field must present if
    // 'z' is set in CIE.
    append_vec<uint8_t>(buffer, 0);

    // Fixup the length field for this FDE.  Again length doesn't
    // include the length field itself.
    void* vp = &buffer[fdeIdx];
    *static_cast<uint32_t*>(vp) = buffer.size() - fdeIdx - sizeof(uint32_t);
  }
  // Add one more zero'd length field---this indicates that there are
  // no more FDEs sharing this CIE.
  append_vec<uint32_t>(buffer, 0);

  __register_frame(&buffer[fdeIdx]);

  return std::shared_ptr<std::vector<char>>(
    bufferMem.release(),
    deregister_unwind_region
  );
}

//////////////////////////////////////////////////////////////////////

}}
