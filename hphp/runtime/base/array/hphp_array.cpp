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

#define INLINE_VARIANT_HELPER 1

#include "hphp/runtime/base/array/hphp_array.h"
#include "hphp/runtime/base/array/array_init.h"
#include "hphp/runtime/base/array/array_iterator.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/runtime_error.h"
#include "hphp/runtime/base/variable_serializer.h"
#include "hphp/runtime/base/shared/shared_map.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "hphp/util/trace.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/vm/member_operations.h"
#include "hphp/runtime/base/stats.h"

// If PEDANTIC is defined, extra checks are performed to ensure correct
// function even as an array approaches 2^31 elements.  In practice this is
// just wasted effort though, since such an array would require on the order of
// 128 GiB of memory.
//#define PEDANTIC

namespace HPHP {

static_assert(
  sizeof(HphpArray) == 160,
  "Performance is sensitive to sizeof(HphpArray)."
  " Make sure you changed it with good reason and then update this assert.");

TRACE_SET_MOD(runtime);
///////////////////////////////////////////////////////////////////////////////

/*
 * Allocation of HphpArray buffers works like this: the smallest buffer
 * size is allocated inline in HphpArray.  Larger buffer sizes are smart
 * allocated or malloc-allocated depending on whether the array itself
 * was smart-allocated or not.  (nonSmartCopy() is used to create static
 * arrays).  HphpArray::m_allocMode tracks the state as it progresses:
 *
 *   kInline -> kSmart, or
 *           -> kMalloc
 *
 * Hashtables never shrink, so the allocMode Never goes backwards.
 * If an array is pre-sized, we might skip directly to kSmart or kMalloc.
 * If an array is created via nonSmartCopy(), we skip kSmart.
 * Since kMalloc is only used for static arrays, and static arrays are
 * never swept, we don't need any sweep method.
 *
 * For kInline, we use space in HphpArray defined as InlineSlots, which
 * has enough room for slots and the hashtable.  The next few larger array
 * sizes use the inline space for just the hashtable, with slots allocated
 * separately.  Even larger tables allocate the hashtable and slots
 * contiguously.
 */
void *HphpArray::SmaAllocatorInitSetup = SmartAllocatorInitSetup<HphpArray>();

void HphpArray::release() {
  assert(typeid(*this) == typeid(HphpArray));
  this->HphpArray::~HphpArray();
  HphpArray::AllocatorType::getNoCheck()->dealloc(this);
}

//=============================================================================
// Static members.

HphpArray HphpArray::s_theEmptyArray(StaticEmptyArray);

//=============================================================================
// Helpers.

static inline size_t computeMaskFromNumElms(const uint32_t n) {
  assert(n <= 0x7fffffffU);
  size_t lgSize = HphpArray::MinLgTableSize;
  size_t maxElms = (size_t(3U)) << (HphpArray::MinLgTableSize - 2);
  assert(lgSize >= 2);
  while (maxElms < n) {
    ++lgSize;
    maxElms <<= 1;
  }
  assert(lgSize <= 32);
  // return 2^lgSize - 1
  return ((size_t(1U)) << lgSize) - 1;
  static_assert(HphpArray::MinLgTableSize >= 2,
                "lower limit for 0.75 load factor");
}

//=============================================================================
// Construction/destruction.

inline uint32_t HphpArray::initWithoutHash(uint capacity) {
  m_tableMask = computeMaskFromNumElms(capacity);
  auto const tableSize = computeTableSize(m_tableMask);
  allocData(computeMaxElms(m_tableMask), tableSize);
  return tableSize;
}

inline void HphpArray::init(uint capacity) {
  assert(m_size == 0);
  const auto tableSize = initWithoutHash(capacity);
  initHash(m_hash, tableSize);
}

HphpArray::HphpArray(uint capacity)
    : ArrayData(ArrayKind::kHphpArray, AllocationMode::smart, 0)
    , m_used(0)
    , m_hLoad(0)
    , m_nextKI(0) {
#ifdef PEDANTIC
  if (size > 0x7fffffffU) {
    raise_error("Cannot create an array with more than 2^31 - 1 elements");
  }
#endif
  assert(m_size == 0);
  init(capacity);
}

HphpArray::HphpArray(uint size, const TypedValue* values)
    : ArrayData(ArrayKind::kHphpArray, AllocationMode::smart, size)
    , m_used(size)
    , m_hLoad(size)
    , m_nextKI(size) {
#ifdef PEDANTIC
  if (size > 0x7fffffffU) {
    raise_error("Cannot create an array with more than 2^31 - 1 elements");
  }
#endif
  initWithoutHash(size);
  assert(size <= m_tableMask + 1);
  // append values by moving -- Caller assumes we update refcount.  Values
  // are in reverse order since they come from the stack, which grows down.
  // This code is hand-specialized from nextInsert().
  assert(m_size == size && m_hLoad == size && m_nextKI == size);
  ElmInd* hash = m_hash;
  Elm* data = m_data;
  uint i = 0;
  for (; i < size; i++) {
    const TypedValue& tv = values[size - i - 1];
    data[i].data.m_data = tv.m_data;
    data[i].data.m_type = tv.m_type;
    data[i].setIntKey(i);
    hash[i] = i;
  }
  // Initialize the leftover hash
  for (; i <= m_tableMask; i++) {
    hash[i] = ElmIndEmpty;
  }
  assert(m_size == size);
  assert(m_hLoad == size);
  assert(m_used == size);
  assert(m_nextKI == size);
  assert(size == 0 || m_pos == 0);
}

HphpArray::HphpArray(EmptyMode)
    : ArrayData(ArrayKind::kHphpArray, AllocationMode::smart, 0)
    , m_used(0)
    , m_hLoad(0)
    , m_nextKI(0) {
  init(0);
  setStatic();
}

// Empty constructor for internal use by nonSmartCopy() and copyImpl()
HphpArray::HphpArray(AllocationMode mode) :
    ArrayData(ArrayKind::kHphpArray, mode) {
}

HOT_FUNC_VM
HphpArray::~HphpArray() {
  auto const elms = m_data;
  auto const used = m_used;
  for (uint32_t pos = 0; pos < used; ++pos) {
    auto& e = elms[pos];
    if (isTombstone(e.data.m_type)) continue;
    if (e.hasStrKey()) decRefStr(e.key);
    tvRefcountedDecRef(&e.data);
  }
  if (m_data == m_inline_data.slots) {
    return;
  }
  if (m_allocMode == AllocationMode::smart) {
    smart_free(elms);
  } else {
    free(elms);
  }
}

ssize_t HphpArray::vsize() const {
  assert(false && "vsize() called, but m_size should "
                  "never be -1 in HphpArray");
  return m_size;
}

//=============================================================================
// Iteration.

inline ssize_t HphpArray::prevElm(Elm* elms, ssize_t ei) const {
  assert(ei <= ssize_t(m_used));
  while (ei > 0) {
    --ei;
    if (!isTombstone(elms[ei].data.m_type)) {
      return ei;
    }
  }
  return (ssize_t)ElmIndEmpty;
}

ssize_t HphpArray::iter_begin() const {
  return nextElm(m_data, ElmIndEmpty);
}

ssize_t HphpArray::iter_end() const {
  return prevElm(m_data, m_used);
}

ssize_t HphpArray::iter_advance(ssize_t pos) const {
  assert(ArrayData::invalid_index == -1);
  // Since m_used is always less than 2^32 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead
  // of a signed comparison.
  if (size_t(++pos) < m_used && !isTombstone(m_data[pos].data.m_type)) {
    return pos;
  }
  return iter_advance_helper(pos);
}

// caller has already incremented pos but encountered a tombstone
ssize_t HphpArray::iter_advance_helper(ssize_t next_pos) const {
  Elm* elms = m_data;
  // Since m_used is always less than 2^32 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead of
  // a signed comparison.
  for (auto limit = m_used; size_t(next_pos) < limit; ++next_pos) {
    if (!isTombstone(elms[next_pos].data.m_type)) {
      return next_pos;
    }
  }
  return ArrayData::invalid_index;
}

ssize_t HphpArray::iter_rewind(ssize_t pos) const {
  if (pos == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  return prevElm(m_data, pos);
}

Variant HphpArray::getKey(ssize_t pos) const {
  assert(pos != ArrayData::invalid_index);
  Elm* e = &m_data[pos];
  assert(!isTombstone(e->data.m_type));
  if (e->hasStrKey()) {
    return e->key; // String key.
  }
  return e->ikey; // Integer key.
}

Variant HphpArray::getValue(ssize_t pos) const {
  assert(pos != ArrayData::invalid_index);
  Elm* e = &m_data[pos];
  assert(!isTombstone(e->data.m_type));
  return tvAsCVarRef(&e->data);
}

CVarRef HphpArray::getValueRef(ssize_t pos) const {
  assert(pos != ArrayData::invalid_index);
  Elm* e = &m_data[pos];
  assert(!isTombstone(e->data.m_type));
  return tvAsCVarRef(&e->data);
}

bool HphpArray::isVectorData() const {
  if (m_size == 0) {
    return true;
  }
  Elm* elms = m_data;
  int64_t i = 0;
  for (uint32_t pos = 0, limit = m_used; pos < limit; ++pos) {
    Elm* e = &elms[pos];
    if (isTombstone(e->data.m_type)) {
      continue;
    }
    if (e->hasStrKey() || e->ikey != i) {
      return false;
    }
    ++i;
  }
  return true;
}

Variant HphpArray::reset() {
  Elm* elms = m_data;
  m_pos = ssize_t(nextElm(elms, ElmIndEmpty));
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &elms[(ElmInd)m_pos];
    return tvAsCVarRef(&e->data);
  }
  m_pos = ArrayData::invalid_index;
  return false;
}

Variant HphpArray::prev() {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = m_data;
    m_pos = prevElm(elms, m_pos);
    if (m_pos != ArrayData::invalid_index) {
      Elm* e = &elms[m_pos];
      return tvAsCVarRef(&e->data);
    }
  }
  return false;
}

Variant HphpArray::next() {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = m_data;
    m_pos = nextElm(elms, m_pos);
    if (m_pos != ArrayData::invalid_index) {
      Elm* e = &elms[m_pos];
      assert(!isTombstone(e->data.m_type));
      return tvAsCVarRef(&e->data);
    }
  }
  return false;
}

Variant HphpArray::end() {
  Elm* elms = m_data;
  m_pos = prevElm(elms, m_used);
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &elms[m_pos];
    assert(!isTombstone(e->data.m_type));
    return tvAsCVarRef(&e->data);
  }
  return false;
}

Variant HphpArray::key() const {
  if (m_pos != ArrayData::invalid_index) {
    assert(size_t(m_pos) < m_used);
    Elm* e = &m_data[m_pos];
    assert(!isTombstone(e->data.m_type));
    if (e->hasStrKey()) {
      return e->key;
    }
    return e->ikey;
  }
  return uninit_null();
}

Variant HphpArray::value(int32_t& pos) const {
  if (pos != ArrayData::invalid_index) {
    Elm* e = &m_data[pos];
    assert(!isTombstone(e->data.m_type));
    return tvAsCVarRef(&e->data);
  }
  return false;
}

Variant HphpArray::current() const {
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &m_data[m_pos];
    assert(!isTombstone(e->data.m_type));
    return tvAsCVarRef(&e->data);
  }
  return false;
}

static StaticString s_value("value");
static StaticString s_key("key");

Variant HphpArray::each() {
  if (m_pos != ArrayData::invalid_index) {
    ArrayInit init(4);
    Variant key = HphpArray::getKey(m_pos);
    Variant value = HphpArray::getValue(m_pos);
    init.set(int64_t(1), value);
    init.set(s_value, value, true);
    init.set(int64_t(0), key);
    init.set(s_key, key, true);
    m_pos = nextElm(m_data, m_pos);
    return Array(init.create());
  }
  return false;
}

//=============================================================================
// Lookup.

#define STRING_HASH(x)   (int32_t(x) | 0x80000000)

static bool hitStringKey(const HphpArray::Elm* e, const StringData* s,
                         int32_t hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HphpArray::isTombstone(e->data.m_type));

  if (e->hash() != hash) {
    return false;
  }
  if (e->key == s) {
    return true;
  }
  const char* data = e->key->data();
  const char* sdata = s->data();
  int slen = s->size();
  return data == sdata || ((e->key->size() == slen)
                          && (memcmp(data, sdata, slen) == 0));
}

static bool hitIntKey(const HphpArray::Elm* e, int64_t ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  assert(!HphpArray::isTombstone(e->data.m_type));
  return e->ikey == ki && e->hasIntKey();
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.

#define FIND_BODY(h0, hit) \
  size_t tableMask = m_tableMask; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Elm* elms = m_data; \
  ssize_t pos = m_hash[probeIndex]; \
  if ((validElmInd(pos) && hit) || pos == ssize_t(ElmIndEmpty)) { \
    return pos; \
  } \
  /* Quadratic probe. */ \
  for (size_t i = 1;; ++i) { \
    assert(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    assert(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    pos = m_hash[probeIndex]; \
    if ((validElmInd(pos) && hit) || pos == ssize_t(ElmIndEmpty)) { \
      return pos; \
    } \
  }

NEVER_INLINE
ssize_t HphpArray::find(int64_t ki) const {
  if (uint64_t(ki) < m_size) {
    // Try to get at it without dirtying a data cache line.
    Elm* e = m_data + uint64_t(ki);
    if (!isTombstone(e->data.m_type) && hitIntKey(e, ki)) {
      Stats::inc(Stats::HA_FindIntFast);
      assert([&] {
          // Our results had better match the other path
          FIND_BODY(ki, hitIntKey(&elms[pos], ki));
      }() == ki);
      return ki;
    }
  }
  Stats::inc(Stats::HA_FindIntSlow);
  FIND_BODY(ki, hitIntKey(&elms[pos], ki));
}

NEVER_INLINE
ssize_t HphpArray::find(const StringData* s,
                                   strhash_t prehash) const {
  int32_t h = STRING_HASH(prehash);
  FIND_BODY(prehash, hitStringKey(&elms[pos], s, h));
}
#undef FIND_BODY

NEVER_INLINE
HphpArray::ElmInd* warnUnbalanced(size_t n, HphpArray::ElmInd* ei) {
  raise_error("Array is too unbalanced (%lu)", n);
  return ei;
}

#define FIND_FOR_INSERT_BODY(h0, hit)                                   \
  ElmInd* ret = nullptr;                                                \
  size_t tableMask = m_tableMask;                                       \
  size_t probeIndex = size_t(h0) & tableMask;                           \
  Elm* elms = m_data;                                                   \
  ElmInd* ei = &m_hash[probeIndex];                                     \
  ssize_t pos = *ei;                                                    \
  if ((validElmInd(pos) && hit) || pos == ssize_t(ElmIndEmpty)) {       \
    return ei;                                                          \
  }                                                                     \
  if (!validElmInd(pos)) ret = ei;                                      \
  /* Quadratic probe. */                                                \
  for (size_t i = 1;; ++i) {                                            \
    assert(i <= tableMask);                                             \
    probeIndex = (probeIndex + i) & tableMask;                          \
    assert(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex);  \
    ei = &m_hash[probeIndex];                                           \
    pos = ssize_t(*ei);                                                 \
    if (validElmInd(pos)) {                                             \
      if (hit) {                                                        \
        assert(m_hLoad <= computeMaxElms(tableMask));                   \
        return ei;                                                      \
      }                                                                 \
    } else {                                                            \
      if (!ret) ret = ei;                                               \
      if (pos == ElmIndEmpty) {                                         \
        assert(m_hLoad <= computeMaxElms(tableMask));                   \
        return LIKELY(i <= 100) ||                                      \
          LIKELY(i <= size_t(RuntimeOption::MaxArrayChain)) ?           \
          ret : warnUnbalanced(i, ret);                                 \
      }                                                                 \
    }                                                                   \
  }

NEVER_INLINE
HphpArray::ElmInd* HphpArray::findForInsert(int64_t ki) const {
  FIND_FOR_INSERT_BODY(ki, hitIntKey(&elms[pos], ki));
}

NEVER_INLINE
HphpArray::ElmInd* HphpArray::findForInsert(const StringData* s,
                                            strhash_t prehash) const {
  int32_t h = STRING_HASH(prehash);
  FIND_FOR_INSERT_BODY(prehash, hitStringKey(&elms[pos], s, h));
}
#undef FIND_FOR_INSERT_BODY

NEVER_INLINE HphpArray::ElmInd*
HphpArray::findForNewInsertLoop(size_t tableMask, size_t h0) const {
  /* Quadratic probe. */
  size_t probeIndex = h0 & tableMask;
  for (size_t i = 1;; ++i) {
    assert(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    assert(((h0 + ((i + i * i) >> 1)) & tableMask) == probeIndex);
    ElmInd* ei = &m_hash[probeIndex];
    ssize_t pos = ssize_t(*ei);
    if (!validElmInd(pos)) {
      return ei;
    }
  }
}

bool HphpArray::exists(int64_t k) const {
  return find(k) != (ssize_t)ElmIndEmpty;
}

bool HphpArray::exists(const StringData* k) const {
  ssize_t pos = find(k, k->hash());
  return pos != ssize_t(ElmIndEmpty);
}

CVarRef HphpArray::get(int64_t k, bool error /* = false */) const {
  ElmInd pos = find(k);
  if (pos != ElmIndEmpty) {
    Elm* e = &m_data[pos];
    return tvAsCVarRef(&e->data);
  }
  return error ? getNotFound(k) : null_variant;
}

CVarRef HphpArray::get(const StringData* key, bool error /* = false */) const {
  ElmInd pos = find(key, key->hash());
  if (pos != ElmIndEmpty) {
    Elm* e = &m_data[pos];
    return tvAsCVarRef(&e->data);
  }
  return error ? getNotFound(key) : null_variant;
}

//=============================================================================
// Append/insert/update.

inline ALWAYS_INLINE bool HphpArray::isFull() const {
  uint32_t maxElms = computeMaxElms(m_tableMask);
  assert(m_used <= maxElms);
  assert(m_hLoad <= maxElms);
  return m_used == maxElms || m_hLoad == maxElms;
}

inline ALWAYS_INLINE HphpArray::Elm* HphpArray::allocElmFast(ElmInd* ei) {
  assert(!validElmInd(*ei) && !isFull());
  assert(m_size != 0 || m_used == 0);
#ifdef PEDANTIC
  if (m_size >= 0x7fffffffU) {
    raise_error("Cannot insert into array with 2^31 - 1 elements");
    return nullptr;
  }
#endif
  ++m_size;
  m_hLoad += (*ei == ElmIndEmpty);
  ElmInd i = m_used++;
  (*ei) = i;
  return &m_data[i];
}

inline ALWAYS_INLINE HphpArray::Elm* HphpArray::allocElm(ElmInd* ei) {
  Elm* e = allocElmFast(ei);
  if (m_pos == ArrayData::invalid_index) m_pos = ssize_t(*ei);
  return e;
}

inline ALWAYS_INLINE
HphpArray::Elm* HphpArray::newElm(ElmInd* ei, size_t h0) {
  if (isFull()) return newElmGrow(h0);
  return allocElm(ei);
}

NEVER_INLINE
HphpArray::Elm* HphpArray::newElmGrow(size_t h0) {
  resize();
  return allocElm(findForNewInsert(h0));
}

inline ALWAYS_INLINE
void HphpArray::initElmInt(Elm* e, int64_t ki, CVarRef rhs, bool isRef) {
  if (isRef) {
    tvAsUninitializedVariant(&e->data).constructRefHelper(rhs);
  } else {
    tvAsUninitializedVariant(&e->data).constructValHelper(rhs);
  }
  e->setIntKey(ki);
}

inline ALWAYS_INLINE
void HphpArray::initElmStr(Elm* e, strhash_t h, StringData* key, CVarRef rhs,
                           bool isRef) {
  if (isRef) {
    tvAsUninitializedVariant(&e->data).constructRefHelper(rhs);
  } else {
    tvAsUninitializedVariant(&e->data).constructValHelper(rhs);
  }
  e->setStrKey(key, h);
  key->incRefCount();
}

inline ALWAYS_INLINE
void HphpArray::newElmInt(ElmInd* ei, int64_t ki, CVarRef data,
                               bool byRef) {
  initElmInt(newElm(ei, ki), ki, data, byRef);
}

inline ALWAYS_INLINE
void HphpArray::newElmStr(ElmInd* ei, strhash_t h, StringData* key,
                               CVarRef data, bool byRef) {
  initElmStr(newElm(ei, h), h, key, data, byRef);
}

void HphpArray::allocData(size_t maxElms, size_t tableSize) {
  if (maxElms <= SmallSize) {
    m_data = m_inline_data.slots;
    m_hash = m_inline_data.hash;
    return;
  }
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  if (m_allocMode == AllocationMode::smart) {
    m_data = (Elm*) smart_malloc(allocSize);
  } else {
    m_data = (Elm*) Util::safe_malloc(allocSize);
  }
  m_hash = hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
           (ElmInd*)(uintptr_t(m_data) + dataSize);
}

void HphpArray::reallocData(size_t maxElms, size_t tableSize, uint oldMask) {
  assert(m_data && oldMask > 0 && maxElms > SmallSize);
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  size_t oldDataSize = computeMaxElms(oldMask) * sizeof(Elm); // slots only.
  if (m_allocMode == AllocationMode::smart) {
    if (m_data == m_inline_data.slots) {
      m_data = (Elm*) smart_malloc(allocSize);
      copyData:
      memcpy(m_data, m_inline_data.slots, oldDataSize);
    } else {
      m_data = (Elm*) smart_realloc(m_data, allocSize);
    }
  } else {
    if (m_data == m_inline_data.slots) {
      m_data = (Elm*) Util::safe_malloc(allocSize);
      // This goto doesn't loop, just saves the memcpy call code.
      goto copyData;
    }
    m_data = (Elm*) Util::safe_realloc(m_data, allocSize);
  }
  m_hash = hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
           (ElmInd*)(uintptr_t(m_data) + dataSize);
}

inline ALWAYS_INLINE void HphpArray::resizeIfNeeded() {
  if (isFull()) resize();
}

NEVER_INLINE void HphpArray::resize() {
  uint32_t maxElms = computeMaxElms(m_tableMask);
  assert(m_used <= maxElms);
  assert(m_hLoad <= maxElms);
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  bool doGrow = (m_size > (maxElms >> 1));
#ifdef PEDANTIC
  if (m_tableMask > 0x7fffffffU && doGrow) {
    // If the hashtable is at its maximum size, we cannot grow
    doGrow = false;
    // Check if compaction would actually make room for at least one new
    // element. If not, raise an error.
    if (m_size >= 0x7fffffffU) {
      raise_error("Cannot grow an array with 2^31 - 1 elements");
      return;
    }
  }
#endif
  if (doGrow) {
    grow();
  } else {
    compact();
  }
}

void HphpArray::grow() {
  assert(m_tableMask <= 0x7fffffffU);
  uint32_t oldMask = m_tableMask;
  m_tableMask = (uint)(size_t(m_tableMask) + size_t(m_tableMask) + size_t(1));
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  reallocData(maxElms, tableSize, oldMask);
  // All the elements have been copied and their offsets from the base are
  // still the same, so we just need to build the new hash table.
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_size until after rebuilding is complete,
  // in order to maintain invariants in findForNewInsert().
  m_hLoad = 0;
#else
  m_hLoad = m_size;
#endif
  if (m_size > 0) {
    Elm* elms = m_data;
    for (uint32_t pos = 0, limit = m_used; pos < limit; ++pos) {
      Elm* e = &elms[pos];
      if (isTombstone(e->data.m_type)) {
        continue;
      }
      ElmInd* ei = findForNewInsert(e->hasIntKey() ? e->ikey : e->hash());
      *ei = pos;
    }
#ifdef DEBUG
    m_hLoad = m_size;
#endif
  }
}

void HphpArray::compact(bool renumber /* = false */) {
  ElmKey mPos;
  if (m_pos != ArrayData::invalid_index) {
    // Cache key for element associated with m_pos in order to update m_pos
    // below.
    assert(size_t(m_pos) < m_used);
    Elm* e = &(m_data[(ElmInd)m_pos]);
    mPos.hash = e->hasIntKey() ? 0 : e->hash();
    mPos.key = e->key;
  } else {
    // Silence compiler warnings.
    mPos.hash = 0;
    mPos.key = nullptr;
  }
  TinyVector<ElmKey, 3> siKeys;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    ElmInd ei = r.front()->m_pos;
    if (ei != ElmIndEmpty) {
      Elm* e = &m_data[ei];
      siKeys.push_back(ElmKey(e->hash(), e->key));
    }
  }
  if (renumber) {
    m_nextKI = 0;
  }
  Elm* elms = m_data;
  size_t tableSize = computeTableSize(m_tableMask);
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_size until after rebuilding is complete,
  // in order to maintain invariants in findForNewInsert().
  m_hLoad = 0;
#else
  m_hLoad = m_size;
#endif
  for (uint32_t frPos = 0, toPos = 0; toPos < m_size; ++toPos, ++frPos) {
    while (isTombstone(elms[frPos].data.m_type)) {
      assert(frPos + 1 < m_used);
      ++frPos;
    }
    Elm& toE = elms[toPos];
    if (toPos != frPos) {
      toE = elms[frPos];
    }
    if (renumber && !toE.hasStrKey()) {
      toE.ikey = m_nextKI++;
    }
    ElmInd* ie = findForNewInsert(toE.hasIntKey() ? toE.ikey : toE.hash());
    *ie = toPos;
  }
  m_used = m_size;
#ifdef DEBUG
  m_hLoad = m_size;
#endif
  if (m_pos != ArrayData::invalid_index) {
    // Update m_pos, now that compaction is complete.
    if (mPos.hash) {
      m_pos = ssize_t(find(mPos.key, mPos.hash));
    } else {
      m_pos = ssize_t(find(mPos.ikey));
    }
  }
  // Update strong iterators, now that compaction is complete.
  int key = 0;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->m_pos != ArrayData::invalid_index) {
      ElmKey &k = siKeys[key];
      key++;
      if (k.hash) { // string key
        fp->m_pos = ssize_t(find(k.key, k.hash));
      } else { // int key
        fp->m_pos = ssize_t(find(k.ikey));
      }
    }
  }
}

static inline void elemConstruct(const TypedValue* fr, TypedValue* to) {
  tvDupCell(tvToCell(fr), to);
}

bool HphpArray::nextInsert(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  assert(!validElmInd(*ei));
  // Allocate and initialize a new element.
  initElmInt(allocElm(ei), ki, data);
  // Update next free element.
  ++m_nextKI;
  return true;
}

ArrayData* HphpArray::nextInsertRef(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return this;
  }
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  initElmInt(allocElm(ei), ki, data, true /*byRef*/);
  // Update next free element.
  ++m_nextKI;
  return this;
}

ArrayData* HphpArray::nextInsertWithRef(CVarRef data) {
  resizeIfNeeded();
  int64_t ki = m_nextKI;
  ElmInd* ei = findForInsert(ki);
  assert(!validElmInd(*ei));

  // Allocate a new element.
  Elm* e = allocElm(ei);
  tvWriteNull(&e->data);
  tvAsVariant(&e->data).setWithRef(data);
  // Set key.
  e->setIntKey(ki);
  // Update next free element.
  ++m_nextKI;
  return this;
}

ArrayData* HphpArray::addLvalImpl(int64_t ki, Variant** pDest) {
  assert(pDest != nullptr);
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    *pDest = &tvAsVariant(&m_data[*ei].data);
    return this;
  }
  Elm* e = newElm(ei, ki);
  tvWriteNull(&e->data);
  e->setIntKey(ki);
  *pDest = &(tvAsVariant(&e->data));
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return this;
}

ArrayData* HphpArray::addLvalImpl(StringData* key, strhash_t h, Variant** pDest) {
  assert(key != nullptr && pDest != nullptr);
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    TypedValue* tv;
    tv = &e->data;
    *pDest = &tvAsVariant(tv);
    return this;
  }
  Elm* e = newElm(ei, h);
  // Initialize element to null and store the address of the element into
  // *pDest.
  tvWriteNull(&e->data);
  // Set key.
  e->setStrKey(key, h);
  e->key->incRefCount();
  *pDest = &(tvAsVariant(&e->data));
  return this;
}

inline ArrayData* HphpArray::addVal(int64_t ki, CVarRef data) {
  assert(!exists(ki));
  resizeIfNeeded();
  ElmInd* ei = findForNewInsert(ki);
  Elm* e = allocElm(ei);
  TypedValue* fr = (TypedValue*)(&data);
  TypedValue* to = (TypedValue*)(&e->data);
  elemConstruct(fr, to);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return this;
}

inline ArrayData* HphpArray::addVal(StringData* key, CVarRef data) {
  assert(!exists(key));
  resizeIfNeeded();
  strhash_t h = key->hash();
  ElmInd* ei = findForNewInsert(h);
  Elm *e = allocElm(ei);
  // Set the element
  TypedValue* to = (TypedValue*)(&e->data);
  TypedValue* fr = (TypedValue*)(&data);
  elemConstruct(fr, to);
  // Set the key after data is written
  e->setStrKey(key, h);
  e->key->incRefCount();
  return this;
}

inline ArrayData* HphpArray::addValWithRef(int64_t ki, CVarRef data) {
  resizeIfNeeded();
  ElmInd* ei = findForInsert(ki);
  if (!validElmInd(*ei)) {
    Elm* e = allocElm(ei);
    tvWriteNull(&e->data);
    tvAsVariant(&e->data).setWithRef(data);
    e->setIntKey(ki);
    if (ki >= m_nextKI) {
      m_nextKI = ki + 1;
    }
  }
  return this;
}

inline ArrayData* HphpArray::addValWithRef(StringData* key, CVarRef data) {
  resizeIfNeeded();
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (!validElmInd(*ei)) {
    Elm* e = allocElm(ei);
    tvWriteNull(&e->data);
    tvAsVariant(&e->data).setWithRef(data);
    e->setStrKey(key, h);
    e->key->incRefCount();
  }
  return this;
}

inline INLINE_SINGLE_CALLER
ArrayData* HphpArray::update(int64_t ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignValHelper(data);
    return this;
  }
  newElmInt(ei, ki, data);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return this;
}

inline INLINE_SINGLE_CALLER
ArrayData* HphpArray::update(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignValHelper(data);
    return this;
  }
  newElmStr(ei, h, key, data);
  return this;
}

ArrayData* HphpArray::updateRef(int64_t ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignRefHelper(data);
    return this;
  }
  newElmInt(ei, ki, data, true /*byRef*/);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return this;
}

ArrayData* HphpArray::updateRef(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignRefHelper(data);
    return this;
  }
  newElmStr(ei, h, key, data, true /*byRef*/);
  return this;
}

ArrayData* HphpArray::lval(int64_t k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  if (!copy) return addLvalImpl(k, &ret);
  if (checkExist) {
    auto pos = find(k);
    if (pos != (ssize_t)ElmIndEmpty) {
      Elm* e = &m_data[pos];
      if (tvAsVariant(&e->data).isReferenced() ||
          tvAsVariant(&e->data).isObject()) {
        ret = &tvAsVariant(&e->data);
        return this;
      }
    }
  }
  return copyImpl()->addLvalImpl(k, &ret);
}

ArrayData* HphpArray::lval(StringData* key, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  strhash_t prehash = key->hash();
  if (!copy) return addLvalImpl(key, prehash, &ret);
  if (checkExist) {
    auto pos = find(key, prehash);
    if (pos != (ssize_t)ElmIndEmpty) {
      Elm* e = &m_data[pos];
      TypedValue* tv = &e->data;
      if (tvAsVariant(tv).isReferenced() ||
          tvAsVariant(tv).isObject()) {
        ret = &tvAsVariant(tv);
        return this;
      }
    }
  }
  return copyImpl()->addLvalImpl(key, prehash, &ret);
}

ArrayData *HphpArray::createLvalPtr(StringData* key, Variant*& ret, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->addLvalImpl(key, key->hash(), &ret);
}

ArrayData *HphpArray::getLvalPtr(StringData* key, Variant*& ret, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  auto pos = a->find(key, key->hash());
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* e = &a->m_data[pos];
    ret = &tvAsVariant(&e->data);
  } else {
    ret = nullptr;
  }
  return a;
}

ArrayData* HphpArray::lvalNew(Variant*& ret, bool copy) {
  TypedValue* tv;
  ArrayData* a = nvNew(tv, copy);
  if (tv == nullptr) {
    ret = &(Variant::lvalBlackHole());
  } else {
    ret = &tvAsVariant(tv);
  }
  return a;
}

ArrayData* HphpArray::set(int64_t k, CVarRef v, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->update(k, v);
}

ArrayData* HphpArray::set(StringData* k, CVarRef v, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->update(k, v);
}

ArrayData* HphpArray::setRef(int64_t k, CVarRef v, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->updateRef(k, v);
}

ArrayData* HphpArray::setRef(StringData* k, CVarRef v, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->updateRef(k, v);
}

ArrayData* HphpArray::add(int64_t k, CVarRef v, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->addVal(k, v);
}

ArrayData* HphpArray::add(StringData* k, CVarRef v, bool copy) {
  assert(!exists(k));
  HphpArray* a = !copy ? this : copyImpl();
  return a->addVal(k, v);
}

ArrayData* HphpArray::addLval(int64_t k, Variant*& ret, bool copy) {
  assert(!exists(k));
  HphpArray* a = !copy ? this : copyImpl();
  return a->addLvalImpl(k, &ret);
}

ArrayData* HphpArray::addLval(StringData* k, Variant*& ret, bool copy) {
  assert(!exists(k));
  HphpArray* a = !copy ? this : copyImpl();
  return a->addLvalImpl(k, k->hash(), &ret);
}

//=============================================================================
// Delete.

ArrayData* HphpArray::erase(ElmInd* ei, bool updateNext /* = false */) {
  ElmInd pos = *ei;
  if (!validElmInd(pos)) {
    return this;
  }

  Elm* elms = m_data;

  ElmInd eIPrev = ElmIndTombstone;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->m_pos == ssize_t(pos)) {
      if (eIPrev == ElmIndTombstone) {
        // eIPrev will actually be used, so properly initialize it with the
        // previous element before pos, or ElmIndEmpty if pos is the first
        // element.
        eIPrev = prevElm(elms, pos);
      }
      if (eIPrev == ElmIndEmpty) {
        fp->setResetFlag(true);
      }
      fp->m_pos = ssize_t(eIPrev);
    }
  }

  // If the internal pointer points to this element, advance it.
  if (m_pos == ssize_t(pos)) {
    ElmInd eINext = nextElm(elms, pos);
    m_pos = ssize_t(eINext);
  }

  Elm* e = &elms[pos];
  // Mark the value as a tombstone.
  TypedValue* tv = &e->data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfInvalid;
  // Free the key if necessary, and clear the h and key fields in order to
  // increase the chances that subsequent searches will quickly/safely fail
  // when encountering tombstones, even though checking for KindOfInvalid is
  // the last validation step during search.
  if (e->hasStrKey()) {
    decRefStr(e->key);
    e->setIntKey(0);
  } else {
    // Match PHP 5.3.1 semantics
    // Hacky: don't removed the unsigned cast, else g++ can optimize away
    // the check for == 0x7fff..., since there is no signed int k
    // for which k-1 == 0x7fff...
    if ((uint64_t)e->ikey == (uint64_t)m_nextKI-1
          && (e->ikey == 0x7fffffffffffffffLL || updateNext)) {
      --m_nextKI;
    }
  }
  --m_size;
  // If this element was last, adjust m_used.
  if (size_t(pos + 1) == m_used) {
    do {
      --m_used;
    } while (m_used > 0 && isTombstone(elms[m_used - 1].data.m_type));
  }
  // Mark the hash entry as "deleted".
  *ei = ElmIndTombstone;
  assert(m_used <= computeMaxElms(m_tableMask));
  assert(m_hLoad <= computeMaxElms(m_tableMask));

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);

  if (m_size < m_used / 2) {
    // Compact in order to keep elms from being overly sparse.
    compact();
  }
  return this;
}

ArrayData* HphpArray::remove(int64_t k, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->erase(a->findForInsert(k));
}

ArrayData* HphpArray::remove(const StringData* key, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  return a->erase(a->findForInsert(key, key->hash()));
}

ArrayData* HphpArray::copy() const {
  return copyImpl();
}

ArrayData* HphpArray::copyWithStrongIterators() const {
  HphpArray* copied = copyImpl();
  moveStrongIterators(copied, const_cast<HphpArray*>(this));
  return copied;
}

//=============================================================================
// non-variant interface

TypedValue* HphpArray::nvGetCell(int64_t k) const {
  ElmInd pos = find(k);
  return LIKELY(pos != ElmIndEmpty) ? tvToCell(&m_data[pos].data) :
         nvGetNotFound(k);
}

TypedValue* HphpArray::nvGetCell(const StringData* k) const {
  ElmInd pos = find(k, k->hash());
  return LIKELY(pos != ElmIndEmpty) ? tvToCell(&m_data[pos].data) :
         nvGetNotFound(k);
}

TypedValue* HphpArray::nvGet(int64_t ki) const {
  ElmInd pos = find(ki);
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    return &e->data;
  }
  return nullptr;
}

TypedValue* HphpArray::nvGet(const StringData* k) const {
  ElmInd pos = find(k, k->hash());
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    return &e->data;
  }
  return nullptr;
}

ArrayData* HphpArray::nvNew(TypedValue*& ret, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  if (UNLIKELY(!a->nextInsert(uninit_null()))) {
    ret = nullptr;
    return a;
  }
  assert(a->m_used > 0);
  ret = &a->m_data[a->m_used - 1].data;
  return a;
}

TypedValue* HphpArray::nvGetValueRef(ssize_t pos) {
  assert(pos != ArrayData::invalid_index);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  assert(!isTombstone(e->data.m_type));
  return &e->data;
}

// nvGetKey does not touch out->_count, so can be used
// for inner or outer cells.
void HphpArray::nvGetKey(TypedValue* out, ssize_t pos) {
  assert(pos != ArrayData::invalid_index);
  assert(!isTombstone(m_data[pos].data.m_type));
  Elm* e = &m_data[/*(ElmInd)*/pos];
  getElmKey(e, out);
}

/*
 * Insert a new element with index k in to the array,
 * doing nothing and returning false if the element
 * already exists.
 */
bool HphpArray::nvInsert(StringData *k, TypedValue *data) {
  strhash_t h = k->hash();
  ElmInd* ei = findForInsert(k, h);
  if (validElmInd(*ei)) {
    return false;
  }
  newElmStr(ei, h, k, tvAsVariant(data));
  return true;
}

ArrayData* HphpArray::append(CVarRef v, bool copy) {
  HphpArray *a = !copy ? this : copyImpl();
  a->nextInsert(v);
  return a;
}

/*
 * Cold path helper for AddNewElemC delegates to the ArrayData::append
 * virtual method.
 */
static NEVER_INLINE
ArrayData* genericAddNewElemC(ArrayData* a, TypedValue value) {
  ArrayData* r = a->append(tvAsCVarRef(&value), a->getCount() != 1);
  if (UNLIKELY(r != a)) {
    r->incRefCount();
    decRefArr(a);
  }
  tvRefcountedDecRef(value);
  return r;
}

/*
 * The pass-by-value and move semantics of this helper are slightly different
 * than other array helpers, but tuned for the opcode.  See doc comment in
 * hphp_array.h.
 */
ArrayData* HphpArray::AddNewElemC(ArrayData* a, TypedValue value) {
  assert(value.m_type != KindOfRef);
  HphpArray* h;
  ElmInd* ei;
  int64_t k;
  if (LIKELY(a->isHphpArray()) &&
      ((h = (HphpArray*)a), LIKELY(h->m_pos >= 0)) &&
      LIKELY(h->getCount() <= 1) &&
      LIKELY(!h->isFull()) &&
      ((k = h->m_nextKI), LIKELY(k >= 0)) &&
      ((ei = &h->m_hash[k & h->m_tableMask]), LIKELY(!validElmInd(*ei)))) {
    // Fast path is a streamlined copy of Variant.constructValHelper()
    // with no incref+decref because we're moving (data,type) to this array.
    Elm* e = h->allocElmFast(ei);
    e->data.m_type = typeInitNull(value.m_type);
    e->data.m_data.num = value.m_data.num;
    e->setIntKey(k);
    h->m_nextKI = k + 1;
    return a;
  }
  return genericAddNewElemC(a, value);
}

ArrayData* HphpArray::appendRef(CVarRef v, bool copy) {
  HphpArray *a = !copy ? this : copyImpl();
  return a->nextInsertRef(v);
}

ArrayData *HphpArray::appendWithRef(CVarRef v, bool copy) {
  HphpArray *a = !copy ? this : copyImpl();
  return a->nextInsertWithRef(v);
}

ArrayData* HphpArray::plus(const ArrayData* elems, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    CVarRef value = it.secondRef();
    if (key.isNumeric()) {
      a->addValWithRef(key.toInt64(), value);
    } else {
      a->addValWithRef(key.getStringData(), value);
    }
  }
  return a;
}

ArrayData* HphpArray::merge(const ArrayData* elems, bool copy) {
  HphpArray* a = !copy ? this : copyImpl();
  for (ArrayIter it(elems); !it.end(); it.next()) {
    Variant key = it.first();
    CVarRef value = it.secondRef();
    if (key.isNumeric()) {
      a->nextInsertWithRef(value);
    } else {
      Variant *p;
      StringData *sd = key.getStringData();
      a->addLvalImpl(sd, sd->hash(), &p);
      p->setWithRef(value);
    }
  }
  return a;
}

ArrayData* HphpArray::pop(Variant& value) {
  HphpArray* a = getCount() <= 1 ? this : copyImpl();
  Elm* elms = a->m_data;
  ElmInd pos = a->HphpArray::iter_end();
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    assert(!isTombstone(e->data.m_type));
    value = tvAsCVarRef(&e->data);
    ElmInd* ei = e->hasStrKey()
        ? a->findForInsert(e->key, e->hash())
        : a->findForInsert(e->ikey);
    a->erase(ei, true);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, ElmIndEmpty);
  return a;
}

ArrayData* HphpArray::dequeue(Variant& value) {
  HphpArray* a = getCount() <= 1 ? this : copyImpl();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  a->freeStrongIterators();
  Elm* elms = a->m_data;
  ElmInd pos = a->nextElm(elms, ElmIndEmpty);
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    value = tvAsCVarRef(&e->data);
    a->erase(e->hasStrKey() ?
             a->findForInsert(e->key, e->hash()) :
             a->findForInsert(e->ikey));
    a->compact(true);
  } else {
    value = uninit_null();
  }
  // To conform to PHP behavior, the dequeue operation resets the array's
  // internal iterator
  a->m_pos = ssize_t(a->nextElm(elms, ElmIndEmpty));
  return a;
}

ArrayData* HphpArray::prepend(CVarRef v, bool copy) {
  HphpArray* a = getCount() <= 1 ? this : copyImpl();
  // To conform to PHP behavior, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  a->freeStrongIterators();

  Elm* elms = a->m_data;
  if (a->m_used == 0 || !isTombstone(elms[0].data.m_type)) {
    // Make sure there is room to insert an element.
    a->resizeIfNeeded();
    // Reload elms, in case resizeIfNeeded() had side effects.
    elms = a->m_data;
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], a->m_used * sizeof(Elm));
    ++a->m_used;
  }

  // Prepend.
  Elm* e = &elms[0];

  TypedValue* fr = (TypedValue*)(&v);
  TypedValue* to = (TypedValue*)(&e->data);
  elemConstruct(fr, to);

  e->setIntKey(0);
  ++a->m_size;

  // Renumber.
  a->compact(true);
  // To conform to PHP behavior, the prepend operation resets the array's
  // internal iterator
  a->m_pos = ssize_t(a->nextElm(elms, ElmIndEmpty));
  return a;
}

void HphpArray::renumber() {
  compact(true);
}

void HphpArray::onSetEvalScalar() {
  Elm* elms = m_data;
  for (uint32_t pos = 0, limit = m_used; pos < limit; ++pos) {
    Elm* e = &elms[pos];
    if (!isTombstone(e->data.m_type)) {
      StringData *key = e->key;
      if (e->hasStrKey() && !key->isStatic()) {
        StringData *skey = StringData::GetStaticString(key);
        if (key->decRefCount() == 0) {
          DELETE(StringData)(key);
        }
        e->key = skey;
      }
      tvAsVariant(&e->data).setEvalScalar();
    }
  }
}

bool HphpArray::validFullPos(const FullPos &fp) const {
  assert(fp.getContainer() == (ArrayData*)this);
  if (fp.getResetFlag()) return false;
  return (fp.m_pos != ssize_t(ElmIndEmpty));
}

bool HphpArray::advanceFullPos(FullPos& fp) {
  Elm* elms = m_data;
  if (fp.getResetFlag()) {
    fp.setResetFlag(false);
    fp.m_pos = ElmIndEmpty;
  } else if (fp.m_pos == ssize_t(ElmIndEmpty)) {
    return false;
  }
  fp.m_pos = nextElm(elms, fp.m_pos);
  if (fp.m_pos == ssize_t(ElmIndEmpty)) {
    return false;
  }
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  m_pos = nextElm(elms, fp.m_pos);
  return true;
}

CVarRef HphpArray::endRef() {
  assert(m_used > 0);
  Elm* e = &m_data[m_used - 1];
  return tvAsCVarRef(&e->data);
}

//=============================================================================

ALWAYS_INLINE HphpArray* HphpArray::clone(AllocationMode am) const {
  const auto p = am == AllocationMode::smart
    ? HphpArray::AllocatorType::getNoCheck()->alloc(sizeof(HphpArray))
    : operator new(sizeof(HphpArray));
  auto target = new(p) HphpArray(am);

  if (m_size) {
    cloneNonEmpty(target);
    return target;
  }

  // Over-optimize for empty arrays; this case seems to be exceedingly
  // frequent. Do this only for arrays that actually don't allocate
  // data so the copied array doesn't lose capacity.
  target->ArrayData::m_pos = invalid_index;
  assert(target->ArrayData::m_allocMode == am);
  // Conservatively copy m_nextKI
  target->m_nextKI = m_nextKI;
  target->m_tableMask = SmallHashSize - 1;
  target->m_size = 0;
  target->m_hLoad = 0;
  target->m_used = 0;
  target->m_data = target->m_inline_data.slots;
  auto const ht = target->m_inline_data.hash;
  target->m_hash = ht;
  static_assert(SmallHashSize == 4, "review code below");
  ht[0] = ElmIndEmpty;
  ht[1] = ElmIndEmpty;
  ht[2] = ElmIndEmpty;
  ht[3] = ElmIndEmpty;
  return target;
}

NEVER_INLINE ArrayData* HphpArray::nonSmartCopy() const {
  return clone(AllocationMode::nonSmart);
}

NEVER_INLINE HphpArray* HphpArray::copyImpl() const {
  return clone(AllocationMode::smart);
}

NEVER_INLINE void HphpArray::cloneNonEmpty(HphpArray* target) const {
  target->m_pos = m_pos;
  target->m_data = nullptr;
  target->m_nextKI = m_nextKI;
  target->m_tableMask = m_tableMask;
  target->m_size = m_size;
  target->m_hLoad = m_hLoad;
  target->m_used = m_used;
  const auto tableSize = computeTableSize(m_tableMask);
  const auto maxElms = computeMaxElms(m_tableMask);
  target->allocData(maxElms, tableSize);
  // Copy the hash.
  memcpy(target->m_hash, m_hash, tableSize * sizeof(ElmInd));

  // Copy the elements and bump up refcounts as needed.
  Elm* elms = m_data;
  Elm* targetElms = target->m_data;
  for (uint32_t pos = 0, limit = m_used; pos < limit; ++pos) {
    Elm* e = &elms[pos];
    Elm* te = &targetElms[pos];
    if (!isTombstone(e->data.m_type)) {
      te->key = e->key;
      te->data.hash() = e->data.hash();
      if (te->hasStrKey()) te->key->incRefCount();
      tvDupFlattenVars(&e->data, &te->data, this);
      assert(te->hash() == e->hash()); // ensure not clobbered.
    } else {
      // Tombstone.
      te->data.m_type = KindOfInvalid;
    }
  }
  // It's possible that there were indirect elements at the end that were
  // converted to tombstones, so check if we should adjust target->m_used
  while (target->m_used > 0) {
    auto i = target->m_used - 1;
    if (!isTombstone(targetElms[i].data.m_type)) {
      break;
    }
    target->m_used = i;
  }
  // If the element density dropped below 50% due to indirect elements
  // being converted into tombstones, we should do a compaction
  if (target->m_size < target->m_used / 2) {
    target->compact();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
