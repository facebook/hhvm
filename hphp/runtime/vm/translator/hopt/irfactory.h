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

#ifndef incl_HPHP_VM_IRFACTORY_H_
#define incl_HPHP_VM_IRFACTORY_H_

#include <type_traits>

#include "util/arena.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * ReturnValue makeInstruction(Lambda, Args...) --
 *
 *   Create an IRInstruction on the stack using Args, and call Lambda
 *   with a pointer to it, returning the result.
 *
 *   Normally IRInstruction creation should go through either
 *   IRFactory::gen or TraceBuilder::gen.  This utility is used to
 *   implement those.
 */

namespace detail {

template<class Ret, class Func>
struct InstructionBuilder {
  explicit InstructionBuilder(const Func& func) : func(func) {}

  template<class... Args>
  Ret go(Opcode op, Args... args) {
    return go2(op, Type::None, nullptr, args...);
  }

  template<class... Args>
  Ret go(Opcode op, Type::Tag t, Args... args) {
    return go2(op, t, nullptr, args...);
  }

  template<class... Args>
  Ret go(Opcode op, LabelInstruction* l, Args... args) {
    return go2(op, Type::None, l, args...);
  }

  template<class... Args>
  Ret go(Opcode op, Type::Tag t, LabelInstruction* l, Args... args) {
    return go2(op, t, l, args...);
  }

private:
  template<class Int>
  Ret go2(Opcode op, Type::Tag t, LabelInstruction* l, Int numArgs,
      SSATmp** tmps) {
    IRInstruction inst(op, numArgs, tmps);
    inst.setTypeParam(t);
    inst.setLabel(l);
    if (debug) assertOperandTypes(&inst);
    return func(&inst);
  }

  template<class... SSATmps>
  Ret go2(Opcode op, Type::Tag t, LabelInstruction* l, SSATmps... args) {
    SSATmp* tmps[] = { args... };
    return go2(op, t, l, sizeof...(args), tmps);
  }

private:
  const Func& func;
};

}

template<class Func, class... Args>
typename std::result_of<Func (IRInstruction*)>::type
makeInstruction(Func func, Args... args) {
  typedef typename std::result_of<Func (IRInstruction*)>::type Ret;
  return detail::InstructionBuilder<Ret,Func>(func).go(args...);
}

//////////////////////////////////////////////////////////////////////

/*
 * IRFactory is used for allocating and controlling the lifetime of IR
 * objects.
 */
class IRFactory {
public:
  IRFactory()
    : m_nextLabelId(0)
    , m_nextOpndId(0)
    , m_nextInstId(0)
  {}

  /*
   * Create an IRInstruction with lifetime equivalent to this IRFactory.
   *
   * Arguments are passed in the following format:
   *
   *   gen(Opcode, [type param], [exit label], [srcs ...]);
   *
   * All arguments are optional except the opcode.  `srcs' may be
   * specified either as a list of SSATmp*'s, or as a integer size and
   * a SSATmp**.
   */
  template<class... Args>
  IRInstruction* gen(Args... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) { return inst->clone(this); },
      args...
    );
  }

  template<class T>
  T* cloneInstruction(const T* inst) {
    T* newInst = new (m_arena) T(m_arena, inst,
                                 IRInstruction::IId(m_nextInstId++));
    newSSATmp(newInst);
    return newInst;
  }

  ConstInstruction* defConst(int64 val);
  LabelInstruction* defLabel(const Func*);
  LabelInstruction* defLabel(const Func*, unsigned numDst);
  MarkerInstruction* marker(uint32 bcOff, const Func* func, int32 spOff);
  /*
   * Creates move instrution that moves from src to dst. We can't use gen
   * to create such a move because gen assigns a newly allocated destination
   * SSATmp whereas we want to use the given dst SSATmp.
   */
  IRInstruction* mov(SSATmp* dst, SSATmp* src);

  Arena&   arena()               { return m_arena; }
  uint32_t numTmps() const       { return m_nextOpndId; }
  uint32_t numLabels() const     { return m_nextLabelId; }
  uint32_t numInsts() const      { return m_nextInstId; }

private:
  void newSSATmp(IRInstruction* inst) {
    if (!inst->hasDst()) return;
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
  }

  uint32_t m_nextLabelId;
  uint32_t m_nextOpndId;
  uint32_t m_nextInstId;

  // SSATmp and IRInstruction objects are allocated here.
  Arena m_arena;
};

//////////////////////////////////////////////////////////////////////

}}}

#endif
