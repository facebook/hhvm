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

#include "hphp/runtime/base/apc-collection.h"

#include <cstdlib>

#include "hphp/util/logger.h"

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/runtime/ext/ext_collections.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

APCHandle* APCCollection::MakeShared(
    ObjectData* data, size_t& size, bool inner) {
  assert(data->isCollection());

  APCHandle* array = nullptr;
  auto colType = data->getCollectionType();
  switch (colType) {
  case Collection::Type::VectorType:
  case Collection::Type::ImmVectorType:
    array = APCArray::MakeShared(
        static_cast<BaseVector*>(data)->arrayData(),
        size,
        inner,
        true,
        false);
    break;

  case Collection::Type::MapType:
  case Collection::Type::ImmMapType:
  case Collection::Type::SetType:
  case Collection::Type::ImmSetType:
    array = APCArray::MakeShared(
        static_cast<HashCollection*>(data)->arrayData()->asArrayData(),
        size,
        inner,
        true,
        false);
    break;

  default:
    break;
  }

  if (array == nullptr) {
    return APCObject::MakeShared(apc_serialize(data), size);
  }

  void* p = malloc(sizeof(APCCollection));
  auto apcColl = new (p) APCCollection(array, colType, size);
  size += sizeof(APCCollection);
  return apcColl->getHandle();
}

ALWAYS_INLINE
APCCollection::APCCollection(
    APCHandle* data, Collection::Type type, uint32_t size)
  : m_handle(KindOfObject)
  , m_data(data)
  , m_type(type)
  , m_size(size)
{
  m_handle.setIsObj();
  m_handle.setIsCollection();
  m_handle.mustCache();
}

ALWAYS_INLINE
APCCollection::~APCCollection() {
  m_data->unreferenceRoot(m_size);
}

void APCCollection::Delete(APCHandle* handle) {
  auto const obj = fromHandle(handle);
  obj->~APCCollection();
  std::free(obj);
}

namespace {
BaseMap* fillMap(BaseMap* map, const APCArray* data) {
  for (auto i = 0; i < data->size(); i++) {
    map->set(data->getKey(i).asTypedValue(),
             data->getValue(i)->toLocal().asTypedValue());
  }
  return map;
}

template<class TColl>
TColl* fillCollection(TColl* coll, const APCArray* data) {
  for (auto i = 0; i < data->size(); i++) {
    coll->add(data->getValue(i)->toLocal().asTypedValue());
  }
  return coll;
}

template<class TColl>
TColl* makeFromArray(ArrayData* data) {
  TColl* coll = NEWOBJ(TColl)();
  coll->t___construct(data);
  return coll;
}
}

Variant APCCollection::MakeObject(APCHandle* handle) {
  auto data = APCCollection::fromHandle(handle);
  if (data->m_data->isUncounted()) {
    auto array = APCTypedValue::fromHandle(data->m_data)->getArrayData();
    switch (data->m_type) {
    case Collection::Type::VectorType:
      return makeFromArray<c_Vector>(array);
    case Collection::Type::ImmVectorType:
      return makeFromArray<c_ImmVector>(array);
    case Collection::Type::MapType:
      return makeFromArray<c_Map>(array);
    case Collection::Type::ImmMapType:
      return makeFromArray<c_ImmMap>(array);
    case Collection::Type::SetType:
      return makeFromArray<c_Set>(array);
    case Collection::Type::ImmSetType:
      return makeFromArray<c_ImmSet>(array);
    default:
      assert(false);
    }
  } else {
    auto array = APCArray::fromHandle(data->m_data);
    switch (data->m_type) {
    case Collection::Type::VectorType:
      return fillCollection<BaseVector>(NEWOBJ(c_Vector)(), array);
    case Collection::Type::ImmVectorType:
      return fillCollection<BaseVector>(NEWOBJ(c_ImmVector)(), array);
    case Collection::Type::MapType:
      return fillMap(NEWOBJ(c_Map)(), array);
    case Collection::Type::ImmMapType:
      return fillMap(NEWOBJ(c_ImmMap)(), array);
    case Collection::Type::SetType:
      return fillCollection<BaseSet>(NEWOBJ(c_Set)(), array);
    case Collection::Type::ImmSetType:
      return fillCollection<BaseSet>(NEWOBJ(c_ImmSet)(), array);
    default:
      assert(false);
    }
  }
  return init_null();
}

//////////////////////////////////////////////////////////////////////

}
