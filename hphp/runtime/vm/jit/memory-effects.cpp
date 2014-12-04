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

  if (ptr->type() <= Type::PtrToPropGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdPropAddr)) {
      return AProp {
        sinst->src(0),
        safe_cast<uint32_t>(sinst->src(1)->intVal())
      };
    }
    return APropAny;
  }

  // We have various other situations here that we don't track in this module
  // yet, but we can possibly exclude the locations we care about so far.
  if (!ptr->type().maybe(Type::PtrToFrameGen)) return ANonFrame;

  // We don't have any idea what it might refer to for other things, yet.
  // Return the set of all locations.
  return AUnknown;
}

bool call_destroys_locals(const IRInstruction& inst) {
  switch (inst.op()) {
  case Call:         return inst.extra<Call>()->destroyLocals;
  case CallArray:    return inst.extra<CallArray>()->destroyLocals;
  case CallBuiltin:  return inst.extra<CallBuiltin>()->destroyLocals;
  case ContEnter:    return false;
  default:           break;
  }
  always_assert(0);
}

MemEffects memory_effects_impl(const IRInstruction& inst) {
  switch (inst.op()) {

  //////////////////////////////////////////////////////////////////////
  // Region exits

  // These exits don't leave the current php function, and could head to code
  // that could read or write anything as far as we know (including frame
  // locals).
  case ReqBindJmp:
  case ReqBindJmpEq:
  case ReqBindJmpEqInt:
  case ReqBindJmpGt:
  case ReqBindJmpGte:
  case ReqBindJmpGteInt:
  case ReqBindJmpGtInt:
  case ReqBindJmpLt:
  case ReqBindJmpLte:
  case ReqBindJmpLteInt:
  case ReqBindJmpLtInt:
  case ReqBindJmpNeq:
  case ReqBindJmpNeqInt:
  case ReqBindJmpNSame:
  case ReqBindJmpNZero:
  case ReqBindJmpSame:
  case ReqBindJmpZero:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case SideExitGuardLoc:
  case SideExitGuardStk:
  case SideExitJmpEq:
  case SideExitJmpEqInt:
  case SideExitJmpGt:
  case SideExitJmpGtInt:
  case SideExitJmpGte:
  case SideExitJmpGteInt:
  case SideExitJmpLt:
  case SideExitJmpLtInt:
  case SideExitJmpLte:
  case SideExitJmpLteInt:
  case SideExitJmpNSame:
  case SideExitJmpNZero:
  case SideExitJmpNeq:
  case SideExitJmpNeqInt:
  case SideExitJmpSame:
  case SideExitJmpZero:
  case JmpSwitchDest:
  case JmpSSwitchDest:
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
    return MayLoadStore { AUnknown, ANonFrame };

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
    return ReturnEffects {};

  case GenericRetDecRefs:
    return MayLoadStore { AUnknown, ANonFrame };

  /*
   * This has to act as a use of the locals for the outer frame, but really
   * only because of how we deal with FramePtrs in the JIT.
   *
   * Since the frame is going to get allocated rbp, we can't push local stores
   * into inlined callees, since their FramePtr isn't available anymore.  We'd
   * be willing to do so without this, since there's no other constraint saying
   * the caller's locals have to be in memory at this point.
   */
  case DefInlineFP:
    /*
     * TODO(#3634984): this is an unfortunate pessimization---we can still
     * eliminate those stores if nothing reads them, we just can't move them
     * past this point.  Reserved registers must die.
     */
    return MayLoadStore { AFrameAny, AEmpty };

  case InlineReturn:
    return KillFrameLocals { inst.src(0) };

  case InterpOne:
  case InterpOneCF:
    return InterpOneEffects {};

  case EndCatch:
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
    return MayLoadStore { ANonFrame, ANonFrame };

  case Call:
  case CallArray:
  case CallBuiltin:
  case ContEnter:
    return CallEffects { call_destroys_locals(inst) };

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateAFWH:
  case CreateCont:
    return MayLoadStore { AFrameAny, ANonFrame };

  // This calls extension-defined instance constructors.  They can read or
  // write whatever except frame locals.
  case ConstructInstance:
    return MayLoadStore { ANonFrame, ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case MIterInit:
  case WIterInit:
    return IterEffects { inst.src(1), inst.extra<IterData>()->valId };
  case IterNext:
  case MIterNext:
  case WIterNext:
    return IterEffects { inst.src(0), inst.extra<IterData>()->valId };

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    return IterEffects2 {
      inst.src(1),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId
    };

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    return IterEffects2 {
      inst.src(0),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId
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
  case DecRefLoc:
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

  case LdElem:
  case LdMem:
    return PureLoad { pointee(inst.src(0)) };

  case StElem:
  case StMem:
    return PureStore { pointee(inst.src(0)), inst.src(2) };

  case BoxPtr:
  case UnboxPtr:
    {
      auto const mem = pointee(inst.src(0));
      return MayLoadStore { mem, mem };
    }

  case CheckInitMem:
  case CheckTypeMem:
  case DbgAssertPtr:
  case IsNTypeMem:
  case IsTypeMem:
  case ProfileStr:
    return MayLoadStore { pointee(inst.src(0)), AEmpty };

  case DecRefMem:
    return MayLoadStore {
      // DecRefMem can re-enter to run a destructor and read any non-frame
      // locals.  We also need to union in the pointee in case it points to a
      // local.
      pointee(inst.src(0)) | ANonFrame,
      ANonFrame
    };

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case StProp:
    return PureStore {
      AProp { inst.src(0), safe_cast<uint32_t>(inst.src(1)->intVal()) },
      inst.src(2)
    };

  case CheckRefInner:
    // We don't have AliasClass support for refs yet.
    return MayLoadStore { ANonFrame, AEmpty };
  case LdRef:
    return PureLoad { ANonFrame };

  case StRef:
    // We don't have anything for ref locations at this point, but we know it
    // can't be a frame location.
    return PureStore { ANonFrame };

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
    return MayLoadStore { AEmpty, AElemIAny };

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
  case BindElem:              case BindElemStk:
  case BindNewElem:           case BindNewElemStk:
  case ElemDX:                case ElemDXStk:
  case ElemUX:                case ElemUXStk:
  case IncDecElem:            case IncDecElemStk:
  case SetElem:               case SetElemStk:
  case SetNewElemArray:       case SetNewElemArrayStk:
  case SetNewElem:            case SetNewElemStk:
  case SetOpElem:             case SetOpElemStk:
  case SetWithRefElem:        case SetWithRefElemStk:
  case SetWithRefNewElem:     case SetWithRefNewElemStk:
  case UnsetElem:             case UnsetElemStk:
  case VGetElem:              case VGetElemStk:
    // Right now we generally can't limit any of these, since they can raise
    // warnings and re-enter.
    assert(inst.src(0)->type() <= Type::PtrToGen);
    return MayLoadStore {
      pointee(inst.src(0)) | ANonFrame,
      pointee(inst.src(0)) | ANonFrame
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
  case BindProp:              case BindPropStk:
  case IncDecProp:            case IncDecPropStk:
  case PropDX:                case PropDXStk:
  case SetOpProp:             case SetOpPropStk:
  case SetProp:               case SetPropStk:
  case VGetProp:              case VGetPropStk:
    if (inst.src(0)->type() <= Type::PtrToGen) {
      return MayLoadStore {
        pointee(inst.src(0)) | ANonFrame,
        pointee(inst.src(0)) | ANonFrame
      };
    }
    return MayLoadStore { ANonFrame, ANonFrame };

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
    return MayLoadStore { ANonFrame, AEmpty /* Note */ };

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
  case NewStructArray:
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

  case AllocObj:  // AllocObj calls constructors.
    return MayLoadStore { ANonFrame, ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // Instructions that only affect the stack.  Currently memory-effects doesn't
  // report anything about this.

  case SpillFrame:
  case SpillStack:
  case AssertStk:
  case HintStkInner:
  case GuardStk:
  case CheckStk:
  case CastStkIntToDbl:
  case CufIterSpillFrame:
  case DecRefStack:
  case LdStack:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that never do anything to memory

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
  case LdStackAddr:
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
  case JmpNeqInt:
  case JmpGteInt:
  case JmpGtInt:
  case JmpLteInt:
  case JmpLtInt:
  case JmpEqInt:
  case ExceptionBarrier:
  case SyncABIRegs:
  case DecRefNZ:
  case CheckInit:
  case Nop:
  case ClsNeq:
  case Mod:
  case TakeRef:
  case TakeStack:
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

  case LdClsCctx:
  case ABCUnblock:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case FreeActRec:
  case IncRef:
  case IncRefCtx:
  case LdRetAddr:
  case RegisterLiveObj:
  case RetAdjustStack:
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
  case AFWHBlockOn:
  case CheckCold:
  case CheckInitProps:
  case CheckInitSProps:
  case CheckRefs:
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
  case JmpNSame:
  case JmpSame:
  case Gt:
  case Gte:
  case Eq:
  case Lt:
  case Lte:
  case Neq:
  case IncTransCounter:
  case LdBindAddr:
  case JmpEq:
  case JmpGt:
  case JmpGte:
  case JmpLt:
  case JmpLte:
  case JmpNeq:
  case LdClsCtor:
  case LdAsyncArParentChain:
  case GuardRefs:
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
  case LdARFuncPtr:
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
  case ProfileArray:
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
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch anything except frame
  // memory.

  case CastStk:
  case BaseG:
  case DecRef:
  case DecRefThis:
  case Clone:
  case WarnNonObjProp:
  case RaiseArrayIndexNotice:
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
  case LdArrFPushCuf:  // autoload
  case LdArrFuncCtx:   // autoload
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFunc:         // autoload
  case LdFuncCached:   // autoload
  case LdFuncCachedU:  // autoload
  case LdObjMethod:    // can't autoload, but can decref $this right now
  case LdStrFPushCuf:  // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case LookupClsCns:      // autoload
  case LookupClsMethod:   // autoload
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case StringGet:   // raise_warning
  case CoerceStk:   // object __toString
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
    return MayLoadStore { ANonFrame, ANonFrame };

  //////////////////////////////////////////////////////////////////////

  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

MemEffects memory_effects(const IRInstruction& inst) {
  always_assert_flog(
    !RuntimeOption::EnableArgsInBacktraces,
    "memory_effects currently doesn't report some possible "
    "loads for EnableArgsInBacktraces, so you must not call it from "
    "passes that run if that flag is on"
  );

  auto const ret = memory_effects_impl(inst);
  if (!debug) return ret;

  auto check_fp = [&] (SSATmp* fp) {
    always_assert_flog(
      fp->type() <= Type::FramePtr,
      "Non frame pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(ret)
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      // Maybe we should actually just give up on this (or check it /before/
      // doing canonicalize?).
      obj->type() <= Type::Obj,
      "Non obj pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(ret)
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.frame()) {
      check_fp(fr->fp);
      return;
    }
    if (auto const pr = a.prop()) {
      check_obj(pr->obj);
    }
  };

  // In debug let's do some type checking in case people move instruction
  // argument numbers.
  match<void>(
    ret,
    [&] (MayLoadStore m)    { check(m.loads); check(m.stores); },
    [&] (PureLoad m)        { check(m.src); },
    [&] (PureStore m)       { check(m.dst); },
    [&] (PureStoreNT m)     { check(m.dst); },
    [&] (IterEffects m)     { check_fp(m.fp); },
    [&] (IterEffects2 m)    { check_fp(m.fp); },
    [&] (KillFrameLocals m) { check_fp(m.fp); },
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    {},
    [&] (InterpOneEffects)  {},
    [&] (CallEffects)       {},
    [&] (ReturnEffects)     {}
  );

  return ret;
}

//////////////////////////////////////////////////////////////////////

MemEffects canonicalize(MemEffects me) {
  using R = MemEffects;
  return match<R>(
    me,
    [&] (MayLoadStore l) -> R {
      return MayLoadStore { canonicalize(l.loads), canonicalize(l.stores) };
    },
    [&] (PureLoad l) -> R {
      return PureLoad { canonicalize(l.src) };
    },
    [&] (PureStore l) -> R {
      return PureStore { canonicalize(l.dst), l.value };
    },
    [&] (PureStoreNT l) -> R {
      return PureStoreNT { canonicalize(l.dst), l.value };
    },
    [&] (KillFrameLocals l)   -> R { return l; },
    [&] (IterEffects l)       -> R { return l; },
    [&] (IterEffects2 l)      -> R { return l; },
    [&] (CallEffects l)       -> R { return l; },
    [&] (ReturnEffects l)     -> R { return l; },
    [&] (InterpOneEffects l)  -> R { return l; },
    [&] (IrrelevantEffects l) -> R { return l; },
    [&] (UnknownEffects l)    -> R { return l; }
  );
}

//////////////////////////////////////////////////////////////////////

std::string show(MemEffects effects) {
  using folly::sformat;
  return match<std::string>(
    effects,
    [&] (MayLoadStore m) {
      return sformat("mls({} ; {})", show(m.loads), show(m.stores));
    },
    [&] (PureLoad m)        { return sformat("ld({})", show(m.src)); },
    [&] (PureStore m)       { return sformat("st({})", show(m.dst)); },
    [&] (PureStoreNT m)     { return sformat("stNT({})", show(m.dst)); },
    [&] (IterEffects)       { return "IterEffects"; },
    [&] (IterEffects2)      { return "IterEffects2"; },
    [&] (KillFrameLocals)   { return "KillFrameLocals"; },
    [&] (IrrelevantEffects) { return "IrrelevantEffects"; },
    [&] (UnknownEffects)    { return "UnknownEffects"; },
    [&] (InterpOneEffects)  { return "InterpOneEffects"; },
    [&] (CallEffects)       { return "CallEffects"; },
    [&] (ReturnEffects)     { return "ReturnEffects"; }
  );
}

//////////////////////////////////////////////////////////////////////

}}
