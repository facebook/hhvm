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

#ifndef incl_HPHP_STAT_CACHE_H_
#define incl_HPHP_STAT_CACHE_H_

#ifdef __linux__
#include <sys/inotify.h>
#endif

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/base.h"
#include "hphp/util/lock.h"
#include "hphp/runtime/base/smart-ptr.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StatCache {
 public:
  struct Node;
  typedef AtomicSmartPtr<Node> NodePtr;
  typedef tbb::concurrent_hash_map<std::string, NodePtr,
                                   stringHashCompare> NameNodeMap;
  typedef hphp_hash_map<int, NodePtr, int64_hash> WatchNodeMap;

  class Node : public AtomicCountable {
   public:
    typedef hphp_hash_map<std::string, NodePtr, string_hash> NameNodeMap;
    typedef hphp_hash_map<std::string, void*, string_hash> NameMap;

    explicit Node(StatCache& statCache, int wd=-1);
    void atomicRelease();

    void touch(bool invalidate=true);
    void expirePaths(bool invalidate=true);
    int stat(const std::string& path, struct stat* buf, time_t lastRefresh=0);
    int lstat(const std::string& path, struct stat* buf, time_t lastRefresh=0);
    bool isLink();
    std::string readlink(const std::string& path, time_t lastRefresh=0);
    void insertChild(const std::string& childName, NodePtr child, bool follow);
    void removeChild(const std::string& childName);
    NodePtr getChild(const std::string& childName, bool follow);
    void setPath(const std::string& path) {
      m_path = path;
    }
    const std::string& path() const { return m_path; }

   private:
    template <bool removePaths>
    void touchLocked(bool invalidate=true);
    void detachLocked();
    void sanityCheck(const std::string& path, bool isStat,
                     const struct stat* buf, time_t lastRefresh);
    bool isLinkLocked();
    bool validate(const std::string& path, bool& cached);

    StatCache& m_statCache;
    SimpleMutex m_lock;
    int m_wd;                // Watch descriptor; -1 if a file.

    bool m_valid;            // True if m_stat/m_lstat are currently valid.
    struct stat m_stat;      // Cached stat() result.
    struct stat m_lstat;     // Cached lstat() result.
    std::string m_link;      // Cached readlink() result.

    bool m_inExpirePaths;
    NameNodeMap m_children;  // stat() children.
    NameNodeMap m_lChildren; // lstat() children.

    NameMap m_paths;         // Associated entries in StatCache::m_path2Node.
    NameMap m_lpaths;        // Associated entries in StatCache::m_lpath2Node.

    std::string m_path;
  };

  StatCache();
  ~StatCache();

  static void requestInit(); // Process pending file change notifications.
  static int stat(const std::string& path, struct stat* buf);
  static int lstat(const std::string& path, struct stat* buf);
  static std::string readlink(const std::string& path);
  static std::string realpath(const char* path);

 private:
  bool init();
  void clear();
  void reset();
  NodePtr getNode(const std::string& path, bool follow);
  bool mergePath(const std::string& path, bool follow);
#ifdef __linux__
  bool handleEvent(const struct inotify_event* event);
#endif
  void removeWatch(int wd);
  void removePath(const std::string& path, Node* node);
  void removeLPath(const std::string& path, Node* node);
  void refresh();
  time_t lastRefresh();
  int statImpl(const std::string& path, struct stat* buf);
  int lstatImpl(const std::string& path, struct stat* buf);
  std::string readlinkImpl(const std::string& path);
  std::string realpathImpl(const char* path);

  static StatCache s_sc;

  NameNodeMap m_path2Node;  // stat() path cache.
  NameNodeMap m_lpath2Node; // lstat() path cache.

  SimpleMutex m_lock;       // Protects the following fields.
  int m_ifd;
#ifdef __linux__
  static const size_t kReadBufSize = 10 * (sizeof(struct inotify_event)
                                           + NAME_MAX + 1);
  char m_readBuf[kReadBufSize];
#endif
  time_t m_lastRefresh; // Used for debugging.
  WatchNodeMap m_watch2Node;
  NodePtr m_root;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_STAT_CACHE_H_
