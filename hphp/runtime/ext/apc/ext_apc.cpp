/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/apc/ext_apc.h"

#include <fstream>

#ifndef _MSC_VER
#include <dlfcn.h>
#endif
#include <memory>
#include <set>
#include <vector>
#include <stdexcept>
#include <type_traits>

#include <folly/portability/SysTime.h>

#include "hphp/util/alloc.h"
#include "hphp/util/async-job.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/hdf.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/fb/ext_fb.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/upload.h"

using HPHP::ScopedMem;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// This symbol intentionally not static to expose to debugger.
std::aligned_storage<
  sizeof(ConcurrentTableSharedStore),
  alignof(ConcurrentTableSharedStore)
>::type s_apc_storage;

namespace {

using UserAPCCache = folly::AtomicHashMap<uid_t, ConcurrentTableSharedStore*>;

std::aligned_storage<
  sizeof(UserAPCCache),
  alignof(UserAPCCache)
>::type s_user_apc_storage;

UserAPCCache& apc_store_local() {
  void* vpUserStore = &s_user_apc_storage;
  return *static_cast<UserAPCCache*>(vpUserStore);
}

ConcurrentTableSharedStore& apc_store_local(uid_t uid) {
  auto& cache = apc_store_local();
  auto iter = cache.find(uid);
  if (iter != cache.end()) return *(iter->second);
  auto table = new ConcurrentTableSharedStore;
  auto res = cache.insert(uid, table);
  if (!res.second) delete table;
  return *res.first->second;
}

ConcurrentTableSharedStore& apc_store() {
  if (UNLIKELY(!RuntimeOption::RepoAuthoritative &&
               RuntimeOption::EvalUnixServerQuarantineApc)) {
    if (auto uc = get_cli_ucred()) {
      return apc_store_local(uc->uid);
    }
  }
  void* vpStore = &s_apc_storage;
  return *static_cast<ConcurrentTableSharedStore*>(vpStore);
}

bool isKeyInvalid(const String &key) {
  // T39154441 - check if invalid chars exist
  return key.find('\0') != -1;
}

}

//////////////////////////////////////////////////////////////////////

void initialize_apc() {
  APCStats::Create();
  // Note: we never destruct APC, currently.
  void* vpStore = &s_apc_storage;
  new (vpStore) ConcurrentTableSharedStore;

  if (UNLIKELY(!RuntimeOption::RepoAuthoritative &&
               RuntimeOption::EvalUnixServerQuarantineApc)) {
    new (&s_user_apc_storage) UserAPCCache(10);
  }
}

//////////////////////////////////////////////////////////////////////

const StaticString s_internal_preload("__apc_internal_preload");

typedef ConcurrentTableSharedStore::DumpMode DumpMode;

void apcExtension::moduleLoad(const IniSetting::Map& ini, Hdf config) {
  Config::Bind(Enable, ini, config, "Server.APC.EnableApc", true);
  Config::Bind(EnableApcSerialize, ini, config, "Server.APC.EnableApcSerialize",
               true);
  Config::Bind(ExpireOnSets, ini, config, "Server.APC.ExpireOnSets");
  Config::Bind(PurgeInterval, ini, config, "Server.APC.PurgeIntervalSeconds",
               PurgeInterval);
  Config::Bind(AllowObj, ini, config, "Server.APC.AllowObject");
  Config::Bind(TTLLimit, ini, config, "Server.APC.TTLLimit", -1);
  Config::Bind(DeferredExpiration, ini, config,
               "Server.APC.DeferredExpiration", DeferredExpiration);
  // Any TTL > TTLMaxFinite will be made infinite. NB: Applied *after* TTLLimit.
  Config::Bind(TTLMaxFinite, ini, config, "Server.APC.TTLMaxFinite",
               std::numeric_limits<int64_t>::max());
  Config::Bind(HotPrefix, ini, config, "Server.APC.HotPrefix");
  Config::Bind(SerializePrefix, ini, config, "Server.APC.SerializePrefix");
  Config::Bind(HotSize, ini, config, "Server.APC.HotSize", 30000);
  Config::Bind(HotLoadFactor, ini, config, "Server.APC.HotLoadFactor", 0.5);
  Config::Bind(HotKeyAllocLow, ini, config, "Server.APC.HotKeyAllocLow", false);
  Config::Bind(HotMapAllocLow, ini, config, "Server.APC.HotMapAllocLow", false);

#ifdef NO_M_DATA
  Config::Bind(UseUncounted, ini, config, "Server.APC.MemModelTreadmill", true);
#else
  Config::Bind(UseUncounted, ini, config, "Server.APC.MemModelTreadmill",
               RuntimeOption::ServerExecutionMode());
#endif
  Config::Bind(ShareUncounted, ini, config, "Server.APC.ShareUncounted", true);
  if (!UseUncounted && ShareUncounted) ShareUncounted = false;

  Config::Bind(SizedSampleBytes, ini, config, "Server.APC.SizedSampleBytes", 0);

  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.enabled", &Enable);
  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.stat",
                   RuntimeOption::RepoAuthoritative ? "0" : "1", &Stat);
  IniSetting::Bind(this, IniSetting::PHP_INI_SYSTEM, "apc.enable_cli",
                   &EnableCLI);
}

void apcExtension::moduleInit() {
#ifdef NO_M_DATA
  if (!UseUncounted) {
    Logger::Error("Server.APC.MemModelTreadmill=false ignored in lowptr build");
    UseUncounted = true;
  }
#endif // NO_M_DATA
  apc_store().init();

  HHVM_RC_INT(APC_ITER_TYPE, 0x1);
  HHVM_RC_INT(APC_ITER_KEY, 0x2);
  HHVM_RC_INT(APC_ITER_FILENAME, 0x4);
  HHVM_RC_INT(APC_ITER_DEVICE, 0x8);
  HHVM_RC_INT(APC_ITER_INODE, 0x10);
  HHVM_RC_INT(APC_ITER_VALUE, 0x20);
  HHVM_RC_INT(APC_ITER_MD5, 0x40);
  HHVM_RC_INT(APC_ITER_NUM_HITS, 0x80);
  HHVM_RC_INT(APC_ITER_MTIME, 0x100);
  HHVM_RC_INT(APC_ITER_CTIME, 0x200);
  HHVM_RC_INT(APC_ITER_DTIME, 0x400);
  HHVM_RC_INT(APC_ITER_ATIME, 0x800);
  HHVM_RC_INT(APC_ITER_REFCOUNT, 0x1000);
  HHVM_RC_INT(APC_ITER_MEM_SIZE, 0x2000);
  HHVM_RC_INT(APC_ITER_TTL, 0x4000);
  HHVM_RC_INT(APC_ITER_NONE, 0x0);
  HHVM_RC_INT(APC_ITER_ALL, 0xFFFFFFFFFF);
  HHVM_RC_INT(APC_LIST_ACTIVE, 1);
  HHVM_RC_INT(APC_LIST_DELETED, 2);

  HHVM_FE(apc_add);
  HHVM_FE(apc_store);
  HHVM_FE(apc_fetch);
  HHVM_FE(apc_delete);
  HHVM_FE(apc_clear_cache);
  HHVM_FE(apc_inc);
  HHVM_FE(apc_dec);
  HHVM_FE(apc_cas);
  HHVM_FE(apc_exists);
  HHVM_FE(apc_size);
  HHVM_FE(apc_extend_ttl);
  HHVM_FE(apc_cache_info);
  loadSystemlib();
}

void apcExtension::moduleShutdown() {
}

void apcExtension::requestShutdown() {
  apc_store().purgeExpired();
}

std::string apcExtension::serialize() {
  std::ostringstream oss;
  apc_store().dumpKeysWithPrefixes(oss, SerializePrefix);
  return oss.str();
}

void apcExtension::deserialize(std::string data) {
  auto sd = StringData::MakeUncounted(data);
  data.clear();
  apc_store().set(s_internal_preload, Variant{sd}, 0, 0);
  DecRefUncountedString(sd); // APC did an uncounted inc-ref
}

void apcExtension::purgeDeferred(req::vector<StringData*>&& keys) {
  apc_store().purgeDeferred(std::move(keys));
}

bool apcExtension::Enable = true;
bool apcExtension::EnableApcSerialize = true;
bool apcExtension::AllowObj = false;
bool apcExtension::ExpireOnSets = false;
int apcExtension::PurgeInterval = 1;
int apcExtension::TTLLimit = -1;
int64_t apcExtension::TTLMaxFinite = std::numeric_limits<int64_t>::max();
int apcExtension::HotSize = 30000;
double apcExtension::HotLoadFactor = 0.5;
std::vector<std::string> apcExtension::HotPrefix;
std::vector<std::string> apcExtension::SerializePrefix;
bool apcExtension::HotKeyAllocLow = false;
bool apcExtension::HotMapAllocLow = false;
#ifdef NO_M_DATA
bool apcExtension::UseUncounted = true;
#else
bool apcExtension::UseUncounted = false;
#endif
bool apcExtension::ShareUncounted = true;
bool apcExtension::Stat = true;
// Different from zend default but matches what we've been returning for years
bool apcExtension::EnableCLI = true;
bool apcExtension::DeferredExpiration = true;
uint32_t apcExtension::SizedSampleBytes = 0;

static apcExtension s_apc_extension;

Variant HHVM_FUNCTION(apc_store,
                      const Variant& key_or_array,
                      const Variant& var /* = null */,
                      int64_t ttl /* = 0 */,
                      int64_t bump_ttl /* = 0 */) {
  if (!apcExtension::Enable) return Variant(false);

  if (key_or_array.isArray()) {
    Array valuesArr = key_or_array.toArray();

    for (ArrayIter iter(valuesArr); iter; ++iter) {
      Variant key = iter.first();
      if (!key.isString()) {
        raise_invalid_argument_warning("apc key: (not a string)");
        return Variant(false);
      }
      Variant v = iter.second();

      auto const& strKey = key.asCStrRef();
      if (strKey.empty()) {
        raise_invalid_argument_warning("apc key: (empty string)");
        return false;
      }
      if (isKeyInvalid(strKey)) {
        raise_invalid_argument_warning("apc key: (contains invalid characters)");
        return Variant(false);
      }
      apc_store().set(strKey, v, ttl, bump_ttl);
      if (RuntimeOption::EnableAPCStats) {
        ServerStats::Log("apc.write", 1);
      }
    }
    return Variant(ArrayData::CreateDict());
  }

  if (!key_or_array.isString()) {
    raise_invalid_argument_warning("apc key: (not a string)");
    return Variant(false);
  }
  String strKey = key_or_array.toString();

  if (strKey.empty()) {
    raise_invalid_argument_warning("apc key: (empty string)");
    return false;
  }
  if (isKeyInvalid(strKey)) {
    raise_invalid_argument_warning("apc key: (contains invalid characters)");
    return Variant(false);
  }
  apc_store().set(strKey, var, ttl, bump_ttl);
  if (RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.write", 1);
  }
  return Variant(true);
}

Variant HHVM_FUNCTION(apc_add,
                      const Variant& key_or_array,
                      const Variant& var /* = null */,
                      int64_t ttl /* = 0 */,
                      int64_t bump_ttl /* = 0 */) {
  if (!apcExtension::Enable) return false;

  if (key_or_array.isArray()) {
    auto valuesArr = key_or_array.asCArrRef();

    // errors stores all keys corresponding to entries that could not be cached
    DArrayInit errors(valuesArr.size());

    for (ArrayIter iter(valuesArr); iter; ++iter) {
      Variant key = iter.first();
      if (!key.isString()) {
        raise_invalid_argument_warning("apc key: (not a string)");
        return false;
      }
      Variant v = iter.second();

      auto const& strKey = key.asCStrRef();
      if (strKey.empty()) {
        raise_invalid_argument_warning("apc key: (empty string)");
        return false;
      }
      if (isKeyInvalid(strKey)) {
        raise_invalid_argument_warning("apc key: (contains invalid characters)");
        return false;
      }

      if (!apc_store().add(strKey, v, ttl, bump_ttl)) {
        errors.add(strKey, -1);
      }
    }
    return errors.toVariant();
  }

  if (!key_or_array.isString()) {
    raise_invalid_argument_warning("apc key: (not a string)");
    return false;
  }
  auto strKey = key_or_array.asCStrRef();
  if (strKey.empty()) {
    raise_invalid_argument_warning("apc key: (empty string)");
    return false;
  }
  if (isKeyInvalid(strKey)) {
    raise_invalid_argument_warning("apc key: (contains invalid characters)");
    return false;
  }
  if (RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.write", 1);
  }
  return apc_store().add(strKey, var, ttl, bump_ttl);
}

bool HHVM_FUNCTION(apc_extend_ttl, const String& key, int64_t new_ttl) {
  if (!apcExtension::Enable) return false;

  if (new_ttl < 0) return false;
  return apc_store().extendTTL(key, new_ttl);
}

TypedValue HHVM_FUNCTION(apc_fetch, const Variant& key, bool& success) {
  if (!apcExtension::Enable) return make_tv<KindOfBoolean>(false);

  Variant v;

  if (key.isArray()) {
    bool tmp = false;
    auto keys = key.asCArrRef();
    DArrayInit init(keys.size());
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        raise_invalid_argument_warning("apc key: (not a string)");
        return make_tv<KindOfBoolean>(false);
      }
      auto strKey = k.asCStrRef();
      if (apc_store().get(strKey, v)) {
        if (RuntimeOption::EnableAPCStats) {
          ServerStats::Log("apc.hit", 1);
        }
        tmp = true;
        init.set(strKey, v);
      }
    }
    success = tmp;
    return tvReturn(init.toVariant());
  }

  if (apc_store().get(key.toString(), v)) {
    success = true;
    if (RuntimeOption::EnableAPCStats) {
      ServerStats::Log("apc.hit", 1);
    }
  } else {
    success = false;
    v = false;
    if (RuntimeOption::EnableAPCStats) {
      ServerStats::Log("apc.miss", 1);
    }
  }
  return tvReturn(std::move(v));
}

Variant HHVM_FUNCTION(apc_delete,
                      const Variant& key) {
  if (!apcExtension::Enable) return false;

  if (key.isArray()) {
    auto keys = key.asCArrRef();
    VArrayInit init(keys.size());
    for (ArrayIter iter(keys); iter; ++iter) {
      Variant k = iter.second();
      if (!k.isString()) {
        raise_warning("apc key is not a string");
        init.append(k);
      } else if (!apc_store().eraseKey(k.asCStrRef())) {
        init.append(k);
      }
    }
    return init.toVariant();
  } else if (key.is(KindOfObject)) {
    raise_error(
      "apc_delete(): apc_delete argument may not be an object"
    );
    return false;
  }

  return apc_store().eraseKey(key.toString());
}

bool HHVM_FUNCTION(apc_clear_cache, const String& /*cache_type*/ /* = "" */) {
  if (!apcExtension::Enable) return false;
  return apc_store().clear();
}

Variant HHVM_FUNCTION(apc_inc,
                      const String& key,
                      int64_t step,
                      bool& success) {
  if (!apcExtension::Enable) return false;

  bool found = false;
  int64_t newValue = apc_store().inc(key, step, found);
  success = found;
  if (!found) return false;
  return newValue;
}

Variant HHVM_FUNCTION(apc_dec,
                      const String& key,
                      int64_t step,
                      bool& success) {
  if (!apcExtension::Enable) return false;

  bool found = false;
  int64_t newValue = apc_store().inc(key, -step, found);
  success = found;
  if (!found) return false;
  return newValue;
}

bool HHVM_FUNCTION(apc_cas,
                   const String& key,
                   int64_t old_cas,
                   int64_t new_cas) {
  if (!apcExtension::Enable) return false;
  return apc_store().cas(key, old_cas, new_cas);
}

Variant HHVM_FUNCTION(apc_exists,
                      const Variant& key) {
  if (!apcExtension::Enable) return false;

  if (!key.isArray()) return apc_store().exists(key.toString());
  bool failed = false;
  auto keys = key.toArray();
  VArrayInit init(keys.size());
  IterateV(keys.get(), [&](TypedValue k) {
    if (!isStringType(type(k))) {
      raise_invalid_argument_warning("apc key: (not a string)");
      failed = true;
      return true;
    }
    auto strKey = String{val(k).pstr};
    if (apc_store().exists(strKey)) {
      init.append(strKey);
    }
    return false;
  });
  if  (failed) return false;
  return init.toVariant();
}

TypedValue HHVM_FUNCTION(apc_size, const String& key) {
  if (!apcExtension::Enable) return make_tv<KindOfNull>();

  bool found = false;
  int64_t size = apc_store().size(key, found);

  return found ? make_tv<KindOfInt64>(size) : make_tv<KindOfNull>();
}

const StaticString s_user("user");
const StaticString s_start_time("start_time");
const StaticString s_ttl("ttl");
const StaticString s_cache_list("cache_list");
const StaticString s_info("info");
const StaticString s_in_memory("in_memory");
const StaticString s_mem_size("mem_size");
const StaticString s_type("type");
const StaticString s_c_time("creation_time");
const StaticString s_max_ttl("max_ttl");
const StaticString s_bump_ttl("bump_ttl");
const StaticString s_in_hotcache("in_hotcache");

// This is a guess to the size of the info array. It is significantly
// bigger than what we need but hard to control all the info that we
// may want to add here.
// Try to keep it such that we do not have to resize the array
const uint32_t kCacheInfoSize = 40;
// Number of elements in the entry array
const int32_t kEntryInfoSize = 10;

Array HHVM_FUNCTION(
  apc_cache_info,
  const String& cache_type,
  bool limited /* = false */) {

  DArrayInit info(kCacheInfoSize);
  info.add(s_start_time, start_time());
  if (cache_type.size() != 0 && !cache_type.same(s_user)) {
    return info.toArray();
  }

  info.add(s_ttl, apcExtension::TTLLimit);

  std::map<const StringData*, int64_t> stats;
  APCStats::getAPCStats().collectStats(stats);
  for (auto const& stat : stats) {
    info.add(StrNR{stat.first}, make_tv<KindOfInt64>(stat.second));
  }
  if (!limited) {
    auto const entries = apc_store().getEntriesInfo();
    VArrayInit ents(entries.size());
    for (auto& entry : entries) {
      DArrayInit ent(kEntryInfoSize);
      ent.add(s_info,
              Variant::attach(StringData::Make(entry.key.c_str())));
      ent.add(s_in_memory, 1);
      ent.add(s_ttl, entry.ttl);
      ent.add(s_mem_size, entry.size);
      ent.add(s_type, static_cast<int64_t>(entry.type));
      ent.add(s_c_time, entry.c_time);
      ent.add(s_max_ttl, entry.maxTTL);
      ent.add(s_bump_ttl, entry.bumpTTL);
      ent.add(s_in_hotcache, entry.inHotCache);
      ents.append(ent.toArray());
    }
    info.add(s_cache_list, ents.toArray(), false);
  }
  return info.toArray();
}

///////////////////////////////////////////////////////////////////////////////

static double my_time() {
  struct timeval a;
  double t;
  gettimeofday(&a, nullptr);
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

int apc_rfc1867_progress(apc_rfc1867_data* rfc1867ApcData, unsigned int event,
                         void* event_data, void** /*extra*/) {
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
      assertx(false); // TODO: support percentage
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
      DArrayInit track(6);
      track.set(s_total, rfc1867ApcData->content_length);
      track.set(s_current, rfc1867ApcData->bytes_processed);
      track.set(s_filename, rfc1867ApcData->filename);
      track.set(s_name, rfc1867ApcData->name);
      track.set(s_done, 0);
      track.set(s_start_time, rfc1867ApcData->start_time);
      HHVM_FN(apc_store)(rfc1867ApcData->tracking_key, track.toVariant(), 3600, 0);
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
        if (apc_store().get(rfc1867ApcData->tracking_key, v)) {
          if (v.isArray()) {
            DArrayInit track(6);
            track.set(s_total, rfc1867ApcData->content_length);
            track.set(s_current, rfc1867ApcData->bytes_processed);
            track.set(s_filename, rfc1867ApcData->filename);
            track.set(s_name, rfc1867ApcData->name);
            track.set(s_done, 0);
            track.set(s_start_time, rfc1867ApcData->start_time);
            HHVM_FN(apc_store)(rfc1867ApcData->tracking_key, track.toVariant(), 3600, 0);
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
      DArrayInit track(8);
      track.set(s_total, rfc1867ApcData->content_length);
      track.set(s_current, rfc1867ApcData->bytes_processed);
      track.set(s_filename, rfc1867ApcData->filename);
      track.set(s_name, rfc1867ApcData->name);
      track.set(s_temp_filename, rfc1867ApcData->temp_filename);
      track.set(s_cancel_upload, rfc1867ApcData->cancel_upload);
      track.set(s_done, 0);
      track.set(s_start_time, rfc1867ApcData->start_time);
      HHVM_FN(apc_store)(rfc1867ApcData->tracking_key, track.toVariant(), 3600, 0);
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
        DArrayInit track(8);
        track.set(s_total, rfc1867ApcData->content_length);
        track.set(s_current, rfc1867ApcData->bytes_processed);
        track.set(s_rate, rfc1867ApcData->rate);
        track.set(s_filename, rfc1867ApcData->filename);
        track.set(s_name, rfc1867ApcData->name);
        track.set(s_cancel_upload, rfc1867ApcData->cancel_upload);
        track.set(s_done, 1);
        track.set(s_start_time, rfc1867ApcData->start_time);
        HHVM_FN(apc_store)(rfc1867ApcData->tracking_key, track.toVariant(), 3600, 0);
      }
    }
    break;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// apc serialization

String apc_serialize(const_variant_ref value) {
  auto const enableApcSerialize = apcExtension::EnableApcSerialize;
  VariableSerializer::Type sType =
    enableApcSerialize ?
      VariableSerializer::Type::APCSerialize :
      VariableSerializer::Type::Internal;
  VariableSerializer vs(sType);
  return vs.serialize(value, true);
}

Variant apc_unserialize(const char* data, int len) {
  VariableUnserializer::Type sType =
    apcExtension::EnableApcSerialize ?
      VariableUnserializer::Type::APCSerialize :
      VariableUnserializer::Type::Internal;
  return unserialize_ex(data, len, sType);
}

String apc_reserialize(const String& str) {
  if (str.empty() ||
      !apcExtension::EnableApcSerialize) return str;

  VariableUnserializer uns(str.data(), str.size(),
                           VariableUnserializer::Type::APCSerialize);
  StringBuffer buf;
  uns.reserialize(buf);

  return buf.detach();
}

///////////////////////////////////////////////////////////////////////////////
// debugging support

bool apc_dump(const char *filename, bool keyOnly, bool metaDump) {
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
    mode = DumpMode::KeyOnly;
  } else if (metaDump) {
    mode = DumpMode::KeyAndMeta;
  } else {
    mode = DumpMode::KeyAndValue;
  }

  apc_store().dump(out, mode);
  out.close();
  return true;
}

bool apc_dump_prefix(const char *filename,
                     const std::string &prefix,
                     uint32_t count) {
  std::ofstream out(filename);
  if (out.fail()) {
    return false;
  }
  SCOPE_EXIT { out.close(); };

  apc_store().dumpPrefix(out, prefix, count);
  return true;
}

bool apc_get_random_entries(std::ostream &out, uint32_t count) {
  apc_store().dumpRandomKeys(out, count);
  return true;
}

// skewed sampling, so that bigger ones are more likely to be sampled.
void apc_sample_by_size() {
  if (apcExtension::SizedSampleBytes == 0) return;
  if (!StructuredLog::enabled()) return;
  auto entries =
    apc_store().sampleEntriesInfoBySize(apcExtension::SizedSampleBytes);
  StructuredLogEntry sample;
  for (auto& entry : entries) {
    sample.setStr("key", entry.key);
    sample.setInt("in_mem", 1);
    sample.setInt("ttl", entry.ttl);
    sample.setInt("size", entry.size);
    StructuredLog::log("apc_samples", sample);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
