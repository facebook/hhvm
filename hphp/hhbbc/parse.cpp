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
#include "hphp/hhbbc/parse.h"

#include <thread>
#include <unordered_map>
#include <map>

#include <boost/variant.hpp>
#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include <folly/gen/Base.h>
#include <folly/gen/String.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/sorted_vector_types.h>

#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_parse);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Closure("Closure");
const StaticString s_toString("__toString");
const StaticString s_Stringish("Stringish");
const StaticString s_XHPChild("XHPChild");
const StaticString s_86cinit("86cinit");
const StaticString s_86sinit("86sinit");
const StaticString s_86linit("86linit");
const StaticString s_86pinit("86pinit");
const StaticString s_attr_Deprecated("__Deprecated");
const StaticString s_class_alias("class_alias");

//////////////////////////////////////////////////////////////////////

void record_const_init(php::Program& prog, uintptr_t encoded_func) {
  auto id = prog.nextConstInit.fetch_add(1, std::memory_order_relaxed);
  prog.constInits.ensureSize(id + 1);

  DEBUG_ONLY auto const oldVal = prog.constInits.exchange(id, encoded_func);
  assert(oldVal == 0);
}

//////////////////////////////////////////////////////////////////////

struct ParseUnitState {
  std::atomic<uint32_t>& nextFuncId;

  /*
   * This is computed once for each unit and stashed here.  We support
   * having either a SourceLocTable or a LineTable.  If we're
   * optimizing a repo that was already created by hphpc, it won't
   * have the full SourceLocTable information in it, so we're limited
   * to line numbers.
   */
  boost::variant< SourceLocTable
                , LineTable
                > srcLocInfo;

  /*
   * Map from class id to the function containing its DefCls
   * instruction.  We use this to compute whether classes are defined
   * at top-level.
   *
   * TODO_4: if we don't end up with a use for this, remove it.
   */
  std::vector<php::Func*> defClsMap;

  /*
   * Map from Closure index to the function(s) containing their
   * associated CreateCl opcode(s).
   */
  hphp_fast_map<
    int32_t,
    hphp_fast_set<php::Func*>
  > createClMap;

  struct SrcLocHash {
    size_t operator()(const php::SrcLoc& sl) const {
      auto const h1 = ((size_t)sl.start.col << 32) | sl.start.line;
      auto const h2 = ((size_t)sl.past.col << 32) | sl.past.line;
      return hash_int64_pair(h1, h2);
    }
  };
  hphp_fast_map<php::SrcLoc, int32_t, SrcLocHash> srcLocs;

  /*
   * Set of functions that should be processed in the constant
   * propagation pass.
   *
   * Must include every function with a DefCns for correctness; cinit,
   * pinit and sinit functions are added to improve overall
   * performance.
   */
  hphp_fast_map<php::Func*, int> constPassFuncs;

  /*
   * List of class aliases defined by this unit
   */
  CompactVector<std::pair<SString,SString>> classAliases;
};

//////////////////////////////////////////////////////////////////////

std::set<Offset> findBasicBlocks(const FuncEmitter& fe) {
  std::set<Offset> blockStarts;
  auto markBlock = [&] (Offset off) { blockStarts.insert(off); };

  // Each entry point for a DV funclet is the start of a basic
  // block.
  for (auto& param : fe.params) {
    if (param.hasDefaultValue()) markBlock(param.funcletOff);
  }

  // The main entry point is also a basic block start.
  markBlock(fe.base);

  bool traceBc = false;

  /*
   * For each instruction, add it to the set if it must be the start
   * of a block.  It is the start of a block if it is:
   *
   *   - A jump target
   *
   *   - Immediatelly following a control flow instruction, other than
   *     a call.
   */
  auto offset = fe.base;
  for (;;) {
    auto const bc = fe.ue().bc();
    auto const pc = bc + offset;
    auto const nextOff = offset + instrLen(pc);
    auto const atLast = nextOff == fe.past;
    auto const op = peek_op(pc);
    auto const breaksBB = instrIsNonCallControlFlow(op) || instrFlags(op) & TF;

    if (options.TraceBytecodes.count(op)) traceBc = true;

    if (breaksBB && !atLast) {
      markBlock(nextOff);
    }

    if (isSwitch(op)) {
      foreachSwitchTarget(pc, [&] (Offset delta) {
        markBlock(offset + delta);
      });
    } else {
      auto const target = instrJumpTarget(bc, offset);
      if (target != InvalidAbsoluteOffset) markBlock(target);
    }

    offset = nextOff;
    if (atLast) break;
  }

  /*
   * Find blocks associated with exception handlers.
   *
   *   - The start of each fault-protected region begins a block.
   *
   *   - The instruction immediately after the end of any
   *     fault-protected region begins a block.
   *
   *   - Each fault or catch entry point begins a block.
   *
   *   - The instruction immediately after the end of any
   *     fault or catch region begins a block.
   */
  for (auto& eh : fe.ehtab) {
    markBlock(eh.m_base);
    markBlock(eh.m_past);
    markBlock(eh.m_handler);
    if (eh.m_end != kInvalidOffset) {
      markBlock(eh.m_end);
    }
  }

  // Now, each interval in blockStarts delinates a basic block.
  blockStarts.insert(fe.past);

  if (traceBc) {
    FTRACE(0, "TraceBytecode (parse): {}::{} in {}\n",
           fe.pce() ? fe.pce()->name()->data() : "",
           fe.name, fe.ue().m_filepath);
  }

  return blockStarts;
}

struct ExnTreeInfo {
  /*
   * Map from EHEnt to the ExnNode that will represent exception
   * behavior in that region.
   */
  hphp_fast_map<const EHEnt*,php::ExnNode*> ehMap;

  /*
   * Keep track of the start offsets for all fault funclets.  This is
   * used to find the extents of each handler for find_fault_funclets.
   * It is assumed that each fault funclet handler extends from its
   * entry offset until the next fault funclet entry offset (or end of
   * the function).
   *
   * This relies on the following bytecode invariants:
   *
   *   - All fault funclets come after the primary function body.
   *
   *   - Each fault funclet is a contiguous region of bytecode that
   *     does not jump into other fault funclets or into the primary
   *     function body.
   *
   *   - Nothing comes after the fault funclets.
   */
  std::set<Offset> faultFuncletStarts;
};

template<class FindBlock>
ExnTreeInfo build_exn_tree(const FuncEmitter& fe,
                           php::Func& func,
                           FindBlock findBlock) {
  ExnTreeInfo ret;
  auto nextExnNode = uint32_t{0};

  for (auto& eh : fe.ehtab) {
    auto node = std::make_unique<php::ExnNode>();
    node->id = nextExnNode++;
    node->parent = nullptr;
    node->depth = 1; // 0 depth means no ExnNode

    switch (eh.m_type) {
    case EHEnt::Type::Fault:
      {
        auto const fault = findBlock(eh.m_handler);
        ret.faultFuncletStarts.insert(eh.m_handler);
        node->info = php::FaultRegion { fault->id, eh.m_iterId, eh.m_itRef };
      }
      break;
    case EHEnt::Type::Catch:
      {
        auto const catchBlk = findBlock(eh.m_handler);
        node->info = php::CatchRegion { catchBlk->id, eh.m_iterId, eh.m_itRef };
      }
      break;
    }

    ret.ehMap[&eh] = node.get();

    if (eh.m_parentIndex != -1) {
      auto it = ret.ehMap.find(&fe.ehtab[eh.m_parentIndex]);
      assert(it != end(ret.ehMap));
      node->parent = it->second;
      node->depth = node->parent->depth + 1;
      it->second->children.emplace_back(std::move(node));
    } else {
      func.exnNodes.emplace_back(std::move(node));
    }
  }

  ret.faultFuncletStarts.insert(fe.past);

  return ret;
}

/*
 * Locate all the basic blocks associated with fault funclets, and
 * mark them as such.
 */
template <class BlockStarts, class FindBlock>
void find_fault_funclets(ExnTreeInfo& tinfo, const php::Func& /*func*/,
                         const BlockStarts& blockStarts, FindBlock findBlock) {
  auto sectionId = uint32_t{1};

  for (auto funcletStartIt = begin(tinfo.faultFuncletStarts);
      std::next(funcletStartIt) != end(tinfo.faultFuncletStarts);
      ++funcletStartIt, ++sectionId) {
    auto const nextFunclet = *std::next(funcletStartIt);

    auto offIt = blockStarts.find(*funcletStartIt);
    assert(offIt != end(blockStarts));

    do {
      auto const blk = findBlock(*offIt);
      blk->section   = static_cast<php::Block::Section>(sectionId);
      ++offIt;
    } while (offIt != end(blockStarts) && *offIt < nextFunclet);
  }
}

BlockId node_entry_block(const php::ExnNode& node) {
  return match<BlockId>(
    node.info,
    [] (const php::CatchRegion& cr) { return cr.catchEntry; },
    [] (const php::FaultRegion& fr) { return fr.faultEntry; }
  );
}

/*
 * Build the exceptional edge lists for a function for the simple (and common
 * case). If the function's exception tree is all flat (no children), then
 * unwind edges are always empty (you cannot unwind from the main section, and
 * if you're in the fault handler you'll exit the function). Throw exits just
 * jump to the associated handler (and any throw in the handler will exit the
 * function).
 */
void build_exceptional_edges_simple(const php::Func& func) {
  for (auto& blk : func.blocks) {
    assert(blk->throwExits.empty());
    assert(blk->unwindExits.empty());
    if (!blk->exnNode) continue;
    blk->throwExits.push_back(node_entry_block(*blk->exnNode));
  }
}

/*
 * Populate the throw and unwind edges for all blocks in the function.
 *
 * - An Unwind instruction will jump to the handler's parent, if any. If the
 *   handler has no parent, it will either exit the function (and thus no edge),
 *   or it will jump to a different handler, depending on the unwinder's state
 *   machine.
 *
 * - A throwing instruction (which can happen anytime within a block) will jump
 *   to the block's exception handler (given by the exnNode), if any. If there
 *   is not, and we're in a handler, it will jump to the same place an Unwind
 *   instruction would (which may be nowhere).
 *
 * These cases are difficult to get right (especially when unwinding without a
 * parent region). So, we simulate the unwinder as a state machine and find the
 * closure of all of its possible states. From these states we can then infer
 * which edges can be traversed by the unwinder. Once we have the unwinder
 * edges, we can then calculate the throw edges.
 */
void build_exceptional_edges(const ExnTreeInfo& tinfo, const php::Func& func) {
  // Check for the simple and quicker cases
  if (func.exnNodes.empty()) return;
  if (std::all_of(
        func.exnNodes.begin(),
        func.exnNodes.end(),
        [](auto const& n){ return n->children.empty(); })
     ) {
    return build_exceptional_edges_simple(func);
  }

  FTRACE(4, "    -------- build exceptional edges (full) --------\n");
  FTRACE(8, "{}\n", show(func));

  // Map of exceptional regions that can be entered from other exceptional
  // regions via throws.
  folly::sorted_vector_map<
    const php::ExnNode*,
    folly::sorted_vector_set<const php::ExnNode*>
  > throws;

  // Map of blocks to the exceptional regions that block belongs to (a block can
  // belong to multiple exceptional regions at once).
  folly::sorted_vector_map<
    BlockId,
    folly::sorted_vector_set<const php::ExnNode*>
  > blocksToNodes;

  auto const add = [&] (const php::Block& blk, const php::ExnNode* node) {
    assert(blk.throwExits.empty());
    assert(blk.unwindExits.empty());
    if (node) blocksToNodes[blk.id].insert(node);
    if (!blk.exnNode) return;
    throws[node].insert(blk.exnNode);
  };

  // There's no easy way to determine which blocks belong to which exceptional
  // regions, except by doing a walk from each region's entry block.
  auto const mainBlocks = rpoSortAddDVs(func);
  for (auto const& blk : mainBlocks) add(*blk, nullptr);

  uint32_t nodeCount = 0;
  visitExnLeaves(
    func,
    [&] (const php::ExnNode& node) {
      ++nodeCount;
      auto const blocks = rpoSortFromBlock(func, node_entry_block(node));
      for (auto const& blk : blocks) add(*blk, &node);
    }
  );

  FTRACE(
    8, "    throws: {}\n",
    [&]{
      using namespace folly::gen;
      return from(throws)
        | map([] (auto const& p) {
            return folly::sformat(
              "{} -> {}",
              p.first ? folly::sformat("E{}", p.first->id) : "*",
              from(p.second)
                | map([] (auto const& n) {
                    return folly::sformat("E{}", n->id);
                  })
                | unsplit<std::string>(",")
            );
          })
        | unsplit<std::string>(", ");
    }()
  );

  FTRACE(
    8, "    blocks to nodes: {}\n",
    [&]{
      using namespace folly::gen;
      return from(blocksToNodes)
        | map([] (auto const& p) {
            return folly::sformat(
              "B{} -> {}",
              p.first,
              from(p.second)
                | map([] (auto const& n) {
                    return folly::sformat("E{}", n->id);
                  })
                | unsplit<std::string>(",")
            );
          })
        | unsplit<std::string>(", ");
    }()
  );

  /* Unwinder state machine simulation */

  // The unwinder's state is a stack of regions (the current region is just the
  // top of the stack).
  using State = std::vector<const php::ExnNode*>;

  DEBUG_ONLY auto const showState = [&](const State& state) {
    using namespace folly::gen;
    return from(state)
    | map([&] (const php::ExnNode* n) {
        return folly::sformat("E{}", n->id);
      })
    | unsplit<std::string>("->");
  };

  folly::sorted_vector_set<State> states;
  folly::sorted_vector_set<State> newStates;
  folly::sorted_vector_map<
    const php::ExnNode*,
    folly::sorted_vector_set<const php::ExnNode*>
  > unwindEdges;

  auto iters = 0;
  auto changed = false;

  auto const dumpStates = [&] (const char* header, int space, int level) {
    FTRACE(
      level, "{}{}:\n{}\n",
      std::string(space, ' '),
      header,
      [&]{
        using namespace folly::gen;
        return from(states)
          | map([&] (const State& state) {
              return folly::sformat(
                "{}{}",
                std::string(space+2, ' '),
                showState(state)
              );
            })
          | unsplit<std::string>("\n");
      }()
    );
  };

  /*
   * We need to find all the possible states that the unwinder can be in from
   * the (known) initial states. These initial states are just the regions that
   * are reachable via throws from the main region.
   *
   * At each round, we take the current known states and then apply the various
   * unwinder rules to them to generate new states. We accumulate the states
   * until we reach a fixed point (no new states encountered). At this point,
   * the edges between the various states give the unwind edges between regions.
   */

  for (auto const& dst : throws[nullptr]) states.insert({dst});
  do {
    FTRACE(6, "    -- iteration #{}\n", iters+1);
    dumpStates("start", 6, 6);

    FTRACE(6, "      update:\n");
    // Iterate over new states, generate new ones.
    for (auto state : states) {
      assert(!state.empty());
      auto const current = state.back();
      assert(current);

      FTRACE(6, "        * {}\n", showState(state));

      // Sanity check. In a well-formed func, we should always reach a
      // fixed-point. This is a very crude test to avoid looping forever. We
      // should never have a stack with duplicate regions.
      always_assert(state.size() <= nodeCount);

      // If this is a catch region, the unwinder pops off the top of its stack
      // first.
      match<void>(
        current->info,
        [&](const php::CatchRegion&) {
          state.pop_back();
          FTRACE(6, "          Pop (catch)\n");
        },
        [] (const php::FaultRegion&) {}
      );

      // For each region that we can jump to via a throw from this region,
      // record a new state. The new state is the current state with the
      // destination region pushed onto it.
      for (auto const& dst : throws[current]) {
        state.push_back(dst);
        FTRACE(6, "          => {} (throw)\n", showState(state));
        newStates.insert(state);
        state.pop_back();
      }

      // Handle the unwind case. While the top doesn't have a parent, pop. If
      // the stack is now empty, an unwind will exit the function, so there's no
      // new state.
      while (!state.empty() && !state.back()->parent) state.pop_back();
      if (state.empty()) continue;

      // The stack is non-empty and the top has a parent. We'll unwind to the
      // top's parent, so record that new state and record the edge.
      auto const oldNode = state.back();
      state.back() = oldNode->parent;
      FTRACE(6, "          =>  {} (unwind E{} to E{})\n",
             showState(state), oldNode->id, state.back()->id);
      newStates.insert(state);
      unwindEdges[current].insert(state.back());
    }

    // Merge the new states into the old ones. If we've generated new ones, then
    // we have to repeat.
    auto const old = states.size();
    states.insert(newStates.begin(), newStates.end());
    changed = states.size() != old;
    newStates.clear();
    ++iters;
  } while (changed);

  FTRACE(4, "    fixed-point reached in {} iterations\n", iters);
  dumpStates("fixed-point", 4, 4);

  FTRACE(
    6, "    unwind edges: {}\n",
    [&]{
      using namespace folly::gen;
      return from(unwindEdges)
        | map([] (auto const& p) {
            return folly::sformat(
              "E{} -> {}",
              p.first->id,
              from(p.second)
                | map([] (auto const& n) {
                    return folly::sformat("E{}", n->id);
                  })
                | unsplit<std::string>(",")
            );
          })
        | unsplit<std::string>(", ");
    }()
  );

  // Now that we have the unwind edges for regions, we can populate the block's
  // unwind edges. For each block, they're just the union of the unwind edges of
  // all the regions the block belongs to. With the unwind edges done, the throw
  // edges are easily computed.
  auto const unwindExitsForBlock = [&] (BlockId blk) {
    CompactVector<BlockId> exits;
    for (auto const& node : blocksToNodes[blk]) {
      for (auto const& edge : unwindEdges[node]) {
        exits.push_back(node_entry_block(*edge));
      }
    }
    // We might have duplicate exits now, so we need to remove them.
    std::sort(exits.begin(), exits.end());
    exits.resize(std::unique(exits.begin(), exits.end()) - exits.begin());
    return exits;
  };

  for (auto& blk : func.blocks) {
    assert(blk->throwExits.empty());
    assert(blk->unwindExits.empty());

    if (ends_with_unwind(*blk)) {
      blk->unwindExits = unwindExitsForBlock(blk->id);
    }

    if (blk->exnNode) {
      blk->throwExits.push_back(node_entry_block(*blk->exnNode));
    } else if (blk->section != php::Block::Section::Main) {
      blk->throwExits = unwindExitsForBlock(blk->id);
    }

    FTRACE(
      8, "    blk:{} (throw:{}) (unwind:{})\n",
      blk->id,
      [&]{
        using namespace folly::gen;
        return from(blk->throwExits)
          | map([] (BlockId b) { return folly::sformat(" blk:{}", b); })
          | unsplit<std::string>("");
      }(),
      [&]{
        using namespace folly::gen;
        return from(blk->unwindExits)
          | map([] (BlockId b) { return folly::sformat(" blk:{}", b); })
          | unsplit<std::string>("");
      }()
    );
  }
}

template<class T> T decode(PC& pc) {
  auto const ret = *reinterpret_cast<const T*>(pc);
  pc += sizeof ret;
  return ret;
}

template<class T> void decode(PC& pc, T& val) {
  val = decode<T>(pc);
}

MKey make_mkey(const php::Func& /*func*/, MemberKey mk) {
  switch (mk.mcode) {
    case MEL: case MPL:
      return MKey{mk.mcode, static_cast<LocalId>(mk.iva)};
    case MEC: case MPC:
      return MKey{mk.mcode, mk.iva};
    case MET: case MPT: case MQT:
      return MKey{mk.mcode, mk.litstr};
    case MEI:
      return MKey{mk.mcode, mk.int64};
    case MW:
      return MKey{};
  }
  not_reached();
}

template<class FindBlock>
void populate_block(ParseUnitState& puState,
                    const FuncEmitter& fe,
                    php::Func& func,
                    php::Block& blk,
                    PC pc,
                    PC const past,
                    FindBlock findBlock) {
  auto const& ue = fe.ue();

  auto decode_stringvec = [&] {
    auto const vecLen = decode_iva(pc);
    CompactVector<LSString> keys;
    for (auto i = size_t{0}; i < vecLen; ++i) {
      keys.push_back(ue.lookupLitstr(decode<int32_t>(pc)));
    }
    return keys;
  };

  auto decode_switch = [&] (PC opPC) {
    SwitchTab ret;
    auto const vecLen = decode_iva(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      ret.push_back(findBlock(
        opPC + decode<Offset>(pc) - ue.bc()
      )->id);
    }
    return ret;
  };

  auto decode_sswitch = [&] (PC opPC) {
    SSwitchTab ret;

    auto const vecLen = decode_iva(pc);
    for (int32_t i = 0; i < vecLen - 1; ++i) {
      auto const id = decode<Id>(pc);
      auto const offset = decode<Offset>(pc);
      ret.emplace_back(ue.lookupLitstr(id),
                       findBlock(opPC + offset - ue.bc())->id);
    }

    // Final case is the default, and must have a litstr id of -1.
    DEBUG_ONLY auto const defId = decode<Id>(pc);
    auto const defOff = decode<Offset>(pc);
    assert(defId == -1);
    ret.emplace_back(nullptr, findBlock(opPC + defOff - ue.bc())->id);
    return ret;
  };

  auto decode_itertab = [&] {
    IterTab ret;
    auto const vecLen = decode_iva(pc);
    for (int32_t i = 0; i < vecLen; ++i) {
      auto const kind = static_cast<IterKind>(decode_iva(pc));
      auto const id = decode_iva(pc);
      auto const local = [&]{
        if (kind != KindOfLIter) return NoLocalId;
        auto const loc = decode_iva(pc);
        always_assert(loc < func.locals.size());
        return loc;
      }();
      ret.push_back(IterTabEnt{kind, static_cast<IterId>(id), local});
    }
    return ret;
  };

  auto decode_argv32 = [&] {
    CompactVector<uint32_t> ret;
    auto const vecLen = decode_iva(pc);
    for (uint32_t i = 0; i < vecLen; ++i) {
      ret.emplace_back(decode<uint32_t>(pc));
    }
    return ret;
  };

  auto decode_argvb = [&] {
    CompactVector<bool> ret;
    auto const vecLen = decode_iva(pc);
    uint8_t tmp = 0;
    for (uint32_t i = 0; i < vecLen; ++i) {
      if (i % 8 == 0) tmp = decode<uint8_t>(pc);
      ret.emplace_back((tmp >> (i % 8)) & 1);
    }
    return ret;
  };

  auto defcns = [&] () {
    puState.constPassFuncs[&func] |= php::Program::ForAnalyze;
  };
  auto addelem = [&] () {
    puState.constPassFuncs[&func] |= php::Program::ForOptimize;
  };
  auto defcls = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefCls.arg1] = &func;
  };
  auto defclsnop = [&] (const Bytecode& b) {
    puState.defClsMap[b.DefClsNop.arg1] = &func;
  };
  auto aliascls = [&] (const Bytecode& b) {
    puState.classAliases.emplace_back(b.AliasCls.str1, b.AliasCls.str2);
  };
  auto createcl = [&] (const Bytecode& b) {
    puState.createClMap[b.CreateCl.arg2].insert(&func);
  };
  auto fpushfuncu = [&] (const Bytecode& b) {
    if (b.FPushFuncU.str3 == s_class_alias.get()) {
      puState.constPassFuncs[&func] |= php::Program::ForAnalyze;
    }
  };
  auto fpushfuncd = [&] (const Bytecode& b) {
    if (b.FPushFuncD.str2 == s_class_alias.get()) {
      puState.constPassFuncs[&func] |= php::Program::ForAnalyze;
    }
  };
  auto has_call_unpack = [&] {
    auto const fpi = Func::findFPI(&*fe.fpitab.begin(),
                                   &*fe.fpitab.end(), pc - ue.bc());
    auto pc = ue.bc() + fpi->m_fpiEndOff;
    auto const op = decode_op(pc);
    if (op != OpFCall) return false;
    decode_iva(pc);
    return decode_iva(pc) != 0;
  };

#define IMM_BLA(n)     auto targets = decode_switch(opPC);
#define IMM_SLA(n)     auto targets = decode_sswitch(opPC);
#define IMM_ILA(n)     auto iterTab = decode_itertab();
#define IMM_I32LA(n)   auto argv = decode_argv32();
#define IMM_BLLA(n)    auto argv = decode_argvb();
#define IMM_IVA(n)     auto arg##n = decode_iva(pc);
#define IMM_I64A(n)    auto arg##n = decode<int64_t>(pc);
#define IMM_LA(n)      auto loc##n = [&] {                       \
                         LocalId id = decode_iva(pc);            \
                         always_assert(id < func.locals.size()); \
                         return id;                              \
                       }();
#define IMM_IA(n)      auto iter##n = [&] {                      \
                         IterId id = decode_iva(pc);             \
                         always_assert(id < func.numIters);      \
                         return id;                              \
                       }();
#define IMM_CAR(n)     auto slot = [&] {                                \
                         ClsRefSlotId id = decode_iva(pc);              \
                         always_assert(id >= 0 && id < func.numClsRefSlots); \
                         return id;                                     \
                       }();
#define IMM_CAW(n)     auto slot = [&] {                                \
                         ClsRefSlotId id = decode_iva(pc);              \
                         always_assert(id >= 0 && id < func.numClsRefSlots); \
                         return id;                                     \
                       }();
#define IMM_DA(n)      auto dbl##n = decode<double>(pc);
#define IMM_SA(n)      auto str##n = ue.lookupLitstr(decode<Id>(pc));
#define IMM_RATA(n)    auto rat = decodeRAT(ue, pc);
#define IMM_AA(n)      auto arr##n = ue.lookupArray(decode<Id>(pc));
#define IMM_BA(n)      assert(next == past);     \
                       auto target = findBlock(  \
                         opPC + decode<Offset>(pc) - ue.bc())->id;
#define IMM_OA_IMPL(n) subop##n; decode(pc, subop##n);
#define IMM_OA(type)   type IMM_OA_IMPL
#define IMM_VSA(n)     auto keys = decode_stringvec();
#define IMM_KA(n)      auto mkey = make_mkey(func, decode_member_key(pc, &ue));
#define IMM_LAR(n)     auto locrange = [&] {                             \
                         auto const range = decodeLocalRange(pc);        \
                         always_assert(range.first + range.count         \
                                       <= func.locals.size());           \
                         return LocalRange { range.first, range.count }; \
                       }();

#define IMM_NA
#define IMM_ONE(x)           IMM_##x(1)
#define IMM_TWO(x, y)        IMM_##x(1)          IMM_##y(2)
#define IMM_THREE(x, y, z)   IMM_TWO(x, y)       IMM_##z(3)
#define IMM_FOUR(x, y, z, n) IMM_THREE(x, y, z)  IMM_##n(4)
#define IMM_FIVE(x, y, z, n, m) IMM_FOUR(x, y, z, n)  IMM_##m(5)

#define IMM_ARG(which, n)         IMM_NAME_##which(n)
#define IMM_ARG_NA
#define IMM_ARG_ONE(x)            IMM_ARG(x, 1)
#define IMM_ARG_TWO(x, y)         IMM_ARG(x, 1), IMM_ARG(y, 2)
#define IMM_ARG_THREE(x, y, z)    IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                    IMM_ARG(z, 3)
#define IMM_ARG_FOUR(x, y, z, l)  IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                   IMM_ARG(z, 3), IMM_ARG(l, 4)
#define IMM_ARG_FIVE(x, y, z, l, m) IMM_ARG(x, 1), IMM_ARG(y, 2), \
                                   IMM_ARG(z, 3), IMM_ARG(l, 4), \
                                   IMM_ARG(m, 5)

#define FLAGS_NF
#define FLAGS_TF
#define FLAGS_CF
#define FLAGS_FF
#define FLAGS_PF auto hu = has_call_unpack();
#define FLAGS_CF_TF
#define FLAGS_CF_FF

#define FLAGS_ARG_NF
#define FLAGS_ARG_TF
#define FLAGS_ARG_CF
#define FLAGS_ARG_FF
#define FLAGS_ARG_PF ,hu
#define FLAGS_ARG_CF_TF
#define FLAGS_ARG_CF_FF

#define O(opcode, imms, inputs, outputs, flags)                    \
  case Op::opcode:                                                 \
    {                                                              \
      auto b = [&] () -> Bytecode {                                \
        IMM_##imms /*these two macros advance the pc as required*/ \
        FLAGS_##flags                                              \
        if (isTypeAssert(op)) return bc::Nop {};                   \
        return bc::opcode { IMM_ARG_##imms FLAGS_ARG_##flags };    \
      }();                                                         \
      b.srcLoc = srcLocIx;                                         \
      if (Op::opcode == Op::DefCns) defcns();                      \
      if (Op::opcode == Op::AddElemC ||                            \
          Op::opcode == Op::AddNewElemC) addelem();                \
      if (Op::opcode == Op::DefCls)      defcls(b);                \
      if (Op::opcode == Op::DefClsNop)   defclsnop(b);             \
      if (Op::opcode == Op::AliasCls)    aliascls(b);              \
      if (Op::opcode == Op::CreateCl)    createcl(b);              \
      if (Op::opcode == Op::FPushFuncU)  fpushfuncu(b);            \
      if (Op::opcode == Op::FPushFuncD)  fpushfuncd(b);            \
      blk.hhbcs.push_back(std::move(b));                           \
      assert(pc == next);                                          \
    }                                                              \
    break;

  assert(pc != past);
  do {
    auto const opPC = pc;
    auto const next = pc + instrLen(opPC);
    assert(next <= past);

    auto const srcLoc = match<php::SrcLoc>(
      puState.srcLocInfo,
      [&] (const SourceLocTable& tab) {
        SourceLoc sloc;
        if (getSourceLoc(tab, opPC - ue.bc(), sloc)) {
          return php::SrcLoc {
            { static_cast<uint32_t>(sloc.line0),
              static_cast<uint32_t>(sloc.char0) },
            { static_cast<uint32_t>(sloc.line1),
              static_cast<uint32_t>(sloc.char1) }
          };
        }
        return php::SrcLoc{};
      },
      [&] (const LineTable& tab) {
        auto const line = getLineNumber(tab, opPC - ue.bc());
        if (line != -1) {
          return php::SrcLoc {
            { static_cast<uint32_t>(line), 0 },
            { static_cast<uint32_t>(line), 0 },
          };
        };
        return php::SrcLoc{};
      }
    );

    auto const srcLocIx = puState.srcLocs.emplace(
      srcLoc, puState.srcLocs.size()).first->second;

    auto const op = decode_op(pc);
    switch (op) { OPCODES }

    if (next == past) {
      if (instrAllowsFallThru(op)) {
        blk.fallthrough = findBlock(next - ue.bc())->id;
      }
    }

    pc = next;
  } while (pc != past);

#undef O

#undef FLAGS_NF
#undef FLAGS_TF
#undef FLAGS_CF
#undef FLAGS_FF
#undef FLAGS_PF
#undef FLAGS_CF_TF
#undef FLAGS_CF_FF

#undef FLAGS_ARG_NF
#undef FLAGS_ARG_TF
#undef FLAGS_ARG_CF
#undef FLAGS_ARG_FF
#undef FLAGS_ARG_PF
#undef FLAGS_ARG_CF_TF
#undef FLAGS_ARG_CF_FF

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_I32LA
#undef IMM_BLLA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA
#undef IMM_LAR

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR
#undef IMM_FIVE

#undef IMM_ARG
#undef IMM_ARG_NA
#undef IMM_ARG_ONE
#undef IMM_ARG_TWO
#undef IMM_ARG_THREE
#undef IMM_ARG_FOUR
#undef IMM_ARG_FIVE

  /*
   * If a block ends with an unconditional jump, change it to a
   * fallthrough edge.
   *
   * Just convert the opcode to a Nop, because this could create an
   * empty block and we have an invariant that no blocks are empty.
   */

  auto make_fallthrough = [&] {
    blk.fallthrough = blk.hhbcs.back().Jmp.target;
    blk.hhbcs.back() = bc_with_loc(blk.hhbcs.back().srcLoc, bc::Nop{});
  };

  switch (blk.hhbcs.back().op) {
  case Op::Jmp:   make_fallthrough();                           break;
  case Op::JmpNS: make_fallthrough(); blk.fallthroughNS = true; break;
  default:                                                      break;
  }
}

template<class FindBlk>
void link_entry_points(php::Func& func,
                       const FuncEmitter& fe,
                       FindBlk findBlock) {
  func.dvEntries.resize(fe.params.size(), NoBlockId);
  for (size_t i = 0, sz = fe.params.size(); i < sz; ++i) {
    if (fe.params[i].hasDefaultValue()) {
      auto const dv = findBlock(fe.params[i].funcletOff)->id;
      func.params[i].dvEntryPoint = dv;
      func.dvEntries[i] = dv;
    }
  }
  func.mainEntry = findBlock(fe.base)->id;
}

void build_cfg(ParseUnitState& puState,
               php::Func& func,
               const FuncEmitter& fe) {
  auto const blockStarts = findBasicBlocks(fe);

  FTRACE(3, "    blocks are at: {}\n",
    [&]() -> std::string {
      using namespace folly::gen;
      return from(blockStarts)
        | eachTo<std::string>()
        | unsplit<std::string>(" ");
    }()
  );

  std::map<Offset,std::unique_ptr<php::Block>> blockMap;
  auto const bc = fe.ue().bc();

  auto findBlock = [&] (Offset off) {
    auto& ptr = blockMap[off];
    if (!ptr) {
      ptr               = std::make_unique<php::Block>();
      ptr->id           = blockMap.size() - 1;
      ptr->section      = php::Block::Section::Main;
      ptr->exnNode      = nullptr;
    }
    return ptr.get();
  };

  auto exnTreeInfo = build_exn_tree(fe, func, findBlock);

  for (auto it = begin(blockStarts);
       std::next(it) != end(blockStarts);
       ++it) {
    auto const block   = findBlock(*it);
    auto const bcStart = bc + *it;
    auto const bcStop  = bc + *std::next(it);

    if (auto const eh = Func::findEH(fe.ehtab, *it)) {
      auto it = exnTreeInfo.ehMap.find(eh);
      assert(it != end(exnTreeInfo.ehMap));
      block->exnNode = it->second;
    }

    populate_block(puState, fe, func, *block, bcStart, bcStop, findBlock);
  }

  link_entry_points(func, fe, findBlock);
  find_fault_funclets(exnTreeInfo, func, blockStarts, findBlock);

  func.blocks.resize(blockMap.size());
  for (auto& kv : blockMap) {
    auto const id = kv.second->id;
    func.blocks[id] = std::move(kv.second);
  }

  build_exceptional_edges(exnTreeInfo, func);
}

void add_frame_variables(php::Func& func, const FuncEmitter& fe) {
  for (auto& param : fe.params) {
    func.params.push_back(
      php::Param {
        param.defaultValue,
        NoBlockId,
        param.typeConstraint,
        param.userType,
        param.phpCode,
        param.userAttributes,
        param.builtinType,
        param.inout,
        param.byRef,
        param.variadic
      }
    );
  }

  func.locals.reserve(fe.numLocals());
  for (LocalId id = 0; id < fe.numLocals(); ++id) {
    func.locals.push_back({nullptr, id, false});
  }
  for (auto& kv : fe.localNameMap()) {
    func.locals[kv.second].name = kv.first;
  }

  func.numIters = fe.numIterators();
  func.numClsRefSlots = fe.numClsRefSlots();

  func.staticLocals.reserve(fe.staticVars.size());
  for (auto& sv : fe.staticVars) {
    func.staticLocals.push_back(
      php::StaticLocalInfo { sv.name }
    );
  }
}

std::unique_ptr<php::Func> parse_func(ParseUnitState& puState,
                                      php::Unit* unit,
                                      php::Class* cls,
                                      const FuncEmitter& fe) {
  FTRACE(2, "  func: {}\n",
    fe.name->data() && *fe.name->data() ? fe.name->data() : "pseudomain");

  auto ret         = std::make_unique<php::Func>();
  ret->idx         = puState.nextFuncId.fetch_add(1, std::memory_order_relaxed);
  ret->name        = fe.name;
  ret->srcInfo     = php::SrcInfo { fe.getLocation(),
                                    fe.docComment };
  ret->unit        = unit;
  ret->cls         = cls;

  ret->attrs              = static_cast<Attr>(fe.attrs & ~AttrNoOverride);
  ret->userAttributes     = fe.userAttributes;
  ret->returnUserType     = fe.retUserType;
  ret->retTypeConstraint  = fe.retTypeConstraint;
  ret->originalFilename   = fe.originalFilename;

  ret->top                 = fe.top;
  ret->isClosureBody       = fe.isClosureBody;
  ret->isAsync             = fe.isAsync;
  ret->isGenerator         = fe.isGenerator;
  ret->isPairGenerator     = fe.isPairGenerator;
  ret->isMemoizeWrapper    = fe.isMemoizeWrapper;
  ret->isMemoizeWrapperLSB = fe.isMemoizeWrapperLSB;
  ret->isMemoizeImpl       = Func::isMemoizeImplName(fe.name);

  add_frame_variables(*ret, fe);

  if (!RuntimeOption::ConstantFunctions.empty()) {
    auto const name = [&] {
      if (!cls) return fe.name->toCppString();
      return folly::sformat("{}::{}", cls->name, ret->name);
    }();
    auto const it = RuntimeOption::ConstantFunctions.find(name);
    if (it != RuntimeOption::ConstantFunctions.end()) {
      ret->locals.resize(fe.params.size());
      ret->numIters = 0;
      ret->numClsRefSlots = 0;
      ret->staticLocals.clear();
      ret->attrs |= AttrIsFoldable;

      auto const mainEntry = BlockId{0};

      auto blk          = std::make_unique<php::Block>();
      blk->id           = mainEntry;
      blk->section      = php::Block::Section::Main;
      blk->exnNode      = nullptr;

      blk->hhbcs.push_back(gen_constant(it->second));
      blk->hhbcs.push_back(bc::RetC {});
      ret->blocks.push_back(std::move(blk));

      ret->dvEntries.resize(fe.params.size(), NoBlockId);
      ret->mainEntry = mainEntry;

      for (size_t i = 0, sz = fe.params.size(); i < sz; ++i) {
        if (fe.params[i].hasDefaultValue()) {
          ret->params[i].dvEntryPoint = mainEntry;
          ret->dvEntries[i] = mainEntry;
        }
      }
      return ret;
    }
  }

  /*
   * Builtin functions get some extra information.  The returnType flag is only
   * non-folly::none for these, but note that something may be a builtin and
   * still have a folly::none return type.
   */
  if (fe.isNative) {
    auto const f = [&] () -> HPHP::Func* {
      if (ret->cls) {
        auto const cls = Unit::lookupClass(ret->cls->name);
        return cls ? cls->lookupMethod(ret->name) : nullptr;
      } else {
        return Unit::lookupBuiltin(ret->name);
      }
    }();

    ret->nativeInfo                   = std::make_unique<php::NativeInfo>();
    ret->nativeInfo->returnType       = fe.hniReturnType;
    if (f && ret->params.size()) {
      for (auto i = 0; i < ret->params.size(); i++) {
        auto& pi = ret->params[i];
        if (pi.isVariadic || !f->params()[i].hasDefaultValue()) continue;
        if (pi.defaultValue.m_type == KindOfUninit &&
            pi.phpCode != nullptr) {
          auto res = eval_cell_value([&] {
              auto val = f_constant(StrNR(pi.phpCode));
              val.setEvalScalar();
              return *val.asTypedValue();
            });
          if (!res) {
            FTRACE(4, "Argument {} to {}: Failed to evaluate {}\n",
                   i, f->fullName(), pi.phpCode);
            continue;
          }
          pi.defaultValue = *res;
        }
      }
    }
    if (!f || !f->nativeFuncPtr() ||
        (f->userAttributes().count(
          LowStringPtr(s_attr_Deprecated.get())))) {
      ret->attrs |= AttrNoFCallBuiltin;
    }
  }

  build_cfg(puState, *ret, fe);

  return ret;
}

void parse_methods(ParseUnitState& puState,
                   php::Class* ret,
                   php::Unit* unit,
                   const PreClassEmitter& pce) {
  std::unique_ptr<php::Func> cinit;
  for (auto& me : pce.methods()) {
    auto f = parse_func(puState, unit, ret, *me);
    if (f->name == s_86cinit.get()) {
      puState.constPassFuncs[f.get()] |= php::Program::ForAnalyze;
      cinit = std::move(f);
    } else {
      if (f->name == s_86pinit.get() ||
          f->name == s_86sinit.get() ||
          f->name == s_86linit.get()) {
        puState.constPassFuncs[f.get()] |= php::Program::ForAnalyze;
      }
      ret->methods.push_back(std::move(f));
    }
  }
  if (cinit) ret->methods.push_back(std::move(cinit));
}

void add_stringish(php::Class* cls) {
  // The runtime adds Stringish to any class providing a __toString() function,
  // so we mirror that here to make sure analysis of interfaces is correct.
  // All Stringish are also XHPChild, so handle it here as well.
  if (cls->attrs & AttrInterface && cls->name->isame(s_Stringish.get())) {
    return;
  }

  bool hasXHP = false;
  for (auto& iface : cls->interfaceNames) {
    if (iface->isame(s_Stringish.get())) return;
    if (iface->isame(s_XHPChild.get())) { hasXHP = true; }
  }

  for (auto& func : cls->methods) {
    if (func->name->isame(s_toString.get())) {
      FTRACE(2, "Adding Stringish and XHPChild to {}\n", cls->name->data());
      cls->interfaceNames.push_back(s_Stringish.get());
      if (!hasXHP && !cls->name->isame(s_XHPChild.get())) {
        cls->interfaceNames.push_back(s_XHPChild.get());
      }
      return;
    }
  }
}

std::unique_ptr<php::Class> parse_class(ParseUnitState& puState,
                                        php::Unit* unit,
                                        const PreClassEmitter& pce) {
  FTRACE(2, "  class: {}\n", pce.name()->data());

  auto ret               = std::make_unique<php::Class>();
  ret->name              = pce.name();
  ret->srcInfo           = php::SrcInfo { pce.getLocation(),
                                          pce.docComment() };
  ret->unit              = unit;
  ret->closureContextCls = nullptr;
  ret->parentName        = pce.parentName()->empty() ? nullptr
                                                     : pce.parentName();
  ret->attrs             = static_cast<Attr>(pce.attrs() & ~AttrNoOverride);
  ret->hoistability      = pce.hoistability();
  ret->userAttributes    = pce.userAttributes();
  ret->id                = pce.id();

  for (auto& iface : pce.interfaces()) {
    ret->interfaceNames.push_back(iface);
  }

  copy(ret->usedTraitNames,  pce.usedTraits());
  copy(ret->traitPrecRules,  pce.traitPrecRules());
  copy(ret->traitAliasRules, pce.traitAliasRules());
  copy(ret->requirements,    pce.requirements());

  parse_methods(puState, ret.get(), unit, pce);
  add_stringish(ret.get());

  auto& propMap = pce.propMap();
  for (size_t idx = 0; idx < propMap.size(); ++idx) {
    auto& prop = propMap[idx];
    ret->properties.push_back(
      php::Prop {
        prop.name(),
        prop.attrs(),
        prop.userAttributes(),
        prop.docComment(),
        prop.userType(),
        prop.typeConstraint(),
        prop.val()
      }
    );
  }

  auto& constMap = pce.constMap();
  for (size_t idx = 0; idx < constMap.size(); ++idx) {
    auto& cconst = constMap[idx];
    ret->constants.push_back(
      php::Const {
        cconst.name(),
        ret.get(),
        cconst.valOption(),
        cconst.phpCode(),
        cconst.typeConstraint(),
        cconst.isTypeconst()
      }
    );
  }

  if (ret->attrs & AttrBuiltin) {
    if (auto nativeConsts = Native::getClassConstants(ret->name)) {
      for (auto const& cnsMap : *nativeConsts) {
        TypedValueAux tvaux;
        tvCopy(cnsMap.second, tvaux);
        tvaux.constModifiers() = { false, false };
        ret->constants.push_back(
          php::Const {
            cnsMap.first,
            ret.get(),
            tvaux,
            staticEmptyString(),
            staticEmptyString(),
            false
          }
        );
      }
    }
  }

  ret->enumBaseTy = pce.enumBaseTy();

  return ret;
}

//////////////////////////////////////////////////////////////////////

void assign_closure_context(const ParseUnitState&, php::Class*);

php::Class*
find_closure_context(const ParseUnitState& puState,
                     php::Func* createClFunc) {
  if (auto const cls = createClFunc->cls) {
    if (cls->parentName &&
        cls->parentName->isame(s_Closure.get())) {
      // We have a closure created by a closure's invoke method, which
      // means it should inherit the outer closure's context, so we
      // have to know that first.
      assign_closure_context(puState, cls);
      return cls->closureContextCls;
    }
    return cls;
  }
  return nullptr;
}

void assign_closure_context(const ParseUnitState& puState,
                            php::Class* clo) {
  if (clo->closureContextCls) return;

  auto clIt = puState.createClMap.find(clo->id);
  if (clIt == end(puState.createClMap)) {
    // Unused closure class.  Technically not prohibited by the spec.
    return;
  }

  /*
   * Any route to the closure context must yield the same class, or
   * things downstream won't understand.  We try every route and
   * assert they are all the same here.
   *
   * See bytecode.specification for CreateCl for the relevant
   * invariants.
   */
  always_assert(!clIt->second.empty());
  auto it = begin(clIt->second);
  auto const representative = find_closure_context(puState, *it);
  if (debug) {
    for (++it; it != end(clIt->second); ++it) {
      assert(find_closure_context(puState, *it) == representative);
    }
  }
  clo->closureContextCls = representative;
}

void find_additional_metadata(const ParseUnitState& puState,
                              php::Unit* unit) {
  for (auto& c : unit->classes) {
    if (!c->parentName || !c->parentName->isame(s_Closure.get())) {
      continue;
    }
    assign_closure_context(puState, c.get());
  }
}

//////////////////////////////////////////////////////////////////////

}

std::unique_ptr<php::Unit> parse_unit(php::Program& prog,
                                      std::unique_ptr<UnitEmitter> uep) {
  Trace::Bump bumper{Trace::hhbbc_parse, kSystemLibBump, uep->isASystemLib()};
  FTRACE(2, "parse_unit {}\n", uep->m_filepath->data());

  if (RuntimeOption::EvalAbortBuildOnVerifyError) {
    always_assert_flog(
      uep->check(false),
      "The unoptimized unit for {} did not pass verification, "
      "bailing because Eval.AbortBuildOnVerifyError is set",
      uep->m_filepath
    );
  }

  auto const& ue = *uep;

  auto ret      = std::make_unique<php::Unit>();
  ret->md5      = ue.md5();
  ret->filename = ue.m_filepath;
  ret->preloadPriority = ue.m_preloadPriority;
  ret->isHHFile = ue.m_isHHFile;
  ret->useStrictTypes = ue.m_useStrictTypes;
  ret->useStrictTypesForBuiltins = ue.m_useStrictTypesForBuiltins;
  ret->metaData = ue.m_metaData;

  ParseUnitState puState{ prog.nextFuncId };
  if (ue.hasSourceLocInfo()) {
    puState.srcLocInfo = ue.createSourceLocTable();
  } else {
    puState.srcLocInfo = ue.lineTable();
  }
  puState.defClsMap.resize(ue.numPreClasses(), nullptr);

  for (size_t i = 0; i < ue.numPreClasses(); ++i) {
    auto cls = parse_class(puState, ret.get(), *ue.pce(i));
    ret->classes.push_back(std::move(cls));
  }

  for (auto& fe : ue.fevec()) {
    auto func = parse_func(puState, ret.get(), nullptr, *fe);
    assert(!fe->pce());
    if (fe->isPseudoMain()) {
      ret->pseudomain = std::move(func);
    } else {
      ret->funcs.push_back(std::move(func));
    }
  }

  ret->srcLocs.resize(puState.srcLocs.size());
  for (auto& srcInfo : puState.srcLocs) {
    ret->srcLocs[srcInfo.second] = srcInfo.first;
  }

  for (auto& ta : ue.typeAliases()) {
    ret->typeAliases.push_back(
      std::make_unique<php::TypeAlias>(ta)
    );
  }

  ret->classAliases = std::move(puState.classAliases);

  find_additional_metadata(puState, ret.get());

  for (auto const item : puState.constPassFuncs) {
    auto encoded_val = reinterpret_cast<uintptr_t>(item.first) | item.second;
    record_const_init(prog, encoded_val);
  }

  assert(check(*ret));
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
