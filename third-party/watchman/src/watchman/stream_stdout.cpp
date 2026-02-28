/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Logging.h"
#include "watchman/portability/WinError.h"
#include "watchman/watchman_stream.h"

using watchman::FileDescriptor;
using namespace watchman;

namespace {
class StdioStream : public watchman_stream {
  const FileDescriptor& fd_;

 public:
  explicit StdioStream(const FileDescriptor& fd) : fd_(fd) {}

  int read(void* buf, int size) override {
    auto result = fd_.read(buf, size);
    if (result.hasError()) {
      errno = result.error().value();
#ifdef _WIN32
      // TODO: propagate Result<int, std::error_code> as return type
      errno = map_win32_err(errno);
#endif
      return -1;
    }
    return result.value();
  }

  int write(const void* buf, int size) override {
    auto result = fd_.write(buf, size);
    if (result.hasError()) {
      errno = result.error().value();
#ifdef _WIN32
      // TODO: propagate Result<int, std::error_code> as return type
      errno = map_win32_err(errno);
#endif
      return -1;
    }
    return result.value();
  }

  watchman_event* getEvents() override {
    log(FATAL, "calling get_events on a stdio stm\n");
    return nullptr;
  }

  void setNonBlock(bool) override {}

  bool rewind() override {
    return false;
  }

  bool shutdown() override {
    return false;
  }

  bool peerIsOwner() override {
    return false;
  }

  pid_t getPeerProcessID() const override {
    return 0;
  }

  const watchman::FileDescriptor& getFileDescriptor() const override {
    return fd_;
  }
};
} // namespace

watchman_stream* w_stm_stdout() {
  static StdioStream stdoutStream(FileDescriptor::stdOut());
  return &stdoutStream;
}

watchman_stream* w_stm_stdin() {
  static StdioStream stdinStream(FileDescriptor::stdIn());
  return &stdinStream;
}
