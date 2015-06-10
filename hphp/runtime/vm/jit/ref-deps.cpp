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

namespace HPHP { namespace jit {

TRACE_SET_MOD(trans);

std::string RefDeps::Record::pretty() const {
  std::ostringstream out;
  out << "mask=";
  for (size_t i = 0; i < m_mask.size(); ++i) {
    out << (m_mask[i] ? "1" : "0");
  }
  out << " vals=";
  for (size_t i = 0; i < m_vals.size(); ++i) {
    out << (m_vals[i] ? "1" : "0");
  }
  return out.str();
}

void RefDeps::addDep(int entryArDelta, unsigned argNum, bool isRef) {
  if (m_arMap.find(entryArDelta) == m_arMap.end()) {
    m_arMap[entryArDelta] = Record();
  }
  Record& r = m_arMap[entryArDelta];
  if (argNum >= r.m_mask.size()) {
    assertx(argNum >= r.m_vals.size());
    r.m_mask.resize(argNum + 1);
    r.m_vals.resize(argNum + 1);
  }
  r.m_mask[argNum] = true;
  r.m_vals[argNum] = isRef;
}

void
ActRecState::pushFunc(const NormalizedInstruction& inst) {
  assertx(isFPush(inst.op()));

  const Unit& unit = *inst.unit();
  const Func* func = nullptr;

  if (inst.op() == OpFPushFuncD || inst.op() == OpFPushFuncU) {
    Id funcId = inst.imm[1].u_SA;
    auto const& nep = unit.lookupNamedEntityPairId(funcId);
    func = Unit::lookupFunc(nep.second);
  } else if (inst.op() == OpFPushCtorD) {
    Id clsId = inst.imm[1].u_SA;
    auto const& nep = unit.lookupNamedEntityPairId(clsId);
    auto const cls = Unit::lookupClass(nep.second);
    auto const scopeFunc = knownFunc();
    auto const ctx = scopeFunc ? scopeFunc->cls() : nullptr;
    func = lookupImmutableCtor(cls, ctx);
  }

  if (func) func->validate();
  if (func && func->isNameBindingImmutable(&unit)) {
    pushFuncD(func);
    return;
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
  assertx(r.m_topFunc);
  bool retval = r.m_topFunc->byRef(argNum);
  if (r.m_state == State::GUESSABLE) {
    assertx(r.m_entryArDelta != InvalidEntryArDelta);
    TRACE(2, "ActRecState: guessing arg%d -> %d\n", argNum, retval);
    refDeps->addDep(r.m_entryArDelta, argNum, retval);
  }
  return retval;
}

const Func*
ActRecState::knownFunc() {
  if (currentState() != State::KNOWN) return nullptr;
  assertx(!m_arStack.empty());
  return m_arStack.back().m_topFunc;
}

ActRecState::State
ActRecState::currentState() {
  if (m_arStack.empty()) return State::GUESSABLE;
  return m_arStack.back().m_state;
}

} } // HPHP::jit
