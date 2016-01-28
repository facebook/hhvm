/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/check.h"

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
  case ConvStrToArr:
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
  case ConvArrToInt:
  case ConvBoolToInt:
  case ConvDblToInt:
  case ConvStrToInt:
  case ConvResToInt:
  case ConvBoolToStr:
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
  case GtRes:
  case GteRes:
  case LtRes:
  case LteRes:
  case EqRes:
  case NeqRes:
  case CmpRes:
  case EqCls:
  case InstanceOf:
  case InstanceOfIface:
  case InstanceOfIfaceVtable:
  case ExtendsClass:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InterfaceSupportsArr:
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
  case BoxPtr:
  case LdStk:
  case LdLoc:
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
  case CastCtxThis:
  case LdClsCtx:
  case LdClsCctx:
  case DefConst:
  case Conjure:
  case LdClsCachedSafe:
  case LdClsInitData:
  case LookupClsRDSHandle:
  case DerefClsRDSHandle:
  case LdCns:
  case LdClsMethodFCacheFunc:
  case GetCtxFwdCallDyn:
  case GetCtxFwdCall:
  case LdClsMethodCacheFunc:
  case LdClsMethodCacheCls:
  case LdClsMethod:
  case LdIfaceMethod:
  case LdPropAddr:
  case LdObjClass:
  case LdClsName:
  case LdFuncCachedSafe:
  case LdARFuncPtr:
  case LdARNumParams:
  case LdFuncNumParams:
  case LdStrLen:
  case LdStaticLocCached:
  case NewInstanceRaw:
  case NewArray:
  case NewMixedArray:
  case NewLikeArray:
  case LdPackedArrayElemAddr:
  case NewCol:
  case FreeActRec:
  case DefInlineFP:
  case Mov:
  case CountArray:
  case CountArrayFast:
  case CountCollection:
  case Nop:
  case AKExistsArr:
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
  case LdColArray:
  case OrdStr:
  case CheckRange:
  case LdARInvName:
  case PackMagicArgs:
  case LdMBase:
  case MethodExists:
  case LdTVAux:
    assertx(!inst->isControlFlow());
    return true;

  case DbgTraceCall:
  case AKExistsObj:
  case StStk:
  case SpillFrame:
  case CufIterSpillFrame:
  case CheckType:
  case CheckNullptr:
  case CheckTypeMem:
  case HintLocInner:
  case CheckLoc:
  case AssertLoc:
  case HintStkInner:
  case CheckStk:
  case AssertStk:
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
  case CheckStaticLocInit:
  case DivDbl:
  case DivInt:
  case AddIntO:
  case SubIntO:
  case MulIntO:
  case ConvObjToArr:
  case ConvCellToArr:
  case ConvObjToDbl:
  case ConvCellToDbl:
  case ConvObjToInt:
  case ConvCellToInt:
  case ConvCellToObj:
  case ConvObjToStr:
  case ConvResToStr:
  case ConvCellToStr:
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
  case Halt:
  case Jmp:
  case DefLabel:
  case Box:
  case LdLocPseudoMain:
  case LdVectorBase:
  case LdPairBase:
  case CheckRefInner:
  case CheckCtxThis:
  case LdClsCtor:
  case LdCls:
  case LdClsCached:
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case LookupClsCns:
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
  case AllocObj:
  case RegisterLiveObj:
  case CheckInitProps:
  case InitProps:
  case CheckInitSProps:
  case InitSProps:
  case InitObjProps:
  case ConstructInstance:
  case AllocPackedArray:
  case InitPackedArray:
  case InitPackedArrayLoop:
  case NewStructArray:
  case Clone:
  case InlineReturn:
  case CallArray:
  case Call:
  case NativeImpl:
  case CallBuiltin:
  case RetCtrl:
  case AsyncRetCtrl:
  case StRetVal:
  case ReleaseVVAndSkip:
  case GenericRetDecRefs:
  case StMem:
  case StElem:
  case StLoc:
  case StLocPseudoMain:
  case StLocRange:
  case StRef:
  case EagerSyncVMRegs:
  case ReqBindJmp:
  case ReqRetranslate:
  case ReqRetranslateOpt:
  case IncRef:
  case IncRefCtx:
  case DecRef:
  case DecRefNZ:
  case DefFP:
  case DefSP:
  case Count:
  case VerifyParamCls:
  case VerifyParamCallable:
  case VerifyParamFail:
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
  case RaiseUninitLoc:
  case RaiseUndefProp:
  case RaiseMissingArg:
  case RaiseError:
  case RaiseWarning:
  case RaiseNotice:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case ClosureStaticLocInit:
  case StaticLocInitCached:
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
  case ArrayAdd:
  case ArrayIdx:
  case GetMemoKey:
  case GenericIdx:
  case LdSwitchObjIndex:
  case LdSSwitchDestSlow:
  case InterpOne:
  case InterpOneCF:
  case OODeclExists:
  case StClosureCtx:
  case StClosureArg:
  case CreateCont:
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
  case ABCUnblock:
  case IncStat:
  case IncTransCounter:
  case IncStatGrouped:
  case IncProfCounter:
  case DbgAssertRefCount:
  case DbgAssertPtr:
  case DbgAssertType:
  case DbgAssertFunc:
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
  case CIterFree:
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
  case ElemArray:
  case ElemArrayD:
  case ElemArrayW:
  case ElemArrayU:
  case ElemDX:
  case ElemUX:
  case ArrayGet:
  case StringGet:
  case MapGet:
  case CGetElem:
  case VGetElem:
  case BindElem:
  case ArraySet:
  case ArraySetRef:
  case MapSet:
  case SetElem:
  case SetWithRefElem:
  case UnsetElem:
  case SetOpElem:
  case IncDecElem:
  case SetNewElem:
  case SetNewElemArray:
  case SetWithRefNewElem:
  case BindNewElem:
  case ArrayIsset:
  case VectorIsset:
  case PairIsset:
  case MapIsset:
  case IssetElem:
  case EmptyElem:
  case ProfilePackedArray:
  case ProfileStructArray:
  case ProfileObjClass:
  case CheckPackedArrayBounds:
  case LdStructArrayElem:
  case LdVectorSize:
  case VectorDoCow:
  case VectorHasImmCopy:
  case ColAddNewElemC:
  case MapAddElemC:
  case BeginCatch:
  case EndCatch:
  case UnwindCheckSideExit:
  case CountBytecode:
  case DbgTrashStk:
  case DbgTrashFrame:
  case DbgTrashMem:
  case EnterFrame:
  case CheckStackOverflow:
  case InitExtraArgs:
  case InitCtx:
  case CheckSurpriseFlagsEnter:
  case CheckARMagicFlag:
  case LdARNumArgsAndFlags:
  case StARNumArgsAndFlags:
  case StTVAux:
  case StARInvName:
  case ExitPlaceholder:
  case ThrowOutOfBounds:
  case ThrowInvalidOperation:
  case ThrowArithmeticError:
  case ThrowDivisionByZeroError:
  case MapIdx:
  case StMBase:
  case FinishMemberOp:
  case InlineReturnNoFrame:
  case BeginInlining:
  case SyncReturnBC:
    return false;
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
IRSPOffset locToStkOff(IRInstruction& inst) {
  assertx(inst.is(LdLoc, StLoc, LdLocAddr, AssertLoc, CheckLoc, HintLocInner));

  auto locId = inst.extra<LocalId>()->locId;
  auto fpInst = inst.src(0)->inst();
  assertx(fpInst->is(DefInlineFP));

  return fpInst->extra<DefInlineFP>()->spOffset - locId - 1;
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

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

void convertToStackInst(IRUnit& unit, IRInstruction& inst) {
  assertx(inst.is(CheckLoc, AssertLoc, LdLoc, StLoc, LdLocAddr, HintLocInner));
  assertx(inst.src(0)->inst()->is(DefInlineFP));
  switch (inst.op()) {
    case StLoc: {
      IRSPOffsetData data {locToStkOff(inst)};
      unit.replace(&inst, StStk, data, unit.mainSP(), inst.src(1));
      return;
    }
    case LdLoc: {
      auto ty = inst.typeParam();
      IRSPOffsetData data {locToStkOff(inst)};
      unit.replace(&inst, LdStk, data, ty, unit.mainSP());
      return;
    }
    case LdLocAddr: {
      IRSPOffsetData data {locToStkOff(inst)};
      unit.replace(&inst, LdStkAddr, data, unit.mainSP());
      retypeDests(&inst, &unit);
      return;
    }
    case AssertLoc: {
      auto ty = inst.typeParam();
      IRSPOffsetData data {locToStkOff(inst)};
      unit.replace(&inst, AssertStk, data, ty, unit.mainSP());
      return;
    }
    case CheckLoc: {
      auto ty = inst.typeParam();
      // NOTE: RelOffsetData takes an optional BCSPOffset but it only gets
      // read inside of region-tracelet when we walk guards-- if we're
      // killing an inlined frame its locals won't be part of the guards
      RelOffsetData data {locToStkOff(inst)};
      auto next = inst.next();
      unit.replace(&inst, CheckStk, data, ty, inst.taken(), unit.mainSP());
      inst.setNext(next);
      return;
    }
    case HintLocInner: {
      auto ty = inst.typeParam();
      // NOTE: same as above
      RelOffsetData data {locToStkOff(inst)};
      unit.replace(&inst, HintStkInner, data, ty, unit.mainSP());
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
  InlineReturnNoFrameData data {
    // +-------------------+
    // |                   |
    // | Outer Frame       |
    // |                   |  <-- FP    --- ---
    // +-------------------+             |   |
    // |                   |             |   |  B: DefSP.offset
    // | ...               |             |   |     (FPInvOffset, >= 0)
    // +-------------------+             |   |
    // |                   |  <-- SP     |  ---
    // +-------------------+           C |   |
    // |                   |             |   |
    // | ...               |             |   |  A: DefInlineFP.spOffset
    // +-------------------+             |   |     (IRSPRelOffset, <= 0)
    // |                   |            ---  |
    // | Callee Frame      |                 |
    // |                   |                ---
    // +-------------------+
    //
    // What we're trying to compute is C, a FPRelOffset (<0) from the
    // Outer FP to the top cell in the Callee Frame. From the picture,
    // we have |C| = |A| + |B| - 2. To get the negative result, A
    // already has the correct sign, but we need to negate B and the
    // minus 2 becomes +2, so: C = A - B + 2.
    FPRelOffset {
      frameInst->extra<DefInlineFP>()->spOffset.offset -
        spInst->extra<DefSP>()->offset.offset + 2
    }
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
