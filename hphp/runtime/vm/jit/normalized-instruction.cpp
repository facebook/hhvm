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
#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP {
namespace JIT {

NormalizedInstruction::NormalizedInstruction(SrcKey sk, const Unit* u)
    : source(sk)
    , funcd(nullptr)
    , m_unit(u)
    , outPred(Type::Gen)
    , immVec()
    , nextOffset(kInvalidOffset)
    , breaksTracelet(false)
    , includeBothPaths(false)
    , nextIsMerge(false)
    , changesPC(false)
    , preppedByRef(false)
    , outputPredicted(false)
    , ignoreInnerType(false)
    , noOp(false)
    , interp(false)
    , inlineReturn(false) {
  memset(imm, 0, sizeof(imm));
}

NormalizedInstruction::NormalizedInstruction()
    : NormalizedInstruction(SrcKey{}, nullptr)
  {}

NormalizedInstruction::~NormalizedInstruction() { }

/*
 *   Helpers for recovering context of this instruction.
 */
Op NormalizedInstruction::op() const {
  return *reinterpret_cast<const Op*>(pc());
}

Op NormalizedInstruction::mInstrOp() const {
  auto const opcode = op();
#define MII(instr, a, b, i, v, d) case Op##instr##M: return opcode;
  switch (opcode) {
    MINSTRS
  case Op::FPassM:
    return preppedByRef ? Op::VGetM : Op::CGetM;
  default:
    not_reached();
  }
#undef MII
}

PC NormalizedInstruction::pc() const {
  return unit()->at(source.offset());
}

const Unit* NormalizedInstruction::unit() const {
  return m_unit;
}

const Func* NormalizedInstruction::func() const {
  return source.func();
}

Offset NormalizedInstruction::offset() const {
  return source.offset();
}

std::string NormalizedInstruction::toString() const {
  return instrToString((Op*)pc(), unit());
}

SrcKey NormalizedInstruction::nextSk() const {
  return source.advanced(m_unit);
}

} } // HPHP::JIT
