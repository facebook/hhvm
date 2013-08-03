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
#include "hphp/runtime/base/stat-cache.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>

#include "hphp/util/trace.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/hooks.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(stat);

UNUSED static std::string statToString(const struct stat* buf) {
  std::ostringstream os;
  os << "struct stat {";
  os <<   "dev="                << buf->st_dev                 << ", ";
  os <<   "ino="                << buf->st_ino                 << ", ";
  os <<   "mode=0"  << std::oct << buf->st_mode    << std::dec << ", ";
  os <<   "nlink="              << buf->st_nlink               << ", ";
  os <<   "uid="                << buf->st_uid                 << ", ";
  os <<   "gid="                << buf->st_gid                 << ", ";
  os <<   "rdev="               << buf->st_rdev                << ", ";
  os <<   "size="               << buf->st_size                << ", ";
  os <<   "blksize="            << buf->st_blksize             << ", ";
  os <<   "blocks="             << buf->st_blocks              << ", ";
  os <<   "atime="              << buf->st_atime               << ", ";
  os <<   "mtime="              << buf->st_mtime               << ", ";
  os <<   "ctime="              << buf->st_ctime;
  os << "}";
  return os.str();
}

UNUSED static bool statEquiv(const struct stat* stA, const struct stat* stB) {
  return (stA->st_dev == stB->st_dev
          && stA->st_ino == stB->st_ino
          && stA->st_mode == stB->st_mode
          && stA->st_nlink == stB->st_nlink
          && stA->st_uid == stB->st_uid
          && stA->st_gid == stB->st_gid
          && stA->st_rdev == stB->st_rdev
          && stA->st_size == stB->st_size
          && stA->st_blksize == stB->st_blksize
          && stA->st_blocks == stB->st_blocks
          /* Intentionally omitted:
          && stA->st_atime == stB->st_atime
          */
          && stA->st_mtime == stB->st_mtime
          && stA->st_ctime == stB->st_ctime);
}

#ifdef __linux__
UNUSED static std::string eventToString(const struct inotify_event* ie) {
  bool first = true;
  std::ostringstream os;
  os << "struct inotify_event {wd=" << ie->wd << ", mask=(";
#define EVENT(e) do {                                                         \
  if (ie->mask & e) {                                                         \
    if (first) {                                                              \
      first = false;                                                          \
    } else {                                                                  \
      os << "|";                                                              \
    }                                                                         \
    os << #e;                                                                 \
  }                                                                           \
} while (0)
  EVENT(IN_ACCESS);
  EVENT(IN_MODIFY);
  EVENT(IN_ATTRIB);
  EVENT(IN_CLOSE_WRITE);
  EVENT(IN_CLOSE_NOWRITE);
  EVENT(IN_OPEN);
  EVENT(IN_MOVED_FROM);
  EVENT(IN_MOVED_TO);
  EVENT(IN_CREATE);
  EVENT(IN_DELETE);
  EVENT(IN_DELETE_SELF);
  EVENT(IN_MOVE_SELF);
  EVENT(IN_UNMOUNT);
  EVENT(IN_Q_OVERFLOW);
  EVENT(IN_IGNORED);
  EVENT(IN_ISDIR);
#undef EVENT
  os << ")";
  if (ie->cookie != 0) os << ", cookie=" << ie->cookie;
  if (ie->len != 0) os << ", name='" << ie->name << "'";
  os << "}";
  return os.str();
}
#endif

static int statSyscall(const std::string& path, struct stat* buf) {
  int ret = ::stat(path.c_str(), buf);
  if (ret == 0) {
    TRACE(5, "StatCache: stat '%s' %s\n",
             path.c_str(), statToString(buf).c_str());
  } else {
    TRACE(5, "StatCache: stat '%s' --> error\n", path.c_str());
  }
  return ret;
}

static int lstatSyscall(const std::string& path, struct stat* buf) {
  int ret = ::lstat(path.c_str(), buf);
  if (ret == 0) {
    TRACE(5, "StatCache: lstat '%s' %s\n",
             path.c_str(), statToString(buf).c_str());
  } else {
    TRACE(5, "StatCache: lstat '%s' --> error\n", path.c_str());
  }
  return ret;
}

static std::string readlinkSyscall(const std::string& path) {
  char lbuf[PATH_MAX + 1];
  ssize_t llen = ::readlink(path.c_str(), lbuf, sizeof(lbuf) - 1);
  if (llen == -1) {
    TRACE(5, "StatCache: readlink('%s') --> error\n", path.c_str());
    return "";
  }
  lbuf[llen] = '\0';
  TRACE(5, "StatCache: readlink('%s') --> '%s'\n", path.c_str(), lbuf);
  return lbuf;
}

static std::string realpathLibc(const char* path) {
  char buf[PATH_MAX];

  std::string ret;
  if (!::realpath(path, buf)) {
    TRACE(5, "StatCache: realpath('%s') --> error\n", path);
    return ret;
  }
  TRACE(5, "StatCache: realpath('%s') --> '%s'\n", path, buf);
  ret = buf;
  return ret;
}

//==============================================================================
// StatCache::Node.

StatCache::Node::Node(StatCache& statCache, int wd /* = -1 */)
  : m_statCache(statCache),
    m_lock(false /*reentrant*/, RankStatCacheNode),
    m_wd(wd), m_valid(false), m_inExpirePaths(false) { }

void StatCache::Node::atomicRelease() {
  if (m_wd != -1) {
    m_statCache.removeWatch(m_wd);
  }
  touchLocked<true>();
  detachLocked();
  TRACE(1, "StatCache: delete node '%s'\n", m_path.c_str());
  delete this;
}

template <bool removePaths>
void StatCache::Node::touchLocked(bool invalidate /* = true */) {
  TRACE(1, "StatCache: touch %snode '%s'%s\n",
           m_valid ? "" : "invalid ",
           m_path.c_str(), removePaths ? " (remove paths)" : "");
  if ((invalidate && m_valid) || removePaths) {
    // Call path invalidation callback once for each path associated with this
    // node and/or remove paths.
    for (NameMap::const_iterator it = m_paths.begin(); it != m_paths.end();
         ++it) {
      if (invalidate && m_valid) {
        TRACE(1, "StatCache: invalidate path '%s'\n", it->first.c_str());
        HPHP::invalidatePath(it->first);
      }
      if (removePaths) {
        m_statCache.removePath(it->first, this);
      }
    }
    for (NameMap::const_iterator it = m_lpaths.begin(); it != m_lpaths.end();
         ++it) {
      if (invalidate && m_valid) {
        // Avoid duplicate invalidations.
        NameMap::const_iterator it2 = m_paths.find(it->first);
        if (it2 == m_paths.end()) {
          TRACE(1, "StatCache: invalidate link path '%s'\n", it->first.c_str());
          HPHP::invalidatePath(it->first);
        }
      }
      if (removePaths) {
        m_statCache.removeLPath(it->first, this);
      }
    }
    if (removePaths) {
      m_paths.clear();
      m_lpaths.clear();
    }
  }
  m_link.clear();
  m_valid = false;
}

void StatCache::Node::touch(bool invalidate /* = true */) {
  SimpleLock lock(m_lock);
  touchLocked<false>(invalidate);
}

void StatCache::Node::detachLocked() {
  m_children.clear();
  m_lChildren.clear();
}

void StatCache::Node::expirePaths(bool invalidate /* = true */) {
  NameNodeMap children, lChildren;

  {
    SimpleLock lock(m_lock);

    if (m_inExpirePaths) {
      // Terminate loop in recursion.
      return;
    }

    touchLocked<true>(invalidate);
    children = m_children;
    lChildren = m_lChildren;
    // expirePaths() is only called in situations where the entire subtree
    // needs to be completely invalidated.  If there were call for a 'touch'
    // operation, then the detachLocked() call would need to be omitted.
    detachLocked();
    m_inExpirePaths = true;
  }

  for (NameNodeMap::const_iterator it = children.begin(); it != children.end();
       ++it) {
    it->second->expirePaths(invalidate);
  }
  for (NameNodeMap::const_iterator it = lChildren.begin();
       it != lChildren.end(); ++it) {
    // Only recurse if this node differs from the equivalent one in children,
    // in order to keep recursion from being of exponential complexity.
    NameNodeMap::const_iterator it2 = children.find(it->first);
    if (it2 == children.end() || it->second.get() != it2->second.get()) {
      it->second->expirePaths(invalidate);
    }
  }

  SimpleLock lock(m_lock);
  m_inExpirePaths = false;
}

bool StatCache::Node::validate(const std::string& path, bool& cached) {
  if (!m_valid) {
    if (statSyscall(path, &m_stat) == -1) {
      TRACE(4, "StatCache: stat '%s' --> error (node=%p)\n",
               path.c_str(), this);
      return true;
    }
    TRACE(4, "StatCache: stat '%s' %s (node=%p)\n",
             path.c_str(), statToString(&m_stat).c_str(), this);
    if (lstatSyscall(path, &m_lstat) == -1) {
      TRACE(4, "StatCache: lstat '%s' --> error (node=%p)\n",
               path.c_str(), this);
      return true;
    }
    TRACE(4, "StatCache: lstat '%s' %s (node=%p)\n",
             path.c_str(), statToString(&m_lstat).c_str(), this);
    m_valid = true;
    cached = false;
  } else {
    TRACE(4, "StatCache: stat '%s' (node=%p, cached)\n", path.c_str(), this);
    TRACE(4, "StatCache: lstat '%s' (node=%p, cached)\n", path.c_str(), this);
    cached = true;
  }
  setPath(path);
  return false;
}

void StatCache::Node::sanityCheck(const std::string& path, bool isStat,
                                  const struct stat* buf, time_t lastRefresh) {
  struct stat tbuf;
  int err = isStat ? statSyscall(path, &tbuf) : lstatSyscall(path, &tbuf);
  if (err != -1 && !statEquiv(buf, &tbuf)) {
    if (lastRefresh == 0) {
      lastRefresh = m_statCache.lastRefresh();
    }
    // stat info has changed since it was cached.  If the changes were all made
    // prior to the most recent refresh (excluding atime, since IN_ACCESS
    // events aren't being processed), then they generally should have been
    // merged into the cache during the refresh.  Reality is a bit messier
    // because inotify is asynchronous; the delay between a filesystem
    // modification and the availability of a corresponding inotify event makes
    // it possible for the most recent refresh to have missed in-flight
    // notifications.
    if (tbuf.st_mtime < lastRefresh && tbuf.st_ctime < lastRefresh) {
      TRACE(0, "StatCache: suspect cached %s '%s' %s (node=%p);"
               " actual %s; last refresh %lu\n", isStat ? "stat" : "lstat",
               path.c_str(),
               statToString(buf).c_str(), this,
               statToString(&tbuf).c_str(), (unsigned long)lastRefresh);
    }
  }
}

int StatCache::Node::stat(const std::string& path, struct stat* buf,
                          time_t lastRefresh /* = 0 */) {
  bool cached;
  {
    SimpleLock lock(m_lock);
    if (validate(path, cached)) {
      return -1;
    }
    mapInsert(m_paths, path, this);
    memcpy(buf, &m_stat, sizeof(struct stat));
  }
  if (debug && cached) {
    sanityCheck(path, true, buf, lastRefresh);
  }
  return 0;
}

int StatCache::Node::lstat(const std::string& path, struct stat* buf,
                           time_t lastRefresh /* = 0 */) {
  bool cached;
  {
    SimpleLock lock(m_lock);
    if (validate(path, cached)) {
      return -1;
    }
    mapInsert(m_lpaths, path, this);
    memcpy(buf, &m_lstat, sizeof(struct stat));
  }
  if (debug && cached) {
    sanityCheck(path, false, buf, lastRefresh);
  }
  return 0;
}

bool StatCache::Node::isLinkLocked() {
  m_lock.assertOwnedBySelf();
  return S_ISLNK(m_lstat.st_mode);
}

bool StatCache::Node::isLink() {
  SimpleLock lock(m_lock);
  return isLinkLocked();
}

std::string StatCache::Node::readlink(const std::string& path,
                                      time_t lastRefresh /* = 0 */) {
  std::string link;
  bool cached;
  struct stat buf;
  {
    SimpleLock lock(m_lock);
    if (validate(path, cached) || !isLinkLocked()) {
      return "";
    }
    if (debug && cached) {
      memcpy(&buf, &m_lstat, sizeof(struct stat));
    }
    if (m_link.size() == 0) {
      m_link = readlinkSyscall(path);
    }
    link = m_link;
  }
  if (debug && cached) {
    sanityCheck(path, false, &buf, lastRefresh);
  }
  return link;
}

void StatCache::Node::insertChild(const std::string& childName,
                                  StatCache::NodePtr child, bool follow) {
  mapInsertUnique(follow ? m_children : m_lChildren, childName, child);
}

void StatCache::Node::removeChild(const std::string& childName) {
  if (mapContains(m_children, childName)) {
    m_children.erase(childName);
  }
  if (mapContains(m_lChildren, childName)) {
    m_lChildren.erase(childName);
  }
}

StatCache::NodePtr StatCache::Node::getChild(const std::string& childName,
                                             bool follow) {
  NodePtr child;
  if (!mapGet(follow ? m_children : m_lChildren, childName, &child)) {
    child = nullptr;
  }
  return child;
}

//==============================================================================
// StatCache.

StatCache::StatCache()
  : m_lock(false /*reentrant*/, RankStatCache), m_ifd(-1),
    m_lastRefresh(time(nullptr)) {
}

StatCache::~StatCache() {
  clear();
}

bool StatCache::init() {
  // inotify_init1() directly supports the fcntl() settings, but it's only
  // available starting in Linux 2.6.27.
#ifdef __linux__
  if ((m_ifd = inotify_init()) == -1
      || fcntl(m_ifd, F_SETFD, FD_CLOEXEC) == -1
      || fcntl(m_ifd, F_SETFL, O_NONBLOCK) == -1
      || (m_root = getNode("/", false)).get() == nullptr) {
    clear();
    return true;
  }
  return false;
#else
  return true;
#endif
}

void StatCache::clear() {
  if (m_ifd != -1) {
    close(m_ifd);
    m_ifd = -1;
  }
  m_watch2Node.clear();
  // It's unsafe to reset() m_path2Node / m_lpath2Node while concurrent
  // accessors might be touching it.  Recursively letting them remove
  // themselves via expiry will remove them one by one via erase().  The call
  // to expirePaths() cannot be safely omitted, because it would otherwise be
  // possible for a symlink-induced cycle to keep some or all of the node tree
  // alive.
  if (m_root.get()) {
    m_root->expirePaths();
  }
  m_root = nullptr;
  assert(m_path2Node.size() == 0);
  assert(m_lpath2Node.size() == 0);
}

void StatCache::reset() {
  clear();
  init();
}

StatCache::NodePtr StatCache::getNode(const std::string& path, bool follow) {
#ifdef __linux__
  int wd = inotify_add_watch(m_ifd, path.c_str(),
                             0
                             | IN_MODIFY
                             | IN_ATTRIB
                             | IN_MOVED_FROM
                             | IN_MOVED_TO
                             | IN_CREATE
                             | IN_DELETE
                             | (follow ? 0 : IN_DONT_FOLLOW)
                             | IN_ONLYDIR);
  if (wd == -1 && errno != ENOTDIR) {
    TRACE(2, "StatCache: getNode('%s', follow=%s) failed\n",
             path.c_str(), follow ? "true" : "false");
    return NodePtr(nullptr);
  }
  NodePtr node;
  if (wd != -1) {
    if (!mapGet(m_watch2Node, wd, &node)) {
      node = new Node(*this, wd);
      mapInsertUnique(m_watch2Node, wd, node);
      TRACE(2, "StatCache: getNode('%s', follow=%s) --> %p (wd=%d)\n",
               path.c_str(), follow ? "true" : "false", node.get(), wd);
    } else {
      TRACE(3, "StatCache: getNode('%s', follow=%s) --> alias %p (wd=%d)\n",
               path.c_str(), follow ? "true" : "false", node.get(), wd);
    }
  } else {
    node = new Node(*this);
    TRACE(3, "StatCache: getNode('%s', follow=%s) --> %p\n",
             path.c_str(), follow ? "true" : "false", node.get());
  }
  node->setPath(path);
  return node;
#else
  return NodePtr(nullptr);
#endif
}

bool StatCache::mergePath(const std::string& path, bool follow) {
  std::string canonicalPath = Util::canonicalize(path);
  std::vector<std::string> pvec;
  Util::split('/', canonicalPath.c_str(), pvec);
  assert((pvec[0].size() == 0)); // path should be absolute.
  // Lazily initialize so that if StatCache never gets used, no kernel
  // resources are consumed.
  if (m_ifd == -1 && init()) {
    return true;
  }
  NodePtr curNode = m_root;
  std::string curPath = "/";
  for (unsigned i = 1; i < pvec.size(); ++i) {
    // Follow links unless 'follow' is false and this is the last path
    // component.
    bool curFollow = (follow || i + 1 < pvec.size());
    curPath += pvec[i];
    NodePtr child = curNode->getChild(pvec[i], curFollow);
    if (child.get() == nullptr) {
      child = getNode(curPath, curFollow);
      if (child.get() == nullptr) {
        return true;
      }
      curNode->insertChild(pvec[i], child, curFollow);
    }
    curNode = child;
    curPath += "/";
  }
  NameNodeMap::accessor acc;
  NameNodeMap& p2n = follow ? m_path2Node : m_lpath2Node;
  if (p2n.insert(acc, path)) {
    acc->second = curNode;
    TRACE(1, "StatCache: merge '%s' --> %p (follow=%s)\n",
             path.c_str(), curNode.get(), follow ? "true" : "false");
  }
  return false;
}

#ifdef __linux__
bool StatCache::handleEvent(const struct inotify_event* event) {
  if (event->mask & IN_Q_OVERFLOW) {
    // The event queue overflowed, so all bets are off.  Start over.
    TRACE(0, "StatCache: event queue overflowed\n");
    reset();
    return true;
  }
  assert(event->wd != -1);
  NodePtr node;
  if (!mapGet(m_watch2Node, event->wd, &node)) {
    TRACE(1, "StatCache: inotify event (obsolete) %s\n",
             eventToString(event).c_str());
    return false;
  }
  TRACE(1, "StatCache: inotify event for '%s': %s\n",
           node->path().c_str(), eventToString(event).c_str());

  if (event->mask & (IN_MODIFY|IN_ATTRIB)) {
    bool touched = false;
    NodePtr child = node->getChild(event->name, true);
    if (child.get() != nullptr) {
      if ((event->mask & IN_MODIFY) && child->isLink()) {
        // A modified link is logically equivalent to IN_MOVED_FROM.
        child->expirePaths();
        node->removeChild(event->name);
      } else {
        child->touch();
      }
      touched = true;
    }
    child = node->getChild(event->name, false);
    if (child.get() != nullptr) {
      // The follow=false child is equivalent to the follow=true child unless
      // it's a link.  Avoid duplicate invalidations for non-links.
      child->touch(!touched || child->isLink());
    }
  }
  if (event->mask & (IN_MOVED_FROM|IN_MOVED_TO|IN_CREATE|IN_DELETE)) {
    // The directory itself was modified, so invalidate its cached stat
    // structure.
    node->touch();
    // Recursively invalidate the cached paths rooted at "node/name".
    bool expired = false;
    NodePtr child = node->getChild(event->name, true);
    if (child.get() != nullptr) {
      child->expirePaths();
      expired = true;
    }
    child = node->getChild(event->name, false);
    if (child.get() != nullptr) {
      // The follow=false child is equivalent to the follow=true child unless
      // it's a link.  Avoid duplicate invalidations for non-links.
      child->expirePaths(!expired || child->isLink());
      expired = true;
    }
    if (expired) {
      node->removeChild(event->name);
    }
  }
  if (event->mask & IN_IGNORED) {
    // The kernel removed the directory watch, either as a side effect of
    // directory deletion, or because inotify_rm_watch() was explicitly called
    // during Node destruction.  Delete the corresponding entry from
    // m_watch2Node.  Removal should always succeed here because no other code
    // performs removal.
    m_watch2Node.erase(event->wd);
  }
  return false;
}
#endif

void StatCache::removeWatch(int wd) {
#ifdef __linux__
  inotify_rm_watch(m_ifd, wd);
#endif
}

void StatCache::removePath(const std::string& path, Node* node) {
  NameNodeMap::accessor acc;
  if (m_path2Node.find(acc, path) && acc->second.get() == node) {
    TRACE(1, "StatCache: remove path '%s'\n", path.c_str());
    m_path2Node.erase(acc);
  }
}

void StatCache::removeLPath(const std::string& path, Node* node) {
  NameNodeMap::accessor acc;
  if (m_lpath2Node.find(acc, path) && acc->second.get() == node) {
    TRACE(1, "StatCache: remove link path '%s'\n", path.c_str());
    m_lpath2Node.erase(acc);
  }
}

void StatCache::refresh() {
#ifdef __linux__
  SimpleLock lock(m_lock);

  if (m_ifd == -1) {
    return;
  }

  while (true) {
    int nread = read(m_ifd, m_readBuf, kReadBufSize);
    if (nread == -1) {
      // No pending events.
      assert(errno == EAGAIN);
      // Record the last refresh time *after* processing the event queue, in
      // order to assure that once the event queue has been merged into the
      // cache state, all cached values have timestamps older than
      // m_lastRefresh (assuming no timestamps are ever set into the future).
      m_lastRefresh = time(nullptr);
      TRACE(1, "StatCache: refresh time %lu\n", (unsigned long)m_lastRefresh);
      return;
    }
    for (char* p = m_readBuf; p < m_readBuf + nread;) {
      struct inotify_event* event = (struct inotify_event*) p;
      if (handleEvent(event)) {
        return;
      }
      p += sizeof(struct inotify_event) + event->len;
    }
  }
#endif
}

time_t StatCache::lastRefresh() {
  SimpleLock lock(m_lock);

  return m_lastRefresh;
}

int StatCache::statImpl(const std::string& path, struct stat* buf) {
  // Punt if path is relative.
  if (path.size() == 0 || path[0] != '/') {
    return statSyscall(path, buf);
  }

  {
    NameNodeMap::const_accessor acc;
    if (m_path2Node.find(acc, path)) {
      return acc->second->stat(path, buf);
    }
  }
  {
    SimpleLock lock(m_lock);
    if (mergePath(path, true)) {
      return statSyscall(path, buf);
    }
    {
      NameNodeMap::const_accessor acc;
      if (m_path2Node.find(acc, path)) {
        return acc->second->stat(path, buf, m_lastRefresh);
      }
    }
  }
  not_reached();
}

int StatCache::lstatImpl(const std::string& path, struct stat* buf) {
  // Punt if path is relative.
  if (path.size() == 0 || path[0] != '/') {
    return statSyscall(path, buf);
  }

  {
    NameNodeMap::const_accessor acc;
    if (m_lpath2Node.find(acc, path)) {
      return acc->second->lstat(path, buf);
    }
  }
  {
    SimpleLock lock(m_lock);
    if (mergePath(path, false)) {
      return lstatSyscall(path, buf);
    }
    {
      NameNodeMap::const_accessor acc;
      if (m_lpath2Node.find(acc, path)) {
        return acc->second->lstat(path, buf, m_lastRefresh);
      }
    }
  }
  not_reached();
}

std::string StatCache::readlinkImpl(const std::string& path) {
  // Punt if path is relative.
  if (path.size() == 0 || path[0] != '/') {
    return readlinkSyscall(path);
  }

  {
    NameNodeMap::const_accessor acc;
    if (m_lpath2Node.find(acc, path)) {
      return acc->second->readlink(path);
    }
  }
  {
    SimpleLock lock(m_lock);
    if (mergePath(path, false)) {
      return readlinkSyscall(path);
    }
    {
      NameNodeMap::const_accessor acc;
      if (m_lpath2Node.find(acc, path)) {
        return acc->second->readlink(path, m_lastRefresh);
      }
    }
  }
  not_reached();
}

// StatCache::realpath() is based on the realpath(3) implementation that is
// part of FreeBSD's libc. The following license applies:

/*
 * Copyright (c) 2003 Constantin S. Svintsoff <kostik@iclub.nsu.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#if 0
__FBSDID("$FreeBSD: src/lib/libc/stdlib/realpath.c,v 1.24 2011/11/04 19:56:34 ed Exp $");
#endif

// Find the real name of path, by removing all ".", ".." and symlink
// components.  Returns the resolved path on success, or "" on failure,
std::string StatCache::realpathImpl(const char* path) {
  std::string resolved;
  assert(path != nullptr);
  if (path[0] != '/') {
    return realpathLibc(path);
  }
  struct stat sb;
  unsigned symlinks;
  std::string left, next_token, symlink;
  size_t left_pos;

  symlinks = 0;
  resolved += "/";
  if (path[1] == '\0') {
    TRACE(4, "StatCache: realpath('%s') --> '%s'\n", path, resolved.c_str());
    return resolved;
  }
  left = path;
  left_pos = 0;

  // Iterate over path components in `left'.
  while (left.size() - left_pos != 0) {
    // Extract the next path component and adjust `left' and its length.
    size_t pos = left.find_first_of('/', left_pos);
    next_token = left.substr(left_pos, pos - left_pos);
    left_pos += next_token.size();
    if (pos != std::string::npos) {
      ++left_pos;
    }

    if (resolved[resolved.size() - 1] != '/') {
      resolved += "/";
    }
    if (next_token.size() == 0) {
      continue;
    } else if (next_token.compare(".") == 0) {
      continue;
    } else if (next_token.compare("..") == 0) {
      // Strip the last path component except when we have single "/".
      if (resolved.size() > 1) {
        resolved.erase(resolved.size() - 1);
        resolved.erase(resolved.find_last_of('/') + 1);
      }
      continue;
    }

    // Append the next path component and lstat() it. If lstat() fails we still
    // can return successfully if there are no more path components left.
    resolved += next_token;
    if (lstatImpl(resolved, &sb) != 0) {
      if (errno == ENOENT && pos == std::string::npos) {
        TRACE(4, "StatCache: realpath('%s') --> '%s'\n",
                 path, resolved.c_str());
        return resolved;
      }
      TRACE(4, "StatCache: realpath('%s') --> error\n", path);
      return "";
    }
    if (S_ISLNK(sb.st_mode)) {
      if (symlinks++ > MAXSYMLINKS) {
        TRACE(4, "StatCache: realpath('%s') --> error\n", path);
        return "";
      }
      symlink = readlinkImpl(resolved);
      if (symlink.size() == 0) {
        TRACE(4, "StatCache: realpath('%s') --> error\n", path);
        return "";
      }
      if (symlink[0] == '/') {
        resolved.erase(1);
      } else if (resolved.size() > 1) {
        // Strip the last path component.
        resolved.erase(resolved.size() - 1);
        resolved.erase(resolved.find_last_of('/') + 1);
      }

      // If there are any path components left, then append them to symlink.
      // The result is placed in `left'.
      if (pos != std::string::npos) {
        if (symlink[symlink.size() - 1] != '/') {
          symlink += "/";
        }
        symlink += left.substr(left_pos);
      }
      left = symlink;
      left_pos = 0;
    }
  }

  // Remove trailing slash except when the resolved pathname is a single "/".
  if (resolved.size() > 1 && resolved[resolved.size() - 1] == '/') {
    resolved.erase(resolved.size() - 1);
  }

  TRACE(4, "StatCache: realpath('%s') --> '%s'\n", path, resolved.c_str());
  return resolved;
}

StatCache StatCache::s_sc;

void StatCache::requestInit() {
  if (!RuntimeOption::ServerStatCache) return;
  s_sc.refresh();
}

int StatCache::stat(const std::string& path, struct stat* buf) {
  if (!RuntimeOption::ServerStatCache) return statSyscall(path, buf);
  return s_sc.statImpl(path, buf);
}

int StatCache::lstat(const std::string& path, struct stat* buf) {
  if (!RuntimeOption::ServerStatCache) return lstatSyscall(path, buf);
  return s_sc.lstatImpl(path, buf);
}

std::string StatCache::readlink(const std::string& path) {
  if (!RuntimeOption::ServerStatCache) return readlinkSyscall(path);
  return s_sc.readlinkImpl(path);
}

std::string StatCache::realpath(const char* path) {
  if (!RuntimeOption::ServerStatCache) return realpathLibc(path);
  return s_sc.realpathImpl(path);
}

///////////////////////////////////////////////////////////////////////////////
}
