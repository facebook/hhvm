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
  cellInit(*v.toCell());
}

ArrayIter::ArrayIter(const ArrayIter& iter) {
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
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

template <bool incRef>
void ArrayIter::objInit(ObjectData* obj) {
  assertx(obj);

  if (LIKELY(obj->isCollection())) {
    if (auto ad = collections::asArray(obj)) {
      ad->incRefCount();
      if (!incRef) decRefObj(obj);
      m_pos = ad->iter_begin();
      setArrayData(ad);
    } else {
      assertx(obj->collectionType() == CollectionType::Pair);
      auto arr = collections::toArray(obj);
      if (!incRef) decRefObj(obj);
      m_pos = arr->iter_begin();
      setArrayData(arr.detach());
    }
    return;
  }

  assertx(obj->instanceof(SystemLib::s_IteratorClass));
  setObject(obj);
  if (incRef) obj->incRefCount();
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
    this->m_data = nullptr;
    if (debug) this->m_itype = TypeUndefined;
    decRefObj(obj);
    throw;
  }
}

void ArrayIter::cellInit(const Cell c) {
  assertx(cellIsPlausible(c));
  if (LIKELY(isArrayLikeType(c.m_type))) {
    arrInit(c.m_data.parr);
  } else if (LIKELY(c.m_type == KindOfObject)) {
    objInit<true>(c.m_data.pobj);
  } else {
    arrInit(nullptr);
  }
}

void ArrayIter::rewind() {
  assertx(hasArrayData());
  if (auto* data = getArrayData()) {
    m_pos = data->iter_begin();
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
  assertx(obj);
  decRefObj(obj);
}

ArrayIter& ArrayIter::operator=(const ArrayIter& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
    obj->incRefCount();
  }
  return *this;
}

ArrayIter& ArrayIter::operator=(ArrayIter&& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_itype = iter.m_itype;
  m_nextHelperIdx = iter.m_nextHelperIdx;
  iter.m_data = nullptr;
  return *this;
}

bool ArrayIter::endHelper() const  {
  auto obj = getObject();
  return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
}

void ArrayIter::nextHelper() {
  auto obj = getObject();
  obj->o_invoke_few_args(s_next, 0);
}

Variant ArrayIter::firstHelper() {
  auto obj = getObject();
  return obj->o_invoke_few_args(s_key, 0);
}

Variant ArrayIter::second() {
  if (LIKELY(hasArrayData())) {
    const ArrayData* ad = getArrayData();
    assertx(ad);
    assertx(m_pos != ad->iter_end());
    return ad->getValue(m_pos);
  }
  auto obj = getObject();
  return obj->o_invoke_few_args(s_current, 0);
}

tv_rval ArrayIter::secondRval() const {
  if (!hasArrayData()) {
    raise_fatal_error("taking reference on iterator objects");
  }
  assertx(hasArrayData());
  const ArrayData* ad = getArrayData();
  assertx(ad);
  assertx(m_pos != ad->iter_end());
  return ad->rvalPos(m_pos);
}

tv_rval ArrayIter::secondRvalPlus() {
  if (LIKELY(hasArrayData())) {
    const ArrayData* ad = getArrayData();
    assertx(ad);
    assertx(m_pos != ad->iter_end());
    return ad->rvalPos(m_pos);
  }
  throw_param_is_not_container();
}

//////////////////////////////////////////////////////////////////////

THREAD_LOCAL_FLAT(MIterTable, tl_miter_table);

void MIterTable::clear() {
  if (!tl_miter_table) return;
  auto t = tl_miter_table.get();
  t->ents.fill({nullptr, nullptr});
  t->extras.clear();
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
  auto& table = *tl_miter_table.getCheck();
  if (LIKELY(!table.ents[0].array)) {
    return &table.ents[0];
  }
  return find_empty_strong_iter_slow(table);
}

void newMArrayIter(MArrayIter* marr, ArrayData* ad) {
  assertx(!marr->getContainer());
  auto const slot = find_empty_strong_iter();
  assertx(!slot->array);
  slot->iter = marr;
  slot->array = ad;
  marr->setContainer(ad);
  assertx(strong_iterators_exist());
}

template<class Cond>
void free_strong_iterator_impl(Cond cond) {
  assertx(strong_iterators_exist());

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

  auto& table = *tl_miter_table;
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
  assertx(strong_iterators_exist());
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
  assertx(hasRef());
  escalateCheck();
  auto const data = cowCheck();
  assertx(data);
  data->reset();
  data->next();
  newMArrayIter(this, data);
  assertx(getContainer() == data);
}

MArrayIter::MArrayIter(ArrayData* data)
  : m_pos(0)
  , m_container(nullptr)
  , m_resetFlag(true)
{
  // this constructor is only used for object iteration, so we always get in a
  // mixed array with refcount 1
  assertx(data);
  assertx(data->isMixed() && data->hasExactlyOneRef());
  setAd(data);
  data->reset();
  data->next();
  newMArrayIter(this, data);
  assertx(getContainer() == data);
}

MArrayIter::~MArrayIter() {
  auto const container = getContainer();
  if (container) {
    freeMArrayIter(this);
    assertx(getContainer() == nullptr);
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
  assertx(data && data == getContainer());
  assertx(!getResetFlag());
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
  assertx(hasRef());
  auto const data = getData();
  if (!data) return;
  auto const esc = data->escalate();
  if (data != esc) {
    cellMove(make_array_like_tv(esc), *getRef()->cell());
  }
}

ArrayData* MArrayIter::cowCheck() {
  assertx(hasRef());
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
  cellMove(make_array_like_tv(copy), *getRef()->cell());
  return copy;
}

ArrayData* MArrayIter::reregister() {
  assertx(hasRef());
  ArrayData* container = getContainer();
  assertx(getData() != nullptr && container != getData());
  if (container != nullptr) {
    freeMArrayIter(this);
  }
  assertx(getContainer() == nullptr);
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

template <bool Local>
bool Iter::init(TypedValue* c1) {
  assertx(!isRefType(c1->m_type));
  bool hasElems = true;
  if (isArrayLikeType(c1->m_type)) {
    if (!c1->m_data.parr->empty()) {
      if (Local) {
        (void) new (&arr()) ArrayIter(c1->m_data.parr, ArrayIter::local);
      } else {
        (void) new (&arr()) ArrayIter(c1->m_data.parr);
      }
      arr().setIterType(ArrayIter::TypeArray);
    } else {
      hasElems = false;
    }
  } else if (c1->m_type == KindOfObject) {
    bool isIterator;
    if (c1->m_data.pobj->isCollection()) {
      isIterator = false;
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

template bool Iter::init<false>(TypedValue*);
template bool Iter::init<true>(TypedValue*);

bool Iter::next() {
  assertx(arr().getIterType() == ArrayIter::TypeArray ||
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

bool Iter::nextLocal(const ArrayData* ad) {
  assertx(arr().getIterType() == ArrayIter::TypeArray);
  assertx(!arr().getArrayData());
  auto ai = &arr();
  if (ai->nextLocal(ad)) {
    ai->~ArrayIter();
    return false;
  }
  return true;
}

void Iter::free() {
  assertx(arr().getIterType() == ArrayIter::TypeArray ||
          arr().getIterType() == ArrayIter::TypeIterator);
  arr().~ArrayIter();
}

void Iter::mfree() {
  marr().~MArrayIter();
}

void Iter::cfree() {
  cuf().~CufIter();
}

/*
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */

template <bool typeArray, bool withRef>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assertx(withRef || !isRefType(oldVal.m_type));
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  assertx((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    auto const cur = arrIter.nvSecond();
    if (isRefType(cur.type())) {
      if (!withRef || !cur.val().pref->isReferenced()) {
        cellDup(*(cur.val().pref->cell()), *out);
      } else {
        refDup(cur.tv(), *out);
      }
    } else {
      cellDup(cur.tv(), *out);
    }
  } else {
    Variant val = arrIter.second();
    assertx(!isRefType(val.getRawType()));
    cellDup(*val.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

template <bool typeArray, bool withRef>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assertx(withRef || !isRefType(oldVal.m_type));
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  assertx((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
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

namespace {

inline void liter_value_cell_local_impl(Iter* iter,
                                        TypedValue* out,
                                        const ArrayData* ad) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  auto const& arrIter = iter->arr();
  assertx(arrIter.getIterType() == ArrayIter::TypeArray);
  assertx(!arrIter.getArrayData());
  auto const cur = arrIter.nvSecondLocal(ad);
  cellDup(tvToCell(cur.tv()), *out);
  tvDecRefGen(oldVal);
}

inline void liter_key_cell_local_impl(Iter* iter,
                                      TypedValue* out,
                                      const ArrayData* ad) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  auto const& arr = iter->arr();
  assertx(arr.getIterType() == ArrayIter::TypeArray);
  assertx(!arr.getArrayData());
  cellCopy(arr.nvFirstLocal(ad), *out);
  tvDecRefGen(oldVal);
}

}

static NEVER_INLINE
int64_t iter_next_free_packed(Iter* iter, ArrayData* arr) {
  assertx(arr->decWillRelease());
  assertx(arr->hasPackedLayout());
  // Use non-specialized release call so ArrayTracer can track its destruction
  arr->release();
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

static NEVER_INLINE
int64_t iter_next_free_mixed(Iter* iter, ArrayData* arr) {
  assertx(arr->hasMixedLayout());
  assertx(arr->decWillRelease());
  // Use non-specialized release call so ArrayTracer can track its destruction
  arr->release();
  if (debug) {
    iter->arr().setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

NEVER_INLINE
static int64_t iter_next_free_apc(Iter* iter, APCLocalArray* arr) {
  assertx(arr->decWillRelease());
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
template <bool withRef, bool Local>
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
    if (Local) {
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::local);
      dest->arr().setIterType(ArrayIter::TypeArray);
      liter_value_cell_local_impl(dest, valOut, arr);
      if (keyOut) {
        liter_key_cell_local_impl(dest, keyOut, arr);
      }
    } else {
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noInc);
      dest->arr().setIterType(ArrayIter::TypeArray);
      iter_value_cell_local_impl<true, withRef>(dest, valOut);
      if (keyOut) {
        iter_key_cell_local_impl<true, withRef>(dest, keyOut);
      }
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  if (!Local) decRefArr(arr);
  return 0LL;
}

template <bool Local>
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  if (UNLIKELY(ad->getSize() == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    } else if (debug) {
      dest->arr().setIterType(ArrayIter::TypeUndefined);
    }
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<false, Local>(dest, ad, valOut, nullptr);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = Local ? nullptr : ad;
  auto const itypeU32 = static_cast<uint32_t>(ArrayIter::TypeArray);

  if (LIKELY(ad->hasPackedLayout())) {
    aiter.m_pos = 0;
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayPacked) << 16 | itypeU32;
    assertx(aiter.m_itype == ArrayIter::TypeArray);
    assertx(aiter.m_nextHelperIdx == IterNextIndex::ArrayPacked);
    cellDup(*tvToCell(packedData(ad)), *valOut);
    return 1;
  }

  if (LIKELY(ad->hasMixedLayout())) {
    auto const mixed = MixedArray::asMixed(ad);
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayMixed) << 16 | itypeU32;
    assertx(aiter.m_itype == ArrayIter::TypeArray);
    assertx(aiter.m_nextHelperIdx == IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  return new_iter_array_cold<false, Local>(dest, ad, valOut, nullptr);
}

template int64_t new_iter_array<false>(Iter*, ArrayData*, TypedValue*);
template int64_t new_iter_array<true>(Iter*, ArrayData*, TypedValue*);

template<bool WithRef, bool Local>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  if (UNLIKELY(ad->getSize() == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    } else if (debug) {
      dest->arr().setIterType(ArrayIter::TypeUndefined);
    }
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<WithRef, Local>(
      dest, ad, valOut, keyOut
    );
  }
  if (UNLIKELY(isRefcountedType(keyOut->m_type))) {
    return new_iter_array_cold<WithRef, Local>(
      dest, ad, valOut, keyOut
    );
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = Local ? nullptr : ad;
  auto const itypeU32 = static_cast<uint32_t>(ArrayIter::TypeArray);

  if (ad->hasPackedLayout()) {
    aiter.m_pos = 0;
    aiter.m_itypeAndNextHelperIdx =
      static_cast<uint32_t>(IterNextIndex::ArrayPacked) << 16 | itypeU32;
    assertx(aiter.m_itype == ArrayIter::TypeArray);
    assertx(aiter.m_nextHelperIdx == IterNextIndex::ArrayPacked);
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
    assertx(aiter.m_itype == ArrayIter::TypeArray);
    assertx(aiter.m_nextHelperIdx == IterNextIndex::ArrayMixed);
    if (WithRef) {
      mixed->dupArrayElmWithRef(aiter.m_pos, valOut, keyOut);
    } else {
      mixed->getArrayElm(aiter.m_pos, valOut, keyOut);
    }
    return 1;
  }

  return new_iter_array_cold<WithRef, Local>(dest, ad, valOut, keyOut);
}

template int64_t new_iter_array_key<false, true>(Iter* dest, ArrayData* ad,
                                                 TypedValue* valOut,
                                                 TypedValue* keyOut);
template int64_t new_iter_array_key<true, true>(Iter* dest, ArrayData* ad,
                                                TypedValue* valOut,
                                                TypedValue* keyOut);
template int64_t new_iter_array_key<false, false>(Iter* dest, ArrayData* ad,
                                                  TypedValue* valOut,
                                                  TypedValue* keyOut);
template int64_t new_iter_array_key<true, false>(Iter* dest, ArrayData* ad,
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
    iter_value_cell_local_impl<false, false>(dest, tvToCell(valOut));
    if (keyOut) {
      iter_key_cell_local_impl<false, false>(dest, tvToCell(keyOut));
    }
  } else {
    iter_value_cell_local_impl<true, false>(dest, tvToCell(valOut));
    if (keyOut) {
      iter_key_cell_local_impl<true, false>(dest, tvToCell(keyOut));
    }
  }
  return 1LL;
}

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator or Object\n",
        __func__, dest, obj, ctx);
  if (UNLIKELY(!obj->isCollection())) {
    return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  }

  if (auto ad = collections::asArray(obj)) {
    ad->incRefCount();
    decRefObj(obj);
    return keyOut
      ? new_iter_array_key<false, false>(dest, ad, valOut, keyOut)
      : new_iter_array<false>(dest, ad, valOut);
  }

  assertx(obj->collectionType() == CollectionType::Pair);
  auto arr = collections::toArray(obj);
  decRefObj(obj);
  return keyOut
    ? new_iter_array_key<false, false>(dest, arr.detach(), valOut, keyOut)
    : new_iter_array<false>(dest, arr.detach(), valOut);
}

template <bool withRef>
NEVER_INLINE
int64_t iter_next_cold(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  auto const ai = &iter->arr();
  assertx(ai->getIterType() == ArrayIter::TypeArray ||
         ai->getIterType() == ArrayIter::TypeIterator);
  assertx(ai->hasArrayData() || !ai->getObject()->isCollection());
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
int64_t liter_next_cold(Iter* iter,
                        const ArrayData* ad,
                        TypedValue* valOut,
                        TypedValue* keyOut) {
  auto const ai = &iter->arr();
  assertx(ai->getIterType() == ArrayIter::TypeArray);
  assertx(!ai->getArrayData());
  if (ai->nextLocal(ad)) {
    ai->~ArrayIter();
    return 0;
  }
  liter_value_cell_local_impl(iter, valOut, ad);
  if (keyOut) liter_key_cell_local_impl(iter, keyOut, ad);
  return 1;
}

template <bool Local>
NEVER_INLINE
static int64_t iter_next_apc_array(Iter* iter,
                                   TypedValue* valOut,
                                   TypedValue* keyOut,
                                   ArrayData* ad) {
  assertx(ad->kind() == ArrayData::kApcKind);

  auto const arrIter = &iter->arr();
  auto const arr = APCLocalArray::asApcArray(ad);
  ssize_t const pos = arr->iterAdvanceImpl(arrIter->getPos());
  if (UNLIKELY(pos == ad->getSize())) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_apc(iter, arr);
      }
      arr->decRefCount();
    }
    if (debug) {
      iter->arr().setIterType(ArrayIter::TypeUndefined);
    }
    return 0;
  }
  arrIter->setPos(pos);

  // Note that APCLocalArray can never return KindOfRefs.
  auto const rval = APCLocalArray::RvalAtPos(arr->asArrayData(), pos);
  assertx(!isRefType(rval.type()));
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
  assertx(iter->arr().getIterType() == ArrayIter::TypeArray ||
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
        return iter_next_apc_array<false>(iter, valOut, keyOut, ad);
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

      if (UNLIKELY(tvDecRefWillCallHelper(*valOut)) ||
          UNLIKELY(tvDecRefWillCallHelper(*keyOut))) {
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

    if (UNLIKELY(tvDecRefWillCallHelper(*valOut)) ||
        UNLIKELY(tvDecRefWillCallHelper(*keyOut))) {
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

  auto rtv = v1->cell();
  assertx(isArrayLikeType(rtv->m_type));
  ArrayData* ad = rtv->m_data.parr;

  if (UNLIKELY(ad->isHackArray())) {
    throwRefInvalidArrayValueException(ad);
  }
  if (checkHACRefBind()) raiseHackArrCompatRefIter();

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
  ObjectData *obj = ref->cell()->m_data.pobj;
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
  tvIncRefGen(*valOut);
  return iter_next_cold<false>(it, valOut, keyOut);
}

NEVER_INLINE
int64_t liter_next_cold_inc_val(Iter* it,
                                TypedValue* valOut,
                                TypedValue* keyOut,
                                const ArrayData* ad) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(*valOut);
  return liter_next_cold(it, ad, valOut, keyOut);
}

template<bool HasKey, bool Local>
ALWAYS_INLINE
int64_t iter_next_mixed_impl(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut,
                             ArrayData* arrData) {
  ArrayIter& iter    = it->arr();
  auto const arr     = MixedArray::asMixed(arrData);
  ssize_t pos        = iter.getPos();

  do {
    if (size_t(++pos) >= size_t(arr->iterLimit())) {
      if (!Local) {
        if (UNLIKELY(arr->decWillRelease())) {
          return iter_next_free_mixed(it, arr->asArrayData());
        }
        arr->decRefCount();
      }
      if (debug) {
        iter.setIterType(ArrayIter::TypeUndefined);
      }
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold(it, arrData, valOut, keyOut)
        : iter_next_cold<false>(it, valOut, keyOut);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold_inc_val(it, valOut, keyOut, arrData)
        : iter_next_cold_inc_val(it, valOut, keyOut);
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

template<bool HasKey, bool Local>
int64_t iter_next_packed_impl(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut,
                              ArrayData* ad) {
  auto& iter = it->arr();
  assertx(PackedArray::checkInvariants(ad));

  ssize_t pos = iter.getPos() + 1;
  if (LIKELY(pos < ad->getSize())) {
    if (isRefcountedType(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold(it, ad, valOut, keyOut)
          : iter_next_cold<false>(it, valOut, keyOut);
      }
      valOut->m_data.pcnt->decRefCount();
    }
    if (HasKey && UNLIKELY(isRefcountedType(keyOut->m_type))) {
      if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold_inc_val(it, valOut, keyOut, ad)
          : iter_next_cold_inc_val(it, valOut, keyOut);
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
  if (!Local) {
    if (UNLIKELY(ad->decWillRelease())) {
      return iter_next_free_packed(it, ad);
    }
    ad->decRefCount();
  }
  if (debug) {
    iter.setIterType(ArrayIter::TypeUndefined);
  }
  return 0;
}

}

int64_t iterNextArrayPacked(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_packed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayPacked(Iter* it,
                             TypedValue* valOut,
                             ArrayData* ad) {
  TRACE(2, "literNextArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasPackedLayout());
  return iter_next_packed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayPacked(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_packed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayPacked(Iter* it,
                              TypedValue* valOut,
                              TypedValue* keyOut,
                              ArrayData* ad) {
  TRACE(2, "literNextKArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasPackedLayout());
  return iter_next_packed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArrayMixed(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayMixed(Iter* it,
                            TypedValue* valOut,
                            ArrayData* ad) {
  TRACE(2, "literNextArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasMixedLayout());
  return iter_next_mixed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayMixed(Iter* it,
                            TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "iterNextKArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayMixed(Iter* it,
                             TypedValue* valOut,
                             TypedValue* keyOut,
                             ArrayData* ad) {
  TRACE(2, "literNextKArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasMixedLayout());
  return iter_next_mixed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArray(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assertx(!it->arr().getArrayData()->hasPackedLayout());
  assertx(!it->arr().getArrayData()->hasMixedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array<false>(it, valOut, nullptr, ad);
  }
  return iter_next_cold<false>(it, valOut, nullptr);
}

int64_t literNextArray(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  // NB: We could have an APC local array which then promotes to a packed/mixed
  // array, so we can't assert that the array isn't packed or mixed layout here.
  if (ad->isApcArray()) {
    return iter_next_apc_array<true>(it, valOut, nullptr, ad);
  }
  return liter_next_cold(it, ad, valOut, nullptr);
}

int64_t iterNextKArray(Iter* it,
                       TypedValue* valOut,
                       TypedValue* keyOut) {
  TRACE(2, "iterNextKArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assertx(!it->arr().getArrayData()->hasMixedLayout());
  assertx(!it->arr().getArrayData()->hasPackedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array<false>(it, valOut, keyOut, ad);
  }
  return iter_next_cold<false>(it, valOut, keyOut);
}

int64_t literNextKArray(Iter* it,
                        TypedValue* valOut,
                        TypedValue* keyOut,
                        ArrayData* ad) {
  TRACE(2, "literNextKArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  // NB: We could have an APC local array which then promotes to a packed/mixed
  // array, so we can't assert that the array isn't packed or mixed layout here.
  if (ad->isApcArray()) {
    return iter_next_apc_array<true>(it, valOut, keyOut, ad);
  }
  return liter_next_cold(it, ad, valOut, keyOut);
}

int64_t iterNextObject(Iter* it, TypedValue* valOut) {
  TRACE(2, "iterNextObject: I %p\n", it);
  // We can't just put the address of iter_next_cold in the table
  // below right now because we need to get a nullptr into the third
  // argument register for it.
  return iter_next_cold<false>(it, valOut, nullptr);
}

int64_t literNextObject(Iter*, TypedValue*, ArrayData*) {
  always_assert(false);
}
int64_t literNextKObject(Iter*, TypedValue*, TypedValue*, ArrayData*) {
  always_assert(false);
}

using IterNextHelper  = int64_t (*)(Iter*, TypedValue*);
using IterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*);

using LIterNextHelper  = int64_t (*)(Iter*, TypedValue*, ArrayData*);
using LIterNextKHelper = int64_t (*)(Iter*, TypedValue*,
                                     TypedValue*, ArrayData*);

const IterNextHelper g_iterNextHelpers[] = {
  &iterNextArrayPacked,
  &iterNextArrayMixed,
  &iterNextArray,
  &iterNextObject,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  &iterNextKArrayPacked,
  &iterNextKArrayMixed,
  &iterNextKArray,
  &iter_next_cold<false>, // iterNextKObject
};

const LIterNextHelper g_literNextHelpers[] = {
  &literNextArrayPacked,
  &literNextArrayMixed,
  &literNextArray,
  &literNextObject,
};

const LIterNextKHelper g_literNextKHelpers[] = {
  &literNextKArrayPacked,
  &literNextKArrayMixed,
  &literNextKArray,
  &literNextKObject
};

int64_t iter_next_ind(Iter* iter, TypedValue* valOut) {
  TRACE(2, "iter_next_ind: I %p\n", iter);
  assertx(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  IterNextHelper iterNext =
      g_iterNextHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return iterNext(iter, valOut);
}

int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "iter_next_key_ind: I %p\n", iter);
  assertx(iter->arr().getIterType() == ArrayIter::TypeArray ||
         iter->arr().getIterType() == ArrayIter::TypeIterator);
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  IterNextKHelper iterNextK =
      g_iterNextKHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return iterNextK(iter, valOut, keyOut);
}

int64_t liter_next_ind(Iter* iter, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "liter_next_ind: I %p\n", iter);
  assertx(iter->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!iter->arr().getArrayData());
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  LIterNextHelper literNext =
      g_literNextHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return literNext(iter, valOut, ad);
}

int64_t liter_next_key_ind(Iter* iter,
                           TypedValue* valOut,
                           TypedValue* keyOut,
                           ArrayData* ad) {
  TRACE(2, "liter_next_key_ind: I %p\n", iter);
  assertx(iter->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!iter->arr().getArrayData());
  auto const arrIter = &iter->arr();
  valOut = tvToCell(valOut);
  keyOut = tvToCell(keyOut);
  LIterNextKHelper literNextK =
      g_literNextKHelpers[static_cast<uint32_t>(arrIter->getHelperIndex())];
  return literNextK(iter, valOut, keyOut, ad);
}

///////////////////////////////////////////////////////////////////////////////
}
