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

#ifndef __IMMSTACK_H__
#define __IMMSTACK_H__

#include <runtime/vm/bytecode.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

class ImmStack {
private:
  struct StackItem {
    enum StackType {
      StackType_Unknown,
      StackType_Int,
      StackType_Litstr,
    };

    StackType type;
    union {
      int64_t i64a;
      Id sa;
    };
  };

public:

  // calls pop(), and pushUnknown() appropriately
  void processOpcode(const Opcode* opcode);

  void pushUnknown();
  void insUnknown(int pos);
  void pop();

  // By using these methods (instead of the popUknown() above) you signify that
  //   the instruction most recently added via addInstr() is side-effect free,
  //   i.e. that if this particular value on the stack is not needed then
  //   the corresponding opcode can be removed. Thus we also cannot use
  //   these methods for opcodes which push multiple values on to the stack,
  //   such as dup.
  void pushInt(int64_t value);
  void pushLitstr(Id value);

  // 0 is the item most recently pushed on to the stack, followed by 1 etc.
  bool isInt(int above);
  bool isLitstr(Id above);

  StackItem &get(int above);

private:
  std::vector<StackItem> m_stack;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
