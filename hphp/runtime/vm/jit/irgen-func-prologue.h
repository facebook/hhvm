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

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"

#include <cstdint>

namespace HPHP { namespace jit {

struct SSATmp;

namespace irgen {

struct IRGS;

///////////////////////////////////////////////////////////////////////////////

/*
 * Check for presence, count and wildcard match of generics.
 *
 * If `pushed' is true, generics are on the stack. Otherwise, generics may or
 * may not be above the stack, depending on the callFlags.
 *
 * After this check, generics will be on the stack iff the function has reified
 * generics.
 */
void emitCalleeGenericsChecks(IRGS& env, const Func* callee, SSATmp* callFlags,
                              bool pushed);

void emitCalleeDynamicCallChecks(IRGS& env, const Func* callee,
                                SSATmp* callFlags);

void emitCalleeCoeffectChecks(IRGS& env, const Func* callee,
                              SSATmp* callFlags, SSATmp* providedCoeffects,
                              uint32_t argc, SSATmp* prologueCtx);

void emitCalleeImplicitContextChecks(IRGS& env, const Func* callee);

void emitInitFuncInputs(IRGS& env, const Func* callee, uint32_t argc);

void emitInitFuncLocals(IRGS& env, const Func* callee, SSATmp* prologueCtx);

void emitFuncPrologue(IRGS& env, const Func* callee, uint32_t argc,
                      TransID transID);

///////////////////////////////////////////////////////////////////////////////

}}}

