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

#ifndef __HPHP_SHARED_VARIANT_H__
#define __HPHP_SHARED_VARIANT_H__

#include <runtime/base/types.h>
#include <util/shared_memory_allocator.h>
#include <runtime/base/memory/unsafe_pointer.h>
#include <runtime/base/memory/leak_detectable.h>
#include <util/mutex.h>
#include <util/hash.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedMap;

class SharedVariantStats;

class SharedVariant
#ifdef DEBUG_APC_LEAK
  : public LeakDetectable
#endif
{
public:
  SharedVariant() : m_ref(1), m_flags(0) {}
  virtual ~SharedVariant() {}

  bool is(DataType d) const {
    return m_type == d;
  }

  /**
   * Reference counting. Needs to release memory when count == 0 in decRef().
   */
  virtual void incRef() = 0;
  virtual void decRef() = 0;

  virtual Variant toLocal() = 0;
  virtual bool operator<(const SharedVariant& other) const { return false; }

  virtual int64 intData() const = 0;

  virtual const char* stringData() const = 0;
  virtual size_t stringLength() const = 0;
  virtual int64 stringHash() const {
    return hash_string(stringData(), stringLength());
  }

  virtual size_t arrSize() const = 0;

  virtual int getIndex(CVarRef key) = 0;
  virtual int getIndex(CStrRef key) = 0;
  virtual int getIndex(litstr key) = 0;
  virtual int getIndex(int64 key) = 0;

  virtual void loadElems(ArrayData *&elems, const SharedMap &sharedMap,
                         bool keepRef = false) = 0;

  /** Returns a key in thread-local space. */
  virtual Variant getKey(ssize_t pos) const = 0;
  virtual SharedVariant* getValue(ssize_t pos) const = 0;

  int countReachable();

  // recursively get stats from the SharedVariant
  virtual void getStats(SharedVariantStats *stat) = 0;
  virtual int32 getSpaceUsage() { return 0; }

  // whether it is an object, or an array that recursively contains an object
  // or an array with circular reference
  bool shouldCache() { return getShouldCache(); }

  virtual SharedVariant *convertObj(CVarRef var) { return NULL; }
  virtual bool isUnserializedObj() { return false; }

 protected:
  const static uint16 SerializedArray = (1<<15);
  const static uint16 ShouldCache = (1<<14);
  int m_ref;
  uint16 m_flags;
  uint8 m_type;

  bool getSerializedArray() const { return (bool)(m_flags & SerializedArray);}
  void setSerializedArray() { m_flags |= SerializedArray;}
  void clearSerializedArray() { m_flags &= ~SerializedArray;}

  bool getShouldCache() const { return (bool)(m_flags & ShouldCache);}
  void setShouldCache() { m_flags |= ShouldCache;}
  void clearShouldCache() { m_flags &= ~ShouldCache;}

  // only for countReachable() return NULL if it is vector and key is not
  // SharedVariant
  virtual SharedVariant* getKeySV(ssize_t pos) const = 0;
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
