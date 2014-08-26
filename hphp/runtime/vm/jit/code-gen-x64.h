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
#include "hphp/runtime/vm/jit/vasm-x64.h"

namespace HPHP { namespace jit { namespace x64 {

// Cache alignment is required for mutable instructions to make sure
// mutations don't "tear" on remote cpus.
constexpr size_t kCacheLineSize = 64;
constexpr size_t kCacheLineMask = kCacheLineSize - 1;

struct CodeGenerator : public jit::CodeGenerator {
  typedef jit::X64Assembler Asm;

  CodeGenerator(const IRUnit& unit, Vout& main, Vout& cold, Vout& frozen,
                CodegenState& state)
    : m_unit(unit)
    , m_vmain(main)
    , m_vcold(cold)
    , m_vfrozen(frozen)
    , m_state(state)
    , m_curInst(nullptr)
  {
  }

  virtual ~CodeGenerator() {
  }

  void cgInst(IRInstruction* inst) override;

private:
  Vloc srcLoc(unsigned i) const;
  Vloc dstLoc(unsigned i) const;
  ArgGroup argGroup() const;

  // Autogenerate function declarations for each IR instruction in ir.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  void cgCallNative(Vout&, IRInstruction* inst);

  CallDest callDest(Vreg reg0) const;
  CallDest callDest(Vreg reg0, Vreg reg1) const;
  CallDest callDest(const IRInstruction*) const;
  CallDest callDestTV(const IRInstruction*) const;
  CallDest callDestDbl(const IRInstruction*) const;

  // Main call helper:
  void cgCallHelper(Vout&, CppCall call, const CallDest& dstInfo,
                    SyncOptions sync, ArgGroup& args, RegSet toSave);
  // Overload to make the toSave RegSet optional:
  void cgCallHelper(Vout&, CppCall call, const CallDest& dstInfo,
                    SyncOptions sync, ArgGroup& args);
  void cgInterpOneCommon(IRInstruction* inst);

  enum class Width { Value, Full };
  void cgStore(Vptr dst, SSATmp* src, Vloc src_loc, Width);
  void cgStoreTypedValue(Vptr dst, SSATmp* src, Vloc src_loc);

  // helpers to load a value in dst. When label is not null a type check
  // is performed against value to ensure it is of the type expected by dst
  void cgLoad(SSATmp* dst, Vloc dstLoc, Vptr base,
              Block* label = nullptr);
  void cgLoadTypedValue(SSATmp* dst, Vloc dstLoc, Vptr ref,
                        Block* label = nullptr);

  template<class Loc1, class Loc2, class JmpFn>
  void emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc, JmpFn doJcc);

  template<class DataLoc, class JmpFn>
  void emitSpecializedTypeTest(Type type, DataLoc data, JmpFn doJcc);

  template<class Loc>
  void emitTypeCheck(Type type, Loc typeSrc, Loc dataSrc, Block* taken);
  template<class Loc>
  void emitTypeGuard(Type type, Loc typeLoc, Loc dataLoc);

  void cgIncRefWork(Type type, SSATmp* src, Vloc srcLoc);
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck);

  template<class OpInstr>
  void cgUnaryIntOp(Vloc dst, SSATmp* src, Vloc src_loc);

  enum Commutativity { Commutative, NonCommutative };

  void cgRoundCommon(IRInstruction* inst, RoundDirection dir);

  template<class Op, class Opi>
  void cgBinaryIntOp(IRInstruction*);
  template<class Emit> void cgBinaryDblOp(IRInstruction*, Emit);
  template<class Op, class Opi> void cgShiftCommon(IRInstruction*);

  void emitVerifyCls(IRInstruction* inst);

  void emitGetCtxFwdCallWithThis(Vreg ctxReg, bool staticCallee);

  void emitGetCtxFwdCallWithThisDyn(Vreg destCtxReg, Vreg thisReg,
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
  void emitReqBindJcc(Vout&, ConditionCode cc, const ReqBindJccData*);

  void emitCompare(Vout&, IRInstruction* inst);
  void emitCompareInt(Vout&, IRInstruction* inst);
  void emitTestZero(Vout&, SSATmp*, Vloc);
  template<class Inst>
  bool emitIncDec(Vloc dst, SSATmp* src0, Vloc loc0,
                  SSATmp* src1, Vloc loc1);

private:
  Vreg selectScratchReg(IRInstruction* inst);
  RegSet findFreeRegs(IRInstruction* inst);
  VregXMM prepXMM(Vout&, const SSATmp* src, Vloc srcLoc);
  void emitSetCc(IRInstruction*, ConditionCode);
  template<class JmpFn>
  void emitIsTypeTest(IRInstruction* inst, JmpFn doJcc);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  void emitInstanceBitmaskCheck(Vout&, IRInstruction*);
  void emitTraceRet(Vout&);
  void emitInitObjProps(Vreg dstReg, const Class* cls, size_t nProps);

  bool decRefDestroyIsUnlikely(OptDecRefProfile& profile, Type type);
  template <typename F>
  void cgCheckStaticBitAndDecRef(Vout&, Vlabel done, Type type,
                                 Vreg dataReg, F destroyImpl);
  void cgCheckStaticBitAndDecRef(Vout&, Vlabel done, Type type,
                                 Vreg dataReg);
  void cgCheckRefCountedType(Vreg typeReg, Vlabel done);
  void cgCheckRefCountedType(Vreg baseReg, int64_t offset, Vlabel done);
  void cgDecRefStaticType(Vout&, Type type, Vreg dataReg, bool genZeroCheck);
  void cgDecRefDynamicType(Vreg typeReg, Vreg dataReg, bool genZeroCheck);
  void cgDecRefDynamicTypeMem(Vreg baseReg, int64_t offset);
  void cgDecRefMem(Type type, Vreg baseReg, int64_t offset);

  void cgIterNextCommon(IRInstruction* inst);
  void cgIterInitCommon(IRInstruction* inst);
  void cgMIterNextCommon(IRInstruction* inst);
  void cgMIterInitCommon(IRInstruction* inst);
  void cgLdFuncCachedCommon(IRInstruction* inst);
  void cgLookupCnsCommon(IRInstruction* inst);
  RDS::Handle cgLdClsCachedCommon(IRInstruction* inst);
  Vlabel label(Block*);
  void emitFwdJcc(Vout&, ConditionCode cc, Block* target);
  const Func* curFunc() const { return m_curInst->marker().func(); };
  const Class* curClass() const { return curFunc()->cls(); }
  const Unit* curUnit() const { return curFunc()->unit(); }
  bool resumed() const { return m_curInst->marker().resumed(); };
  void recordSyncPoint(Vout&, SyncOptions sync = SyncOptions::kSyncPoint);
  int iterOffset(SSATmp* tmp) { return iterOffset(tmp->intVal()); }
  int iterOffset(uint32_t id);

  void emitAdjustSp(Vreg spReg, Vreg dstReg, int adjustment);
  void emitConvBoolOrIntToDbl(IRInstruction* inst);
  void cgLdClsMethodCacheCommon(IRInstruction* inst, Offset offset);
  void emitLdRaw(IRInstruction* inst, size_t extraOff);
  void emitStRaw(IRInstruction* inst, size_t offset, int size);

  /*
   * Execute the code emitted by 'taken' only if the given condition code is
   * true.
   */
  template <class Block>
  void ifBlock(Vout& v, Vout& vcold, ConditionCode cc, Block taken,
             bool unlikely = false);

  /*
   * Generate an if-block that branches around some unlikely code, handling
   * the cases when a == acold and a != acold.  cc is the branch condition
   * to run the unlikely block.
   *
   * Passes the proper assembler to use to the unlikely function.
   */
  template <class Then>
  void unlikelyIfBlock(Vout& v, Vout& vcold, ConditionCode cc, Then then);

  // Generate an if-then-else block
  template <class Then, class Else>
  void ifThenElse(Vout& v, ConditionCode cc, Then thenBlock, Else elseBlock);

  // Generate an if-then-else block into m_as.
  template <class Then, class Else>
  void ifThenElse(Vout& v, Vout& vcold, ConditionCode cc, Then thenBlock,
                  Else elseBlock, bool unlikely = false);

  /*
   * Same as ifThenElse except the first block is off in acold
   */
  template <class Then, class Else>
  void unlikelyIfThenElse(Vout& v, Vout& vcold, ConditionCode cc,
                          Then unlikelyBlock, Else elseBlock);

  // This is for printing partially-generated traces when debugging
  void print() const;

  Vout& vmain() { return m_vmain; }
  Vout& vcold() { return m_vcold; }
  Vout& vfrozen() { return m_vfrozen; }

private:
  const IRUnit&       m_unit;
  Vout&               m_vmain;
  Vout&               m_vcold;
  Vout&               m_vfrozen;
  CodegenState&       m_state;
  IRInstruction*      m_curInst;  // current instruction being generated
  jit::vector<Vloc>   m_slocs, m_dlocs;
};

// Helpers to compute a reference to a TypedValue type and data
inline Vptr refTVType(Vreg reg) {
  return reg[TVOFF(m_type)];
}

inline Vptr refTVData(Vreg reg) {
  return reg[TVOFF(m_data)];
}

inline Vptr refTVType(Vptr ref) {
  return ref + TVOFF(m_type);
}

inline Vptr refTVData(Vptr ref) {
  return ref + TVOFF(m_data);
}

}}}

#endif
