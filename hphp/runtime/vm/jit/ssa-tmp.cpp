/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <sstream>

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
  if (t.subtypeOfAny(TUninit,
                     TInitNull,
                     TNullptr)) {
    // These don't need a register because their values are static or unused.
    return 0;
  }
  if (t.maybe(TNullptr)) {
    return typeNeededWords(t - TNullptr);
  }
  if (t <= TCtx || t <= TPtrToGen) {
    // Ctx and PtrTo* may be statically unknown but always need just one
    // register.
    return 1;
  }
  if (t <= TLvalToGen) {
    // If tv_val<> is ever anything other than 1 or more normal pointers, this
    // will need to change.
    static_assert(sizeof(tv_lval) % 8 == 0, "");
    return sizeof(tv_lval) / 8;
  }
  if (!t.isUnion()) {
    // Not a union type and not a special case: 1 register.
    assertx(IMPLIES(t <= TGen, t.isKnownDataType()));
    return 1;
  }

  assertx(t <= TGen);

  // XXX(t4592459): This will return 2 for TNull, even though it only
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
    case KindOfPersistentString:
      return Variant{strVal(), Variant::PersistentStrInit{}};
    case KindOfPersistentVec:
      return Variant{vecVal(), KindOfPersistentVec,
                     Variant::PersistentArrInit{}};
    case KindOfPersistentDict:
      return Variant{dictVal(), KindOfPersistentDict,
                     Variant::PersistentArrInit{}};
    case KindOfPersistentKeyset:
      return Variant{keysetVal(), KindOfPersistentKeyset,
                     Variant::PersistentArrInit{}};
    case KindOfPersistentShape:
      return Variant{shapeVal(), KindOfPersistentShape,
                     Variant::PersistentArrInit{}};
    case KindOfPersistentArray:
      return Variant{arrVal(), KindOfPersistentArray,
                     Variant::PersistentArrInit{}};
    case KindOfString:
    case KindOfVec:
    case KindOfDict:
    case KindOfKeyset:
    case KindOfShape:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    // TODO (T29639296)
    case KindOfFunc:
    case KindOfClass:
      break;
  }
  always_assert(false);
}

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

}}
