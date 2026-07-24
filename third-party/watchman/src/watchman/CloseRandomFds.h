/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace watchman {

// Whether `fd` looks like a descriptor we inherited from a parent and should
// close. Inherited descriptors that leaked across our exec are above the stdio
// range and are NOT close-on-exec: cloexec descriptors are closed by exec, so
// anything still open with FD_CLOEXEC set was created by watchman itself.
//
// Preserving cloexec descriptors is what avoids the daemon corruption that
// motivated this helper: libunwind's O_CLOEXEC address-validation pipe (used by
// jemalloc heap profiling) caches its raw fd numbers and later close()s them.
// If we hard-closed it here, watchman would reuse those numbers for the pidfile
// lock and listener socket, and libunwind would tear them down. Exposed for
// unit testing.
bool shouldCloseInheritedFd(int fd);

// Close descriptors we may have inherited so they don't leak into child
// processes we later execute (triggers, hooks), while leaving our own
// close-on-exec descriptors (e.g. libunwind's validation pipe) intact.
// See shouldCloseInheritedFd.
void close_random_fds();

} // namespace watchman
