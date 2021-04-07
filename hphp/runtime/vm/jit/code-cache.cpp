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
#include "hphp/util/hugetlb.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/numa.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(mcg);

// This value should be enough bytes to emit a REQ_RETRANSLATE: lea (4 or 7
// bytes), movq (10 bytes), and jmp (5 bytes). We then add some extra slack for
// safety.
static constexpr int kMinTranslationBytes = 32;
static constexpr size_t kRoundUp = 2ull << 20;

/* Initialized by RuntimeOption. */
uint32_t CodeCache::AHotSize = 0;
uint32_t CodeCache::ASize = 0;
uint32_t CodeCache::AProfSize = 0;
uint32_t CodeCache::AColdSize = 0;
uint32_t CodeCache::AFrozenSize = 0;
uint32_t CodeCache::ABytecodeSize = 0;
uint32_t CodeCache::GlobalDataSize = 0;
uint32_t CodeCache::AMaxUsage = 0;
uint32_t CodeCache::AProfMaxUsage = 0;
uint32_t CodeCache::AColdMaxUsage = 0;
uint32_t CodeCache::AFrozenMaxUsage = 0;
bool CodeCache::MapTCHuge = false;
uint32_t CodeCache::AutoTCShift = 0;
uint32_t CodeCache::TCNumHugeHotMB = 0;
uint32_t CodeCache::TCNumHugeMainMB = 0;
uint32_t CodeCache::TCNumHugeColdMB = 0;

static size_t ru(size_t sz) { return sz + (-sz & (kRoundUp - 1)); }

static size_t rd(size_t sz) { return sz & ~(kRoundUp - 1); }

CodeCache::CodeCache()
  : m_useHot{RuntimeOption::RepoAuthoritative && CodeCache::AHotSize > 0}
{

  // We want to ensure that all code blocks are close to each other so that we
  // can short jump/point between them. Thus we allocate one slab and divide it
  // between the various blocks.
  auto const thread_local_size = ru(
    RuntimeOption::EvalThreadTCMainBufferSize +
    RuntimeOption::EvalThreadTCColdBufferSize +
    RuntimeOption::EvalThreadTCFrozenBufferSize +
    RuntimeOption::EvalThreadTCDataBufferSize
  );

  auto const kAHotSize = RuntimeOption::EvalJitAHotSizeRoundUp ?
    ru(CodeCache::AHotSize) : CodeCache::AHotSize;
  auto const kASize       = ru(CodeCache::ASize);
  auto const kAProfSize   = ru(CodeCache::AProfSize);
  auto const kAColdSize   = ru(CodeCache::AColdSize);
  auto const kAFrozenSize = ru(CodeCache::AFrozenSize);
  auto const kABytecodeSize = ru(CodeCache::ABytecodeSize);

  auto kGDataSize = ru(CodeCache::GlobalDataSize);
  m_totalSize = ru(kAHotSize + kASize + kAColdSize + kAProfSize +
               kAFrozenSize + kABytecodeSize + kGDataSize + thread_local_size);
  m_tcSize = m_totalSize - kABytecodeSize - kGDataSize;
  m_codeSize = m_totalSize - kGDataSize;

  if ((kASize < (2 << 20)) ||
      (kAColdSize < (2 << 20)) ||
      (kAFrozenSize < (2 << 20)) ||
      (kGDataSize < (2 << 20))) {
    fprintf(stderr, "Allocation sizes ASize, AColdSize, AFrozenSize and "
                    "GlobalDataSize are too small.\n");
    exit(1);
  }

  auto const cutTCSizeTo = [] (size_t targetSize) {
    assertx(targetSize < (2ull << 30));
    // Make sure the result if size_t to avoid 32-bit overflow
    auto const total = static_cast<size_t>(AHotSize) + ASize + AProfSize +
                       AColdSize + AFrozenSize + GlobalDataSize;
    if (total <= targetSize) return;

    AHotSize = rd(AHotSize * targetSize / total);
    ASize = rd(ASize * targetSize / total);
    AProfSize = rd(AProfSize * targetSize / total);
    AColdSize = rd(AColdSize * targetSize / total);
    AFrozenSize = rd(AFrozenSize * targetSize / total);
    GlobalDataSize = rd(GlobalDataSize * targetSize / total);

    AMaxUsage = maxUsage(ASize);
    AProfMaxUsage = maxUsage(AProfSize);
    AColdMaxUsage = maxUsage(AColdSize);
    AFrozenMaxUsage = maxUsage(AFrozenSize);

    assertx(static_cast<size_t>(AHotSize) + ASize + AProfSize + AColdSize +
            AFrozenSize + GlobalDataSize <= targetSize);

    if (RuntimeOption::ServerExecutionMode()) {
      Logger::FWarning("Adjusted TC sizes to fit in {} bytes: AHotSize = {}, "
                       "ASize = {}, AProfSize = {}, AColdSize = {}, "
                       "AFrozenSize = {}, GlobalDataSize = {}",
                       targetSize, AHotSize, ASize, AProfSize, AColdSize,
                       AFrozenSize, GlobalDataSize);
    }
  };

  auto const currBase = ru(reinterpret_cast<uintptr_t>(sbrk(0)));
  if (m_totalSize > (2ul << 30)) {
    fprintf(stderr, "Combined size of ASize, AColdSize, AFrozenSize, "
                    "ABytecodeSize, and GlobalDataSize must be < 2GiB "
                    "to support 32-bit relative addresses.\n"
                    "The sizes will be automatically reduced.\n");
    cutTCSizeTo((2ul << 30) - kRoundUp - currBase - thread_local_size);
    new (this) CodeCache;
    return;
  }

#if USE_JEMALLOC_EXTENT_HOOKS
  if (use_lowptr) {
    // in LOWPTR builds, TC must fit below lowArenaMinAddr().  If it doesn't, we
    // shrink things to make it so.
    if (currBase + (32u << 20) > lowArenaMinAddr()) {
      fprintf(stderr, "brk is too big for LOWPTR build\n");
      exit(1);
    }
    auto const endAddr = currBase + m_totalSize;
    if (endAddr > lowArenaMinAddr()) {
      cutTCSizeTo(lowArenaMinAddr() - kRoundUp - currBase - thread_local_size);
      new (this) CodeCache;
      return;
    }
  }
#endif

  auto enhugen = [&](void* base, unsigned numMB) {
    if (CodeCache::MapTCHuge) {
      assertx((uintptr_t(base) & (kRoundUp - 1)) == 0);
      assertx(numMB < (1 << 12));
#ifdef __linux__
      remap_interleaved_2m_pages(base, /* number of 2M pages */ numMB / 2);
      madvise(base, numMB << 20, MADV_DONTFORK);
#endif
    }
  };

  // We want to ensure that all code blocks are close to each other so that we
  // can short jump/point between them. Thus we allocate one slab and divide it
  // between the various blocks.

  // Using sbrk to ensure its in the bottom 2G, so we avoid the need for
  // trampolines, and get to use shorter instructions for tc addresses.
  size_t allocationSize = m_totalSize;
  size_t baseAdjustment = 0;
  uint8_t* base = (uint8_t*)sbrk(0);

  // Adjust the start of TC relative to hot runtime code. What really matters
  // is a number of 2MB pages in-between. We appear to benefit from odd numbers.
  auto const shiftTC = [&]() -> size_t {
    if (!CodeCache::AutoTCShift || __hot_start == nullptr) return 0;
    // Make sure the offset from hot text is either odd or even number
    // of huge pages.
    const auto hugePagesDelta = (ru(reinterpret_cast<size_t>(base)) -
                                 rd(reinterpret_cast<size_t>(__hot_start))) /
                                kRoundUp;
    return ((hugePagesDelta & 1) == (CodeCache::AutoTCShift & 1))
      ? 0
      : kRoundUp;
  };

  if (base != (uint8_t*)-1) {
    assertx(!(allocationSize & (kRoundUp - 1)));
    // Make sure that we have space to round up to the start of a huge page
    allocationSize += -(uint64_t)base & (kRoundUp - 1);
    allocationSize += shiftTC();
    base = (uint8_t*)sbrk(allocationSize);
    baseAdjustment = allocationSize - m_totalSize;
  }
  if (base == (uint8_t*)-1) {
    allocationSize = m_totalSize + kRoundUp - 1;
    if (CodeCache::AutoTCShift) {
      allocationSize += kRoundUp;
    }
    base = (uint8_t*)low_malloc(allocationSize);
    if (!base) {
      fprintf(stderr, "could not allocate %zd bytes for translation cache\n",
              allocationSize);
      exit(1);
    }
    baseAdjustment = -(uint64_t)base & (kRoundUp - 1);
    baseAdjustment += shiftTC();
  }
  assertx(base);
  base += baseAdjustment;

  m_base = base;

  numa_interleave(base, m_totalSize);

  if (kAHotSize) {
    FTRACE(1, "init ahot @{}, size = {}\n", base, kAHotSize);
    m_hot.init(base, kAHotSize, "hot");
    const uint32_t hugeHotMBs = std::min(CodeCache::TCNumHugeHotMB,
                                         uint32_t(kAHotSize >> 20));
    enhugen(base, hugeHotMBs);
    base += kAHotSize;
  }

  TRACE(1, "init a @%p\n", base);

  m_main.init(base, kASize, "main");
  const uint32_t hugeMainMBs = std::min(CodeCache::TCNumHugeMainMB,
                                        uint32_t(kASize >> 20));
  enhugen(base, hugeMainMBs);
  base += kASize;

  TRACE(1, "init aprof @%p\n", base);
  m_prof.init(base, kAProfSize, "prof");
  base += kAProfSize;

  TRACE(1, "init acold @%p\n", base);
  m_cold.init(base, kAColdSize, "cold");
  const uint32_t hugeColdMBs = std::min(CodeCache::TCNumHugeColdMB,
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

  AMaxUsage = maxUsage(ASize);
  AProfMaxUsage = maxUsage(AProfSize);
  AColdMaxUsage = maxUsage(AColdSize);
  AFrozenMaxUsage = maxUsage(AFrozenSize);

  assertx(base - m_base <= allocationSize);
  assertx(base - m_base + 2 * kRoundUp > allocationSize);
  assertx(base - m_base <= (2ul << 30));
}

CodeBlock& CodeCache::blockFor(CodeAddress addr) {
  return codeBlockChoose(addr, m_main, m_hot, m_prof, m_cold, m_frozen);
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
  if (m_profFreed && m_prof.contains(addr)) {
    return false;
  }

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

void CodeCache::freeProf() {
  if (RuntimeOption::ServerExecutionMode()) {
    Logger::Info("Freeing code.prof");
  }

  // If the transdb is enabled, we don't actually free the memory in order to
  // allow the profile code to be included in TC dumps.  However, we still set
  // m_profFreed below in order to trigger asserts.
  if (!transdb::enabled()) {
    if (madvise(m_prof.base(), m_prof.size(), MADV_DONTNEED) == -1) {
      if (RuntimeOption::ServerExecutionMode()) {
        Logger::Warning("code.prof madvise failure: %s\n",
                        folly::errnoStr(errno).c_str());
      }
    }
    mprotect(m_prof.base(), m_prof.size(), PROT_NONE);
#if USE_JEMALLOC_EXTENT_HOOKS
    // Reuse the memory region as an emergency buffer for low memory.
    if (use_lowptr && RO::EvalRecycleAProf &&
        is_low_mem(m_prof.base()) && low_arena) {
      auto const base = reinterpret_cast<uintptr_t>(m_prof.base());
      assertx((base & ~(s_pageSize - 1)) == 0);
      assertx((m_prof.size() & ~(s_pageSize - 1)) == 0);
      using namespace alloc;
      auto prof_range = new RangeState(base, base + m_prof.size(), Reserved{});
      auto mapper =
        new BumpEmergencyMapper([]{ kill(getpid(), SIGTERM); }, *prof_range);
      reinterpret_cast<LowArena*>(&g_lowerArena)->appendMapper(mapper);
      reinterpret_cast<LowArena*>(&g_lowArena)->appendMapper(mapper);
      reinterpret_cast<LowArena*>(&g_lowColdArena)->appendMapper(mapper);
    }
#endif
  }

  m_profFreed = true;
}

CodeCache::View CodeCache::view(TransKind kind) {
  auto view = [&] {
    if (isProfiling(kind)) {
      return View{m_prof, m_frozen, m_frozen, m_data, false};
    }

    const bool isOpt = kind == TransKind::Optimize ||
                       kind == TransKind::OptPrologue;
    if (isOpt && m_useHot && m_hot.available() > kMinTranslationBytes) {
      return View{m_hot, m_cold, m_frozen, m_data, false};
    }

    return View{m_main, m_cold, m_frozen, m_data, false};
  }();

  tc::assertOwnsCodeLock(view);
  return view;
}

void CodeCache::View::alignForTranslation() {
  main().alignFrontier(tc::Translator::kTranslationAlign);
  cold().alignFrontier(tc::Translator::kTranslationAlign);
  data().alignFrontier(tc::Translator::kTranslationAlign);
  frozen().alignFrontier(tc::Translator::kTranslationAlign);
}

}}
