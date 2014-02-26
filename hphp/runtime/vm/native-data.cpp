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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/memory-manager.h" // SweepNode

namespace HPHP { namespace Native {
//////////////////////////////////////////////////////////////////////////////

typedef std::unordered_map<const StringData*,NativeDataInfo> NativeDataInfoMap;
static NativeDataInfoMap s_nativedatainfo;

void registerNativeDataInfo(const StringData* name,
                            size_t sz,
                            NativeDataInfo::InitFunc init,
                            NativeDataInfo::CopyFunc copy,
                            NativeDataInfo::DestroyFunc destroy) {
  assert(s_nativedatainfo.find(name) == s_nativedatainfo.end());
  NativeDataInfo info;
  info.sz = sz;
  info.init = init;
  info.copy = copy;
  info.destroy = destroy;
  s_nativedatainfo[name] = info;
}

NativeDataInfo* getNativeDataInfo(const StringData* name) {
  auto it = s_nativedatainfo.find(name);
  if (it == s_nativedatainfo.end()) {
    return nullptr;
  }
  return &it->second;
}

static __thread SweepNode* s_sweep = nullptr;

DEBUG_ONLY
static bool nodeInSweepList(SweepNode *check) {
  for (auto node = s_sweep; node; node = node->next) {
    if (node == check) return true;
  }
  return false;
}

static void prependSweepNode(SweepNode *node) {
  assert(!nodeInSweepList(node));
  if (s_sweep) {
    s_sweep->prev = node;
  }
  node->next = s_sweep;
  node->prev = nullptr;
  s_sweep = node;
}

static void removeSweepNode(SweepNode *node) {
  if (node->prev) {
    node->prev->next = node->next;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }
  if (s_sweep == node) {
    s_sweep = node->next;
  }
}

inline SweepNode* getSweepNode(ObjectData *obj) {
  return reinterpret_cast<SweepNode*>(obj) - 1;
}

DEBUG_ONLY
static bool invalidateNativeData(ObjectData* obj, const NativeDataInfo* ndi) {
  const size_t size = ndi->sz + sizeof(SweepNode);
  void *ptr = reinterpret_cast<char*>(obj) - size;
  memset(ptr, kSmartFreeFill, size);
  return true;
}

void sweepNativeData() {
  for (auto node = s_sweep; node;) {
    auto obj = reinterpret_cast<ObjectData*>(node + 1);
    auto ndi = obj->getVMClass()->getNativeDataInfo();
    ndi->destroy(obj);
    node = node->next;
    assert(invalidateNativeData(obj, ndi));
  }
  s_sweep = nullptr;
}


/* Classes with NativeData structs allocate extra memory prior
 * to the ObjectData.
 *
 * [padding][NativeData][SweepNode][ObjectData](prop0)...(propN)
 *                                /\
 *                             ObjectData* points here
 *
 * padding is added by alignTypedValue() to ensure that ObjectData*
 *   falls on a memory alignment boundary
 * NativeData is info.sz bytes for the custom class Native Data
 * SweepNode is a link in the NativeData sweep list for this ND block
 */
ObjectData* nativeDataInstanceCtor(Class* cls) {
  Attr attrs = cls->attrs();
  if (UNLIKELY(attrs & (AttrAbstract | AttrInterface | AttrTrait))) {
    ObjectData::raiseAbstractClassError(cls);
  }
  auto ndi = cls->getNativeDataInfo();
  size_t nativeDataSize = alignTypedValue(ndi->sz + sizeof(SweepNode));
  size_t nProps = cls->numDeclProperties();
  size_t size = ObjectData::sizeForNProps(nProps) + nativeDataSize;

  void *ptr = MM().objMallocLogged(size);
  auto obj = new (static_cast<char*>(ptr) + nativeDataSize) ObjectData(cls);
  obj->setAttribute(ObjectData::Attribute::HasNativeData);
  ndi->init(obj);
  prependSweepNode(getSweepNode(obj));
  return obj;
}

void nativeDataInstanceCopy(ObjectData* dest, ObjectData *src) {
  auto ndi = dest->getVMClass()->getNativeDataInfo();
  if (!ndi) return;
  assert(ndi == src->getVMClass()->getNativeDataInfo());
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
  assert(ndi);
  ndi->destroy(obj);
  removeSweepNode(getSweepNode(obj));

  size_t nativeDataSize = alignTypedValue(ndi->sz + sizeof(SweepNode));
  size_t size = ObjectData::sizeForNProps(nProps) + nativeDataSize;
  void *ptr = obj;
  ptr = static_cast<char*>(ptr) - nativeDataSize;

  if (LIKELY(size <= kMaxSmartSize)) {
    return MM().smartFreeSizeLogged(ptr, size);
  }
  MM().smartFreeSizeBigLogged(ptr, size);
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Native
