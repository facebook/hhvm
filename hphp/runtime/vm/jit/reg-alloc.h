/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_REG_ALLOC_H_
#define incl_HPHP_VM_REG_ALLOC_H_

#include "hphp/runtime/vm/jit/state-vector.h"
#include <vector>
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP {  namespace JIT {

class IRUnit;

// This value must be consistent with the number of pre-allocated
// bytes for spill locations in __enterTCHelper in mc-generator.cpp.
// Be careful when changing this value.
const size_t NumPreAllocatedSpillLocs = kReservedRSPSpillSpace /
                                        sizeof(uint64_t);

struct RegAllocInfo {
  struct RegMap {
    // new way
    PhysLoc& src(unsigned i) {
      assert(i < m_dstOff);
      return at(i);
    }
    PhysLoc& dst(unsigned i) { return at(i + m_dstOff); }
    const PhysLoc& src(unsigned i) const {
      assert(i < m_dstOff);
      return at(i);
    }
    const PhysLoc& dst(unsigned i) const { return at(i + m_dstOff); }
    void resize(unsigned n) const {
      m_dstOff = n;
      m_locs.resize(n);
    }
  private:
    PhysLoc& at(unsigned i) {
      assert(i < m_locs.size());
      return m_locs[i];
    }
    const PhysLoc& at(unsigned i) const {
      assert(i < m_locs.size());
      return m_locs[i];
    }
    friend struct RegAllocInfo;
    RegMap& init(const IRInstruction* inst) const {
      if (m_locs.empty()) {
        m_dstOff = inst->numSrcs();
        m_locs.resize(inst->numSrcs() + inst->numDsts());
      }
      return *const_cast<RegMap*>(this);
    }
  private:
    mutable unsigned m_dstOff { 0 };
    mutable std::vector<PhysLoc> m_locs;
  };
  explicit RegAllocInfo(const IRUnit& unit) : m_regs(unit, RegMap()) {}
  RegAllocInfo(const RegAllocInfo& other) : m_regs(other.m_regs) {}
  RegAllocInfo(RegAllocInfo&& other) : m_regs(other.m_regs) {}
  RegMap& operator[](const IRInstruction* i) { return m_regs[i].init(i); }
  RegMap& operator[](const IRInstruction& i) { return m_regs[i].init(&i); }
  const RegMap& operator[](const IRInstruction* i) const {
    return m_regs[i].init(i);
  }
  const RegMap& operator[](const IRInstruction& i) const {
    return m_regs[i].init(&i);
  }

  RegSet srcRegs(const IRInstruction& inst) const {
    auto regs = RegSet();
    auto& map = m_regs[inst];
    for (unsigned i = 0, n = inst.numSrcs(); i < n; ++i) {
      regs |= map.src(i).regs();
    }
    return regs;
  }

  RegSet dstRegs(const IRInstruction& inst) const {
    auto regs = RegSet();
    if (inst.is(Shuffle)) {
      for (auto const& dest : *inst.extra<Shuffle>()) {
        regs |= dest.regs();
      }
    } else {
      auto& map = m_regs[inst];
      for (unsigned i = 0, n = inst.numDsts(); i < n; ++i) {
        regs |= map.dst(i).regs();
      }
    }
    return regs;
  }

private:
  StateVector<IRInstruction,RegMap> m_regs;
};

/*
 * New register allocator doing extended linear scan
 */
RegAllocInfo allocateRegs(IRUnit&);

/*
 * A Constraint represents a set of locations an operand may
 * be assigned to by the register allocator.  GP and SIMD are
 * self explanitory.  VOID means no-location, i.e. InvalidReg.
 * Only some instructions allow a VOID destination, so VOID
 * is explicit.
 */
struct Constraint {
  enum Mask: uint8_t {
    GP = 1,
    SIMD = 2,
    VOID = 4,   // used for unused dests that can be InvalidReg
    IMM = 8
  };

  /* implicit */ Constraint(Mask m)
    : m_mask(m)
    , m_reg(InvalidReg)
  {}
  /* implicit */ Constraint(PhysReg r)
    : m_mask(maskFromReg(r))
    , m_reg(r)
  {}

  Constraint& operator=(Constraint c2) {
    m_mask = c2.m_mask;
    m_reg = c2.m_reg;
    return *this;
  }

  bool operator==(Constraint c2) const {
    return m_mask == c2.m_mask && m_reg == c2.m_reg;
  }

  bool operator!=(Constraint c2) const {
    return !(*this == c2);
  }

  PhysReg reg() const { return m_reg; }

  Constraint operator|(Constraint c2) const {
    return (*this == c2) ? *this :
           Constraint(Mask(m_mask | c2.m_mask));
  }

  Constraint operator&(Constraint c2) const {
    return (*this == c2) ? *this :
           Constraint(Mask(m_mask & c2.m_mask));
  }

  Constraint operator-(Constraint c2) const {
    assert(m_reg == InvalidReg && c2.m_reg == InvalidReg);
    return Mask(m_mask & ~c2.m_mask);
  }

  Constraint& operator|=(Constraint c2) { return *this = *this | c2; }
  Constraint& operator&=(Constraint c2) { return *this = *this & c2; }
  Constraint& operator-=(Constraint c2) { return *this = *this - c2; }

  explicit operator bool() const { return m_mask != 0; }

private:
  static Mask maskFromReg(PhysReg r) {
    return r.isGP() ? GP : r.isSIMD() ? SIMD : VOID;
  }

private:
  Mask m_mask;
  PhysReg m_reg; // if valid, force this register
};

/*
 * return a constraint for the given src
 */
Constraint srcConstraint(const IRInstruction& inst, unsigned src);

/*
 * Return a constraint for the given destination.
 */
Constraint dstConstraint(const IRInstruction& inst, unsigned dst);

}}

#endif
