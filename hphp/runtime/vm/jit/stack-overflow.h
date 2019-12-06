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

namespace HPHP {

struct ActRec;
struct Func;
struct TypedValue;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Return whether the call to the `callee` could overflow the stack, assuming
 * `calleeFP` points to the address of the possibly uninitialized callee frame.
 */
bool checkCalleeStackOverflow(const TypedValue* calleeFP, const Func* callee);

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
