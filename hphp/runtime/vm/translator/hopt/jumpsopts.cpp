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
  if (jmp.op() == Jmp_ && !isJoin[jmp.getTaken()->getId()]) {
    Block* target = jmp.getTaken();
    lastBlock->splice(lastInst, target, target->skipLabel(), target->end());
    lastBlock->erase(lastInst); // delete the jmp
  }
}

/**
 * If main trace ends with a conditional jump with no side-effects on exit,
 * hook it to the exitTrace and make it a TraceExitType::NormalCc.
 *
 * This function essentially looks for the following code pattern:
 *
 * Main Trace:
 * ----------
 * L1: // jccBlock
 *    ...
 *    Jcc ... -> L3
 * L2: // lastBlock
 *    DefLabel
 *    [Marker]
 *    ExitTrace
 *
 * Exit Trace:
 * ----------
 * L3: // targetBlock
 *   DefLabel
 *   [Marker]
 *   ExitTraceCc
 *
 * If the pattern is found, Jcc's dst operand is linked to the ExitTrace and
 * ExitTraceCc instructions and it's flagged with kIRDirectJccJmpActive.  This
 * then triggers CodeGenerator to emit a REQ_BIND_JMPCC_FIRST service request.
 *
 */
static void hoistConditionalJumps(Trace* trace, IRFactory* irFactory) {
  IRInstruction* exitInst       = nullptr;
  IRInstruction* exitCcInst     = nullptr;
  Opcode opc = OpAdd;
  // Normally Jcc comes before a Marker
  auto& blocks = trace->getBlocks();
  if (blocks.size() < 2) return;
  auto it = blocks.end();
  Block* lastBlock = *(--it);
  Block* jccBlock  = *(--it);

  IRInstruction& jccInst = *(jccBlock->back());
  if (!jccCanBeDirectExit(jccInst.op())) return;

  for (auto it = lastBlock->skipLabel(), end = lastBlock->end(); it != end;
       it++) {
    IRInstruction& inst = *it;
    opc = inst.op();
    if (opc == ExitTrace) {
      exitInst = &inst;
      break;
    }
    if (opc != Marker) {
      // Found real instruction on the last block
      return;
    }
  }
  if (exitInst) {
    Block* targetBlock = jccInst.getTaken();
    auto targetInstIter = targetBlock->skipLabel();

    // Check for a NormalCc exit with no side effects
    for (auto it = targetInstIter, end = targetBlock->end(); it != end; ++it) {
      IRInstruction* instr = &*it;
      // Extend to support ExitSlow, ExitSlowNoProgress, ...
      Opcode opc = instr->op();
      if (opc == ExitTraceCc) {
        exitCcInst = instr;
        break;
      } else if (opc != Marker) {
        // Do not optimize if there are other instructions
        break;
      }
    }

    if (exitCcInst) {
      // Found both exits, link them to Jcc for codegen
      ExitData* exitData = new (irFactory->arena()) ExitData(&jccInst);
      exitCcInst->setExtra(exitData);
      exitInst->setExtra(exitData);
      // Set flag so Jcc and exits know this is active
      jccInst.setTCA(kIRDirectJccJmpActive);
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
      Opcode opc = inst->op();
      if (inst->getTaken() &&
          (opc == LdLoc    || opc == LdStack ||
           opc == GuardLoc || opc == GuardStk)) {
        Block* exitLabel = inst->getTaken();
        // Find the GuardFailure's label and confirm this branches there
        if (!guardLabel && exitLabel->getTrace() != trace) {
          auto instIter = exitLabel->skipLabel();
          // Confirm this is a GuardExit
          for (auto it = instIter, end = exitLabel->end(); it != end; ++it) {
            Opcode op = it->op();
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
