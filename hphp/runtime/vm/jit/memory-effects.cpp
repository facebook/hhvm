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

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/simplify.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace {

MemEffects memory_effects_impl(const IRInstruction& inst) {
  switch (inst.op()) {
  case StLoc:
    return StoreLocal { inst.src(0), inst.extra<StLoc>()->locId };

  case StLocNT:
    return StoreLocalNT { inst.src(0), inst.extra<StLocNT>()->locId };

  /*
   * Before the FunctionReturnHook throws, it will set up the ActRec so the
   * unwinder knows everything is already released (i.e. it calls
   * ar->setLocalsDecRefd()).
   *
   * So it has no upward exposed uses, even though it has a catch block as a
   * successor that looks like it can use any locals (and in fact it can, if it
   * weren't for this instruction).
   */
  case FunctionReturnHook:
    return KillFrameLocals { inst.src(0) };

  /*
   * If we're returning from a function, none of the locals can be read
   * anymore.
   */
  case RetCtrl:
    if (inst.extra<RetCtrl>()->suspendingResumed) {
      return ReadAllLocals {};
    }
    assert(inst.src(1)->inst()->op() == FreeActRec);
    return KillFrameLocals { inst.src(1)->inst()->src(0) };

  case CheckLoc:
  case DecRefLoc:
  case GuardLoc:
  case LdGbl:
  case LdLoc:
    return ReadLocal { inst.src(0), inst.extra<LocalId>()->locId };

  /*
   * These call instructions potentially throw, even though we don't (yet) have
   * explicit catch traces for them, which means it counts as possibly reading
   * any local, on any frame.
   */
  case Call:
  case CallArray:
  case CallBuiltin:
  case ContEnter:
    return ReadAllLocals {};

  case GenericRetDecRefs:
    return ReadAllLocals {};

  case InlineReturn:
    return KillFrameLocals { inst.src(0) };

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
    return ReadAllLocals {};

  case EndCatch:
  case FunctionSuspendHook:
  case InterpOne:
  case InterpOneCF:
  case NativeImpl:
  case TryEndCatch:
    return UnknownEffects {};

  // NB: on the failure path, these C++ helpers do a fixup and read frame
  // locals before they throw.  Let's not worry about which ones because these
  // things only happen at the very start of a function.
  //
  // TODO(#5372569): if we combine dv inits into the same regions we could
  // possibly avoid storing KindOfUninits if we support this.
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamFail:
    return UnknownEffects {};
  // However the following ones are fine.
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
    return IrrelevantEffects {};

  // Resumable suspension takes everything from the frame and move it into the
  // heap.
  case CreateAFWH:
  case CreateCont:
    return ReadAllLocals {};

  // Tracelet exits that don't leave the function act as reads of all locals.
  case ReqBindJmp:
  case ReqBindJmpEq:
  case ReqBindJmpEqInt:
  case ReqBindJmpGt:
  case ReqBindJmpGte:
  case ReqBindJmpGteInt:
  case ReqBindJmpGtInt:
  case ReqBindJmpInstanceOfBitmask:
  case ReqBindJmpLt:
  case ReqBindJmpLte:
  case ReqBindJmpLteInt:
  case ReqBindJmpLtInt:
  case ReqBindJmpNeq:
  case ReqBindJmpNeqInt:
  case ReqBindJmpNInstanceOfBitmask:
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
  case SideExitJmpInstanceOfBitmask:
  case SideExitJmpLt:
  case SideExitJmpLtInt:
  case SideExitJmpLte:
  case SideExitJmpLteInt:
  case SideExitJmpNInstanceOfBitmask:
  case SideExitJmpNSame:
  case SideExitJmpNZero:
  case SideExitJmpNeq:
  case SideExitJmpNeqInt:
  case SideExitJmpSame:
  case SideExitJmpZero:
  case JmpSwitchDest:
  case JmpSSwitchDest:
    return ReadAllLocals {};

  case IterInit:
  case MIterInit:
  case WIterInit:
    return ReadLocal { inst.src(1), inst.extra<IterData>()->keyId };
  case IterNext:
  case MIterNext:
  case WIterNext:
    return ReadLocal { inst.src(0), inst.extra<IterData>()->keyId };

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    return ReadLocal2 {
      inst.src(1),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId
    };

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    return ReadLocal2 {
      inst.src(0),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId
    };

  case DbgAssertPtr:
    if (inst.src(0)->inst()->is(LdStackAddr)) {
      return IrrelevantEffects {};
    }
    return UnknownEffects {};
  case ProfileStr:
    {
      auto const sinst = inst.src(0)->inst();
      if (sinst->is(LdLocAddr)) {
        return ReadLocal { sinst->src(0), sinst->extra<LdLocAddr>()->locId };
      }
      if (sinst->is(LdStackAddr)) {
        return IrrelevantEffects {};
      }
      return UnknownEffects {};
    }

  case ABCUnblock:
  case AbsDbl:
  case AddDbl:
  case AddElemIntKey:
  case AddElemStrKey:
  case AddInt:
  case AddIntO:
  case AddNewElem:
  case AFWHBlockOn:
  case AFWHPrepareChild:
  case AKExists:
  case AllocObj:
  case AllocPackedArray:
  case AndInt:
  case ArrayAdd:
  case ArrayGet:
  case ArrayIdx:
  case ArrayIsset:
  case ArraySet:
  case ArraySetRef:
  case AssertLoc:
  case AssertNonNull:
  case AssertStk:
  case AssertType:
  case BaseG:
  case BeginCatch:
  case Box:
  case BoxPtr:
  case CastStk:
  case CastStkIntToDbl:
  case Ceil:
  case CheckBounds:
  case CheckCold:
  case CheckDefinedClsEq:
  case CheckInit:
  case CheckInitMem:
  case CheckInitProps:
  case CheckInitSProps:
  case CheckNonNull:
  case CheckNullptr:
  case CheckPackedArrayBounds:
  case CheckPackedArrayElemNull:
  case CheckRefs:
  case CheckStaticLocInit:
  case CheckStk:
  case CheckSurpriseFlags:
  case CheckType:
  case CIterFree:
  case Clone:
  case ClosureStaticLocInit:
  case ClsNeq:
  case CoerceCellToBool:
  case CoerceCellToDbl:
  case CoerceCellToInt:
  case CoerceStk:
  case CoerceStrToDbl:
  case CoerceStrToInt:
  case ColAddElemC:
  case ColAddNewElemC:
  case ColIsEmpty:
  case ColIsNEmpty:
  case ConcatCellCell:
  case ConcatIntStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConcatStrInt:
  case ConcatStrStr:
  case Conjure:
  case ConstructInstance:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContPreNext:
  case ContStartedCheck:
  case ContValid:
  case ConvArrToBool:
  case ConvArrToDbl:
  case ConvArrToInt:
  case ConvBoolToArr:
  case ConvBoolToDbl:
  case ConvBoolToInt:
  case ConvBoolToStr:
  case ConvCellToArr:
  case ConvCellToBool:
  case ConvCellToDbl:
  case ConvCellToInt:
  case ConvCellToObj:
  case ConvCellToStr:
  case ConvClsToCctx:
  case ConvDblToArr:
  case ConvDblToBool:
  case ConvDblToInt:
  case ConvDblToStr:
  case ConvIntToArr:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvIntToStr:
  case ConvObjToArr:
  case ConvObjToBool:
  case ConvObjToDbl:
  case ConvObjToInt:
  case ConvObjToStr:
  case ConvResToStr:
  case ConvStrToArr:
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvStrToInt:
  case Count:
  case CountArray:
  case CountArrayFast:
  case CountCollection:
  case CreateSSWH:
  case CufIterSpillFrame:
  case CustomInstanceInit:
  case DbgAssertRefCount:
  case DbgAssertRetAddr:
  case DbgAssertType:
  case DecodeCufIter:
  case DecRef:
  case DecRefMem:
  case DecRefNZ:
  case DecRefStack:
  case DecRefThis:
  case DefConst:
  case DefFP:
  case DefLabel:
  case DefMIStateBase:
  case DefSP:
  case DeleteUnwinderException:
  case DerefClsRDSHandle:
  case DivDbl:
  case EagerSyncVMRegs:
  case EndGuards:
  case Eq:
  case EqDbl:
  case EqInt:
  case EqX:
  case ExceptionBarrier:
  case ExtendsClass:
  case Floor:
  case FreeActRec:
  case GenericIdx:
  case GetCtxFwdCall:
  case GetCtxFwdCallDyn:
  case Gt:
  case GtDbl:
  case Gte:
  case GteDbl:
  case GteInt:
  case GteX:
  case GtInt:
  case GtX:
  case GuardRefs:
  case GuardStk:
  case Halt:
  case IncProfCounter:
  case IncRef:
  case IncRefCtx:
  case IncStat:
  case IncStatGrouped:
  case IncTransCounter:
  case InitObjProps:
  case InitPackedArray:
  case InitPackedArrayLoop:
  case InitProps:
  case InitSProps:
  case InstanceOf:
  case InstanceOfBitmask:
  case InstanceOfIface:
  case InterfaceSupportsArr:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case IsNType:
  case IsNTypeMem:
  case IsScalarType:
  case IsType:
  case IsTypeMem:
  case IsWaitHandle:
  case IterFree:
  case Jmp:
  case JmpEq:
  case JmpEqInt:
  case JmpGt:
  case JmpGte:
  case JmpGteInt:
  case JmpGtInt:
  case JmpInstanceOfBitmask:
  case JmpLt:
  case JmpLte:
  case JmpLteInt:
  case JmpLtInt:
  case JmpNeq:
  case JmpNeqInt:
  case JmpNInstanceOfBitmask:
  case JmpNSame:
  case JmpNZero:
  case JmpSame:
  case JmpZero:
  case LdAFWHActRec:
  case LdARFuncPtr:
  case LdArrFPushCuf:
  case LdArrFuncCtx:
  case LdAsyncArParentChain:
  case LdBindAddr:
  case LdCctx:
  case LdCls:
  case LdClsCached:
  case LdClsCachedSafe:
  case LdClsCctx:
  case LdClsCns:
  case LdClsCtor:
  case LdClsCtx:
  case LdClsInitData:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case LdClsName:
  case LdClsPropAddrKnown:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdCns:
  case LdContActRec:
  case LdContArKey:
  case LdContArRaw:
  case LdContArValue:
  case LdCtx:
  case LdElem:
  case LdFunc:
  case LdFuncCached:
  case LdFuncCachedSafe:
  case LdFuncCachedU:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdLocAddr:
  case LdMIStateAddr:
  case LdObjClass:
  case LdObjInvoke:
  case LdObjMethod:
  case LdPackedArrayElem:
  case LdPairBase:
  case LdProp:
  case LdPropAddr:
  case LdRaw:
  case LdRef:
  case LdResumableArObj:
  case LdRetAddr:
  case LdSSwitchDestFast:
  case LdSSwitchDestSlow:
  case LdStack:
  case LdStackAddr:
  case LdStaticLocCached:
  case LdStrFPushCuf:
  case LdSwitchDblIndex:
  case LdSwitchObjIndex:
  case LdSwitchStrIndex:
  case LdThis:
  case LdUnwinderValue:
  case LdVectorBase:
  case LdVectorSize:
  case LdWHResult:
  case LdWHState:
  case LookupClsCns:
  case LookupClsMethod:
  case LookupClsMethodCache:
  case LookupClsMethodFCache:
  case LookupClsRDSHandle:
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case Lt:
  case LtDbl:
  case Lte:
  case LteDbl:
  case LteInt:
  case LteX:
  case LtInt:
  case LtX:
  case MapGet:
  case MapIsset:
  case MapSet:
  case MIterFree:
  case Mod:
  case Mov:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case Neq:
  case NeqDbl:
  case NeqInt:
  case NeqX:
  case NewArray:
  case NewCol:
  case NewInstanceRaw:
  case NewLikeArray:
  case NewMIArray:
  case NewMixedArray:
  case NewMSArray:
  case NewPackedArray:
  case NewStructArray:
  case NewVArray:
  case NInstanceOfBitmask:
  case Nop:
  case NSame:
  case OODeclExists:
  case OrInt:
  case PairIsset:
  case PrintBool:
  case PrintInt:
  case PrintStr:
  case ProfileArray:
  case RaiseArrayIndexNotice:
  case RaiseError:
  case RaiseNotice:
  case RaiseUndefProp:
  case RaiseUninitLoc:
  case RaiseWarning:
  case RBTrace:
  case ReDefSP:
  case RegisterLiveObj:
  case ReleaseVVOrExit:
  case RestoreErrorLevel:
  case RetAdjustStack:
  case Same:
  case Shl:
  case Shr:
  case SpillFrame:
  case SpillStack:
  case Sqrt:
  case StAsyncArRaw:
  case StAsyncArResult:
  case StaticLocInitCached:
  case StClosureArg:
  case StClosureCtx:
  case StClosureFunc:
  case StContArKey:
  case StContArRaw:
  case StContArValue:
  case StElem:
  case StGbl:
  case StMem:
  case StProp:
  case StRaw:
  case StRef:
  case StRetVal:
  case StringGet:
  case StringIsset:
  case SubDbl:
  case SubInt:
  case SubIntO:
  case SurpriseHook:
  case SyncABIRegs:
  case TakeRef:
  case TakeStack:
  case ThrowNonObjProp:
  case TrackLoc:
  case TypeProfileFunc:
  case UnwindCheckSideExit:
  case VectorDoCow:
  case VectorHasImmCopy:
  case VectorIsset:
  case WarnNonObjProp:
  case XorBool:
  case XorInt:
  case ZeroErrorLevel:
    return IrrelevantEffects {};

  /*
   * Some instructions can read locals only if they have an initial argument
   * that points at a local address.  There are ir.spec rules ensuring that the
   * only case this is allowed to point to a local is when the source comes
   * canonically from a LdLocAddr.
   */
  case CheckTypeMem:
  case LdMem:
  case CGetProp:
  case UnboxPtr:
  case PropX:
  case UnsetProp:
  case CGetElem:
  case ElemArray:
  case ElemArrayW:
  case ElemX:
  case EmptyElem:
  case EmptyProp:
  case IssetElem:
  case IssetProp:
  case BindElem:              case BindElemStk:
  case BindNewElem:           case BindNewElemStk:
  case BindProp:              case BindPropStk:
  case ElemDX:                case ElemDXStk:
  case ElemUX:                case ElemUXStk:
  case IncDecElem:            case IncDecElemStk:
  case IncDecProp:            case IncDecPropStk:
  case PropDX:                case PropDXStk:
  case SetElem:               case SetElemStk:
  case SetNewElem:            case SetNewElemStk:
  case SetNewElemArray:       case SetNewElemArrayStk:
  case SetOpElem:             case SetOpElemStk:
  case SetOpProp:             case SetOpPropStk:
  case SetProp:               case SetPropStk:
  case SetWithRefElem:        case SetWithRefElemStk:
  case SetWithRefNewElem:     case SetWithRefNewElemStk:
  case UnsetElem:             case UnsetElemStk:
  case VGetElem:              case VGetElemStk:
  case VGetProp:              case VGetPropStk:
    {
      auto const sinst = canonical(inst.src(0))->inst();
      if (sinst->is(LdLocAddr)) {
        return ReadLocal { sinst->src(0), sinst->extra<LdLocAddr>()->locId };
      }
      return IrrelevantEffects {};
    }
  }

  not_reached();
}

}

//////////////////////////////////////////////////////////////////////

MemEffects memory_effects(const IRInstruction& inst) {
  auto const ret = memory_effects_impl(inst);
  if (debug) {
    // In debug let's do some type checking in case people move instruction
    // argument numbers.
    auto const fp = match<SSATmp*>(
      ret,
      [&] (UnknownEffects)    { return nullptr; },
      [&] (IrrelevantEffects) { return nullptr; },
      [&] (ReadAllLocals)     { return nullptr; },
      [&] (KillFrameLocals l) { return l.fp; },
      [&] (ReadLocal l)       { return l.fp; },
      [&] (ReadLocal2 l)      { return l.fp; },
      [&] (StoreLocal l)      { return l.fp; },
      [&] (StoreLocalNT l)    { return l.fp; }
    );
    if (fp != nullptr) {
      always_assert_flog(
        fp->type() <= Type::FramePtr,
        "Non frame pointer in memory effects:\n  inst: {}\n  effects: {}",
        inst.toString(),
        show(ret)
      );
    }
  }
  return ret;
}

const char* show(MemEffects effects) {
  return match<const char*>(
    effects,
    [&] (UnknownEffects)    { return "UnknownEffects"; },
    [&] (IrrelevantEffects) { return "IrrelevantEffects"; },
    [&] (KillFrameLocals)   { return "KillFrameLocals"; },
    [&] (ReadLocal)         { return "ReadLocal"; },
    [&] (ReadLocal2)        { return "ReadLocal2"; },
    [&] (StoreLocal)        { return "StoreLocal"; },
    [&] (StoreLocalNT)      { return "StoreLocalNT"; },
    [&] (ReadAllLocals)     { return "ReadAllLocals"; }
  );
}

//////////////////////////////////////////////////////////////////////

}}
