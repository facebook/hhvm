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

#ifndef incl_HPHP_JIT_FUNC_PROLOGUE_H
#define incl_HPHP_JIT_FUNC_PROLOGUE_H

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"

#include <cstdint>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Func;

///////////////////////////////////////////////////////////////////////////////

namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Emit a func prologue, preceded by its func guard, to the TC, and return the
 * prologue's start address.
 *
 * Func prologues are responsible for initializing the parts of a callee's
 * frame not set up by Call---namely, stashing the return IP and setting up all
 * local variables, including parameters, closure use variables, and variadic
 * arg structures.  They also take care of function entry hooks, including
 * surprise flag and stack overflow checks.
 *
 * A func prologue does a large portion of the work of an interpreted FCall;
 * the rest of it is handled by the Call instruction.
 */
TCA genFuncPrologue(TransID transID, Func* func, int argc);

/*
 * Emit a func body dispatch entry point to the TC.
 *
 * This entry point calls DV init funclets for any un-passed parameters, and
 * then performs a bindjmp to the function's actual entry point translation.
 */
TCA genFuncBodyDispatch(Func* func, const DVFuncletsVec& dvs);

///////////////////////////////////////////////////////////////////////////////

}}

#endif // incl_HPHP_JIT_FUNC_PROLOGUE_H
