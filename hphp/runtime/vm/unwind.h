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
#ifndef incl_HPHP_UNWIND_H_
#define incl_HPHP_UNWIND_H_

#include <stdexcept>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Enumerates actions that should be taken by the enterVM loop after
 * unwinding an exception.
 */
enum class UnwindAction {
  /*
   * The exception was not handled in this nesting of the VM---it
   * needs to be rethrown.
   */
  Propagate,

  /*
   * A catch or fault handler was identified and the VM state has been
   * prepared for entry to it.
   */
  ResumeVM,

  /**
   * An exception thrown from an eagerly executed async function was
   * wrapped into a StaticExceptionWaitHandle. The async function was
   * running in the top frame, so we need to return from the VM instance.
   */
  Return,
};

/*
 * The main entry point to the unwinder.
 *
 * When an exception propagates up to the top-level try/catch in
 * enterVM, it calls to this module to perform stack unwinding as
 * appropriate.  This function must be called from within the catch
 * handler (it rethrows the exception to determine what to do).
 *
 * The returned UnwindAction instructs enterVM on how to proceed.
 */
UnwindAction exception_handler() noexcept;

//////////////////////////////////////////////////////////////////////

/*
 * This exception is thrown when executing an Unwind bytecode, which
 * will reraise the current fault and resume propagating it.
 */
struct VMPrepareUnwind : std::exception {
  const char* what() const throw() { return "VMPrepareUnwind"; }
};

/*
 * Thrown when we need to "switch modes" by re-starting the current VM
 * invocation.  For example, if we need to break for the debugger, or
 * enable code coverage mode.
 */
struct VMSwitchMode : std::exception {
  const char* what() const throw() { return "VMSwitchMode"; }
};

/*
 * Thrown for stack overflow thrown from a prolog while
 * re-entering
 */
struct VMReenterStackOverflow : std::exception {
  const char* what() const throw() { return "VMReenterStackOverflow"; }
};

/*
 * Same as VMSwitchMode, except for use from a builtin---the frame for
 * the builtin function should be unwound before resuming the VM.
 */
struct VMSwitchModeBuiltin : std::exception {
  const char* what() const throw() { return "VMSwitchModeBuiltin"; }
};

//////////////////////////////////////////////////////////////////////

}

#endif
