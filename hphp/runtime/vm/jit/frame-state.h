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

#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-source.h"
#include "hphp/runtime/vm/jit/local-effects.h"

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
   * The current type of the local.  We may have a tracked type even if we
   * don't have an available value.  This happens across PHP-level calls, for
   * example, or at some joint points where we couldn't find the same available
   * value for all incoming edges.
   */
  Type type{Type::Gen};

  /*
   * Prediction for the type of a local, if it's boxed or if we're in a
   * pseudomain.  Otherwise it will be the same as `type'.
   *
   * Invariants:
   *   always a subtype of `type'
   */
  Type predictedType{Type::Gen};

  /*
   * The sources of the currently known type. They may be values. If the value
   * is unavailable, we won't hold onto it in the value field, but we'll keep
   * it around in typeSrcs for guard relaxation.
   */
  TypeSourceSet typeSrcs;
};

inline bool operator==(const LocalState& a, const LocalState& b) {
  return a.value         == b.value &&
         a.type          == b.type &&
         a.predictedType == b.predictedType &&
         a.typeSrcs      == b.typeSrcs;
}

//////////////////////////////////////////////////////////////////////

/*
 * State related to a particular frame.  These state structures are stored in a
 * stack, that we push and pop as we enter and leave inlined frames.
 */
struct FrameState {
  /*
   * Current Func, VM stack pointer, VM frame pointer, offset between sp and
   * fp, and bytecode position.
   */
  const Func* curFunc;
  SSATmp* spValue{nullptr};
  SSATmp* fpValue{nullptr};
  int32_t spOffset;

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
   * frameMaySpan is true iff a Call instruction has been seen on any path
   * since the definition of the current frame pointer.
   */
  bool frameMaySpanCall{false};
};

inline bool operator==(const FrameState& a, const FrameState& b) {
  return
    a.spValue          == b.spValue &&
    a.fpValue          == b.fpValue &&
    a.curFunc          == b.curFunc &&
    a.spOffset         == b.spOffset &&
    a.thisAvailable    == b.thisAvailable &&
    a.locals           == b.locals &&
    a.frameMaySpanCall == b.frameMaySpanCall;
}

//////////////////////////////////////////////////////////////////////

/*
 * FrameStateMgr tracks state about the VM stack frame in the function currently
 * being translated. It is responsible for both storing the state and updating
 * it appropriately as instructions and blocks are processed.
 *
 * The types of state tracked by FrameStateMgr include:
 *
 *   - value availability for values stored in locals, or the $this pointer
 *
 *      Used for value propagation.
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
  explicit FrameStateMgr(const IRUnit&, BCMarker);

  FrameStateMgr(const FrameStateMgr&) = delete;
  FrameStateMgr(FrameStateMgr&&) = default;
  FrameStateMgr& operator=(const FrameStateMgr&) = delete;
  FrameStateMgr& operator=(FrameStateMgr&&) = default;

  /*
   * Put the FrameStateMgr in building mode.  This function must be called
   * after constructing a FrameStateMgr before you start updating it, unless
   * you're using it for fixed point mode.
   */
  void setBuilding() { m_status = Status::Building; }

  /*
   * Tell the FrameStateMgr we're doing reoptimize without being aware of all
   * types of control flow.
   */
  void setLegacyReoptimize() { m_status = Status::LegacyReoptimize; }

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
   * Starts tracking state for a block and reloads any previously saved
   * state. Can set local values to null if hitting a block with an
   * unprocessed predecessor, so we pass in an optional LocalStateHook. The
   * isLoopHeader parameter is used during initial IR generation to indicate
   * that the given block has a predecessor in the region that might not yet
   * be linked into the IR cfg.
   */
  void startBlock(Block* b, BCMarker marker, bool isLoopHeader = false);

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
   * Iterates through a control-flow graph, until a fixed-point is
   * reached. Must be called before this FrameStateMgr has any state.
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
  bool frameMaySpanCall() const { return cur().frameMaySpanCall; }
  unsigned inlineDepth() const { return m_stack.size() - 1; }
  uint32_t stackDeficit() const { return cur().stackDeficit; }
  void incStackDeficit() { cur().stackDeficit++; }
  void clearStackDeficit() { cur().stackDeficit = 0; }
  EvalStack& evalStack() { return cur().evalStack; }

  Type localType(uint32_t id) const;
  Type predictedLocalType(uint32_t id) const;
  SSATmp* localValue(uint32_t id) const;
  TypeSourceSet localTypeSources(uint32_t id) const;

  /*
   * Call a function with const access to the LocalState& for each local we're
   * tracking.
   */
  void walkAllInlinedLocals(
    const std::function<void (uint32_t, unsigned, const LocalState&)>& body,
    bool skipThisFrame) const;

  /*
   * Call `func' with all non-null tracked local values, including callers if
   * this is an inlined frame.
   */
  void forEachLocalValue(const std::function<void (SSATmp*)>& func) const;

private:
  struct BlockState {
    jit::vector<FrameState> in;
    folly::Optional<jit::vector<FrameState>> paused;
  };

  enum class Status : uint8_t {
    /*
     * Status we have after initially being created.
     */
    None,

    /*
     * Changes when we propagate state to taken blocks.  This status is used
     * during IR generation time.
     */
    Building,

    /*
     * Changes how we handle predecessors we haven't visited yet.  This state
     * means we're doing computeFixedPoint() still.
     */
    RunningFixedPoint,

    /*
     * Stops us from merging new state to blocks.  The computeFixedPoint call
     * has finished, and blocks may be inspected with that information, but we
     * don't need to propagate anything anywhere anymore.
     */
    FinishedFixedPoint,

    /*
     * We're doing a reoptimize that's not based on a fixed-point computation.
     */
    LegacyReoptimize,
  };

private:
  bool checkInvariants() const;
  bool save(Block*);
  jit::vector<LocalState>& locals(unsigned inlineIdx);
  void trackDefInlineFP(const IRInstruction* inst);
  void trackInlineReturn();
  void loopHeaderClear(BCMarker);

private:
  FrameState& cur() {
    assert(!m_stack.empty());
    return m_stack.back();
  }
  const FrameState& cur() const {
    return const_cast<FrameStateMgr*>(this)->cur();
  }

private: // LocalStateHook overrides
  void setLocalValue(uint32_t id, SSATmp* value) override;
  void refineLocalValues(SSATmp* oldVal, SSATmp* newVal) override;
  void dropLocalRefsInnerTypes() override;
  void killLocalsForCall(bool) override;
  void refineLocalType(uint32_t id, Type type, TypeSource typeSrc) override;
  void predictLocalType(uint32_t id, Type type) override;
  void setLocalType(uint32_t id, Type type) override;
  void setBoxedLocalPrediction(uint32_t id, Type type) override;
  void updateLocalRefPredictions(SSATmp*, SSATmp*) override;
  void setLocalTypeSource(uint32_t id, TypeSource typeSrc) override;
  void clearLocals() override;

private:
  Status m_status{Status::None};

  /*
   * Stack of states.  We push and pop frames as we enter and leave inlined
   * calls.
   */
  jit::vector<FrameState> m_stack;

  /*
   * Saved snapshots of the incoming and outgoing state of blocks.
   */
  jit::hash_map<Block*,BlockState> m_states;
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug stringification.
 */
std::string show(const FrameStateMgr&);

//////////////////////////////////////////////////////////////////////

} }

#endif
