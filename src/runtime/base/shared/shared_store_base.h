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

#ifndef __HPHP_SHARED_STORE_BASE_H__
#define __HPHP_SHARED_STORE_BASE_H__

#include <runtime/base/types.h>
#include <runtime/base/shared/shared_variant.h>
#include <util/lock.h>
#include <util/smalllocks.h>
#include <runtime/base/complex_types.h>

#define SHARED_STORE_APPLICATION_CACHE 0
#define SHARED_STORE_DNS_CACHE 1
#define MAX_SHARED_STORE 2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StoreValue {
public:
  StoreValue() : var(NULL), sAddr(NULL), expiry(0), size(0), sSize(0) {}
  StoreValue(const StoreValue& v) : var(v.var), sAddr(v.sAddr),
                                    expiry(v.expiry), size(v.size),
                                    sSize(v.sSize) {}
  void set(SharedVariant *v, int64 ttl);
  bool expired() const;

  // Mutable fields here are so that we can deserialize the object from disk
  // while holding a const pointer to the StoreValue. Mostly a hacky workaround
  // for how we use TBB
  mutable SharedVariant *var;
  char *sAddr; // For file storage
  int64 expiry;
  mutable int32 size;
  int32 sSize; // For file storage, negative means serailized object
  mutable SmallLock lock;

  bool inMem() const {
    return var != NULL;
  }
  bool inFile() const {
    return sAddr != NULL;
  }

  int32 getSerializedSize() const {
    return abs(sSize);
  }
  bool isSerializedObj() const {
    return sSize < 0;
  }
};

class SharedStore {
public:
  SharedStore(int id);
  virtual ~SharedStore();

  virtual bool clear() = 0;

  virtual int size() = 0;

  virtual bool get(CStrRef key, Variant &value) = 0;
  virtual bool store(CStrRef key, CVarRef val, int64 ttl,
                     bool overwrite = true) = 0;
  bool erase(CStrRef key, bool expired = false);
  virtual int64 inc(CStrRef key, int64 step, bool &found) = 0;
  virtual bool cas(CStrRef key, int64 old, int64 val) = 0;
  virtual bool exists(CStrRef key) {
    // Default implementation does a copy
    Variant tmp;
    return get(key, tmp);
  }

  // for priming only
  struct KeyValuePair {
    KeyValuePair() : value(NULL), sAddr(NULL) {}
    litstr key;
    int len;
    SharedVariant *value;
    char *sAddr;
    int32 sSize;

    bool inMem() const {
      return value != NULL;
    }
  };
  virtual void prime(const std::vector<KeyValuePair> &vars) = 0;
  virtual bool constructPrime(CStrRef v, KeyValuePair& item,
                              bool serialized) = 0;
  virtual bool constructPrime(CVarRef v, KeyValuePair& item) = 0;
  virtual void primeDone() {}

  virtual bool check() { return true; }
  static size_t s_lockCount;
  static std::string GetSkeleton(CStrRef key);

  // debug support
  virtual void dump(std::ostream & out, bool keyOnly, int waitSeconds) {
    /* Default does nothing*/
  }

protected:
  int m_id;

  virtual bool eraseImpl(CStrRef key, bool expired) = 0;
  virtual SharedVariant* construct(CVarRef v) = 0;
  virtual SharedVariant* putVar(SharedVariant* v) const { return v; };
  virtual SharedVariant* getVar(SharedVariant* v) const { return v; };
};

///////////////////////////////////////////////////////////////////////////////

class SharedStores {
public:
  static void Create();

public:
  SharedStores();
  ~SharedStores();
  void create();
  void clear();
  void reset();

  SharedStore& operator[](int id) {
    ASSERT(id >= 0 && id < MAX_SHARED_STORE);
    return *m_stores[id];
  }

private:
  SharedStore* m_stores[MAX_SHARED_STORE];
};

extern SharedStores s_apc_store;

///////////////////////////////////////////////////////////////////////////////

class SharedStoreFileStorage {
public:
  enum StorageState {
    StateInvalid,
    StateOpen,
    StateSealed,
    StateFull
  };

  SharedStoreFileStorage()
  : m_state(StateInvalid), m_current(NULL), m_chunkRemain(0) {}
  void enable(const std::string& prefix, int64 chunkSize, int64 maxSize);
  char *put(const char *data, int32 len);
  void seal();
  void adviseOut();
  bool hashCheck();
  void cleanup();
  StorageState getState() { return m_state; }

private:
  bool addFile();

private:
  std::vector<void*> m_chunks;
  std::string m_prefix;
  int64 m_chunkSize;
  int64 m_maxSize;
  StorageState m_state;
  char *m_current;
  int32 m_chunkRemain;
  std::vector<std::string> m_fileNames;

  Mutex m_lock;
  static const strhash_t TombHash = 0xdeadbeef;
  static const int PaddingSize = sizeof(strhash_t) + // hash
                                 sizeof(int32) + // len
                                 sizeof(char); // '\0'
};

extern SharedStoreFileStorage s_apc_file_storage;

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_STORE_BASE_H__ */
