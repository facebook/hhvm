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
#ifndef incl_HHBBC_DEBUG_H_
#define incl_HHBBC_DEBUG_H_

#include "hphp/util/trace.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Dump the entire program to a temporary directory as readable text.
 */
void debug_dump_program(const php::Program&);

/*
 * Utilities for printing the state of the program after various
 * transformations.
 */

inline void banner(const char* what) {
  TRACE_SET_MOD(hhbbc);
  FTRACE(2, "{:-^70}\n", what);
}

inline void state_after(const char* when, const php::Program& program) {
  TRACE_SET_MOD(hhbbc);
  banner(when);
  FTRACE(4, "{}", show(program));
  banner("");
}

//////////////////////////////////////////////////////////////////////

}}

#endif
