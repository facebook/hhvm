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

#ifndef incl_HPHP_JIT_DEBUGGER_H_
#define incl_HPHP_JIT_DEBUGGER_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct SrcKey;
struct ActRec;

namespace jit {

/*
 * Debug blacklist.
 *
 * The set of PC's and SrcKey's we refuse to JIT because they contain hphpd
 * breakpoints.
 */

/*
 * Atomically clear all entries from the debug blacklist.
 */
void clearDbgBL();

/*
 * Add `pc' to the debug blacklist.
 *
 * Return whether we actually performed an insertion.
 */
bool addDbgBLPC(PC pc);

/*
 * Check if `sk' is in the debug blacklist.
 *
 * Lazily populates m_dbgBLSrcKey from m_dbgBLPC if we don't find the entry.
 */
bool isSrcKeyInDbgBL(SrcKey sk);

/*
 * Look up the catch block associated with the saved return address in `ar' and
 * stash it in a map.
 *
 * This is called by debugger helpers right before smashing the return address
 * to prevent returning directly the to TC.
 */
void stashDebuggerCatch(const ActRec* ar);

/*
 * Unstash the debugger catch block for `ar' and return it.
 */
TCA unstashDebuggerCatch(const ActRec* ar);

/*
 * Clear all stashed debugger catches
 */
void clearDebuggerCatches();

}}

#endif
