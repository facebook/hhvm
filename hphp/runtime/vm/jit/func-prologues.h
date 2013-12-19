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
#ifndef incl_HPHP_JIT_FUNC_PROLOGUES_H
#define incl_HPHP_JIT_FUNC_PROLOGUES_H

#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/func-prologues-arm.h"
#include "hphp/runtime/vm/jit/func-prologues-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT {

inline bool funcPrologueHasGuard(JIT::TCA prologue, const Func* func) {
  if (arch() == Arch::X64) {
    return X64::funcPrologueHasGuard(prologue, func);
  } else if (arch() == Arch::ARM) {
    return ARM::funcPrologueHasGuard(prologue, func);
  }
  not_implemented();
}

inline JIT::TCA funcPrologueToGuard(JIT::TCA prologue, const Func* func) {
  if (arch() == Arch::X64) {
    return X64::funcPrologueToGuard(prologue, func);
  } else if (arch() == Arch::ARM) {
    return ARM::funcPrologueToGuard(prologue, func);
  }
  not_implemented();
}

}}

#endif
