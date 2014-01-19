/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_DEBUG_H_
#define incl_HPHP_DEBUG_H_

#include <signal.h>

#ifdef DEBUG

/*
 * Conditionally drop into the debugger
 */
#define DEBUGGER()    kill(getpid(), SIGTRAP)

/*
 * DEBUGGER_IF: use like always_assert(), i.e., do not rely on side effects
 * of pred, as it will be compiled out of debug builds.
 */
#define DEBUGGER_IF(pred) do { \
  if ((pred)) {                \
    DEBUGGER();                \
  }                            \
} while(0)

#else
#define DEBUGGER()       do { } while(0)
#define DEBUGGER_IF(...) do { } while(0)
#endif /* DEBUG */

#endif /* incl_HPHP_DEBUG_H_ */
