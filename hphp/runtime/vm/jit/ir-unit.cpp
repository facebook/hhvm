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

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-trace.h"

namespace HPHP {  namespace JIT {

IRInstruction* IRUnit::defLabel(unsigned numDst, BCMarker marker) {
  IRInstruction inst(DefLabel, marker);
  IRInstruction* label = cloneInstruction(&inst);
  if (numDst > 0) {
    SSATmp* dsts = (SSATmp*) m_arena.alloc(numDst * sizeof(SSATmp));
    for (unsigned i = 0; i < numDst; ++i) {
      new (&dsts[i]) SSATmp(m_nextOpndId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

Block* IRUnit::defBlock(const Func* func) {
  return new (m_arena) Block(m_nextBlockId++, func);
}

IRInstruction* IRUnit::mov(SSATmp* dst, SSATmp* src, BCMarker marker) {
  IRInstruction* inst = gen(Mov, marker, dst->type(), src);
  dst->setInstruction(inst);
  inst->setDst(dst);
  return inst;
}

SSATmp* IRUnit::findConst(ConstData& cdata, Type ctype) {
  IRInstruction inst(DefConst, BCMarker());
  inst.setExtra(&cdata);
  inst.setTypeParam(ctype);
  if (SSATmp* tmp = m_constTable.lookup(&inst)) {
    assert(tmp->type().equals(ctype));
    return tmp;
  }
  return m_constTable.insert(cloneInstruction(&inst)->dst());
}

IRTrace* IRUnit::makeTrace(const Func* func, uint32_t bcOff) {
  return new (m_arena) IRTrace(*this, defBlock(func), bcOff);
}

IRTrace* IRUnit::makeMain(const Func* func, uint32_t bcOff) {
  assert(!m_main);
  return m_main = makeTrace(func, bcOff);
}

IRTrace* IRUnit::addExit(const Func* func, uint32_t bcOff) {
  auto exit = makeTrace(func, bcOff);
  m_exits.push_back(exit);
  return exit;
}

Block* IRUnit::entry() const {
  return m_main->front();
}

// IRTrace methods declared here because of circular dependencies.

bool IRTrace::isMain() const {
  return this == m_unit.main();
}

}}
