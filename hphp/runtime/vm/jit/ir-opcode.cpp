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

#include "hphp/runtime/vm/jit/ir-opcode.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

#define NF     0
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define T      Terminal
#define B      Branch
#define P      Passthrough
#define MProp  MInstrProp
#define MElem  MInstrElem
#define LA     LayoutAgnostic
#define LP     (LayoutPreserving|LayoutAgnostic)

#define ND             0
#define D(n)           HasDest
#define DofS(n)        HasDest
#define DRefineS(n)    HasDest
#define DParamMayRelax(t) HasDest
#define DParam(t)      HasDest
#define DLdObjCls      HasDest
#define DAllocObj      HasDest
#define DVecElem       HasDest
#define DDictElem      HasDest
#define DDictSet       HasDest
#define DVecSet        HasDest
#define DKeysetElem    HasDest
#define DVecFirstElem     HasDest
#define DVecLastElem      HasDest
#define DVecKey           HasDest
#define DDictFirstElem    HasDest
#define DDictLastElem     HasDest
#define DDictFirstKey     HasDest
#define DDictLastKey      HasDest
#define DKeysetFirstElem  HasDest
#define DKeysetLastElem   HasDest
#define DLoggingArrLike   HasDest
#define DVArr          HasDest
#define DDArr          HasDest
#define DStaticDArr    HasDest
#define DCol           HasDest
#define DMulti         NaryDest
#define DSetElem       HasDest
#define DPtrToParam    HasDest
#define DBuiltin       HasDest
#define DCall          HasDest
#define DGenIter       HasDest
#define DSubtract(n,t) HasDest
#define DCns           HasDest
#define DUnion(...)    HasDest
#define DMemoKey       HasDest
#define DLvalOfPtr     HasDest
#define DPtrIter       HasDest
#define DPtrIterVal    HasDest

namespace {
template<Opcode op, uint64_t flags>
struct op_flags {
  static constexpr uint64_t value =
    (OpHasExtraData<op>::value ? HasExtra : 0) | flags;

  static_assert(!(value & ProducesRC) ||
                (value & (HasDest | NaryDest)) == HasDest,
                "ProducesRC instructions must have exactly one dest");
};
}

OpInfo g_opInfo[] = {
#define O(name, dsts, srcs, flags)                    \
    { #name,                                          \
       op_flags<name, dsts | flags>::value            \
    },
  IR_OPCODES
#undef O
  { 0 }
};

#undef NF
#undef C
#undef E
#undef PRc
#undef CRc
#undef Er
#undef T
#undef B
#undef P
#undef K
#undef MProp
#undef MElem

#undef ND
#undef D
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DLdObjCls
#undef DVecElem
#undef DDictElem
#undef DDictSet
#undef DVecSet
#undef DKeysetElem
#undef DVecFirstElem
#undef DVecLastElem
#undef DVecKey
#undef DDictFirstElem
#undef DDictLastElem
#undef DDictFirstKey
#undef DDictLastKey
#undef DKeysetFirstElem
#undef DKeysetLastElem
#undef DLoggingArrLike
#undef DVArr
#undef DDArr
#undef DStaticDArr
#undef DCol
#undef DAllocObj
#undef DMulti
#undef DSetElem
#undef DPtrToParam
#undef DBuiltin
#undef DCall
#undef DGenIter
#undef DSubtract
#undef DCns
#undef DUnion
#undef DMemoKey
#undef DLvalOfPtr

///////////////////////////////////////////////////////////////////////////////

const StringData* findClassName(SSATmp* cls) {
  assertx(cls->isA(TCls));

  if (cls->hasConstVal()) {
    return cls->clsVal()->preClass()->name();
  }
  // Try to get the class name from a LdCls
  IRInstruction* clsInst = cls->inst();
  if (clsInst->op() == LdCls || clsInst->op() == LdClsCached) {
    SSATmp* clsName = clsInst->src(0);
    assertx(clsName->isA(TStr));
    if (clsName->hasConstVal()) {
      return clsName->strVal();
    }
  }
  return nullptr;
}

bool isCallOp(Opcode opc) {
  // CallBuiltin doesn't count because it is not a php-level call.  (It will
  // call a C++ helper and we can push/pop around it normally.)
  switch (opc) {
  case Call:
  case ContEnter:
    return true;
  default:
    return false;
  }
}

bool isGuardOp(Opcode opc) {
  switch (opc) {
    case CheckLoc:
    case CheckStk:
    case CheckType:
    case CheckMBase:
      return true;

    default:
      return false;
  }
}

folly::Optional<Opcode> negateCmpOp(Opcode opc) {
  switch (opc) {
    case GtBool:              return LteBool;
    case GteBool:             return LtBool;
    case LtBool:              return GteBool;
    case LteBool:             return GtBool;
    case EqBool:              return NeqBool;
    case NeqBool:             return EqBool;

    case GtInt:               return LteInt;
    case GteInt:              return LtInt;
    case LtInt:               return GteInt;
    case LteInt:              return GtInt;
    case EqInt:               return NeqInt;
    case NeqInt:              return EqInt;

    // Due to NaN only equality comparisons with doubles can be negated.
    case EqDbl:               return NeqDbl;
    case NeqDbl:              return EqDbl;

    case GtStr:               return LteStr;
    case GteStr:              return LtStr;
    case LtStr:               return GteStr;
    case LteStr:              return GtStr;
    case EqStr:               return NeqStr;
    case NeqStr:              return EqStr;
    case SameStr:             return NSameStr;
    case NSameStr:            return SameStr;

    case GtStrInt:            return LteStrInt;
    case GteStrInt:           return LtStrInt;
    case LtStrInt:            return GteStrInt;
    case LteStrInt:           return GtStrInt;
    case EqStrInt:            return NeqStrInt;
    case NeqStrInt:           return EqStrInt;

    // Objects can contain a property with NaN, so only equality comparisons can
    // be negated.
    case EqObj:               return NeqObj;
    case NeqObj:              return EqObj;
    case SameObj:             return NSameObj;
    case NSameObj:            return SameObj;

    // Arrays/vec/dicts can contain an element with NaN, so only equality
    // comparisons can be negated.
    case EqArrLike:           return NeqArrLike;
    case NeqArrLike:          return EqArrLike;
    case SameArrLike:         return NSameArrLike;
    case NSameArrLike:        return SameArrLike;

    case GtRes:               return LteRes;
    case GteRes:              return LtRes;
    case LtRes:               return GteRes;
    case LteRes:              return GtRes;
    case EqRes:               return NeqRes;
    case NeqRes:              return EqRes;

    default:                  return folly::none;
  }
}

bool opcodeMayRaise(Opcode opc) {
  switch (opc) {
  case NSameArrLike:
  case SameArrLike:
    return RuntimeOption::EvalHackArrCompatCheckCompare;

  case IsTypeStruct:
    return RuntimeOption::EvalHackArrIsShapeTupleNotices ||
           RuntimeOption::EvalIsExprEnableUnresolvedWarning ||
           RuntimeOption::EvalIsVecNotices;

  case AddNewElemKeyset:
  case AFWHPrepareChild:
  case AKExistsObj:
  case AllocObj:
  case AllocObjReified:
  case BaseG:
  case Call:
  case CallBuiltin:
  case CGetElem:
  case CGetProp:
  case CGetPropQ:
  case CheckClsMethFunc:
  case CheckClsReifiedGenericMismatch:
  case CheckFunReifiedGenericMismatch:
  case CheckStackOverflow:
  case CheckSurpriseAndStack:
  case CheckSurpriseFlagsEnter:
  case Clone:
  case CmpArrLike:
  case CmpObj:
  case ConcatIntStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConcatStrInt:
  case ConcatStrStr:
  case ConstructInstance:
  case ContEnter:
  case ConvArrLikeToDict:
  case ConvArrLikeToKeyset:
  case ConvArrLikeToVec:
  case ConvTVToBool:
  case ConvTVToDbl:
  case ConvTVToInt:
  case ConvTVToStr:
  case ConvClsMethToDArr:
  case ConvClsMethToDict:
  case ConvClsMethToKeyset:
  case ConvClsMethToVArr:
  case ConvClsMethToVec:
  case ConvObjToBool:
  case ConvObjToDArr:
  case ConvObjToDbl:
  case ConvObjToDict:
  case ConvObjToInt:
  case ConvObjToKeyset:
  case ConvObjToStr:
  case ConvObjToVArr:
  case ConvObjToVec:
  case ConvResToStr:
  case Count:
  case CreateAAWH:
  case DictGet:
  case DictSet:
  case ElemDictD:
  case ElemDictU:
  case ElemDX:
  case ElemKeysetU:
  case ElemUX:
  case ElemVecD:
  case ElemVecU:
  case ElemX:
  case EqArrLike:
  case EqObj:
  case GetMemoKey:
  case GtArrLike:
  case GteArrLike:
  case GteObj:
  case GtObj:
  case HandleRequestSurprise:
  case IncDecElem:
  case IncDecProp:
  case InitClsCns:
  case InitProps:
  case InitSProps:
  case InterpOne:
  case IssetElem:
  case IssetProp:
  case IterInit:
  case IterInitK:
  case IterNext:
  case IterNextK:
  case KeysetGet:
  case LdCls:
  case LdClsCached:
  case LdClsCtor:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdClsTypeCns:
  case LdClsTypeCnsClsName:
  case LdFunc:
  case LdFuncCached:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdRecDescCached:
  case LdObjMethodD:
  case LdObjMethodS:
  case LdSSwitchDestSlow:
  case LdSwitchObjIndex:
  case LookupClsMethod:
  case LookupClsMethodCache:
  case LookupClsMethodFCache:
  case LookupCnsE:
  case LookupFuncCached:
  case LtArrLike:
  case LteArrLike:
  case LteObj:
  case LtObj:
  case MapGet:
  case MapSet:
  case NativeImpl:
  case NeqArrLike:
  case NeqObj:
  case NewKeysetArray:
  case NewRecord:
  case OODeclExists:
  case OrdStrIdx:
  case OutlineSetOp:
  case PrintBool:
  case PrintInt:
  case PrintStr:
  case PropDX:
  case PropQ:
  case PropTypeRedefineCheck:
  case PropX:
  case RaiseArraySerializeNotice:
  case RaiseClsMethPropConvertNotice:
  case RaiseError:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseHackArrCompatNotice:
  case RaiseNotice:
  case RaiseRxCallViolation:
  case RaiseStrToClassNotice:
  case RaiseTooManyArg:
  case RaiseUndefProp:
  case RaiseUninitLoc:
  case RaiseWarning:
  case RecordReifiedGenericsAndGetTSList:
  case ResolveTypeStruct:
  case ReturnHook:
  case SetElem:
  case SetNewElem:
  case SetNewElemDict:
  case SetNewElemKeyset:
  case SetNewElemVec:
  case SetOpTV:
  case SetOpElem:
  case SetOpProp:
  case SetProp:
  case SetRange:
  case SetRangeRev:
  case StringGet:
  case SuspendHookAwaitEF:
  case SuspendHookAwaitEG:
  case SuspendHookAwaitR:
  case SuspendHookCreateCont:
  case SuspendHookYield:
  case ThrowAsTypeStructException:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case ThrowCallReifiedFunctionWithoutGenerics:
  case ThrowDivisionByZeroException:
  case ThrowHasThisNeedStatic:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowLateInitPropError:
  case ThrowMissingArg:
  case ThrowMissingThis:
  case ThrowOutOfBounds:
  case ThrowParameterWrongType:
  case ThrowParamInOutMismatch:
  case ThrowParamInOutMismatchRange:
  case UnsetElem:
  case UnsetProp:
  case VecSet:
  case VectorSet:
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamFail:
  case VerifyParamFailHard:
  case VerifyProp:
  case VerifyPropAll:
  case VerifyPropCls:
  case VerifyPropCoerce:
  case VerifyPropCoerceAll:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case VerifyReifiedLocalType:
  case VerifyReifiedReturnType:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
  case VerifyRetFailHard:
  case VerifyParamRecDesc:
  case VerifyRetRecDesc:
  case VerifyPropRecDesc:
    return true;

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AddNewElemVec:
  case AdvanceDictPtrIter:
  case AdvanceVecPtrIter:
  case AFWHBlockOn:
  case AFWHPushTailFrame:
  case AKExistsDict:
  case AKExistsKeyset:
  case AllocStructDArray:
  case AllocStructDict:
  case AllocVArray:
  case AllocVec:
  case AndInt:
  case AssertLoc:
  case AssertMBase:
  case AssertNonNull:
  case AssertStk:
  case AssertType:
  case AsyncFuncRet:
  case AsyncFuncRetSlow:
  case AsyncSwitchFast:
  case BeginCatch:
  case BeginInlining:
  case Ceil:
  case CheckArrayCOW:
  case CheckCold:
  case CheckDictOffset:
  case CheckImplicitContextNull:
  case CheckInit:
  case CheckInitMem:
  case CheckIter:
  case CheckKeysetOffset:
  case CheckLoc:
  case CheckMBase:
  case CheckDictKeys:
  case CheckMixedArrayOffset:
  case CheckMissingKeyInArrLike:
  case CheckNonNull:
  case CheckNullptr:
  case CheckVecBounds:
  case CheckRange:
  case CheckRDSInitialized:
  case CheckInOuts:
  case CheckSmashableClass:
  case CheckStk:
  case CheckSubClsCns:
  case CheckSurpriseFlags:
  case CheckType:
  case CheckTypeMem:
  case ChrInt:
  case CmpBool:
  case CmpDbl:
  case CmpInt:
  case CmpRes:
  case CmpStr:
  case CmpStrInt:
  case ColIsEmpty:
  case ColIsNEmpty:
  case Conjure:
  case ConjureUse:
  case ConstructClosure:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContPreNext:
  case ContStarted:
  case ContStartedCheck:
  case ContValid:
  case ConvArrLikeToDArr:
  case ConvArrLikeToVArr:
  case ConvBoolToDbl:
  case ConvBoolToInt:
  case ConvDblToBool:
  case ConvDblToInt:
  case ConvDblToStr:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvIntToStr:
  case ConvResToDbl:
  case ConvResToInt:
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvStrToInt:
  case ConvPtrToLval:
  case CountCollection:
  case CountDict:
  case CountKeyset:
  case CountVec:
  case CountWHNotDone:
  case CreateAFWH:
  case CreateAGen:
  case CreateAGWH:
  case CreateGen:
  case CreateSSWH:
  case DbgAssertFunc:
  case DbgAssertRefCount:
  case DbgCheckLocalsDecRefd:
  case DbgTraceCall:
  case DbgTrashFrame:
  case DbgTrashMem:
  case DbgTrashRetVal:
  case DbgTrashStk:
  case DblAsBits:
  case DebugBacktrace:
  case DebugBacktraceFast:
  case DecRef:
  case DecRefNZ:
  case DefCallFlags:
  case DefCallFunc:
  case DefCallNumArgs:
  case DefCallCtx:
  case DefConst:
  case DefFP:
  case DefFuncEntryFP:
  case DefLabel:
  case DefFrameRelSP:
  case DefRegSP:
  case DictFirst:
  case DictFirstKey:
  case DictGetK:
  case DictGetQuiet:
  case DictIdx:
  case DictIsset:
  case DictLast:
  case DictLastKey:
  case DivDbl:
  case DivInt:
  case EagerSyncVMRegs:
  case ElemDictK:
  case ElemKeysetK:
  case ElemMixedArrayK:
  case EndBlock:
  case EndCatch:
  case EndGuards:
  case EndInlining:
  case EnterPrologue:
  case EnterTCUnwind:
  case EqArrayDataPtr:
  case EqBool:
  case EqCls:
  case EqRecDesc:
  case EqDbl:
  case EqFunc:
  case EqInt:
  case EqPtrIter:
  case EqRes:
  case EqStr:
  case EqStrInt:
  case EqStrPtr:
  case ExtendsClass:
  case FinishMemberOp:
  case Floor:
  case FuncCred:
  case FuncHasAttr:
  case GenericRetDecRefs:
  case GetMemoKeyScalar:
  case GetDictPtrIter:
  case GetVecPtrIter:
  case GetTime:
  case GetTimeNs:
  case GtBool:
  case GtDbl:
  case GteBool:
  case GteDbl:
  case GteInt:
  case GteRes:
  case GteStr:
  case GteStrInt:
  case GtInt:
  case GtRes:
  case GtStr:
  case GtStrInt:
  case HasReifiedGenerics:
  case HasToString:
  case IncCallCounter:
  case IncProfCounter:
  case IncRef:
  case IncStat:
  case InitDictElem:
  case InitObjProps:
  case InitObjMemoSlots:
  case InitVecElem:
  case InitVecElemLoop:
  case InitThrowableFileAndLine:
  case InlineCall:
  case InlineReturn:
  case InstanceOf:
  case InstanceOfBitmask:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case InstanceOfRecDesc:
  case InterfaceSupportsArrLike:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case InterpOneCF:
  case IsCol:
  case IsFunReifiedGenericsMatched:
  case IsClsDynConstructible:
  case IsNType:
  case IsNTypeMem:
  case IsType:
  case IsTypeMem:
  case IsTypeStructCached:
  case IsWaitHandle:
  case IterFree:
  case Jmp:
  case JmpNZero:
  case JmpSSwitchDest:
  case JmpSwitchDest:
  case JmpZero:
  case JmpPlaceholder:
  case KeysetFirst:
  case KeysetGetK:
  case KeysetGetQuiet:
  case KeysetIdx:
  case KeysetIsset:
  case KeysetLast:
  case KillIter:
  case LdAFWHActRec:
  case LdARFlags:
  case LdBindAddr:
  case LdClosureCls:
  case LdClosureThis:
  case LdClsCachedSafe:
  case LdRecDescCachedSafe:
  case LdClsCns:
  case LdClsCnsVecLen:
  case LdClsFromClsMeth:
  case LdClsFromRClsMeth:
  case LdClsInitData:
  case LdClsInitElem:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case LdClsName:
  case LdLazyClsName:
  case LdCns:
  case LdColDict:
  case LdColVec:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
  case LdContField:
  case LdContResumeAddr:
  case LdFuncCls:
  case LdFrameCls:
  case LdFrameThis:
  case LdFuncFromClsMeth:
  case LdFuncFromRClsMeth:
  case LdFuncFromRFunc:
  case LdFuncNumParams:
  case LdFuncName:
  case LdFuncRxLevel:
  case LdFuncVecLen:
  case LdGenericsFromRFunc:
  case LdGenericsFromRClsMeth:
  case LdIfaceMethod:
  case LdInitPropAddr:
  case LdInitRDSAddr:
  case LdIterBase:
  case LdIterPos:
  case LdIterEnd:
  case LdLoc:
  case LdLocAddr:
  case LdMBase:
  case LdMem:
  case LdMethCallerName:
  case LdMIStateAddr:
  case LdPtrIterKey:
  case LdPtrIterVal:
  case LdRecDesc:
  case LdObjClass:
  case LdObjInvoke:
  case LdOutAddr:
  case LdVecElemAddr:
  case LdPairElem:
  case LdPropAddr:
  case LdRDSAddr:
  case LdRetVal:
  case LdSSwitchDestFast:
  case LdSmashable:
  case LdSmashableFunc:
  case LdStk:
  case LdStkAddr:
  case LdStrLen:
  case LdSubClsCns:
  case LdSubClsCnsClsName:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdTVAux:
  case LdTypeCns:
  case LdUnwinderValue:
  case LdVecElem:
  case LdVectorSize:
  case LdWHNotDone:
  case LdWHResult:
  case LdWHState:
  case LIterInit:
  case LIterInitK:
  case LIterNext:
  case LIterNextK:
  case LockObj:
  case LogArrayReach:
  case LookupClsRDS:
  case LookupSPropSlot:
  case Lshr:
  case LtBool:
  case LtDbl:
  case LteBool:
  case LteDbl:
  case LteInt:
  case LteRes:
  case LteStr:
  case LteStrInt:
  case LtInt:
  case LtRes:
  case LtStr:
  case LtStrInt:
  case MapIsset:
  case MarkRDSInitialized:
  case MemoGetInstanceCache:
  case MemoGetInstanceValue:
  case MemoGetStaticCache:
  case MemoGetStaticValue:
  case MemoGetLSBCache:
  case MemoGetLSBValue:
  case MemoSetInstanceCache:
  case MemoSetInstanceValue:
  case MemoSetStaticCache:
  case MemoSetStaticValue:
  case MemoSetLSBCache:
  case MemoSetLSBValue:
  case MethodExists:
  case Mod:
  case Mov:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case NeqBool:
  case NeqDbl:
  case NeqInt:
  case NeqRes:
  case NeqStr:
  case NeqStrInt:
  case NewClsMeth:
  case NewRClsMeth:
  case NewCol:
  case NewColFromArray:
  case NewDArray:
  case NewDictArray:
  case NewInstanceRaw:
  case NewLoggingArray:
  case NewPair:
  case NewRFunc:
  case NewStructDArray:
  case NewStructDict:
  case NInstanceOfBitmask:
  case Nop:
  case NSameObj:
  case NSameStr:
  case OrdStr:
  case OrInt:
  case PairIsset:
  case ProfileCall:
  case ProfileDecRef:
  case ProfileDictAccess:
  case ProfileInstanceCheck:
  case ProfileIsTypeStruct:
  case ProfileKeysetAccess:
  case ProfileMethod:
  case ProfileProp:
  case ProfileSubClsCns:
  case ProfileSwitchDest:
  case ProfileType:
  case RBTraceEntry:
  case RBTraceMsg:
  case ReqBindJmp:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case ReserveVecNewElem:
  case RestoreErrorLevel:
  case RetCtrl:
  case SameObj:
  case SameStr:
  case Select:
  case SetLegacyDict:
  case SetLegacyVec:
  case UnsetLegacyDict:
  case UnsetLegacyVec:
  case Shl:
  case Shr:
  case Sqrt:
  case StArResumeAddr:
  case StClosureArg:
  case StClsInitElem:
  case StContArKey:
  case StContArState:
  case StContArValue:
  case StFrameCtx:
  case StFrameFunc:
  case StFrameMeta:
  case StIterBase:
  case StIterType:
  case StIterPos:
  case StIterEnd:
  case StLoc:
  case StLocRange:
  case StMBase:
  case StMem:
  case StImplicitContext:
  case StOutValue:
  case StrictlyIntegerConv:
  case StringIsset:
  case StStk:
  case SubDbl:
  case SubInt:
  case SubIntO:
  case SyncReturnBC:
  case Unreachable:
  case UnwindCheckSideExit:
  case VecFirst:
  case VecLast:
  case VectorIsset:
  case XorBool:
  case XorInt:
  case ZeroErrorLevel:
    return false;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}}
