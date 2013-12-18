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

#ifndef incl_HPHP_VM_LINEAR_SCAN_H_
#define incl_HPHP_VM_LINEAR_SCAN_H_

#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/phys-loc.h"

namespace HPHP {  namespace JIT {

class IRUnit;

// This value must be consistent with the number of pre-allocated
// bytes for spill locations in __enterTCHelper in translator-x64.cpp.
// Be careful when changing this value.
const size_t NumPreAllocatedSpillLocs = X64::kReservedRSPSpillSpace /
                                        sizeof(uint64_t);

struct UseInfo {
  UseInfo() : lastUse(0), count(0) {}
  uint32_t lastUse; // linear id of last use
  uint32_t count;   // number of uses
};

typedef StateVector<IRInstruction, uint32_t> LinearIdVector;
typedef StateVector<SSATmp, UseInfo> UsesVector;

struct LifetimeInfo {
  explicit LifetimeInfo(const IRUnit& unit)
    : linear(unit, 0), uses(unit, UseInfo()) {
  }
  explicit LifetimeInfo(const LinearIdVector& linear,
                        const UsesVector& uses)
    : linear(linear), uses(uses) {
  }
  explicit LifetimeInfo(LinearIdVector&& linear,
                        UsesVector&& uses)
    : linear(linear), uses(uses) {
  }

  LinearIdVector linear; // linear id of each instruction
  UsesVector uses;       // last use id and use count of each tmp
};

struct RegAllocInfo {
  struct RegMap {
    PhysLoc& operator[](const SSATmp* k) { return m_map[k->id()]; }
    PhysLoc& operator[](const SSATmp& k) { return m_map[k.id()]; }
    const PhysLoc& operator[](const SSATmp* k) const {
      return m_map[k->id()];
    }
    const PhysLoc& operator[](const SSATmp& k) const {
      return m_map[k.id()];
    }
  private:
    mutable smart::flat_map<uint32_t,PhysLoc> m_map;
  };
  explicit RegAllocInfo(const IRUnit& unit) : m_regs(unit, RegMap()) {}
  RegAllocInfo(const RegAllocInfo& other) : m_regs(other.m_regs) {}
  RegAllocInfo(RegAllocInfo&& other) : m_regs(other.m_regs) {}
  RegMap& operator[](const IRInstruction* i) { return m_regs[i]; }
  RegMap& operator[](const IRInstruction& i) { return m_regs[i]; }
  const RegMap& operator[](const IRInstruction* i) const { return m_regs[i]; }
  const RegMap& operator[](const IRInstruction& i) const { return m_regs[i]; }
private:
  StateVector<IRInstruction,RegMap> m_regs;
};

// Return a valid register if this tmp should be forced into a particular
// register, otherwise return InvalidReg.
PhysReg forceAlloc(SSATmp& t);

/*
 * The main entry point for register allocation.  Called prior to code
 * generation.
 */
RegAllocInfo allocRegsForUnit(IRUnit&);

/*
 * New register allocator doing extended linear scan
 */
RegAllocInfo allocateRegs(IRUnit&);

}}

#endif
