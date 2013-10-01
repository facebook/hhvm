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

Block* IRUnit::makeMain(const Func* func, uint32_t bcOff) {
  assert(!m_main);
  auto entry = defBlock(func);
  m_bcOff = bcOff;
  m_main = new (m_arena) IRTrace(*this, entry);
  return entry;
}

Block* IRUnit::addExit(const Func* func) {
  auto exit = defBlock(func);
  exit->setHint(Block::Hint::Unlikely);
  m_exits.push_back(new (m_arena) IRTrace(*this, exit));
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
