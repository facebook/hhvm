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
#ifndef incl_HPHP_JIT_UNIQUE_STUBS_H_
#define incl_HPHP_JIT_UNIQUE_STUBS_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/base/datatype.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////


constexpr int kNumFreeLocalsHelpers = 9;

/*
 * Addresses of various unique, long-lived JIT helper routines are
 * emitted when we first start up our code cache.
 */
struct UniqueStubs {
  /*
   * Stub that returns from this level VM-nesting to the previous one,
   * with whatever value is on the top of the stack.
   */
  TCA callToExit;

  /*
   * Returning from a function when the ActRec was pushed by the
   * interpreter.  The return IP on the ActRec will be set to one of
   * these stubs, so if someone tries to execute a return instruction,
   * we get a chance to set up state for a POST_INTERP_RET service
   * request.
   *
   * Generators need a different stub because the ActRec for a
   * generator is in the heap.
   */
  TCA retHelper;
  TCA genRetHelper;  // version for generators

  /*
   * Returning from a function where the ActRec was pushed by an
   * inlined call.  This is the same as retHelper but separated just
   * for debugability.
   */
  TCA retInlHelper;

  /*
   * Helpers used for restarting execution based on the value of PC,
   * after things like InterpOne of an instruction that changes PC.
   */
  TCA resumeHelperRet;
  TCA resumeHelper;

  /*
   * A helper routine for implementing the DefCls opcode.
   */
  TCA defClsHelper;

  /*
   * Helper stubs for doing generic decrefs on a function return.  The
   * stub is a partially-unrolled loop with kNumFreeLocalsHelpers
   * points to call to.  The freeManyLocalsHelper entry point should
   * be used when there's more locals than that.
   */
  TCA freeManyLocalsHelper;
  TCA freeLocalsHelpers[kNumFreeLocalsHelpers];

  /*
   * When we enter a func prologue based on a prediction of which
   * Func* we'll be calling, if the prediction was wrong we bail to
   * this stub to redispatch.
   */
  TCA funcPrologueRedispatch;

  /*
   * Utility routine that helps implement a fast path to avoid full VM
   * re-entry during translations of Op::FCallArray.
   */
  TCA fcallArrayHelper;

  /*
   * The stub we jump to when a stack overflow check fails.
   */
  TCA stackOverflowHelper;

  /*
   * A Func's prologue table is initialized to this stub for every entry. The
   * stub calls fcallHelper, which looks up or generates the appropriate
   * prologue and returns it. The stub then dispatches to the prologue.
   */
  TCA fcallHelperThunk;

  /*
   * A Func's "function body entry point" is initialized to this stub. The stub
   * calls funcBodyHelper, which creates a real translation. The stub then
   * dispatches to the translation.
   */
  TCA funcBodyHelperThunk;

  /*
   * Calls EventHook::onFunctionEnter, and handles the case where it requests
   * that we skip the function.
   */
  TCA functionEnterHelper;

  /*
   * Utility for logging stubs addresses during startup and registering the gdb
   * symbols. It's often useful to know where they were when debugging.
   */
  TCA add(const char* name, TCA start);

};

//////////////////////////////////////////////////////////////////////

}}

#endif
