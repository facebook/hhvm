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

#include <atomic>
#include <cassert>
#include <cstdio>
#include <map>
#include <mutex>
#include <vector>

#include <folly/Bits.h>
#include <folly/Hash.h>
#include <folly/portability/SysMman.h>
#include <folly/sorted_vector_types.h>
#include <folly/String.h>

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/logger.h"
#include "hphp/util/maphuge.h"
#include "hphp/util/numa.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/smalllocks.h"
#include "hphp/util/type-scan.h"

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/rds-symbol.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/jit/vm-protect.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/vm-regs.h"

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

struct HashCompare {
  bool equal(const Symbol& k1, const Symbol& k2) const {
    return symbol_eq(k1, k2);
  }
  size_t hash(const Symbol& k) const {
    return symbol_hash(k);
  }
};

struct LinkEntry {
  Handle   handle;
  uint32_t size;
};

using LinkTable = tbb::concurrent_hash_map<
  Symbol,
  LinkEntry,
  HashCompare
>;
LinkTable s_linkTable;

struct RevLinkEntry {
  uint32_t size;
  Symbol sym;
};
using RevLinkTable = std::map<Handle,RevLinkEntry>;
RevLinkTable s_handleTable;

__thread std::atomic<bool> s_hasFullInit{false};

struct StoreRevLink : boost::static_visitor<bool> {
  bool operator()(Profile) const { return false; }
  template<typename T>
  bool operator()(T) const { return true; }
};

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

#if RDS_FIXED_PERSISTENT_BASE
// Allocate 2M from low memory each time.
constexpr size_t kPersistentChunkSize = 1u << 20;
#endif
}

//////////////////////////////////////////////////////////////////////

namespace detail {

// Current allocation frontier for the non-persistent region.
size_t s_normal_frontier = sizeof(Header);

// Frontier for the "local" part of the persistent region (data not
// shared between threads, but not zero'd)---downward-growing.
size_t s_local_frontier = 0;
size_t s_local_base = 0;

#if !RDS_FIXED_PERSISTENT_BASE
uintptr_t s_persistent_base = 0;
size_t s_persistent_size = 0;
#else
// It is a constexpr equal to 0 defined in rds-inl.h
#endif

// Persistent region grows down from frontier towards limit, when it runs out of
// space, we can allocate another chunk and redefine the frontier and the limit,
// as guarded by s_allocMutex.
uintptr_t s_persistent_frontier = 0;
uintptr_t s_persistent_limit = 0;

size_t s_persistent_usage = 0;

AllocDescriptorList s_normal_alloc_descs;
AllocDescriptorList s_local_alloc_descs;

/*
 * Pushing a value into tbb::concurrent_vector is a racy operation; there is
 * period of time where the vector's size is increased (so iterators may see
 * the new value) but we haven't written data to it.
 *
 * To avoid reading these uninitialized values, we keep separate size bounds
 * on these vectors to use as secondary limits during iteration.
 */
std::atomic<size_t> s_normal_alloc_descs_size;
std::atomic<size_t> s_local_alloc_descs_size;

/*
 * Round base up to align, which must be a power of two.
 */
size_t roundUp(size_t base, size_t align) {
  assertx(folly::isPowTwo(align));
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
 * found instead, the remaining space before and/or after the return space is
 * re-added to the appropriate free lists.
 */
folly::Optional<Handle> findFreeBlock(FreeLists& lists, size_t size,
                                      size_t align) {
  for (auto it = lists.lower_bound(size); it != lists.end(); ++it) {
    auto const blockSize = it->first;
    for (auto list_it = it->second.begin();
         list_it != it->second.end();
         ++list_it) {
      auto const raw = static_cast<size_t>(*list_it);
      static_assert(sizeof(raw) > 4, "avoid 32-bit overflow");
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

// Create a new chunk for use in persistent RDS, but don't add to
// 's_persistent_free_lists' yet.
NEVER_INLINE void addNewPersistentChunk(size_t size) {
  assertx(size > 0 && size < kMaxHandle && size % 4096 == 0);
  auto const raw = static_cast<char*>(lower_malloc(size));
  auto const addr = reinterpret_cast<uintptr_t>(raw);
  memset(raw, 0, size);
#if !RDS_FIXED_PERSISTENT_BASE
  // This is only called once in processInit() if we don't have a persistent
  // base.
  always_assert(s_persistent_base == 0);
  s_persistent_limit = addr;
  s_persistent_frontier = addr + size;
  s_persistent_base = s_persistent_frontier - size4g;
#else
  always_assert_flog(addr >= kMinPersistentHandle && addr < size4g,
                     "failed to suitable address for persistent RDS");
  assertx(s_persistent_frontier >= s_persistent_limit);
  if (s_persistent_frontier != s_persistent_limit) {
    addFreeBlock(s_persistent_free_lists,
                 ptrToHandle<Mode::Persistent>(s_persistent_limit),
                 s_persistent_frontier - s_persistent_limit);
  }
  s_persistent_limit = addr;
  s_persistent_frontier = addr + size;
#endif
}

Handle alloc(Mode mode, size_t numBytes,
             size_t align, type_scan::Index tyIndex) {
  assertx(align <= 16);
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
          assertx(s_normal_alloc_descs_size.load(std::memory_order_acquire) ==
                  s_normal_alloc_descs.size());
          s_normal_alloc_descs.push_back(
            AllocDescriptor{Handle(handle), uint32_t(numBytes), tyIndex}
          );
          s_normal_alloc_descs_size.fetch_add(1, std::memory_order_acq_rel);
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
        assertx(s_normal_alloc_descs_size.load(std::memory_order_acquire) ==
                s_normal_alloc_descs.size());
        s_normal_alloc_descs.push_back(
          AllocDescriptor{Handle(handle), uint32_t(numBytes), tyIndex}
        );
        s_normal_alloc_descs_size.fetch_add(1, std::memory_order_acq_rel);
      }
      return handle;
    }
    case Mode::Persistent: {
      align = folly::nextPowTwo(align);
      always_assert(align <= numBytes);
      s_persistent_usage += numBytes;

      if (auto free = findFreeBlock(s_persistent_free_lists, numBytes, align)) {
        return *free;
      }

      auto const newFrontier =
        (s_persistent_frontier - numBytes) & ~(align - 1);
      if (newFrontier >= s_persistent_limit) {
        s_persistent_frontier = newFrontier;
        return ptrToHandle<Mode::Persistent>(newFrontier);
      }

#if RDS_FIXED_PERSISTENT_BASE
      // Allocate on demand, add kPersistentChunkSize each time.
      assertx(numBytes <= kPersistentChunkSize);
      addNewPersistentChunk(kPersistentChunkSize);
      return alloc(mode, numBytes, align, tyIndex); // retry after a new chunk
#else
      // We reserved plenty of space in s_persistent_free_lists in the beginning
      // of the process, but maybe it is time to increase the size in the
      // config.
      always_assert_flog(
        false,
        "Ran out of RDS space (mode=Persistent)"
      );
#endif
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
        assertx(s_local_alloc_descs_size.load(std::memory_order_acquire) ==
                s_local_alloc_descs.size());
        s_local_alloc_descs.push_back(
          AllocDescriptor{Handle(frontier), uint32_t(numBytes), tyIndex}
        );
        s_local_alloc_descs_size.fetch_add(1, std::memory_order_acq_rel);
      }
      return frontier;
    }
    default:
      not_reached();
  }
}

Handle allocUnlocked(Mode mode, size_t numBytes,
                     size_t align, type_scan::Index tyIndex) {
  Guard g(s_allocMutex);
  return alloc(mode, numBytes, align, tyIndex);
}

Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes,
                size_t align, type_scan::Index tyIndex) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second.handle;

  Guard g(s_allocMutex);
  if (s_linkTable.find(acc, key)) return acc->second.handle;

  auto const handle = alloc(mode, sizeBytes, align, tyIndex);
  recordRds(handle, sizeBytes, key);

  LinkTable::const_accessor insert_acc;
  // insert_acc is held until after s_handleTable is updated
  if (!s_linkTable.insert(
        insert_acc,
        LinkTable::value_type(key, {handle, safe_cast<uint32_t>(sizeBytes)}))) {
    always_assert(0);
  }
  if (boost::apply_visitor(StoreRevLink(), key)) {
    s_handleTable.emplace(handle, RevLinkEntry {
      safe_cast<uint32_t>(sizeBytes), key
    });
  }
  return handle;
}

Handle attachImpl(Symbol key) {
  LinkTable::const_accessor acc;
  if (s_linkTable.find(acc, key)) return acc->second.handle;
  return kUninitHandle;
}

NEVER_INLINE
void bindOnLinkImpl(std::atomic<Handle>& handle,
                    Symbol sym, Mode mode, size_t size, size_t align,
                    type_scan::Index tsi, const void* init_val) {
  Handle c = kUninitHandle;
  if (handle.compare_exchange_strong(c, kBeingBound,
                                     std::memory_order_relaxed,
                                     std::memory_order_relaxed)) {
    // we flipped it from kUninitHandle, so we get to fill in the value.
    auto const h = allocUnlocked(mode, size, align, tsi);
    recordRds(h, size, sym);

    if (init_val != nullptr && isPersistentHandle(h)) {
      memcpy(handleToPtr<void, Mode::Persistent>(h), init_val, size);
    }
    if (handle.exchange(h, std::memory_order_relaxed) ==
        kBeingBoundWithWaiters) {
      futex_wake(&handle, INT_MAX);
    }
    return;
  }
  // Someone else beat us to it, so wait until they've filled it in.
  if (c == kBeingBound) {
    handle.compare_exchange_strong(c, kBeingBoundWithWaiters,
                                   std::memory_order_relaxed,
                                   std::memory_order_relaxed);
  }
  while (handle.load(std::memory_order_relaxed) == kBeingBoundWithWaiters) {
    futex_wait(&handle, kBeingBoundWithWaiters);
  }
  assertx(isHandleBound(handle.load(std::memory_order_relaxed)));
}

}

void unbind(Symbol key, Handle handle) {
  Guard g(s_allocMutex);
  s_linkTable.erase(key);
  s_handleTable.erase(handle);
}

using namespace detail;

void visitSymbols(std::function<void(const Symbol&,Handle,uint32_t)> fun) {
  Guard g(s_allocMutex);
  // make sure that find/count don't interfere with iteration.
  s_linkTable.rehash();
  for (auto it : s_linkTable) {
    fun(it.first, it.second.handle, it.second.size);
  }
}

//////////////////////////////////////////////////////////////////////

__thread void* tl_base = nullptr;

rds::Link<bool, Mode::Persistent> s_persistentTrue;

// All threads tl_bases are kept in a set, to allow iterating Local
// and Normal RDS sections across threads.
std::mutex s_tlBaseListLock;
std::vector<void*> s_tlBaseList;

//////////////////////////////////////////////////////////////////////

static size_t s_next_bit;
static size_t s_bits_to_go;

//////////////////////////////////////////////////////////////////////

void processInit() {
  assertx(!s_local_base);
  if (RuntimeOption::EvalRDSSize > 1u << 30) {
    // The encoding of RDS handles require that the normal and local regions
    // together be smaller than 1G.
    RuntimeOption::EvalRDSSize = 1u << 30;
  }
  s_local_base = RuntimeOption::EvalRDSSize * 3 / 4;
  s_local_frontier = s_local_base;

#if RDS_FIXED_PERSISTENT_BASE
  auto constexpr allocSize = kPersistentChunkSize;
#else
  auto const allocSize = RuntimeOption::EvalRDSSize / 4;
#endif
  addNewPersistentChunk(allocSize),

  s_persistentTrue.bind(Mode::Persistent, LinkID{"RDSTrue"});
  *s_persistentTrue = true;

  local::RDSInit();
}

void requestInit() {
  assertx(tl_base);

  auto gen = header()->currentGen;
  memset(tl_base, 0, sizeof(Header));
  if (debug) {
    // Trash the normal section in debug mode, so that we can catch errors with
    // not checking the gen number quickly.
    memset(
      static_cast<char*>(tl_base) + sizeof(Header),
      kRDSTrashFill,
      s_normal_frontier - sizeof(Header)
    );
    gen = 1;
  } else if (++gen == kInvalidGenNumber) {
    // If the current gen number has wrapped around back to the "invalid"
    // number, memset the entire normal section.  Once the current gen number
    // wraps, it becomes ambiguous whether any given gen number is up to date.
    memset(
      static_cast<char*>(tl_base) + sizeof(Header),
      kInvalidGenNumber,
      s_normal_frontier - sizeof(Header)
    );
    ++gen;
  }
  header()->currentGen = gen;
}

void requestExit() {
  // Don't bother running the dtor ...
}

void flush() {
  if (madvise(tl_base, s_normal_frontier, MADV_DONTNEED) == -1) {
    Logger::Warning("RDS madvise failure: %s\n",
                    folly::errnoStr(errno).c_str());
  }
  if (jit::mcgen::retranslateAllEnabled() &&
      !jit::mcgen::retranslateAllPending()) {
    size_t offset = s_local_frontier & ~0xfff;
    size_t protectedSpace = local::detail::s_usedbytes +
                            (-local::detail::s_usedbytes & 0xfff);
    if (madvise(static_cast<char*>(tl_base) + offset,
                s_local_base - protectedSpace - offset,
                MADV_DONTNEED)) {
      Logger::Warning("RDS local madvise failure: %s\n",
                      folly::errnoStr(errno).c_str());
    }
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
 * +-------------+ <-- tl_base + s_local_base
 * | \ \ \ \ \ \ |
 * +-------------+ higher addresses
 *
 * +-------------+ <--- s_persistent_base
 * |             |
 * | Persistent  | not necessarily contiguous when RDS_FIXED_PERSISTENT_BASE
 * |     region  |
 * |             |
 * +-------------+
 */

size_t usedBytes() {
  return s_normal_frontier;
}

size_t usedLocalBytes() {
  return s_local_base - s_local_frontier;
}

size_t usedPersistentBytes() {
  return s_persistent_usage;
}

folly::Range<const char*> normalSection() {
  return {(const char*)tl_base, usedBytes()};
}

folly::Range<const char*> localSection() {
  return {(const char*)tl_base + s_local_frontier, usedLocalBytes()};
}

GenNumber currentGenNumber() {
  return header()->currentGen;
}

Handle currentGenNumberHandle() {
  return offsetof(Header, currentGen);
}

constexpr size_t kAllocBitNumBytes = 8;

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
    auto ptr = handleToPtr<unsigned char, Mode::Normal>(handle);
    memset(ptr, 0, kAllocBitNumBytes);
    initHandle(handle);
  }
  auto& ref = handleToRef<unsigned char, Mode::Normal>(block);
  bool ret = ref & mask;
  ref |= mask;
  return ret;
}

bool isValidHandle(Handle handle) {
  return handle >= kMinPersistentHandle ||
    (handle >= sizeof(Header) && handle < s_normal_frontier) ||
    (handle >= s_local_frontier && handle < s_local_base);
}

void threadInit(bool shouldRegister) {
  if (!s_local_base) {
    processInit();
  }
  assertx(tl_base == nullptr);
  tl_base = mmap(nullptr, s_local_base, PROT_READ | PROT_WRITE,
                 MAP_ANON | MAP_PRIVATE, -1, 0);
  always_assert_flog(
    tl_base != MAP_FAILED,
    "Failed to mmap RDS region. errno = {}",
    folly::errnoStr(errno).c_str()
  );
  numa_bind_to(tl_base, s_local_base, s_numaNode);
#ifdef NDEBUG
  // A huge-page RDS is incompatible with VMProtect in vm-regs.cpp
  if (RuntimeOption::EvalMapTgtCacheHuge) {
    hintHuge(tl_base, s_local_base);
  }
#endif

  if (shouldRegister) {
    Guard g(s_tlBaseListLock);
    assertx(std::find(begin(s_tlBaseList), end(s_tlBaseList), tl_base) ==
            end(s_tlBaseList));
    s_tlBaseList.push_back(tl_base);
  }

  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      tl_base,
      (char*)tl_base + s_local_base,
      "rds");
  }

  header()->currentGen = 1;
  if (shouldRegister) {
    local::init();
    s_hasFullInit.store(true, std::memory_order_release);
  }
}

void threadExit(bool shouldUnregister) {
  if (shouldUnregister) {
    s_hasFullInit.store(false, std::memory_order_release);
    local::fini(true);
    Guard g(s_tlBaseListLock);
    auto it = std::find(begin(s_tlBaseList), end(s_tlBaseList), tl_base);
    if (it != end(s_tlBaseList)) {
      s_tlBaseList.erase(it);
    }
  }

  if (RuntimeOption::EvalPerfDataMap) {
    Debug::DebugInfo::recordDataMap(
      tl_base,
      (char*)tl_base + s_local_base,
      "-rds");
  }

  auto const base = tl_base;
  auto do_unmap = [base] {
    munmap(base, s_local_base);
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

bool isFullyInitialized() {
  return s_hasFullInit.load(std::memory_order_acquire);
}

void recordRds(Handle h, size_t size,
               folly::StringPiece type, folly::StringPiece msg) {
  if (RuntimeOption::EvalPerfDataMap) {
    if (isNormalHandle(h)) {
      h = genNumberHandleFrom(h);
      size += sizeof(GenNumber);
    }
    Debug::DebugInfo::recordDataMap(
      (char*)(intptr_t)h,
      (char*)(intptr_t)h + size,
      folly::sformat("rds+{}-{}", type, msg));
  }
}

void recordRds(Handle h, size_t size, const Symbol& sym) {
  if (RuntimeOption::EvalPerfDataMap) {
    recordRds(h, size, symbol_kind(sym), symbol_rep(sym));
  }
}

std::vector<void*> allTLBases() {
  Guard g(s_tlBaseListLock);
  return s_tlBaseList;
}

folly::Optional<Symbol> reverseLink(Handle handle) {
  Guard g(s_allocMutex);
  auto const it = s_handleTable.lower_bound(handle);
  if (it == s_handleTable.end()) return folly::none;
  if (it->first + it->second.size < handle) return folly::none;
  return it->second.sym;
}

namespace {
local::RegisterConfig s_rdsLocalConfigRegistration({
  .rdsInitFunc =
    [] (size_t size) -> uint32_t {
      return rds::detail::allocUnlocked(rds::Mode::Local,
                                        std::max(size, 16UL), 16U,
                                        type_scan::kIndexUnknown);
    },
  .initFunc =
    [](size_t size, uint32_t handle) -> void* {
      if (rds::tl_base) {
        return rds::handleToPtr<void, rds::Mode::Local>(handle);
      }
      return local_malloc(size);
    },
  .finiFunc =
    [](void* ptr) -> void{
      local_free(ptr);
    },
  .inRdsFunc =
    [](void* ptr, size_t size) -> bool {
      return tl_base &&
             std::less_equal<void>()(localSection().cbegin(), ptr)
                && std::less_equal<void>()(
                  (const char*)ptr
                  + size, localSection().cend());
    },
  .initRequestEventHandler =
    [](RequestEventHandler* h) -> void {
      h->setInited(true);
      // This registration makes sure obj->requestShutdown() will be called.
      // Do it before calling requestInit() so that obj is reachable to the
      // GC no matter what the callback does.
      auto index = g_context->registerRequestEventHandler(h);
      SCOPE_FAIL {
        h->setInited(false);
        g_context->unregisterRequestEventHandler(h, index);
      };

      h->requestInit();
    }
});
}

//////////////////////////////////////////////////////////////////////

}}
