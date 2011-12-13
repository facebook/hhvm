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

#include "util/trace.h"
#include "translator.h"
#include "asm-x64.h"

namespace HPHP { namespace VM { namespace Transl {

using HPHP::x64::register_name_t;

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
typedef register_name_t PhysReg;

struct RegContent {
  enum Kind {
    Invalid,
    Loc,
    Int
  }        m_kind;
  int64    m_int;
  Location m_loc;

  RegContent(const Location &_m_loc) : m_kind(Loc), m_int(0), m_loc(_m_loc) { }

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
    return HPHP::hash_int64_pair(cont.m_kind,
                                 HPHP::hash_int64_pair(cont.m_int,
                                                       cont.m_loc(cont.m_loc)));
  }

};

/*
 * In the virtual memory analogy, a RegInfo is the PTE: it contains
 * the v2p mapping, permissions, and LRU linkage.
 */
struct RegInfo {
#define REGSTATES \
  REGSTATE(FREE) REGSTATE(CLEAN) REGSTATE(SCRATCH) REGSTATE(DIRTY)
  enum State {
#define REGSTATE(x) x,
    // Order is meaningful here; we use <, > to test "strength" of state
    REGSTATES
#undef REGSTATE
  };
  RegContent m_cont;
  PhysReg    m_pReg;
  State      m_state;
  DataType   m_type;
  RegInfo   *m_lruPrev;
  RegInfo   *m_lruNext;

  void insertAfter(RegInfo* pred) {
    if (m_lruPrev) m_lruPrev->m_lruNext = m_lruNext;
    if (m_lruNext) m_lruNext->m_lruPrev = m_lruPrev;
    RegInfo* oldNext = pred->m_lruNext;
    // Link to car
    pred->m_lruNext = this;
    m_lruPrev = pred;
    // Link to cdr
    oldNext->m_lruPrev = this;
    m_lruNext = oldNext;
  }

  std::string pretty() const {
    const char* names[] = {
#define REGSTATE(x) #x,
      REGSTATES
#undef REGSTATE
    };
    char buf[1024];
    sprintf(buf, "Reg:%02d:%s:Type:%d", m_pReg, names[m_state], m_type);
    return Trace::prettyNode(buf, m_cont);
  }
  RegInfo()
    : m_cont(),
      m_state(FREE),
      m_lruPrev(NULL),
      m_lruNext(NULL) { }
#undef REGSTATES
};

/**
 * SpillFill -- machine-specific details about how to spill and fill
 * registers in given virtual location. Should probably be a template
 * parameter, but I'd like to not waste my short remaining years of health
 * recompiling.
 */
typedef hphp_hash_map<RegContent, RegInfo*, RegContent> ContToRegMap;
class SpillFill {
 public:
  virtual void spill(const Location& loc, DataType t, PhysReg reg,
                     bool writeType) = 0;
  virtual void fill(const Location& loc, PhysReg reg) = 0;
  virtual void loadImm(int64 immVal, PhysReg reg) = 0;
  virtual void poison(PhysReg reg) = 0;
};

class RegAlloc {
  vector<PhysReg> m_regPri; // prioritized order of registers
  ContToRegMap    m_contToRegMap;
  RegInfo*        m_lruHead;
  RegInfo         m_lruSentinelHead, m_lruSentinelTail;
  RegInfo*        m_lruTail;
  SpillFill*      m_spf;

  RegInfo* alloc(const Location& loc, DataType t, RegInfo::State state,
                 bool needsFill);
  RegInfo* evict();
  RegInfo* findFreeReg();
  void assignRegInfo(RegInfo *regInfo, const RegContent &cont,
                     RegInfo::State state, DataType type);

  void lruFront(RegInfo *r) {
    r->insertAfter(&m_lruSentinelHead);
  }

  void lruBack(RegInfo *r) {
    if (r != m_lruSentinelTail.m_lruPrev) {
      r->insertAfter(m_lruSentinelTail.m_lruPrev);
    }
  }

  RegInfo *physRegToInfo(PhysReg pr) const;
  void freeRegInfo(RegInfo* ri);

  void spill(RegInfo *toSpill);
  void trace();
  void verify();

 public:
  // allocReg: allocate a single operand
  PhysReg allocReg(const Location& loc, DataType t, RegInfo::State state) {
    RegInfo* ri = alloc(loc, t, state, state == RegInfo::CLEAN);
    return ri->m_pReg;
  }

  static const PhysReg InvalidReg = -1;

  // allocInputRegs: given an instruction, find/fill its inputs.
  void allocInputRegs(const NormalizedInstruction& ni);
  // allocOutputRegs: destructively mark output registers. Should only
  // be done when we know that the code we're emitting will drive valid
  // values into these outputs.
  void allocOutputRegs(const NormalizedInstruction& ni);

  PhysReg allocScratchReg();
  void freeScratchReg(PhysReg r);
  void bind(PhysReg reg, const Location& loc, DataType t, RegInfo::State state);
  void markAsClean(const Location& loc);
  void invalidate(const Location& loc);
  bool hasReg(const Location &loc) const;
  PhysReg getReg(const Location &loc) const;
  PhysReg getImmReg(int64 immVal);
  void cleanAll();
  void cleanRegs(const std::set<PhysReg>& regsToPurge);
  void cleanLocals();
  void smashPhysRegs(const std::set<PhysReg>& smashedRegs);
  void reset();
  RegAlloc(const vector<PhysReg>& allocableRegs, SpillFill* spf);
  void scrubStackEntries(int firstUnreachable);
  void scrubStackRange(int firstToDiscard, int lastToDiscard);
  void swapRegisters(PhysReg r1, PhysReg r2);
};

// RAII ScratchReg holder:
//   ScratchReg r(m_regMap);
//   ... neg_reg64(*r);
class ScratchReg {
  RegAlloc& m_regMap;
  PhysReg m_reg;
 public:
  ScratchReg(RegAlloc& regMap);
  ~ScratchReg();
  PhysReg operator*() const;
};

} } } // HPHP::VM::Transl

#endif /* incl_REG_ALLOC_H_ */
