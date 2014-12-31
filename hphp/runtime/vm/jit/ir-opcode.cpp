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

#include "hphp/runtime/vm/jit/ir-opcode.h"

#include <algorithm>
#include <cstring>
#include <forward_list>
#include <sstream>
#include <type_traits>

#include <folly/Format.h>
#include <folly/Traits.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/cfg.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

#define NF     0
#define C      CanCSE
#define Er     MayRaiseError
#define PRc    ProducesRC
#define CRc    ConsumesRC
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
#define DBox(n)        HasDest
#define DRefineS(n)    HasDest
#define DParamMayRelax HasDest
#define DParam         HasDest
#define DParamPtr(k)   HasDest
#define DUnboxPtr      HasDest
#define DBoxPtr        HasDest
#define DAllocObj      HasDest
#define DArrElem       HasDest
#define DArrPacked     HasDest
#define DThis          HasDest
#define DMulti         NaryDest
#define DSetElem       HasDest
#define DStk(x)        ModifiesStack|(x)
#define DPtrToParam    HasDest
#define DBuiltin       HasDest
#define DSubtract(n,t) HasDest
#define DCns           HasDest

OpInfo g_opInfo[] = {
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
#undef DBox
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DArrElem
#undef DArrPacked
#undef DAllocObj
#undef DThis
#undef DMulti
#undef DSetElem
#undef DStk
#undef DPtrToParam
#undef DBuiltin
#undef DSubtract
#undef DCns

//////////////////////////////////////////////////////////////////////

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

bool isCallOp(Opcode opc) {
  // CallBuiltin doesn't count because it is not a php-level call.  (It will
  // call a C++ helper and we can push/pop around it normally.)
  switch (opc) {
  case Call:
  case CallArray:
  case ContEnter:
    return true;
  default:
    return false;
  }
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
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
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

bool isDblQueryOp(Opcode opc) {
  switch (opc) {
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
    return true;
  default:
    return false;
  }
}

bool isQueryJmpOp(Opcode opc) {
  switch (opc) {
  case JmpZero:
  case JmpNZero:
    return true;
  default:
    return false;
  }
}

Opcode jmpToReqBindJmp(Opcode opc) {
  switch (opc) {
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
  case EqDbl:               return NeqDbl;
  case NeqDbl:              return EqDbl;
  case Same:                return NSame;
  case NSame:               return Same;
  case InstanceOfBitmask:   return NInstanceOfBitmask;
  case NInstanceOfBitmask:  return InstanceOfBitmask;
  case IsType:              return IsNType;
  case IsNType:             return IsType;
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
    // Negating dbl relational ops probably isn't what you want:
    // (X < Y) != !(X >= Y)  --  when NaN gets involved
    always_assert(false);
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
  case GtDbl: return LtDbl;
  case GteDbl:return LteDbl;
  case LtDbl: return GtDbl;
  case LteDbl:return GteDbl;
  case EqDbl: return EqDbl;
  case NeqDbl:return NeqDbl;
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
  case GtDbl: return GtInt;
  case GteDbl:return GteInt;
  case LtDbl: return LtInt;
  case LteDbl:return LteInt;
  case EqDbl: return EqInt;
  case NeqDbl:return NeqInt;
  default: always_assert(0);
  }
}

Opcode queryToDblQueryOp(Opcode opc) {
  assert(isQueryOp(opc));
  switch (opc) {
  case Gt:    return GtDbl;
  case Gte:   return GteDbl;
  case Lt:    return LtDbl;
  case Lte:   return LteDbl;
  case Eq:    return EqDbl;
  case Neq:   return NeqDbl;
  case GtInt: return GtDbl;
  case GteInt:return GteDbl;
  case LtInt: return LtDbl;
  case LteInt:return LteDbl;
  case EqInt: return EqDbl;
  case NeqInt:return NeqDbl;
  default: always_assert(0);
  }
}

int32_t spillValueCells(const IRInstruction* spillStack) {
  assert(spillStack->op() == SpillStack);
  int32_t numSrcs = spillStack->numSrcs();
  return numSrcs - 2;
}

}}
