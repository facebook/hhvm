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
#include "hphp/hhbbc/dce.h"

#include <vector>
#include <string>
#include <utility>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <set>

#include <boost/dynamic_bitset.hpp>

#include "folly/gen/Base.h"
#include "folly/gen/String.h"

#include "hphp/util/trace.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"
#include "hphp/hhbbc/cfg.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc_dce);

//////////////////////////////////////////////////////////////////////

/*
 * This module contains code to perform both local DCE and global DCE.
 *
 * The local DCE algorithm addresses dead eval stack manipulations and
 * dead stores to locals that are visible within a single basic block.
 *
 * The global DCE performs a liveness analysis and then uses this
 * information to allow dead stores to locals to be eliminated across
 * blocks.  It does not attempt to remove unnecessary evaluation stack
 * manipulation spanning basic blocks, but it uses the same local DCE
 * code and will eliminate intra-block stack manipulations.
 *
 * Both types of DCE here need to be type-aware, but they must visit
 * blocks backward.  They accomplish this by forward-propagating the
 * block input states from a FuncAnalysis to each instruction in the
 * block prior to doing the backward iteration.
 *
 * Eval stack:
 *
 *   During backward traversal of a block, we maintain a "backwards"
 *   stack, indicating which eval stack slots are going to be required
 *   in the future or not.
 *
 *   During this traversal, each instruction that pops when going
 *   forward instead "pushes" information about whether that input
 *   will be required.  If it is not required, it also pushes an
 *   accumulating set of instruction ids that must be removed if the
 *   instruction which produces the stack slot is removed.  (All
 *   instructions in these sets must be removed if any are, in order
 *   to keep the stack depths correct.)
 *
 *   Similarly, each instruction that would push a value going forward
 *   instead "pops" the information about whether its stack output is
 *   going to be needed.  If not, the instruction can mark itself (and
 *   all downstream instructions that depended on it) as removable.
 *
 * Locals:
 *
 *   While a block is iterated backward, the set of live locals is
 *   tracked.  The initial state of this live set depends on whether
 *   we are performing global or local DCE, and in the local case
 *   includes all locals in the function.
 *
 *   When a local may be read, it is added to the live set.  When a
 *   local is definitely-written, it is removed from the set.
 *
 *   If a instruction may write to a local that is not live, it can be
 *   marked as removable if it is known to have no other side-effects.
 *   Currently this is only hooked up to SetL.
 *
 * Liveness analysis:
 *
 *   The global algorithm first performs a liveness analysis to
 *   propagate live out sets to each block.
 *
 *   This analysis is basically normal, but slightly modified from the
 *   usual in order to deal with the way exceptional control flow is
 *   represented in our CFG (with factored edges).
 *
 *   It essentially is the liveness analysis algorithm described at
 *   http://dl.acm.org/citation.cfm?id=316171, except that we don't
 *   need to track kill sets for each PEI because we don't have a
 *   means of determining which factored edges may be traversed by any
 *   given PEI.  (Maybe that may be more usual to have in the context
 *   of a language with declared exception clauses...)
 *
 *   Since we only deal with the most pessimistic exception case, this
 *   means for each block we just determine a gen and kill set, along
 *   with a subset of the latter set that is the locals that must be
 *   killed before any PEI.  (See killBeforePEI below.)
 *
 * Final note about types:
 *
 *   Global DCE can change the types of locals in a way that spans
 *   basic blocks.  For a simple example, take the following code:
 *
 *      // $foo :: Uninit here.
 *      $foo = 12;
 *      // ... code that breaks a block ...
 *      $foo = 100;
 *
 *   If the first store to $foo is removed, the type of $foo before
 *   the SetL in the later block is now Uninit, instead of Int.
 *
 *   This means that after calling global_dce on a function, the type
 *   information in the block input states in the associated
 *   FuncAnalysis can no longer be considered accurate.
 *
 *   Moreover, since global DCE makes use of type information to
 *   determine whether a store is dead, we need to be careful that
 *   this never changes whether the assumptions used to perform DCE
 *   were correct.
 *
 *   This is ok right now: DCE only uses the types to figure out which
 *   values can either have lifetimes extended or shortened without
 *   visible side-effects, or which values may be refs (so we can't
 *   omit stores to them).  If we omit a store that changes the type
 *   of a local globally, this means the new type cannot be different
 *   with regard to these features (or we wouldn't have omitted it),
 *   which means we won't have made different decisions elsewhere in
 *   the algorithm based on the new type.
 *
 *   Specifically: we will never omit a store where the old local type
 *   was something that could've had side effects, and if a new value
 *   is stored into a local where destroying it could have
 *   side-effects, some point along the path to the function exit (if
 *   nothing else, the RetC) will have added it to the gen set and the
 *   store also won't be removable.
 *
 *   In contrast, note that local DCE can not change types across
 *   block boundaries.
 *
 */

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_unreachable("static analysis error: supposedly "
                                 "unreachable code was reached");


//////////////////////////////////////////////////////////////////////

// Returns whether decrefing a type could run a destructor.
bool couldRunDestructor(const Type& t) {
  // We could check for specialized objects to see if they don't
  // declare a user-defined destructor, but currently don't.
  return t.couldBe(TObj) || t.couldBe(TCArr) || t.couldBe(TRef);
}

// Returns whether a set on something containing type t could have
// side-effects (running destuctors, or modifying arbitrary things via
// a Ref).
bool setCouldHaveSideEffects(const Type& t) {
  return t.couldBe(TObj) || t.couldBe(TCArr) || t.couldBe(TRef);
}

//////////////////////////////////////////////////////////////////////

enum class Use { Not, Used };
using InstrId    = size_t;
using InstrIdSet = std::set<InstrId>;
using UseInfo    = std::pair<Use,InstrIdSet>;

struct DceState {
  /*
   * Eval stack use information.  Stacks slots are marked as being
   * needed or not needed.  If they aren't needed, they carry a set of
   * instructions that must be removed if the instruction that
   * produces the stack value is also removable.
   */
  std::vector<UseInfo> stack;

  /*
   * Locals known to be live at a point in a DCE walk.  This is used
   * when we're actually acting on information we discovered during
   * liveness analysis.
   */
  std::bitset<kMaxTrackedLocals> liveLocals;

  /*
   * These variable sets are used to compute the transfer function for
   * the global liveness analysis in global_dce.
   *
   * The gen set accumulates the set of variables in the block with
   * upward-exposed uses.  The kill set is the set of variables the
   * block will re-define, ignoring exceptional control flow.
   *
   * The killBeforePEI set is the set of variables killed before a
   * PEI.  Propagation of liveness needs to use this (always more
   * conservative) set instead of kill when crossing a factored exit
   * edge.
   */
  std::bitset<kMaxTrackedLocals> gen;
  std::bitset<kMaxTrackedLocals> kill;
  std::bitset<kMaxTrackedLocals> killBeforePEI;

  /*
   * Instructions marked in this set are dead.  If any of them are
   * removed, however, they must all be removed, because of the need
   * to keep eval stack consumers and producers balanced.
   */
  boost::dynamic_bitset<> markedDead;
};

//////////////////////////////////////////////////////////////////////

const char* show(Use u) {
  switch (u) {
  case Use::Not:   return "0";
  case Use::Used:  return "U";
  }
  not_reached();
}

std::string show(const InstrIdSet& set) {
  using namespace folly::gen;
  return from(set)
    | eachTo<std::string>()
    | unsplit<std::string>(";")
    ;
}

std::string show(const UseInfo& ui) {
  return folly::format("{}@{}", show(ui.first), show(ui.second)).str();
}

std::string show(std::bitset<kMaxTrackedLocals> locs) {
  std::ostringstream out;
  out << locs;
  return out.str();
}

//////////////////////////////////////////////////////////////////////

struct DceVisitor : boost::static_visitor<void> {
  DceVisitor(DceState& dceState,
             InstrId id,
             const State& stateBefore,
             const StepFlags& flags)
    : m_dceState(dceState)
    , m_id(id)
    , m_stateBefore(stateBefore)
    , m_flags(flags)
  {}

  void operator()(const bc::PopC&)       { discardNonDtors(); }
  // For PopV and PopR currently we never know if can't run a
  // destructor.
  void operator()(const bc::PopA&)       { discard(); }
  void operator()(const bc::Int&)        { pushRemovable(); }
  void operator()(const bc::String&)     { pushRemovable(); }
  void operator()(const bc::Array&)      { pushRemovable(); }
  void operator()(const bc::Double&)     { pushRemovable(); }
  void operator()(const bc::True&)       { pushRemovable(); }
  void operator()(const bc::False&)      { pushRemovable(); }
  void operator()(const bc::Null&)       { pushRemovable(); }
  void operator()(const bc::NullUninit&) { pushRemovable(); }
  void operator()(const bc::File&)       { pushRemovable(); }
  void operator()(const bc::Dir&)        { pushRemovable(); }
  void operator()(const bc::NameA&)      { popCond(push()); }
  void operator()(const bc::NewArray&)   { pushRemovable(); }
  void operator()(const bc::NewCol&)     { pushRemovable(); }

  /*
   * Note that these instructions with popConds are relying on the
   * consumer of the values they push to check whether lifetime
   * changes can have side-effects.
   *
   * For example, in bytecode like this, assuming $x is an object with
   * a destructor:
   *
   *   CGetL $x
   *   UnsetL $x
   *   // ...
   *   PopC $x // dtor should be here.
   *
   * The PopC will decide it can't be eliminated, which prevents us
   * from eliminating the CGetL.
   */

  void operator()(const bc::Dup&) {
    auto const u1 = push();
    auto const u2 = push();
    popCond(u1, u2);
  }

  void operator()(const bc::CGetL& op) {
    addGen(op.loc1->id);
    pushRemovable();
  }

  void operator()(const bc::CGetL2& op) {
    addGen(op.loc1->id);
    auto const u1 = push();
    auto const u2 = push();
    popCond(u1, u2);
  }

  void operator()(const bc::CGetL3& op) {
    addGen(op.loc1->id);
    auto const u1 = push();
    auto const u2 = push();
    auto const u3 = push();
    popCond(u1, u2, u3);
    popCond(u1, u2, u3);
  }

  void operator()(const bc::RetC&)  {         pop(); readDtorLocs(); }
  void operator()(const bc::Throw&) {         pop(); readDtorLocs(); }
  void operator()(const bc::Fatal&) {         pop(); readDtorLocs(); }
  void operator()(const bc::Exit&)  { push(); pop(); readDtorLocs(); }

  void operator()(const bc::SetL& op) {
    auto const oldTy   = locRaw(op.loc1);
    auto const effects = setCouldHaveSideEffects(oldTy);
    if (!isLive(op.loc1->id) && !effects) return markDead();
    push();
    pop();
    if (!effects) addKill(op.loc1->id);
    if (effects)  addGen(op.loc1->id);
  }

  /*
   * Default implementation is conservative: assume we use all of our
   * inputs, and can't be removed even if our output is unused.
   *
   * We also assume all the locals in the mayReadLocalSet must be
   * added to the live local set, and don't remove anything from it.
   */
  template<class Op>
  void operator()(const Op& op) {
    addGenSet(m_flags.mayReadLocalSet);
    m_dceState.liveLocals |= m_flags.mayReadLocalSet;
    for (auto i = uint32_t{0}; i < op.numPush(); ++i) {
      push();
    }
    for (auto i = uint32_t{0}; i < op.numPop(); ++i) {
      pop(Use::Used, InstrIdSet{});
    }
  }

private: // eval stack
  void pop() { pop(Use::Used, InstrIdSet{}); }
  void pop(Use u, InstrIdSet set) {
    FTRACE(2, "      pop({})\n", show(u));
    m_dceState.stack.emplace_back(u, std::move(set));
  }

  void discard() {
    pop(Use::Not, InstrIdSet{m_id});
  }

  bool allUnused() { return true; }
  template<class... Args>
  bool allUnused(const UseInfo& ui, Args&&... args) {
    return ui.first == Use::Not &&
      allUnused(std::forward<Args>(args)...);
  }

  void combineSets(InstrIdSet&) {}
  template<class... Args>
  void combineSets(InstrIdSet& accum, const UseInfo& ui, Args&&... args) {
    accum.insert(begin(ui.second), end(ui.second));
    combineSets(accum, std::forward<Args>(args)...);
  }

  // If all the supplied UseInfos represent unused stack slots, make a
  // pop that is considered unused.  Otherwise pop as a Use::Used.
  template<class... Args>
  void popCond(Args&&... args) {
    bool unused = allUnused(std::forward<Args>(args)...);
    if (!unused) return pop(Use::Used, InstrIdSet{});
    auto accum = InstrIdSet{m_id};
    combineSets(accum, std::forward<Args>(args)...);
    pop(Use::Not, accum);
  }

  /*
   * It may be ok to remove pops on objects with destructors in some
   * scenarios (where it won't change the observable point at which a
   * destructor runs).  We could also look at the object type and see
   * if it is known that it can't have a user-defined destructor.
   *
   * For now we're not trying though, since at the time this was
   * tested there were only two additional "dead" PopC's in all of www
   * if you remove the couldBe checks below.
   */
  void discardNonDtors() {
    auto const t = topC();
    if (couldRunDestructor(t)) {
      return pop(Use::Used, InstrIdSet{});
    }
    discard();
  }

  UseInfo push() {
    always_assert(!m_dceState.stack.empty());
    auto ret = m_dceState.stack.back();
    m_dceState.stack.pop_back();
    FTRACE(2, "      {}@{} = push()\n", show(ret.first), show(ret.second));
    return ret;
  }

  void pushRemovable() {
    auto const ui = push();
    switch (ui.first) {
    case Use::Not:
      markSetDead(ui.second);
      break;
    case Use::Used:
      break;
    }
  }

  Type topT(uint32_t idx = 0) {
    assert(idx < m_stateBefore.stack.size());
    return m_stateBefore.stack[m_stateBefore.stack.size() - idx - 1];
  }

  Type topC(uint32_t idx = 0) {
    auto const t = topT(idx);
    assert(t.subtypeOf(TInitCell));
    return t;
  }

private: // locals
  void addGenSet(std::bitset<kMaxTrackedLocals> locs) {
    FTRACE(4, "      conservative: {}\n", show(locs));
    m_dceState.liveLocals |= locs;
    m_dceState.gen |= locs;
    m_dceState.kill &= ~locs;
    m_dceState.killBeforePEI &= ~locs;
  }

  void addGen(uint32_t id) {
    FTRACE(2, "      gen: {}\n", id);
    if (id >= kMaxTrackedLocals) return;
    m_dceState.liveLocals[id] = 1;
    m_dceState.gen[id] = 1;
    m_dceState.kill[id] = 0;
    m_dceState.killBeforePEI[id] = 0;
  }

  void addKill(uint32_t id) {
    FTRACE(2, "     kill: {}\n", id);
    if (id >= kMaxTrackedLocals) return;
    m_dceState.liveLocals[id] = 0;
    m_dceState.gen[id] = 0;
    m_dceState.kill[id] = 1;
    m_dceState.killBeforePEI[id] = 1;
  }

  bool isLive(uint32_t id) {
    if (id >= kMaxTrackedLocals) {
      // Conservatively assume it's potentially live.
      return true;
    }
    return m_dceState.liveLocals[id];
  }

  Type locRaw(borrowed_ptr<php::Local> loc) {
    return m_stateBefore.locals[loc->id];
  }

  void readDtorLocs() {
    for (auto i = size_t{0}; i < m_stateBefore.locals.size(); ++i) {
      if (couldRunDestructor(m_stateBefore.locals[i])) {
        addGen(i);
      }
    }
  }

private:
  void markSetDead(const InstrIdSet& set) {
    m_dceState.markedDead[m_id] = 1;
    FTRACE(2, "    marking {} {}\n", m_id, show(set));
    for (auto& i : set) m_dceState.markedDead[i] = 1;
  }

  void markDead() {
    m_dceState.markedDead[m_id] = 1;
    FTRACE(2, "    marking {}\n", m_id);
  }

private:
  DceState& m_dceState;
  InstrId m_id;
  const State& m_stateBefore;
  const StepFlags& m_flags;
};

//////////////////////////////////////////////////////////////////////

folly::Optional<DceState>
dce_visit(const Index& index,
          Context const ctx,
          borrowed_ptr<const php::Block> const blk,
          const State& stateIn,
          std::bitset<kMaxTrackedLocals> liveOut,
          std::bitset<kMaxTrackedLocals> liveOutExn) {
  if (!stateIn.initialized) {
    /*
     * Skip unreachable blocks.
     *
     * For DCE analysis it is ok to assume the transfer function is
     * the identity on unreachable blocks (i.e. gen and kill sets are
     * empty).  For optimize, we don't need to bother doing anything
     * to these---another pass is responsible for removing completely
     * unreachable blocks.
     */
    return folly::none;
  }

  auto const states = locally_propagated_states(index, ctx, blk, stateIn);

  auto dceState = DceState{};
  dceState.markedDead.resize(blk->hhbcs.size());
  dceState.liveLocals = liveOut;
  dceState.stack.resize(states.back().first.stack.size());
  for (auto& s : dceState.stack) {
    s = UseInfo { Use::Used, InstrIdSet{} };
  }

  for (auto idx = blk->hhbcs.size(); idx-- > 0;) {
    auto const& op = blk->hhbcs[idx];

    FTRACE(2, "  == #{} {}\n", idx, show(op));

    auto visitor = DceVisitor {
      dceState,
      idx,
      states[idx].first,
      states[idx].second
    };
    visit(op, visitor);

    /*
     * When we see a PEI, we need to start over on the killBeforePEI
     * set, and the local-liveness must take into account the fact
     * that we could take an exception edge here (or'ing in the
     * liveOutExn set).
     */
    if (states[idx].second.wasPEI) {
      FTRACE(2, "    <-- exceptions\n");
      dceState.liveLocals |= liveOutExn;
      dceState.killBeforePEI.reset();
    }

    FTRACE(4, "    dce stack: {}\n",
      [&] {
        using namespace folly::gen;
        return from(dceState.stack)
          | map([&] (const UseInfo& ui) { return show(ui); })
          | unsplit<std::string>(" ");
      }()
    );
    FTRACE(4, "    interp stack: {}\n",
      [&] {
        using namespace folly::gen;
        return from(states[idx].first.stack)
          | map([&] (const Type& t) { return show(t); })
          | unsplit<std::string>(" ");
      }()
    );

    // We're now at the state before this instruction, so the stack
    // sizes must line up.
    assert(dceState.stack.size() == states[idx].first.stack.size());
  }

  return dceState;
}

struct DceAnalysis {
  std::bitset<kMaxTrackedLocals> gen;
  std::bitset<kMaxTrackedLocals> kill;
  std::bitset<kMaxTrackedLocals> killExn;
};

DceAnalysis analyze_dce(const Index& index,
                        Context const ctx,
                        borrowed_ptr<php::Block> const blk,
                        const State& stateIn) {
  auto allLive = std::bitset<kMaxTrackedLocals>{};
  allLive.set();
  if (auto dceState = dce_visit(index, ctx, blk, stateIn, allLive, allLive)) {
    return DceAnalysis {
      dceState->gen,
      dceState->kill,
      dceState->killBeforePEI
    };
  }
  return DceAnalysis {};
}

void optimize_dce(const Index& index,
                  Context const ctx,
                  borrowed_ptr<php::Block> const blk,
                  const State& stateIn,
                  std::bitset<kMaxTrackedLocals> liveOut,
                  std::bitset<kMaxTrackedLocals> liveOutExn) {
  auto const dceState = dce_visit(index, ctx, blk,
    stateIn, liveOut, liveOutExn);
  if (!dceState) return;

  // Remove all instructions that were marked dead, and replace
  // instructions that can be replaced with pops but aren't dead.
  for (auto idx = blk->hhbcs.size(); idx-- > 0;) {
    if (!dceState->markedDead.test(idx)) continue;
    blk->hhbcs.erase(begin(blk->hhbcs) + idx);
  }

  // Blocks must be non-empty.  Make sure we don't change that.
  if (blk->hhbcs.empty()) {
    blk->hhbcs.push_back(bc::Nop {});
  }
}

//////////////////////////////////////////////////////////////////////

}

void local_dce(const Index& index,
               Context const ctx,
               borrowed_ptr<php::Block> const blk,
               const State& stateIn) {
  Trace::Bump bumper{Trace::hhbbc_dce, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};

  // For local DCE, we have to assume all variables are in the
  // live-out set for the block.
  auto allLive = std::bitset<kMaxTrackedLocals>();
  allLive.set();
  optimize_dce(index, ctx, blk, stateIn, allLive, allLive);
}

//////////////////////////////////////////////////////////////////////

void global_dce(const Index& index, const FuncAnalysis& ai) {
  Trace::Bump bumper{Trace::hhbbc_dce, kSystemLibBump,
    is_systemlib_part(*ai.ctx.unit)};

  auto rpoId = [&] (borrowed_ptr<php::Block> blk) {
    return ai.bdata[blk->id].rpoId;
  };

  FTRACE(1, "|---- global DCE analyze ({})\n", show(ai.ctx));

  /*
   * Create a DceAnalysis for each block, indexed by rpo id.
   *
   * Here we want to pre-compute the transfer function for each block,
   * so we don't need to visit each instruction repeatedly during the
   * fixed point computation.  The transfer function is a function of
   * the set of locals read and killed in the block, and does not
   * depend on the final live out state, so we can compute it here.
   */
  auto blockAnalysis = std::vector<DceAnalysis>{};
  for (auto& b : ai.rpoBlocks) {
    FTRACE(2, "block #{}\n", b->id);
    auto const dinfo = analyze_dce(
      index,
      ai.ctx,
      b,
      ai.bdata[b->id].stateIn
    );
    blockAnalysis.push_back(dinfo);
  }

  /*
   * States for each block, indexed by RPO id.
   *
   * The liveOut state is the union of liveIn states of each normal
   * successor, and liveOutExn is the union of liveIn states of each
   * exceptional successor.
   */
  struct BlockState {
    std::bitset<kMaxTrackedLocals> liveOut;
    std::bitset<kMaxTrackedLocals> liveOutExn;
  };
  std::vector<BlockState> blockStates(ai.rpoBlocks.size());

  /*
   * Set of block reverse post order ids that still need to be
   * visited.  This is ordered by std::greater, so we'll generally
   * visit blocks before their predecessors.  (The algorithm doesn't
   * need this for correctness, but it might make it iterate less.)
   *
   * Every block must be visited at least once, so we throw them all
   * in to start.
   */
  std::set<uint32_t,std::greater<uint32_t>> incompleteQ;
  for (auto& b : ai.rpoBlocks) {
    incompleteQ.insert(rpoId(b));
  }

  auto const normalPreds   = computeNormalPreds(ai.rpoBlocks);
  auto const factoredPreds = computeFactoredPreds(ai.rpoBlocks);

  /*
   * Iterate on live out states until we reach a fixed point.
   *
   * This algorithm treats the exceptional live-out states differently
   * from the live-out states during normal control flow.  The
   * liveOutExn sets only take part in the liveIn computation when the
   * block has factored exits.
   */
  while (!incompleteQ.empty()) {
    auto const blk = ai.rpoBlocks[*begin(incompleteQ)];
    incompleteQ.erase(begin(incompleteQ));

    FTRACE(2, "block #{}\n", blk->id);

    auto const liveOut    = blockStates[rpoId(blk)].liveOut;
    auto const liveOutExn = blockStates[rpoId(blk)].liveOutExn;
    auto const transfer   = blockAnalysis[rpoId(blk)];
    auto const liveIn     = transfer.gen | (liveOut & ~transfer.kill)
                                         | (liveOutExn & ~transfer.killExn);

    FTRACE(2, "live out : {}\n"
              "out exn  : {}\n"
              "gen      : {}\n"
              "kill     : {}\n"
              "kill exn : {}\n"
              "live in  : {}\n",
              show(liveOut),
              show(liveOutExn),
              show(transfer.gen),
              show(transfer.kill),
              show(transfer.killExn),
              show(liveIn));

    // Merge the liveIn into the liveOut of each normal predecessor.
    // If the set changes, reschedule that predecessor.
    for (auto& pred : normalPreds[blk->id]) {
      FTRACE(2, "  -> {}\n", pred->id);
      auto& predState = blockStates[rpoId(pred)].liveOut;
      auto const oldPredState = predState;
      predState |= liveIn;
      if (predState != oldPredState) {
        incompleteQ.insert(rpoId(pred));
      }
    }

    // Merge the liveIn into the liveOutExn state for each exceptional
    // precessor.  The liveIn computation also depends on the
    // liveOutExn state, so again reschedule if it changes.
    for (auto& pred : factoredPreds[blk->id]) {
      FTRACE(2, "  => {}\n", pred->id);
      auto& predState = blockStates[rpoId(pred)].liveOutExn;
      auto const oldPredState = predState;
      predState |= liveIn;
      if (predState != oldPredState) {
        incompleteQ.insert(rpoId(pred));
      }
    }
  }

  /*
   * Now that we're at a fixed point, use the propagated states to
   * remove instructions that don't need to be there.
   */
  FTRACE(1, "|---- global DCE optimize ({})\n", show(ai.ctx));
  for (auto& b : ai.rpoBlocks) {
    FTRACE(2, "block #{}\n", b->id);
    optimize_dce(
      index,
      ai.ctx,
      b,
      ai.bdata[b->id].stateIn,
      blockStates[rpoId(b)].liveOut,
      blockStates[rpoId(b)].liveOutExn
    );
  }
}

//////////////////////////////////////////////////////////////////////

void remove_unreachable_blocks(const Index& index, const FuncAnalysis& ainfo) {
  boost::dynamic_bitset<> reachable(ainfo.ctx.func->nextBlockId);
  for (auto& blk : ainfo.rpoBlocks) {
    reachable[blk->id] = ainfo.bdata[blk->id].stateIn.initialized;
    if (reachable[blk->id]) continue;
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = nullptr;
  }

  if (!options.RemoveDeadBlocks) return;

  for (auto& blk : ainfo.rpoBlocks) {
    auto reachableTargets = false;
    forEachTakenEdge(blk->hhbcs.back(), [&] (php::Block& target) {
      if (reachable[target.id]) reachableTargets = true;
    });
    if (reachableTargets) continue;
    switch (blk->hhbcs.back().op) {
    case Op::JmpNZ:
    case Op::JmpZ:
      blk->hhbcs.back() = bc_with_loc(blk->hhbcs.back().srcLoc, bc::PopC {});
      break;
    default:
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
