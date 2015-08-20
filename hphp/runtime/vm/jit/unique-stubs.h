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
#ifndef incl_HPHP_JIT_UNIQUE_STUBS_H_
#define incl_HPHP_JIT_UNIQUE_STUBS_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

constexpr int kNumFreeLocalsHelpers = 9;

/*
 * Addresses of various unique, long-lived JIT helper routines.
 *
 * The global set of unique stubs is emitted when we initialize the TC.
 */
struct UniqueStubs {

  /////////////////////////////////////////////////////////////////////////////
  // Function entry.

  /*
   * Dynamically dispatch to the appropriate func prologue, when the called
   * function fails the prologue's func guard.
   *
   * @see: emitFuncGuard()
   */
  TCA funcPrologueRedispatch;

  /*
   * Look up or emit a func prologue and jump to it---or, failing that, call
   * native routines that do the same work.
   *
   * All entries in the prologue tables of all Funcs are initialized to this
   * stub, so that we can lazily generate their prologues.  If codegen for the
   * prologue succeeds, we update the prologue table to point to the new
   * prologue instead of this stub.
   */
  TCA fcallHelperThunk;

  /*
   * Look up or emit a func body entry point and jump to it---or, failing that,
   * fall back to the interpreter.
   *
   * This func body is just the translation for Func::base(), for functions
   * with no DV init funclets.  For functions with DV funclets, the func body
   * first dispatches to any necessary funclets before jumping to the base()
   * translation.
   *
   * @see: MCGenerator::getFuncBody()
   */
  TCA funcBodyHelperThunk;

  /*
   * Call EventHook::onFunctionEnter() and handle the case where it requests
   * that we skip the function.
   *
   * functionEnterHelperReturn is used by unwinder code that needs to detect
   * calls made from this stub.
   */
  TCA functionEnterHelper;
  TCA functionEnterHelperReturn;

  /*
   * Handle either a surprise condition or a stack overflow.
   *
   * Also gracefully handles spurious wake-ups that result from racing with a
   * background thread clearing surprise flags.
   */
  TCA functionSurprisedOrStackOverflow;


  /////////////////////////////////////////////////////////////////////////////
  // Function return.

  /*
   * Return from a function when the ActRec was pushed by the interpreter.
   *
   * The return IP on the ActRec will be set to one of these stubs, so if
   * someone tries to execute a return instruction, we get a chance to set up
   * state for a POST_INTERP_RET service request.
   *
   * Generators need a different stub because the ActRec for a generator is in
   * the heap.
   */
  TCA retHelper;
  TCA genRetHelper;       // version for generator
  TCA asyncGenRetHelper;  // version for async generators

  /*
   * Return from a function when the ActRec was pushed by an inlined call.
   *
   * This is the same as retHelper, but is kept separate to aid in debugging.
   */
  TCA retInlHelper;

  /*
   * Return from a function when the ActRec was called from jitted code but
   * had its m_savedRip smashed by the debugger.
   *
   * These stubs call a helper that looks up the original catch trace from the
   * call, executes it, then executes a REQ_POST_DEBUGGER_RET.
   */
  TCA debuggerRetHelper;
  TCA debuggerGenRetHelper;
  TCA debuggerAsyncGenRetHelper;


  /////////////////////////////////////////////////////////////////////////////
  // Function calls.

  /*
   * Bindcall stubs for immutable/non-immutable calls.
   */
  TCA bindCallStub;
  TCA immutableBindCallStub;

  /*
   * Utility routine that helps implement a fast path to avoid full VM re-entry
   * during translations of Op::FCallArray.
   */
  TCA fcallArrayHelper;


  /////////////////////////////////////////////////////////////////////////////
  // Interpreter stubs.

  /*
   * Restart execution based on the value of vmpc after an instruction (e.g.,
   * InterpOne) that changes vmpc.
   *
   * These expect that all VM registers are synced.
   */
  TCA resumeHelperRet;
  TCA resumeHelper;

  /*
   * Like resumeHelper, but interpret a basic block first to ensure we make
   * forward progress.
   *
   * interpHelper expects the correct value of vmpc to be in the first argument
   * register and syncs it, whereas interpHelperSyncedPC expects vmpc to be
   * synced a priori.  Both stubs will sync the vmsp and vmfp registers to
   * vmRegs before interpreting.
   */
  TCA interpHelper;
  TCA interpHelperSyncedPC;

  /*
   * Stubs for each bytecode with the CF flag, which InterpOne the bytecode and
   * then call resumeHelper.
   *
   * These stubs expect rvmfp() and rvmsp() to be live, and rAsm to contain the
   * offset to the bytecode to interpret.
   *
   * TODO(#6730846): Use an architecture-independent argument register API.
   */
  std::unordered_map<Op, TCA> interpOneCFHelpers;


  /////////////////////////////////////////////////////////////////////////////
  // DecRefs.

  /*
   * Expensive, generic decref of a value with an unknown (but known to be
   * refcounted) DataType.
   *
   * The value to be decref'd should be in the first two argument registers
   * (data, type).  All GP registers are saved around the destructor call.
   */
  TCA decRefGeneric;

  /*
   * Perform generic decrefs of locals on function return.
   *
   * Each freeLocalHelpers[i] is an entry point to a partially-unrolled loop.
   * freeManyLocalsHelper should be used instead when there are more than
   * kNumFreeLocalsHelpers locals.
   */
  TCA freeLocalsHelpers[kNumFreeLocalsHelpers];
  TCA freeManyLocalsHelper;


  /////////////////////////////////////////////////////////////////////////////
  // Other stubs.

  /*
   * Return from this VM nesting level to the previous one, with whatever value
   * is on the top of the stack.
   */
  TCA callToExit;

  /*
   * Perform dispatch at the end of a catch block.
   *
   * If the unwinder has set state indicating a return address to jump to, we
   * load vmfp and vmsp and jump there.  Otherwise, we call _Unwind_Resume.
   */
  TCA endCatchHelper;
  TCA endCatchHelperPast;

  /*
   * Throw a VMSwitchMode exception.  Used in switchModeForDebugger().
   */
  TCA throwSwitchMode;


  /////////////////////////////////////////////////////////////////////////////

  /*
   * Emit one of every unique stub.
   */
  void emitAll();

  /*
   * Utility for logging stubs addresses during startup and registering the gdb
   * symbols. It's often useful to know where they were when debugging.
   *
   * TODO(#6730846): Kill this once we have one emitter per unique stub (or
   * close to it).
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

///////////////////////////////////////////////////////////////////////////////

}}

#endif
