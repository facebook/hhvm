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
#ifndef incl_REG_ALLOC_H_
#define incl_REG_ALLOC_H_

#include <boost/noncopyable.hpp>

#include "util/trace.h"
#include "util/asm-x64.h"
#include "runtime/vm/translator/translator.h"
#include "runtime/vm/translator/physreg.h"

namespace HPHP { namespace VM { namespace Transl {

// Assumption: the set interfaces are limited to the first 64 registers.
static const int kMaxRegs = 64;

/*
 * We take a "virtual memory" approach to register allocation. The virtual
 * registers are PHP Location's, and the physical registers are
 * an opaque collection of integers that presumably map to machine regs.
 * We assume that the machine has enough physical registers to get through
 * a single normalized instruction with all inputs and outputs in
 * registers.
 *
 * By analogy with page replacement, we do LRU to replace physical
 * registers. While this is an imperfect analogy, it provides constant
 * overheads per register, and a guarantee that any runnable code will
 * actually be able to allocate registers.
 */

struct RegContent {
  enum Kind {
    Invalid,
    Loc,
    Int
  }        m_kind;
  int64    m_int;
  Location m_loc;

  RegContent(const Location &loc, int64 intval = 0)
      : m_kind(Loc), m_int(intval), m_loc(loc) { }

  RegContent(int64 _m_int) : m_kind(Int), m_int(_m_int), m_loc(Location()) { }

  RegContent() : m_kind(Invalid), m_int(0), m_loc(Location()) { }

  bool isInt() const {
    return m_kind == Int;
  }

  bool isLoc() const {
    return m_kind == Loc;
  }

  bool isValid() const {
    return m_kind == Int || m_kind == Loc;
  }

  bool isInvalid() const {
    return !isValid();
  }

  bool isUnreachableStack(int firstUnreachable) const {
    return isLoc() && m_loc.isStack() && m_loc.offset >= firstUnreachable;
  }

  int cmp(const RegContent &other) const {
    if (m_kind != other.m_kind) {
      return m_kind - other.m_kind;
    }
    if (m_kind == Int) {
      if (m_int == other.m_int) {
        return 0;
      }
      return m_int > other.m_int ? 1 : -1;
    }
    ASSERT(m_kind == Loc);
    return m_loc.cmp(other.m_loc);
  }

  bool operator==(const RegContent &other) const {
    return cmp(other) == 0;
  }

  bool operator!=(const RegContent &other) const {
    return cmp(other) != 0;
  }

  const char *kindStr() const {
    switch (m_kind) {
      case Int : return "Int";
      case Loc : return "Loc";
      default  : return "Invalid";
    }
  }

  std::string pretty() const {
    char  buf[256];
    char  val[256] = "";
    switch (m_kind) {
      case Int : sprintf(val, "0x%llx", m_int); break;
      case Loc : sprintf(val, "%s", m_loc.pretty().data()); break;
      default  : break;
    }
    sprintf(buf, "(RegContent %s %s)", kindStr(), val);
    return std::string(buf);
  }

  // Hash function
  size_t operator()(const RegContent& cont) const {
    return HPHP::hash_int64_pair(
      cont.m_kind, cont.isInt() ? cont.m_int : cont.m_loc(cont.m_loc));
  }
};

/*
 * In the virtual memory analogy, a RegInfo is the PTE: it contains
 * the v2p mapping and dirty bit.
 */
struct RegInfo {
#define REGSTATES \
  REGSTATE(INVALID) \
  REGSTATE(FREE) REGSTATE(CLEAN) REGSTATE(SCRATCH) REGSTATE(DIRTY)
  enum State {
#define REGSTATE(x) x,
    // Order is meaningful here; we use <, > to test "strength" of state
    REGSTATES
#undef REGSTATE
  };
  RegContent m_cont;
  uint64     m_epoch;
  PhysReg    m_pReg;
  State      m_state;
  DataType   m_type;

  std::string pretty() const {
    const char* names[] = {
#define REGSTATE(x) #x,
      REGSTATES
#undef REGSTATE
    };
    char buf[1024];
    sprintf(buf, "Reg:%02d:%s:%lld:Type:%d",
            int(m_pReg), names[m_state], m_epoch, m_type);
    return Trace::prettyNode(buf, m_cont);
  }
  RegInfo() : m_cont(), m_state(FREE) { }
#undef REGSTATES
};

/**
 * SpillFill -- machine-specific details about how to spill and fill
 * registers in given virtual location. Should probably be a template
 * parameter, but I'd like to not waste my short remaining years of health
 * recompiling.
 */
class SpillFill {
 public:
  virtual ~SpillFill() { }
  virtual void spill(const Location& loc, DataType t, PhysReg reg,
                     bool writeType) = 0;
  virtual void fill(const Location& loc, PhysReg reg) = 0;
  virtual void fillByMov(PhysReg src, PhysReg dst) = 0;
  virtual void loadImm(int64 immVal, PhysReg reg) = 0;
  virtual void poison(PhysReg reg) = 0;
};

class LazyScratchReg;

class RegAlloc {
  friend class LazyScratchReg;
  // RegInfo: indexed by PhysReg.
  RegInfo         m_info[kMaxRegs];

  // Secondary indices on m_info.
  RegSet          m_callerSaved;      // Good short-lived regs
  RegSet          m_calleeSaved;      // Good long-lived regs
  int             m_numRegs;          // Number of real registers, <= kMaxRegs
  RegSet          m_allRegs;
  PhysReg         m_lru[kMaxRegs];    // lru order over registers
  typedef hphp_hash_map<RegContent, PhysReg, RegContent> ContToRegMap;
  ContToRegMap    m_contToRegMap;     // Content -> PhysReg
  SpillFill*      m_spf;
  mutable int     m_freezeCount;      // support immutability
  uint64          m_epoch;
  bool            m_branchSynced;

  RegInfo* alloc(const Location& loc, DataType t, RegInfo::State state,
                 bool needsFill, int64 immVal = 0, PhysReg target = InvalidReg);
  RegInfo* findFreeReg(const Location& loc);
  void assignRegInfo(RegInfo *regInfo, const RegContent &cont,
                     RegInfo::State state, DataType type);
  void stateTransition(RegInfo* r, RegInfo::State to);

  static bool isValidReg(PhysReg pr) {
    return int(pr) >= 0 && int(pr) < kMaxRegs;
  }

  // lru operations are O(numRegs), but numRegs is small.
  void lruFront(RegInfo *r);
  void lruBack(RegInfo* r);
  RegInfo *physRegToInfo(PhysReg pr) const;
  void freeRegInfo(RegInfo* ri);

  void spill(RegInfo *toSpill);
  void trace();
  void verify();
  void smashRegImpl(RegInfo *r);
  void reconcileOne(RegInfo* r, RegAlloc* branchRA, PhysReg branchPR);
  bool checkNoScratch();
  PhysReg allocScratchReg(PhysReg pr = InvalidReg);
  void freeScratchReg(PhysReg r);

 public:
  RegAlloc(RegSet callerSaved, RegSet calleeSaved, SpillFill* spf);

  RegAlloc(const RegAlloc& rhs) {
    *this = rhs; // operator= invocation
  }

  // allocReg: allocate a single operand
  PhysReg allocReg(const Location& loc, DataType t, RegInfo::State state) {
    RegInfo* ri = alloc(loc, t, state, state == RegInfo::CLEAN);
    return ri->m_pReg;
  }

  void setBranchSynced() {
    ASSERT(!m_branchSynced);
    m_branchSynced = true;
  }
  bool branchSynced() {
    return m_branchSynced;
  }

  void assertNoScratch() { ASSERT(checkNoScratch()); }

  const RegInfo* getInfo(PhysReg pr) const {
    return physRegToInfo(pr);
  }
  bool regIsDirty(PhysReg pr) const {
    return getInfo(pr)->m_state == RegInfo::DIRTY;
  }
  bool regIsClean(PhysReg pr) const {
    return getInfo(pr)->m_state == RegInfo::CLEAN;
  }
  bool regIsFree(PhysReg pr) const {
    return getInfo(pr)->m_state == RegInfo::FREE;
  }
  void assertRegIsFree(PhysReg pr) const {
    if (debug && !regIsFree(pr)) {
      std::cerr << getInfo(pr)->pretty() << std::endl;
      always_assert(false && "Expected register to be free");
    }
  }
  DataType regType(PhysReg pr) const {
    return getInfo(pr)->m_type;
  }
  void setRegType(PhysReg pr, DataType type) const {
    physRegToInfo(pr)->m_type = type;
  }
  Location regLoc(PhysReg pr) const {
    const RegInfo* info = getInfo(pr);
    return info->m_cont.isLoc() ? info->m_cont.m_loc : Location();
  }

  // allocInputRegs: given an instruction, find/fill its inputs.
  void allocInputReg(const DynLocation& dl, PhysReg target = InvalidReg);
  void allocInputReg(const NormalizedInstruction& ni, int index,
                     PhysReg target = InvalidReg);
  void allocInputRegs(const NormalizedInstruction& ni);
  // allocOutputRegs: destructively mark output registers. Should only
  // be done when we know that the code we're emitting will drive valid
  // values into these outputs.
  void allocOutputRegs(const NormalizedInstruction& ni);

  void bind(PhysReg reg, const Location& loc, DataType t,
            RegInfo::State state);
  void bindScratch(LazyScratchReg& reg, const Location& loc, DataType t,
                   RegInfo::State state);
  void markAsClean(const Location& loc);

  /*
   * Invalidating a location means to drop any register mapped to that
   * location down to FREE state, regardless of the current state.
   *
   * (This differs from smashing a location in that it doesn't require
   * that the register is not DIRTY.)
   */
  void invalidate(const Location& loc);
  void invalidateLocals(int first, int last);

  bool hasReg(const Location &loc) const;
  RegSet getRegsLike(RegInfo::State state) const;
  bool hasDirtyRegs(int firstUnreachableStk) const;
  PhysReg getReg(const Location &loc);

  /*
   * Returns a PhysReg containing the given immediate value (immVal),
   * or InvalidReg if this could not be accomplished.
   *
   * If a physical register already contains `immVal', this function
   * returns it.  Otherwise, if `allowAllocate' is true and this
   * register allocator is not frozen, a free register, if available,
   * will be allocated and set to this value.
   *
   * Otherwise InvalidReg is returned.
   */
  PhysReg getImmReg(int64 immVal, bool allowAllocate = true);

  /*
   * Reset the register mapping to an empty state, epoch zero.
   *
   * Post: pristine() == true
   */
  void reset();

  /*
   * Indicates whether the register map is in its initial, empty
   * state.  That is, empty() == true and bumpEpoch has not been
   * called since the last time reset() was called.
   */
  bool pristine() const;

  /*
   * Returns true if this RegMap has no non-FREE registers in it.
   */
  bool empty() const;

  /*
   * Clean any dirty registers from various sets.
   *
   * For these functions, only registers in the DIRTY state are
   * cleaned and transitioned to the CLEAN state.  Scratch registers
   * remain in scratch state.
   */
  void cleanAll();
  void cleanRegs(RegSet regsToPurge);
  void cleanLoc(const Location& loc);
  void cleanLocals();
  void cleanReg(PhysReg reg);

  /*
   * Forget the mapping for all registers in the set.  The regs must
   * not be DIRTY (if you want to forget a dirty register without
   * spilling, scrub it first).
   */
  void smashRegs(RegSet smashedRegs);
  void smashReg(PhysReg pr);
  void smashLoc(const Location& loc);

  /*
   * Forget a mapping for a register, after cleaning it if it is DIRTY.
   *
   * This is equivalent to calling clean followed by smash.
   */
  void cleanSmashRegs(RegSet set);
  void cleanSmashReg(PhysReg pr);
  void cleanSmashLoc(const Location& loc);

  /*
   * Scrubbing a register means to change it to the CLEAN state,
   * regardless of whether we've actually spilled its contents to
   * memory.  These functions may not be called for a scratch
   * register---it must be a program location---but it is legal to
   * scrub an already-free register.
   *
   * This is often going to be followed by smashing the register.
   * Special functions help for the case of dealing with discarding
   * dead execution stack locations, since that's usually what this is
   * about.
   */
  void scrubStackEntries(int firstUnreachable);
  void scrubStackRange(int firstToDiscard, int lastToDiscard);
  void scrubReg(PhysReg pr);
  void scrubRegs(RegSet regs);
  void scrubLoc(const Location&);

  void killImms(RegSet imms);
  void swapRegisters(PhysReg r1, PhysReg r2);

  /*
   * Emit spills and fills in order to bring the `branch' state into
   * the state of *this.
   *
   * This reconcile method is not symmetric: it will only emit
   * spills/fills to the branch.  In particular, this means that
   * whatever is happening on the other side of branch.m_spf-> needs
   * to be aware of the branch register state.  Specifically: this
   * means tx64->m_regMap must be the *branch* register map during
   * reconciliation, because spill() may try to use/load immediate
   * registers.
   */
  void reconcile(RegAlloc& branch);

  void freeze() const  { m_freezeCount++; }
  void defrost() const { m_freezeCount--; ASSERT(m_freezeCount >= 0); }
  bool frozen() const  { return m_freezeCount > 0; }

  void bumpEpoch() { m_epoch++; }

  std::string pretty() const;
};

// RAII ScratchReg holder:
//   LazyScratchReg r(m_regMap);
//   ...
//   r.alloc();
//   ... neg_reg64(*r);
//
//   ScratchReg r(m_regMap);
//   .. neg_reg64(*r);
class LazyScratchReg : boost::noncopyable {
 protected:
  RegAlloc& m_regMap;
  PhysReg m_reg;
 public:
  LazyScratchReg(RegAlloc& regMap);
  ~LazyScratchReg();

  bool isAllocated() const { return m_reg != reg::noreg; }

  void alloc(PhysReg pr = InvalidReg);
  void dealloc();
  void realloc(PhysReg pr = InvalidReg);

  friend PhysReg r(const LazyScratchReg& l) { return l.m_reg; }
  friend Reg8 rbyte(const LazyScratchReg& l) { return rbyte(l.m_reg); }
  friend Reg32 r32(const LazyScratchReg& l) { return r32(l.m_reg); }
  friend Reg64 r64(const LazyScratchReg& l) { return r64(l.m_reg); }
};

class ScratchReg : public LazyScratchReg {
 public:
  ScratchReg(RegAlloc& regMap);
  // Use this constructor to reserve an already-selected register, which
  // must be free.
  ScratchReg(RegAlloc& regMap, PhysReg pr);
};

/*
 * DumbScratch allocates a register out of a RegSet, putting it back
 * when it's done.  This is used for very simple register selection
 * when we don't want to use the whole register allocator
 * (e.g. between tracelets).
 *
 * Since this thing is dumb, there's no recourse if the set has no
 * registers available.  This thing will assert, then throw, in that
 * case.
 */
struct DumbScratchReg : private boost::noncopyable {
  explicit DumbScratchReg(RegSet& allocSet);
  ~DumbScratchReg();

  friend PhysReg r(const DumbScratchReg& d) { return d.m_reg; }
  friend Reg32 r32(const DumbScratchReg& d) { return r32(d.m_reg); }
  friend Reg64 r64(const DumbScratchReg& d) { return r64(d.m_reg); }

private:
  RegSet& m_regPool;
  const PhysReg m_reg;
};

} } } // HPHP::VM::Transl

#endif /* incl_REG_ALLOC_H_ */
