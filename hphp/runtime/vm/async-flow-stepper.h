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

#ifndef incl_HPHP_ASYNC_FLOW_STEPPER_H_
#define incl_HPHP_ASYNC_FLOW_STEPPER_H_

#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/pc-filter.h"

namespace HPHP {

enum class AsyncStepperStage {
  Disabled,       // Not enabled.
  StepOver,       // Normal step over did not counter "await" opcode yet.
  StepOverAwait,  // In the middle of step over "await" opcode.
  WaitResume,     // Wait for resume internal breakpoint to hit.
};

// Indicate if AsyncFlowStepper has handled this opcode or not.
enum class AsyncStepHandleOpcodeResult {
  Unhandled,  // Not handled, other logic should handle it.
  Handled,    // Handled, other stepping logic should not handle it.
  Completed,  // Async stepping completes.
};

/**
 * This stepper is responsible for doing logic async
 * stepping over "await" opcode.
 * Caller should call setup() before stepping
 * and pass each step opcode to handleOpcode().
 */
struct AsyncFlowStepper {
public:
  void setup();
  AsyncStepHandleOpcodeResult handleOpcode(PC pc);
  void handleExceptionThrown();
  bool handleExceptionHandler();

private:
  void handleBlockedAwaitOpcode(PC pc);
  bool isActRecOnAsyncStack(const ActRec* target);
  void stepOverAwaitOpcode();
  void captureResumeIdAfterAwait();
  void setResumeInternalBreakpoint(PC pc);
  bool didResumeBreakpointTrigger(PC pc);
  const ActRec* getAsyncResumableId(const ActRec* fp);
  void updateStepStartStackDepth();
  bool isCompleted(PC pc);
  void reset();

private:
  AsyncStepperStage m_stage{AsyncStepperStage::Disabled};
  // ActRec address that issued current async operation
  // We used it to check if the completed async operation
  // is the one we are current stepping.
  ActRec* m_asyncResumableId{nullptr};
  int m_stepStartStackDepth{0};
  // Whether there is an exception thrown for current
  // stepped async operation.
  bool m_isCurrentAsyncStepException{false};

  PCFilter m_stepRangeFlowFilter;
  PCFilter m_awaitOpcodeBreakpointFilter;
  PCFilter m_resumeBreakpointFilter;
};

}

#endif
