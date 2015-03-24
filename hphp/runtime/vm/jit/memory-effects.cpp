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
#include "hphp/runtime/vm/jit/memory-effects.h"

#include "hphp/util/match.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/assertions.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

AliasClass pointee(const SSATmp* ptr) {
  always_assert(ptr->type() <= Type::PtrToGen);

  /*
   * First check various kinds of known locations.
   */

  if (ptr->type() <= Type::PtrToFrameGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdLocAddr)) {
      return AFrame { sinst->src(0), sinst->extra<LdLocAddr>()->locId };
    }
    return AFrameAny;
  }

  if (ptr->type() <= Type::PtrToStkGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdStkAddr)) {
      return AStack { sinst->src(0),
        sinst->extra<LdStkAddr>()->offset.offset, 1 };
    }
    return AStackAny;
  }

  if (ptr->type() <= Type::PtrToPropGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdPropAddr)) {
      return AProp {
        sinst->src(0),
        safe_cast<uint32_t>(sinst->extra<LdPropAddr>()->offsetBytes)
      };
    }
    return APropAny;
  }

  if (ptr->type() <= Type::PtrToMISGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdMIStateAddr)) {
      return AMIState { safe_cast<int32_t>(sinst->src(1)->intVal()) };
    }
    return AMIStateAny;
  }

  /*
   * None of the above worked, so try to make the smallest union we can based
   * on the pointer type.
   *
   * Note: we don't support refs in AliasClass yet, so any pointer that
   * contains the R bits unions all heap locations.
   */
  auto const pty = ptr->type();
  auto ret = AliasClass{AEmpty};
  if (pty.maybe(Type::PtrToStkGen))     ret = ret | AStackAny;
  if (pty.maybe(Type::PtrToFrameGen))   ret = ret | AFrameAny;
  if (pty.maybe(Type::PtrToPropGen))    ret = ret | APropAny;
  if (pty.maybe(Type::PtrToArrGen))     ret = ret | AElemAny;
  if (pty.maybe(Type::PtrToMISGen))     ret = ret | AMIStateAny;
  if (pty.maybe(Type::PtrToRefGen))     ret = ret | AHeapAny;
  if (pty.maybe(Type::PtrToClsInitGen)) ret = ret | AHeapAny;
  if (pty.maybe(Type::PtrToClsCnsGen))  ret = ret | AHeapAny;
  return ret;
}

// Return an AliasClass containing all locations pointed to by any PtrToGen
// sources to an instruction.
AliasClass all_pointees(const IRInstruction& inst) {
  auto ret = AliasClass{AEmpty};
  for (auto& src : inst.srcs()) {
    if (src->type() <= Type::PtrToGen) {
      ret = ret | pointee(src);
    }
  }
  return ret;
}

// Return an AliasClass representing a range of the eval stack that contains
// everything below a logical depth.
AliasClass stack_below(SSATmp* base, int32_t offset) {
  return AStack { base, offset, std::numeric_limits<int32_t>::max() };
}

/*
 * Modify a GeneralEffects to take potential VM re-entry into account.  This
 * affects may-load, may-store, and kills information for the instruction.  The
 * GeneralEffects should already contain AHeapAny in both loads and stores if
 * it affects those locations for reasons other than re-entry, but does not
 * need to if it doesn't.
 *
 * For loads, we need to take into account EnableArgsInBacktraces: if this flag
 * is on, any instruction that could re-enter could call debug_backtrace, which
 * could read the argument locals of any activation record in the callstack.
 * We don't try to limit the load effects to argument locals here, though, and
 * just union in all the locals.
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
            DecRefThis,
            ReleaseVVOrExit,
            CIterFree,
            MIterFree,
            IterFree,
            ABCUnblock,
            GenericRetDecRefs);
  always_assert_flog(
    may_reenter_is_ok,
    "instruction {} claimed may_reenter, but it isn't allowed to say that",
    inst
  );

  auto const killed_stack =
    stack_below(inst.marker().fp(), -inst.marker().spOff().offset - 1);
  return GeneralEffects {
    x.loads | AHeapAny
            | (RuntimeOption::EnableArgsInBacktraces ? AFrameAny : AEmpty),
    x.stores | AHeapAny,
    x.moves,
    x.kills | killed_stack
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
  assert(move <= loads);
  return GeneralEffects { loads, stores, move, AEmpty };
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
    return ExitEffects { AUnknown, stack_below(inst.src(0), -1) };
  case JmpSwitchDest:
  case JmpSSwitchDest:
    return ExitEffects { AUnknown, stack_below(inst.src(1), -1) };
  case ReqRetranslate:
  case ReqRetranslateOpt:
    return UnknownEffects {};

  //////////////////////////////////////////////////////////////////////
  // Unusual instructions

  /*
   * The ReturnHook sets up the ActRec so the unwinder knows everything is
   * already released (i.e. it calls ar->setLocalsDecRefd()).
   *
   * This means it can block upward exposed uses of locals via kills, even
   * though it has a catch block as a successor that looks like it can use any
   * locals (and in fact it can, if it weren't for this instruction).  It also
   * can block uses of the stack below the depth at the ReturnHook.
   */
  case ReturnHook:
    // Note, this instruction can re-enter, but doesn't need the may_reenter()
    // treatmeant because of the special kill semantics for locals and stack.
    return may_load_store_kill(
      AHeapAny, AHeapAny,
      stack_below(inst.src(0), 2) | AFrameAny
    );

  // The suspend hooks can load anything (re-entering the VM), but can't write
  // to frame locals.
  case SuspendHookE:
  case SuspendHookR:
    // TODO: may-load here probably doesn't need to include AFrameAny normally.
    return may_reenter(inst, may_load_store(AUnknown, AHeapAny));

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
      stack_below(inst.src(0), inst.extra<RetCtrl>()->spOffset.offset - 1)
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
    return may_reenter(inst, may_load_store(AUnknown, AUnknown));

  case EndCatch:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(1), inst.extra<EndCatch>()->offset.offset - 1)
    };

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
   * TODO(#3634984): Additionally, DefInlineFP is marking may-load on all the
   * locals of the outer frame.  This is probably not necessary anymore, but we
   * added it originally because a store sinking prototype needed to know it
   * can't push StLocs past a DefInlineFP, because of reserved registers.
   * Right now it's just here because we need to think about and test it before
   * removing that set.
   */
  case DefInlineFP:
    return may_load_store(
      AFrameAny | inline_fp_frame(&inst),
      /*
       * Note that although DefInlineFP is going to store some things into the
       * memory for the new frame (m_soff, etc), it's as part of converting it
       * from a pre-live frame to a live frame.  We don't need to report those
       * effects on memory because they are logically to a 'different location
       * class' (i.e. an activation record for the callee) than the AStack
       * locations that represented the pre-live ActRec, even though they are at
       * the same physical addresses in memory.
       */
      AEmpty
    );

  case InlineReturn:
    return ReturnEffects { stack_below(inst.src(0), 2) };

  case InterpOne:
  case InterpOneCF:
    return InterpOneEffects {
      // We could be more precise about which stack locations (or which locals)
      // an InterpOne may read (this information is in its extra data), but
      // this hasn't been implemented.
      stack_below(inst.src(1), -inst.marker().spOff().offset - 1)
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
  case VerifyParamFail:
    return may_reenter(inst, may_load_store(AUnknown, AHeapAny));
  // However the following ones can't read locals from our frame on the way
  // out.
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
    return may_reenter(inst, may_load_store(AHeapAny, AHeapAny));

  case CallArray:
    return CallEffects {
      inst.extra<CallArray>()->destroyLocals,
      AEmpty,
      // The AStackAny on this is more conservative than it could be; see Call
      // and CallBuiltin.
      AStackAny
    };
  case ContEnter:
    return CallEffects { false, AEmpty, AStackAny };

  case Call:
    {
      auto const extra = inst.extra<Call>();
      return CallEffects {
        extra->destroyLocals,
        stack_below(inst.src(0), extra->spOffset.offset - 1), // kill
        // We might side-exit inside the callee, and interpret a return.  So we
        // can read anything anywhere on the eval stack above the call's entry
        // depth here.
        AStackAny
      };
    }

  case CallBuiltin:
    {
      auto const extra = inst.extra<CallBuiltin>();
      auto const stk = [&] () -> AliasClass {
        AliasClass ret = AEmpty;
        for (auto i = uint32_t{2}; i < inst.numSrcs(); ++i) {
          if (inst.src(i)->type() <= Type::PtrToGen) {
            auto const cls = pointee(inst.src(i));
            if (cls.maybe(AStackAny)) {
              ret = ret | cls;
            }
          }
        }
        return ret;
      }();
      auto const locs = extra->destroyLocals ? AFrameAny : AEmpty;
      return may_reenter(inst, may_load_store(stk | AHeapAny | locs, locs));
    }

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateAFWH:
  case CreateCont:
    return may_load_store_move(AFrameAny, AHeapAny, AFrameAny);

  // This re-enters to call extension-defined instance constructors.
  case ConstructInstance:
    return may_reenter(inst, may_load_store(AHeapAny, AHeapAny));

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case MIterInit:
  case WIterInit:
    return IterEffects {
      inst.src(1),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff().offset - 1)
    };
  case IterNext:
  case MIterNext:
  case WIterNext:
    return IterEffects {
      inst.src(0),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff().offset - 1)
    };

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    return IterEffects2 {
      inst.src(1),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff().offset - 1)
    };

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    return IterEffects2 {
      inst.src(0),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff().offset - 1)
    };

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate locals

  case StLoc:
    return PureStore {
      AFrame { inst.src(0), inst.extra<StLoc>()->locId },
      inst.src(1)
    };

  case StLocNT:
    return PureStoreNT {
      AFrame { inst.src(0), inst.extra<StLocNT>()->locId },
      inst.src(1)
    };

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

  case ClosureStaticLocInit:
    return may_load_store(AFrameAny, AFrameAny);

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
      inst.src(0)->type() <= Type::PtrToRMembCell
        ? AHeapAny | AMIStateAny
        : AUnknown,
      inst.src(2)
    };
  case LdElem:
    return PureLoad {
      inst.src(0)->type() <= Type::PtrToRMembCell
        ? AHeapAny | AMIStateAny
        : AUnknown
    };

  case BoxPtr:
  case UnboxPtr:
    {
      auto const mem = pointee(inst.src(0));
      return may_load_store(mem, mem);
    }

  case IsNTypeMem:
  case IsTypeMem:
  case CheckTypeMem:
  case DbgAssertPtr:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  case CheckInitMem:
    return may_load_store(pointee(inst.src(0)), AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case CheckRefInner:
    // We don't have AliasClass support for refs yet, so it's a load from an
    // unknown heap location.
    return may_load_store(AHeapAny, AEmpty);
  case LdRef:
    return PureLoad { AHeapAny };

  case StRef:
    // We don't have anything for ref locations at this point, but we know it
    // is a heap location.
    return PureStore { AHeapAny, inst.src(1) };

  case InitObjProps:
    return may_load_store(AEmpty, APropAny);

  //////////////////////////////////////////////////////////////////////
  // Array loads and stores

  case InitPackedArray:
    return PureStore {
      AElemI { inst.src(0), inst.extra<InitPackedArray>()->index },
      inst.src(1)
    };

  // TODO(#5575265): use LdMem for this instruction.
  case LdPackedArrayElem:
    if (inst.src(1)->hasConstVal() && inst.src(1)->intVal() >= 0) {
      return PureLoad {
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) }
      };
    }
    return PureLoad { AElemIAny };

  case LdStructArrayElem:
    assert(inst.src(1)->strVal()->isStatic());
    return PureLoad { AElemS { inst.src(0), inst.src(1)->strVal() } };

  // TODO(#5575265): replace this instruction with CheckTypeMem.
  case CheckTypePackedArrayElem:
  case IsPackedArrayElemNull:
    if (inst.src(1)->hasConstVal() && inst.src(1)->intVal() >= 0) {
      return may_load_store(
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) },
        AEmpty
      );
    }
    return may_load_store(AElemIAny, AEmpty);

  case InitPackedArrayLoop:
    {
      auto const extra = inst.extra<InitPackedArrayLoop>();
      auto const stack_in = AStack {
        inst.src(1),
        extra->offset.offset + static_cast<int32_t>(extra->size) - 1,
        static_cast<int32_t>(extra->size)
      };
      return may_load_store_move(stack_in, AElemIAny, stack_in);
    }

  case NewStructArray:
    {
      // NewStructArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructArray>();
      auto const stack_in = AStack {
        inst.src(0),
        extra->offset.offset + static_cast<int32_t>(extra->numKeys) - 1,
        static_cast<int32_t>(extra->numKeys)
      };
      return may_load_store_move(stack_in, AEmpty, stack_in);
    }

  case ArrayIdx:
    return may_load_store(AHeapAny, AHeapAny);

  case AKExistsArr:
    return may_load_store(AHeapAny, AHeapAny);
  case AKExistsObj:
    return may_reenter(inst, may_load_store(AHeapAny, AHeapAny));

  //////////////////////////////////////////////////////////////////////
  // Member instructions

  /*
   * Various minstr opcodes that take a PtrToGen in src 0, which may or may not
   * point to a frame local or the evaluation stack.  These instructions can
   * all re-enter the VM and access arbitrary heap locations, and some of them
   * take pointers to MinstrState locations, which they may both load and store
   * from if present.
   */
  case CGetElem:
  case ElemArray:
  case ElemArrayW:
  case ElemX:
  case EmptyElem:
  case IssetElem:
  case BindElem:
  case BindNewElem:
  case ElemDX:
  case ElemUX:
  case IncDecElem:
  case SetElem:
  case SetNewElemArray:
  case SetNewElem:
  case SetOpElem:
  case SetWithRefElem:
  case SetWithRefNewElem:
  case UnsetElem:
  case VGetElem:
    // Right now we generally can't limit any of these better than general
    // re-entry rules, since they can raise warnings and re-enter.
    assert(inst.src(0)->type() <= Type::PtrToGen);
    return may_reenter(inst, may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny | all_pointees(inst)
    ));

  /*
   * These minstr opcodes either take a PtrToGen or an Obj as the base.  The
   * pointer may point at frame locals or the stack.  These instructions can
   * all re-enter the VM and access arbitrary non-frame/stack locations, as
   * well.
   */
  case CGetProp:
  case CGetPropQ:
  case EmptyProp:
  case IssetProp:
  case PropX:
  case PropQ:
  case UnsetProp:
  case BindProp:
  case IncDecProp:
  case PropDX:
  case SetOpProp:
  case SetProp:
  case VGetProp:
    return may_reenter(inst, may_load_store(
      AHeapAny | all_pointees(inst),
      AHeapAny | all_pointees(inst)
    ));

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
  case NewInstanceRaw:
  case NewMixedArray:
  case AllocPackedArray:
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
      AStack { inst.src(0), inst.extra<LdStk>()->offset.offset, 1 }
    };

  case StStk:
    return PureStore {
      AStack { inst.src(0), inst.extra<StStk>()->offset.offset, 1 },
      inst.src(1)
    };

  case SpillFrame:
    {
      auto const spOffset = inst.extra<SpillFrame>()->spOffset;
      return PureSpillFrame {
        AStack {
          inst.src(0),
          // SpillFrame's spOffset is to the bottom of where it will store the
          // ActRec, but AliasClass needs an offset to the highest cell it will
          // store.
          spOffset.offset + int32_t{kNumActRecCells} - 1,
          kNumActRecCells
        },
        AStack {
          inst.src(0),
          // The context is in the highest slot.
          spOffset.offset + int32_t{kNumActRecCells} - 1,
          1
        }
      };
    }

  case CheckStk:
    return may_load_store(
      AStack { inst.src(0), inst.extra<CheckStk>()->irSpOffset.offset, 1 },
      AEmpty
    );
  case CufIterSpillFrame:
    return may_load_store(AEmpty, AStackAny);

  // The following may re-enter, and also deal with a stack slot.
  case CastStk:
    {
      auto const stk = AStack {
        inst.src(0), inst.extra<CastStk>()->offset.offset, 1
      };
      return may_reenter(inst, may_load_store(stk, stk));
    }
  case CoerceStk:
    {
      auto const stk = AStack {
        inst.src(0),
        inst.extra<CoerceStk>()->offset.offset, 1
      };
      return may_reenter(inst, may_load_store(stk, stk));
    }

  case LdARFuncPtr:
    // This instruction is essentially a PureLoad, but we don't handle non-TV's
    // in PureLoad so we have to treat it as may_load_store.  We also treat it
    // as loading an entire ActRec-sized part of the stack, although it only
    // loads the slot containing the Func.
    return may_load_store(
      AStack {
        inst.src(0),
        inst.extra<LdARFuncPtr>()->offset + int32_t{kNumActRecCells} - 1,
        int32_t{kNumActRecCells}
      },
      AEmpty
    );

  //////////////////////////////////////////////////////////////////////
  // Instructions that never do anything to memory

  case AssertStk:
  case HintStkInner:
  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AndInt:
  case AssertLoc:
  case AssertType:
  case DefFP:
  case DefSP:
  case EndGuards:
  case EqDbl:
  case EqInt:
  case GteInt:
  case GtInt:
  case HintLocInner:
  case Jmp:
  case JmpNZero:
  case JmpZero:
  case LdPropAddr:
  case LdStkAddr:
  case LteDbl:
  case LteInt:
  case LtInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case DivDbl:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case NeqDbl:
  case NeqInt:
  case ReDefSP:
  case AdjustSP:
  case ResetSP:
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
  case ClsNeq:
  case Mod:
  case TakeRef:
  case TakeStk:
  case Conjure:
  case DefMIStateBase:
  case Halt:
  case ConvBoolToInt:
  case ConvBoolToDbl:
  case DbgAssertType:
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
  case LdStaticLocCached:
  case CheckCtxThis:
  case CastCtxThis:
  case LdRDSAddr:
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
  case IncRefCtx:
  case StClosureArg:
  case StClosureCtx:
  case StContArKey:
  case StContArValue:
  case StRetVal:
  case ConvStrToInt:
  case OrdStr:
  case CreateSSWH:
  case NewLikeArray:
  case CheckRefs:
  case LdClsCctx:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case FreeActRec:
  case LdRetAddr:
  case RegisterLiveObj:
  case RetAdjustStk:
  case StClosureFunc:
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
  case IncProfCounter:
  case IncStat:
  case IncStatGrouped:
  case CountBytecode:
  case ContPreNext:
  case ContStartedCheck:
  case ConvArrToBool:
  case ConvArrToDbl:
  case ConvArrToInt:
  case ConvBoolToStr:
  case CountArray:
  case CountArrayFast:
  case StAsyncArResult:
  case StAsyncArResume:
  case StAsyncArSucceeded:
  case InstanceOf:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfIface:
  case InterfaceSupportsArr:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case IsWaitHandle:
  case DbgAssertRefCount:
  case NSame:
  case Same:
  case Gt:
  case Gte:
  case Eq:
  case Lt:
  case Lte:
  case Neq:
  case IncTransCounter:
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
  case DerefClsRDSHandle:
  case EagerSyncVMRegs:
  case ExtendsClass:
  case LdUnwinderValue:
  case GetCtxFwdCall:
  case LdCctx:
  case LdCtx:
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
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case ProfilePackedArray:
  case ProfileStructArray:
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
  case LookupClsRDSHandle:
  case GetCtxFwdCallDyn:
    return may_load_store(AEmpty, AEmpty);

  // Some that touch memory we might care about later, but currently don't:
  case CheckStaticLocInit:
  case StaticLocInitCached:
  case ColIsEmpty:
  case ColIsNEmpty:
  case ConvCellToBool:
  case ConvObjToBool:
  case CountCollection:
  case LdVectorSize:
  case VectorHasImmCopy:
  case CheckPackedArrayBounds:
  case LdColArray:
    return may_load_store(AEmpty, AEmpty);

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch most heap things.  They
  // also may generally write to the eval stack below an offset (see
  // alias-class.h above AStack for more).

  case DecRefThis:
    return may_reenter(inst, may_load_store(AEmpty, AEmpty));
  case DecRef:
    if (inst.src(0)->type().maybe(Type::Arr | Type::Obj)) {
      // Could re-enter to run a destructor.
      return may_reenter(inst, may_load_store(AEmpty, AEmpty));
    }
    return may_load_store(AEmpty, AEmpty);

  case LdArrFPushCuf:  // autoloads
  case LdArrFuncCtx:   // autoloads
  case LdObjMethod:    // can't autoload, but can decref $this right now
  case LdStrFPushCuf:  // autoload
    /*
     * Note that these instructions make stores to a pre-live actrec on the
     * eval stack.
     *
     * It is probably safe for these instructions to have may-load only from
     * the portion of the evaluation stack below the actrec they are
     * manipulating, but since there's always going to be either a Call or a
     * region exit following it it doesn't help us eliminate anything for now,
     * so we just pretend it can read/write anything on the stack.
     */
    return may_reenter(inst, may_load_store(AStackAny, AStackAny));

  case LdClsPropAddrOrNull:   // may run 86{s,p}init, which can autoload
  case LdClsPropAddrOrRaise:  // raises errors, and 86{s,p}init
  case BaseG:
  case Clone:
  case WarnNonObjProp:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case RaiseError:
  case RaiseNotice:
  case RaiseUndefProp:
  case RaiseUninitLoc:
  case RaiseWarning:
  case ConvCellToStr:
  case ConvObjToStr:
  case ConcatCellCell:
  case Count:      // re-enters on CountableClass
  case CIterFree:  // decrefs context object in iter
  case MIterFree:
  case IterFree:
  case EqX:
  case GteX:
  case GtX:
  case LteX:
  case LtX:
  case NeqX:
  case DecodeCufIter:
  case ReleaseVVOrExit:  // can decref fields in an ExtraArgs structure
  case ConvCellToArr:  // decrefs src, may read obj props
  case ConvCellToObj:  // decrefs src
  case ConvObjToArr:   // decrefs src
  case CustomInstanceInit:
  case GenericIdx:
  case InitProps:
  case InitSProps:
  case OODeclExists:
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFunc:         // autoload
  case LdFuncCached:   // autoload
  case LdFuncCachedU:  // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case LookupClsCns:      // autoload
  case LookupClsMethod:   // autoload
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case StringGet:   // raise_warning
  case ArrayAdd:    // decrefs source
  case AddElemIntKey:  // decrefs value
  case AddElemStrKey:  // decrefs value
  case AddNewElem:     // decrefs value
  case ArrayGet:       // kVPackedKind warnings
  case ArrayIsset:     // kVPackedKind warnings
  case ArraySet:       // kVPackedKind warnings
  case ArraySetRef:    // kVPackedKind warnings
  case GetMemoKey:  // re-enters to call getInstanceKey() in some cases
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
  case ColAddElemC:
  case ColAddNewElemC:
  case CoerceStrToInt:
  case CoerceStrToDbl:
  case CoerceCellToDbl:
  case CoerceCellToInt:
  case CoerceCellToBool:
  case CheckBounds:
  case ConvCellToInt:
  case ConvResToStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConvCellToDbl:
    return may_reenter(inst, may_load_store(AHeapAny, AHeapAny));

  // These two instructions don't touch memory we track, except that they may
  // re-enter to construct php Exception objects.  During this re-entry anything
  // can happen (e.g. a surprise flag check could cause a php signal handler to
  // run arbitrary code).
  case ABCUnblock:
  case AFWHPrepareChild:
    return may_reenter(inst, may_load_store(AEmpty, AEmpty));

  //////////////////////////////////////////////////////////////////////
  // The following instructions are used for debugging memory optimizations, so
  // this analyzer should pretend they don't exist.

  case DbgTrashStk:
  case DbgTrashFrame:
  case DbgTrashMem:
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
      fp->type() <= Type::FramePtr,
      "Non frame pointer in memory effects"
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      obj->type() <= Type::Obj,
      "Non obj pointer in memory effects"
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.frame()) check_fp(fr->fp);
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
         * depth.
         */
        always_assert(AStackAny.maybe(x.kills));
      }
    },
    [&] (PureLoad x)         { check(x.src); },
    [&] (PureStore x)        { check(x.dst);
                               always_assert(x.value != nullptr); },
    [&] (PureStoreNT x)      { check(x.dst);
                               always_assert(x.value != nullptr); },
    [&] (PureSpillFrame x)   { check(x.dst); check(x.ctx);
                               always_assert(x.ctx <= x.dst); },
    [&] (IterEffects x)      { check_fp(x.fp); check(x.kills); },
    [&] (IterEffects2 x)     { check_fp(x.fp); check(x.kills); },
    [&] (ExitEffects x)      { check(x.live); check(x.kills); },
    [&] (IrrelevantEffects)  {},
    [&] (UnknownEffects)     {},
    [&] (InterpOneEffects x) { check(x.kills); },
    [&] (CallEffects x)      { check(x.kills); check(x.stack); },
    [&] (ReturnEffects x)    { check(x.kills); }
  );

  return true;
}

//////////////////////////////////////////////////////////////////////

}

MemEffects memory_effects(const IRInstruction& inst) {
  auto const ret = memory_effects_impl(inst);
  assert(check_effects(inst, ret));
  return ret;
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
    [&] (PureStoreNT x) -> R {
      return PureStoreNT { canonicalize(x.dst), x.value };
    },
    [&] (PureSpillFrame x) -> R {
      return PureSpillFrame { canonicalize(x.dst), canonicalize(x.ctx) };
    },
    [&] (ExitEffects x) -> R {
      return ExitEffects { canonicalize(x.live), canonicalize(x.kills) };
    },
    [&] (CallEffects x) -> R {
      return CallEffects {
        x.destroys_locals,
        canonicalize(x.kills),
        canonicalize(x.stack)
      };
    },
    [&] (ReturnEffects x) -> R {
      return ReturnEffects { canonicalize(x.kills) };
    },
    [&] (IterEffects x) -> R {
      return IterEffects { x.fp, x.id, canonicalize(x.kills) };
    },
    [&] (IterEffects2 x) -> R {
      return IterEffects2 { x.fp, x.id1, x.id2, canonicalize(x.kills) };
    },
    [&] (InterpOneEffects x) -> R {
      return InterpOneEffects { canonicalize(x.kills) };
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
      return sformat("call({} ; {})", show(x.kills), show(x.stack));
    },
    [&] (InterpOneEffects x) {
      return sformat("interp({})", show(x.kills));
    },
    [&] (PureSpillFrame x) {
      return sformat("stFrame({} ; {})", show(x.dst), show(x.ctx));
    },
    [&] (PureLoad x)        { return sformat("ld({})", show(x.src)); },
    [&] (PureStore x)       { return sformat("st({})", show(x.dst)); },
    [&] (PureStoreNT x)     { return sformat("stNT({})", show(x.dst)); },
    [&] (ReturnEffects x)   { return sformat("return({})", show(x.kills)); },
    [&] (IterEffects)       { return "IterEffects"; },
    [&] (IterEffects2)      { return "IterEffects2"; },
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
