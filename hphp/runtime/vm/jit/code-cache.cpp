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

#include "hphp/runtime/vm/jit/code-cache.h"

#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/runtime/base/program-functions.h"

#include "hphp/util/alloc.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/configs/codecache.h"
#include "hphp/util/configs/eval.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"
#include "hphp/util/trace.h"

namespace HPHP::jit {

TRACE_SET_MOD(mcg)

/////////////////////////////////////////////////////////////////////////

namespace {

void enhugen(void* base, unsigned numMB) {
  if (Cfg::CodeCache::MapTCHuge) {
    assertx((uintptr_t(base) & (size2m - 1)) == 0);
    assertx(numMB < (1 << 12));
    remap_interleaved_2m_pages(base, /* number of 2M pages */ numMB / 2);
    madvise(base, numMB << 20, MADV_DONTFORK);
  }
}

/*
 * Adjust the start of TC relative to hot runtime code. What really matters
 * is a number of 2MB pages in-between. We appear to benefit from odd numbers.
 */
uintptr_t shiftTC(uintptr_t base) {
  if (!Cfg::CodeCache::AutoTCShift || __hot_start == nullptr) return base;
  // Make sure the offset from hot text is either odd or even number
  // of huge pages.
  const auto hugePagesDelta =
    (ru<size2m>(base) -
     rd<size2m>(reinterpret_cast<uintptr_t>(__hot_start))) / size2m;
  const size_t shiftAmount =
    ((hugePagesDelta & 1) == (Cfg::CodeCache::AutoTCShift & 1)) ? 0 : size2m;

  return base + shiftAmount;
}

bool mapTC(uintptr_t usedBase, uintptr_t size) {
  // Use MAP_FIXED_NOREPLACE instead of MAP_FIXED so we actually get
  // an error if we overlap with an existing mapping.
  auto const allocBase =
    (uintptr_t)mmap(reinterpret_cast<void*>(usedBase), size,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE,
                    -1, 0);

  if (allocBase != usedBase) {
#ifdef FOLLY_SANITIZE
    // If we hit an already existing mapping when running sanitizers,
    // it's almost certainly because of the ASAN shadow region (which
    // starts just below the 2GB mark). Keep halving the TC size until
    // we no longer collide. This isn't a big deal because we don't
    // expect to run the JIT that much when running sanitizers.
    if (errno == EEXIST) {
      if (Cfg::Server::Mode) {
        Logger::FWarning(
          "Reducing TC sizes from {} to {}, "
          "due to possible ASAN collision\n",
          size, size / 2
          );
      }
      return false;
    }
#endif
    always_assert_flog(
      false,
      "mmap failed for translation cache (error = {})",
      errno == EEXIST ? "allocated range overlap" : strerror(errno)
    );
  }

  always_assert_flog(allocBase >= tc_start_address(),
                     "unexpected tc start address movement");
  return true;
}

uintptr_t getTCMaxExtent(uintptr_t usedBase) {
  // Use all the low 2GB space, but do not conflict with static literals.
  auto constexpr tcMaxAddr =
    rd<size2m>(std::min(kLowArenaMinAddr, kStaticLiteralsMinAddr));

  if (Cfg::Server::Mode) {
    Logger::Info("tcMaxAddr: 0x%lx", tcMaxAddr);
  }
  always_assert_flog(
    usedBase + (32u << 20) <= tcMaxAddr,
    "brk is too big (usedBase = {}, tcMaxAddr = {})",
    usedBase, tcMaxAddr
  );

  return tcMaxAddr;
}

}

/////////////////////////////////////////////////////////////////////////

CodeCache::CodeCache() {
  // We want to ensure that all code blocks are close to each other so that we
  // can short jump/point between them. Thus we allocate one slab and divide it
  // between the various blocks.
  auto const thread_local_size = ru<size2m>(
    Cfg::Eval::ThreadTCMainBufferSize +
    Cfg::Eval::ThreadTCColdBufferSize +
    Cfg::Eval::ThreadTCFrozenBufferSize +
    Cfg::Eval::ThreadTCDataBufferSize
  );

  auto const usedBase = shiftTC(ru<size2m>(tc_start_address()));

  auto mapSize = getTCMaxExtent(usedBase) - usedBase;
  while (mapSize > 0 && !mapTC(usedBase, mapSize)) mapSize /= 2;
  m_codeSize = mapSize - thread_local_size;

  always_assert_flog(
    m_codeSize >= size2m * 4,
    "Insufficient low memory available to allocate minimum 2MB per section "
    "(usedBase = {}, lowArenaStart = {}, m_totalSize = {})",
    usedBase, getTCMaxExtent(usedBase), mapSize);

  m_base = reinterpret_cast<CodeAddress>(usedBase);
  m_threadLocalStart = m_base + m_codeSize;
  numa_interleave(m_base, m_codeSize);

  TRACE(1, "init a @%p\n", m_base);
  m_all.init(m_base, m_codeSize, "all");

  if (!isJitDeserializing()) {
    m_main.setHugePageBudget(m_codeSize >> 20);
  } else {
    m_main.setHugePageBudget(
      Cfg::CodeCache::TCNumHugeHotMB + Cfg::CodeCache::TCNumHugeMainMB);
  }
  m_cold.setHugePageBudget(Cfg::CodeCache::TCNumHugeColdMB);
  // Assert that no one is actually writing to or reading from the pseudo
  // addresses used to emit thread local translations
  if (thread_local_size) {
    mprotect(m_threadLocalStart, thread_local_size, PROT_NONE);
  }
  m_threadLocalSize = thread_local_size;
}

CodeBlock& CodeCache::blockFor(ConstCodeAddress addr) {
  auto const idx = (addr - base()) >> 21;
  auto block = m_blocks[idx].load(std::memory_order_relaxed);
  always_assert(block);
  return *block;
}

const CodeBlock& CodeCache::blockFor(ConstCodeAddress addr) const {
  return const_cast<CodeCache&>(*this).blockFor(addr);
}

size_t CodeCache::totalUsed() const {
  return m_main.used() + m_cold.used() + m_frozen.used() + m_data.used();
}

bool CodeCache::isValidCodeAddress(ConstCodeAddress addr) const {
  return addr >= m_base && addr < m_base + m_codeSize &&
    (addr < m_threadLocalStart ||
     addr >= m_threadLocalStart + m_threadLocalSize);
}

void CodeCache::protect() {
  mprotect(m_base, m_codeSize, PROT_READ | PROT_EXEC);
}

const bool CodeCache::isAnySectionFull() const {
  return m_all.available() < CodeCache::MinUnassigned;
}

// A section spans an unbounded chain of blocks, so these per-block statistics
// no longer describe it. They stay reported as zero rather than dropped so the
// TC stats and hhvm_jit log schemas keep their shape.
size_t CodeCache::Section::numFrees() const {
  return 0;
}
size_t CodeCache::Section::numAllocs() const {
  return 0;
}
size_t CodeCache::Section::bytesFree() const {
  return 0;
}
size_t CodeCache::Section::blocksFree() const {
  return 0;
}

size_t CodeCache::Section::capacity() const {
  return 0;
}

template<const char* name, bool code, bool overAllocate, bool forwardAllocation, size_t pageSize>
void CodeCache::SectionImpl<name, code, overAllocate, forwardAllocation, pageSize>::ensure(CodeCache& cc, size_t size) {
  auto newState = m_state.load(std::memory_order_acquire);
  if (newState.m_last) {
    if (newState.m_last->canEmit(size)) return;
    newState.m_used += newState.m_last->used();
  }

  auto const largePagSize = static_cast<size_t>(cc.m_all.capacity() * Cfg::Jit::DataBlockSizeRatio);
  auto const closeToCapacityLimit = Cfg::Eval::MaxConcurrentCodeViews * largePagSize;
  bool closeToCapacity = cc.m_all.available() < closeToCapacityLimit;
  auto minBlockSize = overAllocate && !closeToCapacity
      ? largePagSize
      : 0;
  auto block = std::make_unique<DataBlock>(cc.m_all.allocChild(std::max(size, minBlockSize),
                                           name, forwardAllocation,
                                           pageSize));
  if (m_hugePageBudget && block->size() >= size2m &&
      (uintptr_t(block->frontier()) & (size2m - 1)) == 0) {
    auto const huge = std::min(m_hugePageBudget, block->size() >> 20);
    enhugen(block->frontier(), huge);
    m_hugePageBudget -= huge;
  }

  if (code) {
    mprotect(block->frontier(), block->size(),
             PROT_READ | PROT_WRITE | PROT_EXEC);
  }

  size_t first = (block->base() - cc.base()) >> 21;
  size_t past = first + (block->size() >> 21);
  always_assert(first != past);
  always_assert(first < cc.m_blocks.size());
  always_assert(past - 1 < cc.m_blocks.size());

  for (auto i = first; i < past; ++i) {
    assertx(!cc.m_blocks[i]);
    cc.m_blocks[i].store(block.get(), std::memory_order_relaxed);
  }

  newState.m_last = block.release();
  m_state.store(std::move(newState), std::memory_order_release);
}

std::string CodeCache::blockMap() const {
  std::string ret;
  ret.reserve(m_blocks.size());
  for (int i = 0; i < m_blocks.size(); ++i) {
    auto b = m_blocks[i].load(std::memory_order_relaxed);
    if (!b) ret += '*';
    else if (b->name() == kMain) ret += 'm';
    else if (b->name() == kCold) ret += 'c';
    else if (b->name() == kFrozen) ret += 'f';
    else if (b->name() == kData) ret += 'd';
    else ret += '*';
  }
  return ret;
}

CodeCache::SectionSizes CodeCache::sectionSizes(
    TransKind kind, const tc::TransRange* sizes) {
  if (!sizes) {
    if (kind == TransKind::Anchor) {
      return {0, kDefaultBlockSize, 0, kDefaultBlockSize};
    }
    return {kDefaultBlockSize, kDefaultBlockSize,
            kDefaultBlockSize, kDefaultBlockSize};
  }
  if (isProfiling(kind)) {
    return {
      sizes->main.size() + kPad,
      sizes->cold.size() + kPad,
      sizes->cold.size() + sizes->frozen.size() + kPad,
      sizes->data.size() + kPad
    };
  }
  return {
    sizes->main.size() + kPad,
    sizes->cold.size() + kPad,
    sizes->frozen.size() + kPad,
    sizes->data.size() + kPad
  };
}

CodeCache::View CodeCache::view(TransKind kind, const tc::TransRange* sizes, bool release) {
  auto const ss = sectionSizes(kind, sizes);
  m_main.ensure(*this, ss.main);
  if (isProfiling(kind)) {
    m_frozen.ensure(*this, ss.frozen);
  } else {
    m_cold.ensure(*this, ss.cold);
    m_frozen.ensure(*this, ss.frozen);
  }
  m_data.ensure(*this, ss.data);

  View view{
    m_main.block(),
    isProfiling(kind) ? m_frozen.block() : m_cold.block(),
    m_frozen.block(),
    m_data.block(),
    false,
    release
  };

  if (release) {
    m_main.releaseBlock();
    m_cold.releaseBlock();
    m_frozen.releaseBlock();
    m_data.releaseBlock();
  }

  return view;
}

template<typename S>
CodeBlock* CodeCache::releaseSection(S& section, size_t size) {
  section.ensure(*this, size);
  auto block = &section.block();
  section.releaseBlock();
  return block;
}

CodeBlock* CodeCache::releaseMain(size_t size) {
  return releaseSection(m_main, size);
}
CodeBlock* CodeCache::releaseCold(size_t size) {
  return releaseSection(m_cold, size);
}
CodeBlock* CodeCache::releaseData(size_t size) {
  return releaseSection(m_data, size);
}
CodeBlock* CodeCache::releaseFrozen(size_t size) {
  return releaseSection(m_frozen, size);
}

void CodeCache::View::alignForTranslation(bool alignMain) {
  if (alignMain) main().alignFrontier(tc::Translator::kTranslationAlign);
  cold().alignFrontier(tc::Translator::kTranslationAlign);
  data().alignFrontier(tc::Translator::kTranslationAlign);
  frozen().alignFrontier(tc::Translator::kTranslationAlign);
}

/////////////////////////////////////////////////////////////////////////

}
