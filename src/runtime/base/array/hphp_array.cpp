/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/array/hphp_array.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/variable_serializer.h>
#include <util/hash.h>
#include <util/lock.h>
#include <util/alloc.h>

// If PEDANTIC is defined, extra checks are performed to ensure correct
// function even as an array approaches 2^31 elements.  In practice this is
// just wasted effort though, since such an array would require on the order of
// 128 GiB of memory.
//#define PEDANTIC

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SMART_ALLOCATION(HphpArray, SmartAllocatorImpl::NeedRestoreOnce);
//=============================================================================
// Static members.

StaticEmptyHphpArray StaticEmptyHphpArray::s_theEmptyArray;

//=============================================================================
// Helpers.

static inline uint32 computeMaxElms(uint32 lgTableSize) {
  ASSERT(lgTableSize <= 32);
  // maxElms is 0.75*tableSize.  Take care to avoid shifting by 32 bits.
  return ((uint32(3)) << (lgTableSize-2));
}

static inline size_t computeTableSize(uint32 lgTableSize) {
  ASSERT(lgTableSize <= 32);
  return ((size_t(1U)) << lgTableSize);
}

static inline size_t computeTableMask(uint32 lgTableSize) {
  ASSERT(lgTableSize <= 32);
  return computeTableSize(lgTableSize) - 1;
}

static inline uint32 computeLgTableSize(uint32 nEntries) {
  ASSERT(nEntries <= 0x7fffffffU);
  uint32 lgTableSize = HphpArray::MinLgTableSize;
  ASSERT(lgTableSize >= 2);
  ASSERT(lgTableSize <= 32);
  while (computeMaxElms(lgTableSize) < nEntries) {
    ++lgTableSize;
  }
  ASSERT(lgTableSize <= 32);
  return lgTableSize;
}

static inline HphpArray::Elm* data2Elms(void* data) {
  ASSERT((HphpArray::ElmAlignment & HphpArray::ElmAlignmentMask) == 0);
  HphpArray::Elm* elms = (HphpArray::Elm*)((uintptr_t(data)
                                           + HphpArray::ElmAlignmentMask)
                                           & ~HphpArray::ElmAlignmentMask);
  ASSERT((uintptr_t(elms) & HphpArray::ElmAlignmentMask) == 0);
  return elms;
}

static inline HphpArray::ElmInd* elms2Hash(HphpArray::Elm* elms,
                                           size_t maxElms) {
  HphpArray::ElmInd* hash = (HphpArray::ElmInd*)(uintptr_t(elms)
                                                 + (maxElms
                                                    * sizeof(HphpArray::Elm)));
  ASSERT((uintptr_t(hash) & HphpArray::ElmAlignmentMask) == 0);
  return hash;
}

static inline HphpArray::Elm* hash2Elms(HphpArray::ElmInd* hash,
                                        size_t maxElms) {
  HphpArray::Elm* elms = (HphpArray::Elm*)(uintptr_t(hash)
                                           - (maxElms
                                              * sizeof(HphpArray::Elm)));
  ASSERT((uintptr_t(elms) & HphpArray::ElmAlignmentMask) == 0);
  return elms;
}

static inline bool validElmInd(const HphpArray::ElmInd ei) {
  return (ei >= 0);
  // Alternative:
  //   return ((ei & HphpArray::ElmIndTombstone)
  //           != HphpArray::ElmIndTombstone);
}

static inline void initHash(HphpArray::ElmInd* hash, size_t tableSize) {
  ASSERT(HphpArray::ElmIndEmpty == -1);
  memset(hash, 0xffU, tableSize * sizeof(HphpArray::ElmInd));
}

//=============================================================================
// Construction/destruction.

HphpArray::HphpArray(uint nSize /* = 0 */)
  : m_data(NULL), m_nextKI(0), m_nElms(0), m_hLoad(0), m_lastE(ElmIndEmpty),
    m_linear(false), m_siPastEnd(false) {
  m_lgTableSize = computeLgTableSize(nSize);
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data);
  m_hash = elms2Hash(elms, maxElms);
  initHash(m_hash, tableSize);
  m_pos = ArrayData::invalid_index;
}

// Empty constructor for internal use.
HphpArray::HphpArray(int,int) {
}

HphpArray::~HphpArray() {
  Elm* elms = data2Elms(m_data);
  for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
    Elm* e = &elms[pos];
    if (e->data.m_type != KindOfTombstone) {
      if (e->key != NULL) {
        if (e->key->decRefCount() == 0) {
          e->key->release();
        }
      }
      if (IS_REFCOUNTED_TYPE(e->data.m_type)) {
        tvDecRef(&e->data);
      }
    }
  }
  if (m_data != NULL) {
    if (!m_linear) {
      free(m_data);
    }
  }
}

void HphpArray::dumpDebugInfo() const {
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  Elm* elms = hash2Elms(m_hash, maxElms);

  fprintf(stderr,
          "--- dumpDebugInfo(this=0x%08zx) ----------------------------\n",
         uintptr_t(this));
  fprintf(stderr, "m_data = %p\n"
         "elms   = %p\tm_hash = %p\n"
         "m_lgTableSize = %u\tm_nElms = %d\tm_hLoad = %d\n"
         "m_nextKI = %lld\t\tm_lastE = %d\tm_pos = %zd\tm_linear = %s\n",
         m_data, elms, m_hash,
         m_lgTableSize, m_nElms, m_hLoad, m_nextKI, m_lastE, m_pos,
         m_linear ? "true" : "false");
  fprintf(stderr, "Elements:\n");
  for (ElmInd i = 0; i <= m_lastE; ++i) {
    if (elms[i].data.m_type != KindOfTombstone) {
      Variant v = tvAsVariant(&elms[i].data);
      VariableSerializer vs(VariableSerializer::DebugDump);
      Variant v2(vs.serialize(v, true));
      String s = v2.toString().data();
      if (elms[i].key != NULL) {
        String k = Util::escapeStringForCPP(elms[i].key->data());
        fprintf(stderr, "  [%3d] hash=0x%016llx key=\"%s\" data=(%.*s)\n",
               int(i), elms[i].h, k.data(), s.size()-1, s.data());
      } else {
        fprintf(stderr, "  [%3d] ind=%lld data.m_type=(%.*s)\n", int(i),
               elms[i].h, s.size()-1, s.data());
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

inline HphpArray::ElmInd HphpArray::nextElm(Elm* elms, ElmInd ei) const {
  while (ei < m_lastE) {
    ++ei;
    if (elms[ei].data.m_type != KindOfTombstone) {
      return ei;
    }
  }
  return ElmIndEmpty;
}

ssize_t HphpArray::iter_begin() const {
  Elm* elms = data2Elms(m_data);
  return ssize_t(nextElm(elms, ElmIndEmpty));
}

ssize_t HphpArray::iter_end() const {
  Elm* elms = data2Elms(m_data);
  for (ElmInd pos = m_lastE; pos >= 0; --pos) {
    if (elms[pos].data.m_type != KindOfTombstone) {
      return ssize_t(pos);
    }
  }
  return ArrayData::invalid_index;
}

ssize_t HphpArray::iter_advance(ssize_t pos) const {
  if (pos == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  Elm* elms = data2Elms(m_data);
  return ssize_t(nextElm(elms, (ElmInd)pos));
}

ssize_t HphpArray::iter_rewind(ssize_t pos) const {
  if (pos == ArrayData::invalid_index) {
    return ArrayData::invalid_index;
  }
  ElmInd ipos = pos;
  ASSERT(ssize_t(ipos) == pos);
  Elm* elms = data2Elms(m_data);
  for (--ipos; ipos >= 0; --ipos) {
    if (elms[ipos].data.m_type != KindOfTombstone) {
      return ssize_t(ipos);
    }
  }
  return ArrayData::invalid_index;
}

Variant HphpArray::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  if (e->key != NULL) {
    return e->key; // String key.
  }
  return e->h; // Integer key.
}

Variant HphpArray::getValue(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)pos];
  return *(const Variant*)&(e->data);
}

void HphpArray::fetchValue(ssize_t pos, Variant& v) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)pos];
  v = tvAsCVarRef(&e->data);
}

CVarRef HphpArray::getValueRef(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)pos];
  return tvAsCVarRef(&e->data);
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
    if (e->key != NULL || e->h != i) {
      return false;
    }
    ++i;
  }
  return true;
}

Variant HphpArray::reset() {
  if (m_nElms > 0) {
    Elm* elms = data2Elms(m_data);
    m_pos = ssize_t(nextElm(elms, ElmIndEmpty));
    ASSERT(m_pos != ArrayData::invalid_index);
    Elm* e = &elms[(ElmInd)m_pos];
    return tvAsCVarRef(&e->data);
  }
  m_pos = ArrayData::invalid_index;
  return false;
}

Variant HphpArray::prev() {
  if (m_pos != ArrayData::invalid_index) {
    ASSERT(m_nElms > 0);
    Elm* elms = data2Elms(m_data);

    --m_pos;
    while (m_pos >= 0) {
      Elm* e = &elms[(ElmInd)m_pos];
      if (e->data.m_type != KindOfTombstone) {
        return tvAsCVarRef(&e->data);
      }
      --m_pos;
    }
    m_pos = ArrayData::invalid_index;
  }
  return false;
}

Variant HphpArray::next() {
  if (m_pos != ArrayData::invalid_index) {
    ASSERT(m_nElms > 0);
    Elm* elms = data2Elms(m_data);
    m_pos = ssize_t(nextElm(elms, (ElmInd)m_pos));
    if (m_pos != ArrayData::invalid_index) {
      Elm* e = &elms[(ElmInd)m_pos];
      return tvAsCVarRef(&e->data);
    }
  }
  return false;
}

Variant HphpArray::end() {
  if (m_lastE != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    ASSERT(elms[m_lastE].data.m_type != KindOfTombstone);
    m_pos = ssize_t(m_lastE);
    return tvAsCVarRef(&elms[(ElmInd)m_pos].data);
  }
  m_pos = ArrayData::invalid_index;
  return false;
}

Variant HphpArray::key() const {
  if (m_pos != ArrayData::invalid_index) {
    ASSERT((ElmInd)m_pos <= m_lastE);
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
    Elm* e = &elms[(ElmInd)pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    return tvAsCVarRef(&e->data);
  }
  return false;
}

Variant HphpArray::current() const {
  if (m_pos != ArrayData::invalid_index) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[(ElmInd)m_pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    return tvAsCVarRef(&e->data);
  }
  return false;
}

static StaticString s_value("value");
static StaticString s_key("key");

Variant HphpArray::each() {
  if (m_pos != ArrayData::invalid_index) {
    ArrayInit init(4, false);
    Variant key = getKey((ElmInd)m_pos);
    Variant value = getValue((ElmInd)m_pos);
    init.set(int64(1), value);
    init.set(s_value, value, true);
    init.set(int64(0), key);
    init.set(s_key, key, true);
    Elm* elms = data2Elms(m_data);
    m_pos = ssize_t(nextElm(elms, (ElmInd)m_pos));
    return Array(init.create());
  }
  return false;
}

//=============================================================================
// Lookup.

static inline bool hitStringKey(const HphpArray::Elm* e, const char* k, int len,
                                int64 hash) {
  if (e->key == NULL) {
    return false;
  }
  ASSERT(e->data.m_type != HphpArray::KindOfTombstone); // As (e->key != NULL).
  const char* data = e->key->data();
  return data == k || (e->h == hash && e->key->size() == len
                       && (memcmp(data, k, len) == 0));
}

static inline bool hitIntKey(const HphpArray::Elm* e, int64 ki) {
  return e->h == ki && e->key == NULL
         && e->data.m_type != HphpArray::KindOfTombstone;
}

// Quadratic probe is:
//
//   h(k, i) = (k + c1*i + c2*(i^2)) % tableSize
//
// Use 1/2 for c1 and c2.  In combination with a table size that is a power of
// 2, this guarantees a probe sequence of length tableSize that probes all
// table elements exactly once.
static inline size_t qProbe(size_t k, size_t i, size_t tableMask) {
  return (k + ((i + i*i) >> 1)) & tableMask;
}

#define FIND_BODY(h0, hit)                                                    \
  size_t maxElms = computeMaxElms(m_lgTableSize);                             \
  size_t tableMask = computeTableMask(m_lgTableSize);                         \
  size_t h = size_t(h0) & tableMask;                                          \
  Elm* elms = hash2Elms(m_hash, maxElms);                                     \
  /* Primary probe (unrolled to avoid multiplication for (i == 0)). */        \
  ElmInd pos = m_hash[h];                                                     \
  if (validElmInd(pos)) {                                                     \
    Elm* e = &elms[pos];                                                      \
    if (hit) {                                                                \
      return pos;                                                             \
    }                                                                         \
  } else if (pos == ElmIndEmpty) {                                            \
    return ElmIndEmpty;                                                       \
  }                                                                           \
  /* Quadratic secondary probe. */                                            \
  for (size_t i = 1;; ++i) {                                                  \
    ASSERT(i < computeTableSize(m_lgTableSize));                              \
    size_t probeIndex = qProbe(h, i, tableMask);                              \
    pos = m_hash[probeIndex];                                                 \
    if (validElmInd(pos)) {                                                   \
      Elm* e = &elms[pos];                                                    \
      if (hit) {                                                              \
        return pos;                                                           \
      }                                                                       \
    } else if (pos == ElmIndEmpty) {                                          \
      return ElmIndEmpty;                                                     \
    }                                                                         \
  }

HphpArray::ElmInd HphpArray::find(int64 ki) const {
  FIND_BODY(ki, hitIntKey(e, ki))
}

HphpArray::ElmInd HphpArray::find(const char* k, int len, int64 prehash) const {
  FIND_BODY(prehash, hitStringKey(e, k, len, prehash))
}
#undef FIND_BODY

#define FIND_FOR_INSERT_BODY(h0, hit)                                         \
  ASSERT(!m_linear);                                                          \
  ElmInd* ret = NULL;                                                         \
  size_t maxElms = computeMaxElms(m_lgTableSize);                             \
  size_t tableMask = computeTableMask(m_lgTableSize);                         \
  size_t h = size_t(h0) & tableMask;                                          \
  Elm* elms = hash2Elms(m_hash, maxElms);                                     \
  /* Primary probe (unrolled to avoid multiplication for (i == 0)). */        \
  ElmInd* ei = &m_hash[h];                                                    \
  ElmInd pos = *ei;                                                           \
  if (validElmInd(pos)) {                                                     \
    Elm* e = &elms[pos];                                                      \
    if (hit) {                                                                \
      ASSERT(m_hLoad <= computeMaxElms(m_lgTableSize));                       \
      return ei;                                                              \
    }                                                                         \
  } else {                                                                    \
    if (ret == NULL) {                                                        \
      ret = ei;                                                               \
    }                                                                         \
    if (pos == ElmIndEmpty) {                                                 \
      ASSERT((*ret == ElmIndEmpty)                                            \
             ? (m_hLoad < computeMaxElms(m_lgTableSize))                      \
             : (m_hLoad <= computeMaxElms(m_lgTableSize))                     \
             );                                                               \
      return ret;                                                             \
    }                                                                         \
  }                                                                           \
  /* Quadratic secondary probe. */                                            \
  for (size_t i = 1;; ++i) {                                                  \
    ASSERT(i < computeTableSize(m_lgTableSize));                              \
    size_t probeIndex = qProbe(h, i, tableMask);                              \
    ei = &m_hash[probeIndex];                                                 \
    pos = *ei;                                                                \
    if (validElmInd(pos)) {                                                   \
      Elm* e = &elms[pos];                                                    \
      if (hit) {                                                              \
        ASSERT(m_hLoad <= computeMaxElms(m_lgTableSize));                     \
        return ei;                                                            \
      }                                                                       \
    } else {                                                                  \
      if (ret == NULL) {                                                      \
        ret = ei;                                                             \
      }                                                                       \
      if (pos == ElmIndEmpty) {                                               \
        ASSERT((*ret == ElmIndEmpty)                                          \
               ? (m_hLoad < computeMaxElms(m_lgTableSize))                    \
               : (m_hLoad <= computeMaxElms(m_lgTableSize))                   \
               );                                                             \
        return ret;                                                           \
      }                                                                       \
    }                                                                         \
  }

HphpArray::ElmInd* HphpArray::findForInsert(int64 ki) const {
  FIND_FOR_INSERT_BODY(ki, hitIntKey(e, ki))
}

HphpArray::ElmInd* HphpArray::findForInsert(const char* k, int len,
                                            int64 prehash) const {
  FIND_FOR_INSERT_BODY(prehash, hitStringKey(e, k, len, prehash))
}
#undef FIND_FOR_INSERT_BODY

bool HphpArray::exists(int64 k) const {
  return find(k) != ElmIndEmpty;
}

bool HphpArray::exists(litstr k) const {
  return find(k, strlen(k), hash_string(k)) != ElmIndEmpty;
}

bool HphpArray::exists(CStrRef k) const {
  return find(k.data(), k.size(), k->hash()) != ElmIndEmpty;
}

bool HphpArray::exists(CVarRef k) const {
  if (k.isNumeric()) {
    return find(k.toInt64()) != ElmIndEmpty;
  }
  String key = k.toString();
  return find(key.data(), key.size(), key->hash()) != ElmIndEmpty;
}

bool HphpArray::idxExists(ssize_t idx) const {
  return (idx != ArrayData::invalid_index);
}

Variant HphpArray::get(int64 k, bool error /* = false */) const {
  ElmInd pos = find(k);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return tvAsCVarRef(&e->data);
  }
  if (error) {
    raise_notice("Undefined index: %lld", k);
  }
  return null;
}

Variant HphpArray::get(litstr k, bool error /* = false */) const {
  int len = strlen(k);
  ElmInd pos = find(k, len, hash_string(k, len));
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return tvAsCVarRef(&e->data);
  }
  if (error) {
    raise_notice("Undefined index: %s", k);
  }
  return null;
}

Variant HphpArray::get(CStrRef k, bool error /* = false */) const {
  StringData* key = k.get();
  int64 prehash = key->hash();
  ElmInd pos = find(key->data(), key->size(), prehash);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return tvAsCVarRef(&e->data);
  }
  if (error) {
    raise_notice("Undefined index: %s", k.data());
  }
  return null;
}

Variant HphpArray::get(CVarRef k, bool error /* = false */) const {
  ElmInd pos;
  if (k.isNumeric()) {
    pos = find(k.toInt64());
  } else {
    String key = k.toString();
    StringData* strkey = key.get();
    int64 prehash = strkey->hash();
    pos = find(strkey->data(), strkey->size(), prehash);
  }
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return tvAsCVarRef(&e->data);
  }
  if (error) {
    raise_notice("Undefined index: %s", k.toString().data());
  }
  return null;
}

Variant HphpArray::fetch(CStrRef k) const {
  StringData* key = k.get();
  int64 prehash = key->hash();
  ElmInd pos = find(key->data(), key->size(), prehash);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    return tvAsCVarRef(&e->data);
  }
  return false;
}

void HphpArray::load(CVarRef k, Variant& v) const {
  ElmInd pos;
  if (k.isNumeric()) {
    pos = find(k.toInt64());
  } else {
    String key = k.toString();
    StringData* strkey = key.get();
    int64 prehash = strkey->hash();
    pos = find(strkey->data(), strkey->size(), prehash);
  }
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    if (tvAsCVarRef(&e->data).isReferenced()) {
      v = ref(tvAsCVarRef(&e->data));
    } else {
      v = tvAsCVarRef(&e->data);
    }
  }
}

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
  if (k.isNumeric()) {
    return ssize_t(find(k.toInt64()));
  } else {
    String key = k.toString();
    return ssize_t(find(key.data(), key.size(), key->hash()));
  }
}

//=============================================================================
// Append/insert/update.

HphpArray::Elm* HphpArray::allocElm(ElmInd* ei) {
  ASSERT(!m_linear);
  ASSERT(!validElmInd(*ei));
  ASSERT(m_nElms != 0 || m_lastE == ElmIndEmpty);
#ifdef PEDANTIC
  if (m_nElms == 0x7fffffff) {
    raise_error("Cannot insert into array with (2^31)-1 elements");
  }
#endif
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
  // If there could be any strong iterators that are past the end, we need to a
  // pass and update these iterators to point to the newly added element.
  if (m_siPastEnd) {
    m_siPastEnd = false;
    int sz = m_strongIterators.size();
    bool shouldWarn = false;
    for (int i = 0; i < sz; ++i) {
      if (m_strongIterators[i]->primary == ssize_t(ElmIndEmpty)) {
        m_strongIterators[i]->primary = ssize_t(*ei);
        shouldWarn = true;
      }
    }
    if (shouldWarn) {
      raise_warning("An element was added to an array while a foreach by"
                    " reference loop was iterating over the last element of the"
                    " array. This may lead to unexpeced results.");
    }
  }
  return e;
}

void HphpArray::reallocData(size_t maxElms, size_t tableSize) {
  // Allocate extra padding space so that if the resulting region is not
  // Elm-aligned, there is enough padding to be able to leave the first bytes
  // unused and start the element table at an Elm alignment boundary.
  //
  // Ideally it would be possible to specify alignment during reallocation, but
  // no allocator interface for this currently exists.
  //
  // NB: It would be possible to optimistically allocate without padding, then
  // reallocate if alignment was inadquate.  However, this would not save very
  // much memory in practice, and recovering from the OOM failure case for the
  // reallocation would be messy to handle correctly.
  void* data = realloc(m_linear ? NULL : m_data,
                       (maxElms * sizeof(Elm))
                       + (tableSize * sizeof(ElmInd))
                       + ElmAlignment); // <-- pad
  if (data == NULL) {
    throw OutOfMemoryException(tableSize);
  }
  if (!m_linear) {
    size_t oldPad = uintptr_t(data2Elms(m_data)) - uintptr_t(m_data);
    size_t pad = uintptr_t(data2Elms(data)) - uintptr_t(data);
    if (pad != oldPad) {
      // The alignment padding changed due to realloc(), so move the element
      // array to its proper offset.
      Elm* misalignedElms = (Elm*)(uintptr_t(data) + oldPad);
      Elm* elms = data2Elms(data);
      memmove((void*)elms, (void*)misalignedElms, (m_lastE+1) * sizeof(Elm));
    }
  } else {
    Elm* oldElms = data2Elms(m_data);
    Elm* elms = data2Elms(data);
    memcpy((void*)elms, (void*)oldElms, (m_lastE+1) * sizeof(Elm));
    m_linear = false;
  }
  m_data = data;
}

void HphpArray::delinearize() {
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data);
  ElmInd* oldHash = m_hash;
  m_hash = elms2Hash(elms, maxElms);
  memcpy((void*)m_hash, (void*)oldHash, tableSize * sizeof(ElmInd));
}

inline void HphpArray::resize() {
  uint32 maxElms = computeMaxElms(m_lgTableSize);
  ASSERT(m_lastE == ElmIndEmpty || uint32(m_lastE)+1 <= maxElms);
  ASSERT(m_hLoad <= maxElms);
  if (uint32(m_lastE)+1 == maxElms || m_hLoad == maxElms) {
    // At a minimum, compaction is required.  If the load factor would be >0.5
    // even after compaction, grow instead, in order to avoid the possibility
    // of repeated compaction if the load factor were to hover at nearly 0.75.
    if (m_nElms > (ElmInd)(maxElms >> 1)) {
      grow();
    } else {
      compact();
    }
  } else if (m_nElms < ((m_lastE+1) >> 1)) {
    // Compact in order to keep elms from being overly sparse.
    compact();
  }
}

void HphpArray::grow() {
  ++m_lgTableSize;
  ASSERT(m_lgTableSize <= 32);
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  reallocData(maxElms, tableSize);
  Elm* elms = data2Elms(m_data); // m_hash is currently invalid.
  m_hash = elms2Hash(elms, maxElms);

  // All the elements have been copied and their offsets from the base are
  // still the same, so we just need to build the new hash table.
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_nElms until after rebuilding is complete, in
  // order to maintain invariants in findForInsert().
  m_hLoad = 0;
#else
  m_hLoad = m_nElms;
#endif

  if (m_nElms > 0) {
    size_t tableMask = computeTableMask(m_lgTableSize);
    Elm* elms = hash2Elms(m_hash, maxElms);
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &elms[pos];
      if (e->data.m_type == KindOfTombstone) {
        continue;
      }
      size_t h = size_t(e->h) & tableMask;
      for (size_t i = 0;; ++i) {
        ASSERT(i < tableSize);
        size_t probeIndex = qProbe(h, i, tableMask);
        ElmInd* ei = &m_hash[probeIndex];
        if (*ei == ElmIndEmpty) {
          *ei = pos;
          break;
        }
      }
    }
#ifndef DEBUG
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
      ElmInd ei = (ElmInd)m_strongIterators[i]->primary;
      if (ei != ElmIndEmpty) {
        siKeys[i] = *(ElmKey*)&elms[(ElmInd)m_strongIterators[i]->primary];
      }
    }
  }
  if (renumber) {
    m_nextKI = 0;
  }
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  Elm* elms = hash2Elms(m_hash, maxElms);
  initHash(m_hash, tableSize);
#ifdef DEBUG
  // Wait to set m_hLoad to m_nElms until after rebuilding is complete, in
  // order to maintain invariants in findForInsert().
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
    ElmInd* ie;
    if (toE->key != NULL) {
      ie = findForInsert(toE->key->data(), toE->key->size(), toE->h);
    } else {
      if (renumber) {
        toE->h = m_nextKI;
        ++m_nextKI;
      }
      ie = findForInsert(toE->h);
    }
    *ie = toPos;
    ++frPos;
  }
  m_lastE = m_nElms - 1;
#ifndef DEBUG
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
      ssize_t* siPos = &m_strongIterators[i]->primary;
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

bool HphpArray::nextInsert(CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  resize();
  int64 ki = m_nextKI;
  ElmInd* ei = findForInsert(ki);
  ASSERT(!validElmInd(*ei));

  // Allocate a new element.
  Elm* e = allocElm(ei);
  // Set key.
  e->h = ki;
  e->key = NULL;
  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  tvAsVariant(&e->data) = data;
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
  resize();
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    *pDest = (Variant*)&(e->data);
    return false;
  }

  Elm* e = allocElm(ei);

  e->h = ki;
  e->key = NULL;

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  *pDest = &(tvAsVariant(&e->data));

  if (ki >= m_nextKI) {
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
  resize();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    *pDest = &(tvAsVariant(&e->data));
    return false;
  }

  Elm* e = allocElm(ei);
  // Set key.
  e->h = h;
  e->key = key;
  e->key->incRefCount();
  // Initialize element to null and store the address of the element into
  // *pDest.
  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  *pDest = &(tvAsVariant(&e->data));

  return true;
}

inline bool HphpArray::addVal(int64 ki, CVarRef data,
                              bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  resize();
  ElmInd* ei = findForInsert(ki);
  if (checkExists && validElmInd(*ei)) {
    return false;
  } else {
    ASSERT(!validElmInd(*ei));
  }

  Elm* e = allocElm(ei);

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;

  e->h = ki;
  e->key = NULL;

  tvAsVariant(&e->data) = data;

  if (ki >= m_nextKI) {
    m_nextKI = ki + 1;
  }
  return true;
}

inline bool HphpArray::addVal(StringData* key, CVarRef data,
                              bool checkExists /* = true */) {
  if (m_linear) {
    delinearize();
  }
  resize();
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

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  tvAsVariant(&e->data) = data;

  return true;
}

bool HphpArray::update(int64 ki, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  resize();
  ElmInd* ei = findForInsert(ki);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    tvAsVariant(&e->data) = data;
    return true;
  }

  Elm* e = allocElm(ei);

  e->h = ki;
  e->key = NULL;

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  tvAsVariant(&e->data) = data;

  if (ki >= m_nextKI) {
    m_nextKI = ki + 1;
  }

  return true;
}

bool HphpArray::update(litstr key, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  resize();
  int len = strlen(key);
  int64 h = hash_string(key, len);
  ElmInd* ei = findForInsert(key, len, h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    tvAsVariant(&e->data) = data;
    return true;
  }

  Elm* e = allocElm(ei);

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;

  e->h = h;
  e->key = NEW(StringData)(key, len, AttachLiteral);
  e->key->incRefCount();
  tvAsVariant(&e->data) = data;

  return true;
}

bool HphpArray::update(StringData* key, CVarRef data) {
  if (m_linear) {
    delinearize();
  }
  resize();
  int64 h = key->hash();
  ElmInd* ei = findForInsert(key->data(), key->size(), h);
  if (validElmInd(*ei)) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[*ei];
    tvAsVariant(&e->data) = data;
    return true;
  }

  Elm* e = allocElm(ei);

  e->h = h;
  e->key = key;
  e->key->incRefCount();

  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  tvAsVariant(&e->data) = data;

  return true;
}

ArrayData* HphpArray::lval(Variant*& ret, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    ASSERT(a->m_lastE != ElmIndEmpty);
    Elm* aElms = data2Elms(a->m_data);
    ret = &tvAsVariant(&aElms[m_lastE].data);
    return a;
  }
  ASSERT(m_lastE != ElmIndEmpty);
  Elm* elms = data2Elms(m_data);
  ret = &(tvAsVariant(&elms[m_lastE].data));
  return NULL;
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
  ElmInd pos = find(k);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    ret = &(tvAsVariant(&e->data));
    return NULL;
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
  ElmInd pos = find(key->data(), key->size(), prehash);
  if (pos != ElmIndEmpty) {
    Elm* elms = data2Elms(m_data);
    Elm* e = &elms[pos];
    ret = &(tvAsVariant(&e->data));
    return NULL;
  }
  HphpArray* a = copyImpl();
  a->addLvalImpl(key, prehash, &ret, false);
  return a;
}

ArrayData* HphpArray::lval(CVarRef k, Variant*& ret, bool copy,
                           bool checkExist /* = false */) {
  if (k.isNumeric()) {
    return lval(k.toInt64(), ret, copy, checkExist);
  }
  return lval(k.toString(), ret, copy, checkExist);
}

ArrayData *HphpArray::lvalPtr(CStrRef k, Variant*& ret, bool copy,
                              bool create) {
  StringData *key = k.get();
  int64 prehash = key->hash();
  HphpArray* a = 0;
  HphpArray* t = this;
  if (copy) {
    a = t = copyImpl();
  }

  if (create) {
    t->addLvalImpl(key, prehash, &ret);
  } else {
    ElmInd pos = t->find(key->data(), key->size(), prehash);
    if (pos != ElmIndEmpty) {
      Elm* elms = data2Elms(m_data);
      Elm* e = &elms[pos];
      ret = &(tvAsVariant(&e->data));
    } else {
      ret = NULL;
    }
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

ArrayData* HphpArray::set(litstr k, CVarRef v, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->update(k, v);
    return a;
  }
  update(k, v);
  return NULL;
}

ArrayData* HphpArray::set(CVarRef k, CVarRef v, bool copy) {
  if (k.isNumeric()) {
    if (copy) {
      HphpArray* a = copyImpl();
      a->update(k.toInt64(), v);
      return a;
    }
    update(k.toInt64(), v);
    return NULL;
  }
  String sk = k.toString();
  StringData* sd = sk.get();
  if (copy) {
    HphpArray* a = copyImpl();
    a->update(sd, v);
    return a;
  }
  update(sd, v);
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
  if (k.isNumeric()) {
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
  if (k.isNumeric()) {
    return addLval(k.toInt64(), ret, copy);
  }
  return addLval(k.toString(), ret, copy);
}

//=============================================================================
// Delete.

void HphpArray::erase(ElmInd* ei) {
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
    if (m_strongIterators[i]->primary == ssize_t(pos)) {
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
      m_strongIterators[i]->primary = ssize_t(eINext);
    }
  }

  // If the internal pointer points to this element, advance it.
  if (m_pos == ssize_t(pos)) {
    if (eINext == ElmIndTombstone) {
      eINext = nextElm(elms, pos);
    }
    m_pos = ssize_t(eINext);
  }

  // Free the value if necessary and mark it as a tombstone.
  Elm* e = &elms[pos];
  if (IS_REFCOUNTED_TYPE(e->data.m_type)) {
    tvDecRef(&e->data);
  }
  e->data.m_type = KindOfTombstone;

  // Free the key if necessary, and clear the h and key fields in order to
  // increase the chances that subsequent searches will quickly/safely fail
  // when encountering tombstones, even though checking for KindOfTombstone is
  // the last validation step during search.
  if (e->key != NULL) {
    if (e->key->decRefCount() == 0) {
      e->key->release();
    }
    e->key = NULL;
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

  resize();

  if (nextElementUnsetInsideForeachByReference) {
    if (RuntimeOption::EnableHipHopErrors) {
      raise_error("Cannot unset the next element inside foreach by reference");
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

ArrayData* HphpArray::remove(litstr k, bool copy) {
  int len = strlen(k);
  int64 prehash = hash_string(k, len);
  if (copy) {
    HphpArray* a = copyImpl();
    a->erase(a->findForInsert(k, len, prehash));
    return a;
  }
  if (m_linear) {
    delinearize();
  }
  erase(findForInsert(k, len, prehash));
  return NULL;
}

ArrayData* HphpArray::remove(CVarRef k, bool copy) {
  if (k.isNumeric()) {
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
    String key = k.toString();
    int64 prehash = key->hash();
    if (copy) {
      HphpArray* a = copyImpl();
      a->erase(a->findForInsert(key.data(), key.size(), prehash));
      return a;
    }
    if (m_linear) {
      delinearize();
    }
    erase(findForInsert(key.data(), key.size(), prehash));
    return NULL;
  }
}

ArrayData* HphpArray::copy() const {
  return copyImpl();
}

HphpArray* HphpArray::copyImpl() const {
  HphpArray* target = NEW(HphpArray)(0,0);
  target->m_pos = m_pos;
  target->m_data = NULL;
  target->m_nextKI = m_nextKI;
  target->m_lgTableSize = m_lgTableSize;
  target->m_nElms = m_nElms;
  target->m_hLoad = m_hLoad;
  target->m_lastE = m_lastE;
  target->m_linear = false;
  target->m_siPastEnd = false;
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  target->reallocData(maxElms, tableSize);
  Elm* targetElms = data2Elms(target->m_data);
  target->m_hash = elms2Hash(targetElms, maxElms);
  // Copy the hash.
  memcpy(target->m_hash, m_hash, tableSize * sizeof(ElmInd));
  // Copy the elements and bump up refcounts as needed.
  if (m_nElms > 0) {
    Elm* elms = hash2Elms(m_hash, maxElms);
    for (ElmInd pos = 0; pos <= m_lastE; ++pos) {
      Elm* e = &elms[pos];
      Elm* te = &targetElms[pos];
      if (e->data.m_type != KindOfTombstone) {
        te->h = e->h;
        te->key = e->key;
        if (te->key != NULL) {
          te->key->incRefCount();
        }

        if (tvAsVariant(&e->data).isReferenced()) {
          tvAsVariant(&e->data).setContagious();
        }
        te->data.m_data.num = 0;
        te->data._count = 0;
        te->data.m_type = KindOfNull;
        tvAsVariant(&te->data) = tvAsVariant(&e->data);
      } else {
        // Tombstone.
        te->h = 0;
        te->key = NULL;
        te->data.m_type = KindOfTombstone;
      }
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

ArrayData* HphpArray::append(const ArrayData* elems, ArrayOp op, bool copy) {
  if (copy) {
    HphpArray* a = copyImpl();
    a->append(elems, op, false);
    return a;
  }

  if (elems->supportValueRef()) {
    if (op == Plus) {
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
        if (value.isReferenced()) {
          value.setContagious();
        }
        if (key.isNumeric()) {
          addVal(key.toInt64(), value);
        } else {
          String skey = key.toString();
          addVal(skey.get(), value);
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        CVarRef value = it.secondRef();
        if (value.isReferenced()) {
          value.setContagious();
        }
        if (key.isNumeric()) {
          nextInsert(value);
        } else {
          String skey = key.toString();
          update(skey.get(), value);
        }
      }
    }
  } else {
    if (op == Plus) {
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          addVal(key.toInt64(), it.second());
        } else {
          String skey = key.toString();
          addVal(skey.get(), it.second());
        }
      }
    } else {
      ASSERT(op == Merge);
      for (ArrayIter it(elems); !it.end(); it.next()) {
        Variant key = it.first();
        if (key.isNumeric()) {
          nextInsert(it.second());
        } else {
          String skey = key.toString();
          update(skey.get(), it.second());
        }
      }
    }
  }
  return NULL;
}

ArrayData* HphpArray::pop(Variant& value) {
  if (getCount() > 1) {
    HphpArray* a = copyImpl();
    a->pop(value);
    return a;
  }
  Elm* elms = data2Elms(m_data);
  if (m_nElms > 0) {
    if (m_linear) {
      delinearize();
    }
    ElmInd pos = m_lastE;
    Elm* e = &elms[pos];
    ASSERT(e->data.m_type != KindOfTombstone);
    value = tvAsCVarRef(&e->data);
    if (e->key == NULL && e->h == m_nextKI - 1) {
      --m_nextKI;
    }
    erase((e->key != NULL)
          ? findForInsert(e->key->data(), e->key->size(), e->h)
          : findForInsert(e->h)
          );
  } else {
    value = null;
  }
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator.
  m_pos = nextElm(elms, ElmIndEmpty);
  return NULL;
}

ArrayData* HphpArray::dequeue(Variant& value) {
  if (getCount() > 1) {
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
  if (m_nElms > 0) {
    if (m_linear) {
      delinearize();
    }
    ElmInd pos = nextElm(elms, ElmIndEmpty);
    Elm* e = &elms[pos];
    value = tvAsCVarRef(&e->data);
    erase((e->key != NULL)
          ? findForInsert(e->key->data(), e->key->size(), e->h)
          : findForInsert(e->h)
          );
    compact(true);
  } else {
    value = null;
  }
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator.
  m_pos = nextElm(elms, ElmIndEmpty);
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
    resize();
    // Recompute elms, in case resize() had side effects.
    elms = data2Elms(m_data);
    // Move the existing elements to make element 0 available.
#ifdef PEDANTIC
    if (m_nElms == 0x7fffffff) {
      raise_error("Cannot insert into array with (2^31)-1 elements");
    }
#endif
    memmove(&elms[1], &elms[0], (m_lastE+1) * sizeof(Elm));
    ++m_lastE;
  }
  // Prepend.
  Elm* e = &elms[0];
  e->key = NULL;
  e->data.m_data.num = 0;
  e->data._count = 0;
  e->data.m_type = KindOfNull;
  tvAsVariant(&e->data) = v;
  ++m_nElms;
  // Renumber.
  compact(true);
  // To match PHP-like semantics, the prepend operation resets the array's
  // internal iterator.
  m_pos = 0;

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
      tvAsVariant(&e->data).setStatic();
    }
  }
}

void HphpArray::getFullPos(FullPos& pos) {
  ASSERT(pos.container == (ArrayData*)this);
  pos.primary = m_pos;
  if (pos.primary == ssize_t(ElmIndEmpty)) {
    // Record that there is a strong iterator out there that is past the end.
    m_siPastEnd = true;
  }
}

bool HphpArray::setFullPos(const FullPos& pos) {
  ASSERT(pos.container == (ArrayData*)this);
  if (pos.primary != ssize_t(ElmIndEmpty)) {
    m_pos = pos.primary;
    return true;
  }
  return false;
}

CVarRef HphpArray::currentRef() {
  ASSERT(m_pos != ArrayData::invalid_index);
  Elm* elms = data2Elms(m_data);
  Elm* e = &elms[(ElmInd)m_pos];
  ASSERT(e->data.m_type != KindOfTombstone);
  return tvAsCVarRef(&e->data);
}

CVarRef HphpArray::endRef() {
  ASSERT(m_lastE != ElmIndEmpty);
  Elm* elms = data2Elms(m_data);
  ElmInd pos = m_lastE;
  Elm* e = &elms[pos];
  return tvAsCVarRef(&e->data);
}

//=============================================================================
// Memory allocator methods.

bool HphpArray::calculate(int& size) {
  size += sizeof(void*); // Pointer to aligned data (starts out NULL).
  size += computeMaxElms(m_lgTableSize) * sizeof(Elm); // Array elements.
  size += computeTableSize(m_lgTableSize) * sizeof(ElmInd); // Hash table.
  size += ElmAlignment; // Padding to allow for alignment in restore().
  return true;
}

void HphpArray::backup(LinearAllocator& allocator) {
  Elm* elms = data2Elms(m_data);
  void* alignedData = NULL;
  allocator.backup((const char*)&alignedData, sizeof(void*));
  allocator.backup((const char*)elms,
                   computeMaxElms(m_lgTableSize) * sizeof(Elm));
  allocator.backup((const char*)m_hash,
                   computeTableSize(m_lgTableSize) * sizeof(ElmInd));
  Elm pad;
  memset((void*)&pad, 0, sizeof(Elm));
  allocator.backup((const char*)&pad, sizeof(Elm));
  ASSERT(m_strongIterators.empty());
}

void HphpArray::restore(const char*& data) {
  size_t maxElms = computeMaxElms(m_lgTableSize);
  size_t tableSize = computeTableSize(m_lgTableSize);
  void** alignedData = (void**)data;
  data += sizeof(void*);

  if (*alignedData == NULL) {
    m_data = (void*)data2Elms((void*)data);
    if (m_data != data) {
      // Move data in order to guarantee proper alignment.  This only happens
      // (at most) the first time restore() is called for this linearized array.
      memmove(m_data, data,
              (maxElms * sizeof(Elm)) + (tableSize * sizeof(ElmInd)));
    }
    *alignedData = m_data;
  } else {
    m_data = (void*)data2Elms(*alignedData);
  }

  Elm* elms = data2Elms(m_data);
  m_hash = elms2Hash(elms, maxElms);

  data += maxElms * sizeof(Elm);
  data += tableSize * sizeof(ElmInd);
  data += ElmAlignment;
  m_linear = true;
  m_strongIterators.m_data = NULL;
}

void HphpArray::sweep() {
  if (m_data != NULL) {
    if (!m_linear) {
      free(m_data);
    }
    m_data = NULL;
  }
  m_strongIterators.clear();
}

///////////////////////////////////////////////////////////////////////////////
}
