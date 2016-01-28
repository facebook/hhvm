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

#include "hphp/runtime/vm/jit/func-guard.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/func-guard-arm.h"
#include "hphp/runtime/vm/jit/func-guard-x64.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

void emitFuncGuard(const Func* func, CodeBlock& cb) {
  return ARCH_SWITCH_CALL(emitFuncGuard, func, cb);
}

TCA funcGuardFromPrologue(TCA prologue, const Func* func) {
#if defined(__powerpc64__)
  // Returning null since PPC64 specific code is under development
  return nullptr;
#endif
  return ARCH_SWITCH_CALL(funcGuardFromPrologue, prologue, func);
}

bool funcGuardMatches(TCA guard, const Func* func) {
#if defined(__powerpc64__)
  // Returning false since PPC64 specific code is under development
  return false;
#endif
  return ARCH_SWITCH_CALL(funcGuardMatches, guard, func);
}

void clobberFuncGuard(TCA guard, const Func* func) {
#if defined(__powerpc64__)
  // Returning since PPC64 specific code is under development
  return;
#endif
  return ARCH_SWITCH_CALL(clobberFuncGuard, guard, func);
}

///////////////////////////////////////////////////////////////////////////////

}}
