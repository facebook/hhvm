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

#ifndef __HPHP_SHARED_VARIANT_H__
#define __HPHP_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/lock.h>
#include <util/hash.h>
#include <util/atomic.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/shared/immutable_map.h>
#include <runtime/base/shared/immutable_obj.h>

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

class SharedMap;

class SharedVariantStats;

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
    ASSERT(m_shouldCache == false);
    ASSERT(m_flags == 0);
    ASSERT(!IS_REFCOUNTED_TYPE(m_tv.m_type));
    return tvAsCVarRef(&m_tv);
  }

  void incRef() {
    atomic_inc(m_count);
  }

  void decRef() {
    ASSERT(m_count);
    if (atomic_dec(m_count) == 0) {
      delete this;
    }
  }

  Variant toLocal();

  int64 intData() const {
    ASSERT(is(KindOfInt64));
    return m_data.num;
  }

  const char *stringData() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->data();
  }

  size_t stringLength() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->size();
  }

  int64 stringHash() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->hash();
  }

  size_t arrSize() const;
  int getIndex(CVarRef key);
  int getIndex(CStrRef key);
  int getIndex(litstr key);
  int getIndex(int64 key);

  void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                         bool keepRef = false);

  Variant getKey(ssize_t pos) const;

  SharedVariant* getValue(ssize_t pos) const;

  // implementing LeakDetectable
  void dump(std::string &out);

  void getStats(SharedVariantStats *stats);
  int32 getSpaceUsage();

  StringData *getStringData() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str;
  }

  SharedVariant *convertObj(CVarRef var);
  bool isUnserializedObj() { return getIsObj(); }
  bool shouldCache() const { return m_shouldCache; }

  int countReachable() const;

private:
  class VectorData {
  public:
    size_t size;
    SharedVariant **vals;

    VectorData(size_t s) : size(s) {
      vals = new SharedVariant *[s];
    }

    ~VectorData() {
      for (size_t i = 0; i < size; i++) {
        vals[i]->decRef();
      }
      delete [] vals;
    }
  };

  /* This macro is to help making the object layout binary compatible with
   * Variant for primitive types. We want to have compile time assertion to
   * guard it but still want to have anonymous struct. For non-refcounted
   * types, m_shouldCache and m_flags are guaranteed to be 0, and other parts
   * of runtime code will not touch the count.*/
#ifdef WORDS_BIGENDIAN
 #define SharedVarData \
  union {\
    int64 num;\
    double dbl;\
    StringData *str;\
    ImmutableMap* map;\
    VectorData* vec;\
    ImmutableObj* obj;\
  } m_data;\
  int m_count;\
  bool m_shouldCache;\
  uint8 m_flags;\
  uint16 m_type

#else
 #define SharedVarData \
  union {\
    int64 num;\
    double dbl;\
    StringData *str;\
    ImmutableMap* map;\
    VectorData* vec;\
    ImmutableObj* obj;\
  } m_data;\
  int m_count;\
  uint16 m_type;\
  bool m_shouldCache;\
  uint8 m_flags

#endif

  struct SharedVar {
    SharedVarData;
  };

  union {
    struct {
      SharedVarData;
    };
    TypedValue m_tv;
  };
#undef SharedVarData

  const static uint8 SerializedArray = (1<<0);
  const static uint8 IsVector = (1<<1);
  const static uint8 IsObj = (1<<2);
  const static uint8 ObjAttempted = (1<<3);

  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(SharedVar, m_data) == offsetof(TypedValue, m_data));
    CT_ASSERT(offsetof(SharedVar, m_count) == offsetof(TypedValue, _count));
    CT_ASSERT(offsetof(SharedVar, m_type) == offsetof(TypedValue, m_type));
  }

  bool getSerializedArray() const { return (bool)(m_flags & SerializedArray);}
  void setSerializedArray() { m_flags |= SerializedArray;}
  void clearSerializedArray() { m_flags &= ~SerializedArray;}

  bool getIsVector() const { return (bool)(m_flags & IsVector);}
  void setIsVector() { m_flags |= IsVector;}
  void clearIsVector() { m_flags &= ~IsVector;}

  bool getIsObj() const { return (bool)(m_flags & IsObj);}
  void setIsObj() { m_flags |= IsObj;}
  void clearIsObj() { m_flags &= ~IsObj;}

  bool getObjAttempted() const { return (bool)(m_flags & ObjAttempted);}
  void setObjAttempted() { m_flags |= ObjAttempted;}
  void clearObjAttempted() { m_flags &= ~ObjAttempted;}
};

class SharedVariantStats {
 public:
  int64 dataSize;
  int64 dataTotalSize;
  int32 variantCount;

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

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_VARIANT_H__ */
