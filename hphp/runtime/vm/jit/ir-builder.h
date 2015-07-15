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

#ifndef incl_HPHP_VM_IRBUILDER_H_
#define incl_HPHP_VM_IRBUILDER_H_

#include <functional>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/guard-constraints.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

struct ExnStackState {
  FPInvOffset spOffset;
  FPInvOffset syncedSpLevel;
  uint32_t stackDeficit;
  EvalStack evalStack;
  SSATmp* sp;
};

/*
 * This module provides the basic utilities for generating the IR instructions
 * in a trace and emitting control flow. It also performs some optimizations
 * while generating IR, and may be reinvoked for a second optimization pass.
 *
 * This module is also responsible for organizing a few types of
 * gen-time optimizations:
 *
 *   - preOptimize pass
 *
 *      Before an instruction is linked into the trace, IRBuilder
 *      internally runs preOptimize() on it, which can do some
 *      tracelet-state related modifications to the instruction.  For
 *      example, it can eliminate redundant guards.
 *
 *   - simplification pass
 *
 *      After the preOptimize pass, IRBuilder calls out to
 *      Simplifier to perform state-independent optimizations, like
 *      copy propagation and strength reduction.  (See simplify.h.)
 *
 *
 * After all the instructions are linked into the trace, this module can also
 * be used to perform a second round of the above two optimizations via the
 * reoptimize() entry point.
 */
struct IRBuilder {
  IRBuilder(IRUnit&, BCMarker);

  /*
   * Updates the marker used for instructions generated without one
   * supplied.
   */
  void setCurMarker(BCMarker);

  /*
   * Exception handling and IRBuilder.
   *
   * Normally HHBC opcodes that throw don't have any effects before they throw.
   * By default, when you gen() instructions that could throw, IRBuilder
   * automatically creates catch blocks that take the current frame-state
   * information, except spill the stack as if the instruction has not yet
   * started.
   *
   * There are some exceptions, and so there are two ways to modify this
   * behavior.  If an HHBC opcode should have some effects on the stack prior
   * to throwing, the lowering function can call exceptionStackBoundary after
   * doing this to inform IRBuilder that it's not a bug---in this case the
   * automatically created catch blocks will spill the stack as of the last
   * boundary.
   *
   * The other way is to set a custom catch creator function.  This is
   * basically for the minstr instructions, which has various temporary stack
   * state to clean up during unwinding.
   */
  void exceptionStackBoundary();
  const ExnStackState& exceptionStackState() const { return m_exnStack; }

  /*
   * The following functions are an abstraction layer we probably don't need.
   * You can keep using them until we find time to remove them.
   */
  IRUnit& unit() const { return m_unit; }
  FrameStateMgr& fs() { return m_state; }
  BCMarker curMarker() const { return m_curMarker; }
  const Func* curFunc() const { return m_state.func(); }
  FPInvOffset spOffset() { return m_state.spOffset(); }
  SSATmp* sp() const { return m_state.sp(); }
  SSATmp* fp() const { return m_state.fp(); }
  uint32_t stackDeficit() const { return m_state.stackDeficit(); }
  void incStackDeficit() { m_state.incStackDeficit(); }
  void clearStackDeficit() { m_state.clearStackDeficit(); }
  void setStackDeficit(uint32_t d) { m_state.setStackDeficit(d); }
  void syncEvalStack() { m_state.syncEvalStack(); }
  EvalStack& evalStack() { return m_state.evalStack(); }
  FPInvOffset syncedSpLevel() const { return m_state.syncedSpLevel(); }
  bool thisAvailable() const { return m_state.thisAvailable(); }
  void setThisAvailable() { m_state.setThisAvailable(); }
  Type localType(uint32_t id, TypeConstraint tc);
  Type stackType(IRSPOffset, TypeConstraint tc);
  Type predictedInnerType(uint32_t id);
  Type predictedLocalType(uint32_t id);
  Type predictedStackType(IRSPOffset);
  SSATmp* localValue(uint32_t id, TypeConstraint tc);
  SSATmp* stackValue(IRSPOffset offset, TypeConstraint tc);
  TypeSourceSet localTypeSources(uint32_t id) const {
    return m_state.localTypeSources(id);
  }
  TypeSourceSet stackTypeSources(IRSPOffset offset) const {
    return m_state.stackTypeSources(offset);
  }
  bool frameMaySpanCall() const { return m_state.frameMaySpanCall(); }
  Type stackInnerTypePrediction(IRSPOffset) const;
  const PostConditions& postConds(Block* b) const {
    return m_state.postConds(b);
  }

  /*
   * Support for guard relaxation.
   *
   * Whenever the semantics of an hhir operation depends on the type of one of
   * its input values, that value's type must be constrained using one of these
   * methods. This happens automatically for most values, when obtained through
   * irgen-internal functions like popC (and friends).
   */
  void setConstrainGuards(bool constrain) { m_constrainGuards = constrain; }
  bool shouldConstrainGuards()      const { return m_constrainGuards; }
  bool constrainGuard(const IRInstruction* inst, TypeConstraint tc);
  bool constrainValue(SSATmp* const val, TypeConstraint tc);
  bool constrainLocal(uint32_t id, TypeConstraint tc, const std::string& why);
  bool constrainStack(IRSPOffset offset, TypeConstraint tc);
  bool typeMightRelax(SSATmp* val = nullptr) const;
  const GuardConstraints* guards() const { return &m_constraints; }

public:
  /*
   * API for managing state when building IR with bytecode-level control flow.
   */

  /*
   * Start the given block.  Returns whether or not it succeeded.  A failure
   * may occur in case the block turned out to be unreachable.
   */
  bool startBlock(Block* block, bool hasUnprocPred);

  /*
   * Returns whether or not `block' will succeed if passed to
   * startBlock, which implies that we have state saved for `block',
   * and therefore it's currently reachable from the unit's entry
   * block.
   */
  bool canStartBlock(Block* block) const;

  /*
   * Create a new block corresponding to bytecode control flow.
   */
  Block* makeBlock(Offset offset);

  /*
   * Clear the map from bytecode offsets to Blocks.
   */
  void resetOffsetMapping();

  /*
   * Checks whether or not there's a block associated with the given
   * bytecode offset.
   */
  bool hasBlock(Offset offset) const;

  /*
   * Set the block associated with the given offset in the offset->block map.
   */
  void setBlock(Offset offset, Block* block);

  /*
   * Get the block that we're currently emitting code to.
   */
  Block* curBlock() { return m_curBlock; }

  /*
   * Append a new block to the unit.
   */
  void appendBlock(Block* block);

  /*
   * Set the block to branch to in case a guard fails.
   */
  void setGuardFailBlock(Block* block);

  /*
   * Resets the guard failure block to nullptr.
   */
  void resetGuardFailBlock();

  /*
   * Returns the block to branch to in case of a guard failure.  This
   * returns nullptr if no such block has been set, and therefore
   * guard failures should end the region and perform a service
   * request.
   */
  Block* guardFailBlock() const;

public:
  /*
   * To emit code to a block other than the current block, call pushBlock(),
   * emit instructions as usual with gen(...), then call popBlock(). This is
   * best done using the BlockPusher struct:
   *
   * gen(CodeForMainBlock, ...);
   * {
   *   BlockPusher<PauseExit> bp(m_irb, marker, exitBlock);
   *   gen(CodeForExitBlock, ...);
   * }
   * gen(CodeForMainBlock, ...);
   */
  void pushBlock(BCMarker marker, Block* b);
  void popBlock();

  /*
   * Conditionally-append a new instruction to the current Block, depending on
   * what some optimizations have to say about it.
   */
  enum class CloneFlag { Yes, No };
  SSATmp* optimizeInst(IRInstruction* inst,
                       CloneFlag doClone,
                       Block* srcBlock);


private:
  struct BlockState {
    Block* block;
    BCMarker marker;
    ExnStackState exnStack;
    std::function<Block* ()> catchCreator;
  };

private:
  // Helper for cond() and such.  We should move them out of IRBuilder so they
  // can just use irgen::gen.
  template<class... Args>
  SSATmp* gen(Opcode op, Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) {
        return optimizeInst(inst, CloneFlag::Yes, nullptr);
      },
      op,
      m_curMarker,
      std::forward<Args>(args)...
    );
  }

private:
  SSATmp* preOptimizeCheckTypeOp(IRInstruction*, Type);
  SSATmp* preOptimizeCheckType(IRInstruction*);
  SSATmp* preOptimizeCheckStk(IRInstruction*);
  SSATmp* preOptimizeCheckLoc(IRInstruction*);
  SSATmp* preOptimizeHintLocInner(IRInstruction*);
  SSATmp* preOptimizeAssertTypeOp(IRInstruction* inst,
                                  Type oldType,
                                  SSATmp* oldVal,
                                  const IRInstruction* typeSrc);
  SSATmp* preOptimizeAssertType(IRInstruction*);
  SSATmp* preOptimizeAssertStk(IRInstruction*);
  SSATmp* preOptimizeAssertLoc(IRInstruction*);
  SSATmp* preOptimizeCheckCtxThis(IRInstruction*);
  SSATmp* preOptimizeLdCtx(IRInstruction*);
  SSATmp* preOptimizeLdLocPseudoMain(IRInstruction*);
  SSATmp* preOptimizeLdLoc(IRInstruction*);
  SSATmp* preOptimizeStLoc(IRInstruction*);
  SSATmp* preOptimizeCastStk(IRInstruction*);
  SSATmp* preOptimizeCoerceStk(IRInstruction*);
  SSATmp* preOptimizeLdStk(IRInstruction*);
  SSATmp* preOptimize(IRInstruction*);

private:
  void appendInstruction(IRInstruction* inst);
  bool constrainSlot(int32_t idOrOffset,
                     TypeSource typeSrc,
                     TypeConstraint tc,
                     const std::string& why);

private:
  IRUnit& m_unit;
  BCMarker m_initialMarker;
  BCMarker m_curMarker;
  FrameStateMgr m_state;

  /*
   * m_savedBlocks will be nonempty iff we're emitting code to a block other
   * than the main block. m_curMarker, and m_curBlock are all set from the
   * most recent call to pushBlock() or popBlock().
   */
  jit::vector<BlockState> m_savedBlocks;
  Block* m_curBlock;
  ExnStackState m_exnStack{
    FPInvOffset{0},
    FPInvOffset{0},
    0,
    EvalStack{},
    nullptr
  };

  bool m_enableSimplification{false};
  bool m_constrainGuards;

  GuardConstraints m_constraints;

  // Keep track of blocks created to support bytecode control flow.
  //
  // TODO(t3730559): Offset is used here since it's passed from
  // emitJmp*, but SrcKey might be better in case of inlining.
  jit::flat_map<Offset,Block*> m_offsetToBlockMap;

  // Keeps the block to branch to (if any) in case a guard fails.
  // This holds nullptr if the guard failures should perform a service
  // request (REQ_RETRANSLATE or REQ_BIND_JMP).
  Block* m_guardFailBlock{nullptr};
};

//////////////////////////////////////////////////////////////////////

/*
 * RAII helper for emitting code to exit traces. See IRBuilder::pushBlock
 * for usage.
 */
struct BlockPusher {
  BlockPusher(IRBuilder& irb, BCMarker marker, Block* block)
    : m_irb(irb)
  {
    irb.pushBlock(marker, block);
  }

  ~BlockPusher() {
    m_irb.popBlock();
  }

 private:
  IRBuilder& m_irb;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
