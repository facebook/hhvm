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

#ifndef incl_HPHP_RUNTIME_VM_JIT_FRAME_STATE_H_
#define incl_HPHP_RUNTIME_VM_JIT_FRAME_STATE_H_

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <vector>

#include "folly/Optional.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-source.h"

namespace HPHP {

struct Func;

namespace jit {

struct IRInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

struct EvalStack {
  explicit EvalStack() {}

  void push(SSATmp* tmp) {
    m_vector.push_back(tmp);
  }

  SSATmp* pop() {
    if (m_vector.size() == 0) {
      return nullptr;
    }
    SSATmp* tmp = m_vector.back();
    m_vector.pop_back();
    return tmp;
  }

  SSATmp* top(uint32_t offset = 0) const {
    if (offset >= m_vector.size()) {
      return nullptr;
    }
    uint32_t index = m_vector.size() - 1 - offset;
    return m_vector[index];
  }

  void replace(uint32_t offset, SSATmp* tmp) {
    assert(offset < m_vector.size());
    uint32_t index = m_vector.size() - 1 - offset;
    m_vector[index] = tmp;
  }

  uint32_t numCells() const {
    uint32_t ret = 0;
    for (auto& t : m_vector) {
      ret += t->type() == Type::ActRec ? kNumActRecCells : 1;
    }
    return ret;
  }

  bool empty() const { return m_vector.empty(); }
  int  size()  const { return m_vector.size(); }
  void clear()       { m_vector.clear(); }

  void swap(jit::vector<SSATmp*>& vector) {
    m_vector.swap(vector);
  }

private:
  jit::vector<SSATmp*> m_vector;
};

//////////////////////////////////////////////////////////////////////

/*
 * LocalState stores information about a local variable in the current
 * function.
 */
struct LocalState {
  /*
   * The current value of the local.
   */
  SSATmp* value{nullptr};

  /*
   * The current type of the local.
   */
  Type type{Type::Gen};

  /*
   * Prediction for the type of a local, if it's boxed, otherwise Bottom.
   *
   * Invariants:
   *   always a subtype of `type'
   *   always a subtype of BoxedInitCell
   *   only boxed if `type' is also boxed
   */
  Type boxedPrediction{Type::Bottom};

  /*
   * The sources of the currently known type. They may be values. If the value
   * is unavailable, we won't hold onto it in the value field, but we'll keep
   * it around in typeSrcs for guard relaxation.
   */
  TypeSourceSet typeSrcs;
};

inline bool operator==(const LocalState& a, const LocalState& b) {
  return a.value           == b.value &&
         a.type            == b.type &&
         a.boxedPrediction == b.boxedPrediction &&
         a.typeSrcs        == b.typeSrcs;
}

//////////////////////////////////////////////////////////////////////

/*
 * TODO: rename
 */
struct Snapshot {
  /*
   * Current Func, VM stack pointer, VM frame pointer, offset between sp and
   * fp, and bytecode position.
   */
  const Func* curFunc;
  SSATmp* spValue{nullptr};
  SSATmp* fpValue{nullptr};
  int32_t spOffset;
  BCMarker marker;

  /*
   * m_thisAvailable tracks whether the current frame is known to have a
   * non-null $this pointer.
   */
  bool thisAvailable{false};

  /*
   * Tracking of the state of the virtual execution stack:
   *
   *   During HhbcTranslator's run over the bytecode, these stacks
   *   contain SSATmp values representing the execution stack state
   *   since the last SpillStack.
   *
   *   The EvalStack contains cells and ActRecs that need to be
   *   spilled in order to materialize the stack.
   *
   *   m_stackDeficit represents the number of cells we've popped off
   *   the virtual stack since the last sync.
   */
  uint32_t stackDeficit{0};
  EvalStack evalStack;

  /*
   * Vector of local variable inforation; sized for numLocals on the curFunc
   * (if the state is initialized).
   */
  jit::vector<LocalState> locals;

  /*
   * m_frameSpansCall is true iff a Call instruction has been seen since the
   * definition of the current frame pointer.
   */
  bool frameSpansCall{false};
};

inline bool operator==(const Snapshot& a, const Snapshot& b) {
  return
    a.spValue        == b.spValue &&
    a.fpValue        == b.fpValue &&
    a.curFunc        == b.curFunc &&
    a.spOffset       == b.spOffset &&
    a.thisAvailable  == b.thisAvailable &&
    a.locals         == b.locals &&
    a.frameSpansCall == b.frameSpansCall &&
    a.marker         == b.marker;
}

//////////////////////////////////////////////////////////////////////

/*
 * LocalStateHook is used to separate the acts of determining which locals are
 * affected by an instruction and recording those changes. It allows consumers
 * of FrameStateMgr to get details about how an instruction affects the locals
 * without having to query the state of each local before and after having
 * FrameStateMgr process the instruction.
 */
struct LocalStateHook {
  virtual void setLocalValue(uint32_t id, SSATmp* value) {}
  virtual void refineLocalValue(uint32_t id, unsigned inlineIdx,
                                SSATmp* oldVal, SSATmp* newVal) {}
  virtual void killLocalForCall(uint32_t id,
                                unsigned inlineIdx,
                                SSATmp* val) {}
  virtual void dropLocalInnerType(uint32_t id, unsigned inlineIdx) {}

  virtual void refineLocalType(uint32_t id, Type type, TypeSource typeSrc) {}
  virtual void setLocalType(uint32_t id, Type type) {}
  virtual void setBoxedLocalPrediction(uint32_t id, Type type) {}
  virtual void setLocalTypeSource(uint32_t id, TypeSource typeSrc) {}
};

//////////////////////////////////////////////////////////////////////

/*
 * FrameStateMgr tracks state about the VM stack frame in the function currently
 * being translated. It is responsible for both storing the state and updating
 * it appropriately as instructions and blocks are processed.
 *
 * The types of state tracked by FrameStateMgr include:
 *
 *   - value availability
 *
 *      Used for value propagation and tracking which values can be CSE'd.
 *
 *   - local types and values
 *
 *      We track the current view of these types as we link in new instructions
 *      that mutate them.
 *
 *   - current frame and stack pointers
 *
 *   - current function and bytecode offset
 */
struct FrameStateMgr final : private LocalStateHook {
  FrameStateMgr(IRUnit& unit, BCMarker firstMarker);
  FrameStateMgr(IRUnit& unit, Offset initialSpOffset, const Func* func);

  FrameStateMgr(const FrameStateMgr&) = delete;
  FrameStateMgr& operator=(const FrameStateMgr&) = delete;

  FrameStateMgr(FrameStateMgr&&) = default;

  /*
   * Update state by computing the effects of an instruction.
   *
   * Returns true iff the state for the instruction's taken edge is changed.
   */
  bool update(const IRInstruction*);

  /*
   * Whether we have state saved for the given block.
   */
  bool hasStateFor(Block*) const;

  /*
   * Return an unprocessed predecessor of the given block, or nullptr if none
   * exists.
   */
  Block* findUnprocessedPred(Block*) const;

  /*
   * Starts tracking state for a block and reloads any previously saved
   * state. Can set local values to null if hitting a block with an
   * unprocessed predecessor, so we pass in an optional LocalStateHook. The
   * isLoopHeader parameter is used during initial IR generation to indicate
   * that the given block has a predecessor in the region that might not yet
   * be linked into the IR cfg.
   */
  void startBlock(Block* b,
                  BCMarker marker,
                  LocalStateHook* hook = nullptr,
                  bool isLoopHeader = false);

  /*
   * Finish tracking state for a block and save the current state to
   * any successors.
   *
   * Returns true iff the out-state for the block has changed.
   */
  bool finishBlock(Block*);

  /*
   * Save current state of a block so we can resume processing it after working
   * on another.
   *
   * Leaves the current state for this FrameStateMgr untouched: if you
   * startBlock something new it'll keep using it.  Right now we rely on this
   * for exit and catch traces (relevant: TODO(#4323657)).
   */
  void pauseBlock(Block*);

  /*
   * Resumes processing a block that was stopped by pauseBlock.
   */
  void unpauseBlock(Block*);

  /*
   * Clear state associated with the given block.
   */
  void clearBlock(Block*);

  /*
   * Clear all tracked state, including both the current state and the
   * state associated with all blocks.
   */
  void clear();

  /*
   * Clear the CSE table.
   */
  void clearCse();

  /*
   * Iterates through a control-flow graph, until a fixed-point is
   * reached. Drops all previous state.
   */
  void computeFixedPoint(const BlocksWithIds&);

  /*
   * Loads the in-state for a block. Requires that the block has already been
   * processed. Intended to be used after computing the fixed-point of a CFG.
   */
  void loadBlock(Block*);

  const Func* func() const { return cur().curFunc; }
  Offset spOffset() const { return cur().spOffset; }
  SSATmp* sp() const { return cur().spValue; }
  SSATmp* fp() const { return cur().fpValue; }
  bool thisAvailable() const { return cur().thisAvailable; }
  void setThisAvailable() { cur().thisAvailable = true; }
  bool frameSpansCall() const { return cur().frameSpansCall; }
  BCMarker marker() const { return cur().marker; }
  void setMarker(BCMarker m) { cur().marker = m; }
  unsigned inlineDepth() const { return m_stack.size() - 1; }
  uint32_t stackDeficit() const { return cur().stackDeficit; }
  void incStackDeficit() { cur().stackDeficit++; }
  void clearStackDeficit() { cur().stackDeficit = 0; }
  EvalStack& evalStack() { return cur().evalStack; }
  bool enableCse() const { return m_enableCse; }
  void setEnableCse(bool e) { m_enableCse = e; }

  void setBuilding(bool b) { m_status = b ? Status::Building : Status::None; }

  Type localType(uint32_t id) const;
  Type predictedInnerType(uint32_t id) const;
  SSATmp* localValue(uint32_t id) const;
  TypeSourceSet localTypeSources(uint32_t id) const;

  typedef std::function<void(SSATmp*, int32_t)> FrameFunc;
  // Call func for all enclosing frames, starting with the current one and
  // proceeding upward through callers.
  void forEachFrame(FrameFunc func) const;

  typedef std::function<void(uint32_t, SSATmp*)> LocalFunc;
  // Call func with all tracked locals, including callers if this is an inlined
  // frame.
  void forEachLocal(LocalFunc func) const;

  SSATmp* cseLookup(IRInstruction* inst,
                    Block* srcBlock,
                    const folly::Optional<IdomVector>&);

  void getLocalEffects(const IRInstruction* inst, LocalStateHook& hook) const;

  /*
   * What the FrameStateMgr is doing.
   *
   * Building - Changes when we propagate state to taken blocks.
   *
   * RunningFixedPoint - Changes how we handle predecessors we haven't visited
   * yet.
   *
   * FinishedFixedPoint - Stops us from merging new state to blocks.
   */
  enum class Status : uint8_t {
    None,
    Building,
    RunningFixedPoint,
    FinishedFixedPoint,
  };

  /*
   * Info about state leaving a block. The block must have already been
   * processed.
   */
  const jit::vector<LocalState>& localsLeavingBlock(Block*) const;
  SSATmp* spLeavingBlock(Block*) const;

  /*
   * Marks a block as visited in the current iteration.
   * FrameStateMgr::startBlock does this automatically.
   */
  void markVisited(const Block*);

  /*
   * Clears state upon hitting an loop header. Takes an optional hook whose
   * locals will get nulled out.
   */
  void loopHeaderClear(BCMarker, LocalStateHook* hook = nullptr);

 private:
  struct BlockState {
    jit::vector<Snapshot> in;
    jit::vector<Snapshot> out;
  };

private:
  bool checkInvariants() const;
  /*
   * Clear the current state, but keeps the state associated with all
   * other blocks intact.
   */
  void clearCurrentState();
  /*
   * Clears the current state and resets the current marker to the
   * given value.
   */
  void resetCurrentState(BCMarker);

  void trackDefInlineFP(const IRInstruction* inst);
  void trackInlineReturn();

  /* LocalStateHook overrides */
  void setLocalValue(uint32_t id, SSATmp* value) override;
  void refineLocalValue(uint32_t id, unsigned inlineIdx,
                        SSATmp* oldVal, SSATmp* newVal) override;
  void killLocalForCall(uint32_t id, unsigned inlineIdx, SSATmp* val) override;
  void dropLocalInnerType(uint32_t id, unsigned inlineIdx) override;
  void refineLocalType(uint32_t id, Type type, TypeSource typeSrc) override;
  void setLocalType(uint32_t id, Type type) override;
  void setBoxedLocalPrediction(uint32_t id, Type type) override;
  void setLocalTypeSource(uint32_t id, TypeSource typeSrc) override;

  jit::vector<LocalState>& locals(unsigned inlineIdx);

  /* Support for getLocalEffects */
  void clearLocals(LocalStateHook& hook) const;
  void refineLocalValues(LocalStateHook& hook,
                         SSATmp* oldVal, SSATmp* newVal) const;
  template<typename L>
  void walkAllInlinedLocals(L body, bool skipThisFrame = false) const;
  void killLocalsForCall(LocalStateHook& hook, bool skipThisFrame) const;
  void updateLocalValues(LocalStateHook& hook,
                         SSATmp* oldVal, SSATmp* newVal) const;
  void updateLocalRefPredictions(LocalStateHook&, SSATmp*, SSATmp*) const;
  void dropLocalRefsInnerTypes(LocalStateHook& hook) const;

  void cseInsert(const IRInstruction* inst);
  void cseKill(SSATmp* src);
  CSEHash* cseHashTable(const IRInstruction* inst);

  bool save(Block*);

  /*
   * Whether a block has been visited in the current iteration.
   */
  bool isVisited(const Block*) const;

  Snapshot& cur() {
    assert(!m_stack.empty());
    return m_stack.back();
  }
  const Snapshot& cur() const {
    return const_cast<FrameStateMgr*>(this)->cur();
  }

 private:
  IRUnit& m_unit;

  /*
   * Status of the FrameStateMgr.
   */
  Status m_status{Status::None};

  /*
   * Stack of states.  We push and pop frames as we enter and leave inlined
   * calls.
   */
  jit::vector<Snapshot> m_stack;

  /*
   * m_cseHash holds the destination of all tracked instructions that produced
   * values eligible for CSE.
   */
  CSEHash m_cseHash;
  bool m_enableCse{false};

  /*
   * Saved snapshots of the incoming and outgoing state of blocks.
   */
  jit::hash_map<Block*,BlockState> m_states;

  /*
   * Set of visited blocks during the traversal of the unit.
   */
  boost::dynamic_bitset<> m_visited;
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug stringification.
 */
std::string show(const FrameStateMgr&);

//////////////////////////////////////////////////////////////////////

} }

#endif
