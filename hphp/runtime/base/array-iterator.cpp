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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/ext/ext_collections.h"

// inline methods of HphpArray.
#include "hphp/runtime/base/hphp-array-defs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Static strings.

TRACE_SET_MOD(runtime);

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

///////////////////////////////////////////////////////////////////////////////
// ArrayIter

ArrayIter::ArrayIter(const ArrayData* data) {
  arrInit(data);
}

ArrayIter::ArrayIter(const Array& array) {
  arrInit(array.get());
}

ArrayIter::ArrayIter(ObjectData* obj)
  : m_pos(ArrayData::invalid_index) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(ObjectData* obj, NoInc)
  : m_pos(ArrayData::invalid_index) {
  objInit<false>(obj);
}

ArrayIter::ArrayIter(const Object& obj)
  : m_pos(ArrayData::invalid_index) {
  objInit<true>(obj.get());
}

ArrayIter::ArrayIter(const Cell& c) {
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
  } else {
    m_pos = ArrayData::invalid_index;
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

void ArrayIter::PairInit(ArrayIter* iter, ObjectData* obj) {
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
  Class* cls = obj->getVMClass();
  if (cls == c_Vector::classof()) {
    return IterNextIndex::Vector;
  } else if (cls == c_Map::classof()) {
    return IterNextIndex::Map;
  } else if (cls == c_Set::classof()) {
    return IterNextIndex::Set;
  } else if (cls == c_ImmVector::classof()) {
    return IterNextIndex::ImmVector;
  } else if (cls == c_Pair::classof()) {
    return IterNextIndex::Pair;
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

const ArrayIter::InitFuncPtr
ArrayIter::initFuncTable[Collection::MaxNumTypes] = {
  &ArrayIter::IteratorObjInit,
  &ArrayIter::VectorInit,
  &ArrayIter::MapInit,
  &ArrayIter::SetInit,
  &ArrayIter::PairInit,
  &ArrayIter::ImmVectorInit,
  &ArrayIter::ImmMapInit,
  &ArrayIter::ImmSetInit,
};

template <bool incRef>
void ArrayIter::objInit(ObjectData* obj) {
  assert(obj);
  setObject(obj);
  if (incRef) {
    obj->incRefCount();
  }
  initFuncTable[getCollectionType()](this, obj);
}

void ArrayIter::cellInit(const Cell& c) {
  assert(cellIsPlausible(c));
  if (LIKELY(c.m_type == KindOfArray)) {
    arrInit(c.m_data.parr);
  } else if (LIKELY(c.m_type == KindOfObject)) {
    objInit<true>(c.m_data.pobj);
  } else {
    arrInit(nullptr);
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

bool ArrayIter::endHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      return m_pos >= getVector()->size();
    }
    case Collection::MapType:
    case Collection::ImmMapType: {
      return m_pos == 0;
    }
    case Collection::SetType:
    case Collection::ImmSetType: {
      return m_pos == 0;
    }
    case Collection::PairType: {
      return m_pos >= getPair()->size();
    }
    case Collection::ImmVectorType: {
      return m_pos >= getImmVector()->size();
    }
    case Collection::InvalidType: {
      ObjectData* obj = getIteratorObj();
      return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
    }
  }
  not_reached();
}

void ArrayIter::nextHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      m_pos++;
      return;
    }
    case Collection::MapType:
    case Collection::ImmMapType: {
      BaseMap* mp = getMappish();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      m_pos = mp->iter_next(m_pos);
      return;
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      m_pos = st->iter_next(m_pos);
      return;
    }
    case Collection::PairType: {
      m_pos++;
      return;
    }
    case Collection::ImmVectorType: {
      m_pos++;
      return;
    }
    case Collection::ImmSetType: {
      assert(m_pos != 0);
      c_ImmSet* st = getImmSet();
      assert(m_version == st->getVersion());
      m_pos = st->iter_next(m_pos);
      return;
    }
    case Collection::InvalidType: {
      ObjectData* obj = getIteratorObj();
      obj->o_invoke_few_args(s_next, 0);
    }
  }
}

Variant ArrayIter::firstHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      return m_pos;
    }
    case Collection::MapType:
    case Collection::ImmMapType: {
      BaseMap* mp = getMappish();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      return mp->iter_key(m_pos);
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      return st->iter_key(m_pos);
    }
    case Collection::PairType: {
      return m_pos;
    }
    case Collection::ImmVectorType: {
      return m_pos;
    }
    case Collection::ImmSetType: {
      c_ImmSet* st = getImmSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      return st->iter_key(m_pos);
    }
    case Collection::InvalidType:
      break;
  }
  ObjectData* obj = getIteratorObj();
  return obj->o_invoke_few_args(s_key, 0);
}

Variant ArrayIter::second() {
  if (LIKELY(hasArrayData())) {
    assert(m_pos != ArrayData::invalid_index);
    const ArrayData* ad = getArrayData();
    assert(ad);
    return ad->getValue(m_pos);
  }
  switch (getCollectionType()) {
    case Collection::VectorType: {
      c_Vector* vec = getVector();
      if (UNLIKELY(m_version != vec->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(vec->at(m_pos));
    }
    case Collection::MapType:
    case Collection::ImmMapType: {
      BaseMap* mp = getMappish();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(mp->iter_value(m_pos));
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(st->iter_value(m_pos));
    }
    case Collection::PairType: {
      return tvAsCVarRef(getPair()->at(m_pos));
    }
    case Collection::ImmVectorType: {
      c_ImmVector* fvec = getImmVector();
      if (UNLIKELY(m_version != fvec->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(fvec->at(m_pos));
    }
    case Collection::ImmSetType: {
      c_ImmSet* st = getImmSet();
      assert(m_version == st->getVersion());
      return tvAsCVarRef(st->iter_value(m_pos));
    }
    case Collection::InvalidType:
      break;
  }

  ObjectData* obj = getIteratorObj();
  return obj->o_invoke_few_args(s_current, 0);
}

const Variant& ArrayIter::secondRef() {
  if (!hasArrayData()) {
    throw FatalErrorException("taking reference on iterator objects");
  }
  assert(hasArrayData());
  assert(m_pos != ArrayData::invalid_index);
  const ArrayData* ad = getArrayData();
  assert(ad);
  return ad->getValueRef(m_pos);
}

const Variant& ArrayIter::secondRefPlus() {
  if (LIKELY(hasArrayData())) {
    assert(m_pos != ArrayData::invalid_index);
    const ArrayData* ad = getArrayData();
    assert(ad);
    return ad->getValueRef(m_pos);
  }
  switch (getCollectionType()) {
    case Collection::VectorType: {
      c_Vector* vec = getVector();
      if (UNLIKELY(m_version != vec->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(vec->at(m_pos));
    }
    case Collection::MapType:
    case Collection::ImmMapType: {
      BaseMap* mp = getMappish();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(mp->iter_value(m_pos));
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(st->iter_value(m_pos));
    }
    case Collection::PairType: {
      return tvAsCVarRef(getPair()->at(m_pos));
    }
    case Collection::ImmVectorType: {
      c_ImmVector* fvec = getImmVector();
      if (UNLIKELY(m_version != fvec->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(fvec->at(m_pos));
    }
    case Collection::ImmSetType: {
      c_ImmSet* st = getImmSet();
      assert(m_version == st->getVersion());
      return tvAsCVarRef(st->iter_value(m_pos));
    }
    case Collection::InvalidType: {
      throw_param_is_not_container();
    }
  }
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

RefData* ArrayIter::zSecond() {
  auto tv = nvSecond();
  if (tv->m_type != KindOfRef) {
    tvBox(tv);
  }
  return tv->m_data.pref;
}

///////////////////////////////////////////////////////////////////////////////
// FullPos

bool FullPos::end() const {
  return !const_cast<FullPos*>(this)->prepare();
}

bool FullPos::advance() {
  ArrayData* data = getArray();
  ArrayData* container = getContainer();
  if (!data) {
    if (container) {
      container->freeFullPos(*this);
    }
    setResetFlag(false);
    return false;
  }
  if (container == data) {
    return cowCheck()->advanceFullPos(*this);
  }
  data = reregister();
  assert(data && data == getContainer());
  assert(!getResetFlag());
  if (!data->validFullPos(*this)) return false;
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  data->next();
  return true;
}

bool FullPos::prepare() {
  ArrayData* data = getArray();
  ArrayData* container = getContainer();
  if (!data) {
    if (container) {
      container->freeFullPos(*this);
    }
    setResetFlag(false);
    return false;
  }
  if (container != data) {
    data = reregister();
  }
  return data->validFullPos(*this);
}

void FullPos::escalateCheck() {
  ArrayData* data;
  if (hasVar()) {
    data = getData();
    if (!data) return;
    ArrayData* esc = data->escalate();
    if (data != esc) {
      *const_cast<Variant*>(getVar()) = esc;
    }
  } else {
    assert(hasAd());
    data = getAd();
    ArrayData* esc = data->escalate();
    if (data != esc) {
      esc->incRefCount();
      decRefArr(data);
      setAd(esc);
    }
  }
}

ArrayData* FullPos::cowCheck() {
  ArrayData* data;
  if (hasVar()) {
    data = getData();
    if (!data) return nullptr;
    if (data->hasMultipleRefs() && !data->noCopyOnWrite()) {
      *const_cast<Variant*>(getVar()) = data = data->copyWithStrongIterators();
    }
  } else {
    assert(hasAd());
    data = getAd();
    if (data->hasMultipleRefs() && !data->noCopyOnWrite()) {
      ArrayData* copied = data->copyWithStrongIterators();
      copied->incRefCount();
      decRefArr(data);
      setAd(data = copied);
    }
  }
  return data;
}

ArrayData* FullPos::reregister() {
  ArrayData* container = getContainer();
  assert(getArray() != nullptr && container != getArray());
  if (container != nullptr) {
    container->freeFullPos(*this);
  }
  setResetFlag(false);
  assert(getContainer() == nullptr);
  escalateCheck();
  ArrayData* data = cowCheck();
  data->newFullPos(*this);
  return data;
}

///////////////////////////////////////////////////////////////////////////////
// MutableArrayIter

MutableArrayIter::MutableArrayIter(const Variant *var, Variant *key,
                                   Variant &val) {
  m_var = nullptr;
  m_key = key;
  m_valp = &val;
  setVar(var);
  assert(getVar());
  escalateCheck();
  ArrayData* data = cowCheck();
  if (!data) return;
  data->reset();
  data->newFullPos(*this);
  setResetFlag(true);
  data->next();
  assert(getContainer() == data);
}

MutableArrayIter::MutableArrayIter(ArrayData *data, Variant *key,
                                   Variant &val) {
  m_var = nullptr;
  m_key = key;
  m_valp = &val;
  if (!data) return;
  setAd(data);
  escalateCheck();
  data = cowCheck();
  data->reset();
  data->newFullPos(*this);
  setResetFlag(true);
  data->next();
  assert(getContainer() == data);
}

MutableArrayIter::~MutableArrayIter() {
  // free the iterator
  ArrayData* container = getContainer();
  if (container) {
    container->freeFullPos(*this);
    assert(getContainer() == nullptr);
  }
  // unprotect the data
  if (hasAd()) decRefArr(getAd());
}

bool MutableArrayIter::advance() {
  if (!this->FullPos::advance()) return false;
  ArrayData* data = getArray();
  assert(data);
  assert(!getResetFlag());
  assert(getContainer() == data);
  assert(data->validFullPos(*this));
  m_valp->assignRef(data->getValueRef(m_pos));
  if (m_key) m_key->assignVal(data->getKey(m_pos));
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// MArrayIter

MArrayIter::MArrayIter(const RefData* ref) {
  m_var = nullptr;
  ref->incRefCount();
  setVar(ref->var());
  assert(hasVar());
  escalateCheck();
  ArrayData* data = cowCheck();
  if (!data) return;
  data->reset();
  data->newFullPos(*this);
  setResetFlag(true);
  data->next();
  assert(getContainer() == data);
}

MArrayIter::MArrayIter(ArrayData *data) {
  m_var = nullptr;
  if (!data) return;
  assert(!data->isStatic());
  setAd(data);
  escalateCheck();
  data = cowCheck();
  data->reset();
  data->newFullPos(*this);
  setResetFlag(true);
  data->next();
  assert(getContainer() == data);
}

MArrayIter::~MArrayIter() {
  // free the iterator
  ArrayData* container = getContainer();
  if (container) {
    container->freeFullPos(*this);
    assert(getContainer() == nullptr);
  }
  // unprotect the data
  if (hasVar()) {
    RefData* ref = RefData::refDataFromVariantIfYouDare(getVar());
    decRefRef(ref);
  } else if (hasAd()) {
    decRefArr(getAd());
  }
}

CufIter::~CufIter() {
  if (m_ctx && !(uintptr_t(m_ctx) & 1)) {
    decRefObj((ObjectData*)m_ctx);
  }
  if (m_name) decRefStr(m_name);
}

bool Iter::init(TypedValue* c1) {
  assert(c1->m_type != KindOfRef);
  bool hasElems = true;
  if (c1->m_type == KindOfArray) {
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
        Class* ctx = arGetContextClass(g_context->getFP());
        const String& ctxStr = ctx ? ctx->nameRef() : null_string;
        Array iterArray(obj->o_toIterArray(ctxStr));
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

  DataType vType = valOut->m_type;
  assert(vType != KindOfRef);
  uint64_t vDatum = valOut->m_data.num;
  iterValue<Coll, Style>(&dest->arr(), valOut);
  tvRefcountedDecRefHelper(vType, vDatum);

  if (keyOut) {
    DataType kType = keyOut->m_type;
    uint64_t kDatum = keyOut->m_data.num;
    iterKey<Coll, Style>(&dest->arr(), keyOut);
    tvRefcountedDecRefHelper(kType, kDatum);
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

  DataType vType = valOut->m_type;
  assert(vType != KindOfRef);
  uint64_t vDatum = valOut->m_data.num;
  iterValue<Coll, Style>(iter, valOut);
  tvRefcountedDecRefHelper(vType, vDatum);

  if (keyOut) {
    DataType kType = keyOut->m_type;
    uint64_t kDatum = keyOut->m_data.num;
    iterKey<Coll, Style>(iter, keyOut);
    tvRefcountedDecRefHelper(kType, kDatum);
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
  DataType oldType = out->m_type;
  assert(withRef || oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    TypedValue* cur = arrIter.nvSecond();
    if (cur->m_type == KindOfRef) {
      if (!withRef || !cur->m_data.pref->isReferenced()) {
        cellDup(*(cur->m_data.pref->tv()), *out);
      } else {
        refDup(*cur, *out);
      }
    } else {
      cellDup(*cur, *out);
    }
  } else {
    Variant val = arrIter.second();
    assert(val.getRawType() != KindOfRef);
    cellDup(*val.asTypedValue(), *out);
  }
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

template <bool typeArray, bool withRef>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  assert(withRef || oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    arr.nvFirst(out);
  } else {
    Variant key = arr.first();
    cellDup(*key.asTypedValue(), *out);
  }
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/**
 * new_iter_array creates an iterator for the specified array iff the array is
 * not empty. If new_iter_array creates an iterator, it does not increment the
 * refcount of the specified array. If new_iter_array does not create an
 * iterator, it decRefs the array.
 */
template <bool withRef>
NEVER_INLINE
int64_t new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
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
  valOut = tvToCell(valOut);
  if (UNLIKELY(!ad->isHphpArray())) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (UNLIKELY(tvDecRefWillCallHelper(valOut))) {
        goto cold;
      }
      tvDecRefOnly(valOut);
      // We are transferring ownership of the array to the iterator, therefore
      // we do not need to adjust the refcount.
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noIncNonNull);
      dest->arr().setIterType(ArrayIter::TypeArray);
      arr->getArrayElm<false>(dest->arr().m_pos, valOut, nullptr);
      return 1LL;
    }
    // We did not transfer ownership of the array to an iterator, so we need
    // to decRef the array.
    if (UNLIKELY(arr->hasExactlyOneRef())) {
      goto cold;
    }
    arr->decRefCount();
    return 0LL;
  }
cold:
  return new_iter_array_cold<false>(dest, ad, valOut, nullptr);
}

template <bool withRef>
int64_t new_iter_array_key(Iter* dest, ArrayData* ad,
                           TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  if (!withRef) {
    valOut = tvToCell(valOut);
    keyOut = tvToCell(keyOut);
  }
  if (UNLIKELY(!ad->isHphpArray())) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (!withRef) {
        if (UNLIKELY(tvDecRefWillCallHelper(valOut)) ||
            UNLIKELY(tvDecRefWillCallHelper(keyOut))) {
          goto cold;
        }
        tvDecRefOnly(valOut);
        tvDecRefOnly(keyOut);
      }
      // We are transferring ownership of the array to the iterator, therefore
      // we do not need to adjust the refcount.
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noIncNonNull);
      dest->arr().setIterType(ArrayIter::TypeArray);
      arr->getArrayElm<withRef>(dest->arr().m_pos, valOut, keyOut);
      return 1LL;
    }
    // We did not transfer ownership of the array to an iterator, so we need
    // to decRef the array.
    if (UNLIKELY(arr->hasExactlyOneRef())) {
      goto cold;
    }
    arr->decRefCount();
    return 0LL;
  }
cold:
  return new_iter_array_cold<withRef>(dest, ad, valOut, keyOut);
}

template int64_t new_iter_array_key<false>(Iter* dest, ArrayData* ad,
                                           TypedValue* valOut,
                                           TypedValue* keyOut);
template int64_t new_iter_array_key<true>(Iter* dest, ArrayData* ad,
                                          TypedValue* valOut,
                                          TypedValue* keyOut);

class FreeObj {
 public:
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
  valOut = tvToCell(valOut);
  if (keyOut) {
    keyOut = tvToCell(keyOut);
  }
  ArrayIter::Type itType;
  {
    FreeObj fo;
    if (obj->implementsIterator()) {
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
        const String& ctxStr = ctx ? ctx->nameRef() : null_string;
        Array iterArray(itObj->o_toIterArray(ctxStr));
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
  Collection::Type type = obj->getCollectionType();
  switch (type) {
    case Collection::VectorType:
      return iterInit<c_Vector, ArrayIter::Versionable>(
                                dest, static_cast<c_Vector*>(obj),
                                valOut, keyOut);
    case Collection::MapType:
    case Collection::ImmMapType:
      return iterInit<BaseMap, ArrayIter::VersionableSparse>(
                                dest,
                                static_cast<BaseMap*>(obj),
                                valOut, keyOut);
    case Collection::SetType:
      return iterInit<c_Set, ArrayIter::VersionableSparse>(
                                dest,
                                static_cast<c_Set*>(obj),
                                valOut, keyOut);
    case Collection::PairType:
      return iterInit<c_Pair, ArrayIter::Fixed>(
                                dest,
                                static_cast<c_Pair*>(obj),
                                valOut, keyOut);
    case Collection::ImmVectorType:
      return iterInit<c_ImmVector, ArrayIter::Fixed>(
                                dest, static_cast<c_ImmVector*>(obj),
                                valOut, keyOut);
    case Collection::ImmSetType:
      return iterInit<c_ImmSet, ArrayIter::VersionableSparse>(
                                   dest,
                                   static_cast<c_ImmSet*>(obj),
                                   valOut, keyOut);
    case Collection::InvalidType:
      return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  }
  not_reached();
}

template <bool withRef>
NEVER_INLINE
static int64_t iter_next_collection(ArrayIter* ai,
                                    TypedValue* valOut,
                                    TypedValue* keyOut,
                                    Collection::Type type) {
  assert(!ai->hasArrayData());
  assert(type != Collection::InvalidType);

  switch (type) {
    case Collection::VectorType:
      return iterNext<c_Vector, ArrayIter::Versionable>(
        ai, valOut, keyOut);
    case Collection::MapType:
    case Collection::ImmMapType:
      return iterNext<BaseMap, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
    case Collection::SetType:
      return iterNext<c_Set, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
    case Collection::PairType:
      return iterNext<c_Pair, ArrayIter::Fixed>(
        ai, valOut, keyOut);
    case Collection::ImmVectorType:
      return iterNext<c_ImmVector, ArrayIter::Fixed>(
        ai, valOut, keyOut);
    case Collection::ImmSetType:
      return iterNext<c_ImmSet, ArrayIter::VersionableSparse>(
        ai, valOut, keyOut);
    case Collection::InvalidType:
      break;
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
    auto const coll = ai->getObject()->getCollectionType();
    if (UNLIKELY(coll != Collection::InvalidType)) {
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

static NEVER_INLINE
int64_t iter_next_free_arr(Iter* iter, HphpArray* arr) {
  assert(arr->hasExactlyOneRef());
  if (arr->isPacked()) {
    HphpArray::ReleasePacked(arr);
  } else {
    assert(arr->isHphpArray());
    HphpArray::Release(arr);
  }
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

NEVER_INLINE
static int64_t iter_next_free_apc_array(Iter* iter, APCLocalArray* arr) {
  assert(arr->hasExactlyOneRef());
  APCLocalArray::Release(arr);
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

NEVER_INLINE
static int64_t iter_next_apc_array(Iter* iter,
                                   TypedValue* valOut,
                                   TypedValue* keyOut,
                                   ArrayData* ad) {
  assert(ad->kind() == ArrayData::kSharedKind);

  auto const arrIter = &iter->arr();
  auto const arr = static_cast<APCLocalArray*>(ad);
  ssize_t const pos = arr->iterAdvanceImpl(arrIter->getPos());
  if (UNLIKELY(pos == ArrayData::invalid_index)) {
    if (UNLIKELY(arr->hasExactlyOneRef())) {
      return iter_next_free_apc_array(iter, arr);
    }
    arr->decRefCount();
    if (debug) {
      iter->arr().setIterType(ArrayIter::TypeUndefined);
    }
    return 0;
  }
  arrIter->setPos(pos);

  const Variant& var = APCLocalArray::GetValueRef(arr, pos);
  cellSet(*var.asCell(), *valOut);
  if (LIKELY(!keyOut)) return 1;

  Cell key;
  APCLocalArray::NvGetKey(ad, &key, pos);
  auto const keyType  = keyOut->m_type;
  auto const keyDatum = keyOut->m_data.num;
  cellCopy(key, *keyOut);
  tvRefcountedDecRefHelper(keyType, keyDatum);

  return 1;
}

int64_t iter_next(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }

  {
    auto const ad = const_cast<ArrayData*>(arrIter->getArrayData());
    if (UNLIKELY(!ad->isHphpArray())) {
      if (ad->isSharedArray()) {
        return iter_next_apc_array(iter, valOut, nullptr, ad);
      }
      goto cold;
    }
    auto const arr = static_cast<HphpArray*>(ad);

    ssize_t pos = arrIter->getPos();
    do {
      if (size_t(++pos) >= size_t(arr->iterLimit())) {
        if (UNLIKELY(arr->hasExactlyOneRef())) {
          return iter_next_free_arr(iter, arr);
        }
        arr->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }
    } while (UNLIKELY(arr->isTombstone(pos)));

    if (UNLIKELY(tvDecRefWillCallHelper(valOut))) {
      goto cold;
    }
    tvDecRefOnly(valOut);
    arrIter->setPos(pos);
    arr->getArrayElm<false>(pos, valOut, nullptr);
    return 1;
  }

cold:
  return iter_next_cold<false>(iter, valOut, nullptr);
}

template <bool withRef>
int64_t iter_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  if (!withRef) {
    valOut = tvToCell(valOut);
    keyOut = tvToCell(keyOut);
  }
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }

  {
    auto const ad = const_cast<ArrayData*>(arrIter->getArrayData());
    if (UNLIKELY(!ad->isHphpArray())) {
      if (ad->isSharedArray()) {
        return iter_next_apc_array(iter, valOut, keyOut, ad);
      }
      goto cold;
    }
    auto const arr = static_cast<HphpArray*>(ad);

    ssize_t pos = arrIter->getPos();
    do {
      ++pos;
      if (size_t(pos) >= size_t(arr->iterLimit())) {
        if (UNLIKELY(arr->hasExactlyOneRef())) {
          return iter_next_free_arr(iter, arr);
        }
        arr->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }
    } while (UNLIKELY(arr->isTombstone(pos)));

    if (!withRef) {
      if (UNLIKELY(tvDecRefWillCallHelper(valOut))) {
        goto cold;
      }
      if (UNLIKELY(tvDecRefWillCallHelper(keyOut))) {
        goto cold;
      }
      tvDecRefOnly(valOut);
      tvDecRefOnly(keyOut);
    }
    arrIter->setPos(pos);
    arr->getArrayElm<withRef>(pos, valOut, keyOut);
    return 1;
  }

cold:
  return iter_next_cold<withRef>(iter, valOut, keyOut);
}

template int64_t iter_next_key<false>(Iter* dest,
                                      TypedValue* valOut,
                                      TypedValue* keyOut);
template int64_t iter_next_key<true>(Iter* dest,
                                     TypedValue* valOut,
                                     TypedValue* keyOut);

///////////////////////////////////////////////////////////////////////////////
// MIter functions

int64_t new_miter_array_key(Iter* dest, RefData* v1,
                           TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, v1);

  TypedValue* rtv = v1->tv();
  ArrayData* ad = rtv->m_data.parr;

  if (UNLIKELY(ad->empty())) {
    return 0LL;
  }

  (void) new (&dest->marr()) MArrayIter(v1);
  dest->marr().advance();

  tvAsVariant(valOut).assignRef(dest->marr().val());
  if (keyOut) {
    tvAsVariant(keyOut).assignVal(dest->marr().key());
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
  const String& ctxStr = ctx ? ctx->nameRef() : null_string;
  Array iterArray(itObj->o_toIterArray(ctxStr, true));
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
    tvAsVariant(keyOut).assignVal(dest->marr().key());
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
    tvAsVariant(keyOut).assignVal(marr.key());
  }

  return 1LL;
}

ArrayIter getContainerIter(const Variant& v) {
  auto c = v.asCell();
  if (c->m_type == KindOfArray) {
    ArrayData* a = c->m_data.parr;
    return ArrayIter(a);
  }
  if (c->m_type == KindOfObject) {
    ObjectData* o = c->m_data.pobj;
    if (o->isCollection()) return ArrayIter(o);
  }
  throw_param_is_not_container();
}

ArrayIter getContainerIter(const Variant& v, size_t& sz) {
  auto c = v.asCell();
  if (c->m_type == KindOfArray) {
    auto a = c->m_data.parr;
    sz = a->size();
    return ArrayIter(a);
  }
  if (c->m_type == KindOfObject) {
    auto o = c->m_data.pobj;
    if (o->isCollection()) {
      sz = getCollectionSize(o);
      return ArrayIter(o);
    }
  }
  throw_param_is_not_container();
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers


namespace {

template <bool isPacked, bool key>
ALWAYS_INLINE
int64_t iterNextArrayGeneric(Iter* it, TypedValue* valOut, TypedValue* keyOut) {
  ArrayIter& iter = it->arr();
  auto const arrData = const_cast<ArrayData*>(iter.getArrayData());
  auto const arr = static_cast<HphpArray*>(arrData);
  ssize_t pos = iter.getPos();

  do {
    if (size_t(++pos) >= size_t(arr->iterLimit())) {
      if (UNLIKELY(arr->hasExactlyOneRef())) {
        return iter_next_free_arr(it, arr);
      }
      arr->decRefCount();
      if (debug) {
        iter.setIterType(ArrayIter::TypeUndefined);
      }
      return 0;
    }
  } while (!isPacked && UNLIKELY(arr->isTombstone(pos)));

  if (UNLIKELY(tvDecRefWillCallHelper(valOut))) {
    return iter_next_cold<false>(it, valOut, keyOut);
  }
  if (key && UNLIKELY(tvDecRefWillCallHelper(keyOut))) {
    return iter_next_cold<false>(it, valOut, keyOut);
  }
  tvDecRefOnly(valOut);
  if (key) {
    tvDecRefOnly(keyOut);
  }
  iter.setPos(pos);
  if (key) {
    arr->getArrayElm<false>(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

}

int64_t iterNextArrayPacked(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayPacked: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->isPacked());

  return iterNextArrayGeneric<true, false>(it, valOut, nullptr);
}

int64_t iterNextKArrayPacked(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayPacked: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->isPacked());

  return iterNextArrayGeneric<true, true>(it, valOut, keyOut);
}

int64_t iterNextArrayMixed(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayMixed: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->isHphpArray() &&
         !it->arr().getArrayData()->isPacked());

  return iterNextArrayGeneric<false, false>(it, valOut, nullptr);
}

int64_t iterNextKArrayMixed(Iter* it,
                            TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayMixed: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->isHphpArray() &&
         !it->arr().getArrayData()->isPacked());

  return iterNextArrayGeneric<false, true>(it, valOut, keyOut);
}

int64_t iterNextArray(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArray: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         !it->arr().getArrayData()->isHphpArray());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isSharedArray()) {
    return iter_next_apc_array(it, valOut, nullptr, ad);
  }
  return iter_next_cold<false>(it, valOut, nullptr);
}

int64_t iterNextKArray(Iter* it,
                       TypedValue* valOut,
                       TypedValue* keyOut) {
  TRACE(2, "iterNextKArray: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         !it->arr().getArrayData()->isHphpArray());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isSharedArray()) {
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

int64_t iterNextKMap(Iter* it,
                     TypedValue* valOut,
                     TypedValue* keyOut) {
  TRACE(2, "iterNextKMap: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Map, ArrayIter::VersionableSparse>(iter, valOut, keyOut);
}

int64_t iterNextSet(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextSet: I %p\n", it);
  assert(it->arr().getIterType() == ArrayIter::TypeIterator &&
         it->arr().hasCollection());

  auto const iter = &it->arr();
  return iterNext<c_Set, ArrayIter::VersionableSparse>(iter, valOut, nullptr);
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
  return iter_next_cold<false>(it, valOut, nullptr);
}

int64_t iterNextKObject(Iter* it,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  TRACE(2, "iterNextKObject: I %p\n", it);
  return iter_next_cold<false>(it, valOut, keyOut);
}

const IterNextHelper g_iterNextHelpers[] = {
  (IterNextHelper)&iterNextArrayPacked,
  (IterNextHelper)&iterNextArrayMixed,
  (IterNextHelper)&iterNextArray,
  (IterNextHelper)&iterNextVector,
  (IterNextHelper)&iterNextImmVector,
  (IterNextHelper)&iterNextMap,
  (IterNextHelper)&iterNextSet,
  (IterNextHelper)&iterNextPair,
  (IterNextHelper)&iterNextObject,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  (IterNextKHelper)&iterNextKArrayPacked,
  (IterNextKHelper)&iterNextKArrayMixed,
  (IterNextKHelper)&iterNextKArray,
  (IterNextKHelper)&iterNextKVector,
  (IterNextKHelper)&iterNextKImmVector,
  (IterNextKHelper)&iterNextKMap,
  (IterNextKHelper)&iterNextKSet,
  (IterNextKHelper)&iterNextKPair,
  (IterNextKHelper)&iterNextKObject,
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
