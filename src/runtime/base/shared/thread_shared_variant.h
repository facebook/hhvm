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

#ifndef __HPHP_THREAD_SHARED_VARIANT_H__
#define __HPHP_THREAD_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/lock.h>
#include <util/hash.h>
#include <util/atomic.h>
#include <runtime/base/shared/shared_variant.h>
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

class ThreadSharedVariant;

typedef hphp_hash_map<int64, int, int64_hash> Int64ToIntMap;
typedef hphp_hash_map<StringData *, int, string_data_hash, string_data_same>
        StringDataToIntMap;

///////////////////////////////////////////////////////////////////////////////

class ThreadSharedVariant : public SharedVariant {
public:
  ThreadSharedVariant(CVarRef source, bool serialized, bool inner = false,
                      bool unserializeObj = false);
  virtual ~ThreadSharedVariant();

  // Create will do the wrapped check before creating a ThreadSharedVariant
  static ThreadSharedVariant* Create(CVarRef source, bool serialized,
                                     bool inner = false,
                                     bool unserializeObj = false);

  bool is(DataType d) const { return m_type == d; }
  virtual DataType getType() const { return (DataType)m_type; }
  virtual CVarRef asCVarRef() const {
    // Must be non-refcounted types
    ASSERT(m_shouldCache == false);
    ASSERT(m_flags == 0);
    ASSERT(!IS_REFCOUNTED_TYPE(m_tv.m_type));
    return tvAsCVarRef(&m_tv);
  }

  virtual void incRef() {
    atomic_inc(m_count);
  }

  virtual void decRef() {
    ASSERT(m_count);
    if (atomic_dec(m_count) == 0) {
      delete this;
    }
  }

  virtual Variant toLocal();

  int64 intData() const {
    ASSERT(is(KindOfInt64));
    return m_data.num;
  }

  virtual const char* stringData() const;
  virtual size_t stringLength() const;
  virtual int64 stringHash() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str->hash();
  }

  virtual size_t arrSize() const;
  virtual int getIndex(CVarRef key);
  virtual int getIndex(CStrRef key);
  virtual int getIndex(litstr key);
  virtual int getIndex(int64 key);

  virtual void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                         bool keepRef = false);

  virtual Variant getKey(ssize_t pos) const;

  virtual SharedVariant* getValue(ssize_t pos) const;

  // implementing LeakDetectable
  virtual void dump(std::string &out);

  virtual void getStats(SharedVariantStats *stats);
  virtual int32 getSpaceUsage();

  StringData *getStringData() const {
    ASSERT(is(KindOfString) || is(KindOfStaticString));
    return m_data.str;
  }

  virtual SharedVariant *convertObj(CVarRef var);
  virtual bool isUnserializedObj() { return getIsObj(); }

  virtual bool shouldCache() const { return m_shouldCache; }

protected:
  virtual SharedVariant* getKeySV(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (getIsVector()) return NULL;
    else return m_data.map->getKeyIndex(pos);
  }

private:
  class VectorData {
  public:
    size_t size;
    ThreadSharedVariant **vals;

    VectorData(size_t s) : size(s) {
      vals = new ThreadSharedVariant *[s];
    }

    ~VectorData() {
      for (size_t i = 0; i < size; i++) {
        vals[i]->decRef();
      }
      delete [] vals;
    }
  };

  class MapData {
  public:
    size_t size;
    Int64ToIntMap *intMap;
    StringDataToIntMap *strMap;
    ThreadSharedVariant **keys;
    ThreadSharedVariant **vals;

    MapData(size_t s) : size(s), intMap(NULL), strMap(NULL) {
      keys = new ThreadSharedVariant *[s];
      vals = new ThreadSharedVariant *[s];
    }
    ~MapData() {
      for (size_t i = 0; i < size; i++) {
        keys[i]->decRef();
        vals[i]->decRef();
      }
      if (intMap) delete intMap;
      if (strMap) delete strMap;
      delete [] keys;
      delete [] vals;
    }
    void set(int p, ThreadSharedVariant *key, ThreadSharedVariant *val) {
      keys[p] = key;
      vals[p] = val;
      if (key->is(KindOfInt64)) {
        if (!intMap) intMap = new Int64ToIntMap(size);
        (*intMap)[key->m_data.num] = p;
      } else {
        ASSERT(key->is(KindOfString) || key->is(KindOfStaticString));
        if (!strMap) strMap = new StringDataToIntMap(size);
        (*strMap)[key->m_data.str] = p;
      }
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
    MapData *gnuMap;\
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
    MapData *gnuMap;\
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

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_THREAD_SHARED_VARIANT_H__ */
