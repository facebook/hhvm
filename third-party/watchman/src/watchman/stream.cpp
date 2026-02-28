/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <thread>
#include "watchman/WatchmanConfig.h"
#include "watchman/watchman_stream.h"

#ifdef _WIN32
#include <io.h> // @manual
#endif

using namespace watchman;

int w_poll_events(EventPoll* p, int n, int timeoutms) {
#ifdef _WIN32
  if (!p->evt->isSocket()) {
    return w_poll_events_named_pipe(p, n, timeoutms);
  }
#endif
  return w_poll_events_sockets(p, n, timeoutms);
}

#if defined(HAVE_MKOSTEMP) && defined(sun)
// Not guaranteed to be defined in stdlib.h
extern int mkostemp(char*, int);
#endif

std::unique_ptr<watchman_stream> w_mkstemp(char* templ) {
#if defined(_WIN32)
  char* name = _mktemp(templ);
  if (!name) {
    return nullptr;
  }
  // Most annoying aspect of windows is the latency around
  // file handle exclusivity.  We could avoid this dumb loop
  // by implementing our own mkostemp, but this is the most
  // expedient option for the moment.
  for (size_t attempts = 0; attempts < 10; ++attempts) {
    auto stm = w_stm_open(name, O_RDWR | O_CLOEXEC | O_CREAT | O_TRUNC, 0600);
    if (stm) {
      return stm;
    }
    if (errno == EACCES) {
      /* sleep override */ std::this_thread::sleep_for(
          std::chrono::microseconds(2000));
      continue;
    }
    return nullptr;
  }
  return nullptr;
#else
  FileDescriptor fd;
#ifdef HAVE_MKOSTEMP
  fd = FileDescriptor(
      mkostemp(templ, O_CLOEXEC), FileDescriptor::FDType::Generic);
#else
  fd = FileDescriptor(mkstemp(templ), FileDescriptor::FDType::Generic);
#endif
  if (!fd) {
    return nullptr;
  }
  fd.setCloExec();

  return w_stm_fdopen(std::move(fd));
#endif
}
