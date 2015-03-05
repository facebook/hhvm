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

#include "hphp/runtime/vm/jit/check.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/type.h"

#include <folly/Format.h>

#include <bitset>
#include <iostream>
#include <string>
#include <unordered_set>

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

/*
 * Return the number of parameters required for this block.
 */
DEBUG_ONLY static int numBlockParams(Block* b) {
  return b->empty() || b->front().op() != DefLabel ? 0 :
         b->front().numDsts();
}

/*
 * Check one block for being well formed. Invariants verified:
 * 1. The block begins with an optional DefLabel, followed by an optional
 *    BeginCatch.
 * 2. DefLabel and BeginCatch may not appear anywhere in a block other than
 *    where specified in #1.
 * 3. If this block is a catch block, it must have at most one predecessor.
 * 4. The last instruction must be isBlockEnd() and the middle instructions
 *    must not be isBlockEnd().  Therefore, blocks cannot be empty.
 * 5. If the last instruction isTerminal(), block->next() must be null.
 * 6. Every instruction must have a catch block attached to it if and only if it
 *    has the MayRaiseError flag.
 * 7. Any path from this block to a Block that expects values must be
 *    from a Jmp instruciton.
 * 8. Every instruction's BCMarker must point to a valid bytecode instruction.
 */
bool checkBlock(Block* b) {
  auto it = b->begin();
  auto end = b->end();
  always_assert(!b->empty());

  // Invariant #1
  if (it->op() == DefLabel) {
    ++it;
  }

  // Invariant #1
  if (it != end && it->op() == BeginCatch) {
    ++it;
  }

  // Invariants #2, #4
  always_assert(it != end && b->back().isBlockEnd());
  --end;
  for (IRInstruction& inst : folly::range(it, end)) {
    always_assert(inst.op() != DefLabel);
    always_assert(inst.op() != BeginCatch);
    always_assert(!inst.isBlockEnd());
  }
  for (IRInstruction& inst : *b) {
    // Invariant #8
    always_assert(inst.marker().valid());
    always_assert(inst.block() == b);
    // Invariant #6
    always_assert_log(
      inst.mayRaiseError() == (inst.taken() && inst.taken()->isCatch()),
      [&]{ return inst.toString(); }
    );
  }

  // Invariant #5
  always_assert(IMPLIES(b->back().isTerminal(), !b->next()));

  // Invariant #7
  if (b->taken()) {
    // only Jmp can branch to a join block expecting values.
    IRInstruction* branch = &b->back();
    auto numArgs = branch->op() == Jmp ? branch->numSrcs() : 0;
    always_assert(numBlockParams(b->taken()) == numArgs);
  }

  // Invariant #3
  if (b->isCatch()) {
    // keyed off a tca, so there needs to be exactly one
    always_assert(b->preds().size() <= 1);
  }

  return true;
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Build the CFG, then the dominator tree, then use it to validate SSA.
 * 1. Each src must be defined by some other instruction, and each dst must
 *    be defined by the current instruction.
 * 2. Each src must be defined earlier in the same block or in a dominator.
 * 3. Each dst must not be previously defined.
 * 4. Treat tmps defined by DefConst as always defined.
 * 5. Each predecessor of a reachable block must be reachable (deleted
 *    blocks must not have out-edges to reachable blocks).
 * 6. The entry block must not have any predecessors.
 * 7. The entry block starts with a DefFP instruction.
 */
bool checkCfg(const IRUnit& unit) {
  auto const blocksIds = rpoSortCfgWithIds(unit);
  auto const& blocks = blocksIds.blocks;
  jit::hash_set<const Edge*> edges;

  // Entry block can't have predecessors.
  always_assert(unit.entry()->numPreds() == 0);

  // Entry block starts with DefFP
  always_assert(!unit.entry()->empty() &&
                unit.entry()->begin()->op() == DefFP);

  // Check valid successor/predecessor edges.
  for (Block* b : blocks) {
    auto checkEdge = [&] (const Edge* e) {
      always_assert(e->from() == b);
      edges.insert(e);
      for (auto& p : e->to()->preds()) if (&p == e) return;
      always_assert(false); // did not find edge.
    };
    checkBlock(b);
    if (auto *e = b->nextEdge())  checkEdge(e);
    if (auto *e = b->takenEdge()) checkEdge(e);
  }
  for (Block* b : blocks) {
    for (auto const &e : b->preds()) {
      always_assert(&e == e.inst()->takenEdge() || &e == e.inst()->nextEdge());
      always_assert(e.to() == b);
    }
  }

  // Visit every instruction and make sure their sources are defined in a block
  // that dominates the block containing the instruction.
  auto const idoms = findDominators(unit, blocksIds);
  forEachInst(blocks, [&] (const IRInstruction* inst) {
    for (auto src : inst->srcs()) {
      if (src->inst()->is(DefConst)) continue;
      auto const dom = findDefiningBlock(src);
      always_assert_flog(
        dom && dominates(dom, inst->block(), idoms),
        "src '{}' in '{}' came from '{}', which is not a "
        "DefConst and is not defined at this use site",
        src->toString(), inst->toString(),
        src->inst()->toString()
      );
    }
  });

  return true;
}

bool checkTmpsSpanningCalls(const IRUnit& unit) {
  auto ignoreSrc = [&](IRInstruction& inst, SSATmp* src) {
    /*
     * ReDefSP, TakeStk, and FramePtr/StkPtr-typed tmps are used only for stack
     * analysis in the simplifier and therefore may live across calls. In
     * particular, ReDefSP are used to bridge the logical stack of the caller
     * when a callee is inlined so that analysis does not scan into the callee
     * stack when searching for a type of value in the caller.
     *
     * Tmps defined by DefConst are always available and may be
     * assigned to registers if needed by the instructions using the
     * const.
     */
    return (inst.is(ReDefSP) && src->isA(Type::StkPtr)) ||
           inst.is(TakeStk) ||
           src->isA(Type::StkPtr) ||
           src->isA(Type::FramePtr) ||
           src->inst()->is(DefConst);
  };

  StateVector<Block,IdSet<SSATmp>> livein(unit, IdSet<SSATmp>());
  bool isValid = true;
  postorderWalk(unit, [&](Block* block) {
    auto& live = livein[block];
    if (auto taken = block->taken()) live = livein[taken];
    if (auto next  = block->next()) live |= livein[next];
    for (auto it = block->end(); it != block->begin();) {
      auto& inst = *--it;
      for (auto& dst : inst.dsts()) {
        live.erase(dst);
      }
      if (isCallOp(inst.op())) {
        live.forEach([&](uint32_t tmp) {
          auto msg = folly::format("checkTmpsSpanningCalls failed\n"
                                   "  instruction: {}\n"
                                   "  src:         t{}\n"
                                   "\n"
                                   "Unit:\n"
                                   "{}\n",
                                   inst.toString(),
                                   tmp,
                                   unit.toString()).str();
          std::cerr << msg;
          FTRACE(1, "{}", msg);
          isValid = false;
        });
      }
      for (auto* src : inst.srcs()) {
        if (!ignoreSrc(inst, src)) live.add(src);
      }
    }
  });

  return isValid;
}

///////////////////////////////////////////////////////////////////////////////
// checkOperandTypes().

namespace {

/*
 * Return a union type containing all the types in the argument list.
 */
Type buildUnion() {
  return Type::Bottom;
}

template<class... Args>
Type buildUnion(Type t, Args... ts) {
  return t | buildUnion(ts...);
}

}

/*
 * Runtime typechecking for IRInstruction operands.
 *
 * This is generated using the table in ir-opcode.h.  We instantiate
 * IR_OPCODES after defining all the various source forms to do type
 * assertions according to their form (see ir-opcode.h for documentation on
 * the notation).  The checkers appear in argument order, so each one
 * increments curSrc, and at the end we can check that the argument
 * count was also correct.
 */
bool checkOperandTypes(const IRInstruction* inst, const IRUnit* unit) {
  int curSrc = 0;

  auto bail = [&] (const std::string& msg) {
    FTRACE(1, "{}", msg);
    fprintf(stderr, "%s\n", msg.c_str());

    always_assert_log(false, [&] { return msg; });
  };

  if (opHasExtraData(inst->op()) != (bool)inst->rawExtra()) {
    bail(folly::format("opcode {} should{} have an ExtraData struct "
                       "but instruction {} does{}",
                       inst->op(),
                       opHasExtraData(inst->op()) ? "" : "n't",
                       *inst,
                       inst->rawExtra() ? "" : "n't").str());
  }

  auto src = [&]() -> SSATmp* {
    if (curSrc < inst->numSrcs()) {
      return inst->src(curSrc);
    }

    bail(folly::format(
      "Error: instruction had too few operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
    not_reached();
  };

  // If expected is not nullptr, it will be used. Otherwise, t.toString() will
  // be used as the expected string.
  auto check = [&] (bool cond, const Type t, const char* expected) {
    if (cond) return true;

    std::string expectStr = expected ? expected : t.toString();

    bail(folly::format(
      "Error: failed type check on operand {}\n"
      "   instruction: {}\n"
      "   was expecting: {}\n"
      "   received: {}\n",
        curSrc,
        inst->toString(),
        expectStr,
        inst->src(curSrc)->type().toString()
      ).str()
    );
    return true;
  };

  auto checkNoArgs = [&]{
    if (inst->numSrcs() == 0) return true;
    bail(folly::format(
      "Error: instruction expected no operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
    return true;
  };

  auto countCheck = [&]{
    if (inst->numSrcs() == curSrc) return true;
    bail(folly::format(
      "Error: instruction had too many operands\n"
      "   instruction: {}\n"
      "   expected {} arguments\n",
        inst->toString(),
        curSrc
      ).str()
    );
    return true;
  };

  auto checkDst = [&] (bool cond, const std::string& errorMessage) {
    if (cond) return true;

    bail(folly::format("Error: failed type check on dest operand\n"
                       "   instruction: {}\n"
                       "   message: {}\n",
                       inst->toString(),
                       errorMessage).str());
    return true;
  };

  auto requireTypeParam = [&] {
    checkDst(inst->hasTypeParam() || inst->is(DefConst),
             "Missing paramType for DParam instruction");
  };

  auto requireTypeParamPtr = [&] (Ptr kind) {
    checkDst(inst->hasTypeParam(),
      "Missing paramType for DParamPtr instruction");
    if (inst->hasTypeParam()) {
      checkDst(inst->typeParam() <= Type::Gen.ptr(kind),
               "Invalid paramType for DParamPtr instruction");
    }
  };

  auto checkVariadic = [&] (Type super) {
    for (; curSrc < inst->numSrcs(); ++curSrc) {
      auto const valid = (inst->src(curSrc)->type() <= super);
      check(valid, Type(), nullptr);
    }
  };

#define IRT(name, ...) UNUSED static const Type name = Type::name;
#define IRTP(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP

#define NA            return checkNoArgs();
#define S(...)        {                                   \
                        Type t = buildUnion(__VA_ARGS__); \
                        check(src()->isA(t), t, nullptr); \
                        ++curSrc;                         \
                      }
#define AK(kind)      {                                                 \
                        Type t = Type::Array(ArrayData::k##kind##Kind); \
                        check(src()->isA(t), t, nullptr);               \
                        ++curSrc;                                       \
                      }
#define C(type)       check(src()->isConst() && \
                            src()->isA(type),   \
                            Type(),             \
                            "constant " #type); \
                      ++curSrc;
#define CStr          C(StaticStr)
#define SVar(...)     checkVariadic(buildUnion(__VA_ARGS__));
#define ND
#define DMulti
#define DSetElem
#define D(...)
#define DBuiltin
#define DSubtract(src, t)checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DofS(src)   checkDst(src < inst->numSrcs(),  \
                             "invalid src num");
#define DRefineS(src) checkDst(src < inst->numSrcs(),  \
                               "invalid src num");     \
                      requireTypeParam();
#define DParamMayRelax requireTypeParam();
#define DParam         requireTypeParam();
#define DParamPtr(k)   requireTypeParamPtr(Ptr::k);
#define DUnboxPtr
#define DBoxPtr
#define DAllocObj
#define DArrElem
#define DArrPacked
#define DThis
#define DCtx
#define DCns

#define O(opcode, dstinfo, srcinfo, flags) \
  case opcode: dstinfo srcinfo countCheck(); return true;

  switch (inst->op()) {
    IR_OPCODES
  default: always_assert(false);
  }

#undef O

#undef NA
#undef S
#undef AK
#undef C
#undef CStr
#undef SVar

#undef ND
#undef D
#undef DBuiltin
#undef DSubtract
#undef DMulti
#undef DSetElem
#undef DofS
#undef DRefineS
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DAllocObj
#undef DArrElem
#undef DArrPacked
#undef DThis
#undef DCtx
#undef DCns

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
