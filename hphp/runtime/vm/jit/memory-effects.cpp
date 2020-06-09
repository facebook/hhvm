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
#include "hphp/runtime/vm/jit/dce.h"
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
          ALocal {
            sinst->src(0),
            sinst->extra<LdLocAddr>()->locId,
            sinst->extra<LdLocAddr>()->fpOffset
          }
        };
      }
      return ALocalAny;
    }

    if (type <= TMemToStkCell) {
      if (sinst->is(LdStkAddr)) {
        return AliasClass {
          AStack { sinst->src(0), sinst->extra<LdStkAddr>()->offset, 1 }
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
      if (sinst->is(LdPackedArrayDataElemAddr)) return elem();
      return AElemAny;
    }

    // The result of ElemArray{,W,U} is either the address of an array element,
    // or &immutable_null_base.
    if (type <= TMemToMembCell) {
      // Takes a PtrToCell as its first operand, so we can't easily grab an
      // array base.
      if (sinst->is(ElemArrayU, ElemVecU, ElemDictU, ElemKeysetU)) {
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
template<typename Off>
AliasClass stack_below(SSATmp* base, Off offset) {
  return AStack { base, offset, std::numeric_limits<int32_t>::max() };
}

//////////////////////////////////////////////////////////////////////

// Return an AliasClass representing an entire ActRec at base + offset.
AliasClass actrec(SSATmp* base, IRSPRelOffset offset) {
  return AStack {
    base,
    // The offset is the bottom of where the ActRec is stored, but AliasClass
    // needs an offset to the highest cell.
    offset + int32_t{kNumActRecCells} - 1,
    int32_t{kNumActRecCells}
  };
}

/*
 * Returning from an inlined function kills the stack below the
 * function's ActRec and all locals in the inlined frame.
 */
InlineExitEffects inline_exit_effects(SSATmp* fp) {
  fp = canonical(fp);
  auto fpInst = fp->inst();
  if (UNLIKELY(fpInst->is(DefLabel))) fpInst = resolveFpDefLabel(fp);
  assertx(fpInst && fpInst->is(DefInlineFP));
  auto const func = fpInst->extra<DefInlineFP>()->target;
  auto const frame = [&] () -> AliasClass {
    if (!func->numLocals()) return AEmpty;
    return ALocal {fp, AliasIdSet::IdRange(0, func->numLocals())};
  }();
  auto const stack = stack_below(fp, FPRelOffset{kNumActRecCells - 1});
  return InlineExitEffects{ stack, frame, AMIStateAny };
}

/*
 * Returning from an inlined function where we've elided the frame
 * have the same effects as the non-elided case, but we must calculate
 * the offsets a different way (since we have no frame pointer).
 */
InlineExitEffects inline_exit_effects_no_frame(const IRInstruction& inst) {
  assertx(inst.is(InlineReturnNoFrame));

  // We don't have a frame pointer for this frame, but
  // InlineReturnNoFrame has the SP relative offset of where the frame
  // would be. Use this to calculate the FPRelOffset from the
  // outermost frame pointer, which is what ALocal and AStack would
  // canonicalize to if given a frame pointer anyways.
  auto const spInst = inst.src(0)->inst();
  assertx(spInst->is(DefFrameRelSP, DefRegSP));
  auto const spOffset = spInst->extra<FPInvOffsetData>()->offset;
  auto const frameOffset =
    inst.extra<InlineReturnNoFrame>()->offset.to<FPRelOffset>(spOffset);

  auto const stack = AStack {
    frameOffset + int32_t{kNumActRecCells} - 1,
    std::numeric_limits<int32_t>::max()
  };
  auto const frame = [&] () -> AliasClass {
    auto const func = inst.marker().func();
    if (!func->numLocals()) return AEmpty;
    return ALocal { frameOffset, AliasIdSet::IdRange(0, func->numLocals()) };
  }();

  return InlineExitEffects { stack, frame, AMIStateAny };
}

//////////////////////////////////////////////////////////////////////

// Determine an AliasClass representing any locals in the instruction's frame
// which might be accessed via debug_backtrace().

AliasClass backtrace_locals(const IRInstruction& inst) {
  auto const func = [&]() -> const Func* {
    auto fp = inst.marker().fp();
    if (!fp) return nullptr;
    fp = canonical(fp);
    auto fpInst = fp->inst();
    if (UNLIKELY(fpInst->is(DefLabel))) {
      fpInst = resolveFpDefLabel(fp);
      assertx(fpInst);
    }
    if (fpInst->is(DefFP)) return fpInst->marker().func();
    if (fpInst->is(DefFuncEntryFP)) return fpInst->extra<DefFuncEntryFP>()->func;
    if (fpInst->is(DefInlineFP)) return fpInst->extra<DefInlineFP>()->target;
    always_assert(false);
  }();

  // Either there's no func or no frame-pointer. Either way, be conservative and
  // assume anything can be read. This can happen in test code, for instance.
  if (!func) return ALocalAny;

  auto const add86meta = [&] (AliasClass ac) {
    // The 86metadata variable can also exist in a VarEnv, but accessing that is
    // considered a heap effect, so we can ignore it.
    auto const local = func->lookupVarId(s_86metadata.get());
    if (local == kInvalidId) return ac;
    return ac | ALocal { inst.marker().fp(), (uint32_t)local };
  };

  auto ac = AEmpty;
  auto const numParams = func->numParams();

  if (!RuntimeOption::EnableArgsInBacktraces) return add86meta(ac);

  if (func->hasReifiedGenerics()) {
    // First non param local contains reified generics
    AliasIdSet reifiedgenerics{ AliasIdSet::IdRange{numParams, numParams + 1} };
    ac |= ALocal { inst.marker().fp(), reifiedgenerics };
  }

  if (func->cls() && func->cls()->hasReifiedGenerics()) {
    // There is no way to access the SSATmp for ObjectData of `this` here,
    // so be very pessimistic
    ac |= APropAny;
  }

  if (!numParams) return add86meta(ac);

  AliasIdSet params{ AliasIdSet::IdRange{0, numParams} };
  return add86meta(ac | ALocal { inst.marker().fp(), params });
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
 *
 * Important note: because of the `kills' set modifications, an instruction may
 * not report that it can re-enter if it actually can't.  The reason this can
 * go wrong is that if the instruction was in an inlined function, if we've
 * removed the DefInlineFP its spOff will not be meaningful.  In this case
 * the `kills' set will refer to the wrong stack locations.  This
 * means instructions that can re-enter must have catch traces.
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

    auto const killed_stack = stack_below(
      inst.marker().fp(),
      -inst.marker().spOff() - 1
    );
    auto const kills_union = x.kills.precise_union(killed_stack);
    return kills_union ? *kills_union : killed_stack;
  }();

  return GeneralEffects {
    x.loads | AHeapAny | backtrace_locals(inst),
    x.stores | AHeapAny,
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
  auto loads  = AHeapAny | AStackAny | ALocalAny | ARdsAny;
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
      stack_below(inst.src(0), inst.extra<ReqBindJmp>()->irSPOff - 1)
    };
  case ReqRetranslate:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(0), inst.extra<ReqRetranslate>()->irSPOff - 1)
    };
  case ReqRetranslateOpt:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(0), inst.extra<ReqRetranslateOpt>()->offset - 1)
    };
  case JmpSwitchDest:
    return ExitEffects {
      AUnknown,
      *stack_below(inst.src(1),
                   inst.extra<JmpSwitchDest>()->spOffBCFromIRSP - 1).
        precise_union(AMIStateAny)
    };
  case JmpSSwitchDest:
    return ExitEffects {
      AUnknown,
      *stack_below(inst.src(1),
                   inst.extra<JmpSSwitchDest>()->offset - 1).
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
      AHeapAny, AHeapAny,
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
      AStackAny | ALocalAny | AMIStateAny
    };

  case AsyncFuncRet:
  case AsyncFuncRetSlow:
    return ReturnEffects { AStackAny | AMIStateAny };

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
    auto const stack_kills = stack_below(
      inst.src(1),
      inst.extra<EndCatch>()->offset - 1
    );
    return ExitEffects {
      AUnknown,
      stack_kills | AMIStateTempBase | AMIStateBase
    };
  }

  case EnterTCUnwind: {
    auto const stack_kills = stack_below(
      inst.marker().fp(),
      -inst.marker().spOff() - 1
    );
    return ExitEffects {
      AUnknown,
      stack_kills | AMIStateTempBase | AMIStateBase
    };
  }

  /*
   * DefInlineFP has some special treatment here.
   *
   * It's logically `publishing' a pointer to a pre-live ActRec, making it
   * live.  It doesn't actually load from this ActRec, but after it's done this
   * the set of things that can load from it is large enough that the easiest
   * way to model this is to consider it as a load on behalf of `publishing'
   * the ActRec.  Once it's published, it's a live activation record, and
   * doesn't get written to as if it were a stack slot anymore (we've
   * effectively converted AStack locations into a frame until the
   * InlineReturn).
   *
   * Note: We may push the publishing of the inline frame below the start of
   * the inline function so that we can avoid spilling the inline frame in the
   * common case. Because of this we cannot add the stack positions within the
   * inline function to the kill set here as they may be live having been stored
   * on the main trace.
   *
   * TODO(#3634984): Additionally, DefInlineFP is marking may-load on all the
   * locals of the outer frame.  This is probably not necessary anymore, but we
   * added it originally because a store sinking prototype needed to know it
   * can't push StLocs past a DefInlineFP, because of reserved registers.
   * Right now it's just here because we need to think about and test it before
   * removing that set.
   */
  case DefInlineFP: {
    /*
     * Notice that the stack positions and frame locals described here are
     * exactly the set of alias locations that are about to be overlapping
     * inside the inlined frame.
     */
    auto const func = inst.extra<DefInlineFP>()->target;
    AliasClass stack = func->numLocals()
      ? AStack{inst.dst(), FPRelOffset{-1}, func->numLocals()}
      : AEmpty;
    AliasClass frame = func->numLocals()
      ? ALocal{inst.dst(), AliasIdSet::IdRange(0, func->numLocals())}
      : AEmpty;
    AliasClass actrec = AStack {
      inst.src(0),
      inst.extra<DefInlineFP>()->spOffset + int32_t{kNumActRecCells} - 1,
      int32_t{kNumActRecCells}
    };
    return InlineEnterEffects{ stack, frame, actrec };
  }

  /*
   * BeginInlining is similar to DefInlineFP, however, it must always be the
   * first instruction in the inlined call and has no effect serving only as
   * a marker to memory effects that the stack cells within the inlined call
   * are now dead.
   */
  case BeginInlining: {
    /*
     * SP relative offset of the first non-frame cell within the inlined call.
     */
    auto inlineStackOff = inst.extra<BeginInlining>()->offset;
    return may_load_store_kill(
      AEmpty,
      AEmpty,
      /*
       * This prevents stack slots from the caller from being sunk into the
       * callee. Note that some of these stack slots overlap with the frame
       * locals of the callee-- those slots are inacessible in the inlined
       * call as frame and stack locations may not alias.
       */
      stack_below(inst.src(0), inlineStackOff)
    );
  }

  case InlineSuspend:
  case InlineReturn: {
    return inline_exit_effects(inst.src(0));
  }

  case InlineReturnNoFrame:
    return inline_exit_effects_no_frame(inst);

  case SyncReturnBC: {
    auto const spOffset = inst.extra<SyncReturnBC>()->spOffset;
    auto const arStack = actrec(inst.src(0), spOffset);
    // This instruction doesn't actually load but DefInlineFP cannot be pushed
    // past it
    return may_load_store(arStack, arStack);
  }

  case InterpOne:
    return interp_one_effects(inst);
  case InterpOneCF:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(1), -inst.marker().spOff() - 1) | AMIStateAny
    };

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
    return may_load_store(AHeapAny, AHeapAny);

  case VerifyRetFail:
  case VerifyRetFailHard:
    return may_load_store(AHeapAny | AStackAny, AHeapAny);

  case VerifyPropCls:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case VerifyProp:
  case VerifyPropAll:
  case VerifyPropCoerce:
  case VerifyPropCoerceAll:
  case VerifyPropRecDesc:
    return may_load_store(AHeapAny, AHeapAny);

  case CallUnpack:
    {
      auto const extra = inst.extra<CallUnpack>();
      return CallEffects {
        // Kills. Everything on the stack below the incoming parameters.
        stack_below(inst.src(0), extra->spOffset - 1) | AMIStateAny,
        // Input arguments.
        extra->numInputs() == 0 ? AEmpty : AStack {
          inst.src(0),
          extra->spOffset + extra->numInputs() - 1,
          static_cast<int32_t>(extra->numInputs())
        },
        // ActRec.
        actrec(inst.src(0), extra->spOffset + extra->numInputs()),
        // Inout outputs.
        extra->numOut == 0 ? AEmpty : AStack {
          inst.src(0),
          extra->spOffset + extra->numInputs() + kNumActRecCells +
            extra->numOut - 1,
          static_cast<int32_t>(extra->numOut)
        },
        // Locals.
        backtrace_locals(inst)
      };
    }

  case ContEnter:
    {
      auto const extra = inst.extra<ContEnter>();
      return CallEffects {
        // Kills. Everything on the stack.
        stack_below(inst.src(0), extra->spOffset) | AMIStateAny,
        // No inputs. The value being sent is passed explicitly.
        AEmpty,
        // ActRec. It is on the heap and we already implicitly assume that
        // CallEffects can perform arbitrary heap operations.
        AEmpty,
        // No outputs.
        AEmpty,
        // Locals.
        backtrace_locals(inst)
      };
    }

  case Call:
    {
      auto const extra = inst.extra<Call>();
      return CallEffects {
        // Kills. Everything on the stack below the incoming parameters.
        stack_below(inst.src(0), extra->spOffset - 1) | AMIStateAny,
        // Input arguments.
        extra->numInputs() == 0 ? AEmpty : AStack {
          inst.src(0),
          extra->spOffset + extra->numInputs() - 1,
          static_cast<int32_t>(extra->numInputs())
        },
        // ActRec.
        actrec(inst.src(0), extra->spOffset + extra->numInputs()),
        // Inout outputs.
        extra->numOut == 0 ? AEmpty : AStack {
          inst.src(0),
          extra->spOffset + extra->numInputs() + kNumActRecCells +
            extra->numOut - 1,
          static_cast<int32_t>(extra->numOut)
        },
        // Locals.
        backtrace_locals(inst)
      };
    }

  case CallBuiltin:
    {
      AliasClass out_stk = AEmpty;
      auto const callee = inst.extra<CallBuiltin>()->callee;
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
      if (callee->isFoldable()) {
        return may_load_store_kill(
          stk | AHeapAny, out_stk | AHeapAny, AMIStateAny
        );
      } else {
        return may_load_store_kill(
          stk | AHeapAny | ARdsAny,
          out_stk | AHeapAny | ARdsAny,
          AMIStateAny
        );
      }
    }

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateGen:
  case CreateAGen:
  case CreateAFWH:
  case CreateAFWHNoVV: {
    auto const fp = canonical(inst.src(0));
    auto fpInst = fp->inst();
    if (fpInst->is(DefLabel)) fpInst = resolveFpDefLabel(fp);
    auto const frame = [&] () -> AliasClass {
      if (fpInst->is(DefFP, DefFuncEntryFP)) return ALocalAny;
      assertx(fpInst->is(DefInlineFP));
      auto const nlocals = fpInst->extra<DefInlineFP>()->target->numLocals();
      return nlocals
        ? ALocal { fp, AliasIdSet::IdRange(0, nlocals)}
        : AEmpty;
    }();
    return may_load_store_move(
      frame,
      AHeapAny,
      frame
    );
  }

  // AGWH construction updates the AsyncGenerator object.
  case CreateAGWH:
    return may_load_store(AHeapAny, AHeapAny);

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
      ALocal {
        inst.src(0),
        inst.extra<StLoc>()->locId,
        inst.extra<StLoc>()->fpOffset
      },
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
    return PureLoad {
      ALocal {
        inst.src(0),
        inst.extra<LocalId>()->locId,
        inst.extra<LocalId>()->fpOffset
      }
    };

  case CheckLoc:
  case LdLocPseudoMain:
    // Note: LdLocPseudoMain is both a guard and a load, so it must not be a
    // PureLoad.
    return may_load_store(
      ALocal {
        inst.src(0),
        inst.extra<LocalId>()->locId,
        inst.extra<LocalId>()->fpOffset
      },
      AEmpty
    );

  case StLocPseudoMain:
    // This can store to globals or locals, but we don't have globals supported
    // in AliasClass yet.
    return PureStore { AUnknown, inst.src(1), nullptr };

  //////////////////////////////////////////////////////////////////////
  // Pointer-based loads and stores

  case LdMem:
    return PureLoad { pointee(inst.src(0)) };
  case StMem:
    return PureStore { pointee(inst.src(0)), inst.src(1), inst.src(0) };

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

  case LdClsFromClsMeth:
  case LdFuncFromClsMeth:
    return may_load_store(AHeapAny, AEmpty);

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

  case InitPackedLayoutArray: {
    auto const arr = inst.src(0);
    auto const val = inst.src(1);
    auto const idx = inst.extra<InitPackedLayoutArray>()->index;
    return PureStore { AElemI { arr, idx }, val, arr };
  }

  case InitMixedLayoutArray: {
    auto const arr = inst.src(0);
    auto const val = inst.src(1);
    auto const key = inst.extra<InitMixedLayoutArray>()->key;
    return PureStore { AElemS { arr, key }, val, arr };
  }

  case LdVecElem:
  case LdPackedElem: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    return PureLoad {
      key->hasConstVal() ? AElemI { base, key->intVal() } : AElemIAny
    };
  }

  case InitPackedLayoutArrayLoop:
    {
      auto const extra = inst.extra<InitPackedLayoutArrayLoop>();
      auto const stack_in = AStack {
        inst.src(1),
        extra->offset + static_cast<int32_t>(extra->size) - 1,
        static_cast<int32_t>(extra->size)
      };
      return may_load_store_move(stack_in, AElemIAny, stack_in);
    }

  case NewKeysetArray:
    {
      // NewKeysetArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewKeysetArray>();
      auto const stack_in = AStack {
        inst.src(0),
        extra->offset + static_cast<int32_t>(extra->size) - 1,
        static_cast<int32_t>(extra->size)
      };
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case NewStructArray:
  case NewStructDArray:
  case NewStructDict:
    {
      // NewStructArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructData>();
      auto const stack_in = AStack {
        inst.src(0),
        extra->offset + static_cast<int32_t>(extra->numKeys) - 1,
        static_cast<int32_t>(extra->numKeys)
      };
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case NewRecord:
  case NewRecordArray:
    {
      auto const extra = inst.extra<NewStructData>();
      auto const stack_in = AStack {
        inst.src(1),
        extra->offset + static_cast<int32_t>(extra->numKeys) - 1,
        static_cast<int32_t>(extra->numKeys)
      };
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
      },
      extra->fpOffset
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
        },
        extra->fpOffset
      };
    }();
    return may_load_store(frame, AEmpty);
  }

  case MixedArrayGetK:
  case DictGetK:
  case KeysetGetK: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    always_assert(key->isA(TInt | TStr));
    if (key->isA(TInt)) {
      return PureLoad {
        key->hasConstVal() ? AElemI { base, key->intVal() } : AElemIAny
      };
    }
    if (key->isA(TStr)) {
      return PureLoad {
        key->hasConstVal() ? AElemS { base, key->strVal() } : AElemSAny
      };
    }
    return PureLoad { AElemAny };
  }

  case LdPtrIterKey:
    // Array element keys are not tracked by memory effects right now.
    return may_load_store(AEmpty, AEmpty);

  case LdPtrIterVal: {
    // NOTE: The type param for this op restricts the key, not the value.
    if (inst.typeParam() <= TInt) return PureLoad { AElemIAny };
    if (inst.typeParam() <= TStr) return PureLoad { AElemSAny };
    return PureLoad { AElemAny };
  }

  case ElemMixedArrayK:
  case ElemDictK:
  case ElemKeysetK:
    return IrrelevantEffects {};

  case VecFirst: {
    auto const base = inst.src(0);
    return may_load_store(AElemI { base, 0 }, AEmpty);
  }
  case VecLast: {
    auto const base = inst.src(0);
    if (base->hasConstVal(TArr)) {
      return may_load_store(
          AElemI { base, static_cast<int64_t>(base->arrVal()->size() - 1) },
          AEmpty);
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
    return may_load_store(AEmpty, AEmpty);

  case CheckMixedArrayKeys:
  case CheckMixedArrayOffset:
  case CheckDictOffset:
  case CheckKeysetOffset:
  case CheckMissingKeyInArrLike:
  case ProfileMixedArrayAccess:
  case ProfileDictAccess:
  case ProfileKeysetAccess:
  case CheckArrayCOW:
    return may_load_store(AHeapAny, AEmpty);

  case ArrayIsset:
  case AKExistsArr:
    return may_load_store(AElemAny, AEmpty);

  case ArrayIdx:
    return may_load_store(AElemAny, AEmpty);

  case SameArr:
  case NSameArr:
    return may_load_store(AEmpty, AEmpty);

  case DictGetQuiet:
  case DictIsset:
  case DictIdx:
  case KeysetGetQuiet:
  case KeysetIsset:
  case KeysetIdx:
  case AKExistsDict:
  case AKExistsKeyset:
    return may_load_store(AElemAny, AEmpty);

  case SameVec:
  case NSameVec:
  case SameDict:
  case NSameDict:
  case EqKeyset:
  case NeqKeyset:
  case SameKeyset:
  case NSameKeyset:
    return may_load_store(AElemAny, AEmpty);

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
  case SetNewElemArray:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case UnsetElem:
  case ElemArrayD:
  case ElemArrayU:
  case ElemVecD:
  case ElemVecU:
  case ElemDictD:
  case ElemDictU:
  case ElemKeysetU:
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

  case ReservePackedArrayDataNewElem:
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
  case NewCol:
  case NewColFromArray:
  case NewPair:
  case NewInstanceRaw:
  case NewDArray:
  case NewDictArray:
  case NewPlainArray:
  case FuncCred:
  case AllocVArray:
  case AllocVec:
  case AllocStructArray:
  case AllocStructDArray:
  case AllocStructDict:
  case ConvBoolToArr:
  case ConvDblToStr:
  case ConvDblToArr:
  case ConvFuncToArr:
  case ConvIntToArr:
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
    return PureLoad {
      AStack { inst.src(0), inst.extra<LdStk>()->offset, 1 }
    };

  case StStk:
    return PureStore {
      AStack { inst.src(0), inst.extra<StStk>()->offset, 1 },
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
      AStack { inst.src(0), inst.extra<CheckStk>()->offset, 1 },
      AEmpty
    );

  case DbgTraceCall:
    return may_load_store(AStackAny | ALocalAny, AEmpty);

  case Unreachable:
    // Unreachable code kills every memory location.
    return may_load_store_kill(AEmpty, AEmpty, AUnknown);

  case ResolveTypeStruct: {
    auto const extra = inst.extra<ResolveTypeStructData>();
    auto const stack_in = AStack {
      inst.src(0),
      extra->offset + static_cast<int32_t>(extra->size) - 1,
      static_cast<int32_t>(extra->size)
    };
    return may_load_store(AliasClass(stack_in)|AHeapAny, AHeapAny);
  }

  //////////////////////////////////////////////////////////////////////
  // Instructions that never read or write memory locations tracked by this
  // module.

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AdvanceMixedPtrIter:
  case AdvancePackedPtrIter:
  case AndInt:
  case AssertType:
  case AssertLoc:
  case AssertStk:
  case AssertMBase:
  case DefFP:
  case DefFuncEntryFP:
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
  case EqRecDesc:
  case EqFunc:
  case EqStrPtr:
  case EqArrayDataPtr:
  case EqDbl:
  case EqInt:
  case EqPtrIter:
  case GetMixedPtrIter:
  case GetPackedPtrIter:
  case GteBool:
  case GteInt:
  case GtBool:
  case GtInt:
  case Jmp:
  case JmpNZero:
  case JmpZero:
  case LdPropAddr:
  case LdStkAddr:
  case LdPackedArrayDataElemAddr:
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
  case ProfileSubClsCns:
  case FuncHasAttr:
  case IsFunReifiedGenericsMatched:
  case IsClsDynConstructible:
  case JmpPlaceholder:
  case LdFuncRxLevel:
  case LdSmashable:
  case LdSmashableFunc:
  case LdARNumParams:
  case LdRDSAddr:
  case CheckRange:
  case ProfileType:
  case LdIfaceMethod:
  case InstanceOfIfaceVtable:
  case IsTypeStructCached:
  case LdARFlags:
  case LdTVAux:
  case MethodExists:
  case GetTime:
  case GetTimeNs:
  case ProfileInstanceCheck:
  case Select:
  case LookupSPropSlot:
  case ConvPtrToLval:
  case ProfileProp:
  case ProfileIsTypeStruct:
    return IrrelevantEffects {};

  case StClosureArg:
    return PureStore {
      AProp {
        inst.src(0),
        safe_cast<uint16_t>(inst.extra<StClosureArg>()->index)
      },
      inst.src(1),
      inst.src(0)
    };

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
  case StContArKey:
  case StContArValue:
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
  case CheckVArray:
  case CheckDArray:
  case CheckDVArray:
  case StArResumeAddr:
  case StContArState:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case CheckCold:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContValid:
  case ContStarted:
  case IncProfCounter:
  case IncCallCounter:
  case IncStat:
  case ContPreNext:
  case ContStartedCheck:
  case ConvArrToBool:
  case ConvArrToDbl:
  case CountArray:
  case CountArrayFast:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case InstanceOf:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfIface:
  case InstanceOfRecDesc:
  case InterfaceSupportsArr:
  case InterfaceSupportsVec:
  case InterfaceSupportsDict:
  case InterfaceSupportsKeyset:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
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
  case ConvStrToArr:   // decrefs src, but src is a string
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvResToDbl:
  case EagerSyncVMRegs:
  case ExtendsClass:
  case LdUnwinderValue:
  case LdFrameThis:
  case LdFrameCls:
  case LdClsName:
  case LdAFWHActRec:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
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
  case ProfileArrayKind:
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

  // Some that touch memory we might care about later, but currently don't:
  case ColIsEmpty:
  case ColIsNEmpty:
  case ConvTVToBool:
  case ConvObjToBool:
  case CountCollection:
  case LdVectorSize:
  case CheckPackedArrayDataBounds:
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

  case LdClsPropAddrOrNull:   // may run 86{s,p}init, which can autoload
  case LdClsPropAddrOrRaise:  // raises errors, and 86{s,p}init
  case Clone:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case RaiseClsMethPropConvertNotice:
  case RaiseArraySerializeNotice:
  case RaiseUninitLoc:
  case RaiseUndefProp:
  case RaiseTooManyArg:
  case RaiseError:
  case RaiseNotice:
  case RaiseWarning:
  case RaiseHackArrCompatNotice:
  case RaiseHackArrParamNotice:
  case RaiseHackArrPropNotice:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseRxCallViolation:
  case RaiseStrToClassNotice:
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
  case GtArr:
  case GteArr:
  case LtArr:
  case LteArr:
  case EqArr:
  case NeqArr:
  case CmpArr:
  case GtVec:
  case GteVec:
  case LtVec:
  case LteVec:
  case EqVec:
  case NeqVec:
  case CmpVec:
  case EqDict:
  case NeqDict:
  case ConvTVToArr:  // decrefs src, may read obj props
  case ConvObjToArr:   // decrefs src
  case ConvObjToVArr:  // can invoke PHP
  case ConvObjToDArr:  // can invoke PHP
  case OODeclExists:
  case DefCls:         // autoload
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFunc:         // autoload
  case LdFuncCached:   // autoload
  case LdRecDescCached:    // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case InitClsCns:      // autoload
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCnsE:
  case LookupFuncCached: // autoload
  case StringGet:      // raise_notice
  case OrdStrIdx:      // raise_notice
  case ArrayAdd:       // decrefs source
  case AddNewElem:         // can re-enter
  case AddNewElemKeyset:   // can re-enter
  case ArrayGet:       // kVPackedKind warnings
  case ArraySet:       // kVPackedKind warnings
  case DictGet:
  case KeysetGet:
  case VecSet:
  case DictSet:
  case LdClsCtor:
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
  case ConvResToStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConvTVToDbl:
  case ConvArrToVec:
  case ConvArrToDict:
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
  case SetLegacyDict:
  case SetLegacyVec:
  case SetOpTV:
  case SetOpTVVerify:
  case ThrowAsTypeStructException:
  case PropTypeRedefineCheck: // Can raise and autoload
  case HandleRequestSurprise:
    return may_load_store(AHeapAny, AHeapAny);

  case AddNewElemVec:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case IsTypeStruct:
  case RecordReifiedGenericsAndGetTSList:
    return may_load_store(AElemAny, AEmpty);

  case ConvArrToKeyset: // Decrefs input values
  case ConvVecToKeyset:
  case ConvDictToKeyset:
  case ConvDictToDArr: // These 4 may raise Hack array compat notices
  case ConvKeysetToDArr:
  case ConvDictToArr:
  case ConvKeysetToArr:
  case ConvClsMethToArr:
  case ConvClsMethToDArr:
  case ConvClsMethToDict:
  case ConvClsMethToKeyset:
  case ConvClsMethToVArr:
  case ConvClsMethToVec:
    return may_load_store(AElemAny, AEmpty);

  case ConvVecToArr:
  case ConvArrToNonDVArr:
  case ConvDictToVec:
  case ConvKeysetToVec:
  case ConvVecToDict:
  case ConvKeysetToDict:
  case ConvArrToVArr:
  case ConvVecToVArr:
  case ConvDictToVArr:
  case ConvKeysetToVArr:
  case ConvArrToDArr:
  case ConvVecToDArr:
    return may_load_store(AElemAny, AEmpty);

  case ReleaseVVAndSkip:
    return may_load_store(AHeapAny|ALocalAny, AHeapAny|ALocalAny);

  // debug_backtrace() traverses stack and WaitHandles on the heap.
  case DebugBacktrace:
  case DebugBacktraceFast:
    return may_load_store(AHeapAny|ALocalAny|AStackAny, AHeapAny);

  // This instruction doesn't touch memory we track, except that it may
  // re-enter to construct php Exception objects.  During this re-entry anything
  // can happen (e.g. a surprise flag check could cause a php signal handler to
  // run arbitrary code).
  case AFWHPrepareChild:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // The following instructions are used for debugging memory optimizations.
  // We can't ignore them, because they can prevent future optimizations;
  // eg t1 = LdStk<N>; DbgTrashStk<N>; StStk<N> t1
  // If we ignore the DbgTrashStk it looks like the StStk is redundant

  case DbgTrashStk:
    return GeneralEffects {
      AEmpty, AEmpty, AEmpty,
      AStack { inst.src(0), inst.extra<DbgTrashStk>()->offset, 1 }
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
  case DbgTrashRetVal:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////

  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool check_effects(const IRInstruction& inst, MemEffects me) {
  SCOPE_ASSERT_DETAIL("Memory Effects") {
    return folly::sformat("  inst: {}\n  effects: {}\n", inst, show(me));
  };

  auto check_fp = [&] (FPRelOffset base) {
    always_assert_flog(
      base.offset <= 0,
      "frame offset is above base frame"
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      obj->type() <= TObj,
      "Non obj pointer in memory effects"
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.local()) check_fp(fr->base);
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
    [&] (InlineEnterEffects x){ check(x.inlStack);
                                check(x.inlFrame);
                                check(x.actrec); },
    [&] (InlineExitEffects x){ check(x.inlStack);
                               check(x.inlFrame);
                               check(x.inlMeta); },
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
      [&] (InlineExitEffects)  { return fail(); },
      [&] (InlineEnterEffects) { return fail(); },
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
    [&] (InlineEnterEffects x) -> R {
      return InlineEnterEffects {
        canonicalize(x.inlStack),
        canonicalize(x.inlFrame),
        canonicalize(x.actrec),
      };
    },
    [&] (InlineExitEffects x) -> R {
      return InlineExitEffects {
        canonicalize(x.inlStack),
        canonicalize(x.inlFrame),
        canonicalize(x.inlMeta)
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
    [&] (InlineEnterEffects x) {
      return sformat("inline_enter({} ; {} ; {})",
        show(x.inlStack),
        show(x.inlFrame),
        show(x.actrec)
      );
    },
    [&] (InlineExitEffects x) {
      return sformat("inline_exit({} ; {} ; {})",
        show(x.inlStack),
        show(x.inlFrame),
        show(x.inlMeta)
      );
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
