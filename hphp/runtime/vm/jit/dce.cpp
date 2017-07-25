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

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
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
  case Floor:
  case Ceil:
  case XorBool:
  case Mod:
  case ConvBoolToArr:
  case ConvDblToArr:
  case ConvIntToArr:
  case ConvArrToBool:
  case ConvDblToBool:
  case ConvIntToBool:
  case ConvStrToBool:
  case ConvObjToBool:
  case ConvCellToBool:
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
  case ConvClsToCctx:
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
  case SameArr:
  case NSameArr:
  case SameVec:
  case NSameVec:
  case SameDict:
  case NSameDict:
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
  case IsScalarType:
  case IsWaitHandle:
  case IsCol:
  case UnboxPtr:
  case LdStk:
  case LdLoc:
  case LdClsRef:
  case LdStkAddr:
  case LdLocAddr:
  case LdRDSAddr:
  case LdMem:
  case LdContField:
  case LdElem:
  case LdRef:
  case LdCtx:
  case LdCctx:
  case LdClosure:
  case LdClsCtx:
  case LdClsCctx:
  case FwdCtxStaticCall:
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
  case LdARFuncPtr:
  case LdARNumParams:
  case LdFuncNumParams:
  case LdStrLen:
  case LdVecElem:
  case LdPackedArrayDataElemAddr:
  case LdClosureStaticLoc:
  case NewInstanceRaw:
  case NewArray:
  case NewMixedArray:
  case NewDictArray:
  case NewLikeArray:
  case NewCol:
  case NewPair:
  case FreeActRec:
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
  case AKExistsDict:
  case AKExistsKeyset:
  case LdBindAddr:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdSSwitchDestFast:
  case LdClosureCtx:
  case CreateSSWH:
  case LdContActRec:
  case LdContArValue:
  case LdContArKey:
  case LdAsyncArParentChain:
  case LdWHState:
  case LdWHResult:
  case LdAFWHActRec:
  case LdResumableArObj:
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
  case LdARInvName:
  case PackMagicArgs:
  case LdMBase:
  case MethodExists:
  case LdTVAux:
  case DictGetQuiet:
  case DictGetK:
  case DictIsset:
  case DictEmptyElem:
  case DictIdx:
  case KeysetGetQuiet:
  case KeysetGetK:
  case KeysetIsset:
  case KeysetEmptyElem:
  case KeysetIdx:
  case GetTime:
  case Select:
  case MemoGet:
  case LdARCtx:
  case LdCufIterFunc:
  case LdCufIterCtx:
  case LdCufIterInvName:
  case LdStaticLoc:
  case StrictlyIntegerConv:
    assertx(!inst->isControlFlow());
    return true;

  case DbgTraceCall:
  case AKExistsObj:
  case StStk:
  case SpillFrame:
  case CheckType:
  case CheckNullptr:
  case CheckTypeMem:
  case HintLocInner:
  case HintStkInner:
  case HintMBaseInner:
  case CheckLoc:
  case CheckStk:
  case CheckMBase:
  case AssertLoc:
  case AssertStk:
  case AssertMBase:
  case CastStk:
  case CastMem:
  case CoerceStk:
  case CoerceMem:
  case CoerceCellToBool:
  case CoerceCellToInt:
  case CoerceStrToInt:
  case CoerceCellToDbl:
  case CoerceStrToDbl:
  case CheckInit:
  case CheckInitMem:
  case CheckCold:
  case CheckRefs:
  case EndGuards:
  case CheckNonNull:
  case DivDbl:
  case DivInt:
  case AddIntO:
  case SubIntO:
  case MulIntO:

    // These conversion functions either can run arbitrary PHP code, or decref
    // their inputs.
  case ConvObjToArr:
  case ConvCellToArr:
  case ConvStrToArr:
  case ConvVecToArr:
  case ConvDictToArr:
  case ConvKeysetToArr:
  case ConvObjToDbl:
  case ConvCellToDbl:
  case ConvObjToInt:
  case ConvCellToInt:
  case ConvCellToObj:
  case ConvObjToStr:
  case ConvResToStr:
  case ConvCellToStr:
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
  case ConvObjToDArr:

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
  case ReturnHook:
  case SuspendHookE:
  case SuspendHookR:
  case EndBlock:
  case Unreachable:
  case Jmp:
  case DefLabel:
  case Box:
  case LdLocPseudoMain:
  case LdVectorBase:
  case LdPairBase:
  case CheckRefInner:
  case CheckCtxThis:
  case CheckFuncStatic:
  case DefCls:
  case LdClsCtor:
  case LdCls:
  case LdClsCached:
  case LdClsCachedSafe:
  case LdCns:
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case LdClsCns:
  case InitClsCns:
  case LdSubClsCns:
  case CheckSubClsCns:
  case LdClsCnsVecLen:
  case LookupClsMethodFCache:
  case LookupClsMethodCache:
  case LookupClsMethod:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case LdObjMethod:
  case LdObjInvoke:
  case LdArrFuncCtx:
  case LdArrFPushCuf:
  case LdStrFPushCuf:
  case LdFunc:
  case LdFuncCached:
  case LdFuncCachedU:
  case LdFuncCachedSafe:
  case AllocObj:
  case RegisterLiveObj:
  case CheckInitProps:
  case InitProps:
  case CheckInitSProps:
  case InitSProps:
  case InitObjProps:
  case DebugBacktrace:
  case DebugBacktraceFast:
  case InitThrowableFileAndLine:
  case ConstructInstance:
  case AllocPackedArray:
  case AllocVecArray:
  case InitPackedLayoutArray:
  case InitPackedLayoutArrayLoop:
  case NewKeysetArray:
  case NewStructArray:
  case Clone:
  case InlineReturn:
  case CallArray:
  case Call:
  case NativeImpl:
  case CallBuiltin:
  case RetCtrl:
  case AsyncRetCtrl:
  case ReleaseVVAndSkip:
  case GenericRetDecRefs:
  case StMem:
  case StElem:
  case StLoc:
  case StLocPseudoMain:
  case StLocRange:
  case StClsRef:
  case StRef:
  case EagerSyncVMRegs:
  case ReqBindJmp:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case IncRef:
  case DecRef:
  case DecRefNZ:
  case FuncGuard:
  case DefFP:
  case DefSP:
  case Count:
  case VerifyParamCls:
  case VerifyParamCallable:
  case VerifyParamFail:
  case VerifyParamFailHard:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
  case VerifyRetFailHard:
  case RaiseUninitLoc:
  case RaiseUndefProp:
  case RaiseMissingArg:
  case RaiseError:
  case RaiseWarning:
  case RaiseMissingThis:
  case FatalMissingThis:
  case RaiseNotice:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case RaiseVarEnvDynCall:
  case RaiseHackArrCompatNotice:
  case InitStaticLoc:
  case PrintStr:
  case PrintInt:
  case PrintBool:
  case ConcatIntStr:
  case ConcatStrInt:
  case ConcatStrStr:
  case ConcatStr3:
  case ConcatStr4:
  case AddElemStrKey:
  case AddElemIntKey:
  case AddNewElem:
  case DictAddElemStrKey:
  case DictAddElemIntKey:
  case ArrayAdd:
  case GetMemoKey:
  case LdSwitchObjIndex:
  case LdSSwitchDestSlow:
  case InterpOne:
  case InterpOneCF:
  case OODeclExists:
  case StClosureCtx:
  case StClosureArg:
  case CreateGen:
  case CreateAGen:
  case CreateAFWH:
  case CreateAFWHNoVV:
  case AFWHPrepareChild:
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
  case StContArResume:
  case StContArValue:
  case StContArKey:
  case StAsyncArSucceeded:
  case StAsyncArResume:
  case StAsyncArResult:
  case AFWHBlockOn:
  case AsyncRetFast:
  case AsyncSwitchFast:
  case ABCUnblock:
  case IncStat:
  case IncStatGrouped:
  case IncProfCounter:
  case DbgAssertRefCount:
  case DbgAssertARFunc:
  case RBTraceEntry:
  case RBTraceMsg:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case IterInit:
  case IterInitK:
  case WIterInit:
  case WIterInitK:
  case MIterInit:
  case MIterInitK:
  case IterNext:
  case IterNextK:
  case WIterNext:
  case WIterNextK:
  case MIterNext:
  case MIterNextK:
  case IterFree:
  case MIterFree:
  case DecodeCufIter:
  case StCufIterFunc:
  case StCufIterCtx:
  case StCufIterInvName:
  case BaseG:
  case PropX:
  case PropQ:
  case PropDX:
  case CGetProp:
  case CGetPropQ:
  case VGetProp:
  case BindProp:
  case SetProp:
  case UnsetProp:
  case SetOpProp:
  case IncDecProp:
  case EmptyProp:
  case IssetProp:
  case ElemX:
  case ProfileMixedArrayOffset:
  case CheckMixedArrayOffset:
  case CheckArrayCOW:
  case ProfileDictOffset:
  case CheckDictOffset:
  case ProfileKeysetOffset:
  case CheckKeysetOffset:
  case ElemArray:
  case ElemArrayD:
  case ElemArrayW:
  case ElemArrayU:
  case ElemMixedArrayK:
  case ElemVecD:
  case ElemVecU:
  case ElemDict:
  case ElemDictD:
  case ElemDictW:
  case ElemDictU:
  case ElemDictK:
  case ElemKeyset:
  case ElemKeysetW:
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
  case VGetElem:
  case BindElem:
  case ArraySet:
  case ArraySetRef:
  case VecSet:
  case VecSetRef:
  case DictSet:
  case DictSetRef:
  case MapSet:
  case SetElem:
  case SetWithRefElem:
  case UnsetElem:
  case SetOpElem:
  case IncDecElem:
  case SetNewElem:
  case SetNewElemArray:
  case SetNewElemVec:
  case SetNewElemKeyset:
  case ReservePackedArrayDataNewElem:
  case BindNewElem:
  case VectorIsset:
  case PairIsset:
  case MapIsset:
  case IssetElem:
  case EmptyElem:
  case ProfileArrayKind:
  case ProfileType:
  case ProfileMethod:
  case ProfileSubClsCns:
  case CheckPackedArrayDataBounds:
  case LdVectorSize:
  case VectorDoCow:
  case VectorHasImmCopy:
  case BeginCatch:
  case EndCatch:
  case UnwindCheckSideExit:
  case DbgTrashStk:
  case DbgTrashFrame:
  case DbgTrashMem:
  case DbgTrashRetVal:
  case EnterFrame:
  case CheckStackOverflow:
  case InitExtraArgs:
  case InitCtx:
  case CheckSurpriseFlagsEnter:
  case CheckARMagicFlag:
  case LdARNumArgsAndFlags:
  case StARNumArgsAndFlags:
  case StARInvName:
  case ExitPlaceholder:
  case ThrowOutOfBounds:
  case ThrowInvalidArrayKey:
  case ThrowInvalidOperation:
  case ThrowArithmeticError:
  case ThrowDivisionByZeroError:
  case StMBase:
  case FinishMemberOp:
  case InlineReturnNoFrame:
  case BeginInlining:
  case SyncReturnBC:
  case SetOpCell:
  case ConjureUse:
  case CheckStaticLoc:
  case LdClsMethodFCacheFunc:
  case LdClsMethodCacheFunc:
  case ProfileInstanceCheck:
  case MemoSet:
  case KillClsRef:
  case KillCufIter:
  case BoxPtr:
    return false;

  case AKExistsArr:
  case ArrayIsset:
  case ArrayIdx:
    return !RuntimeOption::EvalHackArrCompatNotices;
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
typedef jit::vector<const IRInstruction*> WorkList;

void removeDeadInstructions(IRUnit& unit, const DceState& state) {
  postorderWalk(unit, [&](Block* block) {
    block->remove_if([&] (const IRInstruction& inst) {
      ONTRACE(4,
              if (state[inst].isDead()) {
                FTRACE(1, "Removing dead instruction {}\n", inst.toString());
              });

      // For now, all control flow instructions are essential. If we ever
      // change this, we'll need to be careful about unlinking dead CF
      // instructions here.
      assertx(IMPLIES(inst.isControlFlow(), !state[inst].isDead()));
      return state[inst].isDead();
    });
  });
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
    case StLoc:
    case LdLoc:
    case CheckLoc:
    case AssertLoc:
    case LdLocAddr:
    case HintLocInner:
    // these can be rewritten to use an outer frame pointer
    case LdClsRef:
    case StClsRef:
    case KillClsRef:
      incWeak(inst, inst->src(0));
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
IRSPRelOffset locToStkOff(IRInstruction& inst) {
  assertx(inst.is(LdLoc, StLoc, LdLocAddr, AssertLoc, CheckLoc, HintLocInner));

  auto locId = inst.extra<LocalId>()->locId;
  auto fpInst = inst.src(0)->inst();
  assertx(fpInst->is(DefInlineFP));

  return fpInst->extra<DefInlineFP>()->spOffset - locId - 1;
}

/*
 * Convert an instruction using the frame pointer into one that uses the
 * caller's frame pointer, skipping frames marked as dead.
 */
template <typename F>
void rewriteToParentFrameImpl(IRUnit& /*unit*/, IRInstruction& inst, F dead) {
  assertx(inst.is(LdClsRef, StClsRef, KillClsRef));

  auto fp = inst.src(0);
  assertx(canonical(fp)->inst()->is(DefInlineFP, DefLabel));

  auto const chaseFpTmp = [](const SSATmp* s) {
    s = canonical(s);
    auto i = s->inst();
    if (UNLIKELY(i->is(DefLabel))) {
      i = resolveFpDefLabel(s);
      assertx(i);
    }
    always_assert(i->is(DefFP, DefInlineFP));
    return i->dst();
  };

  fp = chaseFpTmp(fp);
  assertx(fp->inst()->is(DefInlineFP));

  // Figure out the FPInvOffset of the stack pointer from the outermost frame
  // pointer. We'll use this to find the offsets of the various frame pointers.
  auto const spOffsetFromTop = [&]{
    auto const sp = fp->inst()->src(0);
    auto const defSp = sp->inst();
    assertx(defSp->is(DefSP));
    return defSp->extra<DefSP>()->offset;
  }();

  // Given a frame pointer, determine its offset from the outermost frame
  // pointer in the unit.
  auto const getFpOffsetFromTop = [&](const SSATmp* s) {
    auto const i = s->inst();
    if (i->is(DefFP)) return FPInvOffset{0};
    if (i->is(DefInlineFP)) {
      return i->extra<DefInlineFP>()->spOffset.to<FPInvOffset>(spOffsetFromTop);
    }
    always_assert(false);
  };

  // Given a frame pointer, determine the associated Func*.
  auto const getFunc = [](const SSATmp* s) {
    auto const i = s->inst();
    if (i->is(DefFP)) return i->func();
    if (i->is(DefInlineFP)) return i->extra<DefInlineFP>()->target;
    always_assert(false);
  };

  auto const slot = inst.extra<ClsRefSlotData>()->slot;
  auto const origFpOffset = getFpOffsetFromTop(fp);
  auto const origFunc = getFunc(fp);

  // Walk up the def/use chain of the frame pointers, stopping if we encounter
  // the outermost frame pointer, or if we find an inlined frame which is not
  // dead. This will be the frame pointer we rewrite the instruction to.
  do {
    fp = chaseFpTmp(fp->inst()->src(1));
  } while (!fp->inst()->is(DefFP) && dead(fp->inst()));

  // Calculate the new offset (in bytes) that should be used to calculate the
  // new slot. Take the difference between the original frame pointer offset and
  // the new frame pointer offset. This is in slots, so multiple by the slot
  // size. Add in the space between the original frame pointer to the original
  // slot, and subtract out the space between the new frame pointer and the
  // first slot. (frame_clsref_offset returns negative numbers, hence the
  // reversed operations).

  /*
   *  --------------------------------
   *  |   ActRec                     |
   *  -------------------------------- fp <---------------------|
   *  |   ..................         |                          |
   *  |   ..................         |                          |
   *  |   ..................         |                          |
   *  --------------------------------                          |
   *  |   ActRec                     |                          |
   *  -------------------------------- origFp <-|               |- new offset
   *  |   Locals + Iterators         |          |               |
   *  --------------------------------          |               |
   *  |   Class-ref slots #0 -> N-1  |          |- orig offset  |
   *  --------------------------------          |               |
   *  |   Class-ref slot #N          |          |               |
   *  -------------------------------- <--------- <--------------
   */

  auto const newOffset =
    cellsToBytes(origFpOffset - getFpOffsetFromTop(fp))
    - frame_clsref_offset(origFunc, slot)
    + frame_clsref_offset(getFunc(fp), 0);
  assertx((newOffset % sizeof(LowPtr<Class>)) == 0);
  // Now that we have the new offset in bytes, convert it to an actual slot
  // number.
  auto const newSlot = newOffset / sizeof(LowPtr<Class>);

  // Sanity check that both the before and after result in the same byte offset
  if (debug) {
    DEBUG_ONLY auto const origOffset =
      cellsToBytes(origFpOffset.offset)
      - frame_clsref_offset(origFunc, slot);
    DEBUG_ONLY auto const newOffset =
      cellsToBytes(getFpOffsetFromTop(fp).offset)
      - frame_clsref_offset(getFunc(fp), newSlot);
    assertx(origOffset == newOffset);
  }

  ITRACE(3, "rewriting {} to use frame-ptr {} with slot {}\n",
         inst, *fp, newSlot);

  // Update the instruction:
  inst.setSrc(0, fp);
  switch (inst.op()) {
    case LdClsRef:   inst.extra<LdClsRef>()->slot = newSlot; break;
    case StClsRef:   inst.extra<StClsRef>()->slot = newSlot; break;
    case KillClsRef: inst.extra<KillClsRef>()->slot = newSlot; break;
    default: not_reached();
  }
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

  for (auto block : blocks) {
    ITRACE(2, "Visiting block {}\n", block->id());
    Trace::Indent indenter;

    for (auto& inst : *block) {
      ITRACE(5, "{}\n", inst.toString());

      if (auto const fp = inst.marker().fp()) {
        if (state[fp->inst()].isDead()) {
          always_assert(fp->inst()->is(DefInlineFP));
          auto const prev = fp->inst()->src(1);
          inst.marker() = inst.marker().adjustFP(prev);
          assertx(!state[prev->inst()].isDead());
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
      case HintLocInner:
        if (state[inst.src(0)->inst()].isDead()) {
          convertToStackInst(unit, inst);
        }
        break;

      case LdClsRef:
      case StClsRef:
      case KillClsRef:
        if (state[inst.src(0)->inst()].isDead()) {
          rewriteToParentFrameImpl(
            unit,
            inst,
            [&](const IRInstruction* i){ return state[i].isDead(); }
          );
        }
        break;

      /*
       * DecRef* are special: they're the only instructions that can reenter
       * but not throw. This means it's safe to elide their inlined frame, as
       * long as we adjust their markers to a depth that is guaranteed to not
       * stomp on the caller's frame if it reenters.
       */
      case DecRef:
        if (inst.marker().func() != outerFunc) {
          ITRACE(3, "pushing stack depth of {} to {}\n", safeDepth, inst);
          inst.marker() = inst.marker().adjustSP(FPInvOffset{safeDepth});
        }
        break;

      default:
        break;
      }
    }
  }
}

/*
 * Look for InlineReturn instructions that are the only "non-weak" use
 * of a DefInlineFP.  In this case we can kill both, which may allow
 * removing a SpillFrame as well.
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
  jit::flat_set<const IRInstruction*>& visited
) {
  auto const inst = fp->inst();
  assertx(inst->is(DefLabel));

  // We already examined this, avoid loops.
  if (visited.count(inst)) return nullptr;

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
  visited.insert(inst);
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

} // anonymous namespace

IRInstruction* resolveFpDefLabel(const SSATmp* fp) {
  jit::flat_set<const IRInstruction*> visited;
  return resolveFpDefLabelImpl(fp, visited);
}

void convertToStackInst(IRUnit& unit, IRInstruction& inst) {
  assertx(inst.is(CheckLoc, AssertLoc, LdLoc, StLoc, LdLocAddr, HintLocInner));
  assertx(inst.src(0)->inst()->is(DefInlineFP));

  auto const data = IRSPRelOffsetData { locToStkOff(inst) };
  auto const mainSP = unit.mainSP();

  switch (inst.op()) {
    case StLoc:
      unit.replace(&inst, StStk, data, mainSP, inst.src(1));
      return;
    case LdLoc:
      unit.replace(&inst, LdStk, data, inst.typeParam(), mainSP);
      return;
    case LdLocAddr:
      unit.replace(&inst, LdStkAddr, data, mainSP);
      retypeDests(&inst, &unit);
      return;
    case AssertLoc:
      unit.replace(&inst, AssertStk, data, inst.typeParam(), mainSP);
      return;
    case CheckLoc: {
      auto next = inst.next();
      unit.replace(&inst, CheckStk, data, inst.typeParam(),
                   inst.taken(), mainSP);
      inst.setNext(next);
      return;
    }
    case HintLocInner:
      unit.replace(&inst, HintStkInner, data, inst.typeParam(), mainSP);
      return;

    default: break;
  }
  not_reached();
}

void rewriteToParentFrame(IRUnit& unit, IRInstruction& inst) {
  rewriteToParentFrameImpl(
    unit, inst, [&](const IRInstruction*) { return false; }
  );
}

void convertToInlineReturnNoFrame(IRUnit& unit, IRInstruction& inst) {
  assertx(inst.is(InlineReturn));
  auto const frameInst = inst.src(0)->inst();
  auto const spInst = frameInst->src(0)->inst();

  auto const calleeAROff = frameInst->extra<DefInlineFP>()->spOffset;
  auto const spOff = spInst->extra<DefSP>()->offset;

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
  UseCounts uses(unit, 0);
  WorkList wl = initInstructions(unit, blocks, state);

  // process the worklist
  while (!wl.empty()) {
    auto* inst = wl.back();
    wl.pop_back();
    for (auto src : inst->srcs()) {
      IRInstruction* srcInst = src->inst();
      if (srcInst->op() == DefConst) continue;

      if (RuntimeOption::EvalHHIRInlineFrameOpts) {
        if (srcInst->is(DefInlineFP)) {
          FTRACE(3, "adding use to {} from {}\n", *src, *inst);
          ++uses[src];
        }
      }

      if (state[srcInst].isDead()) {
        state[srcInst].setLive();
        wl.push_back(srcInst);
      }
    }
  }

  if (RuntimeOption::EvalHHIRInlineFrameOpts) {
    optimizeActRecs(blocks, state, unit, uses);
  }

  // Now remove instructions whose state is DEAD.
  removeDeadInstructions(unit, state);
}

}}
