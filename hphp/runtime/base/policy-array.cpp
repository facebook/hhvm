/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/policy-array.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/base/sort-helpers.h"
#include "folly/Foreach.h"

TRACE_SET_MOD(runtime);
#define MYLOG if (true) {} else LOG(INFO)
#define APILOG(a) MYLOG << "{" << (a) << ":m_size=" << (a)->m_size    \
  << ";cap=" << (a)->capacity() << ";m_pos=" << (a)->m_pos << "}->" \
  << __FUNCTION__

namespace HPHP {

static string keystr(const StringData* key) {
  return "s:" + string(key->data(), key->size());
}
static string keystr(int64_t key) {
  return "i:" + std::to_string(key);
}
static string valstr(const Variant& v) {
  try {
    auto result = v.toString();
    return string(result.data(), result.size());
  } catch (...) {
    return "<messedup>";
  }
}

SimpleArrayStore::SimpleArrayStore(const SimpleArrayStore& rhs,
                                   uint length, uint capacity,
                                   ArrayData::AllocationMode am,
                                   const ArrayData* owner)
    : m_capacity(std::max<uint>(startingCapacity, capacity))
    , m_nextKey(rhs.m_nextKey) {
  assert(length <= capacity && this != &rhs);
  allocate(m_keys, m_vals, m_capacity, am);
  // Copy data with flattening
  FOR_EACH_RANGE (i, 0, length) {
    tvDupFlattenVars(rhs.m_vals + i, m_vals + i, owner);
    if (rhs.hasStrKey(toPos(i))) {
      setKey(toPos(i), rhs.m_keys[i].s);
    } else {
      setKey(toPos(i), rhs.m_keys[i].i);
    }
  }
}

void SimpleArrayStore::grow(uint length, uint minCap, uint idealCap,
                            ArrayData::AllocationMode am) {
  assert(idealCap >= minCap);
  if (m_capacity >= minCap) return;
  MYLOG << (void*)this << "->grow(" << length << ", " << minCap << ", "
        << idealCap << ", " << uint(am) << "); m_capacity=" << m_capacity;
  idealCap = std::max<uint>(startingCapacity, idealCap);
  Key* newKeys;
  TypedValueAux* newVals;
  allocate(newKeys, newVals, idealCap, am);
  // Move data
  memcpy(newKeys, m_keys, length * sizeof(*m_keys));
  memcpy(newVals, m_vals, length * sizeof(*m_vals));
  deallocate(m_keys, m_vals, am);
  // Change state
  m_capacity = idealCap;
  m_keys = newKeys;
  m_vals = newVals;
}

void SimpleArrayStore::destroy(uint length, ArrayData::AllocationMode am) {
  FOR_EACH_RANGE (i, 0, length) {
    if (hasStrKey(toPos(i))) {
      auto k = m_keys[i].s;
      assert(k);
      decRefStr(k);
    }
    lval(toPos(i)).~Variant();
  }
  deallocate(m_keys, m_vals, am);
#ifndef NDEBUG
  m_keys = nullptr;
  m_vals = nullptr;
#endif
}

PosType SimpleArrayStore::find(int64_t key, uint length) const {
  assert(m_keys && length <= m_capacity);
  // glorious linear find
  for (uint i = 0; i < length; ++i) {
    if (key == m_keys[i].i && !hasStrKey(toPos(i))) {
      return toPos(i);
    }
  }
  return PosType::invalid;
}

PosType SimpleArrayStore::find(const StringData* key, uint length) const {
  // glorious linear find
  assert(key && m_keys && length <= m_capacity);
  for (uint i = 0; i < length; ++i) {
    if (!hasStrKey(toPos(i))) continue;
    auto const k = m_keys[i].s;
    if (key == k || key->same(k)) return toPos(i);
  }
  return PosType::invalid;
}

template <class K>
bool SimpleArrayStore::update(K key, const Variant& val, uint length,
                              ArrayData::AllocationMode am) {
  assert(length <= m_capacity && m_vals);
  auto const pos = find(key, length);
  if (pos != PosType::invalid) {
    // found, overwrite
    assert(tvIsPlausible(m_vals[toInt<uint32_t>(pos)]));
    lval(pos) = val;
    return false;
  }
  // not found, insert
  assert(length <= m_capacity);
  if (length == m_capacity) {
    grow(length, length + 1, length * 2 + 1, am);
  }
  assert(m_keys && m_vals && length < m_capacity);
  new(&lval(toPos(length))) Variant(val);
  setKey(toPos(length), key);
  return true;
}

void SimpleArrayStore::erase(PosType pos, uint length) {
  auto const ipos = toInt<uint32_t>(pos);
  assert(ipos < length && length <= capacity());
  // Destroy data at pos
  if (hasStrKey(pos)) {
    auto const k = m_keys[ipos].s;
    assert(k);
    decRefStr(k);
  }
  lval(pos).~Variant();
  // Shift over memory
  auto const itemsToMove = length - ipos - 1;
  memmove(m_keys + ipos, m_keys + ipos + 1, itemsToMove * sizeof(*m_keys));
  memmove(m_vals + ipos, m_vals + ipos + 1, itemsToMove * sizeof(*m_vals));
}

void SimpleArrayStore::prepend(const Variant& v, uint length,
                               ArrayData::AllocationMode am) {
  if (length == capacity()) {
    grow(length, length + 1, length * 2 + 1, am);
  }
  assert(length < capacity());
  // Shift stuff over
  memmove(m_keys + 1, m_keys, length * sizeof(*m_keys));
  memmove(m_vals + 1, m_vals, length * sizeof(*m_vals));
  // Construct the new value
  new(m_vals) Variant(v);
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION(PolicyArray)

PolicyArray::PolicyArray(uint capacity)
    : ArrayData(kPolicyKind)
    , Store(m_allocMode, capacity) {
  m_size = 0;
  m_pos = invalid_index;
  // Log at the end of the ctor so as to show the properly initialized
  // members.
  APILOG(this) << "(" << capacity << ");";
}

PolicyArray::PolicyArray(const PolicyArray& rhs, uint capacity,
                       AllocationMode am)
    : ArrayData(kPolicyKind, am)
    , Store(rhs, rhs.m_size, capacity, am, &rhs) {
  m_size = rhs.m_size;
  m_pos = rhs.m_pos;
  // Log at the end of the ctor so as to show the properly initialized
  // members.
  APILOG(this) << "(" << &rhs << ", " << capacity << ", " << uint(am) << ");";
}

PolicyArray::~PolicyArray() {
  APILOG(this) << "()";
  SimpleArrayStore::destroy(m_size, m_allocMode);
}

void PolicyArray::Release(ArrayData* ad) {
  asPolicyArray(ad)->release();
}

inline PolicyArray* PolicyArray::asPolicyArray(ArrayData* ad) {
  assert(ad->kind() == kPolicyKind);
  return static_cast<PolicyArray*>(ad);
}

inline const PolicyArray* PolicyArray::asPolicyArray(const ArrayData* ad) {
  assert(ad->kind() == kPolicyKind);
  return static_cast<const PolicyArray*>(ad);
}

const Variant& PolicyArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << pos << ")";
  assert(size_t(pos) < a->m_size);
  return a->val(toPos(pos));
}

bool PolicyArray::IsVectorData(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  for (ssize_t i = 0; i < a->m_size; ++i) {
    if (a->Store::find(i, a->m_size) != toPos(i)) return false;
  }
  return true;
}

bool PolicyArray::ExistsInt(const ArrayData* ad, int64_t k) {
  auto a = asPolicyArray(ad);
  return a->Store::find(k, a->m_size) < toPos(a->m_size);
}

bool PolicyArray::ExistsStr(const ArrayData* ad, const StringData* k) {
  auto a = asPolicyArray(ad);
  return a->Store::find(k, a->m_size) < toPos(a->m_size);
}

static_assert(ArrayData::invalid_index == size_t(-1), "ehm");

template <class K>
TypedValue* PolicyArray::nvGetImpl(K k) const {
  APILOG(this) << "(" << keystr(k) << ")";
  auto const pos = find(k, m_size);
  return LIKELY(pos != PosType::invalid)
    ? reinterpret_cast<TypedValue*>(&lval(pos))
    : nullptr;
}

TypedValue* PolicyArray::NvGetInt(const ArrayData* ad, int64_t k) {
  return asPolicyArray(ad)->nvGetImpl(k);
}

TypedValue* PolicyArray::NvGetStr(const ArrayData* ad, const StringData* k) {
  return asPolicyArray(ad)->nvGetImpl(k);
}

void PolicyArray::NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << out << ", " << pos << ")";
  assert(size_t(pos) < a->m_size);
  new(out) Variant(a->key(toPos(pos)));
}

template <class K>
ArrayData *PolicyArray::lvalImpl(K k, Variant*& ret, bool copy) {
  APILOG(this) << "(" << keystr(k) << ", " << ret << ", "
         << copy << ", " << ")";

  if (copy) {
    return asPolicyArray(Copy(this))->lvalImpl(k, ret, false);
  }

  PosType pos = find(k, m_size);
  if (pos != PosType::invalid) {
    // found, don't overwrite anything
    assert(toInt<uint32_t>(pos) <= m_size);
    ret = &lval(pos);
    MYLOG << (void*)this << "->lvalImpl:" << "found at " << toInt<int64_t>(pos)
          << ", value=" << valstr(*ret) << ", size=" << m_size;
  } else {
    // not found, initialize
    if (m_size == capacity()) {
      grow(m_size, m_size + 1, m_size * 2 + 1, m_allocMode);
    }
    assert(m_size < capacity());
    ret = appendNoGrow(k, Variant::NullInit());
  }

  return this;
}

ArrayData* PolicyArray::LvalInt(ArrayData* ad, int64_t k, Variant *&ret,
                                bool copy) {
  return asPolicyArray(ad)->lvalImpl(k, ret, copy);
}

ArrayData* PolicyArray::LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                                bool copy) {
  return asPolicyArray(ad)->lvalImpl(k, ret, copy);
}

ArrayData *PolicyArray::LvalNew(ArrayData* ad, Variant *&ret, bool copy) {
  auto a = asPolicyArray(ad);
  if (copy) a = asPolicyArray(Copy(a));

  // Andrei: TODO - append() currently never fails, probably it
  // should.
  auto oldSize = a->m_size;
  a->append(uninit_null(), false);
  assert(a->m_size == oldSize + 1);
  if (UNLIKELY(oldSize == a->m_size)) {
    ret = &Variant::lvalBlackHole();
  } else {
    assert(a->lastIndex(a->m_size) != PosType::invalid);
    ret = &a->lval(a->lastIndex(a->m_size));
  }
  return a;
}

template <class K>
PolicyArray* PolicyArray::setImpl(K k, const Variant& v, bool copy) {
  APILOG(this) << "(" << keystr(k) << ", " << valstr(v) << ", " << copy
         << ")";
  PolicyArray* result = this;
  if (copy) result = asPolicyArray(Copy(this));
  if (result->update(k, v, result->m_size, result->m_allocMode)) {
    // Added a new element, must update size and possibly m_pos
    if (m_pos == invalid_index) m_pos = result->m_size;
    result->m_size++;
  }
  return result;
}

ArrayData*
PolicyArray::SetInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->setImpl(k, v, copy);
}

ArrayData*
PolicyArray::SetStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->setImpl(k, v, copy);
}

template <class K>
ArrayData *PolicyArray::setRefImpl(K k, CVarRef v, bool copy) {
  APILOG(this) << "(" << keystr(k) << ", " << valstr(v) << ", " << copy << ")";

  if (copy) {
    return asPolicyArray(Copy(this))->setRef(k, v, false);
  }

  auto const pos = find(k, m_size);
  assert(m_size <= capacity());
  if (pos != PosType::invalid) {
    // found, update
    lval(pos).assignRef(v);
  } else {
    // not found, create new element
    MYLOG << "setRef: not found, appending at " << m_size;
    if (m_size == capacity()) {
      MYLOG << "grow";
      grow(m_size, m_size + 1, m_size * 2 + 1, m_allocMode);
    }
    appendNoGrow(k, Variant::NoInit())->constructRefHelper(v);
  }
  return this;
}

ArrayData*
PolicyArray::SetRefInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->setRefImpl(k, v, copy);
}

ArrayData*
PolicyArray::SetRefStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->setRefImpl(k, v, copy);
}

template <class K>
ArrayData *PolicyArray::addImpl(K k, const Variant& v, bool copy) {
  APILOG(this) << "(" << keystr(k) << ", " << valstr(v) << ", " << copy << ");";
  if (copy) {
    auto result = PolicyArray::copy(m_size * 2 + 1);
    result->add(k, v, false);
    return result;
  }
  assert(!exists(k));
  // Make sure there's enough capacity
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_allocMode);
  }
  appendNoGrow(k, v);
  return this;
}

ArrayData*
PolicyArray::AddInt(ArrayData* ad, int64_t k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->addImpl(k, v, copy);
}

ArrayData*
PolicyArray::AddStr(ArrayData* ad, StringData* k, CVarRef v, bool copy) {
  return asPolicyArray(ad)->addImpl(k, v, copy);
}

template <class K>
ArrayData *PolicyArray::removeImpl(K k, bool copy) {
  APILOG(this) << "(" << keystr(k) << ", " << copy << ")";

  if (copy) {
    return asPolicyArray(Copy(this))->remove(k, false);
  }

  auto const pos = find(k, m_size);
  if (pos == PosType::invalid) {
    // Not found, nothing to delete
    MYLOG << "not found, nothing to delete: " << keystr(k);
    return this;
  }

  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos& fp = *r.front();
    if (ssize_t(pos) <= fp.m_pos) {
      // We are removing something before or at the current position,
      // back off position to account for the shifting.
      if (!fp.m_pos) fp.setResetFlag(true);
      else --fp.m_pos;
    }
  }

  Store::erase(pos, m_size);
  --m_size;

  if (!Store::before(m_pos, pos)) {
    // We removed something before or at the current position, back
    // off position to account for the shifting.
    m_pos = ssize_t(prevIndex(toPos(m_pos), m_size));
  }

  assert(size_t(m_pos) < m_size || m_pos == invalid_index);
  return this;
}

ArrayData*
PolicyArray::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  return asPolicyArray(ad)->removeImpl(k, copy);
}

ArrayData*
PolicyArray::RemoveStr(ArrayData* ad, const StringData* k, bool copy) {
  return asPolicyArray(ad)->removeImpl(k, copy);
}

ssize_t PolicyArray::IterBegin(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  return a->m_size ? toInt<int64_t>(a->firstIndex(a->m_size)) : invalid_index;
}

ssize_t PolicyArray::IterEnd(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  return ssize_t(a->lastIndex(a->m_size));
}

ssize_t PolicyArray::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << prev << ")";
  auto const result = toInt<int64_t>(a->nextIndex(toPos(prev), a->m_size));
  MYLOG << "returning " << result;
  return result;
}

ssize_t PolicyArray::IterRewind(const ArrayData* ad, ssize_t prev) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << prev << ")";
  return toInt<int64_t>(a->prevIndex(toPos(prev), a->m_size));
}

bool PolicyArray::ValidFullPos(const ArrayData* ad, const FullPos& fp) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << fp.m_pos << ";" << fp.getResetFlag() << ")";
  assert(fp.getContainer() == a);
  return fp.m_pos != invalid_index;
}

bool PolicyArray::AdvanceFullPos(ArrayData* ad, FullPos &fp) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << fp.m_pos << ";" << fp.getResetFlag() << ")";
  assert(fp.getContainer() == a);
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = invalid_index;
  } else if (fp.m_pos == invalid_index) {
    return false;
  }
  fp.m_pos = toInt<int64_t>(a->nextIndex(toPos(fp.m_pos), a->m_size));
  if (fp.m_pos == invalid_index) {
    return false;
  }
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  a->m_pos = toInt<int64_t>(a->nextIndex(toPos(fp.m_pos), a->m_size));
  return true;
}

HphpArray* PolicyArray::toHphpArray() const {
  auto result = ArrayData::Make(m_size);
  FOR_EACH_RANGE (i, 0, m_size) {
    if (hasStrKey(toPos(i))) {
      result->add(key(toPos(i)).getStringData(), val(toPos(i)), false);
    } else {
      result->add(key(toPos(i)).getInt64(), val(toPos(i)), false);
    }
  }
  return result;
}

ArrayData* PolicyArray::EscalateForSort(ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  return a->toHphpArray();
}

ArrayData* PolicyArray::Copy(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  auto result = NEW(PolicyArray)(*a,
      a->capacity() + (a->m_size == a->capacity()), a->m_allocMode);
  assert(result->getCount() == 0);
  return result;
}

PolicyArray* PolicyArray::copy(uint capacity) {
  APILOG(this) << "(" << capacity << ")";
  return NEW(PolicyArray)(*this, capacity, m_allocMode);
}

ArrayData* PolicyArray::CopyWithStrongIterators(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  auto result = Copy(a);
  moveStrongIterators(result, const_cast<PolicyArray*>(a));
  assert(result->getCount() == 0);
  return result;
}

ArrayData* PolicyArray::NonSmartCopy(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  return a->toHphpArray()->nonSmartCopy();
}

ArrayData* PolicyArray::Append(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << valstr(v) << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));
  a->grow(a->m_size, a->m_size + 1, a->m_size * 2 + 1, a->m_allocMode);
  a->appendNoGrow(a->nextKeyBump(), v);
  return a;
}

ArrayData* PolicyArray::AppendRef(ArrayData* ad, const Variant& v, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << valstr(v) << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));
  auto const k = a->nextKeyBump();
  if (a->m_size == a->capacity()) {
    a->grow(a->m_size, a->m_size + 1, a->m_size * 2 + 1, a->m_allocMode);
  }
  assert(a->m_size < a->capacity());
  a->appendNoGrow(k, Variant::NoInit())->constructRefHelper(v);
  return a;
}

  /**
   * Similar to append(v, copy), with reference in v preserved.
   */
ArrayData* PolicyArray::AppendWithRef(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << valstr(v) << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));
  if (a->m_size == a->capacity()) {
    a->grow(a->m_size, a->m_size + 1, a->m_size * 2 + 1, a->m_allocMode);
  }
  assert(a->m_size < a->capacity());
  a->appendNoGrow(a->nextKeyBump(), Variant::NullInit())->setWithRef(v);
  return a;
}

template <class K>
void PolicyArray::addValWithRef(K k, const Variant& v) {
  MYLOG << (void*)this << "->addValWithRef("
        << keystr(k) << ", " << valstr(v)
        << "); size=" << m_size;
  auto pos = find(k, m_size);
  if (pos != PosType::invalid) {
    return;
  }
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_allocMode);
  }
  assert(m_size < capacity());
  appendNoGrow(k, Variant::NullInit())->setWithRef(v);
}

void PolicyArray::nextInsertWithRef(const Variant& v) {
  MYLOG << (void*)this << "->nextInsertWithRef("
        << valstr(v)
        << "); size=" << m_size;
  // We need to define k here (before the if/grow) because otherwise
  // the overzealous gcc issues a spurious warning as such:
  //
  // hphp/runtime/base/policy-array.h: In member function 'void
  // HPHP::PolicyArray::nextInsertWithRef(const HPHP::Variant&)':
  // hphp/runtime/base/policy-array.h:114:5: error: assuming
  // signed overflow does not occur when assuming that (X + c) < X is
  // always false [-Werror=strict-overflow]
  auto const k = nextKeyBump();
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_allocMode);
  }
  assert(m_size < capacity());
  appendNoGrow(k, Variant::NullInit())->setWithRef(v);
}

ArrayData*
PolicyArray::Plus(ArrayData* ad, const ArrayData *elems, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << elems << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));

  assert(elems);
  a->grow(a->m_size, a->m_size + 1, a->m_size * 2 + 1, a->m_allocMode);

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();
    if (key.isNumeric()) {
      a->addValWithRef(key.toInt64(), value);
    } else {
      a->addValWithRef(key.getStringData(), value);
    }
  }
  return a;
}

ArrayData*
PolicyArray::Merge(ArrayData* ad, const ArrayData *elems, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << elems << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));

  assert(elems);
  a->grow(a->m_size, a->m_size + 1, a->m_size * 2 + 1, a->m_allocMode);

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();
    if (key.isNumeric()) {
      a->nextInsertWithRef(value);
    } else {
      StringData *s = key.getStringData();
      Variant *p;
      // Andrei TODO: make sure this is the right semantics
      LvalStr(a, s, p, false);
      p->setWithRef(value);
    }
  }
  return a;
}

/**
 * Stack function: pop the last item and return it.
 */
ArrayData* PolicyArray::Pop(ArrayData* ad, Variant &value) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << &value << ")";
  if (a->getCount() > 1) a = asPolicyArray(Copy(a));
  if (!a->m_size) {
    value = uninit_null();
    return a;
  }
  auto pos = a->lastIndex(a->m_size);
  assert(size_t(pos) < a->m_size);
  value = a->val(pos);

  // Match PHP 5.3.1 semantics
  if (!a->hasStrKey(pos)
      && a->Store::nextKey() == 1 + a->key(pos).toInt64()) {
    a->nextKeyPop();
  }

  a->Store::erase(pos, a->m_size);
  --a->m_size;
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->m_size ? toInt<int64_t>(a->firstIndex(a->m_size)) :
             invalid_index;
  return a;
}

ArrayData* PolicyArray::Dequeue(ArrayData* ad, Variant &value) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << &value << ")";
  if (a->getCount() > 1) a = asPolicyArray(Copy(ad));

  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  a->freeStrongIterators();
  if (!a->m_size) {
    value = uninit_null();
    return a;
  }

  auto& front = a->lval(a->firstIndex(a->m_size));
  value = std::move(front);
  new(&front) Variant;
  a->erase(a->firstIndex(a->m_size), a->m_size);
  --a->m_size;
  a->renumber();

  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  a->m_pos = a->m_size ? toInt<int64_t>(a->firstIndex(a->m_size)) :
             invalid_index;
  return a;
}

ArrayData* PolicyArray::Prepend(ArrayData* ad, CVarRef v, bool copy) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "(" << valstr(v) << ", " << copy << ")";
  if (copy) a = asPolicyArray(Copy(a));
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  a->freeStrongIterators();
  a->Store::prepend(v, a->m_size, a->m_allocMode);
  ++a->m_size;
  auto first = a->firstIndex(a->m_size);
  a->setKey(first, int64_t(0));
  a->renumber();
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  a->m_pos = toInt<int64_t>(first);
  return a;
}

void PolicyArray::renumber() {
  APILOG(this) << "()";
  if (!m_size) {
    return;
  }

  Variant currentPosKey;
  if (m_pos != invalid_index) {
    // Cache key for element associated with m_pos in order to update m_pos
    // below.
    assert(size_t(m_pos) < m_size);
    currentPosKey = key(toPos(m_pos));
  }

  vector<Variant> siKeys;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    auto const pos = toPos(r.front()->m_pos);
    if (pos != PosType::invalid) {
      siKeys.push_back(key(pos));
    }
  }
  nextKeyReset();
  FOR_EACH_RANGE (i, 0, m_size) {
    if (!hasStrKey(toPos(i))) {
      setKey(toPos(i), nextKeyBump());
    }
  }
  if (m_pos != invalid_index) {
    // Update m_pos, now that compaction is complete.
    if (currentPosKey.isString()) {
      m_pos = toInt<int64_t>(find(currentPosKey.getStringData(), m_size));
    } else if (currentPosKey.is(KindOfInt64)) {
      m_pos = toInt<int64_t>(find(currentPosKey.getInt64(), m_size));
    } else {
      assert(false);
    }
  }

  // Update strong iterators, now that compaction is complete.
  auto i = siKeys.cbegin();
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->m_pos == invalid_index) {
      continue;
    }
    auto& k = *i++;
    if (k.isString()) {
      fp->m_pos = toInt<int64_t>(find(k.getStringData(), m_size));
    } else {
      assert(k.is(KindOfInt64));
      fp->m_pos = toInt<int64_t>(find(k.getInt64(), m_size));
    }
  }
  assert(i == siKeys.cend());
}

void PolicyArray::Renumber(ArrayData* ad) {
  return asPolicyArray(ad)->renumber();
}

void PolicyArray::OnSetEvalScalar(ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  //FOR_EACH_RANGE (pos, 0, m_size) {
  for (auto pos = a->firstIndex(a->m_size); pos != PosType::invalid;
       pos = a->nextIndex(pos, a->m_size)) {
    if (a->hasStrKey(pos)) {
      auto k = a->key(pos).getStringData();
      if (!k->isStatic()) {
        auto sk = StringData::GetStaticString(k);
        decRefStr(k);
        // Andrei TODO: inefficient, does one incref and then decref
        a->setKey(pos, sk);
        sk->decRefCount();
      }
    }
    a->lval(pos).setEvalScalar();
  }
}

ArrayData *PolicyArray::Escalate(const ArrayData* ad) {
  auto a = asPolicyArray(ad);
  APILOG(a) << "()";
  return ArrayData::Escalate(a);
}

} // namespace HPHP
