/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_
#define incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_

#include "hphp/runtime/vm/jit/unwind-types.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////

TCA lookup_catch_trace(TCA rip, _Unwind_Exception* exn);

/*
 * Called whenever we create a new translation cache for the whole
 * region of code.
 */
UnwindInfoHandle register_unwind_region(unsigned char* address, size_t size);

/*
 * The personality routine for code emitted by the jit.
 */
_Unwind_Reason_Code
tc_unwind_personality(int version,
                      _Unwind_Action actions,
                      uint64_t exceptionClass,
                      _Unwind_Exception* exceptionObj,
                      _Unwind_Context* context);

TCUnwindInfo tc_unwind_resume(ActRec* fp);

//////////////////////////////////////////////////////////////////////

}}}

#endif
