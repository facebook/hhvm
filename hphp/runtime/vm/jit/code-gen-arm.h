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
                Transl::TranslatorX64* tx64, CodegenState& state)
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
  const Func* curFunc() { return m_curInst->marker().func; }

  void emitRegGetsRegPlusImm(vixl::MacroAssembler& as,
                             vixl::Register dstReg,
                             vixl::Register srcReg,
                             int64_t imm);
  template<class Loc, class JmpFn>
  void emitTypeGuard(Type type, Loc typeSrc, Loc dataSrc, JmpFn doJcc);

  Address cgInst(IRInstruction* inst);

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
