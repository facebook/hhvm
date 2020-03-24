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

namespace HPHP { namespace jit {
namespace {

TRACE_SET_MOD(hhir_dce);

bool canDCE(IRInstruction* inst) {
  switch (inst->op()) {
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
  case ConvBoolToArr:
  case ConvDblToArr:
  case ConvIntToArr:
  case ConvFuncToArr:
  case ConvArrToBool:
  case ConvDblToBool:
  case ConvIntToBool:
  case ConvStrToBool:
  case ConvArrToDbl:
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
  case GtBool:
  case GteBool:
  case LtBool:
  case LteBool:
  case EqBool:
  case NeqBool:
  case CmpBool:
  case SameObj:
  case NSameObj:
  case EqKeyset:
  case NeqKeyset:
  case SameKeyset:
  case NSameKeyset:
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case EqRes:
  case NeqRes:
  case CmpRes:
  case EqRecDesc:
  case EqCls:
  case EqFunc:
  case EqStrPtr:
  case EqArrayDataPtr:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case ExtendsClass:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfRecDesc:
  case InterfaceSupportsArr:
  case InterfaceSupportsVec:
  case InterfaceSupportsDict:
  case InterfaceSupportsKeyset:
  case InterfaceSupportsStr:
  case InterfaceSupportsInt:
  case InterfaceSupportsDbl:
  case HasToString:
  case IsType:
  case IsNType:
  case IsTypeMem:
  case IsNTypeMem:
  case IsWaitHandle:
  case IsCol:
  case LdStk:
  case LdLoc:
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
  case LdRecDesc:
  case DefConst:
  case Conjure:
  case LdClsInitData:
  case LookupClsRDS:
  case LdClsMethodCacheCls:
  case LdFuncVecLen:
  case LdClsMethod:
  case LdIfaceMethod:
  case LdPropAddr:
  case LdObjClass:
  case LdClsName:
  case LdARNumParams:
  case LdFuncCls:
  case LdFuncNumParams:
  case LdFuncName:
  case LdMethCallerName:
  case LdStrLen:
  case LdVecElem:
  case LdPackedElem:
  case LdPackedArrayDataElemAddr:
  case NewInstanceRaw:
  case NewArray:
  case NewMixedArray:
  case NewDArray:
  case NewDictArray:
  case NewLikeArray:
  case NewCol:
  case NewPair:
  case DefCallFlags:
  case DefCallFunc:
  case DefCallNumArgs:
  case DefCallCtx:
  case DefInlineFP:
  case LdRetVal:
  case Mov:
  case CountArray:
  case CountArrayFast:
  case CountVec:
  case CountDict:
  case CountKeyset:
  case CountCollection:
  case Nop:
  case AKExistsArr:
  case AKExistsDict:
  case AKExistsKeyset:
  case LdBindAddr:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdSSwitchDestFast:
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
  case LdMIPropStateAddr:
  case LdMIStateAddr:
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
  case ArrayIdx:
  case ArrayIsset:
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
  case IsFunReifiedGenericsMatched:
  case IsClsDynConstructible:
  case LdFuncRxLevel:
  case StrictlyIntegerConv:
  case SetLegacyDict:
  case SetLegacyVec:
  case GetMemoKeyScalar:
  case LookupSPropSlot:
  case ConstructClosure:
  case AllocPackedArray:
  case AllocStructArray:
  case AllocStructDArray:
  case AllocStructDict:
  case AllocVArray:
  case AllocVecArray:
  case GetMixedPtrIter:
  case GetPackedPtrIter:
  case AdvanceMixedPtrIter:
  case AdvancePackedPtrIter:
  case LdPtrIterKey:
  case LdPtrIterVal:
  case EqPtrIter:
    assertx(!inst->isControlFlow());
    return true;

  // These may raise oom, but its still ok to delete them if the
  // result is unused
  case ConcatIntStr:
  case ConcatStrInt:
  case ConcatStrStr:
  case ConcatStr3:
  case ConcatStr4:
  case AddNewElem:
  case AddNewElemKeyset:
  case AddNewElemVec:
    return true;

  // Some of these conversion functions can run arbitrary PHP code.
  case ConvObjToArr:
  case ConvTVToArr:
  case ConvStrToArr:
  case ConvVecToArr:
  case ConvDictToArr:
  case ConvKeysetToArr:
  case ConvArrToNonDVArr:
  case ConvObjToDbl:
  case ConvTVToDbl:
  case ConvObjToInt:
  case ConvTVToInt:
  case ConvTVToBool:
  case ConvObjToBool:
  case ConvObjToStr:
  case ConvResToStr:
  case ConvTVToStr:
  case ConvArrToVec:
  case ConvDictToVec:
  case ConvKeysetToVec:
  case ConvObjToVec:
  case ConvArrToDict:
  case ConvVecToDict:
  case ConvKeysetToDict:
  case ConvObjToDict:
  case ConvArrToKeyset:
  case ConvVecToKeyset:
  case ConvDictToKeyset:
  case ConvObjToKeyset:
  case ConvArrToVArr:
  case ConvVecToVArr:
  case ConvDictToVArr:
  case ConvKeysetToVArr:
  case ConvObjToVArr:
  case ConvArrToDArr:
  case ConvVecToDArr:
  case ConvDictToDArr:
  case ConvKeysetToDArr:
  case ConvObjToDArr:
  case LdOutAddr:
    return !opcodeMayRaise(inst->op()) &&
      (!inst->consumesReferences() || inst->producesReference());

  case ConvClsMethToArr:
  case ConvClsMethToDArr:
  case ConvClsMethToDict:
  case ConvClsMethToKeyset:
  case ConvClsMethToVArr:
  case ConvClsMethToVec: {
    bool consumeRef = use_lowptr ? false : inst->consumesReferences();
    return !opcodeMayRaise(inst->op()) &&
      (!consumeRef || inst->producesReference());
  }

  case DbgTraceCall:
  case AKExistsObj:
  case StStk:
  case StOutValue:
  case CheckIter:
  case CheckType:
  case CheckNullptr:
  case CheckTypeMem:
  case CheckVArray:
  case CheckDArray:
  case CheckDVArray:
  case CheckMixedArrayKeys:
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
  case CheckInOuts:
  case EndGuards:
  case CheckNonNull:
  case DivDbl:
  case DivInt:
  case AddIntO:
  case SubIntO:
  case MulIntO:

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
  case LdLocPseudoMain:
  case LdPairElem:
  case DefCls:
  case LdClsCtor:
  case LdCls:
  case LdClsCached:
  case LdClsCachedSafe:
  case LdClsTypeCns:
  case LdClsTypeCnsClsName:
  case LdRecDescCached:
  case LdRecDescCachedSafe:
  case LdCns:
  case LookupCnsE:
  case LdClsCns:
  case InitClsCns:
  case LdSubClsCns:
  case LdSubClsCnsClsName:
  case LdTypeCns:
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
  case LdObjInvoke:
  case LdFunc:
  case LdFuncCached:
  case LookupFuncCached:
  case AllocObj:
  case AllocObjReified:
  case NewClsMeth:
  case FuncCred:
  case InitProps:
  case PropTypeRedefineCheck:
  case InitSProps:
  case InitObjProps:
  case InitObjMemoSlots:
  case LockObj:
  case DebugBacktrace:
  case DebugBacktraceFast:
  case InitThrowableFileAndLine:
  case ConstructInstance:
  case InitMixedLayoutArray:
  case InitPackedLayoutArray:
  case InitPackedLayoutArrayLoop:
  case NewKeysetArray:
  case NewRecord:
  case NewRecordArray:
  case NewStructArray:
  case NewStructDArray:
  case NewStructDict:
  case Clone:
  case InlineReturn:
  case InlineSuspend:
  case CallUnpack:
  case Call:
  case NativeImpl:
  case CallBuiltin:
  case RetCtrl:
  case AsyncFuncRet:
  case AsyncFuncRetSlow:
  case AsyncSwitchFast:
  case ReleaseVVAndSkip:
  case GenericRetDecRefs:
  case StClsInitElem:
  case StMem:
  case StIterBase:
  case StIterType:
  case StIterEnd:
  case StIterPos:
  case StLoc:
  case StLocPseudoMain:
  case StLocRange:
  case EagerSyncVMRegs:
  case ReqBindJmp:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case IncRef:
  case DecRef:
  case DecRefNZ:
  case ProfileDecRef:
  case DefFP:
  case DefFuncEntryFP:
  case DefFrameRelSP:
  case DefRegSP:
  case Count:
  case VerifyParamCls:
  case VerifyParamCallable:
  case VerifyParamFail:
  case VerifyParamFailHard:
  case VerifyReifiedLocalType:
  case VerifyReifiedReturnType:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
  case VerifyRetFailHard:
  case VerifyProp:
  case VerifyPropCls:
  case VerifyPropCoerce:
  case VerifyPropFail:
  case VerifyPropFailHard:
  case VerifyParamRecDesc:
  case VerifyRetRecDesc:
  case VerifyPropRecDesc:
  case RaiseClsMethPropConvertNotice:
  case RaiseHackArrParamNotice:
  case RaiseHackArrPropNotice:
  case RaiseUninitLoc:
  case RaiseUndefProp:
  case RaiseTooManyArg:
  case RaiseError:
  case RaiseErrorOnInvalidIsAsExpressionType:
  case RaiseWarning:
  case RaiseNotice:
  case ThrowArrayIndexException:
  case ThrowArrayKeyException:
  case RaiseArraySerializeNotice:
  case RaiseHackArrCompatNotice:
  case RaiseForbiddenDynCall:
  case RaiseForbiddenDynConstruct:
  case RaiseRxCallViolation:
  case RaiseStrToClassNotice:
  case CheckClsReifiedGenericMismatch:
  case CheckFunReifiedGenericMismatch:
  case PrintStr:
  case PrintInt:
  case PrintBool:
  case ArrayAdd:
  case GetMemoKey:
  case LdSwitchObjIndex:
  case LdSSwitchDestSlow:
  case InterpOne:
  case InterpOneCF:
  case OODeclExists:
  case StClosureArg:
  case CreateGen:
  case CreateAGen:
  case CreateAAWH:
  case CreateAFWH:
  case CreateAFWHNoVV:
  case CreateAGWH:
  case AFWHPrepareChild:
  case StArResumeAddr:
  case ContEnter:
  case ContPreNext:
  case ContStartedCheck:
  case ContValid:
  case ContStarted:
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
  case KillIter:
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
  case ProfileMixedArrayAccess:
  case CheckMixedArrayOffset:
  case CheckArrayCOW:
  case ProfileDictAccess:
  case CheckDictOffset:
  case ProfileKeysetAccess:
  case CheckKeysetOffset:
  case ElemArrayX:
  case ElemArrayD:
  case ElemArrayU:
  case ElemMixedArrayK:
  case ElemVecD:
  case ElemVecU:
  case ElemDictX:
  case ElemDictD:
  case ElemDictU:
  case ElemDictK:
  case ElemKeysetX:
  case ElemKeysetU:
  case ElemKeysetK:
  case ElemDX:
  case ElemUX:
  case ArrayGet:
  case MixedArrayGetK:
  case DictGet:
  case KeysetGet:
  case StringGet:
  case OrdStrIdx:
  case MapGet:
  case CGetElem:
  case ArraySet:
  case VecSet:
  case DictSet:
  case MapSet:
  case VectorSet:
  case SetElem:
  case SetRange:
  case SetRangeRev:
  case UnsetElem:
  case SetOpElem:
  case IncDecElem:
  case SetNewElem:
  case SetNewElemArray:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case ReservePackedArrayDataNewElem:
  case VectorIsset:
  case PairIsset:
  case MapIsset:
  case IssetElem:
  case ProfileArrayKind:
  case ProfileType:
  case ProfileCall:
  case ProfileMethod:
  case ProfileSubClsCns:
  case CheckPackedArrayDataBounds:
  case LdVectorSize:
  case BeginCatch:
  case EndCatch:
  case EnterTCUnwind:
  case UnwindCheckSideExit:
  case DbgTrashStk:
  case DbgTrashFrame:
  case DbgTrashMem:
  case DbgTrashRetVal:
  case EnterPrologue:
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
  case ThrowParamInOutMismatch:
  case ThrowParamInOutMismatchRange:
  case StMBase:
  case StMIPropState:
  case FinishMemberOp:
  case InlineReturnNoFrame:
  case BeginInlining:
  case SyncReturnBC:
  case SetOpTV:
  case SetOpTVVerify:
  case ConjureUse:
  case LdClsMethodFCacheFunc:
  case LdClsMethodCacheFunc:
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
  case ProfileProp:
    return false;

  case SameArr:
  case NSameArr:
  case SameVec:
  case NSameVec:
  case SameDict:
  case NSameDict:
    return
      !RuntimeOption::EvalHackArrCompatCheckCompare &&
      !RuntimeOption::EvalHackArrCompatDVCmpNotices;

  case IsTypeStruct:
    return !opcodeMayRaise(IsTypeStruct);
  }
  not_reached();
}

/* DceFlags tracks the state of one instruction during dead code analysis. */
struct DceFlags {
  DceFlags()
    : m_state(DEAD)
    , m_weakUseCount(0)
  {}

  bool isDead() const { return m_state == DEAD; }
  void setDead()      { m_state = DEAD; }
  void setLive()      { m_state = LIVE; }

  /*
   * "Weak" uses are used in optimizeActRecs.
   *
   * If a frame pointer is used for something that can be modified to
   * not be a use as long as the whole frame can go away, we'll track
   * that here.
   */
  void incWeakUse() {
    if (m_weakUseCount + 1 > kMaxWeakUseCount) {
      // Too many weak uses for us to know we can optimize it away.
      return;
    }
    ++m_weakUseCount;
  }

  int32_t weakUseCount() const {
    return m_weakUseCount;
  }

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
  static constexpr uint8_t kMaxWeakUseCount = 0x7f;
  uint8_t m_weakUseCount:7;
};
static_assert(sizeof(DceFlags) == 1, "sizeof(DceFlags) should be 1 byte");

// DCE state indexed by instr->id().
typedef StateVector<IRInstruction, DceFlags> DceState;
typedef StateVector<SSATmp, uint32_t> UseCounts;
typedef jit::vector<IRInstruction*> WorkList;

void removeDeadInstructions(IRUnit& unit, const DceState& state) {
  postorderWalk(
    unit,
    [&](Block* block) {
      auto const next = block->next();
      auto const bcctx = block->back().bcctx();
      block->remove_if(
        [&] (const IRInstruction& inst) {
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
    if (!canDCE(inst)) {
      state[inst].setLive();
      wl.push_back(inst);
    }
  });
  TRACE(1, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
  return wl;
}

//////////////////////////////////////////////////////////////////////

/*
 * A use of an inlined frame that can be modified to work without the
 * frame is called a "weak use" here.  For example, storing to a local
 * on a frame is weak because if no other uses of the frame are
 * keeping it alive (for example a load of that same local), we can
 * just remove the store.
 *
 * This routine counts the weak uses of inlined frames and marks them
 * dead if they have no non-weak uses.  Returns true if any inlined
 * frames were marked dead.
 */
bool findWeakActRecUses(const BlockList& blocks,
                        DceState& state,
                        IRUnit& unit,
                        const UseCounts& uses) {
  bool killedFrames = false;

  auto const incWeak = [&] (const IRInstruction* inst, const SSATmp* src) {
    assertx(src->isA(TFramePtr));
    auto const frameInst = src->inst();
    if (frameInst->op() == DefInlineFP) {
      ITRACE(3, "weak use of {} from {}\n", *frameInst, *inst);
      state[frameInst].incWeakUse();
    }
  };

  forEachInst(blocks, [&] (IRInstruction* inst) {
    if (state[inst].isDead()) return;

    switch (inst->op()) {
    // these can be made stack relative
    case StLoc: {
      auto const id = inst->marker().func()->lookupVarId(s_86metadata.get());
      if (inst->extra<StLoc>()->locId != id) incWeak(inst, inst->src(0));
      break;
    }
    case LdLoc:
    case CheckLoc:
    case AssertLoc:
    case LdLocAddr:
      incWeak(inst, inst->src(0));
      break;

    // These can be made stack relative if they haven't been already.
    case MemoGetStaticCache:
    case MemoSetStaticCache:
    case MemoGetLSBCache:
    case MemoSetLSBCache:
    case MemoGetInstanceCache:
    case MemoSetInstanceCache:
      if (inst->src(0)->isA(TFramePtr)) incWeak(inst, inst->src(0));
      break;

    case InlineReturn:
      {
        auto const frameInst = inst->src(0)->inst();
        assertx(frameInst->is(DefInlineFP));
        auto const frameUses = uses[frameInst->dst()];
        auto const weakUses  = state[frameInst].weakUseCount();
        /*
         * We can kill the frame if all uses of the frame are counted
         * as weak uses.  Note that this InlineReturn counts as a weak
         * use, but we haven't incremented for it yet, which is where
         * the "+ 1" comes from below.
         */
        ITRACE(2, "frame {}: weak/strong {}/{}\n",
          *frameInst, weakUses, frameUses);
        if (frameUses - (weakUses + 1) == 0) {
          ITRACE(1, "killing frame {}\n", *frameInst);
          killedFrames = true;
          state[frameInst].setDead();

          // Ensure that the frame is still dead for the purposes of
          // memory-effects
          convertToInlineReturnNoFrame(unit, *inst);
        }
      }
      break;

    default:
      // Default is conservative: we don't increment a weak use if it
      // uses the frame (or stack), so they can't be eliminated.
      break;
    }
  });

  return killedFrames;
}

/*
 * Convert a localId in a callee frame into an SP relative offset in the caller
 * frame.
 */
IRSPRelOffset locToStkOff(LocalId locId, const SSATmp* fp) {
  auto const fpInst = fp->inst();
  assertx(fpInst->is(DefInlineFP));
  return fpInst->extra<DefInlineFP>()->spOffset - locId.locId - 1;
}

/*
 * The first time through, we've counted up weak uses of the frame and then
 * finally marked it dead.  The instructions in between that were weak uses may
 * need modifications now that their frame is going away.
 *
 * Also, if we eliminated some frames, DecRef instructions (which can re-enter
 * the VM without requiring a materialized frame) need to have stack depths in
 * their markers adjusted so they can't stomp on parts of the outer function.
 * We handle this conservatively by just pushing all DecRef markers where the
 * DecRef is from a function other than the outer function down to a safe
 * re-entry depth.
 *
 * Finally, any removed frame pointers in BCMarkers must be rewritten to point
 * to the outer frame of that frame.
 */
void performActRecFixups(const BlockList& blocks,
                         DceState& state,
                         IRUnit& unit,
                         const UseCounts& uses) {
  // We limit the total stack depth during inlining, so this is the deepest
  // we'll ever have to worry about.
  auto const outerFunc = blocks.front()->front().marker().func();
  auto const safeDepth = outerFunc->maxStackCells() + kStackCheckLeafPadding;
  ITRACE(3, "safeDepth: {}, outerFunc depth: {}\n",
         safeDepth,
         outerFunc->maxStackCells());

  bool needsReflow = false;

  for (auto block : blocks) {
    ITRACE(2, "Visiting block {}\n", block->id());
    Trace::Indent indenter;

    for (auto& inst : *block) {
      ITRACE(5, "{}\n", inst.toString());

      bool adjustedMarkerFp = false;
      if (auto const fp = inst.marker().fp()) {
        if (state[fp->inst()].isDead()) {
          always_assert(fp->inst()->is(DefInlineFP));
          auto const prev = fp->inst()->src(1);
          inst.marker() = inst.marker().adjustFP(prev);
          assertx(!state[prev->inst()].isDead());
          adjustedMarkerFp = true;
        }
      }

      switch (inst.op()) {
      case DefInlineFP:
        ITRACE(3, "DefInlineFP ({}): weak/strong uses: {}/{}\n",
             inst, state[inst].weakUseCount(), uses[inst.dst()]);
        break;

      case StLoc:
      case LdLoc:
      case LdLocAddr:
      case AssertLoc:
      case CheckLoc:
        if (state[inst.src(0)->inst()].isDead()) {
          convertToStackInst(unit, inst);
          needsReflow = true;
        }
        break;

      /*
       * These are special: they're the only instructions that can reenter
       * but not throw. This means it's safe to elide their inlined frame, as
       * long as we adjust their markers to a depth that is guaranteed to not
       * stomp on the caller's frame if it reenters.
       */
      case DecRef:
      case MemoSetStaticValue:
      case MemoSetLSBValue:
      case MemoSetInstanceValue:
        if (adjustedMarkerFp) {
          ITRACE(3, "pushing stack depth of {} to {}\n", safeDepth, inst);
          inst.marker() = inst.marker().adjustSP(FPInvOffset{safeDepth});
        }
        break;

      case MemoGetStaticCache:
      case MemoGetLSBCache:
      case MemoGetInstanceCache:
        if (inst.src(0)->isA(TFramePtr) &&
            state[inst.src(0)->inst()].isDead()) {
          convertToStackInst(unit, inst);
        }
        break;
      case MemoSetStaticCache:
      case MemoSetLSBCache:
      case MemoSetInstanceCache:
        if (inst.src(0)->isA(TFramePtr) &&
            state[inst.src(0)->inst()].isDead()) {
          // For the same reason as above, we need to adjust the markers for
          // re-entracy.
          convertToStackInst(unit, inst);
          if (adjustedMarkerFp) {
            ITRACE(3, "pushing stack depth of {} to {}\n", safeDepth, inst);
            inst.marker() = inst.marker().adjustSP(FPInvOffset{safeDepth});
          }
        }
        break;

      default:
        break;
      }
    }
  }

  if (needsReflow) reflowTypes(unit);
}

/*
 * Look for InlineReturn instructions that are the only "non-weak" use
 * of a DefInlineFP.  In this case we can kill both, avoiding the ActRec
 * spill.
 *
 * Prior to calling this routine, `uses' should contain the direct
 * (non-transitive) use counts of each DefInlineFP instruction.  If
 * the weak references are equal to the normal references, the
 * instruction is not necessary and can be removed (if we make the
 * required changes to each instruction that used it weakly).
 */
void optimizeActRecs(const BlockList& blocks,
                     DceState& state,
                     IRUnit& unit,
                     const UseCounts& uses) {
  FTRACE(1, "AR:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "AR:^^^^^^^^^^^^^^^^^^^^^\n"); };

  // Make a pass to find if we can kill any of the frames.  If so, we
  // have to do some fixups.  These two routines are coupled---most
  // cases in findWeakActRecUses should have a corresponding case in
  // performActRecFixups to deal with the frame being removed.
  auto const killedFrames = findWeakActRecUses(blocks, state, unit, uses);
  if (killedFrames) {
    ITRACE(1, "Killed some frames. Iterating over blocks for fixups.\n");
    performActRecFixups(blocks, state, unit, uses);
  }
}

IRInstruction* resolveFpDefLabelImpl(
  const SSATmp* fp,
  IdSet<SSATmp>& visited
) {
  auto const inst = fp->inst();
  assertx(inst->is(DefLabel));

  // We already examined this, avoid loops.
  if (visited[fp]) return nullptr;

  auto const dests = inst->dsts();
  auto const destIdx =
    std::find(dests.begin(), dests.end(), fp) - dests.begin();
  always_assert(destIdx >= 0 && destIdx < inst->numDsts());

  // If any of the inputs to the Phi aren't Phis themselves, then just choose
  // that.
  IRInstruction* outInst = nullptr;
  inst->block()->forEachSrc(
    destIdx,
    [&] (const IRInstruction*, const SSATmp* tmp) {
      if (outInst) return;
      auto const i = canonical(tmp)->inst();
      if (!i->is(DefLabel)) outInst = i;
    }
  );
  if (outInst) return outInst;

  // Otherwise we need to recursively look at the linked Phis, avoiding visiting
  // this Phi again.
  visited.add(fp);
  inst->block()->forEachSrc(
    destIdx,
    [&] (const IRInstruction*, const SSATmp* tmp) {
      if (outInst) return;
      tmp = canonical(tmp);
      auto const DEBUG_ONLY label = tmp->inst();
      assertx(label->is(DefLabel));
      outInst = resolveFpDefLabelImpl(tmp, visited);
    }
  );

  return outInst;
}

//////////////////////////////////////////////////////////////////////

void processCatchBlock(IRUnit& unit, DceState& state, Block* block,
                       FPRelOffset stackTop, const UseCounts& uses) {
  using Bits = std::bitset<64>;

  auto const stackSize = (stackTop.offset < -64) ? 64 : -stackTop.offset;
  if (stackSize == 0) return;
  auto const stackBase = stackTop + stackSize;
  // subtract 1 because we want the cells at offsets -1, -2, ... -stackSize
  auto const stackRange = AStack { stackBase - 1, stackSize };

  Bits usedLocations = {};
  // stores that are only read by the EndCatch
  jit::fast_set<IRInstruction*> candidateStores;
  // Any IncRefs we see; if they correspond to stores above, we can
  // replace the store with a store of Null, and kill the IncRef.
  jit::fast_map<SSATmp*, std::vector<Block::iterator>> candidateIncRefs;

  auto const range =
    [&] (const AliasClass& cls) -> std::pair<int, int> {
      if (!cls.maybe(stackRange)) return {};
      auto const stk = cls.stack();
      if (!stk) return { 0, stackSize };
      if (stk->offset < stackTop) {
        auto const delta = stackTop.offset - stk->offset.offset;
        if (delta >= stk->size) return {};
        return { 0, stk->size - delta };
      }
      auto const base = stk->offset.offset - stackTop.offset;
      if (base >= stackSize) return {};
      auto const end = base + stk->size < stackSize ?
        base + stk->size : stackSize;
      return { base, end };
    };

  auto const process_stack =
    [&] (const AliasClass& cls) {
      auto r = range(cls);
      while (r.first < r.second) {
        usedLocations.set(r.first++);
      }
      return false;
    };

  auto const do_store =
    [&] (const AliasClass& cls, IRInstruction* store) {
      if (!store->is(StStk)) return false;
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
          process_stack(x.stores) ||
          process_stack(x.kills);
      },
      [&] (PureLoad x)           { return process_stack(x.src); },
      [&] (PureStore x)          { return do_store(x.dst, &*inst); },
      [&] (ExitEffects x)        { return process_stack(x.live); },
      [&] (InlineEnterEffects x) {
        return
          process_stack(x.inlStack) ||
          process_stack(x.actrec);
      },
      [&] (InlineExitEffects x)  { return process_stack(x.inlStack); }
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
    } else {
      auto const srcInst = src->inst();
      if (!srcInst->producesReference() ||
          !canDCE(srcInst) ||
          uses[src] != 1) {
        continue;
      }
      FTRACE(3, "Erasing {} for {}\n",
             srcInst->toString(), store->toString());
      state[srcInst].setDead();
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
                         const UseCounts& uses) {

  for (auto block : blocks) {
    if (block->back().is(EndCatch) &&
        block->back().extra<EndCatch>()->mode !=
          EndCatchData::CatchMode::SideExit &&
        block->front().is(BeginCatch)) {
      auto const astk = AStack {
        block->back().src(1), block->back().extra<EndCatch>()->offset, 0
      };
      processCatchBlock(unit, state, block, astk.offset, uses);
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
        auto const blk = inst->block();
        auto const ins = unit.gen(DecRef, inst->bcctx(), DecRefData{}, src);
        blk->insert(blk->iteratorTo(inst), ins);
        FTRACE(3, "Inserting {} to replace {}\n",
               ins->toString(), inst->toString());
        state[ins].setLive();
      }
    }
  }
  for (auto dec : decs) {
    auto replaced = dec->src(0) != inst->dst();
    auto srcIx = 0;
    if (anyRemaining) {
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
            auto const blk = dec->block();
            auto const ins = unit.gen(DecRef, dec->bcctx(), DecRefData{}, src);
            blk->insert(blk->iteratorTo(dec), ins);
            FTRACE(3, "Inserting {} before {} for {}\n",
                   ins->toString(), dec->toString(), inst->toString());
            state[ins].setLive();
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

struct TrackedInstr {
  // DecRefs which refer to the tracked instruction.
  jit::vector<IRInstruction*> decs;

  // Auxiliary instructions which must be killed if the tracked instruction is
  // killed.
  jit::vector<IRInstruction*> aux;
};

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

IRInstruction* resolveFpDefLabel(const SSATmp* fp) {
  IdSet<SSATmp> visited;
  auto const fpInst = resolveFpDefLabelImpl(fp, visited);
  assertx(fpInst);
  return fpInst;
}

void convertToStackInst(IRUnit& unit, IRInstruction& inst) {
  assertx(inst.is(CheckLoc, AssertLoc, LdLoc, StLoc, LdLocAddr,
                  MemoGetStaticCache, MemoSetStaticCache,
                  MemoGetLSBCache, MemoSetLSBCache,
                  MemoGetInstanceCache, MemoSetInstanceCache));
  assertx(inst.src(0)->inst()->is(DefInlineFP));

  auto const mainSP = unit.mainSP();

  switch (inst.op()) {
    case StLoc:
      unit.replace(
        &inst,
        StStk,
        IRSPRelOffsetData { locToStkOff(*inst.extra<LocalId>(), inst.src(0)) },
        mainSP,
        inst.src(1)
      );
      return;
    case LdLoc:
      unit.replace(
        &inst,
        LdStk,
        IRSPRelOffsetData { locToStkOff(*inst.extra<LocalId>(), inst.src(0)) },
        inst.typeParam(),
        mainSP
      );
      return;
    case LdLocAddr:
      unit.replace(
        &inst,
        LdStkAddr,
        IRSPRelOffsetData { locToStkOff(*inst.extra<LocalId>(), inst.src(0)) },
        mainSP
      );
      retypeDests(&inst, &unit);
      return;
    case AssertLoc:
      unit.replace(
        &inst,
        AssertStk,
        IRSPRelOffsetData { locToStkOff(*inst.extra<LocalId>(), inst.src(0)) },
        inst.typeParam(),
        mainSP
      );
      return;
    case CheckLoc: {
      auto next = inst.next();
      unit.replace(
        &inst,
        CheckStk,
        IRSPRelOffsetData { locToStkOff(*inst.extra<LocalId>(), inst.src(0)) },
        inst.typeParam(),
        inst.taken(),
        mainSP
      );
      inst.setNext(next);
      return;
    }
    case MemoGetStaticCache:
    case MemoSetStaticCache:
    case MemoGetLSBCache:
    case MemoSetLSBCache: {
      auto& extra = *inst.extra<MemoCacheStaticData>();
      extra.stackOffset = locToStkOff(LocalId{extra.keys.first}, inst.src(0));
      inst.setSrc(0, mainSP);
      return;
    }
    case MemoGetInstanceCache:
    case MemoSetInstanceCache: {
      auto& extra = *inst.extra<MemoCacheInstanceData>();
      extra.stackOffset = locToStkOff(LocalId{extra.keys.first}, inst.src(0));
      inst.setSrc(0, mainSP);
      return;
    }
    default: break;
  }
  not_reached();
}

void convertToInlineReturnNoFrame(IRUnit& unit, IRInstruction& inst) {
  assertx(inst.is(InlineReturn));
  auto const frameInst = inst.src(0)->inst();
  auto const spInst = frameInst->src(0)->inst();
  assertx(spInst->is(DefFrameRelSP, DefRegSP));

  auto const calleeAROff = frameInst->extra<DefInlineFP>()->spOffset;
  auto const spOff = spInst->extra<FPInvOffsetData>()->offset;

  auto const data = FPRelOffsetData {
    // Offset of the callee's return value relative to the frame pointer.
    calleeAROff.to<FPRelOffset>(spOff) + (kArRetOff / sizeof(TypedValue))
  };
  unit.replace(&inst, InlineReturnNoFrame, data);
}

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

  // process the worklist
  while (!wl.empty()) {
    auto* inst = wl.back();
    wl.pop_back();
    for (uint32_t ix = 0; ix < inst->numSrcs(); ++ix) {
      auto const src = inst->src(ix);
      IRInstruction* srcInst = src->inst();
      if (srcInst->op() == DefConst) continue;

      if (RuntimeOption::EvalHHIRInlineFrameOpts) {
        if (srcInst->is(DefInlineFP)) {
          FTRACE(3, "adding use to {} from {}\n", *src, *inst);
          ++uses[src];
        }
      }

      if (srcInst->producesReference() && canDCE(srcInst)) {
        ++uses[src];
        if (inst->is(DecRef)) {
          rcInsts[srcInst].decs.emplace_back(inst);
        }
        if (inst->is(InitPackedLayoutArray, StClosureArg)) {
          if (ix == 0) rcInsts[srcInst].aux.emplace_back(inst);
        }
      }

      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }
    }
  }

  // If every use of a dce-able PRc instruction is a DecRef or PureStore based
  // on its dst, then we can kill it, and DecRef any of its consumesReference
  // inputs.
  for (auto& pair : rcInsts) {
    auto& info = pair.second;
    if (uses[pair.first->dst()] != info.decs.size() + info.aux.size()) continue;
    killInstrAdjustRC(state, unit, pair.first, info.decs);
    for (auto inst : info.aux) killInstrAdjustRC(state, unit, inst, info.decs);
  }

  optimizeCatchBlocks(blocks, state, unit, uses);

  if (RuntimeOption::EvalHHIRInlineFrameOpts) {
    optimizeActRecs(blocks, state, unit, uses);
  }

  // Now remove instructions whose state is DEAD.
  removeDeadInstructions(unit, state);
}

}}
