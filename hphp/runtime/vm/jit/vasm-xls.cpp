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

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/base/arch.h"
#include "hphp/util/assertions.h"
#include "hphp/util/ringbuffer.h"

#include <boost/dynamic_bitset.hpp>
#include <algorithm>

// future work
//  - #3098509 streamline code, vectors vs linked lists, etc
//  - #3409409 Enable use of SIMD at least for doubles, for packed_tv
//  - #3098685 Optimize lifetime splitting
//  - #3098712 reuse spill slots
//  - #3098739 new features now possible with XLS

TRACE_SET_MOD(xls);

namespace HPHP { namespace jit {
using Trace::RingBufferType;
using Trace::ringbufferName;

namespace {
using namespace Stats;

size_t s_counter;

// Sort blocks in reverse postorder, and try to arrange fall-through
// blocks in the same area to be close together.
struct BlockSorter {
  explicit BlockSorter(const Vunit& unit)
    : unit(unit)
    , visited(unit.blocks.size()) {
    blocks.reserve(unit.blocks.size());
  }
  unsigned area(Vlabel b) {
    return (unsigned)unit.blocks[b].area;
  }
  void dfs(Vlabel b) {
    assert(size_t(b) < unit.blocks.size() && !unit.blocks[b].code.empty());
    if (visited.test(b)) return;
    visited.set(b);
    if (area(b) == 0) {
      for (auto s : succs(unit.blocks[b])) {
        // visit colder
        if (area(s) > area(b)) dfs(s);
      }
      for (auto s : succs(unit.blocks[b])) {
        if (area(s) <= area(b)) dfs(s);
      }
    } else {
      for (auto s : succs(unit.blocks[b])) dfs(s);
    }
    blocks.push_back(b);
  }
  const Vunit& unit;
  jit::vector<Vlabel> blocks;
  boost::dynamic_bitset<> visited;
};

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
  bool contains(LiveRange r) const;
public:
  unsigned start, end;
};

typedef jit::vector<LiveRange> RangeList;
typedef jit::vector<Use> UseList;

// An Interval stores the lifetime of an Vreg as a sorted list of disjoint
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
    , cns(parent->cns)
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
  int slot{-1};
  bool wide{false};
  PhysReg reg;
  bool cns{false};
  Vunit::Cns val;
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
    , m_sp(mcg->backEnd().rSp()) {
    switch (arch()) {
      case Arch::X64:
        m_tmp = reg::xmm15; // reserve xmm15 to break shuffle cycles
        m_srkill = RegSet(x64::rAsm);
        break;
      case Arch::ARM:
        m_tmp = vixl::x17; // also used as tmp1 by MacroAssembler
        m_srkill = RegSet(arm::rAsm);//m_abi.all();
        break;
    }
    m_abi.simdUnreserved.remove(m_tmp);
    m_abi.simdReserved.add(m_tmp);
    assert(!m_abi.gpUnreserved.contains(m_sp));
    assert(!m_abi.gpUnreserved.contains(m_tmp));
  }
  ~Vxls();
  // phases
  void allocate();
  void splitCritEdges();
  void computePositions();
  void analyzeRsp();
  void buildIntervals();
  void walkIntervals();
  void renameOperands();
  void resolveSplits();
  void lowerCopyargs();
  void resolveEdges();
  void insertCopies();
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
  void forwardJmp(Vlabel middleLabel, Vlabel destLabel);
  // debugging
  void print(const char* caption);
  void printInstr(std::ostringstream& out, Vinstr* instr, unsigned pos, Vlabel);
  void dumpIntervals();
  int spEffect(Vinstr& inst) const;
  void getEffects(const Vinstr& i, RegSet& uses, RegSet& across, RegSet& defs);
public:
  struct Compare { bool operator()(const Interval*, const Interval*); };
public:
  Vunit& unit;
  Abi m_abi;
  PhysReg m_sp; // arch-dependent stack pointer
  PhysReg m_tmp; // only for breaking cycles
  RegSet m_srkill; // killed by service requests
  jit::vector<Vlabel> blocks;          // sorted blocks
  jit::vector<LiveRange> block_ranges; // [start,end) position of each block
  jit::vector<int> spill_offsets;      // per-block sp[offset] to spill-slots
  jit::vector<LiveSet> livein;         // per-block live-in sets
  jit::vector<Interval*> intervals;    // parent intervals, null if unused
  jit::priority_queue<Interval*,Compare> pending; // sorted by Interval start
  jit::vector<Interval*> active, inactive; // intervals that overlap
  jit::hash_map<unsigned,CopyPlan> copies; // where to insert copies
  jit::hash_map<unsigned,CopyPlan> spills; // where to insert spills
  jit::hash_map<EdgeKey,CopyPlan,EdgeHasher> edge_copies; // copies on edges
  unsigned m_nextSlot{0}; // next available spill slot
};

//////////////////////////////////////////////////////////////////////////////

const unsigned kMaxPos = UINT_MAX; // "infinity" use position

// comparison function for pending priority queue. priority_queue
// requires a less operation, but sorts the heap highest-first; we
// need the opposite (lowest-first), so use greater-than.
bool Vxls::Compare::operator()(const Interval* i1, const Interval* i2) {
  return i1->start() > i2->start();
}

//////////////////////////////////////////////////////////////////////////////

// returns true if this range contains r
bool LiveRange::contains(LiveRange r) const {
  return r.start >= start && r.end <= end;
}

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
  assert(lo == ranges.size() || pos < ranges[lo].start);
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
  assert(lo == uses.size() || pos < uses[lo].pos);
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
  assert(!parent);
  for (auto ivl = this; ivl; ivl = ivl->next) {
    if (pos < ivl->start()) return nullptr;
    if (ivl->usedAt(pos)) return ivl;
  }
  return nullptr;
}

// Return the next intersection point between current and other, or kMaxPos
// if they never intersect.
unsigned nextIntersect(Interval* current, Interval* other) {
  assert(!current->fixed());
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
  assert(pos > start() && pos < end()); // both parts will be non-empty
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

bool is_nop(copy& i) { return i.s == i.d; }
bool is_nop(copy2& i) { return i.s0 == i.d0 && i.s1 == i.d1; }

bool is_nop(lea& i) {
  return i.s.disp == 0 && (
         (i.s.base == i.d && !i.s.index.isValid()) ||
         (!i.s.base.isValid() && i.s.index == i.d && i.s.scale == 1));
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
 * visit every instruction and modify its Vreg operands to the physical
 * register that was assigned.
 *
 * 6. Splitting creates sub-intervals that are assigned to different registers
 * or spill locations, so insert resolving copies at the split positions
 * between intervals that were split in a block, and copies on control-flow
 * edges connecting different sub-intervals. When more than one copy occurs
 * in a position, they are parallel-copies (all sources read before any dest
 * is written).
 *
 * If any sub-interval was spilled, we a single store is generated after each
 * definition point.
 */

void Vxls::allocate() {
  blocks = sortBlocks(unit);
  assert(check(unit));
  splitCritEdges();
  computePositions();
  analyzeRsp();
  buildIntervals();
  walkIntervals();
  resolveSplits();
  lowerCopyargs();
  resolveEdges();
  renameOperands();
  insertCopies();
  // we're completely done. remove nop-copies
  for (auto b : blocks) {
    auto& code = unit.blocks[b].code;
    auto end = std::remove_if(code.begin(), code.end(), [&](Vinstr& inst) {
      return (inst.op == Vinstr::copy && is_nop(inst.copy_)) ||
             (inst.op == Vinstr::copy2 && is_nop(inst.copy2_)) ||
             (inst.op == Vinstr::lea && is_nop(inst.lea_)) ||
             (inst.op == Vinstr::copyargs) || // we lowered it
             (inst.op == Vinstr::nop) || // we inserted it
             (inst.op == Vinstr::phidef); // we lowered it
    });
    code.erase(end, code.end());
  }
  printUnit(kVasmRegAllocLevel, "after vasm-xls", unit);
}

void Vxls::forwardJmp(Vlabel middleLabel, Vlabel destLabel) {
  auto& middle = unit.blocks[middleLabel];
  auto& dest = unit.blocks[destLabel];
  // We need to preserve any phidefs in the forwarding block if they're present
  // in the original destination block.
  auto& headInst = dest.code.front();
  if (headInst.op == Vinstr::phidef) {
    auto tuple = headInst.phidef_.defs;
    auto forwardedRegs = unit.tuples[tuple];
    VregList regs(forwardedRegs.size());
    for (unsigned i = 0; i < forwardedRegs.size(); ++i) {
      regs[i] = unit.makeReg();
    }
    auto newTuple = unit.makeTuple(regs);
    middle.code.emplace_back(phidef{newTuple});
    middle.code.emplace_back(phijmp{destLabel, newTuple});
    return;
  }
  middle.code.emplace_back(jmp{destLabel});
}

void Vxls::splitCritEdges() {
  jit::vector<unsigned> preds(unit.blocks.size());
  for (auto pred : blocks) {
    auto succlist = succs(unit.blocks[pred]);
    for (auto succ : succlist) {
      preds[succ]++;
    }
  }
  auto resort = false;
  for (auto pred : blocks) {
    auto succlist = succs(unit.blocks[pred]);
    if (succlist.size() <= 1) continue;
    for (auto& succ : succlist) {
      if (preds[succ] <= 1) continue;
      // split the critical edge.
      auto middle = unit.makeBlock(unit.blocks[succ].area);
      forwardJmp(middle, succ);
      succ = middle;
      resort = true;
    }
  }
  if (resort) {
    blocks = sortBlocks(unit);
  }
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
      code.insert(code.begin(), nop{});
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
      if (debug) visitDefs(unit, inst, [&](Vreg r) { assert(r != m_sp); });
      return 0;
    case Vinstr::push:
    case Vinstr::pushm:
      return -8;
    case Vinstr::pop:
    case Vinstr::popm:
      return 8;
    case Vinstr::addqi: {
      auto& i = inst.addqi_;
      if (i.d == Vreg64(m_sp)) {
        assert(i.s1 == Vreg64(m_sp));
        return i.s0.l();
      }
      return 0;
    }
    case Vinstr::subqi: {
      auto& i = inst.subqi_;
      if (i.d == Vreg64(m_sp)) {
        assert(i.s1 == Vreg64(m_sp));
        return -i.s0.l();
      }
      return 0;
    }
    case Vinstr::lea: {
      auto& i = inst.lea_;
      if (i.d == Vreg64(m_sp)) {
        assert(i.s.base == i.d && !i.s.index.isValid());
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

// Return the set of physical registers implicitly accessed (used or defined)
// TODO: t4779515: replace this, and other switches, with logic using
// attributes, instead of hardcoded opcode names.
void Vxls::getEffects(const Vinstr& i, RegSet& uses, RegSet& across,
                      RegSet& defs) {
  uses = defs = across = RegSet();
  switch (i.op) {
    case Vinstr::mccall:
    case Vinstr::call:
    case Vinstr::callm:
    case Vinstr::callr:
      defs = m_abi.all() - m_abi.calleeSaved;
      switch (arch()) {
        case Arch::ARM: defs.add(PhysReg(arm::rLinkReg)); break;
        case Arch::X64: break;
      }
      break;
    case Vinstr::callstub:
      defs = i.callstub_.kills;
      break;
    case Vinstr::bindcall:
    case Vinstr::contenter:
      defs = m_abi.all();
      break;
    case Vinstr::cqo:
      uses = RegSet(reg::rax);
      defs = RegSet().add(reg::rax).add(reg::rdx);
      break;
    case Vinstr::idiv:
      uses = defs = RegSet(reg::rax).add(reg::rdx);
      break;
    case Vinstr::shlq:
    case Vinstr::sarq:
      across = RegSet(reg::rcx);
      break;
    case Vinstr::bindaddr:
    case Vinstr::bindjcc1st:
    case Vinstr::bindjcc2nd:
    case Vinstr::bindjmp:
    case Vinstr::bindexit:
      defs = m_srkill;
      break;
    // arm instrs
    case Vinstr::hostcall:
      defs = (m_abi.all() - m_abi.calleeSaved) |
             RegSet(PhysReg(arm::rHostCallReg));
      break;
    case Vinstr::vcall:
    case Vinstr::vinvoke:
      always_assert(false && "Unsupported instruction in vxls");
    default:
      break;
  }
}

// Compute lifetime intervals and use positions of all intervals by walking
// the code bottom-up once. Loops aren't handled yet.
void Vxls::buildIntervals() {
  ONTRACE(kRegAllocLevel, printCfg(unit, blocks));
  livein.resize(unit.blocks.size());
  intervals.resize(unit.next_vr);
  UNUSED auto loops = false;
  auto preds = computePreds(unit);
  for (auto blockIt = blocks.end(); blockIt != blocks.begin();) {
    auto b = *--blockIt;
    auto& block = unit.blocks[b];
    // initial live set is the union of successor live sets.
    LiveSet live(unit.next_vr);
    for (auto s : succs(block)) {
      if (!livein[s].empty()) {
        live |= livein[s];
      } else {
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
      DefVisitor dv(live, *this, pos);
      UseVisitor uv(live, *this, {block_range.start, pos});
      visitOperands(inst, dv);
      RegSet implicit_uses, implicit_across, implicit_defs;
      getEffects(inst, implicit_uses, implicit_across, implicit_defs);
      dv.def(implicit_defs);
      visitOperands(inst, uv);
      uv.use(implicit_uses);
      uv.across(implicit_across);
    }
    // save live set so it can propagate to predecessors
    livein[b] = live;
    // add a loop-covering range to each interval live into a loop.
    for (auto p: preds[b]) {
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
  for (auto& c : unit.cpool) {
    if (auto ivl = intervals[c.second]) {
      ivl->ranges.back().start = 0;
      ivl->cns = true;
      ivl->val = c.first;
    }
  }
  // Ranges and uses were generated in reverse order. Unreverse them now.
  for (auto ivl : intervals) {
    if (!ivl) continue;
    assert(!ivl->ranges.empty()); // no empty intervals
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
      assert(ivl->cns || ivl->fixed());
    });
    for (auto ivl : intervals) {
      if (!ivl) continue;
      for (unsigned i = 1; i < ivl->uses.size(); i++) {
        assert(ivl->uses[i].pos >= ivl->uses[i-1].pos); // monotonic
      }
      for (unsigned i = 1; i < ivl->ranges.size(); i++) {
        assert(ivl->ranges[i].end > ivl->ranges[i].start); // no empty ranges
        assert(ivl->ranges[i].start > ivl->ranges[i-1].end); // no empty gaps
      }
    }
  }
}

void Vxls::walkIntervals() {
  for (auto ivl : intervals) {
    if (!ivl) continue;
    if (ivl->fixed()) {
      assignReg(ivl, ivl->vreg);
    } else if (ivl->cns) {
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
  // check for active intervals in that are expired or inactive
  for (auto i = active.begin(); i != active.end();) {
    auto ivl = *i;
    if (pos >= ivl->end()) {
      erase(active, i);
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
    assert(min_split <= max_split);
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
    assert(cur_start < min_split && min_split <= max_split);
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
  assert(ivl->uses.empty());
  ivl->reg = InvalidReg;
  if (!ivl->cns) assignSpill(ivl);
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
  assert(!ivl->fixed() && ivl->parent && ivl->uses.empty());
  auto leader = ivl->parent;
  if (leader->slot < 0) {
    if (!leader->wide) {
      leader->slot = m_nextSlot++;
    } else {
      assert(leader->reg.isSIMD());
      if (!isSlotAligned(m_nextSlot)) m_nextSlot++;
      leader->slot = m_nextSlot;
      m_nextSlot += 2;
    }
    if (m_nextSlot > NumPreAllocatedSpillLocs) {
      // ran out of spill slots
      ONTRACE(kRegAllocLevel, dumpIntervals());
      TRACE(1, "vxls-punt TooManySpills\n");
      PUNT(LinearScan_TooManySpills);
    }
  }
  ivl->slot = leader->slot;
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
    assert((kind == VregKind::Gpr && reg.isGP()) ||
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
        assert(range.start == pos);
      } else {
        // odd position
        assert(pos > range.start); // implicit label position per block
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
    assert(pos % 2 == 1);
    DEBUG_ONLY auto b = findBlock(pos);
    DEBUG_ONLY auto range = block_ranges[b];
    assert(pos - 1 >= range.start && pos + 1 < range.end);
    return true;
  };
  auto pos = ivl->def_pos + 1;
  assert(checkPos(pos));
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
            assert(!copies[pos][i2->reg]);
            copies[pos][i2->reg] = i1;
          }
        }
      }
      pos += 2;
    }
  }
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
        assert((edge_copies[{block,targetIndex}][i2->reg] == nullptr));
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
          assert((edge_copies[{b1,i}][i2->reg] == nullptr));
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

void Vxls::insertSpillsAt(jit::vector<Vinstr>& code, unsigned& j,
                          const CopyPlan& spills, MemoryRef slots,
                          unsigned pos) {
  jit::vector<Vinstr> stores;
  for (auto src : spills) {
    auto ivl = spills[src];
    if (!ivl) continue;
    auto slot = ivl->leader()->slot;
    assert(slot >= 0 && src == ivl->reg);
    MemoryRef ptr{slots.r + slotOffset(slot)};
    if (!ivl->wide) {
      stores.emplace_back(store{src, ptr});
    } else {
      assert(src.isSIMD());
      stores.emplace_back(storedqu{src, ptr});
    }
  }
  code.insert(code.begin() + j, stores.size(), ud2{});
  for (auto& inst : stores) {
    code[j] = inst;
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
    } else if (ivl->cns) {
      if (ivl->val.isByte) {
        loads.emplace_back(ldimmb{bool(ivl->val.val), dst, true});
      } else {
        loads.emplace_back(ldimm{ivl->val.val, dst, true});
      }
    } else {
      assert(ivl->spilled());
      MemoryRef ptr{slots.r + slotOffset(ivl->slot)};
      if (!ivl->wide) {
        loads.emplace_back(load{ptr, dst});
      } else {
        assert(dst.isSIMD());
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
  auto d = pos%2 == 1 ? s : f(pos+1);
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
  if (cns) {
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
  Trace::traceRelease("Spills %u\n", m_nextSlot);
  for (auto ivl : intervals) {
    if (!ivl || ivl->fixed()) continue;
    HPHP::Trace::traceRelease("%%%-2lu %s\n", size_t(ivl->vreg),
                              ivl->toString().c_str());
    for (ivl = ivl->next; ivl; ivl = ivl->next) {
      HPHP::Trace::traceRelease("    %s\n", ivl->toString().c_str());
    }
  }
}

auto const ignore_reserved = true;
auto const collapse_fixed = true;

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
    assert(p-pos < 2);
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

}

jit::vector<Vlabel> sortBlocks(const Vunit& unit) {
  BlockSorter s(unit);
  s.dfs(unit.entry);
  std::reverse(s.blocks.begin(), s.blocks.end());
  // put the blocks containing "fallthru" last; expect at most one per Vunit
  std::stable_partition(s.blocks.begin(), s.blocks.end(), [&] (Vlabel b) {
    auto& block = unit.blocks[b];
    auto& code = block.code;
    return code.back().op != Vinstr::fallthru;
  });
  return s.blocks;
}

void allocateRegisters(Vunit& unit, const Abi& abi) {
  s_counter++;
  Vxls a(unit, abi);
  a.allocate();
}

folly::Range<Vlabel*> succs(Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::jcc: return {inst.jcc_.targets, 2};
    case Vinstr::jmp: return {&inst.jmp_.target, 1};
    case Vinstr::phijmp: return {&inst.phijmp_.target, 1};
    case Vinstr::phijcc: return {inst.phijcc_.targets, 2};
    case Vinstr::unwind: return {inst.unwind_.targets, 2};
    case Vinstr::vinvoke: return {inst.vinvoke_.targets, 2};
    case Vinstr::cbcc: return {inst.cbcc_.targets, 2};
    case Vinstr::tbcc: return {inst.tbcc_.targets, 2};
    case Vinstr::hcunwind: return {inst.hcunwind_.targets, 2};
    default: return {nullptr, nullptr};
  }
}
folly::Range<const Vlabel*> succs(const Vinstr& inst) {
  return succs(const_cast<Vinstr&>(inst)).castToConst();
}

folly::Range<Vlabel*> succs(Vblock& block) {
  if (block.code.empty()) return {nullptr, nullptr};
  return succs(block.code.back());
}
folly::Range<const Vlabel*> succs(const Vblock& block) {
  return succs(const_cast<Vblock&>(block)).castToConst();
}

}}
