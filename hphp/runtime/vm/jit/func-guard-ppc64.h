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

#ifndef incl_HPHP_JIT_FUNC_GUARD_PPC64_H
#define incl_HPHP_JIT_FUNC_GUARD_PPC64_H

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

namespace HPHP {

struct Func;

namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of func-guard.h.
 */

void emitFuncGuard(const Func* func, CodeBlock& cb);
TCA funcGuardFromPrologue(TCA prologue, const Func* func);
bool funcGuardMatches(TCA guard, const Func* func);
void clobberFuncGuard(TCA guard, const Func* func);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
