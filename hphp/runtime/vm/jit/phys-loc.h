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

#ifndef incl_HPHP_VM_PHYS_LOC_H_
#define incl_HPHP_VM_PHYS_LOC_H_

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP { namespace JIT {

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
 * PhysLoc contains Register/spill locations for one SSATmp
 */
class PhysLoc {
  enum Kind { kRegister, kSIMD, kSpill };

public:
  PhysLoc() : m_kind(kRegister) {
    m_regs[0] = m_regs[1] = InvalidReg;
  }

  /*
   * Returns whether or not a given register index is allocated to a
   * register, or returns false if it is spilled.
   *
   * Right now, we only spill both at the same time and only Spill and
   * Reload instructions need to deal with SSATmps that are spilled.
   */
  bool hasReg(uint32_t i = 0) const {
    return m_kind != kSpill && m_regs[i] != InvalidReg;
  }

  /*
   * The number of registers or spill slots actually allocated.  This might
   * end up fewer than SSATmp::numWords if the SSATmp isn't used, is
   * a constant, or if we allocated a full SIMD register to a 2-word tmp.
   */
  int numAllocated() const;

  /*
   * Number of words held by this PhysLoc.  If isFullSIMD() is true this returns
   * 2, otherwise it returns numAllocated().
   */
  int numWords() const;

  /*
   * Access to allocated registers.
   *
   * Returns InvalidReg for slots that aren't allocated.
   */
  PhysReg reg() const {
    assert(m_kind != kSpill);
    return m_regs[0];
  }

  PhysReg reg(uint32_t i) const {
    assert(m_kind != kSpill);
    return m_regs[i];
  }

  void setReg(PhysReg reg, uint32_t i) {
    assert(m_kind != kSpill);
    m_regs[i] = reg;
  }

  /*
   * Used when the SSATmp needs two 64-bit registers and got assigned
   * one 128-bit SIMD register.
   */
  void setRegFullSIMD(PhysReg reg) {
    assert(reg.isSIMD() && m_kind != kSpill);
    m_regs[0] = reg;
    m_kind = kSIMD;
  }

  bool spilled() const {
    return m_kind == kSpill;
  }

  /*
   * Returns whether the SSATmp needed 2 regs and was allocated to a
   * whole 128-bit SIMD register.
   */
  bool isFullSIMD() const {
    return m_kind == kSIMD;
  }

  /* Returns the set of registers in this PhysLoc */
  RegSet regs() const;

  /*
   * Set/get information about this operand's spill location.
   * slot is the logical slot number, offset is the offset in bytes
   * from RSP.
   */
  void setSlot(int idx, uint32_t slot) {
    m_slots[idx] = slot;
    m_kind = kSpill;
  }

  uint32_t slot(int idx) const {
    assert(m_kind == kSpill);
    return m_slots[idx];
  }

  /*
   * compute the offset from RSP for a logical spill slot.  Given a logical
   * slot number, return a byte offset from RSP, taking into account the layout
   * above.  LinearScan punts if any extra spill locations would be required,
   * so all we really need to do is adjust for the return address and scale
   * by the machine word size.
   */
  static int offset(uint32_t slot) {
    return (slot + 1) * sizeof(uint64_t);
  }

  uint32_t offset(int idx) const {
    assert(m_kind == kSpill);
    return offset(m_slots[idx]);
  }

  bool operator==(const PhysLoc& other) const {
    return m_kind == other.m_kind &&
           m_regs[0] == other.m_regs[0] &&
           m_regs[1] == other.m_regs[1];
  }

  bool operator!=(const PhysLoc& other) const {
    return !(*this == other);
  }

private:
  Kind m_kind;
  union {
    PhysReg m_regs[2];
    uint32_t m_slots[2];
  };
};

std::ostream& operator<<(std::ostream& os, const PhysLoc&);

}}

#endif
