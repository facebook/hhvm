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

#include "hphp/runtime/base/array/policy_array.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/array/hphp_array.h"
#include "hphp/runtime/base/array/sort_helpers.h"
#include "folly/Foreach.h"

TRACE_SET_MOD(runtime);
#define MYLOG if (true) {} else LOG(INFO)
#define APILOG MYLOG << "{" << this << ":m_size=" << this->m_size     \
  << ";cap=" << this->capacity() << ";m_pos=" << this->m_pos << "}->" \
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
                                   bool nonSmart, const ArrayData* owner)
    : m_capacity(std::max<uint>(startingCapacity, capacity))
    , m_nextKey(rhs.m_nextKey) {
  assert(length <= capacity && this != &rhs);
  allocate(m_keys, m_vals, m_capacity, nonSmart);
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
                            bool nonSmart) {
  assert(idealCap >= minCap);
  if (m_capacity >= minCap) return;
  MYLOG << (void*)this << "->grow(" << length << ", " << minCap << ", "
        << idealCap << ", " << nonSmart << "); m_capacity=" << m_capacity;
  idealCap = std::max<uint>(startingCapacity, idealCap);
  Key* newKeys;
  TypedValueAux* newVals;
  allocate(newKeys, newVals, idealCap, nonSmart);
  // Move data
  memcpy(newKeys, m_keys, length * sizeof(*m_keys));
  memcpy(newVals, m_vals, length * sizeof(*m_vals));
  deallocate(m_keys, m_vals, nonSmart);
  // Change state
  m_capacity = idealCap;
  m_keys = newKeys;
  m_vals = newVals;
}

void SimpleArrayStore::destroy(uint length, bool nonSmart) {
  FOR_EACH_RANGE (i, 0, length) {
    if (hasStrKey(toPos(i))) {
      auto k = m_keys[i].s;
      assert(k);
      if (!k->decRefCount()) DELETE(StringData)(k);
    }
    lval(toPos(i)).~Variant();
  }
  deallocate(m_keys, m_vals, nonSmart);
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
  auto const d0 = key->data();
  auto const sz = key->size();
  for (uint i = 0; i < length; ++i) {
    if (!hasStrKey(toPos(i))) continue;
    auto const k = m_keys[i].s;
    if (key == k) return toPos(i);
    assert(k);
    if (sz != k->size()) continue;
    auto const data = k->data();
    if (d0 == data) return toPos(i);
    assert(d0 && data);
    if (memcmp(d0, data, sz) == 0) return toPos(i);
  }
  return PosType::invalid;
}

template <class K>
bool SimpleArrayStore::update(K key, const Variant& val, uint length,
                              bool nonSmart) {
  assert(length <= m_capacity && m_vals);
  auto const pos = find(key, length);
  if (pos != PosType::invalid) {
    // found, overwrite
    assert(tvIsPlausible(m_vals + toUint32(pos)));
    lval(pos) = val;
    return false;
  }
  // not found, insert
  assert(length <= m_capacity);
  if (length == m_capacity) {
    grow(length, length + 1, length * 2 + 1, nonSmart);
  }
  assert(m_keys && m_vals && length < m_capacity);
  new(&lval(toPos(length))) Variant(val);
  setKey(toPos(length), key);
  return true;
}

void SimpleArrayStore::erase(PosType pos, uint length) {
  auto const ipos = toUint32(pos);
  assert(ipos < length && length <= capacity());
  // Destroy data at pos
  if (hasStrKey(pos)) {
    auto const k = m_keys[ipos].s;
    assert(k);
    if (!k->decRefCount()) DELETE(StringData)(k);
  }
  lval(pos).~Variant();
  // Shift over memory
  auto const itemsToMove = length - ipos - 1;
  memmove(m_keys + ipos, m_keys + ipos + 1, itemsToMove * sizeof(*m_keys));
  memmove(m_vals + ipos, m_vals + ipos + 1, itemsToMove * sizeof(*m_vals));
}

void SimpleArrayStore::prepend(const Variant& v, uint length, bool nonSmart) {
  if (length == capacity()) {
    grow(length, length + 1, length * 2 + 1, nonSmart);
  }
  assert(length < capacity());
  // Shift stuff over
  memmove(m_keys + 1, m_keys, length * sizeof(*m_keys));
  memmove(m_vals + 1, m_vals, length * sizeof(*m_vals));
  // Construct the new value
  new(m_vals) Variant(v);
}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION(ArrayShell)

ArrayShell::ArrayShell(uint capacity)
    : ArrayData(kArrayShell)
    , Store(m_nonsmart, capacity) {
  m_size = 0;
  m_pos = invalid_index;
  // Log at the end of the ctor so as to show the properly initialized
  // members.
  APILOG << "(" << capacity << ");";
}

ArrayShell::ArrayShell(const ArrayShell& rhs, uint capacity,
                           bool nonSmart)
    : ArrayData(kArrayShell, nonSmart)
    , Store(rhs, rhs.m_size, capacity, nonSmart, &rhs) {
  m_size = rhs.m_size;
  m_pos = rhs.m_pos;
  // Log at the end of the ctor so as to show the properly initialized
  // members.
  APILOG << "(" << &rhs << ", " << capacity << ", " << nonSmart << ");";
}

ArrayShell::~ArrayShell() {
  APILOG << "()";
  destroy(m_size, m_nonsmart);
}

Variant ArrayShell::getKey(ssize_t pos) const {
  APILOG << "(" << pos << ")";
  assert(size_t(pos) < m_size);
  return key(toPos(pos));
}

Variant ArrayShell::getValue(ssize_t pos) const {
  APILOG << "(" << pos << ")";
  assert(size_t(pos) < m_size);
  return getValueRef(pos);
}

const Variant& ArrayShell::getValueRef(ssize_t pos) const {
  APILOG << "(" << pos << ")";
  assert(size_t(pos) < m_size);
  return val(toPos(pos));
}

bool ArrayShell::isVectorData() const {
  APILOG << "()";
  return ArrayData::isVectorData();
}

Variant ArrayShell::reset() {
  APILOG << "()";
  if (m_size) {
    auto const first = firstIndex(m_size);
    m_pos = toUint32(first);
    return val(first);
  }
  m_pos = invalid_index;
  return false;
}

Variant ArrayShell::prev() {
  NOT_IMPLEMENTED();
}

Variant ArrayShell::current() const {
  APILOG << "()";
  if (m_pos == invalid_index) {
    return false;
  }
  assert(size_t(m_pos) < m_size);
  return val(toPos(m_pos));
}

Variant ArrayShell::next() {
  APILOG << "()";
  if (m_pos == invalid_index
      || (m_pos = iter_advance(m_pos)) == invalid_index) {
    return false;
  }
  assert(size_t(m_pos) < m_size);
  return val(toPos(m_pos));
}

Variant ArrayShell::end() {
  if (m_size) {
    auto const last = lastIndex(m_size);;
    m_pos = toUint32(last);
    return val(last);
  }
  m_pos = invalid_index;
  return false;
}

Variant ArrayShell::key() const {
  APILOG << ")";
  if (m_pos == invalid_index) {
    return uninit_null();
  }
  assert(size_t(m_pos) < m_size);
  return key(toPos(m_pos));
}

Variant ArrayShell::value(ssize_t &pos) const {
  NOT_IMPLEMENTED();
}

static const StaticString s_value("value");
static const StaticString s_key("key");
static_assert(ArrayData::invalid_index == size_t(-1), "ehm");

Variant ArrayShell::each() {
  APILOG << "()";
  if (m_pos == invalid_index) return false;
  assert(m_size);
  ArrayInit init(4);
  assert(size_t(m_pos) < m_size);
  Variant key = Store::key(toPos(m_pos));
  Variant value = val(toPos(m_pos));
  init.set(int64_t(1), value);
  init.set(s_value, value, true);
  init.set(int64_t(0), key);
  init.set(s_key, key, true);
  m_pos = toInt64(nextIndex(toPos(m_pos), m_size));
  return Array(init.create());
}

template <class K>
TypedValue* ArrayShell::nvGetImpl(K k) const {
  APILOG << "(" << keystr(k) << ")";
  auto const pos = find(k, m_size);
  return LIKELY(pos != PosType::invalid)
    ? reinterpret_cast<TypedValue*>(&lval(pos))
    : nullptr;
}

void ArrayShell::nvGetKey(TypedValue* out, ssize_t pos) {
  APILOG << "(" << out << ", " << pos << ")";
  assert(size_t(pos) < m_size);
  new(out) Variant(key(toPos(pos)));
}

TypedValue* ArrayShell::nvGetValueRef(ssize_t pos) {
  APILOG << "(" << pos << ")";
  assert(size_t(pos) < m_size);
  return reinterpret_cast<TypedValue*>(&lval(toPos(pos)));
}

template <class K>
TypedValue* ArrayShell::nvGetCellImpl(K k) const {
  APILOG << "(" << keystr(k) << ")";
  auto const pos = find(k, m_size);
  return LIKELY(pos != PosType::invalid)
    ? tvToCell(reinterpret_cast<TypedValue*>(&lval(pos)))
    : nvGetNotFound(k);
}

template <class K>
ssize_t ArrayShell::getIndexImpl(K k) const {
  APILOG << "(" << keystr(k) << ")";
  return toInt64(find(k, m_size));
}

template <class K>
const Variant& ArrayShell::getImpl(K k, bool error) const {
  APILOG << "(" << keystr(k) << ", " << error << ")";
  auto const pos = find(k, m_size);
  if (pos != PosType::invalid) {
    return val(pos);
  }
  return error ? getNotFound(k) : null_variant;
}

template <class K>
ArrayData *ArrayShell::lvalImpl(K k, Variant*& ret,
                                  bool copy, bool checkExist) {
  APILOG << "(" << keystr(k) << ", " << ret << ", "
         << copy << ", " << checkExist << ")";

  if (copy) {
    return ArrayShell::copy()->lvalImpl(k, ret, false, checkExist);
  }

  PosType pos = PosType::invalid;

  if (checkExist && (pos = find(k, m_size)) != PosType::invalid) {
    assert(toUint32(pos) < m_size);
    auto& e = lval(pos);
    if (e.isReferenced() || e.isObject()) {
      MYLOG << (void*)this << "->lval:" << "found1";
      ret = &e;
      return this;
    }
  }

  // Make sure the search is done. TODO: this may actually search
  // twice sometimes.
  if (pos == PosType::invalid) {
    pos = find(k, m_size);
  }

  if (pos != PosType::invalid) {
    // found, don't overwrite anything
    assert(toUint32(pos) <= m_size);
    ret = &lval(pos);
    MYLOG << (void*)this << "->lvalImpl:" << "found at " << toInt64(pos)
          << ", value=" << valstr(*ret) << ", size=" << m_size;
  } else {
    // not found, initialize
    if (m_size == capacity()) {
      grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
    }
    assert(m_size < capacity());
    ret = appendNoGrow(k, Variant::nullInit);
  }

  return this;
}

ArrayData *ArrayShell::lvalNew(Variant *&ret, bool copy) {
  if (copy) {
    return ArrayShell::copy()->lvalNew(ret, false);
  }

  // Andrei: TODO - append() currently never fails, probably it
  // should.
  auto oldSize = m_size;
  append(uninit_null(), false);
  assert(m_size == oldSize + 1);
  if (UNLIKELY(oldSize == m_size)) {
    ret = &Variant::lvalBlackHole();
  } else {
    assert(lastIndex(m_size) != PosType::invalid);
    ret = &lval(lastIndex(m_size));
  }

  return this;
}

ArrayData *ArrayShell::lvalPtr(StringData* k, Variant *&ret, bool copy,
                             bool create) {
  NOT_IMPLEMENTED();
}

template <class K>
ArrayShell* ArrayShell::setImpl(K k, const Variant& v, bool copy) {
  APILOG << "(" << keystr(k) << ", " << valstr(v) << ", " << copy
         << ")";
  ArrayShell* result = this;
  if (copy) result = ArrayShell::copy();
  if (result->update(k, v, result->m_size, result->m_nonsmart)) {
    // Added a new element, must update size and possibly m_pos
    if (m_pos == invalid_index) m_pos = result->m_size;
    result->m_size++;
  }
  return result;
}

template <class K>
ArrayData *ArrayShell::setRefImpl(K k, CVarRef v, bool copy) {
  APILOG << "(" << keystr(k) << ", " << valstr(v) << ", " << copy << ")";

  if (copy) {
    return ArrayShell::copy()->setRef(k, v, false);
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
      grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
    }
    appendNoGrow(k, Variant::noInit)->constructRefHelper(v);
  }
  return this;
}

template <class K>
ArrayData *ArrayShell::addImpl(K k, const Variant& v, bool copy) {
  APILOG << "(" << keystr(k) << ", " << valstr(v) << ", " << copy << ");";
  if (copy) {
    auto result = ArrayShell::copy(m_size * 2 + 1);
    result->add(k, v, false);
    return result;
  }
  assert(!exists(k));
  // Make sure there's enough capacity
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  appendNoGrow(k, v);
  return this;
}

ArrayShell *ArrayShell::addLval(int64_t k, Variant*& ret, bool copy) {
  APILOG << "(" << k << ", " << ret << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->addLval(k, ret, false);
  }
  assert(!exists(k) && m_size <= capacity());
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  ret = appendNoGrow(k, Variant::nullInit);
  MYLOG << (void*)this << "->lval:" << "added";
  return this;
}

ArrayShell *ArrayShell::addLval(StringData* k, Variant *&ret, bool copy) {
  NOT_IMPLEMENTED();
}

template <class K>
ArrayData *ArrayShell::removeImpl(K k, bool copy) {
  APILOG << "(" << keystr(k) << ", " << copy << ")";

  if (copy) {
    return ArrayShell::copy()->remove(k, false);
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

ssize_t ArrayShell::iter_begin() const {
  APILOG << "()";
  return m_size ? toInt64(firstIndex(m_size)) : invalid_index;
}

ssize_t ArrayShell::iter_end() const {
  APILOG << "()";
  return ssize_t(lastIndex(m_size));
}

ssize_t ArrayShell::iter_advance(ssize_t prev) const {
  APILOG << "(" << prev << ")";
  auto const result = toInt64(nextIndex(toPos(prev), m_size));
  MYLOG << "returning " << result;
  return result;
}

ssize_t ArrayShell::iter_rewind(ssize_t prev) const {
  APILOG << "(" << prev << ")";
  return toInt64(prevIndex(toPos(prev), m_size));
}

bool ArrayShell::validFullPos(const FullPos& fp) const {
  APILOG << "(" << fp.m_pos << ";" << fp.getResetFlag() << ")";
  assert(fp.getContainer() == this);
  return fp.m_pos != invalid_index;
}

bool ArrayShell::advanceFullPos(FullPos &fp) {
  APILOG << "(" << fp.m_pos << ";" << fp.getResetFlag() << ")";
  assert(fp.getContainer() == this);
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = invalid_index;
  } else if (fp.m_pos == invalid_index) {
    return false;
  }
  fp.m_pos = toInt64(nextIndex(toPos(fp.m_pos), m_size));
  if (fp.m_pos == invalid_index) {
    return false;
  }
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  m_pos = toInt64(nextIndex(toPos(fp.m_pos), m_size));
  return true;
}

CVarRef ArrayShell::currentRef() {
  NOT_IMPLEMENTED();
}

CVarRef ArrayShell::endRef() {
  NOT_IMPLEMENTED();
}

HphpArray* ArrayShell::toHphpArray() const {
  auto result = NEW(HphpArray)(m_size);
  FOR_EACH_RANGE (i, 0, m_size) {
    if (hasStrKey(toPos(i))) {
      result->add(key(toPos(i)).getStringData(), val(toPos(i)), false);
    } else {
      result->add(key(toPos(i)).getInt64(), val(toPos(i)), false);
    }
  }
  return result;
}

ArrayData* ArrayShell::escalateForSort() {
  APILOG << "()";
  return toHphpArray();
}

void ArrayShell::ksort(int sort_flags, bool ascending) {
  NOT_IMPLEMENTED();
}

void ArrayShell::sort(int sort_flags, bool ascending) {
  NOT_IMPLEMENTED();
}

void ArrayShell::asort(int sort_flags, bool ascending) {
  NOT_IMPLEMENTED();
}

void ArrayShell::uksort(CVarRef cmp_function) {
  NOT_IMPLEMENTED();
}

void ArrayShell::usort(CVarRef cmp_function) {
  NOT_IMPLEMENTED();
}

void ArrayShell::uasort(CVarRef cmp_function) {
  NOT_IMPLEMENTED();
}

ArrayShell *ArrayShell::copy() const {
  APILOG << "()";
  auto result = NEW(ArrayShell)(
    *this,
    capacity() + (m_size == capacity()),
    m_nonsmart);
  assert(result->getCount() == 0);
  return result;
}

ArrayShell* ArrayShell::copy(uint capacity) {
  APILOG << "(" << capacity << ")";
  return NEW(ArrayShell)(*this, capacity, m_nonsmart);
}

ArrayShell *ArrayShell::copyWithStrongIterators() const {
  APILOG << "()";
  auto result = ArrayShell::copy();
  moveStrongIterators(result, const_cast<ArrayShell*>(this));
  assert(result->getCount() == 0);
  return result;
}

ArrayData *ArrayShell::nonSmartCopy() const {
  APILOG << "()";
  //return NEW(ArrayShell)(*this, capacity(), true);
  return toHphpArray()->nonSmartCopy();
}

ArrayShell *ArrayShell::append(const Variant& v, bool copy) {
  APILOG << "(" << valstr(v) << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->append(v, false);
  }
  grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  appendNoGrow(nextKeyBump(), v);
  return this;
}

ArrayShell *ArrayShell::appendRef(const Variant& v, bool copy) {
  APILOG << "(" << valstr(v) << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->appendRef(v, false);
  }
  //addValWithRef(nextKeyBump(), v);
  auto const k = nextKeyBump();
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  assert(m_size < capacity());
  appendNoGrow(k, Variant::noInit)->constructRefHelper(v);
  return this;
}

  /**
   * Similar to append(v, copy), with reference in v preserved.
   */
ArrayData *ArrayShell::appendWithRef(CVarRef v, bool copy) {
  APILOG << "(" << valstr(v) << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->appendWithRef(v, false);
  }
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  assert(m_size < capacity());
  appendNoGrow(nextKeyBump(), Variant::nullInit)->setWithRef(v);
  return this;
}

template <class K>
void ArrayShell::addValWithRef(K k, const Variant& v) {
  MYLOG << (void*)this << "->addValWithRef("
        << keystr(k) << ", " << valstr(v)
        << "); size=" << m_size;
  auto pos = find(k, m_size);
  if (pos != PosType::invalid) {
    return;
  }
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  assert(m_size < capacity());
  appendNoGrow(k, Variant::nullInit)->setWithRef(v);
}

void ArrayShell::nextInsertWithRef(const Variant& v) {
  MYLOG << (void*)this << "->nextInsertWithRef("
        << valstr(v)
        << "); size=" << m_size;
  // We need to define k here (before the if/grow) because otherwise
  // the overzealous gcc issues a spurious warning as such:
  //
  // hphp/runtime/base/array/policy_array.h: In member function 'void
  // HPHP::ArrayShell::nextInsertWithRef(const HPHP::Variant&)':
  // hphp/runtime/base/array/policy_array.h:114:5: error: assuming
  // signed overflow does not occur when assuming that (X + c) < X is
  // always false [-Werror=strict-overflow]
  auto const k = nextKeyBump();
  if (m_size == capacity()) {
    grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);
  }
  assert(m_size < capacity());
  appendNoGrow(k, Variant::nullInit)->setWithRef(v);
}

ArrayData *ArrayShell::append(const ArrayData *elems, ArrayOp op, bool copy) {
  APILOG << "(" << elems << ", " << op << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->append(elems, op, false);
  }

  assert(elems);
  assert(op == Plus || op == Merge);
  grow(m_size, m_size + 1, m_size * 2 + 1, m_nonsmart);

  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    const Variant& value = it.secondRef();
    if (op == Plus) {
      if (key.isNumeric()) {
        addValWithRef(key.toInt64(), value);
      } else {
        addValWithRef(key.getStringData(), value);
      }
    } else {
      if (key.isNumeric()) {
        nextInsertWithRef(value);
      } else {
        StringData *s = key.getStringData();
        Variant *p;
        // Andrei TODO: make sure this is the right semantics
        lval(s, p, false, true);
        p->setWithRef(value);
      }
    }
  }
  return this;
}

  /**
   * Stack function: pop the last item and return it.
   */
ArrayData* ArrayShell::pop(Variant &value) {
  if (getCount() > 1) {
    return ArrayShell::copy()->pop(value);
  }
  if (!m_size) {
    value = uninit_null();
    return this;
  }
  auto pos = lastIndex(m_size);
  assert(size_t(pos) < m_size);
  value = val(pos);
  erase(pos, m_size);
  --m_size;
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator.
  m_pos = m_size ? toInt64(firstIndex(m_size)) : invalid_index;
  return this;
}

ArrayData *ArrayShell::dequeue(Variant &value) {
  APILOG << "(" << &value << ")";
  if (getCount() > 1) {
    return ArrayShell::copy()->dequeue(value);
  }

  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  freeStrongIterators();
  if (!m_size) {
    value = uninit_null();
    return this;
  }

  auto& front = lval(firstIndex(m_size));
  value = std::move(front);
  new(&front) Variant;
  erase(firstIndex(m_size), m_size);
  --m_size;
  renumber();

  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  m_pos = m_size ? toInt64(firstIndex(m_size)) : invalid_index;
  return this;
}

ArrayData* ArrayShell::prepend(CVarRef v, bool copy) {
  APILOG << "(" << valstr(v) << ", " << copy << ")";
  if (copy) {
    return ArrayShell::copy()->prepend(v, false);
  }
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  freeStrongIterators();
  Store::prepend(v, m_size, m_nonsmart);
  ++m_size;
  auto first = firstIndex(m_size);
  setKey(first, int64_t(0));
  // Renumber.
  renumber();
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  m_pos = toInt64(first);
  return this;
}

void ArrayShell::renumber() {
  APILOG << "()";
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
      m_pos = toInt64(find(currentPosKey.getStringData(), m_size));
    } else if (currentPosKey.is(KindOfInt64)) {
      m_pos = toInt64(find(currentPosKey.getInt64(), m_size));
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
      fp->m_pos = toInt64(find(k.getStringData(), m_size));
    } else {
      assert(k.is(KindOfInt64));
      fp->m_pos = toInt64(find(k.getInt64(), m_size));
    }
  }
  assert(i == siKeys.cend());
}

void ArrayShell::onSetEvalScalar() {
  APILOG << "()";
  //FOR_EACH_RANGE (pos, 0, m_size) {
  for (auto pos = firstIndex(m_size); pos != PosType::invalid;
       pos = nextIndex(pos, m_size)) {
    if (hasStrKey(pos)) {
      auto k = key(pos).getStringData();
      if (!k->isStatic()) {
        auto sk = StringData::GetStaticString(k);
        if (k->decRefCount() == 0) {
          DELETE(StringData)(k);
        }
        // Andrei TODO: inefficient, does one incref and then decref
        setKey(pos, sk);
        sk->decRefCount();
      }
    }
    lval(pos).setEvalScalar();
  }
}

void ArrayShell::dump() {
  NOT_IMPLEMENTED();
}

void ArrayShell::dump(std::string &out) {
  NOT_IMPLEMENTED();
}

void ArrayShell::dump(std::ostream &os) {
  NOT_IMPLEMENTED();
}

ArrayData *ArrayShell::escalate() const {
  APILOG << "()";
  return ArrayData::escalate();
}


} // namespace HPHP
