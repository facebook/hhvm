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

#include "hphp/runtime/vm/hhbc.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

namespace HPHP {

struct SrcKey;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

constexpr int kNumFreeLocalsHelpers = 8;

/*
 * Addresses of various unique, long-lived JIT helper routines.
 *
 * The global set of unique stubs is emitted when we initialize the TC.
 */
struct UniqueStubs {
  /*
   * Unique stubs and stack alignment.
   *
   * A lot of complex control flow occurs amongst the various unique stubs, as
   * well as between the unique stubs and the TC.  This is obviously important
   * to understanding the control flow of jitted code in general, but is also
   * critical to performance-correct implementations of the unique stubs on
   * platforms that care about stack alignment.
   *
   * On x64, for example, the calling convention is that the stack is 16-byte
   * aligned before calls, which allows the compiler to safely use aligned movs
   * for, e.g., saving callee-saved XMM registers---after pushing the old RIP
   * (from the call) and %rbp (in the callee) which puts the stack back into
   * alignment.
   *
   * We make the following assumption about stack alignment:
   *
   *    - Stack alignment is a parity condition; i.e., a valid stack pointer is
   *      only ever aligned or one-off from being aligned.  This is the case on
   *      x64.  Platforms with no alignment constraints also satisfy this
   *      assumption, but platforms with higher-order alignments will have a
   *      bad time without big changes to the system.
   *
   * Additionally, we maintain the following alignment invariants:
   *
   *    - When executing the body of any PHP function in the TC, the native
   *      stack is aligned.  (This need not hold in the func prologue until the
   *      EnterFrame IR instruction is executed.)
   *
   *    - The native stack is aligned on every call to a native function from
   *      anywhere in the TC.
   *
   * Notably, we do /not/ unconditionally maintain stack alignment in the
   * bodies of the unique stubs.  For x64, which relies on the native calling
   * convention (pushing %rbp at the beginning of each frame) to maintain
   * alignment, this means we need to know how we reached a given stub, and
   * from where, in order to determine whether we need to manually realign the
   * stack when we do native calls or return to the TC.
   *
   * How each stub is reached is documented below.  Whether the stack is
   * aligned on entry to the stub is also documented---these are x64 specific
   * (since a `call' instruction on x64 unbalances the stack until the %rbp
   * push, or a manual alignment operation simulating it).
   */

  /////////////////////////////////////////////////////////////////////////////
  // Function entry.

  /*
   * Dynamically dispatch to the appropriate func prologue, when the called
   * function fails the prologue's func guard.
   *
   * @reached:  jmp from func guard
   * @aligned:  false (func guards are pre-EnterFrame, hence unaligned)
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
   *
   * @reached:  bindcall from TC; or
   *            jmp from bindCallStub; or
   *            jmp from funcPrologueRedispatch
   *            (same callsites as func prologues)
   * @aligned:  false
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
   * @reached:  call from enterTCHelper
   *            jmp from fcallArrayHelper
   * @aligned:  true (we ensure alignment when entering the TC, and
   *            fcallArrayHelper performs the EnterFrame of func prologues)
   */
  TCA funcBodyHelperThunk;

  /*
   * Call EventHook::onFunctionEnter() and handle the case where it requests
   * that we skip the function.
   *
   * functionEnterHelperReturn is only used by unwinder code to detect calls
   * made from this stub.
   *
   * @reached:  vinvoke from TC (func prologue after EnterFrame)
   * @aligned:  false
   */
  TCA functionEnterHelper;
  TCA functionEnterHelperReturn;

  /*
   * Handle either a surprise condition or a stack overflow.
   *
   * Also gracefully handles spurious wake-ups that result from racing with a
   * background thread clearing surprise flags.
   *
   * @reached:  vinvoke from TC (func prologue after EnterFrame)
   * @aligned:  false
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
   *
   * @reached:  ret from TC
   * @aligned:  true
   */
  TCA retHelper;
  TCA genRetHelper;       // version for generator
  TCA asyncGenRetHelper;  // version for async generators

  /*
   * Return from a function when the ActRec was pushed by an inlined call.
   *
   * This is the same as retHelper, but is kept separate to aid in debugging.
   *
   * @reached:  ret from TC
   * @aligned:  true
   */
  TCA retInlHelper;

  /*
   * Return from a function when the ActRec was called from jitted code but
   * had its m_savedRip smashed by the debugger.
   *
   * These stubs call a helper that looks up the original catch trace from the
   * call, executes it, then executes a REQ_POST_DEBUGGER_RET.
   *
   * @reached:  ret from TC
   * @aligned:  true
   */
  TCA debuggerRetHelper;
  TCA debuggerGenRetHelper;
  TCA debuggerAsyncGenRetHelper;


  /////////////////////////////////////////////////////////////////////////////
  // Function calls.

  /*
   * Bindcall stubs for immutable/non-immutable calls.
   *
   * @reached:  bindcall from TC
   * @aligned:  false
   */
  TCA bindCallStub;
  TCA immutableBindCallStub;

  /*
   * Use interpreter functions to enter the pre-live ActRec that we place on
   * the stack (along with the Array of parameters) in a CallArray instruction.
   *
   * @reached:  vcallarray from TC
   * @aligned:  false
   */
  TCA fcallArrayHelper;


  /////////////////////////////////////////////////////////////////////////////
  // Interpreter stubs.

  /*
   * Restart execution based on the value of vmpc.  Used, e.g., to resume
   * execution after an InterpOne.
   *
   * Expects that all VM registers are synced.
   *
   * @reached:  jmp from funcBodyHelperThunk, call from enterTCHelper
   * @aligned:  true
   */
  TCA resumeHelper;

  /*
   * Like resumeHelper, but specifically for an interpreted FCall.
   *
   * @reached:  jmp from fcallHelperThunk
   * @aligned:  false
   */
  TCA resumeHelperRet;

  /*
   * Like resumeHelper, but interpret a basic block first to ensure we make
   * forward progress.
   *
   * interpHelper expects the correct value of vmpc to be in the first argument
   * register and syncs it, whereas interpHelperSyncedPC expects vmpc to be
   * synced a priori.  Both stubs will sync the vmsp and vmfp registers to
   * vmRegs before passing control to the interpreter.
   *
   * @reached:  jmp from TC
   * @aligned:  true
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
   * @reached:  jmp from TC
   * @aligned:  true
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
   *
   * @reached:  callfaststub from TC
   * @aligned:  false
   */
  TCA decRefGeneric;

  /*
   * Perform generic decrefs of locals on function return.
   *
   * Each freeLocalHelpers[i] is an entry point to a partially-unrolled loop.
   * freeManyLocalsHelper should be used instead when there are more than
   * kNumFreeLocalsHelpers locals.
   *
   * These helpers expect the address of the frame's last local variable (which
   * has the lowest address) to be passed in the second argument register.  The
   * first argument register is ignored.
   *
   * @reached:  vcall from TC
   * @aligned:  false
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
   * symbols.  It's often useful to know where they were when debugging.
   */
  TCA add(const char* name, TCA start);

  /*
   * If the given address is within one of the registered stubs, return a
   * string indicating which stub and how far in it is: "retHelper+0xfa".
   *
   * Otherwise, return a string representation of the raw address: "0xabcdef".
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

/*
 * Registers that are live on entry to an interpOneCFHelper.
 */
RegSet interp_one_cf_regs();

/*
 * Emit code to `v' which jumps to interpHelper with the proper arguments.
 */
void emitInterpReq(Vout& v, SrcKey sk, FPInvOffset spOff);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
