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

#ifndef incl_HPHP_ACT_REC_DEFS_H_
#define incl_HPHP_ACT_REC_DEFS_H_

#include "hphp/runtime/vm/act-rec.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"

#include "hphp/util/assertions.h"

/*
 * This is separate from act-rec-inl.h in order to keep the transitive header
 * dependencies of act-rec.h as tight as possible.
 *
 * However, for performance reasons, we need to make sure certain functions get
 * inlined.  Callers of these functions need to additionally include this file.
 */

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline void ActRec::setReturnVMExit() {
  assertx(isReturnHelper(jit::tc::ustubs().callToExit));
  m_sfp = nullptr;
  m_savedRip = reinterpret_cast<uintptr_t>(jit::tc::ustubs().callToExit);
  m_callOffAndFlags = 0;
#ifdef __powerpc64__
  // Save the TOC value, as it will need to be restored when unwinding.
  asm ("mr %0, 2" : "=r" (m_savedToc));
#endif
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
