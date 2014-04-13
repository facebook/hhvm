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

#include <vector>
#include <memory>

#include "folly/Optional.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP {

struct Func;

namespace JIT {

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

  void swap(std::vector<SSATmp*> &vector) {
    m_vector.swap(vector);
  }
private:
  std::vector<SSATmp*> m_vector;
};

//////////////////////////////////////////////////////////////////////

/*
 * LocalStateHook is used to separate the acts of determining which locals are
 * affected by an instruction and recording those changes. It allows consumers
 * of FrameState to get details about how an instruciton affects the locals
 * without having to query the state of each local before and after having
 * FrameState process the instruction.
 */
struct LocalStateHook {
  virtual void setLocalValue(uint32_t id, SSATmp* value) {}
  virtual void refineLocalValue(uint32_t id, unsigned inlineIdx,
                                SSATmp* oldVal, SSATmp* newVal) {}
  virtual void killLocalForCall(uint32_t id, unsigned inlineIdx, SSATmp* val) {}
  virtual void updateLocalRefValue(uint32_t id, unsigned inlineIdx,
                                   SSATmp* oldRef, SSATmp* newRef) {}
  virtual void dropLocalInnerType(uint32_t id, unsigned inlineIdx) {}

  virtual void refineLocalType(uint32_t id, Type type, SSATmp* typeSource) {}
  virtual void setLocalType(uint32_t id, Type type) {}
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
  FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func,
             uint32_t numLocals);

  FrameState(const FrameState&) = delete;
  FrameState& operator=(const FrameState&) = delete;

  FrameState(FrameState&&) = default;

  void update(const IRInstruction* inst);

  /*
   * Starts tracking state for a block and reloads any previously
   * saved state.
   */
  void startBlock(Block*);

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
   * Clear all tracked state.
   */
  void clear();

  /*
   * Clear the CSE table.
   */
  void clearCse();

  /*
   * Check current state for compatibility (matching types of
   * stack/locals) with the state at block.
   */
  bool compatible(Block*);

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
  SSATmp* localValue(uint32_t id) const;
  SSATmp* localTypeSource(uint32_t id) const;

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
    LocalState()
      : value(nullptr)
      , type(Type::Gen)
      , typeSource(nullptr)
    {}

    SSATmp* value; // The current value of the local. nullptr if unknown
    Type type;     // The current type of the local.
    SSATmp* typeSource; // The source of the currently known type: either the
                        // current value, a FramePtr with a guard, or nullptr
                        // if the value is new and unknown.

    bool operator==(const LocalState& b) const {
      return value == b.value &&
        type == b.type &&
        typeSource == b.typeSource;
    }
  };

  typedef smart::vector<LocalState> LocalVec;

  const LocalVec& localsForBlock(Block* b) const;

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
    smart::vector<Snapshot> inlineSavedStates;

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

  void trackDefInlineFP(const IRInstruction* inst);
  void trackInlineReturn();

  /* LocalStateHook overrides */
  void setLocalValue(uint32_t id, SSATmp* value) override;
  void refineLocalValue(uint32_t id, unsigned inlineIdx,
                        SSATmp* oldVal, SSATmp* newVal) override;
  void killLocalForCall(uint32_t id, unsigned inlineIdx, SSATmp* val) override;
  void updateLocalRefValue(uint32_t id, unsigned inlineIdx, SSATmp* oldRef,
                           SSATmp* newRef) override;
  void dropLocalInnerType(uint32_t id, unsigned inlineIdx) override;
  void refineLocalType(uint32_t id, Type type, SSATmp* typeSource) override;
  void setLocalType(uint32_t id, Type type) override;

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
  void updateLocalRefValues(LocalStateHook& hook,
                            SSATmp* oldRef, SSATmp* newRef) const;
  void dropLocalRefsInnerTypes(LocalStateHook& hook) const;

  void cseInsert(const IRInstruction* inst);
  void cseKill(SSATmp* src);
  CSEHash* cseHashTable(const IRInstruction* inst);

  Snapshot createSnapshot() const;
  void save(Block*);
  void load(Snapshot& state);
  void merge(Snapshot& s1);

 private:
  IRUnit& m_unit;

  /*
   * Current Func, VM stack pointer, VM frame pointer, offset between sp and
   * fp, and bytecode position.
   */
  const Func* m_curFunc;
  SSATmp* m_spValue;
  SSATmp* m_fpValue;
  int32_t m_spOffset;
  BCMarker m_marker;

  /*
   * m_thisAvailable tracks whether the current frame is known to have a
   * non-null $this pointer.
   */
  bool m_thisAvailable;

  /*
   * m_frameSpansCall is true iff a Call instruction has been seen since the
   * definition of the current frame pointer.
   */
  bool m_frameSpansCall;

  /*
   * m_building is true if we're using FrameState to build the IR,
   * since some state updates are conditional in that case.
   */
  bool m_building = false;

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
  uint32_t m_stackDeficit;
  EvalStack m_evalStack;

  /*
   * m_locals tracks the current types and values of locals.
   */
  LocalVec m_locals;

  /*
   * m_inlineSavedStates holds snapshots of the caller(s)'s state while in an
   * inlined callee.
   */
  smart::vector<Snapshot> m_inlineSavedStates;

  /*
   * m_cseHash holds the destination of all tracked instructions that produced
   * values eligible for CSE.
   */
  CSEHash m_cseHash;
  bool m_enableCse;

  /*
   * Saved snapshots of the incoming state for Blocks.
   */
  smart::hash_map<Block*, Snapshot> m_snapshots;
};

/*
 * Debug stringification.
 */
std::string show(const FrameState&);

} }

#endif
