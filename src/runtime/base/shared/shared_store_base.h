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
#include <runtime/base/complex_types.h>

#define SHARED_STORE_APPLICATION_CACHE 0
#define SHARED_STORE_DNS_CACHE 1
#define MAX_SHARED_STORE 2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StoreValue {
public:
  StoreValue() : var(NULL), expiry(0) {}
  void set(SharedVariant *v, int64 ttl);
  bool expired() const;
  SharedVariant *var;
  int64 expiry;
  int32 size;
};

class SharedStore {
public:
  SharedStore(int id);
  virtual ~SharedStore();

  virtual void clear() = 0;

  virtual int size() = 0;
  virtual void count(int &reachable, int &expired, int &persistent) = 0;

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
  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) = 0;
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) = 0;

  struct KeyValuePair {
    litstr key;
    int len;
    SharedVariant *value;
    int32 size;
  };
  virtual void prime(const std::vector<KeyValuePair> &vars) = 0;

  virtual std::string reportStats(int &reachable, int indent);
  virtual bool check() { return true; }
  static size_t s_lockCount;
  static std::string GetSkeleton(CStrRef key);

  // debug support
  virtual void dump(std::ostream & out) { /* Default does nothing*/ }

protected:
  int m_id;

  virtual bool eraseImpl(CStrRef key, bool expired) = 0;
  virtual SharedVariant* construct(CStrRef key, CVarRef v) = 0;
  virtual SharedVariant* putVar(SharedVariant* v) const { return v; };
  virtual SharedVariant* getVar(SharedVariant* v) const { return v; };
};

///////////////////////////////////////////////////////////////////////////////

class SharedStores {
public:
  static void Create();
  static std::string ReportStats(int indent);

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

  std::string reportStats(int indent);

private:
  SharedStore* m_stores[MAX_SHARED_STORE];
};

extern SharedStores s_apc_store;

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_STORE_BASE_H__ */
