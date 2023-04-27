/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "watchman/fs/FileDescriptor.h"

namespace watchman {

// Convenience for constructing a Pipe
class Pipe {
 public:
  FileDescriptor read;
  FileDescriptor write;

  // Construct a pipe, setting the close-on-exec and
  // non-blocking bits.
  Pipe();
};

// Convenience for constructing a SocketPair
class SocketPair {
 public:
  FileDescriptor read;
  FileDescriptor write;

  // Construct a socketpair, setting the close-on-exec and
  // non-blocking bits.
  SocketPair();
};

} // namespace watchman
