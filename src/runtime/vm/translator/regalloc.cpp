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

#include <set>

#include "runtime/base/types.h"
#include "util/trace.h"
#include "regalloc.h"

using std::set;

namespace HPHP { namespace VM { namespace Transl {

static const Trace::Module TRACEMOD = Trace::regalloc;

#define FOR_EACH_REG(r) \
  for (RegInfo* r = m_lruSentinelHead.m_lruNext; \
       r != &m_lruSentinelTail; r = r->m_lruNext)

RegAlloc::RegAlloc(const vector<register_name_t>& allocableRegs,
                   SpillFill* spf)
  : m_regPri(allocableRegs), m_spf(spf)
{
  reset();
}

void
RegAlloc::freeRegInfo(RegInfo *r) {
  r->m_state = RegInfo::FREE;
  // For ease of debugging, invalidate content
  r->m_cont = RegContent();
  lruBack(r);
}

/**
 * alloc --
 *
 *   Allocate a single register.
 */
RegInfo*
RegAlloc::alloc(const Location& loc, DataType type, RegInfo::State state,
                bool needsFill) {
  RegInfo   *retval = NULL;
  RegContent cont   = RegContent(loc);

  if (loc.isValid()) {
    // Best possible result: it's already there.
    if (mapGet(m_contToRegMap, cont, &retval)) {
      ASSERT(retval);
      ASSERT(state == RegInfo::DIRTY || state == RegInfo::CLEAN);
      ASSERT(retval->m_state == RegInfo::CLEAN ||
             retval->m_state == RegInfo::DIRTY);
      ASSERT(retval->m_cont == cont);
      TRACE(1, "alloc (%s, %d) t%d state %d hit r%d\n",
            loc.spaceName(), loc.offset, type, state, retval->m_pReg);
      needsFill = false;
    }
  }
  if (!retval) {
    // Oops, not there yet. First look for a free one.
    retval = findFreeReg();
    if (retval) {
      TRACE(1, "alloc (%s, %d) found a free reg %d state %d\n",
            loc.spaceName(), loc.offset, retval->m_pReg, retval->m_state);
    }
  }

  if (!retval) {
    // Least-recently-used reg; don't care if they're clean or dirty.  The
    // allocator depends on strict adherence to LRU for correctness when
    // allocating non-free regs. XXX: This is lame, mixing policy and
    // mechanism. The client should be able to "pin" values while they're
    // immobile, allowing the register allocator to use whatever policy
    // it wants.

    retval = m_lruSentinelTail.m_lruPrev;
    ASSERT(retval->m_state == RegInfo::CLEAN ||
           retval->m_state == RegInfo::DIRTY);
    TRACE(1, "alloc (%s, %d) found a %s victim reg r%d\n",
          loc.spaceName(), loc.offset,
          retval->m_state == RegInfo::CLEAN ? "clean" : "dirty",
          retval->m_pReg);

    if (retval->m_state == RegInfo::DIRTY) {
      spill(retval);
    }

    // Retval is a victim. Remove it from the index of valid locations.
    ContToRegMap::iterator evictI = m_contToRegMap.find(retval->m_cont);
    ASSERT(evictI != m_contToRegMap.end());
    m_spf->poison(evictI->second->m_pReg);
    m_spf->poison(retval->m_pReg);
    m_contToRegMap.erase(evictI);
    retval->m_state = RegInfo::FREE;
  }

  ASSERT(retval);
  TRACE(1, "alloc (%s, %d) t%d state %d r%d fill? %d\n",
        loc.spaceName(), loc.offset, type, state, retval->m_pReg, needsFill);
  if (needsFill) {
    m_spf->fill(loc, retval->m_pReg);
  }

  // Can't happen: if we're evicting a dirty register, we should have set
  // the old m_state to free after cleaning.
  ASSERT(!(retval->m_state == RegInfo::DIRTY && state == RegInfo::SCRATCH));

  RegInfo::State new_state = retval->m_state;
  if (state != retval->m_state && retval->m_state != RegInfo::DIRTY) {
    new_state = state;
  }
  assignRegInfo(retval, cont, new_state, type);
  verify();
  return retval;
}

void
RegAlloc::allocInputRegs(const NormalizedInstruction& ni) {
  for (unsigned i = 0; i < ni.inputs.size(); i++) {
    const RuntimeType& rtt = ni.inputs[i]->rtt;
    if (rtt.isIter()) {
      continue;
    }
    DataType t = rtt.outerType();
    ASSERT(!rtt.isHome());
    (void) alloc(ni.inputs[i]->location, t, RegInfo::CLEAN, true);
  }
}

void
RegAlloc::allocOutputRegs(const NormalizedInstruction& ni) {
  DynLocation* outputs[] = { ni.outStack, ni.outLocal };
  for (ssize_t i = 0; i < 2; ++i) {
    if (outputs[i]) {
      const DynLocation* out = outputs[i];
      DataType t = out->rtt.outerType();
      if (out->rtt.isHome()) {
        t = KindOfHome;
      }
      (void) alloc(outputs[i]->location, t, RegInfo::DIRTY, false);
    }
  }
  verify();
}

bool
RegAlloc::hasReg(const Location& loc) const {
  RegContent cont = RegContent(loc);
  return mapContains(m_contToRegMap, cont);
}

PhysReg
RegAlloc::getReg(const Location& loc) const {
  RegContent cont    = RegContent(loc);
  RegInfo   *regInfo = mapGet(m_contToRegMap, cont, NULL);
  ASSERT(regInfo); // Usage error; didn't call allocRegs()?
  return regInfo->m_pReg;
}

void
RegAlloc::markAsClean(const Location& loc) {
  RegContent cont    = RegContent(loc);
  RegInfo   *regInfo = mapGet(m_contToRegMap, cont, NULL);
  if (regInfo) {
    regInfo->m_state = RegInfo::CLEAN;
  }
}

void
RegAlloc::invalidate(const Location& loc) {
  RegContent cont = RegContent(loc);
  ContToRegMap::iterator i = m_contToRegMap.find(cont);
  if (i != m_contToRegMap.end()) {
    RegInfo *r = i->second;
    freeRegInfo(r);
    m_contToRegMap.erase(i);
  }
}

void
RegAlloc::reset() {
  TRACE(1, ">>> regalloc reset! <<<\n");
  m_contToRegMap.clear();
  // Initialize queue sentinels
  m_lruSentinelHead.m_lruPrev = NULL;
  m_lruSentinelHead.m_lruNext = &m_lruSentinelTail;
  m_lruSentinelHead.m_pReg  = InvalidReg;
  m_lruSentinelTail.m_lruPrev = &m_lruSentinelHead;
  m_lruSentinelTail.m_lruNext = NULL;
  m_lruSentinelTail.m_pReg  = -1;

  // Touch them in reverse order
  for (int i = m_regPri.size() - 1; i >= 0; i--) {
    RegInfo *ri = new RegInfo();
    ri->m_pReg  = m_regPri[i];
    ASSERT(ri->m_pReg != 3 && ri->m_pReg != 5);
    ri->m_state = RegInfo::FREE;
    lruFront(ri);
  }
  verify();
}

void
RegAlloc::cleanRegs(const std::set<PhysReg>& regs) {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY &&
        regs.find(r->m_pReg) != regs.end()) {
      spill(r);
      r->m_state = RegInfo::CLEAN;
    }
  }
  verify();
}

void RegAlloc::cleanLocals() {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY &&
        r->m_cont.isLoc() &&
        r->m_cont.m_loc.isLocal()) {
      spill(r);
      r->m_state = RegInfo::CLEAN;
    }
  }
  verify();
}

void RegAlloc::cleanAll() {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY) {
      spill(r);
      r->m_state = RegInfo::CLEAN;
    }
  }
  verify();
}

void RegAlloc::spill(RegInfo *toSpill) {
  ASSERT(toSpill->m_cont.isLoc());
  if (toSpill->m_type == KindOfInvalid) {
    // KindOfInvalid outputs are auto-spilled; it is the translator's
    // responsibility to keep them sync'ed in memory and registers.
    TRACE(1, "spill: (%s, %d) skipping invalid output\n",
          toSpill->m_cont.m_loc.spaceName(), toSpill->m_cont.m_loc.offset);
    return;
  }
  TRACE(1, "spill: (%s, %d) <- type %d, r%d\n",
        toSpill->m_cont.m_loc.spaceName(), toSpill->m_cont.m_loc.offset,
        toSpill->m_type, toSpill->m_pReg);
  m_spf->spill(toSpill->m_cont.m_loc, toSpill->m_type, toSpill->m_pReg, true);
  verify();
}

void RegAlloc::smashPhysRegs(const set<PhysReg>& smashedRegs) {
  FOR_EACH_REG(r) {
    if (smashedRegs.find(r->m_pReg) != smashedRegs.end()) {
      // Callers responsiblity to scrub this before it was smashed!
      ASSERT(r->m_state != RegInfo::DIRTY);
      if (r->m_state == RegInfo::CLEAN) {
        ContToRegMap::iterator rmi = m_contToRegMap.find(r->m_cont);
        ASSERT(rmi != m_contToRegMap.end());
        m_contToRegMap.erase(rmi);
        r->m_state = RegInfo::FREE;
        r->m_cont = RegContent();
      }
    }
  }
  verify();
}

RegInfo*
RegAlloc::physRegToInfo(PhysReg reg) const {
  FOR_EACH_REG(r) {
    if (r->m_pReg == reg) {
      return r;
    }
  }
  not_reached();
  return NULL;
}

/*
 * Scratch regs are not free, and but have no Location, cannot be spilled
 * or filled, and do not appear in m_regMap.
 */
PhysReg
RegAlloc::allocScratchReg() {
  return alloc(Location(), KindOfInvalid, RegInfo::SCRATCH, false)->m_pReg;
}

void
RegAlloc::freeScratchReg(PhysReg r) {
  RegInfo *ri = physRegToInfo(r);
  if (ri->m_state == RegInfo::SCRATCH) {
    // Funny story: It's possible for a scratch register to get forcibly bound
    // while the ScratchReg enclosing it is still live.
    freeRegInfo(ri);
  }
}

RegInfo *
RegAlloc::findFreeReg() {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::FREE) {
      m_spf->poison(r->m_pReg);
      return r;
    }
  }
  return NULL;
}

void
RegAlloc::assignRegInfo(RegInfo *regInfo, const RegContent &cont,
                        RegInfo::State state, DataType type) {
  ASSERT(regInfo);
  ASSERT(cont.isValid());
  ASSERT(!cont.isInt() || state == RegInfo::CLEAN);
  ASSERT(!cont.isInt() || type == KindOfInt64);

  regInfo->m_cont  = cont;
  regInfo->m_state = state;
  regInfo->m_type  = type;

  if (state == RegInfo::CLEAN || state == RegInfo::DIRTY) {
    ASSERT(cont.isValid());
    ASSERT(!cont.isLoc() || cont.m_loc.isValid());

    m_contToRegMap.insert(std::pair<RegContent, RegInfo*>(cont, regInfo));

    ASSERT(m_contToRegMap[cont] == regInfo);
  }
  if (state != RegInfo::FREE) {
    lruFront(regInfo);
  }
}


/*
 * Returns a PhysReg containing the given immediate value (immVal).
 * If no physical register currently contains immVal, a free register,
 * if available, will be allocated and set to this value.
 * If no register contains immVal and no register is free,
 * InvalidReg is returned.
 */
PhysReg
RegAlloc::getImmReg(int64 immVal) {
  DataType       type    = KindOfInt64;
  RegInfo::State state   = RegInfo::CLEAN;
  RegInfo       *freeReg = NULL;
  RegContent     cont    = RegContent(immVal);

  // Check if val is already in some reg, and return it if so.
  RegInfo *regInfo = mapGet(m_contToRegMap, cont, NULL);
  if (regInfo) {
    ASSERT(regInfo->m_cont == cont);
    ASSERT(regInfo->m_state == RegInfo::CLEAN);
    lruFront(regInfo);
    return regInfo->m_pReg;
  }

  // Look for a free reg; give up if none.
  freeReg = findFreeReg();
  if (!freeReg) {
    return InvalidReg;
  }

  // Allocate freeReg, load it with immVal, and return it.
  TRACE(1, "allocImmReg (0x%llx) t%d state %d r%d\n",
        immVal, type, state, freeReg->m_pReg);
  m_spf->loadImm(immVal, freeReg->m_pReg);
  assignRegInfo(freeReg, cont, state, type);
  verify();
  return freeReg->m_pReg;
}

void
RegAlloc::bind(PhysReg reg, const Location& loc, DataType t,
               RegInfo::State state) {
  ASSERT(state != RegInfo::FREE);

  invalidate(loc);
  RegInfo *r = physRegToInfo(reg);
  ASSERT(r->m_state != RegInfo::DIRTY); // Too late to write this back
  if (state != RegInfo::SCRATCH) {
    ContToRegMap::iterator i = m_contToRegMap.find(r->m_cont);
    if (i != m_contToRegMap.end()) {
      m_contToRegMap.erase(i);
    }
    RegContent cont = RegContent(loc);
    r->m_cont = cont;
    m_contToRegMap.insert(std::pair<RegContent, RegInfo*>(cont, r));
  }
  r->m_type  = t;
  r->m_state = state;
  lruFront(r);
  verify();
}

void
RegAlloc::scrubStackEntries(int firstUnreachable) {
  FOR_EACH_REG(r) {
    if (r->m_cont.isLoc() &&
        r->m_cont.m_loc.space == Location::Stack &&
        r->m_cont.m_loc.offset >= firstUnreachable) {
      TRACE(1, "scrubbing dead stack value: (Stack, %d)\n",
            r->m_cont.m_loc.offset);
      ASSERT(r->m_state == RegInfo::CLEAN || r->m_state == RegInfo::DIRTY);
      r->m_state = RegInfo::CLEAN;
    }
  }
  verify();
}

void
RegAlloc::scrubStackRange(int firstToDiscard, int lastToDiscard) {
  FOR_EACH_REG(r) {
    if (r->m_cont.isLoc() &&
        r->m_cont.m_loc.space == Location::Stack &&
        r->m_cont.m_loc.offset >= firstToDiscard &&
        r->m_cont.m_loc.offset <= lastToDiscard) {
      TRACE(1, "scrubbing dead stack value: (Stack, %d)\n",
            r->m_cont.m_loc.offset);
      ASSERT(r->m_state == RegInfo::CLEAN || r->m_state == RegInfo::DIRTY);
      r->m_state = RegInfo::CLEAN;
    }
  }
  verify();
}

void
RegAlloc::swapRegisters(PhysReg r1, PhysReg r2) {
  RegInfo *ri1 = physRegToInfo(r1),
          *ri2 = physRegToInfo(r2);
  std::swap(ri1->m_pReg, ri2->m_pReg);
  // TODO: lru?
  verify();
}

void
RegAlloc::verify() {
#ifdef DEBUG
  std::set<PhysReg> allRegs;
  for (unsigned i = 0; i < m_regPri.size(); i++) {
    allRegs.insert(m_regPri[i]);
  }

  // LRU invariants: forward.
  std::set<PhysReg> lruRegs;
  FOR_EACH_REG(r) {
    // The state is reasonable
    ASSERT(r->m_state == RegInfo::FREE ||
           r->m_state == RegInfo::CLEAN ||
           r->m_state == RegInfo::SCRATCH ||
           r->m_state == RegInfo::DIRTY);
    // Each reg appears only once.
    ASSERT(lruRegs.find(r->m_pReg) == lruRegs.end());
    lruRegs.insert(r->m_pReg);
  }
  // All regs are there.
  ASSERT(lruRegs == allRegs);
  lruRegs.clear();

  FOR_EACH_REG(r) {
    // Each reg appears only once.
    ASSERT(lruRegs.find(r->m_pReg) == lruRegs.end());
    lruRegs.insert(r->m_pReg);
  }
  // All regs are there.
  ASSERT(lruRegs == allRegs);

  // The map from content to registers.
  for (ContToRegMap::const_iterator lri = m_contToRegMap.begin();
       lri != m_contToRegMap.end(); ++lri) {
    const RegContent& cont = lri->first;
    const RegInfo* ri = lri->second;
    // The location and mapping are consistent.
    ASSERT(ri->m_cont == cont);
    // If it's a location, make sure it's is valid.
    ASSERT(!(ri->m_cont.isLoc()) || ri->m_cont.m_loc.isValid());
    // If it's an integer/immediate, make sure it's clean.
    ASSERT(!(ri->m_cont.isInt()) || ri->m_state == RegInfo::CLEAN);
    // The register is live.
    ASSERT(ri->m_state != RegInfo::FREE);
  }

  FOR_EACH_REG(r) {
    if (r->m_state != RegInfo::FREE &&
        r->m_state != RegInfo::SCRATCH) {
      ASSERT(mapContains(m_contToRegMap, r->m_cont));
      ASSERT(m_contToRegMap[r->m_cont] == r);
      if (r->m_cont.isInt()) {
        ASSERT(r->m_state == RegInfo::CLEAN);
      }
    }
  }
#endif
}

void
RegAlloc::trace() {
  TRACE(10, "----\n");
  FOR_EACH_REG(r) {
    TRACE(10, *r);
  }
}


ScratchReg::ScratchReg(RegAlloc& regMap) :
  m_regMap(regMap),
  m_reg(m_regMap.allocScratchReg()) {
  TRACE(1, "ScratchReg: alloc %d\n", m_reg);
}

ScratchReg::~ScratchReg() {
  TRACE(1, "ScratchReg: free %d\n", m_reg);
  m_regMap.freeScratchReg(m_reg);
}

PhysReg ScratchReg::operator*() const {
  return m_reg;
}

} } } // HPHP::VM::Transl
