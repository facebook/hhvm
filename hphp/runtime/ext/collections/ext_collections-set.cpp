#include "hphp/runtime/ext/collections/ext_collections-set.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

Class* c_Set::s_cls;
Class* c_ImmSet::s_cls;

inline
bool invokeAndCastToBool(const CallCtx& ctx, int argc,
                         const TypedValue* argv) {
  auto ret = Variant::attach(
    g_context->invokeFuncFew(ctx, argc, argv)
  );
  return ret.toBoolean();
}

/////////////////////////////////////////////////////////////////////////////
// BaseSet

void BaseSet::addAllKeysOf(const Cell container) {
  assert(isContainer(container));

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
                mutateAndBump();
                return false;
              },
              [this](Cell k, TypedValue /*v*/) { addRaw(k); },
              [this](ObjectData* coll) {
                if (!m_size && coll->collectionType() == CollectionType::Set) {
                  auto hc = static_cast<HashCollection*>(coll);
                  replaceArray(hc->arrayData());
                  return true;
                }
                if (coll->collectionType() == CollectionType::Pair) {
                  mutateAndBump();
                }
                return false;
              });

  if (UNLIKELY(!ok)) {
    throw_invalid_collection_parameter();
  }
  // ... and shrink back if that was incorrect
  if (oldCap) shrinkIfCapacityTooHigh(oldCap);
}

void BaseSet::addAll(const Variant& t) {
  if (t.isNull()) { return; } // nothing to do

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
      mutateAndBump();
      return false;
    },
    [this](TypedValue v) {
      addRaw(tvToCell(v));
    },
    [this](ObjectData* coll) {
      if (!m_size && coll->collectionType() == CollectionType::Set) {
        auto hc = static_cast<HashCollection*>(coll);
        replaceArray(hc->arrayData());
        return true;
      }
      if (coll->collectionType() == CollectionType::Pair) {
        mutateAndBump();
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
  auto h = hash_int64(k);
  auto p = findForInsert(k, h);
  assert(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
    // When there is a conflict, the add() API is supposed to replace the
    // existing element with the new element in place. However since Sets
    // currently only support integer and string elements, there is no way
    // user code can really tell whether the existing element was replaced
    // so for efficiency we do nothing.
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(k, h);
  }
  auto& e = allocElm(p);
  e.setIntKey(k, h);
  e.data.m_type = KindOfInt64;
  e.data.m_data.num = k;
  updateNextKI(k);
  if (!raw) {
    ++m_version;
  }
}

template<bool raw>
ALWAYS_INLINE
void BaseSet::addImpl(StringData *key) {
  if (!raw) {
    mutate();
  }
  strhash_t h = key->hash();
  auto p = findForInsert(key, h);
  assert(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
    return;
  }
  if (UNLIKELY(isFull())) {
    makeRoom();
    p = findForInsert(key, h);
  }
  auto& e = allocElm(p);
  // This increments the string's refcount twice, once for
  // the key and once for the value
  e.setStrKey(key, h);
  cellDup(make_tv<KindOfString>(key), e.data);
  if (!raw) {
    ++m_version;
  }
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
  assert(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
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
  e.data.m_type = KindOfInt64;
  e.data.m_data.num = k;
  updateNextKI(k);
  ++m_version;
}

void BaseSet::addFront(StringData *key) {
  mutate();
  strhash_t h = key->hash();
  auto p = findForInsert(key, h);
  assert(MixedArray::isValidIns(p));
  if (MixedArray::isValidPos(*p)) {
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
  cellDup(make_tv<KindOfString>(key), e.data);
  ++m_version;
}

Variant BaseSet::pop() {
  if (UNLIKELY(m_size == 0)) {
    SystemLib::throwInvalidOperationExceptionObject("Cannot pop empty Set");
  }
  mutateAndBump();
  auto e = elmLimit() - 1;
  for (;; --e) {
    assert(e >= data());
    if (!isTombstone(e)) break;
  }
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
  mutateAndBump();
  auto e = data();
  for (;; ++e) {
    assert(e != elmLimit());
    if (!isTombstone(e)) break;
  }
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
  assert(e != elmLimit());
  return tvAsCVarRef(&e->data);
}

Variant BaseSet::lastValue() {
  if (!m_size) return init_null();
  // TODO Task# 4281431: If nthElmPos(n) is optimized to
  // walk backward from the end when n > m_size/2, then
  // we could use that here instead of having to use a
  // manual while loop.
  uint32_t pos = posLimit() - 1;
  while (isTombstone(pos)) {
    assert(pos > 0);
    --pos;
  }
  return tvAsCVarRef(&data()[pos].data);
}

void BaseSet::throwNoMutableIndexAccess() {
  SystemLib::throwInvalidOperationExceptionObject(
    "[] operator cannot be used to modify elements of a Set");
}

Array BaseSet::ToArray(const ObjectData* obj) {
  check_collection_cast_to_array();
  return const_cast<BaseSet*>(static_cast<const BaseSet*>(obj))->toArray();
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
    set->m_version = m_version;
    set->m_arr = m_arr;
    m_immCopy = std::move(set);
    arrayData()->incRefCount();
  }
  assert(!m_immCopy.isNull());
  assert(data() == static_cast<c_ImmSet*>(m_immCopy.get())->data());
  assert(arrayData()->hasMultipleRefs());
  return m_immCopy;
}

bool BaseSet::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto st1 = static_cast<const BaseSet*>(obj1);
  auto st2 = static_cast<const BaseSet*>(obj2);
  return MixedArray::DictEqual(st1->arrayData(), st2->arrayData());
}

BaseSet::~BaseSet() {
  auto const mixed = MixedArray::asMixed(arrayData());
  // Avoid indirect call, as we know it is a MixedArray
  if (mixed->decReleaseCheck()) MixedArray::ReleaseDict(mixed);
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
  target->m_arr = thiz->m_arr;
  return target.detach();
}

bool BaseSet::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num);
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr);
  }
  throwBadValueType();
  return false;
}

bool BaseSet::OffsetEmpty(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num) ? !cellToBool(*key) : true;
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr) ? !cellToBool(*key) : true;
  }
  throwBadValueType();
  return true;
}

bool BaseSet::OffsetContains(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    return set->contains(key->m_data.num);
  }
  if (isStringType(key->m_type)) {
    return set->contains(key->m_data.pstr);
  }
  throwBadValueType();
  return false;
}

void BaseSet::OffsetUnset(ObjectData* obj, const TypedValue* key) {
  assert(key->m_type != KindOfRef);
  auto set = static_cast<BaseSet*>(obj);
  if (key->m_type == KindOfInt64) {
    set->remove(key->m_data.num);
    return;
  }
  if (isStringType(key->m_type)) {
    set->remove(key->m_data.pstr);
    return;
  }
  throwBadValueType();
}

template<typename TSet, bool useKey>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_map(const Variant& callback) {
  VMRegGuard _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object{std::move(set)};
  assert(posLimit() != 0);
  assert(set->arrayData() == staticEmptyDictArrayAsMixed());
  auto oldCap = set->cap();
  set->reserve(posLimit()); // presume minimum collisions ...
  assert(set->canMutateBuffer());
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto e = iter_elm(pos);
    int32_t pVer = m_version;
    if (useKey) {
      argv[0] = e->data;
    }
    argv[argc-1] = e->data;
    auto cbRet = Variant::attach(
      g_context->invokeFuncFew(ctx, argc, argv)
    );
    if (UNLIKELY(m_version != pVer)) throw_collection_modified();
    set->addRaw(*cbRet.asTypedValue());
  }
  // ... and shrink back if that was incorrect
  set->shrinkIfCapacityTooHigh(oldCap);
  return Object{std::move(set)};
}

template<typename TSet, bool useKey>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_filter(const Variant& callback) {
  VMRegGuard _;
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  // we don't reserve(), because we don't know how selective callback will be
  set->mutate();
  int32_t version = m_version;
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    auto e = iter_elm(pos);
    if (useKey) {
      argv[0] = e->data;
    }
    argv[argc-1] = e->data;
    bool b = invokeAndCastToBool(ctx, argc, argv);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (!b) continue;
    e = iter_elm(pos);
    if (e->hasIntKey()) {
      set->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      set->addRaw(e->data.m_data.pstr);
    }
  }
  return Object(std::move(set));
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

template<bool useKey>
Object BaseSet::php_retain(const Variant& callback) {
  CallCtx ctx;
  vm_decode_function(callback, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto size = m_size;
  if (!size) { return Object{this}; }
  constexpr int64_t argc = useKey ? 2 : 1;
  TypedValue argv[argc];
  for (ssize_t pos = iter_begin(); iter_valid(pos); pos = iter_next(pos)) {
    int32_t version = m_version;
    auto e = iter_elm(pos);
    if (useKey) {
      argv[0] = e->data;
    }
    argv[argc-1] = e->data;
    bool b = invokeAndCastToBool(ctx, argc, argv);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
    if (b) { continue; }
    mutateAndBump();
    version = m_version;
    e = iter_elm(pos);
    auto h = e->hash();
    auto pp = e->hasIntKey() ? findForRemove(e->ikey, h) :
              findForRemove(e->skey, h);
    eraseNoCompact(pp);
    if (UNLIKELY(version != m_version)) {
      throw_collection_modified();
    }
  }
  assert(m_size <= size);
  compactOrShrinkIfDensityTooLow();
  return Object{this};
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
      assert(toE.hasStrKey());
    }
  }
  return Object{std::move(set)};
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_takeWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  set->mutate();
  int32_t version UNUSED;
  if (std::is_same<c_Set, TSet>::value) {
    version = m_version;
  }
  uint32_t used = posLimit();
  for (uint32_t i = 0; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm* e = &data()[i];
    bool b = invokeAndCastToBool(ctx, 1, &e->data);
    if (std::is_same<c_Set, TSet>::value) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
    e = &data()[i];
    if (e->hasIntKey()) {
      set->addRaw(e->data.m_data.num);
    } else {
      assert(e->hasStrKey());
      set->addRaw(e->data.m_data.pstr);
    }
  }
  return Object(std::move(set));
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
  assert(sz);
  set->reserve(sz);
  set->setSize(sz);
  set->setPosLimit(sz);
  uint32_t frPos = nthElmPos(len);
  auto table = set->hashTab();
  auto mask = set->tableMask();
  for (uint32_t toPos = 0; toPos < sz; ++toPos, ++frPos) {
    while (isTombstone(frPos)) {
      assert(frPos + 1 < posLimit());
      ++frPos;
    }
    auto& toE = set->data()[toPos];
    dupElm(data()[frPos], toE);
    *findForNewInsert(table, mask, toE.probe()) = toPos;
    if (toE.hasIntKey()) {
      set->updateNextKI(toE.ikey);
    } else {
      assert(toE.hasStrKey());
    }
  }
  return Object{std::move(set)};
}

template<class TSet>
typename std::enable_if<
  std::is_base_of<BaseSet, TSet>::value, Object>::type
BaseSet::php_skipWhile(const Variant& fn) {
  CallCtx ctx;
  vm_decode_function(fn, nullptr, false, ctx);
  if (!ctx.func) {
    SystemLib::throwInvalidArgumentExceptionObject(
               "Parameter must be a valid callback");
  }
  auto set = req::make<TSet>();
  if (!m_size) return Object(std::move(set));
  // we don't reserve(), because we don't know how selective fn will be
  set->mutate();
  int32_t version UNUSED;
  if (std::is_same<c_Set, TSet>::value) {
    version = m_version;
  }
  uint32_t used = posLimit();
  uint32_t i = 0;
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& e = data()[i];
    bool b = invokeAndCastToBool(ctx, 1, &e.data);
    if (std::is_same<c_Set, TSet>::value) {
      if (UNLIKELY(version != m_version)) {
        throw_collection_modified();
      }
    }
    if (!b) break;
  }
  for (; i < used; ++i) {
    if (isTombstone(i)) continue;
    Elm& e = data()[i];
    if (e.hasIntKey()) {
      set->addRaw(e.data.m_data.num);
    } else {
      assert(e.hasStrKey());
      set->addRaw(e.data.m_data.pstr);
    }
  }
  return Object(std::move(set));
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
      assert(toE.hasStrKey());
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
  assert(vec->canMutateBuffer());
  vec->setSize(sz);

  uint32_t used = posLimit();
  for (uint32_t i = 0, j = 0; i < used; ++i) {
    if (isTombstone(i)) {
      continue;
    }
    cellDup(data()[i].data, vec->data()[j]);
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
  ++m_version;
  dropImmCopy();
  decRefArr(arrayData());
  m_arr = staticEmptyDictArrayAsMixed();
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

const StaticString
  s_HH_Set("HH\\Set"),
  s_HH_ImmSet("HH\\ImmSet"),
  s_SetIterator("SetIterator");

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

void CollectionsExtension::initSet() {
  HHVM_ME(SetIterator, current);
  HHVM_ME(SetIterator, key);
  HHVM_ME(SetIterator, valid);
  HHVM_ME(SetIterator, next);
  HHVM_ME(SetIterator, rewind);

  Native::registerNativeDataInfo<SetIterator>(
    s_SetIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

#define BASE_ME(mn, impl) \
  HHVM_NAMED_ME(HH\\Set,    mn, impl); \
  HHVM_NAMED_ME(HH\\ImmSet, mn, impl);
  BASE_ME(__construct,   &BaseSet::init);
  BASE_ME(count,         &BaseSet::size);
  BASE_ME(contains,      &BaseSet::php_contains);
  BASE_ME(toArray,       &BaseSet::toArray);
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
  TMPL_ME(takeWhile, &BaseSet::php_takeWhile, Set);
  TMPL_ME(skip,      &BaseSet::php_skip,      Set);
  TMPL_ME(skipWhile, &BaseSet::php_skipWhile, Set);
  TMPL_ME(slice,     &BaseSet::php_slice,     Set);
  TMPL_ME(values,    &BaseSet::php_values,    Vector);
  TMPL_ME(concat,    &BaseSet::php_concat,    Vector);
#undef TMPL_ME

  auto const m     = &BaseSet::php_map<c_Set, false>;
  auto const mk    = &BaseSet::php_map<c_Set, true>;
  auto const immm  = &BaseSet::php_map<c_ImmSet, false>;
  auto const immmk = &BaseSet::php_map<c_ImmSet, true>;
  HHVM_NAMED_ME(HH\\Set,    map,        m);
  HHVM_NAMED_ME(HH\\Set,    mapWithKey, mk);
  HHVM_NAMED_ME(HH\\ImmSet, map,        immm);
  HHVM_NAMED_ME(HH\\ImmSet, mapWithKey, immmk);

  auto const f     = &BaseSet::php_filter<c_Set, false>;
  auto const fk    = &BaseSet::php_filter<c_Set, true>;
  auto const immf  = &BaseSet::php_filter<c_ImmSet, false>;
  auto const immfk = &BaseSet::php_filter<c_ImmSet, true>;
  HHVM_NAMED_ME(HH\\Set,    filter,        f);
  HHVM_NAMED_ME(HH\\Set,    filterWithKey, fk);
  HHVM_NAMED_ME(HH\\ImmSet, filter,        immf);
  HHVM_NAMED_ME(HH\\ImmSet, filterWithKey, immfk);

  HHVM_NAMED_ME(HH\\Set,    toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Set,    toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Set,    toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\Set,    toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\ImmSet, toVector,    materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\ImmSet, toImmVector, materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\ImmSet, toMap,       materialize<c_Map>);
  HHVM_NAMED_ME(HH\\ImmSet, toImmMap,    materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\ImmSet, toSet,       materialize<c_Set>);

  HHVM_NAMED_ME(HH\\Set, add,            &c_Set::php_add);
  HHVM_NAMED_ME(HH\\Set, addAll,         &c_Set::php_addAll);
  HHVM_NAMED_ME(HH\\Set, addAllKeysOf,   &c_Set::php_addAllKeysOf);
  HHVM_NAMED_ME(HH\\Set, clear,          &c_Set::php_clear);
  HHVM_NAMED_ME(HH\\Set, remove,         &c_Set::php_remove);
  HHVM_NAMED_ME(HH\\Set, removeAll,      &c_Set::php_removeAll);
  HHVM_NAMED_ME(HH\\Set, reserve,        &c_Set::php_reserve);
  HHVM_NAMED_ME(HH\\Set, retain,         &c_Set::php_retain<false>);
  HHVM_NAMED_ME(HH\\Set, retainWithKey,  &c_Set::php_retain<true>);
  HHVM_NAMED_ME(HH\\Set, toImmSet,       &c_Set::getImmutableCopy);

  loadSystemlib("collections-set");

  c_Set::s_cls = Unit::lookupClass(s_HH_Set.get());
  assertx(c_Set::s_cls);
  finishClass<c_Set>();

  c_ImmSet::s_cls = Unit::lookupClass(s_HH_ImmSet.get());
  assertx(c_ImmSet::s_cls);
  finishClass<c_ImmSet>();
}

/////////////////////////////////////////////////////////////////////////////
}}
