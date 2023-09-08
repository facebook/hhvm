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

#include "hphp/runtime/base/array-iterator.h"

#include "hphp/util/bitset-utils.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/trace.h"

#include "hphp/hhbbc/analyze.h"
#include "hphp/hhbbc/cfg-opts.h"
#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/func-util.h"
#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/optimize.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-structure.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/unit-util.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/vanilla-dict.h"

namespace HPHP::HHBBC {

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
 *   represented in our CFG.
 *
 *   It essentially is the liveness analysis algorithm described at
 *   http://dl.acm.org/citation.cfm?id=316171, except that we don't
 *   need to track kill sets for each PEI because we don't have a
 *   means of determining which exceptional edges may be traversed by any
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

// The number of pops as seen by dce.
uint32_t numPop(const Bytecode& bc) {
  if (bc.op == Op::CGetL2) return 1;
  return bc.numPop();
}

// The number of pushes as seen by dce.
uint32_t numPush(const Bytecode& bc) {
  if (bc.op == Op::CGetL2) return 2;
  return bc.numPush();
}

// Some reads could raise warnings and run arbitrary code.
bool readCouldHaveSideEffects(const Type& t) {
  return t.couldBe(BUninit);
}

//////////////////////////////////////////////////////////////////////

/*
 * Use information of a stack cell.
 */
enum class Use {
  // Indicates that the cell is (possibly) used.
  Used = 0,

  // Indicates that the cell is (unconditionally) not used.
  Not = 1,

  /*
   * Indicates that the stack slot contains an array-like being constructed
   * by AddElemCs, which looks like it can be optimized to NewStructDict.
   */
  AddElemC = 3,

  /*
   * Modifier or-ed into the above to indicate that this use is linked
   * to the stack slot below it (in stack order - the bottom of the
   * stack is below the top); either they are both optimized, or
   * neither is.
   */
  Linked = 4,
};

Use operator|(Use a, Use b) {
  return static_cast<Use>(static_cast<int>(a) | static_cast<int>(b));
}

Use operator&(Use a, Use b) {
  return static_cast<Use>(static_cast<int>(a) & static_cast<int>(b));
}

bool any(Use a) {
  return static_cast<int>(a) != 0;
}

Use mask_use(Use a) { return a & static_cast<Use>(3); }

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

struct LocationIdHash {
  size_t operator()(const LocationId& l) const {
    return folly::hash::hash_combine(hash_int64_pair(l.blk, l.id), l.isSlot);
  }
};

bool operator==(const LocationId& i1, const LocationId& i2) {
  return i1.blk == i2.blk &&
         i1.id == i2.id &&
         i1.isSlot == i2.isSlot;
}

using LocationIdSet = hphp_fast_set<LocationId,LocationIdHash>;

struct DceAction {
  enum Action {
    Kill,
    PopInputs,
    PopOutputs,
    Replace,
    PopAndReplace,
    MinstrStackFinal,
    MinstrStackFixup,
    MinstrPushBase,
    AdjustPop,
    UnsetLocalsAfter,
    UnsetLocalsBefore,
  } action;
  using MaskOrCountType = uint32_t;
  static constexpr size_t kMaskSize = 32;
  MaskOrCountType maskOrCount{0};
  CompactVector<Bytecode> bcs{};

  DceAction() = default;
  /* implicit */ DceAction(Action a) : action{a} {}
  DceAction(Action a, MaskOrCountType m) : action{a}, maskOrCount{m} {}
  DceAction(Action a, CompactVector<Bytecode> bcs)
    : action{a}
    , bcs{std::move(bcs)}
  {}
};

using DceActionMap = std::map<InstrId, DceAction>;

struct UseInfo {
  explicit UseInfo(Use u) : usage(u) {}
  UseInfo(Use u, DceActionMap&& ids) : usage(u), actions(std::move(ids)) {}

  Use usage;
  /*
   * Set of actions that should be performed if we decide to
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

struct LocalRemappingIndex {
  // `mrl` is the maximum number of locals we will remap.
  explicit LocalRemappingIndex(size_t mrl) : localInterference(mrl) {
    assertx(mrl <= kMaxTrackedLocals);
  }
  // localInterference[i][j] == true iff the locals i and j are both live at a
  // program point, and therefore cannot share a local slot.
  std::vector<std::bitset<kMaxTrackedLocals>> localInterference;
  // pinnedLocal[i] == true iff the local i depends on its order in the local
  // slots.  This is for keeping local ranges intact for bytecodes that take
  // such immediates.
  std::bitset<kMaxTrackedLocals> pinnedLocals{};
};

//////////////////////////////////////////////////////////////////////

struct DceState {
  explicit DceState(const Index& index, const FuncAnalysis& ainfo) :
      index(index), ainfo(ainfo) {}
  const Index& index;
  const FuncAnalysis& ainfo;
  /*
   * Used to accumulate a set of blk/stack-slot pairs that
   * should be marked used.
   */
  LocationIdSet forcedLiveLocations;

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
   * Instructions marked in this set will be processed by dce_perform.
   * They must all be processed together to keep eval stack consumers
   * and producers balanced.
   */
  DceActionMap actionMap;

  /*
   * Locals that are unset at the next possible offset.
   *
   * At a given instruction during the DCE analysis a local "will be unset" if
   * the local is currently dead, and is unset before any control flow or
   * opcode other than member base, member dim and unset ops.
   */
  std::bitset<kMaxTrackedLocals> willBeUnsetLocals;

  /*
   * Locals that may need to be unset in successor blocks are accumlated into
   * this bitset.
   *
   * A successor may need to unset a local for us if we are unable to because
   * the UnsetL would be placed in an illegal position (after control flow ops).
   */
  std::bitset<kMaxTrackedLocals> mayNeedUnsetting;
  std::bitset<kMaxTrackedLocals> mayNeedUnsettingExn;

  /*
   * The set of local names that were ever referenced in this block.  This set
   * is used by global DCE to remove local names that are completely unused
   * in the entire function.
   */
  std::bitset<kMaxTrackedLocals> usedLocalNames;

  /*
   * Interference graph between local slots we build up over the course of
   * Global DCE and then use to share local slots to reduce frame size.
   */
  LocalRemappingIndex* remappingIndex{nullptr};

  /*
   * Can be set by a minstr-final instruction to indicate the actions
   * to take provided all previous minstrs in the group are non-pei.
   *
   * eg if the result of a QueryM is unused, and the whole sequence is
   * non-pei we can kill it. Or if the result is a literal, and the
   * whole sequence is non-pei, we can replace it with pops plus the
   * constant.
   */
  Optional<UseInfo> minstrUI;
  /*
   * Flag to indicate local vs global dce.
   */
  bool isLocal{false};

  /*
   * When we do add opts, in some cases it can enable further optimization
   * opportunities. Let the optimizer know...
   */
  bool didAddOpts{false};
};

//////////////////////////////////////////////////////////////////////
// debugging

const char* show(DceAction action) {
  switch (action.action) {
    case DceAction::Kill:             return "Kill";
    case DceAction::PopInputs:        return "PopInputs";
    case DceAction::PopOutputs:       return "PopOutputs";
    case DceAction::Replace:          return "Replace";
    case DceAction::PopAndReplace:    return "PopAndReplace";
    case DceAction::MinstrStackFinal: return "MinstrStackFinal";
    case DceAction::MinstrStackFixup: return "MinstrStackFixup";
    case DceAction::MinstrPushBase:   return "MinstrPushBase";
    case DceAction::AdjustPop:        return "AdjustPop";
    case DceAction::UnsetLocalsAfter: return "UnsetLocalsAfter";
    case DceAction::UnsetLocalsBefore:return "UnsetLocalsBefore";
  };
  not_reached();
}

std::string show(InstrId id) {
  return folly::sformat("{}:{}", id.blk, id.idx);
}

inline void validate(Use u) {
  assertx(!any(u & Use::Linked) || mask_use(u) == Use::Not);
}

const char* show(Use u) {
  validate(u);
  auto ret = [&] {
    switch (mask_use(u)) {
      case Use::Used:          return "*U";
      case Use::Not:           return "*0";
      case Use::AddElemC:      return "*AE";
      case Use::Linked: not_reached();
    }
    not_reached();
  }();

  return !any(u & Use::Linked) ? ret + 1 : ret;
}

std::string show(const LocationId& id) {
  return folly::sformat("{}:{}{}", id.blk, id.id, id.isSlot ? "(slot)" : "");
}

std::string show(const DceActionMap::value_type& elm) {
  return folly::sformat("{}={}", show(elm.first), show(elm.second));
}

std::string show(const DceActionMap& actions) {
  using namespace folly::gen;
  return from(actions)
    | map([](const DceActionMap::value_type& elm) { return show(elm); })
    | unsplit<std::string>(";")
    ;
}

std::string DEBUG_ONLY show(const UseInfo& ui) {
  return folly::sformat("{}({})", show(ui.usage), show(ui.actions));
}

std::string DEBUG_ONLY loc_bits_string(
    const php::Func* func,
    const std::bitset<kMaxTrackedLocals>& locs) {
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

//////////////////////////////////////////////////////////////////////

struct Env {
  DceState& dceState;
  const Bytecode& op;
  InstrId id;
  LocalId loc;
  const PropagatedStates& states;
  /*
   * A local is "liveInside" an op if the local is used in the op, but is not
   * live before or after the op.  This is used to ensure the local slot is
   * usable during the op unless the op is DCEd away.  When building an
   * interference graph it is important to know which instruction has the op
   * "live inside", vs just knowing the local is used at some point.
   *
   * An example of such an op is UnsetL.  It stores to a local slot, but its
   * behavior is unaltered by the contents of the local slot.  So we need the
   * local slot to exist, but don't need it to exist before or after the Op.
   */
  std::bitset<kMaxTrackedLocals> liveInside;
};

//////////////////////////////////////////////////////////////////////
// Properties of UseInfo

bool isLinked(const UseInfo& ui) {
  return any(ui.usage & Use::Linked);
}

bool lastUiIsLinked(const UseInfo& ui) {
  return isLinked(ui);
}

template <typename... Args>
bool lastUiIsLinked(const UseInfo& /*ui*/, const Args&... args) {
  return lastUiIsLinked(args...);
}

bool allUnused() { return true; }
template<class... Args>
bool allUnused(const UseInfo& ui, const Args&... args) {
  return (mask_use(ui.usage)) == Use::Not &&
    allUnused(args...);
}

bool allUnusedIfNotLastRef() { return true; }
template<class... Args>
bool allUnusedIfNotLastRef(const UseInfo& ui, const Args&... args) {
  auto u = mask_use(ui.usage);
  return u == Use::Not && allUnusedIfNotLastRef(args...);
}

bool alwaysPop(const UseInfo& ui) {
  return
    ui.location.blk != NoBlockId ||
    ui.actions.size() > 1;
}

template<typename... Args>
bool alwaysPop(const UseInfo& ui, const Args&... args) {
  return alwaysPop(ui) || alwaysPop(args...);
}

bool maybePop(Env& env, const UseInfo& ui) {
  return
    (ui.actions.size() == 1 &&
     ui.actions.begin()->first.idx > env.id.idx + 1);
}

template<typename... Args>
bool maybePop(Env& env, const UseInfo& ui, const Args&... args) {
  return maybePop(env, ui) || maybePop(env, args...);
}

/*
 * Determine whether its worth inserting PopCs after an instruction
 * that can't be dced in order to execute the dependent actions.
 */
template<typename... Args>
bool shouldPopOutputs(Env& env, const Args&... args) {
  if (alwaysPop(args...)) return true;
  return maybePop(env, args...);
}

//////////////////////////////////////////////////////////////////////
// query eval stack

Type topT(Env& env, uint32_t idx = 0) {
  assertx(idx < env.states.stack().size());
  return env.states.stack()[env.states.stack().size() - idx - 1];
}

Type topC(Env& env, uint32_t idx = 0) {
  auto const t = topT(env, idx);
  assertx(t.subtypeOf(BInitCell));
  return t;
}

//////////////////////////////////////////////////////////////////////
// locals
void addInterference(LocalRemappingIndex* index,
                     const std::bitset<kMaxTrackedLocals>& live) {
  auto& li = index->localInterference;
  for (auto i = li.size(); i-- > 0;) {
    if (live[i]) {
      li[i] |= live;
    }
  }
}

void addInterference(Env& env, const std::bitset<kMaxTrackedLocals>& live) {
  // We don't track interfrence until the optimize round of the global dce.
  if (!env.dceState.remappingIndex) return;
  addInterference(env.dceState.remappingIndex, live);
}

void pinLocals(Env& env, const std::bitset<kMaxTrackedLocals>& pinned) {
  // We mark pinned locals to guarantee their index does not change during
  // remapping.
  if (!env.dceState.remappingIndex) return;

  env.dceState.remappingIndex->pinnedLocals |= pinned;
}


void addLocGenSet(Env& env, const std::bitset<kMaxTrackedLocals>& locs) {
  FTRACE(4, "      loc-conservative: {}\n",
         loc_bits_string(env.dceState.ainfo.ctx.func, locs));
  env.dceState.liveLocals |= locs;
}

void addLocGen(Env& env, uint32_t id) {
  FTRACE(2, "      loc-gen: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 1;
}

/*
 * This is marking an op that logically kills a local (like a def), but we
 * cannot remove the write to the slot, so we mark it as used so the local is
 * not killed and removed.
 */
void addLocUse(Env& env, uint32_t id) {
  FTRACE(2, "      loc-use: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.liveInside[id] = 1;
}

void addLocNameUse(Env& env, NamedLocal nloc) {
  if (nloc.name >= kMaxTrackedLocals || nloc.name < 0) return;
  env.dceState.usedLocalNames[nloc.name] = 1;
}

/*
 * Indicate that this instruction will use the value of loc unless we
 * kill it. handle_push will take care of calling addLocGen if
 * appropriate.
 */
void scheduleGenLoc(Env& env, LocalId loc) {
  env.loc = loc;
}

void addLocKill(Env& env, uint32_t id) {
  FTRACE(2, "     loc-kill: {}\n", id);
  if (id >= kMaxTrackedLocals) return;
  env.dceState.liveLocals[id] = 0;
  // Logically unless this op is markedDead we need the local to exist.
  env.liveInside[id] = 1;
}

bool isLocLive(Env& env, uint32_t id) {
  if (id >= kMaxTrackedLocals) {
    // Conservatively assume it's potentially live.
    return true;
  }
  return env.dceState.liveLocals[id];
}

Type locRaw(Env& env, LocalId loc) {
  assertx(loc < env.states.locals().size());
  return env.states.locals()[loc];
}

bool isLocVolatile(Env& env, uint32_t id) {
  if (id >= kMaxTrackedLocals) return true;
  return is_volatile_local(env.dceState.ainfo.ctx.func, id);
}

//////////////////////////////////////////////////////////////////////
// manipulate eval stack

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

void discard(Env& env) {
  pop(env, Use::Not, env.id);
}

//////////////////////////////////////////////////////////////////////

/*
 * Mark a UseInfo used; if its linked also mark the ui below it used.
 * As we mark them used, we may also need to add them to
 * forcedLiveLocations to prevent inconsistencies in the global state
 * (see markUisLive).
 */
void use(LocationIdSet& forcedLive,
         std::vector<UseInfo>& uis, uint32_t i) {
  while (true) {
    auto& ui = uis[i];
    auto linked = isLinked(ui);
    if (ui.usage != Use::Used &&
        ui.location.blk != NoBlockId) {
      forcedLive.insert(ui.location);
    }
    ui.usage = Use::Used;
    ui.actions.clear();
    if (!linked) break;
    assertx(i);
    i--;
  }
}

void combineActions(DceActionMap& dstMap, DceActionMap srcMap) {
  if (srcMap.empty())  return;
  if (dstMap.empty()) {
    dstMap = std::move(srcMap);
    return;
  }

  for (auto& srcElm : srcMap) {
    // Ideally we could use the try_emplace operation once we can rely on c++17.
    auto dstIt = dstMap.find(srcElm.first);
    if (dstIt == dstMap.end()) {
      dstMap.emplace(std::move(srcElm));
      continue;
    }

    auto& src = srcElm.second;
    auto& dst = dstIt->second;

    if (src.action == DceAction::UnsetLocalsBefore ||
        src.action == DceAction::UnsetLocalsAfter) {
      continue;
    }

    if (dst.action == DceAction::UnsetLocalsBefore ||
        dst.action == DceAction::UnsetLocalsAfter) {
      dst = std::move(src);
      continue;
    }

    if (src.action == DceAction::MinstrStackFixup ||
        src.action == DceAction::MinstrStackFinal) {
      assertx(src.action == dst.action);
      dst.maskOrCount |= src.maskOrCount;
      continue;
    }

    if (src.action == DceAction::AdjustPop &&
        dst.action == DceAction::AdjustPop) {
      assertx(dst.maskOrCount > 0);
      assertx(src.maskOrCount > 0);
      dst.maskOrCount += src.maskOrCount;
      continue;
    }

    if (src.action == dst.action) {
      continue;
    }

    assertx(src.action == DceAction::Kill ||
            dst.action == DceAction::Kill);

    if (src.action == DceAction::PopAndReplace ||
        dst.action == DceAction::PopAndReplace) {
      dst.action = DceAction::Replace;
      continue;
    }
    if (src.action == DceAction::Replace ||
        dst.action == DceAction::Replace) {
      dst.action = DceAction::Replace;
      continue;
    }
    if (dst.action == DceAction::PopInputs ||
        src.action == DceAction::PopInputs) {
      dst.action = DceAction::Kill;
      continue;
    }
    if (dst.action == DceAction::AdjustPop ||
        src.action == DceAction::AdjustPop) {
      dst.action = DceAction::Kill;
      dst.maskOrCount = 0;
      continue;
    }

    always_assert(false);
  }
}

/*
 * A UseInfo has been popped, and we want to perform its actions. If
 * its not linked we'll just add them to the dceState; otherwise we'll
 * add them to the UseInfo on the top of the stack.
 */
DceActionMap& commitActions(Env& env, bool linked, const DceActionMap& am) {
  if (!linked) {
    FTRACE(2, "     committing {}: {}\n", show(env.id), show(am));
  }

  assertx(!linked || env.dceState.stack.back().usage != Use::Used);

  auto& dst = linked ?
    env.dceState.stack.back().actions : env.dceState.actionMap;

  combineActions(dst, am);

  if (!am.count(env.id)) {
    dst.emplace(env.id, DceAction::Kill);
  }
  return dst;
}

/*
 * Combine a set of UseInfos into the first, and return the combined one.
 */
UseInfo& combineUis(UseInfo& ui) { return ui; }
template<class... Args>
UseInfo& combineUis(UseInfo& accum, const UseInfo& ui, const Args&... args) {
  accum.actions.insert(begin(ui.actions), end(ui.actions));
  if (accum.location.blk == NoBlockId ||
      accum.location < ui.location) {
    accum.location = ui.location;
  }
  return combineUis(accum, args...);
}

/*
 * The current instruction is going to be replaced with PopCs. Perform
 * appropriate pops (Use::Not)
 */
void ignoreInputs(Env& env, bool linked, DceActionMap&& actions) {
  auto const np = numPop(env.op);
  if (!np) return;

  auto usage = [&] (uint32_t i) {
    auto ret = Use::Not;
    if (linked) ret = ret | Use::Linked;
    return ret;
  };

  for (auto i = np; --i; ) {
    pop(env, usage(i), DceActionMap {});
    linked = true;
  }
  pop(env, usage(0), std::move(actions));
}

DceActionMap& commitUis(Env& env, bool linked, const UseInfo& ui) {
  return commitActions(env, linked, ui.actions);
}

template<typename... Args>
DceActionMap& commitUis(Env& env, bool linked,
                        const UseInfo& ui, const Args&... args) {
  commitUis(env, linked, ui);
  return commitUis(env, linked, args...);
}

/*
 * During global dce, a particular stack element could be pushed
 * on multiple paths. eg:
 *
 *   $a ? f() : 42
 *
 * If f() is known to return a non-counted type, we have FCallFuncD -> PopC
 * on one path, and Int 42 -> PopC on another, and the PopC marks its value
 * Use::Not. When we get to the Int 42 it thinks both instructions can be
 * killed; but when we get to the FCallFuncD it does nothing. So any time we
 * decide to ignore a Use::Not, we have to record that fact so we can prevent
 * the other paths from trying to use that information. We communicate this
 * via the ui.location field, and the forcedLiveLocations set.
 *
 * [ We deal with this case now by inserting a PopC after the
 *   FCallFuncD, which allows the 42/PopC to be removed - but there are
 *   other cases that are not yet handled. ]
 */
void markUisLive(Env& env, bool linked, const UseInfo& ui) {
  validate(ui.usage);
  if (ui.usage != Use::Used &&
      ui.location.blk != NoBlockId) {
    env.dceState.forcedLiveLocations.insert(ui.location);
  }
  if (linked) {
    use(env.dceState.forcedLiveLocations,
        env.dceState.stack, env.dceState.stack.size() - 1);
  }
}

template<typename... Args>
void markUisLive(Env& env, bool linked,
                 const UseInfo& ui, const Args&... args) {
  markUisLive(env, false, ui);
  markUisLive(env, linked, args...);
}

template<typename... Args>
void popOutputs(Env& env, bool linked, const Args&... args) {
  if (shouldPopOutputs(env, args...)) {
    auto& actions = commitUis(env, linked, args...);
    actions[env.id] = DceAction::PopOutputs;
    return;
  }
  markUisLive(env, linked, args...);
}

void markDead(Env& env) {
  env.dceState.actionMap[env.id] = DceAction::Kill;
  FTRACE(2, "     Killing {}\n", show(env.id));
}

void pop_inputs(Env& env, uint32_t numPop) {
  for (auto i = uint32_t{0}; i < numPop; ++i) {
    pop(env);
  }
}

enum class PushFlags {
  MarkLive,
  MarkDead,
  MarkUnused,
  PopOutputs,
  AddElemC
};

template<typename... Args>
void handle_push(Env& env, PushFlags pf, Args&... uis) {
  auto linked = lastUiIsLinked(uis...);

  if (env.loc != NoLocalId && (linked ||
                               pf == PushFlags::MarkLive ||
                               pf == PushFlags::PopOutputs)) {
    addLocGen(env, env.loc);
  }

  switch (pf) {
    case PushFlags::MarkLive:
      markUisLive(env, linked, uis...);
      break;

    case PushFlags::MarkDead:
      commitUis(env, linked, uis...);
      break;

    case PushFlags::MarkUnused: {
      // Our outputs are unused, their consumers are being removed,
      // and we don't care about our inputs. Replace with
      // appropriately unused pops of the inputs.
      auto& ui = combineUis(uis...);
      // use emplace so that the callee can override the action
      ui.actions.emplace(env.id, DceAction::PopInputs);
      commitActions(env, linked, ui.actions);
      if (numPop(env.op)) {
        ignoreInputs(env, linked, {{ env.id, DceAction::Kill }});
      }
      return;
    }

    case PushFlags::PopOutputs:
      popOutputs(env, linked, uis...);
      break;

    case PushFlags::AddElemC: {
      assertx(!linked);
      auto& ui = combineUis(uis...);
      ui.usage = Use::AddElemC;
      // For the last AddElemC, we will already have added a Replace
      // action for env.id - so the following emplace will silently
      // fail; for the rest, we just want to kill the AddElemC
      ui.actions.emplace(env.id, DceAction::Kill);
      pop(env, std::move(ui));

      // The key is known; we must drop it if we convert to New*Array.
      pop(env, Use::Not | Use::Linked, DceActionMap {});

      // The value going into the array is a normal use.
      pop(env);
      return;
    }
  }
  pop_inputs(env, numPop(env.op));
}

template<typename F>
auto stack_ops(Env& env, F fun)
  -> decltype(fun(std::declval<UseInfo&>()),void(0)) {
  always_assert(!env.dceState.stack.empty());
  auto ui = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  FTRACE(2, "      stack_ops({}@{})\n", show(ui.usage), show(ui.actions));
  env.loc = NoLocalId;
  auto const f = fun(ui);
  handle_push(env, f, ui);
}

void stack_ops(Env& env) {
  stack_ops(env, [](const UseInfo&) { return PushFlags::MarkLive; });
}

template<typename F>
auto stack_ops(Env& env, F fun)
  -> decltype(fun(std::declval<UseInfo&>(), std::declval<UseInfo&>()),void(0)) {
  always_assert(env.dceState.stack.size() >= 2);
  auto u1 = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  auto u2 = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  FTRACE(2, "      stack_ops({}@{},{}@{})\n",
         show(u1.usage), show(u1.actions), show(u1.usage), show(u2.actions));
  env.loc = NoLocalId;
  auto const f = fun(u1, u2);
  handle_push(env, f, u1, u2);
}

void push_outputs(Env& env, uint32_t numPush) {
  for (auto i = uint32_t{0}; i < numPush; ++i) {
    auto ui = std::move(env.dceState.stack.back());
    env.dceState.stack.pop_back();
    markUisLive(env, isLinked(ui), ui);
  }
}

void pushRemovable(Env& env) {
  stack_ops(env,[] (const UseInfo& ui) {
      return allUnused(ui) ? PushFlags::MarkUnused : PushFlags::MarkLive;
    });
}

void pushRemovableIfNoThrow(Env& env) {
  stack_ops(env,[&] (const UseInfo& ui) {
    return !env.states.wasPEI() && allUnused(ui)
             ? PushFlags::MarkUnused : PushFlags::MarkLive;
    });
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

void dce(Env& env, const bc::PopC&)          { discard(env); }
void dce(Env& env, const bc::PopU&)          { discard(env); }
void dce(Env& env, const bc::PopU2&)         {
  auto ui = std::move(env.dceState.stack.back());
  env.dceState.stack.pop_back();
  if (isLinked(ui)) {
    // this is way too conservative; but hopefully the PopU2 will be
    // killed (it always should be) and we'll do another pass and fix
    // it.
    markUisLive(env, true, ui);
    ui = UseInfo { Use::Used };
  } else {
    ui.actions.emplace(env.id, DceAction { DceAction::AdjustPop, 1 });
  }
  discard(env);
  env.dceState.stack.push_back(std::move(ui));
}

void dce(Env& env, const bc::Int&)           { pushRemovable(env); }
void dce(Env& env, const bc::String&)        { pushRemovable(env); }
void dce(Env& env, const bc::LazyClass&)     { pushRemovable(env); }
void dce(Env& env, const bc::EnumClassLabel&){ pushRemovable(env); }
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
void dce(Env& env, const bc::FuncCred&)      { pushRemovable(env); }
void dce(Env& env, const bc::NewCol&)        { pushRemovable(env); }
void dce(Env& env, const bc::CheckProp&)     { pushRemovable(env); }

void dce(Env& env, const bc::Dup&) {
  // Various cases:
  //   - If the duplicated value is not used, delete the dup and all
  //     its consumers, then
  //     o If the original value is unused pass that on to let the
  //       instruction which pushed it decide whether to kill it
  //     o Otherwise we're done.
  //   - Otherwise, if the original value is unused, kill the dup
  //     and all dependent instructions.
  stack_ops(env, [&] (UseInfo& dup, UseInfo& orig) {
      // Dup pushes a cell that is guaranteed to be not the last reference.
      // So, it can be eliminated if the cell it pushes is used as Use::Not.
      auto const dup_unused = allUnusedIfNotLastRef(dup);
      auto const orig_unused = allUnused(orig) &&
        (!isLinked(dup) || dup_unused);

      if (dup_unused && orig_unused) {
        return PushFlags::MarkUnused;
      }

      if (dup_unused) {
        markUisLive(env, isLinked(orig), orig);
        orig.actions.clear();
        return PushFlags::MarkDead;
      }

      if (orig_unused) {
        markUisLive(env, false, dup);
        dup.actions.clear();
        return PushFlags::MarkDead;
      }

      return PushFlags::MarkLive;
    });
}

void cgetImpl(Env& env, LocalId loc, bool quiet) {
  stack_ops(env, [&] (UseInfo& ui) {
      scheduleGenLoc(env, loc);
      if (allUnused(ui) &&
          (quiet || !readCouldHaveSideEffects(locRaw(env, loc)))) {
        return PushFlags::MarkUnused;
      }
      if (!isLocLive(env, loc) &&
          !readCouldHaveSideEffects(locRaw(env, loc)) &&
          !isLocVolatile(env, loc)) {
        // note: PushL does not deal with Uninit, so we need the
        // readCouldHaveSideEffects here, regardless of quiet.
        env.dceState.actionMap.emplace(env.id, DceAction(
          DceAction::Replace,
          { bc::PushL { loc } }
        ));
      }
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::CGetL& op) {
  addLocNameUse(env, op.nloc1);
  cgetImpl(env, op.nloc1.id, false);
}

void dce(Env& env, const bc::CGetQuietL& op) {
  cgetImpl(env, op.loc1, true);
}

void dce(Env& env, const bc::CUGetL& op) {
  cgetImpl(env, op.loc1, true);
}

void dce(Env& env, const bc::PushL& op) {
  stack_ops(env, [&] (UseInfo& ui) {
      scheduleGenLoc(env, op.loc1);
      if (allUnused(ui)) {
        if (isLocLive(env, op.loc1)) {
          ui.actions.emplace(env.id, DceAction(
            DceAction::Replace,
            { bc::UnsetL { op.loc1 } }
          ));
        }
        return PushFlags::MarkUnused;
      }
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::CGetL2& op) {
  addLocNameUse(env, op.nloc1);
  auto const ty = locRaw(env, op.nloc1.id);

  stack_ops(env, [&] (const UseInfo& u1, const UseInfo& u2) {
      scheduleGenLoc(env, op.nloc1.id);
      if (readCouldHaveSideEffects(ty) || !allUnused(u1, u2)) {
        return PushFlags::MarkLive;
      }
      return PushFlags::MarkUnused;
    });
}

void dce(Env& env, const bc::BareThis& op) {
  stack_ops(env, [&] (UseInfo& ui) {
      if (allUnusedIfNotLastRef(ui) &&
          op.subop1 != BareThisOp::Notice) {
        return PushFlags::MarkUnused;
      }
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::RetC&)  { pop(env); }
void dce(Env& env, const bc::RetCSuspended&) { pop(env); }
void dce(Env& env, const bc::RetM& op) {
  for (int i =0; i < op.arg1; i++) {
    pop(env);
  }
}
void dce(Env& env, const bc::Throw&) { pop(env); }
void dce(Env& env, const bc::Fatal&) { pop(env); }
void dce(Env& env, const bc::Exit&)  { stack_ops(env); }

void dce(Env& env, const bc::IsTypeC& op) {
  stack_ops(env, [&] (UseInfo& ui) {
      if (allUnused(ui) &&
          !is_type_might_raise(op.subop1, topC(env))) {
        return PushFlags::MarkUnused;
      }
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::IsTypeL& op) {
  addLocNameUse(env, op.nloc1);
  auto const ty = locRaw(env, op.nloc1.id);
  stack_ops(env, [&] (UseInfo& ui) {
      scheduleGenLoc(env, op.nloc1.id);
      if (allUnused(ui) &&
          !readCouldHaveSideEffects(ty) &&
          !is_type_might_raise(op.subop2, ty)) {
        return PushFlags::MarkUnused;
      }
      return PushFlags::MarkLive;
    });
}

void updateSrcLocForAddElemC(UseInfo& ui, int32_t srcLoc) {
  assertx(ui.usage == Use::AddElemC);
  assertx(!ui.actions.empty());

  for (auto& it : ui.actions) {
    if (it.second.bcs.size() != 1) continue;
    if (it.second.action != DceAction::Replace) continue;

    auto& bc = it.second.bcs[0];
    if (bc.op == Op::NewStructDict) {
      bc.srcLoc = srcLoc;
    }
  }
}

void dce(Env& env, const bc::NewDictArray&) {
  stack_ops(env, [&] (UseInfo& ui) {
      if (ui.usage == Use::AddElemC || allUnused(ui)) {
        if (ui.usage == Use::AddElemC) {
          updateSrcLocForAddElemC(ui, env.op.srcLoc);
        }
        env.dceState.didAddOpts  = true;
        return PushFlags::MarkUnused;
      }

      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::AddElemC& /*op*/) {
  assertx(env.states.lastPush().has_value());

  stack_ops(env, [&] (UseInfo& ui) {
      // If the set might throw it needs to be kept.
      if (env.states.wasPEI()) {
        return PushFlags::MarkLive;
      }

      if (allUnused(ui)) {
        return PushFlags::MarkUnused;
      }

      if (env.dceState.isLocal) {
        // Converting to New*Array changes the stack layout,
        // which can invalidate the FuncAnalysis; only do it in global
        // dce, since we're going to recompute the FuncAnalysis anyway.
        return PushFlags::MarkLive;
      }
      auto const& arrPost = *env.states.lastPush();
      auto const& arrPre = topC(env, 2);
      auto const postSize = arr_size(arrPost);
      if (!postSize || postSize == arr_size(arrPre)) {
        // if postSize is known, and equal to preSize, the AddElemC
        // didn't add an element (duplicate key) so we have to give up.
        // if its not known, we also have nothing to do.
        return PushFlags::MarkLive;
      }
      if (ui.usage == Use::AddElemC) {
        return PushFlags::AddElemC;
      }
      auto const cat = categorize_array(arrPost);
      if (cat.hasValue) {
        if (allUnusedIfNotLastRef(ui)) return PushFlags::MarkUnused;
        auto v = tv(arrPost);
        CompactVector<Bytecode> bcs;
        assertx(arrPost.subtypeOf(BDictN));
        bcs.emplace_back(bc::Dict { v->m_data.parr });
        ui.actions[env.id] = DceAction(DceAction::PopAndReplace,
                                       std::move(bcs));
        return PushFlags::MarkUnused;
      }

      if (isLinked(ui)) return PushFlags::MarkLive;

      if (arrPost.strictSubtypeOf(BDictN) &&
          cat.cat == Type::ArrayCat::Struct &&
          *postSize <= ArrayData::MaxElemsOnStack) {
        CompactVector<Bytecode> bcs;
        bcs.emplace_back(bc::NewStructDict { get_string_keys(arrPost) });
        ui.actions[env.id] = DceAction(DceAction::Replace, std::move(bcs));
        return PushFlags::AddElemC;
      }

      return PushFlags::MarkLive;
    });
}

template<typename Op>
void dceNewArrayLike(Env& env, const Op& op) {
  if (op.numPop() == 1 &&
      !env.states.wasPEI() &&
      allUnusedIfNotLastRef(env.dceState.stack.back())) {
    // Just an optimization for when we have a single element array,
    // but we care about its lifetime. By killing the array, and
    // leaving its element on the stack, the lifetime is unaffected.
    return markDead(env);
  }
  pushRemovableIfNoThrow(env);
}

void dce(Env& env, const bc::NewStructDict& op)   { dceNewArrayLike(env, op); }
void dce(Env& env, const bc::NewVec& op)          { dceNewArrayLike(env, op); }
void dce(Env& env, const bc::NewKeysetArray& op)  { dceNewArrayLike(env, op); }

void dce(Env& env, const bc::NewPair& op)         { dceNewArrayLike(env, op); }
void dce(Env& env, const bc::ColFromArray& op)    { dceNewArrayLike(env, op); }

void dce(Env& env, const bc::PopL& op) {
  if (!isLocLive(env, op.loc1) && !isLocVolatile(env, op.loc1)) {
    discard(env);
    env.dceState.actionMap[env.id] = DceAction::PopInputs;
    return;
  }
  pop(env);
  if (isLocVolatile(env, op.loc1)) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

void dce(Env& env, const bc::SetL& op) {
  if (!isLocLive(env, op.loc1) && !isLocVolatile(env, op.loc1)) {
    return markDead(env);
  }
  stack_ops(env, [&] (UseInfo& ui) {
    if (!allUnusedIfNotLastRef(ui)) return PushFlags::MarkLive;
    // If the stack result of the SetL is unused, we can replace it
    // with a PopL.
    CompactVector<Bytecode> bcs { bc::PopL { op.loc1 } };
    ui.actions[env.id] = DceAction(DceAction::Replace, std::move(bcs));
    return PushFlags::MarkDead;
  });
  if (isLocVolatile(env, op.loc1)) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

void dce(Env& env, const bc::UnsetL& op) {
  auto const oldTy = locRaw(env, op.loc1);
  if (oldTy.subtypeOf(BUninit)) return markDead(env);

  auto const couldFreeHeapObj = !oldTy.subtypeOf(TUnc);
  auto const effects = isLocVolatile(env, op.loc1);
  if (!isLocLive(env, op.loc1) && !effects && !couldFreeHeapObj) {
    return markDead(env);
  }
  if (effects) {
    addLocGen(env, op.loc1);
  } else {
    addLocKill(env, op.loc1);
  }
}

/*
 * IncDecL is a read-modify-write: can be removed if the local isn't
 * live, the set can't have side effects, and no one reads the value
 * it pushes.  If the instruction is not dead, add the local to the
 * set of upward exposed uses.
 */
void dce(Env& env, const bc::IncDecL& op) {
  addLocNameUse(env, op.nloc1);
  auto const oldTy   = locRaw(env, op.nloc1.id);
  auto const effects = readCouldHaveSideEffects(oldTy) ||
                       isLocVolatile(env, op.nloc1.id);
  stack_ops(env, [&] (const UseInfo& ui) {
      scheduleGenLoc(env, op.nloc1.id);
      if (!isLocLive(env, op.nloc1.id) && !effects && allUnused(ui)) {
        return PushFlags::MarkUnused;
      }
      return PushFlags::MarkLive;
    });
}

bool setOpLSideEffects(const bc::SetOpL& op, const Type& lhs, const Type& rhs) {
  auto const lhsOk = lhs.subtypeOf(BInt | BDbl | BStr);
  auto const rhsOk = rhs.subtypeOf(BInt | BDbl | BStr);
  if (!lhsOk || !rhsOk) return true;

  switch (op.subop2) {
    case SetOpOp::ConcatEqual:
      return !lhs.subtypeOf(BArrKey) || !rhs.subtypeOf(BArrKey);

    case SetOpOp::AndEqual:
    case SetOpOp::OrEqual:
    case SetOpOp::XorEqual:
    case SetOpOp::SlEqual:
    case SetOpOp::SrEqual:
      return !lhs.subtypeOf(BInt) || !rhs.subtypeOf(BInt);

    case SetOpOp::PlusEqual:
    case SetOpOp::MinusEqual:
    case SetOpOp::MulEqual:
    case SetOpOp::DivEqual:
    case SetOpOp::ModEqual:
    case SetOpOp::PowEqual:
      return lhs.subtypeOf(BStr) || rhs.subtypeOf(BStr);
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
  auto const oldTy = locRaw(env, op.loc1);

  stack_ops(env, [&] (UseInfo& ui) {
      scheduleGenLoc(env, op.loc1);
      if (!isLocLive(env, op.loc1) && allUnused(ui)) {
        if (!readCouldHaveSideEffects(oldTy) &&
            !isLocVolatile(env, op.loc1) &&
            !setOpLSideEffects(op, oldTy, topC(env))) {
          return PushFlags::MarkUnused;
        }
      }
      return PushFlags::MarkLive;
    });
}

void dce(Env& env, const bc::Add&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::AddNewElemC&)      { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::AKExists&)         { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ArrayIdx&)         { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ArrayMarkLegacy&)  { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ArrayUnmarkLegacy&){ pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::BitAnd&)           { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::BitNot&)           { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::BitOr&)            { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::BitXor&)           { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastBool&)         { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastDict&)         { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastDouble&)       { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastInt&)          { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastKeyset&)       { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastString&)       { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CastVec&)          { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CGetS&)            { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ClassHasReifiedGenerics&) {
  pushRemovableIfNoThrow(env);
}
void dce(Env& env, const bc::Cmp&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CombineAndResolveTypeStruct&) {
  pushRemovableIfNoThrow(env);
}
void dce(Env& env, const bc::Concat&)           { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ConcatN&)          { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CreateCl&)         { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::DblAsBits&)        { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Div&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Eq&)               { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Gt&)               { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Gte&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::HasReifiedParent&) { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Idx&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::IsLateBoundCls&)   { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::IssetS&)           { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::IsTypeStructC&)    { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Lt&)               { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Lte&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Mod&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Mul&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Neq&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Not&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::NSame&)            { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Pow&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Same&)             { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Shl&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Shr&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::Sub&)              { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::LateBoundCls&)     { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::SelfCls&)          { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ParentCls&)        { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ClassName&)        { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::LazyClassFromClass&) {
  pushRemovableIfNoThrow(env);
}
void dce(Env& env, const bc::ClassGetC&)        { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::ResolveClass&)     { pushRemovableIfNoThrow(env); }
void dce(Env& env, const bc::CreateSpecialImplicitContext&) {
  pushRemovableIfNoThrow(env);
}

/*
 * Default implementation is conservative: assume we use all of our
 * inputs, and can't be removed even if our output is unused.
 *
 * We also assume all the locals in the mayReadLocalSet must be
 * added to the live local set, and don't remove anything from it.
 */

template<class Op>
void no_dce(Env& env, const Op& op) {
  addLocGenSet(env, env.states.mayReadLocalSet());
  push_outputs(env, op.numPush());
  pop_inputs(env, op.numPop());
}

void dce(Env& env, const bc::AssertRATL& op) { no_dce(env, op); }
void dce(Env& env, const bc::AssertRATStk& op) { no_dce(env, op); }
void dce(Env& env, const bc::Await& op) { no_dce(env, op); }
void dce(Env& env, const bc::AwaitAll& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::BaseGL& op) { no_dce(env, op); }
void dce(Env& env, const bc::BaseH& op) { no_dce(env, op); }
void dce(Env& env, const bc::BreakTraceHint& op) { no_dce(env, op); }
void dce(Env& env, const bc::CGetCUNop& op) { no_dce(env, op); }
void dce(Env& env, const bc::CGetG& op) { no_dce(env, op); }
void dce(Env& env, const bc::ChainFaults& op) { no_dce(env, op); }
void dce(Env& env, const bc::CheckClsReifiedGenericMismatch& op) {
  no_dce(env, op);
}
void dce(Env& env, const bc::CheckClsRGSoft& op) {
  no_dce(env, op);
}
void dce(Env& env, const bc::CheckThis& op) { no_dce(env, op); }
void dce(Env& env, const bc::Clone& op) { no_dce(env, op); }
void dce(Env& env, const bc::ClsCns& op) { no_dce(env, op); }
void dce(Env& env, const bc::ClsCnsD& op) { no_dce(env, op); }
void dce(Env& env, const bc::ClsCnsL& op) { no_dce(env, op); }
void dce(Env& env, const bc::ClassGetTS& op) { no_dce(env, op); }
void dce(Env& env, const bc::CnsE& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContCheck& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContCurrent& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContEnter& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContGetReturn& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContKey& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContRaise& op) { no_dce(env, op); }
void dce(Env& env, const bc::ContValid& op) { no_dce(env, op); }
void dce(Env& env, const bc::CreateCont& op) { no_dce(env, op); }
void dce(Env& env, const bc::Enter& op) { no_dce(env, op); }
void dce(Env& env, const bc::Eval& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallClsMethod& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallClsMethodM& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallClsMethodD& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallClsMethodS& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallClsMethodSD& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallCtor& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallFunc& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallFuncD& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallObjMethod& op) { no_dce(env, op); }
void dce(Env& env, const bc::FCallObjMethodD& op) { no_dce(env, op); }
void dce(Env& env, const bc::GetMemoKeyL& op) { no_dce(env, op); }
void dce(Env& env, const bc::GetClsRGProp& op) { no_dce(env, op); }
void dce(Env& env, const bc::IncDecG& op) { no_dce(env, op); }
void dce(Env& env, const bc::IncDecS& op) { no_dce(env, op); }
void dce(Env& env, const bc::Incl& op) { no_dce(env, op); }
void dce(Env& env, const bc::InclOnce& op) { no_dce(env, op); }
void dce(Env& env, const bc::InitProp& op) { no_dce(env, op); }
void dce(Env& env, const bc::InstanceOf& op) { no_dce(env, op); }
void dce(Env& env, const bc::InstanceOfD& op) { no_dce(env, op); }
void dce(Env& env, const bc::IssetG& op) { no_dce(env, op); }
void dce(Env& env, const bc::IssetL& op) { no_dce(env, op); }
void dce(Env& env, const bc::IsUnsetL& op) { no_dce(env, op); }
void dce(Env& env, const bc::IterFree& op) { no_dce(env, op); }
void dce(Env& env, const bc::Jmp& op) { no_dce(env, op); }
void dce(Env& env, const bc::JmpNZ& op) { no_dce(env, op); }
void dce(Env& env, const bc::JmpZ& op) { no_dce(env, op); }
void dce(Env& env, const bc::LIterFree& op) { no_dce(env, op); }
void dce(Env& env, const bc::MemoGet& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::MemoGetEager& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::MemoSet& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::MemoSetEager& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::Method& op) { no_dce(env, op); }
void dce(Env& env, const bc::NativeImpl& op) { no_dce(env, op); }
void dce(Env& env, const bc::NewObj& op) { no_dce(env, op); }
void dce(Env& env, const bc::NewObjD& op) { no_dce(env, op); }
void dce(Env& env, const bc::NewObjS& op) { no_dce(env, op); }
void dce(Env& env, const bc::LockObj& op) { no_dce(env, op); }
void dce(Env& env, const bc::Nop& op) { no_dce(env, op); }
void dce(Env& env, const bc::OODeclExists& op) { no_dce(env, op); }
void dce(Env& env, const bc::Print& op) { no_dce(env, op); }
void dce(Env& env, const bc::RecordReifiedGeneric& op) { no_dce(env, op); }
void dce(Env& env, const bc::Req& op) { no_dce(env, op); }
void dce(Env& env, const bc::ReqDoc& op) { no_dce(env, op); }
void dce(Env& env, const bc::ReqOnce& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveClsMethod& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveClsMethodD& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveClsMethodS& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveRClsMethod& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveRClsMethodD& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveRClsMethodS& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveFunc& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveMethCaller& op) { no_dce(env, op); }
void dce(Env& env, const bc::ResolveRFunc& op) { no_dce(env, op); }
void dce(Env& env, const bc::Select& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetImplicitContextByValue& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetG& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetOpG& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetOpS& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetRangeM& op) { no_dce(env, op); }
void dce(Env& env, const bc::SetS& op) { no_dce(env, op); }
void dce(Env& env, const bc::Silence& op) {
  pinLocals(env, env.states.mayReadLocalSet());
  no_dce(env, op);
}
void dce(Env& env, const bc::SSwitch& op) { no_dce(env, op); }
void dce(Env& env, const bc::Switch& op) { no_dce(env, op); }
void dce(Env& env, const bc::This& op) { no_dce(env, op); }
void dce(Env& env, const bc::ThrowAsTypeStructException& op) {
  no_dce(env, op);
}
void dce(Env& env, const bc::ThrowNonExhaustiveSwitch& op) { no_dce(env, op); }
void dce(Env& env, const bc::RaiseClassStringConversionWarning& op) {
  no_dce(env, op);
}
void dce(Env& env, const bc::UGetCUNop& op) { no_dce(env, op); }
void dce(Env& env, const bc::UnsetG& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyImplicitContextState& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyOutType& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyParamType& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyParamTypeTS& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyRetNonNullC& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyRetTypeC& op) { no_dce(env, op); }
void dce(Env& env, const bc::VerifyRetTypeTS& op) { no_dce(env, op); }
void dce(Env& env, const bc::WHResult& op) { no_dce(env, op); }
void dce(Env& env, const bc::Yield& op) { no_dce(env, op); }
void dce(Env& env, const bc::YieldK& op) { no_dce(env, op); }

////////////////////////////////////////////////////////////////////////////////

// Iterator ops don't really read their key and value output locals; they just
// dec-ref the old value there before storing the new one (i.e. SetL semantics).
//
// We have to mark these values as live inside (else they could be completely
// killed - that is, remap locals could reused their local slot), but we don't
// have to may-read them.
//
// This is distinct from a kill, because the iter ops don't necessarily replace
// the key or value local.  Eg. on the last iteration, they are left alone.
void iter_dce(Env& env, const IterArgs& ita, LocalId baseId, int numPop) {
  addLocUse(env, ita.valId);
  if (ita.hasKey()) addLocUse(env, ita.keyId);
  if (baseId != NoLocalId) addLocGen(env, baseId);
  pop_inputs(env, numPop);
}

void dce(Env& env, const bc::IterInit& op) {
  iter_dce(env, op.ita, NoLocalId, op.numPop());
}

void dce(Env& env, const bc::LIterInit& op) {
  iter_dce(env, op.ita, op.loc2, op.numPop());
}

void dce(Env& env, const bc::IterNext& op) {
  iter_dce(env, op.ita, NoLocalId, op.numPop());
}

void dce(Env& env, const bc::LIterNext& op) {
  iter_dce(env, op.ita, op.loc2, op.numPop());
}

////////////////////////////////////////////////////////////////////////////////

/*
 * The minstr instructions can read a cell from the stack without
 * popping it. They need special handling.
 *
 * In addition, final minstr instructions can drop a number of
 * elements from the stack. Its possible that these elements are dead
 * (eg because an MEC somewhere in the sequence was optimized to MEI
 * or MET), so again we need some special handling.
 */

void minstr_touch(Env& env, int32_t depth) {
  assertx(depth < env.dceState.stack.size());
  // First make sure that the stack element we reference, and any
  // linked ones are marked used.
  use(env.dceState.forcedLiveLocations,
      env.dceState.stack, env.dceState.stack.size() - 1 - depth);
  // If any stack slots at a lower depth get killed, we'll have to
  // adjust our stack index accordingly, so add a MinstrStackFixup
  // action to candidate stack slots.
  // We only track slots up to kMaskSize though - so nothing below
  // that will get killed.
  if (depth > DceAction::kMaskSize) depth = DceAction::kMaskSize;
  while (depth--) {
    auto& ui = env.dceState.stack[env.dceState.stack.size() - 1 - depth];
    if (ui.usage != Use::Used) {
      auto const result = ui.actions.emplace(
        env.id, DceAction { DceAction::MinstrStackFixup, 1u << depth }
      );
      if (!result.second) {
        always_assert(
          result.first->second.action == DceAction::MinstrStackFixup
        );
        result.first->second.maskOrCount |= (1u << depth);
      }
    }
  }
}

template<class Op>
void minstr_base(Env& env, const Op& op, int32_t ix) {
  no_dce(env, op);
  minstr_touch(env, ix);
}

template<class Op>
void minstr_dim(Env& env, const Op& op) {
  no_dce(env, op);
  if (op.mkey.mcode == MEC || op.mkey.mcode == MPC) {
    minstr_touch(env, op.mkey.idx);
  }
  if (op.mkey.mcode == MEL || op.mkey.mcode == MPL) {
    addLocNameUse(env, op.mkey.local);
  }
}

template<class Op>
void minstr_final(Env& env, const Op& op, int32_t ndiscard) {
  addLocGenSet(env, env.states.mayReadLocalSet());
  if (op.mkey.mcode == MEL || op.mkey.mcode == MPL) {
    addLocNameUse(env, op.mkey.local);
  }

  push_outputs(env, op.numPush());
  auto const numPop = op.numPop();
  auto const stackRead = op.mkey.mcode == MEC || op.mkey.mcode == MPC ?
    op.mkey.idx : -1;

  for (auto i = numPop; i--; ) {
    if (i == stackRead || i >= DceAction::kMaskSize || i < numPop - ndiscard) {
      pop(env);
    } else {
      pop(env, {Use::Not, {{env.id, {DceAction::MinstrStackFinal, 1u << i}}}});
    }
  }

  if (stackRead >= numPop) {
    assertx(stackRead < env.dceState.stack.size());
    use(env.dceState.forcedLiveLocations,
        env.dceState.stack, env.dceState.stack.size() - 1 - stackRead);
  }
}

void dce(Env& env, const bc::BaseC& op)       { minstr_base(env, op, op.arg1); }
void dce(Env& env, const bc::BaseGC& op)      { minstr_base(env, op, op.arg1); }
void dce(Env& env, const bc::BaseSC& op)      {
  minstr_base(env, op, op.arg1);
  minstr_base(env, op, op.arg2);
}

void dce(Env& env, const bc::BaseL& op) {
  addLocNameUse(env, op.nloc1);
  if ((op.subop2 == MOpMode::Warn ||
       op.subop2 == MOpMode::None ||
       op.subop2 == MOpMode::InOut) &&
      !isLocLive(env, op.nloc1.id) &&
      !readCouldHaveSideEffects(locRaw(env, op.nloc1.id)) &&
      !isLocVolatile(env, op.nloc1.id)) {
    env.dceState.actionMap[env.id] = DceAction::MinstrPushBase;
  }
  no_dce(env, op);
}

void dce(Env& env, const bc::Dim& op)         { minstr_dim(env, op); }

void dce(Env& env, const bc::QueryM& op) {
  assertx(env.states.lastPush().has_value());
  if (!env.states.wasPEI()) {
    assertx(!env.dceState.minstrUI);
    auto ui = env.dceState.stack.back();
    if (!isLinked(ui)) {
      if (allUnused(ui)) {
        addLocGenSet(env, env.states.mayReadLocalSet());
        ui.actions[env.id] = DceAction::Kill;
        ui.location.id = env.id.idx;
        env.dceState.minstrUI.emplace(std::move(ui));
      } else if (auto const val = tv(*env.states.lastPush())) {
        addLocGenSet(env, env.states.mayReadLocalSet());
        CompactVector<Bytecode> bcs { gen_constant(*val) };
        ui.actions[env.id] = DceAction(DceAction::Replace, std::move(bcs));
        ui.location.id = env.id.idx;
        env.dceState.minstrUI.emplace(std::move(ui));
      }
    }
  }
  minstr_final(env, op, op.arg1);
}

void dce(Env& env, const bc::SetM& op)       { minstr_final(env, op, op.arg1); }
void dce(Env& env, const bc::IncDecM& op)    { minstr_final(env, op, op.arg1); }
void dce(Env& env, const bc::SetOpM& op)     { minstr_final(env, op, op.arg1); }
void dce(Env& env, const bc::UnsetM& op)     { minstr_final(env, op, op.arg1); }

void dispatch_dce(Env& env, const Bytecode& op) {
#define O(opcode, ...) case Op::opcode: dce(env, op.opcode); return;
  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

using MaskType = DceAction::MaskOrCountType;

void m_adj(uint32_t& depth, MaskType mask) {
  auto i = depth;
  if (i > DceAction::kMaskSize) i = DceAction::kMaskSize;
  while (i--) {
    if ((mask >> i) & 1) depth--;
  }
}

void m_adj(MKey& mkey, MaskType mask) {
  if (mkey.mcode == MEC || mkey.mcode == MPC) {
    uint32_t d = mkey.idx;
    m_adj(d, mask);
    mkey.idx = d;
  }
}

template<typename Op>
void m_adj(uint32_t& ndiscard, Op& op, MaskType mask) {
  auto const adjust = op.numPop() - ndiscard + 1;
  ndiscard += adjust;
  m_adj(ndiscard, mask);
  ndiscard -= adjust;
  m_adj(op.mkey, mask);
}

template <typename Op>
void adjustMinstr(Op& /*op*/, MaskType /*mask*/) {
  always_assert(false);
}

void adjustMinstr(bc::BaseC& op, MaskType m)       { m_adj(op.arg1, m); }
void adjustMinstr(bc::BaseGC& op, MaskType m)      { m_adj(op.arg1, m); }
void adjustMinstr(bc::BaseSC& op, MaskType m)      {
  m_adj(op.arg1, m);
  m_adj(op.arg2, m);
}

void adjustMinstr(bc::Dim& op, MaskType m)         { m_adj(op.mkey, m); }

void adjustMinstr(bc::QueryM& op, MaskType m)  { m_adj(op.arg1, op, m); }
void adjustMinstr(bc::SetM& op, MaskType m)    { m_adj(op.arg1, op, m); }
void adjustMinstr(bc::IncDecM& op, MaskType m) { m_adj(op.arg1, op, m); }
void adjustMinstr(bc::SetOpM& op, MaskType m)  { m_adj(op.arg1, op, m); }
void adjustMinstr(bc::UnsetM& op, MaskType m)  { m_adj(op.arg1, op, m); }

void adjustMinstr(Bytecode& op, MaskType m) {
#define O(opcode, ...) case Op::opcode: adjustMinstr(op.opcode, m); return;
  switch (op.op) { OPCODES }
#undef O
  not_reached();
}

template<typename LocRaw>
CompactVector<Bytecode> eager_unsets(
    const std::bitset<kMaxTrackedLocals>& candidates,
    const php::Func* func,
    LocRaw type) {
  auto loc = std::min(safe_cast<uint32_t>(func->locals.size()),
                      safe_cast<uint32_t>(kMaxTrackedLocals));
  auto const end = RuntimeOption::EnableArgsInBacktraces
                   ? func->params.size() + (uint32_t)func->isReified
                   : 0;
  CompactVector<Bytecode> bcs;
  while (loc-- > end) {
    if (candidates[loc] && !is_volatile_local(func, loc)) {
      auto const& t = type(loc);
      if (!t.subtypeOf(TUnc)) {
        bcs.emplace_back(bc::UnsetL { loc });
      }
    }
  }
  return bcs;
}

//////////////////////////////////////////////////////////////////////

struct DceOutState {
  DceOutState() = default;
  enum Local {};
  explicit DceOutState(Local) : isLocal(true) {
    locLive.set();
    locLiveExn.set();
  }

  /*
   * The union of the liveIn states of each normal successor for
   * locals and stack slots respectively.
   */
  std::bitset<kMaxTrackedLocals>             locLive;

  /*
   * The union of the liveIn states of each exceptional successor for
   * locals and stack slots respectively.
   */
  std::bitset<kMaxTrackedLocals>             locLiveExn;

  /*
   * The union of the dceStacks from the start of the normal successors.
   */
  Optional<std::vector<UseInfo>>      dceStack;

  /*
   * Whether this is for local_dce
   */
  bool                                        isLocal{false};
};

Optional<DceState>
dce_visit(VisitContext& visit, BlockId bid, const State& stateIn,
          const DceOutState& dceOutState,
          LocalRemappingIndex* localRemappingIndex = nullptr) {
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
    return std::nullopt;
  }

  auto& func = visit.func;
  auto const& fa = visit.ainfo;
  auto const ctx = AnalysisContext { fa.ctx.unit, func, fa.ctx.cls };
  auto states = locally_propagated_states(
    visit.index, ctx, visit.collect, bid, stateIn
  );

  auto dceState = DceState{ visit.index, fa };
  dceState.liveLocals = dceOutState.locLive;
  dceState.isLocal    = dceOutState.isLocal;
  dceState.remappingIndex = localRemappingIndex;
  // Any locals live out of the block may be desirable to unset in successors
  // if they are not live there.
  dceState.mayNeedUnsetting = dceState.liveLocals;

  auto const blk = func.blocks()[bid].get();
  auto const dceStkSz = [&] {
    switch (blk->hhbcs.back().op) {
      case Op::MemoGet:
      case Op::MemoGetEager:
        // These only push on the "hit" path. If we can prove they
        // never hit, the final stack state won't have an extra
        // element. But their dce implementations assume that they
        // push something. Fix it by just adding one to their in
        // state.  Note that when dceState.dceStack is populated, we
        // already did the equivalent during global analysis (see
        // isCFPushTaken below).
        states.next();
        return states.stack().size() + 1;
      default:
        auto const size = states.stack().size();
        states.next();
        return size;
    }
  }();

  if (dceOutState.dceStack) {
    dceState.stack = *dceOutState.dceStack;
    assertx(dceState.stack.size() == dceStkSz);
  } else {
    dceState.stack.resize(dceStkSz, UseInfo { Use::Used });
  }

  for (uint32_t idx = blk->hhbcs.size(); idx-- > 0;) {
    auto const& op = blk->hhbcs[idx];

    FTRACE(2, "  == #{} {}\n", idx, show(*func, op));

    if ((idx + 1) < blk->hhbcs.size()) states.next();

    auto visit_env = Env {
      dceState,
      op,
      { bid, idx },
      NoLocalId,
      states,
      {},
    };
    auto const liveAfter = dceState.liveLocals;
    auto const handled = [&] {
      if (dceState.minstrUI) {
        if (visit_env.states.wasPEI()) {
          dceState.minstrUI.reset();
          return false;
        }
        if (isMemberDimOp(op.op)) {
          dceState.minstrUI->actions[visit_env.id] = DceAction::Kill;
          // we're almost certainly going to delete this op, but we might not,
          // so we need to record its local uses etc just in case.
          return false;
        }
        if (isMemberBaseOp(op.op)) {
          auto const& final = blk->hhbcs[dceState.minstrUI->location.id];
          if (final.numPop()) {
            CompactVector<Bytecode> bcs;
            for (auto i = 0; i < final.numPop(); i++) {
              use(dceState.forcedLiveLocations,
                  dceState.stack, dceState.stack.size() - 1 - i);
              bcs.push_back(bc::PopC {});
            }
            dceState.minstrUI->actions[visit_env.id] = DceAction(
              DceAction::Replace,
              std::move(bcs)
            );
          } else {
            dceState.minstrUI->actions[visit_env.id] = DceAction::Kill;
          }
          commitActions(visit_env, false, dceState.minstrUI->actions);
          dceState.minstrUI.reset();
          return true;
        }
      }
      return false;
    }();
    if (!handled) {
      dispatch_dce(visit_env, op);

      /*
       * When we see a PEI, liveness must take into account the fact that we
       * could take an exception edge here by or-ing in the locLiveExn set.
       */
      if (states.wasPEI()) {
        FTRACE(2, "    <-- exceptions\n");
        dceState.liveLocals |= dceOutState.locLiveExn;
      }

      addInterference(visit_env, dceState.liveLocals | visit_env.liveInside);
    }

    if (visit_env.states.unreachable()) {
      FTRACE(4, "    Continuation of program has been marked unreachable.\n");
      dceState.willBeUnsetLocals.set();
    }
    // Any local that is live before this op, dead after this op may be worth
    // unsetting.  We check that we won't be inserting these UnsetLs as the
    // last op in a block to prevent a control flow ops from becoming non
    // terminal.
    auto const unsettable = (~liveAfter)
                           & (dceState.liveLocals | visit_env.liveInside)
                           & (~dceState.willBeUnsetLocals);

    FTRACE(4, "    Unsettable: {} (before: {}, after: {}, will be unset {})\n",
             loc_bits_string(func, unsettable),
             loc_bits_string(func, dceState.liveLocals),
             loc_bits_string(func, liveAfter),
             loc_bits_string(func, dceState.willBeUnsetLocals));

    // If we have a PEI instruction, we should try to unset the unsettable
    // locals at the start of the exception block.
    if (states.wasPEI() && blk->throwExit != NoBlockId) {
      dceState.mayNeedUnsettingExn |= unsettable | liveAfter;
    }
    auto const opIsCntrlFlow = [&] {
      if (idx != blk->hhbcs.size() - 1) return false;
      if (isRet(op.op)) return true;
      bool b = false;
      op.forEachTarget([&b] (BlockId) { b = true; });
      return b;
    };
    if (opIsCntrlFlow()) {
      FTRACE(4, "    Propagating unsettable: {}\n",
             loc_bits_string(func, unsettable));
      // We can't put Unsets in the last position in a block (control flow ops
      // must be last).  So when the last op in a block needs something unset
      // we flag it as maybe needing to be unset in all successors.
      dceState.mayNeedUnsetting |= unsettable;
    } else if (unsettable.any() && !visit_env.states.unreachable()) {
      FTRACE(4, "    Trying to unset: {}\n",
             loc_bits_string(func, unsettable));
      auto bcs = eager_unsets(unsettable, func, [&](uint32_t i) {
        return states.localAfter(i);
      });
      if (!bcs.empty()) {
        dceState.actionMap.emplace(visit_env.id, DceAction(
          DceAction::UnsetLocalsAfter,
          std::move(bcs)
        ));

        // We flag that we shoud rerun DCE since this Unset might be redudant if
        // a CGetL gets replaced with a PushL.
        visit_env.dceState.didAddOpts = true;
      }
    }

    // Update the locals that will be unset.
    if (op.op == Op::UnsetL) {
      if (op.UnsetL.loc1 < kMaxTrackedLocals) {
        dceState.willBeUnsetLocals[op.UnsetL.loc1] = true;
      }
    } else if (!(isMemberFinalOp(op.op) || isMemberDimOp(op.op))) {
      dceState.willBeUnsetLocals.reset();
    }

    FTRACE(4, "    dce frame: {}\n",
           loc_bits_string(func, dceState.liveLocals));
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
        return from(states.stack())
          | map([&] (const Type& t) { return show(t); })
          | unsplit<std::string>(" ");
      }()
    );
    if (dceState.minstrUI) {
      FTRACE(4, "    minstr ui: {}\n", show(*dceState.minstrUI));
    }

    // We're now at the state before this instruction, so the stack
    // sizes must line up.
    assertx(dceState.stack.size() == states.stack().size());
  }

  dceState.minstrUI.reset();
  return dceState;
}

struct DceAnalysis {
  std::bitset<kMaxTrackedLocals>      locLiveIn;
  std::vector<UseInfo>                stack;
  LocationIdSet                       forcedLiveLocations;
  std::bitset<kMaxTrackedLocals>      locMayNeedUnsetting;
  std::bitset<kMaxTrackedLocals>      locMayNeedUnsettingExn;
};

DceAnalysis analyze_dce(VisitContext& visit, BlockId bid,
                        const State& stateIn, const DceOutState& dceOutState,
                        LocalRemappingIndex* localRemappingIndex = nullptr) {
  auto dceState = dce_visit(visit, bid, stateIn, dceOutState,
                            localRemappingIndex);
  if (!dceState) return DceAnalysis {};

  return DceAnalysis {
    dceState->liveLocals,
    dceState->stack,
    dceState->forcedLiveLocations,
    dceState->mayNeedUnsetting,
    dceState->mayNeedUnsettingExn
  };
}

template<class Op>
using has_mkey = std::is_same<decltype(((Op*)0)->mkey), MKey>;

template<class Op>
void adjust_mkey(Op& bc, bool) {}

template<class Op>
typename std::enable_if<has_mkey<Op>::value>::type
adjust_mkey(Op& bc, int64_t adj) {
  if (bc.mkey.mcode == MEC || bc.mkey.mcode == MPC) {
    bc.mkey.idx += adj;
  }
}

void adjust_member_key(Bytecode& bc, int64_t adj) {
#define O(opcode, ...) case Op::opcode: adjust_mkey(bc.opcode, adj); break;
  switch (bc.op) { OPCODES }
#undef O
}

template<class Op>
using has_arg1 = std::is_same<decltype(((Op*)0)->arg1), uint32_t>;

template<class Op>
void adjust_arg1(Op& bc, bool) {}

template<class Op>
typename std::enable_if<has_arg1<Op>::value>::type
adjust_arg1(Op& bc, int64_t adj) {
  bc.arg1 += adj;
}

void adjust_ndiscard(Bytecode& bc, int64_t adj) {
#define O(opcode, ...)                                            \
  case Op::opcode:                                                \
    if (isMemberFinalOp(Op::opcode)) adjust_arg1(bc.opcode, adj); \
    break;
  switch (bc.op) { OPCODES }
#undef O
}

/*
 * Do the actual updates to the bytecodes.
 */
void dce_perform(php::WideFunc& func, const DceActionMap& actionMap) {

  using It = BytecodeVec::iterator;
  auto setloc = [] (int32_t srcLoc, It start, int n) {
    while (n--) {
      if (start->srcLoc < 0) start->srcLoc = srcLoc;
      start++;
    }
  };

  for (auto const& elm : actionMap) {
    auto const& id = elm.first;
    auto const& dceAction = elm.second;
    auto const b = func.blocks()[id.blk].mutate();
    auto const srcLoc = b->hhbcs[id.idx].srcLoc;
    FTRACE(1, "{} {}\n", show(elm), show(*func, b->hhbcs[id.idx]));

    switch (dceAction.action) {
      case DceAction::PopInputs:
        // we want to replace the bytecode with pops of its inputs
        if (auto const numToPop = numPop(b->hhbcs[id.idx])) {
          b->hhbcs.erase(b->hhbcs.begin() + id.idx);
          b->hhbcs.insert(b->hhbcs.begin() + id.idx,
                          numToPop,
                          bc::PopC {});
          setloc(srcLoc, b->hhbcs.begin() + id.idx, numToPop);
          break;
        }
        [[fallthrough]];
      case DceAction::Kill:
        if (b->hhbcs.size() == 1) {
          // we don't allow empty blocks
          b->hhbcs[0] = bc::Nop {};
          b->hhbcs[0].srcLoc = srcLoc;
        } else {
          b->hhbcs.erase(b->hhbcs.begin() + id.idx);
        }
        break;
      case DceAction::PopOutputs:
      {
        // we want to pop the things that the bytecode pushed
        auto const numToPop = numPush(b->hhbcs[id.idx]);
        b->hhbcs.insert(b->hhbcs.begin() + id.idx + 1,
                        numToPop,
                        bc::PopC {});
        setloc(srcLoc, b->hhbcs.begin() + id.idx + 1, numToPop);
        break;
      }
      case DceAction::Replace:
      {
        auto const& bcs = dceAction.bcs;
        always_assert(!bcs.empty());
        b->hhbcs.erase(b->hhbcs.begin() + id.idx);
        b->hhbcs.insert(b->hhbcs.begin() + id.idx,
                        begin(bcs), end(bcs));
        setloc(srcLoc, b->hhbcs.begin() + id.idx, bcs.size());
        break;
      }
      case DceAction::PopAndReplace:
      {
        auto const& bcs = dceAction.bcs;
        always_assert(!bcs.empty());
        auto const numToPop = numPop(b->hhbcs[id.idx]);
        b->hhbcs.erase(b->hhbcs.begin() + id.idx);
        b->hhbcs.insert(b->hhbcs.begin() + id.idx,
                        begin(bcs), end(bcs));
        if (numToPop) {
          b->hhbcs.insert(b->hhbcs.begin() + id.idx,
                          numToPop,
                          bc::PopC {});
        }
        setloc(srcLoc, b->hhbcs.begin() + id.idx, numToPop + bcs.size());
        break;
      }
      case DceAction::MinstrStackFinal:
      case DceAction::MinstrStackFixup:
      {
        adjustMinstr(b->hhbcs[id.idx], dceAction.maskOrCount);
        break;
      }
      case DceAction::MinstrPushBase:
      {
        assertx(b->hhbcs[id.idx].op == OpBaseL);
        auto const base = b->hhbcs[id.idx].BaseL;
        b->hhbcs[id.idx] = bc::PushL {base.nloc1.id};
        b->hhbcs.insert(
          b->hhbcs.begin() + id.idx + 1, bc::BaseC{0, base.subop2});
        setloc(srcLoc, b->hhbcs.begin() + id.idx, 2);
        for (auto p = id.idx + 2; p < b->hhbcs.size(); ++p) {
          auto const& bc = b->hhbcs[p];

          // Sometimes parts of a minstr will be unreachable, hhbbc marks these
          // with a fatal.
          if (bc.op == OpFatal) break;

          assertx(p + 1 < b->hhbcs.size() || isMemberFinalOp(bc.op));
          adjust_member_key(b->hhbcs[p], 1);
          adjust_ndiscard(b->hhbcs[p], 1);
          if (isMemberFinalOp(bc.op)) break;
        }
        break;
      }
      case DceAction::AdjustPop:
      {
        auto const op = b->hhbcs[id.idx].op;
        always_assert(op == Op::PopU2);
        assertx(dceAction.maskOrCount == 1);
        b->hhbcs[id.idx].PopU = bc::PopU {};
        b->hhbcs[id.idx].op = Op::PopU;
        break;
      }
      case DceAction::UnsetLocalsBefore: {
        assertx(id.idx == 0);
        auto const& bcs = dceAction.bcs;
        always_assert(!bcs.empty());
        b->hhbcs.insert(b->hhbcs.begin() + id.idx,
                        begin(bcs), end(bcs));
        setloc(srcLoc, b->hhbcs.begin() + id.idx, bcs.size());
        break;
      }
      case DceAction::UnsetLocalsAfter:
      {
        auto const& bcs = dceAction.bcs;
        always_assert(!bcs.empty());
        // If this is in the middle of a member op sequence, we walk to the
        // end, and place the UnsetLs there.
        auto idx = id.idx;
        for (auto bc = b->hhbcs[idx];
             (isMemberBaseOp(bc.op) || isMemberDimOp(bc.op)) &&
              idx < b->hhbcs.size();
             bc = b->hhbcs[++idx]) {
          if (b->hhbcs[idx + 1].op == OpFatal) {
            // We should not have tried to insert UnsetLs in a member op
            // sequence that is known to fatal.
            always_assert(false);
          }
        }
        // The MBR must not be alive across control flow edges.
        always_assert(idx < b->hhbcs.size());
        assertx(idx == id.idx || isMemberFinalOp(b->hhbcs[idx].op));
        b->hhbcs.insert(b->hhbcs.begin() + idx + 1,
                        begin(bcs), end(bcs));
        setloc(srcLoc, b->hhbcs.begin() + idx + 1, bcs.size());
        break;
      }
    }
  }
}

struct DceOptResult {
  std::bitset<kMaxTrackedLocals> usedLocalNames;
  std::bitset<kMaxTrackedLocals> locLiveIn;
  std::bitset<kMaxTrackedLocals> willBeUnsetLocals;
  DceActionMap actionMap;
  bool didAddOpts{false};
};

DceOptResult
optimize_dce(VisitContext& visit, BlockId bid, const State& stateIn,
             const DceOutState& dceOutState) {
  auto dceState = dce_visit(visit, bid, stateIn, dceOutState);
  if (!dceState) return {std::bitset<kMaxTrackedLocals>{}};

  return {
    std::move(dceState->usedLocalNames),
    std::move(dceState->liveLocals),
    std::move(dceState->willBeUnsetLocals),
    std::move(dceState->actionMap),
    dceState->didAddOpts
  };
}

//////////////////////////////////////////////////////////////////////

void remove_unused_local_names(
    php::WideFunc& func,
    const std::bitset<kMaxTrackedLocals>& usedLocalNames) {
  /*
   * Closures currently rely on name information being available.
   */
  if (func->isClosureBody) return;

  // For reified functions, skip the first non-param local
  auto loc = func->locals.begin() + func->params.size() + (int)func->isReified;
  for (; loc != func->locals.end(); ++loc) {
    // We can't remove the names of volatile locals, as they can be accessed by
    // name.
    assertx(loc->id == loc->nameId);
    if (is_volatile_local(func, loc->id)) continue;
    if (loc->unusedName) {
      assertx(loc->id < kMaxTrackedLocals && !usedLocalNames.test(loc->id));
    }
    if (loc->id < kMaxTrackedLocals && !usedLocalNames.test(loc->id)) {
      FTRACE(2, "  killing: {}\n", local_string(*func, loc->id));
      loc->unusedName = true;
    }
  }
}

// Take a vector mapping local ids to new local ids, and apply it to the
// function passed in via ainfo.
void apply_remapping(const FuncAnalysis& ainfo, php::WideFunc& func,
                     std::vector<LocalId>&& remapping) {
  auto const maxRemappedLocals = remapping.size();
  // During Global DCE we are for the most part free to modify the BCs since
  // we have frozen the index, and are no longer performing context sensitive
  // interps.
  // Walk the bytecode modifying the local usage according to the mapping.
  for (auto const bid : ainfo.rpoBlocks) {
    FTRACE(2, "Remapping block #{}\n", bid);

    auto const blk = func.blocks()[bid].mutate();
    for (uint32_t idx = blk->hhbcs.size(); idx-- > 0;) {
      auto& o = blk->hhbcs[idx];

      auto const fixupLoc = [&](LocalId& id) {
        if (0 <= id && id < maxRemappedLocals) {
          id = remapping[id];
        }
      };

      auto const fixupMKey = [&](MKey& mk) {
        switch (mk.mcode) {
          case MemberCode::MEL:
          case MemberCode::MPL:
            fixupLoc(mk.local.id);
            break;
          default:
            break;
        }
      };

      auto const fixupLAR = [&](const LocalRange& lr) {
        // LAR are pinned.
        if (lr.first < maxRemappedLocals) {
          assertx(lr.first == remapping[lr.first]);
        }
      };

      auto const fixupITA = [&](IterArgs& ita) {
        if (ita.hasKey()) {
          if (0 <= ita.keyId && ita.keyId < maxRemappedLocals) {
            ita.keyId = remapping[ita.keyId];
          }
        }
        if (0 <= ita.valId && ita.valId < maxRemappedLocals) {
          ita.valId = remapping[ita.valId];
        }
      };

#define IMM_BLA(n)
#define IMM_SLA(n)
#define IMM_IVA(n)
#define IMM_I64A(n)
#define IMM_LA(n)       fixupLoc(op.loc##n)
#define IMM_NLA(n)      fixupLoc(op.nloc##n.id)
#define IMM_ILA(n)      fixupLoc(op.loc##n)
#define IMM_IA(n)
#define IMM_DA(n)
#define IMM_SA(n)
#define IMM_RATA(n)
#define IMM_AA(n)
#define IMM_BA(n)
#define IMM_OA_IMPL(n)
#define IMM_OA(type)
#define IMM_VSA(n)
#define IMM_KA(n)      fixupMKey(op.mkey)
#define IMM_LAR(n)     fixupLAR(op.locrange)
#define IMM_ITA(n)     fixupITA(op.ita)
#define IMM_FCA(n)

#define IMM(which, n)             IMM_##which(n)
#define IMM_NA
#define IMM_ONE(x)                IMM(x, 1);
#define IMM_TWO(x, y)             IMM(x, 1); IMM(y, 2);
#define IMM_THREE(x, y, z)        IMM(x, 1); IMM(y, 2); \
                                  IMM(z, 3);
#define IMM_FOUR(x, y, z, l)      IMM(x, 1); IMM(y, 2); \
                                  IMM(z, 3); IMM(l, 4);
#define IMM_FIVE(x, y, z, l, m)   IMM(x, 1); IMM(y, 2); \
                                  IMM(z, 3); IMM(l, 4); \
                                  IMM(m, 5);
#define IMM_SIX(x, y, z, l, m, n) IMM(x, 1); IMM(y, 2); \
                                  IMM(z, 3); IMM(l, 4); \
                                  IMM(m, 5); IMM(n, 6);

#define O(opcode, imms, ...) \
      case Op::opcode: {\
        UNUSED auto& op = o.opcode; IMM_##imms; \
        break; \
      }
      switch (o.op) { OPCODES }
#undef O

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA

#undef IMM
#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR
#undef IMM_FIVE
#undef IMM_SIX
    }
  }
}

void remap_locals(const FuncAnalysis& ainfo, php::WideFunc& func,
                  LocalRemappingIndex&& remappingIndex) {
  /*
   * Remapping locals in closures requires checking which ones
   * are captured variables so we can remove the relevant properties,
   * and then we'd have to mutate the CreateCl callsite, so we don't
   * bother for now.
   *
   * Note: many closure bodies have unused $this local, because of
   * some emitter quirk, so this might be worthwhile.
   */
  if (func->isClosureBody) return;

  auto& localInterference = remappingIndex.localInterference;
  auto const& pinned = remappingIndex.pinnedLocals;
  auto const maxRemappedLocals = localInterference.size();

  auto const localsInterfere = [&](LocalId l1, LocalId l2) -> bool {
    if (l1 >= maxRemappedLocals || l2 >= maxRemappedLocals) return true;
    if (is_volatile_local(func, l1) || is_volatile_local(func, l2)) return true;
    assertx(l1 != l2);
    assertx(localInterference[l1][l2] == localInterference[l2][l1]);
    return localInterference[l1][l2];
  };

  // Unify locals
  // It's worth noting this invalidates the localInterference
  // info for the larger id of l1, l2.
  auto const unifyLocals = [&](LocalId l1, LocalId l2) {
    // We can't join locals that interfere.
    assertx(!localsInterfere(l1, l2));

    // We unify into the smaller localid.  The larger one will no longer be
    // valid.
    assertx(l1 < l2);

    // Everything that was uniquely a conflict to l2 is now also a conflict to
    // l1 and should be updated.
    auto const newConflicts = localInterference[l2] ^
      (localInterference[l1] & localInterference[l2]);
    for (auto i = maxRemappedLocals; i-- > 0;) {
      if (newConflicts[i]) {
        localInterference[i][l1] = true;
      }
    }
    localInterference[l1] |= localInterference[l2];
  };

  auto const isParam = [&](uint32_t i) {
    return i < (func->params.size() + (int)func->isReified);
  };

  std::vector<LocalId> remapping(maxRemappedLocals);
  // Greedy merge the locals.  This could probably be sorted by live range
  // length or something to achieve better coalescing.  Or if live range
  // splitting is added, even be guided by profile info.
  for (auto i = maxRemappedLocals; i-- > 0;) {
    remapping[i] = i;
    if (func->locals[i].killed) {
      // Killed in a previous round of DCE.
      assertx(localInterference[i].none());
      continue;
    }
    if (isParam(i) || pinned[i]) continue;
    for (auto j = i; j-- > 0;) {
      if (func->locals[j].killed) continue;
      if (RuntimeOption::EnableArgsInBacktraces && isParam(j)) continue;
      if (localsInterfere(i, j)) continue;
      // Remap the local and update interference sets.
      func->locals[i].killed = true;
      remapping[i] = j;
      unifyLocals(j, i);
      break;
    }
  }

  bool identityMapping = true;
  // Canonicalize the local remapping.
  for (auto i = 0; i < maxRemappedLocals; i++) {
    if (remapping[i] != i) {
      identityMapping = false;
      // We only have to check one deep because we know that all earlier locals
      // are already cononicalized.
      auto const newId = remapping[remapping[i]];
      assertx(remapping[newId] == newId);
      remapping[i] = newId;
    }
  }

  if (!identityMapping) {
    apply_remapping(ainfo, func, std::move(remapping));
  }
}

//////////////////////////////////////////////////////////////////////

}

void local_dce(VisitContext& visit, BlockId bid, const State& stateIn) {
  // For local DCE, we have to assume all variables are in the
  // live-out set for the block.
  auto const ret = optimize_dce(visit, bid, stateIn,
                                DceOutState{DceOutState::Local{}});
  dce_perform(visit.func, ret.actionMap);
}

//////////////////////////////////////////////////////////////////////

bool global_dce(const Index& index, const FuncAnalysis& ai,
                php::WideFunc& func) {
  auto rpoId = [&] (BlockId blk) {
    return ai.bdata[blk].rpoId;
  };

  auto collect = CollectedInfo{
    index,
    ai.ctx,
    nullptr, CollectionOpts{},
    nullptr,
    &ai
  };
  auto visit = VisitContext(index, ai, collect, func);

  FTRACE(1, "|---- global DCE analyze ({})\n", show(ai.ctx));
  FTRACE(2, "{}", [&] {
    using namespace folly::gen;
    auto i = uint32_t{0};
    return from(func->locals)
      | mapped(
        [&] (const php::Local& l) {
          return folly::sformat("  {} {}\n", i++, local_string(*func, l.id));
        })
      | unsplit<std::string>("");
  }());

  /*
   * States for each block, indexed by block id.
   */
  std::vector<DceOutState> blockStates(func.blocks().size());

  /*
   * If EnableArgsInBacktraces is true, then argument locals (and the reified
   * generics local) are included in the backtrace attached to exceptions, so
   * they are live for any instruction that may throw. In order to converge
   * quickly in this case, we update the locLiveExn sets early.
   */
  if (RuntimeOption::EnableArgsInBacktraces) {
    auto const args = func->params.size() + (int)func->isReified;
    auto locLiveExn = std::bitset<kMaxTrackedLocals>();
    if (args < kMaxTrackedLocals) {
      for (auto i = 0; i < args; i++) locLiveExn.set(i);
    } else {
      locLiveExn.set();
    }
    for (auto& blockState : blockStates) {
      blockState.locLiveExn |= locLiveExn;
    }
  }

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
  for (auto const bid : ai.rpoBlocks) incompleteQ.push(rpoId(bid));

  auto const nonThrowPreds = computeNonThrowPreds(func, ai.rpoBlocks);
  auto const throwPreds    = computeThrowPreds(func, ai.rpoBlocks);

  /*
   * Suppose a stack slot isn't used, but it was pushed on two separate
   * paths, one of which could be killed, but the other can't. We have
   * to prevent either, so any time we find a non-used stack slot that
   * can't be killed, we add it to this set (via markUisLive, and the
   * DceAnalysis), and schedule its block for re-analysis.
   */
  LocationIdSet forcedLiveLocations;

  /*
   * Temporary set used to collect locations that were forced live by
   * block merging/linked locations etc.
   */
  LocationIdSet forcedLiveTemp;

  auto checkLive = [&] (std::vector<UseInfo>& uis, uint32_t i,
                        LocationId location) {
    if (forcedLiveLocations.count(location)) return true;
    if (!isLinked(uis[i])) return false;
    assertx(i);
    return uis[i - 1].usage == Use::Used;
  };

  auto fixupUseInfo = [&] (std::vector<UseInfo>& uis,
                           BlockId blk,
                           bool isSlot) {
    for (uint32_t i = 0; i < uis.size(); i++) {
      auto& out = uis[i];
      if (out.location.blk == NoBlockId) out.location = { blk, i, isSlot };
      if (checkLive(uis, i, out.location)) {
        use(forcedLiveTemp, uis, i);
      }
    }
  };

  auto mergeUIs = [&] (std::vector<UseInfo>& outBase,
                       const std::vector<UseInfo>& inBase,
                       uint32_t i, BlockId blk, bool isSlot) {
    auto& out = outBase[i];
    auto& in = inBase[i];
    if (out.usage == Use::Used) {
      if (in.usage != Use::Used) {
        // This is to deal with the case where blk has multiple preds,
        // and one of those has multiple succs, one of which does use
        // this stack value.
        while (true) {
          auto& ui = inBase[i];
          if (ui.usage != Use::Used) {
            auto location = ui.location;
            if (location.blk == NoBlockId) location = { blk, i, isSlot };
            forcedLiveTemp.insert(location);
          }
          auto linked = isLinked(ui);
          if (!linked) break;
          assertx(i);
          i--;
        }
      }
      return false;
    }

    if (in.usage == Use::Used || out.usage != in.usage) {
      use(forcedLiveTemp, outBase, i);
      return true;
    }
    auto location = in.location;
    if (location.blk == NoBlockId) location = { blk, i, isSlot };
    if (checkLive(outBase, i, location)) {
      use(forcedLiveTemp, outBase, i);
      return true;
    }

    auto ret = false;
    for (auto const& id : in.actions) {
      ret |= out.actions.insert(id).second;
    }
    assertx(out.location.blk != NoBlockId);
    if (out.location < location) {
      // It doesn't matter which one we choose, but it should be
      // independent of the visiting order.
      out.location = location;
      ret = true;
    }
    return ret;
  };

  auto mergeUIVecs = [&] (Optional<std::vector<UseInfo>>& stkOut,
                          const std::vector<UseInfo>& stkIn,
                          BlockId blk, bool isSlot) {
    if (!stkOut) {
      stkOut = stkIn;
      fixupUseInfo(*stkOut, blk, isSlot);
      return true;
    }

    auto ret = false;
    assertx(stkOut->size() == stkIn.size());
    for (uint32_t i = 0; i < stkIn.size(); i++) {
      if (mergeUIs(*stkOut, stkIn, i, blk, isSlot)) ret = true;
    }
    return ret;
  };

  /*
   * If we weren't able to take advantage of any unused entries
   * on the dceStack that originated from another block, mark
   * them used to prevent other paths from trying to delete them.
   *
   * Also, merge the entries into our global forcedLiveLocations, to
   * make sure they always get marked Used.
   */
  auto processForcedLive = [&] (const LocationIdSet& forcedLive) {
    for (auto const& id : forcedLive) {
      if (forcedLiveLocations.insert(id).second) {
        // This is slightly inefficient; we know exactly how this will
        // affect the analysis, so we could get away with just
        // reprocessing the affected predecessors of id.blk; but this
        // is much simpler, and less error prone. We can revisit if it
        // turns out to be a performance issue.
        FTRACE(5, "Forcing {} live\n", show(id));
        incompleteQ.push(rpoId(id.blk));
      }
    }
  };

  /*
   * We track interfering locals in order to compact the number of them.
   * This is useful to reduce fame space at runtime.  The decision made
   * later could benefit from profile info, but HHBBC currently doesn't
   * get fed any such information.
   */
  auto const maxRemappedLocals = std::min(
      (size_t)func->locals.size(),
      (size_t)kMaxTrackedLocals);
  LocalRemappingIndex localRemappingIndex(maxRemappedLocals);

  /*
   * Parameters are not uninit at the entry to the function even if they go
   * unused throughout the function, they conflict with any local that is live
   * at an entrypoint.  Such a local might be depending on the local slot being
   * Unset from the function entry.
   */
  std::bitset<kMaxTrackedLocals> paramSet;
  paramSet.set();
  paramSet >>= kMaxTrackedLocals - (func->params.size() + (int)func->isReified);
  boost::dynamic_bitset<> entrypoint(func.blocks().size());
  entrypoint[func->mainEntry] = true;
  for (auto const blkId: func->dvEntries) {
    if (blkId != NoBlockId) {
      entrypoint[blkId]= true;
    }
  }

  /*
   * The set of locals that may need to be unset by a succesor block.
   */
  std::vector<std::bitset<kMaxTrackedLocals>>
    locMayNeedUnsetting(func.blocks().size());
  std::vector<std::bitset<kMaxTrackedLocals>>
    locMayNeedUnsettingExn(func.blocks().size());

  /*
   * Iterate on live out states until we reach a fixed point.
   *
   * This algorithm treats the exceptional live-out states differently
   * from the live-out states during normal control flow.  The
   * liveOutExn sets only take part in the liveIn computation when the
   * block has exceptional exits.
   */
  while (!incompleteQ.empty()) {
    auto const bid = ai.rpoBlocks[incompleteQ.pop()];

    // skip unreachable blocks
    if (!ai.bdata[bid].stateIn.initialized) continue;

    FTRACE(2, "block #{}\n", bid);

    auto& blockState = blockStates[bid];
    auto const result = analyze_dce(visit, bid, ai.bdata[bid].stateIn,
                                    blockState, &localRemappingIndex);

    FTRACE(2, "loc live out  : {}\n"
              "loc out exn   : {}\n"
              "loc live in   : {}\n",
              loc_bits_string(func, blockState.locLive),
              loc_bits_string(func, blockState.locLiveExn),
              loc_bits_string(func, result.locLiveIn));

    processForcedLive(result.forcedLiveLocations);
    locMayNeedUnsetting[bid] = result.locMayNeedUnsetting;
    locMayNeedUnsettingExn[bid] = result.locMayNeedUnsettingExn;

    // If blk ends with an iterator block, and succId is the loop body for
    // this iterator, this method returns the iterator's arguments so that
    // we can kill its key and value output locals.
    //
    // We can't do this optimization if succId is also the loop end block.
    // This case should never occur, because then the iter would have to
    // be both live and dead at succId, but we guard against it anyway.
    //
    // It would be nice to move this logic to the iter_dce method itself,
    // but this analysis pass only tracks a single out-state for each block,
    // so we must do it here to apply it to the in-state of the body and not
    // to the in-state of the done block. (Catch blocks are handled further
    // down and this logic never applies to them.)
    auto const killIterOutputs = [] (const php::Block* blk, BlockId succId)
        -> Optional<IterArgs> {
      auto const& lastOpc = blk->hhbcs.back();
      auto const next = blk->fallthrough == succId;
      auto ita = Optional<IterArgs>{};
      auto const kill = [&]{
        switch (lastOpc.op) {
          case Op::IterInit:
            ita = lastOpc.IterInit.ita;
            return next && lastOpc.IterInit.target2 != succId;
          case Op::LIterInit:
            ita = lastOpc.LIterInit.ita;
            return next && lastOpc.LIterInit.target3 != succId;
          case Op::IterNext:
            ita = lastOpc.IterNext.ita;
            return !next && lastOpc.IterNext.target2 == succId;
          case Op::LIterNext:
            ita = lastOpc.LIterNext.ita;
            return !next && lastOpc.LIterNext.target3 == succId;
          default:
            return false;
        }
      }();
      return kill ? ita : std::nullopt;
    };

    auto const isCFPushTaken = [] (const php::Block* blk, BlockId succId) {
      auto const& lastOpc = blk->hhbcs.back();
      switch (lastOpc.op) {
        case Op::MemoGet:
          return lastOpc.MemoGet.target1 == succId;
        case Op::MemoGetEager:
          return lastOpc.MemoGetEager.target1 == succId;
        default:
          return false;
      }
    };

    // As mentioned above, at entrypoints all live locals conflict with
    // parameters.  (Any live local that is not a param is depending upon their
    // slot being uninit)
    if (entrypoint[bid]) {
      addInterference(&localRemappingIndex, result.locLiveIn | paramSet);
    }

    // Merge the liveIn into the liveOut of each normal predecessor.
    // If the set changes, reschedule that predecessor.
    for (auto const pid : nonThrowPreds[bid]) {
      FTRACE(2, "  -> {}\n", pid);
      auto& pbs = blockStates[pid];
      auto const oldPredLocLive = pbs.locLive;
      auto const pred = func.blocks()[pid].get();
      if (auto const ita = killIterOutputs(pred, bid)) {
        auto const key = ita->hasKey();
        FTRACE(3, "    Killing iterator output locals: {}\n",
               key ? folly::to<std::string>(ita->valId, ", ", ita->keyId)
                   : folly::to<std::string>(ita->valId));
        auto liveIn = result.locLiveIn;
        if (ita->valId < kMaxTrackedLocals) liveIn[ita->valId] = false;
        if (key && ita->keyId < kMaxTrackedLocals) liveIn[ita->keyId] = false;
        pbs.locLive |= liveIn;
      } else {
        pbs.locLive |= result.locLiveIn;
      }

      auto changed = pbs.locLive != oldPredLocLive;

      changed |= [&] {
        if (isCFPushTaken(pred, bid)) {
          auto stack = result.stack;
          stack.insert(stack.end(), pred->hhbcs.back().numPush(),
                       UseInfo{Use::Not});
          return mergeUIVecs(pbs.dceStack, stack, bid, false);
        } else {
          return mergeUIVecs(pbs.dceStack, result.stack, bid, false);
        }
      }();
      if (changed) {
        incompleteQ.push(rpoId(pid));
      }
    }

    // Merge the liveIn into the liveOutExn state for each throw predecessor.
    // The liveIn computation also depends on the liveOutExn state, so again
    // reschedule if it changes.
    for (auto const pid : throwPreds[bid]) {
      FTRACE(2, "  => {}\n", pid);
      auto& pbs = blockStates[pid];
      auto const oldPredLocLiveExn = pbs.locLiveExn;
      pbs.locLiveExn |= result.locLiveIn;
      if (pbs.locLiveExn != oldPredLocLiveExn) {
        incompleteQ.push(rpoId(pid));
      }
    }

    while (!forcedLiveTemp.empty()) {
      auto t = std::move(forcedLiveTemp);
      processForcedLive(t);
    }
  }

  auto const predMayNeedUnsetting = [&] (BlockId bid){
    std::bitset<kMaxTrackedLocals> needUnsetting;
    if (entrypoint[bid]) {
      needUnsetting |= paramSet;
    }
    for (auto const pid : nonThrowPreds[bid]) {
      needUnsetting |= locMayNeedUnsetting[pid];
    }
    for (auto const pid : throwPreds[bid]) {
      needUnsetting |= locMayNeedUnsettingExn[pid];
    }
    return needUnsetting;
  };

  /*
   * Now that we're at a fixed point, use the propagated states to
   * remove instructions that don't need to be there.
   */
  FTRACE(1, "|---- global DCE optimize ({})\n", show(ai.ctx));
  std::bitset<kMaxTrackedLocals> usedLocalNames;
  DceActionMap actionMap;
  bool didAddOpts = false;
  for (auto const bid : ai.rpoBlocks) {
    FTRACE(2, "block #{}\n", bid);
    auto const& stateIn = ai.bdata[bid].stateIn;
    auto ret = optimize_dce(visit, bid, stateIn, blockStates[bid]);
    auto const unsettable = (~ret.locLiveIn) & predMayNeedUnsetting(bid) &
                           (~ret.willBeUnsetLocals);
    if (unsettable.any() && ai.bdata[bid].stateIn.initialized &&
        !ai.bdata[bid].stateIn.unreachable) {
      auto bcs = eager_unsets(unsettable, func, [&](uint32_t i) {
        return ai.bdata[bid].stateIn.locals[i];
      });
      if (!bcs.empty()) {
        ret.actionMap.emplace(InstrId { bid, 0 }, DceAction(
          DceAction::UnsetLocalsBefore,
          std::move(bcs)
        ));

        // We flag that we shoud rerun DCE since this Unset might be redudant if
        // a CGetL gets replaced with a PushL.
        didAddOpts = true;
      }
    }

    didAddOpts = didAddOpts || ret.didAddOpts;
    usedLocalNames |= ret.usedLocalNames;
    combineActions(actionMap, std::move(ret.actionMap));
  }

  dce_perform(func, actionMap);
  remove_unused_local_names(func, usedLocalNames);
  remap_locals(ai, func, std::move(localRemappingIndex));
  return didAddOpts;
}

//////////////////////////////////////////////////////////////////////

}
