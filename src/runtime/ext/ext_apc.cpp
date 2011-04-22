/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_apc.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_fb.h>
#include <runtime/base/runtime_option.h>
#include <util/async_job.h>
#include <util/timer.h>
#include <dlfcn.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/variable_serializer.h>
#include <util/alloc.h>

using namespace std;

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(apc);
///////////////////////////////////////////////////////////////////////////////

bool f_apc_store(CStrRef key, CVarRef var, int64 ttl /* = 0 */,
                 int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  return s_apc_store[cache_id].store(key, var, ttl);
}

bool f_apc_add(CStrRef key, CVarRef var, int64 ttl /* = 0 */,
               int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  SharedStore &sharedStore = s_apc_store[cache_id];
  return sharedStore.store(key, var, ttl, false);
}

Variant f_apc_fetch(CVarRef key, VRefParam success /* = null */,
                    int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }

  Variant v;

  if (key.is(KindOfArray)) {
    bool tmp = false;
    Array keys = key.toArray();
    ArrayInit init(keys.size(), false);
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      String strKey = k.toString();
      if (s_apc_store[cache_id].get(strKey, v)) {
        tmp = true;
        init.set(strKey, v, true);
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

Variant f_apc_delete(CVarRef key, int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }

  if (key.is(KindOfArray)) {
    Array keys = key.toArray();
    ArrayInit init(keys.size(), true);
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        raise_warning("apc key is not a string");
        init.set(k);
      } else if (!s_apc_store[cache_id].erase(k.toString())) {
        init.set(k);
      }
    }
    return init.create();
  }

  return s_apc_store[cache_id].erase(key.toString());
}

bool f_apc_clear_cache(int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  s_apc_store[cache_id].clear();
  return true;
}

Variant f_apc_inc(CStrRef key, int64 step /* = 1 */,
                  VRefParam success /* = null */, int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  bool found = false;
  int64 newValue = s_apc_store[cache_id].inc(key, step, found);
  success = found;
  return newValue;
}

Variant f_apc_dec(CStrRef key, int64 step /* = 1 */,
                  VRefParam success /* = null */, int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  bool found = false;
  int64 newValue = s_apc_store[cache_id].inc(key, -step, found);
  success = found;
  return newValue;
}

bool f_apc_cas(CStrRef key, int64 old_cas, int64 new_cas,
               int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }
  return s_apc_store[cache_id].cas(key, old_cas, new_cas);
}

Variant f_apc_exists(CVarRef key, int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }

  if (key.is(KindOfArray)) {
    Array keys = key.toArray();
    ArrayInit init(keys.size(), false);
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      String strKey = k.toString();
      if (s_apc_store[cache_id].exists(strKey)) {
        init.set(strKey);
      }
    }
    return init.create();
  }

  return s_apc_store[cache_id].exists(key.toString());
}

Variant f_apc_cache_info(int64 cache_id /* = 0 */, bool limited /* = false */) {
  return CREATE_MAP1("start_time", start_time());
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
                    RuntimeOption::ApcPrimeLibrary.c_str(),
                    error ? error : "(unknown error)");
  }
  return p;
}

DECLARE_BOOST_TYPES(ApcLoadJob);
class ApcLoadJob {
public:
  ApcLoadJob(void *handle, int index) : m_handle(handle), m_index(index) {}
  void *m_handle; int m_index;
};

class ApcLoadWorker {
public:
  void onThreadEnter() {}
  void doJob(ApcLoadJobPtr job) {
    char func_name[128];
    snprintf(func_name, sizeof(func_name), "_apc_load_%d", job->m_index);
    apc_load_func(job->m_handle, func_name)();
  }
  void onThreadExit() {}
};

static size_t s_const_map_size = 0;

void apc_load(int thread) {
  static void *handle = NULL;
  if (handle ||
      RuntimeOption::ApcPrimeLibrary.empty() ||
      !RuntimeOption::EnableApc) {
    return;
  }

  Timer timer(Timer::WallTime, "loading APC data");
  handle = dlopen(RuntimeOption::ApcPrimeLibrary.c_str(), RTLD_LAZY);
  if (!handle) {
    throw Exception("Unable to open apc prime library %s: %s",
                    RuntimeOption::ApcPrimeLibrary.c_str(), dlerror());
  }

  if (thread <= 1) {
    apc_load_func(handle, "_apc_load_all")();
  } else {
    int count = ((int(*)())apc_load_func(handle, "_apc_load_count"))();

    ApcLoadJobPtrVec jobs;
    jobs.reserve(count);
    for (int i = 0; i < count; i++) {
      jobs.push_back(ApcLoadJobPtr(new ApcLoadJob(handle, i)));
    }
    JobDispatcher<ApcLoadJob, ApcLoadWorker>(jobs, thread).run();
  }

  for (set<string>::const_iterator iter =
         RuntimeOption::ApcCompletionKeys.begin();
       iter != RuntimeOption::ApcCompletionKeys.end(); ++iter) {
    f_apc_store(String(*iter), 1);
  }

  if (RuntimeOption::EnableConstLoad) {
#ifndef NO_JEMALLOC
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
#ifndef NO_JEMALLOC
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
extern void const_load_set(CStrRef key, CVarRef value);

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

void const_load_impl(struct cache_info *info,
                     const char **int_keys, int64 *int_values,
                     const char **char_keys, char *char_values,
                     const char **strings, const char **objects,
                     const char **thrifts, const char **others) {
  if (!RuntimeOption::EnableConstLoad || !info || !info->use_const) return;
  {
    int count = count_items(int_keys, 2);
    if (count) {
      const char **k = int_keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        String key(*k, (int)(int64)*(k+1), CopyString);
        int64 value = *v++;
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
        String key(*k, (int)(int64)*(k+1), CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = null; break;
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
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), CopyString);
        const_load_set(key, value);
      }
    }
  }
  // f_unserialize object is extreamly slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = count_items(objects, 4);
    if (count) {
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        const_load_set(key, f_unserialize(value));
      }
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
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
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, f_unserialize failed");
        }
        const_load_set(key, v);
      }
    }
  }
}

void apc_load_impl(struct cache_info *info,
                   const char **int_keys, int64 *int_values,
                   const char **char_keys, char *char_values,
                   const char **strings, const char **objects,
                   const char **thrifts, const char **others) {
  if (!RuntimeOption::ForceConstLoadToAPC) {
    if (RuntimeOption::EnableConstLoad && info && info->use_const) return;
  }
  SharedStore &s = s_apc_store[0];
  {
    int count = count_items(int_keys, 2);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **k = int_keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *k;
        item.len = (int)(int64)*(k+1);
        item.value = s.construct(item.key, item.len, *v++);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(char_keys, 2);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **k = char_keys;
      char *v = char_values;
      for (int i = 0; i < count; i++, k += 2) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *k;
        item.len = (int)(int64)*(k+1);
        switch (*v++) {
        case 0: item.value = s.construct(item.key, item.len, false); break;
        case 1: item.value = s.construct(item.key, item.len, true ); break;
        case 2: item.value = s.construct(item.key, item.len, null ); break;
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
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = strings;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        // Strings would be copied into APC anyway.
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        value.checkStatic();
        item.value = s.construct(item.key, item.len, value, false);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(objects, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        item.value = s.construct(item.key, item.len, value, true);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(others, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = others;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);

        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, f_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
      }
      s.prime(vars);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Constant and APC priming with compressed data

void const_load_impl_compressed
    (struct cache_info *info,
     int *int_lens, const char *int_keys, int64 *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  if (!RuntimeOption::EnableConstLoad || !info || !info->use_const) return;
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++) {
        String key(k, int_lens[i + 2], CopyString);
        int64 value = *v++;
        const_load_set(key, value);
        k += int_lens[i + 2] + 1;
      }
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        String key(k, char_lens[i + 2], CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = null; break;
        default:
          throw Exception("bad const archive, unknown char type");
        }
        const_load_set(key, value);
        k += char_lens[i + 2] + 1;
      }
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, string_lens[i + i + 2], CopyString);
        p += string_lens[i + i + 2] + 1;
        String value(p, string_lens[i + i + 3], CopyString);
        const_load_set(key, value);
        p += string_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  // f_unserialize object is extreamly slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, object_lens[i + i + 2], CopyString);
        p += object_lens[i + i + 2] + 1;
        String value(p, object_lens[i + i + 3], AttachLiteral);
        const_load_set(key, f_unserialize(value));
        p += object_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, thrift_lens[i + i + 2], CopyString);
        p += thrift_lens[i + i + 2] + 1;
        String value(p, thrift_lens[i + i + 3], AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        const_load_set(key, v);
        p += thrift_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  {//Would we use others[]?
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, other_lens[i + i + 2], CopyString);
        p += other_lens[i + i + 2] + 1;
        String value(p, other_lens[i + i + 3], AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, f_unserialize failed");
        }
        const_load_set(key, v);
        p += other_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
}

void apc_load_impl_compressed
    (struct cache_info *info,
     int *int_lens, const char *int_keys, int64 *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  if (!RuntimeOption::ForceConstLoadToAPC) {
    if (RuntimeOption::EnableConstLoad && info && info->use_const) return;
  }
  SharedStore &s = s_apc_store[0];
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = k;
        item.len = int_lens[i + 2];
        item.value = s.construct(item.key, item.len, *v++);
        k += int_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = k;
        item.len = char_lens[i + 2];
        switch (*v++) {
        case 0: item.value = s.construct(item.key, item.len, false); break;
        case 1: item.value = s.construct(item.key, item.len, true ); break;
        case 2: item.value = s.construct(item.key, item.len, null ); break;
        default:
          throw Exception("bad apc archive, unknown char type");
        }
        k += char_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = string_lens[i + i + 2];
        p += string_lens[i + i + 2] + 1; // skip \0
        // Strings would be copied into APC anyway.
        String value(p, string_lens[i + i + 3], AttachLiteral);
        value.checkStatic();
        item.value = s.construct(item.key, item.len, value, false);
        p += string_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed APC archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = object_lens[i + i + 2];
        p += object_lens[i + i + 2] + 1; // skip \0
        String value(p, object_lens[i + i + 3], AttachLiteral);
        item.value = s.construct(item.key, item.len, value, true);
        p += object_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = thrift_lens[i + i + 2];
        p += thrift_lens[i + i + 2] + 1; // skip \0
        String value(p, thrift_lens[i + i + 3], AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
        p += thrift_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = other_lens[i + i + 2];
        p += other_lens[i + i + 2] + 1; // skip \0
        String value(p, other_lens[i + i + 3], AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, f_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
        p += other_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////


// =============================   Temporary   ==============================

void const_load_impl(const char **int_keys, int64 *int_values,
                     const char **char_keys, char *char_values,
                     const char **strings, const char **objects,
                     const char **thrifts, const char **others) {
  {
    int count = count_items(int_keys, 2);
    if (count) {
      const char **k = int_keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        String key(*k, (int)(int64)*(k+1), CopyString);
        int64 value = *v++;
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
        String key(*k, (int)(int64)*(k+1), CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = null; break;
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
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), CopyString);
        const_load_set(key, value);
      }
    }
  }
  // f_unserialize object is extreamly slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = count_items(objects, 4);
    if (count) {
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        const_load_set(key, f_unserialize(value));
      }
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
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
        String key(*p, (int)(int64)*(p+1), CopyString);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, f_unserialize failed");
        }
        const_load_set(key, v);
      }
    }
  }
}

void apc_load_impl(const char **int_keys, int64 *int_values,
                   const char **char_keys, char *char_values,
                   const char **strings, const char **objects,
                   const char **thrifts, const char **others) {
  SharedStore &s = s_apc_store[0];
  {
    int count = count_items(int_keys, 2);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **k = int_keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++, k += 2) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *k;
        item.len = (int)(int64)*(k+1);
        item.value = s.construct(item.key, item.len, *v++);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(char_keys, 2);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **k = char_keys;
      char *v = char_values;
      for (int i = 0; i < count; i++, k += 2) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *k;
        item.len = (int)(int64)*(k+1);
        switch (*v++) {
        case 0: item.value = s.construct(item.key, item.len, false); break;
        case 1: item.value = s.construct(item.key, item.len, true ); break;
        case 2: item.value = s.construct(item.key, item.len, null ); break;
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
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = strings;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        // Strings would be copied into APC anyway.
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        value.checkStatic();
        item.value = s.construct(item.key, item.len, value, false);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(objects, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = objects;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        item.value = s.construct(item.key, item.len, value, true);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(thrifts, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = thrifts;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);
        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
      }
      s.prime(vars);
    }
  }
  {
    int count = count_items(others, 4);
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      const char **p = others;
      for (int i = 0; i < count; i++, p += 4) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = *p;
        item.len = (int)(int64)*(p+1);

        String value(*(p+2), (int)(int64)*(p+3), AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, f_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
      }
      s.prime(vars);
    }
  }
}

void const_load_impl_compressed
    (int *int_lens, const char *int_keys, int64 *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++) {
        String key(k, int_lens[i + 2], CopyString);
        int64 value = *v++;
        const_load_set(key, value);
        k += int_lens[i + 2] + 1;
      }
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed const archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        String key(k, char_lens[i + 2], CopyString);
        Variant value;
        switch (*v++) {
        case 0: value = false; break;
        case 1: value = true; break;
        case 2: value = null; break;
        default:
          throw Exception("bad const archive, unknown char type");
        }
        const_load_set(key, value);
        k += char_lens[i + 2] + 1;
      }
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, string_lens[i + i + 2], CopyString);
        p += string_lens[i + i + 2] + 1;
        String value(p, string_lens[i + i + 3], CopyString);
        const_load_set(key, value);
        p += string_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  // f_unserialize object is extreamly slow here;
  // currently turned off: no objects in haste_maps.
  if (false) {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, object_lens[i + i + 2], CopyString);
        p += object_lens[i + i + 2] + 1;
        String value(p, object_lens[i + i + 3], AttachLiteral);
        const_load_set(key, f_unserialize(value));
        p += object_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, thrift_lens[i + i + 2], CopyString);
        p += thrift_lens[i + i + 2] + 1;
        String value(p, thrift_lens[i + i + 3], AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        const_load_set(key, v);
        p += thrift_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
  {//Would we use others[]?
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed const archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        String key(p, other_lens[i + i + 2], CopyString);
        p += other_lens[i + i + 2] + 1;
        String value(p, other_lens[i + i + 3], AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          throw Exception("bad apc archive, f_unserialize failed");
        }
        const_load_set(key, v);
        p += other_lens[i + i + 3] + 1;
      }
      ASSERT((p - decoded) == len);
    }
  }
}

void apc_load_impl_compressed
    (int *int_lens, const char *int_keys, int64 *int_values,
     int *char_lens, const char *char_keys, char *char_values,
     int *string_lens, const char *strings,
     int *object_lens, const char *objects,
     int *thrift_lens, const char *thrifts,
     int *other_lens, const char *others) {
  SharedStore &s = s_apc_store[0];
  {
    int count = int_lens[0];
    int len = int_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *keys = gzdecode(int_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      int64 *v = int_values;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = k;
        item.len = int_lens[i + 2];
        item.value = s.construct(item.key, item.len, *v++);
        k += int_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = char_lens[0];
    int len = char_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *keys = gzdecode(char_keys, len);
      if (keys == NULL) throw Exception("bad compressed apc archive.");
      String holder(keys, len, AttachString);
      const char *k = keys;
      char *v = char_values;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = k;
        item.len = char_lens[i + 2];
        switch (*v++) {
        case 0: item.value = s.construct(item.key, item.len, false); break;
        case 1: item.value = s.construct(item.key, item.len, true ); break;
        case 2: item.value = s.construct(item.key, item.len, null ); break;
        default:
          throw Exception("bad apc archive, unknown char type");
        }
        k += char_lens[i + 2] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((k - keys) == len);
    }
  }
  {
    int count = string_lens[0] / 2;
    int len = string_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(strings, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = string_lens[i + i + 2];
        p += string_lens[i + i + 2] + 1; // skip \0
        // Strings would be copied into APC anyway.
        String value(p, string_lens[i + i + 3], AttachLiteral);
        value.checkStatic();
        item.value = s.construct(item.key, item.len, value, false);
        p += string_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = object_lens[0] / 2;
    int len = object_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(objects, len);
      if (decoded == NULL) throw Exception("bad compressed APC archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = object_lens[i + i + 2];
        p += object_lens[i + i + 2] + 1; // skip \0
        String value(p, object_lens[i + i + 3], AttachLiteral);
        item.value = s.construct(item.key, item.len, value, true);
        p += object_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = thrift_lens[0] / 2;
    int len = thrift_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(thrifts, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = thrift_lens[i + i + 2];
        p += thrift_lens[i + i + 2] + 1; // skip \0
        String value(p, thrift_lens[i + i + 3], AttachLiteral);
        Variant success;
        Variant v = f_fb_thrift_unserialize(value, ref(success));
        if (same(success, false)) {
          throw Exception("bad apc archive, f_fb_thrift_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
        p += thrift_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
  {
    int count = other_lens[0] / 2;
    int len = other_lens[1];
    if (count) {
      vector<SharedStore::KeyValuePair> vars(count);
      char *decoded = gzdecode(others, len);
      if (decoded == NULL) throw Exception("bad compressed apc archive.");
      String holder(decoded, len, AttachString);
      const char *p = decoded;
      for (int i = 0; i < count; i++) {
        SharedStore::KeyValuePair &item = vars[i];
        item.key = p;
        item.len = other_lens[i + i + 2];
        p += other_lens[i + i + 2] + 1; // skip \0
        String value(p, other_lens[i + i + 3], AttachLiteral);
        Variant v = f_unserialize(value);
        if (same(v, false)) {
          // we can't possibly get here if it was a boolean "false" that's
          // supposed to be serialized as a char
          throw Exception("bad apc archive, f_unserialize failed");
        }
        item.value = s.construct(item.key, item.len, v);
        p += other_lens[i + i + 3] + 1; // skip \0
      }
      s.prime(vars);
      ASSERT((p - decoded) == len);
    }
  }
}

// ================================   End   ================================

///////////////////////////////////////////////////////////////////////////////


static double my_time() {
  struct timeval a;
  double t;
  gettimeofday(&a, NULL);
  t = a.tv_sec + (a.tv_usec/1000000.00);
  return t;
}

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
    rfc1867ApcData->temp_filename = NULL;
    rfc1867ApcData->start_time = my_time();
    rfc1867ApcData->bytes_processed = 0;
    rfc1867ApcData->prev_bytes_processed = 0;
    rfc1867ApcData->rate = 0;
    rfc1867ApcData->update_freq = RuntimeOption::Rfc1867Freq;

    if (rfc1867ApcData->update_freq < 0) {
      ASSERT(false); // TODO: support percentage
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
        string(RuntimeOption::Rfc1867Prefix.c_str(), len);
      len = strlen(*data->value);
      int rem = RFC1867_TRACKING_KEY_MAXLEN -
                rfc1867ApcData->tracking_key.size();
      if (len > rem) len = rem;
      rfc1867ApcData->tracking_key +=
        string(*data->value, len);
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
      rfc1867ApcData->filename = string(*data->filename, len);
      rfc1867ApcData->temp_filename = NULL;
      len = strlen(data->name);
      if (len > RFC1867_NAME_MAXLEN) len = RFC1867_NAME_MAXLEN;
      rfc1867ApcData->name = string(data->name, len);
      Array track;
      track.set("total", rfc1867ApcData->content_length);
      track.set("current", rfc1867ApcData->bytes_processed);
      track.set("filename", rfc1867ApcData->filename);
      track.set("name", rfc1867ApcData->name);
      track.set("done", 0);
      track.set("start_time", rfc1867ApcData->start_time);
      f_apc_store(rfc1867ApcData->tracking_key, track, 3600);
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
            Array track;
            track.set("total", rfc1867ApcData->content_length);
            track.set("current", rfc1867ApcData->bytes_processed);
            track.set("filename", rfc1867ApcData->filename);
            track.set("name", rfc1867ApcData->name);
            track.set("done", 0);
            track.set("start_time", rfc1867ApcData->start_time);
            f_apc_store(rfc1867ApcData->tracking_key, track, 3600);
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
      Array track;
      track.set("total", rfc1867ApcData->content_length);
      track.set("current", rfc1867ApcData->bytes_processed);
      track.set("filename", rfc1867ApcData->filename);
      track.set("name", rfc1867ApcData->name);
      track.set("temp_filename", rfc1867ApcData->temp_filename, CopyString);
      track.set("cancel_upload", rfc1867ApcData->cancel_upload);
      track.set("done", 0);
      track.set("start_time", rfc1867ApcData->start_time);
      f_apc_store(rfc1867ApcData->tracking_key, track, 3600);
    }
    break;

  case MULTIPART_EVENT_END:
    if (!rfc1867ApcData->tracking_key.empty()) {
      double now = my_time();
      multipart_event_end *data = (multipart_event_end *)event_data;
      rfc1867ApcData->bytes_processed = data->post_bytes_processed;
      if(now>rfc1867ApcData->start_time) {
        rfc1867ApcData->rate =
          8.0*rfc1867ApcData->bytes_processed/(now-rfc1867ApcData->start_time);
      } else {
        rfc1867ApcData->rate =
          8.0*rfc1867ApcData->bytes_processed;  /* Too quick */
        Array track;
        track.set("total", rfc1867ApcData->content_length);
        track.set("current", rfc1867ApcData->bytes_processed);
        track.set("rate", rfc1867ApcData->rate);
        track.set("filename", rfc1867ApcData->filename);
        track.set("name", rfc1867ApcData->name);
        track.set("cancel_upload", rfc1867ApcData->cancel_upload);
        track.set("done", 1);
        track.set("start_time", rfc1867ApcData->start_time);
        f_apc_store(rfc1867ApcData->tracking_key, track, 3600);
      }
    }
    break;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// apc serialization

String apc_serialize(CVarRef value) {
  VariableSerializer vs(VariableSerializer::APCSerialize);
  return vs.serialize(value, true);
}

Variant apc_unserialize(CStrRef str) {
  return unserialize_ex(str, VariableUnserializer::APCSerialize);
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
      ASSERT(!v.isNull());
      if (v->isStatic()) {
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
      int64 size = uns->readInt();
      char sep2 = uns->readChar();
      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar();
      buf.append(sep2);
      for (int64 i = 0; i < size; i++) {
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
      int64 size = uns->readInt();
      char sep2 = uns->readChar();

      buf.append(size);
      buf.append(sep2);
      sep2 = uns->readChar(); // '{'
      buf.append(sep2);
      for (int64 i = 0; i < size; i++) {
        reserialize(uns, buf); // property name
        reserialize(uns, buf); // property value
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

String apc_reserialize(CStrRef str) {
  if (str.empty()) return str;

  VariableUnserializer uns(str.data(), str.size(),
                           VariableUnserializer::APCSerialize);
  StringBuffer buf;
  reserialize(&uns, buf);

  return buf.detach();
}

///////////////////////////////////////////////////////////////////////////////
// debugging support

bool apc_dump(const char *filename) {
  const int CACHE_ID = 0; /* 0 is used as default for apc */
  std::ofstream out(filename);
  if (out.fail()) {
    return false;
  }
  s_apc_store[CACHE_ID].dump(out);
  out.close();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
