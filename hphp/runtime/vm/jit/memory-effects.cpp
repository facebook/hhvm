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
#include "hphp/runtime/vm/jit/memory-effects.h"

#include "hphp/util/match.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/assertions.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit {

namespace {

const StaticString s_GLOBALS("GLOBALS");

uint32_t iterId(const IRInstruction& inst) {
  return inst.extra<IterId>()->iterId;
}

AliasClass pointee(
  const SSATmp* ptr,
  jit::flat_set<const IRInstruction*>* visited_labels
) {
  auto const type = ptr->type();
  always_assert(type <= TMemToCell);
  auto const canonPtr = canonical(ptr);
  if (!canonPtr->isA(TMemToCell)) {
    // This can happen when ptr is TBottom from a passthrough instruction with
    // a src that isn't TBottom. The most common cause of this is something
    // like "t5:Bottom = CheckType<Str> t2:Int". It means ptr isn't really a
    // pointer, so return AEmpty to avoid unnecessarily pessimizing any
    // optimizations.
    always_assert(ptr->isA(TBottom));
    return AEmpty;
  }

  auto const sinst = canonPtr->inst();

  if (sinst->is(LdRDSAddr, LdInitRDSAddr)) {
    return ARds { sinst->extra<RDSHandleData>()->handle };
  }

  // For phis, union all incoming values, taking care to not recurse infinitely
  // in the presence of loops.
  if (sinst->is(DefLabel)) {
    if (visited_labels && visited_labels->count(sinst)) {
      return AEmpty;
    }

    auto const dsts = sinst->dsts();
    auto const dstIdx =
      std::find(dsts.begin(), dsts.end(), canonPtr) - dsts.begin();
    always_assert(dstIdx >= 0 && dstIdx < sinst->numDsts());

    folly::Optional<jit::flat_set<const IRInstruction*>> label_set;
    if (visited_labels == nullptr) {
      label_set.emplace();
      visited_labels = &label_set.value();
    }
    visited_labels->insert(sinst);

    auto ret = AEmpty;
    sinst->block()->forEachSrc(
      dstIdx, [&](const IRInstruction* /*jmp*/, const SSATmp* thePtr) {
        ret = ret | pointee(thePtr, visited_labels);
      });
    return ret;
  }

  auto specific = [&] () -> folly::Optional<AliasClass> {
    if (type <= TBottom) return AEmpty;

    if (type <= TMemToFrameCell) {
      if (sinst->is(LdLocAddr)) {
        return AliasClass {
          ALocal { sinst->src(0), sinst->extra<LdLocAddr>()->locId }
        };
      }
      return ALocalAny;
    }

    if (type <= TMemToStkCell) {
      if (sinst->is(LdStkAddr)) {
        return AliasClass {
          AStack::at(sinst->extra<LdStkAddr>()->offset)
        };
      }
      return AStackAny;
    }

    if (type <= TMemToPropCell) {
      if (sinst->is(LdPropAddr, LdInitPropAddr)) {
        return AliasClass {
          AProp {
            sinst->src(0),
            safe_cast<uint16_t>(sinst->extra<IndexData>()->index)
          }
        };
      }
      return APropAny;
    }

    if (type <= TMemToMISCell) {
      if (sinst->is(LdMIStateAddr)) {
        return mis_from_offset(sinst->src(0)->intVal());
      }
      if (ptr->hasConstVal() && ptr->rawVal() == 0) {
        // nullptr tvRef pointer, representing an instruction that doesn't use
        // it.
        return AEmpty;
      }
      return AMIStateTempBase;
    }

    auto elem = [&] () -> AliasClass {
      auto base = sinst->src(0);
      auto key  = sinst->src(1);

      always_assert(base->isA(TArrLike));

      if (key->isA(TInt)) {
        if (key->hasConstVal()) return AElemI { base, key->intVal() };
        return AElemIAny;
      }
      if (key->hasConstVal(TStr)) {
        assertx(!base->isA(TVec));
        return AElemS { base, key->strVal() };
      }
      return AElemAny;
    };

    if (type <= TMemToElemCell) {
      if (sinst->is(LdVecElemAddr)) return elem();
      return AElemAny;
    }

    // The result of ElemArray{,W,U} is either the address of an array element,
    // or &immutable_null_base.
    if (type <= TMemToMembCell) {
      // Takes a PtrToCell as its first operand, so we can't easily grab an
      // array base.
      if (sinst->is(ElemVecU, ElemDictU, ElemKeysetU)) {
        return AElemAny;
      }

      // These instructions can only get at tvRef when given it as a
      // src. Otherwise they can only return pointers to properties or
      // &immutable_null_base.
      if (sinst->is(PropX, PropDX, PropQ)) {
        assertx(sinst->srcs().back()->isA(TMemToMISCell));
        auto const src = sinst->srcs().back();
        return APropAny | pointee(src, visited_labels);
      }

      // Like the Prop* instructions, but for array elements. These ops could
      // pointers to collection elements, which we don't have AliasClasses for.
      if (sinst->is(ElemDX, ElemUX)) {
        return AElemAny;
      }

      return folly::none;
    }

    return folly::none;
  }();

  if (specific) return *specific;

  /*
   * None of the above worked, so try to make the smallest union we can based
   * on the pointer type.
   */
  auto ret = AEmpty;
  if (type.maybe(TMemToStkCell))     ret = ret | AStackAny;
  if (type.maybe(TMemToFrameCell))   ret = ret | ALocalAny;
  if (type.maybe(TMemToPropCell))    ret = ret | APropAny;
  if (type.maybe(TMemToElemCell))    ret = ret | AElemAny;
  if (type.maybe(TMemToMISCell))     ret = ret | AMIStateTempBase;
  if (type.maybe(TMemToClsInitCell)) ret = ret | AHeapAny;
  if (type.maybe(TMemToClsCnsCell))  ret = ret | AHeapAny;
  if (type.maybe(TMemToSPropCell))   ret = ret | ARdsAny;
  return ret;
}

//////////////////////////////////////////////////////////////////////

AliasClass all_pointees(folly::Range<SSATmp**> srcs) {
  auto ret = AliasClass{AEmpty};
  for (auto const& src : srcs) {
    if (src->isA(TMemToCell)) {
      ret = ret | pointee(src);
    }
  }
  return ret;
}

// Return an AliasClass containing all locations pointed to by any MemToCell
// sources to an instruction.
AliasClass all_pointees(const IRInstruction& inst) {
  return all_pointees(inst.srcs());
}

// Return an AliasClass representing a range of the eval stack that contains
// everything below a logical depth.
AliasClass stack_below(IRSPRelOffset offset) {
  return AStack::below(offset);
}

//////////////////////////////////////////////////////////////////////

// Return an AliasClass representing an entire ActRec at base + offset.
AliasClass actrec(SSATmp* base, IRSPRelOffset offset) {
  return AStack::range(offset, offset + int32_t{kNumActRecCells});
}

/*
 * AliasClass that can be used to represent effects on liveFrame().
 */
AliasClass livefp(SSATmp* fp) {
  return AFBasePtr | AActRec { fp };
}

AliasClass livefp(const IRInstruction& inst) {
  return livefp(inst.marker().fp());
}

//////////////////////////////////////////////////////////////////////

// Determine an AliasClass representing any locals in the instruction's frame
// which might be accessed via debug_backtrace().

const Func* func_from_fp(SSATmp* fp) {
  if (!fp) return nullptr;
  auto fpInst = fp->inst();
  if (fpInst->is(DefFP)) return fpInst->marker().func();
  if (fpInst->is(DefFuncEntryFP)) return fpInst->extra<DefFuncEntryFP>()->func;
  if (fpInst->is(BeginInlining)) return fpInst->extra<BeginInlining>()->func;
  always_assert(false);
}

bool any_frame_has_metadata(SSATmp* fp) {
  while (fp) {
    auto const func = func_from_fp(fp);
    if (!func || func->lookupVarId(s_86metadata.get()) != kInvalidId) {
      return true;
    }
    fp = fp->inst()->is(BeginInlining) ? fp->inst()->src(1) : nullptr;
  }
  return false;
}

AliasClass backtrace_locals(const IRInstruction& inst) {
  auto eachFunc = [&] (auto fn) {
    auto ac = AEmpty;
    for (auto fp = inst.marker().fp(); fp; ) {
      ac |= fn(func_from_fp(fp), fp);
      fp = fp->inst()->is(BeginInlining) ? fp->inst()->src(1) : nullptr;
    }
    return ac;
  };

  auto const add86meta = [&] (const Func* func, SSATmp* fp) -> AliasClass {
    // The 86metadata variable can also exist in a VarEnv, but accessing that is
    // considered a heap effect, so we can ignore it.
    auto const local = func->lookupVarId(s_86metadata.get());
    if (local == kInvalidId) return AEmpty;
    return ALocal { fp, (uint32_t)local };
  };

  // Either there's no func or no frame-pointer. Either way, be conservative and
  // assume anything can be read. This can happen in test code, for instance.
  if (!inst.marker().fp()) return ALocalAny;

  if (!RuntimeOption::EnableArgsInBacktraces) return eachFunc(add86meta);

  return eachFunc([&] (const Func* func, SSATmp* fp) {
    auto ac = AEmpty;
    auto const numParams = func->numParams();

    if (func->hasReifiedGenerics()) {
      // First non param local contains reified generics
      AliasIdSet reifiedgenerics{ AliasIdSet::IdRange{numParams, numParams + 1} };
      ac |= ALocal { fp, reifiedgenerics };
    }

    if (func->cls() && func->cls()->hasReifiedGenerics()) {
      // There is no way to access the SSATmp for ObjectData of `this` here,
      // so be very pessimistic
      ac |= APropAny;
    }

    if (!numParams) return add86meta(func, fp) | ac;

    AliasIdSet params{ AliasIdSet::IdRange{0, numParams} };
    return add86meta(func, fp) | ac | ALocal { fp, params };
  });
}

/////////////////////////////////////////////////////////////////////

/*
 * Modify a GeneralEffects to take potential VM re-entry into account.  This
 * affects may-load, may-store, and kills information for the instruction.  The
 * GeneralEffects should already contain AHeapAny in both loads and stores if
 * it affects those locations for reasons other than re-entry, but does not
 * need to if it doesn't.
 *
 * For loads, we need to take into account any locals potentially accessed by
 * debug_backtrace().
 *
 * For kills, locations on the eval stack below the re-entry depth should all
 * be added.
 */
GeneralEffects may_reenter(const IRInstruction& inst, GeneralEffects x) {
  auto const may_reenter_is_ok =
    inst.taken() && inst.taken()->isCatch();
  always_assert_flog(
    may_reenter_is_ok,
    "instruction {} claimed may_reenter, but it isn't allowed to say that",
    inst
  );

  /*
   * We want to union `killed_stack' into whatever else the instruction already
   * said it must kill, but if we end up with an unrepresentable AliasClass we
   * can't return a set that's too big (the `kills' set is unlike the other
   * AliasClasses in GeneralEffects in that means it kills /everything/ in the
   * set, since it's must-information).
   *
   * If we can't represent the union, just take the stack, in part because we
   * have some debugging asserts about this right now---but also nothing
   * actually uses may_reenter with a non-AEmpty kills at the time of this
   * writing anyway.
   */
  auto const new_kills = [&] {
    if (inst.marker().fp() == nullptr) return AEmpty;

    auto const offset = [&]() -> IRSPRelOffset {
      auto const fp = canonical(inst.marker().fp());
      if (fp->inst()->is(BeginInlining)) {
        assertx(inst.marker().resumeMode() == ResumeMode::None);
        auto const fpOffset = fp->inst()->extra<BeginInlining>()->spOffset;
        auto const numStackElemsFromFP = inst.marker().spOff() - FPInvOffset{0};
        return fpOffset - numStackElemsFromFP;
      }

      assertx(fp->inst()->is(DefFP, DefFuncEntryFP));
      auto const sp = inst.marker().sp();
      auto const irSPOff = sp->inst()->extra<DefStackData>()->irSPOff;
      return inst.marker().spOff().to<IRSPRelOffset>(irSPOff);
    }();

    auto const killed_stack = stack_below(offset);
    auto const kills_union = x.kills.precise_union(killed_stack);
    return kills_union ? *kills_union : killed_stack;
  }();

  return GeneralEffects {
    x.loads | AHeapAny | ARdsAny | backtrace_locals(inst),
    x.stores | AHeapAny | ARdsAny,
    x.moves,
    new_kills
  };
}

//////////////////////////////////////////////////////////////////////

GeneralEffects may_load_store(AliasClass loads, AliasClass stores) {
  return GeneralEffects { loads, stores, AEmpty, AEmpty };
}

GeneralEffects may_load_store_kill(AliasClass loads,
                                   AliasClass stores,
                                   AliasClass kill) {
  return GeneralEffects { loads, stores, AEmpty, kill };
}

GeneralEffects may_load_store_move(AliasClass loads,
                                   AliasClass stores,
                                   AliasClass move) {
  assertx(move <= loads);
  return GeneralEffects { loads, stores, move, AEmpty };
}

//////////////////////////////////////////////////////////////////////

/*
 * Helper for iterator instructions.  They all affect some locals, but are
 * otherwise the same. Value iters touch one local; key-value iters touch two.
 */
GeneralEffects iter_effects(const IRInstruction& inst,
                            SSATmp* fp,
                            AliasClass locals) {
  auto const iters =
    AliasClass { aiter_all(fp, inst.extra<IterData>()->args.iterId) };
  return may_load_store_kill(
    iters | locals | AHeapAny,
    iters | locals | AHeapAny,
    AMIStateAny
  );
}

/*
 * Construct effects for InterpOne, using the information in its extra data.
 *
 * We always consider an InterpOne as potentially doing anything to the heap,
 * potentially re-entering, potentially raising warnings in the current frame,
 * potentially reading any locals, and potentially reading/writing any stack
 * location that isn't below the bottom of the stack.
 *
 * The extra data for the InterpOne comes with some additional information
 * about which local(s) it may modify, which is all we try to be more precise
 * about right now.
 */
GeneralEffects interp_one_effects(const IRInstruction& inst) {
  auto const extra  = inst.extra<InterpOne>();
  auto loads  = AHeapAny | AStackAny | ALocalAny | ARdsAny | livefp(inst);
  auto stores = AHeapAny | AStackAny | ARdsAny;
  if (extra->smashesAllLocals) {
    stores = stores | ALocalAny;
  } else {
    for (auto i = uint32_t{0}; i < extra->nChangedLocals; ++i) {
      stores = stores | ALocal { inst.src(1), extra->changedLocals[i].id };
    }
  }

  auto kills = AEmpty;
  if (isMemberBaseOp(extra->opcode)) {
    stores = stores | AMIStateAny;
    kills = kills | AMIStateAny;
  } else if (isMemberDimOp(extra->opcode) || isMemberFinalOp(extra->opcode)) {
    stores = stores | AMIStateAny;
    loads = loads | AMIStateAny;
  } else {
    kills = kills | AMIStateAny;
  }

  return may_load_store_kill(loads, stores, kills);
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Construct effects for member instructions that take &tvRef as their last
 * argument.
 *
 * These instructions never load tvRef, but they might store to it.
 */
MemEffects minstr_with_tvref(const IRInstruction& inst) {
  auto const srcs = inst.srcs();
  assertx(srcs.back()->isA(TMemToMISCell));
  auto const loads = AHeapAny | all_pointees(srcs.subpiece(0, srcs.size() - 1));
  auto const stores = AHeapAny | all_pointees(inst);
  return may_load_store(loads, stores);
}

//////////////////////////////////////////////////////////////////////

MemEffects memory_effects_impl(const IRInstruction& inst) {
  switch (inst.op()) {

  //////////////////////////////////////////////////////////////////////
  // Region exits

  // These exits don't leave the current php function, and could head to code
  // that could read or write anything as far as we know (including frame
  // locals).
  case ReqBindJmp:
    return ExitEffects {
      AUnknown,
      stack_below(inst.extra<ReqBindJmp>()->irSPOff)
    };
  case ReqRetranslate:
    return ExitEffects {
      AUnknown,
      stack_below(inst.extra<ReqRetranslate>()->offset)
    };
  case ReqRetranslateOpt:
    return ExitEffects {
      AUnknown,
      stack_below(inst.extra<ReqRetranslateOpt>()->offset)
    };
  case JmpSwitchDest:
    return ExitEffects {
      AUnknown,
      *stack_below(inst.extra<JmpSwitchDest>()->spOffBCFromIRSP).
        precise_union(AMIStateAny)
    };
  case JmpSSwitchDest:
    return ExitEffects {
      AUnknown,
      *stack_below(inst.extra<JmpSSwitchDest>()->offset).
        precise_union(AMIStateAny)
    };

  //////////////////////////////////////////////////////////////////////
  // Unusual instructions

  /*
   * The ReturnHook sets up the ActRec so the unwinder knows everything is
   * already released (i.e. it calls ar->setLocalsDecRefd()).
   *
   * The eval stack is also dead at this point (the return value is passed to
   * ReturnHook as src(1), and the ReturnHook may not access the stack).
   */
  case ReturnHook:
    return may_load_store_kill(
      AHeapAny | AActRec {inst.src(0)}, AHeapAny,
      *AStackAny.precise_union(ALocalAny)->precise_union(AMIStateAny)
    );

  // The suspend hooks can load anything (re-entering the VM), but can't write
  // to frame locals.
  case SuspendHookAwaitEF:
  case SuspendHookAwaitEG:
  case SuspendHookAwaitR:
  case SuspendHookCreateCont:
  case SuspendHookYield:
    // TODO: may-load here probably doesn't need to include ALocalAny normally.
    return may_load_store_kill(AUnknown, AHeapAny, AMIStateAny);

  /*
   * If we're returning from a function, it's ReturnEffects.  The RetCtrl
   * opcode also suspends resumables, which we model as having any possible
   * effects.
   */
  case RetCtrl:
    if (inst.extra<RetCtrl>()->suspendingResumed) {
      // Suspending can go anywhere, and doesn't even kill locals.
      return UnknownEffects {};
    }
    return ReturnEffects {
      AStackAny | ALocalAny | AMIStateAny | AFBasePtr
    };

  case AsyncFuncRet:
  case AsyncFuncRetSlow:
    return ReturnEffects { AStackAny | AMIStateAny | livefp(inst.src(1)) };

  case AsyncSwitchFast:
    // Suspending can go anywhere, and doesn't even kill locals.
    return UnknownEffects {};

  case GenericRetDecRefs:
    /*
     * The may-store information here is AUnknown: even though we know it
     * doesn't really "store" to the frame locals, the values that used to be
     * there are no longer available because they are DecRef'd, which we are
     * required to report as may-store information to make it visible to
     * reference count optimizations.  It's conceptually the same as if it was
     * storing an Uninit over each of the locals, but the stores of uninits
     * would be dead so we're not actually doing that.
     */
    return may_load_store_kill(AUnknown, AUnknown, AMIStateAny);

  case EndCatch: {
    auto const stack_kills = stack_below(inst.extra<EndCatch>()->offset);
    return ExitEffects {
      AUnknown,
      stack_kills | AMIStateTempBase | AMIStateBase
    };
  }

  case EnterTCUnwind: {
    auto const stack_kills = stack_below(inst.extra<EnterTCUnwind>()->offset);
    return ExitEffects {
      AUnknown,
      stack_kills | AMIStateTempBase | AMIStateBase
    };
  }

  /*
   * BeginInlining must always be the first instruction in the inlined call. It
   * defines a new FP for the callee but does not perform any stores or
   * otherwise initialize the FP.
   */
  case BeginInlining: {
    /*
     * SP relative offset of the firstin the inlined call.
     */
    auto inlineStackOff =
      inst.extra<BeginInlining>()->spOffset + kNumActRecCells;
    return may_load_store_kill(
      AEmpty,
      AEmpty,
      /*
       * This prevents stack slots from the caller from being sunk into the
       * callee. Note that some of these stack slots overlap with the frame
       * locals of the callee-- those slots are inacessible in the inlined
       * call as frame and stack locations may not alias.
       */
      stack_below(inlineStackOff)
    );
  }

  case EndInlining: {
    assertx(inst.src(0)->inst()->is(BeginInlining));
    auto const fp = inst.src(0);
    auto const callee = inst.src(0)->inst()->extra<BeginInlining>()->func;
    const AliasClass ar = AActRec { inst.src(0) };
    auto const locals = [&] () -> AliasClass {
      if (!callee->numLocals()) return AEmpty;
      return ALocal {fp, AliasIdSet::IdRange(0, callee->numLocals())};
    }();

    // NB: It's okay if the AliasIdSet for locals cannot be precise. We want to
    //     kill *every* local in the frame so there's nothing else that can
    //     accidentally be included in the set.
    return may_load_store_kill(AEmpty, AEmpty, ar | locals | AMIStateAny);
  }

  case InlineCall:
    return PureInlineCall {
      AFBasePtr,
      inst.src(0),

      // Right now when we "publish" a frame by storing it in rvmfp() we
      // implicitly depend on the AFFunc and AFMeta bits being stored. In the
      // future we may want to track this explicitly.
      //
      // We also need to ensure that all of our parent frames have this stored
      // this information. To achieve this we also register a load on AFBasePtr,
      // forcing them to also be published. Notice that we doin't actually
      // depend on this load to properly initialize m_sfp or rvmfp().
      AliasClass(AFFunc { inst.src(0) }) | AFMeta { inst.src(0) } | AFBasePtr
    };

  case InlineReturn:
    // Unlike InlineCall we don't need to explicitly require the frame be
    // published. Unlike InlineCall, however, it is not safe to "move" an
    // InlineReturn, it may only be killed once it has been made redundant by
    // the removal of its associated InlineCall.
    return PureInlineReturn { AFBasePtr, inst.src(0), inst.src(1) };

  case InterpOne:
    return interp_one_effects(inst);
  case InterpOneCF: {
    auto const extra = inst.extra<InterpOneData>();
    return ExitEffects {
      AUnknown,
      stack_below(extra->spOffset) | AMIStateAny
    };
  }

  case NativeImpl:
    return UnknownEffects {};

  // NB: on the failure path, these C++ helpers do a fixup and read frame
  // locals before they throw.  They can also invoke the user error handler and
  // go do whatever they want to non-frame locations.
  //
  // TODO(#5372569): if we combine dv inits into the same regions we could
  // possibly avoid storing KindOfUninits if we modify this.
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamFailHard:
  case VerifyParamRecDesc:
    return may_load_store(AUnknown, AHeapAny);
  // VerifyParamFail might coerce the parameter to the desired type rather than
  // throwing.
  case VerifyParamFail: {
    auto const extra = inst.extra<ParamWithTCData>();
    assertx(extra->paramId >= 0);
    auto const stores =
      AHeapAny |
      ALocal{inst.marker().fp(), safe_cast<uint32_t>(extra->paramId)};
    return may_load_store(AUnknown, stores);
  }
  case VerifyReifiedLocalType: {
    auto const extra = inst.extra<ParamData>();
    assertx(extra->paramId >= 0);
    auto const stores =
      AHeapAny |
      ALocal{inst.marker().fp(), safe_cast<uint32_t>(extra->paramId)};
    return may_load_store(AUnknown, stores);
  }
  // However the following ones can't read locals from our frame on the way
  // out, except as a side effect of raising a warning.
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyReifiedReturnType:
  case VerifyRetRecDesc:
    return may_load_store(AHeapAny | livefp(inst), AHeapAny);

  case VerifyRetFail:
  case VerifyRetFailHard:
    return may_load_store(AHeapAny | AStackAny | livefp(inst), AHeapAny);

  case VerifyPropCls:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case VerifyProp:
  case VerifyPropAll:
  case VerifyPropCoerce:
  case VerifyPropCoerceAll:
  case VerifyPropRecDesc:
    return may_load_store(AHeapAny | livefp(inst), AHeapAny);

  case ContEnter:
    {
      auto const extra = inst.extra<ContEnter>();
      return CallEffects {
        // Kills. Everything on the stack.
        stack_below(extra->spOffset) | AMIStateAny,
        // No inputs. The value being sent is passed explicitly.
        AEmpty,
        // ActRec. It is on the heap and we already implicitly assume that
        // CallEffects can perform arbitrary heap operations.
        AEmpty,
        // No outputs.
        AEmpty,
        // Locals.
        backtrace_locals(inst) | livefp(inst.src(1))
      };
    }

  case Call:
    {
      // If any frames in the inlined stack have metadata we need to materialize
      // all of the frames so that debug_backtrace() can find it.
      AliasClass ar = any_frame_has_metadata(inst.src(1))
        ? livefp(inst.src(1))
        : AliasClass(AActRec {inst.src(1)});
      auto const extra = inst.extra<Call>();
      return CallEffects {
        // Kills. Everything on the stack below the incoming parameters.
        stack_below(extra->spOffset) | AMIStateAny,
        // Input arguments.
        extra->numInputs() == 0 ? AEmpty : AStack::range(
          extra->spOffset,
          extra->spOffset + extra->numInputs()
        ),
        // ActRec.
        actrec(inst.src(0), extra->spOffset + extra->numInputs()),
        // Inout outputs.
        extra->numOut == 0 ? AEmpty : AStack::range(
          extra->spOffset + extra->numInputs() + kNumActRecCells,
          extra->spOffset + extra->numInputs() + kNumActRecCells +
            extra->numOut
        ),
        // Locals. We intentionally leave off a dependency on AFBasePtr to allow
        // store-elim to elide the new frame.
        backtrace_locals(inst) | ar
      };
    }

  case CallBuiltin:
    {
      AliasClass out_stk = AEmpty;
      auto const extra = inst.extra<CallBuiltin>();
      auto const callee = extra->callee;
      auto const stk = [&] () -> AliasClass {
        AliasClass ret = AEmpty;
        for (auto i = uint32_t{2}; i < inst.numSrcs(); ++i) {
          if (inst.src(i)->type() <= TPtrToCell) {
            auto const cls = pointee(inst.src(i));
            if (cls.maybe(AStackAny)) {
              ret = ret | cls;
              auto const paramOff = callee->isMethod() ? 3 : 2;
              if (i >= paramOff && callee->isInOut(i - paramOff)) {
                out_stk = out_stk | cls;
              }
            }
          }
        }
        return ret;
      }();
      auto const foldable = callee->isFoldable() ? AEmpty : ARdsAny;
      return may_load_store_kill(
        stk | AHeapAny | foldable,
        out_stk | AHeapAny | foldable,
        stack_below(extra->spOffset) | AMIStateAny
      );
    }

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateGen:
  case CreateAGen:
  case CreateAFWH: {
    auto const fp = canonical(inst.src(0));
    auto fpInst = fp->inst();
    auto const frame = [&] () -> AliasClass {
      if (fpInst->is(DefFP, DefFuncEntryFP)) return ALocalAny;
      assertx(fpInst->is(BeginInlining));
      auto const nlocals = fpInst->extra<BeginInlining>()->func->numLocals();
      return nlocals
        ? ALocal { fp, AliasIdSet::IdRange(0, nlocals)}
        : AEmpty;
    }();
    return may_load_store_move(
      frame | AActRec { fp },
      AHeapAny,
      frame
    );
  }

  // AGWH construction updates the AsyncGenerator object.
  case CreateAGWH:
    return may_load_store(AHeapAny | AActRec { inst.src(0) }, AHeapAny);

  case CreateAAWH:
    {
      auto const extra = inst.extra<CreateAAWH>();
      auto const frame = ALocal {
        inst.src(0),
        AliasIdSet {
          AliasIdSet::IdRange{ extra->first, extra->first + extra->count }
        }
      };
      return may_load_store(frame, AHeapAny);
    }

  case CountWHNotDone:
    {
      auto const extra = inst.extra<CountWHNotDone>();
      auto const frame = ALocal {
        inst.src(0),
        AliasIdSet {
          AliasIdSet::IdRange{ extra->first, extra->first + extra->count }
        }
      };
      return may_load_store(frame, AEmpty);
    }

  // This re-enters to call extension-defined instance constructors.
  case ConstructInstance:
    return may_load_store(AHeapAny, AHeapAny);

  // Closures don't ever throw or reenter on construction
  case ConstructClosure:
    return IrrelevantEffects{};

  case CheckStackOverflow:
  case CheckSurpriseFlagsEnter:
  case CheckSurpriseAndStack:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case LIterInit:
  case IterNext:
  case LIterNext: {
    auto const& args = inst.extra<IterData>()->args;
    assertx(!args.hasKey());
    auto const fp = inst.src(inst.op() == IterNext ? 0 : 1);
    AliasClass val = ALocal { fp, safe_cast<uint32_t>(args.valId) };
    return iter_effects(inst, fp, val);
  }

  case IterInitK:
  case LIterInitK:
  case IterNextK:
  case LIterNextK: {
    auto const& args = inst.extra<IterData>()->args;
    assertx(args.hasKey());
    auto const fp = inst.src(inst.op() == IterNextK ? 0 : 1);
    AliasClass key = ALocal { fp, safe_cast<uint32_t>(args.keyId) };
    AliasClass val = ALocal { fp, safe_cast<uint32_t>(args.valId) };
    return iter_effects(inst, fp, key | val);
  }

  case IterFree: {
    auto const base = aiter_base(inst.src(0), iterId(inst));
    return may_load_store(AHeapAny | base, AHeapAny);
  }

  case CheckIter: {
    auto const iter = inst.extra<CheckIter>()->iterId;
    return may_load_store(aiter_type(inst.src(0), iter), AEmpty);
  }

  case LdIterBase:
    return PureLoad { aiter_base(inst.src(0), iterId(inst)) };

  case LdIterPos:
    return PureLoad { aiter_pos(inst.src(0), iterId(inst)) };

  case LdIterEnd:
    return PureLoad { aiter_end(inst.src(0), iterId(inst)) };

  case StIterBase:
    return PureStore { aiter_base(inst.src(0), iterId(inst)), inst.src(1) };

  case StIterType: {
    auto const iter = inst.extra<StIterType>()->iterId;
    return PureStore { aiter_type(inst.src(0), iter), nullptr };
  }

  case StIterPos:
    return PureStore { aiter_pos(inst.src(0), iterId(inst)), inst.src(1) };

  case StIterEnd:
    return PureStore { aiter_end(inst.src(0), iterId(inst)), inst.src(1) };

  case KillIter: {
    auto const iters = aiter_all(inst.src(0), iterId(inst));
    return may_load_store_kill(AEmpty, AEmpty, iters);
  }

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate locals

  case StLoc:
    return PureStore {
      ALocal { inst.src(0), inst.extra<StLoc>()->locId },
      inst.src(1),
      nullptr
    };

  case StLocRange:
    {
      auto const extra = inst.extra<StLocRange>();
      auto acls = AEmpty;

      for (auto locId = extra->start; locId < extra->end; ++locId) {
        acls = acls | ALocal { inst.src(0), locId };
      }
      return PureStore { acls, inst.src(1), nullptr };
    }

  case LdLoc:
    return PureLoad { ALocal { inst.src(0), inst.extra<LocalId>()->locId } };

  case LdLocForeign:
    return may_load_store(ALocalAny, AEmpty);

  case CheckLoc:
    return may_load_store(
      ALocal { inst.src(0), inst.extra<LocalId>()->locId },
      AEmpty
    );

  //////////////////////////////////////////////////////////////////////
  // Pointer-based loads and stores

  case LdMem:
    return PureLoad { pointee(inst.src(0)) };
  case StMem:
    return PureStore { pointee(inst.src(0)), inst.src(1), inst.src(0) };

  case StImplicitContext:
    return may_load_store(AEmpty, AEmpty);

  case LdClsInitElem:
    return PureLoad { AHeapAny };

  case StClsInitElem:
    return PureStore { AHeapAny };

  case LdPairElem:
    return PureLoad { AHeapAny };

  case LdMBase:
    return PureLoad { AMIStateBase };

  case StMBase:
    return PureStore { AMIStateBase, inst.src(0), nullptr };

  case FinishMemberOp:
    return may_load_store_kill(AEmpty, AEmpty, AMIStateAny);

  case IsNTypeMem:
  case IsTypeMem:
  case CheckTypeMem:
  case CheckInitMem:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  case CheckRDSInitialized:
    return may_load_store(
      ARds { inst.extra<CheckRDSInitialized>()->handle },
      AEmpty
    );
  case MarkRDSInitialized:
    return may_load_store(
      AEmpty,
      ARds { inst.extra<MarkRDSInitialized>()->handle }
    );

  case InitProps:
    return may_load_store(
      AHeapAny,
      AHeapAny | ARds { inst.extra<InitProps>()->cls->propHandle() }
    );

  case InitSProps:
    return may_load_store(
      AHeapAny,
      AHeapAny | ARds { inst.extra<InitSProps>()->cls->sPropInitHandle() }
    );

  case LdARFunc:
  case LdClsFromClsMeth:
  case LdFuncFromClsMeth:
  case LdFuncFromRFunc:
  case LdGenericsFromRFunc:
  case LdClsFromRClsMeth:
  case LdFuncFromRClsMeth:
  case LdGenericsFromRClsMeth:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case InitObjProps:
    return may_load_store(AEmpty, APropAny);

  case InitObjMemoSlots:
    // Writes to memo slots, but these are not modeled.
    return IrrelevantEffects {};

  case LockObj:
    // Writes object attributes, but these are not modeled.
    return IrrelevantEffects {};

  // Loads $obj->trace, stores $obj->file and $obj->line.
  case InitThrowableFileAndLine:
    return may_load_store(AHeapAny, APropAny);

  //////////////////////////////////////////////////////////////////////
  // Array loads and stores

  case InitVecElem: {
    auto const arr = inst.src(0);
    auto const val = inst.src(1);
    auto const idx = inst.extra<InitVecElem>()->index;
    return PureStore { AElemI { arr, idx }, val, arr };
  }

  case InitDictElem: {
    auto const arr = inst.src(0);
    auto const val = inst.src(1);
    auto const key = inst.extra<InitDictElem>()->key;
    return PureStore { AElemS { arr, key }, val, arr };
  }

  case LdMonotypeDictVal: {
    // TODO(mcolavita): When we have a type-array-elem method to get the key
    // of an arbitrary array-like type, use that to narrow this load.
    return PureLoad { AElemAny };
  }

  case LdMonotypeVecElem:
  case LdVecElem: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    return PureLoad {
      key->hasConstVal() ? AElemI { base, key->intVal() } : AElemIAny
    };
  }

  case LdStructDictElem: {
    auto const base = inst.src(0);
    auto const key = inst.src(1);
    return PureLoad { AElemS { base, key->strVal() } };
  }

  case DictGetK:
  case KeysetGetK:
  case BespokeGet:
  case KeysetGetQuiet:
  case DictGetQuiet: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    assertx(key->type().subtypeOfAny(TInt, TStr));
    if (key->isA(TInt)) {
      return PureLoad {
        key->hasConstVal() ? AElemI { base, key->intVal() } : AElemIAny,
      };
    } else {
      return PureLoad {
        key->hasConstVal() ? AElemS { base, key->strVal() } : AElemSAny,
      };
    }
  }

  case DictIsset:
  case DictIdx:
  case KeysetIsset:
  case KeysetIdx:
  case AKExistsDict:
  case AKExistsKeyset:
  case BespokeGetThrow: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    assertx(key->type().subtypeOfAny(TInt, TStr));
    auto const elem = [&] {
      if (key->isA(TInt)) {
        return key->hasConstVal() ? AElemI { base, key->intVal() } : AElemIAny;
      } else {
        return key->hasConstVal() ? AElemS { base, key->strVal() } : AElemSAny;
      }
    }();
    return may_load_store(elem, AEmpty);
  }

  case InitVecElemLoop:
    {
      auto const extra = inst.extra<InitVecElemLoop>();
      auto const stack_in = AStack::range(
        extra->offset,
        extra->offset + static_cast<int32_t>(extra->size)
      );
      return may_load_store_move(stack_in, AElemIAny, stack_in);
    }

  case NewLoggingArray:
  case ProfileArrLikeProps:
    // These ops may read anything referenced by the input array or object,
    // but not any of the locals or stack frame slots.
    return may_load_store(AHeapAny | livefp(inst), AEmpty);

  case NewKeysetArray:
    {
      // NewKeysetArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewKeysetArray>();
      auto const stack_in = AStack::range(
        extra->offset,
        extra->offset + static_cast<int32_t>(extra->size)
      );
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case NewStructDict:
    {
      // NewStructDict reads elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructData>();
      auto const stack_in = AStack::range(
        extra->offset,
        extra->offset + static_cast<int32_t>(extra->numKeys)
      );
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case NewBespokeStructDict:
    {
      // NewBespokeStructDict reads elements from the stack, but writes to
      // a completely new array, so we can treat the stores as empty.
      auto const extra = inst.extra<NewBespokeStructDict>();
      auto const stack_in = AStack::range(
        extra->offset,
        extra->offset + static_cast<int32_t>(extra->numSlots)
      );
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case NewRecord:
    {
      auto const extra = inst.extra<NewStructData>();
      auto const stack_in = AStack::range(
        extra->offset,
        extra->offset + static_cast<int32_t>(extra->numKeys)
      );
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }
  case MemoGetStaticValue:
  case MemoGetLSBValue:
  case MemoGetInstanceValue:
    // Only reads the memo value (which isn't modeled here).
    return may_load_store(AEmpty, AEmpty);

  case MemoSetStaticValue:
  case MemoSetLSBValue:
  case MemoSetInstanceValue:
    // Writes to the memo value (which isn't modeled)
    return may_load_store(AEmpty, AEmpty);

  case MemoGetStaticCache:
  case MemoGetLSBCache:
  case MemoSetStaticCache:
  case MemoSetLSBCache: {
    // Reads some (non-zero) set of locals for keys, and reads/writes from the
    // memo cache (which isn't modeled).
    auto const extra = inst.extra<MemoCacheStaticData>();
    auto const frame = ALocal {
      inst.src(0),
      AliasIdSet{
        AliasIdSet::IdRange{
          extra->keys.first,
          extra->keys.first + extra->keys.count
        }
      }
    };

    return may_load_store(frame, AEmpty);
  }

  case MemoGetInstanceCache:
  case MemoSetInstanceCache: {
    // Reads some set of locals for keys, and reads/writes from the memo cache
    // (which isn't modeled).
    auto const extra = inst.extra<MemoCacheInstanceData>();
    auto const frame = [&]() -> AliasClass {
      // Unlike MemoGet/SetStaticCache, we can have an empty key range here.
      if (extra->keys.count == 0) return AEmpty;

      return ALocal {
        inst.src(0),
        AliasIdSet{
          AliasIdSet::IdRange{
            extra->keys.first,
            extra->keys.first + extra->keys.count
          }
        }
      };
    }();
    return may_load_store(frame, AEmpty);
  }

  case BespokeIterGetKey:
  case LdPtrIterKey:
    // Array element keys are not tracked by memory effects right now.
    return may_load_store(AEmpty, AEmpty);

  case LdPtrIterVal: {
    // NOTE: The type param for this op restricts the key, not the value.
    if (inst.typeParam() <= TInt) return PureLoad { AElemIAny };
    if (inst.typeParam() <= TStr) return PureLoad { AElemSAny };
    return PureLoad { AElemAny };
  }

  case BespokeIterGetVal:
    return may_load_store(AElemAny, AEmpty);

  case ElemDictK:
  case ElemKeysetK:
    return IrrelevantEffects {};

  case VecFirst: {
    auto const base = inst.src(0);
    return may_load_store(AElemI { base, 0 }, AEmpty);
  }
  case VecLast: {
    auto const base = inst.src(0);
    if (base->hasConstVal(TArrLike)) {
      auto const index = static_cast<int64_t>(base->arrLikeVal()->size() - 1);
      return may_load_store(AElemI { base, index }, AEmpty);
    }
    return may_load_store(AElemIAny, AEmpty);
  }
  case DictFirst:
  case DictLast:
  case KeysetFirst:
  case KeysetLast:
    return may_load_store(AElemAny, AEmpty);

  case DictFirstKey:
  case DictLastKey:
  case LdMonotypeDictTombstones:
  case LdMonotypeDictKey:
    return may_load_store(AEmpty, AEmpty);

  case CheckDictKeys:
  case CheckDictOffset:
  case CheckKeysetOffset:
  case CheckMissingKeyInArrLike:
  case ProfileDictAccess:
  case ProfileKeysetAccess:
  case CheckArrayCOW:
    return may_load_store(AHeapAny, AEmpty);

  case SameArrLike:
  case NSameArrLike:
    return may_load_store(AElemAny, AEmpty);

  case EqArrLike:
  case NeqArrLike: {
    if (inst.src(0)->type() <= TKeyset && inst.src(1)->type() <= TKeyset) {
      return may_load_store(AElemAny, AEmpty);
    } else {
      return may_load_store(AHeapAny, AHeapAny);
    }
  }

  case AKExistsObj:
    return may_load_store(AHeapAny, AHeapAny);

  //////////////////////////////////////////////////////////////////////
  // Member instructions

  case CheckMBase:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  /*
   * Various minstr opcodes that take a PtrToGen in src 0, which may or may not
   * point to a frame local or the evaluation stack. Some may read or write to
   * that pointer while some only read. They can all re-enter the VM and access
   * arbitrary heap locations.
   */
  case CGetElem:
  case IssetElem:
  case CGetProp:
  case CGetPropQ:
  case IssetProp:
    return may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny
    );

  /*
   * SetRange behaves like a simpler version of SetElem.
   */
  case SetRange:
  case SetRangeRev:
    return may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny | pointee(inst.src(0))
    );

  case IncDecElem:
  case IncDecProp:
  case SetElem:
  case SetNewElem:
  case SetOpElem:
  case SetOpProp:
  case SetProp:
  case SetNewElemDict:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case UnsetElem:
  case ElemVecD:
  case ElemVecU:
  case ElemDictD:
  case ElemDictU:
  case ElemKeysetU:
  case BespokeElem:
  case ElemX:
  case ElemDX:
  case ElemUX:
  case UnsetProp:
    // These member ops will load and store from the base lval which they
    // take as their first argument, which may point anywhere in the heap.
    return may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny | all_pointees(inst)
    );

  case ReserveVecNewElem:
    return may_load_store(AHeapAny, AHeapAny);

  /*
   * Intermediate minstr operations. In addition to a base pointer like the
   * operations above, these may take a pointer to MInstrState::tvRef, which
   * they may store to (but not read from).
   */
  case PropX:
  case PropDX:
  case PropQ:
    return minstr_with_tvref(inst);

  /*
   * Collection accessors can read from their inner array buffer, but stores
   * COW and behave as if they only affect collection memory locations.  We
   * don't track those, so it's returning AEmpty for now.
   */
  case MapIsset:
  case PairIsset:
  case VectorIsset:
    return may_load_store(AHeapAny, AEmpty /* Note */);
  case MapGet:
  case MapSet:
  case VectorSet:
    return may_load_store(AHeapAny, AEmpty /* Note */);

  case LdInitPropAddr:
    return may_load_store(
      AProp {
        inst.src(0),
        safe_cast<uint16_t>(inst.extra<LdInitPropAddr>()->index)
      },
      AEmpty
    );
  case LdInitRDSAddr:
    return may_load_store(
      ARds { inst.extra<LdInitRDSAddr>()->handle },
      AEmpty
    );

  //////////////////////////////////////////////////////////////////////
  // Instructions that allocate new objects, without reading any other memory
  // at all, so any effects they have on some types of memory locations we
  // track are isolated from anything else we care about.

  case NewClsMeth:
  case NewRClsMeth:
  case NewCol:
  case NewColFromArray:
  case NewPair:
  case NewInstanceRaw:
  case NewDictArray:
  case NewRFunc:
  case FuncCred:
  case AllocVec:
  case AllocStructDict:
  case AllocBespokeStructDict:
  case ConvDblToStr:
  case ConvIntToStr:
    return IrrelevantEffects {};

  case AllocObj:
    // AllocObj re-enters to call constructors, but if it weren't for that we
    // could ignore its loads and stores since it's a new object.
    return may_load_store(AEmpty, AEmpty);
  case AllocObjReified:
    // Similar to AllocObj but also stores the reification
    return may_load_store(AEmpty, AHeapAny);

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate the stack.

  case LdStk:
    return PureLoad { AStack::at(inst.extra<LdStk>()->offset) };

  case StStk:
    return PureStore {
      AStack::at(inst.extra<StStk>()->offset),
      inst.src(1),
      nullptr
    };

  case StOutValue:
    // Technically these writes affect the caller's stack, but there is no way
    // to actually observe them from within the callee. They can also only
    // occur once on any exit path from a function.
    return may_load_store(AEmpty, AEmpty);

  case LdOutAddr:
    return IrrelevantEffects{};

  case CheckStk:
    return may_load_store(
      AStack::at(inst.extra<CheckStk>()->offset),
      AEmpty
    );

  case DbgTraceCall:
    return may_load_store(AStackAny | ALocalAny, AEmpty);

  case Unreachable:
    // Unreachable code kills every memory location.
    return may_load_store_kill(AEmpty, AEmpty, AUnknown);

  case ResolveTypeStruct: {
    auto const extra = inst.extra<ResolveTypeStructData>();
    auto const stack_in = AStack::range(
      extra->offset,
      extra->offset + static_cast<int32_t>(extra->size)
    );
    return may_load_store(AliasClass(stack_in)|AHeapAny, AHeapAny);
  }

  case DefFP:
    return may_load_store(AFBasePtr, AFBasePtr);

  case DefFuncEntryFP:
    return may_load_store(livefp(inst.src(0)), livefp(inst.dst()));

  case LdARFlags:
    return PureLoad { AFMeta { inst.src(0) }};

  case LdUnitPerRequestFilepath:
    return PureLoad {
      ARds { inst.extra<LdUnitPerRequestFilepath>()->handle },
    };

  //////////////////////////////////////////////////////////////////////
  // Instructions that never read or write memory locations tracked by this
  // module.

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AdvanceDictPtrIter:
  case AndInt:
  case AssertType:
  case AssertLoc:
  case AssertStk:
  case AssertMBase:
  case BespokeIterEnd:
  case BespokeIterFirstPos:
  case BespokeIterLastPos:
  case DefFrameRelSP:
  case DefRegSP:
  case DefCallFlags:
  case DefCallFunc:
  case DefCallNumArgs:
  case DefCallCtx:
  case EndGuards:
  case EnterPrologue:
  case EqBool:
  case EqCls:
  case EqLazyCls:
  case EqRecDesc:
  case EqFunc:
  case EqStrPtr:
  case EqArrayDataPtr:
  case EqDbl:
  case EqInt:
  case EqPtrIter:
  case GetDictPtrIter:
  case GteBool:
  case GteInt:
  case GtBool:
  case GtInt:
  case Jmp:
  case JmpNZero:
  case JmpZero:
  case LdPropAddr:
  case LdStkAddr:
  case LdVecElemAddr:
  case LteBool:
  case LteDbl:
  case LteInt:
  case LtBool:
  case LtInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case DivDbl:
  case DivInt:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case NeqBool:
  case NeqDbl:
  case NeqInt:
  case SameObj:
  case NSameObj:
  case EqRes:
  case NeqRes:
  case CmpBool:
  case CmpInt:
  case CmpDbl:
  case SubDbl:
  case SubInt:
  case SubIntO:
  case XorBool:
  case XorInt:
  case OrInt:
  case AssertNonNull:
  case CheckNonNull:
  case CheckNullptr:
  case CheckImplicitContextNull:
  case CheckSmashableClass:
  case Ceil:
  case Floor:
  case DefLabel:
  case CheckInit:
  case Nop:
  case Mod:
  case Conjure:
  case ConjureUse:
  case EndBlock:
  case ConvBoolToInt:
  case ConvBoolToDbl:
  case DefConst:
  case LdLocAddr:
  case Sqrt:
  case Shl:
  case Shr:
  case Lshr:
  case IsNType:
  case IsType:
  case Mov:
  case ConvDblToBool:
  case ConvDblToInt:
  case DblAsBits:
  case LdMIStateAddr:
  case LdClsCns:
  case LdSubClsCns:
  case LdSubClsCnsClsName:
  case LdTypeCns:
  case CheckSubClsCns:
  case LdClsCnsVecLen:
  case FuncHasAttr:
  case ClassHasAttr:
  case LdFuncRequiredCoeffects:
  case IsFunReifiedGenericsMatched:
  case JmpPlaceholder:
  case LdSmashable:
  case LdSmashableFunc:
  case LdRDSAddr:
  case CheckRange:
  case ProfileType:
  case LdIfaceMethod:
  case InstanceOfIfaceVtable:
  case IsTypeStructCached:
  case LdTVAux:
  case GetTime:
  case GetTimeNs:
  case ProfileInstanceCheck:
  case Select:
  case LookupSPropSlot:
  case ConvPtrToLval:
  case ProfileProp:
  case ProfileIsTypeStruct:
  case LdLazyClsName:
  case DirFromFilepath:
    return IrrelevantEffects {};
  case MethodExists:
    if (!RO::EvalRaiseOnCaseInsensitiveLookup) return IrrelevantEffects {};
    return may_load_store(AHeapAny, AHeapAny);

  case LookupClsCns:
  case LookupClsCtxCns:
    return may_load_store(AEmpty, AEmpty);

  case StClosureArg:
    return PureStore {
      AProp {
        inst.src(0),
        safe_cast<uint16_t>(inst.extra<StClosureArg>()->index)
      },
      inst.src(1),
      inst.src(0)
    };

  case StArResumeAddr:
    return PureStore { AFMeta { inst.src(0) }, nullptr };

  case StContArKey:
  case StContArValue:
  case StContArState:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case LdContArKey:
  case LdContArValue:
    return may_load_store(AFContext { inst.src(0) }, AEmpty);

  case LdFrameThis:
  case LdFrameCls:
    return PureLoad { AFContext { inst.src(0) }};

  case StFrameCtx:
    return PureStore { AFContext { inst.src(0) }, inst.src(1) };

  case StFrameFunc:
    return PureStore { AFFunc { inst.src(0) }, nullptr };

  case StFrameMeta:
    return PureStore { AFMeta { inst.src(0) }, nullptr };

  case EagerSyncVMRegs:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Instructions that technically do some things w/ memory, but not in any way
  // we currently care about.  They however don't return IrrelevantEffects
  // because we assume (in refcount-opts) that IrrelevantEffects instructions
  // can't even inspect Countable reference count fields, and several of these
  // can.  All GeneralEffects instructions are assumed to possibly do so.

  case DecRefNZ:
  case ProfileDecRef:
  case AFWHBlockOn:
  case AFWHPushTailFrame:
  case IncRef:
  case LdClosureCls:
  case LdClosureThis:
  case LdRetVal:
  case ConvStrToInt:
  case ConvResToInt:
  case OrdStr:
  case ChrInt:
  case CreateSSWH:
  case CheckInOuts:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case CheckCold:
  case ContValid:
  case ContStarted:
  case IncProfCounter:
  case IncCallCounter:
  case IncStat:
  case ContPreNext:
  case ContStartedCheck:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case HasReifiedGenerics:
  case InstanceOf:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfIface:
  case InstanceOfRecDesc:
  case InterfaceSupportsArrLike:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case IsLegacyArrLike:
  case IsWaitHandle:
  case IsCol:
  case HasToString:
  case DbgAssertRefCount:
  case DbgCheckLocalsDecRefd:
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case EqStr:
  case NeqStr:
  case SameStr:
  case NSameStr:
  case CmpStr:
  case GtStrInt:
  case GteStrInt:
  case LtStrInt:
  case LteStrInt:
  case EqStrInt:
  case NeqStrInt:
  case CmpStrInt:
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case CmpRes:
  case LdBindAddr:
  case LdSSwitchDestFast:
  case RBTraceEntry:
  case RBTraceMsg:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvResToDbl:
  case ExtendsClass:
  case LdUnwinderValue:
  case LdClsName:
  case LdAFWHActRec:
  case LdContActRec:
  case LdContField:
  case LdContResumeAddr:
  case LdClsCachedSafe:
  case LdRecDescCachedSafe:
  case LdClsInitData:
  case UnwindCheckSideExit:
  case LdCns:
  case LdFuncVecLen:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case LdClsTypeCns:
  case LdClsTypeCnsClsName:
  case ProfileSwitchDest:
  case LdFuncCls:
  case LdFuncNumParams:
  case LdFuncName:
  case LdMethCallerName:
  case LdObjClass:
  case LdRecDesc:
  case LdObjInvoke:
  case LdObjMethodD:
  case LdObjMethodS:
  case LdStrLen:
  case StringIsset:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdWHResult:
  case LdWHState:
  case LdWHNotDone:
  case LookupClsMethod:
  case LookupClsRDS:
  case StrictlyIntegerConv:
  case DbgAssertFunc:
  case ProfileCall:
  case ProfileMethod:
    return may_load_store(AEmpty, AEmpty);

  case LogArrayReach:
  case LogGuardFailure:
    return may_load_store(AHeapAny, AEmpty);

  // Some that touch memory we might care about later, but currently don't:
  case ColIsEmpty:
  case ColIsNEmpty:
  case ConvTVToBool:
  case ConvObjToBool:
  case CountCollection:
  case LdVectorSize:
  case CheckVecBounds:
  case LdColVec:
  case LdColDict:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch most heap things.  They
  // also may generally write to the eval stack below an offset (see
  // alias-class.h above AStack for more).

  case DecRef:
    return may_load_store(AHeapAny, AHeapAny);

  case GetMemoKey:
    return may_load_store(AHeapAny, AHeapAny);

  case GetMemoKeyScalar:
    return IrrelevantEffects{};

  // These opcodes raise notices if we access $GLOBALS['GLOBALS'],
  // or, due to case insensitivity, $GLOBALS['gLoBAls'], etc.
  case BaseG:
  case LdGblAddr:
  case LdGblAddrDef: {
    auto const base = inst.op() == BaseG
      ? may_load_store(AHeapAny, AHeapAny)
      : may_load_store(AEmpty, AEmpty);
    auto const& key = inst.src(0)->type();
    auto const safe = key.hasConstVal() && !s_GLOBALS.equal(key.strVal());
    return safe ? base : may_reenter(inst, base);
  }

  case LdClsCtor:
    return may_load_store(AEmpty, AEmpty);

  case RaiseCoeffectsCallViolation:
  case RaiseCoeffectsFunParamCoeffectRulesViolation:
  case RaiseCoeffectsFunParamTypeViolation:
    return may_load_store(AEmpty, AEmpty);

  case LdClsPropAddrOrNull:   // may run 86{s,p}init, which can autoload
  case LdClsPropAddrOrRaise:  // raises errors, and 86{s,p}init
  case Clone:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case RaiseClsMethPropConvertNotice:
  case ThrowUninitLoc:
  case RaiseUndefProp:
  case RaiseTooManyArg:
  case RaiseError:
  case RaiseNotice:
  case RaiseWarning:
  case RaiseHackArrCompatNotice:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseStrToClassNotice:
  case CheckClsMethFunc:
  case CheckClsReifiedGenericMismatch:
  case CheckFunReifiedGenericMismatch:
  case ConvTVToStr:
  case ConvObjToStr:
  case Count:      // re-enters on CountableClass
  case GtObj:
  case GteObj:
  case LtObj:
  case LteObj:
  case EqObj:
  case NeqObj:
  case CmpObj:
  case GtArrLike:
  case GteArrLike:
  case LtArrLike:
  case LteArrLike:
  case CmpArrLike:
  case OODeclExists:
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFunc:         // autoload
  case LdFuncCached:   // autoload
  case LdRecDescCached:    // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case InitClsCns:      // autoload
  case InitSubClsCns: // May run 86cinit
  case ProfileSubClsCns: // May run 86cinit
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCnsE:
  case LookupFuncCached: // autoload
  case StringGet:      // raise_notice
  case OrdStrIdx:      // raise_notice
  case AddNewElemKeyset:   // can re-enter
  case DictGet:
  case KeysetGet:
  case VecSet:
  case DictSet:
  case BespokeSet:
  case BespokeAppend:
  case StructDictSet:
  case ConcatStrStr:
  case PrintStr:
  case PrintBool:
  case PrintInt:
  case ConcatIntStr:
  case ConcatStrInt:
  case LdSSwitchDestSlow:
  case ConvObjToDbl:
  case ConvObjToInt:
  case ConvTVToInt:
  case ConcatStr3:
  case ConcatStr4:
  case ConvTVToDbl:
  case ConvObjToVec:
  case ConvObjToDict:
  case ConvObjToKeyset:
  case ThrowOutOfBounds:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowCallReifiedFunctionWithoutGenerics:
  case ThrowDivisionByZeroException:
  case ThrowHasThisNeedStatic:
  case ThrowLateInitPropError:
  case ThrowMissingArg:
  case ThrowMissingThis:
  case ThrowParameterWrongType:
  case ThrowParamInOutMismatch:
  case ThrowParamInOutMismatchRange:
  case ArrayMarkLegacyShallow:
  case ArrayMarkLegacyRecursive:
  case ThrowMustBeMutableException:
  case ThrowMustBeReadOnlyException:
  case ArrayUnmarkLegacyShallow:
  case ArrayUnmarkLegacyRecursive:
  case SetOpTV:
  case OutlineSetOp:
  case ThrowAsTypeStructException:
  case PropTypeRedefineCheck: // Can raise and autoload
  case HandleRequestSurprise:
  case BespokeEscalateToVanilla:
    return may_load_store(AHeapAny, AHeapAny);

  case AddNewElemVec:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case IsTypeStruct:
  case RecordReifiedGenericsAndGetTSList:
    return may_load_store(AElemAny, AEmpty);

  case ConvArrLikeToVec:
  case ConvArrLikeToDict:
  case ConvArrLikeToKeyset: // Decrefs input values
  case ConvClsMethToDict:
  case ConvClsMethToKeyset:
  case ConvClsMethToVec:
    return may_load_store(AElemAny, AEmpty);

  // debug_backtrace() traverses stack and WaitHandles on the heap.
  case DebugBacktrace:
  case DebugBacktraceFast:
    return may_load_store(AHeapAny|ALocalAny|AStackAny, AHeapAny);

  // This instruction doesn't touch memory we track, except that it may
  // re-enter to construct php Exception objects.  During this re-entry anything
  // can happen (e.g. a surprise flag check could cause a php signal handler to
  // run arbitrary code).
  case AFWHPrepareChild:
    return may_load_store(AActRec { inst.src(0) }, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // The following instructions are used for debugging memory optimizations.
  // We can't ignore them, because they can prevent future optimizations;
  // eg t1 = LdStk<N>; DbgTrashStk<N>; StStk<N> t1
  // If we ignore the DbgTrashStk it looks like the StStk is redundant

  case DbgTrashStk:
    return GeneralEffects {
      AEmpty, AEmpty, AEmpty,
      AStack::at(inst.extra<DbgTrashStk>()->offset)
    };
  case DbgTrashFrame:
    return GeneralEffects {
      AEmpty, AEmpty, AEmpty,
      actrec(inst.src(0), inst.extra<DbgTrashFrame>()->offset)
    };
  case DbgTrashMem:
    return GeneralEffects {
      AEmpty, AEmpty, AEmpty,
      pointee(inst.src(0))
    };

  //////////////////////////////////////////////////////////////////////

  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool check_effects(const IRInstruction& inst, MemEffects me) {
  SCOPE_ASSERT_DETAIL("Memory Effects") {
    return folly::sformat("  inst: {}\n  effects: {}\n", inst, show(me));
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      obj->type() <= TObj,
      "Non obj pointer in memory effects"
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const pr = a.prop())  check_obj(pr->obj);
  };

  match<void>(
    me,
    [&] (GeneralEffects x) {
      check(x.loads);
      check(x.stores);
      check(x.moves);
      check(x.kills);

      // Locations may-moved always should also count as may-loads.
      always_assert(x.moves <= x.loads);

      if (inst.mayRaiseErrorWithSources()) {
        // Any instruction that can raise an error can run a user error handler
        // and have arbitrary effects on the heap.
        always_assert(AHeapAny <= x.loads);
        always_assert(AHeapAny <= x.stores);
        /*
         * They also ought to kill /something/ on the stack, because of
         * possible re-entry.  It's not incorrect to leave things out of the
         * kills set, but this assertion is here because we shouldn't do it on
         * purpose, so this is here until we have a reason not to assert it.
         *
         * The mayRaiseError instructions should all be going through
         * may_reenter right now, which will kill the stack below the re-entry
         * depth---unless the marker for `inst' doesn't have an fp set.
         */
        always_assert(inst.marker().fp() == nullptr ||
                      AStackAny.maybe(x.kills));
      }
    },
    [&] (PureLoad x)         { check(x.src); },
    [&] (PureStore x)        { check(x.dst); },
    [&] (ExitEffects x)      { check(x.live); check(x.kills); },
    [&] (IrrelevantEffects)  {},
    [&] (UnknownEffects)     {},
    [&] (CallEffects x)      { check(x.kills);
                               check(x.inputs);
                               check(x.actrec);
                               check(x.outputs);
                               check(x.locals); },
    [&] (PureInlineCall x)   { check(x.base);
                               check(x.actrec); },
    [&] (PureInlineReturn x) { check(x.base); },
    [&] (ReturnEffects x)    { check(x.kills); }
  );

  return true;
}

//////////////////////////////////////////////////////////////////////

}

MemEffects memory_effects(const IRInstruction& inst) {
  auto const inner = memory_effects_impl(inst);
  auto const ret = [&] () -> MemEffects {
    if (!inst.mayRaiseErrorWithSources()) return inner;

    auto fail = [&] {
      always_assert_flog(
        false,
        "Instruction {} has effects {}, but has been marked as MayRaiseError "
        "and must use a UnknownEffects, GeneralEffects, or CallEffects type.",
        inst,
        show(inner)
      );
      return may_load_store(AUnknown, AUnknown);
    };

    // Calls are implicitly MayRaise, all other instructions must use the
    // GeneralEffects or UnknownEffects class of memory effects
    return match<MemEffects>(
      inner,
      [&] (GeneralEffects x)   { return may_reenter(inst, x); },
      [&] (CallEffects x)      { return x; },
      [&] (UnknownEffects x)   { return x; },
      [&] (PureLoad)           { return fail(); },
      [&] (PureStore)          { return fail(); },
      [&] (ExitEffects)        { return fail(); },
      [&] (PureInlineCall)     { return fail(); },
      [&] (PureInlineReturn)   { return fail(); },
      [&] (IrrelevantEffects)  { return fail(); },
      [&] (ReturnEffects)      { return fail(); }
    );
  }();
  assertx(check_effects(inst, ret));
  return ret;
}

AliasClass pointee(const SSATmp* tmp) {
  return pointee(tmp, nullptr);
}

//////////////////////////////////////////////////////////////////////

MemEffects canonicalize(MemEffects me) {
  using R = MemEffects;
  return match<R>(
    me,
    [&] (GeneralEffects x) -> R {
      return GeneralEffects {
        canonicalize(x.loads),
        canonicalize(x.stores),
        canonicalize(x.moves),
        canonicalize(x.kills)
      };
    },
    [&] (PureLoad x) -> R {
      return PureLoad { canonicalize(x.src) };
    },
    [&] (PureStore x) -> R {
      return PureStore { canonicalize(x.dst), x.value, x.dep };
    },
    [&] (ExitEffects x) -> R {
      return ExitEffects { canonicalize(x.live), canonicalize(x.kills) };
    },
    [&] (PureInlineCall x) -> R {
      return PureInlineCall {
        canonicalize(x.base),
        x.fp,
        canonicalize(x.actrec)
      };
    },
    [&] (PureInlineReturn x) -> R {
      return PureInlineReturn {
        canonicalize(x.base),
        x.calleeFp,
        x.callerFp
      };
    },
    [&] (CallEffects x) -> R {
      return CallEffects {
        canonicalize(x.kills),
        canonicalize(x.inputs),
        canonicalize(x.actrec),
        canonicalize(x.outputs),
        canonicalize(x.locals)
      };
    },
    [&] (ReturnEffects x) -> R {
      return ReturnEffects { canonicalize(x.kills) };
    },
    [&] (IrrelevantEffects x) -> R { return x; },
    [&] (UnknownEffects x)    -> R { return x; }
  );
}

//////////////////////////////////////////////////////////////////////

std::string show(MemEffects effects) {
  using folly::sformat;
  return match<std::string>(
    effects,
    [&] (GeneralEffects x) {
      return sformat("mlsmk({} ; {} ; {} ; {})",
        show(x.loads),
        show(x.stores),
        show(x.moves),
        show(x.kills)
      );
    },
    [&] (ExitEffects x) {
      return sformat("exit({} ; {})", show(x.live), show(x.kills));
    },
    [&] (PureInlineCall x) {
      return sformat("inline_call({} ; {})",
        show(x.base),
        show(x.actrec)
      );
    },
    [&] (PureInlineReturn x) {
      return sformat("inline_return({})", show(x.base));
    },
    [&] (CallEffects x) {
      return sformat("call({} ; {} ; {} ; {} ; {})",
        show(x.kills),
        show(x.inputs),
        show(x.actrec),
        show(x.outputs),
        show(x.locals)
      );
    },
    [&] (PureLoad x)        { return sformat("ld({})", show(x.src)); },
    [&] (PureStore x)       { return sformat("st({})", show(x.dst)); },
    [&] (ReturnEffects x)   { return sformat("return({})", show(x.kills)); },
    [&] (IrrelevantEffects) { return "IrrelevantEffects"; },
    [&] (UnknownEffects)    { return "UnknownEffects"; }
  );
}

//////////////////////////////////////////////////////////////////////

}}
