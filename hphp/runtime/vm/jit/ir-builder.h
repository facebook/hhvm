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

#ifndef incl_HPHP_VM_IRBUILDER_H_
#define incl_HPHP_VM_IRBUILDER_H_

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

#include <folly/Optional.h>
#include <folly/ScopeGuard.h>

#include <functional>

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

struct ExnStackState {
  FPInvOffset syncedSpLevel{0};
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
 */
struct IRBuilder {
  IRBuilder(IRUnit&, BCMarker);

  /*
   * Accessors.
   */
  IRUnit& unit() const { return m_unit; }
  FrameStateMgr& fs() { return m_state; }
  BCMarker curMarker() const { return m_curBCContext.marker; }

  /*
   * Get the current BCContext, incrementing its `iroff'.
   */
  BCContext nextBCContext() {
    return BCContext { m_curBCContext.marker, m_curBCContext.iroff++ };
  }

  /*
   * Update the current BCContext.
   */
  void setCurMarker(BCMarker);
  void resetCurIROff() { m_curBCContext.iroff = 0; }

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
   * Tracked state for bytecode locations.
   *
   * These simply constrain the location, then delegate to fs().
   */
  const LocalState& local(uint32_t id, TypeConstraint tc);
  const StackState& stack(IRSPRelOffset offset, TypeConstraint tc);
  SSATmp* valueOf(Location l, TypeConstraint tc);
  Type     typeOf(Location l, TypeConstraint tc);

  /*
   * Helper for unboxing predicted types.
   *
   * @returns: ldRefReturn(fs().predictedTypeOf(location).unbox())
   */
  Type predictedLocalInnerType(uint32_t id) const;
  Type predictedStackInnerType(IRSPRelOffset) const;

  /////////////////////////////////////////////////////////////////////////////
  /*
   * Guard relaxation.
   *
   * Whenever the semantics of an HHIR instruction depends on the type of one
   * of its input values, that value's type must be constrained using one of
   * these functions. This happens automatically for most values, when obtained
   * through irgen-internal functions like popC (and friends).
   */

  /*
   * All the guards in the managed IRUnit.
   */
  const GuardConstraints* guards() const { return &m_constraints; }

  /*
   * Return true iff `tc' is more specific than the existing constraint for the
   * guard `inst'.
   *
   * This does not necessarily constrain the guard, if `tc.weak' is true.
   */
  bool constrainGuard(const IRInstruction* inst, TypeConstraint tc);

  /*
   * Trace back to the guard that provided the type of `val', if any, then
   * constrain it so that its type will not be relaxed beyond `tc'.
   *
   * Like constrainGuard(), this returns true iff `tc' is more specific than
   * the existing constraint, and does not constrain the guard if `tc.weak' is
   * true.
   */
  bool constrainValue(SSATmp* const val, TypeConstraint tc);

  /*
   * Constrain the type sources of the given bytecode location.
   */
  bool constrainLocation(Location l, TypeConstraint tc);
  bool constrainLocal(uint32_t id, TypeConstraint tc, const std::string& why);
  bool constrainStack(IRSPRelOffset offset, TypeConstraint tc);

  /*
   * Whether `val' might have its type relaxed by guard relaxation.
   *
   * If `val' is nullptr, only conditions that apply to all values are checked.
   */
  bool typeMightRelax(SSATmp* val = nullptr) const;

  /////////////////////////////////////////////////////////////////////////////
  // Bytecode-level control flow helpers.

  /*
   * The block that we're currently emitting code to.
   */
  Block* curBlock() { return m_curBlock; }

  /*
   * Return whether we have state saved for `block'---which indicates that it's
   * currently reachable from the unit's entry block.
   */
  bool canStartBlock(Block* block) const;

  /*
   * Start `block', returning success.
   *
   * We fail if `canStartBlock(block)' is false.
   */
  bool startBlock(Block* block, bool hasUnprocessedPred);

  /*
   * Create a new block corresponding to bytecode control flow.
   */
  Block* makeBlock(SrcKey sk, uint64_t profCount);

  /*
   * Check or set the block corresponding to `sk'.
   */
  bool hasBlock(SrcKey sk) const;
  void setBlock(SrcKey sk, Block* block);

  /*
   * Clear the SrcKey-to-block map.
   */
  void resetOffsetMapping();

  /*
   * Append `block' to the unit.
   *
   * This is used by irgen in IR-level control-flow helpers.  In certain cases,
   * these helpers may append unreachable blocks, which will not have a valid
   * in-state in FrameStateMgr.
   *
   * Rather than implicitly propagating the out state for m_curBlock, which is
   * the default behavior, `pred' can be set to indicate the logical
   * predecessor, in case `block' is unreachable.  If `block' is reachable,
   * `pred' is ignored.
   */
  void appendBlock(Block* block, Block* pred = nullptr);

  /*
   * Get, set, or null out the block to branch to in case of a guard failure.
   *
   * A nullptr guard fail block indicates that guard failures should end the
   * region and perform a service request.
   */
  Block* guardFailBlock() const;
  void setGuardFailBlock(Block* block);
  void resetGuardFailBlock();

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
   * Conditionally append a new instruction to the current Block, depending on
   * what some optimizations have to say about it.
   */
  enum class CloneFlag { Yes, No };
  SSATmp* optimizeInst(IRInstruction* inst,
                       CloneFlag doClone,
                       Block* srcBlock);

  /////////////////////////////////////////////////////////////////////////////
  // Internal API.

private:
  template<class... Args>
  SSATmp* gen(Opcode op, Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) {
        return optimizeInst(inst, CloneFlag::Yes, nullptr);
      },
      op,
      nextBCContext(),
      std::forward<Args>(args)...
    );
  }

  /*
   * Location wrapper helpers.
   */
  Location loc(uint32_t) const;
  Location stk(IRSPRelOffset) const;

  /*
   * preOptimize() and helpers.
   */
  SSATmp* preOptimizeCheckLocation(IRInstruction*, Location);
  SSATmp* preOptimizeCheckLoc(IRInstruction*);
  SSATmp* preOptimizeCheckStk(IRInstruction*);
  SSATmp* preOptimizeHintLocInner(IRInstruction*);
  SSATmp* preOptimizeAssertTypeOp(IRInstruction* inst,
                                  Type oldType,
                                  SSATmp* oldVal,
                                  const IRInstruction* typeSrc);
  SSATmp* preOptimizeAssertType(IRInstruction*);
  SSATmp* preOptimizeAssertLocation(IRInstruction*, Location);
  SSATmp* preOptimizeAssertLoc(IRInstruction*);
  SSATmp* preOptimizeAssertStk(IRInstruction*);
  SSATmp* preOptimizeLdARFuncPtr(IRInstruction*);
  SSATmp* preOptimizeCheckCtxThis(IRInstruction*);
  SSATmp* preOptimizeLdCtxHelper(IRInstruction*);
  SSATmp* preOptimizeLdCtx(IRInstruction* i) {
    return preOptimizeLdCtxHelper(i);
  }
  SSATmp* preOptimizeLdCctx(IRInstruction* i) {
    return preOptimizeLdCtxHelper(i);
  }
  SSATmp* preOptimizeLdLocation(IRInstruction*, Location);
  SSATmp* preOptimizeLdLoc(IRInstruction*);
  SSATmp* preOptimizeLdStk(IRInstruction*);
  SSATmp* preOptimizeCastStk(IRInstruction*);
  SSATmp* preOptimizeCoerceStk(IRInstruction*);
  SSATmp* preOptimizeLdMBase(IRInstruction*);
  SSATmp* preOptimize(IRInstruction*);

  void appendInstruction(IRInstruction* inst);

  /*
   * Type constraint helpers.
   */
  bool constrainLocation(Location l, TypeConstraint tc,
                         const std::string& why);
  bool constrainCheck(const IRInstruction* inst,
                      TypeConstraint tc, Type srcType);
  bool constrainAssert(const IRInstruction* inst,
                       TypeConstraint tc, Type srcType,
                       folly::Optional<Type> knownType = folly::none);
  bool constrainTypeSrc(TypeSource typeSrc, TypeConstraint tc);
  bool shouldConstrainGuards() const;

  /////////////////////////////////////////////////////////////////////////////

private:
  struct BlockState {
    Block* block;
    BCContext bcctx;
    ExnStackState exnStack;
    std::function<Block* ()> catchCreator;
  };

private:
  IRUnit& m_unit;
  BCMarker m_initialMarker;
  BCContext m_curBCContext;
  FrameStateMgr m_state;

  /*
   * m_savedBlocks will be nonempty iff we're emitting code to a block other
   * than the main block. m_curBCContext, and m_curBlock are all set from the
   * most recent call to pushBlock() or popBlock().
   */
  jit::vector<BlockState> m_savedBlocks;
  Block* m_curBlock;
  ExnStackState m_exnStack;

  bool m_enableSimplification{false};

  GuardConstraints m_constraints;

  // Keep track of blocks created to support bytecode control flow.
  jit::flat_map<SrcKey,Block*> m_skToBlockMap;

  // Keeps the block to branch to (if any) in case a guard fails.
  // This holds nullptr if the guard failures should perform a service
  // request (REQ_RETRANSLATE or REQ_BIND_JMP).
  Block* m_guardFailBlock{nullptr};
};

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

bool typeMightRelax(const SSATmp* tmp);

bool dictElemMightRelax(const IRInstruction* inst);
bool keysetElemMightRelax(const IRInstruction* inst);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
