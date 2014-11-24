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
#include "hphp/runtime/vm/jit/hhbc-translator.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * This function returns the offset of instruction i's branch target.
 * This is normally the offset corresponding to the branch being
 * taken.  However, if i does not break a trace and it's followed in
 * the trace by the instruction in the taken branch, then this
 * function returns the offset of the i's fall-through instruction.
 * In that case, the invertCond output argument is set to true;
 * otherwise it's set to false.
 */
Offset iterBranchTarget(const NormalizedInstruction& i, bool& invertCond) {
  assert(instrJumpOffset(reinterpret_cast<const Op*>(i.pc())) != nullptr);
  auto targetOffset = i.offset() + i.imm[1].u_BA;
  invertCond = false;
  if (!i.endsRegion && i.nextOffset == targetOffset) {
    invertCond = true;
    targetOffset = i.offset() + instrLen((Op*)i.pc());
  }
  return targetOffset;
}

}

//////////////////////////////////////////////////////////////////////

template<class Lambda>
void HhbcTranslator::implMIterInit(Offset relOffset, Lambda genFunc) {
  // TODO MIterInit doesn't check iterBranchTarget; this might be bug ...

  auto const exit = makeExit();
  auto const sp   = spillStack();
  auto const pred = getStackInnerTypePrediction(sp, 0);
  auto const src  = topV();

  if (!pred.subtypeOfAny(Type::Arr, Type::Obj)) {
    PUNT(MIterInit-unsupportedSrcType);
  }

  // Guard the inner type before we call the helper.
  gen(CheckRefInner, pred, exit, src);

  auto const res = genFunc(src, pred);
  auto const out = popV();
  gen(DecRef, out);
  implCondJmp(bcOff() + relOffset, true, res);
}

void HhbcTranslator::emitIterInit(int32_t iterId,
                                  Offset relOffset,
                                  int32_t valLocalId) {
  auto const catchBlock = makeCatch();
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC();
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInit);
  auto const res = gen(
    IterInit,
    Type::Bool,
    catchBlock,
    IterData(iterId, -1, valLocalId),
    src,
    m_irb->fp()
  );
  implCondJmp(targetOffset, !invertCond, res);
}

void HhbcTranslator::emitIterInitK(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId,
                                   int32_t keyLocalId) {
  auto const catchBlock = makeCatch();
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);

  auto const src = popC();
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInitK);
  auto const res = gen(
    IterInitK,
    Type::Bool,
    catchBlock,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    m_irb->fp()
  );
  implCondJmp(targetOffset, !invertCond, res);
}

void HhbcTranslator::emitIterNext(int32_t iterId,
                                  Offset relOffset,
                                  int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    IterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  implCondJmp(targetOffset, invertCond, res);
}

void HhbcTranslator::emitIterNextK(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId,
                                   int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    IterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  implCondJmp(targetOffset, invertCond, res);
}

void HhbcTranslator::emitWIterInit(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId) {
  auto const catchBlock = makeCatch();
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC();
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(WIterInit);
  auto const res = gen(
    WIterInit,
    Type::Bool,
    catchBlock,
    IterData(iterId, -1, valLocalId),
    src,
    m_irb->fp()
  );
  implCondJmp(targetOffset, !invertCond, res);
}

void HhbcTranslator::emitWIterInitK(int32_t iterId,
                                    Offset relOffset,
                                    int32_t valLocalId,
                                    int32_t keyLocalId) {
  auto const catchBlock = makeCatch();
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const src = popC();
  if (!src->type().subtypeOfAny(Type::Arr, Type::Obj)) PUNT(WIterInitK);
  auto const res = gen(
    WIterInitK,
    Type::Bool,
    catchBlock,
    IterData(iterId, keyLocalId, valLocalId),
    src,
    m_irb->fp()
  );
  implCondJmp(targetOffset, !invertCond, res);
}

void HhbcTranslator::emitWIterNext(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    WIterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  implCondJmp(targetOffset, invertCond, res);
}

void HhbcTranslator::emitWIterNextK(int32_t iterId,
                                    Offset relOffset,
                                    int32_t valLocalId,
                                    int32_t keyLocalId) {
  bool invertCond = false;
  auto const targetOffset = iterBranchTarget(*m_currentNormalizedInstruction,
                                             invertCond);
  auto const res = gen(
    WIterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  implCondJmp(targetOffset, invertCond, res);
}

void HhbcTranslator::emitMIterInit(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId) {
  auto const catchBlock = makeCatch();
  implMIterInit(relOffset, [&] (SSATmp* src, Type type) {
    return gen(
      MIterInit,
      type,
      catchBlock,
      IterData(iterId, -1, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterInitK(int32_t iterId,
                                    Offset relOffset,
                                    int32_t valLocalId,
                                    int32_t keyLocalId) {
  auto const catchBlock = makeCatch();
  implMIterInit(relOffset, [&] (SSATmp* src, Type type) {
    return gen(
      MIterInitK,
      type,
      catchBlock,
      IterData(iterId, keyLocalId, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterNext(int32_t iterId,
                                   Offset relOffset,
                                   int32_t valLocalId) {
  auto const res = gen(
    MIterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  implCondJmp(bcOff() + relOffset, false, res);
}

void HhbcTranslator::emitMIterNextK(int32_t iterId,
                                    Offset relOffset,
                                    int32_t valLocalId,
                                    int32_t keyLocalId) {
  auto const res = gen(
    MIterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  implCondJmp(bcOff() + relOffset, false, res);
}

void HhbcTranslator::emitIterFree(int32_t iterId) {
  gen(IterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitMIterFree(int32_t iterId) {
  gen(MIterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitIterBreak(const ImmVector& iv,
                                   Offset relOffset) {
  always_assert(m_currentNormalizedInstruction->endsRegion);

  for (int iterIndex = 0; iterIndex < iv.size(); iterIndex += 2) {
    IterKind iterKind = (IterKind)iv.vec32()[iterIndex];
    Id       iterId   = iv.vec32()[iterIndex + 1];
    switch (iterKind) {
      case KindOfIter:  gen(IterFree,  IterId(iterId), m_irb->fp()); break;
      case KindOfMIter: gen(MIterFree, IterId(iterId), m_irb->fp()); break;
      case KindOfCIter: gen(CIterFree, IterId(iterId), m_irb->fp()); break;
    }
  }

  // Would need to change this if we support not ending regions on this:
  gen(Jmp, makeExit(bcOff() + relOffset));
}

//////////////////////////////////////////////////////////////////////

}}
