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

#include "hphp/runtime/vm/jit/unwind-arm.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit { namespace arm {

vixl::Instruction* simulatorExceptionHook(vixl::Simulator* sim,
                                          std::exception_ptr exn) {
  if (tl_regState == VMRegState::DIRTY) {
    // This is a pseudo-copy of the logic in sync_regstate.
    mcg->fixupMap().fixupWorkSimulated(g_context.getNoCheck());
    tl_regState = VMRegState::CLEAN;
  }

  auto ctOpt = mcg->getCatchTrace(reinterpret_cast<TCA>(sim->pc()));
  if (!ctOpt.hasValue()) {
    return nullptr;
  }

  tl_regState = VMRegState::DIRTY;
  return vixl::Instruction::Cast(ctOpt.value());
}

}}}
