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

#define ND             0
#define D(n)           HasDest
#define DofS(n)        HasDest
#define DRefineS(n)    HasDest
#define DParamMayRelax(t) HasDest
#define DParam(t)      HasDest
#define DLdObjCls      HasDest
#define DUnboxPtr      HasDest
#define DBoxPtr        HasDest
#define DAllocObj      HasDest
#define DArrElem       HasDest
#define DVecElem       HasDest
#define DDictElem      HasDest
#define DKeysetElem    HasDest
#define DArrPacked     HasDest
#define DArrMixed      HasDest
#define DVArr          HasDest
#define DDArr          HasDest
#define DStaticDArr    HasDest
#define DCol           HasDest
#define DCtx           HasDest
#define DCtxCls        HasDest
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
#undef DUnboxPtr
#undef DBoxPtr
#undef DArrElem
#undef DVecElem
#undef DDictElem
#undef DKeysetElem
#undef DArrPacked
#undef DArrMixed
#undef DVArr
#undef DDArr
#undef DStaticDArr
#undef DCol
#undef DAllocObj
#undef DCtx
#undef DCtxCls
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
  case CallUnpack:
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
    case EqArr:               return NeqArr;
    case NeqArr:              return EqArr;
    case SameArr:             return NSameArr;
    case NSameArr:            return SameArr;

    case EqVec:               return NeqVec;
    case NeqVec:              return EqVec;
    case SameVec:             return NSameVec;
    case NSameVec:            return SameVec;

    case EqDict:              return NeqDict;
    case NeqDict:             return EqDict;
    case SameDict:            return NSameDict;
    case NSameDict:           return SameDict;

    case EqKeyset:            return NeqKeyset;
    case NeqKeyset:           return EqKeyset;
    case SameKeyset:          return NSameKeyset;
    case NSameKeyset:         return SameKeyset;

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
  // AKExistsArr, ArrayIdx, and ArrayIsset may only raise errors when the option
  // EvalHackArrCompatNotices is set.
  case AKExistsArr:
  case ArrayIdx:
  case ArrayIsset:
    return RuntimeOption::EvalHackArrCompatNotices;

  // Same thing for SameArr and NSameArr, but for EvalHackArrCompatDVCmpNotices.
  case NSameArr:
  case SameArr:
    return RuntimeOption::EvalHackArrCompatDVCmpNotices;

  // Same thing for IsTypeStruct, but for EvalHackArrCompatIsArrayNotices
  // and EvalIsExprEnableUnresolvedWarning.
  case IsTypeStruct:
    return RuntimeOption::EvalHackArrCompatIsArrayNotices
      || RuntimeOption::EvalIsExprEnableUnresolvedWarning;

  case AddElemIntKey:
  case AddElemStrKey:
  case AddNewElem:
  case AddNewElemKeyset:
  case AFWHPrepareChild:
  case AKExistsObj:
  case AllocObj:
  case ArrayAdd:
  case ArrayGet:
  case ArraySet:
  case ArraySetRef:
  case AsTypeStruct:
  case BaseG:
  case BindElem:
  case BindNewElem:
  case BindProp:
  case Call:
  case CallBuiltin:
  case CallUnpack:
  case CastMem:
  case CastStk:
  case CGetElem:
  case CGetProp:
  case CGetPropQ:
  case CheckStackOverflow:
  case CheckSurpriseAndStack:
  case CheckSurpriseFlagsEnter:
  case Clone:
  case CmpArr:
  case CmpObj:
  case CmpVec:
  case CoerceCellToBool:
  case CoerceCellToDbl:
  case CoerceCellToInt:
  case CoerceMem:
  case CoerceStk:
  case CoerceStrToDbl:
  case CoerceStrToInt:
  case ConcatIntStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConcatStrInt:
  case ConcatStrStr:
  case ConstructInstance:
  case ContEnter:
  case ConvArrToDict:
  case ConvArrToKeyset:
  case ConvArrToVec:
  case ConvCellToArr:
  case ConvCellToDbl:
  case ConvCellToInt:
  case ConvCellToObj:
  case ConvCellToStr:
  case ConvDictToArr:
  case ConvDictToDArr:
  case ConvDictToKeyset:
  case ConvKeysetToArr:
  case ConvKeysetToDArr:
  case ConvObjToArr:
  case ConvObjToDArr:
  case ConvObjToDbl:
  case ConvObjToDict:
  case ConvObjToInt:
  case ConvObjToKeyset:
  case ConvObjToStr:
  case ConvObjToVArr:
  case ConvObjToVec:
  case ConvResToStr:
  case ConvVecToKeyset:
  case Count:
  case CreateAAWH:
  case DecodeCufIter:
  case DefCls:
  case DictAddElemIntKey:
  case DictAddElemStrKey:
  case DictGet:
  case DictSet:
  case DictSetRef:
  case ElemArrayD:
  case ElemArrayU:
  case ElemArrayX:
  case ElemDictD:
  case ElemDictU:
  case ElemDictX:
  case ElemDX:
  case ElemKeysetU:
  case ElemKeysetX:
  case ElemUX:
  case ElemVecD:
  case ElemVecU:
  case ElemX:
  case EmptyElem:
  case EmptyProp:
  case EqArr:
  case EqDict:
  case EqObj:
  case EqVec:
  case FatalMissingThis:
  case GetMemoKey:
  case GtArr:
  case GteArr:
  case GteObj:
  case GteVec:
  case GtObj:
  case GtVec:
  case IncDecElem:
  case IncDecProp:
  case InitClsCns:
  case InitExtraArgs:
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
  case LdArrFuncCtx:
  case LdCls:
  case LdClsCached:
  case LdClsCtor:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdClsTypeCns:
  case LdFunc:
  case LdFuncCached:
  case LdFuncCachedU:
  case LdObjMethod:
  case LdSSwitchDestSlow:
  case LdSwitchObjIndex:
  case LookupClsMethod:
  case LookupClsMethodCache:
  case LookupClsMethodFCache:
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case LookupFuncCached:
  case LtArr:
  case LteArr:
  case LteObj:
  case LteVec:
  case LtObj:
  case LtVec:
  case MapGet:
  case MapSet:
  case MIterInit:
  case MIterInitK:
  case NativeImpl:
  case NeqArr:
  case NeqDict:
  case NeqObj:
  case NeqVec:
  case NewKeysetArray:
  case OODeclExists:
  case OrdStrIdx:
  case PrintBool:
  case PrintInt:
  case PrintStr:
  case PropDX:
  case PropQ:
  case PropTypeRedefineCheck:
  case PropX:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case RaiseError:
  case RaiseForbiddenDynCall:
  case RaiseHackArrCompatNotice:
  case RaiseHackArrParamNotice:
  case RaiseHackArrPropNotice:
  case RaiseMissingArg:
  case RaiseMissingThis:
  case RaiseNotice:
  case RaiseParamRefMismatchForFunc:
  case RaiseParamRefMismatchForFuncName:
  case RaiseUndefProp:
  case RaiseUninitLoc:
  case RaiseVarEnvDynCall:
  case RaiseWarning:
  case ResolveTypeStruct:
  case ReturnHook:
  case SetElem:
  case SetNewElem:
  case SetNewElemArray:
  case SetNewElemKeyset:
  case SetNewElemVec:
  case SetOpCell:
  case SetOpElem:
  case SetOpProp:
  case SetProp:
  case SetWithRefElem:
  case StringGet:
  case SuspendHookAwaitEF:
  case SuspendHookAwaitEG:
  case SuspendHookAwaitR:
  case SuspendHookCreateCont:
  case SuspendHookYield:
  case ThrowArithmeticError:
  case ThrowDivisionByZeroError:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowOutOfBounds:
  case UnsetElem:
  case UnsetProp:
  case VecSet:
  case VecSetRef:
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamFail:
  case VerifyParamFailHard:
  case VerifyProp:
  case VerifyPropCls:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
  case VerifyRetFailHard:
  case VGetElem:
  case VGetProp:
  case WIterInit:
  case WIterInitK:
  case WIterNext:
  case WIterNextK:
    return true;

  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AddNewElemVec:
  case AFWHBlockOn:
  case AKExistsDict:
  case AKExistsKeyset:
  case AllocPackedArray:
  case AllocVArray:
  case AllocVecArray:
  case AndInt:
  case AssertLoc:
  case AssertMBase:
  case AssertNonNull:
  case AssertStk:
  case AssertType:
  case AssertARFunc:
  case AsyncFuncRet:
  case AsyncFuncRetSlow:
  case AsyncSwitchFast:
  case BeginCatch:
  case BeginInlining:
  case Box:
  case BoxPtr:
  case Ceil:
  case CheckARMagicFlag:
  case CheckArrayCOW:
  case CheckCold:
  case CheckCtxThis:
  case CheckDArray:
  case CheckDictOffset:
  case CheckFuncStatic:
  case CheckInit:
  case CheckInitMem:
  case CheckKeysetOffset:
  case CheckLoc:
  case CheckMBase:
  case CheckMixedArrayOffset:
  case CheckNonNull:
  case CheckNullptr:
  case CheckPackedArrayDataBounds:
  case CheckRange:
  case CheckRDSInitialized:
  case CheckRefInner:
  case CheckRefs:
  case CheckStaticLoc:
  case CheckStk:
  case CheckSubClsCns:
  case CheckSurpriseFlags:
  case CheckType:
  case CheckTypeMem:
  case CheckVArray:
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
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContPreNext:
  case ContStarted:
  case ContStartedCheck:
  case ContValid:
  case ConvArrToBool:
  case ConvArrToDArr:
  case ConvArrToDbl:
  case ConvArrToNonDVArr:
  case ConvArrToVArr:
  case ConvBoolToArr:
  case ConvBoolToDbl:
  case ConvBoolToInt:
  case ConvCellToBool:
  case ConvClsToCctx:
  case ConvDblToArr:
  case ConvDblToBool:
  case ConvDblToInt:
  case ConvDblToStr:
  case ConvDictToVArr:
  case ConvDictToVec:
  case ConvIntToArr:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvIntToStr:
  case ConvKeysetToDict:
  case ConvKeysetToVArr:
  case ConvKeysetToVec:
  case ConvObjToBool:
  case ConvResToDbl:
  case ConvResToInt:
  case ConvStrToArr:
  case ConvStrToBool:
  case ConvStrToDbl:
  case ConvStrToInt:
  case ConvVecToArr:
  case ConvVecToDArr:
  case ConvVecToDict:
  case ConvVecToVArr:
  case ConvPtrToLval:
  case CountArray:
  case CountArrayFast:
  case CountCollection:
  case CountDict:
  case CountKeyset:
  case CountVec:
  case CountWHNotDone:
  case CreateAFWH:
  case CreateAFWHNoVV:
  case CreateAGen:
  case CreateAGWH:
  case CreateGen:
  case CreateSSWH:
  case DbgAssertFunc:
  case DbgAssertARFunc:
  case DbgAssertRefCount:
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
  case DefConst:
  case DefFP:
  case DefInlineFP:
  case DefLabel:
  case DefSP:
  case DictEmptyElem:
  case DictGetK:
  case DictGetQuiet:
  case DictIdx:
  case DictIsset:
  case DivDbl:
  case DivInt:
  case EagerSyncVMRegs:
  case ElemDictK:
  case ElemKeysetK:
  case ElemMixedArrayK:
  case EndBlock:
  case EndCatch:
  case EndGuards:
  case EnterFrame:
  case EqArrayDataPtr:
  case EqBool:
  case EqCls:
  case EqDbl:
  case EqFunc:
  case EqInt:
  case EqKeyset:
  case EqRes:
  case EqStr:
  case EqStrInt:
  case EqStrPtr:
  case ExitPlaceholder:
  case ExtendsClass:
  case FinishMemberOp:
  case Floor:
  case FuncGuard:
  case FwdCtxStaticCall:
  case GenericRetDecRefs:
  case GetMemoKeyScalar:
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
  case HasToString:
  case HintLocInner:
  case HintMBaseInner:
  case HintStkInner:
  case IncProfCounter:
  case IncRef:
  case IncStat:
  case InitCtx:
  case InitObjProps:
  case InitPackedLayoutArray:
  case InitPackedLayoutArrayLoop:
  case InitStaticLoc:
  case InitThrowableFileAndLine:
  case InlineReturn:
  case InlineReturnNoFrame:
  case InstanceOf:
  case InstanceOfBitmask:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case InterfaceSupportsArr:
  case InterfaceSupportsDbl:
  case InterfaceSupportsDict:
  case InterfaceSupportsInt:
  case InterfaceSupportsKeyset:
  case InterfaceSupportsStr:
  case InterfaceSupportsVec:
  case InterpOneCF:
  case IsCol:
  case IsDVArray:
  case IsFuncDynCallable:
  case IsNType:
  case IsNTypeMem:
  case IsType:
  case IsTypeMem:
  case IsWaitHandle:
  case IterFree:
  case Jmp:
  case JmpNZero:
  case JmpSSwitchDest:
  case JmpSwitchDest:
  case JmpZero:
  case KeysetEmptyElem:
  case KeysetGetK:
  case KeysetGetQuiet:
  case KeysetIdx:
  case KeysetIsset:
  case KillClsRef:
  case KillCufIter:
  case LdAFWHActRec:
  case LdARCtx:
  case LdARFuncPtr:
  case LdARInvName:
  case LdARIsDynamic:
  case LdARNumArgsAndFlags:
  case LdARNumParams:
  case LdBindAddr:
  case LdCctx:
  case LdClosure:
  case LdClosureCtx:
  case LdClosureStaticLoc:
  case LdClsCachedSafe:
  case LdClsCctx:
  case LdClsCns:
  case LdClsCnsVecLen:
  case LdClsCtx:
  case LdClsInitData:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case LdClsName:
  case LdClsRef:
  case LdCns:
  case LdColDict:
  case LdColVec:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
  case LdContField:
  case LdContResumeAddr:
  case LdCtx:
  case LdCufIterCtx:
  case LdCufIterDynamic:
  case LdCufIterFunc:
  case LdCufIterInvName:
  case LdElem:
  case LdFuncNumParams:
  case LdFuncVecLen:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdIfaceMethod:
  case LdLoc:
  case LdLocAddr:
  case LdLocPseudoMain:
  case LdMBase:
  case LdMem:
  case LdMIPropStateAddr:
  case LdMIStateAddr:
  case LdObjClass:
  case LdObjInvoke:
  case LdPackedArrayDataElemAddr:
  case LdPackedElem:
  case LdPairBase:
  case LdPropAddr:
  case LdRDSAddr:
  case LdRef:
  case LdRetVal:
  case LdSSwitchDestFast:
  case LdStaticLoc:
  case LdStk:
  case LdStkAddr:
  case LdStrLen:
  case LdSubClsCns:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdTVAux:
  case LdTypeCns:
  case LdUnwinderValue:
  case LdVecElem:
  case LdVectorBase:
  case LdVectorSize:
  case LdWHNotDone:
  case LdWHResult:
  case LdWHState:
  case LIterInit:
  case LIterInitK:
  case LIterNext:
  case LIterNextK:
  case LookupClsRDS:
  case LookupSPropSlot:
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
  case MIterFree:
  case MIterNext:
  case MIterNextK:
  case MixedArrayGetK:
  case Mod:
  case Mov:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case NeqBool:
  case NeqDbl:
  case NeqInt:
  case NeqKeyset:
  case NeqRes:
  case NeqStr:
  case NeqStrInt:
  case NewArray:
  case NewCol:
  case NewColFromArray:
  case NewDArray:
  case NewDictArray:
  case NewInstanceRaw:
  case NewLikeArray:
  case NewMixedArray:
  case NewPair:
  case NewStructArray:
  case NewStructDArray:
  case NewStructDict:
  case NInstanceOfBitmask:
  case Nop:
  case NSameDict:
  case NSameKeyset:
  case NSameObj:
  case NSameStr:
  case NSameVec:
  case OrdStr:
  case OrInt:
  case PackMagicArgs:
  case PairIsset:
  case ProfileArrayKind:
  case ProfileDictOffset:
  case ProfileInstanceCheck:
  case ProfileKeysetOffset:
  case ProfileFunc:
  case ProfileMethod:
  case ProfileMixedArrayOffset:
  case ProfileSubClsCns:
  case ProfileSwitchDest:
  case ProfileType:
  case RBTraceEntry:
  case RBTraceMsg:
  case RegisterLiveObj:
  case ReleaseVVAndSkip:
  case ReqBindJmp:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case ReservePackedArrayDataNewElem:
  case RestoreErrorLevel:
  case RetCtrl:
  case SameDict:
  case SameKeyset:
  case SameObj:
  case SameStr:
  case SameVec:
  case Select:
  case Shl:
  case Shr:
  case SpillFrame:
  case Sqrt:
  case StARInvName:
  case StARNumArgsAndFlags:
  case StArResumeAddr:
  case StClosureArg:
  case StClosureCtx:
  case StClsRef:
  case StContArKey:
  case StContArState:
  case StContArValue:
  case StCufIterCtx:
  case StCufIterDynamic:
  case StCufIterFunc:
  case StCufIterInvName:
  case StElem:
  case StLoc:
  case StLocPseudoMain:
  case StLocRange:
  case StMBase:
  case StMem:
  case StMIPropState:
  case StOutValue:
  case StRef:
  case StrictlyIntegerConv:
  case StringIsset:
  case StStk:
  case SubDbl:
  case SubInt:
  case SubIntO:
  case SyncReturnBC:
  case UnboxPtr:
  case Unreachable:
  case UnwindCheckSideExit:
  case VectorDoCow:
  case VectorHasImmCopy:
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
