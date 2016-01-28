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

#include "hphp/runtime/vm/jit/loop-analysis.h"

#include <string>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/prof-data.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir_loop);

namespace {

//////////////////////////////////////////////////////////////////////

using BlockSet = jit::flat_set<Block*>;
using EdgeSet  = jit::flat_set<Edge*>;

struct Env {
  Env(const IRUnit& unit,
      const BlockList& rpoBlocks)
    : unit(unit)
    , rpoBlocks(rpoBlocks)
    , idoms(findDominators(unit, rpoBlocks, numberBlocks(unit, rpoBlocks)))
  {}

  const IRUnit&    unit;
  const BlockList& rpoBlocks;
  const IdomVector idoms;
};

//////////////////////////////////////////////////////////////////////

EdgeSet findBackEdges(const Env& env) {
  jit::vector<Edge*> vec;

  // Every back edge must be a retreating edge, which means we must've already
  // visited its target when iterating in RPO.  Keeping this seen set avoids
  // consulting the dominator tree for every edge.
  boost::dynamic_bitset<> seen(env.unit.numBlocks());

  auto consider = [&] (Edge* edge) {
    if (!edge) return;
    if (!seen[edge->to()->id()]) return;
    if (dominates(edge->to(), edge->from(), env.idoms)) {
      vec.push_back(edge);
    } else {
      FTRACE(2, "   edge B{} -> B{} was retreating but not a back edge\n",
             edge->from()->id(), edge->to()->id());
    }
  };

  for (auto& blk : env.rpoBlocks) {
    seen[blk->id()] = true;
    consider(blk->takenEdge());
    consider(blk->nextEdge());
  }

  EdgeSet ret(begin(vec), end(vec));

  FTRACE(2, "back-edges:\n{}",
    [&] () -> std::string {
      auto s = std::string{};
      for (auto& edge : ret) {
        folly::format(&s, "  B{} -> B{}\n", edge->from()->id(),
                      edge->to()->id());
      }
      return s;
    }()
  );

  return ret;
}

/*
 * Finds the unvisited blocks in the CFG by doing a reverse DFS
 * starting at `block', and adds them to `blocks'.
 */
void findBlocks(BlockSet& blocks,
                boost::dynamic_bitset<>& visited,
                Block* block) {
  if (visited.test(block->id())) return;
  visited.set(block->id());
  block->forEachPred([&] (Block* pred) {
    findBlocks(blocks, visited, pred);
  });
  blocks.insert(block);
}

/*
 * The loop associated with this header consists of all nodes reachable
 * in the reverse CFG from its back-edges's sources before the header.
 */
void findLoop(Env& env,
              Block* header,
              const EdgeSet& backEdges,
              BlockSet& blocks) {
  boost::dynamic_bitset<> visited(env.unit.numBlocks());
  visited.set(header->id());
  blocks.insert(header);
  for (auto& backEdge : backEdges) {
    findBlocks(blocks, visited, backEdge->from());
  }
}

Block* findPreHeader(Block* header, const EdgeSet& allBackEdges) {
  Edge* candidate = nullptr;
  for (auto& predEdge : header->preds()) {
    if (allBackEdges.count(&predEdge)) continue;
    if (candidate) return nullptr;
    candidate = &predEdge;
    if (candidate->from()->numSuccs() != 1) return nullptr;
  }
  return candidate ? candidate->from() : nullptr;
}

/*
 * Find and set the parent loop for each loop, if it exists.  This
 * uses the dominator tree, since the a parent loop's header must
 * dominate the headers of its children loops.
 */
void findParents(Env& env, LoopAnalysis& la) {
  for (auto& headerToLoop : la.headers) {
    auto const innerId = headerToLoop.second;
    auto const block   = la.loops[innerId].header;

    // Chase up the dominator tree and see if we find a parent loop.
    for (auto up = env.idoms[block]; up != nullptr; up = env.idoms[up]) {
      if (!la.headers.contains(up)) continue;
      auto upId = la.headers[up];
      if (la.loops[upId].blocks.count(block)) {
        la.loops[innerId].parent = upId;
        break;
      }
    }
  }
}

void findInnerLoops(LoopAnalysis& la) {
  sparse_id_set<LoopID> parents(la.loops.size());
  for (auto& loop : la.loops) {
    if (loop.parent != kInvalidLoopID) {
      parents.insert(loop.parent);
    }
  }

  la.innerLoops.reserve(la.loops.size() - parents.size());
  for (auto& loop : la.loops) {
    if (!parents.contains(loop.id)) {
      la.innerLoops.push_back(loop.id);
    }
  }
}

/*
 * Return the `loop's preExit block if it was already created or if a
 * suitable block already exists.  Otherwise, return nullptr.
 */
Block* findLoopPreExit(LoopInfo& loop) {
  if (loop.preExit) return loop.preExit;
  const auto preHeader = loop.preHeader;
  if (!preHeader) return nullptr;
  if (preHeader->preds().size() != 1) return nullptr;
  const auto& pred = preHeader->preds().front().from();
  if (pred->back().op() == ExitPlaceholder) loop.preExit = pred->taken();
  return loop.preExit;
}

/*
 * Clone the `unit's CFG starting at `startBlock', and rename SSATmps
 * according to `tmpRenames' map along the way.  The new block
 * corresponding to `startBlock' is returned.  All blocks reachable
 * from `startBlock' are also cloned, so that there is no path from
 * the cloned blocks to the original blocks in the `unit'.  Note that,
 * as instructions are cloned into the new blocks, the dest SSATmps of
 * these instructions also need to be renamed, so they're added to
 * `tmpRenames' along the way.
 */
Block* cloneCFG(IRUnit& unit,
                Block* startBlock,
                jit::flat_map<SSATmp*, SSATmp*> tmpRenames) {
  jit::queue<Block*> toClone;
  boost::dynamic_bitset<> toCloneSet(unit.numBlocks());
  jit::hash_map<Block*, Block*> blockRenames;

  FTRACE(5, "cloneCFG: starting at B{}\n", startBlock->id());

  auto push = [&](Block* b) {
    if (!b || toCloneSet[b->id()]) return;
    toClone.push(b);
    toCloneSet[b->id()] = true;
  };

  // Clone each of the blocks.
  push(startBlock);
  while (!toClone.empty()) {
    auto origBlock = toClone.front();
    toClone.pop();
    auto copyBlock = unit.defBlock(origBlock->profCount(), origBlock->hint());
    blockRenames[origBlock] = copyBlock;
    FTRACE(5, "cloneCFG: copying B{} to B{}\n",
           origBlock->id(), copyBlock->id());

    // Clone each of the instructions in the block.
    for (auto& origInst : *origBlock) {
      auto copyInst = unit.clone(&origInst);

      // Remember the new SSATmps (the dests) which will need to be renamed.
      for (size_t d = 0; d < origInst.numDsts(); d++) {
        tmpRenames[origInst.dst(d)] = copyInst->dst(d);
      }

      // Rename all the source SSATmps that need renaming.
      for (size_t s = 0; s < origInst.numSrcs(); s++) {
        auto it = tmpRenames.find(copyInst->src(s));
        if (it != tmpRenames.end()) {
          auto newSrc = it->second;
          copyInst->setSrc(s, newSrc);
        }
      }
      copyBlock->push_back(copyInst);
    }

    push(origBlock->next());
    push(origBlock->taken());
  }

  // Now go through all new blocks and reset their next/taken blocks
  // to their corresponding new blocks.
  for (auto& blockRename : blockRenames) {
    auto newBlock = blockRename.second;
    auto& lastInst = newBlock->back();
    if (lastInst.next())  lastInst.setNext (blockRenames[lastInst.next()]);
    if (lastInst.taken()) lastInst.setTaken(blockRenames[lastInst.taken()]);
  }

  return blockRenames[startBlock];
}

//////////////////////////////////////////////////////////////////////

/*
 * Search the HHIR control-flow graph backwards starting at `b' for
 * the first blocks not in `visited' that belong to a Profile
 * translation other than `headerTID', and add those blocks' TransIDs
 * to `set'.
 */
void findPredTransIDs(TransID headerTID, Block* b,
                      boost::dynamic_bitset<>& visited,
                      TransIDSet& set) {
  if (visited[b->id()]) return;
  visited[b->id()] = true;
  auto bTID = b->front().marker().profTransID();
  if (set.count(bTID)) return;
  if (bTID != headerTID) {
    if (bTID != kInvalidTransID) set.insert(bTID);
    else assertx(b->id() == 0); // only the entry block may have no ProfTransID
    return;
  }
  // Keep searching.
  for (auto& predEdge : b->preds()) {
    findPredTransIDs(headerTID, predEdge.from(), visited, set);
  }
}

/*
 * Computes the approximate number of times that `loop' was invoked
 * (i.e. entered) using profiling data.  This value is computed by
 * adding up the profiling weights of all the Profile translations
 * that may execute immediately before the Profile translation
 * containing the loop header.
 */
uint64_t countInvocations(const LoopInfo& loop, const IRUnit& unit) {
  always_assert(mcg->tx().profData());

  // Find the predecessor TransIDs along each non-back-edge
  // predecessor of the loop header.
  boost::dynamic_bitset<> visited(unit.numBlocks());
  TransIDSet predTIDs;
  auto headerTID = loop.header->front().marker().profTransID();
  for (auto& predEdge : loop.header->preds()) {
    if (loop.backEdges.count(&predEdge) == 0) {
      findPredTransIDs(headerTID, predEdge.from(), visited, predTIDs);
    }
  }
  auto const profData = mcg->tx().profData();
  uint64_t count = 0;
  for (auto tid : predTIDs) {
    count += profData->transCounter(tid);
  }
  return count;
}

//////////////////////////////////////////////////////////////////////

bool DEBUG_ONLY checkInvariants(const LoopAnalysis& la) {
  always_assert(la.backEdges.size() >= la.loops.size());

  size_t totalBackEdges = 0;

  for (auto& loop : la.loops) {
    totalBackEdges += loop.backEdges.size();

    // Any loop that contains another loop's header block also contains its
    // pre-header block.
    for (auto oid = loop.id + 1; oid < la.loops.size(); ++oid) {
      auto& other = la.loops[oid];
      if (other.preHeader && loop.blocks.count(other.header)) {
        always_assert(loop.blocks.count(other.preHeader));
      }
    }

    // A parent loop must contain all the blocks in its children loops.
    if (loop.parent != kInvalidLoopID) {
      auto const& parentBlocks = la.loops[loop.parent].blocks;
      for (auto block : loop.blocks) {
        always_assert(parentBlocks.count(block));
      }
    }
  }

  always_assert(la.backEdges.size() == totalBackEdges);

  return true;
}

//////////////////////////////////////////////////////////////////////

}

LoopAnalysis identifyLoops(const IRUnit& unit, const BlockList& rpoBlocks) {
  FTRACE(1, "identifyLoops:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "identifyLoops:^^^^^^^^^^^^^^^^^^^^\n"); };

  LoopAnalysis la{unit.numBlocks()};
  Env env{unit, rpoBlocks};

  la.backEdges = findBackEdges(env);

  unsigned nextId = 0;
  for (auto& edge : la.backEdges) {
    auto header = edge->to();
    if (la.headers.contains(header)) {
      auto loopId = la.headers[header];
      la.loops[loopId].backEdges.insert(edge);
    } else { // new loop
      la.headers[header] = nextId;
      LoopInfo loopInfo;
      loopInfo.id = nextId;
      loopInfo.header = header;
      la.loops.push_back(loopInfo);
      la.loops.back().backEdges.insert(edge);
      nextId++;
    }
  }

  for (auto& h : la.headers) {
    auto loopId = h.second;
    auto&  loop = la.loops[loopId];
    auto header = loop.header;
    findLoop(env, header, loop.backEdges, loop.blocks);
    loop.preHeader = findPreHeader(header, la.backEdges);
    loop.numInvocations = countInvocations(loop, unit);
  }

  findParents(env, la);
  findInnerLoops(la);

  assertx(checkInvariants(la));

  return la;
}

//////////////////////////////////////////////////////////////////////

/*
 * If loop `loopId' already contains a `preExit' block, then return
 * it.  Otherwise, create one and return it.
 *
 * Creating a `preExit' requires both:
 *   1. the loop to have a preHeader; and
 *   2. the loop header to contain an ExitPlaceholder, optionally
 *      preceded by a DefLabel.
 *
 * This function then transforms a CFG from:
 *
 *             |
 *             v
 * +--------------------------+
 * | ...                      |  B1: oldPreHeader
 * | Jmp ta, tb, ...          |
 * +--------------------------+
 *             |
 *             v
 * +--------------------------+
 * | tx, ty, ... = DefLabel   |  B2: header
 * | ExitPlaceholder origExit |
 * +--------------------------+ --> B3: origExit (note: may use tx, ty)
 *             |
 *             v
 *
 *
 * into the following CFG:
 *
 *             |
 *             v
 * +--------------------------+
 * | ...                      |  B1: oldPreHeader
 * | ExitPlaceholder preExit  |
 * +--------------------------+ --> B4: preExit
 *             |                 (cloned from origExit, renaming tx,ty => ta,tb)
 *             v
 * +--------------------------+
 * | Jmp ta, tb, ...          |  B5: newPreHeader
 * +--------------------------+
 *             |
 *             v
 * +--------------------------+
 * | tx, ty, ... = DefLabel   |  B2: header
 * | ExitPlaceholder origExit |
 * +--------------------------+ --> B3: origExit
 *             |
 *             v
 *
 */
Block* insertLoopPreExit(IRUnit& unit,
                         LoopAnalysis& la,
                         LoopID loopId) {
  auto& loop = la.loops[loopId];
  const auto oldPreHeader = loop.preHeader;
  assertx(oldPreHeader);

  // Check if we already have a pre-exit block, return it.
  if (auto preExit = findLoopPreExit(loop)) return preExit;

  const auto header = loop.header;

  auto const eph = header->skipHeader();
  assertx(eph->is(ExitPlaceholder));

  auto origExit = eph->taken();

  jit::flat_map<SSATmp*, SSATmp*> tmpRenames;
  if (header->front().is(DefLabel)) {
    auto& defLabel = header->front();
    auto& jmpLabel = oldPreHeader->back();
    assertx(jmpLabel.is(Jmp));
    assertx(jmpLabel.numSrcs() == defLabel.numDsts());
    for (size_t i = 0; i < jmpLabel.numSrcs(); i++) {
      tmpRenames[defLabel.dst(i)] = jmpLabel.src(i);
    }
  }

  Block* preExit = cloneCFG(unit, origExit, tmpRenames);
  loop.preExit = preExit;

  // Split oldPreHeader before the Jmp, and append an
  // ExitPlaceholder{ fallthru=newPreHeader, taken=preExit }.
  auto newPreHeader = unit.defBlock(oldPreHeader->profCount(),
                                    oldPreHeader->hint());
  assertx(oldPreHeader->back().is(Jmp));
  auto const jmp = &(oldPreHeader->back());
  oldPreHeader->erase(jmp);
  newPreHeader->prepend(jmp);
  auto exitPlaceholder = unit.gen(ExitPlaceholder, jmp->marker(), preExit);
  oldPreHeader->insert(oldPreHeader->end(), exitPlaceholder);
  exitPlaceholder->setNext(newPreHeader);

  updatePreHeader(la, loopId, newPreHeader); // also checks invariants

  return preExit;
}

//////////////////////////////////////////////////////////////////////

void insertLoopPreHeader(IRUnit& unit,
                         LoopAnalysis& la,
                         LoopID loopId) {
  auto& loop = la.loops[loopId];

  assertx(checkInvariants(la));
  always_assert(loop.preHeader == nullptr);

  auto const header = loop.header;

  // Find all non-back-edge predecessors.  They'll be rechained to go to the
  // new pre-header.
  jit::vector<Block*> preds;
  for (auto& predEdge : header->preds()) {
    if (la.backEdges.count(&predEdge) == 0) {
      preds.push_back(predEdge.from());
    }
  }
  always_assert(!preds.empty());

  // If we already have a block that qualifies as a pre-header, but pre-header
  // was nullptr (asserted above), that means the block was added after this
  // LoopAnalysis was created.  So someone has invalidated the LoopAnalysis
  // structure by changing the CFG before calling this function, which isn't
  // ok.
  if (preds.size() == 1) {
    always_assert_flog(
      preds[0]->numSuccs() != 1,
      "insertLoopPreHeader on L{} (in B{}), which already had a pre-header",
      loopId,
      header->id()
    );
  }

  auto const preHeader = unit.defBlock(loop.numInvocations);
  auto const marker = header->front().marker();

  // If the header starts with a DefLabel, the arguments from all the incoming
  // edges need to be collected in a new DefLabel in the new pre-header, and
  // Jmp'd into the header.
  jit::vector<SSATmp*> args;
  if (header->front().is(DefLabel)) {
    auto const num_args = header->front().numDsts();
    auto const label = unit.defLabel(num_args, marker);
    preHeader->insert(preHeader->begin(), label);
    if (num_args > 0) {
      args.reserve(num_args);
      for (auto& dst : label->dsts()) args.push_back(dst);
    }
  }

  if (args.empty()) {
    preHeader->prepend(unit.gen(Jmp, marker, header));
  } else {
    preHeader->prepend(unit.gen(Jmp, marker, header,
                                std::make_pair(args.size(), &args[0])));
  }

  // Rechain the predecessors to go to the new preHeader.
  for (auto& pred : preds) {
    auto& branch = pred->back();
    if (branch.taken() == header) {
      branch.setTaken(preHeader);
    } else {
      assertx(branch.next() == header);
      branch.setNext(preHeader);
    }
  }

  /*
   * Retype destinations of the label if we had args.  This must happen after
   * rechaining the predecessors, because that's where the types come from.  We
   * don't need to retype the DefLabel in the actual loop header, because it
   * must already contain the types for these incoming values.
   */
  if (!args.empty()) retypeDests(&preHeader->front(), &unit);

  // Will assert invariants on the way out:
  updatePreHeader(la, loopId, preHeader);
}

/*
 * Set the pre-header of loop `loopId' to `preHeader', and also add
 * `preHeader' to any ancestor of `loopId'.
 */
void updatePreHeader(LoopAnalysis& la, LoopID loopId, Block* preHeader) {
  auto& loop = la.loops[loopId];
  assertx(la.headers[loop.header] == loopId);

  loop.preHeader = preHeader;

  for (auto ancestorId = loop.parent; ancestorId != kInvalidLoopID;
       ancestorId = la.loops[ancestorId].parent) {
    la.loops[ancestorId].blocks.insert(preHeader);
  }

  assertx(checkInvariants(la));
}

//////////////////////////////////////////////////////////////////////

std::string show(const LoopInfo& linfo) {
  auto ret = std::string{};
  folly::format(&ret, "Loop {}, header: B{}", linfo.id, linfo.header->id());
  if (linfo.preHeader != nullptr) {
    folly::format(&ret, "  (pre-header: B{})", linfo.preHeader->id());
  }
  if (linfo.preExit != nullptr) {
    folly::format(&ret, "  (pre-exit: B{})", linfo.preExit->id());
  }
  ret += "\n";
  folly::format(&ret, "        numInvocations: {}\n", linfo.numInvocations);
  for (auto& b : linfo.blocks) {
    folly::format(&ret, "                B{}\n", b->id());
  }

  return ret;
}

std::string show(const LoopAnalysis& la) {
  auto ret = std::string{};
  for (auto& x : la.loops) ret += show(x);

  folly::format(&ret, "digraph G {{\n");
  for (auto& x : la.loops) {
    folly::format(&ret, "L{};\n", x.id);

    if (x.parent != kInvalidLoopID) {
      folly::format(&ret, "L{} -> L{};\n", x.id, x.parent);
    }
  }
  folly::format(&ret, "}}\n");

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
