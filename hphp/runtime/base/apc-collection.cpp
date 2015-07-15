/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/apc-collection.h"

#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/base/data-walker.h"

namespace HPHP {

namespace {

//////////////////////////////////////////////////////////////////////

void fillMap(BaseMap* map, const APCArray* ar) {
  for (auto i = uint32_t{0}; i < ar->size(); ++i) {
    map->set(ar->getKey(i).asTypedValue(),
             ar->getValue(i)->toLocal().asTypedValue());
  }
}

template<class T>
void fillCollection(T* coll, const APCArray* ar) {
  for (auto i = uint32_t{0}; i < ar->size(); ++i) {
    coll->add(ar->getValue(i)->toLocal().asTypedValue());
  }
}

// Deserializing an array could give back a different ArrayKind than we need,
// so we have to go with the slow case of calling a collection constructor.
NEVER_INLINE
Object createFromSerialized(CollectionType colType, APCHandle* handle) {
  auto const col = Object::attach(collections::alloc(colType));
  auto const arr = handle->toLocal();
  switch (colType) {
  case CollectionType::ImmVector:
  case CollectionType::Vector:
    static_cast<BaseVector*>(col.get())->t___construct(arr);
    break;
  case CollectionType::ImmSet:
  case CollectionType::Set:
    static_cast<BaseSet*>(col.get())->t___construct(arr);
    break;
  case CollectionType::ImmMap:
  case CollectionType::Map:
    static_cast<BaseMap*>(col.get())->t___construct(arr);
    break;
  case CollectionType::Pair:
    not_reached();
    break;
  }
  return col;
}

//////////////////////////////////////////////////////////////////////

}

APCHandle::Pair APCCollection::Make(const ObjectData* obj,
                                    bool inner,
                                    bool unserializeObj) {
  auto bail = [&] {
    return APCObject::MakeSerializedObj(
      apc_serialize(Variant(const_cast<ObjectData*>(obj)))
    );
  };

  auto const array = collections::asArray(obj);
  if (!array) return bail();

  /*
   * Create an uncounted array if we can.
   *
   * If this collection is !inner, then we need to do a full check on this
   * array for things like circularity.  If we're inner, someone already
   * checked that, but we want to check for whether it's uncounted to use a
   * better representation.  For the !inner case, we just delegate to APCArray
   * below (which will do the full DataWalker pass).
   */
  if (inner && apcExtension::UseUncounted && !array->empty()) {
    DataWalker walker(DataWalker::LookupFeature::HasObjectOrResource);
    auto const features = walker.traverseData(const_cast<ArrayData*>(array));
    assert(!features.isCircular);
    if (!features.hasObjectOrResource) {
      return WrapArray(
        { APCTypedValue::MakeSharedArray(const_cast<ArrayData*>(array)),
          getMemSize(array) + sizeof(APCTypedValue) },
        obj->collectionType()
      );
    }
  }

  return WrapArray(
    APCArray::MakeSharedArray(const_cast<ArrayData*>(array),
                              inner,
                              unserializeObj),
    obj->collectionType()
  );
}

void APCCollection::Delete(APCHandle* h) {
  assert(offsetof(APCCollection, m_handle) == 0);
  delete reinterpret_cast<APCCollection*>(h);
}

APCCollection::APCCollection()
  : m_handle(KindOfObject)
{
  m_handle.setAPCCollection();
}

APCCollection::~APCCollection() {
  // Zero for size is correct, because when this APCCollection was unreferenced
  // it already included the size of the inner handle.
  m_arrayHandle->unreferenceRoot(0);
}

APCHandle::Pair APCCollection::WrapArray(APCHandle::Pair inner,
                                         CollectionType colType) {
  auto const col = new APCCollection;
  col->m_arrayHandle = inner.handle;
  col->m_colType = colType;
  return { &col->m_handle, inner.size + sizeof(APCCollection) };
}

Object APCCollection::createObject() const {
  if (m_arrayHandle->isUncounted()) {
    Variant local(m_arrayHandle->toLocal());
    assert(local.is(KindOfArray));
    return Object::attach(
      collections::alloc(m_colType, local.getArrayData())
    );
  }

  if (UNLIKELY(m_arrayHandle->isSerializedArray())) {
    return createFromSerialized(m_colType, m_arrayHandle);
  }

  // We had a counted inner array---we need to do an O(N) copy to get the
  // collection into the request local heap.
  auto const apcArr = APCArray::fromHandle(m_arrayHandle);
  auto const col = Object::attach(collections::alloc(m_colType));
  switch (m_colType) {
  case CollectionType::ImmVector:
  case CollectionType::Vector:
    fillCollection(static_cast<BaseVector*>(col.get()), apcArr);
    break;
  case CollectionType::ImmSet:
  case CollectionType::Set:
    fillCollection(static_cast<BaseSet*>(col.get()), apcArr);
    break;
  case CollectionType::ImmMap:
  case CollectionType::Map:
    fillMap(static_cast<BaseMap*>(col.get()), apcArr);
    break;
  case CollectionType::Pair:
    always_assert(0);
    break;
  }
  return col;
}

//////////////////////////////////////////////////////////////////////

}
