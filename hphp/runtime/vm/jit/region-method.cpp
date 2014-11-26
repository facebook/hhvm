/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/arena.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/verifier/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(region);

//////////////////////////////////////////////////////////////////////

namespace {

bool isFuncEntry(const Func* func, Offset off) {
  return off == func->base();
}

int numInstrs(PC start, PC end) {
  int ret{};
  for (; start != end; ++ret) {
    start += instrLen((Op*)start);
  }
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Region-selector that unintelligently takes a whole method at a
 * time.  This is primarily intended for use for debugging and
 * development on the JIT.
 *
 * Adds no type annotations to the region beyond those known from the
 * context.  It will list only the parameter types as guards.
 *
 * If the context is not a method entry point, returns nullptr to fall
 * back to the tracelet compiler.  (This will happen for side-exits
 * from method regions, for example.)
 */
RegionDescPtr selectMethod(const RegionContext& context) {
  using namespace HPHP::Verifier;

  if (!isFuncEntry(context.func, context.bcOffset)) return nullptr;
  if (context.func->isPseudoMain()) return nullptr;
  FTRACE(1, "function entry for {}: using selectMethod\n",
         context.func->fullName()->data());

  auto ret = std::make_shared<RegionDesc>();

  Arena arena;
  GraphBuilder gb(arena, context.func);
  auto const graph = gb.build();
  auto const unit = context.func->unit();

  jit::hash_map<Block*,RegionDesc::BlockId> blockMap;

  /*
   * Spit out the blocks in a RPO, but skip DV-initializer blocks
   * (i.e. start with graph->first_linear.  We don't handle those in
   * our method regions for now---they'll get handled by the tracelet
   * compiler and then may branch to the main entry point.
   */
  sortRpo(graph);
  {
    auto spOffset = Offset{context.spOffset};
    for (Block* b = graph->first_linear; b != nullptr; b = b->next_rpo) {
      auto const start  = unit->offsetOf(b->start);
      auto const length = numInstrs(b->start, b->end);
      SrcKey sk{context.func, start, context.resumed};
      auto const rblock = ret->addBlock(sk, length, spOffset);
      blockMap[b] = rblock->id();
      spOffset = -1; // flag SP offset as unknown for all but the first block
    }
  }

  // Add all the ARCs.
  for (Block* b = graph->first_linear; b != nullptr; b = b->next_rpo) {
    auto const myId = blockMap[b];
    auto const numSuccs = numSuccBlocks(b);
    for (auto i = uint32_t{0}; i < numSuccs; ++i) {
      auto const succIt = blockMap.find(b->succs[i]);
      if (succIt != end(blockMap)) {
        ret->addArc(myId, succIt->second);
      }
    }
  }

  // Compute stack depths for each block.
  for (Block* b = graph->first_linear; b != nullptr; b = b->next_rpo) {
    uint32_t sp = ret->block(blockMap[b])->initialSpOffset();
    always_assert_flog(sp != -1, "sp wasn't negative one on block {}\n",
      context.func->unit()->offsetOf(b->start));
    for (InstrRange inst = blockInstrs(b); !inst.empty();) {
      auto const pc   = inst.popFront();
      auto const info = instrStackTransInfo(reinterpret_cast<const Op*>(pc));
      switch (info.kind) {
      case StackTransInfo::Kind::InsertMid:
        ++sp;
        break;
      case StackTransInfo::Kind::PushPop:
        sp += info.numPushes - info.numPops;
        break;
      }
    }

    for (auto idx = uint32_t{0}; idx < numSuccBlocks(b); ++idx) {
      if (!b->succs[idx]) continue;
      auto const succ = ret->block(blockMap[b->succs[idx]]);
      if (succ->initialSpOffset() != -1) {
        always_assert_flog(
          succ->initialSpOffset() == sp,
          "Stack depth mismatch in region method on {}\n"
          "  srcblkoff={}, dstblkoff={}, src={}, target={}",
          context.func->fullName()->data(),
          context.func->unit()->offsetOf(b->start),
          context.func->unit()->offsetOf(b->succs[idx]->start),
          sp,
          succ->initialSpOffset()
        );
        continue;
      }
      succ->setInitialSpOffset(sp);
      FTRACE(2,
        "spOff for {} -> {}\n",
        context.func->unit()->offsetOf(b->succs[idx]->start),
        sp
      );
    }
  }

  /*
   * Fill the first block predictions with the live types.
   */
  assert(!ret->empty());
  auto const startSK = ret->start();
  for (auto& lt : context.liveTypes) {
    typedef RegionDesc::Location::Tag LTag;

    switch (lt.location.tag()) {
    case LTag::Stack:
      break;
    case LTag::Local:
      if (lt.location.localId() < context.func->numParams()) {
        // Only predict objectness, not the specific class type.
        auto const type = lt.type.strictSubtypeOf(Type::Obj)
                           ? Type::Obj
                           : lt.type;
        ret->entry()->addPredicted(startSK, {lt.location, type});
      }
      break;
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
