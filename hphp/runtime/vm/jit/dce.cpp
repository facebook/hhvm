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

#include "hphp/runtime/vm/jit/dce.h"

#include <array>
#include <folly/MapUtil.h>

#include "hphp/util/low-ptr.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP::jit {

TRACE_SET_MOD(hhir_dce);

bool canDCE(const IRInstruction& inst) {
  switch (inst.op()) {
  case AssertNonNull:
  case AssertType:
  case AbsDbl:
  case AddInt:
  case SubInt:
  case MulInt:
  case AndInt:
  case AddDbl:
  case SubDbl:
  case MulDbl:
  case Sqrt:
  case OrInt:
  case XorInt:
  case Shl:
  case Shr:
  case Lshr:
  case Floor:
  case Ceil:
  case XorBool:
  case Mod:
  case ConvDblToBool:
  case ConvIntToBool:
  case ConvStrToBool:
  case ConvBoolToDbl:
  case ConvIntToDbl:
  case ConvStrToDbl:
  case ConvResToDbl:
  case ConvBoolToInt:
  case ConvDblToInt:
  case ConvStrToInt:
  case ConvResToInt:
  case ConvDblToStr:
  case ConvIntToStr:
  case DblAsBits:
  case ConvPtrToLval:
  case NewColFromArray:
  case GtInt:
  case GteInt:
  case LtInt:
  case LteInt:
  case EqInt:
  case NeqInt:
  case CmpInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case LteDbl:
  case EqDbl:
  case NeqDbl:
  case CmpDbl:
  case GtStr:
  case GteStr:
  case LtStr:
  case LteStr:
  case SameStr:
  case NSameStr:
  case CmpStr:
  case GtBool:
  case GteBool:
  case LtBool:
  case LteBool:
  case EqBool:
  case NeqBool:
  case CmpBool:
  case SameObj:
  case NSameObj:
  case EqRes:
  case NeqRes:
  case EqCls:
  case EqLazyCls:
  case EqFunc:
  case EqStrPtr:
  case EqArrayDataPtr:
  case FuncHasReifiedGenerics:
  case ClassHasReifiedGenerics:
  case HasReifiedParent:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case ExtendsClass:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InterfaceSupportsArrLike:
  case InterfaceSupportsStr:
  case InterfaceSupportsInt:
  case InterfaceSupportsDbl:
  case HasToString:
  case IsType:
  case IsNType:
  case IsTypeMem:
  case IsNTypeMem:
  case IsLegacyArrLike:
  case IsWaitHandle:
  case IsCol:
  case LdStk:
  case LdLoc:
  case LdLocForeign:
  case LdStkAddr:
  case LdLocAddr:
  case LdRDSAddr:
  case LdMem:
  case LdContField:
  case LdClsInitElem:
  case LdIterBase:
  case LdIterPos:
  case LdIterEnd:
  case LdFrameThis:
  case LdFrameCls:
  case LdSmashable:
  case LdSmashableFunc:
  case LdClsFromClsMeth:
  case LdFuncFromClsMeth:
  case LdClsFromRClsMeth:
  case LdFuncFromRClsMeth:
  case LdGenericsFromRClsMeth:
  case LdFuncFromRFunc:
  case LdGenericsFromRFunc:
  case LdTVFromRDS:
  case ConvFuncPrologueFlagsToARFlags:
  case DefConst:
  case Conjure:
  case LdClsInitData:
  case LookupClsRDS:
  case LdClsMethodCacheCls:
  case LdFuncVecLen:
  case LdClsMethod:
  case LdSubClsCns:
  case LdIfaceMethod:
  case LdPropAddr:
  case LdObjClass:
  case LdObjInvoke:
  case LdClsName:
  case LdLazyClsName:
  case LdLazyCls:
  case LdFuncCls:
  case LdFuncInOutBits:
  case LdFuncNumParams:
  case LdFuncName:
  case LdMethCallerName:
  case LdStrLen:
  case LdMonotypeDictTombstones:
  case LdMonotypeDictKey:
  case LdMonotypeDictVal:
  case LdMonotypeVecElem:
  case LdVecElem:
  case LdVecElemAddr:
  case NewInstanceRaw:
  case NewLoggingArray:
  case NewDictArray:
  case NewCol:
  case NewPair:
  case NewRFunc:
  case NewRClsMeth:
  case LdRetVal:
  case Mov:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case CountCollection:
  case Nop:
  case AKExistsDict:
  case AKExistsKeyset:
  case LdBindAddr:
  case LdSSwitchDest:
  case LdClosureCls:
  case LdClosureThis:
  case CreateSSWH:
  case LdContActRec:
  case LdContArValue:
  case LdContArKey:
  case LdWHState:
  case LdWHResult:
  case LdWHNotDone:
  case LdAFWHActRec:
  case LdMIStateTempBaseAddr:
  case StringIsset:
  case ColIsEmpty:
  case ColIsNEmpty:
  case LdUnwinderValue:
  case LdColVec:
  case LdColDict:
  case OrdStr:
  case ChrInt:
  case CheckRange:
  case LdMBase:
  case MethodExists:
  case LdTVAux:
  case DictGetQuiet:
  case DictGetK:
  case DictIsset:
  case DictIdx:
  case KeysetGetQuiet:
  case KeysetGetK:
  case KeysetIsset:
  case KeysetIdx:
  case VecFirst:
  case VecLast:
  case DictFirst:
  case DictFirstKey:
  case DictLast:
  case DictLastKey:
  case KeysetFirst:
  case KeysetLast:
  case GetTime:
  case GetTimeNs:
  case Select:
  case LdARFlags:
  case FuncHasAttr:
  case ClassHasAttr:
  case IsFunReifiedGenericsMatched:
  case LdFuncRequiredCoeffects:
  case LdARFunc:
  case StrictlyIntegerConv:
  case GetMemoKeyScalar:
  case LookupSPropSlot:
  case ConstructClosure:
  case AllocBespokeStructDict:
  case AllocStructDict:
  case AllocVec:
  case GetDictPtrIter:
  case GetVecPtrIter:
  case AdvanceDictPtrIter:
  case AdvanceVecPtrIter:
  case LdPtrIterKey:
  case LdPtrIterVal:
  case EqPtrIter:
  case LdUnitPerRequestFilepath:
  case DirFromFilepath:
  case BespokeGet:
  case BespokeIterFirstPos:
  case BespokeIterLastPos:
  case BespokeIterEnd:
  case BespokeIterGetKey:
  case BespokeIterGetVal:
  case LoadBCSP:
  case LdResolvedTypeCnsNoCheck:
  case LdResolvedTypeCnsClsName:
  case LdClsCtxCns:
  case AllocInitROM:
  case VoidPtrAsDataType:
  case CopyArray:
  case StructDictElemAddr:
  case StructDictSlotInPos:
  case LdStructDictKey:
  case LdStructDictVal:
  case LdImplicitContext:
  case LdImplicitContextMemoKey:
  case CallViolatesModuleBoundary:
    assertx(!inst.isControlFlow());
    return true;

  // These may raise oom, but its still ok to delete them if the
  // result is unused
  case ConcatIntStr:
  case ConcatStrInt:
  case ConcatStrStr:
  case ConcatStr3:
  case ConcatStr4:
  case AddNewElemKeyset:
  case AddNewElemVec:
    return true;

  // Some of these conversion functions can run arbitrary PHP code.
  case ConvObjToDbl:
  case ConvTVToDbl:
  case ConvObjToInt:
  case ConvTVToInt:
  case ConvTVToBool:
  case ConvObjToBool:
  case ConvObjToStr:
  case ConvTVToStr:
  case ConvArrLikeToVec:
  case ConvObjToVec:
  case ConvArrLikeToDict:
  case ConvObjToDict:
  case ConvArrLikeToKeyset:
  case ConvObjToKeyset:
  case LdOutAddr:
    return !opcodeMayRaise(inst.op()) &&
      (!inst.consumesReferences() || inst.producesReference());

  case DbgTraceCall:
  case AKExistsObj:
  case StStk:
  case StStkMeta:
  case StStkRange:
  case StOutValue:
  case CheckIter:
  case CheckType:
  case CheckNullptr:
  case CheckTypeMem:
  case CheckDictKeys:
  case CheckSmashableClass:
  case CheckLoc:
  case CheckStk:
  case CheckMBase:
  case AssertLoc:
  case AssertStk:
  case AssertMBase:
  case CheckInit:
  case CheckInitMem:
  case CheckCold:
  case EndGuards:
  case CheckNonNull:
  case DivDbl:
  case DivInt:
  case AddOffset:
  case MulIntO:

  case GtObj:
  case GteObj:
  case LtObj:
  case LteObj:
  case EqObj:
  case NeqObj:
  case CmpObj:
  case CmpRes:
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case GtArrLike:
  case GteArrLike:
  case LtArrLike:
  case LteArrLike:
  case CmpArrLike:
  case JmpZero:
  case JmpNZero:
  case JmpSSwitchDest:
  case JmpSwitchDest:
  case ProfileSwitchDest:
  case CheckSurpriseFlags:
  case CheckSurpriseAndStack:
  case HandleRequestSurprise:
  case ReturnHook:
  case SuspendHookAwaitEF:
  case SuspendHookAwaitEG:
  case SuspendHookAwaitR:
  case SuspendHookCreateCont:
  case SuspendHookYield:
  case EndBlock:
  case Unreachable:
  case Jmp:
  case DefLabel:
  case LdPairElem:
  case LdClsCtor:
  case LdCls:
  case LdClsCached:
  case LdClsCachedSafe:
  case LdCns:
  case LdTypeCns:
  case LdTypeCnsNoThrow:
  case LdTypeCnsClsName:
  case IsTypeStructCached:
  case LookupCnsE:
  case LdClsCns:
  case InitClsCns:
  case InitSubClsCns:
  case LdResolvedTypeCns:
  case CheckSubClsCns:
  case LdClsCnsVecLen:
  case LookupClsMethodFCache:
  case LookupClsMethodCache:
  case LookupClsMethod:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdInitRDSAddr:
  case LdInitPropAddr:
  case LdObjMethodD:
  case LdObjMethodS:
  case LdFunc:
  case LdFuncCached:
  case LookupFuncCached:
  case AllocObj:
  case NewClsMeth:
  case FuncCred:
  case InitProps:
  case PropTypeRedefineCheck:
  case InitSProps:
  case InitObjProps:
  case InitObjMemoSlots:
  case LockObj:
  case DebugBacktrace:
  case InitThrowableFileAndLine:
  case ConstructInstance:
  case InitDictElem:
  case InitStructElem:
  case InitStructPositions:
  case InitVecElem:
  case InitVecElemLoop:
  case NewKeysetArray:
  case NewStructDict:
  case NewBespokeStructDict:
  case Clone:
  case InlineCall:
  case Call:
  case CallFuncEntry:
  case NativeImpl:
  case CallBuiltin:
  case RetCtrl:
  case AsyncFuncRet:
  case AsyncFuncRetPrefetch:
  case AsyncFuncRetSlow:
  case AsyncGenRetR:
  case AsyncGenYieldR:
  case AsyncSwitchFast:
  case GenericRetDecRefs:
  case StClsInitElem:
  case StImplicitContext:
  case StMem:
  case StMemMeta:
  case StIterBase:
  case StIterType:
  case StIterEnd:
  case StIterPos:
  case StLoc:
  case StLocMeta:
  case StLocRange:
  case StPtrAt:
  case StVMFP:
  case StVMSP:
  case StVMPC:
  case StVMReturnAddr:
  case StVMRegState:
  case StTVInRDS:
  case StTypeAt:
  case ReqBindJmp:
  case ReqInterpBBNoTranslate:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case IncRef:
  case DecRef:
  case DecRefNZ:
  case ProfileDecRef:
  case ReleaseShallow:
  case DecReleaseCheck:
  case DefFP:
  case DefFuncEntryFP:
  case DefFrameRelSP:
  case DefRegSP:
  case DefFuncPrologueCallee:
  case DefFuncPrologueCtx:
  case DefFuncPrologueFlags:
  case DefFuncPrologueNumArgs:
  case InitFrame:
  case Count:
  case VerifyParam:
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamCoerce:
  case VerifyParamFail:
  case VerifyParamFailHard:
  case VerifyReifiedLocalType:
  case VerifyReifiedReturnType:
  case VerifyRet:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetCoerce:
  case VerifyRetFail:
  case VerifyRetFailHard:
  case VerifyProp:
  case VerifyPropAll:
  case VerifyPropCls:
  case VerifyPropCoerce:
  case VerifyPropCoerceAll:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case ThrowUninitLoc:
  case ThrowUndefPropException:
  case RaiseTooManyArg:
  case RaiseError:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case RaiseWarning:
  case RaiseNotice:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseCoeffectsCallViolation:
  case RaiseCoeffectsFunParamTypeViolation:
  case RaiseCoeffectsFunParamCoeffectRulesViolation:
  case RaiseStrToClassNotice:
  case RaiseModuleBoundaryViolation:
  case RaiseModulePropertyViolation:
  case RaiseImplicitContextStateInvalid:
  case CheckClsMethFunc:
  case CheckClsReifiedGenericMismatch:
  case CheckClsRGSoft:
  case CheckFunReifiedGenericMismatch:
  case CheckInOutMismatch:
  case CheckReadonlyMismatch:
  case PrintStr:
  case PrintInt:
  case PrintBool:
  case GetMemoKey:
  case InterpOne:
  case InterpOneCF:
  case OODeclExists:
  case StClosureArg:
  case CreateGen:
  case CreateAGen:
  case CreateAAWH:
  case CreateAFWH:
  case CreateAGWH:
  case AFWHPrepareChild:
  case StArResumeAddr:
  case ContEnter:
  case ContCheckNext:
  case ContValid:
  case ContArIncKey:
  case ContArIncIdx:
  case ContArUpdateIdx:
  case LdContResumeAddr:
  case StContArState:
  case StContArValue:
  case StContArKey:
  case AFWHBlockOn:
  case AFWHPushTailFrame:
  case CountWHNotDone:
  case IncStat:
  case IncProfCounter:
  case IncCallCounter:
  case DbgAssertRefCount:
  case DbgAssertFunc:
  case DbgCheckLocalsDecRefd:
  case RBTraceEntry:
  case RBTraceMsg:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case IterInit:
  case IterInitK:
  case LIterInit:
  case LIterInitK:
  case IterNext:
  case IterNextK:
  case LIterNext:
  case LIterNextK:
  case IterFree:
  case KillActRec:
  case KillIter:
  case KillLoc:
  case BaseG:
  case PropX:
  case PropQ:
  case PropDX:
  case CGetProp:
  case CGetPropQ:
  case SetProp:
  case UnsetProp:
  case SetOpProp:
  case IncDecProp:
  case IssetProp:
  case ElemX:
  case CheckMissingKeyInArrLike:
  case CheckArrayCOW:
  case ProfileDictAccess:
  case CheckDictOffset:
  case ProfileKeysetAccess:
  case CheckKeysetOffset:
  case ProfileArrayCOW:
  case ElemDictD:
  case ElemDictU:
  case ElemDictK:
  case ElemDX:
  case ElemUX:
  case DictGet:
  case KeysetGet:
  case StringGet:
  case OrdStrIdx:
  case MapGet:
  case CGetElem:
  case DictSet:
  case MapSet:
  case VectorSet:
  case BespokeSet:
  case BespokeUnset:
  case StructDictUnset:
  case BespokeAppend:
  case SetElem:
  case SetRange:
  case SetRangeRev:
  case UnsetElem:
  case SetOpElem:
  case IncDecElem:
  case SetNewElem:
  case SetNewElemDict:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case ReserveVecNewElem:
  case VectorIsset:
  case PairIsset:
  case MapIsset:
  case IssetElem:
  case ProfileType:
  case ProfileCall:
  case ProfileMethod:
  case ProfileSubClsCns:
  case ProfileGlobal:
  case ProfileCoeffectFunParam:
  case CheckVecBounds:
  case BespokeElem:
  case BespokeEscalateToVanilla:
  case BespokeGetThrow:
  case LdTypeStructureVal:
  case LdTypeStructureValCns:
  case LdVectorSize:
  case BeginCatch:
  case EndCatch:
  case EnterTCUnwind:
  case UnwindCheckSideExit:
  case DbgTrashStk:
  case DbgTrashFrame:
  case DbgTrashMem:
  case EnterPrologue:
  case ExitPrologue:
  case EnterTranslation:
  case CheckStackOverflow:
  case CheckSurpriseFlagsEnter:
  case JmpPlaceholder:
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
  case ThrowInOutMismatch:
  case ThrowReadonlyMismatch:
  case ThrowCannotModifyReadonlyCollection:
  case ThrowLocalMustBeValueTypeException:
  case ThrowMustBeEnclosedInReadonly:
  case ThrowMustBeMutableException:
  case ThrowMustBeReadonlyException:
  case ThrowMustBeValueTypeException:
  case StMBase:
  case StMROProp:
  case CheckMROProp:
  case FinishMemberOp:
  case BeginInlining:
  case EndInlining:
  case SetOpTV:
  case OutlineSetOp:
  case ConjureUse:
  case LdClsMethodFCacheFunc:
  case LdClsMethodCacheFunc:
  case LogArrayReach:
  case LogGuardFailure:
  case ProfileInstanceCheck:
  case MemoGetStaticValue:
  case MemoGetStaticCache:
  case MemoGetLSBValue:
  case MemoGetLSBCache:
  case MemoGetInstanceValue:
  case MemoGetInstanceCache:
  case MemoSetStaticValue:
  case MemoSetStaticCache:
  case MemoSetLSBValue:
  case MemoSetLSBCache:
  case MemoSetInstanceValue:
  case MemoSetInstanceCache:
  case ThrowAsTypeStructException:
  case RecordReifiedGenericsAndGetTSList:
  case ResolveTypeStruct:
  case CheckRDSInitialized:
  case MarkRDSInitialized:
  case MarkRDSAccess:
  case ProfileProp:
  case ProfileIsTypeStruct:
  case StFrameCtx:
  case StFrameFunc:
  case StFrameMeta:
  case LookupClsCns:
  case LookupClsCtxCns:
  case ArrayMarkLegacyShallow:
  case ArrayMarkLegacyRecursive:
  case ArrayUnmarkLegacyShallow:
  case ArrayUnmarkLegacyRecursive:
  case ProfileArrLikeProps:
  case CheckFuncNeedsCoverage:
  case RecordFuncCall:
  case StructDictSlot:
  case StructDictAddNextSlot:
  case StructDictTypeBoundCheck:
  case LdCoeffectFunParamNaive:
  case DeserializeLazyProp:
  case GetClsRGProp:
    return false;

  case IsTypeStruct:
  case EqStr:
  case NeqStr:
    return !opcodeMayRaise(inst.op());

  case EqArrLike:
  case NeqArrLike:
  case SameArrLike:
  case NSameArrLike:
    return !inst.mayRaiseErrorWithSources();
  }
  not_reached();
}

namespace {

/* DceFlags tracks the state of one instruction during dead code analysis. */
struct DceFlags {
  DceFlags()
    : m_state(DEAD)
  {}

  bool isDead() const { return m_state == DEAD; }
  void setDead()      { m_state = DEAD; }
  void setLive()      { m_state = LIVE; }

  std::string toString() const {
    std::array<const char*,2> const names = {{
      "DEAD",
      "LIVE",
    }};
    return folly::format(
      "{}",
      m_state < names.size() ? names[m_state] : "<invalid>"
    ).str();
  }

private:
  enum {
    DEAD = 0,
    LIVE,
  };
  uint8_t m_state:1;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->id().
using DceState = StateVector<IRInstruction, DceFlags>;
using UseCounts = StateVector<SSATmp, uint32_t>;
// Set of live DefLabel operands (keyed by DefLabel and index)
using DefLabelLiveness = jit::fast_set<std::pair<IRInstruction*, size_t>>;
// Worklist is instruction to process. If the instruction is a
// DefLabel, the second item is the index of the operand (DefLabel is
// treated separately for each operand).
using WorkList = jit::vector<std::pair<IRInstruction*, size_t>>;

void removeDeadInstructions(IRUnit& unit,
                            const DceState& state,
                            const DefLabelLiveness& defLabelLive) {
  postorderWalk(
    unit,
    [&](Block* block) {
      auto const next = block->next();
      auto const bcctx = block->back().bcctx();
      block->remove_if(
        [&] (const IRInstruction& inst) {
          // Don't attempt to remove a Jmp feeding a DefLabel. It will
          // be dealt with below.
          if (inst.is(Jmp) && inst.numSrcs() > 0) return false;

          // DecReleaseCheck will be dealt with below.
          if (inst.is(DecReleaseCheck)) return false;

          ONTRACE(
            4,
            if (state[inst].isDead()) {
              FTRACE(1, "Removing dead instruction {}\n", inst.toString());
            }
          );

          auto const dead = state[inst].isDead();
          assertx(!dead || !inst.taken() || inst.taken()->isCatch());
          return dead;
        }
      );

      if (!block->empty()) {
        auto& front = block->front();
        if (front.is(DefLabel)) {
          // If the DefLabel was completely dead, it would have been
          // removed above.
          assertx(!state[&front].isDead());
          // Remove any dead individual operands
          auto dstIdx = 0;
          auto const numDsts = front.numDsts();
          for (auto i = 0; i < numDsts; ++i) {
            if (!defLabelLive.count(std::make_pair(&front, i))) {
              FTRACE(1, "Removing dead DefLabel dst {}: {}\n",
                     i, front.toString());
              front.deleteDst(dstIdx);
            } else {
              ++dstIdx;
            }
          }
          assertx(front.numDsts() > 0);
        }

        auto& back = block->back();
        if (back.is(Jmp) && back.numSrcs() > 0) {
          if (state[&back].isDead()) {
            // If the Jmp's operands are completely dead, we can't
            // remove it (because it's control flow), but we can turn
            // it into a normal Jmp.
            FTRACE(1, "Removing dead instruction {}\n", back.toString());
            back.setSrcs(0, nullptr);
          } else {
            // Otherwise, similarily to the DefLabel, remove
            // individual dead operands.
            auto srcIdx = 0;
            auto const numSrcs = back.numSrcs();
            for (auto i = 0; i < numSrcs; ++i) {
              auto& defLabel = block->taken()->front();
              assertx(defLabel.is(DefLabel));
              if (!defLabelLive.count(std::make_pair(&defLabel, i))) {
                FTRACE(1, "Removing dead Jmp src {}: {}\n",
                       i, back.toString());
                back.deleteSrc(srcIdx);
              } else {
                ++srcIdx;
              }
            }
          }
        }

        if (back.is(DecReleaseCheck) && state[&back].isDead()) {
          block->erase(block->backIter());
          block->push_back(unit.gen(Jmp, bcctx, next));
        }
      }

      if (block->empty() || !block->back().isBlockEnd()) {
        assertx(next);
        block->push_back(unit.gen(Jmp, bcctx, next));
      }
    }
  );
}

// removeUnreachable erases unreachable blocks from unit, and returns
// a sorted list of the remaining blocks.
BlockList prepareBlocks(IRUnit& unit) {
  FTRACE(1, "RemoveUnreachable:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "RemoveUnreachable:^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const blocks = rpoSortCfg(unit);

  // 1. perform copy propagation on every instruction
  for (auto block : blocks) {
    for (auto& inst : *block) {
      copyProp(&inst);
    }
  }

  // 2. erase unreachable blocks and get an rpo sorted list of what remains.
  bool needsReflow = removeUnreachable(unit);

  // 3. if we removed any whole blocks that ended in Jmp instructions, reflow
  //    all types in case they change the incoming types of DefLabel
  //    instructions.
  if (needsReflow) reflowTypes(unit);

  return blocks;
}

WorkList initInstructions(const IRUnit& unit, const BlockList& blocks,
                          DceState& state) {
  TRACE(1, "DCE(initInstructions):vvvvvvvvvvvvvvvvvvvv\n");
  // Mark reachable, essential, instructions live and enqueue them.
  WorkList wl;
  wl.reserve(unit.numInsts());
  forEachInst(blocks, [&] (IRInstruction* inst) {
    // Instructions that cannot be removed are automatically live. The
    // exception is DefLabels and Jmps that feed a DefLabel. There
    // aren't normally DCEable, but will be dealt with specially.
    if (!canDCE(*inst) &&
        !inst->is(DefLabel) &&
        !(inst->is(Jmp) && inst->numSrcs() > 0)) {
      state[inst].setLive();
      wl.push_back(std::make_pair(inst, 0));
    }
  });
  TRACE(1, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
  return wl;
}

//////////////////////////////////////////////////////////////////////

struct TrackedInstr {
  // DecRefs which refer to the tracked instruction.
  jit::vector<IRInstruction*> decs;

  // Auxiliary instructions which must be killed if the tracked instruction is
  // killed.
  jit::vector<IRInstruction*> aux;

  // Stores to the stack in catch traces that can be killed to kill the
  // tracked instruction.
  jit::vector<IRInstruction*> stores;
};

void processCatchBlock(IRUnit& unit, DceState& state, Block* block,
                       const UseCounts& uses,
                       jit::fast_map<IRInstruction*, TrackedInstr>& rcInsts) {
  assertx(block->front().is(BeginCatch));
  assertx(block->back().is(EndCatch));

  auto constexpr numTrackedSlots = 64;
  auto constexpr wholeRange = std::make_pair(0, numTrackedSlots);
  auto const stackTop = block->back().extra<EndCatch>()->offset;
  auto const stackRange = [&] {
    // If the catch block occurs within an inlined frame the outer stack
    // locations (those above the inlined frame) are not dead and cannot be
    // elided as we may not throw through the outer callers.
    if (block->back().src(0)->inst()->is(BeginInlining)) {
      auto const extra = block->back().src(0)->inst()->extra<BeginInlining>();
      auto const spOff = extra->spOffset;
      assertx(stackTop <= spOff);
      if (spOff < stackTop + numTrackedSlots) {
        return AStack::range(stackTop, spOff);
      }
    }
    return AStack::range(stackTop, stackTop + numTrackedSlots);
  }();

  // Nothing to optimize if the stack is empty
  if (stackRange.size() == 0) return;

  std::bitset<numTrackedSlots> usedLocations = {};
  // stores that are only read by the EndCatch
  jit::fast_set<IRInstruction*> candidateStores;
  // Any IncRefs we see; if they correspond to stores above, we can
  // replace the store with a store of Null, and kill the IncRef.
  jit::fast_map<SSATmp*, std::vector<Block::iterator>> candidateIncRefs;

  auto const range =
    [&] (const AliasClass& cls) -> std::pair<int, int> {
      if (!cls.maybe(stackRange)) return {};
      auto const stk = cls.stack();
      if (!stk) return wholeRange;
      auto const lowest_upper = std::min(stackRange.high, stk->high);
      auto const highest_lower = std::max(stackRange.low, stk->low);
      if (lowest_upper <= highest_lower) return {};
      return {
        highest_lower.offset - stackRange.low.offset,
        lowest_upper.offset - stackRange.low.offset
      };
    };

  auto const process_stack =
    [&] (const AliasClass& cls) {
      auto r = range(cls);
      if (r == wholeRange) {
        usedLocations.set();
      } else {
        while (r.first < r.second) {
          usedLocations.set(r.first++);
        }
      }
      return false;
    };

  auto const do_store =
    [&] (const AliasClass& cls, IRInstruction* store) {
      if (!store->is(StStk, StStkMeta)) return false;
      auto const stk = cls.is_stack();
      if (!stk) return process_stack(cls);
      auto const r = range(cls);
      if (r.first != r.second) {
        assertx(r.second == r.first + 1);
        if (!usedLocations.test(r.first)) {
          usedLocations.set(r.first);
          candidateStores.insert(store);
        }
      }
      return false;
    };

  auto done = false;
  for (auto inst = block->end(); inst != block->begin(); ) {
    --inst;
    if (inst->is(EndCatch)) {
      continue;
    }
    if (inst->is(IncRef)) {
      candidateIncRefs[inst->src(0)].push_back(inst);
      continue;
    }
    if (done) continue;
    auto const effects = canonicalize(memory_effects(*inst));
    done = match<bool>(
      effects,
      [&] (IrrelevantEffects)    { return false; },
      [&] (UnknownEffects)       { return true; },
      [&] (ReturnEffects x)      { return true; },
      [&] (CallEffects x)        { return true; },
      [&] (GeneralEffects x)     {
        return
          process_stack(x.loads) ||
          process_stack(x.inout) ||
          process_stack(x.stores) ||
          process_stack(x.backtrace) ||
          process_stack(x.kills);
      },
      [&] (PureLoad x)           { return process_stack(x.src); },
      [&] (PureStore x)          { return do_store(x.dst, &*inst); },
      [&] (ExitEffects x)        { return process_stack(x.live); },
      [&] (PureInlineCall x) {
        return
          process_stack(x.base) ||
          process_stack(x.actrec);
      }
    );
  }

  for (auto store : candidateStores) {
    auto const src = store->src(1);
    auto const it = candidateIncRefs.find(src);
    if (it != candidateIncRefs.end()) {
      FTRACE(3, "Erasing {} for {}\n",
             it->second.back()->toString(), store->toString());
      block->erase(it->second.back());
      if (it->second.size() > 1) {
        it->second.pop_back();
      } else {
        candidateIncRefs.erase(it);
      }
    } else if (src->type().maybe(TCounted)) {
      auto const srcInst = src->inst();
      if (!srcInst->producesReference() ||
          !canDCE(*srcInst) ||
          uses[src] != 1) {
        if (srcInst->producesReference() && canDCE(*srcInst)) {
          rcInsts[srcInst].stores.emplace_back(store);
        }
        continue;
      } else {
        FTRACE(3, "Erasing {} for {}\n",
               srcInst->toString(), store->toString());
        state[srcInst].setDead();
      }
    }
    store->setSrc(1, unit.cns(TInitNull));
  }
}

/*
 * A store to the stack which is post-dominated by the EndCatch and
 * not otherwise read is only there to ensure the unwinder DecRefs the
 * value it contains. If there's also an IncRef of the value in the
 * catch trace we can just store InitNull to the stack location and
 * drop the IncRef (and later, maybe adjust the sp of the
 * catch-trace's owner so we don't even have to do the store).
 */
void optimizeCatchBlocks(const BlockList& blocks,
                         DceState& state,
                         IRUnit& unit,
                         const UseCounts& uses,
                         jit::fast_map<IRInstruction*, TrackedInstr>& rcInsts) {
  FTRACE(1, "OptimizeCatchBlocks:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "OptimizeCatchBlocks:^^^^^^^^^^^^^^^^^^^^\n"); };
  for (auto block : blocks) {
    if (block->back().is(EndCatch) &&
        block->back().extra<EndCatch>()->mode !=
          EndCatchData::CatchMode::SideExit &&
        block->front().is(BeginCatch)) {
      processCatchBlock(unit, state, block, uses, rcInsts);
    }
  }
}

void optimizeConcats(jit::vector<IRInstruction*>& concats,
                     DceState& state,
                     IRUnit& unit,
                     UseCounts& uses,
                     jit::fast_map<IRInstruction*, TrackedInstr>& rcInsts) {
  FTRACE(1, "OptimizeConcats:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "OptimizeConcats:^^^^^^^^^^^^^^^^^^^^\n"); };
  auto const incref = [&] (auto inst, auto src) {
    auto const blk = inst->block();
    auto const ins = unit.gen(IncRef, inst->bcctx(), src);
    blk->insert(blk->iteratorTo(inst), ins);
    state[ins].setLive();
    ++uses[src];
    FTRACE(3, "Adding {}\n", ins->toString());
  };
  auto const decref = [&] (auto inst, auto src) {
    auto const blk = inst->block();
    auto const ins = unit.gen(DecRef, inst->bcctx(), DecRefData{}, src);
    blk->insert(blk->iteratorTo(inst), ins);
    state[ins].setLive();
    ++uses[src];
    FTRACE(3, "Adding {}\n", ins->toString());
  };
  auto const combine = [&] (auto inst, auto inst_prev,
                            auto src1, auto src2, auto src3) {
    /*
     * ~~ Converting ~~
     * t1 = ConcatStrStr a b (implicit Decref a)
     * DecRef b
     * t2 = ConcatStrStr t1 c (implicit Decref t1)
     * DecRef c
     *
     * to
     *
     * IncRef a
     * IncRef b
     * t1 = ConcatStrStr a b (implicit Decref a)
     * DecRef b
     * t2 = ConcatStr3 a b c (implicit Decref a)
     * DecRef b
     * DecRef c

     * ~~ Converting ~~
     * t1 = ConcatStrStr a b (implicit Decref a)
     * DecRef b
     * t2 = ConcatStrStr c t1 (implicit Decref c)
     * DecRef t1
     *
     * to
     *
     * IncRef a
     * IncRef b
     * t1 = ConcatStrStr a b (implicit Decref a)
     * DecRef b
     * t2 = ConcatStr3 c a b (implicit Decref c)
     * DecRef a
     * DecRef b
     * DecRef t1
     *
     * We need to make sure refcounts are correct. We do this by increffing
     * the sources of the first ConcatStrStr and then decreffing them after
     * ConcatStr3 unless ConcatStr3 already consumes the reference
     *
     * Note that later stages of DCE will kill the extra ConcatStrStr and the
     * refcounting.
     */
    assertx(inst_prev->is(ConcatStrStr));
    assertx(inst->is(ConcatStrStr));
    if (uses[inst_prev->dst()] == 1 + rcInsts[inst_prev].decs.size() +
                                      rcInsts[inst_prev].stores.size()) {
      FTRACE(3, "Combining {} into {}",
             inst_prev->toString(), inst->toString());
      auto next = inst->next();
      unit.replace(inst, ConcatStr3, inst->taken(), src1, src2, src3);
      inst->setNext(next);
      FTRACE(3, " and got {}\n", inst->toString());
      state[inst].setLive();
      --uses[inst_prev->dst()];
      ++uses[inst_prev->src(0)];
      ++uses[inst_prev->src(1)];
      // Incref the first source since the first ConcatStrStr controls
      // its refcount
      incref(inst_prev, inst_prev->src(0));
      incref(inst_prev, inst_prev->src(1));
      // ConcatStr3 ends the blocks, so insert the decrefs to the next block
      assertx(inst->next() && !inst->next()->empty());
      decref(&inst->next()->front(), src2);
      if (src3 == inst_prev->src(1)) decref(&inst->next()->front(), src3);
    }
  };

  for (auto& inst : concats) {
    if (!inst->is(ConcatStrStr)) continue;
    auto const src1 = inst->src(0);
    auto const src2 = inst->src(1);
    if (src1->inst()->is(ConcatStrStr)) {
      combine(inst, src1->inst(),
              src1->inst()->src(0), src1->inst()->src(1), src2);
    } else if (src2->inst()->is(ConcatStrStr)) {
      combine(inst, src2->inst(),
              src1, src2->inst()->src(0), src2->inst()->src(1));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void killInstrAdjustRC(
  DceState& state,
  IRUnit& unit,
  IRInstruction* inst,
  jit::vector<IRInstruction*>& decs
) {
  auto const insertDecRef = [&] (auto before, auto src) {
    auto const blk = before->block();
    auto const ins = unit.gen(DecRef, before->bcctx(), DecRefData{}, src);
    blk->insert(blk->iteratorTo(before), ins);
    FTRACE(3, "Inserting {} before {} for {}\n",
           ins->toString(), before->toString(), inst->toString());
    state[ins].setLive();
  };
  auto anyRemaining = false;
  if (inst->consumesReferences()) {
    // ConsumesReference inputs that are definitely not moved can
    // simply be decreffed as a replacement for the dead consumesref
    // instruction
    auto srcIx = 0;
    for (auto src : inst->srcs()) {
      auto const ix = srcIx++;
      if (inst->consumesReference(ix) && src->type().maybe(TCounted)) {
        if (inst->mayMoveReference(ix)) {
          anyRemaining = true;
          continue;
        }
        insertDecRef(inst, src);
      }
    }
  }
  for (auto dec : decs) {
    auto replaced = dec->src(0) != inst->dst();
    auto srcIx = 0;
    if (dec->is(DecReleaseCheck)) {
      if (inst->is(ConstructClosure) && inst->src(0)->type().maybe(TCounted)) {
        assertx(!replaced);
        assertx(anyRemaining);
        assertx(inst->mayMoveReference(0));

        // While the closure is going to be released via ReleaseShallow it will
        // still be responsible for releasing the captured context.
        insertDecRef(dec, inst->src(0));
      }
    } else if (anyRemaining) {
      assertx(dec->is(DecRef));
      // The remaining inputs might be moved, so may need to survive
      // until this instruction is decreffed
      for (auto src : inst->srcs()) {
        if (inst->mayMoveReference(srcIx++) && src->type().maybe(TCounted)) {
          if (!replaced) {
            FTRACE(3, "Converting {} to ", dec->toString());
            dec->setSrc(0, src);
            FTRACE(3, "{} for {}\n", dec->toString(), inst->toString());
            replaced = true;
            state[dec].setLive();
          } else {
            insertDecRef(dec, src);
          }
        }
      }
    }
    if (!replaced) {
      FTRACE(3, "Killing {} for {}\n", dec->toString(), inst->toString());
      state[dec].setDead();
    }
  }
  state[inst].setDead();
}

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

void mandatoryDCE(IRUnit& unit) {
  if (removeUnreachable(unit)) {
    // Removing unreachable incoming edges can change types, so if we changed
    // anything we have to reflow to maintain that IR invariant.
    reflowTypes(unit);
  }
  assertx(checkEverything(unit));
}

void fullDCE(IRUnit& unit) {
  if (!RuntimeOption::EvalHHIRDeadCodeElim) {
    // This portion of DCE cannot be turned off, because it restores IR
    // invariants, and callers of fullDCE are allowed to rely on it for that.
    return mandatoryDCE(unit);
  }

  Timer dceTimer(Timer::optimize_dce);

  // kill unreachable code and remove any traces that are now empty
  auto const blocks = prepareBlocks(unit);

  // At this point, all IR invariants must hold, because we've restored the
  // only one allowed to be violated before fullDCE in prepareBlocks.
  assertx(checkEverything(unit));

  // mark the essential instructions and add them to the initial
  // work list; this will also mark reachable exit traces. All
  // other instructions marked dead.
  DceState state(unit, DceFlags());
  WorkList wl = initInstructions(unit, blocks, state);

  UseCounts uses(unit, 0);
  jit::fast_map<IRInstruction*, TrackedInstr> rcInsts;
  DefLabelLiveness defLabelLive;
  jit::vector<IRInstruction*> concats;

  // process the worklist
  while (!wl.empty()) {
    auto const [inst, defLabelIdx] = wl.back();
    wl.pop_back();
    assertx(!state[inst].isDead());
    // Jmps which feed a DefLabel are dealt with as part of processing
    // the DefLabel. They should never appear on the worklist.
    assertx(IMPLIES(inst->is(Jmp), inst->numSrcs() == 0));
    assertx(IMPLIES(!inst->is(DefLabel), defLabelIdx == 0));

    auto const process = [&] (IRInstruction* inst, uint32_t ix) {
      if (inst->is(ConcatStrStr)) concats.emplace_back(inst);
      auto const src = inst->src(ix);
      IRInstruction* srcInst = src->inst();
      if (srcInst->op() == DefConst) return;

      if (srcInst->producesReference() && canDCE(*srcInst)) {
        ++uses[src];
        if (inst->is(DecRef, DecReleaseCheck)) {
          rcInsts[srcInst].decs.emplace_back(inst);
        }
        if (inst->is(InitVecElem, InitDictElem, InitStructElem,
                     InitStructPositions, ReleaseShallow, StClosureArg)) {
          if (ix == 0) rcInsts[srcInst].aux.emplace_back(inst);
        }
      }

      if (srcInst->is(DefLabel)) {
        // If the source instruction is a DefLabel, figure out which
        // operand this SSATmp corresponds to, and schedule that
        // particular operand.
        auto dstIdx = 0;
        for (; dstIdx < srcInst->numDsts(); dstIdx++) {
          if (src == srcInst->dst(dstIdx)) break;
        }
        assertx(dstIdx != srcInst->numDsts());
        // If the operand isn't already live, schedule it.
        if (defLabelLive.emplace(std::make_pair(srcInst, dstIdx)).second) {
          state[srcInst].setLive();
          wl.push_back(std::make_pair(srcInst, dstIdx));
        }
      } else if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(std::make_pair(srcInst, 0));
      }
    };

    if (inst->is(DefLabel)) {
      // For a DefLabel operand, look "through" each corresponding Jmp
      // and process the source as if we were processing that Jmp.
      assertx(defLabelIdx < inst->numDsts());
      assertx(defLabelLive.count(std::make_pair(inst, defLabelIdx)));
      inst->block()->forEachPred(
        [&, inst = inst, defLabelIdx = defLabelIdx] (Block* pred) {
          auto& jmp = pred->back();
          assertx(jmp.is(Jmp));
          assertx(jmp.numSrcs() == inst->numDsts());
          state[&jmp].setLive();
          process(&jmp, defLabelIdx);
        }
      );
      continue;
    }
    // For a normal instruction, just process each source
    for (uint32_t ix = 0; ix < inst->numSrcs(); ++ix) process(inst, ix);
  }

  optimizeCatchBlocks(blocks, state, unit, uses, rcInsts);
  optimizeConcats(concats, state, unit, uses, rcInsts);

  // If every use of a dce-able PRc instruction is a DecRef or PureStore based
  // on its dst, then we can kill it, and DecRef any of its consumesReference
  // inputs.
  for (auto& pair : rcInsts) {
    auto& info = pair.second;
    auto const trackedUses =
      info.decs.size() + info.aux.size() + info.stores.size();
    if (uses[pair.first->dst()] != trackedUses) continue;
    killInstrAdjustRC(state, unit, pair.first, info.decs);
    for (auto inst : info.aux) killInstrAdjustRC(state, unit, inst, info.decs);
    for (auto store : info.stores) store->setSrc(1, unit.cns(TInitNull));
  }

  // Now remove instructions whose state is DEAD.
  removeDeadInstructions(unit, state, defLabelLive);

  // Kill unreachable catch blocks.
  mandatoryDCE(unit);
}

}
