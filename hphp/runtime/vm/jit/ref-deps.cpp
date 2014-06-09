/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace JIT {

TRACE_SET_MOD(trans);

void
ActRecState::pushFunc(const NormalizedInstruction& inst) {
  assert(isFPush(inst.op()));
  if (inst.op() == OpFPushFuncD || inst.op() == OpFPushFuncU) {
    const Unit& unit = *inst.unit();
    Id funcId = inst.imm[1].u_SA;
    auto const& nep = unit.lookupNamedEntityPairId(funcId);
    auto const func = Unit::lookupFunc(nep.second);
    if (func) func->validate();
    if (func && func->isNameBindingImmutable(&unit)) {
      pushFuncD(func);
      return;
    }
  }
  pushDynFunc();
}

void
ActRecState::pushFuncD(const Func* func) {
  TRACE(2, "ActRecState: pushStatic func %p(%s)\n", func, func->name()->data());
  func->validate();
  Record r;
  r.m_state = State::KNOWN;
  r.m_topFunc = func;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pushDynFunc() {
  TRACE(2, "ActRecState: pushDynFunc\n");
  Record r;
  r.m_state = State::UNKNOWABLE;
  r.m_topFunc = nullptr;
  r.m_entryArDelta = InvalidEntryArDelta;
  m_arStack.push_back(r);
}

void
ActRecState::pop() {
  if (!m_arStack.empty()) {
    m_arStack.pop_back();
  }
}

/**
 * checkByRef() returns true if the parameter specified by argNum is pass
 * by reference, otherwise it returns false. This function may also throw an
 * UnknownInputException if the reffiness cannot be determined.
 *
 * Note that the 'entryArDelta' parameter specifies the delta between sp at
 * the beginning of the tracelet and ar.
 */
bool
ActRecState::checkByRef(int argNum, int entryArDelta, RefDeps* refDeps) {
  FTRACE(2, "ActRecState: getting reffiness for arg {}, arDelta {}\n",
         argNum, entryArDelta);
  if (m_arStack.empty()) {
    // The ActRec in question was pushed before the beginning of the
    // tracelet, so we can make a guess about parameter reffiness and
    // record our assumptions about parameter reffiness as tracelet
    // guards.
    const ActRec* ar = arFromSpOffset((ActRec*)vmsp(), entryArDelta);
    Record r;
    r.m_state = State::GUESSABLE;
    r.m_entryArDelta = entryArDelta;
    ar->m_func->validate();
    r.m_topFunc = ar->m_func;
    m_arStack.push_back(r);
  }
  Record& r = m_arStack.back();
  if (r.m_state == State::UNKNOWABLE) {
    TRACE(2, "ActRecState: unknowable, throwing in the towel\n");
    throwUnknownInput();
    not_reached();
  }
  assert(r.m_topFunc);
  bool retval = r.m_topFunc->byRef(argNum);
  if (r.m_state == State::GUESSABLE) {
    assert(r.m_entryArDelta != InvalidEntryArDelta);
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    refDeps->addDep(r.m_entryArDelta, argNum, retval);
  }
  return retval;
}

const Func*
ActRecState::knownFunc() {
  if (currentState() != State::KNOWN) return nullptr;
  assert(!m_arStack.empty());
  return m_arStack.back().m_topFunc;
}

ActRecState::State
ActRecState::currentState() {
  if (m_arStack.empty()) return State::GUESSABLE;
  return m_arStack.back().m_state;
}

} } // HPHP::JIT
