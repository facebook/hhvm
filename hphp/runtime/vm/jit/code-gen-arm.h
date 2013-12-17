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
#ifndef incl_HPHP_JIT_CODE_GEN_ARM_H
#define incl_HPHP_JIT_CODE_GEN_ARM_H

#include "hphp/vixl/a64/macro-assembler-a64.h"

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

  void cgBlock(Block* block, vector<TransBCMapping>* bcMap);

 private:
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

  template<class Loc, class JmpFn>
  void emitTypeTest(Type type, Loc typeSrc, Loc dataSrc, JmpFn doJcc);

  void emitLoadTypedValue(SSATmp* dst, vixl::Register base, ptrdiff_t offset,
                          Block* label);
  void emitLoad(SSATmp* dst, vixl::Register base, ptrdiff_t offset,
                Block* label = nullptr);

  Address cgInst(IRInstruction* inst);

  const PhysLoc curOpd(const SSATmp* t) const {
    return m_state.regs[m_curInst][t];
  }
  const PhysLoc curOpd(const SSATmp& t) const {
    return curOpd(&t);
  }
  const RegAllocInfo::RegMap& curOpds() const {
    return m_state.regs[m_curInst];
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

}}}

#endif
