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
          "Reducing TC sizes from {:,} to {:,}, "
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
#if USE_JEMALLOC
  // When we have a low arena, TC must fit below lowArenaMinAddr(). If it
  // doesn't, we shrink things to make it so.
  auto const lowArenaStart = lowArenaMinAddr();
  if (Cfg::Server::Mode) {
    Logger::Info("lowArenaMinAddr(): 0x%lx", lowArenaStart);
  }
  always_assert_flog(
    usedBase + (32u << 20) <= lowArenaStart,
    "brk is too big for LOWPTR build (usedBase = {}, lowArenaStart = {})",
    usedBase, lowArenaStart
  );

  return std::min(lowArenaStart, 2ul << 30);
#endif

  return 2ul << 30;
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

  if (Cfg::Jit::DynamicTCSections) {
    auto const aBytecode = Cfg::CodeCache::ABytecodeSize;
    auto mapSize = getTCMaxExtent(usedBase) - usedBase;
    while (mapSize > 0 && !mapTC(usedBase, mapSize)) mapSize /= 2;
    m_tcSize = m_codeSize = mapSize - thread_local_size - aBytecode;

    always_assert_flog(
      m_tcSize >= size2m * 4,
      "Insufficient low memory available to allocate minimum 2MB per section "
      "(usedBase = {}, lowArenaStart = {}, m_totalSize = {})",
      usedBase, getTCMaxExtent(usedBase), mapSize);

    m_base = reinterpret_cast<CodeAddress>(usedBase);
    m_threadLocalStart = m_base + m_tcSize;
    numa_interleave(m_base, m_tcSize);

    TRACE(1, "init a @%p\n", m_base);
    m_all.init(m_base, m_tcSize, "all");

    if (!isJitDeserializing()) {
      m_main.setHugePageBudget(m_tcSize >> 20);
    } else {
      m_main.setHugePageBudget(
        Cfg::CodeCache::TCNumHugeHotMB + Cfg::CodeCache::TCNumHugeMainMB);
    }
    m_cold.setHugePageBudget(Cfg::CodeCache::TCNumHugeColdMB);

    // Allocate initial pages for each TC section
    m_main.ensure(*this, size2m);
    m_cold.ensure(*this, size2m);
    m_frozen.ensure(*this, size2m);
    m_data.ensure(*this, size2m);

    // Assert that no one is actually writing to or reading from the pseudo
    // addresses used to emit thread local translations
    if (thread_local_size) {
      mprotect(m_threadLocalStart, thread_local_size, PROT_NONE);
    }
    m_threadLocalSize = thread_local_size;

    if (aBytecode) {
      auto const bytecodeBase = m_threadLocalStart + thread_local_size;
      m_bytecode.init(bytecodeBase, aBytecode, "abytecode");
      mprotect(m_bytecode.frontier(), m_bytecode.size(),
               PROT_READ | PROT_WRITE | PROT_EXEC);
    }
    return;
  }

  auto const kASize         = ru<size2m>(Cfg::CodeCache::ASize);
  auto const kAColdSize     = ru<size2m>(Cfg::CodeCache::AColdSize);
  auto const kAFrozenSize   = ru<size2m>(Cfg::CodeCache::AFrozenSize);
  auto const kABytecodeSize = ru<size2m>(Cfg::CodeCache::ABytecodeSize);

  auto kGDataSize = ru<size2m>(Cfg::CodeCache::GlobalDataSize);
  auto totalSize = ru<size2m>(kASize + kAColdSize + kAFrozenSize +
                           kABytecodeSize + kGDataSize + thread_local_size);
  m_tcSize = totalSize - kABytecodeSize - kGDataSize;
  m_codeSize = totalSize - kGDataSize;

  always_assert_flog(
    (kASize >= size2m) &&
    (kAColdSize >= size2m) &&
    (kAFrozenSize >= size2m) &&
    (kGDataSize >= size2m),
    "Allocation sizes are too small.\n"
    "  ASize = {}\n"
    "  AColdSize = {}\n"
    "  AFrozenSize = {}\n"
    "  GlobalDataSize = {}",
    kASize,
    kAColdSize,
    kAFrozenSize,
    kGDataSize
  );

  if (totalSize > (2ul << 30)) {
    fprintf(stderr, "Combined size of ASize, AColdSize, AFrozenSize, "
                    "ABytecodeSize, and GlobalDataSize must be < 2GiB "
                    "to support 32-bit relative addresses.\n"
                    "The sizes will be automatically reduced.\n");
    cutTCSizeTo((2ul << 30) - usedBase - thread_local_size);
    new (this) CodeCache;
    return;
  }

  auto const maxTCExtent = getTCMaxExtent(usedBase);
  if (usedBase + totalSize > maxTCExtent) {
    cutTCSizeTo(maxTCExtent - usedBase - thread_local_size);
    new (this) CodeCache;
    return;
  }
  always_assert_flog(
    usedBase + totalSize <= maxTCExtent,
    "computed allocationSize ({}) is too large to fit within "
    "lowArenaStart ({}), usedBase = {}\n",
    totalSize, maxTCExtent, usedBase
  );

  if (!mapTC(usedBase, totalSize)) {
    cutTCSizeTo(totalSize / 2);
    new (this) CodeCache;
    return;
  }

  m_base = reinterpret_cast<CodeAddress>(usedBase);
  numa_interleave(m_base, totalSize);
  m_all.init(m_base, totalSize, "all");

  uint32_t hugeMainMBs = Cfg::CodeCache::TCNumHugeHotMB + Cfg::CodeCache::TCNumHugeMainMB;
  // Don't map more pages to huge pages than kASize.  And if we're not in
  // jumpstart consumer mode, then we'll need to generate profiling code, which
  // will be placed in the beginning of code.main.  In this case, map all of
  // code.main to huge pages to ensure that the optimized code is placed on huge
  // pages.
  if ((kASize >> 20 < hugeMainMBs) || !isJitDeserializing()) {
    hugeMainMBs = uint32_t(kASize >> 20);
  }
  m_main.setHugePageBudget(hugeMainMBs);
  const uint32_t hugeColdMBs = std::min(Cfg::CodeCache::TCNumHugeColdMB,
                                        uint32_t(kAColdSize >> 20));
  m_cold.setHugePageBudget(hugeColdMBs);

  TRACE(1, "init a @%p\n", m_base);
  m_main.ensure(*this, kASize);

  TRACE(1, "init acold @%p\n", m_all.frontier());
  m_cold.ensure(*this, kAColdSize);

  TRACE(1, "init thread_local @%p\n", m_all.frontier());
  m_threadLocalStart = m_all.frontier();
  m_all.moveFrontier(thread_local_size);

  TRACE(1, "init afrozen @%p\n", m_all.frontier());
  m_frozen.ensure(*this, kAFrozenSize);

  TRACE(1, "init abytecode @%p\n", m_all.frontier());
  m_bytecode.init(m_all.frontier(), kABytecodeSize, "abytecode");
  m_all.moveFrontier(kABytecodeSize);

  TRACE(1, "init gdata @%p\n", m_all.frontier());
  m_data.ensure(*this, kGDataSize);

  // The default on linux for the newly allocated memory is read/write/exec
  // but on some systems its just read/write. Call unprotect to ensure that
  // the memory is marked executable.
  unprotect();

  // Assert that no one is actually writing to or reading from the pseudo
  // addresses used to emit thread local translations
  if (thread_local_size) {
    mprotect(m_threadLocalStart, thread_local_size, PROT_NONE);
  }
  m_threadLocalSize = thread_local_size;

  Cfg::CodeCache::AMaxUsage = maxUsage(Cfg::CodeCache::ASize);
  Cfg::CodeCache::AColdMaxUsage = maxUsage(Cfg::CodeCache::AColdSize);
  Cfg::CodeCache::AFrozenMaxUsage = maxUsage(Cfg::CodeCache::AFrozenSize);

  assertx(m_all.frontier() - m_base <= (2ul << 30));
}

void CodeCache::cutTCSizeTo(size_t targetSize) {
  assertx(targetSize < (2ull << 30));
  // Make sure the result if size_t to avoid 32-bit overflow
  auto const total = Cfg::CodeCache::ASize + Cfg::CodeCache::AColdSize +
    Cfg::CodeCache::AFrozenSize + Cfg::CodeCache::ABytecodeSize +
    Cfg::CodeCache::GlobalDataSize;
  if (total <= targetSize) return;

  Cfg::CodeCache::ASize = rd<size2m>(Cfg::CodeCache::ASize * targetSize / total);
  Cfg::CodeCache::AColdSize = rd<size2m>(Cfg::CodeCache::AColdSize * targetSize / total);
  Cfg::CodeCache::AFrozenSize = rd<size2m>(Cfg::CodeCache::AFrozenSize * targetSize / total);
  Cfg::CodeCache::ABytecodeSize = rd<size2m>(Cfg::CodeCache::ABytecodeSize * targetSize / total);
  Cfg::CodeCache::GlobalDataSize = rd<size2m>(Cfg::CodeCache::GlobalDataSize * targetSize / total);

  std::array<uint32_t*, 4> sizes = {
    &Cfg::CodeCache::ASize, &Cfg::CodeCache::AColdSize, &Cfg::CodeCache::AFrozenSize,
    &Cfg::CodeCache::GlobalDataSize
  };
  std::sort(sizes.begin(), sizes.end(), [](auto a, auto b) { return *a < *b; });

  uint32_t adj = 0;
  for (int i = 0; i < sizes.size(); ++i) {
    if (!*sizes[i]) {
      adj += 1;
      *sizes[i] = size2m;
      continue;
    }
    assertx(*sizes[i] >= size2m);

    auto const sub = std::min(
      adj / (sizes.size() - i), *sizes[i] / size2m - 1
    );
    *sizes[i] -= size2m * sub;
    adj -= sub;
  }

  always_assert_flog(
    adj == 0,
    "Could not adjust total TC size from {} to {}, "
    "insufficient space for each section to be at least {}",
    total,
    targetSize,
    size2m
  );

  Cfg::CodeCache::AMaxUsage = maxUsage(Cfg::CodeCache::ASize);
  Cfg::CodeCache::AColdMaxUsage = maxUsage(Cfg::CodeCache::AColdSize);
  Cfg::CodeCache::AFrozenMaxUsage = maxUsage(Cfg::CodeCache::AFrozenSize);

  assertx(Cfg::CodeCache::ASize + Cfg::CodeCache::AColdSize +
          Cfg::CodeCache::AFrozenSize + Cfg::CodeCache::GlobalDataSize <= targetSize);

  if (Cfg::Server::Mode) {
    Logger::FWarning("Adjusted TC sizes to fit in {} bytes: "
                     "ASize = {}, AColdSize = {}, "
                     "AFrozenSize = {}, GlobalDataSize = {}",
                     targetSize, Cfg::CodeCache::ASize, Cfg::CodeCache::AColdSize,
                     Cfg::CodeCache::AFrozenSize, Cfg::CodeCache::GlobalDataSize);
  }
}

CodeBlock& CodeCache::blockFor(ConstCodeAddress addr) {
  auto const idx = (addr - base()) >> 21;
  return *m_blocks[idx].load(std::memory_order_relaxed);
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

void CodeCache::unprotect() {
  mprotect(m_base, m_codeSize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

size_t CodeCache::Section::numFrees() const {
  return !Cfg::Jit::DynamicTCSections ? block().numFrees() : 0;
}
size_t CodeCache::Section::numAllocs() const {
  return !Cfg::Jit::DynamicTCSections ? block().numAllocs() : 0;
}
size_t CodeCache::Section::bytesFree() const {
  return !Cfg::Jit::DynamicTCSections ? block().bytesFree() : 0;
}
size_t CodeCache::Section::blocksFree() const {
  return !Cfg::Jit::DynamicTCSections ? block().blocksFree() : 0;
}

size_t CodeCache::Section::capacity() const {
  return !Cfg::Jit::DynamicTCSections ? block().size() : 0;
}

template<const char* name, bool code>
void CodeCache::SectionImpl<name, code>::ensure(CodeCache& cc, size_t size) {
  auto newState = m_state.load(std::memory_order_acquire);
  if (newState.m_last) {
    if (newState.m_last->canEmit(size)) return;
    newState.m_used += newState.m_last->used();
  }

  auto block = std::make_unique<DataBlock>(cc.m_all.allocChild(size, name));
  if (m_hugePageBudget) {
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
  ret.reserve(1024);
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

CodeCache::View CodeCache::view(TransKind kind, const tc::TransRange* sizes) {
  if (Cfg::Jit::DynamicTCSections) {
    constexpr size_t kDefault = 1024;
    constexpr size_t kPad = 128; // account for shifting during relocation

    m_main.ensure(*this, sizes ? sizes->main.size() + kPad : kDefault);
    if (isProfiling(kind)) {
      m_frozen.ensure(
        *this,
        sizes ? sizes->cold.size() + sizes->frozen.size() + kPad : kDefault
      );
    } else {
      m_cold.ensure(*this, sizes ? sizes->cold.size() + kPad : kDefault);
      m_frozen.ensure(*this, sizes ? sizes->frozen.size() + kPad : kDefault);
    }
    m_data.ensure(*this, sizes ? sizes->data.size() + kPad : kDefault);
  }

  View view{
    m_main.block(),
    isProfiling(kind) ? m_frozen.block() : m_cold.block(),
    m_frozen.block(),
    m_data.block(),
    false
  };

  tc::assertOwnsCodeLock(view);
  return view;
}

void CodeCache::View::alignForTranslation(bool alignMain) {
  if (alignMain) main().alignFrontier(tc::Translator::kTranslationAlign);
  cold().alignFrontier(tc::Translator::kTranslationAlign);
  data().alignFrontier(tc::Translator::kTranslationAlign);
  frozen().alignFrontier(tc::Translator::kTranslationAlign);
}

/////////////////////////////////////////////////////////////////////////

}
