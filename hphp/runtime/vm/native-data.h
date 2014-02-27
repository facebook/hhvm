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

#include "hphp/runtime/base/types.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////
// Class NativeData

struct NativeDataInfo {
  typedef void (*InitFunc)(ObjectData *obj);
  typedef void (*CopyFunc)(ObjectData *dest, ObjectData *src);
  typedef void (*DestroyFunc)(ObjectData *obj);

  size_t sz;
  InitFunc init;
  CopyFunc copy;
  DestroyFunc destroy;
};

NativeDataInfo* getNativeDataInfo(const StringData* name);
size_t getNativeDataSize(const Class* cls);

template<class T>
T* data(ObjectData *obj) {
  auto node = reinterpret_cast<SweepNode*>(obj) - 1;
  return reinterpret_cast<T*>(node) - 1;
}

void registerNativeDataInfo(const StringData* name,
                            size_t sz,
                            NativeDataInfo::InitFunc init,
                            NativeDataInfo::CopyFunc copy,
                            NativeDataInfo::DestroyFunc destroy);

template<class T>
void nativeDataInfoInit(ObjectData* obj) {
  new (data<T>(obj)) T;
}

template<class T>
void nativeDataInfoCopy(ObjectData* dest, ObjectData* src) {
  *data<T>(dest) = *data<T>(src);
}

template<class T>
void nativeDataInfoDestroy(ObjectData* obj) {
  data<T>(obj)->~T();
}

template<class T>
void registerNativeDataInfo(const StringData* name) {
  registerNativeDataInfo(name, sizeof(T),
                         &nativeDataInfoInit<T>,
                         &nativeDataInfoCopy<T>,
                         &nativeDataInfoDestroy<T>);
}

ObjectData* nativeDataInstanceCtor(Class* cls);
void nativeDataInstanceCopy(ObjectData* dest, ObjectData *src);
void nativeDataInstanceDtor(ObjectData* obj, const Class* cls);

void sweepNativeData();

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native

#endif // _incl_HPHP_RUNTIME_VM_NATIVE_DATA_H
