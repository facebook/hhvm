/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

Variant f_apc_fetch(CVarRef key, Variant success /* = null */,
                    int64 cache_id /* = 0 */) {
  if (!RuntimeOption::EnableApc) return false;

  if (cache_id < 0 || cache_id >= MAX_SHARED_STORE) {
    throw_invalid_argument("cache_id: %d", cache_id);
    return false;
  }

  Variant v;

  if (key.is(KindOfArray)) {
    bool tmp = false;
    Array ret = Array::Create();
    Array keys = key.toArray();
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        throw_invalid_argument("apc key: (not a string)");
        return false;
      }
      if (s_apc_store[cache_id].get(k.toString(), v)) {
        tmp = true;
        ret.set(k, v);
      }
    }
    success = tmp;
    return ret;
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
    Array ret = Array::Create();
    Array keys = key.toArray();
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        raise_warning("apc key is not a string");
        ret.append(k);
      } else if (!s_apc_store[cache_id].erase(k.toString())) {
        ret.append(k);
      }
    }
    return ret;
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
                  Variant success /* = null */, int64 cache_id /* = 0 */) {
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
                  Variant success /* = null */, int64 cache_id /* = 0 */) {
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

Variant f_apc_cache_info(int64 cache_id /* = 0 */, bool limited /* = false */) {
  return CREATE_MAP1("start_time", start_time());
}

///////////////////////////////////////////////////////////////////////////////
// loading APC from archive files

typedef void(*PFUNC_APC_LOAD)();

static Mutex dl_mutex;
static PFUNC_APC_LOAD apc_load_func(void *handle, const char *name) {
  Lock lock(dl_mutex);
  dlerror(); // clear errors
  PFUNC_APC_LOAD p = (PFUNC_APC_LOAD)dlsym(handle, name);
  char *error = dlerror();
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

  // We've copied all the data out, so close it out.
  dlclose(handle);
}

static int count_items(const char **p, int step) {
  int count = 0;
  for (const char **k = p; *k; k += step) {
    count++;
  }
  return count;
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
        String value(*(p+2), (int)(int64)*(p+3), CopyString);
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
        String value(*(p+2), (int)(int64)*(p+3), CopyString);
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
        String value(*(p+2), (int)(int64)*(p+3), CopyString);
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
}
