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

#include "hphp/runtime/vm/jit/linear-scan.h"

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/trace-builder.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include <boost/noncopyable.hpp>

namespace HPHP {
namespace JIT{

using namespace JIT::reg;

TRACE_SET_MOD(hhir);

struct LinearScan : private boost::noncopyable {
  explicit LinearScan(IRUnit&);
  RegAllocInfo allocRegs();

private:
  class RegState {
    friend class LinearScan;

  public:
    bool isReserved() const { return m_reserved; }
    bool isCallerSaved() const {
      return X64::kCallerSaved.contains(m_reg);
    }
    bool isCalleeSaved() const { return !isCallerSaved(); }
    bool isAllocated() const { return m_ssaTmp != nullptr; }
    bool isPinned() const { return m_pinned; }
    bool isRetAddr() const {
      if (!m_ssaTmp) return false;
      Type type = m_ssaTmp->type();
      return type == Type::RetAddr;
    }
    PhysReg::Type type() const { return m_reg.type(); }

  private:
    SSATmp*   m_ssaTmp; // non-null when allocated
    // Maintain the position of this register so that we can quickly
    // remove it from the lists.
    // A non-reserved reg is in either LinearScan::m_freeCallerSaved,
    // LinearScan::m_freeCalleeSaved, or LinearScan::m_allocatedRegs.
    // <m_pos> of a reserved reg is undefined.
    smart::list<RegState*>::iterator m_pos;
    PhysReg   m_reg;
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
    PhysReg getPreColoringReg(SSATmp* tmp, uint32_t index) const;
    void clear();
    void add(SSATmp* tmp, uint32_t index, int argNum);
  private:
    // indexed by register number
    PhysReg::Map<std::pair<SSATmp*, uint32_t>> m_preColoredTmps;
  };

  class StateSave {
   public:
    StateSave() {}
    void save(LinearScan* ls);
    void restore(LinearScan* ls);
   private:
    PhysReg::Map<RegState> m_regs;
  };
  typedef smart::map<Block*, StateSave> ExitTraceMap;

private:
  void allocRegToInstruction(InstructionList::iterator it);
  int  allocRegToTmp(SSATmp* ssaTmp, uint32_t index);
  void assignRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index);
  void freeRegsAtId(uint32_t id);
  void spill(SSATmp* tmp);
  void numberInstructions(const BlockList& blocks);

  template<typename T> SSATmp* cns(T val) {
    return m_unit.cns(val);
  }
  void initFreeList();
  void coalesce();
  void genSpillStats(int numSpillLocs);
  void allocRegsOneTrace(BlockList::iterator& blockIt,
                         ExitTraceMap& etm);
  void allocRegsToTrace();
  uint32_t createSpillSlot(SSATmp* tmp);
  static SSATmp* getSpilledTmp(SSATmp* tmp);
  static SSATmp* getOrigTmp(SSATmp* tmp);
  uint32_t assignSpillLoc();
  void collectInfo(BlockList::iterator it, IRTrace* trace);
  RegNumber getJmpPreColor(SSATmp* tmp, uint32_t regIndx, bool isReload);
  void computePreColoringHint();
  void findFullSIMDCandidates();
  IRInstruction* nextNative() const;
  uint32_t nextNativeId() const;

  void pushFreeReg(RegState* reg);
  RegState* popFreeReg(smart::list<RegState*>& freeList);
  void freeReg(RegState* reg);
  RegState* getFreeReg(PhysReg::Type type, bool preferCallerSaved);
  RegState* getReg(RegState* reg);
  PhysReg::Type getRegType(const SSATmp *tmp, int locIdx) const;
  bool crossNativeCall(const SSATmp* tmp) const;
  RegAllocInfo computeRegs() const;
  void resolveJmpCopies();

  void dumpIR(const SSATmp* tmp, const char* msg) {
    if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhir, kExtraLevel)) {
      std::ostringstream str;
      print(str, tmp, &m_allocInfo[tmp], &m_lifetime);
      HPHP::Trace::traceRelease("--- %s: %s\n", msg, str.str().c_str());
    }
  }

  void dumpIR(const IRInstruction* inst, const char* msg) {
    if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhir, kExtraLevel)) {
      auto regs = computeRegs();
      std::ostringstream str;
      print(str, inst, &regs, &m_lifetime);
      HPHP::Trace::traceRelease("--- %s: %s\n", msg, str.str().c_str());
    }
  }

private:
  // Register allocation may generate Spill/Reload.
  IRUnit& m_unit;
  PhysReg::Map<RegState> m_regs;
  // Lists of free caller and callee-saved registers, respectively.
  smart::list<RegState*> m_freeCallerSaved[PhysReg::kNumTypes];
  smart::list<RegState*> m_freeCalleeSaved[PhysReg::kNumTypes];
  // List of assigned registers, sorted high to low by lastUseId.
  smart::list<RegState*> m_allocatedRegs;

  smart::vector<SlotInfo> m_slots;  // Spill info indexed by slot id
  BlockList m_blocks;               // all basic blocks in reverse postorder
  IdomVector m_idoms;               // immediate dominator vector

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

  // a map from SSATmp* to a list of Jmp instructions that have it as
  // a source.
  typedef smart::vector<IRInstruction*> JmpList;
  StateVector<SSATmp, JmpList> m_jmps;

  // final allocation for each SSATmp
  StateVector<SSATmp, PhysLoc> m_allocInfo;

  // SSATmps requiring 2 64-bit registers that are eligible for
  // allocation to a single SIMD register
  boost::dynamic_bitset<> m_fullSIMDCandidates;

  // reserved linear ids for each exit trace
  smart::flat_map<IRTrace*, uint32_t> m_exitIds;
};

static_assert(X64::kReservedRSPSpillSpace ==
              NumPreAllocatedSpillLocs * sizeof(void*),
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
    switch (opc) {
      case IncRef:
      case Mov:
      case StRef:
      case StRefNT:
        tmp = inst->src(0);
        break;

      default:
        return tmp;
    }
  }
}

RegAllocInfo LinearScan::computeRegs() const {
  RegAllocInfo regs(m_unit);
  for (auto b : m_blocks) {
    for (auto& i : *b) {
      for (auto  s : i.srcs()) regs[i][s] = m_allocInfo[s];
      for (auto& d : i.dsts()) regs[i][d] = m_allocInfo[d];
    }
  }
  return regs;
}

void LinearScan::StateSave::save(LinearScan* ls) {
  for (auto r : ls->m_regs) {
    m_regs[r] = ls->m_regs[r];
  }
}

void LinearScan::StateSave::restore(LinearScan* ls) {
  ls->m_allocatedRegs.clear();
  for (int i = 0; i < PhysReg::kNumTypes; i++) {
    ls->m_freeCalleeSaved[i].clear();
    ls->m_freeCallerSaved[i].clear();
  }

  for (auto i : m_regs) {
    ls->m_regs[i] = m_regs[i];
    RegState* reg = &ls->m_regs[i];
    if (reg->isReserved()) continue;
    if (reg->isAllocated()) {
      SSATmp* tmp = reg->m_ssaTmp;
      for (int r = 0; r < ls->m_allocInfo[tmp].numAllocated(); r++) {
        if (ls->m_allocInfo[tmp].reg(r) == i) {
          ls->assignRegToTmp(reg, tmp, r);
        }
      }
    } else {
      ls->pushFreeReg(reg);
    }
  }
}

LinearScan::LinearScan(IRUnit& unit)
  : m_unit(unit)
  , m_idoms(unit, nullptr)
  , m_spillSlots(unit, -1)
  , m_lifetime(unit)
  , m_linear(m_lifetime.linear)
  , m_uses(m_lifetime.uses)
  , m_jmps(unit, JmpList())
  , m_allocInfo(unit, PhysLoc())
  , m_fullSIMDCandidates(unit.numTmps())
{
  m_exitIds.reserve(unit.exits().size());
  for (auto reg : m_regs) {
    m_regs[reg].m_ssaTmp = nullptr;
    m_regs[reg].m_reg = reg;
    m_regs[reg].m_pinned = false;
    m_regs[reg].m_reserved = false;
  }

  // Mark reserved regs.
  using namespace X64;
  m_regs[rVmSp]  .m_reserved = true;
  m_regs[rsp]    .m_reserved = true;
  m_regs[rVmFp]  .m_reserved = true;
  m_regs[rAsm]   .m_reserved = true;
  m_regs[rVmTl]  .m_reserved = true;
  m_regs[rCgGP]  .m_reserved = true;
  m_regs[rCgXMM0].m_reserved = true;
  m_regs[rCgXMM1].m_reserved = true;

  // Reserve extra regs for testing purpose.
  uint32_t numFreeRegs = RuntimeOption::EvalHHIRNumFreeRegs;
  for (auto r : m_regs) {
    if (!m_regs[r].m_reserved) {
      if (numFreeRegs == 0) {
        m_regs[r].m_reserved = true;
      } else {
        --numFreeRegs;
      }
    }
  }
}

PhysReg::Type LinearScan::getRegType(const SSATmp* tmp, int locIdx) const {
  if (!RuntimeOption::EvalHHIRAllocSIMDRegs) return PhysReg::GP;

  // If we're selecting a register for the type, it means this SSATmp
  // didn't get it's value allocated to a SIMD register, which
  // otherwise would store the type too.
  if (locIdx == 1) return PhysReg::GP;

  if (tmp->isA(Type::Dbl)) return PhysReg::SIMD;

  if (packed_tv) return PhysReg::GP;

  DEBUG_ONLY Type tmpType = tmp->type();

  uint32_t tmpId = tmp->id();

  if (tmp->inst()->op() == Reload) {
    // We don't have an entry for reloaded SSATmps in
    // m_fullSIMDCandidates, since they're inserted after this set is
    // computed.  So we approximate this property for the reloaded
    // SSATmp using the original SSATmp that was spilled.  In other
    // words, if the original SSATmp was a candidate to be allocated
    // to a full SIMD register, then so is the reloaded SSATmp.  This
    // might be a bit conservative, but avoids recomputing the analysis.
    auto* reload = tmp->inst();
    auto* spill  = reload->src(0)->inst();
    tmpId = spill->src(0)->id();
  }

  if (m_fullSIMDCandidates[tmpId]) {
    FTRACE(6,
       "getRegType(SSATmp {} : {}): it's a candidate for full SIMD register\n",
           tmpId, tmpType.toString());
    FTRACE(6,
       "getRegType(SSATmp {}): crossNative = {} ; # freeCalleeSaved[GP] = {}\n",
           tmpId, crossNativeCall(tmp), m_freeCalleeSaved[PhysReg::GP].size());

    // Note that there are no callee-saved SIMD registers in the x64
    // ABI.  So, if tmp crosses native calls and there are 2 free GP
    // callee-saved registers, then allocate tmp to GP registers.
    if (RuntimeOption::EvalHHIREnableCalleeSavedOpt &&
        crossNativeCall(tmp) &&
        m_freeCalleeSaved[PhysReg::GP].size() >= 2) {
      return PhysReg::GP;
    }
    return PhysReg::SIMD;
  }
  return PhysReg::GP;
}

PhysReg forceAlloc(SSATmp& dst) {
  auto inst = dst.inst();
  auto opc = inst->op();

  // Note that the point of StashGeneratorSP is to save a StkPtr
  // somewhere other than rVmSp.  (TODO(#2288359): make rbx not
  // special.)
  bool abnormalStkPtr = opc == StashGeneratorSP;

  if (!abnormalStkPtr && dst.isA(Type::StkPtr)) {
    assert(opc == DefSP ||
           opc == ReDefSP ||
           opc == ReDefGeneratorSP ||
           opc == PassSP ||
           opc == DefInlineSP ||
           opc == Call ||
           opc == CallArray ||
           opc == SpillStack ||
           opc == SpillFrame ||
           opc == CufIterSpillFrame ||
           opc == ExceptionBarrier ||
           opc == RetAdjustStack ||
           opc == InterpOne ||
           opc == InterpOneCF ||
           opc == GenericRetDecRefs ||
           opc == CheckStk ||
           opc == GuardStk ||
           opc == AssertStk ||
           opc == CastStk ||
           opc == CoerceStk ||
           opc == SideExitGuardStk  ||
           MInstrEffects::supported(opc));
    return arch() == Arch::X64 ? X64::rVmSp : ARM::rVmSp;
  }

  // LdContActRec and LdAFWHActRec, loading a generator's AR, is the only time
  // we have a pointer to an AR that is not in rVmFp.
  bool abnormalFramePtr = opc == LdContActRec || opc == LdAFWHActRec;

  if (!abnormalFramePtr && dst.isA(Type::FramePtr)) {
    return arch() == Arch::X64 ? X64::rVmFp : ARM::rVmFp;
  }

  if (opc == DefMIStateBase) {
    assert(dst.isA(Type::PtrToCell));
    return arch() == Arch::X64 ? PhysReg(reg::rsp) : PhysReg(vixl::sp);
  }
  return InvalidReg;
}

void LinearScan::allocRegToInstruction(InstructionList::iterator it) {
  IRInstruction* inst = &*it;
  dumpIR(inst, "allocating to instruction");

  // Reload all source operands if necessary.
  // Mark registers as unpinned.
  for (auto r : m_regs) {
    m_regs[r].m_pinned = false;
  }
  smart::vector<bool> needsReloading(inst->numSrcs(), true);
  for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
    SSATmp* tmp = inst->src(i);
    int32_t slotId = m_spillSlots[tmp];
    if (slotId == -1) {
      needsReloading[i] = false;
    } else if ((tmp = m_slots[slotId].latestReload)) {
      needsReloading[i] = false;
      inst->setSrc(i, tmp);
    }
    if (!needsReloading[i]) {
      for (int i = 0, n = m_allocInfo[tmp].numAllocated(); i < n; ++i) {
        m_regs[m_allocInfo[tmp].reg(i)].m_pinned = true;
      }
    }
  }
  for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
    if (needsReloading[i]) {
      SSATmp* tmp = inst->src(i);
      int32_t slotId = m_spillSlots[tmp];
      // <tmp> is spilled, and not reloaded.
      // Therefore, We need to reload the value into a new SSATmp.

      // Insert the Reload instruction.
      SSATmp* spillTmp = m_slots[slotId].spillTmp;
      IRInstruction* reload = m_unit.gen(Reload, inst->marker(),
                                              spillTmp);
      inst->block()->insert(it, reload);

      // Create <reloadTmp> which inherits <tmp>'s slot ID and
      // <spillTmp>'s last use ID.
      // Replace <tmp> with <reloadTmp> in <inst>.
      SSATmp* reloadTmp = reload->dst();
      m_uses[reloadTmp].lastUse = m_uses[spillTmp].lastUse;
      m_spillSlots[reloadTmp] = slotId;
      inst->setSrc(i, reloadTmp);
      // reloadTmp and tmp share the same type.  Since it was spilled, it
      // must be using its entire needed-count of registers.
      assert(reloadTmp->type() == tmp->type());
      for (int locIndex = 0; locIndex < tmp->numWords();) {
        locIndex += allocRegToTmp(reloadTmp, locIndex);
      }
      // Remember this reload tmp in case we can reuse it in later blocks.
      m_slots[slotId].latestReload = reloadTmp;
      dumpIR(reload, "created reload");
    }
  }

  freeRegsAtId(m_linear[inst]);
  // Update next native.
  if (nextNative() == inst) {
    assert(!m_natives.empty());
    m_natives.pop_front();
    computePreColoringHint();
  }

  Range<SSATmp*> dsts = inst->dsts();
  if (dsts.empty()) return;

  for (SSATmp& dst : dsts) {
    for (int numAllocated = 0, n = dst.numWords(); numAllocated < n; ) {
      auto reg = forceAlloc(dst);
      if (reg != InvalidReg) {
        assignRegToTmp(&m_regs[reg], &dst, 0);
        numAllocated++;
        continue;
      }

      if (!RuntimeOption::EvalHHIRDeadCodeElim || m_uses[dst].lastUse != 0) {
        numAllocated += allocRegToTmp(&dst, numAllocated);
      } else {
        numAllocated++;
      }
    }
  }

  if (!RuntimeOption::EvalHHIRDeadCodeElim) {
    // if any outputs were unused, free regs now.
    freeRegsAtId(m_linear[inst]);
  }
}

bool LinearScan::crossNativeCall(const SSATmp* tmp) const {
  return m_uses[tmp].lastUse > nextNativeId();
}

/*
 * Allocates a register to ssaTmp's index component (0 for value, 1 for type).
 * Returns the number of 64-bit register-space allocated.  This is normally 1,
 * but it's 2 when both the type and value need registers and they're allocated
 * together to one 128-bit SIMD register.
 */
int LinearScan::allocRegToTmp(SSATmp* ssaTmp, uint32_t index) {
  bool preferCallerSaved = true;
  PhysReg::Type regType = getRegType(ssaTmp, index);
  FTRACE(6, "getRegType(SSATmp {}, {}) = {}\n", ssaTmp->id(),
         index, int(regType));
  assert(regType == PhysReg::GP || index == 0); // no type-only in SIMD regs

  if (RuntimeOption::EvalHHIREnableCalleeSavedOpt) {
    preferCallerSaved = !crossNativeCall(ssaTmp);
  }

  RegState* reg = nullptr;
  if (!preferCallerSaved) {
    reg = getFreeReg(regType, false);
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
    if (targetRegNo == reg::noreg && ssaTmp->inst()->op() == AssertType) {
      targetRegNo = m_allocInfo[ssaTmp->inst()->src(0)].reg(index);
    }
    if (targetRegNo != reg::noreg) {
      reg = getReg(&m_regs[PhysReg(targetRegNo)]);
    }
  }
  if (reg == nullptr &&
      RuntimeOption::EvalHHIREnablePreColoring &&
      ssaTmp->inst()->isNative()) {
    // Pre-colors ssaTmp if it's the return value of a native.
    if (index == 0) {
      reg = getReg(&m_regs[rax]);
    } else if (index == 1) {
      reg = getReg(&m_regs[rdx]);
    } else {
      not_reached();
    }
  }
  if (reg == nullptr) {
    // No pre-coloring for this tmp.
    // Pick a regular caller-saved reg.
    reg = getFreeReg(regType, true);
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
    m_uses[ssaTmp].lastUse = nextNativeId();
  }

  assignRegToTmp(reg, ssaTmp, index);

  if (m_allocInfo[ssaTmp].isFullSIMD()) {
    // Type and value allocated together to a single SIMD register
    return 2;
  }
  return 1;
}

void LinearScan::assignRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index) {
  reg->m_ssaTmp = ssaTmp;
  // mark inst as using this register
  if (ssaTmp->numWords() == 2 && reg->type() == PhysReg::SIMD) {
    assert(index == 0);
    m_allocInfo[ssaTmp].setRegFullSIMD(reg->m_reg);
  } else {
    m_allocInfo[ssaTmp].setReg(reg->m_reg, index);
  }
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

class SpillLocManager {
 public:
  explicit SpillLocManager(uint32_t startSpillLoc) :
      m_nextSpillLoc(startSpillLoc) { }

  /*
   * Allocates a new spill location.
   */
  uint32_t allocSpillLoc() {
    return m_nextSpillLoc++;
  }

  void alignTo16Bytes() {
    if (PhysLoc::offset(m_nextSpillLoc) % 16 != 0) {
      m_nextSpillLoc++;
    }
  }

  uint32_t getNumSpillLocs() const {
    return m_nextSpillLoc;
  }

  void setNextSpillLoc(uint32_t nextSpillLoc) {
    m_nextSpillLoc = nextSpillLoc;
  }

 private:
  uint32_t m_nextSpillLoc;
};

// Assign spill location numbers to Spill/Reload.
uint32_t LinearScan::assignSpillLoc() {
  uint32_t maxSpillLoc = 0;
  SpillLocManager spillLocManager(0);

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
      spillLocManager.setNextSpillLoc(it->second);
    }
    for (IRInstruction& inst : *block) {
      if (nextNative() == &inst) {
        assert(!m_natives.empty());
        m_natives.pop_front();
      }
      if (inst.op() == Spill) {
        SSATmp* dst = inst.dst();
        SSATmp* src = inst.src(0);
        TRACE(3, "[counter] 1 spill a tmp that %s native\n",
              crossNativeCall(dst) ? "spans" : "does not span");
        for (int locIndex = 0;
             locIndex < src->numWords();
             ++locIndex) {

          // SSATmps with 2 regs are aligned to 16 bytes because they may be
          // allocated to SIMD registers, either before or after being reloaded
          if (src->numWords() == 2 && locIndex == 0) {
            spillLocManager.alignTo16Bytes();
          }
          auto spillLoc = spillLocManager.allocSpillLoc();
          m_allocInfo[dst].setSlot(locIndex, spillLoc);

          if (m_allocInfo[src].isFullSIMD()) {
            // Allocate the next, consecutive spill slot for this SSATmp too
            spillLoc = spillLocManager.allocSpillLoc();
            m_allocInfo[dst].setSlot(1, spillLoc);
            break;
          }
        }
      }
      if (inst.op() == Reload) {
        SSATmp* src = inst.src(0);
        for (int locIndex = 0;
             locIndex < src->numWords();
             ++locIndex) {
          TRACE(3, "[counter] reload\n");
        }
      }
    }
    uint32_t totalSpillLocs = spillLocManager.getNumSpillLocs();
    if (totalSpillLocs > maxSpillLoc) maxSpillLoc = totalSpillLocs;
    if (block->trace()->isMain()) {
      if (Block* taken = block->taken()) {
        if (!taken->trace()->isMain()) {
          exitLocMap[taken] = totalSpillLocs;
        }
      }
    }
  }
  return maxSpillLoc;
}

void LinearScan::collectInfo(BlockList::iterator it, IRTrace* trace) {
  m_natives.clear();
  m_uses.reset(); // TODO(#2536764): serious time sink

  while (it != m_blocks.end()) {
    Block* block = *it++;
    bool offTrace = block->trace() != trace;
    if (offTrace) {
      if (!trace->isMain()) return;
      int lastId = m_exitIds[block->trace()];
      for (IRInstruction& inst : *block) {
        for (auto* src : inst.srcs()) {
          if (lastId > m_uses[src].lastUse) {
            m_uses[src].lastUse = lastId;
          }
        }
      }
    } else {
      for (IRInstruction& inst : *block) {
        for (auto* src : inst.srcs()) {
          m_uses[src].lastUse = m_linear[inst];
        }
        if (inst.isNative()) m_natives.push_back(&inst);
      }

      IRInstruction* jmp = &block->back();
      if (jmp->op() == Jmp && jmp->numSrcs() != 0) {
        for (SSATmp* src : jmp->srcs()) {
          m_jmps[src].push_back(jmp);
        }
      }
    }
  }
}

void LinearScan::computePreColoringHint() {
  m_preColoringHint.clear();
  IRInstruction* inst = nextNative();
  if (inst == nullptr) {
    return;
  }

  Opcode opc = inst->op();
  using NativeCalls::CallMap;
  using NativeCalls::ArgType;
  if (CallMap::hasInfo(opc)) {
    unsigned reg = 0;
    for (auto const& arg : CallMap::info(opc).args) {
      switch (arg.type) {
        case ArgType::SSA:
          m_preColoringHint.add(inst->src(arg.ival), 0, reg++);
          break;
        case ArgType::TV:
        case ArgType::MemberKeyS:
        case ArgType::MemberKeyIS:
          m_preColoringHint.add(inst->src(arg.ival), 0, reg++);
          m_preColoringHint.add(inst->src(arg.ival), 1, reg++);
          break;
        case ArgType::ExtraImm:
        case ArgType::Imm:
          break;
      }
      // Some opcodes (ex. SetM) can have more arguments than there are argument
      // registers. These will always spill so don't do any coloring for them.
      if (reg >= X64::kNumRegisterArgs) {
        break;
      }
    }
    return;
  }

  // For instructions that want to hint a continuous increasing range
  // of sources to a continuous increasing range of argument
  // registers.
  auto normalHint = [&](int count, int srcBase, int argBase) {
    for (int i = 0; i < count; ++i) {
      m_preColoringHint.add(inst->src(i + srcBase), 0,
                            i + argBase);
    }
  };
  switch (opc) {
    case LdFunc:
      m_preColoringHint.add(inst->src(0), 0, 1);
      break;
    case NativeImpl:
      m_preColoringHint.add(inst->src(1), 0, 0);
      break;
    case AKExists:
      normalHint(2, 0, 0);
      break;
    case Eq:
    case Neq:
    case Same:
    case NSame:
      {
        auto src1 = inst->src(0);
        auto src2 = inst->src(1);

        auto type1 = src1->type();
        auto type2 = src2->type();

        if ((type1.isArray() && type2.isArray())
            || (type1.isString() && type2.isString())
            || (type1.isString() && !src1->isConst())
            || ((type1 == Type::Obj || type1 == Type::Res) &&
                (type2 == Type::Obj || type2 == Type::Res))) {
          m_preColoringHint.add(src1, 0, 0);
          m_preColoringHint.add(src2, 0, 1);
        }
      }
      break;
    case IterInit:
    case WIterInit:
      {
        m_preColoringHint.add(inst->src(0), 0, 1);
      }
      break;
    case LdSSwitchDestFast:
      normalHint(1, 0, 0);
      break;
    case LdSSwitchDestSlow:
      normalHint(1, 0, 0);
      break;
    case LdGblAddr:
      normalHint(1, 0, 0);
      break;
    case LdClsPropAddr:
      normalHint(3, 0, 0);
      break;
    case LdCls:
      m_preColoringHint.add(inst->src(0), 0, 1);
      break;
    case BoxPtr:
      normalHint(1, 0, 0);
      break;
    default:
      break;
  }
}

// Given a label, dest index for that label, and register index, scan
// the sources of all incoming Jmps to see if any have a register
// allocated at the specified index.
static RegNumber findLabelSrcReg(StateVector<SSATmp,PhysLoc>& regs,
                                 IRInstruction* label, unsigned dstIdx,
                                 uint32_t regIndex) {
  assert(label->op() == DefLabel);
  SSATmp* withReg = label->block()->findSrc(dstIdx, [&](SSATmp* src) {
    return regs[src].reg(regIndex) != InvalidReg &&
      src->inst()->block()->hint() != Block::Hint::Unlikely;
  });
  return withReg ? regs[withReg].reg(regIndex) : reg::noreg;
}

// This function attempts to find a pre-coloring hint from two
// different sources: If tmp comes from a DefLabel, it will scan up to
// the SSATmps providing values to incoming Jmps to look for a
// hint. If tmp is consumed by a Jmp, look for other incoming Jmps
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
    auto reg = m_allocInfo[tmp].reg(regIndex);
    assert(reg != reg::noreg);
    return reg;
  }

  if (srcInst->op() == DefLabel) {
    // Figure out which dst of the label is tmp
    for (unsigned i = 0, n = srcInst->numDsts(); i < n; ++i) {
      if (srcInst->dst(i) == tmp) {
        auto reg = findLabelSrcReg(m_allocInfo, srcInst, i, regIndex);
        // It's ok for reg to be reg::noreg here if all the incoming values are
        // constant.
        return reg;
      }
    }
    not_reached();
  }

  // If srcInst wasn't a label, check if tmp is used by any Jmp
  // instructions. If it is, trace to the Jmp's label and use the
  // same procedure as above.
  for (unsigned ji = 0, jn = jmps.size(); ji < jn; ++ji) {
    IRInstruction* jmp = jmps[ji];
    IRInstruction* label = &jmp->taken()->front();

    // Figure out which src of the Jmp is tmp
    for (unsigned si = 0, sn = jmp->numSrcs(); si < sn; ++si) {
      SSATmp* src = jmp->src(si);
      if (tmp == src) {
        // For now, a DefLabel should never have a register assigned
        // to it before any of its incoming Jmp instructions.
        always_assert(m_allocInfo[label->dst(si)].reg(regIndex) ==
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
  for (auto r : m_regs) {
    if (!m_regs[r].m_reserved) {
      pushFreeReg(&m_regs[r]);
    }
  }
}

void LinearScan::coalesce() {
  forEachTraceInst(m_unit, [](IRInstruction* inst) {
    for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
      SSATmp* src = inst->src(i);
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
      uint32_t id = nextId++;
      m_linear[inst] = id;
      for (SSATmp* tmp : inst.srcs()) {
        m_uses[tmp].lastUse = id;
        m_uses[tmp].count++;
      }
    }
    if (block->taken() && block->isMain() && !block->taken()->isMain()) {
      // reserve a spot for the lastUseId when we're processing the main
      // trace, if the last use is really in an exit trace.
      m_exitIds[block->taken()->trace()] = nextId++;
    }
  }
}

void LinearScan::genSpillStats(int numSpillLocs) {
  if (!moduleEnabled(HPHP::Trace::statgroups, 1)) return;
  static bool enabled = getenv("HHVM_STATS_SPILLS");
  if (!enabled) return;

  int numMainSpills = 0;
  int numExitSpills = 0;
  int numMainReloads = 0;
  int numExitReloads = 0;
  forEachInst(
    m_blocks,
    [&](IRInstruction* inst) {
      if (inst->op() == Spill) {
        if (inst->block()->isMain()) {
          numMainSpills++;
        } else {
          numExitSpills++;
        }
      } else if (inst->op() == Reload) {
        if (inst->block()->isMain()) {
          numMainReloads++;
        } else {
          numExitReloads++;
        }
      }
    }
  );

  static StringData* spillStats = makeStaticString("SpillStats");
  static StringData* mainSpills = makeStaticString("MainSpills");
  static StringData* mainReloads = makeStaticString("MainReloads");
  static StringData* exitSpills = makeStaticString("ExitSpills");
  static StringData* exitReloads = makeStaticString("ExitReloads");
  static StringData* spillSpace = makeStaticString("SpillSpace");

  auto entry = m_unit.entry(); // entry block
  auto const marker = entry->front().marker();
  auto addStat = [&](const StringData* key, int value) {
    entry->prepend(m_unit.gen(IncStatGrouped, marker,
                              cns(spillStats), cns(key), cns(value)));
  };
  addStat(mainSpills, numMainSpills);
  addStat(mainReloads, numMainReloads);
  addStat(exitSpills, numExitSpills);
  addStat(exitReloads, numExitReloads);
  addStat(spillSpace, numSpillLocs);
}

/*
 * Finds the set of SSATmps that should be considered for allocation
 * to a full SIMD register.  These are the SSATmps that satisfy all the
 * following conditions:
 *   a) it requires 2 64-bit registers
 *   b) it's defined in a load instruction
 *   c) all its uses are simple stores to memory
 *
 * The computed set of SSATmps is stored in m_fullSIMDCandidates.
 */
void LinearScan::findFullSIMDCandidates() {
  boost::dynamic_bitset<> notCandidates(m_unit.numTmps());
  m_fullSIMDCandidates.reset();
  for (auto* block : m_blocks) {
    for (auto& inst : *block) {
      for (SSATmp& tmp : inst.dsts()) {
        if (tmp.numWords() == 2 && inst.isLoad() &&
            !inst.isControlFlow()) {
          m_fullSIMDCandidates[tmp.id()] = true;
        }
      }
      int idx = 0;
      for (SSATmp* tmp : inst.srcs()) {
        if (tmp->numWords() == 2 && !inst.storesCell(idx)) {
          notCandidates[tmp->id()] = true;
        }
        idx++;
      }
    }
  }
  m_fullSIMDCandidates -= notCandidates;
}

// Insert a Shuffle just before each Jmp, to copy the Jmp's src values
// to the target label's assigned destination registers.
void LinearScan::resolveJmpCopies() {
  for (auto b : m_blocks) {
    if (!b->taken()) continue;
    auto jmp = &b->back();
    auto n = jmp->numSrcs();
    if (jmp->op() == Jmp && n > 0) {
      auto srcs = jmp->srcs();
      auto dests = new (m_unit.arena()) PhysLoc[n];
      auto labelDests = jmp->taken()->front().dsts();
      for (unsigned i = 0; i < n; ++i) {
        dests[i] = m_allocInfo[labelDests[i]];
      }
      auto shuffle = m_unit.gen(Shuffle, jmp->marker(),
                                ShuffleData(dests, n, n),
                                std::make_pair(n, &srcs[0]));
      b->insert(b->iteratorTo(jmp), shuffle);
    }
  }
}

RegAllocInfo LinearScan::allocRegs() {
  // Pre: Ensure there are no existing Shuffle instructions
  assert(checkNoShuffles(m_unit));

  if (RuntimeOption::EvalHHIREnableCoalescing) {
    // <coalesce> doesn't need instruction numbering.
    coalesce();
  }

  m_blocks = rpoSortCfg(m_unit);
  m_idoms = findDominators(m_unit, m_blocks);

  if (!packed_tv) {
    findFullSIMDCandidates();
  }

  allocRegsToTrace();

  numberInstructions(m_blocks);

  uint32_t numSpillLocs = assignSpillLoc();
  if (numSpillLocs > (uint32_t)NumPreAllocatedSpillLocs) {
    PUNT(LinearScan_TooManySpills);
  }

  if (m_slots.size()) genSpillStats(numSpillLocs);

  resolveJmpCopies();
  auto regs = computeRegs();
  if (dumpIREnabled()) {
    dumpTrace(kRegAllocLevel, m_unit, " after reg alloc ", &regs,
              &m_lifetime, nullptr, nullptr);
  }
  return regs;
}

void LinearScan::allocRegsOneTrace(BlockList::iterator& blockIt,
                                   ExitTraceMap& etm) {
  auto const trace = (*blockIt)->trace();

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
    if (block->trace() != trace) {
      if (!isMain) {
        break;
      } else {
        ++blockIt;
        continue;
      }
    }
    FTRACE(5, "Block{}: {} ({})\n",
           trace->isMain() ? "" : " (exit trace)",
           (*blockIt)->id(), (*blockIt)->postId());

    // clear remembered reloads that don't dominate this block
    for (SlotInfo& slot : m_slots) {
      if (SSATmp* reload = slot.latestReload) {
        if (!dominates(reload->inst()->block(), block, m_idoms)) {
          slot.latestReload = nullptr;
        }
      }
    }
    for (auto it = block->begin(), end = block->end(); it != end; ++it) {
      allocRegToInstruction(it);
      dumpIR(&*it, "allocated to instruction ");
    }
    if (isMain) {
      assert(block->trace()->isMain());
      if (block->taken() &&
          !block->taken()->trace()->isMain()) {
        etm[block->taken()].save(this);
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
    IRInstruction* inst = spill->src(0)->inst();
    Block* block = inst->block();
    if (!isMain && block->trace()->isMain()) {
      // We're on an exit trace, but the def is on the
      // main trace, so put it at the start of this trace
      if (spill->block()) {
        // its already been inserted in another exit trace
        assert(!spill->block()->trace()->isMain());
        spill = m_unit.cloneInstruction(spill);
      }
      trace->front()->prepend(spill);
    } else if (inst->isBlockEnd()) {
      block->next()->prepend(spill);
    } else {
      auto pos = block->iteratorTo(inst);
      block->insert(++pos, spill);
    }
  }
}

void LinearScan::allocRegsToTrace() {
  ExitTraceMap etm;

  numberInstructions(m_blocks);

  if (HPHP::Trace::moduleEnabled(HPHP::Trace::hhir, 5)) {
    std::stringstream s;
    s << "RPO: ";
    for (auto& b : m_blocks) {
      s << folly::format("{}{} ",
                         b->isMain() ? "M" : "E",
                         b->id());
    }
    s << "\n";
    HPHP::Trace::traceRelease("%s\n", s.str().c_str());
  }

  BlockList::iterator it = m_blocks.begin();
  while (it != m_blocks.end()) {
    allocRegsOneTrace(it, etm);
  }

  for (it = m_blocks.begin(); it != m_blocks.end();) {
    if ((*it)->isMain()) {
      ++it;
      continue;
    }
    allocRegsOneTrace(it, etm);
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
  auto type = reg->type();
  auto& freeList = (reg->isCallerSaved() ?
                    m_freeCallerSaved[type] : m_freeCalleeSaved[type]);
  freeList.erase(reg->m_pos);
  // Pin it so that other operands in the same instruction will not reuse it.
  reg->m_pinned = true;
  return reg;
}

LinearScan::RegState* LinearScan::getFreeReg(PhysReg::Type type,
                                             bool          preferCallerSaved) {
  if (m_freeCallerSaved[type].empty() && m_freeCalleeSaved[type].empty()) {
    assert(!m_allocatedRegs.empty());

    // no free registers --> free a register from the allocatedRegs
    // Pick the first register in <m_allocatedRegs> that is:
    // 1. not used for any source operand in the current instruction, and
    // 2. not used for the return address of a function.
    auto canSpill = [&] (RegState* reg) {
      return !reg->isPinned() && !reg->isRetAddr() && reg->type() == type;
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
    preferred = &m_freeCallerSaved[type];
    other = &m_freeCalleeSaved[type];
  } else {
    preferred = &m_freeCalleeSaved[type];
    other = &m_freeCallerSaved[type];
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
  PhysReg::Type type = reg->type();
  auto& freeList = (reg->isCallerSaved() ?
                    m_freeCallerSaved[type] : m_freeCalleeSaved[type]);
  // If next native is going to use <reg>, put <reg> to the back of the
  // queue so that it's unlikely to be misused by irrelevant tmps.
  if (RuntimeOption::EvalHHIREnablePreColoring &&
      type == PhysReg::GP &&
      (reg->m_reg == PhysReg(rax) || m_preColoringHint.preColorsTmp(reg))) {
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
  dumpIR(tmp, "spilling");
  // If we're spilling, we better actually have registers allocated.
  assert(m_allocInfo[tmp].numAllocated() > 0);
  assert(m_allocInfo[tmp].numWords() == tmp->numWords());

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
  auto* spillInst = m_unit.gen(Spill, tmp->inst()->marker(), tmp);
  SSATmp* spillTmp = spillInst->dst();
  SlotInfo si;
  si.spillTmp = spillTmp;
  si.latestReload = tmp;
  m_slots.push_back(si);
  // The spill slot inherits the last use ID of the spilled tmp.
  m_uses[si.spillTmp].lastUse = m_uses[tmp].lastUse;
  return slotId;
}

IRInstruction* LinearScan::nextNative() const {
  return m_natives.empty() ? nullptr : m_natives.front();
}

uint32_t LinearScan::nextNativeId() const {
  IRInstruction* next = nextNative();
  return next ? m_linear[next] : -1;
}

SSATmp* LinearScan::getSpilledTmp(SSATmp* tmp) {
  assert(tmp->inst()->op() == Reload);
  SSATmp* slot = tmp->inst()->src(0);
  assert(slot->inst()->op() == Spill);
  return slot->inst()->src(0);
}

// If <tmp> is a reloaded value, follow the spill-reload chain to find
// its source; otherwise, return <tmp> itself.
SSATmp* LinearScan::getOrigTmp(SSATmp* tmp) {
  if (tmp->inst()->op() == Reload)
    return getSpilledTmp(tmp);
  return tmp;
}

bool LinearScan::PreColoringHint::preColorsTmp(RegState* reg) const {
  assert(reg->m_reg.isGP());
  return m_preColoredTmps[reg->m_reg].first != nullptr;
}

// Get the pre-coloring register of (<tmp>, <index>).
// A native call has at most six arguments, so the time complexity is
// not a big problem.
PhysReg LinearScan::PreColoringHint::getPreColoringReg(
    SSATmp* tmp, uint32_t index) const {
  for (auto reg : m_preColoredTmps) {
    if (m_preColoredTmps[reg].first == tmp &&
        m_preColoredTmps[reg].second == index) {
      assert(reg.isGP());
      return reg;
    }
  }
  return InvalidReg;
}

void LinearScan::PreColoringHint::clear() {
  for (auto reg : m_preColoredTmps) {
    m_preColoredTmps[reg].first = nullptr;
    m_preColoredTmps[reg].second = 0;
  }
}

// Provide a hint that (<tmp>, <index>) is used as the <argNum>-th arg
// in next native.
void LinearScan::PreColoringHint::add(SSATmp* tmp, uint32_t index, int argNum) {
  assert(argNum < X64::kNumRegisterArgs);
  auto reg = X64::argNumToRegName[argNum];
  assert(reg != InvalidReg && reg.isGP());
  m_preColoredTmps[reg].first  = tmp;
  m_preColoredTmps[reg].second = index;
}

//////////////////////////////////////////////////////////////////////

RegAllocInfo allocRegsForUnit(IRUnit& unit) {
  return LinearScan(unit).allocRegs();
}

}} // HPHP::JIT
