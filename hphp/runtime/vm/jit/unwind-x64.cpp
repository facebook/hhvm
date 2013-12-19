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

#include "hphp/runtime/vm/jit/unwind-x64.h"

#include <vector>
#include <memory>
#include <cxxabi.h>
#include <boost/mpl/identity.hpp>

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/member-operations.h"

// libgcc exports this for registering eh information for
// dynamically-loaded objects.  The pointer is to data in the format
// you find in a .eh_frame section.
extern "C" void __register_frame(void*);
extern "C" void __deregister_frame(void*);

TRACE_SET_MOD(unwind);

namespace HPHP { namespace JIT {

RDS::Link<UnwindRDS> unwindRdsInfo(RDS::kInvalidHandle);

namespace {

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
  assert(tl_regState == VMRegState::DIRTY);

  uintptr_t frameRbp = _Unwind_GetGR(context, Debug::RBP);
  uintptr_t frameRip = _Unwind_GetGR(context, Debug::RIP);

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
  fakeAr.m_savedRbp = frameRbp;
  fakeAr.m_savedRip = frameRip;

  Stats::inc(Stats::TC_SyncUnwind);
  tx64->fixupMap().fixupWork(g_vmContext, &fakeAr);
  tl_regState = VMRegState::CLEAN;
}

bool install_catch_trace(_Unwind_Context* ctx, _Unwind_Exception* exn,
                         InvalidSetMException* ism) {
  const CTCA rip = (CTCA)_Unwind_GetIP(ctx);
  TCA catchTrace = tx64->getCatchTrace(rip);
  assert(IMPLIES(ism, catchTrace));
  if (!catchTrace) return false;

  FTRACE(1, "installing catch trace {} for call {} with ism {}, "
         "returning _URC_INSTALL_CONTEXT\n",
         catchTrace, rip, ism);

  // In theory the unwind api will let us set registers in the frame
  // before executing our landing pad. In practice, trying to use
  // their recommended scratch registers results in a SEGV inside
  // _Unwind_SetGR, so we pass things to the handler using the
  // RDS. This also simplifies the handler code because it doesn't
  // have to worry about saving its arguments somewhere while
  // executing the exit trace.
  unwindRdsInfo->unwinderScratch = (int64_t)exn;
  unwindRdsInfo->doSideExit = ism;
  if (ism) {
    unwindRdsInfo->unwinderTv = ism->tv();
  }
  _Unwind_SetIP(ctx, (uint64_t)catchTrace);
  tl_regState = VMRegState::DIRTY;

  return true;
}

_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context) {
  using namespace abi;
  // Exceptions thrown by g++-generated code will have the class "GNUCC++"
  // packed into a 64-bit int. For now we shouldn't be seeing exceptions from
  // any other runtimes but this may change in the future.
  DEBUG_ONLY constexpr uint64_t kMagicClass = 0x474e5543432b2b00;
  assert(exceptionClass == kMagicClass);
  assert(version == 1);

  auto const& ti = typeInfoFromUnwindException(exceptionObj);
  InvalidSetMException* ism = nullptr;
  if (ti == typeid(InvalidSetMException)) {
    ism = static_cast<InvalidSetMException*>(
      exceptionFromUnwindException(exceptionObj));
  }


  if (Trace::moduleEnabled(TRACEMOD, 1)) {
    DEBUG_ONLY auto const* unwindType =
      (actions & _UA_SEARCH_PHASE) ? "search" : "cleanup";
    int status;
    auto* exnType = __cxa_demangle(ti.name(), nullptr, nullptr, &status);
    SCOPE_EXIT { free(exnType); };
    assert(status == 0);
    FTRACE(1, "unwind {} exn {}: regState: {} ip: {} type: {}. ",
           unwindType, exceptionObj,
           tl_regState == VMRegState::DIRTY ? "dirty" : "clean",
           (TCA)_Unwind_GetIP(context), exnType);
  }

  /*
   * We don't do anything during the search phase---before attempting
   * cleanup, we want all deeper frames to have run their object
   * destructors (which can have side effects like setting
   * tl_regState) and spilled any values they may have been holding in
   * callee-saved regs.
   */
  if (actions & _UA_SEARCH_PHASE) {
    if (ism) {
      FTRACE(1, "thrown value: {} returning _URC_HANDLER_FOUND\n ",
             ism->tv().pretty());
      return _URC_HANDLER_FOUND;
    }
  }

  /*
   * During the cleanup phase, we can either use a landing pad to perform
   * cleanup (with _Unwind_SetIP and _URC_INSTALL_CONTEXT), or we can do it
   * here. We sync the VM registers here, then optionally use a landing pad,
   * which is an exit traces from hhir with a few special instructions.
   */
  else if (actions & _UA_CLEANUP_PHASE) {
    if (tl_regState == VMRegState::DIRTY) {
      sync_regstate(context);
    }
    if (install_catch_trace(context, exceptionObj, ism)) {
      return _URC_INSTALL_CONTEXT;
    }
  }

  FTRACE(1, "returning _URC_CONTINUE_UNWIND\n");
  return _URC_CONTINUE_UNWIND;
}

void deregister_unwind_region(std::vector<char>* p) {
  std::auto_ptr<std::vector<char> > del(p);
  __deregister_frame(&(*p)[0]);
}

}

///////////////////////////////////////////////////////////////////////////////

UnwindInfoHandle
register_unwind_region(unsigned char* startAddr, size_t size) {
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
     *
     * TODO: probably we should have a 'z' field to indicate length.
     */
    append_vec<char>(buffer, 'P');
    append_vec<char>(buffer, '\0');

    // Code and data alignment.
    append_vec<uint8_t>(buffer, 1);
    append_vec<uint8_t>(buffer, 8); // Multiplies offsets below.

    // Return address column (in version 1, this is a single byte).
    append_vec<uint8_t>(buffer, Debug::RIP);

    // Pointer to the personality routine for the TC.
    append_vec<uint8_t>(buffer, DW_EH_PE_absptr);
    append_vec<uintptr_t>(buffer, uintptr_t(tc_unwind_personality));

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
    void* vp = &buffer[0];
    *static_cast<uint32_t*>(vp) = buffer.size() - sizeof(uint32_t);
  }
  const size_t fdeIdx = buffer.size();
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

    // Fixup the length field for this FDE.  Again length doesn't
    // include the length field itself.
    void* vp = &buffer[fdeIdx];
    *static_cast<uint32_t*>(vp) = buffer.size() - fdeIdx - sizeof(uint32_t);
  }
  // Add one more zero'd length field---this indicates that there are
  // no more FDEs sharing this CIE.
  append_vec<uint32_t>(buffer, 0);

  __register_frame(&buffer[0]);

  return std::shared_ptr<std::vector<char> >(
    bufferMem.release(),
    deregister_unwind_region
  );
}

//////////////////////////////////////////////////////////////////////

}}
