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

#ifndef incl_HPHP_VM_CG_X64_H_
#define incl_HPHP_VM_CG_X64_H_

#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/target-profile.h"

namespace HPHP { namespace JIT { namespace X64 {

// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kCacheLineSize = 64;
constexpr size_t kCacheLineMask = kCacheLineSize - 1;

struct CodeGenerator : public JIT::CodeGenerator {
  typedef JIT::X64Assembler Asm;

  CodeGenerator(const IRUnit& unit, CodeBlock& mainCode, CodeBlock& coldCode,
                CodeBlock& frozenCode, CodegenState& state)
    : m_unit(unit)
    , m_mainCode(mainCode)
    , m_coldCode(coldCode)
    , m_frozenCode(frozenCode)
    , m_as(mainCode)
    , m_acold(coldCode)
    , m_state(state)
    , m_rScratch(InvalidReg)
    , m_curInst(nullptr)
  {
  }

  virtual ~CodeGenerator() {
  }

  void cgInst(IRInstruction* inst) override;

private:
  const PhysLoc srcLoc(unsigned i) const {
    return (*m_instRegs).src(i);
  }
  const PhysLoc dstLoc(unsigned i) const {
    return (*m_instRegs).dst(i);
  }
  ArgGroup argGroup() const {
    return ArgGroup(m_curInst, *m_instRegs);
  }

  // Autogenerate function declarations for each IR instruction in ir.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  void cgCallNative(Asm& a, IRInstruction* inst);

  CallDest callDest(PhysReg reg0, PhysReg reg1 = InvalidReg) const;
  CallDest callDest(const IRInstruction*) const;
  CallDest callDestTV(const IRInstruction*) const;
  CallDest callDestDbl(const IRInstruction*) const;

  // Main call helper:
  void cgCallHelper(Asm& a, CppCall call, const CallDest& dstInfo,
                    SyncOptions sync, ArgGroup& args, RegSet toSave);

  // Overload to make the toSave RegSet optional:
  void cgCallHelper(Asm& a, CppCall call, const CallDest& dstInfo,
                    SyncOptions sync, ArgGroup& args);

  void cgInterpOneCommon(IRInstruction* inst);

  enum class Width { Value, Full };
  void cgStore(MemoryRef dst, SSATmp* src, PhysLoc src_loc, Width);
  void cgStoreTypedValue(MemoryRef dst, SSATmp* src, PhysLoc src_loc);

  // helpers to load a value in dst. When label is not null a type check
  // is performed against value to ensure it is of the type expected by dst
  void cgLoad(SSATmp* dst, PhysLoc dstLoc, MemoryRef value,
              Block* label = nullptr);
  void cgLoadTypedValue(SSATmp* dst, PhysLoc dstLoc, MemoryRef base,
                        Block* label = nullptr);

  // internal helpers to manage register conflicts from a source to a PhysReg
  // destination.
  // If the conflict cannot be resolved the out param isResolved is set to
  // false and the caller should take proper action
  MemoryRef resolveRegCollision(PhysReg dst, MemoryRef value,
                                bool& isResolved);

  template<class Loc1, class Loc2, class JmpFn>
  void emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc, JmpFn doJcc);

  template<class DataLoc, class JmpFn>
  void emitSpecializedTypeTest(Type type, DataLoc data, JmpFn doJcc);

  template<class Loc>
  void emitTypeCheck(Type type, Loc typeSrc, Loc dataSrc, Block* taken);
  template<class Loc>
  void emitTypeGuard(Type type, Loc typeLoc, Loc dataLoc);

  void cgIncRefWork(Type type, SSATmp* src, PhysLoc srcLoc);
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck);

  template<class OpInstr>
  void cgUnaryIntOp(PhysLoc dst, SSATmp* src, PhysLoc src_loc, OpInstr);

  enum Commutativity { Commutative, NonCommutative };

  void cgRoundCommon(IRInstruction* inst, RoundDirection dir);

  template<class RegType>
  void cgBinaryIntOp(IRInstruction*,
                     void (Asm::*intImm)(Immed, RegType),
                     void (Asm::*intRR)(RegType, RegType),
                     void (Asm::*mov)(RegType, RegType),
                     RegType (*conv)(PhysReg),
                     Commutativity);
  void cgBinaryDblOp(IRInstruction*,
                     void (Asm::*fpRR)(RegXMM, RegXMM));

  void cgShiftCommon(IRInstruction* inst,
                     void (Asm::*instrIR)(Immed, Reg64),
                     void (Asm::*instrR)(Reg64));

  void emitVerifyCls(IRInstruction* inst);

  void emitGetCtxFwdCallWithThis(PhysReg ctxReg,
                                 bool    staticCallee);

  void emitGetCtxFwdCallWithThisDyn(PhysReg      destCtxReg,
                                    PhysReg      thisReg,
                                    RDS::Handle ch);

  void cgJcc(IRInstruction* inst);          // helper
  void cgReqBindJcc(IRInstruction* inst);   // helper
  void cgExitJcc(IRInstruction* inst);      // helper
  void cgJccInt(IRInstruction* inst);         // helper
  void cgReqBindJccInt(IRInstruction* inst);  // helper
  void cgExitJccInt(IRInstruction* inst); // helper
  void emitCmpInt(IRInstruction* inst, ConditionCode);
  void emitCmpEqDbl(IRInstruction* inst, ComparisonPred);
  void emitCmpRelDbl(IRInstruction* inst, ConditionCode, bool);
  void cgCmpHelper(IRInstruction* inst, ConditionCode,
                   int64_t (*str_cmp_str)(StringData*, StringData*),
                   int64_t (*str_cmp_int)(StringData*, int64_t),
                   int64_t (*str_cmp_obj)(StringData*, ObjectData*),
                   int64_t (*obj_cmp_obj)(ObjectData*, ObjectData*),
                   int64_t (*obj_cmp_int)(ObjectData*, int64_t),
                   int64_t (*arr_cmp_arr)(ArrayData*, ArrayData*));

  template<class Loc>
  void emitSideExitGuard(Type type, Loc typeLoc,
                         Loc dataLoc, Offset taken);
  void emitReqBindJcc(ConditionCode cc, const ReqBindJccData*);

  void emitCompare(IRInstruction* inst);
  void emitCompareInt(IRInstruction* inst);
  void emitTestZero(SSATmp*, PhysLoc);
  bool emitIncDecHelper(PhysLoc dst, SSATmp* src1, PhysLoc loc1,
                        SSATmp* src2, PhysLoc loc2,
                        void(Asm::*emitFunc)(Reg64));

private:
  PhysReg selectScratchReg(IRInstruction* inst);
  void emitLoadImm(Asm& as, int64_t val, PhysReg dstReg);
  PhysReg prepXMMReg(Asm& as, const SSATmp* src, const PhysLoc& srcLoc,
                     RegXMM rXMMScratch);
  void emitSetCc(IRInstruction*, ConditionCode);
  template<class JmpFn>
  void emitIsTypeTest(IRInstruction* inst, JmpFn doJcc);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgJmpIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  void emitInstanceBitmaskCheck(IRInstruction*);
  void emitTraceRet(Asm& as);
  void emitInitObjProps(PhysReg dstReg, const Class* cls, size_t nProps);

  bool decRefDestroyIsUnlikely(OptDecRefProfile& profile, Type type);
  template <typename F>
  Address cgCheckStaticBitAndDecRef(Asm& a, Type type,
                                    PhysReg dataReg,
                                    F destroy);
  Address cgCheckStaticBitAndDecRef(Asm& a, Type type,
                                    PhysReg dataReg);
  Address cgCheckRefCountedType(PhysReg typeReg);
  Address cgCheckRefCountedType(PhysReg baseReg,
                                int64_t offset);
  void cgDecRefStaticType(Asm& a, Type type,
                          PhysReg dataReg,
                          bool genZeroCheck);
  void cgDecRefDynamicType(PhysReg typeReg,
                           PhysReg dataReg,
                           bool genZeroCheck);
  void cgDecRefDynamicTypeMem(PhysReg baseReg,
                              int64_t offset);
  void cgDecRefMem(Type type,
                   PhysReg baseReg,
                   int64_t offset);

  void cgIterNextCommon(IRInstruction* inst);
  void cgIterInitCommon(IRInstruction* inst);
  void cgMIterNextCommon(IRInstruction* inst);
  void cgMIterInitCommon(IRInstruction* inst);
  void cgLdFuncCachedCommon(IRInstruction* inst);
  void cgLookupCnsCommon(IRInstruction* inst);
  RDS::Handle cgLdClsCachedCommon(IRInstruction* inst);
  void emitFwdJcc(ConditionCode cc, Block* target);
  void emitFwdJcc(Asm& a, ConditionCode cc, Block* target);
  const Func* curFunc() const { return m_curInst->marker().func(); };
  const Class* curClass() const { return curFunc()->cls(); }
  const Unit* curUnit() const { return curFunc()->unit(); }
  bool resumed() const { return m_curInst->marker().resumed(); };
  void recordSyncPoint(Asm& as, SyncOptions sync = SyncOptions::kSyncPoint);
  int iterOffset(SSATmp* tmp) { return iterOffset(tmp->intVal()); }
  int iterOffset(uint32_t id);
  void emitReqBindAddr(TCA& dest, SrcKey sk);

  void emitAdjustSp(PhysReg spReg, PhysReg dstReg, int adjustment);
  void emitConvBoolOrIntToDbl(IRInstruction* inst);
  void cgLdClsMethodCacheCommon(IRInstruction* inst, Offset offset);
  void emitLdRaw(IRInstruction* inst, size_t extraOff);
  void emitStRaw(IRInstruction* inst, size_t offset, int size);

  /*
   * Execute the code emitted by 'taken' only if the given condition code is
   * true.
   */
  template <class Block>
  void ifBlock(ConditionCode cc, Block taken, bool unlikely = false);

  /*
   * Generate an if-block that branches around some unlikely code, handling
   * the cases when a == acold and a != acold.  cc is the branch condition
   * to run the unlikely block.
   *
   * Passes the proper assembler to use to the unlikely function.
   */
  template <class Block>
  void unlikelyIfBlock(ConditionCode cc, Block unlikely);

  // Generate an if-then-else block
  template <class Then, class Else>
  void ifThenElse(Asm& a, ConditionCode cc, Then thenBlock, Else elseBlock);

  // Generate an if-then-else block into m_as.
  template <class Then, class Else>
  void ifThenElse(ConditionCode cc, Then thenBlock, Else elseBlock,
                  bool unlikely = false);

  /*
   * Same as ifThenElse except the first block is off in acold
   */
  template <class Then, class Else>
  void unlikelyIfThenElse(ConditionCode cc, Then unlikely, Else elseBlock);

  // This is for printing partially-generated traces when debugging
  void print() const;

private:
  const IRUnit&       m_unit;
  CodeBlock&          m_mainCode;
  CodeBlock&          m_coldCode;
  CodeBlock&          m_frozenCode;
  Asm                 m_as;  // current "main" assembler
  Asm                 m_acold; // for cold code
  CodegenState&       m_state;
  Reg64               m_rScratch; // currently selected GP scratch reg
  IRInstruction*      m_curInst;  // current instruction being generated
  const RegAllocInfo::RegMap* m_instRegs; // registers for current m_curInst.
};

void emitFwdJmp(CodeBlock& cb, Block* target, CodegenState& state);

// Helpers to compute a reference to a TypedValue type and data
inline MemoryRef refTVType(PhysReg reg) {
  return reg[TVOFF(m_type)];
}

inline MemoryRef refTVData(PhysReg reg) {
  return reg[TVOFF(m_data)];
}

inline MemoryRef refTVType(MemoryRef ref) {
  return *(ref.r + TVOFF(m_type));
}

inline MemoryRef refTVData(MemoryRef ref) {
  return *(ref.r + TVOFF(m_data));
}

}}}

#endif
