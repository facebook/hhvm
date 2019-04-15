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
#include "hphp/runtime/vm/jit/ref-deps.h"

#include <iostream>
#include <sstream>

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

void
ActRecState::pushFunc(const NormalizedInstruction& inst) {
  assertx(isFPush(inst.op()));

  const Unit& unit = *inst.unit();
  const Func* func = nullptr;

  if (inst.op() == OpFPushFuncD) {
    Id funcId = inst.imm[1].u_SA;
    auto const& nep = unit.lookupNamedEntityPairId(funcId);
    func = lookupImmutableFunc(&unit, nep.first).func;
  }

  if (func) {
    TRACE(2, "ActRecState: pushFunc %p(%s)\n", func, func->name()->data());
    func->validate();
    m_arStack.push_back(func);
  } else {
    TRACE(2, "ActRecState: pushFunc dyn\n");
    m_arStack.push_back(nullptr);
  }
}

void
ActRecState::pop() {
  if (!m_arStack.empty()) {
    m_arStack.pop_back();
  }
}

const Func*
ActRecState::knownFunc() {
  if (m_arStack.empty()) return nullptr;
  return m_arStack.back();
}

} } // HPHP::jit
