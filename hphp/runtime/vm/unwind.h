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
#pragma once

#include <stdexcept>
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/either.h"
#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

enum class UnwinderResult {
   None,
   // Unwound an async function and placed the exception inside a failed static
   // wait handle
   FSWH,
   // Unwound until the given fp, i.e. did not reach the end of the vm nesting
   ReachedGoal,
   // Unwound until the given fp, but there is a pending C++ exception that
   // should replace the current Hack exception
   ReplaceWithPendingException,
};

//////////////////////////////////////////////////////////////////////

/*
 * Locks the object on the top of the stack if the PC points to a FCallCtor and
 * the FCallArgs indicates the necessity to lock
 */
void lockObjectWhileUnwinding(PC pc, Stack& stack);

/*
 * Find an exception handler for a given raise location if the handler was
 * found or kInvalidOffset.
 */
Offset findExceptionHandler(const Func* func, Offset raiseOffset);

/*
 * Unwind the exception.
 */
UnwinderResult unwindVM(Either<ObjectData*, Exception*> exception,
                        const ActRec* fpToUnwind = nullptr,
                        bool teardown = true);

/*
 * The main entry point to the unwinder.
 *
 * Wraps action in try/catch and executes appropriate unwinding logic based
 * on the type of thrown exception.
 *
 * If the exception was not handled in this nesting of the VM, it will be
 * rethrown. Otherwise, either a catch or fault handler was identified and
 * the VM state has been prepared for entry to it, or end of execution of
 * the action callback was reached, in which case the VM state will be left
 * in the same state as left by the callback.
 *
 * Return true iff the action succeeded.
 */
template<class Action> bool exception_handler(Action action);

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

}

#define incl_HPHP_VM_UNWIND_INL_H_
#include "hphp/runtime/vm/unwind-inl.h"
#undef incl_HPHP_VM_UNWIND_INL_H_

