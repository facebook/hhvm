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

#ifndef _LINEAR_SCAN_H_
#define _LINEAR_SCAN_H_

#include "ir.h"
#include "tracebuilder.h"
#include "codegen.h"

namespace HPHP {
namespace VM {
namespace JIT {

class LinearScan {
public:
  // TODO: remove these in favor of using the abi-x64.h constants.
  const static register_name_t noReg = reg::noreg;
  const static register_name_t rVmSP = Transl::rVmSp;
  const static register_name_t rSP = reg::rsp;
  const static register_name_t rVmFP = Transl::rVmFp;
  const static register_name_t rScratch = reg::rScratch;
  const static register_name_t rTlPtr = Transl::rVmTl;
  const static register_name_t rStashedAR = Transl::rStashedAR;

  static const int NumRegs = 16;
  static const int NumMmxRegs = 8;
  // See the comments for SSATmp::m_assignedLoc in ir.h
  // to find out how we use FirstSpill and FirstMmxReg.
  static const int FirstMmxReg = NumRegs;
  static const int FirstSpill = FirstMmxReg + NumMmxRegs;
  // This value must be consistent with the number of pre-allocated bytes
  // for spill locations in __enterTCHelper in translator-x64.cpp.
  // Be very careful when changing this value.
  static const int NumPreAllocatedSpillLocs = 16;

  // HHIR:TODO ideally wouldn't need to use ints, but very helpful for bit ops
  static int regNameAsInt(register_name_t r) { return (int)r; }

#define REG_MASK(regNo) (1 << (uint)(regNo))

  const static int CallerSavedRegMask =
                              REG_MASK(reg::rax) |
                              REG_MASK(reg::rcx) |
                              REG_MASK(reg::rdx) |
                              REG_MASK(reg::rsi) |
                              REG_MASK(reg::rdi) |
                              REG_MASK(reg::r8)  |
                              REG_MASK(reg::r9)  |
                              REG_MASK(reg::r10) |
                              REG_MASK(reg::r11);

  const static int NumCallerSavedRegs = 9;
  static inline register_name_t getCallerSavedReg(int i) {
    ASSERT(i < NumCallerSavedRegs);
    return CallerSavedRegs[i];
  }

  static inline bool isCallerSavedReg(register_name_t regNo) {
    return CallerSavedRegMask & REG_MASK(LinearScan::regNameAsInt(regNo));
  }

  // HHIR:TODO:MERGE remove this int version just use register_name_t
  static inline int getRegMask(int regNo) {
    ASSERT(regNo < NumRegs && regNo >= 0);
    return REG_MASK(regNo);
  }

  static inline int getRegMask(register_name_t regNo) {
    ASSERT(LinearScan::regNameAsInt(regNo) < NumRegs &&
           LinearScan::regNameAsInt(regNo) >= 0);
    return REG_MASK(LinearScan::regNameAsInt(regNo));
  }

  class RegState {
    friend class LinearScan;

  public:
    bool isReserved() const { return m_reserved; }
    bool isCallerSaved() const {
      return CallerSavedRegMask & (1 << m_regNo);
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
    register_name_t getPreColoringReg(SSATmp* tmp, uint32 index) const;
    void clear();
    void add(SSATmp* tmp, uint32 index, int argNum);
  private:
    // indexed by arg number
    std::pair<SSATmp*, uint32> m_preColoredTmps[LinearScan::NumRegs];
  };

  LinearScan(IRFactory *irFactory, TraceBuilder* traceBuilder);
  void allocRegsToTrace(Trace* trace);
  void allocRegToInstruction(Trace* trace,
                             IRInstruction::Iterator it);
  void allocRegToTmp(RegState* reg, SSATmp* ssaTmp, uint32_t index);
  void allocRegToTmp(SSATmp* ssaTmp, uint32_t index);
  void freeRegsAtId(uint32_t id);
  void spill(SSATmp* tmp);
  static void printLoc(std::ostream& os, int loc);
  static uint32 computeLiveOutRegs(Trace* trace, uint32 liveOutRegs);
  static uint32 computeLiveOutRegs(IRInstruction* inst, uint32 liveRegs);

private:
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
                        std::vector<SSATmp*> localValues);
  void removeUnusedSpills(Trace* trace);
  void removeUnusedSpillsAux(Trace* trace);
  void collectNatives(Trace* trace);
  void collectNativesAux(Trace* trace);
  void computePreColoringHint();
  IRInstruction* getNextNative() const;
  uint32 getNextNativeId() const;

  static const register_name_t CallerSavedRegs[];
  static const char* RegNames[NumRegs];

  void pushFreeReg(RegState* reg);
  RegState* popFreeReg(std::list<RegState*>& freeList);
  void freeReg(RegState* reg);
  RegState* getFreeReg(bool preferCallerSaved);
  RegState* getReg(RegState* reg);

  // Register allocation may generate Spill/Reload.
  IRFactory* m_irFactory;
  TraceBuilder* m_tb;
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

}}} // HPHP::VM::JIT

#endif // _LINEAR_SCAN_H_
