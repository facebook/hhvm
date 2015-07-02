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
#include "hphp/runtime/base/collections.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

#define X(type) const StaticString s_##type("HH\\" #type);
COLLECTIONS_ALL_TYPES(X)
#undef X

/////////////////////////////////////////////////////////////////////////////
// Constructor/Initializer

ObjectData* allocEmptyPair() {
  return newCollectionObj<c_Pair>(c_Pair::NoInit{});
}

#define X(type) \
ObjectData* allocEmpty##type() {                                        \
  return newCollectionObj<c_##type>(c_##type::classof());               \
}                                                                       \
ObjectData* allocFromArray##type(ArrayData* arr) {                      \
  return newCollectionObj<c_##type>(c_##type::classof(), arr);          \
}
COLLECTIONS_PAIRED_TYPES(X)
#undef X

newFromArrayFunc allocFromArrayFunc(CollectionType ctype) {
  switch (ctype) {
#define X(type) case CollectionType::type: return allocFromArray##type;
COLLECTIONS_PAIRED_TYPES(X)
#undef X
    case CollectionType::Pair: not_reached();
  }
  not_reached();
}

newEmptyInstanceFunc allocEmptyFunc(CollectionType ctype) {
  switch (ctype) {
#define X(type) case CollectionType::type: return allocEmpty##type;
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

void reserve(ObjectData* obj, int64_t sz) {
  switch (obj->collectionType()) {
#define X(type) case CollectionType::type: \
                  static_cast<c_##type*>(obj)->reserve(sz); \
                  break;
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
}

void initMapElem(ObjectData* obj, TypedValue* key, TypedValue* val) {
  assertx(obj->isCollection());
  assertx(isMapCollection(obj->collectionType()));
  assertx(key->m_type != KindOfRef);
  assertx(val->m_type != KindOfRef);
  assertx(val->m_type != KindOfUninit);
  BaseMap::OffsetSet(obj, key, val);
}

void initElem(ObjectData* obj, TypedValue* val) {
  assertx(obj->isCollection());
  assertx(!isMapCollection(obj->collectionType()));
  assertx(val->m_type != KindOfRef);
  assertx(val->m_type != KindOfUninit);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      static_cast<BaseVector*>(obj)->add(val);
      break;
    case CollectionType::Set:
    case CollectionType::ImmSet:
      static_cast<BaseSet*>(obj)->add(val);
      break;
    case CollectionType::Pair:
      static_cast<c_Pair*>(obj)->initAdd(val);
      break;
    case CollectionType::Map:
    case CollectionType::ImmMap:
      assertx(false);
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Misc

bool isType(const Class* cls, CollectionType ctype) {
  switch (ctype) {
#define X(type) case CollectionType::type: return cls == c_##type::classof();
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

uint32_t sizeOffset(CollectionType ctype) {
  switch (ctype) {
    case CollectionType::Vector:    return c_Vector::sizeOffset();
    case CollectionType::ImmVector: return c_ImmVector::sizeOffset();
    default:
      always_assert(false);
  }
}

uint32_t dataOffset(CollectionType ctype) {
  switch (ctype) {
#define X(type) case CollectionType::type: return c_##type::dataOffset();
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

void unserialize(ObjectData* obj, VariableUnserializer* uns,
                 int64_t sz, char type) {
  switch (obj->collectionType()) {
    case CollectionType::Pair:
      c_Pair::Unserialize(obj, uns, sz, type);
      break;
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      BaseVector::Unserialize(obj, uns, sz, type);
      break;
    case CollectionType::Map:
    case CollectionType::ImmMap:
      BaseMap::Unserialize(obj, uns, sz, type);
      break;
    case CollectionType::Set:
    case CollectionType::ImmSet:
      BaseSet::Unserialize(obj, uns, sz, type);
      break;
  }
}

void serialize(ObjectData* obj, VariableSerializer* serializer) {
  int64_t sz = getCollectionSize(obj);
  auto type = obj->collectionType();

  if (isMapCollection(type)) {
    serializer->pushObjectInfo(obj->getClassName(), obj->getId(), 'K');
    serializer->writeArrayHeader(sz, false);
    for (ArrayIter iter(obj); iter; ++iter) {
      serializer->writeCollectionKey(iter.first());
      serializer->writeArrayValue(iter.second());
    }
    serializer->writeArrayFooter();

  } else {
    assertx(isVectorCollection(type) ||
            isSetCollection(type) ||
            (type == CollectionType::Pair));
    serializer->pushObjectInfo(obj->getClassName(), obj->getId(), 'V');
    serializer->writeArrayHeader(sz, true);
    auto ser_type = serializer->getType();
    if (ser_type == VariableSerializer::Type::Serialize ||
        ser_type == VariableSerializer::Type::APCSerialize ||
        ser_type == VariableSerializer::Type::DebuggerSerialize ||
        ser_type == VariableSerializer::Type::VarExport ||
        ser_type == VariableSerializer::Type::PHPOutput) {
      // For the 'V' serialization format, we don't print out keys
      // for Serialize, APCSerialize, DebuggerSerialize
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeCollectionKeylessPrefix();
        serializer->writeArrayValue(iter.second());
      }
    } else {
      if (isSetCollection(type)) {
        for (ArrayIter iter(obj); iter; ++iter) {
          serializer->writeCollectionKeylessPrefix();
          serializer->writeArrayValue(iter.second());
        }
      } else {
        for (ArrayIter iter(obj); iter; ++iter) {
          serializer->writeCollectionKey(iter.first());
          serializer->writeArrayValue(iter.second());
        }
      }
    }
    serializer->writeArrayFooter();
  }
  serializer->popObjectInfo();
}

/////////////////////////////////////////////////////////////////////////////
// Casting and Copying

Array toArray(const ObjectData* obj) {
  assertx(obj->isCollection());
  switch (obj->collectionType()) {
#define X(type) case CollectionType::type: return c_##type::ToArray(obj);
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

bool toBool(const ObjectData* obj) {
  assertx(obj->isCollection());
  switch (obj->collectionType()) {
#define X(type) case CollectionType::type: return c_##type::ToBool(obj);
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

ObjectData* clone(ObjectData* obj) {
  assertx(obj->isCollection());
  switch (obj->collectionType()) {
#define X(type) case CollectionType::type: return c_##type::Clone(obj);
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  not_reached();
}

ArrayData* asArray(ObjectData* obj) {
  assertx(obj->isCollection());
  switch (obj->collectionType()) {
  case CollectionType::ImmVector:
  case CollectionType::Vector:
    return static_cast<BaseVector*>(obj)->arrayData();
  case CollectionType::ImmMap:
  case CollectionType::Map:
  case CollectionType::ImmSet:
  case CollectionType::Set:
    return static_cast<HashCollection*>(obj)->arrayData()->asArrayData();
  case CollectionType::Pair:
    return nullptr;
  }
  not_reached();
}

/////////////////////////////////////////////////////////////////////////////
// Deep Copy

ArrayData* deepCopyArray(ArrayData* arr) {
  ArrayInit ai(arr->size(), ArrayInit::Mixed{});
  for (ArrayIter iter(arr); iter; ++iter) {
    Variant v = iter.secondRef();
    deepCopy(v.asTypedValue());
    ai.set(iter.first(), std::move(v));
  }
  return ai.toArray().detach();
}

ObjectData* deepCopySet(c_Set* st) {
  return c_Set::Clone(st);
}

ObjectData* deepCopyImmSet(c_ImmSet* st) {
  return c_ImmSet::Clone(st);
}

void deepCopy(TypedValue* tv) {
  switch (tv->m_type) {
    DT_UNCOUNTED_CASE:
    case KindOfString:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      return;

    case KindOfArray: {
      auto arr = deepCopyArray(tv->m_data.parr);
      decRefArr(tv->m_data.parr);
      tv->m_data.parr = arr;
      return;
    }

    case KindOfObject: {
      auto obj = tv->m_data.pobj;
      if (!obj->isCollection()) return;
      const auto copyVector = [](BaseVector* vec) {
        Object o = Object::attach(vec);
        vec->mutate();
        assertx(vec->canMutateBuffer());
        auto sz = vec->m_size;
        for (size_t i = 0; i < sz; ++i) {
          deepCopy(&vec->m_data[i]);
        }
        return o.detach();
      };
      const auto copyMap = [](BaseMap* mp) {
        Object o = Object::attach(mp);
        mp->mutate();
        auto used = mp->posLimit();
        for (uint32_t i = 0; i < used; ++i) {
          if (mp->isTombstone(i)) continue;
          auto* e = &mp->data()[i];
          deepCopy(&e->data);
        }
        return o.detach();
      };
      switch (obj->collectionType()) {
        case CollectionType::Pair: {
          auto pair = c_Pair::Clone(static_cast<c_Pair*>(obj));
          Object o = Object::attach(pair);
          deepCopy(&pair->elm0);
          deepCopy(&pair->elm1);
          obj = o.detach();
          break;
        }
        case CollectionType::Vector:
          obj = copyVector(c_Vector::Clone(static_cast<c_Vector*>(obj)));
          break;
        case CollectionType::ImmVector:
          obj = copyVector(c_ImmVector::Clone(static_cast<c_ImmVector*>(obj)));
          break;
        case CollectionType::Map:
          obj = copyMap(c_Map::Clone(static_cast<c_Map*>(obj)));
          break;
        case CollectionType::ImmMap:
          obj = copyMap(c_ImmMap::Clone(static_cast<c_ImmMap*>(obj)));
          break;
        case CollectionType::Set:
          obj = c_Set::Clone(static_cast<c_Set*>(obj));
          break;
        case CollectionType::ImmSet:
          obj = c_ImmSet::Clone(static_cast<c_ImmSet*>(obj));
          break;
        default:
          assertx(false);
      }
      decRefObj(tv->m_data.pobj);
      tv->m_data.pobj = obj;
      return;
    }
  }
  not_reached();
}

/////////////////////////////////////////////////////////////////////////////
// Read/Write access

template <bool throwOnMiss>
static inline TypedValue* atImpl(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  switch (obj->collectionType()) {
#define X(type) case CollectionType::type: \
                  return c_##type::OffsetAt<throwOnMiss>(obj, key);
COLLECTIONS_ALL_TYPES(X)
#undef X
  }
  return nullptr;
}

TypedValue* at(ObjectData* obj, const TypedValue* key) {
  return atImpl<true>(obj, key);
}
TypedValue* get(ObjectData* obj, const TypedValue* key) {
  return atImpl<false>(obj, key);
}

TypedValue* atLval(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  TypedValue* ret;
  switch (obj->collectionType()) {
    case CollectionType::Pair:
      ret = c_Pair::OffsetAt<true>(obj, key);
      break;

    case CollectionType::Vector: {
      ret = BaseVector::OffsetAt<true>(obj, key);
      // We're about to expose an element of a Vector in an lvalue context;
      // if the element is a value-type (anything other than objects and
      // resources) we need to sever any buffer sharing that might be going on
      auto* vec = static_cast<c_Vector*>(obj);
      if (UNLIKELY(!vec->canMutateBuffer() &&
                   ret->m_type != KindOfObject &&
                   ret->m_type != KindOfResource)) {
        vec->mutate();
        ret = BaseVector::OffsetAt<true>(obj, key);
      }
      return ret;
    }
    case CollectionType::ImmVector:
      ret = BaseVector::OffsetAt<true>(obj, key);
      break;

    case CollectionType::Map: {
      ret = BaseMap::OffsetAt<true>(obj, key);
      // We're about to expose an element of a Map in an lvalue context;
      // if the element is a value-type (anything other than objects and
      // resources) we need to sever any buffer sharing that might be going on
      auto* mp = static_cast<c_Map*>(obj);
      if (UNLIKELY(!mp->canMutateBuffer() &&
                   ret->m_type != KindOfObject &&
                   ret->m_type != KindOfResource)) {
        mp->mutate();
        ret = BaseMap::OffsetAt<true>(obj, key);
      }
      return ret;
    }
    case CollectionType::ImmMap:
      ret = BaseMap::OffsetAt<true>(obj, key);
      break;

    case CollectionType::Set:
    case CollectionType::ImmSet:
      BaseSet::throwNoMutableIndexAccess();

  }
  // Value-type elements (anything other than objects and resources) of
  // an immutable collection "inherit" the collection's immutable status.
  // We do not allow value-type elements of an immutable collection to
  // be read in an "lvalue" context in order to prevent null->array
  // promotion, null->stdClass promotion, and mutating strings or arrays
  // in place (see "test/slow/collection_classes/invalid-operations.php"
  // for examples).
  if (ret->m_type != KindOfObject && ret->m_type != KindOfResource) {
    throw_cannot_modify_immutable_object(obj->getClassName().data());
  }
  return ret;
}

TypedValue* atRw(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      // Since we're exposing an element of a Vector in an read/write context,
      // we need to sever any buffer sharing that might be going on.
      static_cast<c_Vector*>(obj)->mutate();
      return BaseVector::OffsetAt<true>(obj, key);
    case CollectionType::Map:
      static_cast<c_Map*>(obj)->mutate();
      return BaseMap::OffsetAt<true>(obj, key);
    case CollectionType::Set:
    case CollectionType::ImmSet:
      BaseSet::throwNoMutableIndexAccess();
    case CollectionType::ImmVector:
    case CollectionType::ImmMap:
    case CollectionType::Pair:
      throw_cannot_modify_immutable_object(obj->getClassName().data());
  }
  return nullptr;
}

bool contains(ObjectData* obj, const Variant& offset) {
  auto* key = offset.asCell();
  switch (obj->collectionType()) {
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      return BaseVector::OffsetContains(obj, key);
    case CollectionType::Map:
    case CollectionType::ImmMap:
      return BaseMap::OffsetContains(obj, key);
    case CollectionType::Set:
    case CollectionType::ImmSet:
      return BaseSet::OffsetContains(obj, key);
    case CollectionType::Pair:
      return c_Pair::OffsetContains(obj, key);
  }
  not_reached();
}

bool isset(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      return BaseVector::OffsetIsset(obj, key);
    case CollectionType::Map:
    case CollectionType::ImmMap:
      return BaseMap::OffsetIsset(obj, key);
    case CollectionType::Set:
    case CollectionType::ImmSet:
      return BaseSet::OffsetIsset(obj, key);
    case CollectionType::Pair:
      return c_Pair::OffsetIsset(obj, key);
  }
  not_reached();
}

bool empty(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
    case CollectionType::ImmVector:
      return BaseVector::OffsetEmpty(obj, key);
    case CollectionType::Map:
    case CollectionType::ImmMap:
      return BaseMap::OffsetEmpty(obj, key);
    case CollectionType::Set:
    case CollectionType::ImmSet:
      return BaseSet::OffsetEmpty(obj, key);
    case CollectionType::Pair:
      return c_Pair::OffsetEmpty(obj, key);
  }
  not_reached();
}

void unset(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      c_Vector::OffsetUnset(obj, key);
      break;
    case CollectionType::Map:
      c_Map::OffsetUnset(obj, key);
      break;
    case CollectionType::Set:
      BaseSet::OffsetUnset(obj, key);
      break;
    case CollectionType::ImmVector:
    case CollectionType::ImmMap:
    case CollectionType::ImmSet:
    case CollectionType::Pair:
      throw_cannot_modify_immutable_object(obj->getClassName().data());
  }
}

void append(ObjectData* obj, TypedValue* val) {
  assertx(val->m_type != KindOfRef);
  assertx(val->m_type != KindOfUninit);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      static_cast<c_Vector*>(obj)->add(val);
      break;
    case CollectionType::Map:
      static_cast<c_Map*>(obj)->add(val);
      break;
    case CollectionType::Set:
      static_cast<c_Set*>(obj)->add(val);
      break;
    case CollectionType::ImmVector:
    case CollectionType::ImmMap:
    case CollectionType::ImmSet:
    case CollectionType::Pair:
      throw_cannot_modify_immutable_object(obj->getClassName().data());
  }
}

void set(ObjectData* obj, const TypedValue* key, const TypedValue* val) {
  assertx(key->m_type != KindOfRef);
  assertx(val->m_type != KindOfRef);
  assertx(val->m_type != KindOfUninit);
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      c_Vector::OffsetSet(obj, key, val);
      break;
    case CollectionType::Map:
      BaseMap::OffsetSet(obj, key, val);
      break;
    case CollectionType::Set:
    case CollectionType::ImmSet:
      BaseSet::throwNoMutableIndexAccess();
      break;
    case CollectionType::ImmVector:
    case CollectionType::ImmMap:
    case CollectionType::Pair:
      throw_cannot_modify_immutable_object(obj->getClassName().data());
  }
}

bool equals(const ObjectData* obj1, const ObjectData* obj2) {
  assertx(isValidCollection(obj1->collectionType()));
  if (!obj2->isCollection()) return false;
  auto ct1 = obj1->collectionType();
  auto ct2 = obj2->collectionType();

  if (isMapCollection(ct1) && isMapCollection(ct2)) {
    // For migration purposes, distinct Map types should compare equal
    return BaseMap::Equals(
      BaseMap::EqualityFlavor::OrderIrrelevant, obj1, obj2);
  }

  if (isVectorCollection(ct1) && isVectorCollection(ct2)) {
    return BaseVector::Equals(obj1, obj2);
  }

  if (isSetCollection(ct1) && isSetCollection(ct2)) {
    return BaseSet::Equals(obj1, obj2);
  }

  if (ct1 != ct2) return false;
  assert(ct1 == CollectionType::Pair);
  return c_Pair::Equals(obj1, obj2);
}

Variant pop(ObjectData* obj) {
  assertx(obj->isCollection());
  assertx(isMutableCollection(obj->collectionType()));
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      return static_cast<c_Vector*>(obj)->t_pop();
    case CollectionType::Map:
      return static_cast<c_Map*>(obj)->pop();
    case CollectionType::Set:
      return static_cast<c_Set*>(obj)->pop();
    default:
      assertx(false);
  }
  not_reached();
}

Variant shift(ObjectData* obj) {
  assertx(obj->isCollection());
  assertx(isMutableCollection(obj->collectionType()));
  switch (obj->collectionType()) {
    case CollectionType::Vector:
      return static_cast<c_Vector*>(obj)->popFront();
    case CollectionType::Map:
      return static_cast<c_Map*>(obj)->popFront();
    case CollectionType::Set:
      return static_cast<c_Set*>(obj)->popFront();
    default:
      assertx(false);
  }
  not_reached();
}

/////////////////////////////////////////////////////////////////////////////
}}
