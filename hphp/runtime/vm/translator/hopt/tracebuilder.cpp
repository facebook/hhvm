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
#include "runtime/vm/translator/targetcache.h"
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

static inline Type noneToGen(Type t) {
  return t.subtypeOf(Type::None) ? Type::Gen : t;
}

TraceBuilder::TraceBuilder(Offset initialBcOffset,
                           uint32_t initialSpOffsetFromFp,
                           IRFactory& irFactory,
                           const Func* func)
  : m_irFactory(irFactory)
  , m_simplifier(this)
  , m_initialBcOff(initialBcOffset)
  , m_trace(makeTrace(func, initialBcOffset))
  , m_enableCse(false)
  , m_enableSimplification(false)
  , m_snapshots(&irFactory, nullptr)
  , m_spValue(nullptr)
  , m_fpValue(nullptr)
  , m_spOffset(initialSpOffsetFromFp)
  , m_thisIsAvailable(false)
  , m_refCountedMemValue(nullptr)
  , m_localValues(func->numLocals(), nullptr)
  , m_localTypes(func->numLocals(), Type::None)
{
  FTRACE(2, "TraceBuilder: initial sp offset {}\n", initialSpOffsetFromFp);

  // put a function marker at the start of trace
  m_curFunc = genDefConst<const Func*>(func);
  if (RuntimeOption::EvalHHIRGenOpts) {
    m_enableCse = RuntimeOption::EvalHHIRCse;
    m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  }

  gen(DefFP);
  gen(DefSP, StackOffset(initialSpOffsetFromFp), m_fpValue);

  assert(m_spOffset >= 0);
}

TraceBuilder::~TraceBuilder() {
  for (State* state : m_snapshots) delete state;
}

SSATmp* TraceBuilder::genDefCns(const StringData* cnsName, SSATmp* val) {
  return gen(DefCns, genDefConst<const StringData*>(cnsName), val);
}

SSATmp* TraceBuilder::genConcat(SSATmp* tl, SSATmp* tr) {
  return gen(Concat, tl, tr);
}

SSATmp* TraceBuilder::genLdThis(Trace* exitTrace) {
  if (isThisAvailable()) {
    return gen(LdThis, m_fpValue);
  } else {
    return gen(LdThis, getFirstBlock(exitTrace), m_fpValue);
  }
}

SSATmp* TraceBuilder::genLdCtx(const Func* func) {
  if (isThisAvailable()) {
    return genLdThis(nullptr);
  }
  return gen(LdCtx, m_fpValue, genDefConst(func));
}

SSATmp* TraceBuilder::genLdProp(SSATmp* obj,
                                SSATmp* prop,
                                Type type,
                                Trace* exit) {
  assert(obj->getType() == Type::Obj);
  assert(prop->getType() == Type::Int);
  assert(prop->isConst());
  return gen(LdProp, type, getFirstBlock(exit), obj, prop);
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
                            int64_t offset,
                            SSATmp* src,
                            bool genStoreType) {
  Opcode opc = genStoreType ? StMem : StMemNT;
  gen(opc, addr, genDefConst(offset), src);
}

void TraceBuilder::genSetPropCell(SSATmp* base, int64_t offset, SSATmp* value) {
  SSATmp* oldVal = genLdProp(base, genDefConst(offset), Type::Cell, nullptr);
  genStProp(base, genDefConst(offset), value, true);
  genDecRef(oldVal);
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               int64_t offset,
                               Type type,
                               Trace* target) {
  return gen(LdMem, type, getFirstBlock(target), addr,
             genDefConst(offset));
}

SSATmp* TraceBuilder::genLdMem(SSATmp* addr,
                               Type type,
                               Trace* target) {
  return genLdMem(addr, 0, type, target);
}

SSATmp* TraceBuilder::genLdRef(SSATmp* ref, Type type, Trace* exit) {
  assert(type.notBoxed());
  assert(ref->getType().isBoxed());
  return gen(LdRef, type, getFirstBlock(exit), ref);
}

SSATmp* TraceBuilder::genUnboxPtr(SSATmp* ptr) {
  return gen(UnboxPtr, ptr);
}

/**
 * Checks if the given SSATmp, or any of its aliases, is available in
 * any VM location, including locals and the This pointer.
 */
bool TraceBuilder::isValueAvailable(SSATmp* tmp) const {
  while (true) {
    if (m_refCountedMemValue == tmp) return true;
    if (anyLocalHasValue(tmp)) return true;
    if (callerLocalHasValue(tmp)) return true;

    IRInstruction* srcInstr = tmp->getInstruction();
    Opcode srcOpcode = srcInstr->getOpcode();

    if (srcOpcode == LdThis) return true;

    if (srcInstr->isPassthrough()) {
      tmp = srcInstr->getPassthroughValue();
    } else {
      return false;
    }
  }
}

void TraceBuilder::genDecRef(SSATmp* tmp) {
  if (!isRefCounted(tmp)) {
    return;
  }

  Type type = tmp->getType();
  if (type.isBoxed()) {
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

void TraceBuilder::genDecRefMem(SSATmp* base, int64_t offset, Type type) {
  gen(DecRefMem, type, base, genDefConst(offset));
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

Trace* TraceBuilder::genExitGuardFailure(uint32_t bcOff) {
  Trace* trace = makeExitTrace(bcOff);

  MarkerData marker;
  marker.bcOff    = bcOff;
  marker.stackOff = m_spOffset;
  marker.func     = m_curFunc->getValFunc();
  gen(Marker, marker); // goes on main trace

  SSATmp* pc = genDefConst((int64_t)bcOff);
  // TODO change exit trace to a control flow instruction that
  // takes sp, fp, and a Marker as the target label instruction
  trace->back()->push_back(
    m_irFactory.gen(getExitOpcode(TraceExitType::GuardFailure),
                    m_curFunc,
                    pc,
                    m_spValue,
                    m_fpValue));
  return trace;
}

/*
 * getExitSlowTrace generates a target exit trace for
 * TraceExitType::Slow branches.
 */
Trace* TraceBuilder::getExitSlowTrace(uint32_t bcOff,
                                      int32_t stackDeficit,
                                      uint32_t numOpnds,
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
                               Type type) {
  return gen(LdRaw, type, base, genDefConst(int64_t(kind)));
}

void TraceBuilder::genStRaw(SSATmp* base, RawMemSlot::Kind kind,
                            SSATmp* value) {
  gen(StRaw, base, genDefConst(int64_t(kind)), value);
}

void TraceBuilder::genTraceEnd(uint32_t nextPc,
                               TraceExitType::ExitType exitType /* = Normal */) {
  gen(getExitOpcode(TraceExitType::Normal),
      m_curFunc,
      genDefConst<int64_t>(nextPc),
      m_spValue,
      m_fpValue);
}

Trace* TraceBuilder::genExitTrace(uint32_t   bcOff,
                                  int32_t    stackDeficit,
                                  uint32_t   numOpnds,
                                  SSATmp* const* opnds,
                                  TraceExitType::ExitType exitType,
                                  uint32_t   notTakenBcOff,
                                  std::function<void(IRFactory*, Trace*)>
                                    beforeExit) {
  Trace* exitTrace = makeExitTrace(bcOff);

  MarkerData marker;
  marker.bcOff    = bcOff;
  marker.stackOff = m_spOffset + numOpnds - stackDeficit;
  marker.func     = m_curFunc->getValFunc();
  exitTrace->back()->push_back(m_irFactory.gen(Marker, marker));

  if (beforeExit) {
    beforeExit(&m_irFactory, exitTrace);
  }
  SSATmp* sp = m_spValue;
  if (numOpnds != 0 || stackDeficit != 0) {
    SSATmp* srcs[numOpnds + 2];
    srcs[0] = m_spValue;
    srcs[1] = genDefConst<int64_t>(stackDeficit);
    std::copy(opnds, opnds + numOpnds, srcs + 2);

    SSATmp** decayedPtr = srcs;
    auto* spillInst = m_irFactory.gen(
      SpillStack,
      std::make_pair(numOpnds + 2, decayedPtr)
    );
    sp = spillInst->getDst();
    exitTrace->back()->push_back(spillInst);
  }
  SSATmp* pc = genDefConst<int64_t>(bcOff);
  if (exitType == TraceExitType::NormalCc) {
    assert(notTakenBcOff != 0);
    SSATmp* notTakenPC = genDefConst(notTakenBcOff);
    genFor(exitTrace, getExitOpcode(exitType),
           m_curFunc,
           pc, sp, m_fpValue,
           notTakenPC);
  } else {
    assert(notTakenBcOff == 0);
    genFor(exitTrace, getExitOpcode(exitType),
           m_curFunc,
           pc, sp, m_fpValue);
  }
  return exitTrace;
}

SSATmp* TraceBuilder::genAdd(SSATmp* src1, SSATmp* src2) {
  Type resultType = Type::binArithResultType(src1->getType(),
                                             src2->getType());
  return gen(OpAdd, resultType, src1, src2);
}
SSATmp* TraceBuilder::genSub(SSATmp* src1, SSATmp* src2) {
  Type resultType = Type::binArithResultType(src1->getType(),
                                             src2->getType());
  return gen(OpSub, resultType, src1, src2);
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
  Type resultType = Type::binArithResultType(src1->getType(),
                                             src2->getType());
  return gen(OpMul, resultType, src1, src2);
}

SSATmp* TraceBuilder::genNot(SSATmp* src) {
  assert(src->getType() == Type::Bool);
  return genConvToBool(genXor(src, genDefConst<int64_t>(1)));
}

SSATmp* TraceBuilder::genDefUninit() {
  return gen(DefConst, Type::Uninit, ConstData(0));
}

SSATmp* TraceBuilder::genDefInitNull() {
  return gen(DefConst, Type::InitNull, ConstData(0));
}

SSATmp* TraceBuilder::genDefNull() {
  return gen(DefConst, Type::Null, ConstData(0));
}

SSATmp* TraceBuilder::genPtrToInitNull() {
  return gen(DefConst, Type::PtrToUninit, ConstData(&null_variant));
}

SSATmp* TraceBuilder::genPtrToUninit() {
  return gen(DefConst, Type::PtrToInitNull, ConstData(&init_null_variant));
}

SSATmp* TraceBuilder::genDefNone() {
  return gen(DefConst, Type::None, ConstData(0));
}

SSATmp* TraceBuilder::genConvToBool(SSATmp* src) {
  Type fromType = src->getType();
  if (fromType.isBool()) {
    return src;
  } else if (fromType.isNull()) {
    return genDefConst(false);
  } else if (fromType.isArray()) {
    return gen(ConvArrToBool, src);
  } else if (fromType.isDbl()) {
    return gen(ConvDblToBool, src);
  } else if (fromType.isInt()) {
    return gen(ConvIntToBool, src);
  } else if (fromType.isString()) {
    return gen(ConvStrToBool, src);
  } else if (fromType.isObj()) {
    // If a value is known to be an object, it is known to be non null.
    return genDefConst(true);
  } else {
    return gen(ConvCellToBool, src);
  }
}

SSATmp* TraceBuilder::genCmp(Opcode opc, SSATmp* src1, SSATmp* src2) {
  return gen(opc, src1, src2);
}

SSATmp* TraceBuilder::genJmp(Trace* targetTrace) {
  assert(targetTrace);
  return gen(Jmp_, getFirstBlock(targetTrace));
}

SSATmp* TraceBuilder::genJmpCond(SSATmp* boolSrc, Trace* target, bool negate) {
  assert(target);
  assert(boolSrc->getType() == Type::Bool);
  return gen(negate ? JmpZero : JmpNZero, getFirstBlock(target), boolSrc);
}

void TraceBuilder::genJmp(Block* target, SSATmp* src) {
  gen(Jmp_, target, EdgeData(), src);
}

void TraceBuilder::genExitWhenSurprised(Trace* targetTrace) {
  gen(ExitWhenSurprised, getFirstBlock(targetTrace));
}

void TraceBuilder::genExitOnVarEnv(Trace* targetTrace) {
  gen(ExitOnVarEnv, getFirstBlock(targetTrace), m_fpValue);
}

void TraceBuilder::genReleaseVVOrExit(Trace* exit) {
  gen(ReleaseVVOrExit, getFirstBlock(exit), m_fpValue);
}

void TraceBuilder::genGuardLoc(uint32_t id, Type type, Trace* exitTrace) {
  SSATmp* prevValue = getLocalValue(id);
  if (prevValue) {
    genGuardType(prevValue, type, exitTrace);
    return;
  }
  Type prevType = getLocalType(id);
  if (prevType == Type::None) {
    gen(GuardLoc, type, getFirstBlock(exitTrace), LocalId(id), m_fpValue);
  } else {
    // It doesn't make sense to be guarding on something that's deemed to fail
    assert(prevType == type);
  }
}

/**
 * Generates an AssertLoc instruction for the given local 'id' and 'type'.
 * If the 'override' flag is not set, then 'type' must be a subtype of the
 * previous tracked type for this local.
 */
void TraceBuilder::genAssertLoc(uint32_t id, Type type,
                                bool overrideType /* =false */) {
  Type prevType = overrideType ? Type::None : getLocalType(id);
  if (prevType == Type::None || type.strictSubtypeOf(prevType)) {
    gen(AssertLoc, type, LocalId(id), m_fpValue);
  } else {
    assert(prevType == type || prevType.strictSubtypeOf(type));
  }
}

SSATmp* TraceBuilder::genLdAssertedLoc(uint32_t id, Type type) {
  genAssertLoc(id, type);
  return genLdLoc(id);
}

void TraceBuilder::genGuardStk(uint32_t id, Type type, Trace* exitTrace) {
  gen(GuardStk, type, getFirstBlock(exitTrace), m_spValue,
      genDefConst<int64_t>(id));
}

SSATmp* TraceBuilder::genGuardType(SSATmp* src,
                                   Type type,
                                   Trace* target) {
  assert(target);
  return gen(GuardType, type, getFirstBlock(target), src);
}

void TraceBuilder::genGuardRefs(SSATmp* funcPtr,
                                SSATmp* nParams,
                                SSATmp* bitsPtr,
                                SSATmp* firstBitNum,
                                SSATmp* mask64,
                                SSATmp* vals64,
                                Trace*  exit) {
  gen(GuardRefs,
      getFirstBlock(exit),
      funcPtr,
      nParams,
      bitsPtr,
      firstBitNum,
      mask64,
      vals64);
}

SSATmp* TraceBuilder::genLdARFuncPtr(SSATmp* baseAddr, SSATmp* offset) {
  return gen(LdARFuncPtr, baseAddr, offset);
}

SSATmp* TraceBuilder::genLdPropAddr(SSATmp* obj, SSATmp* prop) {
  return gen(LdPropAddr, obj, prop);
}

SSATmp* TraceBuilder::genLdClsMethod(SSATmp* cls, uint32_t methodSlot) {
  return gen(LdClsMethod, cls, genDefConst<int64_t>(methodSlot));
}

SSATmp* TraceBuilder::genLdClsMethodCache(SSATmp* className,
                                          SSATmp* methodName,
                                          SSATmp* baseClass,
                                          Trace*  exit) {
  return gen(LdClsMethodCache, getFirstBlock(exit), className, methodName,
             baseClass);
}

SSATmp* TraceBuilder::genBoxLoc(uint32_t id) {
  SSATmp* prevValue  = genLdLoc(id);
  Type prevType = prevValue->getType();
  // Don't box if local's value already boxed
  if (prevType.isBoxed()) {
    return prevValue;
  }
  assert(prevType.notBoxed());
  // The Box helper requires us to incref the values its boxing, but in
  // this case we don't need to incref prevValue because we are simply
  // transfering its refcount from the local to the box.
  if (prevValue->isA(Type::Uninit)) {
    // No box can ever contain Uninit, so promote it to InitNull here.
    prevValue = genDefInitNull();
  }
  SSATmp* newValue = gen(Box, prevValue);
  genStLocAux(id, newValue, true);
  return newValue;
}

void TraceBuilder::genRaiseUninitLoc(uint32_t id) {
  gen(RaiseUninitLoc,
      genDefConst(m_curFunc->getValFunc()->localVarName(id)));
}

SSATmp* TraceBuilder::genLdAddr(SSATmp* base, int64_t offset) {
  return gen(LdAddr, base, genDefConst<int64_t>(offset));
}

/**
 * Returns an SSATmp containing the current value of the given local.
 * This generates a LdLoc instruction if needed.
 *
 * Note: the type of the local must be known already (due to type guards
 *       or assertions).
 */
SSATmp* TraceBuilder::genLdLoc(uint32_t id) {
  SSATmp* tmp = getLocalValue(id);
  if (tmp) {
    return tmp;
  }
  // No prior value for this local is available, so actually generate a LdLoc.
  auto type = getLocalType(id);
  assert(type != Type::None); // tracelet guards guarantee we have a type
  assert(type != Type::Null); // we can get Uninit or InitNull but not both
  if (type.isNull()) {
    tmp = genDefConst(type);
  } else {
    tmp = gen(LdLoc, type, LocalId(id), m_fpValue);
  }
  return tmp;
}

SSATmp* TraceBuilder::genLdLocAsCell(uint32_t id, Trace* exitTrace) {
  SSATmp*    tmp = genLdLoc(id);
  Type type = tmp->getType();
  assert(type.isBoxed() || type.notBoxed());
  if (!type.isBoxed()) {
    return tmp;
  }
  // Unbox tmp into a cell via a LdRef
  return genLdRef(tmp, type.innerType(), exitTrace);
}

SSATmp* TraceBuilder::genLdLocAddr(uint32_t id) {
  return gen(LdLocAddr, getLocalType(id).ptr(), LocalId(id), getFp());
}

void TraceBuilder::genStLocAux(uint32_t id, SSATmp* newValue, bool storeType) {
  gen(storeType ? StLoc : StLocNT,
      LocalId(id),
      m_fpValue,
      newValue);
}

/*
 * Initializes a local to the provided state.
 */
void TraceBuilder::genInitLoc(uint32_t id, SSATmp* t0) {
  genStLocAux(id, t0, true);
}

void TraceBuilder::genDecRefLoc(int id) {
  Type type = getLocalType(id);

  // Don't generate code if type is not refcounted
  if (type != Type::None && type.notCounted()) {
    return;
  }
  type = noneToGen(type);
  // When a parameter goes out of scope, we need to null
  // it out so that debug_backtrace can't capture stale
  // values.
  bool setNull = id < m_curFunc->getValFunc()->numParams();
  SSATmp* val = getLocalValue(id);
  if (val == nullptr && setNull) {
    val = gen(LdLoc, type, LocalId(id), m_fpValue);
  }
  if (val) {
    if (setNull) {
      genStLocAux(id, genDefUninit(), true);
    }
    genDecRef(val);
    return;
  }

  if (type.isBoxed()) {
    // we can't really rely on the types held in the boxed values since
    // aliasing stores may change them. We conservatively set the type
    // of the decref to a boxed cell.
    type = Type::BoxedCell;
  }

  gen(DecRefLoc, type, LocalId(id), m_fpValue);
}

/*
 * Stores a ref (boxed value) to a local. Also handles unsetting a local.
 */
void TraceBuilder::genBindLoc(uint32_t id,
                              SSATmp* newValue,
                              bool doRefCount /* = true */) {
  Type trackedType = getLocalType(id);
  SSATmp* prevValue = 0;
  if (trackedType == Type::None) {
    if (doRefCount) {
      prevValue = gen(LdLoc, Type::Gen, LocalId(id), m_fpValue);
    }
  } else {
    prevValue = getLocalValue(id);
    assert(prevValue == nullptr || prevValue->getType() == trackedType);
    if (prevValue == newValue) {
      // Silent store: local already contains value being stored
      // NewValue needs to be decref'ed
      if (!trackedType.notCounted() && doRefCount) {
        genDecRef(prevValue);
      }
      return;
    }
    if (trackedType.maybeCounted() && !prevValue && doRefCount) {
      prevValue = gen(LdLoc, trackedType, LocalId(id), m_fpValue);
    }
  }
  bool genStoreType = true;
  if ((trackedType.isBoxed() && newValue->getType().isBoxed()) ||
      (trackedType == newValue->getType() && !trackedType.isString())) {
    // no need to store type with local value
    genStoreType = false;
  }
  genStLocAux(id, newValue, genStoreType);
  if (trackedType.maybeCounted() && doRefCount) {
    genDecRef(prevValue);
  }
}

/*
 * Store a cell value to a local that might be boxed.
 */
SSATmp* TraceBuilder::genStLoc(uint32_t id,
                               SSATmp* newValue,
                               bool doRefCount,
                               bool genStoreType,
                               Trace* exit) {
  assert(!newValue->getType().isBoxed());
  /*
   * If prior value of local is a cell, then  re-use genBindLoc.
   * Otherwise, if prior value of local is a ref:
   *
   * prevLocValue = LdLoc<T>{id} fp
   *    prevValue = LdRef [prevLocValue]
   *       newRef = StRef [prevLocValue], newValue
   * DecRef prevValue
   * -- track local value in newRef
   */
  Type trackedType = getLocalType(id);
  assert(trackedType != Type::None);  // tracelet guards guarantee a type
  if (trackedType.notBoxed()) {
    SSATmp* retVal = doRefCount ? genIncRef(newValue) : newValue;
    genBindLoc(id, newValue, doRefCount);
    return retVal;
  }
  assert(trackedType.isBoxed());
  SSATmp* prevRef = getLocalValue(id);
  assert(prevRef == nullptr || prevRef->getType() == trackedType);
  // prevRef is a ref
  if (prevRef == nullptr) {
    // prevRef = ldLoc
    prevRef = gen(LdLoc, trackedType, LocalId(id), m_fpValue);
  }
  SSATmp* prevValue = nullptr;
  if (doRefCount) {
    assert(exit);
    Type innerType = trackedType.innerType();
    prevValue = gen(LdRef, innerType, getFirstBlock(exit), prevRef);
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

SSATmp* TraceBuilder::genNewObj(int32_t numParams, SSATmp* cls) {
  return gen(NewObj, cls, genDefConst<int64_t>(numParams), m_spValue, m_fpValue);
}

SSATmp* TraceBuilder::genNewObjCached(int32_t numParams,
                                      const StringData* className) {
  return gen(NewObjCached,
             genDefConst<int64_t>(numParams),
             genDefConst<const StringData*>(className),
             m_spValue,
             m_fpValue);
}

SSATmp* TraceBuilder::genNewObjNoCtorCached(const StringData* className) {
  return gen(NewObjNoCtorCached,
             genDefConst<const StringData*>(className),
             m_spValue);
}

SSATmp* TraceBuilder::genNewArray(int32_t capacity) {
  return gen(NewArray, genDefConst<int64_t>(capacity));
}

SSATmp* TraceBuilder::genNewTuple(int32_t numArgs, SSATmp* sp) {
  assert(numArgs >= 0);
  return gen(NewTuple, genDefConst<int64_t>(numArgs), sp);
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
                             uint32_t index,
                             bool& spansCall,
                             Type& type) {
  IRInstruction* inst = sp->getInstruction();
  switch (inst->getOpcode()) {
  case DefSP:
    return nullptr;

  case ReDefGeneratorSP: {
    auto srcInst = inst->getSrc(0)->getInstruction();
    assert(srcInst->getOpcode() == StashGeneratorSP);
    return getStackValue(srcInst->getSrc(0), index, spansCall, type);
  }
  case ReDefSP:
    return getStackValue(inst->getSrc(1), index, spansCall, type);

  case ExceptionBarrier:
    return getStackValue(inst->getSrc(0), index, spansCall, type);

  case AssertStk:
    // fallthrough
  case CastStk:
    // fallthrough
  case GuardStk: {
    // sp = GuardStk<T> sp, offset
    // We don't have a value, but we may know the type due to guarding
    // on it.
    if (inst->getSrc(1)->getValInt() == index) {
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
    int64_t numPushed    = 0;
    int32_t numSpillSrcs = inst->getNumSrcs() - 2;

    for (int i = 0; i < numSpillSrcs; ++i) {
      SSATmp* tmp = inst->getSrc(i + 2);
      if (index == numPushed) {
        if (tmp->getInstruction()->getOpcode() == IncRef) {
          tmp = tmp->getInstruction()->getSrc(0);
        }
        type = tmp->getType();
        if (!type.equals(Type::None)) {
          return tmp;
        }
      }
      ++numPushed;
    }

    // This is not one of the values pushed onto the stack by this
    // spillstack instruction, so continue searching.
    SSATmp* prevSp = inst->getSrc(0);
    int64_t numPopped = inst->getSrc(1)->getValInt();
    return getStackValue(prevSp,
                         // pop values pushed by spillstack
                         index - (numPushed - numPopped),
                         spansCall,
                         type);
  }

  case InterpOne: {
    SSATmp* prevSp = inst->getSrc(1);
    int64_t spAdjustment = inst->getSrc(3)->getValInt(); // # popped - # pushed
    Type resultType = inst->getTypeParam();
    if (index == 0 && resultType != Type::None) {
      type = resultType;
      return nullptr;
    }
    return getStackValue(prevSp, index + spAdjustment, spansCall, type);
  }

  case SpillFrame:
    return getStackValue(inst->getSrc(0),
                         // pushes an ActRec
                         index - kNumActRecCells,
                         spansCall,
                         type);

  case NewObj:
  case NewObjCached:
    if (index == kNumActRecCells) {
      // newly allocated object, which we unfortunately don't have any
      // kind of handle to :-(
      type = Type::Obj;
      return nullptr;
    }

    return getStackValue(sp->getInstruction()->getSrc(2),
                         // NewObj pushes an object and an ActRec
                         index - (1 + kNumActRecCells),
                         spansCall,
                         type);

  case NewObjNoCtorCached:
    if (index == 0) {
      type = Type::Obj;
      return nullptr;
    }

    return getStackValue(sp->getInstruction()->getSrc(1),
                         index - 1,
                         spansCall,
                         type);

  default: {
    SSATmp* value;
    if (VectorEffects::getStackValue(sp->getInstruction(), index,
                                     value, type)) {
      return value;
    } else {
      // If VectorEffects::getStackValue failed, it returns the next
      // sp to search in value.
      return getStackValue(value, index, spansCall, type);
    }
  }
  }

  // Should not get here!
  not_reached();
}

void TraceBuilder::genAssertStk(uint32_t id, Type type) {
  Type knownType = Type::None;
  bool spansCall = false;
  UNUSED SSATmp* tmp = getStackValue(m_spValue, id, spansCall, knownType);

  // We may have found a value if there was an inlined call.
  // AssertStk indicated that we knew the type from static analysis,
  // so let's double check.
  if (tmp) {
    assert(tmp->isA(type));
  }

  if (knownType == Type::None || type.strictSubtypeOf(knownType)) {
    gen(AssertStk, type, m_spValue, genDefConst<int64_t>(id));
  }
}

SSATmp* TraceBuilder::genCastStk(uint32_t id, Type type) {
  bool spansCall = false;
  Type knownType = Type::None;
  getStackValue(m_spValue, id, spansCall, knownType);
  if (knownType.subtypeOf(Type::None) || !knownType.subtypeOf(type)) {
    SSATmp* off = genDefConst<int>(id);
    gen(CastStk, m_spValue, off);
    IRInstruction* inst = m_spValue->getInstruction();
    inst->setTypeParam(type);
  }
  return m_spValue;
}

SSATmp* TraceBuilder::genLdStackAddr(SSATmp* sp, int64_t index) {
  Type type;
  bool spansCall;
  UNUSED SSATmp* val = getStackValue(sp, index, spansCall, type);
  type = noneToGen(type);
  assert(IMPLIES(val != nullptr, val->getType().equals(type)));
  assert(type.notPtr());
  return gen(LdStackAddr, type.ptr(), sp, genDefConst(index));
}

void TraceBuilder::genNativeImpl() {
  gen(NativeImpl, m_curFunc, m_fpValue);
}

SSATmp* TraceBuilder::genInterpOne(uint32_t pcOff,
                                   int32_t stackAdjustment,
                                   Type resultType) {
  return gen(InterpOne,
             resultType,
             m_fpValue,
             m_spValue,
             genDefConst<int64_t>(pcOff),
             genDefConst<int64_t>(stackAdjustment));
}

SSATmp* TraceBuilder::genCall(SSATmp* actRec,
                              uint32_t returnBcOffset,
                              SSATmp* func,
                              uint32_t numParams,
                              SSATmp** params) {
  SSATmp* srcs[numParams + 3];
  srcs[0] = actRec;
  srcs[1] = genDefConst<int64_t>(returnBcOffset);
  srcs[2] = func;
  std::copy(params, params + numParams, srcs + 3);
  SSATmp** decayedPtr = srcs;
  return gen(Call, std::make_pair(numParams + 3, decayedPtr));
}

SSATmp* TraceBuilder::genCallBuiltin(SSATmp* func,
                                     Type type,
                                     uint32_t numArgs,
                                     SSATmp** args) {
  SSATmp* srcs[numArgs + 1];
  srcs[0] = func;
  std::copy(args, args + numArgs, srcs + 1);
  SSATmp** decayedPtr = srcs;
  return gen(CallBuiltin, type, std::make_pair(numArgs + 1, decayedPtr));
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

void TraceBuilder::genDecRefStack(Type type, uint32_t stackOff) {
  bool spansCall = false;
  Type knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (!tmp || (spansCall && tmp->getInstruction()->getOpcode() != DefConst)) {
    // We don't want to extend live ranges of tmps across calls, so we
    // don't get the value if spansCall is true; however, we can use
    // any type information known.
    if (knownType != Type::None) {
      type = Type::mostRefined(type, knownType);
    }
    gen(DecRefStack, type, m_spValue, genDefConst<int64_t>(stackOff));
  } else {
    genDecRef(tmp);
  }
}

void TraceBuilder::genDecRefThis() {
  if (isThisAvailable()) {
    auto const thiss = genLdThis(nullptr);
    auto const thisInst = thiss->getInstruction();

    if (thisInst->getOpcode() == IncRef &&
        callerLocalHasValue(thisInst->getSrc(0))) {
      gen(DecRefNZ, thiss);
      return;
    }

    // It's a shame to keep a reference to the frame just to kill the
    // this pointer.  This is handled in optimizeActRecs.
    gen(DecRefKillThis, thiss, m_fpValue);
    return;
  }

  gen(DecRefThis, m_fpValue);
}

SSATmp* TraceBuilder::genGenericRetDecRefs(SSATmp* retVal, int numLocals) {
  return gen(GenericRetDecRefs, m_fpValue, retVal,
             genDefConst<int64_t>(numLocals));
}

void TraceBuilder::genIncStat(int32_t counter, int32_t value, bool force) {
  gen(IncStat,
      genDefConst<int64_t>(counter),
      genDefConst<int64_t>(value),
      genDefConst<bool>(force));
}

SSATmp* TraceBuilder::genIncRef(SSATmp* src) {
  return gen(IncRef, src);
}

SSATmp* TraceBuilder::genSpillStack(uint32_t stackAdjustment,
                                    uint32_t numOpnds,
                                    SSATmp** spillOpnds) {
  if (stackAdjustment == 0 && numOpnds == 0) {
    return m_spValue;
  }

  SSATmp* srcs[numOpnds + 2];
  srcs[0] = m_spValue;
  srcs[1] = genDefConst<int64_t>(stackAdjustment);
  std::copy(spillOpnds, spillOpnds + numOpnds, srcs + 2);
  SSATmp** decayedPtr = srcs;
  return gen(SpillStack, std::make_pair(numOpnds + 2, decayedPtr));
}

SSATmp* TraceBuilder::genLdStack(int32_t stackOff, Type type) {
  bool spansCall = false;
  Type knownType = Type::None;
  SSATmp* tmp = getStackValue(m_spValue, stackOff, spansCall, knownType);
  if (!tmp || (spansCall && tmp->getInstruction()->getOpcode() != DefConst)) {
    // We don't want to extend live ranges of tmps across calls, so we
    // don't get the value if spansCall is true; however, we can use
    // any type information known.
    if (knownType != Type::None) {
      type = Type::mostRefined(type, knownType);
    }
    return gen(LdStack,
               type,
               m_spValue,
               genDefConst<int64_t>(stackOff));
  }
  return tmp;
}

void TraceBuilder::genUnlinkContVarEnv() {
  gen(UnlinkContVarEnv, m_fpValue);
}

void TraceBuilder::genLinkContVarEnv() {
  gen(LinkContVarEnv, m_fpValue);
}

Trace* TraceBuilder::genContRaiseCheck(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContRaiseCheck, getFirstBlock(target), cont);
  return target;
}

Trace* TraceBuilder::genContPreNext(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContPreNext, getFirstBlock(target), cont);
  return target;
}

Trace* TraceBuilder::genContStartedCheck(SSATmp* cont, Trace* target) {
  assert(target);
  gen(ContStartedCheck, getFirstBlock(target), cont);
  return target;
}

SSATmp* TraceBuilder::genIterNext(uint32_t iterId, uint32_t localId) {
  // IterNext fpReg, iterId, localId
  return gen(IterNext,
             Type::Bool,
             m_fpValue,
             genDefConst<int64_t>(iterId),
             genDefConst<int64_t>(localId));
}

SSATmp* TraceBuilder::genIterNextK(uint32_t iterId,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  // IterNextK fpReg, iterId, valLocalId, keyLocalId
  return gen(IterNextK,
             Type::Bool,
             m_fpValue,
             genDefConst<int64_t>(iterId),
             genDefConst<int64_t>(valLocalId),
             genDefConst<int64_t>(keyLocalId));
}

SSATmp* TraceBuilder::genIterInit(SSATmp* src,
                                  uint32_t iterId,
                                  uint32_t valLocalId) {
  // IterInit src, fpReg, iterId, valLocalId
  return gen(IterInit,
             Type::Bool,
             src,
             m_fpValue,
             genDefConst<int64_t>(iterId),
             genDefConst<int64_t>(valLocalId));
}

SSATmp* TraceBuilder::genIterInitK(SSATmp* src,
                                   uint32_t iterId,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId) {
  // IterInitK src, fpReg, iterId, valLocalId, keyLocalId
  return gen(IterInitK,
             Type::Bool,
             src,
             m_fpValue,
             genDefConst<int64_t>(iterId),
             genDefConst<int64_t>(valLocalId),
             genDefConst<int64_t>(keyLocalId));
}

SSATmp* TraceBuilder::genIterFree(uint32_t iterId) {
  return gen(IterFree, m_fpValue, genDefConst<int64_t>(iterId));
}

void TraceBuilder::updateTrackedState(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  // Update tracked state of local values/types, stack/frame pointer, CSE, etc.

  // kill tracked memory values
  if (inst->mayModifyRefs()) {
    m_refCountedMemValue = nullptr;
  }

  switch (opc) {
    case Call:
      m_spValue = inst->getDst();
      // A call pops the ActRec and pushes a return value.
      m_spOffset -= kNumActRecCells;
      m_spOffset += 1;
      assert(m_spOffset >= 0);
      killCse();
      killLocals();
      break;

    case ContEnter:
      killCse();
      killLocals();
      break;

    case DefFP:
    case FreeActRec:
      m_fpValue = inst->getDst();
      break;

    case ReDefGeneratorSP:
    case DefSP:
    case ReDefSP:
      m_spValue = inst->getDst();
      m_spOffset = inst->getExtra<StackOffset>()->offset;
      break;

    case AssertStk:
    case CastStk:
    case GuardStk:
    case ExceptionBarrier:
      m_spValue = inst->getDst();
      break;

    case SpillStack: {
      m_spValue = inst->getDst();
      // Push the spilled values but adjust for the popped values
      int64_t stackAdjustment = inst->getSrc(1)->getValInt();
      m_spOffset -= stackAdjustment;
      m_spOffset += spillValueCells(inst);
      break;
    }

    case SpillFrame:
      m_spValue = inst->getDst();
      m_spOffset += kNumActRecCells;
      break;

    case NewObj:
    case NewObjCached:
      m_spValue = inst->getDst();
      // new obj leaves the new object and an actrec on the stack
      m_spOffset += kNumActRecCells + 1;
      break;

    case NewObjNoCtorCached:
      m_spValue = inst->getDst();
      m_spOffset += 1;
      break;

    case InterpOne: {
      m_spValue = inst->getDst();
      int64_t stackAdjustment = inst->getSrc(3)->getValInt();
      Type resultType = inst->getTypeParam();
      // push the return value if any and adjust for the popped values
      m_spOffset -= stackAdjustment;
      break;
    }

    case StProp:
    case StPropNT:
      // fall through to StMem; stored value is the same arg number (2)
    case StMem:
    case StMemNT:
      m_refCountedMemValue = inst->getSrc(2);
      break;

    case LdMem:
    case LdProp:
    case LdRef:
      m_refCountedMemValue = inst->getDst();
      break;

    case StRefNT:
    case StRef: {
      m_refCountedMemValue = inst->getSrc(2);
      SSATmp* newRef = inst->getDst();
      SSATmp* prevRef = inst->getSrc(0);
      // update other tracked locals that also contain prevRef
      updateLocalRefValues(prevRef, newRef);
      break;
    }

    case StLocNT:
    case StLoc:
      setLocalValue(inst->getExtra<LocalId>()->locId,
                    inst->getSrc(1));
      break;

    case LdLoc:
      setLocalValue(inst->getExtra<LdLoc>()->locId, inst->getDst());
      break;

    case AssertLoc:
    case GuardLoc:
      setLocalType(inst->getExtra<LocalId>()->locId,
                   inst->getTypeParam());
      break;

    case IterInitK:
      // kill the locals to which this instruction stores iter's key and value
      killLocalValue(inst->getSrc(3)->getValInt());
      killLocalValue(inst->getSrc(4)->getValInt());
      break;

    case IterInit:
      // kill the local to which this instruction stores iter's value
      killLocalValue(inst->getSrc(3)->getValInt());
      break;

    case IterNextK:
      // kill the locals to which this instruction stores iter's key and value
      killLocalValue(inst->getSrc(2)->getValInt());
      killLocalValue(inst->getSrc(3)->getValInt());
      break;

    case IterNext:
      // kill the local to which this instruction stores iter's value
      killLocalValue(inst->getSrc(2)->getValInt());
      break;

    case LdThis:
      m_thisIsAvailable = true;
      break;

    default:
      break;
  }

  if (VectorEffects::supported(inst)) {
    VectorEffects::get(inst,
                       [&](uint32_t id, SSATmp* val) { // storeLocalValue
                         setLocalValue(id, val);
                       },
                       [&](uint32_t id, Type t) { // setLocalType
                         setLocalType(id, t);
                       });
  }

  if (inst->modifiesStack()) {
    m_spValue = inst->modifiedStkPtr();
  }

  // update the CSE table
  if (m_enableCse && inst->canCSE()) {
    cseInsert(inst);
  }

  // if the instruction kills any of its sources, remove them from the
  // CSE table
  if (inst->killsSources()) {
    for (int i = 0; i < inst->getNumSrcs(); ++i) {
      if (inst->killsSource(i)) {
        cseKill(inst->getSrc(i));
      }
    }
  }

  // save a copy of the current state for each successor.
  if (Block* target = inst->getTaken()) saveState(target);
}

void TraceBuilder::beginInlining(const Func* target,
                                 SSATmp* calleeFP,
                                 SSATmp* calleeSP,
                                 SSATmp* savedSP,
                                 int32_t savedSPOff) {
  // Saved tracebuilder state will include the "return" fp/sp.
  // Whatever the current fpValue is is good enough, but we have to be
  // passed in the StkPtr that represents the stack prior to the
  // ActRec being allocated.
  m_spOffset = savedSPOff;
  m_spValue = savedSP;

  m_inlineSavedStates.push_back(createState());

  /*
   * Set up the callee state.
   *
   * We set m_thisIsAvailable to true on any object method, because we
   * just don't inline calls to object methods with a null $this.
   */
  m_fpValue         = calleeFP;
  m_spValue         = calleeSP;
  m_thisIsAvailable = target->cls() != nullptr;
  m_curFunc         = genDefConst(target);

  /*
   * Keep the outer locals somewhere for isValueAvailable() to know
   * about their liveness, to help with incref/decref elimination.
   */
  m_callerAvailableValues.insert(m_callerAvailableValues.end(),
                                 m_localValues.begin(),
                                 m_localValues.end());

  m_localValues.clear();
  m_localTypes.clear();
  m_localValues.resize(target->numLocals(), nullptr);
  m_localTypes.resize(target->numLocals(), Type::None);

  gen(ReDefSP, StackOffset(target->numParams()), m_fpValue, m_spValue);
}

void TraceBuilder::endInlining() {
  auto calleeAR = m_fpValue;
  gen(InlineReturn, calleeAR);

  useState(std::move(m_inlineSavedStates.back()));
  m_inlineSavedStates.pop_back();

  // See the comment in beginInlining about generator frames.
  if (m_curFunc->getValFunc()->isGenerator()) {
    gen(ReDefGeneratorSP, StackOffset(m_spOffset), m_spValue);
  } else {
    gen(ReDefSP, StackOffset(m_spOffset), m_fpValue, m_spValue);
  }
}

std::unique_ptr<TraceBuilder::State> TraceBuilder::createState() const {
  std::unique_ptr<State> state(new State);
  state->spValue = m_spValue;
  state->fpValue = m_fpValue;
  state->curFunc = m_curFunc;
  state->spOffset = m_spOffset;
  state->thisAvailable = m_thisIsAvailable;
  state->localValues = m_localValues;
  state->localTypes = m_localTypes;
  state->callerAvailableValues = m_callerAvailableValues;
  state->refCountedMemValue = m_refCountedMemValue;
  return state;
}

/*
 * Save current state for block.  If this is the first time saving state
 * for block, create a new snapshot.  Otherwise merge the current state into
 * the existing snapshot.
 */
void TraceBuilder::saveState(Block* block) {
  if (State* state = m_snapshots[block]) {
    mergeState(state);
  } else {
    m_snapshots[block] = createState().release();
  }
}

/*
 * Merge current state into state.  Frame pointers and stack depth must match.
 * If the stack pointer tmps are different, clear the tracked value (we can
 * make a new one, given fp and spOffset).
 *
 * thisIsAvailable remains true if it's true in both states.
 * local variable values are preserved if the match in both states.
 * types are combined using Type::unionOf.
 */
void TraceBuilder::mergeState(State* state) {
  // cannot merge fp or spOffset state, so assert they match
  assert(state->fpValue == m_fpValue);
  assert(state->spOffset == m_spOffset);
  assert(state->curFunc == m_curFunc);
  if (state->spValue != m_spValue) {
    // we have two different sp definitions but we know they're equal
    // because spOffset matched.
    state->spValue = nullptr;
  }
  // this is available iff it's available in both states
  state->thisAvailable &= m_thisIsAvailable;
  for (unsigned i = 0, n = state->localValues.size(); i < n; ++i) {
    // preserve local values if they're the same in both states,
    // This would be the place to insert phi nodes (jmps+deflabels) if we want
    // to avoid clearing state, which triggers a downstream reload.
    if (state->localValues[i] != m_localValues[i]) {
      state->localValues[i] = nullptr;
    }
  }
  for (unsigned i = 0, n = state->localTypes.size(); i < n; ++i) {
    // combine types using Type::unionOf(), but handle Type::None here (t2135185)
    Type t1 = state->localTypes[i];
    Type t2 = m_localTypes[i];
    state->localTypes[i] = (t1 == Type::None || t2 == Type::None) ? Type::None :
                           Type::unionOf(t1, t2);
  }
  // Reference counted memory value is available only if it is available on both
  // paths
  if (state->refCountedMemValue != m_refCountedMemValue) {
    state->refCountedMemValue = nullptr;
  }

  // Don't attempt to continue tracking caller's available values.
  state->callerAvailableValues.clear();
}

void TraceBuilder::useState(std::unique_ptr<State> state) {
  m_spValue = state->spValue;
  m_fpValue = state->fpValue;
  m_spOffset = state->spOffset;
  m_curFunc = state->curFunc;
  m_thisIsAvailable = state->thisAvailable;
  m_refCountedMemValue = state->refCountedMemValue;
  m_localValues = std::move(state->localValues);
  m_localTypes = std::move(state->localTypes);
  m_callerAvailableValues = std::move(state->callerAvailableValues);
  // If spValue is null, we merged two different but equivalent values.
  // Define a new sp using the known-good spOffset.
  if (!m_spValue) {
    gen(DefSP, StackOffset(m_spOffset), m_fpValue);
  }
}

void TraceBuilder::useState(Block* block) {
  assert(m_snapshots[block]);
  std::unique_ptr<State> state(m_snapshots[block]);
  m_snapshots[block] = nullptr;
  useState(std::move(state));
}

void TraceBuilder::clearTrackedState() {
  killCse(); // clears m_cseHash
  for (uint32_t i = 0; i < m_localValues.size(); i++) {
    m_localValues[i] = nullptr;
  }
  for (uint32_t i = 0; i < m_localTypes.size(); i++) {
    m_localTypes[i] = Type::None;
  }
  m_callerAvailableValues.clear();
  m_spValue = m_fpValue = nullptr;
  m_spOffset = 0;
  m_thisIsAvailable = false;
  m_refCountedMemValue = nullptr;
  for (auto i = m_snapshots.begin(), end = m_snapshots.end(); i != end; ++i) {
    delete *i;
    *i = nullptr;
  }
}

void TraceBuilder::appendInstruction(IRInstruction* inst, Block* block) {
  Opcode opc = inst->getOpcode();
  if (opc != Nop && opc != DefConst) {
    block->push_back(inst);
  }
}

void TraceBuilder::appendInstruction(IRInstruction* inst) {
  Block* block = m_trace->back();
  IRInstruction* prev = block->back();
  if (prev->isBlockEnd()) {
    // start a new block
    Block* next = m_irFactory.defBlock(m_curFunc->getValFunc());
    m_trace->push_back(next);
    if (!prev->isTerminal()) {
      // new block is reachable from old block so link it.
      block->setNext(next);
    }
    block = next;
  }
  appendInstruction(inst, block);
  updateTrackedState(inst);
}

void TraceBuilder::appendBlock(Block* block) {
  if (!m_trace->back()->back()->isTerminal()) {
    // previous instruction falls through; merge current state with block.
    saveState(block);
  }
  m_trace->push_back(block);
  useState(block);
}

CSEHash* TraceBuilder::getCSEHashTable(IRInstruction* inst) {
  return inst->getOpcode() == DefConst ? &m_irFactory.getConstTable() :
         &m_cseHash;
}

void TraceBuilder::cseInsert(IRInstruction* inst) {
  getCSEHashTable(inst)->insert(inst->getDst());
}

void TraceBuilder::cseKill(SSATmp* src) {
  if (src->getInstruction()->canCSE()) {
    getCSEHashTable(src->getInstruction())->erase(src);
  }
}

SSATmp* TraceBuilder::cseLookup(IRInstruction* inst) {
  return getCSEHashTable(inst)->lookup(inst);
}

SSATmp* TraceBuilder::optimizeWork(IRInstruction* inst) {
  static DEBUG_ONLY __thread int instNest = 0;
  if (debug) ++instNest;
  SCOPE_EXIT { if (debug) --instNest; };
  DEBUG_ONLY auto indent = [&] { return std::string(instNest * 2, ' '); };

  FTRACE(1, "{}{}\n", indent(), inst->toString());

  // copy propagation on inst source operands
  copyProp(inst);

  SSATmp* result = nullptr;
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
      assert(inst->hasDst());
      return result;
    }
  }
  return nullptr;
}

SSATmp* TraceBuilder::optimizeInst(IRInstruction* inst) {
  if (SSATmp* tmp = optimizeWork(inst)) {
    return tmp;
  }
  // Couldn't CSE or simplify the instruction; clone it and append.
  if (inst->getOpcode() != Nop) {
    inst = inst->clone(&m_irFactory);
    appendInstruction(inst);
    // returns nullptr if instruction has no dest, returns the first
    // (possibly only) dest otherwise
    return inst->getDst(0);
  }
  return nullptr;
}

void CSEHash::filter(Block* block, IdomVector& idoms) {
  for (auto it = map.begin(), end = map.end(); it != end;) {
    auto next = it; ++next;
    if (!dominates(it->second->getInstruction()->getBlock(), block, idoms)) {
      map.erase(it);
    }
    it = next;
  }
}

/*
 * optimizeTrace runs another pass of CSE and simplification on an
 * already-built trace, like this:
 *
 *   reset state.
 *   move all blocks to a temporary list.
 *   compute immediate dominators.
 *   for each block in trace order:
 *     if we have a snapshot state for this block:
 *       clear cse entries that don't dominate this block.
 *       use snapshot state.
 *     move all instructions to a temporary list.
 *     for each instruction:
 *       optimizeWork - do CSE and simplify again
 *       if not simplified:
 *         append existing instruction and update state.
 *       else:
 *         if the instruction has a result, insert a mov from the
 *         simplified tmp to the original tmp and discard the instruction.
 *     if the last conditional branch was turned into a jump, remove the
 *     fall-through edge to the next block.
 */
void TraceBuilder::optimizeTrace() {
  m_enableCse = RuntimeOption::EvalHHIRCse;
  m_enableSimplification = RuntimeOption::EvalHHIRSimplification;
  if (!m_enableCse && !m_enableSimplification) return;
  if (m_trace->getBlocks().size() >
      RuntimeOption::EvalHHIRSimplificationMaxBlocks) {
    // TODO CSEHash::filter is very slow for large block sizes
    // t2135219 should address that
    return;
  }
  BlockList sortedBlocks = sortCfg(m_trace.get(), m_irFactory);
  IdomVector idoms = findDominators(sortedBlocks);
  clearTrackedState();
  auto blocks = std::move(m_trace->getBlocks());
  assert(m_trace->getBlocks().empty());
  while (!blocks.empty()) {
    Block* block = blocks.front();
    blocks.pop_front();
    assert(block->getTrace() == m_trace.get());
    m_trace->push_back(block);
    if (m_snapshots[block]) {
      useState(block);
      m_cseHash.filter(block, idoms);
    }
    auto instructions = std::move(block->getInstrs());
    assert(block->empty());
    while (!instructions.empty()) {
      auto *inst = &instructions.front();
      instructions.pop_front();
      SSATmp* tmp = optimizeWork(inst); // Can generate new instrs!
      if (!tmp) {
        // Could not optimize; keep the old instruction
        appendInstruction(inst, block);
        updateTrackedState(inst);
        continue;
      }
      SSATmp* dst = inst->getDst();
      if (dst->getType() != Type::None && dst != tmp) {
        // The result of optimization has a different destination than the inst.
        // Generate a mov(tmp->dst) to get result into dst. If we get here then
        // assume the last instruction in the block isn't a guard. If it was,
        // we would have to insert the mov on the fall-through edge.
        assert(!block->back()->isBlockEnd());
        IRInstruction* mov = m_irFactory.mov(dst, tmp);
        appendInstruction(mov, block);
        updateTrackedState(mov);
      }
      // Not re-adding inst; remove the inst->taken edge
      if (inst->getTaken()) inst->setTaken(nullptr);
    }
    if (block->back()->isTerminal()) {
      // Could have converted a conditional branch to Jmp; clear next.
      block->setNext(nullptr);
    } else {
      // if the last instruction was a branch, we already saved state
      // for the target in updateTrackedState().  Now save state for
      // the fall-through path.
      saveState(block->getNext());
    }
  }
}

void TraceBuilder::killCse() {
  m_cseHash.clear();
}

SSATmp* TraceBuilder::getLocalValue(unsigned id) const {
  assert(id < m_localValues.size());
  return m_localValues[id];
}

Type TraceBuilder::getLocalType(unsigned id) const {
  assert(id < m_localTypes.size());
  return m_localTypes[id];
}

void TraceBuilder::setLocalValue(unsigned id, SSATmp* value) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = value;
  m_localTypes[id] = value ? value->getType() : Type::None;
}

void TraceBuilder::setLocalType(uint32_t id, Type type) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = nullptr;
  m_localTypes[id] = type;
}

// Needs to be called if a local escapes as a by-ref or
// otherwise set to an unknown value (e.g., by Iter[Init,Next][K])
void TraceBuilder::killLocalValue(uint32_t id) {
  assert(id < m_localValues.size() && id < m_localTypes.size());
  m_localValues[id] = nullptr;
  m_localTypes[id] = Type::None;
}

bool TraceBuilder::anyLocalHasValue(SSATmp* tmp) const {
  return std::find(m_localValues.begin(),
                   m_localValues.end(),
                   tmp) != m_localValues.end();
}

bool TraceBuilder::callerLocalHasValue(SSATmp* tmp) const {
  return std::find(m_callerAvailableValues.begin(),
                   m_callerAvailableValues.end(),
                   tmp) != m_callerAvailableValues.end();
}

//
// This method updates the tracked values and types of all locals that contain
// oldRef so that they now contain newRef.
// This should only be called for ref/boxed types.
//
void TraceBuilder::updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef) {
  assert(oldRef->getType().isBoxed());
  assert(newRef->getType().isBoxed());

  Type newRefType = newRef->getType();
  size_t nTrackedLocs = m_localValues.size();
  for (size_t id = 0; id < nTrackedLocs; id++) {
    if (m_localValues[id] == oldRef) {
      m_localValues[id] = newRef;
      m_localTypes[id]  = newRefType;
    }
  }
}

/**
 * This method changes any boxed local into a BoxedCell type.
 */
void TraceBuilder::dropLocalRefsInnerTypes() {
  for (size_t id = 0; id < m_localTypes.size(); id++) {
    if (m_localTypes[id].isBoxed()) {
      m_localTypes[id] = Type::BoxedCell;
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
   for (uint32_t i = 0; i < m_localValues.size(); i++) {
    SSATmp* t = m_localValues[i];
    // should not kill DefConst, and LdConst should be replaced by DefConst
    if (!t || t->getInstruction()->getOpcode() == DefConst) {
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
    m_localValues[i] = nullptr;
  }
}

}}} // namespace HPHP::VM::JIT
