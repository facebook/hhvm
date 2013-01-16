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

#ifndef incl_INSTRUMENTATION_H_
#define incl_INSTRUMENTATION_H_

#include <runtime/vm/core_types.h>
#include <runtime/vm/unit.h>
#include <tbb/concurrent_hash_map.h>
#include <util/lock.h>

namespace HPHP {
namespace VM {

// Define a set of hooks to be used
enum InstHookTypeInt64 {
  InstHookTypeBCPC,       // bytecode pc
  InstHookTypeInt64Count
};

enum InstHookTypeSD {
  InstHookTypeCustomEvt,  // custom event by name
  InstHookTypeFuncEntry,  // function entry
  InstHookTypeSDCount
};

class Injection {
public:
  Injection(Unit* unit, const StringData* desc)
    : m_builtin(false), m_unit(unit), m_arg(NULL), m_desc(desc) { }
  typedef void (*Callback)(void*);
  Injection(Callback callback, void* arg, const StringData* desc)
    : m_builtin(true), m_callback(callback), m_arg(arg), m_desc(desc) { }
  Injection(const Injection& inj)
    : m_builtin(inj.m_builtin), m_unit(inj.m_unit), m_arg(inj.m_arg),
      m_desc(inj.m_desc) {}

  bool m_builtin;
  union {
    Unit* m_unit;
    Callback m_callback;
  };
  void *m_arg;
  const StringData* m_desc;

  void execute() const;
};

struct InjectionHashCompare {
  bool equal(const Injection *i1, const Injection *i2) const {
    assert(i1 && i2);
    return memcmp(i1, i2, sizeof(Injection)) == 0;
  }
  size_t hash(const Injection *i) const {
    assert(i);
    return i->m_desc->hash() ^ hash_int64((int64)i->m_unit);
  }
};

class InjectionCacheHolder;

class InjectionCache {
public:
  static const Injection* GetInjection(const StringData* code,
                                       const StringData* desc);
  static const Injection* GetInjection(const std::string& code,
                                       const std::string& desc);
  static const Injection* GetInjection(Injection::Callback callback, void *arg,
                                       const StringData* desc);
  static const StringData* GetStringData(const StringData* sd);
  // ClearCache should be called with caution since it destroys code units.
  static void ClearCache();

private:
  InjectionCache() {}
  ~InjectionCache() { clearCacheImpl(); }

  typedef tbb::concurrent_hash_map<const StringData*, void*,
                                   StringDataHashCompare>  StringDataMap;
  const StringData* getStringData(const StringData* sd);
  StringDataMap m_sdCache;

  typedef tbb::concurrent_hash_map<const StringData*, Unit*,
                                   StringDataHashCompare> UnitMap;
  Unit* getUnit(const StringData* code);
  UnitMap m_unitCache;

  typedef tbb::concurrent_hash_map<const Injection*, void*,
                                   InjectionHashCompare> InjectionMap;
  const Injection* getInjection(const Injection* inj);
  InjectionMap m_injectionCache;

  ReadWriteMutex m_lock;

  void clearCacheImpl();
  const Injection* getInjectionImpl(const StringData* code,
                                    const StringData* desc);
  const Injection* getInjectionImpl(const std::string& code,
                                    const std::string& desc);
  const Injection* getInjectionImpl(Injection::Callback callback, void *arg,
                                    const StringData* desc);
  friend class InjectionCacheHolder;
};

typedef hphp_hash_map<int64, const Injection*, int64_hash> InjectionTableInt64;
typedef hphp_hash_map<const StringData*, const Injection*, string_data_hash,
                      string_data_same>  InjectionTableSD;

class InjectionTables {
public:
  InjectionTables();
  ~InjectionTables();

  void clear();
  InjectionTables* clone();

  InjectionTableInt64* getInt64Table(int hookType) {
    assert(hookType < InstHookTypeInt64Count);
    assert(hookType < (int)m_int64Tables.size());
    return m_int64Tables[hookType];
  }
  InjectionTableSD* getSDTable(int hookType) {
    assert(hookType < InstHookTypeSDCount);
    assert(hookType < (int)m_sdTables.size());
    return m_sdTables[hookType];
  }

  void setInt64Table(int hookType, InjectionTableInt64* table);
  void setSDTable(int hookType, InjectionTableSD* table);

  int countInjections();

private:
  std::vector<InjectionTableInt64*> m_int64Tables;
  std::vector<InjectionTableSD*> m_sdTables;
};

class InstHelpers {
public:
  static void InstCustomStringCallback(const StringData* hook,
                                       Injection::Callback callback,
                                       void *arg, const StringData* desc);

  static void PushInstToGlobal();
  static void PullInstFromGlobal();

  static int CountGlobalInst();
  static void ClearGlobalInst();
};

///////////////////////////////////////////////////////////////////////////////

} }    // HPHP::VM

#endif /* incl_INSTRUMENTATION_H_ */
