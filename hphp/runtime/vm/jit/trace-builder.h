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

#ifndef incl_HPHP_HHVM_HHIR_TRACEBUILDER_H_
#define incl_HPHP_HHVM_HHIR_TRACEBUILDER_H_

#include <boost/scoped_ptr.hpp>

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
 *      Before an instruction is linked into the trace, TraceBuilder
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
 *      After the preOptimize pass, TraceBuilder calls out to
 *      Simplifier to perform state-independent optimizations, like
 *      copy propagation and strength reduction.  (See simplifier.h.)
 *
 *
 * After all the instructions are linked into the trace, this module can also
 * be used to perform a second round of the above two optimizations via the
 * reoptimize() entry point.
 */
struct TraceBuilder {
  TraceBuilder(Offset initialBcOffset,
               Offset initialSpOffsetFromFp,
               IRUnit&,
               const Func* func);
  ~TraceBuilder();

  void setEnableSimplification(bool val) { m_enableSimplification = val; }
  bool inReoptimize() const              { return m_inReoptimize; }
  bool typeMightRelax(SSATmp* val = nullptr) const;
  bool shouldElideAssertType(Type oldType, Type newType, SSATmp* oldVal) const;

  IRTrace* trace() const { return m_curTrace; }
  IRUnit& unit() { return m_unit; }
  FrameState& state() { return m_state; }
  const Func* curFunc() const { return m_state.func(); }
  int32_t spOffset() { return m_state.spOffset(); }
  SSATmp* sp() const { return m_state.sp(); }
  SSATmp* fp() const { return m_state.fp(); }
  const GuardConstraints* guards() const { return &m_guardConstraints; }

  bool thisAvailable() const { return m_state.thisAvailable(); }
  void setThisAvailable() { m_state.setThisAvailable(); }

  bool shouldConstrainGuards() const;
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
  SSATmp* localValueSource(uint32_t id) const {
    return m_state.localValueSource(id);
  }
  bool inlinedFrameSpansCall() const { return m_state.frameSpansCall(); }

  /*
   * Updates the marker used for instructions generated without one
   * supplied.
   */
  void setMarker(BCMarker marker);

  /*
   * To emit code to a trace other than the main trace, call pushTrace(), emit
   * instructions as usual with gen(...), then, call popTrace(). This is best
   * done using the TracePusher struct:
   *
   * gen(CodeForMainTrace, ...);
   * {
   *   TracePusher tp(m_tb, exitTrace, marker);
   *   gen(CodeForExitTrace, ...);
   * }
   * gen(CodeForMainTrace, ...);
   *
   * b and where may be supplied to emit code to a specific location in the
   * trace. b must be nullptr iff where is boost::none, and where (if present)
   * must be an iterator to somewhere in b's InstructionList. Instructions will
   * be inserted at where, before the instruction it currently points to.
   */
  void pushTrace(IRTrace* t, BCMarker marker, Block* b,
                 const boost::optional<Block::iterator>& where);
  void popTrace();

  /*
   * Run another pass of TraceBuilder-managed optimizations on this
   * trace.
   */
  void reoptimize();

  /*
   * Create an IRInstruction attached to the current IRTrace, and
   * allocate a destination SSATmp for it.  Uses the same argument
   * list format as IRUnit::gen.
   */
  template<class... Args>
  SSATmp* gen(Opcode op, Args&&... args) {
    return gen(op, m_state.marker(), std::forward<Args>(args)...);
  }

  template<class... Args>
  SSATmp* gen(Opcode op, BCMarker marker, Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) {
        return optimizeInst(inst, CloneFlag::Yes);
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
    return optimizeInst(inst, CloneFlag::No);
  }

  //////////////////////////////////////////////////////////////////////
  // constants

  SSATmp* genDefUninit();
  SSATmp* genDefInitNull();
  SSATmp* genDefNull();
  SSATmp* genPtrToInitNull();
  SSATmp* genPtrToUninit();
  SSATmp* genDefNone();

  template<class... Args>
  SSATmp* cns(Args&&... args) {
    return m_unit.cns(std::forward<Args>(args)...);
  }

  template<typename T>
  SSATmp* genLdConst(T val) {
    return gen(LdConst, typeForConst(val), ConstData(val));
  }

  //////////////////////////////////////////////////////////////////////
  // control flow

  // hint the execution frequency of the current block
  void hint(Block::Hint h) const {
    m_curTrace->back()->setHint(h);
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
    Block* last = m_curTrace->back();
    assert(!last->next());
    last->back().setNext(done_block);
    appendBlock(taken_block);
    taken();
    // patch the last block added by the Taken lambda to jump to
    // the done block.  Note that last might not be taken_block.
    last = m_curTrace->back();
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
    auto last = m_curTrace->back();
    if (last->empty() || !last->back().isBlockEnd()) {
      gen(Jmp, done_block);
    } else {
      last->back().setNext(done_block);
    }
    appendBlock(done_block);
  }

  /*
   * Create a new "exit trace".  This is a Trace that is assumed to be
   * a cold path, which always exits the tracelet without control flow
   * rejoining the main line.
   */
  Block* makeExit() { return m_unit.addExit(); }

  /*
   * Get all typed locations in current translation.
   */
  std::vector<RegionDesc::TypePred> getKnownTypes() const;

private:
  // RAII disable of CSE; only restores if it used to be on.  Used for
  // control flow, where we currently don't allow CSE.
  struct DisableCseGuard {
    explicit DisableCseGuard(TraceBuilder& tb)
      : m_state(tb.m_state)
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
  SSATmp*   preOptimizeCheckLoc(IRInstruction*);
  SSATmp*   preOptimizeAssertLoc(IRInstruction*);
  SSATmp*   preOptimizeLdThis(IRInstruction*);
  SSATmp*   preOptimizeLdCtx(IRInstruction*);
  SSATmp*   preOptimizeDecRefThis(IRInstruction*);
  SSATmp*   preOptimizeDecRefLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLocAddr(IRInstruction*);
  SSATmp*   preOptimizeStLoc(IRInstruction*);
  SSATmp*   preOptimize(IRInstruction* inst);

  SSATmp*   optimizeWork(IRInstruction* inst,
                         const folly::Optional<IdomVector>&);

  enum class CloneFlag { Yes, No };
  SSATmp*   optimizeInst(IRInstruction* inst, CloneFlag doclone);

private:
  void      appendInstruction(IRInstruction* inst, Block* block);
  void      appendInstruction(IRInstruction* inst);
  void      appendBlock(Block* block);
  enum      CloneInstMode { kCloneInst, kUseInst };

private:
  IRUnit& m_unit;
  Simplifier m_simplifier;
  FrameState m_state;

  /*
   * m_savedTraces will be nonempty iff we're emitting code to a trace other
   * than the main trace. m_curTrace, m_curMarker, m_curBlock, m_curWhere are
   * all set from the most recent call to pushTrace() or popTrace().
   */
  struct TraceState {
    IRTrace* trace;
    Block* block;
    BCMarker marker;
    boost::optional<Block::iterator> where;
  };
  smart::stack<TraceState> m_savedTraces;
  IRTrace* m_curTrace;
  Block* m_curBlock;
  boost::optional<Block::iterator> m_curWhere;

  bool       m_enableSimplification;
  bool       m_inReoptimize;

  GuardConstraints m_guardConstraints;

};

/*
 * BranchImpl is used by TraceBuilder::cond to support branch and next lambdas
 * with different signatures. See cond for details.
 */
template<> struct TraceBuilder::BranchImpl<void> {
  template<typename Branch, typename Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    branch(taken);
    return next();
  }
};

template<> struct TraceBuilder::BranchImpl<SSATmp*> {
  template<typename Branch, typename Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    return next(branch(taken));
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * RAII helper for emitting code to exit traces. See TraceBuilder::pushTrace
 * for usage.
 */
struct TracePusher {
  template<typename... Args>
  TracePusher(TraceBuilder& tb, IRTrace* trace, BCMarker marker,
              Block* block = nullptr,
              const boost::optional<Block::iterator>& where =
                boost::optional<Block::iterator>())
    : m_tb(tb)
  {
    tb.pushTrace(trace, marker, block, where);
  }

  ~TracePusher() {
    m_tb.popTrace();
  }

 private:
  TraceBuilder& m_tb;
};

}}

#endif
