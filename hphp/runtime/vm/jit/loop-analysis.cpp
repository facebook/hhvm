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

jit::flat_set<Edge*> find_back_edges(const Env& env) {
  auto vec = jit::vector<Edge*>{};

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

  auto ret = jit::flat_set<Edge*>(begin(vec), end(vec));

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

template<class Visited>
void natural_loop_dfs(jit::vector<Block*>& out, Visited& visited, Block* blk) {
  if (visited.test(blk->id())) return;
  visited.set(blk->id());
  blk->forEachPred([&] (Block* pred) {
    natural_loop_dfs(out, visited, pred);
  });
  out.push_back(blk);
}

NaturalLoopInfo identify_natural_loop(
    Env& env,
    const jit::flat_set<Edge*>& all_back_edges,
    const Edge* back_edge) {
  auto ret = NaturalLoopInfo{};
  ret.header = back_edge->to();

  /*
   * The natural loop associated with this back edge consists of all nodes
   * reachable in the reverse CFG from back_edge->from() before the header.
   * Since back_edge->to() dominates back_edge->from(), we're guaranteed that
   * every path backwards will reach back_edge->to(), which is already marked
   * as visited, and the search will terminate.
   */
  auto visited = boost::dynamic_bitset<>(env.unit.numBlocks());
  visited.set(ret.header->id());
  auto members = jit::vector<Block*>{};
  natural_loop_dfs(members, visited, back_edge->from());
  ret.members = jit::flat_set<Block*>(begin(members), end(members));

  /*
   * Check if this loop has a pre-header.
   */
  ret.pre_header = [&] () -> Block* {
    Edge* candidate = nullptr;
    for (auto& pred_edge : ret.header->preds()) {
      if (all_back_edges.count(&pred_edge)) continue;
      if (candidate) return nullptr;
      candidate = &pred_edge;
      if (candidate->from()->numSuccs() != 1) return nullptr;
    }
    return candidate ? candidate->from() : nullptr;
  }();

  return ret;
}

template<class Analysis, class F>
void for_each_loop(Analysis& loops,
                   LoopID id,
                   LoopID (NaturalLoopInfo::*next_id),
                   F f) {
  while (id != kInvalidLoopID) {
    auto& cur = loops.naturals[id];
    f(cur);
    id = (cur.*next_id);
  }
}

void determine_nesting(Env& env, LoopAnalysis& loops) {
  /*
   * Link all loops with the same header into a list.  An arbitrary one of them
   * will end up as the first in the list, or the "canonical" natural loop for
   * that header.
   */
  for (auto& linfo : loops.naturals) {
    if (loops.headers.contains(linfo.header)) {
      linfo.header_next = loops.headers[linfo.header];
    }
    loops.headers[linfo.header] = linfo.id;
  }

  /*
   * Use the dominator tree to build the loop nest tree.  If a loop B is nested
   * inside loop A, then the header of B is dominated by the header of A, and
   * the header of B is a member of loop A.  We build the tree bottom up by
   * running upwards in the IdomVector for each loop header, looking for
   * another loop header.
   */
  for (auto& header_to_loop_kv : loops.headers) {
    auto const inner_id = header_to_loop_kv.second;
    auto const block = loops.naturals[inner_id].header;

    // Chase up the dominator tree and see if we find a parent loop.
    auto found_outer = false;
    for (auto up = env.idoms[block];
         up != nullptr && !found_outer;
         up = env.idoms[up]) {
      if (!loops.headers.contains(up)) continue;

      // We have a loop header `up' that dominates the loop header `block'.  If
      // any loop headed by `up' contains `block', then it is a parent loop.
      for (auto outer_id = loops.headers[up];
           outer_id != kInvalidLoopID;
           outer_id = loops.naturals[outer_id].header_next) {
        auto& outer = loops.naturals[outer_id];
        if (!outer.members.count(block)) continue;
        auto const canon_up = loops.headers[up];
        for_each_loop(
          loops, inner_id, &NaturalLoopInfo::header_next,
          [&] (NaturalLoopInfo& inner) {
            inner.canonical_outer = canon_up;
          }
        );
        found_outer = true;
        break;
      }
    }
  }
}

void find_inner_loops(LoopAnalysis& loops) {
  auto parents = sparse_id_set<LoopID>(loops.naturals.size());
  for (auto& loop : loops.naturals) {
    for_each_loop(
      loops, loop.canonical_outer, &NaturalLoopInfo::header_next,
      [&] (const NaturalLoopInfo& out) {
        parents.insert(out.id);
      }
    );
  }

  loops.inner_loops.reserve(loops.naturals.size() - parents.size());
  for (auto& loop : loops.naturals) {
    if (!parents.contains(loop.id)) loops.inner_loops.push_back(loop.id);
  }
}

//////////////////////////////////////////////////////////////////////

bool DEBUG_ONLY check_invariants(const LoopAnalysis& loops) {
  always_assert(loops.back_edges.size() == loops.naturals.size());

  for (auto& nat : loops.naturals) {
    // Loops in the same header list have the same header block and the same
    // pre-header block.
    for_each_loop(
      loops, nat.id, &NaturalLoopInfo::header_next,
      [&] (const NaturalLoopInfo& info) {
        always_assert(info.header == nat.header);
        always_assert(info.pre_header == nat.pre_header);
      }
    );

    // Any loop that contains another loop's header block also contains its
    // pre_header block.
    for (auto oid = nat.id + 1; oid < loops.naturals.size(); ++oid) {
      auto& other = loops.naturals[oid];
      if (other.pre_header && nat.members.count(other.header)) {
        // The `members' set does not contain loop headers, so we have to check
        // that too.
        always_assert(nat.members.count(other.pre_header) ||
                      nat.header == other.pre_header);
      }
    }
  }

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

  ret.naturals.reserve(ret.back_edges.size());
  for (auto& edge : ret.back_edges) {
    ret.naturals.push_back(identify_natural_loop(env, ret.back_edges, edge));
    ret.naturals.back().id = ret.naturals.size() - 1;
  }

  determine_nesting(env, ret);
  find_inner_loops(ret);

  assertx(check_invariants(ret));

  return ret;
}

jit::flat_set<Block*> expanded_loop_blocks(const LoopAnalysis& loops,
                                           LoopID loop_id) {
  auto ret = loops.naturals[loop_id].members;
  if (loops.naturals[loop_id].header_next == kInvalidLoopID) {
    return ret;
  }

  // Start at the second one in the list, inserting the new members.
  for_each_loop(
    loops,
    loops.naturals[loops.naturals[loop_id].header_next].id,
    &NaturalLoopInfo::header_next,
    [&] (const NaturalLoopInfo& info) {
      for (auto& m : info.members) {
        ret.insert(m);
      }
    }
  );

  return ret;
}

//////////////////////////////////////////////////////////////////////

void insert_loop_pre_header(IRUnit& unit,
                            LoopAnalysis& loops,
                            LoopID loop_id) {
  assertx(check_invariants(loops));
  always_assert(loops.naturals[loop_id].pre_header == nullptr);

  // All natural loops with the same header are going to be affected, so find
  // the front of that list.
  auto const first_loop_id = loops.headers[loops.naturals[loop_id].header];
  auto const& first_loop = loops.naturals[first_loop_id];
  auto const header = first_loop.header;

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
      first_loop_id,
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
  update_pre_header(loops, first_loop_id, pre_header);
}

/*
 * We only need to add the new pre_header block to any loop that contained the
 * header block for loop_id, as well as to any loop that has the same header as
 * loop_id.
 *
 * For loops that aren't at the same header as this loop_id:
 *
 *   If a loop L contained loop_id's header block, then the header block had a
 *   path to L's back edge that didn't go through L's header.  Since the new
 *   pre header has a path to the header (as its only successor), this means
 *   the pre header also has a path to L's back edge that doesn't go through
 *   L's header, so it's part of L's loop.
 *
 *   On the other hand, if a loop didn't contain loop_id's header, it means the
 *   new pre_header can't have a path to that loop's back edge that doesn't go
 *   through its header either, since the only successor of the new pre_header
 *   is the header.
 *
 *   Furthermore, natural loops with different header blocks are either totally
 *   disjoint or else one contains the other.  Since we've built the loop nest
 *   already, this means we can find all the loops that may contain the header
 *   block by chasing up the loop parent pointers: since if a loop contained
 *   this loop's header, it must be a parent loop.
 */
void update_pre_header(LoopAnalysis& loops, LoopID loop_id, Block* new_preh) {
  assertx(loops.headers[loops.naturals[loop_id].header] == loop_id);
  auto const header = loops.naturals[loop_id].header;

  for_each_loop(
    loops, loop_id, &NaturalLoopInfo::header_next,
    [&] (NaturalLoopInfo& info) {
      info.pre_header = new_preh;
    }
  );

  for_each_loop(
    loops, loop_id, &NaturalLoopInfo::canonical_outer,
    [&] (const NaturalLoopInfo& canon_out) {
      for_each_loop(
        loops, canon_out.id, &NaturalLoopInfo::header_next,
        [&] (NaturalLoopInfo& possible_parent) {
          if (possible_parent.members.count(header)) {
            possible_parent.members.insert(new_preh);
          }
        }
      );
    }
  );

  assertx(check_invariants(loops));
}

//////////////////////////////////////////////////////////////////////

std::string show(const NaturalLoopInfo& linfo) {
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
  for (auto& x : loops.naturals) ret += show(x);

  folly::format(&ret, "digraph G {{\n");
  for (auto& x : loops.naturals) {
    folly::format(&ret, "L{};\n", x.id);

    if (x.header_next != kInvalidLoopID) {
      folly::format(&ret, "L{} -> L{} [color=green];\n", x.id, x.header_next);
    }
    if (x.canonical_outer != kInvalidLoopID) {
      folly::format(&ret, "L{} -> L{};\n", x.id, x.canonical_outer);
    }
  }
  folly::format(&ret, "}}\n");

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
