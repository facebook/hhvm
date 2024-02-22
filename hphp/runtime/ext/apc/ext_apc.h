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
#include "hphp/runtime/base/concurrent-shared-store.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct apcExtension final : Extension {
  apcExtension() : Extension("apc", "4.0.2", NO_ONCALL_YET) {}

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
  static uint32_t SizedSampleBytes;

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override;
  void moduleInit() override;
  void moduleRegisterNative() override;
  void moduleShutdown() override;
  bool moduleEnabled() const override { return Enable; }

  void requestShutdown() override;

  std::string serialize() override;
  void deserialize(std::string data) override;

  static void purgeDeferred(req::vector<StringData*>&&);
};

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(apc_store,
                      const Variant& key_or_array,
                      const Variant& var = uninit_variant,
                      int64_t ttl = 0,
                      int64_t bump_ttl = 0);
TypedValue HHVM_FUNCTION(apc_fetch, const Variant& key, bool& success);

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

static_assert(sizeof(int64_t) == sizeof(long long),
              "Must be able to cast an int64* to a long long*");

///////////////////////////////////////////////////////////////////////////////
// apc serialization

String apc_serialize(const_variant_ref value, bool pure);
inline String apc_serialize(const Variant& var, bool pure) {
  return apc_serialize(const_variant_ref{var}, pure);
}
Variant apc_unserialize(const char* data, int len, bool pure);

///////////////////////////////////////////////////////////////////////////////
// debugging support
typedef hphp_fast_string_set APCKeySet;
typedef hphp_fast_string_map<HPHP::Optional<std::string>> APCEntryMap;
APCKeySet apc_debug_get_keys();
APCEntryMap apc_debug_get_all_entries();
APCEntryMap apc_debug_get_entries_with_prefix(const std::string& prefix,
                                  HPHP::Optional<uint32_t> count);
std::vector<EntryInfo> apc_debug_get_all_entry_info();
std::vector<EntryInfo> apc_debug_get_random_entry_info(uint32_t count);

bool apc_dump(const char *filename, bool keyOnly, bool metaDump);
bool apc_dump_prefix(const char *filename,
                     const std::string &prefix,
                     uint32_t count);
size_t get_const_map_size();
bool apc_get_random_entries(std::ostream &out, uint32_t count);
void apc_sample_by_size();

///////////////////////////////////////////////////////////////////////////////
}
