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

#include <runtime/vm/translator/immstack.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

void ImmStack::processOpcode(const Opcode* opcode) {
  StackTransInfo sti = instrStackTransInfo(opcode);
  if (sti.kind == StackTransInfo::PushPop) {
    for (int i = 0; i < sti.numPops; i++) {
      pop();
    }
    for (int i = 0; i < sti.numPushes; i++) {
      pushUnknown();
    }
  } else if (sti.kind == StackTransInfo::InsertMid) {
    insUnknown(sti.pos);
  } else {
    ASSERT(false);
  }
}

void ImmStack::pushUnknown() {
  StackItem item;

  item.type = StackItem::StackType_Unknown;
  m_stack.push_back(item);
}

void ImmStack::insUnknown(int pos) {
  int k = m_stack.size() - 1 - pos;
  if (k >= 0 && k < (int)m_stack.size()) {
    StackItem item;
    item.type = StackItem::StackType_Unknown;
    m_stack.insert(m_stack.begin() + k, item);
  }
}

void ImmStack::pop() {
  // Since we only process on the basic block level, we obviously can pop
  //   locations that we don't track. We treat all these locations as unknowns.

  if (!m_stack.empty()) {
    m_stack.pop_back();
  }
}

void ImmStack::pushInt(int64_t value) {
  StackItem item;

  item.type = StackItem::StackType_Int;
  item.i64a = value;

  m_stack.push_back(item);
}

void ImmStack::pushLitstr(Id value) {
  StackItem item;

  item.type = StackItem::StackType_Litstr;
  item.sa = value;

  m_stack.push_back(item);
}

ImmStack::StackItem &ImmStack::get(int above) {
  always_assert(above >= 0 && (size_t)above < m_stack.size());

  return m_stack[m_stack.size() - above - 1];
}

bool ImmStack::isInt(int above) {
  if (above >= (int)m_stack.size()) {
    return false;
  }

  return get(above).type == StackItem::StackType_Int;
}

bool ImmStack::isLitstr(int above) {
  if (above >= (int)m_stack.size()) {
    return false;
  }

  return get(above).type == StackItem::StackType_Litstr;
}

}
}
