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
#include "hphp/runtime/vm/jit/target-cache.h"
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

  void cgCallHelper(Vout& v, CallSpec call, const CallDest& dstInfo,
                    SyncOptions sync, const ArgGroup& args);
  void cgInterpOneCommon(IRInstruction* inst);

  template <class JmpFn>
  void emitReffinessTest(IRInstruction* inst, Vreg sf, JmpFn doJcc);

  void emitVerifyCls(IRInstruction* inst);

  void cgCoerceHelper(IRInstruction* inst, Vreg base, int offset,
                      Func const* callee, int argNum);
  void cgCastHelper(IRInstruction* inst, Vreg base, int offset);
  Vreg emitTestZero(Vout& v, SSATmp* src, Vloc srcLoc);
  template<class Inst>
  bool emitIncDec(Vout& v, Vloc dst, SSATmp* src0, Vloc loc0,
                  SSATmp* src1, Vloc loc1, Vreg& sf);

  Vptr emitPackedLayoutAddr(SSATmp* idx, Vloc idxLoc, Vloc arrLoc);

private:
  Vreg selectScratchReg(IRInstruction* inst);
  RegSet findFreeRegs(IRInstruction* inst);
  void emitInitObjProps(const IRInstruction* inst, Vreg dstReg,
                        const Class* cls, size_t nProps);

  void cgPropImpl(IRInstruction*);
  void cgIssetEmptyPropImpl(IRInstruction*);
  void cgElemImpl(IRInstruction*);
  void cgElemArrayImpl(IRInstruction*);
  void cgArraySetImpl(IRInstruction*);
  void cgIssetEmptyElemImpl(IRInstruction*);

  void arrayLikeCountImpl(IRInstruction*);

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

  int iterOffset(const BCMarker& marker, uint32_t id);

  void emitConvBoolOrIntToDbl(IRInstruction* inst);
  void emitLdRaw(IRInstruction* inst, size_t extraOff);
  void emitStRaw(IRInstruction* inst, size_t offset, int size);

  Vout& vmain() { assert(m_state.vmain); return *m_state.vmain; }
  Vout& vcold() { assert(m_state.vcold); return *m_state.vcold; }

private:
  IRLS& m_state;
};

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
