#include "hphp/runtime/ext/collections/ext_collections-map.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

Class* c_Map::s_cls;
Class* c_ImmMap::s_cls;

/////////////////////////////////////////////////////////////////////////////
// BaseMap

void BaseMap::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer keys and string keys may be used with Maps");
}

BaseMap::~BaseMap() {
  auto const mixed = MixedArray::asMixed(arrayData());
  // Avoid indirect call, as we know it is a MixedArray
  if (mixed->decReleaseCheck()) MixedArray::ReleaseDict(mixed);
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
  target->m_arr = thiz->m_arr;
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
      } else if (adata->isDictKind()) {
        replaceArray(adata);
        return true;
      } else {
        auto dict = adata->toDict(adata->cowCheck());
        replaceArray(dict);
        if (dict != adata) dict->decRefCount();
        return true;
      }
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
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assertx(pos > 0);
    --pos;
  }
  if (data()[pos].hasIntKey()) {
    return data()[pos].ikey;
  }
  assertx(data()[pos].hasStrKey());
  return Variant{data()[pos].skey};
}

Variant BaseMap::lastValue() {
  if (!m_size) return uninit_variant;
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assertx(pos > 0);
    --pos;
  }
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
  auto e = elmLimit() - 1;
  for (;; --e) {
    assertx(e >= data());
    if (!isTombstone(e)) break;
  }
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
  auto e = data();
  for (;; ++e) {
    assertx(e != elmLimit());
    if (!isTombstone(e)) break;
  }
  Variant ret = tvAsCVarRef(&e->data);
  auto h = e->hash();
  auto ei = e->hasIntKey() ? findForRemove(e->ikey, h) :
            findForRemove(e->skey, h);
  erase(ei);
  return ret;
}

Array BaseMap::toPHPArray() {
  if (RuntimeOption::EvalHackArrCompatArrayProducingFuncNotices) {
    raise_hack_arr_compat_array_producing_func_notice("Map::toArray");
  }
  return toPHPArrayImpl<IntishCast::None>();
}

void BaseMap::setImpl(int64_t k, TypedValue tv) {
  assertx(canMutateBuffer());
  auto h = hash_int64(k);
retry:
  auto p = findForInsert(k, h);
  assertx(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
    auto& e = data()[(int32_t)*p];
    tvMove(tv, e.data);
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    goto retry;
  }
  auto& e = allocElm(p);
  tvCopy(tv, e.data);
  e.setIntKey(k, h);
  arrayData()->mutableKeyTypes()->recordInt();
  updateNextKI(k);
}

void BaseMap::setImpl(StringData* key, TypedValue tv) {
  assertx(canMutateBuffer());
retry:
  strhash_t h = key->hash();
  auto p = findForInsert(key, h);
  assertx(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
    auto& e = data()[(int32_t)*p];
    tvMove(tv, e.data);
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    goto retry;
  }
  auto& e = allocElm(p);
  tvCopy(tv, e.data);
  e.setStrKey(key, h);
  arrayData()->mutableKeyTypes()->recordStr(key);
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
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = map->get(key->m_data.num);
  } else if (isStringType(key->m_type)) {
    result = map->get(key->m_data.pstr);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !tvIsNull(result) : false;
}

bool BaseMap::OffsetContains(ObjectData* obj, const TypedValue* key) {
  auto map = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    return map->contains(key->m_data.num);
  }
  if (isStringType(key->m_type)) {
    return map->contains(key->m_data.pstr);
  }
  throwBadKeyType();
}

void BaseMap::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  auto map = static_cast<BaseMap*>(obj);
  if (key->m_type == KindOfInt64) {
    map->remove(key->m_data.num);
    return;
  }
  if (isStringType(key->m_type)) {
    map->remove(key->m_data.pstr);
    return;
  }
  throwBadKeyType();
}

bool BaseMap::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto map1 = static_cast<const BaseMap*>(obj1);
  auto map2 = static_cast<const BaseMap*>(obj2);
  return MixedArray::DictEqual(map1->arrayData(), map2->arrayData());
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_differenceByKey(const Variant& it) {
  if (!it.isObject()) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter it must be an instance of Iterable");
  }
  ObjectData* obj = it.getObjectData();
  TMap* target = BaseMap::Clone<TMap>(this);
  auto ret = Object::attach(target);
  if (obj->isCollection()) {
    if (isMapCollection(obj->collectionType())) {
      auto map = static_cast<BaseMap*>(obj);
      auto eLimit = map->elmLimit();
      for (auto e = map->firstElm(); e != eLimit; e = nextElm(e, eLimit)) {
        if (e->hasIntKey()) {
          target->remove((int64_t)e->ikey);
        } else {
          assertx(e->hasStrKey());
          target->remove(e->skey);
        }
      }
      return ret;
    }
  }
  for (ArrayIter iter(obj); iter; ++iter) {
    Variant k = iter.first();
    if (k.isInteger()) {
      target->remove(k.toInt64());
    } else {
      assertx(k.isString());
      target->remove(k.getStringData());
    }
  }
  return ret;
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_zip(const Variant& iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto map = req::make<TMap>();
  if (!m_size) {
    return Object{std::move(map)};
  }
  map->reserve(std::min(sz, size_t(m_size)));
  uint32_t used = posLimit();
  for (uint32_t i = 0; i < used && iter; ++i) {
    if (isTombstone(i)) continue;
    const Elm& e = data()[i];
    Variant v = iter.second();
    auto pair = req::make<c_Pair>(e.data, *v.asTypedValue());
    auto const tv = make_tv<KindOfObject>(pair.get());
    if (e.hasIntKey()) {
      map->setRaw(e.ikey, tv);
    } else {
      assertx(e.hasStrKey());
      map->setRaw(e.skey, tv);
    }
    ++iter;
  }
  return Object{std::move(map)};
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_take(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto map = req::make<TMap>();
  if (len <= 0) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return Object{std::move(map)};
  }
  size_t sz = size_t(len);
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  map->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(map)};
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Map will simply be a copy of this Map,
    // so we can just call Clone() and return early here.
    return Object::attach(TMap::Clone(this));
  }
  auto map = req::make<TMap>();
  if (len >= m_size) {
    // We know the resulting Map will be empty, so we can return
    // early here.
    return Object{std::move(map)};
  }
  size_t sz = size_t(m_size) - size_t(len);
  assertx(sz);
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  map->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  uint32_t frPos = nthElmPos(len);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(map)};
}

template<class TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::php_slice(const Variant& start, const Variant& len) {
  int64_t istart;
  int64_t ilen;
  if (!start.isInteger() || (istart = start.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter start must be a non-negative integer");
  }
  if (!len.isInteger() || (ilen = len.toInt64()) < 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter len must be a non-negative integer");
  }
  size_t skipAmt = std::min<size_t>(istart, m_size);
  size_t sz = std::min<size_t>(ilen, size_t(m_size) - skipAmt);
  auto map = req::make<TMap>();
  map->reserve(sz);
  map->setSize(sz);
  map->setPosLimit(sz);
  map->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = map->hashTab();
  auto mask = map->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = map->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      map->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(map)};
}

template<class TVector>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseMap::php_concat(const Variant& iterable) {
  size_t itSize;
  ArrayIter iter = getArrayIterHelper(iterable, itSize);
  auto vec = req::make<TVector>();
  uint32_t sz = m_size;
  vec->reserve((size_t)sz + itSize);
  assertx(vec->canMutateBuffer());
  vec->setSize(sz);
  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    tvDup(data()[i].data, vec->dataAt(j));
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::FromItems(const Class*, const Variant& iterable) {
  if (iterable.isNull()) return Object{req::make<TMap>()};
  VMRegGuard _;

  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  auto target = req::make<TMap>();
  target->reserve(sz);
  for (; iter; ++iter) {
    Variant v = iter.second();
    TypedValue* tv = v.asTypedValue();
    if (UNLIKELY(tv->m_type != KindOfObject ||
                 tv->m_data.pobj->getVMClass() != c_Pair::classof())) {
      SystemLib::throwInvalidArgumentExceptionObject(
                 "Parameter must be an instance of Iterable<Pair>");
    }
    auto pair = static_cast<c_Pair*>(tv->m_data.pobj);
    target->setRaw(pair->elm0, pair->elm1);
  }
  return Object{std::move(target)};
}

template<typename TMap>
ALWAYS_INLINE
typename std::enable_if<
  std::is_base_of<BaseMap, TMap>::value, Object>::type
BaseMap::FromArray(const Class*, const Variant& arr) {
  if (!arr.isArray()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter arr must be an array");
  }
  auto map = req::make<TMap>();
  ArrayData* ad = arr.getArrayData();
  map->reserve(ad->size());
  for (ssize_t pos = ad->iter_begin(), limit = ad->iter_end(); pos != limit;
       pos = ad->iter_advance(pos)) {
    Variant k = ad->getKey(pos);
    auto const tv = ad->nvGetVal(pos);
    if (k.isInteger()) {
      map->setRaw(k.toInt64(), tv);
    } else {
      assertx(k.isString());
      map->setRaw(k.getStringData(), tv);
    }
  }
  return Object(std::move(map));
}


/////////////////////////////////////////////////////////////////////////////
// c_Map

void c_Map::clear() {
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = CreateDictAsMixed();
  m_size = 0;
}

// This function will create a immutable copy of this Map (if it doesn't
// already exist) and then return it
Object c_Map::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto map = req::make<c_ImmMap>();
    map->m_size = m_size;
    map->m_arr = m_arr;
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

const StaticString
  s_HH_Map("HH\\Map"),
  s_HH_ImmMap("HH\\ImmMap"),
  s_MapIterator("MapIterator");

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

void CollectionsExtension::initMap() {
  HHVM_ME(MapIterator, current);
  HHVM_ME(MapIterator, key);
  HHVM_ME(MapIterator, valid);
  HHVM_ME(MapIterator, next);
  HHVM_ME(MapIterator, rewind);

  Native::registerNativeDataInfo<MapIterator>(
    s_MapIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

  // BaseMap common

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Map,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmMap, mn, impl);
  BASE_ME(__construct,   &BaseMap::init);
  BASE_ME(count,         &BaseMap::size);
  BASE_ME(toArray,       &BaseMap::toPHPArray);
  BASE_ME(toVArray,      &BaseMap::toVArray);
  BASE_ME(toDArray,      &BaseMap::toDArray);
  BASE_ME(toKeysArray,   &BaseMap::toKeysArray);
  BASE_ME(toValuesArray, &BaseMap::toValuesArray);
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

#define TMPL_ME(mn, col) \
  HHVM_NAMED_ME(HH\\Map,    mn, &BaseMap::php_##mn<c_##col>); \
  HHVM_NAMED_ME(HH\\ImmMap, mn, &BaseMap::php_##mn<c_Imm##col>);
  TMPL_ME(differenceByKey, Map);
  TMPL_ME(slice,           Map);
  TMPL_ME(skip,            Map);
  TMPL_ME(take,            Map);
  TMPL_ME(zip,             Map);
  TMPL_ME(keys,            Vector);
  TMPL_ME(values,          Vector);
  TMPL_ME(concat,          Vector);
#undef TMPL_ME

  HHVM_NAMED_ME(HH\\Map,    toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Map,    toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Map,    toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\Map,    toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\ImmMap, toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\ImmMap, toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\ImmMap, toSet,       materialize<c_Set>);
  HHVM_NAMED_ME(HH\\ImmMap, toImmSet,    materialize<c_ImmSet>);
  HHVM_NAMED_ME(HH\\ImmMap, toMap,       materialize<c_Map>);

  HHVM_NAMED_STATIC_ME(HH\\Map,    fromItems, BaseMap::FromItems<c_Map>);
  HHVM_NAMED_STATIC_ME(HH\\Map,    fromArray, BaseMap::FromArray<c_Map>);
  HHVM_NAMED_STATIC_ME(HH\\ImmMap, fromItems, BaseMap::FromItems<c_ImmMap>);
  HHVM_NAMED_STATIC_ME(HH\\ImmMap, fromArray, BaseMap::FromArray<c_ImmMap>);

  // c_Map specific
  HHVM_NAMED_ME(HH\\Map, add,           &c_Map::php_add);
  HHVM_NAMED_ME(HH\\Map, addAll,        &c_Map::php_addAll);
  HHVM_NAMED_ME(HH\\Map, set,           &c_Map::php_set);
  HHVM_NAMED_ME(HH\\Map, setAll,        &c_Map::php_setAll);
  HHVM_NAMED_ME(HH\\Map, removeKey,     &c_Map::php_removeKey);
  HHVM_NAMED_ME(HH\\Map, clear,         &c_Map::php_clear);
  HHVM_NAMED_ME(HH\\Map, reserve,       &c_Map::php_reserve);

  HHVM_NAMED_ME(HH\\Map, toImmMap,      &c_Map::getImmutableCopy);

  loadSystemlib("collections-map");

  c_Map::s_cls = Unit::lookupClass(s_HH_Map.get());
  assertx(c_Map::s_cls);
  finishClass<c_Map>();

  c_ImmMap::s_cls = Unit::lookupClass(s_HH_ImmMap.get());
  assertx(c_ImmMap::s_cls);
  finishClass<c_ImmMap>();
}

/////////////////////////////////////////////////////////////////////////////
}}
