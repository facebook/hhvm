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

#ifndef incl_HPHP_SHARED_VARIANT_H_
#define incl_HPHP_SHARED_VARIANT_H_

#include "hphp/runtime/base/types.h"
#include "hphp/util/lock.h"
#include "hphp/util/hash.h"
#include "hphp/util/atomic.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/immutable-array.h"

#if (defined(__APPLE__) || defined(__APPLE_CC__)) && (defined(__BIG_ENDIAN__) || defined(__LITTLE_ENDIAN__))
# if defined(__LITTLE_ENDIAN__)
#  undef WORDS_BIGENDIAN
# else
#  if defined(__BIG_ENDIAN__)
#   define WORDS_BIGENDIAN
#  endif
# endif
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedArray;
class SharedVariantStats;
class ImmutablePackedArray;
class ImmutableArray;
struct ImmutableObj;

///////////////////////////////////////////////////////////////////////////////

class SharedVariant {
public:
  SharedVariant(CVarRef source, bool serialized, bool inner = false,
                bool unserializeObj = false);
  ~SharedVariant();

  // Create will do the wrapped check before creating a SharedVariant
  static SharedVariant* Create(CVarRef source, bool serialized,
                               bool inner = false,
                               bool unserializeObj = false);

  bool is(DataType d) const { return m_type == d; }
  DataType getType() const { return (DataType)m_type; }
  CVarRef asCVarRef() const {
    // Must be non-refcounted types
    assert(m_shouldCache == false);
    assert(m_flags == 0);
    assert(!IS_REFCOUNTED_TYPE(m_type));
    return tvAsCVarRef(reinterpret_cast<const TypedValue*>(this));
  }

  void incRef() {
    assert(IS_REFCOUNTED_TYPE(m_type));
    ++m_count;
  }

  void decRef() {
    assert(m_count.load());
    if (IS_REFCOUNTED_TYPE(m_type)) {
      if (--m_count == 0) {
        delete this;
      }
    } else {
      assert(m_count.load() == 1);
      delete this;
    }
  }

  Variant toLocal();

  int64_t intData() const {
    assert(is(KindOfInt64));
    return m_data.num;
  }

  const char *stringData() const {
    assert(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->data();
  }

  size_t stringLength() const {
    assert(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->size();
  }

  StringData* rawStringData() const {
    assert(is(KindOfString) || is(KindOfStaticString));
    return m_data.str;
  }

  size_t arrSize() const {
    assert(is(KindOfArray));
    if (isPacked()) return m_data.packed->size();
    return m_data.array->size();
  }

  size_t arrCap() const {
    assert(is(KindOfArray));
    if (isPacked()) return m_data.packed->size();
    return m_data.array->capacity();
  }

  int getIndex(int64_t key);
  int getIndex(const StringData* key);

  ArrayData* loadElems(const SharedArray&);

  Variant getKey(ssize_t pos) const;

  SharedVariant* getValue(ssize_t pos) const;

  void dump(std::string &out);

  void getStats(SharedVariantStats *stats) const;
  int32_t getSpaceUsage() const;

  StringData *getStringData() const {
    assert(is(KindOfString) || is(KindOfStaticString));
    return m_data.str;
  }

  SharedVariant *convertObj(CVarRef var);
  bool isUnserializedObj() { return getIsObj(); }
  bool shouldCache() const { return m_shouldCache; }

  int countReachable() const;

private:

  /*
   * Keep the object layout binary compatible with Variant for primitive types.
   * For non-refcounted types, m_shouldCache and m_flags are guaranteed to be 0,
   * and other parts of runtime will not touch the count.
   *
   * Note that this is assuming a little-endian system: m_shouldCache and
   * m_flags have to overlay the higher-order bits of TypedValue::m_type.
   */

  union SharedData {
    int64_t num;
    double dbl;
    StringData *str;
    ImmutableArray* array;
    ImmutablePackedArray* packed;
    ImmutableObj* obj;
  };

#if PACKED_TV
  bool m_shouldCache;
  DataType m_type;
  uint8_t m_flags;
  std::atomic<uint32_t> m_count;
  SharedData m_data;
#else
  SharedData m_data;
  DataType m_type;
  bool m_shouldCache;
  uint8_t m_flags;
  std::atomic<uint32_t> m_count;
#endif

  static void compileTimeAssertions() {
    static_assert(offsetof(SharedVariant, m_data) == offsetof(TypedValue, m_data),
                  "Offset of m_data must be equal in SharedVar and TypedValue");
    static_assert(offsetof(SharedVariant, m_count) == TypedValueAux::auxOffset,
                  "Offset of m_count must equal offset of TV.m_aux");
    static_assert(offsetof(SharedVariant, m_type) == offsetof(TypedValue, m_type),
                  "Offset of m_type must be equal in SharedVar and TypedValue");
    static_assert(sizeof(SharedVariant) == sizeof(TypedValue),
                  "Be careful with field layout");
  }

  const static uint8_t SerializedArray = (1<<0);
  const static uint8_t IsPacked = (1<<1);
  const static uint8_t IsObj = (1<<2);
  const static uint8_t ObjAttempted = (1<<3);

  bool getSerializedArray() const { return (bool)(m_flags & SerializedArray); }
  void setSerializedArray() { m_flags |= SerializedArray; }

  bool isPacked() const { return (bool)(m_flags & IsPacked); }
  void setPacked() { m_flags |= IsPacked; }

  bool getIsObj() const { return (bool)(m_flags & IsObj); }
  void setIsObj() { m_flags |= IsObj; }

  bool getObjAttempted() const { return (bool)(m_flags & ObjAttempted); }
  void setObjAttempted() { m_flags |= ObjAttempted; }
};

class SharedVariantStats {
 public:
  int32_t dataSize;
  int32_t dataTotalSize;
  int32_t variantCount;

  void initStats() {
    variantCount = 0;
    dataSize = 0;
    dataTotalSize = 0;
  }

  SharedVariantStats() {
    initStats();
  }

  void addChildStats(const SharedVariantStats *childStats) {
    dataSize += childStats->dataSize;
    dataTotalSize += childStats->dataTotalSize;
    variantCount += childStats->variantCount;
  }

  void removeChildStats(const SharedVariantStats *childStats) {
    dataSize -= childStats->dataSize;
    dataTotalSize -= childStats->dataTotalSize;
    variantCount -= childStats->variantCount;
  }
};

inline ImmutablePackedArray::~ImmutablePackedArray() {
  SharedVariant** v = vals();
  for (size_t i = 0, n = m_size; i < n; i++) {
    v[i]->decRef();
  }
}

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_SHARED_VARIANT_H_ */
