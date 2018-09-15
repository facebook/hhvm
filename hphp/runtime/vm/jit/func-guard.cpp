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

#include "hphp/runtime/vm/jit/func-guard.h"

#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/vm/jit/cg-meta.h"

#include "hphp/util/arch.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

size_t funcGuardLen() {
  return ARCH_SWITCH_CALL(funcGuardLen);
}

void emitFuncGuard(const Func* func, CodeBlock& cb,
                   CGMeta& fixups, TCA* watch) {
  ARCH_SWITCH_CALL(emitFuncGuard, func, cb, fixups);
  if (watch) {
    *watch = cb.frontier();
    fixups.watchpoints.push_back(watch);
  }
}

TCA funcGuardFromPrologue(TCA prologue, const Func* func) {
  return ARCH_SWITCH_CALL(funcGuardFromPrologue, prologue, func);
}

bool funcGuardMatches(TCA guard, const Func* func) {
  return ARCH_SWITCH_CALL(funcGuardMatches, guard, func);
}

void clobberFuncGuard(TCA guard, const Func* func) {
  return ARCH_SWITCH_CALL(clobberFuncGuard, guard, func);
}

void clobberFuncGuards(const Func* func) {
  auto const numPrologues = func->numPrologues();

  for (auto i = 0; i < numPrologues; ++i) {
    auto const guard = funcGuardFromPrologue(func->getPrologue(i), func);
    if (funcGuardMatches(guard, func)) {
      clobberFuncGuard(guard, func);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
