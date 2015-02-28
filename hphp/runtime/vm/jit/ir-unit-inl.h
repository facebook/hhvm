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

#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/trace.h"

#include <folly/ScopeGuard.h>

#include <type_traits>
#include <utility>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

namespace irunit_detail {
///////////////////////////////////////////////////////////////////////////////

template<class Ret, class Func>
struct InstructionBuilder {
  explicit InstructionBuilder(const Func& func) : func(func) {}

  /*
   * Create an IRInstruction, and then recursively chew on the Args list to
   * populate its fields.  Every instruction must have at least an Opcode and a
   * BCMarker.
   *
   * The IRInstruction is stack allocated, and should not escape the lambda, so
   * we fill it with 0xc0 in debug builds after we're done.
   */
  template<class... Args>
  Ret go(Opcode op, BCMarker marker, Args&&... args) {
    std::aligned_storage<sizeof(IRInstruction)>::type buffer;
    void* const vpBuffer = &buffer;
    SCOPE_EXIT { if (debug) memset(&buffer, 0xc0, sizeof(buffer)); };
    Edge edges[2];

    new (vpBuffer) IRInstruction(op, marker, hasEdges(op) ? edges : nullptr);
    auto const inst = static_cast<IRInstruction*>(vpBuffer);

    SCOPE_EXIT { inst->clearExtra(); };
    return go2(inst, std::forward<Args>(args)...);
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  /*
   * Main loop: call setter() on the head of the list and repeat, until there's
   * no overload for the Head type; then fall through to the base case.
   */
  template<class Head, class... Tail>
  Ret go2(IRInstruction* inst, const Head& x, Tail&&... xs) {
    setter(inst, x);
    return go2(inst, std::forward<Tail>(xs)...);
  }

  /*
   * Base case: no SSATmps.
   */
  Ret go2(IRInstruction* inst) { return stop(inst); }

  /*
   * Base case: variadic list of SSATmp*'s.
   */
  template<class... SSAs>
  Ret go2(IRInstruction* inst, SSATmp* t1, SSAs... ts) {
    SSATmp* ssas[] = { t1, ts... };
    auto const nSrcs = 1 + sizeof...(ts);
    for (unsigned i = 0; debug && i < nSrcs; ++i) assert(ssas[i]);

    inst->initializeSrcs(nSrcs, ssas);
    return stop(inst);
  }

  /*
   * Base case: (size, SSATmp**) array of srcs.
   */
  Ret go2(IRInstruction* inst, std::pair<size_t,SSATmp**> ssas) {
    inst->initializeSrcs(ssas.first, ssas.second);
    return stop(inst);
  }

  /*
   * Call the lambda on the initialized IRInstruction.
   */
  Ret stop(IRInstruction* inst) {
    assert(checkOperandTypes(inst));
    return func(inst);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Overloaded setters.

  /*
   * Setters for type param.
   */
  void setter(IRInstruction* inst, Type t) {
    inst->setTypeParam(t);
  }
  void setter(IRInstruction* inst, folly::Optional<Type> t) {
    if (t.hasValue()) {
      inst->setTypeParam(t.value());
    }
  }

  /*
   * Setter for exit label.
   */
  void setter(IRInstruction* inst, Block* target) {
    assert(!target || inst->hasEdges());
    inst->setTaken(target);
  }

  /*
   * Setter for IRExtraData classes.
   */
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

private:
  const Func& func;
};

///////////////////////////////////////////////////////////////////////////////
}

template<class Func, class... Args>
typename std::result_of<Func(IRInstruction*)>::type
makeInstruction(Func func, Args&&... args) {
  typedef typename std::result_of<Func(IRInstruction*)>::type Ret;
  return irunit_detail::InstructionBuilder<Ret,Func>(func).go(args...);
}

///////////////////////////////////////////////////////////////////////////////

template<class... Args>
IRInstruction* IRUnit::gen(Args&&... args) {
  return makeInstruction(
    [this] (IRInstruction* inst) { return clone(inst); },
    std::forward<Args>(args)...
  );
}

template<class... Args>
IRInstruction* IRUnit::gen(SSATmp* dst, Args&&... args) {
  return makeInstruction(
    [this, dst] (IRInstruction* inst) { return clone(inst, dst); },
    std::forward<Args>(args)...
  );
}

template<class... Args>
void IRUnit::replace(IRInstruction* old, Opcode op, Args... args) {
  makeInstruction(
    [&] (IRInstruction* replacement) { old->become(*this, replacement); },
    op,
    old->marker(),
    std::forward<Args>(args)...
  );
}

inline IRInstruction* IRUnit::clone(const IRInstruction* old,
                                    SSATmp* dst /* = nullptr */) {
  auto inst = new (m_arena) IRInstruction(
    m_arena, old, IRInstruction::Id(m_nextInstId++));

  if (dst) {
    dst->setInstruction(inst);
    inst->setDst(dst);
  } else if (inst->hasDst()) {
    dst = new (m_arena) SSATmp(m_nextTmpId++, inst);
    inst->setDst(dst);
  }

  FTRACE_MOD(Trace::hhir, 5, "cloned {}\n", *old);
  return inst;
}

///////////////////////////////////////////////////////////////////////////////

inline Arena& IRUnit::arena() {
  return m_arena;
}

inline CSEHash& IRUnit::constTable() {
  return m_constTable;
}

inline const TransContext& IRUnit::context() const {
  return m_context;
}

inline Block* IRUnit::entry() const {
  return m_entry;
}

inline uint32_t IRUnit::bcOff() const {
  return m_context.initBcOffset;
}

inline SrcKey IRUnit::initSrcKey() const {
  return m_context.srcKey();
}

inline uint32_t IRUnit::numTmps() const {
  return m_nextTmpId;
}

inline uint32_t IRUnit::numBlocks() const {
  return m_nextBlockId;
}

inline uint32_t IRUnit::numInsts() const {
  return m_nextInstId;
}

inline uint32_t IRUnit::numIds(const SSATmp*) const {
  return numTmps();
}

inline uint32_t IRUnit::numIds(const Block*) const {
  return numBlocks();
}

inline uint32_t IRUnit::numIds(const IRInstruction*) const {
  return numInsts();
}


///////////////////////////////////////////////////////////////////////////////

template<typename T> SSATmp* IRUnit::cns(T val) {
  return cns(Type::cns(val));
}

inline const LabelRefs& IRUnit::labelRefs() const {
  return m_labelRefs;
}

inline const PostConditions& IRUnit::postConditions() const {
  return m_postConds;
}

///////////////////////////////////////////////////////////////////////////////
}}
