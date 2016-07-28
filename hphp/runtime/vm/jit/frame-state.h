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

#ifndef incl_HPHP_JIT_FRAME_STATE_H_
#define incl_HPHP_JIT_FRAME_STATE_H_

#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-source.h"

#include <boost/dynamic_bitset.hpp>
#include <folly/Optional.h>

#include <memory>
#include <vector>

namespace HPHP {

struct Func;

namespace jit {

struct BlocksWithIds;
struct IRInstruction;
struct SSATmp;

namespace irgen {

///////////////////////////////////////////////////////////////////////////////

struct FPIInfo {
  SSATmp* returnSP;
  FPInvOffset returnSPOff; // return's logical sp offset; stkptr might differ
  Type ctxType; // tracked separately as a union of observed ctx types
  SSATmp* ctx;
  Op fpushOpc; // bytecode for FPush*
  const Func* func;
  bool interp;
  bool spansCall;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Information about a Location in the current function; used by FrameState.
 */
template<LTag tag>
struct LocationState {
  static_assert(tag == LTag::Stack ||
                tag == LTag::Local ||
                false,
                "invalid LTag for LocationState");

  static constexpr Type default_type() {
    return tag == LTag::Stack ? TStkElem :
        /* tag == LTag::Local */ TGen;
  }

  /*
   * The current value at the location.
   */
  SSATmp* value{nullptr};

  /*
   * The current type of the value at the location.
   *
   * We may have a tracked type even if we don't have an available value.  This
   * happens across PHP-level calls, for example, or at some joint points where
   * we couldn't find the same available value for all incoming edges.
   * However, whenever we have a value, the type of the SSATmp must match this
   * `type' field.
   */
  Type type{LocationState::default_type()};

  /*
   * Prediction for the type at the location, if it's boxed or if we're in a
   * pseudomain.  Otherwise it will be the same as `type'.
   *
   * @requires: predictedType <= type
   */
  Type predictedType{LocationState::default_type()};

  /*
   * The sources of the currently known type, which may be values.
   *
   * If the value is unavailable, we won't hold onto it in the value field, but
   * we'll keep it around in typeSrcs for guard relaxation.
   */
  TypeSourceSet typeSrcs{};

  /*
   * Whether or not the location may have changed since the entry of the unit.
   *
   * This is only used for post-conditions.
   */
  bool maybeChanged{false};
};

using LocalState = LocationState<LTag::Local>;
using StackState = LocationState<LTag::Stack>;

struct MBaseState {
  SSATmp* value{nullptr};
};

/*
 * MBRState tracks the value and type of the member base register pointer.
 *
 * These are used for some gen-time load elimination to preserve important
 * information about the base.
 */
struct MBRState {
  SSATmp* ptr{nullptr};
  AliasClass pointee{AUnknownTV};
  Type ptrType{TPtrToGen};
};

///////////////////////////////////////////////////////////////////////////////

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
  FPInvOffset irSPOff;  // delta from vmfp to `spValue'

  /*
   * Depth of the in-memory eval stack.
   */
  FPInvOffset bcSPOff{0};

  /*
   * Tracks whether we will need to ratchet tvRef and tvRef2 after emitting an
   * intermediate member instruction.
   */
  bool needRatchet{false};

  /*
   * thisAvailable tracks whether the current frame is known to have a
   * non-null $this pointer.
   */
  bool thisAvailable{false};

  /*
   * ctx tracks the current ActRec's this/ctx field
   */
  SSATmp* ctx{nullptr};

  /*
   * frameMaySpan is true iff a Call instruction has been seen on any path
   * since the definition of the current frame pointer.
   */
  bool frameMaySpanCall{false};

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
  jit::vector<FPIInfo> fpiStack;

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
   * Values and types of the member base register and its pointee.
   */
  MBRState mbr;
  MBaseState mbase;

  /*
   * Predicted types for values that lived in a local or stack slot at one
   * point. Used to preserve predictions for values that move between different
   * slots.
   */
  jit::hash_map<SSATmp*, Type> predictedTypes;
};

///////////////////////////////////////////////////////////////////////////////

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
   * Update state by computing the effects of an instruction.
   */
  void update(const IRInstruction*);

  /////////////////////////////////////////////////////////////////////////////
  // Per-block state.

  /*
   * Whether we have state saved for the given block.
   */
  bool hasStateFor(Block*) const;

  /*
   * Start tracking state for a block and reloads any previously saved state.
   *
   * `hasUnprocessedPred' is set to indicate that the given block has a
   * predecessor in the region that might not yet be linked into the IR CFG.
   *
   * `pred' is the logical predecessor of `b' to be used in the event that `b'
   * is unreachable.
   */
  void startBlock(Block* b, bool hasUnprocessedPred,
                  Block* pred = nullptr);

  /*
   * Finish tracking state for `b' and save the current state to b->next()
   * (b->taken() is handled in update()).  Also save the out-state if
   * setSaveOutState() was called on `b'.
   *
   * Returns true iff the in-state for the next block has changed.
   */
  bool finishBlock(Block* b);

  /*
   * Mark that `b' should save its out-state when finishBlock() is called.
   */
  void setSaveOutState(Block* b);

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
   * Resume processing a block that was stopped by pauseBlock.
   */
  void unpauseBlock(Block*);

  /*
   * Return the post-conditions associated with `exitBlock'.
   */
  const PostConditions& postConds(Block* exitBlock) const;

  /*
   * Set an override for the next FPI region's fpushOp.
   */
  void setFPushOverride(Op op)  { m_fpushOverride = op; }
  bool hasFPushOverride() const { return m_fpushOverride.hasValue(); }

  /////////////////////////////////////////////////////////////////////////////

  /*
   * FrameState accessors.
   *
   * In the presence of inlining, these return state for the most-inlined
   * frame.
   */
  const Func* func()              const { return cur().curFunc; }
  SSATmp*     fp()                const { return cur().fpValue; }
  SSATmp*     sp()                const { return cur().spValue; }
  SSATmp*     ctx()               const { return cur().ctx; }
  FPInvOffset irSPOff()           const { return cur().irSPOff; }
  FPInvOffset bcSPOff()           const { return cur().bcSPOff; }
  SSATmp*     memberBaseValue()   const { return cur().mbase.value; }
  bool        needRatchet()       const { return cur().needRatchet; }
  bool        thisAvailable()     const { return cur().thisAvailable; }
  bool        frameMaySpanCall()  const { return cur().frameMaySpanCall; }
  bool        stackModified()     const { return cur().stackModified; }
  const jit::vector<FPIInfo>& fpiStack() const { return cur().fpiStack; }

  /*
   * FrameState modifiers.
   *
   * In the presence of inlining, these modify state for the most-inlined
   * frame.
   */
  void setMemberBaseValue(SSATmp* base) { cur().mbase.value = base; }
  void setNeedRatchet(bool b)           { cur().needRatchet = b; }
  void setThisAvailable()               { cur().thisAvailable = true; }
  void resetStackModified()             { cur().stackModified = false; }
  void setBCSPOff(FPInvOffset o)        { cur().bcSPOff = o; }
  void incBCSPDepth(int32_t n = 1)      { cur().bcSPOff += n; }
  void decBCSPDepth(int32_t n = 1)      { cur().bcSPOff -= n; }

  /*
   * Current inlining depth (not including the toplevel frame).
   */
  unsigned inlineDepth() const { return m_stack.size() - 1; }

  /*
   * Return the LocationState for local `id' or stack element at `off' in the
   * most-inlined frame.
   */
  const LocalState& local(uint32_t id) const;
  const StackState& stack(IRSPRelOffset off) const;
  const StackState& stack(FPInvOffset off) const;

  /*
   * Generic accessors for LocationState members.
   */
  SSATmp* valueOf(Location l) const;
  Type typeOf(Location l) const;
  Type predictedTypeOf(Location l) const;
  const TypeSourceSet& typeSrcsOf(Location l) const;

  /*
   * Return tracked state for the member base register.
   */
  const MBRState& mbr()     const { return cur().mbr; }

  /*
   * Update the predicted type for `l'.
   */
  void refinePredictedType(Location l, Type type);

  /////////////////////////////////////////////////////////////////////////////

private:
  FrameState& cur() {
    assertx(!m_stack.empty());
    return m_stack.back();
  }
  const FrameState& cur() const {
    return const_cast<FrameStateMgr*>(this)->cur();
  }

  /*
   * LocationState access helpers.
   */
  Location loc(uint32_t) const;
  Location stk(IRSPRelOffset) const;
  LocalState& localState(uint32_t);
  LocalState& localState(Location l); // @requires: l.tag() == LTag::Local
  StackState& stackState(IRSPRelOffset);
  StackState& stackState(FPInvOffset);
  StackState& stackState(Location l); // @requires: l.tag() == LTag::Stack

  /*
   * Helpers for update().
   */
  bool checkInvariants() const;
  void updateMInstr(const IRInstruction*);
  void trackDefInlineFP(const IRInstruction* inst);
  void trackInlineReturn();
  void trackCall(bool destroyLocals);

  /*
   * Per-block state helpers.
   */
  bool save(Block* b, Block* pred = nullptr);
  void clearForUnprocessedPred();
  void collectPostConds(Block* exitBlock);

  /*
   * LocationState update helpers.
   */
  void setValue(Location l, SSATmp* value);
  void setType(Location l, Type type);
  void widenType(Location l, Type type);
  void refineType(Location l, Type type, TypeSource typeSrc);
  void setBoxedPrediction(Location l, Type type);
  void refinePredictedTmpType(SSATmp*, Type);

  template<LTag tag>
  void refineValue(LocationState<tag>& state, SSATmp* oldVal, SSATmp* newVal);

  template<LTag tag>
  void setValueImpl(Location l, LocationState<tag>& state, SSATmp* value,
                    folly::Optional<Type> predicted = folly::none);
  template<LTag tag>
  void refinePredictedTypeImpl(LocationState<tag>& state, Type type);

  template<LTag tag> void syncPrediction(LocationState<tag>&);

  /*
   * Local state update helpers.
   */
  void setLocalPredictedType(uint32_t id, Type type);
  void updateLocalRefPredictions(SSATmp*, SSATmp*);
  void killLocalsForCall(bool);
  void dropLocalRefsInnerTypes();
  void clearLocals();

  /*
   * Stack state update helpers.
   */
  void spillFrameStack(IRSPRelOffset, FPInvOffset, const IRInstruction*);
  void clearStackForCall();

private:
  struct BlockState {
    // Mandatory in-state computed from predecessors.
    jit::vector<FrameState> in;
    // Optionally-saved out-state.  Non-none but empty indicates that out-state
    // should be saved.
    folly::Optional<jit::vector<FrameState>> out;
    // Paused state, used by IRBuilder::{push,pop}Block().
    folly::Optional<jit::vector<FrameState>> paused;
  };

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
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

  /*
   * Override for the current fpush* bytecode so we can convert bytecodes
   * to php calls.
   */
  folly::Optional<Op> m_fpushOverride;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Debug stringification.
 */
std::string show(const FrameStateMgr&);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
