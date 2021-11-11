/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/code-gen-helpers.h"

#include "hphp/runtime/base/memory-manager.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {
///////////////////////////////////////////////////////////////////////////////

int64_t allocInitROM(uint8_t* source, size_t actualSize) {
  auto const index = MemoryManager::size2Index(actualSize);
  auto const allocedSize = MemoryManager::sizeIndex2Size(index);
  auto const overAlloced = allocedSize - actualSize;
  auto const buf = (uint8_t*)tl_heap->mallocSmallIndexSize(index, allocedSize);
  memcpy16_inline(buf, source, actualSize);
  if (overAlloced) {
    tl_heap->freeOveralloc(buf + actualSize, overAlloced);
  }
  return reinterpret_cast<int64_t>(buf);
}

///////////////////////////////////////////////////////////////////////////////
}

void cgAllocInitROM(IRLS& env, const IRInstruction* inst) {
  // TODO(michaelofarrell): burn more info in.
  auto& v = vmain(env);
  auto const extra = inst->extra<AllocInitROM>();
  cgCallHelper(v, env, CallSpec::direct(allocInitROM),
               callDest(env, inst), SyncOptions::None,
               argGroup(env, inst).imm(extra->rom).imm(extra->size));
}

void cgStValAt(IRLS& env, const IRInstruction* inst) {
  auto const dst = srcLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 1);
  auto const srcType = inst->src(1)->type();

  auto& v = vmain(env);
  storeTVVal(v, srcType, src, *dst);
}

void cgStTypeAt(IRLS& env, const IRInstruction* inst) {
  auto const dst = srcLoc(env, inst, 0).reg();
  auto const src = srcLoc(env, inst, 1);
  auto const srcType = inst->src(1)->type();

  auto& v = vmain(env);
  storeTVType(v, srcType, src, *dst);
}

void cgIntAsDataType(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0);
  auto const dst = dstLoc(env, inst, 0);

  auto& v = vmain(env);

  assertx(inst->hasTypeParam());
  auto const type = inst->typeParam();
  assertx(!type.needsReg());
  auto const typeReg = v.cns(type.toDataType());

  if (dst.numAllocated() == 2) {
    v << copyargs {
      v.makeTuple({src.reg(0), typeReg}),
      v.makeTuple({dst.reg(0), dst.reg(1)}),
    };
  } else if (dst.isFullSIMD()) {
    pack2(v, src.reg(0), typeReg, dst.reg(0));
  } else {
    v << copy {src.reg(0), dst.reg(0)};
  }
}


///////////////////////////////////////////////////////////////////////////////
}}}
