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

#include "hphp/util/code-cache.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/alloc.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(mcg);

#if defined(__APPLE__) || defined(__CYGWIN__)
const void* __hot_start = nullptr;
const void* __hot_end = nullptr;
#endif

// This value should be enough bytes to emit the "main" part of a minimal
// translation, which consists of a move and a jump.
static const int kMinTranslationBytes = 16;

CodeCache::Selector::Selector(const Args& args)
  : m_cache(args.m_cache)
  , m_oldSelection(m_cache.m_selection)
{
  // If a CodeBlock other than 'main' has already been selected, keep that
  // selection.
  if (m_cache.m_selection == Selection::Default) {
    // Profile has higher precedence than Hot.
    if (args.m_profile) {
      m_cache.m_selection = Selection::Profile;
    } else if (args.m_hot && m_cache.m_hot.available() > kMinTranslationBytes) {
      m_cache.m_selection = Selection::Hot;
    }
  }
}

CodeCache::Selector::~Selector() {
  m_cache.m_selection = m_oldSelection;
}

CodeCache::CodeCache()
  : m_selection(Selection::Default)
  , m_lock(false)
{
  static const size_t kRoundUp = 2 << 20;

  auto ru = [=] (size_t sz) { return sz + (-sz & (kRoundUp - 1)); };
  auto rd = [=] (size_t sz) { return sz & ~(kRoundUp - 1); };

  const size_t kAHotSize    = ru(RuntimeOption::EvalJitAHotSize);
  const size_t kASize       = ru(RuntimeOption::EvalJitASize);
  const size_t kAProfSize   = ru(RuntimeOption::EvalJitPGO ?
                                 RuntimeOption::EvalJitAProfSize : 0);
  const size_t kAColdSize  = ru(RuntimeOption::EvalJitAColdSize);
  const size_t kAFrozenSize = ru(RuntimeOption::EvalJitAFrozenSize);

  const size_t kGDataSize  = ru(RuntimeOption::EvalJitGlobalDataSize);
  m_totalSize = kAHotSize + kASize + kAColdSize + kAProfSize +
                kAFrozenSize + kGDataSize;
  m_codeSize = m_totalSize - kGDataSize;

  if ((kASize < (10 << 20)) ||
      (kAColdSize < (4 << 20)) ||
      (kAFrozenSize < (6 << 20)) ||
      (kGDataSize < (2 << 20))) {
    fprintf(stderr, "Allocation sizes ASize, AColdSize, AFrozenSize and "
                    "GlobalDataSize are too small.\n");
    exit(1);
  }

  if (m_totalSize > (2ul << 30)) {
    fprintf(stderr,"Combined size of ASize, AColdSize, AFrozenSize and "
                    "GlobalDataSize must be < 2GiB to support 32-bit relative "
                    "addresses\n");
    exit(1);
  }

  auto enhugen = [&](void* base, int numMB) {
    if (RuntimeOption::EvalMapTCHuge) {
      assert((uintptr_t(base) & (kRoundUp - 1)) == 0);
      hintHuge(base, numMB << 20);
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
    if (!RuntimeOption::EvalJitAutoTCShift) return 0;
    // Make sure the offset from hot text is either odd or even number
    // of huge pages.
    const auto hugePagesDelta = (ru(reinterpret_cast<size_t>(base)) -
                                 rd(reinterpret_cast<size_t>(__hot_start))) /
                                kRoundUp;
    return ((hugePagesDelta & 1) == (RuntimeOption::EvalJitAutoTCShift & 1))
      ? 0
      : kRoundUp;
  };

  if (base != (uint8_t*)-1) {
    assert(!(allocationSize & (kRoundUp - 1)));
    // Make sure that we have space to round up to the start of a huge page
    allocationSize += -(uint64_t)base & (kRoundUp - 1);
    allocationSize += shiftTC();
    base = (uint8_t*)sbrk(allocationSize);
    baseAdjustment = allocationSize - m_totalSize;
  }
  if (base == (uint8_t*)-1) {
    allocationSize = m_totalSize + kRoundUp - 1;
    if (RuntimeOption::EvalJitAutoTCShift) {
      allocationSize += kRoundUp;
    }
    base = (uint8_t*)low_malloc(allocationSize);
    if (!base) {
      base = (uint8_t*)malloc(allocationSize);
    }
    if (!base) {
      fprintf(stderr, "could not allocate %zd bytes for translation cache\n",
              allocationSize);
      exit(1);
    }
    baseAdjustment = -(uint64_t)base & (kRoundUp - 1);
    baseAdjustment += shiftTC();
  } else {
    low_malloc_skip_huge(base, base + allocationSize - 1);
  }
  assert(base);
  base += baseAdjustment;
  m_base = base;

  numa_interleave(base, m_totalSize);

  if (kAHotSize) {
    TRACE(1, "init ahot @%p\n", base);
    m_hot.init(base, kAHotSize, "hot");
    enhugen(base, kAHotSize >> 20);
    base += kAHotSize;
  }

  TRACE(1, "init a @%p\n", base);

  m_main.init(base, kASize, "main");
  enhugen(base, RuntimeOption::EvalTCNumHugeHotMB);
  base += kASize;

  TRACE(1, "init aprof @%p\n", base);
  m_prof.init(base, kAProfSize, "prof");
  base += kAProfSize;

  TRACE(1, "init acold @%p\n", base);
  m_cold.init(base, kAColdSize, "cold");
  enhugen(base, RuntimeOption::EvalTCNumHugeColdMB);
  base += kAColdSize;

  TRACE(1, "init afrozen @%p\n", base);
  m_frozen.init(base, kAFrozenSize, "afrozen");
  base += kAFrozenSize;

  TRACE(1, "init gdata @%p\n", base);
  m_data.init(base, kGDataSize, "gdata");
  base += kGDataSize;

  // The default on linux for the newly allocated memory is read/write/exec
  // but on some systems its just read/write. Call unprotect to ensure that
  // the memory is marked executable.
  unprotect();

  assert(base - m_base <= allocationSize);
  assert(base - m_base + 2 * kRoundUp > allocationSize);
}

CodeCache::~CodeCache() {
}

CodeBlock& CodeCache::blockFor(CodeAddress addr) {
  always_assert(!m_lock);
  return jit::codeBlockChoose(addr, m_main, m_hot, m_prof,
                              m_cold, m_frozen);
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

bool CodeCache::isValidCodeAddress(CodeAddress addr) const {
    return addr >= m_base && addr < m_base + m_codeSize;
}

void CodeCache::protect() {
  mprotect(m_base, m_codeSize, PROT_READ | PROT_EXEC);
}

void CodeCache::unprotect() {
  mprotect(m_base, m_codeSize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

CodeBlock& CodeCache::main() {
  always_assert(!m_lock);
    switch (m_selection) {
      case Selection::Default: return m_main;
      case Selection::Hot:     return m_hot;
      case Selection::Profile: return m_prof;
    }
    always_assert(false && "Invalid Selection");
}

CodeBlock& CodeCache::cold() {
  always_assert(!m_lock);
  switch (m_selection) {
    case Selection::Default:
    case Selection::Hot:     return m_cold;
    case Selection::Profile: return frozen();
  }
  always_assert(false && "Invalid Selection");
}

CodeBlock& CodeCache::frozen() {
  return m_frozen;
}

}
