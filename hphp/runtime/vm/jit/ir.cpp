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

#include "hphp/runtime/vm/jit/ir.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>
#include <boost/algorithm/string.hpp>

#include "folly/Format.h"
#include "folly/Traits.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ir-trace.h"
#include "hphp/runtime/vm/jit/cfg.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP {  namespace JIT {

using namespace HPHP::Transl;

#define IRT(name, ...) const Type Type::name(Type::k##name);
IR_TYPES
#undef IRT

std::string Type::toString() const {
  // Try to find an exact match to a predefined type
# define IRT(name, ...) if (*this == name) return #name;
  IR_TYPES
# undef IRT

  if (isBoxed()) {
    return folly::to<std::string>("Boxed", innerType().toString());
  }
  if (isPtr()) {
    return folly::to<std::string>("PtrTo", deref().toString());
  }

  auto t = *this;
  std::vector<std::string> parts;
  if (isSpecialized()) {
    if (canSpecializeClass()) {
      assert(m_class);
      parts.push_back(folly::to<std::string>(Type(m_bits & kAnyObj).toString(),
                                             '<', m_class->name()->data(),
                                             '>'));
      t -= AnyObj;
    } else if (canSpecializeArrayKind()) {
      assert(hasArrayKind());
      parts.push_back(
        folly::to<std::string>(Type(m_bits & kAnyArr).toString(), '<',
                               ArrayData::kindToString(m_arrayKind), '>'));
      t -= AnyArr;
    } else {
      not_reached();
    }
  }

  // Concat all of the primitive types in the custom union type
# define IRT(name, ...) if (name <= t) parts.push_back(#name);
  IRT_PRIMITIVE
# undef IRT
    assert(!parts.empty());
  if (parts.size() == 1) {
    return parts.front();
  }
  return folly::format("{{{}}}", folly::join('|', parts)).str();
}

std::string Type::debugString(Type t) {
  return t.toString();
}

Type Type::fromString(const std::string& str) {
  static hphp_string_map<Type> types;
  static bool init = false;
  if (UNLIKELY(!init)) {
#   define IRT(name, ...) types[#name] = name;
    IR_TYPES
#   undef IRT
    init = true;
  }
  return mapGet(types, str, Type::None);
}

TRACE_SET_MOD(hhir);

namespace {

#define NF     0
#define C      CanCSE
#define E      Essential
#define N      CallsNative
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define Refs   MayModifyRefs
#define Er     MayRaiseError
#define Mem    MemEffects
#define T      Terminal
#define P      Passthrough
#define K      KillsSources
#define StkFlags(f) HasStackVersion|(f)
#define MProp  MInstrProp
#define MElem  MInstrElem

#define ND        0
#define D(n)      HasDest
#define DofS(n)   HasDest
#define DUnbox(n) HasDest
#define DBox(n)   HasDest
#define DParam    HasDest
#define DArith    HasDest
#define DMulti    NaryDest
#define DSetElem  HasDest
#define DStk(x)   ModifiesStack|(x)
#define DPtrToParam HasDest
#define DBuiltin  HasDest
#define DSubtract(n,t) HasDest

struct {
  const char* name;
  uint64_t flags;
} OpInfo[] = {
#define O(name, dsts, srcs, flags)                    \
    { #name,                                          \
       (OpHasExtraData<name>::value ? HasExtra : 0) | \
       dsts | (flags)                                 \
    },
  IR_OPCODES
#undef O
  { 0 }
};

#undef NF
#undef C
#undef E
#undef PRc
#undef CRc
#undef Refs
#undef Er
#undef Mem
#undef T
#undef P
#undef K
#undef StkFlags
#undef MProp
#undef MElem

#undef ND
#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DParam
#undef DArith
#undef DMulti
#undef DSetElem
#undef DStk
#undef DPtrToParam
#undef DBuiltin
#undef DSubtract

} // namespace

//////////////////////////////////////////////////////////////////////

const char* opcodeName(Opcode opcode) {
  return OpInfo[uint16_t(opcode)].name;
}

bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return OpInfo[uint16_t(opcode)].flags & flags;
}

bool opHasExtraData(Opcode op) {
  return opcodeHasFlags(op, HasExtra);
}

Opcode getStackModifyingOpcode(Opcode opc) {
  assert(opcodeHasFlags(opc, HasStackVersion));
  opc = Opcode(uint64_t(opc) + 1);
  assert(opcodeHasFlags(opc, ModifiesStack));
  return opc;
}

const StringData* findClassName(SSATmp* cls) {
  assert(cls->isA(Type::Cls));

  if (cls->isConst()) {
    return cls->getValClass()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->src(0);
    assert(clsName->isA(Type::Str));
    if (clsName->isConst()) {
      return clsName->getValStr();
    }
  }
  return nullptr;
}

bool isGuardOp(Opcode opc) {
  switch (opc) {
    case GuardLoc:
    case CheckLoc:
    case GuardStk:
    case CheckStk:
    case CheckType:
      return true;

    default:
      return false;
  }
}

Opcode guardToAssert(Opcode opc) {
  switch (opc) {
    case GuardLoc:
    case CheckLoc:  return AssertLoc;
    case GuardStk:
    case CheckStk:  return AssertStk;
    case CheckType: return AssertType;

    default:        not_reached();
  }
}

bool isQueryOp(Opcode opc) {
  switch (opc) {
  case Gt:
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
  case Same:
  case NSame:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case IsType:
  case IsNType:
    return true;
  default:
    return false;
  }
}

bool isCmpOp(Opcode opc) {
  switch (opc) {
  case Gt:
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
  case Same:
  case NSame:
    return true;
  default:
    return false;
  }
}

bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpSame:
  case JmpNSame:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
  case JmpIsType:
  case JmpIsNType:
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

Opcode queryToJmpOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:                 return JmpGt;
  case Gte:                return JmpGte;
  case Lt:                 return JmpLt;
  case Lte:                return JmpLte;
  case Eq:                 return JmpEq;
  case Neq:                return JmpNeq;
  case Same:               return JmpSame;
  case NSame:              return JmpNSame;
  case InstanceOfBitmask:  return JmpInstanceOfBitmask;
  case NInstanceOfBitmask: return JmpNInstanceOfBitmask;
  case IsType:             return JmpIsType;
  case IsNType:            return JmpIsNType;
  default:                 always_assert(0);
  }
}

Opcode queryJmpToQueryOp(Opcode opc) {
  assert(isQueryJmpOp(opc));
  switch (opc) {
  case JmpGt:                 return Gt;
  case JmpGte:                return Gte;
  case JmpLt:                 return Lt;
  case JmpLte:                return Lte;
  case JmpEq:                 return Eq;
  case JmpNeq:                return Neq;
  case JmpSame:               return Same;
  case JmpNSame:              return NSame;
  case JmpInstanceOfBitmask:  return InstanceOfBitmask;
  case JmpNInstanceOfBitmask: return NInstanceOfBitmask;
  case JmpIsType:             return IsType;
  case JmpIsNType:            return IsNType;
  default:                    always_assert(0);
  }
}

Opcode jmpToReqBindJmp(Opcode opc) {
  switch (opc) {
  case JmpGt:                 return ReqBindJmpGt;
  case JmpGte:                return ReqBindJmpGte;
  case JmpLt:                 return ReqBindJmpLt;
  case JmpLte:                return ReqBindJmpLte;
  case JmpEq:                 return ReqBindJmpEq;
  case JmpNeq:                return ReqBindJmpNeq;
  case JmpSame:               return ReqBindJmpSame;
  case JmpNSame:              return ReqBindJmpNSame;
  case JmpInstanceOfBitmask:  return ReqBindJmpInstanceOfBitmask;
  case JmpNInstanceOfBitmask: return ReqBindJmpNInstanceOfBitmask;
  case JmpZero:               return ReqBindJmpZero;
  case JmpNZero:              return ReqBindJmpNZero;
  default:                    always_assert(0);
  }
}

Opcode negateQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:                  return Lte;
  case Gte:                 return Lt;
  case Lt:                  return Gte;
  case Lte:                 return Gt;
  case Eq:                  return Neq;
  case Neq:                 return Eq;
  case Same:                return NSame;
  case NSame:               return Same;
  case InstanceOfBitmask:   return NInstanceOfBitmask;
  case NInstanceOfBitmask:  return InstanceOfBitmask;
  case IsType:              return IsNType;
  case IsNType:             return IsType;
  default:                  always_assert(0);
  }
}

Opcode commuteQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:    return Lt;
  case Gte:   return Lte;
  case Lt:    return Gt;
  case Lte:   return Gte;
  case Eq:    return Eq;
  case Neq:   return Neq;
  case Same:  return Same;
  case NSame: return NSame;
  default:      always_assert(0);
  }
}

// Objects compared with strings may involve calling a user-defined
// __toString function.
bool cmpOpTypesMayReenter(Opcode op, Type t0, Type t1) {
  if (op == NSame || op == Same) return false;
  assert(!t0.equals(Type::Gen) && !t1.equals(Type::Gen));
  return (t0.maybe(Type::Obj) && t1.maybe(Type::Str)) ||
         (t0.maybe(Type::Str) && t1.maybe(Type::Obj));
}

bool isRefCounted(SSATmp* tmp) {
  return tmp->type().maybeCounted() && !tmp->isConst();
}

int32_t spillValueCells(IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->numSrcs();
  return numSrcs - 2;
}

bool isConvIntOrPtrToBool(IRInstruction* instr) {
  switch (instr->op()) {
    case ConvIntToBool:
      return true;
    case ConvCellToBool:
      return instr->src(0)->type().subtypeOfAny(
        Type::Func, Type::Cls, Type::FuncCls, Type::VarEnv, Type::TCA);
    default:
      return false;
  }
}

}}

