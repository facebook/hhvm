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
#ifndef incl_HPHP_JIT_UNIQUE_STUBS_H_
#define incl_HPHP_JIT_UNIQUE_STUBS_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

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
   * Returning from a function when the ActRec was pushed by the interpreter.
   * The return IP on the ActRec will be set to one of these stubs, so if
   * someone tries to execute a return instruction, we get a chance to set up
   * state for a POST_INTERP_RET service request.
   *
   * Generators need a different stub because the ActRec for a generator is in
   * the heap.
   */
  TCA retHelper;
  TCA genRetHelper;  // version for generators
  TCA asyncGenRetHelper;  // version for async generators


  /*
   * Returning from a function when the ActRec was called from jitted code but
   * had its m_savedRip smashed by the debugger. These stubs call a helper that
   * looks up the original catch trace from the call, executes it, then executes
   * a REQ_POST_DEBUGGER_RET.
   */
  TCA debuggerRetHelper;
  TCA debuggerGenRetHelper;
  TCA debuggerAsyncGenRetHelper;

  /*
   * Returning from a function where the ActRec was pushed by an
   * inlined call.  This is the same as retHelper but separated just
   * for debugability.
   */
  TCA retInlHelper;

  /*
   * Helpers used for restarting execution based on the value of PC, after
   * things like InterpOne of an instruction that changes PC. Assumes all VM
   * regs are synced.
   */
  TCA resumeHelperRet;
  TCA resumeHelper;

  /*
   * Like resumeHelper, but interpret a basic block first to ensure we make
   * forward progress. interpHelper expects the correct value of vmpc to be in
   * the first argument register, and interpHelperSyncedPC expects vmpc to
   * already be synced. Both stubs will sync the sp and fp registers to vmRegs
   * before interpreting.
   */
  TCA interpHelper;
  TCA interpHelperSyncedPC;

  /*
   * For every bytecode with the CF flag, a stub will exist here to interpOne
   * that bytecode, followed by a call to resumeHelper. The stubs expect rVmFp
   * and rVmSp to be live, and rAsm must contain the offset to the bytecode to
   * interpret.
   */
  std::unordered_map<Op, TCA> interpOneCFHelpers;

  /*
   * Throw a VMSwitchMode exception. Used in
   * bytecode.cpp:switchModeForDebugger().
   */
  TCA throwSwitchMode;

  /*
   * Catch blocks jump to endCatchHelper when they've finished executing. If
   * the unwinder has set state indicating a return address to jump to, this
   * stub will load vmfp and vmsp and jump there. Otherwise, it calls
   * _Unwind_Resume.
   */
  TCA endCatchHelper;
  TCA endCatchHelperPast;

  /*
   * Helper stubs for doing generic decrefs on a function return.  The
   * stub is a partially-unrolled loop with kNumFreeLocalsHelpers
   * points to call to.  The freeManyLocalsHelper entry point should
   * be used when there's more locals than that.
   */
  TCA freeManyLocalsHelper;
  TCA freeLocalsHelpers[kNumFreeLocalsHelpers];

  /*
   * A cold, expensive stub to DecRef a value with an unknown (but known to be
   * refcounted) DataType. It saves all GP registers around the destructor
   * call. The value should be in the first two argument registers (data,
   * type).
   */
  TCA genDecRefHelper;

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
   * that we skip the function. functionEnterHelperReturn is used by unwinder
   * code that needs to detect calls made from this stub.
   */
  TCA functionEnterHelper;
  TCA functionEnterHelperReturn;

  /*
   * Unique stub that is called when a JIT'd prologue has detected /either/ a
   * surprise condition or a stack overflow.
   */
  TCA functionSurprisedOrStackOverflow;

  /*
   * BindCall stubs for immutable/non-immutable calls
   */
  TCA bindCallStub;
  TCA immutableBindCallStub;

  /*
   * Utility for logging stubs addresses during startup and registering the gdb
   * symbols. It's often useful to know where they were when debugging.
   */
  TCA add(const char* name, TCA start);

  /*
   * If the given address is within one of the registered stubs, return a
   * string indicating which stub and how far in it is:
   * "fcallArrayHelper+0xfa". Otherwise, return a string representation of the
   * raw address: "0xabcdef".
   */
  std::string describe(TCA addr);

 private:
  struct StubRange {
    std::string name;
    TCA start, end;

    bool operator<(const StubRange& other) const {
      return start < other.start;
    }

    bool contains(TCA address) const {
      return start <= address && address < end;
    }
  };

  std::vector<StubRange> m_ranges;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
