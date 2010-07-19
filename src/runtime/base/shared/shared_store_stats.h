/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_SHARED_STORE_STATS_H__
#define __HPHP_SHARED_STORE_STATS_H__

#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SharedValueProfile {
public:
  char *key;
  SharedVariantStats var;
  int64 totalSize;
  int32 keySize;
  bool isGroup;
  int32 keyCount; // valid only for group
  int64 ttl; // valid for both, for group key stats, it's average
  // Also treat no ttl as 48-hrs.

  SharedValueProfile() {
    // For temporary use only
    key = NULL;
    isGroup = false;
    totalSize = 0;
    keySize = 0;
    keyCount = 0;
    ttl = 0;
  }

  SharedValueProfile(const char *key) {
    this->key = strdup(key);
    isGroup = false;
    totalSize = 0;
    keySize = 0;
    keyCount = 0;
    ttl = 0;
  }

  virtual ~SharedValueProfile() {
    if (key) free(key);
  }

  void calcInd(StringData *key, SharedVariant *var);
  void addToGroup(SharedValueProfile *ind);
  void removeFromGroup(SharedValueProfile *ind);
};

class SharedStoreStats {
public:
  static void onClear();
  static void onStore(StringData *key, SharedVariant *var, int64 ttl);
  static void onDelete(StringData *key, SharedVariant *var, bool replace);
  static void resetStats() {
    s_keyCount = 0;
    s_keySize = 0;
    s_variantCount = 0;
    s_dataSize = 0;
    s_dataTotalSize = 0;
    s_totalSize = 0;
    s_deleteSize = 0;
    s_replaceSize = 0;
  }

  static std::string report_basic();
  static std::string report_keys();
  static bool snapshot(const char *filename);

protected:
  static Mutex s_lock;

  static int32 s_keyCount; // how many distinct keys
  static int32 s_keySize; // how much space these keys take
  static int32 s_variantCount; // how many variant
  static int64 s_dataSize; // how large is the data
  static int64 s_dataTotalSize; // how much space to hold data
                                   // including structures
  static int64 s_totalSize; // total memory usage
  static int64 s_deleteSize;
  static int64 s_replaceSize;

  static void remove(SharedValueProfile *svp, bool replace);
  static void add(SharedValueProfile *svp);

  static void lock() {
    s_lock.lock();
  }
  static void unlock() {
    s_lock.unlock();
  }

  struct StringHash {
    size_t operator()(const char *s) const {
      ASSERT(s);
      return hash_string(s, strlen(s));
    }
  };

  struct StringEqual {
    bool operator()(const char *s1, const char *s2) const {
      ASSERT(s1 && s2);
      return strcmp(s1,s2) == 0;
    }
  };

  typedef hphp_hash_map<char*, SharedValueProfile*, StringHash, StringEqual>
    StatsMap;

  static StatsMap s_statsMap, s_detailMap;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SHARED_STORE_STATS_H__ */
