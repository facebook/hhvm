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

#include "hphp/runtime/vm/jit/translate-region.h"

#include "hphp/util/map-walker.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/vm/bc-pattern.h"

#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/type.h"

TRACE_SET_MOD(trans);

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Create a map from RegionDesc::BlockId -> IR Block* for all region blocks.
 */
BlockIdToIRBlockMap createBlockMap(IRGS& irgs, const RegionDesc& region) {
  auto ret = BlockIdToIRBlockMap{};

  auto& irb = *irgs.irb;
  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    auto const rBlock = blocks[i];
    auto const id = rBlock->id();

    // NB: This maps the region entry block to a new IR block, even though
    // we've already constructed an IR entry block. We'll make the IR entry
    // block jump to this block.
    auto transCount = hasTransID(id)
      ? mcg->tx().profData()->transCounter(getTransID(id))
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
void setIRBlock(IRGS& irgs,
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
void setSuccIRBlocks(IRGS& irgs,
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
 * If this region's entry block is at the entry point for a function, we have
 * some additional information we can assume about the types of non-parameter
 * local variables.
 *
 * Note: we can't assume anything if there are DV initializers, because they
 * can run arbitrary code before they get to the main entry point (and they do,
 * in some hhas-based builtins), if they even go there (they aren't required
 * to).
 */
void emitEntryAssertions(IRGS& irgs, const Func* func, SrcKey sk) {
  if (sk.offset() != func->base()) return;
  for (auto& pinfo : func->params()) {
    if (pinfo.hasDefaultValue()) return;
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
  for (auto loc = func->numParams(); loc < numLocs; ++loc) {
    auto const location = RegionDesc::Location::Local { loc };
    irgen::assertTypeLocation(irgs, location, TUninit);
  }
}

/*
 * Emit type and reffiness prediction guards.
 */
void emitPredictionsAndPreConditions(IRGS& irgs,
                          const RegionDesc& region,
                          const RegionDesc::BlockPtr block,
                          bool isEntry) {
  auto const sk = block->start();
  auto const bcOff = sk.offset();
  auto& typePredictions = block->typePredictions();
  auto& typePreConditions = block->typePreConditions();
  auto& refPreds = block->reffinessPreds();

  if (isEntry) {
    irgen::ringbufferEntry(irgs, Trace::RBTypeTraceletGuards, sk);
    emitEntryAssertions(irgs, block->func(), sk);
  }

  // Emit type predictions.
  for (auto const& pred : typePredictions) {
    auto type = pred.type;
    auto loc  = pred.location;
    irgen::predictTypeLocation(irgs, loc, type);
  }

  // Emit type guards/preconditions.
  for (auto const& preCond : typePreConditions) {
    auto type = preCond.type;
    auto loc  = preCond.location;
    if (type <= TCls) {
      // Do not generate guards for class; instead assert the type.
      assertx(loc.tag() == RegionDesc::Location::Tag::Stack);
      irgen::assertTypeLocation(irgs, loc, type);
    } else {
      // Check inner type eagerly if it is the first block during profiling.
      // Otherwise only check for BoxedInitCell.
      bool checkOuterTypeOnly =
        !isEntry || mcg->tx().mode() != TransKind::Profile;
      irgen::checkTypeLocation(irgs, loc, type, bcOff, checkOuterTypeOnly);
    }
  }

  // Emit reffiness predictions.
  for (auto const& pred : refPreds) {
    irgen::checkRefs(irgs, pred.arSpOffset, pred.mask, pred.vals, bcOff);
  }

  // Finish emitting guards, and emit profiling counters.
  if (isEntry) {
    irgen::gen(irgs, EndGuards);

    if (RuntimeOption::EvalJitTransCounters) {
      irgen::incTransCounter(irgs);
    }

    if (mcg->tx().mode() == TransKind::Profile) {
      if (block->func()->isEntry(bcOff)) {
        irgen::checkCold(irgs, mcg->tx().profData()->curTransID());
      } else {
        irgen::incProfCounter(irgs, mcg->tx().profData()->curTransID());
      }
    }
    irgen::ringbufferEntry(irgs, Trace::RBTypeTraceletBody, sk);

    // In the entry block, hhbc-translator gets a chance to emit some code
    // immediately after the initial checks on the first instruction.
    switch (arch()) {
      case Arch::X64:
        irgen::prepareEntry(irgs);
        break;
      case Arch::ARM:
        // Don't do this for ARM, because it can lead to interpOne on the
        // first SrcKey in a translation, which isn't allowed.
        break;
      case Arch::PPC64:
        not_implemented();
        break;
    }
  }
}

void initNormalizedInstruction(
  NormalizedInstruction& inst,
  MapWalker<RegionDesc::Block::ParamByRefMap>& byRefs,
  IRGS& irgs,
  const RegionDesc& region,
  RegionDesc::BlockId blockId,
  const Func* topFunc,
  bool lastInstr,
  bool toInterp) {

  inst.funcd = topFunc;

  if (lastInstr) {
    inst.endsRegion  = region.isExit(blockId);
  }

  // We can get a more precise output type for interpOne if we know all of
  // its inputs, so we still populate the rest of the instruction even if
  // this is true.
  inst.interp = toInterp;

  auto const inputInfos = getInputs(inst);

  FTRACE(2, "populating inputs for {}\n", inst.toString());
  for (auto const& ii : inputInfos) {
    inst.inputs.push_back(ii.loc);
  }

  if (inputInfos.needsRefCheck) {
    inst.preppedByRef = byRefs.next();
  }
}

bool shouldTrySingletonInline(const RegionDesc& region,
                              RegionDesc::BlockPtr block,
                              const NormalizedInstruction& inst,
                              unsigned instIdx,
                              TransFlags trflags) {
  if (!RuntimeOption::RepoAuthoritative) return false;

  // I don't really want to inline my arm, thanks.
  if (arch() != Arch::X64) return false;

  // Don't inline if we're retranslating due to a side-exit from an
  // inlined call.
  auto const startSk = region.start();
  if (trflags.noinlineSingleton && startSk == inst.source) return false;

  // Bail early if this isn't a push.
  if (inst.op() != Op::FPushFuncD &&
      inst.op() != Op::FPushClsMethodD) {
    return false;
  }

  return true;
}

/*
 * Check if `i' is an FPush{Func,ClsMethod}D followed by an FCall{,D} to a
 * function with a singleton pattern, and if so, inline it.  Returns true if
 * this succeeds, else false.
 */
bool tryTranslateSingletonInline(IRGS& irgs,
                                 const NormalizedInstruction& ninst,
                                 const Func* funcd) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  if (!funcd) return false;

  // Make sure we have an acceptable FPush and non-null callee.
  assertx(ninst.op() == Op::FPushFuncD ||
         ninst.op() == Op::FPushClsMethodD);

  auto fcall = ninst.nextSk();

  // Check if the next instruction is an acceptable FCall.
  if ((fcall.op() != Op::FCall && fcall.op() != Op::FCallD) ||
      funcd->isResumable() || funcd->isReturnRef()) {
    return false;
  }

  // First, check for the static local singleton pattern...

  // Lambda to check if CGetL and StaticLocInit refer to the same local.
  auto has_same_local = [] (PC pc, const Captures& captures) {
    if (captures.size() == 0) return false;

    auto cgetl = pc;
    auto sli = captures[0];

    assertx(peek_op(cgetl) == Op::CGetL);
    assertx(peek_op(sli) == Op::StaticLocInit);

    return (getImm(sli, 0).u_IVA == getImm(cgetl, 0).u_IVA);
  };

  auto cgetl = Atom(Op::CGetL).onlyif(has_same_local);
  auto retc  = Atom(Op::RetC);

  // Look for a static local singleton pattern.
  auto result = BCPattern {
    Atom(Op::Null),
    Atom(Op::StaticLocInit).capture(),
    Atom(Op::IsTypeL),
    Atom::alt(
      Atom(Op::JmpZ).taken({cgetl, retc}),
      Atom::seq(Atom(Op::JmpNZ), cgetl, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      irgen::prepareForNextHHBC(irgs, nullptr, ninst.source, false);
      irgen::inlSingletonSLoc(
        irgs,
        funcd,
        result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sloc] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  // Not found; check for the static property pattern.

  // Factory for String atoms that are required to match another captured
  // String opcode.
  auto same_string_as = [&] (int i) {
    return Atom(Op::String).onlyif([=] (PC pc, const Captures& captures) {
      auto string1 = pc;
      auto string2 = captures[i];
      assertx(peek_op(string1) == Op::String);
      assertx(peek_op(string2) == Op::String);

      auto const unit = funcd->unit();
      auto sd1 = unit->lookupLitstrId(getImmPtr(string1, 0)->u_SA);
      auto sd2 = unit->lookupLitstrId(getImmPtr(string2, 0)->u_SA);

      return (sd1 && sd1 == sd2);
    });
  };

  auto stringProp = same_string_as(0);
  auto stringCls  = same_string_as(1);
  auto agetc = Atom(Op::AGetC);
  auto cgets = Atom(Op::CGetS);

  // Look for a class static singleton pattern.
  result = BCPattern {
    Atom(Op::String).capture(),
    Atom(Op::String).capture(),
    Atom(Op::AGetC),
    Atom(Op::CGetS),
    Atom(Op::IsTypeC),
    Atom::alt(
      Atom(Op::JmpZ).taken({stringProp, stringCls, agetc, cgets, retc}),
      Atom::seq(Atom(Op::JmpNZ), stringProp, stringCls, agetc, cgets, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      irgen::prepareForNextHHBC(irgs, nullptr, ninst.source, false);
      irgen::inlSingletonSProp(
        irgs,
        funcd,
        result.getCapture(1),
        result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sprop] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  return false;
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
  const IRBuilder& irb,
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

uint32_t countRets(const RegionDesc& region) {
  uint32_t count = 0;
  for (auto const& block : region.blocks()) {
    if (isReturnish(block->last().op())) count++;
  }
  return count;
}

/*
 * If `psk' is not an FCall{,D} with inlinable `callee', return nullptr.
 *
 * Otherwise, select a region for `callee' if one is not already present in
 * `retry'.  Update `inl' and return the region if it's inlinable.
 */
RegionDescPtr getInlinableCalleeRegion(const ProfSrcKey& psk,
                                       const Func* callee,
                                       TranslateRetryContext& retry,
                                       InliningDecider& inl,
                                       const IRGS& irgs,
                                       int32_t maxBCInstrs,
                                       bool& needsMerge) {
  if (psk.srcKey.op() != Op::FCall &&
      psk.srcKey.op() != Op::FCallD) {
    return nullptr;
  }

  if (!inl.canInlineAt(psk.srcKey, callee)) return nullptr;

  auto const& fpiStack = irgs.irb->fpiStack();
  // Make sure the FPushOp was in the region
  if (fpiStack.empty()) {
    return nullptr;
  }

  // Make sure the FPushOp wasn't interpreted, based on an FPushCuf, or spanned
  // another call
  auto const info = fpiStack.front();
  if (isFPushCuf(info.fpushOpc) || info.interp || info.spansCall) {
    return nullptr;
  }

  // Task #8249076: Disable inlining when we need to load the context
  // since it seems broken.
  if (!info.ctx && !isFPushFunc(info.fpushOpc)) return nullptr;

  // We can't inline FPushClsMethod when the callee may have a $this pointer
  if (isFPushClsMethod(info.fpushOpc) && callee->mayHaveThis()) {
    return nullptr;
  }

  if (retry.inlineBlacklist.find(psk) != retry.inlineBlacklist.end()) {
    return nullptr;
  }

  auto calleeRegion = selectCalleeRegion(psk.srcKey, callee, irgs, inl,
                                         maxBCInstrs);
  if (!calleeRegion || calleeRegion->instrSize() > maxBCInstrs) {
    return nullptr;
  }

  inl.accountForInlining(callee, *calleeRegion);
  needsMerge = countRets(*calleeRegion) > 1;
  return calleeRegion;
}

TranslateResult irGenRegion(IRGS& irgs,
                            const RegionDesc& region,
                            TranslateRetryContext& retry,
                            TransFlags trflags,
                            InliningDecider& inl,
                            int32_t& budgetBCInstrs,
                            double profFactor) {
  const Timer translateRegionTimer(Timer::translateRegion);
  double prevProfFactor = irgs.profFactor;
  irgs.profFactor = profFactor;
  SCOPE_EXIT { irgs.profFactor = prevProfFactor; };

  FTRACE(1, "translateRegion (mode={}, profFactor={:.2}) starting with:\n{}\n",
         show(mcg->tx().mode()), profFactor, show(region));

  if (RuntimeOption::EvalDumpRegion) {
    mcg->annotations().emplace_back("RegionDesc", show(region));
  }

  std::string errorMsg;
  always_assert_flog(check(region, errorMsg), "{}", errorMsg);

  auto& irb = *irgs.irb;

  auto regionSize = region.instrSize();
  always_assert(regionSize <= budgetBCInstrs);
  budgetBCInstrs -= regionSize;

  // Create a map from region blocks to their corresponding initial IR blocks.
  auto blockIdToIRBlock = createBlockMap(irgs, region);

  if (!inl.inlining()) {
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

  Timer irGenTimer(Timer::translateRegion_irGeneration);
  auto& blocks = region.blocks();

  jit::queue<RegionDesc::BlockId> workQ;
  for (auto& block : blocks) workQ.push(block->id());

  while (auto optBlockId = nextReachableBlock(workQ, irb, blockIdToIRBlock)) {
    auto const blockId = optBlockId.value();
    auto const& block  = region.block(blockId);
    auto sk            = block->start();
    auto byRefs        = makeMapWalker(block->paramByRefs());
    auto knownFuncs    = makeMapWalker(block->knownFuncs());
    auto skipTrans     = false;

    SCOPE_ASSERT_DETAIL("IRGS") { return show(irgs); };

    const Func* topFunc = nullptr;
    if (hasTransID(blockId)) irgs.profTransID = getTransID(blockId);
    irgs.inlineLevel = inl.depth();
    irgs.firstBcInst = inEntryRetransChain(blockId, region) && !inl.inlining();
    irgen::prepareForNextHHBC(irgs, nullptr, sk, false);

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

    // Emit an ExitPlaceholder at the beginning of the block if any of
    // the optimizations that can benefit from it are enabled, and only
    // if we're not inlining. The inlining decision could be smarter
    // but this is enough for now since we never emit guards in inlined
    // functions (t7385908).
    const bool emitExitPlaceholder = irgs.inlineLevel == 0 &&
      ((RuntimeOption::EvalHHIRLICM && hasUnprocPred) ||
       (RuntimeOption::EvalHHIRTypeCheckHoisting));
    if (emitExitPlaceholder) irgen::makeExitPlaceholder(irgs);

    // Emit the type and reffiness predictions for this region block. If this is
    // the first instruction in the region, we check inner type eagerly, insert
    // `EndGuards` after the checks, and generate profiling code in profiling
    // translations.
    auto const isEntry = block == region.entry() && !inl.inlining();
    auto const checkOuterTypeOnly =
      !isEntry || mcg->tx().mode() != TransKind::Profile;
    emitPredictionsAndPreConditions(irgs, region, block, isEntry);
    irb.resetGuardFailBlock();

    // Generate IR for each bytecode instruction in this block.
    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      ProfSrcKey psk { irgs.profTransID, sk };
      auto const lastInstr = i == block->length() - 1;

      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      irgen::prepareForNextHHBC(irgs, nullptr, sk, false);

      // Update the current funcd, if we have a new one.
      if (knownFuncs.hasNext(sk)) {
        topFunc = knownFuncs.next();
      }
      // HHIR may have figured the topFunc even though the RegionDesc
      // didn't know it.  When that happens, update topFunc.
      if (!topFunc && !irb.fpiStack().empty()) {
        auto& fpiInfo = irb.fpiStack().front();
        auto func = fpiInfo.func;
        if (func && func->isNameBindingImmutable(block->unit())) {
          topFunc = func;
        }
      }

      // Create and initialize the instruction.
      NormalizedInstruction inst(sk, block->unit());
      bool toInterpInst = retry.toInterp.count(psk);
      initNormalizedInstruction(inst, byRefs, irgs, region, blockId,
                                topFunc, lastInstr, toInterpInst);

      // Singleton inlining optimization.
      if (RuntimeOption::EvalHHIRInlineSingletons && !lastInstr &&
          shouldTrySingletonInline(region, block, inst, i, trflags) &&
          knownFuncs.hasNext(inst.nextSk())) {

        // This is safe to do even if singleton inlining fails; we just won't
        // change topFunc in the next pass since hasNext() will return false.
        topFunc = knownFuncs.next();

        if (tryTranslateSingletonInline(irgs, inst, topFunc)) {
          // Skip the translation of this instruction (the FPush) -and- the
          // next instruction (the FCall) if singleton inlining succeeds.
          // We still want the fallthrough and prediction logic, though.
          skipTrans = true;
          continue;
        }
      }

      bool calleeIsMerge = false;
      RegionDescPtr calleeRegion{nullptr};
      // See if we have a callee region we can inline---but only if the
      // singleton inliner isn't actively inlining.
      if (!skipTrans) {
        calleeRegion = getInlinableCalleeRegion(psk, inst.funcd, retry, inl,
                                                irgs, budgetBCInstrs,
                                                calleeIsMerge);
      }

      if (calleeRegion) {
        always_assert(inst.op() == Op::FCall || inst.op() == Op::FCallD);
        auto const* callee = inst.funcd;

        // We shouldn't be inlining profiling translations.
        assertx(mcg->tx().mode() != TransKind::Profile);

        assertx(calleeRegion->instrSize() <= budgetBCInstrs);

        FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
               "and stack:\n{}\n",
               block->func()->fullName()->data(),
               callee->fullName()->data(),
               inst.imm[0].u_IVA,
               show(irgs));

        auto returnSk = inst.nextSk();
        auto returnBlock = irb.unit().defBlock(irgen::curProfCount(irgs));
        auto returnFuncOff = returnSk.offset() - block->func()->base();

        if (irgen::beginInlining(irgs, inst.imm[0].u_IVA, callee, returnFuncOff,
                                 returnBlock, calleeIsMerge)) {
          SCOPE_ASSERT_DETAIL("Inlined-RegionDesc")
            { return show(*calleeRegion); };

          // Reset block state before reentering irGenRegion
          irb.resetOffsetMapping();
          irb.resetGuardFailBlock();

          // Calculate the profFactor for the callee as the weight of
          // the caller block over the weight of the entry block of
          // the callee region.
          double calleeProfFactor = irgen::curProfCount(irgs);
          auto const calleeEntryBID = calleeRegion->entry()->id();
          if (hasTransID(calleeEntryBID)) {
            auto const calleeTID = getTransID(calleeEntryBID);
            calleeProfFactor = calleeProfFactor /
                               mcg->tx().profData()->transCounter(calleeTID);
          }

          auto result = irGenRegion(irgs, *calleeRegion, retry, trflags, inl,
                                    budgetBCInstrs, calleeProfFactor);
          assertx(budgetBCInstrs >= 0);

          inl.registerEndInlining(callee);

          if (result != TranslateResult::Success) {
            // Generating the inlined call failed, bailout
            return result;
          }

          // Native calls end inlining before CallBuiltin
          if (!callee->isCPPBuiltin()) {
            // Start a new IR block to hold the remainder of this block.
            auto const did_start =
              irb.startBlock(returnBlock, false /* unprocessedPred */);

            // If the inlined region failed to contain any returns then the
            // rest of this block is dead- we could continue but there's no
            // benefit to inlining this call if it ends in a ReqRetranslate or
            // ReqBind* so instead we mark it as uninlinable and retry.
            if (!did_start) {
              retry.inlineBlacklist.insert(psk);
              return TranslateResult::Retry;
            }

            irgen::endInlining(irgs);
          } else {
            // For native calls we don't use a return block
            assertx(returnBlock->empty());
          }

          // Recursive calls to irGenRegion will reset the successor block
          // mapping
          setSuccIRBlocks(irgs, region, blockId, blockIdToIRBlock);

          // Don't emit the FCall
          skipTrans = true;
        }
      }

      // Emit IR for the body of the instruction.
      try {
        if (!skipTrans) {
          const bool firstInstr = isEntry && i == 0;
          translateInstr(irgs, inst, checkOuterTypeOnly, firstInstr);
        }
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk{irgs.profTransID, sk};
        always_assert_flog(!retry.toInterp.count(psk),
                           "IR generation failed with {}\n",
                           exn.what());
        FTRACE(1, "ir generation for {} failed with {}\n",
          inst.toString(), exn.what());
        retry.toInterp.insert(psk);
        return TranslateResult::Retry;
      }

      irgen::finishHHBC(irgs);

      skipTrans = false;

      // If this is the last instruction, handle block transitions.
      // If the block ends the region, then call irgen::endRegion to
      // sync the state and make a REQ_BIND_JMP service request.
      // Otherwise, if the instruction has a fall-through, then insert
      // a jump to the next offset, since it may not be the next block
      // to be translated.
      if (lastInstr) {
        if (region.isExit(blockId)) {
          if (!inl.inlining() || !isReturnish(inst.op())) {
            irgen::endRegion(irgs);
          }
        } else if (instrAllowsFallThru(inst.op())) {
          irgen::endBlock(irgs, inst.nextSk().offset());
        }
      }
    }

    processedBlocks.insert(blockId);

    assertx(!byRefs.hasNext());
    assertx(!knownFuncs.hasNext());
  }

  if (!inl.inlining()) {
    irgen::sealUnit(irgs);
  }

  irGenTimer.stop();
  return TranslateResult::Success;
}

TranslateResult mcGenRegion(IRGS& irgs,
                            const RegionDesc& region,
                            ProfSrcKeySet& toInterp) {
  auto const startSk = region.start();
  try {
    mcg->traceCodeGen(irgs);
    if (mcg->tx().mode() == TransKind::Profile) {
      mcg->tx().profData()->setProfiling(startSk.func()->getFuncId());
    }
  } catch (const FailedCodeGen& exn) {
    SrcKey sk{exn.vmFunc, exn.bcOff, exn.resumed};
    ProfSrcKey psk{exn.profTransId, sk};
    always_assert_log(
      !toInterp.count(psk),
      [&] {
        std::ostringstream oss;
        oss << folly::format("code generation failed with {}\n", exn.what());
        print(oss, irgs.irb->unit());
        return oss.str();
      });
    toInterp.insert(psk);
    return TranslateResult::Retry;
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      assertx(mcg->tx().useAHot());
      mcg->tx().setUseAHot(false);
      // We can't return Retry here because the code block selection
      // will still say hot.
      return TranslateResult::Failure;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }
  return TranslateResult::Success;
}

}

//////////////////////////////////////////////////////////////////////

TranslateResult translateRegion(IRGS& irgs,
                                const RegionDesc& region,
                                TranslateRetryContext& retry,
                                TransFlags trflags,
                                PostConditions& pConds) {
  SCOPE_ASSERT_DETAIL("RegionDesc") { return show(region); };
  SCOPE_ASSERT_DETAIL("IRUnit") { return show(irgs.unit); };

  // Set up inlining context, but disable it for profiling mode.
  InliningDecider inl(region.entry()->func());
  if (mcg->tx().mode() == TransKind::Profile) inl.disable();

  // Set the profCount of the IRUnit's entry block, which is created a priori.
  if (mcg->tx().mode() == TransKind::Optimize) {
    auto entryBID = region.entry()->id();
    assertx(hasTransID(entryBID));
    auto entryTID = getTransID(entryBID);
    auto entryProfCount = mcg->tx().profData()->transCounter(entryTID);
    irgs.unit.entry()->setProfCount(entryProfCount);
  }
  int32_t budgetBCInstrs = RuntimeOption::EvalJitMaxRegionInstrs;
  auto irGenResult = irGenRegion(irgs, region, retry, trflags, inl,
                                 budgetBCInstrs, 1);
  assertx(budgetBCInstrs >= 0);
  FTRACE(1, "translateRegion: final budgetBCInstrs = {}\n", budgetBCInstrs);
  if (irGenResult != TranslateResult::Success) return irGenResult;

  // For profiling translations, grab the postconditions to be used
  // for region selection whenever we decide to retranslate.
  pConds.changed.clear();
  pConds.refined.clear();
  if (mcg->tx().mode() == TransKind::Profile &&
      RuntimeOption::EvalJitPGOUsePostConditions) {
    auto& unit = irgs.irb->unit();
    auto  lastSrcKey = region.lastSrcKey();
    Block* mainExit = findMainExitBlock(unit, lastSrcKey);
    FTRACE(2, "translateRegion: mainExit: B{}\nUnit: {}\n",
           mainExit->id(), show(unit));
    assertx(mainExit);
    pConds = irgs.irb->postConds(mainExit);
  }

  return mcGenRegion(irgs, region, retry.toInterp);
}

} }
