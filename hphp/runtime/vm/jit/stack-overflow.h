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

#ifndef incl_HPHP_JIT_STACK_OVERFLOW_H_
#define incl_HPHP_JIT_STACK_OVERFLOW_H_

namespace HPHP {

struct ActRec;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Return whether the `calleeAR' frame overflows the stack.
 *
 * Expects `calleeAR' and its arguments to be on the VM stack.
 */
bool checkCalleeStackOverflow(const ActRec* calleeAR);

/*
 * Handle a VM stack overflow condition by throwing an appropriate exception.
 */
void handleStackOverflow(ActRec* calleeAR);

/*
 * Determine whether something is a stack overflow, and if so, handle it.
 *
 * This should only be called from a func prologue after it's finished setting
 * up the callee's frame (including args and locals).
 */
void handlePossibleStackOverflow(ActRec* calleeAR);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
