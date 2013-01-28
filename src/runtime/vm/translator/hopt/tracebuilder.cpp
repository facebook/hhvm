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

#include "runtime/vm/translator/hopt/tracebuilder.h"

#include "folly/ScopeGuard.h"

#include "util/trace.h"
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

TraceBuilder::TraceBuilder(Offset initialBcOffset,
                           uint32_t initialSpOffsetFromFp,
                           IRFactory& irFactory,
                           CSEHash& constants,
                           const Func* func)
  : m_irFactory(irFactory)
  , m_simplifier(this)
  , m_initialBcOff(initialBcOffset)
  , m_trace(makeTrace(func, initialBcOffset, true))
  , m_enableCse(false)
  , m_enableSimplification(false)
  , m_spValue(NULL)
  , m_fpValue(NULL)
  , m_spOffset(initialSpOffsetFromFp)
  , m_constTable(constants)
  , m_thisIsAvailable(false)
{
  // put a function marker at the start of trace
  m_curFunc = genDefConst<const Func*>(func);
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_enableCse = RuntimeOption::EvalHHIRCse;
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }
  genDefFP();
  genDefSP(initialSpOffsetFromFp);
  assert(m_spOffset >= 0);
}

void TraceBuilder::genPrint(SSATmp* arg) {
  gen(Print, arg);
}

SSATmp* TraceBuilder::genAddElem(SSATmp* arr, SSATmp* key, SSATmp* val) {
  return gen(AddElem, arr, key, val);
}

SSATmp* TraceBuilder::genAddNewElem(SSATmp* arr, SSATmp* val) {
  return gen(AddNewElem, arr, val);
}

SSATmp* TraceBuilder::genDefCns(const StringData* cnsName, SSATmp* val) {
  return gen(DefCns, genDefConst<const StringData*>(cnsName), val);
}

SSATmp* TraceBuilder::genConcat(SSATmp* tl, SSATmp* tr) {
  return gen(Concat, tl, tr);
}

SSATmp* TraceBuilder::genArrayAdd(SSATmp* tl, SSATmp* tr) {
  return gen(ArrayAdd, tl, tr);
}

void TraceBuilder::genDefCls(PreClass* clss, const HPHP::VM::Opcode* after) {
  PUNT(DefCls);
}

void TraceBuilder::genDefFunc(Func* func) {
  gen(DefFunc, genDefConst<const Func*>(func));
}

SSATmp* TraceBuilder::genLdThis(Trace* exitTrace) {
  if (m_thisIsAvailable) {
    return gen(LdThis, m_fpValue);
  } else {
    return gen(LdThis, getLabel(exitTrace), m_fpValue);
  }
}

SSATmp* TraceBuilder::genLdProp(SSATmp* obj,
                                SSATmp* prop,
                                Type::Tag type,
                                Trace* exit) {
  assert(obj->getType() == Type::Obj);
  assert(prop->getType() == Type::Int);
  assert(prop->isConst());
  return gen(LdProp, type, getLabel(exit), obj, prop);
}

void TraceBuilder::genStProp(SSATmp* obj,
                             SSATmp* prop,
                             SSATmp* src,
                             bool genStoreType) {
  Opcode opc = genStoreType ? StProp : StPropNT;
  gen(opc, obj, prop, src);
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
  Opcode opc = genStoreType ? StMem : StMemNT;
  gen(opc, addr, genDefConst(offset), src);
}

void TraceBuilder::genSetPropCell(SSATmp* base, int64 offset, SSATmp* value) {
  SSATmp* oldVal = genLdProp(base, genDefConst(offset), Type::Cell, NULL);
  genStProp(base, genDefConst(offset), value, true);
  genDecRef(oldVal);
}

SSATmp* TraceBuilder::genDefMIStateBase() {
  return gen(DefMIStateBase);
}

SSATmp* TraceBuilder::genPropX(TCA func, Class* ctx,
                               SSATmp* base, SSATmp* key, SSATmp* mis) {
  return gen(PropX, genDefConst(func), genDefConst(ctx), base, key, mis);
}

SSATmp* TraceBuilder::genCGetProp(TCA func, Class* ctx, SSATmp* base,
                                  SSATmp* key, SSATmp* mis) {
  return gen(CGetProp, genDefConst(func), genDefConst(ctx), base, key, mis);
}

SSATmp* TraceBuilder::genCGetElem(TCA func, SSATmp* base, SSATmp* key,
                                  SSATmp* mis) {
  return gen(CGetElem, genDefConst(func), base, key, mis);
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               int64_t offset,
                               Type::Tag type,
                               Trace* target) {
  assert(addr->getType() == Type::PtrToCell ||
         addr->getType() == Type::PtrToGen);
  return gen(LdMem, type, getLabel(target), addr,
             genDefConst<int64>(offset));
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               Type::Tag type,
                               Trace* target) {
  return genLdMem(addr, 0, type, target);
}

SSATmp* TraceBuilder::genLdRef(SSATmp* ref, Type::Tag type, Trace* exit) {
  assert(Type::isUnboxed(type));
  assert(Type::isBoxed(ref->getType()));
  return gen(LdRef, type, getLabel(exit), ref);
}

SSATmp* TraceBuilder::genUnboxPtr(SSATmp* ptr) {
  return gen(UnboxPtr, ptr);
}

SSATmp* TraceBuilder::genUnbox(SSATmp* src, Trace* exit) {
  return gen(Unbox, getLabel(exit), src);
}

/**
 * Checks if the given SSATmp, or any of its aliases, is available in
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
      gen(DecRefNZ, tmp);
      return;
    }
  }

  gen(DecRef, tmp);
}

void TraceBuilder::genDecRefMem(SSATmp* base, int64 offset, Type::Tag type) {
  gen(DecRefMem, type, base, genDefConst<int64>(offset));
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

Trace* TraceBuilder::genExitGuardFailure(uint32 bcOff) {
  Trace* trace = makeExitTrace(bcOff);
  IRInstruction* markerInst =
    m_irFactory.marker(bcOff, m_curFunc->getConstValAsFunc(), m_spOffset);
  trace->appendInstruction(markerInst);
  SSATmp* pc = genDefConst<int64>((int64)bcOff);
  // TODO change exit trace to a control flow instruction that
  // takes sp, fp, and a Marker as the target label instruction
  trace->appendInstruction(
    m_irFactory.gen(getExitOpcode(TraceExitType::GuardFailure),
                    m_curFunc,
                    pc,
                    m_spValue,
                    m_fpValue));
  return trace;
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

SSATmp* TraceBuilder::genLdRetAddr() {
  return gen(LdRetAddr, m_fpValue);
}

SSATmp* TraceBuilder::genLdRaw(SSATmp* base, RawMemSlot::Kind kind,
                               Type::Tag type) {
  return gen(LdRaw, type, base, genDefConst(int64(kind)));
}

void TraceBuilder::genStRaw(SSATmp* base, RawMemSlot::Kind kind,
                            SSATmp* value, int64 extraOff) {
  assert(value->getType() == Type::Int || value->getType() == Type::Bool);
  gen(StRaw, base, genDefConst(int64(kind)), value,
      genDefConst<int64>(extraOff));
}

void TraceBuilder::genTraceEnd(uint32 nextPc,
                               TraceExitType::ExitType exitType /* = Normal */) {
  gen(getExitOpcode(TraceExitType::Normal),
      m_curFunc,
      genDefConst<int64>(nextPc),
      m_spValue,
      m_fpValue);
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
    SSATmp* srcs[numOpnds + 2];
    srcs[0] = m_spValue;
    srcs[1] = genDefConst<int64>(stackDeficit);
    std::copy(opnds, opnds + numOpnds, srcs + 2);

    auto* spillInst = m_irFactory.gen(SpillStack, numOpnds + 2, srcs);
    sp = spillInst->getDst();
    exitTrace->appendInstruction(spillInst);
  }
  SSATmp* pc = genDefConst<int64>(bcOff);
  IRInstruction* instr = NULL;
  if (exitType == TraceExitType::NormalCc) {
    assert(notTakenBcOff != 0);
    SSATmp* notTakenPC = genDefConst(notTakenBcOff);
    instr = m_irFactory.gen(getExitOpcode(exitType),
                            m_curFunc,
                            pc,
                            sp,
                            m_fpValue,
                            notTakenPC);
  } else {
    assert(notTakenBcOff == 0);
    instr = m_irFactory.gen(getExitOpcode(exitType),
                            m_curFunc,
                            pc,
                            sp,
                            m_fpValue);
  }
  exitTrace->appendInstruction(instr);
  return exitTrace;
}

SSATmp* TraceBuilder::genAdd(SSATmp* src1, SSATmp* src2) {
  return gen(OpAdd, src1, src2);
}
SSATmp* TraceBuilder::genSub(SSATmp* src1, SSATmp* src2) {
  return gen(OpSub, src1, src2);
}
SSATmp* TraceBuilder::genAnd(SSATmp* src1, SSATmp* src2) {
  return gen(OpAnd, src1, src2);
}
SSATmp* TraceBuilder::genOr(SSATmp* src1, SSATmp* src2) {
  return gen(OpOr, src1, src2);
}
SSATmp* TraceBuilder::genXor(SSATmp* src1, SSATmp* src2) {
  return gen(OpXor, src1, src2);
}
SSATmp* TraceBuilder::genMul(SSATmp* src1, SSATmp* src2) {
  return gen(OpMul, src1, src2);
}

SSATmp* TraceBuilder::genNot(SSATmp* src) {
  assert(src->getType() == Type::Bool);
  return genConvToBool(genXor(src, genDefConst<int64>(1)));
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
  return gen(Conv, Type::Int, src);
}

SSATmp* TraceBuilder::genConvToDbl(SSATmp* src) {
  return gen(Conv, Type::Dbl, src);
}

SSATmp* TraceBuilder::genConvToStr(SSATmp* src) {
  if (src->getType() == Type::Bool) {
    // Bool to string code sequence loads static strings
    return gen(Conv, Type::StaticStr, src);
  } else {
    return gen(Conv, Type::Str, src);
  }
}

SSATmp* TraceBuilder::genConvToArr(SSATmp* src) {
  return gen(Conv, Type::Arr, src);
}

SSATmp* TraceBuilder::genConvToObj(SSATmp* src) {
  return gen(Conv, Type::Obj, src);
}

SSATmp* TraceBuilder::genConvToBool(SSATmp* src) {
  return gen(Conv, Type::Bool, src);
}

SSATmp* TraceBuilder::genCmp(Opcode opc, SSATmp* src1, SSATmp* src2) {
  return gen(opc, src1, src2);
}

SSATmp* TraceBuilder::genJmp(Trace* targetTrace) {
  assert(targetTrace);
  return gen(Jmp_, getLabel(targetTrace));
}

SSATmp* TraceBuilder::genJmpCond(SSATmp* boolSrc, Trace* target, bool negate) {
  assert(target);
  assert(boolSrc->getType() == Type::Bool);
  return gen(negate ? JmpZero : JmpNZero, getLabel(target), boolSrc);
}

void TraceBuilder::genExitWhenSurprised(Trace* targetTrace) {
  gen(ExitWhenSurprised, getLabel(targetTrace));
}

void TraceBuilder::genExitOnVarEnv(Trace* targetTrace) {
  gen(ExitOnVarEnv, getLabel(targetTrace), m_fpValue);
}

void TraceBuilder::genReleaseVVOrExit(Trace* exit) {
  gen(ReleaseVVOrExit, getLabel(exit), m_fpValue);
}

void TraceBuilder::genGuardLoc(uint32 id, Type::Tag type, Trace* exitTrace) {
  SSATmp* prevValue = getLocalValue(id);
  if (prevValue) {
    genGuardType(prevValue, type, exitTrace);
    return;
  }
  Type::Tag prevType = getLocalType(id);
  if (prevType == Type::None) {
    gen(GuardLoc, type, getLabel(exitTrace), genLdHome(id));
  } else {
    // It doesn't make sense to be guarding on something that's deemed to fail
    assert(prevType == type);
  }
}

void TraceBuilder::genAssertLoc(uint32 id, Type::Tag type) {
  Type::Tag prevType = getLocalType(id);
  if (prevType == Type::None || Type::isMoreRefined(type, prevType)) {
    gen(AssertLoc, type, genLdHome(id));
  } else {
    assert(prevType == type || Type::isMoreRefined(prevType, type));
  }
}

SSATmp* TraceBuilder::genLdAssertedLoc(uint32 id, Type::Tag type) {
  genAssertLoc(id, type);
  return genLdLoc(id);
}

void TraceBuilder::genGuardStk(uint32 id, Type::Tag type, Trace* exitTrace) {
  gen(GuardStk, type, getLabel(exitTrace), m_spValue, genDefConst<int64>(id));
}

SSATmp* TraceBuilder::genGuardType(SSATmp* src,
                                   Type::Tag type,
                                   Trace* target) {
  assert(target);
  return gen(GuardType, type, getLabel(target), src);
}

void TraceBuilder::genGuardRefs(SSATmp* funcPtr,
                                SSATmp* nParams,
                                SSATmp* bitsPtr,
                                SSATmp* firstBitNum,
                                SSATmp* mask64,
                                SSATmp* vals64,
                                Trace*  exit) {
  gen(GuardRefs,
      getLabel(exit),
      funcPtr,
      nParams,
      bitsPtr,
      firstBitNum,
      mask64,
      vals64);
}

SSATmp* TraceBuilder::genLdHome(uint32 id) {
  SSATmp* srcs[] = { m_fpValue };
  ConstInstruction inst(1, srcs, Local(id));
  assert(m_fpValue &&
         m_fpValue->getInstruction()->getOpcode() == DefFP);
  return optimizeInst(&inst);
}

SSATmp* TraceBuilder::genLdCls(SSATmp* className) {
  return gen(LdCls, className);
}

SSATmp* TraceBuilder::genLdClsCns(SSATmp* cnsName, SSATmp* cls, Trace* exit) {
  return gen(LdClsCns, Type::Cell, getLabel(exit), cnsName, cls);
}

void TraceBuilder::genCheckInit(SSATmp* src, Trace* target) {
  assert(target);
  gen(CheckInit, getLabel(target), src);
}

SSATmp* TraceBuilder::genLdCurFuncPtr() {
  return gen(LdCurFuncPtr);
}

SSATmp* TraceBuilder::genLdARFuncPtr(SSATmp* baseAddr, SSATmp* offset) {
  return gen(LdARFuncPtr, baseAddr, offset);
}

SSATmp* TraceBuilder::genLdFuncCls(SSATmp* func) {
  return gen(LdFuncCls, func);
}

SSATmp* TraceBuilder::genLdPropAddr(SSATmp* obj, SSATmp* prop) {
  return gen(LdPropAddr, obj, prop);
}

SSATmp* TraceBuilder::genLdClsPropAddr(SSATmp* cls,
                                       SSATmp* clsName,
                                       SSATmp* propName) {
  return gen(LdClsPropAddr, cls, clsName, propName);
}

SSATmp* TraceBuilder::genLdClsMethod(SSATmp* cls, uint32 methodSlot) {
  return gen(LdClsMethod, cls, genDefConst<int64>(methodSlot));
}

SSATmp* TraceBuilder::genLdClsMethodCache(SSATmp* className,
                                          SSATmp* methodName,
                                          SSATmp* baseClass,
                                          Trace*  exit) {
  return gen(LdClsMethodCache, getLabel(exit), className, methodName,
             baseClass);
}

SSATmp* TraceBuilder::genLdObjMethod(const StringData* methodName,
                                     SSATmp* actRec) {
  return gen(LdObjMethod, genDefConst<const StringData*>(methodName),
             actRec);
}

SSATmp* TraceBuilder::genQueryOp(Opcode queryOpc, SSATmp* addr) {
  assert(isQueryOp(queryOpc));
  return gen(queryOpc, addr);
}

// TODO(#2058871): move this to hhbctranslator
void TraceBuilder::genVerifyParamType(SSATmp* objClass,
                                        SSATmp* className,
                                        const Class*  constraintClass,
                                        Trace*  exitTrace) {
  // do NOT use genLdCls() since don't want to load class if it isn't loaded
  SSATmp* constraint =
    constraintClass ? genDefConst<const Class*>(constraintClass)
                    : gen(LdCachedClass, className);
  gen(JmpNSame, getLabel(exitTrace), objClass, constraint);
}

int getLocalIdFromHomeOpnd(SSATmp* srcHome) {
  IRInstruction* ldHomeInstruction = srcHome->getInstruction();
  assert(ldHomeInstruction->getOpcode() == LdHome);
  assert(ldHomeInstruction->isConstInstruction());
  assert(srcHome->getType() == Type::Home);
  return ((ConstInstruction*)ldHomeInstruction)->getLocal().getId();
}

SSATmp* TraceBuilder::genBoxLoc(uint32 id) {
  /*
   * prevValue = LdLoc home(id)
   * boxedValue = Box prevValue
   * StLoc = boxedValue
   * -- track local's value in boxedValue
   */
  SSATmp* prevValue  = genLdLoc(id);
  Type::Tag prevType = prevValue->getType();
  // Don't box if local's value already boxed
  if (Type::isBoxed(prevType)) {
    return prevValue;
  }
  assert(Type::isUnboxed(prevType));
  // The Box helper requires us to incref the values its boxing, but in
  // this case we don't need to incref prevValue because we are simply
  // transfering its refcount from the local to the box.
  SSATmp* newValue = gen(Box, prevValue);
  genStLocAux(id, newValue, true);
  return newValue;
}

void TraceBuilder::genRaiseUninitWarning(uint32 id) {
  gen(RaiseUninitWarning, genLdHome(id));
}

SSATmp* TraceBuilder::genLdAddr(SSATmp* base, int64 offset) {
  return gen(LdAddr, base, genDefConst<int64>(offset));
}

/**
 * Returns an SSATmp containing the current value of the given local.
 * This generates a LdLoc instruction if needed.
 *
 * Note: the type of the local must be known already (due to type guards
 *       or assertions).
 */
SSATmp* TraceBuilder::genLdLoc(uint32 id) {
  SSATmp* tmp = getLocalValue(id);
  if (tmp) {
    return tmp;
  }
  // No prior value for this local is available, so actually generate a LdLoc.
  Type::Tag type = getLocalType(id);
  assert(type != Type::None); // tracelet guards guarantee we have a type
  switch (type) {
    case Type::Uninit : tmp = genDefUninit(); break;
    case Type::Null   : tmp = genDefNull();   break;
    default           : tmp = gen(LdLoc, type, genLdHome(id));
  }
  return tmp;
}

SSATmp* TraceBuilder::genLdLocAsCell(uint32 id, Trace* exitTrace) {
  SSATmp*    tmp = genLdLoc(id);
  Type::Tag type = tmp->getType();
  if (!Type::isBoxed(type)) {
    return tmp;
  }
  // Unbox tmp into a cell via a LdRef
  return genLdRef(tmp, Type::getInnerType(type), exitTrace);
}

SSATmp* TraceBuilder::genLdLocAddr(uint32 id) {
  return gen(LdLocAddr, genLdHome(id));
}

/*
 * Internal helper function that generates a store to a local.
 *
 * stloc [home] = newValue
 * -- track local in newValue
 */
void TraceBuilder::genStLocAux(uint32 id, SSATmp* newValue, bool storeType) {
  Opcode opc = storeType ? StLoc : StLocNT;
  gen(opc, genLdHome(id), newValue);
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

  gen(DecRefLoc, (type == Type::None ? Type::Gen : type), genLdHome(id));
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
      prevValue = gen(LdLoc, Type::Gen, home);
    }
  } else {
    prevValue = getLocalValue(id);
    assert(prevValue == NULL || prevValue->getType() == trackedType);
    if (prevValue == newValue) {
      // Silent store: home already contains value being stored
      // NewValue needs to be decref'ed
      if (Type::isRefCounted(trackedType) && doRefCount) {
        genDecRef(prevValue);
      }
      return;
    }
    if (Type::isRefCounted(trackedType) && !prevValue && doRefCount) {
      prevValue = gen(LdLoc, trackedType, home);
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
  assert(!Type::isBoxed(newValue->getType()));
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
  assert(trackedType != Type::None);  // tracelet guards guarantee a type
  if (Type::isUnboxed(trackedType)) {
    SSATmp* retVal = doRefCount ? genIncRef(newValue) : newValue;
    genBindLoc(id, newValue, doRefCount);
    return retVal;
  }
  assert(Type::isBoxed(trackedType));
  SSATmp* prevRef = getLocalValue(id);
  assert(prevRef == NULL || prevRef->getType() == trackedType);
  // prevRef is a ref
  if (prevRef == NULL) {
    // prevRef = ldLoc
    prevRef = gen(LdLoc, trackedType, genLdHome(id));
  }
  SSATmp* prevValue = NULL;
  if (doRefCount) {
    assert(exit);
    Type::Tag innerType = Type::getInnerType(trackedType);
    prevValue = gen(LdRef, innerType, getLabel(exit), prevRef);
  }
  // stref [prevRef] = t1
  Opcode opc = genStoreType ? StRef : StRefNT;
  gen(opc, prevRef, newValue);

  SSATmp* retVal = newValue;
  if (doRefCount) {
    retVal = genIncRef(newValue);
    genDecRef(prevValue);
  }
  return retVal;
}

SSATmp* TraceBuilder::genNewObj(int32 numParams, SSATmp* cls) {
  return gen(NewObj, genDefConst<int64>(numParams), cls, m_spValue, m_fpValue);
}

SSATmp* TraceBuilder::genNewObj(int32 numParams,
                                const StringData* className) {
  return gen(NewObj,
             genDefConst<int64>(numParams),
             genDefConst<const StringData*>(className),
             m_spValue,
             m_fpValue);
}

SSATmp* TraceBuilder::genNewArray(int32 capacity) {
  return gen(NewArray, genDefConst<int64>(capacity));
}

SSATmp* TraceBuilder::genNewTuple(int32 numArgs, SSATmp* sp) {
  assert(numArgs >= 0);
  return gen(NewTuple, genDefConst<int64>(numArgs), sp);
}

SSATmp* TraceBuilder::genDefActRec(SSATmp* func,
                                   SSATmp* objOrClass,
                                   int32_t numArgs,
                                   const StringData* invName) {
  return gen(DefActRec,
             m_fpValue,
             func,
             objOrClass,
             genDefConst<int64>(numArgs),
             invName ?
               genDefConst<const StringData*>(invName) : genDefNull());
}

SSATmp* TraceBuilder::genFreeActRec() {
  return gen(FreeActRec, m_fpValue);
}

/*
 * Track down a value that was previously spilled onto the stack
 * The spansCall parameter tracks whether the returned value's
 * lifetime on the stack spans a call. This search bottoms out
 * on hitting either a DefSP instruction (failure), a SpillStack
 * instruction that has the spilled location, or a call that returns
 * the value.
 */
static SSATmp* getStackValue(SSATmp* sp,
                             uint32 index,
                             bool& spansCall,
                             Type::Tag& type) {
  IRInstruction* inst = sp->getInstruction();
  switch (inst->getOpcode()) {
  case DefSP:
    return nullptr;

  case AssertStk:
    // fallthrough
  case GuardStk: {
    // sp = GuardStk<T> sp, offset
    // We don't have a value, but we may know the type due to guarding
    // on it.
    if (inst->getSrc(1)->getConstValAsInt() == index) {
      type = inst->getTypeParam();
      return nullptr;
    }
    return getStackValue(inst->getSrc(0),
                         index,
                         spansCall,
                         type);
  }

  case Call:
    // sp = call(actrec, bcoffset, func, args...)
    if (index == 0) {
      // return value from call
      return nullptr;
    }
    spansCall = true;
    // search recursively on the actrec argument
    return getStackValue(inst->getSrc(0), // sp = actrec argument to call
                         index -
                           (1 /* pushed */ - kNumActRecCells /* popped */),
                         spansCall,
                         type);

  case SpillStack: {
    // sp = spillstack(stkptr, stkAdjustment, spilledtmp0, spilledtmp1, ...)
    int64_t numPushed    = 0;
    int32_t numSpillSrcs = inst->getNumSrcs() - 2;

    for (int i = 0; i < numSpillSrcs; ++i) {
      SSATmp* tmp = inst->getSrc(i + 2);
      if (tmp->getType() == Type::ActRec) {
        numPushed += kNumActRecCells;
        i += kSpillStackActRecExtraArgs;
        continue;
      }

      if (index == numPushed) {
        if (tmp->getInstruction()->getOpcode() == IncRef) {
          tmp = tmp->getInstruction()->getSrc(0);
        }
        type = tmp->getType();
        return tmp;
      }
      ++numPushed;
    }

    // this is not one of the values pushed onto the stack by this
    // spillstack instruction, so continue searching
    SSATmp* prevSp = inst->getSrc(0);
    int64_t numPopped = inst->getSrc(1)->getConstValAsInt();
    return getStackValue(prevSp,
                         // pop values pushed by spillstack
                         index - (numPushed - numPopped),
                         spansCall,
                         type);
  }

  case InterpOne: {
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

  case NewObj:
    // sp = NewObj(numParams, className, sp, fp)
    if (index == kNumActRecCells) {
      // newly allocated object, which we unfortunately don't have any
      // kind of handle to :-(
      type = Type::Obj;
      return NULL;
    } else {
      return getStackValue(sp->getInstruction()->getSrc(2),
                           // NewObj pushes an object and an ActRec
                           index - (1 + kNumActRecCells),
                           spansCall,
                           type);
    }

  default:
    break;
  }

  // Should not get here!
  assert(0);
  return NULL;
}

void TraceBuilder::genAssertStk(uint32_t id, Type::Tag type) {
  Type::Tag knownType = Type::None;
  bool spansCall = false;
  UNUSED SSATmp* tmp = getStackValue(m_spValue, id, spansCall, knownType);
  assert(!tmp);
  if (knownType == Type::None || Type::isMoreRefined(type, knownType)) {
    gen(AssertStk, type, m_spValue, genDefConst<int64>(id));
  }
}

SSATmp* TraceBuilder::genDefFP() {
  return gen(DefFP);
}

SSATmp* TraceBuilder::genDefSP(int32 spOffset) {
  return gen(DefSP, m_fpValue, genDefConst(spOffset));
}

SSATmp* TraceBuilder::genLdStackAddr(int64 index) {
  return gen(LdStackAddr, m_spValue, genDefConst(index));
}

void TraceBuilder::genNativeImpl() {
  gen(NativeImpl, m_curFunc, m_fpValue);
}

SSATmp* TraceBuilder::genInterpOne(uint32 pcOff,
                                   uint32 stackAdjustment,
                                   Type::Tag resultType,
                                   Trace* target) {
  return gen(InterpOne,
             getLabel(target),
             m_fpValue,
             m_spValue,
             genDefConst<int64>(pcOff),
             genDefConst<int64>(stackAdjustment),
             genDefConst<int64>((int64)resultType));
}

SSATmp* TraceBuilder::genCall(SSATmp* actRec,
                              uint32 returnBcOffset,
                              SSATmp* func,
                              uint32 numParams,
                              SSATmp** params) {
  SSATmp* srcs[numParams + 3];
  srcs[0] = actRec;
  srcs[1] = genDefConst<int64>(returnBcOffset);
  srcs[2] = func;
  std::copy(params, params + numParams, srcs + 3);
  return gen(Call, numParams + 3, srcs);
}

void TraceBuilder::genRetVal(SSATmp* val) {
  gen(RetVal, m_fpValue, val);
}

SSATmp* TraceBuilder::genRetAdjustStack() {
  return gen(RetAdjustStack, m_fpValue);
}

void TraceBuilder::genRetCtrl(SSATmp* sp, SSATmp* fp, SSATmp* retVal) {
  gen(RetCtrl, sp, fp, retVal);
}

IRInstruction* TraceBuilder::genMarker(uint32 bcOff, int32 spOff) {
  auto* marker = m_irFactory.marker(bcOff,
                                    m_curFunc->getConstValAsFunc(),
                                    spOff);
  appendInstruction(marker);
  return marker;
}

void TraceBuilder::genDecRefStack(Type::Tag type, uint32 stackOff) {
  bool spansCall = false;
  Type::Tag knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (tmp == NULL || (spansCall && !tmp->getInstruction()->isDefConst())) {
    // We don't want to extend live ranges of tmps across calls, so we
    // don't get the value if spansCall is true; however, we can use
    // any type information known.
    if (knownType != Type::None) {
      type = Type::getMostRefined(type, knownType);
    }
    gen(DecRefStack, type, m_spValue, genDefConst<int64>(stackOff));
  } else {
    genDecRef(tmp);
  }
}

void TraceBuilder::genDecRefThis() {
  if (isThisAvailable()) {
    SSATmp* thiss = genLdThis(NULL);
    genDecRef(thiss);
  } else {
    gen(DecRefThis, m_fpValue);
  }
}

SSATmp* TraceBuilder::genGenericRetDecRefs(SSATmp* retVal, int numLocals) {
  return gen(GenericRetDecRefs, m_fpValue, retVal,
             genDefConst<int64>(numLocals));
}

void TraceBuilder::genIncStat(int32 counter, int32 value, bool force) {
  gen(IncStat,
      genDefConst<int64>(counter),
      genDefConst<int64>(value),
      genDefConst<bool>(force));
}

SSATmp* TraceBuilder::genIncRef(SSATmp* src) {
  return gen(IncRef, src);
}

SSATmp* TraceBuilder::genSpillStack(uint32 stackAdjustment,
                                    uint32 numOpnds,
                                    SSATmp** spillOpnds) {
  if (stackAdjustment == 0 && numOpnds == 0) {
    return m_spValue;
  }

  SSATmp* srcs[numOpnds + 2];
  srcs[0] = m_spValue;
  srcs[1] = genDefConst<int64>(stackAdjustment);
  std::copy(spillOpnds, spillOpnds + numOpnds, srcs + 2);
  return gen(SpillStack, numOpnds + 2, srcs);
}

SSATmp* TraceBuilder::genLdStack(int32 stackOff, Type::Tag type) {
  bool spansCall = false;
  Type::Tag knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (tmp == NULL || (spansCall && !tmp->getInstruction()->isDefConst())) {
    // We don't want to extend live ranges of tmps across calls, so we
    // don't get the value if spansCall is true; however, we can use
    // any type information known.
    if (knownType != Type::None) {
      type = Type::getMostRefined(type, knownType);
    }
    return gen(LdStack,
               type,
               m_spValue,
               genDefConst<int64>(stackOff));
  }
  return tmp;
}

SSATmp* TraceBuilder::genCreateCont(bool getArgs,
                                    const Func* origFunc,
                                    const Func* genFunc) {
  return gen(CreateCont,
             m_fpValue,
             genDefConst(getArgs),
             genDefConst(origFunc),
             genDefConst(genFunc));
}

void TraceBuilder::genFillContLocals(const Func* origFunc,
                                     const Func* genFunc,
                                     SSATmp* cont) {
  gen(FillContLocals,
      m_fpValue,
      genDefConst(origFunc),
      genDefConst(genFunc),
      cont);
}

void TraceBuilder::genFillContThis(SSATmp* cont, SSATmp* locals, int64 offset) {
  gen(FillContThis, cont, locals, genDefConst(offset));
}

void TraceBuilder::genContEnter(SSATmp* contAR, SSATmp* addr,
                                int64 returnBcOffset) {
  gen(ContEnter, contAR, addr, genDefConst(returnBcOffset));
}

void TraceBuilder::genUnlinkContVarEnv() {
  gen(UnlinkContVarEnv, m_fpValue);
}

void TraceBuilder::genLinkContVarEnv() {
  gen(LinkContVarEnv, m_fpValue);
}

Trace* TraceBuilder::genContRaiseCheck(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContRaiseCheck, getLabel(target), cont);
  return target;
}

Trace* TraceBuilder::genContPreNext(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContPreNext, getLabel(target), cont);
  return target;
}

Trace* TraceBuilder::genContStartedCheck(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContStartedCheck, getLabel(target), cont);
  return target;
}

SSATmp* TraceBuilder::genIterNext(uint32 iterId, uint32 localId) {
  // IterNext fpReg, iterId, localId
  return gen(IterNext,
             Type::Bool,
             m_fpValue,
             genDefConst<int64>(iterId),
             genDefConst<int64>(localId));
}

SSATmp* TraceBuilder::genIterNextK(uint32 iterId,
                                   uint32 valLocalId,
                                   uint32 keyLocalId) {
  // IterNextK fpReg, iterId, valLocalId, keyLocalId
  return gen(IterNextK,
             Type::Bool,
             m_fpValue,
             genDefConst<int64>(iterId),
             genDefConst<int64>(valLocalId),
             genDefConst<int64>(keyLocalId));
}

SSATmp* TraceBuilder::genIterInit(SSATmp* src,
                                  uint32 iterId,
                                  uint32 valLocalId) {
  // IterInit src, fpReg, iterId, valLocalId
  return gen(IterInit,
             Type::Bool,
             src,
             m_fpValue,
             genDefConst<int64>(iterId),
             genDefConst<int64>(valLocalId));
}

SSATmp* TraceBuilder::genIterInitK(SSATmp* src,
                                   uint32 iterId,
                                   uint32 valLocalId,
                                   uint32 keyLocalId) {
  // IterInitK src, fpReg, iterId, valLocalId, keyLocalId
  return gen(IterInitK,
             Type::Bool,
             src,
             m_fpValue,
             genDefConst<int64>(iterId),
             genDefConst<int64>(valLocalId),
             genDefConst<int64>(keyLocalId));
}

void TraceBuilder::updateTrackedState(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  // Update tracked state of local values/types, stack/frame pointer, CSE, etc.
  switch (opc) {
    case Call: {
      m_spValue = inst->getDst();
      // A call pops the ActRec and pushes a return value.
      m_spOffset -= kNumActRecCells;
      m_spOffset += 1;
      assert(m_spOffset >= 0);
      killCse();
      killLocals();
      break;
    }
    case ContEnter: {
      killCse();
      killLocals();
      break;
    }
    case DefFP: {
      m_fpValue = inst->getDst();
      break;
    }
    case DefSP: {
      m_spValue = inst->getDst();
      m_spOffset = inst->getSrc(1)->getConstValAsInt();
      break;
    }
    case AssertStk:
    case GuardStk: {
      m_spValue = inst->getDst();
      break;
    }
    case SpillStack: {
      m_spValue = inst->getDst();
      // Push the spilled values but adjust for the popped values
      int64 stackAdjustment = inst->getSrc(1)->getConstValAsInt();
      m_spOffset -= stackAdjustment;
      m_spOffset += spillValueCells(inst);
      assert(m_spOffset >= 0);
      break;
    }
    case NewObj: {
      m_spValue = inst->getDst();
      // new obj leaves the new object and an actrec on the stack
      m_spOffset += (sizeof(ActRec) / sizeof(Cell)) + 1;
      assert(m_spOffset >= 0);
      break;
    }
    case InterpOne: {
      m_spValue = inst->getDst();
      int64 stackAdjustment = inst->getSrc(3)->getConstValAsInt();
      Type::Tag resultType = (Type::Tag)inst->getSrc(4)->getConstValAsInt();
      // push the return value if any and adjust for the popped values
      m_spOffset += ((resultType == Type::None ? 0 : 1) - stackAdjustment);
      break;
    }

    case StRefNT:
    case StRef: {
      SSATmp* newRef = inst->getDst();
      SSATmp* prevRef = inst->getSrc(0);
      // update other tracked locals that also contain prevRef
      updateLocalRefValues(prevRef, newRef);
      break;
    }

    case StLocNT:
    case StLoc: {
      uint32 id = getLocalIdFromHomeOpnd(inst->getSrc(0));
      setLocalValue(id, inst->getSrc(1));
      break;
    }
    case LdLoc: {
      uint32 id = getLocalIdFromHomeOpnd(inst->getSrc(0));
      setLocalValue(id, inst->getDst());
      break;
    }
    case AssertLoc:
    case GuardLoc: {
      uint32 id = getLocalIdFromHomeOpnd(inst->getSrc(0));
      setLocalType(id, inst->getTypeParam());
      break;
    }
    case IterInitK:
    case IterNextK: {
      // kill the local to which this instruction stores iter's key
      killLocalValue(inst->getSrc(3)->getConstValAsInt());
      // fall through to case below to handle value local
    }
    case IterInit:
    case IterNext: {
      // kill the local to which this instruction stores iter's value
      killLocalValue(inst->getSrc(2)->getConstValAsInt());
      break;
    }
    case LdThis: {
      m_thisIsAvailable = true;
      break;
    }
    default:
      break;
  }
  // update the CSE table
  if (m_enableCse && inst->canCSE()) {
    cseInsert(inst);
  }
}

void TraceBuilder::clearTrackedState() {
  killCse(); // clears m_cseHash
  for (uint32 i = 0; i < m_localValues.size(); i++) {
    m_localValues[i] = NULL;
  }
  for (uint32 i = 0; i < m_localTypes.size(); i++) {
    m_localTypes[i] = Type::None;
  }
  m_spValue = m_fpValue = NULL;
  m_spOffset = 0;
  m_thisIsAvailable = false;
}

void TraceBuilder::appendInstruction(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  if (opc != Nop && !inst->isDefConst()) {
    m_trace->appendInstruction(inst);
  }
  updateTrackedState(inst);
}

CSEHash* TraceBuilder::getCSEHashTable(IRInstruction* inst) {
  if (inst->isDefConst()) {
    return &m_constTable;
  }
  return &m_cseHash;
}

void TraceBuilder::cseInsert(IRInstruction* inst) {
  getCSEHashTable(inst)->insert(inst->getDst());
}

SSATmp* TraceBuilder::cseLookup(IRInstruction* inst) {
  return getCSEHashTable(inst)->lookup(inst);
}

SSATmp* TraceBuilder::optimizeInst(IRInstruction* inst,
                                   CloneInstMode clone /* =kCloneInst */) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "{}{}\n", indent(), inst->toString());

  // copy propagation on inst source operands
  Simplifier::copyProp(inst);

  SSATmp* result = NULL;
  if (m_enableCse && inst->canCSE()) {
    result = cseLookup(inst);
    if (result) {
      // Found a dominating instruction that can be used instead of inst
      FTRACE(1, "  {}cse found: {}\n",
             indent(), result->getInstruction()->toString());
      return result;
    }
  }

  if (m_enableSimplification) {
    result = m_simplifier.simplify(inst);
    if (result) {
      // Found a simpler instruction that can be used instead of inst
      FTRACE(1, "  {}simplification returned: {}\n",
             indent(), result->getInstruction()->toString());
      assert(result->getInstruction()->hasDst());
      return result;
    }
    // simplifier could change an instruction into a Nop
    // don't bother processing Nops any further
    if (inst->getOpcode() == Nop) return nullptr;
  }
  // Couldn't CSE or simplify the instruction; insert the instruction or
  // its clone into the instruction list
  if (clone == kCloneInst)  inst = inst->clone(&m_irFactory);

  appendInstruction(inst);
  // returns NULL if instruction has no dest.
  return inst->getDst();
}

void TraceBuilder::optimizeTrace() {
  m_enableCse = RuntimeOption::EvalHHIRCse;
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_enableCse && !m_enableSimplification) return;
  auto instructions = std::move(m_trace->getInstructionList());
  assert(m_trace->getInstructionList().empty());

  clearTrackedState();
  for (auto inst : instructions) {
    SSATmp* result = optimizeInst(inst, kUseInst);
    SSATmp* dst = inst->getDst();
    if (dst && dst->getType() != Type::None && dst != result) {
      // The result of optimization has a different destination register
      // than the inst. Generate a move to get result into dst.
      assert(result);
      appendInstruction(m_irFactory.mov(dst, result));
    }
  }
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

// Needs to be called if a local escapes as a by-ref or
// otherwise set to an unknown value (e.g., by Iter[Init,Next][K])
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
  assert(Type::isBoxed(oldRef->getType()));
  assert(Type::isBoxed(newRef->getType()));

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
    assert(!t->isConst());
    m_localValues[i] = NULL;
  }
}

}}} // namespace HPHP::VM::JIT
