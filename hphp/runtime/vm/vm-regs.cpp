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

#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP {

// Register dirtiness: thread-private.
__thread VMRegState tl_regState = VMRegState::CLEAN;

VMRegAnchor::VMRegAnchor()
  : m_old(tl_regState)
{
  assert_native_stack_aligned();
  jit::mcg->sync();
}

VMRegAnchor::VMRegAnchor(ActRec* ar)
  : m_old(tl_regState)
{
  assert(tl_regState == VMRegState::DIRTY);
  tl_regState = VMRegState::CLEAN;

  auto prevAr = g_context->getOuterVMFrame(ar);
  const Func* prevF = prevAr->m_func;
  assert(!ar->resumed());
  auto& regs = vmRegs();
  regs.stack.top() = (TypedValue*)ar - ar->numArgs();
  assert(vmStack().isValidAddress((uintptr_t)vmsp()));
  regs.pc = prevF->unit()->at(prevF->base() + ar->m_soff);
  regs.fp = prevAr;
}

}
