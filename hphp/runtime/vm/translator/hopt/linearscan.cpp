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

#include "hphp/runtime/vm/translator/hopt/linearscan.h"

#include "hphp/runtime/vm/translator/hopt/irfactory.h"
#include "hphp/runtime/vm/translator/hopt/nativecalls.h"
#include "hphp/runtime/vm/translator/hopt/print.h"
#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/tracebuilder.h"
#include "hphp/runtime/vm/translator/hopt/codegen.h"
#include "hphp/runtime/vm/translator/hopt/state_vector.h"
#include "hphp/runtime/vm/translator/physreg.h"
#include "hphp/runtime/vm/translator/abi-x64.h"
#include <boost/noncopyable.hpp>

namespace HPHP {
namespace JIT{

using namespace Transl::reg;

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

int RegisterInfo::numAllocatedRegs() const {
  // Return the number of register slots that actually have an allocated
  // register or spill slot.  We may not have allocated a full numNeededRegs()
  // worth of registers in some cases (if the value of this tmp wasn't used).
  // We rely on InvalidReg (-1) never being equal to a spill slot number.
  int i = 0;
  while (i < kMaxNumRegs && m_regs[i] != InvalidReg) {
    ++i;
  }
  return i;
}

RegSet RegisterInfo::getRegs() const {
  RegSet regs;
  for (int i = 0, n = numAllocatedRegs(); i < n; ++i) {
    if (hasReg(i)) regs.add(getReg(i));
  }
  return regs;
}

struct LinearScan : private boost::noncopyable {
  static const int NumRegs = 16;

  explicit LinearScan(IRFactory*);
  RegAllocInfo allocRegs(Trace*, LifetimeInfo*);

private:
  class RegState {
    friend class LinearScan;

  public:
    bool isReserved() const { return m_reserved; }
    bool isCallerSaved() const {
      return kCallerSaved.contains(PhysReg(m_regNo));
    }
    bool isCalleeSaved() const { return !isCallerSaved(); }
    bool isAllocated() const { return m_ssaTmp != nullptr; }
    bool isPinned() const { return m_pinned; }
    bool isRetAddr() const {
      if (!m_ssaTmp) return false;
      Type type = m_ssaTmp->type();
      return type == Type::RetAddr;
    }

  private:
    SSATmp*   m_ssaTmp; // non-null when allocated
    // Maintain the position of this register so that we can quickly
    // remove it from the lists.
    // A non-reserved reg is in either LinearScan::m_freeCallerSaved,
    // LinearScan::m_freeCalleeSaved, or LinearScan::m_allocatedRegs.
    // <m_pos> of a reserved reg is undefined.
    smart::list<RegState*>::iterator m_pos;
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
    SSATmp* spillTmp;
    // The latest SSATmp that has the most recent reloaded spilled value
    // If it's NULL, we have to reload this slot before using it.
    SSATmp* latestReload;
  };

  class PreColoringHint {
  public:
    PreColoringHint() { clear(); }
    bool preColorsTmp(RegState* reg) const;
    RegNumber getPreColoringReg(SSATmp* tmp, uint32_t index) const;
    void clear();
    void add(SSATmp* tmp, uint32_t index, int argNum);
  private:
    // indexed by arg number
    std::pair<SSATmp*, uint32_t> m_preColoredTmps[LinearScan::NumRegs];
  };

  class StateSave {
   public:
    StateSave() {}
    void save(LinearScan* ls);
    void restore(LinearScan* ls);
   private:
    RegState m_regs[NumRegs];
  };
  typedef smart::map<Block*, StateSave> ExitTraceMap;

private:
  void allocRegToInstruction(InstructionList::iterator it);
  void allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index);
  void allocRegToTmp(SSATmp* ssaTmp, uint32_t index);
  void freeRegsAtId(uint32_t id);
  void spill(SSATmp* tmp);
  void numberInstructions(const BlockList& blocks);

  template<typename T> SSATmp* cns(T val) {
    return m_irFactory->cns(val);
  }
  void initFreeList();
  void coalesce(Trace* trace);
  void genSpillStats(Trace* trace, int numSpillLocs);
  void allocRegsOneTrace(BlockList::iterator& blockIt, ExitTraceMap& etm);
  void allocRegsToTrace();
  uint32_t createSpillSlot(SSATmp* tmp);
  static SSATmp* getSpilledTmp(SSATmp* tmp);
  static SSATmp* getOrigTmp(SSATmp* tmp);
  uint32_t assignSpillLoc();
  void insertAllocFreeSpill(Trace* trace, uint32_t numExtraSpillLocs);
  void insertAllocFreeSpillAux(Trace* trace, uint32_t numExtraSpillLocs);
  void rematerialize();
  void rematerializeAux();
  void removeUnusedSpills();
  void collectInfo(BlockList::iterator it, Trace* trace);
  RegNumber getJmpPreColor(SSATmp* tmp, uint32_t regIndx, bool isReload);
  void computePreColoringHint();
  IRInstruction* getNextNative() const;
  uint32_t getNextNativeId() const;

  void pushFreeReg(RegState* reg);
  RegState* popFreeReg(smart::list<RegState*>& freeList);
  void freeReg(RegState* reg);
  RegState* getFreeReg(bool preferCallerSaved);
  RegState* getReg(RegState* reg);

  template<typename Inner, int DumpVal=4>
  void dumpIR(const Inner* in, const char* msg) {
    if (dumpIREnabled(DumpVal)) {
      std::ostringstream str;
      print(str, in, &m_allocInfo, &m_lifetime);
      HPHP::Trace::traceRelease("--- %s: %s\n", msg, str.str().c_str());
    }
  }

private:
  // Register allocation may generate Spill/Reload.
  IRFactory* const m_irFactory;
  RegState   m_regs[NumRegs];
  // Lists of free caller and callee-saved registers, respectively.
  smart::list<RegState*> m_freeCallerSaved;
  smart::list<RegState*> m_freeCalleeSaved;
  // List of assigned registers, sorted high to low by lastUseId.
  smart::list<RegState*> m_allocatedRegs;

  smart::vector<SlotInfo> m_slots;  // Spill info indexed by slot id
  BlockList m_blocks;             // all basic blocks in reverse postorder
  IdomVector m_idoms;             // immediate dominator vector

  // any tmp that has been spilled has an entry in this array with
  // the spill-slot number, which is an index into m_slots[]. tmps that
  // have not spilled have -1.
  StateVector<SSATmp, int32_t> m_spillSlots;

  LifetimeInfo m_lifetime; // Internal lifetime state
  LinearIdVector& m_linear; // linear id for each inst
  UsesVector& m_uses; // use count of each tmp

  // the list of native instructions in the trace sorted by instruction ID;
  // i.e. a filtered list in the same order as visited by m_blocks.
  smart::list<IRInstruction*> m_natives;

  // stores pre-coloring hints
  PreColoringHint m_preColoringHint;

  // a map from SSATmp* to a list of Jmp_ instructions that have it as
  // a source.
  typedef smart::vector<IRInstruction*> JmpList;
  StateVector<SSATmp, JmpList> m_jmps;

  RegAllocInfo m_allocInfo; // final allocation for each SSATmp
};

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
    IRInstruction* inst = tmp->inst();
    Opcode opc = inst->op();
    // The dst of IncRef, Mov, StRef, and StRefNT has the same value
    // as the src.
    // We follow these instructions to canonicalize an SSATmp.
    if (opc != IncRef && opc != Mov && opc != StRef && opc != StRefNT) {
      return tmp;
    }
    tmp = inst->getSrc(0);
  }
}

void LinearScan::StateSave::save(LinearScan* ls) {
  std::copy(ls->m_regs, ls->m_regs + NumRegs, m_regs);
}

void LinearScan::StateSave::restore(LinearScan* ls) {
  ls->m_allocatedRegs.clear();
  ls->m_freeCalleeSaved.clear();
  ls->m_freeCallerSaved.clear();

  for (size_t i = 0; i < NumRegs; i++) {
    ls->m_regs[i] = m_regs[i];
    RegState* reg = &ls->m_regs[i];
    if (reg->isReserved()) continue;
    if (reg->isAllocated()) {
      SSATmp* tmp = reg->m_ssaTmp;
      for (int r = 0; r < ls->m_allocInfo[tmp].numAllocatedRegs(); r++) {
        if ((int)ls->m_allocInfo[tmp].getReg(r) == i) {
          ls->allocRegToTmp(reg, tmp, r);
        }
      }
    } else {
      ls->pushFreeReg(reg);
    }
  }
}

LinearScan::LinearScan(IRFactory* irFactory)
  : m_irFactory(irFactory)
  , m_spillSlots(irFactory, -1)
  , m_lifetime(irFactory)
  , m_linear(m_lifetime.linear)
  , m_uses(m_lifetime.uses)
  , m_jmps(irFactory, JmpList())
  , m_allocInfo(irFactory)
{
  for (int i = 0; i < kNumX64Regs; i++) {
    m_regs[i].m_ssaTmp = nullptr;
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
  uint32_t numFreeRegs = RuntimeOption::EvalHHIRNumFreeRegs;
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

void LinearScan::allocRegToInstruction(InstructionList::iterator it) {
  IRInstruction* inst = &*it;
  dumpIR<IRInstruction, kExtraLevel>(inst, "allocating to instruction");

  // Reload all source operands if necessary.
  // Mark registers as unpinned.
  for (int regNo = 0; regNo < kNumX64Regs; ++regNo) {
    m_regs[regNo].m_pinned = false;
  }
  smart::vector<bool> needsReloading(inst->getNumSrcs(), true);
  for (uint32_t i = 0; i < inst->getNumSrcs(); ++i) {
    SSATmp* tmp = inst->getSrc(i);
    int32_t slotId = m_spillSlots[tmp];
    if (slotId == -1) {
      needsReloading[i] = false;
    } else if ((tmp = m_slots[slotId].latestReload)) {
      needsReloading[i] = false;
      inst->setSrc(i, tmp);
    }
    if (!needsReloading[i]) {
      for (int i = 0, n = m_allocInfo[tmp].numAllocatedRegs(); i < n; ++i) {
        m_regs[int(m_allocInfo[tmp].getReg(i))].m_pinned = true;
      }
    }
  }
  for (uint32_t i = 0; i < inst->getNumSrcs(); ++i) {
    if (needsReloading[i]) {
      SSATmp* tmp = inst->getSrc(i);
      int32_t slotId = m_spillSlots[tmp];
      // <tmp> is spilled, and not reloaded.
      // Therefore, We need to reload the value into a new SSATmp.

      // Insert the Reload instruction.
      SSATmp* spillTmp = m_slots[slotId].spillTmp;
      IRInstruction* reload = m_irFactory->gen(Reload, spillTmp);
      inst->getBlock()->insert(it, reload);

      // Create <reloadTmp> which inherits <tmp>'s slot ID and
      // <spillTmp>'s last use ID.
      // Replace <tmp> with <reloadTmp> in <inst>.
      SSATmp* reloadTmp = reload->getDst();
      m_uses[reloadTmp].lastUse = m_uses[spillTmp].lastUse;
      m_spillSlots[reloadTmp] = slotId;
      inst->setSrc(i, reloadTmp);
      // reloadTmp and tmp share the same type.  Since it was spilled, it
      // must be using its entire needed-count of registers.
      assert(reloadTmp->type() == tmp->type());
      assert(tmp->numNeededRegs() == m_allocInfo[tmp].numAllocatedRegs());
      for (int locIndex = 0; locIndex < tmp->numNeededRegs(); ++locIndex) {
        allocRegToTmp(reloadTmp, locIndex);
      }
      // Remember this reload tmp in case we can reuse it in later blocks.
      m_slots[slotId].latestReload = reloadTmp;
      dumpIR<IRInstruction, kExtraLevel>(reload, "created reload");
    }
  }

  freeRegsAtId(m_linear[inst]);
  // Update next native.
  if (getNextNative() == inst) {
    assert(!m_natives.empty());
    m_natives.pop_front();
    computePreColoringHint();
  }

  Range<SSATmp*> dsts = inst->getDsts();
  if (dsts.empty()) return;

  Opcode opc = inst->op();
  if (opc == DefMIStateBase) {
    assert(dsts[0].isA(Type::PtrToCell));
    allocRegToTmp(&m_regs[int(rsp)], &dsts[0], 0);
    return;
  }

  for (SSATmp& dst : dsts) {
    for (int i = 0, n = dst.numNeededRegs(); i < n; ++i) {
      // LdRaw, loading a generator's embedded AR, is the only time we have a
      // pointer to an AR that is not in rVmFp.
      const bool abnormalFramePtr =
        (opc == LdRaw &&
          inst->getSrc(1)->getValInt() == RawMemSlot::ContARPtr);

      // Note that the point of StashGeneratorSP is to save a StkPtr
      // somewhere other than rVmSp.  (TODO(#2288359): make rbx not
      // special.)
      const bool abnormalStkPtr = opc == StashGeneratorSP;

      if (!abnormalStkPtr && dst.isA(Type::StkPtr)) {
        assert(opc == DefSP ||
               opc == ReDefSP ||
               opc == ReDefGeneratorSP ||
               opc == Call ||
               opc == CallArray ||
               opc == SpillStack ||
               opc == SpillFrame ||
               opc == ExceptionBarrier ||
               opc == RetAdjustStack ||
               opc == InterpOne ||
               opc == GenericRetDecRefs ||
               opc == GuardStk ||
               opc == AssertStk ||
               opc == CastStk ||
               VectorEffects::supported(opc));
        allocRegToTmp(&m_regs[int(rVmSp)], &dst, 0);
        continue;
      }
      if (!abnormalFramePtr && dst.isA(Type::FramePtr)) {
        assert(opc == DefFP || opc == FreeActRec || opc == DefInlineFP);
        allocRegToTmp(&m_regs[int(rVmFp)], &dst, 0);
        continue;
      }

      // Generally speaking, StkPtrs are pretty special due to
      // tracelet ABI registers. Keep track here of the allowed uses
      // that don't use the above allocation.
      assert(!dst.isA(Type::FramePtr) || abnormalFramePtr);
      assert(!dst.isA(Type::StkPtr) || abnormalStkPtr);

      if (!RuntimeOption::EvalHHIRDeadCodeElim || m_uses[dst].lastUse != 0) {
        allocRegToTmp(&dst, i);
      }
    }
  }
  if (!RuntimeOption::EvalHHIRDeadCodeElim) {
    // if any outputs were unused, free regs now.
    freeRegsAtId(m_linear[inst]);
  }
}

void LinearScan::allocRegToTmp(SSATmp* ssaTmp, uint32_t index) {
  bool preferCallerSaved = true;
  if (RuntimeOption::EvalHHIREnableCalleeSavedOpt) {
    // Prefer caller-saved registers iff <ssaTmp> doesn't span native.
    preferCallerSaved = (m_uses[ssaTmp].lastUse <= getNextNativeId());
  }

  RegState* reg = nullptr;
  if (!preferCallerSaved) {
    reg = getFreeReg(false);
    if (reg->isCallerSaved()) {
      // If we are out of callee-saved registers, fall into the logic of
      // assigning a caller-saved register.
      pushFreeReg(reg);
      // getFreeReg pins the reg. Need restore it here.
      reg->m_pinned = false;
      reg = nullptr;
    }
  }
  if (reg == nullptr && RuntimeOption::EvalHHIREnablePreColoring) {
    // Pre-colors ssaTmp if it's used as an argument of next native.
    // Search for the original tmp instead of <ssaTmp> itself, because
    // the pre-coloring hint is not aware of reloaded tmps.
    SSATmp* orig = getOrigTmp(ssaTmp);
    RegNumber targetRegNo =
      m_preColoringHint.getPreColoringReg(orig, index);
    if (targetRegNo == reg::noreg) {
      targetRegNo = getJmpPreColor(orig, index, orig != ssaTmp);
    }
    if (targetRegNo != reg::noreg) {
      reg = getReg(&m_regs[int(targetRegNo)]);
    }
  }
  if (reg == nullptr &&
      RuntimeOption::EvalHHIREnablePreColoring &&
      ssaTmp->inst()->isNative()) {
    // Pre-colors ssaTmp if it's the return value of a native.
    if (index == 0) {
      reg = getReg(&m_regs[int(rax)]);
    } else if (index == 1) {
      reg = getReg(&m_regs[int(rdx)]);
    } else {
      not_reached();
    }
  }
  if (reg == nullptr) {
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
    if (m_spillSlots[ssaTmp] == -1) {
      createSpillSlot(ssaTmp);
    }
    m_uses[ssaTmp].lastUse = getNextNativeId();
  }

  allocRegToTmp(reg, ssaTmp, index);
}

void LinearScan::allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index) {
  reg->m_ssaTmp = ssaTmp;
  // mark inst as using this register
  m_allocInfo[ssaTmp].setReg(PhysReg(reg->m_regNo), index);
  uint32_t lastUseId = m_uses[ssaTmp].lastUse;
  if (reg->isReserved()) {
    return;
  }
  // insert into the list of assigned registers sorted by last use id
  auto it = m_allocatedRegs.begin();
  for (; it != m_allocatedRegs.end(); ++it) {
    if (lastUseId > m_uses[(*it)->m_ssaTmp].lastUse) {
      break;
    }
  }
  reg->m_pos = m_allocatedRegs.insert(it, reg);
}

// Assign spill location numbers to Spill/Reload.
uint32_t LinearScan::assignSpillLoc() {
  uint32_t nextSpillLoc = 0;
  uint32_t maxSpillLoc = 0;

  // visit blocks in reverse postorder and instructions in forward order,
  // assigning a spill slot id to each Spill. We don't reuse slot id's,
  // but both could be reused either by visiting the dominator tree in
  // preorder or by analyzing lifetimes and reusing id/registers between
  // non-conflicting spills.
  // As an intermediate step, re-use id's for exit traces

  smart::map<Block*, uint32_t> exitLocMap;

  for (Block* block : m_blocks) {
    auto it = exitLocMap.find(block);
    if (it != exitLocMap.end()) {
      nextSpillLoc = it->second;
    }
    for (IRInstruction& inst : *block) {
      if (getNextNative() == &inst) {
        assert(!m_natives.empty());
        m_natives.pop_front();
      }
      if (inst.op() == Spill) {
        SSATmp* dst = inst.getDst();
        SSATmp* src = inst.getSrc(0);
        for (int locIndex = 0;
             locIndex < src->numNeededRegs();
             ++locIndex) {
          if (m_uses[dst].lastUse <= getNextNativeId()) {
            TRACE(3, "[counter] 1 spill a tmp that does not span native\n");
          } else {
            TRACE(3, "[counter] 1 spill a tmp that spans native\n");
          }

          m_allocInfo[dst].setSpillInfo(locIndex, SpillInfo(nextSpillLoc++));
          TRACE(3, "[counter] 1 spill\n");
        }
      }
      if (inst.op() == Reload) {
        SSATmp* src = inst.getSrc(0);
        for (int locIndex = 0;
             locIndex < src->numNeededRegs();
             ++locIndex) {
          TRACE(3, "[counter] reload\n");
        }
      }
    }
    if (nextSpillLoc > maxSpillLoc) maxSpillLoc = nextSpillLoc;
    if (block->getTrace()->isMain()) {
      if (Block* taken = block->getTaken()) {
        if (!taken->getTrace()->isMain()) {
          exitLocMap[taken] = nextSpillLoc;
        }
      }
    }
  }
  return maxSpillLoc;
}

void LinearScan::insertAllocFreeSpill(Trace* trace,
                                      uint32_t numExtraSpillLocs) {
  insertAllocFreeSpillAux(trace, numExtraSpillLocs);
  for (Trace* exit : trace->getExitTraces()) {
    insertAllocFreeSpillAux(exit, numExtraSpillLocs);
  }
}

void LinearScan::insertAllocFreeSpillAux(Trace* trace,
                                         uint32_t numExtraSpillLocs) {
  SSATmp* tmp = m_irFactory->gen(DefConst, Type::Int,
    ConstData(numExtraSpillLocs))->getDst();

  for (Block* block : trace->getBlocks()) {
    for (auto it = block->begin(); it != block->end(); ) {
      auto next = it; ++next;
      IRInstruction& inst = *it;
      Opcode opc = inst.op();
      if (opc == Call) {
        // Insert FreeSpill and AllocSpill around each Call.
        IRInstruction* allocSpill = m_irFactory->gen(AllocSpill, tmp);
        IRInstruction* freeSpill = m_irFactory->gen(FreeSpill, tmp);
        block->insert(it, freeSpill);
        block->insert(next, allocSpill);
      } else if (opc == ExitTrace || opc == ExitSlow || opc == ExitTraceCc ||
                 opc == ExitSlowNoProgress || opc == ExitGuardFailure ||
                 opc == LdRetAddr) {
        // Insert FreeSpill at trace exits.
        IRInstruction* freeSpill = m_irFactory->gen(FreeSpill, tmp);
        block->insert(it, freeSpill);
      }
      it = next;
    }
  }

  // Insert AllocSpill at the start of the main trace.
  if (trace->isMain()) {
    trace->front()->prepend(m_irFactory->gen(AllocSpill, tmp));
  }
}

void LinearScan::collectInfo(BlockList::iterator it, Trace* trace) {
  m_natives.clear();
  m_jmps.reset();
  m_uses.reset();

  while (it != m_blocks.end()) {
    Block* block = *it++;
    bool offTrace = block->getTrace() != trace;
    if (offTrace) {
      if (!trace->isMain()) return;
      int lastId = block->getTrace()->getData();
      for (IRInstruction& inst : *block) {
        for (auto* src : inst.getSrcs()) {
          if (lastId > m_uses[src].lastUse) {
            m_uses[src].lastUse = lastId;
          }
        }
      }
    } else {
      for (IRInstruction& inst : *block) {
        for (auto* src : inst.getSrcs()) {
          m_uses[src].lastUse = m_linear[inst];
        }
        if (inst.isNative()) m_natives.push_back(&inst);
      }

      IRInstruction* jmp = block->back();
      if (jmp->op() == Jmp_ && jmp->getNumSrcs() != 0) {
        for (SSATmp* src : jmp->getSrcs()) {
          m_jmps[src].push_back(jmp);
        }
      }
    }
  }
}

void LinearScan::computePreColoringHint() {
  m_preColoringHint.clear();
  IRInstruction* inst = getNextNative();
  if (inst == nullptr) {
    return;
  }

  Opcode opc = inst->op();
  using namespace NativeCalls;
  if (CallMap::hasInfo(opc)) {
    unsigned reg = 0;
    for (auto const& arg : CallMap::getInfo(opc).args) {
      switch (arg.type) {
        case SSA:
          m_preColoringHint.add(inst->getSrc(arg.srcIdx), 0, reg++);
          break;
        case TV:
        case VecKeyS:
        case VecKeyIS:
          m_preColoringHint.add(inst->getSrc(arg.srcIdx), 0, reg++);
          m_preColoringHint.add(inst->getSrc(arg.srcIdx), 1, reg++);
          break;
        case Immed:
          break;
      }
    }
    return;
  }

  // For instructions that want to hint a continuous increasing range
  // of sources to a continuous increasing range of argument
  // registers.
  auto normalHint = [&](int count, int srcBase = 0, int argBase = 0) {
    for (int i = 0; i < count; ++i) {
      m_preColoringHint.add(inst->getSrc(i + srcBase), 0,
                            i + argBase);
    }
  };
  switch (opc) {
    case LdFunc:
      m_preColoringHint.add(inst->getSrc(0), 0, 1);
      break;
    case NativeImpl:
      m_preColoringHint.add(inst->getSrc(1), 0, 0);
      break;
    case Concat:
      {
        Type lType = inst->getSrc(0)->type();
        Type rType = inst->getSrc(1)->type();
        if ((lType.isString() && rType.isString()) ||
            (lType.isString() && rType == Type::Int) ||
            (lType == Type::Int && rType.isString())) {
          m_preColoringHint.add(inst->getSrc(0), 0, 0);
          m_preColoringHint.add(inst->getSrc(1), 0, 1);
        } else {
          m_preColoringHint.add(inst->getSrc(0), 0, 1);
          m_preColoringHint.add(inst->getSrc(1), 0, 3);
        }
      }
      break;
    case AKExists:
      normalHint(2);
      break;
    case DefFunc:
      normalHint(1);
      break;
    case OpEq:
    case OpNeq:
    case OpSame:
    case OpNSame:
      {
        auto src1 = inst->getSrc(0);
        auto src2 = inst->getSrc(1);

        auto type1 = src1->type();
        auto type2 = src2->type();

        if ((type1.isArray() && type2.isArray())
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
        m_preColoringHint.add(inst->getSrc(0), 0, 1);
      }
      break;
    case InstanceOf:
      normalHint(2);
      break;
    case LdSSwitchDestFast:
      normalHint(1);
      break;
    case LdSSwitchDestSlow:
      normalHint(1);
      break;
    case LdGblAddr:
    case LdGblAddrDef:
      normalHint(1);
      break;
    case LdClsPropAddr:
      normalHint(3);
      break;
    case LdCls:
      m_preColoringHint.add(inst->getSrc(0), 0, 1);
      break;
    case BoxPtr:
      normalHint(1);
      break;
    default:
      break;
  }
}

// Given a label, dest index for that label, and register index, scan
// the sources of all incoming Jmp_s to see if any have a register
// allocated at the specified index.
static RegNumber findLabelSrcReg(const RegAllocInfo& regs, IRInstruction* label,
                                 unsigned dstIdx, uint32_t regIndex) {
  assert(label->op() == DefLabel);
  SSATmp* withReg = label->getBlock()->findSrc(dstIdx, [&](SSATmp* src) {
    return regs[src].getReg(regIndex) != InvalidReg &&
      src->inst()->getBlock()->getHint() != Block::Unlikely;
  });
  return withReg ? regs[withReg].getReg(regIndex) : reg::noreg;
}

// This function attempts to find a pre-coloring hint from two
// different sources: If tmp comes from a DefLabel, it will scan up to
// the SSATmps providing values to incoming Jmp_s to look for a
// hint. If tmp is consumed by a Jmp_, look for other incoming Jmp_s
// to its destination and see if any of them have already been given a
// register. If all of these fail, let normal register allocation
// proceed unhinted.
RegNumber LinearScan::getJmpPreColor(SSATmp* tmp, uint32_t regIndex,
                                     bool isReload) {
  IRInstruction* srcInst = tmp->inst();
  const JmpList& jmps = m_jmps[tmp];
  if (isReload && (srcInst->op() == DefLabel || !jmps.empty())) {
    // If we're precoloring a Reload of a temp that we'd normally find
    // a hint for, just return the register allocated to the spilled
    // temp.
    auto reg = m_allocInfo[tmp].getReg(regIndex);
    assert(reg != reg::noreg);
    return reg;
  }

  if (srcInst->op() == DefLabel) {
    // Figure out which dst of the label is tmp
    for (unsigned i = 0, n = srcInst->getNumDsts(); i < n; ++i) {
      if (srcInst->getDst(i) == tmp) {
        auto reg = findLabelSrcReg(m_allocInfo, srcInst, i, regIndex);
        // Until we handle loops, it's a bug to try and allocate a
        // register to a DefLabel's dest before all of its incoming
        // Jmp_s have had their srcs allocated.
        always_assert(reg != reg::noreg);
        return reg;
      }
    }
    not_reached();
  }

  // If srcInst wasn't a label, check if tmp is used by any Jmp_
  // instructions. If it is, trace to the Jmp_'s label and use the
  // same procedure as above.
  for (unsigned ji = 0, jn = jmps.size(); ji < jn; ++ji) {
    IRInstruction* jmp = jmps[ji];
    IRInstruction* label = jmp->getTaken()->front();

    // Figure out which src of the Jmp_ is tmp
    for (unsigned si = 0, sn = jmp->getNumSrcs(); si < sn; ++si) {
      SSATmp* src = jmp->getSrc(si);
      if (tmp == src) {
        // For now, a DefLabel should never have a register assigned
        // to it before any of its incoming Jmp_ instructions.
        always_assert(m_allocInfo[label->getDst(si)].getReg(regIndex) ==
                      reg::noreg);
        auto reg = findLabelSrcReg(m_allocInfo, label, si, regIndex);
        if (reg != reg::noreg) return reg;
      }
    }
  }

  return reg::noreg;
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
  forEachTraceInst(trace, [](IRInstruction* inst) {
    for (uint32_t i = 0; i < inst->getNumSrcs(); ++i) {
      SSATmp* src = inst->getSrc(i);
      SSATmp* origSrc = canonicalize(src);
      if (origSrc != src) {
        // Replace every operand with its canonicalized version.
        inst->setSrc(i, origSrc);
      }
    }
  });
}

// Assign ids to each instruction in linear order.
void LinearScan::numberInstructions(const BlockList& blocks) {
  m_spillSlots.reset();
  m_uses.reset();
  uint32_t nextId = 1;
  for (auto* block : blocks) {
    for (auto& inst : *block) {
      if (inst.op() == Marker) continue; // don't number markers
      uint32_t id = nextId++;
      m_linear[inst] = id;
      for (SSATmp* tmp : inst.getSrcs()) {
        m_uses[tmp].lastUse = id;
        m_uses[tmp].count++;
      }
    }
    if (block->getTaken() && block->isMain() && !block->getTaken()->isMain()) {
      // reserve a spot for the lastUseId when we're processing the main
      // trace, if the last use is really in an exit trace.
      block->getTaken()->getTrace()->setData(nextId++);
    }
  }
}

void LinearScan::genSpillStats(Trace* trace, int numSpillLocs) {
  if (!moduleEnabled(HPHP::Trace::statgroups, 1)) return;

  int numMainSpills = 0;
  int numExitSpills = 0;
  int numMainReloads = 0;
  int numExitReloads = 0;
  forEachInst(
    m_blocks,
    [&](IRInstruction* inst) {
      if (inst->op() == Spill) {
        if (inst->getBlock()->isMain()) {
          numMainSpills++;
        } else {
          numExitSpills++;
        }
      } else if (inst->op() == Reload) {
        if (inst->getBlock()->isMain()) {
          numMainReloads++;
        } else {
          numExitReloads++;
        }
      }
    }
  );

  static StringData* spillStats = StringData::GetStaticString("SpillStats");
  static StringData* mainSpills = StringData::GetStaticString("MainSpills");
  static StringData* mainReloads = StringData::GetStaticString("MainReloads");
  static StringData* exitSpills = StringData::GetStaticString("ExitSpills");
  static StringData* exitReloads = StringData::GetStaticString("ExitReloads");
  static StringData* spillSpace = StringData::GetStaticString("SpillSpace");
  trace->front()->prepend(m_irFactory->gen(
                            IncStatGrouped,
                            cns(spillStats),
                            cns(mainSpills), cns(numMainSpills)));
  trace->front()->prepend(m_irFactory->gen(
                            IncStatGrouped,
                            cns(spillStats),
                            cns(mainReloads), cns(numMainReloads)));
  trace->front()->prepend(m_irFactory->gen(
                            IncStatGrouped,
                            cns(spillStats),
                            cns(exitSpills), cns(numExitSpills)));
  trace->front()->prepend(m_irFactory->gen(
                            IncStatGrouped,
                            cns(spillStats),
                            cns(exitReloads), cns(numExitReloads)));
  trace->front()->prepend(m_irFactory->gen(
                            IncStatGrouped,
                            cns(spillStats),
                            cns(spillSpace), cns(numSpillLocs)));

}

RegAllocInfo LinearScan::allocRegs(Trace* trace, LifetimeInfo* lifetime) {
  if (RuntimeOption::EvalHHIREnableCoalescing) {
    // <coalesce> doesn't need instruction numbering.
    coalesce(trace);
  }

  m_blocks = sortCfg(trace, *m_irFactory);
  m_idoms = findDominators(m_blocks);

  allocRegsToTrace();

  if (RuntimeOption::EvalHHIREnableRematerialization && m_slots.size() > 0) {
    // Don't bother rematerializing the trace if it has no Spill/Reload.
    rematerialize();
  }
  numberInstructions(m_blocks);

  // Make sure rsp is 16-aligned.
  uint32_t numSpillLocs = assignSpillLoc();
  if (numSpillLocs % 2) {
    static_assert(NumPreAllocatedSpillLocs % 2 == 0, "");
    ++numSpillLocs;
  }
  if (numSpillLocs > (uint32_t)NumPreAllocatedSpillLocs) {
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

  if (m_slots.size()) genSpillStats(trace, numSpillLocs);

  if (lifetime) {
    lifetime->linear = std::move(m_linear);
    lifetime->uses = std::move(m_uses);
  }
  return m_allocInfo;
}

void LinearScan::allocRegsOneTrace(BlockList::iterator& blockIt,
                                   ExitTraceMap& etm) {
  Trace* trace = (*blockIt)->getTrace();
  collectInfo(blockIt, trace);
  computePreColoringHint();

  auto v = etm.find(*blockIt);
  if (v != etm.end()) {
    assert(!trace->isMain());
    v->second.restore(this);
  } else {
    assert(blockIt == m_blocks.begin() && trace->isMain());
    initFreeList();
  }

  // First, visit every instruction, allocating registers as we go,
  // and inserting Reload instructions where necessary.
  bool isMain = trace->isMain();
  size_t sz = m_slots.size();
  while (blockIt != m_blocks.end()) {
    Block* block = *blockIt;
    if (block->getTrace() != trace) {
      break;
    }

    // clear remembered reloads that don't dominate this block
    for (SlotInfo& slot : m_slots) {
      if (SSATmp* reload = slot.latestReload) {
        if (!dominates(reload->inst()->getBlock(), block, m_idoms)) {
          slot.latestReload = nullptr;
        }
      }
    }
    for (auto it = block->begin(), end = block->end(); it != end; ++it) {
      allocRegToInstruction(it);
      dumpIR<IRInstruction, kExtraLevel>(&*it, "allocated to instruction");
    }
    if (isMain) {
      assert(block->getTrace()->isMain());
      if (block->getTaken() &&
          !block->getTaken()->getTrace()->isMain()) {
        etm[block->getTaken()].save(this);
      }
    }
    ++blockIt;
  }

  // Now that we have visited all instructions on this trace,
  // and inserted Reloads for SSATmps which needed to be spilled,
  // we can go back and insert the spills.
  // On the main trace, insert the spill right after the instruction
  // that generated the value (without traversing everything else).
  // On exit traces, if the instruction that generated the value
  // is on the main trace, insert the spill at the start of the trace,
  // otherwise, after the instruction that generated the value
  size_t begin = sz;
  size_t end = m_slots.size();

  while (begin < end) {
    SlotInfo& slot = m_slots[begin++];
    IRInstruction* spill = slot.spillTmp->inst();
    IRInstruction* inst = spill->getSrc(0)->inst();
    Block* block = inst->getBlock();
    if (!isMain && block->getTrace()->isMain()) {
      // We're on an exit trace, but the def is on the
      // main trace, so put it at the start of this trace
      if (spill->getBlock()) {
        // its already been inserted in another exit trace
        assert(!spill->getBlock()->getTrace()->isMain());
        spill = spill->clone(m_irFactory);
      }
      block->getTrace()->front()->prepend(spill);
    } else if (inst->isBlockEnd()) {
      block->getNext()->prepend(spill);
    } else {
      auto pos = block->iteratorTo(inst);
      block->insert(++pos, spill);
    }
  }
}

void LinearScan::allocRegsToTrace() {
  ExitTraceMap etm;

  numberInstructions(m_blocks);

  BlockList::iterator it = m_blocks.begin();
  while (it != m_blocks.end()) {
    allocRegsOneTrace(it, etm);
  }
}

void LinearScan::rematerialize() {
  numberInstructions(m_blocks);
  dumpTrace(kExtraLevel, m_blocks.front()->getTrace(),
            " before rematerialization ", &m_allocInfo, &m_lifetime);
  rematerializeAux();
  numberInstructions(m_blocks);
  // We only replaced Reloads in rematerializeAux().
  // Here, we remove Spills that are never reloaded.
  removeUnusedSpills();
}

// Return true if it's safe to rematerialize inst at reload's position.
bool srcsAreLive(const RegAllocInfo& regs, IRInstruction* inst,
                 IRInstruction* reload) {
  // It's only ok to rematerialize an instruction if its sources are
  // guaranteed to be in known registers at the point we would other wise
  // reload its spilled result.
  // Although we can inspect each SSAtmp's live range (linear id of its
  // definition point and last use), we don't trust that live range given
  // the state of the IR after register allocation has inserted spills
  // and reloads.  So only rematerialize instructions whose sources are
  // in registers we trust to be live: rVmSp and rVmFp.  Ignore DefConst
  // sources because they're implicitly turned into immediates by every
  // instruction that uses them.
  for (SSATmp* src : inst->getSrcs()) {
    if (!regs[src].hasReg(0) && src->inst()->op() != DefConst) return false;
    auto reg = regs[src].getReg(0);
    if (reg != rVmSp && reg != rVmFp) return false;
  }
  return true;
};

void LinearScan::rematerializeAux() {
  struct State {
    SSATmp *sp, *fp;
    smart::vector<SSATmp*> values;
  };
  StateVector<Block, State*> states(m_irFactory, nullptr);
  SCOPE_EXIT { for (State* s : states) delete s; };
  SSATmp* curSp = nullptr;
  SSATmp* curFp = nullptr;
  smart::vector<SSATmp*> localValues;
  auto killLocal = [&](IRInstruction& inst, unsigned src) {
    if (src < inst.getNumSrcs()) {
      unsigned loc = inst.getSrc(src)->getValInt();
      if (loc < localValues.size()) localValues[loc] = nullptr;
    }
  };
  auto setLocal = [&](unsigned loc, SSATmp* value) {
    // Note that when we implement inlining, we will need to deal
    // with the new local id space of the inlined function.
    if (loc >= localValues.size()) localValues.resize(loc + 1);
    localValues[loc] = canonicalize(value);
  };
  // Search for a local that stores <value>
  auto findLocal = [&](SSATmp* value) -> int {
    auto pos = std::find(localValues.begin(), localValues.end(),
                         canonicalize(value));
    return pos != localValues.end() ? pos - localValues.begin() : -1;
  };
  // save the current state for future use by block; merge if necessary.
  auto saveState = [&](Block* block) {
    if (State* state = states[block]) {
      // merge with saved state
      assert(curFp == state->fp);
      if (curSp != state->sp) state->sp = nullptr;
      for (unsigned i = 0; i < state->values.size(); ++i) {
        if (i >= localValues.size() || localValues[i] != state->values[i]) {
          state->values[i] = nullptr;
        }
      }
    } else {
      // snapshot state for use at target.
      state = states[block] = new State;
      state->sp = curSp;
      state->fp = curFp;
      state->values = localValues;
    }
  };

  for (Block* block : m_blocks) {
    if (State* state = states[block]) {
      states[block] = nullptr;
      localValues = state->values;
      curSp = state->sp;
      curFp = state->fp;
      delete state;
    }
    for (auto it = block->begin(); it != block->end(); ++it) {
      IRInstruction& inst = *it;
      Opcode opc = inst.op();
      if (opc == DefFP || opc == FreeActRec) {
        assert(m_allocInfo[inst.getDst()].getReg() == rVmFp);
        curFp = inst.getDst();
      } else if (opc == Reload) {
        // s = Spill t0
        // t = Reload s
        SSATmp* dst = inst.getDst();
        SSATmp* spilledTmp = getSpilledTmp(dst);
        IRInstruction* spilledInst = spilledTmp->inst();
        IRInstruction* newInst = NULL;
        if (spilledInst->isRematerializable() ||
            (spilledInst->op() == LdStack &&
             spilledInst->getSrc(0) == curSp)) {
          // XXX: could change <newInst> to the non-check version.
          // Rematerialize those rematerializable instructions (i.e.,
          // isRematerializable returns true) and LdStack.
          if (srcsAreLive(m_allocInfo, spilledInst, &inst)) {
            newInst = spilledInst->clone(m_irFactory);
            // The new instruction needn't have an exit label; it must always
            // be dominated by the original instruction because reloads are
            // inserted just before uses, which must be dominated by the
            // original (spilled) def.
            newInst->setTaken(nullptr);
          }
        } else if (curFp) {
          // Rematerialize LdLoc.
          int loc = findLocal(spilledTmp);
          if (loc != -1) {
            newInst = m_irFactory->gen(
              LdLoc,
              dst->type(),
              LocalId(loc),
              curFp
            );
          }
        }
        if (newInst) {
          UNUSED Type oldType = dst->type();
          newInst->setDst(dst);
          dst->setInstruction(newInst);
          assert(outputType(newInst) == oldType);
          auto* block = inst.getBlock();
          auto newIt = block->insert(it, newInst);
          block->erase(it);
          it = newIt;
        }
      }

      // Updating curSp and localValues
      if (inst.hasDst() && m_allocInfo[inst.getDst()].getReg() == rVmSp) {
        // inst modifies the stack pointer.
        curSp = inst.getDst();
      }

      if (opc == LdLoc || opc == StLoc || opc == StLocNT) {
        setLocal(inst.getExtra<LocalId>()->locId,
                 opc == LdLoc ? inst.getDst() : inst.getSrc(1));
      }
      // Other instructions that may have side effects on locals must
      // kill the local variable values.
      else if (opc == IterInit) {
        killLocal(inst, 3);
      } else if (opc == IterInitK) {
        killLocal(inst, 3);
        killLocal(inst, 4);
      } else if (opc == IterNext) {
        killLocal(inst, 2);
      } else if (opc == IterNextK) {
        killLocal(inst, 2);
        killLocal(inst, 3);
      }
    }
    if (Block* taken = block->getTaken()) saveState(taken);
    if (Block* next = block->getNext()) saveState(next);
  }
}

void LinearScan::removeUnusedSpills() {
  for (SlotInfo& slot : m_slots) {
    IRInstruction* spill = slot.spillTmp->inst();
    if (m_uses[spill->getDst()].count == 0) {
      Block* block = spill->getBlock();
      block->erase(block->iteratorTo(spill));
      SSATmp* src = spill->getSrc(0);
      auto uses = m_uses[src].count - 1;
      m_uses[src].count = uses;
      if (uses == 0) {
        Opcode srcOpc = src->inst()->op();
        // Not all instructions are able to take noreg as its dest
        // reg.  We pick LdLoc and IncRef because they occur often.
        if (srcOpc == IncRef || srcOpc == LdLoc) {
          for (int i = 0, n = src->numNeededRegs(); i < n; ++i) {
            m_allocInfo[src].setReg(InvalidReg, i);
          }
        }
      }
    }
  }
}

void LinearScan::freeRegsAtId(uint32_t id) {
  // free all registers whose lifetime ends at this id
  // Note that we free registers before we allocate a register
  // to this instruction, so we have to be careful to finish using
  // a register before over-writing it.
  for (auto it = m_allocatedRegs.begin(); it != m_allocatedRegs.end(); ) {
    auto next = it; ++next;
    RegState* reg = *it;
    assert(reg->m_ssaTmp);
    if (m_uses[reg->m_ssaTmp].lastUse <= id) {
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
    return nullptr;
  }
  auto& freeList = (reg->isCallerSaved() ?
                    m_freeCallerSaved : m_freeCalleeSaved);
  freeList.erase(reg->m_pos);
  // Pin it so that other operands in the same instruction will not reuse it.
  reg->m_pinned = true;
  return reg;
}

LinearScan::RegState* LinearScan::getFreeReg(bool preferCallerSaved) {
  if (m_freeCallerSaved.empty() && m_freeCalleeSaved.empty()) {
    assert(!m_allocatedRegs.empty());

    // no free registers --> free a register from the allocatedRegs
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

  smart::list<RegState*>* preferred = nullptr;
  smart::list<RegState*>* other = nullptr;
  if (preferCallerSaved) {
    preferred = &m_freeCallerSaved;
    other = &m_freeCalleeSaved;
  } else {
    preferred = &m_freeCalleeSaved;
    other = &m_freeCallerSaved;
  }

  RegState* theFreeReg = nullptr;
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
  int32_t slotId = m_spillSlots[tmp];
  if (slotId != -1) {
    m_slots[slotId].latestReload = nullptr;
  }
  reg->m_ssaTmp = nullptr;
}

void LinearScan::pushFreeReg(RegState* reg) {
  auto& freeList = (reg->isCallerSaved() ?
                    m_freeCallerSaved : m_freeCalleeSaved);
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

LinearScan::RegState* LinearScan::popFreeReg(smart::list<RegState*>& freeList) {
  if (freeList.empty()) {
    return nullptr;
  }
  RegState* reg = freeList.front();
  freeList.pop_front();
  return reg;
}

void LinearScan::spill(SSATmp* tmp) {
  dumpIR<SSATmp, kExtraLevel>(tmp, "spilling");
  // If we're spilling, we better actually have registers allocated.
  assert(m_allocInfo[tmp].numAllocatedRegs() > 0);
  assert(m_allocInfo[tmp].numAllocatedRegs() == tmp->numNeededRegs());

  // Free the registers used by <tmp>.
  // Need call freeReg and modify <m_allocatedRegs>.
  for (auto it = m_allocatedRegs.begin(); it != m_allocatedRegs.end(); ) {
    auto next = it; ++next;
    RegState* reg = *it;
    if (reg->m_ssaTmp == tmp) {
      freeReg(reg);
      m_allocatedRegs.erase(it);
    }
    it = next;
  }

  if (m_spillSlots[tmp] == -1) {
    // <tmp> hasn't been spilled before.
    // We need to create a new spill slot for it.
    uint32_t slotId = createSpillSlot(tmp);
    // createSpillSlot sets the latest reloaded value of slotId to tmp.
    // Here, we need reset this value because tmp is spilled and no longer
    // synced with memory.
    m_slots[slotId].latestReload = nullptr;
  }
}

// Create a spill slot for <tmp>.
uint32_t LinearScan::createSpillSlot(SSATmp* tmp) {
  uint32_t slotId = m_slots.size();
  m_spillSlots[tmp] = slotId;
  IRInstruction* spillInst = m_irFactory->gen(Spill, tmp);
  SSATmp* spillTmp = spillInst->getDst();
  SlotInfo si;
  si.spillTmp = spillTmp;
  si.latestReload = tmp;
  m_slots.push_back(si);
  // The spill slot inherits the last use ID of the spilled tmp.
  m_uses[si.spillTmp].lastUse = m_uses[tmp].lastUse;
  return slotId;
}

IRInstruction* LinearScan::getNextNative() const {
  return m_natives.empty() ? nullptr : m_natives.front();
}

uint32_t LinearScan::getNextNativeId() const {
  IRInstruction* nextNative = getNextNative();
  return nextNative ? m_linear[nextNative] : -1;
}

SSATmp* LinearScan::getSpilledTmp(SSATmp* tmp) {
  assert(tmp->inst()->op() == Reload);
  SSATmp* slot = tmp->inst()->getSrc(0);
  assert(slot->inst()->op() == Spill);
  return slot->inst()->getSrc(0);
}

// If <tmp> is a reloaded value, follow the spill-reload chain to find
// its source; otherwise, return <tmp> itself.
SSATmp* LinearScan::getOrigTmp(SSATmp* tmp) {
  if (tmp->inst()->op() == Reload)
    return getSpilledTmp(tmp);
  return tmp;
}

bool LinearScan::PreColoringHint::preColorsTmp(RegState* reg) const {
  return m_preColoredTmps[reg->m_regNo].first != nullptr;
}

// Get the pre-coloring register of (<tmp>, <index>).
// A native call has at most six arguments, so the time complexity is
// not a big problem.
RegNumber LinearScan::PreColoringHint::getPreColoringReg(
    SSATmp* tmp, uint32_t index) const {
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
    m_preColoredTmps[i].first = nullptr;
    m_preColoredTmps[i].second = 0;
  }
}

// Provide a hint that (<tmp>, <index>) is used as the <argNum>-th arg
// in next native.
void LinearScan::PreColoringHint::add(SSATmp* tmp, uint32_t index, int argNum) {
  int reg = int(argNumToRegName[argNum]);
  assert(reg >= 0 && reg < kNumX64Regs);
  m_preColoredTmps[reg].first = tmp;
  m_preColoredTmps[reg].second = index;
}

//////////////////////////////////////////////////////////////////////

RegAllocInfo allocRegsForTrace(Trace* trace, IRFactory* irFactory,
                               LifetimeInfo* lifetime) {
  return LinearScan(irFactory).allocRegs(trace, lifetime);
}

}} // HPHP::JIT
