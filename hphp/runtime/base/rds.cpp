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
#include "hphp/runtime/base/rds.h"

#include <cassert>
#include <cstdio>
#include <mutex>
#include <atomic>
#include <vector>

#include <sys/mman.h>
#ifndef __CYGWIN__
#include <execinfo.h>
#endif

#include <folly/String.h>
#include <folly/Hash.h>
#include <folly/Bits.h>

#include "hphp/util/maphuge.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/vm/debug/debug.h"

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using Guard = std::lock_guard<std::mutex>;

// Current allocation frontier for the non-persistent region.
size_t s_normal_frontier = sizeof(Header);

// Frontier and base of the persistent region.
size_t s_persistent_base = 0;
size_t s_persistent_frontier = 0;

// Frontier for the "local" part of the persistent region (data not
// shared between threads, but not zero'd)---downward-growing.
size_t s_local_frontier = 0;

/*
 * This mutex protects actually allocating from RDS (the above
 * statics).  It is ordered *after* the locks in s_linkTable.
 */
std::mutex s_allocMutex;

//////////////////////////////////////////////////////////////////////

struct SymbolKind : boost::static_visitor<std::string> {
  std::string operator()(StaticLocal k) const { return "StaticLocal"; }
  std::string operator()(ClsConstant k) const { return "ClsConstant"; }
  std::string operator()(StaticProp k) const { return "StaticProp"; }
  std::string operator()(StaticMethod k) const { return "StaticMethod"; }
  std::string operator()(StaticMethodF k) const { return "StaticMethodF"; }
  std::string operator()(Profile k) const { return "Profile"; }
};

struct SymbolRep : boost::static_visitor<std::string> {
  std::string operator()(StaticLocal k) const {
    const Func* func = Func::fromFuncId(k.funcId);
    const Class* cls = getOwningClassForFunc(func);
    std::string name;
    if (cls != func->cls()) {
      name = cls->name()->toCppString() + "::" +
        func->name()->toCppString();
    } else {
      name = func->fullName()->toCppString();
    }
    return name + "::" + k.name->toCppString();
  }

  std::string operator()(ClsConstant k) const {
    return k.clsName->data() + std::string("::") + k.cnsName->data();
  }

  std::string operator()(StaticProp k)    const { return k.name->data(); }
  std::string operator()(StaticMethod k)  const { return k.name->data(); }
  std::string operator()(StaticMethodF k) const { return k.name->data(); }

  std::string operator()(Profile k) const {
    return folly::format(
      "{}:t{}:{}",
      k.name,
      k.transId,
      k.bcOff
    ).str();
  }
};

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

  bool operator()(Profile k1, Profile k2) const {
    assert(k1.name->isStatic() && k2.name->isStatic());
    return k1.transId == k2.transId &&
           k1.bcOff == k2.bcOff &&
           k1.name == k2.name;
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

  size_t operator()(Profile k) const {
    return folly::hash::hash_combine(
      k.transId,
      k.bcOff,
      k.name->hash()
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

const char* mode_name(Mode mode) {
  switch (mode) {
  case Mode::Normal:      return "Normal";
  case Mode::Local:       return "Local";
  case Mode::Persistent:  return "Persistent";
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

namespace detail {

Handle alloc(Mode mode, size_t numBytes, size_t align) {
  align = folly::nextPowTwo(align);

  switch (mode) {
  case Mode::Persistent:
  case Mode::Normal:
    {
      auto& frontier = mode == Mode::Persistent ? s_persistent_frontier
                                                : s_normal_frontier;

      // Note: it's ok not to zero new allocations, because we've never
      // done anything with this part of the page yet, so it must still be
      // zero.
      frontier += align - 1;
      frontier &= ~(align - 1);
      frontier += numBytes;

      auto const limit = mode == Mode::Persistent
        ? RuntimeOption::EvalJitTargetCacheSize
        : s_local_frontier;
      always_assert_flog(
        frontier < limit,
        "Ran out of RDS space (mode={})",
        mode_name(mode)
      );

      return frontier - numBytes;
    }
  case Mode::Local:
    {
      auto& frontier = s_local_frontier;

      frontier -= numBytes;
      frontier &= ~(align - 1);

      always_assert_flog(
        frontier >= s_normal_frontier,
        "Ran out of RDS space (mode=Local)"
      );

      return frontier;
    }
  }

  not_reached();
}

Handle allocUnlocked(Mode mode, size_t numBytes, size_t align) {
  Guard g(s_allocMutex);
  return alloc(mode, numBytes, align);
}

Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes, size_t align) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second;

  Guard g(s_allocMutex);
  if (s_linkTable.find(acc, key)) return acc->second;

  auto const retval = alloc(mode, sizeBytes, align);

  recordRds(retval, sizeBytes, key);
  if (!s_linkTable.insert(LinkTable::value_type(key, retval))) {
    always_assert(0);
  }
  return retval;
}

Handle attachImpl(Symbol key) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second;
  return kInvalidHandle;
}

void bindOnLinkImpl(std::atomic<Handle>& handle,
                    Mode mode,
                    size_t sizeBytes,
                    size_t align) {
  Guard g(s_allocMutex);
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

// All threads tl_bases are kept in a set, to allow iterating Local
// and Normal RDS sections across threads.
std::mutex s_tlBaseListLock;
std::vector<void*> s_tlBaseList;

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
  memset(tl_base, 0, s_normal_frontier);
}

void requestExit() {
  s_constants().detach(); // it will be swept
  // Don't bother running the dtor ...
}

void flush() {
  if (madvise(tl_base, s_normal_frontier, MADV_DONTNEED) == -1) {
    Logger::Warning("RDS madvise failure: %s\n",
                    folly::errnoStr(errno).c_str());
  }
  size_t offset = s_local_frontier & ~0xfff;
  if (madvise(static_cast<char*>(tl_base) + offset,
              s_persistent_base - offset, MADV_DONTNEED)) {
    Logger::Warning("RDS local madvise failure: %s\n",
                    folly::errnoStr(errno).c_str());
  }
}

size_t usedBytes() {
  return s_normal_frontier;
}

size_t usedLocalBytes() {
  return s_persistent_base - s_local_frontier;
}

size_t usedPersistentBytes() {
  return s_persistent_frontier - s_persistent_base;
}

folly::Range<const char*> normalSection() {
  return {(const char*)tl_base, usedBytes()};
}

folly::Range<const char*> localSection() {
  return {(const char*)s_local_frontier, usedLocalBytes()};
}

folly::Range<const char*> persistentSection() {
  return {(const char*)s_persistent_base, usedPersistentBytes()};
}

Array& s_constants() {
  void* vp = &s_constantsStorage;
  return *static_cast<Array*>(vp);
}

//////////////////////////////////////////////////////////////////////

size_t allocBit() {
  Guard g(s_allocMutex);
  if (!s_bits_to_go) {
    static const int kNumBytes = 512;
    static const int kNumBytesMask = kNumBytes - 1;
    s_next_bit = s_normal_frontier * CHAR_BIT;
    // allocate at least kNumBytes bytes, and make sure we end
    // on a 64 byte aligned boundary.
    int bytes = ((~s_normal_frontier + 1) & kNumBytesMask) + kNumBytes;
    s_bits_to_go = bytes * CHAR_BIT;
    s_normal_frontier += bytes;
    recordRds(s_normal_frontier - bytes, bytes, "Unknown", "bits");
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
  static_assert(std::is_unsigned<Handle>::value,
                "Handle is supposed to be unsigned");
  assert(handle < RuntimeOption::EvalJitTargetCacheSize);
  return handle >= (unsigned)s_persistent_base;
}

static void initPersistentCache() {
  Guard g(s_allocMutex);
  if (s_tc_fd) return;
  char tmpName[] = "/tmp/tcXXXXXX";
  s_tc_fd = mkstemp(tmpName);
  always_assert(s_tc_fd != -1);
  unlink(tmpName);
  s_persistent_base = RuntimeOption::EvalJitTargetCacheSize * 3 / 4;
  s_persistent_base -= s_persistent_base & (4 * 1024 - 1);
  ftruncate(s_tc_fd,
            RuntimeOption::EvalJitTargetCacheSize - s_persistent_base);
  s_local_frontier = s_persistent_frontier = s_persistent_base;
}

void threadInit() {
  assert(tl_base == nullptr);
  if (!s_tc_fd) {
    initPersistentCache();
  }

  tl_base = mmap(nullptr, RuntimeOption::EvalJitTargetCacheSize,
                 PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  always_assert_flog(
    tl_base != MAP_FAILED,
    "Failed to mmap persistent RDS region. errno = {}",
    folly::errnoStr(errno).c_str()
  );
#ifdef __CYGWIN__
  // MapViewOfFileEx() requires "the specified memory region is not already in
  // use by the calling process" when mapping the shared area below. Otherwise
  // it will return MAP_FAILED. We first map the full size to make sure the
  // memory area is available. Then we unmap and map the lower portion of the
  // RDS at the same address.
  munmap(tl_base, RuntimeOption::EvalJitTargetCacheSize);
  void* tl_same = mmap(tl_base, s_persistent_base,
                       PROT_READ | PROT_WRITE,
                       MAP_ANON | MAP_PRIVATE | MAP_FIXED,
                       -1, 0);
  always_assert(tl_same == tl_base);
#endif
  numa_bind_to(tl_base, s_persistent_base, s_numaNode);
  if (RuntimeOption::EvalMapTgtCacheHuge) {
    hintHuge(tl_base, RuntimeOption::EvalJitTargetCacheSize);
  }

  {
    Guard g(s_tlBaseListLock);
    assert(std::find(begin(s_tlBaseList), end(s_tlBaseList), tl_base) ==
             end(s_tlBaseList));
    s_tlBaseList.push_back(tl_base);
  }

  void* shared_base = (char*)tl_base + s_persistent_base;
  /*
   * Map the upper portion of the RDS to a shared area. This is used
   * for persistent classes and functions, so they are always defined,
   * and always visible to all threads.
   */
  void* mem = mmap(shared_base,
                   RuntimeOption::EvalJitTargetCacheSize - s_persistent_base,
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, s_tc_fd, 0);
  always_assert(mem == shared_base);

  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      tl_base,
      (char*)tl_base + RuntimeOption::EvalJitTargetCacheSize,
      "rds");
  }
}

void threadExit() {
  {
    Guard g(s_tlBaseListLock);
    auto it = std::find(begin(s_tlBaseList), end(s_tlBaseList), tl_base);
    if (it != end(s_tlBaseList)) {
      s_tlBaseList.erase(it);
    }
  }

  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      tl_base,
      (char*)tl_base + RuntimeOption::EvalJitTargetCacheSize,
      "-rds");
  }
#ifdef __CYGWIN__
  munmap(tl_base, s_persistent_base);
  munmap((char*)tl_base + s_persistent_base,
         RuntimeOption::EvalJitTargetCacheSize - s_persistent_base);
#else
  munmap(tl_base, RuntimeOption::EvalJitTargetCacheSize);
#endif
}

void recordRds(Handle h, size_t size,
               const std::string& type, const std::string& msg) {
  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      (char*)(intptr_t)h,
      (char*)(intptr_t)h + size,
      folly::format("rds+{}-{}", type, msg).str());
  }
}

void recordRds(Handle h, size_t size, const Symbol& sym) {
  if (RuntimeOption::EvalPerfDataMap) {
    recordRds(h, size,
              boost::apply_visitor(SymbolKind(), sym),
              boost::apply_visitor(SymbolRep(), sym));
  }
}

std::vector<void*> allTLBases() {
  Guard g(s_tlBaseListLock);
  return s_tlBaseList;
}

//////////////////////////////////////////////////////////////////////

}}
