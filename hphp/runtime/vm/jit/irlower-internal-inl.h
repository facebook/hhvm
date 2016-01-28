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

///////////////////////////////////////////////////////////////////////////////

inline void fwdJcc(Vout& v, IRLS& env, ConditionCode cc,
                   Vreg sf, Block* target) {
  auto const next = v.makeBlock();
  v << jcc{cc, sf, {next, label(env, target)}};
  v = next;
}

///////////////////////////////////////////////////////////////////////////////

}}}
