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

#pragma once

#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-source.h"

#include "hphp/util/tribool.h"

#include <boost/dynamic_bitset.hpp>

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

/*
 * Information about a Location in the current function; used by FrameState.
 */
template<LTag tag>
struct LocationState {
  static_assert(tag == LTag::Stack ||
                tag == LTag::Local ||
                tag == LTag::MBase ||
                false,
                "invalid LTag for LocationState");

  static constexpr Type default_type() { return TCell; }

  template<LTag other>
  LocationState<tag>& operator=(const LocationState<other>& o) {
    value = o.value;
    type = o.type;
    typeSrcs = o.typeSrcs;
    maybeChanged = o.maybeChanged;
    return *this;
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
using MBaseState = LocationState<LTag::MBase>;

using LocalStateMap = jit::hash_map<uint32_t,LocalState>;
using StackStateMap = jit::hash_map<SBInvOffset,StackState,SBInvOffset::Hash>;

/*
 * MBRState tracks the value and type of the member base register pointer.
 *
 * These are used for some gen-time load elimination to preserve important
 * information about the base.
 */
struct MBRState {
  SSATmp* ptr{nullptr};
  AliasClass pointee{AUnknownTV};
  Type ptrType{TLval};
};

/*
 * Track the known value and type of the MInstrTempBase.
 */
struct MTempBaseState {
  SSATmp* value{nullptr};
  Type type{TCell};
};

///////////////////////////////////////////////////////////////////////////////

/*
 * State related to a particular frame.  These state structures are stored in a
 * stack, that we push and pop as we enter and leave inlined frames.
 */
struct FrameState {
  explicit FrameState(const Func* func);
  bool checkInvariants() const;
  bool checkMInstrStateDead() const;

  std::string show() const;

  /*
   * FrameState Accesors.
   */
  SSATmp*     fp()                const { return fpValue; }
  SSATmp*     fixupFP()           const { return fixupFPValue; }
  SSATmp*     sp()                const { return spValue; }

  /*
   * Function, instructions are emitted on behalf of. This may be different from
   * the function of the `fpValue` frame, e.g. stublogues run in the context of
   * the caller, but the code is emitted on behalf of the callee.
   */
  const Func* curFunc;

  /*
   * VM frame pointer.
   *
   * - `fpValue` points to the current logical frame that may not have been
   *   materialized (e.g. due to inlining)
   * - `fixupFPValue` points to the last materialized frame, which is the same
   *   as the value of the `rvmfp()` register
   */
  SSATmp* fpValue{nullptr};
  SSATmp* fixupFPValue{nullptr};

  /*
   * Logical stack base.
   *
   * - `spValue` points to an arbitrary stack position, which may even be
   *   outside of the logical stack
   * - `irSPOff` is an offset from the logical stack base to `spValue`
   */
  SSATmp* spValue{nullptr};
  SBInvOffset irSPOff{0};

  /*
   * Depth of the in-memory eval stack.
   */
  SBInvOffset bcSPOff{0};

  /*
   * Tracks whether we are currently in the stublogue context. This is set in
   * prologues prior to spilling the ActRec and used by catch blocks to ensure
   * the proper alignment of native stack.
   */
  bool stublogue{false};

  /*
   * ctx tracks the current ActRec's this/ctx field, origCtxType is the type of
   * the original non-refined ctx.
   */
  SSATmp* ctx{nullptr};
  Type ctxType{TObj|TCls};
  Type origCtxType{TObj|TCls};


  /*
   * stackModified is reset to false by exceptionStackBoundary() and set to
   * true by anything that modifies the eval stack. It's used to verify that
   * the stack is not modified between the beginning of a bytecode's
   * translation and creation of any catch traces, unless
   * exceptionStackBoundary() is explicitly called.
   */
  bool stackModified{false};

  /*
   * Whether the tracking of all locals has been cleared since the unit's entry.
   */
  bool localsCleared{false};

  /*
   * The values in the eval stack in memory, either above or below the current
   * spValue pointer.  This is keyed by the offset to the base of the eval stack
   * for the whole function (SBInvOffset).
   */
  StackStateMap stack;

  /*
   * Maps the local ids to local variable information.
   */
  LocalStateMap locals;

  /*
   * Values and types of the member base register, its pointee, the
   * temp base, or the read-only prop bool.
   */
  MBRState mbr;
  MBaseState mbase;
  MTempBaseState mTempBase;
  TriBool mROProp{TriBool::Maybe};

  /*
   * More specific types for the mbase if the mbase points at the
   * stack/locals/temp-base. These should always be a subtype of the
   * general mbase type.
   *
   * Tracking these allows for more precise analysis in
   * handleConservative() (we can avoid dropping these types if we
   * know these locations aren't affected).
   */
  Type mbaseStackType{TCell};
  Type mbaseLocalType{TCell};
  Type mbaseTempType{TCell};
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
  explicit FrameStateMgr(const Func* func);

  FrameStateMgr(const FrameStateMgr&) = delete;
  FrameStateMgr(FrameStateMgr&&) = default;
  FrameStateMgr& operator=(const FrameStateMgr&) = delete;
  FrameStateMgr& operator=(FrameStateMgr&&) = default;

  /*
   * Called when we are manually creating diamonds in the CFG.
   */
  void clearForUnprocessedPred();

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
   */
  void startBlock(Block* b, bool hasUnprocessedPred);

  /*
   * Finish tracking state for `b' and save the current state to b->next()
   * (b->taken() is handled in update()).
   *
   * Returns true iff the in-state for the next block has changed.
   */
  bool finishBlock(Block* b);

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

  /////////////////////////////////////////////////////////////////////////////

  /*
   * FrameState accessors.
   *
   * In the presence of inlining, these return state for the most-inlined
   * frame.  Shorthand for accessing the most inlined frame's data eg:
   *   fs()[fs().inlineDepth()].fp()
   */
  SSATmp*     fp()                const { return cur().fp(); }
  SSATmp*     fixupFP()           const { return cur().fixupFP(); }
  SSATmp*     sp()                const { return cur().sp(); }
  SSATmp*     ctx()               const { return cur().ctx; }
  Type        ctxType()           const { return cur().ctxType; }
  SBInvOffset irSPOff()           const { return cur().irSPOff; }
  SBInvOffset bcSPOff()           const { return cur().bcSPOff; }
  bool        stublogue()         const { return cur().stublogue; }
  bool        stackModified()     const { return cur().stackModified; }

  /*
   * Current inlining depth (not including the toplevel frame).
   */
  unsigned inlineDepth() const { return m_stack.size() - 1; }

  /*
   * Perform the frame state effects of EndInlining, without actually using
   * the instruction. InlineSideExit contains the other effects of EndInlining,
   * but it can't include the frame state ones, as they are needed for its own
   * catch block.
   */
  void endInliningForSideExit() { trackEndInlining(); }

  /*
   * Get the irSPOff for the parent frame of the most-inlined frame.
   *
   * @requires: inlineDepth() > 0
   */
  SBInvOffset callerIRSPOff() const {
    return caller().irSPOff;
  }

  /*
   * FrameState modifiers.
   *
   * In the presence of inlining, these modify state for the most-inlined
   * frame.
   */
  void resetStackModified()             { cur().stackModified = false; }
  void setBCSPOff(SBInvOffset o) {
    assertx(o.offset >= 0);
    cur().bcSPOff = o;
  }
  void incBCSPDepth(int32_t n = 1) {
    assertx(n >= 0);
    cur().bcSPOff += n;
  }
  void decBCSPDepth(int32_t n = 1) {
    assertx(bcSPOff().offset >= n);
    cur().bcSPOff -= n;
  }

  /*
   * Return the LocationState for local `id' or stack element at `off' in the
   * most-inlined frame.
   */
  LocalState local(uint32_t id) const;
  StackState stack(IRSPRelOffset off) const;
  StackState stack(SBInvOffset off) const;

  /*
   * Return whether the given location is currently being tracked.
   */
  bool tracked(Location l) const;

  /*
   * Return whether the given stack offset is valid for the current
   * frame (stack offsets from AliasClass might refer to outer frames,
   * which frame-state cannot currently handle).
   */
  bool validStackOffset(IRSPRelOffset off) const;

  /*
   * Return whether the state of the locals have even been cleared since the
   * unit's entry.
   */
  bool localsCleared() const { return cur().localsCleared; }

  /*
   * Generic accessors for LocationState members.
   */
  SSATmp* valueOf(Location l) const;
  Type typeOf(Location l) const;
  TypeSourceSet typeSrcsOf(Location l) const;

  /*
   * Return tracked state for the member base register.
   */
  const MBRState& mbr()             const { return cur().mbr; }
  const MBaseState& mbase()         const { return cur().mbase; }
  const MTempBaseState& mTempBase() const { return cur().mTempBase; }
  TriBool mROProp()                 const { return cur().mROProp; }

  /*
   * Check if a given TMem could be the current mbase.
   */
  TriBool isMBase(SSATmp*) const;

  /*
   * Return the best known type, or known SSATmp* for the pointee of a
   * given TMem.
   *
   * The type calculation will potentially short-cut if the calculated
   * type exceeds `limit`.
   */
  SSATmp* valueOfPointee(SSATmp*) const;
  Type typeOfPointee(SSATmp*, Type limit = TCell) const;

  /*
   * Debug stringification.
   */
  std::string show() const;

  FrameState& operator[](size_t depth) {
    assertx(m_stack.size() > depth);
    return m_stack[depth];
  }
  const FrameState& operator[](size_t depth) const {
    assertx(m_stack.size() > depth);
    return m_stack[depth];
  }

  /////////////////////////////////////////////////////////////////////////////
private:
  FrameState& cur() {
    assertx(!m_stack.empty());
    return m_stack.back();
  }
  const FrameState& cur() const {
    assertx(!m_stack.empty());
    return m_stack.back();
  }
  FrameState& caller() {
    assertx(inlineDepth() > 0);
    return m_stack.at(m_stack.size() - 2);
  }
  const FrameState& caller() const {
    assertx(inlineDepth() > 0);
    return m_stack.at(m_stack.size() - 2);
  }

  /*
   * LocationState access helpers.
   */
  Location loc(uint32_t) const;
  Location stk(SBInvOffset) const;
  Location stk(IRSPRelOffset) const;

  // Returns std::nullopt if offset is not valid for the current frame
  Optional<Location> optStk(SBInvOffset) const;
  Optional<Location> optStk(IRSPRelOffset) const;

  LocalState& localState(uint32_t);
  LocalState& localState(Location l); // @requires: l.tag() == LTag::Local
  StackState& stackState(IRSPRelOffset);
  StackState& stackState(SBInvOffset);
  StackState& stackState(Location l); // @requires: l.tag() == LTag::Stack

  AliasClass locationToAliasClass(Location) const;

  /*
   * Helpers for update().
   */
  bool checkInvariants() const;
  void handleConservatively(const IRInstruction*);
  void handleMInstr(const IRInstruction*);
  void initStack(SSATmp* sp, SBInvOffset irSPOff, SBInvOffset bcSPOff);
  void uninitStack();
  void trackEnterInlineFrame(const IRInstruction* inst);
  void trackEndInlining();
  void trackInlineCall(const IRInstruction* inst);
  void trackInlineSideExit(const IRInstruction* inst);

  void pointerLoad(SSATmp*, SSATmp*);
  void pointerStore(SSATmp*, SSATmp*);
  void pointerRefine(SSATmp*, Type);

  /*
   * Per-block state helpers.
   */
  bool save(Block* b);
  PostConditions collectPostConds();

  /*
   * LocationState update helpers.
   */
  void setValue(Location l, SSATmp* value);
  void setType(Location l, Type type);
  void widenType(Location l, Type type);
  void refineType(Location l, Type type, Optional<TypeSource> typeSrc);

  void setValue(const AliasClass&, SSATmp*);
  void refineType(const AliasClass&, Type, Optional<TypeSource>);

  void setValueAndSyncMBase(Location, SSATmp*, bool);
  void refineTypeAndSyncMBase(Location, Type, TypeSource);

  template<LTag tag>
  bool refineValue(LocationState<tag>& state, SSATmp* oldVal, SSATmp* newVal);
  void refineMBaseValue(SSATmp*, SSATmp*);

  /*
   * Local state update helpers.
   */
  void clearCtx();
  void clearLocals();

  /*
   * MTempBase update helpers.
   */
  void setMTempBase(SSATmp*);
  void setMTempBaseType(Type);
  void widenMTempBase(Type);
  void refineMTempBase(Type);

  /*
   * MBR update helpers.
   */
  void setMBR(SSATmp*, bool);
  void clearMInstr();
  TriBool isMBase(SSATmp*, const AliasClass&) const;

  /*
   * Helpers for working wth pointers
   */
  Type typeOfPointeeFromDefs(SSATmp*, Type) const;
  Type typeFromAliasClass(const AliasClass&) const;

private:
  struct BlockState {
    // Mandatory in-state computed from predecessors.
    jit::vector<FrameState> in;
    // Optionally-saved out-state.  Non-none but empty indicates that out-state
    // should be saved.
    Optional<jit::vector<FrameState>> out;
    // Paused state, used by IRBuilder::{push,pop}Block().
    Optional<jit::vector<FrameState>> paused;
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
};

///////////////////////////////////////////////////////////////////////////////

}}}
