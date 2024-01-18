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

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

#define NF     0
#define PRc    ProducesRC
#define CRc    ConsumesRC
#define T      Terminal
#define B      Branch
#define P      Passthrough
#define LA     LayoutAgnostic
#define LP     (LayoutPreserving|LayoutAgnostic)

#define ND             0
#define D(n)           HasDest
#define DofS(n)        HasDest
#define DRefineS(n)    HasDest
#define DParam(t)      HasDest
#define DLdObjCls      HasDest
#define DAllocObj      HasDest
#define DBespokeElem   HasDest
#define DBespokeElemUninit   HasDest
#define DBespokePosKey HasDest
#define DBespokePosVal HasDest
#define DVecElem       HasDest
#define DDictElem      HasDest
#define DModified(n)   HasDest
#define DArrLikeSet    HasDest
#define DArrLikeUnset  HasDest
#define DArrLikeAppend HasDest
#define DKeysetElem    HasDest
#define DEscalateToVanilla HasDest
#define DVecKey           HasDest
#define DFirstElem        HasDest
#define DLastElem         HasDest
#define DFirstKey         HasDest
#define DLastKey          HasDest
#define DLoggingArrLike   HasDest
#define DStructDict    HasDest
#define DTypeStructElem   HasDest
#define DCol           HasDest
#define DMulti         NaryDest
#define DSetElem       HasDest
#define DPtrToParam    HasDest
#define DBuiltin       HasDest
#define DCall          HasDest
#define DGenIter       HasDest
#define DSubtract(n,t) HasDest
#define DUnion(...)    HasDest
#define DMemoKey       HasDest
#define DLvalOfPtr     HasDest
#define DTypeCnsClsName HasDest
#define DVerifyCoerce  HasDest
#define DPropLval      HasDest
#define DElemLval      HasDest
#define DElemLvalPos   HasDest
#define DCOW           HasDest
#define DStructTypeBound HasDest
#define DSpecialIC     HasDest

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

#undef ND
#undef D
#undef DofS
#undef DRefineS
#undef DParam
#undef DLdObjCls
#undef DBespokeElemUninit
#undef DBespokeElem
#undef DBespokePosKey
#undef DBespokePosVal
#undef DVecElem
#undef DDictElem
#undef DKeysetElem
#undef DEscalateToVanilla
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
#undef DStructDict
#undef DTypeStructElem
#undef DCol
#undef DAllocObj
#undef DMulti
#undef DSetElem
#undef DPtrToParam
#undef DBuiltin
#undef DCall
#undef DGenIter
#undef DSubtract
#undef DUnion
#undef DMemoKey
#undef DLvalOfPtr
#undef DTypeCnsClsName
#undef DVerifyCoerce
#undef DPropLval
#undef DElemLval
#undef DElemLvalPos
#undef DCOW
#undef DStructTypeBound
#undef DSpecialIC

///////////////////////////////////////////////////////////////////////////////

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

Optional<Opcode> negateCmpOp(Opcode opc) {
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

    default:                  return std::nullopt;
  }
}

bool opcodeMayRaise(Opcode opc) {
  switch (opc) {
  case IsTypeStruct:
  case IsTypeStructShallow:
    return RuntimeOption::EvalIsExprEnableUnresolvedWarning ||
           RuntimeOption::EvalIsVecNotices ||
           RuntimeOption::EvalWarnOnImplicitCoercionOfEnumValue;

  case AddNewElemKeyset:
  case AFWHPrepareChild:
  case AKExistsObj:
  case AllocObj:
  case ArrayMarkLegacyShallow:
  case ArrayMarkLegacyRecursive:
  case ArrayUnmarkLegacyShallow:
  case ArrayUnmarkLegacyRecursive:
  case BaseG:
  case BespokeAppend:
  case BespokeElem:
  case BespokeGetThrow:
  case BespokeSet:
  case BespokeUnset:
  case Call:
  case CallBuiltin:
  case CallFuncEntry:
  case CGetElem:
  case CGetProp:
  case CGetPropQ:
  case CheckClsMethFunc:
  case CheckClsReifiedGenericMismatch:
  case CheckClsRGSoft:
  case CheckFunReifiedGenericMismatch:
  case CheckInOutMismatch:
  case CheckReadonlyMismatch:
  case CheckStackOverflow:
  case HandleSurpriseEnter:
  case Clone:
  case CmpArrLike:
  case CmpObj:
  case CmpRes:
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
  case ConvObjToBool:
  case ConvObjToDbl:
  case ConvObjToDict:
  case ConvObjToInt:
  case ConvObjToKeyset:
  case ConvObjToStr:
  case ConvObjToVec:
  case Count:
  case CreateCCWH:
  case DictGet:
  case DictSet:
  case ElemDictD:
  case ElemDictU:
  case ElemDX:
  case ElemUX:
  case ElemX:
  case EqArrLike:
  case EqObj:
  case GetMemoKey:
  case GetClsRGProp:
  case GtArrLike:
  case GteArrLike:
  case GteObj:
  case GteRes:
  case GtObj:
  case GtRes:
  case HandleRequestSurprise:
  case IncDecElem:
  case IncDecProp:
  case InitClsCns:
  case InitProps:
  case InitSProps:
  case InitSubClsCns:
  case InlineSideExit:
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
  case LdCoeffectFunParamNaive:
  case LdFunc:
  case LdFuncCached:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdObjMethodD:
  case LdObjMethodS:
  case LdTypeCns:
  case LdTypeCnsClsName:
  case LdTypeCnsNoThrow:
  case LookupClsCns:
  case LookupClsCtxCns:
  case LookupClsMethod:
  case LookupClsMethodCache:
  case LookupClsMethodFCache:
  case LookupCnsE:
  case LookupFuncCached:
  case LtArrLike:
  case LteArrLike:
  case LteObj:
  case LteRes:
  case LtObj:
  case LtRes:
  case MapGet:
  case MapSet:
  case NativeImpl:
  case NeqArrLike:
  case NeqObj:
  case NewKeysetArray:
  case NSameArrLike:
  case OODeclExists:
  case OrdStrIdx:
  case OutlineSetOp:
  case PrintBool:
  case PrintInt:
  case PrintStr:
  case ProfileSubClsCns:
  case PropDX:
  case PropQ:
  case PropTypeRedefineCheck:
  case PropTypeValid:
  case PropX:
  case RaiseCoeffectsCallViolation:
  case RaiseCoeffectsFunParamCoeffectRulesViolation:
  case RaiseCoeffectsFunParamTypeViolation:
  case RaiseDeploymentBoundaryViolation:
  case RaiseError:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseImplicitContextStateInvalid:
  case RaiseModuleBoundaryViolation:
  case RaiseModulePropertyViolation:
  case RaiseNotice:
  case RaiseStrToClassNotice:
  case RaiseTooManyArg:
  case ThrowUndefPropException:
  case ThrowUninitLoc:
  case RaiseWarning:
  case RecordReifiedGenericsAndGetTSList:
  case ResolveTypeStruct:
  case ReturnHook:
  case SameArrLike:
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
  case ThrowAsTypeStructError:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case ThrowCallReifiedFunctionWithoutGenerics:
  case ThrowDivisionByZeroException:
  case ThrowHasThisNeedStatic:
  case ThrowInOutMismatch:
  case ThrowReadonlyMismatch:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowLateInitPropError:
  case ThrowMissingArg:
  case ThrowMissingThis:
  case ThrowCannotModifyReadonlyCollection:
  case ThrowLocalMustBeValueTypeException:
  case ThrowMustBeEnclosedInReadonly:
  case ThrowMustBeMutableException:
  case ThrowMustBeReadonlyException:
  case ThrowMustBeValueTypeException:
  case ThrowOutOfBounds:
  case ThrowParameterWrongType:
  case UnsetElem:
  case UnsetProp:
  case VectorSet:
  case VerifyParam:
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamCoerce:
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
  case VerifyRet:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetCoerce:
  case VerifyRetFail:
  case VerifyRetFailHard:
    return true;

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddNewElemVec:
  case AddOffset:
  case AdvanceDictPtrIter:
  case AdvanceVecPtrIter:
  case AFWHBlockOn:
  case AFWHPushTailFrame:
  case AKExistsDict:
  case AKExistsKeyset:
  case AllocBespokeStructDict:
  case AllocInitROM:
  case AllocStructDict:
  case AllocVec:
  case AndInt:
  case AssertLoc:
  case AssertMBase:
  case AssertNonNull:
  case AssertStk:
  case AssertType:
  case AsyncFuncRet:
  case AsyncFuncRetPrefetch:
  case AsyncFuncRetSlow:
  case AsyncGenRetR:
  case AsyncGenYieldR:
  case AsyncSwitchFast:
  case BeginCatch:
  case BeginInlining:
  case BespokeEscalateToVanilla:
  case BespokeGet:
  case BespokeIterFirstPos:
  case BespokeIterLastPos:
  case BespokeIterEnd:
  case BespokeIterGetKey:
  case BespokeIterGetVal:
  case CallViolatesDeploymentBoundary:
  case CallViolatesModuleBoundary:
  case Ceil:
  case CheckArrayCOW:
  case CheckCold:
  case CheckDictKeys:
  case CheckDictOffset:
  case CheckFuncNeedsCoverage:
  case CheckInit:
  case CheckInitMem:
  case CheckIter:
  case CheckKeysetOffset:
  case CheckLoc:
  case CheckMBase:
  case CheckMissingKeyInArrLike:
  case CheckMROProp:
  case CheckNonNull:
  case CheckNullptr:
  case CheckRange:
  case CheckRDSInitialized:
  case CheckSmashableClass:
  case CheckStk:
  case CheckSubClsCns:
  case CheckSurpriseFlags:
  case CheckSurpriseFlagsEnter:
  case CheckType:
  case CheckTypeMem:
  case CheckVecBounds:
  case ChrInt:
  case ClassHasAttr:
  case ClassHasReifiedGenerics:
  case CmpBool:
  case CmpDbl:
  case CmpInt:
  case CmpStr:
  case ColIsEmpty:
  case ColIsNEmpty:
  case Conjure:
  case ConjureUse:
  case ConstructClosure:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContCheckNext:
  case ContValid:
  case ConvBoolToDbl:
  case ConvBoolToInt:
  case ConvDblToBool:
  case ConvDblToInt:
  case ConvDblToStr:
  case ConvFuncPrologueFlagsToARFlags:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvIntToStr:
  case ConvPtrToLval:
  case ConvResToDbl:
  case ConvResToInt:
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvStrToInt:
  case CopyArray:
  case CountCollection:
  case CountDict:
  case CountKeyset:
  case CountVec:
  case CountWHNotDone:
  case CreateAFWH:
  case CreateAGen:
  case CreateAGWH:
  case CreateGen:
  case CreateSpecialImplicitContext:
  case CreateSSWH:
  case DbgAssertFunc:
  case DbgAssertRefCount:
  case DbgCheckLocalsDecRefd:
  case DbgTraceCall:
  case DbgTrashFrame:
  case DbgTrashMem:
  case DbgTrashStk:
  case DblAsBits:
  case DebugBacktrace:
  case DecRef:
  case ReleaseShallow:
  case DecReleaseCheck:
  case DecRefNZ:
  case DefConst:
  case DefFP:
  case DefFrameRelSP:
  case DefFuncEntryArFlags:
  case DefFuncEntryCalleeId:
  case DefFuncEntryCtx:
  case DefFuncEntryFP:
  case DefFuncEntryPrevFP:
  case DefFuncPrologueCallee:
  case DefFuncPrologueCtx:
  case DefFuncPrologueFlags:
  case DefFuncPrologueNumArgs:
  case DefLabel:
  case DefRegSP:
  case DeserializeLazyProp:
  case DictFirst:
  case DictFirstKey:
  case DictGetK:
  case DictGetQuiet:
  case DictIdx:
  case DictIsset:
  case DictLast:
  case DictLastKey:
  case DirFromFilepath:
  case DivDbl:
  case DivInt:
  case StVMFP:
  case StVMSP:
  case StVMPC:
  case StVMReturnAddr:
  case ElemDictK:
  case EndBlock:
  case EndCatch:
  case EndGuards:
  case EndInlining:
  case EnterFrame:
  case EnterInlineFrame:
  case EnterPrologue:
  case EnterTCUnwind:
  case EnterTranslation:
  case EqArrayDataPtr:
  case EqBool:
  case EqCls:
  case EqLazyCls:
  case EqDbl:
  case EqFunc:
  case EqInt:
  case EqPtrIter:
  case EqRes:
  case EqStr:
  case EqStrPtr:
  case ExitPrologue:
  case ExtendsClass:
  case FinishMemberOp:
  case Floor:
  case FuncCred:
  case FuncHasAttr:
  case FuncHasReifiedGenerics:
  case GenericRetDecRefs:
  case LoadBCSP:
  case GetDictPtrIter:
  case GetMemoKeyScalar:
  case GetTime:
  case GetTimeNs:
  case GetVecPtrIter:
  case GtBool:
  case GtDbl:
  case GteBool:
  case GteDbl:
  case GteInt:
  case GteStr:
  case GtInt:
  case GtStr:
  case HasReifiedParent:
  case HasToString:
  case IncCallCounter:
  case IncProfCounter:
  case IncRef:
  case IncStat:
  case InitDictElem:
  case InitObjMemoSlots:
  case InitObjProps:
  case InitStructElem:
  case InitStructPositions:
  case InitThrowableFileAndLine:
  case InitVecElem:
  case InitVecElemLoop:
  case InlineCall:
  case InlineSideExitSyncStack:
  case InstanceOf:
  case InstanceOfBitmask:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case InterfaceSupportsArrLike:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case InterpOneCF:
  case IsCol:
  case IsFunReifiedGenericsMatched:
  case IsNType:
  case IsNTypeMem:
  case IsLegacyArrLike:
  case IsType:
  case IsTypeMem:
  case IsTypeStructCached:
  case IsWaitHandle:
  case IterFree:
  case Jmp:
  case JmpExit:
  case JmpNZero:
  case JmpPlaceholder:
  case JmpZero:
  case KeysetFirst:
  case KeysetGetK:
  case KeysetGetQuiet:
  case KeysetIdx:
  case KeysetIsset:
  case KeysetLast:
  case KillActRec:
  case KillIter:
  case KillLoc:
  case LdAFWHActRec:
  case LdARFlags:
  case LdBindAddr:
  case LdClosureCls:
  case LdClosureThis:
  case LdClsCachedSafe:
  case LdClsCns:
  case LdClsCnsVecLen:
  case LdClsCtxCns:
  case LdClsFromClsMeth:
  case LdClsFromRClsMeth:
  case LdClsInitData:
  case LdClsInitElem:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case LdClsName:
  case LdCns:
  case LdColDict:
  case LdColVec:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
  case LdContField:
  case LdContResumeAddr:
  case LdEnumClassLabelName:
  case LdFrameCls:
  case LdFrameThis:
  case LdFuncCls:
  case LdARFunc:
  case LdFuncFromClsMeth:
  case LdFuncFromRClsMeth:
  case LdFuncFromRFunc:
  case LdFuncInOutBits:
  case LdFuncName:
  case LdFuncNumParams:
  case LdFuncRequiredCoeffects:
  case LdFuncVecLen:
  case LdGenericsFromRClsMeth:
  case LdGenericsFromRFunc:
  case LdIfaceMethod:
  case LdImplicitContext:
  case LdImplicitContextMemoKey:
  case LdInitPropAddr:
  case LdInitRDSAddr:
  case LdIterBase:
  case LdIterEnd:
  case LdIterPos:
  case LdLazyCls:
  case LdLazyClsName:
  case LdLoc:
  case LdLocAddr:
  case LdLocForeign:
  case LdMBase:
  case LdMem:
  case LdMethCallerName:
  case LdMIStateTempBaseAddr:
  case LdObjClass:
  case LdObjInvoke:
  case LdOutAddr:
  case LdPairElem:
  case LdPropAddr:
  case LdPtrIterKey:
  case LdPtrIterVal:
  case LdRDSAddr:
  case LdResolvedTypeCns:
  case LdResolvedTypeCnsClsName:
  case LdResolvedTypeCnsNoCheck:
  case LdRetVal:
  case LdSmashable:
  case LdSmashableFunc:
  case LdSwitchDest:
  case LdSSwitchDest:
  case LdStk:
  case LdStkAddr:
  case LdStrLen:
  case LdSubClsCns:
  case LdTVAux:
  case LdTVFromRDS:
  case LdUnitPerRequestFilepath:
  case LdUnwinderValue:
  case LdMonotypeDictTombstones:
  case LdMonotypeDictKey:
  case LdMonotypeDictVal:
  case LdMonotypeVecElem:
  case LdTypeStructureVal:
  case LdTypeStructureValCns:
  case LdVecElem:
  case LdVecElemAddr:
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
  case LogGuardFailure:
  case LookupClsRDS:
  case LookupSPropSlot:
  case Lshr:
  case LtBool:
  case LtDbl:
  case LteBool:
  case LteDbl:
  case LteInt:
  case LteStr:
  case LtInt:
  case LtStr:
  case MapIsset:
  case MarkRDSAccess:
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
  case NewClsMeth:
  case NewRClsMeth:
  case NewCol:
  case NewColFromArray:
  case NewDictArray:
  case NewInstanceRaw:
  case NewLoggingArray:
  case NewPair:
  case NewRFunc:
  case NewStructDict:
  case NewBespokeStructDict:
  case NInstanceOfBitmask:
  case Nop:
  case NSameObj:
  case NSameStr:
  case OrdStr:
  case OrInt:
  case PairIsset:
  case ProfileArrayCOW:
  case ProfileArrLikeProps:
  case ProfileCall:
  case ProfileCoeffectFunParam:
  case ProfileDecRef:
  case ProfileDictAccess:
  case ProfileGlobal:
  case ProfileInstanceCheck:
  case ProfileIsTypeStruct:
  case ProfileKeysetAccess:
  case ProfileMethod:
  case ProfileProp:
  case ProfileSwitchDest:
  case ProfileType:
  case RBTraceEntry:
  case RBTraceMsg:
  case RecordFuncCall:
  case ReqBindJmp:
  case ReqInterpBBNoTranslate:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case ReserveVecNewElem:
  case RestoreErrorLevel:
  case RetCtrl:
  case SameObj:
  case SameStr:
  case Select:
  case Shl:
  case Shr:
  case Sqrt:
  case StArResumeAddr:
  case StClsInitElem:
  case StClosureArg:
  case StContArKey:
  case StContArState:
  case StContArValue:
  case StFrameCtx:
  case StFrameFunc:
  case StFrameMeta:
  case StImplicitContext:
  case StIterBase:
  case StIterEnd:
  case StIterPos:
  case StIterType:
  case StLoc:
  case StLocMeta:
  case StLocRange:
  case StMBase:
  case StMem:
  case StMemMeta:
  case StOutValue:
  case StrictlyIntegerConv:
  case StringIsset:
  case StructDictAddNextSlot:
  case StructDictElemAddr:
  case StructDictSlot:
  case StructDictTypeBoundCheck:
  case StructDictUnset:
  case StructDictSlotInPos:
  case LdStructDictKey:
  case LdStructDictVal:
  case StMROProp:
  case StPtrAt:
  case StTypeAt:
  case StStk:
  case StStkMeta:
  case StStkRange:
  case StVMRegState:
  case StTVInRDS:
  case SubDbl:
  case SubInt:
  case Unreachable:
  case UnwindCheckSideExit:
  case VecFirst:
  case VecLast:
  case VectorIsset:
  case VoidPtrAsDataType:
  case XorBool:
  case XorInt:
  case ZeroErrorLevel:
    return false;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
}
