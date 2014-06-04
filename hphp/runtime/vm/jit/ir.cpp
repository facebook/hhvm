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

#include "hphp/runtime/vm/jit/ir.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>

#include "folly/Format.h"
#include "folly/Traits.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/cfg.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP {  namespace JIT {


TRACE_SET_MOD(hhir);

namespace {

#define NF     0
#define C      CanCSE
#define E      Essential
#define N      CallsNative
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define Er     MayRaiseError
#define T      Terminal
#define B      Branch
#define P      Passthrough
#define K      KillsSources
#define StkFlags(f) HasStackVersion|(f)
#define MProp  MInstrProp
#define MElem  MInstrElem

#define ND             0
#define D(n)           HasDest
#define DofS(n)        HasDest
#define DUnbox(n)      HasDest
#define DBox(n)        HasDest
#define DRefineS(n)    HasDest
#define DParam         HasDest
#define DAllocObj      HasDest
#define DArrElem       HasDest
#define DArrPacked     HasDest
#define DLdRef         HasDest
#define DThis          HasDest
#define DMulti         NaryDest
#define DSetElem       HasDest
#define DStk(x)        ModifiesStack|(x)
#define DPtrToParam    HasDest
#define DBuiltin       HasDest
#define DSubtract(n,t) HasDest
#define DLdRaw         HasDest
#define DCns           HasDest

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
#undef Er
#undef T
#undef B
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
#undef DRefineS
#undef DParam
#undef DArrElem
#undef DArrPacked
#undef DAllocObj
#undef DLdRef
#undef DThis
#undef DMulti
#undef DSetElem
#undef DStk
#undef DPtrToParam
#undef DBuiltin
#undef DSubtract
#undef DLdRaw
#undef DCns

} // namespace

//////////////////////////////////////////////////////////////////////

const char* opcodeName(Opcode opcode) {
  return OpInfo[uint16_t(opcode)].name;
}

bool opcodeHasFlags(Opcode opcode, uint64_t flags) {
  return OpInfo[uint16_t(opcode)].flags & flags;
}

bool hasEdges(Opcode opcode) {
  return opcodeHasFlags(opcode, Branch | MayRaiseError);
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
    return cls->clsVal()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->src(0);
    assert(clsName->isA(Type::Str));
    if (clsName->isConst()) {
      return clsName->strVal();
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

bool isQueryOp(Opcode opc) {
  switch (opc) {
  case Gt:
  case Gte:
  case Lt:
  case Lte:
  case Eq:
  case Neq:
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
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

bool isIntQueryOp(Opcode opc) {
  switch (opc) {
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
    return true;
  default:
    return false;
  }
}

bool isFusableQueryOp(Opcode opc) {
  return isQueryOp(opc) && opc != IsType && opc != IsNType;
}

bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpGtInt:
  case JmpGteInt:
  case JmpLtInt:
  case JmpLteInt:
  case JmpEqInt:
  case JmpNeqInt:
  case JmpSame:
  case JmpNSame:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

Opcode queryToJmpOp(Opcode opc) {
  assert(isFusableQueryOp(opc));
  switch (opc) {
  case Gt:                 return JmpGt;
  case Gte:                return JmpGte;
  case Lt:                 return JmpLt;
  case Lte:                return JmpLte;
  case Eq:                 return JmpEq;
  case Neq:                return JmpNeq;
  case GtInt:              return JmpGtInt;
  case GteInt:             return JmpGteInt;
  case LtInt:              return JmpLtInt;
  case LteInt:             return JmpLteInt;
  case EqInt:              return JmpEqInt;
  case NeqInt:             return JmpNeqInt;
  case Same:               return JmpSame;
  case NSame:              return JmpNSame;
  case InstanceOfBitmask:  return JmpInstanceOfBitmask;
  case NInstanceOfBitmask: return JmpNInstanceOfBitmask;
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
  case JmpGtInt:              return GtInt;
  case JmpGteInt:             return GteInt;
  case JmpLtInt:              return LtInt;
  case JmpLteInt:             return LteInt;
  case JmpEqInt:              return EqInt;
  case JmpNeqInt:             return NeqInt;
  case JmpSame:               return Same;
  case JmpNSame:              return NSame;
  case JmpInstanceOfBitmask:  return InstanceOfBitmask;
  case JmpNInstanceOfBitmask: return NInstanceOfBitmask;
  default:                    always_assert(0);
  }
}

Opcode jmpToSideExitJmp(Opcode opc) {
  switch (opc) {
  case JmpGt:                 return SideExitJmpGt;
  case JmpGte:                return SideExitJmpGte;
  case JmpLt:                 return SideExitJmpLt;
  case JmpLte:                return SideExitJmpLte;
  case JmpEq:                 return SideExitJmpEq;
  case JmpNeq:                return SideExitJmpNeq;
  case JmpGtInt:              return SideExitJmpGtInt;
  case JmpGteInt:             return SideExitJmpGteInt;
  case JmpLtInt:              return SideExitJmpLtInt;
  case JmpLteInt:             return SideExitJmpLteInt;
  case JmpEqInt:              return SideExitJmpEqInt;
  case JmpNeqInt:             return SideExitJmpNeqInt;
  case JmpSame:               return SideExitJmpSame;
  case JmpNSame:              return SideExitJmpNSame;
  case JmpInstanceOfBitmask:  return SideExitJmpInstanceOfBitmask;
  case JmpNInstanceOfBitmask: return SideExitJmpNInstanceOfBitmask;
  case JmpZero:               return SideExitJmpZero;
  case JmpNZero:              return SideExitJmpNZero;
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
  case JmpGtInt:              return ReqBindJmpGtInt;
  case JmpGteInt:             return ReqBindJmpGteInt;
  case JmpLtInt:              return ReqBindJmpLtInt;
  case JmpLteInt:             return ReqBindJmpLteInt;
  case JmpEqInt:              return ReqBindJmpEqInt;
  case JmpNeqInt:             return ReqBindJmpNeqInt;
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
  case GtInt:               return LteInt;
  case GteInt:              return LtInt;
  case LtInt:               return GteInt;
  case LteInt:              return GtInt;
  case EqInt:               return NeqInt;
  case NeqInt:              return EqInt;
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
  case GtInt: return LtInt;
  case GteInt:return LteInt;
  case LtInt: return GtInt;
  case LteInt:return GteInt;
  case EqInt: return EqInt;
  case NeqInt:return NeqInt;
  case Same:  return Same;
  case NSame: return NSame;
  default:      always_assert(0);
  }
}

Opcode queryToIntQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:    return GtInt;
  case Gte:   return GteInt;
  case Lt:    return LtInt;
  case Lte:   return LteInt;
  case Eq:    return EqInt;
  case Neq:   return NeqInt;
  case JmpGt:    return JmpGtInt;
  case JmpGte:   return JmpGteInt;
  case JmpLt:    return JmpLtInt;
  case JmpLte:   return JmpLteInt;
  case JmpEq:    return JmpEqInt;
  case JmpNeq:   return JmpNeqInt;
  case SideExitJmpGt:   return SideExitJmpGtInt;
  case SideExitJmpGte:  return SideExitJmpGteInt;
  case SideExitJmpLt:   return SideExitJmpLtInt;
  case SideExitJmpLte:  return SideExitJmpLteInt;
  case SideExitJmpEq:   return SideExitJmpEqInt;
  case SideExitJmpNeq:  return SideExitJmpNeqInt;
  case ReqBindJmpGt:    return ReqBindJmpGtInt;
  case ReqBindJmpGte:   return ReqBindJmpGteInt;
  case ReqBindJmpLt:    return ReqBindJmpLtInt;
  case ReqBindJmpLte:   return ReqBindJmpLteInt;
  case ReqBindJmpEq:    return ReqBindJmpEqInt;
  case ReqBindJmpNeq:   return ReqBindJmpNeqInt;
  default: always_assert(0);
  }
}

bool isRefCounted(SSATmp* tmp) {
  return tmp->type().maybeCounted() && !tmp->isConst();
}

int32_t spillValueCells(const IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->numSrcs();
  return numSrcs - 2;
}

}}
