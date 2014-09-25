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
#ifndef incl_HPHP_JIT_CODE_GEN_ARM_H
#define incl_HPHP_JIT_CODE_GEN_ARM_H

#include "hphp/vixl/a64/macro-assembler-a64.h"
#include <vector>

#include "hphp/util/data-block.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen.h"

namespace HPHP { namespace jit { namespace arm {

struct CodeGenerator : public jit::CodeGenerator {

  CodeGenerator(const IRUnit& unit, Vout& main, Vout& cold, Vout& frozen,
                CodegenState& state)
    : m_unit(unit)
    , m_vmain(main)
    , m_vcold(cold)
    , m_vfrozen(frozen)
    , m_state(state)
  {}

  virtual ~CodeGenerator() {}
  void cgInst(IRInstruction* inst) override;

 private:
  const Func* curFunc() const { return m_curInst->marker().func(); }
  bool resumed() const { return m_curInst->marker().resumed(); }

  void emitCompareInt(IRInstruction* inst);
  void emitCompareIntAndSet(IRInstruction* inst, ConditionCode cond);

  CallDest callDest(PhysReg reg0, PhysReg reg1 = InvalidReg) const;
  CallDest callDest(const IRInstruction*) const;
  CallDest callDestTV(const IRInstruction*) const;
  CallDest callDestDbl(const IRInstruction*) const;

  void cgCallNative(Vout&, IRInstruction* inst);
  void cgCallHelper(Vout&,
                    CppCall call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args,
                    RegSet toSave);
  void cgCallHelper(Vout&,
                    CppCall call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args);

  void emitDecRefDynamicType(Vout& v, Vreg base, int offset);

  void cgStLocWork(IRInstruction* inst);

  void emitDecRefStaticType(Vout&, Type type, Vreg data);
  void emitDecRefMem(Vout&, Type type, Vreg base, int offset);

  template <class JmpFn>
  void emitReffinessTest(IRInstruction* inst, JmpFn doJcc);

  template<class Loc, class JmpFn>
  void emitTypeTest(Vout& v, Type type, Vreg typeReg, Loc dataSrc, JmpFn doJcc);

  void emitLoadTypedValue(Vout&, Vloc dst, Vreg base, ptrdiff_t offset,
                          Block* label);
  void emitStoreTypedValue(Vout&, Vreg base, ptrdiff_t offset, Vloc src);
  void emitLoad(Vout&, Type type, Vloc dst, Vreg base,
                ptrdiff_t offset, Block* label = nullptr);
  void emitStore(Vout&, Vreg base, ptrdiff_t offset,
                 SSATmp* src, Vloc srcLoc,
                 bool genStoreType = true);
  void emitLdRaw(IRInstruction* inst, size_t extraOff);

  Vloc srcLoc(unsigned i) const;
  Vloc dstLoc(unsigned i) const;
  ArgGroup argGroup() const;
  Vlabel label(Block*);

  void recordHostCallSyncPoint(Vout&, Vpoint p);
  void cgInterpOneCommon(IRInstruction* inst);

#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  Vout& vmain() { return m_vmain; }
  Vout& vcold() { return m_vcold; }
  Vout& vfrozen() { return m_vfrozen; }

 private:
  const IRUnit&         m_unit;
  Vout&                 m_vmain;
  Vout&                 m_vcold;
  Vout&                 m_vfrozen;
  CodegenState&         m_state;
  IRInstruction*        m_curInst{nullptr};
  jit::vector<Vloc>     m_slocs, m_dlocs;
};

void emitJumpToBlock(CodeBlock& cb, Block* target, ConditionCode cc,
                     CodegenState& state);

}}}

#endif
