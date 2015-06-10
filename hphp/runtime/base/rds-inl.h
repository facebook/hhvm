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
#ifndef incl_HPHP_RUNTIME_BASE_RDS_INL_H_
#define incl_HPHP_RUNTIME_BASE_RDS_INL_H_

namespace HPHP { namespace rds {

//////////////////////////////////////////////////////////////////////

namespace detail {

Handle alloc(Mode mode, size_t numBytes, size_t align);
Handle allocUnlocked(Mode mode, size_t numBytes, size_t align);
Handle bindImpl(Symbol key, Mode mode, size_t sizeBytes, size_t align);
Handle attachImpl(Symbol key);
void bindOnLinkImpl(std::atomic<Handle>& handle, Mode mode,
  size_t sizeBytes, size_t align);

}

//////////////////////////////////////////////////////////////////////

template<class T> Link<T>::Link(Handle handle) : m_handle(handle) {}
template<class T> Link<T>::Link(const Link& l) : m_handle{l.handle()} {}

template<class T> Link<T>& Link<T>::operator=(const Link& l) {
  m_handle.store(l.handle(), std::memory_order_relaxed);
  return *this;
}

template<class T> T& Link<T>::operator*() const { return *get(); }
template<class T> T* Link<T>::operator->() const { return get(); }

template<class T> T* Link<T>::get() const {
  assert(bound());
  void* vp = static_cast<char*>(tl_base) + handle();
  return static_cast<T*>(vp);
}

template<class T>
bool Link<T>::bound() const {
  return handle() != kInvalidHandle;
}

template<class T>
Handle Link<T>::handle() const {
  return m_handle.load(std::memory_order_relaxed);
}

template<class T>
bool Link<T>::isPersistent() const {
  return isPersistentHandle(handle());
}

template<class T>
template<size_t Align>
void Link<T>::bind(Mode mode) {
  if (LIKELY(bound())) return;
  detail::bindOnLinkImpl(m_handle, mode, sizeof(T), Align);
  recordRds(m_handle, sizeof(T), "Unknown", __PRETTY_FUNCTION__);
}

//////////////////////////////////////////////////////////////////////

template<class T, size_t Align>
Link<T> bind(Symbol key, Mode mode, size_t extraSize) {
  return Link<T>(detail::bindImpl(key, mode, sizeof(T) + extraSize, Align));
}

template<class T>
Link<T> attach(Symbol key) {
  return Link<T>(detail::attachImpl(key));
}

template<class T, size_t Align>
Link<T> alloc(Mode mode) {
  return Link<T>(detail::allocUnlocked(mode, sizeof(T), Align));
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

}}

#endif
