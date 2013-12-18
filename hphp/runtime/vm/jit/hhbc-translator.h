/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/Optional.h"

#include "hphp/util/assertions.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/runtime-type.h"
#include "hphp/runtime/vm/jit/trace-builder.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/srckey.h"

using HPHP::JIT::NormalizedInstruction;

namespace HPHP {
namespace JIT { struct PropInfo; }
namespace JIT {

//////////////////////////////////////////////////////////////////////

struct EvalStack {
  explicit EvalStack(TraceBuilder& tb)
    : m_tb(tb)
  {}

  void push(SSATmp* tmp) {
    m_vector.push_back(tmp);
  }

  SSATmp* pop(TypeConstraint tc) {
    if (m_vector.size() == 0) {
      return nullptr;
    }
    SSATmp* tmp = m_vector.back();
    m_vector.pop_back();
    m_tb.constrainValue(tmp, tc);
    return tmp;
  }

  SSATmp* top(TypeConstraint tc, uint32_t offset = 0) const {
    if (offset >= m_vector.size()) {
      return nullptr;
    }
    uint32_t index = m_vector.size() - 1 - offset;
    m_tb.constrainValue(m_vector[index], tc);
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

  bool empty() const { return m_vector.empty(); }
  int  size()  const { return m_vector.size(); }
  void clear()       { m_vector.clear(); }

private:
  std::vector<SSATmp*> m_vector;
  TraceBuilder& m_tb;
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
 * AttrUnique class) instead of loading a Class* from RDS are
 * made at this level.
 */
struct HhbcTranslator {
  HhbcTranslator(Offset startOffset,
                 uint32_t initialSpOffsetFromFp,
                 const Func* func);

  // Accessors.
  IRTrace* trace() const { return m_tb->trace(); }
  TraceBuilder& traceBuilder() const { return *m_tb.get(); }
  IRUnit& unit() { return m_unit; }

  // In between each emit* call, irtranslator indicates the new
  // bytecode offset (or whether we're finished) using this API.
  void setBcOff(Offset newOff, bool lastBcOff);
  void end();
  void end(Offset nextPc);

  // Tracelet guards.
  void guardTypeStack(uint32_t stackIndex, Type type, bool outerOnly);
  void guardTypeLocal(uint32_t locId,      Type type, bool outerOnly);
  void guardTypeLocation(const RegionDesc::Location& loc, Type type,
                         bool outerOnly);
  void guardRefs(int64_t entryArDelta,
                 const vector<bool>& mask,
                 const vector<bool>& vals);

  // Interface to irtranslator for predicted and inferred types.
  void assertTypeLocal(uint32_t localIndex, Type type);
  void assertTypeStack(uint32_t stackIndex, Type type);
  void checkTypeLocal(uint32_t localIndex, Type type, Offset dest = -1);
  void checkTypeStack(uint32_t idx, Type type, Offset dest);
  void checkTypeTopOfStack(Type type, Offset nextByteCode);
  void assertType(const RegionDesc::Location& loc, Type type);
  void checkType(const RegionDesc::Location& loc, Type type,
                         Offset dest);
  void assertString(const RegionDesc::Location& loc, const StringData* sd);
  void assertClass(const RegionDesc::Location& loc, const Class* cls);

  RuntimeType rttFromLocation(const Location& loc);

  // Inlining-related functions.
  void beginInlining(unsigned numArgs,
                     const Func* target,
                     Offset returnBcOffset);
  bool isInlining() const;
  int inliningDepth() const;
  void profileFunctionEntry(const char* category);
  void profileInlineFunctionShape(const std::string& str);
  void profileSmallFunctionShape(const std::string& str);
  void profileFailedInlShape(const std::string& str);

  // Other public functions for irtranslator.
  void setThisAvailable();
  void emitInterpOne(const NormalizedInstruction&);
  void emitInterpOne(Type t, int popped);
  void emitInterpOne(Type t, int popped, int pushed, InterpOneData& id);
  std::string showStack() const;
  bool hasExit() const {
    return m_hasExit;
  }

  /*
   * An emit* function for each HHBC opcode.
   */

  void emitPrint();
  void emitThis();
  void emitCheckThis();
  void emitBareThis(int notice);
  void emitInitThisLoc(int32_t id);
  void emitArray(int arrayId);
  void emitNewArrayReserve(int capacity);
  void emitNewPackedArray(int n);
  void emitNewStructArray(uint32_t n, StringData** keys);
  void emitNewCol(int capacity);
  void emitClone();

  void emitArrayAdd();
  void emitAddElemC();
  void emitAddNewElemC();
  void emitNewCol(int type, int numElems);
  void emitColAddElemC();
  void emitColAddNewElemC();
  void emitDefCns(uint32_t id);
  void emitCnsCommon(uint32_t id, uint32_t fallbackId, bool error);
  void emitCns(uint32_t id);
  void emitCnsE(uint32_t id);
  void emitCnsU(uint32_t id, uint32_t fallbackId);
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
  void emitPushL(uint32_t id);
  void emitCGetL2(int32_t id);
  void emitCGetS();
  void emitCGetG();
  void emitMInstr(const NormalizedInstruction& ni);
  void emitVGetL(int32_t id);
  void emitVGetS();
  void emitVGetG();
  void emitSetL(int32_t id);
  void emitSetS();
  void emitSetG();
  void emitBindL(int32_t id);
  void emitBindS();
  void emitBindG();
  void emitUnsetL(int32_t id);
  void emitIssetL(int32_t id);
  void emitIssetS();
  void emitIssetG();
  void emitEmptyL(int32_t id);
  void emitEmptyS();
  void emitEmptyG();
  // The subOpc param can be one of either
  // Add, Sub, Mul, Div, Mod, Shl, Shr, Concat, BitAnd, BitOr, BitXor
  void emitSetOpL(Opcode subOpc, uint32_t id);
  void emitSetOpS(Opcode subOpc);
  // the pre & inc params encode the 4 possible sub opcodes:
  // PreInc, PostInc, PreDec, PostDec
  void emitIncDecL(bool pre, bool inc, uint32_t id);
  void emitIncDecS(bool pre, bool inc);
  void emitPopA();
  void emitPopC();
  void emitPopV();
  void emitPopR();
  void emitDup();
  void emitUnboxR();
  void emitJmpZ(Offset taken);
  void emitJmpNZ(Offset taken);
  void emitJmp(int32_t offset, bool breakTracelet, bool noSurprise);
  void emitGt()    { emitCmp(Gt);    }
  void emitGte()   { emitCmp(Gte);   }
  void emitLt()    { emitCmp(Lt);    }
  void emitLte()   { emitCmp(Lte);   }
  void emitEq()    { emitCmp(Eq);    }
  void emitNeq()   { emitCmp(Neq);   }
  void emitSame()  { emitCmp(Same);  }
  void emitNSame() { emitCmp(NSame); }
  void emitFPassCOp();
  void emitFPassR();
  void emitFPassV();
  void emitFPushCufIter(int32_t numParams, int32_t itId);
  void emitFPushCufOp(Op op, int32_t numArgs);
  bool emitFPushCufArray(SSATmp* callable, int32_t numParams);
  void emitFPushCufUnknown(Op op, int32_t numArgs);
  void emitFPushActRec(SSATmp* func, SSATmp* objOrClass, int32_t numArgs,
                       const StringData* invName = nullptr);
  void emitFPushFuncCommon(const Func* func,
                           const StringData* name,
                           const StringData* fallback,
                           int32_t numParams);
  void emitFPushFuncD(int32_t numParams, int32_t funcId);
  void emitFPushFuncU(int32_t numParams,
                      int32_t funcId,
                      int32_t fallbackFuncId);
  void emitFPushFunc(int32_t numParams);
  void emitFPushFuncObj(int32_t numParams);
  void emitFPushFuncArr(int32_t numParams);
  SSATmp* genClsMethodCtx(const Func* callee, const Class* cls);
  void emitFPushClsMethod(int32_t numParams);
  void emitFPushClsMethodD(int32_t numParams,
                           int32_t methodNameStrId,
                           int32_t clssNamedEntityPairId);
  void emitFPushObjMethodD(int32_t numParams,
                           int32_t methodNameStrId);
  void emitFPushObjMethodCommon(SSATmp* obj,
                                const StringData* methodName,
                                int32_t numParams,
                                bool shouldFatal,
                                SSATmp* extraSpill = nullptr);
  void emitFPushClsMethodF(int32_t numParams);
  void emitFPushCtorD(int32_t numParams, int32_t classNameStrId);
  void emitFPushCtor(int32_t numParams);
  void emitFPushCtorCommon(SSATmp* cls,
                           SSATmp* obj,
                           const Func* func,
                           int32_t numParams);
  void emitCreateCl(int32_t numParams, int32_t classNameStrId);
  void emitFCallArray(const Offset pcOffset, const Offset after,
                      bool destroyLocals);
  void emitFCall(uint32_t numParams, Offset returnBcOffset,
                 const Func* callee, bool destroyLocals);
  void emitFCallBuiltin(uint32_t numArgs, uint32_t numNonDefault,
                        int32_t funcId, bool destroyLocals);
  void emitClsCnsD(int32_t cnsNameStrId, int32_t clsNameStrId, Type outPred);
  void emitClsCns(int32_t cnsNameStrId);
  void emitAKExists();
  void emitAGetC();
  void emitAGetL(int localId);
  void emitIsScalarL(int id);
  void emitIsScalarC();
  void emitVerifyParamType(int32_t paramId);
  void emitInstanceOfD(int classNameStrId);
  void emitInstanceOf();
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

  // miscelaneous ops
  void emitFloor();
  void emitCeil();
  void emitAssertTL(int32_t id, AssertTOp);
  void emitAssertTStk(int32_t offset, AssertTOp);
  void emitAssertObjL(int32_t id, bool exact, Id);
  void emitAssertObjStk(int32_t offset, bool exact, Id);
  void emitPredictTL(int32_t id, AssertTOp);
  void emitPredictTStk(int32_t offset, AssertTOp);

  // binary arithmetic ops
  void emitAdd();
  void emitSub();
  void emitBitAnd();
  void emitBitOr();
  void emitBitXor();
  void emitBitNot();
  void emitAbs();
  void emitMul();
  void emitMod();
  void emitDiv();
  void emitSqrt();
  void emitShl();
  void emitShr();

  // boolean ops
  void emitXor();
  void emitNot();
  void emitNativeImpl();

  void emitClassExists();
  void emitInterfaceExists();
  void emitTraitExists();

  void emitStaticLocInit(uint32_t varId, uint32_t litStrId);
  void emitStaticLoc(uint32_t varId, uint32_t litStrId);
  void emitReqDoc(const StringData* name);

  // iterators
  void emitIterInit(uint32_t iterId,
                    int targetOffset,
                    uint32_t valLocalId,
                    bool invertCond);
  void emitIterInitK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId,
                     bool invertCond);
  void emitIterNext(uint32_t iterId,
                    int targetOffset,
                    uint32_t valLocalId,
                    bool invertCond);
  void emitIterNextK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId,
                     bool invertCond);
  void emitMIterInit(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitMIterInitK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId);
  void emitMIterNext(uint32_t iterId, int targetOffset, uint32_t valLocalId);
  void emitMIterNextK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId);
  void emitWIterInit(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     bool invertCond);
  void emitWIterInitK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      bool invertCond);
  void emitWIterNext(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     bool invertCond);
  void emitWIterNextK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      bool invertCond);

  void emitIterFree(uint32_t iterId);
  void emitMIterFree(uint32_t iterId);
  void emitDecodeCufIter(uint32_t iterId, int targetOffset);
  void emitCIterFree(uint32_t iterId);
  void emitIterBreak(const ImmVector& iv, uint32_t offset, bool breakTracelet);
  void emitVerifyParamType(uint32_t paramId);

  // continuations
  void emitCreateCont();
  void emitContEnter(int32_t returnBcOffset);
  void emitUnpackCont();
  void emitContReturnControl();
  void emitContSuspendImpl(int64_t labelId);
  void emitContSuspend(int64_t labelId);
  void emitContSuspendK(int64_t labelId);
  void emitContRetC();
  void emitContCheck(bool checkStarted);
  void emitContRaise();
  void emitContValid();
  void emitContKey();
  void emitContCurrent();
  void emitContStopped();

  // async functions
  void emitAsyncAwait();
  void emitAsyncESuspend(int64_t labelId, int iters);
  void emitAsyncWrapResult();
  void emitAsyncWrapException();

  void emitStrlen();
  void emitIncStat(int32_t counter, int32_t value, bool force = false);
  void emitIncTransCounter();
  void emitIncProfCounter(JIT::TransID transId);
  void emitCheckCold(JIT::TransID transId);
  void emitRB(Trace::RingBufferType t, SrcKey sk);
  void emitRB(Trace::RingBufferType t, std::string msg) {
    emitRB(t, makeStaticString(msg));
  }
  void emitRB(Trace::RingBufferType t, const StringData* msg);
  void emitIdx();
  void emitIdxCommon(Opcode opc, Block* catchBlock = nullptr);
  void emitArrayIdx();
  void emitIsTypeC(DataType t);
  void emitIsTypeL(uint32_t id, DataType t);

private:
  /*
   * MInstrTranslator is responsible for translating one of the vector
   * instructions (CGetM, SetM, IssetM, etc..) into hhir.
   */
  class MInstrTranslator {
   public:
    MInstrTranslator(const NormalizedInstruction& ni,
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
    void emitPropSpecialized(const MInstrAttr mia, JIT::PropInfo propInfo);
    void emitElem();
    void emitElemArray(SSATmp* key, bool warn);
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
    void emitPackedArrayGet(SSATmp* key);
    void emitPackedArrayIsset();
    void emitStringGet(SSATmp* key);
    void emitStringIsset();
    void emitVectorSet(SSATmp* key, SSATmp* value);
    void emitVectorGet(SSATmp* key);
    void emitVectorIsset();
    void emitPairGet(SSATmp* key);
    void emitPairIsset();
    void emitMapSet(SSATmp* key, SSATmp* value);
    void emitMapGet(SSATmp* key);
    void emitMapIsset();
    void emitStableMapSet(SSATmp* key, SSATmp* value);
    void emitStableMapGet(SSATmp* key);
    void emitStableMapIsset();

    // Generate a catch trace that does not perform any final DecRef operations
    // on scratch space, and return its first block.
    Block* makeEmptyCatch() {
      return m_ht.makeCatch();
    }

    // Generate a catch trace that will contain DecRef instructions for tvRef
    // and/or tvRef2 as required; return the first block.
    Block* makeCatch() {
      auto b = makeEmptyCatch();
      m_failedVec.push_back(b);
      return b;
    }

    // Generate a catch trace that will free any scratch space used and perform
    // a side-exit from a failed set operation, return the first block.
    Block* makeCatchSet() {
      assert(!m_failedSetBlock);
      m_failedSetBlock = makeCatch();

      // This catch trace will be modified in emitMPost to end with a side
      // exit, and TryEndCatch will fall through to that side exit if an
      // InvalidSetMException is thrown.
      m_failedSetBlock->back().setOpcode(TryEndCatch);
      return m_failedSetBlock;
    }

    void prependToTraces(IRInstruction* inst) {
      for (auto b : m_failedVec) {
       b->prepend(m_irf.cloneInstruction(inst));
      }
    }

    // Misc Helpers
    void numberStackInputs();
    void setNoMIState() { m_needMIS = false; }
    SSATmp* genMisPtr();
    SSATmp* getInput(unsigned i, TypeConstraint tc);
    SSATmp* getBase(TypeConstraint tc);
    SSATmp* getKey();
    SSATmp* getValue();
    SSATmp* getValAddr();
    void    constrainBase(TypeConstraint tc, SSATmp* value = nullptr);
    SSATmp* checkInitProp(SSATmp* baseAsObj,
                          SSATmp* propAddr,
                          JIT::PropInfo propOffset,
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
    SSATmp* genStk(Opcode op, Block* taken, Srcs... srcs);

    /* Various predicates about the current instruction */
    bool isSimpleBase();
    bool isSingleMember();

    enum class SimpleOp {
      // the opcode is not in a simple form or not on a proper collection type
      None,
      // simple opcode on Array
      Array,
      // simple opcode on Packed Array
      PackedArray,
      // simple opcode on String
      String,
      // simple opcode on Vector* (c_Vector*)
      Vector,
      // simple opcode on Map* (c_Map*)
      Map,
      // simple opcode on Map* (c_StableMap*)
      StableMap,
      // simple opcode on Map* (c_Pair*)
      Pair
    };
    SimpleOp simpleCollectionOp();
    void constrainCollectionOpBase();

    bool generateMVal() const;
    bool needFirstRatchet() const;
    bool needFinalRatchet() const;
    unsigned nLogicalRatchets() const;
    int ratchetInd() const;

    template<class... Args>
    SSATmp* cns(Args&&... args) {
      return m_irf.cns(std::forward<Args>(args)...);
    }

    template<class... Args>
    SSATmp* gen(Args&&... args) {
      return m_tb.gen(std::forward<Args>(args)...);
    }

    const NormalizedInstruction& m_ni;
    HhbcTranslator& m_ht;
    TraceBuilder& m_tb;
    IRUnit& m_irf;
    const MInstrInfo& m_mii;
    const BCMarker m_marker;
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

    /* If set, contains the catch target for the final set operation of this
     * instruction. The operations that set this member may need to return an
     * unexpected type, in which case they'll throw an InvalidSetMException. To
     * handle this, emitMPost adds code to the catch trace to fish the correct
     * value out of the exception and side exit. */
    Block* m_failedSetBlock;

    /* Contains a list of all catch blocks created in building the vector.
     * Each block must be appended with cleanup tasks (generally just DecRef)
     * to be performed if an exception occurs during the course of the vector
     * operation */
    std::vector<Block*> m_failedVec;
  };

private: // tracebuilder forwarding utilities
  template<class... Args>
  SSATmp* cns(Args&&... args) {
    return m_unit.cns(std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* gen(Args&&... args) {
    return m_tb->gen(std::forward<Args>(args)...);
  }

private:
  /*
   * Emit helpers.
   */
  void emitBindMem(SSATmp* ptr, SSATmp* src);
  void emitEmptyMem(SSATmp* ptr);
  void emitIncDecMem(bool pre, bool inc, SSATmp* ptr, Block* exit);
  void checkStrictlyInteger(SSATmp*& key, KeyType& keyType,
                            bool& checkForInt);
  SSATmp* emitLdClsPropAddrCached(const StringData* propName,
                                  Block* block);
  Type assertObjType(const StringData*);

  /*
   * Helpers for (CGet|VGet|Bind|Set|Isset|Empty)(G|S)
   */
  SSATmp* checkSupportedName(uint32_t stackIdx);
  void destroyName(SSATmp* name);
  typedef SSATmp* (HhbcTranslator::*EmitLdAddrFn)(Block*, SSATmp* name);
  void emitCGet(uint32_t stackIdx, bool exitOnFailure, EmitLdAddrFn);
  void emitVGet(uint32_t stackIdx, EmitLdAddrFn);
  void emitBind(uint32_t stackIdx, EmitLdAddrFn);
  void emitSet(uint32_t stackIdx, EmitLdAddrFn);
  void emitIsset(uint32_t stackIdx, EmitLdAddrFn);
  void emitEmpty(uint32_t stackOff, EmitLdAddrFn);
  SSATmp* emitLdGblAddr(Block* block, SSATmp* name);
  SSATmp* emitLdGblAddrDef(Block* block, SSATmp* name);
  SSATmp* emitLdClsPropAddr(SSATmp* name) {
    return emitLdClsPropAddrOrExit(nullptr, name);
  }
  SSATmp* emitLdClsPropAddrOrExit(Block* block, SSATmp* name);

  void emitUnboxRAux();
  void emitAGet(SSATmp* src, Block* catchBlock);
  void emitRetFromInlined(Type type);
  SSATmp* emitDecRefLocalsInline(SSATmp* retVal);
  void emitRet(Type type, bool freeInline);
  void emitCmp(Opcode opc);
  SSATmp* emitJmpCondHelper(int32_t offset, bool negate, SSATmp* src);
  SSATmp* emitIncDec(bool pre, bool inc, SSATmp* src);
  void emitBinaryArith(Opcode);
  template<class Lambda>
  SSATmp* emitIterInitCommon(int offset, Lambda genFunc, bool invertCond);
  BCMarker makeMarker(Offset bcOff);
  void updateMarker();
  template<class Lambda>
  SSATmp* emitMIterInitCommon(int offset, Lambda genFunc);
  SSATmp* staticTVCns(const TypedValue*);
  void emitJmpSurpriseCheck();
  void emitRetSurpriseCheck(SSATmp* retVal, bool inGenerator);
  void classExistsImpl(ClassKind);

  Type interpOutputType(const NormalizedInstruction&,
                        folly::Optional<Type>&) const;
  smart::vector<InterpOneData::LocalType>
  interpOutputLocals(const NormalizedInstruction&, bool& smashAll,
                     Type pushedType);

private: // Exit trace creation routines.
  Block* makeExit(Offset targetBcOff = -1);
  Block* makeExit(Offset targetBcOff, std::vector<SSATmp*>& spillValues);
  Block* makeExitWarn(Offset targetBcOff, std::vector<SSATmp*>& spillValues,
                      const StringData* warning);

  /*
   * Create a custom side exit---that is, an exit that does some
   * amount work before leaving the trace.
   *
   * The exit trace will spill things with for the current bytecode instruction.
   *
   * Then it will do an ExceptionBarrier, followed by whatever is done by the
   * CustomExit() function. Any instructions emitted by the custom exit will go
   * to the exit trace, and it may return an additional SSATmp* to spill on the
   * stack. If there is no additional SSATmp*, it should return nullptr.
   *
   * TODO(#2447661): this should be way better than this, should allow
   * using gen/push/spillStack/etc.
   */
  template<class ExitLambda>
  Block* makeSideExit(Offset targetBcOff, ExitLambda exit);

  /*
   * Generates an exit trace which will continue execution without HHIR.
   * This should be used in situations that HHIR cannot handle -- ideally
   * only in slow paths.
   */
  Block* makeExitSlow();
  Block* makeExitOpt(JIT::TransID transId);

  Block* makeCatch(std::vector<SSATmp*> extraSpill =
                   std::vector<SSATmp*>());

  /*
   * Implementation for the above.  Takes spillValues, target offset,
   * and a flag for whether to make a no-IR exit.
   *
   * Also takes a CustomExit() function that may perform more operations and
   * optionally return a single additional SSATmp* (otherwise nullptr) to spill
   * on the stack before exiting.
   */
  enum class ExitFlag {
    Interp,     // will bail to the interpreter to execute at least one BC instr
    JIT,        // will attempt to use the JIT to create a new translation
    // DelayedMarker means to use the current instruction marker
    // instead of one for targetBcOff.
    DelayedMarker,
  };
  typedef std::function<SSATmp* ()> CustomExit;
  Block* makeExitImpl(Offset targetBcOff, ExitFlag flag,
                      std::vector<SSATmp*>& spillValues, const CustomExit&);

public:
  /*
   * Accessors for the current function being compiled and its
   * class and unit.
   */
  const Func* curFunc()   const { return m_bcStateStack.back().func; }
  Class*      curClass()  const { return curFunc()->cls(); }
  Unit*       curUnit()   const { return curFunc()->unit(); }
  Offset      bcOff()     const { return m_bcStateStack.back().bcOff; }
  SrcKey      curSrcKey() const { return SrcKey(curFunc(), bcOff()); }
  size_t      spOffset()  const;
  Type        topType(uint32_t i, TypeConstraint c = DataTypeSpecific) const;

private:
  /*
   * Predicates for testing information about the relationship of a
   * class to the current context class.
   */
  bool classIsUniqueOrCtxParent(const Class*) const;
  bool classIsPersistentOrCtxParent(const Class*) const;

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
   * Return the bcOffset of the next instruction (whether it is in
   * this tracelet or not).
   */
  Offset nextBcOff() const { return nextSrcKey().offset(); }

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
  SSATmp* pushIncRef(SSATmp* tmp) { gen(IncRef, tmp); return push(tmp); }
  SSATmp* pop(Type type, TypeConstraint tc = DataTypeSpecific);
  void    popDecRef(Type type, TypeConstraint tc = DataTypeCountness);
  void    discard(unsigned n);
  SSATmp* popC(TypeConstraint tc = DataTypeSpecific) {
    return pop(Type::Cell, {tc.category, std::min(Type::Cell, tc.knownType)});
  }
  SSATmp* popV() { return pop(Type::BoxedCell); }
  SSATmp* popR() { return pop(Type::Gen);       }
  SSATmp* popA() { return pop(Type::Cls);       }
  SSATmp* popF(TypeConstraint tc = DataTypeSpecific) {
    return pop(Type::Gen, tc);
  }
  SSATmp* top(Type type, uint32_t index = 0,
              TypeConstraint tc = DataTypeSpecific);
  SSATmp* topC(uint32_t i = 0, TypeConstraint tc = DataTypeSpecific) {
    return top(Type::Cell, i, tc);
  }
  SSATmp* topV(uint32_t i = 0) { return top(Type::BoxedCell, i); }
  std::vector<SSATmp*> peekSpillValues() const;
  SSATmp* emitSpillStack(SSATmp* sp,
                         const std::vector<SSATmp*>& spillVals);
  SSATmp* spillStack();
  void    exceptionBarrier();
  SSATmp* ldStackAddr(int32_t offset, TypeConstraint tc);
  void    extendStack(uint32_t index, Type type);
  void    replace(uint32_t index, SSATmp* tmp);
  void    refineType(SSATmp* tmp, Type type);

  /*
   * Local instruction helpers.
   */
  SSATmp* ldLoc(uint32_t id, TypeConstraint constraint);
  SSATmp* ldLocAddr(uint32_t id, TypeConstraint constraint);
private:
  SSATmp* ldLocInner(uint32_t id, Block* exit,
                     TypeConstraint constraint);
  SSATmp* ldLocInnerWarn(uint32_t id, Block* target,
                         TypeConstraint constraint,
                         Block* catchBlock = nullptr);
public:
  SSATmp* pushStLoc(uint32_t id, Block* exit, SSATmp* newVal);
  SSATmp* stLoc(uint32_t id, Block* exit, SSATmp* newVal);
  SSATmp* stLocNRC(uint32_t id, Block* exit, SSATmp* newVal);
  SSATmp* stLocImpl(uint32_t id, Block*, SSATmp* newVal, bool doRefCount);

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
  IRUnit m_unit;
  std::unique_ptr<TraceBuilder> const m_tb;

  std::vector<BcState> m_bcStateStack;

  // The first HHBC offset for this tracelet
  const Offset m_startBcOff;

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

  /*
   * When we know that a call site is being inlined we add its StkPtr
   * offset pair to this stack to prevent it from being erroneously
   * popped during an FCall.
   *
   * XXX: There should be a better way to do this.  We don't allow
   * the tracelet to break during inlining so if we're careful it
   * should be possible to make sure FPush* and FCall[Array|Builtin]
   * is always matched with corresponding push()/pop().
   */
  std::stack<std::pair<SSATmp*,int32_t>> m_fpiActiveStack;
};

//////////////////////////////////////////////////////////////////////

}} // namespace HPHP::JIT

#endif
