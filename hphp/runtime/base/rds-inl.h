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
#pragma once

#include "hphp/util/compilation-flags.h"
#include "hphp/util/safe-cast.h"

#include <tbb/concurrent_vector.h>

#include <atomic>

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

namespace detail {

Handle alloc(Mode mode, size_t numBytes, size_t align,
             type_scan::Index tyIndex, const Symbol* symbol);
Handle allocUnlocked(Mode mode, size_t numBytes, size_t align,
                     type_scan::Index tyIndex,
                     const Symbol* symbol);
Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes,
                size_t align, type_scan::Index tyIndex);
Handle attachImpl(Symbol key);

void bindOnLinkImpl(std::atomic<Handle>& handle,
                    Symbol key, Mode mode, size_t size, size_t align,
                    type_scan::Index tsi, const void* init_val);

extern size_t s_normal_frontier;
extern size_t s_local_base;
extern size_t s_local_frontier;
constexpr size_t size4g = 1ull << 32;
#if RDS_FIXED_PERSISTENT_BASE
constexpr uintptr_t s_persistent_base = 0;
#else
extern uintptr_t s_persistent_base;
extern size_t s_persistent_size;
#endif

struct AllocDescriptor {
  Handle handle;
  uint32_t size;
  type_scan::Index index;
};
using AllocDescriptorList = tbb::concurrent_vector<AllocDescriptor>;
extern AllocDescriptorList s_normal_alloc_descs;
extern AllocDescriptorList s_local_alloc_descs;

// See comments in rds.cpp for why these are necessary.
extern std::atomic<size_t> s_normal_alloc_descs_size;
extern std::atomic<size_t> s_local_alloc_descs_size;

}

//////////////////////////////////////////////////////////////////////

template<class T, Mode M, bool P>
T* handleToPtr(void* base, Handle h) {
  using namespace detail;
  if (P && UNLIKELY(shouldProfileAccesses())) markAccess(h);
  if (M == Mode::Persistent) {
    assertx(isPersistentHandle(h));
    return reinterpret_cast<T*>(s_persistent_base + h);
  }
  if (maybe<Mode::Persistent>(M) && isPersistentHandle(h)) {
    return reinterpret_cast<T*>(s_persistent_base + h);
  }
  assertx(maybe<Mode::NonPersistent>(M));
  assertx(!isPersistentHandle(h));
  void* vp = static_cast<char*>(base) + h;
  return reinterpret_cast<T*>(vp);
}

template<class T, Mode M, bool P>
T* handleToPtr(Handle h) {
  return handleToPtr<T, M, P>(tl_base, h);
}

template<class T, Mode M, bool P>
T& handleToRef(void* base, Handle h) {
  return *handleToPtr<T, M, P>(base, h);
}

template<class T, Mode M, bool P>
T& handleToRef(Handle h) {
  return *handleToPtr<T, M, P>(h);
}

template<Mode M>
Handle ptrToHandle(const void* ptr) {
  using namespace detail;
  auto const iptr = reinterpret_cast<uintptr_t>(ptr);
  if (M == Mode::Persistent) {
    auto h = safe_cast<Handle>(iptr - s_persistent_base);
    assertx(isPersistentHandle(h));
    return h;
  }
  if (maybe<Mode::Persistent>(M)) {
#if RDS_FIXED_PERSISTENT_BASE
    if (iptr < s_persistent_base + size4g) {
      auto h = safe_cast<Handle>(iptr);
      assertx(isPersistentHandle(h));
      return h;
    }
#else
    if (iptr < s_persistent_base + size4g &&
        iptr >= s_persistent_base + size4g - s_persistent_size) {
      auto h = safe_cast<Handle>(iptr - s_persistent_base);
      assertx(isPersistentHandle(h));
      return h;
    }
#endif
  }
  assertx(maybe<Mode::NonPersistent>(M));
  auto h = safe_cast<Handle>(iptr - reinterpret_cast<uintptr_t>(tl_base));
  assertx(!isPersistentHandle(h));
  return h;
}

template<Mode M>
Handle ptrToHandle(uintptr_t ptr) {
  return ptrToHandle<M>(reinterpret_cast<void*>(ptr));
}

template<class T, Mode M>
Link<T,M>::Link(Handle handle) : m_handle(handle) {
  checkSanity();
}

template<class T, Mode M>
Link<T,M>::Link(const Link<T,M>& l) : m_handle(l.raw()) {}

template<class T, Mode M>
template<Mode OM>
Link<T,M>::Link(
    const Link<T,OM>& l,
    typename std::enable_if<in<M>(OM), bool>::type)
  : m_handle(l.raw()) {}

template<class T, Mode M>
Link<T,M>& Link<T,M>::operator=(const Link<T,M>& l) {
  if (debug) {
    auto const DEBUG_ONLY old =
      m_handle.exchange(l.raw(), std::memory_order_relaxed);
    assertx(raw() != kBeingBound && raw() != kBeingBoundWithWaiters &&
            old != kBeingBound && old != kBeingBoundWithWaiters);
  } else {
    m_handle.store(l.raw(), std::memory_order_relaxed);
  }
  return *this;
}

template<class T, Mode M>
template<Mode OM>
typename std::enable_if<in<M>(OM),Link<T,M>>::type&
Link<T,M>::operator=(const Link<T,OM>& l) {
  if (debug) {
    auto const DEBUG_ONLY old =
      m_handle.exchange(l.raw(), std::memory_order_relaxed);
    assertx(raw() != kBeingBound && raw() != kBeingBoundWithWaiters &&
            old != kBeingBound && old != kBeingBoundWithWaiters);
  } else {
    m_handle.store(l.raw(), std::memory_order_relaxed);
  }
  return *this;
}

template<class T, Mode M>
T& Link<T,M>::operator*() const { return *get(); }

template<class T, Mode M>
T* Link<T,M>::operator->() const { return get(); }

template<class T, Mode M>
T* Link<T,M>::get() const {
  return handleToPtr<T, M>(handle());
}

template<class T, Mode M>
T* Link<T,M>::getNoProfile() const {
  return handleToPtr<T, M, false>(handle());
}

template<class T, Mode M>
bool Link<T,M>::bound() const {
  return isHandleBound(raw());
}

template<class T, Mode M>
Handle Link<T,M>::handle() const {
  auto const handle = raw();
  assertx(isHandleBound(handle));
  return handle;
}

template<class T, Mode M>
Handle Link<T,M>::maybeHandle() const {
  auto const handle = raw();
  return isHandleBound(handle) ? handle : kUninitHandle;
}

template<class T, Mode M>
Handle Link<T,M>::genNumberHandle() const {
  return genNumberHandleFrom(handle());
}

template<class T, Mode M>
GenNumber Link<T,M>::genNumber() const {
  return genNumberOf(handle());
}

template<class T, Mode M>
bool Link<T,M>::isInit() const {
  return !maybe<Mode::Normal>(M) ||
    (M == Mode::Normal && isHandleInit(handle(), NormalTag{})) ||
    isHandleInit(handle());
}

template<class T, Mode M>
bool Link<T,M>::isInitNoProfile() const {
  return !maybe<Mode::Normal>(M) ||
    (M == Mode::Normal && isHandleInitNoProfile(handle(), NormalTag{})) ||
    isHandleInitNoProfile(handle());
}

template<class T, Mode M>
void Link<T,M>::markInit() const {
  initHandle(handle());
}

template<class T, Mode M>
void Link<T,M>::markUninit() const {
  uninitHandle(handle());
}

template<class T, Mode M>
void Link<T,M>::initWith(const T& val) const {
  new (get()) T(val);
  if (isNormal()) markInit();
}

template<class T, Mode M>
void Link<T,M>::initWith(T&& val) const {
  new (get()) T(std::move(val));
  if (isNormal()) markInit();
}

template <class T, Mode M>
bool Link<T,M>::isNormal() const {
  return M == Mode::Normal ||
    (maybe<Mode::Normal>(M) && isNormalHandle(handle()));
}

template <class T, Mode M>
bool Link<T,M>::isLocal() const {
  return M == Mode::Local ||
    (maybe<Mode::Local>(M) && isLocalHandle(handle()));
}

template<class T, Mode M>
bool Link<T,M>::isPersistent() const {
  return M == Mode::Persistent ||
    (maybe<Mode::Persistent>(M) && isPersistentHandle(handle()));
}

template<class T, Mode M>
template<size_t Align>
void Link<T,M>::bind(Mode mode, Symbol sym, const T* init_val) {
  assertx(maybe<M>(mode));
  if (LIKELY(bound())) return;

  detail::bindOnLinkImpl(
    m_handle, sym, mode, sizeof(T), Align,
    type_scan::getIndexForScan<T>(), init_val
  );
  checkSanity();
}

template<class T, Mode M>
void Link<T,M>::checkSanity() {
  if (debug) {
    if (!bound()) return;
    DEBUG_ONLY auto h = handle();
    assertx(IMPLIES(isNormalHandle(h), maybe<Mode::Normal>(M)));
    assertx(IMPLIES(isLocalHandle(h), maybe<Mode::Local>(M)));
    assertx(IMPLIES(isPersistentHandle(h), maybe<Mode::Persistent>(M)));
  }
}

//////////////////////////////////////////////////////////////////////

template<class T, Mode M, size_t Align>
Link<T,M> bind(Symbol key, size_t extraSize) {
  static_assert(pure(M), "");
  assertx(IMPLIES(extraSize > 0, M != Mode::Normal));
  return Link<T,M>(
    detail::bindImpl(
      key, M, sizeof(T) + extraSize,
      Align, type_scan::getIndexForScan<T>()
    )
  );
}

template<class T, Mode M>
Link<T,M> attach(Symbol key) {
  return Link<T,M>(detail::attachImpl(key));
}

template<class T, Mode M, size_t Align>
Link<T,M> alloc() {
  static_assert(pure(M), "");
  return Link<T,M>(
    detail::allocUnlocked(
      M, sizeof(T), Align,
      type_scan::getIndexForScan<T>(),
      nullptr
    )
  );
}

//////////////////////////////////////////////////////////////////////

inline bool isNormalHandle(Handle handle) {
  assertx(isValidHandle(handle));
  return handle < safe_cast<uint32_t>(detail::s_normal_frontier);
}

inline bool isLocalHandle(Handle handle) {
  assertx(isValidHandle(handle));
  return handle >= safe_cast<uint32_t>(detail::s_local_frontier) &&
    handle < safe_cast<uint32_t>(detail::s_local_base);
}

inline bool isPersistentHandle(Handle handle) {
  assertx(isValidHandle(handle));
  return handle >= kMinPersistentHandle;
}

////////////////////////////////////////////////////////////////////

template <bool P>
inline GenNumber genNumberOf(Handle handle) {
  assertx(isNormalHandle(handle));
  return handleToRef<GenNumber, Mode::Normal, P>(genNumberHandleFrom(handle));
}

inline Handle genNumberHandleFrom(Handle handle) {
  assertx(isNormalHandle(handle));
  // The generation number is stored immediately in front of the element.
  return handle - sizeof(GenNumber);
}

inline bool isHandleBound(Handle handle) {
  static_assert(kUninitHandle == 0 && kBeingBound == 1 &&
                kBeingBoundWithWaiters == 2, "");
  return handle > kBeingBoundWithWaiters;
}

inline bool isHandleInit(Handle handle) {
  return !isNormalHandle(handle) ||
         isHandleInit(handle, NormalTag{});
}

inline bool isHandleInitNoProfile(Handle handle) {
  return !isNormalHandle(handle) ||
         isHandleInitNoProfile(handle, NormalTag{});
}

inline bool isHandleInit(Handle handle, NormalTag) {
  assertx(isNormalHandle(handle));
  return genNumberOf(handle) == currentGenNumber();
}

inline bool isHandleInitNoProfile(Handle handle, NormalTag) {
  assertx(isNormalHandle(handle));
  return genNumberOf<false>(handle) == currentGenNumber();
}

inline void initHandle(Handle handle) {
  assertx(isNormalHandle(handle));
  auto& gen = handleToRef<GenNumber, Mode::Normal>(genNumberHandleFrom(handle));
  gen = currentGenNumber();
}

inline void uninitHandle(Handle handle) {
  assertx(isNormalHandle(handle));
  auto& gen = handleToRef<GenNumber, Mode::Normal>(genNumberHandleFrom(handle));
  gen = kInvalidGenNumber;
}

////////////////////////////////////////////////////////////////////////////////

inline bool shouldProfileAccesses() {
  return isJitSerializing() && RO::EvalReorderRDS;
}

////////////////////////////////////////////////////////////////////////////////

template <typename F> inline void forEachNormalAlloc(F f) {
  auto size = detail::s_normal_alloc_descs_size.load(std::memory_order_acquire);
  for (const auto& desc : detail::s_normal_alloc_descs) {
    if (!(size--)) break;
    if (!isHandleInitNoProfile(desc.handle, NormalTag{})) continue;
    f(static_cast<char*>(tl_base) + desc.handle, desc.size, desc.index);
  }
}

template <typename F> inline void forEachLocalAlloc(F f) {
  auto size = detail::s_local_alloc_descs_size.load(std::memory_order_acquire);
  for (const auto& desc : detail::s_local_alloc_descs) {
    if (!(size--)) break;
    f(static_cast<char*>(tl_base) + desc.handle, desc.size, desc.index);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
