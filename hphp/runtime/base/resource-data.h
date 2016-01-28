/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RESOURCE_DATA_H_
#define incl_HPHP_RESOURCE_DATA_H_

#include <iostream>
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/classname-is.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/imarker.h"
#include "hphp/util/thread-local.h"

namespace HPHP {

class Array;
class String;
struct ResourceData;

namespace req {
template<class T, class... Args>
typename std::enable_if<std::is_convertible<T*,ResourceData*>::value,
                        req::ptr<T>>::type
make(Args&&... args);
}

/*
 * De-virtualized header for Resource objects. The memory layout is:
 *
 * [ResourceHdr] { m_id, m_hdr; }
 * [ResourceData] { vtbl, subclass fields; }
 *
 * Historically, we only had ResourceData. To ease refactoring, we have
 * pointer conversion utilities:
 *   ResourceHdr* ResourceData::hdr()
 *   ResourceData* ResourceHdr::data()
 *
 * ResourceData explicitly declares inc/decref functions that
 * delegate to ResourceHdr, which allows req::ptr<T> in user code to
 * continue doing transparent refcounting.
 *
 * Type-agnostic header access requires TypedValue (and Variant) to have a
 * ResourceHdr* ptr in the m_data union. We also still need to cast &m_data
 * to a Resource**, so Resource owns a req::ptr<Resourcebase>.
 *
 * Runtime and extension code typically will use req::ptr<T> where T extends
 * ResourceData; these are interior pointers, but allow code to continue
 * using ResourceData as the base of the virtual class hierarchy.
 *
 * In the JIT, SSATmps of type Res are ResourceHdr pointers.
 */
struct ResourceHdr {
  static void resetMaxId();

  IMPLEMENT_COUNTABLE_METHODS
  bool kindIsValid() const { return m_hdr.kind == HeaderKind::Resource; }
  void release() noexcept;

  void init(size_t size) {
    m_hdr.init(size, HeaderKind::Resource, 1);
  }

  ResourceData* data() {
    assert(kindIsValid());
    return reinterpret_cast<ResourceData*>(this + 1);
  }
  const ResourceData* data() const {
    assert(kindIsValid());
    return reinterpret_cast<const ResourceData*>(this + 1);
  }

  size_t heapSize() const {
    assert(kindIsValid());
    assert(m_hdr.aux != 0);
    return m_hdr.aux;
  }

  int32_t getId() const { return m_id; }
  void setRawId(int32_t id) { m_id = id; }
  void setId(int32_t id); // only for BuiltinFiles

private:
  static void compileTimeAssertions();
private:
  int32_t m_id;
  HeaderWord<uint16_t> m_hdr; // m_hdr.aux stores heap size
};

/**
 * Base class of all PHP resources.
 */
struct ResourceData {
  ResourceData();

  ResourceData(const ResourceData&) = delete;
  ResourceData& operator=(const ResourceData&) = delete;

  const ResourceHdr* hdr() const {
    auto h = reinterpret_cast<const ResourceHdr*>(this) - 1;
    assert(h->kindIsValid());
    return h;
  }
  ResourceHdr* hdr() {
    auto h = reinterpret_cast<ResourceHdr*>(this) - 1;
    assert(h->kindIsValid());
    return h;
  }

  // delegate refcount operations to base.
  void incRefCount() const { hdr()->incRefCount(); }
  void decRefAndRelease() { hdr()->decRefAndRelease(); }
  bool hasExactlyOneRef() const { return hdr()->hasExactlyOneRef(); }
  bool hasMultipleRefs() const { return hdr()->hasMultipleRefs(); }
  int32_t getId() const { return hdr()->getId(); }
  void setId(int32_t id) { hdr()->setId(id); }

  virtual ~ResourceData(); // all PHP resources need vtables

  void operator delete(void* p) {
    always_assert(false);
  }

  template<class F> void scan(F&) const;
  virtual void vscan(IMarker& mark) const;

  const String& o_getClassName() const;
  virtual const String& o_getClassNameHook() const;
  virtual const String& o_getResourceName() const;
  virtual bool isInvalid() const { return false; }

  template <typename T>
  bool instanceof() const { return dynamic_cast<const T*>(this) != nullptr; }

  bool o_toBoolean() const { return true; }
  int64_t o_toInt64() const { return hdr()->getId(); }
  double o_toDouble() const { return hdr()->getId(); }
  String o_toString() const;
  Array o_toArray() const;

 private:
  template<class T, class... Args> friend
  typename std::enable_if<std::is_convertible<T*,ResourceData*>::value,
                          req::ptr<T>>::type req::make(Args&&... args);
};

inline void ResourceHdr::release() noexcept {
  assert(kindIsValid());
  delete data();
}

inline ResourceData* safedata(ResourceHdr* hdr) {
  return hdr ? hdr->data() : nullptr;
}
inline const ResourceData* safedata(const ResourceHdr* hdr) {
  return hdr ? hdr->data() : nullptr;
}
inline ResourceHdr* safehdr(ResourceData* data) {
  return data ? data->hdr() : nullptr;
}
inline const ResourceHdr* safehdr(const ResourceData* data) {
  return data ? data->hdr() : nullptr;
}

/**
 * Rules to avoid memory problems/leaks from ResourceData classes
 * ==============================================================
 *
 * 1. If a ResourceData is entirely request-allocated, for example,
 *
 *    class EntirelyRequestAllocated : public ResourceData {
 *    public:
 *       int number; // primitives are allocated together with "this"
 *       String str; // request-allocated objects are fine
 *    };
 *
 *    Then, the best choice is to use this macro to make sure the object
 *    is always collected during request shutdown time:
 *
 *       DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(T);
 *
 *    This object doesn't participate in sweep(), as object allocator doesn't
 *    have any callback installed.
 *
 * 2. If a ResourceData is entirely not request allocated, for example,
 *
 *    class NonRequestAllocated : public SweepableResourceData {
 *    public:
 *       int number; // primitives are always not in consideration
 *       std::string str; // this has malloc() in its own
 *       std::vector<int> vec; // all STL collection classes belong here
 *       HANDLE ptr; // raw pointers that need to be free-d somehow
 *    };
 *
 *    Then it has to derive from SweepableResourceData, so sweep() will be
 *    called. By default, it will call this object's destructor automatically,
 *    so everything will be free-d.
 *
 *    When deriving from SweepableResourceData, either "new" or "NEW" can
 *    be used, but we prefer people use NEW with these macros:
 *
 *       DECLARE_RESOURCE_ALLOCATION(T);
 *       IMPLEMENT_RESOURCE_ALLOCATION(T);
 *
 * 3. If a ResourceData is a mix of request allocated data members and globally
 *    allocated data members, sweep() has to be overwritten to only free
 *    the globally allocated members. This is because request-allocated
 *    data members may have their own sweep() defined to destruct, and another
 *    destruction from this ResourceData's default sweep() will cause double-
 *    free problems on these request-allocated data members.
 *
 *    This means, std::vector<String> is almost always wrong, because there is
 *    no way to free up vector's memory without touching String, which is
 *    request-allocated.
 *
 *    class MixedRequestAllocated : public SweepableResourceData {
 *    public:
 *       int number; // primitives are always not in consideration
 *
 *       // STL classes need to new/delete to have clean sweep
 *       std::string *stdstr;
 *       std::vector<int> *vec;
 *
 *       HANDLE ptr; // raw pointers that need to be free-d somehow
 *       String str; // request-allocated objects are fine
 *
 *       DECLARE_RESOURCE_ALLOCATION(T);
 *    };
 *    void MixedRequestAllocated::sweep() {
 *       delete stdstr;
 *       delete vec;
 *       close_handle(ptr);
 *       // without doing anything with Strings, Arrays, or Objects
 *    }
 *
 */
struct SweepableResourceData : ResourceData, Sweepable {
protected:
  void* owner() override {
    return static_cast<ResourceData*>(this)->hdr();
  }
};

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void decRefRes(ResourceData* res) {
  res->hdr()->decRefAndRelease();
}
ALWAYS_INLINE void decRefRes(ResourceHdr* res) {
  res->decRefAndRelease();
}

#define DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(T)                 \
  public:                                                       \
  ALWAYS_INLINE void operator delete(void* p) {                 \
    static_assert(std::is_base_of<ResourceData,T>::value, "");  \
    constexpr auto size = sizeof(ResourceHdr) + sizeof(T);      \
    auto h = static_cast<ResourceData*>(p)->hdr();              \
    assert(h->heapSize() == size);                              \
    MM().freeSmallSize(h, size);                                \
  }

#define DECLARE_RESOURCE_ALLOCATION(T)                          \
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(T)                       \
  void sweep() override;

#define IMPLEMENT_RESOURCE_ALLOCATION(T) \
  static_assert(std::is_base_of<ResourceData,T>::value, ""); \
  void HPHP::T::sweep() { this->~T(); }

namespace req {
// allocate and construct a resource subclass type T,
// wrapped in a req::ptr<T>
template<class T, class... Args>
typename std::enable_if<
  std::is_convertible<T*, ResourceData*>::value,
  req::ptr<T>
>::type make(Args&&... args) {
  constexpr auto size = sizeof(ResourceHdr) + sizeof(T);
  static_assert(size <= 0xffff && size < kMaxSmallSize, "");
  static_assert(std::is_convertible<T*,ResourceData*>::value, "");
  auto const b = static_cast<ResourceHdr*>(MM().mallocSmallSize(size));
  b->init(size); // initialize HeaderWord
  try {
    auto r = new (b->data()) T(std::forward<Args>(args)...);
    assert(r->hasExactlyOneRef());
    return req::ptr<T>::attach(r);
  } catch (...) {
    MM().freeSmallSize(b, size);
    throw;
  }
}
} // namespace req

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_
