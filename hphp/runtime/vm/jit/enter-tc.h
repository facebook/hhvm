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

#ifndef incl_HPHP_JIT_ENTER_TC_H_
#define incl_HPHP_JIT_ENTER_TC_H_

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct ActRec;

namespace jit { namespace detail {
/*
 * Main entry point for the translator from the bytecode interpreter.  It
 * operates on behalf of a given nested invocation of the intepreter (calling
 * back into it as necessary for blocks that need to be interpreted).
 *
 * If `start' is the address of a func prologue, `stashedAR' should be the
 * ActRec prepared for the call to that function.  Otherwise it should be
 * nullptr.
 *
 * But don't call it directly, use one of the helpers below.
 */
void enterTC(TCA start, ActRec* stashedAR);
}

inline void enterTC() {
  detail::enterTC(tc::ustubs().resumeHelper, nullptr);
}

inline void enterTCAtPrologue(ActRec* ar, TCA start) {
  assertx(ar);
  assertx(start);
  detail::enterTC(start, ar);
}

inline void enterTCAfterPrologue(TCA start) {
  assertx(start);
  detail::enterTC(start, nullptr);
}

}}

#endif
