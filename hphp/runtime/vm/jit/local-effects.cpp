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
#include "hphp/runtime/vm/jit/local-effects.h"

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

void local_effects(const FrameStateMgr& frameState,
                   const IRInstruction* inst,
                   LocalStateHook& hook) {
  auto killIterLocals = [&](const std::initializer_list<uint32_t>& ids) {
    for (auto id : ids) {
      hook.setLocalValue(id, nullptr);
    }
  };

  switch (inst->op()) {
    case CallBuiltin:
      if (inst->extra<CallBuiltin>()->destroyLocals) hook.clearLocals();
      break;

    case Call:
    case CallArray:
    case ContEnter:
      {
        auto const callDestroysLocals =
          (inst->is(CallArray) && inst->extra<CallArray>()->destroyLocals) ||
          (inst->is(Call) && inst->extra<Call>()->destroyLocals);
        hook.killLocalsForCall(callDestroysLocals);
      }
      break;

    case StRef:
      hook.updateLocalRefPredictions(inst->src(0), inst->src(1));
      break;

    case StLoc:
      hook.setLocalValue(inst->extra<LocalId>()->locId, inst->src(1));
      break;

    case LdLoc:
      hook.setLocalValue(inst->extra<LdLoc>()->locId, inst->dst());
      break;

    case StLocPseudoMain:
      hook.setLocalPredictedType(inst->extra<LocalId>()->locId,
                            inst->src(1)->type());
      break;

    case AssertLoc:
    case CheckLoc: {
      auto id = inst->extra<LocalId>()->locId;
      if (inst->marker().func()->isPseudoMain()) {
        hook.setLocalPredictedType(id, inst->typeParam());
      } else {
        hook.refineLocalType(id,
                             inst->typeParam(),
                             TypeSource::makeGuard(inst));
      }
      break;
    }

    case HintLocInner:
      hook.setBoxedLocalPrediction(inst->extra<HintLocInner>()->locId,
                                   inst->typeParam());
      break;

    case CheckType:
    case AssertType:
      hook.refineLocalValues(inst->src(0), inst->dst());
      break;

    case IterInitK:
    case WIterInitK:
      // kill the locals to which this instruction stores iter's key and value
      killIterLocals({inst->extra<IterData>()->keyId,
                      inst->extra<IterData>()->valId});
      break;

    case IterInit:
    case WIterInit:
      // kill the local to which this instruction stores iter's value
      killIterLocals({inst->extra<IterData>()->valId});
      break;

    case IterNextK:
    case WIterNextK:
      // kill the locals to which this instruction stores iter's key and value
      killIterLocals({inst->extra<IterData>()->keyId,
                      inst->extra<IterData>()->valId});
      break;

    case IterNext:
    case WIterNext:
      // kill the local to which this instruction stores iter's value
      killIterLocals({inst->extra<IterData>()->valId});
      break;

    case InterpOne:
    case InterpOneCF: {
      auto const& id = *inst->extra<InterpOneData>();
      assertx(!id.smashesAllLocals || id.nChangedLocals == 0);
      if (id.smashesAllLocals || inst->marker().func()->isPseudoMain()) {
        hook.clearLocals();
      } else {
        auto it = id.changedLocals;
        auto const end = it + id.nChangedLocals;
        for (; it != end; ++it) {
          auto& loc = *it;
          // If changing the inner type of a boxed local, also drop the
          // information about inner types for any other boxed locals.
          if (loc.type <= TBoxedCell) hook.dropLocalRefsInnerTypes();
          hook.setLocalType(loc.id, loc.type);
        }
      }
      break;
    }
    default:
      break;
  }

  if (MInstrEffects::supported(inst)) {
    MInstrEffects::get(inst, frameState, hook);
  }
}

//////////////////////////////////////////////////////////////////////

}}
