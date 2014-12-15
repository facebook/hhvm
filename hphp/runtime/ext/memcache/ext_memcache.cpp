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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/ext/libmemcached_portability.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/zend-string.h"
#include <vector>

// MMC values must match pecl-memcache for compatibility
#define MMC_SERIALIZED  0x0001
#define MMC_COMPRESSED  0x0002

#define MMC_TYPE_STRING 0x0000
#define MMC_TYPE_BOOL   0x0100
#define MMC_TYPE_LONG   0x0300
#define MMC_TYPE_DOUBLE 0x0700

#define MMC_TYPE_MASK   0x0F00

namespace HPHP {

const int64_t k_MEMCACHE_COMPRESSED = MMC_COMPRESSED;

static bool ini_on_update_hash_strategy(const std::string& value);
static bool ini_on_update_hash_function(const std::string& value);

struct MEMCACHEGlobals final {
  std::string hash_strategy;
  std::string hash_function;
};

static __thread MEMCACHEGlobals* s_memcache_globals;
#define MEMCACHEG(name) s_memcache_globals->name

const StaticString s_MemcacheData("MemcacheData");

class MemcacheData {
 public:
  memcached_st m_memcache;
  int m_compress_threshold;
  double m_min_compress_savings;

  MemcacheData(): m_memcache(), m_compress_threshold(0),
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
  };
  ~MemcacheData() {
    memcached_free(&m_memcache);
  };
};

static bool ini_on_update_hash_strategy(const std::string& value) {
  if (!strncasecmp(value.data(), "standard", sizeof("standard"))) {
    MEMCACHEG(hash_strategy) = "standard";
  } else if (!strncasecmp(value.data(), "consistent", sizeof("consistent"))) {
    MEMCACHEG(hash_strategy) = "consistent";
  }
  return false;
}

static bool ini_on_update_hash_function(const std::string& value) {
  if (!strncasecmp(value.data(), "crc32", sizeof("crc32"))) {
    MEMCACHEG(hash_function) = "crc32";
  } else if (!strncasecmp(value.data(), "fnv", sizeof("fnv"))) {
    MEMCACHEG(hash_function) = "fnv";
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// methods

static bool HHVM_METHOD(Memcache, connect, const String& host, int port /*= 0*/,
                                           int timeout /*= 0*/,
                                           int timeoutms /*= 0*/) {
  auto data = Native::data<MemcacheData>(this_);
  memcached_return_t ret;

  if (!host.empty() &&
      !strncmp(host.c_str(), "unix://", sizeof("unix://") - 1)) {
    const char *socket_path = host.substr(sizeof("unix://") - 1).c_str();
    ret = memcached_server_add_unix_socket(&data->m_memcache, socket_path);
  } else {
    ret = memcached_server_add(&data->m_memcache, host.c_str(), port);
  }

  return (ret == MEMCACHED_SUCCESS);
}

static uint32_t memcache_get_flag_for_type(const Variant& var) {
  switch (var.getType()) {
    case KindOfBoolean:
      return MMC_TYPE_BOOL;
    case KindOfInt64:
      return MMC_TYPE_LONG;
    case KindOfDouble:
      return MMC_TYPE_DOUBLE;

    case KindOfUninit:
    case KindOfNull:
    case KindOfStaticString:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      return MMC_TYPE_STRING;

    case KindOfClass:
      break;
  }
  not_reached();
}

static void memcache_set_type_from_flag(Variant& var, uint32_t flags) {
  switch (flags & MMC_TYPE_MASK) {
  case MMC_TYPE_BOOL:
    var = var.toBoolean();
    break;
  case MMC_TYPE_LONG:
    var = var.toInt64();
    break;
  case MMC_TYPE_DOUBLE:
    var = var.toDouble();
    break;
  }
}

static std::vector<char> memcache_prepare_for_storage(const MemcacheData* data,
                                                      const Variant& var,
                                                      int &flag) {
  String v;
  if (var.isString()) {
    v = var.toString();
  } else if (var.isNumeric() || var.isBoolean()) {
    flag &= ~MMC_COMPRESSED;
    v = var.toString();
  } else {
    flag |= MMC_SERIALIZED;
    v = f_serialize(var);
  }
  std::vector<char> payload;
  size_t value_len = v.length();

  if (!var.isNumeric() && !var.isBoolean() &&
    data->m_compress_threshold && value_len >= data->m_compress_threshold) {
    flag |= MMC_COMPRESSED;
  }
  if (flag & MMC_COMPRESSED) {
    size_t payload_len = compressBound(value_len);
    payload.resize(payload_len);
    if (compress((Bytef*)payload.data(), &payload_len,
                 (const Bytef*)v.data(), value_len) == Z_OK) {
      payload.resize(payload_len);
      if (payload_len >= value_len * (1 - data->m_min_compress_savings)) {
        flag &= ~MMC_COMPRESSED;
      }
    } else {
      flag &= ~MMC_COMPRESSED;
      raise_warning("could not compress value");
    }
  }
  if (!(flag & MMC_COMPRESSED)) {
    payload.resize(0);
    payload.insert(payload.end(), v.data(), v.data() + value_len);
   }
  flag |= memcache_get_flag_for_type(var);

  return payload;
}

static String memcache_prepare_key(const String& var) {
  auto data = var.get()->mutableData();
  for (int i = 0; i < var.length(); i++) {
    // This is a stupid encoding since it causes collisions but it matches php5
    if (data[i] <= ' ') {
      data[i] = '_';
    }
  }
  return data;
}

static Variant unserialize_if_serialized(const char *payload,
                                         size_t payload_len,
                                         uint32_t flags) {
  Variant ret = uninit_null();
  if (flags & MMC_SERIALIZED) {
    ret = unserialize_from_buffer(payload, payload_len);
  } else {
    if (payload_len == 0) {
      ret = String("");
    } else {
      ret = String(payload, payload_len, CopyString);
    }
  }
  return ret;
 }

static Variant memcache_fetch_from_storage(const char *payload,
                                           size_t payload_len,
                                           uint32_t flags) {
  Variant ret = uninit_null();

  if (flags & MMC_COMPRESSED) {
    bool done = false;
    std::vector<char> buffer;
    size_t buffer_len;
    for (int factor = 1; !done && factor <= 16; ++factor) {
      if (payload_len >=
          std::numeric_limits<unsigned long>::max() / (1 << factor)) {
        break;
      }
      buffer_len = payload_len * (1 << factor) + 1;
      buffer.resize(buffer_len);
      if (uncompress((Bytef*)buffer.data(), &buffer_len,
                     (const Bytef*)payload, (uLong)payload_len) == Z_OK) {
        done = true;
      }
    }
    if (!done) {
      raise_warning("could not uncompress value");
      return init_null();
    }
    ret = unserialize_if_serialized(buffer.data(), buffer_len, flags);
  } else {
    ret = unserialize_if_serialized(payload, payload_len, flags);
  }
  memcache_set_type_from_flag(ret, flags);

  return ret;
}

static bool HHVM_METHOD(Memcache, add, const String& key, const Variant& var,
                                       int flag /*= 0*/, int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  std::vector<char> serialized = memcache_prepare_for_storage(data, var, flag);

  String serializedKey = memcache_prepare_key(key);
  memcached_return_t ret = memcached_add(&data->m_memcache,
                                        serializedKey.c_str(),
                                        serializedKey.length(),
                                        serialized.data(),
                                        serialized.size(),
                                        expire, flag);

  return (ret == MEMCACHED_SUCCESS);
}

static bool HHVM_METHOD(Memcache, set, const String& key, const Variant& var,
                                       int flag /*= 0*/, int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  String serializedKey = memcache_prepare_key(key);
  std::vector<char> serializedVar =
    memcache_prepare_for_storage(data, var, flag);

  memcached_return_t ret = memcached_set(&data->m_memcache,
                                         serializedKey.c_str(),
                                         serializedKey.length(),
                                         serializedVar.data(),
                                         serializedVar.size(), expire, flag);

  if (ret == MEMCACHED_SUCCESS) {
    return true;
  }

  return false;
}

static bool HHVM_METHOD(Memcache, replace, const String& key,
                                           const Variant& var, int flag /*= 0*/,
                                           int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  String serializedKey = memcache_prepare_key(key);
  std::vector<char> serialized = memcache_prepare_for_storage(data, var, flag);

  memcached_return_t ret = memcached_replace(&data->m_memcache,
                                             serializedKey.c_str(),
                                             serializedKey.length(),
                                             serialized.data(),
                                             serialized.size(),
                                             expire, flag);
  return (ret == MEMCACHED_SUCCESS);
}

static Variant HHVM_METHOD(Memcache, get, const Variant& key,
                                          VRefParam flags /*= null*/) {
  auto data = Native::data<MemcacheData>(this_);
  if (key.is(KindOfArray)) {
    std::vector<const char *> real_keys;
    std::vector<size_t> key_len;
    Array keyArr = key.toArray();

    real_keys.reserve(keyArr.size());
    key_len.reserve(keyArr.size());

    for (ArrayIter iter(keyArr); iter; ++iter) {
      auto key = iter.second().toString();
      String serializedKey = memcache_prepare_key(key);
      char *k = new char[serializedKey.length()+1];
      std::strcpy(k, serializedKey.c_str());
      real_keys.push_back(k);
      key_len.push_back(serializedKey.length());
    }

    if (!real_keys.empty()) {
      const char *payload = NULL;
      size_t payload_len = 0;
      uint32_t flags = 0;
      const char *res_key = NULL;
      size_t res_key_len = 0;

      memcached_result_st result;

      memcached_return_t ret = memcached_mget(&data->m_memcache, &real_keys[0],
                                              &key_len[0], real_keys.size());
      memcached_result_create(&data->m_memcache, &result);
      Array return_val;

      while ((memcached_fetch_result(&data->m_memcache, &result, &ret))
             != nullptr) {
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
      for ( size_t i = 0 ; i < real_keys.size() ; i++ ) {
        delete [] real_keys[i];
      }

      return return_val;
    }
  } else {
    char *payload = NULL;
    size_t payload_len = 0;
    uint32_t flags = 0;

    memcached_return_t ret;
    String serializedKey = memcache_prepare_key(key.toString());

    if (serializedKey.length() == 0) {
      return false;
    }

    payload = memcached_get(&data->m_memcache, serializedKey.c_str(),
                            serializedKey.length(), &payload_len, &flags, &ret);

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

static bool HHVM_METHOD(Memcache, delete, const String& key,
                                          int expire /*= 0*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  String serializedKey = memcache_prepare_key(key);
  memcached_return_t ret = memcached_delete(&data->m_memcache,
                                            serializedKey.c_str(),
                                            serializedKey.length(),
                                            expire);
  return (ret == MEMCACHED_SUCCESS);
}

static Variant HHVM_METHOD(Memcache, increment, const String& key,
                                                int offset /*= 1*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  uint64_t value;
  String serializedKey = memcache_prepare_key(key);
  memcached_return_t ret = memcached_increment(&data->m_memcache,
                                               serializedKey.c_str(),
                                               serializedKey.length(), offset,
                                               &value);

  if (ret == MEMCACHED_SUCCESS) {
    return (int64_t)value;
  }

  return false;
}

static Variant HHVM_METHOD(Memcache, decrement, const String& key,
                                                int offset /*= 1*/) {
  if (key.empty()) {
    raise_warning("Key cannot be empty");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);
  uint64_t value;
  String serializedKey = memcache_prepare_key(key);
  memcached_return_t ret = memcached_decrement(&data->m_memcache,
                                               serializedKey.c_str(),
                                               serializedKey.length(), offset,
                                               &value);

  if (ret == MEMCACHED_SUCCESS) {
    return (int64_t)value;
  }

  return false;
}

static bool HHVM_METHOD(Memcache, close) {
  auto data = Native::data<MemcacheData>(this_);
  memcached_quit(&data->m_memcache);
  return true;
}

static Variant HHVM_METHOD(Memcache, getversion) {
  auto data = Native::data<MemcacheData>(this_);
  int server_count = memcached_server_count(&data->m_memcache);
  char version[16];
  int version_len = 0;

  if (memcached_version(&data->m_memcache) != MEMCACHED_SUCCESS) {
    return false;
  }

  for (int x = 0; x < server_count; x++) {
    LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
      memcached_server_instance_by_position(&data->m_memcache, x);
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

static bool HHVM_METHOD(Memcache, flush, int expire /*= 0*/) {
  auto data = Native::data<MemcacheData>(this_);
  return memcached_flush(&data->m_memcache, expire) == MEMCACHED_SUCCESS;
}

static bool HHVM_METHOD(Memcache, setcompressthreshold, int threshold,
                                        double min_savings /* = 0.2 */) {
  if (threshold < 0) {
    raise_warning("threshold must be a positive integer");
    return false;
  }

  if (min_savings < 0 || min_savings > 1) {
    raise_warning("min_savings must be a float in the 0..1 range");
    return false;
  }

  auto data = Native::data<MemcacheData>(this_);

  data->m_compress_threshold = threshold;
  data->m_min_compress_savings = min_savings;

  return true;
}

static Array memcache_build_stats(const memcached_st *ptr,
                                memcached_stat_st *memc_stat,
                                memcached_return_t *ret) {
  char **curr_key;
  char **stat_keys = memcached_stat_get_keys(const_cast<memcached_st*>(ptr),
                                             memc_stat, ret);

  if (*ret != MEMCACHED_SUCCESS) {
    if (stat_keys) {
      free(stat_keys);
    }
    return Array();
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


static Array HHVM_METHOD(Memcache, getstats,
                         const String& type /* = null_string */,
                         int slabid /* = 0 */, int limit /* = 100 */) {
  auto data = Native::data<MemcacheData>(this_);
  if (!memcached_server_count(&data->m_memcache)) {
    return Array();
  }

  char extra_args[30] = {0};

  if (slabid) {
    snprintf(extra_args, sizeof(extra_args), "%s %d %d", type.c_str(),
             slabid, limit);
  } else if (!type.empty()) {
    snprintf(extra_args, sizeof(extra_args), "%s", type.c_str());
  }

  LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
    memcached_server_instance_by_position(&data->m_memcache, 0);
  const char *hostname = LMCD_SERVER_HOSTNAME(instance);
  in_port_t port = LMCD_SERVER_PORT(instance);

  memcached_stat_st stats;
  if (memcached_stat_servername(&stats, extra_args, hostname,
                                port) != MEMCACHED_SUCCESS) {
    return Array();
  }

  memcached_return_t ret;
  return memcache_build_stats(&data->m_memcache, &stats, &ret);
}

static Array HHVM_METHOD(Memcache, getextendedstats,
                         const String& type /* = null_string */,
                         int slabid /* = 0 */, int limit /* = 100 */) {
  auto data = Native::data<MemcacheData>(this_);
  memcached_return_t ret;
  memcached_stat_st *stats;

  stats = memcached_stat(&data->m_memcache, nullptr, &ret);
  if (ret != MEMCACHED_SUCCESS) {
    return Array();
  }

  int server_count = memcached_server_count(&data->m_memcache);

  Array return_val;

  for (int server_id = 0; server_id < server_count; server_id++) {
    memcached_stat_st *stat;
    char stats_key[30] = {0};
    size_t key_len;

    LMCD_SERVER_POSITION_INSTANCE_TYPE instance =
      memcached_server_instance_by_position(&data->m_memcache, server_id);
    const char *hostname = LMCD_SERVER_HOSTNAME(instance);
    in_port_t port = LMCD_SERVER_PORT(instance);

    stat = stats + server_id;

    Array server_stats = memcache_build_stats(&data->m_memcache, stat, &ret);
    if (ret != MEMCACHED_SUCCESS) {
      continue;
    }

    key_len = snprintf(stats_key, sizeof(stats_key), "%s:%d", hostname, port);

    return_val.set(String(stats_key, key_len, CopyString), server_stats);
  }

  free(stats);
  return return_val;
}

static bool HHVM_METHOD(Memcache, addserver, const String& host,
                        int port /* = 11211 */,
                        bool persistent /* = false */,
                        int weight /* = 0 */, int timeout /* = 0 */,
                        int retry_interval /* = 0 */, bool status /* = true */,
                        const Variant& failure_callback /* = null_variant */,
                        int timeoutms /* = 0 */) {
  auto data = Native::data<MemcacheData>(this_);
  memcached_return_t ret;

  if (!host.empty() &&
      !strncmp(host.c_str(), "unix://", sizeof("unix://") - 1)) {
    const char *socket_path = host.substr(sizeof("unix://") - 1).c_str();
    ret = memcached_server_add_unix_socket_with_weight(&data->m_memcache,
                                                       socket_path, weight);
  } else {
    ret = memcached_server_add_with_weight(&data->m_memcache, host.c_str(),
                                           port, weight);
  }

  if (ret == MEMCACHED_SUCCESS) {
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
const StaticString s_MEMCACHE_COMPRESSED("MEMCACHE_COMPRESSED");
const StaticString s_MEMCACHE_HAVE_SESSION("MEMCACHE_HAVE_SESSION");

class MemcacheExtension : public Extension {
  public:
    MemcacheExtension() : Extension("memcache", "3.0.8") {};
    void threadInit() override {
      // TODO: t5226715 We shouldn't need to check s_defaultLocale here,
      // but right now this is called for every request.
      if (s_memcache_globals) return;
      s_memcache_globals = new MEMCACHEGlobals;
      IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                       "memcache.hash_strategy", "standard",
                       IniSetting::SetAndGet<std::string>(
                         ini_on_update_hash_strategy,
                         nullptr
                       ),
                       &MEMCACHEG(hash_strategy));
      IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                       "memcache.hash_function", "crc32",
                       IniSetting::SetAndGet<std::string>(
                         ini_on_update_hash_function,
                         nullptr
                       ),
                       &MEMCACHEG(hash_function));
    }
    void threadShutdown() override {
      delete s_memcache_globals;
      s_memcache_globals = nullptr;
    }

    virtual void moduleInit() {
      Native::registerConstant<KindOfInt64>(
        s_MEMCACHE_COMPRESSED.get(), k_MEMCACHE_COMPRESSED
      );
      Native::registerConstant<KindOfBoolean>(
        s_MEMCACHE_HAVE_SESSION.get(), true
      );
      HHVM_ME(Memcache, connect);
      HHVM_ME(Memcache, add);
      HHVM_ME(Memcache, set);
      HHVM_ME(Memcache, replace);
      HHVM_ME(Memcache, get);
      HHVM_ME(Memcache, delete);
      HHVM_ME(Memcache, increment);
      HHVM_ME(Memcache, decrement);
      HHVM_ME(Memcache, close);
      HHVM_ME(Memcache, getversion);
      HHVM_ME(Memcache, flush);
      HHVM_ME(Memcache, setcompressthreshold);
      HHVM_ME(Memcache, getstats);
      HHVM_ME(Memcache, getextendedstats);
      HHVM_ME(Memcache, addserver);

      Native::registerNativeDataInfo<MemcacheData>(s_MemcacheData.get());

      loadSystemlib();
    }
} s_memcache_extension;;

}
