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

#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/print.h"

namespace HPHP { namespace JIT {

SSATmp::SSATmp(uint32_t opndId, IRInstruction* i, int dstId /* = 0 */)
  : m_id(opndId)
{
  setInstruction(i, dstId);
}

void SSATmp::setInstruction(IRInstruction* inst, int dstId) {
  m_inst = inst;
  m_type = outputType(inst, dstId);
}

namespace {
int typeNeededWords(Type t) {
  assert(!t.equals(Type::Bottom));

  if (t.subtypeOfAny(Type::Null, Type::ActRec, Type::RetAddr, Type::Nullptr)) {
    // These don't need a register because their values are static or unused.
    //
    // RetAddr doesn't take any register because currently we only target x86,
    // which takes the return address from the stack.  This knowledge should be
    // moved to a machine-specific section once we target other architectures.
    return 0;
  }
  if (t.maybe(Type::Nullptr)) {
    return typeNeededWords(t - Type::Nullptr);
  }
  if (t <= Type::Ctx || t.isPtr()) {
    // Ctx and PtrTo* may be statically unknown but always need just 1 register.
    return 1;
  }
  if (!t.isUnion()) {
    // Not a union type and not a special case: 1 register.
    assert(IMPLIES(t <= Type::StackElem, t.isKnownDataType()));
    return 1;
  }

  assert(t <= Type::StackElem);
  return t.needsReg() ? 2 : 1;
}
}

int SSATmp::numWords() const {
  return typeNeededWords(type());
}

Variant SSATmp::variantVal() const {
  switch (type().toDataType()) {
  case KindOfUninit:
    return uninit_null();
  case KindOfNull:
    return init_null();
  case KindOfBoolean:
    return boolVal();
  case KindOfInt64:
    return intVal();
  case KindOfDouble:
    return dblVal();
  case KindOfString:
  case KindOfStaticString:
    return Variant(strVal());
  case KindOfArray:
    return const_cast<ArrayData*>(arrVal());
  default:
    always_assert(false);
  }
}

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

}}
