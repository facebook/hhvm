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
#include "hphp/runtime/vm/jit/guard-constraints.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace jit {

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
 *      copy propagation and strength reduction.  (See simplify.h.)
 *
 *
 * After all the instructions are linked into the trace, this module can also
 * be used to perform a second round of the above two optimizations via the
 * reoptimize() entry point.
 */
struct IRBuilder {
  IRBuilder(IRUnit&, BCMarker);
  ~IRBuilder();

  void setEnableSimplification(bool val) { m_enableSimplification = val; }
  bool typeMightRelax(SSATmp* val = nullptr) const;

  IRUnit& unit() const { return m_unit; }
  BCMarker marker() const { return m_state.marker(); }
  const Func* curFunc() const { return m_state.func(); }
  int32_t spOffset() { return m_state.spOffset(); }
  SSATmp* sp() const { return m_state.sp(); }
  SSATmp* fp() const { return m_state.fp(); }
  const GuardConstraints* guards() const { return &m_constraints; }
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
  bool constrainGuard(const IRInstruction* inst, TypeConstraint tc);
  bool constrainValue(SSATmp* const val, TypeConstraint tc);
  bool constrainLocal(uint32_t id, TypeConstraint tc, const std::string& why);
  bool constrainStack(int32_t offset, TypeConstraint tc);
  bool constrainStack(SSATmp* sp, int32_t offset, TypeConstraint tc);

  Type localType(uint32_t id, TypeConstraint tc);
  Type predictedInnerType(uint32_t id);
  SSATmp* localValue(uint32_t id, TypeConstraint tc);
  TypeSourceSet localTypeSources(uint32_t id) const {
    return m_state.localTypeSources(id);
  }
  bool inlinedFrameSpansCall() const { return m_state.frameSpansCall(); }

  /*
   * Updates the marker used for instructions generated without one
   * supplied.
   */
  void setMarker(BCMarker marker);

 public:
  /*
   * API for managing state when building IR with bytecode-level control flow.
   */

  /*
   * Start the given block.  Returns whether or not it succeeded.  A failure
   * may occur in case the block turned out to be unreachable.
   */
  bool startBlock(Block* block, const BCMarker& marker, bool isLoopHeader);

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
   *
   * Where may be supplied to emit code to a specific location in the block,
   * otherwise code will be appended to the block.
   */
  void pushBlock(BCMarker marker, Block* b,
                 const folly::Optional<Block::iterator>& where);
  void popBlock(bool);

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
        return optimizeInst(inst, CloneFlag::Yes, nullptr, folly::none);
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
    return optimizeInst(inst, CloneFlag::No, nullptr, folly::none);
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
   *
   * The producedRefs argument is needed for the refcount optimizations in
   * refcount-opts.cpp. It should be the number of unconsumed references
   * forwarded from each Jmp src to the DefLabel's dst (for a description of
   * reference producers and consumers, read the "Refcount Optimizations"
   * section in hphp/doc/hackers-guide/jit-optimizations.md). As an example,
   * code that looks like the following should pass 1 for producedRefs, since
   * LdCns and LookupCns each produce a reference that should then be forwarded
   * to t2, the dest of the DefLabel:
   *
   * B0:
   *   t0:FramePtr = DefFP
   *   t1:Cell = LdCns "foo"        // produce reference to t1
   *   CheckInit t1:Cell -> B3<Unlikely>
   *  -> B1
   *
   * B1 (preds B0):
   *   Jmp t1:Cell -> B2            // forward t1's unconsumed ref to t2
   *
   * B2 (preds B1, B3):
   *   t2:Cell = DefLabel           // produce reference to t2, from t1 and t4
   *   StLoc<1> t0:FramePtr t2:Cell // consume reference to t2
   *   Halt
   *
   * B3<Unlikely> (preds B0):
   *   t3:Uninit = AssertType<Uninit> t1:Cell // consume reference to t1
   *   t4:Cell = LookupCns "foo"    // produce reference to t4
   *   Jmp t4:Cell -> B2            // forward t4's unconsumed ref to t2
   *
   * A sufficiently advanced analysis pass could deduce this value from the
   * structure of the IR, but it would require traversing all possible control
   * flow paths, causing an explosion of required CPU time and/or memory.
   */
  template <class Branch, class Next, class Taken>
  SSATmp* cond(unsigned producedRefs, Branch branch, Next next, Taken taken) {
    Block* taken_block = m_unit.defBlock();
    Block* done_block = m_unit.defBlock();
    DisableCseGuard guard(*this);

    typedef decltype(branch(taken_block)) T;
    SSATmp* v1 = BranchImpl<T>::go(branch, taken_block, next);
    gen(Jmp, done_block, v1);
    appendBlock(taken_block);
    SSATmp* v2 = taken();
    gen(Jmp, done_block, v2);

    appendBlock(done_block);
    IRInstruction* label = m_unit.defLabel(1, m_state.marker(), {producedRefs});
    done_block->push_back(label);
    SSATmp* result = label->dst(0);
    result->setType(Type::unionOf(v1->type(), v2->type()));
    return result;
  }

  /*
   * ifThenElse() generates if-then-else blocks within a trace that do not
   * produce values. Code emitted in the {next,taken} lambda will be executed
   * iff the branch emitted in the branch lambda is {not,} taken.
   */
  template <class Branch, class Next, class Taken>
  void ifThenElse(Branch branch, Next next, Taken taken) {
    Block* taken_block = m_unit.defBlock();
    Block* done_block = m_unit.defBlock();
    DisableCseGuard guard(*this);
    branch(taken_block);
    next();
    // patch the last block added by the Next lambda to jump to
    // the done block.  Note that last might not be taken_block.
    Block* last = m_curBlock;
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else if (!last->back().isTerminal()) {
      last->back().setNext(done_block);
    }
    appendBlock(taken_block);
    taken();
    // patch the last block added by the Taken lambda to jump to
    // the done block.  Note that last might not be taken_block.
    last = m_curBlock;
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else if (!last->back().isTerminal()) {
      last->back().setNext(done_block);
    }
    appendBlock(done_block);
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
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else if (!last->back().isTerminal()) {
      last->back().setNext(done_block);
    }
    appendBlock(taken_block);
    taken();
    // patch the last block added by the Taken lambda to jump to
    // the done block.  Note that last might not be taken_block.
    last = m_curBlock;
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else if (!last->back().isTerminal()) {
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
    } else if (!last->back().isTerminal()) {
      last->back().setNext(done_block);
    }
    appendBlock(done_block);
  }

  /*
   * Create a new "exit block". This is a Block that is assumed to be
   * a cold path, which always exits the tracelet without control flow
   * rejoining the main line.
   */
  Block* makeExit(Block::Hint hint = Block::Hint::Unlikely) {
    auto* exit = m_unit.defBlock();
    exit->setHint(hint);
    return exit;
  }

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
  using ConstrainBoxedFunc = std::function<SSATmp*(Type)>;
  SSATmp*   preOptimizeCheckTypeOp(IRInstruction* inst,
                                   Type oldType,
                                   ConstrainBoxedFunc func);
  SSATmp*   preOptimizeCheckType(IRInstruction*);
  SSATmp*   preOptimizeCheckStk(IRInstruction*);
  SSATmp*   preOptimizeCheckLoc(IRInstruction*);
  SSATmp*   preOptimizeHintLocInner(IRInstruction*);

  SSATmp*   preOptimizeAssertTypeOp(IRInstruction* inst,
                                    Type oldType,
                                    SSATmp* oldVal,
                                    const IRInstruction* typeSrc);
  SSATmp*   preOptimizeAssertType(IRInstruction*);
  SSATmp*   preOptimizeAssertStk(IRInstruction*);
  SSATmp*   preOptimizeAssertLoc(IRInstruction*);

  SSATmp*   preOptimizeCheckCtxThis(IRInstruction*);
  SSATmp*   preOptimizeLdCtx(IRInstruction*);
  SSATmp*   preOptimizeDecRefThis(IRInstruction*);
  SSATmp*   preOptimizeDecRefLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLocPseudoMain(IRInstruction*);
  SSATmp*   preOptimizeLdLoc(IRInstruction*);
  SSATmp*   preOptimizeStLoc(IRInstruction*);
  SSATmp*   preOptimize(IRInstruction* inst);

  bool      constrainLocal(uint32_t id,
                           TypeSource typeSrc,
                           TypeConstraint tc,
                           const std::string& why);

  enum class CloneFlag { Yes, No };
  SSATmp*   optimizeInst(IRInstruction* inst,
                         CloneFlag doClone,
                         Block* srcBlock,
                         const folly::Optional<IdomVector>& idoms);

private:
  void      appendInstruction(IRInstruction* inst);
  void      appendBlock(Block* block);
  void      insertPhis(bool forceSpPhi);

private:
  IRUnit& m_unit;
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
  jit::vector<BlockState> m_savedBlocks;
  Block* m_curBlock;
  folly::Optional<Block::iterator> m_curWhere;

  bool m_enableSimplification;
  bool m_constrainGuards;

  GuardConstraints m_constraints;

  // Keep track of blocks created to support bytecode control flow.
  //
  // TODO(t3730559): Offset is used here since it's passed from
  // emitJmp*, but SrcKey might be better in case of inlining.
  jit::flat_map<Offset,Block*> m_offsetToBlockMap;
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
template<bool pause>
struct BlockPusherImpl {
  BlockPusherImpl(IRBuilder& irb, BCMarker marker, Block* block,
              const folly::Optional<Block::iterator>& where = folly::none)
    : m_irb(irb)
  {
    irb.pushBlock(marker, block, where);
  }

  ~BlockPusherImpl() {
    m_irb.popBlock(pause);
  }

 private:
  IRBuilder& m_irb;
};

using BlockPusher = BlockPusherImpl<false>;
using BlockPauser = BlockPusherImpl<true>;

}}

#endif
