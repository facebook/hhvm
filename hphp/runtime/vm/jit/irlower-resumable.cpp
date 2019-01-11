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

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
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
#include "hphp/runtime/ext/asio/ext_await-all-wait-handle.h"
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

void cgStArResumeAddr(IRLS& env, const IRInstruction* inst) {
  auto const ar = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const addrOff = Resumable::resumeAddrOff() - Resumable::arOff();
  auto const offsetOff = Resumable::resumeOffsetOff() - Resumable::arOff();
  v << store{srcLoc(env, inst, 1).reg(), ar[addrOff]};
  v << storeli{inst->extra<ResumeOffset>()->off, ar[offsetOff]};
}

///////////////////////////////////////////////////////////////////////////////

namespace {

ptrdiff_t genOffset(bool isAsync) {
  return isAsync ? AsyncGenerator::objectOff() : Generator::objectOff();
}

}

IMPL_OPCODE_CALL(CreateGen)
IMPL_OPCODE_CALL(CreateAGen)

void cgContEnter(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const genFP = srcLoc(env, inst, 2).reg();
  auto const target = srcLoc(env, inst, 3).reg();

  auto const extra = inst->extra<ContEnter>();
  auto const spOff = extra->spOffset;
  auto const callOff = extra->callBCOffset;

  auto& v = vmain(env);

  auto const next = v.makeBlock();

  v << store{fp, genFP[AROFF(m_sfp)]};
  v << storeli{callOff, genFP[AROFF(m_callOff)]};

  v << copy{genFP, fp};
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(spOff.offset)], sync_sp};
  v << syncvmsp{sync_sp};

  v << contenter{fp, target, cross_trace_regs_resumed(),
                 {next, label(env, inst->taken())}};
  v = next;

  auto const dst = dstLoc(env, inst, 0);
  auto const type = inst->dst()->type();
  if (!type.admitsSingleVal()) {
    v << defvmretdata{dst.reg(0)};
  }
  if (type.needsReg()) {
    v << defvmrettype{dst.reg(1)};
  }
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

using WH = c_Awaitable;
using AFWH = c_AsyncFunctionWaitHandle;

namespace {

constexpr ptrdiff_t ar_rel(ptrdiff_t off) {
  return off - AFWH::arOff();
};

/*
 * Check if obj is an Awaitable. Passes CC_BE on sf if it is.
 */
void emitIsAwaitable(Vout& v, Vreg obj, Vreg sf) {
  auto constexpr minwh = (int)HeaderKind::WaitHandle;
  auto constexpr maxwh = (int)HeaderKind::AwaitAllWH;
  static_assert(maxwh - minwh == 2, "WH range check needs updating");

  auto const kind = v.makeReg();
  auto const wh_index = v.makeReg();
  v << loadzbl{obj[HeaderKindOffset], kind};
  v << subli{minwh, kind, wh_index, v.makeReg()};
  v << cmpli{maxwh - minwh, wh_index, sf};
}

}

IMPL_OPCODE_CALL(CreateAFWH)
IMPL_OPCODE_CALL(CreateAFWHNoVV)
IMPL_OPCODE_CALL(CreateAGWH)
IMPL_OPCODE_CALL(CreateSSWH)
IMPL_OPCODE_CALL(AFWHPrepareChild)

void cgCreateAAWH(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<CreateAAWHData>();

  cgCallHelper(
    vmain(env),
    env,
    CallSpec::direct(c_AwaitAllWaitHandle::fromFrameNoCheck),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst)
      .imm(extra->count)
      .ssa(1)
      .addr(fp, localOffset(extra->first))
  );
}

void cgCountWHNotDone(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<CountWHNotDone>();

  auto& v = vmain(env);
  auto const base = v.makeReg();
  auto const loc = v.cns((extra->count - 1) * 2);
  auto const cnt = v.cns(0);

  v << lea{fp[localOffset(extra->first + extra->count - 1)], base};

  auto out = doWhile(v, CC_GE, {loc, cnt},
    [&] (const VregList& in, const VregList& out) {
      auto const loc_in  = in[0],  cnt_in  = in[1];
      auto const loc_out = out[0], cnt_out = out[1];
      auto const type_loc = base[loc_in * 8 + TVOFF(m_type)];
      auto const data_loc = base[loc_in * 8 + TVOFF(m_data)];
      auto const sf_is_wh = v.makeReg();
      auto const sf_is_finished = v.makeReg();
      auto const sf_cont = v.makeReg();
      auto const obj = v.makeReg();
      auto const cnt_new = v.makeReg();
      auto const loop_cont = v.makeBlock();

      // Skip nulls.
      emitTypeTest(
        v, env, TNull, type_loc, data_loc, v.makeReg(),
        [&] (ConditionCode cc, Vreg sf) {
          ifThen(v, cc, sf, [&] (Vout& v) {
            v << phijmp{loop_cont, v.makeTuple({cnt_in})};
          });
        }
      );

      // Take exit on non-objects.
      emitTypeCheck(v, env, TObj, type_loc, data_loc, inst->taken());

      // Take exit on non-Awaitables.
      v << load{data_loc, obj};
      emitIsAwaitable(v, obj, sf_is_wh);
      fwdJcc(v, env, CC_A, sf_is_wh, inst->taken());

      // We depend on this in the test with 0x0E below.
      static_assert(c_Awaitable::STATE_SUCCEEDED == 0, "");
      static_assert(c_Awaitable::STATE_FAILED == 1, "");

      v << testbim{0x0E, obj[WH::stateOff()], sf_is_finished};
      cond(v, CC_NZ, sf_is_finished, cnt_new,
        [&] (Vout& v) {
          auto ret = v.makeReg();
          v << incq{cnt_in, ret, v.makeReg()};
          return ret;
        },
        [&] (Vout& v) { return cnt_in; }
      );

      v << phijmp{loop_cont, v.makeTuple({cnt_new})};

      // Add 2 to the loop variable because we can only scale by at most 8.
      v = loop_cont;
      v << phidef{v.makeTuple({cnt_out})};
      v << subqi{2, loc_in, loc_out, sf_cont};
      return sf_cont;
    }
  );

  v << copy{out[1], dstLoc(env, inst, 0).reg()};
}

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

///////////////////////////////////////////////////////////////////////////////

void cgIsWaitHandle(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  auto const sf = v.makeReg();

  emitIsAwaitable(v, obj, sf);
  v << setcc{CC_BE, sf, dst};
}

void cgLdWHState(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const state = v.makeReg();
  v << loadzbq{obj[WH::stateOff()], state};
  v << andqi{0x0F, state, dst, v.makeReg()};
}

void cgLdWHNotDone(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // We depend on this in the test with 0x0E below.
  static_assert(c_Awaitable::STATE_SUCCEEDED == 0, "");
  static_assert(c_Awaitable::STATE_FAILED == 1, "");

  auto const sf = v.makeReg();
  auto const result = v.makeReg();
  v << testbim{0x0E, obj[WH::stateOff()], sf};
  v << setcc{CC_NZ, sf, result};
  v << movzbq{result, dst};
}

void cgLdWHResult(IRLS& env, const IRInstruction* inst) {
  auto const obj = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), obj[WH::resultOff()]);
}

void cgLdAFWHActRec(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  v << lea{obj[AFWH::arOff()], dst};
}

///////////////////////////////////////////////////////////////////////////////

}}}
