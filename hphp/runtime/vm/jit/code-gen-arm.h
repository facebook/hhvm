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
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP { namespace JIT { namespace ARM {

struct CodeGenerator {

  CodeGenerator(const IRUnit& unit, CodeBlock& mainCode, CodeBlock& stubsCode,
                JIT::TranslatorX64* tx64, CodegenState& state)
      : m_unit(unit)
      , m_mainCode(mainCode)
      , m_stubsCode(stubsCode)
      , m_as(mainCode)
      , m_astubs(stubsCode)
      , m_tx64(tx64)
      , m_state(state)
      , m_curInst(nullptr)
    {
    }

  void cgBlock(Block* block, std::vector<TransBCMapping>* bcMap);

 private:
  template<class Then>
  void ifThen(vixl::MacroAssembler& a, vixl::Condition cc, Then thenBlock) {
    vixl::Label done;
    a.  B   (&done, InvertCondition(cc));
    thenBlock();
    a.  bind(&done);
  }

  template<class Then, class Else>
  void ifThenElse(vixl::MacroAssembler& a, vixl::Condition cc, Then thenBlock,
                  Else elseBlock) {
    vixl::Label elseLabel, done;
    a.  B   (&elseLabel, InvertCondition(cc));
    thenBlock();
    a.  B   (&done);
    a.  bind(&elseLabel);
    elseBlock();
    a.  bind(&done);
  }

  const Func* curFunc() { return m_curInst->marker().func; }

  void emitJumpToBlock(CodeBlock& cb, Block* target, ConditionCode cc);

  CallDest callDest(PhysReg reg0, PhysReg reg1 = InvalidReg) const;
  CallDest callDest(const IRInstruction*) const;
  CallDest callDestTV(const IRInstruction*) const;
  CallDest callDest2(const IRInstruction*) const;

  void cgCallNative(vixl::MacroAssembler& as, IRInstruction* inst);
  void cgCallHelper(vixl::MacroAssembler& a,
                    CppCall call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args,
                    RegSet toSave);
  void cgCallHelper(vixl::MacroAssembler& a,
                    CppCall call,
                    const CallDest& dstInfo,
                    SyncOptions sync,
                    ArgGroup& args);

  void emitDecRefDynamicType(vixl::Register baseReg, ptrdiff_t offset);
  void emitDecRefStaticType(Type type, vixl::Register reg);
  void emitDecRefMem(Type type, vixl::Register baseReg, ptrdiff_t offset);

  template<class Loc, class JmpFn>
  void emitTypeTest(Type type, vixl::Register typeReg, Loc dataSrc,
                    JmpFn doJcc);

  void emitLoadTypedValue(PhysLoc dst, vixl::Register base, ptrdiff_t offset,
                          Block* label);
  void emitStoreTypedValue(vixl::Register base, ptrdiff_t offset, PhysLoc src);
  void emitLoad(Type dstType, PhysLoc dstLoc, vixl::Register base,
                ptrdiff_t offset, Block* label = nullptr);
  void emitStore(vixl::Register base,
                 ptrdiff_t offset,
                 SSATmp* src, PhysLoc srcLoc,
                 bool genStoreType = true);

  Address cgInst(IRInstruction* inst);

  const PhysLoc srcLoc(unsigned i) const {
    return m_state.regs[m_curInst].src(i);
  }
  const PhysLoc dstLoc(unsigned i) const {
    return m_state.regs[m_curInst].dst(i);
  }
  ArgGroup argGroup() const {
    return ArgGroup(m_curInst, m_state.regs[m_curInst]);
  }

  void recordHostCallSyncPoint(vixl::MacroAssembler& as, JIT::TCA tca);
  void cgInterpOneCommon(IRInstruction* inst);

#define O(name, dsts, srcs, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef O

  const IRUnit&               m_unit;
  CodeBlock&                  m_mainCode;
  CodeBlock&                  m_stubsCode;
  vixl::MacroAssembler        m_as;
  vixl::MacroAssembler        m_astubs;
  TranslatorX64*              m_tx64;
  CodegenState&               m_state;
  IRInstruction*              m_curInst;
};

void patchJumps(CodeBlock& cb, CodegenState& state, Block* block);

}}}

#endif
