/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/trace.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

#define NF     0
#define Er     MayRaiseError
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define T      Terminal
#define B      Branch
#define P      Passthrough
#define MProp  MInstrProp
#define MElem  MInstrElem

#define ND             0
#define D(n)           HasDest
#define DofS(n)        HasDest
#define DRefineS(n)    HasDest
#define DParamMayRelax HasDest
#define DParam         HasDest
#define DParamPtr(k)   HasDest
#define DUnboxPtr      HasDest
#define DBoxPtr        HasDest
#define DAllocObj      HasDest
#define DArrElem       HasDest
#define DArrPacked     HasDest
#define DCol           HasDest
#define DThis          HasDest
#define DCtx           HasDest
#define DMulti         NaryDest
#define DSetElem       HasDest
#define DPtrToParam    HasDest
#define DBuiltin       HasDest
#define DSubtract(n,t) HasDest
#define DCns           HasDest

namespace {
template<Opcode op, uint64_t flags>
struct op_flags {
  static constexpr uint64_t value =
    (OpHasExtraData<op>::value ? HasExtra : 0) | flags;

  static_assert(!(value & ProducesRC) ||
                (value & (HasDest | NaryDest)) == HasDest,
                "ProducesRC instructions must have exactly one dest");
};
}

OpInfo g_opInfo[] = {
#define O(name, dsts, srcs, flags)                    \
    { #name,                                          \
       op_flags<name, dsts | flags>::value            \
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
#undef MProp
#undef MElem

#undef ND
#undef D
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DArrElem
#undef DArrPacked
#undef DCol
#undef DAllocObj
#undef DThis
#undef DCtx
#undef DMulti
#undef DSetElem
#undef DPtrToParam
#undef DBuiltin
#undef DSubtract
#undef DCns

///////////////////////////////////////////////////////////////////////////////

const StringData* findClassName(SSATmp* cls) {
  assertx(cls->isA(TCls));

  if (cls->hasConstVal()) {
    return cls->clsVal()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->src(0);
    assertx(clsName->isA(TStr));
    if (clsName->hasConstVal()) {
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
    case CheckLoc:
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
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
  case GtBool:
  case GteBool:
  case LtBool:
  case LteBool:
  case EqBool:
  case NeqBool:
  case SameObj:
  case NSameObj:
  case SameArr:
  case NSameArr:
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

bool isSideEffectfulQueryOp(Opcode opc) {
  switch (opc) {
  case GtX:
  case GteX:
  case LtX:
  case LteX:
  case EqX:
  case NeqX:
  case GtObj:
  case GteObj:
  case LtObj:
  case LteObj:
  case EqObj:
  case NeqObj:
  case GtArr:
  case GteArr:
  case LtArr:
  case LteArr:
  case EqArr:
  case NeqArr:
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

bool isBoolQueryOp(Opcode opc) {
  switch (opc) {
  case GtBool:
  case GteBool:
  case LtBool:
  case LteBool:
  case EqBool:
  case NeqBool:
    return true;
  default:
    return false;
  }
}

bool isStrQueryOp(Opcode opc) {
  switch (opc) {
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
    return true;
  default:
   return false;
  }
}

bool isObjQueryOp(Opcode opc) {
  switch (opc) {
  case GtObj:
  case GteObj:
  case LtObj:
  case LteObj:
  case EqObj:
  case NeqObj:
  case SameObj:
  case NSameObj:
    return true;
  default:
    return false;
  }
}

bool isArrQueryOp(Opcode opc) {
  switch (opc) {
  case GtArr:
  case GteArr:
  case LtArr:
  case LteArr:
  case EqArr:
  case NeqArr:
  case SameArr:
  case NSameArr:
    return true;
  default:
    return false;
  }
}

Opcode negateQueryOp(Opcode opc) {
  assertx(isQueryOp(opc));
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
  case GtStr:               return LteStr;
  case GteStr:              return LtStr;
  case LtStr:               return GteStr;
  case LteStr:              return GtStr;
  case EqStr:               return NeqStr;
  case NeqStr:              return EqStr;
  case SameStr:             return NSameStr;
  case NSameStr:            return SameStr;
  case GtBool:              return LteBool;
  case GteBool:             return LtBool;
  case LtBool:              return GteBool;
  case LteBool:             return GtBool;
  case EqBool:              return NeqBool;
  case NeqBool:             return EqBool;
  case SameObj:             return NSameObj;
  case NSameObj:            return SameObj;
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
  assertx(isQueryOp(opc) || isSideEffectfulQueryOp(opc));
  switch (opc) {
  case Gt:    return Lt;
  case GtX:   return LtX;
  case Gte:   return Lte;
  case GteX:  return LteX;
  case Lt:    return Gt;
  case LtX:   return GtX;
  case Lte:   return Gte;
  case LteX:  return GteX;
  case Eq:    return Eq;
  case EqX:   return EqX;
  case Neq:   return Neq;
  case NeqX:  return NeqX;
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
  case GtStr: return LtStr;
  case GteStr:return LteStr;
  case LtStr: return GtStr;
  case LteStr:return GteStr;
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
    return opc;
  case GtBool:  return LtBool;
  case GteBool: return LteBool;
  case LtBool:  return GtBool;
  case LteBool: return GteBool;
  case EqBool:
  case NeqBool:
    return opc;
  case GtObj:  return LtObj;
  case GteObj: return LteObj;
  case LtObj:  return GtObj;
  case LteObj: return GteObj;
  case EqObj:
  case NeqObj:
  case SameObj:
  case NSameObj:
    return opc;
  case GtArr:  return LtArr;
  case GteArr: return LteArr;
  case LtArr:  return GtArr;
  case LteArr: return GteArr;
  case EqArr:
  case NeqArr:
  case SameArr:
  case NSameArr:
    return opc;
  case Same:  return Same;
  case NSame: return NSame;
  default:      always_assert(0);
  }
}

Opcode queryToIntQueryOp(Opcode opc) {
  assertx(isQueryOp(opc));
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
  assertx(isQueryOp(opc));
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

Opcode queryToBoolQueryOp(Opcode opc) {
  assertx(isQueryOp(opc));
  switch (opc) {
    case Gt:  return GtBool;
    case Gte: return GteBool;
    case Lt:  return LtBool;
    case Lte: return LteBool;
    case Eq:  return EqBool;
    case Neq: return NeqBool;
    case Same: return EqBool;
    case NSame: return NeqBool;
    default: always_assert(0);
  }
}

Opcode queryToStrQueryOp(Opcode opc) {
  assertx(isQueryOp(opc));
  switch (opc) {
    case Gt: return GtStr;
    case Gte: return GteStr;
    case Lt: return LtStr;
    case Lte: return LteStr;
    case Eq: return EqStr;
    case Neq: return NeqStr;
    case Same: return SameStr;
    case NSame: return NSameStr;
    default: always_assert(0);
  }
}

Opcode queryToObjQueryOp(Opcode opc) {
  assertx(isQueryOp(opc) || isSideEffectfulQueryOp(opc));
  switch (opc) {
    case GtX:
    case Gt: return GtObj;
    case GteX:
    case Gte: return GteObj;
    case LtX:
    case Lt: return LtObj;
    case LteX:
    case Lte: return LteObj;
    case EqX:
    case Eq: return EqObj;
    case NeqX:
    case Neq: return NeqObj;
    case Same: return SameObj;
    case NSame: return NSameObj;
    default: always_assert(0);
  }
}

Opcode queryToArrQueryOp(Opcode opc) {
  assertx(isQueryOp(opc) || isSideEffectfulQueryOp(opc));
  switch (opc) {
    case GtX:
    case Gt: return GtArr;
    case GteX:
    case Gte: return GteArr;
    case LtX:
    case Lt: return LtArr;
    case LteX:
    case Lte: return LteArr;
    case EqX:
    case Eq: return EqArr;
    case NeqX:
    case Neq: return NeqArr;
    case Same: return SameArr;
    case NSame: return NSameArr;
    default: always_assert(0);
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
