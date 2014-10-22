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

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

template<class Lambda>
void HhbcTranslator::emitIterInitCommon(int offset, JmpFlags jmpFlags,
                                           Lambda genFunc,
                                           bool invertCond) {
  auto const src = popC();
  auto const type = src->type();
  if (!type.subtypeOfAny(Type::Arr, Type::Obj)) PUNT(IterInit);
  auto const res = genFunc(src);
  emitJmpCondHelper(offset, !invertCond, jmpFlags, res);
}

template<class Lambda>
void HhbcTranslator::emitMIterInitCommon(int offset, JmpFlags jmpFlags,
                                            Lambda genFunc) {
  auto exit = makeExit();

  SSATmp* src = topV();
  Type type = src->type();

  assert(type.isBoxed());
  m_irb->constrainValue(gen(LdRef, type.innerType(), exit, src),
                        DataTypeSpecific);
  SSATmp* res = genFunc(src);
  SSATmp* out = popV();
  gen(DecRef, out);
  emitJmpCondHelper(offset, true, jmpFlags, res);
}

void HhbcTranslator::emitIterInit(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId,
                                  bool invertCond,
                                  JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
      return gen(IterInit,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, -1, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitIterInitK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
      return gen(IterInitK,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, keyLocalId, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitIterNext(uint32_t iterId,
                                  int offset,
                                  uint32_t valLocalId,
                                  bool invertCond,
                                  JmpFlags jmpFlags) {
  SSATmp* res = gen(
    IterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitIterNextK(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   uint32_t keyLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    IterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitWIterInit(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(
    offset, jmpFlags, [&] (SSATmp* src) {
      return gen(WIterInit,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, -1, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitWIterInitK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    bool invertCond,
                                    JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitIterInitCommon(
    offset, jmpFlags, [&] (SSATmp* src) {
      return gen(WIterInitK,
                 Type::Bool,
                 catchBlock,
                 IterData(iterId, keyLocalId, valLocalId),
                 src,
                 m_irb->fp());
    },
    invertCond);
}

void HhbcTranslator::emitWIterNext(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   bool invertCond,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    WIterNext,
    Type::Bool,
    makeCatch(),
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitWIterNextK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    bool invertCond,
                                    JmpFlags jmpFlags) {
  SSATmp* res = gen(
    WIterNextK,
    Type::Bool,
    makeCatch(),
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, invertCond, jmpFlags, res);
}

void HhbcTranslator::emitMIterInit(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitMIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
    return gen(
      MIterInit,
      Type::Bool,
      catchBlock,
      IterData(iterId, -1, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterInitK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    JmpFlags jmpFlags) {
  auto catchBlock = makeCatch();
  emitMIterInitCommon(offset, jmpFlags, [&] (SSATmp* src) {
    return gen(
      MIterInitK,
      Type::Bool,
      catchBlock,
      IterData(iterId, keyLocalId, valLocalId),
      src,
      m_irb->fp()
    );
  });
}

void HhbcTranslator::emitMIterNext(uint32_t iterId,
                                   int offset,
                                   uint32_t valLocalId,
                                   JmpFlags jmpFlags) {
  SSATmp* res = gen(
    MIterNext,
    Type::Bool,
    IterData(iterId, -1, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, false, jmpFlags, res);
}

void HhbcTranslator::emitMIterNextK(uint32_t iterId,
                                    int offset,
                                    uint32_t valLocalId,
                                    uint32_t keyLocalId,
                                    JmpFlags jmpFlags) {
  SSATmp* res = gen(
    MIterNextK,
    Type::Bool,
    IterData(iterId, keyLocalId, valLocalId),
    m_irb->fp()
  );
  emitJmpCondHelper(offset, false, jmpFlags, res);
}

void HhbcTranslator::emitIterFree(uint32_t iterId) {
  gen(IterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitMIterFree(uint32_t iterId) {
  gen(MIterFree, IterId(iterId), m_irb->fp());
}

void HhbcTranslator::emitIterBreak(const ImmVector& iv,
                                   uint32_t offset,
                                   bool endsRegion) {
  int iterIndex;
  for (iterIndex = 0; iterIndex < iv.size(); iterIndex += 2) {
    IterKind iterKind = (IterKind)iv.vec32()[iterIndex];
    Id       iterId   = iv.vec32()[iterIndex + 1];
    switch (iterKind) {
      case KindOfIter:  gen(IterFree,  IterId(iterId), m_irb->fp()); break;
      case KindOfMIter: gen(MIterFree, IterId(iterId), m_irb->fp()); break;
      case KindOfCIter: gen(CIterFree, IterId(iterId), m_irb->fp()); break;
    }
  }

  if (!endsRegion) return;
  gen(Jmp, makeExit(offset));
}

//////////////////////////////////////////////////////////////////////

}}
