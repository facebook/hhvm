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

#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irlower.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

inline Vout& vmain(IRLS& env) { assert(env.vmain); return *env.vmain; }
inline Vout& vcold(IRLS& env) { assert(env.vcold); return *env.vcold; }

inline Vlabel label(IRLS& env, Block* b) { return env.labels[b]; }

inline Vloc srcLoc(IRLS& env, const IRInstruction* inst, unsigned i) {
  return env.locs[inst->src(i)];
}

inline Vloc dstLoc(IRLS& env, const IRInstruction* inst, unsigned i) {
  return env.locs[inst->dst(i)];
}

inline ArgGroup argGroup(IRLS& env, const IRInstruction* inst) {
  return ArgGroup(inst, env.locs);
}

inline CallDest callDest(Vreg reg0) {
  return { DestType::SSA, reg0 };
}

inline CallDest callDest(Vreg reg0, Vreg reg1) {
  return { DestType::TV, reg0, reg1 };
}

inline CallDest callDest(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;

  auto const loc = dstLoc(env, inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;
  assertx(loc.numAllocated() == 1);

  return {
    inst->dst(0)->isA(TBool) ? DestType::Byte : DestType::SSA,
    loc.reg(0)
  };
}

inline CallDest callDestTV(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;

  auto const loc = dstLoc(env, inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;

  if (loc.isFullSIMD()) {
    assertx(loc.numAllocated() == 1);
    return { DestType::SIMD, loc.reg(0) };
  }
  if (loc.numAllocated() == 2) {
    return { DestType::TV, loc.reg(0), loc.reg(1) };
  }
  assertx(loc.numAllocated() == 1);

  // Sometimes we statically know the type and only need the value.
  return { DestType::TV, loc.reg(0), InvalidReg };
}

inline CallDest callDestDbl(IRLS& env, const IRInstruction* inst) {
  if (!inst->numDsts()) return kVoidDest;
  auto const loc = dstLoc(env, inst, 0);
  return { DestType::Dbl, loc.reg(0) };
}

///////////////////////////////////////////////////////////////////////////////

inline void fwdJcc(Vout& v, IRLS& env, ConditionCode cc,
                   Vreg sf, Block* target) {
  auto const next = v.makeBlock();
  v << jcc{cc, sf, {next, label(env, target)}};
  v = next;
}

///////////////////////////////////////////////////////////////////////////////

}}}
