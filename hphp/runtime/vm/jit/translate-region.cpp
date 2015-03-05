/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
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
void createBlockMap(HTS& hts,
                    const RegionDesc& region,
                    BlockIdToIRBlockMap& blockIdToIRBlock) {
  auto& irb = *hts.irb;
  blockIdToIRBlock.clear();
  auto const& blocks = region.blocks();
  for (unsigned i = 0; i < blocks.size(); i++) {
    auto rBlock = blocks[i];
    auto id = rBlock->id();
    DEBUG_ONLY Offset bcOff = rBlock->start().offset();
    assert(IMPLIES(i == 0, bcOff == irb.unit().bcOff()));

    // NB: This maps the region entry block to a new IR block, even though
    // we've already constructed an IR entry block. We'll make the IR entry
    // block jump to this block.
    Block* iBlock = irb.unit().defBlock();

    blockIdToIRBlock[id] = iBlock;
    FTRACE(1,
           "createBlockMaps: RegionBlock {} => IRBlock {} (BC offset = {})\n",
           id, iBlock->id(), bcOff);
  }
}

/*
 * Set IRBuilder's Block associated to blockId's block according to
 * the mapping in blockIdToIRBlock.
 */
void setIRBlock(HTS& hts,
                RegionDesc::BlockId blockId,
                const RegionDesc& region,
                const BlockIdToIRBlockMap& blockIdToIRBlock) {
  auto& irb = *hts.irb;
  auto rBlock = region.block(blockId);
  Offset bcOffset = rBlock->start().offset();

  auto iit = blockIdToIRBlock.find(blockId);
  assert(iit != blockIdToIRBlock.end());

  assert(!irb.hasBlock(bcOffset));
  FTRACE(3, "  setIRBlock: blockId {}, offset {} => IR Block {}\n",
         blockId, bcOffset, iit->second->id());
  irb.setBlock(bcOffset, iit->second);
}

/*
 * Set IRBuilder's Blocks for srcBlockId's successors' offsets within
 * the region.  It also sets the guard-failure block, if any.
 */
void setSuccIRBlocks(HTS& hts,
                     const RegionDesc& region,
                     RegionDesc::BlockId srcBlockId,
                     const BlockIdToIRBlockMap& blockIdToIRBlock) {
  FTRACE(3, "setSuccIRBlocks: srcBlockId = {}\n", srcBlockId);
  auto& irb = *hts.irb;
  irb.resetOffsetMapping();
  for (auto dstBlockId : region.succs(srcBlockId)) {
    setIRBlock(hts, dstBlockId, region, blockIdToIRBlock);
  }
  if (auto nextRetrans = region.nextRetrans(srcBlockId)) {
    auto it = blockIdToIRBlock.find(nextRetrans.value());
    assert(it != blockIdToIRBlock.end());
    irb.setGuardFailBlock(it->second);
  } else {
    irb.resetGuardFailBlock();
  }
}

bool blockIsLoopHeader(
  const RegionDesc&             region,
  RegionDesc::BlockId           blockId,
  const RegionDesc::BlockIdSet& processedBlocks)
{
  for (auto predId : region.preds(blockId)) {
    if (processedBlocks.count(predId) == 0) {
      return true;
    }
  }
  return false;
}

/*
 * Returns whether or not block `blockId' is a merge point in the
 * `region'.  Normally, blocks are merge points if they have more than
 * one predecessor.  However, the region's entry block is a
 * merge-point if it has any successor, since it has an implicit arc
 * coming from outside of the region.  Additionally, a block with a
 * single predecessor is a merge point if it's the target of both the
 * fallthru and the taken paths.
 */
static bool isMerge(const RegionDesc& region, RegionDesc::BlockId blockId) {
  auto const& preds = region.preds(blockId);
  if (preds.size() == 0)               return false;
  if (preds.size() > 1)                return true;
  if (blockId == region.entry()->id()) return true;

  // The destination of a conditional jump is a merge point if both
  // the fallthru and taken offsets are the same.
  auto predId   = *preds.begin();
  Op* predOpPtr = (Op*)(region.block(predId)->last().pc());
  auto predOp   = *predOpPtr;
  if (!instrHasConditionalBranch(predOp)) return false;
  Offset fallthruOffset = instrLen(predOpPtr);
  Offset    takenOffset = *instrJumpOffset(predOpPtr);
  return fallthruOffset == takenOffset;
}

/*
 * Returns whether any successor of `blockId' in the `region' is a
 * merge-point within the `region'.
 */
static bool hasMergeSucc(const RegionDesc&   region,
                         RegionDesc::BlockId blockId) {
  for (auto succ : region.succs(blockId)) {
    if (isMerge(region, succ)) return true;
  }
  return false;
}

/*
 * Emit type and reffiness prediction guards.
 */
void emitPredictionGuards(HTS& hts,
                          const RegionDesc& region,
                          const RegionDesc::BlockPtr block,
                          bool useGuards) {
  auto sk = block->start();
  auto bcOff = sk.offset();
  auto typePreds = makeMapWalker(block->typePreds());
  auto refPreds  = makeMapWalker(block->reffinessPreds());

  // If the block has a next retranslations in the chain that is a
  // merge point in the region, then we need to call
  // prepareForHHBCMergePoint to spill the stack.
  if (auto retrans = region.nextRetrans(block->id())) {
    if (isMerge(region, retrans.value())) {
      irgen::prepareForHHBCMergePoint(hts);
    }
  }

  if (useGuards) irgen::ringbuffer(hts, Trace::RBTypeTraceletGuards, sk);

  // Emit type guards.
  while (typePreds.hasNext(sk)) {
    auto const& pred = typePreds.next();
    auto type = pred.type;
    auto loc  = pred.location;
    if (type <= Type::Cls) {
      // Do not generate guards for class; instead assert the type.
      assert(loc.tag() == RegionDesc::Location::Tag::Stack);
      irgen::assertTypeLocation(hts, loc, type);
    } else if (useGuards) {
      bool checkOuterTypeOnly = mcg->tx().mode() != TransKind::Profile;
      irgen::guardTypeLocation(hts, loc, type, checkOuterTypeOnly);
    } else {
      irgen::checkTypeLocation(hts, loc, type, bcOff);
    }
  }

  // Emit reffiness guards.
  while (refPreds.hasNext(sk)) {
    auto const& pred = refPreds.next();
    if (useGuards) {
      irgen::guardRefs(hts, pred.arSpOffset, pred.mask, pred.vals);
    } else {
      irgen::checkRefs(hts, pred.arSpOffset, pred.mask, pred.vals, bcOff);
    }
  }

  // Finish emitting guards, and emit profiling counters.
  if (useGuards) {
    irgen::gen(hts, EndGuards);
    if (RuntimeOption::EvalJitTransCounters) {
      irgen::incTransCounter(hts);
    }

    if (mcg->tx().mode() == TransKind::Profile) {
      if (block->func()->isEntry(bcOff)) {
        irgen::checkCold(hts, mcg->tx().profData()->curTransID());
      } else {
        irgen::incProfCounter(hts, mcg->tx().profData()->curTransID());
      }
    }
    irgen::ringbuffer(hts, Trace::RBTypeTraceletBody, sk);
  }

  // In the entry block, hhbc-translator gets a chance to emit some code
  // immediately after the initial guards/checks on the first instruction.
  if (block == region.entry()) {
    switch (arch()) {
      case Arch::X64:
        irgen::prepareEntry(hts);
        break;
      case Arch::ARM:
        // Don't do this for ARM, because it can lead to interpOne on the
        // first SrcKey in a translation, which isn't allowed.
        break;
    }
  }

  assert(!typePreds.hasNext());
  assert(!refPreds.hasNext());
}

void initNormalizedInstruction(
  NormalizedInstruction& inst,
  jit::vector<DynLocation>& dynLocs,
  MapWalker<RegionDesc::Block::ParamByRefMap>& byRefs,
  HTS& hts,
  const RegionDesc& region,
  RegionDesc::BlockId blockId,
  const Func* topFunc,
  bool lastInstr,
  bool toInterp) {

  inst.funcd = topFunc;

  if (lastInstr) {
    inst.endsRegion  = region.isExit(blockId);
    inst.nextIsMerge = hasMergeSucc(region, blockId);
  }

  // We can get a more precise output type for interpOne if we know all of
  // its inputs, so we still populate the rest of the instruction even if
  // this is true.
  inst.interp = toInterp;

  auto const inputInfos = getInputs(inst);

  // Populate the NormalizedInstruction's input vector, using types
  // from IRBuilder.
  dynLocs.reserve(inputInfos.size());
  auto newDynLoc = [&] (const InputInfo& ii) {
    dynLocs.emplace_back(ii.loc,
                         irgen::predictedTypeFromLocation(hts, ii.loc));
    FTRACE(2, "predictedTypeFromLocation: {} -> {}\n",
           ii.loc.pretty(), dynLocs.back().rtt);
    return &dynLocs.back();
  };

  FTRACE(2, "populating inputs for {}\n", inst.toString());
  for (auto const& ii : inputInfos) {
    inst.inputs.push_back(newDynLoc(ii));
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

  auto nextOp = inst.nextSk().op();

  // If the normal machinery is already inlining this function, don't
  // do anything here.
  if (instIdx == block->length() - 2 &&
      (nextOp == Op::FCall || nextOp == Op::FCallD) &&
      block->inlinedCallee()) {
    return false;
  }

  return true;
}

/*
 * Check if `i' is an FPush{Func,ClsMethod}D followed by an FCall{,D} to a
 * function with a singleton pattern, and if so, inline it.  Returns true if
 * this succeeds, else false.
 */
bool tryTranslateSingletonInline(HTS& hts,
                                 const NormalizedInstruction& ninst,
                                 const Func* funcd) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  if (!funcd) return false;

  // Make sure we have an acceptable FPush and non-null callee.
  assert(ninst.op() == Op::FPushFuncD ||
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

    auto cgetl = (const Op*)pc;
    auto sli = (const Op*)captures[0];

    assert(*cgetl == Op::CGetL);
    assert(*sli == Op::StaticLocInit);

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
      irgen::prepareForNextHHBC(hts, nullptr, ninst.offset(), false);
      irgen::inlSingletonSLoc(
        hts,
        funcd,
        (const Op*)result.getCapture(0)
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
      auto string1 = (const Op*)pc;
      auto string2 = (const Op*)captures[i];
      assert(*string1 == Op::String);
      assert(*string2 == Op::String);

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
      irgen::prepareForNextHHBC(hts, nullptr, ninst.offset(), false);
      irgen::inlSingletonSProp(
        hts,
        funcd,
        (const Op*)result.getCapture(1),
        (const Op*)result.getCapture(0)
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

TranslateResult irGenRegion(HTS& hts,
                            const RegionDesc& region,
                            RegionBlacklist& toInterp,
                            TransFlags trflags) {
  const Timer translateRegionTimer(Timer::translateRegion);
  FTRACE(1, "translateRegion starting with:\n{}\n", show(region));

  std::string errorMsg;
  always_assert_flog(check(region, errorMsg), "{}", errorMsg);

  auto& irb = *hts.irb;

  // Create a map from region blocks to their corresponding initial IR blocks.
  BlockIdToIRBlockMap blockIdToIRBlock;
  createBlockMap(hts, region, blockIdToIRBlock);

  // Prepare to start translation of the first region block.
  auto const entry = irb.unit().entry();
  irb.startBlock(entry, entry->front().marker(), false /* isLoopHeader */);

  // Make the IR entry block jump to the IR block we mapped the region entry
  // block to (they are not the same!).
  auto const irBlock = blockIdToIRBlock[region.entry()->id()];
  always_assert(irBlock != entry);
  irgen::gen(hts, Jmp, irBlock);

  RegionDesc::BlockIdSet processedBlocks;

  Timer irGenTimer(Timer::translateRegion_irGeneration);
  auto& blocks = region.blocks();

  for (auto b = 0; b < blocks.size(); b++) {
    auto const& block  = blocks[b];
    auto const blockId = block->id();
    auto sk            = block->start();
    auto byRefs        = makeMapWalker(block->paramByRefs());
    auto knownFuncs    = makeMapWalker(block->knownFuncs());
    auto skipTrans     = false;

    SCOPE_ASSERT_DETAIL("HTS") { return show(hts); };

    const Func* topFunc = nullptr;
    TransID profTransId = getTransId(blockId);
    hts.profTransID = profTransId;

    // Prepare to start translating this region block.  This loads the
    // FrameState for the IR block corresponding to the start of this
    // region block, and it also sets the map from BC offsets to IR
    // blocks for the successors of this block in the region.
    Block* irBlock = blockIdToIRBlock[blockId];
    bool isLoopHeader = blockIsLoopHeader(region, blockId, processedBlocks);
    always_assert(IMPLIES(isLoopHeader, RuntimeOption::EvalJitLoops));
    BCMarker marker(sk, block->initialSpOffset(), profTransId);
    if (!irb.startBlock(irBlock, marker, isLoopHeader)) {
      FTRACE(1, "translateRegion: block {} is unreachable, skipping\n",
             blockId);
      processedBlocks.insert(blockId);
      continue;
    }
    setSuccIRBlocks(hts, region, blockId, blockIdToIRBlock);

    // Emit the type and reffiness predictions for this region block.
    // If this is the first instruction in the region, and the
    // region's entry block is not a loop header, the guards will go
    // to a retranslate request.  Otherwise, they'll go to a side
    // exit.
    auto const useGuards = block == region.entry() && !isLoopHeader;
    emitPredictionGuards(hts, region, block, useGuards);
    irb.resetGuardFailBlock();

    // Generate IR for each bytecode instruction in this block.
    for (unsigned i = 0; i < block->length(); ++i, sk.advance(block->unit())) {
      auto const lastInstr = i == block->length() - 1;

      // Update bcOff here so any guards or assertions from metadata are
      // attributed to this instruction.
      irgen::prepareForNextHHBC(hts, nullptr, sk.offset(), false);

      // Update the current funcd, if we have a new one.
      if (knownFuncs.hasNext(sk)) {
        topFunc = knownFuncs.next();
      }

      // Create and initialize the instruction.
      NormalizedInstruction inst(sk, block->unit());
      jit::vector<DynLocation> dynLocs;
      bool toInterpInst = toInterp.count(ProfSrcKey{profTransId, sk});
      initNormalizedInstruction(inst, dynLocs, byRefs, hts, region, blockId,
                                topFunc, lastInstr, toInterpInst);

      // If this block ends with an inlined FCall, we don't emit anything for
      // the FCall and instead set up HhbcTranslator for inlining. Blocks from
      // the callee will be next in the region.
      if (lastInstr && block->inlinedCallee()) {
        always_assert(inst.op() == Op::FCall || inst.op() == Op::FCallD);
        auto const* callee = block->inlinedCallee();
        FTRACE(1, "\nstarting inlined call from {} to {} with {} args "
               "and stack:\n{}\n",
               block->func()->fullName()->data(),
               callee->fullName()->data(),
               inst.imm[0].u_IVA,
               show(hts));
        auto returnSk = inst.nextSk();
        auto returnFuncOff = returnSk.offset() - block->func()->base();
        irgen::beginInlining(hts, inst.imm[0].u_IVA, callee, returnFuncOff);
        // "Fallthrough" into the callee's first block
        irgen::endBlock(hts, blocks[b + 1]->start().offset(), inst.nextIsMerge);
        continue;
      }

      // Singleton inlining optimization.
      if (RuntimeOption::EvalHHIRInlineSingletons && !lastInstr &&
          shouldTrySingletonInline(region, block, inst, i, trflags) &&
          knownFuncs.hasNext(inst.nextSk())) {

        // This is safe to do even if singleton inlining fails; we just won't
        // change topFunc in the next pass since hasNext() will return false.
        topFunc = knownFuncs.next();

        if (tryTranslateSingletonInline(hts, inst, topFunc)) {
          // Skip the translation of this instruction (the FPush) -and- the
          // next instruction (the FCall) if singleton inlining succeeds.
          // We still want the fallthrough and prediction logic, though.
          skipTrans = true;
          continue;
        }
      }

      // Emit IR for the body of the instruction.
      try {
        if (!skipTrans) translateInstr(hts, inst);
      } catch (const FailedIRGen& exn) {
        ProfSrcKey psk{profTransId, sk};
        always_assert_flog(!toInterp.count(psk),
                           "IR generation failed with {}\n",
                           exn.what());
        toInterp.insert(psk);
        return TranslateResult::Retry;
      }

      skipTrans = false;

      // If this is the last instruction, handle block transitions.
      // If the block ends the region, then call irgen::endRegion to
      // sync the state and make a REQ_BIND_JMP service request.
      // Otherwise, if the instruction has a fall-through, then insert
      // a jump to the next offset, since it may not be the next block
      // to be translated.
      if (lastInstr) {
        if (region.isExit(blockId)) {
          irgen::endRegion(hts);
        } else if (instrAllowsFallThru(inst.op())) {
          if (region.isSideExitingBlock(blockId)) {
            irgen::prepareForSideExit(hts);
          }
          irgen::endBlock(hts, inst.nextSk().offset(), inst.nextIsMerge);
        } else if (isRet(inst.op()) || inst.op() == OpNativeImpl) {
          // "Fallthrough" from inlined return to the next block
          irgen::endBlock(hts, blocks[b+1]->start().offset(), inst.nextIsMerge);
        }
      }
    }

    processedBlocks.insert(blockId);

    assert(!byRefs.hasNext());
    assert(!knownFuncs.hasNext());
  }
  irGenTimer.end();
  return TranslateResult::Success;
}

TranslateResult mcGenRegion(HTS& hts,
                            const RegionDesc& region,
                            RegionBlacklist& toInterp) {
  auto const startSk = region.start();
  try {
    mcg->traceCodeGen(hts);
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
        print(oss, hts.irb->unit());
        return oss.str();
      });
    toInterp.insert(psk);
    return TranslateResult::Retry;
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      assert(mcg->tx().useAHot());
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

TranslateResult translateRegion(HTS& hts,
                                const RegionDesc& region,
                                RegionBlacklist& toInterp,
                                TransFlags trflags,
                                PostConditions& pConds) {
  SCOPE_ASSERT_DETAIL("RegionDesc") { return show(region); };
  SCOPE_ASSERT_DETAIL("IRUnit") { return hts.unit.toString(); };

  auto irGenResult = irGenRegion(hts, region, toInterp, trflags);
  if (irGenResult != TranslateResult::Success) return irGenResult;

  // For profiling translations, grab the postconditions to be used
  // for region selection whenever we decide to retranslate.
  pConds.clear();
  if (mcg->tx().mode() == TransKind::Profile &&
      RuntimeOption::EvalJitPGOUsePostConditions) {
    auto& unit = hts.irb->unit();
    auto  lastSrcKey = region.lastSrcKey();
    Block* mainExit = findMainExitBlock(unit, lastSrcKey);
    FTRACE(2, "translateRegion: mainExit: B{}\nUnit: {}\n",
           mainExit->id(), unit);
    assert(mainExit);
    pConds = hts.irb->postConds(mainExit);
  }

  return mcGenRegion(hts, region, toInterp);
}

} }
