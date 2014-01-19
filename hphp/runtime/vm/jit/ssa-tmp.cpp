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

#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/print.h"

namespace HPHP { namespace JIT {

SSATmp::SSATmp(uint32_t opndId, IRInstruction* i, int dstId /* = 0 */)
  : m_inst(i)
  , m_type(outputType(i, dstId))
  , m_id(opndId)
{
}

bool SSATmp::isConst() const {
  return m_inst->op() == DefConst ||
    m_inst->op() == LdConst;
}

namespace {
int typeNeededWords(Type t) {
  assert(!t.equals(Type::Bottom));

  if (t.subtypeOfAny(Type::None, Type::Null, Type::ActRec, Type::RetAddr,
                     Type::Nullptr)) {
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
  if (t <= Type::FuncCtx) {
    // 2 registers regardless of union status: 1 for the Func* and 1
    // for the {Obj|Cctx}, differentiated by the low bit.
    return 2;
  }
  if (!t.isUnion()) {
    // Not a union type and not a special case: 1 register.
    assert(IMPLIES(t <= Type::Gen, t.isKnownDataType()));
    return 1;
  }

  assert(t <= Type::Gen);
  return t.needsReg() ? 2 : 1;
}
}

int SSATmp::numWords() const {
  return typeNeededWords(type());
}

bool SSATmp::getValBool() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Bool));
  return m_inst->extra<ConstData>()->as<bool>();
}

int64_t SSATmp::getValInt() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Int));
  return m_inst->extra<ConstData>()->as<int64_t>();
}

int64_t SSATmp::getValRawInt() const {
  assert(isConst());
  return m_inst->extra<ConstData>()->as<int64_t>();
}

double SSATmp::getValDbl() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Dbl));
  return m_inst->extra<ConstData>()->as<double>();
}

const StringData* SSATmp::getValStr() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::StaticStr));
  return m_inst->extra<ConstData>()->as<const StringData*>();
}

const ArrayData* SSATmp::getValArr() const {
  assert(isConst());
  // TODO: Task #2124292, Reintroduce StaticArr
  assert(m_inst->typeParam() <= Type::Arr);
  return m_inst->extra<ConstData>()->as<const ArrayData*>();
}

const Func* SSATmp::getValFunc() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Func));
  return m_inst->extra<ConstData>()->as<const Func*>();
}

const Class* SSATmp::getValClass() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Cls));
  return m_inst->extra<ConstData>()->as<const Class*>();
}

const NamedEntity* SSATmp::getValNamedEntity() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::NamedEntity));
  return m_inst->extra<ConstData>()->as<const NamedEntity*>();
}

RDS::Handle SSATmp::getValRDSHandle() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::RDSHandle));
  return m_inst->extra<ConstData>()->as<RDS::Handle>();
}

uintptr_t SSATmp::getValBits() const {
  assert(isConst());
  return m_inst->extra<ConstData>()->as<uintptr_t>();
}

Variant SSATmp::getValVariant() const {
  switch (m_inst->typeParam().toDataType()) {
  case KindOfUninit:
    return uninit_null();
  case KindOfNull:
    return init_null();
  case KindOfBoolean:
    return getValBool();
  case KindOfInt64:
    return getValInt();
  case KindOfDouble:
    return getValDbl();
  case KindOfString:
  case KindOfStaticString:
    return Variant(getValStr());
  case KindOfArray:
    return const_cast<ArrayData*>(getValArr());
  default:
    always_assert(false);
  }
}

TCA SSATmp::getValTCA() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::TCA));
  return m_inst->extra<ConstData>()->as<TCA>();
}

uintptr_t SSATmp::getValCctx() const {
  assert(isConst());
  assert(m_inst->typeParam().equals(Type::Cctx));
  return m_inst->extra<ConstData>()->as<uintptr_t>();
}

std::string SSATmp::toString() const {
  std::ostringstream out;
  print(out, this);
  return out.str();
}

}}
