/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Populates `imm' on `inst'.
 *
 * Assumes that inst.source and inst.unit have been properly set.
 */
static void populateImmediates(NormalizedInstruction& inst) {
  for (int i = 0; i < numImmediates(inst.op()); ++i) {
    inst.imm[i] = getImm(inst.pc(), i, inst.unit());
  }
  if (hasImmVector(inst.op())) {
    inst.immVec = getImmVector(inst.pc());
  }
}

///////////////////////////////////////////////////////////////////////////////

NormalizedInstruction::NormalizedInstruction(SrcKey sk, const Unit* u)
  : source(sk)
  , m_unit(u)
  , immVec()
  , interp(false)
  , forceSurpriseCheck(false)
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
  return peek_op(pc());
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
  return instrToString(pc(), unit());
}

SrcKey NormalizedInstruction::nextSk() const {
  return source.advanced(m_unit);
}

///////////////////////////////////////////////////////////////////////////////
}}
