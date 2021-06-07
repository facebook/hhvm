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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include <set>
#include <vector>
#include "hphp/runtime/server/upload.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct apcExtension final : Extension {
  apcExtension() : Extension("apc", "4.0.2") {}

  static bool Enable;
  static bool EnableApcSerialize;
  static bool ExpireOnSets;
  static bool AllowObj;
  static int PurgeInterval;
  static int TTLLimit;
  static int64_t TTLMaxFinite;
  static std::vector<std::string> HotPrefix;
  static std::vector<std::string> SerializePrefix;
  static int HotSize;
  static double HotLoadFactor;
  static bool HotKeyAllocLow;
  static bool HotMapAllocLow;
  static bool UseUncounted;
  static bool ShareUncounted;
  static bool Stat;
  static bool EnableCLI;
  static bool DeferredExpiration;
  static uint32_t SizedSampleBytes;

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override;
  void moduleInit() override;
  void moduleShutdown() override;
  bool moduleEnabled() const override { return Enable; }

  void requestShutdown() override;

  std::string serialize() override;
  void deserialize(std::string data) override;

  static void purgeDeferred(req::vector<StringData*>&&);
};

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(apc_add,
                      const Variant& key_or_array,
                      const Variant& var = uninit_variant,
                      int64_t ttl = 0,
                      int64_t bump_ttl = 0);
Variant HHVM_FUNCTION(apc_store,
                      const Variant& key_or_array,
                      const Variant& var = uninit_variant,
                      int64_t ttl = 0,
                      int64_t bump_ttl = 0);
TypedValue HHVM_FUNCTION(apc_fetch, const Variant& key, bool& success);
Variant HHVM_FUNCTION(apc_delete,
                      const Variant& key);
bool HHVM_FUNCTION(apc_clear_cache,
                   const String& cache_type = "");
Variant HHVM_FUNCTION(apc_inc,
                      const String& key,
                      int64_t step,
                      bool& success);
Variant HHVM_FUNCTION(apc_dec,
                      const String& key,
                      int64_t step,
                      bool& success);
bool HHVM_FUNCTION(apc_cas,
                   const String& key,
                   int64_t old_cas,
                   int64_t new_cas);
Variant HHVM_FUNCTION(apc_exists,
                      const Variant& key);
bool HHVM_FUNCTION(apc_extend_ttl,
                   const String& key,
                   int64_t new_ttl);
TypedValue HHVM_FUNCTION(apc_size, const String& key);


///////////////////////////////////////////////////////////////////////////////

Array HHVM_FUNCTION(apc_cache_info,
                    const String& cache_type /* = "" */,
                    bool limited /* = false */);

///////////////////////////////////////////////////////////////////////////////
// loading APC from archive files

struct apc_rfc1867_data {
  std::string tracking_key;
  int64_t content_length;
  std::string filename;
  std::string name;
  std::string temp_filename;
  int cancel_upload;
  double start_time;
  int64_t bytes_processed;
  int64_t prev_bytes_processed;
  int update_freq;
  double rate;
};

// file uploading progress support
int apc_rfc1867_progress(apc_rfc1867_data *rfc1867ApcData,
                         unsigned int event, void *event_data, void **extra);

void const_load_impl(struct cache_info *info,
                     const char **int_keys, long long *int_values,
                     const char **char_keys, char *char_values,
                     const char **strings, const char **objects,
                     const char **thrifts, const char **others);

void const_load_impl_compressed(
  struct cache_info *info,
  int *int_lens, const char *int_keys, long long *int_values,
  int *char_lens, const char *char_keys, char *char_values,
  int *string_lens, const char *strings,
  int *object_lens, const char *objects,
  int *thrift_lens, const char *thrifts,
  int *other_lens, const char *others);

static_assert(sizeof(int64_t) == sizeof(long long),
              "Must be able to cast an int64* to a long long*");

///////////////////////////////////////////////////////////////////////////////
// apc serialization

String apc_serialize(const_variant_ref value);
inline String apc_serialize(const Variant& var) {
  return apc_serialize(const_variant_ref{var});
}
Variant apc_unserialize(const char* data, int len);
String apc_reserialize(const String& str);

///////////////////////////////////////////////////////////////////////////////
// debugging support
bool apc_dump(const char *filename, bool keyOnly, bool metaDump);
bool apc_dump_prefix(const char *filename,
                     const std::string &prefix,
                     uint32_t count);
size_t get_const_map_size();
bool apc_get_random_entries(std::ostream &out, uint32_t count);
void apc_sample_by_size();

///////////////////////////////////////////////////////////////////////////////
}
