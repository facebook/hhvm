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

#ifndef incl_HPHP_EXT_APC_H_
#define incl_HPHP_EXT_APC_H_

#include "hphp/runtime/base/base-includes.h"
#include <set>
#include <vector>
#include "hphp/runtime/base/shared-store-base.h"
#include "hphp/runtime/server/upload.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class apcExtension : public Extension {
 public:
  apcExtension() : Extension("apc", "4.0.2") {}

  static bool Enable;
  static bool EnableConstLoad;
  static bool ForceConstLoadToAPC;
  static std::string PrimeLibrary;
  static int LoadThread;
  static std::set<std::string> CompletionKeys;
  enum class TableTypes {
    ConcurrentTable
  };
  static TableTypes TableType;
  static bool EnableApcSerialize;
  static time_t KeyMaturityThreshold;
  static size_t MaximumCapacity;
  static int KeyFrequencyUpdatePeriod;
  static bool ExpireOnSets;
  static int PurgeFrequency;
  static int PurgeRate;
  static bool AllowObj;
  static int TTLLimit;
  static bool UseFileStorage;
  static int64_t FileStorageChunkSize;
  static int64_t FileStorageMaxSize;
  static std::string FileStoragePrefix;
  static int FileStorageAdviseOutPeriod;
  static std::string FileStorageFlagKey;
  static bool ConcurrentTableLockFree;
  static bool FileStorageKeepFileLinked;
  static std::vector<std::string> NoTTLPrefix;
  static bool UseUncounted;
  static bool Stat;
  static bool EnableCLI;

  virtual void moduleLoad(Hdf config);
  virtual void moduleInit();
  virtual void moduleShutdown();
};

///////////////////////////////////////////////////////////////////////////////
Variant f_apc_add(const Variant& key_or_array, const Variant& var = null_variant,
                  int64_t ttl = 0, int64_t cache_id = 0);
Variant f_apc_store(const Variant& key_or_array, const Variant& var = null_variant,
                    int64_t ttl = 0, int64_t cache_id = 0);
bool f_apc_store_as_primed_do_not_use(const String& key, const Variant& var,
                                      int64_t cache_id = 0);

Variant f_apc_fetch(const Variant& key, VRefParam success = uninit_null(),
                    int64_t cache_id = 0);
Variant f_apc_delete(const Variant& key, int64_t cache_id = 0);
bool f_apc_clear_cache(int64_t cache_id = 0);
Variant f_apc_inc(const String& key, int64_t step = 1,
                  VRefParam success = uninit_null(), int64_t cache_id = 0);
Variant f_apc_dec(const String& key, int64_t step = 1,
                  VRefParam success = uninit_null(), int64_t cache_id = 0);
bool f_apc_cas(const String& key, int64_t old_cas, int64_t new_cas,
               int64_t cache_id = 0);
Variant f_apc_exists(const Variant& key, int64_t cache_id = 0);

///////////////////////////////////////////////////////////////////////////////

Variant f_apc_cache_info(int64_t cache_id = 0, bool limited = false);
Array f_apc_sma_info(bool limited = false);

///////////////////////////////////////////////////////////////////////////////
// loading APC from archive files

void apc_load(int thread);

// needed by generated apc archive .cpp files
void apc_load_impl(struct cache_info *info,
                   const char **int_keys, long long *int_values,
                   const char **char_keys, char *char_values,
                   const char **strings, const char **objects,
                   const char **thrifts, const char **others);
void apc_load_impl_compressed(
  struct cache_info *info,
  int *int_lens, const char *int_keys, long long *int_values,
  int *char_lens, const char *char_keys, char *char_values,
  int *string_lens, const char *strings,
  int *object_lens, const char *objects,
  int *thrift_lens, const char *thrifts,
  int *other_lens, const char *others);

class apc_rfc1867_data {
public:
  std::string tracking_key;
  int64_t content_length;
  std::string filename;
  std::string name;
  char *temp_filename;
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

String apc_serialize(const Variant& value);
Variant apc_unserialize(const char* data, int len);
String apc_reserialize(const String& str);

///////////////////////////////////////////////////////////////////////////////
// debugging support

bool apc_dump(const char *filename, bool keyOnly, int waitSeconds);
size_t get_const_map_size();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_APC_H_
