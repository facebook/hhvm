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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/util/struct-log.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////
// Basic logging.

auto const s_ArrayIterProfile = makeStaticString("ArrayIterProfile");

void logArrayIterProfile(IRGS& env) {
  // We generate code for thousands of loops each time we call retranslateAll.
  // We don't want production webservers to log when they do so.
  if (!StructuredLog::coinflip(RuntimeOption::EvalArrayIterLogRate)) return;

  auto const marker  = makeMarker(env, bcOff(env));
  auto const profile = TargetProfile<ArrayIterProfile>(
    env.unit,
    makeMarker(env, bcOff(env)),
    s_ArrayIterProfile
  );
  if (!profile.optimizing()) return;
  if (env.inlineState.conjure) return;

  auto func_str = "(unknown)";
  auto file_str = "(unknown)";
  auto line_int = 0;
  if (marker.hasFunc()) {
    auto const func = marker.func();
    func_str = func->fullName()->data();
    file_str = func->filename()->data();
    line_int = func->unit()->getLineNumber(marker.bcOff());
  }

  std::vector<std::string> inline_state_string;
  std::vector<folly::StringPiece> inline_state;
  for (auto const& state : env.inlineState.bcStateStack) {
    inline_state_string.push_back(show(state));
    inline_state.push_back(inline_state_string.back());
  }

  StructuredLogEntry entry;
  entry.setStr("marker", marker.show());
  entry.setStr("profile", profile.data().toString());
  entry.setStr("source_func", func_str);
  entry.setStr("source_file", file_str);
  entry.setInt("source_line", line_int);
  entry.setInt("prof_count", curProfCount(env));
  entry.setInt("inline_depth", env.inlineState.depth);
  entry.setVec("inline_state", inline_state);

  StructuredLog::log("hhvm_array_iterators", entry);
}

//////////////////////////////////////////////////////////////////////
// Generic implementations.

// We only JIT IterNext bytecodes if FrameState knows that their output locals
// are cells. HHBBC almost always makes these assertions in repo mode.
bool areLocalsCells(IRGS& env, const IterData& data) {
  auto const cell = [&](uint32_t local) {
    if (local == uint32_t(-1)) return true;
    return env.irb->fs().local(local).type <= TCell;
  };
  return cell(data.valId) && cell(data.keyId);
}

// `relOffset` is the offset from the IterInit to the end of the loop.
// `result` is a TBool that is true if the iterator has more items.
void implIterInitJmp(IRGS& env, Offset relOffset, SSATmp* result) {
  auto const targetOffset = bcOff(env) + relOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  gen(env, JmpZero, target, result);
}

// `relOffset` is the offset from the IterNext to the beginning of the loop.
// `result` is a TBool that is true if the iterator has more items.
void implIterNextJmp(IRGS& env, Offset relOffset, SSATmp* result) {
  auto const targetOffset = bcOff(env) + relOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);

  if (relOffset <= 0) {
    ifThen(env,
      [&](Block* taken) {
        gen(env, JmpNZero, taken, result);
      },
      [&]{
        surpriseCheckWithTarget(env, targetOffset);
        gen(env, Jmp, target);
      }
    );
  } else {
    gen(env, JmpNZero, target, result);
  }
}

// If the iterator base is an empty array-like, this method will generate
// trivial IR for the loop (just jump directly to done) and return true.
bool iterInitEmptyBase(IRGS& env, Offset relOffset, SSATmp* base) {
  auto const empty = base->hasConstVal(TArrLike) && base->arrLikeVal()->empty();
  if (!empty) return false;

  // NOTE: `base` is static, so we can skip the dec-ref for non-local iters.
  auto const targetOffset = bcOff(env) + relOffset;
  auto const target = getBlock(env, targetOffset);
  assertx(target != nullptr);
  gen(env, Jmp, target);
  return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////

void emitIterInit(IRGS& env, int32_t iterId, Offset relOffset,
                  int32_t valLocalId) {
  auto const base = popC(env);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(IterInit);
  if (iterInitEmptyBase(env, relOffset, base)) return;
  if (base->isA(TArrLike)) logArrayIterProfile(env);

  auto const type = IterTypeOp::NonLocal;
  auto const data = IterInitData(iterId, uint32_t(-1), valLocalId, type);
  auto const result = gen(env, IterInit, data, base, fp(env));
  implIterInitJmp(env, relOffset, result);
}

void emitIterInitK(IRGS& env, int32_t iterId, Offset relOffset,
                   int32_t valLocalId, int32_t keyLocalId) {
  auto const base = popC(env);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(IterInitK);
  if (iterInitEmptyBase(env, relOffset, base)) return;
  if (base->isA(TArrLike)) logArrayIterProfile(env);

  auto const type = IterTypeOp::NonLocal;
  auto const data = IterInitData(iterId, keyLocalId, valLocalId, type);
  auto const result = gen(env, IterInitK, data, base, fp(env));
  implIterInitJmp(env, relOffset, result);
}

void emitIterNext(IRGS& env,
                  int32_t iterId,
                  Offset relOffset,
                  int32_t valLocalId) {
  auto const data = IterData(iterId, uint32_t(-1), valLocalId);
  if (!areLocalsCells(env, data)) PUNT(IterNext-refs);

  auto const result = gen(env, IterNext, data, fp(env));
  implIterNextJmp(env, relOffset, result);
}

void emitIterNextK(IRGS& env,
                   int32_t iterId,
                   Offset relOffset,
                   int32_t valLocalId,
                   int32_t keyLocalId) {
  auto const data = IterData(iterId, keyLocalId, valLocalId);
  if (!areLocalsCells(env, data)) PUNT(IterNextK-refs);

  auto const result = gen(env, IterNextK, data, fp(env));
  implIterNextJmp(env, relOffset, result);
}

void emitLIterInit(IRGS& env,
                   int32_t iterId,
                   int32_t baseLocalId,
                   Offset relOffset,
                   int32_t valLocalId,
                   IterTypeOp subop) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterInit-pseudomain);
  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(LIterInit);
  if (iterInitEmptyBase(env, relOffset, base)) return;
  if (base->isA(TArrLike)) logArrayIterProfile(env);

  if (base->isA(TObj)) gen(env, IncRef, base);
  auto const data = IterInitData(iterId, uint32_t(-1), valLocalId, subop);
  auto const op = base->isA(TArrLike) ? LIterInit : IterInit;
  auto const result = gen(env, op, data, base, fp(env));
  implIterInitJmp(env, relOffset, result);
}

void emitLIterInitK(IRGS& env,
                    int32_t iterId,
                    int32_t baseLocalId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId,
                    IterTypeOp subop) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterInitK-pseudomain);
  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  if (!base->type().subtypeOfAny(TArrLike, TObj)) PUNT(LIterInitK);
  if (iterInitEmptyBase(env, relOffset, base)) return;
  if (base->isA(TArrLike)) logArrayIterProfile(env);

  if (base->isA(TObj)) gen(env, IncRef, base);
  auto const data = IterInitData(iterId, keyLocalId, valLocalId, subop);
  auto const op = base->isA(TArrLike) ? LIterInitK : IterInitK;
  auto const result = gen(env, op, data, base, fp(env));
  implIterInitJmp(env, relOffset, result);
}

void emitLIterNext(IRGS& env,
                   int32_t iterId,
                   int32_t baseLocalId,
                   Offset relOffset,
                   int32_t valLocalId,
                   IterTypeOp subop) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNext-pseudomain);
  auto const data = IterData(iterId, uint32_t(-1), valLocalId);
  if (!areLocalsCells(env, data)) PUNT(LIterNext-refs);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  auto const result = base->isA(TArrLike)
    ? gen(env, LIterNext, data, base, fp(env))
    : gen(env, IterNext, data, fp(env));
  implIterNextJmp(env, relOffset, result);
}

void emitLIterNextK(IRGS& env,
                    int32_t iterId,
                    int32_t baseLocalId,
                    Offset relOffset,
                    int32_t valLocalId,
                    int32_t keyLocalId,
                    IterTypeOp subop) {
  if (curFunc(env)->isPseudoMain()) PUNT(LIterNextK-pseudomain);
  auto const data = IterData(iterId, keyLocalId, valLocalId);
  if (!areLocalsCells(env, data)) PUNT(LIterNextK-refs);

  auto const base = ldLoc(env, baseLocalId, nullptr, DataTypeSpecific);
  auto const result = base->isA(TArrLike)
    ? gen(env, LIterNextK, data, base, fp(env))
    : gen(env, IterNextK, data, fp(env));
  implIterNextJmp(env, relOffset, result);
}

void emitIterFree(IRGS& env, int32_t iterId) {
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitLIterFree(IRGS& env, int32_t iterId, int32_t baseLocalId) {
  auto const baseType = env.irb->local(baseLocalId, DataTypeSpecific).type;
  // If the base is a known array, there's nothing to actually free in the
  // iterator. However, in debug builds we get some extra sanity checking if we
  // call the free function anyways.
  if (baseType <= TArrLike && !debug) return;
  gen(env, IterFree, IterId(iterId), fp(env));
}

void emitIterBreak(IRGS& env, Offset relOffset, const IterTable& it) {
  for (auto const& ent : it) {
    switch (ent.kind) {
    case KindOfIter:  emitIterFree(env, ent.id);  break;
    case KindOfLIter: emitLIterFree(env, ent.id, ent.local); break;
    }
  }
  jmpImpl(env, bcOff(env) + relOffset);
}

//////////////////////////////////////////////////////////////////////

}}}
