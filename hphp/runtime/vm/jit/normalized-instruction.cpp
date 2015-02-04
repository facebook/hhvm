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

#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Populates `imm' on `inst'.
 *
 * Assumes that inst.source and inst.unit have been properly set.
 */
static void populateImmediates(NormalizedInstruction& inst) {
  auto offset = 1;
  for (int i = 0; i < numImmediates(inst.op()); ++i) {
    if (immType(inst.op(), i) == RATA) {
      auto rataPc = inst.pc() + offset;
      inst.imm[i].u_RATA = decodeRAT(inst.unit(), rataPc);
    } else {
      inst.imm[i] = getImm(reinterpret_cast<const Op*>(inst.pc()), i);
    }
    offset += immSize(reinterpret_cast<const Op*>(inst.pc()), i);
  }
  if (hasImmVector(*reinterpret_cast<const Op*>(inst.pc()))) {
    inst.immVec = getImmVector(reinterpret_cast<const Op*>(inst.pc()));
  }
  if (inst.op() == OpFCallArray) {
    inst.imm[0].u_IVA = 1;
  }
}

///////////////////////////////////////////////////////////////////////////////

NormalizedInstruction::NormalizedInstruction(SrcKey sk, const Unit* u)
  : source(sk)
  , funcd(nullptr)
  , m_unit(u)
  , immVec()
  , endsRegion(false)
  , nextIsMerge(false)
  , preppedByRef(false)
  , ignoreInnerType(false)
  , interp(false)
{
  memset(imm, 0, sizeof(imm));
  populateImmediates(*this);
}

NormalizedInstruction::NormalizedInstruction() { }

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

///////////////////////////////////////////////////////////////////////////////
}}
