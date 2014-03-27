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

#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/id-set.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arch.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"

#include <unordered_set>
#include <algorithm>
#include <utility>

// TODO
//  - #3098109 dests of branch instructions start in next block
//  - #3098509 streamline code, vectors vs linked lists, etc
//  - #3098661 generate spill stats so we can compare side/by/side
//  - #3098678 EvalHHIREnablePreColoring
//  - #3098685 EvalHHIREnableCoalescing, by using hints
//  - #3409409 Enable use of SIMD at least for doubles, for packed_tv
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

// An Interval stores the lifetime of an SSATmp as a sorted list of disjoint
// ranges, and a sorted list of use positions.  If this interval was split,
// then the first interval is deemed "parent" and the rest are "children",
// and they're all connected as a singly linked list sorted by start.
//
// Every use position must be inside one of the ranges, or exactly at the
// end of the last range.  Allowing a use exactly at the end facilitates
// lifetime splitting when the use is a call argument that clobbers the
// argument registers; we need to split the lifetime exactly at the call
// position, which is exactly where the use is.
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
  unsigned firstUseAfter(unsigned pos) const;
  unsigned lastUseBefore(unsigned pos) const;
  unsigned firstUse() const;
  // mutators
  void add(LiveRange r);
  void addUse(unsigned pos) { uses.push_back(Use{pos}); }
  Interval* split(unsigned pos, bool keep_uses = false);
  // register/spill assignment
  RegPair regs() const;
  bool handled() const { return loc.hasReg(0) || loc.spilled(); }
  // debugging
  std::string toString();
  bool checkInvariants() const;
  uint32_t id() const { return tmp->id(); }
public:
  bool blocked { false }; // cannot be spilled
  bool scratch { false }; // used as scratch or arg, cannot be spilled
  uint8_t need { 0 }; // number of required registers (exactly 1 or 2)
  RegSet allow;
  RegSet prefer;
  SSATmp* tmp { nullptr };
  Interval* parent { nullptr }; // if this is a split-off child
  Interval* next { nullptr }; // next split child or nullptr
  smart::vector<LiveRange> ranges;
  smart::vector<Use> uses;
  PhysLoc loc;  // current location assigned to this interval
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
  void allocOne(Interval* current);
  void allocBlocked(Interval* current);
  void spill(Interval*);
  void spillAfter(Interval* ivl, unsigned pos);
  void spillOthers(Interval* current, RegPair r);
  void assignReg(Interval*, RegPair r);
  void assignSpill(Interval*);
  void update(unsigned pos);
  void insertCopy(Block* b, Block::iterator, IRInstruction*& shuffle,
                  SSATmp* src, const PhysLoc& rs, const PhysLoc& rd);
  void insertSpill(Interval* ivl);
  void resolveFlow(Interval* ivl, Block* pred, Block* succ,
                   unsigned pos1, unsigned pos2);
  // debugging
  void print(const char* caption);
  void dumpIntervals();
private:
  struct Compare { bool operator()(const Interval*, const Interval*); };
private:
  Intervals m_intervals; // parent intervals indexed by ssatmp
  unsigned m_nextSpill { 0 };
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
  StateVector<Block,std::pair<IRInstruction*,IRInstruction*>> m_edgeCopies;
  unsigned m_frontier { 0 }; // debug_only to detect backtracking
};

const uint32_t kMaxPos = UINT32_MAX; // "infinity" use position

// Keep track of a future use or conflict position for each register.
// Initially all usable registers have kMaxPos
struct RegPositions {
  explicit RegPositions();
  unsigned find1(RegSet allow, RegPair& regs) const;
  unsigned find2(RegSet allow, RegPair& regs) const;
  unsigned find(Interval* ivl, RegSet allow, RegPair& regs) const;
  unsigned getPos(Interval*, RegPair regs) const;
  unsigned setPos(Interval*, unsigned pos);
private:
  PhysReg::Map<unsigned> posns;
};

bool checkBlockOrder(IRUnit& unit, BlockList& blocks);

//////////////////////////////////////////////////////////////////////////////

// returns true if this range contains r
bool LiveRange::contains(LiveRange r) const {
  return r.start >= start && r.end <= end;
}

//////////////////////////////////////////////////////////////////////////////

bool isDefConst(const Interval* ivl) {
  return ivl->tmp->inst()->is(DefConst);
}

Interval::Interval(Interval* parent)
  : need(parent->need)
  , allow(parent->allow)
  , prefer(parent->prefer)
  , tmp(parent->tmp)
  , parent(parent)
{}

// Add r to this interval, merging r with any existing overlapping ranges
void Interval::add(LiveRange r) {
  assert(r.start < r.end); // not empty
  while (!ranges.empty() && r.contains(ranges.back())) {
    ranges.pop_back();
  }
  if (ranges.empty()) {
    return ranges.push_back(r);
  }
  auto& first = ranges.back();
  if (first.contains(r)) return;
  if (r.end >= first.start) {
    first.start = r.start;
  } else {
    ranges.push_back(r);
  }
}

// Return true if one of the ranges in this interval includes pos
bool Interval::covers(unsigned pos) const {
  if (pos < start() || pos >= end()) return false;
  for (auto r : ranges) {
    if (pos < r.start) return false;
    if (pos < r.end) return true;
  }
  return false;
}

// Return true if there is a use position at pos
bool Interval::usedAt(unsigned pos) const {
  if (pos < start() || pos > end()) return false;
  for (auto& u : uses) if (u.pos == pos) return true;
  return false;
}

// Return the interval which has a use position at pos
Interval* Interval::childAt(unsigned pos) {
  assert(!isChild());
  for (auto ivl = this; ivl; ivl = ivl->next) {
    if (pos < ivl->start()) return nullptr;
    if (ivl->usedAt(pos)) return ivl;
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
      if (++r1 == e1) return kMaxPos;
    } else {
      if (r1->start < r2->end) return r1->start;
      if (++r2 == e2) return kMaxPos;
    }
  }
  return kMaxPos;
}

// Split this interval at pos and return the rest.  Pos must be
// a location that ensures both shorter intervals are nonempty.
// Pos must also be odd, indicating a position between instructions.
// If keep_uses is set, uses exactly at the end of the first interval
// will stay with the first part.
Interval* Interval::split(unsigned pos, bool keep_uses) {
  assert(pos > start() && pos < end());
  auto leader = this->leader();
  Interval* child = smart_new<Interval>(leader);
  child->next = next;
  next = child;
  // advance r1 to the first range we want in child; maybe split a range.
  auto r1 = ranges.begin(), r2 = ranges.end();
  while (r1->end <= pos) r1++;
  if (pos > r1->start) { // split r at pos
    child->ranges.push_back(LiveRange(pos, r1->end));
    r1->end = pos;
    r1++;
  }
  child->ranges.insert(child->ranges.end(), r1, r2);
  ranges.erase(r1, r2);
  // advance u1 to the first use position in child, then copy u1..end to child.
  auto u1 = uses.begin(), u2 = uses.end();
  if (keep_uses) {
    while (u1 != u2 && u1->pos <= end()) u1++;
  } else {
    while (u1 != u2 && u1->pos < child->start()) u1++;
  }
  child->uses.insert(child->uses.end(), u1, u2);
  uses.erase(u1, u2);
  assert(checkInvariants() && child->checkInvariants());
  return child;
}

// Return the position of the next use of this interval after (or equal to)
// pos.  If there are no more uses after pos, return MAX.
unsigned Interval::firstUseAfter(unsigned pos) const {
  for (auto& u : uses) {
    if (pos <= u.pos) return u.pos;
  }
  return kMaxPos;
}

unsigned Interval::lastUseBefore(unsigned pos) const {
  auto prev = 0;
  for (auto& u : uses) {
    if (u.pos > pos) return prev;
    prev = u.pos;
  }
  return prev;
}

// return the position of the first use that requires a register,
// or kMaxPos if no remaining uses need registers.
unsigned Interval::firstUse() const {
  return uses.empty() ? kMaxPos : uses.front().pos;
}

// Return the register(s) assigned to this interval as a pair.
RegPair Interval::regs() const {
  return RegPair(loc.reg(0), loc.reg(1));
}

//////////////////////////////////////////////////////////////////////////////

XLS::XLS(IRUnit& unit, RegAllocInfo& regs, const Abi& abi)
  : m_intervals(unit, nullptr)
  , m_unit(unit)
  , m_regs(regs)
  , m_abi(abi)
  , m_posns(unit, 0)
  , m_liveIn(unit, LiveSet())
  , m_edgeCopies(unit, { nullptr, nullptr }) {
  auto all = abi.all();
  for (auto r : m_blocked) {
    m_blocked[r].blocked = true;
    m_blocked[r].need = 1;
    m_blocked[r].loc.setReg(r, 0);
    if (!all.contains(r)) {
      // r is never available
      m_blocked[r].add(LiveRange(0, kMaxPos));
    }
    m_scratch[r].scratch = true;
    m_scratch[r].need = 1;
    m_scratch[r].loc.setReg(r, 0);
  }
}

XLS::~XLS() {
  for (auto ivl : m_intervals) {
    for (Interval* next; ivl; ivl = next) {
      next = ivl->next;
      smart_delete(ivl);
    }
  }
}

// Split critical edges, remove dead predecessor edges, and put blocks
// in a sutiable order.
void XLS::prepareBlocks() {
  splitCriticalEdges(m_unit);
  m_blocks = rpoSortCfg(m_unit);
}

// compute the position number for each instruction.  Instructions are
// assigned even positions; shuffles may later occupy inbetween positions.
void XLS::computePositions() {
  m_insts.resize(2 * (m_unit.numInsts() + m_unit.numBlocks()));
  unsigned pos = 0;
  for (auto b : m_blocks) {
    auto& front = b->front();
    if (front.numSrcs() > 0) {
      // ensure no uses at block-start so livein ranges are nonempty
      b->prepend(m_unit.gen(DefLabel, front.marker()));
    }
    for (auto& inst : *b) {
      m_insts[pos] = &inst;
      m_posns[inst] = pos;
      pos += 2;
    }
  }
}

// Return the reg mask that corresponds to a constraint, and deal with
// the EvalHHIRAllocSIMDRegs option.  Only disable SIMD for constraints
// that have some other option. (Don't create empty allow sets).
// The check is intentionally here instead of in src/dstConstraint(), so
// the latter can directly reflect the instruction-selection source code
// in CodeGenerator.
RegSet constrainedRegs(Constraint c, const Abi& abi) {
  auto regs = RegSet();
  if (c & Constraint::GP) regs |= abi.gp;
  if (c & Constraint::SIMD) {
    if (regs.empty() || RuntimeOption::EvalHHIRAllocSIMDRegs) {
      regs |= abi.simd;
    }
  }
  return regs;
}

// Reduce the allow and prefer sets according to this particular use
void srcConstraints(Interval* ivl, const Abi& abi, Constraint constraint) {
  auto allow = constrainedRegs(constraint, abi);
  ivl->allow &= allow;
  ivl->prefer &= allow;
  if (!(ivl->allow & abi.simd).empty()) {
    // we allow gp and simd, so prefer just simd
    ivl->prefer &= abi.simd;
  }
}

// Reduce the allow and prefer constraints based on this definition
void dstConstraints(Interval* ivl, const Abi& abi, Constraint constraint) {
  auto allow = constrainedRegs(constraint, abi);
  ivl->allow &= allow;
  ivl->prefer &= allow;
  if (!(ivl->allow & abi.simd).empty()) {
    // if we allow gp and simd, then prefer just simd
    ivl->prefer &= abi.simd;
  }
  if (ivl->tmp->numWords() == 2 && !(ivl->allow & ivl->prefer).empty()) {
    // we prefer simd, but allow gp and simd, so restrict allow to just
    // simd to avoid (simd)<->(gpr,gpr) shuffles.
    ivl->allow = ivl->prefer;
  }
}

// build intervals in one pass by walking the block list backwards.
// no special handling is needed for goto/label (phi) instructions because
// they use/define tmps in the expected locations.
void XLS::buildIntervals() {
  assert(checkBlockOrder(m_unit, m_blocks));
  unsigned min_need = 0;
  for (auto blockIt = m_blocks.end(); blockIt != m_blocks.begin();) {
    auto block = *--blockIt;
    // compute initial live set from liveIn[succsessors]
    LiveSet live;
    if (auto taken = block->taken()) live |= m_liveIn[taken];
    if (auto next  = block->next())  live |= m_liveIn[next];
    // initialize live range for each live tmp to whole block
    auto blockStart = m_posns[block->front()];
    auto blockEnd = m_posns[block->back()] + 2;
    live.forEach([&](uint32_t id) {
      m_intervals[id]->add(LiveRange(blockStart, blockEnd));
    });
    for (auto instIt = block->end(); instIt != block->begin();) {
      auto& inst = *--instIt;
      auto pos = m_posns[inst];
      unsigned dst_need = 0;
      auto& inst_regs = m_regs[inst];
      for (unsigned i = 0, n = inst.numDsts(); i < n; ++i) {
        auto d = inst.dst(i);
        auto dest = m_intervals[d];
        auto constraint = dstConstraint(inst, i);
        if (!live[d]) {
          // dest is not live; give it a register anyway.
          if (d->numWords() == 0) continue;
          if (constraint.reg() != InvalidReg) {
            inst_regs.dst(i).setReg(constraint.reg(), 0);
            continue;
          }
          if (constraint & Constraint::VOID) {
            continue; // unused dest will be InvalidReg
          }
          m_intervals[d] = dest = smart_new<Interval>();
          dest->add(LiveRange(pos, pos + 1));
          dest->tmp = d;
          dest->need = d->numWords();
          dest->allow = dest->prefer = m_abi.all();
        } else {
          // adjust start pos for live intervals defined by this instruction
          dest->ranges.back().start = pos;
          live.erase(d);
        }
        dst_need += dest->need;
        dest->addUse(pos);
        dstConstraints(dest, m_abi, constraint);
      }
      min_need = std::max(min_need, dst_need);
      if (inst.isNative()) {
        if (RuntimeOption::EvalHHIREnableCalleeSavedOpt) {
          auto scratch = m_abi.gp - m_abi.saved;
          scratch.forEach([&](PhysReg r) {
            m_scratch[r].add(LiveRange(pos, pos + 1));
          });
        }
      }
      if (inst.is(Call, CallArray, ContEnter)) {
        // block all registers at php callsites.
        m_abi.all().forEach([&](PhysReg r) {
          m_blocked[r].add(LiveRange(pos, pos + 1));
        });
      }
      // add live ranges for tmps used by this inst
      unsigned src_need = 0;
      for (unsigned i = 0, n = inst.numSrcs(); i < n; ++i) {
        auto s = inst.src(i);
        auto constraint = srcConstraint(inst, i);
        if (constraint == Constraint::IMM) continue;
        if (s->isConst() && (constraint & Constraint::IMM)) continue;
        auto need = s->numWords();
        if (need == 0) continue; //XXX problematic for InitNull|UninitNull
        if (constraint.reg() != InvalidReg) {
          inst_regs.src(i).setReg(constraint.reg(), 0);
          continue;
        }
        auto src = m_intervals[s];
        if (!src) m_intervals[s] = src = smart_new<Interval>();
        src->add(LiveRange(blockStart, pos));
        src->addUse(pos);
        if (!src->tmp) {
          src->tmp = s;
          src->need = need;
          src->allow = src->prefer = m_abi.all();
        }
        srcConstraints(src, m_abi, constraint);
        src_need += src->need;
        live.add(s);
      }
      min_need = std::max(min_need, src_need);
    }
    m_liveIn[block] = live;
  }
  // Implement stress mode by blocking NumFreeRegs more than minimum needed.
  min_need += RuntimeOption::EvalHHIRNumFreeRegs;
  assert(min_need >= RuntimeOption::EvalHHIRNumFreeRegs); // no wraparound.
  for (auto r : m_blocked) {
    auto& blocked = m_blocked[r];
    if (!blocked.empty() && blocked.start() == 0) continue;
    // r is at least partially available
    if (min_need > 0) {
      min_need--;
      std::reverse(blocked.ranges.begin(), blocked.ranges.end());
    } else {
      blocked.add(LiveRange(0, kMaxPos));
      assert(blocked.ranges.size() == 1);
    }
  }
  for (auto r : m_scratch) {
    auto& scratch = m_scratch[r];
    std::reverse(scratch.ranges.begin(), scratch.ranges.end());
  }
  // We built the use list of each interval by appending.  Now reverse those
  // lists so they are in forwards order.
  for (auto ivl : m_intervals) {
    if (!ivl) continue;
    std::reverse(ivl->uses.begin(), ivl->uses.end());
    std::reverse(ivl->ranges.begin(), ivl->ranges.end());
  }
  if (dumpIREnabled(kRegAllocLevel)) {
    print("after building intervals");
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
  assert(ivl->start() >= m_frontier);
  m_pending.push(ivl);
}

// Assign the next available spill slot to interval
void XLS::assignSpill(Interval* ivl) {
  assert(!ivl->blocked && !ivl->scratch && ivl->isChild());
  assert(ivl->need == 1 || ivl->need == 2);
  assert(ivl->firstUse() > ivl->end());
  auto leader = ivl->leader();
  if (!leader->spill.spilled()) {
    if (ivl->need == 1) {
      leader->spill.setSlot(0, m_nextSpill++);
    } else {
      if (!PhysLoc::isAligned(m_nextSpill)) m_nextSpill++;
      leader->spill.setSlot(0, m_nextSpill++);
      leader->spill.setSlot(1, m_nextSpill++);
    }
    if (m_nextSpill > NumPreAllocatedSpillLocs) {
      PUNT(LinearScan_TooManySpills);
    }
  }
  ivl->loc = leader->spill;
}

// Assign one or both of the registers in r to this interval.
void XLS::assignReg(Interval* ivl, RegPair r) {
  assert(!ivl->blocked && !ivl->scratch);
  auto r0 = PhysReg(r.first);
  auto r1 = PhysReg(r.second);
  if (ivl->need == 1) {
    ivl->loc.setReg(r0, 0);
  } else {
    if (r0.isSIMD()) {
      ivl->loc.setRegFullSIMD(r0);
    } else if (r1.isSIMD()) {
      ivl->loc.setRegFullSIMD(r1);
    } else {
      assert(r0 != r1);
      ivl->loc.setReg(r0, 0);
      ivl->loc.setReg(r1, 1);
    }
  }
  // now ivl has a register, so put it in active list.
  m_active.push_back(ivl);
}

// initialize the positions array with maximal use positions.
RegPositions::RegPositions() {
  for (auto r : posns) posns[r] = kMaxPos;
}

// Find the register used furthest in the future, but only consider registers
// in the given set.  Also return that register's position.
unsigned
RegPositions::find1(RegSet allow, RegPair& regs) const {
  unsigned max1 = 0;
  PhysReg r1 = *posns.begin();
  allow.forEach([&](PhysReg r) {
    if (posns[r] > max1) {
      r1 = r;
      max1 = posns[r];
    }
  });
  regs = { r1, InvalidReg };
  return max1;
}

// Find the two registers used furthest in the future, but only
// consider registers in the given set.  Return the registers, and
// their minimum position
unsigned
RegPositions::find2(RegSet allow, RegPair& regs) const {
  unsigned max1 = 0, max2 = 0;
  PhysReg r1 = *posns.begin(), r2 = *posns.begin();
  allow.forEach([&](PhysReg r) {
    if (posns[r] > max2) {
      if (posns[r] > max1) {
        r2 = r1; max2 = max1;
        r1 = r; max1 = posns[r];
      } else {
        r2 = r; max2 = posns[r];
      }
    }
  });
  assert(max1 >= max2);
  regs = { r1, r2 };
  return r1 != r2 ? max2 : 0;
}

unsigned
RegPositions::find(Interval* ivl, RegSet allow, RegPair& regs) const {
  return ivl->need == 1 ? find1(allow, regs) : find2(allow, regs);
}

unsigned RegPositions::getPos(Interval* ivl, RegPair regs) const {
  return ivl->need == 1 ? posns[regs.first] :
         regs.second != regs.first ? posns[regs.second] : 0;
}

// Update the position associated with the registers assigned to ivl,
// to the minimum of pos and the existing position.
unsigned RegPositions::setPos(Interval* ivl, unsigned pos) {
  assert(ivl->loc.numAllocated() >= 1);
  auto r0 = ivl->loc.reg(0);
  auto minpos = posns[r0] = std::min(pos, posns[r0]);
  if (ivl->loc.numAllocated() == 2) {
    auto r1 = ivl->loc.reg(1);
    posns[r1] = std::min(pos, posns[r1]);
    minpos = std::min(minpos, posns[r1]);
  }
  return minpos;
}

// return the closest valid split position on or before pos.
unsigned nearestSplitBefore(unsigned pos) {
  return pos == 0 || pos % 2 == 1 ? pos : pos - 1;
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
  if (current->isChild() && current->start() % 2 == 0) {
    // TODO: #3098697 only spill if it's not on a block boundary.
    assert(current->firstUse() > current->start());
    return spill(current);
  }
  RegPositions until1; // free-until, ignoring scratch
  RegPositions until2; // free-until, including scratch
  for (auto ivl : m_active) {
    if (ivl->scratch) {
      until2.setPos(ivl, 0);
    } else {
      until1.setPos(ivl, 0);
      until2.setPos(ivl, 0);
    }
  }
  for (auto ivl : m_inactive) {
    auto intersectPos = current->nextIntersect(ivl);
    if (ivl->scratch) {
      until2.setPos(ivl, intersectPos);
    } else {
      auto pos = until1.setPos(ivl, intersectPos);
      until2.setPos(ivl, pos);
    }
  }
  // Try to get a preferred-non scratch register first
  RegPair r;
  auto until_pos = until2.find(current, current->prefer, r);
  if (until_pos >= current->end()) {
    // got one for all of current
    return assignReg(current, r);
  }
  // find the register(s) that are free for the longest time.
  until_pos = until2.find(current, current->allow, r);
  if (until_pos >= current->end()) {
    // got register for all of current
    return assignReg(current, r);
  }
  // try prefer set again but ignore scratch register conflicts
  until_pos = until1.find(current, current->prefer, r);
  if (until_pos >= current->end()) {
    // got one for all of current
    return assignReg(current, r);
  }
  // try allow set again but ignore scratch register conflicts
  until_pos = until1.find(current, current->allow, r);
  if (until_pos >= current->end()) {
    // got register for all of current
    return assignReg(current, r);
  }
  if (until_pos <= current->start()) {
    // nothing free for any of current
    return allocBlocked(current);
  }
  // register is free for part of current; assign register and enqueue
  // the remaining part.
  auto prev_use = current->lastUseBefore(until_pos);
  auto min_split = std::max(prev_use, current->start() + 1);
  auto max_split = until_pos;
  assert(min_split <= max_split);
  auto split_pos = std::max(min_split, max_split); // todo: find good spot
  // got register for first part of current
  enqueue(current->split(split_pos, true));
  assignReg(current, r);
}

// When all registers are in use, find a good interval to split and spill,
// which could be the current interval.  When an interval is split and the
// second part is spilled, possibly split the second part again before the
// next use-pos that requires a register, and enqueue the third part.
void XLS::allocBlocked(Interval* current) {
  RegPositions used;
  RegPositions blocked;
  auto const cur_start = current->start();
  // compute next use of active registers, so we can pick the furthest one
  for (auto ivl : m_active) {
    if (ivl->blocked) {
      blocked.setPos(ivl, 0);
      used.setPos(ivl, 0);
    } else {
      used.setPos(ivl, ivl->firstUseAfter(cur_start));
    }
  }
  // compute next intersection/use of inactive regs to find whats free longest
  for (auto ivl : m_inactive) {
    auto intersectPos = current->nextIntersect(ivl);
    if (intersectPos == kMaxPos) continue;
    if (ivl->blocked) {
      auto pos = blocked.setPos(ivl, intersectPos);
      used.setPos(ivl, pos);
    } else {
      used.setPos(ivl, ivl->firstUseAfter(cur_start));
    }
  }
  // choose the best victim register(s) to spill
  RegPair r;
  auto used_pos = used.find(current, current->allow, r);
  if (used_pos < current->firstUse()) {
    // all other intervals are used before current's first register-use
    return spill(current);
  }
  auto block_pos = blocked.getPos(current, r);
  if (block_pos < current->end()) {
    auto prev_use = current->lastUseBefore(block_pos);
    auto min_split = std::max(prev_use, cur_start + 1);
    auto max_split = block_pos;
    assert(cur_start < min_split && min_split <= max_split);
    auto split_pos = std::max(min_split, max_split);
    enqueue(current->split(split_pos, true));
  }
  spillOthers(current, r);
  assignReg(current, r);
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

// split ivl at pos and spill the second part.  If pos is too close
// to ivl->start(), spill all of ivl.
void XLS::spillAfter(Interval* ivl, unsigned pos) {
  auto split_pos = nearestSplitBefore(pos);
  auto tail = split_pos <= ivl->start() ? ivl : ivl->split(split_pos);
  spill(tail);
}

// Spill ivl from its start until its first register use.  If there
// is no use, spill the entire interval.  Otherwise split the
// interval just before the use, and enqueue the second part.
void XLS::spill(Interval* ivl) {
  unsigned first_use = ivl->firstUse();
  if (first_use <= ivl->end()) {
    auto split_pos = nearestSplitBefore(first_use);
    if (split_pos <= ivl->start()) {
      PUNT(RegSpill); // cannot split before first_use
    }
    enqueue(ivl->split(split_pos));
  }
  assert(ivl->uses.size() == 0);
  if (!isDefConst(ivl)) assignSpill(ivl);
}

// Split and spill other intervals that conflict with current for
// register r, at current->start().  If necessary, split the victims
// again before their first use position that requires a register.
void XLS::spillOthers(Interval* current, RegPair r) {
  auto cur_start = current->start();
  for (auto i = m_active.begin(); i != m_active.end();) {
    auto other = *i;
    if (other->scratch || !conflict(r, other->regs())) {
      i++; continue;
    }
    i = m_active.erase(i);
    spillAfter(other, cur_start);
  }
  for (auto i = m_inactive.begin(); i != m_inactive.end();) {
    auto other = *i;
    if (other->scratch || !conflict(r, other->regs())) {
      i++; continue;
    }
    auto intersect = current->nextIntersect(other);
    if (intersect >= current->end()) {
      i++; continue;
    }
    i = m_inactive.erase(i);
    spillAfter(other, cur_start);
  }
}

// Update active/inactive sets based on pos
void XLS::update(unsigned pos) {
  m_frontier = pos;
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
    if (!ivl) continue;
    if (isDefConst(ivl)) {
      spill(ivl);
    } else {
      enqueue(ivl);
    }
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
    assert(isDefConst(current) ||
           (current->handled() && current->loc.numWords() == current->need));
  }
  if (dumpIREnabled(kRegAllocLevel)) {
    dumpIntervals();
    print("after walking intervals");
  }
}

// Assign PhysLocs (registers or spill slots) to every src and dst ssatmp
void XLS::assignLocations() {
  for (auto b : m_blocks) {
    for (auto& inst : *b) {
      auto pos = m_posns[inst];
      auto& inst_regs = m_regs[inst];
      for (unsigned i = 0, n = inst.numSrcs(); i < n; i++) {
        auto ivl = m_intervals[inst.src(i)];
        if (ivl) {
          ivl = ivl->childAt(pos);
          if (ivl) inst_regs.src(i) = ivl->loc;
        }
      }
      for (unsigned i = 0, n = inst.numDsts(); i < n; i++) {
        auto ivl = m_intervals[inst.dst(i)];
        if (ivl) inst_regs.dst(i) = ivl->loc;
      }
    }
  }
}

// Add a copy of tmp from rs to rd to the Shuffle instruction shuffle,
// if it exists.  Otherwise, create a new one and insert it at pos in block b.
void XLS::insertCopy(Block* b, Block::iterator pos, IRInstruction* &shuffle,
                     SSATmp* src, const PhysLoc& rs, const PhysLoc& rd) {
  assert(rs != rd);
  unsigned i;
  if (shuffle) {
    // already have shuffle here
    i = shuffle->numSrcs();
    shuffle->addCopy(m_unit, src, rd);
  } else {
    i = 0;
    auto cap = 1;
    auto dests = new (m_unit.arena()) PhysLoc[cap];
    dests[0] = rd;
    auto& marker = pos != b->end() ? pos->marker() : b->back().marker();
    shuffle = m_unit.gen(Shuffle, marker, ShuffleData(dests, 1, cap), src);
    b->insert(pos, shuffle);
  }
  auto& shuffle_regs = m_regs[shuffle];
  shuffle_regs.resize(i + 1);
  shuffle_regs.src(i) = rs;
  auto inst_pos = m_posns[*pos];
  m_posns[shuffle] = pos->op() == Shuffle ? inst_pos : inst_pos - 1;
}

// Insert a spill-store Shuffle after the instruction that defines ivl->tmp.
// If the instruction is a branch, do the store on the edge to the next block.
void XLS::insertSpill(Interval* ivl) {
  assert(!ivl->isChild() && ivl->start() % 2 == 0);
  auto inst = ivl->tmp->inst();
  if (inst->isBlockEnd()) {
    auto succ = inst->next();
    auto iter = succ->skipHeader();
    auto& shuffle = m_edgeCopies[inst->block()].first;
    insertCopy(succ, iter, shuffle, ivl->tmp, ivl->loc, ivl->spill);
  } else {
    assert(inst != &inst->block()->back()); // can't be last in block
    auto block = inst->block();
    auto iter = block->iteratorTo(inst);
    auto& shuffle = m_insts[ivl->start() + 1];
    insertCopy(block, ++iter, shuffle, ivl->tmp, ivl->loc, ivl->spill);
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
  if (dumpIREnabled(kRegAllocLevel)) dumpIntervals();
  for (auto i1 : m_intervals) {
    if (!i1) continue;
    if (i1->spill.spilled()) insertSpill(i1);
    for (auto i2 = i1->next; i2; i1 = i2, i2 = i2->next) {
      auto pos = i2->start();
      if (i1->end() != pos) continue; // spans lifetime hole
      if (i1->loc == i2->loc) continue; // no copy necessary
      if (i2->loc.spilled() || !i2->loc.numAllocated()) continue; // i2 spilled
      if (pos % 2 == 0) {
        // even position requiring a copy must be on edge
        assert(pos >= 2 && m_insts[pos - 2]->block() != m_insts[pos]->block());
      } else {
        // odd position
        auto inst1 = m_insts[pos-1];
        auto inst2 = m_insts[pos+1];
        auto block = inst2->block();
        if (inst1->block() != block) continue; // copy is on edge
        insertCopy(block, block->iteratorTo(inst2), m_insts[pos],
                   i1->tmp, i1->loc, i2->loc);
      }
    }
  }
}

// Insert copies on control-flow edges, and turn Jmps into Shuffles
void XLS::resolveEdges() {
  const PhysLoc invalid_loc;
  for (auto succ : m_blocks) {
    auto& inst2 = succ->front();
    auto pos2 = m_posns[inst2]; // pos of first inst in succ.
    succ->forEachPred([&](Block* pred) {
      assert(pred->back().op() != Shuffle);
      auto it1 = pred->end();
      auto& inst1 = *(--it1);
      auto pos1 = m_posns[inst1]; // pos of last inst in pred
      if (inst1.op() == Jmp) {
        // insert copies on each Jmp->DefLabel edge
        for (unsigned i = 0, n = inst1.numSrcs(); i < n; ++i) {
          auto i1 = m_intervals[inst1.src(i)]; // null if const
          auto i2 = m_intervals[inst2.dst(i)]; // null if unused
          if (i1) i1 = i1->childAt(pos1);
          auto loc1 = i1 ? i1->loc : invalid_loc;
          auto loc2 = i2 ? i2->loc : invalid_loc;
          if (loc1 == loc2) continue;
          auto& shuffle = m_edgeCopies[pred].second; // taken edge
          insertCopy(pred, it1, shuffle, inst1.src(i), loc1, loc2);
        }
      }
      m_liveIn[succ].forEach([&](uint32_t id) {
        auto ivl = m_intervals[id];
        resolveFlow(ivl, pred, succ, pos1, pos2);
      });
    });
  }
  if (dumpIREnabled(kRegAllocLevel)) {
    print("after resolving intervals");
  }
}

void XLS::resolveFlow(Interval* parent, Block* pred, Block* succ,
                      unsigned pos1, unsigned pos2) {
  Interval* i1 = nullptr;
  Interval* i2 = nullptr;
  for (auto ivl = parent; ivl && !(i1 && i2); ivl = ivl->next) {
    if (ivl->covers(pos1)) i1 = ivl;
    if (ivl->covers(pos2)) i2 = ivl;
  }
  if (i2->loc.spilled()) return; // we did spill store after def.
  if (i1->loc == i2->loc) return; // nothing to do.
  auto& shuffle = pred->next() == succ ? m_edgeCopies[pred].first :
                  m_edgeCopies[pred].second;
  if (pred->taken() && pred->next()) {
    // pred has 2+ successors; insert copy at start of succ
    insertCopy(succ, succ->skipHeader(), shuffle, i1->tmp, i1->loc, i2->loc);
  } else {
    // insert copy at end of predecessor
    insertCopy(pred, pred->backIter(), shuffle, i1->tmp, i1->loc, i2->loc);
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
  unsigned numSplits = 0;
  for (auto ivl : m_intervals) {
    if (!ivl) continue;
    HPHP::Trace::traceRelease("i%-2d %s\n", ivl->id(),
                              ivl->toString().c_str());
    for (ivl = ivl->next; ivl; ivl = ivl->next) {
      numSplits++;
      HPHP::Trace::traceRelease("    %s\n", ivl->toString().c_str());
    }
  }
  HPHP::Trace::traceRelease("Splits %d Spills %d\n", numSplits, m_nextSpill);
}

template<class F>
void forEachInterval(Intervals& intervals, F f) {
  for (auto ivl : intervals) {
    if (ivl) f(ivl);
  }
}

enum Mode { Light, Heavy };
template<class Pred>
const char* draw(Interval* parent, unsigned pos, Mode m, Pred covers) {
                               // Light     Heavy
  static const char* top[]    = { "\u2575", "\u2579" };
  static const char* bottom[] = { "\u2577", "\u257B" };
  static const char* both[]   = { "\u2502", "\u2503" };
  static const char* empty[]  = { " ", " " };
  auto f = [&](unsigned pos) {
    for (auto ivl = parent; ivl; ivl = ivl->next) {
      if (covers(ivl, pos)) return true;
    }
    return false;
  };

  auto s = f(pos);
  auto d = pos%2 == 1 ? s : f(pos+1);
  return ( s && !d) ? top[m] :
         ( s &&  d) ? both[m] :
         (!s &&  d) ? bottom[m] :
         empty[m];
}

void XLS::print(const char* caption) {
  std::ostringstream str;
  str << "Intervals " << caption << "\n";
  forEachInterval(m_intervals, [&] (Interval* ivl) {
    str << folly::format(" {: <2}", ivl->id());
  });
  str << "\n";
  for (auto& b : m_blocks) {
    for (auto& i : *b) {
      auto pos = m_posns[i];
      forEachInterval(m_intervals, [&] (Interval* ivl) {
        str << " ";
        str << draw(ivl, pos, Light, [&](Interval* child, unsigned p) {
          return child->covers(p);
        });
        str << draw(ivl, pos, Heavy, [&](Interval* child, unsigned p) {
          return child->usedAt(p);
        });
      });
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
      JIT::printSrcs(str, &i, &m_regs);
      if (i.numDsts()) {
        str << " => ";
        JIT::printDsts(str, &i, &m_regs);
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
  if (tmp) {
    print(out, tmp, &loc);
  } else {
    out << loc;
  }
  out << " [";
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

// Check validity of this interval
// 1. split-children cannot have more children, nor can the parent
//    be a child of another interval.
// 2. live ranges must be nonempty, sorted, and disjoint
// 3. holes between live ranges must also be non-empty
// 4. uses must be sorted
// 5. every use must be inside or just off the end of a range
// 6. parent and children must all be sorted and disjoint
bool Interval::checkInvariants() const {
  assert(!parent || !parent->parent); // 1: no crazy nesting
  assert(!ranges.empty());
  DEBUG_ONLY auto min_start = 0;
  DEBUG_ONLY auto u = uses.begin();
  for (auto r : ranges) {
    assert(r.start < r.end); // 2: nonempty range
    assert(r.start >= min_start); // 2: ranges are sorted
    min_start = r.end + 1; // 2,3: ranges are disjoint, no empty holes
    DEBUG_ONLY auto min_use = r.start;
    while (u != uses.end() && min_use <= u->pos && u->pos <= r.end) {
      min_use = u->pos; // 4: uses must be sorted
      u++;
    }
  }
  assert(u == uses.end()); // 4,5: all uses covered
  assert(!next || next->start() >= end()); // 6: next child is ok.
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
  Timer _t(Timer::regalloc);

  RegAllocInfo regs(unit);
  Abi abi;
  switch (arch()) {
    case Arch::X64:
      abi = x64_abi;
      break;
    case Arch::ARM:
      abi = arm_abi;
      break;
  }
  XLS xls(unit, regs, abi);
  xls.allocate();
  if (dumpIREnabled()) {
    dumpTrace(kRegAllocLevel, unit, " after extended alloc ", &regs,
              nullptr, nullptr);
  }
  return regs;
}

}}
