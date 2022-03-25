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

#pragma once

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Represents the address to resume at when returning from interpreter to JIT.
 *
 * The reenterTC unique stub will receive the `handler' and `arg' fields in
 * rret(0) and rret(1) registers and it will jump to the rret(0) handler,
 * passing it the optional argument via rret(1).
 *
 * The `handler' operates in the interpreter context. If the next execution is
 * handled by another resume helper, one of the *FromInterp helpers should be
 * used. If it is handled by a TC, an additional helper may be needed to convert
 * from interpreter to TC state.
 *
 * The reenterTC stub reloads the rvmfp() register. This is needed to remove
 * the potentially invalid previous frame pointer from the chain (for more
 * details, see jitReturnPost()). It also reloads the rvmsp() register, which
 * is needed anyway when returning back to TC and allows us to skip an interp
 * to TC helper in the most common case. It is otherwise harmless and helps
 * debugging of resume helpers.
 */
struct JitResumeAddr {
  TCA handler;
  TCA arg;

  // Returns true if the address exists (i.e. not none()).
  operator bool() const;
  // No resume address.
  static JitResumeAddr none();
  // One of the FromInterp resume helpers.
  static JitResumeAddr helper(TCA tca);
  // Resume at `tca', which follows a call inside a translation.
  static JitResumeAddr ret(TCA tca);
  // Resume at `tca', which is a start of a regular translation.
  static JitResumeAddr trans(TCA tca);
  // Resume at `tca', which is a start of a func entry translation.
  static JitResumeAddr transFuncEntry(TCA tca);
};

///////////////////////////////////////////////////////////////////////////////

}
