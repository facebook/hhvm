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

/*
 * Type check hoisting moves type checks higher up in the CFG, allowing the
 * check's type information to be used by more instructions.
 *
 * The way it works is as follows:
 * - Iterate through CFG to identify variables with at least one typecheck. If
 *   there exist more than one typecheck of the same variable, they must be
 *   compatible with each other (i.e. their intersection must not be Bottom).
 *   If the checks are incompatible we then give up on hoisting the checks for
 *   this particular variable.
 * - Iterate over each typecheck that we decided would make sense to hoist.
 *   Combine checks that want to be hoisted to the same destination into hoist
 *   groups. This step allows us to hoist dependent chains of checks as if they
 *   were a single check.
 * - For each hoist group, perform the actual hoisting to the highest
 *   instruction with an exit that still dominates the rest of the hoist group.
 */

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include <unordered_map>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_checkhoist);

//////////////////////////////////////////////////////////////////////

namespace {

template <typename T>
struct UnionFind {
  T* find(T*);
  T* find(T*) const;
  void join(T*, T*);

private:
  struct Node {
    T* tmp;
    Node* parent;
  };

  Node* findImpl(T*);
  const Node* findImpl(T*) const;
  Node* traverseToRoot(Node& start);

  std::unordered_map<T*, Node> m_forest;
};

template <typename T>
T* UnionFind<T>::find(T* tmp) {
  return findImpl(tmp)->tmp;
}

template <typename T>
T* UnionFind<T>::find(T* tmp) const {
  return findImpl(tmp)->tmp;
}

template <typename T>
typename UnionFind<T>::Node* UnionFind<T>::traverseToRoot(
  UnionFind<T>::Node& start
) {
  auto current = &start;
  while (current->parent) {
    current = current->parent;
  }
  if (current != &start) {
    start.parent = current;
  }
  return current;
}

template <typename T>
const typename UnionFind<T>::Node* UnionFind<T>::findImpl(T* tmp) const {
  assert(m_forest.count(tmp));
  return const_cast<UnionFind<T>*>(this)->findImpl(tmp);
}

template <typename T>
typename UnionFind<T>::Node* UnionFind<T>::findImpl(T* tmp) {
  auto result = m_forest.emplace(tmp, Node { tmp, nullptr });
  auto& node = result.first->second;
  if (result.second) return &node;
  return traverseToRoot(node);
}

template <typename T>
void UnionFind<T>::join(T* tmpA, T* tmpB) {
  auto rootA = findImpl(tmpA);
  auto rootB = findImpl(tmpB);
  if (rootA == rootB) return;
  rootB->parent = rootA;
}

struct TypeCheckData {
  bool canHoist;
  IRInstruction* checkType;
};

using CanonicalCheck = IRInstruction;
using SSATmpTypeMap = std::unordered_map<SSATmp*, TypeCheckData>;
using BottomUpHoistGroups = UnionFind<IRInstruction>;
using TopDownHoistGroups = std::unordered_map<CanonicalCheck*,
                                              std::vector<IRInstruction*>>;
using HoistDestinations = std::unordered_map<CanonicalCheck*, IRInstruction*>;
using HoistGroupTypes = std::unordered_map<CanonicalCheck*, Type>;
using HoistGroupDestinations = std::unordered_map<CanonicalCheck*,
                                                  IRInstruction*>;

// 2-DO: check splitting at phis

bool isHoistableExit(Block* block) {
  if (!block->back().is(ReqBindJmp, ReqRetranslate)) return false;
  assert(block->isExit());
  assert(!block->isCatch());
  return true;
}

void findHoistableCheck(
  IRInstruction& inst,
  SSATmpTypeMap& ssaTmpTypes
) {
  switch (inst.op()) {
  case CheckType: {
    if (!isHoistableExit(inst.taken())) {
      FTRACE(2, "CheckType {}'s taken branch is not an exit\n", inst.id());
      break;
    }

    auto src = inst.src(0);
    auto type = inst.typeParam();
    if (src->isA(type)) {
      // This CheckType should've been optimized away but wasn't, presumably
      // because certain optimizations are off or haven't run yet. Ignore it.
      break;
    }

    // Try to put a new TypeCheckData into the map. If this is the first time
    // we've seen a check on this particular SSATmp, the insert will succeed.
    // Otherwise we'll get the previous TypeCheckData back so we can modify it.
    auto result = ssaTmpTypes.emplace(src, TypeCheckData { true, &inst });
    if (result.second) {
      FTRACE(2, "Adding TypeCheckData for {}\n", inst);
      break;
    } else {
      FTRACE(2, "TypeCheckData already exists for {}\n", inst);
    }

    auto& checkData = result.first->second;

    // If we previously decided not to try to hoist checks for this SSATmp then
    // skip this check too.
    if (!checkData.canHoist) {
      FTRACE(2, "Previously decided we can't hoist {}", inst.id());
      break;
    }

    // If we see two checks on the same SSATmp with non-intersecting types then
    // give up on hoisting this particular SSATmp.
    if (!checkData.checkType->typeParam().maybe(type)) {
      FTRACE(2, "Found contradictory CheckType for SSATmp {}, disabling "
             "hoisting...\n", src->id());
      checkData = TypeCheckData { false, nullptr };
      break;
    }

    // The type we're checking here is a subtype of a previously identified
    // check, so update the TypeCheckData with the stronger check since
    // hoisting it will allow us to eliminate the subsequent weaker check.
    if (type < checkData.checkType->typeParam()) {
      FTRACE(2, "Updating TypeCheckData for SSATmp {}, {} -> {}\n",
             src->id(), type, checkData.checkType->typeParam());
      checkData = TypeCheckData { true, &inst };
    }
    break;
  }
  default:
    break;
  }
}

SSATmpTypeMap findHoistableChecks(const BlockList& blocks) {
  SSATmpTypeMap ssaTmpTypes;
  for (auto block : blocks) {
    for (auto& inst : *block) {
      findHoistableCheck(inst, ssaTmpTypes);
    }
  }
  return ssaTmpTypes;
}

BottomUpHoistGroups computeHoistGroups(const SSATmpTypeMap& ssaTmpTypes) {
  // A hoist group is a group of IRInstructions that all want to be hoisted to
  // the same destination IRInstruction. By construction, the final canonical
  // IRInstruction for a hoist group is the first check in that group. A hoist
  // group covers situations with a chain of dependent checks or of multiple
  // checks that consume the same SSATmp.
  BottomUpHoistGroups hoistGroups;
  for (auto const& kv : ssaTmpTypes) {
    auto const& checkData = kv.second;
    if (!checkData.canHoist) continue;

    auto checkType = checkData.checkType;
    auto defInst = checkType->src(0)->inst();
    if (defInst->op() != CheckType) {
      // Join the CheckType to itself in case this is the first time we've
      // seen it (i.e. it's not in the hoist group forest yet).
      hoistGroups.find(checkType);
      continue;
    }
    FTRACE(2, "Joining hoist groups for instruction {} and CheckType {}\n",
           defInst->id(), checkType->id());
    hoistGroups.join(defInst, checkType);
  }
  return hoistGroups;
}

TopDownHoistGroups invertHoistGroups(
  const SSATmpTypeMap& ssaTmpTypes,
  const BottomUpHoistGroups& hoistGroups
) {
  // Invert the hoist group data structure to map from canonical check to a
  // vector of checks.
  TopDownHoistGroups invertedHoistGroups;
  for (auto& kv : ssaTmpTypes) {
    auto const& checkData = kv.second;
    if (!checkData.canHoist) continue;

    auto checkType = checkData.checkType;
    auto canonicalCheck = hoistGroups.find(checkType);
    invertedHoistGroups[canonicalCheck].push_back(checkType);
  }
  return invertedHoistGroups;
}

HoistGroupTypes computeHoistGroupTypes(
  const TopDownHoistGroups& hoistGroups
) {
  // Create a map from canonical check to aggregated type for all hoist groups.
  HoistGroupTypes hoistGroupTypes;
  for (auto& kv : hoistGroups) {
    auto const& checksInGroup = kv.second;
    auto groupType = TTop;
    for (auto const check : checksInGroup) {
      groupType &= check->typeParam();
    }
    assert(groupType != TBottom);
    hoistGroupTypes.emplace(kv.first, groupType);
  }
  return hoistGroupTypes;
}

/*
 * Returns the first instruction, starting from "startInst", with an
 * accompanying exit block that dominates "dominatedInst". If no such
 * instruction exists, returns null.
 */
IRInstruction* findNextDominatingExit(
  const IdomVector& idoms,
  IRInstruction* startInst,
  IRInstruction* dominatedInst
) {
  assert(startInst->op() != DefConst);
  auto const startBlock = startInst->block();
  auto const dominatedBlock = dominatedInst->block();

  auto hasValidExit = [&](IRInstruction* inst) {
    if (!inst->is(ExitPlaceholder)) return false;
    assert(isHoistableExit(inst->taken()));
    return true;
  };

  boost::dynamic_bitset<> seen(idoms.unit().numBlocks());
  std::queue<Block*> queue;
  queue.push(startBlock);
  while (!queue.empty()) {
    auto currentBlock = queue.front();
    queue.pop();

    if (seen.test(currentBlock->id())) continue;
    seen.set(currentBlock->id());

    const bool canUseExitInCurrentBlock = [&] {
      if (!dominates(currentBlock, dominatedBlock, idoms)) return false;

      if (currentBlock != startBlock &&
          dominates(currentBlock, startBlock, idoms)) {
        return false;
      }
      return true;
    }();

    if (canUseExitInCurrentBlock) {
      auto iter = (currentBlock == startBlock) ?
        currentBlock->iteratorTo(startInst) : currentBlock->begin();
      for (; iter != currentBlock->end(); ++iter) {
        auto& currentInst = *iter;
        if (hasValidExit(&currentInst)) return &currentInst;
      }
    }

    auto& lastInst = currentBlock->back();
    if (lastInst.next()) {
      queue.push(lastInst.next());
    }
    if (lastInst.taken()) {
      queue.push(lastInst.taken());
    }
  }

  return nullptr;
}

HoistGroupDestinations findHoistGroupDestinations(
  const IdomVector& idoms,
  const TopDownHoistGroups& hoistGroups
) {
  auto canHoistTo = [&](IRInstruction* inst) {
    return inst->op() != DefConst;
  };
  // Create a map from canonical check to hoist destination.
  std::unordered_map<IRInstruction*, IRInstruction*> hoistDestinations;
  for (auto& kv : hoistGroups) {
    auto canonicalCheck = kv.first;
    auto defInst = canonicalCheck->src(0)->inst();
    FTRACE(2, "Looking for hoist dest of canonical `{}'\n", *canonicalCheck);
    assert(defInst->op() != CheckType);
    auto hoistDest = canHoistTo(defInst)
      ? findNextDominatingExit(idoms, defInst, canonicalCheck)
      : canonicalCheck;
    if (!hoistDest) hoistDest = canonicalCheck;
    hoistDestinations.emplace(canonicalCheck, hoistDest);
  }
  return hoistDestinations;
}

void performHoisting(
  IRUnit& unit,
  const HoistGroupTypes& types,
  const HoistGroupDestinations& destinations
) {
  auto replaceCheckType = [&](IRInstruction* exitingInst, Type type) {
    // We can just change the Type param of the original check rather than
    // inserting a bunch of control flow.
    assert(exitingInst->op() == CheckType);
    auto originalCheck = exitingInst;
    assert(type <= originalCheck->typeParam());
    FTRACE(2, "Replacing type param {} with {} for {}\n",
      exitingInst->typeParam(),
      type,
      *exitingInst);
    originalCheck->setTypeParam(type);
  };

  auto createCheckTypeAfter = [&](
    IRInstruction* exitingInst,
    IRInstruction* oldCheck,
    Type type
  ) {
    auto newBlock = unit.defBlock();
    auto newCheck = unit.gen(
      oldCheck->op(),
      exitingInst->marker(),
      type,
      exitingInst->taken(),
      oldCheck->src(0)
    );
    newCheck->setNext(exitingInst->next());
    newBlock->push_back(newCheck);
    assert(newCheck->dst()->type() == (newCheck->src(0)->type() & type));

    // Change the control flow of the exiting instruction to go through the
    // new block.
    exitingInst->setNext(newBlock);

    FTRACE(2, "Inserted {} to hoist {} at exiting inst {}\n",
      *newCheck,
      *oldCheck,
      *exitingInst);
  };

  for (auto const& kv : destinations) {
    auto const canonicalCheck = kv.first;
    auto const dest = kv.second;

    auto result = types.find(canonicalCheck);
    assert(result != types.end());
    auto const newType = result->second;
    if (canonicalCheck == dest) {
      replaceCheckType(dest, newType);
    } else {
      createCheckTypeAfter(dest, canonicalCheck, newType);
    }
  }
}

void insertHaltBlocks(
  IRUnit& unit,
  const TopDownHoistGroups& hoistGroups,
  const HoistGroupDestinations& groupDests
) {
  auto addHaltBlock = [&](IRInstruction* check) {
    assertx(check->is(CheckType));
    auto haltBlock = unit.defBlock();
    auto halt = unit.gen(Halt, check->marker());
    haltBlock->push_back(halt);
    check->setTaken(haltBlock);
  };
  for (auto const& kv : hoistGroups) {
    auto canonicalCheck = kv.first;
    auto const dest = groupDests.at(canonicalCheck);
    auto& memberChecks = kv.second;
    for (auto check : memberChecks) {
      // If check and dest were the same, we would have replaced the check
      // instead of creating a new one, so we only want to add a Halt block
      // if that's not the case.
      if (check == dest) continue;
      addHaltBlock(check);
    }
  }
}

void forwardHoistedSrcsToDsts(IRUnit& unit) {
  // We have to recompute the CFG and dominators because we changed things
  // when we hoisted.
  auto const rpoBlocks = rpoSortCfg(unit);
  auto const dominators = findDominators(
    unit,
    rpoBlocks,
    numberBlocks(unit, rpoBlocks)
  );

  refineTmps(unit, rpoBlocks, dominators);
}

} // namespace

void hoistTypeChecks(IRUnit& unit) {
  PassTracer tracer{&unit, Trace::hhir_checkhoist, "Hoist typechecks"};

  auto const rpoBlocks = rpoSortCfg(unit);
  auto const dominators = findDominators(
    unit,
    rpoBlocks,
    numberBlocks(unit, rpoBlocks)
  );

  // Identify individual checks that appear hoistable.
  auto ssaTmpTypes = findHoistableChecks(rpoBlocks);

  // Group these individual checks into groups rooted at a canonical
  // non-hoistable IRInstruction.
  auto const bottomUpHoistGroups = computeHoistGroups(ssaTmpTypes);
  auto const topDownHoistGroups = invertHoistGroups(ssaTmpTypes,
                                                    bottomUpHoistGroups);
  auto const groupTypes = computeHoistGroupTypes(topDownHoistGroups);
  auto const groupDests = findHoistGroupDestinations(dominators,
                                                     topDownHoistGroups);

  // Do the actual hoisting.
  performHoisting(unit, groupTypes, groupDests);
  forwardHoistedSrcsToDsts(unit);
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    insertHaltBlocks(unit, topDownHoistGroups, groupDests);
  }
}

} } // namespace HPHP::jit
