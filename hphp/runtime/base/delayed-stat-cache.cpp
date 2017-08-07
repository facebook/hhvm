/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/delayed-stat-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static inline uint64_t get_time_us() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_usec + (uint64_t)tv.tv_sec * 1000000;
}

static int statSyscall(const char* path, struct stat* buf) {
  int ret = ::stat(path, buf);
  TRACE(5, "DelayedStatCache: stat path:%s ret:%d\n", path, ret);
  return ret;
}

static int lstatSyscall(const char* path, struct stat* buf) {
  int ret = ::lstat(path, buf);
  TRACE(5, "DelayedStatCache: lstat path:%s ret:%d\n", path, ret);
  return ret;
}

static int accessSyscall(const char* path, int mode) {
  int ret = ::access(path, mode);
  TRACE(5, "DelayedStatCache: access path:%s mode:%d ret:%d\n", path, mode, ret);
  return ret;
}

static std::string readlinkSyscall(const char* path) {
  char buf[PATH_MAX + 1];
  ssize_t len = ::readlink(path, buf, sizeof(buf) - 1);
  if (len == -1) {
    buf[0] = '\0';
  } else {
    buf[len] = '\0';
  }
  TRACE(5, "DelayedStatCache: readlink path:%s ret:%s\n", path, buf);
  return buf;
}

static std::string realpathSyscall(const char* path) {
  char buf[PATH_MAX];

  if (!::realpath(path, buf)) {
    buf[0] = '\0';
  }
  TRACE(5, "DelayedStatCache: realpath path:%s ret:%s\n", path, buf);
  return buf;
}

class StatCacheLock {
  public:
    StatCacheLock(ReadWriteMutex& mutex, bool enable):
      m_mutex(mutex), m_enable(enable), m_locked(false) {}

    void writeLock() {
      if (!m_enable) {
        return;
      }
      m_mutex.acquireWrite();
      m_locked = true;
    }

    void readLock() {
      if (!m_enable) {
        return;
      }
      m_mutex.acquireRead();
      m_locked = true;
    }

    void release() {
      if (!m_enable) {
        return;
      }
      if (m_locked) {
        m_mutex.release();
        m_locked = false;
      }
    }

    ~StatCacheLock() {
      release();
    }

  private:
    ReadWriteMutex& m_mutex;
    bool m_enable;
    bool m_locked;
};

/**
 * StatCacheBucket storages the stat result. 
 * If Server.DelayedStatCacheBucketNum > 0, the bucket may be shared by different threads,
 *   the performance will be slower, but use less memory. 
 * If Server.DelayedStatCacheBucketNum <= 0, the bucket will be thread local, the performance 
 *   will be better, but use more memory.
 */
class StatCacheBucket {
  private:
    struct StatResult {
      struct stat m_stat;
      int m_ret;
    };
    typedef std::unordered_map<int, int> AccessResult;
    typedef std::unordered_map<std::string, StatResult> StatResultMap;
    typedef std::unordered_map<std::string, AccessResult> AccessResultMap;
    typedef std::unordered_map<std::string, std::string> PathMap;

  public:
    StatCacheBucket(bool is_shared): m_is_shared(is_shared)
                                   , m_last_update_time_us(get_time_us()) {}

    void clearAllCache() {
      {
        StatResultMap m;
        StatCacheLock l(m_stat_mutex, m_is_shared);
        l.writeLock();
        std::swap(m, m_stat_cache);
        l.release();
      }

      {
        StatResultMap m;
        StatCacheLock l(m_lstat_mutex, m_is_shared);
        l.writeLock();
        std::swap(m, m_lstat_cache);
        l.release();
      }

      {
        AccessResultMap m;
        StatCacheLock l(m_access_mutex, m_is_shared);
        l.writeLock();
        std::swap(m, m_access_cache);
        l.release();
      }

      {
        PathMap m;
        StatCacheLock l(m_readlink_mutex, m_is_shared);
        l.writeLock();
        std::swap(m, m_readlink_cache);
        l.release();
      }

      {
        PathMap m;
        StatCacheLock l(m_realpath_mutex, m_is_shared);
        l.writeLock();
        std::swap(m, m_realpath_cache);
        l.release();
      }
    }

    void getStatistics(std::stringstream& ss) {
      ss << "  {\n";
      ss << "    \"stat\":" << m_stat_cache.size() << ",\n";
      ss << "    \"lstat\":" << m_lstat_cache.size() << ",\n";
      ss << "    \"access\":" << m_access_cache.size() << ",\n";
      ss << "    \"readlink\":" << m_readlink_cache.size() << ",\n";
      ss << "    \"realpath\":" << m_realpath_cache.size() << "\n";
      ss << "  }";
    }

    int stat(const char* path, struct stat* buf) {
      checkUpdate();
      StatCacheLock l(m_stat_mutex, m_is_shared);
      l.readLock();
      auto it = m_stat_cache.find(path);
      if (it != m_stat_cache.end()) {
        int ret = it->second.m_ret;
        if (ret == 0) {
          memcpy(buf, &it->second.m_stat, sizeof(struct stat));
        }
        return ret;
      } else {
        l.release();
        int ret = statSyscall(path, buf);
        l.writeLock();
        auto &stat_res = m_stat_cache[path];
        if (ret == 0) {
          memcpy(&stat_res.m_stat, buf, sizeof(struct stat));
        }
        stat_res.m_ret = ret;
        return ret;
      }
    }

    int lstat(const char* path, struct stat* buf) {
      checkUpdate();
      StatCacheLock l(m_lstat_mutex, m_is_shared);
      l.readLock();
      auto it = m_lstat_cache.find(path);
      if (it != m_lstat_cache.end()) {
        int ret = it->second.m_ret;
        if (ret == 0) {
          memcpy(buf, &it->second.m_stat, sizeof(struct stat));
        }
        m_lstat_mutex.release(); 
        return ret;
      } else {
        l.release();
        int ret = lstatSyscall(path, buf);
        l.writeLock();
        auto &stat_res = m_lstat_cache[path];
        if (ret == 0) {
          memcpy(&stat_res.m_stat, buf, sizeof(struct stat));
        }
        stat_res.m_ret = ret;
        return ret;
      }
    }

    int access(const char* path, int mode) {
      checkUpdate();
      StatCacheLock l(m_access_mutex, m_is_shared);
      l.readLock();
      auto it = m_access_cache.find(path);
      if (it != m_access_cache.end()) {
        auto &access_res = it->second;
        auto it2 = access_res.find(mode);
        if (it2 != access_res.end()) {
          int ret = it2->second;
          return ret;
        } else {
          l.release();
          int ret = accessSyscall(path, mode);
          l.writeLock();
          access_res[mode] = ret;
          return ret;
        }
      } else {
        l.release();
        int ret = accessSyscall(path, mode);
        l.writeLock();
        auto &access_res = m_access_cache[path];
        access_res[mode] = ret;
        return ret;
      }
    }

    std::string readlink(const char* path) {
      checkUpdate();
      StatCacheLock l(m_readlink_mutex, m_is_shared);
      l.readLock();
      auto it = m_readlink_cache.find(path);
      if (it != m_readlink_cache.end()) {
        std::string ret = it->second;
        return ret;
      } else {
        l.release();
        std::string ret = readlinkSyscall(path);
        l.writeLock();
        m_readlink_cache[path] = ret;
        return ret;
      }
    }

    std::string realpath(const char* path) {
      checkUpdate();
      StatCacheLock l(m_realpath_mutex, m_is_shared);
      l.readLock();
      auto it = m_realpath_cache.find(path);
      if (it != m_realpath_cache.end()) {
        std::string ret = it->second;
        return ret;
      } else {
        l.release();
        std::string ret = realpathSyscall(path);
        l.writeLock();
        m_realpath_cache[path] = ret;
        return ret;
      }
    }

  private:
    void checkUpdate() {
      if (RuntimeOption::ServerDelayedStatCacheExpireSeconds <= 0) {
        return;
      }
      uint64_t now = get_time_us();
      if (now > m_last_update_time_us + RuntimeOption::ServerDelayedStatCacheExpireSeconds * 1000000) {
        m_last_update_time_us = now;
        clearAllCache();
      }
    }

    bool m_is_shared;
    uint64_t m_last_update_time_us;

    StatResultMap m_stat_cache;
    StatResultMap m_lstat_cache;
    AccessResultMap m_access_cache;
    PathMap m_readlink_cache;
    PathMap m_realpath_cache;

    ReadWriteMutex m_stat_mutex;
    ReadWriteMutex m_lstat_mutex;
    ReadWriteMutex m_access_mutex;
    ReadWriteMutex m_readlink_mutex;
    ReadWriteMutex m_realpath_mutex;
};

std::vector<std::shared_ptr<StatCacheBucket>> g_stat_cache_buckets;
static int g_stat_cache_bucket_id = 0;
static Mutex g_stat_cache_mutex;
static __thread StatCacheBucket* s_stat_cache_bucket = nullptr;

StatCacheBucket* getStatCacheBucket() {
  if (s_stat_cache_bucket) {
    return s_stat_cache_bucket;
  }
  Lock l(g_stat_cache_mutex);
  int bucket_num = RuntimeOption::ServerDelayedStatCacheBucketNum;
  if (bucket_num > 0) {
    while (g_stat_cache_buckets.size() <= g_stat_cache_bucket_id) {
      auto bucket = std::make_shared<StatCacheBucket>(true);
      g_stat_cache_buckets.push_back(bucket);
    }
    s_stat_cache_bucket = g_stat_cache_buckets[g_stat_cache_bucket_id].get();
    // use a round-robin strategy to map a thread to a bucket
    g_stat_cache_bucket_id = (g_stat_cache_bucket_id + 1) % bucket_num;
  } else {
    auto bucket = std::make_shared<StatCacheBucket>(false);
    g_stat_cache_buckets.push_back(bucket);
    s_stat_cache_bucket = bucket.get();
  }
  return s_stat_cache_bucket;
}

int DelayedStatCache::stat(const char* path, struct stat* buf) {
  if (RuntimeOption::ServerDelayedStatCache) {
    return getStatCacheBucket()->stat(path, buf);
  } else {
    return statSyscall(path, buf);
  }
}

int DelayedStatCache::lstat(const char* path, struct stat* buf) {
  if (RuntimeOption::ServerDelayedStatCache) {
    return getStatCacheBucket()->lstat(path, buf);
  } else {
    return lstatSyscall(path, buf);
  }
}

int DelayedStatCache::access(const char* path, int mode) {
  if (RuntimeOption::ServerDelayedStatCache) {
    return getStatCacheBucket()->access(path, mode);
  } else {
    return accessSyscall(path, mode);
  }
}

std::string DelayedStatCache::readlink(const char* path) {
  if (RuntimeOption::ServerDelayedStatCache) {
    return getStatCacheBucket()->readlink(path);
  } else {
    return readlinkSyscall(path);
  }
}

std::string DelayedStatCache::realpath(const char* path) {
  if (RuntimeOption::ServerDelayedStatCache) {
    return getStatCacheBucket()->realpath(path);
  } else {
    return realpathSyscall(path);
  }
}

void DelayedStatCache::clearAllCache() {
  Lock l(g_stat_cache_mutex);
  for (auto bucket : g_stat_cache_buckets) {
    bucket->clearAllCache();
  }
}

void DelayedStatCache::getStatistics(std::stringstream& ss) {
  Lock l(g_stat_cache_mutex);
  ss << "[";
  bool first = true;
  for (auto bucket : g_stat_cache_buckets) {
    if (!first) {
      ss << ',';
    }
    first = false;
    ss << '\n';
    bucket->getStatistics(ss);
  }
  ss << "\n]";
}

///////////////////////////////////////////////////////////////////////////////
}
