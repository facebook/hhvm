/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_EXCEPTION_GATE_H_
#define incl_EXCEPTION_GATE_H_

namespace HPHP {
namespace VM {

int exception_gate_handle();

#define EXCEPTION_GATE_ENTER()                                          \
  do {                                                                  \
    int longJmpType = 0;                                                \
    try { /* Open a block. */                                           \
      do { } while(0) /* make it ok to stick a semi-colon after this. */

#define EXCEPTION_GATE_COMMON(x)                                        \
      x;                                                                \
    } catch (...) { longJmpType = HPHP::VM::exception_gate_handle(); }  \
    g_vmContext->hhvmThrow(longJmpType);                                \
  } while(false)

#define EXCEPTION_GATE_LEAVE()                                          \
  EXCEPTION_GATE_COMMON(break)                                    \

#define EXCEPTION_GATE_RETURN(retval) /* empty for void functions */ \
  EXCEPTION_GATE_COMMON(return retval);

} } // HPHP::VM


#endif
