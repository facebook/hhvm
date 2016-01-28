/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-source.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP {

struct Func;

namespace jit {

struct BlocksWithIds;
struct IRInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

Type refinePredictedType(Type oldPrediction, Type newPrediction, Type proven);
Type updatePredictedType(Type predictedType, Type provenType);

struct FPIInfo {
  SSATmp* returnSP;
  FPInvOffset returnSPOff; // return's logical sp offset; stkptr might differ
  SSATmp* ctx;
  Op fpushOpc; // bytecode for FPush*
  const Func* func;
  bool interp;
  bool spansCall;
};

//////////////////////////////////////////////////////////////////////

/*
 * SlotState stores information about either a local variable or a stack slot
 * in the current function, for FrameState.  LocalState and StackState are the
 * concrete versions of this struct, which differ only by the default type they
 * use.
 */
template<bool Stack>
struct SlotState {
  static constexpr Type default_type() {
    return Stack ? TStkElem : TGen;
  }

  /*
   * The current value of the or stack slot.
   */
  SSATmp* value{nullptr};

  /*
   * The current type of the local or stack slot.  We may have a
   * tracked type even if we don't have an available value.  This
   * happens across PHP-level calls, for example, or at some joint
   * points where we couldn't find the same available value for all
   * incoming edges.  However, whenever we have a value, the type of
   * the SSATmp must match this `type' field.
   */
  Type type{default_type()};

  /*
   * Prediction for the type of a local or stack slot, if it's boxed or if
   * we're in a pseudomain.  Otherwise it will be the same as `type'.
   *
   * Invariants:
   *   always a subtype of `type'
   */
  Type predictedType{default_type()};

  /*
   * The sources of the currently known type. They may be values. If the value
   * is unavailable, we won't hold onto it in the value field, but we'll keep
   * it around in typeSrcs for guard relaxation.
   */
  TypeSourceSet typeSrcs{};

  /*
   * Whether or not the local or stack element may have changed since
   * the entry of the unit.  This is only used for post-conditions.
   */
  bool maybeChanged{false};
};

using LocalState = SlotState<false>;
using StackState = SlotState<true>;

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
  SSATmp* fpValue{nullptr};

  /*
   * Tracking of in-memory state of the evaluation stack.
   */
  SSATmp* spValue{nullptr};
  FPInvOffset spOffset;   // delta from vmfp to spvalue

  /*
   * Here we keep track of the raw pointer value of the member base register,
   * the type of the pointer, as well as the value it points to, which we often
   * know after simple base operations like BaseH or BaseL. These are used for
   * some gen-time load elimination to preserve important information about the
   * base.
   */
  struct {
    SSATmp* ptr{nullptr};
    Type ptrType{TPtrToGen};
    SSATmp* value{nullptr};

    void reset() {
      ptr = nullptr;
      ptrType = TPtrToGen;
      value = nullptr;
    }
  } mbase;

  /*
   * Tracks whether we will need to ratchet tvRef and tvRef2 after emitting an
   * intermediate member instruction.
   */
  bool needRatchet{false};

  /*
   * m_thisAvailable tracks whether the current frame is known to have a
   * non-null $this pointer.
   */
  bool thisAvailable{false};

  /*
   * frameMaySpan is true iff a Call instruction has been seen on any path
   * since the definition of the current frame pointer.
   */
  bool frameMaySpanCall{false};

  /*
   * syncedSpLevel indicates the depth of the in-memory eval stack.
   *
   * TODO(#5868851): this field just dangles meaninglessly when FrameState is
   * being used in LegacyReoptimize mode.
   */
  FPInvOffset syncedSpLevel{0};

  /*
   * stackModified is reset to false by exceptionStackBoundary() and set to
   * true by anything that modifies the eval stack. It's used to verify that
   * the stack is not modified between the beginning of a bytecode's
   * translation and creation of any catch traces, unless
   * exceptionStackBoundary() is explicitly called.
   */
  bool stackModified{false};

  /*
   * The FPI stack is used for inlining---when we start inlining at an FCall,
   * we look in here to find a definition of the StkPtr,offset that can be used
   * after the inlined callee "returns".
   */
  jit::deque<FPIInfo> fpiStack;

  /*
   * The values in the eval stack in memory, either above or below the current
   * spValue pointer.  These are indexed relative to the base of the eval stack
   * for the whole function.
   */
  jit::vector<StackState> stack;

  /*
   * Vector of local variable inforation; sized for numLocals on the curFunc
   * (if the state is initialized).
   */
  jit::vector<LocalState> locals;

  /*
   * Predicted types for values that lived in a local or stack slot at one
   * point. Used to preserve predictions for values that move between different
   * slots.
   */
  jit::hash_map<SSATmp*, Type> predictedTypes;
};

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
struct FrameStateMgr final {
  explicit FrameStateMgr(BCMarker);

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
   * Starts tracking state for a block and reloads any previously
   * saved state.  The `hasUnprocessedPred' argument is used during
   * initial IR generation to indicate that the given block has a
   * predecessor in the region that might not yet be linked into the
   * IR CFG.
   */
  void startBlock(Block* b, bool hasUnprocessedPred = false);

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
  void computeFixedPoint(const BlockList&, const BlockIDs&);

  /*
   * Loads the in-state for a block. Requires that the block has already been
   * processed. Intended to be used after computing the fixed-point of a CFG.
   */
  void loadBlock(Block*);

  /*
   * Returns the post-conditions associated with the given exit block.
   */
  const PostConditions& postConds(Block*) const;

  const Func* func() const { return cur().curFunc; }
  FPInvOffset spOffset() const { return cur().spOffset; }
  SSATmp* sp() const { return cur().spValue; }
  SSATmp* fp() const { return cur().fpValue; }
  bool needRatchet() const { return cur().needRatchet; }
  void setNeedRatchet(bool b) { cur().needRatchet = b; }
  bool thisAvailable() const { return cur().thisAvailable; }
  void setThisAvailable() { cur().thisAvailable = true; }
  bool frameMaySpanCall() const { return cur().frameMaySpanCall; }
  unsigned inlineDepth() const { return m_stack.size() - 1; }
  FPInvOffset syncedSpLevel() const { return cur().syncedSpLevel; }
  void setSyncedSpLevel(FPInvOffset o) { cur().syncedSpLevel = o; }
  void incSyncedSpLevel(int32_t n = 1) { cur().syncedSpLevel += n; }
  void decSyncedSpLevel(int32_t n = 1) { cur().syncedSpLevel -= n; }
  SSATmp* memberBasePtr() const;
  Type memberBasePtrType() const;
  void setMemberBaseValue(SSATmp*);
  SSATmp* memberBaseValue() const;

  void refinePredictedTmpType(SSATmp*, Type);
  Type predictedTmpType(SSATmp*) const;

  Type localType(uint32_t id) const;
  bool localMaybeChanged(uint32_t id) const;
  Type predictedLocalType(uint32_t id) const;
  SSATmp* localValue(uint32_t id) const;
  TypeSourceSet localTypeSources(uint32_t id) const;
  void refineLocalPredictedType(uint32_t id, Type type);

  Type stackType(IRSPOffset) const;
  bool stackMaybeChanged(IRSPOffset) const;
  Type predictedStackType(IRSPOffset) const;
  SSATmp* stackValue(IRSPOffset) const;
  TypeSourceSet stackTypeSources(IRSPOffset) const;
  void refineStackPredictedType(IRSPOffset, Type);
  bool stackModified() const { return cur().stackModified; }
  void resetStackModified() { cur().stackModified = false; }

  const jit::deque<FPIInfo>& fpiStack() const { return cur().fpiStack; }

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
  void clearForUnprocessedPred();
  StackState& stackState(IRSPOffset offset);
  const StackState& stackState(IRSPOffset offset) const;
  void collectPostConds(Block* exitBlock);
  void updateMInstr(const IRInstruction*);

private:
  FrameState& cur() {
    assertx(!m_stack.empty());
    return m_stack.back();
  }
  const FrameState& cur() const {
    return const_cast<FrameStateMgr*>(this)->cur();
  }

  template<bool Stack>
  void syncPrediction(SlotState<Stack>&);

  /*
   * refine(Local|Stack)Type() are used when the value of a slot hasn't changed
   * but we have more information about its type, from a guard or type assert.
   *
   * set(Local|Stack)Type() are used to change the type of a slot when we have
   * a brand new type because the value might have changed. These operations
   * clear the typeSrcs of the slot, so new type may not be derived from the
   * old type in any way.
   *
   * widen(Local|Stack)Type() are used to change the type of a slot, as a
   * result of an operation that might change the value. These operations
   * preserve the typeSrcs of the slot, so the new type may be derived from the
   * old type.
   */
private: // local tracking helpers
  void setLocalValue(uint32_t id, SSATmp* value);
  void refineLocalValues(SSATmp* oldVal, SSATmp* newVal);
  void dropLocalRefsInnerTypes();
  void killLocalsForCall(bool);
  void refineLocalType(uint32_t id, Type type, TypeSource typeSrc);
  void setLocalPredictedType(uint32_t id, Type type);
  void setLocalType(uint32_t id, Type type);
  void widenLocalType(uint32_t id, Type type);
  void setBoxedLocalPrediction(uint32_t id, Type type);
  void updateLocalRefPredictions(SSATmp*, SSATmp*);
  void setLocalTypeSource(uint32_t id, TypeSource typeSrc);
  void clearLocals();

private: // stack tracking helpers
  void setStackValue(IRSPOffset, SSATmp*);
  void setStackType(IRSPOffset, Type);
  void widenStackType(IRSPOffset, Type);
  void refineStackValues(SSATmp* oldval, SSATmp* newVal);
  void refineStackType(IRSPOffset, Type, TypeSource typeSrc);
  void clearStackForCall();
  void setBoxedStkPrediction(IRSPOffset, Type type);
  void spillFrameStack(IRSPOffset, FPInvOffset, const IRInstruction*);
  void clearStack();

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

  /*
   * Post-conditions for exit blocks.
   */
  jit::hash_map<Block*,PostConditions> m_exitPostConds;
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug stringification.
 */
std::string show(const FrameStateMgr&);

//////////////////////////////////////////////////////////////////////

} }

#endif
