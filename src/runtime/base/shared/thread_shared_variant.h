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

  virtual void incRef() {
    atomic_inc(m_ref);
  }

  virtual void decRef() {
    ASSERT(m_ref);
    if (atomic_dec(m_ref) == 0) {
      delete this;
    }
  }

  Variant toLocal();

  virtual int64 intData() const {
    ASSERT(is(KindOfInt64));
    return m_data.num;
  }

  const char* stringData() const;
  size_t stringLength() const;
  virtual int64 stringHash() const {
    ASSERT(is(KindOfString));
    return m_data.str->hash();
  }

  size_t arrSize() const;
  int getIndex(CVarRef key);
  int getIndex(CStrRef key);
  int getIndex(litstr key);
  int getIndex(int64 key);

  void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                 bool keepRef = false);

  virtual Variant getKey(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (getIsVector()) {
      ASSERT(pos < (ssize_t) m_data.vec->size);
      return pos;
    }
    return m_data.map->getKeyIndex(pos)->toLocal();
  }

  virtual SharedVariant* getValue(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (getIsVector()) {
      ASSERT(pos < (ssize_t) m_data.vec->size);
      return m_data.vec->vals[pos];
    }
    return m_data.map->getValIndex(pos);
  }

  // implementing LeakDetectable
  virtual void dump(std::string &out);

  virtual void getStats(SharedVariantStats *stats);
  virtual int32 getSpaceUsage();

  StringData *getStringData() const {
    ASSERT(is(KindOfString));
    return m_data.str;
  }

  virtual SharedVariant *convertObj(CVarRef var);
  virtual bool isUnserializedObj() { return getIsObj(); }

protected:
  virtual SharedVariant* getKeySV(ssize_t pos) const {
    ASSERT(is(KindOfArray));
    if (getIsVector()) return NULL;
    else return m_data.map->getKeyIndex(pos);
  }

private:
  const static uint16 IsVector = (1<<13);
  const static uint16 Owner = (1<<12);
  const static uint16 IsObj = (1<<11);
  const static uint16 ObjAttempted = (1<<10);

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
        ASSERT(key->is(KindOfString));
        if (!strMap) strMap = new StringDataToIntMap(size);
        (*strMap)[key->m_data.str] = p;
      }
    }
  };

  union {
    int64 num;
    double dbl;
    StringData *str;
    ImmutableMap* map;
    VectorData* vec;
    MapData *gnuMap;
    ImmutableObj* obj;
  } m_data;

  bool getIsVector() const { return (bool)(m_flags & IsVector);}
  void setIsVector() { m_flags |= IsVector;}
  void clearIsVector() { m_flags &= ~IsVector;}

  bool getOwner() const { return (bool)(m_flags & Owner);}
  void setOwner() { m_flags |= Owner;}
  void clearOwner() { m_flags &= ~Owner;}

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
