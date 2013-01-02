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

#include "tracebuilder.h"

#include <util/trace.h>

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

void TraceBuilder::start(uint32 initialBcOffset, uint32 initialSpOffsetFromFp) {
  m_initialBcOff = initialBcOffset;
  m_fpValue = genDefFP();
  m_spValue = genDefSP();
  m_spOffset = initialSpOffsetFromFp;
  ASSERT(m_spOffset >= 0);
  genExitGuardFailure(initialBcOffset);
}

void TraceBuilder::genPrint(SSATmp* arg) {
  if (arg->getType() == Type::Null) {
    return;
  }
  genInstruction(Print, Type::None, arg);
}

SSATmp* TraceBuilder::genAddElem(SSATmp* arr, SSATmp* key, SSATmp* val) {
  return genInstruction(AddElem, Type::Arr, arr, key, val);
}

SSATmp* TraceBuilder::genAddNewElem(SSATmp* arr, SSATmp* val) {
  return genInstruction(AddNewElem, Type::Arr, arr, val);
}

SSATmp* TraceBuilder::genDefCns(const StringData* cnsName, SSATmp* val) {
  return genInstruction(DefCns, Type::Bool,
                        genDefConst<const StringData*>(cnsName),
                        val);
}

SSATmp* TraceBuilder::genConcat(SSATmp* tl, SSATmp* tr) {
  return genInstruction(Concat, Type::Str, tl, tr);
}

SSATmp* TraceBuilder::genArrayAdd(SSATmp* tl, SSATmp* tr) {
  ASSERT(tl->getType() == Type::Arr && tr->getType() == Type::Arr);
  return genInstruction(ArrayAdd, Type::Arr, tl, tr);
}

void TraceBuilder::genDefCls(PreClass* clss, const HPHP::VM::Opcode* after) {
  PUNT(DefCls);
}

void TraceBuilder::genDefFunc(Func* func) {
  genInstruction(DefFunc, Type::FuncRef, genDefConst<const Func*>(func));
}

SSATmp* TraceBuilder::genLdThis(Trace* exitTrace) {
  if (m_thisIsAvailable) { // mov m_thisIsAvailable to opt code
    return genInstruction(LdThis, Type::Obj, m_fpValue);
  } else {
    // future LdThis instructions don't need a null check
    m_thisIsAvailable = true;
    return genInstruction(LdThis, Type::Obj, m_fpValue, exitTrace);
  }
}

SSATmp* TraceBuilder::genLdProp(SSATmp* obj,
                                SSATmp* prop,
                                Type::Tag type,
                                Trace* exit) {
  ASSERT(obj->getType() == Type::Obj);
  ASSERT(prop->getType() == Type::Int);
  ASSERT(prop->isConst());
  return genInstruction(LdPropNR, type, obj, prop, exit);
}

void TraceBuilder::genStProp(SSATmp* obj,
                             SSATmp* prop,
                             SSATmp* src,
                             bool genStoreType) {
  Opcode opc = genStoreType ? StProp : StPropNT;
  genInstruction(opc, src->getType(), obj, prop, src);
}

void TraceBuilder::genStMem(SSATmp* addr,
                            SSATmp* src,
                            bool genStoreType) {
  genStMem(addr, 0, src, genStoreType);
}

void TraceBuilder::genStMem(SSATmp* addr,
                            int64 offset,
                            SSATmp* src,
                            bool genStoreType) {
  ASSERT(addr->getType() == Type::PtrToCell ||
         addr->getType() == Type::PtrToGen);
  Opcode opc = genStoreType ? StMem : StMemNT;
  genInstruction(opc, Type::None, addr, genDefConst(offset), src);
}

void TraceBuilder::genSetPropCell(SSATmp* base, int64 offset, SSATmp* value) {
  SSATmp* oldVal = genLdProp(base, genDefConst(offset), Type::Cell, NULL);
  genStProp(base, genDefConst(offset), value, true);
  genDecRef(oldVal);
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               Type::Tag type,
                               Trace* target) {
  return genLdMem(addr, 0, type, target);
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               int64 offset,
                               Type::Tag type,
                               Trace* target) {
  ASSERT(addr->getType() == Type::PtrToCell ||
         addr->getType() == Type::PtrToGen);
  return genInstruction(LdMemNR, type, addr, genDefConst(offset),  target);
}

SSATmp* TraceBuilder::genLdRef(SSATmp* ref, Type::Tag type, Trace* exit) {
  ASSERT(Type::isUnboxed(type));
  ASSERT(Type::isBoxed(ref->getType()));
  return genInstruction(LdRefNR, type, ref, exit);
}

SSATmp* TraceBuilder::genUnboxPtr(SSATmp* ptr) {
  if (ptr->getType() == Type::PtrToCell) {
    return ptr;
  }
  ASSERT(ptr->getType() == Type::PtrToGen);
  return genInstruction(UnboxPtr, Type::PtrToCell, ptr);
}

SSATmp*
TraceBuilder::genUnbox(SSATmp* src, Type::Tag type, Trace* exit) {
  ASSERT(Type::isUnboxed(type));
  Type::Tag srcType = src->getType();
  // TODO: Similar logic in simplifer. Clean up any redundancy.
  if (Type::isBoxed(srcType)) {
    srcType = Type::getInnerType(srcType);
    if (Type::isMoreRefined(srcType, type)) {
      type = srcType;
    }
    return genLdRef(src, type, exit);
  }
  return genInstruction(Unbox, type, src, exit);
}


/**
 * Checks is the given SSATmp, or any of its aliases, is available in
 * any VM location, including locals and the This pointer.
 */
bool TraceBuilder::isValueAvailable(SSATmp* tmp) const {
  while (true) {
    if (anyLocalHasValue(tmp)) return true;

    IRInstruction* srcInstr = tmp->getInstruction();
    Opcode srcOpcode = srcInstr->getOpcode();

    if (srcOpcode == LdThis) return true;

    if (srcOpcode == IncRef || srcOpcode == Mov) {
      tmp = srcInstr->getSrc(0);
    } else {
      return false;
    }
  }
}

void TraceBuilder::genDecRef(SSATmp* tmp) {
  if (!isRefCounted(tmp)) {
    return;
  }

  Type::Tag type = tmp->getType();
  if (Type::isBoxed(type)) {
    // we can't really rely on the types held in the boxed values since
    // aliasing stores may change them. We conservatively set the type
    // of the decref to a boxed cell and rely on later optimizations to
    // refine it based on alias analysis.
    type = Type::BoxedCell;
  }

  // refcount optimization:
  // If the decref'ed value is guaranteed to be available after the decref,
  // generate DecRefNZ instead of DecRef.
  // We could do more accurate availability analysis. For now, we handle
  // simple cases:
  // 1) LdThis is always available.
  // 2) A value stored in a local is always available.
  IRInstruction* incRefInst = tmp->getInstruction();
  if (incRefInst->getOpcode() == IncRef) {
    if (isValueAvailable(incRefInst->getSrc(0))) {
      genInstruction(DecRefNZ, type, tmp);
      return;
    }
  }

  genInstruction(DecRef, type, tmp);
}

/*
 * Code generation support for side exits.
 * There are 3 types of side exits as defined by the ExitType enum:
 * (1) Normal: Conditional or unconditional program branches
 *     that take you out of the trace.
 * (2) Slow: branches to slow paths to handle rare and slow cases
 *     such as null check failures, warnings, fatals, or type guard
 *     failures in the middle of a trace.
 * (3) GuardFailure: branches due to guard failures at the beginning
 *     of a trace.
 */

/*
 * genExitGuardFailure generates a target exit trace for GuardFailure
 * exits. TraceBuilder::start calls this once at the start of trace
 * code generation.
 */
void TraceBuilder::genExitGuardFailure(uint32 bcOff) {
  m_exitGuardFailureTrace = makeExitTrace(bcOff);
  LabelInstruction* markerInst =
    m_irFactory.marker(bcOff, m_curFunc->getConstValAsFunc(), m_spOffset);
  m_exitGuardFailureTrace->appendInstruction(markerInst);
  SSATmp* pc = genDefConst<int64>((int64)bcOff);
  // TODO change exit trace to a control flow instruction that
  // takes sp, fp, and a Marker as the target label instruction
  m_exitGuardFailureTrace->appendInstruction(
    m_irFactory.exitTrace(TraceExitType::GuardFailure,
                          m_curFunc,
                          pc,
                          m_spValue,
                          m_fpValue));
}

/*
 * genExitSlow generates a target exit trace for TraceExitType::Slow branches.
 */
Trace* TraceBuilder::getExitSlowTrace(uint32 bcOff,
                                      int32 stackDeficit,
                                      uint32 numOpnds,
                                      SSATmp** opnds) {
  // this is a newly created check with no label
  TraceExitType::ExitType exitType =
    bcOff == m_initialBcOff ? TraceExitType::SlowNoProgress
                            : TraceExitType::Slow;
  return genExitTrace(bcOff, stackDeficit, numOpnds, opnds, exitType);

}

SSATmp* TraceBuilder::genLdVarEnv() {
  return genInstruction(LdVarEnv, Type::VarEnvRef, m_fpValue);
}

SSATmp* TraceBuilder::genLdRetAddr() {
  return genInstruction(LdRetAddr, Type::None, m_fpValue);
}

SSATmp* TraceBuilder::genLdRaw(SSATmp* base, RawMemSlot::Kind kind,
                               Type::Tag type) {
  ASSERT(type == Type::Int || type == Type::Bool || type == Type::FuncRef);
  return genInstruction(LdRaw, type, base, genDefConst(int64(kind)));
}

void TraceBuilder::genStRaw(SSATmp* base, RawMemSlot::Kind kind,
                            SSATmp* value) {
  ASSERT(value->getType() == Type::Int || value->getType() == Type::Bool);
  genInstruction(StRaw, Type::None, base, genDefConst(int64(kind)), value);
}

void TraceBuilder::genTraceEnd(uint32 nextPc,
                               TraceExitType::ExitType exitType /* = Normal */) {
  SSATmp* pc = genDefConst<int64>(nextPc);
  appendInstruction(m_irFactory.exitTrace(TraceExitType::Normal,
                                          m_curFunc,
                                          pc,
                                          m_spValue,
                                          m_fpValue));
}

Trace* TraceBuilder::genExitTrace(uint32   bcOff,
                                  int32    stackDeficit,
                                  uint32   numOpnds,
                                  SSATmp** opnds,
                                  TraceExitType::ExitType exitType,
                                  uint32   notTakenBcOff) {
  Trace* exitTrace = makeExitTrace(bcOff);
  exitTrace->appendInstruction(
    m_irFactory.marker(bcOff,
                     m_curFunc->getConstValAsFunc(),
                     m_spOffset + numOpnds - stackDeficit));
  SSATmp* sp = m_spValue;
  if (numOpnds != 0 || stackDeficit != 0) {
    SSATmp* stackAdjustment = genDefConst<int64>((int64)stackDeficit);
    IRInstruction * spillInst =
      m_irFactory.spillStack(m_spValue, stackAdjustment, numOpnds, opnds);
    sp = m_irFactory.getSSATmp(spillInst);
    exitTrace->appendInstruction(spillInst);
  }
  SSATmp* pc = genDefConst<int64>(bcOff);
  IRInstruction* instr = NULL;
  if (exitType == TraceExitType::NormalCc) {
    ASSERT(notTakenBcOff != 0);
    SSATmp* notTakenPC = genDefConst<int64>(notTakenBcOff);
    instr = m_irFactory.exitTrace(exitType,
                                  m_curFunc,
                                  pc,
                                  sp,
                                  m_fpValue,
                                  notTakenPC);
  } else {
    ASSERT(notTakenBcOff == 0);
    instr = m_irFactory.exitTrace(exitType,
                                  m_curFunc,
                                  pc,
                                  sp,
                                  m_fpValue);
  }
  exitTrace->appendInstruction(instr);
  return exitTrace;
}

SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     SSATmp* src1,
                                     SSATmp* src2,
                                     SSATmp* src3,
                                     SSATmp* src4) {
  SSATmp* srcs[2] = {src3, src4};
  ExtendedInstruction inst(m_irFactory, opc, type, src1, src2, 2, srcs);
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     SSATmp* src1,
                                     SSATmp* src2,
                                     SSATmp* src3,
                                     SSATmp* src4,
                                     SSATmp* src5) {
  SSATmp* srcs[3] = {src3, src4, src5};
  ExtendedInstruction inst(m_irFactory, opc, type, src1, src2, 3, srcs);
  return optimizeInst(&inst);
}
SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     SSATmp* src1,
                                     SSATmp* src2,
                                     SSATmp* src3,
                                     Trace* exit /* = NULL */) {
  ExtendedInstruction inst(m_irFactory, opc, type, src1, src2, src3,
                           0, NULL, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genInstruction(Opcode opc, Type::Tag type,
                                     SSATmp* src1,
                                     SSATmp* src2,
                                     uint32 nOpnds,
                                     SSATmp** opnds,
                                     Trace* exit /* = NULL*/) {
  ExtendedInstruction inst(m_irFactory, opc, type, src1, src2,
                           nOpnds, opnds, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     SSATmp* src1,
                                     SSATmp* src2,
                                     Trace* exit /* = NULL */) {
  IRInstruction inst(opc, type, src1, src2, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     SSATmp* src,
                                     Trace* exit /* = NULL */) {
  IRInstruction inst(opc, type, src, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genInstruction(Opcode opc,
                                     Type::Tag type,
                                     Trace* exit /* = NULL */) {
  IRInstruction inst(opc, type, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genIntegerOp(Opcode opc, SSATmp* src1, SSATmp* src2) {
  Type::Tag type1 = src1->getType();
  Type::Tag type2 = src2->getType();
  if ((type1 != Type::Int && type1 != Type::Bool) ||
      (type2 != Type::Int && type2 != Type::Bool)) {
    not_reached();
  }
  return genInstruction(opc, Type::Int, src1, src2);
}

SSATmp* TraceBuilder::genIntegerOp(Opcode opc, SSATmp* src) {
  if (src->getType() == Type::Int || src->getType() == Type::Bool) {
    return genInstruction(opc, Type::Int, src);
  }
  not_reached();
}

SSATmp* TraceBuilder::genAdd(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpAdd, src1, src2);
}
SSATmp* TraceBuilder::genSub(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpSub, src1, src2);
}
SSATmp* TraceBuilder::genAnd(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpAnd, src1, src2);
}
SSATmp* TraceBuilder::genOr(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpOr, src1, src2);
}
SSATmp* TraceBuilder::genXor(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpXor, src1, src2);
}
SSATmp* TraceBuilder::genMul(SSATmp* src1, SSATmp* src2) {
  return genIntegerOp(OpMul, src1, src2);
}
SSATmp* TraceBuilder::genNot(SSATmp* src) {
  // TODO: Move this to hhbctranslator
  return genConvToBool(genXor(genConvToBool(src), genDefConst<int64>(1)));
}

SSATmp* TraceBuilder::genLogicalOp(Opcode opc, SSATmp* src1, SSATmp* src2) {
  return genInstruction(opc,
                        Type::Bool,
                        genConvToBool(src1),
                        genConvToBool(src2));
}
SSATmp* TraceBuilder::genDefUninit() {
  ConstInstruction inst(DefConst, Type::Uninit);
  return optimizeInst(&inst);
}
SSATmp* TraceBuilder::genDefNull() {
  ConstInstruction inst(DefConst, Type::Null);
  return optimizeInst(&inst);
}
SSATmp* TraceBuilder::genConvToInt(SSATmp* src) {
  return genInstruction(Conv, Type::Int, src);
}
SSATmp* TraceBuilder::genConvToDbl(SSATmp* src) {
  return genInstruction(Conv, Type::Dbl, src);
}
SSATmp* TraceBuilder::genConvToStr(SSATmp* src) {
  if (src->getType() == Type::Bool) {
    // Bool to string code sequence loads static strings
    return genInstruction(Conv, Type::StaticStr, src);
  } else {
    return genInstruction(Conv, Type::Str, src);
  }
}
SSATmp* TraceBuilder::genConvToArr(SSATmp* src) {
  return genInstruction(Conv, Type::Arr, src);
}
SSATmp* TraceBuilder::genConvToObj(SSATmp* src) {
  return genInstruction(Conv, Type::Obj, src);
}
SSATmp* TraceBuilder::genConvToBool(SSATmp* src) {
  return genInstruction(Conv, Type::Bool, src);
}
SSATmp* TraceBuilder::genCmp(Opcode opc, SSATmp* src1, SSATmp* src2) {
  return genInstruction(opc, Type::Bool, src1, src2);
}
Trace* TraceBuilder::genJmpCond(Opcode opc,
                                SSATmp* src1,
                                SSATmp* src2,
                                Trace* target) {
  ASSERT(target);
  bool canResolve = false;
  bool cond = false; // make compiler happy
  // TODO move this to simplifier and use for comparison as well
  if (src1->getType() == Type::Null && src2->getType() == Type::Null) {
    switch (opc) {
      case JmpSame:
      case JmpEq:
        canResolve = true;
        cond = true;
        break;
      case JmpNSame:
      case JmpNeq:
        canResolve = true;
        cond = false;
        break;
      default:
        canResolve = false;
    }
  }
  if (src1->getType() == Type::Int && src2->getType() == Type::Int &&
      src1->isConst() && src2->isConst()) {
    canResolve = true;
    int64 val1 = src1->getConstValAsInt();
    int64 val2 = src2->getConstValAsInt();
    switch (opc) {
      case JmpGt:  cond = val1 >  val2; break;
      case JmpGte: cond = val1 >= val2; break;
      case JmpLt:  cond = val1 <  val2; break;
      case JmpLte: cond = val1 <= val2; break;
      case JmpSame:
      case JmpEq:  cond = val1 == val2; break;
      case JmpNSame:
      case JmpNeq: cond = val1 != val2; break;
      default:
        always_assert(0);
    }
  }
  if (canResolve) {
    // If cond is always true, generate an unconditional jump;
    // if cond is always false, don't generate anything.
    if (cond) {
      return genJmp(target);
    }
    return NULL;
  }

  // XXX TODO: simplifier
  genInstruction(opc, Type::None, src1, src2, target);
  return target;
}

Trace* TraceBuilder::genJmpCond(Opcode opc,
                                Type::Tag type,
                                SSATmp* src,
                                Trace* target) {
  ASSERT(target);
  genInstruction(opc, type, src, target);
  return target;
}


Trace* TraceBuilder::genJmpCond(Opcode opc,
                                SSATmp* src,
                                Trace* target) {
  ASSERT(target);
  genInstruction(opc, Type::None, src, target);
  return target;
}

Trace* TraceBuilder::genJmp(Trace* targetTrace) {
  ASSERT(targetTrace);
  genInstruction(Jmp_, Type::None, targetTrace);
  return targetTrace;
}

Trace* TraceBuilder::genJmpCond(SSATmp* boolSrc, Trace* target, bool negate) {
  ASSERT(target);
  ASSERT(boolSrc->getType() == Type::Bool);
  if (boolSrc->isConst()) {
    bool val = boolSrc->getConstValAsBool();
    if (negate) {
      val = !val;
    }
    if (val) {
      return genJmp(target); // taken
    }
    return NULL; // not taken
  }

  IRInstruction* srcInst = boolSrc->getInstruction();
  Opcode srcOpcode = srcInst->getOpcode();
  bool srcHasRefCountedOpnds = false;
  for (uint32 i = 0; i < srcInst->getNumSrcs(); i++) {
    if (isRefCounted(srcInst->getSrc(i))) {
      srcHasRefCountedOpnds = true;
      break;
    }
  }
  // try to combine the src inst with the Jmp. We can't combine the src
  // instruction with the jump if the src's are refcounted then we may dec
  // refs between the src instruction and the jump and then combining
  // would be illegal.
  if (!srcHasRefCountedOpnds) {
    if (isCmpOp(srcOpcode)) {
      if (negate) {
        srcOpcode = negateQueryOp(srcOpcode);
      }
      return genJmpCond(queryToJmpOp(srcOpcode),
                        srcInst->getSrc(0),
                        srcInst->getSrc(1),
                        target);
    } else if (isTypeQueryOp(srcOpcode)) {
      if (negate) {
        srcOpcode = negateQueryOp(srcOpcode);
      }
      TypeInstruction* isTypeInst = (TypeInstruction*)srcInst;
      Type::Tag type = isTypeInst->getSrcType();
      return genJmpCond(queryToJmpOp(srcOpcode),
                        type,
                        srcInst->getSrc(0),
                        target);
    } else if (isQueryOp(srcOpcode)) {
      if (negate) {
        srcOpcode = negateQueryOp(srcOpcode);
      }
      return genJmpCond(queryToJmpOp(srcOpcode),
                        srcInst->getSrc(0),
                        target);
    }
  }
  Opcode opc = negate ? JmpZero : JmpNZero;
  if (!srcHasRefCountedOpnds && isConvIntOrPtrToBool(srcInst)) {
    // test the int/ptr value directly
    genInstruction(opc, Type::None, srcInst->getSrc(0), target);
  } else {
    genInstruction(opc, Type::None, boolSrc, target);
  }
  return target;
}

Trace* TraceBuilder::genCheckUninit(SSATmp* src, Trace* target) {
  ASSERT(target);
  Type::Tag type = src->getType();
  // TODO: Add this to simplifier
  if (type == Type::Cell || type == Type::Gen) {
    genInstruction(CheckUninit, Type::None, src, target);
    return target;
  }
  return NULL;
}

Trace* TraceBuilder::genExitWhenSurprised(Trace* targetTrace) {
  ASSERT(targetTrace);
  genInstruction(ExitWhenSurprised, Type::None, targetTrace);
  return targetTrace;
}

Trace* TraceBuilder::genExitOnVarEnv(Trace* targetTrace) {
  ASSERT(targetTrace);
  genInstruction(ExitOnVarEnv, Type::None, m_fpValue, targetTrace);
  return targetTrace;
}

void TraceBuilder::genGuardLoc(uint32 id, Type::Tag type, Trace* exitTrace) {
  genInstruction(GuardLoc, type, genLdHome(id), exitTrace);
  setLocalType(id, type);
}

void TraceBuilder::genGuardStk(uint32 id, Type::Tag type, Trace* exitTrace) {
  genInstruction(GuardStk, type, m_spValue, genDefConst<int64>(id), exitTrace);
}


SSATmp* TraceBuilder::genGuardType(SSATmp* src,
                                   Type::Tag type,
                                   Trace* target) {
  ASSERT(target);
  Type::Tag srcType = src->getType();
  if (srcType == type || Type::isMoreRefined(srcType, type)) {
    /*
     * the type of the src is the same or more refined than type, so the
     * guard is unnecessary.
     */
    return src;
  }
  if (!Type::isMoreRefined(type, srcType)) {
    // incompatible types!
    // XXX TODO: generate a jump here and return NULL
    return genInstruction(GuardType, type, src, target);
  }
  // type is more refined that src's type, so we need a guard
  IRInstruction* srcInst = src->getInstruction();
  Opcode opc = srcInst->getOpcode();
  // if srcInst is an incref or move, then Chase down its src
  while (opc == Mov || opc == IncRef) {
    srcInst->setType(type);
    srcInst = srcInst->getSrc(0)->getInstruction();
    opc = srcInst->getOpcode();
    srcType = srcInst->getType();
  }
  if (srcInst->getLabel() &&
      (opc == LdLoc   || opc == LdStack  ||
       opc == LdMemNR || opc == LdPropNR ||
       opc == LdRefNR || opc == LdClsCns)) {
    if (srcType == Type::Gen ||
        (srcType == Type::Cell && !Type::isBoxed(type))) {
      srcInst->setType(type);
      return src;
    }
  }
  return genInstruction(GuardType, type, src, target);
}

void TraceBuilder::genGuardRefs(SSATmp* funcPtr,
                                SSATmp* nParams,
                                SSATmp* bitsPtr,
                                SSATmp* firstBitNum,
                                SSATmp* mask64,
                                SSATmp* vals64,
                                Trace*  exit) {

  appendInstruction(m_irFactory.guardRefs(funcPtr,
                                          nParams,
                                          bitsPtr,
                                          firstBitNum,
                                          mask64,
                                          vals64,
                                          exit->getLabel()));
}


SSATmp* TraceBuilder::genLdHome(uint32 id) {
  ConstInstruction inst(m_fpValue, Local(id));
  ASSERT(m_fpValue &&
         m_fpValue->getInstruction()->getOpcode() == DefFP);
  return optimizeInst(&inst);
}

// Helper to lookup class* by name, only using thread's cache
SSATmp* TraceBuilder::genLdCachedClass(SSATmp* className) {
  return genInstruction(LdCachedClass, Type::ClassRef, className);
}

SSATmp* TraceBuilder::genLdCls(SSATmp* className) {
  ASSERT(className->isConst() && className->getType() == Type::StaticStr);
  return genInstruction(LdCls, Type::ClassRef, className);
}

SSATmp* TraceBuilder::genLdClsCns(SSATmp* cnsName,
                                  SSATmp* cls,
                                  Trace* exitTrace) {
  ASSERT(cnsName->isConst() && cnsName->getType() == Type::StaticStr);
  // TODO: Weaken this second assert once we support ClassRef for cls
  ASSERT(cls->isConst() && cls->getType() == Type::StaticStr);
  return genInstruction(LdClsCns, Type::Cell, cnsName, cls, exitTrace);
}

SSATmp* TraceBuilder::genLdCurFuncPtr() {
  return genInstruction(LdCurFuncPtr, Type::FuncRef);
}

SSATmp* TraceBuilder::genLdARFuncPtr(SSATmp* baseAddr, SSATmp* offset) {
  return genInstruction(LdARFuncPtr, Type::FuncRef, baseAddr, offset);
}

SSATmp* TraceBuilder::genLdFuncCls(SSATmp* func) {
  return genInstruction(LdFuncCls, Type::ClassRef, func);
}


SSATmp* TraceBuilder::genLdPropAddr(SSATmp* obj, SSATmp* prop) {
  ASSERT(obj->getType() == Type::Obj);
  ASSERT(prop->getType() == Type::Int);
  ASSERT(prop->isConst());
  return genInstruction(LdPropAddr, Type::PtrToGen, obj, prop);
}

SSATmp* TraceBuilder::genLdClsPropAddr(SSATmp* cls,
                                       SSATmp* clsName,
                                       SSATmp* propName) {
  return genInstruction(LdClsPropAddr, Type::PtrToGen, cls, clsName, propName);
}

SSATmp* TraceBuilder::genLdFunc(SSATmp* funcName, SSATmp* actRec) {
  return genInstruction(LdFunc, Type::FuncRef, funcName, actRec);
}

SSATmp* TraceBuilder::genLdFixedFunc(const StringData* funcName,
                                     SSATmp* actRec) {
  return genInstruction(LdFixedFunc,
                        Type::FuncRef,
                        genDefConst<const StringData*>(funcName),
                        actRec);
}

SSATmp* TraceBuilder::genLdClsMethod(SSATmp* cls, uint32 methodSlot) {
  return genInstruction(LdClsMethod, Type::FuncRef, cls,
                        genDefConst<int64>(methodSlot));
}

SSATmp* TraceBuilder::genLdClsMethodCache(SSATmp* methodName,
                                          SSATmp* classRef) {
  return genInstruction(LdClsMethodCache, Type::FuncClassRef,
                        methodName, classRef);
}

SSATmp* TraceBuilder::genLdClsMethodCache(SSATmp* className,
                                          SSATmp* methodName,
                                          SSATmp* baseClass,
                                          Trace*  exit) {
  ExtendedInstruction inst(m_irFactory,
                           LdClsMethodCache,
                           Type::FuncClassRef,
                           className,
                           methodName,
                           baseClass, 0, NULL, getLabel(exit));
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genLdObjMethod(const StringData* methodName,
                                     SSATmp* actRec) {
  return genInstruction(LdObjMethod,
                        Type::FuncRef,
                        genDefConst<const StringData*>(methodName),
                        actRec);
}

SSATmp* TraceBuilder::genQueryOp(Opcode queryOpc, SSATmp* addr) {
  ASSERT(isQueryOp(queryOpc));
  return genInstruction(queryOpc, Type::Bool, addr);
}

SSATmp* TraceBuilder::genLdObjClass(SSATmp* obj) {
  ASSERT(obj->getType() == Type::Obj);
  return genInstruction(LdObjClass, Type::ClassRef, obj);
}

Trace* TraceBuilder::genVerifyParamType(SSATmp* objClass,
                                        SSATmp* className,
                                        const Class*  constraintClass,
                                        Trace*  exitTrace) {
  // do NOT use genLdCls() since don't want to load class if it isn't loaded
  SSATmp* constraint =
    constraintClass ? genDefConst<const Class*>(constraintClass)
                    : genLdCachedClass(className);

  return genJmpCond(JmpNSame, objClass, constraint, exitTrace);
}

SSATmp* TraceBuilder::genInstanceOfD(SSATmp* src, SSATmp* className) {
  return genInstruction(InstanceOfD, Type::Bool, src, className);
}

Local getLocalFromHomeOpnd(SSATmp* srcHome) {
  // Invariant: only LdHome instructions generate home values
  IRInstruction* ldHomeInstruction = srcHome->getInstruction();
  ASSERT(ldHomeInstruction->getOpcode() == LdHome);
  ASSERT(ldHomeInstruction->isConstInstruction());
  ASSERT(srcHome->getType() == Type::Home);
  return ((ConstInstruction*)ldHomeInstruction)->getLocal();
}

int getLocalIdFromHomeOpnd(SSATmp* srcHome) {
  IRInstruction* ldHomeInstruction = srcHome->getInstruction();
  if (ldHomeInstruction->getOpcode() == LdStack) {
    return -1;
  }
  return getLocalFromHomeOpnd(srcHome).getId();
}

SSATmp* TraceBuilder::genBoxLoc(uint32 id) {
  /*
   * prevValue = LdLoc home(id)
   * boxedValue = Box prevValue
   * StLoc = boxedValue
   * -- track local's value in boxedValue
   */
  SSATmp* prevValue = getLocalValue(id);
  SSATmp* home = genLdHome(id);
  if (prevValue == NULL) {
    // generate prevValue = ldloc
    Type::Tag type = getLocalType(id);
    // guards should ensure we have type info at this point
    ASSERT(type != Type::None);
    prevValue = genInstruction(LdLoc, type, home);
  }
  // Don't box if local's value already boxed
  Type::Tag prevType = prevValue->getType();
  if (Type::isBoxed(prevType)) {
    return prevValue;
  }
  ASSERT(Type::isUnboxed(prevType));
  // The Box helper requires us to incref the values its boxing, but in
  // this case we don't need to incref prevValue because we are simply
  // transfering its refcount from the local to the box.
  SSATmp* newValue = genInstruction(Box,
                                    Type::getBoxedType(prevType),
                                    prevValue);
  genStLocAux(id, newValue, true);
  return newValue;
}

void TraceBuilder::genRaiseUninitWarning(uint32 id) {
  IRInstruction inst(RaiseUninitWarning, Type::None, genLdHome(id));
  appendInstruction(inst.clone(&m_irFactory));
}

SSATmp* TraceBuilder::genLdLoc(uint32 id) {
  SSATmp* opnd = getLocalValue(id);
  if (opnd == NULL) {
    opnd = genInstruction(LdLoc, Type::Cell, genLdHome(id));
    setLocalValue(id, opnd);
  }
  return opnd;
}

SSATmp* TraceBuilder::genLdLoc(uint32 id,
                               Type::Tag type,
                               Trace* target) {
  SSATmp* t1 = getLocalValue(id);
  Type::Tag trackedType = getLocalType(id);
  if (t1 == NULL) {
    /*
     * no prior value for this local is available at this point
     * so generate a LdLoc instruction:
     */
    Type::Tag typeToUse = trackedType;
    Trace* ldLocTarget = NULL;
    if (trackedType == Type::None) {
      /* no prior type available either, so this is either a guard or
       * assert that's being generated. Make sure to use a exit target
       * to branch to in case the guard/assert fails.
       */
      typeToUse = type;
      ldLocTarget = target;
    }
    t1 = genInstruction(LdLoc, typeToUse, genLdHome(id), ldLocTarget);
    if (t1->getType() == Type::Null) {
      t1 = genDefNull();
    } else if (t1->getType() == Type::Uninit) {
      t1 = genDefUninit();
    }
    setLocalValue(id, t1);
  }
  if (Type::isBoxed(t1->getType())) {
    if (Type::isUnboxed(type)) {
      /*
       * unbox t1 into a cell via a LdRef
       */
      Type::Tag unboxedType = Type::getInnerType(t1->getType());
      ASSERT(!Type::isMoreRefined(type, unboxedType));
      return genLdRef(t1, unboxedType, target);
    }
    // boxed values can't be uninit, so skip the uninit check
    return t1;
  }
  return t1;
}

SSATmp* TraceBuilder::genLdLocAddr(uint32 id) {
  return genInstruction(LdLocAddr, Type::PtrToGen, genLdHome(id));
}

/*
 * Internal helper function that generates a store to a local.
 *
 * stloc [home] = newValue
 * -- track local in newValue
 */
void TraceBuilder::genStLocAux(uint32 id, SSATmp* newValue, bool storeType) {
  Opcode opc = storeType ? StLoc : StLocNT;
  genInstruction(opc, newValue->getType(), genLdHome(id), newValue);
  setLocalValue(id, newValue);
}

/*
 * Initializes a local to the provided state.
 */
void TraceBuilder::genInitLoc(uint32 id, SSATmp* t0) {
  genStLocAux(id, t0, true);
}

void TraceBuilder::genDecRefLoc(int id) {
  SSATmp* val = getLocalValue(id);
  if (val != NULL) {
    genDecRef(val);
    return;
  }
  Type::Tag type = getLocalType(id);

  // Don't generate code if type is not refcounted
  if (type != Type::None && !Type::isRefCounted(type)) {
    return;
  }

  if (Type::isBoxed(type)) {
    // we can't really rely on the types held in the boxed values since
    // aliasing stores may change them. We conservatively set the type
    // of the decref to a boxed cell.
    type = Type::BoxedCell;
  }

  genInstruction(DecRefLoc,
                 (type == Type::None ? Type::Gen : type),
                 genLdHome(id));
}

/*
 * Stores a ref (boxed value) to a local. Also handles unsetting a local.
 */
void TraceBuilder::genBindLoc(uint32 id,
                              SSATmp* newValue,
                              bool doRefCount /* = true */) {
  /*
   * prevValue = LdLoc [home]
   * StLoc [home] = newValue
   * DecRef prevValue
   * -- track local value in newValue
   */
  SSATmp* home = genLdHome(id);
  Type::Tag trackedType = getLocalType(id);
  SSATmp* prevValue;
  if (trackedType == Type::None) {
    if (doRefCount) {
      prevValue = genInstruction(LdLoc, Type::Gen, home);
    }
  } else {
    prevValue = getLocalValue(id);
    ASSERT(prevValue == NULL || prevValue->getType() == trackedType);
    if (prevValue == newValue) {
      // Silent store: home already contains value being stored
      // NewValue needs to be decref'ed
      if (Type::isRefCounted(trackedType) && doRefCount) {
        genDecRef(prevValue);
      }
      return;
    }
    if (Type::isRefCounted(trackedType) && !prevValue && doRefCount) {
      prevValue = genInstruction(LdLoc, trackedType, home);
    }
  }
  bool genStoreType = true;
  if ((Type::isBoxed(trackedType) && Type::isBoxed(newValue->getType())) ||
      (trackedType == newValue->getType() && !Type::isString(trackedType))) {
    // no need to store type with local value
    genStoreType = false;
  }
  genStLocAux(id, newValue, genStoreType);
  if (Type::isRefCounted(trackedType) && doRefCount) {
    genDecRef(prevValue);
  }
}

/*
 * Store a cell value to a local that might be boxed.
 */
SSATmp* TraceBuilder::genStLoc(uint32 id,
                               SSATmp* newValue,
                               bool doRefCount,
                               bool genStoreType,
                               Trace* exit) {
  ASSERT(!Type::isBoxed(newValue->getType()));
  /*
   * If prior value of local is a cell, then  re-use genBindLoc.
   * Otherwise, if prior value of local is a ref:
   *
   * prevLocValue = LdLoc [home]
   *    prevValue = LdRef [prevLocValue]
   *       newRef = StRef [prevLocValue], newValue
   * DecRef prevValue
   * -- track local value in newRef
   */
  Type::Tag trackedType = getLocalType(id);
  // These asserts make sure we should have info on the local's type at
  // this point thanks to the tracelet guards
  ASSERT (trackedType != Type::None);
  if (Type::isUnboxed(trackedType)) {
    SSATmp* retVal = doRefCount ? genIncRef(newValue) : newValue;
    genBindLoc(id, newValue, doRefCount);
    return retVal;
  }
  ASSERT(Type::isBoxed(trackedType));
  SSATmp* prevRef = getLocalValue(id);
  ASSERT(prevRef == NULL || prevRef->getType() == trackedType);

  // prevRef is a ref
  if (prevRef == NULL) {
    // prevRef = ldLoc
    prevRef = genInstruction(LdLoc, trackedType, genLdHome(id));
  }
  SSATmp* prevValue = NULL;
  if (doRefCount) {
    ASSERT(exit);
    Type::Tag innerType = Type::getInnerType(trackedType);
    prevValue = genInstruction(LdRefNR, innerType, prevRef, exit);
  }
  // stref [prevRef] = t1
  Opcode opc = genStoreType ? StRef : StRefNT;
  SSATmp* newRef = genInstruction(opc,
                                  Type::getBoxedType(newValue->getType()),
                                  prevRef,
                                  newValue);
  // update tracked value for local 'id' -- or create one if not tracked yet
  setLocalValue(id, newRef);
  // update other tracked locals that also contain prevRef
  updateLocalRefValues(prevRef, newRef);

  SSATmp* retVal = newValue;
  if (doRefCount) {
    retVal = genIncRef(newValue);
    genDecRef(prevValue);
  }
  return retVal;
}

SSATmp* TraceBuilder::genNewObj(int32 numParams, SSATmp* cls) {
  SSATmp* newSpValue = genInstruction(NewObj,
                                      Type::SP,
                                      genDefConst<int64>(numParams),
                                      cls,
                                      m_spValue,
                                      m_fpValue);
  m_spValue = newSpValue;
  // new obj leaves the new object and an actrec on the stack
  m_spOffset += (sizeof(ActRec) / sizeof(Cell)) + 1;
  ASSERT(m_spOffset >= 0);
  return newSpValue;
}

SSATmp* TraceBuilder::genNewObj(int32 numParams,
                                const StringData* className) {
  SSATmp* newSpValue = genInstruction(NewObj,
                                      Type::SP,
                                      genDefConst<int64>(numParams),
                                      genDefConst<const StringData*>(className),
                                      m_spValue,
                                      m_fpValue);
  m_spValue = newSpValue;
  // new obj leaves the new object and an actrec on the stack
  m_spOffset += (sizeof(ActRec) / sizeof(Cell)) + 1;
  ASSERT(m_spOffset >= 0);
  return newSpValue;
}

SSATmp* TraceBuilder::genNewArray(int32 capacity) {
  return genInstruction(NewArray, Type::Arr, genDefConst<int64>(capacity));
}

SSATmp* TraceBuilder::genNewTuple(int32 numArgs, SSATmp* sp) {
  ASSERT(numArgs >= 0);
  return genInstruction(NewTuple, Type::Arr, genDefConst<int64>(numArgs), sp);
}

SSATmp* TraceBuilder::genAllocActRec(SSATmp* func,
                                     SSATmp* objOrCls,
                                     int32 numParams,
                                     const StringData* magicName) {
  // This value will be decref'ed by the callee when it pops the act rec
  if (!objOrCls) {
    objOrCls = genDefNull();
  }
  SSATmp* magicNameTmp = magicName ? genDefConst<const StringData*>(magicName)
                                   : genDefNull();
  SSATmp* newSpValue = getSSATmp(
    m_irFactory.allocActRec(m_spValue,
                            m_fpValue,
                            func,
                            objOrCls,
                            genDefConst<int64>(numParams),
                            magicNameTmp));
  m_spValue = newSpValue;
  m_spOffset += (sizeof(ActRec) / sizeof(Cell));
  ASSERT(m_spOffset >= 0);
  return newSpValue;
}

SSATmp* TraceBuilder::genAllocActRec(const Func* func,
                                     SSATmp* objOrCls,
                                     int32 numParams,
                                     const StringData* magicName) {
  SSATmp* funcTmp = func ? genDefConst<const Func*>(func) : genDefNull();
  return genAllocActRec(funcTmp, objOrCls, numParams, magicName);
}

SSATmp* TraceBuilder::genAllocActRec(SSATmp* func,
                                     SSATmp* obj,
                                     int32 numParams) {
  return genAllocActRec(func, obj, numParams, NULL/*magicName*/);
}

SSATmp* TraceBuilder::genAllocActRec(SSATmp* func, int32 numParams) {
  return genAllocActRec(func, NULL/*objOrCls*/, numParams, NULL/*magicName*/);
}

SSATmp* TraceBuilder::genFreeActRec() {
  m_fpValue = getSSATmp(m_irFactory.freeActRec(m_fpValue));
  return m_fpValue;
}

/*
 * Track down a value that was previously spilled onto the stack
 * The spansCall parameter tracks whether the returned value's
 * lifetime on the stack spans a call. This search bottoms out
 * on hitting either a DefSP instruction (failure), a SpillStack
 * instruction that has the spilled location, or a call that returns
 * the value.
 */
SSATmp* getStackValue(SSATmp* sp,
                      uint32 index,
                      bool& spansCall,
                      Type::Tag& type) {
  IRInstruction* inst = sp->getInstruction();
  Opcode opc = inst->getOpcode();
  if (opc == DefSP) {
    return NULL;
  }
  if (opc == Call) {
    // sp = call(actrec, bcoffset, func, args...)
    if (index == 0) {
      // return value from call
      return NULL;
    }
    spansCall = true;
    // search recursively on the actrec argument
    return getStackValue(inst->getSrc(0), // sp = actrec argument to call
                         index-1, // call pushes a value; -1 pops it
                         spansCall,
                         type);
  }
  if (opc == AllocActRec) {
    // sp = allocActRec(stackptr, frameptr, func, ...)
    // search recursively on the stackptr argument
    return getStackValue(inst->getSrc(0),
                         index,
                         spansCall,
                         type);
  }
  if (opc == SpillStack || opc == SpillStackAllocAR) {
    // sp = spillstack(stkptr, stkAdjustment, spilledtmp0, spilledtmp1, ...)
    int64 numPushed = inst->getNumSrcs() - 2;
    if (numPushed > index) {
      SSATmp* tmp = inst->getSrc(index + 2);
      if (tmp->getInstruction()->getOpcode() == IncRef) {
        tmp = tmp->getInstruction()->getSrc(0);
      }
      type = tmp->getType();
      return tmp;
    } else {
      // this is not one of the values pushed onto the stack by this
      // spillstack instruction, so continue searching
      SSATmp* prevSp = inst->getSrc(0);
      int64 numPopped = inst->getSrc(1)->getConstValAsInt();
      return getStackValue(prevSp,
                           // pop values pushed by spillstack:
                           index - (numPushed - numPopped),
                           spansCall,
                           type);
    }
  }
  if (opc == InterpOne) {
    // sp = InterpOne(fp, sp, bcOff, stackAdjustment, resultType)
    SSATmp* prevSp = inst->getSrc(1);
    int64 numPopped = inst->getSrc(3)->getConstValAsInt();
    Type::Tag resultType = (Type::Tag)inst->getSrc(4)->getConstValAsInt();
    int64 numPushed = resultType == Type::None ? 0 : 1;
    if (index == 0 && numPushed == 1) {
      type = resultType;
      return NULL;
    }
    return getStackValue(prevSp, index - (numPushed - numPopped),
                         spansCall, type);
  }
  if (opc == NewObj) {
    // sp = NewObj(numParams, className, sp, fp)
    if (index == 0) {
      // newly allocated object, which we unfortunately don't have any
      // kind of handle to:-(
      type = Type::Obj;
      return NULL;
    } else {
      return getStackValue(sp->getInstruction()->getSrc(2),
                           index-1, // newObj pushes the new obj; -1 pops it
                           spansCall,
                           type);
    }
  }
  // Should not get here!
  ASSERT(0);
  return NULL;
}

SSATmp* TraceBuilder::genDefFP() {
  return genInstruction(DefFP, Type::SP);
}

SSATmp* TraceBuilder::genDefSP() {
  return genInstruction(DefSP, Type::SP);
}

SSATmp* TraceBuilder::genLdStackAddr(int64 index) {
  return genInstruction(LdStackAddr,
                        Type::PtrToGen,
                        m_spValue,
                        genDefConst(index));
}

void TraceBuilder::genNativeImpl() {
  genInstruction(NativeImpl, Type::None, m_curFunc, m_fpValue);
}

SSATmp* TraceBuilder::genInterpOne(uint32 pcOff,
                                   uint32 stackAdjustment,
                                   Type::Tag resultType,
                                   Trace* target) {
  SSATmp* opnds[] = {
    genDefConst<int64>(pcOff),
    genDefConst<int64>(stackAdjustment),
    genDefConst<int64>((int64)resultType) };

  SSATmp* spVal = genInstruction(InterpOne, Type::SP,
                                 m_fpValue, m_spValue,
                                 3, opnds, target);
  m_spValue = spVal;
  // push the return value if any and adjust for the popped values
  m_spOffset += ((resultType == Type::None ? 0 : 1) - stackAdjustment);
  return spVal;
}

SSATmp* TraceBuilder::genCall(SSATmp* actRec,
                              uint32 returnBcOffset,
                              SSATmp* func,
                              uint32 numParams,
                              SSATmp** params) {
  // The call pushes 'numParams' arguments onto the stack
  SSATmp* newSpValue = getSSATmp(
    m_irFactory.call(actRec,
                     genDefConst<int64>(returnBcOffset),
                     func,
                     numParams,
                     params));
  m_spValue = newSpValue;
  // after the call: pop the ActRec and the params, and then push
  // the result value
  m_spOffset -= (sizeof(ActRec) / sizeof(Cell)); // pop actrec
  m_spOffset += 1;// push result value
  ASSERT(m_spOffset >= 0);
  // kill all available expressions; we can't keep them in regs anyway
  killCse();
  killLocals();
  return newSpValue;
}

SSATmp* TraceBuilder::genRetVal(SSATmp* val) {
  m_spValue = getSSATmp(m_irFactory.retVal(m_fpValue, val));
  return m_spValue;
}

SSATmp* TraceBuilder::genRetVal() {
  m_spValue = getSSATmp(m_irFactory.retVal(m_fpValue));
  return m_spValue;
}

IRInstruction* TraceBuilder::genRetCtrl(SSATmp* sp,
                                        SSATmp* fp,
                                        SSATmp* retAddr) {
  return appendInstruction(m_irFactory.retCtrl(sp, fp, retAddr));
}

IRInstruction* TraceBuilder::genMarker(uint32 bcOff, int32 spOff) {
  return appendInstruction(m_irFactory.marker(bcOff,
                                              m_curFunc->getConstValAsFunc(),
                                              spOff));
}

void TraceBuilder::genDecRefStack(Type::Tag type,
                                  uint32 stackOff,
                                  Trace* exit) {
  bool spansCall = false;
  Type::Tag knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (tmp == NULL || (spansCall && !tmp->getInstruction()->isDefConst())) {
    // We don't want to extend live ranges of tmps across calls, so
    // we don't get the value if spansCall is true; however, we
    // use any type information known
    if (knownType != Type::None && knownType != Type::Gen) {
      type = knownType;
    }
    SSATmp* index = genDefConst<int64>(stackOff);
    appendInstruction(m_irFactory.decRefStack(type,
                                              m_spValue,
                                              index,
                                              exit ? exit->getLabel() : NULL));
  } else {
    genDecRef(tmp);
  }
}

void TraceBuilder::genDecRefThis() {
  if (isThisAvailable()) {
    SSATmp* thiss = genLdThis(NULL);
    genDecRef(thiss);
  } else {
    appendInstruction(m_irFactory.decRefThis(m_fpValue, NULL));
  }
}

void TraceBuilder::genDecRefLocalsThis(uint32 numLocals) {
    SSATmp* numLocalsTmp = genDefConst<int64>(numLocals);
    appendInstruction(m_irFactory.decRefLocalsThis(m_fpValue, numLocalsTmp));
}

void TraceBuilder::genDecRefLocals(uint32 numLocals) {
    SSATmp* numLocalsTmp = genDefConst<int64>(numLocals);
    appendInstruction(m_irFactory.decRefLocals(m_fpValue, numLocalsTmp));
}

void TraceBuilder::genIncStat(int32 counter, int32 value) {
  genIncStat(genLdConst<int64>(counter),
             genLdConst<int64>(value));
}

SSATmp* TraceBuilder::genIncRef(SSATmp* src) {
  if (!isRefCounted(src)) {
    return src;
  }
  return genInstruction(IncRef, src->getType(), src);
}

SSATmp* TraceBuilder::genSpillStack(uint32 stackAdjustment,
                                    uint32 numOpnds,
                                    SSATmp** opnds,
                                    bool allocActRec) {
  if (stackAdjustment == 0 && numOpnds == 0 && !allocActRec) {
    return m_spValue;
  }

  SSATmp* stackAdjustmentOpnd = genDefConst<int64>(stackAdjustment);
  SSATmp* newSpValue = getSSATmp(m_irFactory.spillStack(m_spValue,
                                                        stackAdjustmentOpnd,
                                                        numOpnds,
                                                        opnds,
                                                        allocActRec));
  m_spValue = newSpValue;
  // push the spilled values but adjust for the popped values
  m_spOffset += (numOpnds - stackAdjustment);
  if (allocActRec) {
    m_spOffset += (sizeof(ActRec) / sizeof(Cell));
  }
  ASSERT(m_spOffset >= 0);
  return newSpValue;
}

/*
 * If target == NULL, then type is the expected type on the stack not
 * a type to guard against.
 */
SSATmp* TraceBuilder::genLdStack(int32 stackOff,
                                 Type::Tag type,
                                 Trace* target) {
  bool spansCall = false;
  Type::Tag knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (tmp == NULL || (spansCall && !tmp->getInstruction()->isDefConst())) {
    // We don't want to extend live ranges of tmps across calls, so
    // we don't get the value if spansCall is true; however, we
    // use any type information known
    if (knownType != Type::None && knownType != Type::Gen) {
      type = knownType;
    }
    return genInstruction(LdStack,
                          type,
                          m_spValue,
                          genDefConst<int64>(stackOff),
                          target);
  }
  return tmp;
}

SSATmp* TraceBuilder::genCreateCont(bool getArgs,
                                    const Func* origFunc,
                                    const Func* genFunc) {
  return genInstruction(CreateCont, Type::Obj,
                        m_fpValue,
                        genDefConst(getArgs),
                        genDefConst(origFunc),
                        genDefConst(genFunc));
}

void TraceBuilder::genFillContLocals(const Func* origFunc,
                                     const Func* genFunc,
                                     SSATmp* cont) {
  genInstruction(FillContLocals, Type::None,
                 m_fpValue,
                 genDefConst(origFunc),
                 genDefConst(genFunc),
                 cont);
}

void TraceBuilder::genFillContThis(SSATmp* cont, SSATmp* locals, int64 offset) {
  genInstruction(FillContThis, Type::None,
                 cont, locals, genDefConst(offset));
}

void TraceBuilder::genUnlinkContVarEnv() {
  genInstruction(UnlinkContVarEnv, Type::None, m_fpValue, (Trace*)NULL);
}

void TraceBuilder::genLinkContVarEnv() {
  genInstruction(LinkContVarEnv, Type::None, m_fpValue, (Trace*)NULL);
}

Trace* TraceBuilder::genContRaiseCheck(SSATmp* cont, Trace* target) {
  ASSERT(target);
  genInstruction(ContRaiseCheck, Type::None, cont, target);
  return target;
}

Trace* TraceBuilder::genContPreNext(SSATmp* cont, Trace* target) {
  ASSERT(target);
  genInstruction(ContPreNext, Type::None, cont, target);
  return target;
}

Trace* TraceBuilder::genContStartedCheck(SSATmp* cont, Trace* target) {
  ASSERT(target);
  genInstruction(ContStartedCheck, Type::None, cont, target);
  return target;
}

void TraceBuilder::genIncStat(SSATmp* counter, SSATmp* value) {
  genInstruction(IncStat, Type::None, counter, value);
}

SSATmp* TraceBuilder::getSSATmp(IRInstruction* inst) {
  SSATmp* opnd = m_irFactory.getSSATmp(inst);
  if (!inst->isDefConst()) {
    appendInstruction(inst);
  }
  return opnd;
}

IRInstruction* TraceBuilder::appendInstruction(IRInstruction* inst) {
  m_trace->appendInstruction(inst);
  return inst;
}

CSEHash* TraceBuilder::getCSEHashTable(IRInstruction* inst) {
  if (inst->isDefConst()) {
    return &m_constTable;
  }
  return &m_cseHash;
}

SSATmp* TraceBuilder::cseInsert(IRInstruction* inst, bool clone) {
  if (clone) {
    inst = inst->clone(&m_irFactory);
  }
  SSATmp* tmp = getSSATmp(inst);
  getCSEHashTable(inst)->insert(tmp);
  return tmp;
}

SSATmp* TraceBuilder::cseLookup(IRInstruction* inst) {
  return getCSEHashTable(inst)->lookup(inst);
}

/*
 * Should be able to run optimizeInst on everything and
 * have it generate the SSATmp result and appropriately
 * apply common subexpresson elimination according to the
 * tables generated from the IR_OPCODES macro.
 */
SSATmp* TraceBuilder::optimizeInst(IRInstruction* inst) {
  SSATmp* result = NULL;
  if (inst->canCSE()) {
    result = cseLookup(inst);
    if (result) {
      // Found a dominating instruction that can be used instead of inst
      return result;
    }
  }
  // copy propagation on inst source operands
  Simplifier::copyProp(inst);
  // simplification
  // Note: simplifier's return value must be in the cse hash table
  // but if there is no simplification the result will be NULL
  result = inst->simplify(&m_simplifier);
  if (result) {
    ASSERT(result->getInstruction()->hasDst());
    return result;
  }
  if (inst->canCSE()) {
    result = cseInsert(inst, true);
  } else {
    inst = inst->clone(&m_irFactory);
    if (inst->hasDst()) {
      result = getSSATmp(inst);
    } else {
      // we will return NULL in this case because instruction has no dest.
      appendInstruction(inst);
    }
  }
  return result;
}

SSATmp* TraceBuilder::cse(IRInstruction* inst) {
  SSATmp* tmp = cseLookup(inst);
  if (tmp) {
    return tmp;
  }
  cseInsert(inst, false);
  return tmp;
}

void TraceBuilder::killCse() {
  m_cseHash.clear();
}

SSATmp* TraceBuilder::getLocalValue(int id) {
  if (id == -1 || id >= (int)m_localValues.size()) {
    return NULL;
  }
  SSATmp* val = m_localValues[id];
  if (!val) {
    Type::Tag type = getLocalType(id);
    // TODO
    if (type == Type::Null) {
    }
    if (type == Type::Uninit) {
    }
  }
  return val;
}

Type::Tag TraceBuilder::getLocalType(int id) {
  if (id == -1 || id >= (int)m_localValues.size()) {
    return Type::None;
  }
  return m_localTypes[id];
}

void TraceBuilder::setLocalValue(int id, SSATmp* value) {
  if (id == -1) {
    return;
  }
  if (id >= (int)m_localValues.size()) {
    m_localValues.resize(id + 1);
    m_localTypes.resize(id + 1, Type::None);
  }
  m_localValues[id] = value;
  m_localTypes[id] = value->getType();
}

void TraceBuilder::setLocalType(int id, Type::Tag type) {
  if (id == -1) {
    return;
  }
  if (id >= (int)m_localValues.size()) {
    m_localValues.resize(id + 1);
    m_localTypes.resize(id + 1, Type::None);
  }
  m_localValues[id] = NULL;
  m_localTypes[id] = type;
}

// Needs to be called if a local escapes as a by-ref
void TraceBuilder::killLocalValue(int id) {
  if (id == -1 || id >= (int)m_localValues.size()) {
    return;
  }
  m_localValues[id] = NULL;
  m_localTypes[id] = Type::None;
}

bool TraceBuilder::anyLocalHasValue(SSATmp* tmp) const {
  for (size_t id = 0; id < m_localValues.size(); id++) {
    if (m_localValues[id] == tmp) {
      return true;
    }
  }
  return false;
}

//
// This method updates the tracked values and types of all locals that contain
// oldRef so that they now contain newRef.
// This should only be called for ref/boxed types.
//
void TraceBuilder::updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef) {
  ASSERT(Type::isBoxed(oldRef->getType()));
  ASSERT(Type::isBoxed(newRef->getType()));

  Type::Tag newRefType = newRef->getType();
  size_t nTrackedLocs = m_localValues.size();
  for (size_t id = 0; id < nTrackedLocs; id++) {
    if (m_localValues[id] == oldRef) {
      m_localValues[id] = newRef;
      m_localTypes[id]  = newRefType;
    }
  }
}

/**
 * Called to clear out the tracked local values at a call site.
 * Calls kill all registers, so we don't want to keep locals in
 * registers across calls. We do continue tracking the types in
 * locals, however.
 */
void TraceBuilder::killLocals() {
  for (uint32 i = 0; i < m_localValues.size(); i++) {
    SSATmp* t = m_localValues[i];
    // should not kill DefConst, and LdConst should be replaced by DefConst
    if (t == NULL || t->getInstruction()->isDefConst()) {
      continue;
    }
    if (t->getInstruction()->getOpcode() == LdConst) {
      // make the new DefConst instruction
      IRInstruction* clone = t->getInstruction()->clone(&m_irFactory);
      clone->setOpcode(DefConst);
      m_localValues[i] = clone->getDst();
      continue;
    }
    ASSERT(!t->isConst());
    m_localValues[i] = NULL;
  }
}

TraceBuilder::TraceBuilder(uint32 initialBcOffset,
                           IRFactory& irFactory_,
                           CSEHash& constants,
                           const Func* func) :
    m_irFactory(irFactory_),
    m_constTable(constants),
    m_simplifier(this),
    m_spValue(NULL),
    m_fpValue(NULL),
    m_spOffset(0),
    m_exitGuardFailureTrace(NULL),
    m_thisIsAvailable(false),
    m_initialBcOff(-1),
    m_trace(makeTrace(initialBcOffset, true))
{
  // put a function marker at the start of trace
  m_curFunc = genDefConst<const Func*>(func);
}

void TraceBuilder::finalizeTrace() {
}

}}} // namespace HPHP::VM::JIT
