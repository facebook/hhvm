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

#include "hphp/runtime/base/string-data.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdStrLen(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sd = srcLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{sd[StringData::sizeOff()], dst};
}

void cgOrdStr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sd = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

#ifdef NO_M_DATA
  v << loadzbq{sd[sizeof(StringData)], dst};
#else
  auto const data = v.makeReg();
  v << load{sd[StringData::dataOff()], data};
  v << loadzbq{data[0], dst};
#endif
}

void cgOrdStrIdx(IRLS& env, const IRInstruction* inst) {
  auto const sd = srcLoc(env, inst, 0).reg();
  auto const idx = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  auto const length = v.makeReg();

  v << loadzlq{sd[StringData::sizeOff()], length};
  v << cmpq{idx, length, sf};

  unlikelyCond(v, vcold(env), CC_B, sf, dstLoc(env, inst, 0).reg(),
    [&] (Vout& v) {
      auto const args = argGroup(env, inst).ssa(0).ssa(1);
      cgCallHelper(v, env, CallSpec::direct(MInstrHelpers::stringGetI),
                   kVoidDest, SyncOptions::Sync, args);
      return v.cns(0);
    },
    [&] (Vout& v) {
      auto const dst = v.makeReg();
      auto const data = v.makeReg();
#ifdef NO_M_DATA
      v << lea{sd[sizeof(StringData)], data};
#else
      v << load{sd[StringData::dataOff()], data};
#endif
      v << loadzbq{data[idx], dst};
      return dst;
    }
  );
}

void cgChrInt(IRLS& env, const IRInstruction* inst) {
  auto const ppcs = reinterpret_cast<uintptr_t>(precomputed_chars);

  auto& v = vmain(env);

  auto const idx = [&] () -> Vreg {
    auto const srcReg = srcLoc(env, inst, 0).reg();
    if (inst->src(0)->inst()->is(OrdStr, OrdStrIdx)) {
      return srcReg;
    }
    auto const r = v.makeReg();
    v << andqi{255, srcReg, r, v.makeReg()};
    return r;
  }();

  if (ppcs <= std::numeric_limits<int>::max()) {
    v << load{baseless(idx * 8 + ppcs), dstLoc(env, inst, 0).reg()};
  } else {
    auto const pcs = [&] () -> Vreg {
      auto const ipcs = reinterpret_cast<uintptr_t>(&precomputed_chars);
      if (ipcs <= std::numeric_limits<int>::max()) {
        auto const r = v.makeReg();
        v << load{baseless(ipcs), r};
        return r;
      } else {
        return v.cns(ppcs);
      }
    }();
    v << load{pcs[idx * 8], dstLoc(env, inst, 0).reg()};
  }
}

void cgStringIsset(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sd = srcLoc(env, inst, 0).reg();
  auto const idx = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const length = v.makeReg();
  auto const sf = v.makeReg();
  v << loadzlq{sd[StringData::sizeOff()], length};
  v << cmpq{idx, length, sf};
  v << setcc{CC_NBE, sf, dst};
}

IMPL_OPCODE_CALL(ConcatStrStr);
IMPL_OPCODE_CALL(ConcatStrInt);
IMPL_OPCODE_CALL(ConcatIntStr);
IMPL_OPCODE_CALL(ConcatStr3);
IMPL_OPCODE_CALL(ConcatStr4);

///////////////////////////////////////////////////////////////////////////////

}}}
