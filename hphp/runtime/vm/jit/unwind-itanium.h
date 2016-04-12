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

#ifndef incl_HPHP_JIT_UNWIND_ITANIUM_H_
#define incl_HPHP_JIT_UNWIND_ITANIUM_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/asm-x64.h"

#include <cstddef>

#ifndef _MSC_VER
#include <unwind.h>
#else
#include "hphp/util/unwind-itanium-msvc.h"
#endif

namespace HPHP {

struct ActRec;
struct EHFrameWriter;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Information the unwinder needs stored in RDS, and the rds::Link for it.
 * Used to pass values between unwinder code and catch traces.
 */
struct UnwindRDS {
  /* When a cleanup (non-side-exiting) catch trace is executing, this will
   * point to the currently propagating exception, to be passed to
   * _Unwind_Resume at the end of cleanup. */
  _Unwind_Exception* exn;

  /* Some helpers need to signal an error along with a TypedValue to be pushed
   * on the eval stack. When present, that value lives here. */
  TypedValue tv;

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
extern rds::Link<UnwindRDS> g_unwind_rds;

#define IMPLEMENT_OFF(Name, member)                             \
  inline ptrdiff_t unwinder##Name##Off() {                      \
    return g_unwind_rds.handle() + offsetof(UnwindRDS, member); \
  }
IMPLEMENT_OFF(Exn, exn)
IMPLEMENT_OFF(TV, tv)
IMPLEMENT_OFF(SideExit, doSideExit)
IMPLEMENT_OFF(DebuggerReturnOff, debuggerReturnOff)
IMPLEMENT_OFF(DebuggerReturnSP, debuggerReturnSP)
#undef IMPLEMENT_OFF

///////////////////////////////////////////////////////////////////////////////

/*
 * The personality routine for code emitted by the jit.
 */
_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context);

/*
 * Resume unwinding of jitted PHP frames.
 *
 * This is called from the endCatchHelper stub, which is hit at the end of
 * every catch trace.  It returns the new value of vmfp(), as well as the catch
 * trace to jump to---or nullptr if there is none, in which case the native
 * unwinder should be invoked via _Unwind_Resume().
 */
struct TCUnwindInfo {
  TCA catchTrace;
  ActRec* fp;
};
TCUnwindInfo tc_unwind_resume(ActRec* fp);

/*
 * Write a CIE for the TC using `ehfw'.
 *
 * This sets tc_unwind_personality() as the personality routine, and includes
 * basic instructions to the unwinder for rematerializing the call frame
 * registers.
 */
void write_tc_cie(EHFrameWriter& ehfw);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
