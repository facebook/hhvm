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

#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/opt.h"

namespace HPHP { namespace VM { namespace JIT {

// These are the conditional branches supported for direct branch
// to their target trace at TraceExit, TraceExitType::NormalCc
static bool jccCanBeDirectExit(Opcode opc) {
  // JmpGt .. JmpNSame are contiguous and all use cgJcc
  return (JmpGt <= opc && opc <= JmpNSame);
}

// If main trace ends with an unconditional jump, copy the target of
// the jump to the end of the trace
static void elimUnconditionalJump(Trace* trace, IRFactory* irFactory) {
  IRInstruction::List& instList = trace->getInstructionList();
  IRInstruction::Iterator lastInst = instList.end();
  lastInst--; // go back to the last instruction
  IRInstruction* jmpInst = *lastInst;
  if (jmpInst->getOpcode() == Jmp_) {
    Trace* targetTrace = jmpInst->getLabel()->getParent();
    IRInstruction::List& targetInstList = targetTrace->getInstructionList();
    IRInstruction::Iterator instIter = targetInstList.begin();
    instIter++; // skip over label
    // update the parent trace of the moved instructions
    for (IRInstruction::Iterator it = instIter;
         it != targetInstList.end();
         ++it) {
      (*it)->setParent(trace);
    }
    instList.splice(lastInst, targetInstList, instIter, targetInstList.end());
    // delete the jump instruction
    instList.erase(lastInst);
  }
}

// If main trace ends with a conditional jump with no side-effects on exit,
// hook it to the exitTrace and make it a TraceExitType::NormalCc
static void hoistConditionalJumps(Trace* trace, IRFactory* irFactory) {
  IRInstruction::List& instList = trace->getInstructionList();
  IRInstruction::Iterator tail  = instList.end();
  IRInstruction* jccInst        = NULL;
  IRInstruction* exitInst       = NULL;
  IRInstruction* exitCcInst     = NULL;
  Opcode opc = OpAdd;
  // Normally Jcc comes before a Marker
  for (int idx = 3; idx >= 0; idx--) {
    tail--; // go back to the previous instruction
    IRInstruction* inst = *tail;
    opc = inst->getOpcode();
    if (opc == ExitTrace) {
      exitInst = *tail;
      continue;
    }
    if (opc == Marker) {
      continue;
    }
    if (jccCanBeDirectExit(opc)) {
      jccInst = inst;
      break;
    }
    break;
  }
  if (jccCanBeDirectExit(opc)) {
    SSATmp* dst = jccInst->getDst();
    Trace* targetTrace = jccInst->getLabel()->getParent();
    IRInstruction::List& targetInstList = targetTrace->getInstructionList();
    IRInstruction::Iterator targetInstIter = targetInstList.begin();
    targetInstIter++; // skip over label

    // Check for a NormalCc exit with no side effects
    for (IRInstruction::Iterator it = targetInstIter;
         it != targetInstList.end();
         ++it) {
      IRInstruction* instr = (*it);
      // Extend to support ExitSlow, ExitSlowNoProgress, ...
      Opcode opc = instr->getOpcode();
      if (opc == ExitTraceCc) {
        exitCcInst = instr;
        break;
      } else if (opc == Marker) {
        continue;
      } else {
        // Do not optimize if there are other instructions
        break;
      }
    }

    if (exitInst && exitCcInst) {
      // Found both exits, link them to Jcc for codegen
      assert(dst);
      exitCcInst->appendSrc(*irFactory, dst);
      exitInst->appendSrc(*irFactory, dst);
      // Set flag so Jcc and exits know this is active
      dst->setTCA(kIRDirectJccJmpActive);
    }
  }
}

// If main trace starts with guards, have them generate a patchable jump
// to the anchor trace
static void hoistGuardJumps(Trace* trace, IRFactory* irFactory) {
  LabelInstruction* guardLabel = NULL;
  IRInstruction::List& instList = trace->getInstructionList();
  // Check the beginning of the trace for guards
  for (IRInstruction::Iterator it = instList.begin(); it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    Opcode opc = inst->getOpcode();
    if (inst->getLabel() &&
        (opc == LdLoc    || opc == LdStack ||
         opc == GuardLoc || opc == GuardStk)) {
      LabelInstruction* exitLabel = inst->getLabel();
      // Find the GuardFailure's label and confirm this branches there
      if (guardLabel == NULL) {
        Trace* exitTrace = exitLabel->getParent();
        IRInstruction::List& xList = exitTrace->getInstructionList();
        IRInstruction::Iterator instIter = xList.begin();
        instIter++; // skip over label
        // Confirm this is a GuardExit
        for (IRInstruction::Iterator it = instIter; it != xList.end(); ++it) {
          IRInstruction* i = *it;
          Opcode op = i->getOpcode();
          if (op == Marker) {
            continue;
          }
          if (op == ExitGuardFailure) {
            guardLabel = exitLabel;
          }
          // Do not optimize if other instructions are on exit trace
          break;
        }
      }
      if (exitLabel == guardLabel) {
        inst->setTCA(kIRDirectGuardActive);
        continue;
      }
      break;
    }
    if (opc == Marker || opc == DefLabel || opc == DefSP || opc == DefFP ||
        opc == LdStack) {
      continue;
    }
    break;
  }
}

void optimizeJumps(Trace* trace, IRFactory* irFactory) {
  elimUnconditionalJump(trace, irFactory);

  if (RuntimeOption::EvalHHIRDirectExit) {
    hoistConditionalJumps(trace, irFactory);
    hoistGuardJumps(trace, irFactory);
  }
}

}}}
