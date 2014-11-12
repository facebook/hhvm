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

  void swap(std::vector<SSATmp*>& vector) {
    m_vector.swap(vector);
  }
private:
  std::vector<SSATmp*> m_vector;
};

//////////////////////////////////////////////////////////////////////

/*
 * LocalStateHook is used to separate the acts of determining which locals are
 * affected by an instruction and recording those changes. It allows consumers
 * of FrameState to get details about how an instruction affects the locals
 * without having to query the state of each local before and after having
 * FrameState process the instruction.
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
 * FrameState tracks state about the VM stack frame in the function currently
 * being translated. It is responsible for both storing the state and updating
 * it appropriately as instructions and blocks are processed.
 *
 * The types of state tracked by FrameState include:
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
struct FrameState final : private LocalStateHook {
  FrameState(IRUnit& unit, BCMarker firstMarker);
  FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func);

  FrameState(const FrameState&) = delete;
  FrameState& operator=(const FrameState&) = delete;

  FrameState(FrameState&&) = default;

  /*
   * Update state by computing the effects of an instruction.
   */
  void update(const IRInstruction*);

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
   */
  void finishBlock(Block*);

  /*
   * Save current state of a block so we can resume processing it
   * after working on another.
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
   * Clears the current state and resets the current marker to the
   * given value.
   */
  void resetCurrentState(BCMarker);

  const Func* func() const { return m_curFunc; }
  Offset spOffset() const { return m_spOffset; }
  SSATmp* sp() const { return m_spValue; }
  SSATmp* fp() const { return m_fpValue; }
  bool thisAvailable() const { return m_thisAvailable; }
  void setThisAvailable() { m_thisAvailable = true; }
  bool frameSpansCall() const { return m_frameSpansCall; }
  BCMarker marker() const { return m_marker; }
  void setMarker(BCMarker m) { m_marker = m; }
  bool enableCse() const { return m_enableCse; }
  void setEnableCse(bool e) { m_enableCse = e; }
  unsigned inlineDepth() const { return m_inlineSavedStates.size(); }
  void setBuilding(bool b) { m_building = b; }
  uint32_t stackDeficit() const { return m_stackDeficit; }
  void incStackDeficit() { m_stackDeficit++; }
  void clearStackDeficit() { m_stackDeficit = 0; }
  EvalStack& evalStack() { return m_evalStack; }

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
   * LocalState stores information about a local in the current function.
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
     * The sources of the currently known type. They may be values. If
     * the value is unavailable, we won't hold onto it in the value field, but
     * we'll keep it around in typeSrcs for guard relaxation.
     */
    TypeSourceSet typeSrcs;

    bool operator==(const LocalState& b) const {
      if (value != b.value ||
          type != b.type ||
          boxedPrediction != b.boxedPrediction ||
          typeSrcs.size() != b.typeSrcs.size()) {
        return false;
      }
      for (auto it = typeSrcs.begin(), itb = b.typeSrcs.begin();
           it != typeSrcs.end(); it++, itb++) {
        if (*it != *itb) return false;
      }
      return true;
    }
  };

  using LocalVec = jit::vector<LocalState>;

  /*
   * Info about state leaving a block. The block must have already been
   * processed.
   */
  const LocalVec& localsLeavingBlock(Block*) const;
  SSATmp* spLeavingBlock(Block*) const;

  /*
   * Marks a block as visited in the current iteration. FrameState::startBlock
   * does this automatically.
   */
  void markVisited(const Block*);

  /*
   * Clears state upon hitting an loop header. Takes an optional hook whose
   * locals will get nulled out.
   */
  void loopHeaderClear(BCMarker, LocalStateHook* hook = nullptr);

 private:
  /*
   * Snapshot stores fields of FrameState to be saved, restored, and merged for
   * inlining and control flow.
   */
  struct Snapshot {
    SSATmp* spValue;
    SSATmp* fpValue;
    const Func* curFunc;
    int32_t spOffset;
    bool thisAvailable;
    uint32_t stackDeficit;
    EvalStack evalStack;
    LocalVec locals;
    bool frameSpansCall;
    BCMarker curMarker;
    jit::vector<Snapshot> inlineSavedStates;

    bool operator==(const Snapshot& b) const {
      return spValue == b.spValue &&
        fpValue == b.fpValue &&
        curFunc == b.curFunc &&
        spOffset == b.spOffset &&
        thisAvailable == b.thisAvailable &&
        locals == b.locals &&
        frameSpansCall == b.frameSpansCall &&
        curMarker == b.curMarker &&
        inlineSavedStates == b.inlineSavedStates;
    }
  };

  struct BlockState {
    Snapshot in;
    Snapshot out;
  };

private:
  bool checkInvariants() const;
  /*
   * Clear the current state, but keeps the state associated with all
   * other blocks intact.
   */
  void clearCurrentState();

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

  LocalVec& locals(unsigned inlineIdx);

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

  Snapshot createSnapshot() const;
  Snapshot createSnapshotWithInline() const;

  void save(Block*);
  void load(Snapshot& state);
  void merge(Snapshot& s1);

  /*
   * Whether a block has been visited in the current iteration.
   */
  bool isVisited(const Block*) const;

 private:
  IRUnit& m_unit;

  /*
   * Current Func, VM stack pointer, VM frame pointer, offset between sp and
   * fp, and bytecode position.
   */
  const Func* m_curFunc;
  SSATmp* m_spValue{nullptr};
  SSATmp* m_fpValue{nullptr};
  int32_t m_spOffset;
  BCMarker m_marker;

  /*
   * m_thisAvailable tracks whether the current frame is known to have a
   * non-null $this pointer.
   */
  bool m_thisAvailable{false};

  /*
   * m_frameSpansCall is true iff a Call instruction has been seen since the
   * definition of the current frame pointer.
   */
  bool m_frameSpansCall{false};

  /*
   * m_building is true if we're using FrameState to build the IR,
   * since some state updates are conditional in that case.
   */
  bool m_building{false};

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
  uint32_t m_stackDeficit{0};
  EvalStack m_evalStack;

  /*
   * m_locals tracks the current types and values of locals.
   */
  LocalVec m_locals;

  /*
   * m_inlineSavedStates holds snapshots of the caller(s)'s state while in an
   * inlined callee.
   */
  jit::vector<Snapshot> m_inlineSavedStates;

  /*
   * m_cseHash holds the destination of all tracked instructions that produced
   * values eligible for CSE.
   */
  CSEHash m_cseHash;
  bool m_enableCse{false};

  /*
   * Saved snapshots of the incoming and outgoing state of blocks.
   */
  jit::hash_map<Block*, BlockState> m_states;

  /*
   * Set of visited blocks during the traversal of the unit.
   */
  boost::dynamic_bitset<> m_visited;
};

/*
 * Debug stringification.
 */
std::string show(const FrameState&);

} }

#endif
