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

#ifndef incl_HPHP_JIT_MCGEN_PROLOGUE_H_
#define incl_HPHP_JIT_MCGEN_PROLOGUE_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct Func;
struct SrcKey;

namespace jit { namespace mcgen {

/*
 * Regenerate all prologues of func that were previously generated.
 * The prologues are sorted in ascending order of profile counters.
 * For prologues with corresponding DV funclets, their corresponding
 * DV funclet will be regenerated right after them.  The idea is to
 * generate the function body right after calling this function, so
 * that all prologues are placed right before it, and with the hottest
 * prologues closer to it.
 *
 * Returns the starting address for the translation corresponding to
 * triggerSk, if such translation is generated; otherwise returns
 * nullptr.
 */
TCA regeneratePrologues(Func* func, SrcKey triggerSk, bool& includedBody);

}}}

#endif
