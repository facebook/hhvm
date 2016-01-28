/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct Vout;

namespace NativeCalls { struct CallInfo; }

///////////////////////////////////////////////////////////////////////////////

namespace irlower {

///////////////////////////////////////////////////////////////////////////////

struct CodeGenerator {
  explicit CodeGenerator(IRLS& state) : m_state(state) {}

  void cgInst(IRInstruction* inst);

private:
  Vloc srcLoc(const IRInstruction* inst, unsigned i) const;
  Vloc dstLoc(const IRInstruction* inst, unsigned i) const;
  ArgGroup argGroup(const IRInstruction* inst) const;

  // Autogenerate function declarations for each IR instruction in ir-opcode.h
#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  void cgCallNative(Vout& v, IRInstruction* inst);

  CallDest callDest(Vreg reg0) const;
  CallDest callDest(Vreg reg0, Vreg reg1) const;
  CallDest callDest(const IRInstruction*) const;
  CallDest callDestTV(const IRInstruction*) const;
  CallDest callDestDbl(const IRInstruction*) const;

  // Main call helper:
  void cgCallHelper(Vout& v, CallSpec call, const CallDest& dstInfo,
                    SyncOptions sync, const ArgGroup& args);
  void cgInterpOneCommon(IRInstruction* inst);

  void emitTrashTV(Vreg, int32_t, char fillByte);

  void emitLoad(SSATmp* dst, Vloc dstLoc, Vptr base);
  void emitLoadTypedValue(SSATmp* dst, Vloc dstLoc, Vptr ref);

  template <class JmpFn>
  void emitReffinessTest(IRInstruction* inst, Vreg sf, JmpFn doJcc);

  template<class Loc1, class Loc2, class JmpFn>
  void emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc, Vreg sf,
                    JmpFn doJcc);

  template<class DataLoc, class JmpFn>
  void emitSpecializedTypeTest(Type type, DataLoc data, Vreg sf, JmpFn doJcc);

  template<class Loc>
  void emitTypeCheck(Type type, Loc typeSrc,
                     Loc dataSrc, Block* taken);

  void emitVerifyCls(IRInstruction* inst);

  void emitGetCtxFwdCallWithThis(Vreg srcCtx, Vreg dstCtx, bool staticCallee);

  Vreg emitGetCtxFwdCallWithThisDyn(Vreg destCtxReg, Vreg thisReg,
                                    rds::Handle ch);

  void cgCoerceHelper(IRInstruction* inst, Vreg base, int offset,
                      Func const* callee, int argNum);
  void cgCastHelper(IRInstruction* inst, Vreg base, int offset);
  Vreg emitTestZero(Vout& v, SSATmp* src, Vloc srcLoc);
  template<class Inst>
  bool emitIncDec(Vout& v, Vloc dst, SSATmp* src0, Vloc loc0,
                  SSATmp* src1, Vloc loc1, Vreg& sf);

private:
  Vreg selectScratchReg(IRInstruction* inst);
  RegSet findFreeRegs(IRInstruction* inst);
  void emitSetCc(IRInstruction*, ConditionCode cc, Vreg sf);
  template<class JmpFn>
  void emitIsTypeTest(IRInstruction* inst, Vreg sf, JmpFn doJcc);
  void cgIsTypeCommon(IRInstruction* inst, bool negate);
  void cgIsTypeMemCommon(IRInstruction*, bool negate);
  Vreg emitInstanceBitmaskCheck(Vout& v, IRInstruction* inst);
  void emitInitObjProps(const IRInstruction* inst, Vreg dstReg,
                        const Class* cls, size_t nProps);

  void decRefImpl(Vout& v, const IRInstruction*, const OptDecRefProfile&, bool);
  float decRefDestroyRate(const IRInstruction* inst,
                          OptDecRefProfile& profile, Type type);
  void emitDecRefTypeStat(Vout& v, const IRInstruction*);

  void cgIterNextCommon(IRInstruction* inst);
  void cgIterInitCommon(IRInstruction* inst);
  void cgMIterNextCommon(IRInstruction* inst);
  void cgMIterInitCommon(IRInstruction* inst);
  Vreg cgLdFuncCachedCommon(const StringData* name, Vreg dst);
  void cgLookupCnsCommon(IRInstruction* inst);
  rds::Handle cgLdClsCachedCommon(Vout& v, IRInstruction* inst, Vreg dst,
                                  Vreg sf);
  void cgPropImpl(IRInstruction*);
  void cgIssetEmptyPropImpl(IRInstruction*);
  void cgElemImpl(IRInstruction*);
  void cgElemArrayImpl(IRInstruction*);
  void cgArraySetImpl(IRInstruction*);
  void cgIssetEmptyElemImpl(IRInstruction*);

  Vlabel label(Block*);
  void emitFwdJcc(Vout& v, ConditionCode cc, Vreg sf, Block* target);

  static const Func* getFunc(const BCMarker& marker) {
    return marker.func();
  };
  static const Class* getClass(const BCMarker& marker) {
    return getFunc(marker)->cls();
  }
  static const Unit* getUnit(const BCMarker& marker) {
    return getFunc(marker)->unit();
  }
  static bool resumed(const BCMarker& marker) {
    return marker.resumed();
  };

  Fixup makeFixup(const BCMarker& marker,
                  SyncOptions sync = SyncOptions::Sync);
  int iterOffset(const BCMarker& marker, uint32_t id);

  void emitConvBoolOrIntToDbl(IRInstruction* inst);
  void cgLdClsMethodCacheCommon(IRInstruction* inst, Offset offset);
  void emitLdRaw(IRInstruction* inst, size_t extraOff);
  void emitStRaw(IRInstruction* inst, size_t offset, int size);
  void resumableStResumeImpl(IRInstruction*, ptrdiff_t, ptrdiff_t);

  Vout& vmain() { assert(m_state.vmain); return *m_state.vmain; }
  Vout& vcold() { assert(m_state.vcold); return *m_state.vcold; }

private:
  IRLS& m_state;
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

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
