/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array_iterator.h"
#include "hphp/runtime/base/array_data.h"
#include "hphp/runtime/base/hphp_array.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/object_data.h"
#include "hphp/runtime/ext/ext_collections.h"

// inline methods of HphpArray.
#include "hphp/runtime/base/hphp_array-defs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Static strings.

TRACE_SET_MOD(runtime);

static StaticString s_rewind("rewind");
static StaticString s_valid("valid");
static StaticString s_next("next");
static StaticString s_key("key");
static StaticString s_current("current");

///////////////////////////////////////////////////////////////////////////////
// ArrayIter

ArrayIter::ArrayIter() : m_pos(ArrayData::invalid_index) {
  m_data = nullptr;
}

HOT_FUNC
ArrayIter::ArrayIter(const ArrayData *data) {
  setArrayData(data);
  if (data) {
    data->incRefCount();
    m_pos = data->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

HOT_FUNC
ArrayIter::ArrayIter(CArrRef array) : m_pos(0) {
  const ArrayData* ad = array.get();
  setArrayData(ad);
  if (ad) {
    ad->incRefCount();
    m_pos = ad->iter_begin();
  } else {
    m_pos = ArrayData::invalid_index;
  }
}

void ArrayIter::reset() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    m_data = nullptr;
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return;
  }
  ObjectData* obj = getObject();
  m_data = nullptr;
  assert(obj);
  decRefObj(obj);
}

template <bool incRef>
void ArrayIter::objInit(ObjectData *obj) {
  assert(obj);
  setObject(obj);
  if (incRef) {
    obj->incRefCount();
  }
  switch (getCollectionType()) {
    case Collection::VectorType: {
      m_version = getVector()->getVersion();
      m_pos = 0;
      break;
    }
    case Collection::MapType: {
      c_Map* mp = getMap();
      m_version = mp->getVersion();
      m_pos = mp->iter_begin();
      break;
    }
    case Collection::StableMapType: {
      c_StableMap* smp = getStableMap();
      m_version = smp->getVersion();
      m_pos = smp->iter_begin();
      break;
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      m_version = st->getVersion();
      m_pos = st->iter_begin();
      break;
    }
    case Collection::PairType: {
      m_pos = 0;
      break;
    }
    default: {
      assert(obj->instanceof(SystemLib::s_IteratorClass));
      obj->o_invoke_few_args(s_rewind, 0);
      break;
    }
  }
}

ArrayIter::ArrayIter(ObjectData *obj)
  : m_pos(ArrayData::invalid_index) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(Object &obj, TransferOwner)
  : m_pos(ArrayData::invalid_index) {
  objInit<false>(obj.get());
  (void) obj.detach();
}

// Special constructor used by the VM. This constructor does not increment the
// refcount of the specified object.
ArrayIter::ArrayIter(ObjectData *obj, NoInc)
  : m_pos(ArrayData::invalid_index) {
  objInit<false>(obj);
}

HOT_FUNC
ArrayIter::~ArrayIter() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
  } else {
    ObjectData* obj = getObject();
    assert(obj);
    decRefObj(obj);
  }
  if (debug) {
    m_itype = TypeUndefined;
  }
}

bool ArrayIter::endHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      return m_pos >= getVector()->size();
    }
    case Collection::MapType: {
      return m_pos == 0;
    }
    case Collection::StableMapType: {
      return m_pos == 0;
    }
    case Collection::SetType: {
      return m_pos == 0;
    }
    case Collection::PairType: {
      return m_pos >= getPair()->size();
    }
    default: {
      ObjectData* obj = getIteratorObj();
      return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
    }
  }
}

void ArrayIter::nextHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      m_pos++;
      return;
    }
    case Collection::MapType: {
      assert(m_pos != 0);
      c_Map* mp = getMap();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      m_pos = mp->iter_next(m_pos);
      return;
    }
    case Collection::StableMapType: {
      assert(m_pos != 0);
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      m_pos = smp->iter_next(m_pos);
      return;
    }
    case Collection::SetType: {
      assert(m_pos != 0);
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
    default:
      ObjectData* obj = getIteratorObj();
      obj->o_invoke_few_args(s_next, 0);
  }
}

Variant ArrayIter::firstHelper() {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      return m_pos;
    }
    case Collection::MapType: {
      assert(m_pos != 0);
      c_Map* mp = getMap();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      return mp->iter_key(m_pos);
    }
    case Collection::StableMapType: {
      assert(m_pos != 0);
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      return smp->iter_key(m_pos);
    }
    case Collection::SetType: {
      return uninit_null();
    }
    case Collection::PairType: {
      return m_pos;
    }
    default: {
      ObjectData* obj = getIteratorObj();
      return obj->o_invoke_few_args(s_key, 0);
    }
  }
}

HOT_FUNC
Variant ArrayIter::second() {
  if (hasArrayData()) {
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
    case Collection::MapType: {
      c_Map* mp = getMap();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(mp->iter_value(m_pos));
    }
    case Collection::StableMapType: {
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      return tvAsCVarRef(smp->iter_value(m_pos));
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
    default: {
      ObjectData* obj = getIteratorObj();
      return obj->o_invoke_few_args(s_current, 0);
    }
  }
}

void ArrayIter::secondHelper(Variant& v) {
  switch (getCollectionType()) {
    case Collection::VectorType: {
      c_Vector* vec = getVector();
      if (UNLIKELY(m_version != vec->getVersion())) {
        throw_collection_modified();
      }
      v = tvAsCVarRef(vec->at(m_pos));
      break;
    }
    case Collection::MapType: {
      c_Map* mp = getMap();
      if (UNLIKELY(m_version != mp->getVersion())) {
        throw_collection_modified();
      }
      v = tvAsCVarRef(mp->iter_value(m_pos));
      break;
    }
    case Collection::StableMapType: {
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      v = tvAsCVarRef(smp->iter_value(m_pos));
      break;
    }
    case Collection::SetType: {
      c_Set* st = getSet();
      if (UNLIKELY(m_version != st->getVersion())) {
        throw_collection_modified();
      }
      v = tvAsCVarRef(st->iter_value(m_pos));
      break;
    }
    case Collection::PairType: {
      v = tvAsCVarRef(getPair()->at(m_pos));
      break;
    }
    default: {
      ObjectData* obj = getIteratorObj();
      v = obj->o_invoke_few_args(s_current, 0);
      break;
    }
  }
}

HOT_FUNC
CVarRef ArrayIter::secondRef() {
  if (!hasArrayData()) {
    throw FatalErrorException("taking reference on iterator objects");
  }
  assert(hasArrayData());
  assert(m_pos != ArrayData::invalid_index);
  const ArrayData* ad = getArrayData();
  assert(ad);
  return ad->getValueRef(m_pos);
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
inline ALWAYS_INLINE
bool ArrayIter::iterNext(Fixed) {
  return ++m_pos < static_cast<Tuplish*>(getObject())->size();
}

template<class Vectorish>
inline ALWAYS_INLINE
bool ArrayIter::iterNext(Versionable) {
  Vectorish* vec = static_cast<Vectorish*>(getObject());
  if (UNLIKELY(m_version != vec->getVersion())) {
    throw_collection_modified();
  }
  return ++m_pos < vec->size();
}

template<class Mappish>
inline ALWAYS_INLINE
bool ArrayIter::iterNext(VersionableSparse) {
  Mappish* coll = static_cast<Mappish*>(getObject());
  if (UNLIKELY(m_version != coll->getVersion())) {
    throw_collection_modified();
  }
  m_pos = coll->iter_next(m_pos);
  return m_pos != 0;
}

template<class Tuplish>
inline ALWAYS_INLINE
Variant ArrayIter::iterKey(Fixed) {
  return m_pos;
}

template<class Vectorish>
inline ALWAYS_INLINE
Variant ArrayIter::iterKey(Versionable) {
  return m_pos;
}

template<class Mappish>
inline ALWAYS_INLINE
Variant ArrayIter::iterKey(VersionableSparse) {
  return static_cast<Mappish*>(getObject())->iter_key(m_pos);
}

template<class Tuplish>
inline ALWAYS_INLINE
Variant ArrayIter::iterValue(Fixed) {
  return tvAsCVarRef(static_cast<Tuplish*>(getObject())->get(m_pos));
}

template<class Vectorish>
inline ALWAYS_INLINE
Variant ArrayIter::iterValue(Versionable) {
  return tvAsCVarRef(static_cast<Vectorish*>(getObject())->get(m_pos));
}

template<class Mappish>
inline ALWAYS_INLINE
Variant ArrayIter::iterValue(VersionableSparse) {
  return tvAsCVarRef(static_cast<Mappish*>(getObject())->iter_value(m_pos));
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
    if (data->getCount() > 1 && !data->noCopyOnWrite()) {
      *const_cast<Variant*>(getVar()) = data = data->copyWithStrongIterators();
    }
  } else {
    assert(hasAd());
    data = getAd();
    if (data->getCount() > 1 && !data->noCopyOnWrite()) {
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
        (void) new (&arr()) ArrayIter(obj, ArrayIter::transferOwner);
      } else {
        Class* ctx = arGetContextClass(g_vmContext->getFP());
        CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
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
HOT_FUNC static
void iterValue(ArrayIter* iter, TypedValue* out) {
  Variant val = iter->iterValue<Coll>(Style());
  assert(val.getRawType() != KindOfRef);
  cellDup(*val.asTypedValue(), *out);
}

template<class Coll, class Style>
HOT_FUNC static
void iterKey(ArrayIter* iter, TypedValue* out) {
  Variant key = iter->iterKey<Coll>(Style());
  cellDup(*key.asTypedValue(), *out);
}

template<class Coll, class Style>
HOT_FUNC static
int64_t iterInit(Iter* dest, Coll* coll,
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
HOT_FUNC static
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
 *
 * This function has been split into hot and cold parts. The hot part has
 * been carefully crafted so that it's a leaf function (after all functions
 * it calls have been trivially inlined) that then tail calls a cold
 * version of itself (new_value_cell_cold). The hot part should cover the
 * common case, which occurs when the array parameter is an HphpArray.
 * If you make any changes to this function, please keep the hot/cold
 * splitting in mind, and disasemble the optimized version of the binary
 * to make sure the hot part is a good-looking leaf function; otherwise,
 * you're likely to get a performance regression.
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
      if (!withRef || cur->m_data.pref->getCount() == 1) {
        cur = cur->m_data.pref->tv();
      }
    }
    tvDup(*cur, *out);
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

HOT_FUNC
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  valOut = tvToCell(valOut);
  if (UNLIKELY(!ad->isHphpArray())) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (UNLIKELY(tvWillBeReleased(valOut))) {
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
    if (UNLIKELY(arr->getCount() == 1)) {
      goto cold;
    }
    arr->decRefCount();
    return 0LL;
  }
cold:
  return new_iter_array_cold<false>(dest, ad, valOut, nullptr);
}

template <bool withRef>
HOT_FUNC
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
        if (UNLIKELY(tvWillBeReleased(valOut)) ||
            UNLIKELY(tvWillBeReleased(keyOut))) {
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
    if (UNLIKELY(arr->getCount() == 1)) {
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
HOT_FUNC
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
      try {
        (void) new (&dest->arr()) ArrayIter(obj, ArrayIter::noInc);
      } catch (...) {
        decRefObj(obj);
        throw;
      }
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
        (void) new (&dest->arr()) ArrayIter(itObj, ArrayIter::transferOwner);
        itType = ArrayIter::TypeIterator;
      } else {
        TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
              __func__, dest, obj, ctx);
        CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
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

HOT_FUNC
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
      return iterInit<c_Map, ArrayIter::VersionableSparse>(
                                dest,
                                static_cast<c_Map*>(obj),
                                valOut, keyOut);
    case Collection::StableMapType:
      return iterInit<c_StableMap, ArrayIter::VersionableSparse>(
                                dest,
                                static_cast<c_StableMap*>(obj),
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
    default:
      return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  }
}

/**
 * iter_next will advance the iterator to point to the next element.
 * If the iterator reaches the end, iter_next will free the iterator
 * and will decRef the array.
 * This function has been split into hot and cold parts. The hot part has
 * been carefully crafted so that it's a leaf function (after all functions
 * it calls have been trivially inlined) that then tail calls a cold
 * version of itself (iter_next_array_cold). The hot part should cover the
 * common case, which occurs when the array parameter is an HphpArray.
 * If you make any changes to this function, please keep the hot/cold
 * splitting in mind, and disasemble the optimized version of the binary
 * to make sure the hot part is a good-looking leaf function; otherwise,
 * you're likely to get a performance regression.
 */
template <bool withRef>
NEVER_INLINE
int64_t iter_next_cold(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_cold: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  ArrayIter* ai = &iter->arr();
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

template <bool withRef>
HOT_FUNC
static int64_t iter_next_collection(
    Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  TRACE(2, "iter_next_collection: I %p\n", iter);

  ArrayIter* ai = &iter->arr();
  Collection::Type type = (ai->hasArrayData()) ?
                                Collection::InvalidType :
                                ai->getObject()->getCollectionType();
  switch (type) {
    case Collection::VectorType:
      return iterNext<c_Vector, ArrayIter::Versionable>(
                                ai, valOut, keyOut);
    case Collection::MapType:
      return iterNext<c_Map, ArrayIter::VersionableSparse>(
                                ai, valOut, keyOut);
    case Collection::StableMapType:
      return iterNext<c_StableMap, ArrayIter::VersionableSparse>(
                                ai, valOut, keyOut);
    case Collection::SetType:
      return iterNext<c_Set, ArrayIter::VersionableSparse>(
                                ai, valOut, keyOut);
    case Collection::PairType:
      return iterNext<c_Pair, ArrayIter::Fixed>(
                                ai, valOut, keyOut);
    default:
      return iter_next_cold<withRef>(iter, valOut, keyOut);
  }
}

HOT_FUNC
int64_t iter_next(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  ArrayIter* arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }
  {
    const ArrayData* ad = arrIter->getArrayData();
    if (UNLIKELY(!ad->isHphpArray())) {
      goto cold;
    }
    const HphpArray* arr = (HphpArray*)ad;
    ssize_t pos = arrIter->getPos();
    do {
      if (size_t(++pos) >= size_t(arr->iterLimit())) {
        if (UNLIKELY(arr->getCount() == 1)) {
          goto cold;
        }
        arr->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }
    } while (arr->isTombstone(pos));
    if (UNLIKELY(tvWillBeReleased(valOut))) {
      goto cold;
    }
    tvDecRefOnly(valOut);
    arrIter->setPos(pos);
    arr->getArrayElm<false>(pos, valOut, nullptr);
    return 1;
  }
cold:
  return iter_next_collection<false>(iter, valOut, nullptr);
}

template <bool withRef>
HOT_FUNC
int64_t iter_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  ArrayIter* arrIter = &iter->arr();
  if (!withRef) {
    valOut = tvToCell(valOut);
    keyOut = tvToCell(keyOut);
  }
  if (UNLIKELY(!arrIter->hasArrayData())) {
    goto cold;
  }
  {
    const ArrayData* ad = arrIter->getArrayData();
    if (UNLIKELY(!ad->isHphpArray())) {
      goto cold;
    }
    const HphpArray* arr = (HphpArray*)ad;
    ssize_t pos = arrIter->getPos();
    do {
      ++pos;
      if (size_t(pos) >= size_t(arr->iterLimit())) {
        if (UNLIKELY(arr->getCount() == 1)) {
          goto cold;
        }
        arr->decRefCount();
        if (debug) {
          iter->arr().setIterType(ArrayIter::TypeUndefined);
        }
        return 0;
      }
    } while (arr->isTombstone(pos));
    if (!withRef) {
      if (UNLIKELY(tvWillBeReleased(valOut))) {
        goto cold;
      }
      if (UNLIKELY(tvWillBeReleased(keyOut))) {
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
  return iter_next_collection<withRef>(iter, valOut, keyOut);
}

template int64_t iter_next_key<false>(Iter* dest,
                                      TypedValue* valOut,
                                      TypedValue* keyOut);
template int64_t iter_next_key<true>(Iter* dest,
                                     TypedValue* valOut,
                                     TypedValue* keyOut);

///////////////////////////////////////////////////////////////////////////////
// MIter functions

HOT_FUNC
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

HOT_FUNC
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
  CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
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

HOT_FUNC
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

///////////////////////////////////////////////////////////////////////////////
}
