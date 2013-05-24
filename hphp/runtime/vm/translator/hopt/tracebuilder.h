/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/irfactory.h"
#include "hphp/runtime/vm/translator/hopt/cse.h"
#include "hphp/runtime/vm/translator/hopt/simplifier.h"
#include "hphp/runtime/vm/translator/hopt/state_vector.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * This module provides the basic utilities for generating the IR
 * instructions in a trace, emitting control flow, tracking the state
 * of locals, and managing how state should merge at control flow join
 * points.  It also performs some optimizations while generating IR,
 * and may be reinvoked for a second optimization pass.
 *
 *
 * The types of state tracked by TraceBuilder include:
 *
 *   - value availability
 *
 *      Used for value propagation and tracking which values can be
 *      CSE'd (value numbering below).
 *
 *   - local types and values
 *
 *      We track the current view of these types as we link in new
 *      instructions that mutate these.  The state of the stack is
 *      encoded in the IR via the StkPtr chain instead.
 *
 *   - current frame and stack pointers
 *
 *   - the current function and bytecode offset
 *
 *
 * This module is also responsible for organizing a few types of
 * gen-time optimizations:
 *
 *   - preOptimize pass
 *
 *      Before an instruction is linked into the trace, TraceBuilder
 *      internally runs preOptimize() on it, which can do some
 *      tracelet-state related modifications to the instruction.  For
 *      example, it can eliminate redundant guards or weaken DecRef
 *      instructions that cannot go to zero to DecRefNZ.
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
 * After all the instructions are linked into the trace, this module
 * can also be used to perform a second round of the above two
 * optimizations via the reoptimize() entry point.
 */
struct TraceBuilder {
  TraceBuilder(Offset initialBcOffset,
               Offset initialSpOffsetFromFp,
               IRFactory&,
               const Func* func);
  ~TraceBuilder();

  void setEnableCse(bool val)            { m_enableCse = val; }
  void setEnableSimplification(bool val) { m_enableSimplification = val; }

  Trace* getTrace() const { return m_trace.get(); }
  IRFactory* getIrFactory() { return &m_irFactory; }
  int32_t getSpOffset() { return m_spOffset; }
  SSATmp* getSp() const { return m_spValue; }
  SSATmp* getFp() const { return m_fpValue; }

  bool isThisAvailable() const {
    return m_thisIsAvailable;
  }
  void setThisAvailable() {
    m_thisIsAvailable = true;
  }

  /*
   * Run another pass of TraceBuilder-managed optimizations on this
   * trace.
   */
  void reoptimize();

  /*
   * Create an IRInstruction attached to the current main Trace, and
   * allocate a destination SSATmp for it.  Uses the same argument
   * list format as IRFactory::gen.
   */
  template<class... Args>
  SSATmp* gen(Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) { return optimizeInst(inst); },
      std::forward<Args>(args)...
    );
  }

  /*
   * Create an IRInstruction, similar to gen(), except link it into
   * the Trace t instead of the current main trace.
   *
   * Also does not run optimization passes.
   *
   * TODO(#2404447): run simplifier?
   */
  template<class... Args>
  SSATmp* genFor(Trace* t, Args... args) {
    auto instr = m_irFactory.gen(args...);
    t->back()->push_back(instr);
    return instr->getDst();
  }

  //////////////////////////////////////////////////////////////////////
  // constants

  SSATmp* genDefUninit();
  SSATmp* genDefInitNull();
  SSATmp* genDefNull();
  SSATmp* genPtrToInitNull();
  SSATmp* genPtrToUninit();
  SSATmp* genDefNone();

  template<typename T>
  SSATmp* cns(T val) {
    return gen(DefConst, typeForConst(val), ConstData(val));
  }

  template<typename T>
  SSATmp* cns(T val, Type type) {
    return gen(DefConst, type, ConstData(val));
  }

  SSATmp* cns(Type type) {
    return gen(DefConst, type, ConstData(0));
  }

  template<typename T>
  SSATmp* genLdConst(T val) {
    return gen(LdConst, typeForConst(val), ConstData(val));
  }

  //////////////////////////////////////////////////////////////////////
  // control flow

  // hint the execution frequency of the current block
  void hint(Block::Hint h) const {
    m_trace->back()->setHint(h);
  }

  /*
   * cond() generates if-then-else blocks within a trace.  The caller
   * supplies lambdas to create the branch, next-body, and taken-body.
   * The next and taken lambdas must return one SSATmp* value; cond() returns
   * the SSATmp for the merged value.
   */
  template <class Branch, class Next, class Taken>
  SSATmp* cond(const Func* func, Branch branch, Next next, Taken taken) {
    Block* taken_block = m_irFactory.defBlock(func);
    Block* done_block = m_irFactory.defBlock(func, 1);
    DisableCseGuard guard(*this);
    branch(taken_block);
    SSATmp* v1 = next();
    gen(Jmp_, done_block, EdgeData(), v1);
    appendBlock(taken_block);
    SSATmp* v2 = taken();
    gen(Jmp_, done_block, EdgeData(), v2);
    appendBlock(done_block);
    SSATmp* result = done_block->getLabel()->getDst(0);
    result->setType(Type::unionOf(v1->type(), v2->type()));
    return result;
  }

  /*
   * ifThen generates if-then blocks within a trace that do not
   * produce values. Code emitted in the taken lambda will be executed
   * iff the branch emitted in the branch lambda is taken.
   */
  template <class Branch, class Taken>
  void ifThen(const Func* func, Branch branch, Taken taken) {
    Block* taken_block = m_irFactory.defBlock(func);
    Block* done_block = m_irFactory.defBlock(func);
    DisableCseGuard guard(*this);
    branch(taken_block);
    assert(!m_trace->back()->getNext());
    m_trace->back()->setNext(done_block);
    appendBlock(taken_block);
    taken();
    taken_block->setNext(done_block);
    appendBlock(done_block);
  }

  /*
   * ifElse generates if-then-else blocks with an empty 'then' block
   * that do not produce values. Code emitted in the next lambda will
   * be executed iff the branch emitted in the branch lambda is not
   * taken.
   */
  template <class Branch, class Next>
  void ifElse(const Func* func, Branch branch, Next next) {
    Block* done_block = m_irFactory.defBlock(func);
    DisableCseGuard guard(*this);
    branch(done_block);
    next();
    m_trace->back()->setNext(done_block);
    appendBlock(done_block);
  }

  /*
   * Create a new "exit trace".  This is a Trace that is assumed to be
   * a cold path, which always exits the tracelet without control flow
   * rejoining the main line.
   */
  Trace* makeExitTrace(uint32_t bcOff) {
    return m_trace->addExitTrace(makeTrace(m_curFunc->getValFunc(),
                                           bcOff));
  }

private:
  // RAII disable of CSE; only restores if it used to be on.  Used for
  // control flow, where we currently don't allow CSE.
  struct DisableCseGuard {
    explicit DisableCseGuard(TraceBuilder& tb)
      : m_tb(tb)
      , m_oldEnable(tb.m_enableCse)
    {
        m_tb.m_enableCse = false;
    }
    ~DisableCseGuard() {
      m_tb.m_enableCse = m_oldEnable;
    }

   private:
    TraceBuilder& m_tb;
    bool m_oldEnable;
  };

  // Saved state information associated with the start of a block, or
  // for the caller of an inlined function.
  struct State {
    SSATmp* spValue;
    SSATmp* fpValue;
    SSATmp* curFunc;
    int32_t spOffset;
    bool thisAvailable;
    std::vector<SSATmp*> localValues;
    std::vector<Type> localTypes;
    SSATmp* refCountedMemValue;
    std::vector<SSATmp*> callerAvailableValues; // unordered list
    MarkerData lastMarker;
  };

private:
  SSATmp*   preOptimizeCheckLoc(IRInstruction*);
  SSATmp*   preOptimizeAssertLoc(IRInstruction*);
  SSATmp*   preOptimizeLdThis(IRInstruction*);
  SSATmp*   preOptimizeLdCtx(IRInstruction*);
  SSATmp*   preOptimizeDecRef(IRInstruction*);
  SSATmp*   preOptimizeDecRefThis(IRInstruction*);
  SSATmp*   preOptimizeDecRefLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLoc(IRInstruction*);
  SSATmp*   preOptimizeLdLocAddr(IRInstruction*);
  SSATmp*   preOptimizeStLoc(IRInstruction*);

  SSATmp*   preOptimize(IRInstruction* inst);
  SSATmp*   optimizeWork(IRInstruction* inst);
  SSATmp*   optimizeInst(IRInstruction* inst);

private:
  static void appendInstruction(IRInstruction* inst, Block* block);
  void      appendInstruction(IRInstruction* inst);
  void      appendBlock(Block* block);
  enum      CloneInstMode { kCloneInst, kUseInst };
  SSATmp*   cseLookup(IRInstruction* inst);
  void      cseInsert(IRInstruction* inst);
  void      cseKill(SSATmp* src);
  CSEHash*  getCSEHashTable(IRInstruction* inst);
  void      killCse();
  void      killLocals();
  void      killLocalValue(uint32_t id);
  void      setLocalType(uint32_t id, Type type);
  Type      getLocalType(unsigned id) const;
  void      setLocalValue(unsigned id, SSATmp* value);
  SSATmp*   getLocalValue(unsigned id) const;
  bool      isValueAvailable(SSATmp*) const;
  bool      anyLocalHasValue(SSATmp*) const;
  bool      callerHasValueAvailable(SSATmp*) const;
  void      updateLocalRefValues(SSATmp* oldRef, SSATmp* newRef);
  void      trackDefInlineFP(IRInstruction* inst);
  void      trackInlineReturn(IRInstruction* inst);
  void      updateTrackedState(IRInstruction* inst);
  void      clearTrackedState();
  void      dropLocalRefsInnerTypes();

  Trace* makeTrace(const Func* func, uint32_t bcOff) {
    return new Trace(m_irFactory.defBlock(func), bcOff);
  }

private:
  std::unique_ptr<State> createState() const;
  void saveState(Block*);
  void mergeState(State* s1);
  void useState(std::unique_ptr<State> state);
  void useState(Block*);

private:
  IRFactory& m_irFactory;
  Simplifier m_simplifier;

  boost::scoped_ptr<Trace> const m_trace; // generated trace

  // Flags that enable optimizations
  bool       m_enableCse;
  bool       m_enableSimplification;

  // Snapshots of state at the beginning of blocks we haven't reached yet.
  StateVector<Block,State*> m_snapshots;

  /*
   * While building a trace one instruction at a time, a TraceBuilder
   * tracks various state for generating code and for optimization:
   *
   *   (1) m_fpValue & m_spValue track which SSATmp holds the current VM
   *       frame and stack pointer values.
   *
   *   (2) m_spOffset tracks the offset of the m_spValue from m_fpValue.
   *
   *   (3) m_curFunc tracks the current function containing the
   *       generated code.
   *
   *   (4) m_cseHash is for common sub-expression elimination of non-constants.
   *       constants are globally available and managed by IRFactory.
   *
   *   (5) m_thisIsAvailable tracks whether the current ActRec has a
   *       non-null this pointer.
   *
   *   (6) m_localValues & m_localTypes track the current values and
   *       types held in locals. These vectors are indexed by the
   *       local's id.
   *
   * The function updateTrackedState(IRInstruction* inst) updates this
   * state (called after an instruction is appended to the trace), and
   * the function clearTrackedState() clears it.
   *
   * After branch instructions, updateTrackedState() creates an instance
   * of State holding snapshots of these fields (except m_curFunc and
   * m_constTable), optionally merges with the State already saved at
   * the branch target, and saves it.  Then, before generating code for
   * the branch target, useState() restores the saved (and merged) state.
   */
  SSATmp*    m_spValue;      // current physical sp
  SSATmp*    m_fpValue;      // current physical fp
  int32_t    m_spOffset;     // offset of physical sp from physical fp
  SSATmp*    m_curFunc;      // current function context
  CSEHash    m_cseHash;
  bool       m_thisIsAvailable; // true only if current ActRec has non-null this

  // state of values in memory
  SSATmp*    m_refCountedMemValue;

  // vectors that track local values & types
  std::vector<SSATmp*> m_localValues;
  std::vector<Type>    m_localTypes;

  // Values known to be "available" for the purposes of DecRef to
  // DecRefNZ transformations due to locals of the caller for an
  // inlined call.
  std::vector<SSATmp*> m_callerAvailableValues;

  // The data for the last Marker instruction we've seen.
  folly::Optional<MarkerData> m_lastMarker;

  // When we're building traces for an inlined callee, the state of
  // the caller needs to be preserved here.
  std::vector<std::unique_ptr<State>> m_inlineSavedStates;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
