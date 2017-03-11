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

using InstrId    = size_t;
using InstrIdSet = std::set<InstrId>;
using UseInfo    = std::pair<Use,InstrIdSet>;

//////////////////////////////////////////////////////////////////////

struct DceState {
  borrowed_ptr<const php::Func> func;

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
  std::bitset<kMaxTrackedLocals> locGen;
  std::bitset<kMaxTrackedLocals> locKill;
  std::bitset<kMaxTrackedLocals> locKillBeforePEI;

  /*
   * Instructions marked in this set are dead.  If any of them are
   * removed, however, they must all be removed, because of the need
   * to keep eval stack consumers and producers balanced.
   */
  boost::dynamic_bitset<> markedDead{};

  /*
   * The set of locals and class-ref slots that were ever live in this block.
   * (This includes ones that were live going out of this block.)  This set is
   * used by global DCE to remove locals and class-ref slots that are completely
   * unused in the entire function.
   */
  std::bitset<kMaxTrackedLocals> usedLocals;
  std::bitset<kMaxTrackedClsRefSlots> usedSlots;

  /*
   * Mapping of class-ref slots to instruction sets. If the usage of a class-ref
   * slot is removed, all the instructions currently in the set must also be
   * removed.
   */
  std::vector<InstrIdSet> slotDependentInstrs;
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

std::string show(const InstrIdSet& set) {
  using namespace folly::gen;
  return from(set)
    | eachTo<std::string>()
    | unsplit<std::string>(";")
    ;
}

std::string DEBUG_ONLY show(const UseInfo& ui) {
  return folly::format("{}@{}", show(ui.first), show(ui.second)).str();
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
  InstrId id;
  const State& stateBefore;
  const StepFlags& flags;
};

void markSetDead(Env& env, const InstrIdSet& set) {
  env.dceState.markedDead[env.id] = 1;
  FTRACE(2, "     marking {} {}\n", env.id, show(set));
  for (auto& i : set) env.dceState.markedDead[i] = 1;
}

void markDead(Env& env) {
  env.dceState.markedDead[env.id] = 1;
  FTRACE(2, "     marking {}\n", env.id);
}

//////////////////////////////////////////////////////////////////////
// eval stack

void pop(Env& env, Use u, InstrIdSet set) {
  FTRACE(2, "      pop({})\n", show(u));
  env.dceState.stack.emplace_back(u, std::move(set));
}
void pop(Env& env) { pop(env, Use::Used, InstrIdSet{}); }

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
  pop(env, Use::Not, InstrIdSet{env.id});
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
void popCond(Env& env, Args&&... args) {
  bool unused = allUnused(std::forward<Args>(args)...);
  if (!unused) return pop(env, Use::Used, InstrIdSet{});
  auto accum = InstrIdSet{env.id};
  combineSets(accum, std::forward<Args>(args)...);
  pop(env, Use::Not, accum);
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
  auto const t = topC(env);
  if (couldRunDestructor(t)) {
    return pop(env, Use::UsedIfLastRef, InstrIdSet{env.id});
  }
  discard(env);
}

UseInfo push(Env& env) {
  always_assert(!env.dceState.stack.empty());
  auto ret = env.dceState.stack.back();
  env.dceState.stack.pop_back();
  FTRACE(2, "      {}@{} = push()\n", show(ret.first), show(ret.second));
  return ret;
}

void pushRemovable(Env& env) {
  auto const ui = push(env);
  switch (ui.first) {
  case Use::Not:
    markSetDead(env, ui.second);
    break;
  case Use::Used:
  case Use::UsedIfLastRef:
    break;
  }
}

//////////////////////////////////////////////////////////////////////
// locals

void addLocGenSet(Env& env, std::bitset<kMaxTrackedLocals> locs) {
  FTRACE(4, "      loc-conservative: {}\n",
         loc_bits_string(env.dceState.func, locs));
  env.dceState.liveLocals |= locs;
  env.dceState.locGen |= locs;
  env.dceState.locKill &= ~locs;
  env.dceState.locKillBeforePEI &= ~locs;
}

void addLocGen(Env& env, uint32_t id) {
  FTRACE(2, "      loc-gen: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 1;
  env.dceState.locGen[id] = 1;
  env.dceState.locKill[id] = 0;
  env.dceState.locKillBeforePEI[id] = 0;
}

void addLocKill(Env& env, uint32_t id) {
  FTRACE(2, "     loc-kill: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 0;
  env.dceState.locGen[id] = 0;
  env.dceState.locKill[id] = 1;
  env.dceState.locKillBeforePEI[id] = 1;
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

void readDtorLocs(Env& env) {
  for (auto i = size_t{0}; i < env.stateBefore.locals.size(); ++i) {
    if (couldRunDestructor(env.stateBefore.locals[i])) {
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
  env.dceState.usedSlots[id] = 1;
  env.dceState.slotDependentInstrs[id].clear();
}

// Read a slot, but in a usage that is discardable. If this read actually is
// discarded, then also discard the given instructions.
void readSlotDiscardable(Env& env, uint32_t id, InstrIdSet instrs) {
  FTRACE(2, "     read-slot (discardable): {}\n", id);
  if (id >= kMaxTrackedClsRefSlots) return;
  env.dceState.liveSlots[id] = 0;
  env.dceState.usedSlots[id] = 1;
  instrs.insert(env.id);
  env.dceState.slotDependentInstrs[id] = std::move(instrs);
}

void writeSlot(Env& env, uint32_t id) {
  FTRACE(2, "     write-slot: {}\n", id);
  if (id >= kMaxTrackedClsRefSlots) return;
  env.dceState.liveSlots[id] = 0;
  env.dceState.usedSlots[id] = 1;
  env.dceState.slotDependentInstrs[id].clear();
}

bool isSlotLive(Env& env, uint32_t id) {
  if (id >= kMaxTrackedClsRefSlots) return true;
  return env.dceState.liveSlots[id];
}

InstrIdSet slotDependentInstrs(Env& env, uint32_t id) {
  return env.dceState.slotDependentInstrs[id];
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

void dce(Env& env, const bc::ClsRefName& op) {
  // If the usage of the name is discardable, then so is this read of the
  // class-ref.
  auto ui = push(env);
  switch (ui.first) {
    case Use::Not:
      readSlotDiscardable(env, op.slot, std::move(ui.second));
      break;
    case Use::UsedIfLastRef:
    case Use::Used:
      readSlot(env, op.slot);
      break;
  }
}

void dce(Env& env, const bc::ClsRefGetC& op) {
  // If the usage of this class-ref slot is dead, then it can be potentially be
  // removed if the source is dead as well.
  if (!isSlotLive(env, op.slot)) {
    auto instrs = slotDependentInstrs(env, op.slot);
    instrs.insert(env.id);
    writeSlot(env, op.slot);
    return pop(
      env,
      couldRunDestructor(topC(env)) ? Use::UsedIfLastRef : Use::Not,
      std::move(instrs)
    );
  }
  writeSlot(env, op.slot);
  pop(env);
}

void dce(Env& env, const bc::ClsRefGetL& op) {
  auto const ty = locRaw(env, op.loc1);
  addLocGen(env, op.loc1);

  if (!isSlotLive(env, op.slot) && !readCouldHaveSideEffects(ty)) {
    auto instrs = slotDependentInstrs(env, op.slot);
    instrs.insert(env.id);
    writeSlot(env, op.slot);
    markSetDead(env, std::move(instrs));
    return;
  }

  writeSlot(env, op.slot);
}

void dce(Env& env, const bc::DiscardClsRef& op) {
  readSlotDiscardable(env, op.slot, InstrIdSet{});
}

void dce(Env& env, const bc::Dup&) {
  auto const u1 = push(env);
  auto const u2 = push(env);
  // Dup pushes a cell that is guaranteed to be not the last reference.
  // So, it can be eliminated if the cell it pushes is used as either
  // Use::Not or Use::UsedIfLastRef.
  // The cell it pops can be marked Use::Not only if Dup itself
  // can be eliminated.
  switch (u1.first) {
  case Use::Not:
  case Use::UsedIfLastRef:
    // It is ok to eliminate the Dup even if its second output u2
    // is used, because eliminating the Dup still leaves the second
    // output u2 on stack.
    markSetDead(env, u1.second);
    switch (u2.first) {
    case Use::Not:
      pop(env, Use::Not, u2.second);
      break;
    case Use::Used:
    case Use::UsedIfLastRef:
      pop(env, Use::Used, InstrIdSet{});
      break;
    }
    break;
  case Use::Used:
    pop(env, Use::Used, InstrIdSet{});
    break;
  }
}

void dce(Env& env, const bc::CGetL& op) {
  auto const ty = locRaw(env, op.loc1);
  addLocGen(env, op.loc1);
  if (readCouldHaveSideEffects(ty)) {
    push(env);
  } else {
    pushRemovable(env);
  }
}

void dce(Env& env, const bc::CGetL2& op) {
  auto const ty = locRaw(env, op.loc1);
  addLocGen(env, op.loc1);
  auto const u1 = push(env);
  auto const u2 = push(env);
  if (readCouldHaveSideEffects(ty)) {
    pop(env);
  } else {
    popCond(env, u1, u2);
  }
}

void dce(Env& env, const bc::RetC&)  { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Throw&) { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Fatal&) { pop(env); readDtorLocs(env); }
void dce(Env& env, const bc::Exit&)  { push(env); pop(env); readDtorLocs(env); }

void dce(Env& env, const bc::SetL& op) {
  auto const oldTy   = locRaw(env, op.loc1);
  auto const effects = setCouldHaveSideEffects(oldTy);
  if (!isLocLive(env, op.loc1) && !effects) return markDead(env);
  push(env);
  pop(env);
  if (effects) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

void dce(Env& env, const bc::UnsetL& op) {
  auto const oldTy   = locRaw(env, op.loc1);
  auto const effects = setCouldHaveSideEffects(oldTy);
  if (oldTy.subtypeOf(TUninit)) return markDead(env);
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
  auto const effects = setCouldHaveSideEffects(oldTy) ||
                         readCouldHaveSideEffects(oldTy);
  auto const u1      = push(env);
  if (!isLocLive(env, op.loc1) && !effects && allUnused(u1)) {
    return markSetDead(env, u1.second);
  }
  addLocGen(env, op.loc1);
}

/*
 * SetOpL is like IncDecL, but with the complication that we don't know if we
 * can mark it dead when visiting it, because it is going to pop an input but
 * unlike SetL doesn't push the value it popped.  For the current scheme we
 * just add the local to gen even if we're doing a removable push, which is
 * correct but could definitely fail to eliminate some earlier stores.
 */
void dce(Env& env, const bc::SetOpL& op) {
  auto const oldTy   = locRaw(env, op.loc1);
  auto const effects = setCouldHaveSideEffects(oldTy) ||
                         readCouldHaveSideEffects(oldTy);
  if (!isLocLive(env, op.loc1) && !effects) {
    popCond(env, push(env));
  } else {
    push(env);
    pop(env);
  }
  addLocGen(env, op.loc1);
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
  env.dceState.liveLocals |= env.flags.mayReadLocalSet;
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

folly::Optional<DceState>
dce_visit(const Index& index,
          const FuncAnalysis& fa,
          borrowed_ptr<const php::Block> const blk,
          const State& stateIn,
          std::bitset<kMaxTrackedLocals> locLiveOut,
          std::bitset<kMaxTrackedLocals> locLiveOutExn) {
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

  auto dceState = DceState{};
  dceState.func = fa.ctx.func;
  dceState.markedDead.resize(blk->hhbcs.size());
  dceState.liveLocals = locLiveOut;
  dceState.liveSlots.set();
  dceState.stack.resize(states.back().first.stack.size());
  dceState.usedLocals = locLiveOut;
  for (auto& s : dceState.stack) {
    s = UseInfo { Use::Used, InstrIdSet{} };
  }
  dceState.slotDependentInstrs.resize(states.back().first.clsRefSlots.size());

  for (auto idx = blk->hhbcs.size(); idx-- > 0;) {
    auto const& op = blk->hhbcs[idx];

    FTRACE(2, "  == #{} {}\n", idx, show(fa.ctx.func, op));

    auto visit_env = Env {
      dceState,
      idx,
      states[idx].first,
      states[idx].second
    };
    dispatch_dce(visit_env, op);

    /*
     * When we see a PEI, we need to start over on the killBeforePEI
     * set, and the local-liveness must take into account the fact
     * that we could take an exception edge here (or'ing in the
     * liveOutExn set).
     */
    if (states[idx].second.wasPEI) {
      FTRACE(2, "    <-- exceptions\n");
      dceState.liveLocals |= locLiveOutExn;
      dceState.liveSlots.set();
      dceState.locKillBeforePEI.reset();
    }

    dceState.usedLocals |= dceState.liveLocals;

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
      slot_bits_string(dceState.func, dceState.liveSlots),
      [&]{
        using namespace folly::gen;
        auto i = uint32_t{0};
        return from(dceState.slotDependentInstrs)
          | mapped(
            [&] (const InstrIdSet& s) {
              if (s.empty()) return std::string{};
              return folly::sformat("  {}: [{}]\n",
                                    i++, show(s));
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
  std::bitset<kMaxTrackedLocals> locGen;
  std::bitset<kMaxTrackedLocals> locKill;
  std::bitset<kMaxTrackedLocals> locKillExn;
};

DceAnalysis analyze_dce(const Index& index,
                        const FuncAnalysis& fa,
                        borrowed_ptr<php::Block> const blk,
                        const State& stateIn) {
  // During this analysis pass, we have to assume everything could be
  // live out, so we set allLive here.  (Later we'll determine the
  // real liveOut sets.)
  auto allLocLive = std::bitset<kMaxTrackedLocals>{};
  allLocLive.set();
  if (auto dceState = dce_visit(index, fa, blk, stateIn,
                                allLocLive, allLocLive)) {
    return DceAnalysis {
      dceState->locGen,
      dceState->locKill,
      dceState->locKillBeforePEI
    };
  }
  return DceAnalysis {};
}

std::pair<std::bitset<kMaxTrackedLocals>,
          std::bitset<kMaxTrackedClsRefSlots>>
optimize_dce(const Index& index,
             const FuncAnalysis& fa,
             borrowed_ptr<php::Block> const blk,
             const State& stateIn,
             std::bitset<kMaxTrackedLocals> locLiveOut,
             std::bitset<kMaxTrackedLocals> locLiveOutExn) {
  auto const dceState = dce_visit(
    index, fa, blk,
    stateIn, locLiveOut,
    locLiveOutExn
  );
  if (!dceState) {
    return {std::bitset<kMaxTrackedLocals>{},
            std::bitset<kMaxTrackedClsRefSlots>{}};
  }

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

  return {dceState->usedLocals, dceState->usedSlots};
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
  Trace::Bump bumper{Trace::hhbbc_dce, kSystemLibBump,
    is_systemlib_part(*ainfo.ctx.unit)};

  // For local DCE, we have to assume all variables are in the
  // live-out set for the block.
  auto allLocLive = std::bitset<kMaxTrackedLocals>();
  allLocLive.set();
  optimize_dce(index, ainfo, blk, stateIn,
               allLocLive, allLocLive);
}

//////////////////////////////////////////////////////////////////////

void global_dce(const Index& index, const FuncAnalysis& ai) {
  Trace::Bump bumper{Trace::hhbbc_dce, kSystemLibBump,
    is_systemlib_part(*ai.ctx.unit)};

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
      ai,
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
    std::bitset<kMaxTrackedLocals> locLiveOut;
    std::bitset<kMaxTrackedLocals> locLiveOutExn;
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
  auto incompleteQ = dataflow_worklist<uint32_t,std::less<uint32_t>>(
    ai.rpoBlocks.size()
  );
  for (auto& b : ai.rpoBlocks) incompleteQ.push(rpoId(b));

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
    auto const blk = ai.rpoBlocks[incompleteQ.pop()];

    FTRACE(2, "block #{}\n", blk->id);

    auto const locLiveOut    = blockStates[rpoId(blk)].locLiveOut;
    auto const locLiveOutExn = blockStates[rpoId(blk)].locLiveOutExn;
    auto const transfer      = blockAnalysis[rpoId(blk)];
    auto const liveIn        = transfer.locGen |
                               (locLiveOut & ~transfer.locKill) |
                               (locLiveOutExn & ~transfer.locKillExn);

    FTRACE(2, "live out : {}\n"
              "out exn  : {}\n"
              "gen      : {}\n"
              "kill     : {}\n"
              "kill exn : {}\n"
              "live in  : {}\n",
              loc_bits_string(ai.ctx.func, locLiveOut),
              loc_bits_string(ai.ctx.func, locLiveOutExn),
              loc_bits_string(ai.ctx.func, transfer.locGen),
              loc_bits_string(ai.ctx.func, transfer.locKill),
              loc_bits_string(ai.ctx.func, transfer.locKillExn),
              loc_bits_string(ai.ctx.func, liveIn));

    // Merge the liveIn into the liveOut of each normal predecessor.
    // If the set changes, reschedule that predecessor.
    for (auto& pred : normalPreds[blk->id]) {
      FTRACE(2, "  -> {}\n", pred->id);
      auto& predState = blockStates[rpoId(pred)].locLiveOut;
      auto const oldPredState = predState;
      predState |= liveIn;
      if (predState != oldPredState) {
        incompleteQ.push(rpoId(pred));
      }
    }

    // Merge the liveIn into the liveOutExn state for each exceptional
    // precessor.  The liveIn computation also depends on the
    // liveOutExn state, so again reschedule if it changes.
    for (auto& pred : factoredPreds[blk->id]) {
      FTRACE(2, "  => {}\n", pred->id);
      auto& predState = blockStates[rpoId(pred)].locLiveOutExn;
      auto const oldPredState = predState;
      predState |= liveIn;
      if (predState != oldPredState) {
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
  for (auto& b : ai.rpoBlocks) {
    FTRACE(2, "block #{}\n", b->id);
    auto const used = optimize_dce(
      index,
      ai,
      b,
      ai.bdata[b->id].stateIn,
      blockStates[rpoId(b)].locLiveOut,
      blockStates[rpoId(b)].locLiveOutExn
    );
    usedLocals |= used.first;
    usedSlots  |= used.second;
  }

  FTRACE(1, "  used locals: {}\n", loc_bits_string(ai.ctx.func, usedLocals));
  remove_unused_locals(ai.ctx, usedLocals);

  FTRACE(1, "  used slots: {}\n", slot_bits_string(ai.ctx.func, usedSlots));
  remove_unused_clsref_slots(ai.ctx, usedSlots);
}

//////////////////////////////////////////////////////////////////////

void remove_unreachable_blocks(const Index& index, const FuncAnalysis& ainfo) {
  auto reachable = [&](BlockId id) {
    return ainfo.bdata[id].stateIn.initialized;
  };

  for (auto& blk : ainfo.rpoBlocks) {
    if (reachable(blk->id)) continue;
    auto const srcLoc = blk->hhbcs.front().srcLoc;
    blk->hhbcs = {
      bc_with_loc(srcLoc, bc::String { s_unreachable.get() }),
      bc_with_loc(srcLoc, bc::Fatal { FatalOp::Runtime })
    };
    blk->fallthrough = NoBlockId;
  }

  if (!options.RemoveDeadBlocks) return;

  for (auto& blk : ainfo.rpoBlocks) {
    auto reachableTargets = false;
    forEachTakenEdge(blk->hhbcs.back(), [&] (BlockId id) {
        if (reachable(id)) reachableTargets = true;
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

namespace {

struct MergeBlockInfo {
  // This block has a predecessor; used to set the multiplePreds flag
  uint8_t hasPred          : 1;
  // Block has more than one pred, or is an entry block
  uint8_t multiplePreds    : 1;
  // Block has more than one successor
  uint8_t multipleSuccs    : 1;

  // Block contains a sequence that could be part of a switch
  uint8_t couldBeSwitch    : 1;
  // Block contains a sequence that could be part of a switch, and nothing else
  uint8_t onlySwitch       : 1;
  // Block follows the "default" of a prior switch sequence
  uint8_t followsSwitch    : 1;
};

struct SwitchInfo {
  union Case { SString s; int64_t i; };
  std::vector<std::pair<Case,BlockId>> cases;
  BlockId defaultBlock = NoBlockId;
  LocalId switchLoc    = NoLocalId;
  DataType kind;
};

bool analyzeSwitch(const php::Block& blk,
                   std::vector<MergeBlockInfo>& blkInfos,
                   SwitchInfo* switchInfo) {
  auto const jmp = &blk.hhbcs.back();
  auto& blkInfo = blkInfos[blk.id];

  switch (jmp->op) {
    case Op::JmpZ:
    case Op::JmpNZ: {
      if (blk.hhbcs.size() < 4) return false;
      auto const& cmp = jmp[-1];
      if (cmp.op != Op::Eq && cmp.op != Op::Neq) return false;
      auto check = [&] (const Bytecode& arg1, const Bytecode& arg2) -> bool {
        LocalId loc;
        if (arg2.op == Op::CGetL) {
          loc = arg2.CGetL.loc1;
        } else if (arg2.op == Op::CGetL2 && &arg2 == &arg1 + 1) {
          loc = arg2.CGetL2.loc1;
        } else {
          return false;
        }
        SwitchInfo::Case c;
        if (arg1.op == Op::Int) {
          c.i = arg1.Int.arg1;
        } else if (arg1.op == Op::String) {
          c.s = arg1.String.str1;
        } else {
          return false;
        }
        if (switchInfo) {
          auto const dt = arg1.op == Op::Int ? KindOfInt64 : KindOfString;
          if (switchInfo->cases.size()) {
            if (loc != switchInfo->switchLoc) return false;
            if (dt != switchInfo->kind) return false;
          } else {
            switchInfo->switchLoc = loc;
            switchInfo->kind = dt;
          }
        }
        auto const jmpTarget = jmp->op == Op::JmpNZ ?
          jmp->JmpNZ.target : jmp->JmpZ.target;
        BlockId caseTarget, defaultBlock;
        if ((jmp->op == Op::JmpNZ) == (cmp.op == Op::Eq)) {
          defaultBlock = blk.fallthrough;
          caseTarget = jmpTarget;
        } else {
          defaultBlock = jmpTarget;
          caseTarget = blk.fallthrough;
        }
        blkInfo.couldBeSwitch = true;
        blkInfo.onlySwitch = blk.hhbcs.size() == 4;
        blkInfos[defaultBlock].followsSwitch = true;
        if (switchInfo) {
          switchInfo->cases.emplace_back(c, caseTarget);
          switchInfo->defaultBlock = defaultBlock;
        }
        return true;
      };
      return check(jmp[-2], jmp[-3]) || check(jmp[-3], jmp[-2]);
    }
    case Op::Switch:
    case Op::SSwitch: {
      if (blk.hhbcs.size() < 2) return false;
      auto const& cgetl = jmp[-1];
      if (cgetl.op != Op::CGetL) return false;
      auto const dt = jmp->op == Op::Switch ? KindOfInt64 : KindOfString;
      if (switchInfo) {
        if (switchInfo->cases.size()) {
          if (cgetl.CGetL.loc1 != switchInfo->switchLoc) return false;
          if (dt != switchInfo->kind) return false;
        } else {
          switchInfo->switchLoc = cgetl.CGetL.loc1;
          switchInfo->kind = dt;
        }
      }
      if (jmp->op == Op::Switch) {
        if (jmp->Switch.subop1 != SwitchKind::Bounded) return false;
        auto const db = jmp->Switch.targets.back();
        auto const min = jmp->Switch.arg2;
        blkInfos[db].followsSwitch = true;
        if (switchInfo) {
          switchInfo->defaultBlock = db;
          for (size_t i = 0; i < jmp->Switch.targets.size() - 2; i++) {
            auto const t = jmp->Switch.targets[i];
            if (t == db) continue;
            SwitchInfo::Case c;
            c.i = i + min;
            switchInfo->cases.emplace_back(c, t);
          }
        }
      } else {
        auto const db = jmp->SSwitch.targets.back().second;
        blkInfos[db].followsSwitch = true;
        if (switchInfo) {
          switchInfo->defaultBlock = db;
          for (auto& kv : jmp->SSwitch.targets) {
            if (kv.second == db) continue;
            SwitchInfo::Case c;
            c.s = kv.first;
            switchInfo->cases.emplace_back(c, kv.second);
          }
        }
      }
      blkInfo.couldBeSwitch = true;
      blkInfo.onlySwitch = blk.hhbcs.size() == 2;
      return true;
    }
    default:
      return false;
  }
}

Bytecode buildIntSwitch(SwitchInfo& switchInfo) {
  auto min = switchInfo.cases[0].first.i;
  auto max = min;
  for (size_t i = 1; i < switchInfo.cases.size(); ++i) {
    auto v = switchInfo.cases[i].first.i;
    if (v < min) min = v;
    if (v > max) max = v;
  }
  if (switchInfo.cases.size() / ((double)max - (double)min + 1) < .5) {
    return { bc::Nop {} };
  }
  CompactVector<BlockId> switchTab;
  switchTab.resize(max - min + 3, switchInfo.defaultBlock);
  for (auto i = switchInfo.cases.size(); i--; ) {
    auto const& c = switchInfo.cases[i];
    switchTab[c.first.i - min] = c.second;
    if (c.first.i) switchTab[max - min + 1] = c.second;
  }
  return { bc::Switch { SwitchKind::Bounded, min, std::move(switchTab) } };
}

Bytecode buildStringSwitch(SwitchInfo& switchInfo) {
  std::set<SString> seen;
  SSwitchTab sswitchTab;
  for (auto& c : switchInfo.cases) {
    if (seen.insert(c.first.s).second) {
      sswitchTab.emplace_back(c.first.s, c.second);
    }
  }
  sswitchTab.emplace_back(nullptr, switchInfo.defaultBlock);
  return { bc::SSwitch { std::move(sswitchTab) } };
}

bool buildSwitches(php::Func& func,
                   borrowed_ptr<php::Block> blk,
                   std::vector<MergeBlockInfo>& blkInfos) {
  SwitchInfo switchInfo;
  std::vector<BlockId> blocks;
  if (!analyzeSwitch(*blk, blkInfos, &switchInfo)) return false;
  blkInfos[blk->id].couldBeSwitch = false;
  blkInfos[blk->id].onlySwitch = false;
  while (true) {
    auto const& bInfo = blkInfos[switchInfo.defaultBlock];
    auto const nxt = borrow(func.blocks[switchInfo.defaultBlock]);
    if (bInfo.onlySwitch && !bInfo.multiplePreds &&
        analyzeSwitch(*nxt, blkInfos, &switchInfo)) {
      blocks.push_back(switchInfo.defaultBlock);
      continue;
    }
    bool ret = false;
    auto const minSize = switchInfo.kind == KindOfInt64 ? 1 : 8;
    if (switchInfo.cases.size() >= minSize && blocks.size()) {
      while (is_single_nop(*func.blocks[switchInfo.defaultBlock])) {
        switchInfo.defaultBlock =
          func.blocks[switchInfo.defaultBlock]->fallthrough;
      }
      auto bc = switchInfo.kind == KindOfInt64 ?
        buildIntSwitch(switchInfo) : buildStringSwitch(switchInfo);
      if (bc.op != Op::Nop) {
        auto it = blk->hhbcs.end();
        // blk->fallthrough implies it was a JmpZ JmpNZ block,
        // which means we have exactly 4 instructions making up
        // the switch (see analyzeSwitch). Otherwise it was a
        // [S]Switch, and there were exactly two instructions.
        if (blk->fallthrough != NoBlockId) {
          it -= 4;
        } else {
          it -= 2;
        }
        blkInfos[switchInfo.defaultBlock].multiplePreds = true;
        blk->hhbcs.erase(it, blk->hhbcs.end());
        blk->hhbcs.emplace_back(bc::CGetL { switchInfo.switchLoc });
        blk->hhbcs.push_back(std::move(bc));
        blk->fallthrough = NoBlockId;
        for (auto id : blocks) {
          if (blkInfos[id].multiplePreds) continue;
          auto const removed = borrow(func.blocks[id]);
          removed->id = NoBlockId;
          removed->hhbcs = { bc::Nop {} };
          removed->fallthrough = NoBlockId;
          removed->factoredExits = {};
        }
        ret = true;
      }
    }
    return (bInfo.couldBeSwitch && buildSwitches(func, nxt, blkInfos)) || ret;
  }
}

}

bool merge_blocks(const FuncAnalysis& ainfo) {
  auto& func = *ainfo.ctx.func;
  FTRACE(2, "merge_blocks: {}\n", func.name);

  std::vector<MergeBlockInfo> blockInfo(func.blocks.size(), MergeBlockInfo {});

  auto reachable = [&](BlockId id) {
    auto const& state = ainfo.bdata[id].stateIn;
    return state.initialized && !state.unreachable;
  };
  // find all the blocks with multiple preds; they can't be merged
  // into their predecessors
  for (auto const& blk : func.blocks) {
    if (blk->id == NoBlockId) continue;
    auto& bbi = blockInfo[blk->id];
    int numSucc = 0;
    if (!reachable(blk->id)) {
      bbi.multiplePreds = true;
    } else {
      analyzeSwitch(*blk, blockInfo, nullptr);
    }
    forEachSuccessor(*blk, [&](BlockId succId) {
        auto& bsi = blockInfo[succId];
        if (bsi.hasPred) {
          bsi.multiplePreds = true;
        } else {
          bsi.hasPred = true;
        }
        numSucc++;
      });
    if (numSucc > 1) bbi.multipleSuccs = true;
  }
  blockInfo[func.mainEntry].multiplePreds = true;
  for (auto const blkId: func.dvEntries) {
    if (blkId != NoBlockId) {
      blockInfo[blkId].multiplePreds = true;
    }
  }

  bool removedAny = false;
  for (auto& blk : func.blocks) {
    if (blk->id == NoBlockId) continue;
    while (blk->fallthrough != NoBlockId) {
      auto nxt = borrow(func.blocks[blk->fallthrough]);
      if (blockInfo[blk->id].multipleSuccs ||
          blockInfo[nxt->id].multiplePreds ||
          blk->exnNode != nxt->exnNode ||
          blk->section != nxt->section) {
        break;
      }

      FTRACE(1, "merging: {} into {}\n", (void*)nxt, (void*)blk.get());
      auto& bInfo = blockInfo[blk->id];
      auto const& nInfo = blockInfo[nxt->id];
      bInfo.multipleSuccs = nInfo.multipleSuccs;
      bInfo.couldBeSwitch = nInfo.couldBeSwitch;
      bInfo.onlySwitch = false;

      blk->fallthrough = nxt->fallthrough;
      blk->fallthroughNS = nxt->fallthroughNS;
      if (nxt->factoredExits.size()) {
        if (blk->factoredExits.size()) {
          std::set<BlockId> exitSet;
          std::copy(begin(blk->factoredExits), end(blk->factoredExits),
                    std::inserter(exitSet, begin(exitSet)));
          std::copy(nxt->factoredExits.begin(), nxt->factoredExits.end(),
                    std::inserter(exitSet, begin(exitSet)));
          blk->factoredExits.resize(exitSet.size());
          std::copy(begin(exitSet), end(exitSet), blk->factoredExits.begin());
          nxt->factoredExits = decltype(nxt->factoredExits) {};
        } else {
          blk->factoredExits = std::move(nxt->factoredExits);
        }
      }
      std::copy(nxt->hhbcs.begin(), nxt->hhbcs.end(),
                std::back_inserter(blk->hhbcs));
      nxt->fallthrough = NoBlockId;
      nxt->id = NoBlockId;
      nxt->hhbcs = { bc::Nop {} };
      removedAny = true;
    }
    auto const& bInfo = blockInfo[blk->id];
    if (bInfo.couldBeSwitch &&
        (bInfo.multiplePreds || !bInfo.onlySwitch || !bInfo.followsSwitch)) {
      // This block looks like it could be part of a switch, and it's
      // not in the middle of a sequence of such blocks.
      if (buildSwitches(func, borrow(blk), blockInfo)) {
        removedAny = true;
      }
    }
  }

  return removedAny;
}

//////////////////////////////////////////////////////////////////////

}}
