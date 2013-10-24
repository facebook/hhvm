/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SHARED_STORESTATS_H_
#define incl_HPHP_SHARED_STORESTATS_H_

#include <tbb/concurrent_hash_map.h>

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedValueProfile {
private:
  void init() {
    key = nullptr;
    isGroup = false;
    isPrime = false;
    isValid = false;
    totalSize = 0;
    keySize = 0;
    keyCount = 0;
    sizeNoTTL = 0;
    ttl = 0;
    fetchCount = 0;
    lastFetchTime = 0;
    storeCount = 0;
    lastStoreTime = 0;
    deleteCount = 0;
    lastDeleteTime = 0;
  }

public:
  char *key;
  APCHandleStats var;
  int32_t totalSize;
  int32_t keySize;
  bool isGroup;
  bool isPrime;
  bool isValid;
  int32_t keyCount; // valid only for group
  int32_t sizeNoTTL; // valid only for group
  int32_t ttl; // valid for both, for group key stats, it's average
  // Also treat no ttl as 48-hrs.

  // fetchCount and lastGetTime only valid for individual (so that we don't need
  // normalize the key for every get)
  int32_t fetchCount;
  int32_t storeCount;
  int32_t deleteCount;
  time_t lastFetchTime;
  time_t lastStoreTime;
  time_t lastDeleteTime;


  SharedValueProfile() {
    // For temporary use only
    init();
  }

  explicit SharedValueProfile(const char *key) {
    init();
    this->key = strdup(key);
    isGroup = false;
  }

  ~SharedValueProfile() {
    if (key) free(key);
  }

  void calcInd(const StringData *key, const APCHandle *var);
  void addToGroup(SharedValueProfile *ind);
  void removeFromGroup(SharedValueProfile *ind);
};

class SharedStoreStats {
public:
  static void onStore(const StringData *key, const APCHandle *var,
                      int64_t ttl, bool prime);
  static void onDelete(const StringData *key, const APCHandle *var,
                       bool replace, bool noTTL);
  static void onGet(const StringData *key, const APCHandle *var);

  static std::string report_basic();
  static std::string report_basic_flat();
  static std::string report_keys();
  static bool snapshot(const char *filename, std::string& keySample);

  static void addDirect(int32_t keySize, int32_t dataTotal, bool prime, bool file);
  static void removeDirect(int32_t keySize, int32_t dataTotal, bool exp);
  static void updateDirect(int32_t dataTotalOld, int32_t dataTotalNew);

  static void setExpireQueueSize(int32_t size) {
    s_expireQueueSize = size;
  }
  static void addPurgingTime(int64_t purgingTime);

protected:
  static ReadWriteMutex s_rwlock;

  static std::atomic<int32_t> s_keyCount; // how many distinct keys
  static std::atomic<int32_t> s_keySize; // how much space these keys take
  static int32_t s_variantCount; // how many variant
  static int64_t s_dataSize; // how large is the data
  static std::atomic<int64_t> s_dataTotalSize; // how much space to hold data
                                // including structures
  static int64_t s_deleteSize;
  static int64_t s_replaceSize;

  static std::atomic<int32_t> s_addCount;
  static std::atomic<int32_t> s_primeCount;
  static std::atomic<int32_t> s_fromFileCount;
  static std::atomic<int32_t> s_updateCount;
  static std::atomic<int32_t> s_deleteCount;
  static std::atomic<int32_t> s_expireCount;

  static int32_t s_expireQueueSize;
  static std::atomic<int64_t> s_purgingTime;

  static void remove(SharedValueProfile *svp, bool replace);
  static void add(SharedValueProfile *svp);

  struct charHashCompare {
    bool equal(const char *s1, const char *s2) const {
      assert(s1 && s2);
      return strcmp(s1, s2) == 0;
    }
    size_t hash(const char *s) const {
      assert(s);
      return hash_string(s);
    }
  };

  typedef tbb::concurrent_hash_map<const char*, SharedValueProfile*,
                                   charHashCompare> StatsMap;

  static StatsMap s_statsMap, s_detailMap;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_SHARED_STORESTATS_H_ */
