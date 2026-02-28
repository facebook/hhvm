/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "watchman/fs/FileDescriptor.h"

// Very limited stream abstraction to make it easier to
// deal with portability between Windows and POSIX.

namespace watchman {

class Event {
 public:
  virtual ~Event() = default;
  virtual void notify() = 0;
  virtual bool testAndClear() = 0;
  virtual FileDescriptor::system_handle_type system_handle() = 0;
  virtual bool isSocket() = 0;
};

class Stream {
 public:
  virtual ~Stream() = default;
  virtual int read(void* buf, int size) = 0;
  virtual int write(const void* buf, int size) = 0;
  virtual Event* getEvents() = 0;
  virtual void setNonBlock(bool nonBlock) = 0;
  virtual bool rewind() = 0;
  virtual bool shutdown() = 0;
  virtual bool peerIsOwner() = 0;
  virtual pid_t getPeerProcessID() const = 0;
  virtual const FileDescriptor& getFileDescriptor() const = 0;
};

struct EventPoll {
  Event* evt;
  bool ready;
};

} // namespace watchman

using watchman_event = watchman::Event;
using watchman_stream = watchman::Stream;

// Make a event that can be manually signalled
std::unique_ptr<watchman_event> w_event_make_sockets();
std::unique_ptr<watchman_event> w_event_make_named_pipe();

// Go to sleep for up to timeoutms.
// Returns sooner if any of the watchman_event objects referenced
// in the array P are signalled
int w_poll_events_named_pipe(watchman::EventPoll* p, int n, int timeoutms);
int w_poll_events_sockets(watchman::EventPoll* p, int n, int timeoutms);
int w_poll_events(watchman::EventPoll* p, int n, int timeoutms);

watchman_stream* w_stm_stdout();
watchman_stream* w_stm_stdin();
watchman::ResultErrno<std::unique_ptr<watchman_stream>> w_stm_connect_unix(
    const char* path,
    int timeoutms);
#ifdef _WIN32
std::unique_ptr<watchman_stream> w_stm_connect_named_pipe(
    const char* path,
    int timeoutms);
watchman::FileDescriptor w_handle_open(const char* path, int flags);
#endif
std::unique_ptr<watchman_stream> w_stm_fdopen(watchman::FileDescriptor&& fd);
std::unique_ptr<watchman_stream> w_stm_fdopen_windows(
    watchman::FileDescriptor&& fd);
std::unique_ptr<watchman_stream> w_stm_open(const char* path, int flags, ...);

// Make a temporary file name and open it.
// Marks the file as CLOEXEC
std::unique_ptr<watchman_stream> w_mkstemp(char* templ);
