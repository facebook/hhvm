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

#ifndef incl_HPHP_JIT_TC_PROLOGUE_H_
#define incl_HPHP_JIT_TC_PROLOGUE_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

struct ProfTransRec;

namespace tc {

struct CodeMetaLock;

/*
 * Emit a function prologue from rec.
 *
 * Precondition: calling thread owns both code and metadata locks
 */
void emitFuncPrologueOptInternal(PrologueMetaInfo& info,
                                 CodeMetaLock* locker);

/*
 * Publish the metadata for the given func prologue.  Returns whether or not it
 * succeeded.
 */
bool publishFuncPrologueMeta(Func* func, int nArgs, TransKind kind,
                             PrologueMetaInfo& info);

/*
 * Publish the code for the given func prologue.  Returns whether or not it
 * succeeded.
 */
bool publishFuncPrologueCode(Func* func, int nArgs, PrologueMetaInfo& info);

/*
 * Smash the callers of the ProfPrologue associated with `rec' to call a new
 * prologue at `start' address.
 */
void smashFuncCallers(TCA start, ProfTransRec* rec);

/*
 * Emit the prologue dispatch for func which contains dvs DV initializers, and
 * return its start address.  The `kind' of translation argument is used to
 * decide what area of the code cache will be used (hot, main, or prof).
 *
 * Precondition: calling thread owns both code and metadata locks, or
 *               passes a non-null locker param
 */
TransLoc emitFuncBodyDispatchInternal(Func* func, const DVFuncletsVec& dvs,
                                      TransKind kind, CodeMetaLock* locker);

void publishFuncBodyDispatch(Func* func, TCA start, TCA end);

}

}}

#endif
