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
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP { namespace VM { namespace JIT {

// These are the conditional branches supported for direct branch
// to their target trace at TraceExit, TraceExitType::NormalCc
static bool jccCanBeDirectExit(Opcode opc) {
  return isQueryJmpOp(opc) && (opc != JmpIsType) && (opc != JmpIsNType);
    // TODO(#2053369): JmpIsType, etc
}

// If main trace ends with an unconditional jump, and the target is not
// reached by any other branch, then copy the target of the jump to the
// end of the trace
static void elimUnconditionalJump(Trace* trace, IRFactory* irFactory) {
  boost::dynamic_bitset<> isJoin(irFactory->numBlocks());
  boost::dynamic_bitset<> havePred(irFactory->numBlocks());
  for (Block* block : trace->getBlocks()) {
    if (block->getTaken()) {
      auto id = block->getTaken()->getId();
      isJoin[id] = havePred[id];
      havePred[id] = 1;
    }
    if (block->getNext()) {
      auto id = block->getNext()->getId();
      isJoin[id] = havePred[id];
      havePred[id] = 1;
    }
  }
  Block* lastBlock = trace->back();
  auto lastInst = lastBlock->backIter(); // iterator to last instruction
  IRInstruction& jmp = *lastInst;
  if (jmp.getOpcode() == Jmp_ && !isJoin[jmp.getTaken()->getId()]) {
    Block* target = jmp.getTaken();
    lastBlock->splice(lastInst, target, target->skipLabel(), target->end());
    lastBlock->erase(lastInst); // delete the jmp
  }
}

// If main trace ends with a conditional jump with no side-effects on exit,
// hook it to the exitTrace and make it a TraceExitType::NormalCc
static void hoistConditionalJumps(Trace* trace, IRFactory* irFactory) {
  auto tail = trace->back()->end();
  IRInstruction* jccInst        = nullptr;
  IRInstruction* exitInst       = nullptr;
  IRInstruction* exitCcInst     = nullptr;
  Opcode opc = OpAdd;
  // Normally Jcc comes before a Marker
  for (int idx = 3; idx >= 0; idx--) {
    --tail; // go back to the previous instruction
    IRInstruction* inst = &*tail;
    opc = inst->getOpcode();
    if (opc == ExitTrace) {
      exitInst = inst;
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
    Block* targetBlock = jccInst->getTaken();
    auto targetInstIter = targetBlock->skipLabel();

    // Check for a NormalCc exit with no side effects
    for (auto it = targetInstIter, end = targetBlock->end(); it != end; ++it) {
      IRInstruction* instr = &*it;
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
      exitCcInst->appendSrc(irFactory->arena(), dst);
      exitInst->appendSrc(irFactory->arena(), dst);
      // Set flag so Jcc and exits know this is active
      dst->setTCA(kIRDirectJccJmpActive);
    }
  }
}

// If main trace starts with guards, have them generate a patchable jump
// to the anchor trace
static void hoistGuardJumps(Trace* trace, IRFactory* irFactory) {
  Block* guardLabel = nullptr;
  // Check the beginning of the trace for guards
  for (Block* block : trace->getBlocks()) {
    for (IRInstruction& instr : *block) {
      IRInstruction* inst = &instr;
      Opcode opc = inst->getOpcode();
      if (inst->getTaken() &&
          (opc == LdLoc    || opc == LdStack ||
           opc == GuardLoc || opc == GuardStk)) {
        Block* exitLabel = inst->getTaken();
        // Find the GuardFailure's label and confirm this branches there
        if (!guardLabel && exitLabel->getTrace() != trace) {
          auto instIter = exitLabel->skipLabel();
          // Confirm this is a GuardExit
          for (auto it = instIter, end = exitLabel->end(); it != end; ++it) {
            Opcode op = it->getOpcode();
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
        return; // terminate search
      }
      if (opc == Marker || opc == DefLabel || opc == DefSP || opc == DefFP ||
          opc == LdStack) {
        continue;
      }
      return; // terminate search
    }
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
