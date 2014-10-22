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
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/ext/asio/async_generator.h"
#include "hphp/runtime/ext/ext_generator.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitAwaitE(SSATmp* child, Block* catchBlock,
                                Offset resumeOffset, int numIters) {
  assert(curFunc()->isAsync());
  assert(!resumed());
  assert(child->isA(Type::Obj));

  // Create the AsyncFunctionWaitHandle object. CreateAFWH takes care of
  // copying local variables and iterators.
  auto const func = curFunc();
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  auto const waitHandle =
    gen(CreateAFWH, catchBlock, m_irb->fp(), cns(func->numSlotsInFrame()),
        resumeAddr, cns(resumeOffset),
        child);

  SSATmp* asyncAR = gen(LdAFWHActRec, Type::PtrToGen, waitHandle);

  // Call the FunctionSuspend hook and put the AsyncFunctionWaitHandle
  // on the stack so that the unwinder would decref it.
  push(waitHandle);
  emitRetSurpriseCheck(asyncAR, nullptr, makeCatch(), false);
  discard(1);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(StRetVal, m_irb->fp(), waitHandle);
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitAwaitR(SSATmp* child, Block* catchBlock,
                                Offset resumeOffset) {
  assert(curFunc()->isAsync());
  assert(resumed());
  assert(child->isA(Type::Obj));

  // Prepare child for establishing dependency.
  gen(AFWHPrepareChild, catchBlock, m_irb->fp(), child);

  // Suspend the async function.
  auto const resumeSk = SrcKey(curFunc(), resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  gen(StAsyncArRaw, RawMemData{RawMemData::AsyncResumeAddr}, m_irb->fp(),
      resumeAddr);
  gen(StAsyncArRaw, RawMemData{RawMemData::AsyncResumeOffset}, m_irb->fp(),
      cns(resumeOffset));

  // Set up the dependency.
  gen(AFWHBlockOn, m_irb->fp(), child);

  // Transfer control back to the scheduler.
  auto const sp = spillStack();
  push(cns(Type::InitNull));
  emitRetSurpriseCheck(m_irb->fp(), nullptr, makeCatch(), true);
  popC();

  auto const retAddr = gen(LdRetAddr, m_irb->fp());
  auto const fp = gen(FreeActRec, m_irb->fp());

  gen(RetCtrl, RetCtrlData(true), sp, fp, retAddr);
}

void HhbcTranslator::emitAwait(Offset resumeOffset, int numIters) {
  assert(curFunc()->isAsync());

  if (curFunc()->isAsyncGenerator()) PUNT(Await-AsyncGenerator);

  auto const catchBlock = makeCatch();
  auto const exitSlow   = makeExitSlow();

  if (!topC()->isA(Type::Obj)) PUNT(Await-NonObject);

  auto const child = popC();
  gen(JmpZero, exitSlow, gen(IsWaitHandle, child));

  // cns() would ODR-use these
  auto const kSucceeded = c_WaitHandle::STATE_SUCCEEDED;
  auto const kFailed    = c_WaitHandle::STATE_FAILED;

  auto const state = gen(LdWHState, child);
  gen(JmpEq, exitSlow, state, cns(kFailed));

  m_irb->ifThenElse(
    [&] (Block* taken) {
      gen(JmpEq, taken, state, cns(kSucceeded));
    },
    [&] { // Next: the wait handle is not finished, we need to suspend
      if (resumed()) {
        emitAwaitR(child, catchBlock, resumeOffset);
      } else {
        emitAwaitE(child, catchBlock, resumeOffset, numIters);
      }
    },
    [&] { // Taken: retrieve the result from the wait handle
      auto const res = gen(LdWHResult, child);
      gen(IncRef, res);
      gen(DecRef, child);
      push(res);
    }
  );
}

//////////////////////////////////////////////////////////////////////

void HhbcTranslator::emitCreateCont(Offset resumeOffset) {
  assert(!resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(CreateCont-AsyncGenerator);

  // Create the Generator object. CreateCont takes care of copying local
  // variables and iterators.
  auto const func = curFunc();
  auto const resumeSk = SrcKey(func, resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  auto const cont = gen(CreateCont, m_irb->fp(), cns(func->numSlotsInFrame()),
                        resumeAddr, cns(resumeOffset));

  // Teleport local variables into the generator.
  SSATmp* contAR = gen(LdContActRec, Type::PtrToGen, cont);

  // Call the FunctionSuspend hook and put the return value on the stack so that
  // the unwinder would decref it.
  emitRetSurpriseCheck(contAR, nullptr, makeCatch({cont}), false);

  // Grab caller info from ActRec, free ActRec, store the return value
  // and return control to the caller.
  gen(StRetVal, m_irb->fp(), cont);
  SSATmp* retAddr = gen(LdRetAddr, m_irb->fp());
  SSATmp* sp = gen(RetAdjustStack, m_irb->fp());
  SSATmp* fp = gen(FreeActRec, m_irb->fp());
  gen(RetCtrl, RetCtrlData(false), sp, fp, retAddr);
}

void HhbcTranslator::emitContEnter(Offset returnOffset) {
  assert(curClass());
  assert(curClass()->classof(c_AsyncGenerator::classof()) ||
         curClass()->classof(c_Generator::classof()));
  assert(curFunc()->contains(returnOffset));

  // Load generator's FP and resume address.
  auto genObj = gen(LdThis, m_irb->fp());
  auto genFp = gen(LdContActRec, Type::FramePtr, genObj);
  auto resumeAddr =
    gen(LdContArRaw, RawMemData{RawMemData::ContResumeAddr}, genFp);

  // Make sure function enter hook is called if needed.
  auto exitSlow = makeExitSlow();
  gen(CheckSurpriseFlags, exitSlow);

  // Exit to interpreter if resume address is not known.
  resumeAddr = gen(CheckNonNull, exitSlow, resumeAddr);

  // Sync stack.
  auto const sp = spillStack();

  // Enter generator.
  auto returnBcOffset = returnOffset - curFunc()->base();
  gen(ContEnter, sp, m_irb->fp(), genFp, resumeAddr, cns(returnBcOffset));
}

void HhbcTranslator::emitYieldReturnControl(Block* catchBlock) {
  // Push return value of next()/send()/raise().
  push(cns(Type::InitNull));

  auto const sp = spillStack();
  emitRetSurpriseCheck(m_irb->fp(), nullptr, catchBlock, true);

  auto const retAddr = gen(LdRetAddr, m_irb->fp());
  auto const fp = gen(FreeActRec, m_irb->fp());

  gen(RetCtrl, RetCtrlData(true), sp, fp, retAddr);
}

void HhbcTranslator::emitYieldImpl(Offset resumeOffset) {
  // Resumable::setResumeAddr(resumeAddr, resumeOffset)
  auto const resumeSk = SrcKey(curFunc(), resumeOffset, true);
  auto const resumeAddr = gen(LdBindAddr, LdBindAddrData(resumeSk));
  gen(StContArRaw, RawMemData{RawMemData::ContResumeAddr}, m_irb->fp(),
      resumeAddr);
  gen(StContArRaw, RawMemData{RawMemData::ContResumeOffset}, m_irb->fp(),
      cns(resumeOffset));

  // Set yielded value.
  auto const oldValue = gen(LdContArValue, Type::Cell, m_irb->fp());
  gen(StContArValue, m_irb->fp(), popC(DataTypeGeneric)); // teleporting value
  gen(DecRef, oldValue);

  // Set state from Running to Started.
  gen(StContArRaw, RawMemData{RawMemData::ContState}, m_irb->fp(),
      cns(BaseGenerator::State::Started));
}


void HhbcTranslator::emitYield(Offset resumeOffset) {
  assert(resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(Yield-AsyncGenerator);

  auto catchBlock = makeCatchNoSpill();
  emitYieldImpl(resumeOffset);

  // take a fast path if this generator has no yield k => v;
  if (curFunc()->isPairGenerator()) {
    // this needs optimization
    auto const idx =
      gen(LdContArRaw, RawMemData{RawMemData::ContIndex}, m_irb->fp());
    auto const newIdx = gen(AddInt, idx, cns(1));
    gen(StContArRaw, RawMemData{RawMemData::ContIndex}, m_irb->fp(), newIdx);

    auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
    gen(StContArKey, m_irb->fp(), newIdx);
    gen(DecRef, oldKey);
  } else {
    // we're guaranteed that the key is an int
    gen(ContArIncKey, m_irb->fp());
  }

  // transfer control
  emitYieldReturnControl(catchBlock);
}

void HhbcTranslator::emitYieldK(Offset resumeOffset) {
  assert(resumed());
  assert(curFunc()->isGenerator());

  if (curFunc()->isAsyncGenerator()) PUNT(YieldK-AsyncGenerator);

  auto catchBlock = makeCatchNoSpill();
  emitYieldImpl(resumeOffset);

  auto const newKey = popC();
  auto const oldKey = gen(LdContArKey, Type::Cell, m_irb->fp());
  gen(StContArKey, m_irb->fp(), newKey);
  gen(DecRef, oldKey);

  auto const keyType = newKey->type();
  if (keyType <= Type::Int) {
    gen(ContArUpdateIdx, m_irb->fp(), newKey);
  }

  // transfer control
  emitYieldReturnControl(catchBlock);
}

void HhbcTranslator::emitContCheck(bool checkStarted) {
  assert(curClass());
  assert(curClass()->classof(c_AsyncGenerator::classof()) ||
         curClass()->classof(c_Generator::classof()));
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContPreNext, makeExitSlow(), cont, cns(checkStarted));
}

void HhbcTranslator::emitContValid() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  push(gen(ContValid, cont));
}

void HhbcTranslator::emitContKey() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContStartedCheck, makeExitSlow(), cont);
  SSATmp* offset = cns(offsetof(c_Generator, m_key));
  SSATmp* value = gen(LdProp, Type::Cell, cont, offset);
  pushIncRef(value);
}

void HhbcTranslator::emitContCurrent() {
  assert(curClass());
  SSATmp* cont = gen(LdThis, m_irb->fp());
  gen(ContStartedCheck, makeExitSlow(), cont);
  SSATmp* offset = cns(offsetof(c_Generator, m_value));
  SSATmp* value = gen(LdProp, Type::Cell, cont, offset);
  pushIncRef(value);
}

//////////////////////////////////////////////////////////////////////

}}
