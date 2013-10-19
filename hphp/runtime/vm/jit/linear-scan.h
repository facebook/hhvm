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

namespace HPHP {  namespace JIT {

class IRUnit;

// This value must be consistent with the number of pre-allocated
// bytes for spill locations in __enterTCHelper in translator-x64.cpp.
// Be careful when changing this value.
const int NumPreAllocatedSpillLocs = Transl::kReservedRSPSpillSpace /
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

// Information about one spilled value.
struct SpillInfo {
  explicit SpillInfo(uint32_t v) : m_val(v) {
    assert(isValid());
  }

  // Return logical slot number
  uint32_t slot() const {
    assert(isValid());
    return m_val;
  }

  bool isValid() const {
    return int(m_val) != int(Transl::InvalidReg);
  }

  // return the offset from RSP for this slot; takes into account
  // the native stack layout.
  int offset() const;

private:
  uint32_t m_val;
};

// Register allocation info about one SSATmp
class RegisterInfo {
  enum { kMaxNumRegs = 2 };

public:
  RegisterInfo()
      : m_isSpilled(false)
      , m_fullXMM(false) {
    m_regs[0] = m_regs[1] = Transl::InvalidReg;
  }

  /*
   * Returns whether or not a given register index is allocated to a
   * register, or returns false if it is spilled.
   *
   * Right now, we only spill both at the same time and only Spill and
   * Reload instructions need to deal with SSATmps that are spilled.
   */
  bool hasReg(uint32_t i = 0) const {
    return !m_isSpilled && m_regs[i] != Transl::InvalidReg;
  }

  /*
   * The number of regs actually allocated to this SSATmp.  This might
   * end up fewer than numNeededRegs if the SSATmp isn't really
   * being used.
   */
  int numAllocatedRegs() const;

  /*
   * Access to allocated registers.
   *
   * Returns InvalidReg for slots that aren't allocated.
   */
  PhysReg reg() const {
    assert(!m_isSpilled);
    return m_regs[0];
  }

  PhysReg reg(uint32_t i) const {
    assert(!m_isSpilled);
    return m_regs[i];
  }

  void setReg(PhysReg reg, uint32_t i) {
    assert(!m_isSpilled);
    m_regs[i] = reg;
  }

  /*
   * Used when the SSATmp needs two 64-bit registers and got assigned
   * one 128-bit XMM register.
   */
  void setRegFullXMM(PhysReg reg) {
    assert(reg.isXMM());
    assert(!m_isSpilled);
    m_regs[0] = reg;
    m_fullXMM = true;
  }

  bool spilled() const {
    return m_isSpilled;
  }

  /*
   * Returns whether the SSATmp needed 2 regs and was allocated to a
   * whole 128-bit XMM register.
   */
  bool isFullXMM() const {
    return m_fullXMM;
  }

  /* Returns the set of registers in this RegisterInfo */
  RegSet regs() const;

  /*
   * Returns information about how to spill/fill a SSATmp.
   *
   * These functions are only valid if this SSATmp is being spilled or
   * filled.  In all normal instructions (i.e. other than Spill and
   * Reload), SSATmps are assigned registers instead of spill
   * locations.
   */
  void setSpillInfo(int i, SpillInfo si) {
    assert(si.isValid());
    m_spillInfo[i] = si;
    m_isSpilled = true;
  }

  SpillInfo spillInfo(int idx) const {
    assert(m_isSpilled);
    return m_spillInfo[idx];
  }

private:
  bool m_isSpilled;
  bool m_fullXMM;
  union {
    PhysReg m_regs[kMaxNumRegs];
    SpillInfo m_spillInfo[kMaxNumRegs];
  };
};

struct RegAllocInfo {
  explicit RegAllocInfo(const IRUnit& unit)
    : m_regs(unit, RegisterInfo()) {}
  RegAllocInfo(const RegAllocInfo& other) : m_regs(other.m_regs) {}
  RegAllocInfo(RegAllocInfo&& other) : m_regs(other.m_regs) {}
  RegisterInfo& operator[](const SSATmp* k) { return m_regs[k]; }
  RegisterInfo& operator[](const SSATmp& k) { return m_regs[k]; }
  const RegisterInfo& operator[](const SSATmp* k) const { return m_regs[k]; }
  const RegisterInfo& operator[](const SSATmp& k) const { return m_regs[k]; }
private:
  StateVector<SSATmp, RegisterInfo> m_regs;
};

inline std::ostream& operator<<(std::ostream& os, SpillInfo si) {
  os << "spill[" << si.slot() << "]";
  return os;
}

/*
 * The main entry point for register allocation.  Called prior to code
 * generation.
 */
RegAllocInfo allocRegsForUnit(IRUnit&);

// Native stack layout:
// |               |
// +---------------+
// |               |
// | MInstr state  |
// |               |
// +---------------+
// |               |  <-- spill[kReservedRSPSpillSpace - 1]
// |  spill slots  |  <-- spill[..]
// |               |  <-- spill[1]
// |               |  <-- spill[0]
// +---------------+
// |  return addr  |
// +---------------+
//
// We need to increase spill indexes by 1 to avoid overwriting the
// return address.

/*
 * compute the offset from RSP for a logical spill slot.  Given a logical
 * slot number, return a byte offset from RSP, taking into account the layout
 * above.  LinearScan punts if any extra spill locations would be required,
 * so all we really need to do is adjust for the return address and scale
 * by the machine word size.
 */
inline int SpillInfo::offset() const {
  return (m_val + 1) * sizeof(uint64_t);
}

}}

#endif
