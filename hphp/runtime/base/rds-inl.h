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
#ifndef incl_HPHP_RUNTIME_BASE_RDS_INL_H_
#define incl_HPHP_RUNTIME_BASE_RDS_INL_H_

#include <tbb/concurrent_vector.h>

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

namespace detail {

Handle alloc(Mode mode, size_t numBytes, size_t align,
             type_scan::Index tyIndex);
Handle allocUnlocked(Mode mode, size_t numBytes, size_t align,
                     type_scan::Index tyIndex);
Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes,
                size_t align, type_scan::Index tyIndex);
Handle attachImpl(Symbol key);
void bindOnLinkImpl(std::atomic<Handle>& handle, Mode mode,
                    size_t sizeBytes, size_t align,
                    type_scan::Index tyIndex);

extern size_t s_normal_frontier;
extern size_t s_persistent_base;
extern size_t s_persistent_frontier;
extern size_t s_local_frontier;

struct AllocDescriptor {
  Handle handle;
  uint32_t size;
  type_scan::Index index;
};
using AllocDescriptorList = tbb::concurrent_vector<AllocDescriptor>;
extern AllocDescriptorList s_normal_alloc_descs;
extern AllocDescriptorList s_local_alloc_descs;

}

//////////////////////////////////////////////////////////////////////

template<class T, bool N>
Link<T,N>::Link(Handle handle) : m_handle(handle) {}

template<class T, bool N>
Link<T,N>::Link(const Link& l) : m_handle{l.handle()} {}

template<class T, bool N>
Link<T,N>& Link<T,N>::operator=(const Link& l) {
  assertx(IMPLIES(N, l.isNormal()));
  m_handle.store(l.handle(), std::memory_order_relaxed);
  return *this;
}

template<class T, bool N>
T& Link<T,N>::operator*() const { return *get(); }

template<class T, bool N>
T* Link<T,N>::operator->() const { return get(); }

template<class T, bool N>
T* Link<T,N>::get() const {
  assert(bound());
  void* vp = static_cast<char*>(tl_base) + handle();
  return static_cast<T*>(vp);
}

template<class T, bool N>
bool Link<T,N>::bound() const {
  return handle() != kInvalidHandle;
}

template<class T, bool N>
Handle Link<T,N>::handle() const {
  return m_handle.load(std::memory_order_relaxed);
}

template<class T, bool N>
Handle Link<T,N>::genNumberHandle() const {
  assertx(bound());
  return genNumberHandleFrom(handle());
}

template<class T, bool N>
GenNumber Link<T,N>::genNumber() const {
  assertx(bound());
  return genNumberOf(handle());
}

template<class T, bool N>
bool Link<T,N>::isInit() const {
  assertx(bound());
  return isHandleInit(handle());
}

template<class T, bool N>
bool Link<T,N>::isInit(NormalTag) const {
  assertx(bound());
  return N
    ? isHandleInit(handle(), NormalTag{})
    : isHandleInit(handle());
}

template<class T, bool N>
void Link<T,N>::markInit() const {
  assertx(bound());
  initHandle(handle());
}

template<class T, bool N>
void Link<T,N>::markUninit() const {
  assertx(bound());
  uninitHandle(handle());
}

template <class T, bool N>
void Link<T,N>::initWith(const T& val) const {
  new (get()) T(val);
  if (isNormal()) markInit();
}

template <class T, bool N>
void Link<T,N>::initWith(T&& val) const {
  new (get()) T(std::move(val));
  if (isNormal()) markInit();
}

template <class T, bool N>
bool Link<T,N>::isNormal() const {
  assertx(bound());
  return N || isNormalHandle(handle());
}

template <class T, bool N>
bool Link<T,N>::isLocal() const {
  assertx(bound());
  return !N && isLocalHandle(handle());
}

template<class T, bool N>
bool Link<T,N>::isPersistent() const {
  assertx(bound());
  return !N && isPersistentHandle(handle());
}

template<class T, bool N>
template<size_t Align>
void Link<T,N>::bind(Mode mode) {
  assertx(IMPLIES(N, mode == Mode::Normal));
  if (LIKELY(bound())) return;
  detail::bindOnLinkImpl(
    m_handle, mode, sizeof(T),
    Align, type_scan::getIndexForScan<T>()
  );
  recordRds(m_handle, sizeof(T), "Unknown", __PRETTY_FUNCTION__);
}

//////////////////////////////////////////////////////////////////////

template<class T, bool N, size_t Align>
Link<T,N> bind(Symbol key, Mode mode, size_t extraSize) {
  assertx(IMPLIES(N, mode == Mode::Normal));
  assertx(IMPLIES(extraSize > 0, mode != Mode::Normal));
  return Link<T,N>(
    detail::bindImpl(
      key, mode, sizeof(T) + extraSize,
      Align, type_scan::getIndexForScan<T>()
    )
  );
}

template<class T>
Link<T> attach(Symbol key) {
  return Link<T>(detail::attachImpl(key));
}

template<class T, size_t Align, bool N>
Link<T,N> alloc(Mode mode) {
  assertx(IMPLIES(N, mode == Mode::Normal));
  return Link<T,N>(
    detail::allocUnlocked(
      mode, sizeof(T), Align,
      type_scan::getIndexForScan<T>()
    )
  );
}

//////////////////////////////////////////////////////////////////////

template<class T>
T& handleToRef(Handle h) {
  return handleToRef<T>(tl_base, h);
}

template<class T>
T& handleToRef(void* base, Handle h) {
  void* vp = static_cast<char*>(base) + h;
  return *static_cast<T*>(vp);
}

//////////////////////////////////////////////////////////////////////

inline bool isNormalHandle(Handle handle) {
  assertx(isValidHandle(handle));
  return handle < (unsigned)detail::s_normal_frontier;
}

inline bool isLocalHandle(Handle handle) {
  assertx(isValidHandle(handle));
  return !isNormalHandle(handle) && !isPersistentHandle(handle);
}

inline bool isPersistentHandle(Handle handle) {
  static_assert(std::is_unsigned<Handle>::value,
                "Handle is supposed to be unsigned");
  assertx(isValidHandle(handle));
  return handle >= (unsigned)detail::s_persistent_base;
}

////////////////////////////////////////////////////////////////////

inline GenNumber genNumberOf(Handle handle) {
  assertx(isNormalHandle(handle));
  return handleToRef<GenNumber>(genNumberHandleFrom(handle));
}

inline Handle genNumberHandleFrom(Handle handle) {
  assertx(isNormalHandle(handle));
  // The generation number is stored immediately in front of the element.
  return handle - sizeof(GenNumber);
}

inline bool isHandleInit(Handle handle) {
  return !isNormalHandle(handle) ||
         isHandleInit(handle, NormalTag{});
}

inline bool isHandleInit(Handle handle, NormalTag) {
  assertx(isNormalHandle(handle));
  return genNumberOf(handle) == currentGenNumber();
}

inline void initHandle(Handle handle) {
  assertx(isNormalHandle(handle));
  handleToRef<GenNumber>(genNumberHandleFrom(handle)) = currentGenNumber();
}

inline void uninitHandle(Handle handle) {
  assertx(isNormalHandle(handle));
  handleToRef<GenNumber>(genNumberHandleFrom(handle)) = kInvalidGenNumber;
}

////////////////////////////////////////////////////////////////////////////////

template <typename F> inline void forEachNormalAlloc(F f) {
  for (const auto& desc : detail::s_normal_alloc_descs) {
    if (!isHandleInit(desc.handle, NormalTag{})) continue;
    f(static_cast<char*>(tl_base) + desc.handle, desc.size, desc.index);
  }
}

template <typename F> inline void forEachLocalAlloc(F f) {
  for (const auto& desc : detail::s_local_alloc_descs) {
    f(static_cast<char*>(tl_base) + desc.handle, desc.size, desc.index);
  }
}

///////////////////////////////////////////////////////////////////////////////

}}

#endif
