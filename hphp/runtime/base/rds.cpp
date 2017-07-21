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
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <cassert>
#include <cstdio>
#include <mutex>
#include <atomic>
#include <vector>

#ifndef _MSC_VER
#include <execinfo.h>
#endif

#include <folly/sorted_vector_types.h>
#include <folly/String.h>
#include <folly/Hash.h>
#include <folly/Bits.h>
#include <folly/portability/SysMman.h>

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/logger.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/numa.h"
#include "hphp/util/type-scan.h"

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/jit/vm-protect.h"

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

using Guard = std::lock_guard<std::mutex>;

/*
 * This mutex protects actually allocating from RDS (the above
 * statics).  It is ordered *after* the locks in s_linkTable.
 */
std::mutex s_allocMutex;

//////////////////////////////////////////////////////////////////////

struct SymbolKind : boost::static_visitor<std::string> {
  std::string operator()(StaticLocal /*k*/) const { return "StaticLocal"; }
  std::string operator()(ClsConstant /*k*/) const { return "ClsConstant"; }
  std::string operator()(StaticMethod /*k*/) const { return "StaticMethod"; }
  std::string operator()(StaticMethodF /*k*/) const { return "StaticMethodF"; }
  std::string operator()(Profile /*k*/) const { return "Profile"; }
  std::string operator()(SPropCache /*k*/) const { return "SPropCache"; }
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
  std::string operator()(SPropCache k) const {
    return k.cls->name()->toCppString() + "::" +
           k.cls->staticProperties()[k.slot].name->toCppString();
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

  bool operator()(SPropCache k1, SPropCache k2) const {
    return k1.cls == k2.cls && k1.slot == k2.slot;
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

  size_t operator()(StaticMethod k)  const { return k.name->hash(); }
  size_t operator()(StaticMethodF k) const { return k.name->hash(); }

  size_t operator()(SPropCache k) const {
    return folly::hash::hash_combine(
      k.cls, k.slot
    );
  }

};

struct HashCompare {
  bool equal(const Symbol& k1, const Symbol& k2) const {
    return boost::apply_visitor(SymbolEq(), k1, k2);
  }

  size_t hash(const Symbol& k) const {
    return boost::apply_visitor(SymbolHash(), k);
  }
};

using LinkTable = tbb::concurrent_hash_map<
  Symbol,
  Handle,
  HashCompare
>;
LinkTable s_linkTable;

using RevLinkTable = tbb::concurrent_hash_map<Handle,Symbol>;
RevLinkTable s_handleTable;

//////////////////////////////////////////////////////////////////////

/*
 * Space wasted by alignment is tracked in these maps. We don't bother with
 * free lists for local RDS because we aren't sensitive to its layout or
 * compactness.
 */
using FreeLists = folly::sorted_vector_map<unsigned,
                                           std::deque<rds::Handle>>;
FreeLists s_normal_free_lists;
FreeLists s_persistent_free_lists;

}

//////////////////////////////////////////////////////////////////////

namespace detail {

// Current allocation frontier for the non-persistent region.
size_t s_normal_frontier = sizeof(Header);

// Frontier and base of the persistent region.
size_t s_persistent_base = 0;
size_t s_persistent_frontier = 0;

// Frontier for the "local" part of the persistent region (data not
// shared between threads, but not zero'd)---downward-growing.
size_t s_local_frontier = 0;

Link<GenNumber> g_current_gen_link{kInvalidHandle};

AllocDescriptorList s_normal_alloc_descs;
AllocDescriptorList s_local_alloc_descs;

/*
 * Round base up to align, which must be a power of two.
 */
size_t roundUp(size_t base, size_t align) {
  assert(folly::isPowTwo(align));
  --align;
  return (base + align) & ~align;
}

/*
 * Add the given offset to the free list for its size.
 */
void addFreeBlock(FreeLists& lists, size_t where, size_t size) {
  if (size == 0) return;
  lists[size].emplace_back(where);
}

/*
 * Try to find a tracked free block of a suitable size. If an oversized block is
 * found instead, the remaining space before and/or after the return space it
 * re-added to the appropriate free lists.
 */
folly::Optional<Handle> findFreeBlock(FreeLists& lists, size_t size,
                                      size_t align) {
  for (auto it = lists.lower_bound(size); it != lists.end(); ++it) {
    for (auto list_it = it->second.begin();
         list_it != it->second.end();
         ++list_it) {
      auto const blockSize = it->first;
      auto const raw = *list_it;
      auto const end = raw + blockSize;

      auto const handle = roundUp(raw, align);

      if (handle + size > end) continue;
      it->second.erase(list_it);

      auto const headerSize = handle - raw;
      addFreeBlock(lists, raw, headerSize);

      auto const footerSize = blockSize - size - headerSize;
      addFreeBlock(lists, handle + size, footerSize);

      return handle;
    }
  }
  return folly::none;
}

Handle alloc(Mode mode, size_t numBytes,
             size_t align, type_scan::Index tyIndex) {
  switch (mode) {
    case Mode::Normal: {
      align = folly::nextPowTwo(std::max(align, alignof(GenNumber)));
      auto const prefix = roundUp(sizeof(GenNumber), align);
      auto const adjBytes = numBytes + prefix;
      always_assert(align <= adjBytes);

      if (auto free = findFreeBlock(s_normal_free_lists, adjBytes, align)) {
        auto const begin = *free;
        addFreeBlock(s_normal_free_lists, begin, prefix - sizeof(GenNumber));
        auto const handle = begin + prefix;
        if (type_scan::hasScanner(tyIndex)) {
          s_normal_alloc_descs.push_back(
            AllocDescriptor{Handle(handle), uint32_t(numBytes), tyIndex}
          );
        }
        return handle;
      }

      auto const oldFrontier = s_normal_frontier;
      s_normal_frontier = roundUp(s_normal_frontier, align);

      addFreeBlock(s_normal_free_lists, oldFrontier,
                  s_normal_frontier - oldFrontier);
      s_normal_frontier += adjBytes;
      if (debug && !jit::VMProtect::is_protected) {
        memset(
          (char*)(tl_base) + oldFrontier,
          kRDSTrashFill,
          s_normal_frontier - oldFrontier
        );
      }
      always_assert_flog(
        s_normal_frontier < s_local_frontier,
        "Ran out of RDS space (mode=Normal)"
      );

      auto const begin = s_normal_frontier - adjBytes;
      addFreeBlock(s_normal_free_lists, begin, prefix - sizeof(GenNumber));

      auto const handle = begin + prefix;

      if (type_scan::hasScanner(tyIndex)) {
        s_normal_alloc_descs.push_back(
          AllocDescriptor{Handle(handle), uint32_t(numBytes), tyIndex}
        );
      }
      return handle;
    }
    case Mode::Persistent: {
      align = folly::nextPowTwo(align);
      always_assert(align <= numBytes);

      if (auto free = findFreeBlock(s_persistent_free_lists, numBytes, align)) {
        return *free;
      }

      // Note: it's ok not to zero new allocations, because we've never done
      // anything with this part of the page yet, so it must still be zero.
      auto const oldFrontier = s_persistent_frontier;
      s_persistent_frontier = roundUp(s_persistent_frontier, align);
      addFreeBlock(s_persistent_free_lists, oldFrontier,
                   s_persistent_frontier - oldFrontier);
      s_persistent_frontier += numBytes;

      always_assert_flog(
        s_persistent_frontier < RuntimeOption::EvalJitTargetCacheSize,
        "Ran out of RDS space (mode=Persistent)"
      );

      return s_persistent_frontier - numBytes;
    }
    case Mode::Local: {
      align = folly::nextPowTwo(align);
      always_assert(align <= numBytes);

      auto& frontier = s_local_frontier;

      frontier -= numBytes;
      frontier &= ~(align - 1);

      always_assert_flog(
        frontier >= s_normal_frontier,
        "Ran out of RDS space (mode=Local)"
      );
      if (type_scan::hasScanner(tyIndex)) {
        s_local_alloc_descs.push_back(
          AllocDescriptor{Handle(frontier), uint32_t(numBytes), tyIndex}
        );
      }
      return frontier;
    }
  }

  not_reached();
}

Handle allocUnlocked(Mode mode, size_t numBytes,
                     size_t align, type_scan::Index tyIndex) {
  Guard g(s_allocMutex);
  return alloc(mode, numBytes, align, tyIndex);
}

Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes,
                size_t align, type_scan::Index tyIndex) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second;

  Guard g(s_allocMutex);
  if (s_linkTable.find(acc, key)) return acc->second;

  auto const handle = alloc(mode, sizeBytes, align, tyIndex);
  recordRds(handle, sizeBytes, key);

  LinkTable::const_accessor insert_acc;
  // insert_acc lives until after s_handleTable is updated
  if (!s_linkTable.insert(insert_acc, LinkTable::value_type(key, handle))) {
    always_assert(0);
  }
  if (type_scan::hasScanner(tyIndex)) {
    s_handleTable.insert(std::make_pair(handle, key));
  }
  return handle;
}

Handle attachImpl(Symbol key) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second;
  return kInvalidHandle;
}

void bindOnLinkImpl(std::atomic<Handle>& handle,
                    Mode mode,
                    size_t sizeBytes,
                    size_t align,
                    type_scan::Index tyIndex) {
  Guard g(s_allocMutex);
  if (handle.load(std::memory_order_relaxed) == kInvalidHandle) {
    handle.store(alloc(mode, sizeBytes, align, tyIndex),
                 std::memory_order_relaxed);
  }
}

}

void unbind(Symbol key, Handle handle) {
  Guard g(s_allocMutex);
  s_linkTable.erase(key);
  s_handleTable.erase(handle);
}

using namespace detail;

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
  assert(g_current_gen_link.bound());

  memset(tl_base, 0, sizeof(Header));
  if (debug) {
    // Trash the normal section in debug mode, so that we can catch errors with
    // not checking the gen number quickly.
    memset(
      static_cast<char*>(tl_base) + sizeof(Header),
      kRDSTrashFill,
      s_normal_frontier - sizeof(Header)
    );
    *g_current_gen_link = 1;
    return;
  } else if (++*g_current_gen_link == kInvalidGenNumber) {
    // If the current gen number has wrapped around back to the "invalid"
    // number, memset the entire normal section.  Once the current gen number
    // wraps, it becomes ambiguous whether any given gen number is up to date.
    memset(
      static_cast<char*>(tl_base) + sizeof(Header),
      kInvalidGenNumber,
      s_normal_frontier - sizeof(Header)
    );
    ++*g_current_gen_link;
  }
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

/* RDS Layout:
 * +-------------+ <-- tl_base
 * |  Header     |
 * +-------------+
 * |             |
 * |  Normal     | growing higher
 * |    region   | vvv
 * |             |
 * +-------------+ <-- tl_base + s_normal_frontier
 * | \ \ \ \ \ \ |
 * +-------------+ <-- tl_base + s_local_frontier
 * |             |
 * |  Local      | ^^^
 * |    region   | growing lower
 * |             |
 * +-------------+ <-- tl_base + s_persistent_base
 * |             |
 * | Persistent  | growing higher
 * |     region  | vvv
 * |             |
 * +-------------+ <-- tl_base + s_persistent_frontier
 * | \ \ \ \ \ \ |
 * +-------------+ higher addresses
 */

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
  return {(const char*)tl_base + s_local_frontier, usedLocalBytes()};
}

folly::Range<const char*> persistentSection() {
  return {(const char*)tl_base + s_persistent_base, usedPersistentBytes()};
}

Array& s_constants() {
  void* vp = &s_constantsStorage;
  return *static_cast<Array*>(vp);
}

//////////////////////////////////////////////////////////////////////

namespace {

constexpr std::size_t kAllocBitNumBytes = 8;

}

/////////////////////////////////////////////////////////////////////

size_t allocBit() {
  Guard g(s_allocMutex);
  if (s_bits_to_go == 0) {
    auto const handle = detail::alloc(
      Mode::Normal,
      kAllocBitNumBytes,
      kAllocBitNumBytes,
      type_scan::getIndexForScan<unsigned char[kAllocBitNumBytes]>()
    );
    s_next_bit = handle * CHAR_BIT;
    s_bits_to_go = kAllocBitNumBytes * CHAR_BIT;
    recordRds(handle, kAllocBitNumBytes, "Unknown", "bits");
  }
  s_bits_to_go--;
  return s_next_bit++;
}

bool testAndSetBit(size_t bit) {
  size_t block = bit / CHAR_BIT;
  unsigned char mask = 1 << (bit % CHAR_BIT);
  Handle handle = block & ~(kAllocBitNumBytes - 1);

  if (!isHandleInit(handle, NormalTag{})) {
    auto ptr = &handleToRef<unsigned char>(handle);
    for (size_t i = 0; i < kAllocBitNumBytes; ++i) ptr[i] = 0;
    initHandle(handle);
  }
  bool ret = handleToRef<unsigned char>(block) & mask;
  handleToRef<unsigned char>(block) |= mask;
  return ret;
}

bool isValidHandle(Handle handle) {
  return handle >= sizeof(Header) &&
    handle < RuntimeOption::EvalJitTargetCacheSize;
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

void threadInit(bool shouldRegister) {
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
#ifdef _MSC_VER
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
#ifdef NDEBUG
  // A huge-page RDS is incompatible with VMProtect in vm-regs.cpp
  if (RuntimeOption::EvalMapTgtCacheHuge) {
    hintHuge(tl_base, RuntimeOption::EvalJitTargetCacheSize);
  }
#endif

  if (shouldRegister) {
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

  g_current_gen_link.bind(Mode::Local);
  *g_current_gen_link = 1;
  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      (char*)tl_base + g_current_gen_link.handle(),
      (char*)tl_base + g_current_gen_link.handle() +
      RuntimeOption::EvalJitTargetCacheSize,
      "-rds-gen-number");
  }
}

void threadExit(bool shouldUnregister) {
  if (shouldUnregister) {
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

  auto const base = tl_base;
  auto do_unmap = [base] {
#ifdef _MSC_VER
    munmap(base, s_persistent_base);
    munmap((char*)base + s_persistent_base,
           RuntimeOption::EvalJitTargetCacheSize - s_persistent_base);
#else
    munmap(base, RuntimeOption::EvalJitTargetCacheSize);
#endif
  };

  // Other requests may be reading from this rds section via the s_tlBaseList.
  // We just removed ourself from the list now, but defer the unmap until after
  // any outstanding requests have completed.
  if (shouldUnregister) {
    Treadmill::enqueue(std::move(do_unmap));
  } else {
    do_unmap();
  }
}

void recordRds(Handle h, size_t size,
               const std::string& type, const std::string& msg) {
  if (RuntimeOption::EvalPerfDataMap) {
    if (isNormalHandle(h)) {
      h = genNumberHandleFrom(h);
      size += sizeof(GenNumber);
    }
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

folly::Optional<Symbol> reverseLink(Handle handle) {
  RevLinkTable::const_accessor acc;
  if (s_handleTable.find(acc, handle)) {
    return acc->second;
  }
  return folly::none;
}

//////////////////////////////////////////////////////////////////////

}}
