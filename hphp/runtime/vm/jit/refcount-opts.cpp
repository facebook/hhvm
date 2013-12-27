/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/opt.h"

#include "folly/Lazy.h"
#include "folly/Optional.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

namespace {

// These are used a lot in this file.
using HPHP::Trace::Indent;
using HPHP::Trace::indent;

// A Point is an id representing a program point, determined by IdMap.
typedef uint32_t Point;

/*
 * IdMap stores a linear position id for program points before (even numbers)
 * and after (odd numbers) each instruction. The distinction is necessary so we
 * can insert sink points after the last instruction of a block.
 */
struct IdMap {
  IdMap(const BlockList& blocks, const IRUnit& unit)
    : m_ids(unit, -1)
  {
    Point nextId = 0;
    for (auto* block : blocks) {
      for (auto& inst : *block) {
        m_ids[inst] = nextId;
        nextId += 2;
        m_insts.push_back(&inst);
      }
    }
  }

  IRInstruction* inst(Point id) const { return m_insts.at(id / 2); }

  static bool isBefore(Point id) { return !(id % 2); }
  static bool isAfter(Point id)  { return id % 2; }

  Point before(const IRInstruction* inst) const { return m_ids[inst]; }
  Point before(const IRInstruction& inst) const { return before(&inst); }

  Point after(const IRInstruction* inst) const { return m_ids[inst] + 1; }
  Point after(const IRInstruction& inst) const { return after(&inst); }

 private:
  StateVector<IRInstruction, Point> m_ids;
  smart::vector<IRInstruction*> m_insts;
};

/*
 * IncSet holds a set of ids of IncRef instructions, representing a pending
 * reference in a Value (defined below). We have to use a set of ids instead of
 * just a single id because it's possible for the same pending reference to
 * come from different instructions in different control flow paths.
 */
typedef smart::flat_set<Point> IncSet;
std::string show(const IncSet& incs, const IdMap& ids) {
  std::string ret;
  auto* separator = "";
  for (auto id : incs) {
    folly::toAppend(separator, ids.inst(id)->toString(), &ret);
    separator = "\n";
  }
  return ret;
}

/*
 * Value represents what we know about the current state of a php value with a
 * refcount. It holds lower bounds on the real refcount and optimized refcount
 * of an object. As the analysis pass progresses, it uses this information to
 * ensure that any optimizations will not affect what observers of the refcount
 * see.
 */
struct Value {
  explicit Value(int c = 0, bool load = false)
    : realCount(c)
    , fromLoad(load)
  {}

  int optCount() const { return realCount - optDelta(); }
  int optDelta() const { return pendingIncs.size(); }
  bool empty() const {
    return pendingIncs.empty() && realCount == 0 && !fromLoad;
  }

  void pessimize() {
    realCount = 0;
    pendingIncs.clear();
  }

  /* Merge other's state with this state. realCount becomes the minimum from
   * the two. pendingIncs is truncated to the size of the smaller of the two,
   * then the sets at each index are merged. */
  void merge(const Value& other) {
    assert(fromLoad == other.fromLoad);
    realCount = std::min(realCount, other.realCount);
    auto minSize = std::min(pendingIncs.size(), other.pendingIncs.size());
    pendingIncs.resize(minSize);
    for (unsigned i = 0; i < minSize; ++i) {
      for (auto id : other.pendingIncs[i]) {
        pendingIncs[i].insert(id);
      }
    }
  }

  void pushInc(Point id) {
    ++realCount;
    pendingIncs.emplace_back();
    pendingIncs.back().insert(id);
  }

  IncSet popRef() {
    assert(!pendingIncs.empty());
    auto incs = std::move(pendingIncs.back());
    pendingIncs.pop_back();
    return incs;
  }

  /* realCount is the number of live references currently owned by the
   * jit. Since it represents references with lifetimes we can reliably track,
   * it's also used as a lower bound of the refcount of the object. */
  int realCount;

  /* fromLoad represents whether or not the value came from a load
   * instruction. This optimization is based on the idea of producing and
   * consuming references, and most of our load instructions sometimes produce
   * a reference and sometimes don't. Rather that splitting up all the loads
   * into two flavors, we allow consumption of a value from a load even if
   * there appear to be no live references. */
  bool fromLoad;

 private:
  friend std::string show(const Value&);

  /* pendingIncs contains ids of IncRef instructions that have yet to be placed
   * before an observer. The size of pendingIncs represents the delta between
   * the real count and the optimized count. */
  smart::vector<IncSet> pendingIncs;
};

std::string show(const Value& state) {
  std::string incs;
  auto* sep = "";
  for (auto const& set : state.pendingIncs) {
    folly::toAppend(sep, '{', folly::join(", ", set), '}', &incs);
    sep = ", ";
  }

  return folly::format(
    "(real, opt) = ({:>2}, {:>2}) - [{}]{}",
    state.realCount, state.optCount(),
    incs,
    state.fromLoad ? " - from load" : "").str();
}

/*
 * Frame represents a call frame and is used to track references to the current
 * $this pointer.
 */
struct Frame {
  explicit Frame(SSATmp* main = nullptr, SSATmp* current = nullptr)
    : mainThis(main)
    , currentThis(current)
  {}

  /* The canonical form of $this. For the outermost frame of the trace, this
   * will be nullptr if we haven't loaded $this yet, or a LdThis if we
   * have. For inlined frames, this will be the canonical SSATmp* for the $this
   * pointer given to the SpillFrame. */
  SSATmp* mainThis;

  /* Latest live temp holding $this. This is updated whenever we see a new
   * LdThis in a frame, and it's set to nullptr after any instructions that
   * kill all live temps (calls, mostly). */
  SSATmp* currentThis;

  bool operator==(const Frame b) const {
    return mainThis == b.mainThis && currentThis == b.currentThis;
  }
};

std::string show(const Frame frame) {
  if (!frame.mainThis && !frame.currentThis) {
    return "<no $this>";
  }
  std::string ret;
  if (frame.mainThis) {
    folly::toAppend("main this: ", frame.mainThis->toString(), &ret);
    if (frame.currentThis) ret += " ";
  }
  if (frame.currentThis) {
    folly::toAppend("current this: ", frame.currentThis->toString(), &ret);
  }
  return ret;
}

/*
 * FrameStack contains a Frame struct for all live and pre-live frames.
 */
struct FrameStack {
  /* Push a possibly empty Frame representing a pre-live ActRec. We don't yet
   * know if the call with be inlined or not. */
  void pushPreLive(Frame f = Frame()) {
    preLive.emplace_back(f);
  }

  /* Push a new frame representing a newly defined FramePtr for an inlined
   * call. */
  void pushInline(const IRInstruction* fpInst) {
    assert(!preLive.empty());
    assert(fpInst->is(DefInlineFP));
    live[fpInst] = std::move(preLive.back());
    preLive.pop_back();
  }

  /* Pop an inlined frame to represent an InlineReturn, forgetting what we know
   * about its $this pointer. */
  void popInline(const IRInstruction* fpInst) {
    assert(frameRoot(fpInst) == fpInst);
    assert(live.size() >= 2);
    auto it = live.find(fpInst);
    assert(it != live.end());

    assert(dead.count(fpInst) == 0);
    dead[fpInst] = std::move(it->second);
    live.erase(it);
  }

  /* Pop a non-inlined frame. This is only called if a trace includes a RetC in
   * the outermost function. */
  void pop() {
    assert(live.size() == 1);
    live.erase(live.begin());
  }

  bool operator==(const FrameStack& b) const {
    return live == b.live && preLive == b.preLive;
  }

  /* Map from the instruction defining the frame to a Frame object. */
  smart::hash_map<const IRInstruction*, Frame> live;

  /* Similar to live, but for frames that have been popped. We keep track of
   * these because we often refer to a LdThis from an inlined function after
   * it's returned. */
  smart::hash_map<const IRInstruction*, Frame> dead;

  /* Frames that have been pushed but not activated. */
  smart::vector<Frame> preLive;
};

typedef smart::hash_map<SSATmp*, SSATmp*> CanonMap;

/*
 * State holds all the information we care about during the analysis pass, and
 * is used to propagate and merge state across control flow edges.
 */
struct State {
  /* values maps from live SSATmps to the currently known state about the
   * value. */
  smart::hash_map<SSATmp*, Value> values;

  /* canon keeps track of values that have been through passthrough
   * instructions, like CheckType and AssertType. */
  CanonMap canon;

  /* frames keeps track of live $this pointers in call frames. */
  FrameStack frames;
};

std::string show(const State& state) {
  std::string ret;

  for (auto const& val : state.values) {
    if (val.second.empty()) continue;

    folly::toAppend(indent(), show(val.second), " | ",
                    val.first->inst()->toString(), '\n', &ret);
  }

  if (!state.frames.live.empty()) {
    folly::toAppend(indent(), "live frames (unordered):\n", &ret);
    Indent _i;
    for (auto const& pair : state.frames.live) {
      folly::toAppend(indent(), pair.first->toString(), ": ",
                      show(pair.second), '\n', &ret);
    }
  }

  if (!state.frames.preLive.empty()) {
    folly::toAppend(indent(), "pre-live frames:\n", &ret);
    Indent _i;
    for (auto const& frame : state.frames.preLive) {
      folly::toAppend(indent(), show(frame), '\n', &ret);
    }
  }

  if (!state.canon.empty()) {
    folly::toAppend(indent(), "replaced values:\n", &ret);
    Indent _i;
    for (auto const& pair : state.canon) {
      ret += folly::format("{}canon[{}] = {}\n",
                           indent(), *pair.first, *pair.second).str();
    }
  }

  return ret;
}

struct IncomingState {
  IncomingState(const Block* f, const State& s)
    : from(f)
    , state(s)
  {}

  const Block* from;
  State state;
};
typedef smart::vector<IncomingState> IncomingStateVec;
typedef smart::hash_map<const Block*, IncomingStateVec> SavedStates;

/*
 * One SinkPoint exists for each optimizable IncRef in each control flow path
 * dominated by it.
 */
struct SinkPoint {
  SinkPoint(Point i, SSATmp* v, bool erase)
    : id(i)
    , eraseOnly(erase)
    , value(v)
  {}

  /* The position of the sink point, from IdMap. */
  Point id;

  /* In some very rare situations we may know that we want to insert a
   * SinkPoint for a value without having a live SSATmp for that value. This
   * means it's not safe to sink the IncRef to that point, but if the
   * instruction immediately after the SinkPoint is a DecRef of the same value,
   * it's safe to eliminate both instructions. */
  bool eraseOnly;

  /* The current version of the value, if it has been through a passthrough
   * instruction. Keeping track of this allows us to sink an IncRef of a Cell
   * past a CheckType and turn it into a cheaper IncRef. */
  SSATmp* value;
};

#ifdef HAVE_BOOST1_49
typedef smart::flat_multimap<IncSet, SinkPoint> SinkPoints;
#else
typedef smart::multimap<IncSet, SinkPoint> SinkPoints;
#endif
struct SinkPointsMap {
  // Maps values to SinkPoints for their IncRefs
  smart::hash_map<SSATmp*, SinkPoints> points;

  // Maps ids of DecRef instructions to the incoming lower bound of the object's
  // refcount. Only DecRefs that cannot go to zero are in this map, so there
  // will be no entries with a value of less than 2.
  smart::hash_map<Point, int> decRefs;
};

/*
 * We often want to insert sink points on control flow edges. idForEdge returns
 * the appropriate id to use for the given edge, assuming it is not a critical
 * edge.
 */
Point idForEdge(const Block* from, const Block* to, const IdMap& ids) {
  auto* next = from->next();
  auto* taken = from->taken();
  assert(next || taken);

  auto before = [&](const IRInstruction& inst) {
    ITRACE(3, "id for B{} -> B{} is before {}\n", from->id(), to->id(), inst);
    return ids.before(inst);
  };
  auto after = [&](const IRInstruction& inst) {
    ITRACE(3, "id for B{} -> B{} is after {}\n", from->id(), to->id(), inst);
    return ids.after(inst);
  };

  if (next && taken) {
    // from has two outgoing edges. Use the beginning of to.
    assert(to->numPreds() == 1 && !to->empty());
    auto it = to->begin();
    assert(it != to->end() && !it->is(DefLabel, BeginCatch));
    return before(*it);
  } else {
    // from has one outgoing edges. Use the end of from.
    assert(!from->empty());
    auto it = from->end();
    --it;
    if (it->isControlFlow()) {
      assert(it->isTerminal());
      return before(*it);
    } else {
      return after(*it);
    }
  }
}

/*
 * SinkPointAnalyzer is responsible for inspecting a trace and determining two
 * things: the latest safe point to sink each IncRef to, and which DecRefs
 * cannot take their src to 0.
 */
struct SinkPointAnalyzer : private LocalStateHook {
  SinkPointAnalyzer(const BlockList* blocks, const IdMap* ids, IRUnit& unit)
    : m_unit(unit)
    , m_blocks(*blocks)
    , m_block(nullptr)
    , m_inst(nullptr)
    , m_ids(*ids)
    , m_takenState(nullptr)
    , m_frameState(unit)
  {}

  SinkPointsMap find() {
    assert(!m_blocks.empty());
    auto& fpInst = m_blocks.front()->front();
    assert(fpInst.is(DefFP));
    m_state.frames.live[&fpInst] = Frame();

    for (auto* block : m_blocks) {
      ITRACE(3, "entering B{}\n", block->id());
      Indent _i;

      m_block = block;
      m_frameState.startBlock(block);

      if (block != m_blocks.front()) {
        assert(m_savedStates.count(block) == 1);
        m_state = mergeStates(m_savedStates[block]);
        m_savedStates.erase(block);
      }

      for (auto& inst : *block) {
        m_inst = &inst;
        processInst();
      }
      m_inst = nullptr;

      // Propagate current state to the next block, if one exists. If we have a
      // taken branch, that information will be propagated in processInst().
      auto* next = block->next();
      auto* taken = block->taken();
      if (next) {
        ITRACE(3, "propagating B{}'s state to next - B{}\n",
               m_block->id(), next->id());
        Indent _i;
        FTRACE(3, "{}", show(m_state));
        m_savedStates[next].emplace_back(block, m_state);
      }

      if (!next && !taken) {
        // This block is terminal. Ensure that all live references have been
        // accounted for.
        ITRACE(3, "finishing terminal B{}\n", block->id());
        Indent _i;
        FTRACE(3, "{}", show(m_state));

        auto showFailure = [&]{
          std::string ret;
          ret += folly::format("Unconsumed reference(s) leaving B{}\n",
                               block->id()).str();
          ret += show(m_state);
          ret += folly::format("{:-^80}\n{}{:-^80}\n",
                               " trace ", m_unit.main()->toString(), "").str();
          return ret;
        };

        // Terminal block. Everything must be resolved.
        for (auto const& valState : m_state.values) {
          always_assert_log(valState.second.realCount == 0 &&
                            valState.second.optDelta() == 0,
                            showFailure);
        }
      }
      FTRACE(3, "\n");

      m_frameState.finishBlock(block);
    }

    return std::move(m_ret);
  }

 private:
  struct IncomingBranch {
    IncomingBranch(const Block* b, const Value& v)
      : from(b)
      , value(v)
    {}

    const Block* from;
    Value value;
  };
  struct IncomingValue {
    Value value;
    smart::vector<IncomingBranch> inBlocks;
  };

  State mergeStates(const IncomingStateVec& states) {
    DEBUG_ONLY auto doTrace = [&] {
      ITRACE(3, "merging {} state(s) into B{}\n", states.size(), m_block->id());
      for (DEBUG_ONLY auto const& inState : states) {
        Indent _i;
        ITRACE(3, "from B{}\n", inState.from->id());
        Indent __i;
        FTRACE(3, "{}", show(inState.state));
      }
    };
    ONTRACE(3, doTrace());

    assert(!states.empty());
    // Short circuit the easy, common case: one incoming state
    if (states.size() == 1) return states.front().state;

    // We start by building a map from values to their merged incoming state
    // and which blocks provide information about the value.

    auto const& firstFrames = states.front().state.frames;
    smart::hash_map<SSATmp*, IncomingValue> mergedValues;
    for (auto const& inState : states) {
      assert(inState.state.frames == firstFrames &&
             "merging states with different FrameStacks is not supported");

      for (auto const& inPair : inState.state.values) {
        auto* value = inPair.first;
        assert(!value->inst()->isPassthrough());
        const bool existed = mergedValues.count(value);
        auto& mergedState = mergedValues[value];

        // If the value was already provided by another block, merge this
        // block's state in.
        if (existed) {
          mergedState.value.merge(inPair.second);
        } else {
          mergedState.value = inPair.second;
        }

        // Register this block as an incoming provider of the value.
        mergedState.inBlocks.emplace_back(inState.from, inPair.second);
      }
    }

    State retState;
    retState.canon = mergeCanons(states);
    retState.frames = firstFrames;

    // Now, for each incoming value, insert it into the resulting state. If
    // there is a difference in a value's state between incoming branches,
    // resolve it by inserting sink points on the appropriate incoming edges.
    for (auto& pair : mergedValues) {
      auto mergedState = pair.second.value;

      // If the value wasn't provided by every incoming branch, we have to
      // completely resolve it in all incoming branches.
      if (pair.second.inBlocks.size() < states.size()) {
        mergedState.pessimize();
      }

      auto* incVal = mapGet(retState.canon, pair.first, pair.first);
      auto const mergedDelta = mergedState.optDelta();
      for (auto& inBlock : pair.second.inBlocks) {
        auto& inState = inBlock.value;
        assert(inState.optDelta() >= mergedDelta);

        Point insertId = idForEdge(inBlock.from, m_block, m_ids);
        while (inState.optDelta() > mergedDelta) {
          ITRACE(3, "Inserting sink point on edge B{} -> B{}\n",
                 inBlock.from->id(), m_block->id());
          m_ret.points[pair.first].insert(std::make_pair(inState.popRef(),
                                           SinkPoint(insertId, incVal, false)));
        }
      }

      // Only put the merged state in the result if it provides useful
      // information.
      assert(mergedState.optCount() >= 0);
      if (mergedState.realCount || mergedState.fromLoad) {
        retState.values[pair.first] = mergedState;
      }
    }

    return retState;
  }

  /* If control flow after something like a CheckType rejoins, we need to make
   * sure we only reference values that dominate the join. In the example
   * below, t4 is not defined in B3 and therefore cannot be used in B4:
   *
   * B1:
   *   t4:Obj = CheckType<Obj> t3:Cell -> B3
   *  fallthrough to B2
   *
   * B2:
   *   IncRef t4:Obj
   *   Jmp B4
   *
   * B3:
   *   IncRef t3:Cell
   *   Jmp B4
   *
   * B2's incoming state will have a mapping from t3 -> t4, and B3 will have no
   * mapping for t3. We need to look at all the incoming mappings and pick the
   * most specific value defined in all incoming paths (t3 in this case).
   */
  CanonMap mergeCanons(const IncomingStateVec& states) const {
    CanonMap retMap;

    auto passthrough = [](SSATmp*& val) -> void {
      val = val->inst()->getPassthroughValue();
    };

    // First, build a map from canonical values to the values they map to and
    // how many incoming branches have the mapped value available.
#ifdef HAVE_BOOST1_49
    smart::flat_map<SSATmp*, smart::flat_map<SSATmp*, int>> mergedCanon;
#else
    smart::map<SSATmp*, smart::map<SSATmp*, int>> mergedCanon;
#endif

    for (auto const& inState : states) {
      for (auto const& pair : inState.state.canon) {
        for (auto* val = pair.second; val->inst()->isPassthrough();
             passthrough(val)) {
          ++mergedCanon[pair.first][val];
        }
      }
    }

    for (auto const& pair : mergedCanon) {
      auto* trueRoot = pair.first;
      // Build a list of versions of this value that exist in all incoming
      // branches.
      smart::flat_set<SSATmp*> tmps;
      for (auto const& countPair : pair.second) {
        if (countPair.first != trueRoot && countPair.second == states.size()) {
          tmps.insert(countPair.first);
        }
      }

      if (tmps.empty()) {
        // None of the derived values exist, so use trueRoot. This is
        // equivalent to not having an entry in the map.
        continue;
      } else if (tmps.size() == 1) {
        // Only one derived value exists in all incoming branches, so use that
        // value.
        retMap[trueRoot] = *tmps.begin();
        continue;
      }

      // We found more than one value coming in from all branches, so find the
      // most derived one. This is the one temp that doesn't appear as an
      // ancestor of any of the others.
      smart::flat_set<SSATmp*> ancestors;
      for (auto* val : tmps) {
        // trueRoot shouldn't be in the set
        assert(val->inst()->isPassthrough());
        passthrough(val);

        // For each ancestor of val, if it's not yet in ancestors, add it and
        // continue. If it is in ancestors we've already visited it and all of
        // its ancestors, so do nothing more.
        for (; val->inst()->isPassthrough(); passthrough(val)) {
          auto it = ancestors.find(val);
          if (it == ancestors.end()) {
            ancestors.insert(val);
          } else {
            break;
          }
        }
      }

      // There should now be exactly one value in tmps that isn't
      // ancestors. This is the value to use in the merge map. The sets are
      // sorted, so walk them in parallel to find the discrepancy.
      assert(ancestors.size() == tmps.size() - 1);
      SSATmp* mostDerived = nullptr;

      for (auto tIt = tmps.begin(), aIt = ancestors.begin(); ; ++tIt, ++aIt) {
        assert(tIt != tmps.end());
        if (aIt == ancestors.end() || *tIt != *aIt) {
          mostDerived = *tIt;
          break;
        }
      }
      retMap[trueRoot] = mostDerived;
    }

    return retMap;
  }

  void processInst() {
    ITRACE(3, "processing instruction id {:>3}: {}\n",
           m_ids.before(m_inst), *m_inst);
    Indent _i;

    auto const nSrcs = m_inst->numSrcs();
    auto const nDsts = m_inst->numDsts();
    auto const bcOp = m_inst->marker().sk().op();
    m_frameState.setMarker(m_inst->marker());

    if (auto* taken = m_inst->taken()) {
      // If an instruction's branch is ever taken, none of its sources will be
      // consumed. This means we should propagate state across this edge before
      // consuming any values and modifying the current state.
      ITRACE(3, "propagating B{}'s state to taken - B{}\n",
             m_block->id(), taken->id());
      Indent _i;
      FTRACE(3, "{}", show(m_state));
      auto& takenStates = m_savedStates[taken];
      takenStates.emplace_back(m_block, m_state);

      // Since we can't insert sink points after the branch but before the
      // instruction consumes its sources, we have to insert them before the
      // whole instruction. This happens after we've already propagated the
      // state to taken, so we keep a pointer to the propagated state so we can
      // update it when needed.
      m_takenState = &takenStates.back().state;
    } else {
      m_takenState = nullptr;
    }

    if (m_inst->is(IncRef, IncRefCtx)) {
      auto* src = m_inst->src(0);

      // We only consider an IncRef optimizable if it's not an IncRefCtx and
      // the value doesn't have an optimized count of 0. This prevents any
      // subsequent instructions from taking in a source with count 0.
      if (src->type().maybeCounted()) {
        auto& valState = m_state.values[canonical(src)];
        if (valState.optCount() > 0 && m_inst->is(IncRef)) {
          auto const id = m_ids.before(m_inst);
          m_state.values[canonical(src)].pushInc(id);
        } else {
          // Even if the IncRef isn't optimizable, it still needs to be tracked
          // as a live reference.
          ++valState.realCount;
        }
        ITRACE(3, "{}\n", show(m_state.values[canonical(src)]));
      }
    } else if (m_inst->is(TakeStack)) {
      // TakeStack is used to indicate that we've logically popped a value off
      // the stack, in place of a LdStack.
      auto* src = m_inst->src(0);
      if (src->type().maybeCounted()) {
        m_state.values[canonical(src)].fromLoad = true;
      }
    } else if (m_inst->is(Jmp)) {
      // Completely resolve the deficit for any sources to Jmp. This isn't a
      // perfect solution but it's vastly simpler than other options and
      // probably good enough for our purposes.
      for (uint32_t i = 0; i < nSrcs; ++i) {
        resolveValue(m_inst->src(i));
      }
    } else if (m_inst->is(DefLabel)) {
      // Values produced by a label take the pessimistic combinations of
      // their incoming values.
      constexpr int32_t kInfCount = 1 << 30;
      auto minCount = kInfCount;
      auto fromLoad = false;
      for (uint32_t i = 0; i < nDsts; ++i) {
        m_block->forEachSrc(i, [&](const IRInstruction* inst, SSATmp* src) {
          src = canonical(src);
          if (src->type().notCounted()) return;

          auto const& valState = m_state.values[src];
          minCount = std::min(minCount, valState.realCount);
          fromLoad = fromLoad || valState.fromLoad;
        });

        assert(IMPLIES(fromLoad, minCount != kInfCount));
        if (minCount != kInfCount) {
          m_state.values.emplace(m_inst->dst(i),
                                 Value(minCount, fromLoad));
        }
      }
    } else if (m_inst == &m_block->back() &&
               (!m_block->taken() && !m_block->next()) &&
               (!m_inst->is(RetCtrl) ||
                bcOp == OpContSuspend || bcOp == OpContSuspendK ||
                bcOp == OpContRetC)) {
      // When leaving a trace, we need to account for all live references in
      // locals and $this pointers.
      consumeAllLocals();
      consumeAllFrames();
    } else if (m_inst->is(GenericRetDecRefs)) {
      consumeAllLocals();
    } else {
      // All other instructions take the generic path.
      consumeInputs();
      defineOutputs();
    }

    m_frameState.update(m_inst);
  }

  /* JIT::canonical() traces through passthrough instructions to get the root
   * of a value. Since we're tracking the state of inlined frames in the trace,
   * there are often cases where the root value for a LdThis is really a value
   * up in some enclosing frame. */
  SSATmp* canonical(SSATmp* value) {
    auto* root = JIT::canonical(value);
    auto* inst = root->inst();
    if (!inst->is(LdThis)) return root;

    auto* fpInst = frameRoot(inst->src(0)->inst());
    auto it = m_state.frames.live.find(fpInst);
    if (it == m_state.frames.live.end()) {
      it = m_state.frames.dead.find(fpInst);
      assert(it != m_state.frames.dead.end());
    }
    return it->second.mainThis;
  }

  void consumeAllLocals() {
    ITRACE(3, "consuming all locals\n");
    Indent _i;
    m_frameState.forEachLocal(
      [&](uint32_t id, SSATmp* value) {
        if (value) consumeValue(value);
      }
    );
  }

  void consumeInputs() {
    if (m_inst->is(DecRef, DecRefNZ)) {
      auto* src = m_inst->src(0);

      if (src->type().notCounted()) return;
      src = canonical(src);

      auto const& valState = m_state.values[src];
      // Record DecRefs that won't go to 0
      assert(IMPLIES(m_inst->is(DecRefNZ), valState.realCount > 1));
      if (valState.realCount > 1) {
        m_ret.decRefs[m_ids.before(m_inst)] = valState.realCount;
      }
    } else if (m_inst->is(SpillFrame)) {
      auto* this_ = m_inst->src(3);
      if (this_->isA(Type::Obj)) {
        m_state.frames.pushPreLive(Frame(canonical(this_), this_));
        // When spilling an Object to a pre-live ActRec, we can reliably track
        // the reference in the frame. This allows us to optimize away many
        // refcounting operations on $this in inlined calls.
        ITRACE(3, "{}({}) becoming $this in an ActRec; not consuming "
               "reference\n",
               *this_, *canonical(this_));
        Indent _i;

        auto& valState = m_state.values[canonical(this_)];
        if (valState.realCount == 0) {
          assert(valState.fromLoad);
          ++valState.realCount;
        }
        ITRACE(3, " after: {}\n", show(valState));
        return;
      }
      m_state.frames.pushPreLive();
    } else if (m_inst->is(DefInlineFP)) {
      m_state.frames.pushInline(m_inst);
    } else if (m_inst->is(InlineReturn)) {
      FTRACE(3, "{}", show(m_state));
      m_state.frames.popInline(frameRoot(m_inst->src(0)->inst()));
    } else if (m_inst->is(RetCtrl) &&
               !m_inst->extra<InGeneratorData>()->inGenerator) {
      m_state.frames.pop();
    } else if (m_inst->is(Call, CallArray)) {
      resolveAllFrames();
      callPreLiveFrame();
    } else if (m_inst->is(ContEnter)) {
      resolveAllFrames();
    } else if (m_inst->is(CallBuiltin) &&
               !strcasecmp(m_inst->src(0)->getValFunc()->fullName()->data(),
                           "get_defined_vars")) {
      observeLocalRefs();
    } else if (m_inst->is(InterpOne, InterpOneCF)) {
      // InterpOne can push and pop ActRecs.
      auto const op = m_inst->extra<InterpOneData>()->opcode;
      if (JIT::getInstrInfo(op).type == JIT::InstrFlags::OutFDesc) {
        m_state.frames.pushPreLive();
      } else if (isFCallStar(op)) {
        resolveAllFrames();
        callPreLiveFrame();
      } else if (op == OpContEnter) {
        resolveAllFrames();
      } else if (op == OpFCallBuiltin) {
        // This isn't optimal, but InterpOne of FCallBuiltin is rare enough
        // that it doesn't matter.
        observeLocalRefs();
      }
    }

    // SpillStack and Call may have some of their sources replaced with None,
    // to indicate that the value doesn't need to be stored again. We still
    // want to trace down the original source to track its refcount state, and
    // we do so by scanning all currently tracked values from LdStacks.
    auto ldStacksLazy = folly::lazy([&]{
      auto* spill = m_inst->is(SpillStack) ? m_inst : m_inst->src(0)->inst();
      assert(spill->is(SpillStack));
      return collectLdStacks(spill->src(0));
    });

    ITRACE(3, "consuming normal inputs\n");
    for (uint32_t i = 0; i < m_inst->numSrcs(); ++i) {
      Indent _i;
      auto* src = m_inst->src(i);

      if (src->isA(Type::None)) {
        assert(m_inst->is(Call, SpillStack));
        auto const isCall = m_inst->is(Call);
        auto const& ldStacks = ldStacksLazy();
        auto* spillInst = isCall ? m_inst->src(0)->inst() : m_inst;
        auto const adjustment =
          spillInst->src(1)->getValInt() - spillValueCells(spillInst);
        uint32_t targetOff;
        if (isCall) {
          targetOff = -(i - 3 + 1) + adjustment;
        } else {
          targetOff = i - 2 + adjustment;
        }

        auto it = ldStacks.find(targetOff);
        if (it != ldStacks.end()) {
          ITRACE(3, "consuming {} instead of src {} of None\n", *it->second, i);
          consumeValue(it->second);
        }
      } else if (src->isA(Type::StkPtr) && src->inst()->is(SpillFrame) &&
                 !m_inst->is(DefInlineFP, DefInlineSP,
                             Call, CallArray, ContEnter)) {
        // If the StkPtr being consumed points to a pre-live ActRec, observe
        // its $this pointer since many of our helper functions decref it.
        auto* this_ = src->inst()->src(3);
        if (this_->isA(Type::Obj)) {
          auto const sinkPoint = SinkPoint(m_ids.before(m_inst), this_, false);
          this_ = canonical(this_);
          observeValue(this_, m_state.values[this_], sinkPoint);
        }
      }

      if (src->type().notCounted()) continue;

      auto& valState = m_state.values[canonical(src)];
      const SinkPoint sp(m_ids.before(m_inst), src, false);
      assert(IMPLIES(valState.optCount() == 0, valState.realCount == 0));
      if (m_inst->consumesReference(i)) {
        consumeValue(canonical(src), valState, sp);
      }
    }

    // Many instructions have complicated effects on the state of the
    // locals. Get this information from FrameState.
    ITRACE(3, "getting local effects from FrameState\n");
    Indent _i;
    m_frameState.getLocalEffects(m_inst, *this);
  }

  /* For all LdStack instructions from the given sp, build a map from stack
   * offset to value. Used above in consumeInputs. */
  smart::hash_map<int32_t, SSATmp*> collectLdStacks(SSATmp* sp) {
    smart::hash_map<int32_t, SSATmp*> ret;

    for (auto const& pair : m_state.values) {
      auto* inst = pair.first->inst();
      if (inst->is(LdStack) && inst->src(0) == sp) {
        auto const offset = inst->extra<LdStack>()->offset;
        ret[offset] = inst->dst();
      }
    }

    return ret;
  }

  /*
   * Process a normal call to a prelive frame. Consume $this, if any, and pop
   * the prelive frame.
   */
  void callPreLiveFrame() {
    // If we have no prelive frames the FPush* was in a different trace.
    if (m_state.frames.preLive.empty()) return;

    auto& frame = m_state.frames.preLive.back();
    if (frame.currentThis) {
      consumeValue(frame.currentThis);
    } else if (frame.mainThis) {
      consumeValueEraseOnly(frame.mainThis);
    }
    m_state.frames.preLive.pop_back();
  }

  template<typename L>
  void forEachFrame(L body) {
    for (auto& pair : m_state.frames.live) body(pair.second);
    for (auto& frame : m_state.frames.preLive) body(frame);
  }

  /*
   * Call, CallArray, and ContEnter destroy all live values. They also might
   * throw and we can't attach catch traces to them. Resolve the opt delta for
   * any live $this pointers and forget the "current" this pointer so we don't
   * reuse it after the call.
   */
  void resolveAllFrames() {
    forEachFrame([this](Frame& f) {
      if (f.currentThis) {
        resolveValue(f.currentThis);
        f.currentThis = nullptr;
      } else if (f.mainThis) {
        resolveValueEraseOnly(f.mainThis);
      }
    });
  }

  void consumeAllFrames() {
    forEachFrame([this](Frame& f) {
      if (f.currentThis) {
        consumeValue(f.currentThis);
      } else if (f.mainThis) {
        consumeValueEraseOnly(f.mainThis);
      }
    });
  }

  /* When an instruction consumes a reference to a value, two things in our
   * tracked state must be updated. We first ensure that the behavior of DecRef
   * and COW will not be affected by the difference between the real count and
   * the optimized count. This is done by inserting as many IncRef sink points
   * as needed before the instruction. Second, the real count of the value is
   * decremented to reflect the consumption of the reference. If there is no
   * reference to consume, this is either a bug in this analysis pass or the
   * incoming IR, so abort with a (hopefully) helpful error. */
  void consumeValueImpl(SSATmp* value, bool eraseOnly) {
    assert(value);
    if (value->type().notCounted()) return;

    auto* root = canonical(value);
    consumeValue(root, m_state.values[root],
                 SinkPoint(m_ids.before(m_inst), value, eraseOnly));
  }
  void consumeValue(SSATmp* value)          { consumeValueImpl(value, false); }
  void consumeValueEraseOnly(SSATmp* value) { consumeValueImpl(value, true); }

  void consumeValue(SSATmp* value, Value& valState, SinkPoint sinkPoint) {
    ITRACE(3, "consuming value {}\n", *value->inst());
    assert(value == canonical(value));
    Indent _i;

    ITRACE(3, "before: {}\n", show(valState));
    assert(valState.realCount >= 0);
    assert(valState.optCount() >= 0);

    assertCanConsume(value);

    // Note that we're treating consumers and observers the same here, which is
    // necessary until we have better alias analysis.
    observeValue(value, valState, sinkPoint);
    if (valState.realCount) --valState.realCount;

    ITRACE(3, " after: {}\n", show(valState));
  }

  /* DecRef and COW both care about the same thing: is _count == 1 coming into
   * the operation? */
  struct ConsumeObserver {
    bool operator()(int count) { return count == 1; }
  };
  /* When deciding if something is a user-visible reference, we need to know if
   * _count >= 2. */
  struct RefObserver {
    bool operator()(int count) { return count >= 2; }
  };

  template<typename T = ConsumeObserver>
  void observeValue(SSATmp* value, Value& valState, SinkPoint sp,
                    T pred = T()) {
    while (pred(valState.realCount) != pred(valState.optCount())) {
      placeSinkPoint(value, valState, sp);
    }
  }

  void placeSinkPoint(SSATmp* value, Value& valState, SinkPoint sp) {
    m_ret.points[value].insert(std::make_pair(valState.popRef(), sp));
    ITRACE(3, "placing IncRef at id {}\n", sp.id);

    // If the current instruction has a taken branch, we need to tell the
    // propagated state that we used one of the pending IncRefs.
    if (m_takenState && sp.id == m_ids.before(m_inst)) {
      ITRACE(3, "updating taken state for {}\n", *value);
      m_takenState->values[value].popRef();
    }
  }

  void assertCanConsumeImpl(SSATmp* value, bool checkConsume) {
    auto const& valState = m_state.values[value];
    auto showState = [&](const std::string& what) {
      std::string ret;
      ret += folly::format("'{}' wants to consume {} but {}\n",
                           *m_inst, *value, what).str();
      ret += show(m_state);
      ret += folly::format("{:-^80}\n{}{:-^80}\n",
                           " trace ", m_unit.main()->toString(), "").str();
      return ret;
    };

    if (valState.realCount == 0) {
      auto showFailure = [&] {
        return showState("it has no unconsumed references");
      };

      // This is ok as long as the value came from a load (see the Value struct
      // for why) or it's from a phi node where one or more of the incoming
      // values has an uncounted type (when we process the DefLabel we take the
      // pessimistic combination of all inputs and the uncounted value won't
      // have any references).
      auto uncountedPhiSource = [&]{
        auto* inst = value->inst();
        if (!inst->is(DefLabel)) return false;
        for (uint32_t i = 0; i < inst->numDsts(); ++i) {
          auto foundUncounted = false;
          inst->block()->forEachSrc(i,
            [&](const IRInstruction* inst, SSATmp* src) {
              if (src->type().notCounted()) foundUncounted = true;
            }
          );
          if (foundUncounted) return true;
        }
        return false;
      };

      always_assert_log((valState.fromLoad && valState.optCount() == 0) ||
                        uncountedPhiSource(),
                        showFailure);
    } else if (checkConsume) {
      auto showFailure = [&] {
        return showState(folly::format("it has an optCount of {}\n",
                                       valState.optCount()).str());
      };
      always_assert_log(valState.optCount() >= 1, showFailure);
    }
  }

  void assertHasUnconsumedReference(SSATmp* value) {
    assertCanConsumeImpl(value, false);
  }

  void assertCanConsume(SSATmp* value) {
    assertCanConsumeImpl(value, true);
  }

  void observeLocalRefs() {
    // If any locals are RefDatas, the behavior of get_defined_vars can
    // depend on whether or not the count is >= 2.
    m_frameState.forEachLocal(
      [&](uint32_t id, SSATmp* value) {
        if (!value) return;

        auto const sp = SinkPoint(m_ids.before(m_inst), value, false);
        value = canonical(value);
        observeValue(value, m_state.values[value], sp, RefObserver());
      }
    );
  }

  void consumeLocal(Point id) {
    if (auto* val = m_frameState.localValue(id)) {
      consumeValue(val);
    }
  }

  /* Completely resolve the delta between realCount and optCount. */
  void resolveValueImpl(SSATmp* const origVal, bool eraseOnly) {
    assert(origVal);

    if (origVal->type().notCounted()) return;
    auto* value = canonical(origVal);

    ITRACE(3, "resolving value {}\n", *value->inst());
    Indent _i;

    auto& valState = m_state.values[value];
    if (!valState.optDelta()) return;

    const SinkPoint sp(m_ids.before(m_inst), origVal, eraseOnly);
    while (valState.optDelta()) {
      placeSinkPoint(value, valState, sp);
    }
  }
  void resolveValue(SSATmp* val)          { resolveValueImpl(val, false); }
  void resolveValueEraseOnly(SSATmp* val) { resolveValueImpl(val, true); }

  /* Remember that oldVal has been replace by newVal, either because of a
   * passthrough instruction or something from FrameState. */
  void replaceValue(SSATmp* oldVal, SSATmp* newVal) {
    ITRACE(3, "replacing {} with {}\n", *oldVal, *newVal);

    assert(canonical(oldVal) == canonical(newVal) &&
           oldVal != newVal);
    m_state.canon[canonical(newVal)] = newVal;
  }

  void defineOutputs() {
    if (m_inst->is(LdLoc)) {
      // LdLoc's output is the new value of the local we loaded, and this has
      // already been tracked in setLocalValue().
      return;
    } else if (m_inst->is(LdThis)) {
      auto* fpInst = frameRoot(m_inst->src(0)->inst());
      assert(m_state.frames.live.count(fpInst));
      auto& frame = m_state.frames.live[fpInst];
      frame.currentThis = m_inst->dst();
      if (frame.mainThis == nullptr) {
        // Nobody has tried to load $this in the current frame yet. Set it as
        // the main $this for the frame and indicate that we're tracking a
        // reference to it.
        frame.mainThis = m_inst->dst();
        m_state.values[frame.mainThis].realCount = 1;
      }
      return;
    }

    if (m_inst->isPassthrough()) {
      assert(m_inst->numDsts() == 1);
      auto* dst = m_inst->dst();
      auto* src = m_inst->src(0);
      if (dst->type().maybeCounted()) {
        replaceValue(src, dst);
      }

      if (m_inst->is(CheckType, AssertType)) {
        // If we're doing CheckType or AssertType from a counted type to an
        // uncounted type, we need to do something with the pending references
        // that won't be consumed now that the type is uncounted. Since we now
        // know that for its entire lifetime, the value has been an uncounted
        // type, we consume a live reference and drop all pending IncRefs on
        // the floor. This will result in any IncRefs of this value being sunk
        // to the CheckType's taken branch, and erased completely from the path
        // falling through the CheckType.
        if (dst->type().notCounted() && src->type().maybeCounted()) {
          auto* src = canonical(dst);
          auto& valState = m_state.values[src];
          assertHasUnconsumedReference(src);

          ITRACE(3, "consuming reference to {}: {} and dropping "
                 "opt delta of {}\n",
                 *src, show(valState), valState.optDelta());
          if (valState.realCount) --valState.realCount;
          while (valState.optDelta()) valState.popRef();
        } else if (dst->type().maybeCounted() && src->type().notCounted()) {
          // Going from notCounted to maybeCounted. This sounds silly, but it's
          // possible so we have to handle it "correctly". Treat it like a
          // load instruction.
          ITRACE(3, "producing fake reference to {}\n", *dst);
          m_state.values[canonical(dst)].fromLoad = true;
        }
      }

      return;
    }

    // Define any values produced by the instruction.
    for (uint32_t i = 0; i < m_inst->numDsts(); ++i) {
      auto dst = m_inst->dst(i);

      if (dst->type().notCounted()) continue;

      auto pair = m_state.values.emplace(dst, Value());
      assert(pair.second);

      ITRACE(3, "defining value {}\n", *m_inst);
      Indent _i;

      auto& state = pair.first->second;
      if (m_inst->producesReference(i)) state.realCount = 1;

      if (m_inst->is(LdMem, LdRef, LdStack, LdElem, LdProp,
                     LdPackedArrayElem, Unbox)) {
        state.fromLoad = true;
      }
      ITRACE(3, "{}\n", show(state));
      assert(state.optDelta() == 0);
    }
  }

  ///// LocalStateHook overrides /////
  void setLocalValue(uint32_t id, SSATmp* newVal) override {
    assert(IMPLIES(m_inst->is(LdLoc), m_frameState.localValue(id) == nullptr));

    // When a local's value is updated by StLoc(NT), the consumption of the old
    // value should've been visible to us, so we ignore that here.
    if (!m_inst->is(StLoc, StLocNT)) {
      ITRACE(3, "consuming local {} for setLocalValue\n", id);
      Indent _i;
      consumeLocal(id);
    }

    if (newVal && newVal->type().maybeCounted()) {
      ITRACE(3, "{} is now in a local, adding tracked reference\n",
             *newVal);
      Indent _i;
      auto& valState = m_state.values[canonical(newVal)];
      ++valState.realCount;
      if (m_inst->is(LdLoc)) valState.fromLoad = true;
      ITRACE(3, " after: {}\n", show(valState));
    }
  }

  void refineLocalValue(uint32_t id, SSATmp* oldVal, SSATmp* newVal) {
    if (oldVal->type().maybeCounted() && newVal->type().notCounted()) {
      assert(newVal->inst()->is(CheckType));
      assert(newVal->inst()->src(0) == oldVal);
      // Similar to what we do when processing the CheckType directly, we
      // "consume" the value on behalf of the CheckType.
      oldVal = canonical(oldVal);
      auto& valState = m_state.values[oldVal];
      assertHasUnconsumedReference(oldVal);
      if (valState.realCount) --valState.realCount;
    }
  }

  void setLocalType(uint32_t id, Type) override {
    ITRACE(3, "consuming local {} for setLocalType\n", id);
    Indent _i;
    consumeLocal(id);
  }

  void killLocalForCall(uint32_t id, unsigned inlineIdx,
                        SSATmp* value) override {
    ITRACE(3, "consuming local {} at inline level {} for killLocalForCall\n",
           id, inlineIdx);
    Indent _i;
    consumeValue(value);
  }

  void updateLocalRefValue(uint32_t id, unsigned,
                           SSATmp* oldVal, SSATmp* newVal) override {
    ITRACE(3, "replacing {} with {} in local {} for updateLocalRefValue\n",
           *oldVal, *newVal, id);
    replaceValue(oldVal, newVal);
  }

  /* The IRUnit being processed and its blocks */
  const IRUnit& m_unit;
  const BlockList& m_blocks;

  /* Current block and current instruction */
  const Block* m_block;
  const IRInstruction* m_inst;

  /* Map between linear ids and IRInstruction* */
  const IdMap& m_ids;

  /* Current state of the world */
  State m_state;

  /* Sometimes present pointer to state propagated to m_inst's taken
   * branch. See consumeValue() for usage. */
  State* m_takenState;
  SavedStates m_savedStates;

  /* Analysis results to be returned to caller */
  SinkPointsMap m_ret;

  /* Used to track local state and other information about the trace */
  FrameState m_frameState;
};

////////// Refcount validation pass //////////
typedef smart::hash_map<SSATmp*, double> TmpDelta;
typedef smart::hash_map<Block*, TmpDelta> BlockMap;

/*
 * Simulate the effect of block b on refcounts of SSATmps.
 */
void updateCounts(Block* b, TmpDelta& delta) {
  for (auto& inst: *b) {
    if (inst.is(IncRef, DecRef, DecRefNZ)) {
      SSATmp* src = canonical(inst.src(0));
      if (src->type().notCounted()) continue;

      if (inst.is(IncRef)) {
        delta[src]++;
      } else {
        delta[src]--;
      }
    }
  }
}

/*
 * Populate each block in BlockMap with a map of the refcount delta for each
 * SSATmp. Ideally, we want to maintain the map for every path through the
 * trace. However, to avoid the exponential cost, we use a linear-time
 * approximation here using an idea similar to Random Interpretation (Gulwani
 * and Necula, POPL 2003). The logic for phi-nodes in the approximiation
 * considers each predecessor equally likely, and uses the average as the
 * incoming ref-count for the phi-node.
 */
void getRefDeltas(IRUnit& unit, BlockMap& map) {
  auto blocks = rpoSortCfg(unit);
  for (auto* block : blocks) {
    TmpDelta& delta = map[block];
    int numPreds = block->numPreds();
    block->forEachPred([&](Block* from) {
      TmpDelta& predMap = map[from];
      for (auto it = predMap.begin(), end = predMap.end(); it != end; ) {
        SSATmp* src = it->first;
        delta[src] += it->second / numPreds;
        ++it;
      }
    });
    updateCounts(block, delta);
  }
}

const double deltaThreshold = 1E-15;

/*
 * Validate that orig and opt have the same ref-count deltas for SSATmps in
 * each exit block.
 */
bool validateDeltas(BlockMap& orig, BlockMap& opt) {
  for (auto it = opt.begin(), end = opt.end(); it != end; ++it) {
    if (!it->first->isExit()) {
      continue;
    }
    TmpDelta& deltaOpt = it->second;
    TmpDelta& deltaOrig = orig[it->first];
    for (auto it2 = deltaOpt.begin(), end2 = deltaOpt.end(); it2 != end2;
           it2++) {
      SSATmp* src = it2->first;
      double delta = deltaOrig[src];
      if (fabs(delta - it2->second) > deltaThreshold) {
        if (src->inst()->is(LdThis)) {
          // The optimization does some nontrivial state tracking to keep track
          // of $this pointers, so this is probably a false positive.
          FTRACE(1, "possible ");
        }
        FTRACE(1, "refcount mismatch in {}: orig: {} opt: {}, block {}\n",
                   src->toString(),
                   deltaOrig[src],
                   it2->second,
                   it->first->id());
        return src->inst()->is(LdThis);
      }
    }
  }
  return true;
}

std::string show(const SinkPointsMap& sinkPoints, const IdMap& ids) {
  std::string ret;

  typedef std::pair<SSATmp*, SinkPoints> SinkPair;
  smart::vector<SinkPair> sortedPoints(
    sinkPoints.points.begin(), sinkPoints.points.end());
  std::sort(sortedPoints.begin(), sortedPoints.end(),
            [&](const SinkPair& a, const SinkPair& b) {
              return ids.before(a.first->inst()) < ids.before(b.first->inst());
            });

  for (auto const& pair : sortedPoints) {
    auto* value = pair.first;
    auto& points = pair.second;
    ret += folly::format("  {}\n", *value->inst()).str();

    const IncSet* lastIncs = nullptr;
    for (auto const& point : points) {
      auto* thisIncs = &point.first;
      if (lastIncs != thisIncs) {
        ret += folly::format("    {}\n", show(*thisIncs, ids)).str();
        lastIncs = thisIncs;
      }
      ret += folly::format("      {} {} (using {}){}\n",
                           (point.second.id % 2) ? "after " : "before",
                           *ids.inst(point.second.id),
                           *point.second.value,
                           point.second.eraseOnly ? ", erase only" : "").str();

    }
    ret += '\n';
  }

  typedef std::pair<Point, int> DecRefPair;
  smart::vector<DecRefPair> sortedDecs(
    sinkPoints.decRefs.begin(), sinkPoints.decRefs.end());
  std::sort(sortedDecs.begin(), sortedDecs.end(),
            [](const DecRefPair& a, const DecRefPair& b) {
              return a.first < b.first;
            });
  for (auto const& pair : sortedDecs) {
    ret += folly::format("  {} has incoming count >= {}\n",
                         *ids.inst(pair.first), pair.second).str();
  }

  return ret;
}

/* Using the information from SinkPointAnalyzer, sink IncRefs when and where
 * appropriate. This pass only removes and inserts IncRef instructions. */
void sinkIncRefs(IRUnit& unit, const SinkPointsMap& info, const IdMap& ids) {
  ITRACE(3, "optimizing refcounts\n");
  Indent _i;

  for (auto const& pair : info.points) {
    auto& incRefs = pair.second;
    ITRACE(3, "looking at value {}, with {} IncRefs\n",
           *pair.first, incRefs.size());
    Indent _i;
    auto it = incRefs.begin();
    while (it != incRefs.end()) {
      auto const& incs = it->first;
      auto const range = incRefs.equal_range(incs);
      auto const end = range.second;
      ITRACE(3, "processing {}\n", show(incs, ids));
      Indent _i;

      // Eval.HHIRRefcountOptsAlwaysSink allows us to stress test the sinking
      // logic.
      auto doSink = RuntimeOption::EvalHHIRRefcountOptsAlwaysSink;
      if (it == end) {
        // There are no sink points for this IncRef. This can only happen if
        // the value started out as a maybeCounted type and was refined to a
        // notCounted type by an AssertType (if it was refined by a CheckType
        // there should be a sink point on the taken branch). In this case
        // we'll intentionally delete the IncRef down below without sinking any
        // copies anywhere.
        doSink = true;
      }

      // For each sink point of this IncRef, check if doing the sink would
      // allow us to optimize it. If we find no optimizable sink points, don't
      // bother sinking.
      for (; it != end; ++it) {
        auto const& sp = it->second;
        if (sp.eraseOnly) {
          // An "erase only" sink point means we've proven that it's safe to
          // sink the IncRef to this point but we don't have access to the
          // value, probably because the SSATmp was killed by a Call. For now
          // we refuse to optimize this case, but it's possible to use this
          // sink point as long as we've proven that doing so will eliminte the
          // Inc/Dec pair.
          ITRACE(2, "found erase-only SinkPoint; not optimizing\n");
          doSink = false;
          break;
        }

        if (IdMap::isAfter(sp.id)) continue;

        auto canOptimize = [&](IRInstruction* inst) {
          // If inst is a DecRef that won't go to zero of the same value as the
          // IncRef, we're good.
          if (info.decRefs.count(ids.before(inst)) &&
              sp.value == canonical(inst->src(0))) {
            return true;
          }

          // If the current type of the value is more specific than the
          // original source of the IncRef, sink it to get a possibly cheaper
          // IncRef.
          if (sp.value->type() < ids.inst(*incs.begin())->src(0)->type()) {
            return true;
          }

          return false;
        };

        if (canOptimize(ids.inst(sp.id))) {
          doSink = true;
        }
      }

      if (!doSink) {
        ITRACE(3, "no optimizable sink points\n");
        it = end;
        continue;
      }

      ITRACE(3, "sinking {}\n", show(incs, ids));
      Indent __i;
      for (auto id : incs) {
        auto* incRef = ids.inst(id);
        incRef->block()->erase(incRef);
      }

      for (it = range.first; it != end; ++it) {
        auto const& point = it->second;
        auto destId = point.id;
        auto* destInst = ids.inst(destId);
        auto* destBlock = destInst->block();
        auto destIt = destBlock->iteratorTo(destInst);
        if (IdMap::isAfter(point.id)) ++destIt;

        auto* newInc = unit.gen(IncRef, destInst->marker(), point.value);
        ITRACE(2, "inserting {} {} {}\n",
               *newInc,
               IdMap::isBefore(point.id) ? "before" : "after",
               *destInst);
        destBlock->insert(destIt, newInc);
      }
    }
  }
  ITRACE(2, "\n");
}

/* If an IncRef of a value is followed immediately by a DecRef of the same
 * value that is guaranteed to not destroy the value, erase both
 * instructions. */
void eliminateRefcounts(IRUnit& unit, const SinkPointsMap& info,
                        const IdMap& ids) {
  for (auto const& pair : info.decRefs) {
    auto* decRef = ids.inst(pair.first);
    auto* decBlock = decRef->block();
    auto decIt = decBlock->iteratorTo(decRef);
    if (decIt == decBlock->begin()) continue;

    auto incIt = decIt;
    --incIt;
    auto* incRef = &*incIt;
    if (!incRef->is(IncRef) ||
        canonical(incRef->src(0)) != canonical(decRef->src(0))) {
      // We can't erase an IncRef/DecRef pair but we can turn the DecRef into
      // the cheaper DecRefNZ.
      ITRACE(2, "changing {} into DecRefNZ\n", *decRef);
      decRef->setOpcode(DecRefNZ);
      continue;
    }

    ITRACE(2, "erasing {} and {}\n", *incIt, *decIt);
    incRef->convertToNop();
    decRef->convertToNop();
  }
  ITRACE(2, "\n");
}

}

/* optimizeRefcounts attempts to remove IncRef/DecRef pairs when we can prove
 * that doing so won't alter the visible behavior of the program. It does so in
 * three phases:
 *
 * - Analysis: Each reachable instruction is inspected to determine its effect
 *   on the refcount of its sources and dests. We keep track of two values for
 *   each SSATmp: the real count and the optimized count. The real count is a
 *   lower bound on the refcount of the object, assuming all IncRefs and
 *   DecRefs are left as is. The optimized count is a lower bound with as many
 *   IncRefs delayed as possible. When we get to an instruction that consumes a
 *   reference to one of its sources, we ensure that the delta between
 *   realCount and optCount won't cause a behavioral difference by logically
 *   inserting as many IncRefs as needed. This phase doesn't modify the code;
 *   it just records where the IncRefs may be sunk to.
 *
 * - IncRef sinking: Using the result of the previous phase, any IncRefs for
 *   which sinking would enable optimizations are moved to their sink
 *   points. Depending on the cfg being optimized, this may change the number
 *   of IncRef instructions present in the trace. The number of IncRefs
 *   executed in each control flow path will not be modified.
 *
 * - IncRef/DecRef removal: All of the hard work was done in the first two
 *   passes, leaving a simple peephole optimization. When an IncRef has been
 *   pushed down to right before the DecRef consuming its reference and that
 *   DecRef is guaranteed to not destroy the object, we can delete both
 *   instructions.
 *
 * SinkPointAnalyzer verifies that each produced reference is consumed exactly
 * once in all paths as it walks through the trace. After the optimization is
 * complete, a separate validation pass is run to ensure the net effect on the
 * refcount of each object has not changed.
 */
void optimizeRefcounts(IRUnit& unit) noexcept {
  FTRACE(2, "vvvvvvvvvv refcount opts vvvvvvvvvv\n");
  auto const changed = splitCriticalEdges(unit);
  if (changed) {
    dumpTrace(6, unit, "after splitting critical edges for refcount opts");
  }

  auto const blocks = rpoSortCfg(unit);
  IdMap ids(blocks, unit);

  Indent _i;
  auto const sinkPoints = SinkPointAnalyzer(&blocks, &ids, unit).find();
  ITRACE(2, "Found sink points:\n{}\n", show(sinkPoints, ids));

  BlockMap before;
  if (RuntimeOption::EvalHHIRValidateRefCount) {
    getRefDeltas(unit, before);
  }

  sinkIncRefs(unit, sinkPoints, ids);
  eliminateRefcounts(unit, sinkPoints, ids);

  if (RuntimeOption::EvalHHIRValidateRefCount) {
    BlockMap after;
    getRefDeltas(unit, after);
    if (!validateDeltas(before, after)) {
      dumpTrace(0, unit, "after refcount optimization");
      always_assert(false && "refcount validation failed");
    }
  }

  FTRACE(2, "^^^^^^^^^^ refcount opts ^^^^^^^^^^\n");
}

} }
