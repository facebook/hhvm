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

#ifndef incl_HPHP_JIT_TC_PROLOGUE_H_
#define incl_HPHP_JIT_TC_PROLOGUE_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

struct ProfTransRec;

namespace tc {

/*
 * Emit a function prologue from rec.
 *
 * Precondition: calling thread owns both code and metadata locks
 */
TCA emitFuncPrologueOptInternal(ProfTransRec* rec);

/*
 * Emit the prologue dispatch for func which contains dvs DV initializers, and
 * return its start address.  The `kind' of translation argument is used to
 * decide what area of the code cache will be used (hot, main, or prof).
 *
 * Precondition: calling thread owns both code and metadata locks
 */
TCA emitFuncBodyDispatchInternal(Func* func, const DVFuncletsVec& dvs,
                                 TransKind kind);

}

}}

#endif
