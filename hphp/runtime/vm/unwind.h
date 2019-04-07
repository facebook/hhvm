/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Find a catch exception handler for a given raise location if the handler was
 * found or InvalidAbsoluteOffset.
 */
Offset findCatchHandler(const Func* func, Offset raiseOffset);

/*
 * Unwind the PHP exception.
 */
void unwindPhp(ObjectData* phpException);

/*
 * Unwind the C++ exception.
 */
void unwindCpp(Exception* cppException);

/*
 * Unwind the frame for a builtin.  Currently only used when switching modes
 * for hphpd_break, and fb_enable_code_coverage.
 */
void unwindBuiltinFrame();

/*
 * The main entry point to the unwinder.
 *
 * Wraps action in try/catch and executes appropriate unwinding logic based
 * on the type of thrown exception.
 *
 * If the exception was not handled in this nesting of the VM, it will be
 * rethrown. Otherwise, either a catch or fault handler was identified and
 * the VM state has been prepared for entry to it, or end of execution was
 * reached and vmpc() will be zero.
 */
template<class Action> void exception_handler(Action action);

/*
 * top and prev must implement Throwable. Walk the chain of top's previous
 * pointers, finding the first unset one. If there is a cycle in either top or
 * prev's previous chains, do nothing. Otherwise, add prev to the end of top's
 * previous chain.
 *
 * Either way, this function takes ownership of one existing reference to prev.
 */
void chainFaultObjects(ObjectData* top, ObjectData* prev);

//////////////////////////////////////////////////////////////////////

/*
 * Thrown when we need to "switch modes" by re-starting the current VM
 * invocation.  For example, if we need to break for the debugger, or
 * enable code coverage mode.
 */
struct VMSwitchMode : BaseException {
  const char* what() const noexcept override { return "VMSwitchMode"; }
};

/*
 * Thrown for stack overflow in a jitted prologue.
 */
struct VMStackOverflow : BaseException {
  const char* what() const noexcept override {
    return "VMStackOverflow";
  }
};

/*
 * Same as VMSwitchMode, except for use from a builtin---the frame for
 * the builtin function should be unwound before resuming the VM.
 */
struct VMSwitchModeBuiltin : BaseException {
  const char* what() const noexcept override { return "VMSwitchModeBuiltin"; }
};

//////////////////////////////////////////////////////////////////////

}

#define incl_HPHP_VM_UNWIND_INL_H_
#include "hphp/runtime/vm/unwind-inl.h"
#undef incl_HPHP_VM_UNWIND_INL_H_

#endif
