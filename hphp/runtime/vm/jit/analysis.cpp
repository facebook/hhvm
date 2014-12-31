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

  while (inst->isPassthrough()) {
    value = inst->getPassthroughValue();
    inst = value->inst();
  }
  return value;
}

//////////////////////////////////////////////////////////////////////

IRInstruction* findSpillFrame(SSATmp* sp) {
  auto inst = sp->inst();
  while (!inst->is(SpillFrame)) {
    if (debug) {
      [&] {
        for (auto const& dst : inst->dsts()) {
          if (dst.isA(Type::StkPtr)) return;
        }
        assert(false);
      }();
    }

    assert(!inst->is(RetAdjustStack));
    if (inst->is(DefSP)) return nullptr;
    if (inst->is(InterpOne) && isFPush(inst->extra<InterpOne>()->opcode)) {
      // A non-punted translation of this bytecode would contain a SpillFrame.
      return nullptr;
    }

    // M-instr support opcodes have the previous sp in varying sources.
    if (inst->modifiesStack()) inst = inst->previousStkPtr()->inst();
    else                       inst = inst->src(0)->inst();
  }

  return inst;
}

//////////////////////////////////////////////////////////////////////

Block* findDefiningBlock(const SSATmp* t) {
  assert(!t->inst()->is(DefConst));
  auto const srcInst = t->inst();

  /*
   * Instructions that define temporaries that also have edges define the
   * temporary only if they do not take the taken edge.  Since the temporary is
   * defined on the fallthrough edge, this means we only have a block that we
   * can return if this edge is not the only edge leading to its target.
   */
  if (srcInst->hasEdges()) {
    auto const next = srcInst->next();
    UNUSED auto const taken = srcInst->taken();
    always_assert_flog(
      next && taken,
      "hasEdges instruction defining a dst had no edges:\n  {}\n",
      srcInst->toString()
    );
    return next->numPreds() == 1 ? next : nullptr;
  }

  return srcInst->block();
}

//////////////////////////////////////////////////////////////////////

SSATmp* least_common_ancestor(SSATmp* s1, SSATmp* s2) {
  if (s1 == s2) return s1;
  if (s1 == nullptr || s2 == nullptr) return nullptr;

  IdSet<SSATmp> seen;

  auto const step = [] (SSATmp* v) {
    assert(v != nullptr);
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

}}
