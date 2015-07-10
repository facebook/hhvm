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

#include "hphp/runtime/vm/native-data.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

typedef std::unordered_map<const StringData*,NativeDataInfo> NativeDataInfoMap;
static NativeDataInfoMap s_nativedatainfo;

size_t ndsize(const ObjectData* obj, const NativeDataInfo* ndi) {
  auto cls = obj->getVMClass();
  if (cls == Generator::getClass() || cls == AsyncGenerator::getClass()) {
    return (cls == Generator::getClass())
      ? Native::data<Generator>(obj)->resumable()->size()
      - sizeof(ObjectData)
      : Native::data<AsyncGenerator>(obj)->resumable()->size()
      - sizeof(ObjectData);
  }
  return ndsize(ndi->sz);
}

void registerNativeDataInfo(const StringData* name,
                            size_t sz,
                            NativeDataInfo::InitFunc init,
                            NativeDataInfo::CopyFunc copy,
                            NativeDataInfo::DestroyFunc destroy,
                            NativeDataInfo::SweepFunc sweep,
                            NativeDataInfo::SleepFunc sleep,
                            NativeDataInfo::WakeupFunc wakeup) {
  assert(s_nativedatainfo.find(name) == s_nativedatainfo.end());
  assert((sleep == nullptr && wakeup == nullptr) ||
         (sleep != nullptr && wakeup != nullptr));
  NativeDataInfo info;
  info.sz = sz;
  info.odattrs = ObjectData::Attribute::HasNativeData;
  info.init = init;
  info.copy = copy;
  info.destroy = destroy;
  info.sweep = sweep;
  info.sleep = sleep;
  info.wakeup = wakeup;
  s_nativedatainfo[name] = info;
}

NativeDataInfo* getNativeDataInfo(const StringData* name) {
  auto it = s_nativedatainfo.find(name);
  if (it == s_nativedatainfo.end()) {
    return nullptr;
  }
  return &it->second;
}

/* Classes with NativeData structs allocate extra memory prior
 * to the ObjectData.
 *
 * [NativeNode][padding][NativeData][ObjectData](prop0)...(propN)
 *                                 /\
 *                             ObjectData* points here
 *
 * padding is added by alignTypedValue(sizeof(NativeData)) to ensure
 * that ObjectData* falls on a 16-aligned boundary. NativeData is
 * sizeof(NativeData) (NativeDataInfo.sz) bytes for the custom struct.
 * NativeNode is a link in the NativeData sweep list for this ND block
 */
ObjectData* nativeDataInstanceCtor(Class* cls) {
  HPHP::Attr attrs = cls->attrs();
  if (UNLIKELY(attrs &
               (AttrAbstract | AttrInterface | AttrTrait | AttrEnum))) {
    ObjectData::raiseAbstractClassError(cls);
  }
  if (cls->needInitialization()) {
    cls->initialize();
  }
  auto ndi = cls->getNativeDataInfo();
  size_t nativeDataSize = ndsize(ndi->sz);
  size_t nProps = cls->numDeclProperties();
  size_t size = ObjectData::sizeForNProps(nProps) + nativeDataSize;

  auto node = reinterpret_cast<NativeNode*>(
    MM().objMalloc(size)
  );
  node->obj_offset = nativeDataSize;
  node->hdr.kind = HeaderKind::NativeData;
  auto obj = new (reinterpret_cast<char*>(node) + nativeDataSize)
             ObjectData(cls);
  assert(obj->hasExactlyOneRef());
  obj->setAttribute(static_cast<ObjectData::Attribute>(ndi->odattrs));
  if (ndi->init) {
    ndi->init(obj);
  }
  if (ndi->sweep) {
    MM().addNativeObject(node);
  }
  if (UNLIKELY(cls->callsCustomInstanceInit())) {
    obj->callCustomInstanceInit();
  }
  return obj;
}

void nativeDataInstanceCopy(ObjectData* dest, ObjectData *src) {
  auto ndi = dest->getVMClass()->getNativeDataInfo();
  if (!ndi) return;
  assert(ndi == src->getVMClass()->getNativeDataInfo());
  if (!ndi->copy) {
    throw_not_implemented("NativeDataInfoCopy");
  }
  ndi->copy(dest, src);
  // Already in the sweep list from init call, no need to add again
}

void nativeDataInstanceDtor(ObjectData* obj, const Class* cls) {
  assert(!cls->preClass()->builtinObjSize());
  assert(!cls->preClass()->builtinODOffset());
  obj->~ObjectData();

  auto const nProps = size_t{cls->numDeclProperties()};
  auto prop = reinterpret_cast<TypedValue*>(obj + 1);
  auto const stop = prop + nProps;
  for (; prop != stop; ++prop) {
    tvRefcountedDecRef(prop);
  }

  auto ndi = cls->getNativeDataInfo();
  if (ndi->destroy) {
    ndi->destroy(obj);
  }
  auto node = getNativeNode(obj, ndi);
  if (ndi->sweep) {
    MM().removeNativeObject(node);
  }

  size_t size = ObjectData::sizeForNProps(nProps) + ndsize(obj, ndi);
  if (LIKELY(size <= kMaxSmallSize)) {
    return MM().freeSmallSize(node, size);
  }
  MM().freeBigSize(node, size);
}

Variant nativeDataSleep(const ObjectData* obj) {
  auto ndi = obj->getVMClass()->getNativeDataInfo();
  assert(ndi);
  assert(ndi->sleep);
  return ndi->sleep(obj);
}

void nativeDataWakeup(ObjectData* obj, const Variant& data) {
  auto ndi = obj->getVMClass()->getNativeDataInfo();
  assert(ndi);
  assert(ndi->wakeup);
  ndi->wakeup(obj, data);
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
