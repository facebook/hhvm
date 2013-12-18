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
#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/tracelet.h"
#include "hphp/runtime/vm/jit/translator.h"

namespace HPHP {
namespace JIT {

NormalizedInstruction::NormalizedInstruction()
    : next(nullptr)
    , prev(nullptr)
    , funcd(nullptr)
    , outStack(nullptr)
    , outLocal(nullptr)
    , outLocal2(nullptr)
    , outStack2(nullptr)
    , outStack3(nullptr)
    , outPred(Type::Gen)
    , checkedInputs(0)
    , outputPredicted(false)
    , outputPredictionStatic(false)
    , ignoreInnerType(false)
    , guardedThis(false)
    , guardedCls(false)
    , noOp(false)
    , interp(false)
    , inlineReturn(false) {
  memset(imm, 0, sizeof(imm));
}

NormalizedInstruction::~NormalizedInstruction() { }

/*
 *   Helpers for recovering context of this instruction.
 */
Op NormalizedInstruction::op() const {
  return toOp(*pc());
}

Op NormalizedInstruction::mInstrOp() const {
  Op opcode = op();
#define MII(instr, a, b, i, v, d) case Op##instr##M: return opcode;
  switch (opcode) {
    MINSTRS
  case OpFPassM:
    return preppedByRef ? OpVGetM : OpCGetM;
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

NormalizedInstruction::OutputUse
NormalizedInstruction::getOutputUsage(const DynLocation* output) const {
  for (NormalizedInstruction* succ = next; succ; succ = succ->next) {
    if (succ->noOp) continue;
    for (size_t i = 0; i < succ->inputs.size(); ++i) {
      if (succ->inputs[i] == output) {
        if (succ->inputWasInferred(i)) {
          return OutputUse::Inferred;
        }
        if (dontGuardAnyInputs(succ->op())) {
          /* the consumer doesnt care about its inputs
             but we may still have inferred something about
             its outputs that a later instruction may depend on
          */
          if (!outputDependsOnInput(succ->op()) ||
              !(succ->outStack && !succ->outStack->rtt.isVagueValue() &&
                succ->getOutputUsage(succ->outStack) != OutputUse::Used) ||
              !(succ->outLocal && !succ->outLocal->rtt.isVagueValue() &&
                succ->getOutputUsage(succ->outLocal) != OutputUse::Used)) {
            return OutputUse::DoesntCare;
          }
        }
        return OutputUse::Used;
      }
    }
  }
  return OutputUse::Unused;
}

bool NormalizedInstruction::isOutputUsed(const DynLocation* output) const {
  return (output && !output->rtt.isVagueValue() &&
          getOutputUsage(output) == OutputUse::Used);
}

bool NormalizedInstruction::isAnyOutputUsed() const
{
  return (isOutputUsed(outStack) ||
          isOutputUsed(outLocal));
}

} } // HPHP::JIT
