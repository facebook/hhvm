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

#ifndef _TRACEBUILDER_H_
#define _TRACEBUILDER_H_

#include <boost/scoped_ptr.hpp>

#include "ir.h"
#include "cse.h"
#include "simplifier.h"

namespace HPHP {
namespace VM {
namespace JIT {

class TraceBuilder {
public:
  TraceBuilder(uint32 initialBcOffset,
               IRFactory& irFactory_,
               CSEHash& constants,
               const Func* func=NULL);

  Trace* makeExitTrace(uint32 bcOff) {
    return m_trace->addExitTrace(makeTrace(bcOff, false));
  }
  bool isThisAvailable() const {
    return m_thisIsAvailable;
  }
  void setThisAvailable() {
    m_thisIsAvailable = true;
  }

  void start(uint32 initialBcOffset, uint32 initialSpOffsetFromFp);
  void genPrint(SSATmp*);

  uint32 getInitialBcOffset() {return m_initialBcOff;}

  SSATmp* genAddElem(SSATmp* arr, SSATmp* key, SSATmp* val);
  SSATmp* genAddNewElem(SSATmp* arr, SSATmp* val);
  SSATmp* genDefCns(const StringData* cnsName, SSATmp* val);
  SSATmp* genConcat(SSATmp* tl, SSATmp* tr);
  SSATmp* genArrayAdd(SSATmp* src1, SSATmp* src2);
  void    genDefCls(PreClass*, const HPHP::VM::Opcode* after);
  void    genDefFunc(Func*);

  SSATmp* genLdThis(Trace* trace);
  SSATmp* genLdVarEnv();
  SSATmp* genLdRetAddr();
  SSATmp* genLdRaw(SSATmp* base, RawMemSlot::Kind kind, Type::Tag type);
  void    genStRaw(SSATmp* base, RawMemSlot::Kind kind, SSATmp* value);

  SSATmp* genLdLoc(uint32 id);
  SSATmp* genLdLoc(uint32 id, Type::Tag type, Trace* exitTrace);
  SSATmp* genLdLocAddr(uint32 id);
  SSATmp* genStLoc(uint32 id,
                   SSATmp* src,
                   bool doRefCount,
                   bool genStoreType,
                   Trace* exit);
  SSATmp* genLdMem(SSATmp* addr, Type::Tag type, Trace* target);
  SSATmp* genLdMem(SSATmp* addr, int64 offset, Type::Tag type, Trace* target);
  void    genStMem(SSATmp* addr, SSATmp* src, bool genStoreType);
  void    genStMem(SSATmp* addr, int64 offset, SSATmp* src, bool stType);
  SSATmp* genLdProp(SSATmp* obj, SSATmp* prop, Type::Tag type, Trace* exit);
  void    genStProp(SSATmp* obj, SSATmp* prop, SSATmp* src, bool genStoreType);
  void    genSetPropCell(SSATmp* base, int64 offset, SSATmp* value);

  SSATmp* genBoxLoc(uint32 id);
  void    genBindLoc(uint32 id, SSATmp* ref, bool doRefCount = true);
  void    genInitLoc(uint32 id, SSATmp* t);
  void    killLocalValue(int id);
  bool    anyLocalHasValue(SSATmp* tmp) const;
  bool    isValueAvailable(SSATmp* tmp) const;

  SSATmp* genLdHome(uint32 id);
  SSATmp* genLdCachedClass(SSATmp* classNameOpnd);
  SSATmp* genLdCls(SSATmp* classNameOpnd);
  SSATmp* genLdClsCns(SSATmp* cnsName,
                      SSATmp* cls, /* Str or Cls */
                      Trace* exitTrace);
  SSATmp* genLdCurFuncPtr();
  SSATmp* genLdARFuncPtr(SSATmp* baseAddr, SSATmp* offset);
  SSATmp* genLdFuncCls(SSATmp* func);
  SSATmp* genNewObj(int32 numParams, const StringData* clsName);
  SSATmp* genNewObj(int32 numParams, SSATmp* cls);
  SSATmp* genNewArray(int32 capacity);
  SSATmp* genNewTuple(int32 numArgs, SSATmp* sp);
  // The first three genAllocActRecs delegate to the forth one
  SSATmp* genAllocActRec(const Func* func,
                         SSATmp* objOrCls,
                         int32 numParams,
                         const StringData* magicName);
  SSATmp* genAllocActRec(SSATmp* func, int32 numParams);
  SSATmp* genAllocActRec(SSATmp* func, SSATmp* obj, int32 numParams);
  SSATmp* genAllocActRec(SSATmp* func,
                         SSATmp* objOrCls,
                         int32   numParams,
                         const StringData* magicName);
  SSATmp* genAllocActRec(); // creates an uninitialized actrec
  SSATmp* genFreeActRec();
  void    genGuardLoc(uint32 id, Type::Tag type, Trace* exitTrace);
  void    genGuardStk(uint32 id, Type::Tag type, Trace* exitTrace);
  SSATmp* genGuardType(SSATmp* src, Type::Tag type, Trace* nextTrace);
  void    genGuardRefs(SSATmp* funcPtr,
                       SSATmp* nParams,
                       SSATmp* bitsPtr,
                       SSATmp* firstBitNum,
                       SSATmp* mask64,
                       SSATmp* vals64,
                       Trace*  exitTrace);

  SSATmp* genUnbox(SSATmp* src, Type::Tag type, Trace* exit);
  SSATmp* genUnboxPtr(SSATmp* ptr);
  SSATmp* genLdRef(SSATmp* ref, Type::Tag type, Trace* exit);
  SSATmp* genIntegerOp(Opcode opc, SSATmp* src1, SSATmp* src2);
  SSATmp* genIntegerOp(Opcode opc, SSATmp* src);
  SSATmp* genAdd(SSATmp* src1, SSATmp* src2);
  void    genRaiseUninitWarning(uint32 id);

  SSATmp* genSub(SSATmp* src1, SSATmp* src2);
  SSATmp* genMul(SSATmp* src1, SSATmp* src2);
  SSATmp* genAnd(SSATmp* src1, SSATmp* src2);
  SSATmp* genOr(SSATmp* src1, SSATmp* src2);
  SSATmp* genXor(SSATmp* src1, SSATmp* src2);
  SSATmp* genNot(SSATmp* src);

  SSATmp* genDefUninit();
  SSATmp* genDefNull();
  Trace*  genJmpCond(SSATmp* src, Trace* target, bool negate);
  Trace*  genJmp(Trace* target);
  Trace*  genExitWhenSurprised(Trace* target);
  Trace*  genExitOnVarEnv(Trace* target);
  Trace*  genCheckUninit(SSATmp* src, Trace* target);
  SSATmp* genCmp(Opcode opc, SSATmp* src1, SSATmp* src2);
  SSATmp* genConvToBool(SSATmp* src);
  SSATmp* genConvToInt(SSATmp* src);
  SSATmp* genConvToDbl(SSATmp* src);
  SSATmp* genConvToStr(SSATmp* src);
  SSATmp* genConvToArr(SSATmp* src);
  SSATmp* genConvToObj(SSATmp* src);
  SSATmp* genLdPropAddr(SSATmp* obj, SSATmp* prop);
  SSATmp* genLdClsPropAddr(SSATmp* cls, SSATmp* clsName, SSATmp* propName);
  SSATmp* genLdClsMethod(SSATmp* cls, uint32 methodSlot);
  SSATmp* genLdClsMethodCache(SSATmp* methodName, SSATmp* classOpnd);
  SSATmp* genLdClsMethodCache(SSATmp* className,
                              SSATmp* methodName,
                              SSATmp* baseClass,
                              Trace* slowPathExit);
  SSATmp* genLdObjMethod(const StringData* methodName, SSATmp* obj);
  SSATmp* genLdObjClass(SSATmp* obj);
  SSATmp* genLdFunc(SSATmp* funcName, SSATmp* actRec);
  SSATmp* genLdFixedFunc(const StringData* funcName, SSATmp* actRec);
  SSATmp* genCall(SSATmp* actRec,
                  uint32 returnBcOffset,
                  SSATmp* func,
                  uint32 numParams,
                  SSATmp** params);
  IRInstruction* genMarker(uint32 bcOff, int spOff);
  SSATmp* genRetVal(SSATmp* val);
  SSATmp* genRetVal(); // for native impl
  IRInstruction* genRetCtrl(SSATmp* sp, SSATmp* fp, SSATmp* retAddr);
  void genDecRef(SSATmp* tmp);
  void genDecRefStack(Type::Tag type,
                      uint32 stackOff,
                      Trace* exit);
  void genDecRefLoc(int id);
  void genDecRefThis();
  void genDecRefLocalsThis(uint32 numLocals);
  void genDecRefLocals(uint32 numLocals);
  void genIncStat(int32 counter, int32 value);
  SSATmp* genIncRef(SSATmp* src);
  SSATmp* genSpillStack(uint32 stackAdjustment,
                        uint32 numOpnds,
                        SSATmp** opnds,
                        bool allocActRec = false);
  SSATmp* genLdStack(int32 stackOff, Type::Tag type, Trace* target);
  SSATmp* genDefFP();
  SSATmp* genDefSP();
  SSATmp* genLdStackAddr(int64 offset);
  SSATmp* genQueryOp(Opcode queryOpc, SSATmp* addr);
  Trace*  genVerifyParamType(SSATmp* objClass, SSATmp* className,
                             const Class* constraint, Trace* exitTrace);
  SSATmp* genInstanceOfD(SSATmp* src, SSATmp* className);

  void    genNativeImpl();

  SSATmp* genCreateCont(bool getArgs, const Func* origFunc,
                        const Func* genFunc);
  void    genFillContLocals(const Func* origFunc, const Func* genFunc,
                            SSATmp* cont);
  void    genFillContThis(SSATmp* cont, SSATmp* locals, int64 offset);
  void    genUnlinkContVarEnv();
  void    genLinkContVarEnv();
  Trace*  genContRaiseCheck(SSATmp* cont, Trace* target);
  Trace*  genContPreNext(SSATmp* cont, Trace* target);
  Trace*  genContStartedCheck(SSATmp* cont, Trace* target);

  void    genIncStat(SSATmp* counter, SSATmp* value);

  SSATmp* genInterpOne(uint32 pcOff, uint32 stackAdjustment,
                       Type::Tag resultType, Trace* target);
  Trace* getExitSlowTrace(uint32 bcOff,
                          int32 stackDeficit,
                          uint32 numOpnds,
                          SSATmp** opnds);

  // generates a trace exit that can be the target of a conditional
  // or unconditional control flow instruction from the main trace
  Trace* genExitTrace(uint32 bcOff,
                      int32  stackDeficit,
                      uint32 numOpnds,
                      SSATmp** opnds,
                      TraceExitType::ExitType,
                      uint32 notTakenBcOff = 0);

  void genExitGuardFailure(uint32 off);
  // generates the ExitTrace instruction at the end of a trace
  void genTraceEnd(uint32 nextPc,
                   TraceExitType::ExitType exitType = TraceExitType::Normal);
  SSATmp* getSSATmp(IRInstruction* inst);
  SSATmp* optimizeInst(IRInstruction* inst);
  SSATmp* cse(IRInstruction* inst);
  SSATmp* cseLookup(IRInstruction* inst);
  SSATmp* cseInsert(IRInstruction* inst, bool clone);
  CSEHash* getCSEHashTable(IRInstruction* inst);
  void killCse();
  void killLocals();
  void updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef);
  Local getLocal(uint32 id);
  IRInstruction* appendInstruction(IRInstruction* inst);
  SSATmp* getLocalValue(int id);
  Type::Tag getLocalType(int id);
  void setLocalValue(int id, SSATmp* value);
  void setLocalType(int id, Type::Tag type);

  void finalizeTrace();

  template<Type::Tag T>
  SSATmp* genIsType(SSATmp* src) {
    TypeInstruction inst(IsType, T, Type::Bool, src);
    return optimizeInst(&inst);
  }

  template<typename T>
  SSATmp* genDefConst(T val) {
    ConstInstruction inst(DefConst, val);
    return optimizeInst(&inst);
  }

  template<typename T>
  SSATmp* genLdConst(T val) {
    ConstInstruction inst(LdConst, val);
    return optimizeInst(&inst);
  }

  Trace* getTrace() { return m_trace.get(); }
  Trace* getExitGuardFailureTrace() { return m_exitGuardFailureTrace; }
  IRFactory* getIrFactory() { return &m_irFactory; }
  int32 getSpOffset() { return m_spOffset; }
  SSATmp* getSp() { return m_spValue; }
private:
  friend class Simplifier;
  LabelInstruction* getLabel(Trace* trace) {
    return trace ? trace->getLabel() : NULL;
  }
  SSATmp* genInstruction(Opcode,
                         Type::Tag,
                         SSATmp* src1,
                         SSATmp* src2,
                         SSATmp* src3,
                         Trace* exit = NULL);
  SSATmp* genInstruction(Opcode,
                         Type::Tag,
                         SSATmp* src1,
                         SSATmp* src2,
                         SSATmp* src3,
                         SSATmp* src4);
  SSATmp* genInstruction(Opcode,
                         Type::Tag,
                         SSATmp* src1,
                         SSATmp* src2,
                         SSATmp* src3,
                         SSATmp* src4,
                         SSATmp* src5);
  SSATmp* genInstruction(Opcode, Type::Tag, Trace* exit = NULL);
  SSATmp* genInstruction(Opcode, Type::Tag,
                         SSATmp* src,
                         Trace* exit = NULL);
  SSATmp* genInstruction(Opcode, Type::Tag,
                         SSATmp* src1,
                         SSATmp* src2,
                         Trace* exit = NULL);
  SSATmp* genInstruction(Opcode, Type::Tag,
                         SSATmp* src1,
                         SSATmp* src2,
                         uint32 nOpnds,
                         SSATmp** opnds,
                         Trace* exit = NULL);
  Trace*  genJmpCond(Opcode opc, SSATmp* src1, SSATmp* src2, Trace* target);
  Trace*  genJmpCond(Opcode opc, SSATmp* src, Trace* target);
  Trace*  genJmpCond(Opcode opc, Type::Tag, SSATmp* src, Trace* target);

  SSATmp* genLogicalOp(Opcode opc, SSATmp* src1, SSATmp* src2);
  Trace* makeTrace(uint32 bcOff, bool isMain) {
    return new Trace(m_irFactory.defLabel(), bcOff, isMain);
  }
  void genStLocAux(uint32 id, SSATmp* t0, bool genStoreType);
  /*
   * Fields
   */
  CSEHash    m_cseHash;
  IRFactory& m_irFactory;
  CSEHash&   m_constTable;
  Simplifier m_simplifier;
  SSATmp*    m_spValue;      // current physical sp
  SSATmp*    m_fpValue;      // current physical fp
  int32      m_spOffset;     // offset of physical sp from physical fp
  // Pointer to function being compiled.
  SSATmp*    m_curFunc;

  // Trace pointers; lifetime is owned by m_trace.
  Trace*     m_exitGuardFailureTrace;

  bool       m_thisIsAvailable;
  uint32     m_initialBcOff;
  boost::scoped_ptr<Trace> const m_trace;

  std::vector<SSATmp*>   m_localValues;
  std::vector<Type::Tag> m_localTypes;
};

}}} // namespace HPHP::VM::JIT

#endif //  _TRACEBUILDER_H_
