/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* TODO:
 * This watcher fails with the scm tests */

#include <folly/String.h>
#include <folly/Synchronized.h>
#include <memory>
#include "watchman/InMemoryView.h"

#ifdef HAVE_PORT_CREATE

#define WATCHMAN_PORT_EVENTS FILE_MODIFIED | FILE_ATTRIB | FILE_NOFOLLOW

struct watchman_port_file {
  file_obj_t port_file;
  w_string name;
  bool is_dir;
};

using watchman::FileDescriptor;
using watchman::Pipe;

struct PortFSWatcher : public Watcher {
  FileDescriptor port_fd;
  FileDescriptor port_delete_fd;
  Pipe terminatePipe_;

  /* map of file name to watchman_port_file */
  folly::Synchronized<
      std::unordered_map<w_string, std::unique_ptr<watchman_port_file>>>
      port_files;

  std::unique_ptr<watchman_port_file> root_delete_w_port_file;
  bool root_deleted;

  port_event_t portevents[WATCHMAN_BATCH_LIMIT];

  explicit PortFSWatcher(watchman_root* root);

  bool start(const std::shared_ptr<watchman_root>& root) override;

  std::unique_ptr<DirHandle> startWatchDir(
      const std::shared_ptr<watchman_root>& root,
      const char* path) override;

  bool startWatchFile(struct watchman_file* file) override;

  Watcher::ConsumeNotifyRet consumeNotify(
      const std::shared_ptr<watchman_root>& root,
      PendingCollection::LockedPtr& coll) override;

  bool waitNotify(int timeoutms) override;
  void signalThreads() override;
  bool do_watch(
      const w_string& name,
      const watchman::FileInformation& finfo,
      bool throw_on_error);
};

static const struct flag_map pflags[] = {
    {FILE_ACCESS, "FILE_ACCESS"},
    {FILE_MODIFIED, "FILE_MODIFIED"},
    {FILE_ATTRIB, "FILE_ATTRIB"},
    {FILE_DELETE, "FILE_DELETE"},
    {FILE_RENAME_TO, "FILE_RENAME_TO"},
    {FILE_RENAME_FROM, "FILE_RENAME_FROM"},
    {UNMOUNTED, "UNMOUNTED"},
    {MOUNTEDOVER, "MOUNTEDOVER"},
    {0, nullptr},
};

static std::unique_ptr<watchman_port_file> make_port_file(
    const w_string& name,
    const watchman::FileInformation& finfo) {
  auto f = std::make_unique<watchman_port_file>();

  f->name = name;
  f->port_file.fo_name = (char*)name.c_str();
  f->port_file.fo_atime = finfo.atime;
  f->port_file.fo_mtime = finfo.mtime;
  f->port_file.fo_ctime = finfo.ctime;
  f->is_dir = finfo.isDir();

  return f;
}

PortFSWatcher::PortFSWatcher(watchman_root* root)
    : Watcher("portfs", 0),
      port_fd(port_create(), "port_create()"),
      port_delete_fd(port_create(), "port_create()"),
      root_deleted(false) {
  auto wlock = port_files.wlock();
  wlock->reserve(root->config.getInt(CFG_HINT_NUM_DIRS, HINT_NUM_DIRS));
  port_fd.setCloExec();
  port_delete_fd.setCloExec();
}

bool PortFSWatcher::do_watch(
    const w_string& name,
    const watchman::FileInformation& finfo,
    bool throw_on_error) {
  auto wlock = port_files.wlock();
  if (wlock->find(name) != wlock->end()) {
    // Already watching it
    return true;
  }

  auto f = make_port_file(name, finfo);
  auto rawFile = f.get();
  wlock->emplace(name, std::move(f));

  logf(DBG, "watching {}\n", name);
  errno = 0;
  if (port_associate(
          port_fd.fd(),
          PORT_SOURCE_FILE,
          (uintptr_t)&rawFile->port_file,
          WATCHMAN_PORT_EVENTS,
          (void*)rawFile)) {
    int err = errno;
    logf(
        ERR,
        "port_associate {} {}\n",
        rawFile->port_file.fo_name,
        folly::errnoStr(errno));
    wlock->erase(name);
    if (throw_on_error) {
      throw std::system_error(err, std::generic_category(), "port_associate");
    }
    return false;
  }

  return true;
}

/*
 * We need to have an extra port the catches the delete event on the root.
 * The reason for this is that the creation or delete of one of the
 * files/directories directly under the root will cause an FILE_MODIFIED,
 * FILE_ATTRIB event on the root, because just one event can be generated
 * per file_obj the delete event coming afterwards is not seen.
 */
bool PortFSWatcher::start(const std::shared_ptr<watchman_root>& root) {
  struct stat st;
  if (stat(root->root_path.c_str(), &st)) {
    watchman::log(watchman::ERR, "stat failed in PortFS root delete watch");
    root->cancel("root inaccessible");
    return false;
  }

  auto f = make_port_file(root->root_path, watchman::FileInformation(st));
  auto rawFile = f.get();
  root_delete_w_port_file = std::move(f);

  logf(DBG, "watching {} for delete events\n", root->root_path);
  errno = 0;
  if (port_associate(
          port_delete_fd.fd(),
          PORT_SOURCE_FILE,
          (uintptr_t)&rawFile->port_file,
          0, // we only want the delete events
          (void*)rawFile)) {
    watchman::log(
        watchman::ERR, "port_associate failed in PortFS root delete watch");
    return false;
  }
  return true;
}

bool PortFSWatcher::startWatchFile(struct watchman_file* file) {
  auto name = file->parent->getFullPathToChild(file->getName());
  if (!name) {
    return false;
  }

  return do_watch(name, file->stat, false);
}

std::unique_ptr<DirHandle> PortFSWatcher::startWatchDir(
    const std::shared_ptr<watchman_root>& root,
    const char* path) {
  struct stat st;

  auto osdir = w_dir_open(path);

  w_string fullPath{path};
  if (fstat(osdir->getFd(), &st) == -1) {
    if (fullPath == root->root_path) {
      root->cancel("root inaccessible");
    } else {
      // whaaa?
      root->scheduleRecrawl("fstat failed");
    }
    throw std::system_error(
        errno,
        std::generic_category(),
        std::string("fstat failed for dir ") + path);
  }

  do_watch(fullPath, watchman::FileInformation(st), true);

  return osdir;
}

Watcher::ConsumeNotifyRet PortFSWatcher::consumeNotify(
    const std::shared_ptr<watchman_root>& root,
    PendingCollection::LockedPtr& coll) {
  uint_t i, n;
  struct timeval now;

  // root got deleted, cancel the watch
  if (root_deleted) {
    return {false, true};
  }

  errno = 0;

  n = 1;
  if (port_getn(
          port_fd.fd(),
          portevents,
          sizeof(portevents) / sizeof(portevents[0]),
          &n,
          nullptr)) {
    if (errno == EINTR) {
      return {false, false};
    }
    logf(FATAL, "port_getn: {}\n", folly::errnoStr(errno));
  }

  logf(DBG, "port_getn: n={}\n", n);

  if (n == 0) {
    return {false, false};
  }

  auto wlock = port_files.wlock();

  gettimeofday(&now, nullptr);
  for (i = 0; i < n; i++) {
    struct watchman_port_file* f;
    uint32_t pe = portevents[i].portev_events;
    char flags_label[128];

    f = (struct watchman_port_file*)portevents[i].portev_user;
    w_expand_flags(pflags, pe, flags_label, sizeof(flags_label));
    logf(DBG, "port: {} [{:x} {}]\n", f->port_file.fo_name, pe, flags_label);

    if ((pe & (FILE_RENAME_FROM | UNMOUNTED | MOUNTEDOVER | FILE_DELETE)) &&
        (f->name == root->root_path)) {
      logf(
          ERR,
          "root dir {} has been (re)moved (code {:x} {}), canceling watch\n",
          root->root_path,
          pe,
          flags_label);
      return {false, true};
    }
    coll->add(
        f->name,
        now,
        (f->is_dir ? W_PENDING_RECURSIVE : 0) | W_PENDING_VIA_NOTIFY);

    // It was port_dissociate'd implicitly.  We'll re-establish a
    // watch later when portfs_root_start_watch_(file|dir) are called again
    wlock->erase(f->name);
  }

  return {true, false};
}

bool PortFSWatcher::waitNotify(int timeoutms) {
  int n;
  std::array<struct pollfd, 3> pfd;

  pfd[0].fd = port_fd.fd();
  pfd[0].events = POLLIN;
  pfd[1].fd = port_delete_fd.fd();
  pfd[1].events = POLLIN;
  pfd[2].fd = terminatePipe_.read.fd();
  pfd[2].events = POLLIN;

  n = poll(pfd.data(), pfd.size(), timeoutms);

  if (n > 0) {
    if (pfd[2].revents) {
      // We were signalled via signalThreads
      return false;
    }
    if (pfd[1].revents) {
      // An exceptional event (delete) occured on the root so delete it
      root_deleted = true;
      return true;
    }
    return (pfd[0].revents != 0);
  }

  return false;
}
void PortFSWatcher::signalThreads() {
  ignore_result(write(terminatePipe_.write.fd(), "X", 1));
}

static RegisterWatcher<PortFSWatcher> reg(
    "portfs",
    1 /* higher priority than inotify */);

#endif // HAVE_PORT_CREATE

/* vim:ts=2:sw=2:et:
 */
