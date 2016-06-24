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

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

void implStARResume(IRLS& env, const IRInstruction* inst,
                    ptrdiff_t addrOff, ptrdiff_t offsetOff) {
  auto const ar = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  v << store{srcLoc(env, inst, 1).reg(), ar[addrOff]};
  v << storeli{inst->extra<ResumeOffset>()->off, ar[offsetOff]};
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLdResumableArObj(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const resumableAR = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const objectOff = Resumable::dataOff() - Resumable::arOff();
  v << lea{resumableAR[objectOff], dst};
}

///////////////////////////////////////////////////////////////////////////////

namespace {

ptrdiff_t genOffset(bool isAsync) {
  return isAsync ? AsyncGenerator::objectOff() : Generator::objectOff();
}

}

IMPL_OPCODE_CALL(CreateCont)

void cgContEnter(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const genFP = srcLoc(env, inst, 2).reg();
  auto const target = srcLoc(env, inst, 3).reg();

  auto const extra = inst->extra<ContEnter>();
  auto const spOff = extra->spOffset;
  auto const returnOff = extra->returnBCOffset;

  auto& v = vmain(env);

  auto const next = v.makeBlock();

  v << store{fp, genFP[AROFF(m_sfp)]};
  v << storeli{returnOff, genFP[AROFF(m_soff)]};

  v << copy{genFP, fp};
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(spOff.offset)], sync_sp};
  v << syncvmsp{sync_sp};

  v << contenter{fp, target, cross_trace_regs_resumed(),
                 {next, label(env, inst->taken())}};
  env.catch_calls[inst->taken()] = CatchCall::PHP;
  v = next;
}

void cgContPreNext(IRLS& env, const IRInstruction* inst) {
  auto const cont = srcLoc(env, inst, 0).reg();
  auto const checkStarted = inst->src(1)->boolVal();
  auto const isAsync = inst->extra<IsAsyncData>()->isAsync;
  auto& v = vmain(env);

  auto const sf = v.makeReg();

  // These asserts make sure that the startedCheck work.
  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");
  static_assert(uint8_t(BaseGenerator::State::Started) == 1, "used below");
  static_assert(uint8_t(BaseGenerator::State::Done) > 3, "");

  // These asserts ensure that the state transition works.  If we're in the
  // Created state we want to transition to Priming, and if we're in the
  // Started state we want to transition to Running.  By laying out the enum
  // this way we can avoid the branch and just transition by adding 2 to the
  // current state.
  static_assert(uint8_t(BaseGenerator::State::Priming) ==
                uint8_t(BaseGenerator::State::Created) + 2, "used below");
  static_assert(uint8_t(BaseGenerator::State::Running) ==
                uint8_t(BaseGenerator::State::Started) + 2, "used below");

  // Take exit if state != 1 (checkStarted) or if state > 1 (!checkStarted).
  auto stateOff = BaseGenerator::stateOff() - genOffset(isAsync);
  v << cmpbim{int8_t(BaseGenerator::State::Started), cont[stateOff], sf};
  fwdJcc(v, env, checkStarted ? CC_NE : CC_A, sf, inst->taken());

  // Transition the generator into either the Priming state (if we were just
  // created) or the Running state (if we were started).  Due to the way the
  // enum is layed out, we can model this by just adding 2.
  v << addlim{int8_t(2), cont[stateOff], v.makeReg()};
}

///////////////////////////////////////////////////////////////////////////////

void cgContStartedCheck(IRLS& env, const IRInstruction* inst) {
  auto const cont = srcLoc(env, inst, 0).reg();
  auto const isAsync = inst->extra<IsAsyncData>()->isAsync;
  auto& v = vmain(env);

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");

  // Take exit if state == 0.
  auto const sf = v.makeReg();
  auto const stateOff = BaseGenerator::stateOff() - genOffset(isAsync);
  v << testbim{int8_t(0xffu), cont[stateOff], sf};
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgContStarted(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cont = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // Return true if generator state is not in the Created state.
  auto const sf = v.makeReg();
  auto const stateOff = BaseGenerator::stateOff() -
                        genOffset(false /* isAsync */);
  v << cmpbim{int8_t(BaseGenerator::State::Created), cont[stateOff], sf};
  v << setcc{CC_NE, sf, dst};
}

void cgContValid(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cont = srcLoc(env, inst, 0).reg();
  auto const isAsync = inst->extra<IsAsyncData>()->isAsync;
  auto& v = vmain(env);

  // Return true if generator state is not Done.
  auto const sf = v.makeReg();
  auto const stateOff = BaseGenerator::stateOff() - genOffset(isAsync);
  v << cmpbim{int8_t(BaseGenerator::State::Done), cont[stateOff], sf};
  v << setcc{CC_NE, sf, dst};
}

void cgStContArState(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << storebi{
    static_cast<int8_t>(inst->extra<StContArState>()->state),
    fp[BaseGenerator::stateOff() - BaseGenerator::arOff()]
  };
}

///////////////////////////////////////////////////////////////////////////////

void cgLdContField(IRLS& env, const IRInstruction* inst) {
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0),
         srcLoc(env, inst, 0).reg()[inst->src(1)->intVal()]);
}

void cgLdContActRec(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cont = srcLoc(env, inst, 0).reg();
  auto const isAsync = inst->extra<IsAsyncData>()->isAsync;
  auto const arOff = BaseGenerator::arOff() - genOffset(isAsync);
  vmain(env) << lea{cont[arOff], dst};
}

void cgLdContArValue(IRLS& env, const IRInstruction* inst) {
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto const valueOff = GENDATAOFF(m_value) - Generator::arOff();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), contAR[valueOff]);
}

void cgStContArValue(IRLS& env, const IRInstruction* inst) {
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto const valueOff = GENDATAOFF(m_value) - Generator::arOff();
  storeTV(vmain(env), contAR[valueOff], srcLoc(env, inst, 1), inst->src(1));
}

void cgLdContResumeAddr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const cont = srcLoc(env, inst, 0).reg();
  auto const isAsync = inst->extra<IsAsyncData>()->isAsync;
  auto const addrOff = BaseGenerator::resumeAddrOff() - genOffset(isAsync);
  vmain(env) << load{cont[addrOff], dst};
}

void cgStContArResume(IRLS& env, const IRInstruction* inst) {
  implStARResume(
    env, inst,
    BaseGenerator::resumeAddrOff() - BaseGenerator::arOff(),
    BaseGenerator::resumeOffsetOff() - BaseGenerator::arOff()
  );
}

void cgLdContArKey(IRLS& env, const IRInstruction* inst) {
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto const keyOff = GENDATAOFF(m_key) - Generator::arOff();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), contAR[keyOff]);
}

void cgStContArKey(IRLS& env, const IRInstruction* inst) {
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto const keyOff = GENDATAOFF(m_key) - Generator::arOff();
  storeTV(vmain(env), contAR[keyOff], srcLoc(env, inst, 1), inst->src(1));
}

void cgContArIncKey(IRLS& env, const IRInstruction* inst) {
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const keyOff = GENDATAOFF(m_key) - Generator::arOff();
  v << incqm{contAR[keyOff + TVOFF(m_data)], v.makeReg()};
}

void cgContArIncIdx(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const contAR = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const tmp = v.makeReg();
  auto const idxOff = GENDATAOFF(m_index) - Generator::arOff();
  v << load{contAR[idxOff], tmp};
  v << incq{tmp, dst, v.makeReg()};
  v << store{dst, contAR[idxOff]};
}

void cgContArUpdateIdx(IRLS& env, const IRInstruction* inst) {
  auto contAR = srcLoc(env, inst, 0).reg();
  auto newIdx = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto mem_index = v.makeReg();
  auto const idxOff = GENDATAOFF(m_index) - Generator::arOff();
  v << load{contAR[idxOff], mem_index};

  auto const sf = v.makeReg();
  auto res = v.makeReg();
  v << cmpq{mem_index, newIdx, sf};
  v << cmovq{CC_G, sf, mem_index, newIdx, res};
  v << store{res, contAR[idxOff]};
}

///////////////////////////////////////////////////////////////////////////////

using WH = c_WaitHandle;
using AFWH = c_AsyncFunctionWaitHandle;

namespace {

constexpr ptrdiff_t ar_rel(ptrdiff_t off) {
  return off - AFWH::arOff();
};

}

IMPL_OPCODE_CALL(CreateAFWH)
IMPL_OPCODE_CALL(CreateAFWHNoVV)
IMPL_OPCODE_CALL(CreateSSWH)
IMPL_OPCODE_CALL(AFWHPrepareChild)

void cgAFWHBlockOn(IRLS& env, const IRInstruction* inst) {
  auto parentAR = srcLoc(env, inst, 0).reg();
  auto child = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  // parent->setState(STATE_BLOCKED);
  v << storebi{
    AFWH::toKindState(WH::Kind::AsyncFunction, AFWH::STATE_BLOCKED),
    parentAR[ar_rel(AFWH::stateOff())]
  };

  auto const blockableOff = AFWH::childrenOff() + AFWH::Node::blockableOff();

  // parent->m_blockable.m_bits = child->m_parentChain.m_firstParent|Kind::AFWH;
  static_assert(
    uint8_t(AsioBlockable::Kind::AsyncFunctionWaitHandleNode) == 0,
    "AFWH kind must be 0."
  );
  auto const firstParentOff = c_WaitableWaitHandle::parentChainOff() +
                              AsioBlockableChain::firstParentOff();
  auto const nextParentOff = blockableOff + AsioBlockable::bitsOff();

  auto const firstParent = v.makeReg();
  v << load{child[firstParentOff], firstParent};
  v << store{firstParent, parentAR[ar_rel(nextParentOff)]};

  // child->m_parentChain.m_firstParent = &parent->m_blockable;
  auto objToAR = v.makeReg();
  v << lea{parentAR[ar_rel(blockableOff)], objToAR};
  v << store{objToAR, child[firstParentOff]};

  // parent->m_child = child;
  auto const childOff = AFWH::childrenOff() + AFWH::Node::childOff();
  v << store{child, parentAR[ar_rel(childOff)]};
}

IMPL_OPCODE_CALL(ABCUnblock)

///////////////////////////////////////////////////////////////////////////////

void cgIsWaitHandle(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();

  static_assert(
    ObjectData::IsWaitHandle < 0xff,
    "We use byte instructions for IsWaitHandle."
  );
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testwim{ObjectData::IsWaitHandle, obj[ObjectData::attributeOff()], sf};
  v << setcc{CC_NZ, sf, dst};
}

void cgLdWHState(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const state = v.makeReg();
  v << loadzbq{obj[WH::stateOff()], state};
  v << andqi{0x0F, state, dst, v.makeReg()};
}

void cgStAsyncArSucceeded(IRLS& env, const IRInstruction* inst) {
  auto const ar = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << storebi{
    WH::toKindState(WH::Kind::AsyncFunction, WH::STATE_SUCCEEDED),
    ar[ar_rel(WH::stateOff())]
  };
}

void cgLdWHResult(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), obj[WH::resultOff()]);
}

void cgStAsyncArResult(IRLS& env, const IRInstruction* inst) {
  auto const ar = srcLoc(env, inst, 0).reg();
  storeTV(vmain(env), ar[ar_rel(AFWH::resultOff())],
          srcLoc(env, inst, 1), inst->src(1));
}

void cgLdAFWHActRec(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << lea{obj[AFWH::arOff()], dst};
}

void cgLdAsyncArParentChain(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto ar = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << load{ar[ar_rel(AFWH::parentChainOff())], dst};
}

void cgStAsyncArResume(IRLS& env, const IRInstruction* inst) {
  implStARResume(
    env, inst,
    ar_rel(AFWH::resumeAddrOff()),
    ar_rel(AFWH::resumeOffsetOff())
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
