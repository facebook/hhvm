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

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

AliasClass pointee(const SSATmp* ptr) {
  always_assert(ptr->type().isPtr());

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
      return AStack { sinst->src(0), sinst->extra<LdStkAddr>()->offset, 1 };
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

  // We have various other situations here that we don't track in this module
  // yet, but we can possibly exclude the locations we care about so far.
  auto const pty = ptr->type();
  if (!pty.maybe(Type::PtrToStkGen) && !pty.maybe(Type::PtrToFrameGen)) {
    return AHeapAny;
  }
  if (!pty.maybe(Type::PtrToFrameGen)) return ANonFrame;
  if (!pty.maybe(Type::PtrToStkGen)) return ANonStack;

  return AUnknown;
}

// Return an AliasClass representing a range of the eval stack that contains
// everything below a logical depth.
AliasClass stack_below(SSATmp* base, int32_t offset) {
  return AStack { base, offset, std::numeric_limits<int32_t>::max() };
}

/*
 * Returns an AliasClass that must be unioned into the may-load set of any
 * instruction that can re-enter the VM.  This set is empty if
 * EnableArgsInBacktraces is off---when it's on, in general re-entry could lead
 * to a call to debug_backtrace which could read the argument locals of any
 * activation in the callstack.
 *
 * We don't try to limit the effects to argument locals, though, and just union
 * in all the locals.
 *
 * This is unioned in in general when an instruction can re-enter because it
 * also makes that somewhat more obvious.
 */
AliasClass reentry_extra() {
  return RuntimeOption::EnableArgsInBacktraces ? AFrameAny : AEmpty;
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
   * So it has no upward exposed uses of locals, even though it has a catch
   * block as a successor that looks like it can use any locals (and in fact it
   * can, if it weren't for this instruction).
   */
  case ReturnHook:
    return KillFrameLocals { inst.src(0) };

  // The suspend hooks can load anything (re-entering the VM), but can't write
  // to frame locals.
  case SuspendHookE:
  case SuspendHookR:
    // TODO: may-load here probably doesn't need to include AFrameAny normally.
    return MayLoadStore { AUnknown | reentry_extra(), ANonFrame };

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
      stack_below(inst.src(0), inst.extra<RetCtrl>()->spOffset - 1)
    };

  case GenericRetDecRefs:
    return MayLoadStore { AUnknown, ANonFrame };

  case EndCatch:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(1), inst.extra<EndCatch>()->offset - 1)
    };

  /*
   * DefInlineFP has some special treatment here.
   *
   * It's logically `publishing' a pointer to a pre-live ActRec, making it
   * live.  It doesn't actually load from this ActRec, but after it's done this
   * the set of things that can load from it is large enough that the easiest
   * way to model this is to consider `publishing' it the load.  This works
   * because once it's publish, it's an activation record, and doesn't get
   * written to as if it were a stack slot anymore (we've effectively converted
   * AStack locations into a frame until the InlineReturn).
   *
   * TODO(#3634984): Additionally, DefInlineFP is marking may-load on all the
   * locals of the outer frame.  This is probably not necessary anymore, but we
   * added it originally because a store sinking prototype needed to know it
   * can't push StLocs past a DefInlineFP, because of reserved registers.
   * Right now it's just here because we need to think about and test it before
   * removing that set.
   */
  case DefInlineFP:
    return MayLoadStore {
      AFrameAny |
        AStack {
          inst.src(0),
          inst.extra<DefInlineFP>()->spOffset + int32_t{kNumActRecCells} - 1,
          int32_t{kNumActRecCells}
        },
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
    };

  case InlineReturn:
    return KillFrameLocals { inst.src(0) };

  case InterpOne:
  case InterpOneCF:
    return InterpOneEffects {
      // We could be more precise about which stack locations (or which locals)
      // an InterpOne may read (this information is in its extra data), but
      // this hasn't been implemented.
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
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
    return MayLoadStore { AUnknown, ANonFrame };
  // However the following ones can't read locals from our frame on the way
  // out.
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

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
        stack_below(inst.src(0), extra->spOffset - 1), // kill
        // We might side-exit inside the callee, and interpret a return.  So we
        // can read anything anywhere on the eval stack above the call's entry
        // depth here.
        AStackAny
      };
    }

  /*
   * CallBuiltin takes args either as php values (subtypes of Gen) or as
   * PtrToStkGen args that it writes/reads.  The `stack' set for its call
   * effects need only contain those portions of the stack.  See DefInlineFP
   * for discussion of how live ActRecs for inlined calls fit into that.
   */
  case CallBuiltin:
    {
      auto const extra = inst.extra<CallBuiltin>();
      AliasClass stk = AEmpty;
      for (auto i = uint32_t{2}; i < inst.numSrcs(); ++i) {
        if (inst.src(i)->type() <= Type::PtrToGen) {
          auto const cls = pointee(inst.src(i));
          if (cls.maybe(AStackAny)) {
            stk = stk | cls;
          }
        }
      }
      return CallEffects {
        extra->destroyLocals,
        stack_below(inst.src(1), extra->spOffset - 1),
        stk
      };
    }

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateAFWH:
  case CreateCont:
    return MayLoadStore { AFrameAny, ANonFrame };

  // This re-enters to call extension-defined instance constructors.
  case ConstructInstance:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case MIterInit:
  case WIterInit:
    return IterEffects {
      inst.src(1),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
    };
  case IterNext:
  case MIterNext:
  case WIterNext:
    return IterEffects {
      inst.src(0),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff() - 1)
    };

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    return IterEffects2 {
      inst.src(1),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
    };

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    return IterEffects2 {
      inst.src(0),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff() - 1)
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
  case GuardLoc:
  case LdLocPseudoMain:
    return MayLoadStore {
      AFrame { inst.src(0), inst.extra<LocalId>()->locId },
      AEmpty
    };

  case StLocPseudoMain:
    // This can store to globals or locals, but we don't have globals supported
    // in AliasClass yet.
    return PureStore { AUnknown };

  case ClosureStaticLocInit:
    return MayLoadStore { AFrameAny, AFrameAny };

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
  case LdElem:
    if (inst.src(0)->type() <= Type::PtrToMembCell) {
      return MayLoadStore { AHeapAny, AEmpty };
    }
    return MayLoadStore { AUnknown, AEmpty };

  case BoxPtr:
  case UnboxPtr:
    {
      auto const mem = pointee(inst.src(0));
      return MayLoadStore { mem, mem };
    }

  case IsNTypeMem:
  case IsTypeMem:
  case CheckTypeMem:
  case DbgAssertPtr:
  case ProfileStr:
    return MayLoadStore { pointee(inst.src(0)), AEmpty };

  case CheckInitMem:
    return MayLoadStore { pointee(inst.src(0)), AEmpty };

  case DecRefMem:
    return MayLoadStore {
      // DecRefMem can re-enter to run a destructor.  We also need to union in
      // the pointee because it may point to a non-heap location.
      pointee(inst.src(0)) | AHeapAny | reentry_extra(),
      ANonFrame
    };

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case CheckRefInner:
    // We don't have AliasClass support for refs yet, so it's a load from an
    // unknown heap location.
    return MayLoadStore { AHeapAny, AEmpty };
  case LdRef:
    return PureLoad { AHeapAny };

  case StRef:
    // We don't have anything for ref locations at this point, but we know it
    // is a heap location.
    return PureStore { AHeapAny };

  case InitObjProps:
    return MayLoadStore { AEmpty, APropAny };

  //////////////////////////////////////////////////////////////////////
  // Array loads and stores

  case InitPackedArray:
    return PureStore {
      AElemI { inst.src(0), inst.extra<InitPackedArray>()->index },
      inst.src(1)
    };

  // TODO(#5575265): use LdMem for this instruction.
  case LdPackedArrayElem:
    if (inst.src(1)->isConst() && inst.src(1)->intVal() >= 0) {
      return PureLoad {
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) }
      };
    }
    return PureLoad { AElemIAny };

  case LdStructArrayElem:
    assert(inst.src(1)->isConst() && inst.src(1)->strVal()->isStatic());
    return PureLoad { AElemS { inst.src(0), inst.src(1)->strVal() } };

  // TODO(#5575265): replace this instruction with CheckTypeMem.
  case CheckTypePackedArrayElem:
  case IsPackedArrayElemNull:
    if (inst.src(1)->isConst() && inst.src(1)->intVal() >= 0) {
      return MayLoadStore {
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) },
        AEmpty
      };
    }
    return MayLoadStore { AElemIAny, AEmpty };

  case InitPackedArrayLoop:
    {
      auto const extra = inst.extra<InitPackedArrayLoop>();
      return MayLoadStore {
        AStack {
          inst.src(1),
          extra->offset + static_cast<int32_t>(extra->size) - 1,
          static_cast<int32_t>(extra->size)
        },
        AElemIAny
      };
    }

  case NewStructArray:
    {
      // NewStructArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructArray>();
      return MayLoadStore {
        AStack {
          inst.src(0),
          extra->offset + static_cast<int32_t>(extra->numKeys) - 1,
          static_cast<int32_t>(extra->numKeys)
        },
        AEmpty
      };
    }

  //////////////////////////////////////////////////////////////////////
  // Member instructions

  /*
   * Various minstr opcodes that take a PtrToGen in src 0, which may or may not
   * point to a frame local or the evaluation stack.  These instructions can
   * all re-enter the VM and access arbitrary non-frame/stack locations, as
   * well.
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
    return MayLoadStore {
      AHeapAny | pointee(inst.src(0)) | reentry_extra(),
      ANonFrame | pointee(inst.src(0))
    };

  /*
   * These minstr opcodes either take a PtrToGen or an Obj as the base.  The
   * pointer may point at frame locals or the stack.  These instructions can
   * all re-enter the VM and access arbitrary non-frame/stack locations, as
   * well.
   */
  case CGetProp:
  case EmptyProp:
  case IssetProp:
  case PropX:
  case UnsetProp:
  case BindProp:
  case IncDecProp:
  case PropDX:
  case SetOpProp:
  case SetProp:
  case VGetProp:
    if (inst.src(0)->type() <= Type::PtrToGen) {
      return MayLoadStore {
        AHeapAny | pointee(inst.src(0)) | reentry_extra(),
        ANonFrame | pointee(inst.src(0))
      };
    }
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  /*
   * Collection accessors can read from their inner array buffer, but stores
   * COW and behave as if they only affect collection memory locations.  We
   * don't track those, so it's returning AEmpty for now.
   */
  case MapGet:
  case MapIsset:
  case MapSet:
  case PairIsset:
  case VectorDoCow:
  case VectorIsset:
    return MayLoadStore { AHeapAny, AEmpty /* Note */ };

  //////////////////////////////////////////////////////////////////////
  // Instructions that allocate new objects, so any effects they have on some
  // types of memory locations we track are isolated from anything else we care
  // about.

  case NewArray:
  case NewCol:
  case NewInstanceRaw:
  case NewLikeArray:
  case NewMIArray:
  case NewMixedArray:
  case NewMSArray:
  case NewVArray:
  case AllocPackedArray:
  case ConvBoolToArr:
  case ConvDblToStr:
  case ConvDblToArr:
  case ConvIntToArr:
  case ConvIntToStr:
  case ConvResToStr:
  case CreateSSWH:
  case Box:  // conditional allocation
    return IrrelevantEffects {};

  case AllocObj:  // AllocObj re-enters to call constructors.
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

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
    return PureSpillFrame {
      AStack {
        inst.src(0),
        // SpillFrame's spOffset is to the bottom of where it will store the
        // ActRec, but AliasClass needs an offset to the highest cell it will
        // store.
        inst.extra<SpillFrame>()->spOffset + int32_t{kNumActRecCells} - 1,
        kNumActRecCells
      }
    };

  case GuardStk:
  case CheckStk:
    return MayLoadStore {
      AStack { inst.src(0), inst.extra<StackOffset>()->offset, 1 },
      AEmpty
    };
  case CufIterSpillFrame:
    return MayLoadStore { AEmpty, AStackAny };

  // The following may re-enter, and also deal with a stack slot.
  case CastStk:
    return MayLoadStore {
      AHeapAny | reentry_extra()
               | AStack { inst.src(0), inst.extra<CastStk>()->offset, 1 },
      ANonFrame
    };
  case CoerceStk:
    return MayLoadStore {
      AHeapAny | reentry_extra()
               | AStack { inst.src(0), inst.extra<CoerceStk>()->offset, 1 },
      ANonFrame
    };

  case GuardRefs:
    // We're not bothering with being exact about where on the stack this
    // instruction can load, because it's always before anything else in a
    // region.
    return MayLoadStore { AStackAny, AEmpty };

  case LdARFuncPtr:
    // This instruction is essentially a PureLoad, but we don't handle non-TV's
    // in PureLoad so we have to treat it as MayLoadStore.  We also treat it as
    // loading an entire ActRec-sized part of the stack, although it only loads
    // the slot containing the Func.
    return MayLoadStore {
      AStack {
        inst.src(0),
        inst.extra<LdARFuncPtr>()->offset + int32_t{kNumActRecCells} - 1,
        int32_t{kNumActRecCells}
      },
      AEmpty
    };

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
  case DecRefNZ:
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
  // we currently care about.

  case CheckRefs:
  case ABCUnblock:
  case AFWHBlockOn:
  case LdClsCctx:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case FreeActRec:
  case IncRef:
  case IncRefCtx:
  case LdRetAddr:
  case RegisterLiveObj:
  case RetAdjustStk:
  case StClosureArg:
  case StClosureCtx:
  case StClosureFunc:
  case StContArKey:
  case StContArResume:
  case StContArState:
  case StContArValue:
  case StRetVal:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case CheckCold:
  case CheckInitProps:
  case CheckInitSProps:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContValid:
  case ConcatIntStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConcatStrInt:
  case ConcatStrStr:
  case CoerceStrToDbl:
  case CoerceStrToInt:
  case ConvStrToInt:
  case IncProfCounter:
  case IncStat:
  case IncStatGrouped:
  case CountBytecode:
  case ContPreNext:
  case ContStartedCheck:
  case ConvArrToBool:
  case ConvArrToDbl:
  case ConvArrToInt:
  case CoerceCellToBool:
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
  case DbgAssertRetAddr:
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
  case LdClsCtor:
  case LdAsyncArParentChain:
  case LdSSwitchDestFast:
  case LdSSwitchDestSlow:
  case RBTrace:
  case ConvCellToInt:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvCellToDbl:
  case ConvObjToDbl:
  case ConvStrToArr:   // decrefs src, but src is a string
  case ConvStrToBool:
  case ConvStrToDbl:
  case DeleteUnwinderException:
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
  case PrintBool:
  case PrintInt:
  case PrintStr:
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
  case AFWHPrepareChild:
  case CoerceCellToDbl:
  case CoerceCellToInt:
    return IrrelevantEffects {};

  // Some that touch memory we might care about later, but currently don't:
  case CheckStaticLocInit:
  case StaticLocInitCached:
  case ColAddElemC:
  case ColAddNewElemC:
  case ColIsEmpty:
  case ColIsNEmpty:
  case CheckBounds:
  case ConvCellToBool:
  case ConvObjToBool:
  case ConvObjToInt:
  case CountCollection:
  case LdVectorSize:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case VectorHasImmCopy:
  case CheckPackedArrayBounds:
  case LdColArray:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch most heap things.  They
  // also may generally write to the eval stack below an offset (see
  // alias-class.h above AStack for more).

  case DecRefThis:
  case DecRef:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  case DecRefStk:
    return MayLoadStore {
      AHeapAny | AStack { inst.src(0), inst.extra<DecRefStk>()->offset, 1 }
               | reentry_extra(),
      ANonFrame
    };

  case DecRefLoc:
    return MayLoadStore {
      AHeapAny | AFrame { inst.src(0), inst.extra<LocalId>()->locId }
               | reentry_extra(),
      ANonFrame
    };

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
     * so we just pretend it can read anything on the stack.
     */
    return MayLoadStore { ANonFrame | reentry_extra(), ANonFrame };

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
  case GetCtxFwdCallDyn: // autoload in StaticMethodCache
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
  case AKExists:    // re-enters for warnings on kVPackedKind, etc
  case AddElemIntKey:  // decrefs value
  case AddElemStrKey:  // decrefs value
  case AddNewElem:     // decrefs value
  case ArrayIdx:       // kVPackedKind warnings
  case ArrayGet:       // kVPackedKind warnings
  case ArrayIsset:     // kVPackedKind warnings
  case ArraySet:       // kVPackedKind warnings
  case ArraySetRef:    // kVPackedKind warnings
  case GetMemoKey:  // re-enters to call getInstanceKey() in some cases
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

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

bool check_effects(const IRInstruction& inst, MemEffects me) {
  auto check_fp = [&] (SSATmp* fp) {
    always_assert_flog(
      fp->type() <= Type::FramePtr,
      "Non frame pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(me)
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      // Maybe we should actually just give up on this (or check it /before/
      // doing canonicalize?).
      obj->type() <= Type::Obj,
      "Non obj pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(me)
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.frame()) check_fp(fr->fp);
    if (auto const pr = a.prop())  check_obj(pr->obj);
  };

  // In debug let's do some type checking in case people move instruction
  // argument numbers.
  match<void>(
    me,
    [&] (MayLoadStore x)    { check(x.loads); check(x.stores); },
    [&] (PureLoad x)        { check(x.src); },
    [&] (PureStore x)       { check(x.dst); },
    [&] (PureStoreNT x)     { check(x.dst); },
    [&] (PureSpillFrame x)  { check(x.dst); },
    [&] (IterEffects x)     { check_fp(x.fp); check(x.killed); },
    [&] (IterEffects2 x)    { check_fp(x.fp); check(x.killed); },
    [&] (KillFrameLocals x) { check_fp(x.fp); },
    [&] (ExitEffects x)     { check(x.live); check(x.kill); },
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    {},
    [&] (InterpOneEffects x){ check(x.killed); },
    [&] (CallEffects x)     { check(x.killed); check(x.stack); },
    [&] (ReturnEffects x)   { check(x.killed); }
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
    [&] (MayLoadStore x) -> R {
      return MayLoadStore { canonicalize(x.loads), canonicalize(x.stores) };
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
      return PureSpillFrame { canonicalize(x.dst) };
    },
    [&] (ExitEffects x) -> R {
      return ExitEffects { canonicalize(x.live), canonicalize(x.kill) };
    },
    [&] (CallEffects x) -> R {
      return CallEffects {
        x.destroys_locals,
        canonicalize(x.killed),
        canonicalize(x.stack)
      };
    },
    [&] (ReturnEffects x) -> R {
      return ReturnEffects { canonicalize(x.killed) };
    },
    [&] (IterEffects x) -> R {
      return IterEffects { x.fp, x.id, canonicalize(x.killed) };
    },
    [&] (IterEffects2 x) -> R {
      return IterEffects2 { x.fp, x.id1, x.id2, canonicalize(x.killed) };
    },
    [&] (InterpOneEffects x) -> R {
      return InterpOneEffects { canonicalize(x.killed) };
    },
    [&] (KillFrameLocals x)   -> R { return x; },
    [&] (IrrelevantEffects x) -> R { return x; },
    [&] (UnknownEffects x)    -> R { return x; }
  );
}

//////////////////////////////////////////////////////////////////////

std::string show(MemEffects effects) {
  using folly::sformat;
  return match<std::string>(
    effects,
    [&] (MayLoadStore x) {
      return sformat("mls({} ; {})", show(x.loads), show(x.stores));
    },
    [&] (ExitEffects x) {
      return sformat("exit({} ; {})", show(x.live), show(x.kill));
    },
    [&] (CallEffects x) {
      return sformat("call({} ; {})", show(x.killed), show(x.stack));
    },
    [&] (InterpOneEffects x) {
      return sformat("interp({})", show(x.killed));
    },
    [&] (PureLoad x)        { return sformat("ld({})", show(x.src)); },
    [&] (PureStore x)       { return sformat("st({})", show(x.dst)); },
    [&] (PureStoreNT x)     { return sformat("stNT({})", show(x.dst)); },
    [&] (PureSpillFrame x)  { return sformat("stFrame({})", show(x.dst)); },
    [&] (ReturnEffects x)   { return sformat("return({})", show(x.killed)); },
    [&] (IterEffects)       { return "IterEffects"; },
    [&] (IterEffects2)      { return "IterEffects2"; },
    [&] (KillFrameLocals)   { return "KillFrameLocals"; },
    [&] (IrrelevantEffects) { return "IrrelevantEffects"; },
    [&] (UnknownEffects)    { return "UnknownEffects"; }
  );
}

//////////////////////////////////////////////////////////////////////

}}
