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

#ifndef incl_HPHP_JIT_FUNC_GUARD_H
#define incl_HPHP_JIT_FUNC_GUARD_H

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

namespace HPHP {

struct Func;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Emit a func guard for `func' into `cb'.
 *
 * A func guard is a small stub of code immediately preceding a func prologue.
 * It checks the called Func* against the Func* the prologue corresponds to,
 * and if the check fails, it jumps to a redispatch stub.
 *
 * We don't always statically know what function to call from a given callsite.
 * When we don't, rather than calling straight into the func prologue, we call
 * the func guard instead.
 */
void emitFuncGuard(const Func* func, CodeBlock& cb);

/*
 * Get the address of the guard preceding a `prologue' for `func'.
 *
 * If `prologue' is the fcallHelperThunk unique stub, just return it.
 * Otherwise, `prologue' is required to be a valid func prologue.
 */
TCA funcGuardFromPrologue(TCA prologue, const Func* func);

/*
 * Whether `guard' is guarding on `func'.
 *
 * Returns false if `guard' is the fcallHelperThunk unique stub.  Otherwise,
 * `guard' is required to be a valid func guard.
 */
bool funcGuardMatches(TCA guard, const Func* func);

/*
 * Clobber `guard' so that it always fails.
 */
void clobberFuncGuard(TCA guard, const Func* func);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
