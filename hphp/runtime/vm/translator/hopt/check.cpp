/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/hopt/check.h"
#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/irfactory.h"
#include "hphp/runtime/vm/translator/hopt/linearscan.h"
#include "hphp/runtime/vm/translator/physreg.h"

namespace HPHP {  namespace JIT {

/*
 * Check one block for being well formed.  It must:
 * 1. have exactly one DefLabel as the first instruction
 * 2. if any instruction is isBlockEnd(), it must be last
 * 3. if the last instruction isTerminal(), block->next must be null.
 */
bool checkBlock(Block* b) {
  assert(!b->empty());
  assert(b->front()->op() == DefLabel);
  if (b->back()->isTerminal()) assert(!b->getNext());
  if (b->getTaken()) {
    // only Jmp_ can branch to a join block expecting values.
    assert(b->back()->op() == Jmp_ ||
           b->getTaken()->front()->getNumDsts() == 0);
  }
  if (b->getNext()) {
    // cannot fall-through to join block expecting values
    assert(b->getNext()->front()->getNumDsts() == 0);
  }
  auto i = b->begin(), e = b->end();
  ++i;
  if (b->back()->isBlockEnd()) --e;
  for (DEBUG_ONLY IRInstruction& inst : folly::makeRange(i, e)) {
    assert(inst.op() != DefLabel);
    assert(!inst.isBlockEnd());
  }
  return true;
}

/*
 * Build the CFG, then the dominator tree, then use it to validate SSA.
 * 1. Each src must be defined by some other instruction, and each dst must
 *    be defined by the current instruction.
 * 2. Each src must be defined earlier in the same block or in a dominator.
 * 3. Each dst must not be previously defined.
 * 4. Treat tmps defined by DefConst as always defined.
 */
bool checkCfg(Trace* trace, const IRFactory& factory) {
  forEachTraceBlock(trace, [&](Block* b) {
    checkBlock(b);
  });
  BlockList blocks = sortCfg(trace, factory);
  std::vector<BlockPtrList> children = findDomChildren(blocks);

  // visit dom tree in preorder, checking all tmps
  boost::dynamic_bitset<> defined0(factory.numTmps());
  forPreorderDoms(blocks.front(), children, defined0,
                  [] (Block* block, boost::dynamic_bitset<>& defined) {
    for (IRInstruction& inst : *block) {
      for (DEBUG_ONLY SSATmp* src : inst.getSrcs()) {
        assert(src->inst() != &inst);
        assert_log(src->inst()->op() == DefConst ||
                   defined[src->getId()],
                   [&]{ return folly::format(
                       "src '{}' in '{}' came from '{}', which is not a "
                       "DefConst and is not defined at this use site",
                       src->toString(), inst.toString(),
                       src->inst()->toString()).str();
                   });
      }
      for (SSATmp& dst : inst.getDsts()) {
        assert(dst.inst() == &inst &&
               inst.op() != DefConst);
        assert(!defined[dst.getId()]);
        defined[dst.getId()] = 1;
      }
    }
  });
  return true;
}

enum Limits : unsigned {
  kNumRegisters = Transl::kNumX64Regs,
  kNumSlots = NumPreAllocatedSpillLocs
};

struct RegState {
  RegState() {
    memset(regs, 0, sizeof(regs));
    memset(slots, 0, sizeof(slots));
  }
  SSATmp* regs[kNumRegisters];  // which tmp is in each register
  SSATmp* slots[kNumSlots]; // which tmp is in each spill slot
  SSATmp*& tmp(const RegisterInfo& info, int i) {
    if (info.spilled()) {
      auto slot = info.getSpillInfo(i).slot();
      assert(unsigned(slot) < kNumSlots);
      return slots[slot];
    }
    auto r = info.getReg(i);
    assert(r != Transl::InvalidReg && unsigned(int(r)) < kNumRegisters);
    return regs[int(r)];
  }
};

bool checkRegisters(Trace* trace, const IRFactory& factory,
                    const RegAllocInfo& regs) {
  assert(checkCfg(trace, factory));

  auto blocks = sortCfg(trace, factory);
  auto children = findDomChildren(blocks);
  forPreorderDoms(blocks.front(), children, RegState(),
                  [&] (Block* block, RegState& state) {
    for (IRInstruction& inst : *block) {
      for (SSATmp* src : inst.getSrcs()) {
        auto const &info = regs[src];
        if (!info.spilled() &&
            (info.getReg(0) == Transl::rVmSp ||
             info.getReg(0) == Transl::rVmFp)) {
          // hack - ignore rbx and rbp
          continue;
        }
        for (unsigned i = 0, n = info.numAllocatedRegs(); i < n; ++i) {
          assert(state.tmp(info, i) == src);
        }
      }
      for (SSATmp& dst : inst.getDsts()) {
        auto const &info = regs[dst];
        for (unsigned i = 0, n = info.numAllocatedRegs(); i < n; ++i) {
          state.tmp(info, i) = &dst;
        }
      }
    }
  });
  return true;
}

}}

