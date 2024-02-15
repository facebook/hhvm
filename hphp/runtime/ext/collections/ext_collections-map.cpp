#include "hphp/runtime/ext/collections/ext_collections-map.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/coeffects.h"

namespace HPHP {

/////////////////////////////////////////////////////////////////////////////
// BaseMap

void BaseMap::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps");
}

BaseMap::~BaseMap() {
  auto const mixed = VanillaDict::as(arrayData());
  // Avoid indirect call, as we know it is a VanillaDict
  if (mixed->decReleaseCheck()) VanillaDict::Release(mixed);
}

template<typename TMap>
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, TMap*>::type
BaseMap::Clone(ObjectData* obj) {
  auto thiz = static_cast<TMap*>(obj);
  auto target = req::make<TMap>();
  if (!thiz->m_size) {
    return target.detach();
  }
  thiz->arrayData()->incRefCount();
  target->m_size = thiz->m_size;
  target->setArrayData(thiz->arrayData());
  return target.detach();
}

void BaseMap::setAllImpl(const Variant& iterable) {
  if (iterable.isNull()) return;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  for (; iter; ++iter) {
    set(iter.first(), iter.second());
  }
}

void BaseMap::addAllImpl(const Variant& iterable) {
  if (iterable.isNull()) return;
  VMRegGuard _;
  CoeffectsAutoGuard _2;

  decltype(cap()) oldCap = 0;
  bool ok = IterateKV(
    *iterable.asTypedValue(),
    [&](ArrayData* adata) {
      auto sz = adata->size();
      if (!sz) return true;
      if (m_size) {
        oldCap = cap(); // assume minimal collisions
        reserve(m_size + sz);
        mutate();
        return false;
      }
      // The ArrayData backing a Map must be a vanilla, unmarked dict.
      // Do all three escalations here. Dec-ref any intermediate values we
      // create along the way, but do not dec-ref the original adata.
      auto array = adata;
      if (!array->isVanilla()) {
        array = BespokeArray::ToVanilla(array, "BaseMap::addAllImpl");
      }
      if (array->isVanillaDict() && array->isLegacyArray()) {
        auto const tmp = array->setLegacyArray(array->cowCheck(), false);
        if (array != adata && array != tmp) decRefArr(array);
        array = tmp;
      }
      if (!array->isVanillaDict()) {
        auto const dict = array->toDict(array->cowCheck());
        if (array != adata && array != dict) decRefArr(array);
        array = dict;
      }
      replaceArray(array);
      if (array != adata) decRefArr(array);
      return true;
    },
    [this](TypedValue k, TypedValue v) {
      setRaw(k, v);
    },
    [this](ObjectData* coll) {
      switch (coll->collectionType()) {
        case CollectionType::Map:
        case CollectionType::Set:
        {
          if (m_size) break;
          auto hc = static_cast<HashCollection*>(coll);
          replaceArray(hc->arrayData());
          return true;
        }
        case CollectionType::Pair:
          mutate();
          break;
        default:
          break;
      }
      return false;
    },
    [this](const TypedValue* key, const TypedValue* value) {
      set(tvAsCVarRef(key), tvAsCVarRef(value));
    });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
  // ... and shrink back if that was incorrect
  if (oldCap) shrinkIfCapacityTooHigh(oldCap);
}

void BaseMap::addAllPairs(const Variant& iterable) {
  if (iterable.isNull()) return;
  VMRegGuard _;
  CoeffectsAutoGuard _2;
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto oldCap = cap();
  reserve(m_size + sz); // presume minimum key collisions ...
  for (; iter; ++iter) {
    add(iter.second());
  }
  // ... and shrink back if that was incorrect
  shrinkIfCapacityTooHigh(oldCap);
}

Variant BaseMap::firstKey() {
  if (!m_size) return uninit_variant;
  auto e = firstElm();
  assertx(e != elmLimit());
  if (e->hasIntKey()) {
    return e->ikey;
  }
  assertx(e->hasStrKey());
  return Variant{e->skey};
}

Variant BaseMap::firstValue() {
  if (!m_size) return uninit_variant;
  auto e = firstElm();
  assertx(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant BaseMap::lastKey() {
  if (!m_size) return uninit_variant;
  uint32_t pos = nthElmPos(m_size - 1);
  if (data()[pos].hasIntKey()) {
    return data()[pos].ikey;
  }
  assertx(data()[pos].hasStrKey());
  return Variant{data()[pos].skey};
}

Variant BaseMap::lastValue() {
  if (!m_size) return uninit_variant;
  uint32_t pos = nthElmPos(m_size - 1);
  return tvAsCVarRef(&data()[pos].data);
}

Object BaseMap::getIterator() {
  auto iter = collections::MapIterator::newInstance();
  Native::data<collections::MapIterator>(iter)->setMap(this);
  return iter;
}

void BaseMap::add(TypedValue tv) {
  if (UNLIKELY(tv.m_type != KindOfObject ||
               tv.m_data.pobj->getVMClass() != c_Pair::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be an instance of Pair");
  }
  auto pair = static_cast<c_Pair*>(tv.m_data.pobj);
  set(pair->elm0, pair->elm1);
}

Variant BaseMap::pop() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Map");
  }
  mutate();
  auto e = data() + nthElmPos(m_size - 1);
  Variant ret = tvAsCVarRef(&e->data);
  auto h = e->hash();
  auto ei = e->hasIntKey() ? findForRemove(e->ikey, h) :
            findForRemove(e->skey, h);
  erase(ei);
  return ret;
}

Variant BaseMap::popFront() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Map");
  }
  mutate();
  auto e = data() + nthElmPos(0);
  Variant ret = tvAsCVarRef(&e->data);
  auto h = e->hash();
  auto ei = e->hasIntKey() ? findForRemove(e->ikey, h) :
            findForRemove(e->skey, h);
  erase(ei);
  return ret;
}

void BaseMap::setImpl(int64_t k, TypedValue tv) {
  assertx(canMutateBuffer());
  auto ad = VanillaDict::SetIntMove(arrayData(), k, tv);
  setArrayData(VanillaDict::as(ad));
  m_size = arrayData()->m_size;
}

void BaseMap::setImpl(StringData* key, TypedValue tv) {
  assertx(canMutateBuffer());
  auto ad = VanillaDict::SetStrMove(arrayData(), key, tv);
  setArrayData(VanillaDict::as(ad));
  m_size = arrayData()->m_size;
}

void BaseMap::set(int64_t k, TypedValue val) {
  mutate();
  tvIncRefGen(val);
  setImpl(k, val);
}
void BaseMap::set(StringData* k, TypedValue val) {
  mutate();
  tvIncRefGen(val);
  setImpl(k, val);
}

void BaseMap::setMove(int64_t k, TypedValue val) {
  mutate();
  setImpl(k, val);
}
void BaseMap::setMove(StringData* k, TypedValue val) {
  mutate();
  setImpl(k, val);
}

void BaseMap::setRaw(int64_t k, TypedValue val) {
  tvIncRefGen(val);
  setImpl(k, val);
}
void BaseMap::setRaw(StringData* k, TypedValue val) {
  tvIncRefGen(val);
  setImpl(k, val);
}

bool BaseMap::ToBool(const ObjectData* obj) {
  return static_cast<const BaseMap*>(obj)->toBoolImpl();
}

void BaseMap::OffsetSet(ObjectData* obj, const TypedValue* key,
                        const TypedValue* val) {
  static_cast<BaseMap*>(obj)->set(*key, *val);
}

bool BaseMap::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  auto map = static_cast<BaseMap*>(obj);
  auto const ktv = tvClassToString(*key);
  TypedValue* result;
  if (ktv.m_type == KindOfInt64) {
    result = map->get(ktv.m_data.num);
  } else if (isStringType(ktv.m_type)) {
    result = map->get(ktv.m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !tvIsNull(result) : false;
}

bool BaseMap::OffsetContains(ObjectData* obj, const TypedValue* key) {
  auto map = static_cast<BaseMap*>(obj);
  auto const ktv = tvClassToString(*key);
  if (ktv.m_type == KindOfInt64) {
    return map->contains(ktv.m_data.num);
  }
  if (isStringType(ktv.m_type)) {
    return map->contains(ktv.m_data.pstr);
  }
  throwBadKeyType();
}

void BaseMap::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  auto map = static_cast<BaseMap*>(obj);
  auto const ktv = tvClassToString(*key);
  if (ktv.m_type == KindOfInt64) {
    map->remove(ktv.m_data.num);
    return;
  }
  if (isStringType(ktv.m_type)) {
    map->remove(ktv.m_data.pstr);
    return;
  }
  throwBadKeyType();
}

bool BaseMap::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto map1 = static_cast<const BaseMap*>(obj1);
  auto map2 = static_cast<const BaseMap*>(obj2);
  return VanillaDict::DictEqual(map1->arrayData(), map2->arrayData());
}

/////////////////////////////////////////////////////////////////////////////
// c_Map

void c_Map::clear() {
  dropImmCopy();
  decRefArr(arrayData());
  setArrayData(CreateDictAsMixed());
  m_size = 0;
}

// This function will create a immutable copy of this Map (if it doesn't
// already exist) and then return it
Object c_Map::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto map = req::make<c_ImmMap>();
    map->m_size = m_size;
    map->setArrayData(arrayData());
    m_immCopy = std::move(map);
    arrayData()->incRefCount();
  }
  assertx(!m_immCopy.isNull());
  assertx(data() == static_cast<c_ImmMap*>(m_immCopy.get())->data());
  assertx(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

c_Map* c_Map::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_Map>(obj);
}

/////////////////////////////////////////////////////////////////////////////
// c_ImmMap

c_ImmMap* c_ImmMap::Clone(ObjectData* obj) {
  return BaseMap::Clone<c_ImmMap>(obj);
}

namespace collections {

/////////////////////////////////////////////////////////////////////////////
// MapIterator

static Variant HHVM_METHOD(MapIterator, current) {
  return Native::data<MapIterator>(this_)->current();
}

static Variant HHVM_METHOD(MapIterator, key) {
  return Native::data<MapIterator>(this_)->key();
}

static bool HHVM_METHOD(MapIterator, valid) {
  return Native::data<MapIterator>(this_)->valid();
}

static void HHVM_METHOD(MapIterator, next) {
  Native::data<MapIterator>(this_)->next();
}

static void HHVM_METHOD(MapIterator, rewind) {
  Native::data<MapIterator>(this_)->rewind();
}

/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::registerNativeMap() {
  HHVM_ME(MapIterator, current);
  HHVM_ME(MapIterator, key);
  HHVM_ME(MapIterator, valid);
  HHVM_ME(MapIterator, next);
  HHVM_ME(MapIterator, rewind);

  Native::registerNativeDataInfo<MapIterator>(Native::NDIFlags::NO_SWEEP);

  // BaseMap common

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Map,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmMap, mn, impl);
  BASE_ME(__construct,   &BaseMap::init);
  BASE_ME(count,         &BaseMap::size);
  BASE_ME(firstKey,      &BaseMap::firstKey);
  BASE_ME(firstValue,    &BaseMap::firstValue);
  BASE_ME(lastKey,       &BaseMap::lastKey);
  BASE_ME(lastValue,     &BaseMap::lastValue);
  BASE_ME(at,            &BaseMap::php_at);
  BASE_ME(get,           &BaseMap::php_get);

  BASE_ME(containsKey,   &BaseMap::php_containsKey);
  BASE_ME(contains,      &BaseMap::php_containsKey);
  BASE_ME(getIterator,   &BaseMap::getIterator);

#undef BASE_ME

  HHVM_NAMED_ME(HH\\Map,    toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Map,    toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Map,    toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\Map,    toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\ImmMap, toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\ImmMap, toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\ImmMap, toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\ImmMap, toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\ImmMap, toMap,       materialize<c_Map>);

  // c_Map specific
  HHVM_NAMED_ME(HH\\Map, clearNative,     &c_Map::clear);
  HHVM_NAMED_ME(HH\\Map, setNative,       &c_Map::php_set);
  HHVM_NAMED_ME(HH\\Map, addNative,       &c_Map::php_add);
  HHVM_NAMED_ME(HH\\Map, addAllNative,    &c_Map::php_addAll);
  HHVM_NAMED_ME(HH\\Map, removeKeyNative, &c_Map::php_removeKey);
  HHVM_NAMED_ME(HH\\Map, reserve,         &c_Map::php_reserve);

  HHVM_NAMED_ME(HH\\Map, toImmMap,      &c_Map::getImmutableCopy);

  Native::registerNativePropHandler<CollectionPropHandler>(c_Map::className());
  Native::registerNativePropHandler<CollectionPropHandler>(c_ImmMap::className());

  Native::registerClassExtraDataHandler(c_Map::className(), finish_class<c_Map>);
  Native::registerClassExtraDataHandler(c_ImmMap::className(), finish_class<c_ImmMap>);
}

/////////////////////////////////////////////////////////////////////////////
}}
