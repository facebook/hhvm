/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_memcache.h"
#include "hphp/runtime/ext/libmemcached_portability.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/ini-setting.h"

#include "hphp/system/systemlib.h"

#define MMC_SERIALIZED 1
#define MMC_COMPRESSED 2

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(memcache);

static bool ini_on_update_hash_strategy(const String& value, void *p);
static String ini_get_hash_strategy(void *p);
static bool ini_on_update_hash_function(const String& value, void *p);
static String ini_get_hash_function(void *p);

class MEMCACHEGlobals : public RequestEventHandler {
public:
  std::string hash_strategy;
  std::string hash_function;

  MEMCACHEGlobals() {}

  virtual void requestInit() {
    hash_strategy = "standard";
    hash_function = "crc32";

    IniSetting::Bind("memcache.hash_strategy", "standard",
                     ini_on_update_hash_strategy, ini_get_hash_strategy,
                     &hash_strategy);
    IniSetting::Bind("memcache.hash_function", "crc32",
                     ini_on_update_hash_function, ini_get_hash_function,
                     &hash_function);
  }

  virtual void requestShutdown() {
  }
};

IMPLEMENT_STATIC_REQUEST_LOCAL(MEMCACHEGlobals, s_memcache_globals);
#define MEMCACHEG(name) s_memcache_globals->name

static bool ini_on_update_hash_strategy(const String& value, void *p) {
  if (!strncasecmp(value.data(), "standard", sizeof("standard"))) {
    MEMCACHEG(hash_strategy) = "standard";
  } else if (!strncasecmp(value.data(), "consistent", sizeof("consistent"))) {
    MEMCACHEG(hash_strategy) = "consistent";
  }
  return false;
}

static String ini_get_hash_strategy(void *p) {
  return MEMCACHEG(hash_strategy);
}

static bool ini_on_update_hash_function(const String& value, void *p) {
  if (!strncasecmp(value.data(), "crc32", sizeof("crc32"))) {
    MEMCACHEG(hash_strategy) = "crc32";
  } else if (!strncasecmp(value.data(), "fnv", sizeof("fnv"))) {
    MEMCACHEG(hash_strategy) = "fnv";
  }
  return false;
}

static String ini_get_hash_function(void *p) {
  return MEMCACHEG(hash_strategy);
}

///////////////////////////////////////////////////////////////////////////////
// methods

c_Memcache::c_Memcache(Class* cb) :
    ExtObjectData(cb), m_memcache(), m_compress_threshold(0),
    m_min_compress_savings(0.2) {
  memcached_create(&m_memcache);

  if (MEMCACHEG(hash_strategy) == "consistent") {
    // need to hook up a global variable to set this
    memcached_behavior_set(&m_memcache, MEMCACHED_BEHAVIOR_DISTRIBUTION,
                           MEMCACHED_DISTRIBUTION_CONSISTENT_KETAMA);
  } else {
    memcached_behavior_set(&m_memcache, MEMCACHED_BEHAVIOR_DISTRIBUTION,
                           MEMCACHED_DISTRIBUTION_MODULA);
  }

  if (MEMCACHEG(hash_function) == "fnv") {
    memcached_behavior_set(&m_memcache, MEMCACHED_BEHAVIOR_HASH,
                           MEMCACHED_HASH_FNV1A_32);
  } else {
    memcached_behavior_set(&m_memcache, MEMCACHED_BEHAVIOR_HASH,
                           MEMCACHED_HASH_CRC);
  }
}

c_Memcache::~c_Memcache() {
  memcached_free(&m_memcache);
}

void c_Memcache::t___construct() {
  return;
}

bool c_Memcache::t_connect(const String& host, int port /*= 0*/,
                           int timeout /*= 0*/,
                           int timeoutms /*= 0*/) {
  memcached_return_t ret;

  if (!host.empty() && host[0] == '/') {
    ret = memcached_server_add_unix_socket(&m_memcache, host.c_str());
  } else {
    ret = memcached_server_add(&m_memcache, host.c_str(), port);
  }

  return (ret == MEMCACHED_SUCCESS);
}

bool c_Memcache::t_pconnect(const String& host, int port /*= 0*/,
                            int timeout /*= 0*/,
                            int timeoutms /*= 0*/) {
  return t_connect(host, port, timeout, timeoutms);
}

String static memcache_prepare_for_storage(CVarRef var, int &flag) {
  if (var.isString()) {
    return var.toString();
  } else if (var.isNumeric() || var.isBoolean()) {
    return var.toString();
  } else {
    flag |= MMC_SERIALIZED;
    return f_serialize(var);
  }
}

Variant static memcache_fetch_from_storage(const char *payload,
                                           size_t payload_len,
                                           uint32_t flags) {
  Variant ret = uninit_null();

  if (flags & MMC_COMPRESSED) {
    raise_warning("Unable to handle compressed values yet");
    return uninit_null();
  }

  if (flags & MMC_SERIALIZED) {
    ret = unserialize_from_buffer(payload, payload_len);
  } else {
    ret = String(payload, payload_len, CopyString);
  }

  return ret;
}

bool c_Memcache::t_add(const String& key, CVarRef var, int flag /*= 0*/,
                       int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  String serialized = memcache_prepare_for_storage(var, flag);

  memcached_return_t ret = memcached_add(&m_memcache,
                                        key.c_str(), key.length(),
                                        serialized.c_str(),
                                        serialized.length(),
                                        expire, flag);

  return (ret == MEMCACHED_SUCCESS);
}

bool c_Memcache::t_set(const String& key, CVarRef var, int flag /*= 0*/,
                       int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  String serialized = memcache_prepare_for_storage(var, flag);

  memcached_return_t ret = memcached_set(&m_memcache,
                                        key.c_str(), key.length(),
                                        serialized.c_str(),
                                        serialized.length(),
                                        expire, flag);

  if (ret == MEMCACHED_SUCCESS) {
    return true;
  }

  return false;
}

bool c_Memcache::t_replace(const String& key, CVarRef var, int flag /*= 0*/,
                           int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  String serialized = memcache_prepare_for_storage(var, flag);

  memcached_return_t ret = memcached_replace(&m_memcache,
                                             key.c_str(), key.length(),
                                             serialized.c_str(),
                                             serialized.length(),
                                             expire, flag);
  return (ret == MEMCACHED_SUCCESS);
}

Variant c_Memcache::t_get(CVarRef key, VRefParam flags /*= null*/) {
  if (key.is(KindOfArray)) {
    std::vector<const char *> real_keys;
    std::vector<size_t> key_len;
    Array keyArr = key.toArray();

    real_keys.reserve(keyArr.size());
    key_len.reserve(keyArr.size());

    for (ArrayIter iter(keyArr); iter; ++iter) {
      real_keys.push_back(const_cast<char *>(iter.second().toString().c_str()));
      key_len.push_back(iter.second().toString().length());
    }

    if (!real_keys.empty()) {
      const char *payload = NULL;
      size_t payload_len = 0;
      uint32_t flags = 0;
      const char *res_key = NULL;
      size_t res_key_len = 0;

      memcached_result_st result;

      memcached_return_t ret = memcached_mget(&m_memcache, &real_keys[0],
                                              &key_len[0], real_keys.size());
      memcached_result_create(&m_memcache, &result);
      Array return_val;

      while ((memcached_fetch_result(&m_memcache, &result, &ret)) != NULL) {
        if (ret != MEMCACHED_SUCCESS) {
          // should probably notify about errors
          continue;
        }

        payload     = memcached_result_value(&result);
        payload_len = memcached_result_length(&result);
        flags       = memcached_result_flags(&result);
        res_key     = memcached_result_key_value(&result);
        res_key_len = memcached_result_key_length(&result);

        return_val.set(String(res_key, res_key_len, CopyString),
                       memcache_fetch_from_storage(payload,
                                                   payload_len, flags));
      }
      memcached_result_free(&result);

      return return_val;
    }
  } else {
    char *payload = NULL;
    size_t payload_len = 0;
    uint32_t flags = 0;

    memcached_return_t ret;
    String skey = key.toString();

    if (skey.length() == 0) {
      return false;
    }

    payload = memcached_get(&m_memcache, skey.c_str(), skey.length(),
                            &payload_len, &flags, &ret);

    /* This is for historical reasons from libmemcached*/
    if (ret == MEMCACHED_END) {
      ret = MEMCACHED_NOTFOUND;
    }

    if (ret == MEMCACHED_NOTFOUND) {
      return false;
    }

    Variant retval = memcache_fetch_from_storage(payload, payload_len, flags);
    free(payload);

    return retval;
  }
  return false;
}

bool c_Memcache::t_delete(const String& key, int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  memcached_return_t ret = memcached_delete(&m_memcache,
                                            key.c_str(), key.length(),
                                            expire);
  return (ret == MEMCACHED_SUCCESS);
}

int64_t c_Memcache::t_increment(const String& key, int offset /*= 1*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  uint64_t value;
  memcached_return_t ret = memcached_increment(&m_memcache, key.c_str(),
                                              key.length(), offset, &value);

  if (ret == MEMCACHED_SUCCESS) {
    return (int64_t)value;
  }

  return false;
}

int64_t c_Memcache::t_decrement(const String& key, int offset /*= 1*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  uint64_t value;
  memcached_return_t ret = memcached_decrement(&m_memcache, key.c_str(),
                                              key.length(), offset, &value);

  if (ret == MEMCACHED_SUCCESS) {
    return (int64_t)value;
  }

  return false;
}

bool c_Memcache::t_close() {
  memcached_quit(&m_memcache);
  return true;
}

Variant c_Memcache::t_getversion() {
  int server_count = memcached_server_count(&m_memcache);
  char version[16];
  int version_len = 0;

  if (memcached_version(&m_memcache) != MEMCACHED_SUCCESS) {
    return false;
  }

  for (int x = 0; x < server_count; x++) {
    LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
      memcached_server_instance_by_position(&m_memcache, x);
    uint8_t majorVersion = LMCD_SERVER_MAJOR_VERSION(instance);
    uint8_t minorVersion = LMCD_SERVER_MINOR_VERSION(instance);
    uint8_t microVersion = LMCD_SERVER_MICRO_VERSION(instance);

    if (!majorVersion) {
      continue;
    }

    version_len = snprintf(version, sizeof(version),
        "%" PRIu8 ".%" PRIu8 ".%" PRIu8,
        majorVersion, minorVersion, microVersion);
    return String(version, version_len, CopyString);
  }

  return false;
}

bool c_Memcache::t_flush(int expire /*= 0*/) {
  return memcached_flush(&m_memcache, expire) == MEMCACHED_SUCCESS;
}

bool c_Memcache::t_setoptimeout(int64_t timeoutms) {
  if (timeoutms < 1) {
    timeoutms = 1000; // make default
  }

  /* intentionally doing nothing for now */

  return true;
}

int64_t c_Memcache::t_getserverstatus(const String& host, int port /* = 0 */) {
  /* intentionally doing nothing for now */
  return 1;
}

bool c_Memcache::t_setcompressthreshold(int threshold,
                                        double min_savings /* = 0.2 */) {
  if (threshold < 0) {
    raise_warning("threshold must be a positive integer");
    return false;
  }

  if (min_savings < 0 || min_savings > 1) {
    raise_warning("min_savings must be a float in the 0..1 range");
    return false;
  }

  m_compress_threshold = threshold;
  m_min_compress_savings = min_savings;

  return true;
}

Array static memcache_build_stats(const memcached_st *ptr,
                                memcached_stat_st *memc_stat,
                                memcached_return_t *ret) {
  char **curr_key;
  char **stat_keys = memcached_stat_get_keys(const_cast<memcached_st*>(ptr),
                                             memc_stat, ret);

  if (*ret != MEMCACHED_SUCCESS) {
    if (stat_keys) {
      free(stat_keys);
    }
    return NULL;
  }

  Array return_val = Array::Create();

  for (curr_key = stat_keys; *curr_key; curr_key++) {
    char *mc_val;
    mc_val = memcached_stat_get_value(ptr, memc_stat, *curr_key, ret);
    if (*ret != MEMCACHED_SUCCESS) {
      break;
    }
    return_val.set(String(*curr_key, CopyString),
                   String(mc_val, CopyString));
    free(mc_val);
  }

  free(stat_keys);
  return return_val;
}


Array c_Memcache::t_getstats(const String& type /* = null_string */,
                             int slabid /* = 0 */, int limit /* = 100 */) {
  if (!memcached_server_count(&m_memcache)) {
    return NULL;
  }

  char extra_args[30] = {0};

  if (slabid) {
    snprintf(extra_args, sizeof(extra_args), "%s %d %d", type.c_str(),
             slabid, limit);
  } else if (!type.empty()) {
    snprintf(extra_args, sizeof(extra_args), "%s", type.c_str());
  }

  LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
    memcached_server_instance_by_position(&m_memcache, 0);
  const char *hostname = LMCD_SERVER_HOSTNAME(instance);
  in_port_t port = LMCD_SERVER_PORT(instance);

  memcached_stat_st stats;
  if (memcached_stat_servername(&stats, extra_args, hostname,
                                port) != MEMCACHED_SUCCESS) {
    return NULL;
  }

  memcached_return_t ret;
  return memcache_build_stats(&m_memcache, &stats, &ret);
}

Array c_Memcache::t_getextendedstats(const String& type /* = null_string */,
                                     int slabid /* = 0 */,
                                     int limit /* = 100 */) {
  memcached_return_t ret;
  memcached_stat_st *stats;

  stats = memcached_stat(&m_memcache, NULL, &ret);
  if (ret != MEMCACHED_SUCCESS) {
    return NULL;
  }

  int server_count = memcached_server_count(&m_memcache);

  Array return_val;

  for (int server_id = 0; server_id < server_count; server_id++) {
    memcached_stat_st *stat;
    char stats_key[30] = {0};
    size_t key_len;

    LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
      memcached_server_instance_by_position(&m_memcache, server_id);
    const char *hostname = LMCD_SERVER_HOSTNAME(instance);
    in_port_t port = LMCD_SERVER_PORT(instance);

    stat = stats + server_id;

    Array server_stats = memcache_build_stats(&m_memcache, stat, &ret);
    if (ret != MEMCACHED_SUCCESS) {
      continue;
    }

    key_len = snprintf(stats_key, sizeof(stats_key), "%s:%d", hostname, port);

    return_val.set(String(stats_key, key_len, CopyString), server_stats);
  }

  free(stats);
  return return_val;
}

bool c_Memcache::t_setserverparams(const String& host, int port /* = 11211 */,
                                   int timeout /* = 0 */,
                                   int retry_interval /* = 0 */,
                                   bool status /* = true */,
                                   CVarRef failure_callback /* = null_variant */) {
  /* intentionally doing nothing for now */
  return true;
}

bool c_Memcache::t_addserver(const String& host, int port /* = 11211 */,
                             bool persistent /* = false */,
                             int weight /* = 0 */, int timeout /* = 0 */,
                             int retry_interval /* = 0 */,
                             bool status /* = true */,
                             CVarRef failure_callback /* = null_variant */,
                             int timeoutms /* = 0 */) {
  memcached_return_t ret;

  if (!host.empty() && host[0] == '/') {
    ret = memcached_server_add_unix_socket_with_weight(&m_memcache,
                                                       host.c_str(), weight);
  } else {
    ret = memcached_server_add_with_weight(&m_memcache, host.c_str(),
                                           port, weight);
  }

  if (ret == MEMCACHED_SUCCESS) {
    return true;
  }

  return false;
}

Variant c_Memcache::t___destruct() {
  t_close();
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////
// these all pass to their OO equivalents

Object f_memcache_connect(const String& host, int port /* = 0 */,
                          int timeout /* = 0 */, int timeoutms /* = 0 */) {
  c_Memcache *memcache_obj = NEWOBJ(c_Memcache)();
  Object ret(memcache_obj);
  memcache_obj->t_connect(host, port, timeout, timeoutms);
  return ret;
}

Object f_memcache_pconnect(const String& host, int port /* = 0 */,
                           int timeout /* = 0 */, int timeoutms /* = 0 */) {
  return f_memcache_connect(host, port, timeout, timeoutms);
}

bool f_memcache_add(CObjRef memcache, const String& key, CVarRef var,
                    int flag /* = 0 */, int expire /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_add(key, var, flag, expire);
}

bool f_memcache_set(CObjRef memcache, const String& key, CVarRef var,
                    int flag /* = 0 */, int expire /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_set(key, var, flag, expire);
}

bool f_memcache_replace(CObjRef memcache, const String& key, CVarRef var,
                        int flag /* = 0 */, int expire /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_replace(key, var, flag, expire);
}

Variant f_memcache_get(CObjRef memcache, CVarRef key,
                       VRefParam flags /* = null */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_get(key, flags);
}

bool f_memcache_delete(CObjRef memcache, const String& key, int expire /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_delete(key, expire);
}

int64_t f_memcache_increment(CObjRef memcache, const String& key,
                           int offset /* = 1 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_increment(key, offset);
}

int64_t f_memcache_decrement(CObjRef memcache, const String& key,
                           int offset /* = 1 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_decrement(key, offset);
}

bool f_memcache_close(CObjRef memcache) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_close();
}

Variant f_memcache_get_version(CObjRef memcache) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_getversion();
}

bool f_memcache_flush(CObjRef memcache, int timestamp /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_flush(timestamp);
}

bool f_memcache_setoptimeout(CObjRef memcache, int timeoutms) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_setoptimeout(timeoutms);
}

int64_t f_memcache_get_server_status(CObjRef memcache, const String& host,
                                 int port /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_getserverstatus(host, port);
}

bool f_memcache_set_compress_threshold(CObjRef memcache, int threshold,
                                       double min_savings /* = 0.2 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_setcompressthreshold(threshold, min_savings);
}

Array f_memcache_get_stats(CObjRef memcache, const String& type /* = null_string */,
                           int slabid /* = 0 */, int limit /* = 100 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_getstats(type, slabid, limit);
}

Array f_memcache_get_extended_stats(CObjRef memcache,
                                    const String& type /* = null_string */,
                                    int slabid /* = 0 */,
                                    int limit /* = 100 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_getextendedstats(type, slabid, limit);
}

bool f_memcache_set_server_params(CObjRef memcache, const String& host,
                                 int port /* = 11211 */,
                                 int timeout /* = 0 */,
                                 int retry_interval /* = 0 */,
                                 bool status /* = true */,
                                 CVarRef failure_callback /* = null_variant */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_setserverparams(host, port, timeout, retry_interval,
                                         status, failure_callback);
}

bool f_memcache_add_server(CObjRef memcache, const String& host,
                           int port /* = 11211 */,
                           bool persistent /* = false */,
                           int weight /* = 0 */,
                           int timeout /* = 0 */,
                           int retry_interval /* = 0 */,
                           bool status /* = true */,
                           CVarRef failure_callback /* = null_variant */,
                           int timeoutms /* = 0 */) {
  c_Memcache *memcache_obj = memcache.getTyped<c_Memcache>();
  return memcache_obj->t_addserver(host, port, persistent, weight, timeout,
                                   retry_interval, status, failure_callback,
                                   timeoutms);
}


///////////////////////////////////////////////////////////////////////////////
}
