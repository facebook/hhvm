/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CloseRandomFds.h"

#include <folly/portability/Fcntl.h>
#include <folly/portability/GTest.h>
#include <folly/portability/Unistd.h>

// shouldCloseInheritedFd inspects real descriptors via fcntl; there is nothing
// to exercise on Windows, where it is a no-op.
#ifndef _WIN32

namespace {

// A descriptor that is open, above stdio, and not close-on-exec looks like an
// inherited leak and is selected for closing.
TEST(CloseRandomFds, selectsNonCloexecDescriptor) {
  const int fd = ::open("/dev/null", O_RDONLY);
  ASSERT_NE(fd, -1);
  EXPECT_TRUE(watchman::shouldCloseInheritedFd(fd));
  ::close(fd);
}

// A descriptor we created close-on-exec (as watchman's own libraries do) is
// preserved, so we never free a descriptor number a library has cached.
TEST(CloseRandomFds, preservesCloexecDescriptor) {
  const int fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
  ASSERT_NE(fd, -1);
  EXPECT_FALSE(watchman::shouldCloseInheritedFd(fd));
  ::close(fd);
}

// The standard streams are always left alone.
TEST(CloseRandomFds, preservesStdioDescriptors) {
  EXPECT_FALSE(watchman::shouldCloseInheritedFd(STDERR_FILENO));
}

} // namespace

#endif
