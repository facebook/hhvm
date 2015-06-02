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

#ifndef incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_
#define incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_

#include <cstdlib>
#include <sstream>
#include <string>
#include <unwind.h>
#include <memory>
#include <exception>
#include <typeinfo>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/tread-hash-map.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/assertions.h"

namespace HPHP {
struct ActRec;

namespace jit {

//////////////////////////////////////////////////////////////////////

typedef TreadHashMap<CTCA, TCA, ctca_identity_hash> CatchTraceMap;

/*
 * Information the unwinder needs stored in RDS, and the rds::Link for
 * it.  Used to pass values between unwinder code and catch traces.
 */
struct UnwindRDS {
  /* When a cleanup (non-side-exiting) catch trace is executing, this will
   * point to the currently propagating exception, to be passed to
   * _Unwind_Resume at the end of cleanup. */
  _Unwind_Exception* exn;

  /* Some helpers need to signal an error along with a TypedValue to be pushed
   * on the eval stack. When present, that value lives here. */
  TypedValue unwinderTv;

  /* When returning from a frame that had its m_savedRip smashed by the
   * debugger, the return stub stashes values here to be used after running the
   * appropriate catch trace. In addition, a non-nullptr debuggerReturnSP is
   * used as the flag to endCatchHelper that it should perform a
   * REQ_POST_DEBUGGER_RET rather than resuming the unwind process. */
  TypedValue* debuggerReturnSP;
  Offset debuggerReturnOff;

  /* This will be true iff the currently executing catch trace should side exit
   * to somewhere else in the TC, rather than resuming the unwind process. */
  bool doSideExit;
};
extern rds::Link<UnwindRDS> unwindRdsInfo;

inline ptrdiff_t unwinderExnOff() {
  return unwindRdsInfo.handle() + offsetof(UnwindRDS, exn);
}

inline ptrdiff_t unwinderSideExitOff() {
  return unwindRdsInfo.handle() + offsetof(UnwindRDS, doSideExit);
}

inline ptrdiff_t unwinderTvOff() {
  return unwindRdsInfo.handle() + offsetof(UnwindRDS, unwinderTv);
}

inline ptrdiff_t unwinderDebuggerReturnOffOff() {
  return unwindRdsInfo.handle() + offsetof(UnwindRDS, debuggerReturnOff);
}

inline ptrdiff_t unwinderDebuggerReturnSPOff() {
  return unwindRdsInfo.handle() + offsetof(UnwindRDS, debuggerReturnSP);
}

//////////////////////////////////////////////////////////////////////

/*
 * Meant to work like __cxxabiv1::__is_dependent_exception
 * See libstdc++-v3/libsupc++/unwind-cxx.h in GCC
 */
static inline bool isDependentException(uint64_t c)
{
  return (c & 1);
}

/**
 * Meant to work like __cxxabiv1::__get_object_from_ue() but with a specific
 * return type.
 * See libstdc++-v3/libsupc++/unwind-cxx.h in GCC
 */
inline std::exception* exceptionFromUnwindException(
  _Unwind_Exception* exceptionObj)
{
  constexpr size_t sizeOfDependentException = 112;
  if (isDependentException(exceptionObj->exception_class)) {
    return *reinterpret_cast<std::exception**>(
      reinterpret_cast<char*>(exceptionObj + 1) - sizeOfDependentException);
  } else {
    return reinterpret_cast<std::exception*>(exceptionObj + 1);
  }
}

inline const std::type_info& typeInfoFromUnwindException(
  _Unwind_Exception* exceptionObj
  )
{
  if (isDependentException(exceptionObj->exception_class)) {
    // like __cxxabiv1::__get_refcounted_exception_header_from_obj()
    constexpr size_t sizeOfRefcountedException = 128;
    char * obj = reinterpret_cast<char*>(
        exceptionFromUnwindException(exceptionObj));
    char * header = obj - sizeOfRefcountedException;
    // Dereference the exc field, the type_info* is the first field inside that
    constexpr size_t excOffset = 16;
    return *reinterpret_cast<std::type_info*>(header + excOffset);
  } else {
    // like __cxxabiv1::__get_exception_header_from_ue()
    constexpr size_t sizeOfCxaException = 112;
    return **reinterpret_cast<std::type_info**>(
      reinterpret_cast<char*>(exceptionObj + 1) - sizeOfCxaException);
  }
}

/*
 * Called whenever we create a new translation cache for the whole
 * region of code.
 */
typedef std::shared_ptr<void> UnwindInfoHandle;
UnwindInfoHandle register_unwind_region(unsigned char* address, size_t size);

/*
 * The personality routine for code emitted by the jit.
 */
_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context);

struct TCUnwindInfo {
  TCA catchTrace;
  ActRec* fp;
};
TCUnwindInfo tc_unwind_resume(ActRec* fp);

//////////////////////////////////////////////////////////////////////

}}

#endif
