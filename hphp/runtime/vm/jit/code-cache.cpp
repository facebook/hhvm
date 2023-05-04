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

namespace HPHP::jit {

TRACE_SET_MOD(mcg);

static constexpr size_t kRoundUp = 2ull << 20;

/* Initialized by RuntimeOption. */
uint32_t CodeCache::ASize = 0;
uint32_t CodeCache::AColdSize = 0;
uint32_t CodeCache::AFrozenSize = 0;
uint32_t CodeCache::ABytecodeSize = 0;
uint32_t CodeCache::GlobalDataSize = 0;
uint32_t CodeCache::AMaxUsage = 0;
uint32_t CodeCache::AColdMaxUsage = 0;
uint32_t CodeCache::AFrozenMaxUsage = 0;
bool CodeCache::MapTCHuge = false;
uint32_t CodeCache::AutoTCShift = 0;
uint32_t CodeCache::TCNumHugeHotMB = 0;
uint32_t CodeCache::TCNumHugeMainMB = 0;
uint32_t CodeCache::TCNumHugeColdMB = 0;

static size_t ru(size_t sz) { return sz + (-sz & (kRoundUp - 1)); }

static size_t rd(size_t sz) { return sz & ~(kRoundUp - 1); }

/////////////////////////////////////////////////////////////////////////

namespace {

void enhugen(void* base, unsigned numMB) {
  if (CodeCache::MapTCHuge) {
    assertx((uintptr_t(base) & (kRoundUp - 1)) == 0);
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
  if (!CodeCache::AutoTCShift || __hot_start == nullptr) return base;
  // Make sure the offset from hot text is either odd or even number
  // of huge pages.
  const auto hugePagesDelta = (ru(base) -
                               rd(reinterpret_cast<size_t>(__hot_start))) /
                              kRoundUp;
  const size_t shiftAmount =
    ((hugePagesDelta & 1) == (CodeCache::AutoTCShift & 1)) ? 0 : kRoundUp;

  return base + shiftAmount;
}

}

/////////////////////////////////////////////////////////////////////////

CodeCache::CodeCache() {
  // We want to ensure that all code blocks are close to each other so that we
  // can short jump/point between them. Thus we allocate one slab and divide it
  // between the various blocks.
  auto const thread_local_size = ru(
    RuntimeOption::EvalThreadTCMainBufferSize +
    RuntimeOption::EvalThreadTCColdBufferSize +
    RuntimeOption::EvalThreadTCFrozenBufferSize +
    RuntimeOption::EvalThreadTCDataBufferSize
  );

  auto const kASize         = ru(CodeCache::ASize);
  auto const kAColdSize     = ru(CodeCache::AColdSize);
  auto const kAFrozenSize   = ru(CodeCache::AFrozenSize);
  auto const kABytecodeSize = ru(CodeCache::ABytecodeSize);

  auto kGDataSize = ru(CodeCache::GlobalDataSize);
  m_totalSize = ru(kASize + kAColdSize + kAFrozenSize + kABytecodeSize +
                   kGDataSize + thread_local_size);
  m_tcSize = m_totalSize - kABytecodeSize - kGDataSize;
  m_codeSize = m_totalSize - kGDataSize;

  always_assert_flog(
    (kASize >= (2 << 20)) &&
    (kAColdSize >= (2 << 20)) &&
    (kAFrozenSize >= (2 << 20)) &&
    (kGDataSize >= (2 << 20)),
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

  auto const currBase = (uintptr_t)sbrk(0);
  auto const usedBase = shiftTC(ru(currBase));

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
  if (RuntimeOption::ServerExecutionMode()) {
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
  auto const newBrk = (uintptr_t) sbrk(0);
  always_assert_flog(allocBase >= (uintptr_t) newBrk,
                     "unexpected brk movement, we cannot proceed safely");
  CodeAddress base = reinterpret_cast<CodeAddress>(usedBase);
  m_base = base;

  numa_interleave(base, m_totalSize);

  TRACE(1, "init a @%p\n", m_base);

  m_main.init(base, kASize, "main");
  uint32_t hugeMainMBs = CodeCache::TCNumHugeHotMB + CodeCache::TCNumHugeMainMB;
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
  AColdMaxUsage = maxUsage(AColdSize);
  AFrozenMaxUsage = maxUsage(AFrozenSize);

  assertx(base - m_base <= (2ul << 30));
}

void CodeCache::cutTCSizeTo(size_t targetSize) {
  assertx(targetSize < (2ull << 30));
  // Make sure the result if size_t to avoid 32-bit overflow
  auto const total =
    ASize + AColdSize + AFrozenSize + ABytecodeSize + GlobalDataSize;
  if (total <= targetSize) return;

  ASize = rd(ASize * targetSize / total);
  AColdSize = rd(AColdSize * targetSize / total);
  AFrozenSize = rd(AFrozenSize * targetSize / total);
  ABytecodeSize = rd(ABytecodeSize * targetSize / total);
  GlobalDataSize = rd(GlobalDataSize * targetSize / total);

  AMaxUsage = maxUsage(ASize);
  AColdMaxUsage = maxUsage(AColdSize);
  AFrozenMaxUsage = maxUsage(AFrozenSize);

  assertx(ASize + AColdSize + AFrozenSize + GlobalDataSize <= targetSize);

  if (RuntimeOption::ServerExecutionMode()) {
    Logger::FWarning("Adjusted TC sizes to fit in {} bytes: "
                     "ASize = {}, AColdSize = {}, "
                     "AFrozenSize = {}, GlobalDataSize = {}",
                     targetSize, ASize, AColdSize,
                     AFrozenSize, GlobalDataSize);
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
