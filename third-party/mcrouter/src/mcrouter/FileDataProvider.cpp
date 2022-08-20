/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FileDataProvider.h"

#include <poll.h>
#include <sys/inotify.h>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>
#include <glog/logging.h>

#include <folly/FileUtil.h>
#include <folly/Format.h>

using boost::filesystem::complete;
using boost::filesystem::path;
using boost::filesystem::read_symlink;

DEFINE_bool(
    mcrouter_enable_inotify_watch,
    true,
    "Enable inotify watch on config file");

namespace facebook {
namespace memcache {
namespace mcrouter {

const size_t INOTIFY_BUF_SIZE = 1024;

/**
 *  We want to watch for the file being modified, moved, or deleted,
 *  and the same for any symlinks (inotify should watch the symlink itself)
 */
const uint32_t INOTIFY_MASK =
    IN_MODIFY | IN_MOVE_SELF | IN_DELETE_SELF | IN_DONT_FOLLOW;

void FileDataProvider::updateInotifyWatch() {
  /**
   * Initiailze inotify and get the file descriptor we will use to watch for
   * changes.
   */
  auto inotifyFD = inotify_init();
  if (inotifyFD < 0) {
    throw std::runtime_error(
        folly::format(
            "Failed to initialize inotify for '{}'. Errno: '{}'",
            filePath_.data(),
            errno)
            .str());
  }
  folly::File tmpInotify(inotifyFD, /* ownsFd */ true);
  /**
   * Right now mcrouter configs are a symlink to the actual config file.
   * We need to watch both the link and the config file for changes. Just
   * to be safe, write the code to handle chained symlinks.
   */
  path link(filePath_);
  while (true) {
    // Add a watch on the current link or file
    int wd = inotify_add_watch(inotifyFD, link.string().data(), INOTIFY_MASK);
    if (wd < 0) {
      throw std::runtime_error(
          folly::format(
              "Can not add inotify watch for '{}'", link.string().data())
              .str());
    }
    // Read the link (if it is one)
    boost::system::error_code ec;
    auto file = read_symlink(link, ec);
    if (file.empty()) {
      break;
    }
    // We read a link
    file = complete(file, link.parent_path());
    std::swap(link, file);
  }
  inotify_ = std::move(tmpInotify);
}

FileDataProvider::FileDataProvider(std::string filePath)
    : filePath_(std::move(filePath)) {
  if (filePath_.empty()) {
    throw std::runtime_error("File path empty");
  }

  if (FLAGS_mcrouter_enable_inotify_watch) {
    updateInotifyWatch();
  }
}

std::string FileDataProvider::load() const {
  std::string result;
  if (!folly::readFile(filePath_.data(), result)) {
    throw std::runtime_error(
        folly::format("Can not read file '{}'", filePath_.data()).str());
  }
  return result;
}

bool FileDataProvider::hasUpdate() {
  if (!inotify_) {
    return false;
  }

  struct pollfd pollfd;
  pollfd.fd = inotify_.fd();
  pollfd.events = POLLIN;
  auto retval = poll(&pollfd, /* nfds = */ 1, /* timeout = */ 0);
  if (retval < 0) {
    throw std::runtime_error(
        folly::format("poll on inotifyFD for '{}' failed", filePath_.data())
            .str());
  } else if (retval > 0) {
    struct inotify_event inotifyBuffer[INOTIFY_BUF_SIZE];
    // Data is ready to read from inotifyFD.  We need to read it so that
    // select doesn't immediately return next time.
    auto len = read(inotify_.fd(), inotifyBuffer, sizeof(inotifyBuffer));
    LOG_IF(ERROR, len < 0) << "Read from inotify failed";

    updateInotifyWatch();
    return true;
  }
  return false;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
