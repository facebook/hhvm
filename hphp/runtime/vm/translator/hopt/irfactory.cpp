/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP { namespace VM { namespace JIT {

IRInstruction* IRFactory::defLabel() {
  IRInstruction inst(DefLabel);
  return cloneInstruction(&inst);
}

IRInstruction* IRFactory::defLabel(unsigned numDst) {
  IRInstruction* label = defLabel();
  if (numDst > 0) {
    SSATmp* dsts = (SSATmp*) m_arena.alloc(numDst * sizeof(SSATmp));
    for (unsigned i = 0; i < numDst; ++i) {
      new (&dsts[i]) SSATmp(m_nextOpndId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

Block* IRFactory::defBlock(const Func* func, IRInstruction* label) {
  return new (m_arena) Block(m_nextBlockId++, func, label);
}

IRInstruction* IRFactory::mov(SSATmp* dst, SSATmp* src) {
  IRInstruction* inst = gen(Mov, dst->getType(), src);
  dst->setInstruction(inst);
  inst->setDst(dst);
  return inst;
}

}}}
