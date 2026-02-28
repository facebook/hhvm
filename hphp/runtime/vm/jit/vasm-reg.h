/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/asm-x64.h"

#include <vector>

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

enum class Width : uint8_t;
struct Vptr;
struct Vscaled;
struct VscaledDisp;
template <Width w> struct Vp;

///////////////////////////////////////////////////////////////////////////////

/*
 * Vreg is like PhysReg, but numbers go beyond the physical register names.
 * Since it is unconstrained, it has predicates to test whether `rn' is a GPR,
 * XMM, or virtual register.
 */
struct Vreg {
  static constexpr unsigned kNumGP = PhysReg::kNumGP;
  static constexpr unsigned kNumXMM = PhysReg::kNumSIMD;
  static constexpr unsigned kNumSF = PhysReg::kNumSF;
  static constexpr unsigned G0 = PhysReg::kGPOffset;
  static constexpr unsigned X0 = PhysReg::kSIMDOffset;
  static constexpr unsigned S0 = PhysReg::kSFOffset;
  static constexpr unsigned V0 = PhysReg::kMaxRegs;
  static constexpr unsigned kInvalidReg = 0xffffffffU;

  /*
   * Constructors.
   */
  Vreg() = default;
  explicit Vreg(size_t r) : rn(r) {}
  /* implicit */ Vreg(Reg64 r)  : rn(int(r)) {}
  /* implicit */ Vreg(Reg32 r)  : rn(int(r)) {}
  /* implicit */ Vreg(Reg16 r)  : rn(int(r)) {}
  /* implicit */ Vreg(Reg8 r)   : rn(int(r)) {}
  /* implicit */ Vreg(RegXMM r) : rn(X0+int(r)) {}
  /* implicit */ Vreg(RegSF r)  : rn(S0+int(r)) {}
  /* implicit */ Vreg(PhysReg r);

  /*
   * Casts.
   */
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg64() const;
  /* implicit */ operator RegXMM() const;
  /* implicit */ operator RegSF() const;
  /* implicit */ operator PhysReg() const { return physReg(); }

  PhysReg physReg() const;

  /*
   * Is-a checks.
   */
  bool isPhys() const {
    static_assert(G0 < V0 && X0 < V0 && S0 < V0 && V0 < kInvalidReg, "");
    return rn < V0;
  }
  bool isGP() const { return /* rn >= G0 && */ rn < G0+kNumGP; }
  bool isSIMD() const { return rn >= X0 && rn < X0+kNumXMM; }
  bool isSF() const { return rn >= S0 && rn < S0+kNumSF; }
  bool isVirt() const { return rn >= V0 && isValid(); }
  bool isValid() const { return rn != kInvalidReg; }

  /*
   * Comparisons.
   */
  bool operator==(Vreg r) const { return rn == r.rn; }
  bool operator!=(Vreg r) const { return rn != r.rn; }
  bool operator==(PhysReg r) const { return rn == Vreg(r); }
  bool operator!=(PhysReg r) const { return rn != Vreg(r); }
  bool operator==(RegXMM r) const { return rn == Vreg(r); }
  bool operator!=(RegXMM r) const { return rn != Vreg(r); }

  /*
   * Addressing.
   */
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex) const;
  Vptr operator[](ScaledIndexDisp) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;
  Vptr operator[](Vscaled) const;
  Vptr operator[](VscaledDisp) const;
  Vptr operator[](Vreg) const;

  Vptr operator*() const;
  Vscaled operator*(int scale) const;
  friend void operator*(int scale, const Vreg&) = delete;

  Vptr operator+(size_t) const;
  Vptr operator+(int32_t) const;
  Vptr operator+(intptr_t) const;
  friend void operator+(int, const Vreg&) = delete;
  friend void operator+(size_t, const Vreg&) = delete;
  friend void operator+(intptr_t, const Vreg&) = delete;

  Vptr operator-(size_t) const = delete;
  Vptr operator-(int32_t) const;
  Vptr operator-(intptr_t) const;

private:
  unsigned rn{kInvalidReg};
};

/*
 * Vector of Vregs, for Vtuples and VcallArgs (see vasm-unit.h).
 */
using VregList = jit::vector<Vreg>;

///////////////////////////////////////////////////////////////////////////////

/*
 * An efficient representation of a set of Vregs
 *
 * We use blocks of bits to represent the Vregs. Using multiple words
 * per-block lets the compiler unroll some of the loops and take
 * advantage of SIMD instructions. If the Vregs all fit within the
 * first block (currently 256 bits), they're stored in-line, avoiding
 * any memory allocation. For most units, this is the case.
 */

struct VregSet {
private:
  struct Block;
public:

  /*
   * Constructors
   */
  VregSet() = default;

  template <typename InputIt>
  explicit VregSet(InputIt i1, InputIt i2) {
    while (i1 != i2) add(*i1++);
  }
  explicit VregSet(std::initializer_list<Vreg> i) {
    for (auto const r : i) add(r);
  }
  explicit VregSet(const VregList& l) {
    for (auto const r : l) add(r);
  }
  explicit VregSet(const RegSet& s) { add(s); }
  explicit VregSet(Vreg r) { add(r); }

  VregSet(const VregSet& o)
    : blocks{o.blocks}
  {
    if (LIKELY(!isExtended())) return;
    auto newBlocks = (Block*)malloc(o.extended.size * sizeof(Block));
    std::memcpy(newBlocks, o.extended.blocks, o.extended.size * sizeof(Block));
    extended.blocks = newBlocks;
  }

  VregSet(VregSet&& o) noexcept
    : blocks{o.blocks}
  {
    // This makes o's dtor do nothing
    o.clearExtendedBit();
  }

  /*
   * Assignment
   */

  VregSet& operator=(const VregSet& o) {
    if (this == &o) return *this;
    if (UNLIKELY(isExtended())) free(extended.blocks);
    blocks = o.blocks;
    if (LIKELY(!isExtended())) return *this;
    auto newBlocks = (Block*)malloc(o.extended.size * sizeof(Block));
    std::memcpy(
      newBlocks, o.extended.blocks, o.extended.size * sizeof(Block)
    );
    extended.blocks = newBlocks;
    return *this;
  }

  VregSet& operator=(VregSet&& o) noexcept {
    this->swap(o);
    return *this;
  }

  ~VregSet() {
    if (LIKELY(!isExtended())) return;
    free(extended.blocks);
  }

  /*
   * Getters
   */

  bool operator[](Vreg r) const {
    assertx(r != kExtendedBit);
    if (LIKELY(!isExtended())) {
      return LIKELY(r < kBitsPerBlock) ? bitvec_test(blocks.data, r) : false;
    }
    return (r < (extended.size * kBitsPerBlock))
      ? bitvec_test(extended.blocks->data, r)
      : false;
  }

  bool any() const {
    if (LIKELY(!isExtended())) return blocks.any();
    auto const size = extended.size;
    for (size_t i = 0; i < size; ++i) {
      if (extended.blocks[i].any()) return true;
    }
    return false;
  }
  bool none() const { return !any(); }
  bool empty() const { return none(); }

  size_t size() const {
    if (LIKELY(!isExtended())) return blocks.count();
    auto const size = extended.size;
    size_t n = 0;
    for (size_t i = 0; i < size; ++i) {
      n += extended.blocks[i].count();
    }
    return n;
  }

  // Returns true if this set contains any physical registers.
  bool containsPhys() const {
    static_assert(PhysReg::kMaxRegs == 128, "");
    static_assert(kBitsPerBlock >= PhysReg::kMaxRegs, "");
    // We know any physical registers will be in the first two words
    // of the first block.
    auto const b = LIKELY(!isExtended()) ? &blocks : extended.blocks;
    return b->data[0] | b->data[1];
  }

  // Returns true if this set contains only physical registers. Note
  // that an empty set will return true.
  bool allPhys() const {
    static_assert(PhysReg::kMaxRegs == 128, "");
    // This will need to be updated if we change the block size.
    static_assert(kBitsPerBlock == 256, "");
    // If we have inline storage, any non-physical Vregs will be in
    // the upper two words of the block, so just check that those are
    // empty.
    if (LIKELY(!isExtended())) return !(blocks.data[2] | blocks.data[3]);
    // Likewise if we have extended storage, check the two upper
    // words. However, we also have to check if any other blocks are
    // non-empty.
    if (extended.blocks[0].data[2] | extended.blocks[0].data[3]) return false;
    auto const size = extended.size;
    for (size_t i = 1; i < size; ++i) {
      if (extended.blocks[i].any()) return false;
    }
    return true;
  }

  // Return a RegSet containing the physical registers in this set.
  RegSet physRegs() const {
    return LIKELY(!isExtended()) ? blocks.phys() : extended.blocks->phys();
  }

  /*
   * Iteration
   *
   * Note that unlike a standard container, you get a value (not a
   * reference) to the Vreg out of the iterator.
   */
  struct iterator : public std::iterator<std::forward_iterator_tag, Vreg> {
    Vreg operator*() const {
      assertx(bit < (size * kBitsPerBlock));
      return Vreg{bit};
    }

    iterator operator++() {
      next();
      return iterator{blocks, size, bit};
    }
    iterator operator++(int) {
      auto const old = bit;
      next();
      return iterator{blocks, size, old};
    }

    bool operator==(const iterator& o) const { return bit == o.bit; }
    bool operator!=(const iterator& o) const { return bit != o.bit; }
  private:
    iterator(const Block* blocks, size_t size, size_t bit)
      : blocks{blocks}
      , size{size}
      , bit{bit} {}

    void next() {
      assertx(bit < (size * kBitsPerBlock));

      ++bit;

      auto blockIdx = bit / kBitsPerBlock;
      if (blockIdx >= size) {
        bit = size * kBitsPerBlock;
        return;
      }

      {
        auto const next = blocks[blockIdx].next(bit % kBitsPerBlock);
        if (next < kBitsPerBlock) {
          bit = blockIdx * kBitsPerBlock + next;
          return;
        }
      }

      ++blockIdx;
      for (; blockIdx < size; ++blockIdx) {
        auto const next = blocks[blockIdx].first();
        if (next < kBitsPerBlock) {
          bit = blockIdx * kBitsPerBlock + next;
          return;
        }
      }

      bit = size * kBitsPerBlock;
    }

    const Block* blocks;
    size_t size;
    size_t bit;
    friend struct VregSet;
  };

  iterator begin() const {
    if (LIKELY(!isExtended())) return iterator{&blocks, 1, blocks.first()};

    auto const size = extended.size;
    for (size_t i = 0; i < size; ++i) {
      auto const first = extended.blocks[i].first();
      if (first < kBitsPerBlock) {
        return iterator{
          extended.blocks, size, (i * kBitsPerBlock) + first
        };
      }
    }
    return iterator{nullptr, size, size * kBitsPerBlock};
  }

  iterator end() const {
    if (LIKELY(!isExtended())) return iterator{nullptr, 1, kBitsPerBlock};
    auto const size = extended.size;
    return iterator{nullptr, size, size * kBitsPerBlock};
  }

  /*
   * Operators
   */

  bool operator==(const VregSet& o) const {
    // Hopefully common case, just do a single block comparison
    if (LIKELY(!isExtended() && !o.isExtended())) {
      return blocks.equals(o.blocks);
    }

    // Otherwise check if the common block prefix of both sets are the
    // same, and the rest of the blocks are empty.
    auto const ptr1 = blocksPtr();
    auto const ptr2 = o.blocksPtr();

    auto const common = std::min(ptr1.second, ptr2.second);
    for (size_t i = 0; i < common; ++i) {
      if (!ptr1.first[i].equals(ptr2.first[i])) return false;
    }
    for (size_t i = common; i < ptr1.second; ++i) {
      if (ptr1.first[i].any()) return false;
    }
    for (size_t i = common; i < ptr2.second; ++i) {
      if (ptr2.first[i].any()) return false;
    }
    return true;
  }
  bool operator!=(const VregSet& o) const { return !(*this == o); }

  // Returns true if this set and the other set has any Vregs in
  // common. This is more efficient than computing the actual
  // intersection and checking if it's non-empty.
  bool intersects(const VregSet& o) const {
    // Hopefuly common case, just AND the inline storage blocks.
    if (LIKELY(!isExtended() && !o.isExtended())) {
      return blocks.intersects(o.blocks);
    }

    // Otherwise AND the common block prefix of both sets. Anything
    // beyond the common prefix cannot intersect, so we can ignore it.
    auto const ptr1 = blocksPtr();
    auto const ptr2 = o.blocksPtr();

    auto const common = std::min(ptr1.second, ptr2.second);
    for (size_t i = 0; i < common; ++i) {
      if (ptr1.first[i].intersects(ptr2.first[i])) return true;
    }
    return false;
  }

  /*
   * Setters
   */

  void add(Vreg r) {
    assertx(r != kExtendedBit);

    if (LIKELY(!isExtended())) {
      if (LIKELY(r < kBitsPerBlock)) return bitvec_set(blocks.data, r);
      auto const size = (r / kBitsPerBlock) + 1;
      auto newBlocks = (Block*)malloc(size * sizeof(Block));
      newBlocks[0] = blocks;
      for (size_t i = 1; i < size; ++i) new (&newBlocks[i]) Block{};
      setExtendedBit();
      extended.size = size;
      extended.blocks = newBlocks;
      return bitvec_set(extended.blocks->data, r);
    }

    auto const size = (r / kBitsPerBlock) + 1;
    if (extended.size < size) {
      extended.blocks = (Block*)realloc(extended.blocks, size * sizeof(Block));
      for (size_t i = extended.size; i < size; ++i) {
        new (&extended.blocks[i]) Block{};
      }
      extended.size = size;
    }
    bitvec_set(extended.blocks->data, r);
  }

  void add(RegSet s) { s.forEach([this] (Vreg r) { add(r); }); }

  void remove(Vreg r) {
    assertx(r != kExtendedBit);

    if (LIKELY(!isExtended())) {
      if (LIKELY(r < kBitsPerBlock)) bitvec_clear(blocks.data, r);
      return;
    }
    if (r < (extended.size * kBitsPerBlock)) {
      bitvec_clear(extended.blocks->data, r);
    }
  }

  void reset() {
    if (LIKELY(!isExtended())) {
      blocks = Block{};
      return;
    }
    auto const size = extended.size;
    for (size_t i = 0; i < size; ++i) {
      extended.blocks[i] = Block{};
    }
  }

  // Remove any physical registers from this set (in-place).
  void removePhys() {
    static_assert(PhysReg::kMaxRegs == 128, "");
    static_assert(kBitsPerBlock >= PhysReg::kMaxRegs, "");
    // Any physical registers will be in the first two words of the
    // first block, so just zero them. Note that this does not change
    // the extended bit. If we're not extended, the bit is already
    // clear. If we're extended, then we're not touching the storage
    // which contains the bit.
    auto const b = LIKELY(!isExtended()) ? &blocks : extended.blocks;
    b->data[0] = 0;
    b->data[1] = 0;
  }

  void swap(VregSet& o) noexcept {
    using std::swap;
    swap(blocks, o.blocks);
  }

  // Union another VregSet into this one
  VregSet& operator|=(const VregSet& o) {
    if (LIKELY(!o.isExtended())) {
      // If the other VregSet is using inline storage, just union its
      // single block into this one's first block.
      (LIKELY(!isExtended()) ? blocks : extended.blocks[0]) |= o.blocks;
      return *this;
    }

    // Otherwise find the last non-empty block in the other
    // VregSet. We want to ignore trailing empty blocks to avoid
    // unnecessarily expanding this VregSet.
    auto limit = o.extended.size;
    while (limit > 0) {
      if (o.extended.blocks[limit - 1].any()) break;
      --limit;
    }

    // Other VregSet is completely empty
    if (limit == 0) return *this;
    // Other VregSet's only non-empty block is the first one, so just
    // union it into this VregSet's first block.
    if (limit == 1) {
      (LIKELY(!isExtended()) ? blocks : extended.blocks[0])
        |= o.extended.blocks[0];
      return *this;
    }

    if (LIKELY(!isExtended())) {
      // This VregSet is using in-line storage, but the other VregSet
      // has a non-first non-empty block, so we need to migrate to
      // extended storage.
      auto newBlocks = (Block*)malloc(limit * sizeof(Block));
      // Union together the first block, which both VregSets have.
      Block::unionInto(newBlocks[0], blocks, o.extended.blocks[0]);
      // This VregSet only had a first block, so the "union" with the
      // rest is just the other VregSet's blocks.
      for (size_t i = 1; i < limit; ++i) newBlocks[i] = o.extended.blocks[i];
      // We're now extended
      setExtendedBit();
      extended.size = limit;
      extended.blocks = newBlocks;
      return *this;
    }

    // We're using extended storage already. Check if we need to
    // expand it.
    if (extended.size < limit) {
      extended.blocks = (Block*)realloc(extended.blocks, limit * sizeof(Block));
      for (size_t i = extended.size; i < limit; ++i) {
        new (&extended.blocks[i]) Block{};
      }
      extended.size = limit;
    }

    // We now have enough storage, so union together the blocks up to
    // last non-empty block in the other VregSet.
    for (size_t i = 0; i < limit; ++i) {
      extended.blocks[i] |= o.extended.blocks[i];
    }
    return *this;
  }

  // Intersect another VregSet with this one
  VregSet& operator&=(const VregSet& o) {
    if (LIKELY(!isExtended())) {
      // If this VregSet is using inline storage, then just intersect
      // with the other VregSet's first block. We can ignore the rest
      // of the blocks since their intersection has to be zero.
      blocks &= (LIKELY(!o.isExtended()) ? o.blocks : o.extended.blocks[0]);
      return *this;
    }

    if (LIKELY(!o.isExtended())) {
      // If the other VregSet is using inline storage, then intersect
      // with this VregSet's first block, and zero the rest.
      extended.blocks[0] &= o.blocks;
      for (size_t i = 1; i < extended.size; ++i) extended.blocks[i] = Block{};
      return *this;
    }

    // Otherwise intersect the common block prefix of both sets, and
    // zero anything in this VregSet beyond that.
    auto const common = std::min(extended.size, o.extended.size);
    for (size_t i = 0; i < common; ++i) {
      extended.blocks[i] &= o.extended.blocks[i];
    }
    for (size_t i = common; i < extended.size; ++i) {
      extended.blocks[i] = Block{};
    }
    return *this;
  }

  // Remove any Vregs in this VregSet which are also in the other
  // VregSet.
  VregSet& operator-=(const VregSet& o) {
    if (LIKELY(!isExtended())) {
      // If this VregSet is using inline storage, remove any Vregs
      // from just the first block in the other VregSet.
      blocks -= (LIKELY(!o.isExtended()) ? o.blocks : o.extended.blocks[0]);
      return *this;
    }

    if (LIKELY(!o.isExtended())) {
      // If the other VregSet is using inline storage, remove any
      // Vregs from just the first block in this VregSet.
      extended.blocks[0] -= o.blocks;
      return *this;
    }

    // Otherwise use the common block prefix of both sets
    auto const common = std::min(extended.size, o.extended.size);
    for (size_t i = 0; i < common; ++i) {
      extended.blocks[i] -= o.extended.blocks[i];
    }
    return *this;
  }

  // Implementation detail, but lets testing be more precise.
  static constexpr size_t kBitsPerBlock = 256;

private:
  /*
   * A VregSet can either use inline storage (consisting of a single
   * block as part of the VregSet), or "extended" storage (consisting
   * of a heap allocated array of blocks). We use the inline storage
   * until we have to store a Vreg too large to fit into the inline
   * block.
   *
   * To save space, the inline block and the extended blocks metadata
   * is represented with the same storage space. We distinguish the
   * two cases using a single bit in the inline block. In inline mode
   * this bit could represent a Vreg, which would cause
   * ambiguity. We've selected a bit which would represent an invalid
   * Vreg, which means it should never be present in the VregSet. In
   * inline mode, the storage just represents a block. In extended
   * mode, the storage represents a pointer to a list of blocks, and a
   * size.
   */

  // Generic way to get the pointer to the block array and its size.
  std::pair<const Block*, size_t> blocksPtr() const {
    return UNLIKELY(isExtended())
      ? std::make_pair(extended.blocks, extended.size)
      : std::make_pair(&blocks, 1);
  }

  // Check/set/clear the extended bit. If we're in inline mode, the
  // bit is actually part of the inline block, but will never
  // represent an actual Vreg. We could use bitvec_test and others for
  // this, but this generates better code.
  bool isExtended() const {
    return extendedByte() & (1U << (kExtendedBit % 8));
  }
  void setExtendedBit() {
    extendedByte() |= (1U << (kExtendedBit % 8));
  }
  void clearExtendedBit() {
    extendedByte() &= ~(1U << (kExtendedBit % 8));
  }

  // The extended bit is chosen to be an invalid physical register (so
  // should never legitimately appear as a Vreg in the set).
  static constexpr size_t kExtendedBit = PhysReg::kMaxRegs - 1;
  static constexpr size_t kExtendedIndex = kExtendedBit / 8 - sizeof(size_t);
  static_assert(kExtendedBit < PhysReg::kMaxRegs, "");
  static_assert(kExtendedBit >= PhysReg::kSFOffset + PhysReg::kNumSF, "");
  static_assert((kExtendedBit / 64) == 1, "");

  /*
   * A block is a set of words (right now 256 bits). We allocate bits
   * in blocks and do all operations on the blocks because it lets us
   * unroll loops (to a degree) and lets the compiler leverage SIMD
   * operations in a few cases.
   */
  struct Block {
    bool any() const {
      return data[0] | data[1] | data[2] | data[3];
    }
    bool intersects(const Block& o) const {
      return
        (data[0] & o.data[0]) ||
        (data[1] & o.data[1]) ||
        (data[2] & o.data[2]) ||
        (data[3] & o.data[3]);
    }
    bool equals(const Block& o) const {
      return
        data[0] == o.data[0] &&
        data[1] == o.data[1] &&
        data[2] == o.data[2] &&
        data[3] == o.data[3];
    }
    size_t count() const {
      return
        folly::popcount(data[0]) +
        folly::popcount(data[1]) +
        folly::popcount(data[2]) +
        folly::popcount(data[3]);
    }
    size_t first() const {
      if (data[0]) return ffs64(data[0]);
      if (data[1]) return ffs64(data[1]) + 64;
      if (data[2]) return ffs64(data[2]) + 128;
      if (data[3]) return ffs64(data[3]) + 192;
      return 256;
    }
    RegSet phys() const {
      RegSet out;
      auto bits = data[0];
      size_t off;
      while (ffs64(bits, off)) {
        assertx(0 <= off && off < 64);
        bits &= ~(uint64_t{1} << off);
        out |= Vreg{off};
      }
      bits = data[1];
      while (ffs64(bits, off)) {
        assertx(0 <= off && off < 64);
        bits &= ~(uint64_t{1} << off);
        out |= Vreg{off + 64ULL};
      }
      return out;
    }

    // Given an index (less than the block size), return the next set
    // bit in the block from that index (or 256 if none).
    size_t next(size_t current) const {
      auto const mask = ~static_cast<uint64_t>(0) << (current % 64);

      if (current < 64) {
        auto const masked = data[0] & mask;
        if (masked)  return ffs64(masked);
        if (data[1]) return ffs64(data[1]) + 64;
        if (data[2]) return ffs64(data[2]) + 128;
        if (data[3]) return ffs64(data[3]) + 192;
        return 256;
      }

      if (current < 128) {
        auto const masked = data[1] & mask;
        if (masked)  return ffs64(masked) + 64;
        if (data[2]) return ffs64(data[2]) + 128;
        if (data[3]) return ffs64(data[3]) + 192;
        return 256;
      }

      if (current < 192) {
        auto const masked = data[2] & mask;
        if (masked)  return ffs64(masked) + 128;
        if (data[3]) return ffs64(data[3]) + 192;
        return 256;
      }

      assertx(current < 256);
      auto const masked = data[3] & mask;
      return masked ? (ffs64(masked) + 192) : 256;
    }

    void operator|=(const Block& o) {
      data[0] |= o.data[0];
      data[1] |= o.data[1];
      data[2] |= o.data[2];
      data[3] |= o.data[3];
    }
    void operator&=(const Block& o) {
      data[0] &= o.data[0];
      data[1] &= o.data[1];
      data[2] &= o.data[2];
      data[3] &= o.data[3];
    }
    void operator-=(const Block& o) {
      data[0] &= ~o.data[0];
      data[1] &= ~o.data[1];
      data[2] &= ~o.data[2];
      data[3] &= ~o.data[3];
    }

    static void unionInto(Block& d, const Block& s1, const Block& s2) {
      d.data[0] = s1.data[0] | s2.data[0];
      d.data[1] = s1.data[1] | s2.data[1];
      d.data[2] = s1.data[2] | s2.data[2];
      d.data[3] = s1.data[3] | s2.data[3];
    }
    static void intersectInto(Block& d, const Block& s1, const Block& s2) {
      d.data[0] = s1.data[0] & s2.data[0];
      d.data[1] = s1.data[1] & s2.data[1];
      d.data[2] = s1.data[2] & s2.data[2];
      d.data[3] = s1.data[3] & s2.data[3];
    }
    static void diffInto(Block& d, const Block& s1, const Block& s2) {
      d.data[0] = s1.data[0] & ~s2.data[0];
      d.data[1] = s1.data[1] & ~s2.data[1];
      d.data[2] = s1.data[2] & ~s2.data[2];
      d.data[3] = s1.data[3] & ~s2.data[3];
    }

    uint64_t data[4];
  };
  static_assert(sizeof(Block::data) * 8 == kBitsPerBlock, "");
  static_assert(kExtendedBit < kBitsPerBlock, "");
  static_assert(kBitsPerBlock >= PhysReg::kMaxRegs, "");

  // Metadata when in extended mode
  struct Extended {
    size_t size;
    // This has to overlap with the second word of Block
    uint8_t bits[sizeof(uint64_t)];
    Block* blocks;
  };

  const uint8_t& extendedByte() const {
    return *(reinterpret_cast<const uint8_t*>(&extended) + (kExtendedBit / 8));
  }
  uint8_t& extendedByte() {
    return *(reinterpret_cast<uint8_t*>(&extended) + (kExtendedBit / 8));
  }

  // Make sure that Block and Extended alias in precisely the way we expect
  static_assert(offsetof(Block, data[0]) == offsetof(Extended, size), "");
  static_assert(offsetof(Block, data[1]) == offsetof(Extended, bits), "");
  static_assert(offsetof(Block, data[2]) == offsetof(Extended, blocks), "");

  static_assert(sizeof(Block::data[0]) == sizeof(Extended::size), "");
  static_assert(sizeof(Block::data[1]) == sizeof(Extended::bits), "");
  static_assert(sizeof(Block::data[2]) == sizeof(Extended::blocks), "");

  union {
    Block blocks{};
    Extended extended;
  };

  friend VregSet operator|(const VregSet&, const VregSet&);
  friend VregSet operator-(const VregSet&, const VregSet&);
  friend VregSet operator&(const VregSet&, const VregSet&);
};

//////////////////////////////////////////////////////////////////////

inline void swap(VregSet& a, VregSet& b) noexcept {
  a.swap(b);
}

inline VregSet operator|(const VregSet& s1, const VregSet& s2) {
  using Block = VregSet::Block;
  VregSet o;

  if (LIKELY(!s1.isExtended() && !s2.isExtended())) {
    // If they're both inline, union both single blocks and we're done
    Block::unionInto(o.blocks, s1.blocks, s2.blocks);
    return o;
  }

  // Otherwise find the highest non-empty block in either set
  auto const ptr1 = s1.blocksPtr();
  auto const ptr2 = s2.blocksPtr();

  auto size = std::max(ptr1.second, ptr2.second);
  while (size > 0) {
    if (size <= ptr1.second && ptr1.first[size-1].any()) break;
    if (size <= ptr2.second && ptr2.first[size-1].any()) break;
    --size;
  }

  // The only non-empty block is the first, so just union that
  if (size <= 1) {
    Block::unionInto(
      o.blocks,
      ptr1.first[0],
      ptr2.first[0]
    );
    return o;
  }

  // Otherwise the new VregSet cannot use inline storage. Union
  // together the blocks in common, and copy the blocks from the
  // larger VregSet (if any).
  auto newBlocks = (Block*)malloc(size * sizeof(Block));
  auto const copy1 = std::min(ptr1.second, size);
  auto const copy2 = std::min(ptr2.second, size);
  auto const common = std::min(copy1, copy2);
  for (size_t i = 0; i < common; ++i) {
    Block::unionInto(
      newBlocks[i],
      ptr1.first[i],
      ptr2.first[i]
    );
  }
  for (size_t i = common; i < copy1; ++i) {
    newBlocks[i] = ptr1.first[i];
  }
  for (size_t i = common; i < copy2; ++i) {
    newBlocks[i] = ptr2.first[i];
  }
  o.setExtendedBit();
  o.extended.size = size;
  o.extended.blocks = newBlocks;
  return o;
}

inline VregSet operator&(const VregSet& s1, const VregSet& s2) {
  using Block = VregSet::Block;
  VregSet o;

  // If either VregSet is using inline storage, so will the
  // output. Just intersect the single blocks.
  if (LIKELY(!s1.isExtended())) {
    Block::intersectInto(
      o.blocks,
      s1.blocks,
      LIKELY(!s2.isExtended()) ? s2.blocks : *s2.extended.blocks
    );
    return o;
  }

  if (LIKELY(!s2.isExtended())) {
    Block::intersectInto(
      o.blocks,
      *s1.extended.blocks,
      s2.blocks
    );
    return o;
  }

  // Find the highest block among the two VregSets which have a
  // non-empty intersection. This duplicates some computation, but
  // lets us keep the VregSets smaller on average.
  auto size = std::min(s1.extended.size, s2.extended.size);
  while (size > 0) {
    Block b;
    Block::intersectInto(
      b,
      s1.extended.blocks[size-1],
      s2.extended.blocks[size-1]
    );
    if (b.any()) break;
    --size;
  }

  // The only non-empty block is the first one, so we can keep using
  // inline storage.
  if (size <= 1) {
    Block::intersectInto(
      o.blocks,
      s1.extended.blocks[0],
      s2.extended.blocks[0]
    );
    return o;
  }

  // Intersect the blocks in common and make the output be in extended
  // mode.
  auto newBlocks = (Block*)malloc(size * sizeof(Block));
  for (size_t i = 0; i < size; ++i) {
    Block::intersectInto(
      newBlocks[i],
      s1.extended.blocks[i],
      s2.extended.blocks[i]
    );
  }
  o.setExtendedBit();
  o.extended.size = size;
  o.extended.blocks = newBlocks;
  return o;
}

inline VregSet operator-(const VregSet& s1, const VregSet& s2) {
  using Block = VregSet::Block;
  VregSet o;

  // If the lhs VregSet is using inline storage, so will the
  // output. Just diff the single inline block.
  if (LIKELY(!s1.isExtended())) {
    Block::diffInto(
      o.blocks,
      s1.blocks,
      LIKELY(!s2.isExtended()) ? s2.blocks : *s2.extended.blocks
    );
    return o;
  }

  // Otherwise find the highest block where the difference will be
  // non-empty. This duplicates some computation, but lets us keep the
  // VregSets smaller.
  auto size = s1.extended.size;
  auto const ptr2 = s2.blocksPtr();
  while (size > 0) {
    if (size <= ptr2.second) {
      Block b;
      Block::diffInto(
        b,
        s1.extended.blocks[size-1],
        ptr2.first[size-1]
      );
      if (b.any()) break;
    } else if (s1.extended.blocks[size-1].any()) {
      break;
    }
    --size;
  }

  // The only non-empty block is the first one, so we can keep inline
  // storage.
  if (size <= 1) {
    Block::diffInto(
      o.blocks,
      s1.extended.blocks[0],
      ptr2.first[0]
    );
    return o;
  }

  // Otherwise use extended storage. Compute the difference for the
  // blocks in common, and keep the lhs VregSet blocks for anything
  // beyond that.
  auto newBlocks = (Block*)malloc(size * sizeof(Block));
  auto const common = std::min(size, ptr2.second);
  for (size_t i = 0; i < common; ++i) {
    Block::diffInto(
      newBlocks[i],
      s1.extended.blocks[i],
      ptr2.first[i]
    );
  }
  for (size_t i = common; i < size; ++i) newBlocks[i] = s1.extended.blocks[i];
  o.setExtendedBit();
  o.extended.size = size;
  o.extended.blocks = newBlocks;
  return o;
}

// Overloads for r-value references, which lets us do more things in
// place.
inline VregSet operator|(VregSet&& s1, const VregSet& s2) {
  s1 |= s2;
  return std::move(s1);
}
inline VregSet operator&(VregSet&& s1, const VregSet& s2) {
  s1 &= s2;
  return std::move(s1);
}
inline VregSet operator-(VregSet&& s1, const VregSet& s2) {
  s1 -= s2;
  return std::move(s1);
}

inline VregSet operator|(VregSet&& s1, VregSet&& s2) {
  s1 |= s2;
  return std::move(s1);
}
inline VregSet operator&(VregSet&& s1, VregSet&& s2) {
  s1 &= s2;
  return std::move(s1);
}
inline VregSet operator-(VregSet&& s1, VregSet&& s2) {
  s1 -= s2;
  return std::move(s1);
}

inline VregSet operator|(const VregSet& s1, VregSet&& s2) {
  s2 |= s1;
  return std::move(s2);
}
inline VregSet operator&(const VregSet& s1, VregSet&& s2) {
  s2 &= s1;
  return std::move(s2);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Instantiations or distinct subclasses of Vr wrap virtual register numbers
 * in in a strongly typed wrapper that conveys physical-register constraints,
 * similar to Reg64, Reg32, RegXMM, etc.
 */
template<class Reg>
struct Vr {
  /*
   * Constructors.
   */
  explicit Vr(size_t rn) : rn(rn) {}
  /* implicit */ Vr(Vreg r);
  /* implicit */ Vr(Reg r) : Vr{Vreg(r)} {}
  /* implicit */ Vr(PhysReg pr) : Vr{Vreg(pr)} {}

  /*
   * Casting.
   */
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg() const { return asReg(); }
  /* implicit */ operator Vreg() const { return Vreg(rn); }
  explicit operator PhysReg() const { return asReg(); }

  Reg asReg() const;

  /*
   * Is-a checks.
   */
  bool isPhys() const {
    static_assert(Vreg::G0 == 0, "");
    return rn < Vreg::V0;
  }
  bool isGP() const { return rn>=Vreg::G0 && rn<Vreg::G0+Vreg::kNumGP; }
  bool isSIMD() const { return rn>=Vreg::X0 && rn<Vreg::X0+Vreg::kNumXMM; }
  bool isSF() const { return rn>=Vreg::S0 && rn<Vreg::S0+Vreg::kNumSF; }
  bool isVirt() const { return rn >= Vreg::V0 && isValid(); }
  bool isValid() const { return rn != Vreg::kInvalidReg; }

  /*
   * Comparisons.
   */
  bool operator==(Vr<Reg> r) const { return rn == r.rn; }
  bool operator!=(Vr<Reg> r) const { return rn != r.rn; }
  bool operator==(Vreg r) const { return rn == Vreg(r); }
  bool operator!=(Vreg r) const { return rn != Vreg(r); }
  bool operator==(PhysReg r) const { return rn == Vreg(r); }
  bool operator!=(PhysReg r) const { return rn != Vreg(r); }
  bool operator==(RegSF r) const { return rn == Vreg(r); }
  bool operator!=(RegSF r) const { return rn != Vreg(r); }

  /*
   * Addressing.
   */
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex) const;
  Vptr operator[](ScaledIndexDisp) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;
  Vptr operator*() const;
  Vptr operator+(size_t) const;
  Vptr operator+(intptr_t) const;

private:
  unsigned rn;
};

///////////////////////////////////////////////////////////////////////////////

using Vreg64  = Vr<Reg64>;
using Vreg32  = Vr<Reg32>;
using Vreg16  = Vr<Reg16>;
using Vreg8   = Vr<Reg8>;
using VregSF  = Vr<RegSF>;

struct VregDbl : Vr<RegXMM> {
  explicit VregDbl(size_t rn) : Vr<RegXMM>(rn) {}
  template<class... Args> /* implicit */ VregDbl(Args&&... args)
    : Vr<RegXMM>(std::forward<Args>(args)...) {}
  static bool allowable(Vreg r) { return r.isVirt() || r.isSIMD(); }
};

struct Vreg128 : Vr<RegXMM> {
  explicit Vreg128(size_t rn) : Vr<RegXMM>(rn) {}
  template<class... Args> /* implicit */ Vreg128(Args&&... args)
    : Vr<RegXMM>(std::forward<Args>(args)...)
  {}
};

inline Reg64 r64(Vreg64 r) { return r; }

/*
 * Vreg width constraint (or flags).
 *
 * Guaranteed to be a bitfield, which users of Width can do with as they
 * please.
 */
enum class Width : uint8_t {
  None  = 0,
  Byte  = 1,
  Word  = 1 << 1,
  Long  = 1 << 2,
  Quad  = 1 << 3,
  Octa  = 1 << 4,
  Flags = 1 << 5,
  // X-or-narrower widths.
  WordN = Byte | Word,
  LongN = Byte | Word | Long,
  QuadN = Byte | Word | Long | Quad,
  // Any non-flags register.
  AnyNF = Byte | Word | Long | Quad | Octa,
  Any   = Byte | Word | Long | Quad | Octa | Flags,
};

inline Width operator&(Width w1, Width w2) {
  return static_cast<Width>(
    static_cast<uint8_t>(w1) & static_cast<uint8_t>(w2)
  );
}
inline Width& operator&=(Width& w1, Width w2) {
  return w1 = w1 & w2;
}

inline Width width(Vreg)    { return Width::AnyNF; }
inline Width width(Vreg8)   { return Width::Byte; }
inline Width width(Vreg16)  { return Width::Word; }
inline Width width(Vreg32)  { return Width::Long; }
inline Width width(Vreg64)  { return Width::Quad; }
inline Width width(Vreg128) { return Width::Octa; }
inline Width width(VregDbl) { return Width::Quad; }
inline Width width(VregSF)  { return Width::Flags; }

std::string show(Width w);

///////////////////////////////////////////////////////////////////////////////

struct Vscaled {
  Vreg64 index;
  int scale;
  Width width;
};

struct VscaledDisp {
  Vscaled vs;
  int32_t disp;
};

VscaledDisp operator+(Vscaled, int32_t);

/*
 * Result of virtual register addressing: base + (index * scale) + disp.
 *    - base is optional, implying baseless address
 *    - index is optional
 */
struct Vptr {
  Vptr(Width w = Width::None)
    : base(Vreg{})
    , index(Vreg{})
    , disp(0)
    , scale(1)
    , width(w)
  {}

  Vptr(Vreg b, uint32_t d, Width w = Width::None)
    : base(b)
    , index(Vreg{})
    , disp(d)
    , scale(1)
    , width(w)
  {}

  Vptr(Vreg b, Vreg i, uint8_t s, uint32_t d, Width w = Width::None)
    : base(b)
    , index(i)
    , disp(d)
    , scale(s)
    , width(w)
  {
    validate();
  }

  /* implicit */ Vptr(MemoryRef m, Segment s = DS)
    : base(m.r.base)
    , index(m.r.index)
    , disp(m.r.disp)
    , scale(m.r.scale)
    , seg(s)
  {
    validate();
  }

  Vptr(MemoryRef m, Width w, Segment s = DS)
    : base(m.r.base)
    , index(m.r.index)
    , disp(m.r.disp)
    , scale(m.r.scale)
    , seg(s)
    , width(w)
  {
    validate();
  }

  Vptr(const Vptr& o) = default;
  Vptr& operator=(const Vptr& o) = default;

  MemoryRef mr() const;
  /* implicit */ operator MemoryRef() const;

  bool operator==(const Vptr&) const;
  bool operator!=(const Vptr&) const;

  void validate() {
    assertx((scale == 0x1 || scale == 0x2 || scale == 0x4 || scale == 0x8) &&
           "Invalid index register scaling (must be 1,2,4 or 8).");
    assertx(IMPLIES(!index.isValid(), scale == 0x1));
  }

  Vreg64 base;      // optional, for baseless mode
  Vreg64 index;     // optional
  int32_t disp;
  uint8_t scale;    // 1,2,4,8
  Segment seg{DS};  // DS, FS or GS
  Width width{Width::None};
};

template <Width w>
struct Vp : Vptr {
  Vp(Vreg b, uint32_t d) : Vptr(b, d, w) {}
  /* implicit */ Vp(MemoryRef m, Segment s = DS) : Vptr(m, w, s) {}
  Vp(Vreg b, Vreg i, uint8_t s, int32_t d) : Vptr(b, i, d, s, w) {}
  /* implicit */ Vp(const Vptr& m) : Vptr(m) { width = w; }
};

using Vptr8 = Vp<Width::Byte>;
using Vptr16 = Vp<Width::Word>;
using Vptr32 = Vp<Width::Long>;
using Vptr64 = Vp<Width::Quad>;
using Vptr128 = Vp<Width::Octa>;

Vptr operator+(Vptr lhs, int32_t d);
Vptr operator+(Vptr lhs, intptr_t d);
Vptr operator+(Vptr lhr, size_t d);

Vptr baseless(VscaledDisp);

inline Width width(Vptr8)   { return Width::Byte; }
inline Width width(Vptr16)  { return Width::Word; }
inline Width width(Vptr32)  { return Width::Long; }
inline Width width(Vptr64)  { return Width::Quad; }
inline Width width(Vptr128) { return Width::Octa; }
inline Width width(Vptr m)  { return m.width; }

///////////////////////////////////////////////////////////////////////////////

/*
 * A Vloc is either a single or pair of vregs, for keeping track of where we
 * have stored an SSATmp.
 */
struct Vloc {
  enum Kind { kPair, kWide };

  /*
   * Constructors.
   */
  Vloc() {}
  explicit Vloc(Vreg r) { m_regs[0] = r; }
  Vloc(Vreg r0, Vreg r1) { m_regs[0] = r0; m_regs[1] = r1; }
  Vloc(Kind kind, Vreg r) : m_kind(kind) { m_regs[0] = r; }

  /*
   * Accessors.
   */
  bool hasReg(int i = 0) const;
  Vreg reg(int i = 0) const;
  VregList regs() const;
  int numAllocated() const;
  int numWords() const;
  bool isFullSIMD() const;

  /*
   * Comparisons.
   */
  bool operator==(Vloc other) const;
  bool operator!=(Vloc other) const;

private:
  Kind m_kind{kPair};
  Vreg m_regs[2];
};

///////////////////////////////////////////////////////////////////////////////
}

namespace std {
  template<> struct hash<HPHP::jit::Vreg> {
    size_t operator()(HPHP::jit::Vreg r) const { return (size_t)r; }
  };
}

///////////////////////////////////////////////////////////////////////////////

#include "hphp/runtime/vm/jit/vasm-reg-inl.h"

