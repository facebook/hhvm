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

#include "runtime/vm/translator/hopt/state_vector.h"
#include "runtime/vm/translator/physreg.h"
#include "runtime/vm/translator/abi-x64.h"

namespace HPHP { namespace VM { namespace JIT {

class Trace;
class IRFactory;

struct UseInfo {
  UseInfo() : lastUse(0), count(0) {}
  uint32_t lastUse; // linear id of last use
  uint32_t count;   // number of uses
};

typedef StateVector<IRInstruction, uint32_t> LinearIdVector;
typedef StateVector<SSATmp, UseInfo> UsesVector;

struct LifetimeInfo {
  explicit LifetimeInfo(const IRFactory* factory)
    : linear(factory, 0), uses(factory, UseInfo()) {
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

  // return offset in 8-byte-words from stack pointer
  uint32_t mem() const { return m_val; }

  bool isValid() const { return int(m_val) != int(Transl::InvalidReg); }

private:
  uint32_t m_val;
};

// Register allocation info about one SSATmp
class RegisterInfo {
  enum { kMaxNumRegs = 2 };

public:
  RegisterInfo() : m_isSpilled(false) {
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
  PhysReg getReg() const {
    assert(!m_isSpilled);
    return m_regs[0];
  }

  PhysReg getReg(uint32_t i) const {
    assert(!m_isSpilled);
    return m_regs[i];
  }

  void setReg(PhysReg reg, uint32_t i) {
    assert(!m_isSpilled);
    m_regs[i] = reg;
  }

  bool spilled() const {
    return m_isSpilled;
  }

  /* Returns the set of registers in this RegisterInfo */
  RegSet getRegs() const;

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

  SpillInfo getSpillInfo(int idx) const {
    assert(m_isSpilled);
    return m_spillInfo[idx];
  }

private:
  bool m_isSpilled;
  union {
    PhysReg m_regs[kMaxNumRegs];
    SpillInfo m_spillInfo[kMaxNumRegs];
  };
};

struct RegAllocInfo {
  explicit RegAllocInfo(const IRFactory* factory)
    : m_regs(factory, RegisterInfo()) {}
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
  os << "spill[" << si.mem() << "]";
  return os;
}

/*
 * The main entry point for register allocation.  Called prior to code
 * generation.
 */
RegAllocInfo allocRegsForTrace(Trace*, IRFactory*, LifetimeInfo* = nullptr);

}}}

#endif
