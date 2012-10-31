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
#define INLINE_VARIANT_HELPER 1 // for selected inlining

#include <runtime/base/array/vector_array.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/externals.h>
#include <runtime/base/runtime_option.h>
#include <util/util.h>

namespace HPHP {

StaticEmptyVectorArray StaticEmptyVectorArray::s_theEmptyVectorArray;

IMPLEMENT_SMART_ALLOCATION_HOT(VectorArray);

#ifdef DEBUGGING_SMART_ALLOCATOR
#define DECLARE_ALLOCATOR(a, T, I)
#define NEWALLOC(a) new
#define DEALLOC(a, e, T) delete e;
#else
#define DECLARE_ALLOCATOR(a, T, I)                  \
  SmartAllocator<T, SmartAllocatorImpl::I> *a =     \
    T::AllocatorType::getNoCheck();
#define NEWALLOC(a) new (a)
#define DEALLOC(a, e, T) (e->~T(),a->dealloc(e))
#endif
ssize_t VectorArray::vsize() const { not_reached(); }

HOT_FUNC_HPHP
ssize_t VectorArray::iter_begin() const {
  if (!m_size) return VectorArray::invalid_index;
  return 0;
}

ssize_t VectorArray::iter_end() const {
  if (!m_size) return VectorArray::invalid_index;
  return m_size - 1;
}

HOT_FUNC_HPHP
ssize_t VectorArray::iter_advance(ssize_t prev) const {
  ASSERT(prev >= 0 && prev < (ssize_t)m_size);
  ssize_t next = prev + 1;
  if (next >= m_size) return VectorArray::invalid_index;
  return next;
}

ssize_t VectorArray::iter_rewind(ssize_t prev) const {
  ASSERT(prev >= 0 && prev < (ssize_t)m_size);
  ssize_t next = prev - 1;
  if (next < 0) return VectorArray::invalid_index;
  return next;
}

HOT_FUNC_HPHP
Variant VectorArray::getKey(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_size);
  return pos;
}

HOT_FUNC_HPHP
Variant VectorArray::getValue(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_size);
  return tvAsCVarRef(&m_elems[pos]);
}

HOT_FUNC_HPHP
CVarRef VectorArray::getValueRef(ssize_t pos) const {
  ASSERT(pos >= 0 && pos < m_size);
  return tvAsCVarRef(&m_elems[pos]);
}

HOT_FUNC_HPHP
Variant VectorArray::value(ssize_t &pos) const {
  if (pos >= 0 && pos < m_size) {
    return tvAsCVarRef(&m_elems[pos]);
  }
  pos = VectorArray::invalid_index;
  return false;
}

Variant VectorArray::key() const {
  if (m_pos >= 0 && m_pos < m_size) {
    return m_pos;
  }
  return null;
}

Variant VectorArray::current() const {
  if (m_pos >= 0 && m_pos < m_size) {
    return tvAsCVarRef(&m_elems[m_pos]);
  }
  return false;
}

Variant VectorArray::reset() {
  ssize_t pos = 0;
  Variant v = VectorArray::value(pos);
  if (m_pos != pos) m_pos = pos;
  return v;
}
Variant VectorArray::prev() {
  ssize_t pos = m_pos - 1;
  Variant v = VectorArray::value(pos);
  if (m_pos != pos) m_pos = pos;
  return v;
}
Variant VectorArray::next() {
  ssize_t pos = m_pos + 1;
  Variant v = VectorArray::value(pos);
  if (m_pos != pos) m_pos = pos;
  return v;
}
Variant VectorArray::end() {
  ssize_t pos = (ssize_t)m_size - 1;
  Variant v = VectorArray::value(pos);
  if (m_pos != pos) m_pos = pos;
  return v;
}

void VectorArray::alloc(uint size) {
  if (size <= FixedSize) {
    m_capacity = FixedSize;
    m_elems = m_fixed;
    m_allocMode = kInline;
    return;
  }
  uint cap = Util::nextPower2(size);
  m_capacity = cap;
  if (!m_nonsmart) {
    m_elems = (TypedValue*) smart_malloc(cap * sizeof(TypedValue));
    m_allocMode = kSmart;
  } else {
    m_elems = (TypedValue*) malloc(cap * sizeof(TypedValue));
    m_allocMode = kMalloc;
  }
}

HOT_FUNC_HPHP
VectorArray::VectorArray(uint capacity /* = 0 */) :
    m_nonsmart(false) {
  m_size = 0;
  alloc(capacity);
  m_pos = ArrayData::invalid_index;
}

HOT_FUNC_HPHP
VectorArray::VectorArray(const VectorArray *src, uint start /* = 0 */,
  uint size /* = 0 */) :
    ArrayData(src), m_nonsmart(false) {
  ASSERT(src);
  ASSERT(size == 0 || (size == src->m_size - 1L && size > 0));
  ASSERT(!src->strongIterators());
  ASSERT(m_pos == src->m_pos);
  m_size = size ? size : src->m_size;
  alloc(m_size);
  for (uint i = 0; i < m_size; i++) {
    Variant& to = tvAsUninitializedVariant(&m_elems[i]);
    CVarRef fm = tvAsCVarRef(&src->m_elems[i + start]);
    to.constructWithRefHelper(fm, src);
  }
}

HOT_FUNC_HPHP
VectorArray::VectorArray(uint size, const Variant *values[]) :
    m_nonsmart(false) {
  ASSERT(size > 0);
  m_size = size;
  alloc(size);
  for (uint i = 0; i < size; i++) {
    Variant& to = tvAsUninitializedVariant(&m_elems[i]);
    to.constructValHelper(*values[i]);
  }
  ASSERT(m_pos == 0);
}

HOT_FUNC_HPHP
VectorArray::~VectorArray() {
  uint size = m_size;
  for (uint i = 0; i < size; i++) {
    tvAsVariant(&m_elems[i]).~Variant();
  }
  if (m_allocMode == kSmart) {
    smart_free(m_elems);
  } else if (m_allocMode == kMalloc) {
    free(m_elems);
  }
}

// This constructor is for nonSmartCopy()
VectorArray::VectorArray(const VectorArray *src, bool sma /* ignored */) :
    ArrayData(src), m_nonsmart(true) {
  ASSERT(src);
  ASSERT(!src->strongIterators());
  m_size = src->m_size;
  if (m_size <= FixedSize) {
    m_capacity = FixedSize;
    m_elems = m_fixed;
    m_allocMode = kInline;
  } else {
    m_capacity = Util::nextPower2(m_size);
    m_elems = (TypedValue*) malloc(m_capacity * sizeof(TypedValue));
    m_allocMode = kMalloc;
  }
  for (uint i = 0, n = m_size; i < n; i++) {
    ASSERT(src->m_elems[i].m_type != KindOfRef &&
           src->m_elems[i].m_type != KindOfObject);
    Variant& to = tvAsUninitializedVariant(&m_elems[i]);
    CVarRef fm = tvAsCVarRef(&src->m_elems[i]);
    to.constructWithRefHelper(fm, src);
  }
  ASSERT(src->m_pos == 0 && m_pos == 0);
}

void VectorArray::grow(uint newSize) {
  ASSERT(newSize > FixedSize);
  m_capacity = Util::nextPower2(newSize);
  if (!m_nonsmart) {
    ASSERT(m_allocMode == kInline || m_allocMode == kSmart);
    if (m_allocMode == kInline) {
      m_elems = (TypedValue*)smart_malloc(m_capacity * sizeof(TypedValue));
      memcpy(m_elems, m_fixed, m_size * sizeof(TypedValue));
      m_allocMode = kSmart;
    } else {
      m_elems = (TypedValue*)smart_realloc(m_elems,
                                           m_capacity * sizeof(TypedValue));
    }
  } else if (m_allocMode == kInline) {
    m_elems = (TypedValue*)malloc(m_capacity * sizeof(TypedValue));
    memcpy(m_elems, m_fixed, m_size * sizeof(TypedValue));
    m_allocMode = kMalloc;
  } else {
    ASSERT(m_allocMode == kMalloc);
    m_elems = (TypedValue*)realloc(m_elems, m_capacity * sizeof(TypedValue));
  }
}

inline void ALWAYS_INLINE VectorArray::checkSize(uint n /* = 1 */) {
  ASSERT(n >= 1);
  ASSERT(m_size <= m_capacity);
  uint newSize = m_size + n;
  if (UNLIKELY(m_capacity < newSize)) {
    grow(newSize);
  }
}

/*
 * Do unsigned comparison to cheaply exclude k < 0
 */
inline ALWAYS_INLINE bool inRange(int64_t k, uint size) {
  return size_t(k) < size_t(size);
}

HOT_FUNC_HPHP
bool VectorArray::exists(int64 k) const {
  return inRange(k, m_size);
}

bool VectorArray::exists(const StringData* k) const {
  return false;
}

HOT_FUNC_HPHP
CVarRef VectorArray::get(int64 k, bool error /* = false */) const {
  if (LIKELY(inRange(k, m_size))) {
    return tvAsCVarRef(&m_elems[k]);
  }
  return error ? getNotFound(k) : null_variant;
}

CVarRef VectorArray::get(const StringData* k, bool error /* = false */) const {
  return error ? getNotFound(k) : null_variant;
}

ssize_t VectorArray::getIndex(int64 k) const {
  if (k >= 0 && k < m_size) return k;
  return ArrayData::invalid_index;
}

ssize_t VectorArray::getIndex(const StringData* k) const {
  return ArrayData::invalid_index;
}

ZendArray *VectorArray::escalateToNonEmptyZendArray() const {
  ASSERT(m_size);
  ZendArray *ret;
  ZendArray::Bucket *p[256], **pp;
  if (LIKELY(m_size < 256)) {
    pp = p;
  } else {
    pp =
      (ZendArray::Bucket **)malloc(sizeof(ZendArray::Bucket *) * (m_size + 1));
  }
  DECLARE_ALLOCATOR(a, ZendArray::Bucket, Bucket);
  for (int64 i = 0; i < m_size; i++) {
    CVarRef v = tvAsCVarRef(&m_elems[i]);
    pp[i] = NEWALLOC(a) ZendArray::Bucket(i, withRefBind(v));
  }
  pp[m_size] = NULL;
  ret = NEW(ZendArray)(m_size, m_size, pp);
  if (UNLIKELY(pp != p)) free(pp);
  if (m_pos != ArrayData::invalid_index) {
    ret->setPosition(ret->getIndex(m_pos));
  } else {
    ret->setPosition(0);
  }
  return ret;
}

HOT_FUNC_HPHP
ZendArray *VectorArray::escalateToZendArray() const {
  if (m_size == 0) {
    ASSERT(m_pos == ArrayData::invalid_index);
    return NEW(ZendArray)();
  }
  return escalateToNonEmptyZendArray();
}

inline void ALWAYS_INLINE VectorArray::checkInsertIterator(ssize_t pos) {
  if (m_pos == ArrayData::invalid_index) m_pos = pos;
}

ArrayData *VectorArray::lvalNew(Variant *&ret, bool copy) {
  if (UNLIKELY(copy)) {
    VectorArray *a = NEW(VectorArray)(this);
    a->VectorArray::lvalNew(ret, false);
    return a;
  }
  uint index = m_size;
  checkSize();
  Variant& v = tvAsUninitializedVariant(&m_elems[index]);
  v.setUninitNull();
  ret = &v;
  checkInsertIterator((ssize_t)index);
  m_size++;
  return NULL;
}

ArrayData *VectorArray::lval(int64 k, Variant *&ret, bool copy,
                             bool checkExist /* = false */) {
  ret = inRange(k, m_size) ? &tvAsVariant(&m_elems[k]) : NULL;
  if (ret == NULL && k != m_size) {
    ZendArray *a = escalateToZendArray();
    a->addLvalImpl(k, &ret, false);
    return a;
  }
  if (LIKELY(!copy)) {
    if (ret) return NULL;
    ASSERT(m_size == k);
    checkSize();
    Variant& v = tvAsUninitializedVariant(&m_elems[k]);
    v.setUninitNull();
    ret = &v;
    checkInsertIterator((ssize_t)k);
    m_size++;
    return NULL;
  }
  if (checkExist && ret && (ret->isReferenced() || ret->isObject())) {
    return NULL;
  }
  VectorArray *a = NEW(VectorArray)(this);
  if (ret) {
    Variant& v = tvAsVariant(&a->m_elems[k]);
    ret = &v;
    ASSERT(ret);
    return a;
  }
  ASSERT(m_size == k);
  a->VectorArray::lvalNew(ret, false);
  return a;
}

ArrayData *VectorArray::lval(StringData* k, Variant *&ret, bool copy,
                             bool checkExist /* = false */) {
  ZendArray *a = escalateToZendArray();
  a->addLvalImpl(k, k->hash(), &ret);
  return a;
}

ArrayData *VectorArray::lvalPtr(StringData* k, Variant *&ret, bool copy,
                                bool create) {
  ZendArray *a = escalateToZendArray();
  a->lvalPtr(k, ret, false, create);
  return a;
}

ArrayData *VectorArray::lvalPtr(int64 k, Variant *&ret, bool copy,
                                bool create) {
  ZendArray *a = escalateToZendArray();
  a->lvalPtr(k, ret, false, create);
  return a;
}

HOT_FUNC_HPHP
ArrayData *VectorArray::set(int64 k, CVarRef v, bool copy) {
  if (inRange(k, m_size)) {
    if (copy) {
      VectorArray *a = NEW(VectorArray)(this);
      tvAsVariant(&a->m_elems[k]).assignVal(v);
      return a;
    }
    tvAsVariant(&m_elems[k]).assignVal(v);
    return NULL;
  }
  if (k == m_size) return VectorArray::append(v, copy);
  ZendArray *a = escalateToZendArray();
  a->add(k, v, false);
  return a;
}

HOT_FUNC_HPHP
ArrayData *VectorArray::set(StringData* k, CVarRef v, bool copy) {
  ZendArray *a = escalateToZendArray();
  a->add(k, v, false);
  return a;
}

ArrayData *VectorArray::setRef(int64 k, CVarRef v, bool copy) {
  if (UNLIKELY(copy)) {
    if (inRange(k, m_size) || k == m_size) {
      VectorArray *a = NEW(VectorArray)(this);
      a->VectorArray::setRef(k, v, false);
      return a;
    }
  } else {
    if (inRange(k, m_size)) {
      tvAsVariant(&m_elems[k]).assignRef(v);
      return NULL;
    } else if (k == m_size) {
      checkSize();
      tvAsUninitializedVariant(&m_elems[k]).constructRefHelper(v);
      checkInsertIterator((ssize_t)k);
      m_size++;
      return NULL;
    }
  }
  ZendArray *a = escalateToZendArray();
  a->updateRef(k, v);
  return a;
}

ArrayData *VectorArray::setRef(StringData* k, CVarRef v, bool copy) {
  ZendArray *a = escalateToZendArray();
  a->updateRef(k, v);
  return a;
}

ArrayData *VectorArray::copy() const {
  return NEW(VectorArray)(this);
}

ArrayData *VectorArray::nonSmartCopy() const {
  return new VectorArray(this, false);
}

HOT_FUNC_HPHP
ArrayData *VectorArray::append(CVarRef v, bool copy) {
  uint index = m_size;
  if (copy) {
    VectorArray *a = NEW(VectorArray)(this);
    a->checkSize();
    tvAsUninitializedVariant(&a->m_elems[index]).constructValHelper(v);
    a->checkInsertIterator((ssize_t)index);
    a->m_size++;
    return a;
  }
  checkSize();
  tvAsUninitializedVariant(&m_elems[index]).constructValHelper(v);
  checkInsertIterator((ssize_t)index);
  m_size++;
  return NULL;
}

ArrayData *VectorArray::appendRef(CVarRef v, bool copy) {
  if (UNLIKELY(copy)) {
    VectorArray *a = NEW(VectorArray)(this);
    a->VectorArray::appendRef(v, false);
    return a;
  }
  uint index = m_size;
  checkSize();
  tvAsUninitializedVariant(&m_elems[index]).constructRefHelper(v);
  checkInsertIterator((ssize_t)index);
  m_size++;
  return NULL;
}

ArrayData *VectorArray::appendWithRef(CVarRef v, bool copy) {
  if (UNLIKELY(copy)) {
    VectorArray *a = NEW(VectorArray)(this);
    a->VectorArray::appendWithRef(v, false);
    return a;
  }
  uint index = m_size;
  checkSize();
  Variant& to = tvAsUninitializedVariant(&m_elems[index]);
  to.setUninitNull();
  to.setWithRef(v);
  checkInsertIterator((ssize_t)index);
  m_size++;
  return NULL;
}

HOT_FUNC_HPHP
ArrayData *VectorArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  if (UNLIKELY(!elems->isVectorArray())) {
    ZendArray *a = escalateToZendArray();
    a->append(elems, op, false);
    return a;
  }
  if (UNLIKELY(copy)) {
    VectorArray *a = NEW(VectorArray)(this);
    a->VectorArray::append(elems, op, false);
    return a;
  }
  ASSERT(dynamic_cast<const VectorArray *>(elems));
  const VectorArray *velems = static_cast<const VectorArray *>(elems);
  if (op == Plus) {
    if (velems->m_size > m_size) {
      checkSize(velems->m_size - m_size);
      for (uint i = m_size; i < velems->m_size; i++) {
        Variant& to = tvAsUninitializedVariant(&m_elems[i]);
        CVarRef fm = tvAsCVarRef(&velems->m_elems[i]);
        to.constructWithRefHelper(fm, 0);
      }
      checkInsertIterator((ssize_t)m_size);
      m_size = velems->m_size;
    }
  } else {
    ASSERT(op == Merge);
    if (velems->m_size > 0) {
      checkSize(velems->m_size);
      for (uint i = m_size; i < m_size + velems->m_size; i++) {
        Variant& to = tvAsUninitializedVariant(&m_elems[i]);
        CVarRef fm = tvAsCVarRef(&velems->m_elems[i - m_size]);
        to.constructWithRefHelper(fm, 0);
      }
      checkInsertIterator((ssize_t)m_size);
      m_size += velems->m_size;
    }
  }
  return NULL;
}

ArrayData *VectorArray::pop(Variant &value) {
  if (UNLIKELY(!m_size)) {
    value.setNull();
    return NULL;
  }
  if (UNLIKELY(getCount() > 1)) {
    value = tvAsCVarRef(&m_elems[m_size - 1]);
    if (m_size == 1) {
      return StaticEmptyVectorArray::Get();
    }
    VectorArray *a = NEW(VectorArray)(this, 0, m_size - 1);
    a->m_pos = (ssize_t)0;
    return a;
  }
  ssize_t pos = m_size - 1;
  value = tvAsCVarRef(&m_elems[pos]);
  tvAsVariant(&m_elems[pos]).~Variant();
  ASSERT(m_size && pos == m_size - 1L);
  m_size--;
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator
  m_pos = m_size ? (ssize_t)0 : ArrayData::invalid_index;
  return NULL;
}

ArrayData *VectorArray::add(int64 k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (k == m_size) return VectorArray::append(v, copy);
  ZendArray *a = escalateToZendArray();
  a->add(k, v, false);
  return a;
}

ArrayData *VectorArray::add(StringData* k, CVarRef v, bool copy) {
  ZendArray *a = escalateToZendArray();
  a->add(k, v, false);
  return a;
}

ArrayData *VectorArray::addLval(int64 k, Variant *&ret, bool copy) {
  ASSERT(!exists(k));
  if (k != m_size) {
    ZendArray *a = escalateToZendArray();
    a->addLval(k, ret, false);
    return a;
  }
  uint index = m_size;
  if (UNLIKELY(copy)) {
    VectorArray *a = NEW(VectorArray)(this);
    a->VectorArray::addLval(k, ret, false);
    return a;
  }
  checkSize();
  Variant& v = tvAsUninitializedVariant(&m_elems[index]);
  v.setUninitNull();
  ret = &v;
  checkInsertIterator((ssize_t)index);
  m_size++;
  return NULL;
}

ArrayData *VectorArray::addLval(StringData* k, Variant *&ret, bool copy) {
  ZendArray *a = escalateToZendArray();
  a->addLval(k, ret, false);
  return a;
}

ArrayData *VectorArray::remove(int64 k, bool copy) {
  if (!inRange(k, m_size)) return NULL;
  if (k != m_size - 1L) {
    ArrayData *a = escalateToNonEmptyZendArray();
    a->remove(k, false);
    return a;
  }
  // k == m_size-1
  if (copy) {
    if (k == 0) return StaticEmptyVectorArray::Get();
    VectorArray *a = NEW(VectorArray)(this, 0, m_size - 1);
    if (a->m_pos == m_size - 1) a->m_pos = ArrayData::invalid_index;
    return a;
  }
  ASSERT(m_size > 0 && k == m_size - 1);
  tvAsCVarRef(&m_elems[k]).~Variant();
  if (m_pos == k) m_pos = ArrayData::invalid_index;
  ASSERT(m_size && k == m_size - 1L);
  m_size--;
  return NULL;
}

ArrayData *VectorArray::remove(const StringData* k, bool copy) {
  return NULL;
}

ArrayData *VectorArray::prepend(CVarRef v, bool copy) {
  if (UNLIKELY(m_size == m_capacity)) {
    ZendArray *a = escalateToNonEmptyZendArray();
    ArrayData *aa UNUSED = a->prepend(v, false);
    ASSERT(!aa);
    return a;
  }
  if (UNLIKELY(copy)) {
    ArrayData *a = UNLIKELY(m_size >= FixedSize && Util::isPowerOfTwo(m_size)) ?
      // in this case, we would escalate in the capacity check anyway
      static_cast<ArrayData*>(escalateToNonEmptyZendArray()) :
      static_cast<ArrayData*>(NEW(VectorArray)(this));
    ArrayData *aa UNUSED = a->prepend(v, false);
    ASSERT(!aa);
    return a;
  }
  checkSize();
  for (uint i = m_size; i > 0; i--) {
    // copying TV's by value, intentionally not refcounting.
    m_elems[i] = m_elems[i-1];
  }
  tvAsUninitializedVariant(&m_elems[0]).constructValHelper(v);
  m_size++;
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  m_pos = (ssize_t)0;
  return NULL;
}

ArrayData *VectorArray::dequeue(Variant &value) {
  if (UNLIKELY(!m_size)) {
    value.setNull();
    return NULL;
  }
  if (UNLIKELY(getCount() > 1)) {
    value = tvAsCVarRef(&m_elems[0]);
    if (m_size == 1) {
      return StaticEmptyVectorArray::Get();
    }
    VectorArray *a = NEW(VectorArray)(this, 1, m_size - 1);
    a->m_pos = (ssize_t)0;
    return a;
  }
  value = tvAsCVarRef(&m_elems[0]);
  m_size--;
  tvAsVariant(&m_elems[0]).~Variant();
  for (uint i = 0; i < m_size; i++) {
    // TypedValue copy without refcounting.
    m_elems[i] = m_elems[i+1];
  }
  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  m_pos = m_size ? (ssize_t)0 : ArrayData::invalid_index;
  return NULL;
}

void VectorArray::onSetEvalScalar() {
  for (uint i = 0; i < m_size; i++) {
    tvAsVariant(&m_elems[i]).setEvalScalar();
  }
}

void VectorArray::getFullPos(FullPos &fp) {
  ASSERT(fp.container == (ArrayData*)this);
  fp.pos = m_pos;
}

bool VectorArray::setFullPos(const FullPos &fp) {
  ASSERT(fp.container == (ArrayData*)this);
  if (fp.pos != ArrayData::invalid_index) {
    m_pos = fp.pos;
    return true;
  }
  return false;
}

CVarRef VectorArray::currentRef() {
  ASSERT(m_pos >= 0 && m_pos < m_size);
  return tvAsCVarRef(&m_elems[m_pos]);
}

CVarRef VectorArray::endRef() {
  ASSERT(m_pos >= 0 && m_pos < m_size);
  return tvAsCVarRef(&m_elems[m_size - 1]);
}

ArrayData *VectorArray::escalate(bool mutableIteration /* = false */) const {
  if (mutableIteration) {
    return escalateToZendArray();
  }
  return const_cast<VectorArray *>(this);
}

ArrayData* VectorArray::escalateForSort() {
  return escalateToZendArray();
}

///////////////////////////////////////////////////////////////////////////////
}
