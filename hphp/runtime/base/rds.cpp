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
#include "hphp/runtime/base/rds.h"

#include <cassert>
#include <cstdio>

#include <sys/mman.h>

#include "folly/String.h"
#include "folly/Hash.h"

#include "hphp/util/base.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/mutex.h"
#include "hphp/util/lock.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/unit.h"

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

// Current allocation frontier for the non-persistent region.
size_t s_frontier = sizeof(Header);

// Frontier and base of the persistent region.
size_t s_persistent_base = 0;
size_t s_persistent_frontier = 0;

/*
 * This mutex protects actually allocating from RDS (the above
 * statics).  It is ordered *after* the locks in s_linkTable.
 */
SimpleMutex s_allocMutex(false /*recursive*/, RankLeaf);

//////////////////////////////////////////////////////////////////////

struct SymbolEq : boost::static_visitor<bool> {
  template<class T, class U>
  typename std::enable_if<
    !std::is_same<T,U>::value,
    bool
  >::type operator()(const T&, const U&) const { return false; }

  bool operator()(StaticLocal k1, StaticLocal k2) const {
    assert(k1.name->isStatic() && k2.name->isStatic());
    return k1.funcId == k2.funcId && k1.name == k2.name;
  }

  bool operator()(ClsConstant k1, ClsConstant k2) const {
    assert(k1.clsName->isStatic() && k1.cnsName->isStatic());
    assert(k2.clsName->isStatic() && k2.cnsName->isStatic());
    return k1.clsName->isame(k2.clsName) &&
           k1.cnsName == k2.cnsName;
  }

  bool operator()(StaticProp k1, StaticProp k2) const {
    assert(k1.name->isStatic() && k2.name->isStatic());
    return k1.name == k2.name;
  }

  template<class T>
  typename std::enable_if<
    std::is_same<T,StaticMethod>::value ||
      std::is_same<T,StaticMethodF>::value,
    bool
  >::type operator()(const T& t1, const T& t2) const {
    assert(t1.name->isStatic() && t2.name->isStatic());
    return t1.name->isame(t2.name);
  }
};

struct SymbolHash : boost::static_visitor<size_t> {
  size_t operator()(StaticLocal k) const {
    return folly::hash::hash_128_to_64(
      std::hash<FuncId>()(k.funcId),
      k.name->hash()
    );
  }

  size_t operator()(ClsConstant k) const {
    return folly::hash::hash_128_to_64(
      k.clsName->hash(),
      k.cnsName->hash()
    );
  }

  size_t operator()(StaticProp k)    const { return k.name->hash(); }
  size_t operator()(StaticMethod k)  const { return k.name->hash(); }
  size_t operator()(StaticMethodF k) const { return k.name->hash(); }
};

struct HashCompare {
  bool equal(const Symbol& k1, const Symbol& k2) const {
    return boost::apply_visitor(SymbolEq(), k1, k2);
  }

  size_t hash(const Symbol& k) const {
    return boost::apply_visitor(SymbolHash(), k);
  }
};

typedef tbb::concurrent_hash_map<
  Symbol,
  Handle,
  HashCompare
> LinkTable;

LinkTable s_linkTable;

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace detail {

Handle alloc(Mode mode, size_t numBytes, size_t align) {
  s_allocMutex.assertOwnedBySelf();
  align = Util::roundUpToPowerOfTwo(align);
  auto& frontier = mode == Mode::Persistent ? s_persistent_frontier
                                            : s_frontier;

  // Note: it's ok not to zero new allocations, because we've never
  // done anything with this part of the page yet, so it must still be
  // zero.
  frontier += align - 1;
  frontier &= ~(align - 1);
  frontier += numBytes;

  auto const limit = mode == Mode::Persistent
    ? RuntimeOption::EvalJitTargetCacheSize
    : s_persistent_base;
  always_assert(frontier < limit);

  return frontier - numBytes;
}

Handle allocUnlocked(Mode mode, size_t numBytes, size_t align) {
  SimpleLock l(s_allocMutex);
  return alloc(mode, numBytes, align);
}

Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes, size_t align) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second;

  SimpleLock l(s_allocMutex);
  if (s_linkTable.find(acc, key)) return acc->second;

  auto const retval = alloc(mode, sizeBytes, align);
  if (!s_linkTable.insert(LinkTable::value_type(key, retval))) {
    always_assert(0);
  }
  return retval;
}

void bindOnLinkImpl(std::atomic<Handle>& handle,
                    Mode mode,
                    size_t sizeBytes,
                    size_t align) {
  SimpleLock l(s_allocMutex);
  if (handle.load(std::memory_order_relaxed) == kInvalidHandle) {
    handle.store(alloc(mode, sizeBytes, align), std::memory_order_relaxed);
  }
}

}


//////////////////////////////////////////////////////////////////////

__thread void* tl_base = nullptr;
static __thread std::aligned_storage<
  sizeof(Array),
  alignof(Array)
>::type s_constantsStorage;

//////////////////////////////////////////////////////////////////////

static size_t s_next_bit;
static size_t s_bits_to_go;
static int s_tc_fd;

// Mapping from names to targetcache locations.
typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashICompare>
  HandleMapIS;

typedef tbb::concurrent_hash_map<const StringData*, Handle,
        StringDataHashCompare>
  HandleMapCS;

//////////////////////////////////////////////////////////////////////

void requestInit() {
  assert(tl_base);
  new (&s_constantsStorage) Array();
  assert(!s_constants().get());
  memset(tl_base, 0, s_frontier);
}

void requestExit() {
  s_constants().detach(); // it will be swept
  // Don't bother running the dtor ...
}

void flush() {
  if (madvise(tl_base, s_frontier, MADV_DONTNEED) == -1) {
    fprintf(stderr, "RDS madvise failure: %s\n",
      folly::errnoStr(errno).c_str());
  }
}

size_t usedBytes() {
  return s_frontier;
}

size_t usedPersistentBytes() {
  return s_persistent_frontier - s_persistent_base;
}

Array& s_constants() {
  void* vp = &s_constantsStorage;
  return *static_cast<Array*>(vp);
}

//////////////////////////////////////////////////////////////////////

size_t allocBit() {
  SimpleLock l(s_allocMutex);
  if (!s_bits_to_go) {
    static const int kNumBytes = 512;
    static const int kNumBytesMask = kNumBytes - 1;
    s_next_bit = s_frontier * CHAR_BIT;
    // allocate at least kNumBytes bytes, and make sure we end
    // on a 64 byte aligned boundary.
    int bytes = ((~s_frontier + 1) & kNumBytesMask) + kNumBytes;
    s_bits_to_go = bytes * CHAR_BIT;
    s_frontier += bytes;
  }
  s_bits_to_go--;
  return s_next_bit++;
}

bool testAndSetBit(size_t bit) {
  Handle handle = bit / CHAR_BIT;
  unsigned char mask = 1 << (bit % CHAR_BIT);
  bool ret = handleToRef<unsigned char>(handle) & mask;
  handleToRef<unsigned char>(handle) |= mask;
  return ret;
}

bool isPersistentHandle(Handle handle) {
  assert(handle >= 0 && handle < RuntimeOption::EvalJitTargetCacheSize);
  return handle >= (unsigned)s_persistent_base;
}

static void initPersistentCache() {
  SimpleLock l(s_allocMutex);
  if (s_tc_fd) return;
  char tmpName[] = "/tmp/tcXXXXXX";
  s_tc_fd = mkstemp(tmpName);
  always_assert(s_tc_fd != -1);
  unlink(tmpName);
  s_persistent_base = RuntimeOption::EvalJitTargetCacheSize * 3 / 4;
  s_persistent_base -= s_persistent_base & (4 * 1024 - 1);
  ftruncate(s_tc_fd,
            RuntimeOption::EvalJitTargetCacheSize - s_persistent_base);
  s_persistent_frontier = s_persistent_base;
}

void threadInit() {
  if (!s_tc_fd) {
    initPersistentCache();
  }

  tl_base = mmap(nullptr, RuntimeOption::EvalJitTargetCacheSize,
                 PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  always_assert(tl_base != MAP_FAILED);
  Util::numa_bind_to(tl_base, s_persistent_base, Util::s_numaNode);
  if (RuntimeOption::EvalMapTgtCacheHuge) {
    hintHuge(tl_base, RuntimeOption::EvalJitTargetCacheSize);
  }

  void *shared_base = (char*)tl_base + s_persistent_base;
  /*
   * map the upper portion of the RDS to a shared area This is used
   * for persistent classes and functions, so they are always defined,
   * and always visible to all threads.
   */
  void *mem = mmap(shared_base,
                   RuntimeOption::EvalJitTargetCacheSize - s_persistent_base,
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, s_tc_fd, 0);
  always_assert(mem == shared_base);
}

void threadExit() {
  munmap(tl_base, RuntimeOption::EvalJitTargetCacheSize);
}

//////////////////////////////////////////////////////////////////////

}}
