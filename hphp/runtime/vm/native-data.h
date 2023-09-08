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
#ifndef _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H
#define _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H

#include <folly/functional/Invoke.h>

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-object.h"

#include <type_traits>

namespace HPHP::Native {
//////////////////////////////////////////////////////////////////////////////
// Class NativeData

struct NativeDataInfo {
  typedef void (*InitFunc)(ObjectData *obj);
  typedef void (*CopyFunc)(ObjectData *dest, ObjectData *src);
  typedef void (*DestroyFunc)(ObjectData *obj);
  typedef void (*SweepFunc)(ObjectData *sweep);
  typedef Variant (*SleepFunc)(const ObjectData *sleep);
  typedef void (*WakeupFunc)(ObjectData *wakeup, const Variant& data);

  size_t sz;
  uint8_t rt_attrs;
  bool ctor_throws;
  type_scan::Index tyindex;
  InitFunc init; // new Object
  CopyFunc copy; // clone $obj
  DestroyFunc destroy; // unset($obj)
  SweepFunc sweep; // sweep $obj
  SleepFunc sleep; // serialize($obj)
  WakeupFunc wakeup; // unserialize($obj)

  bool isSerializable() const {
    return sleep != nullptr && wakeup != nullptr;
  }
};

NativeDataInfo* getNativeDataInfo(const StringData* name);
NativeNode* getNativeNode(ObjectData*, const NativeDataInfo*);
const NativeNode* getNativeNode(const ObjectData*, const NativeDataInfo*);

template<class T>
T* data(ObjectData *obj) {
  return reinterpret_cast<T*>(obj) - 1;
}

template<class T>
const T* data(const ObjectData *obj) {
  return reinterpret_cast<const T*>(obj) - 1;
}

template <class T>
T* data(Object& obj) {
  return data<T>(obj.get());
}

template <class T>
T* data(const Object& obj) {
  return data<T>(obj.get());
}

template<class T>
constexpr ptrdiff_t dataOffset() {
  return -sizeof(T);
}

template<class T>
ObjectData* object(T *data) {
  return reinterpret_cast<ObjectData*>(data + 1);
}

void registerNativeDataInfo(const StringData* name,
                            size_t sz,
                            NativeDataInfo::InitFunc init,
                            NativeDataInfo::CopyFunc copy,
                            NativeDataInfo::DestroyFunc destroy,
                            NativeDataInfo::SweepFunc sweep,
                            NativeDataInfo::SleepFunc sleep,
                            NativeDataInfo::WakeupFunc wakeup,
                            type_scan::Index tyindex,
                            uint8_t rt_attrs = 0,
                            bool ctor_throws = false);

template<class T>
void nativeDataInfoInit(ObjectData* obj) {
  new (data<T>(obj)) T;
}

template<class T>
typename std::enable_if<std::is_assignable<T,T>::value,
void>::type nativeDataInfoCopy(ObjectData* dest, ObjectData* src) {
  *data<T>(dest) = *data<T>(src);
}

// Dummy copy method for classes where the assignment has been deleted
template <class T>
typename std::enable_if<!std::is_assignable<T, T>::value, void>::type
nativeDataInfoCopy(ObjectData* /*dest*/, ObjectData* /*src*/) {}

template<class T>
void nativeDataInfoDestroy(ObjectData* obj) {
  data<T>(obj)->~T();
};

template<class T>
typename std::enable_if<!std::is_trivially_destructible<T>::value,
                        void(*)(ObjectData*)>::type
getNativeDataInfoDestroy() {
  return &nativeDataInfoDestroy<T>;
}

template<class T>
typename std::enable_if<std::is_trivially_destructible<T>::value,
                        void(*)(ObjectData*)>::type
getNativeDataInfoDestroy() {
  return nullptr;
}

// If the NDI class has a void sweep() method,
// call it during sweep, otherwise call ~T()
// unless its trivial, or the class defines a
//
//   static const bool sweep = false;
FOLLY_CREATE_MEMBER_INVOKER(invoke_sweep, sweep);

// class to test for presence of a sweep set to false
template <typename T>
struct doSweep {
  template <typename C>
  constexpr static typename std::enable_if<C::sweep == false, bool>::type
  test(void*) { return false; }
  template <typename>
  constexpr static bool test(...) { return true; }
  constexpr static bool value = test<T>(nullptr);
};

template<class T>
void callSweep(ObjectData* obj) {
  data<T>(obj)->sweep();
};

template<class T>
auto getNativeDataInfoSweep() -> void(*)(ObjectData*) {
  if constexpr (std::is_invocable_v<invoke_sweep, T>) {
    return &callSweep<T>;
  }
  if constexpr (!std::is_trivially_destructible_v<T> && doSweep<T>::value) {
    return &nativeDataInfoDestroy<T>;
  }
  return nullptr;
}

FOLLY_CREATE_MEMBER_INVOKER(invoke_sleep, sleep);

template<class T, class DeferVariant = Variant>
DeferVariant nativeDataInfoSleep(const ObjectData* obj) {
  if constexpr (std::is_invocable_v<invoke_sleep, T>) {
    return data<T>(obj)->sleep();
  } else {
    always_assert(0);
  }
}

FOLLY_CREATE_MEMBER_INVOKER(invoke_wakeup, wakeup);

template<class T>
void nativeDataInfoWakeup(ObjectData* obj, const Variant& content) {
  constexpr bool has_wakeup =
      std::is_invocable_v<invoke_wakeup, T, const Variant&, ObjectData*>;
  if constexpr (has_wakeup) {
    data<T>(obj)->wakeup(content, obj);
  } else {
    always_assert(0);
  }
}

enum NDIFlags {
  NONE           = 0,
  // Skipping the ctor/dtor is generally a bad idea
  // since memory props won't get setup/torn-down
  NO_COPY        = (1<<0),
  NO_SWEEP       = (1<<1),
  // Forbid all forms of construction from hack code by throwing in the
  // instanceCtor (i.e. only native funcs using other allocation code paths
  // can create instances).
  CTOR_THROWS    = (1<<2),
};

template<class T>
type_scan::Index nativeTyindex() {
  // get the gc scanner type-id for T. getIndexForMalloc<T> provides
  // a scanner for T and causes pointer fields of type T* to be scanned.
  return type_scan::getIndexForMalloc<T>();
}

// NativeData's should not extend sweepable, to sweep a native data define
// a sweep() function and register the NativeData without the NO_SWEEP flag.
template<class T>
typename std::enable_if<
  !std::is_base_of<Sweepable, T>::value,
  void
>::type registerNativeDataInfo(const StringData* name,
                               int64_t flags = 0, uint8_t rt_attrs = 0) {
  auto ndic = &nativeDataInfoCopy<T>;
  auto ndisw = getNativeDataInfoSweep<T>();
  auto ndisl = &nativeDataInfoSleep<T>;
  auto ndiw = &nativeDataInfoWakeup<T>;

  registerNativeDataInfo(
    name, sizeof(T),
    &nativeDataInfoInit<T>,
    (flags & NDIFlags::NO_COPY) ? nullptr : ndic,
    getNativeDataInfoDestroy<T>(),
    (flags & NDIFlags::NO_SWEEP) ? nullptr : ndisw,
    std::is_invocable_v<invoke_sleep, T const&> ? ndisl : nullptr,
    std::is_invocable_v<invoke_wakeup, T&, const Variant&, ObjectData*> ? ndiw : nullptr,
    nativeTyindex<T>(),
    rt_attrs,
    (flags & NDIFlags::CTOR_THROWS));
}

template<class T>
typename std::enable_if<
  !std::is_base_of<Sweepable, T>::value,
  void
>::type registerNativeDataInfo(int64_t flags = 0, uint8_t rt_attrs = 0) {
  registerNativeDataInfo<T>(T::className().get(), flags, rt_attrs);
}

// Return the ObjectData payload allocated after this NativeNode header
inline ObjectData* obj(NativeNode* node) {
  auto obj = reinterpret_cast<ObjectData*>(
    reinterpret_cast<char*>(node) + node->obj_offset
  );
  assertx(obj->hasNativeData());
  return obj;
}

// Return the ObjectData payload allocated after this NativeNode header
inline const ObjectData* obj(const NativeNode* node) {
  auto obj = reinterpret_cast<const ObjectData*>(
    reinterpret_cast<const char*>(node) + node->obj_offset
  );
  assertx(obj->hasNativeData());
  return obj;
}

template <bool Unlocked>
ObjectData* nativeDataInstanceCtor(Class* cls);
ObjectData* nativeDataInstanceCopyCtor(ObjectData *src, Class* cls,
                                       size_t nProps);
void nativeDataInstanceDtor(ObjectData* obj, const Class* cls);

Variant nativeDataSleep(const ObjectData* obj);
void nativeDataWakeup(ObjectData* obj, const Variant& data);

size_t ndsize(const ObjectData* obj, const NativeDataInfo* ndi);

inline NativeNode* getNativeNode(ObjectData* obj, const NativeDataInfo* ndi) {
  return reinterpret_cast<NativeNode*>(
    reinterpret_cast<char*>(obj) - ndsize(obj, ndi)
  );
}

inline const NativeNode*
getNativeNode(const ObjectData* obj, const NativeDataInfo* ndi) {
  return reinterpret_cast<const NativeNode*>(
    reinterpret_cast<const char*>(obj) - ndsize(obj, ndi)
  );
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H
