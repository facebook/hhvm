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

#include <folly/Optional.h>

namespace HPHP { namespace jit {

namespace {

AliasClass pointee(
  const SSATmp* ptr,
  jit::flat_set<const IRInstruction*>* visited_labels
) {
  auto const type = ptr->type();
  always_assert(type <= TPtrToGen);
  auto const maybeRef = type.maybe(TPtrToRefGen);
  auto const typeNR = type - TPtrToRefGen;
  auto const canonPtr = canonical(ptr);
  if (!canonPtr->isA(TPtrToGen)) {
    // This can happen when ptr is TBottom from a passthrough instruction with
    // a src that isn't TBottom. The most common cause of this is something
    // like "t5:Bottom = CheckType<Str> t2:Int". It means ptr isn't really a
    // pointer, so return AEmpty to avoid unnecessarily pessimizing any
    // optimizations.
    always_assert(ptr->isA(TBottom));
    return AEmpty;
  }

  auto const sinst = canonPtr->inst();

  if (sinst->is(UnboxPtr)) {
    return ARefAny | pointee(sinst->src(0), visited_labels);
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
    if (typeNR <= TBottom) return AEmpty;

    if (typeNR <= TPtrToFrameGen) {
      if (sinst->is(LdLocAddr)) {
        return AliasClass {
          AFrame { sinst->src(0), sinst->extra<LdLocAddr>()->locId }
        };
      }
      return AFrameAny;
    }

    if (typeNR <= TPtrToStkGen) {
      if (sinst->is(LdStkAddr)) {
        return AliasClass {
          AStack { sinst->src(0), sinst->extra<LdStkAddr>()->offset, 1 }
        };
      }
      return AStackAny;
    }

    if (typeNR <= TPtrToPropGen) {
      if (sinst->is(LdPropAddr)) {
        return AliasClass {
          AProp { sinst->src(0),
                  safe_cast<uint32_t>(sinst->extra<LdPropAddr>()->offsetBytes) }
        };
      }
      return APropAny;
    }

    if (typeNR <= TPtrToMISGen) {
      if (sinst->is(LdMIStateAddr)) {
        return mis_from_offset(sinst->src(0)->intVal());
      }
      if (ptr->hasConstVal() && ptr->rawVal() == 0) {
        // nullptr tvRef pointer, representing an instruction that doesn't use
        // it.
        return AEmpty;
      }
      return AMIStateTV;
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
        int64_t n;
        auto const arrTy = base->type();
        if (!arrTy.subtypeOfAny(TDict, TKeyset) &&
            key->strVal()->isStrictlyInteger(n)) {
          if (arrTy.maybe(TDict) || arrTy.maybe(TKeyset)) return AElemAny;
          return AElemI { base, n };
        }
        return AElemS { base, key->strVal() };
      }
      return AElemAny;
    };

    if (typeNR <= TPtrToElemGen) {
      if (sinst->is(LdPackedArrayDataElemAddr)) return elem();
      return AElemAny;
    }

    // The result of ElemArray{,W,U} is either the address of an array element,
    // or &immutable_null_base.
    if (typeNR <= TPtrToMembGen) {
      if (sinst->is(ElemArray, ElemArrayW, ElemDict,
                    ElemDictW, ElemKeyset, ElemKeysetW)) return elem();

      // Takes a PtrToGen as its first operand, so we can't easily grab an array
      // base.
      if (sinst->is(ElemArrayU, ElemVecU, ElemDictU, ElemKeysetU)) {
        return AElemAny;
      }

      // These instructions can only get at tvRef when given it as a
      // src. Otherwise they can only return pointers to properties or
      // &immutable_null_base.
      if (sinst->is(PropX, PropDX, PropQ)) {
        assertx(sinst->srcs().back()->isA(TPtrToMISGen));
        return APropAny | pointee(sinst->srcs().back(), visited_labels);
      }

      // Like the Prop* instructions, but for array elements. These could also
      // return pointers to collection elements but those don't exist in
      // AliasClass yet.
      if (sinst->is(ElemX, ElemDX, ElemUX)) {
        assertx(sinst->srcs().back()->isA(TPtrToMISGen));
        return AElemAny | pointee(sinst->srcs().back(), visited_labels);
      }

      return folly::none;
    }

    return folly::none;
  }();

  auto ret = maybeRef ? ARefAny : AEmpty;
  if (specific) return *specific | ret;

  /*
   * None of the above worked, so try to make the smallest union we can based
   * on the pointer type.
   */
  if (typeNR.maybe(TPtrToStkGen))     ret = ret | AStackAny;
  if (typeNR.maybe(TPtrToFrameGen))   ret = ret | AFrameAny;
  if (typeNR.maybe(TPtrToPropGen))    ret = ret | APropAny;
  if (typeNR.maybe(TPtrToElemGen))    ret = ret | AElemAny;
  if (typeNR.maybe(TPtrToMISGen))     ret = ret | AMIStateTV;
  if (typeNR.maybe(TPtrToClsInitGen)) ret = ret | AHeapAny;
  if (typeNR.maybe(TPtrToClsCnsGen))  ret = ret | AHeapAny;
  return ret;
}

//////////////////////////////////////////////////////////////////////

AliasClass all_pointees(folly::Range<SSATmp**> srcs) {
  auto ret = AliasClass{AEmpty};
  for (auto const& src : srcs) {
    if (src->isA(TPtrToGen)) {
      ret = ret | pointee(src);
    }
  }
  return ret;
}

// Return an AliasClass containing all locations pointed to by any PtrToGen
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

/*
 * Helper functions for alias classes representing an ActRec on the stack
 * (whether pre-live or live).
 */

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

// Return an AliasClass representing just the context field of an ActRec at base
// + offset.
AliasClass actrec_ctx(SSATmp* base, IRSPRelOffset offset) {
  return AStack { base, offset + int32_t{kActRecCtxCellOff}, 1 };
}

// Return AliasClass representing just the func field of an ActRec at base +
// offset.
AliasClass actrec_func(SSATmp* base, IRSPRelOffset offset) {
  return AStack { base, offset + int32_t{kActRecFuncCellOff}, 1 };
}

//////////////////////////////////////////////////////////////////////

// Determine an AliasClass representing any locals in the instruction's frame
// which might be accessed via debug_backtrace().

const StaticString s_86metadata("86metadata");

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
    if (fpInst->is(DefInlineFP)) return fpInst->extra<DefInlineFP>()->target;
    always_assert(false);
  }();

  // Either there's no func or no frame-pointer. Either way, be conservative and
  // assume anything can be read. This can happen in test code, for instance.
  if (!func) return AFrameAny;

  auto const add86meta = [&] (AliasClass ac) {
    // The 86metadata variable can also exist in a VarEnv, but accessing that is
    // considered a heap effect, so we can ignore it.
    auto const local = func->lookupVarId(s_86metadata.get());
    if (local == kInvalidId) return ac;
    return ac | AFrame { inst.marker().fp(), local };
  };

  if (!RuntimeOption::EnableArgsInBacktraces || !func->numParams()) {
    return add86meta(AEmpty);
  }

  AliasIdSet params{ AliasIdSet::IdRange{0, func->numParams()} };
  return add86meta(AFrame { inst.marker().fp(), params });
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
 * removed the DefInlineFP its spOff will not be meaningful (unless it's a
 * DecRef instruction, which we explicitly adjust in dce.cpp).  In this case
 * the `kills' set will refer to the wrong stack locations.  In general this
 * means instructions that can re-enter must have catch traces---but a few
 * other instructions are exceptions, either since they are not allowed in
 * inlined functions or because they take the (possibly-inlined) FramePtr as a
 * source.
 */
GeneralEffects may_reenter(const IRInstruction& inst, GeneralEffects x) {
  auto const may_reenter_is_ok =
    (inst.taken() && inst.taken()->isCatch()) ||
    inst.is(DecRef,
            ReleaseVVAndSkip,
            MIterFree,
            MIterNext,
            MIterNextK,
            IterFree,
            ABCUnblock,
            GenericRetDecRefs,
            MemoSet);
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

/*
 * Modify a GeneralEffects for instructions that could call the user
 * error handler for the current frame (ie something that can raise
 * a warning/notice/error, or a builtin call), because the error
 * handler gets a context array which contains all the locals.
 */
GeneralEffects may_raise(const IRInstruction& inst, GeneralEffects x) {
  return may_reenter(
    inst,
    GeneralEffects {
      x.loads |
        (RuntimeOption::EnableContextInErrorHandler ? AFrameAny : AEmpty),
      x.stores, x.moves, x.kills
    }
  );
}

// Equivalent to may_raise if EvalHackArrCompatNotices is enabled for certain
// opcodes, no-op otherwise.
GeneralEffects hack_arr_compat_may_raise(const IRInstruction& inst,
                                         GeneralEffects x) {
  assertx(inst.is(AKExistsArr, ArrayIdx, ArrayIsset));
  if (!RuntimeOption::EvalHackArrCompatNotices) return x;
  return may_raise(inst, x);
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
 * otherwise the same.
 *
 * N.B. Currently the memory for frame iterator slots is not part of the
 * AliasClass lattice, since we never really manipulate them from the TC yet,
 * so we don't report the effect these instructions have on it.
 */
GeneralEffects iter_effects(const IRInstruction& inst,
                            SSATmp* fp,
                            AliasClass locals) {
  auto const iterID = inst.extra<IterData>()->iterId;
  AliasClass const iterPos = AIterPos { fp, iterID };
  AliasClass const iterBase = AIterBase { fp, iterID };
  auto const iterMem = iterPos | iterBase;
  return may_reenter(
    inst,
    may_load_store_kill(
      locals | AHeapAny | iterMem,
      locals | AHeapAny | iterMem,
      AMIStateAny
    )
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
  auto loads  = AHeapAny | AStackAny | AFrameAny;
  auto stores = AHeapAny | AStackAny;
  if (extra->smashesAllLocals) {
    stores = stores | AFrameAny;
  } else {
    for (auto i = uint32_t{0}; i < extra->nChangedLocals; ++i) {
      stores = stores | AFrame { inst.src(1), extra->changedLocals[i].id };
    }
  }

  for (auto i = uint32_t{0}; i < extra->nChangedClsRefSlots; ++i) {
    auto const& slot = extra->changedClsRefSlots[i];
    if (slot.write) {
      stores = stores | AClsRefSlot { inst.src(1), slot.id };
    } else {
      loads = loads | AClsRefSlot { inst.src(1), slot.id };
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

  return may_raise(inst, may_load_store_kill(loads, stores, kills));
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
  assertx(srcs.back()->isA(TPtrToMISGen));
  return may_raise(
    inst,
    may_load_store(
      AHeapAny | all_pointees(srcs.subpiece(0, srcs.size() - 1)),
      AHeapAny | all_pointees(inst)
    )
  );
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
    // Note, this instruction can re-enter, but doesn't need the may_reenter()
    // treatmeant because of the special kill semantics for locals and stack.
    return may_load_store_kill(
      AHeapAny, AHeapAny,
      *AStackAny.precise_union(AFrameAny)->precise_union(AMIStateAny)
    );

  // The suspend hooks can load anything (re-entering the VM), but can't write
  // to frame locals.
  case SuspendHookE:
  case SuspendHookR:
    // TODO: may-load here probably doesn't need to include AFrameAny normally.
    return may_reenter(inst,
                       may_load_store_kill(AUnknown, AHeapAny, AMIStateAny));

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
      AStackAny | AFrameAny | AClsRefSlotAny | ACufIterAny | AMIStateAny
    };

  case AsyncRetCtrl:
  case AsyncRetFast:
  case AsyncSwitchFast:
    if (inst.extra<RetCtrlData>()->suspendingResumed) {
      return UnknownEffects {};
    }
    return ReturnEffects {
      *stack_below(
        inst.src(0),
        inst.extra<RetCtrlData>()->spOffset - 1
      ).precise_union(AMIStateAny)
    };

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
    return may_reenter(inst,
                       may_load_store_kill(AUnknown, AUnknown, AMIStateAny));

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
  case DefInlineFP:
    return may_load_store(
      /*
       * We need to mark DefInlineFP as both loading and storing the entire
       * stack below its frame because it may have been pushed. If DefInlineFP
       * is not pushed these cells were marked as both stored and killed by
       * BeginInlining so now actual stores should be aliased.
       *
       * Importantly, if we do sink DefInlineFP stack cells from above may
       * alias locals within DefInlineFP, this confuses alias analysis, these
       * stores must not be sunk past DefInlineFP where they could clobber a
       * local.
       */
      AFrameAny | AClsRefSlotAny | ACufIterAny  |
        stack_below(inst.dst(), FPRelOffset{0}) |
        inline_fp_frame(&inst),
      AFrameAny | AClsRefSlotAny | ACufIterAny |
        stack_below(inst.dst(), FPRelOffset{0})
    );

  /*
   * BeginInlining is similar to DefInlineFP, however, it must always be the
   * first instruction in the inlined call and has no effect serving only as
   * a marker to memory effects that the stack cells within the inlined call
   * are now dead.
   *
   * Unlike DefInlineFP it does not load the SpillFrame, which we hope to push
   * off the main trace or elide entirely.
   */
  case BeginInlining: {
    /*
     * SP relative offset of the first non-frame cell within the inlined call.
     */
    auto inlineStackOff = inst.extra<BeginInlining>()->offset;
    return may_load_store_kill(
      AEmpty,
      /*
       * This prevents stack slots from the caller from being sunk into the
       * callee. Note that some of these stack slots overlap with the frame
       * locals of the callee-- those slots are inacessible in the inlined
       * call as frame and stack locations may not alias.
       */
      stack_below(inst.src(0), inlineStackOff),
      /*
       * While not required for correctness adding these slots to the kill set
       * will hopefully avoid some extra stores.
       */
      stack_below(inst.src(0), inlineStackOff)
    );
  }

  case InlineReturn: {
    auto const callee = stack_below(inst.src(0), FPRelOffset{2}) |
                        AMIStateAny | AFrameAny | AClsRefSlotAny |
                        ACufIterAny;
    return may_load_store_kill(AEmpty, callee, callee);
  }

  case InlineReturnNoFrame: {
    auto const callee = AliasClass(AStack {
      inst.extra<InlineReturnNoFrame>()->offset,
      std::numeric_limits<int32_t>::max()
    }) | AMIStateAny;
    return may_load_store_kill(AEmpty, callee, callee);
  }

  case SyncReturnBC: {
    auto const spOffset = inst.extra<SyncReturnBC>()->spOffset;
    auto const arStack = actrec(inst.src(0), spOffset);
    // This instruction doesn't actually load but SpillFrame cannot be pushed
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
    return may_raise(inst, may_load_store(AUnknown, AHeapAny));
  // VerifyParamFail might coerce the parameter to the desired type rather than
  // throwing.
  case VerifyParamFail: {
    auto const localId = inst.src(0)->intVal();
    auto const stores = AHeapAny | AFrame{inst.marker().fp(), localId};
    return may_raise(inst, may_load_store(AUnknown, stores));
  }
  // However the following ones can't read locals from our frame on the way
  // out, except as a side effect of raising a warning.
  case VerifyRetCallable:
  case VerifyRetCls:
    return may_raise(inst, may_load_store(AHeapAny, AHeapAny));
  // In PHP 7 VerifyRetFail can coerce the return type in weak files-- even in
  // a strict file we may still coerce int to float. This is not true of HH
  // files.
  case VerifyRetFail: {
    auto func = inst.marker().func();
    auto mayCoerce =
      RuntimeOption::PHP7_ScalarTypes &&
      !RuntimeOption::EnableHipHopSyntax &&
      !func->unit()->isHHFile();
    auto stores = mayCoerce ? AHeapAny | AStackAny : AHeapAny;
    return may_raise(inst, may_load_store(AHeapAny | AStackAny, stores));
  }
  case VerifyRetFailHard:
    return may_raise(inst, may_load_store(AHeapAny | AStackAny, AHeapAny));

  case CallArray:
    {
      auto const extra = inst.extra<CallArray>();
      return CallEffects {
        extra->writeLocals,
        // Kills. Everything on the stack below the incoming parameters.
        stack_below(inst.src(0), extra->spOffset - 1) | AMIStateAny,
        // Stack. The act-rec, incoming parameters, and everything below.
        stack_below(
          inst.src(0),
          extra->spOffset + extra->numParams + kNumActRecCells - 1
        ),
        // Locals.
        (extra->writeLocals || extra->readLocals)
          ? AFrameAny : backtrace_locals(inst),
        // Callee.
        actrec_func(inst.src(0), extra->spOffset + extra->numParams)
      };
    }

  case ContEnter:
    {
      auto const extra = inst.extra<ContEnter>();
      return CallEffects {
        false,
        // Kills. Everything on the stack below the sent value.
        stack_below(inst.src(0), extra->spOffset - 1) | AMIStateAny,
        // Stack. The value being sent, and everything below.
        stack_below(inst.src(0), extra->spOffset),
        // Locals.
        backtrace_locals(inst),
        // Callee. Stored inside the generator object, not used by ContEnter.
        AEmpty
      };
    }

  case Call:
    {
      auto const extra = inst.extra<Call>();
      return CallEffects {
        extra->writeLocals,
        // Kills. Everything on the stack below the incoming parameters.
        stack_below(inst.src(0), extra->spOffset - 1) | AMIStateAny,
        // Stack. The act-rec, incoming parameters, and everything below.
        stack_below(
          inst.src(0),
          extra->spOffset + extra->numParams + kNumActRecCells - 1
        ),
        // Locals.
        (extra->writeLocals || extra->readLocals)
          ? AFrameAny : backtrace_locals(inst),
        // Callee.
        actrec_func(inst.src(0), extra->spOffset + extra->numParams)
      };
    }

  case CallBuiltin:
    {
      auto const extra = inst.extra<CallBuiltin>();
      auto const stk = [&] () -> AliasClass {
        AliasClass ret = AEmpty;
        for (auto i = uint32_t{2}; i < inst.numSrcs(); ++i) {
          if (inst.src(i)->type() <= TPtrToGen) {
            auto const cls = pointee(inst.src(i));
            if (cls.maybe(AStackAny)) {
              ret = ret | cls;
            }
          }
        }
        return ret;
      }();
      auto const writeLocs = extra->writeLocals ? AFrameAny : AEmpty;
      auto const readLocs =
        (extra->readLocals || extra->writeLocals) ? AFrameAny : AEmpty;
      return may_raise(
        inst,
        may_load_store_kill(stk | AHeapAny | readLocs, writeLocs, AMIStateAny)
      );
    }

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateGen:
  case CreateAGen:
  case CreateAFWH:
  case CreateAFWHNoVV:
    return may_load_store_move(
      AFrameAny | AClsRefSlotAny | ACufIterAny,
      AHeapAny,
      AFrameAny | AClsRefSlotAny | ACufIterAny
    );

  // This re-enters to call extension-defined instance constructors.
  case ConstructInstance:
    return may_reenter(inst, may_load_store(AHeapAny, AHeapAny));

  case CheckStackOverflow:
  case CheckSurpriseFlagsEnter:
  case CheckSurpriseAndStack:
    return may_raise(inst, may_load_store(AEmpty, AEmpty));

  case InitExtraArgs:
    return UnknownEffects {};

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case MIterInit:
  case WIterInit:
    return iter_effects(
      inst,
      inst.src(1),
      AFrame { inst.src(1), inst.extra<IterData>()->valId }
    );
  case IterNext:
  case MIterNext:
  case WIterNext:
    return iter_effects(
      inst,
      inst.src(0),
      AFrame { inst.src(0), inst.extra<IterData>()->valId }
    );

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    {
      AliasClass key = AFrame { inst.src(1), inst.extra<IterData>()->keyId };
      AliasClass val = AFrame { inst.src(1), inst.extra<IterData>()->valId };
      return iter_effects(inst, inst.src(1), key | val);
    }

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    {
      AliasClass key = AFrame { inst.src(0), inst.extra<IterData>()->keyId };
      AliasClass val = AFrame { inst.src(0), inst.extra<IterData>()->valId };
      return iter_effects(inst, inst.src(0), key | val);
    }

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate locals

  case StLoc:
    return PureStore {
      AFrame { inst.src(0), inst.extra<StLoc>()->locId },
      inst.src(1)
    };

  case StLocRange:
    {
      auto const extra = inst.extra<StLocRange>();
      auto acls = AEmpty;

      for (auto locId = extra->start; locId < extra->end; ++locId) {
        acls = acls | AFrame { inst.src(0), locId };
      }
      return PureStore { acls, inst.src(1) };
    }

  case LdLoc:
    return PureLoad { AFrame { inst.src(0), inst.extra<LocalId>()->locId } };

  case CheckLoc:
  case LdLocPseudoMain:
    // Note: LdLocPseudoMain is both a guard and a load, so it must not be a
    // PureLoad.
    return may_load_store(
      AFrame { inst.src(0), inst.extra<LocalId>()->locId },
      AEmpty
    );

  case StLocPseudoMain:
    // This can store to globals or locals, but we don't have globals supported
    // in AliasClass yet.
    return PureStore { AUnknown, inst.src(1) };

  case LdClosureStaticLoc:
    return may_load_store(AFrameAny, AFrameAny);

  //////////////////////////////////////////////////////////////////////
  // Instructions that manipulate class-ref slots

  case LdClsRef:
    return PureLoad {
      AClsRefSlot { inst.src(0), inst.extra<LdClsRef>()->slot }
    };

  case StClsRef:
    return PureStore {
      AClsRefSlot { inst.src(0), inst.extra<StClsRef>()->slot },
      inst.src(1)
    };

  case KillClsRef:
    return may_load_store_kill(
      AEmpty, AEmpty,
      AClsRefSlot { inst.src(0), inst.extra<KillClsRef>()->slot }
    );

  //////////////////////////////////////////////////////////////////////
  // Instructions that manipulate cuf-iter slots

  case DecodeCufIter: {
    auto const iterId = inst.extra<DecodeCufIter>()->iterId;
    auto const func = AliasClass { ACufIterFunc { inst.src(1), iterId } };
    auto const ctx = AliasClass { ACufIterCtx { inst.src(1), iterId } };
    auto const invName = AliasClass { ACufIterInvName { inst.src(1), iterId } };
    return may_raise(
      inst,
      may_load_store(AHeapAny, AHeapAny | func | ctx | invName)
    );
  }

  case StCufIterFunc:
    return PureStore {
      ACufIterFunc { inst.src(0), inst.extra<StCufIterFunc>()->iterId },
      inst.src(1)
    };
  case StCufIterCtx:
    return PureStore {
      ACufIterCtx { inst.src(0), inst.extra<StCufIterCtx>()->iterId },
      inst.src(1)
    };
  case StCufIterInvName:
    return PureStore {
      ACufIterInvName { inst.src(0), inst.extra<StCufIterInvName>()->iterId },
      inst.src(1)
    };
  case LdCufIterFunc:
    return PureLoad {
      ACufIterFunc { inst.src(0), inst.extra<LdCufIterFunc>()->iterId }
    };
  case LdCufIterCtx:
    return PureLoad {
      ACufIterCtx { inst.src(0), inst.extra<LdCufIterCtx>()->iterId }
    };
  case LdCufIterInvName:
    return PureLoad {
      ACufIterInvName { inst.src(0), inst.extra<LdCufIterInvName>()->iterId }
    };

  case KillCufIter: {
    auto const iterId = inst.extra<KillCufIter>()->iterId;
    auto const func = AliasClass { ACufIterFunc { inst.src(0), iterId } };
    auto const ctx = AliasClass { ACufIterCtx { inst.src(0), iterId } };
    auto const invName = AliasClass { ACufIterInvName { inst.src(0), iterId } };
    return may_load_store_kill(AEmpty, AEmpty, func | ctx | invName);
  }

  //////////////////////////////////////////////////////////////////////
  // Pointer-based loads and stores

  case LdMem:
    return PureLoad { pointee(inst.src(0)) };
  case StMem:
    return PureStore { pointee(inst.src(0)), inst.src(1) };

  // TODO(#5962341): These take non-constant offset arguments, and are
  // currently only used for collections and class property inits, so we aren't
  // hooked up yet.
  case StElem:
    return PureStore {
      inst.src(0)->type() <= TPtrToRMembCell
        ? AHeapAny
        : AUnknown,
      inst.src(2)
    };
  case LdElem:
    return PureLoad {
      inst.src(0)->type() <= TPtrToRMembCell
        ? AHeapAny
        : AUnknown
    };

  case LdMBase:
    return PureLoad { AMIStateBase };

  case StMBase:
    return PureStore { AMIStateBase, inst.src(0) };

  case FinishMemberOp:
    return may_load_store_kill(AEmpty, AEmpty, AMIStateAny);

  case BoxPtr:
    {
      auto const mem = pointee(inst.src(0));
      return may_load_store(mem, mem);
    }
  case UnboxPtr:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  case IsNTypeMem:
  case IsTypeMem:
  case CheckTypeMem:
  case CheckInitMem:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case CheckRefInner:
    return may_load_store(ARef { inst.src(0) }, AEmpty);
  case LdRef:
    return PureLoad { ARef { inst.src(0) } };
  case StRef:
    return PureStore { ARef { inst.src(0) }, inst.src(1) };

  case InitObjProps:
    return may_load_store(AEmpty, APropAny);

  // Loads $obj->trace, stores $obj->file and $obj->line.
  case InitThrowableFileAndLine:
    return may_load_store(AHeapAny, APropAny);

  //////////////////////////////////////////////////////////////////////
  // Array loads and stores

  case InitPackedLayoutArray:
    return PureStore {
      AElemI { inst.src(0), inst.extra<InitPackedLayoutArray>()->index },
      inst.src(1)
    };

  case LdVecElem: {
    auto const base = inst.src(0);
    auto const key  = inst.src(1);
    always_assert(base->isA(TVec));
    always_assert(key->isA(TInt));
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
      return may_raise(inst, may_load_store_move(stack_in, AEmpty, stack_in));
    }

  case NewStructArray:
    {
      // NewStructArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructArray>();
      auto const stack_in = AStack {
        inst.src(0),
        extra->offset + static_cast<int32_t>(extra->numKeys) - 1,
        static_cast<int32_t>(extra->numKeys)
      };
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case MemoGet: {
    auto const extra = inst.extra<MemoGet>();
    auto const frame = AFrame {
      inst.src(0),
      AliasIdSet{
        AliasIdSet::IdRange{
          extra->locals.first,
          extra->locals.first + extra->locals.restCount + 1
        }
      }
    };
    auto const base = pointee(inst.src(1));
    return may_load_store(AElemAny | frame | base, AEmpty);
  }
  case MemoSet: {
    auto const extra = inst.extra<MemoSet>();
    auto const frame = AFrame {
      inst.src(0),
      AliasIdSet{
        AliasIdSet::IdRange{
          extra->locals.first,
          extra->locals.first + extra->locals.restCount + 1
        }
      }
    };
    auto const base = pointee(inst.src(1));
    // May re-enter when decrementing previously stored value
    return may_reenter(
      inst,
      may_load_store(
        AElemAny | frame | base,
        AElemAny | base
      )
    );
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

  case ElemMixedArrayK:
  case ElemDictK:
  case ElemKeysetK:
    return IrrelevantEffects {};

  case ProfileMixedArrayOffset:
  case CheckMixedArrayOffset:
  case CheckArrayCOW:
  case ProfileDictOffset:
  case ProfileKeysetOffset:
  case CheckDictOffset:
  case CheckKeysetOffset:
    return may_load_store(AHeapAny, AEmpty);

  case ArrayIsset:
  case AKExistsArr:
    return hack_arr_compat_may_raise(inst, may_load_store(AElemAny, AEmpty));

  case ArrayIdx:
    return hack_arr_compat_may_raise(
      inst,
      may_load_store(AElemAny | ARefAny, AEmpty)
    );

  case DictGetQuiet:
  case DictIsset:
  case DictEmptyElem:
  case DictIdx:
  case KeysetGetQuiet:
  case KeysetIsset:
  case KeysetEmptyElem:
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
    return may_raise(inst, may_load_store(AHeapAny, AHeapAny));

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
  case EmptyElem:
  case IssetElem:
  case CGetProp:
  case CGetPropQ:
  case EmptyProp:
  case IssetProp:
    return may_raise(inst, may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny
    ));

  case VGetElem:
  case SetElem:
  case SetNewElemArray:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case SetNewElem:
  case SetOpElem:
  case SetWithRefElem:
  case UnsetElem:
  case BindElem:
  case BindNewElem:
  case IncDecElem:
  case ElemArrayD:
  case ElemArrayU:
  case ElemVecD:
  case ElemVecU:
  case ElemDictD:
  case ElemDictU:
  case ElemKeysetU:
  case VGetProp:
  case UnsetProp:
  case IncDecProp:
  case SetProp:
  case SetOpProp:
  case BindProp:
    // Right now we generally can't limit any of these better than general
    // re-entry rules, since they can raise warnings and re-enter.
    return may_raise(inst, may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny | all_pointees(inst)
    ));

  case ReservePackedArrayDataNewElem:
    return may_load_store(AHeapAny, AHeapAny);

  /*
   * Intermediate minstr operations. In addition to a base pointer like the
   * operations above, these may take a pointer to MInstrState::tvRef, which
   * they may store to (but not read from).
   */
  case ElemX:
  case ElemDX:
  case ElemUX:
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
  case VectorDoCow:
  case VectorIsset:
    return may_load_store(AHeapAny, AEmpty /* Note */);
  case MapGet:
  case MapSet:
    return may_reenter(inst, may_load_store(AHeapAny, AEmpty /* Note */));


  //////////////////////////////////////////////////////////////////////
  // Instructions that allocate new objects, without reading any other memory
  // at all, so any effects they have on some types of memory locations we
  // track are isolated from anything else we care about.

  case NewArray:
  case NewCol:
  case NewPair:
  case NewInstanceRaw:
  case NewMixedArray:
  case NewDictArray:
  case AllocPackedArray:
  case AllocVecArray:
  case ConvBoolToArr:
  case ConvDblToStr:
  case ConvDblToArr:
  case ConvIntToArr:
  case ConvIntToStr:
  case Box:  // conditional allocation
    return IrrelevantEffects {};

  case AllocObj:
    // AllocObj re-enters to call constructors, but if it weren't for that we
    // could ignore its loads and stores since it's a new object.
    return may_reenter(inst, may_load_store(AEmpty, AEmpty));

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate the stack.

  case LdStk:
    return PureLoad {
      AStack { inst.src(0), inst.extra<LdStk>()->offset, 1 }
    };

  case StStk:
    return PureStore {
      AStack { inst.src(0), inst.extra<StStk>()->offset, 1 },
      inst.src(1)
    };

  case SpillFrame:
    {
      auto const spOffset = inst.extra<SpillFrame>()->spOffset;
      return PureSpillFrame {
        actrec(inst.src(0), spOffset),
        actrec_ctx(inst.src(0), spOffset),
        actrec_func(inst.src(0), spOffset),
        inst.src(1)
      };
    }

  case CheckStk:
    return may_load_store(
      AStack { inst.src(0), inst.extra<CheckStk>()->offset, 1 },
      AEmpty
    );

  // The following may re-enter, and also deal with a stack slot.
  case CastStk:
    {
      auto const stk = AStack {
        inst.src(0), inst.extra<CastStk>()->offset, 1
      };
      return may_raise(inst, may_load_store(stk, stk));
    }
  case CoerceStk:
    {
      auto const stk = AStack {
        inst.src(0), inst.extra<CoerceStk>()->offset, 1
      };
      return may_raise(inst, may_load_store(stk, stk));
    }

  case CastMem:
  case CoerceMem:
    {
      auto aInst = inst.src(0)->inst();
      if (aInst->is(LdLocAddr)) {
        return may_raise(inst, may_load_store(AFrameAny, AFrameAny));
      }
      return may_raise(inst, may_load_store(AUnknown, AUnknown));
    }

  case LdARCtx:
    return PureLoad {
      actrec_ctx(inst.src(0), inst.extra<LdARCtx>()->offset)
    };
  case LdARFuncPtr:
    return PureLoad {
      actrec_func(inst.src(0), inst.extra<LdARFuncPtr>()->offset)
    };

  case DbgAssertARFunc:
    return may_load_store(
      actrec_func(inst.src(0), inst.extra<DbgAssertARFunc>()->offset),
      AEmpty
    );

  case Unreachable:
    // Unreachable code kills every memory location.
    return may_load_store_kill(AEmpty, AEmpty, AUnknown);

  //////////////////////////////////////////////////////////////////////
  // Instructions that never read or write memory locations tracked by this
  // module.

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AndInt:
  case AssertType:
  case AssertLoc:
  case AssertStk:
  case AssertMBase:
  case FuncGuard:
  case DefFP:
  case DefSP:
  case EndGuards:
  case EqBool:
  case EqCls:
  case EqFunc:
  case EqStrPtr:
  case EqArrayDataPtr:
  case EqDbl:
  case EqInt:
  case GteBool:
  case GteInt:
  case GtBool:
  case GtInt:
  case HintLocInner:
  case HintStkInner:
  case HintMBaseInner:
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
  case LdResumableArObj:
  case Shl:
  case Shr:
  case IsNType:
  case IsType:
  case Mov:
  case ConvClsToCctx:
  case ConvDblToBool:
  case ConvDblToInt:
  case IsScalarType:
  case LdMIStateAddr:
  case LdPairBase:
  case CheckStaticLoc:
  case LdStaticLoc:
  case LdClsCns:
  case LdSubClsCns:
  case CheckSubClsCns:
  case LdClsCnsVecLen:
  case ProfileSubClsCns:
  case CheckCtxThis:
  case CheckFuncStatic:
  case LdARNumParams:
  case LdRDSAddr:
  case ExitPlaceholder:
  case CheckRange:
  case ProfileType:
  case LdIfaceMethod:
  case InstanceOfIfaceVtable:
  case CheckARMagicFlag:
  case LdARNumArgsAndFlags:
  case StARNumArgsAndFlags:
  case LdTVAux:
  case LdARInvName:
  case StARInvName:
  case MethodExists:
  case GetTime:
  case ProfileInstanceCheck:
  case Select:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that technically do some things w/ memory, but not in any way
  // we currently care about.  They however don't return IrrelevantEffects
  // because we assume (in refcount-opts) that IrrelevantEffects instructions
  // can't even inspect Countable reference count fields, and several of these
  // can.  All GeneralEffects instructions are assumed to possibly do so.

  case DecRefNZ:
  case AFWHBlockOn:
  case IncRef:
  case LdClosureCtx:
  case StClosureCtx:
  case StClosureArg:
  case StContArKey:
  case StContArValue:
  case LdRetVal:
  case ConvStrToInt:
  case ConvResToInt:
  case OrdStr:
  case ChrInt:
  case CreateSSWH:
  case NewLikeArray:
  case CheckRefs:
  case LdClsCctx:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case FreeActRec:
  case RegisterLiveObj:
  case StContArResume:
  case StContArState:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case CheckCold:
  case CheckInitProps:
  case CheckInitSProps:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContValid:
  case ContStarted:
  case IncProfCounter:
  case IncStat:
  case IncStatGrouped:
  case ContPreNext:
  case ContStartedCheck:
  case ConvArrToBool:
  case ConvArrToDbl:
  case NewColFromArray:
  case CountArray:
  case CountArrayFast:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case StAsyncArResult:
  case StAsyncArResume:
  case StAsyncArSucceeded:
  case InstanceOf:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfIface:
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
  case SameArr:
  case NSameArr:
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case CmpRes:
  case LdBindAddr:
  case LdAsyncArParentChain:
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
  case LdCtx:
  case LdCctx:
  case LdClosure:
  case LdClsName:
  case LdAFWHActRec:
  case LdClsCtx:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
  case LdContField:
  case LdContResumeAddr:
  case LdClsCachedSafe:
  case LdClsInitData:
  case UnwindCheckSideExit:
  case LdCns:
  case LdFuncVecLen:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case FwdCtxStaticCall:
  case ProfileArrayKind:
  case ProfileSwitchDest:
  case LdFuncCachedSafe:
  case LdFuncNumParams:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdObjClass:
  case LdObjInvoke:
  case LdStrLen:
  case StringIsset:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdVectorBase:
  case LdWHResult:
  case LdWHState:
  case LookupClsRDS:
  case DbgTraceCall:
  case InitCtx:
  case PackMagicArgs:
  case StrictlyIntegerConv:
    return may_load_store(AEmpty, AEmpty);

  // Some that touch memory we might care about later, but currently don't:
  case InitStaticLoc:
  case ColIsEmpty:
  case ColIsNEmpty:
  case ConvCellToBool:
  case ConvObjToBool:
  case CountCollection:
  case LdVectorSize:
  case VectorHasImmCopy:
  case CheckPackedArrayDataBounds:
  case LdColVec:
  case LdColDict:
  case EnterFrame:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch most heap things.  They
  // also may generally write to the eval stack below an offset (see
  // alias-class.h above AStack for more).

  case DecRef:
    {
      auto const src = inst.src(0);
      // It could decref the inner ref.
      auto const maybeRef = src->isA(TBoxedCell) ? ARef { src } :
                            src->type().maybe(TBoxedCell) ? ARefAny : AEmpty;
      // Need to add maybeRef to the `store' set. See comments about
      // `GeneralEffects' in memory-effects.h.
      auto const effect = may_load_store(maybeRef, maybeRef);
      if (src->type().maybe(TArr | TVec | TDict | TObj | TRes |
                            TBoxedArr | TBoxedVec | TBoxedDict |
                            TBoxedObj | TBoxedRes)) {
        // Could re-enter to run a destructor. Keysets are exempt because they
        // can only contain strings or integers.
        return may_reenter(inst, effect);
      }
      return effect;
    }

  case GetMemoKey: {
    auto const src = inst.src(0);
    if (src->type().maybe(TArr | TVec | TDict | TKeyset | TObj | TRes)) {
      return may_raise(inst, may_load_store(AHeapAny, AHeapAny));
    }
    return IrrelevantEffects{};
  }

  case LdArrFPushCuf:
  case LdArrFuncCtx:
  case LdStrFPushCuf:
  case LdFunc: // these all can autoload
    {
      AliasClass effects =
        actrec(inst.src(1), inst.extra<IRSPRelOffsetData>()->offset);
      return may_raise(inst, may_load_store(effects, effects));
    }

  case LdObjMethod:    // can't autoload, but can decref $this right now
    {
      AliasClass effects =
        actrec(inst.src(1), inst.extra<LdObjMethod>()->offset);
      return may_raise(inst, may_load_store(effects, effects));
    }

  case LookupClsMethod:   // autoload, and it writes part of the new actrec
    {
      AliasClass effects =
        actrec(inst.src(2), inst.extra<LookupClsMethod>()->calleeAROffset);
      return may_raise(inst, may_load_store(effects, effects));
    }

  case ProfileMethod:
    {
      AliasClass effects =
        actrec(inst.src(0), inst.extra<ProfileMethod>()->bcSPOff);
      return may_load_store(effects, AEmpty);
    }

  case LdClsPropAddrOrNull:   // may run 86{s,p}init, which can autoload
  case LdClsPropAddrOrRaise:  // raises errors, and 86{s,p}init
  case BaseG:
  case Clone:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case RaiseUninitLoc:
  case RaiseUndefProp:
  case RaiseMissingArg:
  case RaiseError:
  case RaiseNotice:
  case RaiseWarning:
  case RaiseMissingThis:
  case FatalMissingThis:
  case RaiseVarEnvDynCall:
  case RaiseHackArrCompatNotice:
  case ConvCellToStr:
  case ConvObjToStr:
  case Count:      // re-enters on CountableClass
  case MIterFree:
  case IterFree:
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
  case ConvCellToArr:  // decrefs src, may read obj props
  case ConvCellToObj:  // decrefs src
  case ConvObjToArr:   // decrefs src
  case ConvObjToVArr:  // can invoke PHP
  case ConvObjToDArr:  // can invoke PHP
  case InitProps:
  case InitSProps:
  case OODeclExists:
  case DefCls:         // autoload
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFuncCached:   // autoload
  case LdFuncCachedU:  // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case InitClsCns:      // autoload
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case StringGet:      // raise_notice
  case OrdStrIdx:      // raise_notice
  case ArrayAdd:       // decrefs source
  case AddElemIntKey:
  case AddElemStrKey:
  case AddNewElem:     // decrefs value
  case DictAddElemIntKey:  // decrefs value
  case DictAddElemStrKey:  // decrefs value
  case ArrayGet:       // kVPackedKind warnings
  case ArraySet:       // kVPackedKind warnings
  case ArraySetRef:    // kVPackedKind warnings
  case DictGet:
  case KeysetGet:
  case VecSet:
  case VecSetRef:
  case DictSet:
  case DictSetRef:
  case ElemArray:
  case ElemArrayW:
  case ElemDict:
  case ElemDictW:
  case ElemKeyset:
  case ElemKeysetW:
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
  case CoerceStrToInt:
  case CoerceStrToDbl:
  case CoerceCellToDbl:
  case CoerceCellToInt:
  case CoerceCellToBool:
  case ConvCellToInt:
  case ConvResToStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConvCellToDbl:
  case ConvArrToVec:
  case ConvArrToDict:
  case ConvObjToVec:
  case ConvObjToDict:
  case ConvObjToKeyset:
  case ThrowOutOfBounds:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowArithmeticError:
  case ThrowDivisionByZeroError:
  case SetOpCell:
    return may_raise(inst, may_load_store(AHeapAny, AHeapAny));

  case ConvArrToKeyset: // Decrefs input values
  case ConvVecToKeyset:
  case ConvDictToKeyset:
    return may_raise(inst, may_load_store(AElemAny, AEmpty));

  case ConvVecToArr:
  case ConvDictToArr:
  case ConvKeysetToArr:
  case ConvDictToVec:
  case ConvKeysetToVec:
  case ConvVecToDict:
  case ConvKeysetToDict:
  case ConvArrToVArr:
  case ConvVecToVArr:
  case ConvDictToVArr:
  case ConvKeysetToVArr:
    return may_load_store(AElemAny, AEmpty);

  case ReleaseVVAndSkip:  // can decref ExtraArgs or VarEnv and Locals
    return may_reenter(inst,
                       may_load_store(AHeapAny|AFrameAny, AHeapAny|AFrameAny));

  // debug_backtrace() traverses stack and WaitHandles on the heap.
  case DebugBacktrace:
  case DebugBacktraceFast:
    return may_load_store(AHeapAny|AFrameAny|AStackAny, AHeapAny);

  // These two instructions don't touch memory we track, except that they may
  // re-enter to construct php Exception objects.  During this re-entry anything
  // can happen (e.g. a surprise flag check could cause a php signal handler to
  // run arbitrary code).
  case ABCUnblock:
  case AFWHPrepareChild:
    return may_reenter(inst, may_load_store(AEmpty, AEmpty));

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

  auto check_fp = [&] (SSATmp* fp) {
    always_assert_flog(
      fp->type() <= TFramePtr,
      "Non frame pointer in memory effects"
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      obj->type() <= TObj,
      "Non obj pointer in memory effects"
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.frame()) check_fp(fr->fp);
    if (auto const sl = a.clsRefSlot()) check_fp(sl->fp);
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

      if (inst.mayRaiseError()) {
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
    [&] (PureStore x)        { check(x.dst);
                               always_assert(x.value != nullptr); },
    [&] (PureSpillFrame x)   { check(x.stk); check(x.ctx); check(x.callee);
                               always_assert(x.ctx <= x.stk);
                               always_assert(x.callee <= x.stk); },
    [&] (ExitEffects x)      { check(x.live); check(x.kills); },
    [&] (IrrelevantEffects)  {},
    [&] (UnknownEffects)     {},
    [&] (CallEffects x)      { check(x.kills);
                               check(x.stack);
                               check(x.locals);
                               check(x.callee);
                               always_assert(x.callee <= x.stack); },
    [&] (ReturnEffects x)    { check(x.kills); }
  );

  return true;
}

//////////////////////////////////////////////////////////////////////

}

MemEffects memory_effects(const IRInstruction& inst) {
  auto const ret = memory_effects_impl(inst);
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
      return PureStore { canonicalize(x.dst), x.value };
    },
    [&] (PureSpillFrame x) -> R {
      return PureSpillFrame {
        canonicalize(x.stk),
        canonicalize(x.ctx),
        canonicalize(x.callee)
      };
    },
    [&] (ExitEffects x) -> R {
      return ExitEffects { canonicalize(x.live), canonicalize(x.kills) };
    },
    [&] (CallEffects x) -> R {
      return CallEffects {
        x.writes_locals,
        canonicalize(x.kills),
        canonicalize(x.stack),
        canonicalize(x.locals),
        canonicalize(x.callee)
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
    [&] (CallEffects x) {
      return sformat("call({} ; {} ; {} ; {})",
        show(x.kills),
        show(x.stack),
        show(x.locals),
        show(x.callee)
      );
    },
    [&] (PureSpillFrame x) {
      return sformat("stFrame({} ; {} ; {})",
        show(x.stk),
        show(x.ctx),
        show(x.callee)
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

AliasClass inline_fp_frame(const IRInstruction* inst) {
  return AStack {
    inst->src(0),
    inst->extra<DefInlineFP>()->spOffset + int32_t{kNumActRecCells} - 1,
    int32_t{kNumActRecCells}
  };
}

//////////////////////////////////////////////////////////////////////

}}
