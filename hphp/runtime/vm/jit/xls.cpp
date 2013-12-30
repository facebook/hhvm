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

#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/check.h"

#include <unordered_set>
#include <algorithm>

// TODO
//  - #3098109 dests of branch instructions start in next block
//  - #3098509 streamline code, vectors vs linked lists, etc
//  - #3098661 generate spill stats so we can compare side/by/side
//  - #3098678 EvalHHIREnablePreColoring
//  - #3098685 EvalHHIREnableCoalescing, by using hints
//  - #3098685 Optimize lifetime splitting
//  - #3098712 reuse spill slots
//  - #3098739 new features now possible with XLS
//  - #3099647 support loops

namespace HPHP { namespace JIT {
namespace {
using namespace reg;
struct Interval;
struct RegPositions;

// machine-specific register conventions
struct Abi {
  RegSet gp;      // general purpose 64-bit registers
  RegSet simd;    // floating point / simd 128-bit registers
  RegSet saved;   // callee-saved (gp and simd)

  // convenience methods
  RegSet all() const { return gp | simd; }
};

typedef StateVector<SSATmp, Interval*> Intervals;
typedef IdSet<SSATmp> LiveSet;
typedef std::pair<PhysReg,PhysReg> RegPair;

// A Use refers to the position where an interval is read or written.
struct Use {
  unsigned pos;
};

// A LiveRange is an open-ended range of positions where an interval is live.
struct LiveRange {
  LiveRange(unsigned s, unsigned e) : start(s), end(e) { assert(s <= e); }
  bool contains(unsigned pos) const { return pos >= start && pos < end; }
  bool intersects(LiveRange r) const { return r.start < end && start < r.end; }
  bool contains(LiveRange r) const;
public:
  unsigned start, end;
};

// return [first, last+1)
LiveRange closedRange(unsigned first, unsigned last);

// An Interval stores the lifetime of an SSATmp as a sorted list of disjoint
// ranges, and a sorted list of use positions.  If this interval was split,
// then children contains the split intervals in start-order, and each child's
// parent points to this interval.
struct Interval {
  Interval() {}
  explicit Interval(Interval* parent);
  // accessors
  bool empty() const { return ranges.empty(); }
  unsigned start() const { return ranges.front().start; }
  unsigned end() const { return ranges.back().end; }
  Interval* leader() { return parent ? parent : this; }
  const Interval* leader() const { return parent ? parent : this; }
  bool isChild() const { return parent != nullptr; }
  // queries
  bool covers(unsigned pos) const;
  bool usedAt(unsigned pos) const;
  Interval* childAt(unsigned pos);
  unsigned nextIntersect(Interval*) const;
  unsigned nextUseAfter(unsigned pos) const;
  unsigned firstRegUse() const;
  unsigned firstUse() const { return uses.front().pos; }
  // mutators
  void add(LiveRange r);
  void setStart(unsigned start);
  void addUse(unsigned pos) { uses.push_front(Use{pos}); }
  Interval* split(unsigned pos);
  // register/spill assignment
  RegPair regs() const;
  bool handled() const { return info.hasReg(0) || info.spilled(); }
  // debugging
  std::string toString();
  bool checkInvariants() const;
  uint32_t id() const { return leader()->tmp->id(); }
public:
  bool blocked { false }; // cannot be spilled
  bool scratch { false }; // used as scratch or arg, cannot be spilled
  int need { 0 }; // number of required registers (exactly 1 or 2)
  RegSet allow;
  RegSet prefer;
  SSATmp* tmp { nullptr };
  Interval* parent { nullptr }; // if this is a split-off child
  smart::list<LiveRange> ranges;
  smart::list<Use> uses;
  smart::list<Interval> children; // if parent was split
  PhysLoc info;  // current location assigned to this interval
  PhysLoc spill; // spill location (parent only)
};

// Extended Linear Scan. This just encapsulates the intermediate
// data structures we use during the algorithm so we don't have
// to pass them around everywhere.
struct XLS {
  XLS(IRUnit& unit, RegAllocInfo& regs, const Abi&);
  ~XLS();
  void allocate();
  // phases
  void prepareBlocks();
  void computePositions();
  void buildIntervals();
  void walkIntervals();
  void assignLocations();
  void resolveSplits();
  void resolveEdges();
  // utilities
  void enqueue(Interval* interval);
  bool allocPrefer(Interval* current, const RegPositions& until);
  void allocOne(Interval* current);
  void allocBlocked(Interval* current);
  Interval* goodSplit(Interval*, unsigned pos);
  void spill(Interval*);
  void assign(Interval*, RegPair r);
  void update(unsigned pos);
  void spillActive(Interval* current);
  void spillInactive(Interval* current);
  bool force(IRInstruction&, SSATmp& t);
  void insertCopy(Block* b, Block::iterator, IRInstruction*& shuffle,
                  SSATmp* src, const PhysLoc& rs, const PhysLoc& rd);
  void insertSpill(Interval* ivl);
  // debugging
  void print(const char* caption);
  void dumpIntervals();
  bool checkEdgeShuffles();
private:
  struct Compare { bool operator()(const Interval*, const Interval*); };
private:
  Intervals m_intervals; // parent intervals indexed by ssatmp
  int m_nextSpill { 0 };
  int m_numSplits { 0 };
  IRUnit& m_unit;
  RegAllocInfo& m_regs;
  const Abi& m_abi;
  StateVector<IRInstruction, unsigned> m_posns;
  smart::vector<IRInstruction*> m_insts;
  BlockList m_blocks;
  PhysReg::Map<Interval> m_scratch;
  PhysReg::Map<Interval> m_blocked;
  StateVector<Block,LiveSet> m_liveIn;
  smart::priority_queue<Interval*,Compare> m_pending;
  smart::vector<Interval*> m_active;
  smart::vector<Interval*> m_inactive;
  StateVector<IRInstruction,IRInstruction*> m_between;
  StateVector<Block,IRInstruction*> m_before;
  StateVector<Block,IRInstruction*> m_after;
};

const uint32_t kMaximalPos = UINT32_MAX; // "infinity" use position

// Keep track of a future use or conflict position for each register.
// Initially all usable registers have kMaximalPos
struct RegPositions {
  explicit RegPositions();
  RegPair max2Reg(RegSet regs) const;
  unsigned operator[](PhysReg r) const { return posns[r]; }
  void setPos(Interval*, unsigned pos);
private:
  PhysReg::Map<unsigned> posns;
};

bool checkBlockOrder(IRUnit& unit, BlockList& blocks);

//////////////////////////////////////////////////////////////////////////////

// returns true if this range contains r
bool LiveRange::contains(LiveRange r) const {
  return r.start >= start && r.end <= end;
}

// Return a range for the closed interval [first,last], i.e. [first, last+1)
LiveRange closedRange(unsigned first, unsigned last) {
  return LiveRange(first, last + 1);
}

//////////////////////////////////////////////////////////////////////////////

Interval::Interval(Interval* parent)
  : need(parent->need)
  , allow(parent->allow)
  , prefer(parent->prefer)
  , tmp(parent->tmp)
  , parent(parent)
{}

// Add r to this interval, merging r with any existing overlapping ranges
void Interval::add(LiveRange r) {
  while (!ranges.empty() && r.contains(ranges.front())) {
    ranges.pop_front();
  }
  if (ranges.empty()) {
    return ranges.push_front(r);
  }
  auto& f = ranges.front();
  if (f.contains(r)) return;
  if (r.end >= f.start) {
    f.start = r.start;
  } else {
    ranges.push_front(r);
  }
  assert(checkInvariants());
}

// Update the start-position of the frontmost range in this interval
void Interval::setStart(unsigned start) {
  assert(!ranges.empty() && ranges.front().contains(start));
  ranges.front().start = start;
}

// Return true if one of the ranges in this interval includes pos
bool Interval::covers(unsigned pos) const {
  for (auto r : ranges) {
    if (pos < r.start) return false;
    if (pos < r.end) return true;
  }
  return false;
}

// Return true if there is a use position at pos
bool Interval::usedAt(unsigned pos) const {
  for (auto& u : uses) if (u.pos == pos) return true;
  return false;
}

// Return the interval which includes pos (this one or a child)
Interval* Interval::childAt(unsigned pos) {
  assert(!isChild());
  if (covers(pos)) return this;
  for (auto& child : children) {
    if (child.start() > pos) break;
    if (child.covers(pos)) return &child;
  }
  return nullptr;
}

// return the next intersection point between this and ivl, or MAX_UINT32
// if they never intersect.
unsigned Interval::nextIntersect(Interval* ivl) const {
  assert(!ranges.empty() && !ivl->ranges.empty());
  auto r1 = ranges.begin(), e1 = ranges.end();
  auto r2 = ivl->ranges.begin(), e2 = ivl->ranges.end();
  for (;;) {
    if (r1->start < r2->start) {
      if (r2->start < r1->end) return r2->start;
      if (++r1 == e1) return kMaximalPos;
    } else {
      if (r1->start < r2->end) return r1->start;
      if (++r2 == e2) return kMaximalPos;
    }
  }
  return kMaximalPos;
}

// Split this interval at pos and return the rest.  Pos must be
// a location that ensures both shorter intervals are nonempty.
// Pos must also be even, indicating a position between instructions.
Interval* Interval::split(unsigned pos) {
  assert(checkInvariants());
  assert(pos > start() && pos < end());
  auto leader = this->leader();
  auto iter = leader->children.begin();
  while (iter != leader->children.end() && pos > iter->start()) {
    iter++;
  }
  iter = leader->children.emplace(iter, leader);
  Interval* child = &(*iter);
  // move r to the first range we want in child, splitting a range if needed.
  auto r = ranges.begin();
  while (pos >= r->end) r++;
  if (pos > r->start) { // split r at pos
    child->ranges.push_back(LiveRange(pos, r->end));
    r->end = pos;
    r++;
  }
  child->ranges.splice(child->ranges.end(), ranges, r, ranges.end());
  // move u to the first use position in child
  auto u = uses.begin(), uses_end = uses.end();
  for (; u != uses_end && pos > u->pos; ++u) {}
  child->uses.splice(child->uses.end(), uses, u, uses_end);
  assert(child->checkInvariants());
  return child;
}

// Return the position of the next use of this interval after (or equal to)
// pos.  If there are no more uses after pos, return MAX.
unsigned Interval::nextUseAfter(unsigned pos) const {
  for (auto& u : uses) {
    if (pos <= u.pos) return u.pos;
  }
  return kMaximalPos;
}

// return the position of the first use that requires a register,
// or kMaximalPos if no remaining uses need registers.
unsigned Interval::firstRegUse() const {
  return uses.empty() ? kMaximalPos : uses.front().pos;
}

// Return the register(s) assigned to this interval as a pair.
RegPair Interval::regs() const {
  return RegPair(info.reg(0), info.reg(1));
}

//////////////////////////////////////////////////////////////////////////////

XLS::XLS(IRUnit& unit, RegAllocInfo& regs, const Abi& abi)
  : m_intervals(unit, nullptr)
  , m_unit(unit)
  , m_regs(regs)
  , m_abi(abi)
  , m_posns(unit, 0)
  , m_liveIn(unit, LiveSet())
  , m_between(unit, nullptr)
  , m_before(unit, nullptr)
  , m_after(unit, nullptr) {
  auto all = abi.all();
  for (auto r : m_blocked) {
    m_blocked[r].blocked = true;
    m_blocked[r].need = 1;
    m_blocked[r].info.setReg(r, 0);
    if (!all.contains(r)) {
      // r is never available
      m_blocked[r].add(LiveRange(0, kMaximalPos));
    }
    m_scratch[r].scratch = true;
    m_scratch[r].need = 1;
    m_scratch[r].info.setReg(r, 0);
  }
}

XLS::~XLS() {
  for (auto ivl : m_intervals) {
    if (ivl) smart_delete(ivl);
  }
}

// Split critical edges, remove dead predecessor edges, and put blocks
// in a sutiable order.
void XLS::prepareBlocks() {
  splitCriticalEdges(m_unit);
  m_blocks = rpoSortCfg(m_unit);
}

// compute the position number for each instruction.  Each instruction
// gets two positions: the position where srcs are read, and the position
// where dests are written.
void XLS::computePositions() {
  m_insts.reserve(m_unit.numInsts());
  unsigned pos = 0;
  for (auto b : m_blocks) {
    for (auto& inst : *b) {
      m_insts[pos/2] = &inst;
      m_posns[inst] = pos;
      pos += 2;
    }
  }
}

// Return true if t was forced into a register.
bool XLS::force(IRInstruction& inst, SSATmp& t) {
  auto reg = forceAlloc(t);
  if (reg != InvalidReg) {
    m_regs[inst][t].setReg(reg, 0);
    return true;
  }
  return false;
}

bool allocUnusedDest(IRInstruction&) {
  return false;
}

// Reduce the allow and prefer sets according to this particular use
void srcConstraints(IRInstruction& inst, int i, SSATmp* src, Interval* ivl,
                    const Abi& abi) {
  if (RuntimeOption::EvalHHIRAllocSIMDRegs) {
    if (src->type() <= Type::Dbl) {
      ivl->prefer &= abi.simd;
      return;
    }
    if (inst.storesCell(i) && src->numWords() == 2) {
      ivl->prefer &= abi.simd;
      return;
    }
  }
  ivl->allow &= abi.gp;
  ivl->prefer &= abi.gp;
}

// Reduce the allow and prefer constraints based on this definition
void dstConstraints(IRInstruction& inst, int i, SSATmp* dst, Interval* ivl,
                    const Abi& abi) {
  if (RuntimeOption::EvalHHIRAllocSIMDRegs) {
    if (dst->type() <= Type::Dbl) {
      ivl->prefer &= abi.simd;
      return;
    }
    if (inst.isLoad() && !inst.isControlFlow() && dst->numWords() == 2) {
      ivl->prefer &= abi.simd;
      if (!(ivl->allow & ivl->prefer).empty()) {
        // we prefer simd, but allow gp and simd, so restrict allow to just
        // simd to avoid (simd)<->(gpr,gpr) shuffles.
        ivl->allow = ivl->prefer;
      }
      return;
    }
  }
  ivl->allow &= abi.gp;
  ivl->prefer &= abi.gp;
}

// build intervals in one pass by walking the block list backwards.
// no special handling is needed for goto/label (phi) instructions because
// they use/define tmps in the expected locations.
void XLS::buildIntervals() {
  assert(checkBlockOrder(m_unit, m_blocks));
  size_t stress = 0;
  for (auto blockIt = m_blocks.end(); blockIt != m_blocks.begin();) {
    auto block = *--blockIt;
    // compute initial live set from liveIn[succsessors]
    LiveSet live;
    if (auto taken = block->taken()) live |= m_liveIn[taken];
    if (auto next  = block->next())  live |= m_liveIn[next];
    // initialize live range for each live tmp to whole block
    auto blockRange = closedRange(m_posns[block->front()],
                                  m_posns[block->back()] + 1);
    live.forEach([&](uint32_t id) {
      m_intervals[id]->add(blockRange);
    });
    for (auto instIt = block->end(); instIt != block->begin();) {
      auto& inst = *--instIt;
      auto spos = m_posns[inst]; // position where srcs are read
      auto dpos = spos + 1;     // position where dsts are written
      size_t dst_need = 0;
      for (unsigned i = 0, n = inst.numDsts(); i < n; ++i) {
        auto d = inst.dst(i);
        auto dest = m_intervals[d];
        if (!live[d]) {
          // dest is not live; give it a register anyway.
          assert(!dest);
          if (d->numWords() == 0) continue;
          if (force(inst, *d)) continue;
          if (!allocUnusedDest(inst)) continue;
          m_intervals[d] = dest = smart_new<Interval>();
          dest->add(closedRange(dpos, dpos));
          dest->tmp = d;
          dest->need = d->numWords();
          dest->allow = dest->prefer = m_abi.all();
        } else {
          // adjust start pos for live intervals defined by this instruction
          assert(dest->tmp == d);
          dest->setStart(dpos);
          live.erase(d);
        }
        dst_need += dest->need;
        dest->addUse(dpos);
        dstConstraints(inst, i, d, dest, m_abi);
      }
      stress = std::max(dst_need, stress);
      if (inst.isNative()) {
        if (RuntimeOption::EvalHHIREnableCalleeSavedOpt) {
          auto scratch = m_abi.gp - m_abi.saved;
          scratch.forEach([&](PhysReg r) {
            m_scratch[r].add(LiveRange(spos, dpos));
          });
        }
      }
      // add live ranges for tmps used by this inst
      size_t src_need = 0;
      for (unsigned i = 0, n = inst.numSrcs(); i < n; ++i) {
        auto s = inst.src(i);
        if (s->inst()->op() == DefConst) continue;
        auto need = s->numWords();
        if (need == 0) continue;
        if (force(inst, *s)) continue;
        auto src = m_intervals[s];
        if (!src) m_intervals[s] = src = smart_new<Interval>();
        src->add(closedRange(blockRange.start, spos));
        src->addUse(spos);
        if (!src->tmp) {
          src->tmp = s;
          src->need = need;
          src->allow = src->prefer = m_abi.all();
        } else {
          assert(src->tmp == s);
        }
        srcConstraints(inst, i, s, src, m_abi);
        src_need += src->need;
        live.add(s);
      }
      stress = std::max(src_need, stress);
    }
    m_liveIn[block] = live;
  }
  if (dumpIREnabled()) {
    print(" after building intervals ");
  }
  stress += RuntimeOption::EvalHHIRNumFreeRegs;
  for (auto r : m_blocked) {
    if (m_blocked[r].start() > 0) {
      if (stress > 0) {
        stress--;
      } else {
        m_blocked[r].add(LiveRange(0, kMaximalPos));
      }
    }
  }
}

// comparison function for pending priority queue. std::priority_queue
// requies a less operation, but sorts the heap highest-first; we
// need the opposite (lowest-first), so use greater-than.
bool XLS::Compare::operator()(const Interval* i1, const Interval* i2) {
  return i1->start() > i2->start();
}

// insert interval into pending list in order of start position
void XLS::enqueue(Interval* ivl) {
  assert(ivl->checkInvariants() && !ivl->handled());
  m_pending.push(ivl);
}

// Assign the next available spill slot to interval
void XLS::spill(Interval* ivl) {
  assert(!ivl->blocked && !ivl->scratch && ivl->isChild());
  assert(ivl->need == 1 || ivl->need == 2);
  auto leader = ivl->leader();
  if (!leader->spill.spilled()) {
    leader->spill.setSlot(0, m_nextSpill++);
    if (ivl->need == 2) leader->spill.setSlot(1, m_nextSpill++);
    if (m_nextSpill > NumPreAllocatedSpillLocs) {
      PUNT(LinearScan_TooManySpills);
    }
  }
  ivl->info = leader->spill;
}

// Assign one or both of the registers in r to this interval.
void XLS::assign(Interval* ivl, RegPair r) {
  assert(!ivl->blocked && !ivl->scratch);
  auto r0 = PhysReg(r.first);
  auto r1 = PhysReg(r.second);
  if (ivl->need == 1) {
    ivl->info.setReg(r0, 0);
  } else {
    if (r0.isSIMD()) {
      ivl->info.setRegFullSIMD(r0);
    } else if (r1.isSIMD()) {
      ivl->info.setRegFullSIMD(r1);
    } else {
      assert(r0 != r1);
      ivl->info.setReg(r0, 0);
      ivl->info.setReg(r1, 1);
    }
  }
}

// initialize the positions array with maximal use positions.
RegPositions::RegPositions() {
  for (auto r : posns) posns[r] = kMaximalPos;
}

// Find the two registers used furthest in the future, but only
// consider registers in the given set
RegPair RegPositions::max2Reg(const RegSet regs) const {
  unsigned max1 = 0, max2 = 0;
  PhysReg r1 = *posns.begin(), r2 = *posns.begin();
  regs.forEach([&](PhysReg r) {
    if (posns[r] > max2) {
      if (posns[r] > max1) {
        r2 = r1; max2 = max1;
        r1 = r; max1 = posns[r];
      } else {
        r2 = r; max2 = posns[r];
      }
    }
  });
  assert(posns[r1] >= posns[r2]);
  return { r1, r2 };
}

// Update the position associated with the registers assigned to ivl,
// to the minimum of pos and the existing position.
void RegPositions::setPos(Interval* ivl, unsigned pos) {
  assert(ivl->info.numAllocated() >= 1);
  auto r0 = ivl->info.reg(0);
  posns[r0] = std::min(pos, posns[r0]);
  if (ivl->info.numAllocated() == 2) {
    auto r1 = ivl->info.reg(1);
    posns[r1] = std::min(pos, posns[r1]);
  }
}

bool XLS::allocPrefer(Interval* current, const RegPositions& until) {
  auto r = until.max2Reg(current->prefer);
  auto untilPos = current->need == 1 ? until[r.first] : // 1 needed, 1 found
                 r.second != r.first ? until[r.second] : // 2 needed, 2 found
                 0; // 2 needed, 1 found
  if (untilPos > current->end()) {
    assign(current, r);
    m_active.push_back(current);
    return true;
  }
  return false;
}

// Allocate one register for the current interval.
// First try to find a register to assign to current, or its first part.
// If none can be found, tail-call to allocBlocked which will spill
// something, maybe current or another interval.
//
// SSA form guarantees that two SSA intervals that intersect,
// must intersect at one or the other's start position.  Current
// does not intersect with inactive intervals at current->start(),
// and inactive intervals must have started earlier.  Thus they
// cannot intersect.  But we can only skip the intersection test
// for the original interval -- split intervals no longer have
// the SSA property.
void XLS::allocOne(Interval* current) {
  assert(!current->handled());
  RegPositions until;
  for (auto ivl : m_active) {
    until.setPos(ivl, 0);
  }
  for (auto ivl : m_inactive) {
    until.setPos(ivl, current->nextIntersect(ivl));
  }
  if (allocPrefer(current, until)) {
    return;
  }
  // find the register(s) that are free for the longest time.
  auto r = until.max2Reg(current->allow);
  auto untilPos = current->need == 1 ? until[r.first] : // 1 needed, 1 found
                 r.second != r.first ? until[r.second] : // 2 needed, 2 found
                 0; // 2 needed, 1 found
  if (untilPos <= current->start()) {
    return allocBlocked(current);
  }
  if (untilPos < current->end()) {
    auto second = goodSplit(current, untilPos); // got register for first part
    enqueue(second);
  }
  assign(current, r);
  m_active.push_back(current);
}

// When all registers are in use, find a good interval to split and spill,
// which could be the current interval.  When an interval is split and the
// second part is spilled, possibly split the second part again before the
// next use-pos that requires a register, and enqueue the third part.
void XLS::allocBlocked(Interval* current) {
  RegPositions used;
  RegPositions blocked;
  auto start = current->start();
  // compute next use of active registers, so we can pick the furthest one
  for (auto ivl : m_active) {
    if (ivl->blocked) {
      blocked.setPos(ivl, 0);
      used.setPos(ivl, 0);
    } else {
      used.setPos(ivl, ivl->nextUseAfter(start));
    }
  }
  // compute next intersection/use of inactive regs to find whats free longest
  for (auto ivl : m_inactive) {
    auto intersectPos = current->nextIntersect(ivl);
    if (intersectPos == kMaximalPos) continue;
    if (ivl->blocked) {
      blocked.setPos(ivl, intersectPos);
      used.setPos(ivl, blocked[ivl->regs().first]);
    } else {
      used.setPos(ivl, ivl->nextUseAfter(start));
    }
  }
  auto r = used.max2Reg(current->allow);
  auto usedPos = current->need == 1 ? used[r.first] : used[r.second];
  if (usedPos < current->firstUse()) {
    // all active+inactive intervals used before current: spill current
    auto second = goodSplit(current, current->firstRegUse());
    assert(second != current);
    spill(current);
    enqueue(second);
    return;
  }
  auto blockPos = current->need == 1 ? blocked[r.first] : blocked[r.second];
  assert(blockPos >= usedPos);
  if (blockPos < current->end()) {
    // spilling made a register free for first part of current
    auto second = goodSplit(current, blockPos);
    assert(second != current);
    enqueue(second);
  }
  assign(current, r);
  spillActive(current);
  spillInactive(current);
  m_active.push_back(current);
}

// return true if r1 and r2 have any registers in common
bool conflict(RegPair r1, RegPair r2) {
  assert(r1.first != InvalidReg && r2.first != InvalidReg);
  assert(r1.first != r1.second && r2.first != r2.second);
  return r1.first == r2.first ||
         r1.first == r2.second ||
         (r1.second != InvalidReg &&
          (r1.second == r2.first || r1.second == r2.second));
}

// Split and spill other active intervals that conflict with current for
// register r, at current->start().  If necessary, split the victims
// again before their first use position that requires a register.
void XLS::spillActive(Interval* current) {
  auto start = current->start();
  auto r = current->regs();
  for (auto i = m_active.begin(); i != m_active.end();) {
    auto i2 = i++;
    Interval* first = *i2;
    if (first->scratch) continue;
    auto r2 = first->regs();
    if (!conflict(r, r2)) continue;
    // split and spill first at current.start
    i = m_active.erase(i2);
    auto second = goodSplit(first, start);
    auto reloadPos = second->firstRegUse();
    if (reloadPos <= start) {
      // not enough registers to hold srcs that must be in a register.
      PUNT(RegSpill);
    }
    if (reloadPos < kMaximalPos) {
      if (reloadPos > second->start()) {
        auto third = goodSplit(second, reloadPos);
        spill(second);
        enqueue(third);
      } else {
        enqueue(second);
      }
    } else {
      spill(second);
    }
  }
}

// Split and spill other inactive intervals that conflict with current
// for register r, at current->start().  If necessary, split the victims
// again before their first use position that requires a register.
void XLS::spillInactive(Interval* current) {
  auto start = current->start();
  auto r = current->regs();
  for (auto i = m_inactive.begin(); i != m_inactive.end();) {
    auto i2 = i++;
    Interval* first = *i2;
    if (first->scratch) continue;
    auto r2 = first->regs();
    if (!conflict(r, r2)) continue;
    auto intersect = current->nextIntersect(first);
    if (intersect == kMaximalPos) continue;
    // split and spill the rest of it starting @current
    i = m_inactive.erase(i2);
    auto second = goodSplit(first, start);
    auto reloadPos = second->firstRegUse();
    if (reloadPos <= start) {
      // not enough registers to hold srcs that must be in a register.
      PUNT(RegSpill);
    }
    if (reloadPos < kMaximalPos) {
      if (reloadPos > second->start()) {
        auto third = goodSplit(second, reloadPos);
        spill(second);
        enqueue(third);
      } else {
        enqueue(second);
      }
    } else {
      spill(second);
    }
  }
}

// split interval at or before pos, then return the new child.
// If the position is at ivl.start, return the whole ivl without
// splitting it.
Interval* XLS::goodSplit(Interval* ivl, unsigned pos) {
  m_numSplits++;
  if (pos <= ivl->start()) return ivl;
  if (pos % 2 == 1 && m_insts[pos/2]->op() == DefLabel) {
    // correctness: move split point to start of block instead of
    // in the middle of the DefLabel instruction.
    pos--;
  }
  return ivl->split(pos);
}

// Update active/inactive sets based on pos
void XLS::update(unsigned pos) {
  // check for intervals in active that are expired or inactive
  for (auto i = m_active.begin(); i != m_active.end();) {
    auto ivl = *i;
    if (ivl->end() <= pos) {
      i = m_active.erase(i); // active is now handled.
    } else if (!ivl->covers(pos)) {
      i = m_active.erase(i);
      m_inactive.push_back(ivl);
    } else {
      i++;
    }
  }
  // check for intervals that are expired or active
  for (auto i = m_inactive.begin(); i != m_inactive.end();) {
    auto ivl = *i;
    if (ivl->end() <= pos) {
      i = m_inactive.erase(i); // inactive is now handled.
    } else if (ivl->covers(pos)) {
      i = m_inactive.erase(i);
      m_active.push_back(ivl);
    } else {
      i++;
    }
  }
}

// assign registers to intervals, split & spill where needed.
void XLS::walkIntervals() {
  // fill the pending queue with nonempty intervals in order of start position
  for (auto ivl : m_intervals) {
    if (ivl) m_pending.push(ivl);
  }
  for (auto r : m_scratch) {
    if (!m_scratch[r].empty()) m_inactive.push_back(&m_scratch[r]);
  }
  for (auto r : m_blocked) {
    if (!m_blocked[r].empty()) m_inactive.push_back(&m_blocked[r]);
  }
  while (!m_pending.empty()) {
    Interval* current = m_pending.top();
    m_pending.pop();
    update(current->start());
    allocOne(current);
    assert(current->handled() && current->info.numWords() == current->need);
  }
  if (dumpIREnabled()) {
    if (m_numSplits) dumpIntervals();
    print(" after walking intervals ");
  }
}

// Assign PhysLocs (registers or spill slots) to every src and dst ssatmp
void XLS::assignLocations() {
  for (auto b : m_blocks) {
    for (auto& inst : *b) {
      auto spos = m_posns[inst];
      for (auto s : inst.srcs()) {
        if (!m_intervals[s]) continue;
        m_regs[inst][s] = m_intervals[s]->childAt(spos)->info;
      }
      for (auto& d : inst.dsts()) {
        if (!m_intervals[d]) continue;
        m_regs[inst][d] = m_intervals[d]->info;
      }
    }
  }
}

// Add a copy of tmp from rs to rd to the Shuffle instruction shuffle,
// if it exists.  Otherwise, create a new one and insert it at pos in block b.
void XLS::insertCopy(Block* b, Block::iterator pos, IRInstruction* &shuffle,
                     SSATmp* src, const PhysLoc& rs, const PhysLoc& rd) {
  if (rs == rd) return;
  if (shuffle) {
    // already have shuffle here
    shuffle->addCopy(m_unit, src, rd);
  } else {
    auto cap = 1;
    auto dests = new (m_unit.arena()) PhysLoc[cap];
    dests[0] = rd;
    auto& marker = pos != b->end() ? pos->marker() : b->back().marker();
    shuffle = m_unit.gen(Shuffle, marker, ShuffleData(dests, 1, cap), src);
    b->insert(pos, shuffle);
  }
  m_regs[shuffle][src] = rs;
}

// Return the first instruction in block b that is not a Shuffle
IRInstruction* skipShuffle(Block* b) {
  auto i = b->begin();
  while (i->op() == Shuffle) ++i;
  return i != b->end() ? &*i : nullptr;
}

// Insert a spill-store Shuffle after the instruction that defines ivl->tmp.
// If the instruction is a branch, do the store at the beginning of the
// next block.
void XLS::insertSpill(Interval* ivl) {
  auto inst = ivl->tmp->inst();
  if (inst->isBlockEnd()) {
    auto succ = inst->next();
    auto pos = succ->skipHeader();
    insertCopy(succ, pos, m_before[succ], ivl->tmp, ivl->info, ivl->spill);
  } else {
    auto block = inst->block();
    auto pos = block->iteratorTo(inst);
    auto& shuffle = (++pos) != block->end() ? m_between[*pos] :
                    m_after[block];
    insertCopy(block, pos, shuffle, ivl->tmp, ivl->info, ivl->spill);
  }
}

/*
 * Insert Shuffle instructions.  Each Shuffle at a given position implements
 * a parallel copy: all sources are read before any destination is written.
 * 1. If any part of interval was spilled, insert a copy (store) to the spill
 * slot after the defining instruction.
 * 2. For intervals split in the middle of a block, connect the two
 * sub-intervals by inserting a copy at the split point.  "middle" means any
 * program point after the first instruction and before the last instruction
 * in the block.  Intervals that were split on block boundaries are handled
 * in step 3.
 * 3. (resolveSplits) When a sub-interval is live at the start of a block
 * (i.e. live-in), and a different sub-interval was live at the end of the
 * predecessor, insert a copy on the edge connecting the two blocks.  If that
 * edge is a critical edge, split it first.
 * 4. Resolve "phi"s by inserting copies on Jmp->DefLabel edges.
 * These are similar to case 3, except they are two different intervals
 * logically connected by the phi (Jmp->DefLabel) copy.  This is done at the
 * same time as resolving edges.
 */

// Insert spills and copies that connect sub-intervals that were split between
// instructions
void XLS::resolveSplits() {
  if (dumpIREnabled()) dumpIntervals();
  for (auto i1 : m_intervals) {
    if (!i1) continue;
    if (i1->spill.spilled()) insertSpill(i1);
    for (auto& i2 : i1->children) {
      auto pos1 = i1->end();
      auto pos2 = i2.start();
      if (!i2.info.spilled() && pos1 == pos2 && i1->info != i2.info) {
        auto inst = m_insts[pos2 / 2];
        auto block = inst->block();
        if (inst != skipShuffle(block)) {
          assert(pos2 % 2 == 0);
          insertCopy(block, block->iteratorTo(inst), m_between[inst], i1->tmp,
                     i1->info, i2.info);
        }
      }
      i1 = &i2;
    }
  }
}

// Insert copies on control-flow edges, and turn Jmps into Shuffles
void XLS::resolveEdges() {
  const PhysLoc invalid_loc;
  for (auto succ : m_blocks) {
    auto& inst2 = *skipShuffle(succ);
    auto pos2 = m_posns[inst2]; // pos of first inst in succ.
    succ->forEachPred([&](Block* pred) {
      auto it1 = pred->end();
      while ((--it1)->op() == Shuffle) {}
      auto& inst1 = *it1;
      auto pos1 = m_posns[inst1]; // pos of last inst in pred
      if (inst1.op() == Jmp) {
        // resolve Jmp->DefLabel copies
        for (unsigned i = 0, n = inst1.numSrcs(); i < n; ++i) {
          auto i1 = m_intervals[inst1.src(i)];
          auto i2 = m_intervals[inst2.dst(i)];
          if (i1) i1 = i1->childAt(pos1);
          insertCopy(pred, it1, m_after[pred], inst1.src(i),
                     i1 ? i1->info : invalid_loc,
                     i2 ? i2->info : invalid_loc);
        }
      }
      m_liveIn[succ].forEach([&](uint32_t id) {
        auto ivl = m_intervals[id];
        auto i1 = ivl->childAt(pos1 + 1);
        auto i2 = ivl->childAt(pos2);
        assert(i1 && i2);
        if (i2->info.spilled()) return; // we did spill store after def.
        if (pred->taken() && pred->next()) {
          // insert copy at start of succesor
          insertCopy(succ, succ->skipHeader(), m_before[succ],
                     i1->tmp, i1->info, i2->info);
        } else {
          // insert copy at end of predecessor
          auto pos = inst1.op() == Jmp ? it1 : pred->end();
          insertCopy(pred, pos, m_after[pred], i1->tmp, i1->info, i2->info);
        }
      });
    });
  }
  assert(checkEdgeShuffles());
  if (dumpIREnabled()) {
    print(" after resolving intervals ");
  }
}

/*
 * Extended Linear Scan is based on Wimmer & Franz "Linear Scan Register
 * Allocation on SSA Form".
 *
 * 1. Sort blocks such that all predecessors of B come before B, except
 * loop-edge predecessors.  Because the input IR is in SSA form, this also
 * implies the definition of each SSATmp comes before all uses.
 *
 * 2. Assign an even numbered position to every instruction.  Uses where
 * inputs are read occur on even positions, and definition where outputs
 * are written occur on odd positions.  We can only insert new instructions
 * after odd positions and before even positions.  Points after even positions
 * and before odd positions are "inside" existing instructions.
 *
 * 3. Create one interval I for each SSATmp T that requires register allocation,
 * by iterating blocks and instructions in reverse order, computing live
 * SSATmps as we go.  Each interval consists of a sorted list of disjoint,
 * live ranges covering the positions where T must be in a register or
 * spill location.  SSATmps that are constants or have forced registers
 * (e.g. VmSp) are skipped.  Because of SSA form, the start position of each
 * interval dominates every live range and use position in the interval.
 *
 * 4. Process intervals in order of start position, maintaining the set of
 * active (live) and inactive (not live, but with live ranges that start
 * after the current interval).  When choosing a register, prefer the one
 * available furthest into the future.  If necessary, split the current
 * interval so the first part gets a register, and enqueue the rest.
 * When no registers are available, choose either the current interval or
 * another one to spill, trying to free up the longest-available register.
 *
 * Split positions must be after an interval's start position, and on or before
 * the chosen split point.  We're free try to choose a good position inbetween,
 * for example block boundaries and cold blocks.
 *
 * 5. Once intervals have been walked and split, every interval has an assigned
 * operand (register or spill location) for all positions where it's alive.
 * visit every instruction and store the position of its sources and
 * destinations in the RegAllocInfo structure that we pass onto CodeGenerator.
 *
 * 6. Splitting creates sub-intervals that are assigned to different registers
 * or spill locations, so we must insert resolving copies at the split
 * positions between intervals that were split in a block, and copies on
 * control-flow edges connecting different sub-intervals.  When more than one
 * copy occurs in a position, they are parallel-copies (all sources read before
 * any dest is written).
 *
 * If any sub-interval was spilled, we a single store is generated after the
 * definition point.  SSA form ensures this position dominates all uses, so
 * therefore it dominates all reloads.
 *
 * Copies for Jmp->DefLabel edges are also converted to Shuffles, which are
 * combined with any other resolving copies on the same edges.
 */

void XLS::allocate() {
  prepareBlocks();
  computePositions();
  buildIntervals();
  walkIntervals();
  assignLocations();
  resolveSplits();
  resolveEdges();
  assert(checkRegisters(m_unit, m_regs));
}

//////////////////////////////////////////////////////////////////////////////

void XLS::dumpIntervals() {
  HPHP::Trace::traceRelease("Splits %d Spills %d\n", m_numSplits, m_nextSpill);
  for (auto ivl : m_intervals) {
    if (!ivl) continue;
    HPHP::Trace::traceRelease("i%-2d %s\n", ivl->tmp->id(),
                              ivl->toString().c_str());
    if (!ivl->children.empty()) {
      for (auto& c : ivl->children) {
        HPHP::Trace::traceRelease("    %s\n", c.toString().c_str());
      }
    }
  }
}

template<class F>
void forEachInterval(Intervals& intervals, F f) {
  for (auto ivl : intervals) {
    if (ivl) f(ivl);
  }
}

enum Mode { Light, Heavy };
template<class F>
const char* draw(unsigned pos, Mode m, F f) {
                               // Light     Heavy
  static const char* top[]    = { "\u2575", "\u2579" };
  static const char* bottom[] = { "\u2577", "\u257B" };
  static const char* both[]   = { "\u2502", "\u2503" };
  static const char* empty[]  = { " ", " " };
  auto s = f(pos);
  auto d = f(pos+1);
  return ( s && !d) ? top[m] :
         ( s &&  d) ? both[m] :
         (!s &&  d) ? bottom[m] :
         empty[m];
}

void XLS::print(const char* caption) {
  std::ostringstream str;
  str << "Intervals " << caption << "\n";
  forEachInterval(m_intervals, [&] (Interval* ivl) {
    str << folly::format(" {: <2}", ivl->tmp->id());
  });
  str << "\n";
  for (auto& b : m_blocks) {
    for (auto& i : *b) {
      auto pos = m_posns[i];
      if (!pos) {
        forEachInterval(m_intervals, [&](Interval* ivl) {
          str << "   ";
        });
      } else {
        forEachInterval(m_intervals, [&] (Interval* ivl) {
          str << " ";
          str << draw(pos, Light, [&](unsigned p) {
            auto c = ivl->childAt(p);
            return c && c->covers(p);
          });
          str << draw(pos, Heavy, [&](unsigned p) {
            auto c = ivl->childAt(p);
            return c && c->usedAt(p);
          });
        });
      }
      if (&i == &b->front()) {
        str << folly::format(" B{: <2}", b->id());
      } else {
        str << "    ";
      }
      if (i.isNative()) {
        str << folly::format(" {: <3}-", pos);
      } else {
        str << folly::format(" {: <3} ", pos);
      }
      JIT::printOpcode(str, &i, nullptr);
      JIT::printSrcs(str, &i, &m_regs, nullptr);
      if (i.numDsts()) {
        str << " => ";
        JIT::printDsts(str, &i, &m_regs, nullptr);
      }
      if (&i == &b->back()) {
        if (auto next = b->next()) {
          str << folly::format(" next->B{}", next->id());
        }
        if (auto taken = b->taken()) {
          str << folly::format(" taken->B{}", taken->id());
        }
      }
      str << "\n";
    }
  }
  HPHP::Trace::traceRelease("%s\n", str.str().c_str());
}

std::string Interval::toString() {
  std::ostringstream out;
  auto delim = "";
  out << info << " [";
  for (auto r : ranges) {
    out << delim << folly::format("{}-{}", r.start, r.end);
    delim = ",";
  }
  out << ") {";
  delim = "";
  for (auto u : uses) {
    out << delim << u.pos;
    delim = ",";
  }
  out << "}";
  return out.str();
}

//////////////////////////////////////////////////////////////////////////////

// During resolveEdges, we can insert shuffles "on edges", which comes down
// to either before a successor or after a predecessor.  We must never have
// a shuffle in both places, because that would be two sequential shuffles
// along one edge.
bool XLS::checkEdgeShuffles() {
  for (auto b : m_blocks) {
    DEBUG_ONLY auto numSucc = (!!b->next()) + (!!b->taken());
    DEBUG_ONLY auto numPred = b->numPreds();
    if (m_after[b]) {
      assert(numSucc == 1);
      assert(!m_before[b->next() ? b->next() : b->taken()]);
    }
    if (m_before[b]) {
      assert(numPred == 1);
      assert(!m_after[b->preds().front().inst()->block()]);
    }
    if (numSucc != 1) assert(!m_after[b]);
    if (numPred != 1) assert(!m_before[b]);
  }
  return true;
}

// Check validity of this interval
// 1. split-children cannot have more children, nor can the parent
//    be a child of another interval.
// 2. live ranges must be sorted and disjoint
// 3. every use position must be inside a live range
// 4. parent and children must all be sorted and disjoint
bool Interval::checkInvariants() const {
  assert(!parent || children.empty()); // 1: no crazy nesting
  DEBUG_ONLY auto pos = 0;
  for (auto r : ranges) {
    assert(r.start <= r.end); // valid range
    assert(r.start >= pos); // 2: ranges are sorted
    pos = r.end + 1; // 2: ranges are disjoint with no empty holes
  }
  for (DEBUG_ONLY auto u : uses) {
    assert(covers(u.pos)); // 3: every use must be in a live range
  }
  if (!parent && !children.empty()) {
    // this is the parent, and there are children.
    DEBUG_ONLY auto pos = end();
    for (auto& c : children) {
      c.checkInvariants();
      assert(!c.empty());
      assert(pos <= c.start()); // 4: children are sorted
      pos = c.end(); // 4: and disjoint
    }
  }
  return true;
}

// check that each block comes after all its predecessors (XXX won't be
// true once we have loops).
bool checkBlockOrder(IRUnit& unit, BlockList& blocks) {
  StateVector<Block, bool> seen(unit, false);
  for (auto b : blocks) {
    b->forEachPred([&](Block* p) {
      assert(seen[p]);
    });
    seen[b] = true;
  }
  return true;
}
}
//////////////////////////////////////////////////////////////////////////////

namespace {
const Abi x64_abi {
  X64::kAllRegs - X64::kXMMRegs, // general purpose
  X64::kAllRegs & X64::kXMMRegs, // fp/simd
  X64::kCalleeSaved
};

const Abi arm_abi {
  // For now this is the same as x64, since we're pretending arm
  // has the same register conventions as x64.
  ARM::kCallerSaved | ARM::kCalleeSaved,
  RegSet(), // fp/simd
  ARM::kCalleeSaved
};
}

// This is the public entry-point
RegAllocInfo allocateRegs(IRUnit& unit) {
  RegAllocInfo regs(unit);
  XLS xls(unit, regs, arch() == Arch::ARM ? arm_abi : x64_abi);
  xls.allocate();
  if (dumpIREnabled()) {
    dumpTrace(kRegAllocLevel, unit, " after extended alloc ", &regs,
              nullptr, nullptr, nullptr);
  }
  return regs;
}

}}
