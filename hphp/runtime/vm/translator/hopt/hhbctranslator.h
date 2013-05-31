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

#include "hphp/util/assertions.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/member_operations.h"
#include "hphp/runtime/vm/translator/runtime-type.h"
#include "hphp/runtime/vm/translator/hopt/tracebuilder.h"

using HPHP::Transl::SrcKey;
using HPHP::Transl::NormalizedInstruction;

namespace HPHP {
namespace Transl { struct PropInfo; }
namespace JIT {

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

/*
 * This module is responsible for determining high-level HHBC->IR
 * compilation strategy.
 *
 * For each bytecode Foo in HHBC, there is a function in this class
 * called emitFoo which handles translating it into HHIR.
 *
 * Additionally, while transating bytecode, this module manages a
 * virtual execution stack modelling the state of the stack since the
 * last time we emitted an IR instruction that materialized it
 * (e.g. SpillStack or SpillFrame).
 *
 * HhbcTranslator is where we make optimiations that relate to overall
 * knowledge of the runtime and HHBC.  For example, decisions like
 * whether to use IR instructions that have constant Class*'s (for a
 * AttrUnique class) instead of loading a Class* from TargetCache are
 * made at this level.
 */
struct HhbcTranslator {
  HhbcTranslator(IRFactory& irFactory,
                 Offset startOffset,
                 Offset nextTraceOffset,
                 uint32_t initialSpOffsetFromFp,
                 const Func* func);

  // Accessors.
  Trace* trace() const { return m_tb->trace(); }
  TraceBuilder* traceBuilder() const { return m_tb.get(); }

  // In between each emit* call, irtranslator indicates the new
  // bytecode offset (or whether we're finished) using this API.
  void setBcOff(Offset newOff, bool lastBcOff);
  void end();

  // Tracelet guards.
  void guardTypeStack(uint32_t stackIndex, Type type);
  void guardTypeLocal(uint32_t locId, Type type);
  void guardRefs(int64_t entryArDelta,
                 const vector<bool>& mask,
                 const vector<bool>& vals);

  // Interface to irtranslator for predicted and inferred types.
  void assertTypeLocal(uint32_t localIndex, Type type);
  void assertTypeStack(uint32_t stackIndex, Type type);
  void checkTypeLocal(uint32_t localIndex, Type type);
  void checkTypeTopOfStack(Type type, Offset nextByteCode);
  void overrideTypeLocal(uint32_t localIndex, Type type);

  // Inlining-related functions.
  void beginInlining(unsigned numArgs,
                     const Func* target,
                     Offset returnBcOffset);
  void endInlining();
  bool isInlining() const;
  void profileFunctionEntry(const char* category);
  void profileInlineFunctionShape(const std::string& str);
  void profileSmallFunctionShape(const std::string& str);
  void profileFailedInlShape(const std::string& str);

  // Other public functions for irtranslator.
  void setThisAvailable();
  void emitInterpOne(Type type, int numPopped, int numExtraPushed = 0);
  void emitInterpOneCF(int numPopped);

  /*
   * An emit* function for each HHBC opcode.
   */

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
  void emitCnsE(uint32_t id);
  void emitCnsU(uint32_t id);
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
  void emitJmpZ(Offset taken);
  void emitJmpNZ(Offset taken);
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
  void emitFPushCufIter(int32_t numParams, int32_t itId);
  void emitFPushCufOp(Op op, Class* cls, StringData* invName,
                      const Func* func, int numArgs);
  void emitFPushActRec(SSATmp* func, SSATmp* objOrClass, int32_t numArgs,
                       const StringData* invName = nullptr);
  void emitFPushFuncD(int32_t numParams, int32_t funcId);
  void emitFPushFuncU(int32_t numParams,
                      int32_t funcId,
                      int32_t fallbackFuncId);
  void emitFPushFunc(int32_t numParams);
  void emitFPushFunc(int32_t numParams, SSATmp* funcName);
  SSATmp* genClsMethodCtx(const Func* callee, const Class* cls);
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
  void emitFPushCtorCommon(SSATmp* cls,
                           SSATmp* obj,
                           const Func* func,
                           int32_t numParams,
                           Trace* catchTrace);
  void emitCreateCl(int32_t numParams, int32_t classNameStrId);
  void emitFCallArray(const Offset pcOffset, const Offset after);
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
  void emitWIterInit(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitWIterInitK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId);
  void emitWIterNext(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitWIterNextK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId);

  void emitIterFree(uint32_t iterId);
  void emitCIterFree(uint32_t iterId);
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
  void emitArrayIdx();

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
    void emitSideExits(SSATmp* catchSp, int nStack);
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
    void emitSetWithRefNewElem();
    void emitSetOpNewElem();
    void emitIncDecNewElem();
    void emitBindNewElem();
    void emitArraySet(SSATmp* key, SSATmp* value);
    void emitArrayGet(SSATmp* key);
    void emitArrayIsset();

    // Misc Helpers
    void numberStackInputs();
    void setNoMIState() { m_needMIS = false; }
    SSATmp* genMisPtr();
    SSATmp* getInput(unsigned i);
    SSATmp* getBase();
    SSATmp* getKey();
    SSATmp* getValue();
    SSATmp* getValAddr();
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
    SSATmp* genStk(Opcode op, Trace* taken, Srcs... srcs);

    /* Various predicates about the current instruction */
    bool isSimpleArrayOp();
    bool isSimpleBase();
    bool isSingleMember();

    bool generateMVal() const;
    bool needFirstRatchet() const;
    bool needFinalRatchet() const;
    unsigned nLogicalRatchets() const;
    int ratchetInd() const;

    template<class... Args>
    SSATmp* cns(Args&&... args) {
      return m_tb.cns(std::forward<Args>(args)...);
    }

    template<class... Args>
    SSATmp* gen(Args&&... args) {
      return m_tb.gen(std::forward<Args>(args)...);
    }

    const Transl::NormalizedInstruction& m_ni;
    HhbcTranslator& m_ht;
    TraceBuilder& m_tb;
    IRFactory& m_irf;
    const MInstrInfo& m_mii;
    hphp_hash_map<unsigned, unsigned> m_stackInputs;

    unsigned m_mInd;
    unsigned m_iInd;

    bool m_needMIS;

    /* The base for any accesses to the current MInstrState. */
    SSATmp* m_misBase;

    /* The value of the base for the next member operation. Starts as the base
     * for the whole instruction and is updated as the translator makes
     * progress. */
    SSATmp* m_base;

    /* The result of the vector instruction. nullptr if the current instruction
     * doesn't produce a result. */
    SSATmp* m_result;

    /* If set, contains a value of type CountedStr|Nullptr. If a runtime test
     * determines that the value is not Nullptr, we incorrectly predicted the
     * output type of the instruction and must side exit. */
    SSATmp* m_strTestResult;

    /* If set, contains the catch trace for the final set operation of this
     * instruction. The operations that set this member may need to return an
     * unexpected type, in which case they'll throw an InvalidSetMException. To
     * handle this, emitMPost adds code to the catch trace to fish the correct
     * value out of the exception and side exit. */
    Trace*  m_failedSetTrace;
  };

private: // tracebuilder forwarding utilities
  template<class... Args>
  SSATmp* cns(Args&&... args) {
    return m_tb->cns(std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* gen(Args&&... args) {
    return m_tb->gen(std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* genFor(Trace* trace, Args&&... args) {
    return m_tb->genFor(trace, std::forward<Args>(args)...);
  }

private:
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
                             HPHP::JIT::Type resultType,
                             int stkIndex);
  void checkStrictlyInteger(SSATmp*& key, KeyType& keyType,
                            bool& checkForInt);
  SSATmp* emitLdClsPropAddrOrExit(const StringData* propName, Block* block);
  SSATmp* emitLdClsPropAddr(const StringData* propName) {
    return emitLdClsPropAddrOrExit(propName, nullptr);
  }
  SSATmp* emitLdClsPropAddrCached(const StringData* propName,
                                  Block* block);
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
  void emitBinaryArith(Opcode);
  template<class Lambda>
  SSATmp* emitIterInitCommon(int offset, Lambda genFunc);
  IRInstruction* makeMarker(Offset bcOff);
  void emitMarker();

  // Exit trace creation routines.
  Trace* getExitTrace(Offset targetBcOff = -1);
  Trace* getExitTrace(Offset targetBcOff,
                      std::vector<SSATmp*>& spillValues);
  Trace* getExitTraceWarn(Offset targetBcOff,
                          std::vector<SSATmp*>& spillValues,
                          const StringData* warning);
  Trace* getExitSlowTrace();
  Trace* getCatchTrace();

  enum class ExitFlag {
    None,
    NoIR,
  };
  Trace* getExitTraceImpl(Offset targetBcOff,
                          ExitFlag noIRExit,
                          std::vector<SSATmp*>& spillValues,
                          const StringData* warning);

  /*
   * Accessors for the current function being compiled and its
   * class and unit.
   */
  const Func* curFunc()   const { return m_bcStateStack.back().func; }
  Class*      curClass()  const { return curFunc()->cls(); }
  Unit*       curUnit()   const { return curFunc()->unit(); }
  Offset      bcOff()     const { return m_bcStateStack.back().bcOff; }
  SrcKey      curSrcKey() const { return SrcKey(curFunc(), bcOff()); }

  /*
   * Return the SrcKey for the next HHBC (whether it is in this
   * tracelet or not).
   */
  SrcKey nextSrcKey() const {
    SrcKey srcKey(curFunc(), bcOff());
    srcKey.advance(curFunc()->unit());
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
  const NamedEntity* lookupNamedEntityId(int id);

  /*
   * Eval stack helpers.
   */
  SSATmp* push(SSATmp* tmp);
  SSATmp* pushIncRef(SSATmp* tmp) { return push(gen(IncRef, tmp)); }
  SSATmp* pop(Type type);
  void    popDecRef(Type type);
  void    discard(unsigned n);
  SSATmp* popC() { return pop(Type::Cell);      }
  SSATmp* popV() { return pop(Type::BoxedCell); }
  SSATmp* popR() { return pop(Type::Gen);       }
  SSATmp* popA() { return pop(Type::Cls);       }
  SSATmp* popF() { return pop(Type::Gen);       }
  SSATmp* topC(uint32_t i = 0) { return top(Type::Cell, i); }
  SSATmp* topV(uint32_t i = 0) { return top(Type::BoxedCell, i); }
  std::vector<SSATmp*> peekSpillValues() const;
  SSATmp* emitSpillStack(Trace* t, SSATmp* sp,
                         const std::vector<SSATmp*>& spillVals);
  SSATmp* spillStack();
  void    exceptionBarrier();
  SSATmp* ldStackAddr(int32_t offset);
  SSATmp* top(Type type, uint32_t index = 0);
  void    extendStack(uint32_t index, Type type);
  void    replace(uint32_t index, SSATmp* tmp);
  void    refineType(SSATmp* tmp, Type type);

  /*
   * Local instruction helpers.
   */
  SSATmp* ldLoc(uint32_t id);
  SSATmp* ldLocAddr(uint32_t id);
  SSATmp* ldLocInner(uint32_t id, Trace* exitTrace);
  SSATmp* ldLocInnerWarn(uint32_t id, Trace* target,
                         Trace* catchTrace = nullptr);
  SSATmp* stLoc(uint32_t id, Trace* exitTrace, SSATmp* newVal);
  SSATmp* stLocNRC(uint32_t id, Trace* exitTrace, SSATmp* newVal);
  SSATmp* stLocImpl(uint32_t id, Trace*, SSATmp* newVal, bool doRefCount);

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
  IRFactory& m_irFactory;
  std::unique_ptr<TraceBuilder> const m_tb;

  std::vector<BcState> m_bcStateStack;

  // The first HHBC offset for this tracelet, and the offset for the
  // next Traclet.
  const Offset m_startBcOff;
  const Offset m_nextTraceBcOff;

  // True if we're on the last HHBC opcode that will be emitted for
  // this tracelet.
  bool m_lastBcOff;

  // True if we've emitted an instruction that already handled
  // end-of-tracelet duties.  (E.g. emitRetC, etc.)  If it's not true,
  // we'll create a generic ReqBindJmp instruction after we're done.
  bool m_hasExit;

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
  uint32_t m_stackDeficit;
  EvalStack m_evalStack;

  /*
   * The FPI stack is used for inlining---when we start inlining at an
   * FCall, we look in here to find a definition of the StkPtr,offset
   * that can be used after the inlined callee "returns".
   */
  std::stack<std::pair<SSATmp*,int32_t>> m_fpiStack;
};

//////////////////////////////////////////////////////////////////////

}} // namespace HPHP::JIT

#endif
