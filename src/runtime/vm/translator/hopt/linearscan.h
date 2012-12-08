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

#ifndef incl_HPHP_VM_LINEAR_SCAN_H_
#define incl_HPHP_VM_LINEAR_SCAN_H_

#include <boost/noncopyable.hpp>

#include "runtime/vm/translator/physreg.h"
#include "runtime/vm/translator/abi-x64.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/tracebuilder.h"
#include "runtime/vm/translator/hopt/codegen.h"

namespace HPHP { namespace VM { namespace JIT {

struct LinearScan : private boost::noncopyable {
  static const int NumRegs = 16;

  LinearScan(IRFactory*, TraceBuilder*);
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
    SSATmp* m_slotTmp;
    // the latest SSATmp that has the value of a slot and is in registers
    // If it's NULL, we have to reload this slot before using it.
    SSATmp* m_latestTmp;
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
  void allocRegToInstruction(Trace* trace,
                             IRInstruction::Iterator it);
  void allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index);
  void allocRegToTmp(SSATmp* ssaTmp, uint32_t index);
  void freeRegsAtId(uint32_t id);
  void spill(SSATmp* tmp);
  static RegSet computeLiveOutRegs(Trace* trace, RegSet = RegSet());
  static RegSet computeLiveOutRegs(IRInstruction* inst, RegSet liveRegs);

  void initFreeList();
  void coalesce(Trace* trace);
  void coalesceAux(Trace* trace);
  void allocRegsToTraceAux(Trace* trace);
  uint32 createSpillSlot(SSATmp* tmp);
  static SSATmp* canonicalize(SSATmp* tmp);
  static SSATmp* getSpilledTmp(SSATmp* tmp);
  static SSATmp* getOrigTmp(SSATmp* tmp);
  void preAllocSpillLoc(Trace* trace, uint32 numSpillLocs);
  void preAllocSpillLocAux(Trace* trace, uint32 numSpillLocs);
  uint32 assignSpillLoc(Trace* trace);
  uint32 assignSpillLocAux(Trace* trace,
                           uint32 nextSpillLoc,
                           uint32 nextMmxReg);
  void insertAllocFreeSpill(Trace* trace, uint32 numExtraSpillLocs);
  void insertAllocFreeSpillAux(Trace* trace, uint32 numExtraSpillLocs);
  void rematerialize(Trace* trace);
  void rematerializeAux(Trace* trace,
                        SSATmp* curSp,
                        SSATmp* curFp,
                        std::vector<SSATmp*> localValues);
  void removeUnusedSpills(Trace* trace);
  void removeUnusedSpillsAux(Trace* trace);
  void collectNatives(Trace* trace);
  void collectNativesAux(Trace* trace);
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
  IRFactory* m_irFactory;
  RegState   m_regs[NumRegs];
  std::list<RegState*> m_freeCallerSaved;
  std::list<RegState*> m_freeCalleeSaved;
  std::list<RegState*> m_allocatedRegs;
  // Indexed by slot ID.
  std::vector<SlotInfo> m_slots;
  // the list of native instructions in the trace
  // sorted by instruction ID
  IRInstruction::List m_natives;
  // stores pre-coloring hints
  PreColoringHint m_preColoringHint;
};

/*
 * The main entry point for register allocation.  Called prior to code
 * generation.
 */
void allocRegsForTrace(Trace*, IRFactory*, TraceBuilder*);

}}}

#endif
