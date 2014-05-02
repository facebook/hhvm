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

#include "hphp/runtime/vm/jit/ir-unit.h"

#include "hphp/runtime/vm/jit/block.h"

namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

IRUnit::IRUnit(TransContext context)
  : m_context(context)
  , m_entry(defBlock())
{}

IRInstruction* IRUnit::defLabel(unsigned numDst, BCMarker marker,
                                const smart::vector<unsigned>& producedRefs) {
  IRInstruction inst(DefLabel, marker);
  IRInstruction* label = cloneInstruction(&inst);
  always_assert(producedRefs.size() == numDst);
  m_labelRefs[label] = producedRefs;
  if (numDst > 0) {
    SSATmp* dsts = (SSATmp*) m_arena.alloc(numDst * sizeof(SSATmp));
    for (unsigned i = 0; i < numDst; ++i) {
      new (&dsts[i]) SSATmp(m_nextOpndId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

Block* IRUnit::defBlock() {
  FTRACE(2, "IRUnit defining B{}\n", m_nextBlockId);
  return new (m_arena) Block(m_nextBlockId++);
}

IRInstruction* IRUnit::mov(SSATmp* dst, SSATmp* src, BCMarker marker) {
  IRInstruction* inst = gen(Mov, marker, src);
  dst->setInstruction(inst);
  inst->setDst(dst);
  return inst;
}

SSATmp* IRUnit::findConst(Type type) {
  assert(type.isConst());
  IRInstruction inst(DefConst, BCMarker{});
  inst.setTypeParam(type);
  if (SSATmp* tmp = m_constTable.lookup(&inst)) {
    assert(tmp->type().equals(type));
    return tmp;
  }
  return m_constTable.insert(cloneInstruction(&inst)->dst());
}

}}
