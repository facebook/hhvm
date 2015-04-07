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

namespace HPHP { namespace jit {

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
  if (t.subtypeOfAny(Type::Uninit,
                     Type::InitNull,
                     Type::Nullptr)) {
    // These don't need a register because their values are static or unused.
    return 0;
  }
  if (t.maybe(Type::Nullptr)) {
    return typeNeededWords(t - Type::Nullptr);
  }
  if (t <= Type::Ctx || t <= Type::PtrToGen) {
    // Ctx and PtrTo* may be statically unknown but always need just 1 register.
    return 1;
  }
  if (!t.isUnion()) {
    // Not a union type and not a special case: 1 register.
    assertx(IMPLIES(t <= Type::StkElem, t.isKnownDataType()));
    return 1;
  }

  assertx(t <= Type::StkElem);

  // XXX(t4592459): This will return 2 for Type::Null, even though it only
  // needs 1 register (one for the type, none for the value). This is to work
  // around limitations in codegen; see the task for details. It does mean we
  // will be loading and storing garbage m_data for Null values but that's fine
  // since m_data is undefined for Null values.
  return t.needsReg() ? 2 : 1;
}
}

int SSATmp::numWords() const {
  return typeNeededWords(type());
}

Variant SSATmp::variantVal() const {
  switch (type().toDataType()) {
    case KindOfUninit:
    case KindOfNull:
      // Upon return both will converted to KindOfNull anyway.
      return init_null();
    case KindOfBoolean:
      return boolVal();
    case KindOfInt64:
      return intVal();
    case KindOfDouble:
      return dblVal();
    case KindOfStaticString:
    case KindOfString:
      return Variant(const_cast<StringData*>(strVal()));
    case KindOfArray:
      return const_cast<ArrayData*>(arrVal());
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

}}
