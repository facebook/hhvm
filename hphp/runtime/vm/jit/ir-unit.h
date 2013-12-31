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

#ifndef incl_HPHP_VM_IRUNIT_H_
#define incl_HPHP_VM_IRUNIT_H_

#include <type_traits>
#include <vector>

#include "folly/ScopeGuard.h"
#include "hphp/util/arena.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {  namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * ReturnValue makeInstruction(Lambda, Args...) --
 *
 *   Create an IRInstruction on the stack using Args, and call Lambda
 *   with a pointer to it, returning the result.
 *
 *   Normally IRInstruction creation should go through either
 *   IRUnit::gen or TraceBuilder::gen.  This utility is used to
 *   implement those.  The lambda must not escape the IRInstruction*.
 */

namespace detail {

template<class Ret, class Func>
struct InstructionBuilder {
  explicit InstructionBuilder(const Func& func) : func(func) {}

  /*
   * Create an IRInstruction, and then recursively chew on the Args
   * list to populate its fields. Every instruction must have at least
   * an Opcode and a BCMarker.
   *
   * The IRInstruction is stack allocated, and should not escape the
   * lambda, so we fill it with 0xc0 in debug builds after we're done.
   */
  template<class... Args>
  Ret go(Opcode op, BCMarker marker, Args&&... args) {
    std::aligned_storage<sizeof(IRInstruction)>::type buffer;
    void* const vpBuffer = &buffer;
    SCOPE_EXIT { if (debug) memset(&buffer, 0xc0, sizeof buffer); };
    Edge edges[2];

    new (vpBuffer) IRInstruction(op, marker, hasEdges(op) ? edges : nullptr);
    auto const inst = static_cast<IRInstruction*>(vpBuffer);

    SCOPE_EXIT { inst->clearExtra(); };
    return go2(inst, std::forward<Args>(args)...);
  }

private:
  // Main loop: call setter() on the head of the list and repeat,
  // until there's no overload for the Head type; then it will fall
  // through to the base case.
  template<class Head, class... Tail>
  Ret go2(IRInstruction* inst, const Head& x, Tail&&... xs) {
    setter(inst, x);
    return go2(inst, std::forward<Tail>(xs)...);
  }

  // Base cases: either there are no SSATmps, or there's a variadic
  // list of SSATmp*'s, or there's an int followed by a SSATmp**.

  Ret go2(IRInstruction* inst) { return stop(inst); }

  template<class... SSAs>
  Ret go2(IRInstruction* inst, SSATmp* t1, SSAs... ts) {
    SSATmp* ssas[] = { t1, ts... };
    auto const nSrcs = 1 + sizeof...(ts);
    for (unsigned i = 0; debug && i < nSrcs; ++i) assert(ssas[i]);

    inst->initializeSrcs(nSrcs, ssas);
    return stop(inst);
  }

  // For each of the optional parameters, there's an overloaded
  // setter:

  void setter(IRInstruction* inst, std::pair<uint32_t,SSATmp**> ssas) {
    inst->initializeSrcs(ssas.first, ssas.second);
  }

  void setter(IRInstruction* inst, Block* target) {
    inst->setTaken(target);
  }

  void setter(IRInstruction* inst, Type t) {
    inst->setTypeParam(t);
  }

  template<typename T>
  typename std::enable_if<std::is_base_of<IRExtraData,T>::value, void>::type
  setter(IRInstruction* inst, const T& extra) {
    /*
     * Taking the address of this temporary seems scary, but actually
     * it is safe: `extra' was forwarded in all the way from the
     * makeInstruction call, but then we bound a const reference to it
     * at go2() when it was the head of the varargs list, so it must
     * last until the end of the full-expression that called that
     * go2().
     *
     * This is long enough for it to outlast our call to func,
     * although the transient IRInstruction actually will outlive it.
     * We null out the extra data in go() before ~IRInstruction runs,
     * though.
     */
    assert_opcode_extra_same<T>(inst->op());
    const IRExtraData* dataPtr = &extra;
    inst->setExtra(const_cast<IRExtraData*>(dataPtr));
  }

  // Finally we end up here.
  Ret stop(IRInstruction* inst) {
    if (debug) assertOperandTypes(inst);
    return func(inst);
  }

private:
  const Func& func;
};

}

template<class Func, class... Args>
typename std::result_of<Func (IRInstruction*)>::type
makeInstruction(Func func, Args&&... args) {
  typedef typename std::result_of<Func (IRInstruction*)>::type Ret;
  return detail::InstructionBuilder<Ret,Func>(func).go(args...);
}

//////////////////////////////////////////////////////////////////////

struct IRTrace;

/*
 * IRUnit is the compilation unit for the JIT.  It owns an Arena used
 * for allocating and controlling the lifetime of Block, IRInstruction,
 * ExtraData, SSATmp, and IRTrace objects, as well as a constant table
 * containing all DefConst instructions, which don't live in Blocks.
 *
 * IRUnit also assigns unique ids to each block, instruction, and tmp,
 * which are useful for StateVector or sparse maps of pass specific
 * information.
 */
class IRUnit {
  TRACE_SET_MOD(hhir);

public:
  explicit IRUnit(Offset initialBcOffset);

  /*
   * Create an IRInstruction with lifetime equivalent to this IRUnit.
   *
   * Arguments are passed in the following format:
   *
   *   gen(Opcode, [IRExtraData&,] [type param,] [exit label,] [srcs ...]);
   *
   * All arguments are optional except the opcode.  `srcs' may be
   * specified either as a list of SSATmp*'s, or as a integer size and
   * a SSATmp**.
   */
  template<class... Args>
  IRInstruction* gen(Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) { return cloneInstruction(inst); },
      std::forward<Args>(args)...
    );
  }

  /*
   * Replace an existing IRInstruction with a new one.
   *
   * This may involve making more allocations in the arena, but the
   * actual IRInstruction* itself, its Id, etc, will stay unchanged.
   *
   * This function takes arguments in the same format as gen().
   */
  template<class... Args>
  void replace(IRInstruction* old, Opcode op, Args... args) {
    makeInstruction(
      [&] (IRInstruction* replacement) { old->become(*this, replacement); },
      op,
      old->marker(),
      std::forward<Args>(args)...
    );
  }

  /*
   * Deep-copy an IRInstruction and its src/dests.
   */
  IRInstruction* cloneInstruction(const IRInstruction* inst) {
    auto newInst = new (m_arena) IRInstruction(
      m_arena, inst, IRInstruction::Id(m_nextInstId++));
    if (newInst->modifiesStack()) {
      assert(newInst->naryDst());
      // The instruction is an opcode that modifies the stack, returning a new
      // StkPtr.
      int numDsts = 1 + (newInst->hasMainDst() ? 1 : 0);
      SSATmp* dsts = (SSATmp*)m_arena.alloc(numDsts * sizeof(SSATmp));
      for (int dstNo = 0; dstNo < numDsts; ++dstNo) {
        new (&dsts[dstNo]) SSATmp(m_nextOpndId++, newInst, dstNo);
      }
      newInst->setDsts(numDsts, dsts);
    } else {
      newSSATmp(newInst);
    }
    FTRACE(5, "cloned {}\n", *inst);
    return newInst;
  }

  /*
   * Some helpers for creating specific instruction patterns.
   */
  IRInstruction* defLabel(unsigned numDst, BCMarker marker);
  Block* defBlock();

  template<typename T> SSATmp* cns(T val) {
    return cns(val, typeForConst(val));
  }

  template<typename T> SSATmp* cns(T val, Type type) {
    ConstData cdata(val);
    return findConst(cdata, type);
  }

  SSATmp* cns(Type type) {
    ConstData cdata(0);
    return findConst(cdata, type);
  }

  /*
   * Creates move instrution that moves from src to dst. We can't use gen
   * to create such a move because gen assigns a newly allocated destination
   * SSATmp whereas we want to use the given dst SSATmp.
   */
  IRInstruction* mov(SSATmp* dst, SSATmp* src, BCMarker marker);

  /*
   * Create a new exit trace.
   */
  Block* addExit();

  Arena&   arena()               { return m_arena; }
  uint32_t numTmps() const       { return m_nextOpndId; }
  uint32_t numBlocks() const     { return m_nextBlockId; }
  uint32_t numInsts() const      { return m_nextInstId; }
  CSEHash& constTable()          { return m_constTable; }
  IRTrace* main()                { return m_main; }
  const IRTrace* main() const    { return m_main; }
  uint32_t bcOff() const         { return m_bcOff; }

  // Overloads useful for StateVector and IdSet
  uint32_t numIds(const SSATmp*) const { return numTmps(); }
  uint32_t numIds(const Block*) const { return numBlocks(); }
  uint32_t numIds(const IRInstruction*) const { return numInsts(); }

  typedef smart::vector<IRTrace*> ExitList;
  ExitList& exits()             { return m_exits; }
  const ExitList& exits() const { return m_exits; }
  Block* entry() const;
  std::string toString() const;

private:
  SSATmp* findConst(ConstData& cdata, Type t);
  void newSSATmp(IRInstruction* inst) {
    if (!inst->hasDst()) return;
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
  }

private:
  Arena m_arena; // contains IRTrace, Block, IRInstruction, and SSATmp objects
  CSEHash m_constTable; // DefConst's for each unique constant in this IR
  uint32_t m_nextBlockId;
  uint32_t m_nextOpndId;
  uint32_t m_nextInstId;
  uint32_t m_bcOff; // bytecode offset where this unit starts
  IRTrace* m_main; // main entry point trace
  ExitList m_exits; // exit traces
};

//////////////////////////////////////////////////////////////////////

}}

#endif
