/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Fifo.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include <boost/filesystem.hpp>

#include <glog/logging.h>

#include <folly/FileUtil.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

namespace {

bool exists(const char* fifoPath) {
  return access(fifoPath, F_OK) == 0;
}

bool canWrite(const char* fifoPath) {
  return access(fifoPath, W_OK) == 0;
}

bool create(const char* fifoPath) {
  return mkfifo(fifoPath, 0666) == 0;
}

bool removeFile(const char* fifoPath) {
  return ::remove(fifoPath) == 0;
}

bool isFifo(const char* fifoPath) {
  struct stat st;
  return (stat(fifoPath, &st) != -1) && S_ISFIFO(st.st_mode);
}

} // anonymous namespace

Fifo::Fifo(std::string path) : path_(std::move(path)) {
  if (FOLLY_UNLIKELY(path_.empty())) {
    throw std::invalid_argument("Fifo path cannot be empty");
  }
  auto dir = boost::filesystem::path(path_).parent_path().string();
  ensureDirExistsAndWritable(dir);
}

Fifo::~Fifo() {
  disconnect();
  if (exists(path_.c_str())) {
    if (!removeFile(path_.c_str())) {
      PLOG(ERROR) << "Error removing debug fifo file";
    }
  }
}

bool Fifo::tryConnect() noexcept {
  if (isConnected()) {
    return true;
  }

  if (!exists(path_.c_str())) {
    if (!create(path_.c_str())) {
      static bool logged{false};
      if (!logged) {
        VLOG(1) << "Error creating debug fifo at \"" << path_
                << "\": " << strerror(errno) << " [" << errno << "]";
        logged = true;
      }
      return false;
    }
  }

  if (!isFifo(path_.c_str()) || !canWrite(path_.c_str())) {
    if (!removeFile(path_.c_str()) || !create(path_.c_str())) {
      return false;
    }
  }

  int fd = folly::openNoInt(path_.c_str(), O_WRONLY | O_NONBLOCK);
  if (fd >= 0) {
    fd_.store(fd);
    return true;
  }

  return false;
}

void Fifo::disconnect() noexcept {
  auto oldFd = fd_.exchange(-1);
  if (oldFd >= 0) {
    close(oldFd);
  }
}

bool Fifo::write(void* buf, size_t len) noexcept {
  iovec iov[1];
  iov[0].iov_base = buf;
  iov[0].iov_len = len;
  return write(iov, 1);
}

bool Fifo::write(const struct iovec* iov, size_t iovcnt) noexcept {
  if (folly::writevNoInt(fd_, iov, iovcnt) == -1) {
    if (errno != EAGAIN) {
      PLOG(WARNING) << "Error writing to debug pipe.";
    }
    if (errno == EPIPE) {
      disconnect();
    }
    return false;
  }

  return true;
}

} // namespace memcache
} // namespace facebook
