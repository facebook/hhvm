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
struct FrameState {
  FrameState(IRUnit& unit, Offset initialSpOffset, const Func* func);
  ~FrameState();

  void update(IRInstruction* inst);
  void appendBlock(Block* block);
  void load(Block* dest);
  void save(Block* dest);
  void clear();

  const Func* func() const { return m_curFunc; }
  Offset spOffset() const { return m_spOffset; }
  SSATmp* sp() const { return m_spValue; }
  SSATmp* fp() const { return m_fpValue; }
  bool thisAvailable() const { return m_thisAvailable; }
  void setThisAvailable() { m_thisAvailable = true; }
  bool frameSpansCall() const { return m_frameSpansCall; }
  BCMarker marker() const { return m_marker; }
  void setMarker(BCMarker m) { m_marker = m; }
  bool needsFPAnchor(IRInstruction*) const;
  void setHasFPAnchor() { m_hasFPAnchor = true; }
  bool enableCse() const { return m_enableCse; }
  void setEnableCse(bool e) { m_enableCse = e; }

  Type localType(uint32_t id) const;
  SSATmp* localValue(uint32_t id) const;
  SSATmp* localValueSource(uint32_t id) const;

  bool isValueAvailable(SSATmp*) const;
  bool callerHasValueAvailable(SSATmp*) const;

  SSATmp* cseLookup(IRInstruction* inst, const folly::Optional<IdomVector>&);

 private:
  /*
   * LocalState stores information about a local in the current function.
   */
  struct LocalState {
    LocalState()
      : value(nullptr)
      , type(Type::Gen)
      , unsafe(false)
      , written(false)
    {}

    SSATmp* value; // The current value of the local. nullptr if unknown
    Type type;     // The current type of the local.
    bool unsafe;   // true iff value is not safe to use at runtime. Currently
                   // this only happens across a Call or CallArray instruction.
    bool written;  // true iff the local has been written in this trace
  };

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
    smart::vector<LocalState> locals;
    bool frameSpansCall;
    bool needsFPAnchor;
    std::vector<SSATmp*> callerAvailableValues;
    BCMarker curMarker;
  };

  void trackDefInlineFP(IRInstruction* inst);
  void trackInlineReturn(IRInstruction* inst);

  void setLocalValue(uint32_t id, SSATmp* value);
  void setLocalType(uint32_t id, Type type);
  void refineLocalType(uint32_t id, Type type);
  void clearLocals();
  void killLocalValue(uint32_t id);
  void killLocalsForCall();
  void updateLocalValues(SSATmp* oldVal, SSATmp* newVal);
  void updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef);
  void dropLocalRefsInnerTypes();
  bool anyLocalHasValue(SSATmp*) const;

  void cseInsert(IRInstruction* inst);
  void cseKill(SSATmp* src);
  CSEHash* cseHashTable(IRInstruction* inst);
  void clearCse();

  std::unique_ptr<Snapshot> createSnapshot() const;
  void merge(Snapshot* s1);
  void use(std::unique_ptr<Snapshot> state);
  void use(Block*);

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
   * m_hasFPAnchor is a (hopefully) temporary hack to work around the fact that
   * many instructions that can use the frame pointer by raising an error don't
   * take m_fpValue as a source.
   */
  bool m_hasFPAnchor;

  /*
   * m_locals tracks the current types and values of locals.
   */
  smart::vector<LocalState> m_locals;

  /*
   * m_inlineSavedStates holds snapshots of the caller(s)'s state while in an
   * inlined callee.
   */
  std::vector<std::unique_ptr<Snapshot>> m_inlineSavedStates;

  // Values known to be "available" for the purposes of DecRef to
  // DecRefNZ transformations due to locals of the caller for an
  // inlined call.
  std::vector<SSATmp*> m_callerAvailableValues;

  /*
   * m_cseHash holds the destination of all tracked instructions that produced
   * values eligible for CSE.
   */
  CSEHash m_cseHash;
  bool m_enableCse;

  /*
   * Saved snapshots of the incoming state for Blocks.
   */
  StateVector<Block, Snapshot*> m_snapshots;
};

} }

#endif
