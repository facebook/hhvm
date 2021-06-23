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
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/id-set.h"

namespace HPHP { namespace jit {

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

Optional<IRSPRelOffset> offsetOfFrame(SSATmp *fp) {
  assertx(fp->isA(TFramePtr));
  auto const inst = canonical(fp)->inst();
  if (inst->is(DefFP)) return inst->extra<DefFP>()->offset;
  if (inst->is(DefFuncEntryFP)) return IRSPRelOffset { 0 };
  if (inst->is(BeginInlining)) return inst->extra<BeginInlining>()->spOffset;
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

}}
