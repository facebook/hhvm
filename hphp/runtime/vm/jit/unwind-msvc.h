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

#ifndef incl_HPHP_JIT_UNWIND_MSVC_H_
#define incl_HPHP_JIT_UNWIND_MSVC_H_

#include "hphp/util/assertions.h"

/*
 * MSVC doesn't have unwind.h, so when we need it, we instead include these
 * garbage stubs to convince MSVC to compile without freaking out.
 */
struct _Unwind_Exception {
  uint64_t exception_class;
};

#define _URC_CONTINUE_UNWIND 0
#define _URC_INSTALL_CONTEXT 0
#define _URC_HANDLER_FOUND 0
using _Unwind_Reason_Code = int;

#define _UA_HANDLER_FRAME 0
#define _UA_CLEANUP_PHASE 0
#define _UA_SEARCH_PHASE 0
using _Unwind_Action = int;

using _Unwind_Context = void;

inline uintptr_t _Unwind_GetGR(_Unwind_Context*, int) {
  always_assert(false);
  return 0;
}
inline uintptr_t _Unwind_GetIP(_Unwind_Context*) {
  always_assert(false);
  return 0;
}
inline void _Unwind_Resume() { always_assert(false); }
inline void _Unwind_SetIP(_Unwind_Context*, uint64_t) { always_assert(false); }

#endif
