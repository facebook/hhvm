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

#define INLINE_VARIANT_HELPER 1

#include <runtime/base/array/hphp_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/shared/shared_map.h>
#include <util/hash.h>
#include <util/lock.h>
#include <util/alloc.h>
#include <util/trace.h>
#include <util/util.h>
#include <runtime/base/tv_macros.h>
#include <runtime/base/execution_context.h>
#include <runtime/vm/stats.h>

// If PEDANTIC is defined, extra checks are performed to ensure correct
// function even as an array approaches 2^31 elements.  In practice this is
// just wasted effort though, since such an array would require on the order of
// 128 GiB of memory.
//#define PEDANTIC

namespace HPHP {

static const Trace::Module TRACEMOD = Trace::runtime;
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
IMPLEMENT_SMART_ALLOCATION(HphpArray);

//=============================================================================
// Static members.

HphpArray HphpArray::s_theEmptyArray(StaticEmptyArray);

//=============================================================================
// Helpers.

static inline size_t computeMaskFromNumElms(uint32 numElms) {
  ASSERT(numElms <= 0x7fffffffU);
  size_t lgSize = HphpArray::MinLgTableSize;
  size_t maxElms = (size_t(3U)) << (lgSize-2);
  ASSERT(lgSize >= 2);
  while (maxElms < numElms) {
    ++lgSize;
    maxElms <<= 1;
  }
  ASSERT(lgSize <= 32);
  // return 2^lgSize - 1
  return ((size_t(1U)) << lgSize) - 1;
  static_assert(HphpArray::MinLgTableSize >= 2,
                "lower limit for 0.75 load factor");
}

static inline bool isIntegerKey(CVarRef v) __attribute__((always_inline));
static inline bool isIntegerKey(CVarRef v) {
  if (v.getRawType() <= KindOfInt64) return true;
  if (v.getRawType() != KindOfRef) return false;
  if (v.getRefData()->getRawType() <= KindOfInt64) return true;
  return false;
}

//=============================================================================
// Construction/destruction.

inline void HphpArray::init(uint size) {
  m_size = 0;
  m_tableMask = computeMaskFromNumElms(size);
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  allocData(maxElms, tableSize);
  initHash(m_hash, tableSize);
  m_pos = ArrayData::invalid_index;
}

HphpArray::HphpArray(uint size)
  : m_data(NULL), m_nextKI(0), m_hLoad(0), m_lastE(ElmIndEmpty),
    m_nonsmart(false) {
#ifdef PEDANTIC
  if (size > 0x7fffffffU) {
    raise_error("Cannot create an array with more than 2^31 - 1 elements");
  }
#endif
  init(size);
}

HphpArray::HphpArray(uint size, const TypedValue* values)
  : m_data(NULL), m_nextKI(0), m_hLoad(0), m_lastE(ElmIndEmpty),
    m_nonsmart(false) {
#ifdef PEDANTIC
  if (size > 0x7fffffffU) {
    raise_error("Cannot create an array with more than 2^31 - 1 elements");
  }
#endif
  init(size);
  ASSERT(size <= m_tableMask + 1);
  // append values by moving -- Caller assumes we update refcount.  Values
  // are in reverse order since they come from the stack, which grows down.
  // This code is hand-specialized from nextInsert().
  ASSERT(m_size == 0 && m_hLoad == 0 && m_nextKI == 0);
  ElmInd* hash = m_hash;
  Elm* data = m_data;
  for (uint i = 0; i < size; i++) {
    ASSERT(hash[i] == HphpArray::ElmIndEmpty);
    data[i].data = values[size - i - 1];
    data[i].setIntKey(i);
    hash[i] = i;
  }
  m_size = size;
  m_hLoad = size;
  m_lastE = size - 1;
  m_nextKI = size;
}

HphpArray::HphpArray(EmptyMode)
  : m_data(NULL), m_nextKI(0), m_hLoad(0), m_lastE(ElmIndEmpty),
    m_nonsmart(false) {
  init(0);
  setStatic();
}

// Empty constructor for internal use by nonSmartCopy() and copyImpl()
HphpArray::HphpArray(CopyMode mode) : m_nonsmart(mode == kNonSmartCopy) {
}

HOT_FUNC_VM
HphpArray::~HphpArray() {
  Elm* elms = m_data;
  ssize_t lastE = (ssize_t)m_lastE;
  for (ssize_t /*ElmInd*/ pos = 0; pos <= lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type == KindOfTombstone) {
      continue;
    }
    if (e->hasStrKey()) {
      if (e->key->decRefCount() == 0) {
        e->key->release();
      }
    }
    TypedValue* tv = &e->data;
    if (IS_REFCOUNTED_TYPE(tv->m_type)) {
      tvDecRef(tv);
    }
  }
  if (m_allocMode == kSmart) {
    smart_free(m_data);
  } else if (m_allocMode == kMalloc) {
    free(m_data);
  }
}

ssize_t HphpArray::vsize() const {
  ASSERT(false && "vsize() called, but m_size should "
                  "never be -1 in HphpArray");
  return m_size;
}

void HphpArray::dumpDebugInfo() const {
  size_t maxElms = computeMaxElms(m_tableMask);
  size_t tableSize = computeTableSize(m_tableMask);

  fprintf(stderr,
          "--- dumpDebugInfo(this=0x%08zx) ----------------------------\n",
         uintptr_t(this));
  fprintf(stderr, "m_data = %p\tm_hash = %p\n"
         "m_tableMask = %u\tm_size = %d\tm_hLoad = %d\n"
         "m_nextKI = %lld\t\tm_lastE = %d\tm_pos = %zd\n",
         m_data, m_hash, m_tableMask, m_size, m_hLoad,
         m_nextKI, m_lastE, m_pos);
  fprintf(stderr, "Elements:\n");
  ssize_t lastE = m_lastE;
  Elm* elms = m_data;
  for (ssize_t /*ElmInd*/ i = 0; i <= lastE; ++i) {
    if (elms[i].data.m_type < KindOfTombstone) {
      Variant v = tvAsVariant(&elms[i].data);
      VariableSerializer vs(VariableSerializer::DebugDump);
      String s = vs.serialize(v, true);
      if (elms[i].hasStrKey()) {
        String k = Util::escapeStringForCPP(elms[i].key->data(),
                                            elms[i].key->size());
        fprintf(stderr, "  [%3d] hash=0x%016x key=\"%s\" data=(%.*s)\n",
               int(i), elms[i].hash, k.data(), s.size()-1, s.data());
      } else {
        fprintf(stderr, "  [%3d] ind=%lld data.m_type=(%.*s)\n", int(i),
               elms[i].ikey, s.size()-1, s.data());
      }
    } else {
      fprintf(stderr, "  [%3d] <tombstone>\n", int(i));
    }
  }
  if (size_t(m_lastE+1) < maxElms) {
    fprintf(stderr, "  [%3d..%-3zd] <uninitialized>\n", m_lastE+1, maxElms-1);
  }
  fprintf(stderr, "Hash table:");
  for (size_t i = 0; i < tableSize; ++i) {
    if ((i % 8) == 0) {
      fprintf(stderr, "\n  [%3zd..%-3zd]", i, i+7);
    }
    switch (m_hash[i]) {
    default: fprintf(stderr, "%12d", m_hash[i]); break;
    case ElmIndTombstone: fprintf(stderr, "%12s", "<tombstone>"); break;
    case ElmIndEmpty: fprintf(stderr, "%12s", "<empty>"); break;
    }
  }
  fprintf(stderr, "\n");
  fprintf(stderr,
          "---------------------------------------------------------------\n");
}

//=============================================================================
// Iteration.

inline /*ElmInd*/ ssize_t HphpArray::nextElm(Elm* elms,
                                             /*ElmInd*/ ssize_t ei) const {
  ASSERT(ei >= -1);
  ssize_t lastE = m_lastE;
  while (ei < lastE) {
    ++ei;
    if (elms[ei].data.m_type < KindOfTombstone) {
      return ei;
    }
  }
  return (ssize_t)ElmIndEmpty;
}

inline /*ElmInd*/ ssize_t HphpArray::prevElm(Elm* elms,
                                             /*ElmInd*/ ssize_t ei) const {
  ASSERT(ei <= (ssize_t)(m_lastE+1));
  while (ei > 0) {
    --ei;
    if (elms[ei].data.m_type < KindOfTombstone) {
      return ei;
    }
  }
  return (ssize_t)ElmIndEmpty;
}

ssize_t HphpArray::iter_begin() const {
  return nextElm(m_data, ElmIndEmpty);
}

ssize_t HphpArray::iter_end() const {
  return prevElm(m_data, (ssize_t)(m_lastE + 1));
}

ssize_t HphpArray::iter_advance(ssize_t pos) const {
  ssize_t lastE = m_lastE;
  ASSERT(ArrayData::invalid_index == -1);
  // Since lastE is always less than 2^32-1 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead
  // of a signed comparison.
  if (size_t(pos) < size_t(lastE) &&
      m_data[pos + 1].data.m_type < KindOfTombstone) {
    return pos + 1;
  }
  return iter_advance_helper(pos);
}

ssize_t HphpArray::iter_advance_helper(ssize_t pos) const {
  Elm* elms = m_data;
  ssize_t lastE = m_lastE;
  // Since lastE is always less than 2^32-1 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead of
  // a signed comparison.
  while (size_t(pos) < size_t(lastE)) {
    ++pos;
    if (elms[pos].data.m_type < KindOfTombstone) {
      return pos;
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
  ASSERT(pos != ArrayData::invalid_index);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (e->hasStrKey()) {
    return e->key; // String key.
  }
  return e->ikey; // Integer key.
}

Variant HphpArray::getValue(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  return tvAsCVarRef(&e->data);
}

CVarRef HphpArray::getValueRef(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  return tvAsCVarRef(&e->data);
}

bool HphpArray::isVectorData() const {
  if (m_size == 0) {
    return true;
  }
  Elm* elms = m_data;
  int64 i = 0;
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type == KindOfTombstone) {
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
      ASSERT(e->data.m_type != KindOfTombstone);
      return tvAsCVarRef(&e->data);
    }
  }
  return false;
}

Variant HphpArray::end() {
  Elm* elms = m_data;
  m_pos = prevElm(elms, (ssize_t)(m_lastE+1));
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &elms[m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    return tvAsCVarRef(&e->data);
  }
  return false;
}

Variant HphpArray::key() const {
  if (m_pos != ArrayData::invalid_index) {
    ASSERT(m_pos <= (ssize_t)m_lastE);
    Elm* e = &m_data[(ElmInd)m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (e->hasStrKey()) {
      return e->key;
    }
    return e->ikey;
  }
  return null;
}

Variant HphpArray::value(ssize_t& pos) const {
  if (pos != ArrayData::invalid_index) {
    Elm* e = &m_data[pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    return tvAsCVarRef(&e->data);
  }
  return false;
}

Variant HphpArray::current() const {
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &m_data[m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
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
    init.set(int64(1), value);
    init.set(s_value, value, true);
    init.set(int64(0), key);
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
  ASSERT(e->data.m_type != HphpArray::KindOfTombstone);

  if (e->hash != hash) {
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

static bool hitIntKey(const HphpArray::Elm* e, int64 ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  ASSERT(e->data.m_type != HphpArray::KindOfTombstone);
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
  ssize_t /*ElmInd*/ pos = m_hash[probeIndex]; \
  if (LIKELY(pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit))) { \
    return pos; \
  } \
  /* Quadratic probe. */ \
  for (size_t i = 1;; ++i) { \
    ASSERT(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    ASSERT(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    pos = m_hash[probeIndex]; \
    if (pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit)) { \
      return pos; \
    } \
  }

NEVER_INLINE
ssize_t /*ElmInd*/ HphpArray::find(int64 ki) const {
  FIND_BODY(ki, hitIntKey(&elms[pos], ki));
}

NEVER_INLINE
ssize_t /*ElmInd*/ HphpArray::find(const StringData* s,
                                   strhash_t prehash) const {
  int32_t h = STRING_HASH(prehash);
  FIND_BODY(prehash, hitStringKey(&elms[pos], s, h));
}
#undef FIND_BODY

#define FIND_FOR_INSERT_BODY(h0, hit) \
  ElmInd* ret = NULL; \
  size_t tableMask = m_tableMask; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Elm* elms = m_data; \
  ElmInd* ei = &m_hash[probeIndex]; \
  ssize_t /*ElmInd*/ pos = *ei; \
  if (LIKELY(pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit))) { \
    return ei; \
  } \
  if (!validElmInd(pos)) ret = ei; \
  /* Quadratic probe. */ \
  for (size_t i = 1;; ++i) { \
    ASSERT(i <= tableMask); \
    probeIndex = (probeIndex + i) & tableMask; \
    ASSERT(((size_t(h0)+((i + i*i) >> 1)) & tableMask) == probeIndex); \
    ei = &m_hash[probeIndex]; \
    pos = ssize_t(*ei); \
    if (validElmInd(pos)) { \
      if (hit) { \
        ASSERT(m_hLoad <= computeMaxElms(tableMask)); \
        return ei; \
      } \
    } else { \
      if (ret == NULL) { \
        ret = ei; \
      } \
      if (pos == ElmIndEmpty) { \
        ASSERT(m_hLoad <= computeMaxElms(tableMask)); \
        return ret; \
      } \
    } \
  }

NEVER_INLINE
HphpArray::ElmInd* HphpArray::findForInsert(int64 ki) const {
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
    ASSERT(i <= tableMask);
    probeIndex = (probeIndex + i) & tableMask;
    ASSERT(((h0 + ((i + i * i) >> 1)) & tableMask) == probeIndex);
    ElmInd* ei = &m_hash[probeIndex];
    ssize_t pos = ssize_t(*ei);
    if (!validElmInd(pos)) {
      return ei;
    }
  }
}

bool HphpArray::exists(int64 k) const {
  return find(k) != (ssize_t)ElmIndEmpty;
}

bool HphpArray::exists(litstr k) const {
  StringData s(k, strlen(k), AttachLiteral);
  ssize_t /*ElmInd*/ pos = find(&s, hash_string(k));
  return pos != ssize_t(ElmIndEmpty);
}

bool HphpArray::exists(CStrRef k) const {
  ssize_t /*ElmInd*/ pos = find(k.get(), k->hash());
  return pos != ssize_t(ElmIndEmpty);
}

bool HphpArray::exists(CVarRef k) const {
  if (isIntegerKey(k)) {
    return find(k.toInt64()) != (ssize_t)ElmIndEmpty;
  }
  StringData* key = k.getStringData();
  ssize_t /*ElmInd*/ pos = find(key, key->hash());
  return pos != ssize_t(ElmIndEmpty);
}

CVarRef HphpArray::get(int64 k, bool error /* = false */) const {
  ElmInd pos = find(k);
  if (pos != ElmIndEmpty) {
    Elm* e = &m_data[pos];
    return tvAsCVarRef(&e->data);
  }
  return error ? getNotFound(k) : null_variant;
}

CVarRef HphpArray::get(litstr k, bool error /* = false */) const {
  int len = strlen(k);
  StringData s(k, len, AttachLiteral);
  ElmInd pos = find(&s, hash_string(k, len));
  if (pos != ElmIndEmpty) {
    Elm* e = &m_data[pos];
    return tvAsCVarRef(&e->data);
  }
  return error ? getNotFound(k) : null_variant;
}

CVarRef HphpArray::get(CStrRef k, bool error /* = false */) const {
  StringData* key = k.get();
  strhash_t prehash = key->hash();
  ElmInd pos = find(key, prehash);
  if (pos != ElmIndEmpty) {
    Elm* e = &m_data[pos];
    return tvAsCVarRef(&e->data);
  }
  return error ? getNotFound(k) : null_variant;
}

CVarRef HphpArray::get(CVarRef k, bool error /* = false */) const {
  ElmInd pos;
  if (isIntegerKey(k)) {
    pos = find(k.toInt64());
    if (pos != ElmIndEmpty) {
      Elm* e = &m_data[pos];
      return tvAsCVarRef(&e->data);
    }
  } else {
    StringData* strkey = k.getStringData();
    strhash_t prehash = strkey->hash();
    pos = find(strkey, prehash);
    if (pos != ElmIndEmpty) {
      Elm* e = &m_data[pos];
      return tvAsCVarRef(&e->data);
    }
  }
  return error ? getNotFound(k) : null_variant;
}

ssize_t HphpArray::getIndex(int64 k) const {
  return ssize_t(find(k));
}

ssize_t HphpArray::getIndex(litstr k) const {
  size_t len = strlen(k);
  StringData s(k, len, AttachLiteral);
  return ssize_t(find(&s, hash_string(k, len)));
}

ssize_t HphpArray::getIndex(CStrRef k) const {
  return ssize_t(find(k.get(), k->hash()));
}

ssize_t HphpArray::getIndex(CVarRef k) const {
  if (isIntegerKey(k)) {
    return ssize_t(find(k.toInt64()));
  } else {
    StringData* key = k.getStringData();
    return ssize_t(find(key, key->hash()));
  }
}

//=============================================================================
// Append/insert/update.

inline ALWAYS_INLINE bool HphpArray::isFull() const {
  uint32 maxElms = computeMaxElms(m_tableMask);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE) + 1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
  return uint32(m_lastE) + 1 == maxElms || m_hLoad == maxElms;
}

inline ALWAYS_INLINE HphpArray::Elm* HphpArray::allocElm(ElmInd* ei) {
  ASSERT(!validElmInd(*ei) && !isFull());
  ASSERT(m_size != 0 || m_lastE == ElmIndEmpty);
#ifdef PEDANTIC
  if (m_size >= 0x7fffffffU) {
    raise_error("Cannot insert into array with 2^31 - 1 elements");
    return NULL;
  }
#endif
  ++m_size;
  m_hLoad += (*ei == ElmIndEmpty);
  ElmInd i = ++m_lastE;
  (*ei) = i;
  if (m_pos == ArrayData::invalid_index) m_pos = ssize_t(i);
  Elm* e = &m_data[i];
  return LIKELY(!siPastEnd()) ? e : allocElmExtra(e, ei);
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

NEVER_INLINE HphpArray::Elm* HphpArray::allocElmExtra(Elm* e, ElmInd* ei) {
  // If there could be any strong iterators that are past the end, we need to
  // do a pass and update these iterators to point to the newly added element.
  if (siPastEnd()) {
    setSiPastEnd(false);
    bool shouldWarn = false;
    for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
      FullPos* fp = r.front();
      if (fp->pos == ssize_t(ElmIndEmpty)) {
        fp->pos = ssize_t(*ei);
        shouldWarn = true;
      }
    }
    if (shouldWarn) {
      raise_warning("An element was added to an array inside foreach "
                    "by reference when iterating over the last "
                    "element. This may lead to unexpeced results.");
    }
  }
  return e;
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
  ASSERT(!m_data);
  if (maxElms <= SmallSize) {
    m_data = m_inline_data.slots;
    m_hash = m_inline_data.hash;
    m_allocMode = kInline;
    return;
  }
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  if (!m_nonsmart) {
    m_data = (Elm*) smart_malloc(allocSize);
    m_allocMode = kSmart;
  } else {
    m_data = (Elm*) Util::safe_malloc(allocSize);
    m_allocMode = kMalloc;
  }
  m_hash = hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
           (ElmInd*)(uintptr_t(m_data) + dataSize);
}

void HphpArray::reallocData(size_t maxElms, size_t tableSize, uint oldMask) {
  ASSERT(m_data && oldMask > 0 && maxElms > SmallSize);
  size_t hashSize = tableSize * sizeof(ElmInd);
  size_t dataSize = maxElms * sizeof(Elm);
  size_t allocSize = hashSize <= sizeof(m_inline_hash) ? dataSize :
                     dataSize + hashSize;
  size_t oldDataSize = computeMaxElms(oldMask) * sizeof(Elm); // slots only.
  if (!m_nonsmart) {
    ASSERT(m_allocMode == kInline || m_allocMode == kSmart);
    if (m_allocMode == kInline) {
      m_data = (Elm*) smart_malloc(allocSize);
      memcpy(m_data, m_inline_data.slots, oldDataSize);
      m_allocMode = kSmart;
    } else {
      m_data = (Elm*) smart_realloc(m_data, allocSize);
    }
  } else {
    ASSERT(m_allocMode == kInline || m_allocMode == kMalloc);
    if (m_allocMode == kInline) {
      m_data = (Elm*) Util::safe_malloc(allocSize);
      memcpy(m_data, m_inline_data.slots, oldDataSize);
      m_allocMode = kMalloc;
    } else {
      m_data = (Elm*) Util::safe_realloc(m_data, allocSize);
    }
  }
  m_hash = hashSize <= sizeof(m_inline_hash) ? m_inline_hash :
           (ElmInd*)(uintptr_t(m_data) + dataSize);
}

inline ALWAYS_INLINE void HphpArray::resizeIfNeeded() {
  if (isFull()) resize();
}

NEVER_INLINE void HphpArray::resize() {
  uint32 maxElms = computeMaxElms(m_tableMask);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE)+1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
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
  ASSERT(m_tableMask <= 0x7fffffffU);
  uint32 oldMask = m_tableMask;
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
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &elms[pos];
      if (e->data.m_type == KindOfTombstone) {
        continue;
      }
      ElmInd* ei = findForNewInsert(e->hasIntKey() ? e->ikey : e->hash);
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
    ASSERT(m_pos <= ssize_t(m_lastE));
    Elm* e = &(m_data[(ElmInd)m_pos]);
    mPos.hash = e->hasIntKey() ? 0 : e->hash;
    mPos.key = e->key;
  } else {
    // Silence compiler warnings.
    mPos.hash = 0;
    mPos.key = NULL;
  }
  TinyVector<ElmKey, 3> siKeys;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    ElmInd ei = r.front()->pos;
    if (ei != ElmIndEmpty) {
      Elm* e = &m_data[ei];
      siKeys.push_back(ElmKey(e->hash, e->key));
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
  ElmInd frPos = 0;
  for (ElmInd toPos = 0; toPos < ElmInd(m_size); ++toPos) {
    Elm* frE = &elms[frPos];
    while (frE->data.m_type == KindOfTombstone) {
      ++frPos;
      ASSERT(frPos <= m_lastE);
      frE = &elms[frPos];
    }
    Elm* toE = &elms[toPos];
    if (toE != frE) {
      memcpy((void*)toE, (void*)frE, sizeof(Elm));
    }
    if (renumber && !toE->hasStrKey()) {
      toE->ikey = m_nextKI;
      ++m_nextKI;
    }
    ElmInd* ie = findForNewInsert(toE->hasIntKey() ? toE->ikey : toE->hash);
    *ie = toPos;
    ++frPos;
  }
  m_lastE = m_size - 1;
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
    if (fp->pos != ArrayData::invalid_index) {
      ElmKey &k = siKeys[key];
      key++;
      if (k.hash) { // string key
        fp->pos = ssize_t(find(k.key, k.hash));
      } else { // int key
        fp->pos = ssize_t(find(k.ikey));
      }
    }
  }
}

#define ELEMENT_CONSTRUCT(fr, to) \
  if (fr->m_type == KindOfRef) fr = fr->m_data.pref->tv(); \
  TV_DUP_CELL_NC(fr, to); \

bool HphpArray::nextInsert(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  resizeIfNeeded();
  int64 ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  ASSERT(!validElmInd(*ei));
  // Allocate and initialize a new element.
  initElmInt(allocElm(ei), ki, data);
  // Update next free element.
  ++m_nextKI;
  return true;
}

void HphpArray::nextInsertRef(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return;
  }
  resizeIfNeeded();
  int64 ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  initElmInt(allocElm(ei), ki, data, true /*byRef*/);
  // Update next free element.
  ++m_nextKI;
}

void HphpArray::nextInsertWithRef(CVarRef data) {
  resizeIfNeeded();
  int64 ki = m_nextKI;
  ElmInd* ei = findForInsert(ki);
  ASSERT(!validElmInd(*ei));

  // Allocate a new element.
  Elm* e = allocElm(ei);
  TV_WRITE_NULL(&e->data);
  tvAsVariant(&e->data).setWithRef(data);
  // Set key.
  e->setIntKey(ki);
  // Update next free element.
  ++m_nextKI;
}

void HphpArray::addLvalImpl(int64 ki, Variant** pDest) {
  ASSERT(pDest != NULL);
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    *pDest = &tvAsVariant(&m_data[*ei].data);
    return;
  }
  Elm* e = newElm(ei, ki);
  TV_WRITE_NULL(&e->data);
  e->setIntKey(ki);
  *pDest = &(tvAsVariant(&e->data));
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
}

void HphpArray::addLvalImpl(StringData* key, strhash_t h, Variant** pDest) {
  ASSERT(key != NULL && pDest != NULL);
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    TypedValue* tv;
    tv = &e->data;
    *pDest = &tvAsVariant(tv);
    return;
  }
  Elm* e = newElm(ei, h);
  // Initialize element to null and store the address of the element into
  // *pDest.
  TV_WRITE_NULL(&e->data);
  // Set key.
  e->setStrKey(key, h);
  e->key->incRefCount();
  *pDest = &(tvAsVariant(&e->data));
}

inline void HphpArray::addVal(int64 ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  Elm* e = newElm(ei, ki);
  TypedValue* fr = (TypedValue*)(&data);
  TypedValue* to = (TypedValue*)(&e->data);
  ELEMENT_CONSTRUCT(fr, to);
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
}

inline void HphpArray::addVal(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  Elm *e = newElm(ei, h);
  // Set the element
  TypedValue* to = (TypedValue*)(&e->data);
  TypedValue* fr = (TypedValue*)(&data);
  ELEMENT_CONSTRUCT(fr, to);
  // Set the key after data is written
  e->setStrKey(key, h);
  e->key->incRefCount();
}

inline void HphpArray::addValWithRef(int64 ki, CVarRef data) {
  resizeIfNeeded();
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    return;
  }
  Elm* e = allocElm(ei);
  TV_WRITE_NULL(&e->data);
  tvAsVariant(&e->data).setWithRef(data);
  e->setIntKey(ki);
  if (ki >= m_nextKI) {
    m_nextKI = ki + 1;
  }
}

inline void HphpArray::addValWithRef(StringData* key, CVarRef data) {
  resizeIfNeeded();
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    return;
  }
  Elm* e = allocElm(ei);
  TV_WRITE_NULL(&e->data);
  tvAsVariant(&e->data).setWithRef(data);
  e->setStrKey(key, h);
  e->key->incRefCount();
}

void HphpArray::update(int64 ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignValHelper(data);
    return;
  }
  newElmInt(ei, ki, data);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
}

HOT_FUNC_VM
void HphpArray::update(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignValHelper(data);
    return;
  }
  newElmStr(ei, h, key, data);
}

void HphpArray::updateRef(int64 ki, CVarRef data) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignRefHelper(data);
    return;
  }
  newElmInt(ei, ki, data, true /*byRef*/);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
}

void HphpArray::updateRef(StringData* key, CVarRef data) {
  strhash_t h = key->hash();
  ElmInd* ei = findForInsert(key, h);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    tvAsVariant(&e->data).assignRefHelper(data);
    return;
  }
  newElmStr(ei, h, key, data, true /*byRef*/);
}

ArrayData* HphpArray::lval(int64 k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  if (!copy) {
    addLvalImpl(k, &ret);
    return NULL;
  }
  if (!checkExist) {
    HphpArray* a = copyImpl();
    a->addLvalImpl(k, &ret);
    return a;
  }
  ssize_t /*ElmInd*/ pos = find(k);
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* e = &m_data[pos];
    if (tvAsVariant(&e->data).isReferenced() ||
        tvAsVariant(&e->data).isObject()) {
      ret = &tvAsVariant(&e->data);
      return NULL;
    }
  }
  HphpArray* a = copyImpl();
  a->addLvalImpl(k, &ret);
  return a;
}

ArrayData* HphpArray::lval(litstr k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  String s(k, AttachLiteral);
  return HphpArray::lval(s, ret, copy, checkExist);
}

ArrayData* HphpArray::lval(CStrRef k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  StringData* key = k.get();
  strhash_t prehash = key->hash();
  if (!copy) {
    addLvalImpl(key, prehash, &ret);
    return NULL;
  }
  if (!checkExist) {
    HphpArray* a = copyImpl();
    a->addLvalImpl(key, prehash, &ret);
    return a;
  }
  ssize_t /*ElmInd*/ pos = find(key, prehash);
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* e = &m_data[pos];
    TypedValue* tv = &e->data;
    if (tvAsVariant(tv).isReferenced() ||
        tvAsVariant(tv).isObject()) {
      ret = &tvAsVariant(tv);
      return NULL;
    }
  }
  HphpArray* a = copyImpl();
  a->addLvalImpl(key, prehash, &ret);
  return a;
}

ArrayData* HphpArray::lval(CVarRef k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  if (isIntegerKey(k)) {
    return HphpArray::lval(k.toInt64(), ret, copy, checkExist);
  }
  return HphpArray::lval(k.toString(), ret, copy, checkExist);
}

ArrayData *HphpArray::lvalPtr(CStrRef k, Variant*& ret, bool copy,
                              bool create) {
  StringData* key = k.get();
  strhash_t prehash = key->hash();
  HphpArray* a = 0;
  HphpArray* t = this;
  if (copy) {
    a = t = copyImpl();
  }
  if (create) {
    t->addLvalImpl(key, prehash, &ret);
  } else {
    ssize_t /*ElmInd*/ pos = t->find(key, prehash);
    if (pos != (ssize_t)ElmIndEmpty) {
      Elm* e = &t->m_data[pos];
      ret = &tvAsVariant(&e->data);
    } else {
      ret = NULL;
    }
  }
  return a;
}

ArrayData *HphpArray::lvalPtr(int64 k, Variant*& ret, bool copy,
                              bool create) {
  HphpArray* a = 0;
  HphpArray* t = this;
  if (copy) {
    a = t = copyImpl();
  }

  if (create) {
    t->addLvalImpl(k, &ret);
  } else {
    ElmInd pos = t->find(k);
    if (pos != ElmIndEmpty) {
      Elm* e = &t->m_data[pos];
      ret = &tvAsVariant(&e->data);
    } else {
      ret = NULL;
    }
  }
  return a;
}

ArrayData* HphpArray::lvalNew(Variant*& ret, bool copy) {
  TypedValue* tv;
  ArrayData* a = nvNew(tv, copy);
  if (tv == NULL) {
    ret = &(Variant::lvalBlackHole());
  } else {
    ret = &tvAsVariant(tv);
  }
  return a;
}

ArrayData* HphpArray::set(int64 k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->update(k, v);
  return t;
}

ArrayData* HphpArray::set(CStrRef k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->update(k.get(), v);
  return t;
}

ArrayData* HphpArray::set(CVarRef k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  if (isIntegerKey(k)) {
    a->update(k.toInt64(), v);
  } else {
    a->update(k.getStringData(), v);
  }
  return t;
}

ArrayData* HphpArray::setRef(int64 k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->updateRef(k, v);
  return t;
}

ArrayData* HphpArray::setRef(CStrRef k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->updateRef(k.get(), v);
  return t;
}

ArrayData* HphpArray::setRef(CVarRef k, CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  if (isIntegerKey(k)) {
    a->updateRef(k.toInt64(), v);
  } else {
    a->updateRef(k.getStringData(), v);
  }
  return t;
}

ArrayData* HphpArray::add(int64 k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->addVal(k, v);
  return t;
}

ArrayData* HphpArray::add(CStrRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->addVal(k.get(), v);
  return t;
}

ArrayData* HphpArray::add(CVarRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (isIntegerKey(k)) {
    return HphpArray::add(k.toInt64(), v, copy);
  }
  return HphpArray::add(k.toString(), v, copy);
}

ArrayData* HphpArray::addLval(int64 k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->addLvalImpl(k, &ret);
  return t;
}

ArrayData* HphpArray::addLval(CStrRef k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  StringData* key = k.get();
  a->addLvalImpl(key, key->hash(), &ret);
  return t;
}

ArrayData* HphpArray::addLval(CVarRef k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  if (isIntegerKey(k)) {
    a->addLvalImpl(k.toInt64(), &ret);
  } else {
    StringData* sd = k.getStringData();
    a->addLvalImpl(sd, sd->hash(), &ret);
  }
  return t;
}

//=============================================================================
// Delete.

void HphpArray::erase(ElmInd* ei, bool updateNext /* = false */) {
  ElmInd pos = *ei;
  if (!validElmInd(pos)) {
    return;
  }

  Elm* elms = m_data;

  bool nextElementUnsetInsideForeachByReference = false;
  ElmInd eINext = ElmIndTombstone;
  for (FullPosRange r(strongIterators()); !r.empty(); r.popFront()) {
    FullPos* fp = r.front();
    if (fp->pos == ssize_t(pos)) {
      nextElementUnsetInsideForeachByReference = true;
      if (eINext == ElmIndTombstone) {
        // eINext will actually be used, so properly initialize it with the
        // next element past pos, or ElmIndEmpty if pos is the last element.
        eINext = nextElm(elms, pos);
        if (eINext == ElmIndEmpty) {
          // Remember there is a strong iterator past the end.
          setSiPastEnd(true);
        }
      }
      fp->pos = ssize_t(eINext);
    }
  }

  // If the internal pointer points to this element, advance it.
  if (m_pos == ssize_t(pos)) {
    if (eINext == ElmIndTombstone) {
      eINext = nextElm(elms, pos);
    }
    m_pos = ssize_t(eINext);
  }

  Elm* e = &elms[pos];
  // Mark the value as a tombstone.
  TypedValue* tv = &e->data;
  DataType oldType = tv->m_type;
  uint64_t oldDatum = tv->m_data.num;
  tv->m_type = KindOfTombstone;
  // Free the key if necessary, and clear the h and key fields in order to
  // increase the chances that subsequent searches will quickly/safely fail
  // when encountering tombstones, even though checking for KindOfTombstone is
  // the last validation step during search.
  if (e->hasStrKey()) {
    if (e->key->decRefCount() == 0) {
      e->key->release();
    }
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
  // If this element was last, adjust m_lastE.
  if (pos == m_lastE) {
    do {
      --m_lastE;
    } while (m_lastE >= 0 && elms[m_lastE].data.m_type == KindOfTombstone);
  }
  // Mark the hash entry as "deleted".
  *ei = ElmIndTombstone;
  ASSERT(m_lastE == ElmIndEmpty ||
         uint32(m_lastE)+1 <= computeMaxElms(m_tableMask));
  ASSERT(m_hLoad <= computeMaxElms(m_tableMask));

  // Finally, decref the old value
  tvRefcountedDecRefHelper(oldType, oldDatum);

  if (m_size < (uint32_t)((m_lastE+1) >> 1)) {
    // Compact in order to keep elms from being overly sparse.
    compact();
  }

  if (nextElementUnsetInsideForeachByReference) {
    if (RuntimeOption::EnableHipHopErrors) {
      raise_warning("The next element was unset inside foreach by reference. "
                    "This may lead to unexpeced results.");
    }
  }
}

ArrayData* HphpArray::remove(int64 k, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->erase(a->findForInsert(k));
  return t;
}

ArrayData* HphpArray::remove(CStrRef k, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  StringData* key = k.get();
  a->erase(a->findForInsert(key, key->hash()));
  return t;
}

ArrayData* HphpArray::remove(CVarRef k, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  if (isIntegerKey(k)) {
    a->erase(a->findForInsert(k.toInt64()));
  } else {
    StringData* key = k.getStringData();
    a->erase(a->findForInsert(key, key->hash()));
  }
  return t;
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

TypedValue* HphpArray::nvGetCell(int64 ki, bool error /* = false */) const {
  ElmInd pos = find(ki);
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    TypedValue* tv = &e->data;
    return (tv->m_type != KindOfRef) ? tv :
           tv->m_data.pref->tv();
  }
  return error ? nvGetNotFound(ki) : NULL;
}

inline TypedValue*
HphpArray::nvGetCell(const StringData* k, bool error /* = false */) const {
  ElmInd pos = find(k, k->hash());
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    TypedValue* tv = &e->data;
    if (tv->m_type < KindOfRef) {
      return tv;
    }
    if (LIKELY(tv->m_type == KindOfRef)) {
      return tv->m_data.pref->tv();
    }
  }
  return error ? nvGetNotFound(k) : NULL;
}

TypedValue* HphpArray::nvGet(int64 ki) const {
  ElmInd pos = find(ki);
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    return &e->data;
  }
  return NULL;
}

TypedValue* HphpArray::nvGet(const StringData* k) const {
  ElmInd pos = find(k, k->hash());
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* e = &m_data[pos];
    return &e->data;
  }
  return NULL;
}

ArrayData* HphpArray::nvSet(int64 ki, int64 vi, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->nvUpdate(ki, vi);
  return retval;
}

ArrayData* HphpArray::nvSet(int64 ki, const TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->update(ki, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvSet(StringData* k, const TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->update(k, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvAppend(const TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->nextInsert(tvAsCVarRef(v));
  return retval;
}

void HphpArray::nvAppendWithRef(const TypedValue* v) {
  nextInsertWithRef(tvAsCVarRef(v));
}

ArrayData* HphpArray::nvNew(TypedValue*& ret, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  if (UNLIKELY(!a->nextInsert(null))) {
    ret = NULL;
    return t;
  }
  ASSERT(a->m_lastE != ElmIndEmpty);
  ssize_t lastE = (ssize_t)a->m_lastE;
  ret = &a->m_data[lastE].data;
  return t;
}

TypedValue* HphpArray::nvGetValueRef(ssize_t pos) {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  return &e->data;
}

// nvGetKey does not touch out->_count, so can be used
// for inner or outer cells.
void HphpArray::nvGetKey(TypedValue* out, ssize_t pos) {
  ASSERT(pos != ArrayData::invalid_index);
  ASSERT(m_data[pos].data.m_type != KindOfTombstone);
  Elm* e = &m_data[/*(ElmInd)*/pos];
  if (e->hasIntKey()) {
    out->m_data.num = e->ikey;
    out->m_type = KindOfInt64;
    return;
  }
  out->m_data.pstr = e->key;
  out->m_type = KindOfString;
  e->key->incRefCount();
}

bool HphpArray::nvUpdate(int64 ki, int64 vi) {
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* e = &m_data[*ei];
    TypedValue* to = (TypedValue*)(&e->data);
    if (to->m_type == KindOfRef) to = to->m_data.pref->tv();
    DataType oldType = to->m_type;
    uint64_t oldDatum = to->m_data.num;
    if (IS_REFCOUNTED_TYPE(oldType)) {
      tvDecRefHelper(oldType, oldDatum);
    }
    to->m_data.num = vi;
    to->m_type = KindOfInt64;
    return true;
  }
  Elm* e = newElm(ei, ki);
  TypedValue* to = (TypedValue*)(&e->data);
  to->m_data.num = vi;
  to->m_type = KindOfInt64;
  e->setIntKey(ki);
  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return true;
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

inline ALWAYS_INLINE HphpArray* HphpArray::copyImpl(HphpArray* target) const {
  ASSERT(!target->siPastEnd());
  target->m_pos = m_pos;
  target->m_data = NULL;
  target->m_nextKI = m_nextKI;
  target->m_tableMask = m_tableMask;
  target->m_size = m_size;
  target->m_hLoad = m_hLoad;
  target->m_lastE = m_lastE;
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  target->allocData(maxElms, tableSize);
  // Copy the hash.
  memcpy(target->m_hash, m_hash, tableSize * sizeof(ElmInd));
  // Copy the elements and bump up refcounts as needed.
  if (m_size > 0) {
    Elm* elms = m_data;
    Elm* targetElms = target->m_data;
    ssize_t lastE = (ssize_t)m_lastE;
    for (ssize_t /*ElmInd*/ pos = 0; pos <= lastE; ++pos) {
      Elm* e = &elms[pos];
      Elm* te = &targetElms[pos];
      if (e->data.m_type != KindOfTombstone) {
        te->hash = e->hash;
        te->key = e->key;
        TypedValue* fr;
        if (te->hasStrKey()) {
          fr = &e->data;
          te->key->incRefCount();
        } else {
          fr = &e->data;
        }
        TypedValue* to = &te->data;
        TV_DUP_FLATTEN_VARS(fr, to, this);
        te->hash = e->hash;
      } else {
        // Tombstone.
        te->setIntKey(0);
        te->data.m_type = KindOfTombstone;
      }
    }
    // It's possible that there were indirect elements at the end that were
    // converted to tombstones, so check if we should adjust target->m_lastE
    while (target->m_lastE >= 0) {
      Elm* te = &targetElms[target->m_lastE];
      if (te->data.m_type != KindOfTombstone) {
        break;
      }
      --(target->m_lastE);
    }
    // If the element density dropped below 50% due to indirect elements
    // being converted into tombstones, we should do a compaction
    if (target->m_size < (uint32_t)((target->m_lastE+1) >> 1)) {
      target->compact();
    }
  }
  return target;
}

NEVER_INLINE ArrayData* HphpArray::nonSmartCopy() const {
  return copyImpl(new HphpArray(kNonSmartCopy));
}

NEVER_INLINE HphpArray* HphpArray::copyImpl() const {
  return copyImpl(NEW(HphpArray)(kSmartCopy));
}

ArrayData* HphpArray::append(CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->nextInsert(v);
  return t;
}

ArrayData* HphpArray::appendRef(CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->nextInsertRef(v);
  return t;
}

ArrayData *HphpArray::appendWithRef(CVarRef v, bool copy) {
  HphpArray *a = this, *t = 0;
  if (copy) a = t = copyImpl();
  a->nextInsertWithRef(v);
  return t;
}

ArrayData* HphpArray::append(const ArrayData* elems, ArrayOp op, bool copy) {
  HphpArray* a = this;
  HphpArray* result = NULL;
  if (copy) {
    result = a = copyImpl();
  }

  if (op == Plus) {
    for (ArrayIter it(elems); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      if (key.isNumeric()) {
        a->addValWithRef(key.toInt64(), value);
      } else {
        a->addValWithRef(key.getStringData(), value);
      }
    }
  } else {
    ASSERT(op == Merge);
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
  }
  return result;
}

ArrayData* HphpArray::pop(Variant& value) {
  HphpArray* a = this;
  HphpArray* result = NULL;
  if (getCount() > 1) {
    result = a = copyImpl();
  }
  Elm* elms = a->m_data;
  ElmInd pos = a->HphpArray::iter_end();
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    value = tvAsCVarRef(&e->data);
    ElmInd* ei = e->hasStrKey()
        ? a->findForInsert(e->key, e->hash)
        : a->findForInsert(e->ikey);
    a->erase(ei, true);
  } else {
    value = null;
  }
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator.
  a->m_pos = a->nextElm(elms, ElmIndEmpty);
  return result;
}

ArrayData* HphpArray::dequeue(Variant& value) {
  HphpArray* a = this;
  HphpArray* result = NULL;
  if (getCount() > 1) {
    result = a = copyImpl();
  }
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  a->freeStrongIterators();
  Elm* elms = a->m_data;
  ElmInd pos = a->nextElm(elms, ElmIndEmpty);
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    value = tvAsCVarRef(&e->data);
    a->erase(e->hasStrKey() ?
             a->findForInsert(e->key, e->hash) :
             a->findForInsert(e->ikey));
    a->compact(true);
  } else {
    value = null;
  }
  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  a->m_pos = ssize_t(a->nextElm(elms, ElmIndEmpty));
  return result;
}

ArrayData* HphpArray::prepend(CVarRef v, bool copy) {
  HphpArray* a = this;
  HphpArray* result = NULL;
  if (copy) {
    result = a = copyImpl();
  }
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  a->freeStrongIterators();

  Elm* elms = a->m_data;
  if (a->m_lastE == 0 || elms[0].data.m_type != KindOfTombstone) {
    // Make sure there is room to insert an element.
    a->resizeIfNeeded();
    // Reload elms, in case resizeIfNeeded() had side effects.
    elms = a->m_data;
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], (a->m_lastE+1) * sizeof(Elm));
    ++a->m_lastE;
  }
  // Prepend.
  Elm* e = &elms[0];

  TypedValue* fr = (TypedValue*)(&v);
  TypedValue* to = (TypedValue*)(&e->data);
  ELEMENT_CONSTRUCT(fr, to);

  e->setIntKey(0);
  ++a->m_size;

  // Renumber.
  a->compact(true);
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  a->m_pos = ssize_t(a->nextElm(elms, ElmIndEmpty));
  return result;
}

void HphpArray::renumber() {
  compact(true);
}

void HphpArray::onSetEvalScalar() {
  Elm* elms = m_data;
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type != KindOfTombstone) {
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

void HphpArray::getFullPos(FullPos& fp) {
  ASSERT(fp.container == (ArrayData*)this);
  fp.pos = m_pos;
  if (fp.pos == ssize_t(ElmIndEmpty)) {
    // Remember there is a strong iterator past the end.
    setSiPastEnd(true);
  }
}

bool HphpArray::setFullPos(const FullPos& fp) {
  ASSERT(fp.container == (ArrayData*)this);
  if (fp.pos != ssize_t(ElmIndEmpty)) {
    m_pos = fp.pos;
    return true;
  }
  return false;
}

CVarRef HphpArray::currentRef() {
  ASSERT(m_pos != ArrayData::invalid_index);
  Elm* e = &m_data[(ElmInd)m_pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  return tvAsCVarRef(&e->data);
}

CVarRef HphpArray::endRef() {
  ASSERT(m_lastE != ElmIndEmpty);
  ElmInd pos = m_lastE;
  Elm* e = &m_data[pos];
  return tvAsCVarRef(&e->data);
}

//=============================================================================
// VM runtime support functions.
namespace VM {

// Helpers for array_setm.
template<typename Value>
inline ArrayData* nv_set_with_integer_check(ArrayData* a, StringData* key,
                                            Value value, bool copy) {
  int64 lval;
  if (UNLIKELY(key->isStrictlyInteger(lval))) {
    return a->nvSet(lval, value, copy);
  } else {
    return a->nvSet(key, value, copy);
  }
}

template<typename Value>
ArrayData* nvCheckedSet(ArrayData* a, StringData* sd, Value value, bool copy) {
  return nv_set_with_integer_check<Value>(a, sd, value, copy);
}

template<typename Value>
ArrayData* nvCheckedSet(ArrayData* a, int64 key, Value value, bool copy) {
  return a->nvSet(key, value, copy);
}

void setmDecRef(int64 i) { /* nop */ }
void setmDecRef(TypedValue* tv) { tvRefcountedDecRef(tv); }
void setmDecRef(StringData* sd) { if (sd->decRefCount() == 0) sd->release(); }

static inline HphpArray*
array_mutate_pre(ArrayData* ad) {
  ASSERT(ad);
  return (HphpArray*)ad;
}

VarNR toVar(int64 i)        { return VarNR(i); }
Variant &toVar(TypedValue* tv) { return tvCellAsVariant(tv); }

static inline ArrayData*
array_mutate_post(Cell *cell, ArrayData* old, ArrayData* retval) {
  if (NULL == retval) {
    return old;
  }
  retval->incRefCount();
  // TODO: It would be great if there were nvSet() methods that didn't
  // bump up the refcount so that we didn't have to decrement it here
  if (old->decRefCount() == 0) old->release();
  if (cell) cell->m_data.parr = retval;
  return retval;
}

template<typename Key, typename Value,
         bool DecRefValue, bool CheckInt, bool DecRefKey>
static inline
ArrayData*
array_setm(TypedValue* cell, ArrayData* ad, Key key, Value value) {
  ArrayData* retval;
  bool copy = ad->getCount() > 1;
  // nvSet will decRef any old value that may have been overwritten
  // if appropriate
  retval = CheckInt ? nvCheckedSet(ad, key, value, copy) :
           ad->nvSet(key, value, copy);
  if (DecRefKey) setmDecRef(key);
  if (DecRefValue) setmDecRef(value);
  return array_mutate_post(cell, ad, retval);
}

template<typename Value, bool DecRefValue>
ArrayData*
array_append(TypedValue* cell, ArrayData* ad, Value v) {
  HphpArray* ha = array_mutate_pre(ad);
  bool copy = ha->getCount() > 1;
  ArrayData* retval = ha->nvAppend(v, copy);
  if (DecRefValue) setmDecRef(v);
  return array_mutate_post(cell, ad, retval);
}

/**
 * Unary integer keys.
 *    array_setm_ik1_iv --
 *       Integer value.
 *
 *    array_setm_ik1_v --
 *       Polymorphic value.
 *
 *    array_setm_ik1_v0 --
 *       Don't count the array's reference to the polymorphic value.
 */
ArrayData*
array_setm_ik1_iv(TypedValue* cell, ArrayData* ad, int64 key, int64 value) {
  return
    array_setm<int64, int64, false, false, false>(cell, ad, key, value);
}

ArrayData*
array_setm_ik1_v(TypedValue* cell, ArrayData* ad, int64 key,
                 TypedValue* value) {
  return
    array_setm<int64, TypedValue*, false, false, false>(cell, ad, key, value);
}

ArrayData*
array_setm_ik1_v0(TypedValue* cell, ArrayData* ad, int64 key,
                  TypedValue* value) {
  return
    array_setm<int64, TypedValue*, true, false, false>(cell, ad, key, value);
}

/**
 * String keys.
 *
 *    array_setm_sk1_v --
 *      $a[$keyOfTypeString] = <polymorphic value>;
 *
 *    array_setm_sk1_v0 --
 *      Like above, but don't count the new reference.
 *
 *    array_setm_s0k1_v --
 *    array_setm_s0k1_v0 --
 *      As above, but dont decRef the key
 *
 *    array_setm_s0k1nc_v --
 *    array_setm_s0k1nc_v0 --
 *       Dont decRef the key, and skip the check for
 *       whether the key is really an integer.
 */
ArrayData* array_setm_sk1_v(TypedValue* cell, ArrayData* ad, StringData* key,
                            TypedValue* value) {
  return array_setm<StringData*, TypedValue*, false, true, true>(
    cell, ad, key, value);
}

ArrayData* array_setm_sk1_v0(TypedValue* cell, ArrayData* ad, StringData* key,
                             TypedValue* value) {
  return array_setm<StringData*, TypedValue*, true, true, true>(
    cell, ad, key, value);
}

ArrayData* array_setm_s0k1_v(TypedValue* cell, ArrayData* ad, StringData* key,
                             TypedValue* value) {
  return array_setm<StringData*, TypedValue*, false, true, false>(
    cell, ad, key, value);
}

ArrayData* array_setm_s0k1_v0(TypedValue* cell, ArrayData* ad, StringData* key,
                              TypedValue* value) {
  return array_setm<StringData*, TypedValue*, true, true, false>(
    cell, ad, key, value);
}

ArrayData* array_setm_s0k1nc_v(TypedValue* cell, ArrayData* ad, StringData* key,
                               TypedValue* value) {
  return array_setm<StringData*, TypedValue*, false, false, false>(
    cell, ad, key, value);
}

ArrayData* array_setm_s0k1nc_v0(TypedValue* cell, ArrayData* ad,
                                StringData* key, TypedValue* value) {
  return array_setm<StringData*, TypedValue*, true, false, false>(
    cell, ad, key, value);
}

/**
 * Append.
 *
 *   array_setm_wk1_v --
 *      $a[] = <polymorphic value>
 *   array_setm_wk1_v0 --
 *      ... but don't count the reference to the new value.
 */
ArrayData* array_setm_wk1_v(TypedValue* cell, ArrayData* ad,
                            TypedValue* value) {
  return array_append<TypedValue*, false>(cell, ad, value);
}

ArrayData* array_setm_wk1_v0(TypedValue* cell, ArrayData* ad,
                             TypedValue* value) {
  return array_append<TypedValue*, true>(cell, ad, value);
}


// Helpers for getm and friends.
inline TypedValue* nv_get_cell_with_integer_check(ArrayData* arr,
                                                  StringData* key) {
  int64 lval;
  if (UNLIKELY(key->isStrictlyInteger(lval))) {
    return arr->nvGetCell(lval, true /* error */);
  } else {
    return arr->nvGetCell(key, true /* error */);
  }
}

/**
 * Array runtime helpers. For code-sharing purposes, all of these handle as
 * much ref-counting machinery as possible. They differ by -arity, type
 * signature, and necessity of various costly checks.
 *
 * They return the array that was just passed in as a convenience to
 * callers, which may have "lost" the array in volatile registers before
 * calling.
 */

ArrayData*
array_getm_i(void* dptr, int64 key, TypedValue* out) {
  ASSERT(dptr);
  ArrayData* ad = (ArrayData*)dptr;
  TRACE(2, "array_getm_ik1: (%p) <- %p[%lld]\n", out, dptr, key);
  // Ref-counting the value is the translator's responsibility. We know out
  // pointed to uninitialized memory, so no need to dec it.
  TypedValue* ret = ad->nvGetCell(key, true /* error */);
  if (UNLIKELY(!ret)) {
    TV_WRITE_NULL(out);
  } else {
    tvDup(ret, out);
  }
  return ad;
}

enum GetFlags {
  DecRefKey = 1,
  CheckInts = 2,
};

static ArrayData*
array_getm_impl(ArrayData* ad, StringData* sd, int flags, TypedValue* out)
  NEVER_INLINE;
ArrayData* array_getm_impl(ArrayData* ad, StringData* sd,
                           int flags, TypedValue* out) {
  bool drKey = flags & DecRefKey;
  bool checkInts = flags & CheckInts;
  TypedValue* ret = checkInts ? nv_get_cell_with_integer_check(ad, sd) :
                    ad->nvGetCell(sd, true /*error*/);
  if (UNLIKELY(!ret)) {
    TV_WRITE_NULL(out);
  } else {
    tvDup((ret), (out));
  }
  if (drKey && sd->decRefCount() == 0) sd->release();
  TRACE(2, "%s: (%p) <- %p[\"%s\"@sd%p]\n", __FUNCTION__,
        out, ad, sd->data(), sd);
  return ad;
}

/**
 * array_getm_s: conservative unary string key.
 */
ArrayData*
array_getm_s(void* dptr, StringData* sd, TypedValue* out) {
  return array_getm_impl((ArrayData*)dptr, sd, DecRefKey | CheckInts, out);
}

/**
 * array_getm_s0: unary string key where we know there is no need
 * to decRef the key.
 */
ArrayData*
array_getm_s0(void* dptr, StringData* sd, TypedValue* out) {
  return array_getm_impl((ArrayData*)dptr, sd, CheckInts, out);
}

/**
 * array_getm_s_fast:
 * array_getm_s0:
 *
 *   array_getm_s[0] but without the integer check on the key
 */
ArrayData*
array_getm_s_fast(void* dptr, StringData* sd, TypedValue* out) {
  return array_getm_impl((ArrayData*)dptr, sd, DecRefKey, out);
}

ArrayData*
array_getm_s0_fast(void* dptr, StringData* sd, TypedValue* out) {
  return array_getm_impl((ArrayData*)dptr, sd, 0, out);
}

template<DataType keyType, bool decRefBase>
inline void non_array_getm(TypedValue* base, int64 key, TypedValue* out) {
  ASSERT(base->m_type != KindOfRef);
  TypedValue keyTV;
  keyTV.m_type = keyType;
  keyTV.m_data.num = key;
  VMExecutionContext::getElem(base, &keyTV, out);
  if (decRefBase) {
    tvRefcountedDecRef(base);
  }
  if (IS_REFCOUNTED_TYPE(keyType)) {
    tvDecRef(&keyTV);
  }
}

void
non_array_getm_i(TypedValue* base, int64 key, TypedValue* out) {
  non_array_getm<KindOfInt64, true>(base, key, out);
}

void
non_array_getm_s(TypedValue* base, StringData* key, TypedValue* out) {
  non_array_getm<KindOfString, true>(base, (intptr_t)key, out);
}

void
array_getm_is_impl(ArrayData* ad, int64 ik, StringData* sd, TypedValue* out,
                   bool decRefKey) NEVER_INLINE;
void
array_getm_is_impl(ArrayData* ad, int64 ik, StringData* sd, TypedValue* out,
                   bool decRefKey) {
  TypedValue* base2 = ad->nvGetCell(ik, true);
  if (UNLIKELY(base2 == NULL || base2->m_type != KindOfArray)) {
    if (base2 == NULL) {
      base2 = (TypedValue*)&init_null_variant;
    }
    non_array_getm<KindOfString, false>(base2, (int64)sd, out);
  } else {
    ad = base2->m_data.parr;
    array_getm_impl(ad, sd, CheckInts | (decRefKey ? DecRefKey : 0), out);
  }
}

/**
 * array_getm_is will increment the refcount of the return value if
 * appropriate and it will decrement the refcount of the string key
 */
void
array_getm_is(ArrayData* ad, int64 ik, StringData* sd, TypedValue* out) {
  array_getm_is_impl(ad, ik, sd, out, /*decRefKey=*/ true);
}

/**
 * array_getm_is0 will increment the refcount of the return value if
 * appropriate
 */
void
array_getm_is0(ArrayData* ad, int64 ik, StringData* sd, TypedValue* out) {
  array_getm_is_impl(ad, ik, sd, out, /*decRefKey=*/false);
}

// issetm's DNA.
static bool
issetMUnary(const void* dptr, StringData* sd, bool decRefKey, bool checkInt) {
  const ArrayData* ad = (const ArrayData*)dptr;
  bool retval;
  int64 keyAsInt;

  TypedValue* c;
  if (checkInt && sd->isStrictlyInteger(keyAsInt)) {
    c = ad->nvGet(keyAsInt);
  } else {
    c = ad->nvGet(sd);
  }
  const Variant* cell = &tvAsVariant(c);

  retval = cell && !cell->isNull();
  TRACE(2, "issetMUnary: %p[\"%s\"@sd%p] -> %d\n",
        dptr, sd->data(), sd, retval);

  if (decRefKey && sd->decRefCount() == 0) sd->release();
  return retval;
}

uint64 array_issetm_s(const void* dptr, StringData* sd)
{ return issetMUnary(dptr, sd, true  /*decRefKey*/, true  /*checkInt*/); }
uint64 array_issetm_s0(const void* dptr, StringData* sd)
{ return issetMUnary(dptr, sd, false /*decRefKey*/, true  /*checkInt*/); }
uint64 array_issetm_s_fast(const void* dptr, StringData* sd)
{ return issetMUnary(dptr, sd, true  /*decRefKey*/, false /*checkInt*/); }
uint64 array_issetm_s0_fast(const void* dptr, StringData* sd)
{ return issetMUnary(dptr, sd, false /*decRefKey*/, false /*checkInt*/); }

uint64 array_issetm_i(const void* dptr, int64_t key) {
  ArrayData* ad = (ArrayData*)dptr;
  TypedValue* ret = ad->nvGetCell(key, false /* error */);
  return ret && !tvAsCVarRef(ret).isNull();
}

ArrayData* array_add(ArrayData* a1, ArrayData* a2) {
  if (!a2->empty()) {
    if (a1->empty()) {
      if (a1->decRefCount() == 0) a1->release();
      return a2;
    }
    if (a1 != a2) {
      ArrayData *escalated = a1->append(a2, ArrayData::Plus,
                                        a1->getCount() > 1);
      if (escalated) {
        escalated->incRefCount();
        if (a2->decRefCount() == 0) a2->release();
        if (a1->decRefCount() == 0) a1->release();
        return escalated;
      }
    }
  }
  if (a2->decRefCount() == 0) a2->release();
  return a1;
}

}

//=============================================================================

///////////////////////////////////////////////////////////////////////////////
}
