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

#include "hphp/runtime/vm/jit/translate-region.h"

#include "hphp/util/arch.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/bc-pattern.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/location.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/type.h"

TRACE_SET_MOD(trans);

namespace HPHP { namespace jit {

/*
 * Data used by irGenRegion() and friends to pass information between retries.
 */
struct TranslateRetryContext {
  // Bytecode instructions that must be interpreted.
  ProfSrcKeySet toInterp;

  // Regions to not inline
  jit::fast_set<ProfSrcKey, ProfSrcKey::Hasher> inlineBlacklist;
};

//////////////////////////////////////////////////////////////////////

namespace {

enum class TranslateResult {
  Retry,
  Success
};

/*
 * Create a map from RegionDesc::BlockId -> IR Block* for all region blocks.
 */
BlockIdToIRBlockMap createBlockMap(irgen::IRGS& irgs,
                                   const RegionDesc& region) {
  auto ret = BlockIdToIRBlockMap{};

  auto& irb = *irgs.irb;
  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    auto const rBlock = blocks[i];
    auto const id = rBlock->id();

    // NB: This maps the region entry block to a new IR block, even though
    // we've already constructed an IR entry block. We'll make the IR entry
    // block jump to this block.
    assertx(!hasTransID(id) || profData());
    auto transCount = hasTransID(id)
      ? region.blockProfCount(id)
      : 1;
    uint64_t profCount = transCount * irgs.profFactor;
    auto const iBlock = irb.unit().defBlock(profCount);

    ret[id] = iBlock;
    FTRACE(1,
           "createBlockMaps: RegionBlock {} => IRBlock {} (BC offset = {})\n",
           id, iBlock->id(), rBlock->start().offset());
  }

  return ret;
}

/*
 * Set IRBuilder's Block associated to blockId's block according to
 * the mapping in blockIdToIRBlock.
 */
void setIRBlock(irgen::IRGS& irgs,
                RegionDesc::BlockId blockId,
                const RegionDesc& region,
                const BlockIdToIRBlockMap& blockIdToIRBlock) {
  auto& irb = *irgs.irb;
  auto rBlock = region.block(blockId);
  auto sk = rBlock->start();

  auto iit = blockIdToIRBlock.find(blockId);
  assertx(iit != blockIdToIRBlock.end());

  assertx(!irb.hasBlock(sk));
  FTRACE(3, "  setIRBlock: blockId {}, offset {} => IR Block {}\n",
         blockId, sk.offset(), iit->second->id());
  irb.setBlock(sk, iit->second);
}

/*
 * Set IRBuilder's Blocks for srcBlockId's successors' offsets within
 * the region.  It also sets the guard-failure block, if any.
 */
void setSuccIRBlocks(irgen::IRGS& irgs,
                     const RegionDesc& region,
                     RegionDesc::BlockId srcBlockId,
                     const BlockIdToIRBlockMap& blockIdToIRBlock) {
  FTRACE(3, "setSuccIRBlocks: srcBlockId = {}\n", srcBlockId);
  auto& irb = *irgs.irb;
  irb.resetOffsetMapping();
  for (auto dstBlockId : region.succs(srcBlockId)) {
    setIRBlock(irgs, dstBlockId, region, blockIdToIRBlock);
  }
  if (auto nextRetrans = region.nextRetrans(srcBlockId)) {
    auto it = blockIdToIRBlock.find(nextRetrans.value());
    assertx(it != blockIdToIRBlock.end());
    irb.setGuardFailBlock(it->second);
  } else {
    irb.resetGuardFailBlock();
  }
}

bool blockHasUnprocessedPred(
  const RegionDesc&             region,
  RegionDesc::BlockId           blockId,
  const RegionDesc::BlockIdSet& processedBlocks)
{
  for (auto predId : region.preds(blockId)) {
    if (processedBlocks.count(predId) == 0) {
      return true;
    }
  }
  if (auto prevRetrans = region.prevRetrans(blockId)) {
    if (processedBlocks.count(prevRetrans.value()) == 0) {
      return true;
    }
  }
  return false;
}

/*
 * If this region's entry block is at an entry point for a function (DV init or
 * main entry), we can assert that all non-parameter locals are Uninit under
 * some conditions.  This function checks the necessary conditions and, if they
 * hold, emits such type assertions.
 */
void emitEntryAssertions(irgen::IRGS& irgs, const Func* func, SrcKey sk) {
  if (!func->isEntry(sk.offset())) return;

  // If we're at the Func main entry point, we can't assume anything if there
  // are DV initializers, because they can run arbitrary code before they get
  // here (and they do, in some hhas-based builtins, and they may not even get
  // to the Func main entry point).
  if (sk.offset() == func->base()) {
    // The assertions inserted here are only valid if the first bytecode
    // instruction does not have unprocessed predecessors.  This invariant is
    // ensured by the emitter using an EntryNop instruction when necessary.
    for (auto& pinfo : func->params()) {
      if (pinfo.hasDefaultValue()) return;
    }
  }

  if (func->isClosureBody()) {
    // In a closure, non-parameter locals can have types other than Uninit
    // after the prologue runs.  (Local 0 will be the closure itself, and other
    // locals will have used vars unpacked into them.)  We rely on hhbbc to
    // assert these types.
    return;
  }
  if (func->isPseudoMain()) {
    // Pseudomains inherit the variable environment of their caller, so don't
    // assert anything in them.
    return;
  }
  auto const numLocs = func->numLocals();
  auto loc = func->numParams();
  if (func->hasReifiedGenerics()) {
    // First non parameter local will specially set
    auto const t = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
    irgen::assertTypeLocation(irgs, Location::Local { loc }, t);
    loc++;
  }
  for (; loc < numLocs; ++loc) {
    auto const location = Location::Local { loc };
    irgen::assertTypeLocation(irgs, location, TUninit);
  }
}

/*
 * Emit type and reffiness prediction guards.
 */
void emitPredictionsAndPreConditions(irgen::IRGS& irgs,
                                     const RegionDesc& /*region*/,
                                     const RegionDesc::Block& block,
                                     bool isEntry, bool checkOuterTypeOnly) {
  auto const sk = block.start();
  auto const bcOff = sk.offset();
  auto& typePredictions = block.typePredictions();
  auto& typePreConditions = block.typePreConditions();

  if (isEntry) {
    irgen::ringbufferEntry(irgs, Trace::RBTypeTraceletGuards, sk);
    emitEntryAssertions(irgs, block.func(), sk);
  }

  // Emit type predictions.
  for (auto const& pred : typePredictions) {
    auto type = pred.type;
    auto loc  = pred.location;
    assertx(type <= TGen);
    irgen::predictType(irgs, loc, type);
  }

  // Emit type guards/preconditions.
  for (auto const& preCond : typePreConditions) {
    auto type = preCond.type;
    auto loc  = preCond.location;
    assertx(type <= TGen);
    irgen::checkType(irgs, loc, type, bcOff, checkOuterTypeOnly);
  }

  // Finish emitting guards, and emit profiling counters.
  if (isEntry) {
    irgen::gen(irgs, EndGuards);

    if (irgs.context.kind == TransKind::Profile) {
      if (block.func()->isEntry(bcOff) && !mcgen::retranslateAllEnabled()) {
        irgen::checkCold(irgs, irgs.context.transID);
      } else {
        irgen::incProfCounter(irgs, irgs.context.transID);
      }
    }
    irgen::ringbufferEntry(irgs, Trace::RBTypeTraceletBody, sk);

    // In the entry block, hhbc-translator gets a chance to emit some code
    // immediately after the initial checks on the first instruction.
    irgen::prepareEntry(irgs);
  }
}

void initNormalizedInstruction(
  NormalizedInstruction& inst,
  irgen::IRGS& irgs,
  const RegionDesc& region,
  RegionDesc::BlockId blockId,
  bool lastInstr,
  bool toInterp
) {
  if (lastInstr) {
    inst.endsRegion  = region.isExit(blockId);
  }

  // We can get a more precise output type for interpOne if we know all of
  // its inputs, so we still populate the rest of the instruction even if
  // this is true.
  inst.interp = toInterp;
}

/*
 * Returns the id of the next region block in workQ whose
 * corresponding IR block is currently reachable from the IR unit's
 * entry, or folly::none if no such block exists.  Furthermore, any
 * unreachable blocks appearing before the first reachable block are
 * moved to the end of workQ.
 */
folly::Optional<RegionDesc::BlockId> nextReachableBlock(
  jit::queue<RegionDesc::BlockId>& workQ,
  const irgen::IRBuilder& irb,
  const BlockIdToIRBlockMap& blockIdToIRBlock
) {
  auto const size = workQ.size();
  for (size_t i = 0; i < size; i++) {
    auto const regionBlockId = workQ.front();
    workQ.pop();
    auto it = blockIdToIRBlock.find(regionBlockId);
    assertx(it != blockIdToIRBlock.end());
    auto irBlock = it->second;
    if (irb.canStartBlock(irBlock)) return regionBlockId;
    // Put the block back at the end of workQ, since it may become
    // reachable after processing some of the other blocks.
    workQ.push(regionBlockId);
  }
  return folly::none;
}

/*
 * Returns whether or not block `bid' is in the retranslation chain
 * for `region's entry block.
 */
bool inEntryRetransChain(RegionDesc::BlockId bid, const RegionDesc& region) {
  auto block = region.entry();
  if (block->start() != region.block(bid)->start()) return false;
  while (true) {
    if (block->id() == bid) return true;
    auto nextRetrans = region.nextRetrans(block->id());
    if (!nextRetrans) return false;
    block = region.block(nextRetrans.value());
  }
  not_reached();
}

/*
 * If `psk' is not an FCall with inlinable `callee', return nullptr.
 *
 * Otherwise, select a region for `callee' if one is not already present in
 * `retry'.  Update `inl' and return the region if it's inlinable.
 */
RegionDescPtr getInlinableCalleeRegion(const irgen::IRGS& irgs,
                                       const Func* callee,
                                       const FCallArgs& fca,
                                       Type ctxType,
                                       const ProfSrcKey& psk,
                                       int& calleeCost) {
  assertx(isFCall(psk.srcKey.op()));
  if (isProfiling(irgs.context.kind) || irgs.inlineState.conjure) {
    return nullptr;
  }
  if (!irgs.region || !irgs.retryContext) {
    return nullptr;
  }
  auto annotationsPtr = mcgen::dumpTCAnnotation(irgs.context.kind) ?
                        irgs.unit.annotationData.get() : nullptr;
  if (!canInlineAt(psk.srcKey, callee, annotationsPtr)) return nullptr;

  auto const& inlineBlacklist = irgs.retryContext->inlineBlacklist;
  if (inlineBlacklist.find(psk) != inlineBlacklist.end()) {
    return nullptr;
  }

  auto calleeRegion = selectCalleeRegion(
    irgs, callee, fca, ctxType, psk.srcKey);
  if (!calleeRegion || calleeRegion->instrSize() > irgs.budgetBCInstrs) {
    return nullptr;
  }

  calleeCost = costOfInlining(psk.srcKey, callee, *calleeRegion,
                              annotationsPtr);
  return calleeRegion;
}

static bool needsSurpriseCheck(Op op) {
  return op == Op::JmpZ || op == Op::JmpNZ || op == Op::Jmp;
}

// Unlike isCompare, this also allows Not, Same, NSame and Cmp.
static bool isCmp(Op op) {
  return op == Op::Not ||
      op == Op::Same ||
      op == Op::NSame ||
      op == Op::Eq ||
      op == Op::Neq ||
      op == Op::Lt ||
      op == Op::Lte ||
      op == Op::Gt ||
      op == Op::Gte ||
      op == Op::Cmp;
}

TranslateResult irGenRegionImpl(irgen::IRGS& irgs,
                                const RegionDesc& region,
                                double profFactor) {
  const Timer irGenTimer(Timer::irGenRegionAttempt);
  auto& irb = *irgs.irb;
  auto prevRegion      = irgs.region;      irgs.region      = &region;
  auto prevProfFactor  = irgs.profFactor;  irgs.profFactor  = profFactor;
  auto prevProfTransID = irgs.profTransID; irgs.profTransID = kInvalidTransID;
  auto prevOffsetMapping = irb.saveAndClearOffsetMapping();
  auto prevGuardFailBlock = irb.guardFailBlock(); irb.resetGuardFailBlock();
  SCOPE_EXIT {
    irgs.region      = prevRegion;
    irgs.profFactor  = prevProfFactor;
    irgs.profTransID = prevProfTransID;
    irb.restoreOffsetMapping(std::move(prevOffsetMapping));
    irb.setGuardFailBlock(prevGuardFailBlock);
  };

  FTRACE(1, "translateRegion (mode={}, profFactor={:.2}) starting with:\n{}\n",
         show(irgs.context.kind), profFactor, show(region));

  if (RuntimeOption::EvalDumpRegion &&
      mcgen::dumpTCAnnotation(irgs.context.kind)) {
    irgs.annotations.emplace_back("RegionDesc", show(region));
  }

  std::string errorMsg;
  always_assert_flog(check(region, errorMsg), "{}", errorMsg);

  auto regionSize = region.instrSize();
  always_assert(regionSize <= irgs.budgetBCInstrs);
  irgs.budgetBCInstrs -= regionSize;

  // Create a map from region blocks to their corresponding initial IR blocks.
  auto const inlining = isInlining(irgs);
  auto blockIdToIRBlock = createBlockMap(irgs, region);

  if (!inlining) {
    // Prepare to start translation of the first region block.
    auto const entry = irb.unit().entry();
    irb.startBlock(entry, false /* hasUnprocPred */);

    // Make the IR entry block jump to the IR block we mapped the region entry
    // block to (they are not the same!).
    {
      auto const irBlock = blockIdToIRBlock[region.entry()->id()];
      always_assert(irBlock != entry);
      irgen::gen(irgs, Jmp, irBlock);
    }
  } else {
    // Set the first callee block as a successor to the FCall's block and
    // "fallthrough" from the caller into the callee's first block.
    setIRBlock(irgs, region.entry()->id(), region, blockIdToIRBlock);
    irgen::endBlock(irgs, region.start().offset());
  }

  RegionDesc::BlockIdSet processedBlocks;

  auto& blocks = region.blocks();

  jit::queue<RegionDesc::BlockId> workQ;
  for (auto& block : blocks) workQ.push(block->id());

  while (auto optBlockId = nextReachableBlock(workQ, irb, blockIdToIRBlock)) {
    auto const blockId = optBlockId.value();
    auto const& block  = *region.block(blockId);
    auto sk            = block.start();
    bool emitedSurpriseCheck = false;

    SCOPE_ASSERT_DETAIL("IRGS") { return show(irgs); };

    irgs.profTransID = hasTransID(blockId) ? getTransID(blockId)
                                           : kInvalidTransID;
    irgs.firstBcInst = inEntryRetransChain(blockId, region) && !inlining;
    irgen::prepareForNextHHBC(irgs, sk);

    // Prepare to start translating this region block.  This loads the
    // FrameState for the IR block corresponding to the start of this
    // region block, and it also sets the map from BC offsets to IR
    // blocks for the successors of this block in the region.
    auto const irBlock = blockIdToIRBlock[blockId];
    const bool hasUnprocPred = blockHasUnprocessedPred(region, blockId,
                                                       processedBlocks);
    // Note: a block can have an unprocessed predecessor even if the
    // region is acyclic, e.g. if the IR was able to prove a path was
    // unfeasible due to incompatible types.
    if (!irb.startBlock(irBlock, hasUnprocPred)) {
      // This can't happen because we picked a reachable block from the workQ.
      always_assert_flog(
        0, "translateRegion: tried to startBlock on unreachable block {}\n",
        blockId
      );
    }
    setSuccIRBlocks(irgs, region, blockId, blockIdToIRBlock);

    // Emit an ExitPlaceholder at the beginning of the block if any of the
    // optimizations that can benefit from it are enabled, and only if we're
    // not inlining. The inlining decision could be smarter but this is enough
    // for now since we never emit guards in inlined functions (t7385908).
    const bool emitExitPlaceholder = !inlining &&
      (RuntimeOption::EvalHHIRLICM && hasUnprocPred);
    if (emitExitPlaceholder) irgen::makeExitPlaceholder(irgs);

    // Emit the type and reffiness predictions for this region block. If this is
    // the first instruction in the region, we check inner type eagerly, insert
    // `EndGuards` after the checks, and generate profiling code in profiling
    // translations.
    auto const isEntry = &block == region.entry().get() && !inlining;
    auto const checkOuterTypeOnly = !irb.guardFailBlock() &&
      (!isEntry || irgs.context.kind != TransKind::Profile);
    emitPredictionsAndPreConditions(irgs, region, block, isEntry,
                                    checkOuterTypeOnly);
    irb.resetGuardFailBlock();

    // Generate IR for each bytecode instruction in this block.
    for (unsigned i = 0; i < block.length(); ++i, sk.advance(block.unit())) {
      ProfSrcKey psk { irgs.profTransID, sk };
      auto const lastInstr = i == block.length() - 1;
      auto const penultimateInst = i == block.length() - 2;

      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      irgen::prepareForNextHHBC(irgs, sk);

      // Create and initialize the instruction.
      NormalizedInstruction inst(sk, block.unit());
      bool toInterpInst = irgs.retryContext->toInterp.count(psk);
      initNormalizedInstruction(inst, irgs, region, blockId,
                                lastInstr, toInterpInst);

      if (penultimateInst && isCmp(inst.op())) {
          SrcKey nextSk = inst.nextSk();
          Op nextOp = nextSk.op();
          auto const backwards = [&]{
            auto const offsets = instrJumpOffsets(nextSk.pc());
            return std::any_of(
              offsets.begin(), offsets.end(), [] (Offset o) { return o < 0; }
            );
          };
          if (needsSurpriseCheck(nextOp) && backwards()) {
            emitedSurpriseCheck = true;
            inst.forceSurpriseCheck = true;
          }
      }

      // Emit IR for the body of the instruction.
      try {
        const bool firstInstr = isEntry && i == 0;
        auto const backwards = [&]{
          auto const offsets = instrJumpOffsets(inst.pc());
          return std::any_of(
            offsets.begin(), offsets.end(), [] (Offset o) { return o < 0; }
          );
        };
        if (lastInstr && !emitedSurpriseCheck &&
            needsSurpriseCheck(inst.op()) &&
            backwards()) {
          emitedSurpriseCheck = true;
          inst.forceSurpriseCheck = true;
        }
        translateInstr(irgs, inst, checkOuterTypeOnly, firstInstr);
      } catch (const RetryIRGen& e) {
        return TranslateResult::Retry;
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk2{irgs.profTransID, sk};
        always_assert_flog(!irgs.retryContext->toInterp.count(psk2),
                           "IR generation failed with {}\n",
                           exn.what());
        FTRACE(1, "ir generation for {} failed with {}\n",
          inst.toString(), exn.what());
        irgs.retryContext->toInterp.insert(psk2);
        return TranslateResult::Retry;
      }

      irgen::finishHHBC(irgs);

      // If this is the last instruction, handle block transitions.
      // If the block ends the region, then call irgen::endRegion to
      // sync the state and make a REQ_BIND_JMP service request.
      // Otherwise, if the instruction has a fall-through, then insert
      // a jump to the next offset, since it may not be the next block
      // to be translated.
      if (lastInstr) {
        if (region.isExit(blockId)) {
          if (!inlining || !isReturnish(inst.op())) {
            irgen::endRegion(irgs);
          }
        } else if (instrAllowsFallThru(inst.op())) {
          irgen::endBlock(irgs, inst.nextSk().offset());
        }
      }
    }

    processedBlocks.insert(blockId);
  }

  if (!inlining) {
    irgen::sealUnit(irgs);
  }

  return TranslateResult::Success;
}

}

//////////////////////////////////////////////////////////////////////

std::unique_ptr<IRUnit> irGenRegion(const RegionDesc& region,
                                    const TransContext& context,
                                    PostConditions& pConds) noexcept {
  Timer irGenTimer(Timer::irGenRegion);
  SCOPE_ASSERT_DETAIL("RegionDesc") { return show(region); };

  std::unique_ptr<IRUnit> unit;
  SCOPE_ASSERT_DETAIL("IRUnit") { return unit ? show(*unit) : "<null>"; };
  TranslateRetryContext retryContext;

  rqtrace::EventGuard trace{"IRGEN"};
  uint32_t tries = 0;

  while (true) {
    int32_t budgetBCInstrs = RuntimeOption::EvalJitMaxRegionInstrs;
    unit = std::make_unique<IRUnit>(context,
                                    std::make_unique<AnnotationData>());
    unit->initLogEntry(context.initSrcKey.func());
    irgen::IRGS irgs{*unit, &region, budgetBCInstrs, &retryContext};
    tries++;

    // Set the profCount of the IRUnit's entry block, which is created a
    // priori.
    if (context.kind == TransKind::Optimize) {
      assertx(profData());
      auto entryBID = region.entry()->id();
      assertx(hasTransID(entryBID));
      auto entryTID = getTransID(entryBID);
      auto entryProfCount = region.blockProfCount(entryTID);
      irgs.unit.entry()->setProfCount(entryProfCount);
    }

    try {
      auto const result = irGenRegionImpl(irgs, region, 1);
      if (result == TranslateResult::Retry) continue;
      assertx(result == TranslateResult::Success);
    } catch (const FailedTraceGen& e) {
      always_assert_flog(false, "irGenRegion failed with {}\n", e.what());
      continue;
    }

    assertx(budgetBCInstrs >= 0);
    FTRACE(1, "translateRegion: final budgetBCInstrs = {}\n", budgetBCInstrs);

    assertx(!isInlining(irgs));
    // For profiling translations, grab the postconditions to be used for
    // region selection whenever we decide to retranslate.
    assertx(pConds.changed.empty() && pConds.refined.empty());
    if (context.kind == TransKind::Profile &&
        RuntimeOption::EvalJitPGOUsePostConditions) {
      auto const lastSrcKey = region.lastSrcKey();
      if (auto const mainExit = findMainExitBlock(irgs.unit, lastSrcKey)) {
        FTRACE(2, "translateRegion: mainExit: B{}\nUnit: {}\n",
               mainExit->id(), show(irgs.unit));
        pConds = irgs.irb->fs().postConds(mainExit);
      } else {
        FTRACE(2, "translateRegion: no main exit\n");
      }
    }

    break;
  }

  trace.annotate("tries", folly::to<std::string>(tries));
  trace.finish();
  irGenTimer.stop();

  auto finishPass = [&](const char* msg, int level) {
    printUnit(level, *unit, msg, nullptr, nullptr);
    assertx(checkCfg(*unit));
  };

  finishPass(" after initial translation ", kIRLevel);
  optimize(*unit, context.kind);
  finishPass(" after optimizing ", kOptLevel);

  return unit;
}

bool irGenTryInlineFCall(irgen::IRGS& irgs, const Func* callee,
                         const FCallArgs& fca, SSATmp* ctx, bool dynamicCall) {
  auto const psk = ProfSrcKey { irgs.profTransID, curSrcKey(irgs) };
  int calleeCost{0};

  // See if we have a callee region we can inline.
  ctx = ctx ? ctx : cns(irgs, TNullptr);
  auto const calleeRegion = getInlinableCalleeRegion(
    irgs, callee, fca, ctx->type(), psk, calleeCost);
  if (!calleeRegion) return false;

  // We shouldn't be inlining profiling translations.
  assertx(irgs.context.kind != TransKind::Profile);
  assertx(calleeRegion->instrSize() <= irgs.budgetBCInstrs);

  FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
         "and stack:\n{}\n",
         curFunc(irgs)->fullName()->data(),
         callee->fullName()->data(),
         fca.numArgs,
         show(irgs));

  auto const& irb = *irgs.irb;
  auto const returnBlock = irb.unit().defBlock(irgen::curProfCount(irgs));
  auto const suspendRetBlock = irb.unit().defBlock(irgen::curProfCount(irgs));
  auto const asyncEagerOffset = callee->supportsAsyncEagerReturn()
    ? fca.asyncEagerOffset : kInvalidOffset;
  auto const returnTarget = InlineReturnTarget {
    returnBlock, suspendRetBlock, asyncEagerOffset
  };
  auto callFuncOff = bcOff(irgs) - curFunc(irgs)->base();

  irgen::beginInlining(irgs, callee, fca, ctx, ctx->type(), dynamicCall,
                       psk.srcKey.op(),
                       calleeRegion->start(),
                       callFuncOff,
                       returnTarget,
                       calleeCost);

  SCOPE_ASSERT_DETAIL("Inlined-RegionDesc")
    { return show(*calleeRegion); };

  // Calculate the profFactor for the callee as the weight of
  // the caller block over the weight of the entry block of
  // the callee region.
  double calleeProfFactor = irgen::curProfCount(irgs);
  auto const calleeEntryBID = calleeRegion->entry()->id();
  if (hasTransID(calleeEntryBID)) {
    assertx(profData());
    auto const calleeTID = getTransID(calleeEntryBID);
    auto calleeProfCount = calleeRegion->blockProfCount(calleeTID);
    if (calleeProfCount == 0) calleeProfCount = 1; // avoid div by zero
    calleeProfFactor /= calleeProfCount;
    assert_flog(calleeProfFactor >= 0, "calleeProfFactor = {:.5}\n",
                calleeProfFactor);
  }

  auto result = irGenRegionImpl(irgs, *calleeRegion, calleeProfFactor);
  assertx(irgs.budgetBCInstrs >= 0);

  if (result != TranslateResult::Success) {
    assertx(result == TranslateResult::Retry);
    throw RetryIRGen("inline-propagate-retry");
  }

  // Native calls end inlining before CallBuiltin
  if (!callee->isCPPBuiltin()) {
    // If the inlined region failed to contain any returns then the
    // rest of this block is dead- we could continue but there's no
    // benefit to inlining this call if it ends in a ReqRetranslate or
    // ReqBind* so instead we mark it as uninlinable and retry.
    if (!irgen::endInlining(irgs, *calleeRegion)) {
      irgs.retryContext->inlineBlacklist.insert(psk);
      throw RetryIRGen("inline-failed");
    }
  } else {
    // For native calls we don't use a return block
    assertx(returnBlock->empty());
  }

  // Success.
  return true;
}

std::unique_ptr<IRUnit> irGenInlineRegion(const TransContext& ctx,
                                          const RegionDesc& region) {
  SCOPE_ASSERT_DETAIL("Inline-RegionDesc") { return show(region); };

  std::unique_ptr<IRUnit> unit;
  TranslateRetryContext retryContext;
  auto const entryBID = region.entry()->id();

  while (true) {
    int32_t budgetBCInstrs = RuntimeOption::EvalJitMaxRegionInstrs;
    unit = std::make_unique<IRUnit>(ctx, std::make_unique<AnnotationData>());
    irgen::IRGS irgs{*unit, &region, budgetBCInstrs, &retryContext};
    irgs.inlineState.conjure = true;
    if (hasTransID(entryBID)) irgs.profTransID = getTransID(entryBID);

    auto& irb = *irgs.irb;
    auto const& argTypes = region.inlineInputTypes();
    auto const ctxType = region.inlineCtxType();

    auto const func = region.entry()->func();

    auto const entry = irb.unit().entry();
    auto returnBlock = irb.unit().defBlock();
    auto suspendRetBlock = irb.unit().defBlock();
    auto returnTarget = InlineReturnTarget {
      returnBlock, suspendRetBlock, kInvalidOffset
    };

    // Set the profCount of the entry and return blocks we just created.
    entry->setProfCount(curProfCount(irgs));
    returnBlock->setProfCount(curProfCount(irgs));

    SCOPE_ASSERT_DETAIL("Inline-IRUnit") { return show(*unit); };
    irb.startBlock(entry, false /* hasUnprocPred */);
    irgen::conjureBeginInlining(irgs, func, region.start(),
                                ctxType, argTypes, returnTarget);

    try {
      auto const result = irGenRegionImpl(irgs, region, 1 /* profFactor */);
      if (result == TranslateResult::Retry) continue;
      assertx(result == TranslateResult::Success);
    } catch (const FailedTraceGen& e) {
      FTRACE(2, "irGenInlineRegion failed with {}\n", e.what());
      always_assert_flog(false, "irGenInlineRegion failed with {}\n", e.what());
      continue;
    }

    if (!irgen::conjureEndInlining(irgs, region, func->isCPPBuiltin())) {
      return nullptr;
    }

    irgen::sealUnit(irgs);
    optimize(*unit, TransKind::Optimize);
    return unit;
  }
}

}}
