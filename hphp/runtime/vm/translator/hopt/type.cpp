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

#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT {

//////////////////////////////////////////////////////////////////////

Type::Tag outputType(const IRInstruction* inst) {
  assert(inst->hasDst() && "outputType requires inst->hasDst()");

  switch (inst->getOpcode()) {
  case Unbox:
    return Type::unbox(inst->getSrc(0)->getType());

  case Box:
    return Type::box(inst->getSrc(0)->getType());

  case StRef:
  case StRefNT:
    return Type::box(inst->getSrc(1)->getType());

  case AddElem:            return Type::Arr;
  case AddNewElem:         return Type::Arr;
  case ArrayAdd:           return Type::Arr;
  case AssertStk:          return Type::StkPtr;
  case Call:               return Type::StkPtr;
  case Concat:             return Type::Str;
  case CreateCont:         return Type::Obj;
  case DefActRec:          return Type::ActRec;
  case DefCns:             return Type::Bool;
  case DefFP:              return Type::StkPtr;
  case DefFunc:            return Type::FuncPtr;
  case DefSP:              return Type::StkPtr;
  case FreeActRec:         return Type::StkPtr;
  case GenericRetDecRefs:  return Type::StkPtr;
  case GuardStk:           return Type::StkPtr;
  case InstanceOf:         return Type::Bool;
  case InstanceOfBitmask:  return Type::Bool;
  case ExtendsClass:       return Type::Bool;
  case InterpOne:          return Type::StkPtr;
  case IsNSet:             return Type::Bool;
  case IsNType:            return Type::Bool;
  case IsSet:              return Type::Bool;
  case IsType:             return Type::Bool;
  case LdARFuncPtr:        return Type::FuncPtr;
  case LdCachedClass:      return Type::ClassPtr;
  case LdClsMethodCache:   return Type::FuncClassPtr;
  case LdClsMethod:        return Type::FuncPtr;
  case LdClsPropAddr:      return Type::PtrToGen;
  case LdCls:              return Type::ClassPtr;
  case LdContLocalsPtr:    return Type::PtrToCell;
  case LdCurFuncPtr:       return Type::FuncPtr;
  case LdFixedFunc:        return Type::FuncPtr;
  case LdFuncCls:          return Type::ClassPtr;
  case LdFunc:             return Type::FuncPtr;
  case LdHome:             return Type::Home;
  case LdLocAddr:          return Type::PtrToGen;
  case LdObjClass:         return Type::ClassPtr;
  case LdObjMethod:        return Type::FuncPtr;
  case LdPropAddr:         return Type::PtrToGen;
  case LdRetAddr:          return Type::RetAddr;
  case LdStackAddr:        return Type::PtrToGen;
  case LdThis:             return Type::Obj;
  case NewArray:           return Type::Arr;
  case NewObj:             return Type::StkPtr;
  case NewTuple:           return Type::Arr;
  case NInstanceOf:        return Type::Bool;
  case NInstanceOfBitmask: return Type::Bool;
  case RetAdjustStack:     return Type::StkPtr;
  case SpillStack:         return Type::StkPtr;
  case UnboxPtr:           return Type::PtrToCell;

  // Vector translator opcodes
  case DefMIStateBase:     return Type::PtrToCell;
  case PropX:              return Type::PtrToGen;
  case CGetProp:
  case CGetElem:           return Type::Cell;

  case OpAdd:
  case OpSub:
  case OpAnd:
  case OpOr:
  case OpXor:
  case OpMul:
    return Type::Int;

  case OpGt:
  case OpGte:
  case OpLt:
  case OpLte:
  case OpEq:
  case OpNeq:
  case OpSame:
  case OpNSame:
  case IterNext:
  case IterNextK:
  case IterInit:
  case IterInitK:
    return Type::Bool;

  // Jumps have dests just to allow finding the TCA to patch.  The
  // dest SSATmp doesn't actually carry any value.
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpEq:
  case JmpNeq:
  case JmpZero:
  case JmpNZero:
  case JmpSame:
  case JmpNSame:
  case JmpInstanceOf:
  case JmpNInstanceOf:
  case JmpInstanceOfBitmask:
  case JmpNInstanceOfBitmask:
  case JmpIsSet:
  case JmpIsType:
  case JmpIsNSet:
  case JmpIsNType:
  case Jmp_:
    return Type::None;

  // Output type is the same as the first input's type.
  case Mov:
  case IncRef:
  case Spill:
  case Reload:
  case LdAddr:
    return inst->getSrc(0)->getType();

  // Output type is given by a type paramter to the instruction.
  case Conv:
  case LdClsCns:
  case LdLoc:
  case LdMem:
  case LdProp:
  case LdRaw:
  case LdRef:
  case LdStack:
  case DefConst:
  case LdConst:
  case GuardType:
    return inst->getTypeParam();

  default:
    always_assert(0 && "outputType not known for opcode");
  }
}

void assertOperandTypes(const IRInstruction* inst) {
#ifdef DEBUG
  auto const tparam = inst->getTypeParam();
  auto const s0 = inst->getSrc(0);
  auto const s1 = inst->getSrc(1);
  auto const t0 = s0 ? s0->getType() : Type::None;
  auto const t1 = s1 ? s1->getType() : Type::None;

  auto constStaticStr = [] (SSATmp* ssa) {
    assert(ssa->isConst() && ssa->getType() == Type::StaticStr);
  };

  auto constInt = [] (SSATmp* ssa) {
    assert(ssa->isConst() && ssa->getType() == Type::Int);
  };

  switch (inst->getOpcode()) {
  case OpAdd:
  case OpSub:
  case OpAnd:
  case OpOr:
  case OpXor:
  case OpMul:
    assert(t0 == Type::Int || t0 == Type::Bool);
    assert(t1 == Type::Int || t1 == Type::Bool);
    break;

  case LdRaw:
    assert(tparam == Type::Int || tparam == Type::Bool ||
           tparam == Type::StkPtr || tparam == Type::TCA);
    break;

  case LdPropAddr:
    assert(t0 == Type::Obj);
    constInt(s1);
    break;

  case LdCls:
    constStaticStr(s0);
    break;

  case LdClsCns:
    constStaticStr(s0);
    constStaticStr(s1);
    break;

  case LdObjClass:
    assert(t0 == Type::Obj);
    break;

  case StRaw: {
    auto valT = inst->getSrc(2)->getType();
    assert(valT == Type::Int || valT == Type::Bool);
    break;
  }

  case StMem:
    assert(t0 == Type::PtrToCell || t0 == Type::PtrToGen);
    break;

  default:
    break;
  }
#endif
}

//////////////////////////////////////////////////////////////////////

}}}

