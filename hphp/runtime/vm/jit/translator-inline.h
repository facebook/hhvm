/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRANSLATOR_INLINE_H_
#define incl_HPHP_TRANSLATOR_INLINE_H_

#include <boost/noncopyable.hpp>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"

/*
 * Because of a circular dependence with ExecutionContext, these
 * translation-related helpers cannot live in translator.h.
 */
namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline ActRec* liveFrame() { return vmfp(); }
inline const Func* liveFunc() { return liveFrame()->m_func; }
inline const Unit* liveUnit() { return liveFunc()->unit(); }
inline Class* liveClass() { return liveFunc()->cls(); }
inline bool liveResumed() { return liveFrame()->resumed(); }

inline jit::FPInvOffset liveSpOff() {
  Cell* fp = reinterpret_cast<Cell*>(vmfp());
  if (liveFrame()->resumed()) {
    fp = (Cell*)Stack::resumableStackBase((ActRec*)fp);
  }
  return jit::FPInvOffset{safe_cast<int32_t>(fp - vmsp())};
}

///////////////////////////////////////////////////////////////////////////////

namespace jit {

///////////////////////////////////////////////////////////////////////////////

inline int cellsToBytes(int nCells) {
  return safe_cast<int32_t>(nCells * ssize_t(sizeof(Cell)));
}

inline int localOffset(int locId) {
  return -cellsToBytes(locId + 1);
}

///////////////////////////////////////////////////////////////////////////////

}}

#endif
