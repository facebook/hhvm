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

#include <boost/format.hpp>
#include "runtime/base/types.h"
#include "util/trace.h"
#include "regalloc.h"

namespace HPHP { namespace VM { namespace Transl {

using std::set;
using namespace reg;

static const Trace::Module TRACEMOD = Trace::regalloc;

#define FOR_EACH_REG(r) \
  for (RegInfo* r = const_cast<RegInfo*>(&m_info[0]); \
       r < &m_info[kMaxRegs]; r++) if (r->m_state != RegInfo::INVALID)

#define FOR_EACH_LRU_REG(r) \
    for (int _i = 0, RegInfo* r = &m_info[m_lru[_i]]; \
         _i < m_numRegs; ++_i, r = &m_info[m_lru[_i]])

#define FOR_EACH_REG_IN_SET(r, s) \
  FOR_EACH_REG(r) if (s.contains(r->m_pReg))

RegAlloc::RegAlloc(RegSet callerSaved,
                   RegSet calleeSaved,
                   SpillFill* spf)
  : m_callerSaved(callerSaved),
    m_calleeSaved(calleeSaved),
    m_numRegs(callerSaved.size() + calleeSaved.size()),
    m_allRegs(callerSaved | calleeSaved),
    m_spf(spf),
    m_freezeCount(0),
    m_branchSynced(false)
{
  ASSERT(m_calleeSaved - m_callerSaved == m_calleeSaved);
  reset();
}

void
RegAlloc::freeRegInfo(RegInfo *r) {
  ContToRegMap::iterator it = r->m_state != RegInfo::SCRATCH
    ? m_contToRegMap.find(r->m_cont) : m_contToRegMap.end();
  stateTransition(r, RegInfo::FREE);
  // For ease of debugging, invalidate content
  r->m_cont = RegContent();
  if (it != m_contToRegMap.end()) {
    m_contToRegMap.erase(it);
  }
  lruBack(r);
}

/**
 * alloc --
 *
 *   Allocate a single register.
 */
RegInfo*
RegAlloc::alloc(const Location& loc, DataType type, RegInfo::State state,
                bool needsFill, int64 immVal, PhysReg target) {
  RegInfo   *retval = NULL;
  RegContent cont   = RegContent(loc, immVal);

  if (loc.isValid()) {
    // Best possible result: it's already there.
    PhysReg pr;
    if (mapGet(m_contToRegMap, cont, &pr)) {
      retval = physRegToInfo(pr);
      ASSERT(state == RegInfo::DIRTY || state == RegInfo::CLEAN);
      ASSERT(retval->m_state == RegInfo::CLEAN ||
             retval->m_state == RegInfo::DIRTY);
      ASSERT(retval->m_cont == cont);
      TRACE(1, "alloc (%s, %lld) t%d state %d hit r%d\n",
            loc.spaceName(), loc.offset, type, state, retval->m_pReg);
      needsFill = false;
    }
  }
  if (!retval) {
    // Oops, not there yet. First look for a free one.
    if (target != InvalidReg) {
      ASSERT(regIsFree(target));
      retval = physRegToInfo(target);
      m_spf->poison(retval->m_pReg);
    } else {
      retval = findFreeReg(loc);
    }
    if (retval) {
      TRACE(1, "alloc (%s, %lld) found a free reg %d state %d\n",
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
    PhysReg lruPr = m_lru[m_numRegs - 1];
    retval = physRegToInfo(lruPr);

    // This epoch mechanism ensures that we aren't forcefully killing a
    // register that still might need preservation.
    ASSERT(retval->m_epoch < m_epoch);

    TRACE(1, "alloc (%s, %lld) found a %s victim reg r%d\n",
          loc.spaceName(), loc.offset,
          retval->m_state == RegInfo::CLEAN ? "clean" : "dirty",
          retval->m_pReg);

    if (retval->m_state == RegInfo::DIRTY) {
      spill(retval);
    }

    // Retval is a victim. Remove it from the index of valid locations.
    ContToRegMap::iterator evictI = m_contToRegMap.find(retval->m_cont);
    ASSERT(evictI != m_contToRegMap.end());
    m_spf->poison(evictI->second);
    m_spf->poison(retval->m_pReg);
    m_contToRegMap.erase(evictI);
    stateTransition(retval, RegInfo::FREE);
  }

  ASSERT(retval);
  retval->m_epoch = m_epoch;
  TRACE(1, "alloc (%s, %lld) t%d state %d r%d fill? %d\n",
        loc.spaceName(), loc.offset, type, state, retval->m_pReg, needsFill);
  if (needsFill && !IS_NULL_TYPE(type)) {
    if (loc.isLiteral()) {
      m_spf->loadImm(immVal, retval->m_pReg);
    } else {
      m_spf->fill(loc, retval->m_pReg);
    }
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
RegAlloc::allocInputReg(const DynLocation& dl, PhysReg target) {
  const RuntimeType& rtt = dl.rtt;
  if (rtt.isIter()) {
    // Note: if this changes to enregister iterators unwinding will
    // have to be updated.
    return;
  }

  DataType t = rtt.outerType();
  if (t == KindOfInvalid) return;

  const Location& loc = dl.location;

  int64 litVal = 0;
  if (loc.isLiteral()) {
    litVal = rtt.valueGeneric();
  }

  (void) alloc(loc, t, RegInfo::CLEAN, true, litVal, target);
}

void
RegAlloc::allocInputReg(const NormalizedInstruction& ni, int index,
                        PhysReg target /* = InvalidReg */) {
  allocInputReg(*ni.inputs[index], target);
}

void
RegAlloc::allocInputRegs(const NormalizedInstruction& ni) {
  for (unsigned i = 0; i < ni.inputs.size(); i++) {
    allocInputReg(ni, i);
  }
}

void
RegAlloc::allocOutputRegs(const NormalizedInstruction& ni) {
  DynLocation* outputs[] = { ni.outStack, ni.outLocal, ni.outStack2,
                             ni.outStack3 };
  for (size_t i = 0; i < sizeof outputs / sizeof *outputs; ++i) {
    if (outputs[i]) {
      const DynLocation* out = outputs[i];
      DataType t = out->rtt.outerType();
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

RegSet
RegAlloc::getRegsLike(RegInfo::State state) const {
  RegSet retval;
  FOR_EACH_REG(r) {
    if (r->m_state == state) {
      retval |= RegSet(r->m_pReg);
    }
  }
  return retval;
}

bool
RegAlloc::hasDirtyRegs(int firstUnreachableStk) const {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY &&
        !r->m_cont.isUnreachableStack(firstUnreachableStk)) {
      return true;
    }
  }
  return false;
}

PhysReg
RegAlloc::getReg(const Location& loc) {
  PhysReg reg = mapGet(m_contToRegMap, RegContent(loc), InvalidReg);
  lruFront(physRegToInfo(reg));
  ASSERT(reg != InvalidReg); // Usage error; didn't call allocInputRegs()?
  return reg;
}

void
RegAlloc::markAsClean(const Location& loc) {
  PhysReg    pr = mapGet(m_contToRegMap, RegContent(loc), InvalidReg);
  if (pr != InvalidReg) {
    stateTransition(physRegToInfo(pr), RegInfo::CLEAN);
  }
}

void
RegAlloc::invalidate(const Location& loc) {
  ContToRegMap::iterator i = m_contToRegMap.find(RegContent(loc));
  if (i != m_contToRegMap.end()) {
    freeRegInfo(physRegToInfo(i->second));
  }
}

void
RegAlloc::invalidateLocals(int first, int last) {
  for (int i = first; i <= last; ++i) {
    invalidate(Location(Location::Local, i));
  }
}

void
RegAlloc::reset() {
  TRACE(1, ">>> regalloc reset! <<<\n");
  m_epoch = 0;
  m_contToRegMap.clear();
  // m_info is sparse.
  for (int i = 0; i < kMaxRegs; ++i) {
    m_info[i].m_epoch = 0;
    m_info[i].m_pReg = PhysReg(i);
    m_info[i].m_cont = RegContent();
    m_info[i].m_type = KindOfInvalid;
    m_info[i].m_state = RegInfo::INVALID;
  }
  RegSet all = m_allRegs;
  PhysReg pr;
  for (int i = 0; all.findFirst(pr); i++) {
    all.remove(pr);
    physRegToInfo(pr)->m_pReg = PhysReg(pr);
    stateTransition(physRegToInfo(pr), RegInfo::FREE);
    // Put the most favorable register last, so it is picked first.
    m_lru[(m_numRegs - 1) - i] = pr;
  }
  m_branchSynced = false;
  verify();
}

void RegAlloc::reconcile(RegAlloc& branch) {
  RegSet unconsideredRegs;

  TRACE(1, "Beginning reconcile with a branch\n");

  FOR_EACH_REG (r) {
    if (r->m_state == RegInfo::FREE || r->m_state == RegInfo::SCRATCH) {
      unconsideredRegs |= RegSet(r->m_pReg);
      continue;
    }

    const ContToRegMap::iterator it = branch.m_contToRegMap.find(r->m_cont);
    const bool inSameReg = it != branch.m_contToRegMap.end() &&
                           it->second == r->m_pReg;
    if (inSameReg) {
      if (r->m_state == branch.getInfo(r->m_pReg)->m_state) {
        // Done. Same state, same content, same register. We're cool.
        continue;
      }
    }

    /*
     * If we got past the above, we now have one of two situations.
     *
     *  A) The branch's register is mapped to the same location, but
     *     the states are different between the branch and the main
     *     line.  We skipped FREE and SCRATCH above, so:
     *
     *      - If the branch version is DIRTY, that means the mainline
     *        is CLEAN, so we need to spill now or it will never be
     *        spilt.
     *
     *      - If the branch version is CLEAN, that means the main line
     *        thinks the register is DIRTY---nothing wrong will happen
     *        but we could spill unnecessarily if the branch was
     *        taken.
     *
     *  B) The branch's register is mapped to a possibly different
     *     location (or completely unmapped).
     *
     *      - We still need to clean the register if it is DIRTY, to
     *        evict whatever it holds.
     *
     *      - We then need to fill it with what is actually supposed
     *        to be there.  If it is also DIRTY in the main line
     *        that's ok: we'll just spill more than we needed to if we
     *        took the branch.
     */

    // In both situation A and B we need to spill a DIRTY branch reg.
    if (branch.regIsDirty(r->m_pReg)) {
      branch.cleanReg(r->m_pReg);
    }

    // In situation B (i.e. !inSameReg), we also need to fill the reg
    // with the main line's expected content.
    if (!inSameReg) {
      /*
       * The register allocator has an invariant that only one
       * register can be mapped to a given RegContent at a time.  So
       * if we're going to steal this content from another register
       * via fillByMov, we need to unmap the RegContent from it first.
       * If the register was dirty, we have to spill (note that it's
       * important to do this before freeing r->m_pReg so spill()
       * can't allocate it for an immediate register).
       *
       * XXX: the above comment is out of date (spill can't allocate
       * anymore).
       *
       * If the other register was dirty, we know we'll need to clean
       * it, because the location is obviously in a different
       * register.  (The only case we don't have to spill a dirty
       * branch register is when it is the same state and location in
       * the main line.)
       *
       * Note: we could do better in the case where two register just
       * have swapped locations (this can happen via shuffleRegisters
       * when making a call).
       */
      PhysReg oldReg = InvalidReg;
      if (it != branch.m_contToRegMap.end()) {
        oldReg = it->second;
        if (branch.regIsDirty(oldReg)) {
          branch.cleanReg(oldReg);
        }
        branch.freeRegInfo(branch.physRegToInfo(oldReg));
      }

      branch.freeRegInfo(branch.physRegToInfo(r->m_pReg));
      branch.assignRegInfo(branch.physRegToInfo(r->m_pReg),
                           r->m_cont,
                           RegInfo::CLEAN,
                           r->m_type);
      branch.verify();

      if (r->m_cont.m_kind == RegContent::Int ||
          r->m_cont.m_loc.isLiteral()) {
        ASSERT(r->m_state == RegInfo::CLEAN);
        branch.m_spf->loadImm(r->m_cont.m_int, r->m_pReg);
      } else if (oldReg != InvalidReg) {
        branch.m_spf->fillByMov(oldReg, r->m_pReg);
      } else {
        branch.m_spf->fill(r->m_cont.m_loc, r->m_pReg);
      }
    }
  }

  FOR_EACH_REG_IN_SET (r, unconsideredRegs) {
    if (branch.regIsDirty(r->m_pReg)) {
      branch.cleanReg(r->m_pReg);
    }
  }

  TRACE(1, "Done with branch reconcile\n");
}

void
RegAlloc::cleanReg(PhysReg reg) {
  RegInfo* r = physRegToInfo(reg);
  if (r->m_state == RegInfo::DIRTY) {
    spill(r);
    stateTransition(r, RegInfo::CLEAN);
  }
}

void
RegAlloc::cleanRegs(RegSet regs) {
  FOR_EACH_REG_IN_SET(r, regs) {
    if (r->m_state == RegInfo::DIRTY) {
      spill(r);
      stateTransition(r, RegInfo::CLEAN);
    }
  }
  verify();
}

void RegAlloc::cleanLoc(const Location& loc) {
  RegContent cont(loc);
  PhysReg pr = mapGet(m_contToRegMap, cont, InvalidReg);
  if (pr == InvalidReg) {
    return;
  }
  RegInfo* info = physRegToInfo(pr);
  ASSERT(info->m_state == RegInfo::CLEAN ||
         info->m_state == RegInfo::DIRTY);
  if (info->m_state == RegInfo::DIRTY) {
    spill(info);
    stateTransition(info, RegInfo::CLEAN);
  }
}

void RegAlloc::cleanLocals() {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY &&
        r->m_cont.isLoc() &&
        r->m_cont.m_loc.isLocal()) {
      spill(r);
      stateTransition(r, RegInfo::CLEAN);
    }
  }
  verify();
}

bool RegAlloc::pristine() const {
  return empty() && m_epoch == 0;
}

bool RegAlloc::empty() const {
  FOR_EACH_REG (r) {
    if (r->m_state != RegInfo::FREE) return false;
  }
  return true;
}

void RegAlloc::cleanAll() {
  FOR_EACH_REG(r) {
    if (r->m_state == RegInfo::DIRTY) {
      spill(r);
      stateTransition(r, RegInfo::CLEAN);
    }
  }
  verify();
}

void RegAlloc::spill(RegInfo *toSpill) {
  if (toSpill->m_type == KindOfInvalid) {
    // KindOfInvalid outputs are auto-spilled; it is the translator's
    // responsibility to keep them sync'ed in memory and registers.
    TRACE(1, "spill: (%s, %lld) skipping invalid output\n",
          toSpill->m_cont.m_loc.spaceName(), toSpill->m_cont.m_loc.offset);
    return;
  }
  TRACE(1, "spill: (%s, %lld) <- type %d, r%d\n",
        toSpill->m_cont.m_loc.spaceName(), toSpill->m_cont.m_loc.offset,
        toSpill->m_type, toSpill->m_pReg);
  m_spf->spill(toSpill->m_cont.m_loc, toSpill->m_type, toSpill->m_pReg, true);
  verify();
}

void RegAlloc::smashRegImpl(RegInfo* r) {
  ASSERT(r->m_state != RegInfo::DIRTY);
  if (r->m_state == RegInfo::CLEAN) {
    ContToRegMap::iterator rmi = m_contToRegMap.find(r->m_cont);
    ASSERT(mapContains(m_contToRegMap, r->m_cont));
    m_contToRegMap.erase(rmi);
  }
  // Smash scratch regs, too, if asked.
  TRACE(3, "smashing %d\n", r->m_pReg);
  stateTransition(r, RegInfo::FREE);
  r->m_cont = RegContent();
}

void RegAlloc::smashReg(PhysReg r) {
  smashRegImpl(physRegToInfo(r));
}

void RegAlloc::smashRegs(RegSet toSmash) {
  FOR_EACH_REG_IN_SET(r, toSmash) {
    // Callers responsiblity to scrub this before it was smashed!
    smashRegImpl(r);
  }
  verify();
}

void RegAlloc::smashLoc(const Location& loc) {
  PhysReg reg = mapGet(m_contToRegMap, RegContent(loc), InvalidReg);
  if (reg != InvalidReg) smashReg(reg);
}

void RegAlloc::cleanSmashRegs(RegSet rs) {
  cleanRegs(rs);
  smashRegs(rs);
}

void RegAlloc::cleanSmashReg(PhysReg r) {
  cleanReg(r);
  smashReg(r);
}

void RegAlloc::cleanSmashLoc(const Location& loc) {
  cleanLoc(loc);
  smashLoc(loc);
}

void RegAlloc::killImms(RegSet toKill) {
  FOR_EACH_REG_IN_SET(r, toKill) {
    if (r->m_cont.m_kind == RegContent::Int) {
      // Callers responsiblity to scrub this before it was smashed!
      smashRegImpl(r);
    }
  }
  verify();
}

#define LRU_MOVE(INIT) do {                                             \
  PhysReg last = m_lru[ INIT ];                                         \
  PhysReg target = r->m_pReg;                                           \
  for (int i = SUCC(INIT); last != target; i = SUCC(i)) {               \
    /* Ripple the other registers down until we see the old
     * location of r. This works even if you only have one register,
     * because m_numRegs == 1, so r == m_lru[m_numRegs - 1] and we
     * never enter this loop.
     */                                                                 \
    ASSERT( i >= 0 && i < m_numRegs);                                   \
    PhysReg prevLast = last;                                            \
    last = m_lru[i];                                                    \
    m_lru[i] = prevLast;                                                \
  }                                                                     \
  m_lru[INIT] = target;                                                 \
  verify();                                                             \
} while(0)

void RegAlloc::lruFront(RegInfo* r) {
  r->m_epoch = m_epoch;
#define SUCC(x) (x + 1)
  LRU_MOVE(0);
#undef SUCC
}

void RegAlloc::lruBack(RegInfo* r) {
#define SUCC(x) (x - 1)
  LRU_MOVE(m_numRegs - 1);
#undef SUCC
}

RegInfo*
RegAlloc::physRegToInfo(PhysReg reg) const {
  ASSERT(isValidReg(reg));
  return const_cast<RegInfo*>(&m_info[int(reg)]);
}

/*
 * Scratch regs are not free, but have no Location, cannot be spilled
 * or filled, and do not appear in m_regMap.
 */
PhysReg
RegAlloc::allocScratchReg(PhysReg pr /* = InvalidReg */) {
  if (pr != InvalidReg) {
    RegInfo* ri = physRegToInfo(pr);
    if (ri->m_state == RegInfo::DIRTY) {
      cleanReg(pr);
    }
    smashRegImpl(ri);
    bind(pr, Location(), KindOfInvalid, RegInfo::SCRATCH);
    return pr;
  } else {
    return alloc(Location(), KindOfInvalid, RegInfo::SCRATCH, false)->m_pReg;
  }
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

bool
RegAlloc::checkNoScratch() {
  for (int i = 0; i < m_numRegs; ++i) {
    PhysReg pr = m_lru[i];
    RegInfo* r = physRegToInfo(pr);
    if (r->m_state == RegInfo::SCRATCH) {
      return false;
    }
  }
  return true;
}

std::string
RegAlloc::pretty() const {
  std::ostringstream ss;
  ss << "Most recently used\n";
  ss << "Reg:NUM:STATE:EPOCH:Type:TYPE\n";
  for (int i = 0; i < m_numRegs; ++i) {
    ss << "  " << m_info[(int)m_lru[i]].pretty() << "\n";
  }
  ss << "Least recently used\n";
  return ss.str();
}

RegInfo *
RegAlloc::findFreeReg(const Location& loc) {
  RegSet favoriteRegs = loc.isLocal() ? m_calleeSaved : m_callerSaved;
  int i = 0;
  do {
    PhysReg pr;
    while (favoriteRegs.findFirst(pr)) {
      favoriteRegs.remove(pr);
      RegInfo* r = physRegToInfo(pr);
      if (r->m_state == RegInfo::FREE) {
        m_spf->poison(r->m_pReg);
        return r;
      }
    }
    favoriteRegs = m_allRegs;
  } while(i++ < 2);
  return NULL;
}

void
RegAlloc::assignRegInfo(RegInfo *regInfo, const RegContent &cont,
                        RegInfo::State state, DataType type) {
  ASSERT(regInfo);
  ASSERT(cont.isValid());
  ASSERT(IMPLIES(cont.isInt(), state == RegInfo::CLEAN));
  ASSERT(IMPLIES(cont.isInt(), type == KindOfInt64));

  regInfo->m_cont  = cont;
  regInfo->m_type  = type;
  stateTransition(regInfo, state);

  if (state == RegInfo::CLEAN || state == RegInfo::DIRTY) {
    ASSERT(cont.isValid());
    ASSERT(!cont.isLoc() || cont.m_loc.isValid());

    m_contToRegMap.insert(ContToRegMap::value_type(cont, regInfo->m_pReg));
  }
  if (state != RegInfo::FREE) {
    lruFront(regInfo);
  }
}

void RegAlloc::stateTransition(RegInfo* r, RegInfo::State to) {
  ASSERT(!frozen());
  // The valid state transitions are:
  //     FREE < - > SCRATCH
  //     ^
  //     |--------> CLEAN
  //     |           ^
  //     |           |
  //     |           V
  //     |--------> DIRTY
  //
  // No scratch <-> live transitions.
  ASSERT(r->m_state != RegInfo::SCRATCH || to == RegInfo::FREE);
  // No transitions from the data-bearing states to scratch.
  ASSERT(r->m_state == RegInfo::FREE || to != RegInfo::SCRATCH);
  TRACE(2, "Reg %d from:\n   ", r->m_pReg);
  TRACE(2, *r);
  r->m_state = to;
  TRACE(2, "Reg %d to:\n   ", r->m_pReg);
  TRACE(2, *r);
}

PhysReg
RegAlloc::getImmReg(int64 immVal, bool allowAllocate /* = true */) {
  DataType       type    = KindOfInt64;
  RegInfo::State state   = RegInfo::CLEAN;
  RegInfo       *freeReg = NULL;
  RegContent     cont    = RegContent(immVal);

  // Check if val is already in some reg, and return it if so.
  PhysReg r;
  if (mapGet(m_contToRegMap, cont, &r)) {
    RegInfo* info = physRegToInfo(r);
    ASSERT(info->m_cont == cont);
    ASSERT(info->m_state == RegInfo::CLEAN);
    lruFront(info);
    return r;
  }

  // Fail gracefully if we're frozen
  if (!allowAllocate || frozen()) {
    return InvalidReg;
  }

  // Look for a free reg; give up if none.
  freeReg = findFreeReg(Location());
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
    m_contToRegMap.insert(ContToRegMap::value_type(cont, reg));
  }
  r->m_type = t;
  stateTransition(r, state);
  lruFront(r);
  verify();
}

void
RegAlloc::bindScratch(LazyScratchReg& reg, const Location& loc, DataType t,
                      RegInfo::State state) {
  ASSERT(reg.isAllocated());
  freeScratchReg(*reg);
  bind(*reg, loc, t, state);
}

void
RegAlloc::scrubStackEntries(int firstUnreachable) {
  FOR_EACH_REG(r) {
    if (r->m_cont.isUnreachableStack(firstUnreachable)) {
      TRACE(1, "scrubbing dead stack value: (Stack, %lld)\n",
            r->m_cont.m_loc.offset);
      ASSERT(r->m_state == RegInfo::CLEAN || r->m_state == RegInfo::DIRTY);
      stateTransition(r, RegInfo::CLEAN);
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
      TRACE(1, "scrubbing dead stack value: (Stack, %lld)\n",
            r->m_cont.m_loc.offset);
      ASSERT(r->m_state == RegInfo::CLEAN || r->m_state == RegInfo::DIRTY);
      stateTransition(r, RegInfo::CLEAN);
    }
  }
  verify();
}

void RegAlloc::scrubReg(PhysReg pr) {
  RegInfo* ri = physRegToInfo(pr);
  ASSERT(ri->m_state == RegInfo::CLEAN ||
         ri->m_state == RegInfo::DIRTY ||
         ri->m_state == RegInfo::FREE);
  TRACE(1, "scrubbing register %d: %s\n", pr,
    ri->m_cont.isLoc()
      ? ri->m_cont.m_loc.pretty().c_str() :
    ri->m_cont.isInt()
      ? str(boost::format("(Int %d)") % ri->m_cont.m_int).c_str()
    : "FREE");
  if (ri->m_state != RegInfo::FREE) {
    stateTransition(ri, RegInfo::CLEAN);
  }
  verify();
}

void RegAlloc::scrubRegs(RegSet rs) {
  FOR_EACH_REG_IN_SET (r, rs) {
    scrubReg(r->m_pReg);
  }
}

void RegAlloc::scrubLoc(const Location& l) {
  if (hasReg(l)) scrubReg(getReg(l));
}

void
RegAlloc::swapRegisters(PhysReg pr1, PhysReg pr2) {
  int r1 = int(pr1);
  int r2 = int(pr2);

  ASSERT(m_info[r1].m_state != RegInfo::INVALID &&
         m_info[r1].m_state != RegInfo::FREE);
  ASSERT(m_info[r2].m_state != RegInfo::INVALID &&
         m_info[r2].m_state != RegInfo::FREE);
  RegContent c1 = m_info[r1].m_cont;
  RegContent c2 = m_info[r2].m_cont;
  std::swap(m_info[r1], m_info[r2]);

  TRACE(1, "swap registers %d <---> %d\n", r1, r2);

  // pReg
  m_info[r1].m_pReg = PhysReg(r1);
  m_info[r2].m_pReg = PhysReg(r2);

  // content map.
  if (m_info[r2].m_state != RegInfo::SCRATCH) {
    m_contToRegMap[c1] = PhysReg(r2);
  }
  if (m_info[r1].m_state != RegInfo::SCRATCH) {
    m_contToRegMap[c2] = PhysReg(r1);
  }

  // consider this a touch on both regs.
  lruFront(&m_info[r2]);
  lruFront(&m_info[r1]);
  verify();
}

void
RegAlloc::verify() {
#ifdef DEBUG
  RegSet allRegs = m_allRegs;

  // LRU invariants
  RegSet lruRegs;
  for (int i = 0; i < m_numRegs; ++i) {
    PhysReg pr = m_lru[i];
    RegInfo* r = physRegToInfo(pr);
    ASSERT(r->m_pReg == pr);
    // The state is reasonable
    ASSERT(r->m_state == RegInfo::FREE ||
           r->m_state == RegInfo::CLEAN ||
           r->m_state == RegInfo::SCRATCH ||
           r->m_state == RegInfo::DIRTY);
    // Each reg appears only once.
    ASSERT(!lruRegs.contains(pr));
    lruRegs |= RegSet(r->m_pReg);
  }
  // All regs are there.
  ASSERT(lruRegs == allRegs);
  lruRegs.clear();

  FOR_EACH_REG(r) {
    // Each reg appears only once.
    ASSERT(!lruRegs.contains(r->m_pReg));
    lruRegs |= RegSet(r->m_pReg);
  }
  // All regs are there.
  ASSERT(lruRegs == allRegs);

  // The map from content to registers.
  for (ContToRegMap::const_iterator lri = m_contToRegMap.begin();
       lri != m_contToRegMap.end(); ++lri) {
    const RegContent& cont = lri->first;
    const RegInfo* ri = physRegToInfo(lri->second);
    // The location and mapping are consistent.
    ASSERT(ri->m_cont == cont);
    // If it's a location, make sure it's is valid.
    ASSERT(IMPLIES(ri->m_cont.isLoc(), ri->m_cont.m_loc.isValid()));
    // If it's an integer/immediate, make sure it's clean.
    ASSERT(IMPLIES(ri->m_cont.isInt(), ri->m_state == RegInfo::CLEAN));
    // The register is live.
    ASSERT(ri->m_state != RegInfo::FREE);
  }

  FOR_EACH_REG(r) {
    if (r->m_state != RegInfo::FREE &&
        r->m_state != RegInfo::SCRATCH) {
      ASSERT(mapContains(m_contToRegMap, r->m_cont));
      ASSERT(m_contToRegMap[r->m_cont] == r->m_pReg);
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


LazyScratchReg::LazyScratchReg(RegAlloc& regMap) :
  m_regMap(regMap),
  m_reg(noreg) {
}

LazyScratchReg::~LazyScratchReg() {
  dealloc();
}

void
LazyScratchReg::alloc(PhysReg pr /* = InvalidReg */) {
  ASSERT(m_reg == noreg);
  if (pr != InvalidReg) {
    m_regMap.assertRegIsFree(pr);
  }
  m_reg = m_regMap.allocScratchReg(pr);
  TRACE(1, "LazyScratchReg: alloc %d\n", m_reg);
}

void LazyScratchReg::dealloc() {
  if (m_reg != noreg) {
    TRACE(1, "LazyScratchReg: free %d\n", m_reg);
    m_regMap.freeScratchReg(m_reg);
    m_reg = noreg;
  }
}

void LazyScratchReg::realloc(PhysReg pr /* = InvalidReg */) {
  ASSERT(m_reg != noreg);
  dealloc();
  alloc(pr);
}

PhysReg LazyScratchReg::operator*() const {
  ASSERT(m_reg != noreg);
  return m_reg;
}

ScratchReg::ScratchReg(RegAlloc& regMap) :
  LazyScratchReg(regMap) {
  alloc();
}

ScratchReg::ScratchReg(RegAlloc& regMap, PhysReg reg) :
  LazyScratchReg(regMap) {
  alloc(reg);
  TRACE(1, "ScratchReg: wired alloc %d\n", m_reg);
}

static PhysReg getRegForDumb(RegSet& regs) {
  PhysReg ret;
  if (!regs.findFirst(ret)) {
    ASSERT(false &&
      "DumbScratchReg can only be used when you know you have "
      "enough registers.  We ran out.");
    throw std::runtime_error("DumbScratchReg ran out of registers");
  }
  regs.remove(ret);
  return ret;
}

DumbScratchReg::DumbScratchReg(RegSet& regs)
  /*
   * We could heuristically try to select registers to prefer using
   * regs that don't have REX prefixes or something.  But we don't
   * really know how long-lived these guys or how many uses they will
   * have.  (We could have the calleer provide a hint, but for now we
   * just do this all braindead.)
   */
  : m_regPool(regs)
  , m_reg(getRegForDumb(regs))
{}

DumbScratchReg::~DumbScratchReg() {
  ASSERT(!m_regPool.contains(m_reg) &&
         "The register we thought we owned was already back in the pool");
  m_regPool.add(m_reg);
}

PhysReg DumbScratchReg::operator*() const {
  return m_reg;
}

} } } // HPHP::VM::Transl
