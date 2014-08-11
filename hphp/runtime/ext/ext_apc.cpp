/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/ext_apc.h"

#include <fstream>

#include <dlfcn.h>
#include <sys/time.h> // gettimeofday
#include <limits>
#include <memory>
#include <set>
#include <vector>
#include <stdexcept>

#include "hphp/runtime/ext/ext_fb.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/async-job.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/util/alloc.h"
#include "hphp/util/hdf.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/config.h"

using HPHP::ScopedMem;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern void const_load();

typedef ConcurrentTableSharedStore::KeyValuePair KeyValuePair;
typedef ConcurrentTableSharedStore::DumpMode DumpMode;

void apcExtension::moduleLoad(const IniSetting::Map& ini, Hdf config) {
  Hdf apc = config["Server"]["APC"];

  Enable = Config::GetBool(ini, apc["EnableApc"], true);
  EnableConstLoad = Config::GetBool(ini, apc["EnableConstLoad"], false);
  ForceConstLoadToAPC = Config::GetBool(ini, apc["ForceConstLoadToAPC"], true);
  PrimeLibrary = Config::GetString(ini, apc["PrimeLibrary"]);
  LoadThread = Config::GetInt16(ini, apc["LoadThread"], 2);
  Config::Get(ini, apc["CompletionKeys"], CompletionKeys);
  std::string tblType = Config::GetString(ini, apc["TableType"], "concurrent");
  if (strcasecmp(tblType.c_str(), "concurrent") == 0) {
    TableType = TableTypes::ConcurrentTable;
  } else {
    throw std::runtime_error("invalid apc table type");
  }
  EnableApcSerialize = Config::GetBool(ini, apc["EnableApcSerialize"], true);
  ExpireOnSets = Config::GetBool(ini, apc["ExpireOnSets"]);
  PurgeFrequency = Config::GetInt32(ini, apc["PurgeFrequency"], 4096);
  PurgeRate = Config::GetInt32(ini, apc["PurgeRate"], -1);

  AllowObj = Config::GetBool(ini, apc["AllowObject"]);
  TTLLimit = Config::GetInt32(ini, apc["TTLLimit"], -1);

  Hdf fileStorage = apc["FileStorage"];
  UseFileStorage = Config::GetBool(ini, fileStorage["Enable"]);
  FileStorageChunkSize = Config::GetInt64(ini, fileStorage["ChunkSize"],
                                          1LL << 29);
  FileStorageMaxSize = Config::GetInt64(ini, fileStorage["MaxSize"], 1LL << 32);
  FileStoragePrefix = Config::GetString(ini, fileStorage["Prefix"],
                                        "/tmp/apc_store");
  FileStorageFlagKey = Config::GetString(ini, fileStorage["FlagKey"],
                                         "_madvise_out");
  FileStorageAdviseOutPeriod =
    Config::GetInt32(ini, fileStorage["AdviseOutPeriod"], 1800);
  FileStorageKeepFileLinked =
    Config::GetBool(ini, fileStorage["KeepFileLinked"]);

  ConcurrentTableLockFree =
    Config::GetBool(ini, apc["ConcurrentTableLockFree"], false);
  KeyMaturityThreshold = Config::GetInt32(ini, apc["KeyMaturityThreshold"], 20);
  MaximumCapacity = Config::GetInt64(ini, apc["MaximumCapacity"], 0);
  KeyFrequencyUpdatePeriod =
    Config::GetInt32(ini, apc["KeyFrequencyUpdatePeriod"], 1000);

  Config::Get(ini, apc["NoTTLPrefix"], NoTTLPrefix);

  UseUncounted = Config::GetBool(ini, apc["MemModelTreadmill"],
                                 RuntimeOption::ServerExecutionMode());

  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.enabled", &Enable);
  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.stat",
                   RuntimeOption::RepoAuthoritative ? "0" : "1", &Stat);
  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.enable_cli",
                   &EnableCLI);
}

void apcExtension::moduleInit() {
  if (UseFileStorage) {
    s_apc_file_storage.enable(FileStoragePrefix,
                              FileStorageChunkSize,
                              FileStorageMaxSize);
  }
}

void apcExtension::moduleShutdown() {
  if (UseFileStorage) {
    s_apc_file_storage.cleanup();
  }
}


bool apcExtension::Enable = true;
bool apcExtension::EnableConstLoad = false;
bool apcExtension::ForceConstLoadToAPC = true;
std::string apcExtension::PrimeLibrary;
int apcExtension::LoadThread = 1;
std::set<std::string> apcExtension::CompletionKeys;
apcExtension::TableTypes apcExtension::TableType =
  TableTypes::ConcurrentTable;
bool apcExtension::EnableApcSerialize = true;
time_t apcExtension::KeyMaturityThreshold = 20;
size_t apcExtension::MaximumCapacity = 0;
int apcExtension::KeyFrequencyUpdatePeriod = 1000;
bool apcExtension::ExpireOnSets = false;
int apcExtension::PurgeFrequency = 4096;
int apcExtension::PurgeRate = -1;
bool apcExtension::AllowObj = false;
int apcExtension::TTLLimit = -1;
bool apcExtension::UseFileStorage = false;
int64_t apcExtension::FileStorageChunkSize = int64_t(1LL << 29);
int64_t apcExtension::FileStorageMaxSize = int64_t(1LL << 32);
std::string apcExtension::FileStoragePrefix = "/tmp/apc_store";
int apcExtension::FileStorageAdviseOutPeriod = 1800;
std::string apcExtension::FileStorageFlagKey = "_madvise_out";
bool apcExtension::ConcurrentTableLockFree = false;
bool apcExtension::FileStorageKeepFileLinked = false;
std::vector<std::string> apcExtension::NoTTLPrefix;
bool apcExtension::UseUncounted = false;
bool apcExtension::Stat = true;
// Different from zend default but matches what we've been returning for years
bool apcExtension::EnableCLI = true;

static apcExtension s_apc_extension;

///////////////////////////////////////////////////////////////////////////////
Variant f_apc_store(const Variant& key_or_array,
                    const Variant& var /* = null_variant */,
                    int64_t ttl /* = 0 */, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  if (key_or_array.is(KindOfArray)) {
    Array valuesArr = key_or_array.toArray();

    // errors stores all keys corresponding to entries that could not be cached
    ArrayInit errors(valuesArr.size(), ArrayInit::Map{});

    for (ArrayIter iter(valuesArr); iter; ++iter) {
      Variant key = iter.first();
      if (!key.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      Variant v = iter.second();
      if (!(s_apc_store[cache_id].store(key.toString(), v, ttl))) {
        errors.add(key, -1);
      }
    }
    return errors.toVariant();
  }

  if (!key_or_array.isString()) {
    throw_invalid_argument("apc key: (not a string)");
    return false;
  }
  String strKey = key_or_array.toString();
  return s_apc_store[cache_id].store(strKey, var, ttl);
}

/**
 * Stores the key in a similar fashion as "priming" would do (no TTL limit).
 * Using this function is equivalent to adding your key to apc_prime.so.
 */
bool f_apc_store_as_primed_do_not_use(const String& key, const Variant& var,
                                      int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  return s_apc_store[cache_id].store(key, var, 0, true, false);
}

Variant f_apc_add(const Variant& key_or_array,
                  const Variant& var /* = null_variant */,
                  int64_t ttl /* = 0 */, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  if (key_or_array.is(KindOfArray)) {
    Array valuesArr = key_or_array.toArray();

    // errors stores all keys corresponding to entries that could not be cached
    ArrayInit errors(valuesArr.size(), ArrayInit::Map{});

    for (ArrayIter iter(valuesArr); iter; ++iter) {
      Variant key = iter.first();
      if (!key.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      Variant v = iter.second();
      if (!(s_apc_store[cache_id].store(key.toString(), v, ttl, false))) {
        errors.add(key, -1);
      }
    }
    return errors.create();
  }

  if (!key_or_array.isString()) {
    throw_invalid_argument("apc key: (not a string)");
    return false;
  }
  String strKey = key_or_array.toString();
  return s_apc_store[cache_id].store(strKey, var, ttl, false);
}

Variant f_apc_fetch(const Variant& key, VRefParam success /* = null */,
                    int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  Variant v;

  if (key.is(KindOfArray)) {
    bool tmp = false;
    Array keys = key.toArray();
    ArrayInit init(keys.size(), ArrayInit::Map{});
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      String strKey = k.toString();
      if (s_apc_store[cache_id].get(strKey, v)) {
        tmp = true;
        init.set(strKey, v);
      }
    }
    success = tmp;
    return init.create();
  }

  if (s_apc_store[cache_id].get(key.toString(), v)) {
    success = true;
  } else {
    success = false;
    v = false;
  }
  return v;
}

Variant f_apc_delete(const Variant& key, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  if (key.is(KindOfArray)) {
    Array keys = key.toArray();
    PackedArrayInit init(keys.size());
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        raise_warning("apc key is not a string");
        init.append(k);
      } else if (!s_apc_store[cache_id].erase(k.toString())) {
        init.append(k);
      }
    }
    return init.create();
  }

  return s_apc_store[cache_id].erase(key.toString());
}

bool f_apc_clear_cache(int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }
  return s_apc_store[cache_id].clear();
}

Variant f_apc_inc(const String& key, int64_t step /* = 1 */,
                  VRefParam success /* = null */, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }
  bool found = false;
  int64_t newValue = s_apc_store[cache_id].inc(key, step, found);
  success = found;
  if (!found) return false;
  return newValue;
}

Variant f_apc_dec(const String& key, int64_t step /* = 1 */,
                  VRefParam success /* = null */, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }
  bool found = false;
  int64_t newValue = s_apc_store[cache_id].inc(key, -step, found);
  success = found;
  if (!found) return false;
  return newValue;
}

bool f_apc_cas(const String& key, int64_t old_cas, int64_t new_cas,
               int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }
  return s_apc_store[cache_id].cas(key, old_cas, new_cas);
}

Variant f_apc_exists(const Variant& key, int64_t cache_id /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %" PRId64, cache_id);
    return false;
  }

  if (key.is(KindOfArray)) {
    Array keys = key.toArray();
    PackedArrayInit init(keys.size());
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      String strKey = k.toString();
      if (s_apc_store[cache_id].exists(strKey)) {
        init.append(strKey);
      }
    }
    return init.create();
  }

  return s_apc_store[cache_id].exists(key.toString());
}


const StaticString s_user("user");
const StaticString s_start_time("start_time");
const StaticString s_ttl("ttl");
const StaticString s_cache_list("cache_list");
const StaticString s_entry_name("entry_name");
const StaticString s_in_memory("in_memory");
const StaticString s_mem_size("mem_size");
const StaticString s_type("type");

// This is a guess to the size of the info array. It is significantly
// bigger than what we need but hard to control all the info that we
// may want to add here.
// Try to keep it such that we do not have to resize the array
const uint32_t kCacheInfoSize = 40;
// Number of elements in the entry array
const int32_t kEntryInfoSize = 5;

Variant f_apc_cache_info(const String& cache_type,
                         bool limited /* = false */) {
  ArrayInit info(kCacheInfoSize, ArrayInit::Map{});
  info.add(s_start_time, start_time());
  if (cache_type.size() != 0 && !cache_type.same(s_user)) {
    return info.toArray();
  }

  info.add(s_ttl, apcExtension::TTLLimit);

  std::map<const StringData*, int64_t> stats;
  APCStats::getAPCStats().collectStats(stats);
  for (auto it = stats.begin(); it != stats.end(); it++) {
    info.add(Variant(it->first, Variant::StaticStrInit{}), it->second);
  }
  if (!limited) {
    std::vector<EntryInfo> entries;
    s_apc_store[0].getEntriesInfo(entries);
    PackedArrayInit ents(entries.size());
    for (auto& entry: entries) {
      ArrayInit ent(kEntryInfoSize, ArrayInit::Map{});
      ent.add(s_entry_name, StringData::Make(entry.key));
      ent.add(s_in_memory, entry.inMem);
      ent.add(s_ttl, entry.ttl);
      ent.add(s_mem_size, entry.size);
      ent.add(s_type, static_cast<int64_t>(entry.type));
      ents.append(ent.toArray());
    }
    info.add(s_cache_list, ents.toArray(), false);
  }
  return info.toArray();
}

Array f_apc_sma_info(bool limited /* = false */) {
  return empty_array();
}

///////////////////////////////////////////////////////////////////////////////
// loading APC from archive files

typedef void(*PFUNC_APC_LOAD)();

// Structure to hold cache meta data
// Same definition in ext_apc.cpp
struct cache_info {
  char *a_name;
  bool use_const;
};

static Mutex dl_mutex;
static PFUNC_APC_LOAD apc_load_func(void *handle, const char *name) {
  Lock lock(dl_mutex);
  dlerror(); // clear errors
  PFUNC_APC_LOAD p = (PFUNC_APC_LOAD)dlsym(handle, name);
  const char *error = dlerror();
  if (error || p == NULL) {
    throw Exception("Unable to find %s in %s: %s", name,
                    apcExtension::PrimeLibrary.c_str(),
                    error ? error : "(unknown error)");
  }
  return p;
}

class ApcLoadJob {
public:
  ApcLoadJob(void *handle, int index) : m_handle(handle), m_index(index) {}
  void *m_handle; int m_index;
};

class ApcLoadWorker {
public:
  void onThreadEnter() {}
  void doJob(std::shared_ptr<ApcLoadJob> job) {
    char func_name[128];
    snprintf(func_name, sizeof(func_name), "_apc_load_%d", job->m_index);
    apc_load_func(job->m_handle, func_name)();
  }
  void onThreadExit() {}
};

static size_t s_const_map_size = 0;

EXTERNALLY_VISIBLE
void apc_load(int thread) {
  static void *handle = NULL;
  if (handle ||
      apcExtension::PrimeLibrary.empty() ||
      !apcExtension::Enable) {
    static uint64_t keep_entry_points_around_under_lto;
    if (++keep_entry_points_around_under_lto ==
        std::numeric_limits<uint64_t>::max()) {
      // this had better never happen...

      // Fill out a cache_info to prevent g++ from optimizing out
      // the calls to const_load_impl*
      cache_info info;
      info.a_name = "dummy";
      info.use_const = true;

      const_load();
      const_load_impl(&info, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      const_load_impl_compressed(&info,
                                 NULL, NULL, NULL,
                                 NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL);
      apc_load_impl(&info, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      apc_load_impl_compressed(&info,
                               NULL, NULL, NULL,
                               NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL);
    }
    return;
  }

  Timer timer(Timer::WallTime, "loading APC data");
  handle = dlopen(apcExtension::PrimeLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    throw Exception("Unable to open apc prime library %s: %s",
                    apcExtension::PrimeLibrary.c_str(), dlerror());
  }

  if (thread <= 1) {
    apc_load_func(handle, "_apc_load_all")();
  } else {
    int count = ((int(*)())apc_load_func(handle, "_apc_load_count"))();

    std::vector<std::shared_ptr<ApcLoadJob>> jobs;
    jobs.reserve(count);
    for (int i = 0; i < count; i++) {
      jobs.push_back(std::make_shared<ApcLoadJob>(handle, i));
    }
    JobDispatcher<ApcLoadJob, ApcLoadWorker>(jobs, thread).run();
  }

  s_apc_store[0].primeDone();

  if (apcExtension::EnableConstLoad) {
#ifdef USE_JEMALLOC
    size_t allocated_before = 0;
    size_t allocated_after = 0;
    size_t sz = sizeof(size_t);
    if (mallctl) {
      uint64_t epoch = 1;
      mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));
      mallctl("stats.allocated", &allocated_before, &sz, NULL, 0);
      // Ignore the first result because it may be inaccurate due to internal
      // allocation.
      epoch = 1;
      mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));
      mallctl("stats.allocated", &allocated_before, &sz, NULL, 0);
    }
#endif
    apc_load_func(handle, "_hphp_const_load_all")();
#ifdef USE_JEMALLOC
    if (mallctl) {
      uint64_t epoch = 1;
      mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));
      sz = sizeof(size_t);
      mallctl("stats.allocated", &allocated_after, &sz, NULL, 0);
      s_const_map_size = allocated_after - allocated_before;
    }
#endif
  }

  // We've copied all the data out, so close it out.
  dlclose(handle);
}

size_t get_const_map_size() {
  return s_const_map_size;
}

//define in ext_fb.cpp
extern void const_load_set(const String& key, const Variant& value);

///////////////////////////////////////////////////////////////////////////////
// Constant and APC priming with uncompressed data
// Note (qixin): this is going to be deprecated by the compressed version.

static int count_items(const char **p, int step) {
  int count = 0;
  for (const char **k = p; *k; k += step) {
    count++;
  }
  return count;
}

EXTERNALLY_VISIBLE
void const_load_impl(struct cache_info *info,
                     const char **int_keys, long long *int_values,
                     const char **char_keys, char *char_values,
                     const char **strings, const char **objects,
                     const char **thrifts, const char **others) {
  if (!apcExtension::EnableConstLoad || !info || !info->use_const) return;
  {
    int count = count_items(int_keys, 2);
    if (count) {
      const char **k = int_keys;
      long long* v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        String key(*k, (int)(int64_t)*(k+1), CopyString);
        int64_t value = *v++;
        const_load_set(key, value);
      }
    }
  }
  {
    int count = count_items(char_keys, 2);
    if (count) {
      const char **k = char_keys;
      char *v = char_values;
      for (int i = 0; i < count; i++, k += 2) {
        String key(*k, (int)(int64_t)*(k+1), CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = init_null(); break;
        default:
          throw Exception("bad apc archive, unknown char type");
        }
        const_load_set(key, value);
      }
    }
  }
  {
    int count = count_items(strings, 4);
    if (count) {
      const char **p = strings;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64_t)*(p+1), CopyString);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        const_load_set(key, value);
      }
    }
  }
  // unserialize_from_string object is extremely slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = count_items(objects, 4);
    if (count) {
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64_t)*(p+1), CopyString);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        const_load_set(key, unserialize_from_string(value));
      }
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64_t)*(p+1), CopyString);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        Variant success;
        Variant v = f_fb_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_unserialize failed");
        }
        const_load_set(key, v);
      }
    }
  }
  {//Would we use others[]?
    int count = count_items(others, 4);
    if (count) {
      const char **p = others;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64_t)*(p+1), CopyString);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        Variant v = unserialize_from_string(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, unserialize_from_string failed");
        }
        const_load_set(key, v);
      }
    }
  }
}

EXTERNALLY_VISIBLE
void apc_load_impl(struct cache_info *info,
                   const char **int_keys, long long *int_values,
                   const char **char_keys, char *char_values,
                   const char **strings, const char **objects,
                   const char **thrifts, const char **others) {
  if (!apcExtension::ForceConstLoadToAPC) {
    if (apcExtension::EnableConstLoad && info && info->use_const) return;
  }
  auto& s = s_apc_store[0];
  {
    int count = count_items(int_keys, 2);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **k = int_keys;
      long long*v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        auto& item = vars[i];
        item.key = *k;
        item.len = (int)(int64_t)*(k+1);
        s.constructPrime(*v++, item);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(char_keys, 2);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **k = char_keys;
      char *v = char_values;
      for (int i = 0; i < count; i++, k += 2) {
        auto& item = vars[i];
        item.key = *k;
        item.len = (int)(int64_t)*(k+1);
        switch (*v++) {
        case 0: s.constructPrime(false, item); break;
        case 1: s.constructPrime(true , item); break;
        case 2: s.constructPrime(uninit_null() , item); break;
        default:
          throw Exception("bad apc archive, unknown char type");
        }
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(strings, 4);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **p = strings;
      for (int i = 0; i < count; i++, p += 4) {
        auto& item = vars[i];
        item.key = *p;
        item.len = (int)(int64_t)*(p+1);
        // Strings would be copied into APC anyway.
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        s.constructPrime(value, item, false);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(objects, 4);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        auto& item = vars[i];
        item.key = *p;
        item.len = (int)(int64_t)*(p+1);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        s.constructPrime(value, item, true);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        auto& item = vars[i];
        item.key = *p;
        item.len = (int)(int64_t)*(p+1);
        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        Variant success;
        Variant v = f_fb_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_unserialize failed");
        }
        s.constructPrime(v, item);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(others, 4);
    if (count) {
      std::vector<KeyValuePair> vars(count);
      const char **p = others;
      for (int i = 0; i < count; i++, p += 4) {
        auto& item = vars[i];
        item.key = *p;
        item.len = (int)(int64_t)*(p+1);

        String value(*(p+2), (int)(int64_t)*(p+3), CopyString);
        Variant v = unserialize_from_string(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, unserialize_from_string failed");
        }
        s.constructPrime(v, item);
      }
      s.prime(vars);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Constant and APC priming with compressed data

EXTERNALLY_VISIBLE
void const_load_impl_compressed
    (struct cache_info *info,
     int *int_lens, const char *int_keys, long long *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  if (!apcExtension::EnableConstLoad || !info || !info->use_const) return;
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(keys);
      const char *k = keys;
      long long* v = int_values;
      for (int i = 0; i < count; i++) {
        String key(k, int_lens[i + 2], CopyString);
        int64_t value = *v++;
        const_load_set(key, value);
        k += int_lens[i + 2] + 1;
      }
      assert((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(keys);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        String key(k, char_lens[i + 2], CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = uninit_null(); break;
        default:
          throw Exception("bad const archive, unknown char type");
        }
        const_load_set(key, value);
        k += char_lens[i + 2] + 1;
      }
      assert((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, string_lens[i + i + 2], CopyString);
        p += string_lens[i + i + 2] + 1;
        String value(p, string_lens[i + i + 3], CopyString);
        const_load_set(key, value);
        p += string_lens[i + i + 3] + 1;
      }
      assert((p - decoded) == len);
    }
  }
  // unserialize_from_string object is extremely slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, object_lens[i + i + 2], CopyString);
        p += object_lens[i + i + 2] + 1;
        String value(p, object_lens[i + i + 3], CopyString);
        const_load_set(key, unserialize_from_string(value));
        p += object_lens[i + i + 3] + 1;
      }
      assert((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, thrift_lens[i + i + 2], CopyString);
        p += thrift_lens[i + i + 2] + 1;
        String value(p, thrift_lens[i + i + 3], CopyString);
        Variant success;
        Variant v = f_fb_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_unserialize failed");
        }
        const_load_set(key, v);
        p += thrift_lens[i + i + 3] + 1;
      }
      assert((p - decoded) == len);
    }
  }
  {//Would we use others[]?
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, other_lens[i + i + 2], CopyString);
        p += other_lens[i + i + 2] + 1;
        String value(p, other_lens[i + i + 3], CopyString);
        Variant v = unserialize_from_string(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, unserialize_from_string failed");
        }
        const_load_set(key, v);
        p += other_lens[i + i + 3] + 1;
      }
      assert((p - decoded) == len);
    }
  }
}

EXTERNALLY_VISIBLE
void apc_load_impl_compressed
    (struct cache_info *info,
     int *int_lens, const char *int_keys, long long *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  if (!apcExtension::ForceConstLoadToAPC) {
    if (apcExtension::EnableConstLoad && info && info->use_const) return;
  }
  auto& s = s_apc_store[0];
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      ScopedMem holder(keys);
      const char *k = keys;
      long long* v = int_values;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = k;
        item.len = int_lens[i + 2];
        s.constructPrime(*v++, item);
        k += int_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      assert((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      ScopedMem holder(keys);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = k;
        item.len = char_lens[i + 2];
        switch (*v++) {
        case 0: s.constructPrime(false, item); break;
        case 1: s.constructPrime(true , item); break;
        case 2: s.constructPrime(uninit_null() , item); break;
        default:
          throw Exception("bad apc archive, unknown char type");
        }
        k += char_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      assert((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = p;
        item.len = string_lens[i + i + 2];
        p += string_lens[i + i + 2] + 1; // skip \0
        // Strings would be copied into APC anyway.
        String value(p, string_lens[i + i + 3], CopyString);
        // todo: t2539893: check if value is already a static string
        s.constructPrime(value, item, false);
        p += string_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      assert((p - decoded) == len);
    }
  }
  {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed APC archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = p;
        item.len = object_lens[i + i + 2];
        p += object_lens[i + i + 2] + 1; // skip \0
        String value(p, object_lens[i + i + 3], CopyString);
        s.constructPrime(value, item, true);
        p += object_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      assert((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = p;
        item.len = thrift_lens[i + i + 2];
        p += thrift_lens[i + i + 2] + 1; // skip \0
        String value(p, thrift_lens[i + i + 3], CopyString);
        Variant success;
        Variant v = f_fb_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_unserialize failed");
        }
        s.constructPrime(v, item);
        p += thrift_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      assert((p - decoded) == len);
    }
  }
  {
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      std::vector<KeyValuePair> vars(count);
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      ScopedMem holder(decoded);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        auto& item = vars[i];
        item.key = p;
        item.len = other_lens[i + i + 2];
        p += other_lens[i + i + 2] + 1; // skip \0
        String value(p, other_lens[i + i + 3], CopyString);
        Variant v = unserialize_from_string(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, unserialize_from_string failed");
        }
        s.constructPrime(v, item);
        p += other_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      assert((p - decoded) == len);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

static double my_time() {
  struct timeval a;
  double t;
  gettimeofday(&a, NULL);
  t = a.tv_sec + (a.tv_usec/1000000.00);
  return t;
}

const StaticString
  s_total("total"),
  s_current("current"),
  s_filename("filename"),
  s_name("name"),
  s_done("done"),
  s_temp_filename("temp_filename"),
  s_cancel_upload("cancel_upload"),
  s_rate("rate");

#define RFC1867_TRACKING_KEY_MAXLEN 63
#define RFC1867_NAME_MAXLEN 63
#define RFC1867_FILENAME_MAXLEN 127

int apc_rfc1867_progress(apc_rfc1867_data *rfc1867ApcData,
                         unsigned int event, void *event_data,
                         void **extra) {
  switch (event) {
  case MULTIPART_EVENT_START: {
    multipart_event_start *data = (multipart_event_start *) event_data;
    rfc1867ApcData->content_length = data->content_length;
    rfc1867ApcData->tracking_key.clear();
    rfc1867ApcData->name.clear();
    rfc1867ApcData->cancel_upload = 0;
    rfc1867ApcData->temp_filename = "";
    rfc1867ApcData->start_time = my_time();
    rfc1867ApcData->bytes_processed = 0;
    rfc1867ApcData->prev_bytes_processed = 0;
    rfc1867ApcData->rate = 0;
    rfc1867ApcData->update_freq = RuntimeOption::Rfc1867Freq;

    if (rfc1867ApcData->update_freq < 0) {
      assert(false); // TODO: support percentage
      // frequency is a percentage, not bytes
      rfc1867ApcData->update_freq =
        rfc1867ApcData->content_length * RuntimeOption::Rfc1867Freq / 100;
    }
    break;
  }

  case MULTIPART_EVENT_FORMDATA: {
    multipart_event_formdata *data = (multipart_event_formdata *)event_data;
    if (data->name &&
        !strncasecmp(data->name, RuntimeOption::Rfc1867Name.c_str(),
                     RuntimeOption::Rfc1867Name.size()) &&
        data->value && data->length &&
        data->length < RFC1867_TRACKING_KEY_MAXLEN -
                       RuntimeOption::Rfc1867Prefix.size()) {
      int len = RuntimeOption::Rfc1867Prefix.size();
      if (len > RFC1867_TRACKING_KEY_MAXLEN) {
        len = RFC1867_TRACKING_KEY_MAXLEN;
      }
      rfc1867ApcData->tracking_key =
        std::string(RuntimeOption::Rfc1867Prefix.c_str(), len);
      len = strlen(*data->value);
      int rem = RFC1867_TRACKING_KEY_MAXLEN -
                rfc1867ApcData->tracking_key.size();
      if (len > rem) len = rem;
      rfc1867ApcData->tracking_key +=
        std::string(*data->value, len);
      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
    }
    /* Facebook: Temporary fix for a bug in PHP's rfc1867 code,
       fixed here for convenience:
       http://cvs.php.net/viewvc.cgi/php-src/main/
       rfc1867.c?r1=1.173.2.1.2.11&r2=1.173.2.1.2.12 */
    (*data->newlength) = data->length;
    break;
  }

  case MULTIPART_EVENT_FILE_START:
    if (!rfc1867ApcData->tracking_key.empty()) {
      multipart_event_file_start *data =
        (multipart_event_file_start *)event_data;

      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
      int len = strlen(*data->filename);
      if (len > RFC1867_FILENAME_MAXLEN) len = RFC1867_FILENAME_MAXLEN;
      rfc1867ApcData->filename = std::string(*data->filename, len);
      rfc1867ApcData->temp_filename = "";
      len = strlen(data->name);
      if (len > RFC1867_NAME_MAXLEN) len = RFC1867_NAME_MAXLEN;
      rfc1867ApcData->name = std::string(data->name, len);
      ArrayInit track(6, ArrayInit::Map{});
      track.set(s_total, rfc1867ApcData->content_length);
      track.set(s_current, rfc1867ApcData->bytes_processed);
      track.set(s_filename, rfc1867ApcData->filename);
      track.set(s_name, rfc1867ApcData->name);
      track.set(s_done, 0);
      track.set(s_start_time, rfc1867ApcData->start_time);
      f_apc_store(rfc1867ApcData->tracking_key, track.create(), 3600);
    }
    break;

  case MULTIPART_EVENT_FILE_DATA:
    if (!rfc1867ApcData->tracking_key.empty()) {
      multipart_event_file_data *data =
        (multipart_event_file_data *) event_data;
      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
      if (rfc1867ApcData->bytes_processed -
          rfc1867ApcData->prev_bytes_processed >
          rfc1867ApcData->update_freq) {
        Variant v;
        if (s_apc_store[0].get(rfc1867ApcData->tracking_key, v)) {
          if (v.is(KindOfArray)) {
            ArrayInit track(6, ArrayInit::Map{});
            track.set(s_total, rfc1867ApcData->content_length);
            track.set(s_current, rfc1867ApcData->bytes_processed);
            track.set(s_filename, rfc1867ApcData->filename);
            track.set(s_name, rfc1867ApcData->name);
            track.set(s_done, 0);
            track.set(s_start_time, rfc1867ApcData->start_time);
            f_apc_store(rfc1867ApcData->tracking_key, track.create(), 3600);
          }
          rfc1867ApcData->prev_bytes_processed =
            rfc1867ApcData->bytes_processed;
        }
      }
    }
    break;

  case MULTIPART_EVENT_FILE_END:
    if (!rfc1867ApcData->tracking_key.empty()) {
      multipart_event_file_end *data =
        (multipart_event_file_end *)event_data;
      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
      rfc1867ApcData->cancel_upload = data->cancel_upload;
      rfc1867ApcData->temp_filename = data->temp_filename;
      ArrayInit track(8, ArrayInit::Map{});
      track.set(s_total, rfc1867ApcData->content_length);
      track.set(s_current, rfc1867ApcData->bytes_processed);
      track.set(s_filename, rfc1867ApcData->filename);
      track.set(s_name, rfc1867ApcData->name);
      track.set(s_temp_filename, rfc1867ApcData->temp_filename);
      track.set(s_cancel_upload, rfc1867ApcData->cancel_upload);
      track.set(s_done, 0);
      track.set(s_start_time, rfc1867ApcData->start_time);
      f_apc_store(rfc1867ApcData->tracking_key, track.create(), 3600);
    }
    break;

  case MULTIPART_EVENT_END:
    if (!rfc1867ApcData->tracking_key.empty()) {
      double now = my_time();
      multipart_event_end *data = (multipart_event_end *)event_data;
      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
      if (now>rfc1867ApcData->start_time) {
        rfc1867ApcData->rate =
          8.0*rfc1867ApcData->bytes_processed/(now-rfc1867ApcData->start_time);
      } else {
        rfc1867ApcData->rate =
          8.0*rfc1867ApcData->bytes_processed;  /* Too quick */
        ArrayInit track(8, ArrayInit::Map{});
        track.set(s_total, rfc1867ApcData->content_length);
        track.set(s_current, rfc1867ApcData->bytes_processed);
        track.set(s_rate, rfc1867ApcData->rate);
        track.set(s_filename, rfc1867ApcData->filename);
        track.set(s_name, rfc1867ApcData->name);
        track.set(s_cancel_upload, rfc1867ApcData->cancel_upload);
        track.set(s_done, 1);
        track.set(s_start_time, rfc1867ApcData->start_time);
        f_apc_store(rfc1867ApcData->tracking_key, track.create(), 3600);
      }
    }
    break;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// apc serialization

String apc_serialize(const Variant& value) {
  VariableSerializer::Type sType =
    apcExtension::EnableApcSerialize ?
      VariableSerializer::Type::APCSerialize :
      VariableSerializer::Type::Serialize;
  VariableSerializer vs(sType);
  return vs.serialize(value, true);
}

Variant apc_unserialize(const char* data, int len) {
  VariableUnserializer::Type sType =
    apcExtension::EnableApcSerialize ?
      VariableUnserializer::Type::APCSerialize :
      VariableUnserializer::Type::Serialize;
  return unserialize_ex(data, len, sType);
}

void reserialize(VariableUnserializer *uns, StringBuffer &buf) {

  char type = uns->readChar();
  char sep = uns->readChar();

  if (type == 'N') {
    buf.append(type);
    buf.append(sep);
    return;
  }

  switch (type) {
  case 'r':
  case 'R':
  case 'b':
  case 'i':
  case 'd':
    {
      buf.append(type);
      buf.append(sep);
      while (uns->peek() != ';') {
        char ch;
        ch = uns->readChar();
        buf.append(ch);
      }
    }
    break;
  case 'S':
  case 'A':
    {
      // shouldn't happen, but keep the code here anyway.
      buf.append(type);
      buf.append(sep);
      char pointer[8];
      uns->read(pointer, 8);
      buf.append(pointer, 8);
    }
    break;
  case 's':
    {
      String v;
      v.unserialize(uns);
      assert(!v.isNull());
      if (v.get()->isStatic()) {
        union {
          char pointer[8];
          StringData *sd;
        } u;
        u.sd = v.get();
        buf.append("S:");
        buf.append(u.pointer, 8);
        buf.append(';');
      } else {
        buf.append("s:");
        buf.append(v.size());
        buf.append(":\"");
        buf.append(v.data(), v.size());
        buf.append("\";");
      }
      sep = uns->readChar();
      return;
    }
    break;
  case 'a':
    {
      buf.append("a:");
      int64_t size = uns->readInt();
      char sep2 = uns->readChar();
      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar();
      buf.append(sep2);
      for (int64_t i = 0; i < size; i++) {
        reserialize(uns, buf); // key
        reserialize(uns, buf); // value
      }
      sep2 = uns->readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'o':
  case 'O':
  case 'V':
  case 'K':
    {
      buf.append(type);
      buf.append(sep);

      String clsName;
      clsName.unserialize(uns);
      buf.append(clsName.size());
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      uns->readChar();
      int64_t size = uns->readInt();
      char sep2 = uns->readChar();

      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar(); // '{'
      buf.append(sep2);
      // 'V' type is a series with values only, while all other
      // types are series with keys and values
      int64_t i = type == 'V' ? size : size * 2;
      while (i--) {
        reserialize(uns, buf);
      }
      sep2 = uns->readChar(); // '}'
      buf.append(sep2);
      return;
    }
    break;
  case 'C':
    {
      buf.append(type);
      buf.append(sep);

      String clsName;
      clsName.unserialize(uns);
      buf.append(clsName.size());
      buf.append(":\"");
      buf.append(clsName.data(), clsName.size());
      buf.append("\":");

      sep = uns->readChar(); // ':'
      String serialized;
      serialized.unserialize(uns, '{', '}');
      buf.append(serialized.size());
      buf.append(":{");
      buf.append(serialized.data(), serialized.size());
      buf.append('}');
      return;
    }
    break;
  default:
    throw Exception("Unknown type '%c'", type);
  }

  sep = uns->readChar(); // the last ';'
  buf.append(sep);
}

String apc_reserialize(const String& str) {
  if (str.empty() ||
      !apcExtension::EnableApcSerialize) return str;

  VariableUnserializer uns(str.data(), str.size(),
                           VariableUnserializer::Type::APCSerialize);
  StringBuffer buf;
  reserialize(&uns, buf);

  return buf.detach();
}

///////////////////////////////////////////////////////////////////////////////
// debugging support

bool apc_dump(const char *filename, bool keyOnly, bool metaDump,
              int waitSeconds) {
  const int CACHE_ID = 0; /* 0 is used as default for apc */
  DumpMode mode;
  std::ofstream out(filename);

  // only one of these should ever be specified
  if (keyOnly && metaDump) {
    return false;
  }

  if (out.fail()) {
    return false;
  }

  if (keyOnly) {
    mode = DumpMode::keyOnly;
  } else if (metaDump) {
    mode = DumpMode::keyAndMeta;
  } else {
    mode = DumpMode::keyAndValue;
  }

  s_apc_store[CACHE_ID].dump(out, mode, waitSeconds);
  out.close();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
