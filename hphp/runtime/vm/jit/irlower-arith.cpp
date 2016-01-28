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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

template<class vinst>
void implUnop(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const s = srcLoc(env, inst, 0).reg();
  auto const d = dstLoc(env, inst, 0).reg();

  v << vinst{s, d};
}

template<class vinst>
void implBinop(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();

  v << vinst{s1, s0, d};
}

template<class vinst>
Vreg implBinopSF(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const s0 = srcLoc(env, inst, 0).reg();
  auto const s1 = srcLoc(env, inst, 1).reg();
  auto const d  = dstLoc(env, inst, 0).reg();
  auto const sf = v.makeReg();

  v << vinst{s1, s0, d, sf};
  return sf;
}

template<class vinst>
void implArithO(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const sf = implBinopSF<vinst>(v, env, inst);
  v << jcc{CC_O, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

template<RoundDirection rd>
void implRound(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const s = srcLoc(env, inst, 0).reg();
  auto const d = dstLoc(env, inst, 0).reg();
  v << roundsd{rd, s, d};
}

template<class vinst, class vinsti>
void implShift(Vout& v, IRLS& env, const IRInstruction* inst) {
  auto const shift = inst->src(1);
  auto const s0 = srcLoc(env, inst, 0).reg();  // bytes to be shifted
  auto const s1 = srcLoc(env, inst, 1).reg();  // shift amount
  auto const d  = dstLoc(env, inst, 0).reg();
  auto const sf = v.makeReg();

  if (shift->hasConstVal()) {
    int n = shift->intVal() & 0x3f; // only use low 6 bits.
    v << vinsti{n, s0, d, sf};
  } else {
    v << vinst{s1, s0, d, sf};
  }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

#define ARITH_OPS               \
  AO(AddInt,  BinopSF,  addq)   \
  AO(SubInt,  BinopSF,  subq)   \
  AO(MulInt,  BinopSF,  imul)   \
  AO(AddIntO, ArithO,   addq)   \
  AO(SubIntO, ArithO,   subq)   \
  AO(MulIntO, ArithO,   imul)   \
  AO(AddDbl,  Binop,    addsd)  \
  AO(SubDbl,  Binop,    subsd)  \
  AO(MulDbl,  Binop,    mulsd)  \
  AO(AbsDbl,  Unop,     absdbl) \
  AO(Sqrt,    Unop,     sqrtsd) \
  AO(AndInt,  BinopSF,  andq)   \
  AO(OrInt,   BinopSF,  orq)    \
  AO(XorInt,  BinopSF,  xorq)   \
  AO(XorBool, BinopSF,  xorb)   \

#define AO(Inst, Impl, vinst)                           \
  void cg##Inst(IRLS& env, const IRInstruction* inst) { \
    impl##Impl<vinst>(vmain(env), env, inst);           \
  }
ARITH_OPS
#undef AO

#undef ARITH_OPS

///////////////////////////////////////////////////////////////////////////////

void cgFloor(IRLS& env, const IRInstruction* inst) {
  implRound<RoundDirection::floor>(vmain(env), env, inst);
}
void cgCeil(IRLS& env, const IRInstruction* inst) {
  implRound<RoundDirection::ceil>(vmain(env), env, inst);
}

void cgShl(IRLS& env, const IRInstruction* inst) {
  implShift<shl,shlqi>(vmain(env), env, inst);
}
void cgShr(IRLS& env, const IRInstruction* inst) {
  implShift<sar,sarqi>(vmain(env), env, inst);
}

void cgDivDbl(IRLS& env, const IRInstruction* inst) {
  auto const d = dstLoc(env, inst, 0).reg();
  auto const dividend = srcLoc(env, inst, 0).reg();
  auto const divisor  = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  v << divsd{divisor, dividend, d};
}

void cgDivInt(IRLS& env, const IRInstruction* inst) {
  auto const d = dstLoc(env, inst, 0).reg();
  auto const dividend = srcLoc(env, inst, 0).reg();
  auto const divisor  = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  v << divint{dividend, divisor, d};
}

void cgMod(IRLS& env, const IRInstruction* inst) {
  auto const d = dstLoc(env, inst, 0).reg();
  auto const dividend = srcLoc(env, inst, 0).reg();
  auto const divisor  = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  v << srem{dividend, divisor, d};
}

///////////////////////////////////////////////////////////////////////////////

}}}
