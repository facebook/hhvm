/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/array-iterator.h"

#include <algorithm>

#include <folly/Likely.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/array-iterator-defs.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

//////////////////////////////////////////////////////////////////////

ArrayIter::ArrayIter(const ArrayData* data) {
  arrInit(data);
}

ArrayIter::ArrayIter(const Array& array) {
  arrInit(array.get());
}

ArrayIter::ArrayIter(ObjectData* obj) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(ObjectData* obj, NoInc) {
  objInit<false>(obj);
}

ArrayIter::ArrayIter(const Object& obj) {
  objInit<true>(obj.get());
}

ArrayIter::ArrayIter(const Cell c) {
  cellInit(c);
}

ArrayIter::ArrayIter(const Variant& v) {
  cellInit(*v.asCell());
}

ArrayIter::ArrayIter(const ArrayIter& iter) {
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_version = iter.m_version;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assert(obj);
    obj->incRefCount();
  }
}

void ArrayIter::arrInit(const ArrayData* arr) {
  setArrayData(arr);
  if (arr) {
    arr->incRefCount();
    m_pos = arr->iter_begin();
  }
}

void ArrayIter::VectorInit(ArrayIter* iter, ObjectData* obj) {
  auto vec = static_cast<c_Vector*>(obj);
  iter->m_version = vec->getVersion();
  iter->m_pos = 0;
}

void ArrayIter::MapInit(ArrayIter* iter, ObjectData* obj) {
  auto mp = static_cast<c_Map*>(obj);
  iter->m_version = mp->getVersion();
  iter->m_pos = mp->iter_begin();
}

void ArrayIter::ImmMapInit(ArrayIter* iter, ObjectData* obj) {
  auto smp = static_cast<c_ImmMap*>(obj);
  iter->m_version = smp->getVersion();
  iter->m_pos = smp->iter_begin();
}

void ArrayIter::SetInit(ArrayIter* iter, ObjectData* obj) {
  auto st = static_cast<c_Set*>(obj);
  iter->m_version = st->getVersion();
  iter->m_pos = st->iter_begin();
}

void ArrayIter::PairInit(ArrayIter* iter, ObjectData* /*obj*/) {
  iter->m_pos = 0;
}

void ArrayIter::ImmVectorInit(ArrayIter* iter, ObjectData* obj) {
  auto vec = static_cast<c_ImmVector*>(obj);
  iter->m_version = vec->getVersion();
  iter->m_pos = 0;
}
void ArrayIter::ImmSetInit(ArrayIter* iter, ObjectData* obj) {
  auto st = static_cast<c_ImmSet*>(obj);
  iter->m_version = st->getVersion();
  iter->m_pos = st->iter_begin();
}

IterNextIndex ArrayIter::getNextHelperIdx(ObjectData* obj) {
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
#define X(type) case CollectionType::type: return IterNextIndex::type;
COLLECTIONS_ALL_TYPES(X)
#undef X
    }
    not_reached();
  } else {
    return IterNextIndex::Object;
  }
}

void ArrayIter::IteratorObjInit(ArrayIter* iter, ObjectData* obj) {
  assert(obj->instanceof(SystemLib::s_IteratorClass));
  try {
    obj->o_invoke_few_args(s_rewind, 0);
  } catch (...) {
    // Regardless of whether the incRef template parameter is true or
    // false, at this point this ArrayIter "owns" a reference to the
    // object and is responsible for decreffing the object when the
    // ArrayIter's lifetime ends. Normally ArrayIter's destructor takes
    // care of this, but the destructor will not get invoked if an
    // exception is thrown before the constructor finishes so we have
    // to manually handle decreffing the object here.
    iter->m_data = nullptr;
    if (debug) iter->m_itype = TypeUndefined;
    decRefObj(obj);
    throw;
  }
}

constexpr unsigned ctype_index(CollectionType t) {
  return unsigned(t) - unsigned(CollectionType::Vector);
}

static_assert(ctype_index(CollectionType::Vector) == 0, "");
static_assert(ctype_index(CollectionType::Map) == 1, "");
static_assert(ctype_index(CollectionType::Set) == 2, "");
static_assert(ctype_index(CollectionType::Pair) == 3, "");
static_assert(ctype_index(CollectionType::ImmVector) == 4, "");
static_assert(ctype_index(CollectionType::ImmMap) == 5, "");
static_assert(ctype_index(CollectionType::ImmSet) == 6, "");
const unsigned MaxCollectionTypes = 7;

const ArrayIter::InitFuncPtr
ArrayIter::initFuncTable[MaxCollectionTypes + 1] = {
  &ArrayIter::VectorInit,
  &ArrayIter::MapInit,
  &ArrayIter::SetInit,
  &ArrayIter::PairInit,
  &ArrayIter::ImmVectorInit,
  &ArrayIter::ImmMapInit,
  &ArrayIter::ImmSetInit,
  &ArrayIter::IteratorObjInit,
};

template <bool incRef>
void ArrayIter::objInit(ObjectData* obj) {
  assert(obj);
  setObject(obj);
  if (incRef) obj->incRefCount();
  auto i = obj->isCollection() ? ctype_index(obj->collectionType()) :
           MaxCollectionTypes;
  initFuncTable[i](this, obj);
}

void ArrayIter::cellInit(const Cell c) {
  assert(cellIsPlausible(c));
  if (LIKELY(isArrayLikeType(c.m_type))) {
    arrInit(c.m_data.parr);
  } else if (LIKELY(c.m_type == KindOfObject)) {
    objInit<true>(c.m_data.pobj);
  } else {
    arrInit(nullptr);
  }
}

void ArrayIter::rewind() {
  if (LIKELY(hasArrayData())) {
    if (auto* data = getArrayData())
      m_pos = data->iter_begin();
  } else {
    assert(hasCollection());
    auto* obj = getObject();
    auto i = obj->isCollection() ? ctype_index(obj->collectionType()) :
      MaxCollectionTypes;
    initFuncTable[i](this, obj);
  }
}

void ArrayIter::destruct() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (debug) m_itype = TypeUndefined;
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return;
  }
  ObjectData* obj = getObject();
  if (debug) m_itype = TypeUndefined;
  assert(obj);
  decRefObj(obj);
}

ArrayIter& ArrayIter::operator=(const ArrayIter& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_version = iter.m_version;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assert(obj);
    obj->incRefCount();
  }
  return *this;
}

ArrayIter& ArrayIter::operator=(ArrayIter&& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_version = iter.m_version;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  iter.m_data = nullptr;
  return *this;
}

bool ArrayIter::endHelper() const  {
  auto obj = getObject();
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
      case CollectionType::Vector:
      case CollectionType::ImmVector:
        return m_pos >= static_cast<BaseVector*>(obj)->size();
      case CollectionType::Map:
      case CollectionType::ImmMap:
        return !static_cast<BaseMap*>(obj)->iter_valid(m_pos);
      case CollectionType::Set:
      case CollectionType::ImmSet:
        return !static_cast<BaseSet*>(obj)->iter_valid(m_pos);
      case CollectionType::Pair:
        return m_pos >= static_cast<c_Pair*>(obj)->size();
    }
  } else {
    return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
  }
  not_reached();
}

void ArrayIter::nextHelper() {
  auto obj = getObject();
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
      case CollectionType::Pair:
      case CollectionType::ImmVector:
      case CollectionType::Vector:
        m_pos++;
        return;
      case CollectionType::Map:
      case CollectionType::ImmMap: {
        auto map = static_cast<BaseMap*>(obj);
        if (UNLIKELY(m_version != map->getVersion())) {
          throw_collection_modified();
        }
        m_pos = map->iter_next(m_pos);
        return;
      }
      case CollectionType::Set: {
        auto set = static_cast<BaseSet*>(obj);
        if (UNLIKELY(m_version != set->getVersion())) {
          throw_collection_modified();
        }
        m_pos = set->iter_next(m_pos);
        return;
      }
      case CollectionType::ImmSet: {
        auto set = static_cast<c_ImmSet*>(obj);
        assert(m_version == set->getVersion());
        m_pos = set->iter_next(m_pos);
        return;
      }
    }
  } else {
    obj->o_invoke_few_args(s_next, 0);
  }
}

Variant ArrayIter::firstHelper() {
  auto obj = getObject();
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
      case CollectionType::Vector:
      case CollectionType::Pair:
      case CollectionType::ImmVector:
        return m_pos;
      case CollectionType::Map:
      case CollectionType::ImmMap: {
        auto map = static_cast<BaseMap*>(obj);
        if (UNLIKELY(m_version != map->getVersion())) {
          throw_collection_modified();
        }
        return map->iter_key(m_pos);
      }
      case CollectionType::Set: {
        auto set = static_cast<BaseSet*>(obj);
        if (UNLIKELY(m_version != set->getVersion())) {
          throw_collection_modified();
        }
        return set->iter_key(m_pos);
      }
      case CollectionType::ImmSet: {
        auto set = static_cast<c_ImmSet*>(obj);
        if (UNLIKELY(m_version != set->getVersion())) {
          throw_collection_modified();
        }
        return set->iter_key(m_pos);
      }
    }
  }
  return obj->o_invoke_few_args(s_key, 0);
}

Variant ArrayIter::second() {
  if (LIKELY(hasArrayData())) {
    const ArrayData* ad = getArrayData();
    assert(ad);
    assert(m_pos != ad->iter_end());
    return ad->getValue(m_pos);
  }
  auto obj = getObject();
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
      case CollectionType::Vector: {
        auto vec = static_cast<BaseVector*>(obj);
        if (UNLIKELY(m_version != vec->getVersion())) {
          throw_collection_modified();
        }
        return tvAsCVarRef(vec->at(m_pos));
      }
      case CollectionType::Map:
      case CollectionType::ImmMap: {
        auto map = static_cast<BaseMap*>(obj);
        if (UNLIKELY(m_version != map->getVersion())) {
          throw_collection_modified();
        }
        return tvAsCVarRef(map->iter_value(m_pos));
      }
      case CollectionType::Set: {
        auto set = static_cast<BaseSet*>(obj);
        if (UNLIKELY(m_version != set->getVersion())) {
          throw_collection_modified();
        }
        return tvAsCVarRef(set->iter_value(m_pos));
      }
      case CollectionType::Pair: {
        auto pair = static_cast<c_Pair*>(obj);
        return tvAsCVarRef(pair->at(m_pos));
      }
      case CollectionType::ImmVector: {
        auto fvec = static_cast<c_ImmVector*>(obj);
        if (UNLIKELY(m_version != fvec->getVersion())) {
          throw_collection_modified();
        }
        return tvAsCVarRef(fvec->at(m_pos));
      }
      case CollectionType::ImmSet: {
        auto set = static_cast<c_ImmSet*>(obj);
        assert(m_version == set->getVersion());
        return tvAsCVarRef(set->iter_value(m_pos));
      }
    }
  }
  return obj->o_invoke_few_args(s_current, 0);
}

member_rval ArrayIter::secondRval() const {
  if (!hasArrayData()) {
    raise_fatal_error("taking reference on iterator objects");
  }
  assert(hasArrayData());
  const ArrayData* ad = getArrayData();
  assert(ad);
  assert(m_pos != ad->iter_end());
  return ad->rvalPos(m_pos);
}

member_rval ArrayIter::secondRvalPlus() {
  if (LIKELY(hasArrayData())) {
    const ArrayData* ad = getArrayData();
    assert(ad);
    assert(m_pos != ad->iter_end());
    return ad->rvalPos(m_pos);
  }
  auto obj = getObject();
  if (obj->isCollection()) {
    switch (obj->collectionType()) {
      case CollectionType::Vector: {
        auto vec = static_cast<BaseVector*>(obj);
        if (UNLIKELY(m_version != vec->getVersion())) {
          throw_collection_modified();
        }
        return member_rval { obj, vec->at(m_pos) };
      }
      case CollectionType::Map:
      case CollectionType::ImmMap: {
        auto map = static_cast<BaseMap*>(obj);
        if (UNLIKELY(m_version != map->getVersion())) {
          throw_collection_modified();
        }
        return member_rval { obj, map->iter_value(m_pos) };
      }
      case CollectionType::Set: {
        auto set = static_cast<BaseSet*>(obj);
        if (UNLIKELY(m_version != set->getVersion())) {
          throw_collection_modified();
        }
        return member_rval { obj, set->iter_value(m_pos) };
      }
      case CollectionType::Pair: {
        auto pair = static_cast<c_Pair*>(obj);
        return member_rval { obj, pair->at(m_pos) };
      }
      case CollectionType::ImmVector: {
        auto fvec = static_cast<c_ImmVector*>(obj);
        if (UNLIKELY(m_version != fvec->getVersion())) {
          throw_collection_modified();
        }
        return member_rval { obj, fvec->at(m_pos) };
      }
      case CollectionType::ImmSet: {
        auto set = static_cast<c_ImmSet*>(obj);
        assert(m_version == set->getVersion());
        return member_rval { obj, set->iter_value(m_pos) };
      }
    }
  }
  throw_param_is_not_container();
  not_reached();
}

//
// Collection iterator specialized functions.
//

template<class Tuplish>
ArrayIter::ArrayIter(Tuplish* coll, Fixed)
    : m_pos(0), m_itype(ArrayIter::TypeIterator) {
  assert(coll);
  setObject(coll);
  // TODO Task #4204598: In theory, we might be able to squeeze out a win
  // here by not checking the version for immutable collections, but we'd
  // to make sure all iteration implementations are consistent about this.
  m_version = coll->getVersion();
}

template<class Vectorish>
ArrayIter::ArrayIter(Vectorish* coll, Versionable)
    : m_pos(0), m_itype(ArrayIter::TypeIterator) {
  assert(coll && coll->size() > 0);
  setObject(coll);
  m_version = coll->getVersion();
}

template<class Mappish>
ArrayIter::ArrayIter(Mappish* coll, VersionableSparse)
    : m_itype(ArrayIter::TypeIterator) {
  assert(coll && coll->size() > 0);
  setObject(coll);
  m_version = coll->getVersion();
  m_pos = coll->iter_begin();
}

template<class Tuplish>
ALWAYS_INLINE
bool ArrayIter::iterNext(Fixed) {
  return ++m_pos < static_cast<Tuplish*>(getObject())->size();
}

template<class Vectorish>
ALWAYS_INLINE
bool ArrayIter::iterNext(Versionable) {
  Vectorish* vec = static_cast<Vectorish*>(getObject());
  if (UNLIKELY(m_version != vec->getVersion())) {
    throw_collection_modified();
  }
  return ++m_pos < vec->size();
}

template<class Mappish>
ALWAYS_INLINE
bool ArrayIter::iterNext(VersionableSparse) {
  Mappish* coll = static_cast<Mappish*>(getObject());
  if (UNLIKELY(m_version != coll->getVersion())) {
    throw_collection_modified();
  }
  m_pos = coll->iter_next(m_pos);
  return coll->iter_valid(m_pos);
}

template<class Tuplish>
ALWAYS_INLINE
Variant ArrayIter::iterKey(Fixed) {
  return m_pos;
}

template<class Vectorish>
ALWAYS_INLINE
Variant ArrayIter::iterKey(Versionable) {
  return m_pos;
}

template<class Mappish>
ALWAYS_INLINE
Variant ArrayIter::iterKey(VersionableSparse) {
  return static_cast<Mappish*>(getObject())->iter_key(m_pos);
}

template<class Tuplish>
ALWAYS_INLINE
Variant ArrayIter::iterValue(Fixed) {
  return tvAsCVarRef(static_cast<Tuplish*>(getObject())->get(m_pos));
}

template<class Vectorish>
ALWAYS_INLINE
Variant ArrayIter::iterValue(Versionable) {
  return tvAsCVarRef(static_cast<Vectorish*>(getObject())->get(m_pos));
}

template<class Mappish>
ALWAYS_INLINE
Variant ArrayIter::iterValue(VersionableSparse) {
  return tvAsCVarRef(static_cast<Mappish*>(getObject())->iter_value(m_pos));
}

//////////////////////////////////////////////////////////////////////

void MIterTable::Create(void* storage) {
  new (storage) MIterTable();
}

void MIterTable::clear() {
  if (TlsWrapper::isNull()) return;
  auto t = TlsWrapper::getNoCheck();
  t->ents.fill({nullptr, nullptr});
  if (!t->extras.empty()) {
    t->extras.release_if([](const MIterTable::Ent& /*e*/) { return true; });
  }
}

namespace {

// Handle the cases where we didn't have enough preallocated Ents in
// MIterTable, and we need to allocate from `extras'.
NEVER_INLINE
MIterTable::Ent* find_empty_strong_iter_slower(MIterTable& table) {
  return table.extras.find_unpopulated();
}

// Handle finding an empty strong iterator slot when the first slot
// was already in use.
NEVER_INLINE
MIterTable::Ent* find_empty_strong_iter_slow(MIterTable& table) {
#define X(i) \
  if (LIKELY(!table.ents[i].array)) return &table.ents[i];
X(1);
X(2);
X(3);
X(4);
X(5);
X(6);
  static_assert(MIterTable::ents_size == 7, "");
#undef X
  return find_empty_strong_iter_slower(table);
}

// Find a strong iterator slot that is empty.  Almost always the first
// one will be empty, so that path is inlined---everything else
// delegates to slow.
ALWAYS_INLINE
MIterTable::Ent* find_empty_strong_iter() {
  auto& table = *MIterTable::TlsWrapper::getCheck();
  if (LIKELY(!table.ents[0].array)) {
    return &table.ents[0];
  }
  return find_empty_strong_iter_slow(table);
}

void newMArrayIter(MArrayIter* marr, ArrayData* ad) {
  assert(!marr->getContainer());
  auto const slot = find_empty_strong_iter();
  assert(!slot->array);
  slot->iter = marr;
  slot->array = ad;
  marr->setContainer(ad);
  assert(strong_iterators_exist());
}

template<class Cond>
void free_strong_iterator_impl(Cond cond) {
  assert(strong_iterators_exist());

  // We need to maintain the invariant that if there are any strong
  // iterators bound to arrays, one of the bindings is in slot zero.
  // This pvalid will point to something we can move into the first
  // slot if alreadyValid is false.  If when we're done alreadyValid
  // is false, and pvalid is also nullptr, it means this function
  // freed the last strong iterator.
  MIterTable::Ent* pvalid = nullptr;
  bool alreadyValid = true;  // because strong_iterators_exist()

  auto rm = [&] (MIterTable::Ent& ent) {
    if (cond(ent)) {
      ent.iter->setContainer(nullptr);
      ent.array = nullptr;
      ent.iter = nullptr;
    } else if (!alreadyValid && ent.array) {
      pvalid = &ent;
    }
  };

  auto& table = miter_table();
  if (cond(table.ents[0])) {
    table.ents[0].iter->setContainer(nullptr);
    table.ents[0].array = nullptr;
    table.ents[0].iter = nullptr;
    alreadyValid = false;
  }
  rm(table.ents[1]);
  rm(table.ents[2]);
  rm(table.ents[3]);
  rm(table.ents[4]);
  rm(table.ents[5]);
  rm(table.ents[6]);
  static_assert(MIterTable::ents_size == 7, "");

  if (UNLIKELY(pvalid != nullptr)) {
    std::swap(*pvalid, table.ents[0]);
    alreadyValid = true;
  }
  if (LIKELY(table.extras.empty())) return;

  table.extras.release_if([&] (const MIterTable::Ent& e) {
    if (cond(e)) {
      e.iter->setContainer(nullptr);
      return true;
    }
    return false;
  });

  // If we didn't manage to keep something in the first non-extra
  // slot, scan extras again to swap something over.
  if (LIKELY(alreadyValid)) return;
  if (!table.extras.empty()) {
    table.extras.visit_to_remove(
      [&] (const MIterTable::Ent& ent) {
        table.ents[0] = ent;
      }
    );
  }
}

void freeMArrayIter(MArrayIter* marr) {
  assert(strong_iterators_exist());
  free_strong_iterator_impl(
    [marr] (const MIterTable::Ent& e) {
      return e.iter == marr;
    }
  );
}

}

void free_strong_iterators(ArrayData* ad) {
  free_strong_iterator_impl([ad] (const MIterTable::Ent& e) {
    return e.array == ad;
  });
}

bool has_strong_iterator(ArrayData* ad) {
  if (LIKELY(!strong_iterators_exist())) return false;
  bool found = false;
  for_each_strong_iterator([&] (MIterTable::Ent& ent) {
    if (ent.array == ad) found = true;
  });
  return found;
}

//////////////////////////////////////////////////////////////////////

MArrayIter::MArrayIter(RefData* ref)
  : m_pos(0)
  , m_container(nullptr)
  , m_resetFlag(true)
{
  ref->incRefCount();
  setRef(ref);
  assert(hasRef());
  escalateCheck();
  auto const data = cowCheck();
  assert(data);
  data->reset();
  data->next();
  newMArrayIter(this, data);
  assert(getContainer() == data);
}

MArrayIter::MArrayIter(ArrayData* data)
  : m_pos(0)
  , m_container(nullptr)
  , m_resetFlag(true)
{
  // this constructor is only used for object iteration, so we always get in a
  // mixed array with refcount 1
  assert(data);
  assert(data->isMixed() && data->hasExactlyOneRef());
  setAd(data);
  data->reset();
  data->next();
  newMArrayIter(this, data);
  assert(getContainer() == data);
}

MArrayIter::~MArrayIter() {
  auto const container = getContainer();
  if (container) {
    freeMArrayIter(this);
    assert(getContainer() == nullptr);
  }
  if (hasRef()) {
    decRefRef(getRef());
  } else if (hasAd()) {
    decRefArr(getAd());
  }
}

bool MArrayIter::end() const {
  return !const_cast<MArrayIter*>(this)->prepare();
}

bool MArrayIter::advance() {
  if (hasAd()) {
    return getAd()->advanceMArrayIter(*this);
  }

  ArrayData* data = getData();
  ArrayData* container = getContainer();
  if (!data) {
    if (container) {
      freeMArrayIter(this);
    }
    setResetFlag(false);
    return false;
  }
  if (container == data) {
    return cowCheck()->advanceMArrayIter(*this);
  }
  data = reregister();
  assert(data && data == getContainer());
  assert(!getResetFlag());
  if (!data->validMArrayIter(*this)) return false;
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  data->next();
  return true;
}

bool MArrayIter::prepare() {
  if (hasAd()) {
    return getAd()->validMArrayIter(*this);
  }

  ArrayData* data = getData();
  ArrayData* container = getContainer();
  if (!data) {
    if (container) {
      freeMArrayIter(this);
    }
    setResetFlag(false);
    return false;
  }
  if (container != data) {
    data = reregister();
  }
  return data->validMArrayIter(*this);
}

void MArrayIter::escalateCheck() {
  assert(hasRef());
  auto const data = getData();
  if (!data) return;
  auto const esc = data->escalate();
  if (data != esc) {
    cellMove(make_array_like_tv(esc), *getRef()->tv());
  }
}

ArrayData* MArrayIter::cowCheck() {
  assert(hasRef());
  auto data = getData();
  if (!data) return nullptr;
  if (!data->cowCheck() || data->noCopyOnWrite()) return data;
  // This copy should not interrupt strong iteration. If there are nested
  // strong iterators over the same array, we need to update all of them,
  // so we have to move_strong_iterators, we can't just setContainer. We
  // have to check if strong_iterators_exist because we may be in the process
  // of creating the first one.
  auto copy = data->copy();
  if (strong_iterators_exist()) move_strong_iterators(copy, data);
  cellMove(make_array_like_tv(copy), *getRef()->tv());
  return copy;
}

ArrayData* MArrayIter::reregister() {
  assert(hasRef());
  ArrayData* container = getContainer();
  assert(getData() != nullptr && container != getData());
  if (container != nullptr) {
    freeMArrayIter(this);
  }
  assert(getContainer() == nullptr);
  setResetFlag(false);
  escalateCheck();
  ArrayData* data = cowCheck();
  m_pos = data->getPosition();
  newMArrayIter(this, data);
  return data;
}

//////////////////////////////////////////////////////////////////////

CufIter::~CufIter() {
  if (m_obj_or_cls && !(uintptr_t(m_obj_or_cls) & 1)) {
    decRefObj(m_obj_or_cls);
  }
  if (m_name) decRefStr(m_name);
}

bool Iter::init(TypedValue* c1) {
  assert(c1->m_type != KindOfRef);
  bool hasElems = true;
  if (isArrayLikeType(c1->m_type)) {
    if (!c1->m_data.parr->empty()) {
      (void) new (&arr()) ArrayIter(c1->m_data.parr);
      arr().setIterType(ArrayIter::TypeArray);
    } else {
      hasElems = false;
    }
  } else if (c1->m_type == KindOfObject) {
    bool isIterator;
    if (c1->m_data.pobj->isCollection()) {
      isIterator = true;
      (void) new (&arr()) ArrayIter(c1->m_data.pobj);
    } else {
      Object obj = c1->m_data.pobj->iterableObject(isIterator);
      if (isIterator) {
        (void) new (&arr()) ArrayIter(obj.detach(), ArrayIter::noInc);
      } else {
        Class* ctx = arGetContextClass(vmfp());
        auto ctxStr = ctx ? ctx->nameStr() : StrNR();
        Array iterArray(obj->o_toIterArray(ctxStr, ObjectData::EraseRefs));
        ArrayData* ad = iterArray.get();
        (void) new (&arr()) ArrayIter(ad);
      }
    }
    try {
      if (arr().end()) {
        // Iterator was empty; call the destructor on the iterator we
        // just constructed and branch to done case
        arr().~ArrayIter();
        hasElems = false;
      } else {
        arr().setIterType(
          isIterator ? ArrayIter::TypeIterator : ArrayIter::TypeArray);
      }
    } catch (...) {
      arr().~ArrayIter();
      throw;
    }
  } else {
    raise_warning("Invalid argument supplied for foreach()");
    hasElems = false;
  }
  return hasElems;
}

bool Iter::next() {
  assert(arr().getIterType() == ArrayIter::TypeArray ||
         arr().getIterType() == ArrayIter::TypeIterator);
  // The emitter should never generate bytecode where the iterator
  // is at the end before IterNext is executed. However, even if
  // the iterator is at the end, it is safe to call next().
  ArrayIter* ai = &arr();
  ai->next();
  if (ai->end()) {
    // If after advancing the iterator we have reached the end, free
    // the iterator and fall through to the next instruction.
    // The ArrayIter destructor will decRef the array.
    ai->~ArrayIter();
    return false;
  }
  // If after advancing the iterator we have not reached the end,
  // jump to the location specified by the second immediate argument.
  return true;
}

void Iter::free() {
  assert(arr().getIterType() == ArrayIter::TypeArray ||
         arr().getIterType() == ArrayIter::TypeIterator);
  arr().~ArrayIter();
}

void Iter::mfree() {
  marr().~MArrayIter();
}

void Iter::cfree() {
  cuf().~CufIter();
}

/**
 * Helper functions for collection style iterators.
 * Iterators over collections are never by-ref so there is no reason to
 * unbox any value.
 * Templates are instantiated over the collection class and the iterator
 * style. See the definition of Fixed, Versionable and VersionableSparse
 * in the header for details.
 * IterInit and IterNext can be called directly from the JIT for specialized
 * iterators.
 */
template<class Coll, class Style>
static void iterValue(ArrayIter* iter, TypedValue* out) {
  Variant val = iter->iterValue<Coll>(Style());
  assert(val.getRawType() != KindOfRef);
  cellDup(*val.asTypedValue(), *out);
}

template<class Coll, class Style>
static void iterKey(ArrayIter* iter, TypedValue* out) {
  Variant key = iter->iterKey<Coll>(Style());
  cellDup(*key.asTypedValue(), *out);
}

template<class Coll, class Style>
static int64_t iterInit(Iter* dest, Coll* coll,
                        TypedValue* valOut, TypedValue* keyOut) {
  int64_t size = coll->size();
  if (UNLIKELY(size == 0)) {
    decRefObj(coll);
    return 0LL;
  }
  (void) new (&dest->arr()) ArrayIter(coll, Style());

  auto const oldVal = *valOut;
  assert(oldVal.m_type != KindOfRef);
  iterValue<Coll, Style>(&dest->arr(), valOut);
  tvDecRefGen(oldVal);

  if (keyOut) {
    auto const oldKey = *keyOut;
    iterKey<Coll, Style>(&dest->arr(), keyOut);
    tvDecRefGen(oldKey);
  }
  return 1LL;
}

template<class Coll, class Style>
static
int64_t iterNext(ArrayIter* iter, TypedValue* valOut, TypedValue* keyOut) {
  if (!iter->iterNext<Coll>(Style())) {
    iter->~ArrayIter();
    return 0LL;
  }

  auto const oldVal = *valOut;
  assert(oldVal.m_type != KindOfRef);
  iterValue<Coll, Style>(iter, valOut);
  tvDecRefGen(oldVal);

  if (keyOut) {
    auto const oldKey = *keyOut;
    iterKey<Coll, Style>(iter, keyOut);
    tvDecRefGen(oldKey);
  }
  return 1LL;
}

/*
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */

template <bool typeArray, bool withRef>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assert(withRef || oldVal.m_type != KindOfRef);
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    auto const cur = arrIter.nvSecond();
    if (cur.type() == KindOfRef) {
      if (!withRef || !cur.val().pref->isReferenced()) {
        cellDup(*(cur.val().pref->tv()), *out);
      } else {
        refDup(cur.tv(), *out);
      }
    } else {
      cellDup(cur.tv(), *out);
    }
  } else {
    Variant val = arrIter.second();
    assert(val.getRawType() != KindOfRef);
    cellDup(*val.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

template <bool typeArray, bool withRef>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assert(withRef || oldVal.m_type != KindOfRef);
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    cellCopy(arr.nvFirst(), *out);
  } else {
    Variant key = arr.first();
    cellDup(*key.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

static NEVER_INLINE
int64_t iter_next_free_packed(Iter* iter, ArrayData* arr) {
  assert(arr->decWillRelease());
  assert(arr->hasPackedLayout());
  // Use non-specialized release call so ArrayTracer can track its destruction
  arr->release();
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

static NEVER_INLINE
int64_t iter_next_free_mixed(Iter* iter, ArrayData* arr) {
  assert(arr->hasMixedLayout());
  assert(arr->decWillRelease());
  // Use non-specialized release call so ArrayTracer can track its destruction
  arr->release();
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

NEVER_INLINE
static int64_t iter_next_free_apc(Iter* iter, APCLocalArray* arr) {
  assert(arr->decWillRelease());
  APCLocalArray::Release(arr->asArrayData());
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

/*
 * new_iter_array creates an iterator for the specified array iff the
 * array is not empty.  If new_iter_array creates an iterator, it does
 * not increment the refcount of the specified array.  If
 * new_iter_array does not create an iterator, it decRefs the array.
 */
template <bool withRef>
NEVER_INLINE
int64_t new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  if (!withRef) {
    valOut = tvToCell(valOut);
    if (keyOut) keyOut = tvToCell(keyOut);
  }
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noInc);
    dest->arr().setIterType(ArrayIter::TypeArray);
    iter_value_cell_local_impl<true, withRef>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true, withRef>(dest, keyOut);
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  decRefArr(arr);
  return 0LL;
}

int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  if (UNLIKELY(ad->getSize() == 0)) {
    if (UNLIKELY(ad->decWillRelease())) {
      if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
      if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
    }
    ad->decRefCount();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<false>(dest, ad, valOut, nullptr);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = ad;
  auto const itypeU32 = static_cast<uint32_t>(ArrayIter::TypeArray);

  if (LIKELY(ad->hasPackedLayout())) {
    aiter.m_pos = 0;
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayPacked) << 16 | itypeU32;
    assert(aiter.m_itype == ArrayIter::TypeArray);
    assert(aiter.m_nextHelperIdx == IterNextIndex::ArrayPacked);
    cellDup(*tvToCell(packedData(ad)), *valOut);
    return 1;
  }

  if (LIKELY(ad->hasMixedLayout())) {
    auto const mixed = MixedArray::asMixed(ad);
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayMixed) << 16 | itypeU32;
    assert(aiter.m_itype == ArrayIter::TypeArray);
    assert(aiter.m_nextHelperIdx == IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  return new_iter_array_cold<false>(dest, ad, valOut, nullptr);
}

template<bool WithRef>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  if (UNLIKELY(ad->getSize() == 0)) {
    if (UNLIKELY(ad->decWillRelease())) {
      if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
      if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
    }
    ad->decRefCount();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<WithRef>(dest, ad, valOut, keyOut);
  }
  if (UNLIKELY(isRefcountedType(keyOut->m_type))) {
    return new_iter_array_cold<WithRef>(dest, ad, valOut, keyOut);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = ad;
  auto const itypeU32 = static_cast<uint32_t>(ArrayIter::TypeArray);

  if (ad->hasPackedLayout()) {
    aiter.m_pos = 0;
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayPacked) << 16 | itypeU32;
    assert(aiter.m_itype == ArrayIter::TypeArray);
    assert(aiter.m_nextHelperIdx == IterNextIndex::ArrayPacked);
    if (WithRef) {
      tvDupWithRef(*packedData(ad), *valOut);
    } else {
      cellDup(*tvToCell(packedData(ad)), *valOut);
    }
    keyOut->m_type = KindOfInt64;
    keyOut->m_data.num = 0;
    return 1;
  }

  if (ad->hasMixedLayout()) {
    auto const mixed = MixedArray::asMixed(ad);
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayMixed) << 16 | itypeU32;
    assert(aiter.m_itype == ArrayIter::TypeArray);
    assert(aiter.m_nextHelperIdx == IterNextIndex::ArrayMixed);
    if (WithRef) {
      mixed->dupArrayElmWithRef(aiter.m_pos, valOut, keyOut);
    } else {
      mixed->getArrayElm(aiter.m_pos, valOut, keyOut);
    }
    return 1;
  }

  return new_iter_array_cold<WithRef>(dest, ad, valOut, keyOut);
}

template int64_t new_iter_array_key<false>(Iter* dest, ArrayData* ad,
                                           TypedValue* valOut,
                                           TypedValue* keyOut);
template int64_t new_iter_array_key<true>(Iter* dest, ArrayData* ad,
                                          TypedValue* valOut,
                                          TypedValue* keyOut);

struct FreeObj {
  FreeObj() : m_obj(0) {}
  void operator=(ObjectData* obj) { m_obj = obj; }
  ~FreeObj() { if (UNLIKELY(m_obj != nullptr)) decRefObj(m_obj); }
 private:
  ObjectData* m_obj;
};

/**
 * new_iter_object_any creates an iterator for the specified object if the
 * object is iterable and it is non-empty (has properties). If
 * new_iter_object_any creates an iterator, it does not increment the refcount
 * of the specified object. If new_iter_object does not create an iterator,
 * it decRefs the object.
 *
 * If exceptions are thrown, new_iter_object_any takes care of decRefing the
 * object.
 */
static int64_t new_iter_object_any(Iter* dest, ObjectData* obj, Class* ctx,
                                   TypedValue* valOut, TypedValue* keyOut) {
  ArrayIter::Type itType;
  {
    FreeObj fo;
    if (obj->isIterator()) {
      TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator\n",
            __func__, dest, obj, ctx);
      (void) new (&dest->arr()) ArrayIter(obj, ArrayIter::noInc);
      itType = ArrayIter::TypeIterator;
    } else {
      bool isIteratorAggregate;
      /*
       * We are not going to transfer ownership of obj to the iterator,
       * so arrange to decRef it later. The actual decRef has to happen
       * after the call to arr().end() below, because both can have visible side
       * effects (calls to __destruct() and valid()). Similarly it has to
       * happen before the iter_*_cell_local_impl calls below, because they call
       * current() and key() (hence the explicit scope around FreeObj fo;)
       */
      fo = obj;

      Object itObj = obj->iterableObject(isIteratorAggregate, false);
      if (isIteratorAggregate) {
        TRACE(2, "%s: I %p, obj %p, ctx %p, IteratorAggregate\n",
              __func__, dest, obj, ctx);
        (void) new (&dest->arr()) ArrayIter(itObj.detach(), ArrayIter::noInc);
        itType = ArrayIter::TypeIterator;
      } else {
        TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
              __func__, dest, obj, ctx);
        auto ctxStr = ctx ? ctx->nameStr() : StrNR();
        Array iterArray(itObj->o_toIterArray(ctxStr, ObjectData::EraseRefs));
        ArrayData* ad = iterArray.get();
        (void) new (&dest->arr()) ArrayIter(ad);
        itType = ArrayIter::TypeArray;
      }
    }
    try {
      if (dest->arr().end()) {
        // Iterator was empty; call the destructor on the iterator we just
        // constructed.
        dest->arr().~ArrayIter();
        return 0LL;
      }
    } catch (...) {
      dest->arr().~ArrayIter();
      throw;
    }
  }

  dest->arr().setIterType(itType);
  if (itType == ArrayIter::TypeIterator) {
    iter_value_cell_local_impl<false, false>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false, false>(dest, keyOut);
    }
  } else {
    iter_value_cell_local_impl<true, false>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true, false>(dest, keyOut);
    }
  }
  return 1LL;
}

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator or Object\n",
        __func__, dest, obj, ctx);
  valOut = tvToCell(valOut);
  if (keyOut) {
    keyOut = tvToCell(keyOut);
  }
  if (obj->isCollection()) {
    auto type = obj->collectionType();
    switch (type) {
      case CollectionType::Vector:
        return iterInit<c_Vector, ArrayIter::Versionable>(
                                  dest, static_cast<c_Vector*>(obj),
                                  valOut, keyOut);
      case CollectionType::Map:
      case CollectionType::ImmMap:
        return iterInit<BaseMap, ArrayIter::VersionableSparse>(
                                  dest,
                                  static_cast<BaseMap*>(obj),
                                  valOut, keyOut);
      case CollectionType::Set:
        return iterInit<c_Set, ArrayIter::VersionableSparse>(
                                  dest,
                                  static_cast<c_Set*>(obj),
                                  valOut, keyOut);
      case CollectionType::Pair:
        return iterInit<c_Pair, ArrayIter::Fixed>(
                                  dest,
                                  static_cast<c_Pair*>(obj),
                                  valOut, keyOut);
      case CollectionType::ImmVector:
        return iterInit<c_ImmVector, ArrayIter::Fixed>(
                                  dest, static_cast<c_ImmVector*>(obj),
                                  valOut, keyOut);
      case CollectionType::ImmSet:
        return iterInit<c_ImmSet, ArrayIter::VersionableSparse>(
                                     dest,
                                     static_cast<c_ImmSet*>(obj),
                                     valOut, keyOut);
    }
  }
  return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  not_reached();
}

template <bool withRef>
NEVER_INLINE
static int64_t iter_next_collection(ArrayIter* ai,
                                    TypedValue* valOut,
                                    TypedValue* keyOut,
                                    CollectionType type) {
  assert(!ai->hasArrayData());
  assert(isValidCollection(type));
  switch (type) {
    case CollectionType::Vector:
      return iterNext<c_Vector, ArrayIter::Versionable>(
        ai, valOut, keyOut);
    case CollectionType::Map:
    case CollectionType::ImmMap:
      return iterNext<BaseMap, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
    case CollectionType::Set:
      return iterNext<c_Set, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
    case CollectionType::Pair:
      return iterNext<c_Pair, ArrayIter::Fixed>(
        ai, valOut, keyOut);
    case CollectionType::ImmVector:
      return iterNext<c_ImmVector, ArrayIter::Fixed>(
        ai, valOut, keyOut);
    case CollectionType::ImmSet:
      return iterNext<c_ImmSet, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
  }
  not_reached();
}

template <bool withRef>
NEVER_INLINE
int64_t iter_next_cold(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  auto const ai = &iter->arr();
  assert(ai->getIterType() == ArrayIter::TypeArray ||
         ai->getIterType() == ArrayIter::TypeIterator);
  if (UNLIKELY(!ai->hasArrayData())) {
    auto obj = ai->getObject();
    if (UNLIKELY(obj->isCollection())) {
      auto const coll = obj->collectionType();
      return iter_next_collection<withRef>(ai, valOut, keyOut, coll);
    }
  }
  ai->next();
  if (ai->end()) {
    // The ArrayIter destructor will decRef the array
    ai->~ArrayIter();
    return 0;
  }
  if (iter->arr().getIterType() == ArrayIter::TypeArray) {
    iter_value_cell_local_impl<true, withRef>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true, withRef>(iter, keyOut);
    }
  } else {
    iter_value_cell_local_impl<false, withRef>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false, withRef>(iter, keyOut);
    }
  }
  return 1;
}

NEVER_INLINE
static int64_t iter_next_apc_array(Iter* iter,
                                   TypedValue* valOut,
                                   TypedValue* keyOut,
                                   ArrayData* ad) {
  assert(ad->kind() == ArrayData::kApcKind);

  auto const arrIter = &iter->arr();
  auto const arr = APCLocalArray::asApcArray(ad);
  ssize_t const pos = arr->iterAdvanceImpl(arrIter->getPos());
  if (UNLIKELY(pos == ad->getSize())) {
    if (UNLIKELY(arr->decWillRelease())) {
      return iter_next_free_apc(iter, arr);
    }
    arr->decRefCount();
    if (debug) {
      iter->arr().setIterType(ArrayIter::TypeUndefined);
    }
    return 0;
  }
  arrIter->setPos(pos);

  // Note that APCLocalArray can never return KindOfRefs.
  auto const rval = APCLocalArray::RvalAtPos(arr->asArrayData(), pos);
  assert(rval.type() != KindOfRef);
  cellSet(rval.tv(), *valOut);
  if (LIKELY(!keyOut)) return 1;

  auto const key = APCLocalArray::NvGetKey(ad, pos);
  auto const oldKey = *keyOut;
  cellCopy(key, *keyOut);
  tvDecRefGen(oldKey);

  return 1;
}

int64_t witer_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }

  {
    auto const ad       = const_cast<ArrayData*>(arrIter->getArrayData());
    auto const isPacked = ad->hasPackedLayout();
    auto const isMixed  = ad->hasMixedLayout();

    if (UNLIKELY(!isMixed && !isPacked)) {
      if (ad->isApcArray()) {
        // TODO(#4055855): what if a local value in an apc array has
        // been turned into a ref?  Is this actually ok to do?
        return iter_next_apc_array(iter, valOut, keyOut, ad);
      }
      goto cold;
    }

    if (LIKELY(isPacked)) {
      ssize_t pos = arrIter->getPos() + 1;
      if (size_t(pos) >= size_t(ad->getSize())) {
        if (UNLIKELY(ad->decWillRelease())) {
          return iter_next_free_packed(iter, ad);
        }
        ad->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }

      if (UNLIKELY(tvDecRefWillCallHelper(valOut)) ||
          UNLIKELY(tvDecRefWillCallHelper(keyOut))) {
        goto cold;
      }
      tvDecRefGenNZ(valOut);
      tvDecRefGenNZ(keyOut);

      arrIter->setPos(pos);
      tvDupWithRef(packedData(ad)[pos], *valOut);
      keyOut->m_type = KindOfInt64;
      keyOut->m_data.num = pos;
      return 1;
    }

    auto const mixed = MixedArray::asMixed(ad);
    ssize_t pos = arrIter->getPos();
    do {
      ++pos;
      if (size_t(pos) >= size_t(mixed->iterLimit())) {
        if (UNLIKELY(mixed->decWillRelease())) {
          return iter_next_free_mixed(iter, mixed->asArrayData());
        }
        mixed->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }
    } while (UNLIKELY(mixed->isTombstone(pos)));

    if (UNLIKELY(tvDecRefWillCallHelper(valOut)) ||
        UNLIKELY(tvDecRefWillCallHelper(keyOut))) {
      goto cold;
    }
    tvDecRefGenNZ(valOut);
    tvDecRefGenNZ(keyOut);

    arrIter->setPos(pos);
    mixed->dupArrayElmWithRef(pos, valOut, keyOut);
    return 1;
  }

cold:
  return iter_next_cold<true>(iter, valOut, keyOut);
}

///////////////////////////////////////////////////////////////////////////////
// MIter functions

int64_t new_miter_array_key(Iter* dest, RefData* v1,
                           TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, v1);

  TypedValue* rtv = v1->tv();
  assert(isArrayLikeType(rtv->m_type));
  ArrayData* ad = rtv->m_data.parr;

  if (UNLIKELY(ad->isHackArray())) {
    throwRefInvalidArrayValueException(ad);
  }
  if (RuntimeOption::EvalHackArrCompatNotices) raiseHackArrCompatRefIter();

  if (UNLIKELY(ad->empty())) {
    return 0LL;
  }

  (void) new (&dest->marr()) MArrayIter(v1);
  dest->marr().advance();

  tvAsVariant(valOut).assignRef(dest->marr().val());
  if (keyOut) {
    tvAsVariant(keyOut).assign(dest->marr().key());
  }

  return 1LL;
}

int64_t new_miter_object(Iter* dest, RefData* ref, Class* ctx,
                      TypedValue* valOut, TypedValue* keyOut) {
  ObjectData *obj = ref->tv()->m_data.pobj;
  if (obj->isCollection()) {
    raise_error("Collection elements cannot be taken by reference");
  }

  bool isIterator;
  Object itObj = obj->iterableObject(isIterator);
  if (isIterator) {
    raise_error("An iterator cannot be used with foreach by reference");
  }

  TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
        __func__, dest, obj, ctx);
  auto ctxStr = ctx ? ctx->nameStr() : StrNR();
  Array iterArray(itObj->o_toIterArray(ctxStr, ObjectData::CreateRefs));
  ArrayData* ad = iterArray.detach();
  (void) new (&dest->marr()) MArrayIter(ad);
  if (UNLIKELY(!dest->marr().advance())) {
    // Iterator was empty; call the destructor on the iterator we just
    // constructed.
    dest->marr().~MArrayIter();
    return 0LL;
  }

  tvAsVariant(valOut).assignRef(dest->marr().val());
  if (keyOut) {
    tvAsVariant(keyOut).assign(dest->marr().key());
  }
  return 1LL;
}

int64_t new_miter_other(Iter* dest, RefData* data) {
  TRACE(2, "%s: I %p, data %p, invalid type\n",
        __func__, dest, data);

  // TODO(#2570852): we should really issue a warning here
  return 0LL;
}

int64_t miter_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "miter_next_key: I %p\n", iter);
  MArrayIter& marr = iter->marr();

  if (UNLIKELY(!marr.advance())) {
    marr.~MArrayIter();
    return 0LL;
  }

  tvAsVariant(valOut).assignRef(marr.val());
  if (keyOut) {
    tvAsVariant(keyOut).assign(marr.key());
  }

  return 1LL;
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers

namespace {

NEVER_INLINE
int64_t iter_next_cold_inc_val(Iter* it,
                               TypedValue* valOut,
                               TypedValue* keyOut) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(valOut);
  return iter_next_cold<false>(it, valOut, keyOut);
}

template<bool HasKey>
ALWAYS_INLINE
int64_t iter_next_mixed_impl(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  ArrayIter& iter    = it->arr();
  auto const arrData = const_cast<ArrayData*>(iter.getArrayData());
  auto const arr     = MixedArray::asMixed(arrData);
  ssize_t pos        = iter.getPos();

  do {
    if (size_t(++pos) >= size_t(arr->iterLimit())) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_mixed(it, arr->asArrayData());
      }
      arr->decRefCount();
      if (debug) {
        iter.setIterType(ArrayIter::TypeUndefined);
      }
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));


  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return iter_next_cold<false>(it, valOut, keyOut);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return iter_next_cold_inc_val(it, valOut, keyOut);
    }
    keyOut->m_data.pcnt->decRefCount();
  }

  iter.setPos(pos);
  if (HasKey) {
    arr->getArrayElm(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

template<bool HasKey>
int64_t iter_next_packed_impl(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut) {
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  auto& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  assert(PackedArray::checkInvariants(ad));

  ssize_t pos = iter.getPos() + 1;
  if (LIKELY(pos < ad->getSize())) {
    if (isRefcountedType(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
        return iter_next_cold<false>(it, valOut, keyOut);
      }
      valOut->m_data.pcnt->decRefCount();
    }
    if (HasKey && UNLIKELY(isRefcountedType(keyOut->m_type))) {
      if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
        return iter_next_cold_inc_val(it, valOut, keyOut);
      }
      keyOut->m_data.pcnt->decRefCount();
    }
    iter.setPos(pos);
    cellDup(*tvToCell(packedData(ad) + pos), *valOut);
    if (HasKey) {
      keyOut->m_data.num = pos;
      keyOut->m_type = KindOfInt64;
    }
    return 1;
  }

  // Finished iterating---we need to free the array.
  if (UNLIKELY(ad->decWillRelease())) {
    return iter_next_free_packed(it, ad);
  }
  ad->decRefCount();
  if (debug) {
    iter.setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

}

int64_t iterNextArrayPacked(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayPacked: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  return iter_next_packed_impl<false>(it, valOut, nullptr);
}

int64_t iterNextKArrayPacked(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayPacked: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  return iter_next_packed_impl<true>(it, valOut, keyOut);
}

int64_t iterNextArrayMixed(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayMixed: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  return iter_next_mixed_impl<false>(it, valOut, nullptr);
}

int64_t iterNextKArrayMixed(Iter* it,
                            TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayMixed: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  return iter_next_mixed_impl<true>(it, valOut, keyOut);
}

int64_t iterNextArray(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArray: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assert(!it->arr().getArrayData()->hasPackedLayout());
  assert(!it->arr().getArrayData()->hasMixedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array(it, valOut, nullptr, ad);
  }
  return iter_next_cold<false>(it, valOut, nullptr);
}

int64_t iterNextKArray(Iter* it,
                       TypedValue* valOut,
                       TypedValue* keyOut) {
  TRACE(2, "iterNextKArray: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assert(!it->arr().getArrayData()->hasMixedLayout());
  assert(!it->arr().getArrayData()->hasPackedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array(it, valOut, keyOut, ad);
  }
  return iter_next_cold<false>(it, valOut, keyOut);
}

int64_t iterNextVector(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextVector: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Vector, ArrayIter::Versionable>(iter, valOut, nullptr);
}

int64_t iterNextKVector(Iter* it,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  TRACE(2, "iterNextKVector: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Vector, ArrayIter::Versionable>(iter, valOut, keyOut);
}

int64_t iterNextImmVector(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterFrozenNextVector: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_ImmVector, ArrayIter::Fixed>(iter, valOut, nullptr);
}

int64_t iterNextKImmVector(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut) {
  TRACE(2, "iterFrozenNextKVector: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_ImmVector, ArrayIter::Fixed>(iter, valOut, keyOut);
}

int64_t iterNextMap(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextMap: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Map, ArrayIter::VersionableSparse>(iter, valOut, nullptr);
}

int64_t iterNextImmMap(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextImmMap: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return
    iterNext<c_ImmMap, ArrayIter::VersionableSparse>(iter, valOut, nullptr);
}

int64_t iterNextKMap(Iter* it,
                     TypedValue* valOut,
                     TypedValue* keyOut) {
  TRACE(2, "iterNextKMap: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Map, ArrayIter::VersionableSparse>(iter, valOut, keyOut);
}

int64_t iterNextKImmMap(Iter* it,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  TRACE(2, "iterNextKImmMap: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_ImmMap, ArrayIter::VersionableSparse>(iter, valOut, keyOut);
}

int64_t iterNextSet(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextSet: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Set, ArrayIter::VersionableSparse>(iter, valOut, nullptr);
}

int64_t iterNextImmSet(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextImmSet: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return
    iterNext<c_ImmSet, ArrayIter::VersionableSparse>(iter, valOut, nullptr);
}

int64_t iterNextKSet(Iter* it,
                     TypedValue* valOut,
                     TypedValue* keyOut) {
  TRACE(2, "iterNextKSet: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Set, ArrayIter::VersionableSparse>(iter, valOut, keyOut);
}

int64_t iterNextKImmSet(Iter* it,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  TRACE(2, "iterNextKImmSet: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_ImmSet, ArrayIter::VersionableSparse>(iter, valOut, keyOut);
}

int64_t iterNextPair(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextPair: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Pair, ArrayIter::Fixed>(iter, valOut, nullptr);
}

int64_t iterNextKPair(Iter* it,
                      TypedValue* valOut,
                      TypedValue* keyOut) {
  TRACE(2, "iterNextKPair: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Pair, ArrayIter::Fixed>(iter, valOut, keyOut);
}

int64_t iterNextObject(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextObject: I %p\n", it);
  // We can't just put the address of iter_next_cold in the table
  // below right now because we need to get a nullptr into the third
  // argument register for it.
  return iter_next_cold<false>(it, valOut, nullptr);
}

using IterNextHelper  = int64_t (*)(Iter*, TypedValue*);
using IterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*);

const IterNextHelper g_iterNextHelpers[] = {
  &iterNextArrayPacked,
  &iterNextArrayMixed,
  &iterNextArray,
  &iterNextVector,
  &iterNextImmVector,
  &iterNextMap,
  &iterNextImmMap,
  &iterNextSet,
  &iterNextImmSet,
  &iterNextPair,
  &iterNextObject,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  &iterNextKArrayPacked,
  &iterNextKArrayMixed,
  &iterNextKArray,
  &iterNextKVector,
  &iterNextKImmVector,
  &iterNextKMap,
  &iterNextKImmMap,
  &iterNextKSet,
  &iterNextKImmSet,
  &iterNextKPair,
  &iter_next_cold<false>, // iterNextKObject
};

int64_t iter_next_ind(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next_ind: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  IterNextHelper iterNext =
      g_iterNextHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return iterNext(iter, valOut);
}

int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key_ind: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  IterNextKHelper iterNextK =
      g_iterNextKHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return iterNextK(iter, valOut, keyOut);
}

///////////////////////////////////////////////////////////////////////////////
}
