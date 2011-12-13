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
#include <util/hash.h>
#include <util/lock.h>
#include <util/alloc.h>
#include <util/trace.h>
#include <util/util.h>
#include <runtime/base/tv_macros.h>

// If PEDANTIC is defined, extra checks are performed to ensure correct
// function even as an array approaches 2^31 elements.  In practice this is
// just wasted effort though, since such an array would require on the order of
// 128 GiB of memory.
//#define PEDANTIC

namespace HPHP {

static const Trace::Module TRACEMOD = Trace::runtime;
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION(HphpArray, SmartAllocatorImpl::NeedRestoreOnce);
//=============================================================================
// Static members.

StaticEmptyHphpArray StaticEmptyHphpArray::s_theEmptyArray;

//=============================================================================
// Helpers.

static inline size_t computeTableSize(uint32 tableMask) {
  return size_t(tableMask) + size_t(1U);
}

static inline size_t computeMaxElms(uint32 tableMask) {
  return size_t(tableMask) - (size_t(tableMask) >> 2);
}

static inline size_t computeMaskFromNumElms(uint32 numElms) {
  ASSERT(numElms <= 0x7fffffffU);
  size_t lgSize = HphpArray::MinLgTableSize;
  size_t maxElms = (size_t(3U)) << (lgSize-2);
  ASSERT(lgSize >= 2);
  ASSERT(lgSize <= 32);
  while (maxElms < numElms) {
    ++lgSize;
    maxElms <<= 1;
  }
  ASSERT(lgSize <= 32);
  // return 2^lgSize - 1
  return ((size_t(1U)) << lgSize) - 1;
}

static inline void* block2Data(void* block) {
  ASSERT((HphpArray::ElmAlignment & HphpArray::ElmAlignmentMask) == 0);
  void* data = (HphpArray::Elm*)((uintptr_t(block)
                                           + HphpArray::ElmAlignmentMask)
                                           & ~HphpArray::ElmAlignmentMask);
  ASSERT((uintptr_t(data) & HphpArray::ElmAlignmentMask) == 0);
  return data;
}

static inline HphpArray::Elm* data2Elms(void* data) {
  return (HphpArray::Elm*)data;
}

static inline HphpArray::ElmInd* elms2Hash(HphpArray::Elm* elms,
                                           size_t maxElms) {
  HphpArray::ElmInd* hash = (HphpArray::ElmInd*)(uintptr_t(elms)
                            + (maxElms * sizeof(HphpArray::Elm)));
  ASSERT((uintptr_t(hash) & HphpArray::ElmAlignmentMask) == 0);
  return hash;
}

static inline bool validElmInd(ssize_t /*HphpArray::ElmInd*/ ei) {
  return (ei > ssize_t(HphpArray::ElmIndEmpty));
}

static inline void initHash(HphpArray::ElmInd* hash, size_t tableSize) {
  ASSERT(HphpArray::ElmIndEmpty == -1);
  memset(hash, 0xffU, tableSize * sizeof(HphpArray::ElmInd));
}

static inline bool isIntegerKey(CVarRef v) __attribute__((always_inline));
static inline bool isIntegerKey(CVarRef v) {
  if (v.getRawType() <= KindOfInt64) return true;
  if (v.getRawType() != KindOfVariant) return false;
  if (v.getVariantData()->getRawType() <= KindOfInt64) return true;
  return false;
}

//=============================================================================
// Construction/destruction.

HphpArray::HphpArray(uint nSize /* = 0 */, bool suppressCow /* = false */)
  : m_data(NULL), m_nextKI(0), m_nElms(0), m_hLoad(0), m_lastE(ElmIndEmpty),
    m_linear(false), m_siPastEnd(false), m_suppressCow(suppressCow),
#ifndef USE_JEMALLOC
    m_dataPad(0),
#endif
    m_nIndirectElms(0) {
#ifdef PEDANTIC
  if (nSize > 0x7fffffffU) {
    raise_error("Cannot create an array with more than 2^31 - 1 elements");
  }
#endif
  m_tableMask = computeMaskFromNumElms(nSize);
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data);
  m_hash = elms2Hash(elms, maxElms);
  initHash(m_hash, tableSize);
  m_pos = ArrayData::invalid_index;
}

// Empty constructor for internal use.
HphpArray::HphpArray(int,int,int) {
}

HphpArray::~HphpArray() {
  Elm* elms = data2Elms(m_data);
  ssize_t lastE = (ssize_t)m_lastE;
  for (ssize_t /*ElmInd*/ pos = 0; pos <= lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type == KindOfTombstone) {
      continue;
    }
    if (e->key != NULL) {
      if (e->key->decRefCount() == 0) {
        e->key->release();
      }
    }
    TypedValue* tv = &e->data;
    if (IS_REFCOUNTED_TYPE(tv->m_type) && tv->m_type != KindOfIndirect) {
      tvDecRef(tv);
    }
  }
  if (m_data != NULL) {
    if (!m_linear) {
      free(getBlock());
    }
  }
}

ssize_t HphpArray::size() const {
  if (LIKELY(m_nIndirectElms == 0)) {
    return m_nElms;
  }
  // TODO: If we had a way to just iterate over the indirect
  // elements, we could compute this much faster
  ssize_t sz = 0;
  Elm* elms = data2Elms(m_data);
  ssize_t lastE = (ssize_t)m_lastE;
  for (ssize_t /*ElmInd*/ pos = 0; pos <= lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type < KindOfTombstone) {
      ++sz;
    } else if (e->data.m_type == KindOfIndirect) {
      TypedValue* tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type != KindOfUninit) ++sz;
    }
  }
  return sz;
}

void HphpArray::dumpDebugInfo() const {
  size_t maxElms = computeMaxElms(m_tableMask);
  size_t tableSize = computeTableSize(m_tableMask);
  Elm* elms = data2Elms(m_data);

  fprintf(stderr,
          "--- dumpDebugInfo(this=0x%08zx) ----------------------------\n",
         uintptr_t(this));
  fprintf(stderr, "m_data = %p\n"
         "elms   = %p\tm_hash = %p\n"
         "m_tableMask = %u\tm_nElms = %d\tm_hLoad = %d\n"
         "m_nextKI = %lld\t\tm_lastE = %d\tm_pos = %zd\tm_linear = %s\n",
         m_data, elms, m_hash,
         m_tableMask, m_nElms, m_hLoad, m_nextKI, m_lastE, m_pos,
         m_linear ? "true" : "false");
  fprintf(stderr, "Elements:\n");
  ssize_t lastE = m_lastE;
  for (ssize_t /*ElmInd*/ i = 0; i <= lastE; ++i) {
    if (elms[i].data.m_type < KindOfTombstone) {
      Variant v = tvAsVariant(&elms[i].data);
      VariableSerializer vs(VariableSerializer::DebugDump);
      String s = vs.serialize(v, true);
      if (elms[i].key != NULL) {
        String k = Util::escapeStringForCPP(elms[i].key->data(),
                                            elms[i].key->size());
        fprintf(stderr, "  [%3d] hash=0x%016llx key=\"%s\" data=(%.*s)\n",
               int(i), elms[i].h, k.data(), s.size()-1, s.data());
      } else {
        fprintf(stderr, "  [%3d] ind=%lld data.m_type=(%.*s)\n", int(i),
               elms[i].h, s.size()-1, s.data());
      }
    } else if (elms[i].data.m_type == KindOfIndirect) {
      // Integer keys do not support KindOfIndirect
      ASSERT(elms[i].key != NULL);
      TypedValue* fixedLocation = elms[i].data.m_data.ptv;
      Variant v = tvAsVariant(fixedLocation);
      VariableSerializer vs(VariableSerializer::DebugDump);
      String s = vs.serialize(v, true);
      String k = Util::escapeStringForCPP(elms[i].key->data(),
                                          elms[i].key->size());
      fprintf(stderr, "  [%3d] hash=0x%016llx key=\"%s\" "
              "fixed location=%p data=(%.*s)\n", int(i), elms[i].h,
              k.data(), fixedLocation, s.size()-1, s.data());
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
    } else if (elms[ei].data.m_type == KindOfIndirect) {
      TypedValue* tv = elms[ei].data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type != KindOfUninit) return ei;
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
    } else if (elms[ei].data.m_type == KindOfIndirect) {
      TypedValue* tv = elms[ei].data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type != KindOfUninit) return ei;
    }
  }
  return (ssize_t)ElmIndEmpty;
}

ssize_t HphpArray::iter_begin() const {
  Elm* elms = data2Elms(m_data);
  return nextElm(elms, ElmIndEmpty);
}

ssize_t HphpArray::iter_end() const {
  Elm* elms = data2Elms(m_data);
  return prevElm(elms, (ssize_t)(m_lastE+1));
}

ssize_t HphpArray::iter_advance(ssize_t pos) const {
  Elm* elms = data2Elms(m_data);
  ssize_t lastE = m_lastE;
  ASSERT(ArrayData::invalid_index == -1);
  // Since lastE is always less than 2^32-1 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead
  // of a signed comparison.
  if (size_t(pos) < size_t(lastE) &&
      elms[pos+1].data.m_type < KindOfTombstone) {
    return pos+1;
  }
  return iter_advance_helper(pos);
}

ssize_t HphpArray::iter_advance_helper(ssize_t pos) const {
  Elm* elms = data2Elms(m_data);
  ssize_t lastE = m_lastE;
  // Since lastE is always less than 2^32-1 and invalid_index == -1,
  // we can save a check by doing an unsigned comparison instead of
  // a signed comparison.
  while (size_t(pos) < size_t(lastE)) {
    ++pos;
    if (elms[pos].data.m_type < KindOfTombstone) {
      return pos;
    } else if (elms[pos].data.m_type == KindOfIndirect) {
      TypedValue* tv = elms[pos].data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type != KindOfUninit) return pos;
    }
  }
  return ArrayData::invalid_index;
}

ssize_t HphpArray::iter_rewind(ssize_t pos) const {
  if (pos == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  Elm* elms = data2Elms(m_data);
  return prevElm(elms, pos);
}

Variant HphpArray::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (e->key != NULL) {
    return e->key; // String key.
  }
  return e->h; // Integer key.
}

Variant HphpArray::getValue(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (LIKELY(e->data.m_type != KindOfIndirect)) {
    return tvAsCVarRef(&e->data);
  } else {
    return tvAsCVarRef(e->data.m_data.ptv);
  }
}

CVarRef HphpArray::getValueRef(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (LIKELY(e->data.m_type != KindOfIndirect)) {
    return tvAsCVarRef(&e->data);
  } else {
    return tvAsCVarRef(e->data.m_data.ptv);
  }
}

bool HphpArray::isVectorData() const {
  if (m_nElms == 0) {
    return true;
  }
  Elm* elms = data2Elms(m_data);
  int64 i = 0;
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type == KindOfTombstone) {
      continue;
    }
    if (UNLIKELY(e->data.m_type == KindOfIndirect)) {
      TypedValue* tv = e->data.m_data.ptv;
      if (tv->m_type == KindOfUninit) continue;
    }
    if (e->key != NULL || e->h != i) {
      return false;
    }
    ++i;
  }
  return true;
}

Variant HphpArray::reset() {
  Elm* elms = data2Elms(m_data);
  m_pos = ssize_t(nextElm(elms, ElmIndEmpty));
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &elms[(ElmInd)m_pos];
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      return tvAsCVarRef(e->data.m_data.ptv);
    }
  }
  m_pos = ArrayData::invalid_index;
  return false;
}

Variant HphpArray::prev() {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = data2Elms(m_data);
    m_pos = prevElm(elms, m_pos);
    if (m_pos != ArrayData::invalid_index) {
      Elm* e = &elms[m_pos];
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        return tvAsCVarRef(&e->data);
      } else {
        return tvAsCVarRef(e->data.m_data.ptv);
      }
    }
  }
  return false;
}

Variant HphpArray::next() {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = data2Elms(m_data);
    m_pos = nextElm(elms, m_pos);
    if (m_pos != ArrayData::invalid_index) {
      Elm* e = &elms[m_pos];
      ASSERT(e->data.m_type != KindOfTombstone);
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        return tvAsCVarRef(&e->data);
      } else {
        return tvAsCVarRef(e->data.m_data.ptv);
      }
    }
  }
  return false;
}

Variant HphpArray::end() {
  Elm* elms = data2Elms(m_data);
  m_pos = prevElm(elms, (ssize_t)(m_lastE+1));
  if (m_pos != ArrayData::invalid_index) {
    Elm* e = &elms[m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      return tvAsCVarRef(e->data.m_data.ptv);
    }
  }
  return false;
}

Variant HphpArray::key() const {
  if (m_pos != ArrayData::invalid_index) {
    ASSERT(m_pos <= (ssize_t)m_lastE);
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[(ElmInd)m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (e->key) {
      return e->key;
    }
    return e->h;
  }
  return null;
}

Variant HphpArray::value(ssize_t& pos) const {
  if (pos != ArrayData::invalid_index) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      return tvAsCVarRef(e->data.m_data.ptv);
    }
  }
  return false;
}

Variant HphpArray::current() const {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      return tvAsCVarRef(e->data.m_data.ptv);
    }
  }
  return false;
}

static StaticString s_value("value");
static StaticString s_key("key");

Variant HphpArray::each() {
  if (m_pos != ArrayData::invalid_index) {
    ArrayInit init(4);
    Variant key = getKey(m_pos);
    Variant value = getValue(m_pos);
    init.set(int64(1), value);
    init.set(s_value, value, true);
    init.set(int64(0), key);
    init.set(s_key, key, true);
    Elm* elms = data2Elms(m_data);
    m_pos = nextElm(elms, m_pos);
    return Array(init.create());
  }
  return false;
}

bool HphpArray::isHead() const {
  Elm* elms = data2Elms(m_data);
  return m_pos == ssize_t(nextElm(elms, ElmIndEmpty));
}

bool HphpArray::isTail() const {
  Elm* elms = data2Elms(m_data);
  return m_pos == prevElm(elms, (ssize_t)(m_lastE+1));
}

//=============================================================================
// Lookup.

static bool hitStringKey(const HphpArray::Elm* e, const char* k, int len,
                         int64 hash) {
  // hitStringKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  ASSERT(e->data.m_type != HphpArray::KindOfTombstone);

  if (e->key == NULL) {
    return false;
  }
  const char* data = e->key->data();
  return data == k || (e->h == hash && e->key->size() == len
                       && (memcmp(data, k, len) == 0));
}

static bool hitIntKey(const HphpArray::Elm* e, int64 ki) {
  // hitIntKey() should only be called on an Elm that is referenced by a
  // hash table entry. HphpArray guarantees that when it adds a hash table
  // entry that it always sets it to refer to a valid element. Likewise when
  // it removes an element it always removes the corresponding hash entry.
  // Therefore the assertion below must hold.
  ASSERT(e->data.m_type != HphpArray::KindOfTombstone);

  return e->h == ki && e->key == NULL;
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
  Elm* elms = data2Elms(m_data); \
  ssize_t /*ElmInd*/ pos = m_hash[probeIndex]; \
  if (LIKELY(pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit))) { \
    return pos; \
  } \
  /* Quadratic probe. */ \
  for (size_t i = 1, j = 1;; ++i, j += i) { \
    ASSERT(((i + i*i) >> 1) == j); \
    ASSERT(i <= tableMask); \
    probeIndex = (size_t(h0) + j) & tableMask; \
    pos = m_hash[probeIndex]; \
    if (pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit)) { \
      return pos; \
    } \
  }

ssize_t /*ElmInd*/ HphpArray::find(int64 ki) const {
  FIND_BODY(ki, hitIntKey(&elms[pos], ki));
}

ssize_t /*ElmInd*/ HphpArray::find(const char* k, int len,
                                   int64 prehash) const {
  FIND_BODY(prehash, hitStringKey(&elms[pos], k, len, prehash));
}
#undef FIND_BODY

#define FIND_FOR_INSERT_BODY(h0, hit) \
  ASSERT(!m_linear); \
  ElmInd* ret = NULL; \
  size_t tableMask = m_tableMask; \
  size_t probeIndex = size_t(h0) & tableMask; \
  Elm* elms = data2Elms(m_data); \
  ElmInd* ei = &m_hash[probeIndex]; \
  ssize_t /*ElmInd*/ pos = *ei; \
  if (LIKELY(pos == ssize_t(ElmIndEmpty) || (validElmInd(pos) && hit))) { \
    return ei; \
  } \
  if (!validElmInd(pos)) ret = ei; \
  /* Quadratic probe. */ \
  for (size_t i = 1, j = 1;; ++i, j += i) { \
    ASSERT(((i + i*i) >> 1) == j); \
    ASSERT(i <= tableMask); \
    probeIndex = (size_t(h0) + j) & tableMask; \
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

HphpArray::ElmInd* HphpArray::findForInsert(int64 ki) const {
  FIND_FOR_INSERT_BODY(ki, hitIntKey(&elms[pos], ki));
}

HphpArray::ElmInd* HphpArray::findForInsert(const char* k, int len,
                                            int64 prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitStringKey(&elms[pos], k, len, prehash));
}
#undef FIND_FOR_INSERT_BODY

HphpArray::ElmInd* HphpArray::findForNewInsert(size_t h0) const {
  size_t tableMask = m_tableMask;
  size_t probeIndex = h0 & tableMask;
  ElmInd* ei = &m_hash[probeIndex];
  ssize_t /*ElmInd*/ pos = *ei;
  if (LIKELY(!validElmInd(pos))) {
    return ei;
  }
  /* Quadratic probe. */
  for (size_t i = 1, j = 1;; ++i, j += i) {
    ASSERT(((i + i*i) >> 1) == j);
    ASSERT(i <= tableMask);
    probeIndex = (h0 + j) & tableMask;
    ei = &m_hash[probeIndex];
    pos = ssize_t(*ei);
    if (!validElmInd(pos)) {
      return ei;
    }
  }
}

bool HphpArray::exists(int64 k) const {
  return find(k) != (ssize_t)ElmIndEmpty;
}

bool HphpArray::exists(litstr k) const {
  ssize_t /*ElmInd*/ pos = find(k, strlen(k), hash_string(k));
  if (pos == (ssize_t)ElmIndEmpty) return false;
  // If we don't have an indirect elements, we are done
  if (LIKELY(m_nIndirectElms == 0)) return true;
  // Check if this is an indirect element
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[pos];
  if (e->data.m_type != KindOfIndirect) return true;
  // Return true if the type is not KindOfUninit, false otherwise
  TypedValue* tv = e->data.m_data.ptv;
  return (tv->m_type != KindOfUninit);
}

bool HphpArray::exists(CStrRef k) const {
  ssize_t /*ElmInd*/ pos = find(k.data(), k.size(), k->hash());
  if (pos == (ssize_t)ElmIndEmpty) return false;
  // If we don't have an indirect elements, we are done
  if (LIKELY(m_nIndirectElms == 0)) return true;
  // Check if this is an indirect element
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[pos];
  if (e->data.m_type != KindOfIndirect) return true;
  // Return true if the type is not KindOfUninit, false otherwise
  TypedValue* tv = e->data.m_data.ptv;
  return (tv->m_type != KindOfUninit);
}

bool HphpArray::exists(CVarRef k) const {
  if (isIntegerKey(k)) {
    return find(k.toInt64()) != (ssize_t)ElmIndEmpty;
  }
  StringData* key = k.getStringData();
  ssize_t /*ElmInd*/ pos = find(key->data(), key->size(), key->hash());
  if (pos == (ssize_t)ElmIndEmpty) return false;
  // If we don't have an indirect elements, we are done
  if (LIKELY(m_nIndirectElms == 0)) return true;
  // Check if this is an indirect element
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[pos];
  if (e->data.m_type != KindOfIndirect) return true;
  // Return true if the type is not KindOfUninit, false otherwise
  TypedValue* tv = e->data.m_data.ptv;
  return (tv->m_type != KindOfUninit);
}

bool HphpArray::idxExists(ssize_t idx) const {
  return (idx != ArrayData::invalid_index);
}

CVarRef HphpArray::get(int64 k, bool error /* = false */) const {
  ElmInd pos = find(k);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    // Integer keys do not support KindOfIndirect
    ASSERT(e->data.m_type != KindOfIndirect);
    return tvAsCVarRef(&e->data);
  }
  if (error) {
    raise_notice("Undefined index: %lld", k);
  }
  return null_variant;
}

CVarRef HphpArray::get(litstr k, bool error /* = false */) const {
  int len = strlen(k);
  ElmInd pos = find(k, len, hash_string(k, len));
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type == KindOfUninit) goto undefined_index;
      return tvAsCVarRef(tv);
    }
  }
undefined_index:
  if (error) {
    raise_notice("Undefined index: %s", k);
  }
  return null_variant;
}

CVarRef HphpArray::get(CStrRef k, bool error /* = false */) const {
  StringData* key = k.get();
  int64 prehash = key->hash();
  ElmInd pos = find(key->data(), key->size(), prehash);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return tvAsCVarRef(&e->data);
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type == KindOfUninit) goto undefined_index;
      return tvAsCVarRef(tv);
    }
  }
undefined_index:
  if (error) {
    raise_notice("Undefined index: %s", k.data());
  }
  return null_variant;
}

CVarRef HphpArray::get(CVarRef k, bool error /* = false */) const {
  ElmInd pos;
  if (isIntegerKey(k)) {
    pos = find(k.toInt64());
    if (pos != ElmIndEmpty) {
      Elm* elms = data2Elms(m_data);
      Elm* e = &elms[pos];
      // Integer keys do not support KindOfIndirect
      ASSERT(e->data.m_type != KindOfIndirect);
      return tvAsCVarRef(&e->data);
    }
  } else {
    StringData* strkey = k.getStringData();
    int64 prehash = strkey->hash();
    pos = find(strkey->data(), strkey->size(), prehash);
    if (pos != ElmIndEmpty) {
      Elm* elms = data2Elms(m_data);
      Elm* e = &elms[pos];
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        return tvAsCVarRef(&e->data);
      } else {
        TypedValue* tv = e->data.m_data.ptv;
        // Check for uninit null
        if (tv->m_type == KindOfUninit) goto undefined_index;
        return tvAsCVarRef(tv);
      }
    }
  }
undefined_index:
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null_variant;
}

Variant HphpArray::fetch(CStrRef k) const {
  StringData* key = k.get();
  int64 prehash = key->hash();
  ssize_t /*ElmInd*/ pos = find(key->data(), key->size(), prehash);
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    if (e->data.m_type != KindOfIndirect) {
      return tvAsCVarRef(&e->data);
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type == KindOfUninit) return false;
      return tvAsCVarRef(tv);
    }
  }
  return false;
}

void HphpArray::load(CVarRef k, Variant& v) const {
  ssize_t /*ElmInd*/ pos;
  if (isIntegerKey(k)) {
    pos = find(k.toInt64());
  } else {
    StringData* strkey = k.getStringData();
    int64 prehash = strkey->hash();
    pos = find(strkey->data(), strkey->size(), prehash);
  }
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    TypedValue* tv;
    if (e->data.m_type != KindOfIndirect) {
      tv = &e->data;
    } else {
      tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type == KindOfUninit) return;
    }
    v.setWithRef(tvAsCVarRef(tv));
  }
}

// NOTE: getIndex() will return a valid position even if the value
// is KindOfIndirect and it is uninit null

ssize_t HphpArray::getIndex(int64 k) const {
  return ssize_t(find(k));
}

ssize_t HphpArray::getIndex(litstr k) const {
  size_t len = strlen(k);
  return ssize_t(find(k, strlen(k), hash_string(k, len)));
}

ssize_t HphpArray::getIndex(CStrRef k) const {
  return ssize_t(find(k.data(), k.size(), k->hash()));
}

ssize_t HphpArray::getIndex(CVarRef k) const {
  if (isIntegerKey(k)) {
    return ssize_t(find(k.toInt64()));
  } else {
    StringData* key = k.getStringData();
    return ssize_t(find(key->data(), key->size(), key->hash()));
  }
}

//=============================================================================
// Append/insert/update.

HphpArray::Elm* HphpArray::allocElm(ElmInd* ei) {
  ASSERT(!m_linear);
  ASSERT(!validElmInd(*ei));
  ASSERT(m_nElms != 0 || m_lastE == ElmIndEmpty);
#ifdef PEDANTIC
  if (m_nElms >= 0x7fffffffU) {
    raise_error("Cannot insert into array with 2^31 - 1 elements");
    return NULL;
  }
#endif
  // If we need to grow first before allocating another element,
  // return NULL to indicate that allocation failed.
  uint32 maxElms = computeMaxElms(m_tableMask);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE)+1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
  if (uint32(m_lastE)+1 == maxElms || m_hLoad == maxElms) {
    return NULL;
  }
  ++m_nElms;
#if 0
  if (*ei == ElmIndEmpty) {
    ++m_hLoad;
  }
#else
  // Equivalent to disabled code above.
  ASSERT(ElmIndTombstone + 1 == ElmIndEmpty);
  m_hLoad += *ei - ElmIndTombstone;
#endif
  ++m_lastE;
  (*ei) = m_lastE;
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[m_lastE];
  if (m_pos == ArrayData::invalid_index) {
    m_pos = ssize_t(m_lastE);
  }
  // If there could be any strong iterators that are past the end, we need to
  // do a pass and update these iterators to point to the newly added element.
  if (m_siPastEnd) {
    m_siPastEnd = false;
    int sz = m_strongIterators.size();
    bool shouldWarn = false;
    for (int i = 0; i < sz; ++i) {
      if (m_strongIterators.get(i)->pos == ssize_t(ElmIndEmpty)) {
        m_strongIterators.get(i)->pos = ssize_t(*ei);
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

void HphpArray::reallocData(size_t maxElms, size_t tableSize) {
#ifdef USE_JEMALLOC
  size_t allocSize = (maxElms * sizeof(Elm)) + (tableSize * sizeof(ElmInd));
  if (m_data == NULL) {
    ASSERT(!m_linear);
    if (allocm(&m_data, NULL, allocSize, ALLOCM_ALIGN(sizeof(Elm)))) {
      throw OutOfMemoryException(allocSize);
    }
  } else if (m_linear) {
    Elm* oldElms = data2Elms(m_data);
    if (allocm(&m_data, NULL, allocSize, ALLOCM_ALIGN(sizeof(Elm)))) {
      throw OutOfMemoryException(allocSize);
    }
    Elm* elms = data2Elms(m_data);
    memcpy((void*)elms, (void*)oldElms, (m_lastE+1) * sizeof(Elm));
    m_linear = false;
  } else {
    if (rallocm(&m_data, NULL, allocSize, 0, ALLOCM_ALIGN(sizeof(Elm)))) {
      throw OutOfMemoryException(allocSize);
    }
  }
#else
  // Allocate extra padding space so that if the resulting region is not
  // Elm-aligned, there is enough padding to be able to leave the first bytes
  // unused and start the element table at an Elm alignment boundary.
  //
  // NB: It would be possible to optimistically allocate without padding, then
  // reallocate if alignment was inadquate.  However, this would not save very
  // much memory in practice, and recovering from the OOM failure case for the
  // reallocation would be messy to handle correctly.
  size_t allocSize = (maxElms * sizeof(Elm))
                     + (tableSize * sizeof(ElmInd))
                     + ElmAlignment; // <-- pad
  void* block = realloc(m_linear ? NULL : getBlock(), allocSize);
  if (block == NULL) {
    throw OutOfMemoryException(allocSize);
  }
  void* newData = block2Data(block);
  size_t newPad = uintptr_t(newData) - uintptr_t(block);
  if (!m_linear) {
    size_t oldPad = m_dataPad;
    if (newPad != oldPad) {
      // The alignment padding changed due to realloc(), so move the element
      // array to its proper offset.
      Elm* misalignedElms = (Elm*)(uintptr_t(block) + oldPad);
      Elm* elms = (Elm*)newData;
      memmove((void*)elms, (void*)misalignedElms, (m_lastE+1) * sizeof(Elm));
    }
  } else {
    Elm* oldElms = data2Elms(m_data);
    Elm* elms = (Elm*)newData;
    memcpy((void*)elms, (void*)oldElms, (m_lastE+1) * sizeof(Elm));
    m_linear = false;
  }
  m_dataPad = newPad;
  m_data = newData;
#endif
}

void HphpArray::delinearize() {
  size_t maxElms = computeMaxElms(m_tableMask);
  size_t tableSize = computeTableSize(m_tableMask);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data);
  ElmInd* oldHash = m_hash;
  m_hash = elms2Hash(elms, maxElms);
  memcpy((void*)m_hash, (void*)oldHash, tableSize * sizeof(ElmInd));
}

inline void HphpArray::resizeIfNeeded() {
  uint32 maxElms = computeMaxElms(m_tableMask);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE)+1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
  if (uint32(m_lastE)+1 == maxElms || m_hLoad == maxElms) {
    resize();
  }
}

void HphpArray::resize() {
  uint32 maxElms = computeMaxElms(m_tableMask);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE)+1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
  // At a minimum, compaction is required.  If the load factor would be >0.5
  // even after compaction, grow instead, in order to avoid the possibility
  // of repeated compaction if the load factor were to hover at nearly 0.75.
  bool doGrow = (m_nElms > (ElmInd)(maxElms >> 1));
#ifdef PEDANTIC
  if (m_tableMask > 0x7fffffffU && doGrow) {
    // If the hashtable is at its maximum size, we cannot grow
    doGrow = false;
    // Check if compaction would actually make room for at least one new
    // element. If not, raise an error.
    if (m_nElms >= 0x7fffffffU) {
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
  m_tableMask = (uint)(size_t(m_tableMask) + size_t(m_tableMask) + size_t(1));
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data); // m_hash is currently invalid.
  m_hash = elms2Hash(elms, maxElms);

  // All the elements have been copied and their offsets from the base are
  // still the same, so we just need to build the new hash table.
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_nElms until after rebuilding is complete, in
  // order to maintain invariants in findForNewInsert().
  m_hLoad = 0;
#else
  m_hLoad = m_nElms;
#endif

  if (m_nElms > 0) {
    Elm* elms = data2Elms(m_data);
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &elms[pos];
      if (e->data.m_type == KindOfTombstone) {
        continue;
      }
      ElmInd* ei = findForNewInsert(e->h);
      *ei = pos;
    }
#ifdef DEBUG
    m_hLoad = m_nElms;
#endif
  }
}

void HphpArray::compact(bool renumber /* = false */) {
  struct ElmKey {
    int64       h;
    StringData* key;
  };
  ElmKey mPos;
  if (m_pos != ArrayData::invalid_index) {
    // Cache key for element associated with m_pos in order to update m_pos
    // below.
    ASSERT(m_pos <= ssize_t(m_lastE));
    Elm* elms = data2Elms(m_data);
    mPos.h = elms[(ElmInd)m_pos].h;
    mPos.key = elms[(ElmInd)m_pos].key;
  } else {
    // Silence compiler warnings.
    mPos.h = 0;
    mPos.key = NULL;
  }
  int nsi = m_strongIterators.size();
  ElmKey* siKeys = NULL;
  if (nsi > 0) {
    Elm* elms = data2Elms(m_data);
    siKeys = (ElmKey*)malloc(nsi * sizeof(ElmKey));
    for (int i = 0; i < nsi; ++i) {
      ElmInd ei = (ElmInd)m_strongIterators.get(i)->pos;
      if (ei != ElmIndEmpty) {
        siKeys[i] = *(ElmKey*)&elms[(ElmInd)m_strongIterators.get(i)->pos];
      }
    }
  }
  if (renumber) {
    m_nextKI = 0;
  }
  Elm* elms = data2Elms(m_data);
  size_t tableSize = computeTableSize(m_tableMask);
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_nElms until after rebuilding is complete, in
  // order to maintain invariants in findForNewInsert().
  m_hLoad = 0;
#else
  m_hLoad = m_nElms;
#endif
  ElmInd frPos = 0;
  for (ElmInd toPos = 0; toPos < m_nElms; ++toPos) {
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
    if (renumber && toE->key == NULL) {
      toE->h = m_nextKI;
      ++m_nextKI;
    }
    ElmInd* ie = findForNewInsert(toE->h);
    *ie = toPos;
    ++frPos;
  }
  m_lastE = m_nElms - 1;
#ifdef DEBUG
  m_hLoad = m_nElms;
#endif
  if (m_pos != ArrayData::invalid_index) {
    // Update m_pos, now that compaction is complete.
    if (mPos.key != NULL) {
      m_pos = ssize_t(find(mPos.key->data(), mPos.key->size(), mPos.h));
    } else {
      m_pos = ssize_t(find(mPos.h));
    }
  }
  if (nsi > 0) {
    // Update strong iterators, now that compaction is complete.
    for (int i = 0; i < nsi; ++i) {
      ssize_t* siPos = &m_strongIterators.get(i)->pos;
      if (*siPos != ArrayData::invalid_index) {
        if (siKeys[i].key != NULL) {
          *siPos = ssize_t(find(siKeys[i].key->data(),
                                siKeys[i].key->size(),
                                siKeys[i].h));
        } else {
          *siPos = ssize_t(find(siKeys[i].h));
        }
      }
    }
    free(siKeys);
  }
}

#define ELEMENT_CONSTRUCT(fr, to) \
  if (fr->m_type == KindOfVariant) fr = fr->m_data.ptv; \
  TV_DUP_CELL_NC(fr, to); \
  to->_count = 0; \

bool HphpArray::nextInsert(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  if (m_linear) {
    delinearize();
  }
  resizeIfNeeded();
  int64 ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  ASSERT(!validElmInd(*ei));
  // Allocate a new element.
  Elm* e = allocElm(ei);
  ASSERT(e != NULL);
  // Set key.
  e->h = ki;
  e->key = NULL;
  // Set the value.
  tvAsVariant(&e->data).constructValHelper(data);
  // Update next free element.
  ++m_nextKI;
  return true;
}

bool HphpArray::nextInsertRef(CVarRef data) {
  if (UNLIKELY(m_nextKI < 0)) {
    raise_warning("Cannot add element to the array as the next element is "
                  "already occupied");
    return false;
  }
  if (m_linear) {
    delinearize();
  }
  resizeIfNeeded();
  int64 ki = m_nextKI;
  // The check above enforces an invariant that allows us to always
  // know that m_nextKI is not present in the array, so it is safe
  // to use findForNewInsert()
  ElmInd* ei = findForNewInsert(ki);
  ASSERT(!validElmInd(*ei));
  // Allocate a new element.
  Elm* e = allocElm(ei);
  ASSERT(e != NULL);
  // Set key.
  e->h = ki;
  e->key = NULL;
  // Set the value.
  tvAsVariant(&e->data).constructRefHelper(data);
  // Update next free element.
  ++m_nextKI;
  return true;
}

bool HphpArray::nextInsertWithRef(CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  resizeIfNeeded();
  int64 ki = m_nextKI;
  ElmInd* ei = findForInsert(ki);
  ASSERT(!validElmInd(*ei));

  // Allocate a new element.
  Elm* e = allocElm(ei);
  // Set key.
  e->h = ki;
  e->key = NULL;
  TV_WRITE_NULL(&e->data);
  tvAsVariant(&e->data).setWithRef(data);
  // Update next free element.
  ++m_nextKI;
  return true;
}

bool HphpArray::addLvalImpl(int64 ki, Variant** pDest,
                            bool doFind /* = true */) {
  ASSERT(pDest != NULL);
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    // Integer keys do not support KindOfIndirect
    ASSERT(e->data.m_type != KindOfIndirect);
    TypedValue* tv = &e->data;
    *pDest = (Variant*)tv;
    return false;
  }

  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(ki);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }

  e->h = ki;
  e->key = NULL;

  TV_WRITE_NULL(&e->data);
  *pDest = &(tvAsVariant(&e->data));

  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return true;
}

bool HphpArray::addLvalImpl(StringData* key, int64 h, Variant** pDest,
                            bool doFind /* = true */) {
  ASSERT(key != NULL && pDest != NULL);
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    TypedValue* tv;
    if (e->data.m_type != KindOfIndirect) {
      tv = &e->data;
    } else {
      tv = e->data.m_data.ptv;
    }
    *pDest = &(tvAsVariant(tv));
    return false;
  }

  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(h);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }
  // Set key.
  e->h = h;
  e->key = key;
  e->key->incRefCount();
  // Initialize element to null and store the address of the element into
  // *pDest.
  TV_WRITE_NULL(&e->data);
  *pDest = &(tvAsVariant(&e->data));

  return true;
}

inline bool HphpArray::addVal(int64 ki, CVarRef data,
                              bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(ki);
  if (checkExists && validElmInd(*ei)) {
    return false;
  } else {
    ASSERT(!validElmInd(*ei));
  }

  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(ki);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }
  TypedValue* fr = (TypedValue*)(&data);
  TypedValue* to = (TypedValue*)(&e->data);
  e->h = ki;
  e->key = NULL;
  ELEMENT_CONSTRUCT(fr, to);

  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }
  return true;
}

inline bool HphpArray::addVal(StringData* key, CVarRef data,
                              bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  int64 h = key->hash();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  Elm* elms = data2Elms(m_data);
  if (checkExists && validElmInd(*ei)) {
    if (LIKELY(elms[*ei].data.m_type != KindOfIndirect)) {
      return false;
    } else {
      TypedValue* tv = elms[*ei].data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type != KindOfUninit) return false;
    }
  }
  TypedValue* to;
  if (LIKELY(!validElmInd(*ei))) {
    Elm* e = allocElm(ei);
    if (UNLIKELY(e == NULL)) {
      resize();
      ei = findForNewInsert(h);
      e = allocElm(ei);
      ASSERT(e != NULL);
    }
    to = (TypedValue*)(&e->data);
    // Set the key
    e->h = h;
    e->key = key;
    e->key->incRefCount();
  } else {
    Elm* e = &elms[*ei];
    ASSERT(e->data.m_type == KindOfIndirect);
    to = (TypedValue*)(e->data.m_data.ptv);
    ASSERT(to->m_type == KindOfUninit);
  }
  // Set the element
  TypedValue* fr = (TypedValue*)(&data);
  ELEMENT_CONSTRUCT(fr, to);

  return true;
}

inline bool HphpArray::addValWithRef(int64 ki, CVarRef data,
                                     bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  resizeIfNeeded();
  ElmInd* ei = findForInsert(ki);
  if (checkExists && validElmInd(*ei)) {
    return false;
  } else {
    ASSERT(!validElmInd(*ei));
  }

  Elm* e = allocElm(ei);

  TV_WRITE_NULL(&e->data);

  e->h = ki;
  e->key = NULL;

  tvAsVariant(&e->data).setWithRef(data);

  if (ki >= m_nextKI) {
    m_nextKI = ki + 1;
  }
  return true;
}

inline bool HphpArray::addValWithRef(StringData* key, CVarRef data,
                                     bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  resizeIfNeeded();
  int64 h = key->hash();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (checkExists && validElmInd(*ei)) {
    return false;
  } else {
    ASSERT(!validElmInd(*ei));
  }

  Elm* e = allocElm(ei);

  e->h = h;
  e->key = key;
  e->key->incRefCount();

  TV_WRITE_NULL(&e->data);
  tvAsVariant(&e->data).setWithRef(data);

  return true;
}

bool HphpArray::update(int64 ki, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    // Integer keys do not support KindOfIndirect
    ASSERT(e->data.m_type != KindOfIndirect);
    tvAsVariant(&e->data).assignValHelper(data);
    return true;
  }
  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(ki);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }

  e->h = ki;
  e->key = NULL;
  tvAsVariant(&e->data).constructValHelper(data);

  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }

  return true;
}

bool HphpArray::update(StringData* key, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  int64 h = key->hash();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    Variant* to;
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      to = &tvAsVariant(&e->data);
    } else {
      to = &tvAsVariant(e->data.m_data.ptv);
    }
    to->assignValHelper(data);
    return true;
  }

  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(h);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }

  e->h = h;
  e->key = key;
  e->key->incRefCount();
  tvAsVariant(&e->data).constructValHelper(data);

  return true;
}

bool HphpArray::updateRef(int64 ki, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    // Integer keys do not support KindOfIndirect
    ASSERT(e->data.m_type != KindOfIndirect);
    tvAsVariant(&e->data).assignRefHelper(data);
    return true;
  }
  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(ki);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }

  e->h = ki;
  e->key = NULL;
  tvAsVariant(&e->data).constructRefHelper(data);

  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }

  return true;
}

bool HphpArray::updateRef(StringData* key, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  int64 h = key->hash();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    Variant* to;
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      to = (Variant*)(&e->data);
    } else {
      to = (Variant*)(e->data.m_data.ptv);
    }
    to->assignRefHelper(data);
    return true;
  }

  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(h);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }

  e->h = h;
  e->key = key;
  e->key->incRefCount();
  tvAsVariant(&e->data).constructRefHelper(data);

  return true;
}

// NOTE: The migrate() and migrateAndSet() methods do not trigger
// copy-on-write. It is the caller's responsibility to account
// for this.

TypedValue* HphpArray::migrate(StringData* k, TypedValue* tv) {
  if (m_linear) {
    delinearize();
  }
  int64 h = k->hash();
  ElmInd* ei = findForInsert(k->data(), k->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    TypedValue* slot = (TypedValue*)(&e->data);
    if (slot->m_type != KindOfIndirect) {
      if (tv == NULL) {
        // If the caller wants to unpin a key in the array that is not
        // pinned, do nothing and return NULL
        return NULL;
      }
      // Migrate the existing direct element to tv
      memcpy(tv, slot, sizeof(TypedValue));
      // Set the element to a KindOfIndirect that points to tv
      slot->m_data.ptv = tv;
      slot->m_type = KindOfIndirect;
      ++m_nIndirectElms;
      return NULL;
    } else {
      TypedValue* cur = slot->m_data.ptv;
      if (tv == NULL) {
        // Migrate the existing indirect element back to the array
        memcpy(slot, cur, sizeof(TypedValue));
        // Check for uninit null
        if (cur->m_type == KindOfUninit) {
          // Erase the element
          erase(ei);
        }
        --m_nIndirectElms;
        return cur;
      }
      // Migrate the existing indirect element to tv
      memcpy(tv, cur, sizeof(TypedValue));
      // Set the element to point to tv
      slot->m_data.ptv = tv;
      return cur;
    }
  } else {
    if (tv == NULL) {
      // If the caller wants to unpin a key that is not present in the
      // array, do nothing and return NULL
      return NULL;
    }
    // Allocate a new element
    Elm* e = allocElm(ei);
    if (UNLIKELY(e == NULL)) {
      resize();
      ei = findForNewInsert(h);
      e = allocElm(ei);
      ASSERT(e != NULL);
    }
    TypedValue* slot = (TypedValue*)(&e->data);
    // Set the key
    e->h = h;
    e->key = k;
    e->key->incRefCount();
    // Set the element to a KindOfIndirect that points to tv
    slot->m_data.ptv = tv;
    slot->_count = 0;
    slot->m_type = KindOfIndirect;
    // Set tv to uninit null
    TV_WRITE_UNINIT(tv);
    ++m_nIndirectElms;
    return NULL;
  }
}

TypedValue* HphpArray::migrateAndSet(StringData* k, TypedValue* tv) {
  ASSERT(tv != NULL);
  if (m_linear) {
    delinearize();
  }
  int64 h = k->hash();
  ElmInd* ei = findForInsert(k->data(), k->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    TypedValue* slot = (TypedValue*)(&e->data);
    if (slot->m_type != KindOfIndirect) {
      // Destroy the old element
      tvRefcountedDecRef(slot);
      // Set the element to a KindOfIndirect that points to tv
      slot->m_data.ptv = tv;
      slot->m_type = KindOfIndirect;
      ++m_nIndirectElms;
      return NULL;
    } else {
      TypedValue* cur = slot->m_data.ptv;
      // Destroy the old element
      tvUnset(cur);
      // Set the element to point to tv
      slot->m_data.ptv = tv;
      return cur;
    }
  } else {
    // Allocate a new element
    Elm* e = allocElm(ei);
    if (UNLIKELY(e == NULL)) {
      resize();
      ei = findForNewInsert(h);
      e = allocElm(ei);
      ASSERT(e != NULL);
    }
    TypedValue* slot = (TypedValue*)(&e->data);
    // Set the key
    e->h = h;
    e->key = k;
    e->key->incRefCount();
    // Set the element to a KindOfIndirect that points to tv
    slot->m_data.ptv = tv;
    slot->_count = 0;
    slot->m_type = KindOfIndirect;
    ++m_nIndirectElms;
    return NULL;
  }
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
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    // Integer keys do not support KindOfIndirect
    ASSERT(e->data.m_type != KindOfIndirect);
    if (tvAsVariant(&e->data).isReferenced() ||
        tvAsVariant(&e->data).isObject()) {
      ret = &(tvAsVariant(&e->data));
      return NULL;
    }
  }
  HphpArray* a = copyImpl();
  a->addLvalImpl(k, &ret, false);
  return a;
}

ArrayData* HphpArray::lval(litstr k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  String s(k, AttachLiteral);
  return lval(s, ret, copy, checkExist);
}

ArrayData* HphpArray::lval(CStrRef k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  StringData* key = k.get();
  int64 prehash = key->hash();
  if (!copy) {
    addLvalImpl(key, prehash, &ret);
    return NULL;
  }
  if (!checkExist) {
    HphpArray* a = copyImpl();
    a->addLvalImpl(key, prehash, &ret);
    return a;
  }
  ssize_t /*ElmInd*/ pos = find(key->data(), key->size(), prehash);
  if (pos != (ssize_t)ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    TypedValue* tv = &e->data;
    if (tv->m_type == KindOfIndirect) {
      tv = tv->m_data.ptv;
    }
    if (tvAsVariant(tv).isReferenced() ||
        tvAsVariant(tv).isObject()) {
      ret = &(tvAsVariant(tv));
      return NULL;
    }
  }
  HphpArray* a = copyImpl();
  a->addLvalImpl(key, prehash, &ret, false);
  return a;
}

ArrayData* HphpArray::lval(CVarRef k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  if (isIntegerKey(k)) {
    return lval(k.toInt64(), ret, copy, checkExist);
  }
  return lval(k.toString(), ret, copy, checkExist);
}

ArrayData *HphpArray::lvalPtr(CStrRef k, Variant*& ret, bool copy,
                              bool create) {
  StringData* key = k.get();
  int64 prehash = key->hash();
  HphpArray* a = 0;
  HphpArray* t = this;
  if (copy) {
    a = t = copyImpl();
  }
  if (create) {
    t->addLvalImpl(key, prehash, &ret);
  } else {
    ssize_t /*ElmInd*/ pos = t->find(key->data(), key->size(), prehash);
    if (pos != (ssize_t)ElmIndEmpty) {
      Elm* elms = data2Elms(m_data);
      Elm* e = &elms[pos];
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        ret = &(tvAsVariant(&e->data));
      } else {
        TypedValue* tv = e->data.m_data.ptv;
        // Check for uninit null
        if (tv->m_type == KindOfUninit) {
          ret = NULL;
        } else {
          ret = &(tvAsVariant(e->data.m_data.ptv));
        }
      }
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
      Elm* elms = data2Elms(m_data);
      Elm* e = &elms[pos];
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        ret = &(tvAsVariant(&e->data));
      } else {
        TypedValue* tv = e->data.m_data.ptv;
        // Check for uninit null
        if (tv->m_type == KindOfUninit) {
          ret = NULL;
        } else {
          ret = &(tvAsVariant(e->data.m_data.ptv));
        }
      }
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
  if (copy) {
    HphpArray* a = copyImpl();
    a->update(k, v);
    return a;
  }
  update(k, v);
  return NULL;
}

ArrayData* HphpArray::set(CStrRef k, CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->update(k.get(), v);
    return a;
  }
  update(k.get(), v);
  return NULL;
}

ArrayData* HphpArray::set(CVarRef k, CVarRef v, bool copy) {
  if (isIntegerKey(k)) {
    if (copy) {
      HphpArray* a = copyImpl();
      a->update(k.toInt64(), v);
      return a;
    }
    update(k.toInt64(), v);
    return NULL;
  }
  StringData* sd = k.getStringData();
  if (copy) {
    HphpArray* a = copyImpl();
    a->update(sd, v);
    return a;
  }
  update(sd, v);
  return NULL;
}

ArrayData* HphpArray::setRef(int64 k, CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->updateRef(k, v);
    return a;
  }
  updateRef(k, v);
  return NULL;
}

ArrayData* HphpArray::setRef(CStrRef k, CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->updateRef(k.get(), v);
    return a;
  }
  updateRef(k.get(), v);
  return NULL;
}

ArrayData* HphpArray::setRef(CVarRef k, CVarRef v, bool copy) {
  if (isIntegerKey(k)) {
    if (copy) {
      HphpArray* a = copyImpl();
      a->updateRef(k.toInt64(), v);
      return a;
    }
    updateRef(k.toInt64(), v);
    return NULL;
  }
  StringData* sd = k.getStringData();
  if (copy) {
    HphpArray* a = copyImpl();
    a->updateRef(sd, v);
    return a;
  }
  updateRef(sd, v);
  return NULL;
}

ArrayData* HphpArray::add(int64 k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (copy) {
    HphpArray* result = copyImpl();
    result->add(k, v, false);
    return result;
  }
  addVal(k, v, false);
  return NULL;
}

ArrayData* HphpArray::add(CStrRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (copy) {
    HphpArray* result = copyImpl();
    result->add(k, v, false);
    return result;
  }
  addVal(k.get(), v, false);
  return NULL;
}

ArrayData* HphpArray::add(CVarRef k, CVarRef v, bool copy) {
  ASSERT(!exists(k));
  if (isIntegerKey(k)) {
    return add(k.toInt64(), v, copy);
  }
  return add(k.toString(), v, copy);
}

ArrayData* HphpArray::addLval(int64 k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  if (copy) {
    HphpArray* result = copyImpl();
    result->addLvalImpl(k, &ret, false);
    return result;
  }
  addLvalImpl(k, &ret, false);
  return NULL;
}

ArrayData* HphpArray::addLval(CStrRef k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  if (copy) {
    HphpArray* result = copyImpl();
    result->addLvalImpl(k.get(), k->hash(), &ret, false);
    return result;
  }
  addLvalImpl(k.get(), k->hash(), &ret, false);
  return NULL;
}

ArrayData* HphpArray::addLval(CVarRef k, Variant*& ret, bool copy) {
  ASSERT(!exists(k));
  if (copy) {
    HphpArray* a = copyImpl();
    if (isIntegerKey(k)) {
      a->addLvalImpl(k.toInt64(), &ret, false);
    } else {
      StringData* sd = k.getStringData();
      a->addLvalImpl(sd, sd->hash(), &ret, false);
    }
    return a;
  }
  if (isIntegerKey(k)) {
    addLvalImpl(k.toInt64(), &ret, false);
  } else {
    StringData* sd = k.getStringData();
    addLvalImpl(sd, sd->hash(), &ret, false);
  }
  return NULL;
}

//=============================================================================
// Delete.

void HphpArray::erase(ElmInd* ei, bool updateNext /* = false */) {
  ASSERT(!m_linear);
  ElmInd pos = *ei;
  if (!validElmInd(pos)) {
    return;
  }

  Elm* elms = data2Elms(m_data);

  bool nextElementUnsetInsideForeachByReference = false;
  int nsi = m_strongIterators.size();
  ElmInd eINext = ElmIndTombstone;
  for (int i = 0; i < nsi; ++i) {
    if (m_strongIterators.get(i)->pos == ssize_t(pos)) {
      nextElementUnsetInsideForeachByReference = true;
      if (eINext == ElmIndTombstone) {
        // eINext will actually be used, so properly initialize it with the
        // next element past pos, or ElmIndEmpty if pos is the last element.
        eINext = nextElm(elms, pos);
        if (eINext == ElmIndEmpty) {
          // Record that there is a strong iterator out there that is past the
          // end.
          m_siPastEnd = true;
        }
      }
      m_strongIterators.get(i)->pos = ssize_t(eINext);
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
  if (e->data.m_type != KindOfIndirect) {
    // Free the value if necessary and mark it as a tombstone.
    TypedValue* tv = &e->data;
    tvRefcountedDecRef(tv);
    tv->m_type = KindOfTombstone;
    // Free the key if necessary, and clear the h and key fields in order to
    // increase the chances that subsequent searches will quickly/safely fail
    // when encountering tombstones, even though checking for KindOfTombstone is
    // the last validation step during search.
    if (e->key != NULL) {
      if (e->key->decRefCount() == 0) {
        e->key->release();
      }
      e->key = NULL;
    } else {
      // Match PHP 5.3.1 semantics
      if (e->h == m_nextKI-1 && (e->h == 0x7fffffffffffffffLL || updateNext)) {
        --m_nextKI;
      }
    }
    e->h = 0;
    --m_nElms;
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
    if (m_nElms < ((m_lastE+1) >> 1)) {
      // Compact in order to keep elms from being overly sparse.
      compact();
    }
  } else {
    // For KindOfIndirect, we do not free the key or remove the hash entry.
    // Instead, we decref the value that the KindOfIndirect points to and
    // set it to uninit null. m_nElms does not get updated either. It is the
    // responsibility of the size() function to account for this.
    //
    // The main takeaway here is that we leave the hash entry, the key, and
    // the KindOfIndirect pointer intact. That way, if the element is set
    // later, we are able to preserve the relationship between the key and
    // the indirect memory location.
    TypedValue* tv = e->data.m_data.ptv;
    tvRefcountedDecRef(tv);
    tv->m_type = KindOfUninit;
  }

  if (nextElementUnsetInsideForeachByReference) {
    if (RuntimeOption::EnableHipHopErrors) {
      raise_warning("The next element was unset inside foreach by reference. "
                    "This may lead to unexpeced results.");
    }
  }
}

ArrayData* HphpArray::remove(int64 k, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->erase(a->findForInsert(k));
    return a;
  }
  if (m_linear) {
    delinearize();
  }
  erase(findForInsert(k));
  return NULL;
}

ArrayData* HphpArray::remove(CStrRef k, bool copy) {
  int64 prehash = k->hash();
  if (copy) {
    HphpArray* a = copyImpl();
    a->erase(a->findForInsert(k.data(), k.size(), prehash));
    return a;
  }
  if (m_linear) {
    delinearize();
  }
  erase(findForInsert(k.data(), k.size(), prehash));
  return NULL;
}

ArrayData* HphpArray::remove(CVarRef k, bool copy) {
  if (isIntegerKey(k)) {
    if (copy) {
      HphpArray* a = copyImpl();
      a->erase(a->findForInsert(k.toInt64()));
      return a;
    }
    if (m_linear) {
      delinearize();
    }
    erase(findForInsert(k.toInt64()));
    return NULL;
  } else {
    StringData* key = k.getStringData();
    int64 prehash = key->hash();
    if (copy) {
      HphpArray* a = copyImpl();
      a->erase(a->findForInsert(key->data(), key->size(), prehash));
      return a;
    }
    if (m_linear) {
      delinearize();
    }
    erase(findForInsert(key->data(), key->size(), prehash));
    return NULL;
  }
}

ArrayData* HphpArray::copy() const {
  return copyImpl();
}

//=============================================================================
// non-variant interface

TypedValue* HphpArray::nvGetCell(int64 ki, bool error /* = false */) const {
  ElmInd pos = find(ki);
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    TypedValue* tv = &e->data;
    // Integer keys do not support KindOfIndirect
    ASSERT(tv->m_type != KindOfIndirect);
    if (tv->m_type != KindOfVariant) {
      return tv;
    } else {
      return tv->m_data.ptv;
    }
  }
  if (error) {
    raise_notice("Undefined index: %lld", ki);
  }
  return NULL;
}

TypedValue*
HphpArray::nvGetCell(const StringData* k, bool error /* = false */) const {
  ElmInd pos = find(k->data(), k->size(), k->hash());
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    TypedValue* tv = &e->data;
    if (tv->m_type < KindOfVariant) {
      return tv;
    }
    if (LIKELY(tv->m_type == KindOfVariant)) {
      return tv->m_data.ptv;
    }
    ASSERT(tv->m_type == KindOfIndirect);
    tv = tv->m_data.ptv;
    // Check for uninit null
    if (tv->m_type == KindOfUninit) {
      goto undefined_index;
    }
    if (tv->m_type != KindOfVariant) {
      return tv;
    } else {
      return tv->m_data.ptv;
    }
  }
undefined_index:
  if (error) {
    raise_notice("Undefined index: %s", k->data());
  }
  return NULL;
}

TypedValue*
HphpArray::nvGetCell(int64 ki1, const StringData* ks2,
                     bool error /* = false */) const {
  TypedValue* first = nvGetCell(ki1, error);
  if (UNLIKELY(first == NULL)) {
    // nvGet raise_notice'd if necessary
    return NULL;
  }
  ASSERT(first->m_type == KindOfArray); // XXX: not implemented

  // if first :: KindOfArray, things look like this:
  //   first:(Array, parr -> HphpArray)
  // if first :: KindOfVariant, instead it had better be
  //   first:(Variant,
  //          ptv-> (KindOfArray, parr -> HphpArray))
  TypedValue *ad = first->m_data.ptv;
  TRACE(2, "nvGetCell[%d][%s]: ad %p, type %d\n",
        int(ki1), ks2->data(), ad, first->m_type);
  HphpArray *innerArr = (HphpArray*)ad;
  TypedValue *retval  = innerArr->nvGetCell(ks2, error);
  if (retval) {
    TRACE(2, "nvGetCell: retval %p of type %d, value %llx / %g\n",
          retval, retval->m_type, retval->m_data.num,
          retval->m_data.dbl);
  } else {
    TRACE(2, "nvGetCell: returning NULL\n");
  }
  return retval;
}

TypedValue* HphpArray::nvGet(int64 ki, bool error /* = false */) const {
  ElmInd pos = find(ki);
  if (LIKELY(pos != ElmIndEmpty)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return &e->data;
  }
  if (error) {
    raise_notice("Undefined index: %lld", ki);
  }
  return NULL;
}

inline TypedValue*
HphpArray::nvGetImpl(const StringData* k, int* outHint,
                     bool error/*=false*/) const {
  ElmInd pos = find(k->data(), k->size(), k->hash());
  if (LIKELY(pos != ElmIndEmpty)) {
    if (outHint) { *outHint = int(pos); }
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      return &e->data;
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // Check for uninit null
      if (tv->m_type == KindOfUninit) {
        goto undefined_index;
      }
      return tv;
    }
  }
undefined_index:
  if (error) {
    raise_notice("Undefined index: %s", k->data());
  }
  return NULL;
}

TypedValue*
HphpArray::nvGet(const StringData* k, bool error /* = false */) const {
  return nvGetImpl(k, NULL, error);
}

TypedValue*
HphpArray::nvGet(const StringData* k, int posHint, int *posHintOut,
                 bool error/*=false*/) const {
  TRACE(2, "nvGet: hint %d, k %s\n", posHint, k->data());
  if (LIKELY(posHint < m_nElms)) {
    Elm* e = data2Elms(m_data) + posHint;
    TRACE(2, "range check passed\n");
    if (LIKELY(e->data.m_type != KindOfTombstone &&
               hitStringKey(e, k->data(), k->size(), k->hash()))) {
      TRACE(2, "inner hit\n");
      if (e->data.m_type == KindOfIndirect) {
        return e->data.m_data.ptv;
      }
      return &e->data;
    } else {
      TRACE(2, "miss: tombstone %d, KoI %d, key %s\n",
            e->data.m_type == KindOfTombstone,
            e->data.m_type == KindOfIndirect,
            e->key->data());
    }
  }
  TypedValue* ret = nvGetImpl(k, posHintOut, error);
  TRACE(2, "inner miss; old hint %d, new %d\n",
        posHint, *posHintOut);
  return ret;
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

ArrayData* HphpArray::nvSet(int64 ki, TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->update(ki, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvSet(StringData* k, TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->update(k, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvBind(int64 ki, TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->updateRef(ki, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvBind(StringData* k, TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->updateRef(k, tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvAppend(TypedValue* v, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->nextInsert(tvAsCVarRef(v));
  return retval;
}

ArrayData* HphpArray::nvNew(TypedValue*& ret, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    if (UNLIKELY(!a->nextInsert(null))) {
      ret = NULL;
      return a;
    }
    ASSERT(a->m_lastE != ElmIndEmpty);
    ssize_t lastE = (ssize_t)a->m_lastE;
    Elm* aElms = data2Elms(a->m_data);
    ret = &aElms[lastE].data;
    return a;
  }
  if (UNLIKELY(!nextInsert(null))) {
    ret = NULL;
    return NULL;
  }
  ASSERT(m_lastE != ElmIndEmpty);
  ssize_t lastE = (ssize_t)m_lastE;
  Elm* elms = data2Elms(m_data);
  ret = &elms[lastE].data;
  return NULL;
}

ArrayData* HphpArray::nvRemove(int64 ki, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->erase(findForInsert(ki));
  return retval;
}

ArrayData* HphpArray::nvRemove(const StringData* k, bool copy) {
  HphpArray* a = this;
  ArrayData* retval = NULL;
  if (copy) {
    retval = a = copyImpl();
  }
  a->erase(findForInsert(k->data(), k->size(), k->hash()));
  return retval;
}

ssize_t HphpArray::nvSize() const {
  return m_nElms;
}

TypedValue* HphpArray::nvGetValueRef(ssize_t pos) {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (LIKELY(e->data.m_type != KindOfIndirect)) {
    return &e->data;
  } else {
    return e->data.m_data.ptv;
  }
}

// nvGetKey sets out->_count to 0, so out cannot be an inner cell
void HphpArray::nvGetKey(TypedValue* out, ssize_t pos) {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[/*(ElmInd)*/pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  out->_count = 0;
  if (e->key != NULL) {
    out->m_data.pstr = e->key;
    out->m_type = KindOfString;
    e->key->incRefCount();
    return;
  }
  out->m_data.num = e->h;
  out->m_type = KindOfInt64;
}

bool HphpArray::nvUpdate(int64 ki, int64 vi) {
  if (m_linear) {
    delinearize();
  }
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    TypedValue* to = (TypedValue*)(&e->data);
    if (to->m_type == KindOfVariant) to = to->m_data.ptv;
    DataType oldType = to->m_type;
    uint64_t oldDatum = to->m_data.num;
    if (IS_REFCOUNTED_TYPE(oldType)) {
      tvDecRefHelper(oldType, oldDatum);
    }
    to->m_data.num = vi;
    to->m_type = KindOfInt64;
    return true;
  }
  Elm* e = allocElm(ei);
  if (UNLIKELY(e == NULL)) {
    resize();
    ei = findForNewInsert(ki);
    e = allocElm(ei);
    ASSERT(e != NULL);
  }
  TypedValue* to = (TypedValue*)(&e->data);
  e->h = ki;
  e->key = NULL;
  to->m_data.num = vi;
  to->_count = 0;
  to->m_type = KindOfInt64;

  if (ki >= m_nextKI && m_nextKI >= 0) {
    m_nextKI = ki + 1;
  }

  return true;
}

ArrayData* HphpArray::nonSmartCopy() const {
  return copyImpl(NULL, false);
}

HphpArray* HphpArray::copyImpl(HphpArray* target /* = NULL */,
                               bool sma /* = true */) const {
  if (UNLIKELY(m_suppressCow && sma)) {
    return const_cast<HphpArray*>(this);
  }
  if (LIKELY(target == NULL)) {
    if (sma) {
      target = NEW(HphpArray)(0,0,0);
    } else {
      target = new HphpArray(0,0,0);
    }
  }
  target->m_pos = m_pos;
  target->m_data = NULL;
  target->m_nextKI = m_nextKI;
  target->m_tableMask = m_tableMask;
  target->m_nElms = m_nElms;
  target->m_hLoad = m_hLoad;
  target->m_lastE = m_lastE;
  target->m_linear = false;
  target->m_siPastEnd = false;
  target->m_suppressCow = false;
#ifndef USE_JEMALLOC
  target->m_dataPad = 0;
#endif
  target->m_nIndirectElms = 0;
  size_t tableSize = computeTableSize(m_tableMask);
  size_t maxElms = computeMaxElms(m_tableMask);
  target->reallocData(maxElms, tableSize);
  Elm* targetElms = data2Elms(target->m_data);
  target->m_hash = elms2Hash(targetElms, maxElms);
  // Copy the hash.
  memcpy(target->m_hash, m_hash, tableSize * sizeof(ElmInd));
  // Copy the elements and bump up refcounts as needed.
  if (m_nElms > 0) {
    Elm* elms = data2Elms(m_data);
    ssize_t lastE = (ssize_t)m_lastE;
    for (ssize_t /*ElmInd*/ pos = 0; pos <= lastE; ++pos) {
      Elm* e = &elms[pos];
      Elm* te = &targetElms[pos];
      if (e->data.m_type != KindOfTombstone) {
        te->h = e->h;
        te->key = e->key;
        TypedValue* fr;
        if (te->key != NULL) {
          if (LIKELY(e->data.m_type != KindOfIndirect)) {
            fr = &e->data;
          } else {
            fr = e->data.m_data.ptv;
            // Check for uninit null
            if (fr->m_type == KindOfUninit) {
              // If the indirect memory location is set to uninit
              // null, we treat it as a tombstone.
              // Before setting the entry as Tombstone, invalidate its
              // hash index.
              ElmInd* ei = findForInsert(e->key->data(),
                                            e->key->size(), e->h);
              ASSERT(*ei == pos);
              ElmInd* tei = (ElmInd*) ((uintptr_t)target->m_hash
                                       + (uintptr_t)ei - (uintptr_t)m_hash);
              ASSERT(*tei == pos);
              *tei = ElmIndTombstone;
              te->h = 0;
              te->key = NULL;
              te->data.m_type = KindOfTombstone;
              // Because we are converting an indirect element into
              // a tombstone, we need to decrement m_nElms to stay
              // consistent. This is because m_nElms always "counts"
              // an indirect element even if it is uninit null.
              --(target->m_nElms);
              continue;
            }
          }
          te->key->incRefCount();
        } else {
          // Integer keys do not support KindOfIndirect
          ASSERT(e->data.m_type != KindOfIndirect);
          fr = &e->data;
        }
        TypedValue* to = &te->data;
        TV_DUP_FLATTEN_VARS(fr, to, this);
      } else {
        // Tombstone.
        te->h = 0;
        te->key = NULL;
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
    if (target->m_nElms < ((target->m_lastE+1) >> 1)) {
      target->compact();
    }
  }
  return target;
}

ArrayData* HphpArray::append(CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->nextInsert(v);
    return a;
  }
  nextInsert(v);
  return NULL;
}

ArrayData* HphpArray::appendRef(CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->nextInsertRef(v);
    return a;
  }
  nextInsertRef(v);
  return NULL;
}

ArrayData *HphpArray::appendWithRef(CVarRef v, bool copy) {
  if (copy) {
    HphpArray *a = copyImpl();
    a->nextInsertWithRef(v);
    return a;
  }
  nextInsertWithRef(v);
  return NULL;
}

ArrayData* HphpArray::append(const ArrayData* elems, ArrayOp op, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->append(elems, op, false);
    return a;
  }

  if (op == Plus) {
    for (ArrayIter it(elems); !it.end(); it.next()) {
      Variant key = it.first();
      CVarRef value = it.secondRef();
      if (key.isNumeric()) {
        addValWithRef(key.toInt64(), value);
      } else {
        addValWithRef(key.getStringData(), value);
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
      if (key.isNumeric()) {
        nextInsertWithRef(value);
      } else {
        Variant *p;
        StringData *sd = key.getStringData();
        addLvalImpl(sd, sd->hash(), &p);
        p->setWithRef(value);
      }
    }
  }
  return NULL;
}

ArrayData* HphpArray::pop(Variant& value) {
  if (getCount() > 1 && !m_suppressCow) {
    HphpArray* a = copyImpl();
    a->pop(value);
    return a;
  }
  Elm* elms = data2Elms(m_data);
  if (m_linear) {
    delinearize();
  }
  ElmInd pos = iter_end();
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      value = tvAsCVarRef(&e->data);
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // iter_end() must guarantee that it will not return an element
      // which is KindOfIndirect and points to an uninit null value
      ASSERT(tv->m_type != KindOfUninit);
      value = tvAsCVarRef(tv);
    }
    ElmInd* ei = (e->key != NULL)
        ? findForInsert(e->key->data(), e->key->size(), e->h)
        : findForInsert(e->h);
    erase(ei, true);
  } else {
    value = null;
  }
  // To match PHP-like semantics, the pop operation resets the array's
  // internal iterator.
  m_pos = nextElm(elms, ElmIndEmpty);
  return NULL;
}

ArrayData* HphpArray::dequeue(Variant& value) {
  if (getCount() > 1 && !m_suppressCow) {
    HphpArray* a = copyImpl();
    a->dequeue(value);
    return a;
  }
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is removed from the beginning of the array.
  if (!m_strongIterators.empty()) {
    freeStrongIterators();
  }
  Elm* elms = data2Elms(m_data);
  if (m_linear) {
    delinearize();
  }
  ElmInd pos = nextElm(elms, ElmIndEmpty);
  if (validElmInd(pos)) {
    Elm* e = &elms[pos];
    if (LIKELY(e->data.m_type != KindOfIndirect)) {
      value = tvAsCVarRef(&e->data);
    } else {
      TypedValue* tv = e->data.m_data.ptv;
      // nextElm() must guarantee that it will not return an element
      // which is KindOfIndirect and points to an uninit null value
      ASSERT(tv->m_type != KindOfUninit);
      value = tvAsCVarRef(tv);
    }
    erase((e->key != NULL)
          ? findForInsert(e->key->data(), e->key->size(), e->h)
          : findForInsert(e->h)
          );
    compact(true);
  } else {
    value = null;
  }
  // To match PHP-like semantics, the dequeue operation resets the array's
  // internal iterator
  m_pos = ssize_t(nextElm(elms, ElmIndEmpty));
  return NULL;
}

ArrayData* HphpArray::prepend(CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->prepend(v, false);
    return a;
  }
  // To match PHP-like semantics, we invalidate all strong iterators when an
  // element is added to the beginning of the array.
  if (!m_strongIterators.empty()) {
    freeStrongIterators();
  }
  if (m_linear) {
    delinearize();
  }

  Elm* elms = data2Elms(m_data);
  if (elms[0].data.m_type != KindOfTombstone) {
    // Make sure there is room to insert an element.
    resizeIfNeeded();
    // Recompute elms, in case resizeIfNeeded() had side effects.
    elms = data2Elms(m_data);
    // Move the existing elements to make element 0 available.
    memmove(&elms[1], &elms[0], (m_lastE+1) * sizeof(Elm));
    ++m_lastE;
  }
  // Prepend.
  Elm* e = &elms[0];
  e->key = NULL;

  TypedValue* fr = (TypedValue*)(&v);
  TypedValue* to = (TypedValue*)(&e->data);
  ELEMENT_CONSTRUCT(fr, to);

  ++m_nElms;
  // Renumber.
  compact(true);
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator
  m_pos = ssize_t(nextElm(elms, ElmIndEmpty));

  return NULL;
}

void HphpArray::renumber() {
  compact(true);
}

void HphpArray::onSetStatic() {
  Elm* elms = data2Elms(m_data);
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type != KindOfTombstone) {
      if (e->key) {
        e->key->setStatic();
      }
      if (LIKELY(e->data.m_type != KindOfIndirect)) {
        tvAsVariant(&e->data).setStatic();
      } else {
        TypedValue* tv = e->data.m_data.ptv;
        // Check for uninit null
        if (tv->m_type != KindOfUninit) tvAsVariant(tv).setStatic();
      }
    }
  }
}

void HphpArray::onSetEvalScalar() {
  Elm* elms = data2Elms(m_data);
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type != KindOfTombstone) {
      StringData *key = e->key;
      if (key && !key->isStatic()) {
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
    // Record that there is a strong iterator out there that is past the end.
    m_siPastEnd = true;
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
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)m_pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (LIKELY(e->data.m_type != KindOfIndirect)) {
    return tvAsCVarRef(&e->data);
  } else {
    return tvAsCVarRef(e->data.m_data.ptv);
  }
}

CVarRef HphpArray::endRef() {
  ASSERT(m_lastE != ElmIndEmpty);
  Elm* elms = data2Elms(m_data);
  ElmInd pos = m_lastE;
  Elm* e = &elms[pos];
  if (LIKELY(e->data.m_type != KindOfIndirect)) {
    return tvAsCVarRef(&e->data);
  } else {
    return tvAsCVarRef(e->data.m_data.ptv);
  }
}

//=============================================================================
// Memory allocator methods.

bool HphpArray::calculate(int& size) {
  size += computeMaxElms(m_tableMask) * sizeof(Elm); // Array elements.
  size += computeTableSize(m_tableMask) * sizeof(ElmInd); // Hash table.
  size += ElmAlignment; // Padding to allow for alignment in restore().
  return true;
}

void HphpArray::backup(LinearAllocator& allocator) {
  // Pad to an alignment boundary, if necessary.
  const char* frontier = allocator.frontier();
  size_t padRem;
  if (uintptr_t(frontier) & ElmAlignmentMask) {
    char pad[ElmAlignment - (uintptr_t(frontier) & ElmAlignmentMask)];
    memset(pad, 0, sizeof(pad));
    allocator.backup(pad, sizeof(pad));
    padRem = ElmAlignment - sizeof(pad);
  } else {
    padRem = ElmAlignment;
  }

  Elm* elms = data2Elms(m_data);
  if (m_nIndirectElms == 0) {
    allocator.backup((const char*)elms,
                     computeMaxElms(m_tableMask) * sizeof(Elm));
  } else {
    // If this array contains indirect elements, we have to backup
    // the elements one at a time
    Elm ts;
    ts.h = 0LL;
    ts.key = NULL;
    ts.data.m_data.num = 0LL;
    ts.data._count = 0;
    ts.data.m_type = KindOfTombstone;

    for (ElmInd i = 0; i <= m_lastE; ++i) {
      Elm* e = &elms[i];
      if (e->data.m_type != KindOfIndirect) {
        allocator.backup((const char*)e, sizeof(Elm));
      } else {
        TypedValue* tv = e->data.m_data.ptv;
        // Check for uninit null
        if (tv->m_type == KindOfUninit) {
          // Tombstone
          allocator.backup((const char*)(&ts), sizeof(Elm));
        } else {
          Elm indElm;
          indElm.h = e->h;
          indElm.key = e->key;
          indElm.data.m_data.num = tv->m_data.num;
          indElm.data._count = tv->_count;
          indElm.data.m_type = tv->m_type;
          allocator.backup((const char*)(&indElm), sizeof(Elm));
        }
      }
    }
    // Back up the rest of the element slots after m_lastE
    allocator.backup((const char*)elms + ((m_lastE+1) * sizeof(Elm)),
                     (computeMaxElms(m_tableMask) - (m_lastE+1)) * sizeof(Elm));
  }
  allocator.backup((const char*)m_hash,
                   computeTableSize(m_tableMask) * sizeof(ElmInd));
  ASSERT(m_strongIterators.empty());
  // Trailing pad space is [1..ElmAlignment] bytes.
  char pad[padRem];
  memset(pad, 0, sizeof(pad));
  allocator.backup(pad, sizeof(pad));
}

void HphpArray::restore(const char*& buffer) {
  size_t maxElms = computeMaxElms(m_tableMask);
  size_t tableSize = computeTableSize(m_tableMask);

  // m_data is at an alignment boundary.
  if (uintptr_t(buffer) & ElmAlignmentMask) {
    size_t pad = ElmAlignment - (uintptr_t(buffer) & ElmAlignmentMask);
    m_data = (void*)(buffer + pad);
  } else {
    m_data = (void*)buffer;
  }

#ifndef USE_JEMALLOC
  m_dataPad = 0;
#endif
  Elm* elms = data2Elms(m_data);
  m_hash = elms2Hash(elms, maxElms);

  buffer += maxElms * sizeof(Elm);
  buffer += tableSize * sizeof(ElmInd);
  buffer += ElmAlignment;
  m_linear = true;
  m_strongIterators.m_data = NULL;
}

void HphpArray::sweep() {
  if (m_data != NULL) {
    if (!m_linear) {
      free(getBlock());
    }
    m_data = NULL;
#ifndef USE_JEMALLOC
    m_dataPad = 0;
#endif
  }
  m_strongIterators.clear();
}

//=============================================================================
// VM runtime support functions.
namespace VM {

// Helpers for array_setm.
template<typename Value>
inline ArrayData* nv_set_with_integer_check(HphpArray* arr, StringData* key,
                                            Value value, bool copy) {
  int64 lval;
  if (UNLIKELY(key->isStrictlyInteger(lval))) {
    return arr->nvSet(lval, value, copy);
  } else {
    return arr->nvSet(key, value, copy);
  }
}

template<typename Value>
ArrayData*
nvCheckedSet(HphpArray* ha, StringData* sd, Value value, bool copy) {
  return nv_set_with_integer_check<Value>(ha, sd, value, copy);
}

template<typename Value>
ArrayData*
nvCheckedSet(HphpArray* ha, int64 key, Value value, bool copy) {
  return ha->nvSet(key, value, copy);
}

void setmDecRef(int64 i) { /* nop */ }
void setmDecRef(TypedValue* tv) { tvRefcountedDecRef(tv); }
void setmDecRef(StringData* sd) { if (sd->decRefCount() == 0) sd->release(); }

static inline HphpArray*
array_mutate_pre(ArrayData* ad) {
  ASSERT(ad);
  return (HphpArray*)ad;
}

Variant toVar(int64 i)        { return Variant(i); }
Variant toVar(TypedValue* tv) { return tvCellAsVariant(tv); }

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
  if (LIKELY(IsHphpArray(ad))) {
    HphpArray* ha = array_mutate_pre(ad);
    // nvSet will decRef any old value that may have been overwritten
    // if appropriate
    retval = CheckInt ?
      nvCheckedSet(ha, key, value, copy)
      : ha->nvSet(key, value, copy);
  } else {
    String s(key);
    Variant vVal = toVar(value);
    retval = ad->set(s, vVal, copy);
  }
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
 *    array_setm_s0k1_v0 --
 *    array_setm_s0k1_v0 --
 *       Skip checks for $key that is really an integer.
 */
ArrayData* array_setm_sk1_v(TypedValue* cell, ArrayData* ad, StringData* key,
                            TypedValue* value) {
  return array_setm<StringData*, TypedValue*, false, true, false>
    (cell, ad, key, value);
}

ArrayData* array_setm_sk1_v0(TypedValue* cell, ArrayData* ad, StringData* key,
                             TypedValue* value) {
  return array_setm<StringData*, TypedValue*, true, true, false>(cell, ad, key, value);
}

ArrayData* array_setm_s0k1_v(TypedValue* cell, ArrayData* ad, StringData* key,
                             TypedValue* value) {
  return array_setm<StringData*, TypedValue*, false, true, false>
    (cell, ad, key, value);
}

ArrayData* array_setm_s0k1_v0(TypedValue* cell, ArrayData* ad, StringData* key,
                              TypedValue* value) {
  return array_setm<StringData*, TypedValue*, true, true, true>
    (cell, ad, key, value);
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
inline TypedValue* nv_get_cell_with_integer_check(HphpArray* arr,
                                                  StringData* key) {
  int64 lval;
  if (UNLIKELY(key->isStrictlyInteger(lval))) {
    return arr->nvGetCell(lval, true /* error */);
  } else {
    return arr->nvGetCell(key, true /* error */);
  }
}

template<typename Key, DataType keyType>
static void elem(ArrayData* ad, Key k, TypedValue* dest) {
  // The manual reference counting here is subtly necessary; e.g.,
  // for a CGetM <R E>, "dest" is the same stack slot as a
  // reference to ad, so tvAsVariant(dest) = ... will do the
  // wrong thing.
  CVarRef value = ad->get(k, true /* error */);
  TypedValue *tvPtr = (TypedValue*)&value;
  dest->m_type = tvPtr->m_type;
  dest->m_data.num = tvPtr->m_data.num;
  dest->_count = 0; // dest is a root
  tvRefcountedIncRef(dest);
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
  if (UNLIKELY(!IsHphpArray(ad))) {
    elem<int64, KindOfInt64>(ad, key, out);
  } else {
    HphpArray *ha = (HphpArray*)dptr;
    TRACE(2, "array_getm_ik1: (%p) <- %p[%lld]\n", out, dptr, key);
    // Ref-counting the value is the translator's responsibility. We know out
    // pointed to uninitialized memory, so no need to dec it.
    TypedValue* ret = ha->nvGetCell(key, true /* error */);
    if (UNLIKELY(!ret)) {
      TV_WRITE_NULL(out);
    } else {
      tvDup(ret, out);
    }
  }
  return ad;
}

#define ARRAY_GETM_BODY(dptr, sd, out, body, drKey) do {  \
  ArrayData* ad = (ArrayData*)dptr;                       \
  TypedValue* ret;                                        \
  if (UNLIKELY(!IsHphpArray(ad))) {                       \
    String s(sd);                                         \
    elem<String, KindOfString>(ad, s, out);               \
  } else {                                                \
    HphpArray* ha = (HphpArray*)(dptr);                   \
    ARRAY_GETM_TRACE();                                   \
    do {                                                  \
      ret = (body);                                       \
    } while (0);                                          \
    if (UNLIKELY(!ret)) {                                 \
      TV_WRITE_NULL(out);                                 \
    } else {                                              \
      tvDup((ret), (out));                                \
    }                                                     \
  }                                                       \
  if (drKey && sd->decRefCount() == 0) sd->release();     \
  return ad;                                              \
} while(0)

#define ARRAY_GETM_TRACE() do {                           \
  TRACE(2, "%s: (%p) <- %p[\"%s\"@sd%p]\n", __FUNCTION__, \
        (out), (dptr), (sd)->data(), (sd));               \
} while(0)

/**
 * array_getm_s: conservative unary string key.
 */
ArrayData*
array_getm_s(void* dptr, StringData* sd, TypedValue* out) {
  ARRAY_GETM_BODY(dptr, sd, out, nv_get_cell_with_integer_check(ha, sd),
                  true /* drKey */);
}

/**
 * array_getm_s0: unary string key where we know there is no need
 * to decRef the key.
 */
ArrayData*
array_getm_s0(void* dptr, StringData* sd, TypedValue* out) {
  ARRAY_GETM_BODY(dptr, sd, out, nv_get_cell_with_integer_check(ha, sd),
                  false /* drKey */);
}

/**
 * array_getm_s_fast:
 * array_getm_s0:
 *
 *   array_getm_s[0] but without the integer check on the key
 */
ArrayData*
array_getm_s_fast(void* dptr, StringData* sd, TypedValue* out) {
  ARRAY_GETM_BODY(dptr, sd, out, ha->nvGetCell(sd, true /* error */),
                  true /* drKey */);
}

ArrayData*
array_getm_s0_fast(void* dptr, StringData* sd, TypedValue* out) {
  ARRAY_GETM_BODY(dptr, sd, out, ha->nvGetCell(sd, true /* error */),
                  false /* drKey */);
}
#undef ARRAY_GETM_TRACE

#define ARRAY_GETM_TRACE() do {                           \
  TRACE(2, "%s: (%p) <- %p[%lld][\"%s\"@sd%p]\n",         \
        __FUNCTION__, (out), (dptr), (ik), (sd)->data(),  \
        (sd));                                            \
} while(0)

/**
 * array_getm_is will increment the refcount of the return value if
 * appropriate and it will decrement the refcount of the string key
 */
ArrayData*
array_getm_is(void *dptr, int64 ik, StringData* sd, TypedValue *out) {
  ARRAY_GETM_BODY(dptr, sd, out, ha->nvGetCell(ik, sd),
                  true /* drKey */);
}

/**
 * array_getm_is0 will increment the refcount of the return value if
 * appropriate
 */
ArrayData*
array_getm_is0(void *dptr, int64 ik, StringData* sd, TypedValue *out) {
  ARRAY_GETM_BODY(dptr, sd, out,
                  ha->nvGetCell(ik, sd, true /*error*/),
                  false /*drKey*/);
}
#undef ARRAY_GETM_TRACE
#undef ARRAY_GETM_BODY

// issetm's DNA.
static inline bool
issetMUnary(const void* dptr, StringData* sd,
            bool decRefKey = true, bool checkInt = true) {
  const ArrayData* ad = (const ArrayData*)dptr;
  bool retval;
  if (UNLIKELY(!IsHphpArray(ad))) {
    retval = ad->exists(String(sd));
  } else {
    const HphpArray* ha = (const HphpArray*)dptr;
    int64 keyAsInt;

    TypedValue* cell;
    if (checkInt && sd->isStrictlyInteger(keyAsInt)) {
      cell = ha->nvGet(keyAsInt);
    } else {
      cell = ha->nvGet(sd);
    }
    retval = (cell != NULL) && !IS_NULL_TYPE(cell->m_type);
  }

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

/**
 * array_unsetm_s --
 * array_unsetm_s0 --
 *
 *   String-key removal. Might trigger copy-on-write. _s0 doesn't decref
 *   the key.
 */
static inline ArrayData* array_unsetm_s_common(ArrayData *ad, StringData* sd,
                                               bool decRef) {
  ArrayData* retval = 0;
  int64 lval;
  bool copy = ad->getCount() > 1;
  bool isInt = sd->isStrictlyInteger(lval);
  retval = isInt ? ad->remove(lval, copy) : ad->remove(*(String*)&sd, copy);
  if (decRef && sd->decRefCount() == 0) sd->release();
  return array_mutate_post(NULL, ad, retval);
}

ArrayData* array_unsetm_s0(ArrayData *ad, StringData* sd) {
  return array_unsetm_s_common(ad, sd, false);
}

ArrayData* array_unsetm_s(ArrayData *ad, StringData* sd) {
  return array_unsetm_s_common(ad, sd, true);
}

}

//=============================================================================

///////////////////////////////////////////////////////////////////////////////
}
