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
#include "dce.h"
#include "ir.h"
#include "simplifier.h"
#include <util/trace.h>


namespace HPHP {
namespace VM {
namespace JIT {

// An IncRef is marked as REFCOUNT_CONSUMED[_OFF_TRACE], if it is consumed by
// an instruction other than DecRefNZ that decrements the ref count.
// * REFCOUNT_CONSUMED: consumed by such an instruction in the main trace
// * REFCOUNT_CONSUMED_OFF_TRACE: consumed by such an instruction only
//   in exit traces.
const unsigned REFCOUNT_CONSUMED = 2;
const unsigned REFCOUNT_CONSUMED_OFF_TRACE = 3;

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

/*
 * Dead code elimination
 */
bool isEssential(IRInstruction* inst) {
  if (inst->getOpcode() == DecRefNZ) {
    // If the source of a DecRefNZ is not an IncRef, mark it as essential
    // because we won't remove its source as well as itself.
    // If the ref count optimization is turned off, mark all DecRefNZ as
    // essential.
    if (!RuntimeOption::EvalHHIREnableRefCountOpt ||
        inst->getSrc(0)->getInstruction()->getOpcode() != IncRef) {
      return true;
    }
  }
  if (inst->isControlFlowInstruction() && inst->getOpcode() != LdCls) {
    return true;
  }
  return opcodeHasFlags(inst->getOpcode(), Essential);
}

bool instructionIsMarkedDead(const IRInstruction* inst) {
  return inst->getId() == DEAD;
}

void removeDeadInstructions(Trace* trace) {
  trace->getInstructionList().remove_if(instructionIsMarkedDead);
}

bool isUnguardedLoad(IRInstruction* inst) {
  Opcode opc = inst->getOpcode();
  Type::Tag type = inst->getType();
  return (opc == LdStack && (type == Type::Gen || type == Type::Cell))
          || (opc == LdLoc && type == Type::Gen)
          || (opc == LdRefNR && type == Type::Cell)
          || (opc == LdMemNR && type == Type::Cell &&
              inst->getSrc(0)->getType() == Type::PtrToCell);
}

void initInstructions(Trace* trace, IRInstruction::List& wl) {
  IRInstruction::List instructions = trace->getInstructionList();
  IRInstruction::Iterator it;
  bool unreachable = false;
  TRACE(5, "DCE:vvvvvvvvvvvvvvvvvvvv\n");
  for (it = instructions.begin(); it != instructions.end(); it++) {
    IRInstruction* inst = *it;
    ASSERT(inst->getParent() == trace);
    Simplifier::copyProp(inst);
    // if this is a load that does not generate a guard, then get rid
    // of its label so that its not an essential control-flow
    // instruction
    if (isUnguardedLoad(inst)) {
      // LdStack and LdLoc instructions that produce generic types
      // and LdStack instruction that produce Cell types will not
      // generate guards, so remove the label from this instruction so
      // that its no longer an essential control-flow instruction
      inst->setLabel(NULL);
    }
    Opcode opc = inst->getOpcode();
    // decref of anything that isn't ref counted is a nop
    if ((opc == DecRef || opc == DecRefNZ) && !isRefCounted(inst->getSrc(0))) {
      inst->setId(DEAD);
      continue;
    }
    if (!unreachable && inst->isControlFlowInstruction()) {
      // mark the destination label so that the destination trace
      // is marked reachable
      inst->getLabel()->setId(LIVE);
    }
    if (!unreachable && isEssential(inst)) {
      inst->setId(LIVE);
      wl.push_back(inst);
    } else {
      if (moduleEnabled(HPHP::Trace::hhir, 5)) {
        std::ostringstream ss1;
        inst->printSrcs(ss1);
        TRACE(5, "DCE: %s\n", ss1.str().c_str());
        std::ostringstream ss2;
        inst->print(ss2);
        TRACE(5, "DCE: %s\n", ss2.str().c_str());
      }
      inst->setId(DEAD);
    }
    if (inst->getOpcode() == Jmp_) {
      unreachable = true;
    }
  }
  TRACE(5, "DCE:^^^^^^^^^^^^^^^^^^^^\n");
}

// Perform the following transformations:
// 1) Change all unconsumed IncRefs to Mov.
// 2) Mark a conditionally dead DecRefNZ as live if its corresponding IncRef
//    cannot be eliminated.
void optimizeRefCount(Trace* trace) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    if (inst->getOpcode() == IncRef &&
        inst->getId() != REFCOUNT_CONSUMED &&
        inst->getId() != REFCOUNT_CONSUMED_OFF_TRACE) {
      inst->setOpcode(Mov);
      inst->setId(DEAD);
    }
    if (inst->getOpcode() == DecRefNZ) {
      IRInstruction* srcInst = inst->getSrc(0)->getInstruction();
      if (srcInst->getId() == REFCOUNT_CONSUMED ||
          srcInst->getId() == REFCOUNT_CONSUMED_OFF_TRACE) {
        inst->setId(LIVE);
      }
    }
    // Do copyProp at last. When processing DecRefNZs, we still need to look at
    // its source which should not be trampled over.
    Simplifier::copyProp(inst);
  }
}

// Sink IncRefs consumed off trace.
// When <trace> is an exit trace, <toSink> contains all live IncRefs in the
// main trace that are consumed off trace.
void sinkIncRefs(Trace* trace,
                 IRFactory* irFactory,
                 IRInstruction::List& toSink) {
  IRInstruction::List& instList = trace->getInstructionList();
  IRInstruction::Iterator it;

  std::map<SSATmp*, SSATmp*> sunkTmps;
  if (!trace->isMain()) {
    // Sink REFCOUNT_CONSUMED_OFF_TRACE IncRefs before the first non-label
    // instruction, and create a mapping between the original tmps to the sunk
    // tmps so that we can later replace the original ones with the sunk ones.
    for (IRInstruction::ReverseIterator j = toSink.rbegin();
         j != toSink.rend();
         ++j) {
      // prependInstruction inserts an instruction to the beginning. Therefore,
      // we iterate through toSink in the reversed order.
      IRInstruction* sunkInst = irFactory->incRef((*j)->getSrc(0));
      sunkInst->setId(LIVE);
      trace->prependInstruction(sunkInst);

      ASSERT((*j)->getDst());
      ASSERT(!sunkTmps.count((*j)->getDst()));
      sunkTmps[(*j)->getDst()] = irFactory->getSSATmp(sunkInst);
    }
  }

  // An exit trace may be entered from multiple exit points. We keep track of
  // which exit traces we already pushed sunk IncRefs to, so that we won't push
  // them multiple times.
  std::set<Trace*> pushedTo;
  for (it = instList.begin(); it != instList.end(); ++it) {
    IRInstruction* inst = *it;
    if (trace->isMain()) {
      if (inst->getOpcode() == IncRef) {
        // Must be REFCOUNT_CONSUMED or REFCOUNT_CONSUMED_OFF_TRACE;
        // otherwise, it should be already removed in optimizeRefCount.
        ASSERT(inst->getId() == REFCOUNT_CONSUMED ||
               inst->getId() == REFCOUNT_CONSUMED_OFF_TRACE);
        if (inst->getId() == REFCOUNT_CONSUMED_OFF_TRACE) {
          inst->setOpcode(Mov);
          // Mark them as dead so that they'll be removed later.
          inst->setId(DEAD);
          // Put all REFCOUNT_CONSUMED_OFF_TRACE IncRefs to the sinking list.
          toSink.push_back(inst);
        }
      }
      if (inst->getOpcode() == DecRefNZ) {
        IRInstruction* srcInst = inst->getSrc(0)->getInstruction();
        if (srcInst->getId() == DEAD) {
          inst->setId(DEAD);
          // This may take O(I) time where I is the number of IncRefs
          // in the main trace.
          toSink.remove(srcInst);
        }
      }
      if (LabelInstruction* label = inst->getLabel()) {
        Trace* exitTrace = label->getTrace();
        if (!pushedTo.count(exitTrace)) {
          pushedTo.insert(exitTrace);
          sinkIncRefs(exitTrace, irFactory, toSink);
        }
      }
    } else {
      // Replace the original tmps with the sunk tmps.
      for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
        SSATmp* src = inst->getSrc(i);
        if (SSATmp* sunkTmp = sunkTmps[src]) {
          inst->setSrc(i, sunkTmp);
        }
      }
    }
  }

  // Do copyProp at last, because we need to keep REFCOUNT_CONSUMED_OFF_TRACE
  // Movs as the prototypes for sunk instructions.
  for (it = instList.begin(); it != instList.end(); ++it) {
    Simplifier::copyProp(*it);
  }
}

// These are the conditional branches supported for direct branch
// to their target trace at TraceExit, TraceExitType::NormalCc
bool jccCanBeDirectExit(Opcode opc) {
  // JmpGt .. JmpNSame are contiguous and all use cgJcc
  return (JmpGt <= opc && opc <= JmpNSame);
}

void eliminateDeadCode(Trace* trace, IRFactory* irFactory) {
  IRInstruction::List wl; // worklist of live instructions
  Trace::List& exitTraces = trace->getExitTraces();
  // first mark all exit traces as unreachable by setting the id on
  // their labels to 0
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    Trace* trace = *it;
    trace->getLabel()->setId(DEAD);
  }

  // mark the essential instructions and add them to the initial
  // work list; also mark the exit traces that are reachable by
  // any control flow instruction in the main trace.
  initInstructions(trace, wl);
  for (Trace::Iterator it = exitTraces.begin();
       it != exitTraces.end();
       it++) {
    // only process those exit traces that are reachable from
    // the main trace
    Trace* trace = *it;
    if (trace->getLabel()->getId() != DEAD) {
      initInstructions(trace, wl);
    }
  }

  // process the worklist
  while (!wl.empty()) {
    IRInstruction* inst = wl.front();
    wl.pop_front();
    for (uint32 i = 0; i < inst->getNumSrcs(); i++) {
      SSATmp* src = inst->getSrc(i);
      if (src->getInstruction()->isDefConst()) {
        continue;
      }
      IRInstruction* srcInst = src->getInstruction();
      if (srcInst->getId() == DEAD) {
        srcInst->setId(LIVE);
        wl.push_back(srcInst);
      }
      // <inst> consumes <srcInst> which is an IncRef,
      // so we mark <srcInst> as REFCOUNT_CONSUMED.
      if (inst->consumesReference(i) && srcInst->getOpcode() == IncRef) {
        if (inst->getParent()->isMain() || !srcInst->getParent()->isMain()) {
          // <srcInst> is consumed from its own trace.
          srcInst->setId(REFCOUNT_CONSUMED);
        } else {
          // <srcInst> is consumed off trace.
          if (srcInst->getId() != REFCOUNT_CONSUMED) {
            // mark <srcInst> as REFCOUNT_CONSUMED_OFF_TRACE unless it is
            // also consumed from its own trace.
            srcInst->setId(REFCOUNT_CONSUMED_OFF_TRACE);
          }
        }
      }
    }
  }

  // Optimize IncRefs and DecRefs.
  optimizeRefCount(trace);
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); ++it) {
    optimizeRefCount(*it);
  }

  if (RuntimeOption::EvalHHIREnableSinking) {
    // Sink IncRefs consumed off trace.
    IRInstruction::List toSink;
    sinkIncRefs(trace, irFactory, toSink);
  }

  // now remove instructions whose id == DEAD
  removeDeadInstructions(trace);
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); it++) {
    removeDeadInstructions(*it);
  }

  // If main trace ends with an unconditional jump, copy the target of
  // the jump to the end of the trace
  IRInstruction::List& instList = trace->getInstructionList();
  IRInstruction::Iterator lastInst = instList.end();
  lastInst--; // go back to the last instruction
  IRInstruction* jmpInst = *lastInst;
  if (jmpInst->getOpcode() == Jmp_) {
    Trace* targetTrace = jmpInst->getLabel()->getTrace();
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

  // If main trace ends with a conditional jump with no side-effects on exit,
  // hook it to the exitTrace and make it a TraceExitType::NormalCc
  if (RuntimeOption::EvalHHIRDirectExit) {
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
      Trace* targetTrace = jccInst->getLabel()->getTrace();
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

      if (exitInst && exitCcInst &&
          exitCcInst->getNumSrcs() > NUM_FIXED_SRCS &&
          exitInst->getNumSrcs() > NUM_FIXED_SRCS) {
        // Found both exits, link them to Jcc for codegen
        ASSERT(dst);
        ExtendedInstruction* exCcInst = (ExtendedInstruction*)exitCcInst;
        exCcInst->appendExtendedSrc(*irFactory, dst);
        ExtendedInstruction* exInst = (ExtendedInstruction*)exitInst;
        exInst->appendExtendedSrc(*irFactory, dst);
        // Set flag so Jcc and exits know this is active
        dst->setTCA(kIRDirectJccJmpActive);
      }
    }
  }

  // If main trace starts with guards, have them generate a patchable jump
  // to the anchor trace
  if (RuntimeOption::EvalHHIRDirectExit) {
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
          Trace* exitTrace = exitLabel->getTrace();
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
}

} } }
