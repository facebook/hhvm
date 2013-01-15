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

IRInstruction* IRFactory::cloneInstruction(const IRInstruction* inst) {
  return new (m_arena) IRInstruction(*this, inst);
}

ConstInstruction* IRFactory::cloneInstruction(const ConstInstruction* inst) {
  return new (m_arena) ConstInstruction(*this, inst);
}

LabelInstruction* IRFactory::cloneInstruction(const LabelInstruction* inst) {
  return new (m_arena) LabelInstruction(*this, inst);
}

ConstInstruction* IRFactory::defConst(int64 val) {
  return new (m_arena) ConstInstruction(DefConst, val);
}

LabelInstruction* IRFactory::defLabel() {
  return new (m_arena) LabelInstruction(m_nextLabelId++);
}

LabelInstruction* IRFactory::marker(uint32 bcOff, const Func* f, int32 spOff) {
  return new (m_arena) LabelInstruction(Marker, bcOff, f, spOff);
}

}}}
