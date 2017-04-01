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
#include "hphp/hhbbc/dce.h"

#include <vector>
#include <string>
#include <utility>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <set>

#include <boost/dynamic_bitset.hpp>
#include <boost/container/flat_map.hpp>

#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/util/bitops.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/trace.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

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
 * blocks.
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
 *   In addition, by killing dead eval stack slots across basic
 *   blocks, the saved entry stacks in the FuncAnalysis no longer
 *   match the actual stack depths.
 *
 *   This means that after calling global_dce on a function, its
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

// Returns whether decrefing a type could run a destructor.
bool couldRunDestructor(const Type& t) {
  // We could check for specialized objects to see if they don't
  // declare a user-defined destructor, but currently don't.
  return
    t.couldBe(TObj) ||
    t.couldBe(TRef) ||
    t.couldBe(TCArrN) ||
    t.couldBe(TCVecN) ||
    t.couldBe(TCDictN);
}

// Returns whether a set on something containing type t could have
// side-effects (running destuctors, or modifying arbitrary things via
// a Ref).
bool setCouldHaveSideEffects(const Type& t) {
  return
    t.couldBe(TObj) ||
    t.couldBe(TRef) ||
    t.couldBe(TCArrN) ||
    t.couldBe(TCVecN) ||
    t.couldBe(TCDictN);
}

// Some reads could raise warnings and run arbitrary code.
bool readCouldHaveSideEffects(const Type& t) {
  return t.couldBe(TUninit);
}

//////////////////////////////////////////////////////////////////////

/*
 * Use information of a stack cell.
 */
enum class Use {
  // Indicates that the cell is (unconditionally) not used.
  Not,

  // Indicates that the cell is (possibly) used.
  Used,

  /*
   * Indicates that the cell is only used if it was the last reference alive.
   * For instance, a PopC will call the destructor of the top-of-stack object
   * if it was the last reference alive, and this counts as an example of
   * 'UsedIfLastRef'.
   *
   * If the producer of the cell knows that it is not the last reference, then
   * it can treat Use::UsedIfLastRef as being equivalent to Use::Not.
   */
  UsedIfLastRef,
};

struct InstrId {
  BlockId blk;
  uint32_t idx;
};

bool operator<(const InstrId& i1, const InstrId& i2) {
  if (i1.blk != i2.blk) return i1.blk < i2.blk;
  // sort by decreasing idx so that kill_marked_instrs works
  return i1.idx > i2.idx;
}

bool operator==(const InstrId& i1, const InstrId& i2) {
  return i1.blk == i2.blk && i1.idx == i2.idx;
}

struct LocationId {
  BlockId  blk;
  uint32_t id;
  bool     isSlot;
};

bool operator<(const LocationId& i1, const LocationId& i2) {
  if (i1.blk != i2.blk) return i1.blk < i2.blk;
  if (i1.id != i2.id)   return i1.id < i2.id;
  return i2.isSlot && !i1.isSlot;
}

enum class DceAction {
  Kill,
  PopOutputs
};

const char* show(DceAction action) {
  switch (action) {
    case DceAction::Kill: return "Kill";
    case DceAction::PopOutputs: return "PopOutputs";
  };
  not_reached();
}

using DceActionMap = std::map<InstrId, DceAction>;

struct UseInfo {
  explicit UseInfo(Use u) : usage(u) {}
  UseInfo(Use u, DceActionMap&& ids) : usage(u), actions(std::move(ids)) {}

  Use usage;
  /*
   * Set of instructions that should be removed if we decide to
   * discard the corresponding stack slot.
   */
  DceActionMap actions;
  /*
   * Used for stack slots that are live across blocks to indicate a
   * stack slot that should be considered Used at the entry to blk.
   *
   * If its not live across blocks, location.blk will stay NoBlockId.
   */
  LocationId location { NoBlockId, 0, false };
};

std::string show(InstrId id) {
  return folly::sformat("{}:{}", id.blk, id.idx);
}

//////////////////////////////////////////////////////////////////////

struct DceState {
  explicit DceState(const Index& index, const FuncAnalysis& ainfo) :
      index(index), ainfo(ainfo) {}
  const Index& index;
  const FuncAnalysis& ainfo;
  /*
   * Used to accumulate a set of blk/stack-slot pairs that
   * should be marked used
   */
  std::set<LocationId> forcedLiveLocations;

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
   * Class-ref slots known to be live at a point in a DCE walk.  This is used
   * when we're actually acting on information we discovered during liveness
   * analysis.
   */
  std::bitset<kMaxTrackedClsRefSlots> liveSlots;

  /*
   * Instructions marked in this set will be processed by dce_perform.
   * They must all be processed together to keep eval stack consumers
   * and producers balanced.
   */
  DceActionMap actionMap;

  /*
   * The set of locals and class-ref slots that were ever live in this block.
   * (This includes ones that were live going out of this block.)  This set is
   * used by global DCE to remove locals and class-ref slots that are completely
   * unused in the entire function.
   */
  std::bitset<kMaxTrackedLocals> usedLocals;
  std::bitset<kMaxTrackedClsRefSlots> usedSlots;

  /*
   * Mapping of class-ref slots to their usage. If the currently live usage
   * of the slot is removed, the corresponding actions must be taken.
   */
  std::vector<UseInfo> slotUsage;
};

//////////////////////////////////////////////////////////////////////

const char* show(Use u) {
  switch (u) {
  case Use::Not:              return "0";
  case Use::Used:             return "U";
  case Use::UsedIfLastRef:    return "UL";
  }
  not_reached();
}

std::string show(const DceActionMap::value_type& elm) {
  return folly::sformat("{}@{}", show(elm.first), show(elm.second));
}

std::string show(const DceActionMap& actions) {
  using namespace folly::gen;
  return from(actions)
    | map([](const DceActionMap::value_type& elm) { return show(elm); })
    | unsplit<std::string>(";")
    ;
}

std::string DEBUG_ONLY show(const UseInfo& ui) {
  return folly::sformat("{}@{}", show(ui.usage), show(ui.actions));
}

std::string DEBUG_ONLY loc_bits_string(borrowed_ptr<const php::Func> func,
                                       std::bitset<kMaxTrackedLocals> locs) {
  std::ostringstream out;
  if (func->locals.size() < kMaxTrackedLocals) {
    for (auto i = func->locals.size(); i-- > 0;) {
      out << (locs.test(i) ? '1' : '0');
    }
  } else {
    out << locs;
  }
  return out.str();
}

std::string DEBUG_ONLY
slot_bits_string(borrowed_ptr<const php::Func> func,
                 std::bitset<kMaxTrackedClsRefSlots> slots) {
  std::ostringstream out;
  if (func->numClsRefSlots < kMaxTrackedClsRefSlots) {
    for (auto i = 0; i < func->numClsRefSlots; ++i) {
      out << (slots.test(i) ? '1' : '0');
    }
  } else {
    out << slots;
  }
  return out.str();
}

//////////////////////////////////////////////////////////////////////

struct Env {
  DceState& dceState;
  const Bytecode& op;
  InstrId id;
  const State& stateBefore;
  const StepFlags& flags;
};

//////////////////////////////////////////////////////////////////////

void markSetDead(Env& env, const DceActionMap& actions) {
  env.dceState.actionMap.emplace(env.id, DceAction::Kill);
  FTRACE(2, "     marking {} {}\n", show(env.id), show(actions));
  for (auto& i : actions) env.dceState.actionMap.insert(i);
}

void markUisDead(Env& env, const UseInfo& ui) {
  markSetDead(env, ui.actions);
}

template<typename... Args>
void markUisDead(Env& env, const UseInfo& ui, Args&&... args) {
  markUisDead(env, ui);
  markUisDead(env, std::forward<Args>(args)...);
}

/*
 * During global dce, a particular stack element could be pushed
 * on multiple paths. eg:
 *
 *   $a ? f() : 42
 *
 * If f() is known to return a non-counted type, we have UnboxRNop ->
 * PopC on one path, and Int 42 -> PopC on another, and the PopC marks
 * its value Use::Not. When we get to the Int 42 it thinks both
 * instructions can be killed; but when we get to the UnboxRNop it
 * does nothing. So any time we decide to ignore a Use::Not or
 * Use::UsedIfLastRef, we have to record that fact so we can prevent
 * the other paths from trying to use that information. We communicate
 * this via the ui.location field, and the forcedLiveLocations set.
 *
 * Note that a more sophisticated system would insert a PopC after the
 * UnboxRNop, and allow the 42/PopC to be removed (work in progress).
 */
void markUisLive(Env& env, const UseInfo& ui) {
  if (ui.usage != Use::Used && ui.location.blk != NoBlockId) {
    env.dceState.forcedLiveLocations.insert(ui.location);
  }
}

template<typename... Args>
void markUisLive(Env& env, const UseInfo& ui, Args&&... args) {
  markUisLive(env, ui);
  markUisLive(env, std::forward<Args>(args)...);
}

void markDead(Env& env) {
  env.dceState.actionMap.emplace(env.id, DceAction::Kill);
  FTRACE(2, "     marking {}\n", show(env.id));
}

//////////////////////////////////////////////////////////////////////
// eval stack

void pop(Env& env, UseInfo&& ui) {
  FTRACE(2, "      pop({})\n", show(ui.usage));
  env.dceState.stack.push_back(std::move(ui));
}

void pop(Env& env, Use u, DceActionMap actions) {
  pop(env, {u, std::move(actions)});
}

void pop(Env& env, Use u, InstrId id) {
  pop(env, {u, {{id, DceAction::Kill}}});
}

void pop(Env& env) { pop(env, Use::Used, DceActionMap{}); }

Type topT(Env& env, uint32_t idx = 0) {
  assert(idx < env.stateBefore.stack.size());
  return env.stateBefore.stack[env.stateBefore.stack.size() - idx - 1].type;
}

Type topC(Env& env, uint32_t idx = 0) {
  auto const t = topT(env, idx);
  assert(t.subtypeOf(TInitCell));
  return t;
}

void discard(Env& env) {
  pop(env, Use::Not, env.id);
}

bool allUnused() { return true; }
template<class... Args>
bool allUnused(const UseInfo& ui, Args&&... args) {
  return ui.usage == Use::Not &&
    allUnused(std::forward<Args>(args)...);
}

bool allUnusedIfNotLastRef() { return true; }
template<class... Args>
bool allUnusedIfNotLastRef(const UseInfo& ui, Args&&... args) {
  return (ui.usage == Use::Not || ui.usage == Use::UsedIfLastRef) &&
    allUnusedIfNotLastRef(std::forward<Args>(args)...);
}

void combineSets(UseInfo&) {}
template<class... Args>
void combineSets(UseInfo& accum, const UseInfo& ui, Args&&... args) {
  accum.actions.insert(begin(ui.actions), end(ui.actions));
  if (accum.location.blk == NoBlockId ||
      accum.location < ui.location) {
    accum.location = ui.location;
  }
  combineSets(accum, std::forward<Args>(args)...);
}

// The instruction wants to be killed, as long as its inputs can be
// killed; combine the UseInfo sets from its outputs, and make an
// appropriate number of Use::Not pops.
template<class... Args>
void passThrough(Env& env, Args&&... args) {
  UseInfo ui { Use::Not, {{ env.id, DceAction::Kill }} };
  combineSets(ui, std::forward<Args>(args)...);

  for (auto i = 1; i < env.op.numPop(); ++i) {
    pop(env, UseInfo{ui});
  }
  pop(env, std::move(ui));
}

bool popCouldRunDestructor(Env& env) {
  // If there's an equivLocal, we know that it's not the last
  // reference, so popping the stack won't run any destructors.
  return
    env.stateBefore.stack.back().equivLocal == NoLocalId &&
    couldRunDestructor(topC(env));
}

/*
 * It may be ok to remove pops on objects with destructors in some scenarios
 * (where it won't change the observable point at which a destructor runs).  We
 * could also look at the object type and see if it is known that it can't have
 * a user-defined destructor.
 *
 * For now, we mark the cell popped with a Use::UsedIfLastRef. This indicates
 * to the producer of the cell that the it is considered used if it could be
 * the last reference alive (in which case the destructor would be run on
 * Pop). If the producer knows that the cell is not the last reference (e.g. if
 * it is a Dup), then Use:UsedIfLastRef is equivalent to Use::Not.
 */
void discardNonDtors(Env& env) {
  if (popCouldRunDestructor(env)) {
    return pop(env, Use::UsedIfLastRef, env.id);
  }
  discard(env);
}

enum class PushFlags {
  MarkLive,
  MarkDead,
  PassThrough,
  PopOutputs
};

bool alwaysPop(const UseInfo& ui) {
  return
    ui.location.blk != NoBlockId ||
    ui.actions.size() > 1;
}

template<typename... Args>
bool alwaysPop(const UseInfo& ui, Args&&... args) {
  return alwaysPop(ui) || alwaysPop(std::forward<Args>(args)...);
}

bool maybePop(Env& env, const UseInfo& ui) {
  return
    (ui.actions.size() == 1 &&
     ui.actions.begin()->first.idx > env.id.idx + 1);
}

template<typename... Args>
bool maybePop(Env& env, const UseInfo& ui, Args&&... args) {
  return maybePop(env, ui) || maybePop(env, std::forward<Args>(args)...);
}

template<typename... Args>
bool shouldPopOutputs(Env& env, Args&&... args) {
  if (alwaysPop(std::forward<Args>(args)...)) return true;
  return maybePop(env, std::forward<Args>(args)...);
}

template<typename... Args>
void popOutputs(Env& env, Args&&... args) {
  if (shouldPopOutputs(env, std::forward<Args>(args)...)) {
    markUisDead(env, std::forward<Args>(args)...);
    env.dceState.actionMap[env.id] = DceAction::PopOutputs;
    return;
  }
  markUisLive(env, std::forward<Args>(args)...);
}

template<typename... Args>
void handle_push(Env& env, PushFlags pf, Args&&... uis) {
  switch (pf) {
    case PushFlags::MarkLive:
      markUisLive(env, std::forward<Args>(uis)...);
      return;
    case PushFlags::MarkDead:
      markUisDead(env, std::forward<Args>(uis)...);
      return;
    case PushFlags::PassThrough:
      passThrough(env, std::move(uis)...);
      return;
    case PushFlags::PopOutputs:
      popOutputs(env, std::forward<Args>(uis)...);
      return;
  }
}

template<typename F>
void push(Env& env, F fun) {
  always_assert(!env.dceState.stack.empty());
  auto ui = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  FTRACE(2, "      {}@{} = push()\n", show(ui.usage), show(ui.actions));
  auto const f = fun(ui);
  handle_push(env, f, std::move(ui));
}

void push(Env& env) {
  push(env, [](const UseInfo&) { return PushFlags::MarkLive; });
}

template<typename F>
void push2(Env& env, F fun) {
  always_assert(env.dceState.stack.size() >= 2);
  auto u1 = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  FTRACE(2, "      {}@{} = push()\n", show(u1.usage), show(u1.actions));
  auto u2 = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  FTRACE(2, "      {}@{} = push()\n", show(u1.usage), show(u2.actions));
  auto const f = fun(u1, u2);
  handle_push(env, f, std::move(u1), std::move(u2));
}

void pushRemovable(Env& env) {
  push(env,[] (const UseInfo& ui) {
      return ui.usage == Use::Not ? PushFlags::MarkDead : PushFlags::MarkLive;
    });
}

//////////////////////////////////////////////////////////////////////
// locals

void addLocGenSet(Env& env, std::bitset<kMaxTrackedLocals> locs) {
  FTRACE(4, "      loc-conservative: {}\n",
         loc_bits_string(env.dceState.ainfo.ctx.func, locs));
  env.dceState.liveLocals |= locs;
}

void addLocGen(Env& env, uint32_t id) {
  FTRACE(2, "      loc-gen: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 1;
}

void addLocKill(Env& env, uint32_t id) {
  FTRACE(2, "     loc-kill: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 0;
}

bool isLocLive(Env& env, uint32_t id) {
  if (id >= kMaxTrackedLocals) {
    // Conservatively assume it's potentially live.
    return true;
  }
  return env.dceState.liveLocals[id];
}

Type locRaw(Env& env, LocalId loc) {
  return env.stateBefore.locals[loc];
}

bool setLocCouldHaveSideEffects(Env& env, LocalId loc, bool forExit = false) {
  // Normally, if there's an equivLocal this isn't the last reference,
  // so overwriting it won't run any destructors. But if we're
  // destroying all the locals (eg RetC) they can't all protect each
  // other; in that case we require a lower equivLoc to ensure that
  // one local from each equivalence set will still be marked as
  // having effects (choosing lower numbers also means we mark the
  // params as live, which makes it more likely that we can eliminate
  // a local).
  static_assert(NoLocalId == std::numeric_limits<LocalId>::max(),
                "NoLocalId must be greater than all valid local ids");
  if (env.stateBefore.equivLocals.size() > loc) {
    auto l = env.stateBefore.equivLocals[loc];
    if (l != NoLocalId) {
      if (!forExit) return false;
      do {
        if (l < loc) return false;
        l = env.stateBefore.equivLocals[l];
      } while (l != loc);
    }
  }

  // If this local is bound to a static, its type will be TRef, but we
  // may know the type of the static - but statics *are* live out of
  // the function, so don't do this when forExit is set.
  if (!forExit &&
      env.stateBefore.localStaticBindings.size() > loc &&
      env.stateBefore.localStaticBindings[loc] == LocalStaticBinding::Bound &&
      !setCouldHaveSideEffects(env.dceState.ainfo.localStaticTypes[loc])) {
    return false;
  }

  return setCouldHaveSideEffects(locRaw(env, loc));
}

void readDtorLocs(Env& env) {
  for (auto i = LocalId{0}; i < env.stateBefore.locals.size(); ++i) {
    if (setLocCouldHaveSideEffects(env, i, true)) {
      addLocGen(env, i);
    }
  }
}

//////////////////////////////////////////////////////////////////////
// class-ref slots

void readSlot(Env& env, uint32_t id) {
  FTRACE(2, "     read-slot: {}\n", id);
  if (id >= kMaxTrackedClsRefSlots) return;
  env.dceState.liveSlots[id] = 1;
  env.dceState.slotUsage[id] = UseInfo { Use::Used };
}

// Read a slot, but in a usage that is discardable. If this read actually is
// discarded, then also discard the given instructions.
void readSlotDiscardable(Env& env, uint32_t id, DceActionMap actions) {
  FTRACE(2, "     read-slot (discardable): {}\n", id);
  if (id >= kMaxTrackedClsRefSlots) return;
  env.dceState.liveSlots[id] = 0;
  actions.emplace(env.id, DceAction::Kill);
  env.dceState.slotUsage[id] = { Use::Not, std::move(actions) };
}

void writeSlot(Env& env, uint32_t id) {
  FTRACE(2, "     write-slot: {}\n", id);
  if (id >= kMaxTrackedClsRefSlots) return;
  env.dceState.liveSlots[id] = 0;
  env.dceState.usedSlots[id] = 1;
  auto& ui = env.dceState.slotUsage[id];
  if (ui.usage != Use::Used &&
      ui.actions.size() &&
      ui.location.blk != NoBlockId) {
    env.dceState.forcedLiveLocations.insert(ui.location);
  }
  env.dceState.slotUsage[id] = UseInfo { Use::Not };
}

bool isSlotLive(Env& env, uint32_t id) {
  if (id >= kMaxTrackedClsRefSlots) return true;
  return env.dceState.liveSlots[id];
}

UseInfo slotUsage(Env& env, uint32_t id) {
  return std::move(env.dceState.slotUsage[id]);
}

//////////////////////////////////////////////////////////////////////

/*
 * Note that the instructions with popConds are relying on the consumer of the
 * values they push to check whether lifetime changes can have side-effects.
 *
 * For example, in bytecode like this, assuming $x is an object with a
 * destructor:
 *
 *   CGetL $x
 *   UnsetL $x
 *   // ...
 *   PopC $x // dtor should be here.
 *
 * The PopC will decide it can't be eliminated, which prevents us from
 * eliminating the CGetL.
 */

void dce(Env& env, const bc::PopC&)          { discardNonDtors(env); }
// For PopV and PopR currently we never know if can't run a
// destructor.
void dce(Env& env, const bc::PopU&)          { discard(env); }
void dce(Env& env, const bc::Int&)           { pushRemovable(env); }
void dce(Env& env, const bc::String&)        { pushRemovable(env); }
void dce(Env& env, const bc::Array&)         { pushRemovable(env); }
void dce(Env& env, const bc::Dict&)          { pushRemovable(env); }
void dce(Env& env, const bc::Vec&)           { pushRemovable(env); }
void dce(Env& env, const bc::Keyset&)        { pushRemovable(env); }
void dce(Env& env, const bc::Double&)        { pushRemovable(env); }
void dce(Env& env, const bc::True&)          { pushRemovable(env); }
void dce(Env& env, const bc::False&)         { pushRemovable(env); }
void dce(Env& env, const bc::Null&)          { pushRemovable(env); }
void dce(Env& env, const bc::NullUninit&)    { pushRemovable(env); }
void dce(Env& env, const bc::File&)          { pushRemovable(env); }
void dce(Env& env, const bc::Dir&)           { pushRemovable(env); }
void dce(Env& env, const bc::NewArray&)      { pushRemovable(env); }
void dce(Env& env, const bc::NewDictArray&)  { pushRemovable(env); }
void dce(Env& env, const bc::NewMixedArray&) { pushRemovable(env); }
void dce(Env& env, const bc::NewCol&)        { pushRemovable(env); }
void dce(Env& env, const bc::CheckProp&)     { pushRemovable(env); }

void dce(Env& env, const bc::UnboxRNop&) {
  push(env, [&] (const UseInfo& ui) {
      pop(env);
      return allUnused(ui) ?
        PushFlags::PopOutputs : PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::ClsRefName& op) {
  // If the usage of the name is discardable, then so is this read of the
  // class-ref.
  push(env, [&] (UseInfo& ui) {
      if (allUnused(ui)) {
        readSlotDiscardable(env, op.slot, std::move(ui.actions));
      } else {
        readSlot(env, op.slot);
      }
      return PushFlags::MarkLive;
    });
}

bool clsRefGetHelper(Env& env, const Type& ty, ClsRefSlotId slot) {
  if (!ty.strictSubtypeOf(TStr)) return false;
  auto v = tv(ty);
  if (!v) return false;
  auto res = env.dceState.index.resolve_class(
    env.dceState.ainfo.ctx, v->m_data.pstr);
  if (!res || !res->resolved()) return false;
  return !isSlotLive(env, slot);
}

void dce(Env& env, const bc::ClsRefGetC& op) {
  // If the usage of this class-ref slot is dead, then it can be potentially be
  // removed if the source is dead as well.
  if (clsRefGetHelper(env, topC(env), op.slot)) {
    auto ui = slotUsage(env, op.slot);
    ui.actions.emplace(env.id, DceAction::Kill);
    writeSlot(env, op.slot);
    return pop(
      env,
      popCouldRunDestructor(env) ? Use::UsedIfLastRef : Use::Not,
      std::move(ui.actions)
    );
  }
  writeSlot(env, op.slot);
  pop(env);
}

void dce(Env& env, const bc::ClsRefGetL& op) {
  auto const ty = locRaw(env, op.loc1);
  if (clsRefGetHelper(env, ty, op.slot)) {
    assert(!readCouldHaveSideEffects(ty));
    markUisDead(env, slotUsage(env, op.slot));
    return;
  }

  addLocGen(env, op.loc1);
  writeSlot(env, op.slot);
}

void discardableWriteSlot(Env& env, ClsRefSlotId slot, bool safe) {
  if (safe && !isSlotLive(env, slot)) {
    markUisDead(env, slotUsage(env, slot));
    return;
  }
  writeSlot(env, slot);
}

void dce(Env& env, const bc::LateBoundCls& op) {
  discardableWriteSlot(env, op.slot, env.dceState.ainfo.ctx.cls != nullptr);
}

void dce(Env& env, const bc::Self& op) {
  discardableWriteSlot(env, op.slot, env.dceState.ainfo.ctx.cls != nullptr);
}

void dce(Env& env, const bc::Parent& op) {
  discardableWriteSlot(env, op.slot,
                       env.dceState.ainfo.ctx.cls != nullptr &&
                       env.dceState.ainfo.ctx.cls->parentName != nullptr);
}

void dce(Env& env, const bc::DiscardClsRef& op) {
  readSlotDiscardable(env, op.slot, {});
}

void dce(Env& env, const bc::Dup&) {
  // Various cases:
  //   - If the duplicated value is not used, delete the dup and all
  //     its consumers, then
  //     o If the original value is unused pass that on to let the
  //       instruction which pushed it decide whether to kill it
  //     o Otherwise we're done.
  //   - Otherwise, if the original value is unused, kill the dup
  //     and all dependent instructions.
  push(env, [&] (const UseInfo& u1) {
      // Dup pushes a cell that is guaranteed to be not the last reference.
      // So, it can be eliminated if the cell it pushes is used as either
      // Use::Not or Use::UsedIfLastRef.
      auto u1_unused = allUnusedIfNotLastRef(u1);
      push(env, [&] (UseInfo& u2) {
          if (allUnused(u2)) {
            if (u1_unused) {
              return PushFlags::PassThrough;
            }
            pop(env);
            return PushFlags::MarkDead;
          }
          pop(env);
          return PushFlags::MarkLive;
        });
      return u1_unused ? PushFlags::MarkDead : PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::CGetL& op) {
  push(env, [&] (const UseInfo& ui) {
      if (allUnused(ui) && !readCouldHaveSideEffects(locRaw(env, op.loc1))) {
        return PushFlags::MarkDead;
      }
      addLocGen(env, op.loc1);
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::CGetL2& op) {
  auto const ty = locRaw(env, op.loc1);
  addLocGen(env, op.loc1);

  push2(env, [&] (const UseInfo& u1, const UseInfo& u2) {
      if (readCouldHaveSideEffects(ty) || !allUnused(u1, u2)) {
        pop(env);
        return PushFlags::MarkLive;
      } else {
        return PushFlags::PassThrough;
      }
    });
}

void dce(Env& env, const bc::RetC&)  { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Throw&) { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Fatal&) { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Exit&)  { push(env); pop(env); readDtorLocs(env); }

void dce(Env& env, const bc::SetL& op) {
  auto const effects = setLocCouldHaveSideEffects(env, op.loc1);
  if (!isLocLive(env, op.loc1) && !effects) {
    assert(!locRaw(env, op.loc1).couldBe(TRef) ||
           env.stateBefore.localStaticBindings[op.loc1] ==
           LocalStaticBinding::Bound);
    return markDead(env);
  }
  push(env);
  pop(env);
  if (effects || locRaw(env, op.loc1).couldBe(TRef)) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

void dce(Env& env, const bc::UnsetL& op) {
  auto const oldTy   = locRaw(env, op.loc1);
  if (oldTy.subtypeOf(TUninit)) return markDead(env);

  // Unsetting a local bound to a static never has side effects
  // because the static itself has a reference to the value.
  auto const effects =
    setLocCouldHaveSideEffects(env, op.loc1) &&
    (env.stateBefore.localStaticBindings.size() <= op.loc1 ||
     env.stateBefore.localStaticBindings[op.loc1] != LocalStaticBinding::Bound);

  if (!isLocLive(env, op.loc1) && !effects) return markDead(env);
  if (effects) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

/*
 * IncDecL is a read-modify-write: can be removed if the local isn't live, the
 * set can't have side effects, and no one reads the value it pushes.  If the
 * instruction is not dead, always add the local to the set of upward exposed
 * uses.
 */
void dce(Env& env, const bc::IncDecL& op) {
  auto const oldTy   = locRaw(env, op.loc1);
  auto const effects = setLocCouldHaveSideEffects(env, op.loc1) ||
                         readCouldHaveSideEffects(oldTy);
  push(env, [&] (const UseInfo& ui) {
      if (!isLocLive(env, op.loc1) && !effects && allUnused(ui)) {
        return PushFlags::MarkDead;
      }
      addLocGen(env, op.loc1);
      return PushFlags::MarkLive;
    });
}

bool setOpLSideEffects(const bc::SetOpL& op, const Type& lhs, const Type& rhs) {
  auto const lhsOk = lhs.subtypeOfAny(TUninit, TNull, TBool, TInt, TDbl, TStr);
  auto const rhsOk = rhs.subtypeOfAny(TNull, TBool, TInt, TDbl, TStr);
  if (!lhsOk || !rhsOk) return true;

  switch (op.subop2) {
    case SetOpOp::ConcatEqual:
      return false;

    case SetOpOp::PlusEqual:
    case SetOpOp::PlusEqualO:
    case SetOpOp::MinusEqual:
    case SetOpOp::MulEqual:
    case SetOpOp::DivEqual:
    case SetOpOp::ModEqual:
    case SetOpOp::PowEqual:
    case SetOpOp::AndEqual:
    case SetOpOp::OrEqual:
    case SetOpOp::XorEqual:
    case SetOpOp::MinusEqualO:
    case SetOpOp::MulEqualO:
    case SetOpOp::SlEqual:
    case SetOpOp::SrEqual:
      return RuntimeOption::EnableHipHopSyntax &&
        (lhs.subtypeOf(TStr) || rhs.subtypeOf(TStr));
  }
  not_reached();
}

/*
 * SetOpL is like IncDecL, but with the complication that we don't
 * know if we can mark it dead when visiting it, because it is going
 * to pop an input but unlike SetL doesn't push the value it popped.
 * However, since we've checked the input types, it *is* ok to just
 * kill it.
 */
void dce(Env& env, const bc::SetOpL& op) {
  auto const oldTy   = locRaw(env, op.loc1);

  push(env, [&] (UseInfo& ui) {
      if (!isLocLive(env, op.loc1) && allUnused(ui)) {
        if (!setLocCouldHaveSideEffects(env, op.loc1) &&
            !readCouldHaveSideEffects(oldTy) &&
            !setOpLSideEffects(op, oldTy, topC(env))) {
          pop(env, Use::Not, std::move(ui.actions));
          return PushFlags::MarkDead;
        }
      }
      pop(env);
      addLocGen(env, op.loc1);
      return PushFlags::MarkLive;
    });
}

/*
 * Default implementation is conservative: assume we use all of our
 * inputs, and can't be removed even if our output is unused.
 *
 * We also assume all the locals in the mayReadLocalSet must be
 * added to the live local set, and don't remove anything from it.
 */

template<typename Op>
typename std::enable_if<has_car<Op>::value,void>::type
dce_slot_default(Env& env, const Op& op) {
  readSlot(env, op.slot);
}

template<typename Op>
typename std::enable_if<has_caw<Op>::value,void>::type
dce_slot_default(Env& env, const Op& op) {
  writeSlot(env, op.slot);
}

template<typename Op>
typename std::enable_if<!has_car<Op>::value && !has_caw<Op>::value,void>::type
dce_slot_default(Env& env, const Op& op) {}

template<class Op>
void dce(Env& env, const Op& op) {
  addLocGenSet(env, env.flags.mayReadLocalSet);
  for (auto i = uint32_t{0}; i < op.numPush(); ++i) {
    push(env);
  }
  for (auto i = uint32_t{0}; i < op.numPop(); ++i) {
    pop(env);
  }
  dce_slot_default(env, op);
}

void dispatch_dce(Env& env, const Bytecode& op) {
#define O(opcode, ...) case Op::opcode: dce(env, op.opcode); return;
  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

struct DceOutState {
  DceOutState() = default;
  enum Local {};
  explicit DceOutState(Local) {
    locLive.set();
    locLiveExn.set();
    slotLive.set();
    slotLiveExn.set();
  }

  /*
   * The union of the liveIn states of each normal successor for
   * locals and stack slots respectively.
   */
  std::bitset<kMaxTrackedLocals>             locLive;
  std::bitset<kMaxTrackedClsRefSlots>        slotLive;

  /*
   * The union of the liveIn states of each exceptional successor for
   * locals and stack slots respectively.
   */
  std::bitset<kMaxTrackedLocals>             locLiveExn;
  std::bitset<kMaxTrackedClsRefSlots>        slotLiveExn;

  /*
   * The union of the dceStacks from the start of the normal successors.
   */
  folly::Optional<std::vector<UseInfo>>      dceStack;

  /*
   * The union of the slotUsage from the start of the normal
   * successors.
   */
  folly::Optional<std::vector<UseInfo>>       slotUsage;
};

folly::Optional<DceState>
dce_visit(const Index& index,
          const FuncAnalysis& fa,
          borrowed_ptr<const php::Block> const blk,
          const State& stateIn,
          const DceOutState& dceOutState) {
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

  auto const states = locally_propagated_states(index, fa, blk, stateIn);

  auto dceState = DceState{ index, fa };
  dceState.liveLocals = dceOutState.locLive;
  dceState.usedLocals = dceOutState.locLive;
  dceState.liveSlots  = dceOutState.slotLive;
  dceState.usedSlots  = dceOutState.slotLive;

  if (dceOutState.dceStack) {
    dceState.stack = *dceOutState.dceStack;
    assert(dceState.stack.size() == states.back().first.stack.size());
  } else {
    dceState.stack.resize(states.back().first.stack.size(),
                          UseInfo { Use::Used });
  }

  if (dceOutState.slotUsage) {
    dceState.slotUsage = *dceOutState.slotUsage;
    assert(dceState.slotUsage.size() ==
           states.back().first.clsRefSlots.size());
  } else {
    dceState.slotUsage.resize(
      states.back().first.clsRefSlots.size(), UseInfo { Use::Used });
  }

  for (uint32_t idx = blk->hhbcs.size(); idx-- > 0;) {
    auto const& op = blk->hhbcs[idx];

    FTRACE(2, "  == #{} {}\n", idx, show(fa.ctx.func, op));

    auto visit_env = Env {
      dceState,
      op,
      { blk->id, idx },
      states[idx].first,
      states[idx].second
    };
    dispatch_dce(visit_env, op);

    /*
     * When we see a PEI, liveness must take into account the fact
     * that we could take an exception edge here (or'ing in the
     * liveExn sets).
     */
    if (states[idx].second.wasPEI) {
      FTRACE(2, "    <-- exceptions\n");
      dceState.liveLocals |= dceOutState.locLiveExn;
      dceState.liveSlots  |= dceOutState.slotLiveExn;
    }

    dceState.usedLocals |= dceState.liveLocals;
    dceState.usedSlots  |= dceState.liveSlots;

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
          | map([&] (const StackElem& e) { return show(e.type); })
          | unsplit<std::string>(" ");
      }()
    );

    FTRACE(4, "    cls-ref slots: {}\n{}\n",
      slot_bits_string(dceState.ainfo.ctx.func, dceState.liveSlots),
      [&]{
        using namespace folly::gen;
        auto i = uint32_t{0};
        return from(dceState.slotUsage)
          | mapped(
            [&] (const UseInfo& ui) {
              if (ui.actions.empty()) return std::string{};
              return folly::sformat("  {}: [{}]\n",
                                    i++, show(ui.actions));
            })
          | unsplit<std::string>("");
      }()
    );

    // We're now at the state before this instruction, so the stack
    // sizes must line up.
    assert(dceState.stack.size() == states[idx].first.stack.size());
  }

  return dceState;
}

struct DceAnalysis {
  std::bitset<kMaxTrackedLocals>      locLiveIn;
  std::bitset<kMaxTrackedClsRefSlots> slotLiveIn;
  std::vector<UseInfo>                stack;
  std::vector<UseInfo>                slotUsage;
  std::set<LocationId>                forcedLiveLocations;
};

DceAnalysis analyze_dce(const Index& index,
                        const FuncAnalysis& fa,
                        borrowed_ptr<php::Block> const blk,
                        const State& stateIn,
                        const DceOutState& dceOutState) {
  if (auto dceState = dce_visit(index, fa, blk, stateIn, dceOutState)) {
    return DceAnalysis {
      dceState->liveLocals,
      dceState->liveSlots,
      dceState->stack,
      dceState->slotUsage,
      dceState->forcedLiveLocations
    };
  }
  return DceAnalysis {};
}

/*
 * Do the actual updates to the bytecodes.
 */
void dce_perform(const php::Func& func, const DceActionMap& actionMap) {
  for (auto const& elm : actionMap) {
    auto const& id = elm.first;
    auto const b = borrow(func.blocks[id.blk]);
    FTRACE(1, "{}: {}:{} {}\n", show(elm.second),
           id.blk, id.idx, show(func, b->hhbcs[id.idx]));
    switch (elm.second) {
      case DceAction::Kill:
        if (b->hhbcs.size() == 1) {
          // we don't allow empty blocks
          b->hhbcs[0] = bc::Nop {};
        } else {
          b->hhbcs.erase(b->hhbcs.begin() + id.idx);
        }
        break;
      case DceAction::PopOutputs:
      {
        auto const numPop = b->hhbcs[id.idx].numPush();
        b->hhbcs.insert(b->hhbcs.begin() + id.idx + 1,
                        numPop,
                        bc::PopC {});
        break;
      }
    }
  }
}

struct DceOptResult {
  std::bitset<kMaxTrackedLocals> usedLocals;
  std::bitset<kMaxTrackedClsRefSlots> usedSlots;
  DceActionMap actionMap;
};

DceOptResult
optimize_dce(const Index& index,
             const FuncAnalysis& fa,
             borrowed_ptr<php::Block> const blk,
             const State& stateIn,
             const DceOutState& dceOutState) {
  auto dceState = dce_visit(index, fa, blk, stateIn, dceOutState);

  if (!dceState) {
    return {std::bitset<kMaxTrackedLocals>{},
            std::bitset<kMaxTrackedClsRefSlots>{}};
  }

  return {
    std::move(dceState->usedLocals),
    std::move(dceState->usedSlots),
    std::move(dceState->actionMap)
  };
}

//////////////////////////////////////////////////////////////////////

void remove_unused_locals(Context const ctx,
                          std::bitset<kMaxTrackedLocals> usedLocals) {
  if (!options.RemoveUnusedLocals) return;
  auto const func = ctx.func;

  /*
   * Removing unused locals in closures requires checking which ones
   * are captured variables so we can remove the relevant properties,
   * and then we'd have to mutate the CreateCl callsite, so we don't
   * bother for now.
   *
   * Note: many closure bodies have unused $this local, because of
   * some emitter quirk, so this might be worthwhile.
   */
  if (func->isClosureBody) return;

  for (auto loc = func->locals.begin() + func->params.size();
       loc != func->locals.end(); ++loc) {
    if (loc->killed) {
      assert(loc->id < kMaxTrackedLocals && !usedLocals.test(loc->id));
      continue;
    }
    if (loc->id < kMaxTrackedLocals && !usedLocals.test(loc->id)) {
      FTRACE(2, "  killing: {}\n", local_string(*func, loc->id));
      const_cast<php::Local&>(*loc).killed = true;
    }
  }
}

//////////////////////////////////////////////////////////////////////

namespace {

struct WritableClsRefSlotVisitor : boost::static_visitor<ClsRefSlotId*> {
  template<class T>
  typename std::enable_if<
    !has_car<T>::value && !has_caw<T>::value, ClsRefSlotId*
  >::type
  operator()(T& t) const { return nullptr; }

  template<class T>
  typename std::enable_if<
    has_car<T>::value || has_caw<T>::value, ClsRefSlotId*
  >::type
  operator()(T& t) const { return &t.slot; }
};

}

// Remove totally unused class-ref slots. Shift any usages downward so we can
// reduce the total number of class-ref slots needed.
void remove_unused_clsref_slots(Context const ctx,
                                std::bitset<kMaxTrackedClsRefSlots> usedSlots) {
  if (!options.RemoveUnusedClsRefSlots) return;
  auto func = ctx.func;

  auto numSlots = func->numClsRefSlots;
  if (numSlots > kMaxTrackedClsRefSlots) return;

  auto unusedSlots = usedSlots;
  unusedSlots.flip();

  // Construct a mapping of class-ref slots that should be rewritten to a
  // different one. For each totally unused class-ref slot, rewrite a higher
  // (used) class-ref to it.
  boost::container::flat_map<size_t, size_t> rewrites;
  while (numSlots > 0) {
    if (!unusedSlots[numSlots - 1]) {
      auto const unused = bitset_find_first(unusedSlots);
      if (unused >= numSlots) break;
      unusedSlots[unused] = false;
      unusedSlots[numSlots - 1] = true;
      rewrites[numSlots - 1] = unused;
    }
    --numSlots;
  }

  FTRACE(2, "    cls-ref rewrites: {}\n",
    [&]{
      using namespace folly::gen;
      return from(rewrites)
        | mapped(
          [&] (const std::pair<size_t, size_t>& p) {
            return folly::sformat("{}->{}", p.first, p.second);
          })
        | unsplit<std::string>(";");
    }()
  );

  // Do the actually rewriting
  if (!rewrites.empty()) {
    for (auto& block : func->blocks) {
      for (auto& bcop : block->hhbcs) {
        if (auto* slot = visit(bcop, WritableClsRefSlotVisitor{})) {
          auto const iter = rewrites.find(*slot);
          if (iter == rewrites.end()) continue;
          auto const oldOp = bcop;
          *slot = iter->second;
          FTRACE(4, "    rewriting {} to {}\n",
                 show(func, oldOp), show(func, bcop));
        }
      }
    }
  }

  func->numClsRefSlots = numSlots;
}

//////////////////////////////////////////////////////////////////////

}

void local_dce(const Index& index,
               const FuncAnalysis& ainfo,
               borrowed_ptr<php::Block> const blk,
               const State& stateIn) {
  // For local DCE, we have to assume all variables are in the
  // live-out set for the block.
  auto const ret = optimize_dce(index, ainfo, blk, stateIn,
                                DceOutState{DceOutState::Local{}});

  dce_perform(*ainfo.ctx.func, ret.actionMap);
}

//////////////////////////////////////////////////////////////////////

void global_dce(const Index& index, const FuncAnalysis& ai) {
  auto rpoId = [&] (borrowed_ptr<php::Block> blk) {
    return ai.bdata[blk->id].rpoId;
  };

  FTRACE(1, "|---- global DCE analyze ({})\n", show(ai.ctx));
  FTRACE(2, "{}", [&] {
    using namespace folly::gen;
    auto i = uint32_t{0};
    return from(ai.ctx.func->locals)
      | mapped(
        [&] (const php::Local& l) {
          return folly::sformat("  {} {}\n",
                                i++, local_string(*ai.ctx.func, l.id));
        })
      | unsplit<std::string>("");
  }());

  /*
   * States for each block, indexed by block id.
   */
  std::vector<DceOutState> blockStates(ai.ctx.func->blocks.size());

  /*
   * Set of block reverse post order ids that still need to be
   * visited.  This is ordered by std::greater, so we'll generally
   * visit blocks before their predecessors.  The algorithm does not
   * depend on this for correctness, but it will reduce iteration, and
   * the optimality of mergeDceStack depends on visiting at least one
   * successor prior to visiting any given block.
   *
   * Every block must be visited at least once, so we throw them all
   * in to start.
   */
  auto incompleteQ = dataflow_worklist<uint32_t,std::less<uint32_t>>(
    ai.rpoBlocks.size()
  );
  for (auto& b : ai.rpoBlocks) incompleteQ.push(rpoId(b));

  auto const normalPreds   = computeNormalPreds(ai.rpoBlocks);
  auto const factoredPreds = computeFactoredPreds(ai.rpoBlocks);

  /*
   * Suppose a stack slot isn't used, but it was pushed on two separate
   * paths, one of which could be killed, but the other can't. We have
   * to prevent either, so any time we find a non-used stack slot that
   * can't be killed, we add it to this set (via markUisLive, and the
   * DceAnalysis).
   */
  std::set<LocationId> forcedLiveLocations;

  auto fixupUseInfo = [&] (std::vector<UseInfo>& uis,
                           BlockId blk,
                           bool isSlot) {
    for (uint32_t i = 0; i < uis.size(); i++) {
      auto& out = uis[i];
      if (out.location.blk == NoBlockId) out.location = { blk, i, isSlot };
      if (forcedLiveLocations.count(out.location)) {
        out.usage = Use::Used;
        out.actions.clear();
      }
    }
  };

  auto mergeUIs = [&] (UseInfo& out, const UseInfo& in,
                       uint32_t i, BlockId blk, bool isSlot) {
    if (out.usage == Use::Used) {
      return false;
    }
    auto location = in.location;
    if (location.blk == NoBlockId) location = { blk, i, isSlot };
    if (in.usage == Use::Used ||
        forcedLiveLocations.count(location)) {
      out.usage = Use::Used;
      out.actions.clear();
      return true;
    }
    if (out.usage != in.usage) {
      out.usage = Use::Used;
      out.actions.clear();
      return true;
    }
    auto ret = false;
    for (auto const& id : in.actions) {
      ret |= out.actions.insert(id).second;
    }
    assert(out.location.blk != NoBlockId);
    if (out.location < location) {
      // It doesn't matter which one we choose, but it should be
      // independent of the visiting order.
      out.location = location;
      ret = true;
    }
    return ret;
  };

  auto mergeUIVecs = [&] (folly::Optional<std::vector<UseInfo>>& stkOut,
                          const std::vector<UseInfo>& stkIn,
                          BlockId blk, bool isSlot) {
    if (!stkOut) {
      stkOut = stkIn;
      fixupUseInfo(*stkOut, blk, isSlot);
      return true;
    }

    auto ret = false;
    assert(stkOut->size() == stkIn.size());
    for (uint32_t i = 0; i < stkIn.size(); i++) {
      mergeUIs(stkOut.value()[i], stkIn[i], i, blk, isSlot);
    }
    return ret;
  };

  /*
   * Iterate on live out states until we reach a fixed point.
   *
   * This algorithm treats the exceptional live-out states differently
   * from the live-out states during normal control flow.  The
   * liveOutExn sets only take part in the liveIn computation when the
   * block has factored exits.
   */
  while (!incompleteQ.empty()) {
    auto const blk = ai.rpoBlocks[incompleteQ.pop()];

    // skip unreachable blocks
    if (!ai.bdata[blk->id].stateIn.initialized) continue;

    FTRACE(2, "block #{}\n", blk->id);

    auto& blockState = blockStates[blk->id];
    auto const result = analyze_dce(
      index,
      ai,
      blk,
      ai.bdata[blk->id].stateIn,
      blockState
    );

    FTRACE(2, "loc live out  : {}\n"
              "loc out exn   : {}\n"
              "loc live in   : {}\n"
              "slot live out : {}\n"
              "slot out exn  : {}\n"
              "slot live in  : {}\n",
              loc_bits_string(ai.ctx.func, blockState.locLive),
              loc_bits_string(ai.ctx.func, blockState.locLiveExn),
              loc_bits_string(ai.ctx.func, result.locLiveIn),
              slot_bits_string(ai.ctx.func, blockState.slotLive),
              slot_bits_string(ai.ctx.func, blockState.slotLiveExn),
              slot_bits_string(ai.ctx.func, result.slotLiveIn));

    /*
     * If we weren't able to take advantage of any unused entries
     * on the dceStack that originated from another block, mark
     * them used to prevent other paths from trying to delete them.
     *
     * Also, merge the entries into our global forcedLiveLocations, to
     * make sure they always get marked Used.
     */
    for (auto const& id : result.forcedLiveLocations) {
      if (forcedLiveLocations.insert(id).second) {
        for (auto& pred : normalPreds[id.blk]) {
          FTRACE(2, "  {} force-live {}:{}{}\n",
                 id.blk, pred->id, id.id, id.isSlot ? "S" : "");
          auto& pbs = blockStates[pred->id];
          if (id.isSlot) {
            if (!pbs.slotLive[id.id]) {
              pbs.slotLive[id.id] = true;
              if (pred != blk) {
                incompleteQ.push(rpoId(pred));
              }
            }
            continue;
          }
          auto& dceStack = pbs.dceStack;
          if (!dceStack) continue;
          auto& ui = dceStack.value()[id.id];
          if (ui.usage != Use::Used) {
            ui.actions.clear();
            ui.usage = Use::Used;
            // no need to reprocess *this* block, since this is the
            // one that reported the problem.
            if (pred != blk) {
              incompleteQ.push(rpoId(pred));
            }
          }
        }
      }
    }

    // Merge the liveIn into the liveOut of each normal predecessor.
    // If the set changes, reschedule that predecessor.
    for (auto& pred : normalPreds[blk->id]) {
      FTRACE(2, "  -> {}\n", pred->id);
      auto& pbs = blockStates[pred->id];
      auto const oldPredLocLive = pbs.locLive;
      pbs.locLive |= result.locLiveIn;
      auto const oldPredSlotLive = pbs.slotLive;
      pbs.slotLive |= result.slotLiveIn;
      auto changed = mergeUIVecs(pbs.slotUsage, result.slotUsage,
                                 blk->id, true);
      if (mergeUIVecs(pbs.dceStack, result.stack, blk->id, false) ||
          changed ||
          pbs.locLive != oldPredLocLive ||
          pbs.slotLive != oldPredSlotLive) {
        incompleteQ.push(rpoId(pred));
      }
    }

    // Merge the liveIn into the liveOutExn state for each exceptional
    // precessor.  The liveIn computation also depends on the
    // liveOutExn state, so again reschedule if it changes.
    for (auto& pred : factoredPreds[blk->id]) {
      FTRACE(2, "  => {}\n", pred->id);
      auto& pbs = blockStates[pred->id];
      auto const oldPredLocLiveExn = pbs.locLiveExn;
      pbs.locLiveExn |= result.locLiveIn;
      auto const oldPredSlotLiveExn = pbs.slotLiveExn;
      pbs.slotLiveExn |= result.slotLiveIn;
      if (pbs.locLiveExn != oldPredLocLiveExn ||
          pbs.slotLiveExn != oldPredSlotLiveExn) {
        incompleteQ.push(rpoId(pred));
      }
    }
  }

  /*
   * Now that we're at a fixed point, use the propagated states to
   * remove instructions that don't need to be there.
   */
  FTRACE(1, "|---- global DCE optimize ({})\n", show(ai.ctx));
  std::bitset<kMaxTrackedLocals> usedLocals;
  std::bitset<kMaxTrackedClsRefSlots> usedSlots;
  DceActionMap actionMap;
  for (auto& b : ai.rpoBlocks) {
    FTRACE(2, "block #{}\n", b->id);
    auto ret = optimize_dce(
      index,
      ai,
      b,
      ai.bdata[b->id].stateIn,
      blockStates[b->id]
    );
    usedLocals |= ret.usedLocals;
    usedSlots  |= ret.usedSlots;
    if (ret.actionMap.size()) {
      if (!actionMap.size()) {
        actionMap = std::move(ret.actionMap);
      } else {
        for (auto const& elm : ret.actionMap) {
          actionMap.insert(elm);
        }
      }
    }
  }

  dce_perform(*ai.ctx.func, actionMap);

  FTRACE(1, "  used locals: {}\n", loc_bits_string(ai.ctx.func, usedLocals));
  remove_unused_locals(ai.ctx, usedLocals);

  FTRACE(1, "  used slots: {}\n", slot_bits_string(ai.ctx.func, usedSlots));
  remove_unused_clsref_slots(ai.ctx, usedSlots);
}

//////////////////////////////////////////////////////////////////////

}}
