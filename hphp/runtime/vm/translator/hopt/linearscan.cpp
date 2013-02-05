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

#include "runtime/vm/translator/hopt/linearscan.h"
#include "runtime/vm/translator/hopt/irfactory.h"

namespace HPHP {
namespace VM {
namespace JIT{

using namespace Transl::reg;

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

const int NumMmxRegs = 8;

struct LinearScan : private boost::noncopyable {
  static const int NumRegs = 16;

  explicit LinearScan(IRFactory*);
  void allocRegs(Trace*);

private:
  class RegState {
    friend class LinearScan;

  public:
    bool isReserved() const { return m_reserved; }
    bool isCallerSaved() const {
      return kCallerSaved.contains(PhysReg(m_regNo));
    }
    bool isCalleeSaved() const { return !isCallerSaved(); }
    bool isAllocated() const { return m_ssaTmp != NULL; }
    bool isPinned() const { return m_pinned; }
    bool isRetAddr() const {
      if (!m_ssaTmp) return false;
      Type type = m_ssaTmp->getType();
      return type == Type::RetAddr;
    }

  private:
    SSATmp*   m_ssaTmp; // non-null when allocated
    // Maintain the position of this register so that we can quickly
    // remove it from the lists.
    // A non-reserved reg is in either LinearScan::m_freeCallerSaved,
    // LinearScan::m_freeCalleeSaved, or LinearScan::m_allocatedRegs.
    // <m_pos> of a reserved reg is undefined.
    std::list<RegState*>::iterator m_pos;
    uint16_t  m_regNo;
    bool      m_pinned; // do not free this register if pinned
    // We stress test register allocation by reducing the number of
    // free registers.
    // <m_reserved> is true if the register is a reserved register
    // (i.e., rbx, rsp, rbp, r10, and r12) or it is marked as not free for
    // stress testing.
    bool      m_reserved;
  };

  struct SlotInfo {
    // the SSATmp that represents this spill location
    SSATmp* m_spillTmp;
    // The latest SSATmp that has the most recent reloaded spilled value
    // If it's NULL, we have to reload this slot before using it.
    SSATmp* m_latestReload;
  };

  class PreColoringHint {
  public:
    PreColoringHint() { clear(); }
    bool preColorsTmp(RegState* reg) const;
    RegNumber getPreColoringReg(SSATmp* tmp, uint32 index) const;
    void clear();
    void add(SSATmp* tmp, uint32 index, int argNum);
  private:
    // indexed by arg number
    std::pair<SSATmp*, uint32> m_preColoredTmps[LinearScan::NumRegs];
  };

private:
  void allocRegToInstruction(IRInstruction::Iterator it);
  void allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index);
  void allocRegToTmp(SSATmp* ssaTmp, uint32_t index);
  void freeRegsAtId(uint32_t id);
  void spill(SSATmp* tmp);
  void computeLiveOutRegs();
  static RegSet computeLiveOutRegs(IRInstruction* inst, RegSet liveRegs);

  void initFreeList();
  void coalesce(Trace* trace);
  void coalesceAux(Trace* trace);
  void allocRegsToTrace();
  uint32 createSpillSlot(SSATmp* tmp);
  static SSATmp* getSpilledTmp(SSATmp* tmp);
  static SSATmp* getOrigTmp(SSATmp* tmp);
  void preAllocSpillLoc(uint32 numSpillLocs);
  uint32 assignSpillLoc();
  void insertAllocFreeSpill(Trace* trace, uint32 numExtraSpillLocs);
  void insertAllocFreeSpillAux(Trace* trace, uint32 numExtraSpillLocs);
  void rematerialize(Trace* trace);
  void rematerializeAux();
  void removeUnusedSpills(Trace* trace);
  void removeUnusedSpillsAux(Trace* trace);
  void collectNatives();
  void computePreColoringHint();
  IRInstruction* getNextNative() const;
  uint32 getNextNativeId() const;

  void pushFreeReg(RegState* reg);
  RegState* popFreeReg(std::list<RegState*>& freeList);
  void freeReg(RegState* reg);
  RegState* getFreeReg(bool preferCallerSaved);
  RegState* getReg(RegState* reg);

private:
  // Register allocation may generate Spill/Reload.
  IRFactory* const m_irFactory;
  RegState   m_regs[NumRegs];
  // Lists of free caller and callee-saved registers, respectively.
  std::list<RegState*> m_freeCallerSaved;
  std::list<RegState*> m_freeCalleeSaved;
  // List of assigned registers, sorted high to low by lastUseId.
  std::list<RegState*> m_allocatedRegs;
  // Indexed by slot ID.
  std::vector<SlotInfo> m_slots;
  // All basic blocks in linearized order.
  BlockList m_blocks;
  // the list of native instructions in the trace sorted by instruction ID;
  // i.e. a filtered list in the same order as visited by m_blocks.
  IRInstruction::List m_natives;
  // stores pre-coloring hints
  PreColoringHint m_preColoringHint;
};

// This value must be consistent with the number of pre-allocated
// bytes for spill locations in __enterTCHelper in translator-x64.cpp.
// Be careful when changing this value.
static const int NumPreAllocatedSpillLocs = 16;
static_assert(kReservedRSPSpillSpace == NumPreAllocatedSpillLocs * sizeof(void*),
              "kReservedRSPSpillSpace changes require updates in "
              "LinearScan");

// The dst of IncRef, Mov, StRef, and StRefNT has the same value
// as the src. For analysis purpose, we put them in one equivalence class.
// This canonicalize function returns the representative of <tmp>'s
// equivalence class. The function computes the representative by
// following the dst-src chain.
static SSATmp* canonicalize(SSATmp* tmp) {
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

LinearScan::LinearScan(IRFactory* irFactory)
  : m_irFactory(irFactory)
{
  for (int i = 0; i < kNumX64Regs; i++) {
    m_regs[i].m_ssaTmp = NULL;
    m_regs[i].m_regNo = i;
    m_regs[i].m_pinned = false;
    m_regs[i].m_reserved = false;
  }

  // Mark reserved regs.
  m_regs[int(rVmSp)]   .m_reserved = true;
  m_regs[int(rsp)]     .m_reserved = true;
  m_regs[int(rVmFp)]   .m_reserved = true;
  m_regs[int(rScratch)].m_reserved = true;
  m_regs[int(rVmTl)]   .m_reserved = true;

  // Reserve extra regs for testing purpose.
  uint32 numFreeRegs = RuntimeOption::EvalHHIRNumFreeRegs;
  for (int i = kNumX64Regs - 1; i >= 0; i--) {
    if (!m_regs[i].m_reserved) {
      if (numFreeRegs == 0) {
        m_regs[i].m_reserved = true;
      } else {
        --numFreeRegs;
      }
    }
  }
}

RegSet LinearScan::computeLiveOutRegs(IRInstruction* inst, RegSet liveRegs) {
  uint32 instId = inst->getId();
  for (SSATmp* src : inst->getSrcs()) {
    if (src->getLastUseId() <= instId) {
      for (int locIndex = 0;
           locIndex < src->numAllocatedRegs();
           ++locIndex) {
        if (src->hasReg(locIndex)) {
          // inst is the last use of the register assigned to this SSATmp
          // remove src reg from live regs set
          liveRegs.remove(src->getReg(locIndex));
        }
      }
    }
  }
  // add the destination registers to the live regs set
  for (SSATmp* dst : inst->getDsts()) {
    if (dst->getLastUseId() > instId) {
      for (int locIndex = 0;
           locIndex < dst->numAllocatedRegs();
           locIndex++) {
        if (dst->hasReg(locIndex)) {
          liveRegs.add(dst->getReg(locIndex));
        }
      }
    }
  }
  inst->setLiveOutRegs(liveRegs);
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
void LinearScan::computeLiveOutRegs() {
  RegSet liveOutRegs;
  for (Block& block : m_blocks) {
    for (IRInstruction* inst : block) {
      liveOutRegs = LinearScan::computeLiveOutRegs(inst, liveOutRegs);
    }
  }
}

void LinearScan::allocRegToInstruction(IRInstruction::Iterator it) {
  IRInstruction* inst = *it;
  if (RuntimeOption::EvalDumpIR > 3) {
    std::cout << "--- allocating to instruction: ";
    inst->print(std::cout);
    std::cout << std::endl;
  }

  // Reload all source operands if necessary.
  // Mark registers as unpinned.
  for (int regNo = 0; regNo < kNumX64Regs; ++regNo) {
    m_regs[regNo].m_pinned = false;
  }
  std::vector<bool> needsReloading(inst->getNumSrcs(), true);
  for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
    SSATmp* tmp = inst->getSrc(i);
    int32 slotId = tmp->getSpillSlot();
    if (slotId == -1) {
      needsReloading[i] = false;
    } else if ((tmp = m_slots[slotId].m_latestReload)) {
      needsReloading[i] = false;
      inst->setSrc(i, tmp);
    }
    if (!needsReloading[i]) {
      for (int locIndex = 0;
           locIndex < tmp->numAllocatedRegs();
           ++locIndex) {
        auto srcReg = tmp->getReg(locIndex);
        m_regs[int(srcReg)].m_pinned = true;
      }
    }
  }
  for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
    if (needsReloading[i]) {
      SSATmp* tmp = inst->getSrc(i);
      int32 slotId = tmp->getSpillSlot();
      // <tmp> is spilled, and not reloaded.
      // Therefore, We need to reload the value into a new SSATmp.

      // Insert the Reload instruction.
      SSATmp* spillTmp = m_slots[slotId].m_spillTmp;
      IRInstruction* reload = m_irFactory->gen(Reload, spillTmp);
      Trace* trace = inst->getParent();
      trace->getInstructionList().insert(it, reload);
      reload->setParent(trace);

      // Create <reloadTmp> which inherits <tmp>'s slot ID and
      // <spillTmp>'s last use ID.
      // Replace <tmp> with <reloadTmp> in <inst>.
      SSATmp* reloadTmp = reload->getDst();
      reloadTmp->setLastUseId(spillTmp->getLastUseId());
      reloadTmp->setSpillSlot(slotId);
      inst->setSrc(i, reloadTmp);
      // reloadTmp and tmp share the same type.  Since it was spilled, it
      // must be using its entire needed-count of registers.
      assert(reloadTmp->getType() == tmp->getType());
      assert(tmp->numNeededRegs() == tmp->numAllocatedRegs());
      for (int locIndex = 0; locIndex < tmp->numNeededRegs(); ++locIndex) {
        allocRegToTmp(reloadTmp, locIndex);
      }
      // Remember this reload tmp in case we can reuse it in later blocks.
      m_slots[slotId].m_latestReload = reloadTmp;
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
    assert(!m_natives.empty());
    m_natives.pop_front();
    computePreColoringHint();
  }

  SSARange dsts = inst->getDsts();
  if (dsts.empty()) return;

  Opcode opc = inst->getOpcode();
  if (opc == DefFP || opc == FreeActRec) {
    allocRegToTmp(&m_regs[int(rVmFp)], dsts[0], 0);
    return;
  }
  if (opc == DefSP || opc == Call || opc == SpillStack ||
      opc == RetAdjustStack ||
      opc == NewObj || opc == InterpOne || opc == GenericRetDecRefs ||
      opc == GuardStk || opc == AssertStk) {
    assert(dsts[0]->getType() == Type::StkPtr);
    allocRegToTmp(&m_regs[int(rVmSp)], dsts[0], 0);
    return;
  }
  if (opc == DefMIStateBase) {
    assert(dsts[0]->getType() == Type::PtrToCell);
    allocRegToTmp(&m_regs[int(rsp)], dsts[0], 0);
    return;
  }

  // LdRaw, loading a generator's embedded AR, is the only time we have a
  // pointer to an AR that is not in rVmFp or rVmSp.
  assert(dsts[0]->getType() != Type::StkPtr ||
         (opc == LdRaw &&
          inst->getSrc(1)->getValInt() == RawMemSlot::ContARPtr));

  for (SSATmp* dst : dsts) {
    for (int i = 0; i < dst->numNeededRegs(); ++i) {
      if (!RuntimeOption::EvalHHIRDeadCodeElim || dst->getLastUseId() != 0) {
        allocRegToTmp(dst, i);
      }
    }
  }
  if (!RuntimeOption::EvalHHIRDeadCodeElim) {
    // if any outputs were unused, free regs now.
    freeRegsAtId(inst->getId());
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
    RegNumber targetRegNo =
      m_preColoringHint.getPreColoringReg(getOrigTmp(ssaTmp), index);
    if (targetRegNo != reg::noreg) {
      reg = getReg(&m_regs[int(targetRegNo)]);
    }
  }
  if (reg == NULL &&
      RuntimeOption::EvalHHIREnablePreColoring &&
      ssaTmp->getInstruction()->isNative()) {
    // Pre-colors ssaTmp if it's the return value of a native.
    if (index == 0) {
      reg = getReg(&m_regs[int(rax)]);
    } else if (index == 1) {
      reg = getReg(&m_regs[int(rdx)]);
    } else {
      not_reached();
    }
  }
  if (reg == NULL) {
    // No pre-coloring for this tmp.
    // Pick a regular caller-saved reg.
    reg = getFreeReg(true);
  }

  assert(reg);
  if (!preferCallerSaved && reg->isCallerSaved()) {
    // ssaTmp spans native, but we failed to find a free callee-saved reg.
    // We eagerly add a spill ssaTmp, and update ssaTmp's live range
    // to end with next native, because we know we have to spill it at
    // the next native.
    // Setting the last use ID to the next native is conservative.
    // Setting it to the last use before the next native would be more precise,
    // but that would be more expensive to compute.
    if (ssaTmp->getSpillSlot() == -1) {
      createSpillSlot(ssaTmp);
    }
    ssaTmp->setLastUseId(getNextNativeId());
  }

  allocRegToTmp(reg, ssaTmp, index);
}

void LinearScan::allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index) {
  reg->m_ssaTmp = ssaTmp;
  // mark inst as using this register
  ssaTmp->setReg(PhysReg(reg->m_regNo), index);
  uint32_t lastUseId = ssaTmp->getLastUseId();
  if (reg->isReserved()) {
    return;
  }
  // insert into the list of assigned registers sorted by last use id
  std::list<RegState*>::iterator it = m_allocatedRegs.begin();
  for (; it != m_allocatedRegs.end(); ++it) {
    if (lastUseId > (*it)->m_ssaTmp->getLastUseId()) {
      break;
    }
  }
  reg->m_pos = m_allocatedRegs.insert(it, reg);
}

// Assign spill location numbers to Spill/Reload.
uint32 LinearScan::assignSpillLoc() {
  uint32 nextSpillLoc = 0;
  uint32 nextMmxReg = 0;

  // visit blocks in reverse postorder and instructions in forward order,
  // assigning a spill slot id or mmx register number to each Spill.
  // We don't reuse slot id's or mmx registers, but both could be reused
  // either by visiting the dominator tree in preorder or by analyzing
  // lifetimes and reusing id/registers between non-conflicting spills.

  for (Block& block : m_blocks) {
    for (IRInstruction* inst : block) {
      if (getNextNative() == inst) {
        assert(!m_natives.empty());
        m_natives.pop_front();
      }
      if (inst->getOpcode() == Spill) {
        SSATmp* dst = inst->getDst();
        SSATmp* src = inst->getSrc(0);
        for (int locIndex = 0;
             locIndex < src->numNeededRegs();
             ++locIndex) {
          if (dst->getLastUseId() <= getNextNativeId()) {
            TRACE(3, "[counter] 1 spill a tmp that does not span native\n");
          } else {
            TRACE(3, "[counter] 1 spill a tmp that spans native\n");
          }

          const bool allowMmxSpill = RuntimeOption::EvalHHIREnableMmx &&
            // The live range of the spill slot doesn't span native calls,
            // and we still have free MMX registers.
            dst->getLastUseId() <= getNextNativeId() &&
            nextMmxReg < (uint32)NumMmxRegs;

          dst->setSpillInfo(locIndex,
            allowMmxSpill
              ? SpillInfo(RegNumber(nextMmxReg++))
              : SpillInfo(nextSpillLoc++)
          );
          if (allowMmxSpill) {
            TRACE(3, "[counter] 1 spill to mmx\n");
          } else {
            TRACE(3, "[counter] 1 spill to memory\n");
          }
        }
      }
      if (inst->getOpcode() == Reload) {
        SSATmp* src = inst->getSrc(0);
        for (int locIndex = 0;
             locIndex < src->numNeededRegs();
             ++locIndex) {
          if (src->getSpillInfo(locIndex).type() == SpillInfo::MMX) {
            TRACE(3, "[counter] reload from mmx\n");
          } else {
            TRACE(3, "[counter] reload from memory\n");
          }
        }
      }
    }
  }
  return nextSpillLoc;
}

void LinearScan::insertAllocFreeSpill(Trace* trace, uint32 numExtraSpillLocs) {
  insertAllocFreeSpillAux(trace, numExtraSpillLocs);
  for (Trace* exit : trace->getExitTraces()) {
    insertAllocFreeSpillAux(exit, numExtraSpillLocs);
  }
}

void LinearScan::insertAllocFreeSpillAux(Trace* trace,
                                         uint32 numExtraSpillLocs) {
  SSATmp* tmp = m_irFactory->defConst((int64)numExtraSpillLocs)->getDst();

  IRInstruction::List& instList = trace->getInstructionList();
  for (IRInstruction::Iterator it = instList.begin();
       it != instList.end(); ) {
    IRInstruction::Iterator next = it; ++next;
    IRInstruction* inst = *it;
    Opcode opc = inst->getOpcode();
    if (opc == Call) {
      // Insert FreeSpill and AllocSpill around each Call.
      IRInstruction* allocSpill = m_irFactory->gen(AllocSpill, tmp);
      IRInstruction* freeSpill = m_irFactory->gen(FreeSpill, tmp);
      instList.insert(it, freeSpill);
      freeSpill->setParent(trace);
      instList.insert(next, allocSpill);
      allocSpill->setParent(trace);
    } else if (opc == ExitTrace || opc == ExitSlow || opc == ExitTraceCc ||
               opc == ExitSlowNoProgress || opc == ExitGuardFailure ||
               opc == LdRetAddr) {
      // Insert FreeSpill at trace exits.
      IRInstruction* freeSpill = m_irFactory->gen(FreeSpill, tmp);
      instList.insert(it, freeSpill);
      freeSpill->setParent(trace);
    }
    it = next;
  }

  // Insert AllocSpill at the start of the main trace.
  if (trace->isMain()) {
    trace->prependInstruction(
      m_irFactory->gen(AllocSpill, tmp));
  }
}

void LinearScan::collectNatives() {
  // May be re-executed. Need initialize <m_natives> each time.
  m_natives.clear();
  for (Block& block : m_blocks) {
    for (IRInstruction* inst : block) {
      if (inst->isNative()) m_natives.push_back(inst);
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

  // For instructions that want to hint a continuous increasing range
  // of sources to a continuous increasing range of argument
  // registers.
  auto normalHint = [&](int count, int srcBase = 0, int argBase = 0) {
    for (int i = 0; i < count; ++i) {
      m_preColoringHint.add(nextNative->getSrc(i + srcBase), 0,
                            i + argBase);
    }
  };
  switch (nextNative->getOpcode()) {
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
    case Print:
      m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
      break;
    case AddElem:
      if (nextNative->getSrc(1)->getType() == Type::Int &&
          nextNative->getSrc(2)->getType() == Type::Int) {
        normalHint(3, 0, 1);
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
        Type lType = nextNative->getSrc(0)->getType();
        Type rType = nextNative->getSrc(1)->getType();
        if ((lType.isString() && rType.isString()) ||
            (lType.isString() && rType == Type::Int) ||
            (lType == Type::Int && rType.isString())) {
          m_preColoringHint.add(nextNative->getSrc(0), 0, 0);
          m_preColoringHint.add(nextNative->getSrc(1), 0, 1);
        } else {
          m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
          m_preColoringHint.add(nextNative->getSrc(1), 0, 3);
        }
      }
      break;
    case ArrayAdd:
      normalHint(2);
      break;
    case AKExists:
      normalHint(2);
      break;
    case DefFunc:
      normalHint(1);
      break;
    case CreateCont:
      normalHint(4);
      break;
    case FillContLocals:
      normalHint(4);
      break;
    case OpEq:
    case OpNeq:
    case OpSame:
    case OpNSame:
      {
        auto src1 = nextNative->getSrc(0);
        auto src2 = nextNative->getSrc(1);

        auto type1 = src1->getType();
        auto type2 = src2->getType();

        if ((type1 == Type::Arr && type2 == Type::Arr)
            || (type1.isString() && type2.isString())
            || (type1.isString() && !src1->isConst())
            || (type1 == Type::Obj && type2 == Type::Obj)) {
          m_preColoringHint.add(src1, 0, 0);
          m_preColoringHint.add(src2, 0, 1);
        }
      }
      break;
    case IterInit:
    {
      m_preColoringHint.add(nextNative->getSrc(0), 0, 1);
    }
    break;
    case Conv:
    {
      SSATmp* src = nextNative->getSrc(0);
      Type toType = nextNative->getTypeParam();
      Type fromType = src->getType();
      if (toType == Type::Bool) {
        if (fromType == Type::Cell) {
          m_preColoringHint.add(src, 0, 0);
          m_preColoringHint.add(src, 1, 1);
        } else if (fromType == Type::Str ||
                   fromType == Type::StaticStr ||
                   fromType == Type::Arr ||
                   fromType == Type::Obj) {
          m_preColoringHint.add(src, 0, 0);
        }
      } else if (fromType.isString() && toType == Type::Int) {
        m_preColoringHint.add(src, 0, 0);
      }
      break;
    }
    case PropX: {
      unsigned arg = 0;
      m_preColoringHint.add(nextNative->getSrc(1), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(2), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(3), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(3), 1, arg++);
      m_preColoringHint.add(nextNative->getSrc(4), 0, arg++);
      assert(arg == 5);
      break;
    }
    case CGetProp: {
      unsigned arg = 0;
      m_preColoringHint.add(nextNative->getSrc(1), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(2), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(3), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(3), 1, arg++);
      m_preColoringHint.add(nextNative->getSrc(4), 0, arg++);
      assert(arg == 5);
      break;
    }
    case CGetElem: {
      unsigned arg = 0;
      m_preColoringHint.add(nextNative->getSrc(1), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(2), 0, arg++);
      m_preColoringHint.add(nextNative->getSrc(2), 1, arg++);
      m_preColoringHint.add(nextNative->getSrc(3), 0, arg++);
      assert(arg == 4);
      break;
    }
    case InstanceOf:
    case NInstanceOf:
    case JmpInstanceOf:
    case JmpNInstanceOf:
      normalHint(2);
      break;
    default:
      break;
  }
}

// Create the initial free list.
// It must be called after computePreColoringHint, because the order of
// caller-saved regs depends on pre-coloring hints.
void LinearScan::initFreeList() {
  // reserve extra regs for testing purpose.
  for (int i = kNumX64Regs - 1; i >= 0; i--) {
    if (!m_regs[i].m_reserved) {
      pushFreeReg(&m_regs[i]);
    }
  }
}

void LinearScan::coalesce(Trace* trace) {
  coalesceAux(trace);
  for (Trace* exit : trace->getExitTraces()) {
    coalesceAux(exit);
  }
}

void LinearScan::coalesceAux(Trace* trace) {
  for (auto* inst : trace->getInstructionList()) {
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

void LinearScan::preAllocSpillLoc(uint32 numSpillLocs) {
  for (Block& block : m_blocks) {
    for (IRInstruction* inst : block) {
      if (inst->getOpcode() == Spill) {
        SSATmp* dst = inst->getDst();
        for (int index = 0; index < dst->numNeededRegs(); ++index) {
          assert(!dst->hasReg(index));
          if (dst->getSpillInfo(index).type() == SpillInfo::Memory) {
            uint32 spillLoc = dst->getSpillInfo(index).mem();
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
              dst->setSpillInfo(index, SpillInfo(spillLoc + 1));
            }
          }
        }
      }
    }
  }
}

void LinearScan::allocRegs(Trace* trace) {
  if (RuntimeOption::EvalHHIREnableCoalescing) {
    // <coalesce> doesn't need instruction numbering.
    coalesce(trace);
  }

  m_blocks = numberInstructions(trace);

  collectNatives();
  computePreColoringHint();
  initFreeList();
  allocRegsToTrace();
  // Renumber instructions, because we added spills and reloads.
  m_blocks = numberInstructions(trace);

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
  collectNatives();
  // Make sure rsp is 16-aligned.
  uint32 numSpillLocs = assignSpillLoc();
  if (numSpillLocs % 2) {
    ++numSpillLocs;
  }
  assert(NumPreAllocatedSpillLocs % 2 == 0);
  if (numSpillLocs > 0) {
    preAllocSpillLoc(numSpillLocs);
    if (numSpillLocs > (uint32)NumPreAllocatedSpillLocs) {
      /*
       * We only insert AllocSpill and FreeSpill when the pre-allocated
       * spill locations are not enough.
       *
       * AllocSpill and FreeSpill take the number of extra spill locations
       * besides the pre-allocated ones.
       *
       * TODO(#2044051) AllocSpill/FreeSpill are currently disabled
       * due to bugs.
       */
      PUNT(LinearScan_AllocSpill);
      insertAllocFreeSpill(trace, numSpillLocs - NumPreAllocatedSpillLocs);
    }
  }
  m_blocks = numberInstructions(trace);

  // record the live out register set at each instruction
  computeLiveOutRegs();
}

void LinearScan::allocRegsToTrace() {
  // First, visit every instruction, allocating registers as we go,
  // and inserting Reload instructions where necessary.
  for (Block& block : m_blocks) {
    // Clear the remembered reloads because the previous block might not
    // dominate this block.
    for (SlotInfo& slot : m_slots) {
      slot.m_latestReload = nullptr;
    }
    for (IRInstruction::Iterator it = block.begin();
         it != block.end(); ++it) {
      allocRegToInstruction(it);
      if (RuntimeOption::EvalDumpIR > 3) {
        std::cout << "--- allocated to instruction: ";
        (*it)->print(std::cout);
        std::cout << "\n";
      }
    }
  }

  // Now that we have visited all instructions and inserted Reloads
  // for SSATmps which needed to be spilled, we can go back and insert
  // the spills.  All uses must have been visited before we do this.
  for (Block& block : m_blocks) {
    for (IRInstruction::Iterator it = block.begin(), next;
         it != block.end(); it = next) {
      next = it; ++next;
      IRInstruction* inst = *it;
      // Reloaded SSATmps needn't be spilled again, so ignore Reloads.
      if (inst->getOpcode() == Reload) continue;
      for (SSATmp* dst : inst->getDsts()) {
        int32 slotId = dst->getSpillSlot();
        if (slotId != -1) {
          // If this instruction is marked to be spilled, append a spill.
          IRInstruction* spillInst =
            m_slots[slotId].m_spillTmp->getInstruction();
          Trace* trace = (*next)->getParent();
          trace->getInstructionList().insert(next, spillInst);
          spillInst->setParent(trace);
        }
      }
    }
  }
}

void LinearScan::rematerialize(Trace* trace) {
  rematerializeAux();
  m_blocks = numberInstructions(trace);

  // We only replaced Reloads in <rematerializeAux>.
  // Here, we remove Spills that are never reloaded.
  removeUnusedSpills(trace);
  m_blocks = numberInstructions(trace);
}

void LinearScan::rematerializeAux() {
  std::vector<SSATmp*> localValues;
  auto killLocal = [&](int loc) {
    if (localValues.size() > loc) {
      localValues[loc] = nullptr;
    }
  };

  for (Block& block : m_blocks) {
    // only rematerialize within a block since previous block might
    // not dominate this one.  TODO: t2068637.
    SSATmp* curSp = nullptr;
    SSATmp* curFp = nullptr;
    localValues.clear();
    for (IRInstruction::Iterator it = block.begin(); it != block.end(); ++it) {
      IRInstruction* inst = *it;
      Opcode opc = inst->getOpcode();
      if (opc == DefFP || opc == FreeActRec) {
        assert(inst->getDst()->getReg() == rVmFp);
        curFp = inst->getDst();
      }
      else if (opc == Reload) {
        // s = Spill t0
        // t = Reload s
        SSATmp* dst = inst->getDst();
        SSATmp* spilledTmp = getSpilledTmp(dst);
        IRInstruction* spilledInst = spilledTmp->getInstruction();
        IRInstruction* newInst = NULL;
        if (spilledInst->isRematerializable() ||
            (spilledInst->getOpcode() == LdStack &&
             spilledInst->getSrc(0) == curSp)) {
          // XXX: could change <newInst> to the non-check version.
          // Rematerialize those rematerializable instructions (i.e.,
          // isRematerializable returns true) and LdStack.
          newInst = spilledInst->clone(m_irFactory);
          // The new instruction needn't have an exit label; it must always
          // be dominated by the original instruction because reloads are
          // inserted just before uses, which must be dominated by the
          // original (spilled) def.
          newInst->setLabel(NULL);
        } else if (curFp) {
          // Rematerialize LdLoc.
          std::vector<SSATmp*>::iterator pos =
            std::find(localValues.begin(),
                      localValues.end(),
                      canonicalize(spilledTmp));
          // Search for a local that stores the value of <spilledTmp>.
          if (pos != localValues.end()) {
            size_t locId = pos - localValues.begin();
            assert(curFp != NULL);
            SSATmp* srcs[] = { curFp };
            ConstInstruction constInst(1, srcs, Local(locId));
            IRInstruction* ldHome = m_irFactory->cloneInstruction(&constInst);
            newInst = m_irFactory->gen(LdLoc, dst->getType(), ldHome->getDst());
          }
        }
        if (newInst) {
          UNUSED Type oldType = dst->getType();
          newInst->setDst(dst);
          dst->setInstruction(newInst);
          assert(outputType(newInst) == oldType);
          *it = newInst;
          newInst->setParent(inst->getParent());
        }
      }

      // Updating curSp and localValues
      if (inst->hasDst() && inst->getDst()->getReg() == rVmSp) {
        // inst modifies the stack pointer.
        curSp = inst->getDst();
      }

      if (opc == LdLoc || opc == StLoc || opc == StLocNT) {
        // dst = LdLoc home
        // StLoc/StLocNT home, src
        int locId = getLocalIdFromHomeOpnd(inst->getSrc(0));
        // Note that when we implement inlining, we will need to deal
        // with the new local id space of the inlined function.
        SSATmp* localValue = (opc == LdLoc ? inst->getDst() : inst->getSrc(1));
        if (int(localValues.size()) < locId + 1) {
          localValues.resize(locId + 1);
        }
        localValues[locId] = canonicalize(localValue);
      }
      // Other instructions that may have side effects on locals must
      // kill the local variable values.
      else if (opc == IterInit) {
        int valLocId = inst->getSrc(3)->getValInt();
        killLocal(valLocId);
        if (inst->getNumSrcs() == 5) {
          int keyLocId = inst->getSrc(4)->getValInt();
          killLocal(keyLocId);
        }
      } else if (opc == IterNext) {
        int valLocId = inst->getSrc(2)->getValInt();
        killLocal(valLocId);
        if (inst->getNumSrcs() == 4) {
          int keyLocId = inst->getSrc(3)->getValInt();
          killLocal(keyLocId);
        }
      }
    }
  }
}

void LinearScan::removeUnusedSpills(Trace* trace) {
  removeUnusedSpillsAux(trace);
  for (Trace* exit : trace->getExitTraces()) {
    removeUnusedSpillsAux(exit);
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
        // Not all instructions are able to take noreg as its dest
        // reg.  We pick LdLoc and IncRef because they occur often.
        if (srcOpc == IncRef || srcOpc == LdLoc) {
          for (int locIndex = 0;
               locIndex < src->numNeededRegs();
               ++locIndex) {
            src->setReg(InvalidReg, locIndex);
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
    assert(reg->m_ssaTmp);
    if (reg->m_ssaTmp->getLastUseId() <= id) {
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

LinearScan::RegState* LinearScan::getFreeReg(bool preferCallerSaved) {
  if (m_freeCallerSaved.empty() && m_freeCalleeSaved.empty()) {
    // no free registers --> free the first register in the allocatedRegs
    // list; this register is the one whose last use is the most distant
    assert(!m_allocatedRegs.empty());

    // Pick the first register in <m_allocatedRegs> that is:
    // 1. not used for any source operand in the current instruction, and
    // 2. not used for the return address of a function.
    auto canSpill = [&] (RegState* reg) {
      return !reg->isPinned() && !reg->isRetAddr();
    };
    auto pos = std::find_if(m_allocatedRegs.begin(), m_allocatedRegs.end(),
                            canSpill);
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
  assert(theFreeReg);
  // Pin it so that other operands in the same instruction will not reuse it.
  theFreeReg->m_pinned = true;
  return theFreeReg;
}

void LinearScan::freeReg(RegState* reg) {
  pushFreeReg(reg);
  // The <tmp> shouldn't be reused any more.
  SSATmp* tmp = reg->m_ssaTmp;
  int32 slotId = tmp->getSpillSlot();
  if (slotId != -1) {
    m_slots[slotId].m_latestReload = NULL;
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
      (reg->m_regNo == int(rax) || m_preColoringHint.preColorsTmp(reg))) {
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
  // If we're spilling, we better actually have registers allocated.
  assert(tmp->numAllocatedRegs() > 0);
  assert(tmp->numAllocatedRegs() == tmp->numNeededRegs());

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

  if (tmp->getSpillSlot() == -1) {
    // <tmp> hasn't been spilled before.
    // We need to create a new spill slot for it.
    uint32 slotId = createSpillSlot(tmp);
    // createSpillSlot sets the latest reloaded value of slotId to tmp.
    // Here, we need reset this value because tmp is spilled and no longer
    // synced with memory.
    m_slots[slotId].m_latestReload = NULL;
  }
}

// Create a spill slot for <tmp>.
uint32 LinearScan::createSpillSlot(SSATmp* tmp) {
  uint32 slotId = m_slots.size();
  tmp->setSpillSlot(slotId);
  IRInstruction* spillInst = m_irFactory->gen(Spill, tmp);
  SSATmp* spillTmp = spillInst->getDst();
  SlotInfo si;
  si.m_spillTmp = spillTmp;
  si.m_latestReload = tmp;
  m_slots.push_back(si);
  // The spill slot inherits the last use ID of the spilled tmp.
  si.m_spillTmp->setLastUseId(tmp->getLastUseId());
  return slotId;
}

IRInstruction* LinearScan::getNextNative() const {
  return m_natives.empty() ? NULL : m_natives.front();
}

uint32 LinearScan::getNextNativeId() const {
  IRInstruction* nextNative = getNextNative();
  return (nextNative ? nextNative->getId() : -1);
}

SSATmp* LinearScan::getSpilledTmp(SSATmp* tmp) {
  assert(tmp->getInstruction()->getOpcode() == Reload);
  SSATmp* slot = tmp->getInstruction()->getSrc(0);
  assert(slot->getInstruction()->getOpcode() == Spill);
  return slot->getInstruction()->getSrc(0);
}

// If <tmp> is a reloaded value, follow the spill-reload chain to find
// its source; otherwise, return <tmp> itself.
SSATmp* LinearScan::getOrigTmp(SSATmp* tmp) {
  if (tmp->getInstruction()->getOpcode() == Reload)
    return getSpilledTmp(tmp);
  return tmp;
}

bool LinearScan::PreColoringHint::preColorsTmp(RegState* reg) const {
  return m_preColoredTmps[reg->m_regNo].first != NULL;
}

// Get the pre-coloring register of (<tmp>, <index>).
// A native call has at most six arguments, so the time complexity is
// not a big problem.
RegNumber LinearScan::PreColoringHint::getPreColoringReg(
    SSATmp* tmp, uint32 index) const {
  for (int regNo = 0; regNo < kNumX64Regs; ++regNo) {
    if (m_preColoredTmps[regNo].first == tmp &&
        m_preColoredTmps[regNo].second == index) {
      return (RegNumber)regNo;
    }
  }
  return reg::noreg;
}

void LinearScan::PreColoringHint::clear() {
  for (int i = 0; i < kNumX64Regs; ++i) {
    m_preColoredTmps[i].first = NULL;
    m_preColoredTmps[i].second = 0;
  }
}

// Provide a hint that (<tmp>, <index>) is used as the <argNum>-th arg
// in next native.
void LinearScan::PreColoringHint::add(SSATmp* tmp, uint32 index, int argNum) {
  int reg = int(argNumToRegName[argNum]);
  assert(reg >= 0 && reg < kNumX64Regs);
  m_preColoredTmps[reg].first = tmp;
  m_preColoredTmps[reg].second = index;
}

//////////////////////////////////////////////////////////////////////

void allocRegsForTrace(Trace* trace, IRFactory* irFactory) {
  LinearScan(irFactory).allocRegs(trace);
}

}}} // HPHP::VM::JIT
