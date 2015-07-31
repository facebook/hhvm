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

#include "hphp/runtime/vm/jit/loop-analysis.h"

#include <string>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mutation.h"

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

  const IRUnit& unit;
  const BlockList& rpoBlocks;
  const IdomVector idoms;
};

//////////////////////////////////////////////////////////////////////

EdgeSet find_back_edges(const Env& env) {
  jit::vector<Edge*> vec;

  // Every back edge must be a retreating edge, which means we must've already
  // visited its target when iterating in RPO.  Keeping this seen set avoids
  // consulting the dominator tree for every edge.
  auto seen = boost::dynamic_bitset<>(env.unit.numBlocks());

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

  FTRACE(2, "back_edges:\n{}",
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
void find_blocks(BlockSet& blocks,
                 boost::dynamic_bitset<>& visited,
                 Block* blk) {
  if (visited.test(blk->id())) return;
  visited.set(blk->id());
  blk->forEachPred([&] (Block* pred) {
    find_blocks(blocks, visited, pred);
  });
  blocks.insert(blk);
}

/*
 * The loop associated with this header consists of all nodes reachable
 * in the reverse CFG from its back-edges' sources before the header.
 */
void find_loop(Env& env,
               Block* header,
               const EdgeSet& back_edges,
               BlockSet& blocks) {
  boost::dynamic_bitset<> visited(env.unit.numBlocks());
  visited.set(header->id());
  blocks.insert(header);
  for (auto& backEdge : back_edges) {
    find_blocks(blocks, visited, backEdge->from());
  }
}

Block* find_pre_header(Block* header, const EdgeSet& all_back_edges) {
  Edge* candidate = nullptr;
  for (auto& pred_edge : header->preds()) {
    if (all_back_edges.count(&pred_edge)) continue;
    if (candidate) return nullptr;
    candidate = &pred_edge;
    if (candidate->from()->numSuccs() != 1) return nullptr;
  }
  return candidate ? candidate->from() : nullptr;
}

/*
 * Find and set the parent loop for each loop, if it exists.  This
 * uses the dominator tree, since the a parent loop's header must
 * dominate the headers of its children loops.
 */
void find_parents(Env& env, LoopAnalysis& loops) {
  for (auto& header_to_loop_kv : loops.headers) {
    auto const inner_id = header_to_loop_kv.second;
    auto const block = loops.loops[inner_id].header;

    // Chase up the dominator tree and see if we find a parent loop.
    for (auto up = env.idoms[block]; up != nullptr; up = env.idoms[up]) {
      if (!loops.headers.contains(up)) continue;
      auto upId = loops.headers[up];
      if (loops.loops[upId].members.count(block)) {
        loops.loops[inner_id].parent = upId;
        break;
      }
    }
  }
}

void find_inner_loops(LoopAnalysis& loops) {
  auto parents = sparse_id_set<LoopID>(loops.loops.size());
  for (auto& loop : loops.loops) {
    if (loop.parent != kInvalidLoopID) {
      parents.insert(loop.parent);
    }
  }

  loops.inner_loops.reserve(loops.loops.size() - parents.size());
  for (auto& loop : loops.loops) {
    if (!parents.contains(loop.id)) loops.inner_loops.push_back(loop.id);
  }
}

//////////////////////////////////////////////////////////////////////

bool DEBUG_ONLY check_invariants(const LoopAnalysis& loops) {
  always_assert(loops.back_edges.size() >= loops.loops.size());

  size_t total_back_edges = 0;

  for (auto& loop : loops.loops) {
    total_back_edges += loop.back_edges.size();

    // Any loop that contains another loop's header block also contains its
    // pre-header block.
    for (auto oid = loop.id + 1; oid < loops.loops.size(); ++oid) {
      auto& other = loops.loops[oid];
      if (other.pre_header && loop.members.count(other.header)) {
        always_assert(loop.members.count(other.pre_header));
      }
    }

    // A parent loop must contain all the blocks in its children loops.
    if (loop.parent != kInvalidLoopID) {
      auto const& parentBlocks = loops.loops[loop.parent].members;
      for (auto block : loop.members) {
        always_assert(parentBlocks.count(block));
      }
    }
  }

  always_assert(loops.back_edges.size() == total_back_edges);

  return true;
}

//////////////////////////////////////////////////////////////////////

}

LoopAnalysis identify_loops(const IRUnit& unit, const BlockList& rpoBlocks) {
  FTRACE(1, "identify_loops:vvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(1, "identify_loops:^^^^^^^^^^^^^^^^^^^^\n"); };

  auto ret = LoopAnalysis{unit.numBlocks()};
  auto env = Env { unit, rpoBlocks };

  ret.back_edges = find_back_edges(env);

  unsigned nextId = 0;
  for (auto& edge : ret.back_edges) {
    auto header = edge->to();
    if (ret.headers.contains(header)) {
      auto loop_id = ret.headers[header];
      ret.loops[loop_id].back_edges.insert(edge);
    } else { // new loop
      ret.headers[header] = nextId;
      LoopInfo loopInfo;
      loopInfo.id = nextId;
      loopInfo.header = header;
      ret.loops.push_back(loopInfo);
      ret.loops.back().back_edges.insert(edge);
      nextId++;
    }
  }

  for (auto& h : ret.headers) {
    auto loop_id = h.second;
    auto&  loop = ret.loops[loop_id];
    auto header = loop.header;
    find_loop(env, header, loop.back_edges, loop.members);
    loop.pre_header = find_pre_header(header, ret.back_edges);
  }

  find_parents(env, ret);
  find_inner_loops(ret);

  assertx(check_invariants(ret));

  return ret;
}

//////////////////////////////////////////////////////////////////////

void insert_loop_pre_header(IRUnit& unit,
                            LoopAnalysis& loops,
                            LoopID loop_id) {
  auto& loop = loops.loops[loop_id];

  assertx(check_invariants(loops));
  always_assert(loop.pre_header == nullptr);

  auto const header = loop.header;

  // Find all non-back-edge predecessors.  They'll be rechained to go to the
  // new pre-header.
  auto preds = jit::vector<Block*>{};
  for (auto& pred_edge : header->preds()) {
    if (loops.back_edges.count(&pred_edge)) continue;
    preds.push_back(pred_edge.from());
  }
  always_assert(!preds.empty());

  // If we already have a block that qualifies as a pre_header, but pre_header
  // was nullptr (asserted above), that means the block was added after this
  // LoopAnalysis was created.  So someone has invalidated the LoopAnalysis
  // structure by changing the CFG before calling this function, which isn't
  // ok.
  if (preds.size() == 1) {
    always_assert_flog(
      preds[0]->numSuccs() != 1,
      "insert_loop_pre_header on L{} (in B{}), which already had a pre_header",
      loop_id,
      header->id()
    );
  }

  auto const pre_header = unit.defBlock();
  auto const marker = header->front().marker();

  // If the header starts with a DefLabel, the arguments from all the incoming
  // edges need to be collected in a new DefLabel in the new pre_header, and
  // Jmp'd into the header.
  auto args = [&] () -> jit::vector<SSATmp*> {
    auto ret = jit::vector<SSATmp*>{};
    if (!header->front().is(DefLabel)) return ret;
    auto const num_args = header->front().numDsts();
    auto const label = unit.defLabel(num_args, marker);
    pre_header->insert(pre_header->begin(), label);
    if (!num_args) return ret;
    ret.reserve(num_args);
    for (auto& dst : label->dsts()) ret.push_back(dst);
    return ret;
  }();

  if (args.empty()) {
    pre_header->prepend(unit.gen(Jmp, marker, header));
  } else {
    pre_header->prepend(unit.gen(Jmp, marker, header,
      std::make_pair(args.size(), &args[0])));
  }

  // Rechain the predecessors to go to the new pre_header.
  for (auto& pred : preds) {
    auto& branch = pred->back();
    if (branch.taken() == header) {
      branch.setTaken(pre_header);
    } else {
      assertx(branch.next() == header);
      branch.setNext(pre_header);
    }
  }

  /*
   * Retype destinations of the label if we had args.  This must happen after
   * rechaining the predecessors, because that's where the types come from.  We
   * don't need to retype the DefLabel in the actual loop header, because it
   * must already contain the types for these incoming values.
   */
  if (!args.empty()) retypeDests(&pre_header->front(), &unit);

  // Will assert invariants on the way out:
  update_pre_header(loops, loop_id, pre_header);
}

/*
 * Set the pre-header of loop `loop_id' to `pre_header', and also add
 * `pre_header' to any ancestor of `loop_id'.
 */
void update_pre_header(LoopAnalysis& loops, LoopID loop_id, Block* pre_header) {
  auto& loop = loops.loops[loop_id];
  assertx(loops.headers[loop.header] == loop_id);

  loop.pre_header = pre_header;

  for (auto ancestorId = loop.parent; ancestorId != kInvalidLoopID;
       ancestorId = loops.loops[ancestorId].parent) {
    loops.loops[ancestorId].members.insert(pre_header);
  }

  assertx(check_invariants(loops));
}

//////////////////////////////////////////////////////////////////////

std::string show(const LoopInfo& linfo) {
  auto ret = std::string{};
  folly::format(&ret, "Loop {}, header: B{}", linfo.id, linfo.header->id());
  if (linfo.pre_header != nullptr) {
    folly::format(&ret, "  (pre-header: B{})\n", linfo.pre_header->id());
  } else {
    ret += "\n";
  }
  for (auto& b : linfo.members) {
    folly::format(&ret, "                B{}\n", b->id());
  }

  return ret;
}

std::string show(const LoopAnalysis& loops) {
  auto ret = std::string{};
  for (auto& x : loops.loops) ret += show(x);

  folly::format(&ret, "digraph G {{\n");
  for (auto& x : loops.loops) {
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
