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
#include "hphp/runtime/vm/jit/analysis.h"

#include "hphp/util/assertions.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/id-set.h"

namespace HPHP::jit {

//////////////////////////////////////////////////////////////////////

const SSATmp* canonical(const SSATmp* val) {
  return canonical(const_cast<SSATmp*>(val));
}

SSATmp* canonical(SSATmp* value) {
  if (value == nullptr) return nullptr;

  auto inst = value->inst();

  while (true) {
    if (inst->isPassthrough()) {
      value = inst->getPassthroughValue();
    } else if (inst->is(ConvPtrToLval)) {
      // ConvPtrToLval is special in that its not a passthrough instruction
      // (because the dest has a type incompatible with the source and might not
      // be a nop), but we still want to peer through it in order to find the
      // value's utimate origin.
      value = inst->src(0);
    } else {
      break;
    }
    inst = value->inst();
  }
  return value;
}

//////////////////////////////////////////////////////////////////////

Block* ultimateDst(Block* blk) {
  if (blk == nullptr) return nullptr;

  auto constexpr kMaxSteps = 10;

  for (auto i = 0; i < kMaxSteps; ++i) {
    if (blk->empty()) return blk;
    auto jmp = blk->begin();
    if (!jmp->is(Jmp) || jmp->numSrcs() != 0 || jmp->taken() == nullptr) {
      return blk;
    }
    blk = jmp->taken();
  }

  return blk;
}

//////////////////////////////////////////////////////////////////////

Block* findDefiningBlock(const SSATmp* t, const IdomVector& idoms) {
  assertx(!t->inst()->is(DefConst));
  auto const srcInst = t->inst();

  if (srcInst->hasEdges()) {
    auto const next = srcInst->next();
    UNUSED auto const taken = srcInst->taken();
    always_assert_flog(
      next && taken,
      "hasEdges instruction defining a dst had no edges:\n  {}\n",
      srcInst->toString()
    );
    // A branch which goes to the same block on both edges will pass
    // the below dominance check, but the SSATmp it produces is never
    // usable.
    if (next == taken) return nullptr;
    for (const auto& arc : next->preds()) {
      auto pred = arc.from();
      if (pred != srcInst->block() && !dominates(next, pred, idoms)) {
        return nullptr;
      }
    }
    return next;
  }

  return srcInst->block();
}

//////////////////////////////////////////////////////////////////////

bool is_tmp_usable(const IdomVector& idoms,
                   const SSATmp* tmp,
                   const Block* where) {
  if (tmp->inst()->is(DefConst)) return true;
  auto const definingBlock = findDefiningBlock(tmp, idoms);
  if (!definingBlock) return false;
  return dominates(definingBlock, where, idoms);
}

//////////////////////////////////////////////////////////////////////

SSATmp* least_common_ancestor(SSATmp* s1, SSATmp* s2) {
  if (s1 == s2) return s1;
  if (s1 == nullptr || s2 == nullptr) return nullptr;

  IdSet<SSATmp> seen;

  auto const step = [] (SSATmp* v) {
    assertx(v != nullptr);
    return v->inst()->isPassthrough() ?
      v->inst()->getPassthroughValue() :
      nullptr;
  };

  auto const process = [&] (SSATmp*& v) {
    if (v == nullptr) return false;
    if (seen[v]) return true;
    seen.add(v);
    v = step(v);
    return false;
  };

  while (s1 != nullptr || s2 != nullptr) {
    if (process(s1)) return s1;
    if (process(s2)) return s2;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////

const Func* funcFromFP(const SSATmp* fp) {
  auto const inst = canonical(fp)->inst();
  if (inst->is(DefFP)) return inst->marker().func();
  if (inst->is(DefFuncEntryFP)) return inst->extra<DefFuncEntryFP>()->func;
  if (inst->is(BeginInlining)) return inst->extra<BeginInlining>()->func;
  always_assert(false);
}

uint32_t frameDepthIndex(const SSATmp* fp) {
  always_assert(fp->isA(TFramePtr));
  fp = canonical(fp);
  if (fp->inst()->is(BeginInlining)) {
    auto const extra = fp->inst()->extra<BeginInlining>();
    return extra->depth;
  }
  return 0;
}

Optional<IRSPRelOffset> offsetOfFrame(const SSATmp *fp) {
  assertx(fp->isA(TFramePtr));
  auto const inst = canonical(fp)->inst();
  if (inst->is(BeginInlining)) return inst->extra<BeginInlining>()->spOffset;
  auto const resumed = inst->marker().sk().resumeMode() != ResumeMode::None;
  if (inst->is(DefFP)) {
    if (resumed) return std::nullopt;
    return inst->extra<DefFP>()->offset;
  }
  if (inst->is(DefFuncEntryFP)) {
    if (resumed) return std::nullopt;
    return IRSPRelOffset { 0 };
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

std::pair<const IRInstruction*, const SSATmp*>
EveryDefiningInstVisitor::next() {
  if (m_stack.empty()) return {nullptr, nullptr};
  auto const t = m_stack.back();
  m_stack.pop_back();

  if (t->isA(TBottom)) return next();

  auto const canonT = canonical(t);
  auto const inst = canonT->inst();

  if (!inst->is(DefLabel)) return {inst, t};
  if (m_visited.count(inst)) return next();
  m_visited.emplace(inst);

  auto const dsts = inst->dsts();
  auto const dstIdx =
    std::find(dsts.begin(), dsts.end(), canonT) - dsts.begin();
  always_assert(dstIdx >= 0 && dstIdx < inst->numDsts());

  inst->block()->forEachSrc(
    dstIdx,
    [&] (const IRInstruction*, const SSATmp* s) {
      m_stack.emplace_back(s);
    }
  );

  return next();
}

//////////////////////////////////////////////////////////////////////

}
