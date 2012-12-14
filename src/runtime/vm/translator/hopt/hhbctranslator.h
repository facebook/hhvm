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

#ifndef _HHBCTRANSLATOR_H_
#define _HHBCTRANSLATOR_H_

#include <assert.h>
#include <vector>
#include <stack>
#include "tracebuilder.h"
#include "runtime/vm/translator/runtime-type.h"

using namespace HPHP::VM::Transl;

namespace HPHP {
// forward declaration
class StringData;
namespace VM {
namespace JIT {

class EvalStack {
public:
  EvalStack() : vector() {}
  void push(SSATmp* tmp) {
    vector.push_back(tmp);
  }
  SSATmp* pop() {
    if (vector.size() == 0) {
      return NULL;
    }
    SSATmp* tmp = vector.back();
    vector.pop_back();
    return tmp;
  }
  SSATmp* top(uint32 offset=0) {
    if (offset >= vector.size()) {
      return NULL;
    }
    uint32 index = vector.size() - 1 - offset;
    return vector[index];
  }
  void replace(uint32 offset, SSATmp* tmp) {
    ASSERT(offset < vector.size());
    uint32 index = vector.size() - 1 - offset;
    vector[index] = tmp;
  }
  uint32 numElems() {
    return vector.size();
  }
private:
  std::vector<SSATmp*> vector;
};

class FpiStack {
public:
  void push(SSATmp* tmp) {
    stack.push(tmp);
  }
  SSATmp* pop() {
    if (stack.empty()) {
      return NULL;
    }
    SSATmp* tmp = stack.top();
    stack.pop();
    return tmp;
  }
private:
  std::stack<SSATmp*> stack;
};

class TypeGuard {
 public:
  enum Kind {
    Local,
    Stack
  };

  TypeGuard(Kind kind, uint32 index, Type::Tag type)
      : m_kind(kind)
      , m_index(index)
      , m_type(type) {
  }

  Kind      getKind()  const { return m_kind;  }
  uint32    getIndex() const { return m_index; }
  Type::Tag getType()  const { return m_type;  }

 private:
  Kind      m_kind;
  uint32    m_index;
  Type::Tag m_type;
};

class HhbcTranslator {
public:
  HhbcTranslator(TraceBuilder& builder, const Func* func)
      : m_tb(builder),
        m_curFunc(func),
        m_bcOff(0xffffffff),
        m_firstBcOff(true),
        m_lastBcOff(false),
        m_hasRet(false),
        m_unboxPtrs(true),
        m_stackDeficit(0) {
  }
  void end(int nextBcOff);
  void start(uint32 initialBcOffset, uint32 initialSpOffsetFromFp) {
    m_tb.start(initialBcOffset, initialSpOffsetFromFp);
  }

  void setBcOff(uint32 newOff, bool lastBcOff);
  void setBcOffNextTrace(uint32 bcOff) { m_bcOffNextTrace = bcOff; }
  uint32 getBcOffNextTrace() { return m_bcOffNextTrace; }
  void emitUninitLoc(uint32 id);
  void emitPrint();
  void emitThis();
  void emitCheckThis();
  void emitBareThis(int notice);
  void emitInitThisLoc(int32 id);
  void emitArray(int arrayId);
  void emitNewArray(int capacity);
  void emitNewTuple(int n);

  void emitArrayAdd();
  void emitAddElemC();
  void emitAddNewElemC();
  void emitDefCns(uint32 id);
  void emitCns(uint32 id);
  void emitConcat();
  void emitDefCls(int id, Offset after);
  void emitDefFunc(int id);

  void emitSelf();
  void emitParent();

  void emitString(int strId);
  void emitInt(int64 val);
  void emitDouble(double val);
  void emitNull();
  void emitTrue();
  void emitFalse();
  void emitCGetL(int32 id);
  void emitCGetL2(int32 id);
  void emitCGetS(const Class* cls, const StringData* propName,
                 Type::Tag resultType, bool isInferedType);
  void emitCGetProp(LocationCode locCode,
                    int propOffset,
                    bool isPropOnStack,
                    Type::Tag resultType,
                    bool isInferedType);
  void emitVGetL(int32 id);
  void emitCGetG(const StringData* name, Type::Tag resultType,
                 bool isInferedType);
  void emitVGetG(const StringData* name);
  void emitVGetS(); // TODO (tx64 doesn't have a trans for this)
  void emitVGetM(); // (tx64 doesn't have a trans for this);
  void emitSetL(int32 id);
  void emitSetS(const Class* cls, const StringData* propName);
  void emitSetG();
  void emitSetProp(int propOffset, bool isPropOnStack); // object + offset
  void emitBindL(int32 id);
  void emitBindS(); // TODO (tx64 doesn't handle this?)
  void emitBindM(const char* vectorDesc); // TODO (tx64 doesn't handle)
  void emitUnsetL(int32 id);
  void emitUnsetN();
  void emitUnsetG();
  void emitUnsetProp(int offset); // TODO
  void emitIssetL(int32 id);
  void emitIssetS();
  void emitIssetM(const char* vectorDesc /*<H E> */ ); // TODO
  void emitIssetProp(int offset); // TODO
  void emitEmptyL(int32 id);
  void emitEmptyS();
  void emitEmptyM(const char* vectorDesc); // TODO
  void emitEmptyProp(int offset); // TODO
  // The subOpc param can be one of either
  // Add, Sub, Mul, Div, Mod, Shl, Shr, Concat, BitAnd, BitOr, BitXor
  void emitSetOpL(Opcode subOpc, uint32 id); // TODO
  void emitSetOpS(Opcode subOpc); // TODO
  void emitSetOpM(Opcode subOpc, const char* vectorDesc); // TODO
  void emitSetOpProp(Opcode subOpc, int offset); // TODO
  // the pre & inc params encode the 4 possible sub opcodes:
  // PreInc, PostInc, PreDec, PostDec
  void emitIncDecL(bool pre, bool inc, uint32 id);
  void emitIncDecS(bool pre, bool inc);
  void emitIncDecProp(bool pre, bool inc, int offset, bool isPropOnStack);
  void emitPopC();
  void emitPopV();
  void emitPopR();
  void emitDup();
  void emitUnboxR();
  Trace* emitJmpZ(int32 offset);
  Trace* emitJmpNZ(int32 offset);
  Trace* emitJmp(int32 offset, bool breakTracelet);
  void emitGt()    { emitCmp(OpGt);    }
  void emitGte()   { emitCmp(OpGte);   }
  void emitLt()    { emitCmp(OpLt);    }
  void emitLte()   { emitCmp(OpLte);   }
  void emitEq()    { emitCmp(OpEq);    }
  void emitNeq()   { emitCmp(OpNeq);   }
  void emitSame()  { emitCmp(OpSame);  }
  void emitNSame() { emitCmp(OpNSame); }
  void emitFPassCOp();
  void emitFPassR();
  void emitFPushFuncD(int32 numParams, int32 funcId);
  void emitFPushFunc(int32 numParams);
  void emitFPushClsMethodD(int32 numParams,
                           int32 methodNameStrId,
                           int32 clssNamedEntityPairId,
                           bool mightNotBeStatic);
  void emitFPushObjMethodD(int32 numParams,
                           int32 methodNameStrId,
                           const Class* baseClass);
  void emitFPushCtorD(int32 numParams, int32 classNameStrId);
  void emitFPushCtor(int32 numParams);
  void emitFCall(uint32 numParams, uint32 returnBcOffset );
  void emitFCallD(uint32 numParams, const Func* callee,
                  uint32 returnBcOffset);
  void emitClsCnsD(int32 cnsNameStrId, int32 clsNameStrId);
  void emitClsCns(int32 cnsNameStrId); // TODO
  void emitAGetC(const StringData* clsName);
  void emitAGetL(int localId, const StringData* clsName);
  void emitIsNullL(int id);
  void emitIsNullC();
  void emitIsArrayL(int id);
  void emitIsArrayC();
  void emitIsStringL(int id);
  void emitIsStringC();
  void emitIsObjectL(int id);
  void emitIsObjectC();
  void emitIsIntL(int id);
  void emitIsIntC();
  void emitIsBoolL(int id);
  void emitIsBoolC();
  void emitIsDoubleL(int id);
  void emitIsDoubleC();
  void emitVerifyParamType(int32 paramId,
                           const StringData* constraintClsName);
  void emitInstanceOfD(int classNameStrId);
  void emitNop() {}
  void emitCastBool();
  void emitCastInt();
  void emitCastDouble();
  void emitCastString();
  void emitCastArray();
  void emitCastObject();

  void emitRetC(bool freeInline);
  void emitRetV(bool freeInline);

  void emitLateBoundCls(); // TODO: if this, then LdObjCls

  // binary arithmetic ops
  void emitAdd();
  void emitSub();
  void emitBitAnd();
  void emitBitOr();
  void emitBitXor();
  void emitBitNot();
  void emitMul();

  // boolean ops
  void emitXor();
  void emitNot();
  void emitNativeImpl();

  void emitClassExists(const StringData* clsName);
  void emitInterfaceExists(const StringData* ifaceName);
  void emitTraitExists(const StringData* traitName);

  void emitStaticLocInit(uint32 varId, uint32 listStrId);
  void emitReqDoc(const StringData* name);
  void emitReqMod(const StringData* name);
  void emitReqSrc(const StringData* name);

  // iterators
  void emitIterInit(uint32 iterVarId, int targetOffset);
  void emitIterInitK(uint32 iterVarId, int targetOffset);
  void emitIterNext(uint32 iterVarId, int targetOffset);
  void emitIterNextK(uint32 iterVarId, int targetOffset);

  void emitVerifyParamType(uint32 paramId);

  // continuations
  SSATmp* getContLocals(SSATmp* cont);
  void emitCreateCont(bool getArgs, Id funNameStrId);
  void emitContExit();
  void emitUnpackCont();
  void emitPackCont(int64 labelId);
  void emitContReceive();
  void emitContRaised();
  void emitContDone();
  void emitContNext();
  void emitContSendImpl(bool raise);
  void emitContSend();
  void emitContRaise();
  void emitContValid();
  void emitContCurrent();
  void emitContStopped();
  void emitContHandle();

  void emitStrlen();
  void emitIncStat(int32 counter, int32 value);

  // tracelet guards
  Trace* guardTypeStack(uint32 stackIndex,
                        Type::Tag type,
                        Trace* nextTrace = NULL);

  Trace* guardTypeLocal(uint32 localIndex,
                        Type::Tag type,
                        Trace* nextTrace = NULL);

  Trace* guardRefs(int64               entryArDelta,
                   const vector<bool>& mask,
                   const vector<bool>& vals,
                   Trace*              exitTrace = NULL);

  void assertTypeLocal(uint32 localIndex, Type::Tag type);

  void assertTypeStack(uint32 stackIndex, Type::Tag type);

  void checkTypeLocal(uint32 localIndex, Type::Tag type);

  void checkTypeStack(uint32 stackIndex, Type::Tag type, Offset nextByteCode);

  void setThisAvailable();

  void emitLoadDeps();

private:

  /*
   * Emit helpers.
   */
  void emitUnboxRAux();
  void emitAGet(SSATmp* src);
  void emitFCallAux(uint32 numParams,
                    uint32 returnBcOffset,
                    const Func* callee);
  void emitRet(SSATmp* retVal, Trace* exitTrace, bool freeInline);
  template<Type::Tag T> void emitIsTypeC();
  template<Type::Tag T> void emitIsTypeL(int id);
  void emitCmp(Opcode opc);
  Trace* emitJmpCondHelper(int32 offset, bool negate);
  SSATmp* emitIncDec(bool pre, bool inc, SSATmp* src);
  void emitIncDecMem(bool pre, bool inc, SSATmp* propAddr, Trace* exitTrace);
  SSATmp* getMemberAddr(const char* vectorDesc, Trace* exitTrace);
  SSATmp* getClsPropAddr(const Class* cls, const StringData* propName = NULL);
  void   decRefPropAddr(SSATmp* propAddr);
  Trace* getExitTrace(uint32 targetBcOff);
  Trace* getExitTrace(uint32 targetBcOff, uint32 notTakenBcOff);
  Trace* getExitSlowTrace(Offset nextByteCode = -1);
  Trace* getGuardExit();
  SSATmp* emitLdLocWarn(uint32 id, Type::Tag type, Trace* target);
  void emitInterpOne(Type::Tag type, Trace* target = NULL);
  void emitInterpOneOrPunt(Type::Tag type, Trace* target = NULL);
  void emitBinaryArith(Opcode);
  void checkTypeStackAux(uint32 stackIndex, Type::Tag type, Trace* nextTrace);
  void loadStack(uint32 stackIndex, Type::Tag type);


  /*
   * Accessors for the current function being compiled and its
   * class and unit.
   */
  const Func* getCurFunc()  { return m_curFunc; }
  Class*      getCurClass() { return getCurFunc()->cls(); }
  Unit*       getCurUnit()  { return getCurFunc()->unit(); }
  /*
   * Helpers for resolving bytecode immediate ids.
   */
  ArrayData*  lookupArrayId(int arrId);
  StringData* lookupStringId(int strId);
  Func*       lookupFuncId(int funcId);
  PreClass*   lookupPreClassId(int preClassId);
  const NamedEntityPair& lookupNamedEntityPairId(int id);

  /*
   * Eval stack helpers
   */
  SSATmp* push(SSATmp* tmp);
  SSATmp* pushIncRef(SSATmp* tmp) { return push(m_tb.genIncRef(tmp)); }
  SSATmp* pop(Type::Tag type, Trace* exitTrace);
  void    popDecRef(Type::Tag type, Trace* exitTrace);
  SSATmp* pop()  { return pop(Type::Gen, NULL);       }
  SSATmp* popC() { return pop(Type::Cell, NULL);      }
  SSATmp* popV() { return pop(Type::BoxedCell, NULL); }
  SSATmp* popR() { return pop(Type::Gen, NULL);       }
  SSATmp* popA() { return pop(Type::ClassRef, NULL);  }
  SSATmp* popF() { return pop(Type::Gen, NULL);       }
  SSATmp* topC(uint32 i = 0) { return top(Type::Cell, i); }
  SSATmp* spillStack(bool allocActRec = false);
  SSATmp* loadStackAddr(int32 offset);
  SSATmp* top(Type::Tag type, uint32 index = 0);
  void    extendStack(uint32 index,
                      Type::Tag type = Type::Gen,
                      Trace* exitTrace = NULL);
  void    replace(uint32 index, SSATmp* tmp);
  void    refineType(SSATmp* tmp, Type::Tag type);
  /*
   * Fields
   */
  TraceBuilder&     m_tb;
  const Func*       m_curFunc;
  uint32            m_bcOff;
  uint32            m_bcOffNextTrace;
  bool              m_firstBcOff;
  bool              m_lastBcOff;
  bool              m_hasRet;
  // if set, then generate unbox instructions for memory accesses (Get
  // and Set bytecodes). Otherwise, memory accesses will bail the trace
  // on an access to a boxed value.
  bool              m_unboxPtrs;
  uint32            m_stackDeficit; // offset of virtual sp from physical sp
  EvalStack         m_evalStack;
  FpiStack          m_fpiStack;
  vector<TypeGuard> m_typeGuards;
};

}}} // namespace HPHP::VM::JIT

#endif // _HHBCTRANSLATOR_H_
