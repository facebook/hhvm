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

ConstInstruction* IRFactory::defConst(int64 val) {
  ConstInstruction inst(DefConst, val);
  return cloneInstruction(&inst);
}

LabelInstruction* IRFactory::defLabel(const Func* f) {
  LabelInstruction inst(m_nextLabelId++, f);
  return cloneInstruction(&inst);
}

LabelInstruction* IRFactory::defLabel(const Func* f, unsigned numDst) {
  LabelInstruction* label = defLabel(f);
  if (numDst > 0) {
    SSATmp** dsts = new (m_arena) SSATmp*[numDst];
    for (unsigned i = 0; i < numDst; ++i) {
      dsts[i] = new (m_arena) SSATmp(m_nextOpndId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

IRInstruction* IRFactory::mov(SSATmp* dst, SSATmp* src) {
  IRInstruction* inst = gen(Mov, dst->getType(), src);
  dst->setInstruction(inst);
  inst->setDst(dst);
  return inst;
}

}}}
