/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/array/array_iterator.h>
#include <runtime/base/array/array_data.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/object_data.h>
#include <runtime/ext/ext_collections.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Static strings.

static const Trace::Module TRACEMOD = Trace::runtime;

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
    case Collection::PairType: {
      m_pos = 0;
      break;
    }
    default: {
      assert(obj->instanceof(SystemLib::s_IteratorClass));
      obj->o_invoke(s_rewind, Array());
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
      return m_pos >= getVector()->t_count();
    }
    case Collection::MapType: {
      return m_pos == 0;
    }
    case Collection::StableMapType: {
      return m_pos == 0;
    }
    case Collection::PairType: {
      return m_pos >= getPair()->t_count();
    }
    default: {
      ObjectData* obj = getIteratorObj();
      return !obj->o_invoke(s_valid, Array());
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
    case Collection::PairType: {
      m_pos++;
      return;
    }
    default:
      ObjectData* obj = getIteratorObj();
      obj->o_invoke(s_next, Array());
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
    case Collection::PairType: {
      return m_pos;
    }
    default: {
      ObjectData* obj = getIteratorObj();
      return obj->o_invoke(s_key, Array());
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
      return mp->iter_value(m_pos);
    }
    case Collection::StableMapType: {
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      return smp->iter_value(m_pos);
    }
    case Collection::PairType: {
      return tvAsCVarRef(getPair()->at(m_pos));
    }
    default: {
      ObjectData* obj = getIteratorObj();
      return obj->o_invoke(s_current, Array());
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
      v = mp->iter_value(m_pos);
      break;
    }
    case Collection::StableMapType: {
      c_StableMap* smp = getStableMap();
      if (UNLIKELY(m_version != smp->getVersion())) {
        throw_collection_modified();
      }
      v = smp->iter_value(m_pos);
      break;
    }
    case Collection::PairType: {
      v = tvAsCVarRef(getPair()->at(m_pos));
      break;
    }
    default: {
      ObjectData* obj = getIteratorObj();
      v = obj->o_invoke(s_current, Array());
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
        VM::Class* ctx = arGetContextClass(g_vmContext->getFP());
        CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
        Array iterArray(obj->o_toIterArray(ctxStr));
        ArrayData* ad = iterArray.getArrayData();
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

bool Iter::minit(TypedValue* v1) {
  assert(v1->m_type == KindOfRef);
  bool hasElems = true;
  TypedValue* rtv = v1->m_data.pref->tv();
  if (rtv->m_type == KindOfArray) {
    ArrayData* ad = rtv->m_data.parr;
    if (!ad->empty()) {
      MArrayIter& mi = marr();
      (void) new (&mi) MArrayIter(v1->m_data.pref);
      mi.advance();
    } else {
      hasElems = false;
    }
  } else if (rtv->m_type == KindOfObject)  {
    if (rtv->m_data.pobj->isCollection()) {
      raise_error("Collection elements cannot be taken by reference");
    }
    bool isIterator;
    Object obj = rtv->m_data.pobj->iterableObject(isIterator);
    if (isIterator) {
      raise_error("An iterator cannot be used with foreach by reference");
    }
    VM::Class* ctx = arGetContextClass(g_vmContext->getFP());
    CStrRef ctxStr = ctx ? ctx->nameRef() : null_string;
    Array iterArray = obj->o_toIterArray(ctxStr, true);
    if (iterArray->empty()) {
      hasElems = false;
    } else {
      ArrayData* ad = iterArray.detach();
      MArrayIter& mi = marr();
      (void) new (&mi) MArrayIter(ad);
      mi.advance();
    }
  } else {
    if (!hphpiCompat) {
      raise_warning("Invalid argument supplied for foreach()");
    }
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

bool Iter::mnext() {
  MArrayIter &mi = marr();
  if (!mi.advance()) {
    // If after advancing the iterator we have reached the end, free
    // the iterator and fall through to the next instruction.
    mi.~MArrayIter();
    return false;
  } else {
    // If after advancing the iterator we have not reached the end,
    // jump to the location specified by the second immediate argument.
    return true;
  }
}

void Iter::free() {
  assert(arr().getIterType() == ArrayIter::TypeArray ||
         arr().getIterType() == ArrayIter::TypeIterator);
  arr().~ArrayIter();
}

void Iter::mfree() {
  marr().~MArrayIter();
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
template <bool typeArray>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  assert(oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    TypedValue* cur = tvToCell(arrIter.nvSecond());
    tvDupCell(cur, out);
  } else {
    Variant val = arrIter.second();
    assert(val.getRawType() != KindOfRef);
    tvDupCell((TypedValue*)&val, out);
  }
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

template <bool typeArray>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  DataType oldType = out->m_type;
  assert(oldType != KindOfRef);
  uint64_t oldDatum = out->m_data.num;
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  assert((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    arr.nvFirst(out);
  } else {
    Variant key = arr.first();
    tvDupCell((TypedValue*)&key, out);
  }
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/**
 * new_iter_array creates an iterator for the specified array iff the array is
 * not empty. If new_iter_array creates an iterator, it does not increment the
 * refcount of the specified array. If new_iter_array does not create an
 * iterator, it decRefs the array.
 */
static NEVER_INLINE
int64_t new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                          TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noInc);
    dest->arr().setIterType(ArrayIter::TypeArray);
    iter_value_cell_local_impl<true>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, keyOut);
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  decRefArr(arr);
  return 0LL;
}

static inline void getHphpArrayElm(HphpArray::Elm* elm, TypedValue* valOut,
                                   TypedValue* keyOut) {
  TypedValue* cur = tvToCell(&elm->data);
  tvDupCell(cur, valOut);
  if (keyOut) {
    HphpArray::getElmKey(elm, keyOut);
  }
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
      HphpArray::Elm* elm = arr->getElm(dest->arr().m_pos);
      getHphpArrayElm(elm, valOut, nullptr);
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
  return new_iter_array_cold(dest, ad, valOut, nullptr);
}

HOT_FUNC
int64_t new_iter_array_key(Iter* dest, ArrayData* ad, TypedValue* valOut,
                         TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  if (UNLIKELY(!ad->isHphpArray())) {
    goto cold;
  }
  {
    HphpArray* arr = (HphpArray*)ad;
    if (LIKELY(arr->getSize() != 0)) {
      if (UNLIKELY(tvWillBeReleased(valOut)) ||
          UNLIKELY(tvWillBeReleased(keyOut))) {
        goto cold;
      }
      tvDecRefOnly(valOut);
      tvDecRefOnly(keyOut);
      // We are transferring ownership of the array to the iterator, therefore
      // we do not need to adjust the refcount.
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noIncNonNull);
      dest->arr().setIterType(ArrayIter::TypeArray);
      HphpArray::Elm* elm = arr->getElm(dest->arr().m_pos);
      getHphpArrayElm(elm, valOut, keyOut);
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
  return new_iter_array_cold(dest, ad, valOut, keyOut);
}

class FreeObj {
 public:
  FreeObj() : m_obj(0) {}
  void operator=(ObjectData* obj) { m_obj = obj; }
  ~FreeObj() { if (UNLIKELY(m_obj != nullptr)) decRefObj(m_obj); }
 private:
  ObjectData* m_obj;
};

/**
 * new_iter_object creates an iterator for the specified object if the object
 * is iterable and it is non-empty (has properties). If new_iter_object creates
 * an iterator, it does not increment the refcount of the specified object. If
 * new_iter_object does not create an iterator, it decRefs the object.
 *
 * If exceptions are thrown, new_iter_object takes care of decRefing the object.
 */
HOT_FUNC
int64_t new_iter_object(Iter* dest, ObjectData* obj, VM::Class* ctx,
                      TypedValue* valOut, TypedValue* keyOut) {
  valOut = tvToCell(valOut);
  if (keyOut) {
    keyOut = tvToCell(keyOut);
  }
  ArrayIter::Type itType;
  {
    FreeObj fo;
    if (obj->isCollection() || obj->implementsIterator()) {
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
        ArrayData* ad = iterArray.getArrayData();
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
    iter_value_cell_local_impl<false>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(dest, keyOut);
    }
  } else {
    iter_value_cell_local_impl<true>(dest, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, keyOut);
    }
  }
  return 1LL;
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
static NEVER_INLINE
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
    iter_value_cell_local_impl<true>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(iter, keyOut);
    }
  } else {
    iter_value_cell_local_impl<false>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(iter, keyOut);
    }
  }
  return 1;
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
    if (size_t(pos) >= size_t(arr->getLastE())) {
      if (UNLIKELY(arr->getCount() == 1)) {
        goto cold;
      }
      arr->decRefCount();
      if (debug) {
        iter->arr().setIterType(ArrayIter::TypeUndefined);
      }
      return 0;
    }
    pos = pos + 1;
    HphpArray::Elm* elm = arr->getElm(pos);
    if (UNLIKELY(elm->data.m_type >= HphpArray::KindOfTombstone)) {
      goto cold;
    }
    if (UNLIKELY(tvWillBeReleased(valOut))) {
      goto cold;
    }
    tvDecRefOnly(valOut);
    arrIter->setPos(pos);
    getHphpArrayElm(elm, valOut, nullptr);
    return 1;
  }
cold:
  return iter_next_cold(iter, valOut, nullptr);
}

HOT_FUNC
int64_t iter_next_key(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next: I %p\n", iter);
  assert(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  ArrayIter* arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
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
    if (size_t(pos) >= size_t(arr->getLastE())) {
      if (UNLIKELY(arr->getCount() == 1)) {
        goto cold;
      }
      arr->decRefCount();
      if (debug) {
        iter->arr().setIterType(ArrayIter::TypeUndefined);
      }
      return 0;
    }
    pos = pos + 1;
    HphpArray::Elm* elm = arr->getElm(pos);
    if (UNLIKELY(elm->data.m_type >= HphpArray::KindOfTombstone)) {
      goto cold;
    }
    if (UNLIKELY(tvWillBeReleased(valOut))) {
      goto cold;
    }
    if (UNLIKELY(tvWillBeReleased(keyOut))) {
      goto cold;
    }
    tvDecRefOnly(valOut);
    tvDecRefOnly(keyOut);
    arrIter->setPos(pos);
    getHphpArrayElm(elm, valOut, keyOut);
    return 1;
  }
cold:
  return iter_next_cold(iter, valOut, keyOut);
}

///////////////////////////////////////////////////////////////////////////////
}
