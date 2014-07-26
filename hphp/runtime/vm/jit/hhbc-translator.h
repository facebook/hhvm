/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <utility>

#include "folly/Optional.h"

#include "hphp/util/assertions.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-instrs.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP {
namespace JIT {

struct PropInfo;

enum class IRGenMode {
  Trace,
  CFG,
};

enum JmpFlags {
  JmpFlagNone          = 0,
  JmpFlagBreakTracelet = 1,
  JmpFlagNextIsMerge   = 2,
  JmpFlagBothPaths     = 4,
  JmpFlagSurprise      = 8
};

inline JmpFlags operator|(JmpFlags f1, JmpFlags f2) {
  return static_cast<JmpFlags>(
    static_cast<unsigned>(f1) | static_cast<unsigned>(f2));
}

inline JmpFlags operator&(JmpFlags f1, JmpFlags f2) {
  return static_cast<JmpFlags>(
    static_cast<unsigned>(f1) & static_cast<unsigned>(f2));
}

//////////////////////////////////////////////////////////////////////

/*
 * This module is responsible for determining high-level HHBC->IR
 * compilation strategy.
 *
 * For each bytecode Foo in HHBC, there is a function in this class
 * called emitFoo which handles translating it into HHIR.
 *
 * Additionally, while translating bytecode, this module manages a
 * virtual execution stack modeling the state of the stack since the
 * last time we emitted an IR instruction that materialized it
 * (e.g. SpillStack or SpillFrame).
 *
 * HhbcTranslator is where we make optimizations that relate to overall
 * knowledge of the runtime and HHBC.  For example, decisions like
 * whether to use IR instructions that have constant Class*'s (for a
 * AttrUnique class) instead of loading a Class* from RDS are
 * made at this level.
 */
struct HhbcTranslator {
  explicit HhbcTranslator(TransContext context);

  // Accessors.
  IRBuilder& irBuilder() const { return *m_irb.get(); }
  IRUnit& unit() { return m_unit; }

  /*
   * In between each emit* call, irtranslator indicates the new
   * bytecode offset (or whether we're finished) using this API.
   *
   * Also updated is the id of the profiling translation for the code
   * we're about to generate next, if there was one.  (Otherwise,
   * kInvalidTransID.)
   */
  void setProfTransID(TransID);
  void setBcOff(Offset newOff, bool lastBcOff);

  void      setGenMode(IRGenMode mode);
  IRGenMode genMode() const { return m_mode; }

  // End a bytecode block and do the right thing with fallthrough.
  void endBlock(Offset next, bool nextIsMerge);

  void end();
  void end(Offset nextPc);

  // Prepare for a possible side exit.  This forces a SpillStack, so
  // that no instruction is needed in the exit block and thus a branch
  // in the main block can be smashed.
  void prepareForSideExit();

  // Tracelet guards.
  void guardTypeStack(uint32_t stackIndex, Type type, bool outerOnly);
  void guardTypeLocal(uint32_t locId,      Type type, bool outerOnly);
  void guardTypeLocation(const RegionDesc::Location& loc, Type type,
                         bool outerOnly);
  void guardRefs(int64_t entryArDelta,
                 const std::vector<bool>& mask,
                 const std::vector<bool>& vals);
  void endGuards();

  // Interface to irtranslator for predicted and inferred types.
  void assertTypeLocal(uint32_t localIndex, Type type);
  void assertTypeStack(uint32_t stackIndex, Type type);
  void checkTypeLocal(uint32_t localIndex, Type type, Offset dest = -1);
  void checkTypeStack(uint32_t idx, Type type, Offset dest);
  void checkTypeTopOfStack(Type type, Offset nextByteCode);
  void assertType(const RegionDesc::Location& loc, Type type);
  void checkType(const RegionDesc::Location& loc, Type type, Offset dest);
  Type typeFromLocation(const Location& loc);

  // Inlining-related functions.
  void beginInlining(unsigned numArgs,
                     const Func* target,
                     Offset returnBcOffset,
                     Type retTypePred);
  bool isInlining() const;
  int inliningDepth() const;
  void profileFunctionEntry(const char* category);
  void profileInlineFunctionShape(const std::string& str);
  void profileSmallFunctionShape(const std::string& str);
  void profileFailedInlShape(const std::string& str);
  void emitSingletonSProp(const Func* func,
                          const Op* clsOp, const Op* propOp);
  void emitSingletonSLoc(const Func* func, const Op* op);

  // Other public functions for irtranslator.
  void setThisAvailable();
  void emitInterpOne(const NormalizedInstruction&);
  void emitInterpOne(int popped);
  void emitInterpOne(Type t, int popped);
  void emitInterpOne(folly::Optional<Type> t, int popped, int pushed,
                     InterpOneData& id);
  std::string showStack() const;

public:
  /*
   * Accessors for the current function being compiled, its class and unit, the
   * current SrcKey, and the current eval stack.
   */
  const Func* curFunc()     const { return m_bcStateStack.back().func; }
  Class*      curClass()    const { return curFunc()->cls(); }
  Unit*       curUnit()     const { return curFunc()->unit(); }
  Offset      bcOff()       const { return m_bcStateStack.back().bcOff; }
  SrcKey      curSrcKey()   const { return SrcKey(curFunc(), bcOff(),
                                                  resumed()); }
  bool        resumed()     const { return m_bcStateStack.back().resumed; }
  size_t      spOffset()    const;
  Type        topType(uint32_t i, TypeConstraint c = DataTypeSpecific) const;

  /*
   * Eval stack helpers.
   */
  SSATmp* popC(TypeConstraint tc = DataTypeSpecific) {
    return pop(Type::Cell, tc);
  }
  SSATmp* push(SSATmp* tmp);
  SSATmp* pushIncRef(SSATmp* tmp, TypeConstraint tc = DataTypeCountness);
  SSATmp* pop(Type type, TypeConstraint tc = DataTypeSpecific);
  void    popDecRef(Type type, TypeConstraint tc = DataTypeCountness);
  void    discard(unsigned n);
  SSATmp* popV() { return pop(Type::BoxedCell); }
  SSATmp* popR() { return pop(Type::Gen);       }
  SSATmp* popA() { return pop(Type::Cls);       }
  SSATmp* popF(TypeConstraint tc = DataTypeSpecific) {
    return pop(Type::Gen, tc);
  }
  SSATmp* top(TypeConstraint tc, uint32_t offset = 0) const;
  SSATmp* top(Type type, uint32_t index = 0,
              TypeConstraint tc = DataTypeSpecific);
  SSATmp* topC(uint32_t i = 0, TypeConstraint tc = DataTypeSpecific) {
    return top(Type::Cell, i, tc);
  }
  SSATmp* topF(uint32_t i = 0, TypeConstraint tc = DataTypeSpecific) {
    return top(Type::Gen, i, tc);
  }
  SSATmp* topV(uint32_t i = 0) { return top(Type::BoxedCell, i); }
  SSATmp* topR(uint32_t i = 0) { return top(Type::Gen, i); }

public:
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
  void emitNewMixedArray(int capacity);
  void emitNewLikeArrayL(int id, int capacity);
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
  void emitConcatN(int n);
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
  void emitDir();
  void emitFile();
  void emitCGetL(int32_t id);
  void emitFPassL(int32_t id);
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
  // The subOp param can be one of either
  // Add, Sub, Mul, Div, Mod, Shl, Shr, Concat, BitAnd, BitOr, BitXor
  void emitSetOpL(Op subOp, uint32_t id);
  // the pre, inc, and over params encode the 8 possible sub opcodes
  void emitIncDecL(bool pre, bool inc, bool over, uint32_t id);
  void emitPopA();
  void emitPopC();
  void emitPopV();
  void emitPopR();
  void emitDup();
  void emitUnboxR();
  void emitUnbox();
  void emitJmpZ(Offset taken, JmpFlags);
  void emitJmpNZ(Offset taken, JmpFlags);
  void emitJmpImpl(int32_t offset, JmpFlags, Block* catchBlock);
  void emitJmp(int32_t offset, JmpFlags);
  void emitGt()    { emitCmp(Gt);    }
  void emitGte()   { emitCmp(Gte);   }
  void emitLt()    { emitCmp(Lt);    }
  void emitLte()   { emitCmp(Lte);   }
  void emitEq()    { emitCmp(Eq);    }
  void emitNeq()   { emitCmp(Neq);   }
  void emitSame()  { emitCmp(Same);  }
  void emitNSame() { emitCmp(NSame); }
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
  void fpushObjMethodUnknown(SSATmp* obj,
                             const StringData* methodName,
                             int32_t numParams,
                             bool shouldFatal,
                             SSATmp* extraSpill);
  void emitFPushClsMethodF(int32_t numParams);
  void emitInitProps(const Class* cls, Block* catchBlock);
  void emitInitSProps(const Class* cls, Block* catchBlock);
  SSATmp* emitAllocObjFast(const Class* cls);
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
  template<class GetArg>
  void emitBuiltinCall(const Func* callee,
                       uint32_t numArgs,
                       uint32_t numNonDefault,
                       SSATmp* paramThis,
                       bool inlining,
                       bool wasInliningConstructor,
                       bool destroyLocals,
                       GetArg getArg);
  void emitFCallBuiltinCoerce(const Func* callee,
                              uint32_t numArgs,
                              uint32_t numNonDefault,
                              bool destroyLocals);
  void emitFCallBuiltin(uint32_t numArgs, uint32_t numNonDefault,
                        int32_t funcId, bool destroyLocals);
  void emitClsCnsD(int32_t cnsNameStrId, int32_t clsNameStrId, Type outPred);
  void emitClsCns(int32_t cnsNameStrId);
  void emitAKExists();
  void emitAGetC();
  void emitAGetL(int localId);
  void emitIsScalarL(int id);
  void emitIsScalarC();
  void emitVerifyTypeImpl(int32_t id);
  void emitVerifyParamType(int32_t paramId);
  void emitVerifyRetTypeC();
  void emitVerifyRetTypeV();
  void emitInstanceOfD(int classNameStrId);
  void emitInstanceOf();
  void emitCastBool();
  void emitCastInt();
  void emitCastDouble();
  void emitCastString();
  void emitCastArray();
  void emitCastObject();

  void emitNameA();

  void emitSwitch(const ImmVector&, int64_t base, bool bounded);
  void emitSSwitch(const ImmVector&);

  // freeInline indicates whether we should be doing decrefs inlined in
  // the TC, or using the generic decref helper.
  void emitRetC(bool freeInline);
  void emitRetV(bool freeInline);

  // miscellaneous ops
  void emitFloor();
  void emitCeil();
  void emitCheckProp(Id propId);
  void emitInitProp(Id propId, InitPropOp op);
  void emitAssertRATL(int32_t loc, RepoAuthType rat);
  void emitAssertRATStk(int32_t offset, RepoAuthType rat);
  void emitSilence(Id localId, unsigned char subop);

  // arithmetic ops
  void emitAdd();
  void emitSub();
  void emitMul();
  void emitBitAnd();
  void emitBitOr();
  void emitBitXor();
  void emitBitNot();
  void emitAbs();
  void emitMod();
  void emitDiv();
  void emitPow();
  void emitSqrt();
  void emitShl();
  void emitShr();
  void emitAddO();
  void emitSubO();
  void emitMulO();

  // boolean ops
  void emitXor();
  void emitNot();

  void emitNativeImpl();
  void emitOODeclExists(unsigned char subop);

  void emitStaticLocInit(uint32_t varId, uint32_t litStrId);
  void emitStaticLoc(uint32_t varId, uint32_t litStrId);
  void emitReqDoc(const StringData* name);

  // iterators
  void emitIterInit(uint32_t iterId,
                    int targetOffset,
                    uint32_t valLocalId,
                    bool invertCond,
                    JmpFlags jmpFlags);
  void emitIterInitK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId,
                     bool invertCond,
                     JmpFlags jmpFlags);
  void emitIterNext(uint32_t iterId,
                    int targetOffset,
                    uint32_t valLocalId,
                    bool invertCond,
                    JmpFlags jmpFlags);
  void emitIterNextK(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     uint32_t keyLocalId,
                     bool invertCond,
                     JmpFlags jmpFlags);
  void emitMIterInit(uint32_t iterId, int targetOffset, uint32_t valLocalId,
                     JmpFlags jmpFlags);
  void emitMIterInitK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      JmpFlags jmpFlags);
  void emitMIterNext(uint32_t iterId, int targetOffset, uint32_t valLocalId,
                     JmpFlags jmpFlags);
  void emitMIterNextK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      JmpFlags jmpFlags);
  void emitWIterInit(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     bool invertCond,
                     JmpFlags jmpFlags);
  void emitWIterInitK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      bool invertCond,
                      JmpFlags jmpFlags);
  void emitWIterNext(uint32_t iterId,
                     int targetOffset,
                     uint32_t valLocalId,
                     bool invertCond,
                     JmpFlags jmpFlags);
  void emitWIterNextK(uint32_t iterId,
                      int targetOffset,
                      uint32_t valLocalId,
                      uint32_t keyLocalId,
                      bool invertCond,
                      JmpFlags jmpFlags);

  void emitIterFree(uint32_t iterId);
  void emitMIterFree(uint32_t iterId);
  void emitDecodeCufIter(uint32_t iterId, int targetOffset,
                    JmpFlags jmpFlags);
  void emitCIterFree(uint32_t iterId);
  void emitIterBreak(const ImmVector& iv, uint32_t offset, bool breakTracelet);
  void emitVerifyParamType(uint32_t paramId);

  // generators
  void emitCreateCont(Offset resumeOffset);
  void emitContEnter(Offset returnOffset);
  void emitYieldReturnControl(Block* catchBlock);
  void emitYieldImpl(Offset resumeOffset);
  void emitYield(Offset resumeOffset);
  void emitYieldK(Offset resumeOffset);
  void emitContCheck(bool checkStarted);
  void emitContValid();
  void emitContKey();
  void emitContCurrent();

  // async functions
  void emitAwaitE(SSATmp* child, Block* catchBlock, Offset resumeOffset,
                  int iters);
  void emitAwaitR(SSATmp* child, Block* catchBlock, Offset resumeOffset);
  void emitAwait(Offset resumeOffset, int iters);

  void emitStrlen();
  void emitIncStat(int32_t counter, int32_t value, bool force);
  void emitIncTransCounter();
  void emitIncProfCounter(TransID transId);
  void emitCheckCold(TransID transId);
  void emitRB(Trace::RingBufferType t, SrcKey sk, int level = 1);
  void emitRB(Trace::RingBufferType t, std::string msg, int level = 1);
  void emitRB(Trace::RingBufferType t, const StringData* msg, int level = 1);
  void emitDbgAssertRetAddr();
  void emitIdx();
  void emitIdxCommon(Opcode opc, Block* catchBlock = nullptr);
  void emitArrayIdx();
  void emitIsTypeC(DataType t);
  void emitIsTypeL(uint32_t id, DataType t);

#define CASE(nm) static constexpr bool supports##nm = true;
  REGULAR_INSTRS
#undef CASE

private:
  /*
   * MInstrTranslator is responsible for translating one of the m-instr
   * instructions (CGetM, SetM, IssetM, etc..) into hhir.
   */
  struct MInstrTranslator {
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
    void emitPropSpecialized(const MInstrAttr mia, PropInfo propInfo);
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
    SSATmp* emitArrayGet(SSATmp* key);
    void emitProfiledArrayGet(SSATmp* key);
    void emitArrayIsset();
    SSATmp* emitPackedArrayGet(SSATmp* base, SSATmp* key,
                               bool profiled = false);
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

    // Generate a catch block that does not perform any final DecRef operations
    // on scratch space, and return its first block.
    Block* makeEmptyCatch();

    // Generate a catch block that will contain DecRef instructions for tvRef
    // and/or tvRef2 as required; return the first block.
    Block* makeCatch();

    // Generate a catch block that will free any scratch space used and perform
    // a side-exit from a failed set operation, return the first block.
    Block* makeCatchSet();

    void prependToTraces(IRInstruction* inst);

    // Misc Helpers
    void numberStackInputs();
    void setNoMIState() { m_needMIS = false; }
    SSATmp* genMisPtr();
    SSATmp* getInput(unsigned i, TypeConstraint tc);
    SSATmp* getBase(TypeConstraint tc);
    SSATmp* getKey();
    SSATmp* getValue();
    SSATmp* getValAddr();
    void    constrainBase(TypeConstraint tc);
    SSATmp* checkInitProp(SSATmp* baseAsObj,
                          SSATmp* propAddr,
                          PropInfo propOffset,
                          bool warn,
                          bool define);
    Class* contextClass() const;
    PropInfo getCurrentPropertyOffset(const Class*& cls);

    /*
     * genStk is a wrapper around IRBuilder::gen() to deal with instructions
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
      // simple opcode on Profiled Array,
      ProfiledArray,
      // simple opcode on Packed Array
      PackedArray,
      // simple opcode on String
      String,
      // simple opcode on Vector* (c_Vector* or c_ImmVector*)
      Vector,
      // simple opcode on Map* (c_Map*)
      Map,
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
      return m_unit.cns(std::forward<Args>(args)...);
    }

    template<class... Args>
    SSATmp* gen(Args&&... args) {
      return m_irb.gen(std::forward<Args>(args)...);
    }

    const NormalizedInstruction& m_ni;
    HhbcTranslator& m_ht;
    IRBuilder& m_irb;
    IRUnit& m_unit;
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

private: // forwarding utilities
  template<class... Args>
  SSATmp* cns(Args&&... args) {
    return m_unit.cns(std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* gen(Args&&... args) {
    return m_irb->gen(std::forward<Args>(args)...);
  }

private:
  /*
   * Emit helpers.
   */
  void emitBindMem(SSATmp* ptr, SSATmp* src);
  void emitEmptyMem(SSATmp* ptr);
  void emitIncDecMem(bool pre, bool inc, SSATmp* ptr, Block* exit);
  void checkStrictlyInteger(SSATmp*& key, KeyType& keyType,
                            bool& checkForInt, bool& converted);
  folly::Optional<Type> ratToAssertType(RepoAuthType rat) const;
  void destroyName(SSATmp* name);
  SSATmp* ldClsPropAddrKnown(Block* catchBlock,
                             SSATmp* cls, SSATmp* name);
  SSATmp* ldClsPropAddr(Block* catchBlock, SSATmp* cls,
                        SSATmp* name, bool raise);
  void emitUnboxRAux();
  void emitAGet(SSATmp* src, Block* catchBlock);
  void emitRetFromInlined(Type type);
  void emitNativeImplInlined();
  void emitEndInlinedCommon();
  void emitDecRefLocalsInline();
  void emitRet(Type type, bool freeInline);
  void emitCmp(Opcode opc);
  void emitJmpCondHelper(int32_t taken, bool negate, JmpFlags, SSATmp* src);
  SSATmp* emitIncDec(bool pre, bool inc, bool over, SSATmp* src);
  template<class Lambda>
  void emitIterInitCommon(int offset, JmpFlags jmpFlags, Lambda genFunc,
                          bool invertCond);
  BCMarker makeMarker(Offset bcOff);
  void updateMarker();
  template<class Lambda>
  void emitMIterInitCommon(int offset, JmpFlags jmpFlags, Lambda genFunc);
  SSATmp* staticTVCns(const TypedValue*);
  void emitJmpSurpriseCheck(Block* catchBlock);
  void emitRetSurpriseCheck(SSATmp* fp, SSATmp* retVal, Block* catchBlock,
                            bool suspendingResumed);
  void classExistsImpl(ClassKind);

  folly::Optional<Type> interpOutputType(const NormalizedInstruction&,
                                         folly::Optional<Type>&) const;
  smart::vector<InterpOneData::LocalType>
  interpOutputLocals(const NormalizedInstruction&, bool& smashAll,
                     folly::Optional<Type> pushedType);

  template<typename G, typename L>
  void emitProfiledGuard(Type t, const char* location, int32_t id, G doGuard,
                         L loadAddr);

  bool optimizedFCallBuiltin(const Func* func, uint32_t numArgs,
                             uint32_t numNonDefault);
  SSATmp* optimizedCallIniGet();
  SSATmp* optimizedCallCount();
  SSATmp* optimizedCallGetClass(uint32_t);
  SSATmp* optimizedCallGetCalledClass();

private: // Exit trace creation routines.
  Block* makeExit(Offset targetBcOff = -1);
  Block* makeExit(TransFlags trflags);
  Block* makeExit(Offset targetBcOff,
                  std::vector<SSATmp*>& spillValues,
                  TransFlags trflags = TransFlags{});
  Block* makeExitWarn(Offset targetBcOff, std::vector<SSATmp*>& spillValues,
                      const StringData* warning);
  Block* makeExitError(SSATmp* msg, Block* catchBlock);
  Block* makeExitNullThis();
  Block* makePseudoMainExit(Offset targetBcOff = -1);

  SSATmp* promoteBool(SSATmp* src);
  Opcode promoteBinaryDoubles(Op op, SSATmp*& src1, SSATmp*& src2);

  void emitBinaryBitOp(Op op);
  void emitBinaryArith(Op op);

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
  Block* makeExitOpt(TransID transId);

  template<typename Body>
  Block* makeCatchImpl(Body body);
  Block* makeCatch(std::vector<SSATmp*> extraSpill =
                   std::vector<SSATmp*>(),
                   int64_t numPop = 0);
  Block* makeCatchNoSpill();

  /*
   * Create a block for a branch target that will be generated later.
   */
  Block* makeBlock(Offset offset);

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
                      std::vector<SSATmp*>& spillValues,
                      const CustomExit& customFn,
                      TransFlags trflags = TransFlags{});

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
  SrcKey nextSrcKey() const;

  /*
   * Return the bcOffset of the next instruction (whether it is in
   * this tracelet or not).
   */
  Offset nextBcOff() const;

  /*
   * Helpers for resolving bytecode immediate ids.
   */
  ArrayData*  lookupArrayId(int arrId);
  StringData* lookupStringId(int strId);
  Func*       lookupFuncId(int funcId);
  PreClass*   lookupPreClassId(int preClassId);
  const NamedEntityPair& lookupNamedEntityPairId(int id);
  const NamedEntity* lookupNamedEntityId(int id);

  std::vector<SSATmp*> peekSpillValues() const;
  SSATmp* emitSpillStack(SSATmp* sp,
                         const std::vector<SSATmp*>& spillVals,
                         int64_t extraOffset = 0);
  SSATmp* spillStack();
  void    exceptionBarrier();
  SSATmp* ldStackAddr(int32_t offset, TypeConstraint tc);
  void    extendStack(uint32_t index, Type type);
  void    replace(uint32_t index, SSATmp* tmp);

  SSATmp* unbox(SSATmp* val, Block* exit);

  /*
   * Local instruction helpers. The ldgblExit is so helpers can emit the guard
   * for LdGbl insts if we're in the pseudomain. The ldrefExit is for helpers
   * that might need to emit a LdRef to unbox a local.
   */
  SSATmp* ldLoc(uint32_t id,
                Block* ldgblExit,
                TypeConstraint constraint);
  SSATmp* ldLocAddr(uint32_t id, TypeConstraint constraint);
  SSATmp* ldLocInner(uint32_t id,
                     Block* ldrefExit,
                     Block* ldgblExit,
                     TypeConstraint constraint);
  SSATmp* ldLocInnerWarn(uint32_t id,
                         Block* ldrefExit,
                         Block* ldgblExit,
                         TypeConstraint constraint,
                         Block* catchBlock = nullptr);

  SSATmp* pushStLoc(uint32_t id,
                    Block* ldrefExit,
                    Block* ldgblExit,
                    SSATmp* newVal);
  SSATmp* stLoc(uint32_t id,
                Block* ldrefExit,
                Block* ldgblExit,
                SSATmp* newVal);
  SSATmp* stLocNRC(uint32_t id,
                   Block* ldrefExit,
                   Block* ldgblExit,
                   SSATmp* newVal);
  SSATmp* stLocImpl(uint32_t id,
                    Block* ldrefExit,
                    Block* ldgblExit,
                    SSATmp* newVal,
                    bool decRefOld,
                    bool incRefOld);

  SSATmp* genStLocal(uint32_t id, SSATmp* fp, SSATmp* newVal);

  bool inPseudoMain() const;

private:
  // Tracks information about the current bytecode offset and which
  // function we are in.  Goes in m_bcStateStack; we push and pop as
  // we deal with inlined calls.
  struct BcState {
    Offset bcOff;
    bool resumed;
    const Func* func;
  };

private:
  const TransContext m_context;
  IRUnit m_unit;
  std::unique_ptr<IRBuilder> const m_irb;
  std::vector<BcState> m_bcStateStack;

  // The id of the profiling translation for the code we're currently
  // generating, if there was one, otherwise kInvalidTransID.
  TransID m_profTransID{kInvalidTransID};

  // True if we're on the last HHBC opcode that will be emitted for
  // this tracelet.
  bool m_lastBcOff;

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
   */
  std::stack<std::pair<SSATmp*,int32_t>> m_fpiActiveStack;

  IRGenMode m_mode;
};

//////////////////////////////////////////////////////////////////////

}} // namespace HPHP::JIT

#endif
