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

#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/data-block.h"

namespace HPHP::jit {

namespace tc {

struct TransRange;

}

/*
 * CodeCache contains our Translation Cache, which is partitioned into 3
 * sections:
 *   - main: Hot code from all translations.
 *   - cold: Cold code from optimized and live translations.
 *   - frozen: Code that is almost never used, and cold code from profiling
       translations.
 *
 * There is also a 'data' section containing things like jump tables, floating
 * point constants, and other values that must be directly addressable from
 * translated code.
 *
 * CodeCache only allows direct access to const references to its
 * (Code|Data)Blocks. Any user that wants to emit code or data must call the
 * view() function, which returns a struct of mutable references, appropriately
 * selected based on hotness and whether profiling is active.
 */
struct CodeCache {
  struct View;
  static constexpr const char* kMainName = "main";
  static constexpr const char* kColdName = "cold";
  static constexpr const char* kGdataName = "gdata";

  static constexpr uint32_t kNullptrOffset =
    std::numeric_limits<uint32_t>::max();

  static constexpr size_t MinUnassigned = 4 * (2u << 20);

  // Padding added beyond known translation sizes when reserving blocks.
  // kPad covers relocation expansion for main/frozen/data.
  // kColdPad covers relocation expansion plus service request stubs for cold.
  static constexpr size_t kDefaultBlockSize = 1024;
  static constexpr size_t kPad = 128;

  struct SectionSizes {
    size_t main;
    size_t cold;
    size_t frozen;
    size_t data;
  };

  static SectionSizes sectionSizes(TransKind kind,
                                   const tc::TransRange* sizes);

  CodeCache();

  template<typename L>
  void forEachBlock(L body) const {
    // Each block occupies contiguous entries, one per 2MiB page, so comparing
    // adjacent pointers visits it exactly once.
    const CodeBlock* previous = nullptr;
    for (auto const& entry : m_blocks) {
      auto const block = entry.load(std::memory_order_relaxed);
      if (block == previous) continue;
      previous = block;
      if (block && block->name() != kGdataName) body(*block);
    }
  }

  template<typename L>
  void forEachSection(L body) const {
    body("main", m_main);
    body("cold", m_cold);
    body("frozen", m_frozen);
  }

  /*
   * Note: this includes "data", unlike 'forEachBlock' above.
   */
  template<typename M>
  static void forEachName(M body) {
    body("main");
    body("cold");
    body("frozen");
    body("data");
    body("bytecode");
  }

  // maxUsage should be slightly less than the max capacity to avoid overflow.
  static uint32_t maxUsage(uint32_t total) {
    return total - total / 128;
  }

  size_t tcSize() const { return m_tcSize; }
  size_t codeSize() const { return m_codeSize; }

  size_t mainUsed() const { return m_main.used(); }
  size_t coldUsed() const { return m_cold.used(); }
  size_t frozenUsed() const { return m_frozen.used(); }
  size_t dataUsed() const { return m_data.used(); }
  size_t dataCapacity() const { return m_data.capacity(); }

  size_t totalUnassigned() const { return m_all.available(); }

  /*
   * Returns the total amount of code emitted to all blocks. Not synchronized,
   * so the value may be stale by the time this function returns.
   */
  size_t totalUsed() const;

  CodeAddress base() const { return m_base; }
  CodeAddress frontier() const { return m_all.frontier(); }
  CodeAddress toAddr(uint32_t offset) const {
    if (offset == kNullptrOffset) return nullptr;

    auto const addr = m_base + offset;
    assert_flog(addr >= m_base && addr < m_base + kNullptrOffset,
                "{} outside [{}, {})",
                fmt::ptr(addr),
                fmt::ptr(m_base),
                fmt::ptr(m_base + kNullptrOffset));
    return addr;
  }
  uint32_t toOffset(ConstCodeAddress addr) const {
    if (addr == nullptr) return kNullptrOffset;

    assert_flog(addr >= m_base && addr < m_base + kNullptrOffset,
                "{} outside [{}, {})",
                static_cast<const void*>(addr),
                static_cast<const void*>(m_base),
                static_cast<const void*>(m_base + kNullptrOffset));
    return addr - m_base;
  }
  bool isValidCodeAddress(ConstCodeAddress addr) const;

  bool inMain(ConstCodeAddress addr) const {
    return m_all.contains(addr) && blockFor(addr).name() == kMainName;
  }

  bool inCold(ConstCodeAddress addr) const {
    return m_all.contains(addr) && blockFor(addr).name() == kColdName;
  }

  bool inMainOrColdOrFrozen(ConstCodeAddress addr) const {
    return m_all.contains(addr) && blockFor(addr).name() != kGdataName;
  }

  /*
   * Prevent or allow writing to the code section of this CodeCache.
   */
  void protect();
  void unprotect();

  const CodeBlock& all()    const { return m_all; }

  const CodeBlock& bytecode() const { return m_bytecode; }
        CodeBlock& bytecode()       { return m_bytecode; }

  const bool isAnySectionFull() const;

  /*
   * Return a View of this CodeCache, selecting blocks approriately depending
   * on the kind of translation to be emitted.
   */
  View view(TransKind kind = TransKind::Invalid,
            const tc::TransRange* sizes = nullptr,
            bool release = false);

  CodeBlock* releaseMain(size_t size);
  CodeBlock* releaseCold(size_t size);
  CodeBlock* releaseFrozen(size_t size);
  CodeBlock* releaseData(size_t size);

  /*
   * Return the block containing addr.
   */
  const CodeBlock& blockFor(ConstCodeAddress addr) const;
        CodeBlock& blockFor(ConstCodeAddress addr);

  Address threadLocalStart() { return m_threadLocalStart; }

  struct Section {
    CodeBlock& block() const {
      auto const block = m_state.load(std::memory_order_relaxed).m_last;
      assertx(block);
      return *block;
    }

    size_t used() const {
      auto state = m_state.load(std::memory_order_relaxed);
      return state.m_used + (state.m_last ? state.m_last->used() : 0);
    }

    size_t capacity() const;
    size_t numFrees() const;
    size_t numAllocs() const;
    size_t bytesFree() const;
    size_t blocksFree() const;

    void setHugePageBudget(size_t budget) {
      assertx(!m_hugePageBudget);
      m_hugePageBudget = budget;
    }

    void addHugePageBudget(size_t budget) {
      m_hugePageBudget += budget;
    }

    void releaseBlock() {
      auto state = m_state.load(std::memory_order_relaxed);
      if (!state.m_last) return;
      state.m_used += state.m_last->size();
      state.m_last = nullptr;
      m_state.store(std::move(state), std::memory_order_release);
    }

  protected:
    struct State { size_t m_used{0}; CodeBlock* m_last{nullptr}; };
    std::atomic<State> m_state;
    size_t m_hugePageBudget{0};
  };

  const Section& mainSection() const { return m_main; }
  const Section& coldSection() const { return m_cold; }
  const Section& frozenSection() const { return m_frozen; }
  const Section& dataSection() const { return m_data; }

  void addMainHugePageBudget(size_t budgetMBs) {
    m_main.addHugePageBudget(budgetMBs);
  }

  void releaseMainBlock() {
    if (m_all.available() < DataBlock::kPageSize) return;
    m_main.releaseBlock();
    m_main.ensure(*this, DataBlock::kPageSize);
  }

  std::string blockMap() const;

private:
  template<typename S>
  CodeBlock* releaseSection(S& section, size_t size);

  void cutTCSizeTo(size_t targetSize);

  static constexpr const char kMain[] = "main";
  static constexpr const char kCold[] = "cold";
  static constexpr const char kFrozen[] = "afrozen";
  static constexpr const char kData[] = "gdata";

  template<const char* name, bool code, bool overAllocate, bool forwardAllocation, size_t pageSize>
  struct SectionImpl : Section {
    void ensure(CodeCache& cc, size_t size);
  };

  template<const char* name, bool overAllocate, bool forwardAllocation, size_t pageSize>
  using CodeSection = SectionImpl<name, true, overAllocate, forwardAllocation, pageSize>;
  using DataSection = SectionImpl<kData, false, false, false, DataBlock::kPageSize>;

  Address m_threadLocalStart{nullptr};
  CodeAddress m_base;
  size_t m_tcSize; // jit output
  size_t m_codeSize; // all code (jit+bytecode)
  size_t m_threadLocalSize;

  CodeSection<kMain, true, true, DataBlock::kPageSize> m_main;
  CodeSection<kCold, false, false, DataBlock::kPageSize> m_cold;
  CodeSection<kFrozen, false, false, DataBlock::kPageSize> m_frozen;
  DataSection m_data;

  DataBlock m_bytecode;
  DataBlock m_all;

  // Each block is at least DataBlock::kPageSize (2MiB), indexed by >> 21
  std::array<std::atomic<DataBlock*>, 1024> m_blocks{};

  template<const char* name, bool code, bool overAllocate, bool forwardAllocation, size_t pageSize>
  friend struct SectionImpl;
};

struct CodeCache::View {
  View(CodeBlock& main, CodeBlock& cold, CodeBlock& frozen, DataBlock& data,
       bool isLocal, bool isOwned = false)
    : m_main(&main)
    , m_cold(&cold)
    , m_frozen(&frozen)
    , m_data(&data)
    , m_isLocal(isLocal)
    , m_isOwned(isOwned)
  {}

  /*
   * Align all blocks to the right alignment for a translation start.
   */
  void alignForTranslation(bool alignMain);

  CodeBlock& main()   { return *m_main; }
  CodeBlock& cold()   { return *m_cold; }
  CodeBlock& frozen() { return *m_frozen; }
  DataBlock& data()   { return *m_data; }

  bool  isLocal()           const { return m_isLocal; }
  bool  isOwned()           const { return m_isOwned; }
  const CodeBlock& main()   const { return *m_main; }
  const CodeBlock& cold()   const { return *m_cold; }
  const CodeBlock& frozen() const { return *m_frozen; }
  const DataBlock& data()   const { return *m_data; }

  void setMain(CodeBlock* block)   { m_main = block; }
  void setCold(CodeBlock* block)   { m_cold = block; }
  void setFrozen(CodeBlock* block) { m_frozen = block; }
  void setData(DataBlock* block)   { m_data = block; }

private:
  CodeBlock* m_main;
  CodeBlock* m_cold;
  CodeBlock* m_frozen;
  DataBlock* m_data;
  bool m_isLocal;
  bool m_isOwned;
};

}
