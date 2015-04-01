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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/assertions.h"
#include "hphp/util/dataflow-worklist.h"

#include <boost/dynamic_bitset.hpp>

#include <algorithm>
#include <random>

// future work
//  - #3098509 streamline code, vectors vs linked lists, etc
//  - #3098685 Optimize lifetime splitting
//  - #3098739 new features now possible with XLS

TRACE_SET_MOD(xls);

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

namespace {
///////////////////////////////////////////////////////////////////////////////

size_t s_counter;

// A Use refers to the position where an interval is used or defined
struct Use {
  VregKind kind;
  unsigned pos;
  Vreg hint; // if valid, try to use same vreg as this
};

// A LiveRange is an closed-open range of positions where an interval is live.
// Specifically, for the LiveRange [start, end), start is in the range and
// end is not.
struct LiveRange {
  bool contains(unsigned pos) const { return pos >= start && pos < end; }
  bool intersects(LiveRange r) const { return r.start < end && start < r.end; }
  bool contains(LiveRange r) const { return r.start >= start && r.end <= end; }
public:
  unsigned start, end;
};

typedef jit::vector<LiveRange> RangeList;
typedef jit::vector<Use> UseList;

constexpr int kInvalidSpillSlot = -1;

// An Interval stores the lifetime of a Vreg as a sorted list of disjoint
// ranges, and a sorted list of use positions. If this interval was split,
// then the first interval is deemed "parent" and the rest are "children",
// and they're all connected as a singly linked list sorted by start.
//
// Every use position must be inside one of the ranges, or exactly at the
// end of the last range. Allowing a use exactly at the end facilitates
// lifetime splitting when the use at the position of an instruction
// clobbers registers as a side effect, e.g. a call.
struct Interval {
  explicit Interval(Vreg r) : parent(nullptr), vreg(r) {}
  explicit Interval(Interval* parent)
    : parent(parent)
    , vreg(parent->vreg)
    , wide(parent->wide)
    , constant(parent->constant)
    , val(parent->val)
  {}
  // accessors
  unsigned start() const { return ranges.front().start; }
  unsigned end() const { return ranges.back().end; }
  bool fixed() const { return vreg.isPhys(); }
  Interval* leader() { return parent ? parent : this; }
  bool spilled() const { return reg == InvalidReg && slot >= 0; }
  // queries
  unsigned findRange(unsigned pos) const;
  unsigned findUse(unsigned pos) const;
  bool covers(unsigned pos) const;
  bool usedAt(unsigned pos) const;
  unsigned firstUseAfter(unsigned pos) const;
  unsigned lastUseBefore(unsigned pos) const;
  unsigned firstUse() const;
  Interval* childAt(unsigned pos);
  // mutators
  void add(LiveRange r);
  Interval* split(unsigned pos, bool keep_uses = false);
  // debugging
  std::string toString();
public:
  Interval* const parent;
  Interval* next{nullptr};
  RangeList ranges;
  UseList uses;
  const Vreg vreg;
  unsigned def_pos;
  int slot{kInvalidSpillSlot};
  bool wide{false};
  PhysReg reg;
  bool constant{false};
  Vconst val;
};

typedef boost::dynamic_bitset<> LiveSet;
typedef std::pair<Vlabel,unsigned> EdgeKey;
struct EdgeHasher {
  size_t operator()(EdgeKey k) const {
    return size_t(k.first) ^ k.second;
  }
};

typedef PhysReg::Map<Interval*> CopyPlan;
typedef PhysReg::Map<unsigned> PosVec;

// Extended Linear Scan register allocator over vasm virtual registers (Vregs).
// This encapsulates the intermediate data structures used during the algorithm
// so we don't have to pass them around everywhere.
struct Vxls {
  Vxls(Vunit& unit, const Abi& abi)
    : unit(unit)
    , m_abi(abi)
    , m_sp(mcg->backEnd().rSp())
  {
    switch (arch()) {
      case Arch::X64:
        m_tmp = reg::xmm15; // reserve xmm15 to break shuffle cycles
        break;
      case Arch::ARM:
        m_tmp = vixl::x17; // also used as tmp1 by MacroAssembler
        break;
    }
    m_abi.simdUnreserved.remove(m_tmp);
    m_abi.simdReserved.add(m_tmp);
    assertx(!m_abi.gpUnreserved.contains(m_sp));
    assertx(!m_abi.gpUnreserved.contains(m_tmp));
  }
  ~Vxls();

  // phases
  void allocate();
  void computePositions();
  void analyzeRsp();
  void buildIntervals();
  void walkIntervals();
  void resolveSplits();
  void lowerCopyargs();
  void resolveEdges();
  void renameOperands();
  void insertCopies();
  void allocateSpillSpace();

  // utilities
  void update(unsigned pos);
  void allocate(Interval*);
  void allocBlocked(Interval*);
  void assignReg(Interval*, PhysReg);
  unsigned constrain(Interval*, RegSet&);
  void insertSpill(Interval*);
  Vlabel findBlock(unsigned pos);
  LiveRange findBlockRange(unsigned pos);
  void spill(Interval*);
  void spillAfter(Interval* ivl, unsigned pos);
  void spillOthers(Interval* current, PhysReg r);
  void assignSpill(Interval* ivl);
  void insertCopiesAt(jit::vector<Vinstr>& code, unsigned& j,
                      const CopyPlan&, MemoryRef slots, unsigned pos);
  void insertSpillsAt(jit::vector<Vinstr>& code, unsigned& j,
                      const CopyPlan&, MemoryRef slots, unsigned pos);
  PhysReg findHint(Interval* current, const PosVec& free_until, RegSet allow);
  unsigned nearestSplitBefore(unsigned pos);

  // debugging
  void print(const char* caption);
  void printInstr(std::ostringstream& out, Vinstr* instr, unsigned pos, Vlabel);
  void dumpIntervals();
  int spEffect(Vinstr& inst) const;

public:
  /*
   * Comparison function for pending priority queue.
   *
   * std::priority_queue requires a less operation, but sorts the heap
   * highest-first; we need the opposite (lowest-first), so use greater-than.
   */
  struct Compare {
    bool operator()(const Interval* i1, const Interval* i2) {
      return i1->start() > i2->start();
    }
  };

public:
  Vunit& unit;
  Abi m_abi;

  // Arch-dependent stack pointer.
  PhysReg m_sp;
  // Temp register only for breaking cycles.
  PhysReg m_tmp;

  // Sorted blocks.
  jit::vector<Vlabel> blocks;
  // [start,end) position of each block.
  jit::vector<LiveRange> block_ranges;
  // Per-block sp[offset] to spill-slots.
  jit::vector<int> spill_offsets;
  // Per-block live-in sets.
  jit::vector<LiveSet> livein;

  // Parent intervals, null if unused.
  jit::vector<Interval*> intervals;
  // Intervals sorted by Interval start.
  jit::priority_queue<Interval*,Compare> pending;
  // Intervals that overlap.
  jit::vector<Interval*> active, inactive;

  // Where to insert copies.
  jit::hash_map<unsigned,CopyPlan> copies;
  // Where to insert spills.
  jit::hash_map<unsigned,CopyPlan> spills;
  // Copies on edges.
  jit::hash_map<EdgeKey,CopyPlan,EdgeHasher> edge_copies;

  // Number of intervals spilled.
  unsigned num_spills{0};
  // Number of spill slots used.
  size_t used_spill_slots{0};
  // Last position each spill slot was owned; kMaxPos means currently used.
  jit::array<unsigned, kMaxSpillSlots> spill_slots{{0}};
};

const unsigned kMaxPos = UINT_MAX; // "infinity" use position

//////////////////////////////////////////////////////////////////////////////

template<class Fn> void forEach(LiveSet& bits, Fn fn) {
  for (auto i = bits.find_first(); i != bits.npos; i = bits.find_next(i)) {
    fn(Vreg(i));
  }
}

//////////////////////////////////////////////////////////////////////////////

void Interval::add(LiveRange r) {
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

// Return range index containing or higher than pos
unsigned Interval::findRange(unsigned pos) const {
  unsigned lo = 0;
  for (unsigned hi = ranges.size(); lo < hi;) {
    auto mid = (lo + hi) / 2;
    auto r = ranges[mid];
    if (pos < r.start) {
      hi = mid;
    } else if (r.end <= pos) {
      lo = mid + 1;
    } else {
      return mid;
    }
  }
  assertx(lo == ranges.size() || pos < ranges[lo].start);
  return lo;
}

unsigned Interval::findUse(unsigned pos) const {
  unsigned lo = 0, hi = uses.size();
  while (lo < hi) {
    auto mid = (lo + hi) / 2;
    auto u = uses[mid].pos;
    if (pos < u) {
      hi = mid;
    } else if (u < pos) {
      lo = mid + 1;
    } else {
      return mid;
    }
  }
  assertx(lo == uses.size() || pos < uses[lo].pos);
  return lo;
}

// Return true if one of the ranges in this interval includes pos
bool Interval::covers(unsigned pos) const {
  if (pos < start() || pos >= end()) return false;
  auto i = ranges.begin() + findRange(pos);
  return i != ranges.end() && i->contains(pos);
}

// Return true if there is a use position at pos
bool Interval::usedAt(unsigned pos) const {
  if (pos < start() || pos > end()) return false;
  auto i = uses.begin() + findUse(pos);
  return i != uses.end() && pos == i->pos;
}

// Return the interval which has a use position at pos
Interval* Interval::childAt(unsigned pos) {
  assertx(!parent);
  for (auto ivl = this; ivl; ivl = ivl->next) {
    if (pos < ivl->start()) return nullptr;
    if (ivl->usedAt(pos)) return ivl;
  }
  return nullptr;
}

// Return the next intersection point between current and other, or kMaxPos
// if they never intersect.
unsigned nextIntersect(const Interval* current, const Interval* other) {
  assertx(!current->fixed());
  if (!current->parent && !other->parent && !other->fixed()) {
    // Since other is inactive, it cannot cover current's start, and
    // current cannot cover other's start, since other started earlier.
    // Therefore, SSA guarantees no intersection.
    return kMaxPos;
  }
  if (current->end() <= other->start()) {
    // current ends before other starts.
    return kMaxPos;
  }
  // r1,e1 span all of current
  auto r1 = current->ranges.begin();
  auto e1 = current->ranges.end();
  // r2,e2 span the tail of other that might intersect current
  auto r2 = other->ranges.begin() + other->findRange(current->start());
  auto e2 = other->ranges.end();
  // search for the lowest position covered by current and other
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

// Return the position of the next use >= pos.
// If there are no more uses after pos, return kMaxPos.
unsigned Interval::firstUseAfter(unsigned pos) const {
  auto u = uses.begin() + findUse(pos);
  return u != uses.end() && pos <= u->pos ? u->pos :
         kMaxPos;
}

// Return the position of the latest use <= pos.
// If the first use is after pos, return 0.
unsigned Interval::lastUseBefore(unsigned pos) const {
  auto prev = 0;
  for (auto& u : uses) {
    if (u.pos > pos) return prev;
    prev = u.pos;
  }
  return prev;
}

// Return the position of the first use that requires a register,
// or kMaxPos if no remaining uses need registers.
unsigned Interval::firstUse() const {
  return uses.empty() ? kMaxPos : uses.front().pos;
}

// Split this interval at pos and return the rest. Pos must be a location
// that ensures both shorter intervals are nonempty. If keep_uses is set, Uses
// exactly at the end of the first interval will stay with the first part.
Interval* Interval::split(unsigned pos, bool keep_uses) {
  assertx(pos > start() && pos < end()); // both parts will be non-empty
  auto leader = this->leader();
  Interval* child = jit::make<Interval>(leader);
  child->next = next;
  next = child;
  // advance r1 to the first range we want in child; maybe split a range.
  auto r1 = ranges.begin() + findRange(pos);
  if (pos > r1->start) { // split r at pos
    child->ranges.push_back({pos, r1->end});
    r1->end = pos;
    r1++;
  }
  child->ranges.insert(child->ranges.end(), r1, ranges.end());
  ranges.erase(r1, ranges.end());
  // advance u1 to the first use position in child, then copy u1..end to child.
  auto u1 = uses.begin() + findUse(end());
  auto u2 = uses.end();
  if (keep_uses) {
    while (u1 != u2 && u1->pos <= end()) u1++;
  } else {
    while (u1 != u2 && u1->pos < child->start()) u1++;
  }
  child->uses.insert(child->uses.end(), u1, u2);
  uses.erase(u1, u2);
  return child;
}

//////////////////////////////////////////////////////////////////////////////

Vxls::~Vxls() {
  for (auto ivl : intervals) {
    for (Interval* next; ivl; ivl = next) {
      next = ivl->next;
      jit::destroy(ivl);
    }
  }
}

/*
 * Extended Linear Scan is based on Wimmer & Franz "Linear Scan Register
 * Allocation on SSA Form". As currently written, it also works on non-ssa
 * input.
 *
 * 1. Sort blocks such that all predecessors of B come before B, except
 * loop-edge predecessors. If the input IR is in SSA form, this also
 * implies the definition of each SSATmp comes before all uses.
 *
 * 2. Assign an even numbered position to every instruction. Positions
 * between instructions are used to insert copies and spills. Each block
 * starts with an extra position number that corresponds to an imaginary
 * "label" instruction that is not physically in the vasm IR.
 *
 * 3. Create one interval I for each Vreg R that requires register allocation,
 * by iterating blocks and instructions in reverse order, computing live
 * registers as we go. Each interval consists of a sorted list of disjoint,
 * live ranges covering the positions where R must be in a physical register
 * or spill slot. Vregs that are constants or have forced registers
 * (e.g. VmSp) are skipped. If the input is SSA, the start position of each
 * interval dominates every live range and use position in the interval.
 *
 * 4. Process intervals in order of start position, maintaining the set of
 * active (live) and inactive (not live, but with live ranges that start
 * after the current interval). When choosing a register, prefer the one
 * available furthest into the future. If necessary, split the current
 * interval so the first part gets a register, and enqueue the rest.
 * When no registers are available, choose either the current interval or
 * another one to spill, trying to free up the longest-available register.
 *
 * Split positions must be after an interval's start position, and on or before
 * the chosen split point. We're free try to choose a good position inbetween,
 * for example block boundaries and cold blocks.
 *
 * 5. Once intervals have been walked and split, every interval has an assigned
 * operand (register or spill location) for all positions where its alive.
 * Visit every instruction and modify its Vreg operands to the physical
 * register that was assigned.
 *
 * 6. Splitting creates sub-intervals that are assigned to different registers
 * or spill locations, so insert resolving copies at the split positions
 * between intervals that were split in a block, and copies on control-flow
 * edges connecting different sub-intervals. When more than one copy occurs
 * in a position, they are parallel-copies (all sources read before any dest
 * is written).
 *
 * If any sub-interval was spilled, a single store is generated after each
 * definition point.
 */

void Vxls::allocate() {
  splitCriticalEdges(unit);
  blocks = sortBlocks(unit);
  assertx(check(unit));
  computePositions();
  analyzeRsp();
  buildIntervals();
  walkIntervals();
  resolveSplits();
  lowerCopyargs();
  resolveEdges();
  renameOperands();
  insertCopies();
  // Remove nop-copies before allocating spill space, since it might create new
  // blocks and modify the cfg.
  for (auto b : blocks) {
    auto& code = unit.blocks[b].code;
    auto end = std::remove_if(code.begin(), code.end(), [&](Vinstr& inst) {
      return is_trivial_nop(inst) ||
             inst.op == Vinstr::copyargs || // we lowered it
             inst.op == Vinstr::phidef; // we lowered it
    });
    code.erase(end, code.end());
  }

  allocateSpillSpace();
  printUnit(kVasmRegAllocLevel, "after vasm-xls", unit);
}

// compute the linear position range of each block
void Vxls::computePositions() {
  block_ranges.resize(unit.blocks.size());
  unsigned pos = 0;
  for (auto b : blocks) {
    auto& code = unit.blocks[b].code;
    bool front_uses{false};
    visitUses(unit, code.front(), [&](Vreg r) {
      front_uses = true;
    });
    if (front_uses) {
      auto origin = code.front().origin;
      code.insert(code.begin(), nop{});
      code.front().origin = origin;
    }
    auto start = pos;
    for (auto& inst : unit.blocks[b].code) {
      inst.pos = pos;
      pos += 2;
    }
    block_ranges[b] = { start, pos };
  }
}

// Return the effect this instruction has on the value of sp.
// this will assert if an instruction mutates sp in an untrackable way.
int Vxls::spEffect(Vinstr& inst) const {
  switch (inst.op) {
    default:
      if (debug) visitDefs(unit, inst, [&](Vreg r) { assertx(r != m_sp); });
      return 0;
    case Vinstr::push:
      return -8;
    case Vinstr::pop:
      return 8;
    case Vinstr::addqi: {
      auto& i = inst.addqi_;
      if (i.d == Vreg64(m_sp)) {
        assertx(i.s1 == Vreg64(m_sp));
        return i.s0.l();
      }
      return 0;
    }
    case Vinstr::subqi: {
      auto& i = inst.subqi_;
      if (i.d == Vreg64(m_sp)) {
        assertx(i.s1 == Vreg64(m_sp));
        return -i.s0.l();
      }
      return 0;
    }
    case Vinstr::lea: {
      auto& i = inst.lea_;
      if (i.d == Vreg64(m_sp)) {
        assertx(i.s.base == i.d && !i.s.index.isValid());
        return i.s.disp;
      }
      return 0;
    }
  }
}

// compute the offset from RSP to the spill area at each block start.
void Vxls::analyzeRsp() {
  auto num_blocks = unit.blocks.size();
  boost::dynamic_bitset<> visited(num_blocks);
  spill_offsets.resize(num_blocks);
  for (auto b : blocks) {
    int offset;
    if (visited.test(b)) {
      offset = spill_offsets[b];
    } else {
      offset = 0;
    }
    for (auto& inst : unit.blocks[b].code) {
      offset -= spEffect(inst);
    }
    for (auto s : succs(unit.blocks[b])) {
      if (visited.test(s)) {
        assert_flog(offset == spill_offsets[s],
                    "sp mismatch on edge B{}->B{}, expected {} got {}",
                    size_t(b), size_t(s), spill_offsets[s], offset);
      } else {
        spill_offsets[s] = offset;
        visited.set(s);
      }
    }
  }
}

// Visits defs of an instruction, updates their liveness, adds live
// ranges, and adds Uses with appropriate hints.
struct DefVisitor {
  DefVisitor(LiveSet& live, Vxls& vxls, unsigned pos)
    : m_intervals(vxls.intervals)
    , m_tuples(vxls.unit.tuples)
    , m_live(live)
    , m_pos(pos)
  {}
  template<class F> void imm(const F&){} // skip immediates
  template<class R> void use(R){} // skip uses
  template<class S, class H> void useHint(S, H){} // skip uses
  template<class R> void across(R){} // skip uses
  void def(Vtuple defs) {
    for (auto r : m_tuples[defs]) def(r);
  }
  void defHint(Vtuple def_tuple, Vtuple hint_tuple) {
    auto& defs = m_tuples[def_tuple];
    auto& hints = m_tuples[hint_tuple];
    for (int i = 0; i < defs.size(); i++) {
      def(defs[i], VregKind::Any, hints[i]);
    }
  }
  template<class D, class H> void defHint(D dst, H hint) {
    def(dst, dst.kind, hint, dst.bits == 128);
  }
  template<class R> void def(R r) {
    def(r, r.kind, Vreg{}, r.bits == 128);
  }
  void def(Vreg r) { def(r, VregKind::Any); }
  void defHint(Vreg d, Vreg hint) { def(d, VregKind::Any, hint); }
  void def(RegSet rs) { rs.forEach([&](Vreg r) { def(r); }); }
private:
  void def(Vreg r, VregKind kind, Vreg hint = Vreg{}, bool wide = false) {
    auto ivl = m_intervals[r];
    if (m_live.test(r)) {
      m_live.reset(r);
      ivl->ranges.back().start = m_pos;
    } else {
      if (!ivl) {
        ivl = m_intervals[r] = jit::make<Interval>(r);
      }
      ivl->add({m_pos, m_pos + 1});
    }
    if (!ivl->fixed()) {
      ivl->uses.push_back(Use{kind, m_pos, hint});
      ivl->wide |= wide;
      ivl->def_pos = m_pos;
    }
    i++;
  }
private:
  jit::vector<Interval*>& m_intervals;
  jit::vector<VregList>& m_tuples;
  LiveSet& m_live;
  unsigned m_pos;
  unsigned i{0};
};

// this kinda sucks, but I can't use a lambda to handle Vreg and Vptr
// at the same time.
struct UseVisitor {
  UseVisitor(LiveSet& live, Vxls& vxls, LiveRange range)
    : m_intervals(vxls.intervals)
    , m_tuples(vxls.unit.tuples)
    , m_live(live)
    , m_range(range)
  {}
  template<class F> void imm(const F&){} // skip immediates
  template<class R> void def(R){} // skip defs
  template<class D, class H> void defHint(D, H){} // skip defs

  void use(Vptr m) {
    if (m.base.isValid()) use(m.base);
    if (m.index.isValid()) use(m.index);
  }
  void use(Vtuple uses) {
    for (auto r : m_tuples[uses]) use(r);
  }
  void use(VcallArgsId id) {
    always_assert(false && "vcall unsupported in vxls");
  }
  void useHint(Vtuple src_tuple, Vtuple hint_tuple) {
    auto& uses = m_tuples[src_tuple];
    auto& hints = m_tuples[hint_tuple];
    for (int i = 0, n = uses.size(); i < n; i++) {
      useHint(uses[i], hints[i]);
    }
  }
  void use(RegSet regs) {
    regs.forEach([&](Vreg r) { use(r); });
  }
  void across(RegSet regs) {
    regs.forEach([&](Vreg r) { across(r); });
  }

  template<class R> void use(R r) { use(r, r.kind, m_range.end); }
  template<class S, class H> void useHint(S src, H hint) {
    use(src, src.kind, m_range.end, hint);
  }
  void use(Vreg r, VregKind kind, unsigned end, Vreg hint = Vreg{}) {
    m_live.set(r);
    auto ivl = m_intervals[r];
    if (!ivl) ivl = m_intervals[r] = jit::make<Interval>(r);
    ivl->add({m_range.start, end});
    if (!ivl->fixed()) {
      ivl->uses.push_back({kind, m_range.end, hint});
    }
  }

  // An operand marked as UA means use-after or use-across. Mark it live
  // across the instruction so its lifetime conflicts with the destination,
  // which ensures it will be assigned a different register than the
  // destination. This isn't necessary if *both* operands of a binary
  // instruction are the same virtual register, but is still correct.
  template<class R> void across(R r) { use(r, r.kind, m_range.end + 1); }
private:
  jit::vector<Interval*>& m_intervals;
  jit::vector<VregList>& m_tuples;
  LiveSet& m_live;
  const LiveRange m_range;
};

// Compute lifetime intervals and use positions of all intervals by walking
// the code bottom-up once.
void Vxls::buildIntervals() {
  ONTRACE(kRegAllocLevel, printCfg(unit, blocks));
  livein.resize(unit.blocks.size());
  intervals.resize(unit.next_vr);
  UNUSED auto loops = false;
  auto const preds = computePreds(unit);
  for (auto blockIt = blocks.end(); blockIt != blocks.begin();) {
    auto b = *--blockIt;
    auto& block = unit.blocks[b];

    // initial live set is the union of successor live sets.
    LiveSet live(unit.next_vr);
    for (auto s : succs(block)) {
      if (!livein[s].empty()) {
        live |= livein[s];
      } else {
        // livein[s].empty() implies we haven't processed s yet
        loops = true;
      }
    }

    // add a range covering the whole block to every live interval
    auto& block_range = block_ranges[b];
    forEach(live, [&](Vreg r) {
      intervals[r]->add(block_range);
    });

    // visit instructions bottom-up, adding uses & ranges
    auto pos = block_range.end;
    for (auto i = block.code.end(); i != block.code.begin();) {
      auto& inst = *--i;
      pos -= 2;
      RegSet implicit_uses, implicit_across, implicit_defs;
      getEffects(m_abi, inst, implicit_uses, implicit_across, implicit_defs);

      DefVisitor dv(live, *this, pos);
      visitOperands(inst, dv);
      dv.def(implicit_defs);

      UseVisitor uv(live, *this, {block_range.start, pos});
      visitOperands(inst, uv);
      uv.use(implicit_uses);
      uv.across(implicit_across);
    }

    // save live set so it can propagate to predecessors
    livein[b] = live;

    // add a loop-covering range to each interval live into a loop.
    for (auto p : preds[b]) {
      auto pred_end = block_ranges[p].end;
      if (pred_end > block_range.start) {
        forEach(live, [&](Vreg r) {
          auto ivl = intervals[r];
          ivl->add(LiveRange{block_range.start, pred_end});
        });
      }
    }
  }

  // finish processing live ranges for constants
  for (auto& c : unit.constants) {
    if (auto ivl = intervals[c.second]) {
      ivl->ranges.back().start = 0;
      ivl->constant = true;
      ivl->val = c.first;
    }
  }

  // Ranges and uses were generated in reverse order. Unreverse them now.
  for (auto ivl : intervals) {
    if (!ivl) continue;
    assertx(!ivl->ranges.empty()); // no empty intervals
    std::reverse(ivl->uses.begin(), ivl->uses.end());
    std::reverse(ivl->ranges.begin(), ivl->ranges.end());
  }
  ONTRACE(kRegAllocLevel,
    if (loops) HPHP::Trace::traceRelease("vasm-loops\n");
    print("after building intervals");
  );

  // only constants and physical registers can be live-into the entry block.
  if (debug) {
    forEach(livein[unit.entry], [&](Vreg r) {
      UNUSED auto ivl = intervals[r];
      assertx(ivl->constant || ivl->fixed());
    });
    for (auto ivl : intervals) {
      if (!ivl) continue;
      for (unsigned i = 1; i < ivl->uses.size(); i++) {
        assertx(ivl->uses[i].pos >= ivl->uses[i-1].pos); // monotonic
      }
      for (unsigned i = 1; i < ivl->ranges.size(); i++) {
        assertx(ivl->ranges[i].end > ivl->ranges[i].start); // no empty ranges
        assertx(ivl->ranges[i].start > ivl->ranges[i-1].end); // no empty gaps
      }
    }
  }
}

void Vxls::walkIntervals() {
  for (auto ivl : intervals) {
    if (!ivl) continue;
    if (ivl->fixed()) {
      assignReg(ivl, ivl->vreg);
    } else if (ivl->constant) {
      spill(ivl);
    } else {
      pending.push(ivl);
    }
  }
  while (!pending.empty()) {
    auto current = pending.top();
    pending.pop();
    update(current->start());
    allocate(current);
  }
}

void erase(jit::vector<Interval*>& list, jit::vector<Interval*>::iterator i) {
  *i = list.back();
  list.pop_back();
}

// Update active and inactive lists based on pos
void Vxls::update(unsigned pos) {
  auto free_spill_slot = [this] (Interval* ivl) {
    assertx(!ivl->next);
    auto slot = ivl->leader()->slot;

    if (slot != kInvalidSpillSlot) {
      if (ivl->wide) {
        assertx(spill_slots[slot + 1]);
        spill_slots[slot + 1] = ivl->end();
      }
      assertx(spill_slots[slot]);
      spill_slots[slot] = ivl->end();
    }
  };

  // check for active intervals in that are expired or inactive
  for (auto i = active.begin(); i != active.end();) {
    auto ivl = *i;
    if (pos >= ivl->end()) {
      erase(active, i);
      if (!ivl->next) free_spill_slot(ivl);
    } else if (!ivl->covers(pos)) {
      erase(active, i);
      inactive.push_back(ivl);
    } else {
      i++;
    }
  }
  // check for intervals that are expired or active
  for (auto i = inactive.begin(); i != inactive.end();) {
    auto ivl = *i;
    if (pos >= ivl->end()) {
      erase(inactive, i);
      if (!ivl->next) free_spill_slot(ivl);
    } else if (ivl->covers(pos)) {
      erase(inactive, i);
      active.push_back(ivl);
    } else {
      i++;
    }
  }
}

PhysReg find(const PosVec& posns) {
  unsigned max = 0;
  PhysReg r1 = *posns.begin();
  for (auto r : posns) {
    if (posns[r] > max) {
      r1 = r;
      max = posns[r];
    }
  }
  return r1;
}

// Constrain the allowable registers for ivl by inspecting uses, and
// return the latest position for which that set is valid.
unsigned Vxls::constrain(Interval* ivl, RegSet& allow) {
  auto const any = m_abi.unreserved() - m_abi.sf; // Any but not flags.
  allow = m_abi.unreserved();
  for (auto& u : ivl->uses) {
    auto need = u.kind == VregKind::Simd ? m_abi.simdUnreserved :
                u.kind == VregKind::Gpr ? m_abi.gpUnreserved :
                u.kind == VregKind::Sf ? m_abi.sf :
                /* VregKind::Any */ any;
    if ((allow & need).empty()) {
      // cannot satisfy constraints; must split before u.pos
      return u.pos - 1;
    }
    allow &= need;
  }
  return kMaxPos;
}

// return the closest split position on or before pos.
// the result might be exactly on an edge, or inbetween normal positions.
unsigned Vxls::nearestSplitBefore(unsigned pos) {
  auto b = findBlock(pos);
  auto range = block_ranges[b];
  if (pos == range.start) return pos;
  return (pos - 1) | 1;
}

// Return the first usable hint from all the uses in this interval.
// Skip uses that don't have any hint, or have an unsable hint.
PhysReg Vxls::findHint(Interval* current, const PosVec& free_until,
                       RegSet allow) {
  if (!RuntimeOption::EvalHHIREnablePreColoring &&
      !RuntimeOption::EvalHHIREnableCoalescing) return InvalidReg;

  // search ivl for the register assigned to a sub-interval that ends at pos.
  auto search = [&](Interval* ivl, unsigned pos) -> PhysReg {
    for (; ivl; ivl = ivl->next) {
      if (pos == ivl->end() && ivl->reg != InvalidReg) return ivl->reg;
    }
    return InvalidReg;
  };
  for (auto u : current->uses) {
    if (!u.hint.isValid()) continue;
    auto hint_ivl = intervals[u.hint];
    PhysReg hint;
    if (hint_ivl->fixed()) {
      hint = hint_ivl->reg;
    } else if (u.pos == current->def_pos) {
      // this is a def, so u.hint is a src
      hint = search(hint_ivl, u.pos);
    }
    if (hint == InvalidReg) continue;
    if (!allow.contains(hint)) continue;
    if (free_until[hint] >= current->end()) {
      return hint;
    }
  }
  return InvalidReg;
}

void Vxls::allocate(Interval* current) {
  PosVec free_until; // 0 by default
  RegSet allow;
  unsigned conflict = constrain(current, allow);
  allow.forEach([&](PhysReg r) {
    free_until[r] = conflict;
  });
  for (auto ivl : active) {
    free_until[ivl->reg] = 0;
  }
  for (auto ivl : inactive) {
    auto r = ivl->reg;
    if (free_until[r] == 0) continue;
    auto until = nextIntersect(current, ivl);
    free_until[r] = std::min(until, free_until[r]);
  }

  // Try to get a hinted register
  auto hint = findHint(current, free_until, allow);
  if (hint != InvalidReg) {
    return assignReg(current, hint);
  }
  auto r = find(free_until);
  auto pos = free_until[r];
  if (pos >= current->end()) {
    return assignReg(current, r);
  }
  if (pos > current->start()) {
    // r is free for the first part of current
    auto prev_use = current->lastUseBefore(pos);
    UNUSED auto min_split = std::max(prev_use, current->start() + 1);
    auto max_split = pos;
    assertx(min_split <= max_split);
    auto split_pos = nearestSplitBefore(max_split);
    if (split_pos > current->start()) {
      auto second = current->split(split_pos, true);
      pending.push(second);
      return assignReg(current, r);
    }
  }
  // must spill current or another victim
  allocBlocked(current);
}

// When all registers are in use, find a good interval to split and spill,
// which could be the current interval.  When an interval is split and the
// second part is spilled, possibly split the second part again before the
// next use-pos that requires a register, and enqueue the third part.
void Vxls::allocBlocked(Interval* current) {
  PosVec used, blocked;
  RegSet allow;
  unsigned conflict = constrain(current, allow); // repeated from allocate
  allow.forEach([&](PhysReg r) { used[r] = blocked[r] = conflict; });
  auto const cur_start = current->start();
  // compute next use of active registers, so we can pick the furthest one
  for (auto ivl : active) {
    if (ivl->fixed()) {
      blocked[ivl->reg] = used[ivl->reg] = 0;
    } else {
      auto use_pos = ivl->firstUseAfter(cur_start);
      used[ivl->reg] = std::min(use_pos, used[ivl->reg]);
    }
  }
  // compute next intersection/use of inactive regs to find whats free longest
  for (auto ivl : inactive) {
    auto r = ivl->reg;
    if (blocked[r] == 0) continue;
    auto intersect_pos = nextIntersect(current, ivl);
    if (intersect_pos == kMaxPos) continue;
    if (ivl->fixed()) {
      blocked[r] = std::min(intersect_pos, blocked[r]);
      used[r] = std::min(blocked[r], used[r]);
    } else {
      auto use_pos = ivl->firstUseAfter(cur_start);
      used[r] = std::min(use_pos, used[r]);
    }
  }
  // choose the best victim register(s) to spill
  auto r = find(used);
  auto used_pos = used[r];
  if (used_pos < current->firstUse()) {
    // all other intervals are used before current's first register-use
    return spill(current);
  }
  auto block_pos = blocked[r];
  if (block_pos < current->end()) {
    auto prev_use = current->lastUseBefore(block_pos);
    UNUSED auto min_split = std::max(prev_use, cur_start + 1);
    auto max_split = block_pos;
    assertx(cur_start < min_split && min_split <= max_split);
    auto split_pos = nearestSplitBefore(max_split);
    if (split_pos > current->start()) {
      auto second = current->split(split_pos, true);
      pending.push(second);
    }
  }
  spillOthers(current, r);
  assignReg(current, r);
}

// Assign r to this interval.
void Vxls::assignReg(Interval* ivl, PhysReg r) {
  ivl->reg = r;
  active.push_back(ivl);
}

// split ivl at pos and spill the second part.  If pos is too close
// to ivl->start(), spill all of ivl.
void Vxls::spillAfter(Interval* ivl, unsigned pos) {
  auto split_pos = nearestSplitBefore(pos);
  auto tail = split_pos <= ivl->start() ? ivl : ivl->split(split_pos);
  spill(tail);
}

// Spill ivl from its start until its first register use.  If there
// is no use, spill the entire interval.  Otherwise split the
// interval just before the use, and enqueue the second part.
void Vxls::spill(Interval* ivl) {
  unsigned first_use = ivl->firstUse();
  if (first_use <= ivl->end()) {
    auto split_pos = nearestSplitBefore(first_use);
    if (split_pos <= ivl->start()) {
      // this only can happen if we need more than the available registers
      // at a single position. It can happen in phijmp or callargs.
      TRACE(1, "vxls-punt RegSpill\n");
      PUNT(RegSpill); // cannot split before first_use
    }
    pending.push(ivl->split(split_pos));
  }
  assertx(ivl->uses.empty());
  ivl->reg = InvalidReg;
  if (!ivl->constant) assignSpill(ivl);
}

// Split and spill other intervals that conflict with current for
// register r, at current->start().  If necessary, split the victims
// again before their first use position that requires a register.
void Vxls::spillOthers(Interval* current, PhysReg r) {
  auto cur_start = current->start();
  for (auto i = active.begin(); i != active.end();) {
    auto other = *i;
    if (other->fixed() || r != other->reg) {
      i++; continue;
    }
    erase(active, i);
    spillAfter(other, cur_start);
  }
  for (auto i = inactive.begin(); i != inactive.end();) {
    auto other = *i;
    if (other->fixed() || r != other->reg) {
      i++; continue;
    }
    auto intersect = nextIntersect(current, other);
    if (intersect >= current->end()) {
      i++; continue;
    }
    erase(inactive, i);
    spillAfter(other, cur_start);
  }
}

// Assign the next available spill slot to interval
void Vxls::assignSpill(Interval* ivl) {
  assertx(!ivl->fixed() && ivl->parent && ivl->uses.empty());

  auto leader = ivl->parent;

  if (leader->slot != kInvalidSpillSlot) {
    ivl->slot = leader->slot;
    return;
  }

  auto assign_slot = [&] (size_t slot) {
    ivl->slot = leader->slot = slot;
    ++num_spills;

    spill_slots[slot] = kMaxPos;
    if (ivl->wide) {
      used_spill_slots = std::max(used_spill_slots, slot + 2);
      spill_slots[slot + 1] = kMaxPos;
    } else {
      used_spill_slots = std::max(used_spill_slots, slot + 1);
    }
  };

  if (!ivl->wide) {
    for (size_t slot = 0, n = spill_slots.size(); slot < n; ++slot) {
      if (leader->start() >= spill_slots[slot]) {
        return assign_slot(slot);
      }
    }
  } else {
    for (size_t slot = 0, n = spill_slots.size() - 1; slot < n; slot += 2) {
      if (leader->start() >= spill_slots[slot] &&
          leader->start() >= spill_slots[slot + 1]) {
        return assign_slot(slot);
      }
    }
  }

  // Ran out of spill slots.
  ONTRACE(kRegAllocLevel, dumpIntervals());
  TRACE(1, "vxls-punt TooManySpills\n");
  PUNT(LinearScan_TooManySpills);
}

// Visitor class for renaming registers. Visit every virtual-register
// typed operand, and rename it to its assigned physical register.
struct Renamer {
  Renamer(Vxls& xls, unsigned pos) : xls(xls), pos(pos) {}
  template<class T> void imm(const T& r) {}
  template<class R> void def(R& r) { rename(r); }
  template<class D, class H> void defHint(D& dst, H) { rename(dst); }
  void use(VcallArgsId) { always_assert(false && "vcall unsupported in vxls"); }
  template<class R> void use(R& r) { rename(r); }
  template<class S, class H> void useHint(S& src, H) { rename(src); }
  template<class R> void across(R& r) { rename(r); }
  void def(RegSet) {}
  void use(Vptr& m) {
    if (m.base.isValid()) rename(m.base);
    if (m.index.isValid()) rename(m.index);
  }
  void use(RegSet r){}
private:
  void rename(Vreg8& r) { r = lookup(r, VregKind::Gpr); }
  void rename(Vreg16& r) { r = lookup(r, VregKind::Gpr); }
  void rename(Vreg32& r) { r = lookup(r, VregKind::Gpr); }
  void rename(Vreg64& r) { r = lookup(r, VregKind::Gpr); }
  void rename(VregDbl& r) { r = lookup(r, VregKind::Simd); }
  void rename(Vreg128& r) { r = lookup(r, VregKind::Simd); }
  void rename(VregSF& r) { r = lookup(r, VregKind::Sf); }
  void rename(Vreg& r) { r = lookup(r, VregKind::Any); }
  void rename(Vtuple t) { /* phijmp/phijcc+phidef handled by resolveEdges */ }
  PhysReg lookup(Vreg vreg, VregKind kind) {
    auto ivl = xls.intervals[vreg];
    if (!ivl || vreg.isPhys()) return vreg;
    PhysReg reg = ivl->childAt(pos)->reg;
    assertx((kind == VregKind::Gpr && reg.isGP()) ||
           (kind == VregKind::Simd && reg.isSIMD()) ||
           (kind == VregKind::Sf && reg.isSF()) ||
           (kind == VregKind::Any && reg != InvalidReg));
    return reg;
  }
private:
  Vxls& xls;
  unsigned pos;
};

void Vxls::renameOperands() {
  for (auto b : blocks) {
    auto pos = block_ranges[b].start;
    for (auto& inst : unit.blocks[b].code) {
      Renamer renamer(*this, pos);
      visitOperands(inst, renamer);
      pos += 2;
    }
  }
  ONTRACE(kRegAllocLevel, print("after renaming operands"));
}

// Insert spills and copies that connect sub-intervals that were split
// between instructions. Do not assume SSA; insert a spill-store
// after every def, ignoring the interval's full live range.
void Vxls::resolveSplits() {
  ONTRACE(kRegAllocLevel, dumpIntervals());
  for (auto i1 : intervals) {
    if (!i1) continue;
    auto slot = i1->slot;
    if (slot >= 0) insertSpill(i1);
    for (auto i2 = i1->next; i2; i1 = i2, i2 = i2->next) {
      auto pos = i2->start();
      if (i1->end() != pos) continue; // spans lifetime hole
      if (i2->reg == InvalidReg) continue; // no load necessary
      if (i2->reg == i1->reg) continue; // no copy necessary
      auto b = findBlock(pos);
      auto range = block_ranges[b];
      if (pos % 2 == 0) {
        // even position requiring a copy must be on edge
        assertx(range.start == pos);
      } else {
        // odd position
        assertx(pos > range.start); // implicit label position per block
        if (pos + 1 == range.end) continue; // copy belongs on successor edge
        if (i2->reg != i1->reg) {
          copies[pos][i2->reg] = i1;
        }
      }
    }
  }
}

// Insert a spill after the def-position in ivl.
void Vxls::insertSpill(Interval* ivl) {
  auto DEBUG_ONLY checkPos = [&](unsigned pos) {
    assertx(pos % 2 == 1);
    DEBUG_ONLY auto b = findBlock(pos);
    DEBUG_ONLY auto range = block_ranges[b];
    assertx(pos - 1 >= range.start && pos + 1 < range.end);
    return true;
  };
  auto pos = ivl->def_pos + 1;
  assertx(checkPos(pos));
  spills[pos][ivl->reg] = ivl; // store ivl->reg => ivl->slot
}

// Lower copyargs into moveplans at the copyargs position.
void Vxls::lowerCopyargs() {
  for (auto b : blocks) {
    auto pos = block_ranges[b].start;
    for (auto& inst : unit.blocks[b].code) {
      if (inst.op == Vinstr::copyargs) {
        auto& uses = unit.tuples[inst.copyargs_.s];
        auto& defs = unit.tuples[inst.copyargs_.d];
        for (unsigned i = 0, n = uses.size(); i < n; ++i) {
          auto i1 = intervals[uses[i]];
          if (i1 && !i1->fixed()) i1 = i1->childAt(pos);
          auto i2 = intervals[defs[i]];
          if (i2->reg != i1->reg) {
            assertx(!copies[pos][i2->reg]);
            copies[pos][i2->reg] = i1;
          }
        }
      }
      pos += 2;
    }
  }
}

/*
 * Search for the phidef in block `b', then return its dest tuple.
 */
static Vtuple findDefs(const Vunit& unit, Vlabel b) {
  assertx(!unit.blocks[b].code.empty() &&
         unit.blocks[b].code.front().op == Vinstr::phidef);
  return unit.blocks[b].code.front().phidef_.defs;
}

void Vxls::resolveEdges() {
  auto addEdgeCopiesForPhiUseDefIntervalConflicts = [&](
    Vlabel block, Vlabel target, uint32_t targetIndex, const VregList& uses) {
    auto p1 = block_ranges[block].end - 2;
    auto& defs = unit.tuples[findDefs(unit, target)];
    for (unsigned i = 0, n = uses.size(); i < n; ++i) {
      auto i1 = intervals[uses[i]];
      if (i1 && !i1->fixed()) i1 = i1->childAt(p1);
      auto i2 = intervals[defs[i]];
      if (i2->reg != i1->reg) {
        assertx((edge_copies[{block,targetIndex}][i2->reg] == nullptr));
        edge_copies[{block,targetIndex}][i2->reg] = i1;
      }
    }
  };
  for (auto b1 : blocks) {
    auto& block1 = unit.blocks[b1];
    auto p1 = block_ranges[b1].end - 2;
    auto succlist = succs(block1);
    auto& inst1 = block1.code.back();
    if (inst1.op == Vinstr::phijmp) {
      auto target = inst1.phijmp_.target;
      auto& uses = unit.tuples[inst1.phijmp_.uses];
      addEdgeCopiesForPhiUseDefIntervalConflicts(b1, target, 0, uses);

      auto tmp_pos = inst1.pos;
      inst1 = jmp{target};
      inst1.pos = tmp_pos;
    } else if (inst1.op == Vinstr::phijcc) {
      auto& uses = unit.tuples[inst1.phijcc_.uses];
      uint32_t i = 0;
      for (auto target : succs(inst1)) {
        addEdgeCopiesForPhiUseDefIntervalConflicts(b1, target, i, uses);
        i += 1;
      }

      auto& targets = inst1.phijcc_.targets;
      auto tmp_pos = inst1.pos;
      inst1 = jcc{inst1.phijcc_.cc, inst1.phijcc_.sf, {targets[0], targets[1]}};
      inst1.pos = tmp_pos;
    }
    for (unsigned i = 0, n = succlist.size(); i < n; i++) {
      auto b2 = succlist[i];
      auto p2 = block_ranges[b2].start;
      forEach(livein[b2], [&](Vreg vr) {
        auto ivl = intervals[vr];
        if (ivl->fixed()) return;
        Interval* i1 = nullptr;
        Interval* i2 = nullptr;
        for (auto ivl = intervals[vr]; ivl && !(i1 && i2); ivl = ivl->next) {
          if (ivl->covers(p1)) i1 = ivl;
          if (ivl->covers(p2)) i2 = ivl;
        }
        // i2 can be unallocated if the tmp is a constant or is spilled.
        if (i2->reg != InvalidReg && i2->reg != i1->reg) {
          assertx((edge_copies[{b1,i}][i2->reg] == nullptr));
          edge_copies[{b1,i}][i2->reg] = i1;
        }
      });
    }
  }
}

// last phase: mutate the code by inserting copies. this destroyes
// the position numbering, so we can't use interval positions after this.
void Vxls::insertCopies() {
  // insert copies inside blocks
  for (auto b : blocks) {
    auto pos = block_ranges[b].start;
    auto& block = unit.blocks[b];
    auto& code = block.code;
    auto offset = spill_offsets[b];
    for (unsigned j = 0; j < code.size(); j++, pos += 2) {
      MemoryRef slots = m_sp[offset];
      offset -= spEffect(code[j]);
      auto s = spills.find(pos - 1);
      if (s != spills.end()) {
        insertSpillsAt(code, j, s->second, slots, pos - 1);
      }
      auto c = copies.find(pos - 1);
      if (c != copies.end()) {
        insertCopiesAt(code, j, c->second, slots, pos - 1);
      }
      c = copies.find(pos);
      if (c != copies.end()) {
        insertCopiesAt(code, j, c->second, slots, pos);
      }
    }
  }
  // insert copies on edges
  for (auto b : blocks) {
    auto& block = unit.blocks[b];
    auto succlist = succs(block);
    if (succlist.size() == 1) {
      // copies will go at end of b
      auto& code = block.code;
      auto c = edge_copies.find({b, 0});
      if (c != edge_copies.end()) {
        unsigned j = code.size() - 1;
        auto slots = m_sp[spill_offsets[succlist[0]]];
        insertCopiesAt(code, j, c->second, slots, block_ranges[b].end - 1);
      }
    } else {
      // copies will go at start of successor
      for (int i = 0, n = succlist.size(); i < n; i++) {
        auto s = succlist[i];
        auto c = edge_copies.find({b, i});
        if (c != edge_copies.end()) {
          auto slots = m_sp[spill_offsets[s]];
          auto& code = unit.blocks[s].code;
          unsigned j = 0;
          insertCopiesAt(code, j, c->second, slots, block_ranges[s].start);
        }
      }
    }
  }
  ONTRACE(kRegAllocLevel,
    dumpIntervals();
    print("after inserting copies");
  );
}

/*
 * SpillState is used by allocateSpillSpace() to decide where to allocate/free
 * spill space. It represents the state of the spill space as a whole and is
 * computed before each individual instruction.
 *
 * Order is important in this enum: it's only legal to transition to states
 * with higher values, and states are merged using std::max().
 */
enum SpillState : uint8_t {
  // State is uninitialized. All block in-states start here.
  Uninit,

  // Spill space is not currently needed; it's safe to allocate spill space
  // after this point.
  NoSpill,

  // Spill space is needed and must be allocated at or before this point.
  NeedSpill,
};

/*
 * SpillStates is used to hold in/out state for each block after the analysis
 * pass of allocateSpillSpace().
 */
struct SpillStates {
  SpillState in;
  SpillState out;
};

/*
 * Returns true if spill space must be allocated before execution of this
 * instruction. In order to keep things simple, we return true for any
 * instruction that reads or writes sp.
 */
bool instrNeedsSpill(const Vunit& unit, const Vinstr& inst, PhysReg sp) {
  // Implicit sp input/output.
  if (inst.op == Vinstr::push || inst.op == Vinstr::pop) return true;

  auto foundSp = false;
  visitDefs(unit, inst, [&](Vreg r) { if (r == sp) foundSp = true; });
  if (foundSp) return true;

  visitUses(unit, inst, [&](Vreg r) { if (r == sp) foundSp = true; });
  return foundSp;
}

/*
 * Return the required SpillState coming into inst. prevState must not be
 * Uninit.
 */
SpillState instrInState(const Vunit& unit, const Vinstr& inst,
                        SpillState prevState, PhysReg sp) {
  switch (prevState) {
    case Uninit: break;

    case NoSpill:
      if (instrNeedsSpill(unit, inst, sp)) return NeedSpill;
      return NoSpill;

    case NeedSpill:
      return NeedSpill;
  }

  always_assert(false);
}

/*
 * processSpillExits() can insert jcc{} instructions in the middle of a
 * block. fixupBlockJumps() breaks the given block after any jccs, making the
 * unit valid again. This is done as a separate pass from the work in
 * processSpillExits() to reduce complexity.
 */
void fixupBlockJumps(Vunit& unit, Vlabel label) {
  jit::vector<Vinstr> origCode;
  origCode.swap(unit.blocks[label].code);
  auto insertBlock = [&]() -> Vblock& { return unit.blocks[label]; };

  for (auto const& inst : origCode) {
    insertBlock().code.emplace_back(inst);

    if (inst.op == Vinstr::jcc && !inst.jcc_.targets[0].isValid()) {
      auto newLabel = unit.makeBlock(insertBlock().area);
      insertBlock().code.back().jcc_.targets[0] = newLabel;
      label = newLabel;
    }
  }
}

/*
 * Walk through the given block, undoing any fallbackcc/bindjcc optimizations
 * that happen in an area where spill space is live. The latter transformation
 * is necessary to make the hidden edge out of the fallbackcc{} explicit, so we
 * can insert an lea on it to free spill space. It takes something like this:
 *
 * B0:
 *   cmpbim 0, %rbp[0x10] => %flags
 *   fallbackcc CC_E, %flags, <SrcKey>
 *   ...
 *
 * and turns it into something like this:
 *
 * B0:
 *   cmpbim 0, %rbp[0x10] => %flags
 *   jcc CC_E, %flags -> B3, else B2
 * B2:
 *   ...
 * B3:
 *   lea %rsp[0x20] => %rsp
 *   fallback <SrcKey>
 */
void processSpillExits(Vunit& unit, Vlabel label, SpillState state,
                       Vinstr free, PhysReg sp) {
  auto code = &unit.blocks[label].code;
  auto needFixup = false;

  for (unsigned i = 0, n = code->size(); i < n; ++i) {
    auto& inst = (*code)[i];
    state = instrInState(unit, inst, state, sp);

    if (state < NeedSpill ||
        (inst.op != Vinstr::fallbackcc && inst.op != Vinstr::bindjcc)) {
      continue;
    }

    FTRACE(3, "Breaking out {}: {}\n", label, show(unit, inst));
    auto target = unit.makeBlock(AreaIndex::Cold);
    // makeBlock might reallocate unit.blocks
    code = &unit.blocks[label].code;

    auto& targetCode = unit.blocks[target].code;
    free.origin = inst.origin;
    ConditionCode cc;
    Vreg sf;
    if (inst.op == Vinstr::fallbackcc) {
      auto const& fb_i = inst.fallbackcc_;
      targetCode.emplace_back(free);
      targetCode.emplace_back(fallback{fb_i.dest, fb_i.trflags, fb_i.args});
      cc = fb_i.cc;
      sf = fb_i.sf;
    } else {
      auto const& bj_i = inst.bindjcc_;
      targetCode.emplace_back(free);
      targetCode.emplace_back(bindjmp{bj_i.target, bj_i.trflags, bj_i.args});
      cc = bj_i.cc;
      sf = bj_i.sf;
    }
    targetCode.back().origin = inst.origin;

    // Next is set to an invalid block that will be fixed up once we're done
    // iterating through the original block.
    inst = jcc{cc, sf, {Vlabel{}, target}};
    needFixup = true;
  }

  if (needFixup) fixupBlockJumps(unit, label);
}

/*
 * Merge src into dst, returning true iff dst was changed.
 */
bool mergeSpillStates(SpillState& dst, SpillState src) {
  assertx(src != Uninit);
  if (dst == src) return false;

  // The only allowed state transitions are to states with higher values, so we
  // merge with std::max().
  auto const oldDst = dst;
  dst = std::max(dst, src);
  return dst != oldDst;
}

std::default_random_engine s_stressRand(0xfaceb00c);
std::uniform_int_distribution<int> s_stressDist(1,7);

/*
 * If the current unit used any spill slots, allocate and free spill space
 * where appropriate. Spill space is allocated right before it's needed and
 * freed before any instruction that exits the unit, which is any block-ending
 * instruction with no successors in the unit. fallbackcc{} and bindjcc{}
 * instructions have hidden edges that exit the unit, and if we encounter one
 * while spill space is live, we have to make that exit edge explicit to insert
 * code on it (see processSpillExits()). This makes the exit path more
 * expensive, so we try to allocate spill space as late as possible to avoid
 * pessimising fallbackcc/bindjcc instructions unless it's really
 * necessary. The algorithm uses two passes:
 *
 * Analysis:
 *   - For each block in RPO:
 *     - Load in-state, which has been populated by at least one predecessor
 *       (or manually set to NoSpill for the entry block).
 *     - Analyze each instruction in the block, determining what state the
 *       spill space must be in before executing it.
 *     - Record out-state for the block and propagate to successors. If this
 *       changes the in-state for any of them, enqueue them for (re)processing.
 *
 * Mutation:
 *   - For each block (we use RPO to only visit reachable blocks but order
 *     doesn't matter):
 *     - Inspect the block's in-state and out-state:
 *       - NoSpill in, == NeedSpill out: Walk the block to see if we need to
 *         allocate spill space before any instructions.
 *       - NoSpill out: Allocate spill space on any edges to successors with
 *         NeedSpill in-states.
 *       - NeedSpill out: If the block has no in-unit successors, free spill
 *         space before the block-end instruction.
 *       - != NoSpill out: Look for any fallbackcc/bindjcc instructions,
 *         deoptimizing as appropriate (see processSpillExits()).
 */
void Vxls::allocateSpillSpace() {
  if (RuntimeOption::EvalHHIRStressSpill && m_abi.canSpill) {
    auto extra = s_stressDist(s_stressRand);
    FTRACE(1, "StressSpill on; adding {} extra slots\n", extra);
    used_spill_slots += extra;
  }
  if (used_spill_slots == 0) return;
  Timer t(Timer::vasm_xls_spill);
  always_assert(m_abi.canSpill);

  // Make sure we always allocate spill space in multiples of 16 bytes, to keep
  // alignment straightforward.
  if (used_spill_slots % 2) used_spill_slots++;
  FTRACE(1, "Allocating {} spill slots\n", used_spill_slots);

  auto const spillSize = safe_cast<int32_t>(slotOffset(used_spill_slots));
  // Pointer manipulation is traditionally done with lea, and it's safe to
  // insert even where flags might be live.
  Vinstr alloc = lea{m_sp[-spillSize], m_sp};
  Vinstr free  = lea{m_sp[spillSize], m_sp};

  jit::vector<uint32_t> rpoIds(unit.blocks.size());
  for (uint32_t i = 0; i < blocks.size(); ++i) rpoIds[blocks[i]] = i;

  jit::vector<SpillStates> states(unit.blocks.size(), {Uninit, Uninit});
  states[unit.entry].in = NoSpill;
  dataflow_worklist<uint32_t> worklist(unit.blocks.size());
  worklist.push(0);

  // Walk the blocks in rpo. At the end of each block, propagate its out-state
  // to successors, adding them to the worklist if their in-state
  // changes. Blocks may be visited multiple times if loops are present.
  while (!worklist.empty()) {
    auto const label  = blocks[worklist.pop()];
    auto const& block = unit.blocks[label];
    auto state = states[label].in;

    if (state < NeedSpill) {
      for (auto& inst : block.code) {
        state = instrInState(unit, inst, state, m_sp);
        if (state == NeedSpill) break;
      }
    }
    states[label].out = state;

    for (auto s : succs(block)) {
      if (mergeSpillStates(states[s].in, state)) {
        worklist.push(rpoIds[s]);
      }
    }
  }

  // Do a single mutation pass over the blocks.
  for (auto const label : blocks) {
    auto state = states[label];
    auto& block = unit.blocks[label];

    // Any block with a NoSpill in-state and == NeedSpill out-state might have
    // an instruction in it that needs spill space, which we allocate right
    // before the instruction in question.
    if (state.in == NoSpill && state.out == NeedSpill) {
      auto state = NoSpill;
      for (auto it = block.code.begin(); it != block.code.end(); ++it) {
        state = instrInState(unit, *it, state, m_sp);
        if (state == NeedSpill) {
          FTRACE(3, "alloc spill before {}: {}\n", label, show(unit, *it));
          alloc.origin = it->origin;
          block.code.insert(it, alloc);
          break;
        }
      }
    }

    // Allocate spill space on edges from a NoSpill out-state to a NeedSpill
    // in-state.
    auto const successors = succs(block);
    if (state.out == NoSpill) {
      for (auto s : successors) {
        if (states[s].in == NeedSpill) {
          FTRACE(3, "alloc spill on edge from {} -> {}\n", label, s);
          auto it = std::prev(block.code.end());
          alloc.origin = it->origin;
          block.code.insert(it, alloc);
        }
      }
    }

    // Any block with a NeedSpill out-state and no successors must free spill
    // space right before the block-end instruction. We ignore ud2 so spill
    // space is still allocated in core files.
    if (state.out == NeedSpill && successors.empty() &&
        block.code.back().op != Vinstr::ud2) {
      auto it = std::prev(block.code.end());
      FTRACE(3, "free spill before {}: {}\n", label, show(unit, (*it)));
      free.origin = it->origin;
      block.code.insert(it, free);
    }

    // Any block that ends with anything other than NoSpill needs to be walked
    // to look for places to free spill space.
    if (state.out != NoSpill) {
      processSpillExits(unit, label, state.in, free, m_sp);
    }
  }
}

void Vxls::insertSpillsAt(jit::vector<Vinstr>& code, unsigned& j,
                          const CopyPlan& spills, MemoryRef slots,
                          unsigned pos) {
  jit::vector<Vinstr> stores;
  for (auto src : spills) {
    auto ivl = spills[src];
    if (!ivl) continue;
    auto slot = ivl->leader()->slot;
    assertx(slot >= 0 && src == ivl->reg);
    MemoryRef ptr{slots.r + slotOffset(slot)};
    if (!ivl->wide) {
      always_assert_flog(
        !src.isSF(),
        "Tried to spill %flags in Vunit:\n\n{}",
        show(unit)
      );
      stores.emplace_back(store{src, ptr});
    } else {
      assertx(src.isSIMD());
      stores.emplace_back(storedqu{src, ptr});
    }
  }
  auto origin = code[j].origin;
  code.insert(code.begin() + j, stores.size(), ud2{});
  for (auto& inst : stores) {
    code[j] = inst;
    code[j].origin = origin;
    code[j++].pos = pos;
  }
}

void Vxls::insertCopiesAt(jit::vector<Vinstr>& code, unsigned& j,
                          const CopyPlan& copies, MemoryRef slots,
                          unsigned pos) {
  MovePlan moves;
  jit::vector<Vinstr> loads;
  for (auto dst : copies) {
    auto ivl = copies[dst];
    if (!ivl) continue;
    if (ivl->reg != InvalidReg) {
      moves[dst] = ivl->reg;
    } else if (ivl->constant) {
      switch (ivl->val.kind) {
        case Vconst::Quad:
          loads.emplace_back(ldimmq{ivl->val.val, dst, true});
          break;
        case Vconst::Long:
          loads.emplace_back(ldimml{int32_t(ivl->val.val), dst, true});
          break;
        case Vconst::Byte:
          loads.emplace_back(ldimmb{uint8_t(ivl->val.val), dst, true});
          break;
        case Vconst::ThreadLocal:
          loads.emplace_back(
            load{Vptr{baseless(ivl->val.disp), Vptr::FS}, dst}
          );
          break;
      }
    } else {
      assertx(ivl->spilled());
      MemoryRef ptr{slots.r + slotOffset(ivl->slot)};
      if (!ivl->wide) {
        loads.emplace_back(load{ptr, dst});
      } else {
        assertx(dst.isSIMD());
        loads.emplace_back(loaddqu{ptr, dst});
      }
    }
  }
  auto origin = code[j].origin;
  auto hows = doRegMoves(moves, m_tmp);
  auto count = hows.size() + loads.size();
  code.insert(code.begin() + j, count, ud2{});
  for (auto& how : hows) {
    if (how.m_kind == MoveInfo::Kind::Xchg) {
      code[j] = copy2{how.m_src, how.m_dst, how.m_dst, how.m_src};
    } else {
      code[j] = copy{how.m_src, how.m_dst};
    }
    code[j].origin = origin;
    code[j++].pos = pos;
  }
  for (auto& inst : loads) {
    code[j] = inst;
    code[j].origin = origin;
    code[j++].pos = pos;
  }
}

// Return the [start,end) range for the block enclosing pos.
Vlabel Vxls::findBlock(unsigned pos) {
  for (unsigned lo = 0, hi = blocks.size(); lo < hi;) {
    auto mid = (lo + hi) / 2;
    auto r = block_ranges[blocks[mid]];
    if (pos < r.start) {
      hi = mid;
    } else if (pos >= r.end) {
      lo = mid + 1;
    } else {
      return blocks[mid];
    }
  }
  always_assert(false);
  return Vlabel{unit.blocks.size()};
}

LiveRange Vxls::findBlockRange(unsigned pos) {
  return block_ranges[findBlock(pos)];
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
  auto d = pos % 2 == 1 ? s : f(pos + 1);
  return ( s && !d) ? top[m] :
         ( s &&  d) ? both[m] :
         (!s &&  d) ? bottom[m] :
         empty[m];
}

std::string Interval::toString() {
  std::ostringstream out;
  auto delim = "";
  if (reg != InvalidReg) {
    out << show(reg);
    delim = " ";
  }
  if (constant) {
    out << delim << folly::format("#{:08x}", val.val);
  }
  if (slot >= 0) {
    out << delim << folly::format("[%sp+{}]", slotOffset(slot));
  }
  delim = "";
  out << " [";
  for (auto& r : ranges) {
    out << delim << folly::format("{}-{}", r.start, r.end);
    delim = ",";
  }
  out << ") {";
  delim = "";
  for (auto& u : uses) {
    if (u.pos == def_pos) {
      out << delim << "@" << u.pos << "=";
      if (u.hint.isValid()) out << show(u.hint);
    } else {
      if (u.hint.isValid()) {
        out << delim << show(u.hint) << "=@" << u.pos;
      } else {
        out << delim << "=@" << u.pos;
      }
    }
    delim = ",";
  }
  out << "}";
  return out.str();
}

void Vxls::dumpIntervals() {
Trace::traceRelease("Spills %u\n", num_spills);
  for (auto ivl : intervals) {
    if (!ivl || ivl->fixed()) continue;
    Trace::traceRelease("%%%-2lu %s\n", size_t(ivl->vreg),
                              ivl->toString().c_str());
    for (ivl = ivl->next; ivl; ivl = ivl->next) {
      Trace::traceRelease("    %s\n", ivl->toString().c_str());
    }
  }
}

auto const ignore_reserved = !getenv("XLS_SHOW_RESERVED");
auto const collapse_fixed = !getenv("XLS_SHOW_FIXED");

void Vxls::printInstr(std::ostringstream& str, Vinstr* inst, unsigned pos,
                      Vlabel b) {
  bool fixed_covers[2] = { false, false };
  Interval* fixed = nullptr;
  for (auto ivl : intervals) {
    if (!ivl) continue;
    if (ivl->fixed()) {
      if (ignore_reserved && !m_abi.unreserved().contains(ivl->vreg)) {
        continue;
      }
      if (collapse_fixed) {
        fixed = ivl; // can be any.
        fixed_covers[0] |= ivl->covers(pos);
        fixed_covers[1] |= ivl->covers(pos + 1);
        continue;
      }
    }
    str << " ";
    str << draw(ivl, pos, Light, [&](Interval* child, unsigned p) {
      return child->covers(p);
    });
    str << draw(ivl, pos, Heavy, [&](Interval* child, unsigned p) {
      return child->usedAt(p);
    });
  }
  str << " " << draw(fixed, pos, Heavy, [&](Interval*, unsigned p) {
    assertx(p-pos < 2);
    return fixed_covers[p-pos];
  });
  if (pos == block_ranges[b].start) {
    str << folly::format(" B{: <3}", size_t(b));
  } else {
    str << "     ";
  }
  str << folly::format(" {: <3} ", pos) << show(unit, *inst) << "\n";
}

void Vxls::print(const char* caption) {
  std::ostringstream str;
  str << "Intervals " << caption << " " << s_counter << "\n";
  for (auto ivl : intervals) {
    if (!ivl) continue;
    if (ivl->fixed()) {
      if (ignore_reserved && !m_abi.unreserved().contains(ivl->vreg)) {
        continue;
      }
      if (collapse_fixed) {
        continue;
      }
    }
    str << folly::format(" {: <2}", size_t(ivl->vreg));
  }
  str << " FX\n";
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      printInstr(str, &inst, inst.pos, b);
    }
  }
  HPHP::Trace::traceRelease("%s\n", str.str().c_str());
}

///////////////////////////////////////////////////////////////////////////////
}

void allocateRegisters(Vunit& unit, const Abi& abi) {
  s_counter++;
  Vxls a(unit, abi);
  a.allocate();
}

///////////////////////////////////////////////////////////////////////////////
}}
