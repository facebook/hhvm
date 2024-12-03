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

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/runtime/base/program-functions.h"

#include "hphp/util/alloc.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/bump-mapper.h"
#include "hphp/util/configs/codecache.h"
#include "hphp/util/configs/eval.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/numa.h"
#include "hphp/util/trace.h"

namespace HPHP::jit {

TRACE_SET_MOD(mcg);

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

  auto const kASize         = ru<size2m>(Cfg::CodeCache::ASize);
  auto const kAColdSize     = ru<size2m>(Cfg::CodeCache::AColdSize);
  auto const kAFrozenSize   = ru<size2m>(Cfg::CodeCache::AFrozenSize);
  auto const kABytecodeSize = ru<size2m>(Cfg::CodeCache::ABytecodeSize);

  auto kGDataSize = ru<size2m>(Cfg::CodeCache::GlobalDataSize);
  m_totalSize = ru<size2m>(kASize + kAColdSize + kAFrozenSize +
                           kABytecodeSize + kGDataSize + thread_local_size);
  m_tcSize = m_totalSize - kABytecodeSize - kGDataSize;
  m_codeSize = m_totalSize - kGDataSize;

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

  auto const usedBase = shiftTC(ru<size2m>(tc_start_address()));

  if (m_totalSize > (2ul << 30)) {
    fprintf(stderr, "Combined size of ASize, AColdSize, AFrozenSize, "
                    "ABytecodeSize, and GlobalDataSize must be < 2GiB "
                    "to support 32-bit relative addresses.\n"
                    "The sizes will be automatically reduced.\n");
    cutTCSizeTo((2ul << 30) - usedBase - thread_local_size);
    new (this) CodeCache;
    return;
  }

#if USE_JEMALLOC_EXTENT_HOOKS
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

  if (usedBase + m_totalSize > lowArenaStart) {
    cutTCSizeTo(lowArenaStart - usedBase - thread_local_size);
    new (this) CodeCache;
    return;
  }
  always_assert_flog(
    usedBase + m_totalSize <= lowArenaStart,
    "computed allocationSize ({}) is too large to fit within "
    "lowArenaStart ({}), usedBase = {}\n",
    m_totalSize, lowArenaStart, usedBase
  );
#endif
  auto const allocBase =
    (uintptr_t)mmap(reinterpret_cast<void*>(usedBase), m_totalSize,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
  always_assert_flog(allocBase == usedBase,
                     "mmap failed for translation cache (errno = {})",
                     errno);
  always_assert_flog(allocBase >= tc_start_address(),
                     "unexpected tc start address movement");
  CodeAddress base = reinterpret_cast<CodeAddress>(usedBase);
  m_base = base;

  numa_interleave(base, m_totalSize);

  TRACE(1, "init a @%p\n", m_base);

  m_main.init(base, kASize, "main");
  uint32_t hugeMainMBs = Cfg::CodeCache::TCNumHugeHotMB + Cfg::CodeCache::TCNumHugeMainMB;
  // Don't map more pages to huge pages than kASize.  And if we're not in
  // jumpstart consumer mode, then we'll need to generate profiling code, which
  // will be placed in the beginning of code.main.  In this case, map all of
  // code.main to huge pages to ensure that the optimized code is placed on huge
  // pages.
  if ((kASize >> 20 < hugeMainMBs) || !isJitDeserializing()) {
    hugeMainMBs = uint32_t(kASize >> 20);
  }
  enhugen(base, hugeMainMBs);
  base += kASize;

  TRACE(1, "init acold @%p\n", base);
  m_cold.init(base, kAColdSize, "cold");
  const uint32_t hugeColdMBs = std::min(Cfg::CodeCache::TCNumHugeColdMB,
                                        uint32_t(kAColdSize >> 20));
  enhugen(base, hugeColdMBs);
  base += kAColdSize;

  TRACE(1, "init thread_local @%p\n", base);
  m_threadLocalStart = base;
  base += thread_local_size;

  TRACE(1, "init afrozen @%p\n", base);
  m_frozen.init(base, kAFrozenSize, "afrozen");
  base += kAFrozenSize;

  TRACE(1, "init abytecode @%p\n", base);
  m_bytecode.init(base, kABytecodeSize, "abytecode");
  base += kABytecodeSize;

  TRACE(1, "init gdata @%p\n", base);
  m_data.init(base, kGDataSize, "gdata");
  base += kGDataSize;

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

  assertx(base - m_base <= (2ul << 30));
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

CodeBlock& CodeCache::blockFor(CodeAddress addr) {
  return codeBlockChoose(addr, m_main, m_cold, m_frozen);
}

const CodeBlock& CodeCache::blockFor(CodeAddress addr) const {
  return const_cast<CodeCache&>(*this).blockFor(addr);
}

size_t CodeCache::totalUsed() const {
  size_t ret = 0;
  forEachBlock([&ret](const char*, const CodeBlock& b) {
    // A thread with the write lease may be modifying b.m_frontier while we
    // call b.used() but it should never modify b.m_base. This means that at
    // worst b.used() will return a slightly stale value.
    ret += b.used();
  });
  return ret;
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

CodeCache::View CodeCache::view(TransKind kind) {
  auto view = [&] {
    if (isProfiling(kind)) {
      return View{m_main, m_frozen, m_frozen, m_data, false};
    }

    return View{m_main, m_cold, m_frozen, m_data, false};
  }();

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
