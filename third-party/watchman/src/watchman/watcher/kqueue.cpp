/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "kqueue.h"
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <array>
#include "watchman/FlagMap.h"
#include "watchman/InMemoryView.h"
#include "watchman/fs/FileDescriptor.h"
#include "watchman/fs/Pipe.h"
#include "watchman/root/Root.h"
#include "watchman/watcher/Watcher.h"
#include "watchman/watcher/WatcherRegistry.h"
#include "watchman/watchman_dir.h"
#include "watchman/watchman_file.h"

#ifdef HAVE_KQUEUE
#if !defined(O_EVTONLY)
#define O_EVTONLY O_RDONLY
#endif

namespace watchman {

namespace {

enum KQueueUdata {
  IS_DIR = 1,
};

bool is_udata_dir(void* udata) {
  return reinterpret_cast<uintptr_t>(udata) == IS_DIR;
}

void* make_udata(bool is_dir) {
  if (is_dir) {
    return reinterpret_cast<void*>(IS_DIR);
  } else {
    return reinterpret_cast<void*>(0);
  }
}

} // namespace

static const struct flag_map kflags[] = {
    {NOTE_DELETE, "NOTE_DELETE"},
    {NOTE_WRITE, "NOTE_WRITE"},
    {NOTE_EXTEND, "NOTE_EXTEND"},
    {NOTE_ATTRIB, "NOTE_ATTRIB"},
    {NOTE_LINK, "NOTE_LINK"},
    {NOTE_RENAME, "NOTE_RENAME"},
    {NOTE_REVOKE, "NOTE_REVOKE"},
    {0, nullptr},
};

KQueueWatcher::KQueueWatcher(
    const w_string& /*root_path*/,
    const Configuration& config,
    bool recursive)
    : Watcher("kqueue", 0),
      maps_(maps(config.getInt(CFG_HINT_NUM_DIRS, HINT_NUM_DIRS))),
      recursive_(recursive) {
  kq_fd = FileDescriptor(kqueue(), "kqueue", FileDescriptor::FDType::Generic);
  kq_fd.setCloExec();
}

bool KQueueWatcher::startWatchFile(struct watchman_file* file) {
  struct kevent k;

  auto full_name = file->parent->getFullPathToChild(file->getName());
  {
    auto rlock = maps_.rlock();
    if (rlock->name_to_fd.find(full_name) != rlock->name_to_fd.end()) {
      // Already watching it
      return true;
    }
  }

  logf(DBG, "watch_file({})\n", full_name);

  int openFlags = O_EVTONLY | O_CLOEXEC;
#if HAVE_DECL_O_SYMLINK
  openFlags |= O_SYMLINK;
#endif
  FileDescriptor fdHolder(
      open(full_name.c_str(), openFlags), FileDescriptor::FDType::Generic);

  auto rawFd = fdHolder.fd();

  if (rawFd == -1) {
    watchman::log(
        watchman::ERR,
        "failed to open ",
        full_name,
        ", O_EVTONLY: ",
        folly::errnoStr(errno),
        "\n");
    return false;
  }

  // When not recursive, watchman is watching the top-level directories as
  // files, make sure that we properly mark these as directory watches.
  bool isDir = false;
  if (!recursive_) {
    struct stat st;
    if (fstat(rawFd, &st) == -1) {
      watchman::log(
          watchman::ERR,
          "failed to stat ",
          full_name,
          ": ",
          folly::errnoStr(errno),
          "\n");
      return false;
    }
    isDir = S_ISDIR(st.st_mode);
  }

  memset(&k, 0, sizeof(k));
  EV_SET(
      &k,
      rawFd,
      EVFILT_VNODE,
      EV_ADD | EV_CLEAR,
      NOTE_WRITE | NOTE_DELETE | NOTE_EXTEND | NOTE_RENAME | NOTE_ATTRIB,
      0,
      make_udata(isDir));

  {
    auto wlock = maps_.wlock();
    wlock->name_to_fd[full_name] = std::move(fdHolder);
    wlock->fd_to_name[rawFd] = full_name;
  }

  if (kevent(kq_fd.fd(), &k, 1, nullptr, 0, 0)) {
    watchman::log(
        watchman::DBG,
        "kevent EV_ADD file ",
        full_name,
        " failed: ",
        full_name.c_str(),
        folly::errnoStr(errno),
        "\n");
    auto wlock = maps_.wlock();
    wlock->name_to_fd.erase(full_name);
    wlock->fd_to_name.erase(rawFd);
  } else {
    watchman::log(
        watchman::DBG, "kevent file ", full_name, " -> ", rawFd, "\n");
  }

  return true;
}

std::unique_ptr<DirHandle> KQueueWatcher::startWatchDir(
    const std::shared_ptr<Root>& root,
    const char* path) {
  struct stat st, osdirst;
  struct kevent k;

  auto osdir = openDir(path);

  FileDescriptor fdHolder(
      open(path, O_NOFOLLOW | O_EVTONLY | O_CLOEXEC),
      FileDescriptor::FDType::Generic);
  auto rawFd = fdHolder.fd();
  if (rawFd == -1) {
    // directory got deleted between opendir and open
    throw std::system_error(
        errno, std::generic_category(), std::string("open O_EVTONLY: ") + path);
  }
  if (fstat(rawFd, &st) == -1 || fstat(osdir->getFd(), &osdirst) == -1) {
    // whaaa?
    root->scheduleRecrawl("fstat failed");
    throw std::system_error(
        errno,
        std::generic_category(),
        std::string("fstat failed for dir ") + path);
  }

  if (st.st_dev != osdirst.st_dev || st.st_ino != osdirst.st_ino) {
    // directory got replaced between opendir and open -- at this point its
    // parent's being watched, so we let filesystem events take care of it
    throw std::system_error(
        ENOTDIR,
        std::generic_category(),
        std::string("directory replaced between opendir and open: ") + path);
  }

  memset(&k, 0, sizeof(k));
  w_string dir_name{path};
  EV_SET(
      &k,
      rawFd,
      EVFILT_VNODE,
      EV_ADD | EV_CLEAR,
      NOTE_WRITE | NOTE_DELETE | NOTE_EXTEND | NOTE_RENAME,
      0,
      make_udata(true));

  // Our mapping needs to be visible before we add it to the queue,
  // otherwise we can get a wakeup and not know what it is
  {
    auto wlock = maps_.wlock();
    wlock->name_to_fd[dir_name] = std::move(fdHolder);
    wlock->fd_to_name[rawFd] = dir_name;
  }

  if (kevent(kq_fd.fd(), &k, 1, nullptr, 0, 0)) {
    logf(DBG, "kevent EV_ADD dir {} failed: {}", path, folly::errnoStr(errno));

    auto wlock = maps_.wlock();
    wlock->name_to_fd.erase(dir_name);
    wlock->fd_to_name.erase(rawFd);
  } else {
    watchman::log(watchman::DBG, "kevent dir ", dir_name, " -> ", rawFd, "\n");
  }

  return osdir;
}

Watcher::ConsumeNotifyRet KQueueWatcher::consumeNotify(
    const std::shared_ptr<Root>& root,
    PendingChanges& coll) {
  struct timespec ts = {0, 0};

  errno = 0;
  int n = kevent(
      kq_fd.fd(),
      nullptr,
      0,
      keventbuf,
      sizeof(keventbuf) / sizeof(keventbuf[0]),
      &ts);
  logf(
      DBG,
      "consume_kqueue: {} n={} err={}\n",
      root->root_path,
      n,
      folly::errnoStr(errno));
  if (root->inner.cancelled) {
    return {false};
  }

  auto now = std::chrono::system_clock::now();
  for (int i = 0; n > 0 && i < n; i++) {
    uint32_t fflags = keventbuf[i].fflags;
    bool is_dir = is_udata_dir(keventbuf[i].udata);
    char flags_label[128];
    int fd = keventbuf[i].ident;

    w_expand_flags(kflags, fflags, flags_label, sizeof(flags_label));
    auto wlock = maps_.wlock();
    auto it = wlock->fd_to_name.find(fd);
    if (it == wlock->fd_to_name.end()) {
      // Was likely a buffered notification for something that we decided
      // to stop watching
      logf(
          DBG,
          " KQ notif for fd={}; flags={:x} {} no ref for it in fd_to_name\n",
          fd,
          fflags,
          flags_label);
      continue;
    }
    w_string path = it->second;

    logf(DBG, " KQ fd={} path {} [{:x} {}]\n", fd, path, fflags, flags_label);
    if ((fflags & (NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE))) {
      struct kevent k;

      if (path == root->root_path) {
        logf(
            ERR,
            "root dir {} has been (re)moved [code {:x}], canceling watch\n",
            root->root_path,
            fflags);
        return {true};
      }

      // Remove our watch bits
      memset(&k, 0, sizeof(k));
      EV_SET(&k, fd, EVFILT_VNODE, EV_DELETE, 0, 0, nullptr);
      kevent(kq_fd.fd(), &k, 1, nullptr, 0, 0);
      wlock->name_to_fd.erase(path);
      wlock->fd_to_name.erase(fd);
    }

    PendingFlags flags = W_PENDING_VIA_NOTIFY;
    if (!is_dir) {
      // TODO(xavierd): I believe we need this in case a file is replaced by a
      // directory.
      flags |= W_PENDING_RECURSIVE;
    } else {
      // You might be tempted to use W_PENDING_NONRECURSIVE_SCAN here, but this
      // would lead to scanning the changed directories too, which when used in
      // the kqueue+fsevents watcher will lead to cookies being discovered
      // prior to FSEvents reporting them!
      // TODO(xavierd): It's unclear to me why not specifying
      // W_PENDING_NONRECURSIVE_SCAN here still allows cookies to be
      // discovered...
    }
    coll.add(path, now, flags);
  }

  return {false};
}

bool KQueueWatcher::waitNotify(int timeoutms) {
  std::array<struct pollfd, 2> pfd;

  pfd[0].fd = kq_fd.fd();
  pfd[0].events = POLLIN;
  pfd[1].fd = terminatePipe_.read.fd();
  pfd[1].events = POLLIN;

  int n = poll(pfd.data(), pfd.size(), timeoutms);

  if (n > 0) {
    if (pfd[1].revents) {
      // We were signalled via signalThreads
      return false;
    }
    return pfd[0].revents != 0;
  }
  return false;
}

void KQueueWatcher::stopThreads() {
  ignore_result(write(terminatePipe_.write.fd(), "X", 1));
}

static RegisterWatcher<KQueueWatcher> reg(
    "kqueue",
    -1 /* last resort on macOS */);

} // namespace watchman

#endif // HAVE_KQUEUE

/* vim:ts=2:sw=2:et:
 */
