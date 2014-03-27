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

#include <boost/scoped_ptr.hpp>
#include <vector>

#include "folly/ScopeGuard.h"
#include "folly/Optional.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////

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
 *   - value numbering
 *
 *      After preOptimize, instructions that support it are hashed and
 *      looked up in the CSEHash for this trace.  If we find an
 *      available expression for the same value, instead of linking a
 *      new instruction into the trace we will just add a use to the
 *      previous SSATmp.
 *
 *   - simplification pass
 *
 *      After the preOptimize pass, IRBuilder calls out to
 *      Simplifier to perform state-independent optimizations, like
 *      copy propagation and strength reduction.  (See simplifier.h.)
 *
 *
 * After all the instructions are linked into the trace, this module can also
 * be used to perform a second round of the above two optimizations via the
 * reoptimize() entry point.
 */
struct IRBuilder {
  IRBuilder(Offset initialBcOffset,
            Offset initialSpOffsetFromFp,
            IRUnit&,
            const Func* func);
  ~IRBuilder();

  void setEnableSimplification(bool val) { m_enableSimplification = val; }
  bool typeMightRelax(SSATmp* val = nullptr) const;

  IRUnit& unit() const { return m_unit; }
  FrameState& state() { return m_state; }
  const Func* curFunc() const { return m_state.func(); }
  int32_t spOffset() { return m_state.spOffset(); }
  SSATmp* sp() const { return m_state.sp(); }
  SSATmp* fp() const { return m_state.fp(); }
  const GuardConstraints* guards() const { return &m_guardConstraints; }
  uint32_t stackDeficit() const { return m_state.stackDeficit(); }
  void incStackDeficit() { m_state.incStackDeficit(); }
  void clearStackDeficit() { m_state.clearStackDeficit(); }
  EvalStack& evalStack() { return m_state.evalStack(); }
  bool thisAvailable() const { return m_state.thisAvailable(); }
  void setThisAvailable() { m_state.setThisAvailable(); }

  /*
   * Support for guard relaxation. Whenever the semantics of an hhir operation
   * depends on the type of one of its input values, that value's type must be
   * constrained using one of these methods. This happens automatically for
   * most values, when obtained through HhbcTranslator::popC (and friends).
   */
  void setConstrainGuards(bool constrain) { m_constrainGuards = constrain; }
  bool shouldConstrainGuards()      const { return m_constrainGuards; }
  bool constrainGuard(IRInstruction* inst, TypeConstraint tc);
  bool constrainValue(SSATmp* const val, TypeConstraint tc);
  bool constrainLocal(uint32_t id, TypeConstraint tc,
                      const std::string& why);
  bool constrainLocal(uint32_t id, SSATmp* valSrc, TypeConstraint tc,
                      const std::string& why);
  bool constrainStack(int32_t offset, TypeConstraint tc);
  bool constrainStack(SSATmp* sp, int32_t offset, TypeConstraint tc);

  Type localType(uint32_t id, TypeConstraint tc);
  SSATmp* localValue(uint32_t id, TypeConstraint tc);
  SSATmp* localTypeSource(uint32_t id) const {
    return m_state.localTypeSource(id);
  }
  bool inlinedFrameSpansCall() const { return m_state.frameSpansCall(); }

  /*
   * Updates the marker used for instructions generated without one
   * supplied.
   */
  void setMarker(BCMarker marker);

 public:
  /*
   * API for managing state when building IR with bytecode-level
   * control flow.
   */

  /*
   * Start a new block using the current marker.
   */
  void startBlock();

  /*
   * Create a new block corresponding to bytecode control flow.
   */
  Block* makeBlock(Offset offset);

  /*
   * Block has been created and added to the offset map.
   */
  bool blockExists(Offset offset);

  /*
   * True if translating the block at offset is incompatible with the
   * current state.  This is possible if the target block has already
   * been translated, or if the types of guarded locals do not match.
   *
   * TODO(t3730468): Should we check guarded stack types here as well?
   */
  bool blockIsIncompatible(Offset offset);

  /*
   * Note that we've seen this offset as the start of a block.
   */
  void recordOffset(Offset offset);

 public:
  /*
   * To emit code to a block other than the current block, call pushBlock(),
   * emit instructions as usual with gen(...), then call popBlock(). This is
   * best done using the BlockPusher struct:
   *
   * gen(CodeForMainBlock, ...);
   * {
   *   BlockPusher bp(m_irb, marker, exitBlock);
   *   gen(CodeForExitBlock, ...);
   * }
   * gen(CodeForMainBlock, ...);
   *
   * Where may be supplied to emit code to a specific location in the block,
   * otherwise code will be appended to the block.
   */
  void pushBlock(BCMarker marker, Block* b,
                 const folly::Optional<Block::iterator>& where);
  void popBlock();

  /*
   * Run another pass of IRBuilder-managed optimizations on this
   * unit.
   */
  void reoptimize();

  /*
   * Create an IRInstruction at the end of the current Block, and allocate a
   * destination SSATmp for it.  Uses the same argument list format as
   * IRUnit::gen.
   */
  template<class... Args>
  SSATmp* gen(Opcode op, Args&&... args) {
    return gen(op, m_state.marker(), std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* gen(Opcode op, BCMarker marker, Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) {
        return optimizeInst(inst, CloneFlag::Yes, folly::none);
      },
      op,
      marker,
      std::forward<Args>(args)...
    );
  }

  /*
   * Add an already created instruction, running it through the normal
   * optimization passes and updating tracked state.
   */
  SSATmp* add(IRInstruction* inst) {
    return optimizeInst(inst, CloneFlag::No, folly::none);
  }

  //////////////////////////////////////////////////////////////////////
  // constants

  SSATmp* genPtrToInitNull();
  SSATmp* genPtrToUninit();

  template<class... Args>
  SSATmp* cns(Args&&... args) {
    return m_unit.cns(std::forward<Args>(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  // control flow

  // hint the execution frequency of the current block
  void hint(Block::Hint h) const {
    m_curBlock->setHint(h);
  }

 private:
  template<typename T> struct BranchImpl;
 public:
  /*
   * cond() generates if-then-else blocks within a trace.  The caller supplies
   * lambdas to create the branch, next-body, and taken-body.  The next and
   * taken lambdas must return one SSATmp* value; cond() returns the SSATmp for
   * the merged value.
   *
   * If branch returns void, next must take zero arguments. If branch returns
   * SSATmp*, next must take one SSATmp* argument. This allows branch to return
   * an SSATmp* that is only defined in the next branch, without letting it
   * escape into the caller's scope (most commonly used with things like
   * LdMem).
   */
  template <class Branch, class Next, class Taken>
  SSATmp* cond(Branch branch, Next next, Taken taken) {
    Block* taken_block = m_unit.defBlock();
    Block* done_block = m_unit.defBlock();
    IRInstruction* label = m_unit.defLabel(1, m_state.marker());
    done_block->push_back(label);
    DisableCseGuard guard(*this);

    typedef decltype(branch(taken_block)) T;
    SSATmp* v1 = BranchImpl<T>::go(branch, taken_block, next);
    gen(Jmp, done_block, v1);
    appendBlock(taken_block);
    SSATmp* v2 = taken();
    gen(Jmp, done_block, v2);
    appendBlock(done_block);
    SSATmp* result = label->dst(0);
    result->setType(Type::unionOf(v1->type(), v2->type()));
    return result;
  }

  /*
   * ifThen generates if-then blocks within a trace that do not
   * produce values. Code emitted in the taken lambda will be executed
   * iff the branch emitted in the branch lambda is taken.
   */
  template <class Branch, class Taken>
  void ifThen(Branch branch, Taken taken) {
    Block* taken_block = m_unit.defBlock();
    Block* done_block = m_unit.defBlock();
    DisableCseGuard guard(*this);
    branch(taken_block);
    Block* last = m_curBlock;
    assert(!last->next());
    last->back().setNext(done_block);
    appendBlock(taken_block);
    taken();
    // patch the last block added by the Taken lambda to jump to
    // the done block.  Note that last might not be taken_block.
    last = m_curBlock;
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else {
      last->back().setNext(done_block);
    }
    appendBlock(done_block);
  }

  /*
   * ifElse generates if-then-else blocks with an empty 'then' block
   * that do not produce values. Code emitted in the next lambda will
   * be executed iff the branch emitted in the branch lambda is not
   * taken.
   */
  template <class Branch, class Next>
  void ifElse(Branch branch, Next next) {
    Block* done_block = m_unit.defBlock();
    DisableCseGuard guard(*this);
    branch(done_block);
    next();
    // patch the last block added by the Next lambda to jump to
    // the done block.
    auto last = m_curBlock;
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else {
      last->back().setNext(done_block);
    }
    appendBlock(done_block);
  }

  /*
   * Create a new "exit block". This is a Block that is assumed to be
   * a cold path, which always exits the tracelet without control flow
   * rejoining the main line.
   */
  Block* makeExit() {
    auto* exit = m_unit.defBlock();
    exit->setHint(Block::Hint::Unlikely);
    return exit;
  }

  /*
   * Get all typed locations in current translation.
   */
  std::vector<RegionDesc::TypePred> getKnownTypes() const;

private:
  // RAII disable of CSE; only restores if it used to be on.  Used for
  // control flow, where we currently don't allow CSE.
  struct DisableCseGuard {
    explicit DisableCseGuard(IRBuilder& irb)
      : m_state(irb.m_state)
      , m_oldEnable(m_state.enableCse())
    {
      m_state.setEnableCse(false);
    }
    ~DisableCseGuard() {
      m_state.setEnableCse(m_oldEnable);
    }

   private:
    FrameState& m_state;
    bool m_oldEnable;
  };

private:
  using ConstraintFunc = std::function<void(TypeConstraint)>;
  SSATmp*   preOptimizeAssertTypeOp(IRInstruction*, Type, ConstraintFunc);
  SSATmp*   preOptimizeCheckType(IRInstruction*);
  SSATmp*   preOptimizeCheckStk(IRInstruction*);
  SSATmp*   preOptimizeCheckLoc(IRInstruction*);
  SSATmp*   preOptimizeAssertLoc(IRInstruction*);
  SSATmp*   preOptimizeAssertStk(IRInstruction*);
  SSATmp*   preOptimizeAssertType(IRInstruction*);
  SSATmp*   preOptimizeLdThis(IRInstruction*);
  SSATmp*   preOptimizeLdCtx(IRInstruction*);
  SSATmp*   preOptimizeDecRefThis(IRInstruction*);
  SSATmp*   preOptimizeDecRefLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLocAddr(IRInstruction*);
  SSATmp*   preOptimizeStLoc(IRInstruction*);
  SSATmp*   preOptimize(IRInstruction* inst);

  enum class CloneFlag { Yes, No };
  SSATmp*   optimizeInst(IRInstruction* inst,
                         CloneFlag doClone,
                         const folly::Optional<IdomVector>& idoms);

private:
  void      appendInstruction(IRInstruction* inst);
  void      appendBlock(Block* block);

private:
  IRUnit& m_unit;
  Simplifier m_simplifier;
  FrameState m_state;

  /*
   * m_savedBlocks will be nonempty iff we're emitting code to a block other
   * than the main block. m_curMarker, m_curBlock, m_curWhere are
   * all set from the most recent call to pushBlock() or popBlock().
   */
  struct BlockState {
    Block* block;
    BCMarker marker;
    folly::Optional<Block::iterator> where;
  };
  smart::vector<BlockState> m_savedBlocks;
  Block* m_curBlock;
  folly::Optional<Block::iterator> m_curWhere;

  bool m_enableSimplification;
  bool m_constrainGuards;

  GuardConstraints m_guardConstraints;

  // Keep track of blocks created to support bytecode control flow.
  //
  // TODO(t3730559): Offset is used here since it's passed from
  // emitJmp*, but SrcKey might be better in case of inlining.
  smart::flat_map<Offset,Block*> m_offsetToBlockMap;

  // Track the offsets of every bytecode block that is started by
  // translateRegion.
  //
  // TODO(t3730581): Slightly redundant with m_offsetToBlockMap, but
  // not completely.  It is used to prevent translating backward
  // branches to blocks on the main trace.  We should be able to kill
  // this eventually.
  smart::flat_set<Offset> m_offsetSeen;
};

/*
 * BranchImpl is used by IRBuilder::cond to support branch and next lambdas
 * with different signatures. See cond for details.
 */
template<> struct IRBuilder::BranchImpl<void> {
  template<typename Branch, typename Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    branch(taken);
    return next();
  }
};

template<> struct IRBuilder::BranchImpl<SSATmp*> {
  template<typename Branch, typename Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    return next(branch(taken));
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * RAII helper for emitting code to exit traces. See IRBuilder::pushTrace
 * for usage.
 */
struct BlockPusher {
  BlockPusher(IRBuilder& irb, BCMarker marker, Block* block,
              const folly::Optional<Block::iterator>& where = folly::none)
    : m_irb(irb)
  {
    irb.pushBlock(marker, block, where);
  }

  ~BlockPusher() {
    m_irb.popBlock();
  }

 private:
  IRBuilder& m_irb;
};

}}

#endif
