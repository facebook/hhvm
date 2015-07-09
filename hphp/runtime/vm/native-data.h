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
#ifndef _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H
#define _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/type-object.h"

namespace HPHP { namespace Native {
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
  uint16_t odattrs;
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
                            NativeDataInfo::WakeupFunc wakeup);

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
template<class T>
typename std::enable_if<!std::is_assignable<T,T>::value,
void>::type nativeDataInfoCopy(ObjectData* dest, ObjectData* src) {}

template<class T>
void nativeDataInfoDestroy(ObjectData* obj) {
  data<T>(obj)->~T();
}

// If the NDI class has a void sweep() method,
// call it during sweep, otherwise call ~T()
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(hasSweep, sweep);

template<class T>
typename std::enable_if<hasSweep<T,void ()>::value,
void>::type nativeDataInfoSweep(ObjectData* obj) {
  data<T>(obj)->sweep();
}

template<class T>
typename std::enable_if<!hasSweep<T,void ()>::value,
void>::type nativeDataInfoSweep(ObjectData* obj) {
  data<T>(obj)->~T();
}

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(hasSleep, sleep);

template<class T>
typename std::enable_if<hasSleep<T, Variant() const>::value,
Variant>::type nativeDataInfoSleep(const ObjectData* obj) {
  return data<T>(obj)->sleep();
}

template<class T>
typename std::enable_if<!hasSleep<T, Variant() const>::value,
Variant>::type nativeDataInfoSleep(const ObjectData* obj) {
  always_assert(0);
}

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(hasWakeup, wakeup);

template<class T>
typename std::enable_if<hasWakeup<T, void(const Variant&, ObjectData*)>::value,
void>::type nativeDataInfoWakeup(ObjectData* obj, const Variant& content) {
  data<T>(obj)->wakeup(content, obj);
}

template<class T>
typename std::enable_if<!hasWakeup<T, void(const Variant&, ObjectData*)>::value,
void>::type nativeDataInfoWakeup(ObjectData* obj, const Variant& content) {
  always_assert(0);
}

enum NDIFlags {
  NONE           = 0,
  // Skipping the ctor/dtor is generally a bad idea
  // since memory props won't get setup/torn-down
  NO_COPY        = (1<<0),
  NO_SWEEP       = (1<<1),
};

// NativeData's should not extend sweepable, to sweep a native data define
// a sweep() function and register the NativeData without the NO_SWEEP flag.
template<class T>
typename std::enable_if<
  !std::is_base_of<Sweepable, T>::value,
  void
>::type registerNativeDataInfo(const StringData* name,
                               int64_t flags = 0) {
  registerNativeDataInfo(name, sizeof(T),
                         &nativeDataInfoInit<T>,
                         (flags & NDIFlags::NO_COPY)
                           ? nullptr : &nativeDataInfoCopy<T>,
                         &nativeDataInfoDestroy<T>,
                         (flags & NDIFlags::NO_SWEEP)
                           ? nullptr : &nativeDataInfoSweep<T>,
                         hasSleep<T, Variant() const>::value
                           ? &nativeDataInfoSleep<T> : nullptr,
                         hasWakeup<T, void(const Variant&, ObjectData*)>::value
                           ? &nativeDataInfoWakeup<T> : nullptr);
}

// Return the ObjectData payload allocated after this NativeNode header
inline ObjectData* obj(NativeNode* node) {
  return reinterpret_cast<ObjectData*>(
    reinterpret_cast<char*>(node) + node->obj_offset
  );
}

// Return the ObjectData payload allocated after this NativeNode header
inline const ObjectData* obj(const NativeNode* node) {
  return reinterpret_cast<const ObjectData*>(
    reinterpret_cast<const char*>(node) + node->obj_offset
  );
}

ObjectData* nativeDataInstanceCtor(Class* cls);
void nativeDataInstanceCopy(ObjectData* dest, ObjectData *src);
void nativeDataInstanceDtor(ObjectData* obj, const Class* cls);

Variant nativeDataSleep(const ObjectData* obj);
void nativeDataWakeup(ObjectData* obj, const Variant& data);

size_t ndsize(const ObjectData* obj, const NativeDataInfo* ndi);

// return the full native header size, which is also the distance from
// the allocated pointer to the ObjectData*.
inline size_t ndsize(size_t dataSize) {
  return alignTypedValue(dataSize + sizeof(NativeNode));
 }

inline size_t ndextra(const ObjectData* obj, const NativeDataInfo* ndi) {
  return ndsize(obj, ndi) - ndsize(ndi->sz);
}

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
}} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H
