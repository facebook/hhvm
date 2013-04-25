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

#ifndef incl_HPHP_RUNTIME_VM_TRANSLATOR_HOPT_HHBCTRANSLATOR_H_
#define incl_HPHP_RUNTIME_VM_TRANSLATOR_HOPT_HHBCTRANSLATOR_H_

#include <vector>
#include <memory>
#include <stack>

#include "util/assertions.h"
#include "runtime/vm/bytecode.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/translator/runtime-type.h"
#include "runtime/vm/translator/hopt/tracebuilder.h"

using HPHP::VM::Transl::SrcKey;
using HPHP::VM::Transl::NormalizedInstruction;

namespace HPHP {
namespace VM {
namespace Transl { struct PropInfo; }
namespace JIT {

struct EvalStack {
  void push(SSATmp* tmp) {
    m_vector.push_back(tmp);
  }

  SSATmp* pop() {
    if (m_vector.size() == 0) {
      return nullptr;
    }
    SSATmp* tmp = m_vector.back();
    m_vector.pop_back();
    return tmp;
  }

  SSATmp* top(uint32_t offset=0) const {
    if (offset >= m_vector.size()) {
      return nullptr;
    }
    uint32_t index = m_vector.size() - 1 - offset;
    return m_vector[index];
  }

  void replace(uint32_t offset, SSATmp* tmp) {
    assert(offset < m_vector.size());
    uint32_t index = m_vector.size() - 1 - offset;
    m_vector[index] = tmp;
  }

  uint32_t numCells() const {
    uint32_t ret = 0;
    for (auto& t : m_vector) {
      ret += t->type() == Type::ActRec ? kNumActRecCells : 1;
    }
    return ret;
  }

  int  size()  const { return m_vector.size(); }
  void clear()       { m_vector.clear(); }

private:
  std::vector<SSATmp*> m_vector;
};

class TypeGuard {
 public:
  enum Kind {
    Local,
    Stack,
    Iter
  };

  TypeGuard(Kind kind, uint32_t index, Type type)
    : m_kind(kind)
    , m_index(index)
    , m_type(type)
  {}

  Kind      getKind()  const { return m_kind;  }
  uint32_t  getIndex() const { return m_index; }
  Type      type()  const { return m_type;  }

 private:
  Kind      m_kind;
  uint32_t  m_index;
  Type      m_type;
};

struct HhbcTranslator {
  HhbcTranslator(IRFactory& irFactory,
                 Offset bcStartOffset,
                 uint32_t initialSpOffsetFromFp,
                 const Func* func)
    : m_irFactory(irFactory)
    , m_tb(new TraceBuilder(bcStartOffset,
                            initialSpOffsetFromFp,
                            m_irFactory,
                            func))
    , m_bcStateStack {BcState(-1, func)}
    , m_startBcOff(bcStartOffset)
    , m_lastBcOff(false)
    , m_hasExit(false)
    , m_stackDeficit(0)
    , m_exitGuardFailureTrace(m_tb->genExitGuardFailure(bcStartOffset))
  {}

  void end(int nextBcOff);
  Trace* getTrace() const { return m_tb->getTrace(); }
  TraceBuilder* getTraceBuilder() const { return m_tb.get(); }

  void beginInlining(unsigned numArgs,
                     const Func* target,
                     Offset returnBcOffset);
  void endInlining();
  bool isInlining() const;
  void profileFunctionEntry(const char* category);
  void profileInlineFunctionShape(const std::string& str);
  void profileSmallFunctionShape(const std::string& str);
  void profileFailedInlShape(const std::string& str);

  void setBcOff(Offset newOff, bool lastBcOff);
  void setBcOffNextTrace(Offset bcOff) { m_bcOffNextTrace = bcOff; }
  uint32_t getBcOffNextTrace() { return m_bcOffNextTrace; }

  void emitPrint();
  void emitThis();
  void emitCheckThis();
  void emitBareThis(int notice);
  void emitInitThisLoc(int32_t id);
  void emitArray(int arrayId);
  void emitNewArray(int capacity);
  void emitNewTuple(int n);

  void emitArrayAdd();
  void emitAddElemC();
  void emitAddNewElemC();
  void emitNewCol(int type, int numElems);
  void emitColAddElemC();
  void emitColAddNewElemC();
  void emitDefCns(uint32_t id);
  void emitCns(uint32_t id);
  void emitConcat();
  void emitDefCls(int id, Offset after);
  void emitDefFunc(int id);

  void emitLateBoundCls();
  void emitSelf();
  void emitParent();

  void emitString(int strId);
  void emitInt(int64_t val);
  void emitDouble(double val);
  void emitNullUninit();
  void emitNull();
  void emitTrue();
  void emitFalse();
  void emitCGetL(int32_t id);
  void emitCGetL2(int32_t id);
  void emitCGetS(const StringData* propName,
                 Type resultType, bool isInferedType);
  void emitCGetG(const StringData* name, Type resultType,
                 bool isInferedType);
  void emitMInstr(const NormalizedInstruction& ni);
  void emitVGetL(int32_t id);
  void emitVGetS(const StringData* propName);
  void emitVGetG(const StringData* name);
  void emitSetL(int32_t id);
  void emitSetS(const StringData* propName);
  void emitSetG(const StringData* gblName);
  void emitBindL(int32_t id);
  void emitBindS(const StringData* propName);
  void emitBindG(const StringData* gblName);
  void emitUnsetL(int32_t id);
  void emitUnsetN();
  void emitUnsetG(const StringData* gblName);
  void emitIssetL(int32_t id);
  void emitIssetS(const StringData* propName);
  void emitIssetG(const StringData* gblName);
  void emitEmptyL(int32_t id);
  void emitEmptyS(const StringData* propName);
  void emitEmptyG(const StringData* gblName);
  // The subOpc param can be one of either
  // Add, Sub, Mul, Div, Mod, Shl, Shr, Concat, BitAnd, BitOr, BitXor
  void emitSetOpL(Opcode subOpc, uint32_t id);
  void emitSetOpS(Opcode subOpc);
  // the pre & inc params encode the 4 possible sub opcodes:
  // PreInc, PostInc, PreDec, PostDec
  void emitIncDecL(bool pre, bool inc, uint32_t id);
  void emitIncDecS(bool pre, bool inc);
  void emitPopC();
  void emitPopV();
  void emitPopR();
  void emitDup();
  void emitUnboxR();
  void emitJmpZ(int32_t offset);
  void emitJmpNZ(int32_t offset);
  void emitJmp(int32_t offset, bool breakTracelet, bool noSurprise);
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
  void emitFPassV();
  void emitFPushCufOp(VM::Op op, Class* cls, StringData* invName,
                      const Func* func, int numArgs);
  void emitFPushActRec(SSATmp* func, SSATmp* objOrClass, int32_t numArgs,
                       const StringData* invName);
  void emitFPushFuncD(int32_t numParams, int32_t funcId);
  void emitFPushFunc(int32_t numParams);
  void emitFPushFunc(int32_t numParams, SSATmp* funcName);
  SSATmp* getClsMethodCtx(const Func* callee, const Class* cls);
  void emitFPushClsMethodD(int32_t numParams,
                           int32_t methodNameStrId,
                           int32_t clssNamedEntityPairId);
  void emitFPushObjMethodD(int32_t numParams,
                           int32_t methodNameStrId,
                           const Class* baseClass);
  void emitFPushClsMethodF(int32_t             numParams,
                           const Class*      cls,
                           const StringData* methName);
  void emitFPushCtorD(int32_t numParams, int32_t classNameStrId);
  void emitFPushCtor(int32_t numParams);
  void emitCreateCl(int32_t numParams, int32_t classNameStrId);
  void emitFCallArray();
  void emitFCall(uint32_t numParams,
                  Offset returnBcOffset,
                  const Func* callee);
  void emitFCallBuiltin(uint32_t numArgs, uint32_t numNonDefault,
                        int32_t funcId);
  void emitClsCnsD(int32_t cnsNameStrId, int32_t clsNameStrId);
  void emitClsCns(int32_t cnsNameStrId);
  void emitAKExists();
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
  void emitVerifyParamType(int32_t paramId);
  void emitInstanceOfD(int classNameStrId);
  void emitNop() {}
  void emitCastBool();
  void emitCastInt();
  void emitCastDouble();
  void emitCastString();
  void emitCastArray();
  void emitCastObject();

  void emitSwitch(const ImmVector&, int64_t base, bool bounded);
  void emitSSwitch(const ImmVector&);

  // freeInline indicates whether we should be doing decrefs inlined in
  // the TC, or using the generic decref helper.
  void emitRetC(bool freeInline);
  void emitRetV(bool freeInline);

  // binary arithmetic ops
  void emitAdd();
  void emitSub();
  void emitBitAnd();
  void emitBitOr();
  void emitBitXor();
  void emitBitNot();
  void emitMul();
  void emitMod();
  void emitDiv();

  // boolean ops
  void emitXor();
  void emitNot();
  void emitNativeImpl();

  void emitClassExists(const StringData* clsName);
  void emitInterfaceExists(const StringData* ifaceName);
  void emitTraitExists(const StringData* traitName);

  void emitStaticLocInit(uint32_t varId, uint32_t listStrId);
  void emitReqDoc(const StringData* name);

  // iterators
  void emitIterInit(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitIterInitK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId);
  void emitIterNext(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitIterNextK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId);

  void emitIterFree(uint32_t iterId);
  void emitVerifyParamType(uint32_t paramId);

  // continuations
  void emitCreateCont(bool getArgs, Id funNameStrId);
  void emitContEnter(int32_t returnBcOffset);
  void emitContExitImpl();
  void emitContExit();
  void emitUnpackCont();
  void emitPackCont(int64_t labelId);
  void emitContReceive();
  void emitContRetC();
  void emitContNext();
  void emitContSendImpl(bool raise);
  void emitContSend();
  void emitContRaise();
  void emitContValid();
  void emitContCurrent();
  void emitContStopped();
  void emitContHandle();

  void emitStrlen();
  void emitIncStat(int32_t counter, int32_t value, bool force = false);
  void emitIncTransCounter();

  template<typename T>
  SSATmp* cns(T val) {
    return m_tb->genDefConst(val);
  }

  template<class... Args>
  SSATmp* gen(Args&&... args) {
    return m_tb->gen(std::forward<Args>(args)...);
  }

  // tracelet guards
  Trace* guardTypeStack(uint32_t stackIndex,
                        Type type,
                        Trace* nextTrace = nullptr);
  void   guardTypeLocal(uint32_t locId, Type type);
  Trace* guardRefs(int64_t               entryArDelta,
                   const vector<bool>& mask,
                   const vector<bool>& vals,
                   Trace*              exitTrace = nullptr);

  // Interface to irtranslator for predicted and inferred types.
  void assertTypeLocal(uint32_t localIndex, Type type);
  void assertTypeStack(uint32_t stackIndex, Type type);
  void checkTypeLocal(uint32_t localIndex, Type type);
  void checkTypeTopOfStack(Type type, Offset nextByteCode);
  void overrideTypeLocal(uint32_t localIndex, Type type);
  void setThisAvailable();
  void emitLoadDeps();
  void emitInterpOne(Type type, int numPopped, int numExtraPushed = 0);
  void emitInterpOneCF(int numPopped);

private:
  /*
   * VectorTranslator is responsible for translating one of the vector
   * instructions (CGetM, SetM, IssetM, etc..) into hhir.
   */
  class VectorTranslator {
   public:
    VectorTranslator(const NormalizedInstruction& ni,
                     HhbcTranslator& ht);
    void emit();

   private:
    void checkMIState();
    void emitMPre();
    void emitFinalMOp();
    void emitMPost();
    void emitMTrace();

    // Bases
    void emitBaseOp();
    void emitBaseLCR();
    void emitBaseH();
    void emitBaseG();
    void emitBaseN();
    void emitBaseS();

    // Intermediate Operations
    void emitIntermediateOp();
    void emitProp();
    void emitPropGeneric();
    void emitPropSpecialized(const MInstrAttr mia, Transl::PropInfo propInfo);
    void emitElem();
    void emitNewElem();
    void emitRatchetRefs();

    // Final Operations
#   define MII(instr, ...)                      \
    void emit##instr##Elem();                   \
    void emit##instr##Prop();
    MINSTRS
#   undef MII
    void emitIssetEmptyElem(bool isEmpty);
    void emitIssetEmptyProp(bool isEmpty);
    void emitNotSuppNewElem();
    void emitVGetNewElem();
    void emitSetNewElem();
    void emitSetOpNewElem();
    void emitIncDecNewElem();
    void emitBindNewElem();
    void emitArraySet(SSATmp* key, SSATmp* value);
    void emitArrayGet(SSATmp* key);
    void emitArrayIsset();
    void checkStrictlyInteger(SSATmp*& key, KeyType& keyType,
                              bool& checkForInt);

    // Misc Helpers
    void numberStackInputs();
    void setNoMIState() { m_needMIS = false; }
    SSATmp* genMisPtr();
    SSATmp* getInput(unsigned i);
    SSATmp* getBase();
    SSATmp* getKey();
    SSATmp* getValue();
    SSATmp* checkInitProp(SSATmp* baseAsObj,
                          SSATmp* propAddr,
                          Transl::PropInfo propOffset,
                          bool warn,
                          bool define);
    Class* contextClass() const;

    /*
     * genStk is a wrapper around TraceBuilder::gen() to deal with instructions
     * that may modify the stack. It inspects the opcode and the types of the
     * inputs, replacing the opcode with the version that returns a new StkPtr
     * if appropriate.
     */
    template<typename... Srcs>
    SSATmp* genStk(Opcode op, Srcs... srcs);

    /* Various predicates about the current instruction */
    bool isSimpleArrayOp();
    bool isSimpleBase();
    bool isSingleMember();

    bool generateMVal() const;
    bool needFirstRatchet() const;
    bool needFinalRatchet() const;
    unsigned nLogicalRatchets() const;
    int ratchetInd() const;

    template<typename T>
    SSATmp* cns(T val) {
      return m_tb.genDefConst(val);
    }

    template<class... Args>
    SSATmp* gen(Args&&... args) {
      return m_tb.gen(std::forward<Args>(args)...);
    }

    const Transl::NormalizedInstruction& m_ni;
    HhbcTranslator& m_ht;
    TraceBuilder& m_tb;
    const MInstrInfo& m_mii;
    hphp_hash_map<unsigned, unsigned> m_stackInputs;

    unsigned m_mInd;
    unsigned m_iInd;

    bool m_needMIS;

    SSATmp* m_misBase;
    SSATmp* m_base;
    SSATmp* m_result;
  };

  /*
   * Emit helpers.
   */
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitCGet(const StringData* name,
                Type resultType,
                bool isInferedType,
                bool exitOnFailure,
                CheckSupportedFun checkSupported,
                EmitLdAddrFun emitLdAddr);
  void emitVGetMem(SSATmp* addr);
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitVGet(const StringData* name, CheckSupportedFun, EmitLdAddrFun);
  void emitBindMem(SSATmp* ptr, SSATmp* src);
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitBind(const StringData* name, CheckSupportedFun, EmitLdAddrFun);
  void emitSetMem(SSATmp* ptr, SSATmp* src);
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitSet(const StringData* name, CheckSupportedFun, EmitLdAddrFun);
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitIsset(const StringData* name, CheckSupportedFun, EmitLdAddrFun);
  void emitEmptyMem(SSATmp* ptr);
  template<class CheckSupportedFun, class EmitLdAddrFun>
  void emitEmpty(const StringData* name,
                 CheckSupportedFun checkSupported,
                 EmitLdAddrFun emitLdAddr);
  void emitIncDecMem(bool pre, bool inc, SSATmp* ptr, Trace* exitTrace);
  bool checkSupportedClsProp(const StringData* propName,
                             Type resultType,
                             int stkIndex);
  bool checkSupportedGblName(const StringData* gblName,
                             HPHP::VM::JIT::Type resultType,
                             int stkIndex);
  SSATmp* emitLdClsPropAddrOrExit(const StringData* propName, Block* block);
  SSATmp* emitLdClsPropAddr(const StringData* propName) {
    return emitLdClsPropAddrOrExit(propName, nullptr);
  }
  SSATmp* getStrName(const StringData* propName = nullptr);
  SSATmp* emitLdGblAddrDef(const StringData* gblName = nullptr);
  SSATmp* emitLdGblAddr(const StringData* gblName, Block* block);
  void emitUnboxRAux();
  void emitAGet(SSATmp* src, const StringData* clsName);
  void emitRetFromInlined(Type type);
  SSATmp* emitDecRefLocalsInline(SSATmp* retVal);
  void emitRet(Type type, bool freeInline);
  void emitIsTypeC(Type t);
  void emitIsTypeL(Type t, int id);
  void emitCmp(Opcode opc);
  SSATmp* emitJmpCondHelper(int32_t offset, bool negate, SSATmp* src);
  SSATmp* emitIncDec(bool pre, bool inc, SSATmp* src);
  SSATmp* getMemberAddr(const char* vectorDesc, Trace* exitTrace);
  Trace* getExitTrace(Offset targetBcOff = -1);
  Trace* getExitTrace(uint32_t targetBcOff, uint32_t notTakenBcOff);
  Trace* getExitSlowTrace();
  Trace* getGuardExit();
  SSATmp* emitLdLocWarn(uint32_t id, Trace* target);
  void emitInterpOneOrPunt(Type type, int numPopped, int numExtraPushed = 0);
  void emitBinaryArith(Opcode);
  template<class Lambda>
  SSATmp* emitIterInitCommon(int offset, Lambda genFunc);
  void emitMarker();

  /*
   * Accessors for the current function being compiled and its
   * class and unit.
   */
  const Func* getCurFunc()  const { return m_bcStateStack.back().func; }
  Class*      getCurClass() const { return getCurFunc()->cls(); }
  Unit*       getCurUnit()  const { return getCurFunc()->unit(); }
  Offset      bcOff()       const { return m_bcStateStack.back().bcOff; }

  SrcKey      getCurSrcKey()  const { return SrcKey(getCurFunc(), bcOff()); }
  SrcKey      getNextSrcKey() const {
    SrcKey srcKey(getCurFunc(), bcOff());
    srcKey.advance(getCurFunc()->unit());
    return srcKey;
  }

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
  SSATmp* pushIncRef(SSATmp* tmp) { return push(m_tb->gen(IncRef, tmp)); }
  SSATmp* pop(Type type);
  void    popDecRef(Type type);
  void    discard(unsigned n);
  SSATmp* popC() { return pop(Type::Cell);      }
  SSATmp* popV() { return pop(Type::BoxedCell); }
  SSATmp* popR() { return pop(Type::Gen);       }
  SSATmp* popA() { return pop(Type::Cls);       }
  SSATmp* popF() { return pop(Type::Gen);       }
  SSATmp* topC(uint32_t i = 0) { return top(Type::Cell, i); }
  std::vector<SSATmp*> getSpillValues() const;
  SSATmp* spillStack();
  void    exceptionBarrier();
  SSATmp* loadStackAddr(int32_t offset);
  SSATmp* top(Type type, uint32_t index = 0);
  void    extendStack(uint32_t index, Type type);
  void    replace(uint32_t index, SSATmp* tmp);
  void    refineType(SSATmp* tmp, Type type);

private:
  // Tracks information about the current bytecode offset and which
  // function we are in.  Goes in m_bcStateStack; we push and pop as
  // we deal with inlined calls.
  struct BcState {
    explicit BcState(Offset bcOff, const Func* func)
      : bcOff(bcOff)
      , func(func)
    {}

    Offset bcOff;
    const Func* func;
  };

private:
  IRFactory&        m_irFactory;
  std::unique_ptr<TraceBuilder>
                    m_tb;
  std::vector<BcState>
                    m_bcStateStack;
  Offset            m_startBcOff;
  Offset            m_bcOffNextTrace;
  bool              m_lastBcOff;
  bool              m_hasExit;

  /*
   * Tracking of the state of the virtual execution stack:
   *
   *   During HhbcTranslator's run over the bytecode, these stacks
   *   contain SSATmp values representing the execution stack state
   *   since the last SpillStack.
   *
   *   The EvalStack contains cells and ActRecs that need to be
   *   spilled in order to materialize the stack.
   *
   *   m_stackDeficit represents the number of cells we've popped off
   *   the virtual stack since the last sync.
   */
  uint32_t          m_stackDeficit;
  EvalStack         m_evalStack;

  /*
   * The FPI stack is used for inlining---when we start inlining at an
   * FCall, we look in here to find a definition of the StkPtr,offset
   * that can be used after the inlined callee "returns".
   */
  std::stack<std::pair<SSATmp*,int32_t>>
                    m_fpiStack;

  vector<TypeGuard> m_typeGuards;
  Trace* const      m_exitGuardFailureTrace;
};

}}} // namespace HPHP::VM::JIT

#endif
