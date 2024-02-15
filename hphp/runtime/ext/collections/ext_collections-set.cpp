#include "hphp/runtime/ext/collections/ext_collections-set.h"

#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

/////////////////////////////////////////////////////////////////////////////
// BaseSet

void BaseSet::addAllKeysOf(const TypedValue container) {
  assertx(isContainer(container));

  decltype(cap()) oldCap = 0;
  bool ok =
    IterateKV(container,
              [&](ArrayData* adata) {
                auto sz = adata->size();
                if (!sz) return true;
                if (m_size) {
                  oldCap = cap(); // assume minimal collisions
                }
                reserve(m_size + sz);
                mutate();
                return false;
              },
              [this](TypedValue k, TypedValue /*v*/) { addRaw(k); },
              [this](ObjectData* coll) {
                if (!m_size && coll->collectionType() == CollectionType::Set) {
                  auto hc = static_cast<HashCollection*>(coll);
                  replaceArray(hc->arrayData());
                  return true;
                }
                if (coll->collectionType() == CollectionType::Pair) {
                  mutate();
                }
                return false;
              },
              false);

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
  // ... and shrink back if that was incorrect
  if (oldCap) shrinkIfCapacityTooHigh(oldCap);
}

void BaseSet::addAll(const Variant& t) {
  if (t.isNull()) { return; } // nothing to do

  CoeffectsAutoGuard _;
  decltype(cap()) oldCap = 0;
  bool ok = IterateV(
    *t.asTypedValue(),
    [&](ArrayData* adata) {
      auto sz = adata->size();
      if (!sz) return true;
      if (m_size) {
        oldCap = cap(); // assume minimal collisions
      }
      reserve(m_size + sz);
      mutate();
      return false;
    },
    [this](TypedValue v) {
      addRaw(v);
    },
    [this](ObjectData* coll) {
      if (!m_size && coll->collectionType() == CollectionType::Set) {
        auto hc = static_cast<HashCollection*>(coll);
        replaceArray(hc->arrayData());
        return true;
      }
      if (coll->collectionType() == CollectionType::Pair) {
        mutate();
      }
      return false;
    },
    [this](const TypedValue* value) {
      add(tvAsCVarRef(value));
    });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
  // ... and shrink back if that was incorrect
  if (oldCap) shrinkIfCapacityTooHigh(oldCap);
}

template<bool raw>
ALWAYS_INLINE
void BaseSet::addImpl(int64_t k) {
  if (!raw) {
    mutate();
  }
  SetIntMoveSkipConflict(k, make_tv<KindOfInt64>(k));
}

template<bool raw>
ALWAYS_INLINE
void BaseSet::addImpl(StringData *key) {
  if (!raw) {
    mutate();
  }
  SetStrMoveSkipConflict(key, make_tv<KindOfString>(key));
}

void BaseSet::addRaw(int64_t k) {
  addImpl<true>(k);
}

void BaseSet::addRaw(StringData *key) {
  addImpl<true>(key);
}

void BaseSet::add(int64_t k) {
  addImpl<false>(k);
}

void BaseSet::add(StringData *key) {
  addImpl<false>(key);
}

void BaseSet::addFront(int64_t k) {
  mutate();
  auto h = hash_int64(k);
  auto p = findForInsert(k, h);
  assertx(VanillaDict::isValidIns(p));
  if (VanillaDict::isValidPos(*p)) {
    // When there is a conflict, the addFront() API is supposed to replace
    // the existing element with the new element in place. However since
    // Sets currently only support integer and string elements, there is
    // no way user code can really tell whether the existing element was
    // replaced so for efficiency we do nothing.
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(k, h);
  }
  auto& e = allocElmFront(p);
  e.setIntKey(k, h);
  arrayData()->mutableKeyTypes()->recordInt();
  e.data.m_type = KindOfInt64;
  e.data.m_data.num = k;
  updateNextKI(k);
}

void BaseSet::SetIntMoveSkipConflict(int64_t k, TypedValue v) {
  auto ad = VanillaDict::SetIntMoveSkipConflict(arrayData(), k, v);
  setArrayData(VanillaDict::as(ad));
  m_size = arrayData()->m_size;
}

void BaseSet::SetStrMoveSkipConflict(StringData* k, TypedValue v) {
  // This increments the string's refcount twice, once for
  // the key and once for the value
  auto ad = VanillaDict::SetStrMoveSkipConflict(arrayData(), k, v);
  setArrayData(VanillaDict::as(ad));
  if (m_size != arrayData()->m_size) {
    k->incRefCount();
    m_size = arrayData()->m_size;
  }
}

void BaseSet::addFront(StringData *key) {
  mutate();
  strhash_t h = key->hash();
  auto p = findForInsert(key, h);
  assertx(VanillaDict::isValidIns(p));
  if (VanillaDict::isValidPos(*p)) {
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(key, h);
  }
  auto& e = allocElmFront(p);
  // This increments the string's refcount twice, once for
  // the key and once for the value
  e.setStrKey(key, h);
  arrayData()->mutableKeyTypes()->recordStr(key);
  tvDup(make_tv<KindOfString>(key), e.data);
}

Variant BaseSet::pop() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Set");
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

Variant BaseSet::popFront() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Set");
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

Variant BaseSet::firstValue() {
  if (!m_size) return init_null();
  auto e = firstElm();
  assertx(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant BaseSet::lastValue() {
  if (!m_size) return init_null();
  uint32_t pos = nthElmPos(m_size - 1);
  return tvAsCVarRef(&data()[pos].data);
}

void BaseSet::throwNoMutableIndexAccess() {
  SystemLib::throwInvalidOperationExceptionObject(
    "[] operator cannot be used to modify elements of a Set");
}

bool BaseSet::ToBool(const ObjectData* obj) {
  return static_cast<const BaseSet*>(obj)->toBoolImpl();
}

// This function will create a immutable copy of this Set (if it doesn't
// already exist) and then return it
Object c_Set::getImmutableCopy() {
  if (m_immCopy.isNull()) {
    auto set = req::make<c_ImmSet>();
    set->m_size = m_size;
    set->setArrayData(arrayData());
    m_immCopy = std::move(set);
    arrayData()->incRefCount();
  }
  assertx(!m_immCopy.isNull());
  assertx(data() == static_cast<c_ImmSet*>(m_immCopy.get())->data());
  assertx(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  return VanillaDict::DictEqual(st1->arrayData(), st2->arrayData());
}

BaseSet::~BaseSet() {
  auto const mixed = VanillaDict::as(arrayData());
  // Avoid indirect call, as we know it is a VanillaDict
  if (mixed->decReleaseCheck()) VanillaDict::Release(mixed);
}

void BaseSet::throwBadValueType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer values and string values may be used with Sets");
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, TSet*>::type
BaseSet::Clone(ObjectData* obj) {
  auto thiz = static_cast<TSet*>(obj);
  auto target = req::make<TSet>();
  if (!thiz->m_size) {
    return target.detach();
  }
  thiz->arrayData()->incRefCount();
  target->m_size = thiz->m_size;
  target->setArrayData(thiz->arrayData());
  return target.detach();
}

bool BaseSet::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  auto set = static_cast<BaseSet*>(obj);
  auto const ktv = tvClassToString(*key);
  if (ktv.m_type == KindOfInt64) {
    return set->contains(ktv.m_data.num);
  }
  if (isStringType(ktv.m_type)) {
    return set->contains(ktv.m_data.pstr);
  }
  throwBadValueType();
  return false;
}

bool BaseSet::OffsetContains(ObjectData* obj, const TypedValue* key) {
  auto set = static_cast<BaseSet*>(obj);
  auto const ktv = tvClassToString(*key);
  if (ktv.m_type == KindOfInt64) {
    return set->contains(ktv.m_data.num);
  }
  if (isStringType(ktv.m_type)) {
    return set->contains(ktv.m_data.pstr);
  }
  throwBadValueType();
  return false;
}

void BaseSet::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  auto set = static_cast<BaseSet*>(obj);
  auto const ktv = tvClassToString(*key);
  if (ktv.m_type == KindOfInt64) {
    set->remove(ktv.m_data.num);
    return;
  }
  if (isStringType(ktv.m_type)) {
    set->remove(ktv.m_data.pstr);
    return;
  }
  throwBadValueType();
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_zip(const Variant& iterable) {
  size_t sz;
  ArrayIter iter = getArrayIterHelper(iterable, sz);
  if (m_size && iter) {
    // At present, BaseSets only support int values and string values,
    // so if this BaseSet is non empty and the iterable is non empty
    // the zip operation will always fail
    throwBadValueType();
  }
  return Object(req::make<TSet>());
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_take(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len >= int64_t(m_size)) {
    // We know the result Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto set = req::make<TSet>();
  if (len <= 0) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return Object{std::move(set)};
  }
  size_t sz = size_t(len);
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  set->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t frPos = 0, toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(set)};
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skip(const Variant& n) {
  if (!n.isInteger()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter n must be an integer");
  }
  int64_t len = n.toInt64();
  if (len <= 0) {
    // We know the resulting Set will simply be a copy of this Set,
    // so we can just call Clone() and return early here.
    return Object::attach(TSet::Clone(this));
  }
  auto set = req::make<TSet>();
  if (len >= m_size) {
    // We know the resulting Set will be empty, so we can return
    // early here.
    return Object{std::move(set)};
  }
  size_t sz = size_t(m_size) - size_t(len);
  assertx(sz);
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  set->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  uint32_t frPos = nthElmPos(len);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(frPos)) {
      assertx(frPos + 1 < posLimit());
      ++frPos;
    }
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(set)};
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_slice(const Variant& start, const Variant& len) {
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
  auto set = req::make<TSet>();
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  set->arrayData()->mutableKeyTypes()->copyFrom(
    arrayData()->keyTypes(), /*compact=*/true);
  uint32_t frPos = nthElmPos(skipAmt);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    frPos = skipTombstonesNoBoundsCheck(frPos);
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assertx(toE.hasStrKey());
    }
  }
  return Object{std::move(set)};
}

template<class TVector>
typename std::enable_if<
  std::is_base_of<BaseVector, TVector>::value, Object>::type
BaseSet::php_concat(const Variant& iterable) {
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
    tvDup(data()[i].data, vec->lvalAt(j));
    ++j;
  }
  for (; iter; ++iter) {
    vec->addRaw(iter.second());
  }
  return Object{std::move(vec)};
}

Object BaseSet::getIterator() {
  auto iter = collections::SetIterator::newInstance();
  Native::data<collections::SetIterator>(iter)->setSet(this);
  return iter;
}

///////////////////////////////////////////////////////////////////////////////
// Set

void c_Set::clear() {
  dropImmCopy();
  decRefArr(arrayData());
  setArrayData(CreateDictAsMixed());
  m_size = 0;
}

c_Set* c_Set::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_Set>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// ImmSet

c_ImmSet* c_ImmSet::Clone(ObjectData* obj) {
  return BaseSet::Clone<c_ImmSet>(obj);
}

namespace collections {

/////////////////////////////////////////////////////////////////////////////
// SetIterator

static Variant HHVM_METHOD(SetIterator, current) {
  return Native::data<SetIterator>(this_)->current();
}

static Variant HHVM_METHOD(SetIterator, key) {
  return Native::data<SetIterator>(this_)->key();
}

static bool HHVM_METHOD(SetIterator, valid) {
  return Native::data<SetIterator>(this_)->valid();
}

static void HHVM_METHOD(SetIterator, next) {
  Native::data<SetIterator>(this_)->next();
}

static void HHVM_METHOD(SetIterator, rewind) {
  Native::data<SetIterator>(this_)->rewind();
}

/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::registerNativeSet() {
  HHVM_ME(SetIterator, current);
  HHVM_ME(SetIterator, key);
  HHVM_ME(SetIterator, valid);
  HHVM_ME(SetIterator, next);
  HHVM_ME(SetIterator, rewind);

  Native::registerNativeDataInfo<SetIterator>(Native::NDIFlags::NO_SWEEP);

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Set,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmSet, mn, impl);
  BASE_ME(__construct,   &BaseSet::init);
  BASE_ME(count,         &BaseSet::size);
  BASE_ME(contains,      &BaseSet::php_contains);
  BASE_ME(toKeysArray,   &BaseSet::toKeysArray);
  BASE_ME(toValuesArray, &BaseSet::toValuesArray);
  BASE_ME(getIterator,   &BaseSet::getIterator);
  BASE_ME(firstValue,    &BaseSet::firstValue);
  BASE_ME(lastValue,     &BaseSet::lastValue);
#undef BASE_ME

#define TMPL_STATIC_ME(mn) \
  HHVM_NAMED_STATIC_ME(HH\\Set,    mn, BaseSet::mn<c_Set>); \
  HHVM_NAMED_STATIC_ME(HH\\ImmSet, mn, BaseSet::mn<c_ImmSet>);
  TMPL_STATIC_ME(fromItems);
  TMPL_STATIC_ME(fromKeysOf);
  TMPL_STATIC_ME(fromArray);
#undef TMPL_STATIC_ME

#define TMPL_ME(mn, impl, ret) \
  HHVM_NAMED_ME(HH\\Set,    mn, impl<c_##ret>); \
  HHVM_NAMED_ME(HH\\ImmSet, mn, impl<c_Imm##ret>);
  TMPL_ME(zip,       &BaseSet::php_zip,       Set);
  TMPL_ME(take,      &BaseSet::php_take,      Set);
  TMPL_ME(skip,      &BaseSet::php_skip,      Set);
  TMPL_ME(slice,     &BaseSet::php_slice,     Set);
  TMPL_ME(values,    &BaseSet::php_values,    Vector);
  TMPL_ME(concat,    &BaseSet::php_concat,    Vector);
#undef TMPL_ME

  HHVM_NAMED_ME(HH\\Set,    toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Set,    toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Set,    toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\Set,    toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\ImmSet, toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\ImmSet, toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\ImmSet, toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\ImmSet, toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\ImmSet, toSet,       materialize<c_Set>);

  // We don't need a wrapper function for `clear` as it's not overloaded.
  HHVM_NAMED_ME(HH\\Set, clearNative,         &c_Set::clear);
  HHVM_NAMED_ME(HH\\Set, removeNative,        &c_Set::php_remove);
  HHVM_NAMED_ME(HH\\Set, removeAllNative,     &c_Set::php_removeAll);
  HHVM_NAMED_ME(HH\\Set, addNative,           &c_Set::php_add);
  HHVM_NAMED_ME(HH\\Set, addAllNative,        &c_Set::php_addAll);
  HHVM_NAMED_ME(HH\\Set, addAllKeysOfNative,  &c_Set::php_addAllKeysOf);

  HHVM_NAMED_ME(HH\\Set, reserve,  &c_Set::php_reserve);
  HHVM_NAMED_ME(HH\\Set, toImmSet, &c_Set::getImmutableCopy);

  Native::registerNativePropHandler<CollectionPropHandler>(c_Set::className());
  Native::registerNativePropHandler<CollectionPropHandler>(c_ImmSet::className());

  Native::registerClassExtraDataHandler(c_Set::className(), finish_class<c_Set>);
  Native::registerClassExtraDataHandler(c_ImmSet::className(), finish_class<c_ImmSet>);
}

/////////////////////////////////////////////////////////////////////////////
}}
