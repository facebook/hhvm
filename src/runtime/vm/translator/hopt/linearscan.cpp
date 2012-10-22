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

#include "linearscan.h"

namespace HPHP {
namespace VM {
namespace JIT{

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::tx64;

const char* LinearScan::RegNames[LinearScan::NumRegs] = {
  "rax", "rcx", "rdx",
  "rbx", // 3: rVmSP
  "rsp", // 4: native code SP
  "rbp", // 5: rVmFP
  "rsi", "rdi",
  "r8", "r9",
  "r10", // 10: rScratch
  "r11",
  "r12", // 12: TC pointer
  "r13", "r14",
  "r15" // rStashedAR
};

const register_name_t
LinearScan::CallerSavedRegs[LinearScan::NumCallerSavedRegs] = {
  reg::rax, reg::rcx, reg::rdx, reg::rsi, reg::rdi,
  reg::r8,  reg::r9,  reg::r10, reg::r11
};

LinearScan::LinearScan(IRFactory* irFactory, TraceBuilder* traceBuilder) {
  m_irFactory = irFactory;
  m_tb = traceBuilder;
  for (int i = 0; i < NumRegs; i++) {
    m_regs[i].m_ssaTmp = NULL;
    m_regs[i].m_regNo = i;
    m_regs[i].m_pinned = false;
    m_regs[i].m_reserved = false;
  }

  // Mark reserved regs.
  m_regs[LinearScan::regNameAsInt(rVmSP)].m_reserved = true;
  m_regs[LinearScan::regNameAsInt(rSP)].m_reserved = true;
  m_regs[LinearScan::regNameAsInt(rVmFP)].m_reserved = true;
  m_regs[LinearScan::regNameAsInt(rScratch)].m_reserved = true;
  m_regs[LinearScan::regNameAsInt(rTlPtr)].m_reserved = true;

  // Reserve extra regs for testing purpose.
  uint32 numFreeRegs = RuntimeOption::EvalHHIRNumFreeRegs;
  for (int i = NumRegs - 1; i >= 0; i--) {
    if (!m_regs[i].m_reserved) {
      if (numFreeRegs == 0) {
        m_regs[i].m_reserved = true;
      } else {
        --numFreeRegs;
      }
    }
  }
}

uint32 LinearScan::computeLiveOutRegs(IRInstruction* inst, uint32 liveRegs) {
  uint32 instId = inst->getId();
  for (uint32 i = 0; i < inst->getNumSrcs(); i++) {
    SSATmp* src = inst->getSrc(i);
    if (src->getLastUseId() == instId) {
      for (uint32 locIndex = 0;
           locIndex < src->getNumAssignedLocs();
           ++locIndex) {
        if (src->isAssignedReg(locIndex)) {
          // inst is the last use of the register assigned to this SSATmp
          // remove src reg from live regs set
          liveRegs &= ~REG_MASK(src->getAssignedLoc(locIndex));
        }
      }
    }
  }
  // add the destination register to the live regs set
  SSATmp* dst = inst->getDst();
  if (dst != NULL) {
    for (uint32 locIndex = 0;
         locIndex < dst->getNumAssignedLocs();
         locIndex++) {
      if (dst->isAssignedReg(locIndex)) {
        liveRegs |= REG_MASK(dst->getAssignedLoc(locIndex));
      }
    }
  }
  inst->setLiveOutRegs((uint16)liveRegs);
  return liveRegs;
}

/*
 * Computes the live out regs at each instruction in a trace.
 * This function takes as the second argument an initial live
 * register set at the start of the trace and returns the live
 * registers at the end of the trace.
 * The function uses the same last use information and instruction
 * ordering used by the linear scan register allocator, so its
 * important that this function iterates over the instruction in
 * the same order that linear scan orders the instructions.
 */
uint32 LinearScan::computeLiveOutRegs(Trace* trace, uint32 liveOutRegs) {
  IRInstruction::Iterator it;
  IRInstruction::List& instructionList = trace->getInstructionList();
  for (it = instructionList.begin();
       it != instructionList.end();
       it++) {
    IRInstruction* inst = *it;
    liveOutRegs = LinearScan::computeLiveOutRegs(inst, liveOutRegs);
    if (inst->isControlFlowInstruction()) {
      // follow control flow edges to the exit blocks in the same order
      // that linear scan does
      LabelInstruction* label = inst->getLabel();
      if (label != NULL && label->getId() == inst->getId() + 1) {
        liveOutRegs = LinearScan::computeLiveOutRegs(label->getTrace(), liveOutRegs);
      }
    }
  }
  return liveOutRegs;
}

void LinearScan::allocRegToInstruction(Trace* trace,
                                       IRInstruction::Iterator it) {
  IRInstruction* inst = *it;
  if (RuntimeOption::EvalDumpIR > 3) {
    std::cout << "--- allocating to instruction: ";
    inst->print(std::cout);
    std::cout << std::endl;
  }

  // Reload all source operands if necessary.
  // Mark registers as unpinned.
  for (int regNo = 0; regNo < NumRegs; ++regNo) {
    m_regs[regNo].m_pinned = false;
  }
  std::vector<bool> needsReloading(inst->getNumSrcs(), true);
  for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
    SSATmp* tmp = inst->getSrc(i);
    int32 slotId = tmp->getAnalysisValue();
    if (slotId == -1) {
      needsReloading[i] = false;
    } else if (tmp = m_slots[slotId].m_latestTmp) {
      needsReloading[i] = false;
      inst->setSrc(i, tmp);
    }
    if (!needsReloading[i]) {
      for (uint32 locIndex = 0;
           locIndex < tmp->getNumAssignedLocs();
           ++locIndex) {
        register_name_t srcReg = tmp->getAssignedLoc(locIndex);
        m_regs[LinearScan::regNameAsInt(srcReg)].m_pinned = true;
      }
    }
  }
  for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
    SSATmp* tmp = inst->getSrc(i);
    int32 slotId = tmp->getAnalysisValue();
    if (needsReloading[i]) {
      // <tmp> is spilled, and not reloaded.
      // Therefore, We need to reload the value into a new SSATmp.

      // Insert the Reload instruction.
      SSATmp* slotTmp = m_slots[slotId].m_slotTmp;
      IRInstruction* reload = m_irFactory->reload(slotTmp);
      trace->getInstructionList().insert(it, reload);
      reload->setParent(trace);

      // Create <newTmp> which inherits <tmp>'s slot ID and
      // <slotTmp>'s last use ID.
      // Replace <tmp> with <newTmp> in <inst>.
      SSATmp* newTmp = m_irFactory->getSSATmp(reload);
      newTmp->setLastUseId(slotTmp->getLastUseId());
      newTmp->setAnalysisValue(slotId);
      inst->setSrc(i, newTmp);
      // newTmp and tmp share the same type.
      for (uint32 locIndex = 0;
           locIndex < tmp->getNumAssignedLocs();
           ++locIndex) {
        allocRegToTmp(newTmp, locIndex);
      }
      if (trace->isMain()) {
        // To make the algorithm simple, do not add reload in exit traces.
        // Otherwise, we would have to check whether the latest reloaded
        // value dominates the current instruction.
        // std::cout << "m_latestTmp[" << slotId << "] = " << newTmp << "\n";
        m_slots[slotId].m_latestTmp = newTmp;
      }
      if (RuntimeOption::EvalDumpIR > 4) {
        std::cout << "--- created reload: ";
        reload->print(std::cout);
        std::cout << std::endl;
      }
    }
  }

  freeRegsAtId(inst->getId());
  // Update next native.
  if (getNextNative() == inst) {
    ASSERT(!m_natives.empty());
    m_natives.pop_front();
    computePreColoringHint();
  }

  SSATmp* ssaTmp = inst->getDst();
  if (!ssaTmp) {
    return;
  }

  Opcode opc = inst->getOpcode();
  Type::Tag type = inst->getType();

  if (opc == DefFP || opc == FreeActRec) {
    allocRegToTmp(&m_regs[LinearScan::regNameAsInt(rVmFP)], ssaTmp, 0);
    return;
  }
  if (opc == DefSP || opc == Call || opc == SpillStack ||
      opc == AllocActRec || opc == SpillStackAllocAR ||
      opc == RetVal || opc ==  NewObj || opc == InterpOne) {
    ASSERT(type == Type::SP);
    allocRegToTmp(&m_regs[LinearScan::regNameAsInt(rVmSP)], ssaTmp, 0);
    return;
  }

  ASSERT(type != Type::SP);

  if (ssaTmp->getLastUseId() == 0) {
    // This instruction's destination is not used
    return;
  }

  if (type == Type::Null || type == Type::Uninit || type == Type::None) {
    // these types don't get a register because their values are static
    return;
  }
  allocRegToTmp(ssaTmp, 0);
  if (type == Type::Cell || type == Type::Gen || type == Type::FuncClassRef) {
    // need 2 registers for these types, 1 for type and 1 for value
    // or 1 for func* and 1 for class*
    allocRegToTmp(ssaTmp, 1);
  }
}

void LinearScan::allocRegToTmp(SSATmp* ssaTmp, uint32_t index) {
  bool preferCallerSaved = true;
  if (RuntimeOption::EvalHHIREnableCalleeSavedOpt) {
    // Prefer caller-saved registers iff <ssaTmp> doesn't span native.
    preferCallerSaved = (ssaTmp->getLastUseId() <= getNextNativeId());
  }

  RegState* reg = NULL;
  if (!preferCallerSaved) {
    reg = getFreeReg(false);
    if (reg->isCallerSaved()) {
      // If we are out of callee-saved registers, fall into the logic of
      // assigning a caller-saved register.
      pushFreeReg(reg);
      // getFreeReg pins the reg. Need restore it here.
      reg->m_pinned = false;
      reg = NULL;
    }
  }
  if (reg == NULL && RuntimeOption::EvalHHIREnablePreColoring) {
    // Pre-colors ssaTmp if it's used as an argument of next native.
    // Search for the original tmp instead of <ssaTmp> itself, because
    // the pre-coloring hint is not aware of reloaded tmps.
    register_name_t targetRegNo =
      m_preColoringHint.getPreColoringReg(getOrigTmp(ssaTmp), index);
    if (targetRegNo != reg::noreg) {
      reg = getReg(&m_regs[LinearScan::regNameAsInt(targetRegNo)]);
    }
  }
  if (reg == NULL &&
      RuntimeOption::EvalHHIREnablePreColoring &&
      ssaTmp->getInstruction()->isNative()) {
    // Pre-colors ssaTmp if it's the return value of a native.
    ASSERT(index == 0);
    reg = getReg(&m_regs[LinearScan::regNameAsInt(reg::rax)]);
  }
  if (reg == NULL) {
    // No pre-coloring for this tmp.
    // Pick a regular caller-saved reg.
    reg = getFreeReg(true);
  }

  ASSERT(reg);
  if (!preferCallerSaved && reg->isCallerSaved()) {
    // ssaTmp spans native, but we failed to find a free callee-saved reg.
    // We eagerly add a spill ssaTmp, and update ssaTmp's live range
    // to end with next native, because we know we have to spill it at
    // the next native.
    // Setting the last use ID to the next native is conservative.
    // Setting it to the last use before the next native would be more precise,
    // but that would be more expensive to compute.
    if (ssaTmp->getAnalysisValue() == -1) {
      createSpillSlot(ssaTmp);
    }
    ssaTmp->setLastUseId(getNextNativeId());
  }

  allocRegToTmp(reg, ssaTmp, index);
}

void LinearScan::allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index) {
  reg->m_ssaTmp = ssaTmp;
  // mark inst as using this register
  ssaTmp->setAssignedLoc((register_name_t)reg->m_regNo, index);
  uint32_t lastUseId = ssaTmp->getLastUseId();
  if (reg->isReserved()) {
    return;
  }
  // insert into the list of assigned registers sorted by last use id
  std::list<RegState*>::iterator it = m_allocatedRegs.begin();
  for (; it != m_allocatedRegs.end(); ++it) {
    if (lastUseId > (*it)->m_ssaTmp->m_lastUseId) {
      break;
    }
  }
  reg->m_pos = m_allocatedRegs.insert(it, reg);
}

// Assign spill location numbers to Spill/Reload.
uint32 LinearScan::assignSpillLoc(Trace* trace) {
  return assignSpillLocAux(trace, 0, 0);
}

uint32 LinearScan::assignSpillLocAux(Trace* trace,
                                     uint32 nextSpillLoc,
                                     uint32 nextMmxReg) {
  IRInstruction::List& instructionList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instructionList.begin();
       it != instructionList.end();
       ++it) {
    IRInstruction* inst = *it;
    if (getNextNative() == inst) {
      ASSERT(!m_natives.empty());
      m_natives.pop_front();
    }
    if (inst->getOpcode() == Spill) {
      SSATmp* dst = inst->getDst();
      SSATmp* src = inst->getSrc(0);
      for (uint32 locIndex = 0;
           locIndex < src->getNumAssignedLocs();
           ++locIndex) {
        if (dst->getLastUseId() <= getNextNativeId()) {
          TRACE(3, "[counter] 1 spill a tmp that does not span native\n");
        } else {
          TRACE(3, "[counter] 1 spill a tmp that spans native\n");
        }
        if (RuntimeOption::EvalHHIREnableMmx &&
            dst->getLastUseId() <= getNextNativeId() &&
            nextMmxReg < (uint32)NumMmxRegs) {
          // The live range of the spill slot doesn't span native calls,
          // and we still have free MMX registers.
          dst->setMmxReg((register_name_t)nextMmxReg, locIndex);
          ++nextMmxReg;
          TRACE(3, "[counter] 1 spill to mmx\n");
        } else {
          dst->setSpillLoc(nextSpillLoc, locIndex);
          ++nextSpillLoc;
          TRACE(3, "[counter] 1 spill to memory\n");
        }
      }
    }
    if (inst->getOpcode() == Reload) {
      SSATmp* src = inst->getSrc(0);
      for (uint32 locIndex = 0;
           locIndex < src->getNumAssignedLocs();
           ++locIndex) {
        if (src->isAssignedMmxReg(locIndex)) {
          TRACE(3, "[counter] reload from mmx\n");
        } else {
          TRACE(3, "[counter] reload from memory\n");
        }
      }
    }
    if (inst->isControlFlowInstruction()) {
      LabelInstruction* label = inst->getLabel();
      if (label != NULL && label->getId() == inst->getId() + 1) {
        nextSpillLoc = assignSpillLocAux(label->getTrace(),
                                         nextSpillLoc,
                                         nextMmxReg);
      }
    }
  }
  return nextSpillLoc;
}

void LinearScan::insertAllocFreeSpill(Trace* trace, uint32 numExtraSpillLocs) {
  insertAllocFreeSpillAux(trace, numExtraSpillLocs);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); ++it) {
    insertAllocFreeSpillAux(*it, numExtraSpillLocs);
  }
}

void LinearScan::insertAllocFreeSpillAux(Trace* trace,
                                         uint32 numExtraSpillLocs) {
  SSATmp* tmp = m_irFactory->getSSATmp(
    m_irFactory->defConst((int64)numExtraSpillLocs));

  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end(); ) {
    IRInstruction::Iterator next = it; ++next;
    IRInstruction* inst = *it;
    Opcode opc = inst->getOpcode();
    if (opc == Call) {
      // Insert FreeSpill and AllocSpill around each Call.
      IRInstruction* allocSpill = m_irFactory->allocSpill(tmp);
      IRInstruction* freeSpill = m_irFactory->freeSpill(tmp);
      instList.insert(it, freeSpill);
      freeSpill->setParent(trace);
      instList.insert(next, allocSpill);
      allocSpill->setParent(trace);
    } else if (opc == ExitTrace || opc == ExitSlow ||
               opc == ExitSlowNoProgress || opc == ExitGuardFailure ||
               opc == RetCtrl) {
      // Insert FreeSpill at trace exits.
      IRInstruction* freeSpill = m_irFactory->freeSpill(tmp);
      instList.insert(it, freeSpill);
      freeSpill->setParent(trace);
    }
    it = next;
  }

  // Insert AllocSpill at the start of the main trace.
  if (trace->isMain()) {
    IRInstruction* allocSpill = m_irFactory->allocSpill(tmp);
    trace->prependInstruction(allocSpill);
  }
}

void LinearScan::collectNatives(Trace* trace) {
  // May be re-executed. Need initialize <m_natives> each time.
  m_natives.clear();
  collectNativesAux(trace);
}

void LinearScan::collectNativesAux(Trace* trace) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    if (inst->isNative()) {
      m_natives.push_back(inst);
    }
    if (inst->isControlFlowInstruction()) {
      LabelInstruction* label = inst->getLabel();
      if (label != NULL && label->getId() == inst->getId() + 1) {
        collectNativesAux(label->getTrace());
      }
    }
  }
}

// XXX: to be refactored
// This function repeats the logic in cg to pre-color tmps that are
// going to be used in next native.
void LinearScan::computePreColoringHint() {
  m_preColoringHint.clear();
  IRInstruction* nextNative = getNextNative();
  if (nextNative == NULL) {
    return;
  }
  Opcode opc = nextNative->getOpcode();
  switch (opc) {
    case Box:
      if (nextNative->getSrc(0)->getType() == Type::Cell) {
        m_preColoringHint.add(nextNative->getSrc(0), 1, 0);
      }
      m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
      break;
    case LdObjMethod:
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      m_preColoringHint.add(nextNative->getSrc(0), 0, 2);
      break;
    case LdFunc:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
      break;
    case NativeImpl:
      m_preColoringHint.add(nextNative->getSrc(1), 0, 0);
      break;
    case DecRefLocals:
    case DecRefLocalsThis:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      break;
    case Print:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      break;
    case AddElem:
      if (nextNative->getSrc(1)->getType() == Type::Int &&
          nextNative->getSrc(2)->getType() == Type::Int) {
        m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
        m_preColoringHint.add(nextNative->getSrc(1), 0, 2);
        m_preColoringHint.add(nextNative->getSrc(2), 0, 3);
      } else {
        m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
        m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
        m_preColoringHint.add(nextNative->getSrc(2), 0, 2);
        m_preColoringHint.add(nextNative->getSrc(2), 1, 3);
      }
      break;
    case AddNewElem:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      m_preColoringHint.add(nextNative->getSrc(1), 1, 2);
      break;
    case Concat:
      {
        Type::Tag lType = nextNative->getSrc(0)->getType();
        Type::Tag rType = nextNative->getSrc(1)->getType();
        if (Type::isString(lType) && Type::isString(rType) ||
            Type::isString(lType) && rType == Type::Int ||
            lType == Type::Int && Type::isString(rType)) {
          m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
          m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
        } else {
          m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
          m_preColoringHint.add(nextNative->getSrc(1), 0, 3);
        }
      }
      break;
    case ArrayAdd:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      break;
    case DefFunc:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      break;
    case CreateCont:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      m_preColoringHint.add(nextNative->getSrc(2), 0, 2);
      m_preColoringHint.add(nextNative->getSrc(3), 0, 3);
      break;
    case FillContLocals:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      m_preColoringHint.add(nextNative->getSrc(2), 0, 2);
      m_preColoringHint.add(nextNative->getSrc(3), 0, 3);
      break;
    case UnpackCont:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      break;
    case PackCont:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
      m_preColoringHint.add(nextNative->getSrc(2), 0, 2);
      m_preColoringHint.add(nextNative->getSrc(3), 0, 3);
      break;
    case OpEq:
    case OpNeq:
    case OpSame:
    case OpNSame:
      {
        auto src1 = nextNative->getSrc(0);
        auto src2 = nextNative->getSrc(1);

        if ((Type::isString(src1->getType())
             && Type::isString(src2->getType()))
            || (Type::isString(src1->getType()) && !src1->isConst())) {
          m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
          m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
        }
      }
      break;
    case Conv:
    {
      SSATmp* src = nextNative->getSrc(0);
      Type::Tag toType = nextNative->getType();
      Type::Tag fromType = src->getType();
      if (toType == Type::Bool) {
        switch (fromType) {
          case Type::Cell:
            m_preColoringHint.add(src, 0, 0);
            m_preColoringHint.add(src, 1, 1);
            break;
          case Type::Str:
          case Type::StaticStr:
          case Type::Arr:
          case Type::Obj:
            m_preColoringHint.add(src, 0, 0);
            break;
          default:
            break;
        }
      } else if (Type::isString(toType)) {
        if (fromType == Type::Int) {
          m_preColoringHint.add(src, 0, 0);
        }
      } else if (Type::isString(fromType) && toType == Type::Int) {
        m_preColoringHint.add(src, 0, 0);
      }
      break;
    }
    default:
      break;
  }
}

// Create the initial free list.
// It must be called after computePreColoringHint, because the order of
// caller-saved regs depends on pre-coloring hints.
void LinearScan::initFreeList() {
  // reserve extra regs for testing purpose.
  for (int i = NumRegs - 1; i >= 0; i--) {
    if (!m_regs[i].m_reserved) {
      pushFreeReg(&m_regs[i]);
    }
  }
}

void LinearScan::coalesce(Trace* trace) {
  coalesceAux(trace);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); ++it) {
    coalesceAux(*it);
  }
}

void LinearScan::coalesceAux(Trace* trace) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
      SSATmp* src = inst->getSrc(i);
      SSATmp* origSrc = canonicalize(src);
      if (origSrc != src) {
        // Replace every operand with its canonicalized version.
        inst->setSrc(i, origSrc);
      }
    }
  }
}

void LinearScan::preAllocSpillLoc(Trace* trace, uint32 numSpillLocs) {
  preAllocSpillLocAux(trace, numSpillLocs);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); ++it) {
    preAllocSpillLocAux(*it, numSpillLocs);
  }
}

void LinearScan::preAllocSpillLocAux(Trace* trace, uint32 numSpillLocs) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    if (inst->getOpcode() == Spill) {
      SSATmp* dst = inst->getDst();
      for (uint32 index = 0; index < dst->getNumAssignedLocs(); ++index) {
        ASSERT(dst->isAssignedSpillLoc(index) || dst->isAssignedMmxReg(index));
        if (dst->isAssignedSpillLoc(index)) {
          uint32 spillLoc = dst->getSpillLoc(index);
          // Native stack layout:
          // |               |
          // +---------------+
          // |               |  <-- spill[5..]
          // | pre allocated |  <-- spill[4]
          // |  (16 slots)   |  <-- spill[3]
          // +---------------+
          // |  return addr  |
          // +---------------+
          // |    extra      |  <-- spill[2]
          // |    spill      |  <-- spill[1]
          // |  locations    |  <-- spill[0]
          // +---------------+  <-- %rsp
          // If a spill location falls into the pre-allocated region, we
          // need to increase its index by 1 to avoid overwriting the
          // return address.
          if (spillLoc + NumPreAllocatedSpillLocs >= numSpillLocs) {
            dst->setSpillLoc(spillLoc + 1, index);
          }
        }
      }
    }
  }
}

void LinearScan::allocRegsToTrace(Trace* trace) {
  if (RuntimeOption::EvalHHIREnableCoalescing) {
    // <coalesc> doesn't need instruction numbering.
    coalesce(trace);
  }

  numberInstructions(trace);

  collectNatives(trace);
  computePreColoringHint();
  initFreeList();
  allocRegsToTraceAux(trace);
  // Renumber instructions, because we added spills and reloads.
  numberInstructions(trace);

  if (RuntimeOption::EvalHHIREnableRematerialization && m_slots.size() > 0) {
    // Don't bother rematerializing the trace if it has no Spill/Reload.
    if (RuntimeOption::EvalDumpIR > 5) {
      std::cout << "--------- HHIR before rematerialization ---------\n";
      trace->print(std::cout, false);
      std::cout << "-------------------------------------------------\n";
    }
    rematerialize(trace);
  }

  // assignSpillLoc needs next natives in order to decide whether we
  // can use MMX registers.
  collectNatives(trace);
  // Make sure rsp is 16-aligned.
  uint32 numSpillLocs = assignSpillLoc(trace);
  if (numSpillLocs % 2) {
    ++numSpillLocs;
  }
  ASSERT(NumPreAllocatedSpillLocs % 2 == 0);
  if (numSpillLocs > 0) {
    preAllocSpillLoc(trace, numSpillLocs);
    if (numSpillLocs > (uint32)NumPreAllocatedSpillLocs) {
      // We only insert AllocSpill and FreeSpill when the pre-allocated
      // spill locations are not enough.
      // AllocSpill and FreeSpill take the number of extra spill locations
      // besides the pre-allocated ones.
      insertAllocFreeSpill(trace, numSpillLocs - NumPreAllocatedSpillLocs);
    }
  }
  numberInstructions(trace);

  // record the live out register set at each instruction
  LinearScan::computeLiveOutRegs(trace, 0);
}

void LinearScan::allocRegsToTraceAux(Trace* trace) {
  IRInstruction::List& instructionList = trace->getInstructionList();
  IRInstruction::Iterator it;
  for (it = instructionList.begin();
       it != instructionList.end();
       it++) {
    IRInstruction* inst = *it;
    allocRegToInstruction(trace, it);
    if (RuntimeOption::EvalDumpIR > 3) {
      std::cout << "--- allocated to instruction: ";
      inst->print(std::cout);
      std::cout << "\n";
    }
    if (inst->isControlFlowInstruction()) {
      // This instruction may transfer control to another trace
      // If this is the last instruction in the trace that can branch
      // to this target trace, then allocate registers to the target
      // trace, effectively linearizing the target trace after inst.
      LabelInstruction* label = inst->getLabel();
      if (label != NULL && label->getId() == inst->getId() + 1) {
        allocRegsToTraceAux(label->getTrace());
      }
    }
  }

  // Insert spill instructions.
  // Reload instructions are already added in <allocRegsToTrace>.
  for (it = instructionList.begin(); it != instructionList.end(); ) {
    IRInstruction::Iterator next = it; ++next;
    IRInstruction* inst = *it;
    if (inst->getOpcode() != Reload) {
      // Reloaded SSATmps needn't be spilled again.
      if (SSATmp* dst = inst->getDst()) {
        int32 slotId = dst->getAnalysisValue();
        if (slotId != -1) {
          // If this instruction is marked to be spilled,
          // add a spill right afterwards.
          IRInstruction* spillInst =
            m_slots[slotId].m_slotTmp->getInstruction();
          instructionList.insert(next, spillInst);
          spillInst->setParent(trace);
        }
      }
    }
    it = next;
  }
}

void LinearScan::rematerialize(Trace* trace) {
  rematerializeAux(trace,
                   NULL,
                   std::vector<SSATmp*>(m_irFactory->getNumLocals()));
  numberInstructions(trace);

  // We only replaced Reloads in <rematerializeAux>.
  // Here, we remove Spills that are never reloaded.
  removeUnusedSpills(trace);
  numberInstructions(trace);
}

void LinearScan::rematerializeAux(Trace* trace,
                                  SSATmp* curSp,
                                  std::vector<SSATmp*> localValues) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end();
       ++it) {
    IRInstruction* inst = *it;
    Opcode opc = inst->getOpcode();
    SSATmp* dst = inst->getDst();
    if (opc == Reload) {
      // s = Spill t0
      // t = Reload s
      SSATmp* spilledTmp = getSpilledTmp(dst);
      IRInstruction* spilledInst = spilledTmp->getInstruction();
      IRInstruction* newInst = NULL;
      if (spilledInst->isRematerializable() ||
          spilledInst->getOpcode() == LdStack &&
          spilledInst->getSrc(0) == curSp) {
        // XXX: could change <newInst> to the non-check version.
        // Rematerialize those rematerializable instructions (i.e.,
        // isRematerializable returns true) and LdStack.
        newInst = spilledInst->clone(m_irFactory);
        // The new instruction needn't have an exit label, because it is always
        // dominated by the original instruction.
        newInst->setLabel(NULL);
      } else {
        // Rematerialize LdLoc.
        std::vector<SSATmp*>::iterator pos =
          std::find(localValues.begin(),
                    localValues.end(),
                    canonicalize(spilledTmp));
        // Search for a local that stores the value of <spilledTmp>.
        if (pos != localValues.end()) {
          size_t locId = pos - localValues.begin();
          newInst = m_irFactory->ldLoc(m_tb->genLdHome(locId),
                                       dst->getType(),
                                       NULL);
        }
      }
      if (newInst) {
        newInst->setDst(dst);
        newInst->getDst()->m_inst = newInst;
        *it = newInst;
        newInst->setParent(trace);
      }
    }

    // Updating <curSp> and <localValues>.
    if (dst && dst->getAssignedLoc() == rVmSP) {
      // <inst> modifies the stack pointer.
      curSp = dst;
    }
    if (opc == LdLoc || opc == StLoc || opc == StLocNT) {
      // dst = LdLoc home
      // StLoc/StLocNT home, src
      int locId = getLocalIdFromHomeOpnd(inst->getSrc(0));
      SSATmp* localValue = (opc == LdLoc ? dst : inst->getSrc(1));
      localValues[locId] = canonicalize(localValue);
    }

    if (inst->isControlFlowInstruction()) {
      LabelInstruction* label = inst->getLabel();
      if (label != NULL && label->getId() == inst->getId() + 1) {
        rematerializeAux(label->getTrace(), curSp, localValues);
      }
    }
  }
}

void LinearScan::removeUnusedSpills(Trace* trace) {
  removeUnusedSpillsAux(trace);
  Trace::List& exitTraces = trace->getExitTraces();
  for (Trace::Iterator it = exitTraces.begin(); it != exitTraces.end(); ++it) {
    removeUnusedSpillsAux(*it);
  }
}

void LinearScan::removeUnusedSpillsAux(Trace* trace) {
  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end(); ) {
    IRInstruction::Iterator next = it; ++next;
    IRInstruction* inst = *it;
    if (inst->getOpcode() == Spill && inst->getDst()->getUseCount() == 0) {
      instList.erase(it);
      SSATmp* src = inst->getSrc(0);
      if (src->decUseCount() == 0) {
        Opcode srcOpc = src->getInstruction()->getOpcode();
        // Not all instructions are able to take -1 as its dest reg.
        // We pick LdLoc and IncRef because they occur often.
        if (srcOpc == IncRef || srcOpc == LdLoc) {
          for (uint32 locIndex = 0;
               locIndex < src->getNumAssignedLocs();
               ++locIndex) {
            src->setAssignedLoc(reg::noreg, locIndex);
          }
        }
      }
    }
    it = next;
  }
}

void LinearScan::freeRegsAtId(uint32_t id) {
  // free all registers whose lifetime ends at this id
  // Note that we free registers before we allocate a register
  // to this instruction, so we have to be careful to finish using
  // a register before over-writing it.
  for (std::list<RegState*>::iterator it = m_allocatedRegs.begin();
       it != m_allocatedRegs.end(); ) {
    std::list<RegState*>::iterator next = it; ++next;
    RegState* reg = *it;
    ASSERT(reg->m_ssaTmp);
    if (reg->m_ssaTmp->m_lastUseId <= id) {
      m_allocatedRegs.erase(it);
      freeReg(reg);
    }
    it = next;
  }
}

// Try to get a specific register.
// Returns NULL if <reg> is not in the free list;
// otherwise, return <reg> and remove it from the free list.
LinearScan::RegState* LinearScan::getReg(RegState* reg) {
  if (reg->isReserved() || reg->isAllocated()) {
    return NULL;
  }
  std::list<RegState*>& freeList = (reg->isCallerSaved() ?
                                    m_freeCallerSaved :
                                    m_freeCalleeSaved);
  freeList.erase(reg->m_pos);
  // Pin it so that other operands in the same instruction will not reuse it.
  reg->m_pinned = true;
  return reg;
}

static bool isUnpinned(LinearScan::RegState* reg) { return !reg->isPinned(); }

LinearScan::RegState* LinearScan::getFreeReg(bool preferCallerSaved) {
  if (m_freeCallerSaved.empty() && m_freeCalleeSaved.empty()) {
    // no free registers --> free the first register in the allocatedRegs
    // list; this register is the one whose last use is the most distant
    ASSERT(!m_allocatedRegs.empty());
    // Pick the first register in <m_allocatedRegs> that is not used
    // for any source operand in the current instruction.
    std::list<RegState*>::iterator pos = std::find_if(m_allocatedRegs.begin(),
                                                      m_allocatedRegs.end(),
                                                      isUnpinned);
    if (pos == m_allocatedRegs.end()) {
      PUNT(RegSpill);
    }
    spill((*pos)->m_ssaTmp);
  }

  std::list<RegState*>* preferred = NULL;
  std::list<RegState*>* other = NULL;
  if (preferCallerSaved) {
    preferred = &m_freeCallerSaved;
    other = &m_freeCalleeSaved;
  } else {
    preferred = &m_freeCalleeSaved;
    other = &m_freeCallerSaved;
  }

  RegState* theFreeReg = NULL;
  if (!preferred->empty()) {
    theFreeReg = popFreeReg(*preferred);
  } else {
    theFreeReg = popFreeReg(*other);
  }
  ASSERT(theFreeReg);
  // Pin it so that other operands in the same instruction will not reuse it.
  theFreeReg->m_pinned = true;
  return theFreeReg;
}

void LinearScan::freeReg(RegState* reg) {
  pushFreeReg(reg);
  // The <tmp> shouldn't be reused any more.
  SSATmp* tmp = reg->m_ssaTmp;
  int32 slotId = tmp->getAnalysisValue();
  if (slotId != -1) {
    m_slots[slotId].m_latestTmp = NULL;
  }
  reg->m_ssaTmp = NULL;
}

void LinearScan::pushFreeReg(RegState* reg) {
  std::list<RegState*>& freeList = (reg->isCallerSaved() ?
                                    m_freeCallerSaved :
                                    m_freeCalleeSaved);
  // If next native is going to use <reg>, put <reg> to the back of the
  // queue so that it's unlikely to be misused by irrelevant tmps.
  if (RuntimeOption::EvalHHIREnablePreColoring &&
      (reg->m_regNo == LinearScan::regNameAsInt(reg::rax) || m_preColoringHint.preColorsTmp(reg))) {
    freeList.push_back(reg);
    reg->m_pos = (--freeList.end());
  } else {
    freeList.push_front(reg);
    reg->m_pos = freeList.begin();
  }
}

LinearScan::RegState* LinearScan::popFreeReg(std::list<RegState*>& freeList) {
  if (freeList.empty()) {
    return NULL;
  }
  RegState* reg = freeList.front();
  freeList.pop_front();
  return reg;
}

void LinearScan::spill(SSATmp* tmp) {
  if (RuntimeOption::EvalDumpIR > 4) {
    std::cout << "--- spilling ";
    tmp->print(std::cout);
    std::cout << "\n";
  }
  // How many registers does <tmp> takes?
  ASSERT(tmp);
  ASSERT(tmp->getNumAssignedLocs() > 0);

  // Free the registers used by <tmp>.
  // Need call freeReg and modify <m_allocatedRegs>.
  for (std::list<RegState*>::iterator it = m_allocatedRegs.begin();
       it != m_allocatedRegs.end(); ) {
    std::list<RegState*>::iterator next = it; ++next;
    RegState* reg = *it;
    if (reg->m_ssaTmp == tmp) {
      freeReg(reg);
      m_allocatedRegs.erase(it);
    }
    it = next;
  }

  if (tmp->getAnalysisValue() == -1) {
    // <tmp> hasn't been spilled before.
    // We need to create a new spill slot for it.
    uint32 slotId = createSpillSlot(tmp);
    // createSpillSlot sets the latest reloaded value of slotId to tmp.
    // Here, we need reset this value because tmp is spilled and no longer
    // synced with memory.
    m_slots[slotId].m_latestTmp = NULL;
  }
}

// Create a spill slot for <tmp>.
uint32 LinearScan::createSpillSlot(SSATmp* tmp) {
  uint32 slotId = m_slots.size();
  tmp->setAnalysisValue(slotId);
  IRInstruction* spillInst = m_irFactory->spill(tmp);
  SlotInfo si;
  si.m_slotTmp = m_irFactory->getSSATmp(spillInst);
  si.m_latestTmp = tmp;
  m_slots.push_back(si);
  // The spill slot inherits the last use ID of the spilled tmp.
  si.m_slotTmp->setLastUseId(tmp->getLastUseId());
  return slotId;
}

IRInstruction* LinearScan::getNextNative() const {
  return m_natives.empty() ? NULL : m_natives.front();
}

uint32 LinearScan::getNextNativeId() const {
  IRInstruction* nextNative = getNextNative();
  return (nextNative ? nextNative->getId() : -1);
}

// The dst of IncRef, Mov, StRef, and StRefNT has the same value
// as the src. For analysis purpose, we put them in one equivalence class.
// This canonicalize function returns the representative of <tmp>'s
// equivalence class. The function computes the representative by
// following the dst-src chain.
SSATmp* LinearScan::canonicalize(SSATmp* tmp) {
  while (true) {
    IRInstruction* inst = tmp->getInstruction();
    Opcode opc = inst->getOpcode();
    // The dst of IncRef, Mov, StRef, and StRefNT has the same value
    // as the src.
    // We follow these instructions to canonicalize an SSATmp.
    if (opc != IncRef && opc != Mov && opc != StRef && opc != StRefNT) {
      return tmp;
    }
    tmp = inst->getSrc(0);
  }
}

SSATmp* LinearScan::getSpilledTmp(SSATmp* tmp) {
  ASSERT(tmp->getInstruction()->getOpcode() == Reload);
  SSATmp* slot = tmp->getInstruction()->getSrc(0);
  ASSERT(slot->getInstruction()->getOpcode() == Spill);
  return slot->getInstruction()->getSrc(0);
}

// If <tmp> is a reloaded value, follow the spill-reload chain to find
// its source; otherwise, return <tmp> itself.
SSATmp* LinearScan::getOrigTmp(SSATmp* tmp) {
  if (tmp->getInstruction()->getOpcode() == Reload)
    return getSpilledTmp(tmp);
  return tmp;
}

void LinearScan::printLoc(std::ostream& os, int loc) {
  if (loc < NumRegs) {
    os << RegNames[loc];
  } else if (loc >= FirstMmxReg && loc < FirstMmxReg + NumMmxRegs) {
    os << "mmx" << loc - FirstMmxReg;
  } else if (loc >= FirstSpill) {
    os << "spill[" << loc - FirstSpill << "]";
  } else {
    ASSERT(false);
  }
}

bool LinearScan::PreColoringHint::preColorsTmp(RegState* reg) const {
  return m_preColoredTmps[reg->m_regNo].first != NULL;
}

// Get the pre-coloring register of (<tmp>, <index>).
// A native call has at most six arguments, so the time complexity is
// not a big problem.
register_name_t LinearScan::PreColoringHint::getPreColoringReg(
    SSATmp* tmp, uint32 index) const {
  for (int regNo = 0; regNo < LinearScan::NumRegs; ++regNo) {
    if (m_preColoredTmps[regNo].first == tmp &&
        m_preColoredTmps[regNo].second == index) {
      return (register_name_t)regNo;
    }
  }
  return reg::noreg;
}

void LinearScan::PreColoringHint::clear() {
  for (int i = 0; i < LinearScan::NumRegs; ++i) {
    m_preColoredTmps[i].first = NULL;
    m_preColoredTmps[i].second = 0;
  }
}

// Provide a hint that (<tmp>, <index>) is used as the <argNum>-th arg
// in next native.
void LinearScan::PreColoringHint::add(SSATmp* tmp, uint32 index, int argNum) {
  int reg = LinearScan::regNameAsInt(CodeGenerator::argNumToRegName[argNum]);
  ASSERT(reg >= 0 && reg < NumRegs);
  m_preColoredTmps[reg].first = tmp;
  m_preColoredTmps[reg].second = index;
}

}}} // HPHP::VM::JIT
